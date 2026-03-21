#include "infra/websocket_gateway.hpp"
#include <App.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <string>

namespace teleop {

using json = nlohmann::json;

struct PerSocketData {
    // Per-connection state can be added here
};

WebSocketGateway::WebSocketGateway(
    uint16_t port,
    SPSCCommandSource& commandSource,
    IMessageBus& messageBus,
    ILogger& logger,
    const Config& config,
    std::function<void()> onReset
)
    : port_(port)
    , commandSource_(commandSource)
    , messageBus_(messageBus)
    , logger_(logger)
    , rateLimiter_(config.rateLimiting.maxTokens, config.rateLimiting.refillRate)
    , onReset_(std::move(onReset))
{
    setupBroadcastSubscription();
}

WebSocketGateway::~WebSocketGateway() {
    stop();
}

void WebSocketGateway::setupBroadcastSubscription() {
    messageBus_.subscribe(Topic::StateBroadcast, [this](std::span<const uint8_t> data) {
        std::lock_guard<std::mutex> lock(broadcastMutex_);
        broadcastBuffer_.assign(data.begin(), data.end());
        hasPendingBroadcast_.store(true, std::memory_order_release);
    });
}

void WebSocketGateway::run() {
    running_.store(true, std::memory_order_release);

    uWS::App app;
    app_ = &app;

    app.ws<PerSocketData>("/*", {
            .compression = uWS::DISABLED,
            .maxPayloadLength = 16 * 1024,
            .idleTimeout = 120,
            .maxBackpressure = 1 * 1024 * 1024,

            .open = [this](auto* ws) {
                logger_.info("Client connected");
                ws->subscribe("broadcast");

                // Reset sequence tracking for new client
                lastSequence_ = 0;
                lastTimestamp_ = 0;

                // Send welcome message
                json welcome = {
                    {"type", "welcome"},
                    {"tickRate", 60},
                    {"broadcastRate", 30},
                    {"sceneId", "default"},
                    {"protocolVersion", 1},
                    {"serverVersion", "1.0.0"}
                };
                ws->send(welcome.dump(), uWS::OpCode::TEXT);
            },

            .message = [this](auto* ws, std::string_view message, uWS::OpCode opCode) {
                if (opCode != uWS::OpCode::TEXT) return;

                try {
                    json msg = json::parse(message);
                    std::string type = msg.value("type", "");

                    if (type == "command") {
                        if (!rateLimiter_.tryConsume()) {
                            json error = {
                                {"type", "error"},
                                {"code", "RATE_LIMITED"},
                                {"message", "Command rate exceeded"}
                            };
                            ws->send(error.dump(), uWS::OpCode::TEXT);
                            return;
                        }

                        CommandMessage cmd;
                        cmd.type = CommandType::Control;
                        cmd.throttle = msg.value("throttle", 0.0f);
                        cmd.steering = msg.value("steering", 0.0f);
                        cmd.sequence = msg.value("sequence", 0u);
                        cmd.timestamp = msg.value("timestamp", 0ull);

                        auto result = CommandValidator::validate(cmd, lastSequence_, lastTimestamp_);
                        if (!result.valid) {
                            json error = {
                                {"type", "error"},
                                {"code", result.errorCode},
                                {"message", result.errorMessage}
                            };
                            ws->send(error.dump(), uWS::OpCode::TEXT);
                            logger_.debug("Command rejected: " + result.errorCode);
                            return;
                        }

                        lastSequence_ = cmd.sequence;
                        lastTimestamp_ = cmd.timestamp;
                        commandSource_.enqueue(cmd);

                    } else if (type == "ping") {
                        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()
                        ).count();

                        json pong = {
                            {"type", "pong"},
                            {"clientTimestamp", msg.value("timestamp", 0ull)},
                            {"serverTimestamp", static_cast<uint64_t>(now)}
                        };
                        ws->send(pong.dump(), uWS::OpCode::TEXT);

                    } else if (type == "reset") {
                        logger_.info("Reset requested by client");
                        if (onReset_) {
                            onReset_();
                            lastSequence_ = 0;
                            lastTimestamp_ = 0;
                            json ack = {
                                {"type", "event"},
                                {"event", "reset"},
                                {"data", {}}
                            };
                            ws->send(ack.dump(), uWS::OpCode::TEXT);
                        }

                    } else if (type == "handshake") {
                        logger_.info("Client handshake received");
                    }

                } catch (const json::parse_error& e) {
                    logger_.warn("Failed to parse client message: " + std::string(e.what()));
                    json error = {
                        {"type", "error"},
                        {"code", "PARSE_ERROR"},
                        {"message", "Invalid JSON"}
                    };
                    ws->send(error.dump(), uWS::OpCode::TEXT);
                }
            },

            .close = [this](auto* ws, int code, std::string_view message) {
                logger_.info("Client disconnected (code: " + std::to_string(code) + ")");
            }
        })
        .listen(port_, [this](auto* listenSocket) {
            if (listenSocket) {
                listenSocket_ = listenSocket;
                logger_.info("WebSocket server listening on port " + std::to_string(port_));
            } else {
                logger_.error("Failed to listen on port " + std::to_string(port_));
            }
        });

    // Create a timer on the uWS event loop to drain broadcast buffer
    auto* loop = uWS::Loop::get();
    struct us_timer_t* timer = us_create_timer((struct us_loop_t*)loop, 0, sizeof(void*));
    *static_cast<WebSocketGateway**>(us_timer_ext(timer)) = this;

    us_timer_set(timer, [](struct us_timer_t* t) {
        auto* self = *static_cast<WebSocketGateway**>(us_timer_ext(t));
        if (self->hasPendingBroadcast_.exchange(false, std::memory_order_acquire)) {
            std::vector<uint8_t> data;
            {
                std::lock_guard<std::mutex> lock(self->broadcastMutex_);
                data = self->broadcastBuffer_;
            }
            if (!data.empty()) {
                auto* a = static_cast<uWS::App*>(self->app_);
                a->publish("broadcast",
                    std::string_view(reinterpret_cast<const char*>(data.data()), data.size()),
                    uWS::OpCode::BINARY);
            }
        }
    }, 16, 16); // Check every ~16ms (~60Hz)

    app.run();

    us_timer_close(timer);
    app_ = nullptr;
    running_.store(false, std::memory_order_release);
}

void WebSocketGateway::stop() {
    if (listenSocket_) {
        us_listen_socket_close(0, listenSocket_);
        listenSocket_ = nullptr;
    }
    running_.store(false, std::memory_order_release);
}

} // namespace teleop
