#pragma once

#include "net/framing.hpp"
#include "net/tcp.hpp"
#include "types.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ytta::v0 {

class TcpOrderGateway {
 public:
  TcpOrderGateway(std::string host, std::uint16_t port);
  ~TcpOrderGateway();

  TcpOrderGateway(const TcpOrderGateway&) = delete;
  TcpOrderGateway& operator=(const TcpOrderGateway&) = delete;

  std::vector<EngineEvent> new_order(const Order& order, std::int64_t ts_ns);
  std::vector<EngineEvent> cancel(std::uint64_t cl_ord_id, std::int64_t ts_ns);

  void goodbye();

 private:
  std::vector<EngineEvent> submit(const net::WireOrder& wire);

  int fd_{-1};
  net::FrameReader reader_{0};
};

}  // namespace ytta::v0
