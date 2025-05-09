cmake_minimum_required(VERSION 3.23.5)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" SYNAPSE_CPP_VERSION)
string(STRIP "${SYNAPSE_CPP_VERSION}" SYNAPSE_CPP_VERSION)
project(synapse VERSION ${SYNAPSE_CPP_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED CONFIG)
find_package(science-libndtp REQUIRED CONFIG)
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
  OUT_VAR GRPC_SOURCES
)

file(GLOB_RECURSE SOURCES src/science/synapse/*.cpp)
target_sources(
  ${PROJECT_NAME}
  PRIVATE
  ${SOURCES}
  ${PROTO_SOURCES}
  ${GRPC_SOURCES}
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/science/synapse>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
  $<INSTALL_INTERFACE:include/science/synapse>
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
  gRPC::grpc++
  protobuf::libprotobuf
  science::libndtp
  science::scipp
)

include(GNUInstallDirs)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

install(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE science::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

if ("examples" IN_LIST VCPKG_MANIFEST_FEATURES)
  add_executable(stream_out examples/stream_out/main.cpp)
  file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/examples/stream_out/*.cpp")
  target_sources(stream_out PRIVATE ${TEST_SOURCES})
  target_include_directories(stream_out PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/examples)
  target_link_libraries(stream_out PRIVATE ${PROJECT_NAME} science::scipp)
  set_target_properties(stream_out PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
  )

  add_executable(stats examples/stats/main.cpp)
  file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/examples/stats/*.cpp")
  target_sources(stats PRIVATE ${TEST_SOURCES})
  target_include_directories(stats PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/examples)
  target_link_libraries(stats PRIVATE ${PROJECT_NAME} science::scipp)
  set_target_properties(stats PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
  )
endif()

if ("tests" IN_LIST VCPKG_MANIFEST_FEATURES)
  enable_testing()
  
  find_package(GTest REQUIRED)
  
  file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
  
  add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})
  
  target_link_libraries(${PROJECT_NAME}_tests
    PRIVATE
    ${PROJECT_NAME}
    GTest::GTest
    GTest::Main
  )
  
  target_include_directories(${PROJECT_NAME}_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/test
  )
  
  include(GoogleTest)
  gtest_discover_tests(${PROJECT_NAME}_tests)
endif()
