use super::engine::MatchingEngine;
use super::types::{EngineEvent, Op, Order};

pub struct OrderGateway<'a> {
    engine: &'a mut MatchingEngine,
}

impl<'a> OrderGateway<'a> {
    pub fn new(engine: &'a mut MatchingEngine) -> Self {
        Self { engine }
    }

    pub fn new_order(&mut self, order: Order, ts_ns: i64) -> Vec<EngineEvent> {
        self.engine.submit(Op::New, order, ts_ns)
    }

    pub fn cancel(&mut self, cl_ord_id: u64, ts_ns: i64) -> Vec<EngineEvent> {
        self.engine.submit(
            Op::Cancel,
            Order {
                cl_ord_id,
                side: super::types::Side::Buy,
                price: 0,
                qty: 0,
            },
            ts_ns,
        )
    }
}
