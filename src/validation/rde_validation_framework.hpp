#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>
#include "../openfoam/rotating_detonation_engine.hpp"
#include "../openfoam/cellular_detonation_model.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Class RDEValidationFramework
\*---------------------------------------------------------------------------*/

class RDEValidationFramework {
public:
    
    // Constructor
    RDEValidationFramework();
    
    // Destructor
    ~RDEValidationFramework() = default;
    
    // Validation case definition
    struct ValidationCase {
        std::string name;
        std::string description;
        std::string source;         // "NASA_Glenn", "Purdue_DRONE", "NRL", "Analytical"
        std::string fuelType;
        double pressure;            // [Pa]
        double temperature;         // [K]
        double equivalenceRatio;    // [-]
        
        // Expected results
        double expectedCJVelocity;  // [m/s]
        double expectedCellSize;    // [m]
        double expectedPressureRatio; // [-]
        double expectedFrequency;   // [Hz]
        
        // Validation tolerances
        double velocityTolerance;   // [%]
        double cellSizeTolerance;   // [%]
        double pressureTolerance;   // [%]
        double frequencyTolerance;  // [%]
        
        // Geometry for full RDE tests
        RDEGeometry geometry;
        RDESimulationSettings settings;
        
        // Validation type
        enum class Type {
            ANALYTICAL_1D,          // 1D Chapman-Jouguet validation
            CELLULAR_STRUCTURE,     // Cell size prediction validation
            DETONATION_TUBE,        // 1D detonation tube
            WEDGE_GEOMETRY,         // 2D wedge detonation
            SIMPLIFIED_RDE,         // Simplified annular RDE
            FULL_RDE               // Complete RDE validation
        } type;
    };
    
    // Validation result
    struct ValidationResult {
        std::string caseName;
        bool passed;
        
        // Predicted values
        double predictedCJVelocity;
        double predictedCellSize;
        double predictedPressureRatio;
        double predictedFrequency;
        
        // Errors
        double velocityError;       // [%]
        double cellSizeError;       // [%]
        double pressureError;       // [%]
        double frequencyError;      // [%]
        
        // Overall metrics
        double overallAccuracy;     // [%]
        std::vector<std::string> warnings;
        std::vector<std::string> notes;
        
        // Detailed results
        std::map<std::string, double> detailedMetrics;
        std::string logFile;
        std::string dataFile;
    };
    
    // Validation suite result
    struct ValidationSuiteResult {
        std::string suiteName;
        int totalCases;
        int passedCases;
        int failedCases;
        double overallAccuracy;
        
        std::vector<ValidationResult> results;
        std::map<std::string, double> suiteMetrics;
        std::string summaryReport;
        
        // Performance metrics
        double averageVelocityError;
        double averageCellSizeError;
        double averagePressureError;
        double averageFrequencyError;
    };
    
    // Main validation interface
    ValidationResult validateSingleCase(const ValidationCase& testCase);
    ValidationSuiteResult validateSuite(const std::vector<ValidationCase>& testCases);
    
    // Predefined validation suites
    std::vector<ValidationCase> createAnalyticalValidationSuite();
    std::vector<ValidationCase> createCellularValidationSuite();
    std::vector<ValidationCase> createExperimentalValidationSuite();
    std::vector<ValidationCase> createCompleteValidationSuite();
    
    // Expert-level validation cases
    std::vector<ValidationCase> createNASAGlennValidationCases();
    std::vector<ValidationCase> createPurdueDRONEValidationCases();
    std::vector<ValidationCase> createNRLValidationCases();
    
    // Validation execution modes
    enum class ValidationMode {
        FAST,           // Quick analytical validation
        STANDARD,       // Standard validation with cellular model
        COMPREHENSIVE,  // Full CFD validation
        EXPERT         // Expert-level validation with experimental data
    };
    
    ValidationSuiteResult runValidation(ValidationMode mode);
    
    // Reporting and analysis
    void generateValidationReport(const ValidationSuiteResult& result, const std::string& outputFile);
    void generateComparisonPlots(const ValidationSuiteResult& result, const std::string& outputDir);
    void exportValidationData(const ValidationSuiteResult& result, const std::string& outputFile);
    
    // Statistical analysis
    struct ValidationStatistics {
        double meanError;
        double standardDeviation;
        double maxError;
        double minError;
        double r2Coefficient;
        std::vector<double> errorDistribution;
    };
    
    ValidationStatistics calculateStatistics(const std::vector<ValidationResult>& results, 
                                           const std::string& metric);
    
    // Uncertainty quantification
    struct UncertaintyAnalysis {
        double modelUncertainty;
        double experimentalUncertainty;
        double combinedUncertainty;
        double confidenceInterval95;
        std::vector<double> sensitivityFactors;
    };
    
    UncertaintyAnalysis performUncertaintyAnalysis(const ValidationCase& testCase);
    
    // Calibration and improvement
    struct CalibrationResult {
        std::vector<double> originalParameters;
        std::vector<double> calibratedParameters;
        double improvementFactor;
        ValidationSuiteResult beforeCalibration;
        ValidationSuiteResult afterCalibration;
    };
    
    CalibrationResult calibrateModel(const std::vector<ValidationCase>& calibrationCases);
    
    // Configuration
    void setValidationParameters(const json& config);
    void setOutputDirectory(const std::string& dir);
    void setVerboseMode(bool verbose);
    
    // Utilities
    bool loadExperimentalData(const std::string& dataFile);
    bool loadValidationCases(const std::string& caseFile);
    void saveValidationCases(const std::vector<ValidationCase>& cases, const std::string& caseFile);
    
private:
    
    // Core components
    std::unique_ptr<RotatingDetonationEngine> rdeEngine_;
    std::unique_ptr<CellularDetonationModel> cellularModel_;
    
    // Configuration
    std::string outputDirectory_;
    bool verboseMode_;
    json validationConfig_;
    
    // Experimental data
    std::vector<CellularDetonationModel::ValidationData> experimentalData_;
    
    // Validation methods
    ValidationResult validateAnalytical1D(const ValidationCase& testCase);
    ValidationResult validateCellularStructure(const ValidationCase& testCase);
    ValidationResult validateDetonationTube(const ValidationCase& testCase);
    ValidationResult validateWedgeGeometry(const ValidationCase& testCase);
    ValidationResult validateSimplifiedRDE(const ValidationCase& testCase);
    ValidationResult validateFullRDE(const ValidationCase& testCase);
    
    // Helper methods
    double calculateChapmanJouguetVelocity(const std::string& fuel, double pressure, 
                                          double temperature, double phi);
    double calculateTheoreticalCellSize(const std::string& fuel, double pressure, 
                                       double temperature, double phi);
    double calculatePressureRatio(const std::string& fuel, double phi);
    double calculateFrequency(double velocity, double cellSize);
    
    // Error calculation
    double calculateRelativeError(double predicted, double expected);
    double calculateOverallAccuracy(const std::vector<double>& errors);
    
    // Reporting utilities
    void writeValidationLog(const ValidationResult& result, const std::string& logFile);
    void writeValidationData(const ValidationResult& result, const std::string& dataFile);
    std::string generateSummaryReport(const ValidationSuiteResult& result);
    
    // Statistical utilities
    double calculateMean(const std::vector<double>& data);
    double calculateStandardDeviation(const std::vector<double>& data);
    double calculateR2Coefficient(const std::vector<double>& predicted, 
                                 const std::vector<double>& expected);
    
    // Default validation cases
    void initializeDefaultCases();
    void loadExperimentalDatabase();
    
    // Constants
    static constexpr double UNIVERSAL_GAS_CONSTANT = 8314.0;
    static constexpr double REFERENCE_PRESSURE = 101325.0;
    static constexpr double REFERENCE_TEMPERATURE = 298.15;
    static constexpr double DEFAULT_TOLERANCE = 0.05; // 5%
    static constexpr double EXPERT_TOLERANCE = 0.02;  // 2%
};

} // namespace MCP
} // namespace Foam