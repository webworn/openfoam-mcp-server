#!/usr/bin/env python3
"""
SDToolbox Bridge for C++ Integration

This script provides a JSON-based interface to the Shock and Detonation Toolbox
for integration with the C++ Chapman-Jouguet solver. It wraps core SDT functions
to maintain exact compatibility with proven algorithms.

Usage:
    python sdt_bridge.py < input.json > output.json

JSON Protocol:
    Input: {"function": "CJspeed", "P1": 101325, "T1": 300, "q": "H2:2 O2:1", "mech": "mechanism.cti"}
    Output: {"result": 1970.5, "success": true, "errors": [], "metadata": {...}}

Author: OpenFOAM MCP Server
Based on: SDToolbox by Caltech Explosion Dynamics Laboratory
"""

import sys
import json
import os
import traceback
import time
from pathlib import Path

# Add SDToolbox to path
script_dir = Path(__file__).parent
project_root = script_dir.parent.parent.parent
sdt_path = project_root / "RDE" / "data" / "SDToolbox" / "Python3"
sys.path.insert(0, str(sdt_path))

try:
    import cantera as ct
    from sdtoolbox.postshock import CJspeed, PostShock_eq, PostShock_fr, FHFP
    from sdtoolbox.thermo import eq_state, state, soundspeed_eq, soundspeed_fr
    from sdtoolbox.utilities import CJspeed_plot
    CANTERA_AVAILABLE = True
except ImportError as e:
    CANTERA_AVAILABLE = False
    IMPORT_ERROR = str(e)

class SDTBridge:
    """Bridge class for SDToolbox integration"""
    
    PROTOCOL_VERSION = "1.0"
    
    def __init__(self):
        self.mechanism_cache = {}
        self.calculation_count = 0
        self.error_count = 0
        self.start_time = time.time()
        
    def process_request(self, request_data):
        """Process a single JSON request"""
        try:
            # Validate request format
            if not isinstance(request_data, dict):
                raise ValueError("Request must be a JSON object")
            
            if "function" not in request_data:
                raise ValueError("Request must specify 'function' field")
            
            function_name = request_data["function"]
            self.calculation_count += 1
            
            # Dispatch to appropriate function
            if function_name == "CJspeed":
                result = self._calculate_cj_speed(request_data)
            elif function_name == "PostShock_eq":
                result = self._calculate_post_shock_eq(request_data)
            elif function_name == "PostShock_fr":
                result = self._calculate_post_shock_fr(request_data)
            elif function_name == "eq_state":
                result = self._calculate_eq_state(request_data)
            elif function_name == "state":
                result = self._calculate_state(request_data)
            elif function_name == "soundspeed_eq":
                result = self._calculate_soundspeed_eq(request_data)
            elif function_name == "soundspeed_fr":
                result = self._calculate_soundspeed_fr(request_data)
            elif function_name == "FHFP":
                result = self._calculate_fhfp(request_data)
            elif function_name == "test_connection":
                result = self._test_connection(request_data)
            elif function_name == "validate_mechanism":
                result = self._validate_mechanism(request_data)
            elif function_name == "batch":
                result = self._process_batch(request_data)
            else:
                raise ValueError(f"Unknown function: {function_name}")
            
            # Build success response
            response = {
                "success": True,
                "result": result,
                "errors": [],
                "warnings": [],
                "metadata": {
                    "function": function_name,
                    "protocol_version": self.PROTOCOL_VERSION,
                    "calculation_count": self.calculation_count,
                    "processing_time_ms": (time.time() - self.start_time) * 1000
                }
            }
            
            return response
            
        except Exception as e:
            self.error_count += 1
            
            # Build error response
            response = {
                "success": False,
                "result": None,
                "errors": [str(e)],
                "warnings": [],
                "metadata": {
                    "function": request_data.get("function", "unknown"),
                    "protocol_version": self.PROTOCOL_VERSION,
                    "error_count": self.error_count,
                    "traceback": traceback.format_exc()
                }
            }
            
            return response
    
    def _calculate_cj_speed(self, request):
        """Calculate Chapman-Jouguet detonation speed"""
        # Extract parameters
        P1 = request["P1"]          # Initial pressure [Pa]
        T1 = request["T1"]          # Initial temperature [K]
        q = request["q"]            # Composition string
        mech = request["mech"]      # Mechanism file
        
        # Optional parameters
        full_output = request.get("fullOutput", False)
        
        # Validate inputs
        if P1 <= 0:
            raise ValueError(f"Pressure must be positive: {P1}")
        if T1 <= 0:
            raise ValueError(f"Temperature must be positive: {T1}")
        if not q:
            raise ValueError("Composition string cannot be empty")
        
        # Find mechanism file
        mech_path = self._find_mechanism_file(mech)
        
        # Call SDT CJspeed function
        if full_output:
            cj_speed, R2, plot_data = CJspeed(P1, T1, q, mech_path, fullOutput=True)
            
            result = {
                "cj_speed": float(cj_speed),
                "R2": float(R2),
                "plot_data": {
                    "density_ratios": [float(x) for x in plot_data[0]],
                    "wave_speeds": [float(x) for x in plot_data[1]], 
                    "minimum_density": float(plot_data[2]),
                    "parabola_coeffs": [float(plot_data[3]), float(plot_data[4]), float(plot_data[5])]
                }
            }
        else:
            cj_speed = CJspeed(P1, T1, q, mech_path)
            result = {"cj_speed": float(cj_speed)}
        
        return result
    
    def _calculate_post_shock_eq(self, request):
        """Calculate equilibrium post-shock state"""
        U1 = request["U1"]          # Shock speed [m/s]
        P1 = request["P1"]          # Initial pressure [Pa]
        T1 = request["T1"]          # Initial temperature [K]
        q = request["q"]            # Composition string
        mech = request["mech"]      # Mechanism file
        
        # Find mechanism file
        mech_path = self._find_mechanism_file(mech)
        
        # Call SDT PostShock_eq function
        gas = PostShock_eq(U1, P1, T1, q, mech_path)
        
        # Extract thermodynamic properties
        result = self._extract_gas_properties(gas)
        
        return result
    
    def _calculate_post_shock_fr(self, request):
        """Calculate frozen post-shock state"""
        U1 = request["U1"]          # Shock speed [m/s]
        P1 = request["P1"]          # Initial pressure [Pa]
        T1 = request["T1"]          # Initial temperature [K]
        q = request["q"]            # Composition string
        mech = request["mech"]      # Mechanism file
        
        # Find mechanism file
        mech_path = self._find_mechanism_file(mech)
        
        # Call SDT PostShock_fr function
        gas = PostShock_fr(U1, P1, T1, q, mech_path)
        
        # Extract thermodynamic properties
        result = self._extract_gas_properties(gas)
        
        return result
    
    def _calculate_eq_state(self, request):
        """Calculate equilibrium state at given density and temperature"""
        density = request["density"]        # [kg/m³]
        temperature = request["temperature"] # [K]
        composition = request["composition"] # Composition string
        mechanism = request["mechanism"]     # Mechanism file
        
        # Find mechanism file
        mech_path = self._find_mechanism_file(mechanism)
        
        # Create gas object
        gas = ct.Solution(mech_path)
        gas.TPX = temperature, 101325.0, composition  # Initial state
        
        # Call SDT eq_state function
        P, H = eq_state(gas, density, temperature)
        
        # Extract properties
        result = self._extract_gas_properties(gas)
        result["pressure_from_eq_state"] = float(P)
        result["enthalpy_from_eq_state"] = float(H)
        
        return result
    
    def _calculate_state(self, request):
        """Calculate frozen state at given density and temperature"""
        density = request["density"]        # [kg/m³]
        temperature = request["temperature"] # [K]
        composition = request["composition"] # Composition string
        mechanism = request["mechanism"]     # Mechanism file
        
        # Find mechanism file
        mech_path = self._find_mechanism_file(mechanism)
        
        # Create gas object
        gas = ct.Solution(mech_path)
        gas.TPX = temperature, 101325.0, composition  # Initial state
        
        # Call SDT state function
        P, H = state(gas, density, temperature)
        
        # Extract properties
        result = self._extract_gas_properties(gas)
        result["pressure_from_state"] = float(P)
        result["enthalpy_from_state"] = float(H)
        
        return result
    
    def _calculate_soundspeed_eq(self, request):
        """Calculate equilibrium sound speed"""
        # Gas state parameters
        pressure = request["pressure"]
        temperature = request["temperature"]
        composition = request["composition"]
        mechanism = request["mechanism"]
        
        # Find mechanism file
        mech_path = self._find_mechanism_file(mechanism)
        
        # Create gas object and set state
        gas = ct.Solution(mech_path)
        gas.TPX = temperature, pressure, composition
        gas.equilibrate('TP')
        
        # Call SDT soundspeed_eq function
        sound_speed = soundspeed_eq(gas)
        
        result = {"sound_speed": float(sound_speed)}
        
        return result
    
    def _calculate_soundspeed_fr(self, request):
        """Calculate frozen sound speed"""
        # Gas state parameters
        pressure = request["pressure"]
        temperature = request["temperature"]
        composition = request["composition"]
        mechanism = request["mechanism"]
        
        # Find mechanism file
        mech_path = self._find_mechanism_file(mechanism)
        
        # Create gas object and set state
        gas = ct.Solution(mech_path)
        gas.TPX = temperature, pressure, composition
        
        # Call SDT soundspeed_fr function
        sound_speed = soundspeed_fr(gas)
        
        result = {"sound_speed": float(sound_speed)}
        
        return result
    
    def _calculate_fhfp(self, request):
        """Calculate conservation equation errors (FHFP)"""
        w1 = request["w1"]              # Wave speed [m/s]
        state1 = request["state1"]      # Initial state
        state2 = request["state2"]      # Final state
        
        # Create gas objects from state data
        gas1 = self._create_gas_from_state(state1)
        gas2 = self._create_gas_from_state(state2)
        
        # Call SDT FHFP function
        FH, FP = FHFP(w1, gas2, gas1)
        
        result = {
            "enthalpy_error": float(FH),
            "pressure_error": float(FP)
        }
        
        return result
    
    def _test_connection(self, request):
        """Test connection and SDToolbox availability"""
        result = {
            "cantera_available": CANTERA_AVAILABLE,
            "sdt_path": str(sdt_path),
            "python_version": sys.version,
            "protocol_version": self.PROTOCOL_VERSION
        }
        
        if CANTERA_AVAILABLE:
            result["cantera_version"] = ct.__version__
            # Test basic CJ calculation
            try:
                test_speed = CJspeed(101325, 300, "H2:2 O2:1", "gri30.cti")
                result["test_calculation"] = float(test_speed)
                result["test_passed"] = True
            except Exception as e:
                result["test_passed"] = False
                result["test_error"] = str(e)
        else:
            result["import_error"] = IMPORT_ERROR
            
        return result
    
    def _validate_mechanism(self, request):
        """Validate mechanism file"""
        mechanism = request["mechanism"]
        
        try:
            mech_path = self._find_mechanism_file(mechanism)
            
            # Try to load mechanism
            gas = ct.Solution(mech_path)
            
            result = {
                "valid": True,
                "path": mech_path,
                "n_species": gas.n_species,
                "n_reactions": gas.n_reactions,
                "species_names": gas.species_names
            }
            
        except Exception as e:
            result = {
                "valid": False,
                "error": str(e),
                "path": None
            }
            
        return result
    
    def _process_batch(self, request):
        """Process multiple requests in batch"""
        requests = request["requests"]
        
        if not isinstance(requests, list):
            raise ValueError("Batch requests must be a list")
        
        if len(requests) > 100:  # Safety limit
            raise ValueError("Batch size too large (max 100)")
        
        results = []
        for i, sub_request in enumerate(requests):
            try:
                sub_result = self.process_request(sub_request)
                results.append(sub_result)
            except Exception as e:
                error_result = {
                    "success": False,
                    "result": None,
                    "errors": [f"Batch item {i}: {str(e)}"],
                    "warnings": [],
                    "metadata": {"batch_index": i}
                }
                results.append(error_result)
        
        return {"batch_results": results}
    
    def _extract_gas_properties(self, gas):
        """Extract thermodynamic properties from Cantera gas object"""
        return {
            "pressure": float(gas.P),
            "temperature": float(gas.T),
            "density": float(gas.density),
            "enthalpy": float(gas.enthalpy_mass),
            "entropy": float(gas.entropy_mass),
            "mean_molecular_weight": float(gas.mean_molecular_weight),
            "cp": float(gas.cp_mass),
            "cv": float(gas.cv_mass),
            "composition": {name: float(gas[name].X[0]) for name in gas.species_names}
        }
    
    def _create_gas_from_state(self, state_data):
        """Create Cantera gas object from state data"""
        gas = ct.Solution(self._find_mechanism_file(state_data["mechanism"]))
        gas.TPX = state_data["temperature"], state_data["pressure"], state_data["composition"]
        return gas
    
    def _find_mechanism_file(self, mechanism):
        """Find mechanism file in standard locations"""
        if os.path.isabs(mechanism) and os.path.exists(mechanism):
            return mechanism
        
        # Search in standard locations
        search_paths = [
            sdt_path / "sdtoolbox" / "cti",
            project_root / "RDE" / "data" / "SDToolbox" / "cti",
            project_root / "src" / "physics" / "mechanisms",
            Path.cwd()
        ]
        
        for search_path in search_paths:
            candidate = search_path / mechanism
            if candidate.exists():
                return str(candidate)
            
            # Try common mechanism names
            for subdir in ["SanDiego", ".", ""]:
                if subdir:
                    candidate = search_path / subdir / mechanism
                    if candidate.exists():
                        return str(candidate)
        
        # If not found, try relative to SDT cti directory
        sdt_cti_path = sdt_path.parent / "cti" / mechanism
        if sdt_cti_path.exists():
            return str(sdt_cti_path)
            
        raise FileNotFoundError(f"Mechanism file not found: {mechanism}")

def main():
    """Main entry point for command-line usage"""
    if not CANTERA_AVAILABLE:
        error_response = {
            "success": False,
            "result": None,
            "errors": [f"Cantera/SDToolbox not available: {IMPORT_ERROR}"],
            "warnings": [],
            "metadata": {"protocol_version": SDTBridge.PROTOCOL_VERSION}
        }
        print(json.dumps(error_response, indent=2))
        sys.exit(1)
    
    bridge = SDTBridge()
    
    try:
        # Read JSON input from stdin
        input_text = sys.stdin.read().strip()
        if not input_text:
            raise ValueError("No input provided")
        
        request_data = json.loads(input_text)
        
        # Process request
        response = bridge.process_request(request_data)
        
        # Output JSON response
        print(json.dumps(response, indent=2))
        
    except json.JSONDecodeError as e:
        error_response = {
            "success": False,
            "result": None,
            "errors": [f"Invalid JSON input: {str(e)}"],
            "warnings": [],
            "metadata": {"protocol_version": SDTBridge.PROTOCOL_VERSION}
        }
        print(json.dumps(error_response, indent=2))
        sys.exit(1)
        
    except Exception as e:
        error_response = {
            "success": False,
            "result": None,
            "errors": [str(e)],
            "warnings": [],
            "metadata": {
                "protocol_version": SDTBridge.PROTOCOL_VERSION,
                "traceback": traceback.format_exc()
            }
        }
        print(json.dumps(error_response, indent=2))
        sys.exit(1)

if __name__ == "__main__":
    main()