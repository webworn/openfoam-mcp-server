#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cmath>
#include <nlohmann/json.hpp>

namespace Foam {
namespace MCP {

using json = nlohmann::json;

// Forward declarations
struct RDEChemistry;
struct RDEGeometry;

/*---------------------------------------------------------------------------*\
                        Class CellularDetonationModel
\*---------------------------------------------------------------------------*/

class CellularDetonationModel {
public:
    
    // Constructor
    CellularDetonationModel();
    
    // Destructor
    ~CellularDetonationModel() = default;
    
    // Main cellular structure calculation
    double calculateCellSize(const RDEChemistry& chemistry);
    
    // Component calculations for ANN model
    double calculateInductionLength(const RDEChemistry& chemistry);
    double calculateCJMachNumber(const RDEChemistry& chemistry);
    double calculateMaxThermicity(const RDEChemistry& chemistry);
    
    // Neural network prediction λ = f(ΔI, MCJ, σ̇max)
    double neuralNetworkPrediction(double inductionLength, double cjMachNumber, double maxThermicity);
    
    // Mesh validation and constraints
    bool validateMeshResolution(double minCellSize, double detonationCellSize);
    double calculateRequiredMeshSize(double detonationCellSize, double safetyFactor = 0.1);
    
    // Cellular structure analysis
    struct CellularStructure {
        double cellSize;        // λ [m]
        double cellWidth;       // Transverse cell dimension [m]
        double cellHeight;      // Longitudinal cell dimension [m]
        double irregularity;    // Cell size irregularity factor
        double frequency;       // Cell passage frequency [Hz]
        std::vector<double> cellSizeDistribution; // Statistical distribution
    };
    
    CellularStructure analyzeCellularStructure(const RDEChemistry& chemistry, 
                                             const RDEGeometry& geometry);
    
    // Fuel-specific cellular correlations
    struct FuelCellularData {
        std::string fuelType;
        double cellSizeCorrelation;  // Correlation coefficient
        double pressureExponent;     // Pressure dependence
        double phiExponent;          // Equivalence ratio dependence
        double temperatureExponent;  // Temperature dependence
        std::vector<double> validityRange; // [P_min, P_max, phi_min, phi_max, T_min, T_max]
    };
    
    void loadFuelCellularData();
    FuelCellularData getFuelCellularData(const std::string& fuelType);
    
    // Experimental validation
    struct ValidationData {
        std::string source;      // "NASA_Glenn", "Purdue_DRONE", "NRL", etc.
        std::string fuelType;
        double pressure;         // [Pa]
        double equivalenceRatio; // [-]
        double temperature;      // [K]
        double measuredCellSize; // [m]
        double uncertainty;      // [m]
    };
    
    void loadValidationData();
    std::vector<ValidationData> getValidationData(const std::string& fuelType);
    double validatePrediction(const RDEChemistry& chemistry);
    
    // Cellular tracking support
    struct CellularTrackingData {
        double maxPressure;      // Maximum pressure in cell [Pa]
        double pressureGradient; // Pressure gradient magnitude [Pa/m]
        double triplePointVelocity; // Triple point velocity [m/s]
        std::vector<double> cellHistory; // Cell size evolution
    };
    
    void initializeCellularTracking();
    void updateCellularTracking(const CellularTrackingData& data);
    
    // Error handling and warnings
    enum class ModelWarning {
        MESH_TOO_COARSE,
        CHEMISTRY_OUTSIDE_VALIDITY,
        EXPERIMENTAL_DATA_MISSING,
        PREDICTION_UNCERTAIN
    };
    
    std::vector<ModelWarning> validateInputs(const RDEChemistry& chemistry);
    std::string getWarningMessage(ModelWarning warning);
    
private:
    
    // Neural network parameters
    struct NeuralNetworkWeights {
        std::vector<std::vector<double>> hiddenLayer1;
        std::vector<std::vector<double>> hiddenLayer2;
        std::vector<double> outputLayer;
        std::vector<double> biases1;
        std::vector<double> biases2;
        double outputBias;
    };
    
    NeuralNetworkWeights nnWeights_;
    
    // Fuel database
    std::map<std::string, FuelCellularData> fuelDatabase_;
    
    // Validation database
    std::vector<ValidationData> validationDatabase_;
    
    // Cellular tracking state
    bool trackingInitialized_;
    std::vector<CellularTrackingData> trackingHistory_;
    
    // Private helper methods
    double sigmoid(double x);
    double relu(double x);
    std::vector<double> normalizeInputs(double inductionLength, double cjMachNumber, double maxThermicity);
    
    // Correlation-based fallback methods
    double correlationBasedCellSize(const RDEChemistry& chemistry);
    double calculateCellSizeHydrogen(double pressure, double equivalenceRatio, double temperature);
    double calculateCellSizeMethane(double pressure, double equivalenceRatio, double temperature);
    double calculateCellSizePropane(double pressure, double equivalenceRatio, double temperature);
    
    // Initialization methods
    void initializeNeuralNetwork();
    void initializeFuelDatabase();
    void initializeValidationDatabase();
    
    // Constants
    static constexpr double UNIVERSAL_GAS_CONSTANT = 8314.0; // J/(kmol·K)
    static constexpr double REFERENCE_PRESSURE = 101325.0;   // Pa
    static constexpr double REFERENCE_TEMPERATURE = 298.15;  // K
    static constexpr double MESH_SAFETY_FACTOR = 0.1;       // Δx/λ ratio
    static constexpr double CELL_SIZE_TOLERANCE = 0.2;      // ±20% validation tolerance
};

} // namespace MCP
} // namespace Foam