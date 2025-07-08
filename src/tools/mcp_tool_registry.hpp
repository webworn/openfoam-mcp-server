/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Complete MCP Tool Registry
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Complete registry of all MCP tools for OpenFOAM CFD analysis.
    Covers the entire universe of OpenFOAM applications and user scenarios.

    Each tool understands user intent and translates to proper OpenFOAM setup.

\*---------------------------------------------------------------------------*/

#ifndef mcp_tool_registry_H
    #define mcp_tool_registry_H

    #include <functional>
    #include <map>
    #include <memory>
    #include <string>
    #include <vector>

namespace Foam
{
namespace MCP
{

// Forward declarations
class MCPTool;
class PhysicsIntent;

/*---------------------------------------------------------------------------*\
                        Enum ToolCategory
\*---------------------------------------------------------------------------*/

enum class ToolCategory
{
    // Flow Analysis
    EXTERNAL_FLOW,      // Cars, aircraft, buildings
    INTERNAL_FLOW,      // Pipes, ducts, pumps
    COMPRESSIBLE_FLOW,  // High-speed, shock waves
    MULTIPHASE_FLOW,    // Free surface, bubbly flow

    // Heat Transfer
    HEAT_TRANSFER,     // Conjugate HT, natural convection
    THERMAL_ANALYSIS,  // Electronics cooling, HVAC

    // Specialized Physics
    COMBUSTION,        // Reactive flows, fire
    TURBULENT_MIXING,  // LES, mixing analysis
    FLUID_STRUCTURE,   // FSI, aeroelasticity
    ELECTROMAGNETICS,  // MHD, electrochemistry

    // Industry-Specific
    AUTOMOTIVE,     // Vehicle-specific tools
    AEROSPACE,      // Aircraft-specific tools
    ENERGY,         // Turbines, renewable energy
    PROCESS,        // Chemical industry
    MARINE,         // Ships, offshore
    CIVIL,          // Buildings, infrastructure
    BIOMEDICAL,     // Blood flow, drug delivery
    ENVIRONMENTAL,  // Pollution, atmosphere

    // Analysis Types
    FORCES_ANALYSIS,       // Drag, lift, moments
    PERFORMANCE_ANALYSIS,  // Efficiency, optimization
    SAFETY_ASSESSMENT,     // Wind loads, explosions
    RESEARCH_TOOLS,        // DNS, fundamental studies
    QUICK_ESTIMATES        // Fast approximations
};

/*---------------------------------------------------------------------------*\
                     Struct ToolCapability
\*---------------------------------------------------------------------------*/

struct ToolCapability
{
    std::string name;
    std::string description;
    std::vector<std::string> keywords;
    std::vector<std::string> solvers;
    std::vector<std::string> applications;
    ToolCategory category;
    int complexityLevel;        // 1=simple, 5=research-grade
    int typicalRuntime;         // seconds
    std::string accuracyLevel;  // "quick", "engineering", "research"
};

/*---------------------------------------------------------------------------*\
                   Class MCPToolRegistry Declaration
\*---------------------------------------------------------------------------*/

class MCPToolRegistry
{
  private:
    // Registry of all available tools
    std::map<std::string, ToolCapability> tools_;

    // Category mappings for intelligent tool selection
    std::map<ToolCategory, std::vector<std::string>> categoryTools_;

    // Keyword-to-tool mappings for natural language understanding
    std::map<std::string, std::vector<std::string>> keywordMap_;

  public:
    // Constructor - registers all tools
    MCPToolRegistry();

    // Tool discovery
    std::vector<std::string> getAllTools() const;
    std::vector<std::string> getToolsByCategory(ToolCategory category) const;
    std::vector<std::string> findToolsByKeywords(const std::vector<std::string>& keywords) const;
    std::vector<std::string> findToolsByDescription(const std::string& description) const;

    // Tool information
    ToolCapability getToolCapability(const std::string& toolName) const;
    bool isToolAvailable(const std::string& toolName) const;

    // Intelligent tool selection
    std::string recommendTool(const PhysicsIntent& intent) const;
    std::vector<std::string> suggestAlternativeTools(const std::string& toolName) const;

    // Natural language interface
    std::vector<std::string> interpretUserRequest(const std::string& request) const;
    std::string explainToolChoice(const std::string& toolName, const std::string& request) const;

  private:
    // Tool registration methods
    void registerExternalFlowTools();
    void registerInternalFlowTools();
    void registerCompressibleFlowTools();
    void registerMultiphaseFlowTools();
    void registerHeatTransferTools();
    void registerCombustionTools();
    void registerSpecializedTools();
    void registerIndustryTools();
    void registerAnalysisTools();
    void registerQuickEstimateTools();

    // Helper methods
    void registerTool(const ToolCapability& tool);
    void addKeywordMapping(const std::string& keyword, const std::string& toolName);
};

/*---------------------------------------------------------------------------*\
                   Complete Tool Definitions
\*---------------------------------------------------------------------------*/

// External Flow Analysis Tools
namespace ExternalFlow
{
// analyze_vehicle_aerodynamics
const ToolCapability VEHICLE_AERODYNAMICS = {
    "analyze_vehicle_aerodynamics",
    "Analyze external flow around cars, trucks, motorcycles for drag/lift forces",
    {"car", "vehicle", "automobile", "aerodynamics", "drag", "external flow"},
    {"simpleFoam", "pimpleFoam", "SRFSimpleFoam"},
    {"automotive", "motorsports", "transport"},
    ToolCategory::EXTERNAL_FLOW,
    3,
    7200,
    "engineering"
};

// analyze_aircraft_aerodynamics
const ToolCapability AIRCRAFT_AERODYNAMICS = {
    "analyze_aircraft_aerodynamics",
    "Aircraft wing, fuselage, and complete configuration aerodynamic analysis",
    {"aircraft", "airplane", "wing", "airfoil", "flight", "aviation"},
    {"simpleFoam", "rhoPimpleFoam", "SpalartAllmaras"},
    {"aerospace", "aviation", "defense"},
    ToolCategory::EXTERNAL_FLOW,
    4,
    14400,
    "engineering"
};

// analyze_building_wind_loads
const ToolCapability BUILDING_WIND_LOADS = {
    "analyze_building_wind_loads",
    "Wind loading analysis for buildings, structures, and urban environments",
    {"building", "structure", "wind loads", "urban", "architecture"},
    {"pimpleFoam", "LES", "kOmegaSST"},
    {"civil engineering", "architecture", "structural"},
    ToolCategory::EXTERNAL_FLOW,
    3,
    10800,
    "engineering"
};

// analyze_sports_aerodynamics
const ToolCapability SPORTS_AERODYNAMICS = {
    "analyze_sports_aerodynamics",
    "Aerodynamic analysis for sports applications (cycling, running, equipment)",
    {"sports", "cycling", "athlete", "equipment", "performance"},
    {"simpleFoam", "pimpleFoam"},
    {"sports", "equipment design", "performance"},
    ToolCategory::EXTERNAL_FLOW,
    2,
    3600,
    "engineering"
};
}  // namespace ExternalFlow

// Internal Flow Analysis Tools
namespace InternalFlow
{
// analyze_pipe_flow
const ToolCapability PIPE_FLOW = {
    "analyze_pipe_flow",
    "Pressure drop and flow analysis in pipes, ducts, and conduits",
    {"pipe", "duct", "conduit", "pressure drop", "flow rate"},
    {"simpleFoam", "pimpleFoam"},
    {"HVAC", "plumbing", "process", "oil gas"},
    ToolCategory::INTERNAL_FLOW,
    1,
    900,
    "engineering"
};

// analyze_pump_performance
const ToolCapability PUMP_PERFORMANCE = {
    "analyze_pump_performance",
    "Centrifugal and axial pump performance analysis and optimization",
    {"pump", "impeller", "centrifugal", "axial", "performance"},
    {"simpleFoam", "MRFSimpleFoam", "SRFSimpleFoam"},
    {"pumps", "turbomachinery", "water treatment"},
    ToolCategory::INTERNAL_FLOW,
    3,
    5400,
    "engineering"
};

// analyze_valve_flow
const ToolCapability VALVE_FLOW = {
    "analyze_valve_flow",
    "Flow analysis through valves, orifices, and flow control devices",
    {"valve", "orifice", "flow control", "throttling"},
    {"simpleFoam", "pimpleFoam"},
    {"process", "oil gas", "water", "HVAC"},
    ToolCategory::INTERNAL_FLOW,
    2,
    1800,
    "engineering"
};

// optimize_duct_design
const ToolCapability DUCT_DESIGN = {
    "optimize_duct_design",
    "HVAC duct design optimization for minimum pressure loss",
    {"duct", "HVAC", "ventilation", "air flow", "design"},
    {"simpleFoam", "buoyantSimpleFoam"},
    {"HVAC", "building services", "ventilation"},
    ToolCategory::INTERNAL_FLOW,
    2,
    3600,
    "engineering"
};
}  // namespace InternalFlow

// Compressible Flow Tools
namespace CompressibleFlow
{
// analyze_supersonic_flow
const ToolCapability SUPERSONIC_FLOW = {
    "analyze_supersonic_flow",
    "High-speed compressible flow analysis with shock waves",
    {"supersonic", "hypersonic", "shock", "compressible", "high speed"},
    {"rhoCentralFoam", "sonicFoam"},
    {"aerospace", "defense", "propulsion"},
    ToolCategory::COMPRESSIBLE_FLOW,
    4,
    18000,
    "research"
};

// analyze_nozzle_flow
const ToolCapability NOZZLE_FLOW = {
    "analyze_nozzle_flow",
    "Nozzle flow analysis for propulsion and gas dynamics applications",
    {"nozzle", "jet", "propulsion", "gas dynamics", "thrust"},
    {"rhoPimpleFoam", "rhoCentralFoam"},
    {"aerospace", "propulsion", "gas turbines"},
    ToolCategory::COMPRESSIBLE_FLOW,
    3,
    7200,
    "engineering"
};

// analyze_transonic_flow
const ToolCapability TRANSONIC_FLOW = {
    "analyze_transonic_flow",
    "Transonic flow analysis with mixed subsonic/supersonic regions",
    {"transonic", "mixed flow", "critical", "sonic"},
    {"rhoPimpleFoam", "sonicFoam"},
    {"aerospace", "turbomachinery"},
    ToolCategory::COMPRESSIBLE_FLOW,
    4,
    14400,
    "engineering"
};
}  // namespace CompressibleFlow

// Multiphase Flow Tools
namespace MultiphaseFlow
{
// analyze_free_surface_flow
const ToolCapability FREE_SURFACE_FLOW = {
    "analyze_free_surface_flow",
    "Free surface flows using VOF method (waves, sloshing, breaking)",
    {"free surface", "waves", "sloshing", "VOF", "water", "breaking"},
    {"interFoam", "multiphaseInterFoam"},
    {"marine", "offshore", "hydraulics", "coastal"},
    ToolCategory::MULTIPHASE_FLOW,
    3,
    10800,
    "engineering"
};

// analyze_bubble_flow
const ToolCapability BUBBLE_FLOW = {
    "analyze_bubble_flow",
    "Bubbly flow analysis using Eulerian-Eulerian approach",
    {"bubble", "bubbly", "gas liquid", "two phase", "Eulerian"},
    {"twoPhaseEulerFoam", "multiphaseEulerFoam"},
    {"chemical", "process", "water treatment"},
    ToolCategory::MULTIPHASE_FLOW,
    4,
    14400,
    "engineering"
};

// analyze_cavitation
const ToolCapability CAVITATION = {
    "analyze_cavitation",
    "Cavitation analysis in pumps, propellers, and hydraulic devices",
    {"cavitation", "vapor bubbles", "pump", "propeller", "hydraulic"},
    {"cavitatingFoam", "interPhaseChangeFoam"},
    {"marine", "pumps", "hydraulics"},
    ToolCategory::MULTIPHASE_FLOW,
    4,
    18000,
    "engineering"
};

// analyze_spray_injection
const ToolCapability SPRAY_INJECTION = {
    "analyze_spray_injection",
    "Spray and injection analysis using Lagrangian particle tracking",
    {"spray", "injection", "droplets", "atomization", "Lagrangian"},
    {"sprayFoam", "reactingFoam"},
    {"automotive", "combustion", "pharmaceutical"},
    ToolCategory::MULTIPHASE_FLOW,
    4,
    21600,
    "engineering"
};
}  // namespace MultiphaseFlow

// Heat Transfer Tools
namespace HeatTransfer
{
// analyze_heat_exchanger
const ToolCapability HEAT_EXCHANGER = {
    "analyze_heat_exchanger",
    "Heat exchanger thermal and hydraulic performance analysis",
    {"heat exchanger", "thermal", "cooling", "heating", "HX"},
    {"chtMultiRegionFoam", "conjugateHeatFoam"},
    {"HVAC", "process", "power", "automotive"},
    ToolCategory::HEAT_TRANSFER,
    3,
    10800,
    "engineering"
};

// analyze_natural_convection
const ToolCapability NATURAL_CONVECTION = {
    "analyze_natural_convection",
    "Natural convection heat transfer analysis",
    {"natural convection", "buoyancy", "Rayleigh", "free convection"},
    {"buoyantPimpleFoam", "buoyantSimpleFoam"},
    {"electronics", "building", "HVAC"},
    ToolCategory::HEAT_TRANSFER,
    2,
    5400,
    "engineering"
};

// analyze_electronics_cooling
const ToolCapability ELECTRONICS_COOLING = {
    "analyze_electronics_cooling",
    "Electronics cooling analysis for thermal management",
    {"electronics", "CPU", "cooling", "thermal management", "heat sink"},
    {"chtMultiRegionFoam", "buoyantPimpleFoam"},
    {"electronics", "computers", "data centers"},
    ToolCategory::HEAT_TRANSFER,
    3,
    7200,
    "engineering"
};

// analyze_solar_thermal
const ToolCapability SOLAR_THERMAL = {
    "analyze_solar_thermal",
    "Solar thermal collector and concentrator analysis",
    {"solar", "thermal collector", "concentrator", "renewable"},
    {"chtMultiRegionFoam", "solarLoadFoam"},
    {"renewable energy", "solar", "thermal"},
    ToolCategory::HEAT_TRANSFER,
    3,
    9000,
    "engineering"
};
}  // namespace HeatTransfer

// Continue with other categories...
// This would expand to include all tool categories from the universe document

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //