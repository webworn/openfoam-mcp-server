/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | MCP Server Core
    \\  /    A nd           | 
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Core Model Context Protocol server implementation

\*---------------------------------------------------------------------------*/

#include "server.hpp"
#include <iostream>
#include <sstream>

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const ServerInfo& info)
{
    j = json{
        {"name", info.name},
        {"version", info.version},
        {"description", info.description},
        {"author", info.author},
        {"homepage", info.homepage}
    };
}

void from_json(const json& j, ServerInfo& info)
{
    j.at("name").get_to(info.name);
    j.at("version").get_to(info.version);
    
    if (j.contains("description"))
        j.at("description").get_to(info.description);
    if (j.contains("author"))
        j.at("author").get_to(info.author);
    if (j.contains("homepage"))
        j.at("homepage").get_to(info.homepage);
}

void to_json(json& j, const Tool& tool)
{
    j = json{
        {"name", tool.name},
        {"description", tool.description},
        {"inputSchema", tool.inputSchema}
    };
}

void from_json(const json& j, Tool& tool)
{
    j.at("name").get_to(tool.name);
    j.at("description").get_to(tool.description);
    j.at("inputSchema").get_to(tool.inputSchema);
}

void to_json(json& j, const ToolResult& result)
{
    j = json{
        {"content", result.content}
    };
    
    if (result.isError)
    {
        j["isError"] = true;
    }
}

/*---------------------------------------------------------------------------*\
                        McpServer Implementation
\*---------------------------------------------------------------------------*/

McpServer::McpServer()
: jsonRpcHandler_(std::make_unique<JsonRpcHandler>())
, initialized_(false)
, running_(false)
, protocolVersion_("2024-11-05")
{
    setupStandardHandlers();
}

McpServer::~McpServer()
{
    stop();
}

void McpServer::setupStandardHandlers()
{
    jsonRpcHandler_->registerRequestHandler("initialize", 
        [this](const json& params) { return handleInitialize(params); });
    
    jsonRpcHandler_->registerNotificationHandler("initialized",
        [this](const json& params) { handleInitialized(params); });
    
    jsonRpcHandler_->registerRequestHandler("tools/list",
        [this](const json& params) { return handleToolsList(params); });
    
    jsonRpcHandler_->registerRequestHandler("tools/call",
        [this](const json& params) { return handleToolsCall(params); });
    
    jsonRpcHandler_->registerRequestHandler("ping",
        [this](const json& params) { return handlePing(params); });
}

json McpServer::handleInitialize(const json& params)
{
    if (!params.contains("protocolVersion"))
    {
        throw std::runtime_error("Missing protocolVersion in initialize request");
    }
    
    std::string clientProtocolVersion = params["protocolVersion"];
    
    if (params.contains("capabilities"))
    {
        clientCapabilities_ = clientCapabilitiesFromJson(params["capabilities"]);
    }
    
    json response = json{
        {"protocolVersion", protocolVersion_},
        {"capabilities", capabilitiesToJson(serverCapabilities_)},
        {"serverInfo", serverInfo_}
    };
    
    return response;
}

void McpServer::handleInitialized(const json& params)
{
    initialized_.store(true);
    std::cerr << "MCP server initialized successfully" << std::endl;
}

json McpServer::handleToolsList(const json& params)
{
    json toolsArray = json::array();
    
    for (const auto& [name, tool] : tools_)
    {
        toolsArray.push_back(tool);
    }
    
    return json{{"tools", toolsArray}};
}

json McpServer::handleToolsCall(const json& params)
{
    if (!params.contains("name"))
    {
        throw std::runtime_error("Missing tool name in tools/call request");
    }
    
    std::string toolName = params["name"];
    
    if (toolHandlers_.find(toolName) == toolHandlers_.end())
    {
        throw std::runtime_error("Tool not found: " + toolName);
    }
    
    json arguments = params.value("arguments", json::object());
    
    ToolResult result = toolHandlers_[toolName](arguments);
    
    return json(result);
}

json McpServer::handlePing(const json& params)
{
    return json::object();
}

json McpServer::capabilitiesToJson(const ServerCapabilities& caps) const
{
    json j = json::object();
    
    if (caps.tools.listChanged)
    {
        j["tools"] = json{{"listChanged", true}};
    }
    
    if (caps.resources.subscribe || caps.resources.listChanged)
    {
        json resources = json::object();
        if (caps.resources.subscribe)
            resources["subscribe"] = true;
        if (caps.resources.listChanged)
            resources["listChanged"] = true;
        j["resources"] = resources;
    }
    
    if (caps.prompts.listChanged)
    {
        j["prompts"] = json{{"listChanged", true}};
    }
    
    if (caps.logging.enabled)
    {
        j["logging"] = json::object();
    }
    
    return j;
}

ClientCapabilities McpServer::clientCapabilitiesFromJson(const json& caps) const
{
    ClientCapabilities clientCaps;
    
    if (caps.contains("roots") && caps["roots"].contains("listChanged"))
    {
        clientCaps.roots.listChanged = caps["roots"]["listChanged"];
    }
    
    if (caps.contains("sampling"))
    {
        clientCaps.sampling.enabled = caps["sampling"].get<bool>();
    }
    
    if (caps.contains("experimental"))
    {
        clientCaps.experimental = caps["experimental"];
    }
    
    return clientCaps;
}

void McpServer::registerTool(const std::string& name, const std::string& description, 
                            const json& inputSchema, ToolHandler handler)
{
    tools_[name] = Tool(name, description, inputSchema);
    toolHandlers_[name] = handler;
}

void McpServer::start()
{
    running_.store(true);
    std::cerr << "OpenFOAM MCP Server started. Listening on stdin..." << std::endl;
    
    while (running_.load())
    {
        processStdinMessage();
    }
}

void McpServer::stop()
{
    running_.store(false);
    std::cerr << "OpenFOAM MCP Server stopped." << std::endl;
}

void McpServer::processStdinMessage()
{
    std::string line;
    if (std::getline(std::cin, line))
    {
        if (line.empty())
        {
            return;
        }
        
        std::string response = jsonRpcHandler_->processMessage(line);
        
        if (!response.empty())
        {
            std::cout << response << std::endl;
            std::cout.flush();
        }
    }
    else
    {
        running_.store(false);
    }
}

std::vector<std::string> McpServer::getRegisteredTools() const
{
    std::vector<std::string> toolNames;
    
    for (const auto& [name, tool] : tools_)
    {
        toolNames.push_back(name);
    }
    
    return toolNames;
}

} // End namespace MCP
} // End namespace Foam

// ************************************************************************* //