#include <gtest/gtest.h>
#include "infra/token_bucket.hpp"
#include <thread>

using namespace teleop;

TEST(TokenBucketTest, InitialBurstAllowed) {
    TokenBucket bucket(10, 5);
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(bucket.tryConsume());
    }
    EXPECT_FALSE(bucket.tryConsume());
}

TEST(TokenBucketTest, SingleTokenConsumption) {
    TokenBucket bucket(5, 10);
    EXPECT_TRUE(bucket.tryConsume());
    EXPECT_TRUE(bucket.tryConsume());
}

TEST(TokenBucketTest, RefillOverTime) {
    TokenBucket bucket(2, 100);  // Fast refill
    EXPECT_TRUE(bucket.tryConsume());
    EXPECT_TRUE(bucket.tryConsume());
    EXPECT_FALSE(bucket.tryConsume());

    // Wait for refill
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(bucket.tryConsume());
}

TEST(TokenBucketTest, CustomCost) {
    TokenBucket bucket(10, 5);
    EXPECT_TRUE(bucket.tryConsume(5.0f));
    EXPECT_TRUE(bucket.tryConsume(5.0f));
    EXPECT_FALSE(bucket.tryConsume(1.0f));
}

TEST(TokenBucketTest, MaxTokensCapped) {
    TokenBucket bucket(5, 100);
    // Wait for potential over-refill
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Should still only have 5 max
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(bucket.tryConsume());
    }
    EXPECT_FALSE(bucket.tryConsume());
}
