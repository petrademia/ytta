package com.ytta.v0;

import com.ytta.v0.LatencyProbe.Summary;
import com.ytta.v0.Pipeline.GoldenEvent;
import com.ytta.v0.Types.Tick;

import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

public final class YttaV0 {
    public static void main(String[] args) throws IOException {
        String fixture = null;
        String outPath = null;
        for (int i = 0; i < args.length; i++) {
            switch (args[i]) {
                case "--fixture" -> {
                    if (i + 1 >= args.length) {
                        usage();
                    }
                    fixture = args[++i];
                }
                case "--out" -> {
                    if (i + 1 >= args.length) {
                        usage();
                    }
                    outPath = args[++i];
                }
                case "--help", "-h" -> {
                    usage();
                    return;
                }
                default -> usage();
            }
        }
        if (fixture == null || outPath == null) {
            usage();
        }

        List<Tick> ticks = TickSource.load(Path.of(fixture));
        LatencyProbe probe = new LatencyProbe();
        List<GoldenEvent> events = Pipeline.run(ticks, probe);

        try (BufferedWriter writer = Files.newBufferedWriter(Path.of(outPath))) {
            for (GoldenEvent e : events) {
                writer.write(e.line());
                writer.newLine();
            }
        }

        Summary s = probe.summarize();
        System.err.printf(
                "{\"type\":\"latency\",\"count\":%d,\"p50_ns\":%d,\"p99_ns\":%d}%n",
                s.count(), s.p50Ns(), s.p99Ns());
    }

    private static void usage() {
        System.err.println("Usage: com.ytta.v0.YttaV0 --fixture <ticks.ndjson> --out <events.ndjson>");
        System.exit(1);
    }
}
