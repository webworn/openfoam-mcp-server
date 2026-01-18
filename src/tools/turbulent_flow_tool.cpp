/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Turbulent Flow Analysis MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Comprehensive turbulent flow analysis tool implementation.

\*---------------------------------------------------------------------------*/

#include "turbulent_flow_tool.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Input Parsing
\*---------------------------------------------------------------------------*/

TurbulentFlowAnalysisInput TurbulentFlowTool::parseInput(const json& arguments) {
    TurbulentFlowAnalysisInput input;

    // Parse geometry
    if (arguments.contains("geometry")) {
        const auto& geom = arguments["geometry"];
        if (geom.contains("type")) {
            input.geometryType = geom["type"].get<std::string>();
        }
        if (geom.contains("diameter")) {
            input.diameter = geom["diameter"].get<double>();
        }
        if (geom.contains("length")) {
            input.length = geom["length"].get<double>();
        }
        if (geom.contains("hydraulic_diameter")) {
            input.hydraulicDiameter = geom["hydraulic_diameter"].get<double>();
        }
        if (geom.contains("roughness")) {
            input.roughness = geom["roughness"].get<double>();
        }
        if (geom.contains("pipe_material")) {
            input.pipeMaterial = geom["pipe_material"].get<std::string>();
            // If roughness not explicitly set, use material database
            if (!geom.contains("roughness")) {
                input.roughness = PipeRoughness::getByName(input.pipeMaterial);
            }
        }
    }

    // Parse flow conditions
    if (arguments.contains("flow_conditions")) {
        const auto& flow = arguments["flow_conditions"];
        if (flow.contains("velocity")) {
            input.velocity = flow["velocity"].get<double>();
        }
        if (flow.contains("density")) {
            input.density = flow["density"].get<double>();
        }
        if (flow.contains("viscosity")) {
            input.kinematicViscosity = flow["viscosity"].get<double>();
        }
    }

    // Parse turbulence model
    if (arguments.contains("turbulence_model")) {
        input.turbulenceModel = arguments["turbulence_model"].get<std::string>();
    }

    // Parse turbulence parameters
    if (arguments.contains("turbulence_parameters")) {
        const auto& turb = arguments["turbulence_parameters"];
        if (turb.contains("intensity")) {
            input.turbulentIntensity = turb["intensity"].get<double>();
        }
        if (turb.contains("length_scale")) {
            input.turbulentLengthScale = turb["length_scale"].get<double>();
        }
    }

    // Parse mesh parameters
    if (arguments.contains("mesh_parameters")) {
        const auto& mesh = arguments["mesh_parameters"];
        if (mesh.contains("first_cell_height")) {
            input.firstCellHeight = mesh["first_cell_height"].get<double>();
        }
        if (mesh.contains("target_y_plus")) {
            input.targetYPlus = mesh["target_y_plus"].get<double>();
        }
    }

    // Parse analysis mode
    if (arguments.contains("analysis_mode")) {
        input.analysisMode = arguments["analysis_mode"].get<std::string>();
    }

    return input;
}

/*---------------------------------------------------------------------------*\
                        Results to JSON
\*---------------------------------------------------------------------------*/

json TurbulentFlowTool::resultsToJson(const TurbulentFlowAnalysisResult& result) {
    json j;

    j["reynolds_number"] = result.reynoldsNumber;
    j["flow_regime"] = result.flowRegime;
    j["is_fully_turbulent"] = result.isFullyTurbulent;
    j["relative_roughness"] = result.relativeRoughness;

    j["turbulence_model"] = {
        {"recommended", result.recommendedModel},
        {"justification", result.modelJustification},
        {"alternatives", result.modelAlternatives}
    };

    j["friction_factor"] = {
        {"smooth", result.frictionFactorSmooth},
        {"rough", result.frictionFactorRough},
        {"method", result.frictionMethod}
    };

    j["pressure_drop"] = {
        {"pascals", result.pressureDrop},
        {"head_loss_meters", result.headLoss}
    };

    j["mesh_requirements"] = {
        {"estimated_y_plus", result.estimatedYPlus},
        {"recommended_first_cell_height", result.recommendedFirstCellHeight},
        {"target_y_plus", result.recommendedTargetYPlus},
        {"wall_function_valid", result.wallFunctionStatus.isValid}
    };

    j["inlet_conditions"] = {
        {"k", result.inletConditions.k},
        {"epsilon", result.inletConditions.epsilon},
        {"omega", result.inletConditions.omega},
        {"nut", result.inletConditions.nut}
    };

    j["wall_quantities"] = {
        {"shear_stress", result.wallShearStress},
        {"friction_velocity", result.frictionVelocity}
    };

    j["success"] = result.success;
    if (!result.errorMessage.empty()) {
        j["error_message"] = result.errorMessage;
    }

    return j;
}

/*---------------------------------------------------------------------------*\
                        Tool Execution
\*---------------------------------------------------------------------------*/

ToolResult TurbulentFlowTool::execute(const json& arguments) {
    ToolResult toolResult;

    try {
        // Parse input
        TurbulentFlowAnalysisInput input = parseInput(arguments);

        // Perform analysis
        TurbulentFlowAnalysisResult result = performAnalysis(input);

        // Format results
        std::string formattedResults = formatResultsForUser(input, result);
        toolResult.addTextContent(formattedResults);

        // Add educational content
        std::string educational = formatEducationalContent(result);
        toolResult.addTextContent(educational);

        // Add recommendations
        std::string recommendations = formatRecommendations(result);
        toolResult.addTextContent(recommendations);

        // Add JSON resource
        json resultJson = resultsToJson(result);
        toolResult.content.push_back(
            json{{"type", "resource"},
                 {"resource",
                  {{"uri", "openfoam://turbulent_flow_analysis"},
                   {"name", "Turbulent Flow Analysis Results"},
                   {"description", "Complete turbulent flow analysis with model recommendations"},
                   {"mimeType", "application/json"}}},
                 {"text", resultJson.dump(2)}});

    } catch (const std::exception& e) {
        toolResult.addErrorContent("Error in turbulent flow analysis: " + std::string(e.what()));
    }

    return toolResult;
}

/*---------------------------------------------------------------------------*\
                        Core Analysis
\*---------------------------------------------------------------------------*/

TurbulentFlowAnalysisResult TurbulentFlowTool::performAnalysis(
    const TurbulentFlowAnalysisInput& input) {

    TurbulentFlowAnalysisResult result;
    TurbulentFlowAnalysisInput modifiableInput = input;

    try {
        // Auto-calculate turbulent length scale if not provided
        if (modifiableInput.turbulentLengthScale <= 0) {
            double charLength = (modifiableInput.geometryType == "pipe")
                                    ? modifiableInput.diameter
                                    : modifiableInput.hydraulicDiameter;
            if (charLength <= 0) charLength = modifiableInput.diameter;
            modifiableInput.turbulentLengthScale =
                TurbulenceUtils::estimateTurbulentLengthScale(charLength, modifiableInput.geometryType);
        }

        // Step 1: Characterize the flow
        characterizeFlow(modifiableInput, result);

        // Step 2: Select turbulence model
        selectTurbulenceModel(modifiableInput, result);

        // Step 3: Calculate friction factors
        calculateFrictionFactors(modifiableInput, result);

        // Step 4: Analyze mesh requirements (y+)
        analyzeMeshRequirements(modifiableInput, result);

        // Step 5: Calculate inlet conditions
        calculateInletConditions(modifiableInput, result);

        // Step 6: Generate model comparison
        generateModelComparison(result);

        // Step 7: Generate educational content
        generateEducationalContent(modifiableInput, result);

        result.success = true;

    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

void TurbulentFlowTool::characterizeFlow(TurbulentFlowAnalysisInput& input,
                                          TurbulentFlowAnalysisResult& result) {
    // Calculate Reynolds number
    double charLength = (input.geometryType == "pipe") ? input.diameter : input.hydraulicDiameter;
    if (charLength <= 0) charLength = input.diameter;

    result.reynoldsNumber = input.velocity * charLength / input.kinematicViscosity;

    // Determine flow regime
    if (result.reynoldsNumber < 2300) {
        result.flowRegime = "laminar";
        result.isFullyTurbulent = false;
    } else if (result.reynoldsNumber < 4000) {
        result.flowRegime = "transitional";
        result.isFullyTurbulent = false;
    } else {
        result.flowRegime = "turbulent";
        result.isFullyTurbulent = result.reynoldsNumber > 10000;
    }

    // Calculate relative roughness
    if (input.roughness > 0 && charLength > 0) {
        result.relativeRoughness = input.roughness / charLength;
    }
}

void TurbulentFlowTool::selectTurbulenceModel(const TurbulentFlowAnalysisInput& input,
                                               TurbulentFlowAnalysisResult& result) {
    if (input.turbulenceModel != "auto") {
        result.recommendedModel = input.turbulenceModel;
        if (input.turbulenceModel == "laminar") {
            result.modelJustification = "User selected laminar model.";
        } else if (input.turbulenceModel == "kEpsilon") {
            result.modelJustification = "User selected k-epsilon model.";
        } else if (input.turbulenceModel == "kOmegaSST") {
            result.modelJustification = "User selected k-omega SST model.";
        }
        return;
    }

    // Auto-selection logic
    if (result.flowRegime == "laminar") {
        result.recommendedModel = "laminar";
        result.modelJustification =
            "Re = " + std::to_string(static_cast<int>(result.reynoldsNumber)) +
            " < 2300. Flow is laminar; no turbulence model required.";
        result.modelAlternatives = {};

    } else if (result.flowRegime == "transitional") {
        result.recommendedModel = "kOmegaSST";
        result.modelJustification =
            "Re = " + std::to_string(static_cast<int>(result.reynoldsNumber)) +
            " is in the transitional regime (2300-4000). k-omega SST is recommended as it "
            "handles transition better than k-epsilon and can work in both wall-resolved "
            "and wall-function modes.";
        result.modelAlternatives = {"laminar (conservative)", "kEpsilon (if Re > 3500)"};

    } else {
        // Fully turbulent flow
        bool hasAdversePressureGradient = false;  // Would need more info
        bool needsWallResolution = false;

        if (input.geometryType == "external" || hasAdversePressureGradient) {
            result.recommendedModel = "kOmegaSST";
            result.modelJustification =
                "k-omega SST recommended for external flow or flows with adverse pressure gradients. "
                "SST blends k-omega near walls (accurate) with k-epsilon in freestream (robust).";
            result.modelAlternatives = {"kEpsilon (simpler, faster)"};

        } else if (input.geometryType == "pipe" || input.geometryType == "channel") {
            // For internal flows, both models work well
            if (result.reynoldsNumber > 50000) {
                result.recommendedModel = "kEpsilon";
                result.modelJustification =
                    "k-epsilon recommended for high-Re internal flow (Re = " +
                    std::to_string(static_cast<int>(result.reynoldsNumber)) +
                    "). Well-validated for developed pipe/channel flows.";
                result.modelAlternatives = {"kOmegaSST (more accurate near walls)"};
            } else {
                result.recommendedModel = "kOmegaSST";
                result.modelJustification =
                    "k-omega SST recommended for moderate Reynolds number. Provides better "
                    "accuracy in the near-wall region and can handle flow separation.";
                result.modelAlternatives = {"kEpsilon (simpler, well-validated for pipe flow)"};
            }
        } else {
            result.recommendedModel = "kOmegaSST";
            result.modelJustification =
                "k-omega SST is the most versatile RANS model. Recommended as default for "
                "general turbulent flow applications.";
            result.modelAlternatives = {"kEpsilon"};
        }
    }
}

void TurbulentFlowTool::calculateFrictionFactors(const TurbulentFlowAnalysisInput& input,
                                                  TurbulentFlowAnalysisResult& result) {
    double Re = result.reynoldsNumber;

    if (Re < 2300) {
        // Laminar flow
        result.frictionFactorSmooth = 64.0 / Re;
        result.frictionFactorRough = result.frictionFactorSmooth;  // No roughness effect
        result.frictionMethod = "Hagen-Poiseuille (laminar)";
    } else {
        // Turbulent flow - smooth pipe (Blasius)
        result.frictionFactorSmooth = 0.316 / std::pow(Re, 0.25);

        // Turbulent flow - rough pipe (Colebrook-White via Swamee-Jain)
        if (result.relativeRoughness > 1e-8) {
            // Swamee-Jain explicit approximation
            double epsD = std::max(1e-8, std::min(result.relativeRoughness, 0.05));
            double logArg = epsD / 3.7 + 5.74 / std::pow(Re, 0.9);
            double logTerm = std::log10(logArg);
            result.frictionFactorRough = 0.25 / (logTerm * logTerm);
            result.frictionMethod = "Swamee-Jain approximation to Colebrook-White";
        } else {
            result.frictionFactorRough = result.frictionFactorSmooth;
            result.frictionMethod = "Blasius (hydraulically smooth)";
        }
    }

    // Use the appropriate friction factor for pressure drop
    double f = (result.relativeRoughness > 1e-8) ? result.frictionFactorRough
                                                  : result.frictionFactorSmooth;

    // Wall shear stress: τ_w = f * ρ * V² / 8
    result.wallShearStress = f * input.density * input.velocity * input.velocity / 8.0;

    // Friction velocity: u_τ = √(τ_w / ρ)
    result.frictionVelocity = std::sqrt(result.wallShearStress / input.density);

    // Pressure drop: ΔP = f * (L/D) * (ρV²/2)
    double charLength = (input.geometryType == "pipe") ? input.diameter : input.hydraulicDiameter;
    if (charLength <= 0) charLength = input.diameter;

    result.pressureDrop = f * (input.length / charLength) * 0.5 * input.density *
                          input.velocity * input.velocity;

    // Head loss: h = ΔP / (ρ * g)
    result.headLoss = result.pressureDrop / (input.density * 9.81);
}

void TurbulentFlowTool::analyzeMeshRequirements(const TurbulentFlowAnalysisInput& input,
                                                 TurbulentFlowAnalysisResult& result) {
    // Determine target y+ based on model
    if (input.targetYPlus > 0) {
        result.recommendedTargetYPlus = input.targetYPlus;
    } else {
        // Auto-select based on turbulence model
        if (result.recommendedModel == "kOmegaSST") {
            // SST can work in wall-resolved (y+ < 1) or wall-function mode (y+ ~ 30-50)
            result.recommendedTargetYPlus = 1.0;  // Prefer wall-resolved if possible
        } else if (result.recommendedModel == "kEpsilon") {
            // k-epsilon requires wall functions
            result.recommendedTargetYPlus = 50.0;
        } else {
            // Laminar or unknown
            result.recommendedTargetYPlus = 1.0;
        }
    }

    // Calculate recommended first cell height
    double charLength = (input.geometryType == "pipe") ? input.diameter : input.hydraulicDiameter;
    if (charLength <= 0) charLength = input.diameter;

    if (input.geometryType == "pipe") {
        result.recommendedFirstCellHeight = TurbulenceUtils::estimateFirstCellHeightPipe(
            result.recommendedTargetYPlus, input.velocity, input.density,
            input.kinematicViscosity, charLength);
    } else {
        result.recommendedFirstCellHeight = TurbulenceUtils::recommendFirstCellHeight(
            result.recommendedTargetYPlus, input.velocity, input.density,
            input.kinematicViscosity, charLength);
    }

    // Calculate estimated y+ if first cell height is provided
    if (input.firstCellHeight > 0) {
        result.estimatedYPlus = TurbulenceUtils::calculateYPlusFromFrictionVelocity(
            result.frictionVelocity, input.kinematicViscosity, input.firstCellHeight);
    } else {
        result.estimatedYPlus = result.recommendedTargetYPlus;  // Use target as estimate
    }

    // Check wall function validity
    result.wallFunctionStatus = TurbulenceUtils::checkWallFunctionValidity(
        result.estimatedYPlus, result.recommendedModel);
}

void TurbulentFlowTool::calculateInletConditions(const TurbulentFlowAnalysisInput& input,
                                                  TurbulentFlowAnalysisResult& result) {
    result.inletConditions = TurbulenceUtils::calculateInletConditions(
        input.velocity, input.turbulentIntensity,
        input.turbulentLengthScale, input.kinematicViscosity);
}

void TurbulentFlowTool::generateModelComparison(TurbulentFlowAnalysisResult& result) {
    // k-epsilon comparison
    TurbulentFlowAnalysisResult::ModelComparison kEps;
    kEps.model = "k-epsilon (Standard)";
    kEps.strengths = "Well-validated, robust, good for high-Re flows, fast convergence";
    kEps.weaknesses = "Poor near-wall accuracy, struggles with adverse pressure gradients, "
                      "requires wall functions (y+ > 30)";
    kEps.bestFor = "High Reynolds number internal flows, developed pipe/channel flow, "
                   "industrial CFD where speed matters";
    kEps.computationalCost = 1.0;
    result.modelComparisons.push_back(kEps);

    // k-omega SST comparison
    TurbulentFlowAnalysisResult::ModelComparison kOmSST;
    kOmSST.model = "k-omega SST (Menter)";
    kOmSST.strengths = "Excellent near-wall behavior, handles separation well, works in "
                       "wall-resolved (y+ < 1) or wall-function (y+ > 30) modes";
    kOmSST.weaknesses = "More sensitive to inlet conditions, slightly higher computational cost";
    kOmSST.bestFor = "External aerodynamics, flows with separation, adverse pressure gradients, "
                     "cases requiring accurate wall shear stress";
    kOmSST.computationalCost = 1.1;
    result.modelComparisons.push_back(kOmSST);
}

void TurbulentFlowTool::generateEducationalContent(const TurbulentFlowAnalysisInput& input,
                                                    TurbulentFlowAnalysisResult& result) {
    // Physics explanations
    result.physicsExplanations.push_back(
        "Reynolds number (Re = " + std::to_string(static_cast<int>(result.reynoldsNumber)) +
        ") represents the ratio of inertial to viscous forces. Higher Re means turbulence "
        "dominates over molecular diffusion.");

    if (result.flowRegime == "turbulent") {
        result.physicsExplanations.push_back(
            "In turbulent flow, energy cascades from large eddies to smaller ones (Kolmogorov cascade). "
            "The smallest scales (Kolmogorov microscales) are where kinetic energy dissipates to heat.");

        result.physicsExplanations.push_back(
            "y+ characterizes the distance from the wall in viscous wall units. "
            "The wall region has three layers: viscous sublayer (y+ < 5), buffer layer (5 < y+ < 30), "
            "and log-law region (y+ > 30).");
    }

    if (result.relativeRoughness > 1e-6) {
        result.physicsExplanations.push_back(
            "Surface roughness (ε/D = " + std::to_string(result.relativeRoughness) +
            ") affects friction at high Reynolds numbers. The Colebrook-White equation captures "
            "this effect, transitioning from hydraulically smooth to fully rough regimes.");
    }

    // Socratic questions
    result.socraticQuestions.push_back(
        "Why does turbulence increase mixing and heat transfer compared to laminar flow?");

    result.socraticQuestions.push_back(
        "What physical phenomenon does the y+ parameter represent, and why is it important "
        "for turbulence modeling?");

    if (result.modelComparisons.size() > 1) {
        result.socraticQuestions.push_back(
            "When would you choose k-omega SST over k-epsilon, and vice versa?");
    }

    result.socraticQuestions.push_back(
        "How does pipe roughness affect the friction factor at high Reynolds numbers?");

    // Practical tips
    result.practicalTips.push_back(
        "Always verify y+ values post-simulation using ParaView or postProcess utilities.");

    result.practicalTips.push_back(
        "For industrial applications, start with k-epsilon and switch to SST if accuracy is needed.");

    result.practicalTips.push_back(
        "When uncertain about inlet turbulence, use 5% intensity and 0.07*D_h for length scale.");

    if (result.recommendedModel == "kOmegaSST") {
        result.practicalTips.push_back(
            "For k-omega SST, either resolve the boundary layer (y+ < 1) or use wall functions "
            "(y+ = 30-300). Avoid the buffer layer (y+ = 5-30).");
    }

    // References
    result.references.push_back(
        "Menter, F.R. (1994). 'Two-Equation Eddy-Viscosity Turbulence Models for Engineering "
        "Applications.' AIAA Journal, 32(8), 1598-1605.");

    result.references.push_back(
        "Colebrook, C.F. (1939). 'Turbulent Flow in Pipes.' Journal of the Institution of "
        "Civil Engineers, 11(4), 133-156.");

    result.references.push_back(
        "Pope, S.B. (2000). 'Turbulent Flows.' Cambridge University Press.");
}

/*---------------------------------------------------------------------------*\
                        Output Formatting
\*---------------------------------------------------------------------------*/

std::string TurbulentFlowTool::formatResultsForUser(const TurbulentFlowAnalysisInput& input,
                                                     const TurbulentFlowAnalysisResult& result) const {
    std::ostringstream out;
    out << std::fixed;

    out << "**Turbulent Flow Analysis Results**\n\n";

    // Flow Characterization
    out << "**Flow Characterization:**\n";
    out << "- Reynolds Number: " << std::setprecision(0) << result.reynoldsNumber << "\n";
    out << "- Flow Regime: " << result.flowRegime;
    if (result.isFullyTurbulent) {
        out << " (fully developed turbulence)";
    }
    out << "\n";

    if (result.relativeRoughness > 1e-8) {
        out << "- Relative Roughness (ε/D): " << std::setprecision(6) << result.relativeRoughness << "\n";
    }
    out << "\n";

    // Turbulence Model Recommendation
    out << "**Recommended Turbulence Model:**\n";
    out << "- Model: " << result.recommendedModel << "\n";
    out << "- Justification: " << result.modelJustification << "\n";
    if (!result.modelAlternatives.empty()) {
        out << "- Alternatives: ";
        for (size_t i = 0; i < result.modelAlternatives.size(); ++i) {
            out << result.modelAlternatives[i];
            if (i < result.modelAlternatives.size() - 1) out << ", ";
        }
        out << "\n";
    }
    out << "\n";

    // Friction Factor Results
    out << "**Friction Factor Analysis:**\n";
    out << "- Smooth pipe: f = " << std::setprecision(5) << result.frictionFactorSmooth << "\n";
    if (result.relativeRoughness > 1e-8) {
        out << "- Rough pipe: f = " << std::setprecision(5) << result.frictionFactorRough << "\n";
    }
    out << "- Calculation method: " << result.frictionMethod << "\n\n";

    // Pressure Drop
    out << "**Pressure Drop:**\n";
    out << "- ΔP = " << std::setprecision(2) << result.pressureDrop << " Pa\n";
    out << "- Head loss = " << std::setprecision(3) << result.headLoss << " m\n\n";

    // Wall Quantities
    out << "**Wall Quantities:**\n";
    out << "- Wall shear stress: τ_w = " << std::setprecision(3) << result.wallShearStress << " Pa\n";
    out << "- Friction velocity: u_τ = " << std::setprecision(4) << result.frictionVelocity << " m/s\n\n";

    // Mesh Requirements
    out << "**Mesh Requirements:**\n";
    out << "- Target y+: " << std::setprecision(1) << result.recommendedTargetYPlus << "\n";
    out << "- Recommended first cell height: " << std::setprecision(6)
        << result.recommendedFirstCellHeight << " m\n";
    if (input.firstCellHeight > 0) {
        out << "- Estimated y+ (current mesh): " << std::setprecision(1) << result.estimatedYPlus << "\n";
    }
    out << "- Wall function validity: " << (result.wallFunctionStatus.isValid ? "Valid" : "Invalid") << "\n";
    out << "  " << result.wallFunctionStatus.explanation << "\n\n";

    // Inlet Conditions
    out << "**Turbulent Inlet Conditions:**\n";
    out << "- Turbulent kinetic energy: k = " << std::setprecision(4)
        << result.inletConditions.k << " m²/s²\n";
    out << "- Turbulent dissipation: ε = " << std::setprecision(4)
        << result.inletConditions.epsilon << " m²/s³\n";
    out << "- Specific dissipation: ω = " << std::setprecision(2)
        << result.inletConditions.omega << " 1/s\n";
    out << "- Turbulent viscosity: ν_t = " << std::setprecision(6)
        << result.inletConditions.nut << " m²/s\n";

    return out.str();
}

std::string TurbulentFlowTool::formatEducationalContent(
    const TurbulentFlowAnalysisResult& result) const {

    std::ostringstream out;

    out << "\n**Understanding the Physics:**\n\n";

    for (const auto& explanation : result.physicsExplanations) {
        out << explanation << "\n\n";
    }

    out << "**Model Comparison:**\n\n";
    out << "| Model | Best For | Computational Cost |\n";
    out << "|-------|----------|-------------------|\n";
    for (const auto& comp : result.modelComparisons) {
        out << "| " << comp.model << " | " << comp.bestFor.substr(0, 40)
            << "... | " << comp.computationalCost << "x |\n";
    }

    out << "\n**Questions to Consider:**\n";
    for (const auto& question : result.socraticQuestions) {
        out << "- " << question << "\n";
    }

    return out.str();
}

std::string TurbulentFlowTool::formatRecommendations(
    const TurbulentFlowAnalysisResult& result) const {

    std::ostringstream out;

    out << "\n**Practical Recommendations:**\n\n";

    for (const auto& tip : result.practicalTips) {
        out << "- " << tip << "\n";
    }

    if (!result.wallFunctionStatus.recommendations.empty()) {
        out << "\n**Mesh Recommendations:**\n";
        for (const auto& rec : result.wallFunctionStatus.recommendations) {
            out << "- " << rec << "\n";
        }
    }

    out << "\n**References:**\n";
    for (const auto& ref : result.references) {
        out << "- " << ref << "\n";
    }

    return out.str();
}

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerTurbulentFlowTool(McpServer& server) {
    auto tool = std::make_shared<TurbulentFlowTool>();

    server.registerTool(
        TurbulentFlowTool::getName(),
        TurbulentFlowTool::getDescription(),
        TurbulentFlowTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult { return tool->execute(arguments); });
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //
