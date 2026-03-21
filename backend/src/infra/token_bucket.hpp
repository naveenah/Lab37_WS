#pragma once

#include <chrono>
#include <algorithm>

namespace teleop {

class TokenBucket {
public:
    TokenBucket(float maxTokens, float refillRate)
        : tokens_(maxTokens)
        , maxTokens_(maxTokens)
        , refillRate_(refillRate)
        , lastRefill_(std::chrono::steady_clock::now())
    {}

    bool tryConsume(float cost = 1.0f) {
        refill();
        if (tokens_ >= cost) {
            tokens_ -= cost;
            return true;
        }
        return false;
    }

    float availableTokens() const { return tokens_; }

private:
    void refill() {
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - lastRefill_).count();
        tokens_ = std::min(maxTokens_, tokens_ + elapsed * refillRate_);
        lastRefill_ = now;
    }

    float tokens_;
    float maxTokens_;
    float refillRate_;
    std::chrono::steady_clock::time_point lastRefill_;
};

} // namespace teleop
