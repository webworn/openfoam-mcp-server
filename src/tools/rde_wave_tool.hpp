/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | RDE 2D Wave Analyzer MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for 2D Rotating Detonation Engine wave analysis.

    Analyzes RDE simulation data for wave detection, collision analysis,
    and performance metrics calculation with educational physics explanations.

\*---------------------------------------------------------------------------*/

#ifndef rde_wave_tool_H
#define rde_wave_tool_H

#include <memory>

#include "../mcp/server.hpp"
#include "rde_wave_analyzer.hpp"

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Class RDEWaveTool Declaration
\*---------------------------------------------------------------------------*/

class RDEWaveTool {
private:
    std::unique_ptr<RDE2DWaveAnalyzer> analyzer_;

    std::string formatResultsForUser(const RDE2DWaveAnalyzer::WaveAnalysisResult& result) const;
    std::string generateEducationalContent(const RDE2DWaveAnalyzer::WaveAnalysisResult& result) const;
    std::string generatePerformanceAssessment(const RDE2DWaveAnalyzer::WaveAnalysisResult& result) const;

public:
    RDEWaveTool();
    explicit RDEWaveTool(std::unique_ptr<RDE2DWaveAnalyzer> analyzer);
    ~RDEWaveTool() = default;

    static std::string getName() { return "analyze_rde_waves_2d"; }

    static std::string getDescription() {
        return "Analyze 2D Rotating Detonation Engine simulation data for wave detection, "
               "multi-wave interactions, collision analysis, and performance metrics including "
               "thrust, specific impulse, and combustion efficiency.";
    }

    static json getInputSchema() {
        return json{
            {"type", "object"},
            {"properties", {
                {"case_directory", {
                    {"type", "string"},
                    {"description", "Path to the OpenFOAM case directory with RDE simulation results"}
                }},
                {"temperature_threshold", {
                    {"type", "number"},
                    {"default", 2500.0},
                    {"description", "Temperature threshold in Kelvin for wave front detection (default: 2500 K)"}
                }},
                {"time_range", {
                    {"type", "object"},
                    {"properties", {
                        {"start", {{"type", "number"}, {"description", "Start time in seconds"}}},
                        {"end", {{"type", "number"}, {"description", "End time in seconds"}}}
                    }},
                    {"description", "Optional time range for analysis"}
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

void registerRDEWaveTool(McpServer& server);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //
