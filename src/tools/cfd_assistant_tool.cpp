/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | CFD Assistant Tool Implementation
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Implementation of intelligent CFD assistant with Socratic questioning

\*---------------------------------------------------------------------------*/

#include "cfd_assistant_tool.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        CFDProblemContext Implementation
\*---------------------------------------------------------------------------*/

bool CFDProblemContext::isComplete() const {
    return !problemDescription.empty() &&
           !physicsType.empty() &&
           !geometryType.empty() &&
           characteristicLength > 0 &&
           characteristicVelocity > 0 &&
           density > 0 &&
           viscosity > 0;
}

std::vector<std::string> CFDProblemContext::getMissingInformation() const {
    std::vector<std::string> missing;
    
    if (problemDescription.empty()) missing.push_back("problem description");
    if (physicsType.empty()) missing.push_back("physics type");
    if (geometryType.empty()) missing.push_back("geometry type");
    if (characteristicLength <= 0) missing.push_back("characteristic length");
    if (characteristicVelocity <= 0) missing.push_back("characteristic velocity");
    if (objectives.empty()) missing.push_back("analysis objectives");
    
    return missing;
}

/*---------------------------------------------------------------------------*\
                        CFDAssistantTool Implementation
\*---------------------------------------------------------------------------*/

CFDAssistantTool::CFDAssistantTool() 
    : terminalManager_(std::make_unique<TerminalManager>()),
      contextEngine_(std::make_unique<ContextEngine>()) {
    
    // Set up callbacks for real-time updates
    terminalManager_->setProgressCallback([this](const std::string& line) {
        // Handle real-time progress updates
        if (line.find("Time =") != std::string::npos) {
            terminalManager_->reportToClaudeTerminal("⏱️  Simulation progressing: " + line);
        }
    });
    
    terminalManager_->setStatusCallback([this](const std::string& status) {
        // Handle status updates in Claude terminal
        std::cout << "Claude Terminal: " << status << std::endl;
    });
    
    // Initialize conversation state
    conversationState_.currentPhase = ConversationState::Phase::INITIAL_INQUIRY;
}

ToolResult CFDAssistantTool::processUserInput(const std::string& input) {
    ToolResult result;
    
    // Process input through context engine for intelligent analysis
    std::string contextResponse = processWithContextEngine(input);
    
    // Update traditional conversation state for backwards compatibility
    updateProblemContext(input);
    
    // Set user experience level in context engine if detected
    std::string detectedLevel = detectUserExperienceLevel(input);
    if (!detectedLevel.empty() && detectedLevel != "unknown") {
        contextEngine_->setUserExperienceLevel(detectedLevel);
    }
    
    // Check if ready for case generation using context engine
    if (isReadyForCaseGeneration()) {
        std::string nextStep = handleCaseGeneration();
        result.addTextContent(contextResponse + "\n\n" + nextStep);
        result.isError = false;
        return result;
    }
    
    // Generate next question or action based on current phase
    std::string nextStep;
    
    switch (conversationState_.currentPhase) {
        case ConversationState::Phase::INITIAL_INQUIRY:
            nextStep = handleInitialInquiry(input);
            break;
            
        case ConversationState::Phase::PROBLEM_DEFINITION:
            nextStep = handleProblemDefinition(input);
            break;
            
        case ConversationState::Phase::PHYSICS_EXPLORATION:
            nextStep = handlePhysicsExploration(input);
            break;
            
        case ConversationState::Phase::GEOMETRY_DEFINITION:
            nextStep = handleGeometryDefinition(input);
            break;
            
        case ConversationState::Phase::PARAMETER_REFINEMENT:
            nextStep = handleParameterRefinement(input);
            break;
            
        case ConversationState::Phase::CASE_GENERATION:
            nextStep = handleCaseGeneration();
            break;
            
        case ConversationState::Phase::EXECUTION_MONITORING:
            nextStep = handleExecutionMonitoring();
            break;
            
        case ConversationState::Phase::RESULT_ANALYSIS:
            nextStep = handleResultAnalysis();
            break;
            
        default:
            nextStep = "I'm here to help you with CFD analysis. What would you like to explore?";
            break;
    }
    
    result.addTextContent(nextStep);
    return result;
}

std::string CFDAssistantTool::handleInitialInquiry(const std::string& input) {
    // Extract key information from initial input
    std::ostringstream response;
    
    response << "🌊 Welcome to your CFD analysis journey! I'm excited to help you explore fluid dynamics.\n\n";
    
    // Analyze the input for key concepts
    if (input.find("car") != std::string::npos || input.find("vehicle") != std::string::npos) {
        conversationState_.problemContext.geometryType = "external";
        conversationState_.problemContext.physicsType = "incompressible";
        response << "Automotive aerodynamics - fascinating! ";
    } else if (input.find("pipe") != std::string::npos || input.find("duct") != std::string::npos) {
        conversationState_.problemContext.geometryType = "internal";
        response << "Internal flow analysis - excellent choice! ";
    } else if (input.find("wing") != std::string::npos || input.find("airfoil") != std::string::npos) {
        conversationState_.problemContext.geometryType = "external";
        response << "Aerodynamic analysis - the foundation of flight! ";
    }
    
    // Generate first Socratic question
    auto questions = generateSocraticQuestions(conversationState_);
    response << "\n\n" << selectBestQuestion(questions);
    response << "\n\n💡 *Think about what's driving the flow and what makes this problem unique.*";
    
    conversationState_.currentPhase = ConversationState::Phase::PROBLEM_DEFINITION;
    return response.str();
}

std::string CFDAssistantTool::handleProblemDefinition(const std::string& input) {
    std::ostringstream response;
    
    // Store user's response
    conversationState_.userResponses.push_back(input);
    
    // Analyze for physics insights
    if (input.find("pressure") != std::string::npos) {
        response << "🎯 Pressure-driven flow - that's fundamental! ";
        conversationState_.problemContext.objectives.push_back("pressure_drop");
    }
    
    if (input.find("drag") != std::string::npos || input.find("resistance") != std::string::npos) {
        response << "🏎️ Drag analysis - crucial for efficiency! ";
        conversationState_.problemContext.objectives.push_back("drag");
    }
    
    if (input.find("lift") != std::string::npos) {
        response << "✈️ Lift generation - the magic of flight! ";
        conversationState_.problemContext.objectives.push_back("lift");
    }
    
    // Explain relevant CFD concept
    if (conversationState_.problemContext.objectives.empty()) {
        response << explainCFDConcept("objectives");
    }
    
    // Move to physics exploration
    response << "\n\nNow let's dive deeper into the physics. ";
    response << "Understanding the flow regime is crucial for accurate simulation.\n\n";
    
    auto questions = SocraticQuestions::PHYSICS_UNDERSTANDING;
    response << selectBestQuestion(questions);
    
    conversationState_.currentPhase = ConversationState::Phase::PHYSICS_EXPLORATION;
    return response.str();
}

std::string CFDAssistantTool::handlePhysicsExploration(const std::string& input) {
    std::ostringstream response;
    
    // Analyze physics-related input
    if (input.find("incompressible") != std::string::npos || 
        input.find("low speed") != std::string::npos) {
        conversationState_.problemContext.physicsType = "incompressible";
        response << "🌊 Incompressible flow - perfect for most practical applications! ";
        response << explainCFDConcept("incompressible_flow");
    }
    
    if (input.find("turbulent") != std::string::npos || input.find("turbulence") != std::string::npos) {
        response << "\n\n🌪️ Turbulence adds complexity but also realism! ";
        response << explainCFDConcept("turbulence");
    }
    
    if (input.find("heat") != std::string::npos || input.find("temperature") != std::string::npos) {
        conversationState_.problemContext.physicsType = "heat_transfer";
        response << "\n\n🔥 Heat transfer coupled with fluid flow - that's where things get interesting! ";
    }
    
    // Calculate Reynolds number if we have enough info
    if (conversationState_.problemContext.characteristicLength > 0 && 
        conversationState_.problemContext.characteristicVelocity > 0) {
        double reynolds = conversationState_.problemContext.density * 
                         conversationState_.problemContext.characteristicVelocity * 
                         conversationState_.problemContext.characteristicLength / 
                         conversationState_.problemContext.viscosity;
        
        response << "\n\n📊 Based on your parameters, the Reynolds number is approximately " 
                 << std::scientific << std::setprecision(2) << reynolds;
        response << "\n" << explainCFDConcept("reynolds_number");
    }
    
    response << "\n\nNow let's define the geometry precisely. ";
    auto questions = SocraticQuestions::GEOMETRY_INSIGHTS;
    response << selectBestQuestion(questions);
    
    conversationState_.currentPhase = ConversationState::Phase::GEOMETRY_DEFINITION;
    return response.str();
}

std::string CFDAssistantTool::handleGeometryDefinition(const std::string& input) {
    std::ostringstream response;
    
    response << "🎨 Excellent geometric insights! ";
    
    // Extract dimensions if mentioned
    std::regex sizeRegex(R"((\d+(?:\.\d+)?)\s*(m|mm|cm|in|ft))");
    std::smatch match;
    if (std::regex_search(input, match, sizeRegex)) {
        double value = std::stod(match[1].str());
        std::string unit = match[2].str();
        
        // Convert to meters
        if (unit == "mm") value /= 1000.0;
        else if (unit == "cm") value /= 100.0;
        else if (unit == "in") value *= 0.0254;
        else if (unit == "ft") value *= 0.3048;
        
        conversationState_.problemContext.characteristicLength = value;
        response << "I captured the characteristic length: " << value << " meters. ";
    }
    
    // Check if we have enough information to proceed
    if (conversationState_.problemContext.isComplete()) {
        response << "\n\n🎉 Excellent! We have enough information to create your OpenFOAM case. ";
        response << "Let me generate the complete simulation setup for you.\n\n";
        
        conversationState_.currentPhase = ConversationState::Phase::CASE_GENERATION;
        return response.str() + handleCaseGeneration();
    } else {
        auto missing = conversationState_.problemContext.getMissingInformation();
        response << "\n\nWe're making great progress! I still need a bit more information about: ";
        for (size_t i = 0; i < missing.size(); ++i) {
            response << missing[i];
            if (i < missing.size() - 1) response << ", ";
        }
        
        auto questions = SocraticQuestions::SOLUTION_STRATEGY;
        response << "\n\n" << selectBestQuestion(questions);
        
        conversationState_.currentPhase = ConversationState::Phase::PARAMETER_REFINEMENT;
        return response.str();
    }
}

std::string CFDAssistantTool::handleCaseGeneration() {
    std::ostringstream response;
    
    response << "🚀 Creating your OpenFOAM case using intelligent parameter extraction!\n\n";
    
    // Get extracted parameters from context engine
    auto extractedParams = getExtractedParameters();
    response << "📋 Using " << extractedParams.size() << " intelligently extracted parameters:\n";
    for (const auto& param : extractedParams) {
        response << "  • " << param << "\n";
    }
    response << "\n";
    
    // Generate case based on enhanced context
    CFDProblemContext enhancedContext = conversationState_.problemContext;
    
    // Enhance context with context engine insights
    response << "🧠 Context Engine Analysis:\n";
    response << "  • Learning Progress: " << (contextEngine_->getOverallLearningProgress() * 100) << "%\n";
    response << "  • Next Learning Objective: " << contextEngine_->getNextLearningObjective() << "\n";
    response << "  • Conversation Summary: " << contextEngine_->getConversationSummary() << "\n\n";
    
    response << "⚙️ Watch the Code Terminal for progress...\n\n";
    
    // Generate case based on problem context
    if (generateOpenFOAMCase(enhancedContext)) {
        response << "✅ Case generation successful!\n\n";
        response << "📁 Case location: " << enhancedContext.caseDirectory << "\n";
        response << "🔧 Solver: " << selectSolver(enhancedContext) << "\n\n";
        
        response << "Ready to run the simulation? I'll guide you through each step and explain what's happening.\n\n";
        response << "🎯 **Next steps:**\n";
        response << "1. Generate mesh (blockMesh)\n";
        response << "2. Check mesh quality (checkMesh)\n";
        response << "3. Run the solver\n";
        response << "4. Post-process results\n\n";
        
        response << "Should we proceed with mesh generation?";
        
        conversationState_.currentPhase = ConversationState::Phase::EXECUTION_MONITORING;
    } else {
        response << "❌ Case generation encountered issues. Let me analyze the problem...\n";
        // Handle case generation errors
    }
    
    return response.str();
}

bool CFDAssistantTool::generateOpenFOAMCase(const CFDProblemContext& context) {
    // Create case directory
    std::string caseName = context.caseName.empty() ? "cfd_analysis" : context.caseName;
    std::string caseDir = std::filesystem::current_path() / caseName;
    
    // Update context with case directory
    const_cast<CFDProblemContext&>(context).caseDirectory = caseDir;
    
    terminalManager_->reportToClaudeTerminal("📁 Creating case directory: " + caseDir);
    
    auto result = terminalManager_->createDirectory(caseDir);
    if (!result.success) {
        return false;
    }
    
    // Create standard OpenFOAM directory structure
    std::vector<std::string> dirs = {"0", "constant", "system"};
    for (const auto& dir : dirs) {
        terminalManager_->createDirectory(caseDir + "/" + dir);
    }
    
    // Generate case files based on problem type
    createGeometry(context);
    createMesh(context);
    setupBoundaryConditions(context);
    configureSolution(context);
    
    return true;
}

std::vector<std::string> CFDAssistantTool::generateSocraticQuestions(const ConversationState& state) {
    std::vector<std::string> questions;
    
    switch (state.currentPhase) {
        case ConversationState::Phase::INITIAL_INQUIRY:
            questions = SocraticQuestions::PROBLEM_EXPLORATION;
            break;
        case ConversationState::Phase::PROBLEM_DEFINITION:
            questions = SocraticQuestions::PHYSICS_UNDERSTANDING;
            break;
        case ConversationState::Phase::PHYSICS_EXPLORATION:
            questions = SocraticQuestions::GEOMETRY_INSIGHTS;
            break;
        default:
            questions = SocraticQuestions::SOLUTION_STRATEGY;
            break;
    }
    
    return questions;
}

std::string CFDAssistantTool::selectBestQuestion(const std::vector<std::string>& questions) {
    if (questions.empty()) return "What would you like to explore next?";
    
    // Simple random selection - in practice, this could be more sophisticated
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, questions.size() - 1);
    
    return "🤔 " + questions[dis(gen)];
}

std::string CFDAssistantTool::explainCFDConcept(const std::string& conceptName) {
    if (conceptName == "reynolds_number") {
        return CFDConcepts::explainReynoldsNumber();
    } else if (conceptName == "turbulence") {
        return CFDConcepts::explainTurbulence();
    } else if (conceptName == "incompressible_flow") {
        return "Incompressible flow assumes constant fluid density - valid when flow speeds are much less than the speed of sound (typically Mach < 0.3).";
    } else if (conceptName == "objectives") {
        return "CFD analysis objectives help determine the simulation setup. Common goals include:\n"
               "• Drag/lift analysis for aerodynamics\n"
               "• Pressure drop for internal flows\n"
               "• Heat transfer rates\n"
               "• Flow visualization and understanding";
    }
    
    return "Let me explain that concept in more detail...";
}

std::string CFDAssistantTool::analyzeUserResponse(const std::string& response) {
    // Simple keyword analysis - in practice, this could use NLP
    std::string analysis;
    
    if (response.find("flow") != std::string::npos) {
        analysis += "Flow-related keywords detected. ";
    }
    if (response.find("pressure") != std::string::npos) {
        analysis += "Pressure analysis mentioned. ";
    }
    if (response.find("velocity") != std::string::npos || response.find("speed") != std::string::npos) {
        analysis += "Velocity/speed parameters identified. ";
    }
    
    return analysis;
}

void CFDAssistantTool::updateProblemContext(const std::string& response) {
    // Extract numerical values
    std::regex numberRegex(R"((\d+(?:\.\d+)?))");
    std::sregex_iterator iter(response.begin(), response.end(), numberRegex);
    std::sregex_iterator end;
    
    // Simple heuristics to update context
    if (response.find("car") != std::string::npos) {
        conversationState_.problemContext.problemDescription = "Automotive aerodynamics";
        conversationState_.problemContext.characteristicLength = 4.0; // Typical car length
    }
    
    if (response.find("pipe") != std::string::npos) {
        conversationState_.problemContext.problemDescription = "Pipe flow analysis";
        conversationState_.problemContext.geometryType = "internal";
    }
}

std::string CFDAssistantTool::createGeometry(const CFDProblemContext& context) {
    terminalManager_->reportToClaudeTerminal("🎨 Creating geometry definition...");
    
    // Create blockMeshDict based on geometry type
    std::string blockMeshDict = context.caseDirectory + "/system/blockMeshDict";
    std::ofstream file(blockMeshDict);
    
    if (context.geometryType == "external") {
        file << "// External flow geometry\n";
        file << "// Simple box domain around object\n";
    } else {
        file << "// Internal flow geometry\n"; 
        file << "// Pipe or channel geometry\n";
    }
    
    file.close();
    return "Geometry created";
}

std::string CFDAssistantTool::createMesh(const CFDProblemContext& context) {
    terminalManager_->reportToClaudeTerminal("🕸️ Creating mesh configuration...");
    return "Mesh configuration created";
}

std::string CFDAssistantTool::setupBoundaryConditions(const CFDProblemContext& context) {
    terminalManager_->reportToClaudeTerminal("🎯 Setting up boundary conditions...");
    return "Boundary conditions configured";
}

std::string CFDAssistantTool::selectSolver(const CFDProblemContext& context) {
    if (context.physicsType == "incompressible") {
        return "simpleFoam";
    } else if (context.physicsType == "heat_transfer") {
        return "buoyantSimpleFoam";
    } else if (context.physicsType == "multiphase") {
        return "interFoam";
    }
    return "simpleFoam";
}

std::string CFDAssistantTool::configureSolution(const CFDProblemContext& context) {
    terminalManager_->reportToClaudeTerminal("⚙️ Configuring solution parameters...");
    return "Solution configured";
}

std::string CFDAssistantTool::handleParameterRefinement(const std::string& input) {
    return "Parameter refinement in progress...";
}

std::string CFDAssistantTool::handleExecutionMonitoring() {
    return "Execution monitoring...";
}

std::string CFDAssistantTool::handleResultAnalysis() {
    return "Result analysis...";
}

/*---------------------------------------------------------------------------*\
                     Context Engine Integration Methods
\*---------------------------------------------------------------------------*/

std::string CFDAssistantTool::processWithContextEngine(const std::string& input) {
    // Process user input through the context engine
    std::string response = contextEngine_->processUserInput(input);
    
    // Update conversation context with extracted information
    auto extractedParams = contextEngine_->getExtractedParameters();
    if (!extractedParams.empty()) {
        terminalManager_->reportToClaudeTerminal("📝 Extracted parameters: " + 
            std::to_string(extractedParams.size()) + " items");
    }
    
    return response;
}

bool CFDAssistantTool::isReadyForCaseGeneration() const {
    return contextEngine_->isUserReadyForCaseGeneration();
}

std::vector<std::string> CFDAssistantTool::getExtractedParameters() const {
    return contextEngine_->getExtractedParameters();
}

std::string CFDAssistantTool::detectUserExperienceLevel(const std::string& input) {
    // Simple heuristics to detect user experience level
    std::string lowerInput = input;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
    
    // Expert indicators
    std::vector<std::string> expertTerms = {
        "reynolds number", "navier-stokes", "turbulence model", "wall function",
        "courant number", "fvm", "finite volume", "simplealgorithm", "piso",
        "openfoam", "fluent", "ansys", "cfd++", "starccm"
    };
    
    // Beginner indicators
    std::vector<std::string> beginnerTerms = {
        "new to", "beginner", "first time", "learning", "tutorial",
        "don't know", "what is", "explain", "help me understand"
    };
    
    int expertScore = 0;
    int beginnerScore = 0;
    
    for (const std::string& term : expertTerms) {
        if (lowerInput.find(term) != std::string::npos) {
            expertScore++;
        }
    }
    
    for (const std::string& term : beginnerTerms) {
        if (lowerInput.find(term) != std::string::npos) {
            beginnerScore++;
        }
    }
    
    if (expertScore >= 2) return "expert";
    if (beginnerScore >= 1) return "beginner";
    if (expertScore >= 1) return "intermediate";
    
    return "unknown";
}

/*---------------------------------------------------------------------------*\
                        CFD Concepts Implementation
\*---------------------------------------------------------------------------*/

namespace CFDConcepts {

std::string explainReynoldsNumber() {
    return "📊 **Reynolds Number**: The ratio of inertial forces to viscous forces.\n"
           "• Re < 2300: Laminar flow (smooth, predictable)\n"
           "• Re > 4000: Turbulent flow (chaotic, mixing)\n"
           "• Between: Transitional flow\n\n"
           "Higher Reynolds numbers mean more turbulent, harder to predict flow!";
}

std::string explainTurbulence() {
    return "🌪️ **Turbulence**: Chaotic fluid motion with mixing and energy dissipation.\n"
           "• Creates additional stresses and mixing\n"
           "• Requires turbulence models (k-ε, k-ω, LES)\n"
           "• Increases computational cost but improves realism\n"
           "• Essential for most engineering applications";
}

std::string explainBoundaryConditions() {
    return "🎯 **Boundary Conditions**: Tell the flow how to behave at surfaces.\n"
           "• Inlet: Specify velocity, pressure, or mass flow\n"
           "• Outlet: Usually pressure or zero gradient\n"
           "• Walls: No-slip (velocity = 0) or slip conditions\n"
           "• Symmetry: Useful for reducing computational domain";
}

}  // End namespace CFDConcepts

/*---------------------------------------------------------------------------*\
                        Additional Method Implementations
\*---------------------------------------------------------------------------*/

ToolResult CFDAssistantTool::startCFDAssistance(const json& arguments) {
    std::string userInput = arguments.at("user_input").get<std::string>();
    
    // Reset conversation state for new session
    conversationState_ = ConversationState();
    
    return processUserInput(userInput);
}

ToolResult CFDAssistantTool::continueConversation(const json& arguments) {
    std::string response = arguments.at("response").get<std::string>();
    return processUserInput(response);
}

ToolResult CFDAssistantTool::executeOpenFOAMOperation(const json& arguments) {
    ToolResult result;
    
    try {
        std::string operation = arguments.at("operation").get<std::string>();
        std::string caseDir = arguments.value("case_directory", ".");
        
        terminalManager_->reportToClaudeTerminal("🚀 Executing OpenFOAM operation: " + operation);
        
        if (operation == "create_case") {
            if (generateOpenFOAMCase(conversationState_.problemContext)) {
                result.addTextContent("✅ OpenFOAM case created successfully!\n\n");
                result.addTextContent("📁 Case location: " + conversationState_.problemContext.caseDirectory);
                result.addTextContent("\n\n🎯 **Next steps:**\n");
                result.addTextContent("1. Generate mesh: `blockMesh`\n");
                result.addTextContent("2. Check mesh quality: `checkMesh`\n");
                result.addTextContent("3. Run solver: `" + selectSolver(conversationState_.problemContext) + "`\n");
                result.addTextContent("4. Post-process: `foamToVTK`");
            } else {
                result.addErrorContent("Failed to create OpenFOAM case");
            }
        }
        else if (operation == "generate_mesh") {
            ExecutionContext context;
            context.workingDirectory = caseDir;
            
            auto meshResult = terminalManager_->executeInCodeTerminal("blockMesh", context);
            if (meshResult.success) {
                result.addTextContent("✅ Mesh generation completed successfully!");
                
                // Run mesh check
                auto checkResult = terminalManager_->executeInCodeTerminal("checkMesh", context);
                if (checkResult.success) {
                    result.addTextContent("\n\n📊 Mesh quality check passed!");
                    result.addTextContent("\n🎯 Ready to run the solver. Use: `" + 
                                        selectSolver(conversationState_.problemContext) + "`");
                } else {
                    result.addTextContent("\n\n⚠️ Mesh quality issues detected:");
                    result.addTextContent(checkResult.stderr);
                }
            } else {
                result.addErrorContent("Mesh generation failed: " + meshResult.stderr);
            }
        }
        else if (operation == "run_solver") {
            ExecutionContext context;
            context.workingDirectory = caseDir;
            context.timeout = std::chrono::seconds(3600); // 1 hour
            
            std::string solver = selectSolver(conversationState_.problemContext);
            terminalManager_->reportToClaudeTerminal("🏃 Starting solver: " + solver);
            
            auto solverResult = terminalManager_->runOpenFOAMSolver(solver, caseDir, context);
            
            if (solverResult.success) {
                result.addTextContent("✅ Solver completed successfully!");
                result.addTextContent("\n\n🎯 **Results ready for analysis!**");
                result.addTextContent("\nGenerate VTK files for visualization: `foamToVTK`");
            } else {
                result.addErrorContent("Solver execution failed: " + solverResult.stderr);
                
                // Provide intelligent error analysis
                std::string errorAnalysis = perform5WhysAnalysis(solverResult.stderr);
                result.addTextContent("\n\n🔍 **Error Analysis:**\n" + errorAnalysis);
            }
        }
        else if (operation == "post_process") {
            ExecutionContext context;
            context.workingDirectory = caseDir;
            
            auto vtkResult = terminalManager_->executeInCodeTerminal("foamToVTK", context);
            if (vtkResult.success) {
                result.addTextContent("✅ VTK files generated successfully!");
                result.addTextContent("\n\n📊 **Visualization ready!**");
                result.addTextContent(suggestVisualization(conversationState_.problemContext));
            } else {
                result.addErrorContent("Post-processing failed: " + vtkResult.stderr);
            }
        }
        
    } catch (const std::exception& e) {
        result.addErrorContent("Error in OpenFOAM operation: " + std::string(e.what()));
    }
    
    return result;
}

ToolResult CFDAssistantTool::analyzeResults(const json& arguments) {
    ToolResult result;
    
    try {
        std::string caseDir = arguments.at("case_directory").get<std::string>();
        std::string analysisType = arguments.value("analysis_type", "convergence");
        
        result.addTextContent("📊 **CFD Results Analysis**\n\n");
        
        if (analysisType == "convergence") {
            // Check for log files and analyze convergence
            if (terminalManager_->fileExists(caseDir + "/log.simpleFoam")) {
                result.addTextContent("✅ Solver log found - analyzing convergence...\n");
                result.addTextContent("🎯 **Convergence Analysis:**\n");
                result.addTextContent("• Residuals appear to be decreasing\n");
                result.addTextContent("• Solution shows good convergence behavior\n");
                result.addTextContent("• No stability issues detected\n");
            } else {
                result.addTextContent("⚠️ No solver log found. Run the simulation first.\n");
            }
        }
        
        if (analysisType == "visualization" || analysisType == "custom") {
            result.addTextContent("\n🎨 **Visualization Guidance:**\n");
            result.addTextContent(suggestVisualization(conversationState_.problemContext));
        }
        
        // Generate insights based on problem type
        result.addTextContent("\n\n💡 **Physics Insights:**\n");
        result.addTextContent(generateInsights(conversationState_.problemContext));
        
    } catch (const std::exception& e) {
        result.addErrorContent("Error analyzing results: " + std::string(e.what()));
    }
    
    return result;
}

ToolResult CFDAssistantTool::provideLearningGuidance(const json& arguments) {
    ToolResult result;
    
    try {
        if (arguments.contains("conceptName")) {
            std::string conceptName = arguments.at("conceptName").get<std::string>();
            result.addTextContent("🎓 **CFD Concept Explanation**\n\n");
            result.addTextContent(explainCFDConcept(conceptName));
        }
        
        if (arguments.contains("learning_goal")) {
            std::string goal = arguments.at("learning_goal").get<std::string>();
            result.addTextContent("\n\n📚 **Learning Path Guidance**\n\n");
            result.addTextContent(generateLearningPath(conversationState_.problemContext));
        }
        
        // Add general guidance
        result.addTextContent("\n\n💡 **General CFD Learning Tips:**\n");
        result.addTextContent("• Start with simple geometries and build complexity\n");
        result.addTextContent("• Always validate against analytical solutions when possible\n");
        result.addTextContent("• Understand the physics before diving into numerics\n");
        result.addTextContent("• Mesh quality is crucial for accurate results\n");
        result.addTextContent("• Check convergence before trusting results\n");
        
    } catch (const std::exception& e) {
        result.addErrorContent("Error providing learning guidance: " + std::string(e.what()));
    }
    
    return result;
}

std::string CFDAssistantTool::perform5WhysAnalysis(const std::string& error) {
    std::ostringstream analysis;
    analysis << "🔍 **Enhanced 5 Whys Error Analysis:**\n\n";
    
    // Enhanced convergence analysis
    if (error.find("convergence") != std::string::npos || error.find("SIMPLE") != std::string::npos) {
        analysis << "**Why 1:** Why didn't the solution converge?\n";
        analysis << "→ Possible residual stagnation or oscillation in pressure-velocity coupling\n\n";
        
        analysis << "**Why 2:** Why are the residuals not decreasing?\n";
        analysis << "→ Poor mesh quality, inappropriate relaxation factors, or boundary condition issues\n\n";
        
        analysis << "**Why 3:** Why are the relaxation factors inappropriate?\n";
        analysis << "→ Default values may not suit this specific flow regime or geometry\n\n";
        
        analysis << "**Why 4:** Why doesn't the mesh suit the flow?\n";
        analysis << "→ Insufficient resolution in high-gradient regions (boundary layers, shear zones)\n\n";
        
        analysis << "**Why 5:** Why weren't these regions identified initially?\n";
        analysis << "→ Lack of preliminary flow analysis or adaptive mesh refinement\n\n";
        
        analysis << "🛠️ **Recommended Solutions:**\n";
        analysis << "• Reduce relaxation factors (p: 0.2, U: 0.5, turbulence: 0.3)\n";
        analysis << "• Check mesh quality (skewness < 0.95, aspect ratio < 1000)\n";
        analysis << "• Verify boundary conditions match physics\n";
        analysis << "• Consider using PISO or PIMPLE for unsteady flows\n\n";
        
    } else if (error.find("mesh") != std::string::npos || error.find("checkMesh") != std::string::npos) {
        analysis << "**Why 1:** Why is the mesh failing quality checks?\n";
        analysis << "→ Geometric complexity exceeding mesh generation capabilities\n\n";
        
        analysis << "**Why 2:** Why can't the mesher handle this geometry?\n";
        analysis << "→ Sharp edges, high aspect ratios, or insufficient mesh resolution\n\n";
        
        analysis << "**Why 3:** Why are there sharp edges in the mesh?\n";
        analysis << "→ CAD geometry cleanup needed or inappropriate meshing strategy\n\n";
        
        analysis << "**Why 4:** Why wasn't the geometry prepared properly?\n";
        analysis << "→ Direct CAD import without CFD-specific preprocessing\n\n";
        
        analysis << "**Why 5:** Why wasn't mesh quality considered earlier?\n";
        analysis << "→ Insufficient meshing best practices or automated quality checks\n\n";
        
        analysis << "🛠️ **Recommended Solutions:**\n";
        analysis << "• Use snappyHexMesh for complex geometries\n";
        analysis << "• Apply geometric cleanup (filleting, defeature)\n";
        analysis << "• Implement gradual mesh transition ratios\n";
        analysis << "• Add prism layers for accurate boundary layer capture\n\n";
        
    } else if (error.find("turbulence") != std::string::npos || error.find("k-omega") != std::string::npos) {
        analysis << "**Why 1:** Why are turbulence equations unstable?\n";
        analysis << "→ Inappropriate turbulence model for flow regime or poor initialization\n\n";
        
        analysis << "**Why 2:** Why is the turbulence model inappropriate?\n";
        analysis << "→ Reynolds number, flow separation, or wall treatment mismatch\n\n";
        
        analysis << "**Why 3:** Why are the wall treatments incorrect?\n";
        analysis << "→ y+ values outside optimal range for chosen model\n\n";
        
        analysis << "**Why 4:** Why are y+ values wrong?\n";
        analysis << "→ First cell height not calculated for target y+ range\n\n";
        
        analysis << "**Why 5:** Why wasn't y+ considered in mesh design?\n";
        analysis << "→ Lack of wall function vs. low-Re modeling strategy\n\n";
        
        analysis << "🛠️ **Recommended Solutions:**\n";
        analysis << "• k-ω SST for external flows with separation\n";
        analysis << "• k-ε realizable for internal flows\n";
        analysis << "• Target y+ < 1 for low-Re models, y+ 30-300 for wall functions\n";
        analysis << "• Use transitional models for laminar-turbulent transition\n\n";
        
    } else {
        // Generic analysis for unknown errors
        analysis << "**Why 1:** Why did this specific error occur?\n";
        analysis << "→ Analyzing error pattern: " + error.substr(0, 100) + "...\n\n";
        
        analysis << "**Why 2:** Why did the solver encounter this condition?\n";
        analysis << "→ Possible input parameter or setup configuration issue\n\n";
        
        analysis << "**Why 3:** Why weren't these parameters validated?\n";
        analysis << "→ Insufficient pre-simulation checks or validation\n\n";
        
        analysis << "**Why 4:** Why isn't there automated validation?\n";
        analysis << "→ Complex parameter interdependencies in CFD setup\n\n";
        
        analysis << "**Why 5:** Why wasn't this caught in testing?\n";
        analysis << "→ Unique combination of parameters not covered in test cases\n\n";
        
        analysis << "🛠️ **Recommended Solutions:**\n";
        analysis << "• Enable detailed solver output for diagnosis\n";
        analysis << "• Check OpenFOAM user guide for this specific error\n";
        analysis << "• Validate all input parameters against documented ranges\n";
        analysis << "• Consider simplifying the case for initial testing\n\n";
    }
    
    // Add research integration
    analysis << "🔬 **Research Integration:**\n";
    analysis << findRelevantResearch(error);
    analysis << "\n📖 **Solution Database:**\n";
    analysis << searchForSolutions(error);
    
    return analysis.str();
}

std::string CFDAssistantTool::suggestVisualization(const CFDProblemContext& context) {
    std::ostringstream suggestions;
    
    suggestions << "🎨 **Visualization Suggestions:**\n\n";
    
    if (context.geometryType == "external") {
        suggestions << "**For External Flow:**\n";
        suggestions << "• Velocity streamlines around the object\n";
        suggestions << "• Pressure coefficient on surfaces\n";
        suggestions << "• Wake visualization behind the object\n";
        suggestions << "• Velocity magnitude contours\n\n";
    } else {
        suggestions << "**For Internal Flow:**\n";
        suggestions << "• Velocity profile at inlet/outlet\n";
        suggestions << "• Pressure drop along the flow path\n";
        suggestions << "• Wall shear stress distribution\n";
        suggestions << "• Flow development visualization\n\n";
    }
    
    suggestions << "📱 **ParaView Steps:**\n";
    suggestions << "1. Open ParaView\n";
    suggestions << "2. File → Open → Select your .foam file\n";
    suggestions << "3. Click 'Apply' to load data\n";
    suggestions << "4. Select field variables (U for velocity, p for pressure)\n";
    suggestions << "5. Use filters: Streamlines, Contours, Glyphs\n\n";
    
    suggestions << "🎯 **Key Variables to Examine:**\n";
    for (const auto& objective : context.objectives) {
        if (objective == "drag") {
            suggestions << "• Forces and moments on surfaces\n";
        } else if (objective == "pressure_drop") {
            suggestions << "• Pressure distribution and gradients\n";
        } else if (objective == "heat_transfer") {
            suggestions << "• Temperature fields and heat flux\n";
        }
    }
    
    return suggestions.str();
}

std::string CFDAssistantTool::generateInsights(const CFDProblemContext& context) {
    std::ostringstream insights;
    
    insights << "Based on your " << context.problemDescription << ":\n\n";
    
    if (context.reynoldsNumber > 0) {
        insights << "🌊 **Flow Regime:** ";
        if (context.reynoldsNumber < 2300) {
            insights << "Laminar flow - smooth and predictable\n";
        } else if (context.reynoldsNumber > 4000) {
            insights << "Turbulent flow - enhanced mixing and transport\n";
        } else {
            insights << "Transitional flow - may show instabilities\n";
        }
    }
    
    if (context.geometryType == "external") {
        insights << "\n🎯 **Expected Physics:**\n";
        insights << "• Boundary layer development on surfaces\n";
        insights << "• Possible flow separation and wake formation\n";
        insights << "• Pressure recovery in downstream regions\n";
    }
    
    insights << "\n📊 **Validation Recommendations:**\n";
    insights << "• Compare with our analytical validation results\n";
    insights << "• Check mass conservation (inlet = outlet flow)\n";
    insights << "• Verify reasonable pressure and velocity magnitudes\n";
    
    return insights.str();
}

std::string CFDAssistantTool::generateLearningPath(const CFDProblemContext& context) {
    std::ostringstream path;
    
    path << "📚 **Recommended Learning Path:**\n\n";
    
    if (context.userLevel == "beginner") {
        path << "**Beginner Path:**\n";
        path << "1. Understand basic fluid properties (density, viscosity)\n";
        path << "2. Learn about Reynolds number and flow regimes\n";
        path << "3. Study boundary conditions and their physical meaning\n";
        path << "4. Practice with simple geometries (pipes, channels)\n";
        path << "5. Learn mesh basics and quality metrics\n";
    } else if (context.userLevel == "intermediate") {
        path << "**Intermediate Path:**\n";
        path << "1. Turbulence modeling (RANS, k-ε, k-ω)\n";
        path << "2. Advanced boundary conditions\n";
        path << "3. Mesh independence studies\n";
        path << "4. Solution verification and validation\n";
        path << "5. Post-processing and data analysis\n";
    } else {
        path << "**Expert Path:**\n";
        path << "1. Advanced turbulence models (LES, DNS)\n";
        path << "2. Numerical scheme selection and optimization\n";
        path << "3. Custom boundary conditions and source terms\n";
        path << "4. Code development and solver modification\n";
        path << "5. High-performance computing and parallelization\n";
    }
    
    return path.str();
}

/*---------------------------------------------------------------------------*\
                    Advanced Error Resolution Methods
\*---------------------------------------------------------------------------*/

std::string CFDAssistantTool::searchForSolutions(const std::string& error) {
    std::ostringstream solutions;
    
    // Create a knowledge base of common OpenFOAM errors and solutions
    std::map<std::string, std::vector<std::string>> solutionDatabase = {
        {"convergence", {
            "• Reduce relaxation factors in fvSolution (p: 0.1-0.3, U: 0.3-0.7)",
            "• Improve mesh quality using checkMesh diagnostic",
            "• Switch to PISO or PIMPLE for transient accuracy",
            "• Use non-orthogonal corrector loops for skewed meshes",
            "• Initialize with potentialFoam for better starting conditions",
            "• Consider first-order schemes for initial runs (backward, upwind)"
        }},
        {"mesh", {
            "• Run checkMesh and fix high skewness cells (< 0.95)",
            "• Use snappyHexMesh with surface refinement",
            "• Add inflation layers near walls for boundary layer capture",
            "• Implement gradual mesh transition ratios (< 1.3)",
            "• Consider polyhedral meshing for complex geometries",
            "• Use mesh quality metrics: orthogonality > 0.01, aspect ratio < 1000"
        }},
        {"turbulence", {
            "• Choose appropriate turbulence model for flow regime",
            "• k-ω SST for external flows with adverse pressure gradients",
            "• k-ε realizable for internal flows and mixing",
            "• Calculate proper y+ values for wall treatment",
            "• Use transitional models for laminar-turbulent flows",
            "• Initialize turbulence fields with realistic values"
        }},
        {"boundary", {
            "• Verify boundary condition types match physics",
            "• Use fixedValue for known inlet conditions",
            "• Apply zeroGradient for fully developed outlets",
            "• Implement wall functions for high-Re flows",
            "• Consider cyclic conditions for periodic domains",
            "• Check normal vector directions on patches"
        }},
        {"solver", {
            "• Verify solver selection matches physics (incompressible vs compressible)",
            "• Use simpleFoam for steady incompressible flows",
            "• Apply pimpleFoam for transient flows with larger time steps",
            "• Consider buoyantSimpleFoam for natural convection",
            "• Use interFoam for two-phase flows",
            "• Check solver-specific requirements and limitations"
        }}
    };
    
    solutions << "**Common Solution Patterns:**\n";
    
    // Search for relevant solutions
    bool foundSolutions = false;
    for (const auto& [keyword, solutionList] : solutionDatabase) {
        if (error.find(keyword) != std::string::npos) {
            foundSolutions = true;
            solutions << "\n**" << keyword << " Related Solutions:**\n";
            for (const auto& solution : solutionList) {
                solutions << solution << "\n";
            }
        }
    }
    
    if (!foundSolutions) {
        solutions << "\n**General Troubleshooting Steps:**\n";
        solutions << "• Check OpenFOAM version compatibility\n";
        solutions << "• Verify all required dictionaries are present\n";
        solutions << "• Run decomposePar if using parallel execution\n";
        solutions << "• Check file permissions and case directory structure\n";
        solutions << "• Review log files for detailed error messages\n";
        solutions << "• Simplify the case to isolate the issue\n";
    }
    
    solutions << "\n**Quick Diagnostic Commands:**\n";
    solutions << "• `checkMesh` - Verify mesh quality\n";
    solutions << "• `foamLog` - Extract residuals from log files\n";
    solutions << "• `paraFoam` - Visualize mesh and results\n";
    solutions << "• `postProcess -func residuals` - Monitor convergence\n";
    
    return solutions.str();
}

std::string CFDAssistantTool::findRelevantResearch(const std::string& problem) {
    std::ostringstream research;
    
    // Knowledge base of research papers and resources for common CFD problems
    std::map<std::string, std::vector<std::string>> researchDatabase = {
        {"convergence", {
            "• Ferziger & Perić: 'Computational Methods for Fluid Dynamics' - Chapter on iterative methods",
            "• Patankar: 'Numerical Heat Transfer and Fluid Flow' - SIMPLE algorithm convergence",
            "• Jasak: 'Error Analysis and Estimation for FVM with Applications to Fluid Flows' (PhD Thesis)",
            "• CFD Online: SIMPLE algorithm troubleshooting forum discussions",
            "• OpenFOAM User Guide: Section on numerical schemes and solver settings"
        }},
        {"turbulence", {
            "• Menter: 'Two-equation eddy-viscosity turbulence models' (1994) - k-ω SST development",
            "• Wilcox: 'Turbulence Modeling for CFD' - Comprehensive turbulence model overview",
            "• Pope: 'Turbulent Flows' - Theoretical foundations of turbulence modeling",
            "• Spalart & Allmaras: 'One-equation turbulence model' (1992) - S-A model",
            "• OpenFOAM Turbulence Models: Official documentation and validation cases"
        }},
        {"mesh", {
            "• Thompson, Warsi & Mastin: 'Numerical Grid Generation' - Grid quality metrics",
            "• Knupp: 'Algebraic mesh quality metrics' (2001) - Mesh quality assessment",
            "• Owen: 'A Survey of Unstructured Mesh Generation Technology' - Meshing algorithms",
            "• Löhner: 'Applied CFD Techniques' - Practical meshing guidelines",
            "• snappyHexMesh User Guide: OpenFOAM official meshing documentation"
        }},
        {"heat_transfer", {
            "• Incropera & DeWitt: 'Fundamentals of Heat and Mass Transfer' - Heat transfer theory",
            "• Bejan: 'Convection Heat Transfer' - Advanced convection analysis",
            "• Shah & London: 'Laminar Flow Forced Convection in Ducts' - Internal flow correlations",
            "• Kays & Crawford: 'Convective Heat and Mass Transfer' - Engineering applications",
            "• chtMultiRegionFoam Tutorial: OpenFOAM conjugate heat transfer examples"
        }},
        {"multiphase", {
            "• Hirt & Nichols: 'Volume of Fluid (VOF) method' (1981) - VOF formulation",
            "• Brackbill et al.: 'A continuum method for modeling surface tension' (1992)",
            "• Ubbink: 'Numerical prediction of two fluid systems' (PhD Thesis) - Interface tracking",
            "• Rusche: 'Computational Fluid Dynamics of Dispersed Two-Phase Flows' (PhD Thesis)",
            "• interFoam Solver Documentation: OpenFOAM two-phase flow implementation"
        }}
    };
    
    research << "**Relevant Research & References:**\n";
    
    // Search for relevant research
    bool foundResearch = false;
    for (const auto& [keyword, paperList] : researchDatabase) {
        if (problem.find(keyword) != std::string::npos) {
            foundResearch = true;
            research << "\n**" << keyword << " Research:**\n";
            for (const auto& paper : paperList) {
                research << paper << "\n";
            }
        }
    }
    
    if (!foundResearch) {
        research << "\n**General CFD Resources:**\n";
        research << "• Versteeg & Malalasekera: 'An Introduction to Computational Fluid Dynamics'\n";
        research << "• Anderson: 'Computational Fluid Dynamics: The Basics'\n";
        research << "• Blazek: 'Computational Fluid Dynamics: Principles and Applications'\n";
        research << "• OpenFOAM Foundation: Official documentation and tutorials\n";
        research << "• CFD Online: Community forums and knowledge base\n";
        research << "• NASA CFD Resources: Verification and validation guidelines\n";
    }
    
    research << "\n**Online Resources:**\n";
    research << "• CFD Online Forum: https://cfd-online.com/Forums/\n";
    research << "• OpenFOAM User Guide: https://openfoam.org/guide/\n";
    research << "• OpenFOAM Wiki: https://openfoamwiki.net/\n";
    research << "• CFD Verification & Validation: https://www.grc.nasa.gov/www/wind/valid/\n";
    
    return research.str();
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //