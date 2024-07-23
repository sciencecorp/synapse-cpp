#include <fstream>
#include <iostream>
#include <memory>

#include "cli.h"
#include "science/synapse/device.h"
#include "science/synapse/nodes/electrical_broadband.h"
#include "science/synapse/nodes/optical_stimulation.h"
#include "science/synapse/nodes/stream_in.h"
#include "science/synapse/nodes/stream_out.h"
#include "science/synapse/util/discover.h"
#include "science/synapse/version.h"

#define CHECK_STATUS(s) \
  if (!s.ok()) { \
    std::cerr << " - error: " << s.message() << std::endl; \
    return static_cast<int>(s.code()); \
  } \
  std::cout << " - done." << std::endl;

using synapse::Device;
using synapse::DeviceAdvertisement;
using synapse::discover;
using synapse::SYNAPSE_VERSION;


auto discover(const CliArgs& args) -> int {
  std::vector<DeviceAdvertisement> discovered;
  std::string code = args.at("--auth_code").asString();
  unsigned int timeout = args.at("--timeout").asLong();

  std::cout << "Discovering devices..." << std::endl;
  auto status = discover(code, timeout, &discovered);
  CHECK_STATUS(status)

  for (const auto& device : discovered) {
    std::cout
      << device.host
      << ":" << device.port
      << "   " << device.capability
      << "   " << device.name
      << " (" << device.serial << ")" << std::endl;
  }

  return 0;
}

auto configure(const CliArgs& args) -> int {
  std::string filepath = args.at("--config").asString();

  if (filepath.empty()) {
    std::cerr << "Error: --config is required" << std::endl;
    return 1;
  }

  if (filepath[0] == '~') {
    const char* home = getenv("HOME");
    if (home) {
      filepath.replace(0, 1, home);
    }
  }

  // check if file exists
  std::ifstream file(filepath);
  if (!file.good()) {
    std::cerr << "Error: file not found: " << filepath << std::endl;
    return 1;
  }

  std::cerr << "Error: unimplemented" << std::endl;
  return 1;
}

auto stream(const CliArgs& args) -> int {
  bool read = args.at("read").asBool();
  bool write = args.at("write").asBool();
  if (!read && !write) {
    std::cerr << "Error: must specify either 'read' or 'write'" << std::endl;
    return 1;
  }

  const std::string uri = args.at("<uri>").asString();
  synapse::Device device(uri);
  synapse::Config config;
  science::Status s;

  if (read) {
    std::string group_str = args.count("--multicast") ? args.at("--multicast").asString() : "";
    std::optional<std::string> group = !group_str.empty() ? std::make_optional(group_str) : std::nullopt;
    auto stream_out = std::make_shared<synapse::StreamOut>(
      synapse::DataType::kBroadband,
      std::vector<uint32_t>{4},
      group
    );
    auto electrical_broadband = std::make_shared<synapse::ElectricalBroadband>(
      0, 30000, 8, 1, std::nullopt
    );
    config.add_node(stream_out);
    config.add_node(electrical_broadband);
    config.connect(electrical_broadband, stream_out);

    std::cout << "Configuring device..." << std::endl;
    s = device.configure(&config);
    if (!s.ok()) {
      std::cerr << "- error: failed to configure device: " << s.message() << std::endl;
      return 1;
    }
    std::cout << "- done." << std::endl;

    std::cout << "Starting device..." << std::endl;
    s = device.start();
    if (!s.ok()) {
      std::cerr << "- error: failed to start device" << std::endl;
      return 1;
    }
    std::cout << "- done." << std::endl;

    std::cout << "Reading from device @ " << uri << std::endl;
    while (true) {
      std::vector<std::byte> out;
      s = stream_out->read(&out);
      if (!s.ok()) {
        std::cerr << " - error: " << s.message() << std::endl;
        return static_cast<int>(s.code());
      }

      std::cout << " - read (" << out.size() << "): ";
      for (const auto& byte : out) {
        std::cout << std::to_integer<unsigned int>(byte) << " ";
      }
      std::cout << std::endl;
    }

    std::cout << "- done." << std::endl;
  } else {
    auto stream_in = std::make_shared<synapse::StreamIn>(
      synapse::DataType::kImage,
      std::vector<uint32_t>{4}
    );
    auto optical_stim = std::make_shared<synapse::OpticalStimulation>(0, 30000, 8, 1, std::nullopt);
    config.add_node(stream_in);
    config.add_node(optical_stim);
    config.connect(stream_in, optical_stim);

    std::cout << "Configuring device..." << std::endl;
    s = device.configure(&config);
    if (!s.ok()) {
      std::cerr << "- error: failed to configure device: " << s.message() << std::endl;
      return 1;
    }
    std::cout << "- done." << std::endl;

    std::cout << "Starting device..." << std::endl;
    s = device.start();
    if (!s.ok()) {
      std::cerr << "- error: failed to start device" << std::endl;
      return 1;
    }
    std::cout << "- done." << std::endl;

    std::cout << "Writing to device @ " << uri << std::endl;
    unsigned int value = 0;
    while (true) {
      std::vector<std::byte> in;
      for (int i = 0; i < 4; i++) {
        in.push_back(static_cast<std::byte>((value >> (8 * (3 - i))) & 0xFF));
      }

      auto data = stream_in->write(in);
      if (!s.ok()) {
        std::cerr << " - error: " << s.message() << std::endl;
        return static_cast<int>(s.code());
      }

      std::cout << " - wrote (" << in.size() << "): " << std::to_string(value) << std::endl;
      value++;
    }

    std::cout << "- done." << std::endl;
  }

  return 1;
}

int main(int argc, char** argv) {
  CliArgs args = cli(argc, argv, SYNAPSE_VERSION);

  if (args["discover"].asBool()) {
    return discover(args);
  }

  if (args["info"].asBool()) {
    const std::string uri = args["<uri>"].asString();
    Device device(uri);
    synapse::DeviceInfo info;

    std::cout << "Fetching device info..." << std::endl;
    auto s = device.info(&info);
    CHECK_STATUS(s)

    std::cout << info.DebugString() << std::endl;
    return static_cast<int>(s.code());
  }

  if (args["start"].asBool()) {
    const std::string uri = args["<uri>"].asString();
    Device device(uri);

    std::cout << "Starting device..." << std::endl;
    auto s = device.start();
    CHECK_STATUS(s)
    return static_cast<int>(s.code());
  }

  if (args["stop"].asBool()) {
    const std::string uri = args["<uri>"].asString();
    Device device(uri);

    std::cout << "Stopping device..." << std::endl;
    auto s = device.stop();
    CHECK_STATUS(s)
    return static_cast<int>(s.code());
  }

  if (args["stream"].asBool()) {
    return stream(args);
  }

  return 0;
}
