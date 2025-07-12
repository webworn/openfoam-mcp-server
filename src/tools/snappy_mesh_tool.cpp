#include "snappy_mesh_tool.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <memory>
#include <algorithm>

namespace fs = std::filesystem;
using namespace Foam::MCP;

SnappyMeshTool::SnappyMeshTool() 
    : stlAnalyzer_(std::make_unique<STLAnalyzer>())
    , templateLibrary_(std::make_unique<SnappyTemplateLibrary>())
    , qualityAnalyzer_(std::make_unique<MeshQualityAnalyzer>())
    , contextEngine_(std::make_unique<ContextEngine>()) {
}

json SnappyMeshTool::getToolDefinition() const {
    return json{
        {"name", "snappy_mesh"},
        {"description", "Expert-guided snappyHexMesh generation with educational integration. Analyzes STL geometry, applies conservative parameter templates, and provides comprehensive mesh quality assessment with learning guidance."},
        {"inputSchema", {
            {"type", "object"},
            {"properties", {
                {"stl_file", {
                    {"type", "string"},
                    {"description", "Path to STL geometry file for mesh generation"}
                }},
                {"case_directory", {
                    {"type", "string"},
                    {"description", "OpenFOAM case directory path"}
                }},
                {"mesh_type", {
                    {"type", "string"},
                    {"enum", {"conservative_external", "conservative_internal", "custom"}},
                    {"description", "Mesh generation strategy type"}
                }},
                {"characteristic_length", {
                    {"type", "number"},
                    {"description", "Characteristic length scale for mesh sizing (optional - will be estimated if not provided)"}
                }},
                {"max_cells", {
                    {"type", "integer"},
                    {"description", "Maximum number of cells (optional - will be calculated if not provided)"}
                }},
                {"enable_layers", {
                    {"type", "boolean"},
                    {"description", "Enable boundary layer generation (default: false)"}
                }},
                {"refinement_regions", {
                    {"type", "array"},
                    {"items", {"type", "string"}},
                    {"description", "Additional refinement region names (optional)"}
                }}
            }},
            {"required", json::array({"stl_file", "case_directory", "mesh_type"})}
        }}
    };
}

json SnappyMeshTool::handleRequest(const json& request) {
    try {
        // Extract parameters from request
        MeshGenerationRequest meshRequest;
        meshRequest.stlFile = request["stl_file"];
        meshRequest.caseDirectory = request["case_directory"];
        meshRequest.meshType = request["mesh_type"];
        meshRequest.characteristicLength = request.value("characteristic_length", 0.0);
        meshRequest.maxCells = request.value("max_cells", 0);
        meshRequest.enableLayers = request.value("enable_layers", false);
        
        if (request.contains("refinement_regions")) {
            meshRequest.refinementRegions = request["refinement_regions"];
        }
        
        // Generate mesh with educational integration
        auto result = generateMesh(meshRequest);
        
        // Format comprehensive response
        json response = {
            {"success", result.success},
            {"mesh_directory", result.meshDirectory},
            {"educational_explanation", result.educationalExplanation},
            {"warnings", result.warnings},
            {"recommendations", result.recommendations},
            {"next_steps", result.nextSteps}
        };
        
        if (result.success) {
            response["stl_analysis"] = {
                {"total_triangles", result.stlReport.numberOfTriangles},
                {"surface_area", result.stlReport.surfaceArea},
                {"volume", result.stlReport.volume},
                {"complexity_level", result.stlReport.complexity.level},
                {"snappy_readiness", result.stlReport.readyForSnappy}
            };
            
            response["mesh_quality"] = {
                {"overall_score", result.meshQuality.qualityScore},
                {"total_cells", result.meshQuality.totalCells},
                {"overall_grade", result.meshQuality.overallGrade},
                {"ready_for_simulation", result.meshQuality.readyForSimulation}
            };
        }
        
        return response;
        
    } catch (const std::exception& e) {
        return json{
            {"success", false},
            {"error", std::string("SnappyMesh tool error: ") + e.what()},
            {"educational_guidance", "This error suggests an issue with input parameters or file access. Let's systematically check: 1) Is the STL file valid and accessible? 2) Does the case directory exist? 3) Are OpenFOAM environment variables set correctly?"}
        };
    }
}

SnappyMeshTool::MeshGenerationResult SnappyMeshTool::generateMesh(const MeshGenerationRequest& request) {
    MeshGenerationResult result;
    result.success = false;
    
    reportProgress("initialization", "Starting expert-guided mesh generation workflow...");
    
    // Phase 1: Validate environment and inputs
    if (!checkOpenFOAMEnvironment()) {
        result.warnings.push_back("OpenFOAM environment not properly configured");
        result.educationalExplanation = "OpenFOAM requires proper environment setup. Run 'source /opt/openfoam12/etc/bashrc' first.";
        return result;
    }
    
    if (!validateCaseDirectory(request.caseDirectory)) {
        result.warnings.push_back("Invalid or inaccessible case directory");
        result.educationalExplanation = "OpenFOAM cases require a specific directory structure with system/, constant/, and 0/ subdirectories.";
        return result;
    }
    
    // Phase 2: STL Geometry Analysis (90% of failures start here)
    reportProgress("stl_analysis", "Analyzing STL geometry quality and snappyHexMesh readiness...");
    
    if (!validateSTLGeometry(request.stlFile, result.stlReport)) {
        result.educationalExplanation = "STL geometry validation failed. " + 
            generateTroubleshootingGuidance(result);
        return result;
    }
    
    // Phase 3: Intelligent Parameter Selection
    reportProgress("parameter_selection", "Selecting conservative meshing parameters based on geometry analysis...");
    
    auto meshDict = selectMeshTemplate(request, result.stlReport);
    
    // Phase 4: Generate snappyHexMeshDict
    reportProgress("dict_generation", "Generating snappyHexMeshDict with expert-validated parameters...");
    
    if (!generateSnappyHexMeshDict(request.caseDirectory, meshDict)) {
        result.warnings.push_back("Failed to generate snappyHexMeshDict");
        result.educationalExplanation = "Dictionary generation failed. This typically indicates file permission issues or invalid case directory structure.";
        return result;
    }
    
    // Phase 5: Execute Mesh Generation
    reportProgress("mesh_execution", "Executing snappyHexMesh with progress monitoring...");
    
    if (!executeMeshGeneration(request.caseDirectory)) {
        result.warnings.push_back("Mesh generation failed during execution");
        result.educationalExplanation = generateTroubleshootingGuidance(result);
        return result;
    }
    
    // Phase 6: Quality Assessment
    reportProgress("quality_assessment", "Assessing mesh quality and solver compatibility...");
    
    result.meshQuality = assessGeneratedMesh(request.caseDirectory);
    result.meshDirectory = request.caseDirectory + "/constant/polyMesh";
    
    // Phase 7: Educational Content Generation
    auto educationalContent = generateEducationalContent(request, result);
    result.educationalExplanation = educationalContent.conceptExplanation;
    result.recommendations = {educationalContent.practicalGuidance};
    result.nextSteps = "Mesh generated successfully! Next: 1) Review quality metrics, 2) Run checkMesh for detailed analysis, 3) Proceed with boundary conditions setup.";
    
    result.success = true;
    logUserInteraction(request, result);
    
    return result;
}

bool SnappyMeshTool::validateSTLGeometry(const std::string& stlFile, STLQualityReport& report) {
    if (!fs::exists(stlFile)) {
        return false;
    }
    
    report = stlAnalyzer_->analyzeSTL(stlFile);
    return report.readyForSnappy;
}

SnappyHexMeshDict SnappyMeshTool::selectMeshTemplate(const MeshGenerationRequest& request, const STLQualityReport& stlReport) {
    SnappyHexMeshDict dict;
    
    if (request.meshType == "conservative_external") {
        dict = templateLibrary_->getExternalFlowTemplate(stlReport.complexity);
    } else if (request.meshType == "conservative_internal") {
        dict = templateLibrary_->getInternalFlowTemplate(stlReport.complexity);
    } else {
        dict = templateLibrary_->getConservativeDefaults(); // Default fallback
    }
    
    // Intelligent parameter adjustment based on geometry analysis
    if (request.characteristicLength > 0) {
        dict.castellatedMeshControls.maxLocalCells = static_cast<int>(1000000.0 / request.characteristicLength);
    } else {
        // Auto-estimate based on STL analysis
        double estimatedLength = estimateCharacteristicLength(stlReport);
        dict.castellatedMeshControls.maxLocalCells = static_cast<int>(1000000.0 / estimatedLength);
    }
    
    if (request.maxCells > 0) {
        dict.castellatedMeshControls.maxGlobalCells = request.maxCells;
    } else {
        dict.castellatedMeshControls.maxGlobalCells = calculateRecommendedMaxCells(stlReport, request.meshType);
    }
    
    return dict;
}

bool SnappyMeshTool::generateSnappyHexMeshDict(const std::string& caseDir, const SnappyHexMeshDict& dict) {
    std::string dictPath = caseDir + "/system/snappyHexMeshDict";
    
    std::ofstream file(dictPath);
    if (!file.is_open()) {
        return false;
    }
    
    // Generate OpenFOAM dictionary format
    file << "/*--------------------------------*- C++ -*----------------------------------*\\\n";
    file << "| =========                 |                                                 |\n";
    file << "| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n";
    file << "|  \\\\    /   O peration     | Version:  12                                    |\n";
    file << "|   \\\\  /    A nd           | Web:      www.OpenFOAM.org                      |\n";
    file << "|    \\\\/     M anipulation  |                                                 |\n";
    file << "\\*---------------------------------------------------------------------------*/\n";
    file << "FoamFile\n{\n";
    file << "    version     2.0;\n";
    file << "    format      ascii;\n";
    file << "    class       dictionary;\n";
    file << "    object      snappyHexMeshDict;\n";
    file << "}\n\n";
    
    file << "castellatedMesh " << (dict.castellatedMesh ? "true" : "false") << ";\n";
    file << "snap            " << (dict.snap ? "true" : "false") << ";\n";
    file << "addLayers       " << (dict.addLayers ? "true" : "false") << ";\n\n";
    
    // Geometry section
    file << "geometry\n{\n";
    file << "    // STL geometry will be added here\n";
    file << "};\n\n";
    
    // Castellated mesh controls
    file << "castellatedMeshControls\n{\n";
    file << "    maxLocalCells " << dict.castellatedMeshControls.maxLocalCells << ";\n";
    file << "    maxGlobalCells " << dict.castellatedMeshControls.maxGlobalCells << ";\n";
    file << "    minRefinementCells " << dict.castellatedMeshControls.minRefinementCells << ";\n";
    file << "    nCellsBetweenLevels " << dict.castellatedMeshControls.nCellsBetweenLevels << ";\n";
    file << "    // Additional controls...\n";
    file << "};\n\n";
    
    // Snap controls
    file << "snapControls\n{\n";
    file << "    nSmoothPatch " << dict.snapControls.nSmoothPatch << ";\n";
    file << "    tolerance " << dict.snapControls.tolerance << ";\n";
    file << "    nSolveIter " << dict.snapControls.nSolveIter << ";\n";
    file << "    nRelaxIter " << dict.snapControls.nRelaxIter << ";\n";
    file << "};\n\n";
    
    // Add layers controls
    file << "addLayersControls\n{\n";
    file << "    relativeSizes " << (dict.addLayersControls.relativeSizes ? "true" : "false") << ";\n";
    file << "    expansionRatio " << dict.addLayersControls.expansionRatio << ";\n";
    file << "    finalLayerThickness " << dict.addLayersControls.finalLayerThickness << ";\n";
    file << "    minThickness " << dict.addLayersControls.minThickness << ";\n";
    file << "};\n\n";
    
    file << "meshQualityControls\n{\n";
    file << "    maxNonOrtho 65;\n";
    file << "    maxBoundarySkewness 20;\n";
    file << "    maxInternalSkewness 4;\n";
    file << "    maxConcave 80;\n";
    file << "    minFlatness 0.5;\n";
    file << "}\n";
    
    file.close();
    return true;
}

bool SnappyMeshTool::executeMeshGeneration(const std::string& caseDir) {
    // Change to case directory and execute snappyHexMesh
    std::string command = "cd " + caseDir + " && snappyHexMesh -overwrite > snappyHexMesh.log 2>&1";
    
    int result = std::system(command.c_str());
    return result == 0;
}

QualityReport SnappyMeshTool::assessGeneratedMesh(const std::string& caseDir) {
    return qualityAnalyzer_->assessMeshQuality(caseDir);
}

SnappyMeshTool::EducationalContent SnappyMeshTool::generateEducationalContent(
    const MeshGenerationRequest& request, 
    const MeshGenerationResult& result) {
    
    EducationalContent content;
    
    auto meshDict = selectMeshTemplate(request, result.stlReport);
    content.conceptExplanation = explainMeshingStrategy(meshDict, result.stlReport);
    
    content.physicsBackground = "snappyHexMesh uses a cut-cell approach with hexahedral mesh topology. "
        "This provides excellent numerical properties for CFD while maintaining geometric flexibility. "
        "The three-phase process (castellated mesh → snapping → layer addition) ensures robust mesh generation.";
    
    content.practicalGuidance = "Your mesh quality score: " + std::to_string(result.meshQuality.qualityScore) + "/10. "
        "For CFD accuracy: >7 excellent, 5-7 acceptable, <5 requires refinement.";
    
    content.socraticQuestions = generateSocraticQuestions(request.meshType, result.stlReport);
    
    content.academicReferences = {
        "Jasak, H. (1996). Error analysis and estimation for finite volume method",
        "OpenFOAM User Guide: snappyHexMesh",
        "Mesh quality guidelines from NASA CFD best practices"
    };
    
    return content;
}

std::vector<std::string> SnappyMeshTool::generateSocraticQuestions(
    const std::string& meshType, 
    const STLQualityReport& stlReport) {
    
    std::vector<std::string> questions;
    
    // CLARIFY pattern
    questions.push_back("What specific flow features are you trying to capture with this mesh density?");
    
    // EXPLORE pattern  
    questions.push_back("How would increasing the mesh resolution affect your simulation accuracy versus computational cost?");
    
    // CONFIRM pattern
    questions.push_back("So you're expecting the complexity level '" + stlReport.complexity.level + "' to be sufficient for your CFD analysis?");
    
    // APPLY pattern
    questions.push_back("Given your mesh statistics, which OpenFOAM solver would be most appropriate for your intended physics?");
    
    return questions;
}

std::string SnappyMeshTool::explainMeshingStrategy(const SnappyHexMeshDict& dict, const STLQualityReport& stlReport) {
    std::ostringstream explanation;
    
    explanation << "Expert meshing strategy applied:\n\n";
    explanation << "1. Conservative parameter selection based on " << stlReport.numberOfTriangles << " STL triangles\n";
    explanation << "2. Target mesh size: " << dict.castellatedMeshControls.maxGlobalCells << " cells\n";
    explanation << "3. Snap tolerance: " << dict.snapControls.tolerance << " (conservative for reliability)\n";
    explanation << "4. Quality controls: maxNonOrtho=65°, maxSkewness=4.0 (industry standards)\n\n";
    explanation << "This approach prioritizes mesh robustness over aggressive optimization, ";
    explanation << "following the expert principle: 'A slightly coarser mesh that converges is better than a fine mesh that fails.'";
    
    return explanation.str();
}

std::string SnappyMeshTool::generateTroubleshootingGuidance(const MeshGenerationResult& result) {
    if (!result.stlReport.readyForSnappy) {
        return "STL geometry issues detected. Common problems: non-manifold edges, intersecting surfaces, or poor triangle quality. "
               "Use STL repair tools like MeshLab or netfabb before proceeding.";
    }
    
    return "Mesh generation workflow guidance: 1) Check OpenFOAM log files, 2) Verify geometry preprocessing, "
           "3) Review parameter conservativeness, 4) Consider simplified geometry for initial testing.";
}

double SnappyMeshTool::estimateCharacteristicLength(const STLQualityReport& stlReport) {
    // Simple estimation based on geometry bounds and complexity
    double boundingBoxDiagonal = std::sqrt(stlReport.surfaceArea);
    double complexityFactor = (stlReport.complexity.level == "simple") ? 5.0 : 
                             (stlReport.complexity.level == "moderate") ? 10.0 : 
                             (stlReport.complexity.level == "complex") ? 15.0 : 20.0;
    return boundingBoxDiagonal / complexityFactor;
}

int SnappyMeshTool::calculateRecommendedMaxCells(const STLQualityReport& stlReport, const std::string& meshType) {
    int baseCells = 100000; // Conservative starting point
    
    double complexityMultiplier = (stlReport.complexity.level == "simple") ? 1.0 : 
                                 (stlReport.complexity.level == "moderate") ? 2.0 : 
                                 (stlReport.complexity.level == "complex") ? 3.0 : 4.0;
    
    if (meshType == "conservative_external") {
        baseCells = static_cast<int>(complexityMultiplier * 50000);
    } else if (meshType == "conservative_internal") {
        baseCells = static_cast<int>(complexityMultiplier * 75000);
    }
    
    return std::min(baseCells, 2000000); // Cap at 2M cells for safety
}

std::vector<std::string> SnappyMeshTool::identifyRefinementRegions(const STLQualityReport& stlReport) {
    std::vector<std::string> regions;
    
    // Basic heuristics for common refinement needs
    if (stlReport.complexity.level == "complex" || stlReport.complexity.level == "very_complex") {
        regions.push_back("high_curvature_regions");
    }
    
    regions.push_back("near_wall_region");
    
    return regions;
}

bool SnappyMeshTool::validateCaseDirectory(const std::string& caseDir) {
    return fs::exists(caseDir) && 
           fs::exists(caseDir + "/system") && 
           fs::exists(caseDir + "/constant");
}

bool SnappyMeshTool::checkOpenFOAMEnvironment() {
    return std::getenv("FOAM_VERSION") != nullptr || 
           std::getenv("WM_PROJECT_VERSION") != nullptr;
}

std::string SnappyMeshTool::sanitizeInput(const std::string& input) {
    std::string sanitized = input;
    // Remove potentially dangerous characters
    sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(), 
        [](char c) { return c == ';' || c == '&' || c == '|'; }), sanitized.end());
    return sanitized;
}

SnappyMeshTool::ExpertGuidance SnappyMeshTool::getExpertGuidance(
    const std::string& issue, 
    const MeshGenerationRequest& request) {
    
    ExpertGuidance guidance;
    guidance.confidenceLevel = 8;
    
    if (issue == "mesh_quality_low") {
        guidance.recommendation = "Increase mesh resolution in critical regions, check STL quality, use more conservative snap settings";
        guidance.reasoning = "Low quality typically results from aggressive parameters or poor geometry preprocessing";
        guidance.academicReference = "Jasak, H. (1996). Error analysis and estimation for finite volume method";
    } else {
        guidance.recommendation = "Follow systematic debugging: geometry → parameters → execution → quality";
        guidance.reasoning = "Expert workflow prioritizes root cause identification over parameter tweaking";
        guidance.academicReference = "OpenFOAM Best Practices Guide";
    }
    
    return guidance;
}

std::string SnappyMeshTool::formatExpertRecommendation(const ExpertGuidance& guidance) {
    std::ostringstream formatted;
    formatted << "Expert Recommendation (Confidence: " << guidance.confidenceLevel << "/10):\n";
    formatted << guidance.recommendation << "\n\n";
    formatted << "Reasoning: " << guidance.reasoning << "\n";
    formatted << "Reference: " << guidance.academicReference;
    return formatted.str();
}

void SnappyMeshTool::reportProgress(const std::string& stage, const std::string& message) {
    std::cout << "[" << stage << "] " << message << std::endl;
}

void SnappyMeshTool::logUserInteraction(const MeshGenerationRequest& request, const MeshGenerationResult& result) {
    // Log for continuous improvement of AI components
    // Implementation would store interaction patterns for learning
}

// C interface for MCP integration
extern "C" {
    SnappyMeshTool* createSnappyMeshTool() {
        return new SnappyMeshTool();
    }
    
    void destroySnappyMeshTool(SnappyMeshTool* tool) {
        delete tool;
    }
}