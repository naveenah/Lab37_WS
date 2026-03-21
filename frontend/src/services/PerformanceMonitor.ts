export class PerformanceMonitor {
    private frameTimes: number[] = [];
    private readonly maxSamples = 60;
    private lastFrameTime = 0;

    recordFrame(): number {
        const now = performance.now();
        if (this.lastFrameTime > 0) {
            const dt = now - this.lastFrameTime;
            this.frameTimes.push(dt);
            if (this.frameTimes.length > this.maxSamples) {
                this.frameTimes.shift();
            }
        }
        this.lastFrameTime = now;
        return now;
    }

    getFps(): number {
        if (this.frameTimes.length === 0) return 0;
        const avg = this.frameTimes.reduce((a, b) => a + b, 0) / this.frameTimes.length;
        return Math.round(1000 / avg);
    }

    getFrameTimeP99(): number {
        if (this.frameTimes.length === 0) return 0;
        const sorted = [...this.frameTimes].sort((a, b) => a - b);
        const idx = Math.floor(sorted.length * 0.99);
        return sorted[idx] ?? 0;
    }
}
