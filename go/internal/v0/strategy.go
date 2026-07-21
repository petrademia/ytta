package v0

type Strategy struct {
	nextClOrdID  uint64
	cancelTarget uint64
	hasCancel    bool
}

func NewStrategy() *Strategy {
	return &Strategy{nextClOrdID: 1}
}

func (s *Strategy) OnTick(tick Tick) StrategyAction {
	a := StrategyAction{TsNs: tick.TsNs}
	switch tick.Seq {
	case 1:
		a.Kind = ActionNewOrder
		a.ClOrdID = s.nextClOrdID
		s.nextClOrdID++
		a.Side = SideBuy
		a.Price = 100
		a.Qty = 5
		a.Reason = "resting_bid"
		return a
	case 2:
		a.Kind = ActionNewOrder
		a.ClOrdID = s.nextClOrdID
		s.nextClOrdID++
		a.Side = SideSell
		a.Price = 100
		a.Qty = 5
		a.Reason = "cross_self"
		return a
	case 3:
		a.Kind = ActionNewOrder
		a.ClOrdID = s.nextClOrdID
		s.nextClOrdID++
		a.Side = SideBuy
		a.Price = 99
		a.Qty = 1
		a.Reason = "resting_bid"
		s.cancelTarget = a.ClOrdID
		s.hasCancel = true
		return a
	case 4:
		a.Kind = ActionCancel
		if s.hasCancel {
			a.ClOrdID = s.cancelTarget
		}
		a.Reason = "done"
		return a
	default:
		a.Kind = ActionNoop
		a.Reason = "flat"
		return a
	}
}
