
# ParaView Python Script for 2D RDE Production Video
# Field: pressure, Type: shock_wave

import paraview.simple as pv
import os

# Get active view
renderView = pv.GetActiveViewOrCreate('RenderView')
renderView.ViewSize = [1920, 1080]
renderView.Background = [0.1, 0.1, 0.2]

# Load OpenFOAM case
case_path = "/workspaces/openfoam-mcp-server/production_2d_rde/production_2d_rde.foam"
if not os.path.exists(case_path):
    # Create foam file
    with open(case_path, 'w') as f:
        f.write("")

reader = pv.OpenFOAMReader(FileName=case_path)
reader.MeshRegions = ['internalMesh']
reader.CellArrays = ['pressure', 'p', 'T', 'U', 'H2', 'O2', 'rho']

# Get time values
reader.UpdatePipeline()
timeSteps = reader.TimestepValues if reader.TimestepValues else [0.0]

# Display mesh
display = pv.Show(reader, renderView)
display.Representation = 'Surface'

# Configure field visualization
if 'pressure' == 'T':
    # Temperature visualization
    display.ColorArrayName = ['CELLS', 'T']
    colorMap = pv.GetColorTransferFunction('T')
    colorMap.ApplyPreset('Rainbow', True)
    colorMap.RescaleTransferFunction(300, 3000)  # 300K to 3000K
    
    # Add colorbar
    colorBar = pv.GetScalarBar(colorMap, renderView)
    colorBar.Title = 'Temperature (K)'
    colorBar.ComponentTitle = ''
    
elif 'pressure' == 'p':
    # Pressure visualization
    display.ColorArrayName = ['CELLS', 'p']
    colorMap = pv.GetColorTransferFunction('p')
    colorMap.ApplyPreset('Viridis', True)
    colorMap.RescaleTransferFunction(100000, 3000000)  # 1 to 30 bar
    
    colorBar = pv.GetScalarBar(colorMap, renderView)
    colorBar.Title = 'Pressure (Pa)'
    colorBar.ComponentTitle = ''
    
elif 'pressure' == 'H2':
    # H2 concentration
    display.ColorArrayName = ['CELLS', 'H2']
    colorMap = pv.GetColorTransferFunction('H2')
    colorMap.ApplyPreset('Cool to Warm', True)
    colorMap.RescaleTransferFunction(0.0, 0.3)  # 0 to 30%
    
    colorBar = pv.GetScalarBar(colorMap, renderView)
    colorBar.Title = 'H2 Concentration'
    colorBar.ComponentTitle = ''

# Set camera for 2D annular view
camera = renderView.GetActiveCamera()
camera.SetPosition(0, 0, 0.5)
camera.SetFocalPoint(0, 0, 0)
camera.SetViewUp(0, 1, 0)
renderView.ResetCamera()

# Add title annotation
title = pv.Text()
title.Text = '2D Rotating Detonation Engine - PRESSURE Field\nReal CFD Simulation Results'
titleDisplay = pv.Show(title, renderView)
titleDisplay.WindowLocation = 'UpperCenter'
titleDisplay.FontSize = 16
titleDisplay.Color = [1.0, 1.0, 1.0]

# Animation setup
animation = pv.GetAnimationScene()
animation.PlayMode = 'Sequence'
animation.StartTime = timeSteps[0] if timeSteps else 0.0
animation.EndTime = timeSteps[-1] if len(timeSteps) > 1 else 1.0
animation.NumberOfFrames = len(timeSteps)

# Save animation
output_path = "production_videos/rde_pressure_shock_wave.avi"
pv.SaveAnimation(output_path, renderView, 
                FrameRate=24,
                Quality=2)  # High quality

print(f"Animation saved: {output_path}")
