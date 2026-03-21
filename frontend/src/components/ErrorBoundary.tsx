import React from 'react';
import { logger } from '../utils/logger';

interface ErrorBoundaryProps {
    children: React.ReactNode;
}

interface ErrorBoundaryState {
    hasError: boolean;
    error: Error | null;
}

export class ErrorBoundary extends React.Component<ErrorBoundaryProps, ErrorBoundaryState> {
    constructor(props: ErrorBoundaryProps) {
        super(props);
        this.state = { hasError: false, error: null };
    }

    static getDerivedStateFromError(error: Error): ErrorBoundaryState {
        return { hasError: true, error };
    }

    componentDidCatch(error: Error, info: React.ErrorInfo): void {
        logger.error('React error boundary caught error', {
            error: error.message,
            stack: error.stack,
            componentStack: info.componentStack,
        });
    }

    render(): React.ReactNode {
        if (this.state.hasError) {
            return (
                <div style={{
                    display: 'flex',
                    flexDirection: 'column',
                    alignItems: 'center',
                    justifyContent: 'center',
                    height: '100vh',
                    gap: '1rem',
                    color: '#ff6b6b',
                }}>
                    <h2>Something went wrong</h2>
                    <p style={{ color: '#aaa', maxWidth: '600px', textAlign: 'center' }}>
                        {this.state.error?.message ?? 'An unexpected error occurred'}
                    </p>
                    <button
                        onClick={() => window.location.reload()}
                        style={{
                            padding: '0.5rem 1.5rem',
                            background: '#333',
                            color: '#fff',
                            border: '1px solid #555',
                            borderRadius: '4px',
                            cursor: 'pointer',
                        }}
                    >
                        Reload
                    </button>
                </div>
            );
        }

        return this.props.children;
    }
}
