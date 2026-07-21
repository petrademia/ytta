use super::book::Book;
use super::types::{
    AckEvent, AckStatus, EngineEvent, FillEvent, Liquidity, Op, Order,
};

pub struct MatchingEngine {
    book: Book,
}

impl MatchingEngine {
    pub fn new() -> Self {
        Self { book: Book::new() }
    }

    pub fn submit(&mut self, op: Op, order: Order, ts_ns: i64) -> Vec<EngineEvent> {
        if op == Op::Cancel {
            if self.book.cancel(order.cl_ord_id).is_none() {
                return vec![EngineEvent::Ack(AckEvent {
                    ts_ns,
                    cl_ord_id: order.cl_ord_id,
                    status: AckStatus::Rejected,
                    reason: "unknown_id".to_string(),
                })];
            }
            return vec![
                EngineEvent::Ack(AckEvent {
                    ts_ns,
                    cl_ord_id: order.cl_ord_id,
                    status: AckStatus::Accepted,
                    reason: String::new(),
                }),
                EngineEvent::Md(self.book.snapshot(ts_ns)),
            ];
        }

        let slices = self.book.match_order(order.side, order.price, order.qty);
        let filled: i64 = slices.iter().map(|s| s.qty).sum();
        let residual = order.qty - filled;

        let mut events = vec![EngineEvent::Ack(AckEvent {
            ts_ns,
            cl_ord_id: order.cl_ord_id,
            status: AckStatus::Accepted,
            reason: String::new(),
        })];

        for s in &slices {
            events.push(EngineEvent::Fill(FillEvent {
                ts_ns,
                cl_ord_id: s.maker_id,
                price: s.price,
                qty: s.qty,
                liquidity: Liquidity::Maker,
            }));
            events.push(EngineEvent::Fill(FillEvent {
                ts_ns,
                cl_ord_id: order.cl_ord_id,
                price: s.price,
                qty: s.qty,
                liquidity: Liquidity::Taker,
            }));
        }

        if residual > 0 {
            let mut rest = order;
            rest.qty = residual;
            self.book.add(rest);
        }

        events.push(EngineEvent::Md(self.book.snapshot(ts_ns)));
        events
    }
}

impl Default for MatchingEngine {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::v0::gateway::OrderGateway;
    use crate::v0::types::Side;

    #[test]
    fn resting_new() {
        let mut eng = MatchingEngine::new();
        let mut gw = OrderGateway::new(&mut eng);
        let ev = gw.new_order(
            Order {
                cl_ord_id: 1,
                side: Side::Buy,
                price: 100,
                qty: 5,
            },
            1000,
        );
        assert_eq!(ev.len(), 2);
        match &ev[0] {
            EngineEvent::Ack(a) => assert_eq!(a.status, AckStatus::Accepted),
            _ => panic!("ack"),
        }
        match &ev[1] {
            EngineEvent::Md(md) => {
                assert_eq!(md.best_bid, 100);
                assert_eq!(md.best_bid_qty, 5);
                assert_eq!(md.best_ask_qty, 0);
            }
            _ => panic!("md"),
        }
    }

    #[test]
    fn cross_fills() {
        let mut eng = MatchingEngine::new();
        {
            let mut gw = OrderGateway::new(&mut eng);
            gw.new_order(
                Order {
                    cl_ord_id: 1,
                    side: Side::Buy,
                    price: 100,
                    qty: 5,
                },
                1000,
            );
        }
        let mut gw = OrderGateway::new(&mut eng);
        let ev = gw.new_order(
            Order {
                cl_ord_id: 2,
                side: Side::Sell,
                price: 100,
                qty: 5,
            },
            2000,
        );
        assert_eq!(ev.len(), 4);
        match &ev[1] {
            EngineEvent::Fill(f) => {
                assert_eq!(f.cl_ord_id, 1);
                assert_eq!(f.liquidity, Liquidity::Maker);
                assert_eq!(f.price, 100);
                assert_eq!(f.qty, 5);
            }
            _ => panic!("maker"),
        }
        match &ev[2] {
            EngineEvent::Fill(f) => {
                assert_eq!(f.cl_ord_id, 2);
                assert_eq!(f.liquidity, Liquidity::Taker);
            }
            _ => panic!("taker"),
        }
        match &ev[3] {
            EngineEvent::Md(md) => {
                assert_eq!(md.best_bid_qty, 0);
                assert_eq!(md.best_ask_qty, 0);
            }
            _ => panic!("md"),
        }
    }

    #[test]
    fn bad_cancel() {
        let mut eng = MatchingEngine::new();
        let mut gw = OrderGateway::new(&mut eng);
        let ev = gw.cancel(99, 1000);
        assert_eq!(ev.len(), 1);
        match &ev[0] {
            EngineEvent::Ack(a) => {
                assert_eq!(a.status, AckStatus::Rejected);
                assert_eq!(a.reason, "unknown_id");
            }
            _ => panic!("ack"),
        }
    }

    #[test]
    fn good_cancel() {
        let mut eng = MatchingEngine::new();
        {
            let mut gw = OrderGateway::new(&mut eng);
            gw.new_order(
                Order {
                    cl_ord_id: 3,
                    side: Side::Buy,
                    price: 99,
                    qty: 1,
                },
                3000,
            );
        }
        let mut gw = OrderGateway::new(&mut eng);
        let ev = gw.cancel(3, 4000);
        assert_eq!(ev.len(), 2);
        match &ev[0] {
            EngineEvent::Ack(a) => assert_eq!(a.status, AckStatus::Accepted),
            _ => panic!("ack"),
        }
        match &ev[1] {
            EngineEvent::Md(md) => assert_eq!(md.best_bid_qty, 0),
            _ => panic!("md"),
        }
    }
}
