/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | JSON-RPC 2.0 Handler
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    JSON-RPC 2.0 message handling implementation

\*---------------------------------------------------------------------------*/

#include "json_rpc.hpp"

#include <iostream>

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const JsonRpcError& error)
{
    j = json{
        {"code",    error.code   },
        {"message", error.message}
    };

    if (error.data.has_value()) {
        j["data"] = error.data.value();
    }
}

void from_json(const json& j, JsonRpcError& error)
{
    j.at("code").get_to(error.code);
    j.at("message").get_to(error.message);

    if (j.contains("data")) {
        error.data = j.at("data");
    }
}

/*---------------------------------------------------------------------------*\
                        JsonRpcHandler Implementation
\*---------------------------------------------------------------------------*/

bool JsonRpcHandler::validateJsonRpcMessage(const json& message) const
{
    if (!message.is_object()) {
        return false;
    }

    if (!message.contains("jsonrpc") || message["jsonrpc"] != "2.0") {
        return false;
    }

    return true;
}

JsonRpcMessage JsonRpcHandler::parseMessage(const json& message) const
{
    JsonRpcMessage msg;

    if (!validateJsonRpcMessage(message)) {
        return msg;
    }

    msg.jsonrpc = "2.0";

    if (message.contains("id")) {
        msg.id = message["id"];
    }

    if (message.contains("method")) {
        msg.method = message["method"];

        if (message.contains("id")) {
            msg.type = MessageType::REQUEST;
        } else {
            msg.type = MessageType::NOTIFICATION;
        }
    } else if (message.contains("result") || message.contains("error")) {
        msg.type = MessageType::RESPONSE;

        if (message.contains("result")) {
            msg.result = message["result"];
        }

        if (message.contains("error")) {
            msg.error = message["error"];
        }
    } else {
        return msg;
    }

    if (message.contains("params")) {
        msg.params = message["params"];
    }

    return msg;
}

json JsonRpcHandler::createResponse(const json& id, const json& result) const
{
    return json{
        {"jsonrpc", "2.0" },
        {"id",      id    },
        {"result",  result}
    };
}

json JsonRpcHandler::createErrorResponse(const json& id, const JsonRpcError& error) const
{
    json errorJson;
    to_json(errorJson, error);

    return json{
        {"jsonrpc", "2.0"    },
        {"id",      id       },
        {"error",   errorJson}
    };
}

void JsonRpcHandler::registerRequestHandler(const std::string& method, RequestHandler handler)
{
    requestHandlers_[method] = handler;
}

void JsonRpcHandler::registerNotificationHandler(const std::string& method,
                                                 NotificationHandler handler)
{
    notificationHandlers_[method] = handler;
}

std::string JsonRpcHandler::processMessage(const std::string& messageStr)
{
    try {
        json message = json::parse(messageStr);
        json response = processJsonMessage(message);

        if (response.is_null()) {
            return "";
        }

        return response.dump();
    } catch (const json::parse_error& e) {
        json errorResponse = createErrorResponse(nullptr, JsonRpcError::parseError());
        return errorResponse.dump();
    }
}

json JsonRpcHandler::processJsonMessage(const json& message)
{
    JsonRpcMessage msg = parseMessage(message);

    if (!msg.isValid()) {
        return createErrorResponse(msg.id.value_or(nullptr), JsonRpcError::invalidRequest());
    }

    if (msg.isRequest()) {
        const std::string& method = msg.method.value();

        if (!hasRequestHandler(method)) {
            return createErrorResponse(msg.id.value(), JsonRpcError::methodNotFound());
        }

        try {
            json params = msg.params.value_or(json::object());
            json result = requestHandlers_[method](params);
            return createResponse(msg.id.value(), result);
        } catch (const std::exception& e) {
            JsonRpcError error = JsonRpcError::internalError();
            error.data = e.what();
            return createErrorResponse(msg.id.value(), error);
        }
    } else if (msg.isNotification()) {
        const std::string& method = msg.method.value();

        if (hasNotificationHandler(method)) {
            try {
                json params = msg.params.value_or(json::object());
                notificationHandlers_[method](params);
            } catch (const std::exception& e) {
                std::cerr << "Error handling notification " << method << ": " << e.what()
                          << std::endl;
            }
        }

        return json();
    }

    return createErrorResponse(msg.id.value_or(nullptr), JsonRpcError::invalidRequest());
}

bool JsonRpcHandler::hasRequestHandler(const std::string& method) const
{
    return requestHandlers_.find(method) != requestHandlers_.end();
}

bool JsonRpcHandler::hasNotificationHandler(const std::string& method) const
{
    return notificationHandlers_.find(method) != notificationHandlers_.end();
}

std::vector<std::string> JsonRpcHandler::getRegisteredMethods() const
{
    std::vector<std::string> methods;

    for (const auto& [method, handler] : requestHandlers_) {
        methods.push_back(method);
    }

    for (const auto& [method, handler] : notificationHandlers_) {
        methods.push_back(method);
    }

    return methods;
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //