#include "engine/systems/transform_system.hpp"
#include "components/transform.hpp"
#include "components/polygon_shape.hpp"
#include "components/static_tag.hpp"
#include <cmath>

namespace teleop {

void TransformSystem::tick(entt::registry& registry) {
    // Update world vertices for all non-static entities
    auto view = registry.view<const Transform, PolygonShape>(entt::exclude<StaticTag>);
    for (auto [entity, transform, shape] : view.each()) {
        float cosH = std::cos(transform.heading);
        float sinH = std::sin(transform.heading);

        for (size_t i = 0; i < shape.localVertices.size(); ++i) {
            const auto& local = shape.localVertices[i];
            shape.worldVertices[i] = {
                transform.x + local.x * cosH - local.y * sinH,
                transform.y + local.x * sinH + local.y * cosH,
            };
        }

        shape.aabb = AABB::fromVertices(shape.worldVertices.data(), shape.worldVertices.size());
    }
}

} // namespace teleop
