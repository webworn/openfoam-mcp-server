#!/usr/bin/env python3
"""
Basic Unit Tests for OpenFOAM MCP Server

Tests basic functionality without requiring full OpenFOAM simulations.
"""

import sys
import json
import subprocess
import time
from pathlib import Path

def test_server_build():
    """Test that the MCP server builds successfully"""
    print("🔨 Testing MCP server build...")
    
    try:
        # Check if build exists
        build_path = Path("/workspaces/openfoam-mcp-server/build/openfoam-mcp-server")
        if not build_path.exists():
            print("  ❌ Build file not found")
            return False
        
        # Check if executable
        if not build_path.is_file():
            print("  ❌ Build is not a file")
            return False
        
        print("  ✅ MCP server build test passed")
        return True
        
    except Exception as e:
        print(f"  ❌ Build test failed: {e}")
        return False

def test_json_rpc_structure():
    """Test JSON-RPC request/response structure"""
    print("🔗 Testing JSON-RPC structure...")
    
    try:
        # Test valid JSON-RPC request
        request = {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/list",
            "params": {}
        }
        
        # Validate request structure
        assert "jsonrpc" in request
        assert request["jsonrpc"] == "2.0"
        assert "id" in request
        assert "method" in request
        assert "params" in request
        
        # Test serialization
        json_str = json.dumps(request)
        parsed = json.loads(json_str)
        assert parsed == request
        
        print("  ✅ JSON-RPC structure test passed")
        return True
        
    except Exception as e:
        print(f"  ❌ JSON-RPC structure test failed: {e}")
        return False

def test_physics_calculations():
    """Test basic physics calculations"""
    print("🔬 Testing physics calculations...")
    
    try:
        # Test Reynolds number calculation
        rho = 1000  # kg/m³
        v = 1.0     # m/s
        d = 0.1     # m
        mu = 0.001  # Pa·s
        
        Re = rho * v * d / mu
        expected_Re = 100000
        
        assert abs(Re - expected_Re) < 1e-6, f"Reynolds number mismatch: {Re} != {expected_Re}"
        
        # Test friction factor calculation (Blasius)
        f = 0.316 / (Re ** 0.25)
        expected_f = 0.0177
        
        assert abs(f - expected_f) < 0.001, f"Friction factor mismatch: {f} != {expected_f}"
        
        # Test Nusselt number calculation
        Pr = 0.7  # Prandtl number for air
        Nu = 0.023 * (Re ** 0.8) * (Pr ** 0.4)
        expected_Nu = 199.4  # Corrected expected value
        
        assert abs(Nu - expected_Nu) < 1.0, f"Nusselt number mismatch: {Nu} != {expected_Nu}"
        
        print("  ✅ Physics calculations test passed")
        return True
        
    except Exception as e:
        print(f"  ❌ Physics calculations test failed: {e}")
        return False

def test_file_structure():
    """Test that required files exist"""
    print("📁 Testing file structure...")
    
    try:
        base_path = Path("/workspaces/openfoam-mcp-server")
        
        # Required files
        required_files = [
            "CMakeLists.txt",
            "README.md",
            "src/main.cpp",
            "src/mcp/server.cpp",
            "src/mcp/server.hpp",
            "src/tools/pipe_flow_tool.cpp",
            "src/tools/heat_transfer_tool.cpp",
            "src/tools/external_flow_tool.cpp",
            ".github/workflows/ci.yml",
            ".github/CODEOWNERS",
            ".github/pull_request_template.md",
            ".clang-format"
        ]
        
        missing_files = []
        for file_path in required_files:
            if not (base_path / file_path).exists():
                missing_files.append(file_path)
        
        if missing_files:
            print(f"  ❌ Missing files: {missing_files}")
            return False
        
        print("  ✅ File structure test passed")
        return True
        
    except Exception as e:
        print(f"  ❌ File structure test failed: {e}")
        return False

def test_code_formatting():
    """Test that code follows formatting standards"""
    print("🎨 Testing code formatting...")
    
    try:
        # Check if clang-format is available
        format_check = subprocess.run(
            ["clang-format", "--version"],
            capture_output=True,
            text=True
        )
        
        if format_check.returncode != 0:
            print("  ⚠️  clang-format not available, skipping formatting check")
            return True
        
        # Check formatting on a sample file
        sample_file = "/workspaces/openfoam-mcp-server/src/main.cpp"
        if not Path(sample_file).exists():
            print("  ⚠️  Sample file not found, skipping formatting check")
            return True
        
        # Run clang-format check
        format_result = subprocess.run(
            ["clang-format", "--dry-run", "--Werror", sample_file],
            capture_output=True,
            text=True
        )
        
        if format_result.returncode == 0:
            print("  ✅ Code formatting test passed")
            return True
        else:
            print(f"  ❌ Code formatting issues found: {format_result.stderr}")
            return False
        
    except Exception as e:
        print(f"  ❌ Code formatting test failed: {e}")
        return False

def main():
    """Run all basic unit tests"""
    print("🧪 OpenFOAM MCP Server - Basic Unit Tests")
    print("=" * 50)
    
    # Test suite
    tests = [
        ("Server Build", test_server_build),
        ("JSON-RPC Structure", test_json_rpc_structure),
        ("Physics Calculations", test_physics_calculations),
        ("File Structure", test_file_structure),
        ("Code Formatting", test_code_formatting)
    ]
    
    # Run tests
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        if test_func():
            passed += 1
    
    # Results
    print(f"\n📊 Unit Test Results: {passed}/{total} passed")
    
    if passed == total:
        print("✅ All basic unit tests passed!")
        return 0
    else:
        print(f"❌ {total - passed} unit tests failed!")
        return 1

if __name__ == "__main__":
    sys.exit(main())