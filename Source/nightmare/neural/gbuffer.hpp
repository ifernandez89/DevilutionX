#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <SDL.h>

#include "engine/point.hpp"
#include "engine/surface.hpp"

namespace devilution {
namespace nightmare {
namespace neural {

/**
 * @brief Dungeon Biomes across Diablo 1 and NIGHTMARE Expansion
 */
enum class DungeonBiome : uint8_t {
	Town = 0,       // Tristram
	Cathedral = 1,  // Levels 1-4
	Catacombs = 2,  // Levels 5-8
	Caves = 3,      // Levels 9-12
	Hell = 4,       // Levels 13-16
	Crypt = 5,      // Levels 17-20
	Hive = 6,       // Levels 21-24
};

/**
 * @brief Semantic class IDs for scene decomposition
 */
enum class SemanticId : uint8_t {
	Void = 0,        // Sky / Out of bounds
	Floor = 1,       // Grass, dirt paths, dungeon tiles, lava rock
	Wall = 2,        // House walls, cathedral ruins, dungeon pillars, arches
	Player = 3,      // Hero character
	NPC = 4,         // Towners (Cain, Griswold, Pepin, Tremain)
	Water = 5,       // River, lava pool, blood pool, acid
	Interactive = 6, // Bonfire, torches, chests, shrines, barrels
	Monster = 7,     // Enemies / dungeon bosses (Butcher, Fleshdoom, Diablo)
	Missile = 8,     // Magic spells, firebolts, lightning
	Door = 9,        // Wooden/iron doors and gates
};

/**
 * @brief Manages G-Buffer capture (RGB, Logical Depth, Lightmap, Semantic ID)
 */
class GBufferManager {
public:
	static GBufferManager &Instance();

	void Initialize(int width = 640, int height = 480);
	void BeginFrame(int width, int height);
	void EndFrame();

	bool IsEnabled() const { return enabled_; }
	void SetEnabled(bool enable) { enabled_ = enable; }

	void SetCurrentBiome(DungeonBiome biome) { currentBiome_ = biome; }
	DungeonBiome GetCurrentBiome() const { return currentBiome_; }

	// Active context setters during rendering passes
	void SetCurrentSemantic(SemanticId semantic) { currentSemantic_ = semantic; }
	SemanticId GetCurrentSemantic() const { return currentSemantic_; }

	void SetCurrentDepth(float depth) { currentDepth_ = depth; }
	float GetCurrentDepth() const { return currentDepth_; }

	// Write / Record helpers
	void WritePixel(int x, int y, uint32_t rgba, float depth, uint8_t light, SemanticId semantic);
	void StampSemanticRect(int x, int y, int w, int h, SemanticId semantic, float depth, uint8_t light);
	void StampDiamondTile(Point screenPos, SemanticId semantic, float depth, uint8_t light);
	
	void CaptureSceneSurface(const Surface &out, const SDL_Color *palette);
	void CaptureLightValue(int x, int y, uint8_t lightVal);

	// Buffer accessors (for Native inspection / WebAssembly exports)
	int GetWidth() const { return width_; }
	int GetHeight() const { return height_; }
	const uint32_t *GetRGBData() const { return rgbBuffer_.data(); }
	const float *GetDepthData() const { return depthBuffer_.data(); }
	const uint8_t *GetLightData() const { return lightBuffer_.data(); }
	const uint8_t *GetSemanticData() const { return semanticBuffer_.data(); }

	size_t GetRGBByteSize() const { return rgbBuffer_.size() * sizeof(uint32_t); }
	size_t GetDepthByteSize() const { return depthBuffer_.size() * sizeof(float); }
	size_t GetLightByteSize() const { return lightBuffer_.size() * sizeof(uint8_t); }
	size_t GetSemanticByteSize() const { return semanticBuffer_.size() * sizeof(uint8_t); }

private:
	GBufferManager();

	bool enabled_ = true;
	int width_ = 640;
	int height_ = 480;

	DungeonBiome currentBiome_ = static_cast<DungeonBiome>(255);
	SemanticId currentSemantic_ = SemanticId::Floor;
	float currentDepth_ = 0.5f;

	std::vector<uint32_t> rgbBuffer_;      // RGBA8 32-bit
	std::vector<float> depthBuffer_;       // Logical Depth 0.0 - 1.0
	std::vector<uint8_t> lightBuffer_;     // Light table / luminance 0 - 255
	std::vector<uint8_t> semanticBuffer_;  // Semantic class 0 - 9
};

} // namespace neural
} // namespace nightmare
} // namespace devilution

// C-Export API for WebAssembly / Emscripten & WebGPU Harness integration
extern "C" {
	void Nightmare_GBuffer_SetEnabled(int enabled);
	int Nightmare_GBuffer_IsEnabled();
	int Nightmare_GBuffer_GetWidth();
	int Nightmare_GBuffer_GetHeight();
	int Nightmare_GBuffer_GetBiome();
	const uint32_t* Nightmare_GBuffer_GetRGB();
	const float* Nightmare_GBuffer_GetDepth();
	const uint8_t* Nightmare_GBuffer_GetLight();
	const uint8_t* Nightmare_GBuffer_GetSemantic();
}
