package v0

type bookLevelOrder struct {
	clOrdID uint64
	qty     int64
}

type matchSlice struct {
	makerID uint64
	price   int64
	qty     int64
}

// Book is a single-instrument price-time FIFO book.
type Book struct {
	// bids: price -> FIFO queue; best = highest price
	bids map[int64][]bookLevelOrder
	// asks: price -> FIFO queue; best = lowest price
	asks      map[int64][]bookLevelOrder
	sideByID  map[uint64]Side
}

func NewBook() *Book {
	return &Book{
		bids:     make(map[int64][]bookLevelOrder),
		asks:     make(map[int64][]bookLevelOrder),
		sideByID: make(map[uint64]Side),
	}
}

func (b *Book) Add(order Order) {
	lvl := bookLevelOrder{clOrdID: order.ClOrdID, qty: order.Qty}
	if order.Side == SideBuy {
		b.bids[order.Price] = append(b.bids[order.Price], lvl)
	} else {
		b.asks[order.Price] = append(b.asks[order.Price], lvl)
	}
	b.sideByID[order.ClOrdID] = order.Side
}

func (b *Book) Cancel(clOrdID uint64) (qty int64, ok bool) {
	side, exists := b.sideByID[clOrdID]
	if !exists {
		return 0, false
	}
	levels := b.bids
	if side == SideSell {
		levels = b.asks
	}
	for px, q := range levels {
		for i := range q {
			if q[i].clOrdID == clOrdID {
				qty = q[i].qty
				q = append(q[:i], q[i+1:]...)
				if len(q) == 0 {
					delete(levels, px)
				} else {
					levels[px] = q
				}
				delete(b.sideByID, clOrdID)
				return qty, true
			}
		}
	}
	return 0, false
}

func (b *Book) Match(aggressor Side, price, qty int64) []matchSlice {
	var slices []matchSlice
	remaining := qty

	if aggressor == SideBuy {
		for remaining > 0 {
			best, ok := b.bestAsk()
			if !ok || best > price {
				break
			}
			q := b.asks[best]
			for remaining > 0 && len(q) > 0 {
				head := &q[0]
				fillQty := remaining
				if head.qty < fillQty {
					fillQty = head.qty
				}
				slices = append(slices, matchSlice{makerID: head.clOrdID, price: best, qty: fillQty})
				head.qty -= fillQty
				remaining -= fillQty
				if head.qty == 0 {
					delete(b.sideByID, head.clOrdID)
					q = q[1:]
				}
			}
			if len(q) == 0 {
				delete(b.asks, best)
			} else {
				b.asks[best] = q
			}
		}
		return slices
	}

	for remaining > 0 {
		best, ok := b.bestBid()
		if !ok || best < price {
			break
		}
		q := b.bids[best]
		for remaining > 0 && len(q) > 0 {
			head := &q[0]
			fillQty := remaining
			if head.qty < fillQty {
				fillQty = head.qty
			}
			slices = append(slices, matchSlice{makerID: head.clOrdID, price: best, qty: fillQty})
			head.qty -= fillQty
			remaining -= fillQty
			if head.qty == 0 {
				delete(b.sideByID, head.clOrdID)
				q = q[1:]
			}
		}
		if len(q) == 0 {
			delete(b.bids, best)
		} else {
			b.bids[best] = q
		}
	}
	return slices
}

func (b *Book) Snapshot(tsNs int64) MdEvent {
	md := MdEvent{TsNs: tsNs, Instrument: "INSTR1"}
	if px, ok := b.bestBid(); ok {
		md.BestBid = px
		for _, o := range b.bids[px] {
			md.BestBidQty += o.qty
		}
	}
	if px, ok := b.bestAsk(); ok {
		md.BestAsk = px
		for _, o := range b.asks[px] {
			md.BestAskQty += o.qty
		}
	}
	return md
}

func (b *Book) bestBid() (int64, bool) {
	var best int64
	found := false
	for px := range b.bids {
		if !found || px > best {
			best = px
			found = true
		}
	}
	return best, found
}

func (b *Book) bestAsk() (int64, bool) {
	var best int64
	found := false
	for px := range b.asks {
		if !found || px < best {
			best = px
			found = true
		}
	}
	return best, found
}
