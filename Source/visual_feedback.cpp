/**
 * @file visual_feedback.cpp
 * 
 * 🎮 FASE V3 - FEEDBACK VISUAL REACTIVO 🎮
 * 
 * Implementación completa del sistema de feedback visual reactivo
 */

#include "visual_feedback.h"

#include <algorithm>
#include <cmath>
#include <chrono>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "engine/random.hpp"
#include "player.h"
#include "monster.h"
#include "items.h"
#include "spells.h"

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

namespace devilution {

// ============================================================================
// 🔒 ESTADO GLOBAL DEL SISTEMA
// ============================================================================

namespace {
    VisualFeedbackState g_visualState;
    VisualFeedbackStats g_stats;
    
#ifdef _DEBUG
    bool g_debugLogging = false;
#endif
}

// ============================================================================
// 🎯 IMPLEMENTACIÓN DE LA API PRINCIPAL
// ============================================================================

void InitVisualFeedback()
{
    // Inicializar estado
    g_visualState = {};
    g_stats = {};
    
    // Configuración por defecto (preset balanceado)
    ApplyBalancedVisualPreset();
    
#ifdef _DEBUG
    std::cout << "Visual Feedback System initialized" << std::endl;
#endif
}

void UpdateVisualFeedback()
{
    if (!g_visualState.enabled) {
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    uint32_t currentTime = SDL_GetTicks();
    
    // Actualizar efectos activos
    for (int i = 0; i < g_visualState.activeEffectCount; i++) {
        VisualEffect &effect = g_visualState.activeEffects[i];
        
        if (!effect.active) {
            continue;
        }
        
        // Verificar si el efecto ha expirado
        if (currentTime - effect.startTime >= effect.duration) {
            effect.active = false;
            
            // Compactar array de efectos activos
            for (int j = i; j < g_visualState.activeEffectCount - 1; j++) {
                g_visualState.activeEffects[j] = g_visualState.activeEffects[j + 1];
            }
            g_visualState.activeEffectCount--;
            i--; // Reajustar índice después de compactar
            continue;
        }
        
        // Calcular intensidad actual basada en el tiempo transcurrido
        uint32_t elapsed = currentTime - effect.startTime;
        float progress = static_cast<float>(elapsed) / effect.duration;
        
        // Aplicar diferentes curvas según el tipo de efecto
        switch (effect.type) {
            case VisualFeedbackType::PlayerDamageFlash:
                // Flash rápido que se desvanece exponencialmente
                effect.currentIntensity = effect.intensity * std::exp(-progress * 4.0f);
                break;
                
            case VisualFeedbackType::EliteMonsterPulse:
                // Pulse sinusoidal continuo
                effect.currentIntensity = effect.intensity * (0.5f + 0.5f * std::sin(progress * effect.pulseSpeed * 2.0f * M_PI));
                break;
                
            case VisualFeedbackType::PlayerLowHealthPulse:
                // Pulse de advertencia más lento y persistente
                effect.currentIntensity = effect.intensity * (0.3f + 0.7f * std::sin(progress * effect.pulseSpeed * M_PI));
                break;
                
            case VisualFeedbackType::CriticalHitFlash:
                // Flash intenso con caída cuadrática
                effect.currentIntensity = effect.intensity * (1.0f - progress * progress);
                break;
                
            default:
                // Fade out lineal por defecto
                effect.currentIntensity = effect.intensity * (1.0f - progress);
                break;
        }
        
        // Asegurar que la intensidad no sea negativa
        effect.currentIntensity = std::max(0.0f, effect.currentIntensity);
    }
    
    // Actualizar estadísticas
    g_stats.activeEffects = g_visualState.activeEffectCount;
    
    // Calcular intensidad promedio
    float totalIntensity = 0.0f;
    for (int i = 0; i < g_visualState.activeEffectCount; i++) {
        totalIntensity += g_visualState.activeEffects[i].currentIntensity;
    }
    g_stats.averageIntensity = g_visualState.activeEffectCount > 0 ? 
        totalIntensity / g_visualState.activeEffectCount : 0.0f;
    
    // Métricas de performance
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    g_stats.averageProcessingTimeUs = static_cast<uint32_t>(duration.count());
}

void CleanupVisualFeedback()
{
    g_visualState = {};
    
#ifdef _DEBUG
    std::cout << "Visual Feedback System cleaned up" << std::endl;
#endif
}

void SetVisualFeedbackEnabled(bool enabled)
{
    g_visualState.enabled = enabled;
    
#ifdef _DEBUG
    std::cout << "Visual Feedback " << (enabled ? "enabled" : "disabled") << std::endl;
#endif
}

void SetVisualFeedbackIntensity(float intensity)
{
    g_visualState.globalIntensity = std::clamp(intensity, 0.0f, 2.0f);
    
#ifdef _DEBUG
    std::cout << "Visual Feedback intensity set to " << intensity << std::endl;
#endif
}

// ============================================================================
// 🎮 EFECTOS ESPECÍFICOS - FASE V3
// ============================================================================

namespace {
    /**
     * Función auxiliar para añadir un efecto al sistema
     */
    void AddVisualEffect(const VisualEffect &effect) {
        if (g_visualState.activeEffectCount >= static_cast<int>(g_visualState.activeEffects.size())) {
            // Si no hay espacio, reemplazar el efecto más antiguo
            g_visualState.activeEffects[0] = effect;
            return;
        }
        
        g_visualState.activeEffects[g_visualState.activeEffectCount] = effect;
        g_visualState.activeEffectCount++;
        g_stats.totalEffectsTriggered++;
    }
}

void TriggerPlayerDamageFlash(const Player &player, int damage, DamageType damageType)
{
    if (!g_visualState.enabled) return;
    
    VisualEffect effect;
    effect.type = VisualFeedbackType::PlayerDamageFlash;
    effect.startTime = SDL_GetTicks();
    effect.duration = 500; // 500ms flash
    effect.position = player.position.tile;
    effect.active = true;
    effect.radius = 8;
    
    // Color basado en el tipo de daño
    switch (damageType) {
        case DamageType::Physical:
            effect.color = { 255, 100, 100, 255 }; // Rojo para daño físico
            break;
        case DamageType::Fire:
            effect.color = { 255, 150, 50, 255 };  // Naranja para fuego
            break;
        case DamageType::Lightning:
            effect.color = { 150, 150, 255, 255 }; // Azul para rayo
            break;
        case DamageType::Magic:
            effect.color = { 200, 100, 255, 255 }; // Púrpura para magia
            break;
        default:
            effect.color = { 255, 255, 255, 255 }; // Blanco por defecto
            break;
    }
    
    // Intensidad basada en el daño
    float damageRatio = std::min(static_cast<float>(damage) / 100.0f, 1.0f);
    effect.intensity = 0.3f + (damageRatio * 0.7f); // Entre 0.3 y 1.0
    effect.intensity *= g_visualState.globalIntensity;
    
    AddVisualEffect(effect);
    g_stats.damageFlashesTriggered++;
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Player damage flash triggered: damage=" << damage 
                  << ", intensity=" << effect.intensity << std::endl;
    }
#endif
}

void TriggerEliteMonsterPulse(const Monster &monster)
{
    if (!g_visualState.enabled) return;
    
    VisualEffect effect;
    effect.type = VisualFeedbackType::EliteMonsterPulse;
    effect.startTime = SDL_GetTicks();
    effect.duration = 3000; // 3 segundos de pulse
    effect.position = monster.position.tile;
    effect.active = true;
    effect.radius = 6;
    effect.pulseSpeed = 2.0f; // 2 pulsos por segundo
    
    // Color dorado para monstruos elite
    effect.color = { 255, 215, 0, 255 };
    effect.intensity = 0.6f * g_visualState.globalIntensity;
    
    AddVisualEffect(effect);
    g_stats.elitePulsesTriggered++;
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Elite monster pulse triggered at (" 
                  << monster.position.tile.x << "," << monster.position.tile.y << ")" << std::endl;
    }
#endif
}

void TriggerContextualFeedback(VisualFeedbackType type, Point position, float intensity)
{
    if (!g_visualState.enabled || !g_visualState.contextualEffects) return;
    
    VisualEffect effect;
    effect.type = type;
    effect.startTime = SDL_GetTicks();
    effect.position = position;
    effect.active = true;
    effect.intensity = intensity * g_visualState.globalIntensity;
    
    // Configuración específica por tipo
    switch (type) {
        case VisualFeedbackType::SpellCastGlow:
            effect.duration = 800;
            effect.color = { 100, 200, 255, 255 }; // Azul mágico
            effect.radius = 4;
            break;
            
        case VisualFeedbackType::ItemPickupGlow:
            effect.duration = 600;
            effect.color = { 255, 255, 100, 255 }; // Amarillo dorado
            effect.radius = 3;
            break;
            
        case VisualFeedbackType::LevelUpFlash:
            effect.duration = 1200;
            effect.color = { 255, 255, 255, 255 }; // Blanco brillante
            effect.radius = 10;
            break;
            
        case VisualFeedbackType::DeathFlash:
            effect.duration = 2000;
            effect.color = { 150, 0, 0, 255 }; // Rojo oscuro
            effect.radius = 12;
            break;
            
        default:
            effect.duration = 1000;
            effect.color = { 255, 255, 255, 255 };
            effect.radius = 5;
            break;
    }
    
    AddVisualEffect(effect);
    g_stats.contextualEffectsTriggered++;
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Contextual feedback triggered: type=" << static_cast<int>(type) 
                  << ", intensity=" << intensity << std::endl;
    }
#endif
}

void UpdateLowHealthPulse(const Player &player)
{
    if (!g_visualState.enabled) return;
    
    // Verificar si la vida está baja (menos del 25%)
    float healthRatio = static_cast<float>(player._pHitPoints) / player._pMaxHP;
    bool shouldPulse = healthRatio < 0.25f;
    
    if (shouldPulse && !g_visualState.lowHealthWarning) {
        // Activar pulse de vida baja
        VisualEffect effect;
        effect.type = VisualFeedbackType::PlayerLowHealthPulse;
        effect.startTime = SDL_GetTicks();
        effect.duration = 5000; // 5 segundos, se renovará si sigue con vida baja
        effect.position = player.position.tile;
        effect.active = true;
        effect.radius = 10;
        effect.pulseSpeed = 1.5f; // Pulse más lento para advertencia
        effect.color = { 255, 50, 50, 255 }; // Rojo de advertencia
        effect.intensity = 0.4f * g_visualState.globalIntensity;
        
        AddVisualEffect(effect);
        g_visualState.lowHealthWarning = true;
        
#ifdef _DEBUG
        if (g_debugLogging) {
            std::cout << "Low health pulse activated: health=" << healthRatio * 100 << "%" << std::endl;
        }
#endif
    } else if (!shouldPulse) {
        g_visualState.lowHealthWarning = false;
    }
}

void TriggerCriticalHitFlash(Point position, int damage)
{
    if (!g_visualState.enabled) return;
    
    VisualEffect effect;
    effect.type = VisualFeedbackType::CriticalHitFlash;
    effect.startTime = SDL_GetTicks();
    effect.duration = 400; // Flash rápido
    effect.position = position;
    effect.active = true;
    effect.radius = 5;
    effect.color = { 255, 255, 100, 255 }; // Amarillo brillante
    
    // Intensidad basada en el daño crítico
    float damageRatio = std::min(static_cast<float>(damage) / 200.0f, 1.0f);
    effect.intensity = 0.5f + (damageRatio * 0.5f);
    effect.intensity *= g_visualState.globalIntensity;
    
    AddVisualEffect(effect);
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Critical hit flash triggered: damage=" << damage 
                  << ", intensity=" << effect.intensity << std::endl;
    }
#endif
}

void TriggerSpellCastGlow(const Player &player, SpellID spellId)
{
    if (!g_visualState.enabled) return;
    
    // Color específico por tipo de hechizo
    SDL_Color spellColor = { 100, 200, 255, 255 }; // Azul por defecto
    
    // Personalizar color según el hechizo
    switch (spellId) {
        case SpellID::Firebolt:
        case SpellID::Fireball:
        case SpellID::Inferno:
            spellColor = { 255, 100, 50, 255 }; // Rojo-naranja para fuego
            break;
        case SpellID::Lightning:
        case SpellID::ChainLightning:
            spellColor = { 150, 150, 255, 255 }; // Azul para rayo
            break;
        case SpellID::HealOther:
        case SpellID::Healing:
            spellColor = { 100, 255, 100, 255 }; // Verde para curación
            break;
        default:
            spellColor = { 200, 150, 255, 255 }; // Púrpura mágico
            break;
    }
    
    TriggerContextualFeedback(VisualFeedbackType::SpellCastGlow, player.position.tile, 0.7f);
}

// ============================================================================
// 🎨 INTEGRACIÓN CON SISTEMA DE PALETAS
// ============================================================================

float GetEffectIntensityAtPosition(Point position, const VisualEffect &effect)
{
    if (!effect.active) return 0.0f;
    
    // Calcular distancia desde el centro del efecto
    int dx = position.x - effect.position.x;
    int dy = position.y - effect.position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    // Si está fuera del radio, no hay efecto
    if (distance > effect.radius) return 0.0f;
    
    // Calcular intensidad basada en la distancia (falloff cuadrático)
    float distanceRatio = distance / effect.radius;
    float falloff = 1.0f - (distanceRatio * distanceRatio);
    
    return effect.currentIntensity * falloff;
}

SDL_Color BlendVisualEffects(SDL_Color baseColor, Point position)
{
    if (!g_visualState.enabled || g_visualState.activeEffectCount == 0) {
        return baseColor;
    }
    
    float totalRed = baseColor.r;
    float totalGreen = baseColor.g;
    float totalBlue = baseColor.b;
    float totalIntensity = 0.0f;
    
    // Acumular efectos de todos los efectos activos
    for (int i = 0; i < g_visualState.activeEffectCount; i++) {
        const VisualEffect &effect = g_visualState.activeEffects[i];
        float intensity = GetEffectIntensityAtPosition(position, effect);
        
        if (intensity > 0.0f) {
            totalRed += effect.color.r * intensity;
            totalGreen += effect.color.g * intensity;
            totalBlue += effect.color.b * intensity;
            totalIntensity += intensity;
        }
    }
    
    // Normalizar si hay efectos aplicados
    if (totalIntensity > 0.0f) {
        // Mezcla aditiva con clamp
        totalRed = std::min(totalRed, 255.0f);
        totalGreen = std::min(totalGreen, 255.0f);
        totalBlue = std::min(totalBlue, 255.0f);
    }
    
    return {
        static_cast<uint8_t>(totalRed),
        static_cast<uint8_t>(totalGreen),
        static_cast<uint8_t>(totalBlue),
        baseColor.a
    };
}

void ApplyVisualFeedbackToPalette(SDL_Color *palette)
{
    if (!g_visualState.enabled || g_visualState.activeEffectCount == 0 || palette == nullptr) {
        return;
    }
    
    // Aplicar efectos a toda la paleta
    // Esto es una implementación simplificada que aplica un tinte global
    // En una implementación más avanzada, se podría aplicar por posición
    
    float globalRed = 0.0f;
    float globalGreen = 0.0f;
    float globalBlue = 0.0f;
    float globalIntensity = 0.0f;
    
    // Calcular tinte global promedio
    for (int i = 0; i < g_visualState.activeEffectCount; i++) {
        const VisualEffect &effect = g_visualState.activeEffects[i];
        if (effect.active && effect.currentIntensity > 0.0f) {
            float weight = effect.currentIntensity * 0.1f; // Factor de peso reducido
            globalRed += effect.color.r * weight;
            globalGreen += effect.color.g * weight;
            globalBlue += effect.color.b * weight;
            globalIntensity += weight;
        }
    }
    
    if (globalIntensity > 0.0f) {
        // Normalizar
        globalRed /= globalIntensity;
        globalGreen /= globalIntensity;
        globalBlue /= globalIntensity;
        
        // Aplicar tinte sutil a la paleta
        float tintStrength = std::min(globalIntensity, 0.3f); // Máximo 30% de tinte
        
        for (int i = 0; i < 256; i++) {
            palette[i].r = static_cast<uint8_t>(std::min(
                palette[i].r + (globalRed - palette[i].r) * tintStrength, 255.0f));
            palette[i].g = static_cast<uint8_t>(std::min(
                palette[i].g + (globalGreen - palette[i].g) * tintStrength, 255.0f));
            palette[i].b = static_cast<uint8_t>(std::min(
                palette[i].b + (globalBlue - palette[i].b) * tintStrength, 255.0f));
        }
    }
}

// ============================================================================
// 🔧 CONFIGURACIÓN Y PRESETS
// ============================================================================

void ApplyConservativeVisualPreset()
{
    g_visualState.enabled = true;
    g_visualState.globalIntensity = 0.5f;
    g_visualState.contextualEffects = true;
    
#ifdef _DEBUG
    std::cout << "Conservative visual preset applied" << std::endl;
#endif
}

void ApplyBalancedVisualPreset()
{
    // 🛡️ DISABLED: Visual feedback was causing corruption during transitions
    g_visualState.enabled = false;
    g_visualState.globalIntensity = 0.0f;
    g_visualState.contextualEffects = false;
    
#ifdef _DEBUG
    std::cout << "Balanced visual preset applied" << std::endl;
#endif
}

void ApplyIntenseVisualPreset()
{
    g_visualState.enabled = true;
    g_visualState.globalIntensity = 1.2f;
    g_visualState.contextualEffects = true;
    
#ifdef _DEBUG
    std::cout << "Intense visual preset applied" << std::endl;
#endif
}

void ApplyDebugVisualPreset()
{
    ApplyIntenseVisualPreset();
    
#ifdef _DEBUG
    g_debugLogging = true;
    std::cout << "Debug visual preset applied with logging enabled" << std::endl;
#endif
}

// ============================================================================
// 🎯 EFECTOS AVANZADOS
// ============================================================================

void TriggerDeathFlash(const Player &player)
{
    TriggerContextualFeedback(VisualFeedbackType::DeathFlash, player.position.tile, 1.0f);
}

void TriggerBossEncounterPulse(const Monster &boss)
{
    VisualEffect effect;
    effect.type = VisualFeedbackType::BossEncounterPulse;
    effect.startTime = SDL_GetTicks();
    effect.duration = 4000; // 4 segundos
    effect.position = boss.position.tile;
    effect.active = true;
    effect.radius = 12;
    effect.pulseSpeed = 1.0f;
    effect.color = { 200, 0, 200, 255 }; // Púrpura intimidante
    effect.intensity = 0.8f * g_visualState.globalIntensity;
    
    AddVisualEffect(effect);
}

void TriggerItemPickupGlow(Point position, const Item &item)
{
    float intensity = 0.5f;
    
    // Intensidad basada en la rareza del item
    if (item._iMagical == ITEM_QUALITY_MAGIC) {
        intensity = 0.6f;
    } else if (item._iMagical == ITEM_QUALITY_UNIQUE) {
        intensity = 0.9f;
    }
    
    TriggerContextualFeedback(VisualFeedbackType::ItemPickupGlow, position, intensity);
}

void TriggerLevelUpFlash(const Player &player)
{
    TriggerContextualFeedback(VisualFeedbackType::LevelUpFlash, player.position.tile, 1.0f);
}

// ============================================================================
// 📊 ESTADÍSTICAS Y MÉTRICAS
// ============================================================================

VisualFeedbackStats GetVisualFeedbackStats()
{
    return g_stats;
}

void ResetVisualFeedbackStats()
{
    g_stats = {};
    
#ifdef _DEBUG
    std::cout << "Visual feedback stats reset" << std::endl;
#endif
}

// ============================================================================
// 🔍 DEBUGGING Y DIAGNÓSTICO
// ============================================================================

#ifdef _DEBUG
void DebugPrintVisualFeedbackStats()
{
    std::cout << "\n=== VISUAL FEEDBACK STATISTICS ===" << std::endl;
    std::cout << "System enabled: " << (g_visualState.enabled ? "YES" : "NO") << std::endl;
    std::cout << "Global intensity: " << g_visualState.globalIntensity << std::endl;
    
    std::cout << "\n--- EFFECTS ---" << std::endl;
    std::cout << "Total triggered: " << g_stats.totalEffectsTriggered << std::endl;
    std::cout << "Currently active: " << g_stats.activeEffects << std::endl;
    std::cout << "Damage flashes: " << g_stats.damageFlashesTriggered << std::endl;
    std::cout << "Elite pulses: " << g_stats.elitePulsesTriggered << std::endl;
    std::cout << "Contextual effects: " << g_stats.contextualEffectsTriggered << std::endl;
    
    std::cout << "\n--- PERFORMANCE ---" << std::endl;
    std::cout << "Average intensity: " << std::fixed << std::setprecision(3) 
              << g_stats.averageIntensity << std::endl;
    std::cout << "Processing time: " << g_stats.averageProcessingTimeUs << "μs" << std::endl;
    
    std::cout << "=================================" << std::endl;
}

void SetVisualFeedbackDebugLogging(bool enabled)
{
    g_debugLogging = enabled;
    std::cout << "Visual feedback debug logging " << (enabled ? "enabled" : "disabled") << std::endl;
}

void RunVisualFeedbackStressTest()
{
    std::cout << "Running Visual Feedback Stress Test..." << std::endl;
    
    ResetVisualFeedbackStats();
    
    // Simular múltiples efectos
    for (int i = 0; i < 10; i++) {
        TriggerContextualFeedback(VisualFeedbackType::PlayerDamageFlash, {i, i}, 0.8f);
        TriggerContextualFeedback(VisualFeedbackType::CriticalHitFlash, {i+5, i+5}, 1.0f);
    }
    
    // Actualizar sistema varias veces
    for (int i = 0; i < 100; i++) {
        UpdateVisualFeedback();
        SDL_Delay(10); // Simular 10ms entre frames
    }
    
    DebugPrintVisualFeedbackStats();
    std::cout << "Stress test completed!" << std::endl;
}
#endif

} // namespace devilution