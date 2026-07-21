package com.ytta;

import java.util.Optional;

public class App {
    private static void dummyFunction(int a, int b, boolean shouldSleep) {
        System.out.println("dummyFunction(" + a + ", " + b + ")");
        System.out.println("dummyFunction output=" + (a + b));

        if (shouldSleep) {
            System.out.println("dummyFunction sleeping...");
            try {
                Thread.sleep(2000);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            }
        }

        System.out.println("dummyFunction done.");
    }

    public static void main(String[] args) {
        Optional<Thread> t1 = ThreadUtils.createAndStartThread("dummyFunction1", () -> dummyFunction(12, 21, false));
        Optional<Thread> t2 = ThreadUtils.createAndStartThread("dummyFunction2", () -> dummyFunction(15, 51, true));

        if (t1.isEmpty() || t2.isEmpty()) {
            System.err.println("Failed to start one or more threads.");
            t1.ifPresent(thread -> {
                try {
                    thread.join();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            });
            t2.ifPresent(thread -> {
                try {
                    thread.join();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            });
            System.exit(1);
        }

        System.out.println("main waiting for threads to be done.");
        try {
            t1.get().join();
            t2.get().join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            System.exit(1);
        }

        System.out.println("main exiting.");
    }
}
