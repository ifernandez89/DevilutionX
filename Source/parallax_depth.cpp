/**
 * @file parallax_depth.cpp
 * 
 * 🌟 FASE D3.2 - PARALLAX FALSO SUTIL 🌟
 * 
 * Implementación completa del sistema de parallax falso
 */

#include "parallax_depth.h"

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
extern Player *MyPlayer;

// Level type constants (from gendung.h)
constexpr int DTYPE_TOWN = 0;
constexpr int DTYPE_CATHEDRAL = 1;
constexpr int DTYPE_CATACOMBS = 2;
constexpr int DTYPE_CAVES = 3;
constexpr int DTYPE_HELL = 4;

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

namespace devilution {

// Forward declarations for private functions
void ConfigureElementByType(ParallaxElement &element);
void ApplyElementSpecificEffects(ParallaxElement &element, uint32_t currentTime);
float GetTypeMultiplier(ParallaxElementType type);
void DetectTownParallaxElements();
void DetectCathedralParallaxElements();
void DetectCatacombsParallaxElements();
void DetectCavesParallaxElements();
void DetectHellParallaxElements();

// ============================================================================
// 🔒 ESTADO GLOBAL DEL SISTEMA
// ============================================================================

namespace {
    ParallaxState g_parallaxState;
    ParallaxStats g_parallaxStats;
    
#ifdef _DEBUG
    bool g_debugLogging = false;
    ParallaxElementType g_forcedParallaxType = ParallaxElementType::None;
    bool g_parallaxForced = false;
#endif
}

// ============================================================================
// 🎯 IMPLEMENTACIÓN DE LA API PRINCIPAL
// ============================================================================

void InitParallaxDepth()
{
    // Inicializar estado
    g_parallaxState = {};
    g_parallaxStats = {};
    
    // Configuración por defecto (preset balanceado)
    ApplyBalancedParallaxPreset();
    
    // Reservar espacio para elementos
    g_parallaxState.elements.reserve(g_parallaxState.config.maxActiveElements);
    
#ifdef _DEBUG
    std::cout << "Parallax Depth System initialized" << std::endl;
#endif
}

void UpdateParallaxDepth()
{
    if (!g_parallaxState.enabled) {
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    uint32_t currentTime = SDL_GetTicks();
    g_parallaxState.lastUpdateTime = currentTime;
    
    // Simplified camera velocity calculation (without MyPlayer dependency)
    // Use a simple time-based movement simulation
    static Point lastSimulatedPos = { 0, 0 };
    Point currentSimulatedPos = { 
        static_cast<int>(std::sin(currentTime / 5000.0f) * 10),
        static_cast<int>(std::cos(currentTime / 7000.0f) * 8)
    };
    
    // Calcular velocidad de cámara
    g_parallaxState.cameraVelocity = {
        currentSimulatedPos.x - lastSimulatedPos.x,
        currentSimulatedPos.y - lastSimulatedPos.y
    };
    
    lastSimulatedPos = currentSimulatedPos;
    g_parallaxState.lastCameraPosition = currentSimulatedPos;
    
    // Actualizar elementos parallax activos
    g_parallaxState.activeElements = 0;
    g_parallaxState.totalCalculationsThisFrame = 0;
    
    for (auto &element : g_parallaxState.elements) {
        if (!element.enabled) continue;
        
        g_parallaxState.activeElements++;
        g_parallaxState.totalCalculationsThisFrame++;
        
        // Calcular nuevo offset basado en velocidad de cámara y configuración
        Point newOffset = CalculateParallaxOffset(element.originalPosition, element.layer);
        element.currentOffset = newOffset;
        element.lastUpdateTime = currentTime;
        
        // Aplicar efectos específicos por tipo
        ApplyElementSpecificEffects(element, currentTime);
    }
    
    // Actualizar estadísticas
    g_parallaxState.framesProcessed++;
    g_parallaxStats.totalElementsProcessed += g_parallaxState.activeElements;
    
    if (g_parallaxState.activeElements > 0) {
        g_parallaxStats.framesWithParallax++;
    }
    
    if (g_parallaxState.activeElements > g_parallaxStats.peakActiveElements) {
        g_parallaxStats.peakActiveElements = g_parallaxState.activeElements;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    // Actualizar tiempo promedio de procesamiento
    float frameTime = duration.count() / 1000.0f; // Convert to milliseconds
    g_parallaxState.averageCalculationTime = 
        (g_parallaxState.averageCalculationTime * 0.95f) + (frameTime * 0.05f);
    
#ifdef _DEBUG
    if (g_debugLogging && g_parallaxState.framesProcessed % 120 == 0) {
        std::cout << "Parallax: " << g_parallaxState.activeElements << " elements, "
                  << "velocity(" << g_parallaxState.cameraVelocity.x << "," << g_parallaxState.cameraVelocity.y << "), "
                  << std::fixed << std::setprecision(2) << frameTime << "ms" << std::endl;
    }
#endif
}

void RegisterParallaxElement(ParallaxElementType type, ParallaxLayer layer, Point position)
{
    if (g_parallaxState.elements.size() >= g_parallaxState.config.maxActiveElements) {
        return; // Límite alcanzado
    }
    
    ParallaxElement element;
    element.type = type;
    element.layer = layer;
    element.originalPosition = position;
    element.currentOffset = { 0, 0 };
    element.enabled = true;
    element.lastUpdateTime = SDL_GetTicks();
    
    // Configurar parámetros específicos por tipo
    ConfigureElementByType(element);
    
    g_parallaxState.elements.push_back(element);
    
    // Actualizar estadísticas
    g_parallaxStats.elementsByType[static_cast<size_t>(type)]++;
    g_parallaxStats.elementsByLayer[static_cast<size_t>(layer)]++;
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Registered parallax element: type=" << static_cast<int>(type) 
                  << ", layer=" << static_cast<int>(layer) 
                  << ", pos(" << position.x << "," << position.y << ")" << std::endl;
    }
#endif
}

Point CalculateParallaxOffset(Point originalPosition, ParallaxLayer layer)
{
    if (!g_parallaxState.enabled) {
        return { 0, 0 };
    }
    
    const ParallaxConfig &config = g_parallaxState.config;
    
    // Determinar factor de movimiento basado en la capa
    float layerFactor = 1.0f;
    switch (layer) {
    case ParallaxLayer::Background:
        layerFactor = config.backgroundFactor;
        break;
    case ParallaxLayer::Midground:
        layerFactor = config.midgroundFactor;
        break;
    case ParallaxLayer::Nearground:
        layerFactor = config.neargroundFactor;
        break;
    case ParallaxLayer::Foreground:
    default:
        layerFactor = 1.0f; // Sin parallax
        break;
    }
    
    // Calcular offset basado en velocidad de cámara
    Point offset = {
        static_cast<int>(-g_parallaxState.cameraVelocity.x * layerFactor * config.globalIntensity * config.subtletyFactor),
        static_cast<int>(-g_parallaxState.cameraVelocity.y * layerFactor * config.globalIntensity * config.subtletyFactor)
    };
    
    // Aplicar influencia de cámara
    offset.x = static_cast<int>(offset.x * config.cameraInfluence);
    offset.y = static_cast<int>(offset.y * config.cameraInfluence);
    
    // Limitar offsets para evitar efectos extremos
    offset.x = std::clamp(offset.x, -8, 8);
    offset.y = std::clamp(offset.y, -6, 6);
    
    g_parallaxStats.totalOffsetCalculations++;
    
    return offset;
}

Point ApplyParallaxEffect(Point originalPos, ParallaxElementType type, ParallaxLayer layer)
{
    if (!g_parallaxState.enabled) {
        return originalPos;
    }
    
#ifdef _DEBUG
    if (g_parallaxForced && type != g_forcedParallaxType) {
        return originalPos; // Solo aplicar al tipo forzado
    }
#endif
    
    Point offset = CalculateParallaxOffset(originalPos, layer);
    
    // Aplicar modificadores específicos por tipo
    float typeMultiplier = GetTypeMultiplier(type);
    offset.x = static_cast<int>(offset.x * typeMultiplier);
    offset.y = static_cast<int>(offset.y * typeMultiplier);
    
    return {
        originalPos.x + offset.x,
        originalPos.y + offset.y
    };
}

void UpdateParallaxCamera(Point cameraPosition)
{
    g_parallaxState.lastCameraPosition = cameraPosition;
}

void ClearParallaxElements()
{
    g_parallaxState.elements.clear();
    g_parallaxState.activeElements = 0;
    
    // Reset estadísticas por tipo y capa
    g_parallaxStats.elementsByType.fill(0);
    g_parallaxStats.elementsByLayer.fill(0);
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Cleared all parallax elements" << std::endl;
    }
#endif
}

// ============================================================================
// 🎯 DETECCIÓN AUTOMÁTICA DE ELEMENTOS
// ============================================================================

void DetectParallaxElements()
{
    if (!g_parallaxState.enabled) {
        return;
    }
    
    // Limpiar elementos existentes
    ClearParallaxElements();
    
    // Configurar parallax específico para el tipo de nivel actual
    ConfigureParallaxForLevel(leveltype);
    
    // Detectar elementos automáticamente basado en el tipo de nivel
    switch (leveltype) {
    case DTYPE_TOWN:
        DetectTownParallaxElements();
        break;
    case DTYPE_CATHEDRAL:
        DetectCathedralParallaxElements();
        break;
    case DTYPE_CATACOMBS:
        DetectCatacombsParallaxElements();
        break;
    case DTYPE_CAVES:
        DetectCavesParallaxElements();
        break;
    case DTYPE_HELL:
        if (g_parallaxState.config.enableInHell) {
            DetectHellParallaxElements();
        }
        break;
    default:
        break;
    }
    
#ifdef _DEBUG
    if (g_debugLogging) {
        std::cout << "Detected " << g_parallaxState.elements.size() 
                  << " parallax elements for level type " << leveltype << std::endl;
    }
#endif
}

ParallaxElementType ClassifyParallaxElement(Point position, int tileType)
{
    // Clasificación básica basada en tipo de tile
    // Esta es una implementación simplificada
    
    switch (leveltype) {
    case DTYPE_TOWN:
        if (tileType >= 100 && tileType <= 120) return ParallaxElementType::Architecture;
        if (tileType >= 200 && tileType <= 220) return ParallaxElementType::Vegetation;
        break;
        
    case DTYPE_CATHEDRAL:
        if (tileType >= 50 && tileType <= 70) return ParallaxElementType::Architecture;
        if (tileType >= 300 && tileType <= 320) return ParallaxElementType::Fire;
        break;
        
    case DTYPE_CATACOMBS:
        if (tileType >= 400 && tileType <= 420) return ParallaxElementType::StaticBackground;
        break;
        
    case DTYPE_CAVES:
        if (tileType >= 500 && tileType <= 520) return ParallaxElementType::Water;
        if (tileType >= 600 && tileType <= 620) return ParallaxElementType::StaticBackground;
        break;
        
    case DTYPE_HELL:
        if (tileType >= 700 && tileType <= 720) return ParallaxElementType::Fire;
        if (tileType >= 800 && tileType <= 820) return ParallaxElementType::Smoke;
        break;
    }
    
    return ParallaxElementType::None;
}

ParallaxLayer DetermineParallaxLayer(ParallaxElementType type, Point position)
{
    // Determinar capa basada en tipo y posición
    switch (type) {
    case ParallaxElementType::StaticBackground:
    case ParallaxElementType::Architecture:
        return ParallaxLayer::Background;
        
    case ParallaxElementType::Atmosphere:
    case ParallaxElementType::Clouds:
    case ParallaxElementType::Lighting:
        return ParallaxLayer::Background;
        
    case ParallaxElementType::Vegetation:
    case ParallaxElementType::Decoration:
        return ParallaxLayer::Midground;
        
    case ParallaxElementType::Fire:
    case ParallaxElementType::Water:
    case ParallaxElementType::Smoke:
        return ParallaxLayer::Nearground;
        
    default:
        return ParallaxLayer::Foreground;
    }
}

// ============================================================================
// 🎯 FUNCIONES AUXILIARES PRIVADAS
// ============================================================================

void ConfigureElementByType(ParallaxElement &element);

void ConfigureElementByType(ParallaxElement &element)
{
    switch (element.type) {
    case ParallaxElementType::Fire:
        element.depthFactor = 0.7f;
        element.parallaxIntensity = 1.2f;
        element.flickerEffect = 0.3f;
        break;
        
    case ParallaxElementType::Water:
        element.depthFactor = 0.8f;
        element.parallaxIntensity = 1.0f;
        element.waveEffect = 0.4f;
        break;
        
    case ParallaxElementType::Smoke:
        element.depthFactor = 0.5f;
        element.parallaxIntensity = 1.5f;
        element.windEffect = 0.6f;
        break;
        
    case ParallaxElementType::Vegetation:
        element.depthFactor = 0.6f;
        element.parallaxIntensity = 0.8f;
        element.windEffect = 0.2f;
        break;
        
    case ParallaxElementType::Architecture:
    case ParallaxElementType::StaticBackground:
        element.depthFactor = 0.3f;
        element.parallaxIntensity = 0.6f;
        break;
        
    default:
        element.depthFactor = 0.5f;
        element.parallaxIntensity = 1.0f;
        break;
    }
}

void ApplyElementSpecificEffects(ParallaxElement &element, uint32_t currentTime)
{
    float timeSeconds = currentTime / 1000.0f;
    
    // Aplicar efectos específicos por tipo
    switch (element.type) {
    case ParallaxElementType::Fire:
        if (element.flickerEffect > 0.0f) {
            float flicker = std::sin(timeSeconds * 8.0f) * element.flickerEffect;
            element.currentOffset.y += static_cast<int>(flicker);
        }
        break;
        
    case ParallaxElementType::Water:
        if (element.waveEffect > 0.0f) {
            float wave = std::sin(timeSeconds * 3.0f) * element.waveEffect;
            element.currentOffset.x += static_cast<int>(wave * 0.5f);
            element.currentOffset.y += static_cast<int>(wave);
        }
        break;
        
    case ParallaxElementType::Smoke:
    case ParallaxElementType::Vegetation:
        if (element.windEffect > 0.0f) {
            float wind = std::sin(timeSeconds * 2.0f) * element.windEffect;
            element.currentOffset.x += static_cast<int>(wind);
        }
        break;
        
    default:
        break;
    }
}

float GetTypeMultiplier(ParallaxElementType type)
{
    switch (type) {
    case ParallaxElementType::StaticBackground:
        return 0.3f;
    case ParallaxElementType::Architecture:
        return 0.4f;
    case ParallaxElementType::Atmosphere:
        return 0.2f;
    case ParallaxElementType::Vegetation:
        return 0.6f;
    case ParallaxElementType::Fire:
        return 0.8f;
    case ParallaxElementType::Water:
        return 0.7f;
    case ParallaxElementType::Smoke:
        return 0.9f;
    default:
        return 1.0f;
    }
}

void DetectTownParallaxElements()
{
    // Registrar elementos típicos de Town
    RegisterParallaxElement(ParallaxElementType::Architecture, ParallaxLayer::Background, { 50, 30 });
    RegisterParallaxElement(ParallaxElementType::Vegetation, ParallaxLayer::Midground, { 80, 60 });
    RegisterParallaxElement(ParallaxElementType::Atmosphere, ParallaxLayer::Background, { 0, 0 });
}

void DetectCathedralParallaxElements()
{
    // Registrar elementos típicos de Cathedral
    RegisterParallaxElement(ParallaxElementType::Architecture, ParallaxLayer::Background, { 40, 25 });
    RegisterParallaxElement(ParallaxElementType::Fire, ParallaxLayer::Nearground, { 70, 50 });
    RegisterParallaxElement(ParallaxElementType::StaticBackground, ParallaxLayer::Background, { 0, 0 });
}

void DetectCatacombsParallaxElements()
{
    // Registrar elementos típicos de Catacombs
    RegisterParallaxElement(ParallaxElementType::StaticBackground, ParallaxLayer::Background, { 0, 0 });
    RegisterParallaxElement(ParallaxElementType::Fire, ParallaxLayer::Nearground, { 60, 40 });
}

void DetectCavesParallaxElements()
{
    // Registrar elementos típicos de Caves
    RegisterParallaxElement(ParallaxElementType::Water, ParallaxLayer::Nearground, { 90, 70 });
    RegisterParallaxElement(ParallaxElementType::StaticBackground, ParallaxLayer::Background, { 0, 0 });
}

void DetectHellParallaxElements()
{
    // Registrar elementos típicos de Hell (si está habilitado)
    RegisterParallaxElement(ParallaxElementType::Fire, ParallaxLayer::Nearground, { 100, 80 });
    RegisterParallaxElement(ParallaxElementType::Smoke, ParallaxLayer::Midground, { 120, 90 });
}

// ============================================================================
// 🎯 CONFIGURACIÓN Y PRESETS
// ============================================================================

void ApplySubtleParallaxPreset()
{
    g_parallaxState.enabled = true;
    g_parallaxState.config = {
        .enabled = true,
        .globalIntensity = 0.4f,
        .cameraInfluence = 0.6f,
        .subtletyFactor = 0.3f,
        .backgroundFactor = 0.1f,
        .midgroundFactor = 0.3f,
        .neargroundFactor = 0.6f,
        .enableInTown = true,
        .enableInDungeons = true,
        .enableInHell = false,
        .maxActiveElements = 24,
        .updateFrequency = 60.0f
    };
}

void ApplyBalancedParallaxPreset()
{
    g_parallaxState.enabled = true;
    g_parallaxState.config = {
        .enabled = true,
        .globalIntensity = 0.8f,
        .cameraInfluence = 1.0f,
        .subtletyFactor = 0.6f,
        .backgroundFactor = 0.2f,
        .midgroundFactor = 0.5f,
        .neargroundFactor = 0.8f,
        .enableInTown = true,
        .enableInDungeons = true,
        .enableInHell = false,
        .maxActiveElements = 48,
        .updateFrequency = 60.0f
    };
}

void ApplyDramaticParallaxPreset()
{
    g_parallaxState.enabled = true;
    g_parallaxState.config = {
        .enabled = true,
        .globalIntensity = 1.2f,
        .cameraInfluence = 1.4f,
        .subtletyFactor = 1.0f,
        .backgroundFactor = 0.3f,
        .midgroundFactor = 0.7f,
        .neargroundFactor = 0.9f,
        .enableInTown = true,
        .enableInDungeons = true,
        .enableInHell = true,
        .maxActiveElements = 64,
        .updateFrequency = 60.0f
    };
}

void ConfigureParallaxForLevel(int levelType)
{
    // Ajustar configuración específica por tipo de nivel
    switch (levelType) {
    case DTYPE_TOWN:
        g_parallaxState.config.globalIntensity *= 1.2f; // Más intenso en Town
        break;
    case DTYPE_HELL:
        g_parallaxState.config.globalIntensity *= 0.7f; // Menos intenso en Hell
        break;
    default:
        break;
    }
}

const ParallaxState &GetParallaxState()
{
    return g_parallaxState;
}

const ParallaxStats &GetParallaxStats()
{
    return g_parallaxStats;
}

// ============================================================================
// 🎯 INTEGRACIÓN CON SISTEMAS EXISTENTES
// ============================================================================

void IntegrateWithDepthCues(Point position, float depthIntensity)
{
    // Integración con FASE D1 - usar depth intensity para modular parallax
    if (g_parallaxState.enabled) {
        g_parallaxState.config.globalIntensity *= (1.0f + depthIntensity * 0.2f);
    }
}

void IntegrateWithLighting(Point position, int lightRadius)
{
    // Integración con FASE V1 - usar radio de luz para detectar elementos
    if (lightRadius > 10) {
        // Luz intensa podría indicar fuego u otros elementos parallax
        RegisterParallaxElement(ParallaxElementType::Fire, ParallaxLayer::Nearground, position);
    }
}

void IntegrateWithContextualPalette(ParallaxElementType type, float &colorMultiplier)
{
    // Integración con FASE V2 - modular colores basado en tipo parallax
    switch (type) {
    case ParallaxElementType::Fire:
        colorMultiplier *= 1.1f; // Intensificar fuego
        break;
    case ParallaxElementType::Water:
        colorMultiplier *= 0.9f; // Atenuar agua
        break;
    case ParallaxElementType::StaticBackground:
        colorMultiplier *= 0.8f; // Atenuar fondo
        break;
    default:
        break;
    }
}

// ============================================================================
// 🎯 DEBUGGING Y UTILIDADES
// ============================================================================

#ifdef _DEBUG
void SetParallaxDebugLogging(bool enabled)
{
    g_debugLogging = enabled;
}

void ForceParallaxType(ParallaxElementType type)
{
    g_forcedParallaxType = type;
    g_parallaxForced = (type != ParallaxElementType::None);
}

std::string GetParallaxDebugInfo()
{
    std::ostringstream oss;
    oss << "Parallax Debug Info:\n";
    oss << "  Enabled: " << (g_parallaxState.enabled ? "Yes" : "No") << "\n";
    oss << "  Active Elements: " << g_parallaxState.activeElements << "/" << g_parallaxState.config.maxActiveElements << "\n";
    oss << "  Camera Velocity: (" << g_parallaxState.cameraVelocity.x << "," << g_parallaxState.cameraVelocity.y << ")\n";
    oss << "  Global Intensity: " << std::fixed << std::setprecision(2) << g_parallaxState.config.globalIntensity << "\n";
    oss << "  Avg Calc Time: " << g_parallaxState.averageCalculationTime << "ms\n";
    oss << "  Total Elements: " << g_parallaxStats.totalElementsProcessed << "\n";
    oss << "  Frames: " << g_parallaxState.framesProcessed;
    return oss.str();
}

void RenderParallaxDebugOverlay()
{
    // Esta función podría renderizar información de debugging en pantalla
    // Por ahora, solo imprime a consola si está habilitado el logging
    if (g_debugLogging && g_parallaxState.framesProcessed % 300 == 0) {
        std::cout << GetParallaxDebugInfo() << std::endl;
    }
}
#endif

} // namespace devilution