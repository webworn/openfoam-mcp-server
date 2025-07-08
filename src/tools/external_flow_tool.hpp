/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | External Flow MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for external flow analysis

    Provides the interface between MCP protocol and OpenFOAM external flow
    analysis functionality for automotive, aerospace, and civil applications.

\*---------------------------------------------------------------------------*/

#ifndef external_flow_tool_H
    #define external_flow_tool_H

    #include "../mcp/server.hpp"
    #include "../openfoam/external_flow.hpp"

    #include <memory>

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                        Class ExternalFlowTool Declaration
\*---------------------------------------------------------------------------*/

class ExternalFlowTool
{
  private:
    std::unique_ptr<ExternalFlowAnalyzer> analyzer_;

    std::string formatResultsForUser(const ExternalFlowResults& results) const;
    std::string generatePhysicsExplanation(const ExternalFlowInput& input,
                                           const ExternalFlowResults& results) const;
    std::string generateRecommendations(const ExternalFlowInput& input,
                                        const ExternalFlowResults& results) const;
    std::string generateApplicationGuidance(const ExternalFlowInput& input,
                                            const ExternalFlowResults& results) const;

  public:
    ExternalFlowTool();
    explicit ExternalFlowTool(std::unique_ptr<ExternalFlowAnalyzer> analyzer);
    ~ExternalFlowTool() = default;

    static std::string getName()
    {
        return "analyze_external_flow";
    }

    static std::string getDescription()
    {
        return "Analyze external flow around vehicles, aircraft, and buildings using OpenFOAM CFD "
               "simulation. "
               "Calculates drag coefficient, lift coefficient, Reynolds number, and aerodynamic "
               "forces for "
               "automotive, aerospace, and civil engineering applications. Supports cars, "
               "aircraft, buildings, "
               "and cylinders with automatic flow regime detection and expert aerodynamics "
               "guidance.";
    }

    static json getInputSchema()
    {
        ExternalFlowAnalyzer analyzer;
        return analyzer.getInputSchema();
    }

    ToolResult execute(const json& arguments);

    void setWorkingDirectory(const std::string& workingDir);

    std::vector<std::string> listActiveCases() const;
    bool deleteCaseData(const std::string& caseId);
};

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerExternalFlowTool(McpServer& server);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //