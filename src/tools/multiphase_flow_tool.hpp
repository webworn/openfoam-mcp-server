/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Multiphase Flow Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool for multiphase flow analysis using OpenFOAM interFoam solver

\*---------------------------------------------------------------------------*/

#ifndef multiphase_flow_tool_H
#define multiphase_flow_tool_H

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "mcp/server.hpp"
#include "openfoam/multiphase_flow.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                     Class MultiphaseFlowTool Declaration
\*---------------------------------------------------------------------------*/

class MultiphaseFlowTool {
   private:
    MultiphaseFlowAnalyzer analyzer_;
    std::unique_ptr<CaseManager> caseManager_;

    // Helper methods
    MultiphaseFlowInput parseInput(const json& arguments);
    bool validateInputParameters(const MultiphaseFlowInput& input) const;
    std::vector<json> formatResults(const MultiphaseFlowInput& input,
                                    const MultiphaseFlowResults& results) const;

    // Result formatting methods
    std::string generatePhysicsExplanation(const MultiphaseFlowInput& input,
                                           const MultiphaseFlowResults& results) const;
    std::string generateDimensionlessAnalysis(const MultiphaseFlowResults& results) const;
    std::string generateFlowRegimeAnalysis(const MultiphaseFlowInput& input,
                                           const MultiphaseFlowResults& results) const;
    std::string generateApplicationGuidance(const MultiphaseFlowInput& input,
                                            const MultiphaseFlowResults& results) const;

   public:
    MultiphaseFlowTool();
    ~MultiphaseFlowTool() = default;

    // MCP tool interface
    static std::string getName() { return "analyze_multiphase_flow"; }
    static std::string getDescription() {
        return "Analyze multiphase flow using OpenFOAM interFoam solver. "
               "Supports two-phase immiscible fluid simulations including dam break, sloshing, "
               "bubble dynamics, wave breaking, and mixing applications. "
               "Calculates dimensionless numbers (Reynolds, Weber, Bond, Froude), "
               "phase distribution, interface dynamics, and energy analysis for "
               "marine, coastal, chemical processing, and industrial applications.";
    }

    static json getInputSchema();

    ToolResult execute(const json& arguments);
};

/*---------------------------------------------------------------------------*\
                        Registration Function
\*---------------------------------------------------------------------------*/

void registerMultiphaseFlowTool(McpServer& server);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //