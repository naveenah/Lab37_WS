import React, { useRef, useEffect, useCallback } from 'react';
import { useGameStore } from '../store/gameStore';
import { Camera } from '../utils/camera';
import { PerformanceMonitor } from '../services/PerformanceMonitor';
import { EntityType } from '../types/entities';
import type { EntityState } from '../types/entities';

const GRID_SPACING = 1; // 1 meter
const GRID_COLOR = '#1a1a2e';
const BACKGROUND_COLOR = '#0f0f1a';

const camera = new Camera(0, 0, 30);
const perfMonitor = new PerformanceMonitor();

export const SimulationCanvas: React.FC = () => {
    const canvasRef = useRef<HTMLCanvasElement>(null);
    const animFrameRef = useRef<number>(0);
    const entities = useGameStore((s) => s.entities);
    const robotId = useGameStore((s) => s.robotId);
    const entitiesRef = useRef(entities);
    const robotIdRef = useRef(robotId);

    useEffect(() => {
        entitiesRef.current = entities;
        robotIdRef.current = robotId;
    }, [entities, robotId]);

    const draw = useCallback(() => {
        const canvas = canvasRef.current;
        if (!canvas) return;
        const ctx = canvas.getContext('2d');
        if (!ctx) return;

        perfMonitor.recordFrame();
        useGameStore.getState().setFps(perfMonitor.getFps());

        // Resize canvas to fill container
        const dpr = window.devicePixelRatio || 1;
        const rect = canvas.getBoundingClientRect();
        if (canvas.width !== rect.width * dpr || canvas.height !== rect.height * dpr) {
            canvas.width = rect.width * dpr;
            canvas.height = rect.height * dpr;
            ctx.scale(dpr, dpr);
        }

        const w = rect.width;
        const h = rect.height;

        // Follow robot
        const currentEntities = entitiesRef.current;
        const currentRobotId = robotIdRef.current;
        if (currentRobotId !== null) {
            const robot = currentEntities.get(currentRobotId);
            if (robot) {
                camera.followTarget(robot.x, robot.y, 0.08);
            }
        }

        // Clear
        ctx.fillStyle = BACKGROUND_COLOR;
        ctx.fillRect(0, 0, w, h);

        // Draw grid
        drawGrid(ctx, w, h);

        // Sort entities by layer: static first, dynamic, then robot on top
        const sorted = [...currentEntities.values()].sort((a, b) => {
            const layerOrder = (t: EntityType) =>
                t === EntityType.StaticObstacle ? 0 : t === EntityType.DynamicObstacle ? 1 : 2;
            return layerOrder(a.type) - layerOrder(b.type);
        });

        for (const entity of sorted) {
            drawEntity(ctx, entity, w, h);
        }

        animFrameRef.current = requestAnimationFrame(draw);
    }, []);

    useEffect(() => {
        animFrameRef.current = requestAnimationFrame(draw);
        return () => cancelAnimationFrame(animFrameRef.current);
    }, [draw]);

    return (
        <canvas
            ref={canvasRef}
            style={{
                width: '100%',
                height: '100%',
                display: 'block',
            }}
        />
    );
};

function drawGrid(ctx: CanvasRenderingContext2D, w: number, h: number): void {
    const topLeft = camera.screenToWorld({ x: 0, y: 0 }, w, h);
    const bottomRight = camera.screenToWorld({ x: w, y: h }, w, h);

    const startX = Math.floor(topLeft.x / GRID_SPACING) * GRID_SPACING;
    const endX = Math.ceil(bottomRight.x / GRID_SPACING) * GRID_SPACING;
    const startY = Math.floor(bottomRight.y / GRID_SPACING) * GRID_SPACING;
    const endY = Math.ceil(topLeft.y / GRID_SPACING) * GRID_SPACING;

    ctx.strokeStyle = GRID_COLOR;
    ctx.lineWidth = 0.5;
    ctx.beginPath();

    for (let x = startX; x <= endX; x += GRID_SPACING) {
        const screen = camera.worldToScreen({ x, y: 0 }, w, h);
        ctx.moveTo(screen.x, 0);
        ctx.lineTo(screen.x, h);
    }

    for (let y = startY; y <= endY; y += GRID_SPACING) {
        const screen = camera.worldToScreen({ x: 0, y }, w, h);
        ctx.moveTo(0, screen.y);
        ctx.lineTo(w, screen.y);
    }

    ctx.stroke();
}

function drawEntity(
    ctx: CanvasRenderingContext2D,
    entity: EntityState,
    w: number,
    h: number,
): void {
    if (entity.vertices.length < 3) return;

    const screenVerts = entity.vertices.map((v) =>
        camera.worldToScreen(v, w, h),
    );

    // Fill polygon
    ctx.fillStyle = entity.color + '80'; // 50% alpha
    ctx.strokeStyle = entity.color;
    ctx.lineWidth = entity.type === EntityType.Robot ? 2 : 1;

    const first = screenVerts[0];
    if (!first) return;

    ctx.beginPath();
    ctx.moveTo(first.x, first.y);
    for (let i = 1; i < screenVerts.length; i++) {
        const v = screenVerts[i]!;
        ctx.lineTo(v.x, v.y);
    }
    ctx.closePath();
    ctx.fill();
    ctx.stroke();

    // Draw heading arrow for robot
    if (entity.type === EntityType.Robot) {
        const center = camera.worldToScreen({ x: entity.x, y: entity.y }, w, h);
        const arrowLen = camera.zoom * 0.8;
        // Screen Y is flipped, so negate the sin component
        const tipX = center.x + Math.cos(entity.heading) * arrowLen;
        const tipY = center.y - Math.sin(entity.heading) * arrowLen;

        ctx.strokeStyle = '#ffffff';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(center.x, center.y);
        ctx.lineTo(tipX, tipY);
        ctx.stroke();

        // Arrowhead
        const headLen = 6;
        const angle = Math.atan2(-(tipY - center.y), tipX - center.x);
        ctx.beginPath();
        ctx.moveTo(tipX, tipY);
        ctx.lineTo(
            tipX - headLen * Math.cos(angle - Math.PI / 6),
            tipY + headLen * Math.sin(angle - Math.PI / 6),
        );
        ctx.moveTo(tipX, tipY);
        ctx.lineTo(
            tipX - headLen * Math.cos(angle + Math.PI / 6),
            tipY + headLen * Math.sin(angle + Math.PI / 6),
        );
        ctx.stroke();
    }
}
