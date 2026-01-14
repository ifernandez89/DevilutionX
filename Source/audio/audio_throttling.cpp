/**
 * @file audio_throttling.cpp
 * 
 * AUDIO ENHANCEMENT - Phase A1: Audio Throttling and Priority System Implementation
 * 
 * 🔊 IMPLEMENTACIÓN del sistema inteligente de throttling de audio
 */

#include "audio_throttling.h"

#include <algorithm>
#include <unordered_map>
#include <iostream>

#ifdef _DEBUG
#include <iomanip>
#endif

namespace devilution {

// ============================================================================
// 🔊 ESTADO GLOBAL DEL SISTEMA DE THROTTLING
// ============================================================================

namespace {
    // Mapa de configuraciones de throttling por sonido
    std::unordered_map<SfxID, SoundCooldown> g_soundCooldowns;
    
    // Estadísticas globales
    AudioThrottlingStats g_stats;
    
    // Estado del sistema
    bool g_throttlingEnabled = true;
    uint32_t g_currentFrame = 0;
    
    // Métricas del frame actual
    AudioSafetyMetrics g_currentFrameMetrics;
    
#ifdef _DEBUG
    bool g_debugLogging = false;
#endif
}

// ============================================================================
// 🧠 IMPLEMENTACIÓN DE LA API PRINCIPAL
// ============================================================================

void InitAudioThrottling() {
    // Limpiar estado anterior
    g_soundCooldowns.clear();
    g_stats = {};
    g_currentFrameMetrics = {};
    
    // Configurar throttling predefinido
    for (const auto& config : AUDIO_THROTTLE_CONFIGS) {
        SoundCooldown cooldown;
        cooldown.cooldownMs = config.cooldownMs;
        cooldown.priority = static_cast<uint8_t>(config.priority);
        cooldown.maxPerFrame = config.maxPerFrame;
        cooldown.lastPlayTime = 0;
        cooldown.currentFrameCount = 0;
        
        g_soundCooldowns[config.soundId] = cooldown;
    }
    
#ifdef _DEBUG
    std::cout << "Audio Throttling System initialized with " 
              << g_soundCooldowns.size() << " configured sounds" << std::endl;
#endif
}

bool CanPlaySound(SfxID soundId) {
    if (!g_throttlingEnabled) {
        return true;
    }
    
    g_stats.totalSoundsRequested++;
    g_currentFrameMetrics.totalSoundsThisFrame++;
    
    // Buscar configuración para este sonido
    auto it = g_soundCooldowns.find(soundId);
    if (it == g_soundCooldowns.end()) {
        // No hay throttling configurado para este sonido
        g_stats.totalSoundsPlayed++;
        return true;
    }
    
    SoundCooldown& cooldown = it->second;
    uint32_t currentTime = SDL_GetTicks();
    
    // Verificar cooldown temporal
    if (currentTime - cooldown.lastPlayTime < cooldown.cooldownMs) {
#ifdef _DEBUG
        if (g_debugLogging) {
            std::cout << "Audio throttled (cooldown): " << static_cast<int>(soundId) 
                      << " (remaining: " << (cooldown.cooldownMs - (currentTime - cooldown.lastPlayTime)) 
                      << "ms)" << std::endl;
        }
#endif
        g_stats.totalSoundsBlocked++;
        g_currentFrameMetrics.soundsBlockedThisFrame++;
        RecordAudioThrottlingEvent(soundId, true);
        return false;
    }
    
    // Verificar límite por frame
    if (cooldown.currentFrameCount >= cooldown.maxPerFrame) {
#ifdef _DEBUG
        if (g_debugLogging) {
            std::cout << "Audio throttled (frame limit): " << static_cast<int>(soundId) 
                      << " (count: " << static_cast<int>(cooldown.currentFrameCount) 
                      << "/" << static_cast<int>(cooldown.maxPerFrame) << ")" << std::endl;
        }
#endif
        g_stats.totalSoundsBlocked++;
        g_currentFrameMetrics.soundsBlockedThisFrame++;
        RecordAudioThrottlingEvent(soundId, true);
        return false;
    }
    
    // Sonido puede reproducirse
    g_stats.totalSoundsPlayed++;
    RecordAudioThrottlingEvent(soundId, false);
    return true;
}

void RegisterSoundPlayed(SfxID soundId) {
    if (!g_throttlingEnabled) {
        return;
    }
    
    auto it = g_soundCooldowns.find(soundId);
    if (it != g_soundCooldowns.end()) {
        SoundCooldown& cooldown = it->second;
        cooldown.lastPlayTime = SDL_GetTicks();
        cooldown.currentFrameCount++;
        
        g_currentFrameMetrics.currentActiveSounds++;
        
#ifdef _DEBUG
        if (g_debugLogging) {
            std::cout << "Audio played: " << static_cast<int>(soundId) 
                      << " (frame count: " << static_cast<int>(cooldown.currentFrameCount) << ")" << std::endl;
        }
#endif
    }
}

void UpdateAudioThrottlingFrame() {
    g_currentFrame++;
    
    // Reset contadores por frame
    for (auto& pair : g_soundCooldowns) {
        pair.second.currentFrameCount = 0;
    }
    
    // Actualizar estadísticas
    g_stats.blockingPercentage = g_stats.totalSoundsRequested > 0 
        ? (static_cast<float>(g_stats.totalSoundsBlocked) / g_stats.totalSoundsRequested) * 100.0f 
        : 0.0f;
    
    // Detectar sobrecarga de audio
    g_currentFrameMetrics.isAudioOverloaded = 
        g_currentFrameMetrics.totalSoundsThisFrame > 10 || // Más de 10 sonidos por frame
        g_currentFrameMetrics.soundsBlockedThisFrame > 5;   // Más de 5 bloqueados por frame
    
    // Reset métricas del frame
    g_currentFrameMetrics.currentActiveSounds = 0;
    g_currentFrameMetrics.soundsBlockedThisFrame = 0;
    g_currentFrameMetrics.totalSoundsThisFrame = 0;
}

AudioThrottlingStats GetAudioThrottlingStats() {
    return g_stats;
}

void ResetAudioThrottlingStats() {
    g_stats = {};
}

// ============================================================================
// 🔧 CONFIGURACIÓN AVANZADA
// ============================================================================

void SetSoundThrottling(SfxID soundId, uint32_t cooldownMs, AudioPriority priority, uint8_t maxPerFrame) {
    SoundCooldown cooldown;
    cooldown.cooldownMs = cooldownMs;
    cooldown.priority = static_cast<uint8_t>(priority);
    cooldown.maxPerFrame = maxPerFrame;
    cooldown.lastPlayTime = 0;
    cooldown.currentFrameCount = 0;
    
    g_soundCooldowns[soundId] = cooldown;
    
#ifdef _DEBUG
    std::cout << "Audio throttling configured for sound " << static_cast<int>(soundId) 
              << ": cooldown=" << cooldownMs << "ms, priority=" << static_cast<int>(priority)
              << ", maxPerFrame=" << static_cast<int>(maxPerFrame) << std::endl;
#endif
}

const SoundCooldown* GetSoundThrottling(SfxID soundId) {
    auto it = g_soundCooldowns.find(soundId);
    return (it != g_soundCooldowns.end()) ? &it->second : nullptr;
}

void SetAudioThrottlingEnabled(bool enabled) {
    g_throttlingEnabled = enabled;
    
#ifdef _DEBUG
    std::cout << "Audio throttling " << (enabled ? "enabled" : "disabled") << std::endl;
#endif
}

bool IsAudioThrottlingEnabled() {
    return g_throttlingEnabled;
}

// ============================================================================
// 🎮 INTEGRACIÓN CON SAFETY LAYER
// ============================================================================

void RecordAudioThrottlingEvent(SfxID soundId, bool wasBlocked) {
    // Esta función será llamada por el Safety Layer para métricas
    // Por ahora, solo actualizamos estadísticas locales
    
    if (wasBlocked) {
        // Registrar evento de throttling
#ifdef _DEBUG
        if (g_debugLogging) {
            std::cout << "THROTTLING EVENT: Sound " << static_cast<int>(soundId) << " blocked" << std::endl;
        }
#endif
    }
}

AudioSafetyMetrics GetAudioSafetyMetrics() {
    return g_currentFrameMetrics;
}

// ============================================================================
// 🔍 DEBUGGING Y LOGGING
// ============================================================================

#ifdef _DEBUG
void DebugPrintAudioThrottlingStats() {
    std::cout << "\n=== AUDIO THROTTLING STATISTICS ===" << std::endl;
    std::cout << "Total sounds requested: " << g_stats.totalSoundsRequested << std::endl;
    std::cout << "Total sounds played: " << g_stats.totalSoundsPlayed << std::endl;
    std::cout << "Total sounds blocked: " << g_stats.totalSoundsBlocked << std::endl;
    std::cout << "Blocking percentage: " << std::fixed << std::setprecision(2) 
              << g_stats.blockingPercentage << "%" << std::endl;
    std::cout << "Current frame: " << g_currentFrame << std::endl;
    std::cout << "Throttling enabled: " << (g_throttlingEnabled ? "YES" : "NO") << std::endl;
    std::cout << "Configured sounds: " << g_soundCooldowns.size() << std::endl;
    std::cout << "===================================" << std::endl;
}

void DebugPrintSoundConfig(SfxID soundId) {
    auto it = g_soundCooldowns.find(soundId);
    if (it == g_soundCooldowns.end()) {
        std::cout << "Sound " << static_cast<int>(soundId) << ": No throttling configured" << std::endl;
        return;
    }
    
    const SoundCooldown& cooldown = it->second;
    uint32_t currentTime = SDL_GetTicks();
    uint32_t timeSinceLastPlay = currentTime - cooldown.lastPlayTime;
    
    std::cout << "\n=== SOUND CONFIG: " << static_cast<int>(soundId) << " ===" << std::endl;
    std::cout << "Cooldown: " << cooldown.cooldownMs << "ms" << std::endl;
    std::cout << "Priority: " << static_cast<int>(cooldown.priority) << std::endl;
    std::cout << "Max per frame: " << static_cast<int>(cooldown.maxPerFrame) << std::endl;
    std::cout << "Current frame count: " << static_cast<int>(cooldown.currentFrameCount) << std::endl;
    std::cout << "Last play time: " << cooldown.lastPlayTime << std::endl;
    std::cout << "Time since last play: " << timeSinceLastPlay << "ms" << std::endl;
    std::cout << "Can play now: " << (CanPlaySound(soundId) ? "YES" : "NO") << std::endl;
    std::cout << "===============================" << std::endl;
}

void SetAudioThrottlingDebugLogging(bool enabled) {
    g_debugLogging = enabled;
    std::cout << "Audio throttling debug logging " << (enabled ? "enabled" : "disabled") << std::endl;
}
#endif

// ============================================================================
// 🚀 FUNCIONES DE CONVENIENCIA PARA SONIDOS ESPECÍFICOS
// ============================================================================

/**
 * Configuraciones específicas para sonidos problemáticos conocidos
 */
void ConfigureInfernoThrottling() {
    // Configuración ultra-agresiva para Inferno
    SetSoundThrottling(SfxID::SpellInferno, 200, AudioPriority::MEDIUM, 1);
    
#ifdef _DEBUG
    std::cout << "Inferno audio throttling configured: 200ms cooldown, max 1 per frame" << std::endl;
#endif
}

void ConfigureCombatThrottling() {
    // Configuración para sonidos de combate
    SetSoundThrottling(SfxID::Swing, 80, AudioPriority::HIGH, 4);
    SetSoundThrottling(SfxID::SpellFireHit, 60, AudioPriority::MEDIUM, 3);
    SetSoundThrottling(SfxID::ShootBow, 100, AudioPriority::LOW, 3);
    SetSoundThrottling(SfxID::ShootBow2, 100, AudioPriority::LOW, 3);
    
#ifdef _DEBUG
    std::cout << "Combat audio throttling configured" << std::endl;
#endif
}

void ConfigureSpellThrottling() {
    // Configuración para spells problemáticos
    SetSoundThrottling(SfxID::SpellLightning, 150, AudioPriority::MEDIUM, 2);
    SetSoundThrottling(SfxID::SpellFirebolt, 120, AudioPriority::MEDIUM, 2);
    SetSoundThrottling(SfxID::SpellFireWall, 180, AudioPriority::MEDIUM, 1);
    SetSoundThrottling(SfxID::SpellLightningHit, 150, AudioPriority::MEDIUM, 2);
    
#ifdef _DEBUG
    std::cout << "Spell audio throttling configured" << std::endl;
#endif
}

/**
 * Inicialización completa con todas las configuraciones recomendadas
 */
void InitRecommendedAudioThrottling() {
    InitAudioThrottling();
    ConfigureInfernoThrottling();
    ConfigureCombatThrottling();
    ConfigureSpellThrottling();
    
#ifdef _DEBUG
    std::cout << "Recommended audio throttling configuration applied" << std::endl;
#endif
}

} // namespace devilution