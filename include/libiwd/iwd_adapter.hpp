#pragma once

#include "libiwd/types.hpp"

#include <string>
#include <vector>

namespace libiwd {

/**
 * @brief Abstract transport adapter hiding D-Bus / iwd details.
 */
class IIwdAdapter {
public:
    virtual ~IIwdAdapter() = default;

    virtual OperationResult triggerScan() = 0;
    virtual Result<std::vector<WifiNetwork>> getDiscoveredNetworks(bool allowCached) = 0;

    virtual Result<std::vector<SavedWifiNetwork>> getSavedNetworks() = 0;
    virtual OperationResult addNetwork(const SavedWifiNetwork& network) = 0;
    virtual OperationResult modifyNetwork(const SavedWifiNetwork& network) = 0;
    virtual OperationResult deleteNetwork(const std::string& id) = 0;

    virtual OperationResult connectById(const std::string& id) = 0;
    virtual OperationResult disconnect() = 0;

    virtual Result<ConnectionState> getConnectionState() = 0;
    virtual Result<std::string> getCurrentConnectionId() = 0;
};

} // namespace libiwd
