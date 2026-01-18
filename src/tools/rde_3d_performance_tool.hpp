/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | RDE 3D Performance Calculator MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for 3D RDE performance calculation functionality.

    Calculates comprehensive 3D performance metrics including thrust, Isp,
    combustion efficiency, and nozzle performance with component breakdown.

\*---------------------------------------------------------------------------*/

#ifndef rde_3d_performance_tool_H
#define rde_3d_performance_tool_H

#include <memory>

#include "../mcp/server.hpp"
#include "rde_3d_performance.hpp"

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Class RDE3DPerformanceTool Declaration
\*---------------------------------------------------------------------------*/

class RDE3DPerformanceTool {
private:
    std::unique_ptr<RDE3DPerformanceCalculator> calculator_;

    std::string formatResultsForUser(const RDE3DPerformanceCalculator::Performance3DMetrics& metrics) const;
    std::string generateThrustBreakdown(const RDE3DPerformanceCalculator::Performance3DMetrics& metrics) const;
    std::string generateEducationalContent(const RDE3DPerformanceCalculator::Performance3DMetrics& metrics) const;
    std::string generateOptimizationGuidance(const RDE3DPerformanceCalculator::Performance3DMetrics& metrics) const;

public:
    RDE3DPerformanceTool();
    explicit RDE3DPerformanceTool(std::unique_ptr<RDE3DPerformanceCalculator> calculator);
    ~RDE3DPerformanceTool() = default;

    static std::string getName() { return "calculate_rde_3d_performance"; }

    static std::string getDescription() {
        return "Calculate comprehensive 3D RDE performance metrics including thrust breakdown "
               "(combustor + nozzle), specific impulse, combustion efficiency, and nozzle "
               "performance. Validates against 2D baseline (11,519 N, 1,629 s, 85% efficiency).";
    }

    static json getInputSchema() {
        return json{
            {"type", "object"},
            {"properties", {
                {"case_directory", {
                    {"type", "string"},
                    {"description", "Path to the OpenFOAM case directory with 3D RDE simulation results"}
                }},
                {"geometry", {
                    {"type", "object"},
                    {"properties", {
                        {"inner_radius", {{"type", "number"}, {"default", 0.050}, {"description", "Inner combustor radius in meters"}}},
                        {"outer_radius", {{"type", "number"}, {"default", 0.080}, {"description", "Outer combustor radius in meters"}}},
                        {"axial_length", {{"type", "number"}, {"default", 0.150}, {"description", "Combustor axial length in meters"}}},
                        {"nozzle_throat_dia", {{"type", "number"}, {"default", 0.060}, {"description", "Nozzle throat diameter in meters"}}},
                        {"nozzle_exit_dia", {{"type", "number"}, {"default", 0.090}, {"description", "Nozzle exit diameter in meters"}}},
                        {"nozzle_expansion", {{"type", "number"}, {"default", 2.25}, {"description", "Nozzle expansion ratio"}}}
                    }},
                    {"description", "Geometry parameters for performance calculation"}
                }},
                {"analysis_options", {
                    {"type", "object"},
                    {"properties", {
                        {"calculate_axial_thrust", {{"type", "boolean"}, {"default", true}}},
                        {"analyze_nozzle_performance", {{"type", "boolean"}, {"default", true}}},
                        {"analyze_combustion_efficiency", {{"type", "boolean"}, {"default", true}}},
                        {"include_viscous_effects", {{"type", "boolean"}, {"default", false}}},
                        {"validate_against_2d", {{"type", "boolean"}, {"default", true}}}
                    }},
                    {"description", "Options for performance analysis"}
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

void registerRDE3DPerformanceTool(McpServer& server);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //
