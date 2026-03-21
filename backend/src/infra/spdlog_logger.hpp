#pragma once

#include "interfaces/i_logger.hpp"
#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace teleop {

class SpdlogLogger : public ILogger {
public:
    SpdlogLogger(const std::string& name, const std::string& logLevel = "info",
                 const std::string& logDir = "./logs");
    ~SpdlogLogger() override = default;

    void trace(std::string_view msg) override;
    void debug(std::string_view msg) override;
    void info(std::string_view msg) override;
    void warn(std::string_view msg) override;
    void error(std::string_view msg) override;
    void critical(std::string_view msg) override;
    void event(std::string_view category, std::string_view jsonData) override;

private:
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace teleop
