/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Context Engineering Implementation
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Implementation of advanced context engineering system for intelligent
    CFD case generation using Socratic questioning methodology.

\*---------------------------------------------------------------------------*/

#include "context_engine.hpp"
#include <algorithm>
#include <regex>
#include <iostream>
#include <sstream>
#include <random>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        Enhanced User Model Implementation
\*---------------------------------------------------------------------------*/

EnhancedUserModel::EnhancedUserModel()
:
    experienceLevel_("beginner"),
    learningStyle_("analytical"),
    overallConfidence_(0.0),
    questionsAsked_(0),
    questionsAnsweredCorrectly_(0),
    preferredComplexityLevel_("basic")
{
    // Initialize default preferences
    preferences_["explanation_depth"] = "detailed";
    preferences_["math_complexity"] = "basic";
    preferences_["visual_aids"] = "yes";
    preferences_["real_world_examples"] = "yes";
}

void EnhancedUserModel::setExperienceLevel(const std::string& level)
{
    experienceLevel_ = level;
    
    // Adjust complexity preferences based on experience
    if (level == "beginner") {
        preferredComplexityLevel_ = "basic";
        preferences_["explanation_depth"] = "detailed";
        preferences_["math_complexity"] = "basic";
    } else if (level == "intermediate") {
        preferredComplexityLevel_ = "moderate";
        preferences_["explanation_depth"] = "moderate";
        preferences_["math_complexity"] = "intermediate";
    } else if (level == "expert") {
        preferredComplexityLevel_ = "advanced";
        preferences_["explanation_depth"] = "concise";
        preferences_["math_complexity"] = "advanced";
    }
}

void EnhancedUserModel::updateConceptUnderstanding(const std::string& conceptId, bool understands, double confidence)
{
    UserConceptModel& model = conceptModels_[conceptId];
    model.conceptId = conceptId;
    model.understands = understands;
    model.confidenceScore = confidence;
    model.lastAssessed = std::chrono::system_clock::now();
    
    // Update confused/strong concept lists
    auto confusedIt = std::find(confusedConcepts_.begin(), confusedConcepts_.end(), conceptId);
    auto strongIt = std::find(strongConcepts_.begin(), strongConcepts_.end(), conceptId);
    
    if (understands && confidence > 0.7) {
        if (confusedIt != confusedConcepts_.end()) {
            confusedConcepts_.erase(confusedIt);
        }
        if (strongIt == strongConcepts_.end()) {
            strongConcepts_.push_back(conceptId);
        }
    } else if (!understands || confidence < 0.4) {
        if (strongIt != strongConcepts_.end()) {
            strongConcepts_.erase(strongIt);
        }
        if (confusedIt == confusedConcepts_.end()) {
            confusedConcepts_.push_back(conceptId);
        }
    }
    
    // Update overall confidence
    calculateOverallConfidence();
}

bool EnhancedUserModel::isConceptUnderstood(const std::string& conceptId) const
{
    auto it = conceptModels_.find(conceptId);
    return (it != conceptModels_.end()) && it->second.understands;
}

double EnhancedUserModel::getConceptConfidence(const std::string& conceptId) const
{
    auto it = conceptModels_.find(conceptId);
    return (it != conceptModels_.end()) ? it->second.confidenceScore : 0.0;
}

std::vector<std::string> EnhancedUserModel::getUnderstoodConcepts() const
{
    std::vector<std::string> understood;
    for (const auto& pair : conceptModels_) {
        if (pair.second.understands) {
            understood.push_back(pair.first);
        }
    }
    return understood;
}

std::vector<std::string> EnhancedUserModel::getConfusedConcepts() const
{
    return confusedConcepts_;
}

void EnhancedUserModel::recordQuestionResponse(const std::string& conceptId, bool correct)
{
    questionsAsked_++;
    if (correct) {
        questionsAnsweredCorrectly_++;
    }
    
    // Update concept understanding based on response
    double currentConfidence = getConceptConfidence(conceptId);
    double newConfidence = correct ? 
        std::min(1.0, currentConfidence + 0.1) : 
        std::max(0.0, currentConfidence - 0.15);
    
    updateConceptUnderstanding(conceptId, newConfidence > 0.5, newConfidence);
}

std::string EnhancedUserModel::getPreferredComplexityLevel() const
{
    return preferredComplexityLevel_;
}

std::vector<std::string> EnhancedUserModel::identifyKnowledgeGaps() const
{
    std::vector<std::string> gaps;
    
    // Core CFD concepts that should be understood
    std::vector<std::string> coreConcepts = {
        "reynolds_number", "boundary_conditions", "turbulence", 
        "mesh_quality", "solver_selection", "convergence"
    };
    
    for (const std::string& conceptName : coreConcepts) {
        if (!isConceptUnderstood(conceptName)) {
            gaps.push_back(conceptName);
        }
    }
    
    return gaps;
}

std::vector<std::string> EnhancedUserModel::getReadyForNewConcepts() const
{
    std::vector<std::string> ready;
    
    // If user understands basics, they're ready for intermediate concepts
    if (isConceptUnderstood("reynolds_number") && isConceptUnderstood("boundary_conditions")) {
        if (!isConceptUnderstood("turbulence_modeling")) {
            ready.push_back("turbulence_modeling");
        }
        if (!isConceptUnderstood("mesh_refinement")) {
            ready.push_back("mesh_refinement");
        }
    }
    
    return ready;
}

bool EnhancedUserModel::isReadyForAdvancedTopic(const std::string& topicId) const
{
    if (topicId == "turbulence_modeling") {
        return isConceptUnderstood("reynolds_number") && 
               isConceptUnderstood("boundary_layer");
    }
    
    if (topicId == "multiphase_flow") {
        return isConceptUnderstood("surface_tension") && 
               isConceptUnderstood("contact_angle");
    }
    
    return getConceptConfidence(topicId) > 0.6;
}

void EnhancedUserModel::setPreference(const std::string& key, const std::string& value)
{
    preferences_[key] = value;
}

std::string EnhancedUserModel::getPreference(const std::string& key) const
{
    auto it = preferences_.find(key);
    return (it != preferences_.end()) ? it->second : "";
}

void EnhancedUserModel::addApplicationInterest(const std::string& application)
{
    if (std::find(applicationInterests_.begin(), applicationInterests_.end(), application) 
        == applicationInterests_.end()) {
        applicationInterests_.push_back(application);
    }
}

void EnhancedUserModel::calculateOverallConfidence()
{
    if (conceptModels_.empty()) {
        overallConfidence_ = 0.0;
        return;
    }
    
    double totalConfidence = 0.0;
    for (const auto& pair : conceptModels_) {
        totalConfidence += pair.second.confidenceScore;
    }
    
    overallConfidence_ = totalConfidence / conceptModels_.size();
}

/*---------------------------------------------------------------------------*\
                    Strategic Question Engine Implementation
\*---------------------------------------------------------------------------*/

StrategicQuestionEngine::StrategicQuestionEngine()
{
    // Initialize question templates
    questionTemplates_["clarify"] = {
        "What specifically do you mean by {concept}?",
        "Can you elaborate on your understanding of {concept}?",
        "When you mention {concept}, what comes to mind first?",
        "How would you explain {concept} to someone new to CFD?"
    };
    
    questionTemplates_["explore"] = {
        "What would happen if we increased the {parameter}?",
        "How do you think {concept} affects the flow behavior?",
        "What if we applied {concept} to a different geometry?",
        "Can you predict what happens when {condition} changes?"
    };
    
    questionTemplates_["confirm"] = {
        "So you're saying that {understanding}. Is that correct?",
        "Let me check my understanding: {summary}. Does this match your thinking?",
        "Based on what you've said, {interpretation}. Am I on the right track?",
        "It sounds like you believe {belief}. Is that accurate?"
    };
    
    questionTemplates_["apply"] = {
        "How would you use {concept} to solve {problem}?",
        "If you were designing a {application}, how would {concept} influence your approach?",
        "Can you think of a real-world situation where {concept} is critical?",
        "What steps would you take to implement {concept} in your simulation?"
    };
    
    // Initialize core CFD concepts
    initializeCFDConcepts();
}

void StrategicQuestionEngine::initializeCFDConcepts()
{
    // Reynolds Number
    CFDConcept reynolds;
    reynolds.conceptId = "reynolds_number";
    reynolds.name = "Reynolds Number";
    reynolds.description = "Dimensionless number characterizing flow regime";
    reynolds.prerequisites = {"velocity", "density", "viscosity", "characteristic_length"};
    reynolds.complexityLevel = 2;
    reynolds.keyQuestions = {
        "What does Reynolds number tell us about the flow?",
        "How do you calculate Reynolds number?",
        "When is a flow considered turbulent vs laminar?"
    };
    cfdConcepts_[reynolds.conceptId] = reynolds;
    
    // Boundary Conditions
    CFDConcept boundaryConditions;
    boundaryConditions.conceptId = "boundary_conditions";
    boundaryConditions.name = "Boundary Conditions";
    boundaryConditions.description = "Mathematical constraints applied at domain boundaries";
    boundaryConditions.prerequisites = {"mesh", "flow_physics"};
    boundaryConditions.complexityLevel = 2;
    boundaryConditions.keyQuestions = {
        "What boundary condition is appropriate for an inlet?",
        "How do wall functions affect near-wall modeling?",
        "What's the difference between Dirichlet and Neumann conditions?"
    };
    cfdConcepts_[boundaryConditions.conceptId] = boundaryConditions;
}

SocraticQuestion StrategicQuestionEngine::generateQuestion(
    QuestionStrategy strategy,
    const std::string& targetConcept,
    const EnhancedUserModel& userModel,
    const std::string& conversationContext)
{
    SocraticQuestion question;
    question.strategy = strategy;
    question.targetConcept = targetConcept;
    
    std::string strategyKey;
    switch (strategy) {
        case QuestionStrategy::CLARIFY:
            strategyKey = "clarify";
            question.questionText = generateClarifyingQuestion(targetConcept, conversationContext);
            question.cognitiveLoad = 1;
            break;
        case QuestionStrategy::EXPLORE:
            strategyKey = "explore";
            question.questionText = generateExploringQuestion(targetConcept, conversationContext);
            question.cognitiveLoad = 2;
            break;
        case QuestionStrategy::CONFIRM:
            strategyKey = "confirm";
            question.questionText = generateConfirmingQuestion(conversationContext);
            question.cognitiveLoad = 1;
            break;
        case QuestionStrategy::APPLY:
            strategyKey = "apply";
            question.questionText = generateApplicationQuestion(targetConcept, conversationContext);
            question.cognitiveLoad = 3;
            break;
    }
    
    // Set success criteria based on strategy and user level
    question.successCriteria = [targetConcept, strategy](const std::string& response) {
        // Basic keyword checking for now - can be enhanced with NLP
        return response.find(targetConcept) != std::string::npos && response.length() > 20;
    };
    
    return question;
}

QuestionStrategy StrategicQuestionEngine::selectOptimalStrategy(
    const std::string& targetConcept,
    const EnhancedUserModel& userModel)
{
    double confidence = userModel.getConceptConfidence(targetConcept);
    std::string experienceLevel = userModel.getExperienceLevel();
    
    // Strategy selection logic
    if (confidence < 0.3) {
        return QuestionStrategy::CLARIFY;  // Need to understand basics
    } else if (confidence < 0.6) {
        return QuestionStrategy::EXPLORE;  // Build deeper understanding
    } else if (confidence < 0.8) {
        return QuestionStrategy::CONFIRM;  // Validate understanding
    } else {
        return QuestionStrategy::APPLY;    // Apply knowledge
    }
}

std::string StrategicQuestionEngine::generateClarifyingQuestion(
    const std::string& conceptName,
    const std::string& context)
{
    std::map<std::string, std::string> variables;
    variables["concept"] = conceptName;
    variables["context"] = context;
    
    auto& templates = questionTemplates_["clarify"];
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, templates.size() - 1);
    
    return substituteContextVariables(templates[dis(gen)], variables);
}

std::string StrategicQuestionEngine::generateExploringQuestion(
    const std::string& conceptName,
    const std::string& scenario)
{
    std::map<std::string, std::string> variables;
    variables["concept"] = conceptName;
    variables["parameter"] = conceptName;  // Simplified for now
    variables["condition"] = scenario;
    
    auto& templates = questionTemplates_["explore"];
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, templates.size() - 1);
    
    return substituteContextVariables(templates[dis(gen)], variables);
}

std::string StrategicQuestionEngine::generateConfirmingQuestion(const std::string& understanding)
{
    std::map<std::string, std::string> variables;
    variables["understanding"] = understanding;
    variables["summary"] = understanding;
    variables["interpretation"] = understanding;
    variables["belief"] = understanding;
    
    auto& templates = questionTemplates_["confirm"];
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, templates.size() - 1);
    
    return substituteContextVariables(templates[dis(gen)], variables);
}

std::string StrategicQuestionEngine::generateApplicationQuestion(
    const std::string& conceptName,
    const std::string& newContext)
{
    std::map<std::string, std::string> variables;
    variables["concept"] = conceptName;
    variables["problem"] = newContext;
    variables["application"] = newContext;
    
    auto& templates = questionTemplates_["apply"];
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, templates.size() - 1);
    
    return substituteContextVariables(templates[dis(gen)], variables);
}

std::string StrategicQuestionEngine::substituteContextVariables(
    const std::string& templateStr,
    const std::map<std::string, std::string>& variables)
{
    std::string result = templateStr;
    
    for (const auto& pair : variables) {
        std::string placeholder = "{" + pair.first + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), pair.second);
            pos += pair.second.length();
        }
    }
    
    return result;
}

bool StrategicQuestionEngine::validateQuestionEffectiveness(
    const SocraticQuestion& question,
    const std::string& userResponse)
{
    // Check if success criteria are met
    if (question.successCriteria && question.successCriteria(userResponse)) {
        return true;
    }
    
    // Basic effectiveness checks
    if (userResponse.length() < 10) {
        return false;  // Too short, probably not engaged
    }
    
    // Check for keywords related to the target concept
    return userResponse.find(question.targetConcept) != std::string::npos;
}

/*---------------------------------------------------------------------------*\
                        Main Context Engine Implementation
\*---------------------------------------------------------------------------*/

ContextEngine::ContextEngine()
{
    // Initialize with basic CFD concepts
    knowledgeGraph_.buildCoreCFDConcepts();
}

std::string ContextEngine::processUserInput(const std::string& input)
{
    // Add to conversation history
    conversationHistory_.push_back("User: " + input);
    
    // Assess user response for concept understanding
    assessUserResponseForConcepts(input);
    
    // Update user model based on the response
    updateUserModelFromResponse(input);
    
    // Extract parameters if any
    auto extractedParams = parameterExtractor_.extractParametersFromConversation(input);
    for (const auto& param : extractedParams) {
        extractedFacts_.push_back(param.first + " = " + param.second.value);
    }
    
    // Generate appropriate response
    std::string response = generateNextQuestion();
    
    conversationHistory_.push_back("Assistant: " + response);
    return response;
}

std::string ContextEngine::generateNextQuestion()
{
    // Identify what concept to focus on next
    std::vector<std::string> knowledgeGaps = userModel_.identifyKnowledgeGaps();
    
    if (!knowledgeGaps.empty()) {
        std::string targetConcept = knowledgeGaps[0];  // Focus on first gap
        
        // Select optimal strategy
        QuestionStrategy strategy = questionEngine_.selectOptimalStrategy(targetConcept, userModel_);
        
        // Generate question
        std::string context = getConversationSummary();
        SocraticQuestion question = questionEngine_.generateQuestion(
            strategy, targetConcept, userModel_, context);
        
        currentTopic_ = targetConcept;
        return question.questionText;
    }
    
    // If no gaps, check if ready for case generation
    if (isUserReadyForCaseGeneration()) {
        return "Based on our discussion, I think you're ready to start creating an OpenFOAM case. "
               "What type of flow problem would you like to analyze?";
    }
    
    // Default encouraging question
    return "What aspect of CFD would you like to explore further?";
}

void ContextEngine::assessUserResponseForConcepts(const std::string& response)
{
    // Simple keyword-based assessment - can be enhanced with NLP
    std::map<std::string, std::vector<std::string>> conceptKeywords = {
        {"reynolds_number", {"reynolds", "re", "turbulent", "laminar", "transition"}},
        {"boundary_conditions", {"inlet", "outlet", "wall", "boundary", "dirichlet", "neumann"}},
        {"turbulence", {"turbulent", "k-epsilon", "k-omega", "les", "rans", "dns"}},
        {"mesh_quality", {"mesh", "elements", "skewness", "aspect ratio", "orthogonality"}}
    };
    
    for (const auto& conceptPair : conceptKeywords) {
        const std::string& conceptName = conceptPair.first;
        const auto& keywords = conceptPair.second;
        
        int keywordCount = 0;
        for (const std::string& keyword : keywords) {
            if (response.find(keyword) != std::string::npos) {
                keywordCount++;
            }
        }
        
        if (keywordCount > 0) {
            double confidence = std::min(1.0, keywordCount * 0.2 + 0.1);
            bool understands = keywordCount >= 2;  // Needs at least 2 keywords
            
            userModel_.updateConceptUnderstanding(conceptName, understands, confidence);
        }
    }
}

void ContextEngine::updateUserModelFromResponse(const std::string& response)
{
    // Analyze response characteristics
    if (response.length() > 100) {
        // Detailed response suggests higher engagement
        userModel_.setPreference("explanation_depth", "detailed");
    }
    
    // Check for mathematical expressions
    std::regex mathPattern(R"(\d+\.?\d*\s*[*/+-]\s*\d+\.?\d*)");
    if (std::regex_search(response, mathPattern)) {
        userModel_.setPreference("math_complexity", "intermediate");
    }
    
    // Check for application mentions
    std::vector<std::string> applications = {"automotive", "aerospace", "hvac", "marine", "industrial"};
    for (const std::string& app : applications) {
        if (response.find(app) != std::string::npos) {
            userModel_.addApplicationInterest(app);
        }
    }
}

void ContextEngine::identifyConfusionPatterns(const std::string& response)
{
    // Look for confusion indicators
    std::vector<std::string> confusionIndicators = {
        "i don't understand", "confused", "not sure", "don't know", "unclear"
    };
    
    for (const std::string& indicator : confusionIndicators) {
        if (response.find(indicator) != std::string::npos) {
            // Mark current topic as confusing
            if (!currentTopic_.empty()) {
                userModel_.updateConceptUnderstanding(currentTopic_, false, 0.2);
            }
            break;
        }
    }
}

void ContextEngine::setUserExperienceLevel(const std::string& level)
{
    userModel_.setExperienceLevel(level);
}

void ContextEngine::updateConversationContext(const std::string& key, const std::string& value)
{
    conversationContext_[key] = value;
}

std::string ContextEngine::getConversationSummary() const
{
    if (conversationHistory_.empty()) {
        return "Beginning of conversation";
    }
    
    // Return last few exchanges for context
    std::stringstream summary;
    int startIdx = std::max(0, static_cast<int>(conversationHistory_.size()) - 4);
    
    for (int i = startIdx; i < conversationHistory_.size(); ++i) {
        summary << conversationHistory_[i] << " ";
    }
    
    return summary.str();
}

std::vector<std::string> ContextEngine::getExtractedParameters() const
{
    return extractedFacts_;
}

bool ContextEngine::isUserReadyForCaseGeneration() const
{
    // Check if user understands core concepts
    std::vector<std::string> coreConcepts = {"reynolds_number", "boundary_conditions"};
    
    for (const std::string& conceptName : coreConcepts) {
        if (!userModel_.isConceptUnderstood(conceptName)) {
            return false;
        }
    }
    
    // Check overall confidence
    return userModel_.getOverallConfidence() > 0.6;
}

std::vector<std::string> ContextEngine::getRequiredConceptsForCase(const std::string& caseType) const
{
    std::map<std::string, std::vector<std::string>> caseRequirements = {
        {"pipe_flow", {"reynolds_number", "boundary_conditions", "mesh_quality"}},
        {"external_flow", {"reynolds_number", "turbulence", "boundary_conditions", "mesh_refinement"}},
        {"heat_transfer", {"heat_transfer", "boundary_conditions", "material_properties"}},
        {"multiphase", {"surface_tension", "contact_angle", "phase_properties", "vof_method"}}
    };
    
    auto it = caseRequirements.find(caseType);
    return (it != caseRequirements.end()) ? it->second : std::vector<std::string>();
}

double ContextEngine::getOverallLearningProgress() const
{
    return userModel_.getOverallConfidence();
}

std::string ContextEngine::getNextLearningObjective() const
{
    auto gaps = userModel_.identifyKnowledgeGaps();
    if (!gaps.empty()) {
        return "Focus on understanding: " + gaps[0];
    }
    return "Ready for advanced topics";
}

std::vector<std::string> ContextEngine::getSuggestedQuestions() const
{
    std::vector<std::string> suggestions;
    auto gaps = userModel_.identifyKnowledgeGaps();
    
    for (const std::string& gapName : gaps) {
        if (gapName == "reynolds_number") {
            suggestions.push_back("How do you calculate Reynolds number for your application?");
        } else if (gapName == "boundary_conditions") {
            suggestions.push_back("What boundary conditions are appropriate for your problem?");
        } else if (gapName == "turbulence") {
            suggestions.push_back("Do you expect your flow to be turbulent or laminar?");
        }
    }
    
    return suggestions;
}

std::string ContextEngine::generatePersonalizedExplanation(const std::string& conceptId) const
{
    auto interests = userModel_.getApplicationInterests();
    std::string level = userModel_.getExperienceLevel();
    
    // Generate explanation based on user profile
    std::string explanation = "Based on your " + level + " level";
    
    if (!interests.empty()) {
        explanation += " and interest in " + interests[0];
    }
    
    explanation += ", let me explain " + conceptId + " in a way that's most relevant to you.";
    
    return explanation;
}

/*---------------------------------------------------------------------------*\
                 Intelligent Parameter Extractor Implementation
\*---------------------------------------------------------------------------*/

IntelligentParameterExtractor::IntelligentParameterExtractor()
{
    // Initialize parameter patterns for regex matching
    parameterPatterns_["velocity"] = R"((\d+\.?\d*)\s*(m/s|ms|meter.*second))";
    parameterPatterns_["pressure"] = R"((\d+\.?\d*)\s*(pa|pascal|bar|atm|psi))";
    parameterPatterns_["temperature"] = R"((\d+\.?\d*)\s*(k|kelvin|c|celsius|f|fahrenheit))";
    parameterPatterns_["density"] = R"((\d+\.?\d*)\s*(kg/m3|kg.*m.*3|density))";
    parameterPatterns_["viscosity"] = R"((\d+\.?\d*e?[+-]?\d*)\s*(pa\.?s|pas|poise|centipoise))";
    parameterPatterns_["diameter"] = R"((\d+\.?\d*)\s*(m|mm|cm|meter|diameter|pipe.*size))";
    parameterPatterns_["length"] = R"((\d+\.?\d*)\s*(m|mm|cm|meter|length|characteristic.*length))";
    
    // Initialize parameter synonyms
    parameterSynonyms_["velocity"] = {"speed", "flow rate", "inlet velocity", "bulk velocity"};
    parameterSynonyms_["pressure"] = {"static pressure", "gauge pressure", "inlet pressure"};
    parameterSynonyms_["temperature"] = {"temp", "inlet temperature", "wall temperature"};
    parameterSynonyms_["density"] = {"rho", "fluid density", "specific weight"};
    parameterSynonyms_["viscosity"] = {"mu", "dynamic viscosity", "kinematic viscosity"};
    parameterSynonyms_["diameter"] = {"d", "pipe diameter", "characteristic diameter", "hydraulic diameter"};
    
    // Initialize reasonable defaults
    defaultValues_["air_density"] = "1.225";  // kg/m³ at STP
    defaultValues_["air_viscosity"] = "1.81e-5";  // Pa·s at STP
    defaultValues_["water_density"] = "998.2";  // kg/m³ at 20°C
    defaultValues_["water_viscosity"] = "1.002e-3";  // Pa·s at 20°C
    defaultValues_["atmospheric_pressure"] = "101325";  // Pa
    defaultValues_["room_temperature"] = "293.15";  // K (20°C)
}

std::map<std::string, CFDParameter> IntelligentParameterExtractor::extractParametersFromConversation(
    const std::string& conversation)
{
    std::map<std::string, CFDParameter> extractedParams;
    
    for (const auto& pattern : parameterPatterns_) {
        const std::string& paramName = pattern.first;
        const std::string& regexPattern = pattern.second;
        
        CFDParameter param = extractSpecificParameter(paramName, conversation);
        if (!param.value.empty()) {
            extractedParams[paramName] = param;
        }
    }
    
    return extractedParams;
}

CFDParameter IntelligentParameterExtractor::extractSpecificParameter(
    const std::string& parameter,
    const std::string& conversation)
{
    CFDParameter param;
    param.name = parameter;
    param.extractionMethod = "explicit";
    param.confidence = 0.0;
    
    // Try direct pattern matching
    auto patternIt = parameterPatterns_.find(parameter);
    if (patternIt != parameterPatterns_.end()) {
        std::regex pattern(patternIt->second, std::regex_constants::icase);
        std::smatch match;
        
        if (std::regex_search(conversation, match, pattern)) {
            param.value = match[1].str();
            param.unit = match[2].str();
            param.confidence = 0.9;
            param.justification = "Extracted from explicit mention in conversation";
            return param;
        }
    }
    
    // Try synonym matching
    auto synonymIt = parameterSynonyms_.find(parameter);
    if (synonymIt != parameterSynonyms_.end()) {
        for (const std::string& synonym : synonymIt->second) {
            if (conversation.find(synonym) != std::string::npos) {
                // Found synonym but no explicit value
                param.extractionMethod = "inferred";
                param.confidence = 0.3;
                param.needsConfirmation = true;
                param.justification = "Parameter mentioned but value not specified";
                break;
            }
        }
    }
    
    // Try context inference
    if (param.value.empty()) {
        std::string inferredValue = inferParameterFromContext(parameter, conversation);
        if (!inferredValue.empty()) {
            param.value = inferredValue;
            param.extractionMethod = "inferred";
            param.confidence = 0.5;
            param.needsConfirmation = true;
            param.justification = "Inferred from context and application type";
        }
    }
    
    return param;
}

std::vector<std::string> IntelligentParameterExtractor::extractNumericValues(const std::string& text)
{
    std::vector<std::string> values;
    std::regex numberPattern(R"(\d+\.?\d*(?:e[+-]?\d+)?)");
    std::sregex_iterator iter(text.begin(), text.end(), numberPattern);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        values.push_back(iter->str());
    }
    
    return values;
}

std::vector<std::string> IntelligentParameterExtractor::extractUnits(const std::string& text)
{
    std::vector<std::string> units;
    std::regex unitPattern(R"(\b(m/s|pa|k|kg/m3|pas|m|mm|cm|bar|atm|psi|celsius|kelvin)\b)", 
                          std::regex_constants::icase);
    std::sregex_iterator iter(text.begin(), text.end(), unitPattern);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        units.push_back(iter->str());
    }
    
    return units;
}

std::string IntelligentParameterExtractor::inferParameterFromContext(
    const std::string& parameter,
    const std::string& conversation)
{
    // Convert to lowercase for case-insensitive matching
    std::string lowerConv = conversation;
    std::transform(lowerConv.begin(), lowerConv.end(), lowerConv.begin(), ::tolower);
    
    // Infer based on application context
    if (lowerConv.find("air") != std::string::npos || lowerConv.find("atmospheric") != std::string::npos) {
        if (parameter == "density") return defaultValues_["air_density"];
        if (parameter == "viscosity") return defaultValues_["air_viscosity"];
    }
    
    if (lowerConv.find("water") != std::string::npos || lowerConv.find("liquid") != std::string::npos) {
        if (parameter == "density") return defaultValues_["water_density"];
        if (parameter == "viscosity") return defaultValues_["water_viscosity"];
    }
    
    if (lowerConv.find("pipe") != std::string::npos && parameter == "diameter") {
        if (lowerConv.find("small") != std::string::npos) return "0.025";  // 25mm
        if (lowerConv.find("large") != std::string::npos) return "0.2";    // 200mm
        return "0.1";  // Default 100mm
    }
    
    return "";
}

bool IntelligentParameterExtractor::validateParameterRange(
    const std::string& parameter,
    const std::string& value)
{
    double numValue;
    try {
        numValue = std::stod(value);
    } catch (const std::exception&) {
        return false;
    }
    
    // Basic range validation
    if (parameter == "velocity" && (numValue < 0 || numValue > 1000)) return false;
    if (parameter == "pressure" && (numValue < 0 || numValue > 1e8)) return false;
    if (parameter == "temperature" && (numValue < 0 || numValue > 5000)) return false;
    if (parameter == "density" && (numValue <= 0 || numValue > 20000)) return false;
    if (parameter == "viscosity" && (numValue <= 0 || numValue > 1)) return false;
    if (parameter == "diameter" && (numValue <= 0 || numValue > 100)) return false;
    
    return true;
}

std::string IntelligentParameterExtractor::suggestReasonableDefault(
    const std::string& parameter,
    const std::string& applicationContext)
{
    std::string lowerContext = applicationContext;
    std::transform(lowerContext.begin(), lowerContext.end(), lowerContext.begin(), ::tolower);
    
    if (parameter == "velocity") {
        if (lowerContext.find("pipe") != std::string::npos) return "2.0";  // m/s
        if (lowerContext.find("external") != std::string::npos) return "20.0";  // m/s
        return "1.0";
    }
    
    if (parameter == "temperature") {
        return defaultValues_["room_temperature"];
    }
    
    if (parameter == "pressure") {
        return defaultValues_["atmospheric_pressure"];
    }
    
    return "";
}

std::string IntelligentParameterExtractor::generateParameterQuestion(
    const std::string& parameter,
    const EnhancedUserModel& userModel)
{
    std::string userLevel = userModel.getExperienceLevel();
    
    if (parameter == "velocity") {
        if (userLevel == "beginner") {
            return "What is the flow speed? You can specify it in m/s (meters per second).";
        } else {
            return "What is the characteristic velocity for your flow analysis?";
        }
    }
    
    if (parameter == "reynolds_number") {
        return "Do you know the Reynolds number for your flow, or would you like me to calculate it from the flow parameters?";
    }
    
    if (parameter == "boundary_conditions") {
        return "What type of boundary conditions do you need? For example: inlet velocity, outlet pressure, wall conditions?";
    }
    
    return "Can you specify the " + parameter + " for your CFD case?";
}

std::vector<std::string> IntelligentParameterExtractor::suggestParameterOptions(
    const std::string& parameter,
    const std::string& context)
{
    std::vector<std::string> options;
    
    if (parameter == "turbulence_model") {
        options = {"k-epsilon", "k-omega SST", "Spalart-Allmaras", "LES"};
    } else if (parameter == "solver") {
        if (context.find("multiphase") != std::string::npos) {
            options = {"interFoam", "multiphaseEulerFoam", "twoPhaseEulerFoam"};
        } else if (context.find("heat") != std::string::npos) {
            options = {"chtMultiRegionFoam", "buoyantSimpleFoam", "buoyantPimpleFoam"};
        } else {
            options = {"simpleFoam", "pimpleFoam", "pisoFoam"};
        }
    } else if (parameter == "mesh_type") {
        options = {"structured", "unstructured", "cartesian", "polyhedral"};
    }
    
    return options;
}

std::string IntelligentParameterExtractor::explainParameterSignificance(
    const std::string& parameter,
    const std::string& applicationContext)
{
    if (parameter == "reynolds_number") {
        return "Reynolds number determines whether your flow is laminar (Re < 2300) or turbulent (Re > 4000). "
               "This affects which turbulence model to use and how to set up your mesh near walls.";
    }
    
    if (parameter == "mesh_quality") {
        return "Mesh quality directly impacts solution accuracy and convergence. Poor quality meshes "
               "can lead to numerical errors and solver instability.";
    }
    
    if (parameter == "time_step") {
        return "Time step size affects solution stability and accuracy. It should satisfy the CFL condition "
               "for explicit schemes and be small enough to capture the physics of interest.";
    }
    
    return "The " + parameter + " is important for setting up your CFD simulation correctly.";
}

bool IntelligentParameterExtractor::validateParameterConsistency(
    const std::map<std::string, CFDParameter>& parameters)
{
    // Check for inconsistencies between related parameters
    auto velIt = parameters.find("velocity");
    auto reIt = parameters.find("reynolds_number");
    auto densIt = parameters.find("density");
    auto viscIt = parameters.find("viscosity");
    auto diamIt = parameters.find("diameter");
    
    if (velIt != parameters.end() && reIt != parameters.end() && 
        densIt != parameters.end() && viscIt != parameters.end() && 
        diamIt != parameters.end()) {
        
        // Calculate Reynolds number and compare
        try {
            double vel = std::stod(velIt->second.value);
            double rho = std::stod(densIt->second.value);
            double mu = std::stod(viscIt->second.value);
            double d = std::stod(diamIt->second.value);
            double re_given = std::stod(reIt->second.value);
            
            double re_calculated = (rho * vel * d) / mu;
            double tolerance = 0.1;  // 10% tolerance
            
            return std::abs(re_calculated - re_given) / re_given < tolerance;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    return true;  // No inconsistency found
}

std::vector<std::string> IntelligentParameterExtractor::identifyMissingCriticalParameters(
    const std::map<std::string, CFDParameter>& parameters,
    const std::string& analysisType)
{
    std::vector<std::string> missing;
    
    std::vector<std::string> criticalParams;
    if (analysisType == "pipe_flow") {
        criticalParams = {"velocity", "diameter", "density", "viscosity"};
    } else if (analysisType == "external_flow") {
        criticalParams = {"velocity", "characteristic_length", "density", "viscosity"};
    } else if (analysisType == "heat_transfer") {
        criticalParams = {"velocity", "temperature", "thermal_conductivity", "specific_heat"};
    } else if (analysisType == "multiphase") {
        criticalParams = {"phase1_density", "phase2_density", "surface_tension", "contact_angle"};
    }
    
    for (const std::string& param : criticalParams) {
        if (parameters.find(param) == parameters.end()) {
            missing.push_back(param);
        }
    }
    
    return missing;
}

/*---------------------------------------------------------------------------*\
                     CFD Knowledge Graph Implementation
\*---------------------------------------------------------------------------*/

CFDKnowledgeGraph::CFDKnowledgeGraph()
{
    buildCoreCFDConcepts();
}

void CFDKnowledgeGraph::buildCoreCFDConcepts()
{
    // Fundamental concepts
    CFDConcept fluidProps;
    fluidProps.conceptId = "fluid_properties";
    fluidProps.name = "Fluid Properties";
    fluidProps.description = "Physical properties that characterize a fluid";
    fluidProps.complexityLevel = 1;
    fluidProps.applications = {"all_cfd_simulations"};
    fluidProps.keyQuestions = {"What is density?", "What is viscosity?", "How do properties affect flow?"};
    addConcept(fluidProps);
    
    CFDConcept reynoldsNumber;
    reynoldsNumber.conceptId = "reynolds_number";
    reynoldsNumber.name = "Reynolds Number";
    reynoldsNumber.description = "Dimensionless parameter characterizing flow regime";
    reynoldsNumber.prerequisites = {"fluid_properties", "characteristic_length", "velocity"};
    reynoldsNumber.complexityLevel = 2;
    reynoldsNumber.applications = {"pipe_flow", "external_flow", "turbulence_analysis"};
    reynoldsNumber.commonMisconceptions = {"Higher Re always means better", "Re is the same for all geometries"};
    reynoldsNumber.keyQuestions = {"What does Re tell us?", "How to calculate Re?", "When is flow turbulent?"};
    addConcept(reynoldsNumber);
    
    CFDConcept boundaryConditions;
    boundaryConditions.conceptId = "boundary_conditions";
    boundaryConditions.name = "Boundary Conditions";
    boundaryConditions.description = "Mathematical constraints applied at domain boundaries";
    boundaryConditions.prerequisites = {"fluid_properties", "flow_physics"};
    boundaryConditions.complexityLevel = 2;
    boundaryConditions.applications = {"all_cfd_simulations"};
    boundaryConditions.keyQuestions = {"What BC for inlet?", "What BC for walls?", "What BC for outlets?"};
    addConcept(boundaryConditions);
    
    CFDConcept turbulence;
    turbulence.conceptId = "turbulence";
    turbulence.name = "Turbulence Modeling";
    turbulence.description = "Mathematical models for turbulent flow simulation";
    turbulence.prerequisites = {"reynolds_number", "boundary_layer", "navier_stokes"};
    turbulence.complexityLevel = 4;
    turbulence.applications = {"high_re_flows", "external_aerodynamics", "internal_flows"};
    turbulence.keyQuestions = {"Which turbulence model?", "What is y+?", "RANS vs LES?"};
    addConcept(turbulence);
    
    CFDConcept meshQuality;
    meshQuality.conceptId = "mesh_quality";
    meshQuality.name = "Mesh Quality";
    meshQuality.description = "Geometric quality metrics for computational grids";
    meshQuality.prerequisites = {"discretization", "numerical_methods"};
    meshQuality.complexityLevel = 3;
    meshQuality.applications = {"all_cfd_simulations"};
    meshQuality.keyQuestions = {"What is skewness?", "What is aspect ratio?", "How fine should mesh be?"};
    addConcept(meshQuality);
    
    // Set up dependencies
    addConceptDependency("fluid_properties", "reynolds_number");
    addConceptDependency("reynolds_number", "turbulence");
    addConceptDependency("boundary_conditions", "turbulence");
    addConceptDependency("mesh_quality", "turbulence");
    
    // Application domain mapping
    applicationDomains_["automotive"] = {"external_flow", "turbulence", "heat_transfer"};
    applicationDomains_["aerospace"] = {"external_flow", "compressible_flow", "turbulence"};
    applicationDomains_["hvac"] = {"internal_flow", "heat_transfer", "buoyancy"};
    applicationDomains_["marine"] = {"external_flow", "multiphase", "free_surface"};
}

void CFDKnowledgeGraph::addConcept(const CFDConcept& conceptData)
{
    concepts_[conceptData.conceptId] = conceptData;
}

void CFDKnowledgeGraph::addConceptDependency(const std::string& prerequisite, const std::string& dependent)
{
    conceptDependencies_[prerequisite].push_back(dependent);
}

std::vector<std::string> CFDKnowledgeGraph::generateLearningPath(
    const std::string& targetConcept,
    const EnhancedUserModel& userModel)
{
    std::vector<std::string> path;
    std::vector<std::string> understoodConcepts = userModel.getUnderstoodConcepts();
    
    // Find prerequisites not yet understood
    auto conceptIt = concepts_.find(targetConcept);
    if (conceptIt != concepts_.end()) {
        for (const std::string& prereq : conceptIt->second.prerequisites) {
            if (std::find(understoodConcepts.begin(), understoodConcepts.end(), prereq) 
                == understoodConcepts.end()) {
                
                // Recursively find path for prerequisites
                std::vector<std::string> prereqPath = generateLearningPath(prereq, userModel);
                path.insert(path.end(), prereqPath.begin(), prereqPath.end());
            }
        }
        
        // Add target concept if not already understood
        if (std::find(understoodConcepts.begin(), understoodConcepts.end(), targetConcept) 
            == understoodConcepts.end()) {
            path.push_back(targetConcept);
        }
    }
    
    return path;
}

std::vector<std::string> CFDKnowledgeGraph::getReadyToLearnConcepts(const EnhancedUserModel& userModel)
{
    std::vector<std::string> ready;
    std::vector<std::string> understoodConcepts = userModel.getUnderstoodConcepts();
    
    for (const auto& conceptPair : concepts_) {
        const std::string& conceptId = conceptPair.first;
        const CFDConcept& conceptData = conceptPair.second;
        
        // Skip if already understood
        if (std::find(understoodConcepts.begin(), understoodConcepts.end(), conceptId) 
            != understoodConcepts.end()) {
            continue;
        }
        
        // Check if all prerequisites are met
        if (hasPrerequisites(conceptId, userModel)) {
            ready.push_back(conceptId);
        }
    }
    
    return ready;
}

bool CFDKnowledgeGraph::hasPrerequisites(const std::string& conceptId, const EnhancedUserModel& userModel)
{
    auto conceptIt = concepts_.find(conceptId);
    if (conceptIt == concepts_.end()) {
        return false;
    }
    
    std::vector<std::string> understoodConcepts = userModel.getUnderstoodConcepts();
    
    for (const std::string& prereq : conceptIt->second.prerequisites) {
        if (std::find(understoodConcepts.begin(), understoodConcepts.end(), prereq) 
            == understoodConcepts.end()) {
            return false;
        }
    }
    
    return true;
}

double CFDKnowledgeGraph::calculateConceptReadiness(
    const std::string& conceptId,
    const EnhancedUserModel& userModel)
{
    auto conceptIt = concepts_.find(conceptId);
    if (conceptIt == concepts_.end()) {
        return 0.0;
    }
    
    const CFDConcept& conceptData = conceptIt->second;
    std::vector<std::string> understoodConcepts = userModel.getUnderstoodConcepts();
    
    if (conceptData.prerequisites.empty()) {
        return 1.0;  // No prerequisites
    }
    
    int metPrerequisites = 0;
    for (const std::string& prereq : conceptData.prerequisites) {
        if (std::find(understoodConcepts.begin(), understoodConcepts.end(), prereq) 
            != understoodConcepts.end()) {
            metPrerequisites++;
        }
    }
    
    return static_cast<double>(metPrerequisites) / conceptData.prerequisites.size();
}

std::vector<std::string> CFDKnowledgeGraph::findRelatedConcepts(const std::string& conceptId)
{
    std::vector<std::string> related;
    
    // Find concepts that depend on this one
    auto depIt = conceptDependencies_.find(conceptId);
    if (depIt != conceptDependencies_.end()) {
        related.insert(related.end(), depIt->second.begin(), depIt->second.end());
    }
    
    // Find concepts that this one depends on
    auto conceptIt = concepts_.find(conceptId);
    if (conceptIt != concepts_.end()) {
        related.insert(related.end(), 
                      conceptIt->second.prerequisites.begin(), 
                      conceptIt->second.prerequisites.end());
    }
    
    return related;
}

std::string CFDKnowledgeGraph::getConceptExplanation(
    const std::string& conceptId,
    const std::string& userLevel)
{
    auto conceptIt = concepts_.find(conceptId);
    if (conceptIt == concepts_.end()) {
        return "Concept not found in knowledge graph.";
    }
    
    const CFDConcept& conceptData = conceptIt->second;
    std::string explanation = conceptData.description;
    
    if (userLevel == "beginner") {
        explanation += " This is a ";
        explanation += (conceptData.complexityLevel <= 2 ? "fundamental" : "advanced");
        explanation += " concept in CFD.";
    } else if (userLevel == "expert") {
        explanation += " Key applications include: ";
        for (size_t i = 0; i < conceptData.applications.size(); ++i) {
            explanation += conceptData.applications[i];
            if (i < conceptData.applications.size() - 1) explanation += ", ";
        }
    }
    
    return explanation;
}

std::vector<std::string> CFDKnowledgeGraph::getRelevantConceptsForApplication(const std::string& application)
{
    auto appIt = applicationDomains_.find(application);
    return (appIt != applicationDomains_.end()) ? appIt->second : std::vector<std::string>();
}

std::string CFDKnowledgeGraph::getApplicationSpecificExplanation(
    const std::string& conceptId,
    const std::string& application)
{
    auto conceptIt = concepts_.find(conceptId);
    if (conceptIt == concepts_.end()) {
        return "Concept not found.";
    }
    
    std::string explanation = "In " + application + " applications, " + conceptId;
    
    if (conceptId == "reynolds_number" && application == "automotive") {
        explanation += " helps determine if flow around the vehicle is turbulent, "
                      "which affects drag and heat transfer calculations.";
    } else if (conceptId == "turbulence" && application == "aerospace") {
        explanation += " modeling is critical for accurate lift and drag predictions "
                      "on aircraft surfaces.";
    } else if (conceptId == "heat_transfer" && application == "hvac") {
        explanation += " calculations determine the efficiency of heating and cooling systems.";
    } else {
        explanation += " plays a key role in accurate simulation results.";
    }
    
    return explanation;
}

std::vector<std::string> CFDKnowledgeGraph::findShortestLearningPath(
    const std::string& from,
    const std::string& to)
{
    // Simple breadth-first search for shortest path
    std::vector<std::string> path;
    
    // For now, return direct dependency path
    auto conceptIt = concepts_.find(to);
    if (conceptIt != concepts_.end()) {
        path = conceptIt->second.prerequisites;
        path.push_back(to);
    }
    
    return path;
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //