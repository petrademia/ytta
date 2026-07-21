#pragma once

#include "latency_probe.hpp"
#include "order_gateway.hpp"
#include "strategy.hpp"
#include "tick_source.hpp"
#include "types.hpp"

#include <string>
#include <vector>

namespace ytta::v0 {

enum class GoldenType : std::uint8_t { Md, Action, Ack, Fill };

struct GoldenEvent {
  std::int64_t ts_ns{0};
  GoldenType type{GoldenType::Action};
  std::uint64_t cl_ord_id{0};
  std::string line;
};

class Pipeline {
 public:
  std::vector<GoldenEvent> run(const std::vector<Tick>& ticks,
                               LatencyProbe& probe);
};

std::string format_action(const StrategyAction& a);
std::string format_engine_event(const EngineEvent& e);
void sort_golden(std::vector<GoldenEvent>& events);

}  // namespace ytta::v0
