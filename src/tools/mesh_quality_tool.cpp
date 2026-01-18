/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Mesh Quality Assessment MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for mesh quality assessment implementation.

\*---------------------------------------------------------------------------*/

#include "mesh_quality_tool.hpp"

#include <iomanip>
#include <sstream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        MeshQualityTool Implementation
\*---------------------------------------------------------------------------*/

MeshQualityTool::MeshQualityTool()
    : analyzer_(std::make_unique<MeshQualityAnalyzer>()) {}

MeshQualityTool::MeshQualityTool(std::unique_ptr<MeshQualityAnalyzer> analyzer)
    : analyzer_(std::move(analyzer)) {}

ToolResult MeshQualityTool::execute(const json& arguments) {
    ToolResult result;

    try {
        std::string caseDirectory = arguments.at("case_directory").get<std::string>();
        std::string analysisMode = arguments.value("analysis_mode", "detailed");
        std::string targetSolver = arguments.value("target_solver", "");

        QualityReport report;

        if (analysisMode == "quick") {
            report = analyzer_->quickQualityCheck(caseDirectory);
        } else {
            report = analyzer_->assessMeshQuality(caseDirectory);
        }

        // Format results for user
        std::string formattedResults = formatResultsForUser(report);
        result.addTextContent(formattedResults);

        // Add educational content
        std::string educational = generateEducationalContent(report);
        result.addTextContent(educational);

        // Add recommendations
        std::string recommendations = generateRecommendations(report);
        result.addTextContent(recommendations);

        // Add JSON resource
        json reportJson = analyzer_->reportToJson(report);
        result.content.push_back(
            json{{"type", "resource"},
                 {"resource",
                  {{"uri", "openfoam://mesh_quality/" + caseDirectory},
                   {"name", "Mesh Quality Report"},
                   {"description", "Complete mesh quality assessment results"},
                   {"mimeType", "application/json"}}},
                 {"text", reportJson.dump(2)}});

    } catch (const std::exception& e) {
        result.addErrorContent("Error executing mesh quality assessment: " + std::string(e.what()));
    }

    return result;
}

std::string MeshQualityTool::formatResultsForUser(const QualityReport& report) const {
    std::ostringstream output;

    output << "**Mesh Quality Assessment Results**\n\n";
    output << "**Case:** " << report.caseDirectory << "\n";
    output << "**Mesh Type:** " << report.meshType << "\n\n";

    output << "**Overall Assessment:**\n";
    output << "- Grade: " << report.overallGrade << " (Score: "
           << std::fixed << std::setprecision(1) << report.qualityScore << "/100)\n";
    output << "- Ready for Simulation: " << (report.readyForSimulation ? "Yes" : "No") << "\n";
    output << "- Reason: " << report.readinessReason << "\n\n";

    output << "**Mesh Statistics:**\n";
    output << "- Total Cells: " << report.totalCells << "\n";
    output << "- Total Faces: " << report.totalFaces << "\n";
    output << "- Total Points: " << report.totalPoints << "\n\n";

    output << "**Quality Metrics:**\n";
    for (const auto& metric : report.metrics) {
        output << "- " << metric.metricName << ": "
               << std::fixed << std::setprecision(2) << metric.value
               << " (" << metric.status << ")\n";
    }

    if (!report.issues.empty()) {
        output << "\n**Issues Found:**\n";
        for (const auto& issue : report.issues) {
            output << "- [" << issue.severity << "] " << issue.description << "\n";
        }
    }

    return output.str();
}

std::string MeshQualityTool::generateEducationalContent(const QualityReport& report) const {
    std::ostringstream content;

    content << "\n**Understanding Your Mesh Quality:**\n\n";

    for (const auto& metric : report.metrics) {
        if (!metric.explanation.empty()) {
            content << "**" << metric.metricName << ":** " << metric.explanation << "\n";
            if (!metric.impact.empty()) {
                content << "  Impact: " << metric.impact << "\n";
            }
        }
    }

    if (!report.learningPoints.empty()) {
        content << "\n**Key Learning Points:**\n";
        for (const auto& point : report.learningPoints) {
            content << "- " << point << "\n";
        }
    }

    return content.str();
}

std::string MeshQualityTool::generateRecommendations(const QualityReport& report) const {
    std::ostringstream recommendations;

    recommendations << "\n**Recommendations:**\n\n";

    if (!report.nextSteps.empty()) {
        recommendations << "**Next Steps:**\n";
        for (const auto& step : report.nextSteps) {
            recommendations << "- " << step << "\n";
        }
    }

    if (!report.solverCompatibility.empty()) {
        recommendations << "\n**Solver Compatibility:**\n";
        for (const auto& compat : report.solverCompatibility) {
            recommendations << "- " << compat.solverName << ": "
                           << (compat.isCompatible ? "Compatible" : "Not Recommended")
                           << " - " << compat.compatibilityReason << "\n";
        }
    }

    return recommendations.str();
}

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerMeshQualityTool(McpServer& server) {
    auto tool = std::make_shared<MeshQualityTool>();

    server.registerTool(
        MeshQualityTool::getName(),
        MeshQualityTool::getDescription(),
        MeshQualityTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult { return tool->execute(arguments); });
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //
