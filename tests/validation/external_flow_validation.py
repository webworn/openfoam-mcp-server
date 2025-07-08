#!/usr/bin/env python3
"""
External Flow Physics Validation Tests

Validates external flow calculations against analytical solutions:
- Drag coefficient correlations for various geometries
- Boundary layer theory for flat plates
- Flow around cylinders and spheres
"""

import sys
import json
import subprocess
import math
from pathlib import Path

def validate_flat_plate_drag():
    """Test flat plate boundary layer against Blasius solution"""
    print("🧪 Testing flat plate boundary layer drag...")
    
    # Test case: air flow over flat plate
    test_input = {
        "geometry": {
            "type": "flat_plate",
            "length": 1.0,
            "width": 0.5,
            "thickness": 0.01
        },
        "flow_conditions": {
            "velocity": 10.0,
            "fluid": "air",
            "temperature": 298.15,
            "pressure": 101325,
            "turbulence_intensity": 0.01
        },
        "analysis_type": "drag_analysis"
    }
    
    # Run MCP tool
    result = run_external_flow_tool(test_input)
    
    # Analytical solution - Blasius flat plate
    # Air properties
    mu = 1.849e-5  # Pa·s for air at 25°C
    rho = 1.184    # kg/m³
    
    L = test_input["geometry"]["length"]
    V = test_input["flow_conditions"]["velocity"]
    Re = rho * V * L / mu
    
    # Skin friction coefficient (Blasius)
    Cf_analytical = 1.328 / math.sqrt(Re)
    
    # Drag coefficient for flat plate
    Cd_analytical = 2 * Cf_analytical  # both sides
    
    # Drag force
    A = test_input["geometry"]["length"] * test_input["geometry"]["width"]
    F_drag_analytical = 0.5 * rho * V**2 * A * Cd_analytical
    
    # Validate results
    Re_computed = result.get("reynoldsNumber", 0)
    Cd_computed = result.get("dragCoefficient", 0)
    F_drag_computed = result.get("dragForce", 0)
    
    Re_error = abs(Re_computed - Re) / Re * 100
    Cd_error = abs(Cd_computed - Cd_analytical) / Cd_analytical * 100
    F_error = abs(F_drag_computed - F_drag_analytical) / F_drag_analytical * 100
    
    print(f"  Reynolds Number: {Re_computed:.0f} vs {Re:.0f} (error: {Re_error:.1f}%)")
    print(f"  Drag Coefficient: {Cd_computed:.4f} vs {Cd_analytical:.4f} (error: {Cd_error:.1f}%)")
    print(f"  Drag Force: {F_drag_computed:.2f} vs {F_drag_analytical:.2f} N (error: {F_error:.1f}%)")
    
    # Accept up to 10% error for flat plate
    assert Re_error < 5.0, f"Reynolds number error {Re_error:.1f}% exceeds 5%"
    assert Cd_error < 10.0, f"Drag coefficient error {Cd_error:.1f}% exceeds 10%"
    assert F_error < 10.0, f"Drag force error {F_error:.1f}% exceeds 10%"
    
    print("  ✅ Flat plate drag validation passed")

def validate_cylinder_flow():
    """Test flow around cylinder against empirical correlations"""
    print("🧪 Testing flow around cylinder...")
    
    # Test case: air flow around cylinder
    test_input = {
        "geometry": {
            "type": "cylinder",
            "diameter": 0.1,
            "length": 1.0
        },
        "flow_conditions": {
            "velocity": 20.0,
            "fluid": "air",
            "temperature": 298.15,
            "pressure": 101325,
            "turbulence_intensity": 0.05
        },
        "analysis_type": "drag_analysis"
    }
    
    # Run MCP tool
    result = run_external_flow_tool(test_input)
    
    # Analytical solution - Cylinder drag correlation
    # Air properties
    mu = 1.849e-5  # Pa·s for air at 25°C
    rho = 1.184    # kg/m³
    
    D = test_input["geometry"]["diameter"]
    V = test_input["flow_conditions"]["velocity"]
    Re = rho * V * D / mu
    
    # Drag coefficient correlation for cylinder
    if Re < 1e5:
        Cd_analytical = 1.2  # subcritical regime
    else:
        Cd_analytical = 0.3  # supercritical regime
    
    # Drag force
    A = test_input["geometry"]["diameter"] * test_input["geometry"]["length"]
    F_drag_analytical = 0.5 * rho * V**2 * A * Cd_analytical
    
    # Validate results
    Re_computed = result.get("reynoldsNumber", 0)
    Cd_computed = result.get("dragCoefficient", 0)
    F_drag_computed = result.get("dragForce", 0)
    
    Re_error = abs(Re_computed - Re) / Re * 100
    Cd_error = abs(Cd_computed - Cd_analytical) / Cd_analytical * 100
    F_error = abs(F_drag_computed - F_drag_analytical) / F_drag_analytical * 100
    
    print(f"  Reynolds Number: {Re_computed:.0f} vs {Re:.0f} (error: {Re_error:.1f}%)")
    print(f"  Drag Coefficient: {Cd_computed:.2f} vs {Cd_analytical:.2f} (error: {Cd_error:.1f}%)")
    print(f"  Drag Force: {F_drag_computed:.1f} vs {F_drag_analytical:.1f} N (error: {F_error:.1f}%)")
    
    # Accept up to 15% error for cylinder (complex flow)
    assert Re_error < 5.0, f"Reynolds number error {Re_error:.1f}% exceeds 5%"
    assert Cd_error < 15.0, f"Drag coefficient error {Cd_error:.1f}% exceeds 15%"
    assert F_error < 15.0, f"Drag force error {F_error:.1f}% exceeds 15%"
    
    print("  ✅ Cylinder flow validation passed")

def validate_sphere_flow():
    """Test flow around sphere against empirical correlations"""
    print("🧪 Testing flow around sphere...")
    
    # Test case: air flow around sphere
    test_input = {
        "geometry": {
            "type": "sphere",
            "diameter": 0.05
        },
        "flow_conditions": {
            "velocity": 15.0,
            "fluid": "air",
            "temperature": 298.15,
            "pressure": 101325,
            "turbulence_intensity": 0.02
        },
        "analysis_type": "drag_analysis"
    }
    
    # Run MCP tool
    result = run_external_flow_tool(test_input)
    
    # Analytical solution - Sphere drag correlation
    # Air properties
    mu = 1.849e-5  # Pa·s for air at 25°C
    rho = 1.184    # kg/m³
    
    D = test_input["geometry"]["diameter"]
    V = test_input["flow_conditions"]["velocity"]
    Re = rho * V * D / mu
    
    # Drag coefficient correlation for sphere
    if Re < 1e5:
        Cd_analytical = 0.47  # subcritical regime
    else:
        Cd_analytical = 0.2   # supercritical regime
    
    # Drag force
    A = math.pi * (D/2)**2  # frontal area
    F_drag_analytical = 0.5 * rho * V**2 * A * Cd_analytical
    
    # Validate results
    Re_computed = result.get("reynoldsNumber", 0)
    Cd_computed = result.get("dragCoefficient", 0)
    F_drag_computed = result.get("dragForce", 0)
    
    Re_error = abs(Re_computed - Re) / Re * 100
    Cd_error = abs(Cd_computed - Cd_analytical) / Cd_analytical * 100
    F_error = abs(F_drag_computed - F_drag_analytical) / F_drag_analytical * 100
    
    print(f"  Reynolds Number: {Re_computed:.0f} vs {Re:.0f} (error: {Re_error:.1f}%)")
    print(f"  Drag Coefficient: {Cd_computed:.2f} vs {Cd_analytical:.2f} (error: {Cd_error:.1f}%)")
    print(f"  Drag Force: {F_drag_computed:.3f} vs {F_drag_analytical:.3f} N (error: {F_error:.1f}%)")
    
    # Accept up to 15% error for sphere (complex flow)
    assert Re_error < 5.0, f"Reynolds number error {Re_error:.1f}% exceeds 5%"
    assert Cd_error < 15.0, f"Drag coefficient error {Cd_error:.1f}% exceeds 15%"
    assert F_error < 15.0, f"Drag force error {F_error:.1f}% exceeds 15%"
    
    print("  ✅ Sphere flow validation passed")

def run_external_flow_tool(input_params):
    """Execute the MCP external flow tool with given parameters"""
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
                "name": "analyze_external_flow",
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
            "reynoldsNumber": 50000,
            "dragCoefficient": 0.5,
            "dragForce": 10.0,
            "success": False
        }

def main():
    """Run all external flow validation tests"""
    print("🔬 OpenFOAM External Flow Physics Validation")
    print("=" * 50)
    
    try:
        validate_flat_plate_drag()
        validate_cylinder_flow()
        validate_sphere_flow()
        
        print("\n✅ All external flow validation tests passed!")
        return 0
        
    except AssertionError as e:
        print(f"\n❌ Validation failed: {e}")
        return 1
    except Exception as e:
        print(f"\n💥 Test execution failed: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())