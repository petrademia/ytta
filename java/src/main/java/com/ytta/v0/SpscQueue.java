package com.ytta.v0;

/**
 * Bounded SPSC ring. Capacity must be a power of two.
 * Single-producer / single-consumer only.
 */
public final class SpscQueue<T> {
    private final Object[] buf;
    private final int mask;
    private volatile long write;
    private volatile long read;

    public SpscQueue(int capacity) {
        if (capacity < 2 || (capacity & (capacity - 1)) != 0) {
            throw new IllegalArgumentException(
                    "spsc capacity must be power of two >= 2, got " + capacity);
        }
        this.buf = new Object[capacity];
        this.mask = capacity - 1;
    }

    public int capacity() {
        return buf.length;
    }

    public boolean tryPush(T value) {
        long w = write;
        long r = read;
        if (w - r >= buf.length) {
            return false;
        }
        buf[(int) (w & mask)] = value;
        write = w + 1;
        return true;
    }

    @SuppressWarnings("unchecked")
    public T tryPop() {
        long r = read;
        long w = write;
        if (r == w) {
            return null;
        }
        T value = (T) buf[(int) (r & mask)];
        buf[(int) (r & mask)] = null;
        read = r + 1;
        return value;
    }
}
