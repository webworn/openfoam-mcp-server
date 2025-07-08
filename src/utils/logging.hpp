/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Logging Utilities
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Simple logging utilities for MCP server

\*---------------------------------------------------------------------------*/

#ifndef logging_H
    #define logging_H

    #include <chrono>
    #include <iomanip>
    #include <iostream>
    #include <sstream>
    #include <string>

namespace Foam
{
namespace MCP
{

/*---------------------------------------------------------------------------*\
                        Enum LogLevel
\*---------------------------------------------------------------------------*/

enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

/*---------------------------------------------------------------------------*\
                        Class Logger Declaration
\*---------------------------------------------------------------------------*/

class Logger
{
  private:
    static LogLevel currentLevel_;

    static std::string getCurrentTimestamp();
    static std::string levelToString(LogLevel level);

  public:
    static void setLogLevel(LogLevel level)
    {
        currentLevel_ = level;
    }
    static LogLevel getLogLevel()
    {
        return currentLevel_;
    }

    static void log(LogLevel level, const std::string& message);

    static void debug(const std::string& message)
    {
        log(LogLevel::DEBUG, message);
    }
    static void info(const std::string& message)
    {
        log(LogLevel::INFO, message);
    }
    static void warning(const std::string& message)
    {
        log(LogLevel::WARNING, message);
    }
    static void error(const std::string& message)
    {
        log(LogLevel::ERROR, message);
    }
};

/*---------------------------------------------------------------------------*\
                        Convenience Macros
\*---------------------------------------------------------------------------*/

    #define LOG_DEBUG(msg) Foam::MCP::Logger::debug(msg)
    #define LOG_INFO(msg) Foam::MCP::Logger::info(msg)
    #define LOG_WARNING(msg) Foam::MCP::Logger::warning(msg)
    #define LOG_ERROR(msg) Foam::MCP::Logger::error(msg)

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //