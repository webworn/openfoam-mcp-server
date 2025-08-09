#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include "rotating_detonation_engine.hpp"
#include "cellular_detonation_model.hpp"
#include "../physics/cantera_wrapper.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Class RDEExpertWorkflow
\*---------------------------------------------------------------------------*/

class RDEExpertWorkflow {
public:
    
    // Constructor
    RDEExpertWorkflow();
    
    // Destructor  
    ~RDEExpertWorkflow() = default;
    
    // Expert analysis request structure
    struct ExpertAnalysisRequest {
        // Design intent
        std::string analysisGoal;        // "design_optimization", "performance_analysis", "feasibility_study", "troubleshooting"
        std::string applicationDomain;   // "propulsion", "power_generation", "industrial_heating", "research"
        double targetThrust;             // [N] or 0 for non-propulsion applications
        double targetPower;              // [W] or 0 for non-power applications
        std::string performancePriority; // "efficiency", "thrust_density", "cost", "reliability"
        
        // Operating constraints
        double maxPressure;              // Maximum allowable pressure [Pa]
        double maxTemperature;           // Maximum allowable temperature [K]
        double maxMassFlowRate;          // Maximum mass flow rate [kg/s]
        std::vector<std::string> availableFuels; // Available fuel types
        std::string oxidizerConstraint;  // "air_only", "oxygen_available", "any"
        
        // Design constraints  
        double maxOuterDiameter;         // Maximum outer diameter [m]
        double lengthConstraint;         // Maximum axial length [m]
        std::string materialConstraint;  // "standard_steel", "inconel", "ceramic_lined"
        bool coolingRequired;            // Active cooling system required
        
        // Experience level and guidance preferences
        std::string userExperience;      // "beginner", "intermediate", "expert"
        bool requestDetailedExplanations; // Provide educational content
        bool requestDesignRationale;     // Explain design decisions
        std::string preferredUnits;      // "SI", "imperial", "mixed"
    };
    
    // Expert analysis result structure
    struct ExpertAnalysisResult {
        bool feasibilityAssessment;
        std::string feasibilityReason;
        double confidenceScore;          // Overall analysis confidence [0-1]
        
        // Optimized design
        RDEGeometry recommendedGeometry;
        RDEChemistry recommendedChemistry; 
        RDESimulationSettings recommendedSettings;
        RDEOperatingPoint predictedPerformance;
        
        // Design rationale and expert guidance
        std::string designRationale;
        std::vector<std::string> keyDesignDecisions;
        std::vector<std::string> performanceTradeoffs;
        std::string operatingRecommendations;
        
        // Risk assessment and warnings
        std::vector<std::string> technicalRisks;
        std::vector<std::string> safetyConsiderations;
        std::vector<std::string> manufacturingChallenges;
        std::string riskMitigation;
        
        // Educational content (if requested)
        std::string physicsExplanation;
        std::string designPhilosophy;
        std::vector<std::string> literatureReferences;
        std::map<std::string, std::string> glossaryTerms;
        
        // Next steps and implementation guidance
        std::vector<std::string> recommendedNextSteps;
        std::string simulationStrategy;
        std::string experimentalValidationPlan;
        std::string scalingConsiderations;
        
        // Validation and uncertainty quantification
        std::map<std::string, double> parameterUncertainties;
        std::string validationSources;
        double overallValidationConfidence;
        std::vector<std::string> validationLimitations;
    };
    
    // Main expert analysis interface
    ExpertAnalysisResult performExpertAnalysis(const ExpertAnalysisRequest& request);
    
    // Specialized workflow methods
    ExpertAnalysisResult designOptimizationWorkflow(const ExpertAnalysisRequest& request);
    ExpertAnalysisResult performanceAnalysisWorkflow(const ExpertAnalysisRequest& request); 
    ExpertAnalysisResult feasibilityStudyWorkflow(const ExpertAnalysisRequest& request);
    ExpertAnalysisResult troubleshootingWorkflow(const ExpertAnalysisRequest& request);
    
    // Expert guidance generation
    std::string generateDesignPhilosophy(const ExpertAnalysisRequest& request);
    std::vector<std::string> generateKeyInsights(const ExpertAnalysisRequest& request, const ExpertAnalysisResult& result);
    std::string generateOperatingGuidance(const RDEOperatingPoint& operatingPoint);
    std::vector<std::string> generateSafetyGuidelines(const RDEChemistry& chemistry, const RDEGeometry& geometry);
    
    // Multi-objective optimization
    struct OptimizationObjectives {
        double thrustWeight;
        double efficiencyWeight; 
        double reliabilityWeight;
        double costWeight;
        double massWeight;
    };
    
    RDEGeometry optimizeGeometry(const RDEChemistry& chemistry, const OptimizationObjectives& objectives);
    RDEChemistry optimizeChemistry(const ExpertAnalysisRequest& request);
    
    // Validation and uncertainty analysis
    struct ValidationReport {
        std::map<std::string, double> parameterConfidences;
        std::vector<std::string> validationSources;
        std::map<std::string, std::string> uncertaintyAnalysis;
        double overallReliability;
        std::vector<std::string> recommendedValidationTests;
    };
    
    ValidationReport generateValidationReport(const ExpertAnalysisResult& result);
    
    // Educational content generation (adaptive to user experience)
    struct EducationalContent {
        std::string conceptualOverview;
        std::string physicsExplanation;
        std::vector<std::string> keyEquations;
        std::vector<std::string> designPrinciples;
        std::vector<std::string> commonMistakes;
        std::vector<std::string> literatureReferences;
        std::map<std::string, std::string> glossary;
    };
    
    EducationalContent generateEducationalContent(const ExpertAnalysisRequest& request, const ExpertAnalysisResult& result);
    
    // Interactive guidance and Q&A
    struct InteractiveQuestion {
        std::string questionText;
        std::vector<std::string> options;
        std::string rationale;
        std::string recommendedAnswer;
    };
    
    std::vector<InteractiveQuestion> generateGuidedQuestions(const ExpertAnalysisRequest& request);
    std::string provideExpertAnswer(const std::string& userQuestion, const ExpertAnalysisResult& context);
    
private:
    std::unique_ptr<RotatingDetonationEngine> rdeEngine_;
    std::unique_ptr<CellularDetonationModel> cellularModel_;
    std::unique_ptr<Foam::MCP::Physics::CanteraWrapper> canteraWrapper_;
    
    // Expert knowledge databases
    std::map<std::string, std::vector<std::string>> designRuleDatabase_;
    std::map<std::string, std::vector<std::string>> troubleshootingDatabase_;
    std::map<std::string, EducationalContent> educationalDatabase_;
    std::map<std::string, std::vector<std::string>> literatureDatabase_;
    
    // Analysis helper methods
    bool assessFeasibility(const ExpertAnalysisRequest& request);
    std::string determineFeasibilityReason(const ExpertAnalysisRequest& request, bool feasible);
    double calculateConfidenceScore(const ExpertAnalysisRequest& request, const ExpertAnalysisResult& result);
    
    // Design optimization helpers
    std::vector<std::pair<std::string, double>> generateDesignAlternatives(const ExpertAnalysisRequest& request);
    RDEGeometry optimizeForThrust(const RDEChemistry& chemistry, double targetThrust);
    RDEGeometry optimizeForEfficiency(const RDEChemistry& chemistry);
    RDEGeometry optimizeForReliability(const RDEChemistry& chemistry);
    
    // Risk assessment
    std::vector<std::string> assessTechnicalRisks(const ExpertAnalysisRequest& request, const ExpertAnalysisResult& result);
    std::vector<std::string> assessSafetyRisks(const RDEChemistry& chemistry, const RDEGeometry& geometry);
    std::vector<std::string> assessManufacturingRisks(const RDEGeometry& geometry);
    
    // Educational content adaption
    EducationalContent adaptContentToExperience(const EducationalContent& baseContent, const std::string& experienceLevel);
    std::vector<std::string> selectRelevantReferences(const std::vector<std::string>& allReferences, const std::string& experienceLevel);
    
    // Validation and uncertainty helpers
    double assessParameterUncertainty(const std::string& parameter, const RDEChemistry& chemistry);
    std::vector<std::string> identifyValidationSources(const ExpertAnalysisRequest& request);
    double calculateOverallValidationConfidence(const std::map<std::string, double>& parameterConfidences);
    
    // Database initialization
    void initializeDesignRuleDatabase();
    void initializeTroubleshootingDatabase();
    void initializeEducationalDatabase();
    void initializeLiteratureDatabase();
    
    // Expert knowledge constants
    static constexpr double MIN_FEASIBILITY_CONFIDENCE = 0.6;
    static constexpr double HIGH_CONFIDENCE_THRESHOLD = 0.8;
    static constexpr double VALIDATION_CONFIDENCE_THRESHOLD = 0.7;
    static constexpr double SAFETY_MARGIN_FACTOR = 1.5;
};

} // namespace MCP
} // namespace Foam