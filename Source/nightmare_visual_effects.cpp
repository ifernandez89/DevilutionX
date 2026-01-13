/**
 * @file nightmare_visual_effects.cpp
 * @brief Nightmare Edition - Visual Effects System Implementation
 * 
 * IMPLEMENTACIÓN DEL SISTEMA DE EFECTOS VISUALES
 * ==============================================
 * 
 * Sistema que agrega efectos visuales atmosféricos y partículas.
 */

#include "nightmare_visual_effects.h"

#include <cstdlib>
#include <algorithm>
#include <cmath>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "nightmare_config.h"
#include "utils/log.hpp"
#include "levels/gendung.h"

namespace devilution {

// Estado global del sistema
NightmareVisualState g_nightmareVisual;

void InitNightmareVisualEffects()
{
	LogVerbose("✨ Initializing Nightmare Visual Effects System");
	
	g_nightmareVisual.initialized = true;
	g_nightmareVisual.config.enabled = NIGHTMARE_VISUAL_EFFECTS_ENABLED();
	g_nightmareVisual.activeEffectCount = 0;
	g_nightmareVisual.activeParticleCount = 0;
	g_nightmareVisual.lastUpdateTime = SDL_GetTicks();
	
	// Inicializar efectos
	for (auto &effect : g_nightmareVisual.effects) {
		effect.type = VisualEffectType::NONE;
		effect.enabled = false;
		effect.isActive = false;
	}
	
	// Inicializar partículas
	for (auto &particle : g_nightmareVisual.particles) {
		particle.active = false;
		particle.life = 0.0f;
	}
	
	// Configurar efectos por defecto
	if (g_nightmareVisual.config.enabled) {
		// Efecto de viñeta sutil
		ActivateVisualEffect(VisualEffectType::VIGNETTE_EFFECT, 0.3f);
		
		// Corrección de color para atmósfera más oscura
		ActivateVisualEffect(VisualEffectType::COLOR_GRADING, 0.8f);
		
		// Partículas de polvo ambiental
		ActivateVisualEffect(VisualEffectType::PARTICLE_DUST, 0.2f);
	}
	
	LogVerbose("✨ Nightmare Visual Effects System initialized");
	LogVerbose("🎨 Active effects: {}", g_nightmareVisual.activeEffectCount);
	LogVerbose("💫 Max particles: {}", g_nightmareVisual.config.maxParticles);
}

void UpdateNightmareVisualEffects()
{
	if (!g_nightmareVisual.initialized) {
		return;
	}
	
	// Verificar si la feature está habilitada
	bool shouldBeEnabled = NIGHTMARE_VISUAL_EFFECTS_ENABLED();
	if (g_nightmareVisual.config.enabled != shouldBeEnabled) {
		g_nightmareVisual.config.enabled = shouldBeEnabled;
		LogVerbose("✨ Nightmare Visual Effects: {}", shouldBeEnabled ? "ENABLED" : "DISABLED");
	}
	
	if (!g_nightmareVisual.config.enabled) {
		return;
	}
	
	uint32_t currentTime = SDL_GetTicks();
	float deltaTime = (currentTime - g_nightmareVisual.lastUpdateTime) / 1000.0f;
	
	// Solo actualizar cada 16ms (~60 FPS) para performance
	if (currentTime - g_nightmareVisual.lastUpdateTime < 16) {
		return;
	}
	
	g_nightmareVisual.lastUpdateTime = currentTime;
	
	// Actualizar efectos activos
	for (int i = 0; i < g_nightmareVisual.activeEffectCount; i++) {
		NightmareVisualEffect &effect = g_nightmareVisual.effects[i];
		
		if (!effect.enabled || !effect.isActive) {
			continue;
		}
		
		// Verificar duración del efecto
		if (effect.duration > 0.0f) {
			float elapsed = (currentTime - effect.startTime) / 1000.0f;
			if (elapsed >= effect.duration) {
				effect.isActive = false;
				continue;
			}
		}
		
		// Actualizar parámetros específicos del efecto
		switch (effect.type) {
		case VisualEffectType::VIGNETTE_EFFECT:
			// Viñeta que pulsa sutilmente
			g_nightmareVisual.vignetteIntensity = effect.intensity * 
				(0.8f + 0.2f * std::sin(currentTime * 0.001f));
			break;
			
		case VisualEffectType::COLOR_GRADING:
			// Corrección de color para atmósfera más oscura
			g_nightmareVisual.colorGradingR = 0.9f * effect.intensity;
			g_nightmareVisual.colorGradingG = 0.85f * effect.intensity;
			g_nightmareVisual.colorGradingB = 0.8f * effect.intensity;
			break;
			
		case VisualEffectType::SCREEN_DISTORTION:
			// Distorsión sutil de pantalla
			g_nightmareVisual.screenDistortion = effect.intensity * 
				std::sin(currentTime * 0.002f) * 0.01f;
			break;
			
		default:
			break;
		}
	}
	
	// Actualizar partículas
	UpdateParticles(deltaTime);
	
	// Crear nuevas partículas ambientales ocasionalmente
	if (g_nightmareVisual.config.particleEffects && (rand() % 100) < 2) {
		CreateParticles(VisualEffectType::PARTICLE_DUST, 
			rand() % 800, rand() % 600, 1);
	}
}

void UpdateParticles(float deltaTime)
{
	for (auto &particle : g_nightmareVisual.particles) {
		if (!particle.active) {
			continue;
		}
		
		// Actualizar posición
		particle.x += particle.vx * deltaTime;
		particle.y += particle.vy * deltaTime;
		
		// Actualizar vida
		particle.life -= deltaTime * 0.5f; // Las partículas duran ~2 segundos
		
		// Aplicar efectos específicos según el tipo
		switch (particle.type) {
		case VisualEffectType::PARTICLE_DUST:
			// Polvo que flota lentamente hacia abajo
			particle.vy += 10.0f * deltaTime; // Gravedad suave
			particle.vx *= 0.99f; // Fricción del aire
			break;
			
		case VisualEffectType::PARTICLE_EMBERS:
			// Brasas que suben y se desvanecen
			particle.vy -= 20.0f * deltaTime; // Flotan hacia arriba
			particle.vx += (rand() % 20 - 10) * deltaTime * 0.1f; // Movimiento aleatorio
			break;
			
		case VisualEffectType::PARTICLE_MIST:
			// Niebla que se expande lentamente
			particle.size += deltaTime * 0.5f;
			particle.vx *= 0.95f;
			particle.vy *= 0.95f;
			break;
			
		default:
			break;
		}
		
		// Desactivar partícula si se acabó su vida o salió de pantalla
		if (particle.life <= 0.0f || particle.x < -50 || particle.x > 850 || 
		    particle.y < -50 || particle.y > 650) {
			particle.active = false;
			g_nightmareVisual.activeParticleCount--;
		}
	}
}

void RenderNightmareVisualEffects()
{
	if (!g_nightmareVisual.initialized || !g_nightmareVisual.config.enabled) {
		return;
	}
	
	// En una implementación completa, aquí renderizaríamos los efectos visuales
	// Por ahora, solo registramos que se está renderizando
	
	static uint32_t lastLogTime = 0;
	uint32_t currentTime = SDL_GetTicks();
	
	// Log cada 5 segundos para no saturar
	if (currentTime - lastLogTime > 5000) {
		if (g_nightmareVisual.activeEffectCount > 0 || g_nightmareVisual.activeParticleCount > 0) {
			LogVerbose("🎨 Rendering visual effects: {} effects, {} particles", 
				g_nightmareVisual.activeEffectCount, g_nightmareVisual.activeParticleCount);
		}
		lastLogTime = currentTime;
	}
}

void ActivateVisualEffect(VisualEffectType type, float intensity, float duration)
{
	if (!g_nightmareVisual.initialized || !g_nightmareVisual.config.enabled) {
		return;
	}
	
	// Buscar slot libre o reutilizar efecto existente del mismo tipo
	NightmareVisualEffect *targetEffect = nullptr;
	
	// Primero buscar si ya existe un efecto del mismo tipo
	for (auto &effect : g_nightmareVisual.effects) {
		if (effect.type == type) {
			targetEffect = &effect;
			break;
		}
	}
	
	// Si no existe, buscar slot libre
	if (!targetEffect) {
		for (auto &effect : g_nightmareVisual.effects) {
			if (effect.type == VisualEffectType::NONE || !effect.isActive) {
				targetEffect = &effect;
				break;
			}
		}
	}
	
	if (!targetEffect) {
		LogVerbose("⚠️ No free slots for visual effect: {}", static_cast<int>(type));
		return;
	}
	
	// Configurar efecto
	targetEffect->type = type;
	targetEffect->intensity = std::clamp(intensity * g_nightmareVisual.config.masterIntensity, 0.0f, 1.0f);
	targetEffect->duration = duration;
	targetEffect->startTime = SDL_GetTicks();
	targetEffect->enabled = true;
	targetEffect->isActive = true;
	
	// Actualizar contador si es un efecto nuevo
	bool isNewEffect = true;
	for (int i = 0; i < g_nightmareVisual.activeEffectCount; i++) {
		if (&g_nightmareVisual.effects[i] == targetEffect) {
			isNewEffect = false;
			break;
		}
	}
	
	if (isNewEffect && g_nightmareVisual.activeEffectCount < static_cast<int>(g_nightmareVisual.effects.size())) {
		g_nightmareVisual.activeEffectCount++;
	}
	
	LogVerbose("✨ Activated visual effect: type={}, intensity={:.2f}, duration={:.1f}s", 
		static_cast<int>(type), targetEffect->intensity, duration);
}

void DeactivateVisualEffect(VisualEffectType type)
{
	for (auto &effect : g_nightmareVisual.effects) {
		if (effect.type == type && effect.isActive) {
			effect.isActive = false;
			LogVerbose("✨ Deactivated visual effect: type={}", static_cast<int>(type));
			break;
		}
	}
}

void CreateParticles(VisualEffectType type, float x, float y, int count)
{
	if (!g_nightmareVisual.initialized || !g_nightmareVisual.config.enabled || 
	    !g_nightmareVisual.config.particleEffects) {
		return;
	}
	
	int created = 0;
	for (auto &particle : g_nightmareVisual.particles) {
		if (created >= count) {
			break;
		}
		
		if (!particle.active) {
			// Configurar partícula
			particle.x = x + (rand() % 20 - 10); // Variación de posición
			particle.y = y + (rand() % 20 - 10);
			particle.life = 1.0f;
			particle.type = type;
			particle.active = true;
			
			// Configurar parámetros específicos según el tipo
			switch (type) {
			case VisualEffectType::PARTICLE_DUST:
				particle.vx = (rand() % 20 - 10) * 0.5f;
				particle.vy = (rand() % 10) * 0.5f;
				particle.size = 1.0f + (rand() % 3);
				particle.color = 200 + (rand() % 55);
				break;
				
			case VisualEffectType::PARTICLE_EMBERS:
				particle.vx = (rand() % 40 - 20) * 0.3f;
				particle.vy = -(rand() % 30 + 10) * 0.5f;
				particle.size = 2.0f + (rand() % 4);
				particle.color = 255;
				break;
				
			case VisualEffectType::PARTICLE_MIST:
				particle.vx = (rand() % 30 - 15) * 0.2f;
				particle.vy = (rand() % 30 - 15) * 0.2f;
				particle.size = 5.0f + (rand() % 10);
				particle.color = 150 + (rand() % 50);
				break;
				
			default:
				particle.vx = 0.0f;
				particle.vy = 0.0f;
				particle.size = 1.0f;
				particle.color = 255;
				break;
			}
			
			created++;
			g_nightmareVisual.activeParticleCount++;
		}
	}
	
	if (created > 0) {
		LogVerbose("💫 Created {} particles of type {} at ({:.1f}, {:.1f})", 
			created, static_cast<int>(type), x, y);
	}
}

void UpdateLevelVisualEffects(int levelType)
{
	if (!g_nightmareVisual.initialized || !g_nightmareVisual.config.enabled) {
		return;
	}
	
	// Desactivar efectos específicos de nivel anterior
	DeactivateVisualEffect(VisualEffectType::PARTICLE_EMBERS);
	DeactivateVisualEffect(VisualEffectType::PARTICLE_MIST);
	
	// Configurar efectos según el tipo de nivel
	switch (levelType) {
	case 0: // Town
		// Efectos suaves en el pueblo
		ActivateVisualEffect(VisualEffectType::PARTICLE_DUST, 0.1f);
		LogVerbose("🏘️ Town visual effects: light dust");
		break;
		
	case 1: // Cathedral
		// Rayos de luz y polvo
		ActivateVisualEffect(VisualEffectType::LIGHT_RAYS, 0.3f);
		ActivateVisualEffect(VisualEffectType::PARTICLE_DUST, 0.2f);
		LogVerbose("⛪ Cathedral visual effects: light rays + dust");
		break;
		
	case 2: // Catacombs
		// Niebla y efectos de sombra
		ActivateVisualEffect(VisualEffectType::PARTICLE_MIST, 0.2f);
		ActivateVisualEffect(VisualEffectType::SHADOW_ENHANCEMENT, 0.4f);
		LogVerbose("🦴 Catacombs visual effects: mist + shadows");
		break;
		
	case 3: // Caves
		// Niebla densa y goteos
		ActivateVisualEffect(VisualEffectType::PARTICLE_MIST, 0.3f);
		LogVerbose("🕳️ Caves visual effects: dense mist");
		break;
		
	case 4: // Hell
		// Brasas y distorsión
		ActivateVisualEffect(VisualEffectType::PARTICLE_EMBERS, 0.4f);
		ActivateVisualEffect(VisualEffectType::SCREEN_DISTORTION, 0.1f);
		LogVerbose("🔥 Hell visual effects: embers + distortion");
		break;
		
	default:
		LogVerbose("✨ Unknown level type for visual effects: {}", levelType);
		break;
	}
}

void SetNightmareVisualEffectsEnabled(bool enabled)
{
	g_nightmareVisual.config.enabled = enabled;
	LogVerbose("✨ Nightmare Visual Effects manually set to: {}", enabled ? "ENABLED" : "DISABLED");
}

void SetVisualEffectsIntensity(float intensity)
{
	g_nightmareVisual.config.masterIntensity = std::clamp(intensity, 0.0f, 1.0f);
	LogVerbose("🎨 Visual Effects intensity set to: {:.2f}", g_nightmareVisual.config.masterIntensity);
}

void CleanupNightmareVisualEffects()
{
	LogVerbose("✨ Cleaning up Nightmare Visual Effects System");
	
	g_nightmareVisual.initialized = false;
	g_nightmareVisual.activeEffectCount = 0;
	g_nightmareVisual.activeParticleCount = 0;
	
	for (auto &effect : g_nightmareVisual.effects) {
		effect.type = VisualEffectType::NONE;
		effect.enabled = false;
		effect.isActive = false;
	}
	
	for (auto &particle : g_nightmareVisual.particles) {
		particle.active = false;
	}
}

} // namespace devilution