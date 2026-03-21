# Robot Teleoperation Simulator

Real-time robot teleoperation simulator with a C++20 physics backend and React/TypeScript browser frontend. Drive a robot through an obstacle course using keyboard (WASD/arrows) or gamepad controls.

## Architecture

```
┌─────────────────────────────────────────┐
│  Frontend (React 18 + Canvas)  :3000    │
│  WASD/Gamepad → JSON commands @ 10Hz   │
│  FlatBuffers state ← Canvas render     │
└──────────────┬──────────────────────────┘
               │ WebSocket
┌──────────────┴──────────────────────────┐
│  Backend (C++20 + EnTT ECS)    :9001    │
│  ┌─────────────┐  ┌──────────────────┐  │
│  │ Network I/O │◄►│ Simulation 60Hz  │  │
│  │ (uWS)       │  │ (8 ECS systems)  │  │
│  └─────────────┘  └──────────────────┘  │
│       SPSC Queue (lock-free)            │
└─────────────────────────────────────────┘
```

- **Backend:** Two-thread C++20 server. The simulation thread runs a fixed 60 Hz ECS tick loop (Input → Kinematics → Movement → Transform → Collision → Scoring → Broadcast → Logging). The network thread runs a uWebSockets event loop. They communicate via a lock-free SPSC queue.
- **Frontend:** React 18 SPA with a Canvas-based renderer and Zustand state management. Sends JSON commands at ~10 Hz, receives FlatBuffers world state at 30 Hz.
- **Protocol:** Hybrid — JSON text for commands (<100 bytes), FlatBuffers binary for world state broadcasts.
- **Physics:** SAT narrow-phase collision detection + spatial hash grid broad-phase. Ackermann steering model for robot kinematics. Collision response zeroes velocity and increments the impact counter.

## Prerequisites

### Docker (recommended)

- [Docker Desktop](https://www.docker.com/products/docker-desktop/) (includes Docker Compose)

### Local development

**Backend:**
- GCC 13+ or Clang 17+ with C++20 support
- CMake 3.28+
- [vcpkg](https://github.com/microsoft/vcpkg) (for dependency management)

**Frontend:**
- Node.js 20+
- npm 9+

## Quick Start (Docker)

The fastest way to get running:

```bash
# Clone the repository
git clone <repo-url>
cd Lab37_WS

# Build and start both containers
docker compose up --build
```

Open http://localhost:3000 in your browser. The backend runs on port 9001.

To stop:
```bash
docker compose down
```

To rebuild after code changes:
```bash
docker compose down
docker compose up --build
```

### Docker environment variables

The backend container supports these environment variables (set in `docker-compose.yml`):

| Variable | Default | Description |
|----------|---------|-------------|
| `TELEOP_PORT` | `9001` | WebSocket server port |
| `TELEOP_TICK_RATE` | `60` | Simulation ticks per second |
| `TELEOP_BROADCAST_RATE` | `30` | State broadcasts per second |
| `TELEOP_LOG_LEVEL` | `INFO` | Log level (TRACE, DEBUG, INFO, WARN, ERROR) |

## Local Development Setup

### Backend

```bash
# Ensure vcpkg is installed and VCPKG_ROOT is set
export VCPKG_ROOT=/path/to/vcpkg

cd backend
mkdir -p build && cd build

# Configure with CMake (uses vcpkg toolchain)
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

# Build (adjust -j for your CPU cores)
make -j$(nproc)

# Run the server
./teleop-server --config ../../config/default.json
```

The server listens on port 9001 by default.

### Frontend

```bash
cd frontend
npm install
npm run dev
```

The dev server starts at http://localhost:3000 with hot reload. It connects to the backend WebSocket at `ws://<hostname>:9001`.

For a production build:
```bash
npm run build
npm run preview
```

## Testing

### Backend tests (GoogleTest + RapidCheck + Google Benchmark)

```bash
cd backend/build

# Run all tests
ctest --output-on-failure

# Run a specific test suite
ctest -R vec2_test
ctest -R sat_detector_test
ctest -R ackermann_test
ctest -R spatial_hash_test
ctest -R command_validator_test
ctest -R token_bucket_test

# Run property-based tests
ctest -R property

# Run integration tests
ctest -R integration

# Run benchmarks
./teleop-benchmarks
```

### Frontend tests (Vitest + fast-check + Playwright)

```bash
cd frontend

# Unit and property-based tests
npm test

# Run a specific test file
npx vitest run tests/unit/gameStore.test.ts

# Watch mode
npm run test:watch

# E2E tests (requires backend running)
npx playwright test

# E2E with headed browser (visible)
npx playwright test --headed
```

### Test coverage

**Backend test suites:**
- Unit tests: Vec2, AABB, SAT detector, spatial hash grid, Ackermann model, command validator, token bucket
- Property tests: SAT symmetry/separation, Ackermann speed/heading bounds, spatial hash completeness
- Integration tests: command roundtrip (SPSC queue), collision flow (robot→wall)
- Benchmarks: SAT detection, spatial hash grid, FlatBuffers serialization, full tick

**Frontend test suites:**
- Unit tests: gameStore, InputHandler, ConnectionManager, FlatBufferDecoder
- Property tests: input clamping, rendering transforms
- E2E tests: robot control, collision detection, connection lifecycle

## Manual Testing

1. Start the application (Docker or local)
2. Open http://localhost:3000
3. You should see:
   - A green rectangle (robot) facing north at the center
   - Grey/tan static obstacles (walls and blocks)
   - Red/orange triangles and rectangles (dynamic obstacles) patrolling waypoint paths
   - A grid background
4. Use controls (see table below) to drive the robot
5. Drive into an obstacle — the robot should stop and the impact counter in the top center should increment
6. Check the HUD for connection status (green dot = streaming), FPS, and latency
7. Click the **Reset** button (top-right) to restart the scene from scratch
8. Close and reopen the browser tab to test reconnection (exponential backoff)

## Controls

| Input | Action |
|-------|--------|
| W / Arrow Up | Accelerate forward |
| S / Arrow Down | Reverse |
| A / Arrow Left | Steer left |
| D / Arrow Right | Steer right |
| Space | Brake |
| Gamepad left stick | Throttle + steering |
| Reset button (HUD) | Reset scene to initial state |

## HUD Elements

- **Connection indicator** (top-left) — green (streaming), yellow (connecting/reconnecting), red (disconnected), orange (stale data)
- **FPS / Latency** (top-left) — real-time performance metrics
- **Impact counter** (top-center) — increments on each robot-obstacle collision; green when 0, red when >0
- **Control hints** (top-right) — keyboard control reference
- **Reset button** (top-right) — resets the simulation to its initial state
- **Throttle gauge** (bottom-left) — vertical bar showing current throttle level
- **Steering indicator** (bottom-left) — horizontal bar showing steering direction
- **Input source** (bottom-left) — shows "keyboard", "gamepad", or "none"

## Configuration

Scene configuration files are in `config/`:

| File | Description |
|------|-------------|
| `default.json` | Standard scene: robot + 12 static obstacles + 8 dynamic obstacles in a 100x100m world |
| `debug.json` | Minimal scene with DEBUG-level logging for development |
| `benchmark.json` | Large 400x400m world for stress testing |

### Configuration structure

```jsonc
{
  "server": {
    "port": 9001,           // WebSocket port
    "tickRate": 60,          // Simulation Hz
    "broadcastRate": 30      // State broadcast Hz
  },
  "logging": {
    "level": "INFO",         // TRACE, DEBUG, INFO, WARN, ERROR
    "logDir": "/var/log/teleop"
  },
  "world": {
    "minX": -50.0, "maxX": 50.0,
    "minY": -50.0, "maxY": 50.0
  },
  "physics": {
    "spatialHashCellSize": 5.0
  },
  "rateLimiting": {
    "maxTokens": 20,         // Burst capacity
    "refillRate": 15          // Tokens per second
  },
  "scene": {
    "robot": { /* position, heading, vertices, color, wheelbase, maxSpeed, maxSteeringAngle */ },
    "staticObstacles": [ /* array of { position, heading, vertices, color } */ ],
    "dynamicObstacles": [ /* array of { position, heading, vertices, color, speed, loop, waypoints } */ ]
  }
}
```

To use a different config:
```bash
# Docker: mount a custom config
docker compose down
# Edit config/default.json, then:
docker compose up --build

# Local: pass --config flag
./teleop-server --config ../../config/debug.json
```

## Project Structure

```
Lab37_WS/
├── backend/
│   ├── src/
│   │   ├── main.cpp                    # Composition root
│   │   ├── math/                       # Vec2, AABB (header-only)
│   │   ├── components/                 # ECS components (header-only structs)
│   │   ├── interfaces/                 # Abstract interfaces (IMessageBus, ICommandSource, etc.)
│   │   ├── physics/                    # SAT detector, spatial hash grid, Ackermann model
│   │   ├── protocol/                   # Command validation, FlatBuffers serialization
│   │   ├── infra/                      # Concrete implementations (logger, message bus, WebSocket gateway, scene loader)
│   │   └── engine/
│   │       ├── simulation_engine.cpp   # Fixed-timestep tick loop
│   │       └── systems/                # 8 ECS systems
│   ├── tests/
│   │   ├── unit/                       # GoogleTest unit tests
│   │   ├── property/                   # RapidCheck property tests
│   │   ├── integration/                # Integration tests
│   │   └── benchmarks/                 # Google Benchmark performance tests
│   ├── CMakeLists.txt
│   ├── vcpkg.json
│   └── Dockerfile
├── frontend/
│   ├── src/
│   │   ├── App.tsx                     # Main app with connection + input wiring
│   │   ├── components/                 # SimulationCanvas, HUDOverlay, ControlPanel, ErrorBoundary
│   │   ├── services/                   # ConnectionManager, InputHandler, FlatBufferDecoder, CommandEncoder
│   │   ├── store/                      # Zustand game store
│   │   ├── types/                      # TypeScript type definitions
│   │   └── utils/                      # Logger, camera, math helpers
│   ├── tests/
│   │   ├── unit/                       # Vitest unit tests
│   │   ├── property/                   # fast-check property tests
│   │   └── e2e/                        # Playwright E2E tests
│   ├── package.json
│   └── Dockerfile
├── config/                             # Scene configuration files
├── proto/                              # FlatBuffers schema (teleop.fbs)
├── docs/
│   ├── DDD_Robot_Teleoperation_Simulation.md   # Design document
│   ├── Implementation_Plan.md                   # 8-phase implementation plan
│   └── Tech_Stack_Considerations_and_Assumptions.pdf
└── docker-compose.yml
```

## Key Dependencies

| Backend | Frontend |
|---------|----------|
| EnTT 3.13+ (ECS) | React 18.3+ |
| uWebSockets 20.x | Zustand 4.5+ (state) |
| FlatBuffers 24.x | flatbuffers (npm) |
| nlohmann/json 3.11+ | Vite 5.x |
| spdlog 1.12+ | Vitest 1.x |
| Boost.Lockfree 1.83+ | fast-check 3.x |
| GoogleTest | Playwright 1.42+ |
| RapidCheck | |
| Google Benchmark | |

## Performance Targets

| Metric | Target | Measured |
|--------|--------|----------|
| SAT collision (4-vertex) | < 0.5 us | ~ 0.03 us |
| Spatial hash (1000 entities) | < 2 ms | - |
| FlatBuffers serialization (1000 entities) | < 500 us | - |
| Full tick (20 entities) | < 500 us | ~ 40-80 us |
| Full tick (1000 entities) | < 5 ms | - |
| End-to-end latency | < 50 ms | ~ 5-15 ms |
| Frontend rendering | < 16 ms | ~ 2-4 ms |

## Troubleshooting

### WebSocket connection fails
- Ensure the backend is running on port 9001
- In Docker, both containers must be running (`docker compose ps`)
- Check browser console for connection errors

### "Sequence number not monotonic" errors
- This happens when reconnecting — the backend resets sequence tracking on new connections
- If persistent, click the **Reset** button to restart the scene

### No entities visible / blank canvas
- Check that the backend is broadcasting (look for `entity_count` in backend logs)
- Verify the connection indicator shows green ("Streaming")
- Check browser console for FlatBuffer decoding errors

### Docker build fails on ARM Mac (Apple Silicon)
- The Dockerfile auto-detects ARM64 and uses the correct vcpkg triplet
- Ensure Docker Desktop is running with default Linux/ARM64 platform

### Tick overrun warnings
- Occasional overruns in Docker are normal (container scheduling)
- Persistent overruns suggest too many entities — reduce scene complexity or increase `spatialHashCellSize`

## Documentation

- [Design Document](docs/DDD_Robot_Teleoperation_Simulation.md) — comprehensive 3400-line specification with UML diagrams, component schemas, protocol definitions, and testing strategy
- [Implementation Plan](docs/Implementation_Plan.md) — 8-phase build plan with dependency graph and deviation notes
- [Tech Stack Considerations](docs/Tech_Stack_Considerations_and_Assumptions.pdf) — technology choice justifications
