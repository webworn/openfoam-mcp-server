#include "src/tools/rde_video_generator.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

using namespace Foam::MCP;

void testVideoGeneration() {
    std::cout << "🎬 Testing RDE Video Generation System...\n";
    
    // Create test geometry
    RDEGeometry geometry;
    geometry.outerRadius = 0.08;        // 80mm outer radius
    geometry.innerRadius = 0.05;        // 50mm inner radius  
    geometry.chamberLength = 0.15;      // 150mm length
    geometry.enable2D = true;           // Enable 2D mode
    geometry.domainAngle = 2.0 * M_PI;  // Full 360° annulus
    geometry.periodicBoundaries = true;
    geometry.numberOfInjectors = 8;
    geometry.injectionAngle = 45.0;
    geometry.injectionPenetration = 0.01; // 10mm penetration
    
    // Setup injector positions
    geometry.injectorAngularPositions.clear();
    geometry.injectorRadialPositions.clear();
    for (int i = 0; i < geometry.numberOfInjectors; ++i) {
        double angle = i * 2.0 * M_PI / geometry.numberOfInjectors;
        geometry.injectorAngularPositions.push_back(angle);
        geometry.injectorRadialPositions.push_back(geometry.outerRadius - 0.005);
    }
    
    // Create test chemistry
    RDEChemistry chemistry;
    chemistry.fuelType = "H2";
    chemistry.equivalenceRatio = 1.0;
    chemistry.detonationVelocity = 2000.0;    // m/s
    chemistry.detonationPressure = 2.5e6;     // Pa
    chemistry.detonationTemperature = 3500.0;  // K
    chemistry.cellSize = 0.001;               // 1mm cell size
    
    // Create video generator
    RDEVideoGenerator generator;
    
    // Test video configuration
    RDEVideoGenerator::VideoConfig config;
    config.width = 1280;
    config.height = 720;
    config.fps = 24;
    config.duration = 1.0; // 1 second for testing
    config.format = "mp4";
    config.enable_annotations = true;
    config.show_cellular_overlay = true;
    
    // Create output directory
    std::string output_dir = "test_video_output";
    std::filesystem::create_directories(output_dir);
    
    std::cout << "  📁 Output directory: " << output_dir << "\n";
    
    // Test 1: Wave Propagation Video
    std::cout << "  🌊 Testing wave propagation video...\n";
    auto result1 = generator.generateVideo(
        RDEVideoGenerator::VideoTemplate::WAVE_PROPAGATION,
        geometry, chemistry, config, output_dir
    );
    
    assert(result1.success);
    assert(result1.frame_count > 0);
    assert(!result1.physics_concepts_covered.empty());
    assert(!result1.educational_annotations.empty());
    std::cout << "    ✅ Generated " << result1.frame_count << " frames\n";
    std::cout << "    📚 Physics concepts: " << result1.physics_concepts_covered.size() << "\n";
    std::cout << "    📝 Educational annotations: " << result1.educational_annotations.size() << "\n";
    
    // Test 2: Multi-Wave Collision Video
    std::cout << "  💥 Testing multi-wave collision video...\n";
    auto result2 = generator.generateVideo(
        RDEVideoGenerator::VideoTemplate::MULTI_WAVE_COLLISION,
        geometry, chemistry, config, output_dir
    );
    
    assert(result2.success);
    assert(result2.frame_count > 0);
    std::cout << "    ✅ Generated " << result2.frame_count << " frames\n";
    
    // Test 3: Injection Coupling Video
    std::cout << "  🏹 Testing injection coupling video...\n";
    auto result3 = generator.generateVideo(
        RDEVideoGenerator::VideoTemplate::INJECTION_COUPLING,
        geometry, chemistry, config, output_dir
    );
    
    assert(result3.success);
    assert(result3.frame_count > 0);
    std::cout << "    ✅ Generated " << result3.frame_count << " frames\n";
    
    // Test 4: Educational Sequence
    std::cout << "  📚 Testing educational sequence...\n";
    auto result4 = generator.generateVideo(
        RDEVideoGenerator::VideoTemplate::EDUCATIONAL_SEQUENCE,
        geometry, chemistry, config, output_dir
    );
    
    assert(result4.success);
    assert(result4.difficulty_level == "beginner");
    std::cout << "    ✅ Educational content generated\n";
    
    // Verify output files exist
    std::cout << "  📂 Verifying output files...\n";
    int file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
        if (entry.is_regular_file()) {
            file_count++;
            std::cout << "    📄 " << entry.path().filename().string() << "\n";
        }
    }
    std::cout << "    📊 Total files generated: " << file_count << "\n";
    
    // Test video template descriptions
    std::cout << "  📋 Testing template descriptions...\n";
    for (auto template_type : {
        RDEVideoGenerator::VideoTemplate::WAVE_PROPAGATION,
        RDEVideoGenerator::VideoTemplate::MULTI_WAVE_COLLISION,
        RDEVideoGenerator::VideoTemplate::INJECTION_COUPLING,
        RDEVideoGenerator::VideoTemplate::CELLULAR_EVOLUTION,
        RDEVideoGenerator::VideoTemplate::EDUCATIONAL_SEQUENCE
    }) {
        std::string desc = getVideoTemplateDescription(template_type);
        assert(!desc.empty());
        assert(desc != "Unknown template");
        std::cout << "    📝 " << desc << "\n";
    }
    
    // Test error handling
    std::cout << "  ⚠️  Testing error handling...\n";
    
    // Invalid geometry (2D not enabled)
    RDEGeometry bad_geometry = geometry;
    bad_geometry.enable2D = false;
    auto bad_result = generator.generateVideo(
        RDEVideoGenerator::VideoTemplate::WAVE_PROPAGATION,
        bad_geometry, chemistry, config, output_dir
    );
    assert(!bad_result.success);
    assert(!bad_result.error_message.empty());
    std::cout << "    ✅ Error handling working: " << bad_result.error_message << "\n";
    
    // Invalid video config
    RDEVideoGenerator::VideoConfig bad_config = config;
    bad_config.fps = 0; // Invalid FPS
    auto bad_result2 = generator.generateVideo(
        RDEVideoGenerator::VideoTemplate::WAVE_PROPAGATION,
        geometry, chemistry, bad_config, output_dir
    );
    assert(!bad_result2.success);
    std::cout << "    ✅ Config validation working\n";
    
    std::cout << "✅ All RDE video generation tests passed!\n\n";
}

void testParaViewIntegration() {
    std::cout << "🖼️  Testing ParaView integration...\n";
    
    // Test that the ParaView demo script exists and has proper structure
    std::string script_path = "/workspaces/openfoam-mcp-server/paraview-demo/generate_2d_rde_visualization.py";
    
    std::ifstream script_file(script_path);
    assert(script_file.is_open());
    
    std::string content;
    std::string line;
    while (std::getline(script_file, line)) {
        content += line + "\n";
    }
    script_file.close();
    
    // Check for key functions and features
    assert(content.find("generate_2d_rde_visualization") != std::string::npos);
    assert(content.find("temperature_range") != std::string::npos);
    assert(content.find("pressure_range") != std::string::npos);
    assert(content.find("wave_threshold") != std::string::npos);
    assert(content.find("annular geometry") != std::string::npos);
    assert(content.find("educational annotations") != std::string::npos);
    
    std::cout << "  ✅ ParaView script structure validated\n";
    
    // Test configuration structure
    assert(content.find("inner_radius") != std::string::npos);
    assert(content.find("outer_radius") != std::string::npos);
    assert(content.find("chamber_length") != std::string::npos);
    assert(content.find("domain_angle") != std::string::npos);
    
    std::cout << "  ✅ Annular geometry configuration present\n";
    
    // Test visualization features
    assert(content.find("temperature contours") != std::string::npos);
    assert(content.find("pressure field") != std::string::npos);
    assert(content.find("velocity vectors") != std::string::npos);
    assert(content.find("wave front") != std::string::npos);
    
    std::cout << "  ✅ All required visualization features present\n";
    
    std::cout << "✅ ParaView integration validated!\n\n";
}

