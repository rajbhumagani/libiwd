#pragma once

#include "libiwd/iwd_adapter.hpp"

#include <algorithm>

namespace libiwd::tests {

class MockIwdAdapter final : public IIwdAdapter {
public:
    bool scanCalled{false};
    std::vector<WifiNetwork> discovered;
    std::vector<SavedWifiNetwork> saved;
    ConnectionState state{ConnectionState::Disconnected};
    std::string connectedId;

    OperationResult triggerScan() override {
        scanCalled = true;
        return OperationResult::ok();
    }

    Result<std::vector<WifiNetwork>> getDiscoveredNetworks(bool) override { return Result<std::vector<WifiNetwork>>::ok(discovered); }

    Result<std::vector<SavedWifiNetwork>> getSavedNetworks() override { return Result<std::vector<SavedWifiNetwork>>::ok(saved); }

    OperationResult addNetwork(const SavedWifiNetwork& network) override {
        saved.push_back(network);
        return OperationResult::ok();
    }

    OperationResult modifyNetwork(const SavedWifiNetwork& network) override {
        for (auto& item : saved) {
            if (item.id == network.id) {
                item = network;
                return OperationResult::ok();
            }
        }
        return OperationResult::fail(ErrorCode::NotFound, "missing");
    }

    OperationResult deleteNetwork(const std::string& id) override {
        saved.erase(std::remove_if(saved.begin(), saved.end(), [&](const SavedWifiNetwork& n) { return n.id == id; }), saved.end());
        return OperationResult::ok();
    }

    OperationResult connectById(const std::string& id) override {
        connectedId = id;
        state = ConnectionState::Connected;
        return OperationResult::ok();
    }

    OperationResult disconnect() override {
        connectedId.clear();
        state = ConnectionState::Disconnected;
        return OperationResult::ok();
    }

    Result<ConnectionState> getConnectionState() override { return Result<ConnectionState>::ok(state); }

    Result<std::string> getCurrentConnectionId() override { return Result<std::string>::ok(connectedId); }
};

} // namespace libiwd::tests
