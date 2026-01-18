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

// Forward declarations for OpenFOAM types
class RDEGeometry {
public:
    double getAnnularArea() const { return 0.01; } // m²
    double getInjectionVelocity() const { return 100.0; } // m/s
    double getInjectionArea() const { return 0.001; } // m²
};

class RDEChemistry {
public:
    double getDensity() const { return 1.2; } // kg/m³
};

class RDESimulationSettings {
public:
    double meshSize = 0.001; // m
};

/*---------------------------------------------------------------------------*\
                        Class RDE2DWaveAnalyzer
\*---------------------------------------------------------------------------*/

/**
 * Advanced 2D RDE Wave Interaction Analysis System
 * 
 * Analyzes production 2D RDE simulation data for:
 * - Multi-wave detection and tracking
 * - Wave collision analysis
 * - Performance metrics calculation
 * - Educational wave interaction physics
 */
class RDE2DWaveAnalyzer {
public:
    RDE2DWaveAnalyzer();
    ~RDE2DWaveAnalyzer() = default;
    
    // Wave detection and analysis structures
    struct WaveFront {
        int waveId;                    // Unique wave identifier
        double detectionTime;          // Time of detection [s]
        std::vector<std::pair<double, double>> coordinates; // (r,θ) positions [m, rad]
        double meanRadius;             // Mean radial position [m]
        double angularSpan;            // Angular extent [rad]
        double maxTemperature;         // Peak temperature [K]
        double maxPressure;           // Peak pressure [Pa]
        double waveSpeed;             // Estimated wave speed [m/s]
        double strength;              // Wave strength metric
        bool isActive;                // Wave activity flag
    };
    
    struct WaveSystemSnapshot {
        double time;                   // Snapshot time [s]
        int activeWaveCount;          // Number of active waves
        std::vector<WaveFront> waves; // Individual wave data
        double systemFrequency;       // Overall frequency [Hz]
        std::string pattern;          // "single", "double", "triple", "multiple"
        double totalEnergy;           // Total system energy [J]
        double pressureOscillation;   // RMS pressure oscillation [Pa]
    };
    
    struct WaveCollision {
        double collisionTime;         // Time of collision [s]
        std::pair<double, double> location; // (r,θ) collision point [m, rad]
        std::vector<int> involvedWaves; // Wave IDs involved
        std::string collisionType;    // "head-on", "oblique", "merging", "splitting"
        double energyRelease;         // Energy released [J]
        double pressureSpike;         // Peak pressure increase [Pa]
        double temperatureSpike;      // Peak temperature increase [K]
        std::vector<WaveFront> resultingWaves; // Post-collision wave pattern
    };
    
    struct WaveAnalysisResult {
        std::string caseDirectory;
        std::vector<WaveSystemSnapshot> timeHistory;
        std::vector<WaveCollision> collisions;
        double avgWaveCount;
        double avgWaveSpeed;
        double systemStability;       // [0-1] stability metric
        double combustionEfficiency;
        double specificImpulse;       // [s]
        double thrust;                // [N]
        
        // Educational content
        std::string wavePhysicsExplanation;
        std::string collisionAnalysis;
        std::string performanceAssessment;
        std::vector<std::string> designRecommendations;
        std::string safetyConsiderations;
    };
    
    // Core analysis methods
    WaveAnalysisResult analyzeWaveInteractions(const std::string& caseDirectory);
    
    std::vector<WaveFront> detectWaveFronts(const std::string& timeDirectory,
                                           double temperatureThreshold = 2500.0);
    
    int countActiveWaves(const std::vector<WaveFront>& waveFronts);
    
    std::vector<WaveFront> trackWaveEvolution(const std::vector<WaveFront>& previousWaves,
                                            const std::vector<WaveFront>& currentWaves,
                                            double timeStep);
    
    std::vector<WaveCollision> detectWaveCollisions(const WaveSystemSnapshot& snapshot,
                                                   const WaveSystemSnapshot& previousSnapshot);
    
    // Performance metrics calculation
    double calculateSpecificImpulse(const WaveAnalysisResult& result,
                                  const RDEGeometry& geometry,
                                  const RDEChemistry& chemistry);
    
    double calculateThrust(const WaveAnalysisResult& result,
                         const RDEGeometry& geometry);
    
    double calculateCombustionEfficiency(const std::vector<WaveSystemSnapshot>& timeHistory,
                                       const RDEChemistry& chemistry);
    
    // Educational content generation
    std::string explainWaveInteractionPhysics(const WaveAnalysisResult& result);
    std::string explainCollisionMechanisms(const std::vector<WaveCollision>& collisions);
    std::string generateDesignGuidance(const WaveAnalysisResult& result);
    std::string assessSafetyImplications(const WaveAnalysisResult& result);
    
    // Visualization support
    struct VisualizationData {
        std::string waveTrajectoryData;   // Wave path data for plotting
        std::string collisionLocationData; // Collision point markers
        std::string pressureFieldData;    // Pressure contour data
        std::string temperatureFieldData; // Temperature contour data
        std::string performanceTimeHistory; // Performance vs time plots
    };
    
    VisualizationData generateVisualizationData(const WaveAnalysisResult& result);
    
private:
    // OpenFOAM field data reading
    std::vector<std::vector<double>> readScalarField(const std::string& fieldPath);
    std::vector<std::vector<std::pair<double, double>>> readVectorField(const std::string& fieldPath);
    std::vector<std::pair<double, double>> readMeshCoordinates(const std::string& meshPath);
    
    // Temperature gradient analysis
    std::vector<std::vector<double>> calculateTemperatureGradient(const std::vector<std::vector<double>>& temperatureField,
                                                                const std::vector<std::pair<double, double>>& coordinates);
    
    std::vector<std::pair<double, double>> extractWaveFrontFromGradient(const std::vector<std::vector<double>>& gradientField,
                                                                       const std::vector<std::pair<double, double>>& coordinates,
                                                                       double threshold);
    
    // Wave identification algorithms
    WaveFront createWaveFrontFromPoints(const std::vector<std::pair<double, double>>& points,
                                      const std::vector<std::vector<double>>& temperatureField,
                                      const std::vector<std::vector<double>>& pressureField,
                                      int waveId, double time);
    
    std::vector<std::vector<std::pair<double, double>>> clusterWavePoints(const std::vector<std::pair<double, double>>& points,
                                                                         double clusterRadius = 0.005);
    
    // Wave tracking algorithms
    int findCorrespondingWave(const WaveFront& currentWave, const std::vector<WaveFront>& previousWaves);
    double calculateWaveDistance(const WaveFront& wave1, const WaveFront& wave2);
    
    // Collision detection algorithms
    bool checkWaveCollision(const WaveFront& wave1, const WaveFront& wave2, double tolerance = 0.002);
    WaveCollision analyzeCollisionEvent(const std::vector<WaveFront>& collidingWaves,
                                      double time,
                                      const std::vector<std::vector<double>>& pressureField,
                                      const std::vector<std::vector<double>>& temperatureField);
    
    // Performance calculation helpers
    double integrateThrust(const std::vector<std::vector<double>>& pressureField,
                         const RDEGeometry& geometry);
    
    double calculateMassFlowRate(const std::vector<std::vector<double>>& densityField,
                               const std::vector<std::vector<std::pair<double, double>>>& velocityField,
                               const RDEGeometry& geometry);
    
    // Educational content helpers
    std::map<std::string, std::string> educationalTemplates_;
    void initializeEducationalContent();
    std::string generatePhysicsExplanation(const std::string& phenomenon, const json& data);
    
    // Utility methods
    double angleBetween(const std::pair<double, double>& p1, const std::pair<double, double>& p2);
    double distanceBetween(const std::pair<double, double>& p1, const std::pair<double, double>& p2);
    std::pair<double, double> cartesianToCylindrical(double x, double y);
    std::pair<double, double> cylindricalToCartesian(double r, double theta);
    
    // Configuration parameters
    double temperatureThreshold_ = 2500.0;    // Wave detection threshold [K]
    double pressureThreshold_ = 1.0e6;        // Wave pressure threshold [Pa]
    double gradientThreshold_ = 1.0e8;        // Temperature gradient threshold [K/m]
    double waveTrackingTolerance_ = 0.005;    // Wave tracking tolerance [m]
    double collisionTolerance_ = 0.002;       // Collision detection tolerance [m]
    
    // Data storage
    std::vector<WaveSystemSnapshot> analysisHistory_;
    std::vector<WaveCollision> detectedCollisions_;
    int nextWaveId_ = 1;                      // Wave ID counter
};

/*---------------------------------------------------------------------------*\
                    Helper Functions for Wave Analysis
\*---------------------------------------------------------------------------*/

// Wave physics utilities
double calculateChapmanJouguetVelocity(const RDEChemistry& chemistry, double temperature, double pressure);
double calculateDetonationCellSize(const RDEChemistry& chemistry, double temperature, double pressure);
double estimateWaveStrength(double maxTemperature, double maxPressure, const RDEChemistry& chemistry);

// Statistical analysis utilities
double calculateRMSPressureOscillation(const std::vector<WaveSystemSnapshot>& timeHistory);
double calculateSystemStability(const std::vector<WaveSystemSnapshot>& timeHistory);
double calculateAverageWaveSpeed(const std::vector<WaveSystemSnapshot>& timeHistory);

// Educational content generators
std::string explainMultiWavePhysics(int waveCount, const std::string& pattern);
std::string explainWaveCollisionPhysics(const std::string& collisionType);
std::string explainPerformanceImplications(double thrust, double specificImpulse, double efficiency);

} // namespace MCP
} // namespace Foam