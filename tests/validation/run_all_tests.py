#!/usr/bin/env python3
"""
Master Test Runner for OpenFOAM MCP Server Physics Validation

Executes all validation tests and generates comprehensive reports.
"""

import sys
import subprocess
import time
from pathlib import Path

def run_test_suite(test_name, test_script):
    """Run a single test suite and capture results"""
    print(f"\n🔬 Running {test_name}...")
    print("=" * 60)
    
    try:
        start_time = time.time()
        result = subprocess.run(
            [sys.executable, test_script],
            capture_output=True,
            text=True,
            timeout=300  # 5 minute timeout per test suite
        )
        end_time = time.time()
        
        duration = end_time - start_time
        
        if result.returncode == 0:
            print(f"✅ {test_name} PASSED ({duration:.1f}s)")
            return True, duration, result.stdout
        else:
            print(f"❌ {test_name} FAILED ({duration:.1f}s)")
            print(f"Error output:\n{result.stderr}")
            return False, duration, result.stderr
            
    except subprocess.TimeoutExpired:
        print(f"⏰ {test_name} TIMEOUT (>300s)")
        return False, 300, "Test timed out"
    except Exception as e:
        print(f"💥 {test_name} ERROR: {e}")
        return False, 0, str(e)

def main():
    """Run all validation test suites"""
    print("🚀 OpenFOAM MCP Server - Complete Physics Validation Suite")
    print("=" * 70)
    print("Testing all CFD physics against analytical solutions...")
    
    # Test suite definitions
    test_suites = [
        ("Pipe Flow Physics", "pipe_flow_validation.py"),
        ("Heat Transfer Physics", "heat_transfer_validation.py"),
        ("External Flow Physics", "external_flow_validation.py")
    ]
    
    # Results tracking
    results = []
    total_start_time = time.time()
    
    # Run each test suite
    for test_name, test_script in test_suites:
        test_path = Path(__file__).parent / test_script
        if not test_path.exists():
            print(f"❌ Test script not found: {test_script}")
            results.append((test_name, False, 0, "Script not found"))
            continue
            
        success, duration, output = run_test_suite(test_name, str(test_path))
        results.append((test_name, success, duration, output))
    
    total_end_time = time.time()
    total_duration = total_end_time - total_start_time
    
    # Generate final report
    print("\n" + "=" * 70)
    print("🎯 FINAL VALIDATION REPORT")
    print("=" * 70)
    
    passed_tests = sum(1 for _, success, _, _ in results if success)
    total_tests = len(results)
    
    print(f"📊 Test Summary: {passed_tests}/{total_tests} test suites passed")
    print(f"⏱️  Total Runtime: {total_duration:.1f} seconds")
    
    # Detailed results
    print("\n📋 Detailed Results:")
    for test_name, success, duration, output in results:
        status = "✅ PASS" if success else "❌ FAIL"
        print(f"  {status} {test_name:<30} ({duration:.1f}s)")
        if not success:
            print(f"    Error: {output.splitlines()[-1] if output else 'Unknown error'}")
    
    # Physics validation status
    print("\n🔬 Physics Validation Status:")
    physics_areas = [
        ("Pipe Flow", "Laminar/Turbulent flow, Reynolds numbers, Friction factors"),
        ("Heat Transfer", "Forced/Natural convection, Nusselt numbers, Conjugate HT"),
        ("External Flow", "Drag coefficients, Boundary layers, Flow separation")
    ]
    
    for i, (area, description) in enumerate(physics_areas):
        if i < len(results):
            status = "✅ VALIDATED" if results[i][1] else "❌ FAILED"
            print(f"  {status} {area:<15} - {description}")
    
    # Recommendations
    print("\n💡 Recommendations:")
    if passed_tests == total_tests:
        print("  🎉 All physics validation tests passed!")
        print("  ✅ OpenFOAM MCP Server is ready for production use")
        print("  🚀 Consider implementing additional physics domains")
    else:
        failed_tests = total_tests - passed_tests
        print(f"  ⚠️  {failed_tests} test suite(s) failed - investigate before deployment")
        print("  🔧 Check OpenFOAM installation and version compatibility")
        print("  📝 Review test output for specific physics validation issues")
    
    # Quality gates
    print("\n🛡️  Quality Gates:")
    accuracy_threshold = 0.85  # 85% tests must pass
    if passed_tests / total_tests >= accuracy_threshold:
        print(f"  ✅ Accuracy Gate: {passed_tests}/{total_tests} ≥ {accuracy_threshold:.0%}")
    else:
        print(f"  ❌ Accuracy Gate: {passed_tests}/{total_tests} < {accuracy_threshold:.0%}")
    
    performance_threshold = 600  # 10 minutes max
    if total_duration <= performance_threshold:
        print(f"  ✅ Performance Gate: {total_duration:.0f}s ≤ {performance_threshold}s")
    else:
        print(f"  ❌ Performance Gate: {total_duration:.0f}s > {performance_threshold}s")
    
    # Exit with appropriate code
    if passed_tests == total_tests:
        print("\n🎯 ALL VALIDATION TESTS PASSED! 🎯")
        return 0
    else:
        print(f"\n💥 {total_tests - passed_tests} VALIDATION TESTS FAILED! 💥")
        return 1

if __name__ == "__main__":
    sys.exit(main())