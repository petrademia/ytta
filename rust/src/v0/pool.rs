/// Tiny free-list pool. Values are owned; release returns them to the free list.
pub struct ObjectPool<T> {
    free: Vec<T>,
    factory: Box<dyn Fn() -> T>,
    created: usize,
}

impl<T> ObjectPool<T> {
    pub fn new(factory: impl Fn() -> T + 'static, prewarm: usize) -> Self {
        let factory: Box<dyn Fn() -> T> = Box::new(factory);
        let mut free = Vec::with_capacity(prewarm);
        for _ in 0..prewarm {
            free.push((factory)());
        }
        Self {
            free,
            factory,
            created: prewarm,
        }
    }

    pub fn acquire(&mut self) -> T {
        if let Some(v) = self.free.pop() {
            return v;
        }
        self.created += 1;
        (self.factory)()
    }

    pub fn release(&mut self, v: T) {
        self.free.push(v);
    }

    pub fn free_count(&self) -> usize {
        self.free.len()
    }

    pub fn created(&self) -> usize {
        self.created
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn reuse_without_growth() {
        let mut pool = ObjectPool::new(|| Box::new(0_i32), 0);
        let a = pool.acquire();
        let b = pool.acquire();
        assert!(!std::ptr::eq(a.as_ref(), b.as_ref()));
        pool.release(a);
        let before = pool.created();
        let c = pool.acquire();
        assert_eq!(pool.created(), before);
        pool.release(b);
        pool.release(c);
        assert_eq!(pool.free_count(), 2);
        let before = pool.created();
        for i in 0..1000 {
            let mut p = pool.acquire();
            *p = i;
            pool.release(p);
        }
        assert_eq!(pool.created(), before);
    }
}
