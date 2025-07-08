#!/usr/bin/env python3
"""
Simple Web Server for OpenFOAM ParaView Visualization
Serves visualization images on port 8080
"""

import os
import sys
import http.server
import socketserver
import threading
import time
import subprocess
from pathlib import Path

# Set environment for ParaView
os.environ['PYTHONPATH'] = '/usr/lib/python3/dist-packages'

def generate_visualizations():
    """Generate ParaView visualizations of the cavity case"""
    script_content = '''
import sys
sys.path.insert(0, "/usr/lib/python3/dist-packages")

import paraview.simple as pvs
import os

# Disable all output and warnings
pvs._DisableFirstRenderCameraReset()

# Change to cavity directory
os.chdir("/workspaces/openfoam-mcp-server/results/cavity")

try:
    # Load OpenFOAM case
    reader = pvs.OpenFOAMReader(FileName="cavity.foam")
    reader.MeshRegions = ["internalMesh"]
    reader.CellArrays = ["U", "p"]
    
    # Set the time step to the last one
    pvs.GetAnimationScene().GoToLast()
    
    # Create view
    view = pvs.CreateView("RenderView")
    view.ViewSize = [800, 600]
    view.Background = [1, 1, 1]  # White background
    
    # Show the data
    rep = pvs.Show(reader, view)
    
    # Color by velocity magnitude
    pvs.ColorBy(rep, ("POINTS", "U", "Magnitude"))
    
    # Get color transfer function and configure
    lut = pvs.GetColorTransferFunction("U")
    lut.RescaleTransferFunction(0.0, 1.0)
    
    # Add color bar
    colorbar = pvs.GetScalarBar(lut, view)
    colorbar.Title = "Velocity Magnitude (m/s)"
    colorbar.ComponentTitle = ""
    
    # Reset camera and render
    pvs.ResetCamera(view)
    pvs.Render(view)
    
    # Save velocity visualization
    pvs.SaveScreenshot("/workspaces/openfoam-mcp-server/results/velocity_magnitude.png")
    print("Velocity magnitude visualization saved")
    
    # Color by pressure
    pvs.ColorBy(rep, ("CELLS", "p"))
    lut_p = pvs.GetColorTransferFunction("p")
    lut_p.RescaleTransferFunction(-1.0, 0.5)
    
    colorbar.Title = "Pressure (Pa)"
    pvs.Render(view)
    pvs.SaveScreenshot("/workspaces/openfoam-mcp-server/results/pressure.png")
    print("Pressure visualization saved")
    
    # Create streamlines
    pvs.ColorBy(rep, ("POINTS", "U", "Magnitude"))
    
    # Create stream tracer
    stream = pvs.StreamTracer(Input=reader, SeedType="High Resolution Line Source")
    stream.SeedType.Point1 = [0.01, 0.01, 0.005]  
    stream.SeedType.Point2 = [0.01, 0.09, 0.005]
    stream.SeedType.Resolution = 10
    
    # Show streamlines
    stream_rep = pvs.Show(stream, view)
    stream_rep.ColorArrayName = ("POINTS", "U", "Magnitude")
    stream_rep.LineWidth = 2.0
    
    pvs.Hide(reader, view)  # Hide the mesh
    pvs.ResetCamera(view)
    pvs.Render(view)
    pvs.SaveScreenshot("/workspaces/openfoam-mcp-server/results/streamlines.png")
    print("Streamlines visualization saved")
    
    print("All visualizations generated successfully!")
    
except Exception as e:
    print(f"Error generating visualization: {e}")
    import traceback
    traceback.print_exc()
'''
    
    # Write and execute the script
    with open("/tmp/paraview_viz.py", "w") as f:
        f.write(script_content)
    
    # Run the visualization script
    result = subprocess.run([
        "python3", "/tmp/paraview_viz.py"
    ], capture_output=True, text=True, cwd="/workspaces/openfoam-mcp-server/results/cavity")
    
    print("Visualization generation output:")
    print("STDOUT:", result.stdout)
    if result.stderr:
        print("STDERR:", result.stderr)
    print("Return code:", result.returncode)

