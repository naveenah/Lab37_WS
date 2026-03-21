#include <gtest/gtest.h>
#include "protocol/command_validator.hpp"

using namespace teleop;

TEST(CommandValidatorTest, ValidCommandAccepted) {
    auto result = CommandValidator::validate(
        {CommandType::Control, 0.5f, 0.3f, 1, 1000}, 0, 0);
    EXPECT_TRUE(result.valid);
}

TEST(CommandValidatorTest, ThrottleOutOfRangeHigh) {
    auto result = CommandValidator::validate(
        {CommandType::Control, 1.5f, 0.0f, 1, 1000}, 0, 0);
    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.errorCode, "INVALID_THROTTLE");
}

TEST(CommandValidatorTest, ThrottleOutOfRangeLow) {
    auto result = CommandValidator::validate(
        {CommandType::Control, -1.5f, 0.0f, 1, 1000}, 0, 0);
    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.errorCode, "INVALID_THROTTLE");
}

TEST(CommandValidatorTest, SteeringOutOfRange) {
    auto result = CommandValidator::validate(
        {CommandType::Control, 0.5f, 2.0f, 1, 1000}, 0, 0);
    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.errorCode, "INVALID_STEERING");
}

TEST(CommandValidatorTest, SequenceRegression) {
    auto result = CommandValidator::validate(
        {CommandType::Control, 0.5f, 0.0f, 5, 1000}, 10, 500);
    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.errorCode, "STALE_COMMAND");
}

TEST(CommandValidatorTest, SequenceGapTooLarge) {
    auto result = CommandValidator::validate(
        {CommandType::Control, 0.5f, 0.0f, 2000, 1000}, 1, 500);
    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.errorCode, "SEQUENCE_GAP");
}

TEST(CommandValidatorTest, TimestampRegression) {
    auto result = CommandValidator::validate(
        {CommandType::Control, 0.5f, 0.0f, 2, 100}, 1, 500);
    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.errorCode, "TIMESTAMP_REGRESSION");
}

TEST(CommandValidatorTest, BoundaryValuesAccepted) {
    auto result = CommandValidator::validate(
        {CommandType::Control, 1.0f, -1.0f, 1, 1000}, 0, 0);
    EXPECT_TRUE(result.valid);
}

TEST(CommandValidatorTest, NegativeThrottleAccepted) {
    auto result = CommandValidator::validate(
        {CommandType::Control, -1.0f, 0.0f, 1, 1000}, 0, 0);
    EXPECT_TRUE(result.valid);
}

TEST(CommandValidatorTest, FirstCommandAlwaysValid) {
    auto result = CommandValidator::validate(
        {CommandType::Control, 0.0f, 0.0f, 1, 1000}, 0, 0);
    EXPECT_TRUE(result.valid);
}
