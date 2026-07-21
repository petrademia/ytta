#include "book.hpp"

#include <algorithm>

namespace ytta::v0 {

void Book::add(const Order& order) {
  BookLevelOrder level{order.cl_ord_id, order.qty};
  if (order.side == Side::Buy) {
    bids_[order.price].push_back(level);
  } else {
    asks_[order.price].push_back(level);
  }
  side_by_id_[order.cl_ord_id] = order.side;
}

std::optional<std::int64_t> Book::cancel(std::uint64_t cl_ord_id) {
  const auto side_it = side_by_id_.find(cl_ord_id);
  if (side_it == side_by_id_.end()) {
    return std::nullopt;
  }
  const Side side = side_it->second;
  auto erase_from = [&](auto& levels) -> std::optional<std::int64_t> {
    for (auto lit = levels.begin(); lit != levels.end(); ++lit) {
      auto& q = lit->second;
      for (auto oit = q.begin(); oit != q.end(); ++oit) {
        if (oit->cl_ord_id == cl_ord_id) {
          const std::int64_t qty = oit->qty;
          q.erase(oit);
          if (q.empty()) {
            levels.erase(lit);
          }
          side_by_id_.erase(cl_ord_id);
          return qty;
        }
      }
    }
    return std::nullopt;
  };
  if (side == Side::Buy) {
    return erase_from(bids_);
  }
  return erase_from(asks_);
}

std::vector<Book::MatchSlice> Book::match(Side aggressor_side,
                                          std::int64_t price,
                                          std::int64_t qty) {
  std::vector<MatchSlice> slices;
  std::int64_t remaining = qty;

  auto match_levels = [&](auto& levels, auto crosses) {
    while (remaining > 0 && !levels.empty()) {
      auto lit = levels.begin();
      if (!crosses(lit->first, price)) {
        break;
      }
      auto& q = lit->second;
      while (remaining > 0 && !q.empty()) {
        auto& head = q.front();
        const std::int64_t fill_qty = std::min(remaining, head.qty);
        slices.push_back(MatchSlice{head.cl_ord_id, lit->first, fill_qty});
        head.qty -= fill_qty;
        remaining -= fill_qty;
        if (head.qty == 0) {
          side_by_id_.erase(head.cl_ord_id);
          q.pop_front();
        }
      }
      if (q.empty()) {
        levels.erase(lit);
      }
    }
  };

  if (aggressor_side == Side::Buy) {
    match_levels(asks_, [](std::int64_t ask_px, std::int64_t buy_px) {
      return ask_px <= buy_px;
    });
  } else {
    match_levels(bids_, [](std::int64_t bid_px, std::int64_t sell_px) {
      return bid_px >= sell_px;
    });
  }
  return slices;
}

MdEvent Book::snapshot(std::int64_t ts_ns) const {
  MdEvent md;
  md.ts_ns = ts_ns;
  md.instrument = "INSTR1";
  if (!bids_.empty()) {
    md.best_bid = bids_.begin()->first;
    std::int64_t qty = 0;
    for (const auto& o : bids_.begin()->second) {
      qty += o.qty;
    }
    md.best_bid_qty = qty;
  }
  if (!asks_.empty()) {
    md.best_ask = asks_.begin()->first;
    std::int64_t qty = 0;
    for (const auto& o : asks_.begin()->second) {
      qty += o.qty;
    }
    md.best_ask_qty = qty;
  }
  return md;
}

}  // namespace ytta::v0
