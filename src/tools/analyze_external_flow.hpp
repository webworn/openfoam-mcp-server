/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | External Flow Analysis Tool
    \\  /    A nd           | 
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Header for External Flow Analysis Tool
    
    Demonstrates the intent-to-OpenFOAM translation concept where user
    requirements are understood and converted to proper CFD setup.

\*---------------------------------------------------------------------------*/

#ifndef analyze_external_flow_H
#define analyze_external_flow_H

#include <string>
#include <map>
#include <vector>

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                       Struct PhysicsParameters
\*---------------------------------------------------------------------------*/

struct PhysicsParameters
{
    double reynoldsNumber;
    double machNumber;
    std::string flowRegime;        // "laminar", "turbulent", "transitional"
    std::string compressibility;   // "incompressible", "subsonic_compressible", etc.
};

/*---------------------------------------------------------------------------*\
                       Struct MeshParameters
\*---------------------------------------------------------------------------*/

struct MeshParameters
{
    int targetCells;
    double targetYPlus;
    int refinementLevels;
    int boundaryLayerLayers;
};

/*---------------------------------------------------------------------------*\
                       Struct OpenFOAMSetup
\*---------------------------------------------------------------------------*/

struct OpenFOAMSetup
{
    std::string solver;
    std::string turbulenceModel;
    MeshParameters meshParams;
    std::map<std::string, std::string> boundaryConditions;
    std::map<std::string, std::string> schemes;
};

/*---------------------------------------------------------------------------*\
                   Class ExternalFlowAnalyzer Declaration
\*---------------------------------------------------------------------------*/

class ExternalFlowAnalyzer
{
private:

    // User intent parameters
    std::string vehicleType_;       // "car", "aircraft", "building"
    double speed_;                  // m/s
    std::string fluidType_;         // "air", "water"
    double altitude_;               // m (for aircraft)
    double temperature_;            // K
    std::string objective_;         // "drag_analysis", "lift_analysis", etc.
    int timeConstraint_;            // seconds
    double accuracyTarget_;         // fractional (0.05 = 5%)
    
    // Physical geometry parameters
    double characteristicLength_;   // m
    double frontalArea_;           // m²
    double referenceHeight_;       // m

public:

    // Constructors
    ExternalFlowAnalyzer();
    
    // User intent processing
    bool parseUserIntent(const std::string& description);
    void setSpeed(double speed) { speed_ = speed; }
    void setObjective(const std::string& objective) { objective_ = objective; }
    void setTimeConstraint(int seconds) { timeConstraint_ = seconds; }
    void setAccuracyTarget(double accuracy) { accuracyTarget_ = accuracy; }
    
    // Physics analysis
    PhysicsParameters analyzePhysics() const;
    
    // OpenFOAM setup generation
    OpenFOAMSetup recommendOpenFOAMSetup() const;
    
    // Validation and warnings
    std::vector<std::string> validateSetup() const;
    
    // User communication
    std::string generateUserSummary() const;
    std::string explainRecommendations() const;
    
    // Example usage patterns this should handle:
    static ExternalFlowAnalyzer fromDescription(const std::string& description);

private:

    // Helper methods for different vehicle types
    void setDefaultCarParameters();
    void setDefaultAircraftParameters();
    void setDefaultBuildingParameters();
    
    // Physics calculations
    double getCharacteristicLength() const;
    
    // OpenFOAM setup helpers
    MeshParameters calculateMeshRequirements(const PhysicsParameters& physics) const;
    std::map<std::string, std::string> setupBoundaryConditions() const;
    std::map<std::string, std::string> selectNumericalSchemes() const;
};

} // End namespace MCP
} // End namespace Foam

#endif

// ************************************************************************* //