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
    j = json{{"velocity", input.velocity},   {"diameter", input.diameter}, {"length", input.length},
             {"viscosity", input.viscosity}, {"density", input.density},   {"fluid", input.fluid}};
}

void from_json(const json& j, PipeFlowInput& input) {
    j.at("velocity").get_to(input.velocity);
    j.at("diameter").get_to(input.diameter);
    j.at("length").get_to(input.length);

    if (j.contains("viscosity"))
        j.at("viscosity").get_to(input.viscosity);
    if (j.contains("density"))
        j.at("density").get_to(input.density);
    if (j.contains("fluid"))
        j.at("fluid").get_to(input.fluid);
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

    if (j.contains("errorMessage"))
        j.at("errorMessage").get_to(results.errorMessage);
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
        results.maxVelocity = 2.0 * input.velocity;
    } else {
        results.frictionFactor = calculateTurbulentFrictionFactor(results.reynoldsNumber);
        results.maxVelocity = 1.2 * input.velocity;
    }

    results.pressureDrop = calculateTheoreticalPressureDrop(input);
    results.wallShearStress =
        results.frictionFactor * 0.5 * input.density * input.velocity * input.velocity;

    results.success = true;

    return results;
}

double PipeFlowAnalyzer::calculateTheoreticalPressureDrop(const PipeFlowInput& input) const {
    double Re = input.getReynoldsNumber();
    double f;

    if (Re < 2300) {
        f = 64.0 / Re;
    } else {
        f = 0.316 / std::pow(Re, 0.25);
    }

    return f * (input.length / input.diameter) * 0.5 * input.density * input.velocity *
           input.velocity;
}

double PipeFlowAnalyzer::calculateLaminarFrictionFactor(double Re) const {
    return 64.0 / Re;
}

double PipeFlowAnalyzer::calculateTurbulentFrictionFactor(double Re) const {
    return 0.316 / std::pow(Re, 0.25);
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