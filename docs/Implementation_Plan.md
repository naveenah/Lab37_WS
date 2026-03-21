# Implementation Plan: Robot Teleoperation Simulator

## Context

This document describes the implementation plan used to build the full production-grade robot teleoperation simulator from the design specification in `DDD_Robot_Teleoperation_Simulation.md`. The implementation follows the Data-Driven Design (ECS) and SOLID principles outlined in the design document. It is divided into 8 phases ordered by dependency topology — each phase produces compilable, testable artifacts.

**Branch:** `feature/implement-simulated-robot-teleoperation`

---

## Dependency Graph

```
Phase 0 (Scaffolding)
    │
Phase 1 (Math + Components + Interfaces + Types)
    │           \
Phase 2          Phase 3
(Physics)        (Protocol + FlatBuffers)
    \             /
     Phase 4
     (Infrastructure)
         │
     Phase 5
     (ECS Systems + Engine)
         │
     Phase 6
     (WebSocket Gateway)
         │
     Phase 7
     (Frontend)
         │
     Phase 8
     (E2E + Polish)
```

---

## Phase 0: Project Scaffolding & Build Infrastructure

**Goal:** Directory structure, build systems, dependency management. Two "hello world" binaries: C++ server that logs startup, React page that renders placeholder text.

### Files Created

| File | Purpose |
|------|---------|
| `docker-compose.yml` | Backend (port 9001) + frontend (port 3000) services per §17.4 |
| `config/default.json` | Full scene: robot at origin, 5 static walls, 2 dynamic obstacles with waypoints |
| `config/debug.json` | Same scene with DEBUG logging |
| `proto/teleop.fbs` | FlatBuffers IDL from §8.2 (Vec2, EntityType, Entity, WorldState) |
| `backend/CMakeLists.txt` | CMake 3.28+, C++20, vcpkg toolchain, targets: teleop-server, teleop-tests, teleop-property-tests, teleop-integration-tests, teleop-benchmarks, flatc code generation |
| `backend/vcpkg.json` | Manifest: entt, flatbuffers, nlohmann-json, spdlog, boost-lockfree, gtest, benchmark |
| `backend/Dockerfile` | Multi-stage: Ubuntu 24.04 + GCC 13 build → slim runtime |
| `backend/src/main.cpp` | Minimal: init spdlog, log "Simulation starting...", exit |
| `frontend/package.json` | react, react-dom, zustand, flatbuffers + dev deps (typescript, vite, vitest, fast-check, playwright) |
| `frontend/tsconfig.json` | Strict TS, ES2022, react-jsx, path aliases |
| `frontend/vite.config.ts` | React plugin, port 3000, WS proxy to localhost:9001, vitest config |
| `frontend/Dockerfile` | Node 20 Alpine, npm install, npm run dev |
| `frontend/playwright.config.ts` | Base URL, webServer config, Chromium project |
| `frontend/index.html` | Vite entry point with dark theme base styles |
| `frontend/src/main.tsx` | ReactDOM.createRoot → App in StrictMode |
| `frontend/src/App.tsx` | Placeholder: "Robot Teleoperation Simulator — Connecting..." |
| `.gitignore` | Build outputs, node_modules, generated files, logs |

### Verification
- `backend/src/main.cpp` compiles and runs
- `npm run dev` starts Vite at :3000
- `docker-compose up --build` builds both containers

### Deviations from Design Document
- **uWebSockets:** Not available as a standard vcpkg port. Used **CMake FetchContent** to pull uWebSockets + uSockets from GitHub with pinned versions instead of vcpkg.
- **RapidCheck:** Also not in vcpkg standard registry. Used FetchContent with pinned commit.
- **docker-compose.yml `version` field:** Design doc uses `version: '3.8'` which is deprecated in Docker Compose V2. Included for backward compatibility with a comment noting deprecation.
- **Header-only implementations:** Small performance-critical types (Vec2, AABB, TokenBucket, AckermannModel) are header-only `.hpp` files rather than .hpp + .cpp pairs — eliminates linking overhead and enables inlining.

---

## Phase 1: Math Primitives, Components & Interfaces

**Goal:** Foundational data types that everything depends on. All component structs, all interface ABCs, fully tested Vec2/AABB math.

### Files Created

**Math (header-only):**
- `backend/src/math/vec2.hpp` — Vec2 struct with full operator overloads: +, -, *, dot, cross, length, normalized, perpendicular, fromAngle, distance
- `backend/src/math/aabb.hpp` — AABB struct: overlaps(), expand(), center(), halfExtents(), fromVertices()

**Components (header-only, per §5.2):**
- `backend/src/components/transform.hpp` — x, y, heading
- `backend/src/components/polygon_shape.hpp` — localVertices, worldVertices, aabb
- `backend/src/components/velocity.hpp` — speed, angularVelocity
- `backend/src/components/robot_tag.hpp` — steeringAngle, wheelbase, maxSpeed, maxSteeringAngle, throttle, steeringInput
- `backend/src/components/static_tag.hpp` — empty marker
- `backend/src/components/waypoint_path.hpp` — waypoints, currentIndex, speed, loop
- `backend/src/components/collision_state.hpp` — isColliding, collidedWith, wasCollidingLastTick, mtv
- `backend/src/components/score_state.hpp` — impactCount, sessionStartMs (singleton)
- `backend/src/components/session_state.hpp` — operatorConnected, lastCommandTimestamp, commandSequence, stateSequence (singleton)
- `backend/src/components/render_meta.hpp` — color, layer

**Interfaces (header-only pure abstract, per §6.5 and §7.3):**
- `backend/src/interfaces/i_logger.hpp` — trace/debug/info/warn/error/critical/event
- `backend/src/interfaces/i_message_bus.hpp` — Topic enum, MessageCallback, publish/subscribe
- `backend/src/interfaces/i_state_store.hpp` — getEntitySnapshots/getScoreState
- `backend/src/interfaces/i_command_source.hpp` — tryDequeue(CommandMessage&)
- `backend/src/interfaces/i_collision_detector.hpp` — CollisionPair, detect()
- `backend/src/interfaces/i_robot_kinematics.hpp` — RobotKinematicsState, computeStep()

**Protocol data types:**
- `backend/src/protocol/command_message.hpp` — CommandType enum, CommandMessage struct
- `backend/src/protocol/entity_snapshot.hpp` — EntityType enum, EntitySnapshot struct

**Frontend types:**
- `frontend/src/types/entities.ts` — EntityState, EntityType enum, Vec2
- `frontend/src/types/commands.ts` — CommandMessage, RawInput
- `frontend/src/types/protocol.ts` — ConnectionState type, ServerJsonMessage union type (welcome, pong, event, error)

### Tests
- `backend/tests/unit/vec2_test.cpp` — 20 tests: dot commutativity, perpendicular orthogonality, normalization, length, arithmetic, fromAngle, distance
- `backend/tests/unit/aabb_test.cpp` — 13 tests: overlap detection, non-overlap, expand, center, halfExtents, fromVertices

### Deviations from Design Document
- **CollisionState fields:** Design §5.2 does NOT include `wasCollidingLastTick` and `mtv`, but §9.3 (ScoringSystem collision resolution) uses both fields. Added them to CollisionState for consistency.
- **SessionState file:** Not listed in §17.5's file tree but defined in §5.2. Added `session_state.hpp`.

---

## Phase 2: Physics Engine

**Goal:** Fully tested, benchmarked SAT collision detection, spatial hash grid, and Ackermann kinematics — all in isolation before ECS integration.

### Files Created
- `backend/src/physics/sat_detector.hpp` + `.cpp` — SATResult (colliding, normal, penetration), SATDetector implementing ICollisionDetector with edge-normal projection, MTV computation (§9.2)
- `backend/src/physics/spatial_hash_grid.hpp` + `.cpp` — CellKey with int32 coords, CellKeyHash, clear/insert/getCandidatePairs with deduplication (§9.1)
- `backend/src/physics/ackermann_model.hpp` — Header-only AckermannModel implementing IRobotKinematics, smooth steering/acceleration, Euler integration, angle normalization (§10.2)

### Tests (per §14.2–14.4)
- `backend/tests/unit/sat_detector_test.cpp` — 11 tests: identical squares, separated squares, overlapping, touching edges, triangle-quad, MTV resolves, symmetry
- `backend/tests/unit/spatial_hash_test.cpp` — 8 tests: empty grid, overlapping AABBs, distant AABBs, dedup pairs, clear, multiple pairs
- `backend/tests/unit/ackermann_test.cpp` — 10 tests: straight line, turning, zero speed, heading normalization, speed limits, reverse, smooth accel/steer
- `backend/tests/property/sat_property_test.cpp` — 4 RapidCheck properties: symmetry, separation, self-collision, MTV resolution
- `backend/tests/property/ackermann_property_test.cpp` — 3 properties: speed ≤ max, heading normalized, zero throttle decelerates
- `backend/tests/property/spatial_hash_property_test.cpp` — 2 properties: all overlapping pairs found, no duplicates
- `backend/tests/benchmarks/sat_benchmark.cpp` — SAT squares, separated, complex polygons 3-16 vertices
- `backend/tests/benchmarks/spatial_hash_benchmark.cpp` — Insert+query at 20-2000 entities

### Performance Targets
- SAT (4-vertex): < 0.5µs
- Spatial hash at 1000 entities: < 2ms

---

## Phase 3: Protocol Layer

**Goal:** Command validation, state serialization (FlatBuffers), rate limiting. JSON→CommandMessage pipeline and EntitySnapshot→FlatBuffers pipeline both work.

### Files Created
- FlatBuffers code generation configured in CMake (`teleop_generated.h`)
- `backend/src/protocol/command_validator.hpp` — static validate() with ValidationResult, throttle/steering range checks, sequence regression detection, timestamp validation (§8.5)
- `backend/src/protocol/state_serializer.hpp` + `.cpp` — serialize EntitySnapshot[] + ScoreState → FlatBuffers binary using generated `teleop_generated.h`
- `backend/src/infra/token_bucket.hpp` — Header-only TokenBucket with chrono-based refill (§8.6)

### Tests
- `backend/tests/unit/command_validator_test.cpp` — 10 tests: valid accepted, throttle/steering OOB, sequence regression/gap, timestamp regression (§14.2)
- `backend/tests/unit/token_bucket_test.cpp` — 5 tests: burst allowed, depletion, refill, custom cost, cap (§14.2)
- `backend/tests/benchmarks/flatbuffer_benchmark.cpp` — Serialize at 20-2000 entities (§14.4)

### Performance Target
- FlatBuffer serialization at 1000 entities: < 500µs

---

## Phase 4: Infrastructure Implementations

**Goal:** Concrete classes behind all Phase 1 interfaces. Scene loading from JSON config.

### Files Created
- `backend/src/infra/spdlog_logger.hpp` + `.cpp` — ILogger impl with dual sinks (async console + rotating JSON file), configurable log level
- `backend/src/infra/in_process_bus.hpp` + `.cpp` — IMessageBus impl with mutex-protected topic→callback map
- `backend/src/infra/spsc_command_source.hpp` — ICommandSource impl wrapping boost::lockfree::spsc_queue
- `backend/src/infra/ecs_state_store.hpp` + `.cpp` — IStateStore impl reading entt::registry views to produce EntitySnapshot vectors
- `backend/src/infra/config_loader.hpp` + `.cpp` — Config struct hierarchy, fromFile()/fromEnv()/validate()
- `backend/src/infra/scene_loader.hpp` + `.cpp` — loadScene() from JSON, createRobot/StaticObstacle/DynamicObstacle, polygon validation, world vertex computation

### Tests
- `backend/tests/integration/command_roundtrip_test.cpp` — 4 tests: SPSC enqueue/dequeue, empty, multiple, FIFO order (§14.7)

---

## Phase 5: ECS Systems & Simulation Engine

**Goal:** All 8 systems wired into SimulationEngine with 60Hz fixed-timestep loop. Backend runs headlessly — physics works, collisions resolve, scoring increments.

### Files Created

**Systems (§5.4 execution order):**
1. `backend/src/engine/systems/input_system.hpp` + `.cpp` — dequeue from ICommandSource → update RobotTag throttle/steeringInput
2. `backend/src/engine/systems/kinematics_system.hpp` + `.cpp` — AckermannModel → update Velocity/Transform
3. `backend/src/engine/systems/movement_system.hpp` + `.cpp` — WaypointPath following with looping → update Transform
4. `backend/src/engine/systems/transform_system.hpp` + `.cpp` — recompute worldVertices + AABBs, skip StaticTag entities
5. `backend/src/engine/systems/collision_system.hpp` + `.cpp` — spatial hash → SAT → CollisionState, isRobotInitiatedCollision filter (§9.4)
6. `backend/src/engine/systems/scoring_system.hpp` + `.cpp` — new collision → vel=0, impactCount++, MTV pushout, JSON event logging (§9.3)
7. `backend/src/engine/systems/broadcast_system.hpp` + `.cpp` — StateSerializer → IMessageBus publish
8. `backend/src/engine/systems/logging_system.hpp` + `.cpp` — periodic tick timing + metrics logging

**Engine:**
- `backend/src/engine/simulation_engine.hpp` + `.cpp` — Fixed-timestep accumulator loop, 8-system execution order, world bounds clamping, tick overrun detection, SIGINT/SIGTERM handling, configurable broadcast rate (§12.2)

**Composition Root:**
- `backend/src/main.cpp` — Updated: ConfigLoader → SpdlogLogger → InProcessBus → SPSCCommandSource → AckermannModel → ECSStateStore → SceneLoader → SimulationEngine

### Tests
- `backend/tests/integration/collision_flow_test.cpp` — 2 tests: robot collision zeros velocity + increments counter, impact counter only increments once per collision event (§14.7)
- `backend/tests/benchmarks/full_tick_benchmark.cpp` — Full physics tick at 20-2000 entities (§14.4)

### Performance Targets
- Full tick at 20 entities: < 500µs
- Full tick at 1000 entities: < 5ms

### Design Decisions
- **Static entity optimization:** TransformSystem skips world vertex recomputation for StaticTag entities (compute once at scene load). Per §15.3 "dirty flag" optimization.
- **Corner collision resolution:** If robot touches two walls simultaneously, conflicting MTVs may occur. Iterates collision resolution up to 4 times per tick until no overlaps remain. This is a minor extension not specified in the design doc.

---

## Phase 6: WebSocket Gateway & Network Thread

**Goal:** uWebSockets server on separate thread, connected to simulation via SPSC queues. Backend becomes a fully functional WebSocket server: accepts connections, receives JSON commands, broadcasts FlatBuffers state.

### Files Created
- `backend/src/infra/websocket_gateway.hpp` + `.cpp` — uWS::App with:
  - `onOpen`: send welcome JSON with tickRate and protocolVersion
  - `onMessage`: parse JSON → validate → rate-limit via TokenBucket → enqueue to SPSC queue
  - `onClose`: log disconnection
  - `broadcast`: FlatBuffers binary to all connected clients
  - ping/pong handling (§8.3-8.4)

### Updates
- `backend/src/main.cpp` — launch gateway on std::thread, wire SPSC queues (commands → sim, state → network)
- BroadcastSystem publishes to IMessageBus; gateway subscribes and broadcasts (DIP maintained)

### Risks Mitigated
- **uWebSockets thread model:** Single-threaded event loop. Used a dedicated state SPSC queue (sim→gateway direction) matching the command queue pattern, rather than relying on InProcessBus thread safety.
- **uWebSockets build:** Required uSockets as a dependency. FetchContent pulls both with pinned versions.

### Deviation from Design Document
- **Two SPSC queues** (commands + state) instead of IMessageBus for cross-thread communication. IMessageBus is not inherently thread-safe; SPSC is the right primitive for the two-thread model per §4.2.

---

## Phase 7: Frontend Application

**Goal:** Complete React frontend — Canvas rendering, input handling, WebSocket connection, HUD. Operator can drive the robot in the browser.

### Files Created

**Services:**
- `frontend/src/services/ConnectionManager.ts` — WebSocket lifecycle, exponential backoff reconnection (max 10 attempts, capped at 30s), handshake/ping/pong, binary→FlatBufferDecoder, text→JSON parse, stale detection (2s timeout) (§7.5.4, §7.6.1)
- `frontend/src/services/InputHandler.ts` — keyboard WASD/arrows + Gamepad API, deadzone=0.15, input merging (gamepad priority when active), sequence counter (§11.3)
- `frontend/src/services/FlatBufferDecoder.ts` — Manual FlatBuffers binary decoder for WorldState with vtable navigation, sequence validation, entity decoding (§12.3)
- `frontend/src/services/CommandEncoder.ts` — CommandMessage/handshake/ping → JSON string (§8.3)
- `frontend/src/services/PerformanceMonitor.ts` — Rolling FPS (60-sample window), P99 frame time

**State:**
- `frontend/src/store/gameStore.ts` — Zustand store: entities Map, robotId, impactCount, connectionState, latencyMs, fps, lastCommand, inputSource, with EMA-smoothed broadcast FPS (§11.4)

**Utilities:**
- `frontend/src/utils/logger.ts` — Logger class with LogLevel enum, circular buffer, sessionStorage persistence
- `frontend/src/utils/camera.ts` — Camera with worldToScreen/screenToWorld (Y-flipped for screen coords), followTarget with smoothing
- `frontend/src/utils/math.ts` — Vec2, clamp, lerp, vec2Distance, vec2Length

**Components:**
- `frontend/src/App.tsx` — ErrorBoundary wrapper, ConnectionManager + InputHandler initialization, 10Hz command dispatch loop
- `frontend/src/components/SimulationCanvas.tsx` — Canvas with rAF loop, DPR-aware resize, camera follow, grid background, entity rendering sorted by layer (static → dynamic → robot), heading arrow for robot (§11.2)
- `frontend/src/components/HUDOverlay.tsx` — Connection status badge (color-coded: green/yellow/red/orange), FPS, latency, impact counter, WASD control hints (§7.6.1)
- `frontend/src/components/ControlPanel.tsx` — Throttle gauge (vertical, green up/red down), steering indicator (horizontal, blue), input source display
- `frontend/src/components/ErrorBoundary.tsx` — React class error boundary with error logging and reload button (§12.3)

### Tests (per §14.5-14.6)
- `frontend/tests/unit/InputHandler.test.ts` — 13 tests: zero input, WASD keys, arrow keys, space brake, key release, sequence increment, hasInput, clamp bounds, repeat ignore
- `frontend/tests/unit/FlatBufferDecoder.test.ts` — 4 tests: instance creation, zeroed buffer handling, out-of-order sequence rejection, reset
- `frontend/tests/unit/ConnectionManager.test.ts` — 5 tests: connecting state, handshake on open, disconnect, pong latency update, idempotent connect
- `frontend/tests/unit/gameStore.test.ts` — 9 tests: setEntities with robot identification, null robotId, impact count, connection state, latency, FPS, last command, input source, entity replacement
- `frontend/tests/property/input.property.test.ts` — 3 fast-check properties: clamp always in bounds, clamp idempotent, throttle/steering in [-1, 1]
- `frontend/tests/property/rendering.property.test.ts` — 4 fast-check properties: worldToScreen↔screenToWorld roundtrip, camera origin→screen center, followTarget convergence

### Build Verification
- `npx tsc --noEmit` — zero errors
- `npx vite build` — 167 kB bundle (54 kB gzipped)
- `npx vitest run` — 38/38 tests passing

---

## Phase 8: Integration, E2E Tests & Polish

**Goal:** Full system integration, Playwright E2E tests, performance profiling, documentation.

### Files Created
- `frontend/tests/e2e/control.spec.ts` — 4 tests: canvas visible, HUD elements visible, throttle/steering gauges, keyboard input no errors (§14.8)
- `frontend/tests/e2e/collision.spec.ts` — 2 tests: impact counter starts at zero, impacts label visible (§14.8)
- `frontend/tests/e2e/connection.spec.ts` — 4 tests: connection status indicator, FPS/latency metrics, input source, no crash on load (§14.8)
- `config/benchmark.json` — Large world (400x400m) for stress testing with boundary walls
- `README.md` — Architecture diagram, build/run instructions, controls reference, HUD description

### Configuration
- Updated `vite.config.ts` to exclude `tests/e2e/**` from Vitest (Playwright tests run via `npx playwright test` separately)

### Verification Checklist
1. `docker-compose up --build` starts both containers
2. Frontend connects at `http://localhost:3000`
3. WASD/Arrow keys control robot
4. Collision: velocity zeroes, counter increments, no penetration
5. Dynamic obstacles patrol waypoints
6. HUD: green connection dot, FPS, latency, impact count
7. Disconnect/reconnect with exponential backoff
8. All backend tests pass: `cd backend/build && ctest --output-on-failure`
9. All frontend tests pass: `cd frontend && npm test` (38 tests)
10. E2E tests pass: `cd frontend && npx playwright test` (10 tests)
11. Benchmarks meet performance targets (§15.1)

---

## Complete File Inventory

### Backend (C++20)

```
backend/
├── CMakeLists.txt
├── Dockerfile
├── vcpkg.json
└── src/
    ├── main.cpp                              # Composition root
    ├── math/
    │   ├── vec2.hpp                          # 2D vector math (header-only)
    │   └── aabb.hpp                          # Axis-aligned bounding box (header-only)
    ├── components/
    │   ├── transform.hpp                     # Position + heading
    │   ├── polygon_shape.hpp                 # Local/world vertices + AABB
    │   ├── velocity.hpp                      # Speed + angular velocity
    │   ├── robot_tag.hpp                     # Steering, wheelbase, limits
    │   ├── static_tag.hpp                    # Marker for immovable entities
    │   ├── waypoint_path.hpp                 # Patrol route for dynamic obstacles
    │   ├── collision_state.hpp               # Collision detection results
    │   ├── score_state.hpp                   # Impact counter (singleton)
    │   ├── session_state.hpp                 # Operator connection state (singleton)
    │   └── render_meta.hpp                   # Color + layer for rendering
    ├── interfaces/
    │   ├── i_logger.hpp                      # Logging abstraction
    │   ├── i_message_bus.hpp                 # Pub/sub messaging
    │   ├── i_state_store.hpp                 # Entity snapshot provider
    │   ├── i_command_source.hpp              # Command queue consumer
    │   ├── i_collision_detector.hpp          # Narrow-phase collision
    │   └── i_robot_kinematics.hpp            # Kinematics model
    ├── protocol/
    │   ├── command_message.hpp               # Command struct + type enum
    │   ├── entity_snapshot.hpp               # Serializable entity state
    │   ├── command_validator.hpp             # Input validation
    │   └── state_serializer.hpp/.cpp         # FlatBuffers serialization
    ├── physics/
    │   ├── sat_detector.hpp/.cpp             # SAT collision detection
    │   ├── spatial_hash_grid.hpp/.cpp        # Broad-phase spatial partitioning
    │   └── ackermann_model.hpp               # Bicycle steering model (header-only)
    ├── infra/
    │   ├── spdlog_logger.hpp/.cpp            # spdlog-backed logger
    │   ├── in_process_bus.hpp/.cpp           # In-memory pub/sub
    │   ├── spsc_command_source.hpp           # Lock-free command queue
    │   ├── ecs_state_store.hpp/.cpp          # EnTT registry reader
    │   ├── config_loader.hpp/.cpp            # JSON config parser
    │   ├── scene_loader.hpp/.cpp             # Scene entity factory
    │   ├── token_bucket.hpp                  # Rate limiter (header-only)
    │   └── websocket_gateway.hpp/.cpp        # uWebSockets server
    └── engine/
        ├── simulation_engine.hpp/.cpp        # Fixed-timestep game loop
        └── systems/
            ├── input_system.hpp/.cpp         # Command → ECS
            ├── kinematics_system.hpp/.cpp    # Ackermann → transform
            ├── movement_system.hpp/.cpp      # Waypoint following
            ├── transform_system.hpp/.cpp     # Vertex + AABB recompute
            ├── collision_system.hpp/.cpp     # Broad + narrow phase
            ├── scoring_system.hpp/.cpp       # Impact response
            ├── broadcast_system.hpp/.cpp     # State serialization
            └── logging_system.hpp/.cpp       # Tick metrics
```

### Backend Tests

```
backend/tests/
├── unit/
│   ├── vec2_test.cpp                         # 20 tests
│   ├── aabb_test.cpp                         # 13 tests
│   ├── sat_detector_test.cpp                 # 11 tests
│   ├── spatial_hash_test.cpp                 # 8 tests
│   ├── ackermann_test.cpp                    # 10 tests
│   ├── command_validator_test.cpp            # 10 tests
│   └── token_bucket_test.cpp                 # 5 tests
├── property/
│   ├── sat_property_test.cpp                 # 4 properties
│   ├── ackermann_property_test.cpp           # 3 properties
│   └── spatial_hash_property_test.cpp        # 2 properties
├── integration/
│   ├── command_roundtrip_test.cpp            # 4 tests
│   └── collision_flow_test.cpp               # 2 tests
└── benchmarks/
    ├── sat_benchmark.cpp
    ├── spatial_hash_benchmark.cpp
    ├── flatbuffer_benchmark.cpp
    └── full_tick_benchmark.cpp
```

### Frontend (TypeScript/React)

```
frontend/
├── package.json
├── tsconfig.json
├── vite.config.ts
├── playwright.config.ts
├── Dockerfile
├── index.html
└── src/
    ├── main.tsx
    ├── App.tsx                               # Root: ErrorBoundary + services init
    ├── types/
    │   ├── entities.ts                       # EntityState, EntityType, Vec2
    │   ├── commands.ts                       # CommandMessage, RawInput
    │   └── protocol.ts                       # ConnectionState, ServerJsonMessage
    ├── utils/
    │   ├── math.ts                           # clamp, lerp, vec2 helpers
    │   ├── logger.ts                         # Client-side logger with buffer
    │   └── camera.ts                         # World↔screen coordinate transform
    ├── store/
    │   └── gameStore.ts                      # Zustand state management
    ├── services/
    │   ├── ConnectionManager.ts              # WebSocket lifecycle + reconnection
    │   ├── InputHandler.ts                   # Keyboard + gamepad input
    │   ├── FlatBufferDecoder.ts              # Binary state decoder
    │   ├── CommandEncoder.ts                 # JSON command encoder
    │   └── PerformanceMonitor.ts             # FPS + P99 frame time
    └── components/
        ├── SimulationCanvas.tsx              # Canvas renderer
        ├── HUDOverlay.tsx                    # Status + metrics overlay
        ├── ControlPanel.tsx                  # Input gauges
        └── ErrorBoundary.tsx                 # React error boundary
```

### Frontend Tests

```
frontend/tests/
├── unit/
│   ├── gameStore.test.ts                     # 9 tests
│   ├── InputHandler.test.ts                  # 13 tests
│   ├── ConnectionManager.test.ts             # 5 tests
│   └── FlatBufferDecoder.test.ts             # 4 tests
├── property/
│   ├── input.property.test.ts                # 3 properties
│   └── rendering.property.test.ts            # 4 properties
└── e2e/
    ├── control.spec.ts                       # 4 tests
    ├── collision.spec.ts                     # 2 tests
    └── connection.spec.ts                    # 4 tests
```

### Configuration

```
config/
├── default.json                              # Standard scene (5 walls, 2 dynamic obstacles)
├── debug.json                                # Same scene, DEBUG logging
└── benchmark.json                            # Large world (400x400m) stress test
```

---

## Summary of All Deviations from Design Document

| # | Deviation | Reason |
|---|-----------|--------|
| 1 | uWebSockets via FetchContent instead of vcpkg | Not available in vcpkg standard registry |
| 2 | RapidCheck via FetchContent instead of vcpkg | Not available in vcpkg standard registry |
| 3 | CollisionState gains `wasCollidingLastTick` and `mtv` fields | Required by §9.3 ScoringSystem but not declared in §5.2 |
| 4 | `session_state.hpp` added to file tree | Defined in §5.2 but missing from §17.5 file listing |
| 5 | Vec2, AABB, TokenBucket, AckermannModel are header-only | Performance (inlining) and simplicity; they are small types |
| 6 | Corner collision: iterative MTV resolution (up to 4 passes) | Design doc handles single-collision MTV; corners need iteration |
| 7 | docker-compose.yml version field deprecated | Docker Compose V2 ignores it; included for backward compatibility |
| 8 | Two SPSC queues (commands + state) instead of IMessageBus for cross-thread communication | IMessageBus is not inherently thread-safe; SPSC is the right primitive for the two-thread model per §4.2 |

---

## Performance Budget Summary (§15.1)

| Metric | Target | Component |
|--------|--------|-----------|
| SAT collision (4-vertex pair) | < 0.5µs | `sat_detector.cpp` |
| Spatial hash (1000 entities) | < 2ms | `spatial_hash_grid.cpp` |
| FlatBuffer serialization (1000 entities) | < 500µs | `state_serializer.cpp` |
| Full physics tick (20 entities) | < 500µs | `simulation_engine.cpp` |
| Full physics tick (1000 entities) | < 5ms | `simulation_engine.cpp` |
| Frontend render frame | < 16ms | `SimulationCanvas.tsx` |
| End-to-end input latency | < 50ms | Full pipeline |

---

## Test Summary

| Category | Framework | Count | Location |
|----------|-----------|-------|----------|
| Backend unit | GoogleTest | 77 | `backend/tests/unit/` |
| Backend property | RapidCheck | 9 | `backend/tests/property/` |
| Backend integration | GoogleTest | 6 | `backend/tests/integration/` |
| Backend benchmark | GoogleBenchmark | 4 suites | `backend/tests/benchmarks/` |
| Frontend unit | Vitest | 31 | `frontend/tests/unit/` |
| Frontend property | fast-check + Vitest | 7 | `frontend/tests/property/` |
| Frontend E2E | Playwright | 10 | `frontend/tests/e2e/` |
