#pragma once

#include "engine/surface.hpp"

namespace devilution {

/**
 * @brief Render screen-space gothic fog/mist overlay effect in Tristram (Town).
 * @param out Target surface to render on.
 */
void RenderWeatherFog(const Surface &out);

/**
 * @brief Reset the fog particle and wisp pool state.
 */
void ResetWeatherFog();

// Compatibility aliases
inline void RenderWeatherRain(const Surface &out) { RenderWeatherFog(out); }
inline void ResetWeatherRain() { ResetWeatherFog(); }

} // namespace devilution
