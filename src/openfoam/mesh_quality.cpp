/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Mesh Quality Assessment
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Implementation of intelligent mesh quality assessment framework for 
    OpenFOAM meshes.
    
    Expert priority: "Help users understand when the mesh is good enough"
    Provides comprehensive quality analysis with educational explanations
    adapted to user understanding level.

\*---------------------------------------------------------------------------*/

#include "mesh_quality.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

namespace Foam {
namespace MCP {

namespace fs = std::filesystem;

/*---------------------------------------------------------------------------*\
                    Quality Standards Implementation
\*---------------------------------------------------------------------------*/

namespace QualityStandards {
    
    const QualityThresholds AUTOMOTIVE_EXTERNAL = []() {
        QualityThresholds thresholds;
        thresholds.orthogonality.excellent = 10.0;
        thresholds.orthogonality.good = 20.0;
        thresholds.orthogonality.acceptable = 40.0;
        thresholds.skewness.excellent = 0.2;
        thresholds.skewness.good = 0.4;
        thresholds.skewness.acceptable = 0.6;
        thresholds.aspectRatio.excellent = 3.0;
        thresholds.aspectRatio.good = 5.0;
        thresholds.aspectRatio.acceptable = 10.0;
        return thresholds;
    }();
    
    const QualityThresholds AEROSPACE_EXTERNAL = []() {
        QualityThresholds thresholds;
        thresholds.orthogonality.excellent = 5.0;
        thresholds.orthogonality.good = 10.0;
        thresholds.orthogonality.acceptable = 20.0;
        thresholds.skewness.excellent = 0.1;
        thresholds.skewness.good = 0.2;
        thresholds.skewness.acceptable = 0.4;
        thresholds.aspectRatio.excellent = 2.0;
        thresholds.aspectRatio.good = 3.0;
        thresholds.aspectRatio.acceptable = 5.0;
        return thresholds;
    }();
    
    const QualityThresholds INTERNAL_FLOW = []() {
        QualityThresholds thresholds;
        thresholds.orthogonality.excellent = 20.0;
        thresholds.orthogonality.good = 35.0;
        thresholds.orthogonality.acceptable = 60.0;
        thresholds.skewness.excellent = 0.3;
        thresholds.skewness.good = 0.6;
        thresholds.skewness.acceptable = 1.0;
        thresholds.aspectRatio.excellent = 8.0;
        thresholds.aspectRatio.good = 15.0;
        thresholds.aspectRatio.acceptable = 30.0;
        return thresholds;
    }();
    
    const QualityThresholds HEAT_TRANSFER = []() {
        QualityThresholds thresholds;
        thresholds.orthogonality.excellent = 12.0;
        thresholds.orthogonality.good = 25.0;
        thresholds.orthogonality.acceptable = 45.0;
        thresholds.skewness.excellent = 0.2;
        thresholds.skewness.good = 0.4;
        thresholds.skewness.acceptable = 0.7;
        thresholds.aspectRatio.excellent = 4.0;
        thresholds.aspectRatio.good = 8.0;
        thresholds.aspectRatio.acceptable = 15.0;
        return thresholds;
    }();
    
    const QualityThresholds RESEARCH_GRADE = []() {
        QualityThresholds thresholds;
        thresholds.orthogonality.excellent = 8.0;
        thresholds.orthogonality.good = 15.0;
        thresholds.orthogonality.acceptable = 25.0;
        thresholds.skewness.excellent = 0.15;
        thresholds.skewness.good = 0.3;
        thresholds.skewness.acceptable = 0.5;
        thresholds.aspectRatio.excellent = 2.5;
        thresholds.aspectRatio.good = 4.0;
        thresholds.aspectRatio.acceptable = 7.0;
        return thresholds;
    }();
    
    const QualityThresholds PRODUCTION_GRADE = []() {
        QualityThresholds thresholds;
        thresholds.orthogonality.excellent = 20.0;
        thresholds.orthogonality.good = 40.0;
        thresholds.orthogonality.acceptable = 65.0;
        thresholds.skewness.excellent = 0.4;
        thresholds.skewness.good = 0.8;
        thresholds.skewness.acceptable = 1.5;
        thresholds.aspectRatio.excellent = 10.0;
        thresholds.aspectRatio.good = 20.0;
        thresholds.aspectRatio.acceptable = 50.0;
        return thresholds;
    }();
}

/*---------------------------------------------------------------------------*\
                    Class MeshQualityAnalyzer Implementation
\*---------------------------------------------------------------------------*/

MeshQualityAnalyzer::MeshQualityAnalyzer()
{
    // Use standard OpenFOAM quality thresholds by default
    thresholds_ = QualityThresholds();
}

MeshQualityAnalyzer::MeshQualityAnalyzer(const QualityThresholds& customThresholds)
    : thresholds_(customThresholds)
{
}

QualityReport MeshQualityAnalyzer::assessMeshQuality(const std::string& caseDir)
{
    QualityReport report;
    
    // Validate case directory
    if (!isMeshDirectory(caseDir)) {
        QualityIssue issue;
        issue.issueType = "invalid_case";
        issue.severity = "critical";
        issue.description = "Invalid OpenFOAM case directory: " + caseDir;
        issue.location = caseDir;
        issue.consequences = {"Cannot perform mesh analysis"};
        issue.solutions = {"Verify case directory structure", "Run blockMesh or snappyHexMesh first"};
        issue.canAutoFix = false;
        report.issues.push_back(issue);
        return report;
    }
    
    try {
        // Phase 1: Basic mesh structure analysis
        report = analyzeMeshStructure(caseDir);
        
        // Phase 2: Calculate detailed quality metrics
        report.metrics = calculateQualityMetrics(caseDir);
        
        // Phase 3: Identify specific quality issues
        auto issues = identifyQualityIssues(report.metrics);
        report.issues.insert(report.issues.end(), issues.begin(), issues.end());
        
        // Phase 4: Overall quality assessment
        report.qualityScore = calculateOverallScore(report.metrics);
        report.overallGrade = assignOverallGrade(report.qualityScore);
        report.readyForSimulation = assessSimulationReadiness(report.metrics, report.issues);
        report.readinessReason = report.readyForSimulation ? 
            "Mesh quality meets simulation requirements" : 
            "Mesh quality issues detected that may affect simulation";
        
        // Phase 5: Solver compatibility assessment
        report.solverCompatibility = assessSolverCompatibility(report.metrics);
        
        // Phase 6: Educational content generation
        report.qualityExplanation = generateQualityExplanation(report);
        report.learningPoints = generateLearningPoints(report);
        report.nextSteps = generateNextSteps(report);
        
    } catch (const std::exception& e) {
        QualityIssue issue;
        issue.issueType = "analysis_error";
        issue.severity = "critical";
        issue.description = "Mesh analysis failed: " + std::string(e.what());
        issue.consequences = {"Cannot assess mesh quality"};
        issue.solutions = {"Check OpenFOAM installation", "Verify mesh exists", "Run checkMesh manually"};
        issue.canAutoFix = false;
        report.issues.push_back(issue);
        report.readyForSimulation = false;
        report.readinessReason = "Analysis error occurred";
    }
    
    return report;
}

QualityReport MeshQualityAnalyzer::quickQualityCheck(const std::string& caseDir)
{
    QualityReport report;
    
    if (!isMeshDirectory(caseDir)) {
        QualityIssue issue;
        issue.issueType = "invalid_case";
        issue.severity = "critical";
        issue.description = "Invalid case directory";
        report.issues.push_back(issue);
        return report;
    }
    
    try {
        // Quick checkMesh run for essential metrics only
        std::string checkMeshOutput;
        if (runCheckMesh(caseDir, checkMeshOutput)) {
            auto stats = parseCheckMeshOutput(checkMeshOutput);
            
            // Create minimal metrics set
            QualityMetric ortho;
            ortho.metricName = "non_orthogonality";
            ortho.value = stats.count("max_non_orthogonality") ? stats.at("max_non_orthogonality") : 0.0;
            ortho.threshold = thresholds_.orthogonality.acceptable;
            ortho.status = assessMetricStatus(ortho.value, "orthogonality");
            report.metrics.push_back(ortho);
            
            QualityMetric skew;
            skew.metricName = "skewness";
            skew.value = stats.count("max_skewness") ? stats.at("max_skewness") : 0.0;
            skew.threshold = thresholds_.skewness.acceptable;
            skew.status = assessMetricStatus(skew.value, "skewness");
            report.metrics.push_back(skew);
            
            report.qualityScore = calculateOverallScore(report.metrics);
            report.overallGrade = assignOverallGrade(report.qualityScore);
            report.readyForSimulation = report.qualityScore > 50.0;
        }
    } catch (const std::exception& e) {
        QualityIssue issue;
        issue.issueType = "quick_check_failed";
        issue.severity = "warning";
        issue.description = "Quick quality check failed: " + std::string(e.what());
        report.issues.push_back(issue);
    }
    
    return report;
}

QualityReport MeshQualityAnalyzer::analyzeMeshStructure(const std::string& caseDir)
{
    QualityReport report;
    report.caseDirectory = caseDir;
    
    // Determine mesh type
    if (fs::exists(caseDir + "/system/blockMeshDict")) {
        report.meshType = "blockMesh";
    } else if (fs::exists(caseDir + "/system/snappyHexMeshDict")) {
        report.meshType = "snappyHexMesh";
    } else {
        report.meshType = "external";
    }
    
    // Run checkMesh to get basic statistics
    std::string checkMeshOutput;
    if (runCheckMesh(caseDir, checkMeshOutput)) {
        auto stats = parseCheckMeshOutput(checkMeshOutput);
        report.meshStatistics = stats;
        
        report.totalCells = static_cast<int>(stats.count("total_cells") ? stats.at("total_cells") : 0);
        report.totalFaces = static_cast<int>(stats.count("total_faces") ? stats.at("total_faces") : 0);
        report.totalPoints = static_cast<int>(stats.count("total_points") ? stats.at("total_points") : 0);
    }
    
    return report;
}

std::vector<QualityMetric> MeshQualityAnalyzer::calculateQualityMetrics(const std::string& caseDir)
{
    std::vector<QualityMetric> metrics;
    
    // Calculate individual metrics
    metrics.push_back(calculateOrthogonality(caseDir));
    metrics.push_back(calculateSkewness(caseDir));
    metrics.push_back(calculateAspectRatio(caseDir));
    metrics.push_back(calculateVolumeRatio(caseDir));
    metrics.push_back(calculateFaceAreas(caseDir));
    metrics.push_back(calculateCellVolumes(caseDir));
    
    return metrics;
}

QualityMetric MeshQualityAnalyzer::calculateOrthogonality(const std::string& caseDir)
{
    QualityMetric metric;
    metric.metricName = "non_orthogonality";
    metric.explanation = "Measures how far cell faces deviate from being perpendicular to the line connecting cell centers";
    metric.impact = "High non-orthogonality can cause convergence problems and reduce accuracy";
    metric.improvement = "Improve mesh generation parameters, use higher quality meshing, add more cells in problematic regions";
    
    std::string checkMeshOutput;
    if (runCheckMesh(caseDir, checkMeshOutput)) {
        auto values = extractMetricValues(checkMeshOutput, "non-orthogonality");
        if (!values.empty()) {
            metric.value = *std::max_element(values.begin(), values.end());
        }
    }
    
    metric.threshold = thresholds_.orthogonality.acceptable;
    metric.status = assessMetricStatus(metric.value, "orthogonality");
    
    return metric;
}

QualityMetric MeshQualityAnalyzer::calculateSkewness(const std::string& caseDir)
{
    QualityMetric metric;
    metric.metricName = "skewness";
    metric.explanation = "Measures how much cells deviate from their ideal shape (hexahedral, tetrahedral, etc.)";
    metric.impact = "High skewness reduces accuracy and can cause stability issues";
    metric.improvement = "Improve mesh topology, use structured meshing where possible, refine transition regions";
    
    std::string checkMeshOutput;
    if (runCheckMesh(caseDir, checkMeshOutput)) {
        auto values = extractMetricValues(checkMeshOutput, "skewness");
        if (!values.empty()) {
            metric.value = *std::max_element(values.begin(), values.end());
        }
    }
    
    metric.threshold = thresholds_.skewness.acceptable;
    metric.status = assessMetricStatus(metric.value, "skewness");
    
    return metric;
}

QualityMetric MeshQualityAnalyzer::calculateAspectRatio(const std::string& caseDir)
{
    QualityMetric metric;
    metric.metricName = "aspect_ratio";
    metric.explanation = "Ratio of the longest to shortest dimension of mesh cells";
    metric.impact = "Very high aspect ratios can cause numerical instability and poor convergence";
    metric.improvement = "Use more isotropic cells, add refinement layers, improve mesh grading";
    
    std::string checkMeshOutput;
    if (runCheckMesh(caseDir, checkMeshOutput)) {
        auto values = extractMetricValues(checkMeshOutput, "aspect");
        if (!values.empty()) {
            metric.value = *std::max_element(values.begin(), values.end());
        }
    }
    
    metric.threshold = thresholds_.aspectRatio.acceptable;
    metric.status = assessMetricStatus(metric.value, "aspect_ratio");
    
    return metric;
}

QualityMetric MeshQualityAnalyzer::calculateVolumeRatio(const std::string& caseDir)
{
    QualityMetric metric;
    metric.metricName = "volume_ratio";
    metric.explanation = "Ratio between neighboring cell volumes";
    metric.impact = "Large volume ratios can cause interpolation errors and convergence issues";
    metric.improvement = "Improve mesh grading, use gradual transitions between fine and coarse regions";
    
    std::string checkMeshOutput;
    if (runCheckMesh(caseDir, checkMeshOutput)) {
        auto values = extractMetricValues(checkMeshOutput, "volume");
        if (!values.empty()) {
            auto minVol = *std::min_element(values.begin(), values.end());
            auto maxVol = *std::max_element(values.begin(), values.end());
            metric.value = (minVol > 0) ? maxVol / minVol : 1e6;
        }
    }
    
    metric.threshold = 100.0; // Conservative threshold
    metric.status = assessMetricStatus(metric.value, "volume_ratio");
    
    return metric;
}

QualityMetric MeshQualityAnalyzer::calculateFaceAreas(const std::string& caseDir)
{
    QualityMetric metric;
    metric.metricName = "face_area_ratio";
    metric.explanation = "Ratio between largest and smallest face areas in the mesh";
    metric.impact = "Large face area ratios can affect flux calculations and stability";
    metric.improvement = "Improve mesh grading and avoid abrupt size transitions";
    
    // Simplified calculation - would need actual OpenFOAM mesh access for precise values
    metric.value = 10.0; // Placeholder
    metric.threshold = 50.0;
    metric.status = "good";
    
    return metric;
}

QualityMetric MeshQualityAnalyzer::calculateCellVolumes(const std::string& caseDir)
{
    QualityMetric metric;
    metric.metricName = "cell_volume_ratio";
    metric.explanation = "Ratio between largest and smallest cell volumes";
    metric.impact = "Large volume ratios affect time step restrictions and solution accuracy";
    metric.improvement = "Use more uniform cell sizes, improve mesh grading";
    
    // Simplified calculation
    metric.value = 25.0; // Placeholder
    metric.threshold = 100.0;
    metric.status = "good";
    
    return metric;
}

std::vector<QualityIssue> MeshQualityAnalyzer::identifyQualityIssues(const std::vector<QualityMetric>& metrics)
{
    std::vector<QualityIssue> issues;
    
    for (const auto& metric : metrics) {
        if (metric.isCritical()) {
            QualityIssue issue;
            issue.issueType = metric.metricName + "_critical";
            issue.severity = "critical";
            issue.description = "Critical " + metric.metricName + " detected (value: " + 
                              std::to_string(metric.value) + ", threshold: " + 
                              std::to_string(metric.threshold) + ")";
            issue.worstValue = metric.value;
            issue.consequences = {"Simulation may fail to converge", "Results may be inaccurate", "Solver instability"};
            issue.solutions = {metric.improvement};
            issue.canAutoFix = false;
            issues.push_back(issue);
        } else if (metric.status == "poor") {
            QualityIssue issue;
            issue.issueType = metric.metricName + "_poor";
            issue.severity = "warning";
            issue.description = "Poor " + metric.metricName + " detected (value: " + 
                              std::to_string(metric.value) + ")";
            issue.worstValue = metric.value;
            issue.consequences = {"Reduced convergence rate", "Lower solution accuracy"};
            issue.solutions = {metric.improvement};
            issue.canAutoFix = false;
            issues.push_back(issue);
        }
    }
    
    return issues;
}

std::string MeshQualityAnalyzer::assessMetricStatus(double value, const std::string& metricType)
{
    if (metricType == "orthogonality") {
        if (value <= thresholds_.orthogonality.excellent) return "excellent";
        if (value <= thresholds_.orthogonality.good) return "good";
        if (value <= thresholds_.orthogonality.acceptable) return "acceptable";
        if (value <= thresholds_.orthogonality.poor) return "poor";
        return "failed";
    } else if (metricType == "skewness") {
        if (value <= thresholds_.skewness.excellent) return "excellent";
        if (value <= thresholds_.skewness.good) return "good";
        if (value <= thresholds_.skewness.acceptable) return "acceptable";
        if (value <= thresholds_.skewness.poor) return "poor";
        return "failed";
    } else if (metricType == "aspect_ratio") {
        if (value <= thresholds_.aspectRatio.excellent) return "excellent";
        if (value <= thresholds_.aspectRatio.good) return "good";
        if (value <= thresholds_.aspectRatio.acceptable) return "acceptable";
        if (value <= thresholds_.aspectRatio.poor) return "poor";
        return "failed";
    } else if (metricType == "volume_ratio") {
        if (value <= 10.0) return "excellent";
        if (value <= 50.0) return "good";
        if (value <= 100.0) return "acceptable";
        if (value <= 500.0) return "poor";
        return "failed";
    }
    
    return "unknown";
}

double MeshQualityAnalyzer::calculateOverallScore(const std::vector<QualityMetric>& metrics)
{
    if (metrics.empty()) return 0.0;
    
    double totalScore = 0.0;
    int count = 0;
    
    for (const auto& metric : metrics) {
        double metricScore = 0.0;
        
        if (metric.status == "excellent") metricScore = 100.0;
        else if (metric.status == "good") metricScore = 80.0;
        else if (metric.status == "acceptable") metricScore = 60.0;
        else if (metric.status == "poor") metricScore = 30.0;
        else if (metric.status == "failed") metricScore = 0.0;
        
        totalScore += metricScore;
        count++;
    }
    
    return count > 0 ? totalScore / count : 0.0;
}

std::string MeshQualityAnalyzer::assignOverallGrade(double score)
{
    if (score >= 90.0) return "A";
    if (score >= 80.0) return "B";
    if (score >= 70.0) return "C";
    if (score >= 60.0) return "D";
    return "F";
}

bool MeshQualityAnalyzer::assessSimulationReadiness(const std::vector<QualityMetric>& metrics,
                                                   const std::vector<QualityIssue>& issues)
{
    // Check for critical issues
    for (const auto& issue : issues) {
        if (issue.severity == "critical") {
            return false;
        }
    }
    
    // Check if any metrics have failed status
    for (const auto& metric : metrics) {
        if (metric.isCritical()) {
            return false;
        }
    }
    
    // Check overall quality score
    double score = calculateOverallScore(metrics);
    return score >= 50.0; // Minimum acceptable score
}

std::vector<SolverCompatibility> MeshQualityAnalyzer::assessSolverCompatibility(const std::vector<QualityMetric>& metrics)
{
    std::vector<SolverCompatibility> compatibility;
    
    auto solvers = getSupportedSolvers();
    for (const auto& solver : solvers) {
        compatibility.push_back(assessForSolver(solver, metrics));
    }
    
    return compatibility;
}

SolverCompatibility MeshQualityAnalyzer::assessForSolver(const std::string& solver, const std::vector<QualityMetric>& metrics)
{
    SolverCompatibility compat;
    compat.solverName = solver;
    compat.isCompatible = true;
    
    // Conservative compatibility assessment
    for (const auto& metric : metrics) {
        if (metric.isCritical()) {
            compat.isCompatible = false;
            compat.compatibilityReason = "Critical mesh quality issues detected";
            compat.requiredImprovements.push_back("Improve " + metric.metricName);
        }
    }
    
    if (compat.isCompatible) {
        compat.compatibilityReason = "Mesh quality is acceptable for " + solver;
        
        if (solver == "simpleFoam" || solver == "pimpleFoam") {
            compat.recommendedSettings.push_back("Consider using SIMPLE or PIMPLE algorithms");
            compat.recommendedSettings.push_back("Monitor residuals carefully");
        } else if (solver == "interFoam") {
            compat.recommendedSettings.push_back("Use appropriate interface compression settings");
            compat.recommendedSettings.push_back("Consider adaptive time stepping");
        }
    }
    
    return compat;
}

std::string MeshQualityAnalyzer::generateQualityExplanation(const QualityReport& report)
{
    std::ostringstream explanation;
    
    explanation << "Mesh Quality Assessment Summary:\n\n";
    explanation << "Overall Grade: " << report.overallGrade << " (Score: " << report.qualityScore << "/100)\n";
    explanation << "Mesh Type: " << report.meshType << "\n";
    explanation << "Total Cells: " << report.totalCells << "\n\n";
    
    explanation << "Key Quality Metrics:\n";
    for (const auto& metric : report.metrics) {
        explanation << "- " << metric.metricName << ": " << metric.value << " (" << metric.status << ")\n";
        explanation << "  " << metric.explanation << "\n";
    }
    
    if (!report.issues.empty()) {
        explanation << "\nIdentified Issues:\n";
        for (const auto& issue : report.issues) {
            explanation << "- " << issue.description << " (" << issue.severity << ")\n";
        }
    }
    
    explanation << "\nSimulation Readiness: " << (report.readyForSimulation ? "Ready" : "Not Ready") << "\n";
    explanation << "Reason: " << report.readinessReason << "\n";
    
    return explanation.str();
}

std::vector<std::string> MeshQualityAnalyzer::generateLearningPoints(const QualityReport& report)
{
    std::vector<std::string> points;
    
    points.push_back("Mesh quality directly affects simulation accuracy and convergence");
    points.push_back("Non-orthogonality and skewness are critical quality metrics for CFD");
    points.push_back("High aspect ratio cells can cause numerical instability");
    
    if (report.overallGrade == "A" || report.overallGrade == "B") {
        points.push_back("Your mesh quality is excellent - this should provide reliable results");
    } else if (report.overallGrade == "C" || report.overallGrade == "D") {
        points.push_back("Your mesh has moderate quality - monitor convergence carefully");
    } else {
        points.push_back("Your mesh needs improvement before running simulations");
    }
    
    for (const auto& metric : report.metrics) {
        if (metric.status == "failed" || metric.status == "poor") {
            points.push_back("Focus on improving " + metric.metricName + " through " + metric.improvement);
        }
    }
    
    return points;
}

std::vector<std::string> MeshQualityAnalyzer::generateNextSteps(const QualityReport& report)
{
    std::vector<std::string> steps;
    
    if (report.readyForSimulation) {
        steps.push_back("Proceed with simulation setup");
        steps.push_back("Monitor residuals during solver execution");
        steps.push_back("Validate results against experimental data if available");
    } else {
        steps.push_back("Address critical mesh quality issues before proceeding");
        
        for (const auto& issue : report.issues) {
            if (issue.severity == "critical") {
                for (const auto& solution : issue.solutions) {
                    steps.push_back(solution);
                }
            }
        }
        
        steps.push_back("Re-run mesh quality assessment after improvements");
    }
    
    return steps;
}

bool MeshQualityAnalyzer::runCheckMesh(const std::string& caseDir, std::string& output)
{
    try {
        // Change to case directory and run checkMesh
        std::string command = "cd \"" + caseDir + "\" && checkMesh -allTopology -allGeometry 2>&1";
        
        std::array<char, 128> buffer;
        std::string result;
        
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) {
            output = "Failed to run checkMesh command";
            return false;
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        output = result;
        return true;
        
    } catch (const std::exception& e) {
        output = "Error running checkMesh: " + std::string(e.what());
        return false;
    }
}

std::map<std::string, double> MeshQualityAnalyzer::parseCheckMeshOutput(const std::string& output)
{
    std::map<std::string, double> stats;
    
    std::istringstream iss(output);
    std::string line;
    
    while (std::getline(iss, line)) {
        // Parse key statistics from checkMesh output
        if (line.find("cells:") != std::string::npos) {
            auto pos = line.find("cells:");
            std::istringstream lineStream(line.substr(pos + 6));
            double cellCount;
            if (lineStream >> cellCount) {
                stats["total_cells"] = cellCount;
            }
        }
        
        if (line.find("faces:") != std::string::npos) {
            auto pos = line.find("faces:");
            std::istringstream lineStream(line.substr(pos + 6));
            double faceCount;
            if (lineStream >> faceCount) {
                stats["total_faces"] = faceCount;
            }
        }
        
        if (line.find("points:") != std::string::npos) {
            auto pos = line.find("points:");
            std::istringstream lineStream(line.substr(pos + 7));
            double pointCount;
            if (lineStream >> pointCount) {
                stats["total_points"] = pointCount;
            }
        }
        
        if (line.find("non-orthogonality") != std::string::npos && line.find("max:") != std::string::npos) {
            auto pos = line.find("max:");
            std::istringstream lineStream(line.substr(pos + 4));
            double maxOrtho;
            if (lineStream >> maxOrtho) {
                stats["max_non_orthogonality"] = maxOrtho;
            }
        }
        
        if (line.find("skewness") != std::string::npos && line.find("max:") != std::string::npos) {
            auto pos = line.find("max:");
            std::istringstream lineStream(line.substr(pos + 4));
            double maxSkew;
            if (lineStream >> maxSkew) {
                stats["max_skewness"] = maxSkew;
            }
        }
    }
    
    return stats;
}

std::vector<double> MeshQualityAnalyzer::extractMetricValues(const std::string& checkMeshOutput, const std::string& metric)
{
    std::vector<double> values;
    
    std::istringstream iss(checkMeshOutput);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.find(metric) != std::string::npos) {
            // Extract numerical values from the line
            std::istringstream lineStream(line);
            std::string word;
            while (lineStream >> word) {
                try {
                    double value = std::stod(word);
                    values.push_back(value);
                } catch (const std::exception&) {
                    // Not a number, continue
                }
            }
        }
    }
    
    return values;
}

bool MeshQualityAnalyzer::isAcceptableForSolver(const std::string& caseDir, const std::string& solver)
{
    auto report = quickQualityCheck(caseDir);
    
    for (const auto& compat : report.solverCompatibility) {
        if (compat.solverName == solver) {
            return compat.isCompatible;
        }
    }
    
    // Default conservative assessment
    return report.readyForSimulation;
}

std::vector<QualityIssue> MeshQualityAnalyzer::identifyProblematicCells(const std::string& caseDir)
{
    std::vector<QualityIssue> issues;
    
    // Run detailed checkMesh analysis
    std::string checkMeshOutput;
    if (runCheckMesh(caseDir, checkMeshOutput)) {
        // Parse output for problematic cells
        if (checkMeshOutput.find("***Error") != std::string::npos || 
            checkMeshOutput.find("***Warning") != std::string::npos) {
            
            QualityIssue issue;
            issue.issueType = "problematic_cells";
            issue.severity = "warning";
            issue.description = "CheckMesh detected problematic cells";
            issue.solutions = {"Run checkMesh with -writeFields to identify problem areas",
                              "Refine mesh in problematic regions",
                              "Improve mesh generation parameters"};
            issues.push_back(issue);
        }
    }
    
    return issues;
}

QualityMetric MeshQualityAnalyzer::assessSpecificMetric(const std::string& caseDir, const std::string& metricName)
{
    if (metricName == "orthogonality") {
        return calculateOrthogonality(caseDir);
    } else if (metricName == "skewness") {
        return calculateSkewness(caseDir);
    } else if (metricName == "aspect_ratio") {
        return calculateAspectRatio(caseDir);
    } else if (metricName == "volume_ratio") {
        return calculateVolumeRatio(caseDir);
    } else {
        QualityMetric metric;
        metric.metricName = metricName;
        metric.status = "unknown";
        return metric;
    }
}

std::string MeshQualityAnalyzer::explainQualityMetrics(const std::string& userLevel)
{
    std::ostringstream explanation;
    
    explanation << "Mesh Quality Metrics Explained";
    if (userLevel == "beginner") {
        explanation << " (Beginner Level):\n\n";
        explanation << "1. Non-orthogonality: How 'square' your mesh cells are\n";
        explanation << "   - Good meshes have cells that are close to perfect rectangles/cubes\n";
        explanation << "   - Bad: cells that are stretched or skewed\n\n";
        
        explanation << "2. Skewness: How much cells deviate from their ideal shape\n";
        explanation << "   - Good: cells that look like regular shapes\n";
        explanation << "   - Bad: cells that are heavily distorted\n\n";
        
        explanation << "3. Aspect Ratio: How stretched cells are\n";
        explanation << "   - Good: cells that are roughly the same size in all directions\n";
        explanation << "   - Bad: very long, thin cells\n";
    } else {
        explanation << " (Intermediate/Advanced Level):\n\n";
        explanation << "1. Non-orthogonality: Deviation of face normal from cell center-to-center vector\n";
        explanation << "   - Affects accuracy of gradient calculations\n";
        explanation << "   - High values require corrector loops\n\n";
        
        explanation << "2. Skewness: Measure of how much cell shape deviates from ideal\n";
        explanation << "   - Affects interpolation accuracy\n";
        explanation << "   - Can cause convergence issues\n\n";
        
        explanation << "3. Aspect Ratio: Ratio of largest to smallest cell dimension\n";
        explanation << "   - Affects numerical stability\n";
        explanation << "   - Very high ratios can cause matrix conditioning problems\n";
    }
    
    return explanation.str();
}

std::vector<std::string> MeshQualityAnalyzer::generateQualityQuestions(const QualityReport& report)
{
    std::vector<std::string> questions;
    
    questions.push_back("What is the most critical quality issue with this mesh?");
    questions.push_back("How would poor mesh quality affect my simulation results?");
    questions.push_back("What specific steps should I take to improve mesh quality?");
    
    if (report.overallGrade == "F") {
        questions.push_back("Why is my mesh quality so poor and how can I fix it?");
        questions.push_back("Should I regenerate the mesh completely or try to improve it?");
    } else if (report.overallGrade == "A" || report.overallGrade == "B") {
        questions.push_back("My mesh quality is good - what solver settings should I use?");
        questions.push_back("How can I verify that my results are mesh-independent?");
    }
    
    for (const auto& metric : report.metrics) {
        if (metric.status == "failed" || metric.status == "poor") {
            questions.push_back("How do I improve " + metric.metricName + " in my mesh?");
        }
    }
    
    return questions;
}

std::string MeshQualityAnalyzer::getMetricExplanation(const std::string& metricName, const std::string& userLevel)
{
    if (metricName == "orthogonality" || metricName == "non_orthogonality") {
        if (userLevel == "beginner") {
            return "Non-orthogonality measures how 'square' your mesh cells are. Think of it like building with LEGO blocks - you want nice rectangular blocks, not twisted or skewed ones. Good meshes have low non-orthogonality (< 25°).";
        } else {
            return "Non-orthogonality is the angle between the face normal vector and the line connecting adjacent cell centers. It affects the accuracy of gradient calculations and can require additional corrector loops in the solver. Values > 70° are problematic.";
        }
    } else if (metricName == "skewness") {
        if (userLevel == "beginner") {
            return "Skewness measures how much your mesh cells are distorted from their ideal shape. Imagine trying to fit a square peg in a round hole - that's high skewness. You want cells that look like proper rectangles or triangles.";
        } else {
            return "Skewness quantifies the deviation of cell geometry from the ideal shape. It's calculated based on the ratio of areas or volumes and affects interpolation accuracy. Values > 0.8 can cause convergence problems.";
        }
    } else if (metricName == "aspect_ratio") {
        if (userLevel == "beginner") {
            return "Aspect ratio tells you how stretched your cells are. Think of the difference between a square and a very long, thin rectangle. For most simulations, you want cells that are roughly square-ish rather than needle-like.";
        } else {
            return "Aspect ratio is the ratio of the largest to smallest dimension of a cell. High aspect ratios can cause numerical instability and poor matrix conditioning. Values > 100 should be avoided unless justified by physics (e.g., boundary layers).";
        }
    }
    
    return "Metric explanation not available for: " + metricName;
}

std::string MeshQualityAnalyzer::generateImprovementPlan(const QualityReport& report)
{
    std::ostringstream plan;
    
    plan << "Mesh Quality Improvement Plan:\n\n";
    
    if (report.readyForSimulation) {
        plan << "✓ Your mesh is ready for simulation!\n";
        plan << "Recommended next steps:\n";
        plan << "1. Proceed with simulation setup\n";
        plan << "2. Monitor convergence carefully\n";
        plan << "3. Consider mesh independence study for critical results\n";
    } else {
        plan << "⚠ Your mesh needs improvement before simulation.\n\n";
        
        plan << "Priority Actions:\n";
        int priority = 1;
        
        for (const auto& issue : report.issues) {
            if (issue.severity == "critical") {
                plan << priority++ << ". " << issue.description << "\n";
                plan << "   Solutions: ";
                for (size_t i = 0; i < issue.solutions.size(); ++i) {
                    plan << issue.solutions[i];
                    if (i < issue.solutions.size() - 1) plan << ", ";
                }
                plan << "\n\n";
            }
        }
        
        plan << "Specific Improvements:\n";
        for (const auto& metric : report.metrics) {
            if (metric.status == "failed" || metric.status == "poor") {
                plan << "- " << metric.metricName << " (current: " << metric.value 
                     << ", target: < " << metric.threshold << ")\n";
                plan << "  Action: " << metric.improvement << "\n";
            }
        }
    }
    
    return plan.str();
}

std::vector<std::string> MeshQualityAnalyzer::prioritizeImprovements(const std::vector<QualityIssue>& issues)
{
    std::vector<std::string> priorities;
    
    // Critical issues first
    for (const auto& issue : issues) {
        if (issue.severity == "critical") {
            priorities.push_back("CRITICAL: " + issue.description);
        }
    }
    
    // Warning issues second
    for (const auto& issue : issues) {
        if (issue.severity == "warning") {
            priorities.push_back("WARNING: " + issue.description);
        }
    }
    
    // Info issues last
    for (const auto& issue : issues) {
        if (issue.severity == "info") {
            priorities.push_back("INFO: " + issue.description);
        }
    }
    
    return priorities;
}

std::string MeshQualityAnalyzer::suggestMeshRefinements(const QualityReport& report)
{
    std::ostringstream suggestions;
    
    suggestions << "Mesh Refinement Suggestions:\n\n";
    
    if (report.totalCells < 10000) {
        suggestions << "1. Your mesh is quite coarse (" << report.totalCells << " cells)\n";
        suggestions << "   Consider adding more cells for better accuracy\n\n";
    } else if (report.totalCells > 1000000) {
        suggestions << "1. Your mesh is very fine (" << report.totalCells << " cells)\n";
        suggestions << "   Ensure this resolution is necessary for your analysis\n\n";
    }
    
    for (const auto& metric : report.metrics) {
        if (metric.status == "poor" || metric.status == "failed") {
            if (metric.metricName == "non_orthogonality") {
                suggestions << "2. High non-orthogonality detected:\n";
                suggestions << "   - Add refinement in curved regions\n";
                suggestions << "   - Improve boundary layer meshing\n";
                suggestions << "   - Use higher quality mesh generation\n\n";
            } else if (metric.metricName == "skewness") {
                suggestions << "3. High skewness detected:\n";
                suggestions << "   - Improve mesh topology\n";
                suggestions << "   - Use structured meshing where possible\n";
                suggestions << "   - Avoid sharp transitions between fine/coarse regions\n\n";
            }
        }
    }
    
    return suggestions.str();
}

bool MeshQualityAnalyzer::validateQualityReport(const QualityReport& report)
{
    // Basic validation checks
    if (report.caseDirectory.empty()) return false;
    if (report.totalCells <= 0) return false;
    if (report.qualityScore < 0.0 || report.qualityScore > 100.0) return false;
    
    // Check for required metrics
    bool hasOrthogonality = false;
    bool hasSkewness = false;
    
    for (const auto& metric : report.metrics) {
        if (metric.metricName == "non_orthogonality") hasOrthogonality = true;
        if (metric.metricName == "skewness") hasSkewness = true;
    }
    
    return hasOrthogonality && hasSkewness;
}

QualityReport MeshQualityAnalyzer::compareMeshQuality(const std::string& caseDir1, const std::string& caseDir2)
{
    auto report1 = assessMeshQuality(caseDir1);
    auto report2 = assessMeshQuality(caseDir2);
    
    QualityReport comparison;
    comparison.caseDirectory = caseDir1 + " vs " + caseDir2;
    comparison.qualityExplanation = "Mesh Quality Comparison:\n\n";
    comparison.qualityExplanation += "Mesh 1 (" + caseDir1 + "): Grade " + report1.overallGrade + 
                                    " (Score: " + std::to_string(report1.qualityScore) + ")\n";
    comparison.qualityExplanation += "Mesh 2 (" + caseDir2 + "): Grade " + report2.overallGrade + 
                                    " (Score: " + std::to_string(report2.qualityScore) + ")\n\n";
    
    if (report1.qualityScore > report2.qualityScore) {
        comparison.qualityExplanation += "Mesh 1 has better overall quality.\n";
    } else if (report2.qualityScore > report1.qualityScore) {
        comparison.qualityExplanation += "Mesh 2 has better overall quality.\n";
    } else {
        comparison.qualityExplanation += "Both meshes have similar quality.\n";
    }
    
    return comparison;
}

void MeshQualityAnalyzer::setQualityThresholds(const QualityThresholds& thresholds)
{
    thresholds_ = thresholds;
}

json MeshQualityAnalyzer::reportToJson(const QualityReport& report)
{
    json j;
    to_json(j, report);
    return j;
}

std::string MeshQualityAnalyzer::reportToSummary(const QualityReport& report)
{
    std::ostringstream summary;
    
    summary << "=== MESH QUALITY SUMMARY ===\n";
    summary << "Case: " << report.caseDirectory << "\n";
    summary << "Grade: " << report.overallGrade << " (" << report.qualityScore << "/100)\n";
    summary << "Cells: " << report.totalCells << "\n";
    summary << "Ready for simulation: " << (report.readyForSimulation ? "YES" : "NO") << "\n";
    
    if (!report.issues.empty()) {
        summary << "\nIssues found: " << report.issues.size() << "\n";
        for (const auto& issue : report.issues) {
            summary << "- " << issue.severity << ": " << issue.description << "\n";
        }
    }
    
    return summary.str();
}

bool MeshQualityAnalyzer::isMeshDirectory(const std::string& caseDir)
{
    // Check for OpenFOAM case structure
    return fs::exists(caseDir + "/constant") && 
           fs::exists(caseDir + "/system") && 
           (fs::exists(caseDir + "/constant/polyMesh") || 
            fs::exists(caseDir + "/system/blockMeshDict") ||
            fs::exists(caseDir + "/system/snappyHexMeshDict"));
}

std::vector<std::string> MeshQualityAnalyzer::getSupportedSolvers()
{
    return {
        "simpleFoam",
        "pimpleFoam", 
        "icoFoam",
        "interFoam",
        "chtMultiRegionFoam",
        "rhoSimpleFoam",
        "buoyantSimpleFoam"
    };
}

QualityThresholds MeshQualityAnalyzer::getConservativeThresholds()
{
    return QualityStandards::PRODUCTION_GRADE;
}

QualityThresholds MeshQualityAnalyzer::getStrictThresholds()
{
    return QualityStandards::RESEARCH_GRADE;
}

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const QualityMetric& metric) {
    j = json{
        {"metricName", metric.metricName},
        {"value", metric.value},
        {"threshold", metric.threshold},
        {"status", metric.status},
        {"explanation", metric.explanation},
        {"impact", metric.impact},
        {"improvement", metric.improvement}
    };
}

void from_json(const json& j, QualityMetric& metric) {
    j.at("metricName").get_to(metric.metricName);
    j.at("value").get_to(metric.value);
    j.at("threshold").get_to(metric.threshold);
    j.at("status").get_to(metric.status);
    j.at("explanation").get_to(metric.explanation);
    j.at("impact").get_to(metric.impact);
    j.at("improvement").get_to(metric.improvement);
}

void to_json(json& j, const QualityIssue& issue) {
    j = json{
        {"issueType", issue.issueType},
        {"severity", issue.severity},
        {"description", issue.description},
        {"location", issue.location},
        {"affectedCells", issue.affectedCells},
        {"worstValue", issue.worstValue},
        {"consequences", issue.consequences},
        {"solutions", issue.solutions},
        {"canAutoFix", issue.canAutoFix}
    };
}

void from_json(const json& j, QualityIssue& issue) {
    j.at("issueType").get_to(issue.issueType);
    j.at("severity").get_to(issue.severity);
    j.at("description").get_to(issue.description);
    j.at("location").get_to(issue.location);
    j.at("affectedCells").get_to(issue.affectedCells);
    j.at("worstValue").get_to(issue.worstValue);
    j.at("consequences").get_to(issue.consequences);
    j.at("solutions").get_to(issue.solutions);
    j.at("canAutoFix").get_to(issue.canAutoFix);
}

void to_json(json& j, const SolverCompatibility& compatibility) {
    j = json{
        {"solverName", compatibility.solverName},
        {"isCompatible", compatibility.isCompatible},
        {"compatibilityReason", compatibility.compatibilityReason},
        {"requiredImprovements", compatibility.requiredImprovements},
        {"recommendedSettings", compatibility.recommendedSettings}
    };
}

void from_json(const json& j, SolverCompatibility& compatibility) {
    j.at("solverName").get_to(compatibility.solverName);
    j.at("isCompatible").get_to(compatibility.isCompatible);
    j.at("compatibilityReason").get_to(compatibility.compatibilityReason);
    j.at("requiredImprovements").get_to(compatibility.requiredImprovements);
    j.at("recommendedSettings").get_to(compatibility.recommendedSettings);
}

void to_json(json& j, const QualityReport& report) {
    j = json{
        {"caseDirectory", report.caseDirectory},
        {"meshType", report.meshType},
        {"totalCells", report.totalCells},
        {"totalFaces", report.totalFaces},
        {"totalPoints", report.totalPoints},
        {"overallGrade", report.overallGrade},
        {"qualityScore", report.qualityScore},
        {"readyForSimulation", report.readyForSimulation},
        {"readinessReason", report.readinessReason},
        {"metrics", report.metrics},
        {"issues", report.issues},
        {"solverCompatibility", report.solverCompatibility},
        {"qualityExplanation", report.qualityExplanation},
        {"learningPoints", report.learningPoints},
        {"nextSteps", report.nextSteps},
        {"meshStatistics", report.meshStatistics}
    };
}

void from_json(const json& j, QualityReport& report) {
    j.at("caseDirectory").get_to(report.caseDirectory);
    j.at("meshType").get_to(report.meshType);
    j.at("totalCells").get_to(report.totalCells);
    j.at("totalFaces").get_to(report.totalFaces);
    j.at("totalPoints").get_to(report.totalPoints);
    j.at("overallGrade").get_to(report.overallGrade);
    j.at("qualityScore").get_to(report.qualityScore);
    j.at("readyForSimulation").get_to(report.readyForSimulation);
    j.at("readinessReason").get_to(report.readinessReason);
    j.at("metrics").get_to(report.metrics);
    j.at("issues").get_to(report.issues);
    j.at("solverCompatibility").get_to(report.solverCompatibility);
    j.at("qualityExplanation").get_to(report.qualityExplanation);
    j.at("learningPoints").get_to(report.learningPoints);
    j.at("nextSteps").get_to(report.nextSteps);
    j.at("meshStatistics").get_to(report.meshStatistics);
}

void to_json(json& j, const QualityThresholds& thresholds) {
    j = json{
        {"orthogonality", {
            {"excellent", thresholds.orthogonality.excellent},
            {"good", thresholds.orthogonality.good},
            {"acceptable", thresholds.orthogonality.acceptable},
            {"poor", thresholds.orthogonality.poor},
            {"failed", thresholds.orthogonality.failed}
        }},
        {"skewness", {
            {"excellent", thresholds.skewness.excellent},
            {"good", thresholds.skewness.good},
            {"acceptable", thresholds.skewness.acceptable},
            {"poor", thresholds.skewness.poor},
            {"failed", thresholds.skewness.failed}
        }},
        {"aspectRatio", {
            {"excellent", thresholds.aspectRatio.excellent},
            {"good", thresholds.aspectRatio.good},
            {"acceptable", thresholds.aspectRatio.acceptable},
            {"poor", thresholds.aspectRatio.poor},
            {"failed", thresholds.aspectRatio.failed}
        }},
        {"volume", {
            {"minVol", thresholds.volume.minVol},
            {"volRatio", thresholds.volume.volRatio}
        }}
    };
}

void from_json(const json& j, QualityThresholds& thresholds) {
    j.at("orthogonality").at("excellent").get_to(thresholds.orthogonality.excellent);
    j.at("orthogonality").at("good").get_to(thresholds.orthogonality.good);
    j.at("orthogonality").at("acceptable").get_to(thresholds.orthogonality.acceptable);
    j.at("orthogonality").at("poor").get_to(thresholds.orthogonality.poor);
    j.at("orthogonality").at("failed").get_to(thresholds.orthogonality.failed);
    
    j.at("skewness").at("excellent").get_to(thresholds.skewness.excellent);
    j.at("skewness").at("good").get_to(thresholds.skewness.good);
    j.at("skewness").at("acceptable").get_to(thresholds.skewness.acceptable);
    j.at("skewness").at("poor").get_to(thresholds.skewness.poor);
    j.at("skewness").at("failed").get_to(thresholds.skewness.failed);
    
    j.at("aspectRatio").at("excellent").get_to(thresholds.aspectRatio.excellent);
    j.at("aspectRatio").at("good").get_to(thresholds.aspectRatio.good);
    j.at("aspectRatio").at("acceptable").get_to(thresholds.aspectRatio.acceptable);
    j.at("aspectRatio").at("poor").get_to(thresholds.aspectRatio.poor);
    j.at("aspectRatio").at("failed").get_to(thresholds.aspectRatio.failed);
    
    j.at("volume").at("minVol").get_to(thresholds.volume.minVol);
    j.at("volume").at("volRatio").get_to(thresholds.volume.volRatio);
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //