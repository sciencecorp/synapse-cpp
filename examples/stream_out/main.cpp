#include <memory>

#include "science/scipp/status.h"
#include "science/synapse/channel.h"
#include "science/synapse/data.h"
#include "science/synapse/device.h"
#include "science/synapse/nodes/electrical_broadband.h"
#include "science/synapse/nodes/stream_out.h"

using synapse::Ch;
using synapse::ElectricalBroadbandData;
using synapse::SynapseData;

auto stream(const std::string& uri) -> int {
  std::string group = "239.0.0.117";
  synapse::Device device(uri);
  synapse::Config config;
  science::Status s;

  auto stream_out = std::make_shared<synapse::StreamOut>("out", group);

  std::vector<synapse::Ch> channels;
  for (unsigned int i = 0; i < 19; i++) {
    channels.push_back(synapse::Ch{
      .id = i,
      .electrode_id = i * 2,
      .reference_id = i * 2 + 1,
    });
  }
  auto electrical_broadband = std::make_shared<synapse::ElectricalBroadband>(
    1, channels, 16, 30000, 20.0, 500, 6000);
  s = config.add_node(stream_out);
  s = config.add_node(electrical_broadband);
  s = config.connect(electrical_broadband, stream_out);

  s = device.configure(&config);

  s = device.start();
  while (true) {
    SynapseData out;
    s = stream_out->read(&out);

    if (std::holds_alternative<ElectricalBroadbandData>(out)) {
      auto data = std::get<ElectricalBroadbandData>(out);
      std::cout << "recv electrical broadband data" << std::endl;
      std::cout << "  - to: " << data.t0 << std::endl;
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
    } else {
      std::cout << "data type unknown" << std::endl;
    }
  }

  return 1;
}

int main(int argc, char** argv) {
  stream("10.40.60.203:647");

  return 0;
}