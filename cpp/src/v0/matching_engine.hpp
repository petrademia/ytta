#pragma once

#include "book.hpp"
#include "types.hpp"

#include <vector>

namespace ytta::v0 {

class MatchingEngine {
 public:
  std::vector<EngineEvent> submit(Op op, const Order& order, std::int64_t ts_ns);

 private:
  Book book_;
};

}  // namespace ytta::v0
