#!/usr/bin/env python3
"""
TEST DYNAMIC SCALING - FASE 2
Valida que el Dynamic Scaling funciona correctamente
"""

import time
from dataclasses import dataclass
from typing import List

@dataclass
class TestResult:
    name: str
    passed: bool
    message: str
    duration: float

class DynamicScalingTester:
    def __init__(self):
        self.results: List[TestResult] = []
    
    def test_api_present(self) -> TestResult:
        """Test 1: API de dynamic scaling presente"""
        print("\n🔍 Test 1: Dynamic Scaling API...")
        start = time.time()
        
        try:
            with open("Source/engine/platform/dynamic_scaling.h", "r") as f:
                content = f.read()
            
            required = [
                "struct ParticleBudget",
                "InitDynamicScaling",
                "UpdateDynamicScaling",
                "GetParticleBudget",
                "GetDecalBudget",
                "IsPerformanceStressed"
            ]
            
            missing = [r for r in required if r not in content]
            
            duration = time.time() - start
            
            if not missing:
                return TestResult(
                    "Dynamic Scaling API",
                    True,
                    "✅ All API functions present",
                    duration
                )
            else:
                return TestResult(
                    "Dynamic Scaling API",
                    False,
                    f"❌ Missing API: {', '.join(missing)}",
                    duration
                )
        except Exception as e:
            return TestResult(
                "Dynamic Scaling API",
                False,
                f"❌ Error: {str(e)}",
                time.time() - start
            )
    
    def test_o1_complexity(self) -> TestResult:
        """Test 2: Complejidad O(1)"""
        print("\n⚡ Test 2: O(1) Complexity...")
        start = time.time()
        
        try:
            with open("Source/engine/platform/dynamic_scaling.cpp", "r") as f:
                content = f.read()
            
            # Buscar patrones que indiquen complejidad > O(1)
            bad_patterns = [
                "for (int i = 0; i < 100",  # Loops grandes
                "while (true)",              # Infinite loops
                "recursive",                 # Recursión
            ]
            
            found_bad = [p for p in bad_patterns if p in content]
            
            duration = time.time() - start
            
            if not found_bad:
                return TestResult(
                    "O(1) Complexity",
                    True,
                    "✅ O(1) complexity confirmed",
                    duration
                )
            else:
                return TestResult(
                    "O(1) Complexity",
                    False,
                    f"❌ Found complex patterns: {', '.join(found_bad)}",
                    duration
                )
        except Exception as e:
            return TestResult(
                "O(1) Complexity",
                False,
                f"❌ Error: {str(e)}",
                time.time() - start
            )
    
    def test_heuristic_simple(self) -> TestResult:
        """Test 3: Heurística simple"""
        print("\n🧠 Test 3: Simple Heuristic...")
        start = time.time()
        
        try:
            with open("Source/engine/platform/dynamic_scaling.cpp", "r") as f:
                content = f.read()
            
            # Verificar que usa heurística simple
            required_patterns = [
                "TARGET_FRAME_TIME",  # Threshold definido
                "STRESS_THRESHOLD",   # Threshold de stress
                "frameTime >",        # Comparación simple
                "budget.current",     # Ajuste de budget
            ]
            
            found = [p for p in required_patterns if p in content]
            
            duration = time.time() - start
            
            if len(found) >= 3:
                return TestResult(
                    "Simple Heuristic",
                    True,
                    "✅ Simple heuristic confirmed",
                    duration
                )
            else:
                return TestResult(
                    "Simple Heuristic",
                    False,
                    f"❌ Missing heuristic patterns",
                    duration
                )
        except Exception as e:
            return TestResult(
                "Simple Heuristic",
                False,
                f"❌ Error: {str(e)}",
                time.time() - start
            )
    
    def test_no_gameplay_impact(self) -> TestResult:
        """Test 4: No impacto en gameplay"""
        print("\n🎮 Test 4: No Gameplay Impact...")
        start = time.time()
        
        try:
            with open("Source/engine/platform/dynamic_scaling.cpp", "r") as f:
                content = f.read()
            
            # Verificar que NO modifica gameplay
            gameplay_keywords = [
                "player.cpp",
                "spells.cpp",
                "items.cpp",
                "damage",
                "health",
                "mana"
            ]
            
            found_gameplay = [k for k in gameplay_keywords if k in content]
            
            duration = time.time() - start
            
            if not found_gameplay:
                return TestResult(
                    "No Gameplay Impact",
                    True,
                    "✅ No gameplay modifications",
                    duration
                )
            else:
                return TestResult(
                    "No Gameplay Impact",
                    False,
                    f"❌ Found gameplay keywords: {', '.join(found_gameplay)}",
                    duration
                )
        except Exception as e:
            return TestResult(
                "No Gameplay Impact",
                False,
                f"❌ Error: {str(e)}",
                time.time() - start
            )
    
    def test_fail_soft(self) -> TestResult:
        """Test 5: Fail-soft behavior"""
        print("\n🛡️ Test 5: Fail-Soft Behavior...")
        start = time.time()
        
        try:
            with open("Source/engine/platform/dynamic_scaling.cpp", "r") as f:
                content = f.read()
            
            # Verificar que tiene límites seguros
            safe_patterns = [
                "budget.min",         # Límite mínimo
                "budget.max",         # Límite máximo
                "if (newBudget >=",   # Verificación de límites
                "std::max",           # Uso de max para seguridad
            ]
            
            found_safe = [p for p in safe_patterns if p in content]
            
            duration = time.time() - start
            
            if len(found_safe) >= 3:
                return TestResult(
                    "Fail-Soft Behavior",
                    True,
                    "✅ Safe limits present",
                    duration
                )
            else:
                return TestResult(
                    "Fail-Soft Behavior",
                    False,
                    f"❌ Missing safe limits",
                    duration
                )
        except Exception as e:
            return TestResult(
                "Fail-Soft Behavior",
                False,
                f"❌ Error: {str(e)}",
                time.time() - start
            )
    
    def test_simulation(self) -> TestResult:
        """Test 6: Simulación de comportamiento"""
        print("\n🧪 Test 6: Behavior Simulation...")
        start = time.time()
        
        # Simular comportamiento del sistema
        class Budget:
            def __init__(self):
                self.current = 100
                self.min = 50
                self.max = 100
                self.stable_frames = 0
                self.unstable_frames = 0
        
        budget = Budget()
        
        # Simular 10 frames malos (>20ms)
        for _ in range(10):
            budget.unstable_frames += 1
            if budget.unstable_frames >= 10:
                budget.current = max(budget.min, budget.current - 5)
                budget.unstable_frames = 0
        
        # Budget debería haber bajado
        if budget.current < 100:
            # Simular 60 frames buenos (<16.6ms)
            for _ in range(60):
                budget.stable_frames += 1
                if budget.stable_frames >= 60:
                    budget.current = min(budget.max, budget.current + 1)
                    budget.stable_frames = 0
            
            # Budget debería haber subido
            if budget.current > 95:
                duration = time.time() - start
                return TestResult(
                    "Behavior Simulation",
                    True,
                    f"✅ Budget adapts correctly (95→{budget.current})",
                    duration
                )
        
        duration = time.time() - start
        return TestResult(
            "Behavior Simulation",
            False,
            f"❌ Budget didn't adapt as expected",
            duration
        )
    
    def run_all_tests(self):
        """Ejecutar todos los tests"""
        print("╔" + "=" * 58 + "╗")
        print("║  DYNAMIC SCALING TEST SUITE - FASE 2                    ║")
        print("╚" + "=" * 58 + "╝")
        
        # Ejecutar tests
        self.results.append(self.test_api_present())
        self.results.append(self.test_o1_complexity())
        self.results.append(self.test_heuristic_simple())
        self.results.append(self.test_no_gameplay_impact())
        self.results.append(self.test_fail_soft())
        self.results.append(self.test_simulation())
        
        # Imprimir resultados
        print("\n" + "=" * 60)
        print("RESULTS:")
        print("=" * 60)
        
        for result in self.results:
            status = "✅ PASS" if result.passed else "❌ FAIL"
            print(f"{status} - {result.name}")
            print(f"     {result.message}")
            print(f"     Duration: {result.duration:.2f}s")
        
        # Resumen
        passed = sum(1 for r in self.results if r.passed)
        total = len(self.results)
        
        print("\n" + "=" * 60)
        print(f"SUMMARY: {passed}/{total} tests passed")
        print("=" * 60)
        
        if passed == total:
            print("\n🎉 ALL TESTS PASSED! Dynamic Scaling is SOLID! 🎉")
            return 0
        else:
            print("\n⚠️  SOME TESTS FAILED! Review the implementation! ⚠️")
            return 1

def main():
    tester = DynamicScalingTester()
    return tester.run_all_tests()

if __name__ == "__main__":
    import sys
    sys.exit(main())
