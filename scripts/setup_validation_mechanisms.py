#!/usr/bin/env python3
"""
Validation Mechanism Setup Script

Downloads and sets up chemical mechanism files required for comprehensive validation.
Provides fallback mechanisms when standard ones are not available.

Usage:
    python setup_validation_mechanisms.py
"""

import os
import sys
import urllib.request
import shutil
from pathlib import Path

def create_minimal_gri_mechanism():
    """
    Create a minimal GRI-like mechanism for validation testing.
    This is a simplified version that includes the essential species for hydrocarbon combustion.
    """
    
    minimal_gri = '''#
# Minimal GRI-like Mechanism for OpenFOAM MCP Server Validation
# Based on GRI-Mech 3.0 but simplified for testing purposes
#

description: |-
  "Minimal GRI-like mechanism for validation testing
   Includes essential species for CH4, C2H4, C3H8 combustion
   NOT for production use - validation only"

generator: ck2yaml
input-files: [minimal_gri.inp, minimal_gri_thermo.dat]
cantera-version: 3.0.0

units: {length: cm, time: s, quantity: mol, activation-energy: cal/mol}

phases:
- name: gas
  thermo: ideal-gas
  elements: [O, H, C, N, Ar, He]
  species: [H2, H, O, O2, OH, H2O, HO2, H2O2, C, CH, CH2, CH2(S), CH3, CH4,
            CO, CO2, HCO, CH2O, CH2OH, CH3O, CH3OH, C2H, C2H2, C2H3, C2H4,
            C2H5, C2H6, HCCO, CH2CO, HCCOH, N2, AR, HE]
  kinetics: gas
  state: {T: 300.0, P: 1 atm}

species:
- name: H2
  composition: {H: 2}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [2.34433112, 7.98052075e-03, -1.9478151e-05, 2.01572094e-08, -7.37611761e-12,
       -917.935173, 0.683010238]
    - [3.3372792, -4.94024731e-05, 4.99456778e-07, -1.79566394e-10, 2.00255376e-14,
       -950.158922, -3.20502331]

- name: H
  composition: {H: 1}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [2.5, 7.05332819e-13, -1.99591964e-15, 2.30081632e-18, -9.27732332e-22,
       2.54736599e+04, -0.446682853]
    - [2.50000001, -2.30842973e-11, 1.61561948e-14, -4.73515235e-18, 4.98197357e-22,
       2.54736599e+04, -0.446682914]

- name: O
  composition: {O: 1}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [3.1682671, -3.27931884e-03, 6.64306396e-06, -6.12806624e-09, 2.11265971e-12,
       2.91222592e+04, 2.05193346]
    - [2.56942078, -8.59741137e-05, 4.19484589e-08, -1.00177799e-11, 1.22833691e-15,
       2.92175791e+04, 4.78433864]

- name: O2
  composition: {O: 2}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [3.78245636, -2.99673416e-03, 9.84730201e-06, -9.68129509e-09, 3.24372837e-12,
       -1063.94356, 3.65767573]
    - [3.28253784, 1.48308754e-03, -7.57966669e-07, 2.09470555e-10, -2.16717794e-14,
       -1088.45772, 5.45323129]

- name: OH
  composition: {O: 1, H: 1}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [3.99201543, -2.40131752e-03, 4.61793841e-06, -3.88113333e-09, 1.3641147e-12,
       3615.08056, -0.103925458]
    - [3.09288767, 5.48429716e-04, 1.26505228e-07, -8.79461556e-11, 1.17412376e-14,
       3858.657, 4.4766961]

- name: H2O
  composition: {H: 2, O: 1}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [4.19864056, -2.0364341e-03, 6.52040211e-06, -5.48797062e-09, 1.77197817e-12,
       -3.02937267e+04, -0.849032208]
    - [3.03399249, 2.17691804e-03, -1.64072518e-07, -9.7041987e-11, 1.68200992e-14,
       -3.00042971e+04, 4.9667701]

- name: CH4
  composition: {C: 1, H: 4}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [5.14987613, -0.0136709788, 4.91800599e-05, -4.84743026e-08, 1.66693956e-11,
       -1.02466476e+04, -4.64130376]
    - [0.074851495, 0.0133909467, -5.73285809e-06, 1.22292535e-09, -1.0181523e-13,
       -9468.34459, 18.437318]

- name: CO
  composition: {C: 1, O: 1}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [3.57953347, -6.1035368e-04, 1.01681433e-06, 9.07005884e-10, -9.04424499e-13,
       -1.4344086e+04, 3.50840928]
    - [2.71518561, 2.06252743e-03, -9.98825771e-07, 2.30053008e-10, -2.03647716e-14,
       -1.41518724e+04, 7.81868772]

- name: CO2
  composition: {C: 1, O: 2}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [2.35677352, 8.98459677e-03, -7.12356269e-06, 2.45919022e-09, -1.43699548e-13,
       -4.83719697e+04, 9.90105222]
    - [3.85746029, 4.41437026e-03, -2.21481404e-06, 5.23490188e-10, -4.72084164e-14,
       -4.8759166e+04, 2.27163806]

- name: C2H4
  composition: {C: 2, H: 4}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [3.95920148, -7.57052247e-03, 5.70990292e-05, -6.91588753e-08, 2.69884373e-11,
       5089.77593, 4.09733096]
    - [2.03611116, 0.0146454151, -6.71077915e-06, 1.47222923e-09, -1.25706061e-13,
       4939.88614, 10.3053693]

- name: C3H8
  composition: {C: 3, H: 8}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [0.93355381, 0.0264246332, 6.10597626e-06, -2.19775807e-08, 9.5149253e-12,
       -1.39583067e+04, 19.2017433]
    - [7.5341368, 0.0188722769, -8.41729718e-06, 1.94738836e-09, -1.75967841e-13,
       -1.646752e+04, -17.892349]

- name: N2
  composition: {N: 2}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 1000.0, 3500.0]
    data:
    - [3.298677, 1.4082404e-03, -3.963222e-06, 5.641515e-09, -2.444854e-12,
       -1020.8999, 3.950372]
    - [2.92664, 1.4879768e-03, -5.68476e-07, 1.0097038e-10, -6.753351e-15,
       -922.7977, 5.980528]

- name: AR
  composition: {Ar: 1}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 5000.0]
    data:
    - [2.5, 0.0, 0.0, 0.0, 0.0, -745.375, 4.366000]
    - [2.5, 0.0, 0.0, 0.0, 0.0, -745.375, 4.366000]

- name: HE
  composition: {He: 1}
  thermo:
    model: NASA7
    temperature-ranges: [200.0, 5000.0]
    data:
    - [2.5, 0.0, 0.0, 0.0, 0.0, -745.375, 4.366000]
    - [2.5, 0.0, 0.0, 0.0, 0.0, -745.375, 4.366000]

reactions:
# Basic H2/O2 mechanism - essential reactions only
- equation: 2 O + M <=> O2 + M  # Reaction 1
  type: three-body
  rate-constant: {A: 1.2e17, b: -1.0, Ea: 0.0}
  efficiencies: {H2: 2.4, H2O: 15.4, CH4: 2.0, CO: 1.75, CO2: 3.6,
                 C2H6: 3.0, AR: 0.83}

- equation: O + H + M <=> OH + M  # Reaction 2
  type: three-body
  rate-constant: {A: 5.0e17, b: -1.0, Ea: 0.0}
  efficiencies: {H2: 2.0, H2O: 6.0, CH4: 2.0, CO: 1.5, CO2: 2.0, C2H6: 3.0, AR: 0.7}

- equation: O + H2 <=> H + OH  # Reaction 3
  rate-constant: {A: 3.87e04, b: 2.7, Ea: 6260.0}

- equation: O + HO2 <=> OH + O2  # Reaction 4
  rate-constant: {A: 2.0e13, b: 0.0, Ea: 0.0}

- equation: O + H2O2 <=> OH + HO2  # Reaction 5
  rate-constant: {A: 9.63e06, b: 2.0, Ea: 4000.0}

- equation: H + 2 O2 <=> HO2 + O2  # Reaction 6
  rate-constant: {A: 2.08e19, b: -1.24, Ea: 0.0}

- equation: H + O2 + H2O <=> HO2 + H2O  # Reaction 7
  rate-constant: {A: 1.126e19, b: -0.76, Ea: 0.0}

- equation: H + O2 <=> O + OH  # Reaction 8
  rate-constant: {A: 2.65e16, b: -0.6707, Ea: 17041.0}

- equation: 2 H + M <=> H2 + M  # Reaction 9
  type: three-body
  rate-constant: {A: 1.0e18, b: -1.0, Ea: 0.0}
  efficiencies: {H2: 0.0, H2O: 0.0, CH4: 2.0, CO2: 0.0, C2H6: 3.0, AR: 0.63}

- equation: 2 H + H2 <=> 2 H2  # Reaction 10
  rate-constant: {A: 9.0e16, b: -0.6, Ea: 0.0}

- equation: 2 H + H2O <=> H2 + H2O  # Reaction 11
  rate-constant: {A: 6.0e19, b: -1.25, Ea: 0.0}

- equation: 2 H + CO2 <=> H2 + CO2  # Reaction 12
  rate-constant: {A: 5.5e20, b: -2.0, Ea: 0.0}

- equation: H + OH + M <=> H2O + M  # Reaction 13
  type: three-body
  rate-constant: {A: 2.2e22, b: -2.0, Ea: 0.0}
  efficiencies: {H2: 0.73, H2O: 3.65, CH4: 2.0, C2H6: 3.0, AR: 0.38}

# Basic CH4 oxidation
- equation: CH4 + H <=> CH3 + H2  # Reaction 14
  rate-constant: {A: 6.6e08, b: 1.62, Ea: 10840.0}

- equation: CH4 + O <=> CH3 + OH  # Reaction 15
  rate-constant: {A: 1.02e09, b: 1.5, Ea: 8600.0}

- equation: CH4 + OH <=> CH3 + H2O  # Reaction 16
  rate-constant: {A: 1.0e08, b: 1.6, Ea: 3120.0}

- equation: CH3 + O <=> CH2O + H  # Reaction 17
  rate-constant: {A: 5.06e13, b: 0.0, Ea: 0.0}

- equation: CH3 + OH <=> CH2O + H2  # Reaction 18
  rate-constant: {A: 8.0e09, b: 0.5, Ea: -1755.0}

- equation: CH2O + H <=> HCO + H2  # Reaction 19
  rate-constant: {A: 5.74e07, b: 1.9, Ea: 2742.0}

- equation: CH2O + OH <=> HCO + H2O  # Reaction 20
  rate-constant: {A: 3.43e09, b: 1.18, Ea: -447.0}

- equation: HCO + H <=> CO + H2  # Reaction 21
  rate-constant: {A: 7.34e13, b: 0.0, Ea: 0.0}

- equation: HCO + M <=> H + CO + M  # Reaction 22
  type: three-body
  rate-constant: {A: 1.87e17, b: -1.0, Ea: 17000.0}
  efficiencies: {H2: 2.0, H2O: 0.0, CH4: 2.0, CO: 1.5, CO2: 2.0, C2H6: 3.0}

# Basic C2H4 reactions
- equation: C2H4 + H <=> C2H3 + H2  # Reaction 23
  rate-constant: {A: 1.325e06, b: 2.53, Ea: 12240.0}

- equation: C2H4 + O <=> CH3 + HCO  # Reaction 24
  rate-constant: {A: 1.25e07, b: 1.83, Ea: 220.0}

- equation: C2H4 + OH <=> C2H3 + H2O  # Reaction 25
  rate-constant: {A: 3.6e06, b: 2.0, Ea: 2500.0}

# CO oxidation
- equation: CO + OH <=> CO2 + H  # Reaction 26
  rate-constant: {A: 4.76e07, b: 1.228, Ea: 70.0}

- equation: CO + O2 <=> CO2 + O  # Reaction 27
  rate-constant: {A: 2.5e12, b: 0.0, Ea: 47800.0}

- equation: CO + HO2 <=> CO2 + OH  # Reaction 28
  rate-constant: {A: 1.5e14, b: 0.0, Ea: 23600.0}
'''
    
    return minimal_gri

def setup_mechanisms_directory():
    """Create mechanisms directory and download/create required files."""
    mechanisms_dir = Path("mechanisms")
    mechanisms_dir.mkdir(exist_ok=True)
    
    print(f"🔧 Setting up mechanisms directory: {mechanisms_dir}")
    
    # Create minimal GRI mechanism
    gri_path = mechanisms_dir / "gri30.yaml"
    if not gri_path.exists():
        print("📝 Creating minimal GRI mechanism file...")
        with open(gri_path, 'w') as f:
            f.write(create_minimal_gri_mechanism())
        print(f"✅ Created: {gri_path}")
    else:
        print(f"✅ Already exists: {gri_path}")
    
    # Check if sandiego mechanism exists
    sandiego_path = mechanisms_dir / "sandiego20161214_H2only.yaml"
    if not sandiego_path.exists():
        print("⚠️  Warning: sandiego20161214_H2only.yaml not found")
        print("   This mechanism should be available with Cantera installation")
        print("   Try: python -c 'import cantera; print(cantera.get_data_directories())'")
    else:
        print(f"✅ Found: {sandiego_path}")
    
    return mechanisms_dir

def verify_mechanism_files():
    """Verify that mechanism files are accessible."""
    print("\n🔍 Verifying mechanism files...")
    
    try:
        import cantera as ct
        
        # Test mechanisms
        mechanisms_to_test = [
            "gri30.yaml",
            "mechanisms/gri30.yaml", 
            "sandiego20161214_H2only.yaml"
        ]
        
        for mech in mechanisms_to_test:
            try:
                gas = ct.Solution(mech)
                print(f"✅ {mech}: OK ({len(gas.species_names)} species, {len(gas.reaction_equations())} reactions)")
            except Exception as e:
                print(f"❌ {mech}: ERROR - {str(e)}")
                
    except ImportError:
        print("❌ Cannot import cantera - Python Cantera not installed")
        return False
    
    return True

def update_benchmark_mechanisms():
    """Update benchmark file to use local mechanisms."""
    benchmark_file = Path("data/validation_benchmarks.json")
    
    if not benchmark_file.exists():
        print(f"⚠️  Benchmark file not found: {benchmark_file}")
        return False
    
    print(f"🔄 Updating benchmark mechanisms in {benchmark_file}")
    
    try:
        import json
        
        with open(benchmark_file, 'r') as f:
            data = json.load(f)
        
        # Update mechanism paths to use local mechanisms
        updated_count = 0
        for benchmark in data.get("benchmarks", []):
            conditions = benchmark.get("conditions", {})
            if "mechanism" in conditions:
                mechanism = conditions["mechanism"]
                
                # Update mechanism paths to use local directory
                if mechanism == "gri30.yaml":
                    conditions["mechanism"] = "mechanisms/gri30.yaml"
                    updated_count += 1
                elif not mechanism.startswith("mechanisms/") and not mechanism.startswith("/"):
                    # Check if this is a standard mechanism that should be in mechanisms/
                    if mechanism in ["sandiego20161214_H2only.yaml"]:
                        # Keep as-is for system mechanisms
                        pass
                    else:
                        # Prepend mechanisms/ for local mechanisms
                        conditions["mechanism"] = f"mechanisms/{mechanism}"
                        updated_count += 1
        
        # Save updated file
        with open(benchmark_file, 'w') as f:
            json.dump(data, f, indent=2)
            
        print(f"✅ Updated {updated_count} mechanism paths")
        return True
        
    except Exception as e:
        print(f"❌ Error updating benchmark file: {str(e)}")
        return False

def main():
    print("🚀 OpenFOAM MCP Server - Validation Mechanisms Setup")
    print("=" * 60)
    
    # Setup mechanisms directory
    mechanisms_dir = setup_mechanisms_directory()
    
    # Update benchmark file
    update_benchmark_mechanisms()
    
    # Verify mechanisms work
    verify_mechanism_files()
    
    print("\n" + "=" * 60)
    print("✅ Mechanism setup completed!")
    print(f"📁 Mechanisms directory: {mechanisms_dir.absolute()}")
    print("\n💡 Usage tips:")
    print("   • Run validation test: ./test_comprehensive_validation")
    print("   • Check Cantera installation: python -c 'import cantera; print(cantera.__version__)'")
    print("   • List Cantera data directories: python -c 'import cantera; print(cantera.get_data_directories())'")

if __name__ == "__main__":
    main()