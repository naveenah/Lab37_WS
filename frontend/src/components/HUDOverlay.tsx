import React from 'react';
import { useGameStore } from '../store/gameStore';
import type { ConnectionState } from '../types/protocol';

const CONNECTION_COLORS: Record<ConnectionState, string> = {
    disconnected: '#ff4444',
    connecting: '#ffaa00',
    connected: '#44ff44',
    streaming: '#44ff44',
    reconnecting: '#ffaa00',
    stale: '#ff8800',
};

const CONNECTION_LABELS: Record<ConnectionState, string> = {
    disconnected: 'Disconnected',
    connecting: 'Connecting',
    connected: 'Connected',
    streaming: 'Streaming',
    reconnecting: 'Reconnecting',
    stale: 'Stale',
};

interface HUDOverlayProps {
    onReset?: () => void;
}

export const HUDOverlay: React.FC<HUDOverlayProps> = ({ onReset }) => {
    const connectionState = useGameStore((s) => s.connectionState);
    const impactCount = useGameStore((s) => s.impactCount);
    const latencyMs = useGameStore((s) => s.latencyMs);
    const fps = useGameStore((s) => s.fps);

    return (
        <div style={{
            position: 'absolute',
            top: 0,
            left: 0,
            right: 0,
            pointerEvents: 'none',
            padding: '12px 16px',
            display: 'flex',
            justifyContent: 'space-between',
            alignItems: 'flex-start',
            fontFamily: 'monospace',
            fontSize: '13px',
        }}>
            {/* Left: Connection + Performance */}
            <div style={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
                <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
                    <div style={{
                        width: '8px',
                        height: '8px',
                        borderRadius: '50%',
                        backgroundColor: CONNECTION_COLORS[connectionState],
                        boxShadow: `0 0 4px ${CONNECTION_COLORS[connectionState]}`,
                    }} />
                    <span style={{ color: CONNECTION_COLORS[connectionState] }}>
                        {CONNECTION_LABELS[connectionState]}
                    </span>
                </div>
                <span style={{ color: '#888' }}>
                    FPS: {fps} | Latency: {latencyMs}ms
                </span>
            </div>

            {/* Center: Impact Counter */}
            <div style={{
                display: 'flex',
                flexDirection: 'column',
                alignItems: 'center',
                gap: '2px',
            }}>
                <span style={{ color: '#666', fontSize: '11px', textTransform: 'uppercase' }}>
                    Impacts
                </span>
                <span style={{
                    color: impactCount > 0 ? '#ff6b6b' : '#44ff44',
                    fontSize: '24px',
                    fontWeight: 'bold',
                }}>
                    {impactCount}
                </span>
            </div>

            {/* Right: Controls hint + Reset button */}
            <div style={{
                display: 'flex',
                flexDirection: 'column',
                alignItems: 'flex-end',
                gap: '4px',
                color: '#555',
                fontSize: '11px',
                pointerEvents: 'auto',
            }}>
                <span>W/S - Throttle</span>
                <span>A/D - Steering</span>
                <span>Space - Brake</span>
                <span>Gamepad supported</span>
                {onReset && (
                    <button
                        onClick={onReset}
                        style={{
                            marginTop: '8px',
                            padding: '6px 16px',
                            background: '#333',
                            color: '#fff',
                            border: '1px solid #555',
                            borderRadius: '4px',
                            cursor: 'pointer',
                            fontFamily: 'monospace',
                            fontSize: '12px',
                        }}
                        onMouseEnter={(e) => {
                            e.currentTarget.style.background = '#555';
                        }}
                        onMouseLeave={(e) => {
                            e.currentTarget.style.background = '#333';
                        }}
                    >
                        Reset
                    </button>
                )}
            </div>
        </div>
    );
};
