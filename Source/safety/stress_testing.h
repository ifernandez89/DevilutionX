#pragma once

/**
 * @file stress_testing.h
 * 
 * SAFETY LAYER - Phase 5: Stress Testing Execution Framework
 * 
 * Framework para ejecutar los protocolos de stress testing definidos.
 * - Tests automatizados para validación del Safety Layer
 * - Protocolos específicos para cada tipo de stress
 * - Validación automática de criterios de éxito
 * - Certificación del engine
 */

#include "safety_metrics.h"
#include <string>
#include <functional>
#include <vector>
#include <memory>

// Forward declaration for Point structure
struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

namespace devilution {

// ============================================================================
// 🧪 FRAMEWORK DE STRESS TESTING
// ============================================================================

/**
 * Clase base para todos los stress tests
 */
class StressTest {
public:
    StressTest(const std::string& name, int durationSeconds = 120)
        : testName(name), duration(durationSeconds) {}
    
    virtual ~StressTest() = default;
    
    /**
     * Ejecuta el stress test completo
     */
    bool Execute();
    
    /**
     * Obtiene el nombre del test
     */
    const std::string& GetName() const { return testName; }
    
    /**
     * Obtiene la duración del test en segundos
     */
    int GetDuration() const { return duration; }

protected:
    /**
     * Setup específico del test (implementado por subclases)
     */
    virtual bool SetupTest() = 0;
    
    /**
     * Ejecución del loop principal del test (implementado por subclases)
     */
    virtual bool ExecuteTestLoop(uint32_t elapsedTime) = 0;
    
    /**
     * Validación específica del test (implementado por subclases)
     */
    virtual bool ValidateTestResults() = 0;
    
    /**
     * Cleanup específico del test (implementado por subclases)
     */
    virtual void CleanupTest() {}

private:
    std::string testName;
    int duration;
};

// ============================================================================
// 🌋 INFERNO ESTACIONARIO TEST
// ============================================================================

/**
 * Test de resistencia extrema con Inferno spam continuo
 */
class InfernoStationaryTest : public StressTest {
public:
    InfernoStationaryTest() : StressTest("INFERNO_ESTACIONARIO", 180) {} // 3 minutos
    
protected:
    bool SetupTest() override;
    bool ExecuteTestLoop(uint32_t elapsedTime) override;
    bool ValidateTestResults() override;
    void CleanupTest() override;

private:
    uint32_t lastInfernoTime = 0;
    uint32_t lastMonsterSpawnTime = 0;
    size_t infernosCast = 0;
    size_t monstersSpawned = 0;
};

// ============================================================================
// ⚡ CHAIN LIGHTNING STORM TEST
// ============================================================================

/**
 * Test de multiplicación controlada con Chain Lightning
 */
class ChainLightningStormTest : public StressTest {
public:
    ChainLightningStormTest() : StressTest("CHAIN_LIGHTNING_STORM", 120) {} // 2 minutos
    
protected:
    bool SetupTest() override;
    bool ExecuteTestLoop(uint32_t elapsedTime) override;
    bool ValidateTestResults() override;
    void CleanupTest() override;

private:
    uint32_t lastChainTime = 0;
    size_t chainsCast = 0;
    size_t initialMonsterCount = 0;
};

// ============================================================================
// 🎯 MULTIMISSILE BARRAGE TEST
// ============================================================================

/**
 * Test de loops protegidos con multimissiles
 */
class MultimissileBarrageTest : public StressTest {
public:
    MultimissileBarrageTest() : StressTest("MULTIMISSILE_BARRAGE", 120) {} // 2 minutos
    
protected:
    bool SetupTest() override;
    bool ExecuteTestLoop(uint32_t elapsedTime) override;
    bool ValidateTestResults() override;
    void CleanupTest() override;

private:
    uint32_t lastBarrageTime = 0;
    size_t barragesCast = 0;
    size_t multimissileMonsters = 0;
};

// ============================================================================
// 🛡️ TRAP GAUNTLET TEST
// ============================================================================

/**
 * Test de throttling masivo con traps
 */
class TrapGauntletTest : public StressTest {
public:
    TrapGauntletTest() : StressTest("TRAP_GAUNTLET", 120) {} // 2 minutos
    
protected:
    bool SetupTest() override;
    bool ExecuteTestLoop(uint32_t elapsedTime) override;
    bool ValidateTestResults() override;
    void CleanupTest() override;

private:
    uint32_t lastTrapTime = 0;
    size_t trapsActivated = 0;
    size_t trapCount = 0;
};

// ============================================================================
// 🌪️ COMBINED CHAOS TEST
// ============================================================================

/**
 * Test de máxima presión con todos los efectos simultáneos
 */
class CombinedChaosTest : public StressTest {
public:
    CombinedChaosTest() : StressTest("COMBINED_CHAOS", 300) {} // 5 minutos
    
protected:
    bool SetupTest() override;
    bool ExecuteTestLoop(uint32_t elapsedTime) override;
    bool ValidateTestResults() override;
    void CleanupTest() override;

private:
    uint32_t lastInfernoTime = 0;
    uint32_t lastChainTime = 0;
    uint32_t lastBarrageTime = 0;
    uint32_t lastTrapTime = 0;
    
    size_t totalEffectsTriggered = 0;
    size_t maxSimultaneousEffects = 0;
};

// ============================================================================
// 🏆 STRESS TESTING SUITE
// ============================================================================

/**
 * Suite completa de stress testing para certificación del engine
 */
class StressTestingSuite {
public:
    StressTestingSuite();
    ~StressTestingSuite();
    
    /**
     * Ejecuta todos los tests individuales
     */
    bool RunIndividualTests();
    
    /**
     * Ejecuta tests combinados
     */
    bool RunCombinedTests();
    
    /**
     * Ejecuta el test definitivo (Combined Chaos)
     */
    bool RunDefinitiveTest();
    
    /**
     * Ejecuta la suite completa de certificación
     */
    bool RunFullCertification();
    
    /**
     * Obtiene el reporte final de certificación
     */
    std::string GetCertificationReport() const;

private:
    std::vector<std::unique_ptr<StressTest>> individualTests;
    std::vector<std::unique_ptr<StressTest>> combinedTests;
    std::unique_ptr<StressTest> definitiveTest;
    
    bool allIndividualPassed = false;
    bool allCombinedPassed = false;
    bool definitiveTestPassed = false;
    
    void InitializeTests();
    bool ExecuteTestBatch(const std::vector<std::unique_ptr<StressTest>>& tests, const std::string& batchName);
};

// ============================================================================
// 🎯 API SIMPLIFICADA PARA INTEGRACIÓN
// ============================================================================

/**
 * Ejecuta un stress test específico por nombre
 */
bool RunStressTest(const std::string& testName);

/**
 * Ejecuta la certificación completa del engine
 */
bool RunEngineCertification();

/**
 * Obtiene el estado actual de certificación
 */
std::string GetCurrentCertificationStatus();

/**
 * Verifica si el engine está listo para optimizaciones agresivas
 */
bool IsEngineReadyForOptimizations();

// ============================================================================
// 🔧 HELPERS PARA SIMULACIÓN DE GAMEPLAY
// ============================================================================

/**
 * Simula cast de Inferno en posición específica
 */
bool SimulateInfernoCast(Point position);

/**
 * Simula cast de Chain Lightning en pack de monsters
 */
bool SimulateChainLightningCast(Point position);

/**
 * Simula ataque multimissile de monsters
 */
bool SimulateMultimissileBarrage(size_t monsterCount);

/**
 * Simula activación masiva de traps
 */
bool SimulateTrapActivation(size_t trapCount);

/**
 * Spawna pack de monsters para testing
 */
bool SpawnTestMonsterPack(Point center, size_t count, int monsterType = -1);

/**
 * Limpia todos los entities de testing
 */
void CleanupTestEntities();

} // namespace devilution