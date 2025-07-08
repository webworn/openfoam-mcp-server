/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Case Manager
    \\  /    A nd           | 
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    OpenFOAM case management system implementation

\*---------------------------------------------------------------------------*/

#include "case_manager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <iomanip>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                        JSON Conversion Functions
\*---------------------------------------------------------------------------*/

void to_json(json& j, const CaseParameters& params)
{
    j = json{
        {"caseName", params.caseName},
        {"solver", params.solver},
        {"caseType", params.caseType},
        {"endTime", params.endTime},
        {"deltaTime", params.deltaTime},
        {"writeInterval", params.writeInterval},
        {"boundaryConditions", params.boundaryConditions},
        {"physicalProperties", params.physicalProperties},
        {"numericalSchemes", params.numericalSchemes}
    };
}

void from_json(const json& j, CaseParameters& params)
{
    j.at("caseName").get_to(params.caseName);
    j.at("solver").get_to(params.solver);
    j.at("caseType").get_to(params.caseType);
    j.at("endTime").get_to(params.endTime);
    j.at("deltaTime").get_to(params.deltaTime);
    j.at("writeInterval").get_to(params.writeInterval);
    j.at("boundaryConditions").get_to(params.boundaryConditions);
    j.at("physicalProperties").get_to(params.physicalProperties);
    j.at("numericalSchemes").get_to(params.numericalSchemes);
}

void to_json(json& j, const CaseResult& result)
{
    j = json{
        {"caseId", result.caseId},
        {"status", result.status},
        {"exitCode", result.exitCode},
        {"logOutput", result.logOutput},
        {"errorOutput", result.errorOutput},
        {"executionTime", result.executionTime.count()},
        {"results", result.results},
        {"outputFiles", result.outputFiles}
    };
}

void from_json(const json& j, CaseResult& result)
{
    j.at("caseId").get_to(result.caseId);
    j.at("status").get_to(result.status);
    j.at("exitCode").get_to(result.exitCode);
    j.at("logOutput").get_to(result.logOutput);
    j.at("errorOutput").get_to(result.errorOutput);
    
    auto timeMs = j.at("executionTime").get<long>();
    result.executionTime = std::chrono::milliseconds(timeMs);
    
    j.at("results").get_to(result.results);
    j.at("outputFiles").get_to(result.outputFiles);
}

/*---------------------------------------------------------------------------*\
                        CaseManager Implementation
\*---------------------------------------------------------------------------*/

CaseManager::CaseManager()
: workingDirectory_("/tmp/openfoam-mcp-cases")
{
    if (!fs::exists(workingDirectory_))
    {
        fs::create_directories(workingDirectory_);
    }
}

CaseManager::CaseManager(const fs::path& workingDir)
: workingDirectory_(workingDir)
{
    if (!fs::exists(workingDirectory_))
    {
        fs::create_directories(workingDirectory_);
    }
}

CaseManager::~CaseManager()
{
    for (const auto& [caseId, result] : caseResults_)
    {
        cleanup(caseId);
    }
}

std::string CaseManager::generateCaseId() const
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "case_";
    
    for (int i = 0; i < 8; ++i)
    {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}

fs::path CaseManager::getCasePath(const std::string& caseId) const
{
    return workingDirectory_ / caseId;
}

void CaseManager::createCaseDirectory(const std::string& caseId) const
{
    fs::path casePath = getCasePath(caseId);
    
    fs::create_directories(casePath / "0");
    fs::create_directories(casePath / "constant");
    fs::create_directories(casePath / "system");
}

std::string CaseManager::createCase(const CaseParameters& params)
{
    if (!validateCaseParameters(params))
    {
        throw std::runtime_error("Invalid case parameters");
    }
    
    std::string caseId = generateCaseId();
    
    createCaseDirectory(caseId);
    setupCaseStructure(caseId, params);
    
    auto result = std::make_unique<CaseResult>();
    result->caseId = caseId;
    result->status = "created";
    
    caseResults_[caseId] = std::move(result);
    
    return caseId;
}

void CaseManager::setupCaseStructure(const std::string& caseId, const CaseParameters& params) const
{
    fs::path casePath = getCasePath(caseId);
    
    writeControlDict(casePath, params);
    writeFvSchemes(casePath, params);
    writeFvSolution(casePath, params);
    writeTransportProperties(casePath, params);
    writeTurbulenceProperties(casePath, params);
    
    setupMesh(caseId, params);
    setupBoundaryConditions(caseId, params);
}

void CaseManager::writeControlDict(const fs::path& casePath, const CaseParameters& params) const
{
    std::ofstream file(casePath / "system" / "controlDict");
    
    file << "FoamFile\n"
         << "{\n"
         << "    version     2.0;\n"
         << "    format      ascii;\n"
         << "    class       dictionary;\n"
         << "    location    \"system\";\n"
         << "    object      controlDict;\n"
         << "}\n\n";
    
    file << "application     " << params.solver << ";\n\n";
    file << "startFrom       startTime;\n";
    file << "startTime       0;\n";
    file << "stopAt          endTime;\n";
    file << "endTime         " << params.endTime << ";\n";
    file << "deltaT          " << params.deltaTime << ";\n";
    file << "writeControl    timeStep;\n";
    file << "writeInterval   " << params.writeInterval << ";\n";
    file << "purgeWrite      0;\n";
    file << "writeFormat     ascii;\n";
    file << "writePrecision  6;\n";
    file << "writeCompression off;\n";
    file << "timeFormat      general;\n";
    file << "timePrecision   6;\n";
    file << "runTimeModifiable true;\n";
}

void CaseManager::writeFvSchemes(const fs::path& casePath, const CaseParameters& params) const
{
    std::ofstream file(casePath / "system" / "fvSchemes");
    
    file << "FoamFile\n"
         << "{\n"
         << "    version     2.0;\n"
         << "    format      ascii;\n"
         << "    class       dictionary;\n"
         << "    location    \"system\";\n"
         << "    object      fvSchemes;\n"
         << "}\n\n";
    
    file << "ddtSchemes\n"
         << "{\n"
         << "    default         steadyState;\n"
         << "}\n\n";
    
    file << "gradSchemes\n"
         << "{\n"
         << "    default         Gauss linear;\n"
         << "}\n\n";
    
    file << "divSchemes\n"
         << "{\n"
         << "    default         none;\n"
         << "    div(phi,U)      bounded Gauss upwind;\n"
         << "    div(phi,k)      bounded Gauss upwind;\n"
         << "    div(phi,epsilon) bounded Gauss upwind;\n"
         << "    div((nuEff*dev2(T(grad(U))))) Gauss linear;\n"
         << "}\n\n";
    
    file << "laplacianSchemes\n"
         << "{\n"
         << "    default         Gauss linear corrected;\n"
         << "}\n\n";
    
    file << "interpolationSchemes\n"
         << "{\n"
         << "    default         linear;\n"
         << "}\n\n";
    
    file << "snGradSchemes\n"
         << "{\n"
         << "    default         corrected;\n"
         << "}\n\n";
}

void CaseManager::writeFvSolution(const fs::path& casePath, const CaseParameters& params) const
{
    std::ofstream file(casePath / "system" / "fvSolution");
    
    file << "FoamFile\n"
         << "{\n"
         << "    version     2.0;\n"
         << "    format      ascii;\n"
         << "    class       dictionary;\n"
         << "    location    \"system\";\n"
         << "    object      fvSolution;\n"
         << "}\n\n";
    
    file << "solvers\n"
         << "{\n"
         << "    p\n"
         << "    {\n"
         << "        solver          GAMG;\n"
         << "        tolerance       1e-06;\n"
         << "        relTol          0.1;\n"
         << "        smoother        GaussSeidel;\n"
         << "    }\n\n";
    
    file << "    U\n"
         << "    {\n"
         << "        solver          smoothSolver;\n"
         << "        smoother        GaussSeidel;\n"
         << "        tolerance       1e-05;\n"
         << "        relTol          0.1;\n"
         << "    }\n\n";
    
    file << "    k\n"
         << "    {\n"
         << "        solver          smoothSolver;\n"
         << "        smoother        GaussSeidel;\n"
         << "        tolerance       1e-05;\n"
         << "        relTol          0.1;\n"
         << "    }\n\n";
    
    file << "    epsilon\n"
         << "    {\n"
         << "        solver          smoothSolver;\n"
         << "        smoother        GaussSeidel;\n"
         << "        tolerance       1e-05;\n"
         << "        relTol          0.1;\n"
         << "    }\n"
         << "}\n\n";
    
    file << "SIMPLE\n"
         << "{\n"
         << "    nNonOrthogonalCorrectors 0;\n"
         << "    consistent      yes;\n"
         << "    \n"
         << "    residualControl\n"
         << "    {\n"
         << "        p               1e-5;\n"
         << "        U               1e-5;\n"
         << "        \"(k|epsilon)\"   1e-5;\n"
         << "    }\n"
         << "}\n\n";
    
    file << "relaxationFactors\n"
         << "{\n"
         << "    fields\n"
         << "    {\n"
         << "        p               0.3;\n"
         << "    }\n"
         << "    equations\n"
         << "    {\n"
         << "        U               0.7;\n"
         << "        \"(k|epsilon)\"   0.7;\n"
         << "    }\n"
         << "}\n\n";
}

void CaseManager::writeTransportProperties(const fs::path& casePath, const CaseParameters& params) const
{
    std::ofstream file(casePath / "constant" / "transportProperties");
    
    file << "FoamFile\n"
         << "{\n"
         << "    version     2.0;\n"
         << "    format      ascii;\n"
         << "    class       dictionary;\n"
         << "    location    \"constant\";\n"
         << "    object      transportProperties;\n"
         << "}\n\n";
    
    std::string nu = "1e-05";
    if (params.physicalProperties.find("nu") != params.physicalProperties.end())
    {
        nu = params.physicalProperties.at("nu");
    }
    
    file << "transportModel  Newtonian;\n";
    file << "nu              " << nu << ";\n";
}

void CaseManager::writeTurbulenceProperties(const fs::path& casePath, const CaseParameters& params) const
{
    std::ofstream file(casePath / "constant" / "turbulenceProperties");
    
    file << "FoamFile\n"
         << "{\n"
         << "    version     2.0;\n"
         << "    format      ascii;\n"
         << "    class       dictionary;\n"
         << "    location    \"constant\";\n"
         << "    object      turbulenceProperties;\n"
         << "}\n\n";
    
    file << "simulationType  RAS;\n\n";
    file << "RAS\n"
         << "{\n"
         << "    RASModel        kEpsilon;\n"
         << "    turbulence      on;\n"
         << "    printCoeffs     on;\n"
         << "}\n\n";
}

void CaseManager::setupMesh(const std::string& caseId, const CaseParameters& params) const
{
    fs::path casePath = getCasePath(caseId);
    
    std::ofstream file(casePath / "system" / "blockMeshDict");
    
    file << "FoamFile\n"
         << "{\n"
         << "    version     2.0;\n"
         << "    format      ascii;\n"
         << "    class       dictionary;\n"
         << "    object      blockMeshDict;\n"
         << "}\n\n";
    
    file << "convertToMeters 1;\n\n";
    
    file << "vertices\n"
         << "(\n"
         << "    (0 0 0)\n"
         << "    (1 0 0)\n"
         << "    (1 1 0)\n"
         << "    (0 1 0)\n"
         << "    (0 0 0.1)\n"
         << "    (1 0 0.1)\n"
         << "    (1 1 0.1)\n"
         << "    (0 1 0.1)\n"
         << ");\n\n";
    
    file << "blocks\n"
         << "(\n"
         << "    hex (0 1 2 3 4 5 6 7) (20 20 1) simpleGrading (1 1 1)\n"
         << ");\n\n";
    
    file << "edges\n"
         << "(\n"
         << ");\n\n";
    
    file << "boundary\n"
         << "(\n"
         << "    inlet\n"
         << "    {\n"
         << "        type patch;\n"
         << "        faces\n"
         << "        (\n"
         << "            (0 4 7 3)\n"
         << "        );\n"
         << "    }\n"
         << "    outlet\n"
         << "    {\n"
         << "        type patch;\n"
         << "        faces\n"
         << "        (\n"
         << "            (2 6 5 1)\n"
         << "        );\n"
         << "    }\n"
         << "    walls\n"
         << "    {\n"
         << "        type wall;\n"
         << "        faces\n"
         << "        (\n"
         << "            (1 5 4 0)\n"
         << "            (3 7 6 2)\n"
         << "        );\n"
         << "    }\n"
         << "    frontAndBack\n"
         << "    {\n"
         << "        type empty;\n"
         << "        faces\n"
         << "        (\n"
         << "            (0 3 2 1)\n"
         << "            (4 5 6 7)\n"
         << "        );\n"
         << "    }\n"
         << ");\n\n";
    
    file << "mergePatchPairs\n"
         << "(\n"
         << ");\n\n";
}

void CaseManager::setupBoundaryConditions(const std::string& caseId, const CaseParameters& params) const
{
    fs::path casePath = getCasePath(caseId);
    
    std::ofstream pFile(casePath / "0" / "p");
    pFile << "FoamFile\n"
          << "{\n"
          << "    version     2.0;\n"
          << "    format      ascii;\n"
          << "    class       volScalarField;\n"
          << "    object      p;\n"
          << "}\n\n";
    
    pFile << "dimensions      [0 2 -2 0 0 0 0];\n";
    pFile << "internalField   uniform 0;\n\n";
    pFile << "boundaryField\n"
          << "{\n"
          << "    inlet\n"
          << "    {\n"
          << "        type            zeroGradient;\n"
          << "    }\n"
          << "    outlet\n"
          << "    {\n"
          << "        type            fixedValue;\n"
          << "        value           uniform 0;\n"
          << "    }\n"
          << "    walls\n"
          << "    {\n"
          << "        type            zeroGradient;\n"
          << "    }\n"
          << "    frontAndBack\n"
          << "    {\n"
          << "        type            empty;\n"
          << "    }\n"
          << "}\n\n";
    
    std::ofstream UFile(casePath / "0" / "U");
    UFile << "FoamFile\n"
          << "{\n"
          << "    version     2.0;\n"
          << "    format      ascii;\n"
          << "    class       volVectorField;\n"
          << "    object      U;\n"
          << "}\n\n";
    
    UFile << "dimensions      [0 1 -1 0 0 0 0];\n";
    UFile << "internalField   uniform (0 0 0);\n\n";
    UFile << "boundaryField\n"
          << "{\n"
          << "    inlet\n"
          << "    {\n"
          << "        type            fixedValue;\n"
          << "        value           uniform (1 0 0);\n"
          << "    }\n"
          << "    outlet\n"
          << "    {\n"
          << "        type            zeroGradient;\n"
          << "    }\n"
          << "    walls\n"
          << "    {\n"
          << "        type            noSlip;\n"
          << "    }\n"
          << "    frontAndBack\n"
          << "    {\n"
          << "        type            empty;\n"
          << "    }\n"
          << "}\n\n";
    
    // Add turbulent viscosity field (nut)
    std::ofstream nutFile(casePath / "0" / "nut");
    nutFile << "FoamFile\n"
            << "{\n"
            << "    version     2.0;\n"
            << "    format      ascii;\n"
            << "    class       volScalarField;\n"
            << "    object      nut;\n"
            << "}\n\n";
    
    nutFile << "dimensions      [0 2 -1 0 0 0 0];\n";
    nutFile << "internalField   uniform 0;\n\n";
    nutFile << "boundaryField\n"
            << "{\n"
            << "    inlet\n"
            << "    {\n"
            << "        type            calculated;\n"
            << "        value           uniform 0;\n"
            << "    }\n"
            << "    outlet\n"
            << "    {\n"
            << "        type            calculated;\n"
            << "        value           uniform 0;\n"
            << "    }\n"
            << "    walls\n"
            << "    {\n"
            << "        type            nutkWallFunction;\n"
            << "        value           uniform 0;\n"
            << "    }\n"
            << "    frontAndBack\n"
            << "    {\n"
            << "        type            empty;\n"
            << "    }\n"
            << "}\n\n";
    
    // Add turbulent kinetic energy field (k)
    std::ofstream kFile(casePath / "0" / "k");
    kFile << "FoamFile\n"
          << "{\n"
          << "    version     2.0;\n"
          << "    format      ascii;\n"
          << "    class       volScalarField;\n"
          << "    object      k;\n"
          << "}\n\n";
    
    kFile << "dimensions      [0 2 -2 0 0 0 0];\n";
    kFile << "internalField   uniform 0.1;\n\n";
    kFile << "boundaryField\n"
          << "{\n"
          << "    inlet\n"
          << "    {\n"
          << "        type            fixedValue;\n"
          << "        value           uniform 0.1;\n"
          << "    }\n"
          << "    outlet\n"
          << "    {\n"
          << "        type            zeroGradient;\n"
          << "    }\n"
          << "    walls\n"
          << "    {\n"
          << "        type            kqRWallFunction;\n"
          << "        value           uniform 0.1;\n"
          << "    }\n"
          << "    frontAndBack\n"
          << "    {\n"
          << "        type            empty;\n"
          << "    }\n"
          << "}\n\n";
    
    // Add turbulent dissipation rate field (epsilon)
    std::ofstream epsilonFile(casePath / "0" / "epsilon");
    epsilonFile << "FoamFile\n"
                << "{\n"
                << "    version     2.0;\n"
                << "    format      ascii;\n"
                << "    class       volScalarField;\n"
                << "    object      epsilon;\n"
                << "}\n\n";
    
    epsilonFile << "dimensions      [0 2 -3 0 0 0 0];\n";
    epsilonFile << "internalField   uniform 0.01;\n\n";
    epsilonFile << "boundaryField\n"
                << "{\n"
                << "    inlet\n"
                << "    {\n"
                << "        type            fixedValue;\n"
                << "        value           uniform 0.01;\n"
                << "    }\n"
                << "    outlet\n"
                << "    {\n"
                << "        type            zeroGradient;\n"
                << "    }\n"
                << "    walls\n"
                << "    {\n"
                << "        type            epsilonWallFunction;\n"
                << "        value           uniform 0.01;\n"
                << "    }\n"
                << "    frontAndBack\n"
                << "    {\n"
                << "        type            empty;\n"
                << "    }\n"
                << "}\n\n";
}

bool CaseManager::runCase(const std::string& caseId)
{
    auto it = caseResults_.find(caseId);
    if (it == caseResults_.end())
    {
        return false;
    }
    
    fs::path casePath = getCasePath(caseId);
    
    if (!fs::exists(casePath))
    {
        it->second->status = "failed";
        it->second->errorOutput = "Case directory does not exist";
        return false;
    }
    
    it->second->status = "running";
    
    auto startTime = std::chrono::steady_clock::now();
    
    std::string command = "cd " + casePath.string() + " && blockMesh > blockMesh.log 2>&1";
    int meshResult = std::system(command.c_str());
    
    if (meshResult != 0)
    {
        it->second->status = "failed";
        it->second->exitCode = meshResult;
        it->second->errorOutput = "blockMesh failed";
        return false;
    }
    
    CaseParameters params;
    command = "cd " + casePath.string() + " && " + params.solver + " > solver.log 2>&1";
    int solverResult = std::system(command.c_str());
    
    auto endTime = std::chrono::steady_clock::now();
    it->second->executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    if (solverResult == 0)
    {
        it->second->status = "completed";
        it->second->exitCode = 0;
        extractResults(caseId);
    }
    else
    {
        it->second->status = "failed";
        it->second->exitCode = solverResult;
        
        std::ifstream logFile(casePath / "solver.log");
        if (logFile.is_open())
        {
            std::stringstream buffer;
            buffer << logFile.rdbuf();
            it->second->errorOutput = buffer.str();
        }
    }
    
    return it->second->isSuccess();
}

void CaseManager::extractResults(const std::string& caseId)
{
    auto it = caseResults_.find(caseId);
    if (it == caseResults_.end())
    {
        return;
    }
    
    fs::path casePath = getCasePath(caseId);
    
    std::ifstream logFile(casePath / "solver.log");
    if (logFile.is_open())
    {
        std::stringstream buffer;
        buffer << logFile.rdbuf();
        it->second->logOutput = buffer.str();
        
        parseLogOutput(caseId, it->second->logOutput);
    }
    
    for (const auto& entry : fs::directory_iterator(casePath))
    {
        if (entry.is_directory() && entry.path().filename() != "0" && 
            entry.path().filename() != "constant" && entry.path().filename() != "system")
        {
            it->second->outputFiles.push_back(entry.path().filename());
        }
    }
}

void CaseManager::parseLogOutput(const std::string& caseId, const std::string& logOutput)
{
    auto it = caseResults_.find(caseId);
    if (it == caseResults_.end())
    {
        return;
    }
    
    std::istringstream iss(logOutput);
    std::string line;
    
    while (std::getline(iss, line))
    {
        if (line.find("ExecutionTime") != std::string::npos)
        {
            std::size_t pos = line.find("=");
            if (pos != std::string::npos)
            {
                std::string timeStr = line.substr(pos + 1);
                timeStr.erase(0, timeStr.find_first_not_of(" \t"));
                timeStr.erase(timeStr.find_last_not_of(" \ts") + 1);
                
                try
                {
                    double execTime = std::stod(timeStr);
                    it->second->results["executionTime"] = execTime;
                }
                catch (const std::exception&)
                {
                }
            }
        }
    }
}

CaseResult CaseManager::getCaseResult(const std::string& caseId) const
{
    auto it = caseResults_.find(caseId);
    if (it != caseResults_.end())
    {
        return *it->second;
    }
    
    CaseResult result;
    result.caseId = caseId;
    result.status = "not_found";
    return result;
}

std::vector<std::string> CaseManager::listCases() const
{
    std::vector<std::string> caseIds;
    
    for (const auto& [caseId, result] : caseResults_)
    {
        caseIds.push_back(caseId);
    }
    
    return caseIds;
}

bool CaseManager::deleteCaseData(const std::string& caseId)
{
    cleanup(caseId);
    
    auto it = caseResults_.find(caseId);
    if (it != caseResults_.end())
    {
        caseResults_.erase(it);
        return true;
    }
    
    return false;
}

void CaseManager::cleanup(const std::string& caseId)
{
    fs::path casePath = getCasePath(caseId);
    
    if (fs::exists(casePath))
    {
        std::error_code ec;
        fs::remove_all(casePath, ec);
    }
}

void CaseManager::setWorkingDirectory(const fs::path& workingDir)
{
    workingDirectory_ = workingDir;
    
    if (!fs::exists(workingDirectory_))
    {
        fs::create_directories(workingDirectory_);
    }
}

bool CaseManager::validateCaseParameters(const CaseParameters& params) const
{
    if (params.caseName.empty())
    {
        return false;
    }
    
    if (params.solver.empty())
    {
        return false;
    }
    
    if (params.endTime <= 0)
    {
        return false;
    }
    
    if (params.deltaTime <= 0)
    {
        return false;
    }
    
    return true;
}

std::vector<std::string> CaseManager::getAvailableSolvers() const
{
    return {"simpleFoam", "pimpleFoam", "icoFoam", "pisoFoam", "rhoPimpleFoam"};
}

json CaseManager::getCaseParametersSchema() const
{
    return json{
        {"type", "object"},
        {"properties", {
            {"caseName", {{"type", "string"}}},
            {"solver", {{"type", "string"}, {"enum", getAvailableSolvers()}}},
            {"caseType", {{"type", "string"}, {"enum", {"steady", "transient"}}}},
            {"endTime", {{"type", "number"}, {"minimum", 0}}},
            {"deltaTime", {{"type", "number"}, {"minimum", 0}}},
            {"writeInterval", {{"type", "integer"}, {"minimum", 1}}},
            {"boundaryConditions", {{"type", "object"}}},
            {"physicalProperties", {{"type", "object"}}},
            {"numericalSchemes", {{"type", "object"}}}
        }},
        {"required", {"caseName", "solver", "endTime", "deltaTime"}}
    };
}

CaseParameters CaseManager::createPipeFlowParameters(double velocity, double diameter, double length)
{
    CaseParameters params;
    params.caseName = "pipe_flow";
    params.solver = "simpleFoam";
    params.caseType = "steady";
    params.endTime = 1000;
    params.deltaTime = 1;
    params.writeInterval = 100;
    
    params.boundaryConditions["inlet"] = "fixedValue";
    params.boundaryConditions["outlet"] = "zeroGradient";
    params.boundaryConditions["walls"] = "noSlip";
    
    params.physicalProperties["nu"] = "1e-05";
    params.physicalProperties["velocity"] = std::to_string(velocity);
    params.physicalProperties["diameter"] = std::to_string(diameter);
    params.physicalProperties["length"] = std::to_string(length);
    
    return params;
}

CaseParameters CaseManager::createCavityFlowParameters(double velocity, double viscosity)
{
    CaseParameters params;
    params.caseName = "cavity_flow";
    params.solver = "icoFoam";
    params.caseType = "transient";
    params.endTime = 0.5;
    params.deltaTime = 0.005;
    params.writeInterval = 20;
    
    params.boundaryConditions["movingWall"] = "fixedValue";
    params.boundaryConditions["fixedWalls"] = "noSlip";
    
    params.physicalProperties["nu"] = std::to_string(viscosity);
    params.physicalProperties["velocity"] = std::to_string(velocity);
    
    return params;
}

} // End namespace MCP
} // End namespace Foam

// ************************************************************************* //