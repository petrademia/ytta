#include "tcp.hpp"

#include "framing.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace ytta::v0::net {
namespace {

void set_reuseaddr(int fd) {
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    throw std::runtime_error(std::string("setsockopt: ") + std::strerror(errno));
  }
}

sockaddr_in make_addr(const std::string& host, std::uint16_t port) {
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
    throw std::runtime_error("inet_pton failed for host " + host);
  }
  return addr;
}

}  // namespace

int tcp_listen(const std::string& host, std::uint16_t port) {
  const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    throw std::runtime_error(std::string("socket: ") + std::strerror(errno));
  }
  try {
    set_reuseaddr(fd);
    const auto addr = make_addr(host, port);
    if (::bind(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
      throw std::runtime_error(std::string("bind: ") + std::strerror(errno));
    }
    if (::listen(fd, 1) < 0) {
      throw std::runtime_error(std::string("listen: ") + std::strerror(errno));
    }
  } catch (...) {
    ::close(fd);
    throw;
  }
  return fd;
}

int tcp_accept(int listen_fd) {
  const int fd = ::accept(listen_fd, nullptr, nullptr);
  if (fd < 0) {
    throw std::runtime_error(std::string("accept: ") + std::strerror(errno));
  }
  return fd;
}

int tcp_connect(const std::string& host, std::uint16_t port) {
  const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    throw std::runtime_error(std::string("socket: ") + std::strerror(errno));
  }
  const auto addr = make_addr(host, port);
  if (::connect(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
    const int err = errno;
    ::close(fd);
    throw std::runtime_error(std::string("connect: ") + std::strerror(err));
  }
  return fd;
}

void tcp_close(int fd) {
  if (fd >= 0) {
    ::close(fd);
  }
}

void write_all(int fd, std::string_view data) {
  std::size_t off = 0;
  while (off < data.size()) {
    const ssize_t n = ::send(fd, data.data() + off, data.size() - off, 0);
    if (n < 0) {
      throw std::runtime_error(std::string("send: ") + std::strerror(errno));
    }
    off += static_cast<std::size_t>(n);
  }
}

std::string FrameReader::read_frame() {
  char tmp[4096];
  while (true) {
    if (auto decoded = try_decode_frame(buf_)) {
      const std::string payload = std::move(decoded->first);
      buf_.erase(0, decoded->second);
      return payload;
    }
    const ssize_t n = ::recv(fd_, tmp, sizeof(tmp), 0);
    if (n == 0) {
      throw std::runtime_error("connection closed");
    }
    if (n < 0) {
      throw std::runtime_error(std::string("recv: ") + std::strerror(errno));
    }
    buf_.append(tmp, static_cast<std::size_t>(n));
    if (buf_.size() > 4 + kMaxFrameBytes) {
      throw std::runtime_error("incoming buffer too large");
    }
  }
}

}  // namespace ytta::v0::net
