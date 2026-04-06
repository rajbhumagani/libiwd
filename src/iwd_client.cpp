#include "libiwd/iwd_client.hpp"

#include "libiwd/logger.hpp"

namespace libiwd {

IwdClient::IwdClient(std::shared_ptr<IIwdAdapter> adapter, LibraryConfig config)
    : priorityStore_(config.metadataStorePath),
      scanManager_(adapter),
      networkManager_(adapter, priorityStore_),
      connectionManager_(adapter, scanManager_, networkManager_, priorityStore_, selectionEngine_, config) {
    const auto loadStatus = priorityStore_.load();
    if (!loadStatus.success) {
        JournalLogger::instance().warning("IwdClient: failed to load priority metadata: " + loadStatus.message);
    }
}

OperationResult IwdClient::scanNetworks(const ScanOptions& options) { return scanManager_.scanNetworks(options); }

Result<std::vector<WifiNetwork>> IwdClient::getDiscoveredNetworks(bool allowCached) const {
    return scanManager_.getDiscoveredNetworks(allowCached);
}

Result<std::vector<SavedWifiNetwork>> IwdClient::getSavedNetworks() const { return networkManager_.getSavedNetworks(); }

OperationResult IwdClient::addNetwork(const SavedWifiNetwork& network) { return networkManager_.addNetwork(network); }

OperationResult IwdClient::modifyNetwork(const SavedWifiNetwork& network) { return networkManager_.modifyNetwork(network); }

OperationResult IwdClient::deleteNetwork(const std::string& id) { return networkManager_.deleteNetwork(id); }

OperationResult IwdClient::setNetworkPriority(const std::string& id, int priority) {
    return networkManager_.setNetworkPriority(id, priority);
}

Result<int> IwdClient::getNetworkPriority(const std::string& id) const { return networkManager_.getNetworkPriority(id); }

Result<NetworkSelectionResult> IwdClient::connectBestNetwork() { return connectionManager_.connectBestNetwork(); }

OperationResult IwdClient::connectNetworkBySsid(const std::string& ssid) { return connectionManager_.connectNetworkBySsid(ssid); }

OperationResult IwdClient::connectNetworkById(const std::string& id) { return connectionManager_.connectNetworkById(id); }

OperationResult IwdClient::disconnect() { return connectionManager_.disconnect(); }

Result<std::string> IwdClient::getCurrentConnection() const { return connectionManager_.getCurrentConnection(); }

Result<ConnectionState> IwdClient::getConnectionState() const { return connectionManager_.getConnectionState(); }

} // namespace libiwd
