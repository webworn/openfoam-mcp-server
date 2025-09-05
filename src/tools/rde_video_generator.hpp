#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include "../openfoam/rotating_detonation_engine.hpp"
#include "../openfoam/cellular_detonation_2d.hpp"

namespace Foam {
namespace MCP {

using json = nlohmann::json;

/*---------------------------------------------------------------------------*\
                    Class RDEVideoGenerator
\*---------------------------------------------------------------------------*/

/**
 * 2D RDE Video Generation System
 * 
 * Integrates with CellularDetonation2D to create educational videos showing:
 * - Wave propagation in annular geometry
 * - Multi-wave interactions and collisions
 * - Injection-wave coupling dynamics
 * - Cellular structure evolution
 * - Educational annotations and explanations
 */
class RDEVideoGenerator {
public:
    RDEVideoGenerator();
    ~RDEVideoGenerator() = default;
    
    // Video generation configuration
    struct VideoConfig {
        // Video settings
        int width = 1920;
        int height = 1080;
        int fps = 30;
        double duration = 2.0;  // seconds
        std::string format = "mp4";
        
        // Visual settings
        struct ColorScheme {
            std::vector<double> unburned = {0.2, 0.4, 0.8};      // Blue
            std::vector<double> detonation = {1.0, 0.3, 0.1};    // Orange-red
            std::vector<double> products = {0.8, 0.8, 0.2};      // Yellow
            std::vector<double> injection = {0.1, 0.8, 0.1};     // Green
            std::vector<double> background = {0.05, 0.05, 0.15}; // Dark blue
        } colors;
        
        struct TemperatureRange {
            double min = 300.0;
            double max = 3500.0;
            double wave_threshold = 2500.0;
        } temperature;
        
        struct PressureRange {
            double min = 1e5;
            double max = 3e6;
        } pressure;
        
        // Educational settings
        bool enable_annotations = true;
        bool show_physics_equations = true;
        bool show_cellular_overlay = true;
        bool show_injection_vectors = true;
        double annotation_font_size = 16.0;
        
        // Animation settings
        std::string camera_mode = "top_view";  // "top_view", "side_view", "rotating"
        bool enable_zoom_effects = true;
        bool highlight_wave_fronts = true;
    };
    
    // Video template types
    enum class VideoTemplate {
        WAVE_PROPAGATION,      // Single wave propagating around annulus
        MULTI_WAVE_COLLISION,  // Multiple waves colliding
        INJECTION_COUPLING,    // Injection-wave interactions
        CELLULAR_EVOLUTION,    // Cellular structure changes
        COMPLETE_CYCLE,        // Full RDE operational cycle
        EDUCATIONAL_SEQUENCE   // Step-by-step educational explanation
    };
    
    // Video generation result
    struct VideoResult {
        bool success = false;
        std::string output_file;
        std::string log_file;
        double generation_time = 0.0;
        int frame_count = 0;
        std::vector<std::string> generated_files;
        std::string error_message;
        
        // Educational metadata
        std::vector<std::string> physics_concepts_covered;
        std::vector<std::string> educational_annotations;
        std::string difficulty_level; // "beginner", "intermediate", "advanced"
    };
    
    // Main video generation interface
    VideoResult generateVideo(const VideoTemplate& template_type,
                             const RDEGeometry& geometry,
                             const RDEChemistry& chemistry,
                             const VideoConfig& config,
                             const std::string& output_dir = "rde_videos");
    
    // Convenience method with default config
    VideoResult generateVideo(const VideoTemplate& template_type,
                             const RDEGeometry& geometry,
                             const RDEChemistry& chemistry,
                             const std::string& output_dir = "rde_videos");
    
    // Specialized video generation methods
    VideoResult generateWavePropagationVideo(const RDEGeometry& geometry,
                                           const RDEChemistry& chemistry,
                                           const VideoConfig& config,
                                           const std::string& output_dir);
    
    VideoResult generateMultiWaveCollisionVideo(const RDEGeometry& geometry,
                                              const RDEChemistry& chemistry,
                                              const VideoConfig& config,
                                              const std::string& output_dir);
    
    VideoResult generateInjectionCouplingVideo(const RDEGeometry& geometry,
                                             const RDEChemistry& chemistry,
                                             const VideoConfig& config,
                                             const std::string& output_dir);
    
    VideoResult generateCellularEvolutionVideo(const RDEGeometry& geometry,
                                             const RDEChemistry& chemistry,
                                             const VideoConfig& config,
                                             const std::string& output_dir);
    
    VideoResult generateEducationalSequence(const RDEGeometry& geometry,
                                           const RDEChemistry& chemistry,
                                           const VideoConfig& config,
                                           const std::string& output_dir);
    
    // Animation data generation
    struct AnimationFrame {
        double time;
        CellularDetonation2D::WavePropagation2D wave_data;
        CellularDetonation2D::CellularStructure2D cellular_data;
        std::vector<CellularDetonation2D::InjectionWaveInteraction> injection_data;
        std::string educational_text;
        std::vector<std::string> annotations;
    };
    
    std::vector<AnimationFrame> generateAnimationSequence(const VideoTemplate& template_type,
                                                         const RDEGeometry& geometry,
                                                         const RDEChemistry& chemistry,
                                                         const VideoConfig& config);
    
    // Educational content generation
    std::string generateFrameAnnotation(const AnimationFrame& frame,
                                       const VideoTemplate& template_type,
                                       double simulation_time);
    
    std::vector<std::string> generatePhysicsExplanations(const VideoTemplate& template_type);
    
    // ParaView integration
    bool generateParaViewScript(const std::vector<AnimationFrame>& frames,
                               const VideoConfig& config,
                               const std::string& script_file);
    
    bool executeParaViewScript(const std::string& script_file,
                              const std::string& output_dir);
    
    // Post-processing and assembly
    bool assembleVideoFrames(const std::vector<std::string>& frame_files,
                           const VideoConfig& config,
                           const std::string& output_file);
    
    bool addEducationalOverlay(const std::string& video_file,
                              const std::vector<std::string>& annotations,
                              const VideoConfig& config);
    
    // Utility methods
    std::string getTemplateDescription(const VideoTemplate& template_type) const;
    VideoConfig getRecommendedConfig(const VideoTemplate& template_type) const;
    std::string vectorToString(const std::vector<double>& vec);
    
    // Export capabilities
    bool exportAnimationData(const std::vector<AnimationFrame>& frames,
                           const std::string& json_file);
    
    bool exportEducationalNotes(const VideoResult& result,
                               const std::string& notes_file);
    
private:
    std::unique_ptr<CellularDetonation2D> cellularModel_;
    
    // Template-specific generation methods
    std::vector<AnimationFrame> generateWavePropagationFrames(const RDEGeometry& geometry,
                                                             const RDEChemistry& chemistry,
                                                             const VideoConfig& config);
    
    std::vector<AnimationFrame> generateMultiWaveFrames(const RDEGeometry& geometry,
                                                       const RDEChemistry& chemistry,
                                                       const VideoConfig& config);
    
    std::vector<AnimationFrame> generateInjectionFrames(const RDEGeometry& geometry,
                                                       const RDEChemistry& chemistry,
                                                       const VideoConfig& config);
    
    // Wave simulation and tracking
    CellularDetonation2D::WavePropagation2D simulateWaveEvolution(
        const RDEGeometry& geometry,
        const RDEChemistry& chemistry,
        double start_time,
        double end_time,
        int num_steps);
    
    // Educational content templates
    std::map<VideoTemplate, std::vector<std::string>> educationalTemplates_;
    void initializeEducationalTemplates();
    
    // Video processing utilities
    bool checkFFmpegAvailability();
    bool checkParaViewAvailability();
    
    std::string createFFmpegCommand(const std::vector<std::string>& input_files,
                                   const std::string& output_file,
                                   const VideoConfig& config);
    
    // Frame generation helpers
    bool generateTemperatureFrame(const AnimationFrame& frame,
                                const VideoConfig& config,
                                const std::string& output_file);
    
    bool generatePressureFrame(const AnimationFrame& frame,
                             const VideoConfig& config,
                             const std::string& output_file);
    
    bool generateVelocityFrame(const AnimationFrame& frame,
                             const VideoConfig& config,
                             const std::string& output_file);
    
    // Annotation and overlay utilities
    std::string formatPhysicsEquation(const std::string& equation_name,
                                     const RDEChemistry& chemistry);
    
    std::vector<std::string> generateTimeSeriesAnnotations(const std::vector<AnimationFrame>& frames);
    
    // Configuration and validation
    bool validateVideoConfig(const VideoConfig& config, std::string& error_msg);
    bool validateGeometryForVideo(const RDEGeometry& geometry, std::string& error_msg);
    
    // Performance monitoring
    struct PerformanceStats {
        double frame_generation_time = 0.0;
        double video_assembly_time = 0.0;
        double total_processing_time = 0.0;
        int successful_frames = 0;
        int failed_frames = 0;
    };
    
    PerformanceStats trackPerformance_;
    void logPerformance(const std::string& operation, double duration);
};

/*---------------------------------------------------------------------------*\
                    Helper Functions for Video Generation
\*---------------------------------------------------------------------------*/

// Video template utilities
std::string getVideoTemplateDescription(RDEVideoGenerator::VideoTemplate template_type);
RDEVideoGenerator::VideoConfig getDefaultConfigForTemplate(RDEVideoGenerator::VideoTemplate template_type);

// Educational content helpers
std::vector<std::string> generateSocraticQuestions(RDEVideoGenerator::VideoTemplate template_type);
std::string explainVideoPhysics(RDEVideoGenerator::VideoTemplate template_type);

// File and path utilities
std::string generateUniqueVideoFileName(const std::string& base_name, 
                                       RDEVideoGenerator::VideoTemplate template_type);
bool ensureOutputDirectory(const std::string& output_dir);

} // namespace MCP
} // namespace Foam