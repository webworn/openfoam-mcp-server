#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <chrono>
#include <list>
#include <nlohmann/json.hpp>

namespace Foam {
namespace MCP {
namespace Physics {

/*---------------------------------------------------------------------------*\
                        Class CanteraWrapper
\*---------------------------------------------------------------------------*/

/**
 * @brief Cantera integration wrapper for Chapman-Jouguet calculations
 * 
 * This class provides a bridge to Cantera thermodynamics calculations,
 * supporting both Python subprocess and native C++ API approaches.
 * The implementation follows the proven SDToolbox methodology for
 * accurate detonation calculations.
 */
class CanteraWrapper {
public:
    
    // Constructor
    explicit CanteraWrapper();
    
    // Destructor
    ~CanteraWrapper() = default;
    
    /**
     * @brief Thermodynamic state representation (SDT-compatible)
     */
    struct ThermodynamicState {
        double pressure;        // [Pa]
        double temperature;     // [K]
        double density;         // [kg/m³]
        double enthalpy;        // [J/kg]
        double entropy;         // [J/kg/K]
        double soundSpeed;      // [m/s]
        double meanMolecularWeight; // [kg/mol]
        double heatCapacityP;   // [J/kg/K]
        double heatCapacityV;   // [J/kg/K]
        
        // Composition (mole fractions)
        std::map<std::string, double> composition;
        
        // JSON serialization
        nlohmann::json toJson() const;
        static ThermodynamicState fromJson(const nlohmann::json& j);
    };
    
    /**
     * @brief Conservation equation errors (SDT FHFP equivalent)
     */
    struct ConservationErrors {
        double enthalpyError;   // FH: Energy conservation error
        double pressureError;   // FP: Momentum conservation error
        
        nlohmann::json toJson() const;
        static ConservationErrors fromJson(const nlohmann::json& j);
    };
    
    /**
     * @brief Initial conditions for calculations
     */
    struct InitialConditions {
        double pressure;           // [Pa]
        double temperature;        // [K]
        std::string composition;   // Cantera format: "H2:2 O2:1 N2:3.76"
        std::string mechanism;     // Mechanism file path
        
        nlohmann::json toJson() const;
        static InitialConditions fromJson(const nlohmann::json& j);
    };
    
    // Core SDT-compatible functions
    
    /**
     * @brief Calculate Chapman-Jouguet detonation speed
     * Direct wrapper around SDT CJspeed function
     */
    double calculateCJSpeed(double pressure, double temperature, 
                           const std::string& composition, 
                           const std::string& mechanism);
    
    /**
     * @brief Calculate equilibrium post-shock state
     * Wrapper around SDT PostShock_eq function
     */
    ThermodynamicState calculatePostShockEq(double shockSpeed,
                                           double pressure, double temperature,
                                           const std::string& composition,
                                           const std::string& mechanism);
    
    /**
     * @brief Calculate equilibrium state at given density and temperature
     * Wrapper around SDT eq_state function
     */
    ThermodynamicState calculateEquilibriumState(double density, double temperature,
                                                const std::string& composition,
                                                const std::string& mechanism);
    
    /**
     * @brief Calculate frozen state at given density and temperature  
     * Wrapper around SDT state function (no equilibration)
     */
    ThermodynamicState calculateFrozenState(double density, double temperature,
                                           const std::string& composition,
                                           const std::string& mechanism);
    
    /**
     * @brief Calculate conservation equation errors
     * Implementation of SDT FHFP function
     */
    ConservationErrors calculateFHFP(double waveSpeed,
                                    const ThermodynamicState& state1,
                                    const ThermodynamicState& state2);
    
    /**
     * @brief Calculate sound speed using equilibrium method
     * Implementation of SDT soundspeed_eq with thermodynamic identities
     */
    double calculateSoundSpeedEq(const ThermodynamicState& state,
                                const std::string& mechanism);
    
    // Batch processing for efficiency
    
    /**
     * @brief Calculate CJ speeds for multiple conditions
     */
    std::vector<double> calculateCJSpeedBatch(const std::vector<InitialConditions>& conditions);
    
    /**
     * @brief Calculate multiple equilibrium states
     */
    std::vector<ThermodynamicState> calculateEquilibriumStateBatch(
        const std::vector<double>& densities,
        const std::vector<double>& temperatures,
        const std::string& composition,
        const std::string& mechanism);
    
    // Configuration and utilities
    
    /**
     * @brief Set Python interpreter path
     */
    void setPythonPath(const std::string& pythonPath);
    
    /**
     * @brief Set SDToolbox directory path
     */
    void setSDToolboxPath(const std::string& sdtPath);
    
    /**
     * @brief Set working directory for calculations
     */
    void setWorkingDirectory(const std::string& workingDir);
    
    /**
     * @brief Enable/disable verbose output
     */
    void setVerboseMode(bool verbose);
    
    /**
     * @brief Set subprocess timeout (milliseconds)
     */
    void setTimeout(int timeoutMs);
    
    /**
     * @brief Test connection to Python SDToolbox
     */
    bool testSDToolboxConnection();
    
    /**
     * @brief Validate mechanism file
     */
    bool validateMechanism(const std::string& mechanism);
    
    // Utility functions
    
    /**
     * @brief Convert equivalence ratio and fuel type to composition string
     */
    static std::string generateComposition(const std::string& fuelType, 
                                         double equivalenceRatio,
                                         const std::string& oxidizerType = "air");
    
    /**
     * @brief Parse composition string to mole fraction map
     */
    static std::map<std::string, double> parseComposition(const std::string& composition);
    
    /**
     * @brief Get available mechanisms
     */
    std::vector<std::string> getAvailableMechanisms();
    
    // Forward declarations for validation framework
    struct ValidationBenchmark;
    struct AccuracyMetrics;
    struct ValidationReport;
    
    // Performance monitoring
    struct PerformanceMetrics {
        unsigned int totalExecutions;
        double totalExecutionTime;  // milliseconds
        double averageExecutionTime;
        double minExecutionTime;
        double maxExecutionTime;
        std::chrono::system_clock::time_point lastExecution;
        
        nlohmann::json toJson() const;
        static PerformanceMetrics fromJson(const nlohmann::json& j);
    };
    
    // Input validation
    void validateThermodynamicInputs(double pressure, double temperature, 
                                   const std::string& composition) const;
    void validatePhysicalBounds(double pressure, double temperature) const;
    bool isCompositionValid(const std::string& composition) const;
    
    // Statistics
    unsigned int getTotalCalls() const { return totalCalls_; }
    unsigned int getSuccessfulCalls() const { return successfulCalls_; }
    unsigned int getFailedCalls() const { return failedCalls_; }
    unsigned int getRetryAttempts() const { return retryAttempts_; }
    unsigned int getTimeoutFailures() const { return timeoutFailures_; }
    unsigned int getValidationFailures() const { return validationFailures_; }
    double getSuccessRate() const { 
        return totalCalls_ > 0 ? static_cast<double>(successfulCalls_) / totalCalls_ : 0.0; 
    }
    std::map<std::string, unsigned int> getErrorStatistics() const { return errorTypeCount_; }
    
    // Validation framework methods
    void enableValidation(bool enable = true) { validationEnabled_ = enable; }
    bool isValidationEnabled() const { return validationEnabled_; }
    void addValidationBenchmark(const ValidationBenchmark& benchmark);
    ValidationReport runValidationSuite(bool verbose = false);
    ValidationReport runSingleValidation(const std::string& benchmarkName, bool verbose = false);
    std::vector<ValidationBenchmark> getStandardBenchmarks() const;
    AccuracyMetrics getLastAccuracyMetric(const std::string& testCase) const;
    double getCurrentAccuracyScore() const { return totalAccuracyScore_; }
    void loadValidationBenchmarks(const std::string& filePath);
    void saveValidationReport(const ValidationReport& report, const std::string& filePath) const;
    
    // Performance optimization methods
    void enableCaching(bool enable = true, const std::string& cacheFile = "");
    bool isCachingEnabled() const { return cachingEnabled_; }
    void clearCache();
    void saveCacheToFile() const;
    void loadCacheFromFile();
    unsigned int getCacheHits() const { return cacheHits_; }
    unsigned int getCacheMisses() const { return cacheMisses_; }
    double getCacheHitRate() const { 
        unsigned int total = cacheHits_ + cacheMisses_;
        return total > 0 ? static_cast<double>(cacheHits_) / total : 0.0; 
    }
    PerformanceMetrics getPerformanceMetrics() const { return performanceMetrics_; }
    void resetPerformanceMetrics();
    
    // Custom exception classes
    class ThermodynamicException : public std::runtime_error {
    public:
        explicit ThermodynamicException(const std::string& msg) : std::runtime_error("Thermodynamic error: " + msg) {}
    };
    
    class ValidationException : public std::runtime_error {
    public:
        explicit ValidationException(const std::string& msg) : std::runtime_error("Validation error: " + msg) {}
    };
    
    class SubprocessException : public std::runtime_error {
    public:
        explicit SubprocessException(const std::string& msg) : std::runtime_error("Subprocess error: " + msg) {}
    };
    
    class TimeoutException : public std::runtime_error {
    public:
        explicit TimeoutException(const std::string& msg) : std::runtime_error("Timeout error: " + msg) {}
    };
    
    
private:
    
    // Python subprocess management
    
    /**
     * @brief Execute Python script with JSON input/output
     */
    nlohmann::json executePythonScript(const std::string& script, 
                                      const nlohmann::json& input);
    
    /**
     * @brief Execute Python script with automatic retry logic
     */
    nlohmann::json executePythonScriptWithRetry(const std::string& script, 
                                               const nlohmann::json& input, 
                                               int maxRetries = 3);
    
    /**
     * @brief Execute batch Python script
     */
    nlohmann::json executePythonScriptBatch(const std::string& script,
                                           const std::vector<nlohmann::json>& inputs);
    
    /**
     * @brief Validate JSON response from Python
     */
    void validatePythonResponse(const nlohmann::json& response);
    
    /**
     * @brief Handle Python execution errors
     */
    void handlePythonError(const std::string& errorMessage, 
                          const nlohmann::json& input);
    
    // Internal utilities
    
    /**
     * @brief Find mechanism file in standard locations
     */
    std::string findMechanismFile(const std::string& mechanism);
    
    /**
     * @brief Initialize default paths
     */
    void initializeDefaultPaths();
    
    /**
     * @brief Initialize standard validation benchmarks
     */
    void initializeStandardBenchmarks();
    
    /**
     * @brief Log debug information
     */
    void logDebug(const std::string& message) const;
    
    /**
     * @brief Log error information with categorization
     */
    void logError(const std::string& errorType, const std::string& message) const;
    
    // Reliability helpers
    void recordError(const std::string& errorType, const std::string& message);
    bool shouldRetry(const std::exception& e, int attemptCount) const;
    int calculateBackoffDelay(int attemptCount) const;
    
    // Validation helpers
    void recordAccuracyMetric(const AccuracyMetrics& metric);
    bool validateCalculationAccuracy(double calculated, double expected, double tolerance) const;
    ValidationBenchmark createBenchmark(const std::string& name, const std::string& description,
                                       double pressure, double temperature, const std::string& composition,
                                       const std::string& mechanism, double expectedCJSpeed, 
                                       double tolerance, const std::string& reference) const;
    void updateAccuracyScore();
    
    // Caching helper methods
    std::string generateCacheKey(const std::string& function, double pressure, 
                                double temperature, const std::string& composition,
                                const std::string& mechanism) const;
    bool getCachedResult(const std::string& cacheKey, double& result) const;
    void setCachedResult(const std::string& cacheKey, double result) const;
    void updateLRU(const std::string& cacheKey) const;
    void evictLRU() const;
    void recordPerformance(double executionTime) const;
    
    /**
     * @brief Format composition string for SDT
     */
    std::string formatCompositionForSDT(const std::string& composition);
    
    // Configuration
    std::string pythonPath_;        // Path to Python interpreter
    std::string sdtPath_;           // Path to SDToolbox directory
    std::string workingDir_;        // Working directory for calculations
    std::string mechanismDir_;      // Directory containing mechanism files
    bool verboseMode_;              // Enable verbose output
    int timeoutMs_;                 // Subprocess timeout in milliseconds
    
    // Process management
    mutable std::map<std::string, std::string> processCache_; // Cache for repeated calls
    bool useBatchProcessing_;       // Enable batch processing optimization
    int maxBatchSize_;              // Maximum batch size for efficiency
    
    // Error tracking
    mutable int totalCalls_;        // Total number of calls
    mutable int successfulCalls_;   // Successful calls
    mutable int failedCalls_;       // Failed calls
    mutable std::vector<std::string> recentErrors_; // Recent error messages
    
    // Reliability tracking
    mutable unsigned int retryAttempts_;
    mutable unsigned int timeoutFailures_;
    mutable unsigned int validationFailures_;
    
    // Error statistics by type
    mutable std::map<std::string, unsigned int> errorTypeCount_;
    
    // Validation framework
    mutable std::vector<ValidationBenchmark> validationBenchmarks_;
    mutable std::map<std::string, AccuracyMetrics> accuracyHistory_;
    mutable double totalAccuracyScore_;
    mutable bool validationEnabled_;
    
    // Performance optimization - Intelligent caching system
    struct CachedResult {
        double result;
        std::chrono::system_clock::time_point timestamp;
        std::string inputHash;
        int accessCount;
        
        nlohmann::json toJson() const;
        static CachedResult fromJson(const nlohmann::json& j);
    };
    
    mutable std::map<std::string, CachedResult> resultCache_;
    mutable std::list<std::string> lruOrder_;
    mutable unsigned int cacheHits_;
    mutable unsigned int cacheMisses_;
    mutable bool cachingEnabled_;
    mutable std::string cacheFilePath_;
    
    mutable PerformanceMetrics performanceMetrics_;
    
    // Default configuration
    static constexpr int DEFAULT_TIMEOUT_MS = 30000;  // 30 seconds
    static constexpr int DEFAULT_MAX_BATCH_SIZE = 50;
    static constexpr const char* DEFAULT_PYTHON_PATH = "python3";
    static constexpr const char* BRIDGE_SCRIPT_NAME = "sdt_bridge.py";
    static constexpr int MAX_RETRY_ATTEMPTS = 3;
    static constexpr int INITIAL_BACKOFF_MS = 100;
    
    // Physical bounds for validation
    static constexpr double MIN_PRESSURE = 1000.0;      // 1 kPa
    static constexpr double MAX_PRESSURE = 1.0e8;       // 100 MPa
    static constexpr double MIN_TEMPERATURE = 200.0;    // 200 K
    static constexpr double MAX_TEMPERATURE = 6000.0;   // 6000 K
    
    // Validation framework constants
    static constexpr double DEFAULT_TOLERANCE = 0.05;   // 5% tolerance
    static constexpr int MAX_ACCURACY_HISTORY = 100;    // Maximum stored accuracy metrics
    static constexpr double MIN_ACCURACY_SCORE = 0.95;  // 95% minimum accuracy
    
    // Performance optimization constants
    static constexpr int MAX_CACHE_SIZE = 1000;         // Maximum cached results
    static constexpr double CACHE_TOLERANCE = 1e-6;     // Cache key tolerance
    static constexpr const char* DEFAULT_CACHE_FILE = "cantera_cache.json";
    
    // Standard mechanism names (relative to mechanism directory)
    static const std::vector<std::string> STANDARD_MECHANISMS;
    
    // JSON communication protocol version
    static constexpr const char* PROTOCOL_VERSION = "1.0";
};

// Validation framework structures (defined outside class for proper forward declaration)
struct CanteraWrapper::ValidationBenchmark {
    std::string name;
    std::string description;
    double pressure;        // [Pa]
    double temperature;     // [K]
    std::string composition;
    std::string mechanism;
    double expectedCJSpeed; // [m/s]
    double tolerance;       // Relative tolerance (e.g., 0.05 = 5%)
    std::string reference;  // Literature or experimental reference
    
    nlohmann::json toJson() const;
    static ValidationBenchmark fromJson(const nlohmann::json& j);
};

struct CanteraWrapper::AccuracyMetrics {
    std::string testCase;
    double calculatedValue;
    double expectedValue;
    double absoluteError;
    double relativeError;   // As percentage
    bool withinTolerance;
    std::chrono::system_clock::time_point timestamp;
    int executionTimeMs;
    
    nlohmann::json toJson() const;
    static AccuracyMetrics fromJson(const nlohmann::json& j);
};

struct CanteraWrapper::ValidationReport {
    int totalTests;
    int passedTests;
    int failedTests;
    double overallAccuracy; // Percentage
    double averageError;    // Average relative error
    std::vector<AccuracyMetrics> results;
    std::chrono::system_clock::time_point reportTime;
    
    nlohmann::json toJson() const;
    static ValidationReport fromJson(const nlohmann::json& j);
};


} // namespace Physics
} // namespace MCP
} // namespace Foam