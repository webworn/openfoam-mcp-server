/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Multiphase Flow Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool implementation for multiphase flow analysis

\*---------------------------------------------------------------------------*/

#include "multiphase_flow_tool.hpp"

#include <iomanip>
#include <sstream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                      MultiphaseFlowTool Implementation
\*---------------------------------------------------------------------------*/

MultiphaseFlowTool::MultiphaseFlowTool() : caseManager_(std::make_unique<CaseManager>()) {}

ToolResult MultiphaseFlowTool::execute(const json& arguments) {
    ToolResult result;

    try {
        // Parse and validate input
        MultiphaseFlowInput input = parseInput(arguments);

        if (!validateInputParameters(input)) {
            result.isError = true;
            result.content = {
                json{{"type", "text"}, {"text", "❌ Invalid input parameters provided"}}};
            return result;
        }

        // Perform analysis
        MultiphaseFlowResults results = analyzer_.analyze(input);

        // Format results
        result.content = formatResults(input, results);
        result.isError = false;

    } catch (const std::exception& e) {
        result.isError = true;
        result.content = {
            json{{"type", "text"},
                 {"text", "❌ Error in multiphase flow analysis: " + std::string(e.what())}}};
    }

    return result;
}

MultiphaseFlowInput MultiphaseFlowTool::parseInput(const json& arguments) {
    MultiphaseFlowInput input = {};

    // Required parameters
    input.analysisType = arguments.at("analysisType");
    input.characteristicLength = arguments.at("characteristicLength");
    input.height = arguments.at("height");

    // Optional parameters with defaults
    if (arguments.contains("phase1")) input.phase1 = arguments["phase1"];
    if (arguments.contains("phase2")) input.phase2 = arguments["phase2"];
    if (arguments.contains("width")) input.width = arguments["width"];
    if (arguments.contains("depth")) input.depth = arguments["depth"];
    if (arguments.contains("density1")) input.density1 = arguments["density1"];
    if (arguments.contains("density2")) input.density2 = arguments["density2"];
    if (arguments.contains("viscosity1")) input.viscosity1 = arguments["viscosity1"];
    if (arguments.contains("viscosity2")) input.viscosity2 = arguments["viscosity2"];
    if (arguments.contains("surfaceTension")) input.surfaceTension = arguments["surfaceTension"];
    if (arguments.contains("gravity")) input.gravity = arguments["gravity"];
    if (arguments.contains("initialVelocity")) input.initialVelocity = arguments["initialVelocity"];
    if (arguments.contains("contactAngle")) input.contactAngle = arguments["contactAngle"];
    if (arguments.contains("steadyState")) input.steadyState = arguments["steadyState"];
    if (arguments.contains("timeStep")) input.timeStep = arguments["timeStep"];
    if (arguments.contains("endTime")) input.endTime = arguments["endTime"];
    if (arguments.contains("writeInterval")) input.writeInterval = arguments["writeInterval"];
    if (arguments.contains("courantNumber")) input.courantNumber = arguments["courantNumber"];
    if (arguments.contains("turbulenceModel")) input.turbulenceModel = arguments["turbulenceModel"];
    if (arguments.contains("compressible")) input.compressible = arguments["compressible"];

    return input;
}

bool MultiphaseFlowTool::validateInputParameters(const MultiphaseFlowInput& input) const {
    return analyzer_.validateInput(input);
}

std::vector<json> MultiphaseFlowTool::formatResults(const MultiphaseFlowInput& input,
                                                    const MultiphaseFlowResults& results) const {
    std::vector<json> content;

    // Main results summary
    std::ostringstream summary;
    summary << std::fixed << std::setprecision(3);
    summary << "🌊 **Multiphase Flow Analysis Results**\n\n";
    summary << "**Analysis Type:** " << input.analysisType << "\n";
    summary << "**Phases:** " << input.phase1 << " / " << input.phase2 << "\n\n";

    summary << "**Key Dimensionless Numbers:**\n";
    summary << "• Reynolds Number: " << results.reynoldsNumber << "\n";
    summary << "• Weber Number: " << results.webberNumber << "\n";
    summary << "• Bond Number: " << results.bondNumber << "\n";
    summary << "• Froude Number: " << results.froudeNumber << "\n";
    summary << "• Capillary Number: " << results.capillaryNumber << "\n\n";

    summary << "**Flow Characteristics:**\n";
    summary << "• Flow Regime: " << results.flowRegime << "\n";
    summary << "• Mixing: " << results.mixingCharacteristics << "\n";
    summary << "• Max Velocity: " << results.maxVelocity << " m/s\n";
    summary << "• Pressure Drop: " << results.pressureDrop / 1000.0 << " kPa\n\n";

    summary << "**Phase Distribution:**\n";
    summary << "• " << input.phase1 << " Volume: " << results.phase1Volume << " m³\n";
    summary << "• " << input.phase2 << " Volume: " << results.phase2Volume << " m³\n";
    summary << "• Interface Area: " << results.interfaceArea << " m²\n\n";

    summary << "**Energy Analysis:**\n";
    summary << "• Kinetic Energy: " << results.kineticEnergy << " J\n";
    summary << "• Potential Energy: " << results.potentialEnergy << " J\n";
    summary << "• Surface Energy: " << results.surfaceEnergy << " J\n\n";

    if (results.success && !results.caseId.empty()) {
        summary << "📁 **Case ID:** " << results.caseId << "\n";
    }

    content.push_back(json{{"type", "text"}, {"text", summary.str()}});

    // Physics explanation
    content.push_back(json{{"type", "text"}, {"text", generatePhysicsExplanation(input, results)}});

    // Dimensionless analysis
    content.push_back(json{{"type", "text"}, {"text", generateDimensionlessAnalysis(results)}});

    // Flow regime analysis
    content.push_back(json{{"type", "text"}, {"text", generateFlowRegimeAnalysis(input, results)}});

    // Application guidance
    content.push_back(
        json{{"type", "text"}, {"text", generateApplicationGuidance(input, results)}});

    // Add warning if OpenFOAM failed
    if (!results.success && !results.errorMessage.empty()) {
        json warning;
        warning["type"] = "text";
        warning["text"] =
            "⚠️  OpenFOAM simulation failed, but theoretical calculations were provided.";
        content.push_back(warning);

        json error;
        error["type"] = "text";
        error["text"] = "Error details: " + results.errorMessage;
        content.push_back(error);
    }

    // JSON resource with detailed results
    std::string resourceUri = results.success ? "openfoam://multiphase/" + results.caseId
                                              : "theoretical://multiphase/analysis";

    content.push_back(
        json{{"type", "resource"},
             {"resource",
              json{{"uri", resourceUri},
                   {"name", "Multiphase Flow Analysis Results"},
                   {"description", "Complete OpenFOAM interFoam multiphase flow analysis results"},
                   {"mimeType", "application/json"}}},
             {"text", analyzer_.toJson(results).dump(2)}});

    return content;
}

std::string MultiphaseFlowTool::generatePhysicsExplanation(
    const MultiphaseFlowInput& input, const MultiphaseFlowResults& results) const {
    std::ostringstream explanation;
    explanation << "🌊 **Multiphase Flow Physics Explanation:**\n\n";

    explanation << "This analysis simulates the interaction between **" << input.phase1
                << "** and **" << input.phase2 << "** in a ";

    if (input.isDamBreak()) {
        explanation
            << "**dam break scenario**. The sudden release of fluid creates complex free surface "
               "dynamics with gravity-driven flow, wave formation, and energy dissipation.\n\n";
    } else if (input.isSloshing()) {
        explanation
            << "**sloshing scenario**. The fluid motion in a partially filled container creates "
               "complex interface dynamics with potential resonance effects.\n\n";
    } else if (input.isBubble()) {
        explanation
            << "**bubble dynamics scenario**. The analysis captures bubble formation, rise, and "
               "interaction with the surrounding fluid phase.\n\n";
    } else if (input.isWave()) {
        explanation
            << "**wave propagation scenario**. The simulation models wave generation, propagation, "
               "and breaking phenomena.\n\n";
    } else {
        explanation
            << "**mixing scenario**. The analysis studies the interaction and mixing between the "
               "two fluid phases.\n\n";
    }

    explanation << "**Key Physical Phenomena:**\n";
    explanation << "• **Surface Tension**: σ = " << input.surfaceTension
                << " N/m creates interface curvature\n";
    explanation << "• **Density Difference**: Δρ = " << std::abs(input.density1 - input.density2)
                << " kg/m³ drives buoyancy\n";
    explanation << "• **Viscous Effects**: Different viscosities create shear at the interface\n";
    explanation << "• **Gravitational Force**: g = " << input.gravity
                << " m/s² provides body force\n\n";

    return explanation.str();
}

std::string MultiphaseFlowTool::generateDimensionlessAnalysis(
    const MultiphaseFlowResults& results) const {
    std::ostringstream analysis;
    analysis << "📊 **Dimensionless Number Analysis:**\n\n";

    analysis << "**Reynolds Number (Re = " << std::fixed << std::setprecision(1)
             << results.reynoldsNumber << "):**\n";
    if (results.reynoldsNumber < 1) {
        analysis << "• **Viscous-dominated flow** - Viscous forces dominate over inertial forces\n";
        analysis << "• Smooth laminar interface, predictable flow patterns\n";
    } else if (results.reynoldsNumber < 100) {
        analysis << "• **Transitional regime** - Balanced viscous and inertial effects\n";
        analysis << "• Moderate interface deformation\n";
    } else {
        analysis << "• **Inertia-dominated flow** - Turbulent mixing likely\n";
        analysis << "• Complex interface topology, enhanced mixing\n";
    }
    analysis << "\n";

    analysis << "**Weber Number (We = " << std::fixed << std::setprecision(1)
             << results.webberNumber << "):**\n";
    if (results.webberNumber < 1) {
        analysis << "• **Surface tension dominated** - Interface remains smooth\n";
        analysis << "• Droplet formation unlikely, stable interface\n";
    } else if (results.webberNumber < 10) {
        analysis << "• **Moderate deformation** - Interface oscillations possible\n";
        analysis << "• Some droplet formation at high shear regions\n";
    } else {
        analysis << "• **High deformation regime** - Significant interface breakup\n";
        analysis << "• Droplet formation, spray generation likely\n";
    }
    analysis << "\n";

    analysis << "**Bond Number (Bo = " << std::fixed << std::setprecision(1) << results.bondNumber
             << "):**\n";
    if (results.bondNumber < 1) {
        analysis << "• **Surface tension controls** - Spherical droplets/bubbles\n";
        analysis << "• Capillary length dominates characteristic length\n";
    } else {
        analysis << "• **Gravity controls** - Flattened droplets/bubbles\n";
        analysis << "• Gravitational deformation significant\n";
    }
    analysis << "\n";

    return analysis.str();
}

std::string MultiphaseFlowTool::generateFlowRegimeAnalysis(
    const MultiphaseFlowInput& input, const MultiphaseFlowResults& results) const {
    std::ostringstream analysis;
    analysis << "🔄 **Flow Regime Classification:**\n\n";

    analysis << "**Primary Regime: " << results.flowRegime << "**\n";

    if (results.flowRegime == "surface-tension-dominated") {
        analysis << "• Interface curvature effects are significant\n";
        analysis << "• Capillary waves may form\n";
        analysis << "• Contact angle effects important at walls\n";
        analysis << "• Droplet coalescence/breakup governed by surface energy\n";
    } else if (results.flowRegime == "gravity-dominated") {
        analysis << "• Buoyancy forces control interface shape\n";
        analysis << "• Rayleigh-Taylor instability possible\n";
        analysis << "• Hydrostatic pressure gradients significant\n";
        analysis << "• Free surface follows gravitational field lines\n";
    } else if (results.flowRegime == "inertia-dominated") {
        analysis << "• High-speed interface dynamics\n";
        analysis << "• Kelvin-Helmholtz instability possible\n";
        analysis << "• Momentum transfer across interface\n";
        analysis << "• Turbulent mixing enhancement\n";
    }

    analysis << "\n**Mixing Characteristics: " << results.mixingCharacteristics << "**\n";

    if (results.mixingCharacteristics == "laminar-stratified") {
        analysis << "• Clear phase separation maintained\n";
        analysis << "• Diffusion-limited mass transfer\n";
        analysis << "• Smooth interface topology\n";
    } else if (results.mixingCharacteristics == "transitional-mixing") {
        analysis << "• Moderate interface deformation\n";
        analysis << "• Some interfacial area generation\n";
        analysis << "• Enhanced mass transfer rates\n";
    } else if (results.mixingCharacteristics == "turbulent-mixing") {
        analysis << "• High interfacial area generation\n";
        analysis << "• Rapid phase mixing\n";
        analysis << "• Complex droplet size distribution\n";
    }

    analysis << "\n";
    return analysis.str();
}

std::string MultiphaseFlowTool::generateApplicationGuidance(
    const MultiphaseFlowInput& input, const MultiphaseFlowResults& results) const {
    std::ostringstream guidance;
    guidance << "🎯 **Application-Specific Guidance:**\n\n";

    if (input.isDamBreak()) {
        guidance << "**Dam Break / Flood Modeling Applications:**\n";
        guidance << "• **Civil Engineering**: Assess flood wave propagation and impact forces\n";
        guidance << "• **Emergency Planning**: Predict inundation zones and evacuation timing\n";
        guidance << "• **Infrastructure Design**: Calculate loads on downstream structures\n";
        guidance << "• **Environmental Impact**: Model sediment transport and erosion\n\n";

        guidance << "**Design Recommendations:**\n";
        if (results.froudeNumber > 1.0) {
            guidance << "• **Supercritical flow** - High energy, potential for hydraulic jumps\n";
            guidance << "• Consider energy dissipation structures\n";
        } else {
            guidance << "• **Subcritical flow** - More predictable wave propagation\n";
            guidance << "• Focus on accurate bathymetry modeling\n";
        }

    } else if (input.isSloshing()) {
        guidance << "**Sloshing / Tank Design Applications:**\n";
        guidance << "• **Marine Engineering**: Ship tank design and stability analysis\n";
        guidance << "• **Aerospace**: Fuel tank sloshing in rockets and aircraft\n";
        guidance << "• **Industrial**: Chemical reactor and storage tank design\n";
        guidance << "• **LNG Transport**: Cryogenic fluid containment systems\n\n";

        guidance << "**Design Recommendations:**\n";
        guidance << "• Install baffles if mixing characteristics show high energy\n";
        guidance << "• Consider resonance frequency: f ≈ "
                 << std::sqrt(input.gravity / input.characteristicLength) / (2 * M_PI) << " Hz\n";

    } else if (input.isBubble()) {
        guidance << "**Bubble Dynamics Applications:**\n";
        guidance << "• **Chemical Processing**: Gas-liquid contactors and reactors\n";
        guidance << "• **Water Treatment**: Aeration and flotation systems\n";
        guidance << "• **Food Processing**: Foam generation and stability\n";
        guidance << "• **Medical**: Ultrasound contrast agents and drug delivery\n\n";

    } else if (input.isWave()) {
        guidance << "**Wave Dynamics Applications:**\n";
        guidance << "• **Coastal Engineering**: Wave impact on structures\n";
        guidance << "• **Offshore Engineering**: Platform design and wave loading\n";
        guidance << "• **Renewable Energy**: Wave energy converter optimization\n";
        guidance << "• **Port Design**: Harbor resonance and wave penetration\n\n";

    } else if (input.isMixing()) {
        guidance << "**Mixing Process Applications:**\n";
        guidance << "• **Chemical Engineering**: Liquid-liquid extraction systems\n";
        guidance << "• **Food Industry**: Emulsion formation and stability\n";
        guidance << "• **Pharmaceutical**: Drug formulation and delivery\n";
        guidance << "• **Environmental**: Oil spill dispersion modeling\n\n";
    }

    // General optimization suggestions
    guidance << "**Process Optimization:**\n";
    if (results.kineticEnergy > results.potentialEnergy) {
        guidance << "• **High kinetic energy system** - Focus on momentum conservation\n";
        guidance << "• Consider energy recovery or dissipation mechanisms\n";
    }

    if (results.interfaceArea > input.width * input.depth * 2) {
        guidance << "• **High interface area** - Enhanced mass/heat transfer expected\n";
        guidance << "• Optimize contact time for maximum efficiency\n";
    }

    guidance << "\n";
    return guidance.str();
}

json MultiphaseFlowTool::getInputSchema() {
    return json{
        {"type", "object"},
        {"required", json::array({"analysisType", "characteristicLength", "height"})},
        {"additionalProperties", false},
        {"properties",
         json{
             {"analysisType",
              json{{"type", "string"},
                   {"enum", json::array({"dam_break", "sloshing", "bubble", "wave", "mixing"})},
                   {"default", "dam_break"},
                   {"description", "Type of multiphase flow analysis"}}},
             {"characteristicLength", json{{"type", "number"},
                                           {"minimum", 0.01},
                                           {"maximum", 100.0},
                                           {"description", "Domain characteristic length in m"}}},
             {"height", json{{"type", "number"},
                             {"minimum", 0.01},
                             {"maximum", 50.0},
                             {"description", "Initial fluid height in m"}}},
             {"phase1",
              json{{"type", "string"},
                   {"enum", json::array({"water", "oil", "mercury", "glycerin", "ethanol"})},
                   {"default", "water"},
                   {"description", "Primary phase fluid type"}}},
             {"phase2", json{{"type", "string"},
                             {"enum", json::array({"air", "nitrogen", "steam", "co2", "hydrogen"})},
                             {"default", "air"},
                             {"description", "Secondary phase fluid type"}}},
             {"width", json{{"type", "number"},
                            {"minimum", 0.01},
                            {"maximum", 100.0},
                            {"default", 1.0},
                            {"description", "Domain width in m"}}},
             {"depth", json{{"type", "number"},
                            {"minimum", 0.001},
                            {"maximum", 100.0},
                            {"default", 0.1},
                            {"description", "Domain depth in m"}}},
             {"density1", json{{"type", "number"},
                               {"minimum", 0.1},
                               {"maximum", 20000.0},
                               {"default", 1000.0},
                               {"description", "Phase 1 density in kg/m³"}}},
             {"density2", json{{"type", "number"},
                               {"minimum", 0.01},
                               {"maximum", 1000.0},
                               {"default", 1.225},
                               {"description", "Phase 2 density in kg/m³"}}},
             {"viscosity1", json{{"type", "number"},
                                 {"minimum", 1e-6},
                                 {"maximum", 10.0},
                                 {"default", 1e-3},
                                 {"description", "Phase 1 dynamic viscosity in Pa·s"}}},
             {"viscosity2", json{{"type", "number"},
                                 {"minimum", 1e-8},
                                 {"maximum", 1e-3},
                                 {"default", 1.8e-5},
                                 {"description", "Phase 2 dynamic viscosity in Pa·s"}}},
             {"surfaceTension", json{{"type", "number"},
                                     {"minimum", 0.001},
                                     {"maximum", 1.0},
                                     {"default", 0.072},
                                     {"description", "Surface tension coefficient in N/m"}}},
             {"gravity", json{{"type", "number"},
                              {"minimum", 0.1},
                              {"maximum", 50.0},
                              {"default", 9.81},
                              {"description", "Gravitational acceleration in m/s²"}}},
             {"timeStep", json{{"type", "number"},
                               {"minimum", 1e-6},
                               {"maximum", 0.1},
                               {"default", 0.001},
                               {"description", "Time step in s"}}},
             {"endTime", json{{"type", "number"},
                              {"minimum", 0.01},
                              {"maximum", 1000.0},
                              {"default", 1.0},
                              {"description", "End time in s"}}},
             {"turbulenceModel", json{{"type", "string"},
                                      {"enum", json::array({"laminar", "kEpsilon", "kOmegaSST"})},
                                      {"default", "laminar"},
                                      {"description", "Turbulence model selection"}}}}}};
}

void registerMultiphaseFlowTool(McpServer& server) {
    auto tool = std::make_shared<MultiphaseFlowTool>();

    server.registerTool(
        MultiphaseFlowTool::getName(), MultiphaseFlowTool::getDescription(),
        MultiphaseFlowTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult { return tool->execute(arguments); });
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //