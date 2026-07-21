package com.ytta.v0;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public final class LatencyProbe {
    private final List<Long> e2e = new ArrayList<>();
    private final List<Long> ingest = new ArrayList<>();
    private final List<Long> decide = new ArrayList<>();
    private final List<Long> execute = new ArrayList<>();
    private long drops;

    public void addDrop() {
        drops++;
    }

    public void recordNs(long deltaNs) {
        e2e.add(deltaNs);
    }

    public void recordStages(long e2eNs, long ingestNs, long decideNs, long executeNs) {
        e2e.add(e2eNs);
        ingest.add(ingestNs);
        decide.add(decideNs);
        execute.add(executeNs);
    }

    public record Summary(
            long count,
            long p50Ns,
            long p99Ns,
            long ingestP50Ns,
            long ingestP99Ns,
            long decideP50Ns,
            long decideP99Ns,
            long executeP50Ns,
            long executeP99Ns,
            long drops) {}

    public Summary summarize() {
        long[] e = pct(e2e);
        long[] i = pct(ingest);
        long[] d = pct(decide);
        long[] x = pct(execute);
        return new Summary(e2e.size(), e[0], e[1], i[0], i[1], d[0], d[1], x[0], x[1], drops);
    }

    private static long[] pct(List<Long> samples) {
        if (samples.isEmpty()) {
            return new long[] {0, 0};
        }
        List<Long> sorted = new ArrayList<>(samples);
        Collections.sort(sorted);
        return new long[] {sorted.get(index(sorted, 0.50)), sorted.get(index(sorted, 0.99))};
    }

    private static int index(List<Long> sorted, double pct) {
        if (sorted.size() == 1) {
            return 0;
        }
        return (int) (pct * (sorted.size() - 1));
    }
}
