#include "stats/packet_monitoring.h"
#include <iomanip>
#include <sstream>

PacketMonitor::PacketMonitor()
    : packet_count_(0)
    , last_seq_number_(0)
    , dropped_packets_(0)
    , out_of_order_packets_(0)
    , bytes_received_(0)
    , bytes_received_in_interval_(0)
    , last_jitter_(0)
    , avg_jitter_(0) {}

void PacketMonitor::start_monitoring() {
    start_time_ = std::chrono::steady_clock::now();
    last_stats_time_ = start_time_;
    last_bandwidth_time_ = start_time_;
}

bool PacketMonitor::process_packet(uint16_t seq_number, size_t bytes_read) {
  auto now = std::chrono::steady_clock::now();

  if (packet_count_ == 0) {
    first_packet_time_ = now;
    last_packet_time_ = now;
    auto elapsed = std::chrono::duration<double>(now - start_time_).count();
    std::cout << "First packet received after " << std::fixed << std::setprecision(3) 
              << elapsed << " seconds\n\n";
  } else {
    // Calculate jitter
    auto interval = std::chrono::duration<double>(now - last_packet_time_).count();
    if (packet_count_ > 1) {
      double jitter_diff = std::abs(interval - last_jitter_);
      avg_jitter_ += (jitter_diff - avg_jitter_) / 16.0; // RFC 3550 algorithm
    }
    last_jitter_ = interval;
    last_packet_time_ = now;

    // Check for dropped or out-of-order packets
    uint16_t expected = (last_seq_number_ + 1) % (1 << 16);
    if (seq_number != expected) {
      if (seq_number > expected) {
        dropped_packets_ += (seq_number - expected) % (1 << 16);
      } else {
        out_of_order_packets_++;
      }
    }
  }

  packet_count_++;
  bytes_received_ += bytes_read;
  bytes_received_in_interval_ += bytes_read;
  last_seq_number_ = seq_number;

  return true;
}

void PacketMonitor::clear_line() const {
  // Move to start of line and clear it
  std::cout << "\r" << std::string(80, ' ') << "\r";
}

std::string PacketMonitor::format_stats() const {
  auto now = std::chrono::steady_clock::now();
  std::stringstream ss;

  // Runtime
  auto runtime = std::chrono::duration<double>(now - start_time_).count();
  ss << "Runtime " << std::fixed << std::setprecision(1) << runtime << "s | ";

  // Drop calculation
  double drop_percent = (static_cast<double>(dropped_packets_) / std::max<uint64_t>(1, packet_count_)) * 100.0;
  ss << "Dropped: " << dropped_packets_ << "/" << packet_count_ 
      << " (" << std::setprecision(1) << drop_percent << "%) | ";

  // Bandwidth calculation
  auto dt_sec = std::chrono::duration<double>(now - last_bandwidth_time_).count();
  if (dt_sec > 0) {
    double bytes_per_second = bytes_received_in_interval_ / dt_sec;
    double megabits_per_second = (bytes_per_second * 8) / 1'000'000;
    ss << "Mbit/sec: " << std::setprecision(1) << megabits_per_second << " | ";
  }

  // Jitter (in milliseconds)
  double jitter_ms = avg_jitter_ * 1000;
  ss << "Jitter: " << std::setprecision(2) << jitter_ms << " ms | ";

  // Out of order packets
  ss << "Out of Order: " << out_of_order_packets_;

  return ss.str();
}

void PacketMonitor::print_stats() {
  clear_line();
  std::cout << format_stats() << std::flush;

  // Reset interval counters
  bytes_received_in_interval_ = 0;
  last_bandwidth_time_ = std::chrono::steady_clock::now();
}
