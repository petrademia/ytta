#pragma once
#include "matching_engine.hpp"

namespace ytta::m2 {

class OrderGateway {
 public:
  explicit OrderGateway(MatchingEngine& engine) : engine_(engine) {}

  std::vector<EngineEvent> new_order(const Order& order, std::uint64_t ts_ns) {
    return engine_.new_order(order, ts_ns);
  }

  std::vector<EngineEvent> cancel(std::uint64_t cl_ord_id, std::uint64_t ts_ns) {
    return engine_.cancel(cl_ord_id, ts_ns);
  }

 private:
  MatchingEngine& engine_;
};

}  // namespace ytta::m2
