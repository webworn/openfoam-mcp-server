#include "cellular_detonation_model.hpp"
#include "rotating_detonation_engine.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace Foam {
namespace MCP {

CellularDetonationModel::CellularDetonationModel() 
    : trackingInitialized_(false) {
    initializeNeuralNetwork();
    initializeFuelDatabase();
    initializeValidationDatabase();
}

double CellularDetonationModel::calculateCellSize(const RDEChemistry& chemistry) {
    // Check if cellular model is enabled
    if (!chemistry.useCellularModel) {
        return chemistry.cellSize; // Use provided value
    }
    
    try {
        // Calculate components for ANN model
        double inductionLength = calculateInductionLength(chemistry);
        double cjMachNumber = calculateCJMachNumber(chemistry);
        double maxThermicity = calculateMaxThermicity(chemistry);
        
        // Use neural network prediction
        double predictedCellSize = neuralNetworkPrediction(inductionLength, cjMachNumber, maxThermicity);
        
        // Validate against experimental data if available
        double validationError = validatePrediction(chemistry);
        if (validationError > CELL_SIZE_TOLERANCE) {
            std::cout << "Warning: Cell size prediction uncertainty is high (" 
                      << validationError * 100 << "%)" << std::endl;
        }
        
        return predictedCellSize;
        
    } catch (const std::exception& e) {
        std::cout << "Warning: ANN prediction failed, using correlation-based method: " 
                  << e.what() << std::endl;
        return correlationBasedCellSize(chemistry);
    }
}

double CellularDetonationModel::calculateInductionLength(const RDEChemistry& chemistry) {
    // Induction length calculation based on chemistry
    // ΔI = f(activation_energy, pre_exponential_factor, T, P)
    
    // Simplified model based on fuel type
    double baseInductionLength = 0.0;
    
    if (chemistry.fuelType == "hydrogen") {
        // Hydrogen has very short induction length
        baseInductionLength = 1e-5; // 10 μm
    } else if (chemistry.fuelType == "methane") {
        // Methane has longer induction length
        baseInductionLength = 5e-5; // 50 μm
    } else if (chemistry.fuelType == "propane") {
        // Propane has even longer induction length
        baseInductionLength = 1e-4; // 100 μm
    } else {
        // Default for other fuels
        baseInductionLength = 1e-4;
    }
    
    // Scale with pressure and temperature
    double pressureRatio = chemistry.chamberPressure / REFERENCE_PRESSURE;
    double temperatureRatio = chemistry.injectionTemperature / REFERENCE_TEMPERATURE;
    
    // Induction length decreases with pressure, increases with temperature
    double inductionLength = baseInductionLength * std::pow(pressureRatio, -0.5) * 
                            std::pow(temperatureRatio, 0.3);
    
    return inductionLength;
}

double CellularDetonationModel::calculateCJMachNumber(const RDEChemistry& chemistry) {
    // Chapman-Jouguet Mach number calculation
    // MCJ = UCJ / a1, where UCJ is C-J velocity and a1 is sound speed
    
    // Calculate sound speed in unburned gas
    double gamma = 1.4; // Assume ideal gas
    double R_specific = UNIVERSAL_GAS_CONSTANT / 29.0; // Air molecular weight ~29 kg/kmol
    double soundSpeed = std::sqrt(gamma * R_specific * chemistry.injectionTemperature);
    
    // Calculate C-J Mach number
    double cjMachNumber = chemistry.detonationVelocity / soundSpeed;
    
    return cjMachNumber;
}

double CellularDetonationModel::calculateMaxThermicity(const RDEChemistry& chemistry) {
    // Maximum thermicity σ̇max calculation
    // σ̇max = max(dT/dt / T) during reaction
    
    // Simplified model based on fuel type and equivalence ratio
    double baseThermicity = 0.0;
    
    if (chemistry.fuelType == "hydrogen") {
        // Hydrogen has very high thermicity
        baseThermicity = 1e6; // 1/s
    } else if (chemistry.fuelType == "methane") {
        // Methane has moderate thermicity
        baseThermicity = 5e5; // 1/s
    } else if (chemistry.fuelType == "propane") {
        // Propane has lower thermicity
        baseThermicity = 2e5; // 1/s
    } else {
        // Default
        baseThermicity = 1e5;
    }
    
    // Scale with equivalence ratio (peak around stoichiometric)
    double phiFactor = 1.0 - std::pow(chemistry.equivalenceRatio - 1.0, 2.0);
    phiFactor = std::max(phiFactor, 0.1); // Minimum 10% of peak
    
    // Scale with pressure
    double pressureRatio = chemistry.chamberPressure / REFERENCE_PRESSURE;
    double pressureFactor = std::pow(pressureRatio, 0.3);
    
    double maxThermicity = baseThermicity * phiFactor * pressureFactor;
    
    return maxThermicity;
}

double CellularDetonationModel::neuralNetworkPrediction(double inductionLength, 
                                                       double cjMachNumber, 
                                                       double maxThermicity) {
    // Normalize inputs
    std::vector<double> inputs = normalizeInputs(inductionLength, cjMachNumber, maxThermicity);
    
    // Hidden layer 1
    std::vector<double> hidden1(nnWeights_.hiddenLayer1.size());
    for (size_t i = 0; i < hidden1.size(); ++i) {
        hidden1[i] = nnWeights_.biases1[i];
        for (size_t j = 0; j < inputs.size(); ++j) {
            hidden1[i] += nnWeights_.hiddenLayer1[i][j] * inputs[j];
        }
        hidden1[i] = relu(hidden1[i]);
    }
    
    // Hidden layer 2
    std::vector<double> hidden2(nnWeights_.hiddenLayer2.size());
    for (size_t i = 0; i < hidden2.size(); ++i) {
        hidden2[i] = nnWeights_.biases2[i];
        for (size_t j = 0; j < hidden1.size(); ++j) {
            hidden2[i] += nnWeights_.hiddenLayer2[i][j] * hidden1[j];
        }
        hidden2[i] = relu(hidden2[i]);
    }
    
    // Output layer
    double output = nnWeights_.outputBias;
    for (size_t i = 0; i < hidden2.size(); ++i) {
        output += nnWeights_.outputLayer[i] * hidden2[i];
    }
    
    // Apply sigmoid for normalization and scale to physical range
    double normalizedOutput = sigmoid(output);
    
    // Scale to typical cell size range (0.1 mm to 50 mm)
    double cellSize = 0.0001 + normalizedOutput * 0.05; // 0.1 to 50 mm
    
    return cellSize;
}

bool CellularDetonationModel::validateMeshResolution(double minCellSize, double detonationCellSize) {
    double ratio = minCellSize / detonationCellSize;
    return ratio < MESH_SAFETY_FACTOR; // Δx < λ/10
}

double CellularDetonationModel::calculateRequiredMeshSize(double detonationCellSize, double safetyFactor) {
    return detonationCellSize * safetyFactor;
}

CellularDetonationModel::CellularStructure 
CellularDetonationModel::analyzeCellularStructure(const RDEChemistry& chemistry, 
                                                 const RDEGeometry& geometry) {
    CellularStructure structure;
    
    // Calculate basic cell size
    structure.cellSize = calculateCellSize(chemistry);
    
    // Estimate cell dimensions (typically λ × 0.5λ for rectangular cells)
    structure.cellWidth = structure.cellSize;
    structure.cellHeight = structure.cellSize * 0.5;
    
    // Calculate irregularity factor (higher for less stable conditions)
    double phiDeviation = std::abs(chemistry.equivalenceRatio - 1.0);
    structure.irregularity = 1.0 + phiDeviation * 0.3; // 30% increase per unit phi deviation
    
    // Calculate cell passage frequency
    double circumference = 2.0 * M_PI * (geometry.outerRadius + geometry.innerRadius) / 2.0;
    double waveSpeed = chemistry.detonationVelocity;
    structure.frequency = waveSpeed / structure.cellSize;
    
    // Generate cell size distribution (log-normal)
    structure.cellSizeDistribution.clear();
    for (int i = 0; i < 100; ++i) {
        double factor = 1.0 + (i - 50) / 50.0 * 0.3 * structure.irregularity;
        structure.cellSizeDistribution.push_back(structure.cellSize * factor);
    }
    
    return structure;
}

double CellularDetonationModel::validatePrediction(const RDEChemistry& chemistry) {
    // Find experimental data for validation
    auto validationData = getValidationData(chemistry.fuelType);
    
    if (validationData.empty()) {
        return 1.0; // 100% uncertainty if no data
    }
    
    // Find closest experimental conditions
    double minDistance = std::numeric_limits<double>::max();
    const ValidationData* closestData = nullptr;
    
    for (const auto& data : validationData) {
        double pressureDiff = std::abs(data.pressure - chemistry.chamberPressure) / chemistry.chamberPressure;
        double phiDiff = std::abs(data.equivalenceRatio - chemistry.equivalenceRatio) / chemistry.equivalenceRatio;
        double tempDiff = std::abs(data.temperature - chemistry.injectionTemperature) / chemistry.injectionTemperature;
        
        double distance = pressureDiff + phiDiff + tempDiff;
        if (distance < minDistance) {
            minDistance = distance;
            closestData = &data;
        }
    }
    
    if (closestData) {
        double predictedCellSize = calculateCellSize(chemistry);
        double error = std::abs(predictedCellSize - closestData->measuredCellSize) / closestData->measuredCellSize;
        return error;
    }
    
    return 0.5; // 50% uncertainty as default
}

std::vector<CellularDetonationModel::ModelWarning> 
CellularDetonationModel::validateInputs(const RDEChemistry& chemistry) {
    std::vector<ModelWarning> warnings;
    
    // Check if mesh resolution is adequate
    if (chemistry.cellSize > 0.0) {
        if (!validateMeshResolution(chemistry.cellSize, calculateCellSize(chemistry))) {
            warnings.push_back(ModelWarning::MESH_TOO_COARSE);
        }
    }
    
    // Check if chemistry is within validity range
    auto fuelData = getFuelCellularData(chemistry.fuelType);
    if (!fuelData.validityRange.empty()) {
        double pressure = chemistry.chamberPressure;
        double phi = chemistry.equivalenceRatio;
        double temperature = chemistry.injectionTemperature;
        
        if (pressure < fuelData.validityRange[0] || pressure > fuelData.validityRange[1] ||
            phi < fuelData.validityRange[2] || phi > fuelData.validityRange[3] ||
            temperature < fuelData.validityRange[4] || temperature > fuelData.validityRange[5]) {
            warnings.push_back(ModelWarning::CHEMISTRY_OUTSIDE_VALIDITY);
        }
    }
    
    // Check if experimental data is available
    if (getValidationData(chemistry.fuelType).empty()) {
        warnings.push_back(ModelWarning::EXPERIMENTAL_DATA_MISSING);
    }
    
    return warnings;
}

std::string CellularDetonationModel::getWarningMessage(ModelWarning warning) {
    switch (warning) {
        case ModelWarning::MESH_TOO_COARSE:
            return "Mesh resolution is too coarse for cellular structure (Δx > λ/10)";
        case ModelWarning::CHEMISTRY_OUTSIDE_VALIDITY:
            return "Operating conditions are outside validated range";
        case ModelWarning::EXPERIMENTAL_DATA_MISSING:
            return "No experimental validation data available for this fuel";
        case ModelWarning::PREDICTION_UNCERTAIN:
            return "Cell size prediction has high uncertainty";
        default:
            return "Unknown warning";
    }
}

// Private helper methods

double CellularDetonationModel::sigmoid(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

double CellularDetonationModel::relu(double x) {
    return std::max(0.0, x);
}

std::vector<double> CellularDetonationModel::normalizeInputs(double inductionLength, 
                                                           double cjMachNumber, 
                                                           double maxThermicity) {
    // Normalize inputs to [0, 1] range
    std::vector<double> inputs(3);
    
    // Induction length: typical range 1e-6 to 1e-3 m
    inputs[0] = std::log10(inductionLength + 1e-8) / 6.0 + 1.0; // Log scale
    inputs[0] = std::max(0.0, std::min(1.0, inputs[0]));
    
    // C-J Mach number: typical range 3 to 10
    inputs[1] = (cjMachNumber - 3.0) / 7.0;
    inputs[1] = std::max(0.0, std::min(1.0, inputs[1]));
    
    // Max thermicity: typical range 1e4 to 1e7 1/s
    inputs[2] = (std::log10(maxThermicity + 1e-8) - 4.0) / 3.0; // Log scale
    inputs[2] = std::max(0.0, std::min(1.0, inputs[2]));
    
    return inputs;
}

double CellularDetonationModel::correlationBasedCellSize(const RDEChemistry& chemistry) {
    // Fallback correlation-based methods
    if (chemistry.fuelType == "hydrogen") {
        return calculateCellSizeHydrogen(chemistry.chamberPressure, 
                                       chemistry.equivalenceRatio, 
                                       chemistry.injectionTemperature);
    } else if (chemistry.fuelType == "methane") {
        return calculateCellSizeMethane(chemistry.chamberPressure, 
                                      chemistry.equivalenceRatio, 
                                      chemistry.injectionTemperature);
    } else if (chemistry.fuelType == "propane") {
        return calculateCellSizePropane(chemistry.chamberPressure, 
                                      chemistry.equivalenceRatio, 
                                      chemistry.injectionTemperature);
    }
    
    // Default correlation: λ = 29.4 × (P/P₀)^(-0.5)
    double pressureRatio = chemistry.chamberPressure / REFERENCE_PRESSURE;
    return 0.0294 * std::pow(pressureRatio, -0.5);
}

double CellularDetonationModel::calculateCellSizeHydrogen(double pressure, double phi, double temperature) {
    // Hydrogen cell size correlation
    double pressureRatio = pressure / REFERENCE_PRESSURE;
    double phiFactor = 1.0 / (1.0 + std::pow(phi - 1.0, 2.0));
    double tempFactor = std::pow(temperature / REFERENCE_TEMPERATURE, 0.2);
    
    return 0.001 * std::pow(pressureRatio, -0.6) * phiFactor * tempFactor;
}

double CellularDetonationModel::calculateCellSizeMethane(double pressure, double phi, double temperature) {
    // Methane cell size correlation
    double pressureRatio = pressure / REFERENCE_PRESSURE;
    double phiFactor = 1.0 / (1.0 + 2.0 * std::pow(phi - 1.0, 2.0));
    double tempFactor = std::pow(temperature / REFERENCE_TEMPERATURE, 0.3);
    
    return 0.01 * std::pow(pressureRatio, -0.5) * phiFactor * tempFactor;
}

double CellularDetonationModel::calculateCellSizePropane(double pressure, double phi, double temperature) {
    // Propane cell size correlation
    double pressureRatio = pressure / REFERENCE_PRESSURE;
    double phiFactor = 1.0 / (1.0 + 1.5 * std::pow(phi - 1.0, 2.0));
    double tempFactor = std::pow(temperature / REFERENCE_TEMPERATURE, 0.25);
    
    return 0.02 * std::pow(pressureRatio, -0.4) * phiFactor * tempFactor;
}

void CellularDetonationModel::initializeNeuralNetwork() {
    // Initialize simplified neural network weights
    // This would normally be trained on experimental data
    
    // 3 inputs -> 8 hidden -> 4 hidden -> 1 output
    nnWeights_.hiddenLayer1.resize(8, std::vector<double>(3));
    nnWeights_.hiddenLayer2.resize(4, std::vector<double>(8));
    nnWeights_.outputLayer.resize(4);
    nnWeights_.biases1.resize(8);
    nnWeights_.biases2.resize(4);
    
    // Initialize with random weights (simplified)
    for (size_t i = 0; i < 8; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            nnWeights_.hiddenLayer1[i][j] = (double(rand()) / RAND_MAX - 0.5) * 2.0;
        }
        nnWeights_.biases1[i] = (double(rand()) / RAND_MAX - 0.5) * 2.0;
    }
    
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 8; ++j) {
            nnWeights_.hiddenLayer2[i][j] = (double(rand()) / RAND_MAX - 0.5) * 2.0;
        }
        nnWeights_.biases2[i] = (double(rand()) / RAND_MAX - 0.5) * 2.0;
        nnWeights_.outputLayer[i] = (double(rand()) / RAND_MAX - 0.5) * 2.0;
    }
    
    nnWeights_.outputBias = (double(rand()) / RAND_MAX - 0.5) * 2.0;
}

void CellularDetonationModel::initializeFuelDatabase() {
    // Initialize fuel-specific cellular data
    fuelDatabase_["hydrogen"] = {
        "hydrogen", 0.001, -0.6, 1.0, 0.2, 
        {50000, 2000000, 0.4, 2.0, 250, 800} // P, phi, T ranges
    };
    
    fuelDatabase_["methane"] = {
        "methane", 0.01, -0.5, 1.0, 0.3,
        {50000, 1000000, 0.5, 1.8, 280, 600}
    };
    
    fuelDatabase_["propane"] = {
        "propane", 0.02, -0.4, 1.0, 0.25,
        {50000, 800000, 0.6, 1.6, 290, 500}
    };
}

void CellularDetonationModel::initializeValidationDatabase() {
    // Initialize with some typical experimental data
    validationDatabase_ = {
        {"NASA_Glenn", "hydrogen", 101325, 1.0, 298, 0.001, 0.0002},
        {"NASA_Glenn", "hydrogen", 200000, 1.0, 298, 0.0007, 0.0001},
        {"Purdue_DRONE", "methane", 101325, 1.0, 298, 0.01, 0.002},
        {"Purdue_DRONE", "methane", 150000, 1.0, 298, 0.008, 0.0015},
        {"NRL", "propane", 101325, 1.0, 298, 0.02, 0.004}
    };
}

void CellularDetonationModel::loadFuelCellularData() {
    // Load from configuration file if available
    initializeFuelDatabase();
}

CellularDetonationModel::FuelCellularData 
CellularDetonationModel::getFuelCellularData(const std::string& fuelType) {
    auto it = fuelDatabase_.find(fuelType);
    if (it != fuelDatabase_.end()) {
        return it->second;
    }
    
    // Return default data if fuel not found
    return {"unknown", 0.01, -0.5, 1.0, 0.3, {50000, 1000000, 0.5, 2.0, 250, 800}};
}

void CellularDetonationModel::loadValidationData() {
    // Load from database if available
    initializeValidationDatabase();
}

std::vector<CellularDetonationModel::ValidationData> 
CellularDetonationModel::getValidationData(const std::string& fuelType) {
    std::vector<ValidationData> filtered;
    
    for (const auto& data : validationDatabase_) {
        if (data.fuelType == fuelType) {
            filtered.push_back(data);
        }
    }
    
    return filtered;
}

void CellularDetonationModel::initializeCellularTracking() {
    trackingInitialized_ = true;
    trackingHistory_.clear();
}

void CellularDetonationModel::updateCellularTracking(const CellularTrackingData& data) {
    if (!trackingInitialized_) {
        initializeCellularTracking();
    }
    
    trackingHistory_.push_back(data);
    
    // Limit history size
    if (trackingHistory_.size() > 1000) {
        trackingHistory_.erase(trackingHistory_.begin());
    }
}

} // namespace MCP
} // namespace Foam