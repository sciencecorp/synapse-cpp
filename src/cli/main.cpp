#include <fstream>
#include <iostream>
#include <memory>

#include "cli.h"
#include "science/synapse/version.h"
#include "science/synapse/device.h"
#include "science/synapse/util/discover.h"


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
  if (!status.ok()) {
    std::cerr << "Error: " << status.message() << std::endl;
    return 1;
  }

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

int main(int argc, char** argv) {
  CliArgs args = cli(argc, argv, SYNAPSE_VERSION);

  if (args["discover"].asBool()) {
    return discover(args);
  }

  if (args["info"].asBool()) {
    const std::string uri = args["<uri>"].asString();
    Device device(uri);
    auto res = device.info();
    if (!res.has_value()) {
      return 1;
    }

    std::cout << res->DebugString() << std::endl;
    return 0;
  }

  if (args["start"].asBool()) {
    const std::string uri = args["<uri>"].asString();
    Device device(uri);
    auto res = device.start();
    return res;
  }

  if (args["stop"].asBool()) {
    const std::string uri = args["<uri>"].asString();
    Device device(uri);
    auto res = device.stop();
    return res;
  }

  return 0;
}
