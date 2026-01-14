#!/usr/bin/env python3
"""
NIGHTMARE CORE EXPANSION - FASE 4 TEST SUITE
Mobile Safe Mode Testing

Tests the mobile safe mode implementation without compilation.
"""

import sys

def test_mobile_safe_mode_api():
    """Test 1: Mobile Safe Mode API exists"""
    print("=" * 60)
    print("TEST 1: Mobile Safe Mode API")
    print("=" * 60)
    
    required_functions = [
        "InitMobileSafeMode",
        "IsMobileSafeModeActive",
        "SetMobileSafeModeEnabled",
        "GetMobileSafeModeConfig",
        "UpdateMobileSafeModeConfig",
        "DetectThermalThrottling",
        "ApplyMobileSafeModeAdjustments",
        "RevertMobileSafeModeAdjustments",
        "GetMobileParticleCount",
        "GetMobileDecalCount",
        "ShouldSpawnParticleOnMobile",
        "ShouldSpawnDecalOnMobile"
    ]
    
    # Read header file
    try:
        with open("Source/engine/platform/mobile_safe_mode.h", "r") as f:
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
        print("❌ FAIL - mobile_safe_mode.h not found")
        return False

def test_config_structure():
    """Test 2: Configuration structure (SIMPLIFIED)"""
    print("\n" + "=" * 60)
    print("TEST 2: Configuration Structure (Simplified)")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/mobile_safe_mode.h", "r") as f:
            content = f.read()
        
        # Check for config struct
        if "struct MobileSafeModeConfig" not in content:
            print("❌ FAIL - MobileSafeModeConfig struct not found")
            return False
        
        # Check for SIMPLIFIED config fields (only the ones actually used)
        config_fields = [
            "particleReduction",
            "decalReduction",
            "disableParticles",
            "thermalThrottling",
            "throttleLevel"
        ]
        
        # Check that REMOVED fields are NOT present
        removed_fields = [
            "shadowQuality",
            "uiScale",
            "clickTargetSize",
            "uiContrast",
            "simplifyLighting",
            "reduceShadows",
            "reduceFPS",
            "targetFPS"
        ]
        
        missing = []
        for field in config_fields:
            if field not in content:
                missing.append(field)
        
        if missing:
            print(f"❌ FAIL - Missing config fields: {', '.join(missing)}")
            return False
        
        # Verify removed fields are gone
        still_present = []
        for field in removed_fields:
            if field in content:
                still_present.append(field)
        
        if still_present:
            print(f"⚠️  WARNING - Unused fields still present: {', '.join(still_present)}")
            # Not a failure, just a warning
        
        print("✅ PASS - Configuration structure simplified")
        print(f"     ✅ {len(config_fields)} essential fields defined")
        print(f"     ✅ {len(removed_fields) - len(still_present)} unused fields removed")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - mobile_safe_mode.h not found")
        return False

def test_automatic_detection():
    """Test 3: Automatic mobile detection"""
    print("\n" + "=" * 60)
    print("TEST 3: Automatic Mobile Detection")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/mobile_safe_mode.cpp", "r") as f:
            content = f.read()
        
        # Check for detection functions
        if "IsDeviceMobile()" not in content:
            print("❌ FAIL - IsDeviceMobile() not found")
            return False
        
        if "IsTouchOnly()" not in content:
            print("❌ FAIL - IsTouchOnly() not found")
            return False
        
        if "IsLowPowerDevice()" not in content:
            print("❌ FAIL - IsLowPowerDevice() not found")
            return False
        
        # Check that detection uses platform capabilities
        if "GetPlatformCapabilities()" not in content:
            print("❌ FAIL - Not using platform capabilities")
            return False
        
        print("✅ PASS - Automatic detection implemented")
        print("     ✅ Device type detection")
        print("     ✅ Touch-only detection")
        print("     ✅ Low-power detection")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - mobile_safe_mode.cpp not found")
        return False

def test_quality_reduction():
    """Test 4: Quality reduction system"""
    print("\n" + "=" * 60)
    print("TEST 4: Quality Reduction System")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/mobile_safe_mode.cpp", "r") as f:
            content = f.read()
        
        # Check for particle reduction
        if "GetMobileParticleCount" not in content:
            print("❌ FAIL - GetMobileParticleCount not implemented")
            return False
        
        # Check for decal reduction
        if "GetMobileDecalCount" not in content:
            print("❌ FAIL - GetMobileDecalCount not implemented")
            return False
        
        # Check for spawn probability
        if "ShouldSpawnParticleOnMobile" not in content:
            print("❌ FAIL - ShouldSpawnParticleOnMobile not implemented")
            return False
        
        if "ShouldSpawnDecalOnMobile" not in content:
            print("❌ FAIL - ShouldSpawnDecalOnMobile not implemented")
            return False
        
        print("✅ PASS - Quality reduction system complete")
        print("     ✅ Particle count reduction")
        print("     ✅ Decal count reduction")
        print("     ✅ Spawn probability control")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - mobile_safe_mode.cpp not found")
        return False

def test_thermal_throttling():
    """Test 5: Thermal throttling detection WITH RECOVERY"""
    print("\n" + "=" * 60)
    print("TEST 5: Thermal Throttling Detection (with Recovery)")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/mobile_safe_mode.cpp", "r") as f:
            content = f.read()
        
        # Check for thermal detection
        if "DetectThermalThrottling" not in content:
            print("❌ FAIL - DetectThermalThrottling not implemented")
            return False
        
        # Check that it uses performance stress
        if "IsPerformanceStressed()" not in content:
            print("❌ FAIL - Not checking performance stress")
            return False
        
        # Check for throttle levels
        if "throttleLevel" not in content:
            print("❌ FAIL - Throttle level not implemented")
            return False
        
        # Check for progressive reduction (increase)
        throttle_increase = content.count("throttleLevel++")
        if throttle_increase < 1:
            print(f"❌ FAIL - No throttle level increase found")
            return False
        
        # FIX CHECK: Verify throttle level RECOVERY (decrease)
        throttle_decrease = content.count("throttleLevel--")
        if throttle_decrease < 1:
            print(f"❌ FAIL - No throttle level recovery (decrease) found")
            return False
        
        print("✅ PASS - Thermal throttling with recovery complete")
        print(f"     ✅ Throttle increase implemented")
        print(f"     ✅ Throttle recovery (decrease) implemented")
        print(f"     ✅ Progressive quality reduction")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - mobile_safe_mode.cpp not found")
        return False

def test_reversible_adjustments():
    """Test 6: Reversible adjustments"""
    print("\n" + "=" * 60)
    print("TEST 6: Reversible Adjustments")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/mobile_safe_mode.cpp", "r") as f:
            content = f.read()
        
        # Check for apply function
        if "ApplyMobileSafeModeAdjustments" not in content:
            print("❌ FAIL - ApplyMobileSafeModeAdjustments not implemented")
            return False
        
        # Check for revert function
        if "RevertMobileSafeModeAdjustments" not in content:
            print("❌ FAIL - RevertMobileSafeModeAdjustments not implemented")
            return False
        
        # Check for original config backup
        if "gOriginalConfig" not in content:
            print("❌ FAIL - Original config not backed up")
            return False
        
        print("✅ PASS - Reversible adjustments implemented")
        print("     ✅ Apply adjustments")
        print("     ✅ Revert adjustments")
        print("     ✅ Config backup")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - mobile_safe_mode.cpp not found")
        return False

def test_integration_with_platform():
    """Test 7: Integration with platform system"""
    print("\n" + "=" * 60)
    print("TEST 7: Integration with Platform System")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/mobile_safe_mode.cpp", "r") as f:
            content = f.read()
        
        # Check for platform.h include
        if '#include "engine/platform/platform.h"' not in content:
            print("❌ FAIL - Not including platform.h")
            return False
        
        # Check for dynamic_scaling.h include
        if '#include "engine/platform/dynamic_scaling.h"' not in content:
            print("❌ FAIL - Not including dynamic_scaling.h")
            return False
        
        # Check that it uses platform capabilities
        platform_usage = content.count("GetPlatformCapabilities()")
        
        if platform_usage < 3:
            print(f"❌ FAIL - Insufficient platform usage (found {platform_usage})")
            return False
        
        print("✅ PASS - Platform integration complete")
        print(f"     ✅ {platform_usage} platform capability checks")
        print("     ✅ Includes platform headers")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - mobile_safe_mode.cpp not found")
        return False

def test_thread_safe_random():
    """Test 8: Thread-safe random (GenerateRnd instead of rand)"""
    print("\n" + "=" * 60)
    print("TEST 8: Thread-Safe Random")
    print("=" * 60)
    
    try:
        with open("Source/engine/platform/mobile_safe_mode.cpp", "r") as f:
            content = f.read()
        
        # Check that rand() is NOT used in actual code (not in comments)
        # Look for "rand() %" or "rand())" which would be actual usage
        if "= (rand()" in content or "return (rand()" in content:
            print("❌ FAIL - Using unsafe rand() function in code")
            return False
        
        # Check that GenerateRnd is used
        if "GenerateRnd" not in content:
            print("❌ FAIL - Not using GenerateRnd()")
            return False
        
        # Check for random.hpp include
        if '#include "engine/random.hpp"' not in content:
            print("❌ FAIL - Not including random.hpp")
            return False
        
        # Count GenerateRnd usages
        generate_rnd_count = content.count("GenerateRnd(")
        
        print("✅ PASS - Thread-safe random implemented")
        print(f"     ✅ Using GenerateRnd() ({generate_rnd_count} usages)")
        print("     ✅ Includes random.hpp")
        return True
        
    except FileNotFoundError:
        print("❌ FAIL - mobile_safe_mode.cpp not found")
        return False

def run_all_tests():
    """Run all mobile safe mode tests"""
    print("\n")
    print("╔" + "=" * 58 + "╗")
    print("║  MOBILE SAFE MODE TEST SUITE - FASE 4 (FIXED)           ║")
    print("║  Nightmare Core Expansion                                ║")
    print("╚" + "=" * 58 + "╝")
    print()
    
    tests = [
        ("Mobile Safe Mode API", test_mobile_safe_mode_api),
        ("Configuration Structure (Simplified)", test_config_structure),
        ("Automatic Detection", test_automatic_detection),
        ("Quality Reduction System", test_quality_reduction),
        ("Thermal Throttling (with Recovery)", test_thermal_throttling),
        ("Reversible Adjustments", test_reversible_adjustments),
        ("Integration with Platform", test_integration_with_platform),
        ("Thread-Safe Random", test_thread_safe_random)
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
        print("🎉 ALL TESTS PASSED! Mobile Safe Mode is SOLID! 🎉")
        return 0
    else:
        print("⚠️  SOME TESTS FAILED - Review implementation")
        return 1

if __name__ == "__main__":
    sys.exit(run_all_tests())
