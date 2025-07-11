/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Multiphase Flow Analysis
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Multiphase flow analysis implementation using OpenFOAM interFoam

\*---------------------------------------------------------------------------*/

#include "multiphase_flow.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const MultiphaseFlowInput& input) {
    j = json{{"analysisType", input.analysisType},
             {"phase1", input.phase1},
             {"phase2", input.phase2},
             {"characteristicLength", input.characteristicLength},
             {"height", input.height},
             {"width", input.width},
             {"depth", input.depth},
             {"density1", input.density1},
             {"density2", input.density2},
             {"viscosity1", input.viscosity1},
             {"viscosity2", input.viscosity2},
             {"surfaceTension", input.surfaceTension},
             {"gravity", input.gravity},
             {"initialVelocity", input.initialVelocity},
             {"contactAngle", input.contactAngle},
             {"steadyState", input.steadyState},
             {"timeStep", input.timeStep},
             {"endTime", input.endTime},
             {"writeInterval", input.writeInterval},
             {"courantNumber", input.courantNumber},
             {"turbulenceModel", input.turbulenceModel},
             {"compressible", input.compressible}};
}

void from_json(const json& j, MultiphaseFlowInput& input) {
    j.at("analysisType").get_to(input.analysisType);
    j.at("phase1").get_to(input.phase1);
    j.at("phase2").get_to(input.phase2);
    j.at("characteristicLength").get_to(input.characteristicLength);
    j.at("height").get_to(input.height);
    j.at("width").get_to(input.width);
    j.at("depth").get_to(input.depth);
    j.at("density1").get_to(input.density1);
    j.at("density2").get_to(input.density2);
    j.at("viscosity1").get_to(input.viscosity1);
    j.at("viscosity2").get_to(input.viscosity2);
    j.at("surfaceTension").get_to(input.surfaceTension);

    if (j.contains("gravity")) j.at("gravity").get_to(input.gravity);
    if (j.contains("initialVelocity")) j.at("initialVelocity").get_to(input.initialVelocity);
    if (j.contains("contactAngle")) j.at("contactAngle").get_to(input.contactAngle);
    if (j.contains("steadyState")) j.at("steadyState").get_to(input.steadyState);
    if (j.contains("timeStep")) j.at("timeStep").get_to(input.timeStep);
    if (j.contains("endTime")) j.at("endTime").get_to(input.endTime);
    if (j.contains("writeInterval")) j.at("writeInterval").get_to(input.writeInterval);
    if (j.contains("courantNumber")) j.at("courantNumber").get_to(input.courantNumber);
    if (j.contains("turbulenceModel")) j.at("turbulenceModel").get_to(input.turbulenceModel);
    if (j.contains("compressible")) j.at("compressible").get_to(input.compressible);
}

void to_json(json& j, const MultiphaseFlowResults& results) {
    j = json{{"reynoldsNumber", results.reynoldsNumber},
             {"webberNumber", results.webberNumber},
             {"capillaryNumber", results.capillaryNumber},
             {"bondNumber", results.bondNumber},
             {"froudeNumber", results.froudeNumber},
             {"flowRegime", results.flowRegime},
             {"mixingCharacteristics", results.mixingCharacteristics},
             {"phase1Volume", results.phase1Volume},
             {"phase2Volume", results.phase2Volume},
             {"interfaceArea", results.interfaceArea},
             {"maxVelocity", results.maxVelocity},
             {"kineticEnergy", results.kineticEnergy},
             {"potentialEnergy", results.potentialEnergy},
             {"surfaceEnergy", results.surfaceEnergy},
             {"maxPressure", results.maxPressure},
             {"minPressure", results.minPressure},
             {"pressureDrop", results.pressureDrop},
             {"timeHistory", results.timeHistory},
             {"phase1VolumeHistory", results.phase1VolumeHistory},
             {"kineticEnergyHistory", results.kineticEnergyHistory},
             {"maxVelocityHistory", results.maxVelocityHistory},
             {"caseId", results.caseId},
             {"success", results.success}};

    if (!results.errorMessage.empty()) {
        j["errorMessage"] = results.errorMessage;
    }
}

void from_json(const json& j, MultiphaseFlowResults& results) {
    j.at("reynoldsNumber").get_to(results.reynoldsNumber);
    j.at("webberNumber").get_to(results.webberNumber);
    j.at("capillaryNumber").get_to(results.capillaryNumber);
    j.at("bondNumber").get_to(results.bondNumber);
    j.at("froudeNumber").get_to(results.froudeNumber);
    j.at("flowRegime").get_to(results.flowRegime);
    j.at("mixingCharacteristics").get_to(results.mixingCharacteristics);
    j.at("phase1Volume").get_to(results.phase1Volume);
    j.at("phase2Volume").get_to(results.phase2Volume);
    j.at("interfaceArea").get_to(results.interfaceArea);
    j.at("maxVelocity").get_to(results.maxVelocity);
    j.at("kineticEnergy").get_to(results.kineticEnergy);
    j.at("potentialEnergy").get_to(results.potentialEnergy);
    j.at("surfaceEnergy").get_to(results.surfaceEnergy);
    j.at("maxPressure").get_to(results.maxPressure);
    j.at("minPressure").get_to(results.minPressure);
    j.at("pressureDrop").get_to(results.pressureDrop);
    j.at("timeHistory").get_to(results.timeHistory);
    j.at("phase1VolumeHistory").get_to(results.phase1VolumeHistory);
    j.at("kineticEnergyHistory").get_to(results.kineticEnergyHistory);
    j.at("maxVelocityHistory").get_to(results.maxVelocityHistory);
    j.at("caseId").get_to(results.caseId);
    j.at("success").get_to(results.success);

    if (j.contains("errorMessage")) j.at("errorMessage").get_to(results.errorMessage);
}

/*---------------------------------------------------------------------------*\
                MultiphaseFlowInput Physics Methods
\*---------------------------------------------------------------------------*/

double MultiphaseFlowInput::getReynoldsNumber() const {
    // Use characteristic velocity from gravity and height
    double characteristicVelocity = std::sqrt(gravity * height);
    return density1 * characteristicVelocity * characteristicLength / viscosity1;
}

double MultiphaseFlowInput::getWebberNumber() const {
    double characteristicVelocity = std::sqrt(gravity * height);
    return density1 * characteristicVelocity * characteristicVelocity * characteristicLength /
           surfaceTension;
}

double MultiphaseFlowInput::getCapillaryNumber() const {
    double characteristicVelocity = std::sqrt(gravity * height);
    return viscosity1 * characteristicVelocity / surfaceTension;
}

double MultiphaseFlowInput::getBondNumber() const {
    return (density1 - density2) * gravity * characteristicLength * characteristicLength /
           surfaceTension;
}

double MultiphaseFlowInput::getFroudeNumber() const {
    double characteristicVelocity = std::sqrt(gravity * height);
    return characteristicVelocity / std::sqrt(gravity * characteristicLength);
}

/*---------------------------------------------------------------------------*\
                    MultiphaseFlowAnalyzer Implementation
\*---------------------------------------------------------------------------*/

MultiphaseFlowAnalyzer::MultiphaseFlowAnalyzer() : caseManager_(std::make_unique<CaseManager>()) {}

MultiphaseFlowAnalyzer::MultiphaseFlowAnalyzer(std::unique_ptr<CaseManager> caseManager)
    : caseManager_(std::move(caseManager)) {}

MultiphaseFlowResults MultiphaseFlowAnalyzer::analyze(const MultiphaseFlowInput& input) {
    MultiphaseFlowResults results = {};
    results.success = false;

    if (!validateInput(input)) {
        results.errorMessage = "Invalid input parameters";
        return results;
    }

    // Calculate theoretical results first
    results = calculateTheoreticalResults(input);

    // Try to run OpenFOAM simulation
    try {
        std::string caseId = setupInterFoamCase(input);
        results.caseId = caseId;

        if (caseManager_->runCase(caseId)) {
            parseInterFoamResults(caseId, results);
            results.success = true;
        } else {
            results.errorMessage = "OpenFOAM interFoam simulation failed";
            // Keep theoretical results
        }
    } catch (const std::exception& e) {
        results.errorMessage = "Error running interFoam: " + std::string(e.what());
        // Keep theoretical results
    }

    return results;
}

bool MultiphaseFlowAnalyzer::validateInput(const MultiphaseFlowInput& input) const {
    if (input.characteristicLength <= 0.0 || input.height <= 0.0 || input.width <= 0.0 ||
        input.depth <= 0.0) {
        return false;
    }

    if (input.density1 <= 0.0 || input.density2 <= 0.0) {
        return false;
    }

    if (input.viscosity1 <= 0.0 || input.viscosity2 <= 0.0) {
        return false;
    }

    if (input.surfaceTension <= 0.0) {
        return false;
    }

    if (input.timeStep <= 0.0 || input.endTime <= input.timeStep) {
        return false;
    }

    return true;
}

MultiphaseFlowResults MultiphaseFlowAnalyzer::calculateTheoreticalResults(
    const MultiphaseFlowInput& input) const {
    MultiphaseFlowResults results;

    // Calculate dimensionless numbers
    calculateDimensionlessNumbers(input, results);

    // Determine flow regime
    results.flowRegime = determineFlowRegime(input, results);
    results.mixingCharacteristics = analyzeMixingCharacteristics(input, results);

    // Estimate phase volumes
    if (input.isDamBreak()) {
        results.phase1Volume = input.height * input.width * input.depth;
        results.phase2Volume =
            (input.characteristicLength - input.height) * input.width * input.depth;
    } else {
        results.phase1Volume = input.characteristicLength * input.width * input.depth * 0.5;
        results.phase2Volume = input.characteristicLength * input.width * input.depth * 0.5;
    }

    // Estimate interface area
    results.interfaceArea = input.width * input.depth;  // Initial interface

    // Estimate characteristic velocity and energies
    double characteristicVelocity = std::sqrt(input.gravity * input.height);
    results.maxVelocity = characteristicVelocity;

    double totalMass1 = input.density1 * results.phase1Volume;
    double totalMass2 = input.density2 * results.phase2Volume;
    results.kineticEnergy =
        0.5 * (totalMass1 + totalMass2) * characteristicVelocity * characteristicVelocity;

    // Potential energy
    results.potentialEnergy = totalMass1 * input.gravity * input.height * 0.5;

    // Surface energy
    results.surfaceEnergy = input.surfaceTension * results.interfaceArea;

    // Pressure estimates
    results.maxPressure = input.density1 * input.gravity * input.height;
    results.minPressure = 0.0;  // Atmospheric reference
    results.pressureDrop = results.maxPressure;

    return results;
}

void MultiphaseFlowAnalyzer::calculateDimensionlessNumbers(const MultiphaseFlowInput& input,
                                                           MultiphaseFlowResults& results) const {
    results.reynoldsNumber = input.getReynoldsNumber();
    results.webberNumber = input.getWebberNumber();
    results.capillaryNumber = input.getCapillaryNumber();
    results.bondNumber = input.getBondNumber();
    results.froudeNumber = input.getFroudeNumber();
}

std::string MultiphaseFlowAnalyzer::determineFlowRegime(
    const MultiphaseFlowInput& input, const MultiphaseFlowResults& results) const {
    // Classify based on dimensionless numbers
    if (results.bondNumber > 1.0) {
        return "gravity-dominated";
    } else if (results.webberNumber > 1.0) {
        return "inertia-dominated";
    } else {
        return "surface-tension-dominated";
    }
}

std::string MultiphaseFlowAnalyzer::analyzeMixingCharacteristics(
    const MultiphaseFlowInput& input, const MultiphaseFlowResults& results) const {
    if (results.reynoldsNumber > 4000) {
        return "turbulent-mixing";
    } else if (results.reynoldsNumber > 100) {
        return "transitional-mixing";
    } else {
        return "laminar-stratified";
    }
}

std::string MultiphaseFlowAnalyzer::setupInterFoamCase(const MultiphaseFlowInput& input) {
    CaseParameters params;
    params.caseName = "multiphase_flow_analysis";
    params.solver = "interFoam";
    params.caseType = input.analysisType;
    params.endTime = input.endTime;
    params.deltaTime = input.timeStep;
    params.writeInterval = input.writeInterval;

    // Set boundary conditions
    params.boundaryConditions["inlet"] = "zeroGradient";
    params.boundaryConditions["outlet"] = "zeroGradient";
    params.boundaryConditions["walls"] = "noSlip";
    params.boundaryConditions["atmosphere"] = "totalPressure";

    // Set physical properties
    params.physicalProperties["rho1"] = std::to_string(input.density1);
    params.physicalProperties["rho2"] = std::to_string(input.density2);
    params.physicalProperties["nu1"] = std::to_string(input.viscosity1 / input.density1);
    params.physicalProperties["nu2"] = std::to_string(input.viscosity2 / input.density2);
    params.physicalProperties["sigma"] = std::to_string(input.surfaceTension);

    // Set numerical schemes
    params.numericalSchemes["ddtSchemes"] = "Euler";
    params.numericalSchemes["gradSchemes"] = "Gauss linear";
    params.numericalSchemes["divSchemes"] = "Gauss vanLeer";
    params.numericalSchemes["laplacianSchemes"] = "Gauss linear corrected";

    return caseManager_->createCase(params);
}

void MultiphaseFlowAnalyzer::parseInterFoamResults(const std::string& caseId,
                                                   MultiphaseFlowResults& results) const {
    CaseResult caseResult = caseManager_->getCaseResult(caseId);

    if (caseResult.isSuccess()) {
        // Extract results from OpenFOAM output
        if (caseResult.results.find("maxVelocity") != caseResult.results.end()) {
            results.maxVelocity = caseResult.results.at("maxVelocity");
        }

        if (caseResult.results.find("phase1Volume") != caseResult.results.end()) {
            results.phase1Volume = caseResult.results.at("phase1Volume");
        }

        if (caseResult.results.find("maxPressure") != caseResult.results.end()) {
            results.maxPressure = caseResult.results.at("maxPressure");
        }

        if (caseResult.results.find("minPressure") != caseResult.results.end()) {
            results.minPressure = caseResult.results.at("minPressure");
        }

        // Calculate derived quantities
        results.pressureDrop = results.maxPressure - results.minPressure;
        results.phase2Volume = 1.0 - results.phase1Volume;  // Volume fraction based
    }
}

json MultiphaseFlowAnalyzer::toJson(const MultiphaseFlowResults& results) const {
    json j;
    to_json(j, results);
    return j;
}

MultiphaseFlowResults MultiphaseFlowAnalyzer::fromJson(const json& j) const {
    MultiphaseFlowResults results;
    from_json(j, results);
    return results;
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //