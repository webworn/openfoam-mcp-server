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
#include "rde_3d_geometry.hpp"
#include "rde_3d_wave_analyzer.hpp"

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                    Class RDE3DPerformanceCalculator
\*---------------------------------------------------------------------------*/

/**
 * Advanced 3D RDE Performance Calculator with Axial Thrust Components
 * 
 * Enhanced from validated 2D baseline (11,519 N thrust, 1,629 s Isp, 85% efficiency)
 * to comprehensive 3D performance analysis:
 * - Axial thrust integration from 3D pressure fields
 * - Nozzle thrust contribution and expansion analysis
 * - 3D-specific losses and vectoring effects
 * - Multi-component thrust breakdown (combustor + nozzle)
 * - Real-time performance optimization with 3D considerations
 */
class RDE3DPerformanceCalculator {
public:
    RDE3DPerformanceCalculator();
    ~RDE3DPerformanceCalculator() = default;
    
    // 3D Performance structures
    struct ThrustAnalysis3D {
        // Total thrust breakdown
        double totalThrust;              // [N] Total 3D thrust
        double combustorThrust;          // [N] Thrust from combustor pressure
        double nozzleThrust;             // [N] Thrust from nozzle expansion
        double viscousLoss;              // [N] Viscous drag losses
        
        // Directional components
        double axialThrust;              // [N] Thrust in axial direction
        double radialThrustLoss;         // [N] Thrust lost to radial components
        double tangentialComponent;       // [N] Circumferential thrust component
        
        // Vector analysis
        std::array<double, 3> thrustVector;      // [N] 3D thrust vector
        double thrustVectorMagnitude;            // [N] Vector magnitude
        double thrustVectorAngle;                // [deg] Deviation from axial
        double axialEfficiency;                  // [-] Fraction of thrust in axial direction
        
        // Performance validation
        double targetThrust;             // [N] Target from 2D validation (11,519 N)
        double thrustDeviation;          // [%] Deviation from target
        double performanceRating;        // [0-100] Performance score
        
        // Time-dependent analysis
        double instantaneousThrust;      // [N] Current thrust
        double timeAveragedThrust;       // [N] Mean thrust over analysis period
        double thrustVariation;          // [%] RMS variation
        std::vector<std::pair<double, double>> thrustHistory; // Time series
        
        // Wave-specific contributions
        std::map<int, double> waveContributions;    // Thrust per wave ID
        double collisionThrustEnhancement;          // [%] Enhancement from collisions
        double multiWaveAdvantage3D;                // [%] 3D multi-wave benefit
    };
    
    struct SpecificImpulseAnalysis3D {
        // 3D Isp calculations
        double specificImpulse3D;       // [s] 3D specific impulse
        double massFlowRate;            // [kg/s] Total mass flow rate
        double exitVelocity;            // [m/s] Average exit velocity
        double effectiveExhaustVelocity; // [m/s] c* effective velocity
        
        // Nozzle performance
        double nozzleIsp;               // [s] Isp from nozzle expansion alone
        double nozzleEfficiency;        // [-] Nozzle expansion efficiency
        double expansionRatio;          // [-] Area ratio (exit/throat)
        double throatVelocity;          // [m/s] Velocity at nozzle throat
        
        // Flow quality
        double dischargeCoefficientFactor; // [-] Cd factor
        double velocityCoefficient;     // [-] Cv factor
        double thrustCoefficient;       // [-] Cf factor
        
        // Comparative analysis
        double baselineIsp;             // [s] 2D baseline (1,629 s)
        double ispDeviation;            // [%] Deviation from baseline
        double ispRating;               // [0-100] Isp performance score
        
        // Optimization potential
        double theoreticalMaxIsp;       // [s] Theoretical maximum
        double currentEfficiency;       // [%] Current vs theoretical
        std::vector<std::string> optimizationRecommendations;
    };
    
    struct CombustionEfficiencyAnalysis3D {
        // 3D combustion analysis
        double combustionEfficiency3D;   // [%] Overall 3D combustion efficiency
        double fuelConversionRate;       // [%] H2 → H2O conversion
        double combustionCompleteness;   // [%] Reaction completeness
        double mixingEfficiency;         // [%] Fuel-air mixing quality
        
        // Spatial distribution
        std::map<std::string, double> regionEfficiencies; // Efficiency by region
        double axialEfficiencyVariation; // [%] Variation along combustor
        double radialEfficiencyVariation; // [%] Variation across radius
        double circumferentialVariation;  // [%] Variation around circumference
        
        // Wave effects
        double waveEnhancedEfficiency;   // [%] Efficiency in wave regions
        double collisionZoneEfficiency;  // [%] Efficiency in collision zones
        double baseRegionEfficiency;     // [%] Efficiency in non-wave regions
        double waveEfficiencyBonus;      // [%] Bonus from wave presence
        
        // Injection analysis
        double injectionEfficiency;      // [%] Fuel injection effectiveness
        double penetrationDepth;         // [m] Average fuel penetration
        double mixingLength;             // [m] Characteristic mixing length
        std::vector<double> portEfficiencies; // Efficiency per injection port
        
        // Performance comparison
        double baselineEfficiency;       // [%] 2D baseline (85.0%)
        double efficiencyDeviation;      // [%] Deviation from baseline
        double efficiencyRating;         // [0-100] Efficiency score
        
        // Species analysis
        std::map<std::string, double> speciesConcentrations; // Species mass fractions
        std::map<std::string, double> reactionRates;         // Reaction rate analysis
    };
    
    struct NozzlePerformanceAnalysis3D {
        // Nozzle geometry
        double throatArea;               // [m²] Nozzle throat area
        double exitArea;                 // [m²] Nozzle exit area
        double expansionRatio;           // [-] Area expansion ratio
        double nozzleLength;             // [m] Nozzle axial length
        
        // Flow conditions
        double throatPressure;           // [Pa] Pressure at throat
        double throatTemperature;        // [K] Temperature at throat
        double exitPressure;             // [Pa] Pressure at exit
        double exitTemperature;          // [K] Temperature at exit
        
        // Velocity analysis
        double throatVelocity;           // [m/s] Velocity at throat
        double exitVelocity;             // [m/s] Velocity at exit
        double velocityCoefficient;      // [-] Actual/ideal velocity ratio
        
        // Thrust analysis
        double pressureThrust;           // [N] Thrust from pressure forces
        double momentumThrust;           // [N] Thrust from momentum change
        double totalNozzleThrust;        // [N] Total nozzle contribution
        double nozzleEfficiency;         // [%] Nozzle performance efficiency
        
        // Loss analysis
        double frictionLoss;             // [%] Friction losses
        double heatLoss;                 // [%] Heat transfer losses
        double viscousLoss;              // [%] Viscous losses
        double expansionLoss;            // [%] Non-ideal expansion losses
        
        // Optimization
        double optimalExpansionRatio;    // [-] Optimal expansion ratio
        double currentPerformanceRatio;  // [-] Current/optimal performance
        std::vector<std::string> nozzleRecommendations;
    };
    
    struct Performance3DMetrics {
        // Comprehensive 3D performance
        ThrustAnalysis3D thrustAnalysis;
        SpecificImpulseAnalysis3D ispAnalysis;
        CombustionEfficiencyAnalysis3D combustionAnalysis;
        NozzlePerformanceAnalysis3D nozzleAnalysis;
        
        // Overall system metrics
        double overallPerformanceScore;  // [0-100] Combined performance score
        std::string performanceRating;   // "Excellent", "Good", "Needs Optimization"
        double powerDensity;             // [N/m³] Thrust per unit volume
        double thrustToWeight;           // [-] Thrust-to-weight ratio
        
        // Validation against 2D baseline
        double validation2DThrust;       // [N] 2D baseline thrust
        double validation2DIsp;          // [s] 2D baseline Isp
        double validation2DEfficiency;   // [%] 2D baseline efficiency
        double overall3DvsSDRatio;       // [-] 3D/2D performance ratio
        
        // Educational content
        std::string performanceExplanation;
        std::string optimizationGuidance;
        std::string comparisonWith2D;
        std::vector<std::string> keyInsights;
        std::vector<std::string> recommendations;
    };
    
    // Main performance calculation interface
    struct Performance3DRequest {
        std::string caseDirectory;
        RDE3DGeometry::Annular3DGeometry geometry;
        std::vector<RDE3DWaveAnalyzer::Wave3DSystemSnapshot> waveData;
        
        // Analysis parameters
        double analysisStartTime;
        double analysisEndTime;
        double timeStepInterval;
        
        // Target validation (from 2D baseline)
        double targetThrust;             // [N] 11,519 from validation
        double targetIsp;                // [s] 1,629 from validation
        double targetEfficiency;         // [%] 85.0 from validation
        
        // Calculation options
        bool calculateAxialThrust;
        bool analyzeNozzlePerformance;
        bool analyzeCombustionEfficiency;
        bool generateEducationalContent;
        bool optimizePerformance;
        
        // Advanced options
        bool includeViscousEffects;
        bool includeHeatTransfer;
        bool include3DCorrections;
        bool validateAgainst2D;
    };
    
    // Main performance calculation method
    Performance3DMetrics calculatePerformance3D(const Performance3DRequest& request);
    
    // Thrust calculation methods
    ThrustAnalysis3D calculateThrustFromPressureFields3D(const std::string& caseDirectory,
                                                        const RDE3DGeometry::Annular3DGeometry& geometry);
    double integrate3DAxialThrust(const std::string& caseDirectory);
    double calculate3DNozzleThrust(const std::string& caseDirectory, 
                                  const RDE3DGeometry::Annular3DGeometry& geometry);
    std::array<double, 3> calculate3DThrustVector(const std::string& caseDirectory);
    
    // Specific impulse calculation methods
    SpecificImpulseAnalysis3D calculateSpecificImpulse3D(const std::string& caseDirectory,
                                                         const ThrustAnalysis3D& thrustAnalysis,
                                                         const RDE3DGeometry::Annular3DGeometry& geometry);
    double calculate3DMassFlowRate(const std::string& caseDirectory);
    double calculate3DExitVelocity(const std::string& caseDirectory);
    double calculateNozzleIsp(const NozzlePerformanceAnalysis3D& nozzleAnalysis);
    
    // Combustion efficiency methods
    CombustionEfficiencyAnalysis3D analyzeCombustionEfficiency3D(const std::string& caseDirectory,
                                                                const std::vector<RDE3DWaveAnalyzer::Wave3DSystemSnapshot>& waveData);
    double calculateSpeciesConversionRate(const std::string& caseDirectory, const std::string& species);
    double analyzeMixingEfficiency3D(const std::string& caseDirectory);
    std::map<std::string, double> analyzeRegionalEfficiencies(const std::string& caseDirectory);
    
    // Nozzle performance methods
    NozzlePerformanceAnalysis3D analyzeNozzlePerformance3D(const std::string& caseDirectory,
                                                           const RDE3DGeometry::Annular3DGeometry& geometry);
    double calculateNozzleEfficiency(const NozzlePerformanceAnalysis3D& analysis);
    double optimizeExpansionRatio(const RDE3DGeometry::Annular3DGeometry& geometry);
    
    // Performance optimization methods
    Performance3DMetrics optimizePerformance3D(const Performance3DRequest& request);
    std::vector<std::string> generateOptimizationRecommendations(const Performance3DMetrics& metrics);
    double calculateOptimalGeometry(const RDE3DGeometry::Annular3DGeometry& geometry);
    
    // Educational content generation
    std::string generatePerformanceExplanation3D(const Performance3DMetrics& metrics);
    std::string generateOptimizationGuidance3D(const Performance3DMetrics& metrics);
    std::string generateComparison3DvsSD(const Performance3DMetrics& metrics);
    std::vector<std::string> generateKeyInsights3D(const Performance3DMetrics& metrics);
    
    // Validation methods
    bool validatePerformance3D(const Performance3DMetrics& metrics, 
                              double targetThrust, double targetIsp, double targetEfficiency);
    std::string generateValidationReport3D(const Performance3DMetrics& metrics,
                                          double targetThrust, double targetIsp, double targetEfficiency);
    double calculateOverallPerformanceScore(const Performance3DMetrics& metrics);
    
private:
    // 3D field data reading utilities
    bool read3DPressureField(const std::string& timeDir, std::vector<std::vector<std::vector<double>>>& field);
    bool read3DVelocityField(const std::string& timeDir, std::vector<std::vector<std::vector<std::array<double, 3>>>>& field);
    bool read3DTemperatureField(const std::string& timeDir, std::vector<std::vector<std::vector<double>>>& field);
    bool read3DSpeciesField(const std::string& timeDir, const std::string& species,
                           std::vector<std::vector<std::vector<double>>>& field);
    
    // 3D integration utilities
    double integrate3DFieldOverSurface(const std::vector<std::vector<std::vector<double>>>& field,
                                      const std::string& surface);
    double integrate3DFieldOverVolume(const std::vector<std::vector<std::vector<double>>>& field);
    std::array<double, 3> integrate3DVectorField(const std::vector<std::vector<std::vector<std::array<double, 3>>>>& field);
    
    // Performance calculation utilities
    double calculateReynoldsNumber3D(const std::string& caseDirectory);
    double calculateMachNumber3D(const std::string& caseDirectory);
    double calculateHeatReleaseRate3D(const std::string& caseDirectory);
    
    // Nozzle calculation utilities
    double calculateNozzleThrustCoefficient(double expansionRatio, double pressureRatio);
    double calculateNozzleVelocityCoefficient(const NozzlePerformanceAnalysis3D& analysis);
    double calculateNozzleDischargeCoefficient(const NozzlePerformanceAnalysis3D& analysis);
    
    // Optimization utilities
    std::vector<double> optimizeNozzleGeometry(const RDE3DGeometry::Annular3DGeometry& geometry);
    std::vector<double> optimizeInjectionPattern(const RDE3DGeometry::Annular3DGeometry& geometry);
    double optimizeCombustorLength(const RDE3DGeometry::Annular3DGeometry& geometry);
    
    // Educational content utilities
    std::string explain3DThrustGeneration();
    std::string explain3DNozzlePhysics();
    std::string explain3DCombustionPhysics();
    std::string explain3DPerformanceOptimization();
};

/*---------------------------------------------------------------------------*\
                    Helper Functions for 3D Performance Analysis
\*---------------------------------------------------------------------------*/

// 3D thrust calculation helpers
double calculate3DAxialThrustIntegral(const std::vector<std::vector<std::vector<double>>>& pressureField);
std::array<double, 3> calculate3DThrustVectorFromField(const std::vector<std::vector<std::vector<double>>>& pressureField);
double calculate3DViscousDrag(const std::vector<std::vector<std::vector<std::array<double, 3>>>>& velocityField);

// 3D nozzle performance helpers
double calculateIdealNozzleThrust(double throatPressure, double throatArea, double expansionRatio);
double calculateNozzleExpansionEfficiency(double actualThrust, double idealThrust);
double optimizeNozzleExpansionRatio(double combustorPressure, double ambientPressure);

// 3D combustion efficiency helpers
double calculate3DFuelConversionRate(const std::vector<std::vector<std::vector<double>>>& fuelField);
double calculate3DMixingEfficiency(const std::vector<std::vector<std::vector<double>>>& fuelField,
                                  const std::vector<std::vector<std::vector<double>>>& oxidantField);
double analyze3DCombustionCompleteness(const std::vector<std::vector<std::vector<double>>>& productField);

// Educational content helpers
std::string explain3DRDEPerformanceAdvantages();
std::string explain3DAxialThrustGeneration();
std::string explain3DNozzleDesignPrinciples();
std::string explain3DPerformanceOptimization();

} // namespace MCP
} // namespace Foam