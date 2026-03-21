export interface CommandMessage {
    type: 'command';
    throttle: number;   // -1.0 (full reverse) to 1.0 (full forward)
    steering: number;   // -1.0 (full left) to 1.0 (full right)
    sequence: number;   // Monotonically increasing per session
    timestamp: number;  // Client-side Unix ms timestamp
}

export interface RawInput {
    throttle: number;
    steering: number;
    source: 'keyboard' | 'gamepad';
}
