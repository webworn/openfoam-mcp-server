#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "../openfoam/stl_analyzer.hpp"
#include "../openfoam/snappy_templates.hpp"
#include "../openfoam/mesh_quality.hpp"
#include "context_engine.hpp"

using json = nlohmann::json;
using namespace Foam::MCP;

/**
 * SnappyHexMesh MCP Tool with Educational Integration
 * 
 * Expert-validated approach: "Geometry intelligence first, conservative automation second"
 * Handles 90% of snappyHexMesh failures through STL quality analysis and proven parameter templates
 */
class SnappyMeshTool {
public:
    SnappyMeshTool();
    
    // MCP Tool Interface
    json handleRequest(const json& request);
    json getToolDefinition() const;
    
    // Core Mesh Generation Workflow
    struct MeshGenerationRequest {
        std::string stlFile;
        std::string caseDirectory;
        std::string meshType;  // "conservative_external", "conservative_internal", "custom"
        double characteristicLength;
        int maxCells;
        bool enableLayers;
        std::vector<std::string> refinementRegions;
    };
    
    struct MeshGenerationResult {
        bool success;
        std::string meshDirectory;
        STLQualityReport stlReport;
        QualityReport meshQuality;
        std::string educationalExplanation;
        std::vector<std::string> warnings;
        std::vector<std::string> recommendations;
        std::string nextSteps;
    };
    
    MeshGenerationResult generateMesh(const MeshGenerationRequest& request);
    
private:
    std::unique_ptr<STLAnalyzer> stlAnalyzer_;
    std::unique_ptr<SnappyTemplateLibrary> templateLibrary_;
    std::unique_ptr<MeshQualityAnalyzer> qualityAnalyzer_;
    std::unique_ptr<ContextEngine> contextEngine_;
    
    // Educational Integration
    struct EducationalContent {
        std::string conceptExplanation;
        std::string physicsBackground;
        std::string practicalGuidance;
        std::vector<std::string> socraticQuestions;
        std::vector<std::string> academicReferences;
    };
    
    // Workflow Steps with Educational Feedback
    bool validateSTLGeometry(const std::string& stlFile, STLQualityReport& report);
    SnappyHexMeshDict selectMeshTemplate(const MeshGenerationRequest& request, const STLQualityReport& stlReport);
    bool generateSnappyHexMeshDict(const std::string& caseDir, const SnappyHexMeshDict& dict);
    bool executeMeshGeneration(const std::string& caseDir);
    QualityReport assessGeneratedMesh(const std::string& caseDir);
    
    // Educational Content Generation
    EducationalContent generateEducationalContent(const MeshGenerationRequest& request, const MeshGenerationResult& result);
    std::vector<std::string> generateSocraticQuestions(const std::string& meshType, const STLQualityReport& stlReport);
    std::string explainMeshingStrategy(const SnappyHexMeshDict& dict, const STLQualityReport& stlReport);
    std::string generateTroubleshootingGuidance(const MeshGenerationResult& result);
    
    // Parameter Intelligence
    double estimateCharacteristicLength(const STLQualityReport& stlReport);
    int calculateRecommendedMaxCells(const STLQualityReport& stlReport, const std::string& meshType);
    std::vector<std::string> identifyRefinementRegions(const STLQualityReport& stlReport);
    
    // Validation and Safety
    bool validateCaseDirectory(const std::string& caseDir);
    bool checkOpenFOAMEnvironment();
    std::string sanitizeInput(const std::string& input);
    
    // Expert Guidance System
    struct ExpertGuidance {
        std::string recommendation;
        std::string reasoning;
        std::string academicReference;
        int confidenceLevel;  // 1-10
    };
    
    ExpertGuidance getExpertGuidance(const std::string& issue, const MeshGenerationRequest& request);
    std::string formatExpertRecommendation(const ExpertGuidance& guidance);
    
    // Progress Tracking and User Feedback
    void reportProgress(const std::string& stage, const std::string& message);
    void logUserInteraction(const MeshGenerationRequest& request, const MeshGenerationResult& result);
};

// MCP Tool Registration
extern "C" {
    SnappyMeshTool* createSnappyMeshTool();
    void destroySnappyMeshTool(SnappyMeshTool* tool);
}