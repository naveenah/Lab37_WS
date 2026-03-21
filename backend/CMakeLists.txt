cmake_minimum_required(VERSION 3.28)
project(teleop-simulator VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── vcpkg dependencies ──────────────────────────────────
find_package(EnTT CONFIG REQUIRED)
find_package(flatbuffers CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS headers)
find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

# ── FetchContent for libraries not in vcpkg ─────────────
include(FetchContent)

# uWebSockets + uSockets
FetchContent_Declare(
    usockets
    GIT_REPOSITORY https://github.com/uNetworking/uSockets.git
    GIT_TAG v0.8.8
)

FetchContent_Declare(
    uwebsockets
    GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
    GIT_TAG v20.70.0
)

FetchContent_MakeAvailable(usockets)

# Build uSockets as a static library
file(GLOB USOCKETS_SOURCES ${usockets_SOURCE_DIR}/src/*.c ${usockets_SOURCE_DIR}/src/eventing/*.c)
add_library(uSockets STATIC ${USOCKETS_SOURCES})
target_include_directories(uSockets PUBLIC ${usockets_SOURCE_DIR}/src)
target_compile_definitions(uSockets PRIVATE LIBUS_NO_SSL)

FetchContent_MakeAvailable(uwebsockets)

# uWebSockets is header-only
add_library(uWebSockets INTERFACE)
target_include_directories(uWebSockets INTERFACE ${uwebsockets_SOURCE_DIR}/src)
target_link_libraries(uWebSockets INTERFACE uSockets)

# RapidCheck for property-based testing
FetchContent_Declare(
    rapidcheck
    GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
    GIT_TAG ff6af6fc683159deb51c543b065eba14dfcf329b
)
set(RC_ENABLE_GTEST ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(rapidcheck)

# ── FlatBuffers code generation ─────────────────────────
set(FLATBUFFERS_SCHEMA ${CMAKE_SOURCE_DIR}/../proto/teleop.fbs)
set(FLATBUFFERS_GENERATED_DIR ${CMAKE_SOURCE_DIR}/src/generated)

find_program(FLATC flatc HINTS ${flatbuffers_DIR}/../../../tools/flatbuffers)

if(FLATC)
    add_custom_command(
        OUTPUT ${FLATBUFFERS_GENERATED_DIR}/teleop_generated.h
        COMMAND ${FLATC} --cpp -o ${FLATBUFFERS_GENERATED_DIR} ${FLATBUFFERS_SCHEMA}
        DEPENDS ${FLATBUFFERS_SCHEMA}
        COMMENT "Generating FlatBuffers C++ header"
    )
else()
    message(WARNING "flatc not found - using flatbuffers library's flatc target")
    add_custom_command(
        OUTPUT ${FLATBUFFERS_GENERATED_DIR}/teleop_generated.h
        COMMAND flatbuffers::flatc --cpp -o ${FLATBUFFERS_GENERATED_DIR} ${FLATBUFFERS_SCHEMA}
        DEPENDS ${FLATBUFFERS_SCHEMA}
        COMMENT "Generating FlatBuffers C++ header"
    )
endif()

add_custom_target(flatbuffers_generated DEPENDS ${FLATBUFFERS_GENERATED_DIR}/teleop_generated.h)

# ── Main server executable ──────────────────────────────
set(SERVER_SOURCES
    src/main.cpp
    src/physics/sat_detector.cpp
    src/physics/spatial_hash_grid.cpp
    src/infra/spdlog_logger.cpp
    src/infra/in_process_bus.cpp
    src/infra/ecs_state_store.cpp
    src/infra/config_loader.cpp
    src/infra/scene_loader.cpp
    src/infra/websocket_gateway.cpp
    src/protocol/state_serializer.cpp
    src/engine/simulation_engine.cpp
    src/engine/systems/input_system.cpp
    src/engine/systems/kinematics_system.cpp
    src/engine/systems/movement_system.cpp
    src/engine/systems/transform_system.cpp
    src/engine/systems/collision_system.cpp
    src/engine/systems/scoring_system.cpp
    src/engine/systems/broadcast_system.cpp
    src/engine/systems/logging_system.cpp
)

add_executable(teleop-server ${SERVER_SOURCES})
add_dependencies(teleop-server flatbuffers_generated)

target_include_directories(teleop-server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${FLATBUFFERS_GENERATED_DIR}
)

target_link_libraries(teleop-server PRIVATE
    EnTT::EnTT
    flatbuffers::flatbuffers
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    Boost::headers
    uWebSockets
    z
)

# ── Unit tests ──────────────────────────────────────────
set(UNIT_TEST_SOURCES
    tests/unit/vec2_test.cpp
    tests/unit/aabb_test.cpp
    tests/unit/sat_detector_test.cpp
    tests/unit/spatial_hash_test.cpp
    tests/unit/ackermann_test.cpp
    tests/unit/command_validator_test.cpp
    tests/unit/token_bucket_test.cpp
    src/physics/sat_detector.cpp
    src/physics/spatial_hash_grid.cpp
)

add_executable(teleop-tests ${UNIT_TEST_SOURCES})

target_include_directories(teleop-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${FLATBUFFERS_GENERATED_DIR}
)

target_link_libraries(teleop-tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    EnTT::EnTT
    Boost::headers
)

include(GoogleTest)
gtest_discover_tests(teleop-tests)

# ── Property tests ──────────────────────────────────────
set(PROPERTY_TEST_SOURCES
    tests/property/sat_property_test.cpp
    tests/property/ackermann_property_test.cpp
    tests/property/spatial_hash_property_test.cpp
    src/physics/sat_detector.cpp
    src/physics/spatial_hash_grid.cpp
)

add_executable(teleop-property-tests ${PROPERTY_TEST_SOURCES})

target_include_directories(teleop-property-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${FLATBUFFERS_GENERATED_DIR}
)

target_link_libraries(teleop-property-tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    rapidcheck
    rapidcheck_gtest
    EnTT::EnTT
    Boost::headers
)

gtest_discover_tests(teleop-property-tests)

# ── Integration tests ───────────────────────────────────
set(INTEGRATION_TEST_SOURCES
    tests/integration/command_roundtrip_test.cpp
    tests/integration/collision_flow_test.cpp
    src/physics/sat_detector.cpp
    src/physics/spatial_hash_grid.cpp
    src/infra/ecs_state_store.cpp
    src/infra/scene_loader.cpp
    src/protocol/state_serializer.cpp
    src/engine/systems/input_system.cpp
    src/engine/systems/kinematics_system.cpp
    src/engine/systems/movement_system.cpp
    src/engine/systems/transform_system.cpp
    src/engine/systems/collision_system.cpp
    src/engine/systems/scoring_system.cpp
    src/engine/systems/broadcast_system.cpp
    src/engine/systems/logging_system.cpp
)

add_executable(teleop-integration-tests ${INTEGRATION_TEST_SOURCES})
add_dependencies(teleop-integration-tests flatbuffers_generated)

target_include_directories(teleop-integration-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${FLATBUFFERS_GENERATED_DIR}
)

target_link_libraries(teleop-integration-tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    EnTT::EnTT
    flatbuffers::flatbuffers
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    Boost::headers
)

gtest_discover_tests(teleop-integration-tests)

# ── Benchmark tests ─────────────────────────────────────
set(BENCHMARK_SOURCES
    tests/benchmarks/sat_benchmark.cpp
    tests/benchmarks/spatial_hash_benchmark.cpp
    tests/benchmarks/full_tick_benchmark.cpp
    tests/benchmarks/flatbuffer_benchmark.cpp
    src/physics/sat_detector.cpp
    src/physics/spatial_hash_grid.cpp
    src/infra/ecs_state_store.cpp
    src/infra/scene_loader.cpp
    src/protocol/state_serializer.cpp
    src/engine/systems/input_system.cpp
    src/engine/systems/kinematics_system.cpp
    src/engine/systems/movement_system.cpp
    src/engine/systems/transform_system.cpp
    src/engine/systems/collision_system.cpp
    src/engine/systems/scoring_system.cpp
    src/engine/systems/broadcast_system.cpp
    src/engine/systems/logging_system.cpp
)

add_executable(teleop-benchmarks ${BENCHMARK_SOURCES})
add_dependencies(teleop-benchmarks flatbuffers_generated)

target_include_directories(teleop-benchmarks PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${FLATBUFFERS_GENERATED_DIR}
)

target_link_libraries(teleop-benchmarks PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
    EnTT::EnTT
    flatbuffers::flatbuffers
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    Boost::headers
)
