#include "infra/spdlog_logger.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <filesystem>
#include <algorithm>

namespace teleop {

SpdlogLogger::SpdlogLogger(const std::string& name, const std::string& logLevel,
                           const std::string& logDir) {
    std::vector<spdlog::sink_ptr> sinks;

    // Console sink (human-readable)
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
    sinks.push_back(consoleSink);

    // File sink (JSON-structured, rotating)
    try {
        std::filesystem::create_directories(logDir);
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logDir + "/" + name + ".log",
            100 * 1024 * 1024,  // 100MB max
            10                   // 10 rotated files
        );
        fileSink->set_pattern("{\"timestamp\":\"%Y-%m-%dT%H:%M:%S.%e\",\"level\":\"%l\",\"logger\":\"%n\",\"message\":\"%v\"}");
        sinks.push_back(fileSink);
    } catch (const std::exception&) {
        // If we can't create the log directory, continue with console only
    }

    logger_ = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());

    // Set log level
    std::string level = logLevel;
    std::transform(level.begin(), level.end(), level.begin(), ::tolower);
    if (level == "trace") logger_->set_level(spdlog::level::trace);
    else if (level == "debug") logger_->set_level(spdlog::level::debug);
    else if (level == "info") logger_->set_level(spdlog::level::info);
    else if (level == "warn") logger_->set_level(spdlog::level::warn);
    else if (level == "error") logger_->set_level(spdlog::level::err);
    else if (level == "critical") logger_->set_level(spdlog::level::critical);
    else logger_->set_level(spdlog::level::info);

    logger_->flush_on(spdlog::level::warn);
    spdlog::register_logger(logger_);
}

void SpdlogLogger::trace(std::string_view msg) { logger_->trace(msg); }
void SpdlogLogger::debug(std::string_view msg) { logger_->debug(msg); }
void SpdlogLogger::info(std::string_view msg) { logger_->info(msg); }
void SpdlogLogger::warn(std::string_view msg) { logger_->warn(msg); }
void SpdlogLogger::error(std::string_view msg) { logger_->error(msg); }
void SpdlogLogger::critical(std::string_view msg) { logger_->critical(msg); }

void SpdlogLogger::event(std::string_view category, std::string_view jsonData) {
    logger_->info("[EVENT:{}] {}", category, jsonData);
}

} // namespace teleop
