import * as flatbuffers from 'flatbuffers';
import { EntityState, EntityType } from '../types/entities';
import { logger } from '../utils/logger';

// FlatBuffers generated types - manual decoder since we may not have generated code
// The binary format matches proto/teleop.fbs

export interface DecodedWorldState {
    sequence: number;
    timestampMs: bigint;
    tickNumber: bigint;
    entities: EntityState[];
    impactCount: number;
    robotSpeed: number;
    robotSteering: number;
}

export class FlatBufferDecoder {
    private lastProcessedSequence = -1;

    decodeWorldState(data: ArrayBuffer): DecodedWorldState | null {
        try {
            const buf = new flatbuffers.ByteBuffer(new Uint8Array(data));

            // Read root table offset
            const rootOffset = buf.readInt32(buf.position()) + buf.position();

            // Read fields from WorldState table
            // vtable is at rootOffset - buf.readInt32(rootOffset)
            const vtableOffset = rootOffset - buf.readInt32(rootOffset);
            const vtableSize = buf.readInt16(vtableOffset);

            const readField = (fieldIndex: number, defaultVal: number = 0): number => {
                const fieldOffset = (fieldIndex + 2) * 2; // +2 for vtable size and object size
                if (fieldOffset >= vtableSize) return defaultVal;
                const off = buf.readInt16(vtableOffset + fieldOffset);
                return off === 0 ? defaultVal : off;
            };

            // Field 0: sequence (uint32)
            const seqOff = readField(0);
            const sequence = seqOff ? buf.readUint32(rootOffset + seqOff) : 0;

            if (sequence <= this.lastProcessedSequence) {
                return null; // Out of order
            }
            this.lastProcessedSequence = sequence;

            // Field 1: timestamp_ms (uint64)
            const tsOff = readField(1);
            const timestampMs = tsOff ? buf.readUint64(rootOffset + tsOff) : BigInt(0);

            // Field 2: tick_number (uint64)
            const tnOff = readField(2);
            const tickNumber = tnOff ? buf.readUint64(rootOffset + tnOff) : BigInt(0);

            // Field 3: entities (vector of Entity tables)
            const entOff = readField(3);
            const entities: EntityState[] = [];
            if (entOff) {
                const vecOffset = rootOffset + entOff;
                const vectorStart = vecOffset + buf.readInt32(vecOffset);
                const vectorLen = buf.readInt32(vectorStart);

                for (let i = 0; i < vectorLen; i++) {
                    const entityRefOffset = vectorStart + 4 + i * 4;
                    const entityOffset = entityRefOffset + buf.readInt32(entityRefOffset);
                    const entity = this.decodeEntity(buf, entityOffset);
                    if (entity) entities.push(entity);
                }
            }

            // Field 4: impact_count (uint32)
            const icOff = readField(4);
            const impactCount = icOff ? buf.readUint32(rootOffset + icOff) : 0;

            // Field 5: robot_speed (float)
            const rsOff = readField(5);
            const robotSpeed = rsOff ? buf.readFloat32(rootOffset + rsOff) : 0;

            // Field 6: robot_steering (float)
            const rstOff = readField(6);
            const robotSteering = rstOff ? buf.readFloat32(rootOffset + rstOff) : 0;

            return {
                sequence,
                timestampMs,
                tickNumber,
                entities,
                impactCount,
                robotSpeed,
                robotSteering,
            };
        } catch (error) {
            logger.error('Failed to decode FlatBuffer frame', { error: String(error) });
            return null;
        }
    }

    private decodeEntity(buf: flatbuffers.ByteBuffer, offset: number): EntityState | null {
        try {
            const vtableOffset = offset - buf.readInt32(offset);
            const vtableSize = buf.readInt16(vtableOffset);

            const readField = (fieldIndex: number): number => {
                const fieldOffset = (fieldIndex + 2) * 2;
                if (fieldOffset >= vtableSize) return 0;
                const off = buf.readInt16(vtableOffset + fieldOffset);
                return off;
            };

            // Field 0: id
            const idOff = readField(0);
            const id = idOff ? buf.readUint32(offset + idOff) : 0;

            // Field 1: entity_type (byte)
            const typeOff = readField(1);
            const rawType = typeOff ? buf.readInt8(offset + typeOff) : 0;
            const type = rawType as EntityType;

            // Field 2: x
            const xOff = readField(2);
            const x = xOff ? buf.readFloat32(offset + xOff) : 0;

            // Field 3: y
            const yOff = readField(3);
            const y = yOff ? buf.readFloat32(offset + yOff) : 0;

            // Field 4: heading
            const hOff = readField(4);
            const heading = hOff ? buf.readFloat32(offset + hOff) : 0;

            // Field 5: vertices (vector of Vec2 structs, 8 bytes each)
            const vOff = readField(5);
            const vertices: { x: number; y: number }[] = [];
            if (vOff) {
                const vecRefOffset = offset + vOff;
                const vectorStart = vecRefOffset + buf.readInt32(vecRefOffset);
                const vectorLen = buf.readInt32(vectorStart);
                for (let i = 0; i < vectorLen; i++) {
                    const vx = buf.readFloat32(vectorStart + 4 + i * 8);
                    const vy = buf.readFloat32(vectorStart + 4 + i * 8 + 4);
                    vertices.push({ x: vx, y: vy });
                }
            }

            // Field 6: color
            const cOff = readField(6);
            const colorVal = cOff ? buf.readUint32(offset + cOff) : 0xFFFFFFFF;
            const color = '#' + colorVal.toString(16).padStart(8, '0').slice(0, 6);

            return { id, type, x, y, heading, vertices, color };
        } catch {
            return null;
        }
    }

    reset(): void {
        this.lastProcessedSequence = -1;
    }
}
