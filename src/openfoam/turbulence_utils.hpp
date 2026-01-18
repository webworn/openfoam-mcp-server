/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Turbulence Utilities
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Turbulence utility functions for y+ calculations, wall function validation,
    and turbulence parameter estimation.

    Provides tools for:
    - y+ calculation from wall shear stress
    - First cell height recommendations for target y+
    - Wall function validity assessment
    - Turbulence intensity and length scale estimation
    - Inlet boundary condition parameter calculation

\*---------------------------------------------------------------------------*/

#ifndef turbulence_utils_H
#define turbulence_utils_H

#include <cmath>
#include <string>
#include <vector>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Struct YPlusResult
\*---------------------------------------------------------------------------*/

struct YPlusResult {
    double yPlus;                    // Calculated y+ value
    std::string region;              // "viscous_sublayer", "buffer_layer", "log_law", "outer"
    bool validForWallFunction;       // True if y+ is in valid range for wall functions
    std::string recommendation;      // Recommendation for mesh refinement
    double firstCellHeight;          // First cell height used in calculation

    YPlusResult()
        : yPlus(0.0),
          region("unknown"),
          validForWallFunction(false),
          firstCellHeight(0.0) {}
};

/*---------------------------------------------------------------------------*\
                        Struct TurbulenceInletConditions
\*---------------------------------------------------------------------------*/

struct TurbulenceInletConditions {
    double k;           // Turbulent kinetic energy [m²/s²]
    double epsilon;     // Turbulent dissipation rate [m²/s³]
    double omega;       // Specific turbulence dissipation rate [1/s]
    double nut;         // Turbulent viscosity [m²/s]

    // Input parameters used
    double intensity;   // Turbulence intensity (0-1)
    double lengthScale; // Turbulent length scale [m]
    double velocity;    // Mean velocity [m/s]
    double viscosity;   // Kinematic viscosity [m²/s]

    TurbulenceInletConditions()
        : k(0.0), epsilon(0.0), omega(0.0), nut(0.0),
          intensity(0.0), lengthScale(0.0), velocity(0.0), viscosity(0.0) {}
};

/*---------------------------------------------------------------------------*\
                        Struct WallFunctionValidity
\*---------------------------------------------------------------------------*/

struct WallFunctionValidity {
    std::string turbulenceModel;     // Model being assessed
    bool isValid;                    // Overall validity
    double minYPlus;                 // Minimum valid y+
    double maxYPlus;                 // Maximum valid y+
    double targetYPlus;              // Recommended target y+
    std::string explanation;         // Detailed explanation
    std::vector<std::string> recommendations;

    WallFunctionValidity()
        : isValid(false), minYPlus(0.0), maxYPlus(0.0), targetYPlus(0.0) {}
};

/*---------------------------------------------------------------------------*\
                        Class TurbulenceUtils Declaration
\*---------------------------------------------------------------------------*/

class TurbulenceUtils {
   public:
    // ----- y+ Calculations -----

    // Calculate y+ from wall shear stress and first cell height
    static double calculateYPlus(double wallShearStress, double density,
                                 double kinematicViscosity, double firstCellHeight);

    // Calculate y+ from friction velocity directly
    static double calculateYPlusFromFrictionVelocity(double uTau, double kinematicViscosity,
                                                      double firstCellHeight);

    // Calculate friction velocity from wall shear stress
    static double calculateFrictionVelocity(double wallShearStress, double density);

    // Estimate wall shear stress from friction factor
    static double estimateWallShearStress(double frictionFactor, double density, double velocity);

    // ----- First Cell Height Recommendations -----

    // Calculate recommended first cell height for target y+
    static double recommendFirstCellHeight(double targetYPlus, double velocity,
                                           double density, double kinematicViscosity,
                                           double characteristicLength);

    // Calculate first cell height from estimated friction velocity
    static double calculateFirstCellHeightFromUTau(double targetYPlus, double uTau,
                                                    double kinematicViscosity);

    // Quick estimation using flat plate correlation
    static double estimateFirstCellHeightFlatPlate(double targetYPlus, double freeStreamVelocity,
                                                    double density, double kinematicViscosity,
                                                    double plateLength);

    // Pipe flow specific first cell height
    static double estimateFirstCellHeightPipe(double targetYPlus, double velocity,
                                              double density, double kinematicViscosity,
                                              double pipeDiameter);

    // ----- Wall Function Validity -----

    // Check if y+ is valid for given turbulence model
    static WallFunctionValidity checkWallFunctionValidity(double yPlus,
                                                          const std::string& turbulenceModel);

    // Get comprehensive y+ analysis
    static YPlusResult analyzeYPlus(double yPlus, const std::string& turbulenceModel);

    // Determine wall region from y+
    static std::string determineWallRegion(double yPlus);

    // ----- Turbulence Inlet Conditions -----

    // Calculate inlet boundary conditions from intensity and length scale
    static TurbulenceInletConditions calculateInletConditions(
        double velocity, double turbulentIntensity, double turbulentLengthScale,
        double kinematicViscosity);

    // Estimate turbulence intensity for different flow conditions
    static double estimateTurbulenceIntensity(const std::string& flowType);

    // Estimate turbulent length scale
    static double estimateTurbulentLengthScale(double characteristicLength,
                                               const std::string& flowType = "internal");

    // ----- Helper Functions -----

    // Calculate turbulent viscosity ratio (nut/nu)
    static double calculateTurbulentViscosityRatio(double k, double epsilon,
                                                   double kinematicViscosity);

    // Calculate turbulent Reynolds number
    static double calculateTurbulentReynoldsNumber(double k, double kinematicViscosity);

    // Get k-epsilon model constant Cmu
    static constexpr double Cmu() { return 0.09; }

    // Get von Karman constant
    static constexpr double kappa() { return 0.41; }

    // Get log-law additive constant
    static constexpr double E() { return 9.8; }

   private:
    // Prevent instantiation (static utility class)
    TurbulenceUtils() = default;
};

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //
