/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Pipe Flow Analysis
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Pipe flow analysis tool for MCP server demonstration

    Implements a complete pipe flow analysis from user intent to OpenFOAM
    execution and results processing.

\*---------------------------------------------------------------------------*/

#ifndef pipe_flow_H
#define pipe_flow_H

#include <cmath>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "case_manager.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Struct PipeRoughness - Standard Pipe Roughness Database
\*---------------------------------------------------------------------------*/

struct PipeRoughness {
    // Absolute roughness values in meters (m)
    static constexpr double SMOOTH_DRAWN_TUBING = 0.0000015;  // Drawn tubing, glass
    static constexpr double COMMERCIAL_STEEL = 0.000045;      // Commercial steel, wrought iron
    static constexpr double CAST_IRON = 0.00026;              // Cast iron
    static constexpr double GALVANIZED_IRON = 0.00015;        // Galvanized iron
    static constexpr double ASPHALTED_CAST_IRON = 0.00012;    // Asphalted cast iron
    static constexpr double CONCRETE = 0.003;                 // Concrete (rough)
    static constexpr double CONCRETE_SMOOTH = 0.0003;         // Concrete (smooth)
    static constexpr double PVC = 0.0000015;                  // PVC, plastic pipes
    static constexpr double COPPER = 0.0000015;               // Copper tubing
    static constexpr double STAINLESS_STEEL = 0.000015;       // Stainless steel
    static constexpr double RIVETED_STEEL = 0.003;            // Riveted steel
    static constexpr double CORRUGATED_METAL = 0.045;         // Corrugated metal
    static constexpr double WOOD_STAVE = 0.0006;              // Wood stave

    // Get roughness by material name
    static double getByName(const std::string& material) {
        if (material == "smooth" || material == "drawn_tubing" || material == "glass")
            return SMOOTH_DRAWN_TUBING;
        if (material == "commercial_steel" || material == "steel" || material == "wrought_iron")
            return COMMERCIAL_STEEL;
        if (material == "cast_iron")
            return CAST_IRON;
        if (material == "galvanized_iron" || material == "galvanized")
            return GALVANIZED_IRON;
        if (material == "asphalted_cast_iron")
            return ASPHALTED_CAST_IRON;
        if (material == "concrete" || material == "concrete_rough")
            return CONCRETE;
        if (material == "concrete_smooth")
            return CONCRETE_SMOOTH;
        if (material == "pvc" || material == "plastic")
            return PVC;
        if (material == "copper")
            return COPPER;
        if (material == "stainless_steel" || material == "stainless")
            return STAINLESS_STEEL;
        if (material == "riveted_steel")
            return RIVETED_STEEL;
        if (material == "corrugated_metal" || material == "corrugated")
            return CORRUGATED_METAL;
        if (material == "wood_stave" || material == "wood")
            return WOOD_STAVE;
        return 0.0;  // Default smooth
    }

    // Get all available materials
    static std::vector<std::string> getAvailableMaterials() {
        return {
            "smooth", "drawn_tubing", "glass", "commercial_steel", "steel",
            "cast_iron", "galvanized_iron", "asphalted_cast_iron",
            "concrete", "concrete_smooth", "pvc", "plastic", "copper",
            "stainless_steel", "riveted_steel", "corrugated_metal", "wood_stave"
        };
    }
};

/*---------------------------------------------------------------------------*\
                        Struct PipeFlowInput
\*---------------------------------------------------------------------------*/

struct PipeFlowInput {
    double velocity;    // m/s
    double diameter;    // m
    double length;      // m
    double viscosity;   // m²/s
    double density;     // kg/m³
    double roughness;   // Absolute pipe roughness (m), 0 = smooth
    std::string fluid;  // "air", "water", etc.
    std::string pipeMaterial;  // Material name for roughness lookup

    PipeFlowInput()
        : velocity(1.0),
          diameter(0.1),
          length(1.0),
          viscosity(1e-5),
          density(1.225),
          roughness(0.0),
          fluid("air"),
          pipeMaterial("smooth") {}

    double getReynoldsNumber() const { return velocity * diameter / viscosity; }

    double getRelativeRoughness() const {
        return (roughness > 0.0) ? roughness / diameter : 0.0;
    }

    bool isLaminar() const { return getReynoldsNumber() < 2300; }

    bool isTurbulent() const { return getReynoldsNumber() > 4000; }

    bool isTransitional() const {
        double Re = getReynoldsNumber();
        return Re >= 2300 && Re <= 4000;
    }

    bool isHydraulicallySmooth() const {
        // Hydraulically smooth: roughness Reynolds number < 5
        double Re = getReynoldsNumber();
        if (Re < 2300 || roughness <= 0.0) return true;
        double f = 0.316 / std::pow(Re, 0.25);  // Blasius approximation
        double uStar = velocity * std::sqrt(f / 8.0);  // Friction velocity
        double roughnessRe = uStar * roughness / (viscosity / density);
        return roughnessRe < 5.0;
    }
};

/*---------------------------------------------------------------------------*\
                        Struct PipeFlowResults
\*---------------------------------------------------------------------------*/

struct PipeFlowResults {
    double reynoldsNumber;
    double frictionFactor;
    double pressureDrop;
    double wallShearStress;
    double maxVelocity;
    double averageVelocity;
    std::string flowRegime;
    std::string caseId;
    bool success;
    std::string errorMessage;

    PipeFlowResults()
        : reynoldsNumber(0),
          frictionFactor(0),
          pressureDrop(0),
          wallShearStress(0),
          maxVelocity(0),
          averageVelocity(0),
          flowRegime("unknown"),
          success(false) {}
};

/*---------------------------------------------------------------------------*\
                        Class PipeFlowAnalyzer Declaration
\*---------------------------------------------------------------------------*/

class PipeFlowAnalyzer {
   private:
    std::unique_ptr<CaseManager> caseManager_;

    void validateInput(const PipeFlowInput& input);
    CaseParameters createCaseParameters(const PipeFlowInput& input);
    PipeFlowResults processResults(const std::string& caseId, const PipeFlowInput& input);

    double calculateTheoreticalPressureDrop(const PipeFlowInput& input) const;
    double calculateLaminarFrictionFactor(double Re) const;
    double calculateTurbulentFrictionFactor(double Re) const;

    // Colebrook-White equation solver for rough pipes (iterative)
    double calculateColebrookWhiteFrictionFactor(double Re, double relativeRoughness,
                                                  int maxIterations = 50,
                                                  double tolerance = 1e-8) const;

    // Swamee-Jain explicit approximation for rough pipes (faster, <1% error)
    double calculateSwameeJainFrictionFactor(double Re, double relativeRoughness) const;

    // Haaland explicit approximation (alternative to Swamee-Jain)
    double calculateHaalandFrictionFactor(double Re, double relativeRoughness) const;

    // Calculate friction factor with roughness consideration
    double calculateFrictionFactorWithRoughness(const PipeFlowInput& input) const;

    std::string determineFlowRegime(const PipeFlowInput& input) const;
    std::string generateAnalysisReport(const PipeFlowInput& input,
                                       const PipeFlowResults& results) const;

   public:
    PipeFlowAnalyzer();
    explicit PipeFlowAnalyzer(std::unique_ptr<CaseManager> caseManager);
    ~PipeFlowAnalyzer() = default;

    PipeFlowResults analyze(const PipeFlowInput& input);

    json getInputSchema() const;

    static PipeFlowInput parseInput(const json& inputJson);
    static json resultsToJson(const PipeFlowResults& results);

    void setWorkingDirectory(const std::string& workingDir);

    std::vector<std::string> listActiveCases() const;
    bool deleteCaseData(const std::string& caseId);
};

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const PipeFlowInput& input);
void from_json(const json& j, PipeFlowInput& input);

void to_json(json& j, const PipeFlowResults& results);
void from_json(const json& j, PipeFlowResults& results);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //