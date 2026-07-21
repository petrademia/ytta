#include "pipeline.hpp"

#include "matching_engine.hpp"
#include "spsc_queue.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>

namespace ytta::v0 {
namespace {

int type_rank(GoldenType t) {
  switch (t) {
    case GoldenType::Md:
      return 0;
    case GoldenType::Action:
      return 1;
    case GoldenType::Ack:
      return 2;
    case GoldenType::Fill:
      return 3;
  }
  return 99;
}

void append_action_and_engine(std::vector<GoldenEvent>& out, Strategy& strategy,
                              OrderGateway& gateway, const Tick& tick,
                              LatencyProbe& probe,
                              std::chrono::steady_clock::time_point e2e_start,
                              std::int64_t ingest_ns) {
  const auto decide_start = std::chrono::steady_clock::now();
  const StrategyAction action = strategy.on_tick(tick);
  const auto decide_end = std::chrono::steady_clock::now();

  GoldenEvent ge;
  ge.ts_ns = action.ts_ns;
  ge.type = GoldenType::Action;
  ge.cl_ord_id = (action.kind == ActionKind::Noop) ? 0 : action.cl_ord_id;
  ge.line = format_action(action);
  out.push_back(std::move(ge));

  const auto execute_start = std::chrono::steady_clock::now();
  if (action.kind != ActionKind::Noop) {
    std::vector<EngineEvent> events;
    if (action.kind == ActionKind::NewOrder) {
      events = gateway.new_order(
          Order{action.cl_ord_id, action.side, action.price, action.qty},
          action.ts_ns);
    } else {
      events = gateway.cancel(action.cl_ord_id, action.ts_ns);
    }
    for (const auto& ev : events) {
      GoldenEvent ee;
      ee.ts_ns = action.ts_ns;
      ee.line = format_engine_event(ev);
      if (const auto* ack = std::get_if<AckEvent>(&ev)) {
        ee.type = GoldenType::Ack;
        ee.cl_ord_id = ack->cl_ord_id;
      } else if (const auto* fill = std::get_if<FillEvent>(&ev)) {
        ee.type = GoldenType::Fill;
        ee.cl_ord_id = fill->cl_ord_id;
      } else {
        ee.type = GoldenType::Md;
        ee.cl_ord_id = 0;
      }
      out.push_back(std::move(ee));
    }
  }
  const auto execute_end = std::chrono::steady_clock::now();

  probe.record_stages(steady_ns_between(e2e_start, execute_end), ingest_ns,
                      steady_ns_between(decide_start, decide_end),
                      steady_ns_between(execute_start, execute_end));
}

}  // namespace

std::string format_action(const StrategyAction& a) {
  std::ostringstream os;
  os << "{\"type\":\"action\",\"ts_ns\":" << a.ts_ns << ",\"action\":\"";
  switch (a.kind) {
    case ActionKind::Noop:
      os << "NOOP";
      break;
    case ActionKind::NewOrder:
      os << "NEW_ORDER";
      break;
    case ActionKind::Cancel:
      os << "CANCEL";
      break;
  }
  os << "\"";
  if (a.kind == ActionKind::NewOrder) {
    os << ",\"cl_ord_id\":" << a.cl_ord_id << ",\"side\":\"" << to_string(a.side)
       << "\",\"price\":" << a.price << ",\"qty\":" << a.qty;
  } else if (a.kind == ActionKind::Cancel) {
    os << ",\"cl_ord_id\":" << a.cl_ord_id;
  }
  os << ",\"reason\":\"" << a.reason << "\"}";
  return os.str();
}

std::string format_engine_event(const EngineEvent& e) {
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

void sort_golden(std::vector<GoldenEvent>& events) {
  std::sort(events.begin(), events.end(),
            [](const GoldenEvent& a, const GoldenEvent& b) {
              if (a.ts_ns != b.ts_ns) {
                return a.ts_ns < b.ts_ns;
              }
              const int ra = type_rank(a.type);
              const int rb = type_rank(b.type);
              if (ra != rb) {
                return ra < rb;
              }
              return a.cl_ord_id < b.cl_ord_id;
            });
}

std::vector<GoldenEvent> Pipeline::run(const std::vector<Tick>& ticks,
                                       LatencyProbe& probe,
                                       PipelineMode mode) {
  if (mode == PipelineMode::Queued) {
    return run_queued(ticks, probe);
  }
  return run_sync(ticks, probe);
}

std::vector<GoldenEvent> Pipeline::run_sync(const std::vector<Tick>& ticks,
                                            LatencyProbe& probe) {
  MatchingEngine engine;
  OrderGateway gateway(engine);
  Strategy strategy;
  std::vector<GoldenEvent> out;

  for (const auto& tick : ticks) {
    const auto e2e_start = std::chrono::steady_clock::now();
    // Sync ingest is a no-op handoff (copy already owned by caller).
    const auto ingest_end = std::chrono::steady_clock::now();
    append_action_and_engine(out, strategy, gateway, tick, probe, e2e_start,
                             steady_ns_between(e2e_start, ingest_end));
  }

  sort_golden(out);
  return out;
}

std::vector<GoldenEvent> Pipeline::run_queued(const std::vector<Tick>& ticks,
                                              LatencyProbe& probe) {
  // Capacity covers v0 + stage1 burst fixtures without drops when >= tick count.
  constexpr std::size_t kCap = 16384;
  SpscQueue<Tick, kCap> queue;

  MatchingEngine engine;
  OrderGateway gateway(engine);
  Strategy strategy;
  std::vector<GoldenEvent> out;

  for (const auto& tick : ticks) {
    const auto e2e_start = std::chrono::steady_clock::now();
    const auto ingest_start = e2e_start;
    if (!queue.try_push(tick)) {
      probe.add_drop();
      continue;
    }
    const auto ingest_end = std::chrono::steady_clock::now();

    Tick consumed;
    if (!queue.try_pop(consumed)) {
      // Should be unreachable after a successful push on a single-threaded pump.
      probe.add_drop();
      continue;
    }

    append_action_and_engine(out, strategy, gateway, consumed, probe, e2e_start,
                             steady_ns_between(ingest_start, ingest_end));
  }

  sort_golden(out);
  return out;
}

}  // namespace ytta::v0
