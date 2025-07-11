/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | External Flow Analysis
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    External flow analysis tool for MCP server

    Handles automotive, aerospace, and civil engineering external flow
    analysis with proper aerodynamics calculations and OpenFOAM setup.

\*---------------------------------------------------------------------------*/

#ifndef external_flow_H
#define external_flow_H

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "case_manager.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Struct ExternalFlowInput
\*---------------------------------------------------------------------------*/

struct ExternalFlowInput {
    std::string vehicleType;      // "car", "aircraft", "building", "cylinder"
    double velocity;              // m/s
    double characteristicLength;  // m (length for car, chord for aircraft, height for building)
    double frontalArea;           // m² (for drag calculation reference)
    double altitude;              // m (affects air density and viscosity)
    std::string fluidType;        // "air", "water"
    std::string objective;        // "drag_analysis", "lift_analysis", "pressure_distribution"

    // Advanced parameters
    double density;      // kg/m³
    double viscosity;    // m²/s
    double temperature;  // K

    ExternalFlowInput()
        : vehicleType("car"),
          velocity(30.0)  // 30 m/s ~ 108 km/h highway speed
          ,
          characteristicLength(4.0)  // 4m typical car length
          ,
          frontalArea(2.5)  // 2.5 m² typical car frontal area
          ,
          altitude(0.0)  // sea level
          ,
          fluidType("air"),
          objective("drag_analysis"),
          density(1.225)  // air at sea level
          ,
          viscosity(1.5e-5)  // air kinematic viscosity
          ,
          temperature(288.15)  // 15°C
    {}

    double getReynoldsNumber() const { return velocity * characteristicLength / viscosity; }

    double getMachNumber() const {
        double speedOfSound = std::sqrt(1.4 * 287.0 * temperature);  // air at temperature
        return velocity / speedOfSound;
    }

    bool isLaminar() const {
        return getReynoldsNumber() < 500000;  // external flow transition much higher
    }

    bool isTurbulent() const { return getReynoldsNumber() > 1000000; }

    bool isCompressible() const { return getMachNumber() > 0.3; }

    bool isHighSpeed() const { return getMachNumber() > 0.8; }
};

/*---------------------------------------------------------------------------*\
                        Struct ExternalFlowResults
\*---------------------------------------------------------------------------*/

struct ExternalFlowResults {
    double reynoldsNumber;
    double machNumber;
    double dragCoefficient;
    double liftCoefficient;
    double dragForce;            // N
    double liftForce;            // N
    double pressureCoefficient;  // Cp at key points
    double skinFrictionCoefficient;
    std::string flowRegime;
    std::string compressibilityRegime;
    std::string caseId;
    bool success;
    std::string errorMessage;

    ExternalFlowResults()
        : reynoldsNumber(0),
          machNumber(0),
          dragCoefficient(0),
          liftCoefficient(0),
          dragForce(0),
          liftForce(0),
          pressureCoefficient(0),
          skinFrictionCoefficient(0),
          flowRegime("unknown"),
          compressibilityRegime("unknown"),
          success(false) {}
};

/*---------------------------------------------------------------------------*\
                        Class ExternalFlowAnalyzer Declaration
\*---------------------------------------------------------------------------*/

class ExternalFlowAnalyzer {
   private:
    std::unique_ptr<CaseManager> caseManager_;

    void validateInput(const ExternalFlowInput& input);
    CaseParameters createCaseParameters(const ExternalFlowInput& input);
    ExternalFlowResults processResults(const std::string& caseId, const ExternalFlowInput& input);

    // Theoretical calculations for validation/fallback
    double calculateTheoreticalDragCoefficient(const ExternalFlowInput& input) const;
    double calculateSkinFrictionCoefficient(const ExternalFlowInput& input) const;
    double calculatePressureDragCoefficient(const ExternalFlowInput& input) const;

    std::string determineFlowRegime(const ExternalFlowInput& input) const;
    std::string determineCompressibilityRegime(const ExternalFlowInput& input) const;
    std::string generateAnalysisReport(const ExternalFlowInput& input,
                                       const ExternalFlowResults& results) const;

    // Vehicle-specific calculations
    double getCarDragCoefficient(double Re, double aspect_ratio) const;
    double getAircraftDragCoefficient(double Re, double Ma, const std::string& airfoil_type) const;
    double getBuildingDragCoefficient(double Re, double height_to_width) const;
    double getCylinderDragCoefficient(double Re) const;

   public:
    ExternalFlowAnalyzer();
    explicit ExternalFlowAnalyzer(std::unique_ptr<CaseManager> caseManager);
    ~ExternalFlowAnalyzer() = default;

    ExternalFlowResults analyze(const ExternalFlowInput& input);

    json getInputSchema() const;

    static ExternalFlowInput parseInput(const json& inputJson);
    static json resultsToJson(const ExternalFlowResults& results);

    void setWorkingDirectory(const std::string& workingDir);

    std::vector<std::string> listActiveCases() const;
    bool deleteCaseData(const std::string& caseId);
};

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const ExternalFlowInput& input);
void from_json(const json& j, ExternalFlowInput& input);

void to_json(json& j, const ExternalFlowResults& results);
void from_json(const json& j, ExternalFlowResults& results);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //