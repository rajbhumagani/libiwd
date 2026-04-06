#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace libiwd {

/**
 * @brief Security type exposed by iwd for a network.
 */
enum class SecurityType {
    Open,
    Wpa2Psk,
    Wpa3Psk,
    Unknown
};

/**
 * @brief Identifier strategy for network operations.
 */
enum class NetworkIdStrategy {
    Ssid,
    InternalId
};

/**
 * @brief Current connection state.
 */
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Failed
};

/**
 * @brief Error code for operation status.
 */
enum class ErrorCode {
    None,
    InvalidInput,
    JsonStoreError,
    DbusTransportError,
    IwdMethodError,
    SelectionError,
    ConnectionError,
    NotFound
};

/**
 * @brief Generic operation result.
 */
struct OperationResult {
    bool success{false};
    ErrorCode error{ErrorCode::None};
    std::string message{};

    static OperationResult ok(std::string msg = {}) { return {true, ErrorCode::None, std::move(msg)}; }
    static OperationResult fail(ErrorCode code, std::string msg) { return {false, code, std::move(msg)}; }
};

/**
 * @brief Typed result wrapper.
 */
template <typename T>
struct Result {
    OperationResult status{};
    std::optional<T> value{};

    static Result<T> ok(T v) { return {OperationResult::ok(), std::move(v)}; }
    static Result<T> fail(ErrorCode code, std::string msg) { return {OperationResult::fail(code, std::move(msg)), std::nullopt}; }
};

/**
 * @brief Discovered wireless network from scan results.
 */
struct WifiNetwork {
    std::string id{};
    std::string ssid{};
    std::optional<std::string> bssid{};
    int rssi{0};
    std::optional<uint32_t> frequencyMhz{};
    SecurityType security{SecurityType::Unknown};
    bool known{false};
    bool connected{false};
    bool available{true};
};

/**
 * @brief Saved network profile managed by iwd.
 */
struct SavedWifiNetwork {
    std::string id{};
    std::string ssid{};
    SecurityType security{SecurityType::Unknown};
    std::optional<std::string> psk{};
    bool enabled{true};
};

/**
 * @brief Scan behavior options.
 */
struct ScanOptions {
    bool freshScan{true};
    bool allowCachedResults{true};
};

/**
 * @brief Selection policy controlling candidate filtering and ranking.
 */
struct SelectionPolicy {
    bool enforceMinRssi{false};
    int minRssi{-90};
    bool preferLastSuccessful{true};
};

/**
 * @brief Global library configuration.
 */
struct LibraryConfig {
    std::string metadataStorePath{"/opt/misc/config/libiwd_priority_store.json"};
    bool alwaysScanBeforeConnect{true};
    bool allowCachedScanResults{true};
    SelectionPolicy selectionPolicy{};
    NetworkIdStrategy idStrategy{NetworkIdStrategy::Ssid};
};

/**
 * @brief Selection output details.
 */
struct NetworkSelectionResult {
    bool selected{false};
    std::string selectedId{};
    std::vector<std::string> rankingOrder{};
    std::string reason{};
};

} // namespace libiwd
