#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace ytta::m2 {

enum class Side { Buy, Sell };

enum class AckStatus { Accepted, Rejected };

enum class Liquidity { Maker, Taker };

struct Order {
  std::uint64_t cl_ord_id;
  Side side;
  std::int64_t price;
  std::int64_t qty;
};

struct AckEvent {
  std::uint64_t ts_ns;
  std::uint64_t cl_ord_id;
  AckStatus status;
  std::string reason;
};

struct FillEvent {
  std::uint64_t ts_ns;
  std::uint64_t cl_ord_id;
  std::int64_t price;
  std::int64_t qty;
  Liquidity liquidity;
};

struct MdEvent {
  std::uint64_t ts_ns;
  std::int64_t best_bid;
  std::int64_t best_ask;
  std::int64_t best_bid_qty;
  std::int64_t best_ask_qty;
};

using EngineEvent = std::variant<AckEvent, FillEvent, MdEvent>;

}  // namespace ytta::m2
