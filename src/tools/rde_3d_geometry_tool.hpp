/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | RDE 3D Geometry Generator MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for 3D RDE geometry generation functionality.

    Generates complete 3D annular RDE geometry with inlet, combustor,
    and nozzle sections, including mesh specifications and performance targets.

\*---------------------------------------------------------------------------*/

#ifndef rde_3d_geometry_tool_H
#define rde_3d_geometry_tool_H

#include <memory>

#include "../mcp/server.hpp"
#include "rde_3d_geometry.hpp"

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Class RDE3DGeometryTool Declaration
\*---------------------------------------------------------------------------*/

class RDE3DGeometryTool {
private:
    std::unique_ptr<RDE3DGeometry> generator_;

    std::string formatResultsForUser(const RDE3DGeometry::Geometry3DResult& result) const;
    std::string generateEducationalContent(const RDE3DGeometry::Geometry3DResult& result) const;
    std::string generateValidationSummary(const RDE3DGeometry::Geometry3DResult& result) const;

public:
    RDE3DGeometryTool();
    explicit RDE3DGeometryTool(std::unique_ptr<RDE3DGeometry> generator);
    ~RDE3DGeometryTool() = default;

    static std::string getName() { return "generate_rde_3d_geometry"; }

    static std::string getDescription() {
        return "Generate complete 3D Rotating Detonation Engine geometry with annular combustor, "
               "inlet section, nozzle, and distributed injection ports. Creates blockMeshDict "
               "and provides performance predictions based on validated 2D baseline.";
    }

    static json getInputSchema() {
        return json{
            {"type", "object"},
            {"properties", {
                {"case_directory", {
                    {"type", "string"},
                    {"description", "Path to the OpenFOAM case directory for geometry output"}
                }},
                {"geometry", {
                    {"type", "object"},
                    {"properties", {
                        {"inner_radius", {{"type", "number"}, {"default", 0.050}, {"description", "Inner combustor radius in meters"}}},
                        {"outer_radius", {{"type", "number"}, {"default", 0.080}, {"description", "Outer combustor radius in meters"}}},
                        {"axial_length", {{"type", "number"}, {"default", 0.150}, {"description", "Combustor axial length in meters"}}},
                        {"inlet_length", {{"type", "number"}, {"default", 0.050}, {"description", "Inlet section length in meters"}}},
                        {"outlet_length", {{"type", "number"}, {"default", 0.100}, {"description", "Nozzle length in meters"}}},
                        {"nozzle_throat_dia", {{"type", "number"}, {"default", 0.060}, {"description", "Nozzle throat diameter in meters"}}},
                        {"nozzle_exit_dia", {{"type", "number"}, {"default", 0.090}, {"description", "Nozzle exit diameter in meters"}}},
                        {"num_injection_ports", {{"type", "integer"}, {"default", 36}, {"description", "Number of fuel injection ports"}}}
                    }},
                    {"description", "Geometry parameters for the 3D RDE"}
                }},
                {"mesh_spec", {
                    {"type", "object"},
                    {"properties", {
                        {"radial_cells", {{"type", "integer"}, {"default", 30}, {"description", "Cells in radial direction"}}},
                        {"circumferential_cells", {{"type", "integer"}, {"default", 180}, {"description", "Cells around circumference"}}},
                        {"axial_cells", {{"type", "integer"}, {"default", 150}, {"description", "Cells in axial direction"}}},
                        {"enable_boundary_layer", {{"type", "boolean"}, {"default", true}, {"description", "Enable boundary layer meshing"}}}
                    }},
                    {"description", "Mesh resolution specifications"}
                }},
                {"application_target", {
                    {"type", "string"},
                    {"enum", {"propulsion", "power", "research"}},
                    {"default", "propulsion"},
                    {"description", "Target application for the RDE design"}
                }}
            }},
            {"required", {"case_directory"}}
        };
    }

    ToolResult execute(const json& arguments);
};

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerRDE3DGeometryTool(McpServer& server);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //
