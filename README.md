# Synapse C++ client

This repo contains the C++ client for the [Synapse API](https://science.xyz/technologies/synapse). More information about the API can be found in the [docs](https://science.xyz/docs/d/synapse/index).

## Building & Installing

### Prerequisites

- [Git](https://git-scm.com/downloads)
- [CMake](https://cmake.org/download/)
- [Make](https://www.gnu.org/software/make/)

To build:

```sh
git submodule update --init

# Set up vcpkg -- you can also use a vcpkg elsewhere via VCPKG_ROOT
cd external/microsoft/vcpkg && ./bootstrap-vcpkg.sh && cd -
export VCPKG_ROOT="$(pwd)/external/microsoft/vcpkg}"

make all

# or
make clean
make configure
make build
```

To install

```sh
sudo make install
```

### vcpkg

This project is also available as a [vcpkg](https://vcpkg.io/en/) port.

To include this port in your project, add our [overlay port repo](https://github.com/sciencecorp/vcpkg) to your vcpkg configuration (see ['Overlay ports'](https://learn.microsoft.com/en-us/vcpkg/concepts/overlay-ports)).

Then you will be able to include it in your CMakelists:

```
synapse provides CMake targets:

  find_package(synapse CONFIG REQUIRED)
  target_link_libraries(main PRIVATE science::synapse)
```

## Writing clients

This library offers a C++ interface to the Synapse API.

### Device Operations

```cpp
#include <science/synapse/device.h>
#include <science/synapse/config.h>

// Connect to a device
synapse::Device device("192.168.1.100:50051");

// Get device info
synapse::DeviceInfo info;
device.info(&info);

// Configure, start, and stop
synapse::Config config;
// ... add nodes to config ...
device.configure(&config);
device.start();
device.stop();

// Query the device
synapse::QueryRequest req;
req.set_query_type(synapse::QueryType::kListTaps);
synapse::QueryResponse res;
device.query(req, &res);
```

### Tap Client (High-Throughput Data Streaming)

```cpp
#include <science/synapse/tap.h>

// Connect to a tap for streaming data
synapse::Tap tap("192.168.1.100:50051");

// List available taps
auto taps = tap.list_taps();

// Connect to a producer tap (read data from device)
tap.connect("broadband_tap");

// Read data
std::vector<uint8_t> data;
while (tap.read(&data).ok()) {
    // Process protobuf-encoded data (e.g., BroadbandFrame)
}

// Or read in batches for higher throughput
std::vector<std::vector<uint8_t>> batch;
size_t count = tap.read_batch(&batch, 100, 10);  // up to 100 messages, 10ms timeout
```

### Discovery

```cpp
#include <science/synapse/util/discover.h>

// Discover devices on the network (passive UDP listening)
std::vector<synapse::DeviceAdvertisement> devices;
synapse::discover(5000, &devices);  // 5 second timeout

// Or use callback-based discovery
synapse::discover_iter([](const synapse::DeviceAdvertisement& device) {
    std::cout << "Found: " << device.name << " at " << device.host << std::endl;
    return true;  // continue discovery
}, 10000);
```

See the [examples](./examples) for more details.
