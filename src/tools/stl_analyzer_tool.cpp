/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | STL Geometry Analyzer MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for STL geometry analysis implementation.

\*---------------------------------------------------------------------------*/

#include "stl_analyzer_tool.hpp"

#include <iomanip>
#include <sstream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        STLAnalyzerTool Implementation
\*---------------------------------------------------------------------------*/

STLAnalyzerTool::STLAnalyzerTool()
    : analyzer_(std::make_unique<STLAnalyzer>()) {}

STLAnalyzerTool::STLAnalyzerTool(std::unique_ptr<STLAnalyzer> analyzer)
    : analyzer_(std::move(analyzer)) {}

ToolResult STLAnalyzerTool::execute(const json& arguments) {
    ToolResult result;

    try {
        std::string stlFile = arguments.at("stl_file").get<std::string>();
        double featureAngle = arguments.value("feature_angle", 30.0);
        std::string flowType = arguments.value("flow_type", "external");

        // Perform STL analysis
        STLQualityReport report = analyzer_->analyzeSTL(stlFile);

        // Extract feature lines
        std::vector<GeometryFeature> features = analyzer_->extractFeatureLines(stlFile, featureAngle);
        report.features = features;

        // Format results for user
        std::string formattedResults = formatResultsForUser(report);
        result.addTextContent(formattedResults);

        // Add educational content
        std::string educational = generateEducationalContent(report);
        result.addTextContent(educational);

        // Add refinement guidance
        std::string refinement = generateRefinementGuidance(report, flowType);
        result.addTextContent(refinement);

        // Add JSON resource
        json reportJson = analyzer_->reportToJson(report);
        result.content.push_back(
            json{{"type", "resource"},
                 {"resource",
                  {{"uri", "openfoam://stl_analysis/" + stlFile},
                   {"name", "STL Quality Report"},
                   {"description", "Complete STL geometry analysis results"},
                   {"mimeType", "application/json"}}},
                 {"text", reportJson.dump(2)}});

    } catch (const std::exception& e) {
        result.addErrorContent("Error executing STL analysis: " + std::string(e.what()));
    }

    return result;
}

std::string STLAnalyzerTool::formatResultsForUser(const STLQualityReport& report) const {
    std::ostringstream output;

    output << "**STL Geometry Analysis Results**\n\n";
    output << "**File:** " << report.filename << "\n";
    output << "**Size:** " << std::fixed << std::setprecision(2) << report.fileSize << " MB\n\n";

    output << "**Geometry Statistics:**\n";
    output << "- Triangles: " << report.numberOfTriangles << "\n";
    output << "- Vertices: " << report.numberOfVertices << "\n";
    output << "- Surface Area: " << std::fixed << std::setprecision(4) << report.surfaceArea << " m^2\n";
    if (report.volume >= 0) {
        output << "- Volume: " << std::fixed << std::setprecision(6) << report.volume << " m^3\n";
    }
    output << "\n";

    output << "**Quality Checks:**\n";
    output << "- Watertight: " << (report.isWatertight ? "Yes" : "No") << "\n";
    output << "- Manifold Edges: " << (report.hasManifoldEdges ? "Yes" : "No") << "\n";
    output << "- Consistent Normals: " << (report.hasConsistentNormals ? "Yes" : "No") << "\n";
    output << "- Min Triangle Quality: " << std::fixed << std::setprecision(3) << report.minTriangleQuality << "\n";
    output << "- Avg Triangle Quality: " << std::fixed << std::setprecision(3) << report.avgTriangleQuality << "\n\n";

    output << "**Bounding Box:**\n";
    if (report.boundingBox.size() >= 6) {
        output << "- X: [" << report.boundingBox[0] << ", " << report.boundingBox[3] << "] m\n";
        output << "- Y: [" << report.boundingBox[1] << ", " << report.boundingBox[4] << "] m\n";
        output << "- Z: [" << report.boundingBox[2] << ", " << report.boundingBox[5] << "] m\n\n";
    }

    output << "**snappyHexMesh Readiness:** " << (report.readyForSnappy ? "Ready" : "Not Ready") << "\n";
    output << "- Reason: " << report.readinessReason << "\n";

    if (!report.issues.empty()) {
        output << "\n**Issues Found:**\n";
        for (const auto& issue : report.issues) {
            output << "- [" << issue.severity << "] " << issue.issueType << ": " << issue.description << "\n";
            if (!issue.fixSuggestion.empty()) {
                output << "  Fix: " << issue.fixSuggestion << "\n";
            }
        }
    }

    return output.str();
}

std::string STLAnalyzerTool::generateEducationalContent(const STLQualityReport& report) const {
    std::ostringstream content;

    content << "\n**Understanding STL Quality:**\n\n";

    content << "**Geometry Complexity:** " << report.complexity.level << "\n";
    if (report.complexity.hasSharpFeatures) {
        content << "- Contains sharp features (angles < 30 degrees) requiring special handling\n";
    }
    if (report.complexity.hasConcaveRegions) {
        content << "- Contains concave regions that may affect mesh quality\n";
    }

    content << "\n**Feature Analysis:**\n";
    content << "- Number of significant features: " << report.features.size() << "\n";

    int criticalFeatures = 0;
    int importantFeatures = 0;
    for (const auto& feature : report.features) {
        if (feature.importance == "critical") criticalFeatures++;
        else if (feature.importance == "important") importantFeatures++;
    }
    content << "- Critical features: " << criticalFeatures << "\n";
    content << "- Important features: " << importantFeatures << "\n";

    content << "\n**Why STL Quality Matters:**\n";
    content << "90% of snappyHexMesh failures start with bad geometry. Key points:\n";
    content << "- Watertight geometry prevents meshing errors at boundaries\n";
    content << "- Manifold edges ensure proper surface connectivity\n";
    content << "- Consistent normals define inside/outside correctly\n";
    content << "- Triangle quality affects mesh cell quality near surfaces\n";

    return content.str();
}

std::string STLAnalyzerTool::generateRefinementGuidance(const STLQualityReport& report, const std::string& flowType) const {
    std::ostringstream guidance;

    guidance << "\n**Refinement Recommendations:**\n\n";

    guidance << "**Flow Type:** " << flowType << "\n\n";

    // Get refinement suggestions from analyzer
    std::vector<RefinementRegion> regions = analyzer_->suggestRefinementRegions(report, flowType);

    guidance << "**Suggested Refinement Regions:**\n";
    for (const auto& region : regions) {
        guidance << "- " << region.regionName << " (" << region.regionType << ")\n";
        guidance << "  Level: " << region.refinementLevel << ", Priority: " << region.priority << "\n";
        guidance << "  Reason: " << region.reason << "\n";
    }

    guidance << "\n**Feature Extraction:**\n";
    guidance << "- Recommended feature angle: 30 degrees (default)\n";
    guidance << "- Use surfaceFeatureExtract before snappyHexMesh\n";
    guidance << "- Critical features should be captured for proper edge resolution\n";

    return guidance.str();
}

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerSTLAnalyzerTool(McpServer& server) {
    auto tool = std::make_shared<STLAnalyzerTool>();

    server.registerTool(
        STLAnalyzerTool::getName(),
        STLAnalyzerTool::getDescription(),
        STLAnalyzerTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult { return tool->execute(arguments); });
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //
