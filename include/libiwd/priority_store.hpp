#pragma once

#include "libiwd/types.hpp"

#include <optional>
#include <string>
#include <unordered_map>

namespace libiwd {

/**
 * @brief Per-network metadata controlled by this library.
 */
struct NetworkMetadata {
    int priority{0};
    bool enabled{true};
    std::int64_t lastSuccessfulEpochSec{0};
};

/**
 * @brief JSON-backed metadata store for library selection policy.
 */
class PriorityStore {
public:
    /**
     * @brief Construct store bound to a JSON file path.
     */
    explicit PriorityStore(std::string jsonPath);

    /** @brief Load metadata from JSON file. */
    OperationResult load();
    /** @brief Persist metadata to JSON file. */
    OperationResult save() const;

    /** @brief Set or update metadata for an identifier. */
    OperationResult upsert(const std::string& id, const NetworkMetadata& metadata);
    /** @brief Remove metadata for identifier. */
    OperationResult erase(const std::string& id);
    /** @brief Get metadata for identifier. */
    std::optional<NetworkMetadata> get(const std::string& id) const;

    /** @brief Read all metadata entries. */
    const std::unordered_map<std::string, NetworkMetadata>& all() const;

private:
    std::string jsonPath_;
    std::unordered_map<std::string, NetworkMetadata> metadata_;
};

} // namespace libiwd
