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
    // Inicializar throttling
    InitAudioThrottling();
    
    // Configuración por defecto
    ApplyBalancedAudioPreset();
    
    // Reset estadísticas
    g_stats = {};
    
    g_initialized = true;
    
#ifdef _DEBUG
    std::cout << "Enhanced Audio System initialized" << std::endl;
#endif
}

void UpdateEnhancedAudio() {
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
    if (!g_initialized || !g_enhancedAudioEnabled) {
        // Fallback a función original
        PlaySFX(soundId);
        return true;
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
    if (!g_initialized || !g_enhancedAudioEnabled) {
        // Fallback a función original
        PlaySfxLoc(soundId, position, randomizeByCategory);
        return true;
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
    
    if (g_config.spatialAudioEnabled && MyPlayer != nullptr) {
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
    
    // Calcular volumen basado en distancia
    int volumeReduction = (distance * config.volumeFalloff) / config.maxDistance;
    *outVolume = std::max(baseVolume - volumeReduction, -10000);  // Límite mínimo de volumen
    
    // Calcular paneo estéreo basado en posición horizontal
    int panValue = (dx * config.panFactor) / config.maxDistance;
    *outPan = std::clamp(panValue, -10000, 10000);  // Límites de paneo
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Spatial audio: distance=" << distance 
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
    
    // Aplicar variación de volumen
    int volumeVariation = GenerateRnd(config.volumeVariationRange * 2 + 1) - config.volumeVariationRange;
    *outVolume = baseVolume + volumeVariation;
    
    // Aplicar variación de pitch (para futuro uso)
    int pitchVariation = GenerateRnd(config.pitchVariationRange * 2 + 1) - config.pitchVariationRange;
    *outPitch = basePitch + pitchVariation;
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Audio variation applied: volumeVar=" << volumeVariation 
                  << ", pitchVar=" << pitchVariation << std::endl;
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
    
    // Spatial audio completo
    config.spatialAudioEnabled = true;
    config.spatialConfig.maxDistance = 25;
    config.spatialConfig.volumeFalloff = 64;
    config.spatialConfig.panFactor = 256;
    
    // Variación moderada
    config.audioVariationEnabled = true;
    config.variationConfig.pitchVariationRange = 4;
    config.variationConfig.volumeVariationRange = 10;
    config.variationConfig.probability = 0.7f;
    
    // Performance básica
    config.enablePerformanceMetrics = false;
    
    ConfigureEnhancedAudio(config);
    
    // Configurar throttling balanceado (usar configuración por defecto)
    InitRecommendedAudioThrottling();
    
#ifdef _DEBUG
    std::cout << "Balanced audio preset applied" << std::endl;
#endif
}

void ApplyAggressiveAudioPreset() {
    EnhancedAudioConfig config;
    
    // Throttling agresivo
    config.throttlingEnabled = true;
    
    // Spatial audio optimizado
    config.spatialAudioEnabled = true;
    config.spatialConfig.maxDistance = 20;
    config.spatialConfig.volumeFalloff = 96;
    config.spatialConfig.panFactor = 384;
    
    // Variación máxima
    config.audioVariationEnabled = true;
    config.variationConfig.pitchVariationRange = 6;
    config.variationConfig.volumeVariationRange = 15;
    config.variationConfig.probability = 0.9f;
    
    // Performance habilitada
    config.enablePerformanceMetrics = true;
    config.maxProcessingTimeUs = 500;
    
    ConfigureEnhancedAudio(config);
    
    // Configurar throttling ultra-agresivo para Inferno
    SetSoundThrottling(SfxID::SpellInferno, 150, AudioPriority::MEDIUM, 1);
    SetSoundThrottling(SfxID::SpellLightning, 100, AudioPriority::MEDIUM, 1);
    SetSoundThrottling(SfxID::SpellFirebolt, 80, AudioPriority::MEDIUM, 1);
    
#ifdef _DEBUG
    std::cout << "Aggressive audio preset applied" << std::endl;
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