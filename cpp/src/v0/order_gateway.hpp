#pragma once

#include "matching_engine.hpp"
#include "types.hpp"

#include <vector>

namespace ytta::v0 {

class OrderGateway {
 public:
  explicit OrderGateway(MatchingEngine& engine) : engine_(engine) {}

  std::vector<EngineEvent> new_order(const Order& order, std::int64_t ts_ns) {
    return engine_.submit(Op::New, order, ts_ns);
  }

  std::vector<EngineEvent> cancel(std::uint64_t cl_ord_id, std::int64_t ts_ns) {
    Order o;
    o.cl_ord_id = cl_ord_id;
    return engine_.submit(Op::Cancel, o, ts_ns);
  }

 private:
  MatchingEngine& engine_;
};

}  // namespace ytta::v0
