package main

import (
	"flag"
	"fmt"
	"os"
	"strings"

	v0 "ytta/go/internal/v0"
)

func main() {
	fixture := flag.String("fixture", "", "path to ticks.ndjson")
	outPath := flag.String("out", "", "path to write golden event NDJSON")
	modeStr := flag.String("mode", "sync", "pipeline mode: sync|queued")
	flag.Parse()
	if *fixture == "" || *outPath == "" {
		fmt.Fprintf(os.Stderr, "Usage: %s --fixture <ticks.ndjson> --out <events.ndjson> [--mode=sync|queued]\n", os.Args[0])
		os.Exit(1)
	}

	mode, err := parseMode(*modeStr)
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %v\n", err)
		os.Exit(1)
	}

	ticks, err := v0.LoadTicks(*fixture)
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %v\n", err)
		os.Exit(1)
	}

	probe := &v0.LatencyProbe{}
	events := v0.RunPipelineMode(ticks, probe, mode)

	f, err := os.Create(*outPath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %v\n", err)
		os.Exit(1)
	}
	defer f.Close()
	for _, e := range events {
		if _, err := fmt.Fprintln(f, e.Line); err != nil {
			fmt.Fprintf(os.Stderr, "error: %v\n", err)
			os.Exit(1)
		}
	}

	s := probe.Summarize()
	fmt.Fprintf(os.Stderr,
		`{"type":"latency","count":%d,"p50_ns":%d,"p99_ns":%d,"ingest_p50_ns":%d,"ingest_p99_ns":%d,"decide_p50_ns":%d,"decide_p99_ns":%d,"execute_p50_ns":%d,"execute_p99_ns":%d,"drops":%d}`+"\n",
		s.Count, s.P50Ns, s.P99Ns,
		s.IngestP50Ns, s.IngestP99Ns,
		s.DecideP50Ns, s.DecideP99Ns,
		s.ExecuteP50Ns, s.ExecuteP99Ns,
		s.Drops,
	)

	if s.Drops != 0 {
		fmt.Fprintf(os.Stderr, "error: queue drops=%d\n", s.Drops)
		os.Exit(1)
	}
}

func parseMode(s string) (v0.PipelineMode, error) {
	switch strings.ToLower(strings.TrimSpace(s)) {
	case "sync":
		return v0.ModeSync, nil
	case "queued":
		return v0.ModeQueued, nil
	default:
		return 0, fmt.Errorf("invalid --mode %q (use sync|queued)", s)
	}
}
