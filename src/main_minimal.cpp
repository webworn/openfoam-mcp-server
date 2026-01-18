#include <iostream>
#include <exception>
#include "mcp/server.hpp"

using namespace Foam::MCP;

int main(int argc, char* argv[]) {
    std::cerr << "🚀 OpenFOAM MCP Server v1.0.0 (Minimal Build)" << std::endl;
    
    try {
        // Initialize MCP server
        McpServer server;
        
        // Set server information
        ServerInfo info;
        info.name = "OpenFOAM MCP Server (Minimal)";
        info.version = "1.0.0";
        info.description = "Minimal MCP server for testing";
        server.setServerInfo(info);
        
        std::cerr << "🌟 Minimal MCP Server ready!" << std::endl;
        std::cerr << "📡 Listening on stdin for MCP protocol messages..." << std::endl;
        
        // Start server (blocking)
        server.start();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error starting MCP server: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
