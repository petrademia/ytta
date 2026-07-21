#include "tick_source.hpp"

#include <fstream>
#include <stdexcept>

namespace ytta::v0 {
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

}  // namespace

std::optional<Tick> parse_tick_line(std::string_view line) {
  if (line.empty() || line[0] == '#') {
    return std::nullopt;
  }
  const auto type = json_string_field(line, "type");
  if (!type || *type != "tick") {
    return std::nullopt;
  }
  Tick t;
  const auto ts = json_int_field(line, "ts_ns");
  const auto price = json_int_field(line, "price");
  const auto qty = json_int_field(line, "qty");
  const auto seq = json_int_field(line, "seq");
  const auto instrument = json_string_field(line, "instrument");
  const auto side = json_string_field(line, "side");
  if (!ts || !price || !qty || !seq || !instrument || !side) {
    throw std::runtime_error("malformed tick line");
  }
  t.ts_ns = *ts;
  t.price = *price;
  t.qty = *qty;
  t.seq = static_cast<std::uint64_t>(*seq);
  t.instrument = std::string(*instrument);
  if (*side == "BUY") {
    t.side = Side::Buy;
  } else if (*side == "SELL") {
    t.side = Side::Sell;
  } else {
    throw std::runtime_error("invalid tick side");
  }
  return t;
}

std::vector<Tick> TickSource::load() const {
  std::ifstream in(path_);
  if (!in) {
    throw std::runtime_error("failed to open fixture: " + path_);
  }
  std::vector<Tick> ticks;
  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (auto t = parse_tick_line(line)) {
      ticks.push_back(std::move(*t));
    }
  }
  return ticks;
}

}  // namespace ytta::v0
