import { useGameStore } from '../store/gameStore';
import { FlatBufferDecoder } from './FlatBufferDecoder';
import { CommandEncoder } from './CommandEncoder';
import { logger } from '../utils/logger';
import type { CommandMessage } from '../types/commands';
import type { ServerJsonMessage } from '../types/protocol';

export class ConnectionManager {
    private ws: WebSocket | null = null;
    private reconnectAttempts = 0;
    private maxReconnectAttempts = 10;
    private reconnectTimer: ReturnType<typeof setTimeout> | null = null;
    private pingTimer: ReturnType<typeof setInterval> | null = null;
    private staleTimer: ReturnType<typeof setTimeout> | null = null;
    private decoder = new FlatBufferDecoder();
    private encoder = new CommandEncoder();
    private url: string;

    constructor(url: string) {
        this.url = url;
    }

    connect(): void {
        if (this.ws?.readyState === WebSocket.OPEN) return;

        useGameStore.getState().setConnectionState('connecting');
        logger.info('Connecting to ' + this.url);

        try {
            this.ws = new WebSocket(this.url);
            this.ws.binaryType = 'arraybuffer';

            this.ws.onopen = () => {
                logger.info('WebSocket connected');
                this.reconnectAttempts = 0;
                useGameStore.getState().setConnectionState('connected');

                // Send handshake
                this.ws?.send(this.encoder.encodeHandshake());

                // Start ping interval
                this.pingTimer = setInterval(() => {
                    this.ws?.send(this.encoder.encodePing());
                }, 5000);
            };

            this.ws.onmessage = (event: MessageEvent) => {
                if (event.data instanceof ArrayBuffer) {
                    this.handleBinaryMessage(event.data);
                } else if (typeof event.data === 'string') {
                    this.handleTextMessage(event.data);
                }
            };

            this.ws.onclose = (event: CloseEvent) => {
                logger.warn('WebSocket closed', { code: event.code, reason: event.reason });
                this.cleanup();

                if (event.code !== 1000) {
                    // Abnormal close — attempt reconnect
                    this.scheduleReconnect();
                } else {
                    useGameStore.getState().setConnectionState('disconnected');
                }
            };

            this.ws.onerror = () => {
                logger.error('WebSocket error');
            };
        } catch (error) {
            logger.error('Failed to create WebSocket', { error: String(error) });
            this.scheduleReconnect();
        }
    }

    disconnect(): void {
        this.cleanup();
        if (this.ws) {
            if (this.ws.readyState === WebSocket.OPEN || this.ws.readyState === WebSocket.CLOSING) {
                this.ws.close(1000, 'User initiated disconnect');
            }
            this.ws.onopen = null;
            this.ws.onmessage = null;
            this.ws.onclose = null;
            this.ws.onerror = null;
            this.ws = null;
        }
        useGameStore.getState().setConnectionState('disconnected');
    }

    sendCommand(cmd: CommandMessage): void {
        if (this.ws?.readyState !== WebSocket.OPEN) return;
        this.ws.send(this.encoder.encodeCommand(cmd));
        useGameStore.getState().setLastCommand(cmd);
    }

    sendReset(): void {
        if (this.ws?.readyState !== WebSocket.OPEN) return;
        this.ws.send(this.encoder.encodeReset());
    }

    private handleBinaryMessage(data: ArrayBuffer): void {
        const state = this.decoder.decodeWorldState(data);
        if (!state) return;

        const store = useGameStore.getState();
        store.setEntities(state.entities);
        store.setImpactCount(state.impactCount);
        store.recordBroadcast();

        // Update connection state to streaming
        if (store.connectionState !== 'streaming') {
            store.setConnectionState('streaming');
        }

        // Reset stale timer
        this.resetStaleTimer();
    }

    private handleTextMessage(data: string): void {
        try {
            const msg = JSON.parse(data) as ServerJsonMessage;

            switch (msg.type) {
                case 'welcome':
                    logger.info('Server welcome received', { tickRate: msg.tickRate });
                    break;

                case 'pong': {
                    const latency = Date.now() - msg.clientTimestamp;
                    useGameStore.getState().setLatency(latency);
                    break;
                }

                case 'event':
                    logger.event(msg.event, msg.data);
                    break;

                case 'error':
                    logger.warn('Server error: ' + msg.message, { code: msg.code });
                    break;
            }
        } catch (error) {
            logger.error('Failed to parse server message', { error: String(error) });
        }
    }

    private scheduleReconnect(): void {
        if (this.reconnectAttempts >= this.maxReconnectAttempts) {
            logger.error('Max reconnect attempts reached');
            useGameStore.getState().setConnectionState('disconnected');
            return;
        }

        useGameStore.getState().setConnectionState('reconnecting');
        const delay = this.getBackoffDelay(this.reconnectAttempts);
        logger.debug('Reconnecting in ' + delay + 'ms', { attempt: this.reconnectAttempts });

        this.reconnectTimer = setTimeout(() => {
            this.reconnectAttempts++;
            this.decoder.reset();
            this.connect();
        }, delay);
    }

    private getBackoffDelay(attempt: number): number {
        return Math.min(1000 * Math.pow(2, attempt), 30000);
    }

    private resetStaleTimer(): void {
        if (this.staleTimer) clearTimeout(this.staleTimer);
        this.staleTimer = setTimeout(() => {
            const store = useGameStore.getState();
            if (store.connectionState === 'streaming') {
                store.setConnectionState('stale');
            }
        }, 2000);
    }

    private cleanup(): void {
        if (this.pingTimer) {
            clearInterval(this.pingTimer);
            this.pingTimer = null;
        }
        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }
        if (this.staleTimer) {
            clearTimeout(this.staleTimer);
            this.staleTimer = null;
        }
    }
}
