#include <benchmark/benchmark.h>
#include "protocol/state_serializer.hpp"
#include "protocol/entity_snapshot.hpp"
#include <random>
#include <cmath>

using namespace teleop;

namespace {

std::vector<EntitySnapshot> generateSnapshots(int count) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> posDist(-50.0f, 50.0f);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * static_cast<float>(M_PI));

    std::vector<EntitySnapshot> snapshots;
    snapshots.reserve(count);

    for (int i = 0; i < count; ++i) {
        EntitySnapshot snap;
        snap.entityId = static_cast<uint32_t>(i);
        snap.entityType = (i == 0) ? EntityType::Robot :
                          (i % 3 == 0) ? EntityType::DynamicObstacle :
                          EntityType::StaticObstacle;
        snap.x = posDist(rng);
        snap.y = posDist(rng);
        snap.heading = angleDist(rng);
        snap.color = 0xFF0000FF;
        // 4-vertex polygon
        float size = 1.0f;
        snap.vertices = {
            {-size, -size}, {size, -size}, {size, size}, {-size, size}
        };
        snapshots.push_back(std::move(snap));
    }
    return snapshots;
}

} // namespace

static void BM_FlatBufferSerialize(benchmark::State& state) {
    int entityCount = static_cast<int>(state.range(0));
    auto entities = generateSnapshots(entityCount);
    ScoreState score{3, 1000};
    StateSerializer serializer;

    for (auto _ : state) {
        auto [ptr, size] = serializer.serialize(entities, score, 0, 0, 2.5f, 0.3f);
        benchmark::DoNotOptimize(ptr);
        benchmark::DoNotOptimize(size);
    }
    state.SetItemsProcessed(state.iterations() * entityCount);
}
BENCHMARK(BM_FlatBufferSerialize)->Range(20, 2000);
