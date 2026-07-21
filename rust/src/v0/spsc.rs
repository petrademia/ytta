use std::cell::UnsafeCell;
use std::sync::atomic::{AtomicUsize, Ordering};

/// Bounded SPSC ring. Capacity must be a power of two.
pub struct SpscQueue<T> {
    buf: Box<[UnsafeCell<Option<T>>]>,
    mask: usize,
    write: AtomicUsize,
    read: AtomicUsize,
}

// Safety: single producer / single consumer access pattern.
unsafe impl<T: Send> Send for SpscQueue<T> {}
unsafe impl<T: Send> Sync for SpscQueue<T> {}

impl<T> SpscQueue<T> {
    pub fn new(capacity: usize) -> Result<Self, String> {
        if capacity < 2 || !capacity.is_power_of_two() {
            return Err(format!(
                "spsc capacity must be power of two >= 2, got {capacity}"
            ));
        }
        let mut slots = Vec::with_capacity(capacity);
        for _ in 0..capacity {
            slots.push(UnsafeCell::new(None));
        }
        Ok(Self {
            buf: slots.into_boxed_slice(),
            mask: capacity - 1,
            write: AtomicUsize::new(0),
            read: AtomicUsize::new(0),
        })
    }

    pub fn capacity(&self) -> usize {
        self.buf.len()
    }

    pub fn try_push(&self, value: T) -> bool {
        let w = self.write.load(Ordering::Relaxed);
        let r = self.read.load(Ordering::Acquire);
        if w.wrapping_sub(r) >= self.buf.len() {
            return false;
        }
        unsafe {
            *self.buf[w & self.mask].get() = Some(value);
        }
        self.write.store(w.wrapping_add(1), Ordering::Release);
        true
    }

    pub fn try_pop(&self) -> Option<T> {
        let r = self.read.load(Ordering::Relaxed);
        let w = self.write.load(Ordering::Acquire);
        if r == w {
            return None;
        }
        let value = unsafe { (*self.buf[r & self.mask].get()).take() };
        self.read.store(r.wrapping_add(1), Ordering::Release);
        value
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn fifo_and_bounds() {
        let q = SpscQueue::new(4).unwrap();
        assert!(q.try_push(1));
        assert!(q.try_push(2));
        assert!(q.try_push(3));
        assert!(q.try_push(4));
        assert!(!q.try_push(5));
        assert_eq!(q.try_pop(), Some(1));
        assert_eq!(q.try_pop(), Some(2));
        assert_eq!(q.try_pop(), Some(3));
        assert_eq!(q.try_pop(), Some(4));
        assert_eq!(q.try_pop(), None);
    }

    #[test]
    fn reuse_after_pop() {
        let q = SpscQueue::new(2).unwrap();
        assert!(q.try_push("a".to_string()));
        assert!(q.try_push("b".to_string()));
        assert!(!q.try_push("c".to_string()));
        assert_eq!(q.try_pop().as_deref(), Some("a"));
        assert!(q.try_push("c".to_string()));
        assert_eq!(q.try_pop().as_deref(), Some("b"));
        assert_eq!(q.try_pop().as_deref(), Some("c"));
    }

    #[test]
    fn bad_capacity() {
        assert!(SpscQueue::<i32>::new(3).is_err());
    }
}
