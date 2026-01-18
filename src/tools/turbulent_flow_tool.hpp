/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Turbulent Flow Analysis MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool for comprehensive turbulent flow analysis.

    Provides:
    - Reynolds number and flow regime classification
    - Turbulence model selection (k-epsilon, k-omega SST) with justification
    - Friction factor calculations (smooth and rough pipes)
    - Pressure drop predictions using Darcy-Weisbach
    - y+ estimation and mesh recommendations
    - Wall function validity assessment
    - Turbulent inlet condition calculations
    - Educational content about turbulence physics

\*---------------------------------------------------------------------------*/

#ifndef turbulent_flow_tool_H
#define turbulent_flow_tool_H

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "../mcp/server.hpp"
#include "../openfoam/pipe_flow.hpp"
#include "../openfoam/turbulence_utils.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Struct TurbulentFlowAnalysisInput
\*---------------------------------------------------------------------------*/

struct TurbulentFlowAnalysisInput {
    // Geometry
    std::string geometryType;     // "pipe", "channel", "external"
    double diameter;              // For pipe (m)
    double length;                // For pipe/channel (m)
    double hydraulicDiameter;     // For channel (m)
    double roughness;             // Absolute roughness (m)
    std::string pipeMaterial;     // Material name for roughness lookup

    // Flow conditions
    double velocity;              // Mean velocity (m/s)
    double density;               // Fluid density (kg/m³)
    double kinematicViscosity;    // Kinematic viscosity (m²/s)

    // Turbulence settings
    std::string turbulenceModel;  // "auto", "kEpsilon", "kOmegaSST"
    double turbulentIntensity;    // 0-1 (e.g., 0.05 = 5%)
    double turbulentLengthScale;  // meters

    // Mesh info (optional)
    double firstCellHeight;       // First cell height (m), 0 = estimate
    double targetYPlus;           // Target y+ for mesh (0 = auto)

    // Analysis mode
    std::string analysisMode;     // "quick", "detailed", "validation"

    TurbulentFlowAnalysisInput()
        : geometryType("pipe"),
          diameter(0.1),
          length(1.0),
          hydraulicDiameter(0.0),
          roughness(0.0),
          pipeMaterial("smooth"),
          velocity(1.0),
          density(1000.0),
          kinematicViscosity(1e-6),
          turbulenceModel("auto"),
          turbulentIntensity(0.05),
          turbulentLengthScale(0.0),
          firstCellHeight(0.0),
          targetYPlus(0.0),
          analysisMode("detailed") {}
};

/*---------------------------------------------------------------------------*\
                        Struct TurbulentFlowAnalysisResult
\*---------------------------------------------------------------------------*/

struct TurbulentFlowAnalysisResult {
    // Flow characterization
    double reynoldsNumber;
    std::string flowRegime;           // "laminar", "transitional", "turbulent"
    bool isFullyTurbulent;
    double relativeRoughness;

    // Turbulence model recommendation
    std::string recommendedModel;
    std::string modelJustification;
    std::vector<std::string> modelAlternatives;

    // Friction factor results
    double frictionFactorSmooth;      // Blasius/laminar (smooth pipe)
    double frictionFactorRough;       // Colebrook-White (with roughness)
    std::string frictionMethod;       // Method used for calculation

    // Pressure drop
    double pressureDrop;              // Pa
    double headLoss;                  // m

    // y+ and mesh recommendations
    double estimatedYPlus;
    double recommendedFirstCellHeight;
    double recommendedTargetYPlus;
    WallFunctionValidity wallFunctionStatus;

    // Turbulent inlet conditions
    TurbulenceInletConditions inletConditions;

    // Wall shear stress
    double wallShearStress;           // Pa
    double frictionVelocity;          // m/s

    // Model comparison
    struct ModelComparison {
        std::string model;
        std::string strengths;
        std::string weaknesses;
        std::string bestFor;
        double computationalCost;     // Relative (1.0 = baseline)
    };
    std::vector<ModelComparison> modelComparisons;

    // Educational content
    std::vector<std::string> physicsExplanations;
    std::vector<std::string> socraticQuestions;
    std::vector<std::string> practicalTips;
    std::vector<std::string> references;

    // Status
    bool success;
    std::string errorMessage;

    TurbulentFlowAnalysisResult()
        : reynoldsNumber(0.0),
          flowRegime("unknown"),
          isFullyTurbulent(false),
          relativeRoughness(0.0),
          frictionFactorSmooth(0.0),
          frictionFactorRough(0.0),
          pressureDrop(0.0),
          headLoss(0.0),
          estimatedYPlus(0.0),
          recommendedFirstCellHeight(0.0),
          recommendedTargetYPlus(50.0),
          wallShearStress(0.0),
          frictionVelocity(0.0),
          success(false) {}
};

/*---------------------------------------------------------------------------*\
                        Class TurbulentFlowTool Declaration
\*---------------------------------------------------------------------------*/

class TurbulentFlowTool {
   private:
    // Core analysis functions
    TurbulentFlowAnalysisResult performAnalysis(const TurbulentFlowAnalysisInput& input);

    // Flow characterization
    void characterizeFlow(TurbulentFlowAnalysisInput& input,
                          TurbulentFlowAnalysisResult& result);

    // Turbulence model selection
    void selectTurbulenceModel(const TurbulentFlowAnalysisInput& input,
                               TurbulentFlowAnalysisResult& result);

    // Friction factor calculations
    void calculateFrictionFactors(const TurbulentFlowAnalysisInput& input,
                                  TurbulentFlowAnalysisResult& result);

    // y+ and mesh analysis
    void analyzeMeshRequirements(const TurbulentFlowAnalysisInput& input,
                                  TurbulentFlowAnalysisResult& result);

    // Inlet conditions
    void calculateInletConditions(const TurbulentFlowAnalysisInput& input,
                                   TurbulentFlowAnalysisResult& result);

    // Model comparison
    void generateModelComparison(TurbulentFlowAnalysisResult& result);

    // Educational content generation
    void generateEducationalContent(const TurbulentFlowAnalysisInput& input,
                                    TurbulentFlowAnalysisResult& result);

    // Output formatting
    std::string formatResultsForUser(const TurbulentFlowAnalysisInput& input,
                                     const TurbulentFlowAnalysisResult& result) const;

    std::string formatEducationalContent(const TurbulentFlowAnalysisResult& result) const;

    std::string formatRecommendations(const TurbulentFlowAnalysisResult& result) const;

   public:
    TurbulentFlowTool() = default;
    ~TurbulentFlowTool() = default;

    static std::string getName() { return "analyze_turbulent_flow"; }

    static std::string getDescription() {
        return "Comprehensive turbulent flow analysis tool for CFD simulations. "
               "Provides Reynolds number classification, turbulence model recommendations "
               "(k-epsilon vs k-omega SST), friction factor calculations for smooth and rough pipes "
               "(Colebrook-White equation), pressure drop predictions, y+ estimation, "
               "mesh recommendations, and turbulent inlet boundary condition calculations. "
               "Includes educational explanations of turbulence physics.";
    }

    static json getInputSchema() {
        return json{
            {"type", "object"},
            {"properties", {
                {"case_directory", {
                    {"type", "string"},
                    {"description", "Path to OpenFOAM case directory (optional, for case setup)"}
                }},
                {"geometry", {
                    {"type", "object"},
                    {"properties", {
                        {"type", {
                            {"type", "string"},
                            {"enum", {"pipe", "channel", "external"}},
                            {"default", "pipe"},
                            {"description", "Geometry type for analysis"}
                        }},
                        {"diameter", {
                            {"type", "number"},
                            {"minimum", 0.0001},
                            {"description", "Pipe diameter in meters (for pipe geometry)"}
                        }},
                        {"length", {
                            {"type", "number"},
                            {"minimum", 0.001},
                            {"description", "Pipe/channel length in meters"}
                        }},
                        {"hydraulic_diameter", {
                            {"type", "number"},
                            {"minimum", 0.0001},
                            {"description", "Hydraulic diameter in meters (for channel geometry)"}
                        }},
                        {"roughness", {
                            {"type", "number"},
                            {"minimum", 0.0},
                            {"default", 0.0},
                            {"description", "Absolute pipe roughness in meters (0 = smooth)"}
                        }},
                        {"pipe_material", {
                            {"type", "string"},
                            {"enum", {"smooth", "commercial_steel", "cast_iron", "galvanized_iron",
                                     "concrete", "pvc", "copper", "stainless_steel"}},
                            {"description", "Pipe material for automatic roughness lookup"}
                        }}
                    }},
                    {"required", {"diameter"}}
                }},
                {"flow_conditions", {
                    {"type", "object"},
                    {"properties", {
                        {"velocity", {
                            {"type", "number"},
                            {"minimum", 0.001},
                            {"description", "Mean flow velocity in m/s"}
                        }},
                        {"density", {
                            {"type", "number"},
                            {"minimum", 0.1},
                            {"default", 1000.0},
                            {"description", "Fluid density in kg/m³ (default: water)"}
                        }},
                        {"viscosity", {
                            {"type", "number"},
                            {"minimum", 1e-8},
                            {"default", 1e-6},
                            {"description", "Kinematic viscosity in m²/s (default: water at 20°C)"}
                        }}
                    }},
                    {"required", {"velocity"}}
                }},
                {"turbulence_model", {
                    {"type", "string"},
                    {"enum", {"auto", "kEpsilon", "kOmegaSST", "laminar"}},
                    {"default", "auto"},
                    {"description", "Turbulence model selection (auto = automatic recommendation)"}
                }},
                {"turbulence_parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"intensity", {
                            {"type", "number"},
                            {"minimum", 0.001},
                            {"maximum", 1.0},
                            {"default", 0.05},
                            {"description", "Turbulent intensity (0-1, e.g., 0.05 = 5%)"}
                        }},
                        {"length_scale", {
                            {"type", "number"},
                            {"minimum", 0.0001},
                            {"description", "Turbulent length scale in meters (auto-calculated if not provided)"}
                        }}
                    }}
                }},
                {"mesh_parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"first_cell_height", {
                            {"type", "number"},
                            {"minimum", 0.0},
                            {"description", "First cell height in meters (0 = calculate recommendation)"}
                        }},
                        {"target_y_plus", {
                            {"type", "number"},
                            {"minimum", 0.0},
                            {"description", "Target y+ value (0 = automatic based on model)"}
                        }}
                    }}
                }},
                {"analysis_mode", {
                    {"type", "string"},
                    {"enum", {"quick", "detailed", "validation"}},
                    {"default", "detailed"},
                    {"description", "Analysis depth: quick (basic), detailed (full), validation (with benchmarks)"}
                }}
            }},
            {"required", {"geometry", "flow_conditions"}}
        };
    }

    ToolResult execute(const json& arguments);

    // Utility function to parse input from JSON
    static TurbulentFlowAnalysisInput parseInput(const json& arguments);

    // Convert results to JSON
    static json resultsToJson(const TurbulentFlowAnalysisResult& result);
};

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerTurbulentFlowTool(McpServer& server);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //
