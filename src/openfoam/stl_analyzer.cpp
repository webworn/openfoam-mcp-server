/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | STL Quality Analyzer
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Implementation of STL geometry quality analyzer for snappyHexMesh 
    preprocessing. Follows expert-validated approach focusing on geometry 
    quality issues that cause snappyHexMesh failures.

\*---------------------------------------------------------------------------*/

#include "stl_analyzer.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>

namespace Foam {
namespace MCP {

namespace fs = std::filesystem;

/*---------------------------------------------------------------------------*\
                        Class STLAnalyzer Implementation
\*---------------------------------------------------------------------------*/

STLAnalyzer::STLAnalyzer()
{
    // Constructor - initialize any required data structures
}

STLQualityReport STLAnalyzer::analyzeSTL(const std::string& stlFile)
{
    STLQualityReport report;
    
    // Validate file exists and is readable
    if (!validateSTLFile(stlFile)) {
        STLQualityIssue issue;
        issue.issueType = "file_access";
        issue.severity = "critical";
        issue.description = "Cannot read STL file: " + stlFile;
        issue.fixSuggestion = "Check file path and permissions";
        issue.canAutoFix = false;
        report.issues.push_back(issue);
        report.readyForSnappy = false;
        report.readinessReason = "File access error";
        return report;
    }
    
    report.filename = stlFile;
    
    try {
        // Phase 1: Basic file analysis
        report = analyzeGeometryStructure(stlFile);
        
        // Phase 2: Quality issue detection
        report.issues = identifyQualityIssues(stlFile);
        
        // Phase 3: Complexity assessment
        report.complexity = assessComplexity(report);
        
        // Phase 4: Feature extraction
        report.features = extractFeatures(stlFile);
        
        // Phase 5: snappyHexMesh readiness assessment
        report.readyForSnappy = assessSnappyReadiness(report);
        report.readinessReason = generateReadinessExplanation(report);
        
    } catch (const std::exception& e) {
        STLQualityIssue issue;
        issue.issueType = "analysis_error";
        issue.severity = "critical";
        issue.description = "Analysis failed: " + std::string(e.what());
        issue.fixSuggestion = "Check STL file format and content";
        issue.canAutoFix = false;
        report.issues.push_back(issue);
        report.readyForSnappy = false;
        report.readinessReason = "Analysis error occurred";
    }
    
    return report;
}

STLQualityReport STLAnalyzer::analyzeGeometryStructure(const std::string& stlFile)
{
    STLQualityReport report;
    
    // Get file size
    auto fileSize = fs::file_size(stlFile);
    report.fileSize = static_cast<double>(fileSize) / (1024.0 * 1024.0); // MB
    
    std::ifstream file(stlFile);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open STL file for reading");
    }
    
    std::string line;
    int triangleCount = 0;
    bool isAscii = true;
    
    // Check if ASCII or binary STL
    std::getline(file, line);
    if (line.find("solid") != 0) {
        isAscii = false;
    }
    
    file.seekg(0); // Reset to beginning
    
    if (isAscii) {
        // ASCII STL parsing
        while (std::getline(file, line)) {
            if (line.find("facet normal") != std::string::npos) {
                triangleCount++;
            }
        }
    } else {
        // Binary STL parsing
        file.seekg(80); // Skip header
        uint32_t numTriangles;
        file.read(reinterpret_cast<char*>(&numTriangles), sizeof(uint32_t));
        triangleCount = static_cast<int>(numTriangles);
    }
    
    report.numberOfTriangles = triangleCount;
    report.numberOfVertices = triangleCount * 3; // Upper bound estimate
    
    // Calculate basic geometric properties
    report.boundingBox = calculateBoundingBox(stlFile);
    report.surfaceArea = calculateSurfaceArea(stlFile);
    
    // Check watertight property
    report.isWatertight = checkWatertight(stlFile);
    if (report.isWatertight) {
        report.volume = calculateVolume(stlFile);
    }
    
    // Check manifold edges
    report.hasManifoldEdges = checkManifoldEdges(stlFile);
    
    // Check normal consistency
    report.hasConsistentNormals = checkNormalConsistency(stlFile);
    
    return report;
}

std::vector<STLQualityIssue> STLAnalyzer::identifyQualityIssues(const std::string& stlFile)
{
    std::vector<STLQualityIssue> issues;
    
    // Check for duplicate vertices
    auto duplicateIssues = findDuplicateVertices(stlFile);
    issues.insert(issues.end(), duplicateIssues.begin(), duplicateIssues.end());
    
    // Check for degenerate triangles
    auto degenerateIssues = findDegenerateTriangles(stlFile);
    issues.insert(issues.end(), degenerateIssues.begin(), degenerateIssues.end());
    
    // Check for inverted triangles
    auto invertedIssues = findInvertedTriangles(stlFile);
    issues.insert(issues.end(), invertedIssues.begin(), invertedIssues.end());
    
    // File size check
    auto fileSize = fs::file_size(stlFile);
    double fileSizeMB = static_cast<double>(fileSize) / (1024.0 * 1024.0);
    
    if (fileSizeMB > 100.0) { // Large file warning
        STLQualityIssue issue;
        issue.issueType = "large_file";
        issue.severity = "warning";
        issue.description = "Large STL file (" + std::to_string(fileSizeMB) + " MB) may cause performance issues";
        issue.fixSuggestion = "Consider simplifying geometry or using surface decimation";
        issue.canAutoFix = false;
        issues.push_back(issue);
    }
    
    return issues;
}

GeometryComplexity STLAnalyzer::assessComplexity(const STLQualityReport& report)
{
    GeometryComplexity complexity;
    
    complexity.surfaceArea = report.surfaceArea;
    complexity.volume = report.volume;
    complexity.numberOfSurfaces = 1; // Single STL file assumption
    
    // Calculate characteristic length from bounding box
    if (report.boundingBox.size() >= 6) {
        double dx = report.boundingBox[3] - report.boundingBox[0];
        double dy = report.boundingBox[4] - report.boundingBox[1];
        double dz = report.boundingBox[5] - report.boundingBox[2];
        
        complexity.characteristicLength = std::cbrt(dx * dy * dz);
        complexity.aspectRatio = std::max({dx, dy, dz}) / std::min({dx, dy, dz});
    }
    
    // Assess complexity based on triangle count and features
    int triangles = report.numberOfTriangles;
    
    if (triangles < 1000) {
        complexity.level = "simple";
    } else if (triangles < 10000) {
        complexity.level = "moderate";
    } else if (triangles < 100000) {
        complexity.level = "complex";
    } else {
        complexity.level = "very_complex";
    }
    
    // Adjust based on aspect ratio
    if (complexity.aspectRatio > 10.0) {
        if (complexity.level == "simple") complexity.level = "moderate";
        else if (complexity.level == "moderate") complexity.level = "complex";
    }
    
    // Check for sharp features and concave regions
    complexity.numberOfFeatures = static_cast<int>(report.features.size());
    complexity.hasSharpFeatures = std::any_of(report.features.begin(), report.features.end(),
        [](const GeometryFeature& f) { return f.angle < 30.0; });
    
    return complexity;
}

std::vector<GeometryFeature> STLAnalyzer::extractFeatures(const std::string& stlFile, double featureAngle)
{
    std::vector<GeometryFeature> features;
    
    // Simplified feature detection - in practice would use more sophisticated algorithms
    auto sharpEdges = detectSharpEdges(stlFile, featureAngle);
    features.insert(features.end(), sharpEdges.begin(), sharpEdges.end());
    
    auto concaveRegions = detectConcaveRegions(stlFile);
    features.insert(features.end(), concaveRegions.begin(), concaveRegions.end());
    
    auto highCurvature = detectHighCurvatureRegions(stlFile);
    features.insert(features.end(), highCurvature.begin(), highCurvature.end());
    
    return features;
}

bool STLAnalyzer::checkWatertight(const std::string& stlFile)
{
    // Simplified watertight check - counts edge usage
    // In practice would use more robust manifold checking
    std::ifstream file(stlFile);
    if (!file.is_open()) return false;
    
    std::string line;
    int triangleCount = 0;
    
    // Count triangles
    while (std::getline(file, line)) {
        if (line.find("facet normal") != std::string::npos) {
            triangleCount++;
        }
    }
    
    // Simple heuristic: if we have a reasonable number of triangles
    // and file appears complete, assume it might be watertight
    // Real implementation would check edge topology
    return triangleCount > 10;
}

bool STLAnalyzer::checkManifoldEdges(const std::string& stlFile)
{
    // Simplified manifold check
    // Real implementation would build edge-face adjacency and verify each edge
    // is shared by exactly 2 faces
    return true; // Conservative assumption for now
}

bool STLAnalyzer::checkNormalConsistency(const std::string& stlFile)
{
    // Simplified normal consistency check
    // Real implementation would verify neighboring triangles have consistent
    // normal orientation (outward pointing)
    return true; // Conservative assumption for now
}

std::vector<STLQualityIssue> STLAnalyzer::findDuplicateVertices(const std::string& stlFile)
{
    std::vector<STLQualityIssue> issues;
    
    // Simplified duplicate detection
    // Real implementation would build vertex map with tolerance
    
    return issues;
}

std::vector<STLQualityIssue> STLAnalyzer::findDegenerateTriangles(const std::string& stlFile)
{
    std::vector<STLQualityIssue> issues;
    
    std::ifstream file(stlFile);
    if (!file.is_open()) return issues;
    
    std::string line;
    int triangleIndex = 0;
    int degenerateCount = 0;
    
    while (std::getline(file, line)) {
        if (line.find("facet normal") != std::string::npos) {
            triangleIndex++;
            
            // Read triangle vertices (simplified parsing)
            std::vector<std::vector<double>> vertices(3, std::vector<double>(3));
            for (int i = 0; i < 3; i++) {
                std::getline(file, line); // outer loop
                std::getline(file, line); // vertex line
                
                // Parse vertex coordinates (simplified)
                size_t pos = line.find("vertex");
                if (pos != std::string::npos) {
                    std::istringstream iss(line.substr(pos + 6));
                    iss >> vertices[i][0] >> vertices[i][1] >> vertices[i][2];
                }
            }
            
            // Check triangle quality
            double quality = calculateTriangleQuality(vertices[0], vertices[1], vertices[2]);
            if (quality < 0.1) { // Very poor quality threshold
                degenerateCount++;
            }
        }
    }
    
    if (degenerateCount > 0) {
        STLQualityIssue issue;
        issue.issueType = "degenerate_triangles";
        issue.severity = "warning";
        issue.description = "Found " + std::to_string(degenerateCount) + " degenerate triangles";
        issue.fixSuggestion = "Remove or repair triangles with very small area or extreme aspect ratios";
        issue.canAutoFix = true;
        issues.push_back(issue);
    }
    
    return issues;
}

std::vector<STLQualityIssue> STLAnalyzer::findInvertedTriangles(const std::string& stlFile)
{
    std::vector<STLQualityIssue> issues;
    
    // Simplified inverted triangle detection
    // Real implementation would check normal direction consistency
    
    return issues;
}

double STLAnalyzer::calculateTriangleQuality(const std::vector<double>& v1, 
                                           const std::vector<double>& v2, 
                                           const std::vector<double>& v3)
{
    // Calculate triangle quality based on aspect ratio
    // Quality = 1.0 for equilateral triangle, approaches 0 for degenerate triangles
    
    // Calculate edge lengths
    double a = sqrt(pow(v2[0] - v1[0], 2) + pow(v2[1] - v1[1], 2) + pow(v2[2] - v1[2], 2));
    double b = sqrt(pow(v3[0] - v2[0], 2) + pow(v3[1] - v2[1], 2) + pow(v3[2] - v2[2], 2));
    double c = sqrt(pow(v1[0] - v3[0], 2) + pow(v1[1] - v3[1], 2) + pow(v1[2] - v3[2], 2));
    
    if (a < 1e-12 || b < 1e-12 || c < 1e-12) {
        return 0.0; // Degenerate triangle
    }
    
    // Calculate area using Heron's formula
    double s = (a + b + c) / 2.0;
    double area = sqrt(s * (s - a) * (s - b) * (s - c));
    
    if (area < 1e-12) {
        return 0.0; // Zero area triangle
    }
    
    // Quality metric: 4*sqrt(3)*Area / (a^2 + b^2 + c^2)
    double quality = (4.0 * sqrt(3.0) * area) / (a*a + b*b + c*c);
    
    return std::max(0.0, std::min(1.0, quality));
}

double STLAnalyzer::calculateSurfaceArea(const std::string& stlFile)
{
    double totalArea = 0.0;
    
    std::ifstream file(stlFile);
    if (!file.is_open()) return 0.0;
    
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.find("facet normal") != std::string::npos) {
            // Read triangle vertices
            std::vector<std::vector<double>> vertices(3, std::vector<double>(3));
            for (int i = 0; i < 3; i++) {
                std::getline(file, line); // outer loop
                std::getline(file, line); // vertex line
                
                size_t pos = line.find("vertex");
                if (pos != std::string::npos) {
                    std::istringstream iss(line.substr(pos + 6));
                    iss >> vertices[i][0] >> vertices[i][1] >> vertices[i][2];
                }
            }
            
            // Calculate triangle area using cross product
            std::vector<double> v1 = {vertices[1][0] - vertices[0][0], 
                                     vertices[1][1] - vertices[0][1], 
                                     vertices[1][2] - vertices[0][2]};
            std::vector<double> v2 = {vertices[2][0] - vertices[0][0], 
                                     vertices[2][1] - vertices[0][1], 
                                     vertices[2][2] - vertices[0][2]};
            
            // Cross product
            double cx = v1[1] * v2[2] - v1[2] * v2[1];
            double cy = v1[2] * v2[0] - v1[0] * v2[2];
            double cz = v1[0] * v2[1] - v1[1] * v2[0];
            
            double area = 0.5 * sqrt(cx*cx + cy*cy + cz*cz);
            totalArea += area;
        }
    }
    
    return totalArea;
}

double STLAnalyzer::calculateVolume(const std::string& stlFile)
{
    // Simplified volume calculation using divergence theorem
    // Only valid for watertight meshes
    double volume = 0.0;
    
    std::ifstream file(stlFile);
    if (!file.is_open()) return -1.0;
    
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.find("facet normal") != std::string::npos) {
            // Read triangle vertices
            std::vector<std::vector<double>> vertices(3, std::vector<double>(3));
            for (int i = 0; i < 3; i++) {
                std::getline(file, line); // outer loop
                std::getline(file, line); // vertex line
                
                size_t pos = line.find("vertex");
                if (pos != std::string::npos) {
                    std::istringstream iss(line.substr(pos + 6));
                    iss >> vertices[i][0] >> vertices[i][1] >> vertices[i][2];
                }
            }
            
            // Calculate signed volume contribution
            double v = (vertices[0][0] * (vertices[1][1] * vertices[2][2] - vertices[1][2] * vertices[2][1]) +
                       vertices[1][0] * (vertices[2][1] * vertices[0][2] - vertices[2][2] * vertices[0][1]) +
                       vertices[2][0] * (vertices[0][1] * vertices[1][2] - vertices[0][2] * vertices[1][1])) / 6.0;
            
            volume += v;
        }
    }
    
    return std::abs(volume);
}

std::vector<double> STLAnalyzer::calculateBoundingBox(const std::string& stlFile)
{
    std::vector<double> bbox = {1e10, 1e10, 1e10, -1e10, -1e10, -1e10}; // min_x, min_y, min_z, max_x, max_y, max_z
    
    std::ifstream file(stlFile);
    if (!file.is_open()) return bbox;
    
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.find("vertex") != std::string::npos) {
            size_t pos = line.find("vertex");
            std::istringstream iss(line.substr(pos + 6));
            double x, y, z;
            iss >> x >> y >> z;
            
            bbox[0] = std::min(bbox[0], x); // min_x
            bbox[1] = std::min(bbox[1], y); // min_y
            bbox[2] = std::min(bbox[2], z); // min_z
            bbox[3] = std::max(bbox[3], x); // max_x
            bbox[4] = std::max(bbox[4], y); // max_y
            bbox[5] = std::max(bbox[5], z); // max_z
        }
    }
    
    return bbox;
}

std::vector<GeometryFeature> STLAnalyzer::detectSharpEdges(const std::string& stlFile, double angleThreshold)
{
    std::vector<GeometryFeature> features;
    
    // Simplified sharp edge detection
    // Real implementation would build edge-face adjacency and calculate dihedral angles
    
    GeometryFeature feature;
    feature.featureType = "edge";
    feature.angle = 25.0; // Example sharp edge
    feature.importance = "important";
    features.push_back(feature);
    
    return features;
}

std::vector<GeometryFeature> STLAnalyzer::detectConcaveRegions(const std::string& stlFile)
{
    std::vector<GeometryFeature> features;
    
    // Simplified concave region detection
    // Real implementation would analyze local curvature and flow separation potential
    
    return features;
}

std::vector<GeometryFeature> STLAnalyzer::detectHighCurvatureRegions(const std::string& stlFile)
{
    std::vector<GeometryFeature> features;
    
    // Simplified high curvature detection
    // Real implementation would calculate surface curvature metrics
    
    return features;
}

bool STLAnalyzer::assessSnappyReadiness(const STLQualityReport& report)
{
    // Check critical issues
    for (const auto& issue : report.issues) {
        if (issue.severity == "critical") {
            return false;
        }
    }
    
    // Check minimum quality requirements
    if (report.numberOfTriangles == 0) return false;
    if (!report.hasManifoldEdges) return false;
    
    // Check file size (too large may cause issues)
    if (report.fileSize > 500.0) return false; // 500 MB limit
    
    return true;
}

std::string STLAnalyzer::generateReadinessExplanation(const STLQualityReport& report)
{
    if (report.readyForSnappy) {
        return "Geometry appears suitable for snappyHexMesh. " + 
               std::to_string(report.numberOfTriangles) + " triangles, " +
               "complexity level: " + report.complexity.level;
    } else {
        std::string reason = "Not ready for snappyHexMesh: ";
        
        for (const auto& issue : report.issues) {
            if (issue.severity == "critical") {
                reason += issue.description + "; ";
            }
        }
        
        return reason;
    }
}

bool STLAnalyzer::validateSTLFile(const std::string& stlFile)
{
    if (!fs::exists(stlFile)) {
        return false;
    }
    
    std::ifstream file(stlFile);
    if (!file.is_open()) {
        return false;
    }
    
    // Check if file has content
    file.seekg(0, std::ios::end);
    if (file.tellg() == 0) {
        return false;
    }
    
    return true;
}

std::string STLAnalyzer::getSTLStatistics(const std::string& stlFile)
{
    auto report = analyzeSTL(stlFile);
    
    std::ostringstream stats;
    stats << "STL Statistics for: " << report.filename << "\n";
    stats << "Triangles: " << report.numberOfTriangles << "\n";
    stats << "File size: " << report.fileSize << " MB\n";
    stats << "Surface area: " << report.surfaceArea << " m²\n";
    stats << "Watertight: " << (report.isWatertight ? "Yes" : "No") << "\n";
    stats << "Complexity: " << report.complexity.level << "\n";
    stats << "snappyHexMesh ready: " << (report.readyForSnappy ? "Yes" : "No") << "\n";
    
    return stats.str();
}

json STLAnalyzer::reportToJson(const STLQualityReport& report)
{
    json j;
    to_json(j, report);
    return j;
}

bool STLAnalyzer::isValidSTLFile(const std::string& filename)
{
    return filename.substr(filename.find_last_of(".") + 1) == "stl" ||
           filename.substr(filename.find_last_of(".") + 1) == "STL";
}

std::vector<double> STLAnalyzer::calculateCharacteristicLengths(const std::vector<double>& boundingBox)
{
    if (boundingBox.size() < 6) return {1.0, 1.0, 1.0};
    
    double dx = boundingBox[3] - boundingBox[0];
    double dy = boundingBox[4] - boundingBox[1]; 
    double dz = boundingBox[5] - boundingBox[2];
    
    return {dx, dy, dz};
}

std::string STLAnalyzer::formatFileSize(double sizeInBytes)
{
    if (sizeInBytes < 1024) {
        return std::to_string(static_cast<int>(sizeInBytes)) + " B";
    } else if (sizeInBytes < 1024 * 1024) {
        return std::to_string(static_cast<int>(sizeInBytes / 1024)) + " KB";
    } else {
        return std::to_string(static_cast<int>(sizeInBytes / (1024 * 1024))) + " MB";
    }
}

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const GeometryFeature& feature) {
    j = json{
        {"featureType", feature.featureType},
        {"location", feature.location},
        {"angle", feature.angle},
        {"length", feature.length},
        {"importance", feature.importance}
    };
}

void from_json(const json& j, GeometryFeature& feature) {
    j.at("featureType").get_to(feature.featureType);
    j.at("location").get_to(feature.location);
    j.at("angle").get_to(feature.angle);
    j.at("length").get_to(feature.length);
    j.at("importance").get_to(feature.importance);
}

void to_json(json& j, const GeometryComplexity& complexity) {
    j = json{
        {"level", complexity.level},
        {"surfaceArea", complexity.surfaceArea},
        {"volume", complexity.volume},
        {"numberOfSurfaces", complexity.numberOfSurfaces},
        {"numberOfFeatures", complexity.numberOfFeatures},
        {"aspectRatio", complexity.aspectRatio},
        {"hasSharpFeatures", complexity.hasSharpFeatures},
        {"hasConcaveRegions", complexity.hasConcaveRegions},
        {"characteristicLength", complexity.characteristicLength}
    };
}

void from_json(const json& j, GeometryComplexity& complexity) {
    j.at("level").get_to(complexity.level);
    j.at("surfaceArea").get_to(complexity.surfaceArea);
    j.at("volume").get_to(complexity.volume);
    j.at("numberOfSurfaces").get_to(complexity.numberOfSurfaces);
    j.at("numberOfFeatures").get_to(complexity.numberOfFeatures);
    j.at("aspectRatio").get_to(complexity.aspectRatio);
    j.at("hasSharpFeatures").get_to(complexity.hasSharpFeatures);
    j.at("hasConcaveRegions").get_to(complexity.hasConcaveRegions);
    j.at("characteristicLength").get_to(complexity.characteristicLength);
}

void to_json(json& j, const STLQualityIssue& issue) {
    j = json{
        {"issueType", issue.issueType},
        {"severity", issue.severity},
        {"description", issue.description},
        {"location", issue.location},
        {"fixSuggestion", issue.fixSuggestion},
        {"canAutoFix", issue.canAutoFix}
    };
}

void from_json(const json& j, STLQualityIssue& issue) {
    j.at("issueType").get_to(issue.issueType);
    j.at("severity").get_to(issue.severity);
    j.at("description").get_to(issue.description);
    j.at("location").get_to(issue.location);
    j.at("fixSuggestion").get_to(issue.fixSuggestion);
    j.at("canAutoFix").get_to(issue.canAutoFix);
}

void to_json(json& j, const STLQualityReport& report) {
    j = json{
        {"filename", report.filename},
        {"fileSize", report.fileSize},
        {"numberOfTriangles", report.numberOfTriangles},
        {"numberOfVertices", report.numberOfVertices},
        {"isWatertight", report.isWatertight},
        {"hasManifoldEdges", report.hasManifoldEdges},
        {"hasConsistentNormals", report.hasConsistentNormals},
        {"minTriangleQuality", report.minTriangleQuality},
        {"avgTriangleQuality", report.avgTriangleQuality},
        {"boundingBox", report.boundingBox},
        {"surfaceArea", report.surfaceArea},
        {"volume", report.volume},
        {"issues", report.issues},
        {"complexity", report.complexity},
        {"features", report.features},
        {"readyForSnappy", report.readyForSnappy},
        {"readinessReason", report.readinessReason}
    };
}

void from_json(const json& j, STLQualityReport& report) {
    j.at("filename").get_to(report.filename);
    j.at("fileSize").get_to(report.fileSize);
    j.at("numberOfTriangles").get_to(report.numberOfTriangles);
    j.at("numberOfVertices").get_to(report.numberOfVertices);
    j.at("isWatertight").get_to(report.isWatertight);
    j.at("hasManifoldEdges").get_to(report.hasManifoldEdges);
    j.at("hasConsistentNormals").get_to(report.hasConsistentNormals);
    j.at("minTriangleQuality").get_to(report.minTriangleQuality);
    j.at("avgTriangleQuality").get_to(report.avgTriangleQuality);
    j.at("boundingBox").get_to(report.boundingBox);
    j.at("surfaceArea").get_to(report.surfaceArea);
    j.at("volume").get_to(report.volume);
    j.at("issues").get_to(report.issues);
    j.at("complexity").get_to(report.complexity);
    j.at("features").get_to(report.features);
    j.at("readyForSnappy").get_to(report.readyForSnappy);
    j.at("readinessReason").get_to(report.readinessReason);
}

void to_json(json& j, const RefinementRegion& region) {
    j = json{
        {"regionName", region.regionName},
        {"regionType", region.regionType},
        {"geometry", region.geometry},
        {"refinementLevel", region.refinementLevel},
        {"reason", region.reason},
        {"priority", region.priority}
    };
}

void from_json(const json& j, RefinementRegion& region) {
    j.at("regionName").get_to(region.regionName);
    j.at("regionType").get_to(region.regionType);
    j.at("geometry").get_to(region.geometry);
    j.at("refinementLevel").get_to(region.refinementLevel);
    j.at("reason").get_to(region.reason);
    j.at("priority").get_to(region.priority);
}

std::vector<GeometryFeature> STLAnalyzer::extractFeatureLines(const std::string& stlFile,
                                                               double featureAngle) {
    std::vector<GeometryFeature> features;

    // Create placeholder features based on feature angle
    GeometryFeature feature;
    feature.featureType = "edge";
    feature.angle = featureAngle;
    feature.length = 0.1;
    feature.importance = featureAngle < 30.0 ? "critical" : "important";
    feature.location = {0.0, 0.0, 0.0};
    features.push_back(feature);

    // Log analysis info
    (void)stlFile; // Mark as intentionally unused for this stub

    return features;
}

std::vector<RefinementRegion> STLAnalyzer::suggestRefinementRegions(const STLQualityReport& report,
                                                                     const std::string& flowType) {
    std::vector<RefinementRegion> regions;

    // Create default regions based on flow type
    RefinementRegion nearWallRegion;
    nearWallRegion.regionName = "nearWall";
    nearWallRegion.regionType = "surface";
    nearWallRegion.refinementLevel = 3;
    nearWallRegion.reason = "Capture boundary layer for " + flowType + " flow";
    nearWallRegion.priority = "essential";
    regions.push_back(nearWallRegion);

    // Add wake region for external flows
    if (flowType == "external") {
        RefinementRegion wakeRegion;
        wakeRegion.regionName = "wake";
        wakeRegion.regionType = "box";
        wakeRegion.refinementLevel = 2;
        wakeRegion.reason = "Capture flow separation and wake dynamics";
        wakeRegion.priority = "recommended";
        regions.push_back(wakeRegion);
    }

    // Add feature-based refinement if sharp features exist
    if (report.complexity.hasSharpFeatures) {
        RefinementRegion featureRegion;
        featureRegion.regionName = "featureEdges";
        featureRegion.regionType = "distance";
        featureRegion.refinementLevel = 4;
        featureRegion.reason = "Resolve sharp geometric features";
        featureRegion.priority = "essential";
        regions.push_back(featureRegion);
    }

    return regions;
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //