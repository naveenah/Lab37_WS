#pragma once

#include <cmath>
#include <limits>

namespace teleop {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(float x, float y) : x(x), y(y) {}

    constexpr Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }
    constexpr Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }
    constexpr Vec2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
    constexpr Vec2 operator/(float scalar) const { return {x / scalar, y / scalar}; }
    constexpr Vec2 operator-() const { return {-x, -y}; }

    constexpr Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    constexpr Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
    constexpr Vec2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }

    constexpr bool operator==(const Vec2& other) const {
        return x == other.x && y == other.y;
    }

    constexpr float dot(const Vec2& other) const {
        return x * other.x + y * other.y;
    }

    constexpr float cross(const Vec2& other) const {
        return x * other.y - y * other.x;
    }

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    float lengthSquared() const {
        return x * x + y * y;
    }

    Vec2 normalized() const {
        float len = length();
        if (len < std::numeric_limits<float>::epsilon()) {
            return {0.0f, 0.0f};
        }
        return {x / len, y / len};
    }

    constexpr Vec2 perpendicular() const {
        return {-y, x};
    }

    float distance(const Vec2& other) const {
        return (*this - other).length();
    }

    float distanceSquared(const Vec2& other) const {
        return (*this - other).lengthSquared();
    }

    static Vec2 fromAngle(float radians) {
        return {std::cos(radians), std::sin(radians)};
    }
};

inline constexpr Vec2 operator*(float scalar, const Vec2& v) {
    return {scalar * v.x, scalar * v.y};
}

} // namespace teleop
