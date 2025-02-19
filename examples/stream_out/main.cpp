#include <memory>

#include "science/scipp/status.h"
#include "science/synapse/channel.h"
#include "science/synapse/data.h"
#include "science/synapse/device.h"
#include "science/synapse/nodes/broadband_source.h"
#include "science/synapse/nodes/stream_out.h"

using synapse::BinnedSpiketrainData;
using synapse::Ch;
using synapse::Config;
using synapse::Device;
using synapse::DeviceInfo;
using synapse::Electrodes;
using synapse::ElectricalBroadbandData;
using synapse::NodeType;
using synapse::Signal;
using synapse::StreamOut;
using synapse::SynapseData;


auto stream_new(Device& device, std::shared_ptr<StreamOut>* stream_out_ptr) -> science::Status {
  if (stream_out_ptr == nullptr) {
    return { science::StatusCode::kInvalidArgument, "stream out pointer is null" };
  }

  std::string group = "224.0.0.10";
  science::Status s;
  DeviceInfo info;
  s = device.info(&info);
  if (!s.ok()) return s;

  Signal signal{
    Electrodes{
      .channels = {},
      .low_cutoff_hz = 500,
      .high_cutoff_hz = 6000
    }
  };
  auto& electrodes = std::get<Electrodes>(signal.signal);
  electrodes.channels.reserve(19);
  for (unsigned int i = 0; i < 19; i++) {
    electrodes.channels.push_back(Ch{
      .id = i,
      .electrode_id = i * 2,
      .reference_id = i * 2 + 1
    });
  }

  Config config;
  auto broadband_source = std::make_shared<synapse::BroadbandSource>(1, 16, 30000, 20.0, signal);
  *stream_out_ptr = std::make_shared<synapse::StreamOut>("out", group);

  s = config.add_node(broadband_source);
  if (!s.ok()) return s;

  s = config.add_node(*stream_out_ptr);
  if (!s.ok()) return s;
  
  s = config.connect(broadband_source, *stream_out_ptr);
  if (!s.ok()) return s;

  s = device.configure(&config);
  if (!s.ok()) return s;

  s = device.start();

  return s;
}

auto stream_existing(Device& device, std::shared_ptr<StreamOut>* stream_out_ptr) -> science::Status {
  if (stream_out_ptr == nullptr) {
    return { science::StatusCode::kInvalidArgument, "stream out pointer is null" };
  }

  science::Status s;

  DeviceInfo info;
  s = device.info(&info);
  if (!s.ok()) return s;

  uint32_t stream_out_id = 0; // default id
  std::string group;
  const auto& nodes = info.configuration().nodes();
  for (const auto& node : nodes) {
    if (node.type() == NodeType::kStreamOut) {
      stream_out_id = node.id();
      group = node.stream_out().multicast_group();
      break;
    }
  }

  if (stream_out_id == 0) {
    return { science::StatusCode::kNotFound, "no stream out node found" };
  }

  std::cout << "found stream out node with id " << stream_out_id << " and group " << group << std::endl;

  *stream_out_ptr = std::make_shared<synapse::StreamOut>("out", group);

  Config config;
  s = config.add_node(*stream_out_ptr, stream_out_id);
  if (!s.ok()) return s;

  s = config.set_device(&device);
  if (!s.ok()) return s;

  return s;
} 

auto stream(const std::string& uri, bool configure) -> int {
  synapse::Device device(uri);
  science::Status s;

  std::shared_ptr<synapse::StreamOut> stream_out;
  if (configure) {
    s = stream_new(device, &stream_out);
    if (!s.ok()) {
      std::cout << "error configuring stream out node: ("
        << static_cast<int>(s.code()) << ") " << s.message() << std::endl;
      return 1;
    }

  } else {
    s = stream_existing(device, &stream_out);
    if (!s.ok()) {
      std::cout << "error getting existing stream out node: ("
        << static_cast<int>(s.code()) << ") " << s.message() << std::endl;
      return 1;
    }
  }

  if (stream_out == nullptr) {
    std::cout << "stream out node not initialized" << std::endl;
    return 1;
  }

  while (true) {
    SynapseData out;
    s = stream_out->read(&out);
    if (s.code() == science::StatusCode::kUnavailable) {
      continue;
    }

    if (!s.ok()) {
      std::cout << "error reading from stream out node: ("
        << static_cast<int>(s.code()) << ") " << s.message() << std::endl;
      continue;
    }

    if (std::holds_alternative<ElectricalBroadbandData>(out)) {
      auto data = std::get<ElectricalBroadbandData>(out);
      std::cout << "recv electrical broadband data" << std::endl;
      std::cout << "  - t0: " << data.t0 << std::endl;
      std::cout << "  - bit_width: " << data.bit_width << std::endl;
      std::cout << "  - is_signed: " << data.is_signed << std::endl;
      std::cout << "  - sample_rate: " << data.sample_rate << std::endl;
      std::cout << "  - n_channels: " << data.channels.size() << std::endl;

      for (const auto& c : data.channels) {
        size_t n_samples = c.channel_data.size();
        std::cout << "   - channel [" << c.channel_id << "]" << std::endl;

        for (size_t i = 0; i < std::min(n_samples, size_t(10)); i++) {
          std::cout << "     - sample [" << i << "]: " << c.channel_data[i] << std::endl;
        }
      }

    } else if (std::holds_alternative<BinnedSpiketrainData>(out)) {
      auto data = std::get<BinnedSpiketrainData>(out);
      size_t n_spiking_channels = 0;
      std::stringstream ss;
      for (size_t i = 0; i < data.spike_counts.size(); i++) {
        if (data.spike_counts[i] > 0) {
          if (n_spiking_channels > 0) {
            ss << ", ";
          }
          ss << "[" << i << "]: " << static_cast<unsigned int>(data.spike_counts[i]);
          n_spiking_channels++;
        }
      }
      if (n_spiking_channels > 0) {
        std::cout << " - received " << n_spiking_channels << " spiking channels:" << std::endl;
        std::cout << ss.str() << std::endl;
      }
    } else {
      std::cout << "data type unknown" << std::endl;
    }
  }

  return 1;
}

int main(int argc, char** argv) {
  std::string uri = "192.168.0.1:647";
  stream(uri, false);

  return 0;
}
