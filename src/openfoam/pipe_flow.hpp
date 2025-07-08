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

#include "case_manager.hpp"
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace Foam
{
namespace MCP
{

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                        Struct PipeFlowInput
\*---------------------------------------------------------------------------*/

struct PipeFlowInput
{
    double velocity;        // m/s
    double diameter;        // m
    double length;          // m
    double viscosity;       // m²/s
    double density;         // kg/m³
    std::string fluid;      // "air", "water", etc.
    
    PipeFlowInput()
    : velocity(1.0)
    , diameter(0.1)
    , length(1.0)
    , viscosity(1e-5)
    , density(1.225)
    , fluid("air")
    {}
    
    double getReynoldsNumber() const
    {
        return velocity * diameter / viscosity;
    }
    
    bool isLaminar() const
    {
        return getReynoldsNumber() < 2300;
    }
    
    bool isTurbulent() const
    {
        return getReynoldsNumber() > 4000;
    }
    
    bool isTransitional() const
    {
        double Re = getReynoldsNumber();
        return Re >= 2300 && Re <= 4000;
    }
};

/*---------------------------------------------------------------------------*\
                        Struct PipeFlowResults
\*---------------------------------------------------------------------------*/

struct PipeFlowResults
{
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
    : reynoldsNumber(0)
    , frictionFactor(0)
    , pressureDrop(0)
    , wallShearStress(0)
    , maxVelocity(0)
    , averageVelocity(0)
    , flowRegime("unknown")
    , success(false)
    {}
};

/*---------------------------------------------------------------------------*\
                        Class PipeFlowAnalyzer Declaration
\*---------------------------------------------------------------------------*/

class PipeFlowAnalyzer
{
private:

    std::unique_ptr<CaseManager> caseManager_;
    
    void validateInput(const PipeFlowInput& input);
    CaseParameters createCaseParameters(const PipeFlowInput& input);
    PipeFlowResults processResults(const std::string& caseId, const PipeFlowInput& input);
    
    double calculateTheoreticalPressureDrop(const PipeFlowInput& input) const;
    double calculateLaminarFrictionFactor(double Re) const;
    double calculateTurbulentFrictionFactor(double Re) const;
    
    std::string determineFlowRegime(const PipeFlowInput& input) const;
    std::string generateAnalysisReport(const PipeFlowInput& input, const PipeFlowResults& results) const;

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

} // End namespace MCP
} // End namespace Foam

#endif

// ************************************************************************* //