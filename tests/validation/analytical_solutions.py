#!/usr/bin/env python3
"""
Analytical Solutions Library for CFD Tool Validation

This module provides analytical solutions and correlations for validating
OpenFOAM MCP server CFD tools against established physics principles.

Author: OpenFOAM MCP Development Team
License: MIT
"""

import math
import numpy as np
from typing import Dict, Any, List, Tuple
from dataclasses import dataclass


@dataclass
class ValidationResult:
    """Container for validation results"""
    computed_value: float
    analytical_value: float
    relative_error: float
    absolute_error: float
    passes_criteria: bool
    criteria_threshold: float


class PipeFlowSolutions:
    """Analytical solutions for internal pipe flow"""
    
    @staticmethod
    def hagen_poiseuille_pressure_drop(
        density: float, velocity: float, diameter: float, 
        length: float, viscosity: float
    ) -> float:
        """
        Hagen-Poiseuille equation for laminar pipe flow pressure drop
        
        Args:
            density: Fluid density [kg/m³]
            velocity: Average velocity [m/s]
            diameter: Pipe diameter [m]
            length: Pipe length [m]
            viscosity: Dynamic viscosity [Pa·s]
            
        Returns:
            Pressure drop [Pa]
        """
        reynolds = density * velocity * diameter / viscosity
        if reynolds > 2300:
            raise ValueError(f"Reynolds number {reynolds:.1f} > 2300, not laminar flow")
        
        # Hagen-Poiseuille: ΔP = 32μVL/D²
        return 32 * viscosity * velocity * length / (diameter ** 2)
    
    @staticmethod
    def blasius_friction_factor(reynolds: float) -> float:
        """
        Blasius correlation for turbulent pipe flow friction factor
        
        Args:
            reynolds: Reynolds number
            
        Returns:
            Friction factor [-]
        """
        if reynolds < 4000:
            raise ValueError(f"Reynolds number {reynolds:.1f} < 4000, not turbulent flow")
        if reynolds > 100000:
            raise ValueError(f"Reynolds number {reynolds:.1f} > 100000, outside Blasius range")
        
        # Blasius: f = 0.316 * Re^(-1/4)
        return 0.316 * (reynolds ** -0.25)
    
    @staticmethod
    def laminar_friction_factor(reynolds: float) -> float:
        """
        Laminar pipe flow friction factor
        
        Args:
            reynolds: Reynolds number
            
        Returns:
            Friction factor [-]
        """
        if reynolds > 2300:
            raise ValueError(f"Reynolds number {reynolds:.1f} > 2300, not laminar flow")
        
        # Laminar: f = 64/Re
        return 64.0 / reynolds
    
    @staticmethod
    def pressure_drop_from_friction(
        friction_factor: float, density: float, velocity: float,
        diameter: float, length: float
    ) -> float:
        """
        Pressure drop from Darcy-Weisbach equation
        
        Args:
            friction_factor: Friction factor [-]
            density: Fluid density [kg/m³]
            velocity: Average velocity [m/s]
            diameter: Pipe diameter [m]
            length: Pipe length [m]
            
        Returns:
            Pressure drop [Pa]
        """
        # Darcy-Weisbach: ΔP = f * (L/D) * (ρV²/2)
        return friction_factor * (length / diameter) * (density * velocity ** 2 / 2)


class ExternalFlowSolutions:
    """Analytical solutions for external flow around objects"""
    
    @staticmethod
    def cylinder_drag_coefficient(reynolds: float) -> float:
        """
        Drag coefficient for flow over circular cylinder
        
        Args:
            reynolds: Reynolds number based on cylinder diameter
            
        Returns:
            Drag coefficient [-]
        """
        if reynolds < 0.1:
            # Stokes flow
            return 24.0 / reynolds + 6.0 / (1.0 + math.sqrt(reynolds)) + 0.4
        elif reynolds < 1:
            # Low Reynolds
            return 24.0 / reynolds + 4.0 / math.sqrt(reynolds) + 0.4
        elif reynolds < 40:
            # Moderate Reynolds
            return 24.0 / reynolds + 6.0 / (1.0 + math.sqrt(reynolds)) + 0.4
        elif reynolds < 1000:
            # Intermediate Reynolds
            return 24.0 / reynolds * (1.0 + 0.15 * reynolds ** 0.687) + 0.42 / (1.0 + 42500.0 / reynolds ** 1.16)
        elif reynolds < 200000:
            # High Reynolds (before critical)
            return 0.47 + 24.0 / reynolds + 6.0 / (1.0 + math.sqrt(reynolds))
        else:
            # Supercritical
            return 0.2
    
    @staticmethod
    def sphere_drag_coefficient(reynolds: float) -> float:
        """
        Drag coefficient for flow over sphere
        
        Args:
            reynolds: Reynolds number based on sphere diameter
            
        Returns:
            Drag coefficient [-]
        """
        if reynolds < 0.1:
            # Stokes flow
            return 24.0 / reynolds
        elif reynolds < 1000:
            # Intermediate Reynolds
            return 24.0 / reynolds * (1.0 + 0.15 * reynolds ** 0.687)
        elif reynolds < 300000:
            # Newton's regime
            return 0.44
        else:
            # Critical Reynolds
            return 0.1
    
    @staticmethod
    def flat_plate_boundary_layer_thickness(
        distance: float, reynolds_x: float
    ) -> float:
        """
        Blasius boundary layer thickness for flat plate
        
        Args:
            distance: Distance from leading edge [m]
            reynolds_x: Local Reynolds number
            
        Returns:
            Boundary layer thickness [m]
        """
        if reynolds_x < 100000:
            # Laminar boundary layer
            return 5.0 * distance / math.sqrt(reynolds_x)
        else:
            # Turbulent boundary layer (approximate)
            return 0.37 * distance / (reynolds_x ** 0.2)
    
    @staticmethod
    def flat_plate_skin_friction(reynolds_x: float) -> float:
        """
        Local skin friction coefficient for flat plate
        
        Args:
            reynolds_x: Local Reynolds number
            
        Returns:
            Skin friction coefficient [-]
        """
        if reynolds_x < 500000:
            # Laminar: Cf = 0.664/√(Re_x)
            return 0.664 / math.sqrt(reynolds_x)
        else:
            # Turbulent: Cf = 0.0592/Re_x^(1/5)
            return 0.0592 / (reynolds_x ** 0.2)
    
    @staticmethod
    def getBuildingDragCoefficient(reynolds: float, height_to_width: float) -> float:
        """
        Building drag coefficient correlation
        
        Args:
            reynolds: Reynolds number based on building height
            height_to_width: Building height to width ratio
            
        Returns:
            Drag coefficient [-]
        """
        # Simplified building drag coefficient
        # Based on typical values for rectangular buildings
        if height_to_width < 0.5:
            # Wide building
            base_cd = 1.0
        elif height_to_width < 1.5:
            # Square-ish building  
            base_cd = 1.2
        elif height_to_width < 3.0:
            # Tall building
            base_cd = 1.4
        else:
            # Very tall building
            base_cd = 1.6
            
        # Reynolds number effect (minimal for buildings)
        if reynolds > 1e6:
            reynolds_factor = 0.95  # Slight reduction at high Re
        else:
            reynolds_factor = 1.0
            
        return base_cd * reynolds_factor


class HeatTransferSolutions:
    """Analytical solutions for heat transfer"""
    
    @staticmethod
    def dittus_boelter_nusselt(reynolds: float, prandtl: float, heating: bool = True) -> float:
        """
        Dittus-Boelter correlation for forced convection in pipes
        
        Args:
            reynolds: Reynolds number
            prandtl: Prandtl number
            heating: True for heating, False for cooling
            
        Returns:
            Nusselt number [-]
        """
        if reynolds < 10000:
            raise ValueError(f"Reynolds number {reynolds:.1f} < 10000, not in turbulent range")
        if not (0.7 <= prandtl <= 120):
            raise ValueError(f"Prandtl number {prandtl:.2f} outside valid range [0.7, 120]")
        
        # Dittus-Boelter: Nu = 0.023 * Re^0.8 * Pr^n
        n = 0.4 if heating else 0.3
        return 0.023 * (reynolds ** 0.8) * (prandtl ** n)
    
    @staticmethod
    def gnielinski_nusselt(reynolds: float, prandtl: float, friction_factor: float) -> float:
        """
        Gnielinski correlation for forced convection in pipes
        
        Args:
            reynolds: Reynolds number
            prandtl: Prandtl number
            friction_factor: Friction factor
            
        Returns:
            Nusselt number [-]
        """
        if not (3000 <= reynolds <= 5e6):
            raise ValueError(f"Reynolds number {reynolds:.1f} outside valid range [3000, 5e6]")
        if not (0.5 <= prandtl <= 2000):
            raise ValueError(f"Prandtl number {prandtl:.2f} outside valid range [0.5, 2000]")
        
        # Gnielinski: Nu = (f/8)(Re-1000)Pr / (1 + 12.7√(f/8)(Pr^(2/3) - 1))
        numerator = (friction_factor / 8) * (reynolds - 1000) * prandtl
        denominator = 1 + 12.7 * math.sqrt(friction_factor / 8) * (prandtl ** (2/3) - 1)
        return numerator / denominator
    
    @staticmethod
    def rayleigh_nusselt_vertical_plate(rayleigh: float, prandtl: float) -> float:
        """
        Natural convection correlation for vertical plate
        
        Args:
            rayleigh: Rayleigh number
            prandtl: Prandtl number
            
        Returns:
            Nusselt number [-]
        """
        if rayleigh < 1e4:
            # Low Rayleigh number
            return 0.68 + 0.67 * (rayleigh ** 0.25) / ((1 + (0.492 / prandtl) ** (9/16)) ** (4/9))
        elif rayleigh < 1e9:
            # Moderate Rayleigh number
            return 0.68 + 0.67 * (rayleigh ** 0.25) / ((1 + (0.492 / prandtl) ** (9/16)) ** (4/9))
        else:
            # High Rayleigh number
            return (0.825 + 0.387 * (rayleigh ** (1/6)) / 
                   ((1 + (0.492 / prandtl) ** (9/16)) ** (8/27))) ** 2
    
    @staticmethod
    def churchill_chu_nusselt(rayleigh: float, prandtl: float) -> float:
        """
        Churchill-Chu correlation for natural convection
        
        Args:
            rayleigh: Rayleigh number
            prandtl: Prandtl number
            
        Returns:
            Nusselt number [-]
        """
        # Churchill-Chu correlation (valid for all Ra)
        return (0.825 + 0.387 * (rayleigh ** (1/6)) / 
               ((1 + (0.492 / prandtl) ** (9/16)) ** (8/27))) ** 2
    
    @staticmethod
    def conduction_steady_state_1d(
        thermal_conductivity: float, area: float, 
        temperature_difference: float, thickness: float
    ) -> float:
        """
        1D steady-state conduction heat transfer rate
        
        Args:
            thermal_conductivity: Material thermal conductivity [W/m·K]
            area: Heat transfer area [m²]
            temperature_difference: Temperature difference [K]
            thickness: Material thickness [m]
            
        Returns:
            Heat transfer rate [W]
        """
        # Fourier's law: q = kA(ΔT)/L
        return thermal_conductivity * area * temperature_difference / thickness


class MultiphaseFlowSolutions:
    """Analytical solutions for multiphase flow"""
    
    @staticmethod
    def dam_break_front_position(time: float, gravity: float, height: float) -> float:
        """
        Dam break front position (shallow water approximation)
        
        Args:
            time: Time after dam break [s]
            gravity: Gravitational acceleration [m/s²]
            height: Initial water height [m]
            
        Returns:
            Front position [m]
        """
        # Shallow water solution: x = 2*√(gh)*t
        return 2.0 * math.sqrt(gravity * height) * time
    
    @staticmethod
    def capillary_rise_height(
        surface_tension: float, contact_angle: float, 
        density: float, gravity: float, radius: float
    ) -> float:
        """
        Capillary rise height in circular tube
        
        Args:
            surface_tension: Surface tension [N/m]
            contact_angle: Contact angle [radians]
            density: Liquid density [kg/m³]
            gravity: Gravitational acceleration [m/s²]
            radius: Tube radius [m]
            
        Returns:
            Capillary rise height [m]
        """
        # Young-Laplace: h = 2σcos(θ)/(ρgr)
        return 2.0 * surface_tension * math.cos(contact_angle) / (density * gravity * radius)
    
    @staticmethod
    def bubble_terminal_velocity(
        diameter: float, density_liquid: float, density_gas: float,
        viscosity: float, surface_tension: float, gravity: float
    ) -> float:
        """
        Terminal velocity of rising bubble
        
        Args:
            diameter: Bubble diameter [m]
            density_liquid: Liquid density [kg/m³]
            density_gas: Gas density [kg/m³]
            viscosity: Liquid viscosity [Pa·s]
            surface_tension: Surface tension [N/m]
            gravity: Gravitational acceleration [m/s²]
            
        Returns:
            Terminal velocity [m/s]
        """
        # Simplified correlation for small bubbles
        if diameter < 0.001:  # Small bubbles (Stokes regime)
            return (density_liquid - density_gas) * gravity * diameter ** 2 / (18 * viscosity)
        else:  # Larger bubbles (empirical correlation)
            morton = gravity * viscosity ** 4 * (density_liquid - density_gas) / (density_liquid ** 2 * surface_tension ** 3)
            eotvos = gravity * (density_liquid - density_gas) * diameter ** 2 / surface_tension
            
            # Grace correlation (simplified)
            if morton < 1e-3:
                return math.sqrt(2.14 * surface_tension / (density_liquid * diameter) + 0.505 * gravity * diameter)
            else:
                return math.sqrt(gravity * diameter * (density_liquid - density_gas) / density_liquid)
    
    @staticmethod
    def weber_number(velocity: float, density: float, diameter: float, surface_tension: float) -> float:
        """
        Weber number for droplet/bubble dynamics
        
        Args:
            velocity: Relative velocity [m/s]
            density: Continuous phase density [kg/m³]
            diameter: Droplet/bubble diameter [m]
            surface_tension: Surface tension [N/m]
            
        Returns:
            Weber number [-]
        """
        return density * velocity ** 2 * diameter / surface_tension
    
    @staticmethod
    def bond_number(density_diff: float, gravity: float, length: float, surface_tension: float) -> float:
        """
        Bond number (gravity vs surface tension forces)
        
        Args:
            density_diff: Density difference [kg/m³]
            gravity: Gravitational acceleration [m/s²]
            length: Characteristic length [m]
            surface_tension: Surface tension [N/m]
            
        Returns:
            Bond number [-]
        """
        return density_diff * gravity * length ** 2 / surface_tension


class ValidationUtils:
    """Utilities for validation calculations"""
    
    @staticmethod
    def calculate_relative_error(computed: float, analytical: float) -> float:
        """Calculate relative error as percentage"""
        if abs(analytical) < 1e-12:
            return float('inf') if abs(computed) > 1e-12 else 0.0
        return abs(computed - analytical) / abs(analytical) * 100.0
    
    @staticmethod
    def calculate_absolute_error(computed: float, analytical: float) -> float:
        """Calculate absolute error"""
        return abs(computed - analytical)
    
    @staticmethod
    def validate_result(
        computed: float, analytical: float, 
        threshold_percent: float = 5.0
    ) -> ValidationResult:
        """
        Validate computed result against analytical solution
        
        Args:
            computed: Computed value
            analytical: Analytical reference value
            threshold_percent: Acceptance threshold as percentage
            
        Returns:
            ValidationResult object
        """
        rel_error = ValidationUtils.calculate_relative_error(computed, analytical)
        abs_error = ValidationUtils.calculate_absolute_error(computed, analytical)
        passes = rel_error <= threshold_percent
        
        return ValidationResult(
            computed_value=computed,
            analytical_value=analytical,
            relative_error=rel_error,
            absolute_error=abs_error,
            passes_criteria=passes,
            criteria_threshold=threshold_percent
        )
    
    @staticmethod
    def print_validation_summary(results: List[ValidationResult], test_name: str) -> None:
        """Print summary of validation results"""
        total_tests = len(results)
        passed_tests = sum(1 for r in results if r.passes_criteria)
        
        print(f"\n{'='*60}")
        print(f"VALIDATION SUMMARY: {test_name}")
        print(f"{'='*60}")
        print(f"Total tests: {total_tests}")
        print(f"Passed: {passed_tests}")
        print(f"Failed: {total_tests - passed_tests}")
        print(f"Success rate: {passed_tests/total_tests*100:.1f}%")
        
        if results:
            errors = [r.relative_error for r in results if r.relative_error != float('inf')]
            if errors:
                print(f"Mean error: {np.mean(errors):.2f}%")
                print(f"Max error: {max(errors):.2f}%")
                print(f"Std error: {np.std(errors):.2f}%")
        
        print(f"{'='*60}\n")


if __name__ == "__main__":
    # Example usage and basic tests
    print("Testing Analytical Solutions Library...")
    
    # Test pipe flow solutions
    try:
        pressure_drop = PipeFlowSolutions.hagen_poiseuille_pressure_drop(
            density=1000, velocity=1.0, diameter=0.1, length=10.0, viscosity=1e-3
        )
        print(f"Hagen-Poiseuille pressure drop: {pressure_drop:.2f} Pa")
        
        friction_factor = PipeFlowSolutions.blasius_friction_factor(10000)
        print(f"Blasius friction factor: {friction_factor:.4f}")
        
    except Exception as e:
        print(f"Error in pipe flow tests: {e}")
    
    # Test external flow solutions  
    try:
        cd_cylinder = ExternalFlowSolutions.cylinder_drag_coefficient(1000)
        print(f"Cylinder drag coefficient: {cd_cylinder:.3f}")
        
        cd_sphere = ExternalFlowSolutions.sphere_drag_coefficient(1000)
        print(f"Sphere drag coefficient: {cd_sphere:.3f}")
        
    except Exception as e:
        print(f"Error in external flow tests: {e}")
    
    print("Analytical solutions library loaded successfully!")