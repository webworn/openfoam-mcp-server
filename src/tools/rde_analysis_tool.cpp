#include "rde_analysis_tool.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>

RDEAnalysisTool::RDEAnalysisTool() 
    : expertWorkflow_(std::make_unique<RDEExpertWorkflow>())
    , contextEngine_(std::make_unique<ContextEngine>()) {
}

json RDEAnalysisTool::getInputSchema() {
    return json{
        {"type", "object"},
        {"properties", {
            {"outer_radius", {
                {"type", "number"},
                {"minimum", 0.01},
                {"maximum", 0.5},
                {"description", "Outer radius of annular combustor [m]"}
            }},
                {"inner_radius", {
                    {"type", "number"},
                    {"minimum", 0.005},
                    {"maximum", 0.4},
                    {"description", "Inner radius of annular combustor [m]"}
                }},
                {"chamber_length", {
                    {"type", "number"},
                    {"minimum", 0.01},
                    {"maximum", 1.0},
                    {"description", "Axial length of combustion chamber [m]"}
                }},
                {"fuel_type", {
                    {"type", "string"},
                    {"enum", json::array({"hydrogen", "methane", "propane", "kerosene"})},
                    {"description", "Fuel type for combustion analysis"}
                }},
                {"oxidizer_type", {
                    {"type", "string"},
                    {"enum", json::array({"air", "oxygen"})},
                    {"description", "Oxidizer type"}
                }},
                {"equivalence_ratio", {
                    {"type", "number"},
                    {"minimum", 0.5},
                    {"maximum", 2.0},
                    {"description", "Fuel-air equivalence ratio (phi)"}
                }},
                {"chamber_pressure", {
                    {"type", "number"},
                    {"minimum", 50000},
                    {"maximum", 5000000},
                    {"description", "Chamber operating pressure [Pa]"}
                }},
                {"injection_velocity", {
                    {"type", "number"},
                    {"minimum", 10},
                    {"maximum", 500},
                    {"description", "Fuel/air injection velocity [m/s]"}
                }},
                {"analysis_type", {
                    {"type", "string"},
                    {"enum", json::array({"design", "performance", "stability", "optimization"})},
                    {"description", "Type of RDE analysis to perform"}
                }},
                {"case_directory", {
                    {"type", "string"},
                    {"description", "Directory for OpenFOAM case files"}
                }},
                {"solver_type", {
                    {"type", "string"},
                    {"enum", json::array({"rhoCentralFoam", "BYCFoam", "rhoReactingFoam"})},
                    {"description", "OpenFOAM solver for combustion simulation"}
                }},
                {"mesh_resolution", {
                    {"type", "integer"},
                    {"minimum", 1},
                    {"maximum", 3},
                    {"description", "Mesh resolution: 1=coarse, 2=medium, 3=fine"}
                }},
                {"enable_detailed_chemistry", {
                    {"type", "boolean"},
                    {"description", "Enable detailed chemical kinetics modeling"}
                }},
                {"enable_visualization", {
                    {"type", "boolean"},
                    {"description", "Generate visualization output files"}
                }}
            }},
        {"required", json::array({"outer_radius", "inner_radius", "chamber_length", "fuel_type", "analysis_type", "case_directory"})}
    };
}

ToolResult RDEAnalysisTool::execute(const json& arguments) {
    ToolResult result;
    
    try {
        // Use the existing analysis logic
        auto analysisResult = handleRequest(arguments);
        
        // Convert to ToolResult format
        result.addTextContent(analysisResult.dump(2));
        
    } catch (const std::exception& e) {
        result.addErrorContent(std::string("RDE analysis failed: ") + e.what());
    }
    
    return result;
}

json RDEAnalysisTool::handleRequest(const json& request) {
    try {
        // Extract parameters from request
        RDEAnalysisRequest rdeRequest;
        rdeRequest.outerRadius = request["outer_radius"];
        rdeRequest.innerRadius = request["inner_radius"];
        rdeRequest.chamberLength = request["chamber_length"];
        rdeRequest.fuelType = request["fuel_type"];
        rdeRequest.oxidizerType = request.value("oxidizer_type", "air");
        rdeRequest.equivalenceRatio = request.value("equivalence_ratio", 1.0);
        rdeRequest.chamberPressure = request.value("chamber_pressure", 101325.0);
        rdeRequest.injectionTemperature = request.value("injection_temperature", 300.0);
        rdeRequest.injectionVelocity = request.value("injection_velocity", 100.0);
        rdeRequest.analysisType = request["analysis_type"];
        rdeRequest.caseDirectory = request["case_directory"];
        rdeRequest.solverType = request.value("solver_type", "rhoCentralFoam");
        rdeRequest.numberOfInjectors = request.value("number_of_injectors", 12);
        rdeRequest.injectionType = request.value("injection_type", "axial");
        rdeRequest.enableDetailedChemistry = request.value("enable_detailed_chemistry", true);
        rdeRequest.enableVisualization = request.value("enable_visualization", true);
        rdeRequest.meshResolution = request.value("mesh_resolution", 2);
        
        // Perform RDE analysis
        auto result = analyzeRDE(rdeRequest);
        
        // Format comprehensive response
        json response = {
            {"success", result.success},
            {"analysis_type", result.analysisType},
            {"detonation_velocity", result.detonationVelocity},
            {"wave_frequency", result.waveFrequency},
            {"number_of_waves", result.numberOfWaves},
            {"specific_impulse", result.specificImpulse},
            {"thrust", result.thrust},
            {"pressure_gain", result.pressureGain},
            {"combustion_efficiency", result.combustionEfficiency},
            {"mass_flow_rate", result.massFlowRate},
            {"pressure_oscillations", result.pressureOscillations},
            {"theoretical_accuracy", result.theoreticalAccuracy},
            {"physics_explanation", result.physicsExplanation},
            {"detonation_theory", result.detonationTheory},
            {"design_guidance", result.designGuidance},
            {"key_insights", result.keyInsights},
            {"recommendations", result.recommendations},
            {"concept_questions", result.conceptQuestions},
            {"design_questions", result.designQuestions},
            {"physics_questions", result.physicsQuestions}
        };
        
        if (result.enableVisualization) {
            response["visualization"] = {
                {"pressure_contours", result.pressureContours},
                {"temperature_contours", result.temperatureContours},
                {"velocity_vectors", result.velocityVectors},
                {"wave_animation", result.waveAnimation}
            };
        }
        
        if (!result.validationWarnings.empty()) {
            response["validation_warnings"] = result.validationWarnings;
        }
        
        return response;
        
    } catch (const std::exception& e) {
        return json{
            {"success", false},
            {"error", std::string("RDE analysis failed: ") + e.what()},
            {"educational_guidance", "This error suggests an issue with RDE parameters. Let's systematically check: 1) Are geometry dimensions physically reasonable? 2) Is the equivalence ratio within combustible limits? 3) Are operating conditions realistic for detonation propagation?"}
        };
    }
}

RDEAnalysisTool::RDEAnalysisResult RDEAnalysisTool::analyzeRDE(const RDEAnalysisRequest& request) {
    RDEAnalysisResult result;
    result.success = false;
    result.analysisType = request.analysisType;
    
    // Phase 1: Input validation with educational feedback
    std::vector<std::string> validationWarnings;
    if (!validateInputParameters(request, validationWarnings)) {
        result.validationWarnings = validationWarnings;
        result.troubleshooting = generateTroubleshootingGuide(validationWarnings);
        return result;
    }
    
    // Phase 2: Configure RDE expert workflow request
    RDEExpertWorkflow::ExpertAnalysisRequest expertRequest;
    
    // Analysis configuration
    expertRequest.analysisGoal = request.analysisType;
    expertRequest.applicationDomain = "propulsion"; // Default for MCP interface
    expertRequest.targetThrust = 1000.0; // Default target
    expertRequest.performancePriority = "efficiency";
    
    // Operating constraints
    expertRequest.maxPressure = request.chamberPressure * 2.0; // Safety margin
    expertRequest.maxTemperature = 3000.0; // High temperature limit
    expertRequest.maxMassFlowRate = 1.0; // 1 kg/s limit
    expertRequest.availableFuels = {request.fuelType};
    expertRequest.oxidizerConstraint = (request.oxidizerType == "oxygen") ? "oxygen_available" : "air_only";
    
    // Design constraints
    expertRequest.maxOuterDiameter = request.outerRadius * 2.0;
    expertRequest.lengthConstraint = request.chamberLength * 2.0;
    expertRequest.materialConstraint = "inconel";
    expertRequest.coolingRequired = true;
    
    // User preferences
    expertRequest.userExperience = "intermediate"; // Default for MCP users
    expertRequest.requestDetailedExplanations = true;
    expertRequest.requestDesignRationale = true;
    expertRequest.preferredUnits = "SI";
    
    // Phase 3: Run RDE expert analysis
    auto expertResult = expertWorkflow_->performExpertAnalysis(expertRequest);
    
    if (!expertResult.feasibilityAssessment) {
        result.validationWarnings = expertResult.technicalRisks;
        result.troubleshooting = generateTroubleshootingGuide(expertResult.technicalRisks);
        return result;
    }
    
    // Phase 4: Extract results from expert system
    result.detonationVelocity = expertResult.recommendedChemistry.detonationVelocity;
    result.waveFrequency = expertResult.predictedPerformance.waveFrequency;
    result.numberOfWaves = expertResult.predictedPerformance.numberOfWaves;
    result.specificImpulse = expertResult.predictedPerformance.specificImpulse;
    result.thrust = expertResult.predictedPerformance.thrust;
    result.pressureGain = expertResult.predictedPerformance.pressureGain;
    result.combustionEfficiency = expertResult.predictedPerformance.combustionEfficiency;
    result.massFlowRate = expertResult.predictedPerformance.massFlowRate;
    result.pressureOscillations = expertResult.predictedPerformance.pressureOscillations;
    
    // Calculate additional metrics
    result.heatReleaseRate = result.massFlowRate * 120e6; // Simplified heat release [W]
    result.residenceTime = request.chamberLength / request.injectionVelocity;
    result.cellSize = expertResult.recommendedChemistry.cellSize;
    
    // Phase 5: Generate educational content using expert workflow results
    auto educationalContent = generateEducationalContent(request, result);
    result.physicsExplanation = expertResult.physicsExplanation.empty() ? 
                               educationalContent.detonationPhysics : expertResult.physicsExplanation;
    result.detonationTheory = educationalContent.chapmanJouguetTheory;
    result.designGuidance = expertResult.designPhilosophy.empty() ? 
                           generateDesignNarrative(request, result) : expertResult.designPhilosophy;
    
    // Phase 6: Generate insights and recommendations using expert workflow results
    if (!expertResult.keyDesignDecisions.empty()) {
        result.keyInsights = expertResult.keyDesignDecisions;
    } else {
        result.keyInsights = {
            "Detonation velocity is " + std::to_string((int)(result.detonationVelocity)) + " m/s, which is " + 
            (result.detonationVelocity > 1800 ? "optimal" : "low") + " for " + request.fuelType + " combustion",
            
            "Pressure gain of " + std::to_string(result.pressureGain) + "x enables higher cycle efficiency than conventional gas turbines",
            
            "Wave frequency of " + std::to_string((int)result.waveFrequency) + " Hz creates " + 
            (result.waveFrequency > 10000 ? "high" : "moderate") + " frequency pressure oscillations",
            
            "Combustion efficiency of " + std::to_string((int)(result.combustionEfficiency * 100)) + "% indicates " +
            (result.combustionEfficiency > 0.9 ? "excellent" : result.combustionEfficiency > 0.8 ? "good" : "poor") + " fuel utilization"
        };
    }
    
    // Generate recommendations using expert workflow results
    if (!expertResult.recommendedNextSteps.empty()) {
        result.recommendations = expertResult.recommendedNextSteps;
    } else {
        // Fallback recommendations
        if (result.combustionEfficiency < 0.85) {
            result.recommendations.push_back("Consider increasing residence time or injection velocity for better mixing");
        }
        if (result.pressureOscillations > 0.15) {
            result.recommendations.push_back("High pressure oscillations detected - review injection timing and geometry");
        }
        if (result.pressureGain < 2.0) {
            result.recommendations.push_back("Low pressure gain suggests weak detonation - check equivalence ratio and chamber pressure");
        }
        if (request.equivalenceRatio > 1.3) {
            result.recommendations.push_back("Rich mixture increases heat transfer and emissions - consider leaner operation");
        }
    }
    
    // Phase 7: Generate Socratic questions for different learning aspects
    result.conceptQuestions = generateSocraticQuestions("concepts", request);
    result.designQuestions = generateSocraticQuestions("design", request);
    result.physicsQuestions = generateSocraticQuestions("physics", request);
    
    // Phase 8: Validation using expert workflow confidence scores
    result.theoreticalAccuracy = expertResult.confidenceScore;
    
    // Phase 9: Visualization file paths
    result.enableVisualization = request.enableVisualization;
    if (request.enableVisualization) {
        result.pressureContours = request.caseDirectory + "/postProcessing/pressure_contours.png";
        result.temperatureContours = request.caseDirectory + "/postProcessing/temperature_contours.png";
        result.velocityVectors = request.caseDirectory + "/postProcessing/velocity_vectors.png";
        result.waveAnimation = request.caseDirectory + "/postProcessing/wave_animation.gif";
    }
    
    result.success = true;
    return result;
}

RDEAnalysisTool::RDEEducationalContent RDEAnalysisTool::generateEducationalContent(
    const RDEAnalysisRequest& request, const RDEAnalysisResult& result) {
    
    RDEEducationalContent content;
    
    // Detonation physics explanation
    content.detonationPhysics = explainDetonationPhenomena(request.fuelType, request.equivalenceRatio);
    
    // Chapman-Jouguet theory
    content.chapmanJouguetTheory = explainChapmanJouguetTheory(result.detonationVelocity, result.pressureGain);
    
    // Wave dynamics
    content.waveDynamics = explainWavePropagation(result.numberOfWaves, result.waveFrequency);
    
    // Combustion kinetics
    content.combustionKinetics = explainCombustionKinetics(request.fuelType);
    
    // Performance metrics
    content.performanceMetrics = explainSpecificImpulse(result.specificImpulse, request.fuelType) + "\\n\\n" +
                                explainCombustionEfficiency(result.combustionEfficiency, result.residenceTime);
    
    // Design principles
    content.designPrinciples = recommendGeometry(request.outerRadius, request.innerRadius, request.chamberLength) + "\\n\\n" +
                              recommendInjection(request.injectionType, request.injectionVelocity, request.numberOfInjectors);
    
    return content;
}

std::string RDEAnalysisTool::explainDetonationPhenomena(const std::string& fuelType, double phi) {
    std::ostringstream explanation;
    
    explanation << "**Detonation Physics Fundamentals:**\\n\\n";
    explanation << "Detonation is a supersonic combustion wave where the flame front is coupled with a shock wave. ";
    explanation << "This creates a self-sustaining wave that propagates at velocities of 1500-2500 m/s, much faster than ";
    explanation << "deflagration flames (1-10 m/s).\\n\\n";
    
    explanation << "**" << fuelType << " Detonation Characteristics:**\\n";
    if (fuelType == "hydrogen") {
        explanation << "Hydrogen has the fastest detonation velocity (~1970 m/s) and lowest ignition energy. ";
        explanation << "It produces only water vapor, making it environmentally clean. ";
        explanation << "The wide flammability limits (4-75% in air) enable stable detonation across varied equivalence ratios.";
    } else if (fuelType == "methane") {
        explanation << "Methane detonation velocity (~1800 m/s) is lower than hydrogen but still provides excellent pressure gain. ";
        explanation << "It's widely available and has good storage characteristics. ";
        explanation << "Methane produces CO₂ and H₂O, with moderate emissions compared to heavier hydrocarbons.";
    } else if (fuelType == "propane") {
        explanation << "Propane offers good detonation characteristics (~1850 m/s) with excellent energy density. ";
        explanation << "It's easily liquefied for storage and handling. ";
        explanation << "The higher carbon content increases heat release but also emissions.";
    }
    
    explanation << "\\n\\nAt φ=" << phi << ", ";
    if (phi < 0.8) {
        explanation << "the lean mixture may have weaker detonations but lower heat transfer to walls.";
    } else if (phi > 1.2) {
        explanation << "the rich mixture provides strong detonations but increases heat transfer and emissions.";
    } else {
        explanation << "the near-stoichiometric mixture optimizes detonation strength and efficiency.";
    }
    
    return explanation.str();
}

std::string RDEAnalysisTool::explainChapmanJouguetTheory(double cjVelocity, double pressureGain) {
    std::ostringstream explanation;
    
    explanation << "**Chapman-Jouguet (C-J) Theory:**\\n\\n";
    explanation << "C-J theory predicts the minimum stable detonation velocity by balancing chemical energy release ";
    explanation << "with fluid dynamic constraints. The C-J state represents the sonic point in the reaction products ";
    explanation << "frame of reference.\\n\\n";
    
    explanation << "**Key C-J Relations:**\\n";
    explanation << "• Detonation velocity: D = " << (int)cjVelocity << " m/s\\n";
    explanation << "• Pressure gain: P₂/P₁ = " << std::fixed << std::setprecision(1) << pressureGain << "\\n";
    explanation << "• The detonation wave couples shock compression with heat release\\n";
    explanation << "• Products exit at sonic velocity relative to the wave\\n\\n";
    
    explanation << "**Physical Significance:**\\n";
    explanation << "The C-J velocity represents the minimum speed for self-sustaining detonation. ";
    explanation << "Actual RDE waves typically propagate at 80-90% of C-J velocity due to losses from ";
    explanation << "heat transfer, viscosity, and geometric effects. ";
    
    if (pressureGain > 15.0) {
        explanation << "Your high pressure gain indicates strong detonation with excellent pressure rise.";
    } else if (pressureGain < 10.0) {
        explanation << "Lower pressure gain suggests weak detonation - check fuel-air mixing and ignition energy.";
    } else {
        explanation << "Moderate pressure gain is typical for practical RDE operation.";
    }
    
    return explanation.str();
}

std::string RDEAnalysisTool::explainWavePropagation(int numberOfWaves, double frequency) {
    std::ostringstream explanation;
    
    explanation << "**Detonation Wave Propagation:**\\n\\n";
    explanation << "Wave Configuration: " << numberOfWaves << " wave(s)\\n";
    explanation << "Propagation Frequency: " << (int)frequency << " Hz\\n\\n";
    
    if (numberOfWaves == 1) {
        explanation << "**Single Wave Mode:**\\n";
        explanation << "One detonation wave travels continuously around the annulus. ";
        explanation << "This is the most stable configuration with predictable pressure oscillations. ";
        explanation << "The wave creates a triangular 'refill zone' behind it where fresh reactants are injected.";
    } else if (numberOfWaves == 2) {
        explanation << "**Dual Wave Mode:**\\n";
        explanation << "Two waves can propagate either co-rotating (same direction) or counter-rotating. ";
        explanation << "Counter-rotating waves provide higher thrust but increased complexity. ";
        explanation << "Wave collisions create intense pressure spikes and heat release.";
    } else {
        explanation << "**Multi-Wave Mode:**\\n";
        explanation << "Multiple waves create complex interference patterns. ";
        explanation << "This can increase specific impulse but may cause structural challenges. ";
        explanation << "Wave stability becomes more difficult to maintain.";
    }
    
    explanation << "\\n\\n**Frequency Effects:**\\n";
    explanation << "High frequency (" << (int)frequency << " Hz) creates rapid pressure cycling. ";
    if (frequency > 10000) {
        explanation << "This very high frequency may cause fatigue issues but enables compact designs.";
    } else if (frequency < 1000) {
        explanation << "Lower frequency reduces structural stress but may limit performance.";
    } else {
        explanation << "Moderate frequency balances performance with structural considerations.";
    }
    
    return explanation.str();
}

std::vector<std::string> RDEAnalysisTool::generateSocraticQuestions(const std::string& category, const RDEAnalysisRequest& request) {
    std::vector<std::string> questions;
    
    if (category == "concepts") {
        questions = {
            "What fundamental difference between detonation and deflagration enables RDEs to achieve pressure gain?",
            "How does the Chapman-Jouguet velocity relate to the speed of sound in the reaction products?",
            "Why does " + request.fuelType + " have different detonation characteristics compared to other fuels?",
            "What role does the equivalence ratio φ=" + std::to_string(request.equivalenceRatio) + " play in detonation strength?"
        };
    } else if (category == "design") {
        questions = {
            "How would changing the annular gap from " + std::to_string((request.outerRadius - request.innerRadius) * 1000) + "mm affect wave stability?",
            "What injection velocity would optimize mixing without excessive pressure losses?",
            "Why is the chamber length of " + std::to_string(request.chamberLength * 1000) + "mm important for combustion completion?",
            "How would cooling requirements change if you increased the chamber pressure?"
        };
    } else if (category == "physics") {
        questions = {
            "What physical mechanisms couple the shock wave with the chemical reaction zone?",
            "How do pressure oscillations from detonation waves differ from turbine blade passing frequencies?",
            "What heat transfer mechanisms dominate in the detonation wave structure?",
            "How would wall heat transfer affect the detonation propagation speed?"
        };
    }
    
    return questions;
}

std::string RDEAnalysisTool::explainCombustionKinetics(const std::string& fuelType) {
    std::ostringstream explanation;
    explanation << "**Combustion Kinetics for " << fuelType << ":**\\n\\n";
    explanation << "Chemical reaction mechanisms govern the rate of fuel consumption and heat release. ";
    
    if (fuelType == "hydrogen") {
        explanation << "Hydrogen combustion involves relatively simple reaction pathways with fast kinetics, ";
        explanation << "enabling rapid detonation initiation and propagation.";
    } else if (fuelType == "methane") {
        explanation << "Methane requires multi-step reaction mechanisms with intermediate species formation, ";
        explanation << "affecting ignition delay and flame speeds.";
    } else {
        explanation << "Hydrocarbon fuels involve complex reaction networks with multiple intermediate species.";
    }
    
    return explanation.str();
}

std::string RDEAnalysisTool::explainSpecificImpulse(double isp, const std::string& fuelType) {
    std::ostringstream explanation;
    explanation << "**Specific Impulse Analysis:**\\n\\n";
    explanation << "Calculated specific impulse: " << (int)isp << " seconds\\n\\n";
    explanation << "This represents the thrust produced per unit weight of propellant consumed. ";
    explanation << "Higher values indicate more efficient propellant utilization. ";
    explanation << "For " << fuelType << ", this value is ";
    
    if (isp > 300) {
        explanation << "excellent, approaching theoretical limits.";
    } else if (isp > 250) {
        explanation << "good for practical propulsion applications.";
    } else {
        explanation << "moderate, with room for optimization.";
    }
    
    return explanation.str();
}

std::string RDEAnalysisTool::explainCombustionEfficiency(double efficiency, double residenceTime) {
    std::ostringstream explanation;
    explanation << "**Combustion Efficiency Analysis:**\\n\\n";
    explanation << "Efficiency: " << (int)(efficiency * 100) << "%\\n";
    explanation << "Residence time: " << std::fixed << std::setprecision(3) << residenceTime * 1000 << " ms\\n\\n";
    
    explanation << "Combustion efficiency indicates the fraction of fuel successfully consumed. ";
    explanation << "The residence time of " << (residenceTime * 1000) << " ms ";
    
    if (residenceTime > 0.001) {
        explanation << "provides sufficient time for complete combustion.";
    } else {
        explanation << "may be too short for complete fuel consumption.";
    }
    
    return explanation.str();
}

std::string RDEAnalysisTool::recommendGeometry(double outerRadius, double innerRadius, double chamberLength) {
    std::ostringstream recommendation;
    double annularGap = outerRadius - innerRadius;
    
    recommendation << "**Geometry Optimization:**\\n\\n";
    recommendation << "Current annular gap: " << (annularGap * 1000) << " mm\\n";
    recommendation << "Chamber length: " << (chamberLength * 1000) << " mm\\n\\n";
    
    if (annularGap < 0.01) {
        recommendation << "⚠️  Narrow gap may limit wave stability - consider increasing to 10-30mm.";
    } else if (annularGap > 0.05) {
        recommendation << "⚠️  Wide gap may cause wave decay - consider reducing for better confinement.";
    } else {
        recommendation << "✅ Gap width is well-suited for stable detonation propagation.";
    }
    
    return recommendation.str();
}

std::string RDEAnalysisTool::recommendInjection(const std::string& injectionType, double injectionVelocity, int numberOfInjectors) {
    std::ostringstream recommendation;
    recommendation << "**Injection System Optimization:**\\n\\n";
    recommendation << "Type: " << injectionType << " injection\\n";
    recommendation << "Velocity: " << (int)injectionVelocity << " m/s\\n";
    recommendation << "Number of injectors: " << numberOfInjectors << "\\n\\n";
    
    if (injectionVelocity < 50) {
        recommendation << "⚠️  Low injection velocity may cause poor mixing.";
    } else if (injectionVelocity > 200) {
        recommendation << "⚠️  High injection velocity increases pressure losses.";
    } else {
        recommendation << "✅ Injection velocity is well-balanced for mixing and efficiency.";
    }
    
    return recommendation.str();
}

std::string RDEAnalysisTool::generateDesignNarrative(const RDEAnalysisRequest& request, const RDEAnalysisResult& result) {
    std::ostringstream narrative;
    narrative << "**RDE Design Philosophy:**\\n\\n";
    narrative << "This " << request.fuelType << "-fueled RDE design achieves ";
    narrative << (int)(result.combustionEfficiency * 100) << "% combustion efficiency ";
    narrative << "with " << (int)result.detonationVelocity << " m/s detonation velocity. ";
    
    narrative << "The annular geometry promotes continuous detonation propagation while ";
    narrative << "the injection system ensures proper fuel-air mixing. ";
    
    if (result.pressureGain > 2.5) {
        narrative << "Excellent pressure gain characteristics make this design suitable for high-performance applications.";
    } else {
        narrative << "Moderate pressure gain suggests opportunities for further optimization.";
    }
    
    return narrative.str();
}

bool RDEAnalysisTool::validateInputParameters(const RDEAnalysisRequest& request, std::vector<std::string>& warnings) {
    bool isValid = true;
    
    // Geometry validation
    if (request.outerRadius <= request.innerRadius) {
        warnings.push_back("Outer radius must be greater than inner radius");
        isValid = false;
    }
    
    double annularGap = request.outerRadius - request.innerRadius;
    if (annularGap < 0.005) {
        warnings.push_back("Annular gap too small - may prevent stable wave propagation");
    }
    if (annularGap > 0.1) {
        warnings.push_back("Large annular gap may cause wave decay - consider optimization");
    }
    
    // Chemistry validation
    if (request.equivalenceRatio < 0.5 || request.equivalenceRatio > 2.0) {
        warnings.push_back("Equivalence ratio outside typical detonable range");
        isValid = false;
    }
    
    if (request.chamberPressure < 50000 || request.chamberPressure > 5000000) {
        warnings.push_back("Chamber pressure outside practical RDE operating range");
    }
    
    // Injection validation
    if (request.injectionVelocity < 10 || request.injectionVelocity > 500) {
        warnings.push_back("Injection velocity outside typical range for effective mixing");
    }
    
    return isValid;
}

std::string RDEAnalysisTool::generateTroubleshootingGuide(const std::vector<std::string>& warnings) {
    if (warnings.empty()) {
        return "No issues detected - RDE parameters are within acceptable ranges.";
    }
    
    std::ostringstream guide;
    guide << "**RDE Troubleshooting Guide:**\\n\\n";
    
    for (const auto& warning : warnings) {
        guide << "⚠️ " << warning << "\\n";
        
        if (warning.find("radius") != std::string::npos) {
            guide << "   → Check annular combustor geometry - gap width affects wave stability\\n";
        } else if (warning.find("equivalence ratio") != std::string::npos) {
            guide << "   → Adjust fuel-air ratio to detonable limits (typically 0.7-1.5)\\n";
        } else if (warning.find("pressure") != std::string::npos) {
            guide << "   → Review operating pressure for realistic RDE conditions\\n";
        } else if (warning.find("injection") != std::string::npos) {
            guide << "   → Optimize injection velocity for mixing vs. pressure loss\\n";
        }
        guide << "\\n";
    }
    
    guide << "**General RDE Design Principles:**\\n";
    guide << "• Annular gap: 5-50mm for most applications\\n";
    guide << "• Equivalence ratio: 0.8-1.2 for optimal performance\\n";
    guide << "• Injection velocity: 50-200 m/s for good mixing\\n";
    guide << "• Chamber pressure: 1-50 bar for practical operation\\n";
    
    return guide.str();
}

void registerRDEAnalysisTool(McpServer& server) {
    auto tool = std::make_shared<RDEAnalysisTool>();

    server.registerTool(
        RDEAnalysisTool::getName(), 
        RDEAnalysisTool::getDescription(), 
        RDEAnalysisTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult { 
            return tool->execute(arguments); 
        }
    );
}

// C interface for MCP integration
extern "C" {
    RDEAnalysisTool* createRDEAnalysisTool() {
        return new RDEAnalysisTool();
    }
    
    void destroyRDEAnalysisTool(RDEAnalysisTool* tool) {
        delete tool;
    }
}