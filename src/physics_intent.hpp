/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Physics-Aware Intent Understanding
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM MCP Server.

Description
    Physics Intent Understanding - Translates user requirements into OpenFOAM
    case structure. This is the core intelligence that understands what users
    want to simulate and how to set up OpenFOAM accordingly.

\*---------------------------------------------------------------------------*/

#ifndef physics_intent_H
#define physics_intent_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Foam {
namespace MCP {

// Forward declarations
class OpenFOAMCase;

/*---------------------------------------------------------------------------*\
                       Class PhysicsIntent Declaration
\*---------------------------------------------------------------------------*/

class PhysicsIntent {
   public:
    // Physics categories that users commonly want to simulate
    enum class PhysicsType {
        INCOMPRESSIBLE_FLOW,  // "Flow around a car", "Pipe flow", "Mixing"
        COMPRESSIBLE_FLOW,    // "Supersonic jet", "Shock waves", "Nozzle flow"
        HEAT_TRANSFER,        // "Cooling analysis", "Heat exchanger"
        TURBULENT_FLOW,       // "Turbulent mixing", "Boundary layer"
        MULTIPHASE_FLOW,      // "Free surface", "Droplet dynamics"
        COMBUSTION,           // "Flame dynamics", "Engine combustion"
        AERODYNAMICS,         // "Airfoil analysis", "Wing performance"
        FLUID_STRUCTURE       // "Flexible structures", "Vibration"
    };

    // Engineering objectives that drive simulation setup
    enum class Objective {
        DESIGN_OPTIMIZATION,   // Need accurate forces, efficiency metrics
        FLOW_VISUALIZATION,    // Need good visual quality, flow patterns
        ENGINEERING_ANALYSIS,  // Need engineering accuracy, reliable results
        RESEARCH_STUDY,        // Need high accuracy, detailed physics
        QUICK_ESTIMATE,        // Need fast results, approximate answers
        VALIDATION_STUDY       // Need benchmark accuracy, experimental comparison
    };

    // Accuracy vs Speed preferences
    enum class Preference {
        MAXIMUM_ACCURACY,  // Don't care about time, need best results
        BALANCED,          // Good balance of accuracy and speed
        FAST_TURNAROUND,   // Need results quickly, accuracy is secondary
        PRODUCTION_READY   // Robust, reliable, suitable for daily use
    };

   private:
    // User intent data
    PhysicsType physicsType_;
    Objective objective_;
    Preference preference_;

    // Physical parameters (from user description)
    std::map<std::string, double> physicalParams_;
    std::map<std::string, std::string> textParams_;

    // Derived OpenFOAM parameters
    std::string recommendedSolver_;
    std::map<std::string, std::string> boundaryConditions_;
    std::map<std::string, std::string> numericalSchemes_;
    double targetYPlus_;
    int recommendedCells_;
    double maxCourantNumber_;

   public:
    // Constructors
    PhysicsIntent() = default;

    // User intent parsing
    bool parseUserIntent(const std::string& description);
    bool setPhysicsFromDescription(const std::string& description);
    bool setObjectiveFromRequirements(const std::string& requirements);

    // Physics parameter setters (from user input)
    void setReynoldsNumber(double Re) {
        physicalParams_["Re"] = Re;
    }
    void setMachNumber(double Ma) {
        physicalParams_["Ma"] = Ma;
    }
    void setVelocity(double U) {
        physicalParams_["U"] = U;
    }
    void setPressure(double p) {
        physicalParams_["p"] = p;
    }
    void setTemperature(double T) {
        physicalParams_["T"] = T;
    }
    void setCharacteristicLength(double L) {
        physicalParams_["L"] = L;
    }

    // Geometry and boundary information
    void setGeometryType(const std::string& type) {
        textParams_["geometry"] = type;
    }
    void setInletCondition(const std::string& condition) {
        textParams_["inlet"] = condition;
    }
    void setWallCondition(const std::string& condition) {
        textParams_["wall"] = condition;
    }

    // OpenFOAM case generation
    std::unique_ptr<OpenFOAMCase> generateCase();

    // Intelligent recommendations
    std::string recommendSolver() const;
    std::map<std::string, std::string> recommendSchemes() const;
    std::map<std::string, std::string> recommendBoundaryConditions() const;
    double recommendTimeStep() const;
    int recommendMeshSize() const;

    // Physics validation
    bool validatePhysics() const;
    std::vector<std::string> getPhysicsWarnings() const;
    std::vector<std::string> getRecommendations() const;

    // User-friendly explanations
    std::string explainSolverChoice() const;
    std::string explainMeshRequirements() const;
    std::string explainBoundaryConditions() const;
    std::string estimateRuntime() const;

    // Natural language interface
    static PhysicsIntent fromNaturalLanguage(const std::string& description);
    std::string toUserSummary() const;

    // Examples of user descriptions this should understand:
    // "I want to simulate airflow around a car at highway speeds"
    // "Show me the mixing pattern in a stirred tank reactor"
    // "Analyze heat transfer in a heat exchanger"
    // "Design optimization of a wind turbine blade"
    // "Quick estimate of pressure drop in a pipe"
};

/*---------------------------------------------------------------------------*\
                     Class UserRequirement Declaration
\*---------------------------------------------------------------------------*/

class UserRequirement {
   public:
    struct FlowCondition {
        std::string description;    // "High speed flow", "Natural convection"
        double velocity = -1;       // m/s, -1 means not specified
        double pressure = -1;       // Pa
        double temperature = -1;    // K
        std::string fluid = "air";  // Default fluid
    };

    struct GeometryInfo {
        std::string type;                   // "cylinder", "airfoil", "pipe", "custom"
        std::string file;                   // STL file path if custom
        double characteristicLength = -1;   // Characteristic dimension
        std::vector<std::string> surfaces;  // Named surfaces for BCs
    };

    struct SimulationGoal {
        std::string primaryGoal;           // "drag_force", "heat_transfer", "mixing"
        std::vector<std::string> outputs;  // What user wants to see
        double targetAccuracy = 0.05;      // 5% default
        int maxRuntime = 3600;             // Max runtime in seconds
    };

    FlowCondition flow;
    GeometryInfo geometry;
    SimulationGoal goal;

    // Parse from user input
    static UserRequirement fromUserInput(const std::map<std::string, std::string>& input);

    // Convert to PhysicsIntent
    PhysicsIntent toPhysicsIntent() const;
};

// Helper functions for natural language processing
std::string detectPhysicsType(const std::string& description);
std::map<std::string, double> extractPhysicalValues(const std::string& description);
std::vector<std::string> suggestMissingParameters(const PhysicsIntent& intent);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //