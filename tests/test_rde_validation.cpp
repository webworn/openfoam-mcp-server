#include "../src/validation/rde_validation_framework.hpp"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== RDE Validation Framework Test ===" << std::endl;
    std::cout << std::endl;
    
    try {
        // Create validation framework
        Foam::MCP::RDEValidationFramework validator;
        validator.setVerboseMode(true);
        
        std::cout << "1. Running Fast Analytical Validation..." << std::endl;
        auto fastResult = validator.runValidation(Foam::MCP::RDEValidationFramework::ValidationMode::FAST);
        
        std::cout << std::endl;
        std::cout << "=== Fast Validation Results ===" << std::endl;
        std::cout << fastResult.summaryReport << std::endl;
        
        std::cout << "2. Running Standard Cellular Validation..." << std::endl;
        auto standardResult = validator.runValidation(Foam::MCP::RDEValidationFramework::ValidationMode::STANDARD);
        
        std::cout << std::endl;
        std::cout << "=== Standard Validation Results ===" << std::endl;
        std::cout << standardResult.summaryReport << std::endl;
        
        std::cout << "3. Running Comprehensive Validation..." << std::endl;
        auto comprehensiveResult = validator.runValidation(Foam::MCP::RDEValidationFramework::ValidationMode::COMPREHENSIVE);
        
        std::cout << std::endl;
        std::cout << "=== Comprehensive Validation Results ===" << std::endl;
        std::cout << comprehensiveResult.summaryReport << std::endl;
        
        std::cout << "4. Testing Individual Validation Cases..." << std::endl;
        
        // Test individual case
        auto testCases = validator.createAnalyticalValidationSuite();
        if (!testCases.empty()) {
            auto singleResult = validator.validateSingleCase(testCases[0]);
            
            std::cout << std::endl;
            std::cout << "=== Single Case Validation: " << singleResult.caseName << " ===" << std::endl;
            std::cout << "Status: " << (singleResult.passed ? "PASSED" : "FAILED") << std::endl;
            std::cout << "Overall Accuracy: " << std::fixed << std::setprecision(2) 
                      << singleResult.overallAccuracy * 100 << "%" << std::endl;
            std::cout << std::endl;
            
            std::cout << "Predictions vs Expected:" << std::endl;
            std::cout << "  C-J Velocity: " << std::fixed << std::setprecision(1) 
                      << singleResult.predictedCJVelocity << " m/s"
                      << " (Error: " << std::fixed << std::setprecision(2) 
                      << singleResult.velocityError << "%)" << std::endl;
            std::cout << "  Cell Size: " << std::scientific << std::setprecision(3) 
                      << singleResult.predictedCellSize << " m"
                      << " (Error: " << std::fixed << std::setprecision(2) 
                      << singleResult.cellSizeError << "%)" << std::endl;
            std::cout << "  Pressure Ratio: " << std::fixed << std::setprecision(1) 
                      << singleResult.predictedPressureRatio
                      << " (Error: " << std::fixed << std::setprecision(2) 
                      << singleResult.pressureError << "%)" << std::endl;
            std::cout << "  Frequency: " << std::scientific << std::setprecision(3) 
                      << singleResult.predictedFrequency << " Hz"
                      << " (Error: " << std::fixed << std::setprecision(2) 
                      << singleResult.frequencyError << "%)" << std::endl;
            
            if (!singleResult.warnings.empty()) {
                std::cout << std::endl;
                std::cout << "Warnings:" << std::endl;
                for (const auto& warning : singleResult.warnings) {
                    std::cout << "  - " << warning << std::endl;
                }
            }
        }
        
        // Generate validation reports
        std::cout << std::endl;
        std::cout << "5. Generating Validation Reports..." << std::endl;
        
        validator.generateValidationReport(fastResult, "validation_results/fast_validation_report.txt");
        validator.generateValidationReport(standardResult, "validation_results/standard_validation_report.txt");
        validator.generateValidationReport(comprehensiveResult, "validation_results/comprehensive_validation_report.txt");
        
        std::cout << std::endl;
        std::cout << "=== Validation Framework Test Complete ===" << std::endl;
        std::cout << "Results saved to validation_results/ directory" << std::endl;
        
        // Summary of all validation modes
        std::cout << std::endl;
        std::cout << "=== Summary of All Validation Modes ===" << std::endl;
        std::cout << std::setw(20) << "Mode" << std::setw(15) << "Cases" 
                  << std::setw(15) << "Passed" << std::setw(15) << "Accuracy" << std::endl;
        std::cout << std::string(65, '-') << std::endl;
        
        std::cout << std::setw(20) << "Fast" 
                  << std::setw(15) << fastResult.totalCases
                  << std::setw(15) << fastResult.passedCases
                  << std::setw(15) << std::fixed << std::setprecision(1) 
                  << fastResult.overallAccuracy * 100 << "%" << std::endl;
        
        std::cout << std::setw(20) << "Standard" 
                  << std::setw(15) << standardResult.totalCases
                  << std::setw(15) << standardResult.passedCases
                  << std::setw(15) << std::fixed << std::setprecision(1) 
                  << standardResult.overallAccuracy * 100 << "%" << std::endl;
        
        std::cout << std::setw(20) << "Comprehensive" 
                  << std::setw(15) << comprehensiveResult.totalCases
                  << std::setw(15) << comprehensiveResult.passedCases
                  << std::setw(15) << std::fixed << std::setprecision(1) 
                  << comprehensiveResult.overallAccuracy * 100 << "%" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}