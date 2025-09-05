#include "cellular_detonation_2d.hpp"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>

namespace Foam {
namespace MCP {

CellularDetonation2D::CellularDetonation2D() {
    initializeEducationalContent();
}

CellularDetonation2D::CellularStructure2D 
CellularDetonation2D::analyze2DCellularStructure(const RDEGeometry& geometry, 
                                                const RDEChemistry& chemistry) {
    CellularStructure2D structure;
    
    // Calculate base cell size using inherited 1D model
    // Note: Need to cast away const to call non-const inherited method
    double baseCellSize = const_cast<CellularDetonation2D*>(this)->calculateCellSize(chemistry);
    structure.meanCellSize = baseCellSize;
    
    // Apply curvature corrections for annular geometry
    double curvatureEffect = calculateCurvatureEffect(
        (geometry.innerRadius + geometry.outerRadius) / 2.0, baseCellSize);
    structure.curvatureEffect = curvatureEffect;
    structure.meanCellSize *= (1.0 + curvatureEffect);
    
    // Initialize 2D cellular field
    initialize2DCellularField(structure, geometry, chemistry);
    
    // Calculate variation statistics
    structure.radialVariation = 0.15 * structure.meanCellSize;      // 15% typical variation
    structure.circumferentialVariation = 0.10 * structure.meanCellSize; // 10% typical variation
    structure.structureRegularity = 0.75;  // 75% regularity for typical RDE conditions
    
    // Calculate wave angle (typically 10-30 degrees from radial)
    structure.waveAngle = 0.35;  // ~20 degrees in radians
    
    return structure;
}

CellularDetonation2D::WavePropagation2D 
CellularDetonation2D::track2DWavePropagation(const RDEGeometry& geometry,
                                           const RDEChemistry& chemistry,
                                           const std::vector<Wave2DPoint>& initialWave,
                                           double simulationTime) {
    WavePropagation2D waveProp;
    waveProp.waveTrajectory = initialWave;
    
    // Calculate average propagation speed
    waveProp.propagationSpeed = chemistry.detonationVelocity;
    
    // Estimate speed variation around annulus (5-15% typical)
    waveProp.speedVariation = 0.10 * waveProp.propagationSpeed;
    
    // Generate local wave speeds around circumference
    int numPoints = static_cast<int>(geometry.domainAngle / 0.1); // Every ~6 degrees
    waveProp.localWaveSpeeds.reserve(numPoints);
    
    for (int i = 0; i < numPoints; ++i) {
        double angle = i * 0.1; // 0.1 radian increments
        
        // Add some variation based on position (simplified model)
        double speedVariation = 1.0 + 0.1 * std::sin(4.0 * angle); // 4 variations around annulus
        waveProp.localWaveSpeeds.push_back(waveProp.propagationSpeed * speedVariation);
    }
    
    // Estimate wave thickness (typically 2-5 cell sizes)
    waveProp.waveThickness = 3.0 * calculateCellSize(chemistry);
    
    // Predict wave collision points (simplified)
    if (!initialWave.empty()) {
        double circumference = geometry.outerRadius * geometry.domainAngle;
        double waveSpacing = circumference / std::max(1, static_cast<int>(initialWave.size()));
        
        // Add collision points where waves might meet
        for (size_t i = 0; i < initialWave.size(); ++i) {
            double theta = initialWave[i].theta + M_PI; // Opposite side
            if (theta > geometry.domainAngle) theta -= geometry.domainAngle;
            
            waveProp.waveCollisionPoints.emplace_back(
                (geometry.innerRadius + geometry.outerRadius) / 2.0, theta);
        }
    }
    
    // Estimate energy dissipation
    waveProp.energyDissipation = 0.05; // 5% energy dissipated per revolution (typical)
    
    return waveProp;
}

std::vector<CellularDetonation2D::InjectionWaveInteraction>
CellularDetonation2D::analyzeInjectionWaveInteractions(
    const RDEGeometry& geometry,
    const RDEChemistry& chemistry, 
    const WavePropagation2D& waveProp) {
    
    std::vector<InjectionWaveInteraction> interactions;
    
    // Analyze each injector
    for (size_t i = 0; i < geometry.injectorAngularPositions.size(); ++i) {
        InjectionWaveInteraction interaction;
        
        interaction.injectorIndex = static_cast<int>(i);
        interaction.injectorPosition_theta = geometry.injectorAngularPositions[i];
        
        // Calculate wave phase at injector position
        // Simplified: assume wave passes injector every revolution
        double revolutionTime = 2.0 * M_PI * geometry.outerRadius / chemistry.detonationVelocity;
        interaction.wavePhaseAtInjection = 2.0 * M_PI * 
            (interaction.injectorPosition_theta / geometry.domainAngle);
        
        // Calculate momentum coupling
        double injectionMomentum = chemistry.injectionVelocity * 1.0; // Assume 1 kg/m³ density
        double waveInertia = 1000.0 * chemistry.detonationVelocity;   // Approximate wave inertia
        interaction.momentumCoupling = injectionMomentum / waveInertia;
        
        // Estimate pressure disturbance
        interaction.pressureDisturbance = 0.1 * chemistry.detonationPressure; // 10% of CJ pressure
        
        // Set interaction type based on injection angle and wave direction
        if (geometry.injectionAngle > 80.0 && geometry.injectionAngle < 100.0) {
            interaction.interactionType = "neutral";  // Perpendicular injection
        } else if (geometry.injectionAngle < 80.0) {
            interaction.interactionType = "reinforcing"; // Forward injection
        } else {
            interaction.interactionType = "opposing";    // Backward injection  
        }
        
        // Estimate penetration depth
        interaction.penetrationDepth = geometry.injectionPenetration;
        
        interactions.push_back(interaction);
    }
    
    return interactions;
}

CellularDetonation2D::MultiWaveSystem 
CellularDetonation2D::analyzeMultiWaveSystem(const RDEGeometry& geometry,
                                            const RDEChemistry& chemistry,
                                            const std::vector<WavePropagation2D>& waves) {
    MultiWaveSystem system;
    
    system.waveCount = static_cast<int>(waves.size());
    system.waves = waves;
    
    if (system.waveCount > 1) {
        // Calculate wave spacings
        for (size_t i = 0; i < waves.size(); ++i) {
            if (!waves[i].waveTrajectory.empty()) {
                double currentTheta = waves[i].waveTrajectory[0].theta;
                double nextTheta = (i + 1 < waves.size()) ? 
                    waves[i + 1].waveTrajectory[0].theta : 
                    waves[0].waveTrajectory[0].theta + geometry.domainAngle;
                
                double spacing = nextTheta - currentTheta;
                if (spacing < 0) spacing += geometry.domainAngle;
                
                system.waveSpacings.emplace_back(currentTheta, spacing);
            }
        }
        
        // Determine wave pattern
        double avgSpacing = geometry.domainAngle / system.waveCount;
        double spacingVariation = 0.0;
        
        for (const auto& spacing : system.waveSpacings) {
            spacingVariation += std::abs(spacing.second - avgSpacing);
        }
        spacingVariation /= system.waveSpacings.size();
        
        if (spacingVariation < 0.1 * avgSpacing) {
            system.wavePattern = "co_rotating";
            system.stabilityIndex = 0.9;
        } else if (spacingVariation < 0.3 * avgSpacing) {
            system.wavePattern = "mixed";
            system.stabilityIndex = 0.6;
        } else {
            system.wavePattern = "counter_rotating";
            system.stabilityIndex = 0.4;
        }
        
        // Calculate system frequency
        double avgWaveSpeed = 0.0;
        for (const auto& wave : waves) {
            avgWaveSpeed += wave.propagationSpeed;
        }
        avgWaveSpeed /= waves.size();
        
        double circumference = geometry.outerRadius * geometry.domainAngle;
        system.systemFrequency = avgWaveSpeed * system.waveCount / circumference;
        
        // Predict collision pairs
        for (int i = 0; i < system.waveCount; ++i) {
            for (int j = i + 1; j < system.waveCount; ++j) {
                // Simple collision prediction: waves with similar speeds might collide
                if (std::abs(waves[i].propagationSpeed - waves[j].propagationSpeed) < 50.0) {
                    system.collisionPairs.emplace_back(i, j);
                }
            }
        }
    } else {
        // Single wave system
        system.wavePattern = "single_wave";
        system.stabilityIndex = 0.8;
        system.systemFrequency = (waves.empty()) ? 0.0 : 
            waves[0].propagationSpeed / (geometry.outerRadius * geometry.domainAngle);
    }
    
    return system;
}

std::pair<double, double> CellularDetonation2D::cartesianToCylindrical(double x, double y) const {
    double r = std::sqrt(x*x + y*y);
    double theta = std::atan2(y, x);
    if (theta < 0) theta += 2.0 * M_PI;
    return {r, theta};
}

std::pair<double, double> CellularDetonation2D::cylindricalToCartesian(double r, double theta) const {
    double x = r * std::cos(theta);
    double y = r * std::sin(theta);
    return {x, y};
}

bool CellularDetonation2D::validate2DCellularConstraints(const RDEGeometry& geometry,
                                                       const RDEChemistry& chemistry,
                                                       const RDESimulationSettings& settings,
                                                       std::string& report) const {
    std::stringstream ss;
    bool allConstraintsSatisfied = true;
    
    // Analyze 2D cellular structure
    CellularStructure2D structure;
    structure.meanCellSize = const_cast<CellularDetonation2D*>(this)->calculateCellSize(chemistry);
    structure.curvatureEffect = calculateCurvatureEffect(
        (geometry.innerRadius + geometry.outerRadius) / 2.0, structure.meanCellSize);
    structure.meanCellSize *= (1.0 + structure.curvatureEffect);
    
    // Calculate required mesh size
    double requiredMeshSize = calculate2DRequiredMeshSize(structure, geometry, 10.0);
    
    ss << "=== 2D Cellular Constraint Validation ===\n";
    ss << "Base cell size: " << std::scientific << const_cast<CellularDetonation2D*>(this)->calculateCellSize(chemistry) << " m\n";
    ss << "Curvature effect: " << std::fixed << std::setprecision(1) 
       << structure.curvatureEffect * 100.0 << "%\n";
    ss << "Effective 2D cell size: " << std::scientific << structure.meanCellSize << " m\n";
    ss << "Required mesh size (λ/10): " << requiredMeshSize << " m\n\n";
    
    // Check radial direction constraint
    double radialCellSize = (geometry.outerRadius - geometry.innerRadius) / settings.radialCells;
    bool radialOK = (radialCellSize <= requiredMeshSize);
    ss << "Radial direction:\n";
    ss << "  Cells: " << settings.radialCells << "\n";
    ss << "  Cell size: " << std::scientific << radialCellSize << " m\n";
    ss << "  Constraint: " << (radialOK ? "SATISFIED" : "VIOLATED") << "\n\n";
    
    // Check circumferential direction constraint  
    double circumferentialLength = geometry.outerRadius * geometry.domainAngle;
    double circumferentialCellSize = circumferentialLength / settings.circumferentialCells;
    bool circumferentialOK = (circumferentialCellSize <= requiredMeshSize);
    ss << "Circumferential direction:\n";
    ss << "  Cells: " << settings.circumferentialCells << "\n";
    ss << "  Cell size: " << std::scientific << circumferentialCellSize << " m\n";
    ss << "  Constraint: " << (circumferentialOK ? "SATISFIED" : "VIOLATED") << "\n\n";
    
    // Check axial direction constraint
    double axialCellSize = geometry.chamberLength / settings.axialCells;
    bool axialOK = (axialCellSize <= requiredMeshSize);
    ss << "Axial direction:\n";
    ss << "  Cells: " << settings.axialCells << "\n";
    ss << "  Cell size: " << std::scientific << axialCellSize << " m\n";
    ss << "  Constraint: " << (axialOK ? "SATISFIED" : "VIOLATED") << "\n\n";
    
    allConstraintsSatisfied = radialOK && circumferentialOK && axialOK;
    
    ss << "Overall validation: " << (allConstraintsSatisfied ? "PASS" : "FAIL") << "\n";
    
    if (!allConstraintsSatisfied) {
        ss << "\n=== Recommended Corrections ===\n";
        if (!radialOK) {
            int minRadialCells = static_cast<int>(std::ceil(
                (geometry.outerRadius - geometry.innerRadius) / requiredMeshSize));
            ss << "Increase radial cells to at least: " << minRadialCells << "\n";
        }
        if (!circumferentialOK) {
            int minCircumferentialCells = static_cast<int>(std::ceil(
                circumferentialLength / requiredMeshSize));
            ss << "Increase circumferential cells to at least: " << minCircumferentialCells << "\n";
        }
        if (!axialOK) {
            int minAxialCells = static_cast<int>(std::ceil(
                geometry.chamberLength / requiredMeshSize));
            ss << "Increase axial cells to at least: " << minAxialCells << "\n";
        }
    }
    
    report = ss.str();
    return allConstraintsSatisfied;
}

double CellularDetonation2D::calculate2DRequiredMeshSize(const CellularStructure2D& structure,
                                                       const RDEGeometry& geometry,
                                                       double safetyFactor) const {
    // Account for curvature effects and 2D cellular variations
    double effectiveCellSize = structure.meanCellSize * (1.0 + structure.radialVariation + 
                                                        structure.circumferentialVariation);
    return effectiveCellSize / safetyFactor;
}

double CellularDetonation2D::calculateCurvatureEffect(double radius, double cellSize) const {
    // Curvature effect becomes significant when radius approaches cell size
    // Using empirical correlation: effect = exp(-radius/cellSize) * 0.5
    if (cellSize <= 0.0) return 0.0;
    
    double ratio = radius / cellSize;
    if (ratio > 100.0) return 0.0; // Negligible effect for large radii
    
    double curvatureEffect = 0.5 * std::exp(-ratio / 10.0); // Damped exponential
    return std::min(curvatureEffect, 0.3); // Cap at 30% effect
}

std::string CellularDetonation2D::explain2DWaveDynamics(const WavePropagation2D& waveProp) const {
    std::stringstream explanation;
    
    explanation << "=== 2D Wave Dynamics in Cylindrical RDE ===\n\n";
    
    explanation << "Wave Propagation Characteristics:\n";
    explanation << "• Average speed: " << std::fixed << std::setprecision(0) 
                << waveProp.propagationSpeed << " m/s\n";
    explanation << "• Speed variation: ±" << std::setprecision(1) 
                << (waveProp.speedVariation/waveProp.propagationSpeed)*100.0 << "%\n";
    explanation << "• Wave thickness: " << std::scientific << std::setprecision(2) 
                << waveProp.waveThickness << " m\n\n";
    
    explanation << "Cylindrical Coordinate Effects:\n";
    explanation << "In annular RDE geometry, detonation waves experience several unique phenomena:\n\n";
    
    explanation << "1. **Curvature Effects**: Wave fronts naturally curve due to the annular geometry\n";
    explanation << "   - Inner radius waves move slower (shorter path)\n";
    explanation << "   - Outer radius waves move faster (longer path)\n";
    explanation << "   - This creates inherent wave front curvature\n\n";
    
    explanation << "2. **Angular Variations**: Wave speed varies around the circumference\n";
    explanation << "   - Injection regions create local disturbances\n";
    explanation << "   - Flow blockages affect local wave speeds\n";
    explanation << "   - Geometric imperfections cause speed variations\n\n";
    
    explanation << "3. **Wave Interactions**: Multiple waves can exist simultaneously\n";
    if (!waveProp.waveCollisionPoints.empty()) {
        explanation << "   - Predicted collision points: " << waveProp.waveCollisionPoints.size() << "\n";
        explanation << "   - Collisions create complex pressure patterns\n";
        explanation << "   - Energy redistribution occurs during interactions\n\n";
    }
    
    explanation << "Educational Context:\n";
    explanation << "Understanding 2D wave dynamics is crucial for RDE design because:\n";
    explanation << "• Wave stability determines engine performance\n";
    explanation << "• Multi-wave modes can increase thrust and efficiency\n"; 
    explanation << "• Wave-injection coupling affects combustion efficiency\n";
    explanation << "• Proper wave management prevents destructive oscillations\n";
    
    return explanation.str();
}

std::string CellularDetonation2D::explainInjectionCoupling(
    const std::vector<InjectionWaveInteraction>& interactions) const {
    
    std::stringstream explanation;
    
    explanation << "=== Injection-Wave Coupling Analysis ===\n\n";
    
    explanation << "Injection Configuration:\n";
    explanation << "• Number of injectors: " << interactions.size() << "\n";
    
    // Analyze interaction types
    int reinforcing = 0, opposing = 0, neutral = 0;
    for (const auto& interaction : interactions) {
        if (interaction.interactionType == "reinforcing") reinforcing++;
        else if (interaction.interactionType == "opposing") opposing++;
        else neutral++;
    }
    
    explanation << "• Reinforcing interactions: " << reinforcing << "\n";
    explanation << "• Opposing interactions: " << opposing << "\n";
    explanation << "• Neutral interactions: " << neutral << "\n\n";
    
    explanation << "Physical Mechanisms:\n";
    explanation << "Injection-wave coupling occurs through several mechanisms:\n\n";
    
    explanation << "1. **Momentum Transfer**:\n";
    explanation << "   - Fresh reactants inject momentum into the flow\n";
    explanation << "   - Can accelerate or decelerate passing waves\n";
    explanation << "   - Depends on injection angle relative to wave direction\n\n";
    
    explanation << "2. **Chemical Energy Addition**:\n";
    explanation << "   - Fresh fuel-oxidizer mixture provides chemical energy\n";
    explanation << "   - Enhances combustion behind the wave front\n";
    explanation << "   - Can strengthen or sustain detonation waves\n\n";
    
    explanation << "3. **Flow Blockage Effects**:\n";
    explanation << "   - Injection creates local flow disturbances\n";
    explanation << "   - Can cause wave reflection or diffraction\n";
    explanation << "   - Affects wave propagation patterns\n\n";
    
    explanation << "Design Implications:\n";
    for (size_t i = 0; i < std::min(interactions.size(), size_t(3)); ++i) {
        explanation << "• Injector " << i + 1 << ": " << interactions[i].interactionType 
                    << " interaction\n";
        explanation << "  - Momentum coupling: " << std::scientific << std::setprecision(2)
                    << interactions[i].momentumCoupling << "\n";
        explanation << "  - Penetration depth: " << std::fixed << std::setprecision(3)
                    << interactions[i].penetrationDepth*1000.0 << " mm\n";
    }
    
    explanation << "\nOptimization Strategy:\n";
    explanation << "• Maximize reinforcing interactions for better performance\n";
    explanation << "• Consider injection timing relative to wave passage\n";
    explanation << "• Balance injection momentum with wave stability requirements\n";
    
    return explanation.str();
}

void CellularDetonation2D::initialize2DCellularField(CellularStructure2D& structure,
                                                   const RDEGeometry& geometry,
                                                   const RDEChemistry& chemistry) const {
    // Initialize 2D cellular field with reasonable resolution
    int nR = 20;  // Radial points
    int nTheta = 40; // Angular points
    
    structure.cellSizeField.resize(nR);
    for (int i = 0; i < nR; ++i) {
        structure.cellSizeField[i].resize(nTheta);
        
        double r = geometry.innerRadius + 
                  i * (geometry.outerRadius - geometry.innerRadius) / (nR - 1);
        
        for (int j = 0; j < nTheta; ++j) {
            double theta = j * geometry.domainAngle / (nTheta - 1);
            
            // Apply variations based on position  
            double cellSize = structure.meanCellSize;
            
            // Add radial variation (smaller cells near walls due to boundary layer effects)
            if (r < geometry.innerRadius + 0.1 * (geometry.outerRadius - geometry.innerRadius)) {
                cellSize *= 0.9; // 10% smaller near inner wall
            } else if (r > geometry.outerRadius - 0.1 * (geometry.outerRadius - geometry.innerRadius)) {
                cellSize *= 0.95; // 5% smaller near outer wall
            }
            
            // Add circumferential variation near injectors
            for (size_t k = 0; k < geometry.injectorAngularPositions.size(); ++k) {
                double injectorTheta = geometry.injectorAngularPositions[k];
                double angleDistanceToInjector = std::abs(theta - injectorTheta);
                
                // Normalize angle difference
                if (angleDistanceToInjector > geometry.domainAngle / 2.0) {
                    angleDistanceToInjector = geometry.domainAngle - angleDistanceToInjector;
                }
                
                // Cell size perturbation near injectors
                if (angleDistanceToInjector < 0.2) { // Within ~11 degrees of injector
                    double perturbation = 1.0 + 0.2 * std::exp(-angleDistanceToInjector / 0.1);
                    cellSize *= perturbation;
                }
            }
            
            structure.cellSizeField[i][j] = cellSize;
        }
    }
    
    // Initialize triple point locations (simplified)
    structure.triplePoints.clear();
    for (size_t i = 0; i < geometry.injectorAngularPositions.size(); ++i) {
        Wave2DPoint triplePoint;
        triplePoint.r = (geometry.innerRadius + geometry.outerRadius) / 2.0;
        triplePoint.theta = geometry.injectorAngularPositions[i] + 0.1; // Offset from injector
        triplePoint.time = 0.0;
        triplePoint.temperature = 3500.0; // High temperature at triple point
        triplePoint.pressure = 3.0 * chemistry.detonationPressure;
        triplePoint.isWaveFront = true;
        
        structure.triplePoints.push_back(triplePoint);
    }
}

void CellularDetonation2D::initializeEducationalContent() {
    educationalTemplates_["2d_wave_dynamics"] = 
        "2D wave propagation in cylindrical coordinates introduces curvature effects and "
        "complex wave interactions that are not present in 1D analysis.";
        
    educationalTemplates_["cylindrical_effects"] = 
        "Annular geometry creates natural wave front curvature due to the difference in "
        "path lengths between inner and outer radii.";
        
    educationalTemplates_["injection_coupling"] = 
        "Injection-wave interactions are critical for RDE performance, affecting both "
        "wave stability and combustion efficiency through momentum and energy transfer.";
        
    educationalTemplates_["multi_wave_physics"] = 
        "Multiple detonation waves can coexist in RDE systems, leading to complex "
        "collision patterns and energy redistribution mechanisms.";
}

} // namespace MCP
} // namespace Foam