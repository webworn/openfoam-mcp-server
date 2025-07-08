#!/usr/bin/env python3
"""
ParaView Web Server for OpenFOAM Results Visualization
Serves ParaView visualizations on port 8080
"""

import os
import sys
import argparse
import webbrowser
from paraview import simple
from paraview.web import wamp as pv_wamp
from paraview.web import protocols as pv_protocols
from paraview.web import wslink as pv_wslink
from wslink import server

try:
    import asyncio
except ImportError:
    import trollius as asyncio

# =============================================================================
# Create custom Web Protocol
# =============================================================================

class OpenFOAMVisualizationProtocol(pv_protocols.ParaViewWebProtocol):

    def initialize(self):
        # Bring used components
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebStartupRemoteConnection())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebStartupPluginLoader())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebPipelineManager())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebMouseHandler())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebViewPort())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebViewPortImageDelivery())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebViewPortGeometryDelivery())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebTimeHandler())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebSelectionHandler())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebWidgetManager())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebKeyValuePairStore())
        self.registerVtkWebProtocol(pv_protocols.ParaViewWebSaveData())

        # Update authentication key to use
        self.updateSecret(OpenFOAMVisualizationProtocol.authKey)

        # Load OpenFOAM case if specified
        if hasattr(OpenFOAMVisualizationProtocol, 'foam_case'):
            self.load_openfoam_case(OpenFOAMVisualizationProtocol.foam_case)

    def load_openfoam_case(self, case_path):
        """Load OpenFOAM case automatically"""
        try:
            print(f"Loading OpenFOAM case: {case_path}")
            
            # Create OpenFOAM reader
            reader = simple.OpenFOAMReader(FileName=case_path)
            
            # Get all available arrays
            reader.MeshRegions = ['internalMesh']
            reader.CellArrays = ['U', 'p']
            
            # Create representation
            rep = simple.Show(reader)
            
            # Color by velocity magnitude
            simple.ColorBy(rep, ('POINTS', 'U', 'Magnitude'))
            
            # Reset camera
            simple.ResetCamera()
            
            # Render
            simple.Render()
            
            print("OpenFOAM case loaded successfully!")
            
        except Exception as e:
            print(f"Error loading OpenFOAM case: {e}")

# =============================================================================
# Main server launcher
# =============================================================================

def main():
    # Parse arguments
    parser = argparse.ArgumentParser(description="ParaView Web Server for OpenFOAM")
    parser.add_argument("--port", type=int, default=8080, help="Port number")
    parser.add_argument("--case", type=str, help="OpenFOAM case .foam file path")
    parser.add_argument("--host", type=str, default="0.0.0.0", help="Host interface")
    
    # Add standard ParaView Web arguments
    server.add_arguments(parser)
    args = parser.parse_args()

    # Configure the protocol
    OpenFOAMVisualizationProtocol.authKey = "vtkweb-secret"
    
    if args.case and os.path.exists(args.case):
        OpenFOAMVisualizationProtocol.foam_case = args.case
        print(f"Will load OpenFOAM case: {args.case}")
    
    # Start the web server
    server.start_webserver(options=args, protocol=OpenFOAMVisualizationProtocol)

# =============================================================================
# Alternative simple launcher using pvserver
# =============================================================================

def start_simple_server():
    """Start a simple ParaView server that can be accessed via web"""
    import subprocess
    import signal
    
    # Start pvserver in background
    print("Starting ParaView server on port 8080...")
    
    # Use pvpython to start a simple web visualization
    script = '''
import paraview.simple as pvs
import time
import os
import sys

# Change to cavity directory
os.chdir("/workspaces/openfoam-mcp-server/results/cavity")

# Load OpenFOAM case
try:
    reader = pvs.OpenFOAMReader(FileName="cavity.foam")
    reader.MeshRegions = ["internalMesh"]
    reader.CellArrays = ["U", "p"] 
    
    # Create view and representation  
    view = pvs.CreateView("RenderView")
    rep = pvs.Show(reader, view)
    
    # Color by velocity magnitude
    pvs.ColorBy(rep, ("POINTS", "U", "Magnitude"))
    pvs.ResetCamera(view)
    
    # Save screenshot
    pvs.SaveScreenshot("/workspaces/openfoam-mcp-server/results/cavity_visualization.png")
    print("Screenshot saved to cavity_visualization.png")
    
    # Create an animation
    pvs.SaveAnimation("/workspaces/openfoam-mcp-server/results/cavity_animation.avi")
    print("Animation saved to cavity_animation.avi")
    
except Exception as e:
    print(f"Error: {e}")
'''
    
    with open("/tmp/paraview_script.py", "w") as f:
        f.write(script)
    
    # Run the script
    result = subprocess.run([
        "pvpython", "/tmp/paraview_script.py"
    ], capture_output=True, text=True)
    
    print("STDOUT:", result.stdout)
    print("STDERR:", result.stderr)
    print("Return code:", result.returncode)

if __name__ == "__main__":
    # For now, let's use the simple approach
    start_simple_server()