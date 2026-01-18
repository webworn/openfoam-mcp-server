/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Turbulence Utilities
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Turbulence utility functions implementation.

\*---------------------------------------------------------------------------*/

#include "turbulence_utils.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        y+ Calculations
\*---------------------------------------------------------------------------*/

double TurbulenceUtils::calculateYPlus(double wallShearStress, double density,
                                       double kinematicViscosity, double firstCellHeight) {
    if (density <= 0 || kinematicViscosity <= 0 || firstCellHeight <= 0) {
        throw std::invalid_argument("Invalid parameters for y+ calculation");
    }

    double uTau = calculateFrictionVelocity(wallShearStress, density);
    return calculateYPlusFromFrictionVelocity(uTau, kinematicViscosity, firstCellHeight);
}

double TurbulenceUtils::calculateYPlusFromFrictionVelocity(double uTau, double kinematicViscosity,
                                                           double firstCellHeight) {
    if (kinematicViscosity <= 0 || firstCellHeight <= 0) {
        throw std::invalid_argument("Invalid parameters for y+ calculation");
    }

    // y+ = (u_τ * y) / ν
    return (uTau * firstCellHeight) / kinematicViscosity;
}

double TurbulenceUtils::calculateFrictionVelocity(double wallShearStress, double density) {
    if (density <= 0) {
        throw std::invalid_argument("Density must be positive");
    }

    // u_τ = √(τ_w / ρ)
    return std::sqrt(std::abs(wallShearStress) / density);
}

double TurbulenceUtils::estimateWallShearStress(double frictionFactor, double density,
                                                 double velocity) {
    // τ_w = f * ρ * V² / 8 (Darcy friction factor relation)
    return frictionFactor * density * velocity * velocity / 8.0;
}

/*---------------------------------------------------------------------------*\
                        First Cell Height Recommendations
\*---------------------------------------------------------------------------*/

double TurbulenceUtils::recommendFirstCellHeight(double targetYPlus, double velocity,
                                                 double density, double kinematicViscosity,
                                                 double characteristicLength) {
    if (velocity <= 0 || density <= 0 || kinematicViscosity <= 0 || characteristicLength <= 0) {
        throw std::invalid_argument("All parameters must be positive");
    }

    // Calculate Reynolds number
    double Re = velocity * characteristicLength / kinematicViscosity;

    // Estimate skin friction coefficient using flat plate correlation
    double Cf;
    if (Re < 5e5) {
        // Laminar: Cf = 0.664 / sqrt(Re)
        Cf = 0.664 / std::sqrt(Re);
    } else {
        // Turbulent: Cf = 0.027 / Re^(1/7) (Prandtl-Schlichting)
        Cf = 0.027 / std::pow(Re, 1.0 / 7.0);
    }

    // Estimate friction velocity: u_τ = V * sqrt(Cf/2)
    double uTau = velocity * std::sqrt(Cf / 2.0);

    // Calculate first cell height: y = y+ * ν / u_τ
    return targetYPlus * kinematicViscosity / uTau;
}

double TurbulenceUtils::calculateFirstCellHeightFromUTau(double targetYPlus, double uTau,
                                                          double kinematicViscosity) {
    if (uTau <= 0 || kinematicViscosity <= 0) {
        throw std::invalid_argument("Parameters must be positive");
    }

    // y = y+ * ν / u_τ
    return targetYPlus * kinematicViscosity / uTau;
}

double TurbulenceUtils::estimateFirstCellHeightFlatPlate(double targetYPlus, double freeStreamVelocity,
                                                          double density, double kinematicViscosity,
                                                          double plateLength) {
    if (freeStreamVelocity <= 0 || density <= 0 || kinematicViscosity <= 0 || plateLength <= 0) {
        throw std::invalid_argument("All parameters must be positive");
    }

    // Reynolds number based on plate length
    double Re = freeStreamVelocity * plateLength / kinematicViscosity;

    // Skin friction coefficient (turbulent, Schlichting correlation)
    double Cf = 0.074 / std::pow(Re, 0.2);

    // Wall shear stress estimate
    double tauW = 0.5 * Cf * density * freeStreamVelocity * freeStreamVelocity;

    // Friction velocity
    double uTau = std::sqrt(tauW / density);

    // First cell height
    return targetYPlus * kinematicViscosity / uTau;
}

double TurbulenceUtils::estimateFirstCellHeightPipe(double targetYPlus, double velocity,
                                                     double density, double kinematicViscosity,
                                                     double pipeDiameter) {
    if (velocity <= 0 || density <= 0 || kinematicViscosity <= 0 || pipeDiameter <= 0) {
        throw std::invalid_argument("All parameters must be positive");
    }

    // Reynolds number
    double Re = velocity * pipeDiameter / kinematicViscosity;

    // Darcy friction factor (Blasius for smooth pipes)
    double f;
    if (Re < 2300) {
        f = 64.0 / Re;
    } else {
        f = 0.316 / std::pow(Re, 0.25);
    }

    // Wall shear stress: τ_w = f * ρ * V² / 8
    double tauW = f * density * velocity * velocity / 8.0;

    // Friction velocity
    double uTau = std::sqrt(tauW / density);

    // First cell height
    return targetYPlus * kinematicViscosity / uTau;
}

/*---------------------------------------------------------------------------*\
                        Wall Function Validity
\*---------------------------------------------------------------------------*/

WallFunctionValidity TurbulenceUtils::checkWallFunctionValidity(double yPlus,
                                                                 const std::string& turbulenceModel) {
    WallFunctionValidity validity;
    validity.turbulenceModel = turbulenceModel;

    if (turbulenceModel == "kOmegaSST") {
        // k-omega SST can work in two modes:
        // 1. Wall-resolved: y+ < 1 (resolves viscous sublayer)
        // 2. Wall function: y+ = 30-300

        validity.minYPlus = 30.0;
        validity.maxYPlus = 300.0;
        validity.targetYPlus = 50.0;

        if (yPlus < 1.0) {
            validity.isValid = true;
            validity.explanation = "y+ < 1: Wall-resolved mode active. SST model will resolve "
                                  "the viscous sublayer directly without wall functions.";
            validity.recommendations.push_back("Ensure sufficient mesh refinement near wall");
            validity.recommendations.push_back("Check that first cell is within viscous sublayer");
        } else if (yPlus >= 30.0 && yPlus <= 300.0) {
            validity.isValid = true;
            validity.explanation = "y+ in log-law region (30-300): Wall functions active and valid. "
                                  "SST model will use wall functions for near-wall treatment.";
            validity.recommendations.push_back("Current mesh is appropriate for wall functions");
        } else if (yPlus > 1.0 && yPlus < 30.0) {
            validity.isValid = false;
            validity.explanation = "y+ in buffer layer (1-30): Neither wall-resolved nor wall function "
                                  "approach is valid. This is the 'danger zone' for turbulence modeling.";
            validity.recommendations.push_back("Either refine mesh to y+ < 1 for wall-resolved approach");
            validity.recommendations.push_back("Or coarsen mesh to y+ > 30 for wall function approach");
        } else {
            validity.isValid = false;
            validity.explanation = "y+ > 300: First cell too far from wall. Wall functions may be "
                                  "inaccurate and gradients will be poorly captured.";
            validity.recommendations.push_back("Refine mesh near wall to reduce y+");
        }

    } else if (turbulenceModel == "kEpsilon") {
        // Standard k-epsilon requires wall functions
        validity.minYPlus = 30.0;
        validity.maxYPlus = 300.0;
        validity.targetYPlus = 50.0;

        if (yPlus >= 30.0 && yPlus <= 300.0) {
            validity.isValid = true;
            validity.explanation = "y+ in log-law region (30-300): Wall functions valid. "
                                  "k-epsilon model with standard wall functions will perform well.";
            validity.recommendations.push_back("Current mesh is appropriate for k-epsilon");
        } else if (yPlus < 30.0) {
            validity.isValid = false;
            validity.explanation = "y+ < 30: First cell in viscous sublayer or buffer layer. "
                                  "Standard k-epsilon wall functions are not designed for this range.";
            validity.recommendations.push_back("Coarsen near-wall mesh to achieve y+ > 30");
            validity.recommendations.push_back("Or consider using k-omega SST for wall-resolved approach");
        } else {
            validity.isValid = false;
            validity.explanation = "y+ > 300: First cell too far from wall. Wall function "
                                  "assumptions may break down.";
            validity.recommendations.push_back("Refine near-wall mesh to achieve y+ < 300");
        }

    } else if (turbulenceModel == "laminar") {
        validity.isValid = true;
        validity.minYPlus = 0.0;
        validity.maxYPlus = 1e10;
        validity.targetYPlus = 1.0;
        validity.explanation = "Laminar flow: y+ constraints from turbulence modeling do not apply. "
                              "Ensure sufficient mesh resolution to capture velocity gradients.";
        validity.recommendations.push_back("Focus on overall mesh quality for laminar simulation");

    } else {
        validity.isValid = false;
        validity.explanation = "Unknown turbulence model. Cannot assess wall function validity.";
        validity.recommendations.push_back("Specify a known turbulence model (kEpsilon, kOmegaSST, laminar)");
    }

    return validity;
}

YPlusResult TurbulenceUtils::analyzeYPlus(double yPlus, const std::string& turbulenceModel) {
    YPlusResult result;
    result.yPlus = yPlus;
    result.region = determineWallRegion(yPlus);

    auto validity = checkWallFunctionValidity(yPlus, turbulenceModel);
    result.validForWallFunction = validity.isValid;

    // Generate recommendation based on analysis
    if (validity.isValid) {
        result.recommendation = "Current y+ is appropriate for " + turbulenceModel + " model.";
    } else {
        if (!validity.recommendations.empty()) {
            result.recommendation = validity.recommendations[0];
        } else {
            result.recommendation = "Consider adjusting near-wall mesh resolution.";
        }
    }

    return result;
}

std::string TurbulenceUtils::determineWallRegion(double yPlus) {
    if (yPlus < 5.0) {
        return "viscous_sublayer";
    } else if (yPlus < 30.0) {
        return "buffer_layer";
    } else if (yPlus < 300.0) {
        return "log_law";
    } else {
        return "outer";
    }
}

/*---------------------------------------------------------------------------*\
                        Turbulence Inlet Conditions
\*---------------------------------------------------------------------------*/

TurbulenceInletConditions TurbulenceUtils::calculateInletConditions(
    double velocity, double turbulentIntensity, double turbulentLengthScale,
    double kinematicViscosity) {

    TurbulenceInletConditions cond;

    // Store input parameters
    cond.velocity = velocity;
    cond.intensity = turbulentIntensity;
    cond.lengthScale = turbulentLengthScale;
    cond.viscosity = kinematicViscosity;

    // Turbulent kinetic energy: k = 1.5 * (U * I)²
    double Uprime = velocity * turbulentIntensity;
    cond.k = 1.5 * Uprime * Uprime;

    // Turbulent dissipation rate: ε = C_μ^0.75 * k^1.5 / L
    cond.epsilon = std::pow(Cmu(), 0.75) * std::pow(cond.k, 1.5) / turbulentLengthScale;

    // Specific dissipation rate: ω = ε / (C_μ * k)
    // Alternative: ω = k^0.5 / (C_μ^0.25 * L)
    cond.omega = std::sqrt(cond.k) / (std::pow(Cmu(), 0.25) * turbulentLengthScale);

    // Turbulent viscosity: ν_t = C_μ * k² / ε
    cond.nut = Cmu() * cond.k * cond.k / cond.epsilon;

    return cond;
}

double TurbulenceUtils::estimateTurbulenceIntensity(const std::string& flowType) {
    // Typical turbulence intensity values for different flow conditions
    // Reference: ANSYS Fluent Theory Guide, OpenFOAM User Guide

    if (flowType == "freestream" || flowType == "external_low") {
        return 0.01;  // 1% - Low turbulence, wind tunnel quality
    } else if (flowType == "external" || flowType == "external_medium") {
        return 0.05;  // 5% - Typical external flow
    } else if (flowType == "pipe" || flowType == "channel" || flowType == "internal") {
        return 0.05;  // 5% - Fully developed pipe/channel flow
    } else if (flowType == "pipe_inlet" || flowType == "developing") {
        return 0.10;  // 10% - Developing flow, after bends/contractions
    } else if (flowType == "high_turbulence" || flowType == "jet") {
        return 0.20;  // 20% - High turbulence, jets, mixing zones
    } else if (flowType == "combustion" || flowType == "reactive") {
        return 0.15;  // 15% - Combustion/reactive flows
    } else {
        return 0.05;  // Default 5%
    }
}

double TurbulenceUtils::estimateTurbulentLengthScale(double characteristicLength,
                                                     const std::string& flowType) {
    // Turbulent length scale estimation
    // General rule: L ≈ 0.07 * characteristic_length (hydraulic diameter)

    double factor = 0.07;  // Default factor

    if (flowType == "pipe" || flowType == "channel") {
        factor = 0.07;  // L ≈ 0.07 * D_h
    } else if (flowType == "external") {
        factor = 0.04;  // Smaller for external flows
    } else if (flowType == "boundary_layer") {
        factor = 0.40;  // L ≈ 0.4 * boundary layer thickness
    } else if (flowType == "jet") {
        factor = 0.10;  // For jets
    }

    return factor * characteristicLength;
}

/*---------------------------------------------------------------------------*\
                        Helper Functions
\*---------------------------------------------------------------------------*/

double TurbulenceUtils::calculateTurbulentViscosityRatio(double k, double epsilon,
                                                         double kinematicViscosity) {
    if (epsilon <= 0 || kinematicViscosity <= 0) {
        return 0.0;
    }

    // ν_t = C_μ * k² / ε
    double nut = Cmu() * k * k / epsilon;

    // Ratio: ν_t / ν
    return nut / kinematicViscosity;
}

double TurbulenceUtils::calculateTurbulentReynoldsNumber(double k, double kinematicViscosity) {
    if (kinematicViscosity <= 0 || k <= 0) {
        return 0.0;
    }

    // Re_t = k² / (ν * ε)
    // Using ε = C_μ * k^1.5 / L, and simplified:
    // Re_t ≈ k / (ν * ω) or k^2 / (ν * ε)
    // Here we use a simplified form: Re_t = k / ν (not fully accurate but useful estimate)
    return k / kinematicViscosity;
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //
