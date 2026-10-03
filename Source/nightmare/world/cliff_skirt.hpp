#pragma once

#include "engine/point.hpp"
#include "engine/surface.hpp"

namespace devilution {
namespace nightmare {

/**
 * @brief Renders vertical rocky cliff faces (cliff skirts) beneath exposed dungeon/town floor edges.
 * 
 * When a floor tile borders the void/abyss (e.g. on floating islands or void edges),
 * this draws a textured, shaded rock wall extending downward into darkness.
 *
 * @param out Output framebuffer surface
 * @param tilePosition The (x, y) coordinates on the dPiece grid
 * @param targetBufferPosition Top-left pixel coordinates of the floor diamond
 * @param lightTableIndex Current lighting index (0 = bright, 15 = dark)
 */
void RenderCliffSkirts(const Surface &out, Point tilePosition, Point targetBufferPosition, int lightTableIndex);

} // namespace nightmare
} // namespace devilution
