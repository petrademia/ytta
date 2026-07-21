package v0

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
)

func LoadTicks(path string) ([]Tick, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("open fixture: %w", err)
	}
	defer f.Close()

	var ticks []Tick
	sc := bufio.NewScanner(f)
	lineNo := 0
	for sc.Scan() {
		lineNo++
		line := sc.Bytes()
		if len(line) == 0 {
			continue
		}
		var raw struct {
			Type       string `json:"type"`
			TsNs       int64  `json:"ts_ns"`
			Instrument string `json:"instrument"`
			Side       string `json:"side"`
			Price      int64  `json:"price"`
			Qty        int64  `json:"qty"`
			Seq        uint64 `json:"seq"`
		}
		if err := json.Unmarshal(line, &raw); err != nil {
			return nil, fmt.Errorf("tick line %d: %w", lineNo, err)
		}
		if raw.Type != "tick" {
			continue
		}
		var side Side
		switch raw.Side {
		case "BUY":
			side = SideBuy
		case "SELL":
			side = SideSell
		default:
			return nil, fmt.Errorf("tick line %d: invalid side %q", lineNo, raw.Side)
		}
		ticks = append(ticks, Tick{
			TsNs:       raw.TsNs,
			Instrument: raw.Instrument,
			Side:       side,
			Price:      raw.Price,
			Qty:        raw.Qty,
			Seq:        raw.Seq,
		})
	}
	if err := sc.Err(); err != nil {
		return nil, err
	}
	return ticks, nil
}
