# Synapse C++ client

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
