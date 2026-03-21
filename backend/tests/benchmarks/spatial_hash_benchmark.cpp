#include <benchmark/benchmark.h>
#include "physics/spatial_hash_grid.hpp"
#include <random>

using namespace teleop;

static void BM_SpatialHashInsertAndQuery(benchmark::State& state) {
    int entityCount = static_cast<int>(state.range(0));
    SpatialHashGrid grid(5.0f);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-50.0f, 50.0f);

    std::vector<std::pair<entt::entity, AABB>> entities;
    for (int i = 0; i < entityCount; ++i) {
        float x = dist(rng), y = dist(rng);
        entities.push_back({
            static_cast<entt::entity>(i),
            AABB{{x, y}, {x + 1, y + 1}}
        });
    }

    for (auto _ : state) {
        grid.clear();
        for (const auto& [e, aabb] : entities) {
            grid.insert(e, aabb);
        }
        benchmark::DoNotOptimize(grid.getCandidatePairs());
    }
    state.SetItemsProcessed(state.iterations() * entityCount);
}
BENCHMARK(BM_SpatialHashInsertAndQuery)->Range(20, 2000);
