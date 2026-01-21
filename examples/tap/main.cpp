#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "science/synapse/tap.h"
#include "science/synapse/api/datatype.pb.h"

void print_usage(const char* program_name) {
  std::cout << "Usage: " << program_name << " <device_uri> [tap_name]" << std::endl;
  std::cout << "  device_uri: The URI of the Synapse device (e.g., 192.168.1.100:647)" << std::endl;
  std::cout << "  tap_name: Optional tap name to connect to (if not provided, lists available taps)" << std::endl;
}

void list_taps(synapse::Tap& tap) {
  auto taps = tap.list_taps();

  if (taps.empty()) {
    std::cout << "No taps available on device" << std::endl;
    return;
  }

  std::cout << "Available taps:" << std::endl;
  for (const auto& t : taps) {
    std::cout << "  - " << t.name() << std::endl;
    std::cout << "      endpoint: " << t.endpoint() << std::endl;
    std::cout << "      message_type: " << t.message_type() << std::endl;
    std::cout << "      type: ";
    switch (t.tap_type()) {
      case synapse::TapType::TAP_TYPE_PRODUCER:
        std::cout << "producer (read data from device)" << std::endl;
        break;
      case synapse::TapType::TAP_TYPE_CONSUMER:
        std::cout << "consumer (send data to device)" << std::endl;
        break;
      default:
        std::cout << "unspecified" << std::endl;
    }
  }
}

void stream_tap(synapse::Tap& tap, const std::string& tap_name) {
  auto status = tap.connect(tap_name);
  if (!status.ok()) {
    std::cerr << "Failed to connect to tap: " << status.message() << std::endl;
    return;
  }

  auto connected = tap.connected_tap();
  if (!connected) {
    std::cerr << "Not connected" << std::endl;
    return;
  }

  std::cout << "Connected to tap: " << connected->name() << std::endl;
  std::cout << "Message type: " << connected->message_type() << std::endl;

  // Check if this is a producer tap (we read from it)
  if (connected->tap_type() == synapse::TapType::TAP_TYPE_CONSUMER) {
    std::cerr << "This is a consumer tap. This example only reads from producer taps." << std::endl;
    tap.disconnect();
    return;
  }

  std::cout << "Streaming data (Ctrl+C to stop)..." << std::endl;

  uint64_t message_count = 0;
  auto start_time = std::chrono::steady_clock::now();

  while (true) {
    std::vector<uint8_t> data;
    auto status = tap.read(&data, 1000);

    if (!status.ok()) {
      if (status.code() == science::StatusCode::kDeadlineExceeded) {
        // Timeout, just continue
        continue;
      }
      std::cerr << "Error reading: " << status.message() << std::endl;
      break;
    }

    message_count++;

    // Print stats every 1000 messages
    if (message_count % 1000 == 0) {
      auto now = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
      double rate = (elapsed > 0) ? static_cast<double>(message_count) / elapsed : 0;
      std::cout << "Received " << message_count << " messages (" << rate << " msg/s), "
                << "last message size: " << data.size() << " bytes" << std::endl;
    }

    // Try to parse as BroadbandFrame if the message type matches
    if (connected->message_type() == "synapse.BroadbandFrame" && message_count == 1) {
      synapse::BroadbandFrame frame;
      if (frame.ParseFromArray(data.data(), static_cast<int>(data.size()))) {
        std::cout << "First BroadbandFrame: timestamp=" << frame.timestamp_ns()
                  << ", seq=" << frame.sequence_number()
                  << ", samples=" << frame.frame_data_size()
                  << ", sample_rate=" << frame.sample_rate_hz() << " Hz" << std::endl;
      }
    }
  }

  tap.disconnect();
  std::cout << "Disconnected. Total messages: " << message_count << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  std::string device_uri = argv[1];
  synapse::Tap tap(device_uri);

  if (argc == 2) {
    // Just list taps
    list_taps(tap);
  } else {
    // Connect to specified tap and stream
    std::string tap_name = argv[2];
    stream_tap(tap, tap_name);
  }

  return 0;
}
