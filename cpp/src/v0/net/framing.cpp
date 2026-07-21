#include "framing.hpp"

#include <cstring>
#include <sstream>
#include <stdexcept>

namespace ytta::v0::net {
namespace {

std::optional<std::string_view> json_string_field(std::string_view line,
                                                  std::string_view key) {
  const std::string needle = "\"" + std::string(key) + "\":\"";
  const auto pos = line.find(needle);
  if (pos == std::string_view::npos) {
    return std::nullopt;
  }
  const auto start = pos + needle.size();
  const auto end = line.find('"', start);
  if (end == std::string_view::npos) {
    return std::nullopt;
  }
  return line.substr(start, end - start);
}

std::optional<std::int64_t> json_int_field(std::string_view line,
                                           std::string_view key) {
  const std::string needle = "\"" + std::string(key) + "\":";
  const auto pos = line.find(needle);
  if (pos == std::string_view::npos) {
    return std::nullopt;
  }
  auto start = pos + needle.size();
  while (start < line.size() &&
         (line[start] == ' ' || line[start] == '\t')) {
    ++start;
  }
  std::size_t end = start;
  if (end < line.size() && (line[end] == '-' || line[end] == '+')) {
    ++end;
  }
  while (end < line.size() && line[end] >= '0' && line[end] <= '9') {
    ++end;
  }
  if (end == start) {
    return std::nullopt;
  }
  return std::stoll(std::string(line.substr(start, end - start)));
}

void append_be_u32(std::string& out, std::uint32_t v) {
  out.push_back(static_cast<char>((v >> 24) & 0xff));
  out.push_back(static_cast<char>((v >> 16) & 0xff));
  out.push_back(static_cast<char>((v >> 8) & 0xff));
  out.push_back(static_cast<char>(v & 0xff));
}

std::uint32_t read_be_u32(std::string_view bytes) {
  return (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[0])) << 24) |
         (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[1])) << 16) |
         (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[2])) << 8) |
         (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[3])));
}

}  // namespace

std::string encode_frame_payload(std::string_view json_object) {
  return std::string(json_object);
}

std::string encode_frame(std::string_view json_object) {
  if (json_object.size() > kMaxFrameBytes) {
    throw std::runtime_error("frame too large");
  }
  std::string out;
  out.reserve(4 + json_object.size());
  append_be_u32(out, static_cast<std::uint32_t>(json_object.size()));
  out.append(json_object);
  return out;
}

std::optional<std::pair<std::string, std::size_t>> try_decode_frame(
    std::string_view buffer) {
  if (buffer.size() < 4) {
    return std::nullopt;
  }
  const std::uint32_t len = read_be_u32(buffer.substr(0, 4));
  if (len > kMaxFrameBytes) {
    throw std::runtime_error("frame length exceeds max");
  }
  if (buffer.size() < 4 + len) {
    return std::nullopt;
  }
  return std::make_pair(std::string(buffer.substr(4, len)),
                        static_cast<std::size_t>(4 + len));
}

std::string encode_order(const WireOrder& o) {
  std::ostringstream os;
  os << "{\"type\":\"order\",\"ts_ns\":" << o.ts_ns
     << ",\"cl_ord_id\":" << o.cl_ord_id << ",\"op\":\""
     << (o.op == Op::New ? "NEW" : "CANCEL") << "\"";
  if (o.op == Op::New) {
    os << ",\"side\":\"" << to_string(o.side) << "\",\"price\":" << o.price
       << ",\"qty\":" << o.qty;
  }
  os << "}";
  return os.str();
}

std::string encode_goodbye() { return "{\"type\":\"goodbye\"}"; }

std::string encode_done() { return "{\"type\":\"done\"}"; }

std::string encode_engine_event(const EngineEvent& e) {
  std::ostringstream os;
  if (const auto* ack = std::get_if<AckEvent>(&e)) {
    os << "{\"type\":\"ack\",\"ts_ns\":" << ack->ts_ns
       << ",\"cl_ord_id\":" << ack->cl_ord_id << ",\"status\":\""
       << to_string(ack->status) << "\"";
    if (ack->status == AckStatus::Rejected) {
      os << ",\"reason\":\"" << ack->reason << "\"";
    }
    os << "}";
    return os.str();
  }
  if (const auto* fill = std::get_if<FillEvent>(&e)) {
    os << "{\"type\":\"fill\",\"ts_ns\":" << fill->ts_ns
       << ",\"cl_ord_id\":" << fill->cl_ord_id << ",\"price\":" << fill->price
       << ",\"qty\":" << fill->qty << ",\"liquidity\":\""
       << to_string(fill->liquidity) << "\"}";
    return os.str();
  }
  const auto& md = std::get<MdEvent>(e);
  os << "{\"type\":\"md\",\"ts_ns\":" << md.ts_ns << ",\"instrument\":\""
     << md.instrument << "\",\"best_bid\":" << md.best_bid
     << ",\"best_ask\":" << md.best_ask << ",\"best_bid_qty\":" << md.best_bid_qty
     << ",\"best_ask_qty\":" << md.best_ask_qty << "}";
  return os.str();
}

std::optional<WireOrder> parse_order_json(std::string_view json) {
  const auto type = json_string_field(json, "type");
  if (!type || *type != "order") {
    return std::nullopt;
  }
  WireOrder o;
  const auto ts = json_int_field(json, "ts_ns");
  const auto id = json_int_field(json, "cl_ord_id");
  const auto op = json_string_field(json, "op");
  if (!ts || !id || !op) {
    return std::nullopt;
  }
  o.ts_ns = *ts;
  o.cl_ord_id = static_cast<std::uint64_t>(*id);
  if (*op == "NEW") {
    o.op = Op::New;
    const auto side = json_string_field(json, "side");
    const auto price = json_int_field(json, "price");
    const auto qty = json_int_field(json, "qty");
    if (!side || !price || !qty) {
      return std::nullopt;
    }
    o.side = (*side == "BUY") ? Side::Buy : Side::Sell;
    o.price = *price;
    o.qty = *qty;
  } else if (*op == "CANCEL") {
    o.op = Op::Cancel;
  } else {
    return std::nullopt;
  }
  return o;
}

bool is_goodbye_json(std::string_view json) {
  const auto type = json_string_field(json, "type");
  return type && *type == "goodbye";
}

bool is_done_json(std::string_view json) {
  const auto type = json_string_field(json, "type");
  return type && *type == "done";
}

std::optional<EngineEvent> parse_engine_event_json(std::string_view json) {
  const auto type = json_string_field(json, "type");
  if (!type) {
    return std::nullopt;
  }
  if (*type == "ack") {
    AckEvent ack;
    const auto ts = json_int_field(json, "ts_ns");
    const auto id = json_int_field(json, "cl_ord_id");
    const auto status = json_string_field(json, "status");
    if (!ts || !id || !status) {
      return std::nullopt;
    }
    ack.ts_ns = *ts;
    ack.cl_ord_id = static_cast<std::uint64_t>(*id);
    ack.status =
        (*status == "ACCEPTED") ? AckStatus::Accepted : AckStatus::Rejected;
    if (ack.status == AckStatus::Rejected) {
      const auto reason = json_string_field(json, "reason");
      ack.reason = reason ? std::string(*reason) : "";
    }
    return EngineEvent{ack};
  }
  if (*type == "fill") {
    FillEvent fill;
    const auto ts = json_int_field(json, "ts_ns");
    const auto id = json_int_field(json, "cl_ord_id");
    const auto price = json_int_field(json, "price");
    const auto qty = json_int_field(json, "qty");
    const auto liq = json_string_field(json, "liquidity");
    if (!ts || !id || !price || !qty || !liq) {
      return std::nullopt;
    }
    fill.ts_ns = *ts;
    fill.cl_ord_id = static_cast<std::uint64_t>(*id);
    fill.price = *price;
    fill.qty = *qty;
    fill.liquidity =
        (*liq == "MAKER") ? Liquidity::Maker : Liquidity::Taker;
    return EngineEvent{fill};
  }
  if (*type == "md") {
    MdEvent md;
    const auto ts = json_int_field(json, "ts_ns");
    const auto instr = json_string_field(json, "instrument");
    const auto bb = json_int_field(json, "best_bid");
    const auto ba = json_int_field(json, "best_ask");
    const auto bbq = json_int_field(json, "best_bid_qty");
    const auto baq = json_int_field(json, "best_ask_qty");
    if (!ts || !instr || !bb || !ba || !bbq || !baq) {
      return std::nullopt;
    }
    md.ts_ns = *ts;
    md.instrument = std::string(*instr);
    md.best_bid = *bb;
    md.best_ask = *ba;
    md.best_bid_qty = *bbq;
    md.best_ask_qty = *baq;
    return EngineEvent{md};
  }
  return std::nullopt;
}

}  // namespace ytta::v0::net
