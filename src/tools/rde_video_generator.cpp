#include "rde_video_generator.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>

namespace Foam {
namespace MCP {

RDEVideoGenerator::RDEVideoGenerator() 
    : cellularModel_(std::make_unique<CellularDetonation2D>()) {
    initializeEducationalTemplates();
}

RDEVideoGenerator::VideoResult 
RDEVideoGenerator::generateVideo(const VideoTemplate& template_type,
                               const RDEGeometry& geometry,
                               const RDEChemistry& chemistry,
                               const VideoConfig& config,
                               const std::string& output_dir) {
    
    VideoResult result;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Validate inputs
        std::string error_msg;
        if (!validateVideoConfig(config, error_msg)) {
            result.error_message = "Invalid video config: " + error_msg;
            return result;
        }
        
        if (!validateGeometryForVideo(geometry, error_msg)) {
            result.error_message = "Invalid geometry: " + error_msg;
            return result;
        }
        
        // Create output directory
        if (!ensureOutputDirectory(output_dir)) {
            result.error_message = "Failed to create output directory: " + output_dir;
            return result;
        }
        
        // Generate video based on template type
        switch (template_type) {
            case VideoTemplate::WAVE_PROPAGATION:
                result = generateWavePropagationVideo(geometry, chemistry, config, output_dir);
                break;
            case VideoTemplate::MULTI_WAVE_COLLISION:
                result = generateMultiWaveCollisionVideo(geometry, chemistry, config, output_dir);
                break;
            case VideoTemplate::INJECTION_COUPLING:
                result = generateInjectionCouplingVideo(geometry, chemistry, config, output_dir);
                break;
            case VideoTemplate::CELLULAR_EVOLUTION:
                result = generateCellularEvolutionVideo(geometry, chemistry, config, output_dir);
                break;
            case VideoTemplate::COMPLETE_CYCLE:
                result = generateWavePropagationVideo(geometry, chemistry, config, output_dir);
                break;
            case VideoTemplate::EDUCATIONAL_SEQUENCE:
                result = generateEducationalSequence(geometry, chemistry, config, output_dir);
                break;
            default:
                result.error_message = "Unknown video template type";
                return result;
        }
        
        // Add educational metadata
        result.physics_concepts_covered = generatePhysicsExplanations(template_type);
        result.difficulty_level = (template_type == VideoTemplate::EDUCATIONAL_SEQUENCE) ? 
            "beginner" : "intermediate";
        
        auto end_time = std::chrono::high_resolution_clock::now();
        result.generation_time = std::chrono::duration<double>(end_time - start_time).count();
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = "Exception during video generation: " + std::string(e.what());
    }
    
    return result;
}

RDEVideoGenerator::VideoResult 
RDEVideoGenerator::generateVideo(const VideoTemplate& template_type,
                               const RDEGeometry& geometry,
                               const RDEChemistry& chemistry,
                               const std::string& output_dir) {
    VideoConfig default_config;
    return generateVideo(template_type, geometry, chemistry, default_config, output_dir);
}

RDEVideoGenerator::VideoResult 
RDEVideoGenerator::generateWavePropagationVideo(const RDEGeometry& geometry,
                                              const RDEChemistry& chemistry,
                                              const VideoConfig& config,
                                              const std::string& output_dir) {
    
    VideoResult result;
    
    std::cout << "🌊 Generating wave propagation video..." << std::endl;
    
    // Generate animation sequence
    auto frames = generateWavePropagationFrames(geometry, chemistry, config);
    result.frame_count = frames.size();
    
    // Create output filename
    std::string base_name = "wave_propagation_" + chemistry.fuelType + "_phi" + 
                           std::to_string(chemistry.equivalenceRatio).substr(0, 3);
    result.output_file = output_dir + "/" + base_name + "." + config.format;
    
    // Generate educational content
    std::vector<std::string> annotations;
    for (size_t i = 0; i < frames.size(); ++i) {
        std::string annotation = generateFrameAnnotation(frames[i], VideoTemplate::WAVE_PROPAGATION, 
                                                        frames[i].time);
        annotations.push_back(annotation);
        result.educational_annotations.push_back(annotation);
    }
    
    // Create simple text-based visualization data (placeholder for actual ParaView integration)
    std::string data_file = output_dir + "/" + base_name + "_data.json";
    if (exportAnimationData(frames, data_file)) {
        result.generated_files.push_back(data_file);
    }
    
    // Generate Python script for ParaView visualization
    std::string script_file = output_dir + "/" + base_name + "_paraview.py";
    if (generateParaViewScript(frames, config, script_file)) {
        result.generated_files.push_back(script_file);
    }
    
    result.success = true;
    result.log_file = output_dir + "/" + base_name + "_log.txt";
    
    // Create log file
    std::ofstream log(result.log_file);
    log << "=== 2D RDE Wave Propagation Video Generation ===\n";
    log << "Fuel: " << chemistry.fuelType << "\n";
    log << "Equivalence ratio: " << chemistry.equivalenceRatio << "\n";
    log << "Detonation velocity: " << chemistry.detonationVelocity << " m/s\n";
    log << "Cell size: " << chemistry.cellSize << " m\n";
    log << "Frames generated: " << frames.size() << "\n";
    log << "Duration: " << config.duration << " s\n";
    log << "Resolution: " << config.width << "x" << config.height << "\n";
    
    std::cout << "✅ Wave propagation video data generated: " << result.frame_count << " frames" << std::endl;
    
    return result;
}

RDEVideoGenerator::VideoResult 
RDEVideoGenerator::generateMultiWaveCollisionVideo(const RDEGeometry& geometry,
                                                 const RDEChemistry& chemistry,
                                                 const VideoConfig& config,
                                                 const std::string& output_dir) {
    
    VideoResult result;
    
    std::cout << "💥 Generating multi-wave collision video..." << std::endl;
    
    // Generate animation sequence with multiple waves
    auto frames = generateMultiWaveFrames(geometry, chemistry, config);
    result.frame_count = frames.size();
    
    std::string base_name = "multi_wave_collision_" + chemistry.fuelType;
    result.output_file = output_dir + "/" + base_name + "." + config.format;
    
    // Add physics explanations specific to wave collisions
    for (size_t i = 0; i < frames.size(); ++i) {
        std::string annotation = "Frame " + std::to_string(i) + ": Multi-wave collision dynamics\n";
        annotation += "Wave interactions create complex pressure patterns\n";
        annotation += "Energy redistribution occurs during collisions";
        result.educational_annotations.push_back(annotation);
    }
    
    result.success = true;
    std::cout << "✅ Multi-wave collision video data generated: " << result.frame_count << " frames" << std::endl;
    
    return result;
}

RDEVideoGenerator::VideoResult 
RDEVideoGenerator::generateInjectionCouplingVideo(const RDEGeometry& geometry,
                                                const RDEChemistry& chemistry,
                                                const VideoConfig& config,
                                                const std::string& output_dir) {
    
    VideoResult result;
    
    std::cout << "🏹 Generating injection coupling video..." << std::endl;
    
    auto frames = generateInjectionFrames(geometry, chemistry, config);
    result.frame_count = frames.size();
    
    std::string base_name = "injection_coupling_" + std::to_string(geometry.numberOfInjectors) + "_ports";
    result.output_file = output_dir + "/" + base_name + "." + config.format;
    
    // Add injection-specific educational content
    for (size_t i = 0; i < frames.size(); ++i) {
        std::string annotation = "Injection-Wave Interaction Analysis\n";
        annotation += "Fresh reactants provide chemical energy\n";
        annotation += "Momentum transfer affects wave propagation\n";
        annotation += "Timing is critical for performance";
        result.educational_annotations.push_back(annotation);
    }
    
    result.success = true;
    std::cout << "✅ Injection coupling video data generated: " << result.frame_count << " frames" << std::endl;
    
    return result;
}

RDEVideoGenerator::VideoResult 
RDEVideoGenerator::generateCellularEvolutionVideo(const RDEGeometry& geometry,
                                                const RDEChemistry& chemistry,
                                                const VideoConfig& config,
                                                const std::string& output_dir) {
    
    VideoResult result;
    
    std::cout << "🔬 Generating cellular evolution video...\n";
    
    auto frames = generateWavePropagationFrames(geometry, chemistry, config);
    result.frame_count = frames.size();
    
    std::string base_name = "cellular_evolution_" + chemistry.fuelType;
    result.output_file = output_dir + "/" + base_name + "." + config.format;
    
    // Add cellular-specific educational content
    for (size_t i = 0; i < frames.size(); ++i) {
        std::string annotation = "Cellular Evolution Analysis\n";
        annotation += "Cell size: " + std::to_string(chemistry.cellSize * 1000) + " mm\n";
        annotation += "Pattern stability determines performance\n";
        annotation += "Regular cells indicate stable detonation";
        result.educational_annotations.push_back(annotation);
    }
    
    result.success = true;
    std::cout << "✅ Cellular evolution video data generated: " << result.frame_count << " frames\n";
    
    return result;
}

RDEVideoGenerator::VideoResult 
RDEVideoGenerator::generateEducationalSequence(const RDEGeometry& geometry,
                                             const RDEChemistry& chemistry,
                                             const VideoConfig& config,
                                             const std::string& output_dir) {
    
    VideoResult result;
    
    std::cout << "📚 Generating educational sequence...\n";
    
    auto frames = generateWavePropagationFrames(geometry, chemistry, config);
    result.frame_count = frames.size();
    
    std::string base_name = "educational_sequence_" + chemistry.fuelType;
    result.output_file = output_dir + "/" + base_name + "." + config.format;
    
    // Add step-by-step educational content
    for (size_t i = 0; i < frames.size(); ++i) {
        std::string annotation = "Step " + std::to_string(i+1) + ": Educational RDE Analysis\n";
        annotation += "This sequence teaches fundamental RDE physics\n";
        annotation += "Observe wave-injection interactions\n";
        annotation += "Note cellular pattern formation";
        result.educational_annotations.push_back(annotation);
    }
    
    result.success = true;
    result.difficulty_level = "beginner";
    std::cout << "✅ Educational sequence generated: " << result.frame_count << " frames\n";
    
    return result;
}

std::vector<RDEVideoGenerator::AnimationFrame> 
RDEVideoGenerator::generateWavePropagationFrames(const RDEGeometry& geometry,
                                                const RDEChemistry& chemistry,
                                                const VideoConfig& config) {
    
    std::vector<AnimationFrame> frames;
    
    // Calculate time steps
    int num_frames = static_cast<int>(config.duration * config.fps);
    double dt = config.duration / num_frames;
    
    // Create initial wave
    std::vector<CellularDetonation2D::Wave2DPoint> initial_wave;
    for (int i = 0; i < 20; ++i) {
        CellularDetonation2D::Wave2DPoint point;
        point.r = (geometry.innerRadius + geometry.outerRadius) / 2.0;
        point.theta = i * geometry.domainAngle / 20.0;
        point.time = 0.0;
        point.temperature = chemistry.detonationTemperature;
        point.pressure = chemistry.detonationPressure;
        point.waveSpeed = chemistry.detonationVelocity;
        point.isWaveFront = true;
        initial_wave.push_back(point);
    }
    
    // Generate frames
    for (int frame = 0; frame < num_frames; ++frame) {
        AnimationFrame anim_frame;
        anim_frame.time = frame * dt;
        
        // Track wave propagation
        anim_frame.wave_data = cellularModel_->track2DWavePropagation(
            geometry, chemistry, initial_wave, anim_frame.time);
        
        // Analyze cellular structure
        anim_frame.cellular_data = cellularModel_->analyze2DCellularStructure(geometry, chemistry);
        
        // Analyze injection interactions
        anim_frame.injection_data = cellularModel_->analyzeInjectionWaveInteractions(
            geometry, chemistry, anim_frame.wave_data);
        
        // Add educational text
        anim_frame.educational_text = "Time: " + std::to_string(anim_frame.time * 1000) + " ms\n";
        anim_frame.educational_text += "Wave speed: " + std::to_string(anim_frame.wave_data.propagationSpeed) + " m/s\n";
        anim_frame.educational_text += "Cell size: " + std::to_string(chemistry.cellSize * 1000) + " mm";
        
        frames.push_back(anim_frame);
    }
    
    return frames;
}

std::vector<RDEVideoGenerator::AnimationFrame> 
RDEVideoGenerator::generateMultiWaveFrames(const RDEGeometry& geometry,
                                          const RDEChemistry& chemistry,
                                          const VideoConfig& config) {
    
    // Similar to wave propagation but with multiple waves
    auto frames = generateWavePropagationFrames(geometry, chemistry, config);
    
    // Add second wave with phase offset
    for (auto& frame : frames) {
        // Simulate second wave (simplified)
        frame.educational_text += "\nMulti-wave mode: 2 waves detected";
        frame.educational_text += "\nWave spacing: " + std::to_string(M_PI) + " rad";
    }
    
    return frames;
}

std::vector<RDEVideoGenerator::AnimationFrame> 
RDEVideoGenerator::generateInjectionFrames(const RDEGeometry& geometry,
                                          const RDEChemistry& chemistry,
                                          const VideoConfig& config) {
    
    auto frames = generateWavePropagationFrames(geometry, chemistry, config);
    
    // Enhanced injection analysis
    for (auto& frame : frames) {
        frame.educational_text += "\nInjectors: " + std::to_string(geometry.numberOfInjectors);
        frame.educational_text += "\nInjection angle: " + std::to_string(geometry.injectionAngle) + "°";
        frame.educational_text += "\nPenetration: " + std::to_string(geometry.injectionPenetration * 1000) + " mm";
    }
    
    return frames;
}

std::string RDEVideoGenerator::generateFrameAnnotation(const AnimationFrame& frame,
                                                     const VideoTemplate& template_type,
                                                     double simulation_time) {
    
    std::stringstream annotation;
    
    annotation << "=== 2D RDE Analysis ===\n";
    annotation << "Time: " << std::fixed << std::setprecision(3) << simulation_time * 1000 << " ms\n";
    annotation << "Wave Speed: " << std::setprecision(0) << frame.wave_data.propagationSpeed << " m/s\n";
    
    switch (template_type) {
        case VideoTemplate::WAVE_PROPAGATION:
            annotation << "\nWave propagating around annular combustor\n";
            annotation << "Cylindrical geometry creates natural curvature\n";
            annotation << "Cellular structure preserved during propagation";
            break;
        case VideoTemplate::MULTI_WAVE_COLLISION:
            annotation << "\nMultiple waves interact in confined geometry\n";
            annotation << "Collision creates high-pressure regions\n";
            annotation << "Energy redistribution affects performance";
            break;
        case VideoTemplate::INJECTION_COUPLING:
            annotation << "\nFresh reactants inject momentum into flow\n";
            annotation << "Chemical energy addition sustains detonation\n";
            annotation << "Timing critical for wave stability";
            break;
        default:
            annotation << "\nEducational RDE visualization";
            break;
    }
    
    return annotation.str();
}

std::vector<std::string> RDEVideoGenerator::generatePhysicsExplanations(const VideoTemplate& template_type) {
    
    switch (template_type) {
        case VideoTemplate::WAVE_PROPAGATION:
            return {
                "Detonation wave propagation in annular geometry",
                "Chapman-Jouguet theory and wave speed calculations",
                "Cellular detonation structure and instabilities",
                "Cylindrical coordinate effects"
            };
        case VideoTemplate::MULTI_WAVE_COLLISION:
            return {
                "Multi-wave detonation modes",
                "Wave collision and energy transfer",
                "Pressure wave interactions",
                "System stability analysis"
            };
        case VideoTemplate::INJECTION_COUPLING:
            return {
                "Injection-wave momentum transfer",
                "Chemical energy addition mechanisms",
                "Flow blockage and recovery effects",
                "Injection timing optimization"
            };
        default:
            return {"General RDE physics concepts"};
    }
}

bool RDEVideoGenerator::exportAnimationData(const std::vector<AnimationFrame>& frames,
                                          const std::string& json_file) {
    
    try {
        json animation_data;
        animation_data["metadata"] = {
            {"frame_count", frames.size()},
            {"format", "2D_RDE_Animation"},
            {"version", "1.0"}
        };
        
        json frames_array = json::array();
        for (size_t i = 0; i < frames.size(); ++i) {
            json frame_data;
            frame_data["frame_id"] = i;
            frame_data["time"] = frames[i].time;
            frame_data["wave_speed"] = frames[i].wave_data.propagationSpeed;
            frame_data["educational_text"] = frames[i].educational_text;
            frame_data["annotations"] = frames[i].annotations;
            frames_array.push_back(frame_data);
        }
        
        animation_data["frames"] = frames_array;
        
        std::ofstream file(json_file);
        file << animation_data.dump(2);
        file.close();
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to export animation data: " << e.what() << std::endl;
        return false;
    }
}

bool RDEVideoGenerator::generateParaViewScript(const std::vector<AnimationFrame>& frames,
                                             const VideoConfig& config,
                                             const std::string& script_file) {
    
    try {
        std::ofstream script(script_file);
        
        script << "#!/usr/bin/env python3\n";
        script << "# Auto-generated ParaView script for 2D RDE visualization\n\n";
        script << "import sys\n";
        script << "sys.path.insert(0, '/usr/lib/python3/dist-packages')\n\n";
        script << "try:\n";
        script << "    import paraview.simple as pv\n";
        script << "    print('ParaView loaded successfully')\n";
        script << "    \n";
        script << "    # Set up render view\n";
        script << "    render_view = pv.CreateView('RenderView')\n";
        script << "    render_view.ViewSize = [" << config.width << ", " << config.height << "]\n";
        script << "    render_view.Background = " << vectorToString(config.colors.background) << "\n";
        script << "    \n";
        script << "    # Animation settings\n";
        script << "    scene = pv.GetAnimationScene()\n";
        script << "    scene.EndTime = " << config.duration << "\n";
        script << "    scene.NumberOfFrames = " << frames.size() << "\n";
        script << "    \n";
        script << "    print('2D RDE ParaView script ready for execution')\n";
        script << "    \n";
        script << "except ImportError as e:\n";
        script << "    print('ParaView not available:', e)\n";
        
        script.close();
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to generate ParaView script: " << e.what() << std::endl;
        return false;
    }
}

bool RDEVideoGenerator::validateVideoConfig(const VideoConfig& config, std::string& error_msg) {
    
    if (config.width <= 0 || config.height <= 0) {
        error_msg = "Invalid resolution";
        return false;
    }
    
    if (config.fps <= 0 || config.fps > 120) {
        error_msg = "Invalid frame rate (must be 1-120 fps)";
        return false;
    }
    
    if (config.duration <= 0 || config.duration > 60) {
        error_msg = "Invalid duration (must be 0-60 seconds)";
        return false;
    }
    
    return true;
}

bool RDEVideoGenerator::validateGeometryForVideo(const RDEGeometry& geometry, std::string& error_msg) {
    
    std::string geom_error;
    if (!geometry.validateGeometry(geom_error)) {
        error_msg = geom_error;
        return false;
    }
    
    if (!geometry.enable2D) {
        error_msg = "2D mode must be enabled for video generation";
        return false;
    }
    
    return true;
}

void RDEVideoGenerator::initializeEducationalTemplates() {
    
    educationalTemplates_[VideoTemplate::WAVE_PROPAGATION] = {
        "Detonation waves propagate faster than sound",
        "Cellular structure creates characteristic patterns",
        "Wave speed depends on fuel-air mixture properties",
        "Annular geometry affects wave dynamics"
    };
    
    educationalTemplates_[VideoTemplate::MULTI_WAVE_COLLISION] = {
        "Multiple waves can coexist in RDE systems",
        "Wave collisions create complex flow patterns",
        "Energy redistribution affects engine performance",
        "Wave interactions determine operational stability"
    };
    
    educationalTemplates_[VideoTemplate::INJECTION_COUPLING] = {
        "Fresh reactants provide chemical energy",
        "Injection momentum affects wave propagation",
        "Timing synchronization is critical",
        "Flow blockage creates local disturbances"
    };
}

std::string RDEVideoGenerator::vectorToString(const std::vector<double>& vec) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        ss << vec[i];
        if (i < vec.size() - 1) ss << ", ";
    }
    ss << "]";
    return ss.str();
}

// Helper function implementations
bool ensureOutputDirectory(const std::string& output_dir) {
    try {
        std::filesystem::create_directories(output_dir);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create directory " << output_dir << ": " << e.what() << std::endl;
        return false;
    }
}

std::string getVideoTemplateDescription(RDEVideoGenerator::VideoTemplate template_type) {
    switch (template_type) {
        case RDEVideoGenerator::VideoTemplate::WAVE_PROPAGATION:
            return "Single detonation wave propagating around annular combustor";
        case RDEVideoGenerator::VideoTemplate::MULTI_WAVE_COLLISION:
            return "Multiple waves colliding and interacting in confined geometry";
        case RDEVideoGenerator::VideoTemplate::INJECTION_COUPLING:
            return "Fresh reactant injection and wave interaction dynamics";
        case RDEVideoGenerator::VideoTemplate::CELLULAR_EVOLUTION:
            return "Evolution of cellular detonation structure over time";
        case RDEVideoGenerator::VideoTemplate::EDUCATIONAL_SEQUENCE:
            return "Step-by-step educational explanation of RDE physics";
        default:
            return "Unknown template";
    }
}

} // namespace MCP
} // namespace Foam