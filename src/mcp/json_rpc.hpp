/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | JSON-RPC 2.0 Handler
    \\  /    A nd           | 
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    JSON-RPC 2.0 message handling for Model Context Protocol
    
    Implements complete JSON-RPC 2.0 specification for MCP communication
    with proper request/response/notification handling.

\*---------------------------------------------------------------------------*/

#ifndef json_rpc_H
#define json_rpc_H

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace Foam
{
namespace MCP
{

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Enum MessageType
\*---------------------------------------------------------------------------*/

enum class MessageType
{
    REQUEST,
    RESPONSE,
    NOTIFICATION,
    INVALID
};

/*---------------------------------------------------------------------------*\
                        Struct JsonRpcMessage
\*---------------------------------------------------------------------------*/

struct JsonRpcMessage
{
    MessageType type;
    std::string jsonrpc;
    std::optional<json> id;
    std::optional<std::string> method;
    std::optional<json> params;
    std::optional<json> result;
    std::optional<json> error;
    
    JsonRpcMessage() : type(MessageType::INVALID), jsonrpc("2.0") {}
    
    bool isRequest() const { return type == MessageType::REQUEST; }
    bool isResponse() const { return type == MessageType::RESPONSE; }
    bool isNotification() const { return type == MessageType::NOTIFICATION; }
    bool isValid() const { return type != MessageType::INVALID; }
};

/*---------------------------------------------------------------------------*\
                        Struct JsonRpcError
\*---------------------------------------------------------------------------*/

struct JsonRpcError
{
    int code;
    std::string message;
    std::optional<json> data;
    
    static JsonRpcError parseError() {
        return {-32700, "Parse error", std::nullopt};
    }
    
    static JsonRpcError invalidRequest() {
        return {-32600, "Invalid Request", std::nullopt};
    }
    
    static JsonRpcError methodNotFound() {
        return {-32601, "Method not found", std::nullopt};
    }
    
    static JsonRpcError invalidParams() {
        return {-32602, "Invalid params", std::nullopt};
    }
    
    static JsonRpcError internalError() {
        return {-32603, "Internal error", std::nullopt};
    }
    
    static JsonRpcError serverError(int code, const std::string& message) {
        return {code, message, std::nullopt};
    }
};

/*---------------------------------------------------------------------------*\
                        Class JsonRpcHandler Declaration
\*---------------------------------------------------------------------------*/

class JsonRpcHandler
{
public:
    
    using RequestHandler = std::function<json(const json& params)>;
    using NotificationHandler = std::function<void(const json& params)>;

private:

    std::map<std::string, RequestHandler> requestHandlers_;
    std::map<std::string, NotificationHandler> notificationHandlers_;
    
    bool validateJsonRpcMessage(const json& message) const;
    JsonRpcMessage parseMessage(const json& message) const;
    json createResponse(const json& id, const json& result) const;
    json createErrorResponse(const json& id, const JsonRpcError& error) const;

public:

    JsonRpcHandler() = default;
    ~JsonRpcHandler() = default;
    
    void registerRequestHandler(const std::string& method, RequestHandler handler);
    void registerNotificationHandler(const std::string& method, NotificationHandler handler);
    
    std::string processMessage(const std::string& messageStr);
    
    json processJsonMessage(const json& message);
    
    bool hasRequestHandler(const std::string& method) const;
    bool hasNotificationHandler(const std::string& method) const;
    
    std::vector<std::string> getRegisteredMethods() const;
};

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const JsonRpcError& error);
void from_json(const json& j, JsonRpcError& error);

} // End namespace MCP
} // End namespace Foam

#endif

// ************************************************************************* //