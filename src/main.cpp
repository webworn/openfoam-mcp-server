#include <exception>
#include <cstdlib>
#include <iostream>

#include "mcp/server.hpp"
#include "tools/external_flow_tool.hpp"
#include "tools/heat_transfer_tool.hpp"
#include "tools/run_case_tool.hpp"
#include "tools/multiphase_flow_tool.hpp"
#include "tools/pipe_flow_tool.hpp"

// Forward declaration for CFD assistant registration
namespace Foam { namespace MCP { 
    void registerCFDAssistantTool(McpServer& server);
}}

using namespace Foam;
using namespace Foam::MCP;

int main() {
    std::cerr << "🚀 OpenFOAM MCP Server v1.0.0" << std::endl;
    const char* foam_version = std::getenv("WM_PROJECT_VERSION");
    std::cerr << "OpenFOAM runtime version: "
              << (foam_version ? foam_version : "not detected (source etc/bashrc before launch)")
              << std::endl;

    std::cerr << "🎯 Initializing MCP server with OpenFOAM tools..." << std::endl;

    try {
        // Initialize MCP server
        McpServer server;

        // Set server information
        ServerInfo info;
        info.name = "OpenFOAM MCP Server";
        info.version = "1.0.0";
        info.description = "World's first native OpenFOAM Model Context Protocol server";
        info.author = "OpenFOAM Community";
        info.homepage = "https://github.com/openfoam/mcp-server";
        server.setServerInfo(info);

        // Register tools
        registerPipeFlowTool(server);
        registerExternalFlowTool(server);
        registerHeatTransferTool(server);
        registerRunCaseTool(server);
        registerMultiphaseFlowTool(server);
        
        // Register intelligent CFD assistant
        registerCFDAssistantTool(server);

        std::cerr << "🔧 Registered tools: ";
        auto tools = server.getRegisteredTools();
        for (size_t i = 0; i < tools.size(); ++i) {
            std::cerr << tools[i];
            if (i < tools.size() - 1) std::cerr << ", ";
        }
        std::cerr << std::endl;

        std::cerr << "🌟 OpenFOAM MCP Server ready!" << std::endl;
        std::cerr << "📡 Listening on stdin for MCP protocol messages..." << std::endl;

        // Start server (blocking)
        server.start();

    } catch (const std::exception& e) {
        std::cerr << "❌ Error starting MCP server: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
