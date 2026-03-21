import React from 'react';
import { useGameStore } from '../store/gameStore';

export const ControlPanel: React.FC = () => {
    const lastCommand = useGameStore((s) => s.lastCommand);
    const inputSource = useGameStore((s) => s.inputSource);

    const throttle = lastCommand?.throttle ?? 0;
    const steering = lastCommand?.steering ?? 0;

    return (
        <div style={{
            position: 'absolute',
            bottom: '12px',
            left: '50%',
            transform: 'translateX(-50%)',
            display: 'flex',
            gap: '24px',
            alignItems: 'flex-end',
            pointerEvents: 'none',
            fontFamily: 'monospace',
            fontSize: '12px',
            color: '#888',
        }}>
            {/* Throttle gauge */}
            <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '4px' }}>
                <span style={{ fontSize: '10px', color: '#555' }}>THROTTLE</span>
                <div style={{
                    width: '20px',
                    height: '60px',
                    border: '1px solid #333',
                    borderRadius: '3px',
                    position: 'relative',
                    overflow: 'hidden',
                }}>
                    {/* Center line */}
                    <div style={{
                        position: 'absolute',
                        top: '50%',
                        left: 0,
                        right: 0,
                        height: '1px',
                        background: '#444',
                    }} />
                    {/* Fill */}
                    <div style={{
                        position: 'absolute',
                        left: 0,
                        right: 0,
                        ...(throttle >= 0
                            ? {
                                bottom: '50%',
                                height: `${Math.abs(throttle) * 50}%`,
                                background: '#44ff44',
                            }
                            : {
                                top: '50%',
                                height: `${Math.abs(throttle) * 50}%`,
                                background: '#ff4444',
                            }),
                        opacity: 0.7,
                    }} />
                </div>
                <span>{throttle.toFixed(2)}</span>
            </div>

            {/* Steering indicator */}
            <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '4px' }}>
                <span style={{ fontSize: '10px', color: '#555' }}>STEERING</span>
                <div style={{
                    width: '80px',
                    height: '20px',
                    border: '1px solid #333',
                    borderRadius: '3px',
                    position: 'relative',
                    overflow: 'hidden',
                }}>
                    {/* Center line */}
                    <div style={{
                        position: 'absolute',
                        top: 0,
                        bottom: 0,
                        left: '50%',
                        width: '1px',
                        background: '#444',
                    }} />
                    {/* Fill */}
                    <div style={{
                        position: 'absolute',
                        top: 0,
                        bottom: 0,
                        ...(steering >= 0
                            ? {
                                left: '50%',
                                width: `${Math.abs(steering) * 50}%`,
                            }
                            : {
                                right: '50%',
                                width: `${Math.abs(steering) * 50}%`,
                            }),
                        background: '#5599ff',
                        opacity: 0.7,
                    }} />
                </div>
                <span>{steering.toFixed(2)}</span>
            </div>

            {/* Input source */}
            <div style={{
                display: 'flex',
                flexDirection: 'column',
                alignItems: 'center',
                gap: '4px',
                fontSize: '10px',
                color: '#555',
            }}>
                <span>INPUT</span>
                <span style={{
                    color: inputSource === 'none' ? '#444' : '#aaa',
                    textTransform: 'uppercase',
                }}>
                    {inputSource}
                </span>
            </div>
        </div>
    );
};
