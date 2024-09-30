# Synapse C++ client

This repo contains the C++ client for the [Synapse API](https://science.xyz/docs/d/synapse/index).

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
