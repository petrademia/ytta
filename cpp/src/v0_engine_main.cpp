#include "engine_server.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void usage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " --port <N>\n"
            << "  Listen on 127.0.0.1:N and serve sequential client sessions.\n"
            << "  Stays up until killed (CI harness sends SIGTERM).\n";
}

}  // namespace

int main(int argc, char** argv) {
  std::uint16_t port = 0;
  bool port_set = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--port" && i + 1 < argc) {
      port = static_cast<std::uint16_t>(std::stoi(argv[++i]));
      port_set = true;
    } else if (arg == "--help" || arg == "-h") {
      usage(argv[0]);
      return EXIT_SUCCESS;
    } else {
      usage(argv[0]);
      return EXIT_FAILURE;
    }
  }
  if (!port_set) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  try {
    ytta::v0::EngineServer server(port);
    std::cerr << "{\"type\":\"engine_ready\",\"host\":\"127.0.0.1\",\"port\":"
              << server.port() << "}\n";
    server.run();
  } catch (const std::exception& ex) {
    std::cerr << "error: " << ex.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
