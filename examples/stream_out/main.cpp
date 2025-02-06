#include <memory>

#include "science/scipp/status.h"
#include "science/synapse/channel.h"
#include "science/synapse/data.h"
#include "science/synapse/device.h"
#include "science/synapse/nodes/broadband_source.h"
#include "science/synapse/nodes/stream_out.h"

using synapse::BinnedSpiketrainData;
using synapse::Ch;
using synapse::BroadbandSourceData;
using synapse::SynapseData;

auto stream(const std::string& uri, const std::string& group, bool configure) -> int {
  synapse::Device device(uri);
  synapse::Config config;
  science::Status s;

  synapse::DeviceInfo info;
  s = device.info(&info);
  if (!s.ok()) {
    std::cout << "error getting device info: (" << static_cast<int>(s.code()) << ") " << s.message() << std::endl;
    return 1;
  }
  std::cout << "device info: " << info.DebugString() << std::endl;

  std::shared_ptr<synapse::StreamOut> stream_out;
  if (configure) {
    // Configure the device to stream out electrical broadband data
    std::vector<synapse::Ch> channels;
    for (unsigned int i = 0; i < 19; i++) {
      channels.push_back(synapse::Ch{
        .id = i,
        .electrode_id = i * 2,
        .reference_id = i * 2 + 1,
      });
    }
    auto broadband_source = std::make_shared<synapse::BroadbandSource>(
        1, channels, 16, 30000, 20.0, 500, 6000);
    s = config.add_node(stream_out);
    s = config.add_node(broadband_source);
    s = config.connect(broadband_source, stream_out);

    s = device.configure(&config);

    s = device.start();

  } else {
    // Read data from a device that is already configured
    stream_out = std::make_shared<synapse::StreamOut>("out", group);
    s = config.add_node(stream_out);
    s = config.set_device(&device);
  }

  while (true) {
    SynapseData out;
    s = stream_out->read(&out);

    if (!s.ok()) {
      std::cout << "error reading from stream out node: ("
        << static_cast<int>(s.code()) << ") " << s.message() << std::endl;
      continue;
    }

    if (std::holds_alternative<BroadbandSourceData>(out)) {
      auto data = std::get<BroadbandSourceData>(out);
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
  std::string group = "239.0.0.1";
  stream(uri, group, false);

  return 0;
}
