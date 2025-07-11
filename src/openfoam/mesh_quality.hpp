/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Mesh Quality Assessment
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Intelligent mesh quality assessment framework for OpenFOAM meshes.
    
    Expert priority: "Help users understand when the mesh is good enough"
    Provides comprehensive quality analysis with educational explanations
    adapted to user understanding level.

\*---------------------------------------------------------------------------*/

#ifndef mesh_quality_H
#define mesh_quality_H

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Struct QualityMetric
\*---------------------------------------------------------------------------*/

struct QualityMetric {
    std::string metricName;
    double value;
    double threshold;          // Acceptable limit
    std::string status;        // "excellent", "good", "acceptable", "poor", "failed"
    std::string explanation;   // What this metric means
    std::string impact;        // How poor values affect simulation
    std::string improvement;   // How to improve this metric
    
    QualityMetric() : value(0.0), threshold(0.0), status("unknown") {}
    
    bool isPassing() const {
        return status == "excellent" || status == "good" || status == "acceptable";
    }
    
    bool isCritical() const {
        return status == "failed";
    }
};

/*---------------------------------------------------------------------------*\
                        Struct QualityIssue
\*---------------------------------------------------------------------------*/

struct QualityIssue {
    std::string issueType;     // "non_orthogonality", "skewness", "aspect_ratio", etc.
    std::string severity;      // "critical", "warning", "info"
    std::string description;   // Detailed description
    std::string location;      // Where in mesh (if localized)
    int affectedCells;         // Number of cells affected
    double worstValue;         // Worst metric value
    std::vector<std::string> consequences; // What could go wrong
    std::vector<std::string> solutions;    // How to fix
    bool canAutoFix;          // Whether automatic fixing is possible
    
    QualityIssue() : affectedCells(0), worstValue(0.0), canAutoFix(false) {}
};

/*---------------------------------------------------------------------------*\
                        Struct SolverCompatibility
\*---------------------------------------------------------------------------*/

struct SolverCompatibility {
    std::string solverName;
    bool isCompatible;
    std::string compatibilityReason;
    std::vector<std::string> requiredImprovements;
    std::vector<std::string> recommendedSettings;
    
    SolverCompatibility() : isCompatible(false) {}
};

/*---------------------------------------------------------------------------*\
                        Struct QualityReport
\*---------------------------------------------------------------------------*/

struct QualityReport {
    // Basic mesh information
    std::string caseDirectory;
    std::string meshType;      // "blockMesh", "snappyHexMesh", "external"
    int totalCells;
    int totalFaces;
    int totalPoints;
    
    // Overall quality assessment
    std::string overallGrade;  // "A", "B", "C", "D", "F"
    double qualityScore;       // 0-100 scale
    bool readyForSimulation;
    std::string readinessReason;
    
    // Detailed metrics
    std::vector<QualityMetric> metrics;
    std::vector<QualityIssue> issues;
    
    // Solver compatibility
    std::vector<SolverCompatibility> solverCompatibility;
    
    // Educational content
    std::string qualityExplanation;
    std::vector<std::string> learningPoints;
    std::vector<std::string> nextSteps;
    
    // Mesh statistics
    std::map<std::string, double> meshStatistics;
    
    QualityReport() 
        : totalCells(0), totalFaces(0), totalPoints(0),
          overallGrade("F"), qualityScore(0.0), readyForSimulation(false) {}
};

/*---------------------------------------------------------------------------*\
                        Struct QualityThresholds
\*---------------------------------------------------------------------------*/

struct QualityThresholds {
    // Standard OpenFOAM quality thresholds
    struct Orthogonality {
        double excellent = 15.0;    // < 15 degrees
        double good = 25.0;         // < 25 degrees  
        double acceptable = 50.0;   // < 50 degrees
        double poor = 70.0;         // < 70 degrees
        double failed = 90.0;       // >= 90 degrees
    } orthogonality;
    
    struct Skewness {
        double excellent = 0.25;    // < 0.25
        double good = 0.5;          // < 0.5
        double acceptable = 0.8;    // < 0.8
        double poor = 2.0;          // < 2.0
        double failed = 4.0;        // >= 4.0
    } skewness;
    
    struct AspectRatio {
        double excellent = 5.0;     // < 5
        double good = 10.0;         // < 10
        double acceptable = 20.0;   // < 20
        double poor = 50.0;         // < 50
        double failed = 100.0;      // >= 100
    } aspectRatio;
    
    struct Volume {
        double minVol = 1e-13;      // Minimum cell volume
        double volRatio = 0.01;     // Minimum volume ratio
    } volume;
    
    QualityThresholds() {}
};

/*---------------------------------------------------------------------------*\
                        Class MeshQualityAnalyzer Declaration
\*---------------------------------------------------------------------------*/

class MeshQualityAnalyzer {
private:
    QualityThresholds thresholds_;
    
    // Core analysis methods
    QualityReport analyzeMeshStructure(const std::string& caseDir);
    std::vector<QualityMetric> calculateQualityMetrics(const std::string& caseDir);
    std::vector<QualityIssue> identifyQualityIssues(const std::vector<QualityMetric>& metrics);
    
    // Individual metric calculations
    QualityMetric calculateOrthogonality(const std::string& caseDir);
    QualityMetric calculateSkewness(const std::string& caseDir);
    QualityMetric calculateAspectRatio(const std::string& caseDir);
    QualityMetric calculateVolumeRatio(const std::string& caseDir);
    QualityMetric calculateFaceAreas(const std::string& caseDir);
    QualityMetric calculateCellVolumes(const std::string& caseDir);
    
    // Quality assessment
    std::string assessMetricStatus(double value, const std::string& metricType);
    double calculateOverallScore(const std::vector<QualityMetric>& metrics);
    std::string assignOverallGrade(double score);
    bool assessSimulationReadiness(const std::vector<QualityMetric>& metrics,
                                 const std::vector<QualityIssue>& issues);
    
    // Solver compatibility assessment
    std::vector<SolverCompatibility> assessSolverCompatibility(const std::vector<QualityMetric>& metrics);
    SolverCompatibility assessForSolver(const std::string& solver, const std::vector<QualityMetric>& metrics);
    
    // Educational content generation
    std::string generateQualityExplanation(const QualityReport& report);
    std::vector<std::string> generateLearningPoints(const QualityReport& report);
    std::vector<std::string> generateNextSteps(const QualityReport& report);
    std::string explainMetricSignificance(const QualityMetric& metric);
    
    // Issue resolution guidance
    std::vector<std::string> generateImprovementStrategies(const QualityIssue& issue);
    std::string generateTroubleshootingGuide(const std::vector<QualityIssue>& issues);
    
    // Utility methods
    bool runCheckMesh(const std::string& caseDir, std::string& output);
    std::map<std::string, double> parseCheckMeshOutput(const std::string& output);
    std::vector<double> extractMetricValues(const std::string& checkMeshOutput, const std::string& metric);

public:
    MeshQualityAnalyzer();
    explicit MeshQualityAnalyzer(const QualityThresholds& customThresholds);
    ~MeshQualityAnalyzer() = default;
    
    // Main analysis interface
    QualityReport assessMeshQuality(const std::string& caseDir);
    QualityReport quickQualityCheck(const std::string& caseDir);
    
    // Targeted assessments
    bool isAcceptableForSolver(const std::string& caseDir, const std::string& solver);
    std::vector<QualityIssue> identifyProblematicCells(const std::string& caseDir);
    QualityMetric assessSpecificMetric(const std::string& caseDir, const std::string& metricName);
    
    // Educational interface
    std::string explainQualityMetrics(const std::string& userLevel = "intermediate");
    std::vector<std::string> generateQualityQuestions(const QualityReport& report);
    std::string getMetricExplanation(const std::string& metricName, const std::string& userLevel = "intermediate");
    
    // Improvement guidance
    std::string generateImprovementPlan(const QualityReport& report);
    std::vector<std::string> prioritizeImprovements(const std::vector<QualityIssue>& issues);
    std::string suggestMeshRefinements(const QualityReport& report);
    
    // Validation and comparison
    bool validateQualityReport(const QualityReport& report);
    QualityReport compareMeshQuality(const std::string& caseDir1, const std::string& caseDir2);
    
    // Configuration and thresholds
    void setQualityThresholds(const QualityThresholds& thresholds);
    QualityThresholds getQualityThresholds() const { return thresholds_; }
    
    // Utility methods
    json reportToJson(const QualityReport& report);
    std::string reportToSummary(const QualityReport& report);
    
    // Static utility methods
    static bool isMeshDirectory(const std::string& caseDir);
    static std::vector<std::string> getSupportedSolvers();
    static QualityThresholds getConservativeThresholds();
    static QualityThresholds getStrictThresholds();
};

/*---------------------------------------------------------------------------*\
                        Quality Standards Reference
\*---------------------------------------------------------------------------*/

namespace QualityStandards {
    // Industry standard quality requirements for different applications
    extern const QualityThresholds AUTOMOTIVE_EXTERNAL;
    extern const QualityThresholds AEROSPACE_EXTERNAL;
    extern const QualityThresholds INTERNAL_FLOW;
    extern const QualityThresholds HEAT_TRANSFER;
    extern const QualityThresholds RESEARCH_GRADE;
    extern const QualityThresholds PRODUCTION_GRADE;
}

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const QualityMetric& metric);
void from_json(const json& j, QualityMetric& metric);

void to_json(json& j, const QualityIssue& issue);
void from_json(const json& j, QualityIssue& issue);

void to_json(json& j, const SolverCompatibility& compatibility);
void from_json(const json& j, SolverCompatibility& compatibility);

void to_json(json& j, const QualityReport& report);
void from_json(const json& j, QualityReport& report);

void to_json(json& j, const QualityThresholds& thresholds);
void from_json(const json& j, QualityThresholds& thresholds);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //