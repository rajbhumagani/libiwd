#include "libiwd/managers.hpp"

#include "libiwd/logger.hpp"

#include <chrono>

namespace libiwd {

ScanManager::ScanManager(std::shared_ptr<IIwdAdapter> adapter) : adapter_(std::move(adapter)) {}

OperationResult ScanManager::scanNetworks(const ScanOptions& options) {
    if (options.freshScan) {
        JournalLogger::instance().info("ScanManager: triggering fresh scan");
        auto status = adapter_->triggerScan();
        if (!status.success) {
            JournalLogger::instance().error("ScanManager: scan trigger failed: " + status.message);
            return status;
        }
    }
    return OperationResult::ok();
}

Result<std::vector<WifiNetwork>> ScanManager::getDiscoveredNetworks(bool allowCached) const {
    return adapter_->getDiscoveredNetworks(allowCached);
}

NetworkManager::NetworkManager(std::shared_ptr<IIwdAdapter> adapter, PriorityStore& store)
    : adapter_(std::move(adapter)), store_(store) {}

Result<std::vector<SavedWifiNetwork>> NetworkManager::getSavedNetworks() const { return adapter_->getSavedNetworks(); }

OperationResult NetworkManager::addNetwork(const SavedWifiNetwork& network) {
    if (network.id.empty() || network.ssid.empty()) {
        return OperationResult::fail(ErrorCode::InvalidInput, "Network id/ssid must not be empty");
    }
    JournalLogger::instance().info("NetworkManager: adding network id=" + network.id);
    return adapter_->addNetwork(network);
}

OperationResult NetworkManager::modifyNetwork(const SavedWifiNetwork& network) {
    if (network.id.empty()) {
        return OperationResult::fail(ErrorCode::InvalidInput, "Network id must not be empty");
    }
    JournalLogger::instance().info("NetworkManager: modifying network id=" + network.id);
    return adapter_->modifyNetwork(network);
}

OperationResult NetworkManager::deleteNetwork(const std::string& id) {
    if (id.empty()) {
        return OperationResult::fail(ErrorCode::InvalidInput, "Network id must not be empty");
    }
    JournalLogger::instance().info("NetworkManager: deleting network id=" + id);
    auto status = adapter_->deleteNetwork(id);
    if (!status.success) {
        return status;
    }
    return store_.erase(id);
}

OperationResult NetworkManager::setNetworkPriority(const std::string& id, int priority) {
    if (id.empty()) {
        return OperationResult::fail(ErrorCode::InvalidInput, "Network id must not be empty");
    }
    auto metadata = store_.get(id).value_or(NetworkMetadata{});
    metadata.priority = priority;
    JournalLogger::instance().info("NetworkManager: set priority id=" + id + " priority=" + std::to_string(priority));
    return store_.upsert(id, metadata);
}

Result<int> NetworkManager::getNetworkPriority(const std::string& id) const {
    if (id.empty()) {
        return Result<int>::fail(ErrorCode::InvalidInput, "Network id must not be empty");
    }
    auto metadata = store_.get(id);
    if (!metadata.has_value()) {
        return Result<int>::fail(ErrorCode::NotFound, "No metadata for network id");
    }
    return Result<int>::ok(metadata->priority);
}

ConnectionManager::ConnectionManager(std::shared_ptr<IIwdAdapter> adapter,
                                     ScanManager& scanManager,
                                     NetworkManager& networkManager,
                                     PriorityStore& store,
                                     SelectionEngine& engine,
                                     LibraryConfig config)
    : adapter_(std::move(adapter)),
      scanManager_(scanManager),
      networkManager_(networkManager),
      store_(store),
      engine_(engine),
      config_(std::move(config)) {}

Result<NetworkSelectionResult> ConnectionManager::connectBestNetwork() {
    ScanOptions scanOpts{config_.alwaysScanBeforeConnect, config_.allowCachedScanResults};
    auto scanStatus = scanManager_.scanNetworks(scanOpts);
    if (!scanStatus.success) {
        return Result<NetworkSelectionResult>::fail(scanStatus.error, scanStatus.message);
    }

    auto discovered = scanManager_.getDiscoveredNetworks(config_.allowCachedScanResults);
    if (!discovered.status.success) {
        return Result<NetworkSelectionResult>::fail(discovered.status.error, discovered.status.message);
    }

    auto saved = networkManager_.getSavedNetworks();
    if (!saved.status.success) {
        return Result<NetworkSelectionResult>::fail(saved.status.error, saved.status.message);
    }

    auto selection = engine_.selectBest(*discovered.value, *saved.value, store_, config_.selectionPolicy);
    if (!selection.selected) {
        return Result<NetworkSelectionResult>::fail(ErrorCode::SelectionError, selection.reason);
    }

    auto connectStatus = adapter_->connectById(selection.selectedId);
    if (!connectStatus.success) {
        return Result<NetworkSelectionResult>::fail(connectStatus.error, connectStatus.message);
    }

    auto metadata = store_.get(selection.selectedId).value_or(NetworkMetadata{});
    metadata.lastSuccessfulEpochSec = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const auto persistStatus = store_.upsert(selection.selectedId, metadata);
    if (!persistStatus.success) {
        JournalLogger::instance().warning("ConnectionManager: failed to persist last-success metadata");
    }

    return Result<NetworkSelectionResult>::ok(selection);
}

OperationResult ConnectionManager::connectNetworkBySsid(const std::string& ssid) {
    if (ssid.empty()) {
        return OperationResult::fail(ErrorCode::InvalidInput, "SSID must not be empty");
    }
    auto saved = networkManager_.getSavedNetworks();
    if (!saved.status.success) {
        return saved.status;
    }
    for (const auto& network : *saved.value) {
        if (network.ssid == ssid) {
            return connectNetworkById(network.id);
        }
    }
    return OperationResult::fail(ErrorCode::NotFound, "No saved network found for SSID");
}

OperationResult ConnectionManager::connectNetworkById(const std::string& id) {
    if (id.empty()) {
        return OperationResult::fail(ErrorCode::InvalidInput, "Network id must not be empty");
    }
    JournalLogger::instance().info("ConnectionManager: connecting id=" + id);
    return adapter_->connectById(id);
}

OperationResult ConnectionManager::disconnect() { return adapter_->disconnect(); }

Result<ConnectionState> ConnectionManager::getConnectionState() const { return adapter_->getConnectionState(); }

Result<std::string> ConnectionManager::getCurrentConnection() const { return adapter_->getCurrentConnectionId(); }

} // namespace libiwd
