use super::types::{ActionKind, Side, StrategyAction, Tick};

pub struct Strategy {
    next_cl_ord_id: u64,
    cancel_target: Option<u64>,
}

impl Strategy {
    pub fn new() -> Self {
        Self {
            next_cl_ord_id: 1,
            cancel_target: None,
        }
    }

    pub fn on_tick(&mut self, tick: &Tick) -> StrategyAction {
        let mut a = StrategyAction {
            kind: ActionKind::Noop,
            ts_ns: tick.ts_ns,
            cl_ord_id: 0,
            side: Side::Buy,
            price: 0,
            qty: 0,
            reason: String::new(),
        };
        match tick.seq {
            1 => {
                a.kind = ActionKind::NewOrder;
                a.cl_ord_id = self.next_cl_ord_id;
                self.next_cl_ord_id += 1;
                a.side = Side::Buy;
                a.price = 100;
                a.qty = 5;
                a.reason = "resting_bid".to_string();
            }
            2 => {
                a.kind = ActionKind::NewOrder;
                a.cl_ord_id = self.next_cl_ord_id;
                self.next_cl_ord_id += 1;
                a.side = Side::Sell;
                a.price = 100;
                a.qty = 5;
                a.reason = "cross_self".to_string();
            }
            3 => {
                a.kind = ActionKind::NewOrder;
                a.cl_ord_id = self.next_cl_ord_id;
                self.next_cl_ord_id += 1;
                a.side = Side::Buy;
                a.price = 99;
                a.qty = 1;
                a.reason = "resting_bid".to_string();
                self.cancel_target = Some(a.cl_ord_id);
            }
            4 => {
                a.kind = ActionKind::Cancel;
                a.cl_ord_id = self.cancel_target.unwrap_or(0);
                a.reason = "done".to_string();
            }
            _ => {
                a.kind = ActionKind::Noop;
                a.reason = "flat".to_string();
            }
        }
        a
    }
}

impl Default for Strategy {
    fn default() -> Self {
        Self::new()
    }
}
