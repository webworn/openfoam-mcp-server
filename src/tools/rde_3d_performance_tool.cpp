/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM MCP Server
   \\    /   O peration     | RDE 3D Performance Calculator MCP Tool
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
Description
    MCP tool wrapper for 3D RDE performance calculation implementation.

\*---------------------------------------------------------------------------*/

#include "rde_3d_performance_tool.hpp"

#include <iomanip>
#include <sstream>
#include <cmath>

namespace Foam {
namespace MCP {

/*---------------------------------------------------------------------------*\
                        RDE3DPerformanceTool Implementation
\*---------------------------------------------------------------------------*/

RDE3DPerformanceTool::RDE3DPerformanceTool()
    : calculator_(std::make_unique<RDE3DPerformanceCalculator>()) {}

RDE3DPerformanceTool::RDE3DPerformanceTool(std::unique_ptr<RDE3DPerformanceCalculator> calculator)
    : calculator_(std::move(calculator)) {}

ToolResult RDE3DPerformanceTool::execute(const json& arguments) {
    ToolResult result;

    try {
        std::string caseDirectory = arguments.at("case_directory").get<std::string>();

        // Build performance request
        RDE3DPerformanceCalculator::Performance3DRequest request;
        request.caseDirectory = caseDirectory;

        // Parse geometry parameters
        if (arguments.contains("geometry")) {
            auto geomJson = arguments["geometry"];
            request.geometry.innerRadius = geomJson.value("inner_radius", 0.050);
            request.geometry.outerRadius = geomJson.value("outer_radius", 0.080);
            request.geometry.axialLength = geomJson.value("axial_length", 0.150);
            request.geometry.nozzleThroatDia = geomJson.value("nozzle_throat_dia", 0.060);
            request.geometry.nozzleExitDia = geomJson.value("nozzle_exit_dia", 0.090);
            request.geometry.nozzleExpansion = geomJson.value("nozzle_expansion", 2.25);
            request.geometry.outletLength = geomJson.value("outlet_length", 0.100);
        } else {
            // Use validated defaults
            request.geometry.innerRadius = 0.050;
            request.geometry.outerRadius = 0.080;
            request.geometry.axialLength = 0.150;
            request.geometry.nozzleThroatDia = 0.060;
            request.geometry.nozzleExitDia = 0.090;
            request.geometry.nozzleExpansion = 2.25;
            request.geometry.outletLength = 0.100;
        }

        // Parse analysis options
        if (arguments.contains("analysis_options")) {
            auto options = arguments["analysis_options"];
            request.calculateAxialThrust = options.value("calculate_axial_thrust", true);
            request.analyzeNozzlePerformance = options.value("analyze_nozzle_performance", true);
            request.analyzeCombustionEfficiency = options.value("analyze_combustion_efficiency", true);
            request.includeViscousEffects = options.value("include_viscous_effects", false);
            request.validateAgainst2D = options.value("validate_against_2d", true);
        } else {
            request.calculateAxialThrust = true;
            request.analyzeNozzlePerformance = true;
            request.analyzeCombustionEfficiency = true;
            request.includeViscousEffects = false;
            request.validateAgainst2D = true;
        }

        // Set validation targets from 2D baseline
        request.targetThrust = 11519.0;
        request.targetIsp = 1629.0;
        request.targetEfficiency = 85.0;
        request.generateEducationalContent = true;

        // Calculate 3D performance
        RDE3DPerformanceCalculator::Performance3DMetrics metrics = calculator_->calculatePerformance3D(request);

        // Format results for user
        std::string formattedResults = formatResultsForUser(metrics);
        result.addTextContent(formattedResults);

        // Add thrust breakdown
        std::string thrustBreakdown = generateThrustBreakdown(metrics);
        result.addTextContent(thrustBreakdown);

        // Add educational content
        std::string educational = generateEducationalContent(metrics);
        result.addTextContent(educational);

        // Add optimization guidance
        std::string optimization = generateOptimizationGuidance(metrics);
        result.addTextContent(optimization);

        // Add JSON resource with detailed data
        json metricsJson;

        // Overall performance
        metricsJson["overall_performance_score"] = metrics.overallPerformanceScore;
        metricsJson["performance_rating"] = metrics.performanceRating;
        metricsJson["power_density"] = metrics.powerDensity;
        metricsJson["thrust_to_weight"] = metrics.thrustToWeight;
        metricsJson["overall_3d_vs_2d_ratio"] = metrics.overall3DvsSDRatio;

        // Thrust analysis
        json thrustJson;
        thrustJson["total_thrust"] = metrics.thrustAnalysis.totalThrust;
        thrustJson["combustor_thrust"] = metrics.thrustAnalysis.combustorThrust;
        thrustJson["nozzle_thrust"] = metrics.thrustAnalysis.nozzleThrust;
        thrustJson["axial_thrust"] = metrics.thrustAnalysis.axialThrust;
        thrustJson["radial_thrust_loss"] = metrics.thrustAnalysis.radialThrustLoss;
        thrustJson["thrust_vector_angle"] = metrics.thrustAnalysis.thrustVectorAngle;
        thrustJson["axial_efficiency"] = metrics.thrustAnalysis.axialEfficiency;
        thrustJson["thrust_deviation"] = metrics.thrustAnalysis.thrustDeviation;
        thrustJson["collision_thrust_enhancement"] = metrics.thrustAnalysis.collisionThrustEnhancement;
        thrustJson["multi_wave_advantage_3d"] = metrics.thrustAnalysis.multiWaveAdvantage3D;
        metricsJson["thrust_analysis"] = thrustJson;

        // Specific impulse analysis
        json ispJson;
        ispJson["specific_impulse_3d"] = metrics.ispAnalysis.specificImpulse3D;
        ispJson["mass_flow_rate"] = metrics.ispAnalysis.massFlowRate;
        ispJson["exit_velocity"] = metrics.ispAnalysis.exitVelocity;
        ispJson["nozzle_isp"] = metrics.ispAnalysis.nozzleIsp;
        ispJson["nozzle_efficiency"] = metrics.ispAnalysis.nozzleEfficiency;
        ispJson["isp_deviation"] = metrics.ispAnalysis.ispDeviation;
        metricsJson["isp_analysis"] = ispJson;

        // Combustion efficiency analysis
        json combustionJson;
        combustionJson["combustion_efficiency_3d"] = metrics.combustionAnalysis.combustionEfficiency3D;
        combustionJson["fuel_conversion_rate"] = metrics.combustionAnalysis.fuelConversionRate;
        combustionJson["mixing_efficiency"] = metrics.combustionAnalysis.mixingEfficiency;
        combustionJson["wave_enhanced_efficiency"] = metrics.combustionAnalysis.waveEnhancedEfficiency;
        combustionJson["collision_zone_efficiency"] = metrics.combustionAnalysis.collisionZoneEfficiency;
        combustionJson["efficiency_deviation"] = metrics.combustionAnalysis.efficiencyDeviation;
        metricsJson["combustion_analysis"] = combustionJson;

        // Nozzle performance analysis
        json nozzleJson;
        nozzleJson["throat_area"] = metrics.nozzleAnalysis.throatArea;
        nozzleJson["exit_area"] = metrics.nozzleAnalysis.exitArea;
        nozzleJson["expansion_ratio"] = metrics.nozzleAnalysis.expansionRatio;
        nozzleJson["throat_velocity"] = metrics.nozzleAnalysis.throatVelocity;
        nozzleJson["exit_velocity"] = metrics.nozzleAnalysis.exitVelocity;
        nozzleJson["nozzle_efficiency"] = metrics.nozzleAnalysis.nozzleEfficiency;
        nozzleJson["optimal_expansion_ratio"] = metrics.nozzleAnalysis.optimalExpansionRatio;
        metricsJson["nozzle_analysis"] = nozzleJson;

        // Validation
        metricsJson["validation_2d_thrust"] = metrics.validation2DThrust;
        metricsJson["validation_2d_isp"] = metrics.validation2DIsp;
        metricsJson["validation_2d_efficiency"] = metrics.validation2DEfficiency;

        // Insights and recommendations
        metricsJson["key_insights"] = metrics.keyInsights;
        metricsJson["recommendations"] = metrics.recommendations;

        result.content.push_back(
            json{{"type", "resource"},
                 {"resource",
                  {{"uri", "openfoam://rde_3d_performance/" + caseDirectory},
                   {"name", "3D RDE Performance Results"},
                   {"description", "Complete 3D RDE performance calculation results"},
                   {"mimeType", "application/json"}}},
                 {"text", metricsJson.dump(2)}});

    } catch (const std::exception& e) {
        result.addErrorContent("Error calculating 3D RDE performance: " + std::string(e.what()));
    }

    return result;
}

std::string RDE3DPerformanceTool::formatResultsForUser(const RDE3DPerformanceCalculator::Performance3DMetrics& metrics) const {
    std::ostringstream output;

    output << "**3D RDE Performance Calculation Results**\n\n";

    output << "**Overall Assessment:**\n";
    output << "- Performance Rating: " << metrics.performanceRating << "\n";
    output << "- Performance Score: " << std::fixed << std::setprecision(1) << metrics.overallPerformanceScore << "/100\n";
    output << "- Power Density: " << std::scientific << std::setprecision(2) << metrics.powerDensity << " N/m^3\n";
    output << "- Thrust-to-Weight: " << std::fixed << std::setprecision(1) << metrics.thrustToWeight << "\n\n";

    output << "**Primary Performance Metrics:**\n";
    output << "- Total Thrust: " << std::fixed << std::setprecision(0) << metrics.thrustAnalysis.totalThrust << " N\n";
    output << "- Specific Impulse: " << std::fixed << std::setprecision(0) << metrics.ispAnalysis.specificImpulse3D << " s\n";
    output << "- Combustion Efficiency: " << std::fixed << std::setprecision(1) << metrics.combustionAnalysis.combustionEfficiency3D << "%\n";
    output << "- 3D/2D Performance Ratio: " << std::fixed << std::setprecision(3) << metrics.overall3DvsSDRatio << "\n\n";

    output << "**Validation Against 2D Baseline:**\n";
    output << "- 2D Baseline Thrust: " << std::fixed << std::setprecision(0) << metrics.validation2DThrust << " N\n";
    output << "- 2D Baseline Isp: " << std::fixed << std::setprecision(0) << metrics.validation2DIsp << " s\n";
    output << "- 2D Baseline Efficiency: " << std::fixed << std::setprecision(1) << metrics.validation2DEfficiency << "%\n";
    output << "- Thrust Deviation: " << std::fixed << std::setprecision(1) << metrics.thrustAnalysis.thrustDeviation << "%\n";
    output << "- Isp Deviation: " << std::fixed << std::setprecision(1) << metrics.ispAnalysis.ispDeviation << "%\n";

    return output.str();
}

std::string RDE3DPerformanceTool::generateThrustBreakdown(const RDE3DPerformanceCalculator::Performance3DMetrics& metrics) const {
    std::ostringstream breakdown;

    breakdown << "\n**Thrust Component Breakdown:**\n\n";

    breakdown << "**Sources:**\n";
    breakdown << "- Combustor Thrust: " << std::fixed << std::setprecision(0) << metrics.thrustAnalysis.combustorThrust << " N ("
              << std::setprecision(1) << (metrics.thrustAnalysis.combustorThrust / metrics.thrustAnalysis.totalThrust * 100) << "%)\n";
    breakdown << "- Nozzle Thrust: " << std::fixed << std::setprecision(0) << metrics.thrustAnalysis.nozzleThrust << " N ("
              << std::setprecision(1) << (metrics.thrustAnalysis.nozzleThrust / metrics.thrustAnalysis.totalThrust * 100) << "%)\n\n";

    breakdown << "**Directional Components:**\n";
    breakdown << "- Axial Thrust: " << std::fixed << std::setprecision(0) << metrics.thrustAnalysis.axialThrust << " N\n";
    breakdown << "- Radial Thrust Loss: " << std::fixed << std::setprecision(0) << metrics.thrustAnalysis.radialThrustLoss << " N\n";
    breakdown << "- Thrust Vector Angle: " << std::fixed << std::setprecision(1) << metrics.thrustAnalysis.thrustVectorAngle << " degrees\n";
    breakdown << "- Axial Efficiency: " << std::fixed << std::setprecision(1) << (metrics.thrustAnalysis.axialEfficiency * 100) << "%\n\n";

    breakdown << "**Wave Effects:**\n";
    breakdown << "- Collision Thrust Enhancement: " << std::fixed << std::setprecision(1) << metrics.thrustAnalysis.collisionThrustEnhancement << "%\n";
    breakdown << "- Multi-Wave Advantage (3D): " << std::fixed << std::setprecision(1) << metrics.thrustAnalysis.multiWaveAdvantage3D << "%\n\n";

    breakdown << "**Nozzle Performance:**\n";
    breakdown << "- Throat Velocity: " << std::fixed << std::setprecision(0) << metrics.nozzleAnalysis.throatVelocity << " m/s\n";
    breakdown << "- Exit Velocity: " << std::fixed << std::setprecision(0) << metrics.nozzleAnalysis.exitVelocity << " m/s\n";
    breakdown << "- Expansion Ratio: " << std::fixed << std::setprecision(2) << metrics.nozzleAnalysis.expansionRatio << "\n";
    breakdown << "- Nozzle Efficiency: " << std::fixed << std::setprecision(1) << metrics.nozzleAnalysis.nozzleEfficiency << "%\n";
    breakdown << "- Optimal Expansion Ratio: " << std::fixed << std::setprecision(2) << metrics.nozzleAnalysis.optimalExpansionRatio << "\n";

    return breakdown.str();
}

std::string RDE3DPerformanceTool::generateEducationalContent(const RDE3DPerformanceCalculator::Performance3DMetrics& metrics) const {
    std::ostringstream content;

    content << "\n**Understanding 3D RDE Performance:**\n\n";
    content << metrics.performanceExplanation << "\n\n";

    content << "**Key Insights:**\n";
    for (const auto& insight : metrics.keyInsights) {
        content << "- " << insight << "\n";
    }

    content << "\n**3D vs 2D Comparison:**\n";
    content << metrics.comparisonWith2D << "\n";

    content << "\n**Physics of 3D Thrust Generation:**\n";
    content << "- Combustor contributes pressure-driven thrust from detonation wave pressure gain\n";
    content << "- Nozzle adds thrust through supersonic expansion and momentum increase\n";
    content << "- 3D effects include radial velocity components that reduce axial thrust\n";
    content << "- Wave interactions in 3D can enhance local combustion and pressure gain\n";

    return content.str();
}

std::string RDE3DPerformanceTool::generateOptimizationGuidance(const RDE3DPerformanceCalculator::Performance3DMetrics& metrics) const {
    std::ostringstream guidance;

    guidance << "\n**Optimization Guidance:**\n\n";
    guidance << metrics.optimizationGuidance << "\n\n";

    guidance << "**Recommendations:**\n";
    for (const auto& recommendation : metrics.recommendations) {
        guidance << "- " << recommendation << "\n";
    }

    guidance << "\n**Nozzle Optimization:**\n";
    if (std::abs(metrics.nozzleAnalysis.expansionRatio - metrics.nozzleAnalysis.optimalExpansionRatio) > 0.5) {
        guidance << "- Current expansion ratio (" << std::fixed << std::setprecision(2) << metrics.nozzleAnalysis.expansionRatio
                 << ") differs from optimal (" << metrics.nozzleAnalysis.optimalExpansionRatio << ")\n";
        guidance << "- Consider adjusting nozzle geometry for improved thrust coefficient\n";
    } else {
        guidance << "- Nozzle expansion ratio is near optimal\n";
    }

    guidance << "\n**Combustion Optimization:**\n";
    if (metrics.combustionAnalysis.mixingEfficiency < 90.0) {
        guidance << "- Mixing efficiency (" << std::fixed << std::setprecision(1) << metrics.combustionAnalysis.mixingEfficiency
                 << "%) can be improved\n";
        guidance << "- Consider optimizing injection port design and penetration depth\n";
    }

    return guidance.str();
}

/*---------------------------------------------------------------------------*\
                        Tool Registration Helper
\*---------------------------------------------------------------------------*/

void registerRDE3DPerformanceTool(McpServer& server) {
    auto tool = std::make_shared<RDE3DPerformanceTool>();

    server.registerTool(
        RDE3DPerformanceTool::getName(),
        RDE3DPerformanceTool::getDescription(),
        RDE3DPerformanceTool::getInputSchema(),
        [tool](const json& arguments) -> ToolResult { return tool->execute(arguments); });
}

}  // End namespace MCP
}  // End namespace Foam

// ************************************************************************* //
