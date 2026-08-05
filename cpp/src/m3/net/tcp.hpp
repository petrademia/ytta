#pragma once

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

namespace ytta::m3 {

class TcpSocket {
 public:
  bool listen_localhost(std::uint16_t port);
  bool connect_localhost(std::uint16_t port);
  bool send_all(const std::uint8_t* data, std::size_t n);
  ssize_t recv_some(std::uint8_t* data, std::size_t n);
  void close();
};

}  // namespace ytta::m3
