/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | RDE 3D Wave Analyzer MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for 3D RDE wave propagation analysis functionality.

    Analyzes 3D detonation wave fronts, axial propagation, wave interactions,
    and performance correlation with educational physics explanations.

\*---------------------------------------------------------------------------*/

#ifndef rde_3d_wave_tool_H
#define rde_3d_wave_tool_H

#include <memory>

#include "../mcp/server.hpp"
#include "rde_3d_wave_analyzer.hpp"

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Class RDE3DWaveTool Declaration
\*---------------------------------------------------------------------------*/

class RDE3DWaveTool {
private:
    std::unique_ptr<RDE3DWaveAnalyzer> analyzer_;

    std::string formatResultsForUser(const RDE3DWaveAnalyzer::Wave3DAnalysisResult& result) const;
    std::string generateEducationalContent(const RDE3DWaveAnalyzer::Wave3DAnalysisResult& result) const;
    std::string generatePerformanceSummary(const RDE3DWaveAnalyzer::Wave3DAnalysisResult& result) const;

public:
    RDE3DWaveTool();
    explicit RDE3DWaveTool(std::unique_ptr<RDE3DWaveAnalyzer> analyzer);
    ~RDE3DWaveTool() = default;

    static std::string getName() { return "analyze_rde_waves_3d"; }

    static std::string getDescription() {
        return "Analyze 3D Rotating Detonation Engine wave propagation including detonation front "
               "tracking, axial propagation effects, 3D wave interactions, and performance correlation "
               "with validated 2D baseline metrics.";
    }

    static json getInputSchema() {
        return json{
            {"type", "object"},
            {"properties", {
                {"case_directory", {
                    {"type", "string"},
                    {"description", "Path to the OpenFOAM case directory with 3D RDE simulation results"}
                }},
                {"detection_thresholds", {
                    {"type", "object"},
                    {"properties", {
                        {"temperature", {{"type", "number"}, {"default", 2500.0}, {"description", "Temperature threshold in Kelvin"}}},
                        {"pressure", {{"type", "number"}, {"default", 1.0e6}, {"description", "Pressure threshold in Pa"}}},
                        {"velocity", {{"type", "number"}, {"default", 1500.0}, {"description", "Velocity threshold in m/s"}}}
                    }},
                    {"description", "Thresholds for 3D wave detection"}
                }},
                {"time_range", {
                    {"type", "object"},
                    {"properties", {
                        {"start", {{"type", "number"}, {"description", "Start time in seconds"}}},
                        {"end", {{"type", "number"}, {"description", "End time in seconds"}}},
                        {"interval", {{"type", "number"}, {"default", 1.0e-6}, {"description", "Time step interval for analysis"}}}
                    }},
                    {"description", "Time range for 3D wave analysis"}
                }},
                {"analysis_options", {
                    {"type", "object"},
                    {"properties", {
                        {"analyze_axial_propagation", {{"type", "boolean"}, {"default", true}}},
                        {"analyze_3d_interactions", {{"type", "boolean"}, {"default", true}}},
                        {"calculate_performance_metrics", {{"type", "boolean"}, {"default", true}}},
                        {"generate_visualization", {{"type", "boolean"}, {"default", false}}}
                    }},
                    {"description", "Options for 3D wave analysis"}
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

void registerRDE3DWaveTool(McpServer& server);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //
