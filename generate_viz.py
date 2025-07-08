#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, "/usr/lib/python3/dist-packages")

# Set up offscreen rendering
import paraview
paraview.compatibility.major = 5
paraview.compatibility.minor = 11

import paraview.simple as pvs

# Configure for headless/offscreen rendering
# OSPRay plugin not available in this installation

# Disable first render camera reset
pvs._DisableFirstRenderCameraReset()

# Change to cavity directory
os.chdir("/workspaces/openfoam-mcp-server/results/cavity")

print("Starting visualization generation...")
print("Current directory:", os.getcwd())

try:
    # Load OpenFOAM case
    print("Loading OpenFOAM case...")
    reader = pvs.OpenFOAMReader(FileName="cavity.foam")
    reader.MeshRegions = ["internalMesh"]
    reader.CellArrays = ["U", "p"]
    
    print("Setting up view...")
    # Create view
    view = pvs.GetActiveViewOrCreate('RenderView')
    view.ViewSize = [800, 600]
    view.Background = [1, 1, 1]  # White background
    
    print("Creating representation...")
    # Show the data
    rep = pvs.Show(reader, view)
    
    print("Setting up velocity visualization...")
    # Color by velocity magnitude
    pvs.ColorBy(rep, ("POINTS", "U", "Magnitude"))
    
    # Get color transfer function
    lut = pvs.GetColorTransferFunction("U")
    lut.RescaleTransferFunction(0.0, 1.0)
    
    # Reset camera and render
    pvs.ResetCamera(view)
    pvs.Render(view)
    
    print("Saving velocity magnitude image...")
    # Save velocity visualization
    pvs.SaveScreenshot("/workspaces/openfoam-mcp-server/results/velocity_magnitude.png", view)
    print("✅ Velocity magnitude visualization saved")
    
    print("Setting up pressure visualization...")
    # Color by pressure
    pvs.ColorBy(rep, ("CELLS", "p"))
    lut_p = pvs.GetColorTransferFunction("p")
    lut_p.RescaleTransferFunction(-0.5, 0.2)
    
    pvs.Render(view)
    pvs.SaveScreenshot("/workspaces/openfoam-mcp-server/results/pressure.png", view)
    print("✅ Pressure visualization saved")
    
    print("Creating streamlines...")
    # Create streamlines
    pvs.ColorBy(rep, ("POINTS", "U", "Magnitude"))
    
    # Create stream tracer
    stream = pvs.StreamTracer(Input=reader, SeedType="Line")
    stream.SeedType.Point1 = [0.01, 0.01, 0.005]  
    stream.SeedType.Point2 = [0.01, 0.09, 0.005]
    stream.SeedType.Resolution = 10
    
    # Show streamlines
    stream_rep = pvs.Show(stream, view)
    pvs.ColorBy(stream_rep, ("POINTS", "U", "Magnitude"))
    stream_rep.LineWidth = 3.0
    # Note: TubeRadius may not be available in this ParaView version
    
    # Hide the mesh and show only streamlines
    pvs.Hide(reader, view)
    
    pvs.ResetCamera(view)
    pvs.Render(view)
    pvs.SaveScreenshot("/workspaces/openfoam-mcp-server/results/streamlines.png", view)
    print("✅ Streamlines visualization saved")
    
    print("🎉 All visualizations generated successfully!")
    
except Exception as e:
    print(f"❌ Error generating visualization: {e}")
    import traceback
    traceback.print_exc()