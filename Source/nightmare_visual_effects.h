/**
 * @file nightmare_visual_effects.h
 * @brief Nightmare Edition - Visual Effects System
 * 
 * SISTEMA DE EFECTOS VISUALES NIGHTMARE
 * ====================================
 * 
 * Sistema avanzado de efectos visuales que agrega shaders,
 * partículas, y mejoras visuales atmosféricas.
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Efectos sutiles y atmosféricos
 * - Performance optimizada O(1)
 * - Inmersión visual profunda
 * - Respeto por la estética original
 */

#pragma once

#include <cstdint>
#include <array>

namespace devilution {

/**
 * @brief Tipos de efectos visuales
 */
enum class VisualEffectType {
	SCREEN_DISTORTION,      ///< Distorsión de pantalla sutil
	COLOR_GRADING,          ///< Corrección de color atmosférica
	VIGNETTE_EFFECT,        ///< Efecto de viñeta dinámica
	PARTICLE_DUST,          ///< Partículas de polvo
	PARTICLE_EMBERS,        ///< Brasas flotantes
	PARTICLE_MIST,          ///< Niebla de partículas
	LIGHT_RAYS,             ///< Rayos de luz
	SHADOW_ENHANCEMENT,     ///< Mejora de sombras
	MAGIC_AURA,             ///< Aura mágica
	BLOOD_SPLATTER,         ///< Salpicaduras de sangre
	NONE                    ///< Sin efecto
};

/**
 * @brief Estado de un efecto visual nightmare
 */
struct NightmareVisualEffect {
	VisualEffectType type = VisualEffectType::NONE;
	float intensity = 1.0f;                     ///< Intensidad (0.0-1.0)
	float duration = 0.0f;                      ///< Duración en segundos (0 = permanente)
	uint32_t startTime = 0;                     ///< Tiempo de inicio
	bool enabled = true;                        ///< Efecto habilitado
	bool isActive = false;                      ///< Está activo
	
	// Parámetros específicos del efecto
	float param1 = 0.0f;
	float param2 = 0.0f;
	float param3 = 0.0f;
};

/**
 * @brief Configuración del sistema de efectos visuales
 */
struct NightmareVisualConfig {
	bool enabled = true;                        ///< Sistema habilitado
	bool screenEffects = true;                  ///< Efectos de pantalla
	bool particleEffects = true;                ///< Efectos de partículas
	bool lightingEffects = true;                ///< Efectos de iluminación
	bool bloodEffects = true;                   ///< Efectos de sangre
	bool magicEffects = true;                   ///< Efectos mágicos
	float masterIntensity = 1.0f;               ///< Intensidad maestra (0.0-1.0)
	float particleDensity = 0.5f;               ///< Densidad de partículas (0.0-1.0)
	int maxParticles = 64;                      ///< Máximo número de partículas
};

/**
 * @brief Partícula individual
 */
struct Particle {
	float x = 0.0f, y = 0.0f;                   ///< Posición
	float vx = 0.0f, vy = 0.0f;                 ///< Velocidad
	float life = 1.0f;                          ///< Vida restante (0.0-1.0)
	float size = 1.0f;                          ///< Tamaño
	uint8_t color = 255;                        ///< Color/intensidad
	VisualEffectType type = VisualEffectType::NONE;
	bool active = false;                        ///< Partícula activa
};

/**
 * @brief Estado global del sistema de efectos visuales
 */
struct NightmareVisualState {
	bool initialized = false;
	NightmareVisualConfig config;
	std::array<NightmareVisualEffect, 16> effects;
	std::array<Particle, 64> particles;
	int activeEffectCount = 0;
	int activeParticleCount = 0;
	uint32_t lastUpdateTime = 0;
	
	// Estado de efectos de pantalla
	float vignetteIntensity = 0.0f;
	float colorGradingR = 1.0f;
	float colorGradingG = 1.0f;
	float colorGradingB = 1.0f;
	float screenDistortion = 0.0f;
};

// Estado global
extern NightmareVisualState g_nightmareVisual;

/**
 * @brief Inicializa el sistema de efectos visuales
 */
void InitNightmareVisualEffects();

/**
 * @brief Actualiza el sistema de efectos visuales
 */
void UpdateNightmareVisualEffects();

/**
 * @brief Renderiza los efectos visuales
 */
void RenderNightmareVisualEffects();

/**
 * @brief Activa un efecto visual
 * @param type Tipo de efecto
 * @param intensity Intensidad (0.0-1.0)
 * @param duration Duración en segundos (0 = permanente)
 */
void ActivateVisualEffect(VisualEffectType type, float intensity = 1.0f, float duration = 0.0f);

/**
 * @brief Desactiva un efecto visual
 * @param type Tipo de efecto a desactivar
 */
void DeactivateVisualEffect(VisualEffectType type);

/**
 * @brief Crea partículas de un tipo específico
 * @param type Tipo de partícula
 * @param x Posición X
 * @param y Posición Y
 * @param count Número de partículas
 */
void CreateParticles(VisualEffectType type, float x, float y, int count = 1);

/**
 * @brief Actualiza efectos según el nivel
 * @param levelType Tipo de nivel actual
 */
void UpdateLevelVisualEffects(int levelType);

/**
 * @brief Habilita/deshabilita el sistema de efectos visuales
 * @param enabled Estado a establecer
 */
void SetNightmareVisualEffectsEnabled(bool enabled);

/**
 * @brief Establece la intensidad maestra de efectos
 * @param intensity Intensidad (0.0-1.0)
 */
void SetVisualEffectsIntensity(float intensity);

/**
 * @brief Actualiza las partículas del sistema
 * @param deltaTime Tiempo transcurrido en segundos
 */
void UpdateParticles(float deltaTime);

/**
 * @brief Limpia el sistema de efectos visuales
 */
void CleanupNightmareVisualEffects();

} // namespace devilution