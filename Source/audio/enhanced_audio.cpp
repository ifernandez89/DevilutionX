/**
 * @file enhanced_audio.cpp
 * 
 * AUDIO ENHANCEMENT - Enhanced Audio System Implementation
 * 
 * 🔊 IMPLEMENTACIÓN del sistema de audio mejorado con throttling, spatial audio y variación
 */

#include "enhanced_audio.h"
#include "engine/random.hpp"
#include "player.h"

#include <algorithm>
#include <chrono>

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

namespace devilution {

// ============================================================================
// 🔒 SISTEMA DE VERIFICACIÓN DE ESTADO SEGURO
// ============================================================================

namespace {
    // Estado interno del sistema para verificaciones seguras
    struct SafeAudioState {
        bool gameFullyInitialized = false;
        bool playerValid = false;
        bool inDialog = false;
        bool inDialogTransition = false;  // NUEVO: Detectar transiciones de diálogo
        uint32_t lastStateCheck = 0;
        uint32_t dialogTransitionStart = 0;  // NUEVO: Timestamp de inicio de transición
        
        void UpdateState() {
            uint32_t currentTime = SDL_GetTicks();
            
            // Solo verificar estado cada 50ms para mejor responsividad durante transiciones
            if (currentTime - lastStateCheck < 50) {
                return;
            }
            lastStateCheck = currentTime;
            
            // Verificar estado del juego de forma segura
            gameFullyInitialized = (MyPlayer != nullptr);
            
            if (gameFullyInitialized) {
                playerValid = MyPlayer->plractive && 
                             MyPlayer->_pmode != PM_DEATH &&
                             MyPlayer->_pLvlChanging == 0;
            } else {
                playerValid = false;
            }
            
            // Detectar transiciones de diálogo de forma más inteligente
            bool wasInDialog = inDialog;
            inDialog = false; // Simplificado por ahora
            
            // Detectar si estamos en transición de diálogo
            if (!wasInDialog && inDialog) {
                // Iniciando diálogo
                inDialogTransition = true;
                dialogTransitionStart = currentTime;
            } else if (wasInDialog && !inDialog) {
                // Terminando diálogo
                inDialogTransition = true;
                dialogTransitionStart = currentTime;
            } else if (inDialogTransition && (currentTime - dialogTransitionStart > 1000)) {
                // Transición completada después de 1 segundo
                inDialogTransition = false;
            }
        }
        
        bool IsSafeForEnhancedAudio() {
            UpdateState();
            // CAMBIO CRÍTICO: Permitir audio mejorado durante transiciones de diálogo
            // porque InitQTextMsg necesita funcionar correctamente
            return gameFullyInitialized && playerValid;
        }
        
        bool IsInDialogTransition() {
            UpdateState();
            return inDialogTransition;
        }
    };
    
    SafeAudioState g_safeState;
}

// Exportar la variable para uso en minitext.cpp (sin thread_local para compatibilidad)
bool g_calledFromInitQTextMsg = false;

// ============================================================================
// 🔊 ESTADO GLOBAL DEL SISTEMA DE AUDIO MEJORADO
// ============================================================================

namespace {
    // Configuración del sistema
    EnhancedAudioConfig g_config;
    
    // Estadísticas
    EnhancedAudioStats g_stats;
    
    // Estado del sistema
    bool g_enhancedAudioEnabled = true;
    bool g_initialized = false;
    
#ifdef _DEBUG
    bool g_debugLogging = false;
#endif
}

// ============================================================================
// 🧠 IMPLEMENTACIÓN DE LA API PRINCIPAL
// ============================================================================

void InitEnhancedAudio() {
    // 🔒 INICIALIZACIÓN TARDÍA Y SEGURA
    
    // Verificar que el sistema básico de audio esté listo
    if (!gbSndInited) {
        g_initialized = false;
        return; // Audio básico no está listo, postponer inicialización
    }
    
    // Inicializar throttling
    InitAudioThrottling();
    
    // Configuración por defecto
    ApplyBalancedAudioPreset();
    
    // Reset estadísticas
    g_stats = {};
    
    // Reset estado seguro
    g_safeState = {};
    
    g_initialized = true;
    
#ifdef _DEBUG
    std::cout << "Enhanced Audio System initialized safely" << std::endl;
#endif
}

void UpdateEnhancedAudio() {
    // 🔒 INICIALIZACIÓN TARDÍA - Inicializar cuando sea seguro
    if (!g_initialized && gbSndInited) {
        InitEnhancedAudio();
        if (!g_initialized) {
            return; // Inicialización falló, intentar en el próximo frame
        }
    }
    
    if (!g_initialized || !g_enhancedAudioEnabled) {
        return;
    }
    
    // Actualizar throttling
    UpdateAudioThrottlingFrame();
    
    // Actualizar estadísticas
    AudioThrottlingStats throttlingStats = GetAudioThrottlingStats();
    g_stats.totalSoundsRequested = throttlingStats.totalSoundsRequested;
    g_stats.totalSoundsPlayed = throttlingStats.totalSoundsPlayed;
    g_stats.totalSoundsThrottled = throttlingStats.totalSoundsBlocked;
    g_stats.throttlingPercentage = throttlingStats.blockingPercentage;
}

void CleanupEnhancedAudio() {
    g_initialized = false;
    
#ifdef _DEBUG
    std::cout << "Enhanced Audio System cleaned up" << std::endl;
#endif
}

// ============================================================================
// 🎯 WRAPPERS MEJORADOS PARA FUNCIONES DE AUDIO
// ============================================================================

bool EnhancedPlaySFX(SfxID soundId) {
    // 🔒 VERIFICACIÓN DE ESTADO SEGURA CON EXCEPCIÓN PARA DIÁLOGOS
    
    // EXCEPCIÓN CRÍTICA: Si la llamada viene de InitQTextMsg, usar verificación más permisiva
    if (g_calledFromInitQTextMsg) {
        // Durante InitQTextMsg, solo verificar que MyPlayer existe y el sistema esté inicializado
        if (MyPlayer == nullptr || !g_initialized || !g_enhancedAudioEnabled) {
            return false;
        }
        // Permitir procesamiento durante transiciones de diálogo
    } else {
        // Verificación normal para otras llamadas
        if (!g_safeState.IsSafeForEnhancedAudio()) {
            return false; // Estado no seguro, usar sistema original
        }
        
        // Verificar inicialización del sistema
        if (!g_initialized || !g_enhancedAudioEnabled) {
            return false;
        }
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Verificar throttling
    if (g_config.throttlingEnabled && !CanPlaySound(soundId)) {
        g_stats.totalSoundsThrottled++;
        
#ifdef _DEBUG
        if (g_debugLogging) {
            std::cout << "EnhancedPlaySFX: Sound " << static_cast<int>(soundId) << " throttled" << std::endl;
        }
#endif
        return false;
    }
    
    // Aplicar variación si está habilitada
    int volume = 0;  // Volumen base (0 = máximo en el sistema original)
    int pitch = 0;   // Pitch base (no usado actualmente)
    
    if (g_config.audioVariationEnabled) {
        ApplyAudioVariation(volume, pitch, &volume, &pitch);
        g_stats.variationsApplied++;
    }
    
    // Reproducir sonido original
    PlaySFX(soundId);
    
    // Registrar que se reprodujo
    if (g_config.throttlingEnabled) {
        RegisterSoundPlayed(soundId);
    }
    
    g_stats.totalSoundsPlayed++;
    
    // Métricas de performance
    if (g_config.enablePerformanceMetrics) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        uint32_t processingTime = static_cast<uint32_t>(duration.count());
        
        g_stats.peakProcessingTimeUs = std::max(g_stats.peakProcessingTimeUs, processingTime);
        // Promedio móvil simple
        g_stats.averageProcessingTimeUs = (g_stats.averageProcessingTimeUs + processingTime) / 2;
    }
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "EnhancedPlaySFX: Sound " << static_cast<int>(soundId) << " played" << std::endl;
    }
#endif
    
    return true;
}

bool EnhancedPlaySfxLoc(SfxID soundId, Point position, bool randomizeByCategory) {
    // 🔒 VERIFICACIÓN DE ESTADO SEGURA - Sin dependencias externas
    if (!g_safeState.IsSafeForEnhancedAudio()) {
        return false; // Estado no seguro, usar sistema original
    }
    
    // Verificar inicialización del sistema
    if (!g_initialized || !g_enhancedAudioEnabled) {
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Verificar throttling
    if (g_config.throttlingEnabled && !CanPlaySound(soundId)) {
        g_stats.totalSoundsThrottled++;
        
#ifdef _DEBUG
        if (g_debugLogging) {
            std::cout << "EnhancedPlaySfxLoc: Sound " << static_cast<int>(soundId) << " throttled" << std::endl;
        }
#endif
        return false;
    }
    
    // Calcular spatial audio si está habilitado
    int volume = 0;  // Volumen base
    int pan = 0;     // Pan base
    
    // 🔒 SPATIAL AUDIO SEGURO - MyPlayer ya fue verificado en IsGameStateValidForEnhancedAudio()
    if (g_config.spatialAudioEnabled) {
        Point playerPos = MyPlayer->position.tile;
        
        if (!CalculateSpatialAudio(position, playerPos, volume, &volume, &pan)) {
            // Sonido fuera de rango
            g_stats.spatialSoundsOutOfRange++;
            
#ifdef _DEBUG
            if (g_debugLogging) {
                std::cout << "EnhancedPlaySfxLoc: Sound " << static_cast<int>(soundId) 
                          << " out of spatial range" << std::endl;
            }
#endif
            return false;
        }
        
        g_stats.spatialSoundsProcessed++;
    }
    
    // Aplicar variación si está habilitada
    int pitch = 0;
    if (g_config.audioVariationEnabled) {
        ApplyAudioVariation(volume, pitch, &volume, &pitch);
        g_stats.variationsApplied++;
    }
    
    // Reproducir sonido original
    // NOTA: El sistema original ya maneja volumen y pan internamente
    // Por ahora usamos la función original, pero en el futuro podríamos
    // interceptar a un nivel más bajo para aplicar nuestros cálculos
    PlaySfxLoc(soundId, position, randomizeByCategory);
    
    // Registrar que se reprodujo
    if (g_config.throttlingEnabled) {
        RegisterSoundPlayed(soundId);
    }
    
    g_stats.totalSoundsPlayed++;
    
    // Métricas de performance
    if (g_config.enablePerformanceMetrics) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        uint32_t processingTime = static_cast<uint32_t>(duration.count());
        
        g_stats.peakProcessingTimeUs = std::max(g_stats.peakProcessingTimeUs, processingTime);
        g_stats.averageProcessingTimeUs = (g_stats.averageProcessingTimeUs + processingTime) / 2;
    }
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "EnhancedPlaySfxLoc: Sound " << static_cast<int>(soundId) 
                  << " played at (" << position.x << "," << position.y << ")" << std::endl;
    }
#endif
    
    return true;
}

// ============================================================================
// 🎵 SPATIAL AUDIO FAKE (FASE A2)
// ============================================================================

void SetSpatialAudioConfig(const SpatialAudioConfig& config) {
    g_config.spatialConfig = config;
    
#ifdef _DEBUG
    std::cout << "Spatial audio config updated: maxDistance=" << config.maxDistance 
              << ", enabled=" << (config.enabled ? "true" : "false") << std::endl;
#endif
}

SpatialAudioConfig GetSpatialAudioConfig() {
    return g_config.spatialConfig;
}

bool CalculateSpatialAudio(Point soundPosition, Point playerPosition, int baseVolume, int* outVolume, int* outPan) {
    const SpatialAudioConfig& config = g_config.spatialConfig;
    
    if (!config.enabled) {
        *outVolume = baseVolume;
        *outPan = 0;
        return true;
    }
    
    // Calcular distancia Manhattan (más eficiente que Euclidiana)
    int dx = soundPosition.x - playerPosition.x;
    int dy = soundPosition.y - playerPosition.y;
    int distance = std::abs(dx) + std::abs(dy);
    
    // Verificar si está dentro del rango audible
    if (distance > config.maxDistance) {
        return false;  // Fuera de rango
    }
    
    // Calcular volumen basado en distancia con falloff cuadrático
    // MEJORA: Usar falloff cuadrático para más realismo
    float distanceRatio = static_cast<float>(distance) / config.maxDistance;
    float volumeMultiplier = 1.0f - (distanceRatio * distanceRatio); // Falloff cuadrático
    
    int volumeReduction = static_cast<int>((1.0f - volumeMultiplier) * config.volumeFalloff);
    *outVolume = std::max(baseVolume - volumeReduction, -10000);  // Límite mínimo de volumen
    
    // Calcular paneo estéreo basado en posición horizontal con curva suave
    float panRatio = static_cast<float>(dx) / config.maxDistance;
    panRatio = std::clamp(panRatio, -1.0f, 1.0f);
    
    // Aplicar curva suave al paneo para más naturalidad
    float smoothPan = panRatio * std::abs(panRatio); // Curva cuadrática suave
    *outPan = static_cast<int>(smoothPan * config.panFactor);
    *outPan = std::clamp(*outPan, -10000, 10000);  // Límites de paneo
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Spatial audio: distance=" << distance 
                  << ", distRatio=" << std::fixed << std::setprecision(2) << distanceRatio
                  << ", volMult=" << volumeMultiplier
                  << ", volume=" << *outVolume 
                  << ", pan=" << *outPan << std::endl;
    }
#endif
    
    return true;
}

// ============================================================================
// 🎶 VARIACIÓN DE AUDIO (FASE A3)
// ============================================================================

void SetAudioVariationConfig(const AudioVariationConfig& config) {
    g_config.variationConfig = config;
    
#ifdef _DEBUG
    std::cout << "Audio variation config updated: pitchRange=" << config.pitchVariationRange 
              << ", volumeRange=" << config.volumeVariationRange 
              << ", enabled=" << (config.enabled ? "true" : "false") << std::endl;
#endif
}

AudioVariationConfig GetAudioVariationConfig() {
    return g_config.variationConfig;
}

void ApplyAudioVariation(int baseVolume, int basePitch, int* outVolume, int* outPitch) {
    const AudioVariationConfig& config = g_config.variationConfig;
    
    if (!config.enabled) {
        *outVolume = baseVolume;
        *outPitch = basePitch;
        return;
    }
    
    // Verificar probabilidad
    if (GenerateRnd(100) > static_cast<int>(config.probability * 100)) {
        *outVolume = baseVolume;
        *outPitch = basePitch;
        return;
    }
    
    // 🎶 VARIACIÓN MEJORADA - Aplicar variación inteligente
    
    // Variación de volumen con curva suave
    int volumeRange = config.volumeVariationRange;
    int volumeVariation = GenerateRnd(volumeRange * 2 + 1) - volumeRange;
    
    // Aplicar curva suave para variaciones más naturales
    float volumeRatio = static_cast<float>(volumeVariation) / volumeRange;
    volumeRatio = volumeRatio * std::abs(volumeRatio); // Curva cuadrática suave
    volumeVariation = static_cast<int>(volumeRatio * volumeRange);
    
    *outVolume = baseVolume + volumeVariation;
    
    // Variación de pitch con distribución más natural
    int pitchRange = config.pitchVariationRange;
    int pitchVariation = GenerateRnd(pitchRange * 2 + 1) - pitchRange;
    
    // Aplicar distribución gaussiana aproximada para pitch más natural
    int pitch1 = GenerateRnd(pitchRange * 2 + 1) - pitchRange;
    int pitch2 = GenerateRnd(pitchRange * 2 + 1) - pitchRange;
    pitchVariation = (pitch1 + pitch2) / 2; // Promedio para distribución más centrada
    
    *outPitch = basePitch + pitchVariation;
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Audio variation applied: volumeVar=" << volumeVariation 
                  << " (ratio=" << std::fixed << std::setprecision(2) << volumeRatio << ")"
                  << ", pitchVar=" << pitchVariation 
                  << " (gaussian approx)" << std::endl;
    }
#endif
}

// ============================================================================
// 🎮 INTEGRACIÓN CON SISTEMA EXISTENTE
// ============================================================================

void SetEnhancedAudioEnabled(bool enabled) {
    g_enhancedAudioEnabled = enabled;
    
#ifdef _DEBUG
    std::cout << "Enhanced audio " << (enabled ? "enabled" : "disabled") << std::endl;
#endif
}

bool IsEnhancedAudioEnabled() {
    return g_enhancedAudioEnabled && g_initialized;
}

// ============================================================================
// 📊 ESTADÍSTICAS Y MÉTRICAS
// ============================================================================

EnhancedAudioStats GetEnhancedAudioStats() {
    return g_stats;
}

void ResetEnhancedAudioStats() {
    g_stats = {};
    ResetAudioThrottlingStats();
    
#ifdef _DEBUG
    std::cout << "Enhanced audio stats reset" << std::endl;
#endif
}

// ============================================================================
// 🔧 CONFIGURACIÓN AVANZADA
// ============================================================================

void ConfigureEnhancedAudio(const EnhancedAudioConfig& config) {
    g_config = config;
    
    // Aplicar configuración de throttling
    SetAudioThrottlingEnabled(config.throttlingEnabled);
    
#ifdef _DEBUG
    std::cout << "Enhanced audio configured" << std::endl;
#endif
}

EnhancedAudioConfig GetEnhancedAudioConfig() {
    return g_config;
}

// ============================================================================
// 🎯 PRESETS DE CONFIGURACIÓN
// ============================================================================

void ApplyConservativeAudioPreset() {
    EnhancedAudioConfig config;
    
    // Throttling mínimo
    config.throttlingEnabled = true;
    
    // Spatial audio básico
    config.spatialAudioEnabled = true;
    config.spatialConfig.maxDistance = 30;
    config.spatialConfig.volumeFalloff = 32;
    config.spatialConfig.panFactor = 128;
    
    // Variación mínima
    config.audioVariationEnabled = true;
    config.variationConfig.pitchVariationRange = 2;
    config.variationConfig.volumeVariationRange = 5;
    config.variationConfig.probability = 0.5f;
    
    // Performance
    config.enablePerformanceMetrics = false;
    
    ConfigureEnhancedAudio(config);
    
    // Configurar throttling conservador para Inferno
    SetSoundThrottling(SfxID::SpellInferno, 300, AudioPriority::MEDIUM, 1);
    
#ifdef _DEBUG
    std::cout << "Conservative audio preset applied" << std::endl;
#endif
}

void ApplyBalancedAudioPreset() {
    EnhancedAudioConfig config;
    
    // Throttling balanceado
    config.throttlingEnabled = true;
    
    // 🎵 SPATIAL AUDIO MEJORADO - Configuración optimizada
    config.spatialAudioEnabled = true;
    config.spatialConfig.maxDistance = 20;      // Rango más cercano para más inmersión
    config.spatialConfig.volumeFalloff = 128;   // Falloff más pronunciado
    config.spatialConfig.panFactor = 512;       // Paneo más agresivo
    config.spatialConfig.enabled = true;
    
    // 🎶 VARIACIÓN INTELIGENTE - Configuración optimizada
    config.audioVariationEnabled = true;
    config.variationConfig.pitchVariationRange = 3;    // Variación sutil de pitch
    config.variationConfig.volumeVariationRange = 8;   // Variación moderada de volumen
    config.variationConfig.probability = 0.6f;         // 60% probabilidad para naturalidad
    
    // Performance básica
    config.enablePerformanceMetrics = false;
    
    ConfigureEnhancedAudio(config);
    
    // Configurar throttling balanceado (usar configuración por defecto)
    InitRecommendedAudioThrottling();
    
#ifdef _DEBUG
    std::cout << "Balanced audio preset applied with enhanced spatial audio" << std::endl;
#endif
}

void ApplyAggressiveAudioPreset() {
    EnhancedAudioConfig config;
    
    // Throttling agresivo
    config.throttlingEnabled = true;
    
    // 🎵 SPATIAL AUDIO ULTRA-INMERSIVO - Configuración máxima
    config.spatialAudioEnabled = true;
    config.spatialConfig.maxDistance = 15;      // Rango muy cercano para máxima inmersión
    config.spatialConfig.volumeFalloff = 192;   // Falloff muy pronunciado
    config.spatialConfig.panFactor = 768;       // Paneo ultra-agresivo
    config.spatialConfig.enabled = true;
    
    // 🎶 VARIACIÓN ULTRA-NATURAL - Configuración máxima
    config.audioVariationEnabled = true;
    config.variationConfig.pitchVariationRange = 5;    // Variación más pronunciada
    config.variationConfig.volumeVariationRange = 12;  // Variación amplia de volumen
    config.variationConfig.probability = 0.8f;         // 80% probabilidad para máxima naturalidad
    
    // Performance habilitada
    config.enablePerformanceMetrics = true;
    config.maxProcessingTimeUs = 500;
    
    ConfigureEnhancedAudio(config);
    
    // Configurar throttling ultra-agresivo para Inferno
    SetSoundThrottling(SfxID::SpellInferno, 150, AudioPriority::MEDIUM, 1);
    SetSoundThrottling(SfxID::SpellLightning, 100, AudioPriority::MEDIUM, 1);
    SetSoundThrottling(SfxID::SpellFirebolt, 80, AudioPriority::MEDIUM, 1);
    
#ifdef _DEBUG
    std::cout << "Aggressive audio preset applied with ultra-immersive spatial audio" << std::endl;
#endif
}

