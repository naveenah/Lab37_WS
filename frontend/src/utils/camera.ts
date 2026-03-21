import type { Vec2 } from './math';

export class Camera {
    x = 0;
    y = 0;
    zoom = 30; // pixels per meter

    constructor(x = 0, y = 0, zoom = 30) {
        this.x = x;
        this.y = y;
        this.zoom = zoom;
    }

    worldToScreen(world: Vec2, canvasWidth: number, canvasHeight: number): Vec2 {
        return {
            x: (world.x - this.x) * this.zoom + canvasWidth / 2,
            y: canvasHeight / 2 - (world.y - this.y) * this.zoom, // Flip Y for screen
        };
    }

    screenToWorld(screen: Vec2, canvasWidth: number, canvasHeight: number): Vec2 {
        return {
            x: (screen.x - canvasWidth / 2) / this.zoom + this.x,
            y: (canvasHeight / 2 - screen.y) / this.zoom + this.y,
        };
    }

    followTarget(targetX: number, targetY: number, smoothing = 0.1): void {
        this.x += (targetX - this.x) * smoothing;
        this.y += (targetY - this.y) * smoothing;
    }
}
