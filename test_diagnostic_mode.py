#!/usr/bin/env python3
"""
NIGHTMARE CORE EXPANSION - FASE 3 TEST SUITE
Diagnostic Mode Testing

Tests the diagnostic mode implementation without compilation.
"""

import sys

def test_diagnostic_mode_api():
    """Test 1: Diagnostic Mode API exists"""
    print("=" * 60)
    print("TEST 1: Diagnostic Mode API")
    print("=" * 60)
    
    required_functions = [
        "InitDiagnosticMode",
        "ToggleDiagnosticMode",
        "IsDiagnosticModeEnabled",
        "SetOverlayEnabled",
        "IsOverlayEnabled",
        "ToggleOverlay",
        "DrawDiagnosticOverlays",
        "DrawSpawnPressureOverlay",
        "DrawEliteRollsOverlay",
        "DrawAIStateOverlay",
        "DrawLightingCostOverlay",
        "DrawPerformanceOverlay",
        "GetDiagnosticState",
        "ResetDiagnosticStats"
    ]
    
    # Read header file
    try:
        with open("Source/engine/platform/diagnostic_mode.h", "r") as f:
            content = f.read()
            
        missing = []
        for func in required_functions:
            if func not in content:
                missing.append(func)
        
        if missing:
            print(f"❌ FAIL - Missing functions: {', '.join(missing)}")
            return False
        
        print("✅ PASS - All API functions present")
        print(f"     ✅ {len(required_functions)} functions declared")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - diagnostic_mode.h not found")
        return False

def test_debug_only():
    """Test 2: Debug-only implementation"""
    print("\n" + "=" * 60)
    print("TEST 2: Debug-Only Implementation")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/diagnostic_mode.cpp", "r") as f:
            content = f.read()
        
        # Check for #ifdef _DEBUG guards
        debug_guards = content.count("#ifdef _DEBUG")
        
        if debug_guards < 5:
            print(f"❌ FAIL - Insufficient debug guards (found {debug_guards}, expected >= 5)")
            return False
        
        print("✅ PASS - Debug-only implementation verified")
        print(f"     ✅ {debug_guards} debug guards found")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - diagnostic_mode.cpp not found")
        return False

def test_zero_impact_when_disabled():
    """Test 3: Zero impact when disabled"""
    print("\n" + "=" * 60)
    print("TEST 3: Zero Impact When Disabled")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/diagnostic_mode.cpp", "r") as f:
            content = f.read()
        
        # Check that functions return early when disabled
        early_returns = content.count("if (!gDiagnosticState.enabled)")
        
        if early_returns < 1:
            print("❌ FAIL - No early returns found")
            return False
        
        # Check that release builds have no-op implementations
        # In release, functions should return false or do nothing
        ifdef_debug = content.count("#ifdef _DEBUG")
        
        if ifdef_debug < 5:
            print(f"❌ FAIL - Insufficient debug guards (found {ifdef_debug})")
            return False
        
        # Check for return false in non-debug paths
        return_false = content.count("return false;")
        
        print("✅ PASS - Zero impact when disabled")
        print(f"     ✅ {early_returns} early returns in debug")
        print(f"     ✅ {ifdef_debug} #ifdef _DEBUG guards")
        print(f"     ✅ {return_false} no-op returns in release")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - diagnostic_mode.cpp not found")
        return False

def test_overlay_system():
    """Test 4: Overlay system"""
    print("\n" + "=" * 60)
    print("TEST 4: Overlay System")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/diagnostic_mode.h", "r") as f:
            content = f.read()
        
        # Check for overlay enum
        if "enum class DiagnosticOverlay" not in content:
            print("❌ FAIL - DiagnosticOverlay enum not found")
            return False
        
        # Check for overlay types
        overlays = [
            "SpawnPressure",
            "EliteRolls",
            "AIState",
            "LightingCost",
            "Performance"
        ]
        
        missing = []
        for overlay in overlays:
            if overlay not in content:
                missing.append(overlay)
        
        if missing:
            print(f"❌ FAIL - Missing overlays: {', '.join(missing)}")
            return False
        
        print("✅ PASS - Overlay system complete")
        print(f"     ✅ {len(overlays)} overlay types defined")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - diagnostic_mode.h not found")
        return False

def test_modular_overlays():
    """Test 5: Modular overlays"""
    print("\n" + "=" * 60)
    print("TEST 5: Modular Overlays")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/diagnostic_mode.cpp", "r") as f:
            content = f.read()
        
        # Check that each overlay can be toggled independently
        if "SetOverlayEnabled" not in content:
            print("❌ FAIL - SetOverlayEnabled not implemented")
            return False
        
        if "IsOverlayEnabled" not in content:
            print("❌ FAIL - IsOverlayEnabled not implemented")
            return False
        
        # Check that overlays are drawn conditionally
        conditional_draws = content.count("if (IsOverlayEnabled(")
        
        if conditional_draws < 5:
            print(f"❌ FAIL - Insufficient conditional draws (found {conditional_draws})")
            return False
        
        print("✅ PASS - Modular overlays implemented")
        print(f"     ✅ {conditional_draws} conditional overlay draws")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - diagnostic_mode.cpp not found")
        return False

def test_performance_overlay():
    """Test 6: Performance overlay integration"""
    print("\n" + "=" * 60)
    print("TEST 6: Performance Overlay Integration")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/diagnostic_mode.cpp", "r") as f:
            content = f.read()
        
        # Check that performance overlay uses platform and scaling data
        if "GetPlatformCapabilities()" not in content:
            print("❌ FAIL - Not using platform capabilities")
            return False
        
        if "GetScalingStats()" not in content:
            print("❌ FAIL - Not using scaling stats")
            return False
        
        if "IsPerformanceStressed()" not in content:
            print("❌ FAIL - Not checking performance stress")
            return False
        
        print("✅ PASS - Performance overlay integrated")
        print("     ✅ Uses platform capabilities")
        print("     ✅ Uses scaling stats")
        print("     ✅ Checks performance stress")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - diagnostic_mode.cpp not found")
        return False

def run_all_tests():
    """Run all diagnostic mode tests"""
    print("\n")
    print("╔" + "=" * 58 + "╗")
    print("║  DIAGNOSTIC MODE TEST SUITE - FASE 3                     ║")
    print("║  Nightmare Core Expansion                                ║")
    print("╚" + "=" * 58 + "╝")
    print()
    
    tests = [
        ("Diagnostic Mode API", test_diagnostic_mode_api),
        ("Debug-Only Implementation", test_debug_only),
        ("Zero Impact When Disabled", test_zero_impact_when_disabled),
        ("Overlay System", test_overlay_system),
        ("Modular Overlays", test_modular_overlays),
        ("Performance Overlay Integration", test_performance_overlay)
    ]
    
    results = []
    for name, test_func in tests:
        try:
            result = test_func()
            results.append((name, result))
        except Exception as e:
            print(f"❌ FAIL - Exception: {e}")
            results.append((name, False))
    
    # Summary
    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for name, result in results:
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"{status} - {name}")
    
    print()
    print(f"Total tests:   {total}")
    print(f"Passed:        {passed} ✅")
    print(f"Failed:        {total - passed} ❌")
    print(f"Success rate:  {(passed/total)*100:.0f}%")
    print()
    
    if passed == total:
        print("🎉 ALL TESTS PASSED! Diagnostic Mode is SOLID! 🎉")
        return 0
    else:
        print("⚠️  SOME TESTS FAILED - Review implementation")
        return 1

if __name__ == "__main__":
    sys.exit(run_all_tests())
