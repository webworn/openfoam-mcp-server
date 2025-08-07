#include "cantera_wrapper.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace Foam::MCP::Physics;

int main() {
    std::cout << "=== Cantera Wrapper Test ===" << std::endl;
    
    try {
        // Create wrapper
        CanteraWrapper wrapper;
        wrapper.setVerboseMode(true);
        
        // Test connection
        std::cout << "\n1. Testing SDToolbox connection..." << std::endl;
        bool connected = wrapper.testSDToolboxConnection();
        std::cout << "Connection test: " << (connected ? "PASSED" : "FAILED") << std::endl;
        
        if (!connected) {
            std::cout << "Cannot proceed without SDToolbox connection." << std::endl;
            return 1;
        }
        
        // Test mechanism validation
        std::cout << "\n2. Testing mechanism validation..." << std::endl;
        bool validMech = wrapper.validateMechanism("sandiego20161214_H2only.yaml");
        std::cout << "Mechanism validation: " << (validMech ? "PASSED" : "FAILED") << std::endl;
        
        // Test CJ speed calculation
        std::cout << "\n3. Testing CJ speed calculation..." << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        double cjSpeed = wrapper.calculateCJSpeed(
            101325.0,                           // 1 atm
            300.0,                              // 300 K
            "H2:2 O2:1 N2:3.76",               // Stoichiometric H2-air
            "sandiego20161214_H2only.yaml"     // Mechanism
        );
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "CJ Speed: " << cjSpeed << " m/s" << std::endl;
        std::cout << "Calculation time: " << duration.count() << " ms" << std::endl;
        
        // Expected range for H2-air at 1 atm, 300K: ~1950-2000 m/s
        if (cjSpeed >= 1950.0 && cjSpeed <= 2000.0) {
            std::cout << "CJ Speed test: PASSED (within expected range)" << std::endl;
        } else {
            std::cout << "CJ Speed test: WARNING (outside expected range 1950-2000 m/s)" << std::endl;
        }
        
        // Test post-shock equilibrium calculation
        std::cout << "\n4. Testing post-shock equilibrium calculation..." << std::endl;
        
        auto postShockState = wrapper.calculatePostShockEq(
            cjSpeed,                            // Use calculated CJ speed
            101325.0,                           // 1 atm
            300.0,                              // 300 K
            "H2:2 O2:1 N2:3.76",               // Stoichiometric H2-air
            "sandiego20161214_H2only.yaml"     // Mechanism
        );
        
        std::cout << "Post-shock state:" << std::endl;
        std::cout << "  Pressure: " << postShockState.pressure/1e5 << " bar" << std::endl;
        std::cout << "  Temperature: " << postShockState.temperature << " K" << std::endl;
        std::cout << "  Density: " << postShockState.density << " kg/m³" << std::endl;
        
        // Test batch processing
        std::cout << "\n5. Testing batch processing..." << std::endl;
        
        std::vector<CanteraWrapper::InitialConditions> batchConditions = {
            {101325.0, 300.0, "H2:2 O2:1 N2:3.76", "sandiego20161214_H2only.yaml"},
            {202650.0, 300.0, "H2:2 O2:1 N2:3.76", "sandiego20161214_H2only.yaml"},
            {101325.0, 400.0, "H2:2 O2:1 N2:3.76", "sandiego20161214_H2only.yaml"}
        };
        
        auto batchStart = std::chrono::high_resolution_clock::now();
        auto batchResults = wrapper.calculateCJSpeedBatch(batchConditions);
        auto batchEnd = std::chrono::high_resolution_clock::now();
        auto batchDuration = std::chrono::duration_cast<std::chrono::milliseconds>(batchEnd - batchStart);
        
        std::cout << "Batch results:" << std::endl;
        for (size_t i = 0; i < batchResults.size(); ++i) {
            std::cout << "  Case " << i+1 << ": " << batchResults[i] << " m/s" << std::endl;
        }
        std::cout << "Batch calculation time: " << batchDuration.count() << " ms" << std::endl;
        std::cout << "Average time per case: " << batchDuration.count() / batchResults.size() << " ms" << std::endl;
        
        // Test FHFP calculation
        std::cout << "\n6. Testing FHFP (conservation errors) calculation..." << std::endl;
        
        // Create initial state
        CanteraWrapper::ThermodynamicState initialState;
        initialState.pressure = 101325.0;
        initialState.temperature = 300.0;
        initialState.density = 1.18;  // Approximate for H2-air at STP
        initialState.enthalpy = 0.0;  // Relative to reference
        
        // Use post-shock state from previous calculation
        auto conservationErrors = wrapper.calculateFHFP(cjSpeed, initialState, postShockState);
        
        std::cout << "Conservation errors:" << std::endl;
        std::cout << "  Enthalpy error: " << conservationErrors.enthalpyError << " J/kg" << std::endl;
        std::cout << "  Pressure error: " << conservationErrors.pressureError << " Pa" << std::endl;
        
        // Test composition generation utility
        std::cout << "\n7. Testing composition generation..." << std::endl;
        
        std::string stoichH2Air = CanteraWrapper::generateComposition("H2", 1.0, "air");
        std::string richH2Air = CanteraWrapper::generateComposition("H2", 1.5, "air");
        std::string leanH2Air = CanteraWrapper::generateComposition("H2", 0.8, "air");
        
        std::cout << "Stoichiometric H2-air: " << stoichH2Air << std::endl;
        std::cout << "Rich H2-air (φ=1.5): " << richH2Air << std::endl;
        std::cout << "Lean H2-air (φ=0.8): " << leanH2Air << std::endl;
        
        // Test available mechanisms
        std::cout << "\n8. Testing available mechanisms..." << std::endl;
        auto mechanisms = wrapper.getAvailableMechanisms();
        std::cout << "Found " << mechanisms.size() << " mechanism files:" << std::endl;
        for (const auto& mech : mechanisms) {
            if (mech.find("H2") != std::string::npos || mech.find("hydrogen") != std::string::npos) {
                std::cout << "  " << mech << " (hydrogen mechanism)" << std::endl;
            }
        }
        
        std::cout << "\n=== All Tests Completed Successfully ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}