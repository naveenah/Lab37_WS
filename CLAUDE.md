# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Real-time robot teleoperation simulator — a client-server system where a browser-based frontend sends driving commands to a C++20 backend that runs a 60 Hz physics simulation. The backend broadcasts world state to clients via WebSocket using FlatBuffers binary serialization.

## Architecture

**Two-process, two-thread model:**

- **Backend (C++20):** Simulation engine using EnTT ECS. Two threads communicate via a lock-free SPSC queue:
  - *Simulation thread (main)* — fixed 60 Hz timestep: dequeues commands → runs 8 ECS systems in order (Input → Kinematics → Movement → Transform → Collision → Scoring → Broadcast → Logging)
  - *Network I/O thread* — uWebSockets event loop: receives JSON commands, broadcasts FlatBuffers state via pub/sub topics

- **Frontend (TypeScript/React 18):** Canvas-based renderer with Zustand state management. Sends JSON commands (~10 Hz), receives FlatBuffers world state (30 Hz).

**Protocol:** Hybrid — JSON text (client→server commands) and FlatBuffers binary (server→client world state).

**Physics:** Custom SAT narrow-phase + spatial hash grid broad-phase. Ackermann steering model for robot kinematics. Collision response: zero velocity + increment impact counter.

**Cross-thread communication patterns:**
- Commands flow: WebSocket gateway → SPSC queue → InputSystem (network thread to sim thread)
- State broadcasts flow: BroadcastSystem → IMessageBus → WebSocket gateway subscriber → uWS timer drains buffer and publishes to "broadcast" topic (sim thread to network thread)
- Scene reset: WebSocket gateway calls `engine.requestReset()` via atomic flag (no SPSC queue — avoids draining command queue)

## Build & Run Commands

### Docker (recommended)
```bash
docker compose up --build          # Build and start both services
docker compose down                # Stop
docker compose up --build          # Rebuild after changes
```

### Backend (C++20, CMake 3.28+, vcpkg)
```bash
cd backend && mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
make -j$(nproc)
./teleop-server --config ../../config/default.json
```

### Frontend (TypeScript, Vite 5.x, Node 20+)
```bash
cd frontend
npm install
npm run dev          # Dev server at :3000
npm run build        # Production build
```

## Testing

### Backend (GoogleTest, RapidCheck, Google Benchmark)
```bash
cd backend/build
ctest --output-on-failure            # All tests
ctest -R <test_name>                 # Single test (e.g. sat_detector_test, vec2_test)
ctest -R property                    # Property-based tests only
ctest -R integration                 # Integration tests only
./teleop-benchmarks                  # Performance benchmarks
```

### Frontend (Vitest, fast-check, Playwright)
```bash
cd frontend
npm test                             # Unit + property tests (Vitest)
npx vitest run tests/unit/<file>     # Single test file
npm run test:watch                   # Watch mode
npx playwright test                  # E2E tests (requires backend running)
```

## Key Design Decisions

- **Deterministic physics** — fixed timestep ensures reproducible behavior regardless of frame rate
- **Lock-free concurrency** — SPSC queue with zero allocations between simulation and network threads
- **Zero-copy deserialization** — FlatBuffers lets clients read state without parsing
- **Composition over inheritance** — ECS avoids deep class hierarchies; all dependencies injected through interfaces (IMessageBus, IStateStore, ICommandSource, ICollisionDetector, IRobotKinematics, ILogger)
- **Performance budgets** — physics <5ms at 1000 entities, serialization <500us, rendering <16ms, end-to-end latency <50ms
- **Steering inversion** — frontend inverts steering input because the Ackermann model uses standard math convention (positive = counter-clockwise) while users expect positive = visual right turn

## Important Gotchas

- **uWebSockets includes:** Use `#include <App.h>` not `#include <uWebSockets/App.h>` — FetchContent puts headers directly in `src/`
- **uWS timer API:** Requires C-style cast `(struct us_loop_t*)uWS::Loop::get()` for timer creation
- **boost::lockfree::spsc_queue::empty()** is not const-qualified — wrapper methods cannot be const
- **RapidCheck + GCC 13:** `rc::gen::inRange` does not work with float types (triggers invalid `std::make_unsigned<float>`). Use integer ranges mapped to floats via `rc::gen::map`
- **React StrictMode:** Dev mode double-mounts components, causing the first WebSocket to close before opening. ConnectionManager.disconnect() must check readyState before closing
- **Sequence tracking on reconnect:** WebSocket gateway resets `lastSequence_` and `lastTimestamp_` in the `.open` handler to avoid "stale command" errors from a previous session
- **Docker ARM64:** Dockerfile auto-detects architecture via `uname -m` for correct vcpkg triplet (arm64-linux vs x64-linux)
- **Vitest vs Playwright:** `vite.config.ts` excludes `tests/e2e/**` from Vitest to prevent it from picking up Playwright tests

## Key Dependencies

| Backend | Frontend |
|---------|----------|
| EnTT 3.13+ (ECS) | React 18.3+ |
| uWebSockets 20.x (FetchContent) | Zustand 4.5+ (state) |
| RapidCheck (FetchContent) | flatbuffers (npm) |
| FlatBuffers 24.x | Vite 5.x |
| nlohmann/json 3.11+ | Vitest 1.x |
| spdlog 1.12+ | fast-check 3.x |
| boost::lockfree 1.83+ | Playwright 1.42+ |

## Scene Configuration

Scene files live in `config/`. The robot, static obstacles, and dynamic obstacles (with waypoint paths) are defined in JSON. The `--config` flag selects which scene to load. The scene can be reset at runtime via the Reset button (sends `{"type": "reset"}` over WebSocket → atomic flag → engine clears registry and reloads scene).

## Reference

- `docs/DDD_Robot_Teleoperation_Simulation.md` — comprehensive design document (3400 lines) with UML, component specs, system execution order, protocol schemas, and testing strategy
- `docs/Implementation_Plan.md` — 8-phase implementation plan with dependency graph and deviation notes
- `docs/Tech_Stack_Considerations_and_Assumptions.pdf` — technology choice justifications
