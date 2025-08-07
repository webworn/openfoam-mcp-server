#include "src/physics/cantera_wrapper.hpp"
#include <iostream>

using namespace Foam::MCP::Physics;

int main() {
    std::cout << "=== Validation Framework Test ===" << std::endl;
    
    try {
        CanteraWrapper wrapper;
        wrapper.setVerboseMode(true);
        
        // Enable validation framework
        wrapper.enableValidation(true);
        
        std::cout << "Validation enabled: " << (wrapper.isValidationEnabled() ? "Yes" : "No") << std::endl;
        
        // Get standard benchmarks
        auto benchmarks = wrapper.getStandardBenchmarks();
        std::cout << "Standard benchmarks loaded: " << benchmarks.size() << std::endl;
        
        for (const auto& benchmark : benchmarks) {
            std::cout << "  - " << benchmark.name << ": " << benchmark.description << std::endl;
            std::cout << "    Expected CJ speed: " << benchmark.expectedCJSpeed << " m/s" << std::endl;
            std::cout << "    Tolerance: " << (benchmark.tolerance * 100.0) << "%" << std::endl;
        }
        
        // Run validation suite
        std::cout << "\nRunning validation suite..." << std::endl;
        auto report = wrapper.runValidationSuite(true);
        
        std::cout << "\n=== Validation Report ===" << std::endl;
        std::cout << "Total tests: " << report.totalTests << std::endl;
        std::cout << "Passed: " << report.passedTests << std::endl;
        std::cout << "Failed: " << report.failedTests << std::endl;
        std::cout << "Overall accuracy: " << report.overallAccuracy << "%" << std::endl;
        std::cout << "Average error: " << report.averageError << "%" << std::endl;
        
        // Show detailed results
        std::cout << "\nDetailed results:" << std::endl;
        for (const auto& result : report.results) {
            std::cout << "  " << result.testCase << ":" << std::endl;
            std::cout << "    Calculated: " << result.calculatedValue << " m/s" << std::endl;
            std::cout << "    Expected: " << result.expectedValue << " m/s" << std::endl;
            std::cout << "    Error: " << result.relativeError << "%" << std::endl;
            std::cout << "    Within tolerance: " << (result.withinTolerance ? "✓" : "✗") << std::endl;
            std::cout << "    Execution time: " << result.executionTimeMs << " ms" << std::endl;
        }
        
        // Save validation report
        wrapper.saveValidationReport(report, "validation_report.json");
        std::cout << "\nValidation report saved to validation_report.json" << std::endl;
        
        // Show current accuracy score
        std::cout << "Current accuracy score: " << wrapper.getCurrentAccuracyScore() << std::endl;
        
        std::cout << "\n=== Validation Framework Test Completed ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}