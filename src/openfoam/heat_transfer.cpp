/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Website:  https://openfoam.org
     \\/     M anipulation  |
\*---------------------------------------------------------------------------*/

#include "heat_transfer.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                    HeatTransferAnalyzer Implementation
\*---------------------------------------------------------------------------*/

HeatTransferAnalyzer::HeatTransferAnalyzer() {
    initializeMaterialDatabase();
}

HeatTransferResults HeatTransferAnalyzer::analyze(const HeatTransferInput& input) {
    HeatTransferResults results = {};
    results.success = false;

    if (!validateInput(input)) {
        results.errorMessage = "Invalid input parameters";
        return results;
    }

    try {
        // Route to application-specific analysis
        if (input.analysisType == "electronics_cooling") {
            results = analyzeElectronicsCooling(input);
        } else if (input.analysisType == "heat_exchanger") {
            results = analyzeHeatExchanger(input);
        } else if (input.analysisType == "building") {
            results = analyzeBuildingThermal(input);
        } else if (input.analysisType == "engine") {
            results = analyzeEngineCooling(input);
        } else {
            results.errorMessage = "Unknown analysis type: " + input.analysisType;
            return results;
        }

        results.success = true;
    } catch (const std::exception& e) {
        results.errorMessage = "Analysis failed: " + std::string(e.what());
    }

    return results;
}

HeatTransferResults HeatTransferAnalyzer::analyzeElectronicsCooling(
    const HeatTransferInput& input) {
    HeatTransferResults results = {};

    // Calculate key dimensionless numbers
    results.reynoldsNumber = calculateReynoldsNumber(input);
    results.prandtlNumber = calculatePrandtlNumber(input);
    results.pecletNumber = results.reynoldsNumber * results.prandtlNumber;

    // Electronics cooling specific calculations
    double chipArea = input.characteristicLength * input.characteristicLength;  // m²
    double heatFlux = input.heatGeneration / chipArea;                          // W/m²

    // Calculate heat transfer coefficient using forced convection correlations
    results.nusseltNumber =
        calculateNusseltNumber(results.reynoldsNumber, results.prandtlNumber, "flat_plate");

    // Get thermal conductivity of coolant
    double k_fluid = getThermalConductivity(input.coolantType);
    double h = calculateHeatTransferCoefficient(results.nusseltNumber, k_fluid,
                                                input.characteristicLength);

    // Calculate temperatures
    double deltaT = heatFlux / h;  // Temperature rise above coolant
    results.maxTemperature = input.inletTemperature + deltaT;
    results.minTemperature = input.inletTemperature;
    results.averageTemperature = (results.maxTemperature + results.minTemperature) / 2.0;
    results.maxSolidTemperature = results.maxTemperature + 10.0;  // Junction temp slightly higher

    // Thermal resistance calculation
    results.thermalResistance = deltaT / input.heatGeneration;  // K/W

    // Heat transfer rates
    results.totalHeatTransferRate = input.heatGeneration;
    results.convectiveHeatTransfer = input.heatGeneration * 0.8;   // 80% convective
    results.conductiveHeatTransfer = input.heatGeneration * 0.15;  // 15% conductive
    results.radiativeHeatTransfer = input.heatGeneration * 0.05;   // 5% radiative

    // Performance metrics
    results.thermalEfficiency =
        std::min(100.0, (input.maxAllowableTemp - results.maxTemperature) /
                            (input.maxAllowableTemp - input.ambientTemperature) * 100.0);
    results.coolingEffectiveness = (input.inletTemperature - input.ambientTemperature) /
                                   (results.maxTemperature - input.ambientTemperature);

    // Pressure drop calculation
    results.pressureDrop = calculatePressureDrop(input);
    results.pumpingPower = results.pressureDrop * (input.inletVelocity * chipArea);  // W

    // Design metrics
    results.thermalPerformanceIndex =
        input.heatGeneration / (results.maxTemperature - input.ambientTemperature);
    results.coolingCapacity =
        input.heatGeneration / (chipArea * input.characteristicLength);  // W/m³
    results.thermalConstraintsSatisfied = (results.maxSolidTemperature <= input.maxAllowableTemp);

    return results;
}

HeatTransferResults HeatTransferAnalyzer::analyzeHeatExchanger(const HeatTransferInput& input) {
    HeatTransferResults results = {};

    // Calculate dimensionless numbers
    results.reynoldsNumber = calculateReynoldsNumber(input);
    results.prandtlNumber = calculatePrandtlNumber(input);

    // Heat exchanger effectiveness-NTU method
    results.nusseltNumber =
        calculateNusseltNumber(results.reynoldsNumber, results.prandtlNumber, "tube");

    // Estimate effectiveness for counter-flow heat exchanger
    double NTU = 4.0;  // Typical value, would be calculated from geometry
    double Cr = 0.8;   // Heat capacity ratio
    double effectiveness = (1 - std::exp(-NTU * (1 - Cr))) / (1 - Cr * std::exp(-NTU * (1 - Cr)));

    results.coolingEffectiveness = effectiveness;

    // Temperature calculations
    double maxPossibleHeatTransfer = input.heatGeneration / effectiveness;
    results.totalHeatTransferRate = input.heatGeneration;
    results.convectiveHeatTransfer = results.totalHeatTransferRate;

    // Temperature estimates
    results.maxTemperature = input.inletTemperature + 50.0;  // Typical hot side temp
    results.minTemperature = input.inletTemperature - 20.0;  // Cold side outlet
    results.averageTemperature = (results.maxTemperature + results.minTemperature) / 2.0;
    results.maxSolidTemperature = results.maxTemperature + 5.0;

    // Performance metrics
    results.thermalEfficiency = effectiveness * 100.0;
    results.pressureDrop = calculatePressureDrop(input);
    results.thermalResistance =
        (results.maxTemperature - results.minTemperature) / results.totalHeatTransferRate;

    return results;
}

HeatTransferResults HeatTransferAnalyzer::analyzeBuildingThermal(const HeatTransferInput& input) {
    HeatTransferResults results = {};

    // Building thermal analysis - consider natural convection
    results.rayleighNumber = calculateRayleighNumber(input);

    // Natural convection Nusselt number
    if (results.rayleighNumber > 1e9) {
        results.nusseltNumber = 0.15 * std::pow(results.rayleighNumber, 0.33);  // Turbulent
    } else {
        results.nusseltNumber = 0.54 * std::pow(results.rayleighNumber, 0.25);  // Laminar
    }

    // Building envelope heat transfer
    double wallArea =
        6 * input.characteristicLength * input.characteristicLength;  // Simplified cube
    double U_value = 0.3;  // W/m²K - typical building envelope

    results.totalHeatTransferRate =
        U_value * wallArea * (input.ambientTemperature - input.inletTemperature);
    results.convectiveHeatTransfer = results.totalHeatTransferRate * 0.7;
    results.conductiveHeatTransfer = results.totalHeatTransferRate * 0.25;
    results.radiativeHeatTransfer = results.totalHeatTransferRate * 0.05;

    // Temperature calculations
    results.maxTemperature = input.inletTemperature + 5.0;  // Indoor temperature variation
    results.minTemperature = input.inletTemperature - 2.0;
    results.averageTemperature = input.inletTemperature;
    results.maxSolidTemperature = results.maxTemperature;

    // Building performance
    results.thermalEfficiency = 85.0;  // Typical well-insulated building
    results.thermalResistance = 1.0 / (U_value * wallArea);

    return results;
}

HeatTransferResults HeatTransferAnalyzer::analyzeEngineCooling(const HeatTransferInput& input) {
    HeatTransferResults results = {};

    // Engine cooling analysis
    results.reynoldsNumber = calculateReynoldsNumber(input);
    results.prandtlNumber = calculatePrandtlNumber(input);

    // High heat flux conditions in engines
    results.nusseltNumber =
        calculateNusseltNumber(results.reynoldsNumber, results.prandtlNumber, "cylinder");

    // Engine specific heat transfer
    double engineHeatRejection = input.heatGeneration;  // W
    results.totalHeatTransferRate = engineHeatRejection;
    results.convectiveHeatTransfer = engineHeatRejection * 0.85;  // Mostly convection to coolant
    results.conductiveHeatTransfer = engineHeatRejection * 0.10;
    results.radiativeHeatTransfer = engineHeatRejection * 0.05;

    // Temperature calculations - engines run hot
    results.maxTemperature = input.inletTemperature + 80.0;  // Typical engine temp rise
    results.minTemperature = input.inletTemperature + 20.0;  // Coolant outlet temp
    results.averageTemperature = (results.maxTemperature + results.minTemperature) / 2.0;
    results.maxSolidTemperature = results.maxTemperature + 20.0;  // Cylinder head temp

    // Engine cooling performance
    results.thermalEfficiency = 75.0;                           // Typical engine cooling efficiency
    results.pressureDrop = calculatePressureDrop(input) * 2.0;  // Higher pressure drop
    results.thermalResistance =
        (results.maxTemperature - input.inletTemperature) / engineHeatRejection;

    return results;
}

double HeatTransferAnalyzer::calculateReynoldsNumber(const HeatTransferInput& input) const {
    double rho = getDensity(input.coolantType);
    double mu = 1.8e-5;  // Dynamic viscosity, simplified

    return rho * input.inletVelocity * input.characteristicLength / mu;
}

double HeatTransferAnalyzer::calculatePrandtlNumber(const HeatTransferInput& input) const {
    // Typical Prandtl numbers
    if (input.coolantType == "air")
        return 0.7;
    if (input.coolantType == "water")
        return 7.0;
    if (input.coolantType == "oil")
        return 100.0;
    return 0.7;  // Default
}

double HeatTransferAnalyzer::calculateRayleighNumber(const HeatTransferInput& input) const {
    double g = 9.81;                               // m/s²
    double beta = 1.0 / input.ambientTemperature;  // Thermal expansion coefficient
    double deltaT = std::abs(input.ambientTemperature - input.inletTemperature);
    double nu = 1.5e-5;     // Kinematic viscosity
    double alpha = 2.2e-5;  // Thermal diffusivity

    return g * beta * deltaT * std::pow(input.characteristicLength, 3) / (nu * alpha);
}

double HeatTransferAnalyzer::calculateNusseltNumber(double Re, double Pr,
                                                    const std::string& geometry) const {
    if (geometry == "flat_plate") {
        // Forced convection over flat plate
        if (Re < 5e5) {
            return 0.332 * std::pow(Re, 0.5) * std::pow(Pr, 0.33);  // Laminar
        } else {
            return 0.0296 * std::pow(Re, 0.8) * std::pow(Pr, 0.33);  // Turbulent
        }
    } else if (geometry == "tube") {
        // Flow in tubes
        if (Re < 2300) {
            return 3.66;  // Laminar, constant heat flux
        } else {
            return 0.023 * std::pow(Re, 0.8) * std::pow(Pr, 0.4);  // Dittus-Boelter
        }
    } else if (geometry == "cylinder") {
        // Flow over cylinder
        return 0.3 + (0.62 * std::pow(Re, 0.5) * std::pow(Pr, 0.33)) /
                         std::pow(1 + std::pow(0.4 / Pr, 0.67), 0.25);
    }

    return 10.0;  // Default reasonable value
}

double HeatTransferAnalyzer::calculateHeatTransferCoefficient(double Nu, double k, double L) const {
    return Nu * k / L;  // W/m²K
}

double HeatTransferAnalyzer::calculatePressureDrop(const HeatTransferInput& input) const {
    double rho = getDensity(input.coolantType);
    double Re = calculateReynoldsNumber(input);

    // Friction factor for smooth tubes
    double f;
    if (Re < 2300) {
        f = 64.0 / Re;  // Laminar
    } else {
        f = 0.316 / std::pow(Re, 0.25);  // Turbulent, smooth tubes
    }

    // Pressure drop in a channel/tube
    double L = input.characteristicLength * 10;  // Assume length is 10x characteristic length
    return f * (L / input.characteristicLength) * 0.5 * rho * input.inletVelocity *
           input.inletVelocity;
}

void HeatTransferAnalyzer::initializeMaterialDatabase() {
    // Thermal conductivity [W/m·K]
    materialDatabase_["thermal_conductivity"] = {
        {"air", 0.026},    {"water", 0.6},  {"oil", 0.15},      {"aluminum", 237.0},
        {"copper", 401.0}, {"steel", 50.0}, {"silicon", 148.0}, {"plastic", 0.2},
        {"concrete", 1.7}, {"glass", 1.4}};

    // Density [kg/m³]
    materialDatabase_["density"] = {{"air", 1.225},       {"water", 1000.0},   {"oil", 850.0},
                                    {"aluminum", 2700.0}, {"copper", 8960.0},  {"steel", 7850.0},
                                    {"silicon", 2330.0},  {"plastic", 1200.0}, {"concrete", 2400.0},
                                    {"glass", 2500.0}};

    // Specific heat [J/kg·K]
    materialDatabase_["specific_heat"] = {
        {"air", 1005.0},     {"water", 4180.0}, {"oil", 2000.0},    {"aluminum", 900.0},
        {"copper", 385.0},   {"steel", 450.0},  {"silicon", 700.0}, {"plastic", 1500.0},
        {"concrete", 880.0}, {"glass", 840.0}};
}

double HeatTransferAnalyzer::getThermalConductivity(const std::string& material) const {
    auto it = materialDatabase_.find("thermal_conductivity");
    if (it != materialDatabase_.end()) {
        auto mat_it = it->second.find(material);
        if (mat_it != it->second.end()) {
            return mat_it->second;
        }
    }
    return 0.026;  // Default to air
}

double HeatTransferAnalyzer::getDensity(const std::string& material) const {
    auto it = materialDatabase_.find("density");
    if (it != materialDatabase_.end()) {
        auto mat_it = it->second.find(material);
        if (mat_it != it->second.end()) {
            return mat_it->second;
        }
    }
    return 1.225;  // Default to air
}

double HeatTransferAnalyzer::getSpecificHeat(const std::string& material) const {
    auto it = materialDatabase_.find("specific_heat");
    if (it != materialDatabase_.end()) {
        auto mat_it = it->second.find(material);
        if (mat_it != it->second.end()) {
            return mat_it->second;
        }
    }
    return 1005.0;  // Default to air
}

void HeatTransferAnalyzer::populateMaterialProperties(HeatTransferInput& input) const {
    // Populate thermal conductivity
    if (input.thermalConductivity.empty()) {
        input.thermalConductivity[input.coolantType] = getThermalConductivity(input.coolantType);
        input.thermalConductivity["aluminum"] = getThermalConductivity("aluminum");
        input.thermalConductivity["copper"] = getThermalConductivity("copper");
        input.thermalConductivity["silicon"] = getThermalConductivity("silicon");
    }

    // Populate density
    if (input.density.empty()) {
        input.density[input.coolantType] = getDensity(input.coolantType);
        input.density["aluminum"] = getDensity("aluminum");
        input.density["copper"] = getDensity("copper");
        input.density["silicon"] = getDensity("silicon");
    }

    // Populate specific heat
    if (input.specificHeat.empty()) {
        input.specificHeat[input.coolantType] = getSpecificHeat(input.coolantType);
        input.specificHeat["aluminum"] = getSpecificHeat("aluminum");
        input.specificHeat["copper"] = getSpecificHeat("copper");
        input.specificHeat["silicon"] = getSpecificHeat("silicon");
    }
}

bool HeatTransferAnalyzer::validateInput(const HeatTransferInput& input) const {
    return input.characteristicLength > 0 && input.heatGeneration > 0 &&
           input.ambientTemperature > 0 && input.inletTemperature > 0 &&
           input.maxAllowableTemp > input.ambientTemperature;
}

json HeatTransferAnalyzer::toJson(const HeatTransferResults& results) const {
    json j;
    j["maxTemperature"] = results.maxTemperature;
    j["minTemperature"] = results.minTemperature;
    j["averageTemperature"] = results.averageTemperature;
    j["maxSolidTemperature"] = results.maxSolidTemperature;
    j["thermalResistance"] = results.thermalResistance;
    j["totalHeatTransferRate"] = results.totalHeatTransferRate;
    j["convectiveHeatTransfer"] = results.convectiveHeatTransfer;
    j["conductiveHeatTransfer"] = results.conductiveHeatTransfer;
    j["radiativeHeatTransfer"] = results.radiativeHeatTransfer;
    j["thermalEfficiency"] = results.thermalEfficiency;
    j["coolingEffectiveness"] = results.coolingEffectiveness;
    j["pressureDrop"] = results.pressureDrop;
    j["pumpingPower"] = results.pumpingPower;
    j["reynoldsNumber"] = results.reynoldsNumber;
    j["prandtlNumber"] = results.prandtlNumber;
    j["nusseltNumber"] = results.nusseltNumber;
    j["rayleighNumber"] = results.rayleighNumber;
    j["pecletNumber"] = results.pecletNumber;
    j["thermalPerformanceIndex"] = results.thermalPerformanceIndex;
    j["coolingCapacity"] = results.coolingCapacity;
    j["thermalConstraintsSatisfied"] = results.thermalConstraintsSatisfied;
    j["caseId"] = results.caseId;
    j["success"] = results.success;
    j["errorMessage"] = results.errorMessage;

    return j;
}

}  // namespace MCP
}  // namespace Foam