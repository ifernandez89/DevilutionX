/**
 * @file life_volume.cpp
 * 
 * 🎨 FASE D2 - VIDA Y VOLUMEN 🎨
 * 
 * Implementación completa del sistema de vida y volumen
 */

#include "life_volume.h"

#include <algorithm>
#include <cmath>
#include <chrono>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "engine/random.hpp"
// #include "levels/gendung.h"  // Removed to avoid dependency issues

// Forward declarations to avoid heavy includes
extern int leveltype;
struct Player;
struct Monster;

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

namespace devilution {

// ============================================================================
// 🔒 ESTADO GLOBAL DEL SISTEMA
// ============================================================================

namespace {
    LifeVolumeState g_lifeVolumeState;
    LifeVolumeStats g_lifeVolumeStats;
    
#ifdef _DEBUG
    bool g_debugLogging = false;
    LifeAnimationType g_forcedAnimationType = LifeAnimationType::None;
    bool g_animationForced = false;
#endif
}

// ============================================================================
// 🎯 IMPLEMENTACIÓN DE LA API PRINCIPAL
// ============================================================================

void InitLifeVolume()
{
    // Inicializar estado
    g_lifeVolumeState = {};
    g_lifeVolumeStats = {};
    
    // Configuración por defecto (preset balanceado)
    ApplyBalancedLifeVolumePreset();
    
#ifdef _DEBUG
    std::cout << "Life & Volume System initialized" << std::endl;
#endif
}

void UpdateLifeVolume()
{
    if (!g_lifeVolumeState.enabled) {
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Reset contadores de frame
    g_lifeVolumeState.activeOffsets = 0;
    g_lifeVolumeState.activeLightCalculations = 0;
    g_lifeVolumeState.totalCalculationsThisFrame = 0;
    
    // Actualizar estadísticas
    g_lifeVolumeStats.framesProcessed++;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    // Actualizar tiempo promedio de cálculo
    float frameTime = duration.count() / 1000.0f; // Convert to milliseconds
    g_lifeVolumeStats.averageCalculationTime = 
        (g_lifeVolumeStats.averageCalculationTime * 0.95f) + (frameTime * 0.05f);
    
#ifdef _DEBUG
    if (g_debugLogging && g_lifeVolumeStats.framesProcessed % 60 == 0) {
        std::cout << "Life&Volume: " << g_lifeVolumeState.activeOffsets << " offsets, "
                  << g_lifeVolumeState.activeLightCalculations << " lights, "
                  << std::fixed << std::setprecision(2) << frameTime << "ms" << std::endl;
    }
#endif
}

// ============================================================================
// 🎯 D2.1 - MICRO-OFFSETS DE RENDER
// ============================================================================

LifeOffset CalculatePlayerLifeOffset(const Player &player)
{
    if (!g_lifeVolumeState.microOffsetsEnabled) {
        return {};
    }
    
    if (g_lifeVolumeState.activeOffsets >= LifeVolumeState::MAX_ACTIVE_OFFSETS) {
        return {};
    }
    
    LifeOffset result;
    result.isActive = true;
    g_lifeVolumeState.activeOffsets++;
    g_lifeVolumeStats.totalOffsetsCalculated++;
    
    uint32_t currentTime = SDL_GetTicks();
    const LifeOffsetConfig &config = g_lifeVolumeState.defaultPlayerConfig;
    
    // Simplified animation type determination (without accessing player internals)
    LifeAnimationType animType = LifeAnimationType::IdleBreathing;
    
    // Calcular offset basado en tipo de animación
    float timeSeconds = currentTime / 1000.0f;
    float intensity = config.intensity;
    float frequency = config.frequency;
    
    // Respiración sutil (movimiento vertical suave)
    float breathCycle = std::sin(timeSeconds * frequency * 2.0f) * intensity;
    result.offset.y = static_cast<int>(breathCycle * config.breathingAmplitude);
    result.alpha = 1.0f + (breathCycle * 0.02f); // Muy sutil
    
    // Limitar offsets para evitar efectos extremos
    result.offset.x = std::clamp(result.offset.x, -3, 3);
    result.offset.y = std::clamp(result.offset.y, -2, 2);
    result.alpha = std::clamp(result.alpha, 0.9f, 1.1f);
    
    return result;
}

LifeOffset CalculateMonsterLifeOffset(const Monster &monster)
{
    if (!g_lifeVolumeState.microOffsetsEnabled) {
        return {};
    }
    
    if (g_lifeVolumeState.activeOffsets >= LifeVolumeState::MAX_ACTIVE_OFFSETS) {
        return {};
    }
    
    LifeOffset result;
    result.isActive = true;
    g_lifeVolumeState.activeOffsets++;
    g_lifeVolumeStats.totalOffsetsCalculated++;
    
    uint32_t currentTime = SDL_GetTicks();
    const LifeOffsetConfig &config = g_lifeVolumeState.defaultMonsterConfig;
    
    // Simplified animation type (without accessing monster internals)
    LifeAnimationType animType = LifeAnimationType::MonsterBreathing;
    
    // Calcular offset basado en tipo de animación
    float timeSeconds = currentTime / 1000.0f;
    float intensity = config.intensity * 0.8f; // Monsters más sutiles que players
    float frequency = config.frequency;
    
    // Añadir variación basada en dirección de memoria para evitar sincronización
    float monsterVariation = (reinterpret_cast<uintptr_t>(&monster) % 100) / 100.0f;
    timeSeconds += monsterVariation * 2.0f;
    
    // Respiración de monstruo (más lenta que player)
    float breathCycle = std::sin(timeSeconds * frequency * 1.5f) * intensity;
    result.offset.y = static_cast<int>(breathCycle * config.breathingAmplitude * 0.7f);
    
    // Limitar offsets para monsters
    result.offset.x = std::clamp(result.offset.x, -2, 2);
    result.offset.y = std::clamp(result.offset.y, -1, 1);
    result.alpha = std::clamp(result.alpha, 0.95f, 1.05f);
    
    return result;
}

// ============================================================================
// 🎯 D2.2 - FEEDBACK DIRECCIONAL DE LUZ
// ============================================================================

LightVolumeResult CalculateLightVolume(Point position, int lightRadius)
{
    if (!g_lifeVolumeState.lightVolumeEnabled) {
        return {};
    }
    
    if (g_lifeVolumeState.activeLightCalculations >= LifeVolumeState::MAX_LIGHT_CALCULATIONS) {
        return {};
    }
    
    LightVolumeResult result;
    g_lifeVolumeState.activeLightCalculations++;
    g_lifeVolumeStats.totalLightCalculations++;
    
    const LightVolumeConfig &config = g_lifeVolumeState.lightConfig;
    
    // Calcular dirección de luz principal (simulada desde arriba-izquierda)
    result.lightDirection = { -1, -1 };
    
    // Calcular multiplicador de luz basado en radio y configuración
    float normalizedRadius = std::min(lightRadius / 15.0f, 1.0f);
    result.lightMultiplier = 1.0f + (normalizedRadius * config.directionalBoost);
    
    // Calcular intensidad de sombra basada en posición relativa
    float shadowFactor = 0.0f;
    
    // Simular sombras basadas en la "altura" simulada (coordenada Y)
    float heightFactor = position.y / 100.0f; // Normalizar altura
    shadowFactor = std::clamp(heightFactor * 0.1f, 0.0f, 0.3f);
    
    result.shadowIntensity = shadowFactor * config.intensity;
    
    // Determinar si tiene efecto volumétrico (basado en radio de luz)
    result.hasVolumetricEffect = lightRadius > 8;
    
    // Aplicar boost de luz ambiente
    result.lightMultiplier += config.ambientBoost;
    
    // Limitar valores para evitar efectos extremos
    result.lightMultiplier = std::clamp(result.lightMultiplier, 0.7f, 1.5f);
    result.shadowIntensity = std::clamp(result.shadowIntensity, 0.0f, 0.4f);
    
    return result;
}

// ============================================================================
// 🎯 FUNCIONES DE APLICACIÓN
// ============================================================================

Point ApplyLifeOffset(Point originalPos, const LifeOffset &offset)
{
    if (!offset.isActive) {
        return originalPos;
    }
    
    return {
        originalPos.x + offset.offset.x,
        originalPos.y + offset.offset.y
    };
}

uint8_t ApplyLightVolume(uint8_t originalColor, const LightVolumeResult &lightResult)
{
    if (!g_lifeVolumeState.lightVolumeEnabled) {
        return originalColor;
    }
    
    float colorFloat = originalColor / 255.0f;
    
    // Aplicar multiplicador de luz
    colorFloat *= lightResult.lightMultiplier;
    
    // Aplicar sombra
    colorFloat *= (1.0f - lightResult.shadowIntensity);
    
    // Convertir de vuelta a uint8_t
    return static_cast<uint8_t>(std::clamp(colorFloat * 255.0f, 0.0f, 255.0f));
}

// ============================================================================
// 🎯 CONFIGURACIÓN Y PRESETS
// ============================================================================

void ApplyBalancedLifeVolumePreset()
{
    g_lifeVolumeState.enabled = true;
    g_lifeVolumeState.microOffsetsEnabled = true;
    g_lifeVolumeState.lightVolumeEnabled = true;
    
    // Configuración balanceada para players
    g_lifeVolumeState.defaultPlayerConfig = {
        .type = LifeAnimationType::IdleBreathing,
        .intensity = 0.8f,
        .frequency = 1.0f,
        .startTime = 0,
        .enabled = true,
        .breathingAmplitude = 0.4f,
        .bobbingAmplitude = 0.8f,
        .tensionAmplitude = 0.25f
    };
    
    // Configuración balanceada para monsters
    g_lifeVolumeState.defaultMonsterConfig = {
        .type = LifeAnimationType::MonsterBreathing,
        .intensity = 0.6f,
        .frequency = 0.8f,
        .startTime = 0,
        .enabled = true,
        .breathingAmplitude = 0.3f,
        .bobbingAmplitude = 0.6f,
        .tensionAmplitude = 0.2f
    };
    
    // Configuración balanceada para luz
    g_lifeVolumeState.lightConfig = {
        .enabled = true,
        .intensity = 0.6f,
        .falloffRate = 1.1f,
        .ambientBoost = 0.08f,
        .directionalBoost = 0.2f,
        .playerLightBoost = 1.0f,
        .monsterLightBoost = 0.7f,
        .npcLightBoost = 0.5f,
        .itemLightBoost = 0.3f
    };
}

void ApplySubtleLifeVolumePreset()
{
    ApplyBalancedLifeVolumePreset();
    
    // Reducir intensidades para efecto más sutil
    g_lifeVolumeState.defaultPlayerConfig.intensity = 0.4f;
    g_lifeVolumeState.defaultMonsterConfig.intensity = 0.3f;
    g_lifeVolumeState.lightConfig.intensity = 0.3f;
    
    g_lifeVolumeState.defaultPlayerConfig.breathingAmplitude = 0.2f;
    g_lifeVolumeState.defaultPlayerConfig.bobbingAmplitude = 0.4f;
    g_lifeVolumeState.defaultMonsterConfig.breathingAmplitude = 0.15f;
    g_lifeVolumeState.defaultMonsterConfig.bobbingAmplitude = 0.3f;
}

void ApplyDramaticLifeVolumePreset()
{
    ApplyBalancedLifeVolumePreset();
    
    // Aumentar intensidades para efecto más dramático
    g_lifeVolumeState.defaultPlayerConfig.intensity = 1.2f;
    g_lifeVolumeState.defaultMonsterConfig.intensity = 1.0f;
    g_lifeVolumeState.lightConfig.intensity = 1.0f;
    
    g_lifeVolumeState.defaultPlayerConfig.breathingAmplitude = 0.6f;
    g_lifeVolumeState.defaultPlayerConfig.bobbingAmplitude = 1.2f;
    g_lifeVolumeState.defaultMonsterConfig.breathingAmplitude = 0.5f;
    g_lifeVolumeState.defaultMonsterConfig.bobbingAmplitude = 1.0f;
}

void ConfigureLifeAnimation(LifeAnimationType type, const LifeOffsetConfig &config)
{
    // Esta función permitiría configurar animaciones específicas
    // Por ahora, aplicamos la configuración a los defaults
    if (type == LifeAnimationType::IdleBreathing || type == LifeAnimationType::WalkingBobbing) {
        g_lifeVolumeState.defaultPlayerConfig = config;
    } else if (type == LifeAnimationType::MonsterBreathing) {
        g_lifeVolumeState.defaultMonsterConfig = config;
    }
}

const LifeVolumeState &GetLifeVolumeState()
{
    return g_lifeVolumeState;
}

const LifeVolumeStats &GetLifeVolumeStats()
{
    return g_lifeVolumeStats;
}

// ============================================================================
// 🎯 DEBUGGING Y UTILIDADES
// ============================================================================

#ifdef _DEBUG
void SetLifeVolumeDebugLogging(bool enabled)
{
    g_debugLogging = enabled;
}

void ForceLifeAnimationType(LifeAnimationType type)
{
    g_forcedAnimationType = type;
    g_animationForced = (type != LifeAnimationType::None);
}

std::string GetLifeVolumeDebugInfo()
{
    std::ostringstream oss;
    oss << "Life&Volume Debug Info:\n";
    oss << "  Enabled: " << (g_lifeVolumeState.enabled ? "Yes" : "No") << "\n";
    oss << "  Active Offsets: " << g_lifeVolumeState.activeOffsets << "/" << LifeVolumeState::MAX_ACTIVE_OFFSETS << "\n";
    oss << "  Active Lights: " << g_lifeVolumeState.activeLightCalculations << "/" << LifeVolumeState::MAX_LIGHT_CALCULATIONS << "\n";
    oss << "  Avg Calc Time: " << std::fixed << std::setprecision(2) << g_lifeVolumeStats.averageCalculationTime << "ms\n";
    oss << "  Total Offsets: " << g_lifeVolumeStats.totalOffsetsCalculated << "\n";
    oss << "  Total Lights: " << g_lifeVolumeStats.totalLightCalculations << "\n";
    oss << "  Frames: " << g_lifeVolumeStats.framesProcessed;
    return oss.str();
}
#endif

} // namespace devilution