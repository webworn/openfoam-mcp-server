/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Website:  https://openfoam.org
     \\/     M anipulation  |
\*---------------------------------------------------------------------------*/

#include "heat_transfer_tool.hpp"

#include <iomanip>
#include <sstream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                      HeatTransferTool Implementation
\*---------------------------------------------------------------------------*/

HeatTransferTool::HeatTransferTool() : caseManager_(std::make_unique<CaseManager>()) {}

ToolResult HeatTransferTool::execute(const json& arguments) {
    ToolResult result;

    try {
        // Parse input parameters
        HeatTransferInput input = parseInput(arguments);

        if (!validateInputParameters(input)) {
            result.addErrorContent("Invalid input parameters");
            return result;
        }

        // Perform heat transfer analysis
        HeatTransferResults results = analyzer_.analyze(input);

        // Setup and run OpenFOAM case if analysis was successful
        if (results.success) {
            try {
                std::string caseId = setupOpenFOAMCase(input);
                results.caseId = caseId;

                // Run the case
                CaseParameters caseParams = createCaseParameters(input);
                if (caseManager_->runCase(caseId)) {
                    results.success = true;
                } else {
                    // Analysis succeeded but OpenFOAM failed - still provide theoretical results
                    CaseResult caseResult = caseManager_->getCaseResult(caseId);
                    results.errorMessage = "OpenFOAM simulation failed: " + caseResult.errorOutput;
                }
            } catch (const std::exception& e) {
                results.errorMessage = "OpenFOAM case setup failed: " + std::string(e.what());
                // Keep theoretical results even if OpenFOAM fails
            }
        }

        // Format and return results
        std::vector<json> content = formatResults(input, results);
        for (const auto& item : content) {
            result.content.push_back(item);
        }
    } catch (const std::exception& e) {
        result.addErrorContent("Heat transfer analysis failed: " + std::string(e.what()));
    }

    return result;
}

HeatTransferInput HeatTransferTool::parseInput(const json& arguments) {
    HeatTransferInput input = {};

    // Required parameters
    input.analysisType = arguments.value("analysisType", "electronics_cooling");
    input.characteristicLength = arguments.value("characteristicLength", 0.01);  // 1cm chip
    input.heatGeneration = arguments.value("heatGeneration", 10.0);              // 10W
    input.ambientTemperature = arguments.value("ambientTemperature", 298.15);    // 25°C
    input.maxAllowableTemp = arguments.value("maxAllowableTemp", 358.15);        // 85°C

    // Flow conditions
    input.inletVelocity = arguments.value("inletVelocity", 2.0);           // 2 m/s
    input.inletTemperature = arguments.value("inletTemperature", 298.15);  // 25°C
    input.coolantType = arguments.value("coolantType", "air");

    // Optional parameters with defaults
    input.fluidRegions = arguments.value("fluidRegions", "air");
    input.solidRegions = arguments.value("solidRegions", "chip,heatsink");
    input.targetAccuracy = arguments.value("targetAccuracy", 1.0);  // 1K accuracy
    input.maxIterations = arguments.value("maxIterations", 1000);
    input.couplingMode = arguments.value("couplingMode", "strong");
    input.steadyState = arguments.value("steadyState", true);
    input.timeStep = arguments.value("timeStep", 0.01);
    input.endTime = arguments.value("endTime", 100.0);

    // Populate material properties
    analyzer_.populateMaterialProperties(input);

    return input;
}

bool HeatTransferTool::validateInputParameters(const HeatTransferInput& input) const {
    return analyzer_.validateInput(input);
}

std::string HeatTransferTool::setupOpenFOAMCase(const HeatTransferInput& input) {
    CaseParameters params = createCaseParameters(input);
    return caseManager_->createCase(params);
}

CaseParameters HeatTransferTool::createCaseParameters(const HeatTransferInput& input) const {
    CaseParameters params;
    params.caseName = "heat_transfer_analysis";
    params.solver = "chtMultiRegionFoam";
    params.endTime = input.steadyState ? 1000.0 : input.endTime;
    params.deltaTime = input.steadyState ? 1.0 : input.timeStep;
    params.writeInterval = 100;

    // Physical properties
    params.physicalProperties["heatGeneration"] = std::to_string(input.heatGeneration);
    params.physicalProperties["ambientTemperature"] = std::to_string(input.ambientTemperature);
    params.physicalProperties["inletVelocity"] = std::to_string(input.inletVelocity);
    params.physicalProperties["inletTemperature"] = std::to_string(input.inletTemperature);
    params.physicalProperties["coolantType"] = input.coolantType;

    return params;
}

std::vector<json> HeatTransferTool::formatResults(const HeatTransferInput& input,
                                                  const HeatTransferResults& results) const {
    std::vector<json> content;

    // Main results summary
    std::ostringstream summary;
    summary << "🌡️ **OpenFOAM Heat Transfer Analysis Results**\n\n";
    summary << "**Thermal Performance:**\n";
    summary << "• Maximum Temperature: " << std::fixed << std::setprecision(1)
            << (results.maxTemperature - 273.15) << "°C\n";
    summary << "• Temperature Rise: " << std::fixed << std::setprecision(1)
            << (results.maxTemperature - input.ambientTemperature) << "K\n";
    summary << "• Thermal Resistance: " << std::fixed << std::setprecision(3)
            << results.thermalResistance << " K/W\n";
    summary << "• Cooling Effectiveness: " << std::fixed << std::setprecision(1)
            << results.coolingEffectiveness * 100.0 << "%\n\n";

    summary << "**Heat Transfer Breakdown:**\n";
    summary << "• Total Heat Transfer: " << std::fixed << std::setprecision(1)
            << results.totalHeatTransferRate << " W\n";
    summary << "• Convective: " << std::fixed << std::setprecision(1)
            << results.convectiveHeatTransfer << " W (" << std::setprecision(0)
            << (results.convectiveHeatTransfer / results.totalHeatTransferRate) * 100 << "%)\n";
    summary << "• Conductive: " << std::fixed << std::setprecision(1)
            << results.conductiveHeatTransfer << " W (" << std::setprecision(0)
            << (results.conductiveHeatTransfer / results.totalHeatTransferRate) * 100 << "%)\n";
    summary << "• Radiative: " << std::fixed << std::setprecision(1)
            << results.radiativeHeatTransfer << " W (" << std::setprecision(0)
            << (results.radiativeHeatTransfer / results.totalHeatTransferRate) * 100 << "%)\n\n";

    summary << (results.success ? "✅" : "❌") << " **OpenFOAM Simulation Status:** ";
    summary << (results.success ? "Completed successfully"
                                : "Failed - using theoretical calculations")
            << "\n";
    if (!results.caseId.empty()) {
        summary << "📁 **Case ID:** " << results.caseId << "\n";
    }

    content.push_back(json{{"type", "text"}, {"text", summary.str()}});

    // Physics explanation
    content.push_back(json{{"type", "text"}, {"text", generateThermalExplanation(input, results)}});

    // Design recommendations
    content.push_back(
        json{{"type", "text"}, {"text", generateDesignRecommendations(input, results)}});

    // Application-specific guidance
    content.push_back(
        json{{"type", "text"}, {"text", generateApplicationGuidance(input, results)}});

    // Add warning if OpenFOAM failed
    if (!results.success && !results.errorMessage.empty()) {
        content.push_back(
            json{{"type", "text"},
                 {"text",
                  "⚠️  OpenFOAM simulation failed, but theoretical calculations were provided."}});
        content.push_back(
            json{{"type", "text"}, {"text", "Error details: " + results.errorMessage}});
    }

    // JSON resource with detailed results
    std::string resourceUri = results.success ? "openfoam://heat_transfer/" + results.caseId
                                              : "theoretical://heat_transfer/analysis";

    content.push_back(
        json{{"type", "resource"},
             {"resource",
              json{{"uri", resourceUri},
                   {"name", "Heat Transfer Analysis Results"},
                   {"description", "Complete OpenFOAM conjugate heat transfer analysis results"},
                   {"mimeType", "application/json"}}},
             {"text", analyzer_.toJson(results).dump(2)}});

    return content;
}

std::string HeatTransferTool::generateThermalExplanation(const HeatTransferInput& input,
                                                         const HeatTransferResults& results) const {
    std::ostringstream explanation;
    explanation << "🔥 **Heat Transfer Physics Explanation:**\n\n";

    // Heat transfer modes
    explanation << "**Heat Transfer Mechanisms:**\n";
    explanation << "• **Conduction**: Heat transfer through solid materials\n";
    explanation << "• **Convection**: Heat transfer to moving fluid (cooling)\n";
    explanation << "• **Radiation**: Heat transfer through electromagnetic waves\n\n";

    // Dimensionless numbers
    explanation << generateDimensionlessNumbers(results);

    // Application-specific physics
    if (input.analysisType == "electronics_cooling") {
        explanation << "**Electronics Cooling Physics:**\n";
        explanation << "• Junction-to-ambient thermal path critical\n";
        explanation << "• Thermal interface materials reduce contact resistance\n";
        explanation << "• Heat spreading in package and PCB\n";
        explanation << "• Natural vs forced convection trade-offs\n\n";
    } else if (input.analysisType == "heat_exchanger") {
        explanation << "**Heat Exchanger Physics:**\n";
        explanation << "• Effectiveness-NTU method for thermal design\n";
        explanation << "• Counter-flow provides maximum heat transfer\n";
        explanation << "• Fouling resistance reduces performance over time\n";
        explanation << "• Pressure drop vs heat transfer optimization\n\n";
    }

    return explanation.str();
}

std::string HeatTransferTool::generateDimensionlessNumbers(
    const HeatTransferResults& results) const {
    std::ostringstream numbers;
    numbers << "**Key Dimensionless Numbers:**\n";

    if (results.reynoldsNumber > 0) {
        numbers << "• Reynolds Number: Re = " << std::scientific << std::setprecision(2)
                << results.reynoldsNumber << "\n";
        numbers << "  - Represents ratio of inertial to viscous forces\n";
        numbers << "  - Flow regime: " << (results.reynoldsNumber > 2300 ? "Turbulent" : "Laminar")
                << "\n";
    }

    if (results.prandtlNumber > 0) {
        numbers << "• Prandtl Number: Pr = " << std::fixed << std::setprecision(2)
                << results.prandtlNumber << "\n";
        numbers << "  - Ratio of momentum to thermal diffusivity\n";
        numbers << "  - Affects boundary layer thickness\n";
    }

    if (results.nusseltNumber > 0) {
        numbers << "• Nusselt Number: Nu = " << std::fixed << std::setprecision(1)
                << results.nusseltNumber << "\n";
        numbers << "  - Dimensionless heat transfer coefficient\n";
        numbers << "  - Higher Nu = better heat transfer\n";
    }

    if (results.rayleighNumber > 0) {
        numbers << "• Rayleigh Number: Ra = " << std::scientific << std::setprecision(2)
                << results.rayleighNumber << "\n";
        numbers << "  - Drives natural convection\n";
        numbers << "  - Critical for buoyancy-driven flows\n";
    }

    numbers << "\n";
    return numbers.str();
}

std::string HeatTransferTool::generateDesignRecommendations(
    const HeatTransferInput& input, const HeatTransferResults& results) const {
    std::ostringstream recommendations;
    recommendations << "💡 **Thermal Design Recommendations:**\n\n";

    // Temperature check
    if (!results.thermalConstraintsSatisfied) {
        recommendations << "⚠️ **THERMAL VIOLATION**: Maximum temperature exceeds limit!\n";
        recommendations << "• Current max: " << std::fixed << std::setprecision(1)
                        << (results.maxSolidTemperature - 273.15) << "°C\n";
        recommendations << "• Allowable: " << std::fixed << std::setprecision(1)
                        << (input.maxAllowableTemp - 273.15) << "°C\n\n";
    }

    // Performance optimizations
    if (results.thermalEfficiency < 70.0) {
        recommendations << "**Efficiency Improvements:**\n";
        recommendations << "• Increase coolant flow rate\n";
        recommendations << "• Improve heat sink design\n";
        recommendations << "• Add thermal interface material\n";
        recommendations << "• Consider heat spreader\n\n";
    }

    // Pressure drop considerations
    if (results.pressureDrop > 100.0)  // Pa
    {
        recommendations << "**Pressure Drop Optimization:**\n";
        recommendations << "• Current ΔP = " << std::fixed << std::setprecision(0)
                        << results.pressureDrop << " Pa\n";
        recommendations << "• Consider wider flow channels\n";
        recommendations << "• Optimize flow distribution\n";
        recommendations << "• Pumping power = " << std::fixed << std::setprecision(2)
                        << results.pumpingPower << " W\n\n";
    }

    // Application-specific recommendations
    if (input.analysisType == "electronics_cooling") {
        recommendations << "**Electronics Cooling:**\n";
        recommendations << "• Target thermal resistance < 5 K/W for high-power chips\n";
        recommendations << "• Consider direct liquid cooling for >50W\n";
        recommendations << "• Optimize PCB copper layers for heat spreading\n\n";
    }

    return recommendations.str();
}

std::string HeatTransferTool::generateApplicationGuidance(
    const HeatTransferInput& input, const HeatTransferResults& results) const {
    std::ostringstream guidance;
    guidance << "🎯 **Application-Specific Guidance:**\n\n";

    if (input.analysisType == "electronics_cooling") {
        guidance << "**Electronics Thermal Management:**\n";
        guidance << "• **Chip Level**: Junction temp = " << std::fixed << std::setprecision(1)
                 << (results.maxSolidTemperature - 273.15) << "°C\n";
        guidance << "• **Package Level**: Thermal resistance budget allocation\n";
        guidance << "• **System Level**: Ambient to chassis thermal path\n";
        guidance << "• **Reliability**: Every 10°C reduction = 2x longer life\n\n";

        guidance << "**Design Targets:**\n";
        guidance << "• CPU/GPU: θJA < 1-5 K/W depending on power\n";
        guidance << "• Power electronics: Junction temp < 125°C\n";
        guidance << "• LED: Junction temp < 85°C for long life\n";
    } else if (input.analysisType == "heat_exchanger") {
        guidance << "**Heat Exchanger Design:**\n";
        guidance << "• **Effectiveness**: " << std::fixed << std::setprecision(1)
                 << results.coolingEffectiveness * 100.0 << "% (Target: >80%)\n";
        guidance << "• **Thermal Duty**: " << std::fixed << std::setprecision(1)
                 << results.totalHeatTransferRate << " W\n";
        guidance << "• **Pressure Drop**: Balance with pumping power\n";
        guidance << "• **Fouling**: Design margin for degradation\n\n";
    }

    guidance << "**Next Steps:**\n";
    guidance << "• **Detailed CFD**: Run full 3D conjugate heat transfer\n";
    guidance << "• **Validation**: Thermal testing with thermocouples\n";
    guidance << "• **Optimization**: Parametric studies for design space\n";
    guidance << "• **Manufacturing**: Thermal interface material selection\n";

    return guidance.str();
}

json HeatTransferTool::getInputSchema() {
    return json{
        {"type", "object"},
        {"required", json::array({"analysisType", "characteristicLength", "heatGeneration"})},
        {"additionalProperties", false},
        {"properties",
         json{{"analysisType", json{{"type", "string"},
                                    {"enum", json::array({"electronics_cooling", "heat_exchanger",
                                                          "building", "engine"})},
                                    {"default", "electronics_cooling"},
                                    {"description", "Type of heat transfer analysis"}}},
              {"characteristicLength",
               json{{"type", "number"},
                    {"minimum", 0.001},
                    {"maximum", 10.0},
                    {"description",
                     "Characteristic length in m (chip size, tube diameter, room dimension)"}}},
              {"heatGeneration", json{{"type", "number"},
                                      {"minimum", 0.1},
                                      {"maximum", 10000.0},
                                      {"description", "Heat generation in W"}}},
              {"ambientTemperature", json{{"type", "number"},
                                          {"minimum", 200.0},
                                          {"maximum", 400.0},
                                          {"default", 298.15},
                                          {"description", "Ambient temperature in K"}}},
              {"maxAllowableTemp", json{{"type", "number"},
                                        {"minimum", 300.0},
                                        {"maximum", 500.0},
                                        {"default", 358.15},
                                        {"description", "Maximum allowable temperature in K"}}},
              {"inletVelocity", json{{"type", "number"},
                                     {"minimum", 0.1},
                                     {"maximum", 50.0},
                                     {"default", 2.0},
                                     {"description", "Coolant inlet velocity in m/s"}}},
              {"inletTemperature", json{{"type", "number"},
                                        {"minimum", 200.0},
                                        {"maximum", 400.0},
                                        {"default", 298.15},
                                        {"description", "Coolant inlet temperature in K"}}},
              {"coolantType", json{{"type", "string"},
                                   {"enum", json::array({"air", "water", "oil"})},
                                   {"default", "air"},
                                   {"description", "Type of cooling fluid"}}},
              {"fluidRegions", json{{"type", "string"},
                                    {"default", "air"},
                                    {"description", "Comma-separated list of fluid region names"}}},
              {"solidRegions", json{{"type", "string"},
                                    {"default", "chip,heatsink"},
                                    {"description", "Comma-separated list of solid region names"}}},
              {"targetAccuracy", json{{"type", "number"},
                                      {"minimum", 0.1},
                                      {"maximum", 10.0},
                                      {"default", 1.0},
                                      {"description", "Target temperature accuracy in K"}}},
              {"couplingMode", json{{"type", "string"},
                                    {"enum", json::array({"weak", "strong"})},
                                    {"default", "strong"},
                                    {"description", "Fluid-solid coupling mode"}}},
              {"steadyState", json{{"type", "boolean"},
                                   {"default", true},
                                   {"description", "Steady state or transient analysis"}}},
              {"timeStep", json{{"type", "number"},
                                {"minimum", 0.001},
                                {"maximum", 1.0},
                                {"default", 0.01},
                                {"description", "Time step for transient analysis in s"}}},
              {"endTime", json{{"type", "number"},
                               {"minimum", 1.0},
                               {"maximum", 1000.0},
                               {"default", 100.0},
                               {"description", "End time for transient analysis in s"}}}}}};
}

void registerHeatTransferTool(McpServer& server) {
    auto tool = std::make_shared<HeatTransferTool>();

    server.registerTool(
        HeatTransferTool::getName(), HeatTransferTool::getDescription(),
        HeatTransferTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult { return tool->execute(arguments); });
}

}  // namespace MCP
}  // namespace Foam