import type { CommandMessage } from '../types/commands';

export class CommandEncoder {
    encodeCommand(cmd: CommandMessage): string {
        return JSON.stringify(cmd);
    }

    encodeHandshake(): string {
        return JSON.stringify({
            type: 'handshake',
            clientVersion: '1.0.0',
            protocolVersion: 1,
        });
    }

    encodePing(): string {
        return JSON.stringify({
            type: 'ping',
            timestamp: Date.now(),
        });
    }

    encodeReset(): string {
        return JSON.stringify({ type: 'reset' });
    }
}
