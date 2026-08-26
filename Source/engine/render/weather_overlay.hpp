#pragma once

#include "engine/surface.hpp"

namespace devilution {

/**
 * @brief Render screen-space rain and atmospheric effect in Tristram (Town).
 * @param out Target surface to render on.
 */
void RenderWeatherRain(const Surface &out);

/**
 * @brief Reset the rain particle pool state.
 */
void ResetWeatherRain();

} // namespace devilution
