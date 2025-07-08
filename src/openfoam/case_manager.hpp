/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Case Manager
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    OpenFOAM case management system for MCP server

    Handles creation, execution, and cleanup of OpenFOAM cases
    with proper resource management and error handling.

\*---------------------------------------------------------------------------*/

#ifndef case_manager_H
#define case_manager_H

#include <chrono>
#include <filesystem>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Foam {
namespace MCP {

using json = nlohmann::json;
namespace fs = std::filesystem;

/*---------------------------------------------------------------------------*\
                        Struct CaseParameters
\*---------------------------------------------------------------------------*/

struct CaseParameters {
    std::string caseName;
    std::string solver;
    std::string caseType;
    double endTime;
    double deltaTime;
    int writeInterval;
    std::map<std::string, std::string> boundaryConditions;
    std::map<std::string, std::string> physicalProperties;
    std::map<std::string, std::string> numericalSchemes;

    CaseParameters()
        : caseName("case"),
          solver("simpleFoam"),
          caseType("steady"),
          endTime(1000.0),
          deltaTime(1.0),
          writeInterval(100) {}
};

/*---------------------------------------------------------------------------*\
                        Struct CaseResult
\*---------------------------------------------------------------------------*/

struct CaseResult {
    std::string caseId;
    std::string status;
    int exitCode;
    std::string logOutput;
    std::string errorOutput;
    std::chrono::milliseconds executionTime;
    std::map<std::string, double> results;
    std::vector<std::string> outputFiles;

    CaseResult() : status("unknown"), exitCode(-1), executionTime(0) {}

    bool isSuccess() const {
        return status == "completed" && exitCode == 0;
    }
    bool isRunning() const {
        return status == "running";
    }
    bool isFailed() const {
        return status == "failed" || exitCode != 0;
    }
};

/*---------------------------------------------------------------------------*\
                        Class CaseManager Declaration
\*---------------------------------------------------------------------------*/

class CaseManager {
   private:
    fs::path workingDirectory_;
    std::map<std::string, std::unique_ptr<CaseResult>> caseResults_;

    std::string generateCaseId() const;
    fs::path getCasePath(const std::string& caseId) const;

    void createCaseDirectory(const std::string& caseId) const;
    void setupCaseStructure(const std::string& caseId, const CaseParameters& params) const;

    void writeControlDict(const fs::path& casePath, const CaseParameters& params) const;
    void writeFvSchemes(const fs::path& casePath, const CaseParameters& params) const;
    void writeFvSolution(const fs::path& casePath, const CaseParameters& params) const;
    void writeTransportProperties(const fs::path& casePath, const CaseParameters& params) const;
    void writeTurbulenceProperties(const fs::path& casePath, const CaseParameters& params) const;

    void setupMesh(const std::string& caseId, const CaseParameters& params) const;
    void setupBoundaryConditions(const std::string& caseId, const CaseParameters& params) const;

    void parseLogOutput(const std::string& caseId, const std::string& logOutput);
    void extractResults(const std::string& caseId);

    void cleanup(const std::string& caseId);

   public:
    CaseManager();
    explicit CaseManager(const fs::path& workingDir);
    ~CaseManager();

    std::string createCase(const CaseParameters& params);

    bool runCase(const std::string& caseId);

    CaseResult getCaseResult(const std::string& caseId) const;

    std::vector<std::string> listCases() const;

    bool deleteCaseData(const std::string& caseId);

    void setWorkingDirectory(const fs::path& workingDir);
    fs::path getWorkingDirectory() const {
        return workingDirectory_;
    }

    bool validateCaseParameters(const CaseParameters& params) const;

    std::vector<std::string> getAvailableSolvers() const;

    json getCaseParametersSchema() const;

    static CaseParameters createPipeFlowParameters(double velocity, double diameter, double length);
    static CaseParameters createCavityFlowParameters(double velocity, double viscosity);
};

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const CaseParameters& params);
void from_json(const json& j, CaseParameters& params);

void to_json(json& j, const CaseResult& result);
void from_json(const json& j, CaseResult& result);

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //