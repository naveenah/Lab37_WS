import { create } from 'zustand';
import { EntityState, EntityType } from '../types/entities';
import type { ConnectionState } from '../types/protocol';
import type { CommandMessage } from '../types/commands';

export interface GameState {
    // World state
    entities: Map<number, EntityState>;
    robotId: number | null;
    impactCount: number;

    // Connection state
    connectionState: ConnectionState;
    latencyMs: number;

    // Performance
    renderFps: number;
    streamFps: number;
    lastBroadcastTime: number;

    // Input state
    lastCommand: CommandMessage | null;
    inputSource: 'keyboard' | 'gamepad' | 'none';

    // Actions
    setEntities: (entities: EntityState[]) => void;
    setImpactCount: (count: number) => void;
    setConnectionState: (state: ConnectionState) => void;
    setLatency: (ms: number) => void;
    setRenderFps: (fps: number) => void;
    setStreamFps: (fps: number) => void;
    setLastCommand: (cmd: CommandMessage) => void;
    setInputSource: (source: 'keyboard' | 'gamepad' | 'none') => void;
    recordBroadcast: () => void;
}

export const useGameStore = create<GameState>((set, get) => ({
    entities: new Map(),
    robotId: null,
    impactCount: 0,
    connectionState: 'disconnected',
    latencyMs: 0,
    renderFps: 0,
    streamFps: 0,
    lastBroadcastTime: 0,
    lastCommand: null,
    inputSource: 'none',

    setEntities: (entities) => {
        const map = new Map<number, EntityState>();
        let robotId: number | null = null;
        for (const e of entities) {
            map.set(e.id, e);
            if (e.type === EntityType.Robot) robotId = e.id;
        }
        set({ entities: map, robotId });
    },

    setImpactCount: (count) => set({ impactCount: count }),
    setConnectionState: (state) => set({ connectionState: state }),
    setLatency: (ms) => set({ latencyMs: ms }),
    setRenderFps: (fps) => set({ renderFps: fps }),
    setStreamFps: (fps) => set({ streamFps: fps }),
    setLastCommand: (cmd) => set({ lastCommand: cmd }),
    setInputSource: (source) => set({ inputSource: source }),

    recordBroadcast: () => {
        const now = performance.now();
        const prev = get().lastBroadcastTime;
        if (prev > 0) {
            const dt = now - prev;
            const fps = 1000 / dt;
            set({
                streamFps: Math.round(fps * 0.1 + get().streamFps * 0.9),
                lastBroadcastTime: now,
            });
        } else {
            set({ lastBroadcastTime: now });
        }
    },
}));
