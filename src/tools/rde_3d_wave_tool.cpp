/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | RDE 3D Wave Analyzer MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for 3D RDE wave propagation analysis implementation.

\*---------------------------------------------------------------------------*/

#include "rde_3d_wave_tool.hpp"

#include <iomanip>
#include <sstream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        RDE3DWaveTool Implementation
\*---------------------------------------------------------------------------*/

RDE3DWaveTool::RDE3DWaveTool()
    : analyzer_(std::make_unique<RDE3DWaveAnalyzer>()) {}

RDE3DWaveTool::RDE3DWaveTool(std::unique_ptr<RDE3DWaveAnalyzer> analyzer)
    : analyzer_(std::move(analyzer)) {}

ToolResult RDE3DWaveTool::execute(const json& arguments) {
    ToolResult result;

    try {
        std::string caseDirectory = arguments.at("case_directory").get<std::string>();

        // Build analysis request
        RDE3DWaveAnalyzer::Wave3DAnalysisRequest request;
        request.caseDirectory = caseDirectory;

        // Parse detection thresholds
        if (arguments.contains("detection_thresholds")) {
            auto thresholds = arguments["detection_thresholds"];
            request.temperatureThreshold = thresholds.value("temperature", 2500.0);
            request.pressureThreshold = thresholds.value("pressure", 1.0e6);
            request.velocityThreshold = thresholds.value("velocity", 1500.0);
        } else {
            request.temperatureThreshold = 2500.0;
            request.pressureThreshold = 1.0e6;
            request.velocityThreshold = 1500.0;
        }

        // Parse time range
        if (arguments.contains("time_range")) {
            auto timeRange = arguments["time_range"];
            request.analysisStartTime = timeRange.value("start", 0.0);
            request.analysisEndTime = timeRange.value("end", 1.0e-3);
            request.timeStepInterval = timeRange.value("interval", 1.0e-6);
        } else {
            request.analysisStartTime = 0.0;
            request.analysisEndTime = 1.0e-3;
            request.timeStepInterval = 1.0e-6;
        }

        // Parse analysis options
        if (arguments.contains("analysis_options")) {
            auto options = arguments["analysis_options"];
            request.analyzeAxialPropagation = options.value("analyze_axial_propagation", true);
            request.analyze3DInteractions = options.value("analyze_3d_interactions", true);
            request.calculatePerformanceMetrics = options.value("calculate_performance_metrics", true);
            request.generateVisualization = options.value("generate_visualization", false);
        } else {
            request.analyzeAxialPropagation = true;
            request.analyze3DInteractions = true;
            request.calculatePerformanceMetrics = true;
            request.generateVisualization = false;
        }

        // Set validation targets from 2D baseline
        request.targetThrust = 11519.0;
        request.targetIsp = 1629.0;
        request.targetEfficiency = 85.0;
        request.generateEducationalContent = true;

        // Perform 3D wave analysis
        RDE3DWaveAnalyzer::Wave3DAnalysisResult analysisResult = analyzer_->analyze3DWaves(request);

        // Format results for user
        std::string formattedResults = formatResultsForUser(analysisResult);
        result.addTextContent(formattedResults);

        // Add educational content
        std::string educational = generateEducationalContent(analysisResult);
        result.addTextContent(educational);

        // Add performance summary
        std::string performance = generatePerformanceSummary(analysisResult);
        result.addTextContent(performance);

        // Add JSON resource with detailed data
        json analysisJson;
        analysisJson["success"] = analysisResult.success;
        analysisJson["analysis_directory"] = analysisResult.analysisDirectory;
        analysisJson["time_snapshots"] = analysisResult.timeHistory.size();
        analysisJson["average_wave_count_3d"] = analysisResult.averageWaveCount3D;
        analysisJson["wave_count_std_dev"] = analysisResult.waveCountStdDev3D;
        analysisJson["average_wave_spacing_3d"] = analysisResult.averageWaveSpacing3D;
        analysisJson["total_collisions_3d"] = analysisResult.totalCollisions3D;
        analysisJson["collision_frequency_3d"] = analysisResult.collisionFrequency3D;
        analysisJson["average_collision_enhancement"] = analysisResult.averageCollisionEnhancement3D;

        // Performance metrics
        json perfJson;
        perfJson["thrust_3d"] = analysisResult.performanceMetrics3D.thrust3D;
        perfJson["specific_impulse_3d"] = analysisResult.performanceMetrics3D.specificImpulse3D;
        perfJson["efficiency_3d"] = analysisResult.performanceMetrics3D.efficiency3D;
        perfJson["axial_thrust_ratio"] = analysisResult.performanceMetrics3D.axialThrustRatio;
        perfJson["radial_thrust_loss"] = analysisResult.performanceMetrics3D.radialThrustLoss;
        perfJson["multi_wave_advantage_3d"] = analysisResult.performanceMetrics3D.multiWaveAdvantage3D;
        perfJson["performance_score_3d"] = analysisResult.performanceMetrics3D.performanceScore3D;
        perfJson["performance_rating_3d"] = analysisResult.performanceMetrics3D.performanceRating3D;
        analysisJson["performance_metrics"] = perfJson;

        analysisJson["performance_3d_to_2d_ratio"] = analysisResult.performance3Dto2DRatio;
        analysisJson["advantages_3d"] = analysisResult.advantages3D;
        analysisJson["recommendations_3d"] = analysisResult.recommendations3D;

        result.content.push_back(
            json{{"type", "resource"},
                 {"resource",
                  {{"uri", "openfoam://rde_3d_wave_analysis/" + caseDirectory},
                   {"name", "3D RDE Wave Analysis Results"},
                   {"description", "Complete 3D RDE wave propagation analysis"},
                   {"mimeType", "application/json"}}},
                 {"text", analysisJson.dump(2)}});

    } catch (const std::exception& e) {
        result.addErrorContent("Error executing 3D RDE wave analysis: " + std::string(e.what()));
    }

    return result;
}

std::string RDE3DWaveTool::formatResultsForUser(const RDE3DWaveAnalyzer::Wave3DAnalysisResult& result) const {
    std::ostringstream output;

    output << "**3D RDE Wave Propagation Analysis Results**\n\n";
    output << "**Status:** " << (result.success ? "Success" : "Failed") << "\n";
    output << "**Analysis Directory:** " << result.analysisDirectory << "\n\n";

    output << "**Wave System Statistics:**\n";
    output << "- Average Wave Count: " << std::fixed << std::setprecision(2) << result.averageWaveCount3D
           << " +/- " << result.waveCountStdDev3D << "\n";
    output << "- Average Wave Spacing: " << std::fixed << std::setprecision(3) << result.averageWaveSpacing3D
           << " +/- " << result.waveSpacingStdDev3D << " rad\n";
    output << "- Time Snapshots Analyzed: " << result.timeHistory.size() << "\n\n";

    output << "**3D Wave Interactions:**\n";
    output << "- Total Collisions Detected: " << result.totalCollisions3D << "\n";
    output << "- Collision Frequency: " << std::fixed << std::setprecision(0) << result.collisionFrequency3D << " Hz\n";
    output << "- Average Collision Enhancement: " << std::fixed << std::setprecision(1)
           << result.averageCollisionEnhancement3D << "%\n\n";

    output << "**3D Performance Metrics:**\n";
    output << "- Thrust (3D): " << std::fixed << std::setprecision(0) << result.performanceMetrics3D.thrust3D << " N\n";
    output << "- Specific Impulse (3D): " << std::fixed << std::setprecision(0) << result.performanceMetrics3D.specificImpulse3D << " s\n";
    output << "- Combustion Efficiency (3D): " << std::fixed << std::setprecision(1) << result.performanceMetrics3D.efficiency3D << "%\n";
    output << "- Axial Thrust Ratio: " << std::fixed << std::setprecision(2) << result.performanceMetrics3D.axialThrustRatio << "\n";
    output << "- Radial Thrust Loss: " << std::fixed << std::setprecision(1) << result.performanceMetrics3D.radialThrustLoss << "%\n";
    output << "- Multi-Wave Advantage (3D): " << std::fixed << std::setprecision(1) << result.performanceMetrics3D.multiWaveAdvantage3D << "%\n";
    output << "- Performance Rating: " << result.performanceMetrics3D.performanceRating3D
           << " (Score: " << std::fixed << std::setprecision(1) << result.performanceMetrics3D.performanceScore3D << "/100)\n";

    return output.str();
}

std::string RDE3DWaveTool::generateEducationalContent(const RDE3DWaveAnalyzer::Wave3DAnalysisResult& result) const {
    std::ostringstream content;

    content << "\n**Understanding 3D RDE Wave Physics:**\n\n";
    content << result.wave3DPhysicsExplanation << "\n\n";

    content << "**Detonation Theory (3D):**\n";
    content << result.detonation3DTheory << "\n\n";

    content << "**3D Wave Interaction Analysis:**\n";
    content << result.wave3DInteractionAnalysis << "\n\n";

    content << "**Key 3D Physics Principles:**\n";
    content << "- 3D detonation waves have axial propagation components affecting thrust vectoring\n";
    content << "- Wave front curvature in 3D affects local pressure gain and combustion efficiency\n";
    content << "- Nozzle expansion adds thrust contribution not captured in 2D analysis\n";
    content << "- Radial velocity components cause thrust losses that must be minimized\n";

    return content.str();
}

std::string RDE3DWaveTool::generatePerformanceSummary(const RDE3DWaveAnalyzer::Wave3DAnalysisResult& result) const {
    std::ostringstream summary;

    summary << "\n**Performance Correlation:**\n\n";
    summary << result.performance3DCorrelation << "\n\n";

    summary << "**Validation Against 2D Baseline:**\n";
    summary << result.validationAgainst2D << "\n";
    summary << "- 3D/2D Performance Ratio: " << std::fixed << std::setprecision(3) << result.performance3Dto2DRatio << "\n\n";

    summary << "**3D Advantages Identified:**\n";
    for (const auto& advantage : result.advantages3D) {
        summary << "- " << advantage << "\n";
    }

    summary << "\n**Optimization Recommendations:**\n";
    for (const auto& recommendation : result.recommendations3D) {
        summary << "- " << recommendation << "\n";
    }

    summary << "\n**Optimization Guidance:**\n";
    summary << result.optimization3DGuidance << "\n";

    return summary.str();
}

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerRDE3DWaveTool(McpServer& server) {
    auto tool = std::make_shared<RDE3DWaveTool>();

    server.registerTool(
        RDE3DWaveTool::getName(),
        RDE3DWaveTool::getDescription(),
        RDE3DWaveTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult { return tool->execute(arguments); });
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //
