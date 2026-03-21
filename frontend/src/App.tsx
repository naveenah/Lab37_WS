import React, { useCallback, useEffect, useRef } from 'react';
import { ErrorBoundary } from './components/ErrorBoundary';
import { SimulationCanvas } from './components/SimulationCanvas';
import { HUDOverlay } from './components/HUDOverlay';
import { ControlPanel } from './components/ControlPanel';
import { ConnectionManager } from './services/ConnectionManager';
import { InputHandler } from './services/InputHandler';
import { useGameStore } from './store/gameStore';
import { logger } from './utils/logger';

const WS_URL = import.meta.env.VITE_WS_URL ?? `ws://${window.location.hostname}:9001`;
const COMMAND_RATE_MS = 100; // 10 Hz

const AppContent: React.FC = () => {
    const connectionRef = useRef<ConnectionManager | null>(null);
    const inputRef = useRef<InputHandler | null>(null);
    const commandTimerRef = useRef<ReturnType<typeof setInterval> | null>(null);

    useEffect(() => {
        const connection = new ConnectionManager(WS_URL);
        const input = new InputHandler();

        connectionRef.current = connection;
        inputRef.current = input;

        connection.connect();
        input.startListening();

        // Send commands at 10 Hz — always send while connected to keep robot responsive
        commandTimerRef.current = setInterval(() => {
            const cmd = input.getCurrentCommand();
            if (Math.abs(cmd.throttle) > 0.01 || Math.abs(cmd.steering) > 0.01) {
                logger.debug('[CMD]', { throttle: cmd.throttle.toFixed(2), steering: cmd.steering.toFixed(2), seq: cmd.sequence });
            }
            connection.sendCommand(cmd);
            useGameStore.getState().setInputSource(input.getActiveSource());
        }, COMMAND_RATE_MS);

        return () => {
            if (commandTimerRef.current) clearInterval(commandTimerRef.current);
            input.stopListening();
            connection.disconnect();
        };
    }, []);

    const handleReset = useCallback(() => {
        connectionRef.current?.sendReset();
        // Reset the input handler's sequence counter
        if (inputRef.current) {
            inputRef.current.resetSequence();
        }
    }, []);

    return (
        <div style={{
            position: 'relative',
            width: '100vw',
            height: '100vh',
            overflow: 'hidden',
            background: '#0f0f1a',
        }}>
            <SimulationCanvas />
            <HUDOverlay onReset={handleReset} />
            <ControlPanel />
        </div>
    );
};

const App: React.FC = () => {
    return (
        <ErrorBoundary>
            <AppContent />
        </ErrorBoundary>
    );
};

export default App;