class OpenFOAMWebHandler(http.server.SimpleHTTPRequestHandler):
    """Custom HTTP handler for OpenFOAM visualization"""
    
    def do_GET(self):
        if self.path == '/':
            self.serve_index()
        elif self.path == '/generate':
            self.generate_viz()
        else:
            # Serve static files
            super().do_GET()
    
    def serve_index(self):
        """Serve the main visualization page"""
        html_content = '''<!DOCTYPE html>
<html>
<head>
    <title>OpenFOAM ParaView Visualization</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #2c3e50; text-align: center; }
        .viz-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(400px, 1fr)); gap: 20px; margin-top: 30px; }
        .viz-item { text-align: center; padding: 20px; border: 1px solid #ddd; border-radius: 8px; background: #fafafa; }
        .viz-item img { max-width: 100%; height: auto; border-radius: 5px; box-shadow: 0 2px 8px rgba(0,0,0,0.2); }
        .viz-item h3 { color: #34495e; margin-top: 15px; }
        .generate-btn { background: #3498db; color: white; padding: 12px 24px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; margin: 20px 0; }
        .generate-btn:hover { background: #2980b9; }
        .info { background: #e8f4fd; padding: 15px; border-radius: 5px; margin: 20px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌊 OpenFOAM Cavity Flow Visualization</h1>
        
        <div class="info">
            <strong>Case:</strong> 2D Driven Cavity Flow<br>
            <strong>Solver:</strong> incompressibleFluid<br>
            <strong>Reynolds Number:</strong> ~1000<br>
            <strong>Time:</strong> 10 seconds (steady state)
        </div>
        
        <button class="generate-btn" onclick="generateVisualization()">🔄 Generate New Visualizations</button>
        
        <div class="viz-grid">
            <div class="viz-item">
                <h3>Velocity Magnitude</h3>
                <img src="velocity_magnitude.png" alt="Velocity Magnitude" onerror="this.src='data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iNDAwIiBoZWlnaHQ9IjMwMCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cmVjdCB3aWR0aD0iMTAwJSIgaGVpZ2h0PSIxMDAlIiBmaWxsPSIjZjBmMGYwIi8+PHRleHQgeD0iNTAlIiB5PSI1MCUiIGZvbnQtZmFtaWx5PSJBcmlhbCIgZm9udC1zaXplPSIxOCIgZmlsbD0iIzk5OSIgdGV4dC1hbmNob3I9Im1pZGRsZSIgZHk9Ii4zZW0iPkNsaWNrIEdlbmVyYXRlPC90ZXh0Pjwvc3ZnPg=='"/>
                <p>Shows the magnitude of velocity vectors throughout the cavity</p>
            </div>
            
            <div class="viz-item">
                <h3>Pressure Field</h3>
                <img src="pressure.png" alt="Pressure Field" onerror="this.src='data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iNDAwIiBoZWlnaHQ9IjMwMCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cmVjdCB3aWR0aD0iMTAwJSIgaGVpZ2h0PSIxMDAlIiBmaWxsPSIjZjBmMGYwIi8+PHRleHQgeD0iNTAlIiB5PSI1MCUiIGZvbnQtZmFtaWx5PSJBcmlhbCIgZm9udC1zaXplPSIxOCIgZmlsbD0iIzk5OSIgdGV4dC1hbmNob3I9Im1pZGRsZSIgZHk9Ii4zZW0iPkNsaWNrIEdlbmVyYXRlPC90ZXh0Pjwvc3ZnPg=='"/>
                <p>Pressure distribution showing the flow-induced pressure gradients</p>
            </div>
            
            <div class="viz-item">
                <h3>Streamlines</h3>
                <img src="streamlines.png" alt="Streamlines" onerror="this.src='data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iNDAwIiBoZWlnaHQ9IjMwMCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cmVjdCB3aWR0aD0iMTAwJSIgaGVpZ2h0PSIxMDAlIiBmaWxsPSIjZjBmMGYwIi8+PHRleHQgeD0iNTAlIiB5PSI1MCUiIGZvbnQtZmFtaWx5PSJBcmlhbCIgZm9udC1zaXplPSIxOCIgZmlsbD0iIzk5OSIgdGV4dC1hbmNob3I9Im1pZGRsZSIgZHk9Ii4zZW0iPkNsaWNrIEdlbmVyYXRlPC90ZXh0Pjwvc3ZnPg=='"/>
                <p>Streamlines showing the characteristic vortex pattern</p>
            </div>
        </div>
        
        <div class="info" style="margin-top: 40px;">
            <h3>🔬 About This Visualization</h3>
            <p>This is a classic 2D driven cavity flow - a benchmark CFD case where the top wall moves with constant velocity creating a recirculating flow pattern. The OpenFOAM simulation shows the steady-state solution with the characteristic primary vortex and smaller corner vortices.</p>
        </div>
    </div>
    
    <script>
        function generateVisualization() {
            document.querySelector('.generate-btn').innerHTML = '⏳ Generating...';
            fetch('/generate')
                .then(response => response.text())
                .then(data => {
                    document.querySelector('.generate-btn').innerHTML = '🔄 Generate New Visualizations';
                    // Reload images by adding timestamp
                    const timestamp = new Date().getTime();
                    document.querySelectorAll('.viz-item img').forEach(img => {
                        const originalSrc = img.src.split('?')[0];
                        img.src = originalSrc + '?' + timestamp;
                    });
                    alert('Visualizations updated!');
                })
                .catch(error => {
                    document.querySelector('.generate-btn').innerHTML = '🔄 Generate New Visualizations';
                    alert('Error generating visualizations: ' + error);
                });
        }
    </script>
</body>
</html>'''
        
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(html_content.encode())
    
    def generate_viz(self):
        """Generate new visualizations"""
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        
        try:
            generate_visualizations()
            self.wfile.write(b"Visualizations generated successfully!")
        except Exception as e:
            self.wfile.write(f"Error: {str(e)}".encode())

def start_web_server():
    """Start the web server"""
    port = 8080
    
    # Change to the results directory to serve images
    os.chdir("/workspaces/openfoam-mcp-server/results")
    
    # Generate initial visualizations
    print("Generating initial visualizations...")
    generate_visualizations()
    
    # Start web server
    with socketserver.TCPServer(("0.0.0.0", port), OpenFOAMWebHandler) as httpd:
        print(f"🌐 OpenFOAM ParaView Server running on port {port}")
        print(f"🔗 Access at: http://localhost:{port}")
        print("📊 Serving OpenFOAM cavity flow visualizations")
        print("Press Ctrl+C to stop...")
        
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n🛑 Server stopped")

if __name__ == "__main__":
    start_web_server()