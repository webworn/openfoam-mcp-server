#include <exception>
#include <iostream>
#include "mcp/server.hpp"

// Forward declarations for tool registration
namespace Foam { namespace MCP {
    void registerCFDAssistantTool(McpServer& server);
    void registerMeshQualityTool(McpServer& server);
    void registerSTLAnalyzerTool(McpServer& server);
    void registerRDEWaveTool(McpServer& server);
    void registerRDE3DGeometryTool(McpServer& server);
    void registerRDE3DWaveTool(McpServer& server);
    void registerRDE3DPerformanceTool(McpServer& server);
    void registerTurbulentFlowTool(McpServer& server);
}}

using namespace Foam;
using namespace Foam::MCP;

int main(int argc, char* argv[]) {
    std::cerr << "🚀 OpenFOAM MCP Server v1.0.0 (Test Mode)" << std::endl;
    std::cerr << "🎯 Initializing MCP server without OpenFOAM dependencies..." << std::endl;

    try {
        // Initialize MCP server
        McpServer server;

        // Set server information
        ServerInfo info;
        info.name = "OpenFOAM MCP Server (Test)";
        info.version = "1.0.0";
        info.description = "Test version of OpenFOAM MCP server";
        info.author = "OpenFOAM Community";
        info.homepage = "https://github.com/openfoam/mcp-server";
        server.setServerInfo(info);

        // Register minimal tools (skip OpenFOAM-dependent ones for now)
        // registerPipeFlowTool(server);
        // registerExternalFlowTool(server);
        // registerHeatTransferTool(server);
        // registerMultiphaseFlowTool(server);
        
        // Register intelligent CFD assistant (if it doesn't require OpenFOAM)
        try {
            registerCFDAssistantTool(server);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not register CFD assistant: " << e.what() << std::endl;
        }

        // Register mesh quality assessment tool
        try {
            registerMeshQualityTool(server);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not register mesh quality tool: " << e.what() << std::endl;
        }

        // Register STL geometry analyzer tool
        try {
            registerSTLAnalyzerTool(server);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not register STL analyzer tool: " << e.what() << std::endl;
        }

        // Register RDE 2D wave analyzer tool
        try {
            registerRDEWaveTool(server);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not register RDE wave tool: " << e.what() << std::endl;
        }

        // Register RDE 3D geometry generator tool
        try {
            registerRDE3DGeometryTool(server);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not register RDE 3D geometry tool: " << e.what() << std::endl;
        }

        // Register RDE 3D wave analyzer tool
        try {
            registerRDE3DWaveTool(server);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not register RDE 3D wave tool: " << e.what() << std::endl;
        }

        // Register RDE 3D performance calculator tool
        try {
            registerRDE3DPerformanceTool(server);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not register RDE 3D performance tool: " << e.what() << std::endl;
        }

        // Register turbulent flow analysis tool
        try {
            registerTurbulentFlowTool(server);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not register turbulent flow tool: " << e.what() << std::endl;
        }

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