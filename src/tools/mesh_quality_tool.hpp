/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Mesh Quality Assessment MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for mesh quality assessment functionality.

    Provides mesh quality analysis with solver compatibility assessment
    and educational explanations.

\*---------------------------------------------------------------------------*/

#ifndef mesh_quality_tool_H
#define mesh_quality_tool_H

#include <memory>

#include "../mcp/server.hpp"
#include "../openfoam/mesh_quality.hpp"

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Class MeshQualityTool Declaration
\*---------------------------------------------------------------------------*/

class MeshQualityTool {
private:
    std::unique_ptr<MeshQualityAnalyzer> analyzer_;

    std::string formatResultsForUser(const QualityReport& report) const;
    std::string generateEducationalContent(const QualityReport& report) const;
    std::string generateRecommendations(const QualityReport& report) const;

public:
    MeshQualityTool();
    explicit MeshQualityTool(std::unique_ptr<MeshQualityAnalyzer> analyzer);
    ~MeshQualityTool() = default;

    static std::string getName() { return "assess_mesh_quality"; }

    static std::string getDescription() {
        return "Analyze OpenFOAM mesh quality with comprehensive metrics including "
               "non-orthogonality, skewness, and aspect ratio. Provides solver "
               "compatibility assessment and improvement recommendations.";
    }

    static json getInputSchema() {
        return json{
            {"type", "object"},
            {"properties", {
                {"case_directory", {
                    {"type", "string"},
                    {"description", "Path to the OpenFOAM case directory containing the mesh"}
                }},
                {"analysis_mode", {
                    {"type", "string"},
                    {"enum", {"quick", "detailed"}},
                    {"default", "detailed"},
                    {"description", "Analysis mode: 'quick' for basic metrics, 'detailed' for comprehensive analysis"}
                }},
                {"target_solver", {
                    {"type", "string"},
                    {"enum", {"simpleFoam", "pimpleFoam", "icoFoam", "interFoam", "chtMultiRegionFoam", "rhoSimpleFoam", "buoyantSimpleFoam"}},
                    {"description", "Target solver for compatibility assessment (optional)"}
                }}
            }},
            {"required", {"case_directory"}}
        };
    }

    ToolResult execute(const json& arguments);
};

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerMeshQualityTool(McpServer& server);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //
