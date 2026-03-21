#include <gtest/gtest.h>
#include "infra/spsc_command_source.hpp"

using namespace teleop;

TEST(CommandRoundtripTest, EnqueueDequeue) {
    SPSCCommandSource queue(1024);
    CommandMessage cmd{CommandType::Control, 0.5f, 0.3f, 1, 1000};
    EXPECT_TRUE(queue.enqueue(cmd));

    CommandMessage received;
    EXPECT_TRUE(queue.tryDequeue(received));
    EXPECT_FLOAT_EQ(received.throttle, 0.5f);
    EXPECT_FLOAT_EQ(received.steering, 0.3f);
    EXPECT_EQ(received.sequence, 1u);
}

TEST(CommandRoundtripTest, EmptyQueueReturnsFalse) {
    SPSCCommandSource queue(1024);
    CommandMessage received;
    EXPECT_FALSE(queue.tryDequeue(received));
}

TEST(CommandRoundtripTest, MultipleCommands) {
    SPSCCommandSource queue(1024);
    for (uint32_t i = 0; i < 10; ++i) {
        CommandMessage cmd{CommandType::Control, 0.1f * static_cast<float>(i), 0.0f, i, i * 100};
        EXPECT_TRUE(queue.enqueue(cmd));
    }

    for (uint32_t i = 0; i < 10; ++i) {
        CommandMessage received;
        EXPECT_TRUE(queue.tryDequeue(received));
        EXPECT_EQ(received.sequence, i);
    }

    // Queue should be empty now
    CommandMessage extra;
    EXPECT_FALSE(queue.tryDequeue(extra));
}

TEST(CommandRoundtripTest, FIFOOrder) {
    SPSCCommandSource queue(1024);
    queue.enqueue({CommandType::Control, 0.1f, 0.0f, 1, 100});
    queue.enqueue({CommandType::Control, 0.2f, 0.0f, 2, 200});
    queue.enqueue({CommandType::Control, 0.3f, 0.0f, 3, 300});

    CommandMessage r1, r2, r3;
    queue.tryDequeue(r1);
    queue.tryDequeue(r2);
    queue.tryDequeue(r3);

    EXPECT_EQ(r1.sequence, 1u);
    EXPECT_EQ(r2.sequence, 2u);
    EXPECT_EQ(r3.sequence, 3u);
}
