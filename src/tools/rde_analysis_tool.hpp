#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "../openfoam/rotating_detonation_engine.hpp"
#include "context_engine.hpp"

using json = nlohmann::json;
using namespace Foam::MCP;

/**
 * Rotating Detonation Engine Analysis MCP Tool with Educational Integration
 * 
 * Advanced combustion physics analysis with intelligent educational content:
 * - Chapman-Jouguet detonation theory
 * - Wave propagation dynamics  
 * - Pressure gain combustion
 * - Multi-fuel chemistry models
 * - Performance optimization
 */
class RDEAnalysisTool {
public:
    RDEAnalysisTool();
    
    // MCP Tool Interface
    json handleRequest(const json& request);
    json getToolDefinition() const;
    
    // Core RDE Analysis
    struct RDEAnalysisRequest {
        // Geometry parameters
        double outerRadius;         // [m]
        double innerRadius;         // [m] 
        double chamberLength;       // [m]
        int numberOfInjectors;
        std::string injectionType;  // "axial", "radial", "mixed"
        
        // Chemistry parameters
        std::string fuelType;       // "hydrogen", "methane", "propane", "kerosene"
        std::string oxidizerType;   // "air", "oxygen"
        double equivalenceRatio;    // φ (phi)
        double chamberPressure;     // [Pa]
        double injectionTemperature; // [K]
        double injectionVelocity;   // [m/s]
        
        // Analysis type
        std::string analysisType;   // "design", "performance", "stability", "optimization"
        std::string caseDirectory;
        
        // Simulation settings
        std::string solverType;     // "rhoCentralFoam", "BYCFoam", "rhoReactingFoam"
        bool enableDetailedChemistry;
        bool enableVisualization;
        int meshResolution;         // 1=coarse, 2=medium, 3=fine
    };
    
    struct RDEAnalysisResult {
        bool success;
        std::string analysisType;
        
        // Operating characteristics
        double detonationVelocity;  // [m/s]
        double waveFrequency;       // [Hz]
        int numberOfWaves;
        double specificImpulse;     // [s]
        double thrust;              // [N]
        double pressureGain;        // ratio
        double combustionEfficiency;
        
        // Advanced metrics
        double massFlowRate;        // [kg/s]
        double heatReleaseRate;     // [W]
        double pressureOscillations; // RMS
        double cellSize;            // [m]
        double residenceTime;       // [s]
        
        // Educational content
        std::string physicsExplanation;
        std::string detonationTheory;
        std::string designGuidance;
        std::vector<std::string> keyInsights;
        std::vector<std::string> recommendations;
        std::string troubleshooting;
        
        // Socratic questions
        std::vector<std::string> conceptQuestions;
        std::vector<std::string> designQuestions;
        std::vector<std::string> physicsQuestions;
        
        // Visualization files
        std::string pressureContours;
        std::string temperatureContours;
        std::string velocityVectors;
        std::string waveAnimation;
        
        // Validation data
        double theoreticalAccuracy;
        std::vector<std::string> validationWarnings;
    };
    
    RDEAnalysisResult analyzeRDE(const RDEAnalysisRequest& request);
    
private:
    std::unique_ptr<RotatingDetonationEngine> rdeEngine_;
    std::unique_ptr<ContextEngine> contextEngine_;
    
    // Educational Content Generation
    struct RDEEducationalContent {
        std::string detonationPhysics;
        std::string chapmanJouguetTheory;
        std::string waveDynamics;
        std::string combustionKinetics;
        std::string performanceMetrics;
        std::string designPrinciples;
        std::vector<std::string> keyEquations;
        std::vector<std::string> physicalInsights;
    };
    
    // Advanced analysis methods
    RDEEducationalContent generateEducationalContent(const RDEAnalysisRequest& request, const RDEAnalysisResult& result);
    std::vector<std::string> generateSocraticQuestions(const std::string& category, const RDEAnalysisRequest& request);
    std::string generatePhysicsNarrative(const RDEAnalysisRequest& request);
    std::string generateDesignNarrative(const RDEAnalysisRequest& request, const RDEAnalysisResult& result);
    
    // Combustion physics education
    std::string explainDetonationPhenomena(const std::string& fuelType, double phi);
    std::string explainChapmanJouguetTheory(double cjVelocity, double cjPressure);
    std::string explainWavePropagation(int numberOfWaves, double frequency);
    std::string explainPressureGainCombustion(double pressureGain);
    std::string explainCombustionKinetics(const std::string& fuelType);
    
    // Performance analysis education
    std::string explainSpecificImpulse(double isp, const std::string& fuelType);
    std::string explainCombustionEfficiency(double efficiency, double residenceTime);
    std::string explainPressureOscillations(double oscillations, int waves);
    std::string explainHeatTransfer(double heatRate, double wallArea);
    
    // Design guidance
    std::string recommendGeometry(double outerRadius, double innerRadius, double length);
    std::string recommendInjection(const std::string& injectionType, double velocity, int nozzles);
    std::string recommendChemistry(const std::string& fuel, const std::string& oxidizer, double phi);
    std::string recommendOperatingConditions(double pressure, double temperature);
    
    // Validation and troubleshooting
    std::string validateDesignParameters(const RDEAnalysisRequest& request);
    std::string generateTroubleshootingGuide(const std::vector<std::string>& warnings);
    std::string compareWithExperimentalData(const RDEAnalysisResult& result);
    
    // Advanced combustion concepts
    std::string explainCellularDetonation(double cellSize);
    std::string explainDetonationInstabilities(const RDEAnalysisResult& result);
    std::string explainMultiWaveInteraction(int numberOfWaves);
    std::string explainInjectionCoupling(const std::string& injectionType, double velocity);
    
    // Industry context and applications
    std::string explainIndustrialApplications(const std::string& analysisType);
    std::string explainTechnologyReadiness(const std::string& fuelType);
    std::string explainDesignChallenges(const RDEAnalysisRequest& request);
    std::string explainFutureDirections();
    
    // Utility methods
    bool validateInputParameters(const RDEAnalysisRequest& request, std::vector<std::string>& warnings);
    std::string formatPerformanceResults(const RDEAnalysisResult& result);
    std::string generateParameterSummary(const RDEAnalysisRequest& request);
    double calculateReynoldsNumber(const RDEAnalysisRequest& request);
    double calculateDamkohlerNumber(const RDEAnalysisRequest& request);
    double calculateMachNumber(double velocity, double temperature);
};

// MCP Tool Registration
extern "C" {
    RDEAnalysisTool* createRDEAnalysisTool();
    void destroyRDEAnalysisTool(RDEAnalysisTool* tool);
}