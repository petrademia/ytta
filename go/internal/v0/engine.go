package v0

type MatchingEngine struct {
	book *Book
}

func NewMatchingEngine() *MatchingEngine {
	return &MatchingEngine{book: NewBook()}
}

func (e *MatchingEngine) Submit(op Op, order Order, tsNs int64) []EngineEvent {
	if op == OpCancel {
		if _, ok := e.book.Cancel(order.ClOrdID); !ok {
			return []EngineEvent{{
				Ack: &AckEvent{TsNs: tsNs, ClOrdID: order.ClOrdID, Status: AckRejected, Reason: "unknown_id"},
			}}
		}
		md := e.book.Snapshot(tsNs)
		return []EngineEvent{
			{Ack: &AckEvent{TsNs: tsNs, ClOrdID: order.ClOrdID, Status: AckAccepted}},
			{Md: &md},
		}
	}

	slices := e.book.Match(order.Side, order.Price, order.Qty)
	var filled int64
	for _, s := range slices {
		filled += s.qty
	}
	residual := order.Qty - filled

	events := []EngineEvent{
		{Ack: &AckEvent{TsNs: tsNs, ClOrdID: order.ClOrdID, Status: AckAccepted}},
	}
	for _, s := range slices {
		events = append(events,
			EngineEvent{Fill: &FillEvent{TsNs: tsNs, ClOrdID: s.makerID, Price: s.price, Qty: s.qty, Liquidity: LiquidityMaker}},
			EngineEvent{Fill: &FillEvent{TsNs: tsNs, ClOrdID: order.ClOrdID, Price: s.price, Qty: s.qty, Liquidity: LiquidityTaker}},
		)
	}
	if residual > 0 {
		rest := order
		rest.Qty = residual
		e.book.Add(rest)
	}
	md := e.book.Snapshot(tsNs)
	events = append(events, EngineEvent{Md: &md})
	return events
}
