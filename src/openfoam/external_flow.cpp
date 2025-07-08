/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | External Flow Analysis
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    External flow analysis tool implementation

\*---------------------------------------------------------------------------*/

#include "external_flow.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const ExternalFlowInput& input) {
    j = json{{"vehicleType", input.vehicleType},
             {"velocity", input.velocity},
             {"characteristicLength", input.characteristicLength},
             {"frontalArea", input.frontalArea},
             {"altitude", input.altitude},
             {"fluidType", input.fluidType},
             {"objective", input.objective},
             {"density", input.density},
             {"viscosity", input.viscosity},
             {"temperature", input.temperature}};
}

void from_json(const json& j, ExternalFlowInput& input) {
    j.at("vehicleType").get_to(input.vehicleType);
    j.at("velocity").get_to(input.velocity);
    j.at("characteristicLength").get_to(input.characteristicLength);

    if (j.contains("frontalArea")) j.at("frontalArea").get_to(input.frontalArea);
    if (j.contains("altitude")) j.at("altitude").get_to(input.altitude);
    if (j.contains("fluidType")) j.at("fluidType").get_to(input.fluidType);
    if (j.contains("objective")) j.at("objective").get_to(input.objective);
    if (j.contains("density")) j.at("density").get_to(input.density);
    if (j.contains("viscosity")) j.at("viscosity").get_to(input.viscosity);
    if (j.contains("temperature")) j.at("temperature").get_to(input.temperature);
}

void to_json(json& j, const ExternalFlowResults& results) {
    j = json{{"reynoldsNumber", results.reynoldsNumber},
             {"machNumber", results.machNumber},
             {"dragCoefficient", results.dragCoefficient},
             {"liftCoefficient", results.liftCoefficient},
             {"dragForce", results.dragForce},
             {"liftForce", results.liftForce},
             {"pressureCoefficient", results.pressureCoefficient},
             {"skinFrictionCoefficient", results.skinFrictionCoefficient},
             {"flowRegime", results.flowRegime},
             {"compressibilityRegime", results.compressibilityRegime},
             {"caseId", results.caseId},
             {"success", results.success}};

    if (!results.errorMessage.empty()) {
        j["errorMessage"] = results.errorMessage;
    }
}

void from_json(const json& j, ExternalFlowResults& results) {
    j.at("reynoldsNumber").get_to(results.reynoldsNumber);
    j.at("machNumber").get_to(results.machNumber);
    j.at("dragCoefficient").get_to(results.dragCoefficient);
    j.at("liftCoefficient").get_to(results.liftCoefficient);
    j.at("dragForce").get_to(results.dragForce);
    j.at("liftForce").get_to(results.liftForce);
    j.at("pressureCoefficient").get_to(results.pressureCoefficient);
    j.at("skinFrictionCoefficient").get_to(results.skinFrictionCoefficient);
    j.at("flowRegime").get_to(results.flowRegime);
    j.at("compressibilityRegime").get_to(results.compressibilityRegime);
    j.at("caseId").get_to(results.caseId);
    j.at("success").get_to(results.success);

    if (j.contains("errorMessage")) j.at("errorMessage").get_to(results.errorMessage);
}

/*---------------------------------------------------------------------------*\
                        ExternalFlowAnalyzer Implementation
\*---------------------------------------------------------------------------*/

ExternalFlowAnalyzer::ExternalFlowAnalyzer() : caseManager_(std::make_unique<CaseManager>()) {}

ExternalFlowAnalyzer::ExternalFlowAnalyzer(std::unique_ptr<CaseManager> caseManager)
    : caseManager_(std::move(caseManager)) {}

void ExternalFlowAnalyzer::validateInput(const ExternalFlowInput& input) {
    if (input.velocity <= 0) {
        throw std::invalid_argument("Velocity must be positive");
    }

    if (input.characteristicLength <= 0) {
        throw std::invalid_argument("Characteristic length must be positive");
    }

    if (input.frontalArea <= 0) {
        throw std::invalid_argument("Frontal area must be positive");
    }

    if (input.density <= 0) {
        throw std::invalid_argument("Density must be positive");
    }

    if (input.viscosity <= 0) {
        throw std::invalid_argument("Viscosity must be positive");
    }

    if (input.velocity > 340) {
        throw std::invalid_argument(
            "Velocity too high (>340 m/s), supersonic flow not yet supported");
    }

    if (input.characteristicLength > 100) {
        throw std::invalid_argument(
            "Characteristic length too large (>100 m), please verify input");
    }

    if (input.vehicleType != "car" && input.vehicleType != "aircraft" &&
        input.vehicleType != "building" && input.vehicleType != "cylinder") {
        throw std::invalid_argument(
            "Vehicle type must be 'car', 'aircraft', 'building', or 'cylinder'");
    }
}

CaseParameters ExternalFlowAnalyzer::createCaseParameters(const ExternalFlowInput& input) {
    CaseParameters params;
    params.caseName = "external_flow_analysis";

    // Always use incompressible solver for subsonic external flow
    params.solver = "foamRun -solver incompressibleFluid";
    params.caseType = "steady";
    params.endTime = 2000;  // More iterations for external flow convergence
    params.deltaTime = 1;
    params.writeInterval = 100;

    // Set up external flow boundary conditions
    params.boundaryConditions["inlet"] = "fixedValue";
    params.boundaryConditions["outlet"] = "zeroGradient";
    params.boundaryConditions["walls"] = "noSlip";
    params.boundaryConditions["symmetry"] = "symmetryPlane";
    params.boundaryConditions["farfield"] = "freestream";

    // Store flow parameters
    params.physicalProperties["nu"] = std::to_string(input.viscosity);
    params.physicalProperties["velocity"] = std::to_string(input.velocity);
    params.physicalProperties["characteristicLength"] = std::to_string(input.characteristicLength);
    params.physicalProperties["frontalArea"] = std::to_string(input.frontalArea);
    params.physicalProperties["density"] = std::to_string(input.density);
    params.physicalProperties["vehicleType"] = input.vehicleType;

    return params;
}

ExternalFlowResults ExternalFlowAnalyzer::analyze(const ExternalFlowInput& input) {
    ExternalFlowResults results;

    try {
        validateInput(input);

        results.reynoldsNumber = input.getReynoldsNumber();
        results.machNumber = input.getMachNumber();
        results.flowRegime = determineFlowRegime(input);
        results.compressibilityRegime = determineCompressibilityRegime(input);

        // Calculate theoretical values for validation/fallback
        results.dragCoefficient = calculateTheoreticalDragCoefficient(input);
        results.skinFrictionCoefficient = calculateSkinFrictionCoefficient(input);

        // Calculate forces
        double dynamicPressure = 0.5 * input.density * input.velocity * input.velocity;
        results.dragForce = results.dragCoefficient * dynamicPressure * input.frontalArea;
        results.liftForce = results.liftCoefficient * dynamicPressure * input.frontalArea;

        CaseParameters params = createCaseParameters(input);

        std::string caseId = caseManager_->createCase(params);
        results.caseId = caseId;

        bool runSuccess = caseManager_->runCase(caseId);

        if (runSuccess) {
            results = processResults(caseId, input);
            results.success = true;
        } else {
            CaseResult caseResult = caseManager_->getCaseResult(caseId);
            results.success = false;
            results.errorMessage = "OpenFOAM simulation failed: " + caseResult.errorOutput;

            // Use theoretical calculations as fallback
            results.dragCoefficient = calculateTheoreticalDragCoefficient(input);
            results.skinFrictionCoefficient = calculateSkinFrictionCoefficient(input);
            results.dragForce = results.dragCoefficient * dynamicPressure * input.frontalArea;
        }
    } catch (const std::exception& e) {
        results.success = false;
        results.errorMessage = e.what();
    }

    return results;
}

ExternalFlowResults ExternalFlowAnalyzer::processResults(const std::string& caseId,
                                                         const ExternalFlowInput& input) {
    ExternalFlowResults results;

    CaseResult caseResult = caseManager_->getCaseResult(caseId);

    if (!caseResult.isSuccess()) {
        results.success = false;
        results.errorMessage = "Case execution failed";
        return results;
    }

    results.caseId = caseId;
    results.reynoldsNumber = input.getReynoldsNumber();
    results.machNumber = input.getMachNumber();
    results.flowRegime = determineFlowRegime(input);
    results.compressibilityRegime = determineCompressibilityRegime(input);

    // Use theoretical calculations (OpenFOAM would provide actual force coefficients)
    results.dragCoefficient = calculateTheoreticalDragCoefficient(input);
    results.skinFrictionCoefficient = calculateSkinFrictionCoefficient(input);

    // Calculate forces
    double dynamicPressure = 0.5 * input.density * input.velocity * input.velocity;
    results.dragForce = results.dragCoefficient * dynamicPressure * input.frontalArea;
    results.liftForce = results.liftCoefficient * dynamicPressure * input.frontalArea;

    results.success = true;

    return results;
}

double ExternalFlowAnalyzer::calculateTheoreticalDragCoefficient(
    const ExternalFlowInput& input) const {
    double Re = input.getReynoldsNumber();

    if (input.vehicleType == "car") {
        return getCarDragCoefficient(Re, input.characteristicLength / std::sqrt(input.frontalArea));
    } else if (input.vehicleType == "aircraft") {
        return getAircraftDragCoefficient(Re, input.getMachNumber(), "NACA0012");
    } else if (input.vehicleType == "building") {
        return getBuildingDragCoefficient(
            Re, input.characteristicLength / std::sqrt(input.frontalArea));
    } else if (input.vehicleType == "cylinder") {
        return getCylinderDragCoefficient(Re);
    }

    return 1.0;  // Default fallback
}

double ExternalFlowAnalyzer::calculateSkinFrictionCoefficient(
    const ExternalFlowInput& input) const {
    double Re = input.getReynoldsNumber();

    if (input.isLaminar()) {
        return 1.328 / std::sqrt(Re);  // Blasius flat plate
    } else {
        return 0.074 / std::pow(Re, 0.2);  // Turbulent flat plate
    }
}

double ExternalFlowAnalyzer::getCarDragCoefficient(double Re, double aspect_ratio) const {
    // Typical car drag coefficients based on shape
    double base_cd = 0.35;  // Modern car baseline

    // Reynolds number correction (minimal for cars at highway speeds)
    if (Re < 1e6) {
        base_cd *= 1.1;  // Slightly higher at lower Re
    }

    // Aspect ratio effect
    if (aspect_ratio < 1.5) {
        base_cd *= 1.2;  // Stubby cars have higher drag
    } else if (aspect_ratio > 3.0) {
        base_cd *= 0.9;  // Streamlined cars have lower drag
    }

    return base_cd;
}

double ExternalFlowAnalyzer::getAircraftDragCoefficient(double Re, double Ma,
                                                        const std::string& airfoil_type) const {
    // Airfoil drag coefficient estimation
    double cd_friction = calculateSkinFrictionCoefficient({});
    double cd_pressure = 0.01;  // Base pressure drag for airfoil

    // Mach number effects
    if (Ma > 0.3) {
        cd_pressure *= (1.0 + 2.0 * Ma * Ma);  // Compressibility effects
    }

    return cd_friction + cd_pressure;
}

double ExternalFlowAnalyzer::getBuildingDragCoefficient(double Re, double height_to_width) const {
    // Building drag coefficients
    double base_cd = 1.3;  // Typical building

    if (height_to_width > 3.0) {
        base_cd = 1.1;  // Tall slender buildings
    } else if (height_to_width < 0.5) {
        base_cd = 1.5;  // Wide low buildings
    }

    return base_cd;
}

double ExternalFlowAnalyzer::getCylinderDragCoefficient(double Re) const {
    // Cylinder drag coefficient vs Reynolds number
    if (Re < 1) {
        return 24.0 / Re;  // Stokes flow
    } else if (Re < 1000) {
        return 24.0 / Re + 6.0 / (1.0 + std::sqrt(Re)) + 0.4;  // Intermediate
    } else if (Re < 200000) {
        return 1.2;  // Subcritical
    } else if (Re < 500000) {
        return 0.3;  // Critical (drag crisis)
    } else {
        return 0.7;  // Supercritical
    }
}

std::string ExternalFlowAnalyzer::determineFlowRegime(const ExternalFlowInput& input) const {
    if (input.isLaminar()) {
        return "laminar";
    } else if (input.isTurbulent()) {
        return "turbulent";
    } else {
        return "transitional";
    }
}

std::string ExternalFlowAnalyzer::determineCompressibilityRegime(
    const ExternalFlowInput& input) const {
    double Ma = input.getMachNumber();

    if (Ma < 0.3) {
        return "incompressible";
    } else if (Ma < 0.8) {
        return "subsonic_compressible";
    } else if (Ma < 1.2) {
        return "transonic";
    } else {
        return "supersonic";
    }
}

json ExternalFlowAnalyzer::getInputSchema() const {
    return json{
        {"type", "object"},
        {"properties",
         {{"vehicleType",
           {{"type", "string"},
            {"enum", {"car", "aircraft", "building", "cylinder"}},
            {"default", "car"},
            {"description", "Type of vehicle/object for analysis"}}},
          {"velocity",
           {{"type", "number"},
            {"minimum", 0.1},
            {"maximum", 340},
            {"description", "Flow velocity in m/s"}}},
          {"characteristicLength",
           {{"type", "number"},
            {"minimum", 0.001},
            {"maximum", 100},
            {"description", "Characteristic length in m (length for car, chord for aircraft)"}}},
          {"frontalArea",
           {{"type", "number"},
            {"minimum", 0.001},
            {"maximum", 1000},
            {"default", 2.5},
            {"description", "Frontal area in m²"}}},
          {"altitude",
           {{"type", "number"},
            {"minimum", 0},
            {"maximum", 20000},
            {"default", 0},
            {"description", "Altitude in m (affects air properties)"}}},
          {"fluidType",
           {{"type", "string"},
            {"enum", {"air", "water"}},
            {"default", "air"},
            {"description", "Fluid type"}}},
          {"objective",
           {{"type", "string"},
            {"enum", {"drag_analysis", "lift_analysis", "pressure_distribution"}},
            {"default", "drag_analysis"},
            {"description", "Analysis objective"}}},
          {"density",
           {{"type", "number"},
            {"minimum", 0.1},
            {"maximum", 2000},
            {"default", 1.225},
            {"description", "Fluid density in kg/m³"}}},
          {"viscosity",
           {{"type", "number"},
            {"minimum", 1e-7},
            {"maximum", 1e-3},
            {"default", 1.5e-5},
            {"description", "Kinematic viscosity in m²/s"}}},
          {"temperature",
           {{"type", "number"},
            {"minimum", 200},
            {"maximum", 400},
            {"default", 288.15},
            {"description", "Temperature in K"}}}}},
        {"required", {"vehicleType", "velocity", "characteristicLength"}},
        {"additionalProperties", false}};
}

ExternalFlowInput ExternalFlowAnalyzer::parseInput(const json& inputJson) {
    ExternalFlowInput input;
    from_json(inputJson, input);
    return input;
}

json ExternalFlowAnalyzer::resultsToJson(const ExternalFlowResults& results) {
    json j;
    to_json(j, results);
    return j;
}

void ExternalFlowAnalyzer::setWorkingDirectory(const std::string& workingDir) {
    caseManager_->setWorkingDirectory(workingDir);
}

std::vector<std::string> ExternalFlowAnalyzer::listActiveCases() const {
    return caseManager_->listCases();
}

bool ExternalFlowAnalyzer::deleteCaseData(const std::string& caseId) {
    return caseManager_->deleteCaseData(caseId);
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //