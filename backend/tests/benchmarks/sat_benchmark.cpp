#include <benchmark/benchmark.h>
#include "physics/sat_detector.hpp"
#include <vector>
#include <cmath>

using namespace teleop;

namespace {

std::vector<Vec2> makeSquare(float cx, float cy, float halfSize) {
    return {
        {cx - halfSize, cy - halfSize},
        {cx + halfSize, cy - halfSize},
        {cx + halfSize, cy + halfSize},
        {cx - halfSize, cy + halfSize},
    };
}

std::vector<Vec2> makeRegularPolygon(float cx, float cy, float radius, int n) {
    std::vector<Vec2> verts;
    verts.reserve(n);
    for (int i = 0; i < n; ++i) {
        float angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(n);
        verts.push_back({cx + radius * std::cos(angle), cy + radius * std::sin(angle)});
    }
    return verts;
}

} // namespace

static void BM_SATTestSquares(benchmark::State& state) {
    auto a = makeSquare(0, 0, 1.0f);
    auto b = makeSquare(0.5f, 0.5f, 1.0f);
    SATDetector sat;

    for (auto _ : state) {
        benchmark::DoNotOptimize(sat.test(a, b));
    }
}
BENCHMARK(BM_SATTestSquares);

static void BM_SATTestSeparated(benchmark::State& state) {
    auto a = makeSquare(0, 0, 1.0f);
    auto b = makeSquare(10, 10, 1.0f);
    SATDetector sat;

    for (auto _ : state) {
        benchmark::DoNotOptimize(sat.test(a, b));
    }
}
BENCHMARK(BM_SATTestSeparated);

static void BM_SATTestComplexPolygons(benchmark::State& state) {
    int vertexCount = static_cast<int>(state.range(0));
    auto a = makeRegularPolygon(0, 0, 1.0f, vertexCount);
    auto b = makeRegularPolygon(0.5f, 0.5f, 1.0f, vertexCount);
    SATDetector sat;

    for (auto _ : state) {
        benchmark::DoNotOptimize(sat.test(a, b));
    }
}
BENCHMARK(BM_SATTestComplexPolygons)->Range(3, 16);
