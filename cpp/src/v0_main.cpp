#include "latency_probe.hpp"
#include "pipeline.hpp"
#include "tick_source.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace {

void usage(const char* argv0) {
  std::cerr << "Usage: " << argv0
            << " --fixture <ticks.ndjson> --out <events.ndjson>"
            << " [--mode=sync|queued]\n";
}

ytta::v0::PipelineMode parse_mode(const std::string& s) {
  if (s == "sync") {
    return ytta::v0::PipelineMode::Sync;
  }
  if (s == "queued") {
    return ytta::v0::PipelineMode::Queued;
  }
  throw std::runtime_error("invalid --mode (use sync|queued)");
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

}  // namespace

int main(int argc, char** argv) {
  std::string fixture;
  std::string out_path;
  auto mode = ytta::v0::PipelineMode::Sync;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--fixture" && i + 1 < argc) {
      fixture = argv[++i];
    } else if (arg == "--out" && i + 1 < argc) {
      out_path = argv[++i];
    } else if (arg.rfind("--mode=", 0) == 0) {
      try {
        mode = parse_mode(arg.substr(7));
      } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << '\n';
        return EXIT_FAILURE;
      }
    } else if (arg == "--mode" && i + 1 < argc) {
      try {
        mode = parse_mode(argv[++i]);
      } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << '\n';
        return EXIT_FAILURE;
      }
    } else if (arg == "--help" || arg == "-h") {
      usage(argv[0]);
      return EXIT_SUCCESS;
    } else {
      usage(argv[0]);
      return EXIT_FAILURE;
    }
  }
  if (fixture.empty() || out_path.empty()) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  try {
    ytta::v0::TickSource source(fixture);
    const auto ticks = source.load();
    ytta::v0::LatencyProbe probe;
    ytta::v0::Pipeline pipeline;
    const auto events = pipeline.run(ticks, probe, mode);

    std::ofstream out(out_path);
    if (!out) {
      std::cerr << "failed to open --out path\n";
      return EXIT_FAILURE;
    }
    for (const auto& e : events) {
      out << e.line << '\n';
    }

    print_latency(probe.summarize());

    if (probe.summarize().drops != 0) {
      std::cerr << "error: queue drops=" << probe.summarize().drops << '\n';
      return EXIT_FAILURE;
    }
  } catch (const std::exception& ex) {
    std::cerr << "error: " << ex.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
