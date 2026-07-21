package v0_test

import (
	"bytes"
	"os"
	"path/filepath"
	"runtime"
	"testing"

	v0 "ytta/go/internal/v0"
)

func repoRoot(t *testing.T) string {
	t.Helper()
	_, file, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("runtime.Caller failed")
	}
	// go/internal/v0 -> repo root
	return filepath.Clean(filepath.Join(filepath.Dir(file), "..", "..", ".."))
}

func TestGoldenParity(t *testing.T) {
	root := repoRoot(t)
	fixture := filepath.Join(root, "shared", "fixtures", "v0", "ticks.ndjson")
	goldenPath := filepath.Join(root, "shared", "fixtures", "v0", "golden.ndjson")

	ticks, err := v0.LoadTicks(fixture)
	if err != nil {
		t.Fatal(err)
	}
	probe := &v0.LatencyProbe{}
	events := v0.RunPipeline(ticks, probe)

	var got bytes.Buffer
	for _, e := range events {
		got.WriteString(e.Line)
		got.WriteByte('\n')
	}

	want, err := os.ReadFile(goldenPath)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(got.Bytes(), want) {
		t.Fatalf("golden drift\n--- want ---\n%s\n--- got ---\n%s", want, got.Bytes())
	}
}
