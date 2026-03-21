import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { InputHandler } from '../../src/services/InputHandler';

describe('InputHandler', () => {
    let handler: InputHandler;

    beforeEach(() => {
        handler = new InputHandler();
        handler.startListening();
    });

    afterEach(() => {
        handler.stopListening();
    });

    function pressKey(code: string): void {
        window.dispatchEvent(new KeyboardEvent('keydown', { code }));
    }

    function releaseKey(code: string): void {
        window.dispatchEvent(new KeyboardEvent('keyup', { code }));
    }

    it('returns zero command with no input', () => {
        const cmd = handler.getCurrentCommand();
        expect(cmd.throttle).toBe(0);
        expect(cmd.steering).toBe(0);
        expect(cmd.type).toBe('command');
    });

    it('W key produces positive throttle', () => {
        pressKey('KeyW');
        const cmd = handler.getCurrentCommand();
        expect(cmd.throttle).toBe(1);
        expect(cmd.steering).toBe(0);
    });

    it('S key produces negative throttle', () => {
        pressKey('KeyS');
        const cmd = handler.getCurrentCommand();
        expect(cmd.throttle).toBe(-1);
    });

    it('A key produces positive steering (left = counter-clockwise in Ackermann model)', () => {
        pressKey('KeyA');
        const cmd = handler.getCurrentCommand();
        expect(cmd.steering).toBe(1);
    });

    it('D key produces negative steering (right = clockwise in Ackermann model)', () => {
        pressKey('KeyD');
        const cmd = handler.getCurrentCommand();
        expect(cmd.steering).toBe(-1);
    });

    it('arrow keys work as alternatives', () => {
        pressKey('ArrowUp');
        expect(handler.getCurrentCommand().throttle).toBe(1);
        releaseKey('ArrowUp');

        pressKey('ArrowLeft');
        expect(handler.getCurrentCommand().steering).toBe(1);
    });

    it('space cancels throttle', () => {
        pressKey('KeyW');
        pressKey('Space');
        const cmd = handler.getCurrentCommand();
        expect(cmd.throttle).toBe(0);
    });

    it('releasing key stops input', () => {
        pressKey('KeyW');
        expect(handler.getCurrentCommand().throttle).toBe(1);
        releaseKey('KeyW');
        expect(handler.getCurrentCommand().throttle).toBe(0);
    });

    it('increments sequence number', () => {
        const cmd1 = handler.getCurrentCommand();
        const cmd2 = handler.getCurrentCommand();
        expect(cmd2.sequence).toBe(cmd1.sequence + 1);
    });

    it('hasInput returns false with no keys', () => {
        expect(handler.hasInput()).toBe(false);
    });

    it('hasInput returns true with active input', () => {
        pressKey('KeyW');
        expect(handler.hasInput()).toBe(true);
    });

    it('clamps throttle and steering to [-1, 1]', () => {
        pressKey('KeyW');
        pressKey('KeyD');
        const cmd = handler.getCurrentCommand();
        expect(cmd.throttle).toBeGreaterThanOrEqual(-1);
        expect(cmd.throttle).toBeLessThanOrEqual(1);
        expect(cmd.steering).toBeGreaterThanOrEqual(-1);
        expect(cmd.steering).toBeLessThanOrEqual(1);
    });

    it('ignores repeat keydown events', () => {
        window.dispatchEvent(new KeyboardEvent('keydown', { code: 'KeyW' }));
        window.dispatchEvent(new KeyboardEvent('keydown', { code: 'KeyW', repeat: true }));
        const cmd = handler.getCurrentCommand();
        expect(cmd.throttle).toBe(1);
    });
});
