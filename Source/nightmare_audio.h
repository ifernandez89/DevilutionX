/**
 * @file nightmare_audio.h
 * @brief Nightmare Edition - Enhanced Audio System
 * 
 * SISTEMA DE AUDIO MEJORADO NIGHTMARE
 * ==================================
 * 
 * Sistema avanzado de audio que agrega efectos atmosféricos,
 * audio posicional, y mejoras en la inmersión sonora.
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Audio atmosférico y tenebroso
 * - Efectos posicionales realistas
 * - Inmersión sonora profunda
 * - Performance optimizada
 */

#pragma once

#include <cstdint>
#include <array>
#include <string>

namespace devilution {

/**
 * @brief Tipos de efectos de audio atmosférico
 */
enum class AtmosphericAudioType {
	AMBIENT_WIND,       ///< Viento ambiental
	DISTANT_THUNDER,    ///< Truenos lejanos
	CAVE_DRIPS,         ///< Goteos en cuevas
	HELL_WHISPERS,      ///< Susurros infernales
	TORCH_CRACKLE,      ///< Crepitar de antorchas
	FOOTSTEPS_ECHO,     ///< Eco de pasos
	MONSTER_BREATH,     ///< Respiración de monstruos
	MAGIC_RESONANCE,    ///< Resonancia mágica
	NONE                ///< Sin efecto
};

/**
 * @brief Estado de un efecto de audio atmosférico
 */
struct AtmosphericAudio {
	AtmosphericAudioType type = AtmosphericAudioType::NONE;
	float volume = 1.0f;                        ///< Volumen (0.0-1.0)
	float pitch = 1.0f;                         ///< Pitch (0.5-2.0)
	uint32_t lastPlayTime = 0;                  ///< Último tiempo de reproducción
	uint32_t playInterval = 5000;               ///< Intervalo entre reproducciones (ms)
	uint32_t randomVariation = 2000;            ///< Variación aleatoria (ms)
	bool enabled = true;                        ///< Efecto habilitado
	bool isPlaying = false;                     ///< Está reproduciéndose
};

/**
 * @brief Configuración del sistema de audio mejorado
 */
struct NightmareAudioConfig {
	bool enabled = true;                        ///< Sistema habilitado
	bool atmosphericAudio = true;               ///< Audio atmosférico
	bool positionalAudio = true;                ///< Audio posicional
	bool enhancedFootsteps = true;              ///< Pasos mejorados
	bool monsterBreathing = true;               ///< Respiración de monstruos
	bool magicResonance = true;                 ///< Resonancia mágica
	float masterVolume = 1.0f;                  ///< Volumen maestro (0.0-1.0)
	float atmosphericVolume = 0.3f;             ///< Volumen atmosférico (0.0-1.0)
	float positionalRange = 15.0f;              ///< Rango de audio posicional
};

/**
 * @brief Estado global del sistema de audio mejorado
 */
struct NightmareAudioState {
	bool initialized = false;
	NightmareAudioConfig config;
	std::array<AtmosphericAudio, 16> atmosphericEffects;
	int activeEffectCount = 0;
	uint32_t lastUpdateTime = 0;
	
	// Cache de sonidos
	bool soundsLoaded = false;
	int currentLevelType = -1;                  ///< Tipo de nivel actual para cambiar audio
};

// Estado global
extern NightmareAudioState g_nightmareAudio;

/**
 * @brief Inicializa el sistema de audio mejorado
 */
void InitNightmareAudio();

/**
 * @brief Actualiza el sistema de audio mejorado
 */
void UpdateNightmareAudio();

/**
 * @brief Reproduce un efecto atmosférico
 * @param type Tipo de efecto
 * @param volume Volumen (0.0-1.0)
 * @param pitch Pitch (0.5-2.0)
 */
void PlayAtmosphericEffect(AtmosphericAudioType type, float volume = 1.0f, float pitch = 1.0f);

/**
 * @brief Reproduce audio posicional
 * @param soundId ID del sonido
 * @param x Posición X
 * @param y Posición Y
 * @param volume Volumen base
 */
void PlayPositionalAudio(int soundId, int x, int y, float volume = 1.0f);

/**
 * @brief Actualiza audio atmosférico según el nivel
 * @param levelType Tipo de nivel actual
 */
void UpdateLevelAtmosphere(int levelType);

/**
 * @brief Habilita/deshabilita el sistema de audio mejorado
 * @param enabled Estado a establecer
 */
void SetNightmareAudioEnabled(bool enabled);

/**
 * @brief Establece el volumen maestro
 * @param volume Volumen (0.0-1.0)
 */
void SetNightmareAudioVolume(float volume);

/**
 * @brief Limpia el sistema de audio mejorado
 */
void CleanupNightmareAudio();

} // namespace devilution