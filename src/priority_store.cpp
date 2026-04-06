#include "libiwd/priority_store.hpp"

#include "libiwd/logger.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace libiwd {
namespace {

std::string jsonEscape(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    for (char ch : input) {
        if (ch == '\\' || ch == '"') {
            out.push_back('\\');
        }
        out.push_back(ch);
    }
    return out;
}

std::string jsonUnescape(std::string input) {
    std::string out;
    out.reserve(input.size());
    bool escaped = false;
    for (char ch : input) {
        if (escaped) {
            out.push_back(ch);
            escaped = false;
        } else if (ch == '\\') {
            escaped = true;
        } else {
            out.push_back(ch);
        }
    }
    return out;
}

std::string extractString(const std::string& object, const std::string& key) {
    const auto probe = "\"" + key + "\":\"";
    const auto begin = object.find(probe);
    if (begin == std::string::npos) {
        return {};
    }
    const auto valueStart = begin + probe.size();
    auto end = valueStart;
    while (end < object.size()) {
        if (object[end] == '"' && object[end - 1] != '\\') {
            break;
        }
        ++end;
    }
    if (end <= valueStart || end >= object.size()) {
        return {};
    }
    return jsonUnescape(object.substr(valueStart, end - valueStart));
}

std::optional<int> extractInt(const std::string& object, const std::string& key) {
    const auto probe = "\"" + key + "\":";
    const auto begin = object.find(probe);
    if (begin == std::string::npos) {
        return std::nullopt;
    }
    const auto valueStart = begin + probe.size();
    const auto end = object.find_first_of(",}", valueStart);
    if (end == std::string::npos) {
        return std::nullopt;
    }
    return std::stoi(object.substr(valueStart, end - valueStart));
}

std::optional<bool> extractBool(const std::string& object, const std::string& key) {
    const auto probe = "\"" + key + "\":";
    const auto begin = object.find(probe);
    if (begin == std::string::npos) {
        return std::nullopt;
    }
    const auto valueStart = begin + probe.size();
    if (object.compare(valueStart, 4, "true") == 0) {
        return true;
    }
    if (object.compare(valueStart, 5, "false") == 0) {
        return false;
    }
    return std::nullopt;
}

} // namespace

PriorityStore::PriorityStore(std::string jsonPath) : jsonPath_(std::move(jsonPath)) {}

OperationResult PriorityStore::load() {
    metadata_.clear();
    std::ifstream in(jsonPath_);
    if (!in.good()) {
        return OperationResult::ok("Metadata file missing; starting empty");
    }

    std::stringstream ss;
    ss << in.rdbuf();
    const auto text = ss.str();

    auto begin = text.find("[");
    auto end = text.find("]", begin);
    if (begin == std::string::npos || end == std::string::npos || end <= begin) {
        return OperationResult::fail(ErrorCode::JsonStoreError, "Invalid metadata JSON");
    }

    std::string entries = text.substr(begin + 1, end - begin - 1);
    size_t cursor = 0;
    while (cursor < entries.size()) {
        const auto open = entries.find('{', cursor);
        if (open == std::string::npos) {
            break;
        }
        const auto close = entries.find('}', open);
        if (close == std::string::npos) {
            return OperationResult::fail(ErrorCode::JsonStoreError, "Malformed entry in metadata JSON");
        }
        const auto object = entries.substr(open, close - open + 1);
        const auto id = extractString(object, "id");
        const auto priority = extractInt(object, "priority");
        const auto enabled = extractBool(object, "enabled");
        const auto last = extractInt(object, "last_success");
        if (!id.empty() && priority.has_value() && enabled.has_value() && last.has_value()) {
            metadata_[id] = NetworkMetadata{*priority, *enabled, *last};
        }
        cursor = close + 1;
    }

    JournalLogger::instance().debug("PriorityStore loaded entries=" + std::to_string(metadata_.size()));
    return OperationResult::ok();
}

OperationResult PriorityStore::save() const {
    try {
        const auto parent = std::filesystem::path(jsonPath_).parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }

        std::ofstream out(jsonPath_, std::ios::trunc);
        if (!out.good()) {
            return OperationResult::fail(ErrorCode::JsonStoreError, "Unable to open metadata JSON for writing");
        }

        out << "{\n  \"version\":1,\n  \"entries\":[\n";
        bool first = true;
        for (const auto& [id, item] : metadata_) {
            if (!first) {
                out << ",\n";
            }
            first = false;
            out << "    {\"id\":\"" << jsonEscape(id) << "\",\"priority\":" << item.priority
                << ",\"enabled\":" << (item.enabled ? "true" : "false")
                << ",\"last_success\":" << item.lastSuccessfulEpochSec << "}";
        }
        out << "\n  ]\n}\n";
    } catch (const std::exception& ex) {
        return OperationResult::fail(ErrorCode::JsonStoreError, ex.what());
    }

    return OperationResult::ok();
}

OperationResult PriorityStore::upsert(const std::string& id, const NetworkMetadata& metadata) {
    if (id.empty()) {
        return OperationResult::fail(ErrorCode::InvalidInput, "id cannot be empty");
    }
    metadata_[id] = metadata;
    return save();
}

OperationResult PriorityStore::erase(const std::string& id) {
    if (id.empty()) {
        return OperationResult::fail(ErrorCode::InvalidInput, "id cannot be empty");
    }
    metadata_.erase(id);
    return save();
}

std::optional<NetworkMetadata> PriorityStore::get(const std::string& id) const {
    const auto it = metadata_.find(id);
    if (it == metadata_.end()) {
        return std::nullopt;
    }
    return it->second;
}

const std::unordered_map<std::string, NetworkMetadata>& PriorityStore::all() const { return metadata_; }

} // namespace libiwd
