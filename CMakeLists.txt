cmake_minimum_required(VERSION 3.23.5)

SET(VCPKG_MANIFEST_FEATURES  "" CACHE STRING "vcpkg features")

option(SYNAPSE_BUILD_CLI "Build CLI application" OFF)
if(SYNAPSE_BUILD_CLI)
  list(APPEND VCPKG_MANIFEST_FEATURES "cli")
endif()

option(SYNAPSE_BUILD_TESTS "Build tests" OFF)
if(SYNAPSE_BUILD_TESTS)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

option(SYNAPSE_PACKAGE_PROTOS "Package proto files" OFF)
if(SYNAPSE_PACKAGE_PROTOS)
  list(APPEND VCPKG_MANIFEST_FEATURES "protos")
endif()

project(synapse VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wno-deprecated-declarations -Wno-unused-but-set-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_SHARED_LINKER_FLAGS "-pg")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(gRPC CONFIG REQUIRED CONFIG)
find_package(Protobuf REQUIRED CONFIG)
find_package(science-scipp REQUIRED CONFIG)

add_library(${PROJECT_NAME})

configure_file(include/science/synapse/version.h.in ${CMAKE_BINARY_DIR}/include/science/synapse/version.h @ONLY)

get_filename_component(PROTO_INCLUDE_DIR external/sciencecorp/synapse-api REALPATH)
set(PROTO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/sciencecorp/synapse-api")
set(PROTO_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/science/synapse")
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})
file(GLOB_RECURSE PROTOS ${PROTO_INCLUDE_DIR}/api/*.proto)

protobuf_generate(
  TARGET ${PROJECT_NAME}
  LANGUAGE cpp
  IMPORT_DIRS ${PROTO_INCLUDE_DIR}
  PROTOS ${PROTOS}
  PROTOC_OUT_DIR ${PROTO_OUT_DIR}
  OUT_VAR PROTO_SOURCES
)

protobuf_generate(
  TARGET ${PROJECT_NAME}
  LANGUAGE grpc
  IMPORT_DIRS ${PROTO_INCLUDE_DIR}
  PROTOS ${PROTO_INCLUDE_DIR}/api/synapse.proto
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
  PLUGIN protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
  PROTOC_OUT_DIR ${PROTO_OUT_DIR}
  OUT_VAR PROTO_SOURCES
)

file(GLOB_RECURSE SOURCES src/science/synapse/*.cpp)

target_sources(
  ${PROJECT_NAME}
  PRIVATE
  ${SOURCES}
)
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/science/synapse>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
  PRIVATE
  ${PROTO_OUT_DIR}
)
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
  gRPC::grpc++
  protobuf::libprotobuf
  science::scipp
)

# Installation and export settings
include(GNUInstallDirs)

if ("protos" IN_LIST VCPKG_MANIFEST_FEATURES)
  set(PROTO_FILES_DIR "${CMAKE_INSTALL_DATADIR}/science/synapse")

  install(
    DIRECTORY ${PROTO_INCLUDE_DIR}/
    DESTINATION ${PROTO_FILES_DIR}
  )
endif()

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h")

# Export targets
install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE science::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)


if ("cli" IN_LIST VCPKG_MANIFEST_FEATURES)
  find_package(docopt REQUIRED CONFIG)

  add_executable(cli ${CMAKE_CURRENT_SOURCE_DIR}/src/cli/main.cpp)

  file(GLOB_RECURSE SOURCES_CLI src/cli/*.cpp)
  target_sources(
    cli
    PRIVATE
    ${SOURCES_CLI}
  )

  target_include_directories(
    cli
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cli
  )

  target_link_libraries(
    cli
    PRIVATE
    docopt
    science::scipp
    ${PROJECT_NAME}
  )
endif()

if ("tests" IN_LIST VCPKG_MANIFEST_FEATURES)
endif()
