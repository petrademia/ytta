#pragma once
#include "types.hpp"
#include <vector>

namespace ytta::m2 {

class MatchingEngine {
 public:
  std::vector<EngineEvent> new_order(const Order& order, std::uint64_t ts_ns);
  std::vector<EngineEvent> cancel(std::uint64_t cl_ord_id, std::uint64_t ts_ns);
};

}  // namespace ytta::m2
