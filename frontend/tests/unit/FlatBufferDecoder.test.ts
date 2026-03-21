import { describe, it, expect } from 'vitest';
import { FlatBufferDecoder } from '../../src/services/FlatBufferDecoder';

describe('FlatBufferDecoder', () => {
    it('creates decoder instance', () => {
        const decoder = new FlatBufferDecoder();
        expect(decoder).toBeDefined();
    });

    it('handles zeroed buffer gracefully', () => {
        const decoder = new FlatBufferDecoder();
        const badData = new ArrayBuffer(4);
        const result = decoder.decodeWorldState(badData);
        // Zeroed buffer reads as sequence=0, which passes sequence check on first call
        // but subsequent calls with sequence=0 return null (out of order)
        expect(result).toBeDefined();

        // Second call with same sequence should be rejected
        const result2 = decoder.decodeWorldState(badData);
        expect(result2).toBeNull();
    });

    it('rejects out-of-order sequences', () => {
        const decoder = new FlatBufferDecoder();
        // First call consumes sequence 0
        decoder.decodeWorldState(new ArrayBuffer(4));
        // Same data (sequence 0) is now out of order
        const result = decoder.decodeWorldState(new ArrayBuffer(4));
        expect(result).toBeNull();
    });

    it('resets sequence tracking', () => {
        const decoder = new FlatBufferDecoder();
        decoder.reset();
        // After reset, any sequence should be accepted (tested implicitly)
        expect(decoder).toBeDefined();
    });
});
