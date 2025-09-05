#!/usr/bin/env python3
"""
Generate Demo RDE Video 
Creates a demonstration video showing RDE concepts without full OpenFOAM simulation
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from pathlib import Path
import json

def create_demo_rde_video():
    """Create a demonstration RDE video with simulated data"""
    
    print("🎬 Creating Demo RDE Video...")
    
    # Create output directory
    output_dir = Path("demo_video_output")
    output_dir.mkdir(exist_ok=True)
    
    # RDE geometry parameters (from our configuration)
    inner_radius = 0.05  # 50mm
    outer_radius = 0.08  # 80mm
    
    # Create coordinate grids
    theta = np.linspace(0, 2*np.pi, 100)
    r = np.linspace(inner_radius, outer_radius, 30)
    THETA, R = np.meshgrid(theta, r)
    X = R * np.cos(THETA)
    Y = R * np.sin(THETA)
    
    # Simulation parameters
    duration = 2.0  # seconds
    fps = 30
    frames = int(duration * fps)
    
    # Create figure
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 10))
    fig.suptitle('2D RDE Demonstration Video\nHydrogen-Air Detonation Wave', fontsize=16, fontweight='bold')
    
    def animate(frame):
        # Clear previous plots
        for ax in [ax1, ax2, ax3, ax4]:
            ax.clear()
        
        # Current time
        t = frame / fps
        
        # Simulate detonation wave propagation
        wave_position = (t * 1968 / (outer_radius - inner_radius)) % (2 * np.pi)  # 1968 m/s wave speed
        
        # Temperature field (wave propagation)
        temperature = 300 + 3200 * np.exp(-((THETA - wave_position) % (2*np.pi))**2 / 0.1)
        
        # Plot 1: Temperature contours
        c1 = ax1.contourf(X*1000, Y*1000, temperature, levels=20, cmap='hot')
        ax1.set_title('Temperature Field (K)')
        ax1.set_xlabel('X (mm)')
        ax1.set_ylabel('Y (mm)')
        ax1.set_aspect('equal')
        
        # Add inner/outer boundaries
        circle_inner = plt.Circle((0, 0), inner_radius*1000, fill=False, color='black', linewidth=2)
        circle_outer = plt.Circle((0, 0), outer_radius*1000, fill=False, color='black', linewidth=2)
        ax1.add_patch(circle_inner)
        ax1.add_patch(circle_outer)
        
        # Plot 2: Pressure field
        pressure = 1e5 + 2.4e6 * np.exp(-((THETA - wave_position) % (2*np.pi))**2 / 0.15)
        c2 = ax2.contourf(X*1000, Y*1000, pressure/1e6, levels=15, cmap='viridis')
        ax2.set_title('Pressure Field (MPa)')
        ax2.set_xlabel('X (mm)')
        ax2.set_ylabel('Y (mm)')
        ax2.set_aspect('equal')
        
        circle_inner2 = plt.Circle((0, 0), inner_radius*1000, fill=False, color='white', linewidth=2)
        circle_outer2 = plt.Circle((0, 0), outer_radius*1000, fill=False, color='white', linewidth=2)
        ax2.add_patch(circle_inner2)
        ax2.add_patch(circle_outer2)
        
        # Plot 3: Wave speed analysis
        angles = np.linspace(0, 2*np.pi, 100)
        wave_speed = 1968 + 100*np.sin(4*angles) * np.exp(-abs(angles - wave_position))
        ax3.plot(angles*180/np.pi, wave_speed, 'b-', linewidth=2)
        ax3.axvline(wave_position*180/np.pi, color='red', linestyle='--', linewidth=2, label='Current Wave')
        ax3.set_xlabel('Angular Position (degrees)')
        ax3.set_ylabel('Wave Speed (m/s)')
        ax3.set_title('Wave Speed Around Annulus')
        ax3.grid(True, alpha=0.3)
        ax3.legend()
        ax3.set_ylim(1800, 2200)
        
        # Plot 4: Time history
        time_history = np.linspace(0, t, frame+1)
        pressure_history = 1e5 + 2e6*np.sin(2*np.pi*5000*time_history) * np.exp(-time_history*500)
        ax4.plot(time_history*1000, pressure_history/1e6, 'g-', linewidth=2)
        ax4.set_xlabel('Time (ms)')
        ax4.set_ylabel('Pressure (MPa)')
        ax4.set_title(f'Pressure History (t = {t:.3f} s)')
        ax4.grid(True, alpha=0.3)
        ax4.set_xlim(0, duration*1000)
        ax4.set_ylim(0, 3)
        
        # Add educational text
        educational_text = f'''Educational Notes:
• Detonation velocity: 1968 m/s
• Cell size: 1.0 mm (H₂-air)
• Wave completes annulus in {2*np.pi*(outer_radius-inner_radius)/2/1968*1000:.1f} ms
• Current position: {wave_position*180/np.pi:.0f}°
• Physics: Chapman-Jouguet theory'''
        
        fig.text(0.02, 0.02, educational_text, fontsize=10, 
                bbox=dict(boxstyle="round,pad=0.3", facecolor="lightblue", alpha=0.7))
        
        plt.tight_layout()
        return []
    
    # Create animation
    print(f"  🎞️  Creating {frames} frames at {fps} fps...")
    anim = animation.FuncAnimation(fig, animate, frames=frames, interval=1000/fps, blit=False)
    
    # Save as MP4 (if ffmpeg available) or GIF
    try:
        video_file = output_dir / "rde_demo_video.mp4"
        anim.save(str(video_file), writer='ffmpeg', fps=fps, bitrate=1800)
        print(f"  ✅ MP4 video saved: {video_file}")
    except:
        try:
            gif_file = output_dir / "rde_demo_video.gif"
            anim.save(str(gif_file), writer='pillow', fps=fps//2)
            print(f"  ✅ GIF animation saved: {gif_file}")
        except:
            print("  ⚠️  Video/GIF export not available, but animation created")
    
    plt.show()
    
    # Save configuration info
    config_info = {
        "demo_type": "2D RDE Wave Propagation",
        "geometry": {
            "inner_radius_mm": inner_radius * 1000,
            "outer_radius_mm": outer_radius * 1000,
            "annular_width_mm": (outer_radius - inner_radius) * 1000
        },
        "physics": {
            "fuel": "Hydrogen (H2)",
            "detonation_velocity_ms": 1968,
            "cell_size_mm": 1.0,
            "wave_period_ms": 2*np.pi*(outer_radius-inner_radius)/2/1968*1000
        },
        "video": {
            "duration_s": duration,
            "fps": fps,
            "total_frames": frames,
            "resolution": "1200x1000"
        }
    }
    
    with open(output_dir / "demo_config.json", "w") as f:
        json.dump(config_info, f, indent=2)
    
    print(f"  📊 Configuration saved: {output_dir}/demo_config.json")
    print(f"\n🎉 Demo video generation complete!")
    print(f"📁 Output directory: {output_dir}/")
    
    return output_dir

if __name__ == "__main__":
    create_demo_rde_video()