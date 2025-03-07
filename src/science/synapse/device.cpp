#include "science/synapse/device.h"

namespace synapse {

Device::Device(const std::string& uri)
  : uri_(uri),
    channel_(grpc::CreateChannel(uri, grpc::InsecureChannelCredentials())),
    rpc_(synapse::SynapseDevice::NewStub(channel_)) {}

auto Device::configure(Config* config, std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  auto s = config->set_device(this);
  if (!s.ok()) {
    return { s.code(), "failed to set device: " + s.message() };
  }

  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

  synapse::DeviceConfiguration req = config->to_proto();
  synapse::Status res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Configure(
    &context, &req, &res,
    [this, &res, &status, &done, &m, &cv](grpc::Status gstatus) mutable {
      science::Status s;
      if (!gstatus.ok()) {
        s = { static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message() };
      } else {
        s = handle_status_response(res);
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      status = s;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  return status;
}

auto Device::info(synapse::DeviceInfo* info, std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

  google::protobuf::Empty req;
  synapse::DeviceInfo res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Info(
    &context, &req, &res,
    [this, &res, &status, &done, &m, &cv](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = { static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message() };
      } else {
        s = handle_status_response(res.status());
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      status = s;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  if (status.ok()) {
    info->CopyFrom(res);
  }
  return status;
}

auto Device::start(std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

  google::protobuf::Empty req;
  synapse::Status res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Start(
    &context, &req, &res,
    [this, &res, &status, &done, &m, &cv](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = { static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message() };
      } else {
        s = handle_status_response(res);
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      status = s;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  return status;
}

auto Device::stop(std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

  google::protobuf::Empty req;
  synapse::Status res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Stop(
    &context, &req, &res,
    [this, &res, &status, &done, &m, &cv](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = { static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message() };
      } else {
        s = handle_status_response(res);
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      status = s;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  return status;
}

auto Device::sockets() const -> const std::vector<synapse::NodeSocket>& {
  return sockets_;
}

auto Device::uri() const -> const std::string& {
  return uri_;
}

auto Device::get_logs(
    const std::string& log_level,
    std::optional<int64_t> since_ms,
    std::optional<int64_t> start_time_ns,
    std::optional<int64_t> end_time_ns,
    std::optional<std::chrono::milliseconds> timeout) -> std::vector<std::string> {

  synapse::LogQueryRequest request;
  request.set_min_level(log_level_to_pb(log_level));

  if (since_ms.has_value()) {
    request.set_since_ms(since_ms.value());
  } else {
    if (start_time_ns.has_value()) {
      request.set_start_time_ns(start_time_ns.value());
    }
    if (end_time_ns.has_value()) {
      request.set_end_time_ns(end_time_ns.value());
    }
  }

  grpc::ClientContext context;
  if (timeout.has_value()) {
    context.set_deadline(std::chrono::system_clock::now() + timeout.value());
  }

  synapse::LogQueryResponse response;
  std::vector<std::string> logs;

  grpc::Status status = rpc_->GetLogs(&context, request, &response);

  if (status.ok()) {
    for (const auto& entry : response.entries()) {
      logs.push_back(entry.message());
    }
  } else {
    std::cerr << "Error getting logs: " << status.error_message() << std::endl;
  }

  return logs;
}

auto Device::tail_logs(
    const std::string& log_level,
    std::optional<std::chrono::milliseconds> timeout) -> std::vector<std::string> {

  synapse::TailLogsRequest request;
  request.set_min_level(log_level_to_pb(log_level));

  grpc::ClientContext context;
  if (timeout.has_value()) {
    context.set_deadline(std::chrono::system_clock::now() + timeout.value());
  }

  std::vector<std::string> logs;

  std::unique_ptr<grpc::ClientReader<synapse::LogEntry>> reader(
      rpc_->TailLogs(&context, request));

  synapse::LogEntry entry;
  while (reader->Read(&entry)) {
    logs.push_back(entry.message());
  }

  grpc::Status status = reader->Finish();
  if (!status.ok()) {
    std::cerr << "Error tailing logs: " << status.error_message() << std::endl;
  }

  return logs;
}

auto Device::handle_status_response(const synapse::Status& status) -> science::Status {
  if (status.code() != synapse::StatusCode::kOk) {
    return {
      science::StatusCode::kInternal,
      "(code: " + std::to_string(status.code()) + "): " + status.message()
    };
  }

  sockets_.clear();
  for (const auto& socket : status.sockets()) {
    synapse::NodeSocket s;
    s.CopyFrom(socket);
    sockets_.push_back(s);
  }

  return {};
}

auto Device::log_level_to_pb(const std::string& level) -> synapse::LogLevel {
  if (level == "DEBUG") return synapse::LogLevel::LOG_LEVEL_DEBUG;
  if (level == "INFO") return synapse::LogLevel::LOG_LEVEL_INFO;
  if (level == "WARNING") return synapse::LogLevel::LOG_LEVEL_WARNING;
  if (level == "ERROR") return synapse::LogLevel::LOG_LEVEL_ERROR;
  if (level == "CRITICAL") return synapse::LogLevel::LOG_LEVEL_CRITICAL;
  return synapse::LogLevel::LOG_LEVEL_UNKNOWN;
}

}  // namespace synapse
