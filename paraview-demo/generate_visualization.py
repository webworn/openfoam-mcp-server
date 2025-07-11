#!/usr/bin/env python3
"""
ParaView headless visualization generator for OpenFOAM cases
Demonstrates MCP server visualization capabilities
"""

import sys
import os
sys.path.insert(0, '/usr/lib/python3/dist-packages')

def generate_sample_visualization():
    """Generate a sample visualization to demonstrate ParaView capabilities"""
    try:
        # Import ParaView and set up for headless rendering
        import paraview
        paraview.options.offscreen = True
        import paraview.simple as pv
        print("✅ ParaView Simple module loaded successfully")
        
        # Create a simple sphere source for demonstration
        sphere = pv.Sphere()
        sphere.Radius = 1.0
        sphere.ThetaResolution = 32
        sphere.PhiResolution = 16
        
        # Create a render view
        renderView = pv.CreateView('RenderView')
        renderView.ViewSize = [800, 600]
        renderView.Background = [0.32, 0.34, 0.43]  # Nice blue-gray background
        
        # Display the sphere
        sphereDisplay = pv.Show(sphere, renderView)
        sphereDisplay.Representation = 'Surface'
        sphereDisplay.ColorArrayName = [None, '']
        sphereDisplay.DiffuseColor = [0.8, 0.2, 0.2]  # Red color
        
        # Reset camera to fit the object
        renderView.ResetCamera()
        
        # Generate the image
        output_file = '/workspaces/openfoam-mcp-server/paraview-demo/sample_visualization.png'
        pv.SaveScreenshot(output_file, renderView, ImageResolution=[800, 600])
        
        print(f"✅ Visualization saved to: {output_file}")
        return True
        
    except ImportError as e:
        print(f"❌ Import error: {e}")
        return False
    except Exception as e:
        print(f"❌ Error generating visualization: {e}")
        return False

def create_openfoam_cavity_case():
    """Create a simple OpenFOAM cavity case for demonstration"""
    import subprocess
    
    print("🔧 Creating sample OpenFOAM cavity case...")
    
    # Create cavity case directory
    case_dir = '/workspaces/openfoam-mcp-server/paraview-demo/cavity'
    os.makedirs(case_dir, exist_ok=True)
    
    # Copy cavity tutorial case
    try:
        # Source OpenFOAM environment and copy tutorial
        cmd = f"source /opt/openfoam12/etc/bashrc && cp -r $FOAM_TUTORIALS/incompressible/icoFoam/cavity/cavity/* {case_dir}/"
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✅ Cavity case created in: {case_dir}")
            return case_dir
        else:
            print(f"❌ Error creating cavity case: {result.stderr}")
            return None
            
    except Exception as e:
        print(f"❌ Error: {e}")
        return None

if __name__ == "__main__":
    print("🚀 ParaView MCP Server Visualization Demo")
    print("=" * 50)
    
    # Test basic ParaView functionality
    print("\n1. Testing ParaView headless rendering...")
    if generate_sample_visualization():
        print("✅ ParaView headless rendering working!")
    else:
        print("❌ ParaView headless rendering failed")
    
    # Create OpenFOAM demo case
    print("\n2. Setting up OpenFOAM demo case...")
    case_dir = create_openfoam_cavity_case()
    
    if case_dir:
        print("✅ Ready for OpenFOAM-ParaView integration demo!")
        print(f"   Case location: {case_dir}")
        print("   Next: Run the case and visualize with ParaView")
    else:
        print("❌ OpenFOAM case setup failed")
    
    print("\n🎯 Demo complete! Check the generated files.")