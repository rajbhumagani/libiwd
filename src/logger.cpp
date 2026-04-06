#include "libiwd/logger.hpp"

#include <iostream>

#if defined(LIBIWD_HAS_SYSTEMD)
#include <systemd/sd-journal.h>
#endif

namespace libiwd {

JournalLogger& JournalLogger::instance() {
    static JournalLogger logger;
    return logger;
}

void JournalLogger::setSinkForTests(std::function<void(Level, const std::string&)> sink) {
    std::scoped_lock lock(mutex_);
    sink_ = std::move(sink);
}

void JournalLogger::debug(const std::string& message) { log(Level::Debug, message); }
void JournalLogger::info(const std::string& message) { log(Level::Info, message); }
void JournalLogger::warning(const std::string& message) { log(Level::Warning, message); }
void JournalLogger::error(const std::string& message) { log(Level::Error, message); }

void JournalLogger::log(Level level, const std::string& message) {
    std::scoped_lock lock(mutex_);
    if (sink_) {
        sink_(level, message);
        return;
    }

#if defined(LIBIWD_HAS_SYSTEMD)
    int priority = LOG_INFO;
    switch (level) {
    case Level::Debug: priority = LOG_DEBUG; break;
    case Level::Info: priority = LOG_INFO; break;
    case Level::Warning: priority = LOG_WARNING; break;
    case Level::Error: priority = LOG_ERR; break;
    }
    sd_journal_print(priority, "libiwd: %s", message.c_str());
#else
    std::cerr << "libiwd: " << message << '\n';
#endif
}

} // namespace libiwd
