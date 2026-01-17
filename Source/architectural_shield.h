#pragma once

#include <string>
#include <vector>

namespace devilution {

/**
 * @brief Architectural Integrity Shield - Permanent Protection System
 * 
 * This system doesn't block corrupted saves - instead it creates an
 * IMPENETRABLE SHIELD around our architecture that sanitizes ALL incoming
 * data in real-time, ensuring our game engine NEVER receives corrupted data.
 * 
 * Philosophy: "Trust nothing, sanitize everything, protect forever"
 */

/**
 * @brief Sanitizes player coordinates in real-time
 * 
 * Creates a protective layer that intercepts and fixes ANY coordinate
 * before it reaches our game systems
 * 
 * @param x Reference to X coordinate - will be sanitized if corrupted
 * @param y Reference to Y coordinate - will be sanitized if corrupted
 * @return true if coordinates were sanitized, false if they were already clean
 */
bool SanitizeCoordinates(int& x, int& y);

/**
 * @brief Sanitizes color palette data in real-time
 * 
 * Intercepts palette data and ensures it's always valid, fixing the
 * red color corruption bug permanently
 * 
 * @param paletteData Pointer to palette data to sanitize
 * @param dataSize Size of palette data
 */
void SanitizePaletteData(void* paletteData, size_t dataSize);

/**
 * @brief Architectural Data Interceptor
 * 
 * Intercepts ALL data flowing into our systems and sanitizes it
 * This creates an impenetrable shield around our architecture
 * 
 * @param data Pointer to any data entering our systems
 * @param dataType Type identifier for the data
 * @param dataSize Size of the data
 */
void InterceptAndSanitizeData(void* data, const std::string& dataType, size_t dataSize);

/**
 * @brief Permanent Architectural Protection System
 * 
 * Creates hooks and interceptors that protect our architecture
 * from ANY corrupted data, regardless of source
 */
void InitializeArchitecturalShield();

} // namespace devilution