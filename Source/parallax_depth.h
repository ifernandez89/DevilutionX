/**
 * @file parallax_depth.h
 * 
 * 🌟 FASE D3.2 - PARALLAX FALSO SUTIL 🌟
 * 
 * Sistema de parallax falso para crear sensación de profundidad
 * sin modificar la cámara, colisiones o lógica del juego.
 */

#pragma once

#include <cstdint>
#include <array>
#include <vector>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "engine/point.hpp"
#include "engine/rectangle.hpp"

namespace devilution {

// ============================================================================
// 🎯 TIPOS DE ELEMENTOS PARALLAX
// ============================================================================

/**
 * @brief Tipos de elementos que pueden tener efecto parallax
 */
enum class ParallaxElementType : uint8_t {
	None = 0,
	StaticBackground,    // Fondos estáticos (muros lejanos, cielo)
	Fire,               // Fuegos y antorchas
	Water,              // Agua y líquidos
	Smoke,              // Humo y vapores
	Decoration,         // Elementos decorativos lejanos
	Architecture,       // Arquitectura de fondo
	Atmosphere,         // Elementos atmosféricos
	Lighting,           // Efectos de iluminación de fondo
	Vegetation,         // Vegetación de fondo (Town)
	Clouds              // Nubes y efectos de cielo
};

/**
 * @brief Capas de profundidad para parallax
 */
enum class ParallaxLayer : uint8_t {
	Background = 0,     // Capa más lejana (movimiento mínimo)
	Midground,          // Capa media (movimiento moderado)
	Nearground,         // Capa cercana (movimiento casi normal)
	Foreground          // Capa frontal (sin parallax, movimiento normal)
};

/**
 * @brief Configuración de un elemento parallax
 */
struct ParallaxElement {
	ParallaxElementType type = ParallaxElementType::None;
	ParallaxLayer layer = ParallaxLayer::Foreground;
	Point originalPosition = { 0, 0 };    // Posición original del elemento
	Point currentOffset = { 0, 0 };       // Offset actual aplicado
	float depthFactor = 1.0f;              // Factor de profundidad (0.0-1.0)
	float parallaxIntensity = 1.0f;        // Intensidad del efecto (0.0-2.0)
	bool enabled = true;                   // Si está habilitado
	uint32_t lastUpdateTime = 0;           // Último tiempo de actualización
	
	// Parámetros específicos por tipo
	float windEffect = 0.0f;               // Efecto de viento (para vegetation, smoke)
	float flickerEffect = 0.0f;            // Efecto de parpadeo (para fire)
	float waveEffect = 0.0f;               // Efecto de ondas (para water)
};

/**
 * @brief Configuración global del sistema parallax
 */
struct ParallaxConfig {
	bool enabled = true;
	float globalIntensity = 0.8f;          // Intensidad global (0.0-2.0)
	float cameraInfluence = 1.0f;          // Influencia del movimiento de cámara
	float subtletyFactor = 0.6f;           // Factor de sutileza (más bajo = más sutil)
	
	// Factores por capa
	float backgroundFactor = 0.2f;         // Background se mueve 20% de la velocidad normal
	float midgroundFactor = 0.5f;          // Midground se mueve 50% de la velocidad normal
	float neargroundFactor = 0.8f;         // Nearground se mueve 80% de la velocidad normal
	
	// Configuración por tipo de nivel
	bool enableInTown = true;
	bool enableInDungeons = true;
	bool enableInHell = false;             // Deshabilitado en Hell por defecto (muy caótico)
	
	// Límites de performance
	uint32_t maxActiveElements = 48;       // Máximo número de elementos activos
	float updateFrequency = 60.0f;         // Frecuencia de actualización (FPS)
};

/**
 * @brief Estado del sistema parallax
 */
struct ParallaxState {
	bool enabled = true;
	Point lastCameraPosition = { 0, 0 };   // Última posición de cámara conocida
	Point cameraVelocity = { 0, 0 };       // Velocidad de movimiento de cámara
	uint32_t activeElements = 0;           // Número de elementos activos
	uint32_t lastUpdateTime = 0;           // Último tiempo de actualización global
	
	// Elementos parallax activos
	std::vector<ParallaxElement> elements;
	
	// Configuración actual
	ParallaxConfig config;
	
	// Métricas de performance
	uint32_t totalCalculationsThisFrame = 0;
	float averageCalculationTime = 0.0f;
	uint32_t framesProcessed = 0;
};

/**
 * @brief Estadísticas del sistema para debugging
 */
struct ParallaxStats {
	uint64_t totalElementsProcessed = 0;
	uint64_t totalOffsetCalculations = 0;
	uint32_t peakActiveElements = 0;
	float averageProcessingTime = 0.0f;
	uint32_t framesWithParallax = 0;
	
	// Estadísticas por tipo
	std::array<uint32_t, 10> elementsByType = {};  // Contadores por ParallaxElementType
	std::array<uint32_t, 4> elementsByLayer = {};  // Contadores por ParallaxLayer
};

// ============================================================================
// 🎯 API PRINCIPAL
// ============================================================================

/**
 * @brief Inicializar el sistema de parallax
 */
void InitParallaxDepth();

/**
 * @brief Actualizar el sistema cada frame
 */
void UpdateParallaxDepth();

/**
 * @brief Registrar un elemento para parallax
 */
void RegisterParallaxElement(ParallaxElementType type, ParallaxLayer layer, Point position);

/**
 * @brief Calcular offset parallax para una posición
 */
Point CalculateParallaxOffset(Point originalPosition, ParallaxLayer layer);

/**
 * @brief Aplicar efecto parallax a coordenadas de render
 */
Point ApplyParallaxEffect(Point originalPos, ParallaxElementType type, ParallaxLayer layer);

/**
 * @brief Actualizar posición de cámara para cálculos parallax
 */
void UpdateParallaxCamera(Point cameraPosition);

/**
 * @brief Limpiar elementos parallax (al cambiar de nivel)
 */
void ClearParallaxElements();

// ============================================================================
// 🎯 DETECCIÓN AUTOMÁTICA DE ELEMENTOS
// ============================================================================

/**
 * @brief Detectar automáticamente elementos parallax en el nivel actual
 */
void DetectParallaxElements();

/**
 * @brief Clasificar un elemento como parallax basado en su tipo y posición
 */
ParallaxElementType ClassifyParallaxElement(Point position, int tileType);

/**
 * @brief Determinar la capa de profundidad apropiada para un elemento
 */
ParallaxLayer DetermineParallaxLayer(ParallaxElementType type, Point position);

// ============================================================================
// 🎯 CONFIGURACIÓN Y PRESETS
// ============================================================================

/**
 * @brief Aplicar preset sutil (efecto mínimo, máxima compatibilidad)
 */
void ApplySubtleParallaxPreset();

/**
 * @brief Aplicar preset balanceado (efecto moderado)
 */
void ApplyBalancedParallaxPreset();

/**
 * @brief Aplicar preset dramático (efecto máximo)
 */
void ApplyDramaticParallaxPreset();

/**
 * @brief Configurar parallax específico por tipo de nivel
 */
void ConfigureParallaxForLevel(int levelType);

/**
 * @brief Obtener configuración actual del sistema
 */
const ParallaxState &GetParallaxState();

/**
 * @brief Obtener estadísticas del sistema
 */
const ParallaxStats &GetParallaxStats();

// ============================================================================
// 🎯 INTEGRACIÓN CON SISTEMAS EXISTENTES
// ============================================================================

/**
 * @brief Integrar con sistema de depth cues (FASE D1)
 */
void IntegrateWithDepthCues(Point position, float depthIntensity);

/**
 * @brief Integrar con sistema de lighting (FASE V1)
 */
void IntegrateWithLighting(Point position, int lightRadius);

/**
 * @brief Integrar con sistema de paletas contextuales (FASE V2)
 */
void IntegrateWithContextualPalette(ParallaxElementType type, float &colorMultiplier);

// ============================================================================
// 🎯 DEBUGGING Y UTILIDADES
// ============================================================================

#ifdef _DEBUG
/**
 * @brief Habilitar/deshabilitar logging de debugging
 */
void SetParallaxDebugLogging(bool enabled);

/**
 * @brief Forzar tipo de parallax para testing
 */
void ForceParallaxType(ParallaxElementType type);

/**
 * @brief Obtener información de debugging como string
 */
std::string GetParallaxDebugInfo();

/**
 * @brief Renderizar información de debugging en pantalla
 */
void RenderParallaxDebugOverlay();
#endif

} // namespace devilution