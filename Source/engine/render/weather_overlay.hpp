#pragma once

#include "engine/surface.hpp"

namespace devilution {

/**
 * @brief Legacy software weather rendering (disabled in favor of Neural Render & Web Weather Engine).
 */
inline void RenderWeatherRain(const Surface & /*out*/) { }
inline void ResetWeatherRain() { }
inline void RenderWeatherFog(const Surface & /*out*/) { }
inline void ResetWeatherFog() { }

} // namespace devilution
