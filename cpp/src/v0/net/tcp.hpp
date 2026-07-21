#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ytta::v0::net {

int tcp_listen(const std::string& host, std::uint16_t port);
int tcp_accept(int listen_fd);
int tcp_connect(const std::string& host, std::uint16_t port);
void tcp_close(int fd);

void write_all(int fd, std::string_view data);

// Buffered framed reader for one connected socket.
class FrameReader {
 public:
  explicit FrameReader(int fd) : fd_(fd) {}
  std::string read_frame();  // blocking; throws on EOF/error

 private:
  int fd_{-1};
  std::string buf_;
};

}  // namespace ytta::v0::net
