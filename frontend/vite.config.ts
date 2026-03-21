import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import path from 'path';

export default defineConfig({
    plugins: [react()],
    resolve: {
        alias: {
            '@': path.resolve(__dirname, './src'),
        },
    },
    server: {
        port: 3000,
        proxy: {
            '/ws': {
                target: 'ws://localhost:9001',
                ws: true,
            },
        },
    },
    test: {
        globals: true,
        environment: 'jsdom',
        setupFiles: [],
        exclude: ['tests/e2e/**', 'node_modules/**'],
    },
});
