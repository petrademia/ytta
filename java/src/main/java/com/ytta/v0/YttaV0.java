package com.ytta.v0;

import com.ytta.v0.LatencyProbe.Summary;
import com.ytta.v0.Pipeline.GoldenEvent;
import com.ytta.v0.Pipeline.Mode;
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
        Mode mode = Mode.SYNC;
        for (int i = 0; i < args.length; i++) {
            String arg = args[i];
            if (arg.equals("--fixture")) {
                if (i + 1 >= args.length) {
                    usage();
                }
                fixture = args[++i];
            } else if (arg.equals("--out")) {
                if (i + 1 >= args.length) {
                    usage();
                }
                outPath = args[++i];
            } else if (arg.equals("--mode")) {
                if (i + 1 >= args.length) {
                    usage();
                }
                mode = parseMode(args[++i]);
            } else if (arg.startsWith("--mode=")) {
                mode = parseMode(arg.substring("--mode=".length()));
            } else if (arg.equals("--help") || arg.equals("-h")) {
                usage();
                return;
            } else {
                usage();
            }
        }
        if (fixture == null || outPath == null) {
            usage();
        }

        List<Tick> ticks = TickSource.load(Path.of(fixture));
        LatencyProbe probe = new LatencyProbe();
        List<GoldenEvent> events = Pipeline.run(ticks, probe, mode);

        try (BufferedWriter writer = Files.newBufferedWriter(Path.of(outPath))) {
            for (GoldenEvent e : events) {
                writer.write(e.line());
                writer.newLine();
            }
        }

        Summary s = probe.summarize();
        System.err.printf(
                "{\"type\":\"latency\",\"count\":%d,\"p50_ns\":%d,\"p99_ns\":%d,"
                        + "\"ingest_p50_ns\":%d,\"ingest_p99_ns\":%d,"
                        + "\"decide_p50_ns\":%d,\"decide_p99_ns\":%d,"
                        + "\"execute_p50_ns\":%d,\"execute_p99_ns\":%d,\"drops\":%d}%n",
                s.count(),
                s.p50Ns(),
                s.p99Ns(),
                s.ingestP50Ns(),
                s.ingestP99Ns(),
                s.decideP50Ns(),
                s.decideP99Ns(),
                s.executeP50Ns(),
                s.executeP99Ns(),
                s.drops());

        if (s.drops() != 0) {
            System.err.println("error: queue drops=" + s.drops());
            System.exit(1);
        }
    }

    private static Mode parseMode(String s) {
        return switch (s.toLowerCase()) {
            case "sync" -> Mode.SYNC;
            case "queued" -> Mode.QUEUED;
            default -> {
                System.err.println("error: invalid --mode " + s + " (use sync|queued)");
                System.exit(1);
                yield Mode.SYNC;
            }
        };
    }

    private static void usage() {
        System.err.println(
                "Usage: com.ytta.v0.YttaV0 --fixture <ticks.ndjson> --out <events.ndjson> [--mode=sync|queued]");
        System.exit(1);
    }
}
