#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

class PacketMonitor {
 public:
  PacketMonitor();
  
  void start_monitoring();
  void print_stats();
  bool process_packet(uint16_t seq_number, size_t bytes_read);

 private:
  // Packet tracking
  uint64_t packet_count_;
  uint16_t last_seq_number_;
  uint64_t dropped_packets_;
  uint64_t out_of_order_packets_;

  // Timing metrics
  std::chrono::steady_clock::time_point start_time_;
  std::chrono::steady_clock::time_point first_packet_time_;
  std::chrono::steady_clock::time_point last_packet_time_;
  std::chrono::steady_clock::time_point last_stats_time_;

  // Bandwidth tracking
  uint64_t bytes_received_;
  uint64_t bytes_received_in_interval_;
  std::chrono::steady_clock::time_point last_bandwidth_time_;

  // Jitter tracking
  double last_jitter_;
  double avg_jitter_;

  // Helper methods
  void clear_line() const;
  std::string format_stats() const;
};
