package v0_test

import (
	"bytes"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"

	v0 "ytta/go/internal/v0"
)

func repoRoot(t *testing.T) string {
	t.Helper()
	_, file, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("runtime.Caller failed")
	}
	return filepath.Clean(filepath.Join(filepath.Dir(file), "..", "..", ".."))
}

func TestGoldenParitySyncAndQueued(t *testing.T) {
	root := repoRoot(t)
	fixture := filepath.Join(root, "shared", "fixtures", "v0", "ticks.ndjson")
	goldenPath := filepath.Join(root, "shared", "fixtures", "v0", "golden.ndjson")

	ticks, err := v0.LoadTicks(fixture)
	if err != nil {
		t.Fatal(err)
	}
	want, err := os.ReadFile(goldenPath)
	if err != nil {
		t.Fatal(err)
	}

	for _, mode := range []v0.PipelineMode{v0.ModeSync, v0.ModeQueued} {
		t.Run(modeName(mode), func(t *testing.T) {
			probe := &v0.LatencyProbe{}
			events := v0.RunPipelineMode(ticks, probe, mode)
			var got bytes.Buffer
			for _, e := range events {
				got.WriteString(e.Line)
				got.WriteByte('\n')
			}
			if !bytes.Equal(got.Bytes(), want) {
				t.Fatalf("golden drift\n--- want ---\n%s\n--- got ---\n%s", want, got.Bytes())
			}
			if probe.Summarize().Drops != 0 {
				t.Fatalf("drops=%d", probe.Summarize().Drops)
			}
		})
	}
}

func TestBurstQueuedNoDrops(t *testing.T) {
	root := repoRoot(t)
	fixture := filepath.Join(root, "shared", "fixtures", "v1", "ticks_burst.ndjson")
	ticks, err := v0.LoadTicks(fixture)
	if err != nil {
		t.Fatal(err)
	}
	probe := &v0.LatencyProbe{}
	events := v0.RunPipelineMode(ticks, probe, v0.ModeQueued)
	s := probe.Summarize()
	if s.Drops != 0 {
		t.Fatalf("drops=%d", s.Drops)
	}
	actions := 0
	for _, e := range events {
		if strings.Contains(e.Line, `"type":"action"`) {
			actions++
		}
	}
	if actions != len(ticks) {
		t.Fatalf("actions=%d ticks=%d", actions, len(ticks))
	}
	if s.Count != uint64(len(ticks)) {
		t.Fatalf("count=%d ticks=%d", s.Count, len(ticks))
	}
}

func modeName(m v0.PipelineMode) string {
	if m == v0.ModeQueued {
		return "queued"
	}
	return "sync"
}
