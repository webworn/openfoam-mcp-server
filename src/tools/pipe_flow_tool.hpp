/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Pipe Flow MCP Tool
    \\  /    A nd           | 
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for pipe flow analysis
    
    Provides the interface between MCP protocol and OpenFOAM pipe flow
    analysis functionality.

\*---------------------------------------------------------------------------*/

#ifndef pipe_flow_tool_H
#define pipe_flow_tool_H

#include "../mcp/server.hpp"
#include "../openfoam/pipe_flow.hpp"
#include <memory>

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                        Class PipeFlowTool Declaration
\*---------------------------------------------------------------------------*/

class PipeFlowTool
{
private:

    std::unique_ptr<PipeFlowAnalyzer> analyzer_;
    
    std::string formatResultsForUser(const PipeFlowResults& results) const;
    std::string generatePhysicsExplanation(const PipeFlowInput& input, const PipeFlowResults& results) const;
    std::string generateRecommendations(const PipeFlowInput& input, const PipeFlowResults& results) const;

public:

    PipeFlowTool();
    explicit PipeFlowTool(std::unique_ptr<PipeFlowAnalyzer> analyzer);
    ~PipeFlowTool() = default;
    
    static std::string getName()
    {
        return "run_pipe_flow";
    }
    
    static std::string getDescription()
    {
        return "Analyze pipe flow using OpenFOAM CFD simulation. Calculates Reynolds number, "
               "friction factor, pressure drop, and wall shear stress for steady or transient "
               "flow in circular pipes. Supports both laminar and turbulent flow regimes.";
    }
    
    static json getInputSchema()
    {
        PipeFlowAnalyzer analyzer;
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

void registerPipeFlowTool(McpServer& server);

} // End namespace MCP
} // End namespace Foam

#endif

// ************************************************************************* //