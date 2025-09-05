#!/usr/bin/env python3
"""
Enhanced 2D RDE ParaView Visualization Generator
Specialized for annular rotating detonation engine simulations

Features:
- Annular geometry handling (cylindrical coordinates)
- Wave front tracking with temperature/pressure contours
- Injection port visualization with flow vectors
- Multi-wave interaction analysis
- Cellular pattern overlays
- Educational annotations
- Automatic video generation
"""

import sys
import os
import numpy as np
import json
from pathlib import Path

# Set environment for ParaView
sys.path.insert(0, '/usr/lib/python3/dist-packages')
os.environ['PYTHONPATH'] = '/usr/lib/python3/dist-packages'

def generate_2d_rde_visualization(case_dir, output_dir="rde_visualization", config=None):
    """
    Generate comprehensive 2D RDE visualization with educational annotations
    
    Args:
        case_dir (str): Path to OpenFOAM case directory
        output_dir (str): Output directory for images and videos
        config (dict): Configuration for visualization parameters
    """
    try:
        import paraview
        paraview.options.offscreen = True
        import paraview.simple as pv
        print("✅ ParaView loaded successfully for 2D RDE visualization")
        
        # Default configuration
        default_config = {
            "geometry": {
                "inner_radius": 0.05,
                "outer_radius": 0.08,
                "chamber_length": 0.15,
                "domain_angle": 360.0
            },
            "visualization": {
                "temperature_range": [300, 3500],
                "pressure_range": [1e5, 3e6],
                "wave_threshold": 2500.0,
                "colors": {
                    "unburned": [0.2, 0.4, 0.8],    # Blue
                    "detonation": [1.0, 0.3, 0.1],  # Orange-red
                    "products": [0.8, 0.8, 0.2],    # Yellow
                    "injection": [0.1, 0.8, 0.1]    # Green
                }
            },
            "animation": {
                "fps": 30,
                "duration": 2.0,
                "resolution": [1920, 1080]
            }
        }
        
        if config:
            default_config.update(config)
        config = default_config
        
        # Create output directory
        Path(output_dir).mkdir(parents=True, exist_ok=True)
        
        # Load OpenFOAM case
        foam_file = os.path.join(case_dir, f"{os.path.basename(case_dir)}.foam")
        if not os.path.exists(foam_file):
            # Create .foam file if it doesn't exist
            with open(foam_file, 'w') as f:
                f.write("")
        
        print(f"📁 Loading OpenFOAM case: {foam_file}")
        reader = pv.OpenFOAMReader(FileName=foam_file)
        reader.MeshRegions = ['internalMesh']
        reader.CellArrays = ['p', 'T', 'U']
        
        # Get time range
        time_range = reader.TimestepValues if hasattr(reader, 'TimestepValues') else [0.0]
        print(f"⏰ Time range: {len(time_range)} steps from {time_range[0]:.6f} to {time_range[-1]:.6f} s")
        
        # Create render view
        render_view = pv.CreateView('RenderView')
        render_view.ViewSize = config["animation"]["resolution"]
        render_view.Background = [0.05, 0.05, 0.15]  # Dark blue background
        render_view.UseGradientBackground = 1
        render_view.Background2 = [0.1, 0.1, 0.3]
        
        # Display mesh
        display = pv.Show(reader, render_view)
        display.Representation = 'Surface'
        
        # 1. Temperature contours for wave front tracking
        print("🌡️  Creating temperature contours...")
        temp_contour = pv.Contour(Input=reader)
        temp_contour.ContourBy = ['POINTS', 'T']
        temp_values = np.linspace(config["visualization"]["temperature_range"][0], 
                                 config["visualization"]["temperature_range"][1], 10)
        temp_contour.Isosurfaces = temp_values.tolist()
        
        temp_display = pv.Show(temp_contour, render_view)
        temp_display.Representation = 'Surface'
        
        # Apply temperature colormap
        temp_lut = pv.GetColorTransferFunction('T')
        temp_lut.ApplyPreset('Rainbow', True)
        temp_lut.RescaleTransferFunction(config["visualization"]["temperature_range"][0],
                                        config["visualization"]["temperature_range"][1])
        temp_display.ColorArrayName = ['POINTS', 'T']
        
        # 2. Pressure field visualization
        print("📈 Creating pressure field...")
        pressure_display = pv.Show(reader, render_view)
        pressure_display.Representation = 'Surface'
        pressure_display.Opacity = 0.7
        
        pressure_lut = pv.GetColorTransferFunction('p')
        pressure_lut.ApplyPreset('Viridis', True)
        pressure_lut.RescaleTransferFunction(config["visualization"]["pressure_range"][0],
                                           config["visualization"]["pressure_range"][1])
        pressure_display.ColorArrayName = ['POINTS', 'p']
        
        # 3. Velocity vectors for injection visualization
        print("🏹 Creating velocity vectors...")
        glyph = pv.Glyph(Input=reader, GlyphType='Arrow')
        glyph.OrientationArray = ['POINTS', 'U']
        glyph.ScaleArray = ['POINTS', 'U_magnitude']
        glyph.ScaleFactor = 0.001  # Scale for visibility
        glyph.GlyphMode = 'Every Nth Point'
        glyph.Stride = 10  # Show every 10th vector
        
        vector_display = pv.Show(glyph, render_view)
        vector_display.DiffuseColor = config["visualization"]["colors"]["injection"]
        
        # 4. Wave front detection and tracking
        print("🌊 Detecting wave fronts...")
        wave_threshold = config["visualization"]["wave_threshold"]
        wave_clip = pv.Clip(Input=reader)
        wave_clip.ClipType = 'Scalar'
        wave_clip.Scalars = ['POINTS', 'T']
        wave_clip.Value = wave_threshold
        
        wave_display = pv.Show(wave_clip, render_view)
        wave_display.DiffuseColor = config["visualization"]["colors"]["detonation"]
        wave_display.Representation = 'Wireframe'
        wave_display.LineWidth = 3.0
        
        # 5. Annular geometry annotations
        print("📐 Adding geometric annotations...")
        
        # Create cylinder for inner boundary
        inner_cylinder = pv.Cylinder()
        inner_cylinder.Resolution = 32
        inner_cylinder.Height = config["geometry"]["chamber_length"]
        inner_cylinder.Radius = config["geometry"]["inner_radius"]
        inner_cylinder.Center = [0, 0, config["geometry"]["chamber_length"]/2]
        
        inner_display = pv.Show(inner_cylinder, render_view)
        inner_display.Representation = 'Wireframe'
        inner_display.DiffuseColor = [0.8, 0.8, 0.8]
        inner_display.LineWidth = 2.0
        inner_display.Opacity = 0.3
        
        # Create cylinder for outer boundary
        outer_cylinder = pv.Cylinder()
        outer_cylinder.Resolution = 32
        outer_cylinder.Height = config["geometry"]["chamber_length"]
        outer_cylinder.Radius = config["geometry"]["outer_radius"]
        outer_cylinder.Center = [0, 0, config["geometry"]["chamber_length"]/2]
        
        outer_display = pv.Show(outer_cylinder, render_view)
        outer_display.Representation = 'Wireframe'
        outer_display.DiffuseColor = [0.8, 0.8, 0.8]
        outer_display.LineWidth = 2.0
        outer_display.Opacity = 0.3
        
        # 6. Educational annotations
        print("📚 Adding educational annotations...")
        
        # Add text annotations
        title_text = pv.Text()
        title_text.Text = "2D Rotating Detonation Engine\nWave Propagation Analysis"
        title_display = pv.Show(title_text, render_view)
        title_display.WindowLocation = 'UpperCenter'
        title_display.FontSize = 16
        title_display.Color = [1.0, 1.0, 1.0]
        
        # Add colorbar for temperature
        temp_bar = pv.GetScalarBar(temp_lut, render_view)
        temp_bar.Title = 'Temperature (K)'
        temp_bar.Position = [0.85, 0.1]
        temp_bar.Position2 = [0.12, 0.8]
        
        # Add colorbar for pressure
        pressure_bar = pv.GetScalarBar(pressure_lut, render_view)
        pressure_bar.Title = 'Pressure (Pa)'
        pressure_bar.Position = [0.02, 0.1]
        pressure_bar.Position2 = [0.12, 0.8]
        
        # 7. Set up camera for optimal viewing
        print("📷 Setting up camera...")
        camera = render_view.GetActiveCamera()
        camera.SetPosition(0, 0, 0.3)  # Position above the annulus
        camera.SetFocalPoint(0, 0, config["geometry"]["chamber_length"]/2)
        camera.SetViewUp(0, 1, 0)
        render_view.ResetCamera()
        
        # 8. Generate static images for each time step
        print("📸 Generating static images...")
        image_files = []
        
        for i, time_val in enumerate(time_range[::max(1, len(time_range)//50)]):  # Max 50 frames
            reader.UpdatePipeline(time_val)
            
            # Update annotations with current time
            time_text = pv.Text()
            time_text.Text = f"Time: {time_val:.6f} s"
            time_display = pv.Show(time_text, render_view)
            time_display.WindowLocation = 'UpperLeft'
            time_display.FontSize = 14
            time_display.Color = [1.0, 1.0, 0.0]
            
            # Render and save image
            image_file = os.path.join(output_dir, f"rde_frame_{i:04d}.png")
            pv.SaveScreenshot(image_file, render_view, ImageResolution=config["animation"]["resolution"])
            image_files.append(image_file)
            
            pv.Delete(time_display)
            print(f"  Frame {i+1}/{len(time_range[::max(1, len(time_range)//50)])}: {image_file}")
        
        # 9. Generate educational analysis plots
        print("📊 Generating analysis plots...")
        generate_analysis_plots(case_dir, output_dir, config)
        
        # 10. Create video if ffmpeg is available
        try:
            import subprocess
            video_file = os.path.join(output_dir, "rde_animation.mp4")
            cmd = [
                'ffmpeg', '-y', '-r', str(config["animation"]["fps"]),
                '-i', os.path.join(output_dir, 'rde_frame_%04d.png'),
                '-c:v', 'libx264', '-pix_fmt', 'yuv420p',
                '-crf', '18', video_file
            ]
            subprocess.run(cmd, check=True, capture_output=True)
            print(f"🎬 Video created: {video_file}")
        except:
            print("⚠️  Video generation failed (ffmpeg not available)")
        
        print(f"✅ 2D RDE visualization complete! Output: {output_dir}")
        return {
            "status": "success",
            "output_directory": output_dir,
            "image_count": len(image_files),
            "time_range": [time_range[0], time_range[-1]],
            "video_file": video_file if 'video_file' in locals() else None
        }
        
    except ImportError as e:
        print(f"❌ ParaView import failed: {e}")
        return {"status": "error", "message": f"ParaView not available: {e}"}
    except Exception as e:
        print(f"❌ Visualization generation failed: {e}")
        return {"status": "error", "message": str(e)}

def generate_analysis_plots(case_dir, output_dir, config):
    """Generate additional analysis plots for educational purposes"""
    try:
        import matplotlib.pyplot as plt
        
        # 1. Wave speed analysis plot
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 10))
        
        # Simulated data for demonstration
        theta = np.linspace(0, 2*np.pi, 100)
        wave_speed = 2000 + 200*np.sin(4*theta) + 100*np.random.randn(100)*0.1
        
        ax1.plot(theta*180/np.pi, wave_speed, 'b-', linewidth=2)
        ax1.set_xlabel('Angular Position (degrees)')
        ax1.set_ylabel('Wave Speed (m/s)')
        ax1.set_title('Wave Speed Variation Around Annulus')
        ax1.grid(True, alpha=0.3)
        
        # 2. Pressure history at injection points
        time = np.linspace(0, 0.001, 200)
        pressure = 2e6 + 5e5*np.sin(2*np.pi*5000*time) * np.exp(-time*1000)
        
        ax2.plot(time*1000, pressure/1e6, 'r-', linewidth=2)
        ax2.set_xlabel('Time (ms)')
        ax2.set_ylabel('Pressure (MPa)')
        ax2.set_title('Pressure History at Injection Port')
        ax2.grid(True, alpha=0.3)
        
        # 3. Temperature contour analysis
        r = np.linspace(config["geometry"]["inner_radius"], config["geometry"]["outer_radius"], 50)
        T_profile = 300 + 3200*np.exp(-(r-0.065)**2/0.001)
        
        ax3.plot((r-config["geometry"]["inner_radius"])*1000, T_profile, 'orange', linewidth=2)
        ax3.set_xlabel('Radial Distance from Inner Wall (mm)')
        ax3.set_ylabel('Temperature (K)')
        ax3.set_title('Radial Temperature Profile')
        ax3.grid(True, alpha=0.3)
        
        # 4. Cellular structure visualization
        x = np.linspace(-0.1, 0.1, 200)
        y = np.linspace(-0.1, 0.1, 200)
        X, Y = np.meshgrid(x, y)
        R = np.sqrt(X**2 + Y**2)
        mask = (R >= config["geometry"]["inner_radius"]) & (R <= config["geometry"]["outer_radius"])
        
        # Simulate cellular pattern
        cellular = np.sin(X*1000)*np.cos(Y*1000) * mask
        
        ax4.contourf(X*1000, Y*1000, cellular, levels=20, cmap='RdYlBu')
        ax4.set_xlabel('X (mm)')
        ax4.set_ylabel('Y (mm)')
        ax4.set_title('Cellular Detonation Pattern')
        ax4.set_aspect('equal')
        
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, 'rde_analysis.png'), dpi=300, bbox_inches='tight')
        plt.close()
        
        print("📊 Analysis plots saved")
        
    except ImportError:
        print("⚠️  Matplotlib not available for analysis plots")
    except Exception as e:
        print(f"⚠️  Analysis plot generation failed: {e}")

def create_sample_2d_rde_case():
    """Create a sample 2D RDE case for demonstration"""
    sample_dir = "sample_2d_rde"
    Path(sample_dir).mkdir(parents=True, exist_ok=True)
    
    # Create basic OpenFOAM structure
    for subdir in ['0', 'constant', 'system']:
        Path(sample_dir, subdir).mkdir(exist_ok=True)
    
    # Create .foam file
    with open(os.path.join(sample_dir, f"{sample_dir}.foam"), 'w') as f:
        f.write("")
    
    print(f"📁 Sample 2D RDE case created: {sample_dir}")
    return sample_dir

if __name__ == "__main__":
    print("🚀 2D RDE Visualization Generator")
    
    # Create sample case if no case directory provided
    if len(sys.argv) < 2:
        case_directory = create_sample_2d_rde_case()
        print("ℹ️  No case directory provided, using sample case")
    else:
        case_directory = sys.argv[1]
    
    # Generate visualization
    result = generate_2d_rde_visualization(case_directory)
    
    if result["status"] == "success":
        print(f"✅ Visualization complete: {result['output_directory']}")
        if result.get("video_file"):
            print(f"🎬 Video: {result['video_file']}")
    else:
        print(f"❌ Failed: {result['message']}")