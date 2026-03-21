import type { CommandMessage, RawInput } from '../types/commands';
import { clamp } from '../utils/math';

export class InputHandler {
    private keyState = new Map<string, boolean>();
    private gamepadIndex: number | null = null;
    private sequence = 0;
    private readonly DEADZONE = 0.15;

    private readonly KEY_MAP = {
        forward: ['KeyW', 'ArrowUp'],
        backward: ['KeyS', 'ArrowDown'],
        left: ['KeyA', 'ArrowLeft'],
        right: ['KeyD', 'ArrowRight'],
        brake: ['Space'],
    };

    startListening(): void {
        window.addEventListener('keydown', this.onKeyDown);
        window.addEventListener('keyup', this.onKeyUp);
        window.addEventListener('gamepadconnected', this.onGamepadConnected);
        window.addEventListener('gamepaddisconnected', this.onGamepadDisconnected);
    }

    stopListening(): void {
        window.removeEventListener('keydown', this.onKeyDown);
        window.removeEventListener('keyup', this.onKeyUp);
        window.removeEventListener('gamepadconnected', this.onGamepadConnected);
        window.removeEventListener('gamepaddisconnected', this.onGamepadDisconnected);
    }

    getCurrentCommand(): CommandMessage {
        const keyboard = this.getKeyboardInput();
        const gamepad = this.getGamepadInput();
        return this.mergeInputs(keyboard, gamepad);
    }

    resetSequence(): void {
        this.sequence = 0;
    }

    hasInput(): boolean {
        const cmd = this.getCurrentCommand();
        return Math.abs(cmd.throttle) > 0.01 || Math.abs(cmd.steering) > 0.01;
    }

    private getKeyboardInput(): RawInput {
        let throttle = 0;
        let steering = 0;

        if (this.isPressed(this.KEY_MAP.forward)) throttle += 1.0;
        if (this.isPressed(this.KEY_MAP.backward)) throttle -= 1.0;
        if (this.isPressed(this.KEY_MAP.left)) steering += 1.0;
        if (this.isPressed(this.KEY_MAP.right)) steering -= 1.0;
        if (this.isPressed(this.KEY_MAP.brake)) throttle = 0;

        return { throttle, steering, source: 'keyboard' };
    }

    private getGamepadInput(): RawInput | null {
        if (this.gamepadIndex === null) return null;
        const gamepads = navigator.getGamepads();
        const gp = gamepads[this.gamepadIndex];
        if (!gp) return null;

        return {
            throttle: this.applyDeadzone(-(gp.axes[1] ?? 0)), // Left stick Y (inverted)
            steering: this.applyDeadzone(-(gp.axes[0] ?? 0)),   // Left stick X (inverted)
            source: 'gamepad',
        };
    }

    private mergeInputs(keyboard: RawInput, gamepad: RawInput | null): CommandMessage {
        const input = gamepad && (Math.abs(gamepad.throttle) > 0 || Math.abs(gamepad.steering) > 0)
            ? gamepad
            : keyboard;

        return {
            type: 'command',
            throttle: clamp(input.throttle, -1, 1),
            steering: clamp(input.steering, -1, 1),
            sequence: ++this.sequence,
            timestamp: Date.now(),
        };
    }

    private applyDeadzone(value: number): number {
        if (Math.abs(value) < this.DEADZONE) return 0;
        const sign = Math.sign(value);
        return sign * ((Math.abs(value) - this.DEADZONE) / (1 - this.DEADZONE));
    }

    private isPressed(keys: string[]): boolean {
        return keys.some(k => this.keyState.get(k));
    }

    private onKeyDown = (event: KeyboardEvent): void => {
        if (event.repeat) return;
        this.keyState.set(event.code, true);
    };

    private onKeyUp = (event: KeyboardEvent): void => {
        this.keyState.set(event.code, false);
    };

    private onGamepadConnected = (event: GamepadEvent): void => {
        this.gamepadIndex = event.gamepad.index;
    };

    private onGamepadDisconnected = (_event: GamepadEvent): void => {
        this.gamepadIndex = null;
    };
}
