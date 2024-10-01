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
cd external/microsoft/vcpkg && ./bootstrap-vcpkg.sh && cd -
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
  target_link_libraries(main PRIVATE synapse::synapse)
```

## Writing clients

This library offers a C++ interface to the Synapse API:

```c++
#include <memory>

#include "science/scipp/status.h"
#include "science/synapse/channel.h"
#include "science/synapse/device.h"
#include "science/synapse/nodes/electrical_broadband.h"
#include "science/synapse/nodes/stream_out.h"

auto stream() -> int {
  const std::string uri = "127.0.0.1:647";
  std::string group = "239.0.0.1";
  synapse::Device device(uri);
  synapse::Config config;
  science::Status s;

  auto stream_out = std::make_shared<synapse::StreamOut>("out", group);

  std::vector<Ch> channels;
  for (unsigned int i = 0; i < 19; i++) {
    channels.push_back(synapse::Ch{
      .id = i,
      .electrode_id = i * 2,
      .reference_id = i * 2 + 1,
    });
  }
  auto electrical_broadband = std::make_shared<synapse::ElectricalBroadband>(
    1, channels, 12, 30000, 20.0, 500, 6000);
  s = config.add_node(stream_out);
  s = config.add_node(electrical_broadband);
  s = config.connect(electrical_broadband, stream_out);

  s = device.configure(&config);

  s = device.start();

  while (true) {
    std::vector<std::byte> out;
    s = stream_out->read(&out);
    // ...
  }

  return 1;
}

int main(int argc, char** argv) {
  stream();

  return 0;
}
```
