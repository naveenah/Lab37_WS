#pragma once

#include "math/aabb.hpp"
#include <entt/entt.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <cmath>

namespace teleop {

class SpatialHashGrid {
public:
    explicit SpatialHashGrid(float cellSize);

    void clear();
    void insert(entt::entity entity, const AABB& aabb);
    std::vector<std::pair<entt::entity, entt::entity>> getCandidatePairs() const;

private:
    struct CellKey {
        int32_t cx, cy;
        bool operator==(const CellKey&) const = default;
    };

    struct CellKeyHash {
        size_t operator()(const CellKey& k) const {
            return std::hash<int64_t>{}(
                (static_cast<int64_t>(k.cx) << 32) | static_cast<uint32_t>(k.cy)
            );
        }
    };

    float cellSize_;
    std::unordered_map<CellKey, std::vector<entt::entity>, CellKeyHash> cells_;
};

} // namespace teleop
