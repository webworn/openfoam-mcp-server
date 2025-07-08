/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | MCP Server Core
    \\  /    A nd           | 
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Core Model Context Protocol server implementation
    
    Handles MCP lifecycle, capability negotiation, and tool registration
    with proper OpenFOAM integration.

\*---------------------------------------------------------------------------*/

#ifndef server_H
#define server_H

#include "json_rpc.hpp"
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <atomic>

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                        Struct ServerInfo
\*---------------------------------------------------------------------------*/

struct ServerInfo
{
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::string homepage;
    
    ServerInfo()
    : name("OpenFOAM MCP Server")
    , version("1.0.0")
    , description("World's first native OpenFOAM Model Context Protocol server")
    , author("OpenFOAM Community")
    , homepage("https://github.com/openfoam/mcp-server")
    {}
};

/*---------------------------------------------------------------------------*\
                        Struct ServerCapabilities
\*---------------------------------------------------------------------------*/

struct ServerCapabilities
{
    struct Tools
    {
        bool listChanged = false;
    } tools;
    
    struct Resources
    {
        bool subscribe = false;
        bool listChanged = false;
    } resources;
    
    struct Prompts
    {
        bool listChanged = false;
    } prompts;
    
    struct Logging
    {
        bool enabled = true;
    } logging;
};

/*---------------------------------------------------------------------------*\
                        Struct ClientCapabilities
\*---------------------------------------------------------------------------*/

struct ClientCapabilities
{
    struct Roots
    {
        bool listChanged = false;
    } roots;
    
    struct Sampling
    {
        bool enabled = false;
    } sampling;
    
    bool experimental = false;
};

/*---------------------------------------------------------------------------*\
                        Struct Tool
\*---------------------------------------------------------------------------*/

struct Tool
{
    std::string name;
    std::string description;
    json inputSchema;
    
    Tool() = default;
    Tool(const std::string& n, const std::string& desc, const json& schema)
    : name(n), description(desc), inputSchema(schema) {}
};

/*---------------------------------------------------------------------------*\
                        Struct ToolResult
\*---------------------------------------------------------------------------*/

struct ToolResult
{
    std::vector<json> content;
    bool isError = false;
    
    void addTextContent(const std::string& text)
    {
        content.push_back(json{{"type", "text"}, {"text", text}});
    }
    
    void addErrorContent(const std::string& error)
    {
        content.push_back(json{{"type", "text"}, {"text", error}});
        isError = true;
    }
};

/*---------------------------------------------------------------------------*\
                        Class McpServer Declaration
\*---------------------------------------------------------------------------*/

class McpServer
{
public:
    
    using ToolHandler = std::function<ToolResult(const json& arguments)>;

private:

    std::unique_ptr<JsonRpcHandler> jsonRpcHandler_;
    
    ServerInfo serverInfo_;
    ServerCapabilities serverCapabilities_;
    ClientCapabilities clientCapabilities_;
    
    std::map<std::string, Tool> tools_;
    std::map<std::string, ToolHandler> toolHandlers_;
    
    std::atomic<bool> initialized_;
    std::atomic<bool> running_;
    
    std::string protocolVersion_;
    
    void setupStandardHandlers();
    
    json handleInitialize(const json& params);
    void handleInitialized(const json& params);
    
    json handleToolsList(const json& params);
    json handleToolsCall(const json& params);
    
    json handlePing(const json& params);
    
    json capabilitiesToJson(const ServerCapabilities& caps) const;
    ClientCapabilities clientCapabilitiesFromJson(const json& caps) const;

public:

    McpServer();
    ~McpServer();
    
    void registerTool(const std::string& name, const std::string& description, 
                     const json& inputSchema, ToolHandler handler);
    
    void start();
    void stop();
    
    void processStdinMessage();
    
    bool isInitialized() const { return initialized_.load(); }
    bool isRunning() const { return running_.load(); }
    
    const ServerInfo& getServerInfo() const { return serverInfo_; }
    void setServerInfo(const ServerInfo& info) { serverInfo_ = info; }
    
    std::vector<std::string> getRegisteredTools() const;
};

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const ServerInfo& info);
void from_json(const json& j, ServerInfo& info);

void to_json(json& j, const Tool& tool);
void from_json(const json& j, Tool& tool);

void to_json(json& j, const ToolResult& result);

} // End namespace MCP
} // End namespace Foam

#endif

// ************************************************************************* //