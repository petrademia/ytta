#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace ytta::v0 {

enum class Side : std::uint8_t { Buy, Sell };
enum class Op : std::uint8_t { New, Cancel };
enum class AckStatus : std::uint8_t { Accepted, Rejected };
enum class Liquidity : std::uint8_t { Maker, Taker };

struct Order {
  std::uint64_t cl_ord_id{0};
  Side side{Side::Buy};
  std::int64_t price{0};
  std::int64_t qty{0};
};

struct AckEvent {
  std::int64_t ts_ns{0};
  std::uint64_t cl_ord_id{0};
  AckStatus status{AckStatus::Accepted};
  std::string reason;
};

struct FillEvent {
  std::int64_t ts_ns{0};
  std::uint64_t cl_ord_id{0};
  std::int64_t price{0};
  std::int64_t qty{0};
  Liquidity liquidity{Liquidity::Taker};
};

struct MdEvent {
  std::int64_t ts_ns{0};
  std::string instrument{"INSTR1"};
  std::int64_t best_bid{0};
  std::int64_t best_ask{0};
  std::int64_t best_bid_qty{0};
  std::int64_t best_ask_qty{0};
};

using EngineEvent = std::variant<AckEvent, FillEvent, MdEvent>;

inline const char* to_string(Side side) {
  return side == Side::Buy ? "BUY" : "SELL";
}

inline const char* to_string(AckStatus status) {
  return status == AckStatus::Accepted ? "ACCEPTED" : "REJECTED";
}

inline const char* to_string(Liquidity liq) {
  return liq == Liquidity::Maker ? "MAKER" : "TAKER";
}

}  // namespace ytta::v0
