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
                    Class RDE2DWaveAnalyzer - Simplified
\*---------------------------------------------------------------------------*/

/**
 * Simplified 2D RDE Wave Interaction Analysis System
 * 
 * Core Phase 1 implementation focusing on:
 * - Temperature gradient wave detection
 * - Wave counting and tracking
 * - Basic collision detection
 * - Educational physics explanations
 */
class RDE2DWaveAnalyzer {
public:
    RDE2DWaveAnalyzer();
    ~RDE2DWaveAnalyzer() = default;
    
    // Core wave detection structures
    struct WaveFront {
        int waveId;
        double detectionTime;
        std::vector<std::pair<double, double>> coordinates; // (r,θ) positions
        double meanRadius;
        double angularSpan;
        double maxTemperature;
        double maxPressure;
        double waveSpeed;
        bool isActive;
    };
    
    struct WaveSystemSnapshot {
        double time;
        int activeWaveCount;
        std::vector<WaveFront> waves;
        std::string pattern;
    };
    
    struct WaveCollision {
        double collisionTime;
        std::pair<double, double> location;
        std::vector<int> involvedWaves;
        std::string collisionType;
        double energyRelease;
    };
    
    struct WaveAnalysisResult {
        std::string caseDirectory;
        std::vector<WaveSystemSnapshot> timeHistory;
        std::vector<WaveCollision> collisions;
        double avgWaveCount;
        double avgWaveSpeed;
        double systemStability;
        std::string wavePhysicsExplanation;
        std::string collisionAnalysis;
        std::string performanceAssessment;
        
        // Phase 2: Performance metrics integration
        struct PerformanceMetrics {
            double thrust;                    // [N] Time-averaged thrust
            double specificImpulse;          // [s] Isp = F/(mdot*g)
            double massFlowRate;             // [kg/s] Total mass flow
            double combustionEfficiency;     // [%] H2→H2O conversion
            double thrustVariation;          // [%] RMS thrust variation
            double exitVelocity;             // [m/s] Average exit velocity
            double chamberPressure;          // [Pa] Average chamber pressure
            double pressureGainRatio;        // [-] vs constant pressure combustion
            
            // Wave impact metrics
            double multiWaveAdvantage;       // [%] vs single wave operation
            double collisionThrustBonus;     // [%] enhancement from collisions
            double waveEfficiencyCorrelation; // [-] efficiency vs wave count correlation
            
            // Comparative analysis
            std::string performanceRating;   // "Excellent", "Good", "Needs Optimization"
            double thrustToWeightRatio;      // [-] T/W ratio
            double theoreticalEfficiency;    // [%] vs Chapman-Jouguet theoretical
        } performanceMetrics;
        
        // Enhanced educational content with performance correlations
        std::string thrustGenerationExplanation;
        std::string specificImpulseAnalysis;
        std::string combustionEfficiencyAnalysis;
        std::string wavePerformanceCorrelations;
        std::string optimizationRecommendations;
        std::string comparisonWithConventionalSystems;
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
    
    // Educational content generation
    std::string explainWaveInteractionPhysics(const WaveAnalysisResult& result);
    std::string explainCollisionMechanisms(const std::vector<WaveCollision>& collisions);
    std::string generatePerformanceAssessment(const WaveAnalysisResult& result);
    
    // Phase 2: Performance metrics calculation methods
    WaveAnalysisResult::PerformanceMetrics calculatePerformanceMetrics(const std::string& caseDirectory,
                                                                       const std::vector<WaveSystemSnapshot>& timeHistory);
    
    std::string generateThrustExplanation(const WaveAnalysisResult::PerformanceMetrics& metrics);
    std::string generateIspAnalysis(const WaveAnalysisResult::PerformanceMetrics& metrics);
    std::string generateCombustionAnalysis(const WaveAnalysisResult::PerformanceMetrics& metrics);
    std::string generateWavePerformanceCorrelations(const WaveAnalysisResult& result);
    std::string generateOptimizationGuidance(const WaveAnalysisResult::PerformanceMetrics& metrics);
    std::string compareWithConventionalSystems(const WaveAnalysisResult::PerformanceMetrics& metrics);
    
private:
    // Wave detection helpers
    std::vector<std::vector<double>> calculateTemperatureGradient(const std::vector<std::vector<double>>& temperatureField);
    std::vector<std::pair<double, double>> extractWaveFrontFromGradient(const std::vector<std::vector<double>>& gradientField,
                                                                       double threshold);
    std::vector<std::vector<std::pair<double, double>>> clusterWavePoints(const std::vector<std::pair<double, double>>& points,
                                                                         double clusterRadius = 0.005);
    
    WaveFront createWaveFrontFromPoints(const std::vector<std::pair<double, double>>& points,
                                      int waveId, double time);
    
    // Wave tracking helpers
    int findCorrespondingWave(const WaveFront& currentWave, const std::vector<WaveFront>& previousWaves);
    double calculateWaveDistance(const WaveFront& wave1, const WaveFront& wave2);
    
    // Collision detection helpers
    bool checkWaveCollision(const WaveFront& wave1, const WaveFront& wave2, double tolerance = 0.002);
    std::string classifyCollisionType(const WaveFront& wave1, const WaveFront& wave2);
    
    // System analysis helpers
    double calculateAverageWaveCount(const std::vector<WaveSystemSnapshot>& timeHistory);
    double calculateAverageWaveSpeed(const std::vector<WaveSystemSnapshot>& timeHistory);
    double calculateSystemStability(const std::vector<WaveSystemSnapshot>& timeHistory);
    std::string classifyWavePattern(int activeWaveCount);
    
    // Utility methods
    double distanceBetween(const std::pair<double, double>& p1, const std::pair<double, double>& p2);
    std::pair<double, double> cartesianToCylindrical(double x, double y);
    
    // Configuration
    double temperatureThreshold_ = 2500.0;
    double gradientThreshold_ = 1.0e8;
    double waveTrackingTolerance_ = 0.005;
    double collisionTolerance_ = 0.002;
    int nextWaveId_ = 1;
};

} // namespace MCP
} // namespace Foam