#!/usr/bin/env python3
"""
Run All CFD Tool Validations

This script runs validation tests for all OpenFOAM MCP server tools
against analytical solutions and experimental data.

Author: OpenFOAM MCP Development Team
License: MIT
"""

import os
import sys
import subprocess
import time
from pathlib import Path

def run_validation_script(script_name: str) -> tuple[int, str]:
    """
    Run a validation script and return results
    
    Args:
        script_name: Name of the validation script to run
        
    Returns:
        Tuple of (return_code, output)
    """
    try:
        result = subprocess.run(
            [sys.executable, script_name],
            capture_output=True,
            text=True,
            timeout=300  # 5 minute timeout
        )
        return result.returncode, result.stdout + result.stderr
    except subprocess.TimeoutExpired:
        return 1, f"❌ Timeout: {script_name} took longer than 5 minutes"
    except Exception as e:
        return 1, f"❌ Error running {script_name}: {e}"

def main():
    """Main validation runner"""
    
    print("🚀 OpenFOAM MCP Server - Comprehensive Validation Suite")
    print("=" * 70)
    
    # Get validation directory
    validation_dir = Path(__file__).parent.absolute()
    os.chdir(validation_dir)
    
    print(f"📁 Validation directory: {validation_dir}")
    print(f"🕐 Started at: {time.strftime('%Y-%m-%d %H:%M:%S')}")
    print()
    
    # Define validation scripts to run
    validation_scripts = [
        ("analytical_solutions.py", "Analytical Solutions Library Test"),
        ("pipe_flow_validation.py", "Pipe Flow Validation")
    ]
    
    # Track overall results
    total_scripts = len(validation_scripts)
    passed_scripts = 0
    failed_scripts = 0
    
    # Run each validation script
    for script_name, description in validation_scripts:
        print(f"🔍 Running: {description}")
        print(f"   Script: {script_name}")
        
        if not os.path.exists(script_name):
            print(f"   ❌ SKIP: Script not found")
            failed_scripts += 1
            print()
            continue
            
        start_time = time.time()
        return_code, output = run_validation_script(script_name)
        end_time = time.time()
        
        if return_code == 0:
            print(f"   ✅ PASS: Completed in {end_time - start_time:.1f}s")
            passed_scripts += 1
        else:
            print(f"   ❌ FAIL: Return code {return_code}")
            failed_scripts += 1
            
        # Print summary of output (last few lines)
        if output:
            output_lines = output.strip().split('\n')
            if len(output_lines) > 5:
                print("   Last few lines of output:")
                for line in output_lines[-5:]:
                    print(f"     {line}")
            else:
                print("   Output:")
                for line in output_lines:
                    print(f"     {line}")
        
        print()
    
    # Overall summary
    print("=" * 70)
    print("📊 VALIDATION SUMMARY")
    print("=" * 70)
    print(f"Total scripts: {total_scripts}")
    print(f"Passed: {passed_scripts} ✅")
    print(f"Failed: {failed_scripts} ❌")
    print(f"Success rate: {passed_scripts/total_scripts*100:.1f}%")
    print(f"🕐 Finished at: {time.strftime('%Y-%m-%d %H:%M:%S')}")
    
    # Return appropriate exit code
    return 0 if failed_scripts == 0 else 1

if __name__ == "__main__":
    sys.exit(main())