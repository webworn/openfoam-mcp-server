/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Version:  12
     \\/     M anipulation  |
\*---------------------------------------------------------------------------*/

#ifndef RDE_THRUST_CALCULATOR_HPP
#define RDE_THRUST_CALCULATOR_HPP

#include <vector>
#include <string>
#include <map>

/**
 * @brief Rotating Detonation Engine Thrust Calculator
 * 
 * Calculates thrust performance from 2D RDE simulation data using:
 * - Momentum balance equations
 * - Chapman-Jouguet theory
 * - Specific impulse calculations
 * - Thrust-to-weight ratios
 * - Performance efficiency metrics
 */
class RDEThrustCalculator {
public:
    // Thrust calculation methods
    enum class ThrustMethod {
        MOMENTUM_BALANCE,     // F = ṁ_exit*V_exit - ṁ_inlet*V_inlet + (P_exit - P_amb)*A_exit
        PRESSURE_INTEGRAL,    // F = ∫∫ P·dA (pressure integration over exit plane)
        CHAPMAN_JOUGUET,      // Theoretical thrust from C-J detonation theory
        CONTROL_VOLUME       // Control volume momentum analysis
    };

    // Performance metrics structure
    struct ThrustPerformance {
        double thrust_N;                 // Thrust [N]
        double specific_impulse_s;       // Isp [s] 
        double thrust_to_weight;         // T/W ratio
        double combustion_efficiency;    // η_combustion
        double nozzle_efficiency;        // η_nozzle
        double overall_efficiency;       // η_overall
        double mass_flow_rate_kg_s;      // ṁ [kg/s]
        double exit_velocity_m_s;        // V_exit [m/s]
        double chamber_pressure_Pa;      // P_chamber [Pa]
        double pressure_ratio;           // P_chamber/P_ambient
        double characteristic_velocity;  // c* [m/s]
        double thrust_coefficient;       // C_F = F/(P_chamber*A_throat)
        
        // RDE-specific metrics
        double detonation_frequency_Hz;  // Wave rotation frequency
        double pressure_gain_ratio;      // RDE pressure gain vs deflagration
        double wave_speed_m_s;           // Detonation wave velocity
        double cell_size_mm;             // Detonation cell size
        double filling_fraction;         // Chamber volume utilization
    };

    // Thermodynamic state
    struct FlowState {
        double pressure_Pa;
        double temperature_K;
        double density_kg_m3;
        double velocity_m_s;
        double mach_number;
        std::map<std::string, double> species_fractions;
        double molecular_weight_kg_mol;
        double gamma;                    // Specific heat ratio
        double enthalpy_J_kg;
    };

private:
    // System geometry
    double outer_radius_m_;
    double inner_radius_m_;
    double chamber_length_m_;
    double throat_area_m2_;
    double exit_area_m2_;
    double chamber_volume_m3_;
    
    // Operating conditions
    double ambient_pressure_Pa_;
    double ambient_temperature_K_;
    double fuel_flow_rate_kg_s_;
    double oxidizer_flow_rate_kg_s_;
    
    // RDE-specific parameters
    double wave_speed_m_s_;
    double detonation_frequency_Hz_;
    double cell_size_m_;
    double filling_fraction_;
    
    // Performance calculations
    ThrustPerformance performance_metrics_;
    FlowState inlet_state_;
    FlowState chamber_state_;
    FlowState exit_state_;

public:
    // Constructor
    RDEThrustCalculator(double outer_radius, double inner_radius, 
                       double chamber_length, double throat_area, double exit_area);
    
    // Configuration methods
    void setOperatingConditions(double ambient_p, double ambient_T, 
                               double fuel_flow, double oxidizer_flow);
    void setRDEParameters(double wave_speed, double frequency, 
                         double cell_size, double filling_fraction);
    
    // Thrust calculation methods
    ThrustPerformance calculateThrust(ThrustMethod method = ThrustMethod::MOMENTUM_BALANCE);
    double calculateMomentumThrust(const FlowState& inlet, const FlowState& exit);
    double calculatePressureThrust(const std::vector<double>& pressure_field,
                                  const std::vector<double>& area_elements);
    double calculateChapmanJouguetThrust();
    
    // Performance analysis
    double calculateSpecificImpulse(double thrust_N, double mass_flow_kg_s);
    double calculateThrustToWeight(double thrust_N, double engine_mass_kg);
    double calculateCombustionEfficiency(const FlowState& chamber);
    double calculateOverallEfficiency();
    double calculateCharacteristicVelocity(double chamber_pressure, double throat_area, double mass_flow);
    
    // RDE-specific calculations
    double calculateDetonationFrequency(double wave_speed, double chamber_circumference);
    double calculatePressureGainRatio();
    double calculateFillingFraction(double chamber_volume, double wave_thickness);
    double calculateWaveSpeedFromCellSize(double cell_size, const FlowState& mixture);
    
    // Chapman-Jouguet theory
    FlowState calculateChapmanJouguetState(const FlowState& initial_mixture);
    double calculateChapmanJouguetVelocity(const FlowState& mixture);
    double calculateChapmanJouguetPressure(const FlowState& mixture);
    
    // Thermodynamic calculations
    FlowState calculateThermodynamicState(double p, double T, 
                                         const std::map<std::string, double>& species);
    double calculateMixtureGamma(const std::map<std::string, double>& species, double T);
    double calculateMixtureMolecularWeight(const std::map<std::string, double>& species);
    double calculateMixtureEnthalpy(const std::map<std::string, double>& species, double T);
    
    // Data analysis from OpenFOAM results
    void loadSimulationData(const std::string& case_directory, double time);
    void analyzeTimeSeriesData(const std::string& case_directory, 
                              double start_time, double end_time, double dt);
    std::vector<double> extractPressureAtExit(const std::string& field_file);
    std::vector<double> extractVelocityAtExit(const std::string& field_file);
    std::vector<double> extractSpeciesAtExit(const std::string& species_name, const std::string& field_file);
    
    // Performance optimization
    void optimizeThrustPerformance();
    std::vector<double> sensitivityAnalysis(const std::vector<std::string>& parameters);
    void generatePerformanceMap(double equivalence_ratio_min, double equivalence_ratio_max,
                               double pressure_min, double pressure_max);
    
    // Output and visualization
    void writePerformanceReport(const std::string& filename);
    void generateThrustPlots(const std::string& output_directory);
    void exportPerformanceData(const std::string& filename, const std::string& format);
    
    // Validation against experimental data
    void validateWithExperimentalData(const std::string& experimental_data_file);
    double calculateValidationError(const std::vector<double>& simulated, 
                                   const std::vector<double>& experimental);
    
    // Getters
    const ThrustPerformance& getPerformanceMetrics() const { return performance_metrics_; }
    const FlowState& getInletState() const { return inlet_state_; }
    const FlowState& getChamberState() const { return chamber_state_; }
    const FlowState& getExitState() const { return exit_state_; }
    
    // Educational content
    std::string explainThrustCalculation() const;
    std::string explainRDEAdvantages() const;
    std::string explainChapmanJouguetTheory() const;
    std::vector<std::string> generateSocraticQuestions() const;
};

// Utility functions for thrust calculations
namespace RDEThrustUtils {
    // Standard atmosphere properties
    double getStandardAtmospherePressure(double altitude_m);
    double getStandardAtmosphereTemperature(double altitude_m);
    double getStandardAtmosphereDensity(double altitude_m);
    
    // Propellant properties
    double getH2LowerHeatingValue(); // MJ/kg
    double getH2HigherHeatingValue(); // MJ/kg
    double getAirSpecificHeatRatio(double temperature_K);
    double getCombustionProductGamma(double temperature_K);
    
    // Conversion utilities
    double pascalToAtmosphere(double pascal);
    double newtonToLbf(double newton);
    double specificImpulseToExhaustVelocity(double isp_s);
    double exhaustVelocityToSpecificImpulse(double velocity_m_s);
    
    // RDE performance correlations
    double estimateRDEPressureGain(double equivalence_ratio, double inlet_pressure);
    double estimateRDEThrustDensity(double chamber_pressure, double specific_impulse);
    double estimateRDEEngineWeight(double thrust_N, double technology_factor);
}

#endif // RDE_THRUST_CALCULATOR_HPP