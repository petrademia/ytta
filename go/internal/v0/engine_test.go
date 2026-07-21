package v0

import "testing"

func TestRestingNew(t *testing.T) {
	eng := NewMatchingEngine()
	gw := NewOrderGateway(eng)
	ev := gw.NewOrder(Order{ClOrdID: 1, Side: SideBuy, Price: 100, Qty: 5}, 1000)
	if len(ev) != 2 {
		t.Fatalf("want ack+md, got %d events", len(ev))
	}
	if ev[0].Ack == nil || ev[0].Ack.Status != AckAccepted {
		t.Fatalf("expected accepted ack")
	}
	if ev[1].Md == nil || ev[1].Md.BestBid != 100 || ev[1].Md.BestBidQty != 5 || ev[1].Md.BestAskQty != 0 {
		t.Fatalf("unexpected md: %+v", ev[1].Md)
	}
}

func TestCrossFills(t *testing.T) {
	eng := NewMatchingEngine()
	gw := NewOrderGateway(eng)
	gw.NewOrder(Order{ClOrdID: 1, Side: SideBuy, Price: 100, Qty: 5}, 1000)
	ev := gw.NewOrder(Order{ClOrdID: 2, Side: SideSell, Price: 100, Qty: 5}, 2000)
	if len(ev) != 4 {
		t.Fatalf("want ack+2 fills+md, got %d", len(ev))
	}
	if ev[0].Ack == nil || ev[0].Ack.ClOrdID != 2 {
		t.Fatalf("ack aggressor")
	}
	if ev[1].Fill == nil || ev[1].Fill.ClOrdID != 1 || ev[1].Fill.Liquidity != LiquidityMaker {
		t.Fatalf("maker fill: %+v", ev[1].Fill)
	}
	if ev[2].Fill == nil || ev[2].Fill.ClOrdID != 2 || ev[2].Fill.Liquidity != LiquidityTaker {
		t.Fatalf("taker fill: %+v", ev[2].Fill)
	}
	if ev[1].Fill.Price != 100 || ev[1].Fill.Qty != 5 {
		t.Fatalf("fill px/qty")
	}
	if ev[3].Md == nil || ev[3].Md.BestBidQty != 0 || ev[3].Md.BestAskQty != 0 {
		t.Fatalf("book should be empty")
	}
}

func TestBadCancel(t *testing.T) {
	eng := NewMatchingEngine()
	gw := NewOrderGateway(eng)
	ev := gw.Cancel(99, 1000)
	if len(ev) != 1 || ev[0].Ack == nil || ev[0].Ack.Status != AckRejected || ev[0].Ack.Reason != "unknown_id" {
		t.Fatalf("bad cancel: %+v", ev)
	}
}

func TestGoodCancel(t *testing.T) {
	eng := NewMatchingEngine()
	gw := NewOrderGateway(eng)
	gw.NewOrder(Order{ClOrdID: 3, Side: SideBuy, Price: 99, Qty: 1}, 3000)
	ev := gw.Cancel(3, 4000)
	if len(ev) != 2 {
		t.Fatalf("want ack+md, got %d", len(ev))
	}
	if ev[0].Ack == nil || ev[0].Ack.Status != AckAccepted {
		t.Fatalf("cancel accepted")
	}
	if ev[1].Md == nil || ev[1].Md.BestBidQty != 0 {
		t.Fatalf("bid cleared")
	}
}
