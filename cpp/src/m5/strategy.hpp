#pragma once

#include <cstdint>
#include <string>

namespace ytta::m5 {

enum class Side { Buy, Sell };
enum class ActionType { Noop, NewOrder, Cancel };

struct Tick {
  std::uint64_t ts_ns;
  Side side;
  std::int64_t price;
  std::int64_t qty;
};

struct MdSnapshot {
  std::int64_t best_bid{0};
  std::int64_t best_ask{0};
  std::int64_t best_bid_qty{0};
  std::int64_t best_ask_qty{0};
};

struct Action {
  ActionType type{ActionType::Noop};
  std::uint64_t cl_ord_id{0};
  Side side{Side::Buy};
  std::int64_t price{0};
  std::int64_t qty{0};
  std::string reason{"noop"};
};

class Strategy {
 public:
  // Naive taker skeleton:
  // - A Buy tick crossing the best ask emits NEW_ORDER Buy at the ask.
  // - The next tick emits CANCEL for the open order.
  // This stub intentionally always returns Noop.
  Action on_tick(const Tick& tick, const MdSnapshot& md);

 private:
  std::uint64_t next_id_{1};
  bool has_open_{false};
  std::uint64_t open_id_{0};
};

}  // namespace ytta::m5
