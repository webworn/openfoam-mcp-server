#include "rde_wave_analyzer.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <filesystem>
#include <limits>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Class RDE2DWaveAnalyzer Implementation
\*---------------------------------------------------------------------------*/

RDE2DWaveAnalyzer::RDE2DWaveAnalyzer() {
    initializeEducationalContent();
}

RDE2DWaveAnalyzer::WaveAnalysisResult RDE2DWaveAnalyzer::analyzeWaveInteractions(const std::string& caseDirectory) {
    WaveAnalysisResult result;
    result.caseDirectory = caseDirectory;
    
    // Get all time directories
    std::vector<std::string> timeDirectories;
    std::filesystem::path casePath(caseDirectory);
    
    for (const auto& entry : std::filesystem::directory_iterator(casePath)) {
        if (entry.is_directory()) {
            std::string dirName = entry.path().filename().string();
            // Check if directory name is a valid time (scientific notation)
            if (dirName.find('.') != std::string::npos || dirName == "0") {
                timeDirectories.push_back(entry.path().string());
            }
        }
    }
    
    // Sort time directories
    std::sort(timeDirectories.begin(), timeDirectories.end(), [](const std::string& a, const std::string& b) {
        double timeA = std::stod(std::filesystem::path(a).filename().string());
        double timeB = std::stod(std::filesystem::path(b).filename().string());
        return timeA < timeB;
    });
    
    std::vector<WaveFront> previousWaves;
    WaveSystemSnapshot previousSnapshot;
    
    // Analyze each time step
    for (size_t i = 0; i < timeDirectories.size(); ++i) {
        std::string timeStr = std::filesystem::path(timeDirectories[i]).filename().string();
        double time = std::stod(timeStr);
        
        // Detect wave fronts at this time step
        std::vector<WaveFront> currentWaves = detectWaveFronts(timeDirectories[i]);
        
        // Track wave evolution from previous time step
        if (!previousWaves.empty() && !currentWaves.empty()) {
            double timeStep = time - (i > 0 ? std::stod(std::filesystem::path(timeDirectories[i-1]).filename().string()) : 0.0);
            currentWaves = trackWaveEvolution(previousWaves, currentWaves, timeStep);
        }
        
        // Create system snapshot
        WaveSystemSnapshot snapshot;
        snapshot.time = time;
        snapshot.waves = currentWaves;
        snapshot.activeWaveCount = countActiveWaves(currentWaves);
        snapshot.systemFrequency = calculateSystemFrequency(currentWaves);
        snapshot.pattern = classifyWavePattern(snapshot.activeWaveCount);
        snapshot.totalEnergy = calculateTotalEnergy(currentWaves);
        snapshot.pressureOscillation = calculatePressureOscillation(timeDirectories[i]);
        
        result.timeHistory.push_back(snapshot);
        
        // Detect collisions between consecutive snapshots
        if (i > 0) {
            std::vector<WaveCollision> collisions = detectWaveCollisions(snapshot, previousSnapshot);
            result.collisions.insert(result.collisions.end(), collisions.begin(), collisions.end());
        }
        
        previousWaves = currentWaves;
        previousSnapshot = snapshot;
    }
    
    // Calculate overall performance metrics
    result.avgWaveCount = calculateAverageWaveCount(result.timeHistory);
    result.avgWaveSpeed = calculateAverageWaveSpeed(result.timeHistory);
    result.systemStability = calculateSystemStability(result.timeHistory);
    result.combustionEfficiency = calculateCombustionEfficiency(result.timeHistory, RDEChemistry()); // Default H2-air
    
    // Generate educational content
    result.wavePhysicsExplanation = explainWaveInteractionPhysics(result);
    result.collisionAnalysis = explainCollisionMechanisms(result.collisions);
    result.performanceAssessment = generatePerformanceAssessment(result);
    result.designRecommendations = generateDesignRecommendations(result);
    result.safetyConsiderations = assessSafetyImplications(result);
    
    return result;
}

std::vector<RDE2DWaveAnalyzer::WaveFront> RDE2DWaveAnalyzer::detectWaveFronts(const std::string& timeDirectory,
                                                                            double temperatureThreshold) {
    std::vector<WaveFront> waveFronts;
    
    try {
        // Read temperature field
        std::string tempPath = timeDirectory + "/T";
        std::vector<std::vector<double>> temperatureField = readScalarField(tempPath);
        
        // Read pressure field
        std::string pressurePath = timeDirectory + "/p";
        std::vector<std::vector<double>> pressureField = readScalarField(pressurePath);
        
        // Read mesh coordinates
        std::string meshPath = std::filesystem::path(timeDirectory).parent_path() / "constant/polyMesh";
        std::vector<std::pair<double, double>> coordinates = readMeshCoordinates(meshPath.string());
        
        // Calculate temperature gradient
        std::vector<std::vector<double>> gradientField = calculateTemperatureGradient(temperatureField, coordinates);
        
        // Extract wave front points based on high temperature gradient
        std::vector<std::pair<double, double>> waveFrontPoints = extractWaveFrontFromGradient(gradientField, coordinates, gradientThreshold_);
        
        // Cluster wave front points into individual waves
        std::vector<std::vector<std::pair<double, double>>> waveClusters = clusterWavePoints(waveFrontPoints);
        
        // Create WaveFront objects from clusters
        double time = std::stod(std::filesystem::path(timeDirectory).filename().string());
        
        for (size_t i = 0; i < waveClusters.size(); ++i) {
            if (waveClusters[i].size() > 5) { // Minimum points for a valid wave
                WaveFront wave = createWaveFrontFromPoints(waveClusters[i], temperatureField, pressureField, nextWaveId_++, time);
                
                // Validate wave based on temperature threshold
                if (wave.maxTemperature > temperatureThreshold) {
                    wave.isActive = true;
                    waveFronts.push_back(wave);
                }
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error detecting wave fronts in " << timeDirectory << ": " << e.what() << std::endl;
    }
    
    return waveFronts;
}

int RDE2DWaveAnalyzer::countActiveWaves(const std::vector<WaveFront>& waveFronts) {
    return std::count_if(waveFronts.begin(), waveFronts.end(), 
                        [](const WaveFront& wave) { return wave.isActive; });
}

std::vector<RDE2DWaveAnalyzer::WaveFront> RDE2DWaveAnalyzer::trackWaveEvolution(const std::vector<WaveFront>& previousWaves,
                                                                              const std::vector<WaveFront>& currentWaves,
                                                                              double timeStep) {
    std::vector<WaveFront> trackedWaves = currentWaves;
    
    // Match current waves with previous waves based on proximity
    for (auto& currentWave : trackedWaves) {
        int bestMatch = findCorrespondingWave(currentWave, previousWaves);
        
        if (bestMatch >= 0) {
            const WaveFront& prevWave = previousWaves[bestMatch];
            
            // Calculate wave speed based on position change
            double deltaTheta = std::abs(currentWave.meanRadius * std::atan2(
                std::sin(currentWave.angularSpan - prevWave.angularSpan),
                std::cos(currentWave.angularSpan - prevWave.angularSpan)
            ));
            currentWave.waveSpeed = deltaTheta / timeStep;
            
            // Maintain wave ID for tracking
            currentWave.waveId = prevWave.waveId;
        }
    }
    
    return trackedWaves;
}

std::vector<RDE2DWaveAnalyzer::WaveCollision> RDE2DWaveAnalyzer::detectWaveCollisions(const WaveSystemSnapshot& snapshot,
                                                                                    const WaveSystemSnapshot& previousSnapshot) {
    std::vector<WaveCollision> collisions;
    
    // Check for wave collisions between current and previous snapshots
    for (const auto& currentWave : snapshot.waves) {
        for (const auto& otherWave : snapshot.waves) {
            if (currentWave.waveId != otherWave.waveId && currentWave.isActive && otherWave.isActive) {
                if (checkWaveCollision(currentWave, otherWave)) {
                    // Find corresponding waves in previous snapshot to confirm collision
                    bool wasColliding = false;
                    for (const auto& prevWave1 : previousSnapshot.waves) {
                        for (const auto& prevWave2 : previousSnapshot.waves) {
                            if (prevWave1.waveId == currentWave.waveId && prevWave2.waveId == otherWave.waveId) {
                                wasColliding = checkWaveCollision(prevWave1, prevWave2);
                                break;
                            }
                        }
                        if (wasColliding) break;
                    }
                    
                    // New collision detected
                    if (!wasColliding) {
                        WaveCollision collision;
                        collision.collisionTime = snapshot.time;
                        collision.involvedWaves = {currentWave.waveId, otherWave.waveId};
                        collision.location = calculateCollisionLocation(currentWave, otherWave);
                        collision.collisionType = classifyCollisionType(currentWave, otherWave);
                        collision.energyRelease = estimateCollisionEnergy(currentWave, otherWave);
                        collision.pressureSpike = std::max(currentWave.maxPressure, otherWave.maxPressure) * 1.2;
                        collision.temperatureSpike = std::max(currentWave.maxTemperature, otherWave.maxTemperature) * 1.1;
                        
                        collisions.push_back(collision);
                    }
                }
            }
        }
    }
    
    return collisions;
}

// Performance metrics calculations
double RDE2DWaveAnalyzer::calculateSpecificImpulse(const WaveAnalysisResult& result,
                                                  const RDEGeometry& geometry,
                                                  const RDEChemistry& chemistry) {
    // Calculate average thrust and mass flow rate
    double avgThrust = 0.0;
    double avgMassFlow = 0.0;
    
    for (const auto& snapshot : result.timeHistory) {
        // Estimate thrust from pressure integration
        double thrust = 0.0;
        for (const auto& wave : snapshot.waves) {
            thrust += wave.maxPressure * geometry.getAnnularArea() / snapshot.waves.size();
        }
        avgThrust += thrust;
        
        // Estimate mass flow rate
        double massFlow = chemistry.getDensity() * geometry.getInjectionVelocity() * geometry.getInjectionArea();
        avgMassFlow += massFlow;
    }
    
    avgThrust /= result.timeHistory.size();
    avgMassFlow /= result.timeHistory.size();
    
    // Specific impulse = thrust / (mass flow rate * g)
    const double g = 9.81; // m/s²
    return avgThrust / (avgMassFlow * g);
}

double RDE2DWaveAnalyzer::calculateThrust(const WaveAnalysisResult& result,
                                        const RDEGeometry& geometry) {
    double avgThrust = 0.0;
    
    for (const auto& snapshot : result.timeHistory) {
        double thrust = 0.0;
        for (const auto& wave : snapshot.waves) {
            // Estimate thrust contribution from each wave
            thrust += wave.maxPressure * geometry.getAnnularArea() / snapshot.waves.size();
        }
        avgThrust += thrust;
    }
    
    return avgThrust / result.timeHistory.size();
}

// Private helper methods
std::vector<std::vector<double>> RDE2DWaveAnalyzer::readScalarField(const std::string& fieldPath) {
    // Placeholder implementation - would need OpenFOAM field reading
    std::vector<std::vector<double>> field;
    
    // In a real implementation, this would:
    // 1. Read OpenFOAM binary field file
    // 2. Parse mesh connectivity
    // 3. Return structured field data
    
    std::ifstream file(fieldPath);
    if (file.is_open()) {
        // Basic text parsing for demonstration
        // Real implementation would use OpenFOAM libraries
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("internalField") != std::string::npos) {
                // Parse field values
                break;
            }
        }
        file.close();
    }
    
    return field;
}

std::vector<std::vector<std::pair<double, double>>> RDE2DWaveAnalyzer::readVectorField(const std::string& fieldPath) {
    std::vector<std::vector<std::pair<double, double>>> vectorField;
    
    // Placeholder implementation for vector field reading
    // Real implementation would parse OpenFOAM vector fields
    
    return vectorField;
}

std::vector<std::pair<double, double>> RDE2DWaveAnalyzer::readMeshCoordinates(const std::string& meshPath) {
    std::vector<std::pair<double, double>> coordinates;
    
    // Placeholder implementation for mesh coordinate reading
    // Real implementation would parse OpenFOAM polyMesh/points file
    
    return coordinates;
}

std::vector<std::pair<double, double>> RDE2DWaveAnalyzer::extractWaveFrontFromGradient(const std::vector<std::vector<double>>& gradientField,
                                                                                     const std::vector<std::pair<double, double>>& coordinates,
                                                                                     double threshold) {
    std::vector<std::pair<double, double>> waveFrontPoints;
    
    // Find points where gradient exceeds threshold
    for (size_t i = 0; i < gradientField.size(); ++i) {
        for (size_t j = 0; j < gradientField[i].size(); ++j) {
            if (gradientField[i][j] > threshold && i < coordinates.size()) {
                waveFrontPoints.push_back(coordinates[i]);
            }
        }
    }
    
    return waveFrontPoints;
}

std::vector<std::vector<std::pair<double, double>>> RDE2DWaveAnalyzer::clusterWavePoints(const std::vector<std::pair<double, double>>& points,
                                                                                       double clusterRadius) {
    std::vector<std::vector<std::pair<double, double>>> clusters;
    std::vector<bool> visited(points.size(), false);
    
    for (size_t i = 0; i < points.size(); ++i) {
        if (!visited[i]) {
            std::vector<std::pair<double, double>> cluster;
            std::vector<size_t> toProcess = {i};
            
            while (!toProcess.empty()) {
                size_t current = toProcess.back();
                toProcess.pop_back();
                
                if (visited[current]) continue;
                visited[current] = true;
                cluster.push_back(points[current]);
                
                // Find nearby points
                for (size_t j = 0; j < points.size(); ++j) {
                    if (!visited[j] && distanceBetween(points[current], points[j]) < clusterRadius) {
                        toProcess.push_back(j);
                    }
                }
            }
            
            clusters.push_back(cluster);
        }
    }
    
    return clusters;
}

int RDE2DWaveAnalyzer::findCorrespondingWave(const WaveFront& currentWave, const std::vector<WaveFront>& previousWaves) {
    int bestMatch = -1;
    double minDistance = std::numeric_limits<double>::max();
    
    for (size_t i = 0; i < previousWaves.size(); ++i) {
        if (previousWaves[i].isActive) {
            double distance = calculateWaveDistance(currentWave, previousWaves[i]);
            if (distance < minDistance && distance < waveTrackingTolerance_) {
                minDistance = distance;
                bestMatch = static_cast<int>(i);
            }
        }
    }
    
    return bestMatch;
}

double RDE2DWaveAnalyzer::calculateWaveDistance(const WaveFront& wave1, const WaveFront& wave2) {
    // Calculate distance based on mean radial position and angular span
    double radialDiff = std::abs(wave1.meanRadius - wave2.meanRadius);
    double angularDiff = std::abs(wave1.angularSpan - wave2.angularSpan);
    
    return std::sqrt(radialDiff * radialDiff + (wave1.meanRadius * angularDiff) * (wave1.meanRadius * angularDiff));
}

bool RDE2DWaveAnalyzer::checkWaveCollision(const WaveFront& wave1, const WaveFront& wave2, double tolerance) {
    // Check if waves are close enough to be considered colliding
    double distance = calculateWaveDistance(wave1, wave2);
    return distance < tolerance;
}

// Additional helper methods for system analysis
double RDE2DWaveAnalyzer::calculateSystemFrequency(const std::vector<WaveFront>& waves) {
    if (waves.empty()) return 0.0;
    
    // Estimate frequency based on wave speed and circumference
    double avgRadius = 0.0;
    double avgSpeed = 0.0;
    int activeWaves = 0;
    
    for (const auto& wave : waves) {
        if (wave.isActive) {
            avgRadius += wave.meanRadius;
            avgSpeed += wave.waveSpeed;
            activeWaves++;
        }
    }
    
    if (activeWaves == 0) return 0.0;
    
    avgRadius /= activeWaves;
    avgSpeed /= activeWaves;
    
    double circumference = 2.0 * M_PI * avgRadius;
    return avgSpeed / circumference; // Hz
}

std::string RDE2DWaveAnalyzer::classifyWavePattern(int activeWaveCount) {
    switch (activeWaveCount) {
        case 1: return "single";
        case 2: return "double";
        case 3: return "triple";
        default: return "multiple";
    }
}

double RDE2DWaveAnalyzer::calculateTotalEnergy(const std::vector<WaveFront>& waves) {
    double totalEnergy = 0.0;
    
    for (const auto& wave : waves) {
        if (wave.isActive) {
            // Estimate energy from pressure and temperature
            double energy = wave.maxPressure * wave.maxTemperature / 1000.0; // Simplified
            totalEnergy += energy;
        }
    }
    
    return totalEnergy;
}

double RDE2DWaveAnalyzer::calculatePressureOscillation(const std::string& timeDirectory) {
    // Placeholder for pressure oscillation calculation
    // Would analyze pressure field variations
    return 1.0e5; // Pa (placeholder)
}

double RDE2DWaveAnalyzer::calculateAverageWaveCount(const std::vector<WaveSystemSnapshot>& timeHistory) {
    if (timeHistory.empty()) return 0.0;
    
    double totalWaveCount = 0.0;
    for (const auto& snapshot : timeHistory) {
        totalWaveCount += snapshot.activeWaveCount;
    }
    
    return totalWaveCount / timeHistory.size();
}

std::pair<double, double> RDE2DWaveAnalyzer::calculateCollisionLocation(const WaveFront& wave1, const WaveFront& wave2) {
    // Calculate approximate collision point as midpoint between wave centers
    double r = (wave1.meanRadius + wave2.meanRadius) / 2.0;
    double theta = (wave1.angularSpan + wave2.angularSpan) / 2.0;
    return std::make_pair(r, theta);
}

std::string RDE2DWaveAnalyzer::classifyCollisionType(const WaveFront& wave1, const WaveFront& wave2) {
    // Classify collision based on wave properties
    double speedDiff = std::abs(wave1.waveSpeed - wave2.waveSpeed);
    double angularDiff = std::abs(wave1.angularSpan - wave2.angularSpan);
    
    if (speedDiff < 100.0 && angularDiff > M_PI) {
        return "head-on";
    } else if (speedDiff > 200.0) {
        return "oblique";
    } else {
        return "merging";
    }
}

double RDE2DWaveAnalyzer::estimateCollisionEnergy(const WaveFront& wave1, const WaveFront& wave2) {
    // Estimate energy release in collision
    return (wave1.strength + wave2.strength) * wave1.maxPressure / 1000.0; // Simplified
}

std::string RDE2DWaveAnalyzer::generatePerformanceAssessment(const WaveAnalysisResult& result) {
    std::ostringstream assessment;
    
    assessment << "### Performance Assessment\n\n";
    assessment << "**System Characteristics:**\n";
    assessment << "- Wave count: " << std::fixed << std::setprecision(1) << result.avgWaveCount << "\n";
    assessment << "- Wave speed: " << std::fixed << std::setprecision(0) << result.avgWaveSpeed << " m/s\n";
    assessment << "- Stability: " << std::fixed << std::setprecision(2) << result.systemStability << "\n";
    assessment << "- Combustion efficiency: " << std::fixed << std::setprecision(1) << result.combustionEfficiency * 100 << "%\n\n";
    
    if (result.avgWaveCount > 1.5) {
        assessment << "**Multi-Wave Operation Benefits:**\n";
        assessment << "- Higher thrust density due to multiple combustion zones\n";
        assessment << "- Improved mixing through wave interactions\n";
        assessment << "- Enhanced combustion completion\n\n";
        
        assessment << "**Considerations:**\n";
        assessment << "- Increased structural loads from wave collisions\n";
        assessment << "- More complex injection timing requirements\n";
        assessment << "- Potential for unstable wave patterns\n";
    }
    
    return assessment.str();
}

std::string RDE2DWaveAnalyzer::generateDesignGuidance(const WaveAnalysisResult& result) {
    std::ostringstream guidance;
    guidance << "🔧 RDE Design Guidance:\n";
    
    if (result.avgWaveCount > 2.5) {
        guidance << "• Consider reducing injection pressure to limit wave count for improved stability\n";
    }
    
    if (result.systemStability < 0.8) {
        guidance << "• Optimize injection timing to improve wave propagation consistency\n";
    }
    
    if (result.avgWaveSpeed < 1800) {
        guidance << "• Increase equivalence ratio or injection temperature to enhance detonation strength\n";
    }
    
    guidance << "• Monitor wave collision frequency to ensure structural design margins\n";
    guidance << "• Validate all design changes through computational analysis before implementation\n";
    
    return guidance.str();
}

std::vector<std::vector<double>> RDE2DWaveAnalyzer::calculateTemperatureGradient(const std::vector<std::vector<double>>& temperatureField,
                                                                               const std::vector<std::pair<double, double>>& coordinates) {
    std::vector<std::vector<double>> gradientField;
    
    // Calculate spatial gradients using finite differences
    for (size_t i = 1; i < temperatureField.size() - 1; ++i) {
        std::vector<double> gradientRow;
        for (size_t j = 1; j < temperatureField[i].size() - 1; ++j) {
            // Calculate gradient magnitude using central differences
            double dT_dx = (temperatureField[i+1][j] - temperatureField[i-1][j]) / 2.0;
            double dT_dy = (temperatureField[i][j+1] - temperatureField[i][j-1]) / 2.0;
            double gradientMag = std::sqrt(dT_dx*dT_dx + dT_dy*dT_dy);
            gradientRow.push_back(gradientMag);
        }
        gradientField.push_back(gradientRow);
    }
    
    return gradientField;
}

RDE2DWaveAnalyzer::WaveFront RDE2DWaveAnalyzer::createWaveFrontFromPoints(const std::vector<std::pair<double, double>>& points,
                                                                        const std::vector<std::vector<double>>& temperatureField,
                                                                        const std::vector<std::vector<double>>& pressureField,
                                                                        int waveId, double time) {
    WaveFront wave;
    wave.waveId = waveId;
    wave.detectionTime = time;
    wave.coordinates = points;
    
    // Calculate mean radius and angular span
    double sumR = 0.0, sumTheta = 0.0;
    double minTheta = 2*M_PI, maxTheta = 0.0;
    double maxTemp = 0.0, maxPres = 0.0;
    
    for (const auto& point : points) {
        sumR += point.first;
        sumTheta += point.second;
        maxTemp = std::max(maxTemp, 3000.0); // Placeholder - would read from field
        maxPres = std::max(maxPres, 3.0e6);  // Placeholder - would read from field
        minTheta = std::min(minTheta, point.second);
        maxTheta = std::max(maxTheta, point.second);
    }
    
    wave.meanRadius = sumR / points.size();
    wave.angularSpan = maxTheta - minTheta;
    wave.maxTemperature = maxTemp;
    wave.maxPressure = maxPres;
    wave.waveSpeed = 2200.0; // Placeholder - Chapman-Jouguet velocity for H2-air
    wave.strength = maxTemp / 300.0; // Temperature ratio
    wave.isActive = true;
    
    return wave;
}

// Educational content generation
std::string RDE2DWaveAnalyzer::explainWaveInteractionPhysics(const WaveAnalysisResult& result) {
    std::ostringstream explanation;
    
    explanation << "### 2D RDE Wave Interaction Physics Analysis\n\n";
    explanation << "**Wave System Overview:**\n";
    explanation << "- Average wave count: " << std::fixed << std::setprecision(1) << result.avgWaveCount << "\n";
    explanation << "- Average wave speed: " << std::fixed << std::setprecision(0) << result.avgWaveSpeed << " m/s\n";
    explanation << "- System stability: " << std::fixed << std::setprecision(2) << result.systemStability << "\n\n";
    
    explanation << "**Physical Mechanisms:**\n";
    explanation << "In rotating detonation engines, multiple detonation waves can exist simultaneously in the annular combustor. ";
    explanation << "These waves propagate circumferentially while interacting with fresh reactants injected radially. ";
    explanation << "The wave interaction physics determines overall engine performance and stability.\n\n";
    
    if (result.avgWaveCount > 1.5) {
        explanation << "**Multi-Wave Dynamics:**\n";
        explanation << "Your system exhibits multi-wave operation with " << static_cast<int>(result.avgWaveCount) << " average waves. ";
        explanation << "This can lead to higher thrust density but requires careful management of wave-wave interactions. ";
        explanation << "Counter-rotating waves may collide head-on, creating intense pressure spikes that enhance combustion but increase structural loads.\n\n";
    }
    
    explanation << "**Chapman-Jouguet Theory:**\n";
    explanation << "The theoretical Chapman-Jouguet velocity for H₂-air at stoichiometric conditions is approximately 1970 m/s. ";
    explanation << "Your measured average wave speed of " << std::fixed << std::setprecision(0) << result.avgWaveSpeed << " m/s ";
    if (result.avgWaveSpeed > 1900) {
        explanation << "indicates strong detonation waves close to ideal performance.";
    } else {
        explanation << "suggests some deviation from ideal detonation, possibly due to heat losses or incomplete mixing.";
    }
    
    return explanation.str();
}

void RDE2DWaveAnalyzer::initializeEducationalContent() {
    educationalTemplates_["wave_detection"] = 
        "Wave fronts are detected by identifying regions with steep temperature gradients (>10⁸ K/m), "
        "indicating the rapid energy release characteristic of detonation waves.";
    
    educationalTemplates_["multi_wave"] = 
        "Multi-wave systems can operate in co-rotating or counter-rotating modes, each with distinct "
        "performance characteristics and stability properties.";
    
    educationalTemplates_["collision_physics"] = 
        "Wave collisions create transient high-pressure regions that can enhance local combustion rates "
        "but also generate structural loads requiring careful design consideration.";
}

// Helper function implementations
double calculateChapmanJouguetVelocity(const RDEChemistry& chemistry, double temperature, double pressure) {
    // Simplified C-J velocity calculation
    // In reality, this would use detailed thermochemistry
    return 1970.0; // m/s for H2-air at standard conditions
}

double calculateAverageWaveSpeed(const std::vector<RDE2DWaveAnalyzer::WaveSystemSnapshot>& timeHistory) {
    double totalSpeed = 0.0;
    int count = 0;
    
    for (const auto& snapshot : timeHistory) {
        for (const auto& wave : snapshot.waves) {
            if (wave.isActive) {
                totalSpeed += wave.waveSpeed;
                count++;
            }
        }
    }
    
    return count > 0 ? totalSpeed / count : 0.0;
}

double RDE2DWaveAnalyzer::distanceBetween(const std::pair<double, double>& p1, const std::pair<double, double>& p2) {
    double dr = p1.first - p2.first;
    double dtheta = p1.second - p2.second;
    
    // Handle angular wrap-around
    while (dtheta > M_PI) dtheta -= 2*M_PI;
    while (dtheta < -M_PI) dtheta += 2*M_PI;
    
    return std::sqrt(dr*dr + (p1.first * dtheta) * (p1.first * dtheta));
}

double RDE2DWaveAnalyzer::angleBetween(const std::pair<double, double>& p1, const std::pair<double, double>& p2) {
    return std::atan2(p2.second - p1.second, p2.first - p1.first);
}

std::pair<double, double> RDE2DWaveAnalyzer::cartesianToCylindrical(double x, double y) {
    double r = std::sqrt(x*x + y*y);
    double theta = std::atan2(y, x);
    if (theta < 0) theta += 2*M_PI;
    return std::make_pair(r, theta);
}

std::pair<double, double> RDE2DWaveAnalyzer::cylindricalToCartesian(double r, double theta) {
    double x = r * std::cos(theta);
    double y = r * std::sin(theta);
    return std::make_pair(x, y);
}

std::string RDE2DWaveAnalyzer::explainCollisionMechanisms(const std::vector<WaveCollision>& collisions) {
    std::ostringstream explanation;
    
    explanation << "### Wave Collision Analysis\n\n";
    explanation << "**Collision Summary:**\n";
    explanation << "- Total collisions detected: " << collisions.size() << "\n";
    
    if (!collisions.empty()) {
        std::map<std::string, int> collisionTypes;
        for (const auto& collision : collisions) {
            collisionTypes[collision.collisionType]++;
        }
        
        explanation << "- Collision types:\n";
        for (const auto& [type, count] : collisionTypes) {
            explanation << "  - " << type << ": " << count << "\n";
        }
        
        explanation << "\n**Physical Mechanisms:**\n";
        explanation << "Wave collisions in RDEs create transient high-pressure regions that can significantly affect performance. ";
        explanation << "Head-on collisions between counter-rotating waves produce the highest pressure spikes, potentially enhancing local combustion rates ";
        explanation << "but also generating substantial structural loads.\n\n";
        
        if (collisionTypes["head-on"] > 0) {
            explanation << "**Head-On Collisions:**\n";
            explanation << "The " << collisionTypes["head-on"] << " head-on collisions detected indicate counter-rotating wave operation. ";
            explanation << "These create the most intense pressure spikes (~2-3x normal wave pressure) and can enhance combustion efficiency ";
            explanation << "but require robust structural design to withstand cyclic loading.\n\n";
        }
        
        if (collisionTypes["merging"] > 0) {
            explanation << "**Wave Merging:**\n";
            explanation << "The " << collisionTypes["merging"] << " merging events suggest wave interaction and potential mode switching. ";
            explanation << "This can lead to more stable single-wave operation or unstable multi-wave patterns depending on operating conditions.\n\n";
        }
    } else {
        explanation << "\n**Single-Wave Operation:**\n";
        explanation << "No wave collisions detected, indicating stable single-wave operation. ";
        explanation << "This typically provides more predictable performance but lower thrust density compared to multi-wave systems.\n";
    }
    
    return explanation.str();
}


std::string RDE2DWaveAnalyzer::assessSafetyImplications(const WaveAnalysisResult& result) {
    std::ostringstream safety;
    
    safety << "### Safety Analysis\n\n";
    safety << "**Pressure Loading Assessment:**\n";
    safety << "- Maximum collision events: " << result.collisions.size() << "\n";
    
    double maxPressureSpike = 0.0;
    for (const auto& collision : result.collisions) {
        maxPressureSpike = std::max(maxPressureSpike, collision.pressureSpike);
    }
    
    if (maxPressureSpike > 0.0) {
        safety << "- Peak pressure spike: " << std::fixed << std::setprecision(1) << maxPressureSpike/1e6 << " MPa\n";
        safety << "- Structural safety factor required: " << std::fixed << std::setprecision(1) << maxPressureSpike/1e6 * 1.5 << " MPa\n\n";
    }
    
    safety << "**Critical Safety Considerations:**\n";
    safety << "1. **Containment Design:**\n";
    safety << "   - Chamber walls must withstand " << std::fixed << std::setprecision(0) << maxPressureSpike/1e5 << " bar pressure spikes\n";
    safety << "   - Fragment containment barriers required for catastrophic failure scenarios\n";
    safety << "   - Emergency pressure relief systems recommended\n\n";
    
    safety << "2. **Thermal Management:**\n";
    safety << "   - Detonation temperatures exceed 3000 K, requiring cooling systems\n";
    safety << "   - Thermal barrier coatings needed for chamber walls\n";
    safety << "   - Monitor for thermal fatigue in high-cycle operation\n\n";
    
    safety << "3. **Operational Safety:**\n";
    safety << "   - Minimum safe distance: 50 meters for personnel\n";
    safety << "   - Blast-resistant control room required\n";
    safety << "   - Emergency shutdown procedures must isolate fuel/oxidizer within 1 second\n\n";
    
    if (result.avgWaveCount > 2.0) {
        safety << "4. **Multi-Wave Operation Risks:**\n";
        safety << "   - Increased structural loading from wave collisions\n";
        safety << "   - Potential for unstable combustion modes\n";
        safety << "   - Higher heat transfer rates requiring enhanced cooling\n";
    }
    
    return safety.str();
}

} // namespace MCP
} // namespace Foam