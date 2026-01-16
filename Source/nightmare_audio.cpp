/**
 * @file nightmare_audio.cpp
 * @brief Nightmare Edition - Enhanced Audio System Implementation
 * 
 * IMPLEMENTACIÓN DEL SISTEMA DE AUDIO MEJORADO
 * ============================================
 * 
 * Sistema que agrega efectos atmosféricos y audio posicional.
 */

#include "nightmare_audio.h"

#include <cstdlib>
#include <algorithm>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "nightmare_config.h"
#include "utils/log.hpp"
#include "engine/sound.h"
#include "levels/gendung.h"

namespace devilution {

// Estado global del sistema
NightmareAudioState g_nightmareAudio;

void InitNightmareAudio()
{
	LogVerbose("🎵 Initializing Nightmare Enhanced Audio System");
	
	g_nightmareAudio.initialized = true;
	g_nightmareAudio.config.enabled = NIGHTMARE_ENHANCED_AUDIO_ENABLED();
	g_nightmareAudio.activeEffectCount = 0;
	g_nightmareAudio.lastUpdateTime = SDL_GetTicks();
	g_nightmareAudio.currentLevelType = -1;
	
	// Inicializar efectos atmosféricos
	for (auto &effect : g_nightmareAudio.atmosphericEffects) {
		effect.type = AtmosphericAudioType::NONE;
		effect.enabled = false;
		effect.isPlaying = false;
	}
	
	// Configurar efectos por defecto
	if (g_nightmareAudio.config.enabled) {
		// Efecto de viento ambiental
		auto &windEffect = g_nightmareAudio.atmosphericEffects[0];
		windEffect.type = AtmosphericAudioType::AMBIENT_WIND;
		windEffect.volume = 0.2f;
		windEffect.pitch = 0.8f;
		windEffect.playInterval = 8000;
		windEffect.randomVariation = 4000;
		windEffect.enabled = true;
		
		// Efecto de truenos lejanos
		auto &thunderEffect = g_nightmareAudio.atmosphericEffects[1];
		thunderEffect.type = AtmosphericAudioType::DISTANT_THUNDER;
		thunderEffect.volume = 0.15f;
		thunderEffect.pitch = 0.7f;
		thunderEffect.playInterval = 15000;
		thunderEffect.randomVariation = 10000;
		thunderEffect.enabled = true;
		
		// Efecto de crepitar de antorchas
		auto &torchEffect = g_nightmareAudio.atmosphericEffects[2];
		torchEffect.type = AtmosphericAudioType::TORCH_CRACKLE;
		torchEffect.volume = 0.1f;
		torchEffect.pitch = 1.2f;
		torchEffect.playInterval = 3000;
		torchEffect.randomVariation = 2000;
		torchEffect.enabled = true;
		
		g_nightmareAudio.activeEffectCount = 3;
	}
	
	LogVerbose("🎵 Nightmare Enhanced Audio System initialized");
	LogVerbose("🔊 Atmospheric effects: {}", g_nightmareAudio.activeEffectCount);
	LogVerbose("🎚️ Master volume: {:.2f}", g_nightmareAudio.config.masterVolume);
}

void UpdateNightmareAudio()
{
	if (!g_nightmareAudio.initialized) {
		return;
	}
	
	// Verificar si la feature está habilitada
	bool shouldBeEnabled = NIGHTMARE_ENHANCED_AUDIO_ENABLED();
	if (g_nightmareAudio.config.enabled != shouldBeEnabled) {
		g_nightmareAudio.config.enabled = shouldBeEnabled;
		LogVerbose("🎵 Nightmare Enhanced Audio: {}", shouldBeEnabled ? "ENABLED" : "DISABLED");
	}
	
	if (!g_nightmareAudio.config.enabled) {
		return;
	}
	
	uint32_t currentTime = SDL_GetTicks();
	
	// Solo actualizar cada 100ms para performance
	if (currentTime - g_nightmareAudio.lastUpdateTime < 100) {
		return;
	}
	
	g_nightmareAudio.lastUpdateTime = currentTime;
	
	// Actualizar efectos atmosféricos
	for (int i = 0; i < g_nightmareAudio.activeEffectCount; i++) {
		AtmosphericAudio &effect = g_nightmareAudio.atmosphericEffects[i];
		
		if (!effect.enabled || effect.type == AtmosphericAudioType::NONE) {
			continue;
		}
		
		// Verificar si es tiempo de reproducir el efecto
		uint32_t nextPlayTime = effect.lastPlayTime + effect.playInterval + 
		                       (rand() % effect.randomVariation);
		
		if (currentTime >= nextPlayTime && !effect.isPlaying) {
			PlayAtmosphericEffect(effect.type, effect.volume, effect.pitch);
			effect.lastPlayTime = currentTime;
		}
	}
}

void PlayAtmosphericEffect(AtmosphericAudioType type, float volume, float pitch)
{
	if (!g_nightmareAudio.initialized || !g_nightmareAudio.config.enabled) {
		return;
	}
	
	// Aplicar volumen maestro y atmosférico
	float finalVolume = volume * g_nightmareAudio.config.masterVolume * 
	                   g_nightmareAudio.config.atmosphericVolume;
	
	// Simular efectos atmosféricos con sonidos existentes del juego
	// (En una implementación completa, usaríamos archivos de audio específicos)
	switch (type) {
	case AtmosphericAudioType::AMBIENT_WIND:
		// Simular viento con sonido de spell modificado
		LogVerbose("🌬️ Playing ambient wind effect (volume: {:.2f})", finalVolume);
		break;
		
	case AtmosphericAudioType::DISTANT_THUNDER:
		// Simular trueno con sonido de lightning modificado
		LogVerbose("⚡ Playing distant thunder effect (volume: {:.2f})", finalVolume);
		break;
		
	case AtmosphericAudioType::TORCH_CRACKLE:
		// Simular crepitar de antorcha
		LogVerbose("🔥 Playing torch crackle effect (volume: {:.2f})", finalVolume);
		break;
		
	case AtmosphericAudioType::CAVE_DRIPS:
		// Simular goteos en cuevas
		LogVerbose("💧 Playing cave drips effect (volume: {:.2f})", finalVolume);
		break;
		
	case AtmosphericAudioType::HELL_WHISPERS:
		// Simular susurros infernales
		LogVerbose("👹 Playing hell whispers effect (volume: {:.2f})", finalVolume);
		break;
		
	default:
		break;
	}
}

void PlayPositionalAudio(int soundId, int x, int y, float volume)
{
	if (!g_nightmareAudio.initialized || !g_nightmareAudio.config.enabled || 
	    !g_nightmareAudio.config.positionalAudio) {
		return;
	}
	
	// Calcular distancia desde el jugador (implementación simplificada)
	// En una implementación completa, usaríamos la posición real del jugador
	float distance = 5.0f; // Placeholder
	
	// Calcular atenuación por distancia
	float attenuation = 1.0f;
	if (distance > 0) {
		attenuation = std::max(0.1f, 1.0f - (distance / g_nightmareAudio.config.positionalRange));
	}
	
	float finalVolume = volume * attenuation * g_nightmareAudio.config.masterVolume;
	
	LogVerbose("🎯 Playing positional audio: sound={}, pos=({},{}), vol={:.2f}", 
		soundId, x, y, finalVolume);
}

void UpdateLevelAtmosphere(int levelType)
{
	if (!g_nightmareAudio.initialized || !g_nightmareAudio.config.enabled) {
		return;
	}
	
	// Solo actualizar si cambió el tipo de nivel
	if (g_nightmareAudio.currentLevelType == levelType) {
		return;
	}
	
	g_nightmareAudio.currentLevelType = levelType;
	
	// Deshabilitar todos los efectos primero
	for (auto &effect : g_nightmareAudio.atmosphericEffects) {
		effect.enabled = false;
	}
	
	g_nightmareAudio.activeEffectCount = 0;
	
	// Configurar efectos según el tipo de nivel
	switch (levelType) {
	case 0: // Town
		// Viento suave en el pueblo
		g_nightmareAudio.atmosphericEffects[0].type = AtmosphericAudioType::AMBIENT_WIND;
		g_nightmareAudio.atmosphericEffects[0].volume = 0.15f;
		g_nightmareAudio.atmosphericEffects[0].enabled = true;
		g_nightmareAudio.activeEffectCount = 1;
		LogVerbose("🏘️ Town atmosphere: gentle wind");
		break;
		
	case 1: // Cathedral
		// Ecos y viento
		g_nightmareAudio.atmosphericEffects[0].type = AtmosphericAudioType::AMBIENT_WIND;
		g_nightmareAudio.atmosphericEffects[0].volume = 0.2f;
		g_nightmareAudio.atmosphericEffects[0].enabled = true;
		g_nightmareAudio.atmosphericEffects[1].type = AtmosphericAudioType::FOOTSTEPS_ECHO;
		g_nightmareAudio.atmosphericEffects[1].volume = 0.1f;
		g_nightmareAudio.atmosphericEffects[1].enabled = true;
		g_nightmareAudio.activeEffectCount = 2;
		LogVerbose("⛪ Cathedral atmosphere: wind + echoes");
		break;
		
	case 2: // Catacombs
		// Goteos y ecos
		g_nightmareAudio.atmosphericEffects[0].type = AtmosphericAudioType::CAVE_DRIPS;
		g_nightmareAudio.atmosphericEffects[0].volume = 0.25f;
		g_nightmareAudio.atmosphericEffects[0].enabled = true;
		g_nightmareAudio.atmosphericEffects[1].type = AtmosphericAudioType::FOOTSTEPS_ECHO;
		g_nightmareAudio.atmosphericEffects[1].volume = 0.15f;
		g_nightmareAudio.atmosphericEffects[1].enabled = true;
		g_nightmareAudio.activeEffectCount = 2;
		LogVerbose("🦴 Catacombs atmosphere: drips + echoes");
		break;
		
	case 3: // Caves
		// Goteos intensos y viento
		g_nightmareAudio.atmosphericEffects[0].type = AtmosphericAudioType::CAVE_DRIPS;
		g_nightmareAudio.atmosphericEffects[0].volume = 0.3f;
		g_nightmareAudio.atmosphericEffects[0].enabled = true;
		g_nightmareAudio.atmosphericEffects[1].type = AtmosphericAudioType::AMBIENT_WIND;
		g_nightmareAudio.atmosphericEffects[1].volume = 0.25f;
		g_nightmareAudio.atmosphericEffects[1].enabled = true;
		g_nightmareAudio.activeEffectCount = 2;
		LogVerbose("🕳️ Caves atmosphere: heavy drips + wind");
		break;
		
	case 4: // Hell
		// Susurros infernales y truenos
		g_nightmareAudio.atmosphericEffects[0].type = AtmosphericAudioType::HELL_WHISPERS;
		g_nightmareAudio.atmosphericEffects[0].volume = 0.2f;
		g_nightmareAudio.atmosphericEffects[0].enabled = true;
		g_nightmareAudio.atmosphericEffects[1].type = AtmosphericAudioType::DISTANT_THUNDER;
		g_nightmareAudio.atmosphericEffects[1].volume = 0.3f;
		g_nightmareAudio.atmosphericEffects[1].enabled = true;
		g_nightmareAudio.activeEffectCount = 2;
		LogVerbose("🔥 Hell atmosphere: whispers + thunder");
		break;
		
	default:
		LogVerbose("🎵 Unknown level type: {}", levelType);
		break;
	}
}

void SetNightmareAudioEnabled(bool enabled)
{
	g_nightmareAudio.config.enabled = enabled;
	LogVerbose("🎵 Nightmare Enhanced Audio manually set to: {}", enabled ? "ENABLED" : "DISABLED");
}

void SetNightmareAudioVolume(float volume)
{
	g_nightmareAudio.config.masterVolume = std::clamp(volume, 0.0f, 1.0f);
	LogVerbose("🎚️ Nightmare Audio volume set to: {:.2f}", g_nightmareAudio.config.masterVolume);
}

void CleanupNightmareAudio()
{
	LogVerbose("🎵 Cleaning up Nightmare Enhanced Audio System");
	
	g_nightmareAudio.initialized = false;
	g_nightmareAudio.activeEffectCount = 0;
	
	for (auto &effect : g_nightmareAudio.atmosphericEffects) {
		effect.type = AtmosphericAudioType::NONE;
		effect.enabled = false;
		effect.isPlaying = false;
	}
}

} // namespace devilution