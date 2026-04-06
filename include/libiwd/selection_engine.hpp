#pragma once

#include "libiwd/priority_store.hpp"
#include "libiwd/types.hpp"

#include <vector>

namespace libiwd {

/**
 * @brief Policy engine that filters and ranks network candidates.
 */
class SelectionEngine {
public:
    /**
     * @brief Select best candidate according to configured policy.
     */
    NetworkSelectionResult selectBest(const std::vector<WifiNetwork>& discovered,
                                      const std::vector<SavedWifiNetwork>& saved,
                                      const PriorityStore& priorityStore,
                                      const SelectionPolicy& policy) const;
};

} // namespace libiwd
