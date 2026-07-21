#include "engine_server.hpp"

#include "matching_engine.hpp"
#include "net/framing.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdexcept>

namespace ytta::v0 {

EngineServer::EngineServer(std::uint16_t port, std::string host)
    : host_(std::move(host)), port_(port) {
  listen_fd_ = net::tcp_listen(host_, port_);
  if (port_ == 0) {
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    if (getsockname(listen_fd_, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
      throw std::runtime_error("getsockname failed after listen");
    }
    port_ = ntohs(addr.sin_port);
  }
}

EngineServer::~EngineServer() { net::tcp_close(listen_fd_); }

void EngineServer::serve_one_session() {
  const int client_fd = net::tcp_accept(listen_fd_);
  try {
    handle_session(client_fd);
  } catch (...) {
    net::tcp_close(client_fd);
    throw;
  }
  net::tcp_close(client_fd);
}

void EngineServer::run() {
  for (;;) {
    serve_one_session();
  }
}

void EngineServer::handle_session(int client_fd) {
  MatchingEngine engine;
  net::FrameReader reader(client_fd);
  for (;;) {
    std::string payload;
    try {
      payload = reader.read_frame();
    } catch (const std::runtime_error& ex) {
      if (std::string(ex.what()) == "connection closed") {
        return;
      }
      throw;
    }

    if (net::is_goodbye_json(payload)) {
      return;
    }

    auto order = net::parse_order_json(payload);
    if (!order) {
      throw std::runtime_error("engine expected order or goodbye frame");
    }

    Order o;
    o.cl_ord_id = order->cl_ord_id;
    o.side = order->side;
    o.price = order->price;
    o.qty = order->qty;
    const auto events = engine.submit(order->op, o, order->ts_ns);
    for (const auto& ev : events) {
      net::write_all(client_fd, net::encode_frame(net::encode_engine_event(ev)));
    }
    net::write_all(client_fd, net::encode_frame(net::encode_done()));
  }
}

}  // namespace ytta::v0
