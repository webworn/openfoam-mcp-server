/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | SnappyHexMesh Templates
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Implementation of conservative snappyHexMesh parameter templates.
    
    Based on expert-validated parameter sets from successful OpenFOAM
    tutorial cases including motorBike, Ahmed body, and other proven
    configurations.

\*---------------------------------------------------------------------------*/

#include "snappy_templates.hpp"
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Expert Parameter Sets (Static Data)
\*---------------------------------------------------------------------------*/

namespace ExpertParameterSets {
    
    // Based on OpenFOAM motorBike tutorial - proven for external flow
    const SnappyHexMeshDict MOTORBIKE_PARAMETERS = []() {
        SnappyHexMeshDict dict;
        dict.castellatedMesh = true;
        dict.snap = true;
        dict.addLayers = true;
        
        // Conservative castellated controls
        dict.castellatedMeshControls.maxLocalCells = 100000;
        dict.castellatedMeshControls.maxGlobalCells = 2000000;
        dict.castellatedMeshControls.minRefinementCells = 10;
        dict.castellatedMeshControls.nCellsBetweenLevels = 3;
        dict.castellatedMeshControls.maxLoadUnbalance = 0.10;
        
        // Conservative snap controls
        dict.snapControls.nSmoothPatch = 3;
        dict.snapControls.tolerance = 2.0;
        dict.snapControls.nSolveIter = 30;
        dict.snapControls.nRelaxIter = 5;
        
        // Conservative layer addition
        dict.addLayersControls.relativeSizes = true;
        dict.addLayersControls.expansionRatio = 1.2;
        dict.addLayersControls.finalLayerThickness = 0.3;
        dict.addLayersControls.minThickness = 0.1;
        
        return dict;
    }();
    
    // Conservative external flow parameters
    const SnappyHexMeshDict CONSERVATIVE_EXTERNAL = []() {
        SnappyHexMeshDict dict;
        dict.castellatedMesh = true;
        dict.snap = true;
        dict.addLayers = false; // Disabled for reliability
        
        // Very conservative settings
        dict.castellatedMeshControls.maxLocalCells = 50000;
        dict.castellatedMeshControls.maxGlobalCells = 1000000;
        dict.castellatedMeshControls.minRefinementCells = 0;
        dict.castellatedMeshControls.nCellsBetweenLevels = 2;
        
        // Safe snap settings
        dict.snapControls.nSmoothPatch = 5;
        dict.snapControls.tolerance = 4.0;
        dict.snapControls.nSolveIter = 50;
        
        return dict;
    }();
    
    // Minimal reliable parameters for when all else fails
    const SnappyHexMeshDict MINIMAL_RELIABLE = []() {
        SnappyHexMeshDict dict;
        dict.castellatedMesh = true;
        dict.snap = false; // Disabled for maximum reliability
        dict.addLayers = false; // Disabled for maximum reliability
        
        // Minimal settings
        dict.castellatedMeshControls.maxLocalCells = 10000;
        dict.castellatedMeshControls.maxGlobalCells = 200000;
        dict.castellatedMeshControls.minRefinementCells = 0;
        dict.castellatedMeshControls.nCellsBetweenLevels = 1;
        
        return dict;
    }();
}

/*---------------------------------------------------------------------------*\
                    Class SnappyTemplateLibrary Implementation
\*---------------------------------------------------------------------------*/

SnappyTemplateLibrary::SnappyTemplateLibrary()
{
    initializeValidatedStrategies();
}

void SnappyTemplateLibrary::initializeValidatedStrategies()
{
    addExternalFlowStrategies();
    addInternalFlowStrategies(); 
    addGeneralPurposeStrategies();
}

void SnappyTemplateLibrary::addExternalFlowStrategies()
{
    // Conservative external flow strategy
    MeshingStrategy strategy;
    strategy.strategyName = "conservative_external";
    strategy.description = "Conservative external flow meshing based on motorBike tutorial";
    strategy.applicableTo = "external_flow";
    strategy.parameters = ExpertParameterSets::CONSERVATIVE_EXTERNAL;
    strategy.expectedIssues = {"May require manual refinement tuning for complex geometries"};
    strategy.qualityExpectations = {"Good mesh quality for simple to moderate external flows"};
    strategy.troubleshootingGuide = "If meshing fails: 1) Check STL quality, 2) Reduce refinement levels, 3) Disable layers";
    
    strategies_["conservative_external"] = strategy;
    
    // High-quality external flow strategy
    strategy.strategyName = "high_quality_external";
    strategy.description = "High-quality external flow meshing with boundary layers";
    strategy.applicableTo = "external_flow";
    strategy.parameters = ExpertParameterSets::MOTORBIKE_PARAMETERS;
    strategy.expectedIssues = {"Layer addition may fail near sharp features", "Higher computational cost"};
    strategy.qualityExpectations = {"Excellent mesh quality suitable for detailed flow analysis"};
    strategy.troubleshootingGuide = "If layers fail: 1) Reduce layer thickness, 2) Increase nRelaxIter, 3) Check feature angles";
    
    strategies_["high_quality_external"] = strategy;
}

void SnappyTemplateLibrary::addInternalFlowStrategies()
{
    MeshingStrategy strategy;
    strategy.strategyName = "conservative_internal";
    strategy.description = "Conservative internal flow meshing for pipes and ducts";
    strategy.applicableTo = "internal_flow";
    
    // Create internal flow template
    strategy.parameters = createBasicInternalFlowTemplate(GeometryComplexity());
    strategy.expectedIssues = {"May need manual inlet/outlet refinement"};
    strategy.qualityExpectations = {"Good mesh quality for internal flow analysis"};
    strategy.troubleshootingGuide = "For internal flows: 1) Ensure watertight geometry, 2) Refine inlet/outlet, 3) Check wall spacing";
    
    strategies_["conservative_internal"] = strategy;
}

void SnappyTemplateLibrary::addGeneralPurposeStrategies()
{
    MeshingStrategy strategy;
    strategy.strategyName = "minimal_reliable";
    strategy.description = "Minimal reliable meshing for maximum compatibility";
    strategy.applicableTo = "general";
    strategy.parameters = ExpertParameterSets::MINIMAL_RELIABLE;
    strategy.expectedIssues = {"Coarse mesh may miss flow features"};
    strategy.qualityExpectations = {"Basic mesh quality suitable for initial analysis"};
    strategy.troubleshootingGuide = "Last resort option when other strategies fail";
    
    strategies_["minimal_reliable"] = strategy;
}

SnappyHexMeshDict SnappyTemplateLibrary::getOptimalTemplate(const STLQualityReport& geometryReport,
                                                          const std::string& flowType,
                                                          const std::string& accuracyLevel)
{
    // Select strategy based on geometry and requirements
    std::string strategyName = recommendStrategy(geometryReport, flowType);
    
    if (strategies_.find(strategyName) == strategies_.end()) {
        strategyName = "minimal_reliable"; // Fallback
    }
    
    SnappyHexMeshDict baseDict = strategies_[strategyName].parameters;
    
    // Adjust for specific geometry characteristics
    return adjustForGeometry(baseDict, geometryReport);
}

SnappyHexMeshDict SnappyTemplateLibrary::getExternalFlowTemplate(const GeometryComplexity& complexity)
{
    if (complexity.level == "simple" || complexity.level == "moderate") {
        return strategies_["conservative_external"].parameters;
    } else {
        return strategies_["high_quality_external"].parameters;
    }
}

SnappyHexMeshDict SnappyTemplateLibrary::getInternalFlowTemplate(const GeometryComplexity& complexity)
{
    return createBasicInternalFlowTemplate(complexity);
}

SnappyHexMeshDict SnappyTemplateLibrary::getConservativeDefaults()
{
    return ExpertParameterSets::CONSERVATIVE_EXTERNAL;
}

SnappyHexMeshDict SnappyTemplateLibrary::createBasicExternalFlowTemplate(const GeometryComplexity& complexity)
{
    SnappyHexMeshDict dict = ExpertParameterSets::CONSERVATIVE_EXTERNAL;
    
    // Adjust based on complexity
    if (complexity.level == "complex" || complexity.level == "very_complex") {
        dict.castellatedMeshControls.maxGlobalCells = 5000000;
        dict.castellatedMeshControls.nCellsBetweenLevels = 3;
    }
    
    return dict;
}

SnappyHexMeshDict SnappyTemplateLibrary::createBasicInternalFlowTemplate(const GeometryComplexity& complexity)
{
    SnappyHexMeshDict dict;
    dict.castellatedMesh = true;
    dict.snap = true;
    dict.addLayers = false; // Often not needed for internal flows
    
    // Conservative settings for internal flows
    dict.castellatedMeshControls.maxLocalCells = 100000;
    dict.castellatedMeshControls.maxGlobalCells = 2000000;
    dict.castellatedMeshControls.minRefinementCells = 10;
    dict.castellatedMeshControls.nCellsBetweenLevels = 2;
    
    // Conservative snap for internal geometries
    dict.snapControls.nSmoothPatch = 3;
    dict.snapControls.tolerance = 2.0;
    dict.snapControls.nSolveIter = 30;
    
    return dict;
}

SnappyHexMeshDict SnappyTemplateLibrary::createConservativeTemplate()
{
    return ExpertParameterSets::MINIMAL_RELIABLE;
}

std::pair<int, int> SnappyTemplateLibrary::calculateSurfaceRefinement(const GeometryComplexity& complexity, 
                                                                    const std::string& flowType)
{
    // Conservative refinement levels based on complexity
    if (complexity.level == "simple") {
        return {1, 2}; // min, max refinement
    } else if (complexity.level == "moderate") {
        return {1, 3};
    } else if (complexity.level == "complex") {
        return {2, 4};
    } else { // very_complex
        return {2, 5};
    }
}

int SnappyTemplateLibrary::calculateMaxGlobalCells(const GeometryComplexity& complexity)
{
    if (complexity.level == "simple") {
        return 500000;
    } else if (complexity.level == "moderate") {
        return 1000000;
    } else if (complexity.level == "complex") {
        return 2000000;
    } else { // very_complex
        return 5000000;
    }
}

double SnappyTemplateLibrary::calculateBackgroundMeshSize(const std::vector<double>& boundingBox)
{
    if (boundingBox.size() < 6) return 1.0;
    
    double dx = boundingBox[3] - boundingBox[0];
    double dy = boundingBox[4] - boundingBox[1];
    double dz = boundingBox[5] - boundingBox[2];
    
    double characteristicLength = std::cbrt(dx * dy * dz);
    
    // Conservative background mesh size (about 1/10 of characteristic length)
    return characteristicLength / 10.0;
}

std::string SnappyTemplateLibrary::recommendStrategy(const STLQualityReport& geometryReport,
                                                   const std::string& flowType) const
{
    // Check for critical issues first
    for (const auto& issue : geometryReport.issues) {
        if (issue.severity == "critical") {
            return "minimal_reliable";
        }
    }
    
    // Check geometry readiness
    if (!geometryReport.readyForSnappy) {
        return "minimal_reliable";
    }
    
    // Select based on flow type and complexity
    if (flowType == "external" || flowType == "external_flow") {
        if (geometryReport.complexity.level == "simple" || geometryReport.complexity.level == "moderate") {
            return "conservative_external";
        } else {
            return "high_quality_external";
        }
    } else if (flowType == "internal" || flowType == "internal_flow") {
        return "conservative_internal";
    } else {
        return "conservative_external"; // Default to external flow
    }
}

SnappyHexMeshDict SnappyTemplateLibrary::adjustForGeometry(const SnappyHexMeshDict& baseDict,
                                                         const STLQualityReport& geometryReport) const
{
    SnappyHexMeshDict adjustedDict = baseDict;
    
    // Adjust cell counts based on geometry size
    double geometrySize = geometryReport.surfaceArea;
    if (geometrySize > 100.0) { // Large geometry
        adjustedDict.castellatedMeshControls.maxGlobalCells *= 2;
    } else if (geometrySize < 1.0) { // Small geometry
        adjustedDict.castellatedMeshControls.maxGlobalCells /= 2;
    }
    
    // Adjust for aspect ratio
    if (geometryReport.complexity.aspectRatio > 10.0) {
        adjustedDict.castellatedMeshControls.nCellsBetweenLevels = 2; // More conservative
        adjustedDict.snapControls.tolerance = 4.0; // More tolerant
    }
    
    // Disable layers if geometry has issues
    bool hasGeometryIssues = false;
    for (const auto& issue : geometryReport.issues) {
        if (issue.severity == "warning" || issue.severity == "critical") {
            hasGeometryIssues = true;
            break;
        }
    }
    
    if (hasGeometryIssues) {
        adjustedDict.addLayers = false; // Disable layers for problematic geometries
    }
    
    return adjustedDict;
}

bool SnappyTemplateLibrary::validateParameterCombination(const SnappyHexMeshDict& dict) const
{
    // Basic validation checks
    if (dict.castellatedMeshControls.maxGlobalCells <= 0) return false;
    if (dict.castellatedMeshControls.maxLocalCells <= 0) return false;
    if (dict.castellatedMeshControls.nCellsBetweenLevels < 1) return false;
    
    if (dict.snap) {
        if (dict.snapControls.tolerance <= 0) return false;
        if (dict.snapControls.nSolveIter <= 0) return false;
    }
    
    if (dict.addLayers) {
        if (dict.addLayersControls.expansionRatio <= 1.0) return false;
        if (dict.addLayersControls.finalLayerThickness <= 0) return false;
    }
    
    return true;
}

std::vector<std::string> SnappyTemplateLibrary::getAvailableStrategies() const
{
    std::vector<std::string> strategyNames;
    for (const auto& strategy : strategies_) {
        strategyNames.push_back(strategy.first);
    }
    return strategyNames;
}

MeshingStrategy SnappyTemplateLibrary::getStrategy(const std::string& strategyName) const
{
    auto it = strategies_.find(strategyName);
    if (it != strategies_.end()) {
        return it->second;
    }
    return strategies_.at("minimal_reliable"); // Fallback
}

std::string SnappyTemplateLibrary::explainParameterChoices(const SnappyHexMeshDict& dict) const
{
    std::ostringstream explanation;
    
    explanation << "SnappyHexMesh Parameter Explanation:\n\n";
    
    explanation << "Castellated Mesh Settings:\n";
    explanation << "- maxGlobalCells: " << dict.castellatedMeshControls.maxGlobalCells 
                << " (limits total mesh size for computational efficiency)\n";
    explanation << "- nCellsBetweenLevels: " << dict.castellatedMeshControls.nCellsBetweenLevels
                << " (controls transition smoothness between refinement levels)\n";
    
    if (dict.snap) {
        explanation << "\nSnap Settings:\n";
        explanation << "- tolerance: " << dict.snapControls.tolerance
                    << " (higher values = more tolerant snapping, safer for complex geometries)\n";
        explanation << "- nSolveIter: " << dict.snapControls.nSolveIter
                    << " (more iterations = better quality but slower)\n";
    }
    
    if (dict.addLayers) {
        explanation << "\nBoundary Layer Settings:\n";
        explanation << "- expansionRatio: " << dict.addLayersControls.expansionRatio
                    << " (1.2 is conservative, reduces layer failure risk)\n";
        explanation << "- finalLayerThickness: " << dict.addLayersControls.finalLayerThickness
                    << " (relative to local cell size)\n";
    } else {
        explanation << "\nBoundary Layers: Disabled for reliability\n";
    }
    
    return explanation.str();
}

std::vector<std::string> SnappyTemplateLibrary::generateParameterQuestions(const SnappyHexMeshDict& dict) const
{
    std::vector<std::string> questions;
    
    questions.push_back("What determines the maximum number of cells in your mesh?");
    questions.push_back("Why might we disable boundary layers for complex geometries?");
    questions.push_back("How does the nCellsBetweenLevels parameter affect mesh quality?");
    questions.push_back("What happens if the snap tolerance is too low?");
    
    if (dict.addLayers) {
        questions.push_back("Why is the expansion ratio set to 1.2 rather than a higher value?");
        questions.push_back("How would you determine if boundary layers are capturing the flow correctly?");
    }
    
    return questions;
}

std::string SnappyTemplateLibrary::getParameterExplanation(const std::string& parameterName) const
{
    static std::map<std::string, std::string> explanations = {
        {"maxGlobalCells", "Maximum total cells in the mesh. Conservative values prevent memory issues and ensure reasonable computation times."},
        {"nCellsBetweenLevels", "Number of buffer cells between refinement levels. Higher values create smoother transitions but use more cells."},
        {"tolerance", "Snapping tolerance as multiple of local cell size. Higher values are more forgiving for complex geometries."},
        {"expansionRatio", "Growth ratio between boundary layer cells. 1.2 is conservative and reduces layer addition failures."},
        {"finalLayerThickness", "Thickness of the final boundary layer relative to local cell size. 0.3 balances resolution and robustness."}
    };
    
    auto it = explanations.find(parameterName);
    if (it != explanations.end()) {
        return it->second;
    }
    
    return "Parameter explanation not available.";
}

std::string SnappyTemplateLibrary::generateSnappyHexMeshDict(const SnappyHexMeshDict& params) const
{
    std::ostringstream dict;
    
    dict << "/*--------------------------------*- C++ -*----------------------------------*\\\n";
    dict << "| =========                 |                                                 |\n";
    dict << "| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n";
    dict << "|  \\\\    /   O peration     | Version:  12                                     |\n";
    dict << "|   \\\\  /    A nd           | Web:      www.OpenFOAM.org                      |\n";
    dict << "|    \\\\/     M anipulation  |                                                 |\n";
    dict << "\\*---------------------------------------------------------------------------*/\n";
    dict << "FoamFile\n{\n";
    dict << "    version     2.0;\n";
    dict << "    format      ascii;\n";
    dict << "    class       dictionary;\n";
    dict << "    object      snappyHexMeshDict;\n";
    dict << "}\n";
    dict << "// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //\n\n";
    
    dict << "// Which of the steps to run\n";
    dict << "castellatedMesh " << (params.castellatedMesh ? "true" : "false") << ";\n";
    dict << "snap            " << (params.snap ? "true" : "false") << ";\n";
    dict << "addLayers       " << (params.addLayers ? "true" : "false") << ";\n\n";
    
    dict << "// Geometry. Definition of all surfaces.\n";
    dict << "geometry\n{\n";
    for (const auto& geom : params.geometry) {
        dict << "    " << geom.first << "\n    {\n";
        dict << "        " << geom.second << "\n";
        dict << "    }\n";
    }
    dict << "};\n\n";
    
    dict << "// Settings for the castellatedMesh generation.\n";
    dict << "castellatedMeshControls\n{\n";
    dict << "    maxLocalCells " << params.castellatedMeshControls.maxLocalCells << ";\n";
    dict << "    maxGlobalCells " << params.castellatedMeshControls.maxGlobalCells << ";\n";
    dict << "    minRefinementCells " << params.castellatedMeshControls.minRefinementCells << ";\n";
    dict << "    nCellsBetweenLevels " << params.castellatedMeshControls.nCellsBetweenLevels << ";\n";
    dict << "    maxLoadUnbalance " << params.castellatedMeshControls.maxLoadUnbalance << ";\n";
    dict << "    allowFreeStandingZoneFaces " << (params.castellatedMeshControls.allowFreeStandingZoneFaces ? "true" : "false") << ";\n";
    dict << "\n    refinementSurfaces\n    {\n";
    for (const auto& surf : params.castellatedMeshControls.refinementSurfaces) {
        dict << "        " << surf.first << " { level (" << surf.second.first << " " << surf.second.second << "); }\n";
    }
    dict << "    }\n";
    dict << "\n    refinementRegions\n    {\n";
    for (const auto& region : params.castellatedMeshControls.refinementRegions) {
        dict << "        " << region.regionName << "\n        {\n";
        dict << "            mode " << region.regionType << ";\n";
        dict << "            levels ((" << region.refinementLevel << " " << region.refinementLevel << "));\n";
        dict << "        }\n";
    }
    dict << "    }\n";
    dict << "}\n\n";
    
    if (params.snap) {
        dict << "// Settings for the snapping.\n";
        dict << "snapControls\n{\n";
        dict << "    nSmoothPatch    " << params.snapControls.nSmoothPatch << ";\n";
        dict << "    tolerance       " << params.snapControls.tolerance << ";\n";
        dict << "    nSolveIter      " << params.snapControls.nSolveIter << ";\n";
        dict << "    nRelaxIter      " << params.snapControls.nRelaxIter << ";\n";
        dict << "    nFeatureSnapIter " << params.snapControls.nFeatureSnapIter << ";\n";
        dict << "    implicitFeatureSnap " << (params.snapControls.implicitFeatureSnap ? "true" : "false") << ";\n";
        dict << "    explicitFeatureSnap " << (params.snapControls.explicitFeatureSnap ? "true" : "false") << ";\n";
        dict << "    multiRegionFeatureSnap " << (params.snapControls.multiRegionFeatureSnap ? "true" : "false") << ";\n";
        dict << "}\n\n";
    }
    
    if (params.addLayers) {
        dict << "// Settings for the layer addition.\n";
        dict << "addLayersControls\n{\n";
        dict << "    relativeSizes " << (params.addLayersControls.relativeSizes ? "true" : "false") << ";\n";
        dict << "\n    layers\n    {\n";
        for (const auto& layer : params.addLayersControls.layers) {
            dict << "        " << layer.first << " { nSurfaceLayers " << layer.second << "; }\n";
        }
        dict << "    }\n";
        dict << "\n    expansionRatio    " << params.addLayersControls.expansionRatio << ";\n";
        dict << "    finalLayerThickness " << params.addLayersControls.finalLayerThickness << ";\n";
        dict << "    minThickness      " << params.addLayersControls.minThickness << ";\n";
        dict << "}\n\n";
    }
    
    dict << "// Generic mesh quality settings.\n";
    dict << "meshQualityControls\n{\n";
    dict << "    maxNonOrtho         " << params.meshQualityControls.maxNonOrtho << ";\n";
    dict << "    maxBoundarySkewness " << params.meshQualityControls.maxBoundarySkewness << ";\n";
    dict << "    maxInternalSkewness " << params.meshQualityControls.maxInternalSkewness << ";\n";
    dict << "    maxConcave          " << params.meshQualityControls.maxConcave << ";\n";
    dict << "    minVol              " << params.meshQualityControls.minVol << ";\n";
    dict << "    minTetQuality       " << params.meshQualityControls.minTetQuality << ";\n";
    dict << "    minArea             " << params.meshQualityControls.minArea << ";\n";
    dict << "    minTwist            " << params.meshQualityControls.minTwist << ";\n";
    dict << "    minDeterminant      " << params.meshQualityControls.minDeterminant << ";\n";
    dict << "    minFaceWeight       " << params.meshQualityControls.minFaceWeight << ";\n";
    dict << "    minVolRatio         " << params.meshQualityControls.minVolRatio << ";\n";
    dict << "    minTriangleTwist    " << params.meshQualityControls.minTriangleTwist << ";\n";
    dict << "    nSmoothScale        " << params.meshQualityControls.nSmoothScale << ";\n";
    dict << "    errorReduction      " << params.meshQualityControls.errorReduction << ";\n";
    dict << "}\n\n";
    
    dict << "// ************************************************************************* //\n";
    
    return dict.str();
}

std::string SnappyTemplateLibrary::generateSurfaceFeatureExtractDict(const std::string& stlFile,
                                                                   double featureAngle) const
{
    std::ostringstream dict;
    
    dict << "/*--------------------------------*- C++ -*----------------------------------*\\\n";
    dict << "| =========                 |                                                 |\n";
    dict << "| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n";
    dict << "|  \\\\    /   O peration     | Version:  12                                     |\n";
    dict << "|   \\\\  /    A nd           | Web:      www.OpenFOAM.org                      |\n";
    dict << "|    \\\\/     M anipulation  |                                                 |\n";
    dict << "\\*---------------------------------------------------------------------------*/\n";
    dict << "FoamFile\n{\n";
    dict << "    version     2.0;\n";
    dict << "    format      ascii;\n";
    dict << "    class       dictionary;\n";
    dict << "    object      surfaceFeatureExtractDict;\n";
    dict << "}\n";
    dict << "// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //\n\n";
    
    // Extract filename without path
    std::string filename = stlFile.substr(stlFile.find_last_of("/\\") + 1);
    
    dict << filename << "\n{\n";
    dict << "    extractionMethod    extractFromSurface;\n";
    dict << "    includedAngle       " << featureAngle << ";\n";
    dict << "    writeObj            yes;\n";
    dict << "}\n\n";
    
    dict << "// ************************************************************************* //\n";
    
    return dict.str();
}

double SnappyTemplateLibrary::estimateCellCount(const SnappyHexMeshDict& dict, 
                                              const std::vector<double>& boundingBox)
{
    if (boundingBox.size() < 6) return 1000000; // Default estimate
    
    double volume = (boundingBox[3] - boundingBox[0]) * 
                   (boundingBox[4] - boundingBox[1]) * 
                   (boundingBox[5] - boundingBox[2]);
    
    // Rough estimate based on background mesh and refinement
    double backgroundCells = volume / 0.001; // Assume 0.1m background spacing cubed
    
    // Apply refinement factor (crude estimate)
    double refinementFactor = 2.0; // Average refinement effect
    
    return std::min(static_cast<double>(dict.castellatedMeshControls.maxGlobalCells), 
                   backgroundCells * refinementFactor);
}

double SnappyTemplateLibrary::estimateMemoryUsage(const SnappyHexMeshDict& dict,
                                                const std::vector<double>& boundingBox)
{
    double cellCount = estimateCellCount(dict, boundingBox);
    
    // Rough estimate: 1KB per cell for mesh + fields
    return cellCount * 1024.0; // bytes
}

std::string SnappyTemplateLibrary::formatParameterSummary(const SnappyHexMeshDict& dict)
{
    std::ostringstream summary;
    
    summary << "SnappyHexMesh Configuration Summary:\n";
    summary << "- Castellated: " << (dict.castellatedMesh ? "Yes" : "No") << "\n";
    summary << "- Snap: " << (dict.snap ? "Yes" : "No") << "\n";
    summary << "- Layers: " << (dict.addLayers ? "Yes" : "No") << "\n";
    summary << "- Max cells: " << dict.castellatedMeshControls.maxGlobalCells << "\n";
    
    if (dict.snap) {
        summary << "- Snap tolerance: " << dict.snapControls.tolerance << "\n";
    }
    
    if (dict.addLayers) {
        summary << "- Expansion ratio: " << dict.addLayersControls.expansionRatio << "\n";
    }
    
    return summary.str();
}

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const SnappyHexMeshDict::CastellatedControls& controls) {
    j = json{
        {"maxLocalCells", controls.maxLocalCells},
        {"maxGlobalCells", controls.maxGlobalCells},
        {"minRefinementCells", controls.minRefinementCells},
        {"nCellsBetweenLevels", controls.nCellsBetweenLevels},
        {"maxLoadUnbalance", controls.maxLoadUnbalance},
        {"allowFreeStandingZoneFaces", controls.allowFreeStandingZoneFaces},
        {"refinementSurfaces", controls.refinementSurfaces},
        {"refinementRegions", controls.refinementRegions}
    };
}

void from_json(const json& j, SnappyHexMeshDict::CastellatedControls& controls) {
    j.at("maxLocalCells").get_to(controls.maxLocalCells);
    j.at("maxGlobalCells").get_to(controls.maxGlobalCells);
    j.at("minRefinementCells").get_to(controls.minRefinementCells);
    j.at("nCellsBetweenLevels").get_to(controls.nCellsBetweenLevels);
    j.at("maxLoadUnbalance").get_to(controls.maxLoadUnbalance);
    j.at("allowFreeStandingZoneFaces").get_to(controls.allowFreeStandingZoneFaces);
    j.at("refinementSurfaces").get_to(controls.refinementSurfaces);
    j.at("refinementRegions").get_to(controls.refinementRegions);
}

void to_json(json& j, const SnappyHexMeshDict::SnapControls& controls) {
    j = json{
        {"nSmoothPatch", controls.nSmoothPatch},
        {"tolerance", controls.tolerance},
        {"nSolveIter", controls.nSolveIter},
        {"nRelaxIter", controls.nRelaxIter},
        {"nFeatureSnapIter", controls.nFeatureSnapIter},
        {"implicitFeatureSnap", controls.implicitFeatureSnap},
        {"explicitFeatureSnap", controls.explicitFeatureSnap},
        {"multiRegionFeatureSnap", controls.multiRegionFeatureSnap}
    };
}

void from_json(const json& j, SnappyHexMeshDict::SnapControls& controls) {
    j.at("nSmoothPatch").get_to(controls.nSmoothPatch);
    j.at("tolerance").get_to(controls.tolerance);
    j.at("nSolveIter").get_to(controls.nSolveIter);
    j.at("nRelaxIter").get_to(controls.nRelaxIter);
    j.at("nFeatureSnapIter").get_to(controls.nFeatureSnapIter);
    j.at("implicitFeatureSnap").get_to(controls.implicitFeatureSnap);
    j.at("explicitFeatureSnap").get_to(controls.explicitFeatureSnap);
    j.at("multiRegionFeatureSnap").get_to(controls.multiRegionFeatureSnap);
}

void to_json(json& j, const SnappyHexMeshDict::AddLayersControls& controls) {
    j = json{
        {"relativeSizes", controls.relativeSizes},
        {"layers", controls.layers},
        {"expansionRatio", controls.expansionRatio},
        {"finalLayerThickness", controls.finalLayerThickness},
        {"minThickness", controls.minThickness},
        {"nGrow", controls.nGrow},
        {"featureAngle", controls.featureAngle},
        {"nRelaxIter", controls.nRelaxIter}
    };
}

void from_json(const json& j, SnappyHexMeshDict::AddLayersControls& controls) {
    j.at("relativeSizes").get_to(controls.relativeSizes);
    j.at("layers").get_to(controls.layers);
    j.at("expansionRatio").get_to(controls.expansionRatio);
    j.at("finalLayerThickness").get_to(controls.finalLayerThickness);
    j.at("minThickness").get_to(controls.minThickness);
    j.at("nGrow").get_to(controls.nGrow);
    j.at("featureAngle").get_to(controls.featureAngle);
    j.at("nRelaxIter").get_to(controls.nRelaxIter);
}

void to_json(json& j, const SnappyHexMeshDict::MeshQualityControls& controls) {
    j = json{
        {"maxNonOrtho", controls.maxNonOrtho},
        {"maxBoundarySkewness", controls.maxBoundarySkewness},
        {"maxInternalSkewness", controls.maxInternalSkewness},
        {"maxConcave", controls.maxConcave},
        {"minVol", controls.minVol},
        {"minTetQuality", controls.minTetQuality}
    };
}

void from_json(const json& j, SnappyHexMeshDict::MeshQualityControls& controls) {
    j.at("maxNonOrtho").get_to(controls.maxNonOrtho);
    j.at("maxBoundarySkewness").get_to(controls.maxBoundarySkewness);
    j.at("maxInternalSkewness").get_to(controls.maxInternalSkewness);
    j.at("maxConcave").get_to(controls.maxConcave);
    j.at("minVol").get_to(controls.minVol);
    j.at("minTetQuality").get_to(controls.minTetQuality);
}

void to_json(json& j, const SnappyHexMeshDict& dict) {
    j = json{
        {"castellatedMesh", dict.castellatedMesh},
        {"snap", dict.snap},
        {"addLayers", dict.addLayers},
        {"geometry", dict.geometry},
        {"castellatedMeshControls", dict.castellatedMeshControls},
        {"snapControls", dict.snapControls},
        {"addLayersControls", dict.addLayersControls},
        {"meshQualityControls", dict.meshQualityControls}
    };
}

void from_json(const json& j, SnappyHexMeshDict& dict) {
    j.at("castellatedMesh").get_to(dict.castellatedMesh);
    j.at("snap").get_to(dict.snap);
    j.at("addLayers").get_to(dict.addLayers);
    j.at("geometry").get_to(dict.geometry);
    j.at("castellatedMeshControls").get_to(dict.castellatedMeshControls);
    j.at("snapControls").get_to(dict.snapControls);
    j.at("addLayersControls").get_to(dict.addLayersControls);
    j.at("meshQualityControls").get_to(dict.meshQualityControls);
}

void to_json(json& j, const MeshingStrategy& strategy) {
    j = json{
        {"strategyName", strategy.strategyName},
        {"description", strategy.description},
        {"applicableTo", strategy.applicableTo},
        {"complexity", strategy.complexity},
        {"parameters", strategy.parameters},
        {"expectedIssues", strategy.expectedIssues},
        {"qualityExpectations", strategy.qualityExpectations},
        {"troubleshootingGuide", strategy.troubleshootingGuide}
    };
}

void from_json(const json& j, MeshingStrategy& strategy) {
    j.at("strategyName").get_to(strategy.strategyName);
    j.at("description").get_to(strategy.description);
    j.at("applicableTo").get_to(strategy.applicableTo);
    j.at("complexity").get_to(strategy.complexity);
    j.at("parameters").get_to(strategy.parameters);
    j.at("expectedIssues").get_to(strategy.expectedIssues);
    j.at("qualityExpectations").get_to(strategy.qualityExpectations);
    j.at("troubleshootingGuide").get_to(strategy.troubleshootingGuide);
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //