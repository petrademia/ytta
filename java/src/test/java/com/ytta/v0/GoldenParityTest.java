package com.ytta.v0;

import com.ytta.v0.Pipeline.GoldenEvent;
import com.ytta.v0.Pipeline.Mode;
import com.ytta.v0.Types.Tick;

import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class GoldenParityTest {
    @Test
    void fixtureMatchesGoldenSyncAndQueued() throws IOException {
        Path root = findRepoRoot();
        Path fixture = root.resolve("shared/fixtures/v0/ticks.ndjson");
        Path golden = root.resolve("shared/fixtures/v0/golden.ndjson");
        assertTrue(Files.isRegularFile(fixture), () -> "missing " + fixture);
        assertTrue(Files.isRegularFile(golden), () -> "missing " + golden);

        List<Tick> ticks = TickSource.load(fixture);
        String want = Files.readString(golden);

        for (Mode mode : List.of(Mode.SYNC, Mode.QUEUED)) {
            LatencyProbe probe = new LatencyProbe();
            List<GoldenEvent> events = Pipeline.run(ticks, probe, mode);
            StringBuilder got = new StringBuilder();
            for (GoldenEvent e : events) {
                got.append(e.line()).append('\n');
            }
            assertEquals(want, got.toString(), "golden drift mode=" + mode);
            assertEquals(0, probe.summarize().drops());
        }
    }

    @Test
    void burstQueuedNoDrops() throws IOException {
        Path root = findRepoRoot();
        Path fixture = root.resolve("shared/fixtures/v1/ticks_burst.ndjson");
        assertTrue(Files.isRegularFile(fixture), () -> "missing " + fixture);
        List<Tick> ticks = TickSource.load(fixture);
        LatencyProbe probe = new LatencyProbe();
        List<GoldenEvent> events = Pipeline.run(ticks, probe, Mode.QUEUED);
        var s = probe.summarize();
        assertEquals(0, s.drops());
        assertEquals(ticks.size(), s.count());
        long actions = events.stream().filter(e -> e.line().contains("\"type\":\"action\"")).count();
        assertEquals(ticks.size(), actions);
    }

    /** Resolve repo root when Surefire cwd is `java/` or the repo root. */
    static Path findRepoRoot() {
        Path cwd = Path.of("").toAbsolutePath().normalize();
        Path[] candidates = {
            cwd,
            cwd.getParent(),
            cwd.resolve("..").normalize(),
        };
        for (Path c : candidates) {
            if (c != null && Files.isRegularFile(c.resolve("shared/fixtures/v0/golden.ndjson"))) {
                return c;
            }
        }
        throw new IllegalStateException("cannot find repo root from " + cwd);
    }
}
