/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | RDE 2D Wave Analyzer MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for 2D RDE wave analysis implementation.

\*---------------------------------------------------------------------------*/

#include "rde_wave_tool.hpp"

#include <iomanip>
#include <sstream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        RDEWaveTool Implementation
\*---------------------------------------------------------------------------*/

RDEWaveTool::RDEWaveTool()
    : analyzer_(std::make_unique<RDE2DWaveAnalyzer>()) {}

RDEWaveTool::RDEWaveTool(std::unique_ptr<RDE2DWaveAnalyzer> analyzer)
    : analyzer_(std::move(analyzer)) {}

ToolResult RDEWaveTool::execute(const json& arguments) {
    ToolResult result;

    try {
        std::string caseDirectory = arguments.at("case_directory").get<std::string>();
        double temperatureThreshold = arguments.value("temperature_threshold", 2500.0);

        // Perform wave analysis
        RDE2DWaveAnalyzer::WaveAnalysisResult analysisResult = analyzer_->analyzeWaveInteractions(caseDirectory);

        // Format results for user
        std::string formattedResults = formatResultsForUser(analysisResult);
        result.addTextContent(formattedResults);

        // Add educational content
        std::string educational = generateEducationalContent(analysisResult);
        result.addTextContent(educational);

        // Add performance assessment
        std::string performance = generatePerformanceAssessment(analysisResult);
        result.addTextContent(performance);

        // Add JSON resource with detailed data
        json analysisJson;
        analysisJson["case_directory"] = analysisResult.caseDirectory;
        analysisJson["avg_wave_count"] = analysisResult.avgWaveCount;
        analysisJson["avg_wave_speed"] = analysisResult.avgWaveSpeed;
        analysisJson["system_stability"] = analysisResult.systemStability;
        analysisJson["combustion_efficiency"] = analysisResult.combustionEfficiency;
        analysisJson["specific_impulse"] = analysisResult.specificImpulse;
        analysisJson["thrust"] = analysisResult.thrust;
        analysisJson["collision_count"] = analysisResult.collisions.size();
        analysisJson["time_snapshots"] = analysisResult.timeHistory.size();

        result.content.push_back(
            json{{"type", "resource"},
                 {"resource",
                  {{"uri", "openfoam://rde_wave_analysis/" + caseDirectory},
                   {"name", "RDE Wave Analysis Results"},
                   {"description", "Complete 2D RDE wave interaction analysis"},
                   {"mimeType", "application/json"}}},
                 {"text", analysisJson.dump(2)}});

    } catch (const std::exception& e) {
        result.addErrorContent("Error executing RDE wave analysis: " + std::string(e.what()));
    }

    return result;
}

std::string RDEWaveTool::formatResultsForUser(const RDE2DWaveAnalyzer::WaveAnalysisResult& result) const {
    std::ostringstream output;

    output << "**2D RDE Wave Analysis Results**\n\n";
    output << "**Case:** " << result.caseDirectory << "\n\n";

    output << "**Wave System Overview:**\n";
    output << "- Average Wave Count: " << std::fixed << std::setprecision(1) << result.avgWaveCount << "\n";
    output << "- Average Wave Speed: " << std::fixed << std::setprecision(0) << result.avgWaveSpeed << " m/s\n";
    output << "- System Stability: " << std::fixed << std::setprecision(2) << result.systemStability * 100 << "%\n";
    output << "- Total Collisions Detected: " << result.collisions.size() << "\n\n";

    output << "**Performance Metrics:**\n";
    output << "- Thrust: " << std::fixed << std::setprecision(0) << result.thrust << " N\n";
    output << "- Specific Impulse: " << std::fixed << std::setprecision(0) << result.specificImpulse << " s\n";
    output << "- Combustion Efficiency: " << std::fixed << std::setprecision(1) << result.combustionEfficiency * 100 << "%\n\n";

    if (!result.timeHistory.empty()) {
        output << "**Time History Summary:**\n";
        output << "- Analysis Snapshots: " << result.timeHistory.size() << "\n";

        // Find time range
        double startTime = result.timeHistory.front().time;
        double endTime = result.timeHistory.back().time;
        output << "- Time Range: " << std::scientific << std::setprecision(3) << startTime
               << " to " << endTime << " s\n\n";

        // Wave count statistics
        int minWaves = 999, maxWaves = 0;
        for (const auto& snapshot : result.timeHistory) {
            minWaves = std::min(minWaves, snapshot.activeWaveCount);
            maxWaves = std::max(maxWaves, snapshot.activeWaveCount);
        }
        output << "- Wave Count Range: " << minWaves << " to " << maxWaves << "\n";
    }

    return output.str();
}

std::string RDEWaveTool::generateEducationalContent(const RDE2DWaveAnalyzer::WaveAnalysisResult& result) const {
    std::ostringstream content;

    content << "\n**Understanding RDE Wave Physics:**\n\n";
    content << result.wavePhysicsExplanation << "\n\n";

    if (!result.collisions.empty()) {
        content << "**Wave Collision Analysis:**\n";
        content << result.collisionAnalysis << "\n\n";
    }

    content << "**Key Physics Principles:**\n";
    content << "- Detonation waves propagate circumferentially in the annular combustor\n";
    content << "- Multi-wave systems can achieve higher performance through wave interactions\n";
    content << "- Wave collisions can enhance local combustion and pressure gain\n";
    content << "- Optimal wave count depends on combustor geometry and operating conditions\n";

    return content.str();
}

std::string RDEWaveTool::generatePerformanceAssessment(const RDE2DWaveAnalyzer::WaveAnalysisResult& result) const {
    std::ostringstream assessment;

    assessment << "\n**Performance Assessment:**\n\n";
    assessment << result.performanceAssessment << "\n\n";

    assessment << "**Design Recommendations:**\n";
    for (const auto& recommendation : result.designRecommendations) {
        assessment << "- " << recommendation << "\n";
    }

    if (!result.safetyConsiderations.empty()) {
        assessment << "\n**Safety Considerations:**\n";
        assessment << result.safetyConsiderations << "\n";
    }

    // Comparison to validated baseline
    assessment << "\n**Validation Reference:**\n";
    assessment << "- 2D Validated Thrust: 11,519 N\n";
    assessment << "- 2D Validated Isp: 1,629 s\n";
    assessment << "- 2D Validated Efficiency: 85%\n";

    double thrustRatio = result.thrust / 11519.0 * 100;
    double ispRatio = result.specificImpulse / 1629.0 * 100;
    assessment << "- Current Thrust vs Baseline: " << std::fixed << std::setprecision(1) << thrustRatio << "%\n";
    assessment << "- Current Isp vs Baseline: " << std::fixed << std::setprecision(1) << ispRatio << "%\n";

    return assessment.str();
}

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerRDEWaveTool(McpServer& server) {
    auto tool = std::make_shared<RDEWaveTool>();

    server.registerTool(
        RDEWaveTool::getName(),
        RDEWaveTool::getDescription(),
        RDEWaveTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult { return tool->execute(arguments); });
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //
