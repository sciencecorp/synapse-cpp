# Changelog

## [1.0.1](https://github.com/sciencecorp/synapse-cpp/compare/v1.0.0...v1.0.1) (2025-05-06)


### Bug Fixes

* fix missing imports ([b3ca993](https://github.com/sciencecorp/synapse-cpp/commit/b3ca9935a212c81598569a995268f53c21ccb67b))

## [1.0.0](https://github.com/sciencecorp/synapse-cpp/compare/v0.2.0...v1.0.0) (2025-03-13)


### ⚠ BREAKING CHANGES

* support UDP unicast in StreamOut node, remove UDP multicast

### Features

* add packet monitoring 'stats' example ([#20](https://github.com/sciencecorp/synapse-cpp/issues/20)) ([a0e69c7](https://github.com/sciencecorp/synapse-cpp/commit/a0e69c7a6a424546977084a10cf077d1fff9b531))
* switch StreamOut node to UDP unicast; update synapse-api ([#18](https://github.com/sciencecorp/synapse-cpp/issues/18)) ([6c240dc](https://github.com/sciencecorp/synapse-cpp/commit/6c240dca7f5e2a5b5a1acbd9c623fb8dba619d2f))

## [0.2.0](https://github.com/sciencecorp/synapse-cpp/compare/v0.1.0...v0.2.0) (2025-03-13)


### ⚠ BREAKING CHANGES

* **broadband-source, spike-source:** add BroadbandSource, SpikeSource nodes, remove ElectricalBroadband and OpticalBroadband nodes

### Features

* **broadband-source, spike-source:** add BroadbandSource, SpikeSource nodes, remove ElectricalBroadband and OpticalBroadband ([#12](https://github.com/sciencecorp/synapse-cpp/issues/12)) ([5171f59](https://github.com/sciencecorp/synapse-cpp/commit/


### Bug Fixes

* reuse multicast addr, ports ([#13](https://github.com/sciencecorp/synapse-cpp/issues/13)) ([423ebc5](https://github.com/sciencecorp/synapse-cpp/commit/423ebc5ffd5c5bc7b6b8fa640afdca27031b14ab))
