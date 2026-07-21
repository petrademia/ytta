package com.ytta.v0;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Supplier;

/** Tiny free-list object pool. */
public final class ObjectPool<T> {
    private final List<T> free = new ArrayList<>();
    private final Supplier<T> factory;
    private int created;

    public ObjectPool(Supplier<T> factory, int prewarm) {
        this.factory = factory;
        for (int i = 0; i < prewarm; i++) {
            free.add(factory.get());
            created++;
        }
    }

    public T acquire() {
        int n = free.size();
        if (n == 0) {
            created++;
            return factory.get();
        }
        return free.remove(n - 1);
    }

    public void release(T value) {
        free.add(value);
    }

    public int freeCount() {
        return free.size();
    }

    public int created() {
        return created;
    }
}
