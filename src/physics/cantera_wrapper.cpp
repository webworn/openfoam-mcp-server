#include "cantera_wrapper.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <thread>
#include <iomanip>
#include <cmath>
#include <limits>
#include <functional>
#include <list>

using json = nlohmann::json;

namespace Foam {
namespace MCP {
namespace Physics {

// Static member initialization
const std::vector<std::string> CanteraWrapper::STANDARD_MECHANISMS = {
    "sandiego20161214_H2only.cti",
    "Burke2012.cti", 
    "Keromnes2013.cti",
    "Mevel2017.cti",
    "gri30_highT.cti"
};

/*---------------------------------------------------------------------------*\
                    Class CanteraWrapper Implementation
\*---------------------------------------------------------------------------*/

CanteraWrapper::CanteraWrapper()
:   pythonPath_(DEFAULT_PYTHON_PATH),
    verboseMode_(false),
    timeoutMs_(DEFAULT_TIMEOUT_MS),
    useBatchProcessing_(true),
    maxBatchSize_(DEFAULT_MAX_BATCH_SIZE),
    totalCalls_(0),
    successfulCalls_(0),
    failedCalls_(0),
    retryAttempts_(0),
    timeoutFailures_(0),
    validationFailures_(0),
    totalAccuracyScore_(0.0),
    validationEnabled_(false),
    cacheHits_(0),
    cacheMisses_(0),
    cachingEnabled_(false),
    cacheFilePath_(DEFAULT_CACHE_FILE)
{
    initializeDefaultPaths();
    
    // Initialize performance metrics
    performanceMetrics_ = {};
    performanceMetrics_.totalExecutions = 0;
    performanceMetrics_.totalExecutionTime = 0.0;
    performanceMetrics_.averageExecutionTime = 0.0;
    performanceMetrics_.minExecutionTime = std::numeric_limits<double>::max();
    performanceMetrics_.maxExecutionTime = 0.0;
    
    // Initialize standard validation benchmarks
    initializeStandardBenchmarks();
}

void CanteraWrapper::initializeDefaultPaths()
{
    // Find project root
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path projectRoot;
    
    // Search upward for openfoam-mcp-server directory
    for (auto path = currentPath; path != path.root_path(); path = path.parent_path()) {
        if (path.filename() == "openfoam-mcp-server") {
            projectRoot = path;
            break;
        }
    }
    
    if (projectRoot.empty()) {
        projectRoot = currentPath;
    }
    
    // Set default paths
    sdtPath_ = (projectRoot / "RDE" / "data" / "SDToolbox" / "Python3").string();
    mechanismDir_ = (projectRoot / "RDE" / "data" / "SDToolbox" / "cti").string();
    workingDir_ = (projectRoot / "src" / "physics").string();
    
    logDebug("Initialized paths:");
    logDebug("  Project root: " + projectRoot.string());
    logDebug("  SDT path: " + sdtPath_);
    logDebug("  Mechanism dir: " + mechanismDir_);
    logDebug("  Working dir: " + workingDir_);
}

// JSON serialization implementations

json CanteraWrapper::ThermodynamicState::toJson() const
{
    json j;
    j["pressure"] = pressure;
    j["temperature"] = temperature;
    j["density"] = density;
    j["enthalpy"] = enthalpy;
    j["entropy"] = entropy;
    j["sound_speed"] = soundSpeed;
    j["mean_molecular_weight"] = meanMolecularWeight;
    j["cp"] = heatCapacityP;
    j["cv"] = heatCapacityV;
    j["composition"] = composition;
    return j;
}

CanteraWrapper::ThermodynamicState CanteraWrapper::ThermodynamicState::fromJson(const json& j)
{
    ThermodynamicState state;
    state.pressure = j.at("pressure");
    state.temperature = j.at("temperature");
    state.density = j.at("density");
    state.enthalpy = j.at("enthalpy");
    state.entropy = j.at("entropy");
    state.soundSpeed = j.value("sound_speed", 0.0);
    state.meanMolecularWeight = j.value("mean_molecular_weight", 0.0);
    state.heatCapacityP = j.value("cp", 0.0);
    state.heatCapacityV = j.value("cv", 0.0);
    
    if (j.contains("composition")) {
        state.composition = j["composition"];
    }
    
    return state;
}

json CanteraWrapper::ConservationErrors::toJson() const
{
    json j;
    j["enthalpy_error"] = enthalpyError;
    j["pressure_error"] = pressureError;
    return j;
}

CanteraWrapper::ConservationErrors CanteraWrapper::ConservationErrors::fromJson(const json& j)
{
    ConservationErrors errors;
    errors.enthalpyError = j.at("enthalpy_error");
    errors.pressureError = j.at("pressure_error");
    return errors;
}

json CanteraWrapper::InitialConditions::toJson() const
{
    json j;
    j["pressure"] = pressure;
    j["temperature"] = temperature;
    j["composition"] = composition;
    j["mechanism"] = mechanism;
    return j;
}

CanteraWrapper::InitialConditions CanteraWrapper::InitialConditions::fromJson(const json& j)
{
    InitialConditions conditions;
    conditions.pressure = j.at("pressure");
    conditions.temperature = j.at("temperature");
    conditions.composition = j.at("composition");
    conditions.mechanism = j.at("mechanism");
    return conditions;
}

// Core SDT-compatible functions

double CanteraWrapper::calculateCJSpeed(double pressure, double temperature,
                                       const std::string& composition,
                                       const std::string& mechanism)
{
    totalCalls_++;
    auto startTime = std::chrono::steady_clock::now();
    
    try {
        // Validate inputs
        validateThermodynamicInputs(pressure, temperature, composition);
        
        // Check cache first if enabled
        if (cachingEnabled_) {
            std::string cacheKey = generateCacheKey("CJspeed", pressure, temperature, composition, mechanism);
            double cachedResult;
            if (getCachedResult(cacheKey, cachedResult)) {
                auto endTime = std::chrono::steady_clock::now();
                double executionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000.0;
                recordPerformance(executionTime);
                logDebug("CJ speed retrieved from cache: " + std::to_string(cachedResult) + " m/s");
                return cachedResult;
            }
        }
        
        // Prepare request
        json request;
        request["function"] = "CJspeed";
        request["P1"] = pressure;
        request["T1"] = temperature;
        request["q"] = formatCompositionForSDT(composition);
        request["mech"] = mechanism;
        request["fullOutput"] = false;
        
        logDebug("Calculating CJ speed for: " + composition + " at " + 
                std::to_string(pressure) + " Pa, " + std::to_string(temperature) + " K");
        
        // Execute Python script with retry logic
        json response = executePythonScriptWithRetry(BRIDGE_SCRIPT_NAME, request);
        
        // Validate response
        validatePythonResponse(response);
        
        // Extract result
        double cjSpeed = response["result"]["cj_speed"];
        
        // Validate physical bounds for result
        if (cjSpeed <= 0.0 || cjSpeed > 10000.0) {
            throw ValidationException("CJ speed out of physical bounds: " + std::to_string(cjSpeed) + " m/s");
        }
        
        // Cache result if enabled
        if (cachingEnabled_) {
            std::string cacheKey = generateCacheKey("CJspeed", pressure, temperature, composition, mechanism);
            setCachedResult(cacheKey, cjSpeed);
        }
        
        successfulCalls_++;
        
        // Record performance
        auto endTime = std::chrono::steady_clock::now();
        double executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        recordPerformance(executionTime);
        
        logDebug("CJ speed calculated: " + std::to_string(cjSpeed) + " m/s (" + 
                std::to_string(executionTime) + "ms)");
        
        return cjSpeed;
        
    } catch (const ValidationException& e) {
        failedCalls_++;
        validationFailures_++;
        recordError("ValidationError", e.what());
        throw;
    } catch (const TimeoutException& e) {
        failedCalls_++;
        timeoutFailures_++;
        recordError("TimeoutError", e.what());
        throw;
    } catch (const SubprocessException& e) {
        failedCalls_++;
        recordError("SubprocessError", e.what());
        throw;
    } catch (const ThermodynamicException& e) {
        failedCalls_++;
        recordError("ThermodynamicError", e.what());
        throw;
    } catch (const std::exception& e) {
        failedCalls_++;
        std::string error = "CJ speed calculation failed: " + std::string(e.what());
        recordError("UnknownError", error);
        throw std::runtime_error(error);
    }
}

CanteraWrapper::ThermodynamicState CanteraWrapper::calculatePostShockEq(
    double shockSpeed, double pressure, double temperature,
    const std::string& composition, const std::string& mechanism)
{
    totalCalls_++;
    
    try {
        // Validate inputs
        validateThermodynamicInputs(pressure, temperature, composition);
        
        if (shockSpeed <= 0.0 || shockSpeed > 10000.0) {
            throw ValidationException("Shock speed out of bounds: " + std::to_string(shockSpeed) + " m/s");
        }
        
        // Prepare request
        json request;
        request["function"] = "PostShock_eq";
        request["U1"] = shockSpeed;
        request["P1"] = pressure;
        request["T1"] = temperature;
        request["q"] = formatCompositionForSDT(composition);
        request["mech"] = mechanism;
        
        logDebug("Calculating post-shock equilibrium state");
        
        // Execute Python script with retry logic
        json response = executePythonScriptWithRetry(BRIDGE_SCRIPT_NAME, request);
        
        // Validate response
        validatePythonResponse(response);
        
        // Extract result
        ThermodynamicState state = ThermodynamicState::fromJson(response["result"]);
        
        // Validate result
        if (state.pressure <= 0.0 || state.temperature <= 0.0 || state.density <= 0.0) {
            throw ValidationException("Invalid thermodynamic state calculated");
        }
        
        successfulCalls_++;
        return state;
        
    } catch (const ValidationException& e) {
        failedCalls_++;
        validationFailures_++;
        recordError("ValidationError", e.what());
        throw;
    } catch (const TimeoutException& e) {
        failedCalls_++;
        timeoutFailures_++;
        recordError("TimeoutError", e.what());
        throw;
    } catch (const std::exception& e) {
        failedCalls_++;
        recordError("PostShockError", e.what());
        throw std::runtime_error("Post-shock equilibrium calculation failed: " + std::string(e.what()));
    }
}

CanteraWrapper::ThermodynamicState CanteraWrapper::calculateEquilibriumState(
    double density, double temperature, const std::string& composition, const std::string& mechanism)
{
    totalCalls_++;
    
    try {
        // Validate inputs
        validatePhysicalBounds(101325.0, temperature);  // Use standard pressure for validation
        
        if (density <= 0.0 || density > 1000.0) {
            throw ValidationException("Density out of bounds: " + std::to_string(density) + " kg/m³");
        }
        
        if (!isCompositionValid(composition)) {
            throw ValidationException("Invalid composition format: " + composition);
        }
        
        // Prepare request
        json request;
        request["function"] = "eq_state";
        request["density"] = density;
        request["temperature"] = temperature;
        request["composition"] = formatCompositionForSDT(composition);
        request["mechanism"] = mechanism;
        
        logDebug("Calculating equilibrium state at rho=" + std::to_string(density) + 
                " kg/m³, T=" + std::to_string(temperature) + " K");
        
        // Execute Python script with retry logic
        json response = executePythonScriptWithRetry(BRIDGE_SCRIPT_NAME, request);
        
        // Validate response
        validatePythonResponse(response);
        
        // Extract result
        ThermodynamicState state = ThermodynamicState::fromJson(response["result"]);
        
        // Validate result
        if (state.pressure <= 0.0 || state.temperature <= 0.0 || state.density <= 0.0) {
            throw ValidationException("Invalid thermodynamic state calculated");
        }
        
        successfulCalls_++;
        return state;
        
    } catch (const ValidationException& e) {
        failedCalls_++;
        validationFailures_++;
        recordError("ValidationError", e.what());
        throw;
    } catch (const std::exception& e) {
        failedCalls_++;
        recordError("EquilibriumStateError", e.what());
        throw std::runtime_error("Equilibrium state calculation failed: " + std::string(e.what()));
    }
}

CanteraWrapper::ThermodynamicState CanteraWrapper::calculateFrozenState(
    double density, double temperature, const std::string& composition, const std::string& mechanism)
{
    totalCalls_++;
    
    try {
        // Prepare request
        json request;
        request["function"] = "state";
        request["density"] = density;
        request["temperature"] = temperature;
        request["composition"] = formatCompositionForSDT(composition);
        request["mechanism"] = mechanism;
        
        logDebug("Calculating frozen state");
        
        // Execute Python script
        json response = executePythonScript(BRIDGE_SCRIPT_NAME, request);
        
        // Validate response
        validatePythonResponse(response);
        
        // Extract result
        ThermodynamicState state = ThermodynamicState::fromJson(response["result"]);
        
        successfulCalls_++;
        return state;
        
    } catch (const std::exception& e) {
        failedCalls_++;
        throw std::runtime_error("Frozen state calculation failed: " + std::string(e.what()));
    }
}

CanteraWrapper::ConservationErrors CanteraWrapper::calculateFHFP(
    double waveSpeed, const ThermodynamicState& state1, const ThermodynamicState& state2)
{
    // Direct C++ implementation of SDT FHFP function for efficiency
    double P1 = state1.pressure;
    double H1 = state1.enthalpy;
    double r1 = state1.density;
    double P2 = state2.pressure;
    double H2 = state2.enthalpy;
    double r2 = state2.density;
    
    double w1s = waveSpeed * waveSpeed;
    double w2s = w1s * (r1 / r2) * (r1 / r2);
    
    ConservationErrors errors;
    errors.enthalpyError = H2 + 0.5 * w2s - (H1 + 0.5 * w1s);
    errors.pressureError = P2 + r2 * w2s - (P1 + r1 * w1s);
    
    logDebug("FHFP errors: FH=" + std::to_string(errors.enthalpyError) + 
            ", FP=" + std::to_string(errors.pressureError));
    
    return errors;
}

double CanteraWrapper::calculateSoundSpeedEq(const ThermodynamicState& state, const std::string& mechanism)
{
    totalCalls_++;
    
    try {
        // Prepare request
        json request;
        request["function"] = "soundspeed_eq";
        request["pressure"] = state.pressure;
        request["temperature"] = state.temperature;
        request["composition"] = state.composition;
        request["mechanism"] = mechanism;
        
        logDebug("Calculating equilibrium sound speed");
        
        // Execute Python script
        json response = executePythonScript(BRIDGE_SCRIPT_NAME, request);
        
        // Validate response
        validatePythonResponse(response);
        
        // Extract result
        double soundSpeed = response["result"]["sound_speed"];
        
        successfulCalls_++;
        return soundSpeed;
        
    } catch (const std::exception& e) {
        failedCalls_++;
        throw std::runtime_error("Sound speed calculation failed: " + std::string(e.what()));
    }
}

// Batch processing

std::vector<double> CanteraWrapper::calculateCJSpeedBatch(const std::vector<InitialConditions>& conditions)
{
    if (conditions.empty()) {
        return {};
    }
    
    // If small batch or batch processing disabled, use individual calls
    if (!useBatchProcessing_ || conditions.size() <= 3) {
        std::vector<double> results;
        results.reserve(conditions.size());
        
        for (const auto& condition : conditions) {
            double result = calculateCJSpeed(condition.pressure, condition.temperature,
                                           condition.composition, condition.mechanism);
            results.push_back(result);
        }
        
        return results;
    }
    
    // Use batch processing for larger sets
    try {
        // Prepare batch request
        json batchRequest;
        batchRequest["function"] = "batch";
        batchRequest["requests"] = json::array();
        
        for (const auto& condition : conditions) {
            json request;
            request["function"] = "CJspeed";
            request["P1"] = condition.pressure;
            request["T1"] = condition.temperature;
            request["q"] = formatCompositionForSDT(condition.composition);
            request["mech"] = condition.mechanism;
            request["fullOutput"] = false;
            
            batchRequest["requests"].push_back(request);
        }
        
        logDebug("Executing batch CJ speed calculation for " + std::to_string(conditions.size()) + " cases");
        
        // Execute batch
        json response = executePythonScript(BRIDGE_SCRIPT_NAME, batchRequest);
        validatePythonResponse(response);
        
        // Extract results
        std::vector<double> results;
        results.reserve(conditions.size());
        
        for (const auto& batchResult : response["result"]["batch_results"]) {
            if (batchResult["success"]) {
                results.push_back(batchResult["result"]["cj_speed"]);
            } else {
                throw std::runtime_error("Batch item failed: " + 
                                       batchResult["errors"][0].get<std::string>());
            }
        }
        
        totalCalls_ += conditions.size();
        successfulCalls_ += results.size();
        
        return results;
        
    } catch (const std::exception& e) {
        failedCalls_ += conditions.size();
        throw std::runtime_error("Batch CJ speed calculation failed: " + std::string(e.what()));
    }
}

// Configuration methods

void CanteraWrapper::setPythonPath(const std::string& pythonPath)
{
    pythonPath_ = pythonPath;
    logDebug("Python path set to: " + pythonPath_);
}

void CanteraWrapper::setSDToolboxPath(const std::string& sdtPath)
{
    sdtPath_ = sdtPath;
    logDebug("SDToolbox path set to: " + sdtPath_);
}

void CanteraWrapper::setWorkingDirectory(const std::string& workingDir)
{
    workingDir_ = workingDir;
    logDebug("Working directory set to: " + workingDir_);
}

void CanteraWrapper::setVerboseMode(bool verbose)
{
    verboseMode_ = verbose;
    logDebug("Verbose mode " + std::string(verbose ? "enabled" : "disabled"));
}

void CanteraWrapper::setTimeout(int timeoutMs)
{
    timeoutMs_ = timeoutMs;
    logDebug("Timeout set to: " + std::to_string(timeoutMs_) + " ms");
}

bool CanteraWrapper::testSDToolboxConnection()
{
    try {
        json request;
        request["function"] = "test_connection";
        
        json response = executePythonScript(BRIDGE_SCRIPT_NAME, request);
        
        if (response["success"]) {
            logDebug("SDToolbox connection test passed");
            bool cantera_available = response["result"]["cantera_available"];
            logDebug("Cantera available: " + std::string(cantera_available ? "true" : "false"));
            if (response["result"].contains("test_passed")) {
                bool test_passed = response["result"]["test_passed"];
                logDebug("Test calculation passed: " + std::string(test_passed ? "true" : "false"));
            }
            return true;
        } else {
            logDebug("SDToolbox connection test failed");
            return false;
        }
        
    } catch (const std::exception& e) {
        logDebug("SDToolbox connection test error: " + std::string(e.what()));
        return false;
    }
}

bool CanteraWrapper::validateMechanism(const std::string& mechanism)
{
    try {
        json request;
        request["function"] = "validate_mechanism";
        request["mechanism"] = mechanism;
        
        json response = executePythonScript(BRIDGE_SCRIPT_NAME, request);
        
        if (response["success"] && response["result"]["valid"]) {
            logDebug("Mechanism validation passed: " + mechanism);
            return true;
        } else {
            logDebug("Mechanism validation failed: " + mechanism);
            if (response["result"].contains("error")) {
                logDebug("Error: " + response["result"]["error"].get<std::string>());
            }
            return false;
        }
        
    } catch (const std::exception& e) {
        logDebug("Mechanism validation error: " + std::string(e.what()));
        return false;
    }
}

// Utility functions

std::string CanteraWrapper::generateComposition(const std::string& fuelType, 
                                               double equivalenceRatio,
                                               const std::string& oxidizerType)
{
    std::ostringstream oss;
    
    if (fuelType == "H2" || fuelType == "hydrogen") {
        if (oxidizerType == "air") {
            // H2 + 0.5(O2 + 3.76N2) = H2O + 1.88N2
            double h2 = equivalenceRatio * 2.0;
            double o2 = 1.0;
            double n2 = 3.76;
            oss << "H2:" << h2 << " O2:" << o2 << " N2:" << n2;
        } else if (oxidizerType == "O2") {
            // H2 + 0.5O2 = H2O
            double h2 = equivalenceRatio * 2.0;
            double o2 = 1.0;
            oss << "H2:" << h2 << " O2:" << o2;
        }
    } else if (fuelType == "CH4" || fuelType == "methane") {
        if (oxidizerType == "air") {
            // CH4 + 2(O2 + 3.76N2) = CO2 + 2H2O + 7.52N2
            double ch4 = equivalenceRatio * 1.0;
            double o2 = 2.0;
            double n2 = 7.52;
            oss << "CH4:" << ch4 << " O2:" << o2 << " N2:" << n2;
        } else if (oxidizerType == "O2") {
            // CH4 + 2O2 = CO2 + 2H2O
            double ch4 = equivalenceRatio * 1.0;
            double o2 = 2.0;
            oss << "CH4:" << ch4 << " O2:" << o2;
        }
    } else {
        throw std::invalid_argument("Unsupported fuel type: " + fuelType);
    }
    
    return oss.str();
}

std::map<std::string, double> CanteraWrapper::parseComposition(const std::string& composition)
{
    std::map<std::string, double> moleFractions;
    
    // Parse composition string: "H2:2 O2:1 N2:3.76"
    std::regex pattern(R"((\w+):([0-9]*\.?[0-9]+))");
    std::sregex_iterator begin(composition.begin(), composition.end(), pattern);
    std::sregex_iterator end;
    
    double totalMoles = 0.0;
    std::map<std::string, double> moles;
    
    for (std::sregex_iterator i = begin; i != end; ++i) {
        std::smatch match = *i;
        std::string species = match[1].str();
        double value = std::stod(match[2].str());
        moles[species] = value;
        totalMoles += value;
    }
    
    // Convert to mole fractions
    for (const auto& pair : moles) {
        moleFractions[pair.first] = pair.second / totalMoles;
    }
    
    return moleFractions;
}

std::vector<std::string> CanteraWrapper::getAvailableMechanisms()
{
    std::vector<std::string> mechanisms;
    
    try {
        std::filesystem::path mechDir(mechanismDir_);
        if (std::filesystem::exists(mechDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(mechDir)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if ((filename.size() > 4 && filename.substr(filename.size()-4) == ".cti") || 
                        (filename.size() > 5 && filename.substr(filename.size()-5) == ".yaml")) {
                        mechanisms.push_back(filename);
                    }
                }
            }
        }
        
        // Also check subdirectories
        for (const auto& subdir : {"SanDiego", "NASA9", "Mevel2017"}) {
            std::filesystem::path subdirPath = mechDir / subdir;
            if (std::filesystem::exists(subdirPath)) {
                for (const auto& entry : std::filesystem::directory_iterator(subdirPath)) {
                    if (entry.is_regular_file()) {
                        std::string filename = entry.path().filename().string();
                        if ((filename.size() > 4 && filename.substr(filename.size()-4) == ".cti") || 
                        (filename.size() > 5 && filename.substr(filename.size()-5) == ".yaml")) {
                            mechanisms.push_back(std::string(subdir) + "/" + filename);
                        }
                    }
                }
            }
        }
        
    } catch (const std::exception& e) {
        logDebug("Error scanning mechanisms: " + std::string(e.what()));
    }
    
    std::sort(mechanisms.begin(), mechanisms.end());
    return mechanisms;
}

// Private helper methods

json CanteraWrapper::executePythonScript(const std::string& script, const json& input)
{
    // Construct command
    std::filesystem::path scriptPath = std::filesystem::path(workingDir_) / "python_scripts" / script;
    if (!std::filesystem::exists(scriptPath)) {
        throw SubprocessException("Python script not found: " + scriptPath.string());
    }
    
    // Generate unique temporary file names using process ID and timestamp
    auto now = std::chrono::high_resolution_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    std::string tempInput = "/tmp/sdt_input_" + std::to_string(getpid()) + "_" + std::to_string(timestamp) + ".json";
    std::string tempOutput = "/tmp/sdt_output_" + std::to_string(getpid()) + "_" + std::to_string(timestamp) + ".json";
    std::string tempError = "/tmp/sdt_error_" + std::to_string(getpid()) + "_" + std::to_string(timestamp) + ".txt";
    
    try {
        // Write input to temp file
        std::string inputStr = input.dump();
        std::ofstream inputFile(tempInput);
        if (!inputFile.is_open()) {
            throw SubprocessException("Failed to create input file: " + tempInput);
        }
        inputFile << inputStr;
        inputFile.close();
        
        // Construct command with timeout and error capture
        std::string command = "timeout " + std::to_string(timeoutMs_ / 1000) + "s " +
                             pythonPath_ + " " + scriptPath.string() + 
                             " < " + tempInput + " > " + tempOutput + " 2> " + tempError;
        
        logDebug("Executing: " + command);
        
        // Record start time for timeout tracking
        auto startTime = std::chrono::steady_clock::now();
        
        // Execute command
        int result = std::system(command.c_str());
        
        // Calculate execution time
        auto endTime = std::chrono::steady_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        
        logDebug("Python script completed in " + std::to_string(durationMs) + "ms with exit code " + std::to_string(result));
        
        // Check for timeout (exit code 124 from timeout command)
        if (result == 124 * 256) {
            std::string errorMsg = "Python script timed out after " + std::to_string(timeoutMs_) + "ms";
            throw TimeoutException(errorMsg);
        }
        
        // Check for other execution failures
        if (result != 0) {
            std::string errorOutput;
            std::ifstream errorFile(tempError);
            if (errorFile.is_open()) {
                errorOutput = std::string((std::istreambuf_iterator<char>(errorFile)),
                                        std::istreambuf_iterator<char>());
                errorFile.close();
            }
            
            std::string errorMsg = "Python script execution failed with code " + std::to_string(result);
            if (!errorOutput.empty()) {
                errorMsg += "\nError output: " + errorOutput;
            }
            throw SubprocessException(errorMsg);
        }
        
        // Read output
        std::ifstream outputFile(tempOutput);
        if (!outputFile.is_open()) {
            throw SubprocessException("Failed to read output file: " + tempOutput);
        }
        
        std::string outputStr((std::istreambuf_iterator<char>(outputFile)),
                             std::istreambuf_iterator<char>());
        outputFile.close();
        
        if (outputStr.empty()) {
            throw SubprocessException("Python script produced no output");
        }
        
        // Clean up temp files
        std::filesystem::remove(tempInput);
        std::filesystem::remove(tempOutput);
        std::filesystem::remove(tempError);
        
        // Parse output JSON
        try {
            return json::parse(outputStr);
        } catch (const json::parse_error& e) {
            throw SubprocessException("Failed to parse Python output as JSON: " + std::string(e.what()) + 
                                   "\nOutput: " + outputStr);
        }
        
    } catch (const std::exception& e) {
        // Clean up temp files on error
        try {
            std::filesystem::remove(tempInput);
            std::filesystem::remove(tempOutput);
            std::filesystem::remove(tempError);
        } catch (...) {
            // Ignore cleanup errors
        }
        throw;
    }
}

void CanteraWrapper::validatePythonResponse(const json& response)
{
    if (!response.contains("success")) {
        throw std::runtime_error("Invalid response format: missing 'success' field");
    }
    
    if (!response["success"]) {
        std::string error = "Python execution failed";
        if (response.contains("errors") && !response["errors"].empty()) {
            error += ": " + response["errors"][0].get<std::string>();
        }
        throw std::runtime_error(error);
    }
    
    if (!response.contains("result")) {
        throw std::runtime_error("Invalid response format: missing 'result' field");
    }
}

void CanteraWrapper::handlePythonError(const std::string& errorMessage, const json& input)
{
    std::string fullError = "Python error: " + errorMessage + "\nInput: " + input.dump(2);
    recentErrors_.push_back(fullError);
    if (recentErrors_.size() > 10) {
        recentErrors_.erase(recentErrors_.begin());
    }
    logDebug(fullError);
}

std::string CanteraWrapper::findMechanismFile(const std::string& mechanism)
{
    // If absolute path and exists, return as-is
    if (std::filesystem::path(mechanism).is_absolute() && std::filesystem::exists(mechanism)) {
        return mechanism;
    }
    
    // Search in mechanism directory
    std::filesystem::path mechPath = std::filesystem::path(mechanismDir_) / mechanism;
    if (std::filesystem::exists(mechPath)) {
        return mechPath.string();
    }
    
    // Search in subdirectories
    for (const auto& subdir : {"SanDiego", "NASA9", "Mevel2017", "."}) {
        std::filesystem::path subdirPath = std::filesystem::path(mechanismDir_) / subdir / mechanism;
        if (std::filesystem::exists(subdirPath)) {
            return subdirPath.string();
        }
    }
    
    // Not found - return original and let Python handle the error
    return mechanism;
}

void CanteraWrapper::logDebug(const std::string& message) const
{
    if (verboseMode_) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::cout << "[CanteraWrapper " << std::put_time(std::localtime(&time_t), "%H:%M:%S") 
                  << "] " << message << std::endl;
    }
}

void CanteraWrapper::logError(const std::string& errorType, const std::string& message) const
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::cerr << "[CanteraWrapper ERROR " << std::put_time(std::localtime(&time_t), "%H:%M:%S") 
              << "] [" << errorType << "] " << message << std::endl;
}

// Input validation methods
void CanteraWrapper::validateThermodynamicInputs(double pressure, double temperature, 
                                               const std::string& composition) const
{
    validatePhysicalBounds(pressure, temperature);
    
    if (!isCompositionValid(composition)) {
        throw ValidationException("Invalid composition format: " + composition);
    }
}

void CanteraWrapper::validatePhysicalBounds(double pressure, double temperature) const
{
    if (pressure < MIN_PRESSURE || pressure > MAX_PRESSURE) {
        throw ValidationException("Pressure out of bounds: " + std::to_string(pressure) + 
                                " Pa (valid range: " + std::to_string(MIN_PRESSURE) + 
                                " - " + std::to_string(MAX_PRESSURE) + " Pa)");
    }
    
    if (temperature < MIN_TEMPERATURE || temperature > MAX_TEMPERATURE) {
        throw ValidationException("Temperature out of bounds: " + std::to_string(temperature) + 
                                " K (valid range: " + std::to_string(MIN_TEMPERATURE) + 
                                " - " + std::to_string(MAX_TEMPERATURE) + " K)");
    }
}

bool CanteraWrapper::isCompositionValid(const std::string& composition) const
{
    if (composition.empty()) {
        return false;
    }
    
    // Check for valid composition format: "Species:value Species:value"
    std::regex pattern(R"((\w+):(\d*\.?\d+))");
    std::sregex_iterator begin(composition.begin(), composition.end(), pattern);
    std::sregex_iterator end;
    
    // Must have at least one species
    if (begin == end) {
        return false;
    }
    
    // Validate each species:value pair
    for (std::sregex_iterator i = begin; i != end; ++i) {
        std::smatch match = *i;
        try {
            double value = std::stod(match[2].str());
            if (value < 0.0 || std::isnan(value) || std::isinf(value)) {
                return false;
            }
        } catch (const std::exception&) {
            return false;
        }
    }
    
    return true;
}

// Reliability helper methods
void CanteraWrapper::recordError(const std::string& errorType, const std::string& message)
{
    errorTypeCount_[errorType]++;
    recentErrors_.push_back("[" + errorType + "] " + message);
    if (recentErrors_.size() > 10) {
        recentErrors_.erase(recentErrors_.begin());
    }
    logError(errorType, message);
}

bool CanteraWrapper::shouldRetry(const std::exception& e, int attemptCount) const
{
    if (attemptCount >= MAX_RETRY_ATTEMPTS) {
        return false;
    }
    
    // Check if this is a retryable error
    std::string errorMsg = e.what();
    
    // Don't retry validation errors or thermodynamic errors
    if (dynamic_cast<const ValidationException*>(&e) || 
        dynamic_cast<const ThermodynamicException*>(&e)) {
        return false;
    }
    
    // Retry subprocess and timeout errors
    if (dynamic_cast<const SubprocessException*>(&e) || 
        dynamic_cast<const TimeoutException*>(&e)) {
        return true;
    }
    
    // Retry certain system errors
    if (errorMsg.find("Failed to open subprocess") != std::string::npos ||
        errorMsg.find("Python script execution failed") != std::string::npos ||
        errorMsg.find("Failed to read output") != std::string::npos) {
        return true;
    }
    
    return false;
}

int CanteraWrapper::calculateBackoffDelay(int attemptCount) const
{
    // Exponential backoff: 100ms, 200ms, 400ms
    return INITIAL_BACKOFF_MS * static_cast<int>(std::pow(2, attemptCount - 1));
}

// Enhanced subprocess execution with retry logic
json CanteraWrapper::executePythonScriptWithRetry(const std::string& script, 
                                                 const json& input, 
                                                 int maxRetries)
{
    int attemptCount = 0;
    
    while (attemptCount <= maxRetries) {
        try {
            attemptCount++;
            
            if (attemptCount > 1) {
                retryAttempts_++;
                int delay = calculateBackoffDelay(attemptCount - 1);
                logDebug("Retry attempt " + std::to_string(attemptCount) + 
                        " after " + std::to_string(delay) + "ms delay");
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }
            
            json response = executePythonScript(script, input);
            
            if (attemptCount > 1) {
                logDebug("Retry succeeded after " + std::to_string(attemptCount) + " attempts");
            }
            
            return response;
            
        } catch (const std::exception& e) {
            if (!shouldRetry(e, attemptCount) || attemptCount > maxRetries) {
                if (attemptCount > 1) {
                    logError("RetryFailed", "All retry attempts failed after " + 
                           std::to_string(attemptCount) + " attempts: " + e.what());
                }
                throw;
            }
            
            logDebug("Attempt " + std::to_string(attemptCount) + " failed: " + e.what() + 
                    ". Retrying...");
        }
    }
    
    throw SubprocessException("Maximum retry attempts exceeded");
}

std::string CanteraWrapper::formatCompositionForSDT(const std::string& composition)
{
    // SDT expects composition in specific format - validate and normalize
    if (!isCompositionValid(composition)) {
        throw ValidationException("Invalid composition format for SDT: " + composition);
    }
    
    // Normalize format by ensuring proper spacing
    std::string normalized = composition;
    // Remove extra whitespace
    std::regex extraSpaces(R"(\s+)");
    normalized = std::regex_replace(normalized, extraSpaces, " ");
    // Trim leading/trailing whitespace
    normalized.erase(0, normalized.find_first_not_of(" \t\r\n"));
    normalized.erase(normalized.find_last_not_of(" \t\r\n") + 1);
    
    return normalized;
}

// Initialize standard benchmarks
void CanteraWrapper::initializeStandardBenchmarks()
{
    // Add standard benchmarks based on literature data
    
    // H2-air stoichiometric at STP (Shepherd 2009)
    validationBenchmarks_.push_back(createBenchmark(
        "H2_air_STP",
        "Stoichiometric H2-air at standard conditions",
        101325.0,  // 1 atm
        300.0,     // 300 K
        "H2:2 O2:1 N2:3.76",
        "sandiego20161214_H2only.yaml",
        1975.3,    // Expected CJ speed (m/s)
        0.03,      // 3% tolerance
        "Shepherd, J.E. (2009). Detonation in gases. Proc. Combust. Inst., 32(1), 83-98"
    ));
    
    // H2-air rich mixture (Gamezo 2007)
    validationBenchmarks_.push_back(createBenchmark(
        "H2_air_rich",
        "Rich H2-air mixture (φ=2.0)",
        101325.0,
        300.0,
        "H2:4 O2:1 N2:3.76",
        "sandiego20161214_H2only.yaml",
        2130.5,    // Expected CJ speed for rich mixture
        0.05,      // 5% tolerance
        "Gamezo, V.N. et al. (2007). Flame acceleration and DDT in channels with obstacles. Proc. Combust. Inst., 31(2), 2463-2471"
    ));
    
    logDebug("Initialized " + std::to_string(validationBenchmarks_.size()) + " standard validation benchmarks");
}

// Validation framework methods
void CanteraWrapper::addValidationBenchmark(const ValidationBenchmark& benchmark)
{
    validationBenchmarks_.push_back(benchmark);
    logDebug("Added validation benchmark: " + benchmark.name);
}

// JSON serialization for ValidationBenchmark
json CanteraWrapper::ValidationBenchmark::toJson() const
{
    json j;
    j["name"] = name;
    j["description"] = description;
    j["pressure"] = pressure;
    j["temperature"] = temperature;
    j["composition"] = composition;
    j["mechanism"] = mechanism;
    j["expectedCJSpeed"] = expectedCJSpeed;
    j["tolerance"] = tolerance;
    j["reference"] = reference;
    return j;
}

CanteraWrapper::ValidationBenchmark CanteraWrapper::ValidationBenchmark::fromJson(const json& j)
{
    ValidationBenchmark benchmark;
    benchmark.name = j.at("name");
    benchmark.description = j.at("description");
    benchmark.pressure = j.at("pressure");
    benchmark.temperature = j.at("temperature");
    benchmark.composition = j.at("composition");
    benchmark.mechanism = j.at("mechanism");
    benchmark.expectedCJSpeed = j.at("expectedCJSpeed");
    benchmark.tolerance = j.value("tolerance", DEFAULT_TOLERANCE);
    benchmark.reference = j.value("reference", "Unknown");
    return benchmark;
}

// JSON serialization for AccuracyMetrics
json CanteraWrapper::AccuracyMetrics::toJson() const
{
    json j;
    j["testCase"] = testCase;
    j["calculatedValue"] = calculatedValue;
    j["expectedValue"] = expectedValue;
    j["absoluteError"] = absoluteError;
    j["relativeError"] = relativeError;
    j["withinTolerance"] = withinTolerance;
    j["executionTimeMs"] = executionTimeMs;
    
    // Convert timestamp to ISO 8601 string
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    j["timestamp"] = oss.str();
    
    return j;
}

CanteraWrapper::AccuracyMetrics CanteraWrapper::AccuracyMetrics::fromJson(const json& j)
{
    AccuracyMetrics metrics;
    metrics.testCase = j.at("testCase");
    metrics.calculatedValue = j.at("calculatedValue");
    metrics.expectedValue = j.at("expectedValue");
    metrics.absoluteError = j.at("absoluteError");
    metrics.relativeError = j.at("relativeError");
    metrics.withinTolerance = j.at("withinTolerance");
    metrics.executionTimeMs = j.at("executionTimeMs");
    
    // Parse ISO 8601 timestamp (simplified)
    metrics.timestamp = std::chrono::system_clock::now();
    
    return metrics;
}

// JSON serialization for ValidationReport
json CanteraWrapper::ValidationReport::toJson() const
{
    json j;
    j["totalTests"] = totalTests;
    j["passedTests"] = passedTests;
    j["failedTests"] = failedTests;
    j["overallAccuracy"] = overallAccuracy;
    j["averageError"] = averageError;
    
    // Convert timestamp to ISO 8601 string
    auto time_t = std::chrono::system_clock::to_time_t(reportTime);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    j["reportTime"] = oss.str();
    
    j["results"] = json::array();
    for (const auto& result : results) {
        j["results"].push_back(result.toJson());
    }
    
    return j;
}

CanteraWrapper::ValidationReport CanteraWrapper::ValidationReport::fromJson(const json& j)
{
    ValidationReport report;
    report.totalTests = j.at("totalTests");
    report.passedTests = j.at("passedTests");
    report.failedTests = j.at("failedTests");
    report.overallAccuracy = j.at("overallAccuracy");
    report.averageError = j.at("averageError");
    
    // Parse ISO 8601 timestamp (simplified)
    report.reportTime = std::chrono::system_clock::now();
    
    for (const auto& resultJson : j["results"]) {
        report.results.push_back(AccuracyMetrics::fromJson(resultJson));
    }
    
    return report;
}

// Validation suite runner
CanteraWrapper::ValidationReport CanteraWrapper::runValidationSuite(bool verbose)
{
    if (!validationEnabled_) {
        throw ValidationException("Validation framework is not enabled");
    }
    
    ValidationReport report;
    report.reportTime = std::chrono::system_clock::now();
    report.totalTests = validationBenchmarks_.size();
    report.passedTests = 0;
    report.failedTests = 0;
    
    double totalRelativeError = 0.0;
    
    logDebug("Starting validation suite with " + std::to_string(report.totalTests) + " benchmarks");
    
    for (const auto& benchmark : validationBenchmarks_) {
        try {
            auto startTime = std::chrono::steady_clock::now();
            
            // Run calculation
            double calculatedSpeed = calculateCJSpeed(
                benchmark.pressure,
                benchmark.temperature,
                benchmark.composition,
                benchmark.mechanism
            );
            
            auto endTime = std::chrono::steady_clock::now();
            int executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            
            // Calculate accuracy metrics
            AccuracyMetrics metrics;
            metrics.testCase = benchmark.name;
            metrics.calculatedValue = calculatedSpeed;
            metrics.expectedValue = benchmark.expectedCJSpeed;
            metrics.absoluteError = std::abs(calculatedSpeed - benchmark.expectedCJSpeed);
            metrics.relativeError = (metrics.absoluteError / benchmark.expectedCJSpeed) * 100.0;
            metrics.withinTolerance = validateCalculationAccuracy(calculatedSpeed, benchmark.expectedCJSpeed, benchmark.tolerance);
            metrics.timestamp = std::chrono::system_clock::now();
            metrics.executionTimeMs = executionTime;
            
            // Record metrics
            recordAccuracyMetric(metrics);
            report.results.push_back(metrics);
            
            if (metrics.withinTolerance) {
                report.passedTests++;
                if (verbose) {
                    logDebug("✓ " + benchmark.name + ": " + std::to_string(calculatedSpeed) + " m/s (error: " + 
                           std::to_string(metrics.relativeError) + "%, time: " + std::to_string(executionTime) + "ms)");
                }
            } else {
                report.failedTests++;
                logError("ValidationFailure", "✗ " + benchmark.name + ": " + std::to_string(calculatedSpeed) + 
                        " m/s, expected: " + std::to_string(benchmark.expectedCJSpeed) + 
                        " m/s (error: " + std::to_string(metrics.relativeError) + "%, tolerance: " + 
                        std::to_string(benchmark.tolerance * 100.0) + "%)");
            }
            
            totalRelativeError += metrics.relativeError;
            
        } catch (const std::exception& e) {
            report.failedTests++;
            logError("ValidationException", "Failed benchmark " + benchmark.name + ": " + e.what());
        }
    }
    
    // Calculate overall statistics
    report.overallAccuracy = report.totalTests > 0 ? 
        (static_cast<double>(report.passedTests) / report.totalTests) * 100.0 : 0.0;
    report.averageError = report.totalTests > 0 ? 
        totalRelativeError / report.totalTests : 0.0;
    
    // Update accuracy score
    updateAccuracyScore();
    
    if (verbose || report.overallAccuracy < 90.0) {
        logDebug("Validation suite completed: " + std::to_string(report.passedTests) + "/" + 
                std::to_string(report.totalTests) + " passed (" + 
                std::to_string(report.overallAccuracy) + "% accuracy, " + 
                std::to_string(report.averageError) + "% avg error)");
    }
    
    return report;
}

CanteraWrapper::ValidationReport CanteraWrapper::runSingleValidation(const std::string& benchmarkName, bool verbose)
{
    auto it = std::find_if(validationBenchmarks_.begin(), validationBenchmarks_.end(),
                          [&benchmarkName](const ValidationBenchmark& b) { return b.name == benchmarkName; });
    
    if (it == validationBenchmarks_.end()) {
        throw ValidationException("Benchmark not found: " + benchmarkName);
    }
    
    // Temporarily store the single benchmark
    std::vector<ValidationBenchmark> originalBenchmarks = validationBenchmarks_;
    validationBenchmarks_ = {*it};
    
    // Run validation
    ValidationReport report = runValidationSuite(verbose);
    
    // Restore original benchmarks
    validationBenchmarks_ = originalBenchmarks;
    
    return report;
}

std::vector<CanteraWrapper::ValidationBenchmark> CanteraWrapper::getStandardBenchmarks() const
{
    return validationBenchmarks_;
}

CanteraWrapper::AccuracyMetrics CanteraWrapper::getLastAccuracyMetric(const std::string& testCase) const
{
    auto it = accuracyHistory_.find(testCase);
    if (it != accuracyHistory_.end()) {
        return it->second;
    }
    
    throw ValidationException("No accuracy metrics found for test case: " + testCase);
}

void CanteraWrapper::loadValidationBenchmarks(const std::string& filePath)
{
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw ValidationException("Cannot open validation file: " + filePath);
        }
        
        json benchmarkData;
        file >> benchmarkData;
        
        validationBenchmarks_.clear();
        for (const auto& benchmarkJson : benchmarkData["benchmarks"]) {
            validationBenchmarks_.push_back(ValidationBenchmark::fromJson(benchmarkJson));
        }
        
        logDebug("Loaded " + std::to_string(validationBenchmarks_.size()) + " benchmarks from " + filePath);
        
    } catch (const std::exception& e) {
        throw ValidationException("Failed to load validation benchmarks: " + std::string(e.what()));
    }
}

void CanteraWrapper::saveValidationReport(const ValidationReport& report, const std::string& filePath) const
{
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw ValidationException("Cannot create report file: " + filePath);
        }
        
        file << std::setw(2) << report.toJson() << std::endl;
        logDebug("Validation report saved to: " + filePath);
        
    } catch (const std::exception& e) {
        throw ValidationException("Failed to save validation report: " + std::string(e.what()));
    }
}

// Validation helper methods
void CanteraWrapper::recordAccuracyMetric(const AccuracyMetrics& metric)
{
    accuracyHistory_[metric.testCase] = metric;
    
    // Limit history size
    if (accuracyHistory_.size() > MAX_ACCURACY_HISTORY) {
        // Remove oldest entries (simplified - could be improved with timestamp-based cleanup)
        auto oldest = accuracyHistory_.begin();
        accuracyHistory_.erase(oldest);
    }
}

bool CanteraWrapper::validateCalculationAccuracy(double calculated, double expected, double tolerance) const
{
    if (expected == 0.0) {
        return std::abs(calculated) <= tolerance;
    }
    
    double relativeError = std::abs((calculated - expected) / expected);
    return relativeError <= tolerance;
}

CanteraWrapper::ValidationBenchmark CanteraWrapper::createBenchmark(
    const std::string& name, const std::string& description,
    double pressure, double temperature, const std::string& composition,
    const std::string& mechanism, double expectedCJSpeed, 
    double tolerance, const std::string& reference) const
{
    ValidationBenchmark benchmark;
    benchmark.name = name;
    benchmark.description = description;
    benchmark.pressure = pressure;
    benchmark.temperature = temperature;
    benchmark.composition = composition;
    benchmark.mechanism = mechanism;
    benchmark.expectedCJSpeed = expectedCJSpeed;
    benchmark.tolerance = tolerance;
    benchmark.reference = reference;
    
    return benchmark;
}

void CanteraWrapper::updateAccuracyScore()
{
    if (accuracyHistory_.empty()) {
        totalAccuracyScore_ = 0.0;
        return;
    }
    
    int passedTests = 0;
    for (const auto& pair : accuracyHistory_) {
        if (pair.second.withinTolerance) {
            passedTests++;
        }
    }
    
    totalAccuracyScore_ = static_cast<double>(passedTests) / accuracyHistory_.size();
}

// Performance optimization implementation

// JSON serialization for CachedResult
json CanteraWrapper::CachedResult::toJson() const
{
    json j;
    j["result"] = result;
    j["inputHash"] = inputHash;
    j["accessCount"] = accessCount;
    
    // Convert timestamp to ISO 8601 string
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    j["timestamp"] = oss.str();
    
    return j;
}

CanteraWrapper::CachedResult CanteraWrapper::CachedResult::fromJson(const json& j)
{
    CachedResult cached;
    cached.result = j.at("result");
    cached.inputHash = j.at("inputHash");
    cached.accessCount = j.value("accessCount", 1);
    
    // Parse ISO 8601 timestamp (simplified)
    cached.timestamp = std::chrono::system_clock::now();
    
    return cached;
}

// JSON serialization for PerformanceMetrics
json CanteraWrapper::PerformanceMetrics::toJson() const
{
    json j;
    j["totalExecutions"] = totalExecutions;
    j["totalExecutionTime"] = totalExecutionTime;
    j["averageExecutionTime"] = averageExecutionTime;
    j["minExecutionTime"] = minExecutionTime;
    j["maxExecutionTime"] = maxExecutionTime;
    
    // Convert timestamp to ISO 8601 string
    auto time_t = std::chrono::system_clock::to_time_t(lastExecution);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    j["lastExecution"] = oss.str();
    
    return j;
}

CanteraWrapper::PerformanceMetrics CanteraWrapper::PerformanceMetrics::fromJson(const json& j)
{
    PerformanceMetrics metrics;
    metrics.totalExecutions = j.value("totalExecutions", 0);
    metrics.totalExecutionTime = j.value("totalExecutionTime", 0.0);
    metrics.averageExecutionTime = j.value("averageExecutionTime", 0.0);
    metrics.minExecutionTime = j.value("minExecutionTime", 0.0);
    metrics.maxExecutionTime = j.value("maxExecutionTime", 0.0);
    metrics.lastExecution = std::chrono::system_clock::now();
    
    return metrics;
}

void CanteraWrapper::enableCaching(bool enable, const std::string& cacheFile)
{
    cachingEnabled_ = enable;
    if (!cacheFile.empty()) {
        cacheFilePath_ = cacheFile;
    }
    
    if (enable) {
        loadCacheFromFile();
        logDebug("Caching enabled with file: " + cacheFilePath_);
    } else {
        logDebug("Caching disabled");
    }
}

void CanteraWrapper::clearCache()
{
    resultCache_.clear();
    lruOrder_.clear();
    cacheHits_ = 0;
    cacheMisses_ = 0;
    logDebug("Cache cleared");
}

void CanteraWrapper::saveCacheToFile() const
{
    try {
        json cacheData;
        cacheData["cache"] = json::object();
        
        for (const auto& pair : resultCache_) {
            cacheData["cache"][pair.first] = pair.second.toJson();
        }
        
        cacheData["statistics"] = json::object();
        cacheData["statistics"]["cacheHits"] = cacheHits_;
        cacheData["statistics"]["cacheMisses"] = cacheMisses_;
        
        cacheData["performance"] = performanceMetrics_.toJson();
        
        std::ofstream file(cacheFilePath_);
        if (file.is_open()) {
            file << std::setw(2) << cacheData << std::endl;
            logDebug("Cache saved to: " + cacheFilePath_);
        } else {
            logError("CacheError", "Failed to save cache to: " + cacheFilePath_);
        }
        
    } catch (const std::exception& e) {
        logError("CacheError", "Cache save failed: " + std::string(e.what()));
    }
}

void CanteraWrapper::loadCacheFromFile()
{
    try {
        std::ifstream file(cacheFilePath_);
        if (!file.is_open()) {
            logDebug("Cache file not found, starting with empty cache: " + cacheFilePath_);
            return;
        }
        
        json cacheData;
        file >> cacheData;
        
        // Load cached results
        if (cacheData.contains("cache")) {
            for (const auto& pair : cacheData["cache"].items()) {
                resultCache_[pair.key()] = CachedResult::fromJson(pair.value());
                lruOrder_.push_back(pair.key());
            }
        }
        
        // Load statistics
        if (cacheData.contains("statistics")) {
            cacheHits_ = cacheData["statistics"].value("cacheHits", 0);
            cacheMisses_ = cacheData["statistics"].value("cacheMisses", 0);
        }
        
        // Load performance metrics
        if (cacheData.contains("performance")) {
            performanceMetrics_ = PerformanceMetrics::fromJson(cacheData["performance"]);
        }
        
        logDebug("Cache loaded: " + std::to_string(resultCache_.size()) + " entries from " + cacheFilePath_);
        
    } catch (const std::exception& e) {
        logError("CacheError", "Cache load failed: " + std::string(e.what()));
        clearCache();
    }
}

void CanteraWrapper::resetPerformanceMetrics()
{
    performanceMetrics_ = {};
    performanceMetrics_.totalExecutions = 0;
    performanceMetrics_.totalExecutionTime = 0.0;
    performanceMetrics_.averageExecutionTime = 0.0;
    performanceMetrics_.minExecutionTime = std::numeric_limits<double>::max();
    performanceMetrics_.maxExecutionTime = 0.0;
    logDebug("Performance metrics reset");
}

std::string CanteraWrapper::generateCacheKey(const std::string& function, double pressure, 
                                            double temperature, const std::string& composition,
                                            const std::string& mechanism) const
{
    // Create a hash of the input parameters for cache key
    std::ostringstream oss;
    oss << function << "_";
    oss << std::fixed << std::setprecision(6) << pressure << "_";
    oss << std::fixed << std::setprecision(6) << temperature << "_";
    oss << composition << "_";
    oss << mechanism;
    
    // Use hash to create shorter key
    std::hash<std::string> hasher;
    size_t hashValue = hasher(oss.str());
    
    return std::to_string(hashValue);
}

bool CanteraWrapper::getCachedResult(const std::string& cacheKey, double& result) const
{
    auto it = resultCache_.find(cacheKey);
    if (it != resultCache_.end()) {
        result = it->second.result;
        it->second.accessCount++;
        updateLRU(cacheKey);
        cacheHits_++;
        return true;
    }
    
    cacheMisses_++;
    return false;
}

void CanteraWrapper::setCachedResult(const std::string& cacheKey, double result) const
{
    // Check if cache is full and evict LRU if necessary
    if (resultCache_.size() >= MAX_CACHE_SIZE && resultCache_.find(cacheKey) == resultCache_.end()) {
        evictLRU();
    }
    
    CachedResult cached;
    cached.result = result;
    cached.timestamp = std::chrono::system_clock::now();
    cached.inputHash = cacheKey;
    cached.accessCount = 1;
    
    resultCache_[cacheKey] = cached;
    updateLRU(cacheKey);
}

void CanteraWrapper::updateLRU(const std::string& cacheKey) const
{
    // Remove key if it exists in LRU list
    lruOrder_.remove(cacheKey);
    // Add to front (most recently used)
    lruOrder_.push_front(cacheKey);
}

void CanteraWrapper::evictLRU() const
{
    if (!lruOrder_.empty()) {
        std::string lruKey = lruOrder_.back();
        lruOrder_.pop_back();
        resultCache_.erase(lruKey);
        logDebug("Evicted LRU cache entry: " + lruKey);
    }
}

void CanteraWrapper::recordPerformance(double executionTime) const
{
    performanceMetrics_.totalExecutions++;
    performanceMetrics_.totalExecutionTime += executionTime;
    performanceMetrics_.averageExecutionTime = performanceMetrics_.totalExecutionTime / performanceMetrics_.totalExecutions;
    
    if (executionTime < performanceMetrics_.minExecutionTime) {
        performanceMetrics_.minExecutionTime = executionTime;
    }
    if (executionTime > performanceMetrics_.maxExecutionTime) {
        performanceMetrics_.maxExecutionTime = executionTime;
    }
    
    performanceMetrics_.lastExecution = std::chrono::system_clock::now();
}

} // namespace Physics
} // namespace MCP
} // namespace Foam