void ApplyDebugAudioPreset() {
    ApplyBalancedAudioPreset();
    
    // Habilitar métricas y logging
    g_config.enablePerformanceMetrics = true;
    
#ifdef _DEBUG
    g_debugLogging = true;
    SetAudioThrottlingDebugLogging(true);
    std::cout << "Debug audio preset applied with detailed logging" << std::endl;
#endif
}

// ============================================================================
// 🔍 DEBUGGING Y DIAGNÓSTICO
// ============================================================================

#ifdef _DEBUG
void DebugPrintEnhancedAudioStats() {
    std::cout << "\n=== ENHANCED AUDIO STATISTICS ===" << std::endl;
    std::cout << "System enabled: " << (g_enhancedAudioEnabled ? "YES" : "NO") << std::endl;
    std::cout << "Initialized: " << (g_initialized ? "YES" : "NO") << std::endl;
    
    std::cout << "\n--- THROTTLING ---" << std::endl;
    std::cout << "Total requested: " << g_stats.totalSoundsRequested << std::endl;
    std::cout << "Total played: " << g_stats.totalSoundsPlayed << std::endl;
    std::cout << "Total throttled: " << g_stats.totalSoundsThrottled << std::endl;
    std::cout << "Throttling %: " << std::fixed << std::setprecision(2) 
              << g_stats.throttlingPercentage << "%" << std::endl;
    
    std::cout << "\n--- SPATIAL AUDIO ---" << std::endl;
    std::cout << "Processed: " << g_stats.spatialSoundsProcessed << std::endl;
    std::cout << "Out of range: " << g_stats.spatialSoundsOutOfRange << std::endl;
    
    std::cout << "\n--- VARIATION ---" << std::endl;
    std::cout << "Variations applied: " << g_stats.variationsApplied << std::endl;
    
    std::cout << "\n--- PERFORMANCE ---" << std::endl;
    std::cout << "Average processing: " << g_stats.averageProcessingTimeUs << "μs" << std::endl;
    std::cout << "Peak processing: " << g_stats.peakProcessingTimeUs << "μs" << std::endl;
    
    std::cout << "=================================" << std::endl;
}

void DebugPrintEnhancedAudioConfig() {
    std::cout << "\n=== ENHANCED AUDIO CONFIG ===" << std::endl;
    std::cout << "Throttling enabled: " << (g_config.throttlingEnabled ? "YES" : "NO") << std::endl;
    std::cout << "Spatial audio enabled: " << (g_config.spatialAudioEnabled ? "YES" : "NO") << std::endl;
    std::cout << "Audio variation enabled: " << (g_config.audioVariationEnabled ? "YES" : "NO") << std::endl;
    
    std::cout << "\n--- SPATIAL CONFIG ---" << std::endl;
    std::cout << "Max distance: " << g_config.spatialConfig.maxDistance << std::endl;
    std::cout << "Volume falloff: " << g_config.spatialConfig.volumeFalloff << std::endl;
    std::cout << "Pan factor: " << g_config.spatialConfig.panFactor << std::endl;
    
    std::cout << "\n--- VARIATION CONFIG ---" << std::endl;
    std::cout << "Pitch range: " << g_config.variationConfig.pitchVariationRange << std::endl;
    std::cout << "Volume range: " << g_config.variationConfig.volumeVariationRange << std::endl;
    std::cout << "Probability: " << g_config.variationConfig.probability << std::endl;
    
    std::cout << "=============================" << std::endl;
}

void SetEnhancedAudioDebugLogging(bool enabled) {
    g_debugLogging = enabled;
    std::cout << "Enhanced audio debug logging " << (enabled ? "enabled" : "disabled") << std::endl;
}

void RunEnhancedAudioStressTest() {
    std::cout << "Running Enhanced Audio Stress Test..." << std::endl;
    
    // Reset estadísticas
    ResetEnhancedAudioStats();
    
    // Simular spam de Inferno
    for (int i = 0; i < 100; ++i) {
        EnhancedPlaySFX(SfxID::SpellInferno);
    }
    
    // Simular sonidos espaciales
    Point testPos = {10, 10};
    for (int i = 0; i < 50; ++i) {
        EnhancedPlaySfxLoc(SfxID::MonsterHit, testPos);
        testPos.x += 2;
        testPos.y += 1;
    }
    
    // Imprimir resultados
    DebugPrintEnhancedAudioStats();
    
    std::cout << "Stress test completed!" << std::endl;
}
#endif

} // namespace devilution