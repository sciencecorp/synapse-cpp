# Changelog

## [0.2.0](https://github.com/sciencecorp/synapse-cpp/compare/v0.1.0...v0.2.0) (2025-03-13)


### ⚠ BREAKING CHANGES

* **broadband-source, spike-source:** add BroadbandSource, SpikeSource nodes, remove ElectricalBroadband and OpticalBroadband nodes

### Features

* add CMake options ([aadbd9e](https://github.com/sciencecorp/synapse-cpp/commit/aadbd9e84002e075eb7e3182dc44af893b908075))
* add Config::from_proto, add ElectricalStim, OpticalBroadband, SpikeDetect, SpectralFilter nodes ([31d7a9a](https://github.com/sciencecorp/synapse-cpp/commit/31d7a9adf2aaef100074e7df7678a49ea842cc47))
* add Config::nodes(), Config::connections() ([c799e10](https://github.com/sciencecorp/synapse-cpp/commit/c799e10dae16f643e7abbd90538b7428fb7ecd0c))
* add handle_status_response to device info() to retrieve socket information ([fe8aca4](https://github.com/sciencecorp/synapse-cpp/commit/fe8aca4a159f6508abeb0a2e5025b5c72fda7224))
* add microsoft/vcpkg submodule ([9df85f4](https://github.com/sciencecorp/synapse-cpp/commit/9df85f4a97819a56c60a8646016ef9c3ede491e1))
* add optional deadline to device calls, allow specifying of node id ([#11](https://github.com/sciencecorp/synapse-cpp/issues/11)) ([f8d947e](https://github.com/sciencecorp/synapse-cpp/commit/f8d947e5e873a0b61693ff123cb0761e3c091949))
* **broadband-source, spike-source:** add BroadbandSource, SpikeSource nodes, remove ElectricalBroadband and OpticalBroadband ([#12](https://github.com/sciencecorp/synapse-cpp/issues/12)) ([5171f59](https://github.com/sciencecorp/synapse-cpp/commit/5171f599f1b33d55f3b402ddc4c7fdd467287913))
* package protos for consumers ([18bd277](https://github.com/sciencecorp/synapse-cpp/commit/18bd27767aa0c8a15d1f450ebdd09642bd60a0ea))
* parse StreamOut data with NDTP ([#6](https://github.com/sciencecorp/synapse-cpp/issues/6)) ([683746d](https://github.com/sciencecorp/synapse-cpp/commit/683746db8af4138e00c995b7cb6a03a5b7a41feb))
* update StreamIn, StreamOut node config ([75f3bf1](https://github.com/sciencecorp/synapse-cpp/commit/75f3bf13f959cdb44fdb066995955a2a5ebf0cc2))
* use Status returns, impl Stream In, Out nodes ([d205038](https://github.com/sciencecorp/synapse-cpp/commit/d205038e1dc112a16ffe5393a40615d4e7cb78dd))


### Bug Fixes

* do not block on StreamOut read(), return StatusCode kUnavailable if no data available ([#9](https://github.com/sciencecorp/synapse-cpp/issues/9)) ([fd21413](https://github.com/sciencecorp/synapse-cpp/commit/fd214137b444a3b52ae121cb9bf2c2bad023911c))
* fix install include dirs ([8a5d8b3](https://github.com/sciencecorp/synapse-cpp/commit/8a5d8b3067b39b7444d245e822d7ff58f86241d7))
* fix missing udp_node header ([#3](https://github.com/sciencecorp/synapse-cpp/issues/3)) ([96ee7d1](https://github.com/sciencecorp/synapse-cpp/commit/96ee7d1950ac11559a103c6065ac3e97f9bfae3f))
* fixup .gitignore, Makefile for install ([ca68903](https://github.com/sciencecorp/synapse-cpp/commit/ca689034309d82b0f2efda74706470e458153ae7))
* replace 'shape' field in stream-in node ([#4](https://github.com/sciencecorp/synapse-cpp/issues/4)) ([1037e7c](https://github.com/sciencecorp/synapse-cpp/commit/1037e7c17473a90a969bfbcac669f130e27adb7d))
* reuse multicast addr, ports ([#13](https://github.com/sciencecorp/synapse-cpp/issues/13)) ([423ebc5](https://github.com/sciencecorp/synapse-cpp/commit/423ebc5ffd5c5bc7b6b8fa640afdca27031b14ab))
