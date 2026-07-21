package main

import (
	"flag"
	"fmt"
	"os"

	v0 "ytta/go/internal/v0"
)

func main() {
	fixture := flag.String("fixture", "", "path to ticks.ndjson")
	outPath := flag.String("out", "", "path to write golden event NDJSON")
	flag.Parse()
	if *fixture == "" || *outPath == "" {
		fmt.Fprintf(os.Stderr, "Usage: %s --fixture <ticks.ndjson> --out <events.ndjson>\n", os.Args[0])
		os.Exit(1)
	}

	ticks, err := v0.LoadTicks(*fixture)
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %v\n", err)
		os.Exit(1)
	}

	probe := &v0.LatencyProbe{}
	events := v0.RunPipeline(ticks, probe)

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
	fmt.Fprintf(os.Stderr, `{"type":"latency","count":%d,"p50_ns":%d,"p99_ns":%d}`+"\n",
		s.Count, s.P50Ns, s.P99Ns)
}
