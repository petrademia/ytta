#include "tcp_order_gateway.hpp"

#include <stdexcept>
#include <utility>

namespace ytta::v0 {

TcpOrderGateway::TcpOrderGateway(std::string host, std::uint16_t port)
    : fd_(net::tcp_connect(host, port)), reader_(fd_) {}

TcpOrderGateway::~TcpOrderGateway() {
  if (fd_ >= 0) {
    try {
      goodbye();
    } catch (...) {
      // Best-effort teardown.
    }
    net::tcp_close(fd_);
    fd_ = -1;
  }
}

void TcpOrderGateway::goodbye() {
  if (fd_ < 0) {
    return;
  }
  net::write_all(fd_, net::encode_frame(net::encode_goodbye()));
}

std::vector<EngineEvent> TcpOrderGateway::new_order(const Order& order,
                                                    std::int64_t ts_ns) {
  net::WireOrder wire;
  wire.ts_ns = ts_ns;
  wire.cl_ord_id = order.cl_ord_id;
  wire.op = Op::New;
  wire.side = order.side;
  wire.price = order.price;
  wire.qty = order.qty;
  return submit(wire);
}

std::vector<EngineEvent> TcpOrderGateway::cancel(std::uint64_t cl_ord_id,
                                                 std::int64_t ts_ns) {
  net::WireOrder wire;
  wire.ts_ns = ts_ns;
  wire.cl_ord_id = cl_ord_id;
  wire.op = Op::Cancel;
  return submit(wire);
}

std::vector<EngineEvent> TcpOrderGateway::submit(const net::WireOrder& wire) {
  net::write_all(fd_, net::encode_frame(net::encode_order(wire)));
  std::vector<EngineEvent> out;
  for (;;) {
    const std::string payload = reader_.read_frame();
    if (net::is_done_json(payload)) {
      return out;
    }
    auto ev = net::parse_engine_event_json(payload);
    if (!ev) {
      throw std::runtime_error("client expected ack/fill/md/done from engine");
    }
    out.push_back(std::move(*ev));
  }
}

}  // namespace ytta::v0