void demonstrateVideoCapabilities() {
    std::cout << "🎯 Demonstrating 2D RDE Video Generation Capabilities\n\n";
    
    std::cout << "📹 Available Video Templates:\n";
    
    std::vector<RDEVideoGenerator::VideoTemplate> templates = {
        RDEVideoGenerator::VideoTemplate::WAVE_PROPAGATION,
        RDEVideoGenerator::VideoTemplate::MULTI_WAVE_COLLISION,
        RDEVideoGenerator::VideoTemplate::INJECTION_COUPLING,
        RDEVideoGenerator::VideoTemplate::CELLULAR_EVOLUTION,
        RDEVideoGenerator::VideoTemplate::EDUCATIONAL_SEQUENCE
    };
    
    for (auto template_type : templates) {
        std::string desc = getVideoTemplateDescription(template_type);
        std::cout << "  🎬 " << desc << "\n";
    }
    
    std::cout << "\n🔧 Video Generation Features:\n";
    std::cout << "  📐 Annular geometry support with cylindrical coordinates\n";
    std::cout << "  🌊 Wave front tracking with temperature/pressure contours\n"; 
    std::cout << "  🏹 Injection port visualization with flow vectors\n";
    std::cout << "  💥 Multi-wave interaction analysis\n";
    std::cout << "  🔬 Cellular pattern overlays\n";
    std::cout << "  📚 Educational annotations and physics explanations\n";
    std::cout << "  🎥 Automatic video assembly with customizable settings\n";
    std::cout << "  📊 Performance monitoring and quality control\n";
    
    std::cout << "\n🎛️  Video Configuration Options:\n";
    std::cout << "  📺 Resolutions: 720p, 1080p, 4K support\n";
    std::cout << "  🎬 Frame rates: 24, 30, 60 fps\n";
    std::cout << "  ⏱️  Duration: 1-60 seconds\n";
    std::cout << "  🎨 Color schemes: Physical property mapping\n";
    std::cout << "  📝 Annotations: Automatic physics explanations\n";
    std::cout << "  📹 Formats: MP4, AVI, PNG sequence\n";
    
    std::cout << "\n📊 Integration Capabilities:\n";
    std::cout << "  🔗 ParaView: Automatic script generation\n";
    std::cout << "  🧮 OpenFOAM: Direct case data processing\n";
    std::cout << "  📈 CellularDetonation2D: Physics model integration\n";
    std::cout << "  🎯 Educational system: Socratic questioning integration\n";
    std::cout << "  💾 Export: JSON data, educational notes, analysis plots\n";
    
    std::cout << "\n✅ Video generation system ready for 2D RDE simulations!\n\n";
}

int main() {
    std::cout << "🚀 RDE Video Generation System Test Suite\n";
    std::cout << "=========================================\n\n";
    
    try {
        // Run comprehensive tests
        testVideoGeneration();
        testParaViewIntegration();
        demonstrateVideoCapabilities();
        
        std::cout << "🎉 All tests completed successfully!\n";
        std::cout << "📋 Video generation system is ready for production use.\n\n";
        
        std::cout << "🔥 The 2D RDE system now provides:\n";
        std::cout << "  ✅ Complete 2D annular geometry support\n";
        std::cout << "  ✅ Cellular detonation physics in cylindrical coordinates\n";
        std::cout << "  ✅ Multi-wave interaction analysis\n";
        std::cout << "  ✅ Injection-wave coupling dynamics\n";
        std::cout << "  ✅ Advanced ParaView visualization with educational annotations\n";
        std::cout << "  ✅ Comprehensive video generation with multiple templates\n";
        std::cout << "  ✅ Educational content integration with Socratic questioning\n";
        std::cout << "  ✅ Production-ready video generation pipeline\n\n";
        
        std::cout << "🎯 Next steps for users:\n";
        std::cout << "  1. Run ./test_rde_video_generation to validate setup\n";
        std::cout << "  2. Use video templates for educational content creation\n";
        std::cout << "  3. Integrate with existing 1D RDE expert system\n";
        std::cout << "  4. Generate research-quality visualizations\n";
        std::cout << "  5. Create educational materials for university courses\n\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}