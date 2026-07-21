#include "latency_probe.hpp"
#include "pipeline.hpp"
#include "strategy.hpp"
#include "tcp_order_gateway.hpp"
#include "tick_source.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <variant>

namespace {

void usage(const char* argv0) {
  std::cerr << "Usage: " << argv0
            << " --host <addr> --port <N> --fixture <ticks.ndjson>"
            << " --out <events.ndjson>\n";
}

void print_latency(const ytta::v0::LatencyProbe::Summary& summary) {
  std::cerr << "{\"type\":\"latency\",\"count\":" << summary.count
            << ",\"p50_ns\":" << summary.p50_ns
            << ",\"p99_ns\":" << summary.p99_ns
            << ",\"ingest_p50_ns\":" << summary.ingest_p50_ns
            << ",\"ingest_p99_ns\":" << summary.ingest_p99_ns
            << ",\"decide_p50_ns\":" << summary.decide_p50_ns
            << ",\"decide_p99_ns\":" << summary.decide_p99_ns
            << ",\"execute_p50_ns\":" << summary.execute_p50_ns
            << ",\"execute_p99_ns\":" << summary.execute_p99_ns
            << ",\"drops\":" << summary.drops << "}\n";
}

std::vector<ytta::v0::GoldenEvent> run_tcp_client(
    const std::vector<ytta::v0::Tick>& ticks, ytta::v0::LatencyProbe& probe,
    ytta::v0::TcpOrderGateway& gateway) {
  using namespace ytta::v0;
  Strategy strategy;
  std::vector<GoldenEvent> out;

  for (const auto& tick : ticks) {
    const auto e2e_start = std::chrono::steady_clock::now();
    const auto ingest_end = std::chrono::steady_clock::now();

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

    probe.record_stages(steady_ns_between(e2e_start, execute_end),
                        steady_ns_between(e2e_start, ingest_end),
                        steady_ns_between(decide_start, decide_end),
                        steady_ns_between(execute_start, execute_end));
  }

  sort_golden(out);
  return out;
}

}  // namespace

int main(int argc, char** argv) {
  std::string host = "127.0.0.1";
  std::uint16_t port = 0;
  bool port_set = false;
  std::string fixture;
  std::string out_path;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--host" && i + 1 < argc) {
      host = argv[++i];
    } else if (arg == "--port" && i + 1 < argc) {
      port = static_cast<std::uint16_t>(std::stoi(argv[++i]));
      port_set = true;
    } else if (arg == "--fixture" && i + 1 < argc) {
      fixture = argv[++i];
    } else if (arg == "--out" && i + 1 < argc) {
      out_path = argv[++i];
    } else if (arg == "--help" || arg == "-h") {
      usage(argv[0]);
      return EXIT_SUCCESS;
    } else {
      usage(argv[0]);
      return EXIT_FAILURE;
    }
  }
  if (!port_set || fixture.empty() || out_path.empty()) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  try {
    ytta::v0::TickSource source(fixture);
    const auto ticks = source.load();
    ytta::v0::LatencyProbe probe;
    ytta::v0::TcpOrderGateway gateway(host, port);
    const auto events = run_tcp_client(ticks, probe, gateway);

    std::ofstream out(out_path);
    if (!out) {
      std::cerr << "failed to open --out path\n";
      return EXIT_FAILURE;
    }
    for (const auto& e : events) {
      out << e.line << '\n';
    }

    print_latency(probe.summarize());
  } catch (const std::exception& ex) {
    std::cerr << "error: " << ex.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
