/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Pipe Flow Analysis
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Pipe flow analysis tool implementation

\*---------------------------------------------------------------------------*/

#include "pipe_flow.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const PipeFlowInput& input) {
    j = json{{"velocity", input.velocity},
             {"diameter", input.diameter},
             {"length", input.length},
             {"viscosity", input.viscosity},
             {"density", input.density},
             {"roughness", input.roughness},
             {"fluid", input.fluid},
             {"pipeMaterial", input.pipeMaterial}};
}

void from_json(const json& j, PipeFlowInput& input) {
    j.at("velocity").get_to(input.velocity);
    j.at("diameter").get_to(input.diameter);
    j.at("length").get_to(input.length);

    if (j.contains("viscosity")) j.at("viscosity").get_to(input.viscosity);
    if (j.contains("density")) j.at("density").get_to(input.density);
    if (j.contains("fluid")) j.at("fluid").get_to(input.fluid);
    if (j.contains("roughness")) j.at("roughness").get_to(input.roughness);
    if (j.contains("pipeMaterial")) {
        j.at("pipeMaterial").get_to(input.pipeMaterial);
        // If roughness not explicitly set but material provided, use database
        if (!j.contains("roughness") && !input.pipeMaterial.empty()) {
            input.roughness = PipeRoughness::getByName(input.pipeMaterial);
        }
    }
}

void to_json(json& j, const PipeFlowResults& results) {
    j = json{{"reynoldsNumber", results.reynoldsNumber},
             {"frictionFactor", results.frictionFactor},
             {"pressureDrop", results.pressureDrop},
             {"wallShearStress", results.wallShearStress},
             {"maxVelocity", results.maxVelocity},
             {"averageVelocity", results.averageVelocity},
             {"flowRegime", results.flowRegime},
             {"caseId", results.caseId},
             {"success", results.success}};

    if (!results.errorMessage.empty()) {
        j["errorMessage"] = results.errorMessage;
    }
}

void from_json(const json& j, PipeFlowResults& results) {
    j.at("reynoldsNumber").get_to(results.reynoldsNumber);
    j.at("frictionFactor").get_to(results.frictionFactor);
    j.at("pressureDrop").get_to(results.pressureDrop);
    j.at("wallShearStress").get_to(results.wallShearStress);
    j.at("maxVelocity").get_to(results.maxVelocity);
    j.at("averageVelocity").get_to(results.averageVelocity);
    j.at("flowRegime").get_to(results.flowRegime);
    j.at("caseId").get_to(results.caseId);
    j.at("success").get_to(results.success);

    if (j.contains("errorMessage")) j.at("errorMessage").get_to(results.errorMessage);
}

/*---------------------------------------------------------------------------*\
                        PipeFlowAnalyzer Implementation
\*---------------------------------------------------------------------------*/

PipeFlowAnalyzer::PipeFlowAnalyzer() : caseManager_(std::make_unique<CaseManager>()) {}

PipeFlowAnalyzer::PipeFlowAnalyzer(std::unique_ptr<CaseManager> caseManager)
    : caseManager_(std::move(caseManager)) {}

void PipeFlowAnalyzer::validateInput(const PipeFlowInput& input) {
    if (input.velocity <= 0) {
        throw std::invalid_argument("Velocity must be positive");
    }

    if (input.diameter <= 0) {
        throw std::invalid_argument("Diameter must be positive");
    }

    if (input.length <= 0) {
        throw std::invalid_argument("Length must be positive");
    }

    if (input.viscosity <= 0) {
        throw std::invalid_argument("Viscosity must be positive");
    }

    if (input.density <= 0) {
        throw std::invalid_argument("Density must be positive");
    }

    if (input.velocity > 100) {
        throw std::invalid_argument("Velocity too high (>100 m/s), please verify input");
    }

    if (input.diameter > 10) {
        throw std::invalid_argument("Diameter too large (>10 m), please verify input");
    }

    if (input.length > 1000) {
        throw std::invalid_argument("Length too large (>1000 m), please verify input");
    }
}

CaseParameters PipeFlowAnalyzer::createCaseParameters(const PipeFlowInput& input) {
    CaseParameters params;
    params.caseName = "pipe_flow_analysis";

    if (input.isLaminar()) {
        params.solver = "foamRun -solver incompressibleFluid";
        params.caseType = "transient";
        params.endTime = 1.0;
        params.deltaTime = 0.01;
    } else {
        params.solver = "foamRun -solver incompressibleFluid";
        params.caseType = "steady";
        params.endTime = 1000;
        params.deltaTime = 1;
    }

    params.writeInterval = 50;

    params.boundaryConditions["inlet"] = "fixedValue";
    params.boundaryConditions["outlet"] = "zeroGradient";
    params.boundaryConditions["walls"] = "noSlip";

    params.physicalProperties["nu"] = std::to_string(input.viscosity);
    params.physicalProperties["velocity"] = std::to_string(input.velocity);
    params.physicalProperties["diameter"] = std::to_string(input.diameter);
    params.physicalProperties["length"] = std::to_string(input.length);
    params.physicalProperties["density"] = std::to_string(input.density);

    return params;
}

PipeFlowResults PipeFlowAnalyzer::analyze(const PipeFlowInput& input) {
    PipeFlowResults results;

    try {
        validateInput(input);

        results.reynoldsNumber = input.getReynoldsNumber();
        results.flowRegime = determineFlowRegime(input);
        results.averageVelocity = input.velocity;

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

            results.frictionFactor = calculateLaminarFrictionFactor(results.reynoldsNumber);
            results.pressureDrop = calculateTheoreticalPressureDrop(input);
        }
    } catch (const std::exception& e) {
        results.success = false;
        results.errorMessage = e.what();
    }

    return results;
}

PipeFlowResults PipeFlowAnalyzer::processResults(const std::string& caseId,
                                                 const PipeFlowInput& input) {
    PipeFlowResults results;

    CaseResult caseResult = caseManager_->getCaseResult(caseId);

    if (!caseResult.isSuccess()) {
        results.success = false;
        results.errorMessage = "Case execution failed";
        return results;
    }

    results.caseId = caseId;
    results.reynoldsNumber = input.getReynoldsNumber();
    results.flowRegime = determineFlowRegime(input);
    results.averageVelocity = input.velocity;

    if (input.isLaminar()) {
        results.frictionFactor = calculateLaminarFrictionFactor(results.reynoldsNumber);
        results.maxVelocity = 2.0 * input.velocity;  // Parabolic profile
    } else {
        // Use roughness-aware friction factor calculation
        results.frictionFactor = calculateFrictionFactorWithRoughness(input);
        // Turbulent profile: max velocity slightly higher than average
        // Using 1/7th power law approximation: U_max/U_avg ≈ 1.224
        results.maxVelocity = 1.224 * input.velocity;
    }

    results.pressureDrop = calculateTheoreticalPressureDrop(input);
    // Wall shear stress from Darcy friction factor: τ_w = f * ρ * V² / 8
    results.wallShearStress =
        results.frictionFactor * input.density * input.velocity * input.velocity / 8.0;

    results.success = true;

    return results;
}

double PipeFlowAnalyzer::calculateTheoreticalPressureDrop(const PipeFlowInput& input) const {
    // Use friction factor with roughness consideration
    double f = calculateFrictionFactorWithRoughness(input);

    // Darcy-Weisbach equation: ΔP = f * (L/D) * (ρ*V²/2)
    return f * (input.length / input.diameter) * 0.5 * input.density * input.velocity *
           input.velocity;
}

double PipeFlowAnalyzer::calculateLaminarFrictionFactor(double Re) const { return 64.0 / Re; }

double PipeFlowAnalyzer::calculateTurbulentFrictionFactor(double Re) const {
    return 0.316 / std::pow(Re, 0.25);
}

double PipeFlowAnalyzer::calculateColebrookWhiteFrictionFactor(double Re, double relativeRoughness,
                                                                int maxIterations,
                                                                double tolerance) const {
    // Colebrook-White equation (implicit):
    // 1/√f = -2.0 * log10(ε/D/3.7 + 2.51/(Re*√f))
    // Solved iteratively using Newton-Raphson method

    if (Re < 2300) {
        return 64.0 / Re;  // Laminar flow
    }

    // Initial guess using Swamee-Jain for faster convergence
    double f = calculateSwameeJainFrictionFactor(Re, relativeRoughness);
    double sqrtF = std::sqrt(f);

    for (int i = 0; i < maxIterations; ++i) {
        double arg = relativeRoughness / 3.7 + 2.51 / (Re * sqrtF);
        double F = 1.0 / sqrtF + 2.0 * std::log10(arg);

        // Derivative: dF/d(sqrtF) = -1/sqrtF² + 2.51*2/(Re*arg*sqrtF²*ln(10))
        double dF = -1.0 / (sqrtF * sqrtF) +
                    (2.0 * 2.51) / (Re * arg * sqrtF * sqrtF * std::log(10.0));

        double delta = F / dF;
        sqrtF -= delta;

        if (std::abs(delta) < tolerance * sqrtF) {
            break;
        }

        // Ensure sqrtF stays positive
        if (sqrtF <= 0) {
            sqrtF = 0.01;
        }
    }

    return sqrtF * sqrtF;
}

double PipeFlowAnalyzer::calculateSwameeJainFrictionFactor(double Re, double relativeRoughness) const {
    // Swamee-Jain explicit approximation (1976)
    // Valid for: 5000 ≤ Re ≤ 10^8 and 10^-6 ≤ ε/D ≤ 0.05
    // f = 0.25 / [log10(ε/3.7D + 5.74/Re^0.9)]²
    // Accuracy: within 1% of Colebrook-White

    if (Re < 2300) {
        return 64.0 / Re;  // Laminar flow
    }

    // Clamp relative roughness to valid range
    double epsD = std::max(1e-8, std::min(relativeRoughness, 0.05));

    double logArg = epsD / 3.7 + 5.74 / std::pow(Re, 0.9);
    double logTerm = std::log10(logArg);

    return 0.25 / (logTerm * logTerm);
}

double PipeFlowAnalyzer::calculateHaalandFrictionFactor(double Re, double relativeRoughness) const {
    // Haaland explicit approximation (1983)
    // 1/√f = -1.8 * log10[(ε/D/3.7)^1.11 + 6.9/Re]
    // Accuracy: within 2% of Colebrook-White

    if (Re < 2300) {
        return 64.0 / Re;  // Laminar flow
    }

    double epsD = std::max(1e-8, relativeRoughness);
    double logArg = std::pow(epsD / 3.7, 1.11) + 6.9 / Re;
    double oneOverSqrtF = -1.8 * std::log10(logArg);

    return 1.0 / (oneOverSqrtF * oneOverSqrtF);
}

double PipeFlowAnalyzer::calculateFrictionFactorWithRoughness(const PipeFlowInput& input) const {
    double Re = input.getReynoldsNumber();

    // Laminar flow: friction factor independent of roughness
    if (Re < 2300) {
        return 64.0 / Re;
    }

    double relativeRoughness = input.getRelativeRoughness();

    // Smooth pipe (no roughness or hydraulically smooth)
    if (relativeRoughness < 1e-8 || input.isHydraulicallySmooth()) {
        return 0.316 / std::pow(Re, 0.25);  // Blasius for smooth pipes
    }

    // Rough pipe: use Colebrook-White (most accurate)
    return calculateColebrookWhiteFrictionFactor(Re, relativeRoughness);
}

std::string PipeFlowAnalyzer::determineFlowRegime(const PipeFlowInput& input) const {
    if (input.isLaminar()) {
        return "laminar";
    } else if (input.isTurbulent()) {
        return "turbulent";
    } else {
        return "transitional";
    }
}

std::string PipeFlowAnalyzer::generateAnalysisReport(const PipeFlowInput& input,
                                                     const PipeFlowResults& results) const {
    std::ostringstream report;

    report << "=== Pipe Flow Analysis Report ===" << std::endl;
    report << "Input Parameters:" << std::endl;
    report << "  Velocity: " << input.velocity << " m/s" << std::endl;
    report << "  Diameter: " << input.diameter << " m" << std::endl;
    report << "  Length: " << input.length << " m" << std::endl;
    report << "  Fluid: " << input.fluid << std::endl;
    report << std::endl;

    report << "Results:" << std::endl;
    report << "  Reynolds Number: " << std::fixed << std::setprecision(0) << results.reynoldsNumber
           << std::endl;
    report << "  Flow Regime: " << results.flowRegime << std::endl;
    report << "  Friction Factor: " << std::fixed << std::setprecision(4) << results.frictionFactor
           << std::endl;
    report << "  Pressure Drop: " << std::fixed << std::setprecision(2) << results.pressureDrop
           << " Pa" << std::endl;
    report << "  Wall Shear Stress: " << std::fixed << std::setprecision(3)
           << results.wallShearStress << " Pa" << std::endl;
    report << std::endl;

    if (results.success) {
        report << "OpenFOAM simulation completed successfully." << std::endl;
        report << "Case ID: " << results.caseId << std::endl;
    } else {
        report << "OpenFOAM simulation failed: " << results.errorMessage << std::endl;
        report << "Theoretical calculations provided instead." << std::endl;
    }

    return report.str();
}

json PipeFlowAnalyzer::getInputSchema() const {
    return json{{"type", "object"},
                {"properties",
                 {{"velocity",
                   {{"type", "number"},
                    {"minimum", 0.001},
                    {"maximum", 100},
                    {"description", "Flow velocity in m/s"}}},
                  {"diameter",
                   {{"type", "number"},
                    {"minimum", 0.001},
                    {"maximum", 10},
                    {"description", "Pipe diameter in m"}}},
                  {"length",
                   {{"type", "number"},
                    {"minimum", 0.001},
                    {"maximum", 1000},
                    {"description", "Pipe length in m"}}},
                  {"viscosity",
                   {{"type", "number"},
                    {"minimum", 1e-7},
                    {"maximum", 1e-3},
                    {"default", 1e-5},
                    {"description", "Kinematic viscosity in m²/s"}}},
                  {"density",
                   {{"type", "number"},
                    {"minimum", 0.1},
                    {"maximum", 2000},
                    {"default", 1.225},
                    {"description", "Fluid density in kg/m³"}}},
                  {"fluid",
                   {{"type", "string"},
                    {"enum", {"air", "water", "oil", "custom"}},
                    {"default", "air"},
                    {"description", "Fluid type"}}}}},
                {"required", {"velocity", "diameter", "length"}},
                {"additionalProperties", false}};
}

PipeFlowInput PipeFlowAnalyzer::parseInput(const json& inputJson) {
    PipeFlowInput input;
    from_json(inputJson, input);
    return input;
}

json PipeFlowAnalyzer::resultsToJson(const PipeFlowResults& results) {
    json j;
    to_json(j, results);
    return j;
}

void PipeFlowAnalyzer::setWorkingDirectory(const std::string& workingDir) {
    caseManager_->setWorkingDirectory(workingDir);
}

std::vector<std::string> PipeFlowAnalyzer::listActiveCases() const {
    return caseManager_->listCases();
}

bool PipeFlowAnalyzer::deleteCaseData(const std::string& caseId) {
    return caseManager_->deleteCaseData(caseId);
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //