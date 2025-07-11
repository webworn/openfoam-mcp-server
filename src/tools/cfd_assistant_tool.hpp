/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | CFD Assistant Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Intelligent CFD assistant that uses Socratic questioning to understand
    user requirements and guide them through OpenFOAM case creation and
    analysis with dual-terminal execution.

\*---------------------------------------------------------------------------*/

#ifndef cfd_assistant_tool_H
#define cfd_assistant_tool_H

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "../mcp/server.hpp"
#include "../utils/terminal_manager.hpp"
#include "context_engine.hpp"

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Struct CFDProblemContext
\*---------------------------------------------------------------------------*/

struct CFDProblemContext {
    // Problem definition
    std::string problemDescription;
    std::string physicsType;  // "incompressible", "compressible", "multiphase", "heat_transfer"
    std::string geometryType; // "internal", "external", "mixing"
    
    // Physical parameters
    double characteristicLength = 1.0;    // [m]
    double characteristicVelocity = 1.0;  // [m/s]
    double reynoldsNumber = 1000.0;
    double machNumber = 0.1;
    
    // Fluid properties
    std::string fluidType = "air";        // "air", "water", "custom"
    double density = 1.225;               // [kg/m³]
    double viscosity = 1.8e-5;            // [Pa·s]
    double temperature = 300.0;           // [K]
    
    // Simulation goals
    std::vector<std::string> objectives;   // "drag", "lift", "pressure_drop", "heat_transfer"
    std::string accuracyLevel = "moderate"; // "quick", "moderate", "high", "research"
    
    // Computational resources
    int availableCores = 4;
    std::string timeConstraint = "moderate"; // "quick", "moderate", "extended"
    
    // User experience level
    std::string userLevel = "beginner";    // "beginner", "intermediate", "expert"
    
    // Case directory and files
    std::string caseDirectory;
    std::string caseName;
    
    bool isComplete() const;
    std::vector<std::string> getMissingInformation() const;
};

/*---------------------------------------------------------------------------*\
                        Struct ConversationState
\*---------------------------------------------------------------------------*/

struct ConversationState {
    enum Phase {
        INITIAL_INQUIRY,
        PROBLEM_DEFINITION,
        PHYSICS_EXPLORATION,
        GEOMETRY_DEFINITION,
        PARAMETER_REFINEMENT,
        CASE_GENERATION,
        EXECUTION_MONITORING,
        RESULT_ANALYSIS,
        COMPLETED
    };
    
    Phase currentPhase = INITIAL_INQUIRY;
    std::vector<std::string> questionsAsked;
    std::vector<std::string> userResponses;
    CFDProblemContext problemContext;
    
    // Tracking learning journey
    std::vector<std::string> conceptsExplained;
    std::vector<std::string> learningObjectives;
};

/*---------------------------------------------------------------------------*\
                        Class CFDAssistantTool Declaration
\*---------------------------------------------------------------------------*/

class CFDAssistantTool {
private:
    std::unique_ptr<TerminalManager> terminalManager_;
    std::unique_ptr<ContextEngine> contextEngine_;
    ConversationState conversationState_;
    
    // Socratic questioning system
    std::vector<std::string> generateSocraticQuestions(const ConversationState& state);
    std::string selectBestQuestion(const std::vector<std::string>& questions);
    
    // Problem analysis
    std::string analyzeUserResponse(const std::string& response);
    void updateProblemContext(const std::string& response);
    
    // Educational guidance
    std::string explainCFDConcept(const std::string& conceptName);
    std::string generateLearningPath(const CFDProblemContext& context);
    
    // Case generation
    bool generateOpenFOAMCase(const CFDProblemContext& context);
    std::string createGeometry(const CFDProblemContext& context);
    std::string createMesh(const CFDProblemContext& context);
    std::string setupBoundaryConditions(const CFDProblemContext& context);
    std::string selectSolver(const CFDProblemContext& context);
    std::string configureSolution(const CFDProblemContext& context);
    
    // Execution guidance
    std::string guideExecution(const CFDProblemContext& context);
    std::string monitorProgress();
    std::string diagnoseIssues(const CommandResult& result);
    
    // Results interpretation
    std::string interpretResults(const CFDProblemContext& context);
    std::string suggestVisualization(const CFDProblemContext& context);
    std::string generateInsights(const CFDProblemContext& context);
    
    // Error handling with research
    std::string perform5WhysAnalysis(const std::string& error);
    std::string searchForSolutions(const std::string& error);
    std::string findRelevantResearch(const std::string& problem);
    
public:
    CFDAssistantTool();
    ~CFDAssistantTool() = default;
    
    // Main interaction method
    ToolResult processUserInput(const std::string& input);
    
    // Context engine integration
    std::string processWithContextEngine(const std::string& input);
    bool isReadyForCaseGeneration() const;
    std::vector<std::string> getExtractedParameters() const;
    std::string detectUserExperienceLevel(const std::string& input);
    
    // Specific assistant functions
    ToolResult startCFDAssistance(const json& arguments);
    
    // Missing method declarations
    std::string handleInitialInquiry(const std::string& input);
    std::string handleProblemDefinition(const std::string& input);
    std::string handlePhysicsExploration(const std::string& input);
    std::string handleGeometryDefinition(const std::string& input);
    std::string handleCaseGeneration();
    std::string handleParameterRefinement(const std::string& input);
    std::string handleExecutionMonitoring();
    std::string handleResultAnalysis();
    ToolResult continueConversation(const json& arguments);
    ToolResult executeOpenFOAMOperation(const json& arguments);
    ToolResult analyzeResults(const json& arguments);
    ToolResult provideLearningGuidance(const json& arguments);
    
    // Utility methods
    std::string getCurrentPhaseDescription() const;
    std::string getProgressSummary() const;
    std::vector<std::string> getSuggestedNextSteps() const;
};

/*---------------------------------------------------------------------------*\
                        Socratic Question Database
\*---------------------------------------------------------------------------*/

namespace SocraticQuestions {
    // Problem exploration questions
    const std::vector<std::string> PROBLEM_EXPLORATION = {
        "What's the driving force behind the flow in your system?",
        "Are we looking at flow inside something or around something?",
        "What makes this flow problem interesting or challenging?",
        "What would happen if we changed the size by 10x?",
        "How does this relate to real-world applications you've seen?"
    };
    
    // Physics understanding questions
    const std::vector<std::string> PHYSICS_UNDERSTANDING = {
        "Is the fluid compressible or incompressible in your case?",
        "Do you expect turbulence to play a major role?",
        "Are there any heat transfer effects we should consider?",
        "What about gravity - is buoyancy important here?",
        "How fast is the flow compared to the speed of sound?"
    };
    
    // Geometry insight questions
    const std::vector<std::string> GEOMETRY_INSIGHTS = {
        "What's the most important geometric feature for your analysis?",
        "Where do you think the flow will be most complex?",
        "How would you describe the shape to someone who can't see it?",
        "What symmetries can we exploit to simplify the problem?",
        "Which dimensions are most critical for your objectives?"
    };
    
    // Solution strategy questions
    const std::vector<std::string> SOLUTION_STRATEGY = {
        "What level of accuracy do you need for this analysis?",
        "How much time do you have for this simulation?",
        "What would you do if the first attempt doesn't converge?",
        "How will you know if the results are reasonable?",
        "What's the main insight you're hoping to gain?"
    };
}

/*---------------------------------------------------------------------------*\
                        CFD Concept Explanations
\*---------------------------------------------------------------------------*/

namespace CFDConcepts {
    std::string explainReynoldsNumber();
    std::string explainTurbulence();
    std::string explainBoundaryConditions();
    std::string explainMeshQuality();
    std::string explainConvergence();
    std::string explainDimensionalAnalysis();
    std::string explainNonDimensionalNumbers();
}

/*---------------------------------------------------------------------------*\
                        Learning Path Generator
\*---------------------------------------------------------------------------*/

namespace LearningPaths {
    std::vector<std::string> generateBeginnerPath(const CFDProblemContext& context);
    std::vector<std::string> generateIntermediatePath(const CFDProblemContext& context);
    std::vector<std::string> generateExpertPath(const CFDProblemContext& context);
}

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //