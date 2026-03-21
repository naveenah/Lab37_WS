export interface Vec2 {
    x: number;
    y: number;
}

export function clamp(value: number, min: number, max: number): number {
    return Math.max(min, Math.min(max, value));
}

export function lerp(a: number, b: number, t: number): number {
    return a + (b - a) * t;
}

export function vec2Distance(a: Vec2, b: Vec2): number {
    const dx = b.x - a.x;
    const dy = b.y - a.y;
    return Math.sqrt(dx * dx + dy * dy);
}

export function vec2Length(v: Vec2): number {
    return Math.sqrt(v.x * v.x + v.y * v.y);
}
