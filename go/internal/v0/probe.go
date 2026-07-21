package v0

import "sort"

type LatencyProbe struct {
	samples []int64
}

func (p *LatencyProbe) RecordNs(delta int64) {
	p.samples = append(p.samples, delta)
}

type LatencySummary struct {
	Count uint64
	P50Ns int64
	P99Ns int64
}

func (p *LatencyProbe) Summarize() LatencySummary {
	s := LatencySummary{Count: uint64(len(p.samples))}
	if len(p.samples) == 0 {
		return s
	}
	sorted := append([]int64(nil), p.samples...)
	sort.Slice(sorted, func(i, j int) bool { return sorted[i] < sorted[j] })
	idx := func(pct float64) int {
		if len(sorted) == 1 {
			return 0
		}
		return int(pct * float64(len(sorted)-1))
	}
	s.P50Ns = sorted[idx(0.50)]
	s.P99Ns = sorted[idx(0.99)]
	return s
}
