/**
 * @file contextual_palette.h
 * 
 * 🎨 FASE V2 - PALETA CONTEXTUAL 🎨
 * 
 * Sistema de paletas contextuales que se adaptan dinámicamente al entorno del juego
 * para crear mayor inmersión y profundidad visual.
 */

#pragma once

#include <cstdint>

#ifdef USE_SDL3
#include <SDL3/SDL_pixels.h>
#else
#include <SDL.h>
#endif

namespace devilution {

// ============================================================================
// 🎨 SISTEMA DE PALETA CONTEXTUAL
// ============================================================================

/**
 * Tipos de biomas para paletas contextuales
 */
enum class BiomeType {
    Town,           // Pueblo - Paleta cálida y acogedora
    Cathedral,      // Catedral - Paleta fría con tintes rojos de corrupción
    Catacombs,      // Catacumbas - Paleta azul-gris, húmeda y sombría
    Caves,          // Cuevas - Paleta verde-amarilla, natural pero corrupta
    Hell            // Infierno - Paleta roja intensa, fuego y azufre
};

/**
 * Configuración de tinte contextual
 */
struct ContextualTint {
    float redMultiplier = 1.0f;
    float greenMultiplier = 1.0f;
    float blueMultiplier = 1.0f;
    float saturationBoost = 0.0f;
    float contrastBoost = 0.0f;
    float darknessLevel = 0.0f;
};

/**
 * Estado del sistema de paleta contextual
 */
struct ContextualPaletteState {
    BiomeType currentBiome = BiomeType::Town;
    ContextualTint currentTint;
    ContextualTint targetTint;
    
    // Transiciones suaves
    float transitionProgress = 1.0f;
    uint32_t transitionStartTime = 0;
    uint32_t transitionDuration = 2000; // 2 segundos por defecto
    
    // Configuración
    bool enabled = true;
    float globalIntensity = 0.8f;
    bool smoothTransitions = true;
    
    // Oscurecimiento por profundidad
    bool depthDarkening = true;
    float depthDarkeningFactor = 0.05f; // 5% más oscuro por nivel
};

// ============================================================================
// 🎯 API PRINCIPAL DEL SISTEMA
// ============================================================================

/**
 * Inicializa el sistema de paleta contextual
 */
void InitContextualPalette();

/**
 * Actualiza el sistema de paleta contextual (llamar una vez por frame)
 */
void UpdateContextualPalette();

/**
 * Limpia el sistema de paleta contextual
 */
void CleanupContextualPalette();

/**
 * Habilita/deshabilita el sistema de paleta contextual
 */
void SetContextualPaletteEnabled(bool enabled);

/**
 * Establece la intensidad global del sistema
 */
void SetContextualPaletteIntensity(float intensity);

// ============================================================================
// 🎨 FUNCIONES DE BIOMA Y TINTE
// ============================================================================

/**
 * 🎯 V2.1 - TINTE POR BIOMA
 * Cambia la paleta según el tipo de nivel actual
 */
void SetBiomePalette(BiomeType biome, bool smoothTransition = true);

/**
 * 🎯 V2.2 - OSCURECIMIENTO POR PROFUNDIDAD
 * Oscurece la paleta según la profundidad del dungeon
 */
void ApplyDepthDarkening(int currentLevel);

/**
 * 🎯 V2.3 - PALETA DE CORRUPCIÓN
 * Aplica efectos visuales de corrupción demoníaca
 */
void ApplyCorruptionTint(float corruptionLevel);

/**
 * 🎯 V2.4 - TRANSICIONES SUAVES
 * Gestiona transiciones graduales entre paletas
 */
void UpdatePaletteTransition();

// ============================================================================
// 🎨 APLICACIÓN DE PALETAS
// ============================================================================

/**
 * Aplica el tinte contextual a una paleta SDL
 */
void ApplyContextualTint(SDL_Color *palette, const ContextualTint &tint);

/**
 * Mezcla dos tintes con interpolación
 */
ContextualTint BlendTints(const ContextualTint &from, const ContextualTint &to, float progress);

/**
 * Aplica la paleta contextual completa al sistema
 */
void ApplyContextualPalette(SDL_Color *palette);

// ============================================================================
// 🔧 PRESETS DE BIOMAS
// ============================================================================

/**
 * Obtiene el tinte predefinido para un bioma
 */
ContextualTint GetBiomeTint(BiomeType biome);

/**
 * Detecta automáticamente el bioma basado en el nivel actual
 */
BiomeType DetectCurrentBiome();

// ============================================================================
// 🎨 PRESETS DE CONFIGURACIÓN
// ============================================================================

/**
 * Preset sutil - Cambios mínimos de paleta
 */
void ApplySubtlePalettePreset();

/**
 * Preset balanceado - Cambios moderados (recomendado)
 */
void ApplyBalancedPalettePreset();

/**
 * Preset dramático - Cambios intensos de paleta
 */
void ApplyDramaticPalettePreset();

/**
 * Preset para debugging - Cambios muy visibles
 */
void ApplyDebugPalettePreset();

// ============================================================================
// 📊 ESTADÍSTICAS Y MÉTRICAS
// ============================================================================

/**
 * Estadísticas del sistema de paleta contextual
 */
struct ContextualPaletteStats {
    uint32_t totalTransitions = 0;
    uint32_t currentBiomeTime = 0;
    BiomeType mostUsedBiome = BiomeType::Town;
    float averageIntensity = 0.0f;
    uint32_t averageProcessingTimeUs = 0;
};

/**
 * Obtiene estadísticas del sistema
 */
ContextualPaletteStats GetContextualPaletteStats();

/**
 * Resetea estadísticas del sistema
 */
void ResetContextualPaletteStats();

// ============================================================================
// 🔍 DEBUGGING Y DIAGNÓSTICO
// ============================================================================

#ifdef _DEBUG
/**
 * Imprime estadísticas del sistema de paleta contextual
 */
void DebugPrintContextualPaletteStats();

/**
 * Habilita/deshabilita logging detallado
 */
void SetContextualPaletteDebugLogging(bool enabled);

/**
 * Fuerza un bioma específico para testing
 */
void ForceDebugBiome(BiomeType biome);
#endif

} // namespace devilution