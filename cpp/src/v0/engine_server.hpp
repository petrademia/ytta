#pragma once

#include "net/tcp.hpp"

#include <cstdint>
#include <string>

namespace ytta::v0 {

// Serves sequential client sessions on one listen socket. Stays up until
// process kill (Stage 2 CI harness kills the engine after one client run).
// Each session gets a fresh MatchingEngine (paper book starts empty).
class EngineServer {
 public:
  explicit EngineServer(std::uint16_t port, std::string host = "127.0.0.1");
  ~EngineServer();

  EngineServer(const EngineServer&) = delete;
  EngineServer& operator=(const EngineServer&) = delete;

  std::uint16_t port() const { return port_; }

  // Accept one client, process orders until goodbye/disconnect, then return.
  void serve_one_session();

  // Loop: accept sessions forever (or until listen fails).
  void run();

 private:
  void handle_session(int client_fd);

  std::string host_;
  std::uint16_t port_{0};
  int listen_fd_{-1};
};

}  // namespace ytta::v0
