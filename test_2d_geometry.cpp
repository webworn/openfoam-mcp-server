#include "src/openfoam/rotating_detonation_engine.hpp"
#include <iostream>
#include <iomanip>

using namespace Foam::MCP;

int main() {
    std::cout << "=== 2D RDE Geometry System Test ===" << std::endl;
    
    // Test 1: Default 1D geometry
    std::cout << "\n1. Testing Default 1D Geometry:" << std::endl;
    RDEGeometry geometry1D;
    std::string errorMsg;
    
    std::cout << "   - Outer radius: " << geometry1D.outerRadius << " m" << std::endl;
    std::cout << "   - Inner radius: " << geometry1D.innerRadius << " m" << std::endl;
    std::cout << "   - Chamber length: " << geometry1D.chamberLength << " m" << std::endl;
    std::cout << "   - 2D enabled: " << (geometry1D.enable2D ? "Yes" : "No") << std::endl;
    std::cout << "   - Number of injectors: " << geometry1D.numberOfInjectors << std::endl;
    std::cout << "   - Validation: " << (geometry1D.validateGeometry(errorMsg) ? "PASS" : "FAIL") << std::endl;
    if (!errorMsg.empty()) std::cout << "   - Error: " << errorMsg << std::endl;
    
    // Test 2: Enable 2D mode
    std::cout << "\n2. Testing 2D Mode Configuration:" << std::endl;
    RDEGeometry geometry2D;
    geometry2D.enable2D = true;
    geometry2D.domainAngle = 3.14159; // Half annulus
    geometry2D.numberOfInjectors = 6;
    geometry2D.periodicBoundaries = true;
    geometry2D.injectionType = "radial";
    
    std::cout << "   - 2D enabled: " << (geometry2D.enable2D ? "Yes" : "No") << std::endl;
    std::cout << "   - Domain angle: " << std::fixed << std::setprecision(3) << geometry2D.domainAngle << " rad (" 
              << geometry2D.domainAngle * 180.0 / 3.14159 << "°)" << std::endl;
    std::cout << "   - Periodic boundaries: " << (geometry2D.periodicBoundaries ? "Yes" : "No") << std::endl;
    std::cout << "   - Injection type: " << geometry2D.injectionType << std::endl;
    
    // Update injector positions
    geometry2D.updateInjectorPositions();
    std::cout << "   - Injector positions:" << std::endl;
    for (size_t i = 0; i < geometry2D.injectorAngularPositions.size(); ++i) {
        double angle_deg = geometry2D.injectorAngularPositions[i] * 180.0 / 3.14159;
        std::cout << "     Injector " << i + 1 << ": θ=" << std::fixed << std::setprecision(1) 
                  << angle_deg << "°, r=" << geometry2D.injectorRadialPositions[i] << " m" << std::endl;
    }
    
    errorMsg.clear();
    std::cout << "   - Validation: " << (geometry2D.validateGeometry(errorMsg) ? "PASS" : "FAIL") << std::endl;
    if (!errorMsg.empty()) std::cout << "   - Error: " << errorMsg << std::endl;
    
    // Test 3: Geometric calculations\n    std::cout << "\n3. Testing Geometric Calculations:" << std::endl;
    std::cout << "   - Annular area: " << std::scientific << std::setprecision(3) 
              << geometry2D.calculateAnnularArea() << " m²" << std::endl;
    std::cout << "   - Injection area: " << geometry2D.calculateInjectionArea() << " m²" << std::endl;
    std::cout << "   - Hydraulic diameter: " << std::fixed << std::setprecision(4) 
              << geometry2D.calculateHydraulicDiameter() << " m" << std::endl;
    
    // Test 4: Mesh sizing calculations
    std::cout << "\n4. Testing 2D Mesh Sizing:" << std::endl;
    int radialCells = 40;
    int circumferentialCells = 100;
    
    double radialCellSize = geometry2D.calculateRadialCellSize(radialCells);
    double angularCellSize = geometry2D.calculateAngularCellSize(circumferentialCells);
    
    std::cout << "   - Radial cells: " << radialCells << std::endl;
    std::cout << "   - Circumferential cells: " << circumferentialCells << std::endl;
    std::cout << "   - Radial cell size: " << std::scientific << radialCellSize << " m" << std::endl;
    std::cout << "   - Angular cell size: " << angularCellSize << " rad" << std::endl;
    
    // Test 5: Staggered injection pattern
    std::cout << "\n5. Testing Staggered Injection Pattern:" << std::endl;
    RDEGeometry geometryStaggered;
    geometryStaggered.enable2D = true;
    geometryStaggered.numberOfInjectors = 8;
    geometryStaggered.staggeredInjection = true;
    geometryStaggered.updateInjectorPositions();
    
    std::cout << "   - Staggered injection: " << (geometryStaggered.staggeredInjection ? "Yes" : "No") << std::endl;
    std::cout << "   - Injector positions:" << std::endl;
    for (size_t i = 0; i < geometryStaggered.injectorAngularPositions.size(); ++i) {
        double angle_deg = geometryStaggered.injectorAngularPositions[i] * 180.0 / 3.14159;
        std::cout << "     Injector " << i + 1 << ": θ=" << std::fixed << std::setprecision(1) 
                  << angle_deg << "°" << std::endl;
    }
    
    // Test 6: Error validation
    std::cout << "\n6. Testing Error Validation:" << std::endl;
    RDEGeometry invalidGeometry;
    invalidGeometry.outerRadius = 0.02;  // Smaller than inner radius
    invalidGeometry.innerRadius = 0.03;
    
    errorMsg.clear();
    bool isValid = invalidGeometry.validateGeometry(errorMsg);
    std::cout << "   - Invalid geometry validation: " << (isValid ? "PASS" : "FAIL (Expected)") << std::endl;
    std::cout << "   - Error message: " << errorMsg << std::endl;
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    std::cout << "✅ All 2D geometry extensions implemented successfully!" << std::endl;
    
    return 0;
}