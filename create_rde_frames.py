#!/usr/bin/env python3
"""
2D RDE Longitudinal Wave Propagation Visualization
Creates frames showing detonation wave propagation along chamber length (z-direction)
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.patches import Rectangle
from pathlib import Path
import json

def create_longitudinal_rde_visualization():
    """Create lengthwise wave propagation visualization"""
    
    print("🎬 Creating Longitudinal RDE Wave Propagation Video")
    print("📐 Chamber Length: 150mm (z-direction)")
    print("⚡ Detonation Wave Speed: 2200 m/s")
    
    # RDE chamber geometry (lengthwise view)
    chamber_length = 0.150  # 150mm
    chamber_height = 0.030  # 30mm annular gap height
    
    # Set up the figure for lengthwise view
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(16, 12))
    fig.patch.set_facecolor('black')
    
    # Time parameters
    total_time = 70e-6  # 70 microseconds
    wave_speed = 2200   # m/s
    n_frames = 120
    time_steps = np.linspace(0, total_time, n_frames)
    
    # Generate 6 frames at different wave positions
    wave_positions = [0, np.pi/3, 2*np.pi/3, np.pi, 4*np.pi/3, 5*np.pi/3]
    
    for i, wave_pos in enumerate(wave_positions):
        print(f"  📸 Creating frame {i+1}/6 (wave at {wave_pos*180/np.pi:.0f}°)...")
        
        # Create temperature field
        temperature = 300 + 3200 * np.exp(-((THETA - wave_pos) % (2*np.pi))**2 / 0.1)
        
        # Create pressure field  
        pressure = 1e5 + 2.4e6 * np.exp(-((THETA - wave_pos) % (2*np.pi))**2 / 0.15)
        
        # Create figure with 2 subplots
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
        fig.suptitle(f'2D RDE Frame {i+1}: Wave at {wave_pos*180/np.pi:.0f}° (H₂-air)', fontsize=14, fontweight='bold')
        
        # Temperature plot
        c1 = ax1.contourf(X*1000, Y*1000, temperature, levels=20, cmap='hot')
        ax1.set_title('Temperature Field (K)')
        ax1.set_xlabel('X (mm)')
        ax1.set_ylabel('Y (mm)')
        ax1.set_aspect('equal')
        
        # Add geometry boundaries
        circle_inner = plt.Circle((0, 0), inner_radius*1000, fill=False, color='black', linewidth=2)
        circle_outer = plt.Circle((0, 0), outer_radius*1000, fill=False, color='black', linewidth=2)
        ax1.add_patch(circle_inner)
        ax1.add_patch(circle_outer)
        
        # Add colorbar
        plt.colorbar(c1, ax=ax1, shrink=0.8)
        
        # Pressure plot
        c2 = ax2.contourf(X*1000, Y*1000, pressure/1e6, levels=15, cmap='viridis')
        ax2.set_title('Pressure Field (MPa)')
        ax2.set_xlabel('X (mm)')
        ax2.set_ylabel('Y (mm)')
        ax2.set_aspect('equal')
        
        circle_inner2 = plt.Circle((0, 0), inner_radius*1000, fill=False, color='white', linewidth=2)
        circle_outer2 = plt.Circle((0, 0), outer_radius*1000, fill=False, color='white', linewidth=2)
        ax2.add_patch(circle_inner2)
        ax2.add_patch(circle_outer2)
        
        # Add colorbar
        plt.colorbar(c2, ax=ax2, shrink=0.8)
        
        # Add educational annotations
        educational_text = f'''Frame {i+1}: Educational Notes
• Detonation velocity: 1968 m/s
• Maximum temperature: {temperature.max():.0f} K
• Maximum pressure: {pressure.max()/1e6:.1f} MPa
• Wave position: {wave_pos*180/np.pi:.0f}°
• Physics: Chapman-Jouguet detonation'''
        
        fig.text(0.02, 0.02, educational_text, fontsize=10, 
                bbox=dict(boxstyle="round,pad=0.3", facecolor="lightblue", alpha=0.8))
        
        plt.tight_layout()
        
        # Save frame
        frame_file = output_dir / f"rde_frame_{i+1:02d}.png"
        plt.savefig(frame_file, dpi=150, bbox_inches='tight')
        plt.close()
        
        print(f"    ✅ Saved: {frame_file}")
    
    # Create summary image showing all frames
    print("  📋 Creating summary overview...")
    
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    fig.suptitle('2D RDE Wave Propagation Sequence (H₂-air)', fontsize=16, fontweight='bold')
    
    for i, wave_pos in enumerate(wave_positions):
        row = i // 3
        col = i % 3
        ax = axes[row, col]
        
        temperature = 300 + 3200 * np.exp(-((THETA - wave_pos) % (2*np.pi))**2 / 0.1)
        
        c = ax.contourf(X*1000, Y*1000, temperature, levels=15, cmap='hot')
        ax.set_title(f'Frame {i+1}: {wave_pos*180/np.pi:.0f}°')
        ax.set_aspect('equal')
        ax.set_xlabel('X (mm)')
        ax.set_ylabel('Y (mm)')
        
        # Add boundaries
        circle_inner = plt.Circle((0, 0), inner_radius*1000, fill=False, color='black', linewidth=1)
        circle_outer = plt.Circle((0, 0), outer_radius*1000, fill=False, color='black', linewidth=1)
        ax.add_patch(circle_inner)
        ax.add_patch(circle_outer)
    
    plt.tight_layout()
    summary_file = output_dir / "rde_sequence_overview.png"
    plt.savefig(summary_file, dpi=150, bbox_inches='tight')
    plt.close()
    
    print(f"  ✅ Summary saved: {summary_file}")
    
    # Save technical specifications
    specs = {
        "title": "2D RDE Visualization Frames",
        "description": "Static frames showing hydrogen-air detonation wave propagation in annular combustor",
        "geometry": {
            "inner_radius_mm": inner_radius * 1000,
            "outer_radius_mm": outer_radius * 1000,
            "annular_width_mm": (outer_radius - inner_radius) * 1000
        },
        "physics": {
            "fuel": "Hydrogen (H2)",
            "oxidizer": "Air",
            "equivalence_ratio": 1.0,
            "detonation_velocity_ms": 1968,
            "max_temperature_K": 3500,
            "max_pressure_MPa": 2.5,
            "cell_size_mm": 1.0
        },
        "frames_generated": len(wave_positions) + 1,  # +1 for summary
        "wave_positions_deg": [pos*180/np.pi for pos in wave_positions]
    }
    
    with open(output_dir / "frame_specifications.json", "w") as f:
        json.dump(specs, f, indent=2)
    
    print(f"📊 Specifications saved: {output_dir}/frame_specifications.json")
    print(f"\n🎉 Frame generation complete!")
    print(f"📁 Output directory: {output_dir}/")
    print(f"📸 Generated {len(wave_positions)} individual frames + 1 overview")
    
    # List all generated files
    print(f"\n📋 Generated Files:")
    for file in sorted(output_dir.glob("*")):
        print(f"  📄 {file.name}")
    
    return output_dir

if __name__ == "__main__":
    create_rde_frames()