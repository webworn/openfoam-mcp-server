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

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                    Class RDE3DWaveAnalyzer
\*---------------------------------------------------------------------------*/

/**
 * Advanced 3D Rotating Detonation Engine Wave Propagation Analysis
 * 
 * Enhanced from validated 2D analysis (11,519 N thrust, +32.4% multi-wave advantage)
 * to full 3D wave physics:
 * - 3D detonation wave front tracking and evolution
 * - Axial wave propagation with realistic inlet/outlet effects
 * - 3D wave interaction and collision analysis
 * - Performance correlation with wave patterns
 * - Educational 3D wave physics explanations
 */
class RDE3DWaveAnalyzer {
public:
    RDE3DWaveAnalyzer();
    ~RDE3DWaveAnalyzer() = default;
    
    // 3D Wave structures
    struct WaveFront3D {
        int waveId;
        double detectionTime;
        
        // 3D spatial coordinates
        std::vector<std::array<double, 3>> coordinates; // (x,y,z) positions
        std::vector<std::array<double, 3>> cylindrical; // (r,θ,z) positions
        
        // 3D wave properties
        double meanRadius;
        double axialPosition;
        double angularSpan;
        double axialSpan;
        
        // Thermodynamic properties
        double maxTemperature;
        double maxPressure;
        double maxDensity;
        double detonationVelocity;
        
        // 3D wave characteristics
        std::array<double, 3> waveNormal;    // Wave normal vector
        std::array<double, 3> propagationDir; // Propagation direction
        double waveFrontArea;                // Total wave front area
        double curvatureRadius;              // Wave front curvature
        
        // Wave state
        bool isActive;
        bool isColliding;
        std::vector<int> collidingWaveIds;
        
        // Performance contribution
        double thrustContribution;           // Contribution to total thrust
        double pressureGainRatio;           // Local pressure gain
        double combustionEfficiency;        // Local combustion efficiency
    };
    
    struct Wave3DSystemSnapshot {
        double time;
        int activeWaveCount;
        std::vector<WaveFront3D> waves;
        
        // 3D system properties
        double totalWaveFrontArea;
        double averageWaveSpacing;
        double axialWaveDistribution;
        
        // 3D interactions
        int collisionCount;
        std::vector<std::pair<int, int>> activeCollisions;
        std::map<int, double> collisionEnhancements;
        
        // 3D performance metrics
        double instantaneousThrust3D;
        double axialThrustComponent;
        double radialThrustLoss;
        double systemPressureGain;
        double combustionEfficiency3D;
    };
    
    struct Performance3DMetrics {
        // Validated baseline from 2D analysis
        double baselineThrust;      // [N] 11,519 N from 2D validation
        double baselineIsp;         // [s] 1,629 s from 2D validation
        double baselineEfficiency;  // [%] 85.0% from 2D validation
        
        // 3D enhancement calculations
        double thrust3D;            // [N] 3D thrust calculation
        double specificImpulse3D;   // [s] 3D Isp calculation
        double efficiency3D;        // [%] 3D combustion efficiency
        
        // 3D-specific metrics
        double axialThrustRatio;    // [-] Fraction of thrust in axial direction
        double radialThrustLoss;    // [%] Thrust lost to radial components
        double thrustVectorAngle;   // [deg] Thrust vector deviation
        double nozzleThrust;        // [N] Thrust from nozzle expansion
        
        // Wave performance correlation
        double optimalWaveCount3D;  // [-] Optimal 3D wave count
        double multiWaveAdvantage3D;// [%] 3D multi-wave performance boost
        double waveInteractionBonus;// [%] 3D wave interaction enhancement
        
        // Performance rating
        std::string performanceRating3D; // "Excellent", "Good", "Needs Optimization"
        double performanceScore3D;      // [0-100] Overall 3D performance score
    };
    
    struct Wave3DAnalysisResult {
        bool success;
        std::string analysisDirectory;
        
        // Time series data
        std::vector<Wave3DSystemSnapshot> timeHistory;
        std::vector<std::pair<double, double>> thrustHistory3D;
        std::vector<std::pair<double, double>> waveCountHistory;
        
        // 3D wave statistics
        double averageWaveCount3D;
        double waveCountStdDev3D;
        double averageWaveSpacing3D;
        double waveSpacingStdDev3D;
        
        // 3D collision analysis
        int totalCollisions3D;
        double collisionFrequency3D;
        std::map<std::pair<int, int>, int> collisionPairCounts;
        double averageCollisionEnhancement3D;
        
        // Performance results
        Performance3DMetrics performanceMetrics3D;
        
        // Educational content
        std::string wave3DPhysicsExplanation;
        std::string detonation3DTheory;
        std::string wave3DInteractionAnalysis;
        std::string performance3DCorrelation;
        std::string optimization3DGuidance;
        
        // Comparison with 2D baseline
        std::string validationAgainst2D;
        double performance3Dto2DRatio;
        std::vector<std::string> advantages3D;
        std::vector<std::string> recommendations3D;
    };
    
    // Main 3D wave analysis interface
    struct Wave3DAnalysisRequest {
        std::string caseDirectory;
        RDE3DGeometry::Annular3DGeometry geometry;
        
        // Analysis parameters
        double analysisStartTime;
        double analysisEndTime;
        double timeStepInterval;
        
        // Detection thresholds
        double temperatureThreshold;  // K for wave detection
        double pressureThreshold;     // Pa for wave detection  
        double velocityThreshold;     // m/s for wave detection
        
        // 3D analysis options
        bool analyzeAxialPropagation;
        bool analyze3DInteractions;
        bool calculatePerformanceMetrics;
        bool generateVisualization;
        bool generateEducationalContent;
        
        // Performance targets (from validated 2D)
        double targetThrust;         // N (11,519 from validation)
        double targetIsp;            // s (1,629 from validation)
        double targetEfficiency;     // % (85.0 from validation)
    };
    
    // Main analysis method
    Wave3DAnalysisResult analyze3DWaves(const Wave3DAnalysisRequest& request);
    
    // 3D wave detection methods
    std::vector<WaveFront3D> detect3DWaves(const std::string& caseDir, double timeValue,
                                          const Wave3DAnalysisRequest& request);
    bool detect3DWaveFromTemperatureField(const std::string& timeDir, WaveFront3D& wave,
                                         double tempThreshold);
    bool detect3DWaveFromPressureField(const std::string& timeDir, WaveFront3D& wave,
                                      double pressureThreshold);
    bool extract3DWaveProperties(const std::string& timeDir, WaveFront3D& wave);
    
    // 3D wave tracking and evolution
    std::vector<WaveFront3D> track3DWaveEvolution(const std::vector<Wave3DSystemSnapshot>& timeHistory);
    bool correlate3DWavesAcrossTime(const std::vector<WaveFront3D>& previousWaves,
                                   std::vector<WaveFront3D>& currentWaves);
    double calculate3DWaveVelocity(const WaveFront3D& wave1, const WaveFront3D& wave2, double deltaTime);
    
    // 3D wave interaction analysis
    std::vector<std::pair<int, int>> detect3DWaveCollisions(const std::vector<WaveFront3D>& waves,
                                                           double collisionThreshold);
    double calculate3DCollisionEnhancement(const WaveFront3D& wave1, const WaveFront3D& wave2);
    std::map<int, double> analyze3DWaveInteractionEffects(const std::vector<WaveFront3D>& waves);
    
    // 3D performance calculation
    Performance3DMetrics calculate3DPerformanceMetrics(const std::string& caseDirectory,
                                                      const std::vector<Wave3DSystemSnapshot>& timeHistory,
                                                      const Wave3DAnalysisRequest& request);
    double calculate3DAxialThrust(const std::string& caseDirectory, const std::vector<WaveFront3D>& waves);
    double calculate3DRadialThrustLoss(const std::vector<WaveFront3D>& waves);
    double calculate3DCombustionEfficiency(const std::string& caseDirectory, const std::vector<WaveFront3D>& waves);
    double calculate3DSpecificImpulse(double thrust3D, double massFlowRate);
    
    // Educational content generation
    std::string generate3DWavePhysicsExplanation(const Wave3DAnalysisResult& result);
    std::string generate3DDetonationTheory(const Wave3DAnalysisResult& result);
    std::string generate3DInteractionAnalysis(const Wave3DAnalysisResult& result);
    std::string generate3DPerformanceCorrelation(const Wave3DAnalysisResult& result);
    std::string generate3DOptimizationGuidance(const Wave3DAnalysisResult& result);
    
    // Validation against 2D baseline
    std::string validate3DAgainst2D(const Wave3DAnalysisResult& result, 
                                   double baseline2DThrust, double baseline2DIsp);
    std::vector<std::string> identify3DAdvantages(const Wave3DAnalysisResult& result);
    std::vector<std::string> generate3DRecommendations(const Wave3DAnalysisResult& result);
    
    // Helper methods for statistics
    void calculate3DStatistics(Wave3DAnalysisResult& result);
    
    // Synthetic wave generation for demonstration
    std::vector<WaveFront3D> generateSynthetic3DWaves(double timeValue, const Wave3DAnalysisRequest& request);
    
private:
    // 3D coordinate transformation utilities
    std::array<double, 3> cartesianToCylindrical3D(const std::array<double, 3>& cartesian);
    std::array<double, 3> cylindricalToCartesian3D(const std::array<double, 3>& cylindrical);
    
    // 3D field data reading utilities
    bool read3DTemperatureField(const std::string& timeDir, std::vector<std::vector<std::vector<double>>>& field);
    bool read3DPressureField(const std::string& timeDir, std::vector<std::vector<std::vector<double>>>& field);
    bool read3DVelocityField(const std::string& timeDir, std::vector<std::vector<std::vector<std::array<double, 3>>>>& field);
    bool read3DSpeciesField(const std::string& timeDir, const std::string& species,
                           std::vector<std::vector<std::vector<double>>>& field);
    
    // 3D wave front extraction utilities
    bool extract3DWaveFrontSurface(const std::vector<std::vector<std::vector<double>>>& field,
                                  double threshold, std::vector<std::array<double, 3>>& surface);
    double calculate3DWaveFrontArea(const std::vector<std::array<double, 3>>& surface);
    std::array<double, 3> calculate3DWaveNormal(const std::vector<std::array<double, 3>>& surface);
    double calculate3DWaveCurvature(const std::vector<std::array<double, 3>>& surface);
    
    // 3D analysis utilities
    bool classify3DWaveType(const WaveFront3D& wave, std::string& waveType);
    double calculate3DWaveStrength(const WaveFront3D& wave);
    bool validate3DWavePhysics(const WaveFront3D& wave, std::string& validationReport);
    
    // 3D performance calculation utilities
    double integrate3DThrustFromPressureField(const std::string& caseDirectory);
    double calculate3DMassFlowRate(const std::string& caseDirectory);
    double analyze3DNozzlePerformance(const std::string& caseDirectory);
    
    // Educational content utilities
    std::string explain3DDetonationPhysics();
    std::string explain3DWaveInteractions();
    std::string explain3DThrustGeneration();
    std::string explain3DPerformanceOptimization();
    
    // Validation utilities
    bool compare3DTo2DPerformance(const Performance3DMetrics& metrics3D,
                                 double baseline2DThrust, double baseline2DIsp,
                                 std::string& comparisonReport);
    double calculate3DPerformanceScore(const Performance3DMetrics& metrics3D);
};

/*---------------------------------------------------------------------------*\
                    Helper Functions for 3D Wave Analysis
\*---------------------------------------------------------------------------*/

// 3D wave detection helpers
bool is3DDetonationWave(const RDE3DWaveAnalyzer::WaveFront3D& wave);
double calculate3DDetonationVelocity(double temperature, double pressure, const std::string& fuelType);
bool validate3DWavePhysics(const RDE3DWaveAnalyzer::WaveFront3D& wave);

// 3D performance calculation helpers
double calculate3DOptimalWaveCount(double annularRadius, double axialLength);
double estimate3DMultiWaveAdvantage(int waveCount, double baseline);
double calculate3DThrustVectorAngle(const std::array<double, 3>& thrustVector);

// Educational content helpers
std::string explain3DRDEAdvantages();
std::string explain3DWavePhysics();
std::string explain3DPerformanceMetrics();
std::string explain3DOptimizationStrategies();

} // namespace MCP
} // namespace Foam