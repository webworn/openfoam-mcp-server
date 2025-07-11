/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Terminal Management System
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Dual-terminal management system for OpenFOAM execution
    
    Manages two terminal contexts:
    - Code Terminal: Where OpenFOAM commands execute
    - Claude Terminal: Where user interacts with assistant
    
    Provides real-time status updates and intelligent error handling.

\*---------------------------------------------------------------------------*/

#ifndef terminal_manager_H
#define terminal_manager_H

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Enum TerminalType
\*---------------------------------------------------------------------------*/

enum class TerminalType {
    CODE_TERMINAL,    // Where OpenFOAM commands execute
    CLAUDE_TERMINAL   // Where user interacts with assistant
};

/*---------------------------------------------------------------------------*\
                        Struct CommandResult
\*---------------------------------------------------------------------------*/

struct CommandResult {
    int exitCode = 0;
    std::string stdout;
    std::string stderr;
    std::chrono::milliseconds duration{0};
    bool success = false;
    std::string errorAnalysis;  // AI-generated error analysis
    
    CommandResult() = default;
    CommandResult(int code, const std::string& out, const std::string& err)
        : exitCode(code), stdout(out), stderr(err), success(code == 0) {}
};

/*---------------------------------------------------------------------------*\
                        Struct ExecutionContext
\*---------------------------------------------------------------------------*/

struct ExecutionContext {
    std::string workingDirectory;
    std::map<std::string, std::string> environment;
    std::chrono::seconds timeout{300};  // 5 minute default
    bool captureOutput = true;
    bool showProgress = true;
    
    ExecutionContext() = default;
    ExecutionContext(const std::string& workDir) : workingDirectory(workDir) {}
};

/*---------------------------------------------------------------------------*\
                        Class TerminalManager Declaration
\*---------------------------------------------------------------------------*/

class TerminalManager {
public:
    using ProgressCallback = std::function<void(const std::string& line)>;
    using StatusCallback = std::function<void(const std::string& status)>;

private:
    std::atomic<bool> codeTerminalBusy_{false};
    std::atomic<bool> monitoring_{false};
    
    std::mutex executionMutex_;
    std::string currentWorkingDirectory_;
    std::map<std::string, std::string> environment_;
    
    // Callbacks for real-time updates
    ProgressCallback progressCallback_;
    StatusCallback statusCallback_;
    
    // Process monitoring
    std::unique_ptr<std::thread> monitorThread_;
    std::atomic<int> currentProcessPid_{-1};
    
    // Command history and analysis
    std::vector<CommandResult> commandHistory_;
    std::mutex historyMutex_;
    
    // Internal methods
    CommandResult executeCommand(
        const std::string& command,
        const ExecutionContext& context
    );
    
    void monitorProcess(int pid, const std::string& command);
    void parseOpenFOAMOutput(const std::string& line);
    std::string analyzeOpenFOAMError(const CommandResult& result);
    
public:
    TerminalManager();
    ~TerminalManager();
    
    // Core execution methods
    CommandResult executeInCodeTerminal(
        const std::string& command,
        const ExecutionContext& context = ExecutionContext()
    );
    
    void reportToClaudeTerminal(const std::string& message);
    
    // Status and monitoring
    bool isCodeTerminalBusy() const { return codeTerminalBusy_.load(); }
    bool isMonitoring() const { return monitoring_.load(); }
    
    // Process management
    bool killCurrentProcess();
    void waitForCompletion();
    
    // Callback registration
    void setProgressCallback(ProgressCallback callback) { 
        progressCallback_ = std::move(callback); 
    }
    
    void setStatusCallback(StatusCallback callback) { 
        statusCallback_ = std::move(callback); 
    }
    
    // Environment management
    void setWorkingDirectory(const std::string& path);
    void setEnvironmentVariable(const std::string& name, const std::string& value);
    std::string getWorkingDirectory() const { return currentWorkingDirectory_; }
    
    // OpenFOAM specific methods
    bool validateOpenFOAMEnvironment();
    CommandResult checkOpenFOAMCase(const std::string& caseDir);
    CommandResult runOpenFOAMSolver(
        const std::string& solver,
        const std::string& caseDir,
        const ExecutionContext& context = ExecutionContext()
    );
    
    // Utility methods
    CommandResult createDirectory(const std::string& path);
    CommandResult copyFile(const std::string& source, const std::string& dest);
    bool fileExists(const std::string& path);
    std::vector<std::string> listFiles(const std::string& directory);
    
    // Error analysis and guidance
    std::string getLastError() const;
    std::vector<std::string> suggestSolutions(const CommandResult& result);
    std::string generateUserGuidance(const std::string& operation);
    
    // Command history
    std::vector<CommandResult> getCommandHistory() const;
    void clearHistory();
    
    // Terminal output formatting
    std::string formatForCodeTerminal(const std::string& content);
    std::string formatForClaudeTerminal(const std::string& content);
};

/*---------------------------------------------------------------------------*\
                        OpenFOAM Command Helpers
\*---------------------------------------------------------------------------*/

namespace OpenFOAMCommands {
    std::string blockMesh(const std::string& caseDir);
    std::string checkMesh(const std::string& caseDir);
    std::string decomposePar(const std::string& caseDir, int processors = 4);
    std::string reconstructPar(const std::string& caseDir);
    std::string runSolver(const std::string& solver, const std::string& caseDir);
    std::string runParallel(const std::string& solver, const std::string& caseDir, int processors = 4);
    std::string postProcess(const std::string& function, const std::string& caseDir);
    std::string foamToVTK(const std::string& caseDir);
}

/*---------------------------------------------------------------------------*\
                        Progress Parsing Utilities
\*---------------------------------------------------------------------------*/

namespace ProgressParser {
    struct SolverProgress {
        int iteration = 0;
        double time = 0.0;
        std::map<std::string, double> residuals;
        double courantNumber = 0.0;
        bool converged = false;
        std::string status;
    };
    
    SolverProgress parseOpenFOAMLog(const std::string& logContent);
    double estimateProgress(const SolverProgress& progress, double endTime);
    std::string generateProgressReport(const SolverProgress& progress);
}

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //