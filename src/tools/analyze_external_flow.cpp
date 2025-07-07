/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | External Flow Analysis Tool
    \\  /    A nd           | 
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP Tool: analyze_external_flow
    
    Understands user intent for external flow analysis (cars, aircraft, buildings)
    and translates this into proper OpenFOAM case setup.
    
    User says: "I want to analyze airflow around a car"
    MCP understands: External aerodynamics, drag analysis, highway speeds
    OpenFOAM gets: Proper solver, mesh, BC setup for vehicle aerodynamics

\*---------------------------------------------------------------------------*/

#include "analyze_external_flow.hpp"
#include "physics_intent.hpp"
#include "argList.H"
#include "Time.H"
#include <iostream>
#include <map>
#include <vector>

using namespace Foam;
using namespace Foam::MCP;

/*---------------------------------------------------------------------------*\
                    Class ExternalFlowAnalyzer Implementation
\*---------------------------------------------------------------------------*/

ExternalFlowAnalyzer::ExternalFlowAnalyzer()
:
    vehicleType_("generic"),
    speed_(25.0),  // Default highway speed (m/s = ~90 km/h)
    fluidType_("air"),
    altitude_(0.0),
    temperature_(293.15),  // 20°C
    objective_("drag_analysis"),
    timeConstraint_(7200),  // 2 hours default
    accuracyTarget_(0.05)   // 5% engineering accuracy
{}

bool ExternalFlowAnalyzer::parseUserIntent(const std::string& description)
{
    std::cout << "🤖 MCP: Analyzing your external flow requirements..." << std::endl;
    
    // Parse natural language description
    if (description.find("car") != std::string::npos || 
        description.find("vehicle") != std::string::npos ||
        description.find("automobile") != std::string::npos) 
    {
        vehicleType_ = "car";
        setDefaultCarParameters();
    }
    else if (description.find("aircraft") != std::string::npos ||
             description.find("airplane") != std::string::npos ||
             description.find("wing") != std::string::npos)
    {
        vehicleType_ = "aircraft";
        setDefaultAircraftParameters();
    }
    else if (description.find("building") != std::string::npos ||
             description.find("structure") != std::string::npos)
    {
        vehicleType_ = "building";
        setDefaultBuildingParameters();
    }
    
    // Detect speed context
    if (description.find("highway") != std::string::npos ||
        description.find("high speed") != std::string::npos)
    {
        speed_ = 27.8;  // ~100 km/h
    }
    else if (description.find("city") != std::string::npos ||
             description.find("low speed") != std::string::npos)
    {
        speed_ = 13.9;  // ~50 km/h
    }
    
    // Detect objectives
    if (description.find("drag") != std::string::npos)
        objective_ = "drag_analysis";
    else if (description.find("lift") != std::string::npos)
        objective_ = "lift_analysis";
    else if (description.find("pressure") != std::string::npos)
        objective_ = "pressure_distribution";
    else if (description.find("wake") != std::string::npos)
        objective_ = "wake_analysis";
    
    return true;
}

PhysicsParameters ExternalFlowAnalyzer::analyzePhysics() const
{
    PhysicsParameters params;
    
    // Calculate key dimensionless numbers
    double kinematicViscosity = 1.5e-5;  // Air at standard conditions
    double characteristicLength = getCharacteristicLength();
    
    params.reynoldsNumber = speed_ * characteristicLength / kinematicViscosity;
    params.machNumber = speed_ / 343.0;  // Speed of sound in air
    
    // Determine flow regime
    if (params.reynoldsNumber < 2300) {
        params.flowRegime = "laminar";
    } else if (params.reynoldsNumber < 500000) {
        params.flowRegime = "transitional";
    } else {
        params.flowRegime = "turbulent";
    }
    
    // Compressibility assessment
    if (params.machNumber < 0.3) {
        params.compressibility = "incompressible";
    } else if (params.machNumber < 0.8) {
        params.compressibility = "subsonic_compressible";
    } else {
        params.compressibility = "transonic";
    }
    
    std::cout << "📊 Physics Analysis:" << std::endl;
    std::cout << "   Reynolds Number: " << params.reynoldsNumber << std::endl;
    std::cout << "   Mach Number: " << params.machNumber << std::endl;
    std::cout << "   Flow Regime: " << params.flowRegime << std::endl;
    std::cout << "   Compressibility: " << params.compressibility << std::endl;
    
    return params;
}

OpenFOAMSetup ExternalFlowAnalyzer::recommendOpenFOAMSetup() const
{
    OpenFOAMSetup setup;
    PhysicsParameters physics = analyzePhysics();
    
    // Solver selection based on physics
    if (physics.compressibility == "incompressible") {
        if (objective_ == "quick_estimate") {
            setup.solver = "potentialFoam";  // Very fast, inviscid
        } else {
            setup.solver = "simpleFoam";     // Standard steady RANS
        }
    } else {
        setup.solver = "rhoPimpleFoam";      // Compressible unsteady
    }
    
    // Turbulence model selection
    if (physics.flowRegime == "laminar") {
        setup.turbulenceModel = "laminar";
    } else if (vehicleType_ == "car") {
        setup.turbulenceModel = "kOmegaSST";  // Good for external aero
    } else if (vehicleType_ == "aircraft") {
        setup.turbulenceModel = "SpalartAllmaras";  // Aviation standard
    } else {
        setup.turbulenceModel = "kEpsilon";   // Robust default
    }
    
    // Mesh requirements
    setup.meshParams = calculateMeshRequirements(physics);
    
    // Boundary conditions
    setup.boundaryConditions = setupBoundaryConditions();
    
    // Numerical schemes
    setup.schemes = selectNumericalSchemes();
    
    std::cout << "🔧 OpenFOAM Setup Recommendations:" << std::endl;
    std::cout << "   Solver: " << setup.solver << std::endl;
    std::cout << "   Turbulence: " << setup.turbulenceModel << std::endl;
    std::cout << "   Target Cells: " << setup.meshParams.targetCells << std::endl;
    
    return setup;
}

std::vector<std::string> ExternalFlowAnalyzer::validateSetup() const
{
    std::vector<std::string> warnings;
    PhysicsParameters physics = analyzePhysics();
    
    // Physics validation
    if (physics.machNumber > 0.3 && physics.compressibility == "incompressible") {
        warnings.push_back("⚠️  Mach number > 0.3: Consider compressible solver");
    }
    
    if (physics.reynoldsNumber > 1e8) {
        warnings.push_back("⚠️  Very high Reynolds number: Mesh requirements will be extreme");
    }
    
    if (vehicleType_ == "aircraft" && altitude_ == 0.0) {
        warnings.push_back("💡 Consider setting cruise altitude for realistic aircraft analysis");
    }
    
    // Computational requirements
    OpenFOAMSetup setup = recommendOpenFOAMSetup();
    if (setup.meshParams.targetCells > 5000000 && timeConstraint_ < 3600) {
        warnings.push_back("⚠️  Large mesh + short time constraint: Consider coarser mesh");
    }
    
    return warnings;
}

std::string ExternalFlowAnalyzer::generateUserSummary() const
{
    std::string summary = "🚗 External Flow Analysis Summary\n";
    summary += "==========================================\n";
    summary += "Vehicle Type: " + vehicleType_ + "\n";
    summary += "Speed: " + std::to_string(speed_) + " m/s (" + 
               std::to_string(speed_ * 3.6) + " km/h)\n";
    summary += "Primary Objective: " + objective_ + "\n";
    summary += "Time Constraint: " + std::to_string(timeConstraint_/60) + " minutes\n";
    summary += "Target Accuracy: ±" + std::to_string(accuracyTarget_*100) + "%\n\n";
    
    // Add physics insights
    PhysicsParameters physics = analyzePhysics();
    summary += "🔬 Physics Insights:\n";
    summary += "- Flow is " + physics.flowRegime + " (Re = " + 
               std::to_string(int(physics.reynoldsNumber)) + ")\n";
    summary += "- " + physics.compressibility + " assumption valid (Ma = " + 
               std::to_string(physics.machNumber) + ")\n\n";
    
    // Add recommendations
    OpenFOAMSetup setup = recommendOpenFOAMSetup();
    summary += "🎯 Recommended Approach:\n";
    summary += "- Solver: " + setup.solver + "\n";
    summary += "- Turbulence: " + setup.turbulenceModel + "\n";
    summary += "- Estimated Runtime: " + 
               std::to_string(setup.meshParams.targetCells / 500000) + " hours\n";
    
    return summary;
}

// Private helper methods
void ExternalFlowAnalyzer::setDefaultCarParameters()
{
    characteristicLength_ = 4.5;  // Typical car length
    frontalArea_ = 2.5;          // Typical frontal area
    referenceHeight_ = 1.4;      // Typical car height
}

void ExternalFlowAnalyzer::setDefaultAircraftParameters()
{
    characteristicLength_ = 1.0;  // Wing chord
    frontalArea_ = 20.0;         // Wing area
    referenceHeight_ = 0.15;     // Wing thickness
    altitude_ = 10000.0;         // Cruise altitude
}

void ExternalFlowAnalyzer::setDefaultBuildingParameters()
{
    characteristicLength_ = 10.0; // Building width
    frontalArea_ = 100.0;        // Building face area
    referenceHeight_ = 30.0;     // Building height
}

double ExternalFlowAnalyzer::getCharacteristicLength() const
{
    return characteristicLength_;
}

MeshParameters ExternalFlowAnalyzer::calculateMeshRequirements(const PhysicsParameters& physics) const
{
    MeshParameters mesh;
    
    // Base mesh size on accuracy requirements and time constraints
    if (accuracyTarget_ < 0.02) {      // High accuracy
        mesh.targetCells = 5000000;
        mesh.targetYPlus = 1.0;        // Wall-resolved
    } else if (accuracyTarget_ < 0.05) { // Engineering accuracy  
        mesh.targetCells = 2000000;
        mesh.targetYPlus = 50.0;       // Wall function
    } else {                           // Quick estimate
        mesh.targetCells = 500000;
        mesh.targetYPlus = 100.0;      // Coarse wall function
    }
    
    // Adjust for time constraints
    if (timeConstraint_ < 1800) {      // 30 minutes
        mesh.targetCells = std::min(mesh.targetCells, 200000);
    } else if (timeConstraint_ < 3600) { // 1 hour
        mesh.targetCells = std::min(mesh.targetCells, 1000000);
    }
    
    mesh.refinementLevels = (objective_ == "wake_analysis") ? 5 : 3;
    mesh.boundaryLayerLayers = (mesh.targetYPlus < 5.0) ? 15 : 5;
    
    return mesh;
}

std::map<std::string, std::string> ExternalFlowAnalyzer::setupBoundaryConditions() const
{
    std::map<std::string, std::string> bc;
    
    bc["inlet"] = "fixedValue uniform (" + std::to_string(speed_) + " 0 0)";
    bc["outlet"] = "zeroGradient";
    bc["ground"] = "noSlip";
    bc["vehicle"] = "noSlip";
    bc["sides"] = "slip";
    bc["top"] = "slip";
    
    return bc;
}

std::map<std::string, std::string> ExternalFlowAnalyzer::selectNumericalSchemes() const
{
    std::map<std::string, std::string> schemes;
    
    if (accuracyTarget_ < 0.02) {
        // High accuracy schemes
        schemes["convection"] = "Gauss linearUpwind grad(U)";
        schemes["gradient"] = "Gauss linear";
        schemes["laplacian"] = "Gauss linear corrected";
    } else {
        // Robust schemes for engineering accuracy
        schemes["convection"] = "bounded Gauss upwind";
        schemes["gradient"] = "Gauss linear";
        schemes["laplacian"] = "Gauss linear limited 0.5";
    }
    
    return schemes;
}

// ************************************************************************* //