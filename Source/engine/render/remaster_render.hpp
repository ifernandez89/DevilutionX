/**
 * @file remaster_render.hpp
 *
 * Subtle Graphics Remaster GPU Post-Processing Pipeline.
 */
#pragma once

#include <cstdint>

#ifdef USE_SDL3
#include <SDL3/SDL_render.h>
#else
#include <SDL.h>
#endif

namespace devilution {

/**
 * @brief Initialize GPU post-processing shaders and framebuffers.
 */
void RemasterInit(int width, int height);

/**
 * @brief Free GPU post-processing resources.
 */
void RemasterCleanup();

/**
 * @brief Process game texture through GPU post-processing passes (Color Grade, Bloom, Vignette, Sharpen) and present to screen.
 * @return True if GPU remaster presentation succeeded, false if fallback to standard SDL_RenderCopy is required.
 */
bool RemasterProcessAndPresent(SDL_Renderer *renderer, SDL_Texture *sourceTexture, int screenWidth, int screenHeight);

/**
 * @brief Check if Remaster GPU pipeline is active.
 */
bool RemasterIsEnabled();

} // namespace devilution
