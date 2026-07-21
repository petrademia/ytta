#include "net/framing.hpp"
#include "net/tcp.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <variant>

namespace {

int failures = 0;

void expect(bool cond, const char* msg) {
  if (!cond) {
    std::cerr << "FAIL: " << msg << "\n";
    ++failures;
  }
}

}  // namespace

int main() {
  using namespace ytta::v0;
  using namespace ytta::v0::net;

  {
    const std::string payload = "{\"type\":\"done\"}";
    const std::string frame = encode_frame(payload);
    expect(frame.size() == 4 + payload.size(), "frame size");
    auto decoded = try_decode_frame(frame);
    expect(decoded.has_value(), "decode complete");
    expect(decoded->first == payload, "payload roundtrip");
    expect(decoded->second == frame.size(), "consumed all");
  }

  {
    const char incomplete_bytes[] = {'\0', '\0', '\0', '\x05', 'a', 'b', 'c'};
    auto incomplete =
        try_decode_frame(std::string(incomplete_bytes, sizeof(incomplete_bytes)));
    expect(!incomplete.has_value(), "incomplete frame");
  }

  {
    WireOrder o;
    o.ts_ns = 1001;
    o.cl_ord_id = 1;
    o.op = Op::New;
    o.side = Side::Buy;
    o.price = 100;
    o.qty = 5;
    const auto json = encode_order(o);
    auto parsed = parse_order_json(json);
    expect(parsed.has_value(), "parse order");
    expect(parsed->cl_ord_id == 1 && parsed->price == 100 && parsed->qty == 5,
           "order fields");
    expect(is_done_json(encode_done()), "done");
    expect(is_goodbye_json(encode_goodbye()), "goodbye");
  }

  {
    AckEvent ack{1000, 1, AckStatus::Accepted, {}};
    const auto json = encode_engine_event(EngineEvent{ack});
    auto ev = parse_engine_event_json(json);
    expect(ev.has_value() && std::holds_alternative<AckEvent>(*ev), "ack event");
  }

  {
    const int listen_fd = tcp_listen("127.0.0.1", 0);
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    expect(getsockname(listen_fd, reinterpret_cast<sockaddr*>(&addr), &len) == 0,
           "getsockname");
    const std::uint16_t port = ntohs(addr.sin_port);

    const int client_fd = tcp_connect("127.0.0.1", port);
    const int server_fd = tcp_accept(listen_fd);

    const std::string msg = encode_frame(encode_done());
    write_all(client_fd, msg);
    FrameReader reader(server_fd);
    const std::string got = reader.read_frame();
    expect(is_done_json(got), "tcp framed done");

    tcp_close(client_fd);
    tcp_close(server_fd);
    tcp_close(listen_fd);
  }

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "ytta_v0_net_tests ok\n";
  return EXIT_SUCCESS;
}
