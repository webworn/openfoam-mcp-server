/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Multiphase Flow Analysis
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Multiphase flow analysis using OpenFOAM interFoam solver

    Supports two-phase immiscible fluid simulations including:
    - Gas-liquid flows (air-water)
    - Liquid-liquid flows (oil-water)
    - Free surface flows
    - Breaking waves and sloshing
    - Dam break problems
    - Bubble dynamics

\*---------------------------------------------------------------------------*/

#ifndef multiphase_flow_H
#define multiphase_flow_H

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "case_manager.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Struct MultiphaseFlowInput
\*---------------------------------------------------------------------------*/

struct MultiphaseFlowInput {
    // Flow configuration
    std::string analysisType;  // "dam_break", "sloshing", "bubble", "wave", "mixing"
    std::string phase1;        // Primary phase (e.g., "water")
    std::string phase2;        // Secondary phase (e.g., "air")

    // Geometry parameters
    double characteristicLength;  // Domain characteristic length [m]
    double height;                // Initial fluid height [m]
    double width;                 // Domain width [m]
    double depth;                 // Domain depth [m]

    // Fluid properties
    double density1;        // Phase 1 density [kg/m³]
    double density2;        // Phase 2 density [kg/m³]
    double viscosity1;      // Phase 1 dynamic viscosity [Pa·s]
    double viscosity2;      // Phase 2 dynamic viscosity [Pa·s]
    double surfaceTension;  // Surface tension coefficient [N/m]

    // Flow conditions
    double gravity;               // Gravitational acceleration [m/s²]
    std::string initialVelocity;  // Initial velocity field
    double contactAngle;          // Wall contact angle [degrees]

    // Simulation parameters
    bool steadyState;
    double timeStep;       // Time step [s]
    double endTime;        // End time [s]
    double writeInterval;  // Write interval [s]

    // Advanced parameters
    double courantNumber;         // Target Courant number
    std::string turbulenceModel;  // "laminar", "kEpsilon", "kOmegaSST"
    bool compressible;            // Compressible flow

    MultiphaseFlowInput()
        : analysisType("dam_break"),
          phase1("water"),
          phase2("air"),
          characteristicLength(1.0),
          height(0.5),
          width(1.0),
          depth(0.1),
          density1(1000.0),       // water
          density2(1.225),        // air
          viscosity1(1e-3),       // water
          viscosity2(1.8e-5),     // air
          surfaceTension(0.072),  // air-water
          gravity(9.81),
          initialVelocity("zero"),
          contactAngle(90.0),
          steadyState(false),
          timeStep(0.001),
          endTime(1.0),
          writeInterval(0.05),
          courantNumber(0.3),
          turbulenceModel("laminar"),
          compressible(false) {}

    // Physics analysis methods
    double getReynoldsNumber() const;
    double getWebberNumber() const;
    double getCapillaryNumber() const;
    double getBondNumber() const;
    double getFroudeNumber() const;

    bool isDamBreak() const { return analysisType == "dam_break"; }
    bool isSloshing() const { return analysisType == "sloshing"; }
    bool isBubble() const { return analysisType == "bubble"; }
    bool isWave() const { return analysisType == "wave"; }
    bool isMixing() const { return analysisType == "mixing"; }
};

/*---------------------------------------------------------------------------*\
                        Struct MultiphaseFlowResults
\*---------------------------------------------------------------------------*/

struct MultiphaseFlowResults {
    // Dimensionless numbers
    double reynoldsNumber;
    double webberNumber;
    double capillaryNumber;
    double bondNumber;
    double froudeNumber;

    // Flow characteristics
    std::string flowRegime;             // "surface-tension-dominated", "gravity-dominated", etc.
    std::string mixingCharacteristics;  // "well-mixed", "stratified", "dispersed"

    // Phase distribution
    double phase1Volume;   // Phase 1 volume fraction
    double phase2Volume;   // Phase 2 volume fraction
    double interfaceArea;  // Total interface area [m²]
    double maxVelocity;    // Maximum velocity magnitude [m/s]

    // Energy analysis
    double kineticEnergy;    // Total kinetic energy [J]
    double potentialEnergy;  // Total potential energy [J]
    double surfaceEnergy;    // Surface energy [J]

    // Pressure analysis
    double maxPressure;   // Maximum pressure [Pa]
    double minPressure;   // Minimum pressure [Pa]
    double pressureDrop;  // Pressure drop across domain [Pa]

    // Time evolution
    std::vector<double> timeHistory;           // Time points [s]
    std::vector<double> phase1VolumeHistory;   // Phase 1 volume over time
    std::vector<double> kineticEnergyHistory;  // Kinetic energy over time
    std::vector<double> maxVelocityHistory;    // Max velocity over time

    // OpenFOAM case information
    std::string caseId;
    bool success;
    std::string errorMessage;

    MultiphaseFlowResults()
        : reynoldsNumber(0.0),
          webberNumber(0.0),
          capillaryNumber(0.0),
          bondNumber(0.0),
          froudeNumber(0.0),
          flowRegime("unknown"),
          mixingCharacteristics("unknown"),
          phase1Volume(0.0),
          phase2Volume(0.0),
          interfaceArea(0.0),
          maxVelocity(0.0),
          kineticEnergy(0.0),
          potentialEnergy(0.0),
          surfaceEnergy(0.0),
          maxPressure(0.0),
          minPressure(0.0),
          pressureDrop(0.0),
          success(false) {}
};

/*---------------------------------------------------------------------------*\
                     Class MultiphaseFlowAnalyzer Declaration
\*---------------------------------------------------------------------------*/

class MultiphaseFlowAnalyzer {
   private:
    std::unique_ptr<CaseManager> caseManager_;

    // Analysis methods
    MultiphaseFlowResults calculateTheoreticalResults(const MultiphaseFlowInput& input) const;
    void calculateDimensionlessNumbers(const MultiphaseFlowInput& input,
                                       MultiphaseFlowResults& results) const;
    std::string determineFlowRegime(const MultiphaseFlowInput& input,
                                    const MultiphaseFlowResults& results) const;
    std::string analyzeMixingCharacteristics(const MultiphaseFlowInput& input,
                                             const MultiphaseFlowResults& results) const;

    // OpenFOAM integration
    std::string setupInterFoamCase(const MultiphaseFlowInput& input);
    void parseInterFoamResults(const std::string& caseId, MultiphaseFlowResults& results) const;

   public:
    MultiphaseFlowAnalyzer();
    explicit MultiphaseFlowAnalyzer(std::unique_ptr<CaseManager> caseManager);

    MultiphaseFlowResults analyze(const MultiphaseFlowInput& input);

    bool validateInput(const MultiphaseFlowInput& input) const;

    json toJson(const MultiphaseFlowResults& results) const;
    MultiphaseFlowResults fromJson(const json& j) const;
};

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const MultiphaseFlowInput& input);
void from_json(const json& j, MultiphaseFlowInput& input);

void to_json(json& j, const MultiphaseFlowResults& results);
void from_json(const json& j, MultiphaseFlowResults& results);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //