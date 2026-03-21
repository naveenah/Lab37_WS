import { describe, it, expect } from 'vitest';
import * as fc from 'fast-check';
import { Camera } from '../../src/utils/camera';

describe('Rendering property tests', () => {
    it('worldToScreen → screenToWorld roundtrips', () => {
        fc.assert(
            fc.property(
                fc.double({ noNaN: true, noDefaultInfinity: true, min: -1000, max: 1000 }),
                fc.double({ noNaN: true, noDefaultInfinity: true, min: -1000, max: 1000 }),
                (wx, wy) => {
                    const cam = new Camera(0, 0, 30);
                    const w = 800;
                    const h = 600;
                    const screen = cam.worldToScreen({ x: wx, y: wy }, w, h);
                    const world = cam.screenToWorld(screen, w, h);
                    expect(world.x).toBeCloseTo(wx, 6);
                    expect(world.y).toBeCloseTo(wy, 6);
                },
            ),
        );
    });

    it('screenToWorld → worldToScreen roundtrips', () => {
        fc.assert(
            fc.property(
                fc.double({ noNaN: true, noDefaultInfinity: true, min: 0, max: 1920 }),
                fc.double({ noNaN: true, noDefaultInfinity: true, min: 0, max: 1080 }),
                (sx, sy) => {
                    const cam = new Camera(5, 5, 40);
                    const w = 1920;
                    const h = 1080;
                    const world = cam.screenToWorld({ x: sx, y: sy }, w, h);
                    const screen = cam.worldToScreen(world, w, h);
                    expect(screen.x).toBeCloseTo(sx, 6);
                    expect(screen.y).toBeCloseTo(sy, 6);
                },
            ),
        );
    });

    it('camera origin maps to screen center', () => {
        fc.assert(
            fc.property(
                fc.double({ noNaN: true, noDefaultInfinity: true, min: -500, max: 500 }),
                fc.double({ noNaN: true, noDefaultInfinity: true, min: -500, max: 500 }),
                fc.double({ noNaN: true, noDefaultInfinity: true, min: 1, max: 200 }),
                (cx, cy, zoom) => {
                    const cam = new Camera(cx, cy, zoom);
                    const w = 800;
                    const h = 600;
                    const screen = cam.worldToScreen({ x: cx, y: cy }, w, h);
                    expect(screen.x).toBeCloseTo(w / 2, 6);
                    expect(screen.y).toBeCloseTo(h / 2, 6);
                },
            ),
        );
    });

    it('followTarget moves camera toward target', () => {
        fc.assert(
            fc.property(
                fc.double({ noNaN: true, noDefaultInfinity: true, min: -100, max: 100 }),
                fc.double({ noNaN: true, noDefaultInfinity: true, min: -100, max: 100 }),
                (tx, ty) => {
                    const cam = new Camera(0, 0, 30);
                    const startDistSq = tx * tx + ty * ty;
                    cam.followTarget(tx, ty, 0.5);
                    const dx = tx - cam.x;
                    const dy = ty - cam.y;
                    const endDistSq = dx * dx + dy * dy;
                    // After follow, camera should be closer to target (or at target)
                    expect(endDistSq).toBeLessThanOrEqual(startDistSq + 1e-10);
                },
            ),
        );
    });
});
