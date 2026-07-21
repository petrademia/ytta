#include "strategy.hpp"

namespace ytta::v0 {

StrategyAction Strategy::on_tick(const Tick& tick) {
  StrategyAction a;
  a.ts_ns = tick.ts_ns;

  if (tick.seq == 1) {
    a.kind = ActionKind::NewOrder;
    a.cl_ord_id = next_cl_ord_id_++;
    a.side = Side::Buy;
    a.price = 100;
    a.qty = 5;
    a.reason = "resting_bid";
    return a;
  }
  if (tick.seq == 2) {
    a.kind = ActionKind::NewOrder;
    a.cl_ord_id = next_cl_ord_id_++;
    a.side = Side::Sell;
    a.price = 100;
    a.qty = 5;
    a.reason = "cross_self";
    return a;
  }
  if (tick.seq == 3) {
    a.kind = ActionKind::NewOrder;
    a.cl_ord_id = next_cl_ord_id_++;
    a.side = Side::Buy;
    a.price = 99;
    a.qty = 1;
    a.reason = "resting_bid";
    cancel_target_ = a.cl_ord_id;
    return a;
  }
  if (tick.seq == 4) {
    a.kind = ActionKind::Cancel;
    a.cl_ord_id = cancel_target_.value_or(0);
    a.reason = "done";
    return a;
  }

  a.kind = ActionKind::Noop;
  a.reason = "flat";
  return a;
}

}  // namespace ytta::v0
