#pragma once

#include <string>
#include <utility>

namespace science {

/**
 * Library-agnostic status codes for return values.
 * These codes are 1:1 with abseil / gRPC status codes.
 */
enum class StatusCode : int {
  kOk = 0,
  kCancelled = 1,
  kUnknown = 2,
  kInvalidArgument = 3,
  kDeadlineExceeded = 4,
  kNotFound = 5,
  kAlreadyExists = 6,
  kPermissionDenied = 7,
  kResourceExhausted = 8,
  kFailedPrecondition = 9,
  kAborted = 10,
  kOutOfRange = 11,
  kUnimplemented = 12,
  kInternal = 13,
  kUnavailable = 14,
  kDataLoss = 15,
  kUnauthenticated = 16,
};

/**
 * Status, containing a status code and an optional message.
 */
class Status {
 public:
  Status() = default;
  explicit Status(StatusCode code) : code_(code) {}
  Status(StatusCode code, std::string message) : code_(code), message_(std::move(message)) {}

  [[nodiscard]] bool ok() const { return code_ == StatusCode::kOk; }
  [[nodiscard]] StatusCode code() const { return code_; }
  [[nodiscard]] std::string message() const { return message_; }

 private:
  StatusCode code_{StatusCode::kOk};
  std::string message_{};
};

}  // namespace science
