#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include "case_manager.hpp"
#include "cellular_detonation_model.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Struct RDEGeometry
\*---------------------------------------------------------------------------*/

struct RDEGeometry {
    // Annular combustor dimensions
    double outerRadius;         // Outer annulus radius [m]
    double innerRadius;         // Inner annulus radius [m]
    double chamberLength;       // Axial length [m]
    double injectorWidth;       // Injection slot width [m]
    
    // Injection configuration
    std::string injectionType;  // "axial", "radial", "mixed"
    int numberOfInjectors;      // Number of discrete injectors
    double injectionAngle;      // Injection angle [degrees]
    
    // Nozzle configuration
    bool hasNozzle;
    std::string nozzleType;     // "convergent", "convergent_divergent", "aerospike"
    double nozzleThroatArea;    // Throat area [m²]
    double nozzleExitArea;      // Exit area [m²]
    
    RDEGeometry() 
        : outerRadius(0.05), innerRadius(0.03), chamberLength(0.1), 
          injectorWidth(0.002), injectionType("axial"), numberOfInjectors(12), 
          injectionAngle(90.0), hasNozzle(true), nozzleType("convergent"),
          nozzleThroatArea(0.001), nozzleExitArea(0.002) {}
};

/*---------------------------------------------------------------------------*\
                        Struct RDEChemistry
\*---------------------------------------------------------------------------*/

struct RDEChemistry {
    // Fuel and oxidizer
    std::string fuelType;       // "hydrogen", "methane", "propane", "kerosene"
    std::string oxidizerType;   // "air", "oxygen", "nitrous_oxide"
    double equivalenceRatio;    // φ (phi)
    
    // Operating conditions
    double chamberPressure;     // Total pressure [Pa]
    double injectionTemperature; // Injection temperature [K]
    double injectionVelocity;   // Injection velocity [m/s]
    
    // Detonation properties
    double detonationVelocity;  // Chapman-Jouguet velocity [m/s]
    double detonationPressure;  // C-J pressure [Pa]
    double detonationTemperature; // C-J temperature [K]
    
    // Chemistry mechanism
    std::string mechanismFile;  // Path to chemical mechanism
    std::vector<std::string> species; // Chemical species
    
    // Cellular detonation properties
    double cellSize;            // Detonation cell size λ [m]
    double inductionLength;     // Induction length ΔI [m]
    double cjMachNumber;        // Chapman-Jouguet Mach number MCJ
    double maxThermicity;       // Maximum thermicity σ̇max [1/s]
    bool useCellularModel;      // Enable ANN cellular structure model
    
    RDEChemistry()
        : fuelType("hydrogen"), oxidizerType("air"), equivalenceRatio(1.0),
          chamberPressure(101325.0), injectionTemperature(300.0), 
          injectionVelocity(100.0), detonationVelocity(2000.0),
          detonationPressure(2000000.0), detonationTemperature(3000.0),
          mechanismFile("H2_air_mechanism.dat"), cellSize(0.001),
          inductionLength(0.0001), cjMachNumber(5.0), maxThermicity(1000.0),
          useCellularModel(true) {}
};

/*---------------------------------------------------------------------------*\
                        Struct RDEOperatingPoint
\*---------------------------------------------------------------------------*/

struct RDEOperatingPoint {
    // Wave characteristics
    int numberOfWaves;          // Number of detonation waves
    double waveFrequency;       // Wave passage frequency [Hz]
    std::string waveDirection;  // "co_rotating", "counter_rotating", "mixed"
    double waveSpeed;           // Actual wave speed [m/s]
    
    // Performance metrics
    double specificImpulse;     // Isp [s]
    double thrust;              // Net thrust [N]
    double massFlowRate;        // Total mass flow rate [kg/s]
    double pressureGain;        // Pressure gain across combustor
    double combustionEfficiency; // Combustion efficiency
    
    // Instabilities and losses
    double pressureOscillations; // RMS pressure fluctuations
    double heatLossRate;        // Heat loss to walls [W]
    double incompleteyCombustion; // Incomplete combustion losses
    
    RDEOperatingPoint()
        : numberOfWaves(1), waveFrequency(5000.0), waveDirection("co_rotating"),
          waveSpeed(1800.0), specificImpulse(300.0), thrust(1000.0),
          massFlowRate(0.1), pressureGain(3.0), combustionEfficiency(0.95),
          pressureOscillations(0.1), heatLossRate(5000.0), incompleteyCombustion(0.05) {}
};

/*---------------------------------------------------------------------------*\
                        Struct RDESimulationSettings
\*---------------------------------------------------------------------------*/

struct RDESimulationSettings {
    // Solver selection
    std::string solverType;     // "rhoCentralFoam", "detonationFoam", "BYCFoam", "rhoReactingFoam"
    std::string turbulenceModel; // "laminar", "kEpsilon", "kOmegaSST", "LES"
    std::string combustionModel; // "finite_rate", "edm", "flamelet", "detailed_kinetics"
    
    // Mesh parameters
    int radialCells;            // Cells in radial direction
    int circumferentialCells;   // Cells in circumferential direction
    int axialCells;             // Cells in axial direction
    double cellSize;            // Characteristic cell size [m]
    bool adaptiveMeshRefinement; // Enable AMR
    
    // Cellular detonation settings
    bool enableCellularTracking; // Track cellular structure evolution
    double cellularResolution;   // Δx/λ ratio for cellular constraint
    bool autoMeshSizing;         // Automatically size mesh based on cell size
    std::string detonationSolverType; // "Euler", "NS_Sutherland", "NS_mixtureAverage"
    
    // Time stepping
    double timeStep;            // Initial time step [s]
    double maxCourantNumber;    // Maximum Courant number
    double simulationTime;      // Total simulation time [s]
    int writeInterval;          // Write interval
    
    // Numerical schemes
    std::string fluxScheme;     // "Kurganov", "HLLC", "AUSM", "central"
    std::string timeScheme;     // "Euler", "CrankNicolson", "backward"
    std::string gradScheme;     // "Gauss", "leastSquares"
    
    RDESimulationSettings()
        : solverType("detonationFoam"), turbulenceModel("laminar"), 
          combustionModel("finite_rate"), radialCells(50), circumferentialCells(200),
          axialCells(100), cellSize(0.001), adaptiveMeshRefinement(true),
          enableCellularTracking(true), cellularResolution(0.1), autoMeshSizing(true),
          detonationSolverType("NS_Sutherland"), timeStep(1e-7), maxCourantNumber(0.5), 
          simulationTime(0.005), writeInterval(100), fluxScheme("HLLC"), 
          timeScheme("Euler"), gradScheme("Gauss") {}
};

/*---------------------------------------------------------------------------*\
                        Struct RDEValidationData
\*---------------------------------------------------------------------------*/

struct RDEValidationData {
    // Experimental or analytical comparison data
    std::vector<double> experimentalPressure;
    std::vector<double> experimentalTemperature;
    std::vector<double> experimentalVelocity;
    std::vector<double> timePoints;
    
    // Theoretical predictions
    double theoreticalDetonationVelocity;
    double theoreticalPressureRatio;
    double theoreticalSpecificImpulse;
    
    // Validation metrics
    double pressureAccuracy;    // RMSE for pressure prediction
    double velocityAccuracy;    // RMSE for velocity prediction
    double waveSpeedAccuracy;   // Accuracy of wave speed prediction
    
    RDEValidationData()
        : theoreticalDetonationVelocity(2000.0), theoreticalPressureRatio(20.0),
          theoreticalSpecificImpulse(300.0), pressureAccuracy(0.0),
          velocityAccuracy(0.0), waveSpeedAccuracy(0.0) {}
};

/*---------------------------------------------------------------------------*\
                        Class RotatingDetonationEngine
\*---------------------------------------------------------------------------*/

class RotatingDetonationEngine {
public:
    RotatingDetonationEngine();
    ~RotatingDetonationEngine() = default;
    
    // Main analysis interface
    struct RDEAnalysisRequest {
        RDEGeometry geometry;
        RDEChemistry chemistry;
        RDESimulationSettings settings;
        std::string analysisType;   // "design", "performance", "stability", "optimization"
        std::string caseDirectory;
        bool enableVisualization;
    };
    
    struct RDEAnalysisResult {
        bool success;
        std::string analysisType;
        RDEOperatingPoint operatingPoint;
        RDEValidationData validation;
        
        // Detailed results
        std::map<std::string, double> performanceMetrics;
        std::vector<std::string> warnings;
        std::vector<std::string> recommendations;
        
        // Visualization data
        std::string pressureFieldFile;
        std::string temperatureFieldFile;
        std::string velocityFieldFile;
        std::string waveTrackingFile;
        
        // Educational content
        std::string physicsExplanation;
        std::string designGuidance;
        std::vector<std::string> learningPoints;
    };
    
    // Core functionality
    RDEAnalysisResult analyzeRDE(const RDEAnalysisRequest& request);
    
    // Specialized analysis methods
    RDEOperatingPoint calculateOperatingPoint(const RDEGeometry& geometry, const RDEChemistry& chemistry);
    RDEChemistry calculateDetonationProperties(const std::string& fuelType, const std::string& oxidizerType, double phi);
    RDEGeometry optimizeGeometry(const RDEChemistry& chemistry, const std::string& objective);
    
    // Validation and verification
    RDEValidationData validateAgainstExperiment(const RDEAnalysisResult& result, const std::string& experimentFile);
    bool verifyDetonationStability(const RDEOperatingPoint& op);
    
    // Educational content generation
    std::string generatePhysicsExplanation(const RDEAnalysisRequest& request);
    std::string generateDesignGuidance(const RDEGeometry& geometry, const RDEChemistry& chemistry);
    std::vector<std::string> generateLearningPoints(const std::string& analysisType);
    
private:
    std::unique_ptr<CaseManager> caseManager_;
    
    // RDE-specific calculations
    double calculateChapmanJouguetVelocity(const RDEChemistry& chemistry);
    double calculateChapmanJouguetPressure(const RDEChemistry& chemistry);
    double calculateChapmanJouguetTemperature(const RDEChemistry& chemistry);
    
    // Geometry generation
    bool generateRDEMesh(const std::string& caseDir, const RDEGeometry& geometry, const RDESimulationSettings& settings, const RDEChemistry& chemistry);
    bool setupBoundaryConditions(const std::string& caseDir, const RDEChemistry& chemistry);
    bool setupInitialConditions(const std::string& caseDir, const RDEChemistry& chemistry);
    
    // Solver configuration
    bool configureSolver(const std::string& caseDir, const RDESimulationSettings& settings);
    bool configureDetonationFoamSolver(const std::string& caseDir, const RDESimulationSettings& settings);
    bool configureRhoCentralFoamSolver(const std::string& caseDir, const RDESimulationSettings& settings);
    bool configureBYCFoamSolver(const std::string& caseDir, const RDESimulationSettings& settings);
    bool setupChemistryModel(const std::string& caseDir, const RDEChemistry& chemistry);
    bool setupTurbulenceModel(const std::string& caseDir, const RDESimulationSettings& settings);
    
    // Post-processing
    bool extractPerformanceMetrics(const std::string& caseDir, RDEOperatingPoint& op);
    bool trackDetonationWaves(const std::string& caseDir, const std::string& outputFile);
    bool generateVisualization(const std::string& caseDir, const RDEAnalysisResult& result);
    
    // Validation utilities
    bool compareWithAnalytical(const RDEAnalysisResult& result, RDEValidationData& validation);
    bool validateMeshQuality(const std::string& caseDir);
    bool checkConvergence(const std::string& caseDir);
    
    // Educational utilities
    std::string explainDetonationPhysics(const RDEChemistry& chemistry);
    std::string explainWaveDynamics(const RDEOperatingPoint& op);
    std::string explainPerformanceMetrics(const RDEOperatingPoint& op);
    std::string generateTroubleshootingGuide(const std::vector<std::string>& warnings);
    
private:
    std::unique_ptr<CaseManager> caseManager_;
    std::unique_ptr<CellularDetonationModel> cellularModel_;
};

} // namespace MCP
} // namespace Foam