#pragma once

#include "tick_source.hpp"
#include "types.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace ytta::v0 {

enum class ActionKind : std::uint8_t { Noop, NewOrder, Cancel };

struct StrategyAction {
  ActionKind kind{ActionKind::Noop};
  std::int64_t ts_ns{0};
  std::uint64_t cl_ord_id{0};
  Side side{Side::Buy};
  std::int64_t price{0};
  std::int64_t qty{0};
  std::string reason;
};

class Strategy {
 public:
  StrategyAction on_tick(const Tick& tick);

 private:
  std::uint64_t next_cl_ord_id_{1};
  std::optional<std::uint64_t> cancel_target_;  // cl_ord_id from seq==3
};

}  // namespace ytta::v0
