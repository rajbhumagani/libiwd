#include "libiwd/iwd_client.hpp"

#include "mock_iwd_adapter.hpp"

TEST(client_connect_best_uses_scan_and_selection) {
    auto adapter = std::make_shared<libiwd::tests::MockIwdAdapter>();
    adapter->saved = {
        {.id = "net1", .ssid = "Guest", .security = libiwd::SecurityType::Open, .enabled = true},
        {.id = "net2", .ssid = "Corp", .security = libiwd::SecurityType::Wpa2Psk, .enabled = true},
    };
    adapter->discovered = {
        {.id = "net1", .ssid = "Guest", .rssi = -40, .security = libiwd::SecurityType::Open, .available = true},
        {.id = "net2", .ssid = "Corp", .rssi = -50, .security = libiwd::SecurityType::Wpa2Psk, .available = true},
    };

    libiwd::LibraryConfig config;
    config.metadataStorePath = "/tmp/libiwd_client_test.json";
    config.alwaysScanBeforeConnect = true;

    libiwd::IwdClient client(adapter, config);
    ASSERT_TRUE(client.setNetworkPriority("net1", 1).success);
    ASSERT_TRUE(client.setNetworkPriority("net2", 10).success);

    auto result = client.connectBestNetwork();
    ASSERT_TRUE(result.status.success);
    ASSERT_TRUE(adapter->scanCalled);
    ASSERT_TRUE(adapter->connectedId == "net2");
}

TEST(client_network_crud_validation) {
    auto adapter = std::make_shared<libiwd::tests::MockIwdAdapter>();

    libiwd::LibraryConfig config;
    config.metadataStorePath = "/tmp/libiwd_client_test2.json";
    libiwd::IwdClient client(adapter, config);

    ASSERT_TRUE(!client.addNetwork({.id = "", .ssid = "bad", .security = libiwd::SecurityType::Open}).success);
    ASSERT_TRUE(client.addNetwork({.id = "ok", .ssid = "ok", .security = libiwd::SecurityType::Open}).success);
    ASSERT_TRUE(client.modifyNetwork({.id = "ok", .ssid = "ok2", .security = libiwd::SecurityType::Open}).success);
    ASSERT_TRUE(client.deleteNetwork("ok").success);
}
