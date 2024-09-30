
#include <gtest/gtest.h>
#include <science/scipp/status.h>
#include <science/synapse/device_advertisement.h>

namespace synapse {

extern auto parse(const std::string& host,
                 const std::vector<std::string>& payload,
                 DeviceAdvertisement* parsed) -> science::Status;

TEST(DiscoverTest, ParseValidMessage) {
  DeviceAdvertisement parsed;
  std::vector<std::string> payload = {"ID", "ABC123", "SYN1.2.3", "8080", "test-device-1"};
  auto status = parse("192.168.1.1", payload, &parsed);

  EXPECT_TRUE(status.ok());
  EXPECT_EQ(parsed.serial, "ABC123");
  EXPECT_EQ(parsed.capability, "SYN1.2.3");
  EXPECT_EQ(parsed.port, 8080);
  EXPECT_EQ(parsed.name, "test-device-1");
  EXPECT_EQ(parsed.host, "192.168.1.1");
}

TEST(DiscoverTest, ParseValidMessageHumanizedName) {
  DeviceAdvertisement parsed;
  std::vector<std::string> payload = {"ID", "XYZ789", "SYN2.0.0", "9090", "Test Device 2"};
  auto status = parse("10.0.0.1", payload, &parsed);

  EXPECT_TRUE(status.ok());
  EXPECT_EQ(parsed.serial, "XYZ789");
  EXPECT_EQ(parsed.capability, "SYN2.0.0");
  EXPECT_EQ(parsed.port, 9090);
  EXPECT_EQ(parsed.name, "Test Device 2");
  EXPECT_EQ(parsed.host, "10.0.0.1");
}

TEST(DiscoverTest, ParseInvalidCommand) {
  DeviceAdvertisement parsed;
  std::vector<std::string> payload = {"INVALID", "ABC123", "SYN1.2.3", "8080", "Test Device"};
  auto status = parse("192.168.1.1", payload, &parsed);

  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), science::StatusCode::kInvalidArgument);
}

TEST(DiscoverTest, ParseInvalidPortNumberString) {
  DeviceAdvertisement parsed;
  std::vector<std::string> payload = {"ID", "ABC123", "SYN1.2.3", "invalid", "Test Device"};
  auto status = parse("192.168.1.1", payload, &parsed);

  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), science::StatusCode::kInvalidArgument);
}

TEST(DiscoverTest, ParseInvalidPortNumberValue) {
  DeviceAdvertisement parsed;
  std::vector<std::string> payload_a = {"ID", "ABC123", "SYN1.2.3", "100000", "Test Device"};
  auto status = parse("192.168.1.1", payload_a, &parsed);

  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), science::StatusCode::kInvalidArgument);

  std::vector<std::string> payload_b = {"ID", "ABC123", "SYN1.2.3", "0", "Test Device"};
  status = parse("192.168.1.1", payload_b, &parsed);

  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), science::StatusCode::kInvalidArgument);
}

TEST(DiscoverTest, ParseTooFewElements) {
  DeviceAdvertisement parsed;
  std::vector<std::string> payload = {"ID", "ABC123", "SYN1.2.3", "8080"};
  auto status = parse("192.168.1.1", payload, &parsed);

  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), science::StatusCode::kInvalidArgument);
}

}  // namespace synapse
