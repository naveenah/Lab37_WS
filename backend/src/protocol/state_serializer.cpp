#include "protocol/state_serializer.hpp"
#include "generated/teleop_generated.h"
#include <chrono>

namespace teleop {

std::pair<const uint8_t*, size_t> StateSerializer::serialize(
    const std::vector<EntitySnapshot>& entities,
    const ScoreState& score,
    uint32_t sequence,
    uint64_t tickNumber,
    float robotSpeed,
    float robotSteering
) {
    builder_.Clear();

    std::vector<flatbuffers::Offset<Teleop::Entity>> entityOffsets;
    entityOffsets.reserve(entities.size());

    for (const auto& entity : entities) {
        // Convert vertices to FlatBuffers Vec2 structs
        std::vector<Teleop::Vec2> fbVerts;
        fbVerts.reserve(entity.vertices.size());
        for (const auto& v : entity.vertices) {
            fbVerts.emplace_back(v.x, v.y);
        }
        auto vertsVec = builder_.CreateVectorOfStructs(fbVerts);

        auto fbEntity = Teleop::CreateEntity(
            builder_,
            entity.entityId,
            static_cast<Teleop::EntityType>(entity.entityType),
            entity.x,
            entity.y,
            entity.heading,
            vertsVec,
            entity.color
        );
        entityOffsets.push_back(fbEntity);
    }

    auto entitiesVec = builder_.CreateVector(entityOffsets);

    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    auto worldState = Teleop::CreateWorldState(
        builder_,
        sequence,
        static_cast<uint64_t>(now),
        tickNumber,
        entitiesVec,
        score.impactCount,
        robotSpeed,
        robotSteering
    );

    builder_.Finish(worldState);

    return {builder_.GetBufferPointer(), builder_.GetSize()};
}

} // namespace teleop
