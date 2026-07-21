package com.ytta;

import java.util.Optional;
import java.util.concurrent.CountDownLatch;

public final class ThreadUtils {
    private ThreadUtils() {
    }

    public static Optional<Thread> createAndStartThread(String name, Runnable runnable) {
        if (name == null || name.isBlank()) {
            return Optional.empty();
        }

        CountDownLatch started = new CountDownLatch(1);
        Thread thread = new Thread(() -> {
            started.countDown();
            runnable.run();
        }, name);

        thread.start();
        try {
            started.await();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return Optional.empty();
        }

        return Optional.of(thread);
    }
}
