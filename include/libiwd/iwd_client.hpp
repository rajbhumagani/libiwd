#pragma once

#include "libiwd/managers.hpp"

namespace libiwd {

/**
 * @brief High-level synchronous API facade for wireless management through iwd.
 */
class IwdClient {
public:
    /**
     * @brief Construct client with an adapter and configuration.
     */
    IwdClient(std::shared_ptr<IIwdAdapter> adapter, LibraryConfig config = {});

    OperationResult scanNetworks(const ScanOptions& options);
    Result<std::vector<WifiNetwork>> getDiscoveredNetworks(bool allowCached = true) const;

    Result<std::vector<SavedWifiNetwork>> getSavedNetworks() const;
    OperationResult addNetwork(const SavedWifiNetwork& network);
    OperationResult modifyNetwork(const SavedWifiNetwork& network);
    OperationResult deleteNetwork(const std::string& id);

    OperationResult setNetworkPriority(const std::string& id, int priority);
    Result<int> getNetworkPriority(const std::string& id) const;

    Result<NetworkSelectionResult> connectBestNetwork();
    OperationResult connectNetworkBySsid(const std::string& ssid);
    OperationResult connectNetworkById(const std::string& id);
    OperationResult disconnect();
    Result<std::string> getCurrentConnection() const;
    Result<ConnectionState> getConnectionState() const;

private:
    PriorityStore priorityStore_;
    SelectionEngine selectionEngine_;
    ScanManager scanManager_;
    NetworkManager networkManager_;
    ConnectionManager connectionManager_;
};

} // namespace libiwd
