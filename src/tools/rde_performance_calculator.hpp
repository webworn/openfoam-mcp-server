#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                    Class RDEPerformanceCalculator
\*---------------------------------------------------------------------------*/

/**
 * RDE Performance Metrics Calculator - Phase 2 Enhancement
 * 
 * Extends wave interaction analysis with quantitative performance metrics:
 * - Thrust integration from pressure fields
 * - Specific impulse calculation with mass flow analysis  
 * - Combustion efficiency from species conversion tracking
 * - Educational performance-physics correlations
 */
class RDEPerformanceCalculator {
public:
    RDEPerformanceCalculator();
    ~RDEPerformanceCalculator() = default;
    
    // Performance analysis structures
    struct ThrustAnalysis {
        double instantaneousThrust;      // [N] Current thrust
        double timeAveragedThrust;       // [N] Mean thrust over analysis period
        double thrustVariation;          // [%] RMS variation
        double maxThrust;                // [N] Peak thrust (during collisions)
        double minThrust;                // [N] Minimum thrust
        std::vector<double> thrustHistory; // [N] Thrust vs time
        
        // Wave-specific contributions
        std::map<int, double> waveContributions; // Thrust per wave ID
        double collisionThrustEnhancement;      // [%] Enhancement from collisions
        double multiWaveAdvantage;              // [%] vs single wave operation
    };
    
    struct SpecificImpulseAnalysis {
        double specificImpulse;          // [s] Thrust / (mass_flow * g)
        double massFlowRate;             // [kg/s] Total mass flow
        double exitVelocity;             // [m/s] Average exit velocity
        double characteristicVelocity;   // [m/s] c* = pc*A_throat/mass_flow
        
        // Efficiency metrics
        double propulsiveEfficiency;     // [%] Kinetic energy efficiency
        double thermalEfficiency;        // [%] Heat to kinetic energy conversion
        double overallEfficiency;        // [%] Combined efficiency
        
        // Wave impact analysis
        double singleWaveIsp;            // [s] Theoretical single-wave Isp
        double multiWaveIsp;             // [s] Actual multi-wave Isp
        double waveInteractionBonus;     // [%] Performance gain from interactions
    };
    
    struct CombustionEfficiencyAnalysis {
        double overallEfficiency;        // [%] H2 → H2O conversion
        double reactionCompleteness;     // [%] Theoretical vs actual conversion
        double mixingEfficiency;         // [%] Fuel-air mixing quality
        double residenceTime;           // [s] Average residence in chamber
        
        // Species tracking
        std::map<std::string, double> speciesConversionRates; // [kg/s] per species
        std::map<std::string, double> speciesEfficiencies;    // [%] conversion efficiency
        
        // Wave-specific analysis
        std::map<int, double> waveEfficiencies;              // [%] per wave
        double collisionZoneEfficiency;                      // [%] in collision regions
        double freeStreamEfficiency;                         // [%] in free stream regions
        
        // Spatial distribution
        std::vector<double> radialEfficiencyProfile;        // [%] vs radius
        std::vector<double> circumferentialEfficiencyProfile; // [%] vs angle
    };
    
    struct GeometryParameters {
        double outerRadius;      // [m] 0.080
        double innerRadius;      // [m] 0.050  
        double chamberLength;    // [m] 0.150
        double annularArea;      // [m²] π(R²-r²)
        double exitArea;         // [m²] Exit area
        double inletArea;        // [m²] Injection area
        double volume;           // [m³] Chamber volume
    };
    
    struct OperatingConditions {
        double ambientPressure;  // [Pa] 101325
        double ambientTemperature; // [K] 300
        double chamberPressure;  // [Pa] From simulation
        double injectionPressure; // [Pa] Estimated
        double equivalenceRatio; // [-] From phi field
        double injectionTemperature; // [K] From simulation
    };
    
    struct PerformanceMetrics {
        ThrustAnalysis thrust;
        SpecificImpulseAnalysis specificImpulse;
        CombustionEfficiencyAnalysis combustion;
        
        // Comparative analysis
        double theoreticalMaxThrust;     // [N] Chapman-Jouguet theoretical
        double actualThrustRatio;        // [%] Actual/theoretical
        double pressureGainRatio;        // [-] vs constant pressure combustion
        double detonationAdvantage;      // [%] vs deflagration
        
        // Educational metrics
        std::string performanceRating;   // "Excellent", "Good", "Needs Optimization"
        std::vector<std::string> optimizationRecommendations;
        std::string comparisonWithConventional; // vs gas turbines, rockets
    };
    
    // Core performance calculation methods
    PerformanceMetrics calculatePerformanceMetrics(const std::string& caseDirectory,
                                                   const std::vector<std::pair<double, double>>& waveData);
    
    ThrustAnalysis calculateThrustFromPressureFields(const std::string& caseDirectory);
    
    SpecificImpulseAnalysis calculateSpecificImpulse(const std::string& caseDirectory,
                                                    const ThrustAnalysis& thrust);
    
    CombustionEfficiencyAnalysis analyzeCombustionEfficiency(const std::string& caseDirectory);
    
    // Field data processing methods
    std::vector<std::vector<double>> readScalarField(const std::string& fieldPath);
    std::vector<std::vector<std::pair<double, double>>> readVectorField(const std::string& fieldPath);
    std::vector<std::pair<double, double>> readMeshCoordinates(const std::string& meshPath);
    
    // Thrust calculation methods
    double integrateThrustOverBoundary(const std::vector<std::vector<double>>& pressureField,
                                      const std::vector<std::pair<double, double>>& coordinates,
                                      const std::string& boundaryName = "outlet");
    
    double calculateMomentumThrust(const std::vector<std::vector<double>>& densityField,
                                 const std::vector<std::vector<std::pair<double, double>>>& velocityField,
                                 const std::vector<std::pair<double, double>>& coordinates);
    
    // Mass flow analysis methods
    double calculateMassFlowRate(const std::vector<std::vector<double>>& densityField,
                               const std::vector<std::vector<std::pair<double, double>>>& velocityField,
                               const std::vector<std::pair<double, double>>& coordinates,
                               const std::string& boundaryName = "outlet");
    
    double calculateExitVelocity(const std::vector<std::vector<std::pair<double, double>>>& velocityField,
                               const std::vector<std::pair<double, double>>& coordinates);
    
    // Species analysis methods
    std::map<std::string, double> calculateSpeciesConsumption(const std::string& caseDirectory,
                                                            const std::vector<std::string>& species = {"H2", "O2", "H2O"});
    
    double calculateH2ConversionEfficiency(const std::string& caseDirectory);
    
    std::map<std::string, double> trackSpeciesEvolution(const std::string& caseDirectory);
    
    // Educational content generation
    std::string explainThrustGeneration(const ThrustAnalysis& analysis);
    std::string explainSpecificImpulse(const SpecificImpulseAnalysis& analysis);
    std::string explainCombustionEfficiency(const CombustionEfficiencyAnalysis& analysis);
    std::string generateOptimizationGuidance(const PerformanceMetrics& metrics);
    std::string compareWithConventionalSystems(const PerformanceMetrics& metrics);
    
    // Validation methods
    bool validateAgainstTheoreticalLimits(const PerformanceMetrics& metrics);
    double calculateChapmanJouguetThrust(const OperatingConditions& conditions);
    double calculateTheoreticalSpecificImpulse(const std::string& fuelType = "H2");
    
private:
    // Geometry and operating conditions
    GeometryParameters geometry_;
    OperatingConditions conditions_;
    
    // Physical constants
    static constexpr double g_ = 9.81;           // [m/s²] Gravitational acceleration
    static constexpr double R_universal_ = 8314.0; // [J/kmol·K] Universal gas constant
    static constexpr double MW_air_ = 28.97;     // [kg/kmol] Molecular weight of air
    static constexpr double MW_H2_ = 2.016;      // [kg/kmol] Molecular weight of H2
    static constexpr double MW_H2O_ = 18.015;    // [kg/kmol] Molecular weight of H2O
    
    // Performance calculation helpers
    double interpolateField(const std::vector<std::vector<double>>& field,
                          const std::vector<std::pair<double, double>>& coordinates,
                          double r, double theta) const;
    
    std::vector<std::pair<double, double>> getBoundaryNodes(const std::string& meshPath,
                                                          const std::string& boundaryName) const;
    
    double calculateBoundaryIntegral(const std::vector<std::vector<double>>& field,
                                   const std::vector<std::pair<double, double>>& boundaryNodes) const;
    
    // Thermodynamic property calculations
    double calculateDensity(double pressure, double temperature, double molecularWeight) const;
    double calculateSoundSpeed(double temperature, double gamma = 1.4, double R = 287.0) const;
    double calculateMachNumber(double velocity, double soundSpeed) const;
    
    // Educational content templates
    std::map<std::string, std::string> educationalTemplates_;
    void initializeEducationalContent();
    
    // Validation data
    struct ValidationTargets {
        double expectedThrust = 11519.0;        // [N] From existing analysis
        double expectedIsp = 1629.0;           // [s] From existing analysis  
        double expectedCombustionEff = 85.0;   // [%] From existing analysis
        double expectedFrequency = 5387.0;     // [Hz] From existing analysis
    };
    
    ValidationTargets validationTargets_;
};

/*---------------------------------------------------------------------------*\
                    Helper Functions for Performance Analysis
\*---------------------------------------------------------------------------*/

// Thermodynamic utilities
double calculateChapmanJouguetVelocity(double temperature, double pressure, 
                                      double gamma = 1.2, double R = 520.0);
double calculateDetonationPressureRatio(double initialPressure, double gamma = 1.2);
double calculateCombustionTemperature(const std::string& fuel, double equivalenceRatio);

// Performance comparison utilities  
double compareWithGasTurbine(double rdeThrust, double rdeIsp);
double compareWithRocket(double rdeThrust, double rdeIsp);
double calculateThrustToWeight(double thrust, double engineMass = 100.0); // [kg] estimated

// Educational content utilities
std::string formatPerformanceNumber(double value, const std::string& units, int precision = 1);
std::string generatePerformanceRating(double actualValue, double theoreticalMax, double goodThreshold = 0.8);
std::vector<std::string> generateOptimizationRecommendations(const RDEPerformanceCalculator::PerformanceMetrics& metrics);

} // namespace MCP  
} // namespace Foam