#include "net/framing.hpp"
#include "net/tcp.hpp"

#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    ++failures;
  }
}

}  // namespace

int main() {
  using namespace ytta::m3;

  {
    const auto encoded = frame("hi");
    expect(encoded.size() == 6, "frame size");
    expect(encoded.size() >= 4 && encoded[0] == 0 && encoded[1] == 0 &&
               encoded[2] == 0 && encoded[3] == 2,
           "big-endian payload length");

    std::string buffer(encoded.begin(), encoded.end());
    std::string payload;
    const bool deframed = try_deframe(buffer, payload);
    expect(deframed, "deframe complete frame");
    expect(payload == "hi", "framing round-trip");
    if (deframed) {
      expect(buffer.empty(), "deframe consumes frame");
    }
  }

  {
    // Stub API takes a fixed port; prefer ephemeral (listen_localhost(0) + bound port getter) when implementing.
    constexpr std::uint16_t port = 19090;
    const std::string expected = "localhost round-trip";
    bool server_listened = false;
    bool server_received = false;

    std::thread server([&] {
      TcpSocket socket;
      server_listened = socket.listen_localhost(port);
      if (!server_listened) {
        return;
      }

      std::string buffer;
      std::string payload;
      std::array<std::uint8_t, 256> chunk{};
      const auto deadline =
          std::chrono::steady_clock::now() + std::chrono::seconds(1);
      while (!try_deframe(buffer, payload)) {
        if (std::chrono::steady_clock::now() >= deadline) {
          socket.close();
          return;
        }
        const ssize_t received = socket.recv_some(chunk.data(), chunk.size());
        if (received <= 0) {
          socket.close();
          return;
        }
        buffer.append(reinterpret_cast<const char*>(chunk.data()),
                      static_cast<std::size_t>(received));
      }
      server_received = payload == expected;
      socket.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    TcpSocket client;
    const bool client_connected = client.connect_localhost(port);
    const auto encoded = frame(expected);
    const bool client_sent =
        client_connected && client.send_all(encoded.data(), encoded.size());
    client.close();
    server.join();

    expect(server_listened, "listen on localhost");
    expect(client_connected, "connect on localhost");
    expect(client_sent, "send framed payload");
    expect(server_received, "receive and deframe payload");
  }

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "ytta_m3_tests ok\n";
  return EXIT_SUCCESS;
}
