package v0

import "sort"

type LatencyProbe struct {
	e2e     []int64
	ingest  []int64
	decide  []int64
	execute []int64
	drops   uint64
}

func (p *LatencyProbe) AddDrop() { p.drops++ }

func (p *LatencyProbe) SetDrops(n uint64) { p.drops = n }

// RecordNs keeps e2e-only recording for simple callers.
func (p *LatencyProbe) RecordNs(delta int64) {
	p.e2e = append(p.e2e, delta)
}

func (p *LatencyProbe) RecordStages(e2eNs, ingestNs, decideNs, executeNs int64) {
	p.e2e = append(p.e2e, e2eNs)
	p.ingest = append(p.ingest, ingestNs)
	p.decide = append(p.decide, decideNs)
	p.execute = append(p.execute, executeNs)
}

type LatencySummary struct {
	Count         uint64
	P50Ns         int64
	P99Ns         int64
	IngestP50Ns   int64
	IngestP99Ns   int64
	DecideP50Ns   int64
	DecideP99Ns   int64
	ExecuteP50Ns  int64
	ExecuteP99Ns  int64
	Drops         uint64
}

func (p *LatencyProbe) Summarize() LatencySummary {
	s := LatencySummary{Count: uint64(len(p.e2e)), Drops: p.drops}
	s.P50Ns, s.P99Ns = percentiles(p.e2e)
	s.IngestP50Ns, s.IngestP99Ns = percentiles(p.ingest)
	s.DecideP50Ns, s.DecideP99Ns = percentiles(p.decide)
	s.ExecuteP50Ns, s.ExecuteP99Ns = percentiles(p.execute)
	return s
}

func percentiles(samples []int64) (p50, p99 int64) {
	if len(samples) == 0 {
		return 0, 0
	}
	sorted := append([]int64(nil), samples...)
	sort.Slice(sorted, func(i, j int) bool { return sorted[i] < sorted[j] })
	idx := func(pct float64) int {
		if len(sorted) == 1 {
			return 0
		}
		return int(pct * float64(len(sorted)-1))
	}
	return sorted[idx(0.50)], sorted[idx(0.99)]
}
