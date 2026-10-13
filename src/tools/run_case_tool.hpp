#ifndef RUN_CASE_TOOL_HPP
#define RUN_CASE_TOOL_HPP

#include <memory>

#include "mcp/server.hpp"
#include "openfoam/case_manager.hpp"

namespace Foam::MCP {

void registerRunCaseTool(McpServer& server);

}  // namespace Foam::MCP

#endif
