/**
 * @file global_protection_system.h
 * 
 * GLOBAL PROTECTION SYSTEM - ARQUITECTURA AVANZADA 2026
 * 
 * Sistema centralizado de protección que previene crashes de cualquier tipo:
 * - Spells (Apocalypse, Inferno, etc.)
 * - Melee attacks y combos
 * - Missile overflow y loops infinitos
 * - Memory corruption y buffer overflows
 * - Animation locks y player mode issues
 * - Resource exhaustion y performance degradation
 * 
 * CARACTERÍSTICAS:
 * - Monitoreo en tiempo real de recursos críticos
 * - Límites adaptativos basados en performance
 * - Recuperación automática de estados problemáticos
 * - Logging inteligente para debugging
 * - Zero-overhead en condiciones normales
 * 
 * FILOSOFÍA: "Prevenir es mejor que curar"
 * El sistema actúa como un "immune system" para el game engine.
 */

#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <unordered_map>

namespace devilution {

// ============================================================================
// 🛡️ GLOBAL PROTECTION SYSTEM
// ============================================================================

/**
 * Tipos de recursos críticos que el sistema monitorea
 */
enum class CriticalResource {
    MISSILES,           // Missiles activos en el sistema
    SPELLS,            // Spell casts por segundo
    MELEE_ATTACKS,     // Ataques melee por segundo
    ANIMATIONS,        // Animaciones activas
    MEMORY_USAGE,      // Uso de memoria estimado
    FRAME_TIME,        // Tiempo de frame para performance
    PLAYER_ACTIONS     // Acciones del jugador por segundo
};

/**
 * Niveles de alerta del sistema
 */
enum class AlertLevel {
    NORMAL = 0,        // Todo funcionando normalmente
    CAUTION = 1,       // Uso elevado pero controlado
    WARNING = 2,       // Acercándose a límites peligrosos
    CRITICAL = 3,      // Intervención inmediata requerida
    EMERGENCY = 4      // Forzar reset/cleanup inmediato
};

/**
 * Configuración adaptativa de límites
 */
struct AdaptiveLimits {
    int maxMissiles = 80;           // Máximo missiles simultáneos
    int maxSpellsPerSecond = 15;    // Máximo spells por segundo
    int maxMeleePerSecond = 20;     // Máximo ataques melee por segundo
    int maxActionsPerSecond = 30;   // Máximo acciones totales por segundo
    int emergencyResetThreshold = 5; // Frames en estado crítico antes de reset
    
    // Límites se ajustan automáticamente basado en performance
    void adaptToPerformance(int performanceScore);
};

/**
 * Estadísticas en tiempo real del sistema
 */
struct SystemStats {
    int currentMissiles = 0;
    int spellsThisSecond = 0;
    int meleeAttacksThisSecond = 0;
    int totalActionsThisSecond = 0;
    int averageFrameTime = 16; // ms
    int performanceScore = 100; // 0-100
    AlertLevel currentAlert = AlertLevel::NORMAL;
    
    // Timestamps para tracking
    std::chrono::steady_clock::time_point lastSecondReset;
    std::chrono::steady_clock::time_point lastFrameTime;
};

/**
 * Sistema Global de Protección
 * 
 * Singleton que monitorea todos los aspectos críticos del juego
 * y previene crashes mediante intervención inteligente.
 */
class GlobalProtectionSystem {
private:
    static GlobalProtectionSystem* instance;
    
    AdaptiveLimits limits;
    SystemStats stats;
    
    // Contadores por tipo de acción
    std::unordered_map<std::string, int> actionCounters;
    
    // Estado de emergencia
    bool emergencyMode = false;
    int emergencyFrameCount = 0;
    
    GlobalProtectionSystem() = default;
    
public:
    static GlobalProtectionSystem& getInstance();
    
    // ========================================================================
    // 🔍 MONITOREO Y ANÁLISIS
    // ========================================================================
    
    /**
     * Actualiza estadísticas del sistema cada frame
     */
    void updateFrameStats();
    
    /**
     * Registra una acción crítica (spell, melee, etc.)
     */
    bool registerAction(const std::string& actionType, const std::string& details = "");
    
    /**
     * Verifica si una acción es segura de ejecutar
     */
    bool isActionSafe(const std::string& actionType);
    
    /**
     * Obtiene el nivel de alerta actual
     */
    AlertLevel getCurrentAlertLevel();
    
    // ========================================================================
    // 🛡️ PROTECCIÓN ESPECÍFICA
    // ========================================================================
    
    /**
     * Protección para spell casting
     */
    bool canCastSpell(const std::string& spellName);
    
    /**
     * Protección para ataques melee
     */
    bool canPerformMeleeAttack();
    
    /**
     * Protección para creación de missiles
     */
    bool canCreateMissile(const std::string& missileType);
    
    /**
     * Protección para cambios de player mode
     */
    bool canChangePlayerMode(const std::string& newMode);
    
    // ========================================================================
    // 🚨 GESTIÓN DE EMERGENCIAS
    // ========================================================================
    
    /**
     * Activa modo de emergencia
     */
    void activateEmergencyMode(const std::string& reason);
    
    /**
     * Desactiva modo de emergencia
     */
    void deactivateEmergencyMode();
    
    /**
     * Fuerza limpieza de recursos críticos
     */
    void forceResourceCleanup();
    
    /**
     * Reset completo del sistema
     */
    void emergencyReset();
    
    // ========================================================================
    // 📊 ESTADÍSTICAS Y DEBUGGING
    // ========================================================================
    
    /**
     * Obtiene estadísticas actuales
     */
    const SystemStats& getStats() const { return stats; }
    
    /**
     * Obtiene límites actuales
     */
    const AdaptiveLimits& getLimits() const { return limits; }
    
    /**
     * Genera reporte de estado
     */
    std::string generateStatusReport();
    
    /**
     * Log de evento crítico
     */
    void logCriticalEvent(const std::string& event, AlertLevel level);
};

// ============================================================================
// 🔧 MACROS DE CONVENIENCIA
// ============================================================================

#define GPS GlobalProtectionSystem::getInstance()

// Macros para protección rápida
#define SAFE_SPELL_CAST(spellName) \
    if (!GPS.canCastSpell(spellName)) { \
        GPS.logCriticalEvent("Blocked unsafe spell: " + std::string(spellName), AlertLevel::WARNING); \
        return false; \
    }

#define SAFE_MELEE_ATTACK() \
    if (!GPS.canPerformMeleeAttack()) { \
        GPS.logCriticalEvent("Blocked unsafe melee attack", AlertLevel::WARNING); \
        return false; \
    }

#define SAFE_MISSILE_CREATE(missileType) \
    if (!GPS.canCreateMissile(missileType)) { \
        GPS.logCriticalEvent("Blocked unsafe missile: " + std::string(missileType), AlertLevel::WARNING); \
        return nullptr; \
    }

#define REGISTER_ACTION(actionType, details) \
    GPS.registerAction(actionType, details)

} // namespace devilution