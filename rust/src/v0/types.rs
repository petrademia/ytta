#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Side {
    Buy,
    Sell,
}

impl Side {
    pub fn as_str(self) -> &'static str {
        match self {
            Side::Buy => "BUY",
            Side::Sell => "SELL",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Op {
    New,
    Cancel,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AckStatus {
    Accepted,
    Rejected,
}

impl AckStatus {
    pub fn as_str(self) -> &'static str {
        match self {
            AckStatus::Accepted => "ACCEPTED",
            AckStatus::Rejected => "REJECTED",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Liquidity {
    Maker,
    Taker,
}

impl Liquidity {
    pub fn as_str(self) -> &'static str {
        match self {
            Liquidity::Maker => "MAKER",
            Liquidity::Taker => "TAKER",
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub struct Order {
    pub cl_ord_id: u64,
    pub side: Side,
    pub price: i64,
    pub qty: i64,
}

#[derive(Debug, Clone)]
pub struct AckEvent {
    pub ts_ns: i64,
    pub cl_ord_id: u64,
    pub status: AckStatus,
    pub reason: String,
}

#[derive(Debug, Clone)]
pub struct FillEvent {
    pub ts_ns: i64,
    pub cl_ord_id: u64,
    pub price: i64,
    pub qty: i64,
    pub liquidity: Liquidity,
}

#[derive(Debug, Clone)]
pub struct MdEvent {
    pub ts_ns: i64,
    pub instrument: String,
    pub best_bid: i64,
    pub best_ask: i64,
    pub best_bid_qty: i64,
    pub best_ask_qty: i64,
}

#[derive(Debug, Clone)]
pub enum EngineEvent {
    Ack(AckEvent),
    Fill(FillEvent),
    Md(MdEvent),
}

#[derive(Debug, Clone)]
pub struct Tick {
    pub ts_ns: i64,
    pub instrument: String,
    pub side: Side,
    pub price: i64,
    pub qty: i64,
    pub seq: u64,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ActionKind {
    Noop,
    NewOrder,
    Cancel,
}

#[derive(Debug, Clone)]
pub struct StrategyAction {
    pub kind: ActionKind,
    pub ts_ns: i64,
    pub cl_ord_id: u64,
    pub side: Side,
    pub price: i64,
    pub qty: i64,
    pub reason: String,
}
