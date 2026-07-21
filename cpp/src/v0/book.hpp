#pragma once

#include "types.hpp"

#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

namespace ytta::v0 {

struct BookLevelOrder {
  std::uint64_t cl_ord_id{0};
  std::int64_t qty{0};
};

class Book {
 public:
  void add(const Order& order);
  // Returns remaining qty of the cancelled order, or nullopt if unknown.
  std::optional<std::int64_t> cancel(std::uint64_t cl_ord_id);

  // Match aggressor against the opposite side. Mutates book. Returns fills
  // for maker then taker per matched slice (caller sets ts_ns/liquidity already
  // partially — this returns maker_id, price, qty; aggressor fills derived).
  struct MatchSlice {
    std::uint64_t maker_id{0};
    std::int64_t price{0};
    std::int64_t qty{0};
  };
  std::vector<MatchSlice> match(Side aggressor_side, std::int64_t price,
                                std::int64_t qty);

  MdEvent snapshot(std::int64_t ts_ns) const;

 private:
  // Bids: highest price first. Asks: lowest price first.
  std::map<std::int64_t, std::deque<BookLevelOrder>, std::greater<>> bids_;
  std::map<std::int64_t, std::deque<BookLevelOrder>, std::less<>> asks_;
  std::unordered_map<std::uint64_t, Side> side_by_id_;
};

}  // namespace ytta::v0
