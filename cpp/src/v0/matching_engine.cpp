#include "matching_engine.hpp"

namespace ytta::v0 {

std::vector<EngineEvent> MatchingEngine::submit(Op op, const Order& order,
                                                std::int64_t ts_ns) {
  std::vector<EngineEvent> events;

  if (op == Op::Cancel) {
    const auto cancelled = book_.cancel(order.cl_ord_id);
    if (!cancelled) {
      events.emplace_back(AckEvent{ts_ns, order.cl_ord_id, AckStatus::Rejected,
                                   "unknown_id"});
      return events;
    }
    events.emplace_back(
        AckEvent{ts_ns, order.cl_ord_id, AckStatus::Accepted, {}});
    events.emplace_back(book_.snapshot(ts_ns));
    return events;
  }

  // NEW
  auto slices = book_.match(order.side, order.price, order.qty);
  std::int64_t filled = 0;
  for (const auto& s : slices) {
    filled += s.qty;
  }
  const std::int64_t residual = order.qty - filled;

  events.emplace_back(
      AckEvent{ts_ns, order.cl_ord_id, AckStatus::Accepted, {}});

  for (const auto& s : slices) {
    events.emplace_back(FillEvent{ts_ns, s.maker_id, s.price, s.qty,
                                  Liquidity::Maker});
    events.emplace_back(FillEvent{ts_ns, order.cl_ord_id, s.price, s.qty,
                                  Liquidity::Taker});
  }

  if (residual > 0) {
    Order rest = order;
    rest.qty = residual;
    book_.add(rest);
  }

  events.emplace_back(book_.snapshot(ts_ns));
  return events;
}

}  // namespace ytta::v0
