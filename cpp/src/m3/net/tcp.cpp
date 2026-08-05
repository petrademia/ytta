#include "net/tcp.hpp"

namespace ytta::m3 {

bool TcpSocket::listen_localhost(std::uint16_t) {
  return false;
}

bool TcpSocket::connect_localhost(std::uint16_t) {
  return false;
}

bool TcpSocket::send_all(const std::uint8_t*, std::size_t) {
  return false;
}

ssize_t TcpSocket::recv_some(std::uint8_t*, std::size_t) {
  return -1;
}

void TcpSocket::close() {}

}  // namespace ytta::m3
