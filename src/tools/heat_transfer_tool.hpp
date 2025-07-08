/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Website:  https://openfoam.org
     \\/     M anipulation  |
\*---------------------------------------------------------------------------*/

#ifndef HEAT_TRANSFER_TOOL_HPP
#define HEAT_TRANSFER_TOOL_HPP

#include "mcp/server.hpp"
#include "openfoam/case_manager.hpp"
#include "openfoam/heat_transfer.hpp"

#include <nlohmann/json.hpp>

namespace Foam
{
namespace MCP
{

using json = nlohmann::json;

class HeatTransferTool
{
  private:
    HeatTransferAnalyzer analyzer_;
    std::unique_ptr<CaseManager> caseManager_;

  public:
    HeatTransferTool();
    ~HeatTransferTool() = default;

    // MCP tool interface
    static std::string getName()
    {
        return "analyze_heat_transfer";
    }
    static std::string getDescription()
    {
        return "Analyze conjugate heat transfer using OpenFOAM chtMultiRegionFoam solver. "
               "Supports electronics cooling, heat exchangers, building thermal analysis, and "
               "engine cooling. "
               "Calculates thermal resistance, heat transfer coefficients, temperature "
               "distributions, and "
               "provides thermal design recommendations for thermal management applications.";
    }
    static json getInputSchema();
    ToolResult execute(const json& arguments);

    // Analysis functions
    std::string generateThermalExplanation(const HeatTransferInput& input,
                                           const HeatTransferResults& results) const;
    std::string generateDesignRecommendations(const HeatTransferInput& input,
                                              const HeatTransferResults& results) const;
    std::string generateApplicationGuidance(const HeatTransferInput& input,
                                            const HeatTransferResults& results) const;

  private:
    // Input validation and conversion
    HeatTransferInput parseInput(const json& arguments);
    bool validateInputParameters(const HeatTransferInput& input) const;

    // OpenFOAM case setup
    std::string setupOpenFOAMCase(const HeatTransferInput& input);
    CaseParameters createCaseParameters(const HeatTransferInput& input) const;

    // Result formatting
    std::vector<json> formatResults(const HeatTransferInput& input,
                                    const HeatTransferResults& results) const;

    // Educational content generation
    std::string generateHeatTransferPhysics(const HeatTransferInput& input) const;
    std::string generateDimensionlessNumbers(const HeatTransferResults& results) const;
    std::string generateMaterialProperties(const HeatTransferInput& input) const;
};

// Registration function
void registerHeatTransferTool(McpServer& server);

}  // namespace MCP
}  // namespace Foam

#endif  // HEAT_TRANSFER_TOOL_HPP