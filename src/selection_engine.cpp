#include "libiwd/selection_engine.hpp"

#include "libiwd/logger.hpp"

#include <algorithm>
#include <unordered_map>

namespace libiwd {

NetworkSelectionResult SelectionEngine::selectBest(const std::vector<WifiNetwork>& discovered,
                                                   const std::vector<SavedWifiNetwork>& saved,
                                                   const PriorityStore& priorityStore,
                                                   const SelectionPolicy& policy) const {
    struct Candidate {
        WifiNetwork network;
        int priority{0};
        bool enabled{true};
        std::int64_t lastSuccess{0};
    };

    std::unordered_map<std::string, SavedWifiNetwork> savedById;
    for (const auto& net : saved) {
        savedById[net.id] = net;
    }

    std::vector<Candidate> candidates;
    for (const auto& net : discovered) {
        if (!net.available || net.id.empty()) {
            continue;
        }
        const auto savedIt = savedById.find(net.id);
        if (savedIt == savedById.end() || !savedIt->second.enabled) {
            continue;
        }
        if (policy.enforceMinRssi && net.rssi < policy.minRssi) {
            continue;
        }
        if (net.security == SecurityType::Unknown) {
            continue;
        }

        const auto metadata = priorityStore.get(net.id).value_or(NetworkMetadata{});
        if (!metadata.enabled) {
            continue;
        }
        candidates.push_back(Candidate{net, metadata.priority, metadata.enabled, metadata.lastSuccessfulEpochSec});
    }

    std::sort(candidates.begin(), candidates.end(), [&](const Candidate& a, const Candidate& b) {
        if (a.priority != b.priority) {
            return a.priority > b.priority;
        }
        if (a.network.rssi != b.network.rssi) {
            return a.network.rssi > b.network.rssi;
        }
        if (policy.preferLastSuccessful && a.lastSuccess != b.lastSuccess) {
            return a.lastSuccess > b.lastSuccess;
        }
        return a.network.id < b.network.id;
    });

    NetworkSelectionResult result;
    for (const auto& candidate : candidates) {
        result.rankingOrder.push_back(candidate.network.id);
    }

    if (candidates.empty()) {
        result.reason = "No eligible candidates";
        JournalLogger::instance().warning("SelectionEngine: no eligible candidates");
        return result;
    }

    result.selected = true;
    result.selectedId = candidates.front().network.id;
    result.reason = "Selected highest-priority eligible network";
    JournalLogger::instance().info("SelectionEngine: selected network id=" + result.selectedId);
    return result;
}

} // namespace libiwd
