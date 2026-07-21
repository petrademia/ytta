#pragma once

#include "types.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ytta::v0::net {

constexpr std::uint32_t kMaxFrameBytes = 1u << 20;

struct WireOrder {
  std::int64_t ts_ns{0};
  std::uint64_t cl_ord_id{0};
  Op op{Op::New};
  Side side{Side::Buy};
  std::int64_t price{0};
  std::int64_t qty{0};
};

std::string encode_frame_payload(std::string_view json_object);
// Full frame: 4-byte BE length + payload.
std::string encode_frame(std::string_view json_object);

// Decode length-prefixed frame from a contiguous buffer; returns payload JSON
// and bytes consumed, or nullopt if incomplete/invalid.
std::optional<std::pair<std::string, std::size_t>> try_decode_frame(
    std::string_view buffer);

std::string encode_order(const WireOrder& o);
std::string encode_goodbye();
std::string encode_done();
std::string encode_engine_event(const EngineEvent& e);

std::optional<WireOrder> parse_order_json(std::string_view json);
bool is_goodbye_json(std::string_view json);
bool is_done_json(std::string_view json);
std::optional<EngineEvent> parse_engine_event_json(std::string_view json);

}  // namespace ytta::v0::net
