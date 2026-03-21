# Robot Teleoperation Simulator

Real-time robot teleoperation simulator with a C++20 physics backend and React/TypeScript browser frontend. Drive a robot through an obstacle course using keyboard (WASD/arrows) or gamepad controls.

## Quick Start

```bash
docker-compose up --build
```

Open http://localhost:3000 in your browser.

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

**ECS System execution order:** Input → Kinematics → Movement → Transform → Collision → Scoring → Broadcast → Logging

**Physics:** SAT collision detection with spatial hash grid broad-phase, Ackermann steering kinematics.

**Protocol:** JSON (client→server commands) + FlatBuffers binary (server→client world state at 30Hz).

## Development

### Backend

```bash
cd backend && mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./teleop-server --config ../../config/default.json
```

### Frontend

```bash
cd frontend
npm install
npm run dev
```

### Tests

```bash
# Backend
cd backend/build && ctest --output-on-failure

# Frontend unit + property tests
cd frontend && npm test

# Frontend E2E
cd frontend && npx playwright test
```

## Configuration

Scene configuration files in `config/`:

| File | Purpose |
|------|---------|
| `default.json` | Standard scene: robot, 5 walls, 2 dynamic obstacles |
| `debug.json` | Same scene with DEBUG logging |
| `benchmark.json` | Large world (400x400m) for stress testing |

## Controls

| Input | Action |
|-------|--------|
| W / Arrow Up | Accelerate forward |
| S / Arrow Down | Reverse |
| A / Arrow Left | Steer left |
| D / Arrow Right | Steer right |
| Space | Brake |
| Gamepad left stick | Throttle + steering |

## HUD

- **Connection indicator** — green (streaming), yellow (connecting/reconnecting), red (disconnected), orange (stale)
- **Impact counter** — increments on robot-obstacle collision
- **FPS / Latency** — real-time performance metrics
- **Throttle/Steering gauges** — visual input feedback
