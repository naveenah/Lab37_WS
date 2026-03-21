export enum LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    CRITICAL = 5,
}

interface LogEntry {
    timestamp: string;
    level: string;
    message: string;
    data?: Record<string, unknown>;
}

export class Logger {
    private level: LogLevel;
    private buffer: LogEntry[] = [];
    private readonly MAX_BUFFER = 1000;
    private flushTimer: ReturnType<typeof setInterval> | null = null;

    constructor(level: LogLevel = LogLevel.INFO) {
        this.level = level;
        this.flushTimer = setInterval(() => this.flush(), 5000);

        // Prevent the timer from keeping Node/vitest processes alive
        if (typeof this.flushTimer === 'object' && this.flushTimer !== null && 'unref' in this.flushTimer) {
            (this.flushTimer as { unref: () => void }).unref();
        }

        // Clean up on page unload in browser environments
        if (typeof window !== 'undefined') {
            window.addEventListener('beforeunload', () => this.destroy());
        }
    }

    event(category: string, data: Record<string, unknown>): void {
        this.write(LogLevel.INFO, `[EVENT:${category}]`, data);
    }

    debug(msg: string, data?: Record<string, unknown>): void {
        this.write(LogLevel.DEBUG, msg, data);
    }

    info(msg: string, data?: Record<string, unknown>): void {
        this.write(LogLevel.INFO, msg, data);
    }

    warn(msg: string, data?: Record<string, unknown>): void {
        this.write(LogLevel.WARN, msg, data);
    }

    error(msg: string, data?: Record<string, unknown>): void {
        this.write(LogLevel.ERROR, msg, data);
    }

    destroy(): void {
        if (this.flushTimer) {
            clearInterval(this.flushTimer);
            this.flushTimer = null;
        }
        this.flush();
    }

    private write(level: LogLevel, msg: string, data?: Record<string, unknown>): void {
        if (level < this.level) return;

        const entry: LogEntry = {
            timestamp: new Date().toISOString(),
            level: LogLevel[level] ?? 'UNKNOWN',
            message: msg,
            data,
        };

        if (level >= LogLevel.ERROR) {
            console.error(`[${entry.level}] ${msg}`, data ?? '');
        } else if (level >= LogLevel.WARN) {
            console.warn(`[${entry.level}] ${msg}`, data ?? '');
        } else if (level >= LogLevel.INFO) {
            console.log(`[${entry.level}] ${msg}`, data ?? '');
        }

        this.buffer.push(entry);
        if (this.buffer.length >= this.MAX_BUFFER) {
            this.flush();
        }
    }

    private flush(): void {
        if (this.buffer.length === 0) return;
        const batch = this.buffer.splice(0);
        try {
            const existing = JSON.parse(sessionStorage.getItem('teleop_logs') ?? '[]') as LogEntry[];
            sessionStorage.setItem('teleop_logs',
                JSON.stringify([...existing, ...batch].slice(-5000)));
        } catch {
            // sessionStorage full — drop oldest
        }
    }
}

export const logger = new Logger(LogLevel.INFO);
