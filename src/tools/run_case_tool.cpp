#include "run_case_tool.hpp"

namespace Foam::MCP {

void registerRunCaseTool(McpServer& server) {
    auto manager = std::make_shared<CaseManager>();
    const auto choices = manager->getAvailableSolvers();
    json solverChoices = json::array();
    for (const auto& choice : choices) solverChoices.push_back(choice);

    const json schema = {
        {"type", "object"},
        {"properties", {{"case_path", {{"type", "string"},
                                         {"description", "Absolute path to an existing OpenFOAM case directory visible to the MCP process."}}},
                         {"solver", {{"type", "string"},
                                     {"enum", solverChoices},
                                     {"description", "Solver executable or OpenFOAM v13 foamRun module command."}}}}},
        {"required", {"case_path", "solver"}}
    };

    server.registerTool(
        "run_openfoam_case",
        "Run an existing OpenFOAM case. Runs blockMesh when its dictionary exists, then snappyHexMesh when configured, validates with checkMesh, and launches the selected solver. For OpenFOAM 13 thermal cases, select `foamRun -solver fluid`; this runs an already prepared thermal case and does not create its geometry or physics dictionaries.",
        schema,
        [manager](const json& arguments) {
            ToolResult result;
            const std::string casePath = arguments.value("case_path", "");
            const std::string solver = arguments.value("solver", "");
            if (casePath.empty() || solver.empty()) {
                result.addErrorContent("Both case_path and solver are required.");
                return result;
            }

            std::string output;
            if (!manager->runCaseDirectory(casePath, solver, output)) {
                result.addErrorContent(output);
                return result;
            }
            result.addTextContent("OpenFOAM case completed successfully. Logs are saved in the case directory." + output);
            return result;
        });
}

}  // namespace Foam::MCP
