/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | CFD Assistant Tool Registration
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool registration for the intelligent CFD assistant

\*---------------------------------------------------------------------------*/

#include "cfd_assistant_tool.hpp"
#include "../mcp/server.hpp"

namespace Foam {
namespace MCP {

void registerCFDAssistantTool(McpServer& server) {
    // Create the CFD assistant instance
    static auto cfdAssistant = std::make_unique<CFDAssistantTool>();
    
    // Schema for starting CFD assistance
    json startAssistanceSchema = {
        {"type", "object"},
        {"properties", {
            {"user_input", {
                {"type", "string"},
                {"description", "User's initial description of CFD problem or question"}
            }},
            {"user_level", {
                {"type", "string"},
                {"enum", {"beginner", "intermediate", "expert"}},
                {"description", "User's experience level with CFD"},
                {"default", "beginner"}
            }}
        }},
        {"required", {"user_input"}}
    };
    
    // Schema for continuing conversation
    json continueConversationSchema = {
        {"type", "object"},
        {"properties", {
            {"response", {
                {"type", "string"},
                {"description", "User's response to assistant's question or guidance"}
            }},
            {"action", {
                {"type", "string"},
                {"enum", {"continue", "execute", "analyze", "help"}},
                {"description", "Type of action requested"},
                {"default", "continue"}
            }}
        }},
        {"required", {"response"}}
    };
    
    // Schema for OpenFOAM operations
    json executeOperationSchema = {
        {"type", "object"},
        {"properties", {
            {"operation", {
                {"type", "string"},
                {"enum", {"create_case", "generate_mesh", "run_solver", "post_process"}},
                {"description", "OpenFOAM operation to execute"}
            }},
            {"case_directory", {
                {"type", "string"},
                {"description", "Path to OpenFOAM case directory"}
            }},
            {"parameters", {
                {"type", "object"},
                {"description", "Additional parameters for the operation"}
            }}
        }},
        {"required", {"operation"}}
    };
    
    // Schema for result analysis
    json analyzeResultsSchema = {
        {"type", "object"},
        {"properties", {
            {"case_directory", {
                {"type", "string"},
                {"description", "Path to completed OpenFOAM case"}
            }},
            {"analysis_type", {
                {"type", "string"},
                {"enum", {"convergence", "forces", "visualization", "custom"}},
                {"description", "Type of analysis to perform"}
            }},
            {"visualization_request", {
                {"type", "string"},
                {"description", "Specific visualization or output requested"}
            }}
        }},
        {"required", {"case_directory"}}
    };
    
    // Register the CFD assistant tool
    server.registerTool(
        "start_cfd_assistance",
        "🚀 Start intelligent CFD analysis assistance with Socratic questioning and educational guidance. "
        "I'll help you understand your problem, create OpenFOAM cases, and interpret results through guided conversation.",
        startAssistanceSchema,
        [&](const json& arguments) -> ToolResult {
            try {
                std::string userInput = arguments.at("user_input").get<std::string>();
                
                // Set user level if provided
                if (arguments.contains("user_level")) {
                    std::string userLevel = arguments.at("user_level").get<std::string>();
                    // Store user level in assistant (would need to add this functionality)
                }
                
                return cfdAssistant->processUserInput(userInput);
                
            } catch (const std::exception& e) {
                ToolResult result;
                result.addErrorContent("Error starting CFD assistance: " + std::string(e.what()));
                return result;
            }
        }
    );
    
    // Register conversation continuation tool
    server.registerTool(
        "continue_cfd_conversation",
        "💬 Continue the CFD assistance conversation. Respond to questions, provide additional information, "
        "or request specific actions like case generation or execution.",
        continueConversationSchema,
        [&](const json& arguments) -> ToolResult {
            try {
                std::string response = arguments.at("response").get<std::string>();
                std::string action = arguments.value("action", "continue");
                
                if (action == "execute") {
                    // Handle execution requests
                    json execArgs = {{"operation", "create_case"}};
                    return cfdAssistant->executeOpenFOAMOperation(execArgs);
                } else if (action == "analyze") {
                    // Handle analysis requests
                    json analyzeArgs = {{"case_directory", "."}};
                    return cfdAssistant->analyzeResults(analyzeArgs);
                } else {
                    // Continue conversation
                    return cfdAssistant->processUserInput(response);
                }
                
            } catch (const std::exception& e) {
                ToolResult result;
                result.addErrorContent("Error in CFD conversation: " + std::string(e.what()));
                return result;
            }
        }
    );
    
    // Register OpenFOAM operation execution tool
    server.registerTool(
        "execute_openfoam_operation",
        "⚙️ Execute OpenFOAM operations in the Code Terminal with real-time progress monitoring. "
        "Operations include case creation, mesh generation, solver execution, and post-processing.",
        executeOperationSchema,
        [&](const json& arguments) -> ToolResult {
            try {
                return cfdAssistant->executeOpenFOAMOperation(arguments);
                
            } catch (const std::exception& e) {
                ToolResult result;
                result.addErrorContent("Error executing OpenFOAM operation: " + std::string(e.what()));
                return result;
            }
        }
    );
    
    // Register result analysis tool
    server.registerTool(
        "analyze_cfd_results",
        "📊 Analyze CFD simulation results with intelligent interpretation and visualization guidance. "
        "Provides insights into convergence, forces, flow patterns, and suggests next steps.",
        analyzeResultsSchema,
        [&](const json& arguments) -> ToolResult {
            try {
                return cfdAssistant->analyzeResults(arguments);
                
            } catch (const std::exception& e) {
                ToolResult result;
                result.addErrorContent("Error analyzing results: " + std::string(e.what()));
                return result;
            }
        }
    );
    
    // Register learning guidance tool
    server.registerTool(
        "get_cfd_learning_guidance",
        "🎓 Get educational guidance and explanations about CFD concepts, OpenFOAM usage, "
        "and best practices. Includes concept explanations and learning paths.",
        json{{"type", "object"}, {"properties", {
            {"conceptName", {
                {"type", "string"},
                {"description", "CFD concept to explain (reynolds_number, turbulence, boundary_conditions, etc.)"}
            }},
            {"learning_goal", {
                {"type", "string"},
                {"description", "What the user wants to learn or understand"}
            }}
        }}},
        [&](const json& arguments) -> ToolResult {
            try {
                return cfdAssistant->provideLearningGuidance(arguments);
                
            } catch (const std::exception& e) {
                ToolResult result;
                result.addErrorContent("Error providing learning guidance: " + std::string(e.what()));
                return result;
            }
        }
    );
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //