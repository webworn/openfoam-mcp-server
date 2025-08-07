#pragma once

#include <cmath>
#include <vector>
#include <map>
#include <string>
#include <functional>

namespace Foam {
namespace MCP {
namespace Physics {

/*---------------------------------------------------------------------------*\
                    Class ChapmanJouguetSolver
\*---------------------------------------------------------------------------*/

/**
 * @brief Chapman-Jouguet detonation solver using proven SDToolbox algorithm
 * 
 * This class implements the Chapman-Jouguet solver based on the validated
 * Shock and Detonation Toolbox (SDT) from Caltech. The implementation uses
 * the minimum wave speed method with parabolic fitting, following Reynolds'
 * iterative approach for solving the Rankine-Hugoniot relations coupled
 * with the Chapman-Jouguet condition.
 * 
 * Mathematical Foundation:
 * The solver finds the Chapman-Jouguet state by solving:
 * 1. Mass Conservation:     ρ₁u₁ = ρ₂u₂  
 * 2. Momentum Conservation: P₁ + ρ₁u₁² = P₂ + ρ₂u₂²
 * 3. Energy Conservation:   h₁ + ½u₁² = h₂ + ½u₂² + q
 * 4. C-J Condition:         u₂ = a₂ (sonic flow in burned gas)
 * 
 * Algorithm (SDT-based):
 * 1. Bracket C-J point by testing density ratios 1.5 ≤ ρ₂/ρ₁ ≤ 2.0
 * 2. For each ratio, use Newton-Raphson to solve Rankine-Hugoniot
 * 3. Fit parabola to wave speed vs density ratio data
 * 4. Find minimum analytically: ρ_cj = -b/(2a)
 * 
 * Reference:
 * SDToolbox - Numerical Tools for Shock and Detonation Wave Modeling
 * Explosion Dynamics Laboratory, Caltech, 2021
 */
class ChapmanJouguetSolver {
public:
    
    // Constructor
    explicit ChapmanJouguetSolver();
    
    // Destructor
    ~ChapmanJouguetSolver() = default;
    
    /**
     * @brief Thermodynamic state representation
     */
    struct ThermodynamicState {
        double pressure;        // [Pa]
        double temperature;     // [K]
        double density;         // [kg/m³]
        double enthalpy;        // [J/kg]
        double entropy;         // [J/kg/K]
        double soundSpeed;      // [m/s]
        double compressibility; // Z factor [-]
        double heatCapacityP;   // Cp [J/kg/K]
        double heatCapacityV;   // Cv [J/kg/K]
        double velocity;        // [m/s] (in lab frame)
        
        // Species concentrations (mole fractions)
        std::map<std::string, double> composition;
    };
    
    /**
     * @brief Initial conditions for C-J calculation
     */
    struct InitialConditions {
        double pressure;           // [Pa]
        double temperature;        // [K]
        double equivalenceRatio;   // φ = (fuel/oxidizer)actual / (fuel/oxidizer)stoichiometric
        std::string fuelType;      // "H2", "CH4", etc.
        std::string oxidizerType;  // "air", "O2"
        double diluentFraction;    // Inert gas fraction
    };
    
    /**
     * @brief C-J detonation solution
     */
    struct CJSolution {
        ThermodynamicState initialState;    // State 1 (unburned)
        ThermodynamicState finalState;      // State 2 (C-J point)
        
        // Detonation properties
        double cjVelocity;          // [m/s] C-J detonation velocity
        double cjPressure;          // [Pa] C-J pressure
        double cjTemperature;       // [K] C-J temperature
        double pressureRatio;       // P₂/P₁
        double densityRatio;        // ρ₂/ρ₁
        double temperatureRatio;    // T₂/T₁
        
        // Wave properties
        double machNumber;          // M₁ = D/a₁
        double heatRelease;         // [J/kg] q
        double impulse;             // [Pa·s/m] ∫(P-P₁)dt
        
        // Solution quality metrics
        bool converged;
        double residual;
        int iterations;
        std::vector<std::string> warnings;
    };
    
    // Main solver interface
    CJSolution solveCJ(const InitialConditions& initial);
    
    // Real gas property calculations
    ThermodynamicState calculateProperties(double pressure, double temperature, 
                                         const std::map<std::string, double>& composition);
    
    // Chemical equilibrium solver
    std::map<std::string, double> calculateEquilibrium(double pressure, double temperature,
                                                     const InitialConditions& initial);
    
    // Heat of combustion calculation
    double calculateHeatOfCombustion(const InitialConditions& initial);
    
    // Validation methods
    bool validateSolution(const CJSolution& solution);
    double calculateSolutionResidual(const CJSolution& solution);
    
    // Configuration
    void setConvergenceTolerance(double tolerance);
    void setMaxIterations(int maxIter);
    void enableRealGasEffects(bool enable);
    void setVerboseMode(bool verbose);
    
private:
    
    // Solver parameters
    double convergenceTolerance_;
    int maxIterations_;
    bool realGasEffects_;
    bool verboseMode_;
    
    // Property calculation methods
    double calculateCompressibilityFactor(double pressure, double temperature, 
                                        const std::map<std::string, double>& composition);
    
    double calculateSoundSpeed(double pressure, double temperature, double density,
                             const std::map<std::string, double>& composition);
    
    double calculateEnthalpy(double pressure, double temperature,
                           const std::map<std::string, double>& composition);
    
    double calculateEntropy(double pressure, double temperature,
                          const std::map<std::string, double>& composition);
    
    double calculateHeatCapacity(double temperature, const std::string& species, 
                               bool constantPressure = true);
    
    // Equation of state methods
    double peng_robinson_eos(double pressure, double temperature, 
                           const std::map<std::string, double>& composition);
    
    double calculateMolarVolume(double pressure, double temperature,
                              const std::map<std::string, double>& composition);
    
    // Chemical equilibrium methods
    std::vector<double> solveGibbsMinimization(double pressure, double temperature,
                                             const std::vector<double>& initialGuess);
    
    double calculateGibbsFreeEnergy(double temperature, const std::string& species);
    
    // Rankine-Hugoniot solver
    struct RHSolution {
        double pressure2;
        double density2;
        double velocity2;
        double temperature2;
    };
    
    RHSolution solveRankineHugoniot(const ThermodynamicState& state1, 
                                   double detonationVelocity,
                                   double heatRelease);
    
    // C-J condition solver
    double findCJVelocity(const InitialConditions& initial);
    
    // Numerical methods
    double newtonRaphsonSolver(std::function<double(double)> function,
                             std::function<double(double)> derivative,
                             double initialGuess);
    
    double bisectionSolver(std::function<double(double)> function,
                         double lowerBound, double upperBound);
    
    // Thermodynamic property database
    struct SpeciesData {
        std::string name;
        double molecularWeight;  // [kg/mol]
        double criticalPressure; // [Pa]
        double criticalTemperature; // [K]
        double acentricFactor;   // [-]
        
        // NASA polynomial coefficients for Cp/R calculation
        // Cp/R = a₁ + a₂T + a₃T² + a₄T³ + a₅T⁴
        std::vector<double> cpCoefficients; // High temperature range
        std::vector<double> cpCoefficientsLow; // Low temperature range
        double temperatureBreak; // [K] Temperature break between ranges
        
        // Standard formation enthalpy and entropy
        double hf298;  // [J/mol] at 298.15 K
        double s298;   // [J/mol/K] at 298.15 K
    };
    
    void initializeSpeciesDatabase();
    std::map<std::string, SpeciesData> speciesDatabase_;
    
    // Physical constants
    static constexpr double R_UNIVERSAL = 8314.0;      // [J/kmol/K]
    static constexpr double AVOGADRO = 6.02214076e23;  // [1/mol]
    static constexpr double BOLTZMANN = 1.380649e-23;  // [J/K]
    static constexpr double REFERENCE_PRESSURE = 101325.0; // [Pa]
    static constexpr double REFERENCE_TEMPERATURE = 298.15; // [K]
    
    // Numerical constants
    static constexpr double TOLERANCE_DEFAULT = 1e-6;
    static constexpr int MAX_ITERATIONS_DEFAULT = 100;
    static constexpr double PRESSURE_GUESS_FACTOR = 15.0; // Initial P₂/P₁ guess
    static constexpr double TEMPERATURE_GUESS_FACTOR = 10.0; // Initial T₂/T₁ guess
};

} // namespace Physics
} // namespace MCP
} // namespace Foam