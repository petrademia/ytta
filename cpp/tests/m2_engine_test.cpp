#include "order_gateway.hpp"
#include "matching_engine.hpp"
#include "types.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool cond, const char* msg) {
  if (!cond) {
    std::cerr << "FAIL: " << msg << "\n";
    ++failures;
  }
}

ytta::m2::AckEvent as_ack(const ytta::m2::EngineEvent& e) {
  return std::get<ytta::m2::AckEvent>(e);
}

ytta::m2::FillEvent as_fill(const ytta::m2::EngineEvent& e) {
  return std::get<ytta::m2::FillEvent>(e);
}

ytta::m2::MdEvent as_md(const ytta::m2::EngineEvent& e) {
  return std::get<ytta::m2::MdEvent>(e);
}

}  // namespace

int main() {
  using namespace ytta::m2;

  {
    MatchingEngine engine;
    OrderGateway gw(engine);
    Order buy{1, Side::Buy, 100, 5};
    auto ev = gw.new_order(buy, 1000);
    expect(ev.size() == 2, "new resting: ack+md");
    if (ev.size() >= 2) {
      expect(std::holds_alternative<AckEvent>(ev[0]), "first is ack");
      expect(as_ack(ev[0]).status == AckStatus::Accepted, "accepted");
      expect(std::holds_alternative<MdEvent>(ev[1]), "second is md");
      expect(as_md(ev[1]).best_bid == 100, "best bid 100");
      expect(as_md(ev[1]).best_bid_qty == 5, "best bid qty 5");
      expect(as_md(ev[1]).best_ask_qty == 0, "no ask");
    }
  }

  {
    MatchingEngine engine;
    OrderGateway gw(engine);
    gw.new_order(Order{1, Side::Buy, 100, 5}, 1000);
    auto ev = gw.new_order(Order{2, Side::Sell, 100, 5}, 2000);
    expect(ev.size() == 4, "cross: ack+2 fills+md");
    if (ev.size() >= 4) {
      expect(as_ack(ev[0]).cl_ord_id == 2, "ack aggressor");
      expect(as_fill(ev[1]).cl_ord_id == 1, "maker fill");
      expect(as_fill(ev[1]).liquidity == Liquidity::Maker, "maker liq");
      expect(as_fill(ev[2]).cl_ord_id == 2, "taker fill");
      expect(as_fill(ev[2]).liquidity == Liquidity::Taker, "taker liq");
      expect(as_fill(ev[1]).price == 100 && as_fill(ev[1]).qty == 5, "fill px/qty");
      expect(as_md(ev[3]).best_bid_qty == 0 && as_md(ev[3]).best_ask_qty == 0,
             "book empty");
    }
  }

  {
    MatchingEngine engine;
    OrderGateway gw(engine);
    auto ev = gw.cancel(99, 1000);
    expect(ev.size() == 1, "bad cancel: ack only");
    if (ev.size() >= 1) {
      expect(as_ack(ev[0]).status == AckStatus::Rejected, "rejected");
      expect(as_ack(ev[0]).reason == "unknown_id", "unknown_id");
    }
  }

  {
    MatchingEngine engine;
    OrderGateway gw(engine);
    gw.new_order(Order{3, Side::Buy, 99, 1}, 3000);
    auto ev = gw.cancel(3, 4000);
    expect(ev.size() == 2, "cancel: ack+md");
    if (ev.size() >= 2) {
      expect(as_ack(ev[0]).status == AckStatus::Accepted, "cancel accepted");
      expect(as_md(ev[1]).best_bid_qty == 0, "bid cleared");
    }
  }

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "ytta_m2_tests ok\n";
  return EXIT_SUCCESS;
}
