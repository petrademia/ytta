use std::time::Instant;

use super::engine::MatchingEngine;
use super::gateway::OrderGateway;
use super::probe::LatencyProbe;
use super::strategy::Strategy;
use super::types::{
    ActionKind, EngineEvent, Order, StrategyAction, Tick,
};

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum GoldenType {
    Md = 0,
    Action = 1,
    Ack = 2,
    Fill = 3,
}

#[derive(Debug, Clone)]
pub struct GoldenEvent {
    pub ts_ns: i64,
    pub type_: GoldenType,
    pub cl_ord_id: u64,
    pub line: String,
}

pub fn format_action(a: &StrategyAction) -> String {
    let mut s = format!(
        "{{\"type\":\"action\",\"ts_ns\":{},\"action\":\"",
        a.ts_ns
    );
    match a.kind {
        ActionKind::Noop => s.push_str("NOOP"),
        ActionKind::NewOrder => s.push_str("NEW_ORDER"),
        ActionKind::Cancel => s.push_str("CANCEL"),
    }
    s.push('"');
    match a.kind {
        ActionKind::NewOrder => {
            s.push_str(&format!(
                ",\"cl_ord_id\":{},\"side\":\"{}\",\"price\":{},\"qty\":{}",
                a.cl_ord_id,
                a.side.as_str(),
                a.price,
                a.qty
            ));
        }
        ActionKind::Cancel => {
            s.push_str(&format!(",\"cl_ord_id\":{}", a.cl_ord_id));
        }
        ActionKind::Noop => {}
    }
    s.push_str(&format!(",\"reason\":\"{}\"}}", a.reason));
    s
}

pub fn format_engine_event(e: &EngineEvent) -> (GoldenType, u64, String) {
    match e {
        EngineEvent::Ack(ack) => {
            let mut line = format!(
                "{{\"type\":\"ack\",\"ts_ns\":{},\"cl_ord_id\":{},\"status\":\"{}\"",
                ack.ts_ns,
                ack.cl_ord_id,
                ack.status.as_str()
            );
            if ack.status == super::types::AckStatus::Rejected {
                line.push_str(&format!(",\"reason\":\"{}\"", ack.reason));
            }
            line.push('}');
            (GoldenType::Ack, ack.cl_ord_id, line)
        }
        EngineEvent::Fill(fill) => {
            let line = format!(
                "{{\"type\":\"fill\",\"ts_ns\":{},\"cl_ord_id\":{},\"price\":{},\"qty\":{},\"liquidity\":\"{}\"}}",
                fill.ts_ns,
                fill.cl_ord_id,
                fill.price,
                fill.qty,
                fill.liquidity.as_str()
            );
            (GoldenType::Fill, fill.cl_ord_id, line)
        }
        EngineEvent::Md(md) => {
            let line = format!(
                "{{\"type\":\"md\",\"ts_ns\":{},\"instrument\":\"{}\",\"best_bid\":{},\"best_ask\":{},\"best_bid_qty\":{},\"best_ask_qty\":{}}}",
                md.ts_ns, md.instrument, md.best_bid, md.best_ask, md.best_bid_qty, md.best_ask_qty
            );
            (GoldenType::Md, 0, line)
        }
    }
}

pub fn sort_golden(events: &mut [GoldenEvent]) {
    events.sort_by(|a, b| {
        a.ts_ns
            .cmp(&b.ts_ns)
            .then(a.type_.cmp(&b.type_))
            .then(a.cl_ord_id.cmp(&b.cl_ord_id))
    });
}

pub fn run_pipeline(ticks: &[Tick], probe: &mut LatencyProbe) -> Vec<GoldenEvent> {
    let mut engine = MatchingEngine::new();
    let mut strategy = Strategy::new();
    let mut out = Vec::new();

    for tick in ticks {
        let t0 = Instant::now();
        let action = strategy.on_tick(tick);

        let cl = if action.kind == ActionKind::Noop {
            0
        } else {
            action.cl_ord_id
        };
        out.push(GoldenEvent {
            ts_ns: action.ts_ns,
            type_: GoldenType::Action,
            cl_ord_id: cl,
            line: format_action(&action),
        });

        if action.kind != ActionKind::Noop {
            let mut gw = OrderGateway::new(&mut engine);
            let events = match action.kind {
                ActionKind::NewOrder => gw.new_order(
                    Order {
                        cl_ord_id: action.cl_ord_id,
                        side: action.side,
                        price: action.price,
                        qty: action.qty,
                    },
                    action.ts_ns,
                ),
                ActionKind::Cancel => gw.cancel(action.cl_ord_id, action.ts_ns),
                ActionKind::Noop => unreachable!(),
            };
            for ev in events {
                let (gt, id, line) = format_engine_event(&ev);
                out.push(GoldenEvent {
                    ts_ns: action.ts_ns,
                    type_: gt,
                    cl_ord_id: id,
                    line,
                });
            }
        }

        probe.record_ns(t0.elapsed().as_nanos() as i64);
    }

    sort_golden(&mut out);
    out
}
