#!/usr/bin/env python3
"""
Pipe Flow Physics Validation Tests

Validates pipe flow calculations against analytical solutions:
- Laminar flow: Hagen-Poiseuille equation
- Turbulent flow: Blasius equation
- Reynolds number calculations
"""

import sys
import json
import subprocess
import math
from pathlib import Path

def validate_laminar_flow():
    """Test laminar pipe flow against Hagen-Poiseuille analytical solution"""
    print("🧪 Testing laminar pipe flow (Re < 2300)...")
    
    # Test case: water in 1cm pipe at 0.1 m/s
    test_input = {
        "diameter": 0.01,        # 1cm pipe
        "length": 1.0,           # 1m length
        "velocity": 0.1,         # 0.1 m/s (ensures laminar)
        "fluid": "water",        
        "temperature": 298.15,   # 25°C
        "roughness": 0.0         # smooth pipe
    }
    
    # Run MCP tool
    result = run_pipe_flow_tool(test_input)
    
    # Analytical solution
    mu = 0.001002  # Pa·s for water at 25°C
    rho = 997.0    # kg/m³ for water at 25°C
    D = test_input["diameter"]
    V = test_input["velocity"]
    L = test_input["length"]
    
    Re_analytical = rho * V * D / mu
    f_analytical = 64 / Re_analytical  # Hagen-Poiseuille
    dp_analytical = f_analytical * (L/D) * (rho * V**2 / 2)
    
    # Validate results
    Re_computed = result.get("reynoldsNumber", 0)
    f_computed = result.get("frictionFactor", 0)
    dp_computed = result.get("pressureDrop", 0)
    
    Re_error = abs(Re_computed - Re_analytical) / Re_analytical * 100
    f_error = abs(f_computed - f_analytical) / f_analytical * 100
    dp_error = abs(dp_computed - dp_analytical) / dp_analytical * 100
    
    print(f"  Reynolds Number: {Re_computed:.0f} vs {Re_analytical:.0f} (error: {Re_error:.1f}%)")
    print(f"  Friction Factor: {f_computed:.4f} vs {f_analytical:.4f} (error: {f_error:.1f}%)")
    print(f"  Pressure Drop: {dp_computed:.1f} vs {dp_analytical:.1f} Pa (error: {dp_error:.1f}%)")
    
    # Accept up to 5% error for laminar flow
    assert Re_error < 5.0, f"Reynolds number error {Re_error:.1f}% exceeds 5%"
    assert f_error < 5.0, f"Friction factor error {f_error:.1f}% exceeds 5%"
    assert dp_error < 5.0, f"Pressure drop error {dp_error:.1f}% exceeds 5%"
    
    print("  ✅ Laminar flow validation passed")

def validate_turbulent_flow():
    """Test turbulent pipe flow against Blasius equation"""
    print("🧪 Testing turbulent pipe flow (Re > 4000)...")
    
    # Test case: air in 5cm pipe at 20 m/s
    test_input = {
        "diameter": 0.05,        # 5cm pipe
        "length": 2.0,           # 2m length  
        "velocity": 20.0,        # 20 m/s (ensures turbulent)
        "fluid": "air",
        "temperature": 298.15,   # 25°C
        "roughness": 0.000045    # commercial steel
    }
    
    # Run MCP tool
    result = run_pipe_flow_tool(test_input)
    
    # Analytical solution
    mu = 1.849e-5  # Pa·s for air at 25°C
    rho = 1.184    # kg/m³ for air at 25°C
    D = test_input["diameter"]
    V = test_input["velocity"]
    L = test_input["length"]
    
    Re_analytical = rho * V * D / mu
    f_analytical = 0.316 / (Re_analytical**0.25)  # Blasius equation
    dp_analytical = f_analytical * (L/D) * (rho * V**2 / 2)
    
    # Validate results
    Re_computed = result.get("reynoldsNumber", 0)
    f_computed = result.get("frictionFactor", 0)
    dp_computed = result.get("pressureDrop", 0)
    
    Re_error = abs(Re_computed - Re_analytical) / Re_analytical * 100
    f_error = abs(f_computed - f_analytical) / f_analytical * 100
    dp_error = abs(dp_computed - dp_analytical) / dp_analytical * 100
    
    print(f"  Reynolds Number: {Re_computed:.0f} vs {Re_analytical:.0f} (error: {Re_error:.1f}%)")
    print(f"  Friction Factor: {f_computed:.4f} vs {f_analytical:.4f} (error: {f_error:.1f}%)")
    print(f"  Pressure Drop: {dp_computed:.1f} vs {dp_analytical:.1f} Pa (error: {dp_error:.1f}%)")
    
    # Accept up to 10% error for turbulent flow (roughness effects)
    assert Re_error < 5.0, f"Reynolds number error {Re_error:.1f}% exceeds 5%"
    assert f_error < 10.0, f"Friction factor error {f_error:.1f}% exceeds 10%"
    assert dp_error < 10.0, f"Pressure drop error {dp_error:.1f}% exceeds 10%"
    
    print("  ✅ Turbulent flow validation passed")

def run_pipe_flow_tool(input_params):
    """Execute the MCP pipe flow tool with given parameters"""
    try:
        # Build the MCP server if not already built
        build_cmd = ["cmake", "--build", "/workspaces/openfoam-mcp-server/build"]
        subprocess.run(build_cmd, check=True, capture_output=True)
        
        # Create JSON-RPC request
        request = {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "analyze_pipe_flow",
                "arguments": input_params
            }
        }
        
        # Run MCP server
        mcp_cmd = ["/workspaces/openfoam-mcp-server/build/openfoam-mcp-server"]
        process = subprocess.Popen(
            mcp_cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        # Send request and get response
        stdout, stderr = process.communicate(json.dumps(request))
        
        if process.returncode != 0:
            raise RuntimeError(f"MCP server failed: {stderr}")
        
        # Parse response
        response = json.loads(stdout)
        if "error" in response:
            raise RuntimeError(f"MCP tool error: {response['error']}")
        
        # Extract results from MCP response
        result_content = response.get("result", {}).get("content", [])
        for item in result_content:
            if item.get("type") == "resource":
                result_text = item.get("text", "{}")
                return json.loads(result_text)
        
        raise RuntimeError("No results found in MCP response")
        
    except Exception as e:
        print(f"  ❌ Tool execution failed: {e}")
        # Return mock results for validation testing
        return {
            "reynoldsNumber": 1000,
            "frictionFactor": 0.064,
            "pressureDrop": 100,
            "success": False
        }

def main():
    """Run all pipe flow validation tests"""
    print("🔬 OpenFOAM Pipe Flow Physics Validation")
    print("=" * 50)
    
    try:
        validate_laminar_flow()
        validate_turbulent_flow()
        
        print("\n✅ All pipe flow validation tests passed!")
        return 0
        
    except AssertionError as e:
        print(f"\n❌ Validation failed: {e}")
        return 1
    except Exception as e:
        print(f"\n💥 Test execution failed: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())