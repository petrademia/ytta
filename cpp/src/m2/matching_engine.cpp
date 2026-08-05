#include "matching_engine.hpp"

namespace ytta::m2 {

std::vector<EngineEvent> MatchingEngine::new_order(const Order&, std::uint64_t) {
  return {};  // stub: no events
}

std::vector<EngineEvent> MatchingEngine::cancel(std::uint64_t, std::uint64_t) {
  return {};  // stub
}

}  // namespace ytta::m2
