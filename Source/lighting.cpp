/**
 * @file lighting.cpp
 *
 * Implementation of light and vision.
 */
#include "lighting.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <string>

#include <expected.hpp>

#include "automap.h"
#include "engine/displacement.hpp"
#include "engine/lighting_defs.hpp"
#include "engine/load_file.hpp"
#include "engine/point.hpp"
#include "engine/points_in_rectangle_range.hpp"
#include "engine/world_tile.hpp"
#include "levels/tile_properties.hpp"
#include "objects.h"
#include "player.h"
#include "utils/attributes.h"
#include "utils/is_of.hpp"
#include "utils/status_macros.hpp"
#include "vision.hpp"

namespace devilution {

std::array<bool, MAXVISION> VisionActive;
Light VisionList[MAXVISION];
Light Lights[MAXLIGHTS];
std::array<uint8_t, MAXLIGHTS> ActiveLights;
int ActiveLightCount;
std::array<std::array<uint8_t, LightTableSize>, NumLightingLevels> LightTables;
uint8_t *FullyLitLightTable = nullptr;
uint8_t *FullyDarkLightTable = nullptr;
std::array<uint8_t, 256> InfravisionTable;
std::array<uint8_t, 256> StoneTable;
std::array<uint8_t, 256> PauseTable;
#ifdef _DEBUG
bool DisableLighting;
#endif
bool UpdateLighting;

// 🎯 FASE D1 - 3D SIMULADO: Configuración global de depth cues
DepthCuesConfig g_depthCues;

namespace {

/** @brief Number of supported light radiuses (first radius starts with 0) */
constexpr size_t NumLightRadiuses = 16;
/** Falloff tables for the light cone */
uint8_t LightFalloffs[NumLightRadiuses][128];
bool UpdateVision;
/** interpolations of a 32x32 (16x16 mirrored) light circle moving between tiles in steps of 1/8 of a tile */
uint8_t LightConeInterpolations[8][8][16][16];

// 🎥 FASE V1.4 - FAKE VIGNETTE SYSTEM
/** @brief Vignette intensity table for peripheral vision darkening */
float VignetteTable[MAXDUNX][MAXDUNY];
bool VignetteInitialized = false;

/**
 * 🎯 FASE V1.4 - Initialize Fake Vignette System
 * Crea una tabla de intensidad de viñeta para oscurecer la visión periférica
 * Simula el efecto de túnel visual que ocurre naturalmente en ambientes oscuros
 */
void InitializeFakeVignette() {
	if (VignetteInitialized) return;
	
	// Calcular centro de la pantalla/mapa visible
	const float centerX = MAXDUNX / 2.0f;
	const float centerY = MAXDUNY / 2.0f;
	
	// Radio máximo desde el centro hasta las esquinas
	const float maxRadius = std::sqrt(centerX * centerX + centerY * centerY);
	
	// 🎯 Intensidad de viñeta por tipo de nivel
	float vignetteIntensity = 0.3f; // Base intensity
	
	if (leveltype == DTYPE_TOWN) {
		vignetteIntensity = 0.15f; // Viñeta sutil en town
	} else if (leveltype == DTYPE_CATACOMBS) {
		vignetteIntensity = 0.45f; // Viñeta intensa, atmósfera claustrofóbica
	} else if (leveltype == DTYPE_CAVES) {
		vignetteIntensity = 0.40f; // Viñeta media-alta, sensación opresiva
	} else if (leveltype == DTYPE_HELL) {
		vignetteIntensity = 0.50f; // Viñeta máxima, dramática y violenta
	} else {
		vignetteIntensity = 0.30f; // Cathedral: viñeta clásica
	}
	
	// Generar tabla de viñeta
	for (int x = 0; x < MAXDUNX; x++) {
		for (int y = 0; y < MAXDUNY; y++) {
			// Distancia desde el centro
			const float dx = x - centerX;
			const float dy = y - centerY;
			const float distance = std::sqrt(dx * dx + dy * dy);
			
			// Factor de distancia normalizado (0.0 = centro, 1.0 = borde)
			const float distanceFactor = std::min(distance / maxRadius, 1.0f);
			
			// Aplicar curva suave para transición natural
			// Usar función cuadrática suave para evitar bordes duros
			const float smoothFactor = distanceFactor * distanceFactor;
			
			// Calcular intensidad de viñeta (0.0 = sin efecto, 1.0 = máximo oscurecimiento)
			VignetteTable[x][y] = smoothFactor * vignetteIntensity;
		}
	}
	
	VignetteInitialized = true;
}

/**
 * 🎯 FASE V1.4 - Apply Fake Vignette Effect
 * Aplica el efecto de viñeta a un valor de luz específico
 * 
 * @param position Posición en el mapa
 * @param lightValue Valor de luz original (0-15)
 * @return Valor de luz con viñeta aplicada
 */
uint8_t ApplyFakeVignette(Point position, uint8_t lightValue) {
	if (!VignetteInitialized) {
		InitializeFakeVignette();
	}
	
	// Verificar bounds
	if (position.x < 0 || position.x >= MAXDUNX || position.y < 0 || position.y >= MAXDUNY) {
		return lightValue;
	}
	
	// Obtener intensidad de viñeta para esta posición
	const float vignetteIntensity = VignetteTable[position.x][position.y];
	
	// Aplicar viñeta al valor de luz
	// Viñeta oscurece, así que aumenta el valor de luz (más alto = más oscuro)
	const float vignetteEffect = vignetteIntensity * 6.0f; // Máximo 6 niveles de oscurecimiento
	const float newLightValue = lightValue + vignetteEffect;
	
	// Clamp al rango válido
	return static_cast<uint8_t>(std::min(newLightValue, 15.0f));
}

// ============================================================================
// 🎯 FASE D1 - 3D SIMULADO: DEPTH CUES IMPLEMENTATION
// ============================================================================

/**
 * D1.1 + D1.2 - Calcula factor de profundidad basado en distancia y bias vertical
 * 
 * Implementa dos efectos clave:
 * - D1.1: Oscurecimiento por distancia al jugador (depth cues)
 * - D1.2: Bias vertical para reforzar perspectiva isométrica
 * 
 * @param x, y Coordenadas del tile a evaluar
 * @param playerX, playerY Coordenadas del jugador
 * @return Factor de profundidad (0.3-1.0, donde 1.0 = sin efecto, 0.3 = máximo oscurecimiento)
 */
float CalculateDepthCues(int x, int y, int playerX, int playerY) {
	if (!g_depthCues.enabled) {
		return 1.0f; // Sin efecto si está deshabilitado
	}
	
	// D1.1 - Calcular distancia Manhattan al jugador (más eficiente que euclidiana)
	const float dx = static_cast<float>(x - playerX);
	const float dy = static_cast<float>(y - playerY);
	const float distance = std::sqrt(dx * dx + dy * dy);
	
	// D1.2 - Bias vertical para reforzar perspectiva isométrica
	// "Hacia arriba" del mapa (Y menor) se ve más lejano
	const float verticalOffset = (y - playerY) * g_depthCues.verticalBias;
	
	// Combinar distancia real + bias vertical
	const float totalDistance = distance + verticalOffset;
	
	// Calcular factor de profundidad (1.0 = cerca del jugador, 0.0 = muy lejos)
	float depthFactor = 1.0f - (totalDistance / g_depthCues.maxDistance);
	
	// Clamp al rango válido (nunca completamente negro)
	return std::max(g_depthCues.minIntensity, std::min(1.0f, depthFactor));
}

/**
 * Aplica depth cues a un nivel de luz específico
 * 
 * @param lightLevel Nivel de luz original (0-15, donde 0=brillante, 15=oscuro)
 * @param x, y Coordenadas del tile
 * @param playerX, playerY Coordenadas del jugador
 * @return Nivel de luz modificado con depth cues aplicados
 */
uint8_t ApplyDepthCuesToLight(uint8_t lightLevel, int x, int y, int playerX, int playerY) {
	if (!g_depthCues.enabled) {
		return lightLevel;
	}
	
	// Calcular factor de profundidad
	const float depthFactor = CalculateDepthCues(x, y, playerX, playerY);
	
	// Aplicar depth cues al nivel de luz
	// depthFactor bajo = más lejos = más oscuro = lightLevel más alto
	const float adjustedLight = lightLevel / depthFactor;
	
	// Clamp al rango válido de lighting (0-15)
	return static_cast<uint8_t>(std::min(15.0f, adjustedLight));
}

} // namespace

/**
 * 🎯 FASE D1 - DEPTH CUES SYSTEM IMPLEMENTATION
 * Inicializa el sistema de depth cues con valores por defecto
 */
void InitDepthCues() {
	// Configuración por defecto ya está en la struct
	// Ajustar parámetros según el tipo de nivel para mejor efecto
	
	if (leveltype == DTYPE_TOWN) {
		g_depthCues.depthFactor = 12.0f;     // Efecto más sutil en town
		g_depthCues.verticalBias = 0.2f;     // Menos bias vertical
		g_depthCues.maxDistance = 25.0f;     // Mayor alcance
	} else if (leveltype == DTYPE_CATACOMBS) {
		g_depthCues.depthFactor = 6.0f;      // Efecto más intenso, claustrofóbico
		g_depthCues.verticalBias = 0.4f;     // Más bias vertical
		g_depthCues.maxDistance = 15.0f;     // Menor alcance, más dramático
	} else if (leveltype == DTYPE_CAVES) {
		g_depthCues.depthFactor = 7.0f;      // Efecto medio-alto
		g_depthCues.verticalBias = 0.35f;    // Bias medio-alto
		g_depthCues.maxDistance = 18.0f;     // Alcance medio
	} else if (leveltype == DTYPE_HELL) {
		g_depthCues.depthFactor = 5.0f;      // Efecto máximo, atmósfera opresiva
		g_depthCues.verticalBias = 0.5f;     // Máximo bias vertical
		g_depthCues.maxDistance = 12.0f;     // Menor alcance, máximo drama
	} else {
		// Cathedral - valores por defecto
		g_depthCues.depthFactor = 8.0f;
		g_depthCues.verticalBias = 0.3f;
		g_depthCues.maxDistance = 20.0f;
	}
}

/**
 * Habilita/deshabilita el sistema de depth cues
 */
void SetDepthCuesEnabled(bool enabled) {
	g_depthCues.enabled = enabled;
}

namespace {

void RotateRadius(DisplacementOf<int8_t> &offset, DisplacementOf<int8_t> &dist, DisplacementOf<int8_t> &light, DisplacementOf<int8_t> &block)
{
	dist = { static_cast<int8_t>(7 - dist.deltaY), dist.deltaX };
	light = { static_cast<int8_t>(7 - light.deltaY), light.deltaX };
	offset = { static_cast<int8_t>(dist.deltaX - light.deltaX), static_cast<int8_t>(dist.deltaY - light.deltaY) };

	block.deltaX = 0;
	if (offset.deltaX < 0) {
		offset.deltaX += 8;
		block.deltaX = 1;
	}
	block.deltaY = 0;
	if (offset.deltaY < 0) {
		offset.deltaY += 8;
		block.deltaY = 1;
	}
}

DVL_ALWAYS_INLINE void SetLight(Point position, uint8_t v)
{
	// 🎥 FASE V1.4 - Apply Fake Vignette Effect
	// Aplicar viñeta antes de establecer el valor de luz
	v = ApplyFakeVignette(position, v);
	
	if (LoadingMapObjects)
		dPreLight[position.x][position.y] = v;
	else
		dLight[position.x][position.y] = v;
}

DVL_ALWAYS_INLINE uint8_t GetLight(Point position)
{
	if (LoadingMapObjects)
		return dPreLight[position.x][position.y];

	return dLight[position.x][position.y];
}

bool TileAllowsLight(Point position)
{
	if (!InDungeonBounds(position))
		return false;
	return !TileHasAny(position, TileProperties::BlockLight);
}

void DoVisionFlags(Point position, MapExplorationType doAutomap, bool visible)
{
	if (doAutomap != MAP_EXP_NONE) {
		if (dFlags[position.x][position.y] != DungeonFlag::None)
			SetAutomapView(position, doAutomap);
		dFlags[position.x][position.y] |= DungeonFlag::Explored;
	}
	if (visible)
		dFlags[position.x][position.y] |= DungeonFlag::Lit;
	dFlags[position.x][position.y] |= DungeonFlag::Visible;
}

} // namespace

void DoUnLight(Point position, uint8_t radius)
{
	radius++;
	radius++; // If lights moved at a diagonal it can result in some extra tiles being lit

	auto searchArea = PointsInRectangle(WorldTileRectangle { position, radius });

	for (const WorldTilePosition targetPosition : searchArea) {
		if (InDungeonBounds(targetPosition))
			dLight[targetPosition.x][targetPosition.y] = dPreLight[targetPosition.x][targetPosition.y];
	}
}

void DoLighting(Point position, uint8_t radius, DisplacementOf<int8_t> offset)
{
	assert(radius >= 0 && radius <= NumLightRadiuses);
	assert(InDungeonBounds(position));

	DisplacementOf<int8_t> light = {};
	DisplacementOf<int8_t> block = {};

	if (offset.deltaX < 0) {
		offset.deltaX += 8;
		position -= { 1, 0 };
	}
	if (offset.deltaY < 0) {
		offset.deltaY += 8;
		position -= { 0, 1 };
	}

	DisplacementOf<int8_t> dist = offset;

	int minX = 15;
	if (position.x - 15 < 0) {
		minX = position.x + 1;
	}
	int maxX = 15;
	if (position.x + 15 > MAXDUNX) {
		maxX = MAXDUNX - position.x;
	}
	int minY = 15;
	if (position.y - 15 < 0) {
		minY = position.y + 1;
	}
	int maxY = 15;
	if (position.y + 15 > MAXDUNY) {
		maxY = MAXDUNY - position.y;
	}

	// Allow for dim lights in crypt and nest
	if (IsAnyOf(leveltype, DTYPE_NEST, DTYPE_CRYPT)) {
		if (GetLight(position) > LightFalloffs[radius][0])
			SetLight(position, LightFalloffs[radius][0]);
	} else {
		SetLight(position, 0);
	}

	for (int i = 0; i < 4; i++) {
		const int yBound = i > 0 && i < 3 ? maxY : minY;
		const int xBound = i < 2 ? maxX : minX;
		for (int y = 0; y < yBound; y++) {
			for (int x = 1; x < xBound; x++) {
				const int linearDistance = LightConeInterpolations[offset.deltaX][offset.deltaY][x + block.deltaX][y + block.deltaY];
				if (linearDistance >= 128)
					continue;
				const Point temp = position + (Displacement { x, y }).Rotate(-i);
				const uint8_t v = LightFalloffs[radius][linearDistance];
				if (!InDungeonBounds(temp))
					continue;
				if (v < GetLight(temp))
					SetLight(temp, v);
			}
		}
		RotateRadius(offset, dist, light, block);
	}
}

void DoUnVision(Point position, uint8_t radius)
{
	radius++;
	radius++; // increasing the radius even further here prevents leaving stray vision tiles behind and doesn't seem to affect monster AI - applying new vision happens in the same tick

	auto searchArea = PointsInRectangle(WorldTileRectangle { position, radius });

	for (const WorldTilePosition targetPosition : searchArea) {
		if (InDungeonBounds(targetPosition))
			dFlags[targetPosition.x][targetPosition.y] &= ~(DungeonFlag::Visible | DungeonFlag::Lit);
	}
}

void DoVision(Point position, uint8_t radius, MapExplorationType doAutomap, bool visible)
{
	auto markVisibleFn = [doAutomap, visible](Point rayPoint) {
		DoVisionFlags(rayPoint, doAutomap, visible);
	};
	auto markTransparentFn = [](Point rayPoint) {
		const int8_t trans = dTransVal[rayPoint.x][rayPoint.y];
		if (trans != 0)
			TransList[trans] = true;
	};
	auto passesLightFn = [](Point rayPoint) {
		return TileAllowsLight(rayPoint);
	};
	auto inBoundsFn = [](Point rayPoint) {
		return InDungeonBounds(rayPoint);
	};

	DoVision(position, radius, markVisibleFn, markTransparentFn, passesLightFn, inBoundsFn);
}

tl::expected<void, std::string> LoadTrns()
{
	RETURN_IF_ERROR(LoadFileInMemWithStatus("plrgfx\\infra.trn", InfravisionTable));
	RETURN_IF_ERROR(LoadFileInMemWithStatus("plrgfx\\stone.trn", StoneTable));
	return LoadFileInMemWithStatus("gendata\\pause.trn", PauseTable);
}

void MakeLightTable()
{
	// Generate 16 gradually darker translation tables for doing lighting
	uint8_t shade = 0;
	constexpr uint8_t Black = 0;
	constexpr uint8_t White = 255;
	for (auto &lightTable : LightTables) {
		uint8_t colorIndex = 0;
		for (const uint8_t steps : { 16, 16, 16, 16, 16, 16, 16, 16, 8, 8, 8, 8, 16, 16, 16, 16, 16, 16 }) {
			const uint8_t shading = shade * steps / 16;
			const uint8_t shadeStart = colorIndex;
			const uint8_t shadeEnd = shadeStart + steps - 1;
			for (uint8_t step = 0; step < steps; step++) {
				if (colorIndex == Black) {
					lightTable[colorIndex++] = Black;
					continue;
				}
				int color = shadeStart + step + shading;
				if (color > shadeEnd || color == White)
					color = Black;
				lightTable[colorIndex++] = color;
			}
		}
		shade++;
	}

	LightTables[15] = {}; // Make last shade pitch black
	FullyLitLightTable = LightTables[0].data();
	FullyDarkLightTable = LightTables[LightsMax].data();

	if (leveltype == DTYPE_HELL) {
		// Blood wall lighting
		const auto shades = static_cast<int>(LightTables.size() - 1);
		for (int i = 0; i < shades; i++) {
			auto &lightTable = LightTables[i];
			constexpr int Range = 16;
			for (int j = 0; j < Range; j++) {
				uint8_t color = ((Range - 1) << 4) / shades * (shades - i) / Range * (j + 1);
				color = 1 + (color >> 4);
				int idx = j + 1;
				lightTable[idx] = color;
				idx = 31 - j;
				lightTable[idx] = color;
			}
		}
		FullyLitLightTable = nullptr; // A color map is used for the ceiling animation, so even fully lit tiles have a color map
	} else if (IsAnyOf(leveltype, DTYPE_NEST, DTYPE_CRYPT)) {
		// Make the lava fully bright
		for (auto &lightTable : LightTables)
			std::iota(lightTable.begin(), lightTable.begin() + 16, uint8_t { 0 });
		LightTables[15][0] = 0;
		std::fill_n(LightTables[15].begin() + 1, 15, 1);
		FullyDarkLightTable = nullptr; // Tiles in Hellfire levels are never completely black
	}

	// Verify that fully lit and fully dark light table optimizations are correctly enabled/disabled (nullptr = disabled)
	assert((FullyLitLightTable != nullptr) == (LightTables[0][0] == 0 && std::adjacent_find(LightTables[0].begin(), LightTables[0].end() - 1, [](auto x, auto y) { return (x + 1) != y; }) == LightTables[0].end() - 1));
	assert((FullyDarkLightTable != nullptr) == (std::all_of(LightTables[LightsMax].begin(), LightTables[LightsMax].end(), [](auto x) { return x == 0; })));

	// 🎥 FASE V1 - INTELLIGENT LIGHTING SYSTEM 🎥
	// Generate light falloffs ranges with enhanced intelligent lighting
	const float maxDarkness = 15;
	const float maxBrightness = 0;
	
	// FEATURE: Global Dark Atmosphere Enhancement - Darker ambient lighting
	float atmosphereMultiplier = 1.0f;
	
	// 🎯 FASE V1.1 - ADAPTIVE CONTRAST BY LEVEL
	// Contraste adaptativo que se intensifica con la profundidad del dungeon
	float adaptiveContrast = 1.0f;
	
	// 🎯 FASE V1.2 - CONTEXTUAL ATMOSPHERE ENHANCEMENT
	// Variaciones sutiles por tipo de nivel que el jugador siente pero no nota conscientemente
	if (leveltype == DTYPE_TOWN) {
		atmosphereMultiplier = 1.15f; // Town: 15% más oscuro, sensación deteriorada
		adaptiveContrast = 0.9f;      // Contraste suave en town
	} else if (leveltype == DTYPE_CATACOMBS) {
		atmosphereMultiplier = 1.30f; // Catacombs: 30% más oscuro, atmósfera más fría y siniestra
		adaptiveContrast = 1.2f;      // Contraste medio-alto, atmósfera tensa
	} else if (leveltype == DTYPE_CAVES) {
		atmosphereMultiplier = 1.25f; // Caves: 25% más oscuro, sensación opresiva
		adaptiveContrast = 1.1f;      // Contraste medio, sensación claustrofóbica
	} else if (leveltype == DTYPE_HELL) {
		atmosphereMultiplier = 1.40f; // Hell: 40% más oscuro, contraste violento y dramático
		adaptiveContrast = 1.5f;      // Contraste máximo, dramático y violento
	} else {
		atmosphereMultiplier = 1.20f; // Cathedral: 20% más oscuro, atmósfera gótica
		adaptiveContrast = 1.0f;      // Contraste base, atmósfera clásica
	}
	
	for (unsigned radius = 0; radius < NumLightRadiuses; radius++) {
		const unsigned maxDistance = (radius + 1) * 8;
		for (unsigned distance = 0; distance < 128; distance++) {
			if (distance > maxDistance) {
				LightFalloffs[radius][distance] = 15;
			} else {
				const float factor = static_cast<float>(distance) / static_cast<float>(maxDistance);
				float scaled;
				if (IsAnyOf(leveltype, DTYPE_NEST, DTYPE_CRYPT)) {
					// 🎯 FASE V1.3 - QUADRATIC FALLOFF for Hellfire levels with over exposure
					const float brightness = static_cast<float>(radius) * 1.25F;
					// Quadratic falloff with enhanced contrast
					scaled = factor * factor * brightness * adaptiveContrast + (maxDarkness - brightness);
					scaled = std::max(maxBrightness, scaled);
				} else {
					// 🎯 FASE V1.3 - ENHANCED QUADRATIC FALLOFF for all other levels
					// Combinar falloff cuadrático con contraste adaptativo
					float quadraticFactor = factor * factor; // Falloff cuadrático base
					
					// Aplicar contraste adaptativo de forma inteligente
					if (adaptiveContrast > 1.0f) {
						// Para contraste alto, intensificar las sombras
						quadraticFactor = std::pow(quadraticFactor, 1.0f / adaptiveContrast);
					} else if (adaptiveContrast < 1.0f) {
						// Para contraste bajo, suavizar las transiciones
						quadraticFactor = std::pow(quadraticFactor, adaptiveContrast);
					}
					
					// Aplicar multiplicador de atmósfera con falloff cuadrático mejorado
					scaled = quadraticFactor * maxDarkness * atmosphereMultiplier;
				}
				scaled += 0.5F; // Round up
				// Clamp to valid light table range
				scaled = std::min(scaled, maxDarkness);
				LightFalloffs[radius][distance] = static_cast<uint8_t>(scaled);
			}
		}
	}

	// Generate the light cone interpolations
	for (int offsetY = 0; offsetY < 8; offsetY++) {
		for (int offsetX = 0; offsetX < 8; offsetX++) {
			for (int y = 0; y < 16; y++) {
				for (int x = 0; x < 16; x++) {
					const int a = (8 * x - offsetX);
					const int b = (8 * y - offsetY);
					LightConeInterpolations[offsetX][offsetY][x][y] = static_cast<uint8_t>(sqrt(a * a + b * b));
				}
			}
		}
	}
}

#ifdef _DEBUG
void ToggleLighting()
{
	DisableLighting = !DisableLighting;

	if (DisableLighting) {
		memset(dLight, 0, sizeof(dLight));
		return;
	}

	memcpy(dLight, dPreLight, sizeof(dLight));
	for (const Player &player : Players) {
		if (player.plractive && player.isOnActiveLevel()) {
			DoLighting(player.position.tile, player._pLightRad, {});
		}
	}
}
#endif

void InitLighting()
{
	ActiveLightCount = 0;
	UpdateLighting = false;
	UpdateVision = false;
#ifdef _DEBUG
	DisableLighting = false;
#endif

	std::iota(ActiveLights.begin(), ActiveLights.end(), uint8_t { 0 });
	VisionActive = {};
	TransList = {};
	
	// 🎥 FASE V1.4 - Initialize Fake Vignette System
	VignetteInitialized = false; // Force re-initialization for new level
	InitializeFakeVignette();
	
	// 🎯 FASE D1 - Initialize Depth Cues System
	InitDepthCues();
}

int AddLight(Point position, uint8_t radius)
{
#ifdef _DEBUG
	if (DisableLighting)
		return NO_LIGHT;
#endif
	if (ActiveLightCount >= MAXLIGHTS)
		return NO_LIGHT;

	const int lid = ActiveLights[ActiveLightCount++];
	Light &light = Lights[lid];
	light.position.tile = position;
	light.radius = radius;
	light.position.offset = { 0, 0 };
	light.isInvalid = false;
	light.hasChanged = false;

	UpdateLighting = true;

	return lid;
}

void AddUnLight(int i)
{
#ifdef _DEBUG
	if (DisableLighting)
		return;
#endif
	if (i == NO_LIGHT)
		return;

	Lights[i].isInvalid = true;

	UpdateLighting = true;
}

void ChangeLightRadius(int i, uint8_t radius)
{
#ifdef _DEBUG
	if (DisableLighting)
		return;
#endif
	if (i == NO_LIGHT)
		return;

	Light &light = Lights[i];
	light.hasChanged = true;
	light.position.old = light.position.tile;
	light.oldRadius = light.radius;
	light.radius = radius;

	UpdateLighting = true;
}

void ChangeLightXY(int i, Point position)
{
#ifdef _DEBUG
	if (DisableLighting)
		return;
#endif
	if (i == NO_LIGHT)
		return;

	Light &light = Lights[i];
	light.hasChanged = true;
	light.position.old = light.position.tile;
	light.oldRadius = light.radius;
	light.position.tile = position;

	UpdateLighting = true;
}

void ChangeLightOffset(int i, DisplacementOf<int8_t> offset)
{
#ifdef _DEBUG
	if (DisableLighting)
		return;
#endif
	if (i == NO_LIGHT)
		return;

	Light &light = Lights[i];
	if (light.position.offset == offset)
		return;

	light.hasChanged = true;
	light.position.old = light.position.tile;
	light.oldRadius = light.radius;
	light.position.offset = offset;

	UpdateLighting = true;
}

void ChangeLight(int i, Point position, uint8_t radius)
{
#ifdef _DEBUG
	if (DisableLighting)
		return;
#endif
	if (i == NO_LIGHT)
		return;

	Light &light = Lights[i];
	light.hasChanged = true;
	light.position.old = light.position.tile;
	light.oldRadius = light.radius;
	light.position.tile = position;
	light.radius = radius;

	UpdateLighting = true;
}

void ProcessLightList()
{
#ifdef _DEBUG
	if (DisableLighting)
		return;
#endif
	if (!UpdateLighting)
		return;
	for (int i = 0; i < ActiveLightCount; i++) {
		Light &light = Lights[ActiveLights[i]];
		if (light.isInvalid) {
			DoUnLight(light.position.tile, light.radius);
		}
		if (light.hasChanged) {
			DoUnLight(light.position.old, light.oldRadius);
			light.hasChanged = false;
		}
	}
	for (int i = 0; i < ActiveLightCount; i++) {
		const Light &light = Lights[ActiveLights[i]];
		if (light.isInvalid) {
			ActiveLightCount--;
			std::swap(ActiveLights[ActiveLightCount], ActiveLights[i]);
			i--;
			continue;
		}
		if (TileHasAny(light.position.tile, TileProperties::Solid))
			continue; // Monster hidden in a wall, don't spoil the surprise
		DoLighting(light.position.tile, light.radius, light.position.offset);
	}

	UpdateLighting = false;
}

void SavePreLighting()
{
	memcpy(dPreLight, dLight, sizeof(dPreLight));
}

void ActivateVision(Point position, int r, size_t id)
{
	auto &vision = VisionList[id];
	vision.position.tile = position;
	vision.radius = r;
	vision.isInvalid = false;
	vision.hasChanged = false;
	VisionActive[id] = true;

	UpdateVision = true;
}

void ChangeVisionRadius(size_t id, int r)
{
	auto &vision = VisionList[id];
	vision.hasChanged = true;
	vision.position.old = vision.position.tile;
	vision.oldRadius = vision.radius;
	vision.radius = r;
	UpdateVision = true;
}

void ChangeVisionXY(size_t id, Point position)
{
	auto &vision = VisionList[id];
	vision.hasChanged = true;
	vision.position.old = vision.position.tile;
	vision.oldRadius = vision.radius;
	vision.position.tile = position;
	UpdateVision = true;
}

void ProcessVisionList()
{
	if (!UpdateVision)
		return;

	TransList = {};

	for (const Player &player : Players) {
		const size_t id = player.getId();
		if (!VisionActive[id])
			continue;
		Light &vision = VisionList[id];
		if (!player.plractive || !player.isOnActiveLevel() || (player._pLvlChanging && &player != MyPlayer)) {
			DoUnVision(vision.position.tile, vision.radius);
			VisionActive[id] = false;
			continue;
		}
		if (vision.hasChanged) {
			DoUnVision(vision.position.old, vision.oldRadius);
			vision.hasChanged = false;
		}
	}
	for (const Player &player : Players) {
		const size_t id = player.getId();
		if (!VisionActive[id])
			continue;
		const Light &vision = VisionList[id];
		MapExplorationType doautomap = MAP_EXP_SELF;
		if (&player != MyPlayer)
			doautomap = player.friendlyMode ? MAP_EXP_OTHERS : MAP_EXP_NONE;
		DoVision(
		    vision.position.tile,
		    vision.radius,
		    doautomap,
		    &player == MyPlayer);
	}

	UpdateVision = false;
}

void lighting_color_cycling()
{
	for (auto &lightTable : LightTables) {
		// shift elements between indexes 1-31 to left
		std::rotate(lightTable.begin() + 1, lightTable.begin() + 2, lightTable.begin() + 32);
	}
}

} // namespace devilution
