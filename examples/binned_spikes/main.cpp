#include <memory>
#include <fstream>
#include <iomanip>

#include "science/scipp/status.h"
#include "science/synapse/channel.h"
#include "science/synapse/data.h"
#include "science/synapse/device.h"
#include "science/synapse/nodes/electrical_broadband.h"
#include "science/synapse/nodes/stream_out.h"
#include "science/synapse/nodes/spike_detect.h"
#include "science/synapse/nodes/spectral_filter.h"

using synapse::BinnedSpiketrainData;
using synapse::Ch;
using synapse::ElectricalBroadbandData;
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

  std::shared_ptr<synapse::StreamOut> stream_out{};
  if (configure) {
    stream_out = std::make_shared<synapse::StreamOut>("out", group);
    std::vector<synapse::Ch> channels;
    for (unsigned int i = 1; i < 97; i++) {
      channels.push_back(synapse::Ch{
        .id = i,
        .electrode_id = i * 2,
        .reference_id = i * 2 + 1,
      });
    }
    auto electrical_broadband = std::make_shared<synapse::ElectricalBroadband>(
        1, channels, 16, 30000, 20.0, 500, 6000);
    s = config.add_node(electrical_broadband);

    auto spectral_filter = std::make_shared<synapse::SpectralFilter>(
        synapse::SpectralFilterMethod::kBandPass, 500, 6000);

    s = config.add_node(spectral_filter);
    s = config.connect(electrical_broadband, spectral_filter);

    auto channel_mask = synapse::ChannelMask(0); // TODO(kevinc): should this be number of channels?
    auto spike_detect = std::make_shared<synapse::SpikeDetect>(
        synapse::SpikeDetectConfig_SpikeDetectMode_kThreshold, 100, channel_mask, true, 10);
    s = config.add_node(spike_detect);
    s = config.connect(spectral_filter, spike_detect);

    s = config.add_node(stream_out);
    s = config.connect(spike_detect, stream_out);

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

    if (std::holds_alternative<ElectricalBroadbandData>(out)) {
      auto data = std::get<ElectricalBroadbandData>(out);
      std::cout << "recv electrical broadband data" << std::endl;
      std::cout << "  - t0: " << data.t0 << std::endl;
      std::cout << "  - bit_width: " << data.bit_width << std::endl;
      std::cout << "  - is_signed: " << data.is_signed << std::endl;
      std::cout << "  - sample_rate: " << data.sample_rate << std::endl;
      std::cout << "  - n_channels: " << data.channels.size() << std::endl;

      // Open a file for writing
      std::ofstream outfile("electrical_broadband_data.txt", std::ios::app);
      outfile << std::fixed << std::setprecision(6);  // Set precision for timestamp

      // Write timestamp
      outfile << "[" << data.t0 << ", [";

      bool first_channel = true;
      for (const auto& c : data.channels) {
        if (!first_channel) {
          outfile << ", ";
        }
        first_channel = false;

        // Write channel_id and samples
        outfile << "[" << c.channel_id << ", [";

        size_t n_samples = c.channel_data.size();
        for (size_t i = 0; i < n_samples; i++) {
          if (i > 0) {
            outfile << ", ";
          }
          outfile << static_cast<int64_t>(reinterpret_cast<const int64_t&>(c.channel_data[i]));
        }
        outfile << "]]";

        // Print first 10 samples to console (for debugging)
        std::cout << "   - channel [" << c.channel_id << "]" << std::endl;
        for (size_t i = 0; i < std::min(n_samples, size_t(10)); i++) {
          std::cout << "     - sample [" << i << "]: " << static_cast<int64_t>(reinterpret_cast<const int64_t&>(c.channel_data[i])) << std::endl;
        }
      }

      outfile << "]]\n";  // Close the outer brackets and add a newline
      outfile.close();
    } else if (std::holds_alternative<BinnedSpiketrainData>(out)) {
      auto data = std::get<BinnedSpiketrainData>(out);

      // write spikes to file
      // format: [timestamp, bin_size_ms, [spike_count_ch_1, spike_count_ch_2, ...]]
      std::string filename = "spikes_out.txt";
      std::ofstream spikes_file(filename, std::ios::app);
      spikes_file << std::fixed << std::setprecision(std::numeric_limits<uint64_t>::digits10);

      spikes_file << "[" << data.t0 << ", " << 10 << ", [";

      for (size_t i = 0; i < data.spike_counts.size(); i++) {
        if (i > 0) {
          spikes_file << ", ";
        }
        spikes_file << static_cast<unsigned int>(data.spike_counts[i]);
      }
      spikes_file << "]]\n";

      spikes_file.close();

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
  // std::string group = "239.0.0.1";
  std::string uri = "10.20.60.204:647";
  std::string group = "224.0.0.246";
  stream(uri, group, true);

  return 0;
}
