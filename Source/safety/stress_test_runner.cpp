/**
 * @file stress_test_runner.cpp
 * 
 * SAFETY LAYER - Phase 5: Simple Test Runner for Manual Execution
 * 
 * Ejecutor simple para stress tests que puede ser llamado manualmente
 * o integrado en el juego para validación del Safety Layer.
 */

#include "stress_testing.h"
#include "safety.h"

#include <iostream>
#include <string>

namespace devilution {

// ============================================================================
// 🧪 SIMPLE TEST RUNNER
// ============================================================================

/**
 * Ejecuta un test específico y muestra resultados
 */
void RunSingleTest(const std::string& testName) {
    std::cout << "=== EXECUTING STRESS TEST: " << testName << " ===" << std::endl;
    
    bool result = RunStressTest(testName);
    
    std::cout << "Test Result: " << (result ? "PASSED" : "FAILED") << std::endl;
    std::cout << "Safety Status: " << GetSafetyStatusReport() << std::endl;
    std::cout << "Certification Status: " << GetCurrentCertificationStatus() << std::endl;
    std::cout << "========================================" << std::endl;
}

/**
 * Ejecuta la certificación completa del engine
 */
void RunFullCertification() {
    std::cout << "=== STARTING FULL ENGINE CERTIFICATION ===" << std::endl;
    
    bool result = RunEngineCertification();
    
    std::cout << "Certification Result: " << (result ? "BULLETPROOF CERTIFIED" : "CERTIFICATION FAILED") << std::endl;
    std::cout << "Final Status: " << GetCurrentCertificationStatus() << std::endl;
    std::cout << "Ready for Optimizations: " << (IsEngineReadyForOptimizations() ? "YES" : "NO") << std::endl;
    std::cout << "========================================" << std::endl;
}

/**
 * Muestra el menú de opciones de testing
 */
void ShowTestMenu() {
    std::cout << "\n=== SAFETY LAYER STRESS TESTING MENU ===" << std::endl;
    std::cout << "1. Run Inferno Stationary Test (3 minutes)" << std::endl;
    std::cout << "2. Run Combined Chaos Test (5 minutes)" << std::endl;
    std::cout << "3. Run Full Engine Certification" << std::endl;
    std::cout << "4. Show Current Safety Status" << std::endl;
    std::cout << "5. Show Certification Status" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Choose option: ";
}

/**
 * Muestra el estado actual del Safety Layer
 */
void ShowSafetyStatus() {
    std::cout << "=== CURRENT SAFETY STATUS ===" << std::endl;
    std::cout << GetSafetyStatusReport() << std::endl;
    std::cout << "=============================" << std::endl;
}

/**
 * Muestra el estado de certificación actual
 */
void ShowCertificationStatus() {
    std::cout << "=== CERTIFICATION STATUS ===" << std::endl;
    std::cout << GetCurrentCertificationStatus() << std::endl;
    std::cout << "============================" << std::endl;
}

/**
 * Loop principal del test runner interactivo
 */
void RunInteractiveTestRunner() {
    int choice;
    
    std::cout << "SAFETY LAYER - Phase 5: Stress Testing Framework" << std::endl;
    std::cout << "Engine Certification and Validation System" << std::endl;
    
    do {
        ShowTestMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                RunSingleTest("INFERNO_ESTACIONARIO");
                break;
                
            case 2:
                RunSingleTest("COMBINED_CHAOS");
                break;
                
            case 3:
                RunFullCertification();
                break;
                
            case 4:
                ShowSafetyStatus();
                break;
                
            case 5:
                ShowCertificationStatus();
                break;
                
            case 0:
                std::cout << "Exiting stress test runner..." << std::endl;
                break;
                
            default:
                std::cout << "Invalid option. Please try again." << std::endl;
                break;
        }
        
    } while (choice != 0);
}

// ============================================================================
// 🎯 API PARA INTEGRACIÓN CON EL JUEGO
// ============================================================================

/**
 * Ejecuta un test rápido para verificar que el Safety Layer funciona
 */
bool RunQuickSafetyCheck() {
    std::cout << "Running quick safety check..." << std::endl;
    
    // Actualizar métricas
    UpdateSafetyMetrics();
    
    // Verificar que el Safety Layer está funcionando
    SafetyStatus status = GetSafetyStatus();
    
    bool basicFunctionality = 
        (status.monsterUsage >= 0 && status.monsterUsage <= 100) &&
        (status.missileUsage >= 0 && status.missileUsage <= 100);
    
    if (basicFunctionality) {
        std::cout << "Quick safety check: PASSED" << std::endl;
        std::cout << "Monster usage: " << status.monsterUsage << "%" << std::endl;
        std::cout << "Missile usage: " << status.missileUsage << "%" << std::endl;
        std::cout << "In danger zone: " << (status.inDangerZone ? "YES" : "NO") << std::endl;
    } else {
        std::cout << "Quick safety check: FAILED" << std::endl;
    }
    
    return basicFunctionality;
}

/**
 * Ejecuta un test de stress ligero (30 segundos)
 */
bool RunLightStressTest() {
    std::cout << "Running light stress test (30 seconds)..." << std::endl;
    
    // Crear un test de Inferno corto
    InfernoStationaryTest lightTest;
    // TODO: Modificar duración a 30 segundos
    
    bool result = lightTest.Execute();
    
    std::cout << "Light stress test: " << (result ? "PASSED" : "FAILED") << std::endl;
    
    return result;
}

/**
 * Función de conveniencia para testing desde consola de debug
 */
void DebugRunStressTest(const char* testName) {
    if (testName == nullptr) {
        std::cout << "Available tests:" << std::endl;
        std::cout << "- INFERNO_ESTACIONARIO" << std::endl;
        std::cout << "- COMBINED_CHAOS" << std::endl;
        return;
    }
    
    RunSingleTest(std::string(testName));
}

} // namespace devilution

// ============================================================================
// 🔧 FUNCIÓN MAIN PARA TESTING STANDALONE (OPCIONAL)
// ============================================================================

#ifdef STRESS_TEST_STANDALONE
int main() {
    using namespace devilution;
    
    std::cout << "SAFETY LAYER - Standalone Stress Test Runner" << std::endl;
    
    // Inicializar Safety Layer
    InitSafetyLayer();
    
    // Ejecutar runner interactivo
    RunInteractiveTestRunner();
    
    return 0;
}
#endif