import { describe, it, expect, beforeEach } from 'vitest';
import { useGameStore } from '../../src/store/gameStore';
import { EntityType } from '../../src/types/entities';
import type { EntityState } from '../../src/types/entities';

describe('gameStore', () => {
    beforeEach(() => {
        useGameStore.setState({
            entities: new Map(),
            robotId: null,
            impactCount: 0,
            connectionState: 'disconnected',
            latencyMs: 0,
            fps: 0,
            lastBroadcastTime: 0,
            lastCommand: null,
            inputSource: 'none',
        });
    });

    it('sets entities and identifies robot', () => {
        const entities: EntityState[] = [
            { id: 1, type: EntityType.Robot, x: 0, y: 0, heading: 0, vertices: [], color: '#fff' },
            { id: 2, type: EntityType.StaticObstacle, x: 5, y: 5, heading: 0, vertices: [], color: '#f00' },
        ];

        useGameStore.getState().setEntities(entities);

        const state = useGameStore.getState();
        expect(state.entities.size).toBe(2);
        expect(state.robotId).toBe(1);
        expect(state.entities.get(1)?.type).toBe(EntityType.Robot);
    });

    it('sets robotId to null when no robot entity', () => {
        const entities: EntityState[] = [
            { id: 1, type: EntityType.StaticObstacle, x: 0, y: 0, heading: 0, vertices: [], color: '#fff' },
        ];

        useGameStore.getState().setEntities(entities);
        expect(useGameStore.getState().robotId).toBeNull();
    });

    it('updates impact count', () => {
        useGameStore.getState().setImpactCount(5);
        expect(useGameStore.getState().impactCount).toBe(5);
    });

    it('updates connection state', () => {
        useGameStore.getState().setConnectionState('streaming');
        expect(useGameStore.getState().connectionState).toBe('streaming');
    });

    it('updates latency', () => {
        useGameStore.getState().setLatency(42);
        expect(useGameStore.getState().latencyMs).toBe(42);
    });

    it('updates render fps', () => {
        useGameStore.getState().setRenderFps(60);
        expect(useGameStore.getState().renderFps).toBe(60);
    });

    it('updates last command', () => {
        const cmd = { type: 'command' as const, throttle: 0.5, steering: -0.3, sequence: 1, timestamp: Date.now() };
        useGameStore.getState().setLastCommand(cmd);
        expect(useGameStore.getState().lastCommand).toEqual(cmd);
    });

    it('updates input source', () => {
        useGameStore.getState().setInputSource('gamepad');
        expect(useGameStore.getState().inputSource).toBe('gamepad');
    });

    it('replaces entities on subsequent calls', () => {
        const first: EntityState[] = [
            { id: 1, type: EntityType.Robot, x: 0, y: 0, heading: 0, vertices: [], color: '#fff' },
        ];
        const second: EntityState[] = [
            { id: 1, type: EntityType.Robot, x: 5, y: 5, heading: 1, vertices: [], color: '#fff' },
            { id: 3, type: EntityType.DynamicObstacle, x: 10, y: 10, heading: 0, vertices: [], color: '#0f0' },
        ];

        useGameStore.getState().setEntities(first);
        expect(useGameStore.getState().entities.size).toBe(1);

        useGameStore.getState().setEntities(second);
        expect(useGameStore.getState().entities.size).toBe(2);
        expect(useGameStore.getState().entities.get(1)?.x).toBe(5);
    });
});
