#pragma once

#include <cstdint>

namespace teleop {

struct RenderMeta {
    uint32_t color = 0xFF0000FF;      // RGBA color for rendering
    uint8_t layer = 0;                // Render layer (0=ground, 1=obstacles, 2=robot)
};

} // namespace teleop
