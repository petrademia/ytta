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
            << " --fixture <ticks.ndjson> --out <events.ndjson>\n";
}

}  // namespace

int main(int argc, char** argv) {
  std::string fixture;
  std::string out_path;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--fixture" && i + 1 < argc) {
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
  if (fixture.empty() || out_path.empty()) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  try {
    ytta::v0::TickSource source(fixture);
    const auto ticks = source.load();
    ytta::v0::LatencyProbe probe;
    ytta::v0::Pipeline pipeline;
    const auto events = pipeline.run(ticks, probe);

    std::ofstream out(out_path);
    if (!out) {
      std::cerr << "failed to open --out path\n";
      return EXIT_FAILURE;
    }
    for (const auto& e : events) {
      out << e.line << '\n';
    }

    const auto summary = probe.summarize();
    std::cerr << "{\"type\":\"latency\",\"count\":" << summary.count
              << ",\"p50_ns\":" << summary.p50_ns
              << ",\"p99_ns\":" << summary.p99_ns << "}\n";
  } catch (const std::exception& ex) {
    std::cerr << "error: " << ex.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
