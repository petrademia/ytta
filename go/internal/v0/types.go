package v0

type Side int8

const (
	SideBuy Side = iota
	SideSell
)

func (s Side) String() string {
	if s == SideBuy {
		return "BUY"
	}
	return "SELL"
}

type Op int8

const (
	OpNew Op = iota
	OpCancel
)

type AckStatus int8

const (
	AckAccepted AckStatus = iota
	AckRejected
)

func (s AckStatus) String() string {
	if s == AckAccepted {
		return "ACCEPTED"
	}
	return "REJECTED"
}

type Liquidity int8

const (
	LiquidityMaker Liquidity = iota
	LiquidityTaker
)

func (l Liquidity) String() string {
	if l == LiquidityMaker {
		return "MAKER"
	}
	return "TAKER"
}

type Order struct {
	ClOrdID uint64
	Side    Side
	Price   int64
	Qty     int64
}

type AckEvent struct {
	TsNs    int64
	ClOrdID uint64
	Status  AckStatus
	Reason  string
}

type FillEvent struct {
	TsNs      int64
	ClOrdID   uint64
	Price     int64
	Qty       int64
	Liquidity Liquidity
}

type MdEvent struct {
	TsNs        int64
	Instrument  string
	BestBid     int64
	BestAsk     int64
	BestBidQty  int64
	BestAskQty  int64
}

// EngineEvent is one of Ack, Fill, or Md.
type EngineEvent struct {
	Ack  *AckEvent
	Fill *FillEvent
	Md   *MdEvent
}

type Tick struct {
	TsNs       int64
	Instrument string
	Side       Side
	Price      int64
	Qty        int64
	Seq        uint64
}

type ActionKind int8

const (
	ActionNoop ActionKind = iota
	ActionNewOrder
	ActionCancel
)

type StrategyAction struct {
	Kind    ActionKind
	TsNs    int64
	ClOrdID uint64
	Side    Side
	Price   int64
	Qty     int64
	Reason  string
}
