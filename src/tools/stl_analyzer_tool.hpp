/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | STL Geometry Analyzer MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for STL geometry analysis functionality.

    Analyzes STL files for quality issues and provides snappyHexMesh
    readiness assessment with feature extraction guidance.

\*---------------------------------------------------------------------------*/

#ifndef stl_analyzer_tool_H
#define stl_analyzer_tool_H

#include <memory>

#include "../mcp/server.hpp"
#include "../openfoam/stl_analyzer.hpp"

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Class STLAnalyzerTool Declaration
\*---------------------------------------------------------------------------*/

class STLAnalyzerTool {
private:
    std::unique_ptr<STLAnalyzer> analyzer_;

    std::string formatResultsForUser(const STLQualityReport& report) const;
    std::string generateEducationalContent(const STLQualityReport& report) const;
    std::string generateRefinementGuidance(const STLQualityReport& report, const std::string& flowType) const;

public:
    STLAnalyzerTool();
    explicit STLAnalyzerTool(std::unique_ptr<STLAnalyzer> analyzer);
    ~STLAnalyzerTool() = default;

    static std::string getName() { return "analyze_stl_geometry"; }

    static std::string getDescription() {
        return "Analyze STL geometry files for quality issues and snappyHexMesh readiness. "
               "Identifies watertightness, manifold edges, normal consistency, and provides "
               "feature extraction and refinement guidance.";
    }

    static json getInputSchema() {
        return json{
            {"type", "object"},
            {"properties", {
                {"stl_file", {
                    {"type", "string"},
                    {"description", "Path to the STL file to analyze"}
                }},
                {"feature_angle", {
                    {"type", "number"},
                    {"default", 30.0},
                    {"description", "Feature angle threshold in degrees for edge detection (default: 30)"}
                }},
                {"flow_type", {
                    {"type", "string"},
                    {"enum", {"external", "internal", "multiphase", "heat_transfer"}},
                    {"default", "external"},
                    {"description", "Flow type for refinement recommendations"}
                }}
            }},
            {"required", {"stl_file"}}
        };
    }

    ToolResult execute(const json& arguments);
};

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerSTLAnalyzerTool(McpServer& server);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //
