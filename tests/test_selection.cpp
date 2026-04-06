#include "libiwd/selection_engine.hpp"

TEST(selection_prefers_priority_then_rssi_then_id) {
    libiwd::PriorityStore store("/tmp/libiwd_selection_test.json");
    ASSERT_TRUE(store.upsert("A", {5, true, 0}).success);
    ASSERT_TRUE(store.upsert("B", {10, true, 0}).success);
    ASSERT_TRUE(store.upsert("C", {10, true, 0}).success);

    std::vector<libiwd::WifiNetwork> discovered{
        {.id = "A", .ssid = "A", .rssi = -40, .security = libiwd::SecurityType::Wpa2Psk, .available = true},
        {.id = "B", .ssid = "B", .rssi = -60, .security = libiwd::SecurityType::Wpa2Psk, .available = true},
        {.id = "C", .ssid = "C", .rssi = -55, .security = libiwd::SecurityType::Wpa2Psk, .available = true},
    };
    std::vector<libiwd::SavedWifiNetwork> saved{
        {.id = "A", .ssid = "A", .security = libiwd::SecurityType::Wpa2Psk, .enabled = true},
        {.id = "B", .ssid = "B", .security = libiwd::SecurityType::Wpa2Psk, .enabled = true},
        {.id = "C", .ssid = "C", .security = libiwd::SecurityType::Wpa2Psk, .enabled = true},
    };

    libiwd::SelectionEngine engine;
    auto result = engine.selectBest(discovered, saved, store, libiwd::SelectionPolicy{});
    ASSERT_TRUE(result.selected);
    ASSERT_TRUE(result.selectedId == "C");
    ASSERT_TRUE(result.rankingOrder.size() == 3);
}

TEST(selection_applies_min_rssi_threshold) {
    libiwd::PriorityStore store("/tmp/libiwd_selection_test_rssi.json");
    ASSERT_TRUE(store.upsert("A", {10, true, 0}).success);

    std::vector<libiwd::WifiNetwork> discovered{{.id = "A", .ssid = "A", .rssi = -90, .security = libiwd::SecurityType::Wpa2Psk, .available = true}};
    std::vector<libiwd::SavedWifiNetwork> saved{{.id = "A", .ssid = "A", .security = libiwd::SecurityType::Wpa2Psk, .enabled = true}};

    libiwd::SelectionPolicy policy;
    policy.enforceMinRssi = true;
    policy.minRssi = -80;

    libiwd::SelectionEngine engine;
    auto result = engine.selectBest(discovered, saved, store, policy);
    ASSERT_TRUE(!result.selected);
}
