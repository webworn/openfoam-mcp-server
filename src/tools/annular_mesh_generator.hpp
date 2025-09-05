#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <nlohmann/json.hpp>
#include "../openfoam/rotating_detonation_engine.hpp"
#include "snappy_mesh_tool.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                    Class AnnularMeshGenerator
\*---------------------------------------------------------------------------*/

/**
 * 2D Annular Mesh Generator for RDE Applications
 * 
 * Specialized mesh generator for rotating detonation engines with:
 * - Cylindrical coordinate system support
 * - Cellular detonation constraint enforcement (Δx < λ/10)
 * - Injection port refinement
 * - Boundary layer meshing
 * - Educational mesh quality analysis
 */
class AnnularMeshGenerator {
public:
    AnnularMeshGenerator();
    ~AnnularMeshGenerator() = default;
    
    // Core mesh generation interface
    struct AnnularMeshRequest {
        RDEGeometry geometry;
        RDESimulationSettings settings;
        RDEChemistry chemistry;  // For cellular constraints
        std::string caseDirectory;
        
        // Mesh generation options
        std::string meshMethod;     // "blockMesh", "snappyHexMesh", "structured"
        bool enableQualityChecks;
        bool generateEducationalReport;
        double cellularSafetyFactor;  // Safety factor for cellular constraint (default: 10.0)
    };
    
    struct AnnularMeshResult {
        bool success;
        std::string meshDirectory;
        
        // Mesh statistics
        int totalCells;
        int radialCells;
        int circumferentialCells;
        int axialCells;
        double minCellSize;
        double maxCellSize;
        double aspectRatioMax;
        
        // Cellular constraint validation
        bool cellularConstraintSatisfied;
        double requiredCellSize;     // λ/10 requirement
        double actualMinCellSize;    // Actual minimum cell size
        double cellularSafetyMargin; // How much smaller than required
        
        // Quality metrics
        double skewnessMax;
        double orthogonalityMin;
        int nonOrthogonalFaces;
        
        // Injection region analysis
        std::vector<int> injectionRegionCells;
        std::vector<double> injectionRegionRefinement;
        
        // Educational content
        std::string meshingStrategy;
        std::string qualityAnalysis;
        std::string cellularConstraintExplanation;
        std::vector<std::string> recommendations;
        std::vector<std::string> warnings;
    };
    
    // Main mesh generation method
    AnnularMeshResult generateAnnularMesh(const AnnularMeshRequest& request);
    
    // Specialized mesh generation methods
    bool generateStructuredAnnularMesh(const std::string& caseDir, const RDEGeometry& geometry, 
                                      const RDESimulationSettings& settings);
    bool generateBlockMeshDict(const std::string& caseDir, const RDEGeometry& geometry, 
                              const RDESimulationSettings& settings);
    bool generateSnappyAnnularMesh(const std::string& caseDir, const RDEGeometry& geometry, 
                                  const RDESimulationSettings& settings);
    
    // Cellular constraint enforcement
    bool validateCellularConstraints(const RDEGeometry& geometry, const RDEChemistry& chemistry,
                                   const RDESimulationSettings& settings, std::string& report);
    double calculateRequiredCellSize(const RDEChemistry& chemistry, double safetyFactor = 10.0);
    RDESimulationSettings adjustMeshForCellularConstraints(const RDEGeometry& geometry, 
                                                          const RDEChemistry& chemistry,
                                                          const RDESimulationSettings& settings);
    
    // Injection region refinement
    bool refineInjectionRegions(const std::string& caseDir, const RDEGeometry& geometry,
                               double refinementFactor = 2.0);
    std::vector<std::pair<double, double>> identifyInjectionInfluenceZones(const RDEGeometry& geometry);
    
    // Quality analysis and validation
    bool analyzeMeshQuality(const std::string& caseDir, AnnularMeshResult& result);
    bool checkAnnularGeometryConstraints(const RDEGeometry& geometry, std::string& errorMsg);
    bool validatePeriodicBoundaries(const std::string& caseDir, const RDEGeometry& geometry);
    
    // Educational content generation
    std::string generateMeshingStrategyExplanation(const AnnularMeshRequest& request);
    std::string explainCellularConstraints(const RDEChemistry& chemistry, double requiredCellSize);
    std::string explainAnnularMeshChallenges(const RDEGeometry& geometry);
    std::vector<std::string> generateMeshingRecommendations(const AnnularMeshResult& result);
    
private:
    std::unique_ptr<SnappyMeshTool> snappyMeshTool_;
    
    // Mesh generation utilities
    bool createBlockMeshGeometry(const std::string& blockMeshDict, const RDEGeometry& geometry,
                                const RDESimulationSettings& settings);
    bool setupCylindricalCoordinates(const std::string& caseDir, const RDEGeometry& geometry);
    bool createPeriodicPatches(const std::string& caseDir, const RDEGeometry& geometry);
    
    // Boundary layer mesh generation
    bool generateBoundaryLayerMesh(const std::string& caseDir, const RDEGeometry& geometry,
                                  const RDESimulationSettings& settings);
    double calculateFirstCellHeight(double yPlus, double characteristicVelocity, double kinematicViscosity);
    
    // Mesh refinement utilities
    bool implementGradedMeshing(const std::string& caseDir, const RDEGeometry& geometry);
    bool addLocalRefinement(const std::string& caseDir, const std::vector<std::pair<double, double>>& regions,
                           double refinementFactor);
    
    // Quality checking utilities
    double calculateCellAspectRatio(const std::vector<double>& cellDimensions);
    double calculateCellSkewness(const std::vector<std::vector<double>>& cellVertices);
    bool checkMeshOrthogonality(const std::string& caseDir, double& minOrthogonality);
    
    // Educational utilities
    std::string explainMeshQualityMetrics(const AnnularMeshResult& result);
    std::string generateTroubleshootingGuide(const AnnularMeshResult& result);
    std::vector<std::string> suggestMeshImprovements(const AnnularMeshResult& result);
    
    // Coordinate transformation utilities
    std::pair<double, double> cartesianToCylindrical(double x, double y);
    std::pair<double, double> cylindricalToCartesian(double r, double theta);
    
    // Validation utilities
    bool validateMeshParameters(const RDEGeometry& geometry, const RDESimulationSettings& settings,
                               std::string& errorMsg);
    bool estimateComputationalRequirements(const RDEGeometry& geometry, const RDESimulationSettings& settings,
                                          double& estimatedMemoryGB, double& estimatedTimeHours);
};

/*---------------------------------------------------------------------------*\
                    Helper Functions for Annular Meshing
\*---------------------------------------------------------------------------*/

// Mesh quality assessment functions
double calculateAnnularMeshQuality(const std::string& caseDir);
bool validateAnnularTopology(const std::string& caseDir, const RDEGeometry& geometry);

// Educational content helpers
std::string explainCylindricalMeshChallenges();
std::string explainPeriodicBoundaryImplementation();
std::string explainInjectionPortMeshing();

// Cellular detonation mesh constraint helpers
double calculateDetonationCellSize(const std::string& fuelType, double equivalenceRatio, 
                                  double pressure, double temperature);
bool isCellularConstraintSatisfied(double actualCellSize, double requiredCellSize);
std::string explainCellularMeshRequirements(double cellSize, double meshSize, bool satisfied);

} // namespace MCP
} // namespace Foam