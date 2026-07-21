package com.ytta.v0;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

class SpscPoolTest {
    @Test
    void spscFifoAndBounds() {
        SpscQueue<Integer> q = new SpscQueue<>(4);
        assertTrue(q.tryPush(1));
        assertTrue(q.tryPush(2));
        assertTrue(q.tryPush(3));
        assertTrue(q.tryPush(4));
        assertFalse(q.tryPush(5));
        assertEquals(1, q.tryPop());
        assertEquals(2, q.tryPop());
        assertEquals(3, q.tryPop());
        assertEquals(4, q.tryPop());
        assertNull(q.tryPop());
    }

    @Test
    void spscReuseAfterPop() {
        SpscQueue<String> q = new SpscQueue<>(2);
        assertTrue(q.tryPush("a"));
        assertTrue(q.tryPush("b"));
        assertFalse(q.tryPush("c"));
        assertEquals("a", q.tryPop());
        assertTrue(q.tryPush("c"));
        assertEquals("b", q.tryPop());
        assertEquals("c", q.tryPop());
    }

    @Test
    void spscBadCapacity() {
        assertThrows(IllegalArgumentException.class, () -> new SpscQueue<Integer>(3));
    }

    @Test
    void poolReuseWithoutGrowth() {
        ObjectPool<int[]> pool = new ObjectPool<>(() -> new int[1], 0);
        int[] a = pool.acquire();
        int[] b = pool.acquire();
        assertTrue(a != b);
        pool.release(a);
        int before = pool.created();
        int[] c = pool.acquire();
        assertSame(a, c);
        assertEquals(before, pool.created());
        pool.release(b);
        pool.release(c);
        assertEquals(2, pool.freeCount());
        before = pool.created();
        for (int i = 0; i < 1000; i++) {
            int[] p = pool.acquire();
            p[0] = i;
            pool.release(p);
        }
        assertEquals(before, pool.created());
    }
}
