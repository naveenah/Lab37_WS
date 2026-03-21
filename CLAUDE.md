# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Real-time robot teleoperation simulator — a client-server system where a browser-based frontend sends driving commands to a C++20 backend that runs a 60 Hz physics simulation. The backend broadcasts world state to clients via WebSocket using FlatBuffers binary serialization.

## Architecture

**Two-process, two-thread model:**

- **Backend (C++20):** Simulation engine using EnTT ECS. Two threads communicate via a lock-free SPSC queue:
  - *Simulation thread* — fixed 60 Hz timestep: dequeues commands → runs ECS systems (input → kinematics → movement → transform → collision → scoring → broadcast → logging) → enqueues serialized state
  - *Network I/O thread* — uWebSockets event loop: receives JSON commands, broadcasts FlatBuffers state

- **Frontend (TypeScript/React 18):** Canvas-based renderer with Zustand state management. Sends JSON commands (~10 Hz), receives FlatBuffers world state (30-60 Hz).

**Protocol:** Hybrid — JSON text (client→server commands, <100 bytes) and FlatBuffers binary (server→client state, ~20KB/frame at 1000 entities).

**Physics:** Custom SAT narrow-phase + spatial hash grid broad-phase. Ackermann steering model for robot kinematics. Collision response: zero velocity + increment impact counter.

**ECS entity archetypes:** Robot (Transform, PolygonShape, Velocity, RobotTag, CollisionState, RenderMeta), Static Obstacle, Dynamic Obstacle (with WaypointPath), World State singletons (ScoreState, SessionState).

## Build & Run Commands

### Backend (C++20, CMake 3.28+, vcpkg)
```bash
cd backend && mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./teleop-server --config ../config/debug.json
```

### Frontend (TypeScript, Vite 5.x, Node 20+)
```bash
cd frontend
npm install
npm run dev          # Dev server at :3000
```

### Docker
```bash
docker-compose up --build
```

## Testing

### Backend (GoogleTest, RapidCheck, GoogleBenchmark)
```bash
cd backend/build
ctest --output-on-failure            # All tests
ctest -R <test_name>                 # Single test
```

### Frontend (Vitest, fast-check, Playwright)
```bash
cd frontend
npm test                             # Unit tests (Vitest)
npx vitest run <file>                # Single test file
npx playwright test                  # E2E tests
```

## Key Design Decisions

- **Deterministic physics** — fixed timestep ensures reproducible behavior regardless of frame rate
- **Lock-free concurrency** — SPSC queue with zero allocations between simulation and network threads
- **Zero-copy deserialization** — FlatBuffers lets clients read state without parsing
- **Composition over inheritance** — ECS avoids deep class hierarchies; all dependencies injected through interfaces (IMessageBus, IStateStore, ICommandSource, ICollisionDetector, IRobotKinematics)
- **Performance budgets** — physics <5ms at 1000 entities, serialization <500µs, rendering <16ms, end-to-end latency <50ms

## Key Dependencies

| Backend | Frontend |
|---------|----------|
| EnTT 3.13+ (ECS) | React 18.3+ |
| uWebSockets 20.x | Zustand 4.5+ (state) |
| FlatBuffers 24.x | flatbuffers (npm) |
| nlohmann/json 3.11+ | Vite 5.x |
| spdlog 1.12+ | Vitest 1.x |
| boost::lockfree 1.83+ | Playwright 1.42+ |

## Reference

- `docs/DDD_Robot_Teleoperation_Simulation.md` — comprehensive design document (3400 lines) with UML, component specs, system execution order, protocol schemas, and testing strategy
- `docs/Tech_Stack_Considerations_and_Assumptions.pdf` — technology choice justifications
