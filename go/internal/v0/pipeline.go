package v0

import (
	"fmt"
	"sort"
	"strings"
	"time"
)

type GoldenType int8

const (
	GoldenMd GoldenType = iota
	GoldenAction
	GoldenAck
	GoldenFill
)

type GoldenEvent struct {
	TsNs    int64
	Type    GoldenType
	ClOrdID uint64
	Line    string
}

func FormatAction(a StrategyAction) string {
	var b strings.Builder
	b.WriteString(`{"type":"action","ts_ns":`)
	b.WriteString(fmt.Sprintf("%d", a.TsNs))
	b.WriteString(`,"action":"`)
	switch a.Kind {
	case ActionNoop:
		b.WriteString("NOOP")
	case ActionNewOrder:
		b.WriteString("NEW_ORDER")
	case ActionCancel:
		b.WriteString("CANCEL")
	}
	b.WriteByte('"')
	switch a.Kind {
	case ActionNewOrder:
		b.WriteString(fmt.Sprintf(`,"cl_ord_id":%d,"side":"%s","price":%d,"qty":%d`,
			a.ClOrdID, a.Side.String(), a.Price, a.Qty))
	case ActionCancel:
		b.WriteString(fmt.Sprintf(`,"cl_ord_id":%d`, a.ClOrdID))
	}
	b.WriteString(fmt.Sprintf(`,"reason":"%s"}`, a.Reason))
	return b.String()
}

func FormatEngineEvent(e EngineEvent) (GoldenType, uint64, string) {
	switch {
	case e.Ack != nil:
		line := fmt.Sprintf(`{"type":"ack","ts_ns":%d,"cl_ord_id":%d,"status":"%s"`,
			e.Ack.TsNs, e.Ack.ClOrdID, e.Ack.Status.String())
		if e.Ack.Status == AckRejected {
			line += fmt.Sprintf(`,"reason":"%s"`, e.Ack.Reason)
		}
		line += "}"
		return GoldenAck, e.Ack.ClOrdID, line
	case e.Fill != nil:
		line := fmt.Sprintf(`{"type":"fill","ts_ns":%d,"cl_ord_id":%d,"price":%d,"qty":%d,"liquidity":"%s"}`,
			e.Fill.TsNs, e.Fill.ClOrdID, e.Fill.Price, e.Fill.Qty, e.Fill.Liquidity.String())
		return GoldenFill, e.Fill.ClOrdID, line
	default:
		md := e.Md
		line := fmt.Sprintf(`{"type":"md","ts_ns":%d,"instrument":"%s","best_bid":%d,"best_ask":%d,"best_bid_qty":%d,"best_ask_qty":%d}`,
			md.TsNs, md.Instrument, md.BestBid, md.BestAsk, md.BestBidQty, md.BestAskQty)
		return GoldenMd, 0, line
	}
}

func SortGolden(events []GoldenEvent) {
	sort.SliceStable(events, func(i, j int) bool {
		a, b := events[i], events[j]
		if a.TsNs != b.TsNs {
			return a.TsNs < b.TsNs
		}
		if a.Type != b.Type {
			return a.Type < b.Type
		}
		return a.ClOrdID < b.ClOrdID
	})
}

func RunPipeline(ticks []Tick, probe *LatencyProbe) []GoldenEvent {
	engine := NewMatchingEngine()
	gateway := NewOrderGateway(engine)
	strategy := NewStrategy()
	var out []GoldenEvent

	for _, tick := range ticks {
		t0 := time.Now()
		action := strategy.OnTick(tick)

		cl := uint64(0)
		if action.Kind != ActionNoop {
			cl = action.ClOrdID
		}
		out = append(out, GoldenEvent{
			TsNs:    action.TsNs,
			Type:    GoldenAction,
			ClOrdID: cl,
			Line:    FormatAction(action),
		})

		if action.Kind != ActionNoop {
			var events []EngineEvent
			if action.Kind == ActionNewOrder {
				events = gateway.NewOrder(Order{
					ClOrdID: action.ClOrdID,
					Side:    action.Side,
					Price:   action.Price,
					Qty:     action.Qty,
				}, action.TsNs)
			} else {
				events = gateway.Cancel(action.ClOrdID, action.TsNs)
			}
			for _, ev := range events {
				gt, id, line := FormatEngineEvent(ev)
				out = append(out, GoldenEvent{
					TsNs:    action.TsNs,
					Type:    gt,
					ClOrdID: id,
					Line:    line,
				})
			}
		}

		probe.RecordNs(time.Since(t0).Nanoseconds())
	}

	SortGolden(out)
	return out
}
