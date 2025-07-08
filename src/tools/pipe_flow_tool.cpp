/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Pipe Flow MCP Tool
    \\  /    A nd           | 
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for pipe flow analysis implementation

\*---------------------------------------------------------------------------*/

#include "pipe_flow_tool.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                        PipeFlowTool Implementation
\*---------------------------------------------------------------------------*/

PipeFlowTool::PipeFlowTool()
: analyzer_(std::make_unique<PipeFlowAnalyzer>())
{
}

PipeFlowTool::PipeFlowTool(std::unique_ptr<PipeFlowAnalyzer> analyzer)
: analyzer_(std::move(analyzer))
{
}

ToolResult PipeFlowTool::execute(const json& arguments)
{
    ToolResult result;
    
    try
    {
        PipeFlowInput input = PipeFlowAnalyzer::parseInput(arguments);
        
        PipeFlowResults analysisResults = analyzer_->analyze(input);
        
        std::string formattedResults = formatResultsForUser(analysisResults);
        result.addTextContent(formattedResults);
        
        if (analysisResults.success)
        {
            std::string physicsExplanation = generatePhysicsExplanation(input, analysisResults);
            result.addTextContent(physicsExplanation);
            
            std::string recommendations = generateRecommendations(input, analysisResults);
            result.addTextContent(recommendations);
        }
        else
        {
            result.addTextContent("⚠️  OpenFOAM simulation failed, but theoretical calculations were provided.");
            result.addTextContent("Error details: " + analysisResults.errorMessage);
        }
        
        json resultsJson = PipeFlowAnalyzer::resultsToJson(analysisResults);
        result.content.push_back(json{
            {"type", "resource"},
            {"resource", {
                {"uri", "openfoam://pipe_flow/" + analysisResults.caseId},
                {"name", "Pipe Flow Results"},
                {"description", "Complete OpenFOAM pipe flow analysis results"},
                {"mimeType", "application/json"}
            }},
            {"text", resultsJson.dump(2)}
        });
        
    }
    catch (const std::exception& e)
    {
        result.addErrorContent("Error executing pipe flow analysis: " + std::string(e.what()));
    }
    
    return result;
}

std::string PipeFlowTool::formatResultsForUser(const PipeFlowResults& results) const
{
    std::ostringstream output;
    
    output << "🔬 **OpenFOAM Pipe Flow Analysis Results**\n\n";
    
    output << "**Flow Characteristics:**\n";
    output << "• Reynolds Number: " << std::fixed << std::setprecision(0) << results.reynoldsNumber << "\n";
    output << "• Flow Regime: " << results.flowRegime << " flow\n";
    output << "• Average Velocity: " << std::fixed << std::setprecision(2) << results.averageVelocity << " m/s\n";
    output << "• Maximum Velocity: " << std::fixed << std::setprecision(2) << results.maxVelocity << " m/s\n\n";
    
    output << "**Pressure and Friction:**\n";
    output << "• Friction Factor: " << std::fixed << std::setprecision(4) << results.frictionFactor << "\n";
    output << "• Pressure Drop: " << std::fixed << std::setprecision(2) << results.pressureDrop << " Pa";
    if (results.pressureDrop > 1000)
    {
        output << " (" << std::fixed << std::setprecision(1) << results.pressureDrop/1000 << " kPa)";
    }
    output << "\n";
    output << "• Wall Shear Stress: " << std::fixed << std::setprecision(3) << results.wallShearStress << " Pa\n\n";
    
    if (results.success)
    {
        output << "✅ **OpenFOAM Simulation Status:** Completed successfully\n";
        output << "📁 **Case ID:** " << results.caseId << "\n";
    }
    else
    {
        output << "❌ **OpenFOAM Simulation Status:** Failed - using theoretical calculations\n";
    }
    
    return output.str();
}

std::string PipeFlowTool::generatePhysicsExplanation(const PipeFlowInput& input, const PipeFlowResults& results) const
{
    std::ostringstream explanation;
    
    explanation << "📚 **Physics Explanation:**\n\n";
    
    if (input.isLaminar())
    {
        explanation << "**Laminar Flow (Re < 2300):**\n";
        explanation << "• Smooth, orderly flow with parabolic velocity profile\n";
        explanation << "• Viscous forces dominate over inertial forces\n";
        explanation << "• Maximum velocity = 2 × average velocity (theoretical)\n";
        explanation << "• Friction factor = 64/Re (Hagen-Poiseuille equation)\n";
        explanation << "• Pressure drop varies linearly with velocity\n";
    }
    else if (input.isTurbulent())
    {
        explanation << "**Turbulent Flow (Re > 4000):**\n";
        explanation << "• Chaotic, mixing flow with flatter velocity profile\n";
        explanation << "• Inertial forces dominate over viscous forces\n";
        explanation << "• Maximum velocity ≈ 1.2 × average velocity (approximate)\n";
        explanation << "• Friction factor = 0.316/Re^0.25 (Blasius equation)\n";
        explanation << "• Pressure drop varies with velocity squared\n";
    }
    else
    {
        explanation << "**Transitional Flow (2300 < Re < 4000):**\n";
        explanation << "• Unstable regime between laminar and turbulent\n";
        explanation << "• Flow may switch between regimes\n";
        explanation << "• Difficult to predict exact behavior\n";
        explanation << "• Industrial applications typically assume turbulent\n";
    }
    
    explanation << "\n**Key Dimensionless Numbers:**\n";
    explanation << "• Reynolds Number: Re = ρVD/μ = " << std::fixed << std::setprecision(0) << results.reynoldsNumber << "\n";
    explanation << "  - Represents ratio of inertial to viscous forces\n";
    explanation << "  - Critical for determining flow regime\n";
    
    return explanation.str();
}

std::string PipeFlowTool::generateRecommendations(const PipeFlowInput& input, const PipeFlowResults& results) const
{
    std::ostringstream recommendations;
    
    recommendations << "💡 **Engineering Recommendations:**\n\n";
    
    if (results.pressureDrop > 10000)
    {
        recommendations << "⚠️  **High Pressure Drop Warning:**\n";
        recommendations << "• Consider increasing pipe diameter\n";
        recommendations << "• Evaluate pump requirements\n";
        recommendations << "• Check for excessive energy costs\n\n";
    }
    
    if (input.isLaminar())
    {
        recommendations << "**Laminar Flow Considerations:**\n";
        recommendations << "• Excellent for precise flow control\n";
        recommendations << "• Minimal mixing - poor for heat/mass transfer\n";
        recommendations << "• Sensitive to pipe entrance effects\n";
        recommendations << "• Consider L/D > 60 for fully developed flow\n";
    }
    else if (input.isTurbulent())
    {
        recommendations << "**Turbulent Flow Considerations:**\n";
        recommendations << "• Enhanced mixing for heat/mass transfer\n";
        recommendations << "• Higher pressure losses than laminar\n";
        recommendations << "• Surface roughness becomes important\n";
        recommendations << "• Consider L/D > 10 for fully developed flow\n";
    }
    else
    {
        recommendations << "**Transitional Flow Considerations:**\n";
        recommendations << "• Unstable flow regime - avoid if possible\n";
        recommendations << "• Consider design changes to achieve Re > 4000\n";
        recommendations << "• If unavoidable, use turbulent correlations for safety\n";
    }
    
    recommendations << "\n**Optimization Suggestions:**\n";
    
    if (results.pressureDrop > 5000)
    {
        double newDiameter = input.diameter * std::pow(results.pressureDrop / 2000, 0.2);
        recommendations << "• To reduce pressure drop to ~2 kPa, increase diameter to " 
                         << std::fixed << std::setprecision(3) << newDiameter << " m\n";
    }
    
    if (input.velocity > 20)
    {
        recommendations << "• High velocity may cause erosion - consider velocity < 20 m/s\n";
    }
    
    if (input.velocity < 1)
    {
        recommendations << "• Low velocity may cause settling - consider velocity > 1 m/s\n";
    }
    
    return recommendations.str();
}

void PipeFlowTool::setWorkingDirectory(const std::string& workingDir)
{
    analyzer_->setWorkingDirectory(workingDir);
}

std::vector<std::string> PipeFlowTool::listActiveCases() const
{
    return analyzer_->listActiveCases();
}

bool PipeFlowTool::deleteCaseData(const std::string& caseId)
{
    return analyzer_->deleteCaseData(caseId);
}

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerPipeFlowTool(McpServer& server)
{
    auto tool = std::make_shared<PipeFlowTool>();
    
    server.registerTool(
        PipeFlowTool::getName(),
        PipeFlowTool::getDescription(),
        PipeFlowTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult
        {
            return tool->execute(arguments);
        }
    );
}

} // End namespace MCP
} // End namespace Foam

// ************************************************************************* //