use std::collections::{BTreeMap, HashMap, VecDeque};

use super::types::{MdEvent, Order, Side};

#[derive(Clone)]
struct BookLevelOrder {
    cl_ord_id: u64,
    qty: i64,
}

pub struct MatchSlice {
    pub maker_id: u64,
    pub price: i64,
    pub qty: i64,
}

pub struct Book {
    bids: BTreeMap<i64, VecDeque<BookLevelOrder>>,
    asks: BTreeMap<i64, VecDeque<BookLevelOrder>>,
    side_by_id: HashMap<u64, Side>,
}

impl Book {
    pub fn new() -> Self {
        Self {
            bids: BTreeMap::new(),
            asks: BTreeMap::new(),
            side_by_id: HashMap::new(),
        }
    }

    pub fn add(&mut self, order: Order) {
        let level = BookLevelOrder {
            cl_ord_id: order.cl_ord_id,
            qty: order.qty,
        };
        match order.side {
            Side::Buy => self.bids.entry(order.price).or_default().push_back(level),
            Side::Sell => self.asks.entry(order.price).or_default().push_back(level),
        }
        self.side_by_id.insert(order.cl_ord_id, order.side);
    }

    pub fn cancel(&mut self, cl_ord_id: u64) -> Option<i64> {
        let side = *self.side_by_id.get(&cl_ord_id)?;
        let levels = match side {
            Side::Buy => &mut self.bids,
            Side::Sell => &mut self.asks,
        };
        let mut found_px = None;
        let mut found_qty = None;
        for (px, q) in levels.iter_mut() {
            if let Some(pos) = q.iter().position(|o| o.cl_ord_id == cl_ord_id) {
                let removed = q.remove(pos).unwrap();
                found_qty = Some(removed.qty);
                found_px = Some(*px);
                break;
            }
        }
        let px = found_px?;
        let qty = found_qty?;
        if levels.get(&px).map(|q| q.is_empty()).unwrap_or(true) {
            levels.remove(&px);
        }
        self.side_by_id.remove(&cl_ord_id);
        Some(qty)
    }

    pub fn match_order(&mut self, aggressor: Side, price: i64, qty: i64) -> Vec<MatchSlice> {
        let mut slices = Vec::new();
        let mut remaining = qty;

        match aggressor {
            Side::Buy => {
                while remaining > 0 {
                    let best = match self.asks.keys().next().copied() {
                        Some(px) if px <= price => px,
                        _ => break,
                    };
                    Self::match_level(
                        &mut self.asks,
                        &mut self.side_by_id,
                        best,
                        &mut remaining,
                        &mut slices,
                    );
                }
            }
            Side::Sell => {
                while remaining > 0 {
                    let best = match self.bids.keys().next_back().copied() {
                        Some(px) if px >= price => px,
                        _ => break,
                    };
                    Self::match_level(
                        &mut self.bids,
                        &mut self.side_by_id,
                        best,
                        &mut remaining,
                        &mut slices,
                    );
                }
            }
        }
        slices
    }

    fn match_level(
        levels: &mut BTreeMap<i64, VecDeque<BookLevelOrder>>,
        side_by_id: &mut HashMap<u64, Side>,
        px: i64,
        remaining: &mut i64,
        slices: &mut Vec<MatchSlice>,
    ) {
        let q = levels.get_mut(&px).unwrap();
        while *remaining > 0 && !q.is_empty() {
            let head = q.front_mut().unwrap();
            let fill_qty = (*remaining).min(head.qty);
            slices.push(MatchSlice {
                maker_id: head.cl_ord_id,
                price: px,
                qty: fill_qty,
            });
            head.qty -= fill_qty;
            *remaining -= fill_qty;
            if head.qty == 0 {
                let done = q.pop_front().unwrap();
                side_by_id.remove(&done.cl_ord_id);
            }
        }
        if q.is_empty() {
            levels.remove(&px);
        }
    }

    pub fn snapshot(&self, ts_ns: i64) -> MdEvent {
        let mut md = MdEvent {
            ts_ns,
            instrument: "INSTR1".to_string(),
            best_bid: 0,
            best_ask: 0,
            best_bid_qty: 0,
            best_ask_qty: 0,
        };
        if let Some((&px, q)) = self.bids.iter().next_back() {
            md.best_bid = px;
            md.best_bid_qty = q.iter().map(|o| o.qty).sum();
        }
        if let Some((&px, q)) = self.asks.iter().next() {
            md.best_ask = px;
            md.best_ask_qty = q.iter().map(|o| o.qty).sum();
        }
        md
    }
}

impl Default for Book {
    fn default() -> Self {
        Self::new()
    }
}
