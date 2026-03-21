export type ConnectionState =
    | 'disconnected'
    | 'connecting'
    | 'connected'
    | 'streaming'
    | 'stale'
    | 'reconnecting';

export interface WelcomeMessage {
    type: 'welcome';
    tickRate: number;
    broadcastRate: number;
    sceneId: string;
    protocolVersion: number;
    serverVersion: string;
}

export interface PongMessage {
    type: 'pong';
    clientTimestamp: number;
    serverTimestamp: number;
}

export interface EventMessage {
    type: 'event';
    event: string;
    data: Record<string, unknown>;
}

export interface ErrorMessage {
    type: 'error';
    code: string;
    message: string;
}

export type ServerJsonMessage = WelcomeMessage | PongMessage | EventMessage | ErrorMessage;
