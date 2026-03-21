import { describe, it, expect } from 'vitest';
import * as fc from 'fast-check';
import { clamp } from '../../src/utils/math';

describe('Input property tests', () => {
    it('clamp always returns value in [min, max]', () => {
        fc.assert(
            fc.property(
                fc.double({ noNaN: true, noDefaultInfinity: true }),
                fc.double({ noNaN: true, noDefaultInfinity: true }),
                fc.double({ noNaN: true, noDefaultInfinity: true }),
                (value, a, b) => {
                    const min = Math.min(a, b);
                    const max = Math.max(a, b);
                    const result = clamp(value, min, max);
                    expect(result).toBeGreaterThanOrEqual(min);
                    expect(result).toBeLessThanOrEqual(max);
                },
            ),
        );
    });

    it('clamp is idempotent', () => {
        fc.assert(
            fc.property(
                fc.double({ noNaN: true, noDefaultInfinity: true, min: -100, max: 100 }),
                (value) => {
                    const once = clamp(value, -1, 1);
                    const twice = clamp(once, -1, 1);
                    expect(once).toBe(twice);
                },
            ),
        );
    });

    it('throttle and steering are always in [-1, 1] after clamp', () => {
        fc.assert(
            fc.property(
                fc.double({ noNaN: true, noDefaultInfinity: true, min: -1000, max: 1000 }),
                fc.double({ noNaN: true, noDefaultInfinity: true, min: -1000, max: 1000 }),
                (throttle, steering) => {
                    const t = clamp(throttle, -1, 1);
                    const s = clamp(steering, -1, 1);
                    expect(t).toBeGreaterThanOrEqual(-1);
                    expect(t).toBeLessThanOrEqual(1);
                    expect(s).toBeGreaterThanOrEqual(-1);
                    expect(s).toBeLessThanOrEqual(1);
                },
            ),
        );
    });
});
