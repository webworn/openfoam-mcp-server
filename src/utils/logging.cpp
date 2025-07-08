/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Logging Utilities
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Simple logging utilities implementation

\*---------------------------------------------------------------------------*/

#include "logging.hpp"

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                        Logger Implementation
\*---------------------------------------------------------------------------*/

LogLevel Logger::currentLevel_ = LogLevel::INFO;

std::string Logger::getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Logger::levelToString(LogLevel level)
{
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message)
{
    if (level < currentLevel_) {
        return;
    }

    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = levelToString(level);

    std::cerr << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //