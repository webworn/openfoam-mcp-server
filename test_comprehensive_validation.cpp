#include "src/physics/cantera_wrapper.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>

using namespace Foam::MCP::Physics;
using json = nlohmann::json;

/**
 * Comprehensive Validation Suite for Cantera Wrapper
 * 
 * This test suite validates the Cantera wrapper against 18+ literature benchmarks
 * from major detonation research papers including Shepherd, Gamezo, Kessler, 
 * Mevel, Burke, Keromnes, and others.
 */

struct ValidationResult {
    std::string benchmarkId;
    std::string benchmarkName;
    double calculatedValue;
    double expectedValue;
    double absoluteError;
    double relativeError;
    bool withinTolerance;
    double tolerance;
    int executionTimeMs;
    std::string status;
    std::string reference;
};

struct StatisticalSummary {
    int totalTests;
    int passedTests;
    int failedTests;
    double passRate;
    double meanAbsoluteError;
    double meanRelativeError;
    double rmse;
    double maxError;
    double minError;
    std::vector<double> relativeErrors;
    double bias;
    double standardDeviation;
};

class ComprehensiveValidator {
private:
    CanteraWrapper wrapper_;
    std::vector<ValidationResult> results_;
    json benchmarkData_;
    
public:
    ComprehensiveValidator() {
        wrapper_.setVerboseMode(false);
        wrapper_.enableCaching(true, "validation_cache.json");
        wrapper_.enableValidation(true);
    }
    
    bool loadBenchmarks(const std::string& filePath) {
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                std::cerr << "❌ Error: Cannot open benchmark file: " << filePath << std::endl;
                return false;
            }
            
            file >> benchmarkData_;
            
            if (!benchmarkData_.contains("benchmarks") || !benchmarkData_["benchmarks"].is_array()) {
                std::cerr << "❌ Error: Invalid benchmark file format" << std::endl;
                return false;
            }
            
            std::cout << "✅ Loaded " << benchmarkData_["benchmarks"].size() 
                      << " benchmarks from " << filePath << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Error loading benchmarks: " << e.what() << std::endl;
            return false;
        }
    }
    
    ValidationResult runSingleBenchmark(const json& benchmark) {
        ValidationResult result;
        result.benchmarkId = benchmark["id"];
        result.benchmarkName = benchmark["name"];
        result.reference = benchmark["reference"];
        result.tolerance = benchmark["expected_results"]["tolerance"];
        result.expectedValue = benchmark["expected_results"]["cj_speed"];
        
        try {
            auto startTime = std::chrono::steady_clock::now();
            
            // Extract conditions
            double pressure = benchmark["conditions"]["pressure"];
            double temperature = benchmark["conditions"]["temperature"];
            std::string composition = benchmark["conditions"]["composition"];
            std::string mechanism = benchmark["conditions"]["mechanism"];
            
            // Calculate CJ speed
            result.calculatedValue = wrapper_.calculateCJSpeed(pressure, temperature, composition, mechanism);
            
            auto endTime = std::chrono::steady_clock::now();
            result.executionTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            
            // Calculate errors
            result.absoluteError = std::abs(result.calculatedValue - result.expectedValue);
            result.relativeError = (result.absoluteError / result.expectedValue) * 100.0;
            result.withinTolerance = result.relativeError <= (result.tolerance * 100.0);
            result.status = result.withinTolerance ? "PASS" : "FAIL";
            
        } catch (const std::exception& e) {
            result.calculatedValue = 0.0;
            result.absoluteError = result.expectedValue;
            result.relativeError = 100.0;
            result.withinTolerance = false;
            result.status = "ERROR: " + std::string(e.what());
            result.executionTimeMs = 0;
        }
        
        return result;
    }
    
    void runAllBenchmarks() {
        std::cout << "\n=== Comprehensive Validation Suite ===\n" << std::endl;
        std::cout << "🔬 Running " << benchmarkData_["benchmarks"].size() << " literature benchmarks...\n" << std::endl;
        
        results_.clear();
        int currentTest = 0;
        
        for (const auto& benchmark : benchmarkData_["benchmarks"]) {
            currentTest++;
            std::cout << "[" << std::setw(2) << currentTest << "/" 
                      << benchmarkData_["benchmarks"].size() << "] "
                      << benchmark["name"].get<std::string>() << "... ";
            std::cout.flush();
            
            ValidationResult result = runSingleBenchmark(benchmark);
            results_.push_back(result);
            
            // Status output with color coding
            if (result.withinTolerance) {
                std::cout << "✅ PASS (" << std::fixed << std::setprecision(2) 
                          << result.relativeError << "% error, " << result.executionTimeMs << "ms)" << std::endl;
            } else if (result.status.find("ERROR") != std::string::npos) {
                std::cout << "❌ ERROR (" << result.status << ")" << std::endl;
            } else {
                std::cout << "⚠️  FAIL (" << std::fixed << std::setprecision(2) 
                          << result.relativeError << "% error > " << (result.tolerance * 100.0) << "% tolerance, "
                          << result.executionTimeMs << "ms)" << std::endl;
            }
        }
    }
    
    StatisticalSummary calculateStatistics() const {
        StatisticalSummary stats;
        stats.totalTests = results_.size();
        stats.passedTests = std::count_if(results_.begin(), results_.end(), 
                                         [](const ValidationResult& r) { return r.withinTolerance; });
        stats.failedTests = stats.totalTests - stats.passedTests;
        stats.passRate = (static_cast<double>(stats.passedTests) / stats.totalTests) * 100.0;
        
        // Collect valid relative errors (excluding ERROR cases)
        std::vector<double> validRelativeErrors;
        std::vector<double> validAbsoluteErrors;
        
        for (const auto& result : results_) {
            if (result.status.find("ERROR") == std::string::npos) {
                validRelativeErrors.push_back(result.relativeError);
                validAbsoluteErrors.push_back(result.absoluteError);
            }
        }
        
        if (!validRelativeErrors.empty()) {
            // Mean errors
            stats.meanRelativeError = std::accumulate(validRelativeErrors.begin(), validRelativeErrors.end(), 0.0) / validRelativeErrors.size();
            stats.meanAbsoluteError = std::accumulate(validAbsoluteErrors.begin(), validAbsoluteErrors.end(), 0.0) / validAbsoluteErrors.size();
            
            // RMSE
            double sumSquaredErrors = 0.0;
            for (double error : validRelativeErrors) {
                sumSquaredErrors += error * error;
            }
            stats.rmse = std::sqrt(sumSquaredErrors / validRelativeErrors.size());
            
            // Min/Max errors
            stats.maxError = *std::max_element(validRelativeErrors.begin(), validRelativeErrors.end());
            stats.minError = *std::min_element(validRelativeErrors.begin(), validRelativeErrors.end());
            
            // Bias calculation (systematic over/under prediction)
            std::vector<double> signedErrors;
            for (const auto& result : results_) {
                if (result.status.find("ERROR") == std::string::npos) {
                    double signedError = (result.calculatedValue - result.expectedValue) / result.expectedValue * 100.0;
                    signedErrors.push_back(signedError);
                }
            }
            stats.bias = std::accumulate(signedErrors.begin(), signedErrors.end(), 0.0) / signedErrors.size();
            
            // Standard deviation
            double variance = 0.0;
            for (double error : validRelativeErrors) {
                variance += (error - stats.meanRelativeError) * (error - stats.meanRelativeError);
            }
            stats.standardDeviation = std::sqrt(variance / (validRelativeErrors.size() - 1));
        }
        
        stats.relativeErrors = validRelativeErrors;
        return stats;
    }
    
    void printDetailedResults() const {
        std::cout << "\n=== Detailed Validation Results ===\n" << std::endl;
        
        // Group results by physics domain
        std::map<std::string, std::vector<ValidationResult>> domainResults;
        
        for (const auto& result : results_) {
            // Find the corresponding benchmark to get physics domain
            std::string domain = "unknown";
            for (const auto& benchmark : benchmarkData_["benchmarks"]) {
                if (benchmark["id"] == result.benchmarkId) {
                    domain = benchmark["physics_domain"];
                    break;
                }
            }
            domainResults[domain].push_back(result);
        }
        
        // Print results grouped by domain
        for (const auto& [domain, domainResultsList] : domainResults) {
            std::cout << "🔬 " << domain << ":" << std::endl;
            for (const auto& result : domainResultsList) {
                std::cout << "  " << std::setw(40) << std::left << result.benchmarkName 
                          << " | " << std::setw(8) << std::right << std::fixed << std::setprecision(1) 
                          << result.calculatedValue << " m/s"
                          << " | Expected: " << std::setw(8) << result.expectedValue << " m/s"
                          << " | Error: " << std::setw(6) << std::setprecision(2) << result.relativeError << "%"
                          << " | " << (result.withinTolerance ? "✅ PASS" : "❌ FAIL") << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
    void printStatisticalSummary() const {
        StatisticalSummary stats = calculateStatistics();
        
        std::cout << "\n=== Statistical Analysis Summary ===\n" << std::endl;
        std::cout << "📊 Overall Performance:" << std::endl;
        std::cout << "   • Total benchmarks: " << stats.totalTests << std::endl;
        std::cout << "   • Passed: " << stats.passedTests << " ✅" << std::endl;
        std::cout << "   • Failed: " << stats.failedTests << " ❌" << std::endl;
        std::cout << "   • Pass rate: " << std::fixed << std::setprecision(1) << stats.passRate << "%" << std::endl;
        
        std::cout << "\n📈 Error Statistics:" << std::endl;
        std::cout << "   • Mean relative error: " << std::setprecision(3) << stats.meanRelativeError << "%" << std::endl;
        std::cout << "   • RMSE: " << stats.rmse << "%" << std::endl;
        std::cout << "   • Maximum error: " << stats.maxError << "%" << std::endl;
        std::cout << "   • Minimum error: " << stats.minError << "%" << std::endl;
        std::cout << "   • Bias (systematic error): " << std::setprecision(3) << stats.bias << "%" << std::endl;
        std::cout << "   • Standard deviation: " << stats.standardDeviation << "%" << std::endl;
        
        // Performance assessment
        std::cout << "\n🎯 Performance Assessment:" << std::endl;
        if (stats.passRate >= 95.0) {
            std::cout << "   ✅ EXCELLENT: >95% validation success rate" << std::endl;
        } else if (stats.passRate >= 90.0) {
            std::cout << "   ✅ VERY GOOD: >90% validation success rate" << std::endl;
        } else if (stats.passRate >= 80.0) {
            std::cout << "   ⚠️  GOOD: >80% validation success rate" << std::endl;
        } else {
            std::cout << "   ❌ NEEDS IMPROVEMENT: <80% validation success rate" << std::endl;
        }
        
        if (std::abs(stats.bias) < 1.0) {
            std::cout << "   ✅ LOW BIAS: Systematic error < 1%" << std::endl;
        } else if (std::abs(stats.bias) < 3.0) {
            std::cout << "   ⚠️  MODERATE BIAS: Systematic error < 3%" << std::endl;
        } else {
            std::cout << "   ❌ HIGH BIAS: Systematic error > 3%" << std::endl;
        }
        
        if (stats.rmse < 5.0) {
            std::cout << "   ✅ LOW SCATTER: RMSE < 5%" << std::endl;
        } else if (stats.rmse < 10.0) {
            std::cout << "   ⚠️  MODERATE SCATTER: RMSE < 10%" << std::endl;
        } else {
            std::cout << "   ❌ HIGH SCATTER: RMSE > 10%" << std::endl;
        }
    }
    
    void printCachePerformance() const {
        std::cout << "\n=== Cache Performance Analysis ===\n" << std::endl;
        std::cout << "📊 Cache Statistics:" << std::endl;
        std::cout << "   • Cache hits: " << wrapper_.getCacheHits() << std::endl;
        std::cout << "   • Cache misses: " << wrapper_.getCacheMisses() << std::endl;
        std::cout << "   • Cache hit rate: " << std::fixed << std::setprecision(1) 
                  << (wrapper_.getCacheHitRate() * 100.0) << "%" << std::endl;
        
        auto metrics = wrapper_.getPerformanceMetrics();
        std::cout << "\n⚡ Performance Metrics:" << std::endl;
        std::cout << "   • Total executions: " << metrics.totalExecutions << std::endl;
        std::cout << "   • Average execution time: " << std::setprecision(2) 
                  << metrics.averageExecutionTime << "ms" << std::endl;
        std::cout << "   • Min execution time: " << metrics.minExecutionTime << "ms" << std::endl;
        std::cout << "   • Max execution time: " << metrics.maxExecutionTime << "ms" << std::endl;
    }
    
    void saveResults(const std::string& filePath) const {
        try {
            json output;
            output["metadata"]["timestamp"] = std::time(nullptr);
            output["metadata"]["total_benchmarks"] = results_.size();
            
            StatisticalSummary stats = calculateStatistics();
            output["statistics"]["total_tests"] = stats.totalTests;
            output["statistics"]["passed_tests"] = stats.passedTests;
            output["statistics"]["failed_tests"] = stats.failedTests;
            output["statistics"]["pass_rate"] = stats.passRate;
            output["statistics"]["mean_relative_error"] = stats.meanRelativeError;
            output["statistics"]["rmse"] = stats.rmse;
            output["statistics"]["bias"] = stats.bias;
            output["statistics"]["max_error"] = stats.maxError;
            output["statistics"]["min_error"] = stats.minError;
            
            // Individual results
            for (const auto& result : results_) {
                json resultJson;
                resultJson["benchmark_id"] = result.benchmarkId;
                resultJson["benchmark_name"] = result.benchmarkName;
                resultJson["calculated_value"] = result.calculatedValue;
                resultJson["expected_value"] = result.expectedValue;
                resultJson["relative_error"] = result.relativeError;
                resultJson["within_tolerance"] = result.withinTolerance;
                resultJson["status"] = result.status;
                resultJson["execution_time_ms"] = result.executionTimeMs;
                resultJson["reference"] = result.reference;
                
                output["results"].push_back(resultJson);
            }
            
            std::ofstream file(filePath);
            file << std::setw(2) << output << std::endl;
            
            std::cout << "\n💾 Results saved to: " << filePath << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Error saving results: " << e.what() << std::endl;
        }
    }
};

int main() {
    std::cout << "🧪 OpenFOAM MCP Server - Comprehensive Validation Suite" << std::endl;
    std::cout << "Literature Benchmark Testing for Cantera Wrapper" << std::endl;
    std::cout << "============================================================\n" << std::endl;
    
    try {
        ComprehensiveValidator validator;
        
        // Load benchmark database
        if (!validator.loadBenchmarks("data/validation_benchmarks.json")) {
            std::cerr << "❌ Failed to load benchmark database" << std::endl;
            return 1;
        }
        
        // Run all benchmarks
        auto startTime = std::chrono::steady_clock::now();
        validator.runAllBenchmarks();
        auto endTime = std::chrono::steady_clock::now();
        
        auto totalTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
        
        // Print detailed results
        validator.printDetailedResults();
        validator.printStatisticalSummary();
        validator.printCachePerformance();
        
        std::cout << "\n=== Validation Suite Completed ===" << std::endl;
        std::cout << "🕒 Total execution time: " << totalTime << " seconds" << std::endl;
        
        // Save results
        validator.saveResults("validation_results.json");
        
        std::cout << "\n✅ Comprehensive validation suite completed successfully!" << std::endl;
        std::cout << "📄 Detailed results saved to validation_results.json" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal error: " << e.what() << std::endl;
        return 1;
    }
}