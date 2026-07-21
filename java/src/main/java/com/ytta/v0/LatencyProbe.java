package com.ytta.v0;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public final class LatencyProbe {
    private final List<Long> samples = new ArrayList<>();

    public void recordNs(long deltaNs) {
        samples.add(deltaNs);
    }

    public record Summary(long count, long p50Ns, long p99Ns) {}

    public Summary summarize() {
        if (samples.isEmpty()) {
            return new Summary(0, 0, 0);
        }
        List<Long> sorted = new ArrayList<>(samples);
        Collections.sort(sorted);
        return new Summary(sorted.size(), sorted.get(index(sorted, 0.50)), sorted.get(index(sorted, 0.99)));
    }

    private static int index(List<Long> sorted, double pct) {
        if (sorted.size() == 1) {
            return 0;
        }
        return (int) (pct * (sorted.size() - 1));
    }
}
