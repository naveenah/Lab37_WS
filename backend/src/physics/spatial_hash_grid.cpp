#include "physics/spatial_hash_grid.hpp"
#include <algorithm>

namespace teleop {

SpatialHashGrid::SpatialHashGrid(float cellSize)
    : cellSize_(cellSize) {}

void SpatialHashGrid::clear() {
    cells_.clear();
}

void SpatialHashGrid::insert(entt::entity entity, const AABB& aabb) {
    int minCx = static_cast<int>(std::floor(aabb.min.x / cellSize_));
    int minCy = static_cast<int>(std::floor(aabb.min.y / cellSize_));
    int maxCx = static_cast<int>(std::floor(aabb.max.x / cellSize_));
    int maxCy = static_cast<int>(std::floor(aabb.max.y / cellSize_));

    for (int cx = minCx; cx <= maxCx; ++cx) {
        for (int cy = minCy; cy <= maxCy; ++cy) {
            cells_[{cx, cy}].push_back(entity);
        }
    }
}

std::vector<std::pair<entt::entity, entt::entity>> SpatialHashGrid::getCandidatePairs() const {
    std::vector<std::pair<entt::entity, entt::entity>> pairs;
    std::unordered_set<uint64_t> seen;

    for (const auto& [key, entities] : cells_) {
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                auto a = std::min(entities[i], entities[j]);
                auto b = std::max(entities[i], entities[j]);
                uint64_t pairKey = (static_cast<uint64_t>(static_cast<uint32_t>(a)) << 32)
                                 | static_cast<uint32_t>(static_cast<uint32_t>(b));
                if (seen.insert(pairKey).second) {
                    pairs.emplace_back(a, b);
                }
            }
        }
    }
    return pairs;
}

} // namespace teleop
