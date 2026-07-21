#pragma once

#include "types.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ytta::v0 {

struct Tick {
  std::int64_t ts_ns{0};
  std::string instrument;
  Side side{Side::Buy};
  std::int64_t price{0};
  std::int64_t qty{0};
  std::uint64_t seq{0};
};

class TickSource {
 public:
  explicit TickSource(std::string path) : path_(std::move(path)) {}
  std::vector<Tick> load() const;

 private:
  std::string path_;
};

std::optional<Tick> parse_tick_line(std::string_view line);

}  // namespace ytta::v0
