/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | Context Engineering for Socratic CFD Assistant
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    Advanced context engineering system for intelligent CFD case generation
    using Socratic questioning methodology. Builds understanding progressively
    through strategic conversation flow and context-aware interactions.

\*---------------------------------------------------------------------------*/

#ifndef context_engine_H
#define context_engine_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        CFD Concept Knowledge System
\*---------------------------------------------------------------------------*/

struct CFDConcept {
    std::string conceptId;
    std::string name;
    std::string description;
    std::vector<std::string> prerequisites;
    std::vector<std::string> applications;
    int complexityLevel = 1; // 1-5 (basic to advanced)
    std::vector<std::string> commonMisconceptions;
    std::vector<std::string> keyQuestions;
};

struct UserConceptModel {
    std::string conceptId;
    bool understands = false;
    double confidenceScore = 0.0; // 0.0-1.0
    std::vector<std::string> evidenceForUnderstanding;
    std::vector<std::string> evidenceAgainstUnderstanding;
    std::chrono::time_point<std::chrono::system_clock> lastAssessed;
    int timesExplained = 0;
};

/*---------------------------------------------------------------------------*\
                        Enhanced User Model
\*---------------------------------------------------------------------------*/

class EnhancedUserModel {
private:
    std::string userId_;
    std::string experienceLevel_; // "beginner", "intermediate", "expert"
    std::string learningStyle_;   // "visual", "analytical", "practical"
    
    // CFD concept tracking
    std::map<std::string, UserConceptModel> conceptModels_;
    std::vector<std::string> confusedConcepts_;
    std::vector<std::string> strongConcepts_;
    
    // Learning patterns
    double overallConfidence_ = 0.0;
    int questionsAsked_ = 0;
    int questionsAnsweredCorrectly_ = 0;
    std::string preferredComplexityLevel_;
    
    // Context preferences
    std::map<std::string, std::string> preferences_;
    std::vector<std::string> applicationInterests_; // "automotive", "aerospace", "hvac"

    void calculateOverallConfidence();

public:
    EnhancedUserModel();
    
    // User level management
    void setExperienceLevel(const std::string& level);
    std::string getExperienceLevel() const { return experienceLevel_; }
    void setLearningStyle(const std::string& style) { learningStyle_ = style; }
    std::string getLearningStyle() const { return learningStyle_; }
    
    // Concept understanding tracking
    void updateConceptUnderstanding(const std::string& conceptId, bool understands, double confidence);
    bool isConceptUnderstood(const std::string& conceptId) const;
    double getConceptConfidence(const std::string& conceptId) const;
    std::vector<std::string> getUnderstoodConcepts() const;
    std::vector<std::string> getConfusedConcepts() const;
    
    // Learning assessment
    void recordQuestionResponse(const std::string& conceptId, bool correct);
    double getOverallConfidence() const { return overallConfidence_; }
    std::string getPreferredComplexityLevel() const;
    
    // Context analysis
    std::vector<std::string> identifyKnowledgeGaps() const;
    std::vector<std::string> getReadyForNewConcepts() const;
    bool isReadyForAdvancedTopic(const std::string& topicId) const;
    
    // Preferences and interests
    void setPreference(const std::string& key, const std::string& value);
    std::string getPreference(const std::string& key) const;
    void addApplicationInterest(const std::string& application);
    std::vector<std::string> getApplicationInterests() const { return applicationInterests_; }
};

/*---------------------------------------------------------------------------*\
                        Strategic Question Engine
\*---------------------------------------------------------------------------*/

enum class QuestionStrategy {
    CLARIFY,    // "What specifically do you mean by...?"
    EXPLORE,    // "What would happen if...?"
    CONFIRM,    // "So you're saying that...?"
    APPLY       // "How would you use this principle for...?"
};

struct SocraticQuestion {
    std::string questionText;
    QuestionStrategy strategy;
    std::string targetConcept;
    std::vector<std::string> expectedKeywords;
    std::function<bool(const std::string&)> successCriteria;
    int cognitiveLoad = 1; // 1-3
    std::vector<std::string> followUpQuestions;
};

class StrategicQuestionEngine {
private:
    std::map<std::string, std::vector<std::string>> questionTemplates_;
    std::map<std::string, CFDConcept> cfdConcepts_;
    
    // Question generation
    std::string generateClarifyingQuestion(const std::string& conceptName, const std::string& context);
    std::string generateExploringQuestion(const std::string& conceptName, const std::string& scenario);
    std::string generateConfirmingQuestion(const std::string& understanding);
    std::string generateApplicationQuestion(const std::string& conceptName, const std::string& newContext);
    
    // Context substitution
    std::string substituteContextVariables(const std::string& templateStr, 
                                         const std::map<std::string, std::string>& variables);

    void initializeCFDConcepts();

public:
    StrategicQuestionEngine();
    
    // Main question generation
    SocraticQuestion generateQuestion(QuestionStrategy strategy, 
                                    const std::string& targetConcept,
                                    const EnhancedUserModel& userModel,
                                    const std::string& conversationContext);
    
    // Strategy selection
    QuestionStrategy selectOptimalStrategy(const std::string& targetConcept,
                                         const EnhancedUserModel& userModel);
    
    // Question validation
    bool validateQuestionEffectiveness(const SocraticQuestion& question, 
                                     const std::string& userResponse);
    
    // CFD-specific questions
    std::vector<SocraticQuestion> generateReynoldsNumberQuestions(const EnhancedUserModel& userModel);
    std::vector<SocraticQuestion> generateBoundaryConditionQuestions(const std::string& geometryType);
    std::vector<SocraticQuestion> generateTurbulenceQuestions(const EnhancedUserModel& userModel);
};

/*---------------------------------------------------------------------------*\
                        Intelligent Parameter Extractor
\*---------------------------------------------------------------------------*/

struct CFDParameter {
    std::string name;
    std::string value;
    std::string unit;
    std::string extractionMethod; // "explicit", "inferred", "default"
    double confidence = 0.0;
    std::string justification;
    bool needsConfirmation = false;
};

class IntelligentParameterExtractor {
private:
    std::map<std::string, std::string> parameterPatterns_;
    std::map<std::string, std::vector<std::string>> parameterSynonyms_;
    std::map<std::string, std::string> defaultValues_;
    
    // Natural language processing
    std::vector<std::string> extractNumericValues(const std::string& text);
    std::vector<std::string> extractUnits(const std::string& text);
    std::string inferParameterFromContext(const std::string& parameter, 
                                        const std::string& conversation);
    
    // Parameter validation
    bool validateParameterRange(const std::string& parameter, const std::string& value);
    std::string suggestReasonableDefault(const std::string& parameter, 
                                       const std::string& applicationContext);

public:
    IntelligentParameterExtractor();
    
    // Main extraction methods
    std::map<std::string, CFDParameter> extractParametersFromConversation(
        const std::string& conversation);
    
    CFDParameter extractSpecificParameter(const std::string& parameter, 
                                        const std::string& conversation);
    
    // Parameter suggestions
    std::string generateParameterQuestion(const std::string& parameter, 
                                        const EnhancedUserModel& userModel);
    
    std::vector<std::string> suggestParameterOptions(const std::string& parameter,
                                                   const std::string& context);
    
    std::string explainParameterSignificance(const std::string& parameter,
                                           const std::string& applicationContext);
    
    // Validation and consistency
    bool validateParameterConsistency(const std::map<std::string, CFDParameter>& parameters);
    std::vector<std::string> identifyMissingCriticalParameters(
        const std::map<std::string, CFDParameter>& parameters,
        const std::string& analysisType);
};

/*---------------------------------------------------------------------------*\
                        CFD Knowledge Graph
\*---------------------------------------------------------------------------*/

class CFDKnowledgeGraph {
private:
    std::map<std::string, CFDConcept> concepts_;
    std::map<std::string, std::vector<std::string>> conceptDependencies_;
    std::map<std::string, std::vector<std::string>> applicationDomains_;
    
    // Graph traversal
    std::vector<std::string> findShortestLearningPath(const std::string& from, 
                                                     const std::string& to);
    bool hasPrerequisites(const std::string& conceptId, 
                         const EnhancedUserModel& userModel);

public:
    CFDKnowledgeGraph();
    
    // Knowledge graph construction
    void buildCoreCFDConcepts();
    void addConcept(const CFDConcept& conceptData);
    void addConceptDependency(const std::string& prerequisite, const std::string& dependent);
    
    // Learning path generation
    std::vector<std::string> generateLearningPath(const std::string& targetConcept,
                                                 const EnhancedUserModel& userModel);
    
    std::vector<std::string> getReadyToLearnConcepts(const EnhancedUserModel& userModel);
    
    // Concept analysis
    double calculateConceptReadiness(const std::string& conceptId,
                                   const EnhancedUserModel& userModel);
    
    std::vector<std::string> findRelatedConcepts(const std::string& conceptId);
    std::string getConceptExplanation(const std::string& conceptId, 
                                    const std::string& userLevel);
    
    // Application context
    std::vector<std::string> getRelevantConceptsForApplication(const std::string& application);
    std::string getApplicationSpecificExplanation(const std::string& conceptId,
                                                 const std::string& application);
};

/*---------------------------------------------------------------------------*\
                        Main Context Engine
\*---------------------------------------------------------------------------*/

class ContextEngine {
private:
    EnhancedUserModel userModel_;
    StrategicQuestionEngine questionEngine_;
    IntelligentParameterExtractor parameterExtractor_;
    CFDKnowledgeGraph knowledgeGraph_;
    
    // Conversation state
    std::string currentTopic_;
    std::vector<std::string> conversationHistory_;
    std::map<std::string, std::string> conversationContext_;
    std::vector<std::string> extractedFacts_;
    
    // Learning assessment
    void assessUserResponseForConcepts(const std::string& response);
    void updateUserModelFromResponse(const std::string& response);
    void identifyConfusionPatterns(const std::string& response);

public:
    ContextEngine();
    
    // Main interface
    std::string processUserInput(const std::string& input);
    std::string generateNextQuestion();
    
    // User model access
    void setUserExperienceLevel(const std::string& level);
    EnhancedUserModel& getUserModel() { return userModel_; }
    const EnhancedUserModel& getUserModel() const { return userModel_; }
    
    // Context management
    void updateConversationContext(const std::string& key, const std::string& value);
    std::string getConversationSummary() const;
    std::vector<std::string> getExtractedParameters() const;
    
    // Learning assessment
    bool isUserReadyForCaseGeneration() const;
    std::vector<std::string> getRequiredConceptsForCase(const std::string& caseType) const;
    double getOverallLearningProgress() const;
    
    // Strategic guidance
    std::string getNextLearningObjective() const;
    std::vector<std::string> getSuggestedQuestions() const;
    std::string generatePersonalizedExplanation(const std::string& conceptId) const;
};

}  // End namespace MCP
}  // End namespace Foam

#endif

// ************************************************************************* //