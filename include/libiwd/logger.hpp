#pragma once

#include <functional>
#include <mutex>
#include <string>

namespace libiwd {

/**
 * @brief Logger singleton that writes to journald (or stderr fallback).
 */
class JournalLogger {
public:
    enum class Level { Debug, Info, Warning, Error };

    /**
     * @brief Get singleton instance.
     */
    static JournalLogger& instance();

    /**
     * @brief Override sink for tests.
     */
    void setSinkForTests(std::function<void(Level, const std::string&)> sink);

    /** @brief Log debug message. */
    void debug(const std::string& message);
    /** @brief Log info message. */
    void info(const std::string& message);
    /** @brief Log warning message. */
    void warning(const std::string& message);
    /** @brief Log error message. */
    void error(const std::string& message);

private:
    JournalLogger() = default;
    void log(Level level, const std::string& message);

    std::mutex mutex_;
    std::function<void(Level, const std::string&)> sink_{};
};

} // namespace libiwd
