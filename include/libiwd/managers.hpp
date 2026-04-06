#pragma once

#include "libiwd/iwd_adapter.hpp"
#include "libiwd/priority_store.hpp"
#include "libiwd/selection_engine.hpp"
#include "libiwd/types.hpp"

#include <memory>

namespace libiwd {

/**
 * @brief Handles scanning workflows and cache/fresh behavior.
 */
class ScanManager {
public:
    explicit ScanManager(std::shared_ptr<IIwdAdapter> adapter);

    OperationResult scanNetworks(const ScanOptions& options);
    Result<std::vector<WifiNetwork>> getDiscoveredNetworks(bool allowCached) const;

private:
    std::shared_ptr<IIwdAdapter> adapter_;
};

/**
 * @brief Handles saved network add/modify/delete/list operations.
 */
class NetworkManager {
public:
    NetworkManager(std::shared_ptr<IIwdAdapter> adapter, PriorityStore& store);

    Result<std::vector<SavedWifiNetwork>> getSavedNetworks() const;
    OperationResult addNetwork(const SavedWifiNetwork& network);
    OperationResult modifyNetwork(const SavedWifiNetwork& network);
    OperationResult deleteNetwork(const std::string& id);

    OperationResult setNetworkPriority(const std::string& id, int priority);
    Result<int> getNetworkPriority(const std::string& id) const;

private:
    std::shared_ptr<IIwdAdapter> adapter_;
    PriorityStore& store_;
};

/**
 * @brief Handles connect/disconnect and current connection state.
 */
class ConnectionManager {
public:
    ConnectionManager(std::shared_ptr<IIwdAdapter> adapter,
                      ScanManager& scanManager,
                      NetworkManager& networkManager,
                      PriorityStore& store,
                      SelectionEngine& engine,
                      LibraryConfig config);

    Result<NetworkSelectionResult> connectBestNetwork();
    OperationResult connectNetworkBySsid(const std::string& ssid);
    OperationResult connectNetworkById(const std::string& id);
    OperationResult disconnect();
    Result<ConnectionState> getConnectionState() const;
    Result<std::string> getCurrentConnection() const;

private:
    std::shared_ptr<IIwdAdapter> adapter_;
    ScanManager& scanManager_;
    NetworkManager& networkManager_;
    PriorityStore& store_;
    SelectionEngine& engine_;
    LibraryConfig config_;
};

} // namespace libiwd
