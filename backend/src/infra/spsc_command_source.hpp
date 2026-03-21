#pragma once

#include "interfaces/i_command_source.hpp"
#include "protocol/command_message.hpp"
#include <boost/lockfree/spsc_queue.hpp>

namespace teleop {

class SPSCCommandSource : public ICommandSource {
public:
    explicit SPSCCommandSource(size_t capacity = 1024)
        : queue_(capacity) {}

    bool enqueue(const CommandMessage& cmd) {
        return queue_.push(cmd);
    }

    bool tryDequeue(CommandMessage& cmd) override {
        return queue_.pop(cmd);
    }

    bool empty() {
        return queue_.empty();
    }

private:
    boost::lockfree::spsc_queue<CommandMessage> queue_;
};

} // namespace teleop
