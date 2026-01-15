/**
 * @file contextual_palette.cpp
 * 
 * 🎨 FASE V2 - PALETA CONTEXTUAL 🎨
 * 
 * Implementación completa del sistema de paletas contextuales
 */

#include "contextual_palette.h"

#include <algorithm>
#include <cmath>
#include <chrono>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "levels/gendung.h"
#include "engine/random.hpp"
#include "interfac.h"  // 🛡️ For g_isLevelTransition

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

namespace devilution {

// ============================================================================
// 🔒 ESTADO GLOBAL DEL SISTEMA
// ============================================================================

namespace {
    ContextualPaletteState g_paletteState;
    ContextualPaletteStats g_paletteStats;
    
#ifdef _DEBUG
    bool g_debugLogging = false;
    BiomeType g_forcedBiome = BiomeType::Town;
    bool g_biomeForced = false;
#endif
}

// ============================================================================
// 🎯 IMPLEMENTACIÓN DE LA API PRINCIPAL
// ============================================================================

void InitContextualPalette()
{
    // Inicializar estado
    g_paletteState = {};
    g_paletteStats = {};
    
    // Configuración por defecto (preset balanceado)
    ApplyBalancedPalettePreset();
    
    // Detectar bioma inicial
    g_paletteState.currentBiome = DetectCurrentBiome();
    g_paletteState.currentTint = GetBiomeTint(g_paletteState.currentBiome);
    g_paletteState.targetTint = g_paletteState.currentTint;
    
#ifdef _DEBUG
    std::cout << "Contextual Palette System initialized" << std::endl;
#endif
}

void UpdateContextualPalette()
{
    if (!g_paletteState.enabled) {
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Detectar cambios de bioma
    BiomeType detectedBiome = DetectCurrentBiome();
    
#ifdef _DEBUG
    if (g_biomeForced) {
        detectedBiome = g_forcedBiome;
    }
#endif
    
    if (detectedBiome != g_paletteState.currentBiome) {
        SetBiomePalette(detectedBiome, g_paletteState.smoothTransitions);
        g_paletteStats.totalTransitions++;
        
#ifdef _DEBUG
        if (g_debugLogging) {
            std::cout << "Biome transition: " << static_cast<int>(g_paletteState.currentBiome) 
                      << " -> " << static_cast<int>(detectedBiome) << std::endl;
        }
#endif
    }
    
    // Actualizar transiciones suaves
    if (g_paletteState.smoothTransitions) {
        UpdatePaletteTransition();
    }
    
    // Aplicar oscurecimiento por profundidad
    if (g_paletteState.depthDarkening) {
        ApplyDepthDarkening(currlevel);
    }
    
    // Actualizar estadísticas
    g_paletteStats.currentBiomeTime = SDL_GetTicks();
    
    // Métricas de performance
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    g_paletteStats.averageProcessingTimeUs = static_cast<uint32_t>(duration.count());
}

void CleanupContextualPalette()
{
    g_paletteState = {};
    
#ifdef _DEBUG
    std::cout << "Contextual Palette System cleaned up" << std::endl;
#endif
}

void ResetContextualPaletteState()
{
    // 🛡️ PALETTE CORRUPTION FIX: Reset internal state for clean transitions
    // This prevents accumulated tints from corrupting the palette after level changes
    
    // Reset current and target tints to neutral (no modification)
    g_paletteState.currentTint = ContextualTint{}; // All multipliers = 1.0, all boosts = 0.0
    g_paletteState.targetTint = ContextualTint{};
    
    // Reset transition state
    g_paletteState.transitionProgress = 1.0f; // No transition in progress
    g_paletteState.transitionStartTime = 0;
    
    // Re-detect biome for the new level
    g_paletteState.currentBiome = DetectCurrentBiome();
    
    // Apply the correct tint for the new biome (will be applied gradually)
    g_paletteState.targetTint = GetBiomeTint(g_paletteState.currentBiome);
    
#ifdef _DEBUG
    std::cout << "Contextual Palette state reset for new level transition" << std::endl;
#endif
}

void SetContextualPaletteEnabled(bool enabled)
{
    g_paletteState.enabled = enabled;
    
#ifdef _DEBUG
    std::cout << "Contextual Palette " << (enabled ? "enabled" : "disabled") << std::endl;
#endif
}

void SetContextualPaletteIntensity(float intensity)
{
    g_paletteState.globalIntensity = std::clamp(intensity, 0.0f, 2.0f);
    
#ifdef _DEBUG
    std::cout << "Contextual Palette intensity set to " << intensity << std::endl;
#endif
}

// ============================================================================
// 🎨 FUNCIONES DE BIOMA Y TINTE
// ============================================================================

void SetBiomePalette(BiomeType biome, bool smoothTransition)
{
    if (!g_paletteState.enabled) return;
    
    g_paletteState.currentBiome = biome;
    g_paletteState.targetTint = GetBiomeTint(biome);
    
    if (smoothTransition && g_paletteState.smoothTransitions) {
        // Iniciar transición suave
        g_paletteState.transitionProgress = 0.0f;
        g_paletteState.transitionStartTime = SDL_GetTicks();
    } else {
        // Cambio inmediato
        g_paletteState.currentTint = g_paletteState.targetTint;
        g_paletteState.transitionProgress = 1.0f;
    }
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Biome palette set to " << static_cast<int>(biome) 
                  << ", smooth=" << (smoothTransition ? "yes" : "no") << std::endl;
    }
#endif
}

void ApplyDepthDarkening(int currentLevel)
{
    if (!g_paletteState.depthDarkening || currentLevel <= 0) return;
    
    // Calcular factor de oscurecimiento basado en la profundidad
    float darkeningFactor = currentLevel * g_paletteState.depthDarkeningFactor;
    darkeningFactor = std::min(darkeningFactor, 0.5f); // Máximo 50% de oscurecimiento
    
    // Aplicar oscurecimiento al tinte actual
    g_paletteState.currentTint.darknessLevel = darkeningFactor * g_paletteState.globalIntensity;
    
#ifdef _DEBUG
    if (g_debugLogging && currentLevel % 5 == 0) { // Log cada 5 niveles
        std::cout << "Depth darkening applied: level=" << currentLevel 
                  << ", darkness=" << g_paletteState.currentTint.darknessLevel << std::endl;
    }
#endif
}

void ApplyCorruptionTint(float corruptionLevel)
{
    if (!g_paletteState.enabled) return;
    
    corruptionLevel = std::clamp(corruptionLevel, 0.0f, 1.0f);
    
    // Aplicar tinte de corrupción (rojo-marrón)
    float corruptionIntensity = corruptionLevel * g_paletteState.globalIntensity;
    
    g_paletteState.currentTint.redMultiplier += corruptionIntensity * 0.3f;
    g_paletteState.currentTint.greenMultiplier -= corruptionIntensity * 0.1f;
    g_paletteState.currentTint.blueMultiplier -= corruptionIntensity * 0.2f;
    g_paletteState.currentTint.saturationBoost += corruptionIntensity * 0.2f;
    
    // Clamp valores
    g_paletteState.currentTint.redMultiplier = std::clamp(g_paletteState.currentTint.redMultiplier, 0.5f, 1.5f);
    g_paletteState.currentTint.greenMultiplier = std::clamp(g_paletteState.currentTint.greenMultiplier, 0.3f, 1.2f);
    g_paletteState.currentTint.blueMultiplier = std::clamp(g_paletteState.currentTint.blueMultiplier, 0.3f, 1.2f);
}

void UpdatePaletteTransition()
{
    if (g_paletteState.transitionProgress >= 1.0f) {
        return; // Transición completada
    }
    
    uint32_t currentTime = SDL_GetTicks();
    uint32_t elapsed = currentTime - g_paletteState.transitionStartTime;
    
    // Calcular progreso de la transición (0.0 a 1.0)
    g_paletteState.transitionProgress = static_cast<float>(elapsed) / g_paletteState.transitionDuration;
    g_paletteState.transitionProgress = std::min(g_paletteState.transitionProgress, 1.0f);
    
    // Aplicar curva suave (ease-in-out)
    float smoothProgress = g_paletteState.transitionProgress;
    smoothProgress = smoothProgress * smoothProgress * (3.0f - 2.0f * smoothProgress);
    
    // Interpolar entre tinte actual y objetivo
    g_paletteState.currentTint = BlendTints(g_paletteState.currentTint, g_paletteState.targetTint, smoothProgress);
}

// ============================================================================
// 🎨 APLICACIÓN DE PALETAS
// ============================================================================

void ApplyContextualTint(SDL_Color *palette, const ContextualTint &tint)
{
    if (palette == nullptr) return;
    
    for (int i = 0; i < 256; i++) {
        SDL_Color &color = palette[i];
        
        // Aplicar multiplicadores de color
        float r = color.r * tint.redMultiplier;
        float g = color.g * tint.greenMultiplier;
        float b = color.b * tint.blueMultiplier;
        
        // Aplicar boost de saturación
        if (tint.saturationBoost > 0.0f) {
            float gray = (r + g + b) / 3.0f;
            r = gray + (r - gray) * (1.0f + tint.saturationBoost);
            g = gray + (g - gray) * (1.0f + tint.saturationBoost);
            b = gray + (b - gray) * (1.0f + tint.saturationBoost);
        }
        
        // Aplicar boost de contraste
        if (tint.contrastBoost > 0.0f) {
            r = 128.0f + (r - 128.0f) * (1.0f + tint.contrastBoost);
            g = 128.0f + (g - 128.0f) * (1.0f + tint.contrastBoost);
            b = 128.0f + (b - 128.0f) * (1.0f + tint.contrastBoost);
        }
        
        // Aplicar oscurecimiento
        if (tint.darknessLevel > 0.0f) {
            float darkeningFactor = 1.0f - tint.darknessLevel;
            r *= darkeningFactor;
            g *= darkeningFactor;
            b *= darkeningFactor;
        }
        
        // Clamp y asignar valores finales
        color.r = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
        color.g = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
        color.b = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
    }
}

ContextualTint BlendTints(const ContextualTint &from, const ContextualTint &to, float progress)
{
    ContextualTint result;
    
    result.redMultiplier = from.redMultiplier + (to.redMultiplier - from.redMultiplier) * progress;
    result.greenMultiplier = from.greenMultiplier + (to.greenMultiplier - from.greenMultiplier) * progress;
    result.blueMultiplier = from.blueMultiplier + (to.blueMultiplier - from.blueMultiplier) * progress;
    result.saturationBoost = from.saturationBoost + (to.saturationBoost - from.saturationBoost) * progress;
    result.contrastBoost = from.contrastBoost + (to.contrastBoost - from.contrastBoost) * progress;
    result.darknessLevel = from.darknessLevel + (to.darknessLevel - from.darknessLevel) * progress;
    
    return result;
}

/**
 * Aplica la paleta contextual completa al sistema
 */
void ApplyContextualPalette(SDL_Color *palette)
{
    // 🛡️ TRANSITION SAFETY: Skip during level transitions
    if (g_isLevelTransition) {
        return;
    }
    
    if (!g_paletteState.enabled || palette == nullptr) {
        return;
    }
    
    ApplyContextualTint(palette, g_paletteState.currentTint);
}

// ============================================================================
// 🔧 PRESETS DE BIOMAS
// ============================================================================

ContextualTint GetBiomeTint(BiomeType biome)
{
    ContextualTint tint;
    
    switch (biome) {
        case BiomeType::Town:
            // Paleta cálida y acogedora
            tint.redMultiplier = 1.1f;
            tint.greenMultiplier = 1.05f;
            tint.blueMultiplier = 0.9f;
            tint.saturationBoost = 0.1f;
            tint.contrastBoost = 0.05f;
            break;
            
        case BiomeType::Cathedral:
            // Paleta fría con tintes rojos de corrupción
            tint.redMultiplier = 1.2f;
            tint.greenMultiplier = 0.8f;
            tint.blueMultiplier = 0.7f;
            tint.saturationBoost = 0.15f;
            tint.contrastBoost = 0.1f;
            tint.darknessLevel = 0.1f;
            break;
            
        case BiomeType::Catacombs:
            // Paleta azul-gris, húmeda y sombría
            tint.redMultiplier = 0.7f;
            tint.greenMultiplier = 0.8f;
            tint.blueMultiplier = 1.3f;
            tint.saturationBoost = -0.1f; // Desaturar ligeramente
            tint.contrastBoost = 0.15f;
            tint.darknessLevel = 0.15f;
            break;
            
        case BiomeType::Caves:
            // Paleta verde-amarilla, natural pero corrupta
            tint.redMultiplier = 0.9f;
            tint.greenMultiplier = 1.2f;
            tint.blueMultiplier = 0.6f;
            tint.saturationBoost = 0.2f;
            tint.contrastBoost = 0.1f;
            tint.darknessLevel = 0.2f;
            break;
            
        case BiomeType::Hell:
            // Paleta roja intensa, fuego y azufre
            tint.redMultiplier = 1.5f;
            tint.greenMultiplier = 0.6f;
            tint.blueMultiplier = 0.4f;
            tint.saturationBoost = 0.3f;
            tint.contrastBoost = 0.2f;
            tint.darknessLevel = 0.1f;
            break;
    }
    
    return tint;
}

BiomeType DetectCurrentBiome()
{
    // Detectar bioma basado en el tipo de nivel actual
    if (currlevel == 0) {
        return BiomeType::Town;
    }
    
    switch (leveltype) {
        case dungeon_type::DTYPE_TOWN:
            return BiomeType::Town;
            
        case dungeon_type::DTYPE_CATHEDRAL:
            return BiomeType::Cathedral;
            
        case dungeon_type::DTYPE_CATACOMBS:
            return BiomeType::Catacombs;
            
        case dungeon_type::DTYPE_CAVES:
            return BiomeType::Caves;
            
        case dungeon_type::DTYPE_HELL:
            return BiomeType::Hell;
            
        default:
            // Fallback basado en nivel
            if (currlevel <= 4) return BiomeType::Cathedral;
            if (currlevel <= 8) return BiomeType::Catacombs;
            if (currlevel <= 12) return BiomeType::Caves;
            return BiomeType::Hell;
    }
}

// ============================================================================
// 🎨 PRESETS DE CONFIGURACIÓN
// ============================================================================

void ApplySubtlePalettePreset()
{
    g_paletteState.enabled = true;
    g_paletteState.globalIntensity = 0.4f;
    g_paletteState.smoothTransitions = true;
    g_paletteState.transitionDuration = 3000; // 3 segundos
    g_paletteState.depthDarkening = true;
    g_paletteState.depthDarkeningFactor = 0.02f; // 2% por nivel
    
#ifdef _DEBUG
    std::cout << "Subtle palette preset applied" << std::endl;
#endif
}

void ApplyBalancedPalettePreset()
{
    g_paletteState.enabled = true;
    g_paletteState.globalIntensity = 0.7f;
    g_paletteState.smoothTransitions = true;
    g_paletteState.transitionDuration = 2000;
    g_paletteState.depthDarkening = true;
    g_paletteState.depthDarkeningFactor = 0.04f;
    
#ifdef _DEBUG
    std::cout << "Balanced palette preset applied" << std::endl;
#endif
}

void ApplyDramaticPalettePreset()
{
    g_paletteState.enabled = true;
    g_paletteState.globalIntensity = 1.2f;
    g_paletteState.smoothTransitions = true;
    g_paletteState.transitionDuration = 1500; // 1.5 segundos
    g_paletteState.depthDarkening = true;
    g_paletteState.depthDarkeningFactor = 0.06f; // 6% por nivel
    
#ifdef _DEBUG
    std::cout << "Dramatic palette preset applied" << std::endl;
#endif
}

void ApplyDebugPalettePreset()
{
    ApplyDramaticPalettePreset();
    g_paletteState.transitionDuration = 500; // Transiciones rápidas para debug
    
#ifdef _DEBUG
    g_debugLogging = true;
    std::cout << "Debug palette preset applied with logging enabled" << std::endl;
#endif
}

// ============================================================================
// 📊 ESTADÍSTICAS Y MÉTRICAS
// ============================================================================

ContextualPaletteStats GetContextualPaletteStats()
{
    return g_paletteStats;
}

void ResetContextualPaletteStats()
{
    g_paletteStats = {};
    
#ifdef _DEBUG
    std::cout << "Contextual palette stats reset" << std::endl;
#endif
}

// ============================================================================
// 🔍 DEBUGGING Y DIAGNÓSTICO
// ============================================================================

#ifdef _DEBUG
void DebugPrintContextualPaletteStats()
{
    std::cout << "\n=== CONTEXTUAL PALETTE STATISTICS ===" << std::endl;
    std::cout << "System enabled: " << (g_paletteState.enabled ? "YES" : "NO") << std::endl;
    std::cout << "Global intensity: " << g_paletteState.globalIntensity << std::endl;
    std::cout << "Current biome: " << static_cast<int>(g_paletteState.currentBiome) << std::endl;
    
    std::cout << "\n--- TRANSITIONS ---" << std::endl;
    std::cout << "Total transitions: " << g_paletteStats.totalTransitions << std::endl;
    std::cout << "Transition progress: " << g_paletteState.transitionProgress << std::endl;
    std::cout << "Smooth transitions: " << (g_paletteState.smoothTransitions ? "YES" : "NO") << std::endl;
    
    std::cout << "\n--- CURRENT TINT ---" << std::endl;
    std::cout << "Red multiplier: " << std::fixed << std::setprecision(3) 
              << g_paletteState.currentTint.redMultiplier << std::endl;
    std::cout << "Green multiplier: " << g_paletteState.currentTint.greenMultiplier << std::endl;
    std::cout << "Blue multiplier: " << g_paletteState.currentTint.blueMultiplier << std::endl;
    std::cout << "Darkness level: " << g_paletteState.currentTint.darknessLevel << std::endl;
    
    std::cout << "\n--- PERFORMANCE ---" << std::endl;
    std::cout << "Processing time: " << g_paletteStats.averageProcessingTimeUs << "μs" << std::endl;
    
    std::cout << "=======================================" << std::endl;
}

void SetContextualPaletteDebugLogging(bool enabled)
{
    g_debugLogging = enabled;
    std::cout << "Contextual palette debug logging " << (enabled ? "enabled" : "disabled") << std::endl;
}

void ForceDebugBiome(BiomeType biome)
{
    g_forcedBiome = biome;
    g_biomeForced = true;
    std::cout << "Debug biome forced to " << static_cast<int>(biome) << std::endl;
}
#endif

} // namespace devilution