/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | STL Quality Analyzer
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    STL geometry quality analyzer for snappyHexMesh preprocessing.
    
    Expert-validated approach: "90% of snappyHexMesh failures start with bad 
    geometry". This analyzer identifies and provides guidance for fixing
    common STL issues before meshing begins.

\*---------------------------------------------------------------------------*/

#ifndef stl_analyzer_H
#define stl_analyzer_H

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Struct GeometryFeature
\*---------------------------------------------------------------------------*/

struct GeometryFeature {
    std::string featureType;    // "edge", "corner", "surface"
    std::vector<double> location;  // [x, y, z]
    double angle;               // feature angle in degrees
    double length;              // feature length or area
    std::string importance;     // "critical", "important", "minor"
    
    GeometryFeature() : angle(0.0), length(0.0), importance("minor") {}
};

/*---------------------------------------------------------------------------*\
                        Struct GeometryComplexity
\*---------------------------------------------------------------------------*/

struct GeometryComplexity {
    std::string level;          // "simple", "moderate", "complex", "very_complex"
    double surfaceArea;         // total surface area [m²]
    double volume;              // enclosed volume [m³] (if watertight)
    int numberOfSurfaces;       // number of discrete surfaces
    int numberOfFeatures;       // number of significant features
    double aspectRatio;         // max/min characteristic dimensions
    bool hasSharpFeatures;      // features < 30 degrees
    bool hasConcaveRegions;     // concave regions present
    double characteristicLength; // representative size [m]
    
    GeometryComplexity() 
        : level("simple"), surfaceArea(0.0), volume(0.0), 
          numberOfSurfaces(0), numberOfFeatures(0), aspectRatio(1.0),
          hasSharpFeatures(false), hasConcaveRegions(false), 
          characteristicLength(1.0) {}
};

/*---------------------------------------------------------------------------*\
                        Struct STLQualityIssue
\*---------------------------------------------------------------------------*/

struct STLQualityIssue {
    std::string issueType;      // "duplicate_vertices", "non_manifold", "inverted_normals", etc.
    std::string severity;       // "critical", "warning", "info"
    std::string description;    // human-readable description
    std::string location;       // approximate location of issue
    std::string fixSuggestion;  // suggested repair method
    bool canAutoFix;           // whether automatic repair is possible
    
    STLQualityIssue() : severity("info"), canAutoFix(false) {}
};

/*---------------------------------------------------------------------------*\
                        Struct STLQualityReport
\*---------------------------------------------------------------------------*/

struct STLQualityReport {
    // File information
    std::string filename;
    double fileSize;            // file size in MB
    int numberOfTriangles;
    int numberOfVertices;
    
    // Quality metrics
    bool isWatertight;
    bool hasManifoldEdges;
    bool hasConsistentNormals;
    double minTriangleQuality;  // 0-1 scale (aspect ratio based)
    double avgTriangleQuality;
    
    // Geometric properties
    std::vector<double> boundingBox;  // [xmin, ymin, zmin, xmax, ymax, zmax]
    double surfaceArea;
    double volume;              // -1 if not watertight
    
    // Issues and complexity
    std::vector<STLQualityIssue> issues;
    GeometryComplexity complexity;
    std::vector<GeometryFeature> features;
    
    // snappyHexMesh readiness
    bool readyForSnappy;
    std::string readinessReason;
    
    STLQualityReport() 
        : fileSize(0.0), numberOfTriangles(0), numberOfVertices(0),
          isWatertight(false), hasManifoldEdges(false), hasConsistentNormals(false),
          minTriangleQuality(0.0), avgTriangleQuality(0.0), surfaceArea(0.0), 
          volume(-1.0), readyForSnappy(false) {}
};

/*---------------------------------------------------------------------------*\
                        Struct RefinementRegion
\*---------------------------------------------------------------------------*/

struct RefinementRegion {
    std::string regionName;
    std::string regionType;     // "box", "sphere", "surface", "distance"
    std::vector<double> geometry; // region-specific geometric parameters
    int refinementLevel;        // 0-6 typical range
    std::string reason;         // physics-based justification
    std::string priority;       // "essential", "recommended", "optional"
    
    RefinementRegion() : refinementLevel(1), priority("optional") {}
};

/*---------------------------------------------------------------------------*\
                        Class STLAnalyzer Declaration
\*---------------------------------------------------------------------------*/

class STLAnalyzer {
private:
    // Core analysis methods
    STLQualityReport analyzeGeometryStructure(const std::string& stlFile);
    std::vector<STLQualityIssue> identifyQualityIssues(const std::string& stlFile);
    GeometryComplexity assessComplexity(const STLQualityReport& report);
    std::vector<GeometryFeature> extractFeatures(const std::string& stlFile, double featureAngle = 30.0);
    
    // Issue detection methods
    bool checkWatertight(const std::string& stlFile);
    bool checkManifoldEdges(const std::string& stlFile);
    bool checkNormalConsistency(const std::string& stlFile);
    std::vector<STLQualityIssue> findDuplicateVertices(const std::string& stlFile);
    std::vector<STLQualityIssue> findDegenerateTriangles(const std::string& stlFile);
    std::vector<STLQualityIssue> findInvertedTriangles(const std::string& stlFile);
    
    // Quality metrics
    double calculateTriangleQuality(const std::vector<double>& v1, 
                                   const std::vector<double>& v2, 
                                   const std::vector<double>& v3);
    double calculateSurfaceArea(const std::string& stlFile);
    double calculateVolume(const std::string& stlFile);
    std::vector<double> calculateBoundingBox(const std::string& stlFile);
    
    // Feature detection
    std::vector<GeometryFeature> detectSharpEdges(const std::string& stlFile, double angleThreshold);
    std::vector<GeometryFeature> detectConcaveRegions(const std::string& stlFile);
    std::vector<GeometryFeature> detectHighCurvatureRegions(const std::string& stlFile);
    
    // Complexity assessment
    std::string classifyComplexity(const GeometryComplexity& complexity);
    double calculateAspectRatio(const std::vector<double>& boundingBox);
    int countDiscreteComponents(const std::string& stlFile);
    
    // Repair suggestions
    std::string generateRepairGuidance(const std::vector<STLQualityIssue>& issues);
    std::vector<std::string> suggestSTLPreprocessing(const STLQualityReport& report);
    
    // snappyHexMesh readiness
    bool assessSnappyReadiness(const STLQualityReport& report);
    std::string generateReadinessExplanation(const STLQualityReport& report);

public:
    STLAnalyzer();
    ~STLAnalyzer() = default;
    
    // Main analysis interface
    STLQualityReport analyzeSTL(const std::string& stlFile);
    
    // Issue identification and repair
    std::vector<STLQualityIssue> identifyIssues(const std::string& stlFile);
    std::string generateRepairScript(const std::string& stlFile, 
                                   const std::vector<STLQualityIssue>& issues);
    bool attemptAutoRepair(const std::string& stlFile, const std::string& outputFile);
    
    // Feature extraction for snappyHexMesh
    std::vector<GeometryFeature> extractFeatureLines(const std::string& stlFile, 
                                                   double featureAngle = 30.0);
    std::string generateFeatureExtractDict(const std::string& stlFile, 
                                         const std::vector<GeometryFeature>& features);
    
    // Refinement guidance
    std::vector<RefinementRegion> suggestRefinementRegions(const STLQualityReport& report,
                                                          const std::string& flowType = "external");
    std::string explainRefinementStrategy(const std::vector<RefinementRegion>& regions);
    
    // Educational content generation
    std::string explainGeometryIssues(const std::vector<STLQualityIssue>& issues);
    std::string explainComplexityImpact(const GeometryComplexity& complexity);
    std::vector<std::string> generateGeometryQuestions(const STLQualityReport& report);
    
    // Utility methods
    bool validateSTLFile(const std::string& stlFile);
    std::string getSTLStatistics(const std::string& stlFile);
    json reportToJson(const STLQualityReport& report);
    
    // Static utility methods
    static bool isValidSTLFile(const std::string& filename);
    static std::vector<double> calculateCharacteristicLengths(const std::vector<double>& boundingBox);
    static std::string formatFileSize(double sizeInBytes);
};

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const GeometryFeature& feature);
void from_json(const json& j, GeometryFeature& feature);

void to_json(json& j, const GeometryComplexity& complexity);
void from_json(const json& j, GeometryComplexity& complexity);

void to_json(json& j, const STLQualityIssue& issue);
void from_json(const json& j, STLQualityIssue& issue);

void to_json(json& j, const STLQualityReport& report);
void from_json(const json& j, STLQualityReport& report);

void to_json(json& j, const RefinementRegion& region);
void from_json(const json& j, RefinementRegion& region);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //