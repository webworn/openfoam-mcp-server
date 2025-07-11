/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | SnappyHexMesh Templates
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Conservative snappyHexMesh parameter templates for reliable mesh generation.
    
    Expert approach: "Use proven parameter combinations, don't try to be clever"
    These templates are based on successful OpenFOAM tutorial cases and
    expert-validated parameter sets for common scenarios.

\*---------------------------------------------------------------------------*/

#ifndef snappy_templates_H
#define snappy_templates_H

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "stl_analyzer.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Struct SnappyHexMeshDict
\*---------------------------------------------------------------------------*/

struct SnappyHexMeshDict {
    // Main switches
    bool castellatedMesh;
    bool snap;
    bool addLayers;
    
    // Geometry section
    std::map<std::string, std::string> geometry;
    
    // CastellatedMeshControls
    struct CastellatedControls {
        int maxLocalCells;
        int maxGlobalCells;
        int minRefinementCells;
        int nCellsBetweenLevels;
        double maxLoadUnbalance;
        bool allowFreeStandingZoneFaces;
        
        // RefinementSurfaces
        std::map<std::string, std::pair<int, int>> refinementSurfaces; // surface -> (min, max) levels
        
        // RefinementRegions  
        std::vector<RefinementRegion> refinementRegions;
        
        CastellatedControls() 
            : maxLocalCells(100000), maxGlobalCells(2000000), 
              minRefinementCells(10), nCellsBetweenLevels(3),
              maxLoadUnbalance(0.10), allowFreeStandingZoneFaces(true) {}
    } castellatedMeshControls;
    
    // SnapControls
    struct SnapControls {
        int nSmoothPatch;
        double tolerance;
        int nSolveIter;
        int nRelaxIter;
        int nFeatureSnapIter;
        bool implicitFeatureSnap;
        bool explicitFeatureSnap;
        bool multiRegionFeatureSnap;
        
        SnapControls()
            : nSmoothPatch(3), tolerance(2.0), nSolveIter(30), nRelaxIter(5),
              nFeatureSnapIter(10), implicitFeatureSnap(false), 
              explicitFeatureSnap(true), multiRegionFeatureSnap(false) {}
    } snapControls;
    
    // AddLayersControls
    struct AddLayersControls {
        double relativeSizes;
        std::map<std::string, int> layers; // surface -> number of layers
        double expansionRatio;
        double finalLayerThickness;
        double minThickness;
        int nGrow;
        double featureAngle;
        int nRelaxIter;
        int nSmoothSurfaceNormals;
        int nSmoothNormals;
        int nSmoothThickness;
        double maxFaceThicknessRatio;
        double maxThicknessToMedialRatio;
        double minMedianAxisAngle;
        int nBufferCellsNoExtrude;
        int nLayerIter;
        
        AddLayersControls()
            : relativeSizes(true), expansionRatio(1.2), finalLayerThickness(0.3),
              minThickness(0.1), nGrow(0), featureAngle(60), nRelaxIter(3),
              nSmoothSurfaceNormals(1), nSmoothNormals(3), nSmoothThickness(10),
              maxFaceThicknessRatio(0.5), maxThicknessToMedialRatio(0.3),
              minMedianAxisAngle(90), nBufferCellsNoExtrude(0), nLayerIter(50) {}
    } addLayersControls;
    
    // MeshQualityControls
    struct MeshQualityControls {
        double maxNonOrtho;
        double maxBoundarySkewness;
        double maxInternalSkewness;
        double maxConcave;
        double minFlatness;
        double minVol;
        double minTetQuality;
        double minArea;
        double minTwist;
        double minDeterminant;
        double minFaceWeight;
        double minVolRatio;
        double minTriangleTwist;
        int nSmoothScale;
        double errorReduction;
        
        MeshQualityControls()
            : maxNonOrtho(65), maxBoundarySkewness(20), maxInternalSkewness(4),
              maxConcave(80), minFlatness(0.5), minVol(1e-13), minTetQuality(1e-15),
              minArea(-1), minTwist(0.02), minDeterminant(0.001), minFaceWeight(0.02),
              minVolRatio(0.01), minTriangleTwist(-1), nSmoothScale(4), errorReduction(0.75) {}
    } meshQualityControls;
    
    SnappyHexMeshDict()
        : castellatedMesh(true), snap(true), addLayers(false) {}
};

/*---------------------------------------------------------------------------*\
                        Struct MeshingStrategy
\*---------------------------------------------------------------------------*/

struct MeshingStrategy {
    std::string strategyName;
    std::string description;
    std::string applicableTo;      // "external_flow", "internal_flow", "general"
    GeometryComplexity complexity;
    SnappyHexMeshDict parameters;
    
    // Validation and quality expectations
    std::vector<std::string> expectedIssues;
    std::vector<std::string> qualityExpectations;
    std::string troubleshootingGuide;
    
    MeshingStrategy() {}
};

/*---------------------------------------------------------------------------*\
                        Class SnappyTemplateLibrary Declaration
\*---------------------------------------------------------------------------*/

class SnappyTemplateLibrary {
private:
    std::map<std::string, MeshingStrategy> strategies_;
    
    // Template generation methods
    SnappyHexMeshDict createBasicExternalFlowTemplate(const GeometryComplexity& complexity);
    SnappyHexMeshDict createBasicInternalFlowTemplate(const GeometryComplexity& complexity);
    SnappyHexMeshDict createConservativeTemplate();
    
    // Parameter calculation methods
    std::pair<int, int> calculateSurfaceRefinement(const GeometryComplexity& complexity, 
                                                  const std::string& flowType);
    int calculateMaxGlobalCells(const GeometryComplexity& complexity);
    double calculateBackgroundMeshSize(const std::vector<double>& boundingBox);
    
    // Quality control methods
    SnappyHexMeshDict applyConservativeQualityControls(const SnappyHexMeshDict& baseDict);
    bool validateParameterCombination(const SnappyHexMeshDict& dict);
    
    // Expert-validated parameter sets
    void initializeValidatedStrategies();
    void addExternalFlowStrategies();
    void addInternalFlowStrategies();
    void addGeneralPurposeStrategies();

public:
    SnappyTemplateLibrary();
    ~SnappyTemplateLibrary() = default;
    
    // Main template selection methods
    SnappyHexMeshDict getOptimalTemplate(const STLQualityReport& geometryReport,
                                       const std::string& flowType = "external",
                                       const std::string& accuracyLevel = "moderate");
    
    SnappyHexMeshDict getExternalFlowTemplate(const GeometryComplexity& complexity);
    SnappyHexMeshDict getInternalFlowTemplate(const GeometryComplexity& complexity);
    SnappyHexMeshDict getConservativeDefaults(); // Safe fallback
    
    // Strategy management
    std::vector<std::string> getAvailableStrategies() const;
    MeshingStrategy getStrategy(const std::string& strategyName) const;
    std::string recommendStrategy(const STLQualityReport& geometryReport,
                                const std::string& flowType) const;
    
    // Parameter validation and adjustment
    bool validateParameterCombination(const SnappyHexMeshDict& dict) const;
    SnappyHexMeshDict adjustForGeometry(const SnappyHexMeshDict& baseDict,
                                      const STLQualityReport& geometryReport) const;
    
    // Educational content
    std::string explainParameterChoices(const SnappyHexMeshDict& dict) const;
    std::vector<std::string> generateParameterQuestions(const SnappyHexMeshDict& dict) const;
    std::string getParameterExplanation(const std::string& parameterName) const;
    
    // Dictionary generation
    std::string generateSnappyHexMeshDict(const SnappyHexMeshDict& params) const;
    std::string generateSurfaceFeatureExtractDict(const std::string& stlFile,
                                                 double featureAngle = 150.0) const;
    
    // Refinement region management
    std::vector<RefinementRegion> suggestRefinementRegions(const STLQualityReport& geometryReport,
                                                          const std::string& flowType) const;
    SnappyHexMeshDict addRefinementRegions(const SnappyHexMeshDict& baseDict,
                                         const std::vector<RefinementRegion>& regions) const;
    
    // Quality prediction and troubleshooting
    std::vector<std::string> predictPotentialIssues(const SnappyHexMeshDict& dict,
                                                   const STLQualityReport& geometryReport) const;
    std::string generateTroubleshootingGuide(const SnappyHexMeshDict& dict) const;
    
    // Utility methods
    static double estimateCellCount(const SnappyHexMeshDict& dict, 
                                  const std::vector<double>& boundingBox);
    static double estimateMemoryUsage(const SnappyHexMeshDict& dict,
                                    const std::vector<double>& boundingBox);
    static std::string formatParameterSummary(const SnappyHexMeshDict& dict);
};

/*---------------------------------------------------------------------------*\
                        Expert Parameter Sets
\*---------------------------------------------------------------------------*/

namespace ExpertParameterSets {
    // Proven parameter combinations from OpenFOAM tutorials
    extern const SnappyHexMeshDict MOTORBIKE_PARAMETERS;
    extern const SnappyHexMeshDict AHMED_BODY_PARAMETERS;
    extern const SnappyHexMeshDict PIPE_FLOW_PARAMETERS;
    extern const SnappyHexMeshDict BUILDING_PARAMETERS;
    extern const SnappyHexMeshDict AIRCRAFT_PARAMETERS;
    
    // Conservative fallback parameters
    extern const SnappyHexMeshDict CONSERVATIVE_EXTERNAL;
    extern const SnappyHexMeshDict CONSERVATIVE_INTERNAL;
    extern const SnappyHexMeshDict MINIMAL_RELIABLE;
}

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const SnappyHexMeshDict::CastellatedControls& controls);
void from_json(const json& j, SnappyHexMeshDict::CastellatedControls& controls);

void to_json(json& j, const SnappyHexMeshDict::SnapControls& controls);
void from_json(const json& j, SnappyHexMeshDict::SnapControls& controls);

void to_json(json& j, const SnappyHexMeshDict::AddLayersControls& controls);
void from_json(const json& j, SnappyHexMeshDict::AddLayersControls& controls);

void to_json(json& j, const SnappyHexMeshDict::MeshQualityControls& controls);
void from_json(const json& j, SnappyHexMeshDict::MeshQualityControls& controls);

void to_json(json& j, const SnappyHexMeshDict& dict);
void from_json(const json& j, SnappyHexMeshDict& dict);

void to_json(json& j, const MeshingStrategy& strategy);
void from_json(const json& j, MeshingStrategy& strategy);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //