#include <memory>
#include <chrono>

#include "science/scipp/status.h"
#include "science/synapse/channel.h"
#include "science/synapse/data.h"
#include "science/synapse/device.h"
#include "science/synapse/nodes/broadband_source.h"
#include "science/synapse/nodes/stream_out.h"
#include "packet_monitoring.h"

using science::libndtp::NDTPHeader;
using synapse::Ch;
using synapse::Config;
using synapse::Device;
using synapse::DeviceInfo;
using synapse::Electrodes;
using synapse::NodeType;
using synapse::Signal;
using synapse::StreamOut;
using synapse::SynapseData;
using synapse::NodeConfig;
using synapse::Node;

auto configure_stream(Device& device, std::shared_ptr<StreamOut>* stream_out_ptr) -> science::Status {
  const uint32_t N_CHANNELS = 32;  // Using more channels for stats testing
  if (stream_out_ptr == nullptr) {
    return { science::StatusCode::kInvalidArgument, "stream out pointer is null" };
  }

  science::Status s;
  DeviceInfo info;
  s = device.info(&info);
  if (!s.ok()) return s;

  // Configure signal with more channels for statistics gathering
  Signal signal{
    Electrodes{
      .channels = {},
      .low_cutoff_hz = 500,
      .high_cutoff_hz = 6000
    }
  };
  auto& electrodes = std::get<Electrodes>(signal.signal);
  electrodes.channels.reserve(N_CHANNELS);
  for (unsigned int i = 0; i < N_CHANNELS; i++) {
    electrodes.channels.push_back(Ch{
      .id = i,
      .electrode_id = i * 2,
      .reference_id = i * 2 + 1
    });
  }

  Config config;
  auto broadband_source = std::make_shared<synapse::BroadbandSource>(1, 16, 30000, 20.0, signal);
  
  NodeConfig stream_out_config;
  auto* stream_out_proto = stream_out_config.mutable_stream_out();
  stream_out_proto->set_multicast_group("224.0.0.115");

  std::shared_ptr<Node> stream_out_node;
  s = StreamOut::from_proto(stream_out_config, &stream_out_node);
  if (!s.ok()) return s;
  
  *stream_out_ptr = std::dynamic_pointer_cast<StreamOut>(stream_out_node);
  if (!*stream_out_ptr) {
    return { science::StatusCode::kInternal, "failed to cast stream out node" };
  }

  s = config.add_node(broadband_source);
  if (!s.ok()) return s;

  s = config.add_node(*stream_out_ptr);
  if (!s.ok()) return s;
  
  s = config.connect(broadband_source, *stream_out_ptr);
  if (!s.ok()) return s;

  s = device.configure(&config);
  if (!s.ok()) return s;

  std::cout << "Configured device..." << std::endl;

  s = device.start();
  if (!s.ok()) return s;
  
  std::cout << "Started device..." << std::endl;
  return s;
}

auto stream(const std::string& uri) -> int {
  synapse::Device device(uri);
  science::Status s;

  std::shared_ptr<synapse::StreamOut> stream_out;
  s = configure_stream(device, &stream_out);
  if (!s.ok()) {
    std::cout << "error configuring stream: ("
      << static_cast<int>(s.code()) << ") " << s.message() << std::endl;
    return 1;
  }

  if (stream_out == nullptr) {
    std::cout << "stream out node not initialized" << std::endl;
    return 1;
  }

  std::cout << "Monitoring packet statistics..." << std::endl;
  
  // Initialize packet monitor
  PacketMonitor monitor;
  monitor.start_monitoring();
  auto last_stats_time = std::chrono::steady_clock::now();

  while (true) {
    size_t bytes_read;
    NDTPHeader header;
    SynapseData out;
    s = stream_out->read(&out, &header, &bytes_read);
    if (s.code() == science::StatusCode::kUnavailable) {
      continue;
    }

    if (!s.ok()) {
      std::cout << "error reading from stream out node: ("
        << static_cast<int>(s.code()) << ") " << s.message() << std::endl;
      continue;
    }

    monitor.process_packet(header.seq_number, bytes_read);

    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_stats_time).count() >= 1) {
      monitor.print_stats();
      last_stats_time = now;
    }
  }

  return 0;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <uri>" << std::endl;
    std::cout << "  uri: device URI (e.g., 192.168.0.1:647)" << std::endl;
    return 1;
  }

  std::string uri = argv[1];
  return stream(uri);
}
