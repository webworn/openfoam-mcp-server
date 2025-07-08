/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | External Flow MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for external flow analysis implementation

\*---------------------------------------------------------------------------*/

#include "external_flow_tool.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                        ExternalFlowTool Implementation
\*---------------------------------------------------------------------------*/

ExternalFlowTool::ExternalFlowTool() : analyzer_(std::make_unique<ExternalFlowAnalyzer>()) {}

ExternalFlowTool::ExternalFlowTool(std::unique_ptr<ExternalFlowAnalyzer> analyzer)
    : analyzer_(std::move(analyzer))
{}

ToolResult ExternalFlowTool::execute(const json& arguments)
{
    ToolResult result;

    try {
        ExternalFlowInput input = ExternalFlowAnalyzer::parseInput(arguments);

        ExternalFlowResults analysisResults = analyzer_->analyze(input);

        std::string formattedResults = formatResultsForUser(analysisResults);
        result.addTextContent(formattedResults);

        if (analysisResults.success) {
            std::string physicsExplanation = generatePhysicsExplanation(input, analysisResults);
            result.addTextContent(physicsExplanation);

            std::string recommendations = generateRecommendations(input, analysisResults);
            result.addTextContent(recommendations);

            std::string applicationGuidance = generateApplicationGuidance(input, analysisResults);
            result.addTextContent(applicationGuidance);
        } else {
            result.addTextContent(
                "⚠️  OpenFOAM simulation failed, but theoretical calculations were provided.");
            result.addTextContent("Error details: " + analysisResults.errorMessage);
        }

        json resultsJson = ExternalFlowAnalyzer::resultsToJson(analysisResults);
        result.content.push_back(json{
            {"type",     "resource"            },
            {"resource",
             {{"uri", "openfoam://external_flow/" + analysisResults.caseId},
              {"name", "External Flow Analysis Results"},
              {"description", "Complete OpenFOAM external flow aerodynamics analysis results"},
              {"mimeType", "application/json"}}},
            {"text",     resultsJson.dump(2)   }
        });

    } catch (const std::exception& e) {
        result.addErrorContent("Error executing external flow analysis: " + std::string(e.what()));
    }

    return result;
}

std::string ExternalFlowTool::formatResultsForUser(const ExternalFlowResults& results) const
{
    std::ostringstream output;

    output << "🚗 **OpenFOAM External Flow Analysis Results**\n\n";

    output << "**Flow Characteristics:**\n";
    output << "• Reynolds Number: " << std::fixed << std::setprecision(0) << results.reynoldsNumber
           << "\n";
    output << "• Mach Number: " << std::fixed << std::setprecision(3) << results.machNumber << "\n";
    output << "• Flow Regime: " << results.flowRegime << " flow\n";
    output << "• Compressibility: " << results.compressibilityRegime << "\n\n";

    output << "**Aerodynamic Coefficients:**\n";
    output << "• Drag Coefficient (Cd): " << std::fixed << std::setprecision(3)
           << results.dragCoefficient << "\n";
    if (results.liftCoefficient != 0) {
        output << "• Lift Coefficient (Cl): " << std::fixed << std::setprecision(3)
               << results.liftCoefficient << "\n";
    }
    output << "• Skin Friction Coefficient: " << std::fixed << std::setprecision(4)
           << results.skinFrictionCoefficient << "\n\n";

    output << "**Aerodynamic Forces:**\n";
    output << "• Drag Force: " << std::fixed << std::setprecision(1) << results.dragForce << " N";
    if (results.dragForce > 1000) {
        output << " (" << std::fixed << std::setprecision(2) << results.dragForce / 1000 << " kN)";
    }
    output << "\n";

    if (results.liftForce != 0) {
        output << "• Lift Force: " << std::fixed << std::setprecision(1) << results.liftForce
               << " N";
        if (results.liftForce > 1000) {
            output << " (" << std::fixed << std::setprecision(2) << results.liftForce / 1000
                   << " kN)";
        }
        output << "\n";
    }

    if (results.success) {
        output << "\n✅ **OpenFOAM Simulation Status:** Completed successfully\n";
        output << "📁 **Case ID:** " << results.caseId << "\n";
    } else {
        output << "\n❌ **OpenFOAM Simulation Status:** Failed - using theoretical calculations\n";
    }

    return output.str();
}

std::string ExternalFlowTool::generatePhysicsExplanation(const ExternalFlowInput& input,
                                                         const ExternalFlowResults& results) const
{
    std::ostringstream explanation;

    explanation << "📚 **Aerodynamics Physics Explanation:**\n\n";

    // Flow regime explanation
    if (results.flowRegime == "laminar") {
        explanation << "**Laminar External Flow (Re < 500,000):**\n";
        explanation << "• Smooth, orderly flow with minimal mixing\n";
        explanation << "• Lower skin friction but potential for early separation\n";
        explanation << "• More predictable but often unstable in real conditions\n";
    } else if (results.flowRegime == "turbulent") {
        explanation << "**Turbulent External Flow (Re > 1,000,000):**\n";
        explanation << "• Chaotic, mixing flow with enhanced momentum transfer\n";
        explanation << "• Higher skin friction but delayed separation\n";
        explanation << "• More stable flow attachment over curved surfaces\n";
    } else {
        explanation << "**Transitional External Flow (500,000 < Re < 1,000,000):**\n";
        explanation << "• Mixed regime with regions of laminar and turbulent flow\n";
        explanation << "• Critical for boundary layer transition location\n";
        explanation << "• Highly sensitive to surface roughness and disturbances\n";
    }

    explanation << "\n**Compressibility Effects:**\n";
    if (results.compressibilityRegime == "incompressible") {
        explanation << "• Ma < 0.3: Air density remains constant\n";
        explanation << "• Standard automotive/civil engineering analysis\n";
        explanation << "• Pressure changes don't affect fluid properties\n";
    } else if (results.compressibilityRegime == "subsonic_compressible") {
        explanation << "• 0.3 < Ma < 0.8: Density variations become important\n";
        explanation << "• High-speed automotive or low-speed aircraft analysis\n";
        explanation << "• Requires compressible flow formulation\n";
    }

    explanation << "\n**Key Dimensionless Numbers:**\n";
    explanation << "• Reynolds Number: Re = ρVL/μ = " << std::fixed << std::setprecision(0)
                << results.reynoldsNumber << "\n";
    explanation << "  - Represents ratio of inertial to viscous forces\n";
    explanation << "  - Critical for boundary layer behavior and separation\n";
    explanation << "• Mach Number: Ma = V/a = " << std::fixed << std::setprecision(3)
                << results.machNumber << "\n";
    explanation << "  - Represents ratio of flow speed to sound speed\n";
    explanation << "  - Determines compressibility importance\n";

    return explanation.str();
}

std::string ExternalFlowTool::generateRecommendations(const ExternalFlowInput& input,
                                                      const ExternalFlowResults& results) const
{
    std::ostringstream recommendations;

    recommendations << "💡 **Engineering Recommendations:**\n\n";

    // Vehicle-specific recommendations
    if (input.vehicleType == "car") {
        recommendations << "**Automotive Aerodynamics:**\n";
        if (results.dragCoefficient > 0.4) {
            recommendations << "⚠️  **High Drag Coefficient (Cd > 0.4)**\n";
            recommendations << "• Consider body streamlining and rear spoiler\n";
            recommendations << "• Check for flow separation at rear window\n";
            recommendations << "• Reduce frontal area if possible\n";
        } else if (results.dragCoefficient < 0.25) {
            recommendations << "✅ **Excellent Aerodynamic Performance (Cd < 0.25)**\n";
            recommendations << "• Performance comparable to modern efficient vehicles\n";
            recommendations << "• Focus on maintaining laminar flow regions\n";
        }

        recommendations << "• Fuel consumption impact: ΔCd = 0.01 ≈ 0.2% fuel increase\n";
        recommendations << "• At highway speeds, aerodynamic drag dominates total resistance\n";
    } else if (input.vehicleType == "aircraft") {
        recommendations << "**Aircraft Aerodynamics:**\n";
        if (results.dragCoefficient > 0.02) {
            recommendations << "⚠️  **High Aircraft Drag (Cd > 0.02)**\n";
            recommendations << "• Check airfoil shape optimization\n";
            recommendations << "• Consider laminar flow maintenance\n";
            recommendations << "• Evaluate surface roughness effects\n";
        }

        recommendations << "• Lift-to-drag ratio critical for performance\n";
        recommendations << "• Consider wing aspect ratio optimization\n";
        recommendations << "• Boundary layer transition location affects performance\n";
    } else if (input.vehicleType == "building") {
        recommendations << "**Civil Engineering - Wind Loads:**\n";
        recommendations << "• Peak pressure coefficient locations critical for structural design\n";
        recommendations << "• Consider pedestrian-level wind comfort\n";
        recommendations << "• Vortex shedding may cause vibrations\n";

        if (results.dragCoefficient > 1.5) {
            recommendations << "⚠️  **High Wind Load (Cd > 1.5)**\n";
            recommendations << "• Consider building shape modifications\n";
            recommendations << "• Check structural design wind loads\n";
        }
    }

    // General flow recommendations
    recommendations << "\n**CFD Mesh Recommendations:**\n";
    if (results.reynoldsNumber > 1e6) {
        recommendations << "• Use y+ < 1 for accurate boundary layer resolution\n";
        recommendations << "• Consider wall functions for y+ = 30-300 (engineering approach)\n";
        recommendations << "• Minimum 10 boundary layer cells recommended\n";
    } else {
        recommendations << "• Lower Reynolds number allows coarser near-wall mesh\n";
        recommendations << "• y+ < 5 adequate for this flow regime\n";
    }

    recommendations << "\n**Solver Recommendations:**\n";
    if (results.compressibilityRegime == "incompressible") {
        recommendations << "• SIMPLE/PISO algorithms appropriate\n";
        recommendations << "• Steady-state analysis suitable\n";
    } else {
        recommendations << "• Consider compressible solver (rhoPimpleFoam)\n";
        recommendations << "• Density-based methods may be more robust\n";
    }

    return recommendations.str();
}

std::string ExternalFlowTool::generateApplicationGuidance(const ExternalFlowInput& input,
                                                          const ExternalFlowResults& results) const
{
    std::ostringstream guidance;

    guidance << "🎯 **Application-Specific Guidance:**\n\n";

    if (input.vehicleType == "car") {
        guidance << "**Automotive Development Applications:**\n";
        guidance << "• **Fuel Economy**: Current Cd = " << std::fixed << std::setprecision(3)
                 << results.dragCoefficient << " (Target: < 0.30 for efficiency)\n";
        guidance << "• **Performance**: Drag force = " << std::fixed << std::setprecision(0)
                 << results.dragForce << " N at " << input.velocity << " m/s\n";
        guidance << "• **Power Required**: ~" << std::fixed << std::setprecision(1)
                 << (results.dragForce * input.velocity / 1000)
                 << " kW to overcome aerodynamic drag\n";

        guidance << "\n**Design Optimization Targets:**\n";
        guidance << "• Sports car: Cd = 0.25-0.35, focus on cooling airflow\n";
        guidance << "• Sedan: Cd = 0.25-0.30, balance efficiency and stability\n";
        guidance << "• SUV/Truck: Cd = 0.30-0.40, consider underbody airflow\n";
    } else if (input.vehicleType == "aircraft") {
        guidance << "**Aerospace Applications:**\n";
        guidance << "• **Flight Performance**: Drag directly affects range and fuel consumption\n";
        guidance << "• **Cruise Efficiency**: L/D ratio optimization critical\n";
        guidance << "• **Design Point**: Re = " << std::fixed << std::setprecision(0)
                 << results.reynoldsNumber << " typical for cruise conditions\n";

        guidance << "\n**Aircraft Design Considerations:**\n";
        guidance << "• Maintain laminar flow as long as possible\n";
        guidance << "• Consider compressibility effects for Ma > 0.3\n";
        guidance << "• Surface finish critical for boundary layer transition\n";
    } else if (input.vehicleType == "building") {
        guidance << "**Civil Engineering Applications:**\n";
        guidance << "• **Structural Design**: Peak wind loads for structural analysis\n";
        guidance << "• **Pedestrian Comfort**: Check ground-level wind speeds\n";
        guidance << "• **HVAC Design**: Natural ventilation and pressure distributions\n";

        guidance << "\n**Wind Engineering Standards:**\n";
        guidance << "• ASCE 7: Minimum design wind speeds and pressure coefficients\n";
        guidance << "• Consider building height effects on wind profile\n";
        guidance << "• Urban environment affects approaching flow\n";
    }

    guidance << "\n**Next Steps:**\n";
    guidance << "• **Detailed Analysis**: Run full 3D CFD with actual geometry\n";
    guidance << "• **Validation**: Compare with wind tunnel or experimental data\n";
    guidance << "• **Optimization**: Parametric studies for shape optimization\n";
    guidance << "• **Verification**: Grid independence and solver validation\n";

    return guidance.str();
}

void ExternalFlowTool::setWorkingDirectory(const std::string& workingDir)
{
    analyzer_->setWorkingDirectory(workingDir);
}

std::vector<std::string> ExternalFlowTool::listActiveCases() const
{
    return analyzer_->listActiveCases();
}

bool ExternalFlowTool::deleteCaseData(const std::string& caseId)
{
    return analyzer_->deleteCaseData(caseId);
}

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerExternalFlowTool(McpServer& server)
{
    auto tool = std::make_shared<ExternalFlowTool>();

    server.registerTool(
        ExternalFlowTool::getName(),
        ExternalFlowTool::getDescription(),
        ExternalFlowTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult { return tool->execute(arguments); });
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //