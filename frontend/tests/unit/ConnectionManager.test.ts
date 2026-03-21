import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { useGameStore } from '../../src/store/gameStore';

// Mock WebSocket
class MockWebSocket {
    static CONNECTING = 0;
    static OPEN = 1;
    static CLOSING = 2;
    static CLOSED = 3;

    readyState = MockWebSocket.CONNECTING;
    binaryType = '';
    url: string;

    onopen: ((ev: Event) => void) | null = null;
    onclose: ((ev: CloseEvent) => void) | null = null;
    onmessage: ((ev: MessageEvent) => void) | null = null;
    onerror: ((ev: Event) => void) | null = null;

    sent: (string | ArrayBuffer)[] = [];

    constructor(url: string) {
        this.url = url;
        // Simulate async open
        setTimeout(() => {
            this.readyState = MockWebSocket.OPEN;
            this.onopen?.(new Event('open'));
        }, 0);
    }

    send(data: string | ArrayBuffer): void {
        this.sent.push(data);
    }

    close(code?: number, reason?: string): void {
        this.readyState = MockWebSocket.CLOSED;
    }
}

describe('ConnectionManager', () => {
    let originalWebSocket: typeof WebSocket;

    beforeEach(() => {
        originalWebSocket = globalThis.WebSocket;
        (globalThis as Record<string, unknown>).WebSocket = MockWebSocket;

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

    afterEach(() => {
        (globalThis as Record<string, unknown>).WebSocket = originalWebSocket;
        vi.restoreAllMocks();
    });

    it('sets connection state to connecting on connect', async () => {
        const { ConnectionManager } = await import('../../src/services/ConnectionManager');
        const cm = new ConnectionManager('ws://localhost:9001');
        cm.connect();
        expect(useGameStore.getState().connectionState).toBe('connecting');
        cm.disconnect();
    });

    it('sends handshake on open', async () => {
        const { ConnectionManager } = await import('../../src/services/ConnectionManager');
        const cm = new ConnectionManager('ws://localhost:9001');
        cm.connect();

        // Wait for mock async open
        await new Promise((r) => setTimeout(r, 10));

        expect(useGameStore.getState().connectionState).toBe('connected');
        cm.disconnect();
    });

    it('sets disconnected on user-initiated disconnect', async () => {
        const { ConnectionManager } = await import('../../src/services/ConnectionManager');
        const cm = new ConnectionManager('ws://localhost:9001');
        cm.connect();
        await new Promise((r) => setTimeout(r, 10));

        cm.disconnect();
        expect(useGameStore.getState().connectionState).toBe('disconnected');
    });

    it('handles pong messages and updates latency', async () => {
        const { ConnectionManager } = await import('../../src/services/ConnectionManager');
        const cm = new ConnectionManager('ws://localhost:9001');
        cm.connect();
        await new Promise((r) => setTimeout(r, 10));

        // Simulate pong from server
        const ws = (cm as unknown as { ws: MockWebSocket }).ws;
        const pongMsg = JSON.stringify({
            type: 'pong',
            clientTimestamp: Date.now() - 25,
        });
        ws.onmessage?.(new MessageEvent('message', { data: pongMsg }));

        expect(useGameStore.getState().latencyMs).toBeGreaterThanOrEqual(20);
        cm.disconnect();
    });

    it('does not connect if already open', async () => {
        const { ConnectionManager } = await import('../../src/services/ConnectionManager');
        const cm = new ConnectionManager('ws://localhost:9001');
        cm.connect();
        await new Promise((r) => setTimeout(r, 10));

        // Second connect should be a no-op
        cm.connect();
        cm.disconnect();
    });
});
