/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Website:  https://openfoam.org
     \\/     M anipulation  |
\*---------------------------------------------------------------------------*/

#ifndef HEAT_TRANSFER_HPP
#define HEAT_TRANSFER_HPP

#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Foam
{
namespace MCP
{

using json = nlohmann::json;

struct HeatTransferInput
{
    // Geometry and regions
    std::string analysisType;           // "electronics_cooling", "heat_exchanger", "building", "engine"
    double characteristicLength;        // m (chip size, tube diameter, room size)
    std::string fluidRegions;          // Comma-separated list: "air,water"
    std::string solidRegions;          // Comma-separated list: "chip,heatsink,pcb"
    
    // Thermal properties
    double ambientTemperature;         // K
    double heatGeneration;             // W (total heat generation)
    double maxAllowableTemp;           // K (design constraint)
    
    // Flow conditions
    double inletVelocity;              // m/s
    double inletTemperature;           // K
    std::string coolantType;           // "air", "water", "oil"
    
    // Material properties (will be auto-populated based on materials)
    std::map<std::string, double> thermalConductivity;  // W/m·K
    std::map<std::string, double> density;              // kg/m³
    std::map<std::string, double> specificHeat;         // J/kg·K
    
    // Analysis parameters
    double targetAccuracy;             // Temperature accuracy in K
    int maxIterations;                 // Maximum solver iterations
    std::string couplingMode;          // "weak", "strong"
    
    // Time parameters
    bool steadyState;                  // true for steady, false for transient
    double timeStep;                   // s (for transient)
    double endTime;                    // s (for transient)
};

struct HeatTransferResults
{
    // Temperature analysis
    double maxTemperature;             // K
    double minTemperature;             // K
    double averageTemperature;         // K
    double maxSolidTemperature;        // K (hotspot)
    double thermalResistance;          // K/W
    
    // Heat transfer rates
    double totalHeatTransferRate;      // W
    double convectiveHeatTransfer;     // W
    double conductiveHeatTransfer;     // W
    double radiativeHeatTransfer;      // W
    
    // Performance metrics
    double thermalEfficiency;          // %
    double coolingEffectiveness;       // dimensionless
    double pressureDrop;               // Pa
    double pumpingPower;               // W
    
    // Dimensionless numbers
    double reynoldsNumber;             // Re
    double prandtlNumber;              // Pr
    double nusseltNumber;              // Nu
    double rayleighNumber;             // Ra (for natural convection)
    double pecletNumber;               // Pe
    
    // Design metrics
    double thermalPerformanceIndex;    // W/K (heat removal per temp rise)
    double coolingCapacity;            // W/m³
    bool thermalConstraintsSatisfied;  // Design safety check
    
    // Case information
    std::string caseId;
    bool success;
    std::string errorMessage;
};

class HeatTransferAnalyzer
{
public:
    HeatTransferAnalyzer();
    ~HeatTransferAnalyzer() = default;

    // Main analysis functions
    HeatTransferResults analyze(const HeatTransferInput& input);
    json toJson(const HeatTransferResults& results) const;
    
    // Physics calculations
    double calculateThermalResistance(const HeatTransferInput& input) const;
    double calculateNusseltNumber(double Re, double Pr, const std::string& geometry) const;
    double calculateHeatTransferCoefficient(double Nu, double k, double L) const;
    double calculatePressureDrop(const HeatTransferInput& input) const;
    
    // Material property database
    void populateMaterialProperties(HeatTransferInput& input) const;
    double getThermalConductivity(const std::string& material) const;
    double getDensity(const std::string& material) const;
    double getSpecificHeat(const std::string& material) const;
    
    // Validation functions
    bool validateInput(const HeatTransferInput& input) const;
    std::vector<std::string> getRecommendations(const HeatTransferInput& input, 
                                               const HeatTransferResults& results) const;

private:
    // Internal calculation methods
    double calculateReynoldsNumber(const HeatTransferInput& input) const;
    double calculatePrandtlNumber(const HeatTransferInput& input) const;
    double calculateRayleighNumber(const HeatTransferInput& input) const;
    double calculateThermalPerformanceIndex(const HeatTransferInput& input, 
                                          const HeatTransferResults& results) const;
    
    // Application-specific calculations
    HeatTransferResults analyzeElectronicsCooling(const HeatTransferInput& input);
    HeatTransferResults analyzeHeatExchanger(const HeatTransferInput& input);
    HeatTransferResults analyzeBuildingThermal(const HeatTransferInput& input);
    HeatTransferResults analyzeEngineCooling(const HeatTransferInput& input);
    
    // Correlation functions
    double getNusseltCorrelation(const std::string& correlationType, double Re, double Pr) const;
    double getFrictionFactor(double Re, const std::string& geometry) const;
    
    // Material database
    std::map<std::string, std::map<std::string, double>> materialDatabase_;
    void initializeMaterialDatabase();
};

} // namespace MCP
} // namespace Foam

#endif // HEAT_TRANSFER_HPP