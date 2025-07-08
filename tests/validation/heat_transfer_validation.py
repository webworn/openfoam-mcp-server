#!/usr/bin/env python3
"""
Heat Transfer Physics Validation Tests

Validates heat transfer calculations against analytical solutions:
- Forced convection: Dittus-Boelter equation
- Natural convection: Rayleigh-Nusselt correlations
- Conjugate heat transfer: analytical solutions
"""

import sys
import json
import subprocess
import math
from pathlib import Path

def validate_forced_convection():
    """Test forced convection against Dittus-Boelter equation"""
    print("🧪 Testing forced convection heat transfer...")
    
    # Test case: air flow over heated plate
    test_input = {
        "analysis_type": "forced_convection",
        "geometry": {
            "type": "plate",
            "length": 1.0,
            "width": 0.5,
            "thickness": 0.01
        },
        "fluid": {
            "type": "air",
            "temperature": 298.15,
            "velocity": 5.0,
            "pressure": 101325
        },
        "thermal": {
            "wall_temperature": 373.15,
            "heat_flux": 1000
        }
    }
    
    # Run MCP tool
    result = run_heat_transfer_tool(test_input)
    
    # Analytical solution - Dittus-Boelter equation
    # Air properties at film temperature
    T_film = (test_input["fluid"]["temperature"] + test_input["thermal"]["wall_temperature"]) / 2
    mu = 2.075e-5  # Pa·s for air at film temperature
    rho = 1.095    # kg/m³ 
    cp = 1007      # J/kg·K
    k = 0.0295     # W/m·K
    Pr = mu * cp / k
    
    # Reynolds number
    L = test_input["geometry"]["length"]
    V = test_input["fluid"]["velocity"]
    Re = rho * V * L / mu
    
    # Nusselt number (Dittus-Boelter)
    Nu_analytical = 0.0296 * (Re**0.8) * (Pr**0.33)
    h_analytical = Nu_analytical * k / L
    
    # Validate results
    Nu_computed = result.get("nusseltNumber", 0)
    h_computed = result.get("heatTransferCoeff", 0)
    
    Nu_error = abs(Nu_computed - Nu_analytical) / Nu_analytical * 100
    h_error = abs(h_computed - h_analytical) / h_analytical * 100
    
    print(f"  Reynolds Number: {Re:.0f}")
    print(f"  Prandtl Number: {Pr:.3f}")
    print(f"  Nusselt Number: {Nu_computed:.1f} vs {Nu_analytical:.1f} (error: {Nu_error:.1f}%)")
    print(f"  Heat Transfer Coeff: {h_computed:.1f} vs {h_analytical:.1f} W/m²·K (error: {h_error:.1f}%)")
    
    # Accept up to 15% error for forced convection
    assert Nu_error < 15.0, f"Nusselt number error {Nu_error:.1f}% exceeds 15%"
    assert h_error < 15.0, f"Heat transfer coefficient error {h_error:.1f}% exceeds 15%"
    
    print("  ✅ Forced convection validation passed")

def validate_natural_convection():
    """Test natural convection against Rayleigh-Nusselt correlations"""
    print("🧪 Testing natural convection heat transfer...")
    
    # Test case: vertical heated plate in air
    test_input = {
        "analysis_type": "natural_convection",
        "geometry": {
            "type": "vertical_plate",
            "height": 0.5,
            "width": 0.3,
            "thickness": 0.005
        },
        "fluid": {
            "type": "air",
            "temperature": 298.15,
            "pressure": 101325
        },
        "thermal": {
            "wall_temperature": 323.15,
            "ambient_temperature": 298.15
        }
    }
    
    # Run MCP tool
    result = run_heat_transfer_tool(test_input)
    
    # Analytical solution - Natural convection correlation
    # Air properties at film temperature
    T_film = (test_input["fluid"]["temperature"] + test_input["thermal"]["wall_temperature"]) / 2
    mu = 1.963e-5  # Pa·s for air at film temperature
    rho = 1.140    # kg/m³
    cp = 1007      # J/kg·K
    k = 0.0263     # W/m·K
    beta = 1 / T_film  # thermal expansion coefficient
    g = 9.81       # m/s²
    
    # Temperature difference and characteristic length
    dT = test_input["thermal"]["wall_temperature"] - test_input["thermal"]["ambient_temperature"]
    L = test_input["geometry"]["height"]
    
    # Rayleigh number
    Pr = mu * cp / k
    Ra = g * beta * dT * (L**3) * rho**2 * cp / (mu * k)
    
    # Nusselt number (vertical plate correlation)
    Nu_analytical = (0.825 + 0.387 * Ra**(1/6) / (1 + (0.492/Pr)**(9/16))**(8/27))**2
    h_analytical = Nu_analytical * k / L
    
    # Validate results
    Nu_computed = result.get("nusseltNumber", 0)
    h_computed = result.get("heatTransferCoeff", 0)
    Ra_computed = result.get("rayleighNumber", 0)
    
    Nu_error = abs(Nu_computed - Nu_analytical) / Nu_analytical * 100
    h_error = abs(h_computed - h_analytical) / h_analytical * 100
    Ra_error = abs(Ra_computed - Ra) / Ra * 100
    
    print(f"  Rayleigh Number: {Ra_computed:.0f} vs {Ra:.0f} (error: {Ra_error:.1f}%)")
    print(f"  Prandtl Number: {Pr:.3f}")
    print(f"  Nusselt Number: {Nu_computed:.1f} vs {Nu_analytical:.1f} (error: {Nu_error:.1f}%)")
    print(f"  Heat Transfer Coeff: {h_computed:.1f} vs {h_analytical:.1f} W/m²·K (error: {h_error:.1f}%)")
    
    # Accept up to 20% error for natural convection
    assert Ra_error < 10.0, f"Rayleigh number error {Ra_error:.1f}% exceeds 10%"
    assert Nu_error < 20.0, f"Nusselt number error {Nu_error:.1f}% exceeds 20%"
    assert h_error < 20.0, f"Heat transfer coefficient error {h_error:.1f}% exceeds 20%"
    
    print("  ✅ Natural convection validation passed")

def validate_conjugate_heat_transfer():
    """Test conjugate heat transfer against analytical solution"""
    print("🧪 Testing conjugate heat transfer...")
    
    # Test case: heat conduction through wall with convection
    test_input = {
        "analysis_type": "conjugate_heat_transfer",
        "geometry": {
            "type": "wall",
            "thickness": 0.1,
            "height": 1.0,
            "width": 1.0
        },
        "solid": {
            "material": "steel",
            "thermal_conductivity": 50.0,
            "density": 7850,
            "specific_heat": 460
        },
        "fluid": {
            "type": "air",
            "temperature": 298.15,
            "velocity": 2.0,
            "pressure": 101325
        },
        "thermal": {
            "hot_side_temp": 373.15,
            "cold_side_temp": 298.15,
            "convection_coeff": 25.0
        }
    }
    
    # Run MCP tool
    result = run_heat_transfer_tool(test_input)
    
    # Analytical solution - 1D steady-state heat conduction
    k_solid = test_input["solid"]["thermal_conductivity"]
    L = test_input["geometry"]["thickness"]
    h = test_input["thermal"]["convection_coeff"]
    T_hot = test_input["thermal"]["hot_side_temp"]
    T_cold = test_input["thermal"]["cold_side_temp"]
    
    # Overall heat transfer coefficient
    U_analytical = 1 / (1/h + L/k_solid + 1/h)
    q_analytical = U_analytical * (T_hot - T_cold)
    
    # Validate results
    U_computed = result.get("overallHeatTransferCoeff", 0)
    q_computed = result.get("heatFlux", 0)
    
    U_error = abs(U_computed - U_analytical) / U_analytical * 100
    q_error = abs(q_computed - q_analytical) / q_analytical * 100
    
    print(f"  Overall HTC: {U_computed:.2f} vs {U_analytical:.2f} W/m²·K (error: {U_error:.1f}%)")
    print(f"  Heat Flux: {q_computed:.1f} vs {q_analytical:.1f} W/m² (error: {q_error:.1f}%)")
    
    # Accept up to 10% error for conjugate heat transfer
    assert U_error < 10.0, f"Overall HTC error {U_error:.1f}% exceeds 10%"
    assert q_error < 10.0, f"Heat flux error {q_error:.1f}% exceeds 10%"
    
    print("  ✅ Conjugate heat transfer validation passed")

def run_heat_transfer_tool(input_params):
    """Execute the MCP heat transfer tool with given parameters"""
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
                "name": "analyze_heat_transfer",
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
            "nusseltNumber": 50,
            "heatTransferCoeff": 25,
            "rayleighNumber": 1e6,
            "overallHeatTransferCoeff": 15,
            "heatFlux": 500,
            "success": False
        }

def main():
    """Run all heat transfer validation tests"""
    print("🔬 OpenFOAM Heat Transfer Physics Validation")
    print("=" * 50)
    
    try:
        validate_forced_convection()
        validate_natural_convection()
        validate_conjugate_heat_transfer()
        
        print("\n✅ All heat transfer validation tests passed!")
        return 0
        
    except AssertionError as e:
        print(f"\n❌ Validation failed: {e}")
        return 1
    except Exception as e:
        print(f"\n💥 Test execution failed: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())