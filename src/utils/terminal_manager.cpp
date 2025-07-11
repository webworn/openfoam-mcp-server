/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Terminal Management System Implementation
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Implementation of dual-terminal management system

\*---------------------------------------------------------------------------*/

#include "terminal_manager.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <csignal>
#include <thread>
#include <unistd.h>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        TerminalManager Implementation
\*---------------------------------------------------------------------------*/

TerminalManager::TerminalManager() {
    // Initialize with current working directory
    char* cwd = getcwd(nullptr, 0);
    if (cwd) {
        currentWorkingDirectory_ = std::string(cwd);
        free(cwd);
    }
    
    // Validate OpenFOAM environment on startup
    if (!validateOpenFOAMEnvironment()) {
        reportToClaudeTerminal("⚠️  OpenFOAM environment not properly configured");
    } else {
        reportToClaudeTerminal("✅ OpenFOAM environment validated successfully");
    }
}

TerminalManager::~TerminalManager() {
    // Stop any running processes
    if (currentProcessPid_.load() > 0) {
        killCurrentProcess();
    }
    
    // Wait for monitor thread to finish
    monitoring_.store(false);
    if (monitorThread_ && monitorThread_->joinable()) {
        monitorThread_->join();
    }
}

CommandResult TerminalManager::executeInCodeTerminal(
    const std::string& command,
    const ExecutionContext& context
) {
    std::lock_guard<std::mutex> lock(executionMutex_);
    
    if (codeTerminalBusy_.load()) {
        CommandResult result;
        result.success = false;
        result.stderr = "Code terminal is busy with another operation";
        return result;
    }
    
    codeTerminalBusy_.store(true);
    
    reportToClaudeTerminal("🚀 Executing in Code Terminal: " + command);
    
    auto result = executeCommand(command, context);
    
    // Store in history
    {
        std::lock_guard<std::mutex> historyLock(historyMutex_);
        commandHistory_.push_back(result);
        
        // Keep only last 100 commands
        if (commandHistory_.size() > 100) {
            commandHistory_.erase(commandHistory_.begin());
        }
    }
    
    // Analyze errors if command failed
    if (!result.success) {
        result.errorAnalysis = analyzeOpenFOAMError(result);
        reportToClaudeTerminal("❌ Command failed: " + result.errorAnalysis);
    } else {
        reportToClaudeTerminal("✅ Command completed successfully in " + 
                              std::to_string(result.duration.count()) + "ms");
    }
    
    codeTerminalBusy_.store(false);
    return result;
}

void TerminalManager::reportToClaudeTerminal(const std::string& message) {
    // Format message for Claude terminal
    std::string formatted = formatForClaudeTerminal(message);
    
    // Call status callback if set
    if (statusCallback_) {
        statusCallback_(formatted);
    }
    
    // Also output to stderr for immediate visibility
    std::cerr << "Claude Terminal: " << formatted << std::endl;
}

CommandResult TerminalManager::executeCommand(
    const std::string& command,
    const ExecutionContext& context
) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Prepare working directory
    std::string workDir = context.workingDirectory.empty() ? 
                         currentWorkingDirectory_ : context.workingDirectory;
    
    // Build full command with directory change
    std::ostringstream fullCommand;
    fullCommand << "cd \"" << workDir << "\" && " << command;
    
    // Execute command
    FILE* pipe = popen(fullCommand.str().c_str(), "r");
    if (!pipe) {
        CommandResult result;
        result.success = false;
        result.stderr = "Failed to create pipe for command execution";
        return result;
    }
    
    // Read output
    std::string output;
    std::string line;
    char buffer[256];
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        line = std::string(buffer);
        output += line;
        
        // Parse OpenFOAM specific output
        parseOpenFOAMOutput(line);
        
        // Call progress callback if set
        if (progressCallback_) {
            progressCallback_(line);
        }
    }
    
    int exitCode = pclose(pipe);
    auto endTime = std::chrono::high_resolution_clock::now();
    
    CommandResult result;
    result.exitCode = exitCode;
    result.success = (exitCode == 0);
    result.stdout = output;
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    return result;
}

void TerminalManager::parseOpenFOAMOutput(const std::string& line) {
    // Parse common OpenFOAM output patterns
    
    // Check for solver progress
    std::regex timeRegex(R"(Time = ([0-9.e+-]+))");
    std::smatch timeMatch;
    if (std::regex_search(line, timeMatch, timeRegex)) {
        if (statusCallback_) {
            statusCallback_("⏱️  Simulation time: " + timeMatch[1].str());
        }
    }
    
    // Check for residuals
    std::regex residualRegex(R"(([A-Za-z]+):\s*Solving for [A-Za-z]+, Initial residual = ([0-9.e+-]+))");
    std::smatch residualMatch;
    if (std::regex_search(line, residualMatch, residualRegex)) {
        if (statusCallback_) {
            statusCallback_("📊 " + residualMatch[1].str() + " residual: " + residualMatch[2].str());
        }
    }
    
    // Check for convergence
    if (line.find("SIMPLE solution converged") != std::string::npos ||
        line.find("PIMPLE: converged") != std::string::npos) {
        if (statusCallback_) {
            statusCallback_("🎯 Solution converged!");
        }
    }
    
    // Check for warnings/errors
    if (line.find("WARNING") != std::string::npos) {
        if (statusCallback_) {
            statusCallback_("⚠️  Warning detected: " + line);
        }
    }
    
    if (line.find("ERROR") != std::string::npos || line.find("FATAL") != std::string::npos) {
        if (statusCallback_) {
            statusCallback_("🚨 Error detected: " + line);
        }
    }
}

std::string TerminalManager::analyzeOpenFOAMError(const CommandResult& result) {
    std::ostringstream analysis;
    
    // Common OpenFOAM error patterns
    if (result.stderr.find("Cannot find file") != std::string::npos) {
        analysis << "Missing file error. Check if all required files exist in the case directory.";
    }
    else if (result.stderr.find("Bad mesh") != std::string::npos) {
        analysis << "Mesh quality issue. Run checkMesh to identify problems.";
    }
    else if (result.stderr.find("Time step continuity errors") != std::string::npos) {
        analysis << "Convergence issue. Consider reducing time step or improving mesh quality.";
    }
    else if (result.stderr.find("Maximum number of iterations exceeded") != std::string::npos) {
        analysis << "Solver didn't converge. Consider relaxing solution parameters.";
    }
    else if (result.stderr.find("Floating point exception") != std::string::npos) {
        analysis << "Numerical instability. Check boundary conditions and initial values.";
    }
    else {
        analysis << "Unknown error. Exit code: " << result.exitCode;
    }
    
    return analysis.str();
}

bool TerminalManager::validateOpenFOAMEnvironment() {
    // Check for essential OpenFOAM environment variables
    const char* foamSrc = std::getenv("FOAM_SRC");
    const char* foamApp = std::getenv("FOAM_APP");
    
    if (!foamSrc || !foamApp) {
        return false;
    }
    
    // Try to run a simple OpenFOAM command
    CommandResult result = executeCommand("which blockMesh", ExecutionContext());
    return result.success;
}

CommandResult TerminalManager::runOpenFOAMSolver(
    const std::string& solver,
    const std::string& caseDir,
    const ExecutionContext& context
) {
    ExecutionContext solverContext = context;
    solverContext.workingDirectory = caseDir;
    solverContext.timeout = std::chrono::seconds(3600);  // 1 hour timeout for solvers
    
    return executeInCodeTerminal(solver, solverContext);
}

void TerminalManager::setWorkingDirectory(const std::string& path) {
    if (std::filesystem::exists(path)) {
        currentWorkingDirectory_ = std::filesystem::absolute(path);
        reportToClaudeTerminal("📁 Working directory set to: " + currentWorkingDirectory_);
    } else {
        reportToClaudeTerminal("❌ Directory does not exist: " + path);
    }
}

std::string TerminalManager::formatForCodeTerminal(const std::string& content) {
    // Simple formatting for code terminal
    return content;
}

std::string TerminalManager::formatForClaudeTerminal(const std::string& content) {
    // Add timestamp and formatting for Claude terminal
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::ostringstream formatted;
    formatted << "[" << std::put_time(&tm, "%H:%M:%S") << "] " << content;
    return formatted.str();
}

bool TerminalManager::fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::vector<std::string> TerminalManager::listFiles(const std::string& directory) {
    std::vector<std::string> files;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            files.push_back(entry.path().filename().string());
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        reportToClaudeTerminal("❌ Error listing directory: " + std::string(ex.what()));
    }
    
    return files;
}

/*---------------------------------------------------------------------------*\
                        OpenFOAM Command Helpers Implementation
\*---------------------------------------------------------------------------*/

namespace OpenFOAMCommands {

std::string blockMesh(const std::string& caseDir) {
    return "blockMesh -case " + caseDir;
}

std::string checkMesh(const std::string& caseDir) {
    return "checkMesh -case " + caseDir;
}

std::string runSolver(const std::string& solver, const std::string& caseDir) {
    return solver + " -case " + caseDir;
}

std::string foamToVTK(const std::string& caseDir) {
    return "foamToVTK -case " + caseDir;
}

std::string decomposePar(const std::string& caseDir, int processors) {
    return "decomposePar -case " + caseDir + " -cellDist";
}

std::string reconstructPar(const std::string& caseDir) {
    return "reconstructPar -case " + caseDir;
}

std::string runParallel(const std::string& solver, const std::string& caseDir, int processors) {
    return "mpirun -np " + std::to_string(processors) + " " + solver + " -case " + caseDir + " -parallel";
}

}  // End namespace OpenFOAMCommands

/*---------------------------------------------------------------------------*\
                        Progress Parsing Implementation
\*---------------------------------------------------------------------------*/

namespace ProgressParser {

SolverProgress parseOpenFOAMLog(const std::string& logContent) {
    SolverProgress progress;
    
    // Parse time
    std::regex timeRegex(R"(Time = ([0-9.e+-]+))");
    std::sregex_iterator timeIter(logContent.begin(), logContent.end(), timeRegex);
    std::sregex_iterator end;
    
    if (timeIter != end) {
        progress.time = std::stod((*timeIter)[1].str());
    }
    
    // Parse residuals (simplified)
    std::regex residualRegex(R"(Initial residual = ([0-9.e+-]+))");
    std::sregex_iterator residualIter(logContent.begin(), logContent.end(), residualRegex);
    
    if (residualIter != end) {
        progress.residuals["overall"] = std::stod((*residualIter)[1].str());
    }
    
    return progress;
}

std::string generateProgressReport(const SolverProgress& progress) {
    std::ostringstream report;
    report << "Simulation Progress:\n";
    report << "  Time: " << progress.time << "s\n";
    report << "  Iteration: " << progress.iteration << "\n";
    
    if (!progress.residuals.empty()) {
        report << "  Residuals:\n";
        for (const auto& [field, residual] : progress.residuals) {
            report << "    " << field << ": " << residual << "\n";
        }
    }
    
    return report.str();
}

}  // End namespace ProgressParser

/*---------------------------------------------------------------------------*\
                     Missing TerminalManager Methods
\*---------------------------------------------------------------------------*/

bool TerminalManager::killCurrentProcess() {
    int pid = currentProcessPid_.load();
    if (pid > 0) {
        if (kill(pid, SIGTERM) == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (kill(pid, 0) == 0) {
                // Process still running, force kill
                kill(pid, SIGKILL);
            }
            currentProcessPid_ = -1;
            return true;
        }
    }
    return false;
}

CommandResult TerminalManager::createDirectory(const std::string& path) {
    CommandResult result;
    auto startTime = std::chrono::steady_clock::now();
    
    try {
        // Use std::filesystem to create directory
        std::filesystem::create_directories(path);
        result.exitCode = 0;
        result.stdout = "Directory created: " + path;
        result.success = true;
    } catch (const std::exception& e) {
        result.exitCode = 1;
        result.stderr = "Failed to create directory: " + std::string(e.what());
        result.success = false;
    }
    
    auto endTime = std::chrono::steady_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    return result;
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //