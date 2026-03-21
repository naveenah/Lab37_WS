#pragma once

#include <string_view>
#include <string>

namespace teleop {

class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void trace(std::string_view msg) = 0;
    virtual void debug(std::string_view msg) = 0;
    virtual void info(std::string_view msg) = 0;
    virtual void warn(std::string_view msg) = 0;
    virtual void error(std::string_view msg) = 0;
    virtual void critical(std::string_view msg) = 0;

    // Structured event logging
    virtual void event(std::string_view category, std::string_view jsonData) = 0;
};

} // namespace teleop
