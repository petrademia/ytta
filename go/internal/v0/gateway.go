package v0

type OrderGateway struct {
	engine *MatchingEngine
}

func NewOrderGateway(engine *MatchingEngine) *OrderGateway {
	return &OrderGateway{engine: engine}
}

func (g *OrderGateway) NewOrder(order Order, tsNs int64) []EngineEvent {
	return g.engine.Submit(OpNew, order, tsNs)
}

func (g *OrderGateway) Cancel(clOrdID uint64, tsNs int64) []EngineEvent {
	return g.engine.Submit(OpCancel, Order{ClOrdID: clOrdID}, tsNs)
}
