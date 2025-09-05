#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cmath>
#include <functional>
#include <nlohmann/json.hpp>
#include "cellular_detonation_model.hpp"
#include "rotating_detonation_engine.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                    Class CellularDetonation2D
\*---------------------------------------------------------------------------*/

/**
 * 2D Cylindrical Cellular Detonation Model for RDE Analysis
 * 
 * Extends the 1D cellular detonation model to support:
 * - 2D wave propagation in cylindrical coordinates (r, θ)
 * - Multi-wave interactions and collision dynamics
 * - Injection-wave coupling in annular geometry
 * - Cellular structure preservation in curved domains
 * - Educational wave tracking and visualization
 */
class CellularDetonation2D : public CellularDetonationModel {
public:
    CellularDetonation2D();
    ~CellularDetonation2D() = default;
    
    // 2D Wave tracking data structure
    struct Wave2DPoint {
        double r;              // Radial position [m]
        double theta;          // Angular position [rad]
        double time;           // Time [s]
        double temperature;    // Temperature [K]
        double pressure;       // Pressure [Pa]
        double velocity_r;     // Radial velocity [m/s]
        double velocity_theta; // Tangential velocity [m/s]
        double waveSpeed;      // Local wave speed [m/s]
        double cellSize;       // Local cell size [m]
        bool isWaveFront;      // Wave front indicator
    };
    
    // 2D Cellular structure in cylindrical coordinates
    struct CellularStructure2D {
        double meanCellSize;           // Mean cell size in domain [m]
        double radialVariation;        // Cell size variation in r-direction
        double circumferentialVariation; // Cell size variation in θ-direction
        std::vector<std::vector<double>> cellSizeField; // 2D cell size distribution
        double structureRegularity;   // Cellular structure regularity [0-1]
        std::vector<Wave2DPoint> triplePoints; // Triple point locations
        double waveAngle;             // Average wave angle relative to radial [rad]
        double curvatureEffect;       // Curvature effect on cell size
    };
    
    // 2D Wave propagation analysis
    struct WavePropagation2D {
        std::vector<Wave2DPoint> waveTrajectory;     // Wave front trajectory
        double propagationSpeed;                      // Average propagation speed [m/s]
        double speedVariation;                        // Speed variation around annulus
        std::vector<double> localWaveSpeeds;         // Local wave speeds at different θ
        double waveThickness;                         // Wave front thickness [m]
        std::vector<std::pair<double, double>> waveCollisionPoints; // (r, θ) collision locations
        double energyDissipation;                     // Energy dissipated in interactions
    };
    
    // Injection-wave interaction analysis
    struct InjectionWaveInteraction {
        int injectorIndex;                           // Injector ID
        double injectorPosition_theta;               // Angular position [rad]
        double wavePhaseAtInjection;                // Wave phase when passing injector [rad]
        double momentumCoupling;                     // Injection momentum coupling
        double pressureDisturbance;                  // Pressure disturbance magnitude [Pa]
        std::vector<Wave2DPoint> interactionRegion; // Near-field interaction data
        double penetrationDepth;                     // Injection penetration [m]
        std::string interactionType;                // "reinforcing", "opposing", "neutral"
    };
    
    // Core 2D functionality
    CellularStructure2D analyze2DCellularStructure(const RDEGeometry& geometry, 
                                                   const RDEChemistry& chemistry);
    
    WavePropagation2D track2DWavePropagation(const RDEGeometry& geometry,
                                             const RDEChemistry& chemistry,
                                             const std::vector<Wave2DPoint>& initialWave,
                                             double simulationTime);
    
    std::vector<InjectionWaveInteraction> analyzeInjectionWaveInteractions(
        const RDEGeometry& geometry,
        const RDEChemistry& chemistry, 
        const WavePropagation2D& waveProp);
    
    // Multi-wave analysis
    struct MultiWaveSystem {
        int waveCount;                              // Number of waves
        std::vector<WavePropagation2D> waves;      // Individual wave data
        std::vector<std::pair<int, int>> collisionPairs; // Wave collision pairs
        double systemFrequency;                     // Overall system frequency [Hz]
        std::string wavePattern;                    // "co_rotating", "counter_rotating", "mixed"
        double stabilityIndex;                      // System stability metric [0-1]
        std::vector<std::pair<double, double>> waveSpacings; // Angular spacing between waves
    };
    
    MultiWaveSystem analyzeMultiWaveSystem(const RDEGeometry& geometry,
                                          const RDEChemistry& chemistry,
                                          const std::vector<WavePropagation2D>& waves);
    
    // Cylindrical coordinate utilities
    std::pair<double, double> cartesianToCylindrical(double x, double y) const;
    std::pair<double, double> cylindricalToCartesian(double r, double theta) const;
    
    // Wave front detection and tracking
    std::vector<Wave2DPoint> detectWaveFront(const std::vector<std::vector<Wave2DPoint>>& fieldData,
                                            double temperatureThreshold = 2500.0) const;
    
    std::vector<Wave2DPoint> trackWaveFrontEvolution(const std::vector<Wave2DPoint>& previousFront,
                                                   const std::vector<std::vector<Wave2DPoint>>& newFieldData,
                                                   double timeStep) const;
    
    // Cellular constraint validation in 2D
    bool validate2DCellularConstraints(const RDEGeometry& geometry,
                                      const RDEChemistry& chemistry,
                                      const RDESimulationSettings& settings,
                                      std::string& report) const;
    
    double calculate2DRequiredMeshSize(const CellularStructure2D& structure,
                                     const RDEGeometry& geometry,
                                     double safetyFactor = 10.0) const;
    
    // Curvature effects on cellular structure
    double calculateCurvatureEffect(double radius, double cellSize) const;
    void applyCurvatureCorrection(CellularStructure2D& structure, 
                                const RDEGeometry& geometry) const;
    
    // Wave collision and interaction modeling
    struct WaveCollision {
        double collisionPosition_r;     // Radial position of collision [m]
        double collisionPosition_theta; // Angular position of collision [rad]
        double collisionTime;          // Time of collision [s]
        std::string collisionType;     // "head_on", "oblique", "merging"
        double energyRelease;          // Energy released in collision [J]
        std::vector<Wave2DPoint> postCollisionWaves; // Resulting wave pattern
        double cellSizeDisturbance;    // Local cell size perturbation
    };
    
    std::vector<WaveCollision> predictWaveCollisions(const MultiWaveSystem& system,
                                                    double predictionTime) const;
    
    // Educational content generation for 2D physics
    std::string explain2DWaveDynamics(const WavePropagation2D& waveProp) const;
    std::string explainCylindricalEffects(const CellularStructure2D& structure) const;
    std::string explainInjectionCoupling(const std::vector<InjectionWaveInteraction>& interactions) const;
    std::string explainMultiWavePhysics(const MultiWaveSystem& system) const;
    
    // Wave visualization helpers
    struct WaveVisualization {
        std::string pressureContourData;    // Pressure contour data for plotting
        std::string temperatureContourData; // Temperature contour data 
        std::string velocityVectorData;     // Velocity vector field data
        std::string waveTrajectoryData;     // Wave trajectory line data
        std::vector<std::pair<double, double>> triplePointPositions; // Triple point markers
        std::string cellularPatternData;    // Cellular pattern overlay
    };
    
    WaveVisualization generate2DVisualization(const WavePropagation2D& waveProp,
                                             const CellularStructure2D& structure,
                                             const RDEGeometry& geometry) const;
    
    // Performance and accuracy metrics
    struct Analysis2DMetrics {
        double computationalCost;        // Relative computational cost vs 1D
        double accuracyImprovement;      // Accuracy improvement over 1D model
        double memoryRequirement;       // Memory requirement factor
        std::vector<std::string> recommendations; // Performance recommendations
        double validationConfidence;    // Confidence in 2D predictions [0-1]
        std::string applicabilityRange; // Recommended application range
    };
    
    Analysis2DMetrics evaluate2DAnalysis(const RDEGeometry& geometry,
                                       const CellularStructure2D& structure,
                                       const WavePropagation2D& waveProp) const;
    
private:
    // 2D-specific calculation helpers
    double interpolateFieldValue(const std::vector<std::vector<Wave2DPoint>>& field,
                               double r, double theta) const;
    
    std::vector<double> calculateGradientMagnitude(const std::vector<std::vector<Wave2DPoint>>& field,
                                                 const std::string& variable) const;
    
    double calculateWaveSpeedInCylindricalCoords(const Wave2DPoint& point,
                                               const RDEGeometry& geometry) const;
    
    // Wave interaction prediction algorithms
    bool willWavesCollide(const Wave2DPoint& wave1, const Wave2DPoint& wave2,
                        double timeHorizon) const;
    
    Wave2DPoint predictWavePosition(const Wave2DPoint& currentPoint,
                                  double timeStep,
                                  const RDEGeometry& geometry) const;
    
    // Cellular structure initialization
    void initialize2DCellularField(CellularStructure2D& structure,
                                 const RDEGeometry& geometry,
                                 const RDEChemistry& chemistry) const;
    
    void updateCellularStructureFromWaves(CellularStructure2D& structure,
                                        const std::vector<WavePropagation2D>& waves) const;
    
    // Data storage for tracking
    std::vector<WavePropagation2D> waveHistory_;
    std::vector<CellularStructure2D> structureHistory_;
    std::map<int, std::vector<InjectionWaveInteraction>> injectionInteractionHistory_;
    
    // Configuration parameters
    double temperatureThreshold_ = 2500.0;    // Wave detection threshold [K]
    double pressureThreshold_ = 1.0e6;        // Wave detection threshold [Pa]
    double gradientThreshold_ = 1.0e8;        // Gradient threshold for wave detection [Pa/m]
    double trackingTolerance_ = 0.001;        // Wave tracking tolerance [m]
    
    // Educational content templates
    std::map<std::string, std::string> educationalTemplates_;
    void initializeEducationalContent();
};

/*---------------------------------------------------------------------------*\
                    Helper Functions for 2D Analysis
\*---------------------------------------------------------------------------*/

// Forward declaration for helper functions
class CellularDetonation2D;

// Coordinate transformation utilities
std::vector<std::pair<double, double>> transformTrajectoryToCartesian(
    const std::vector<CellularDetonation2D::Wave2DPoint>& trajectory);

std::vector<CellularDetonation2D::Wave2DPoint> transformFieldToCylindrical(
    const std::vector<std::vector<std::pair<double, double>>>& cartesianField,
    const std::vector<std::vector<double>>& pressureField,
    const std::vector<std::vector<double>>& temperatureField);

// Wave analysis utilities
double calculateWaveAngle(const CellularDetonation2D::Wave2DPoint& p1, const CellularDetonation2D::Wave2DPoint& p2);
double calculateWaveCurvature(const std::vector<CellularDetonation2D::Wave2DPoint>& trajectory, size_t index);
double calculateLocalMachNumber(const CellularDetonation2D::Wave2DPoint& point, const RDEChemistry& chemistry);

// Educational content helpers
std::string explainCylindricalWavePhysics();
std::string explainAnnularDetonationCharacteristics();
std::string explainWaveInjectionPhysics();
std::string explainMultiWavePhenomena();

} // namespace MCP
} // namespace Foam