.PHONY: all
all: clean configure build

.PHONY: build
build:
	cmake --build build

.PHONY: clean
clean:
	rm -rf build dist include/science/synapse/api/*

.PHONY: configure
configure:
	cmake --preset=static -DCMAKE_BUILD_TYPE=Debug -DVCPKG_MANIFEST_FEATURES=${VCPKG_MANIFEST_FEATURES}

.PHONY: cli
cli:
	./build/bin/synapse-cli

