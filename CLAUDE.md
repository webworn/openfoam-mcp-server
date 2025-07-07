markdown# OpenFOAM MCP Server - Claude Code Project Context

## Project Overview

This project is building the **world's first native OpenFOAM Model Context Protocol (MCP) server** - a revolutionary integration that enables AI assistants to interact directly with computational fluid dynamics (CFD) simulations using OpenFOAM's C++ API.

### Mission Statement
Create a physics-aware AI system that understands CFD concepts, speaks OpenFOAM natively, and provides expert-level guidance for fluid dynamics simulations.

## Technical Architecture

### Core Technology Stack
- **Language**: C++20 with OpenFOAM integration
- **OpenFOAM Version**: v12 (latest stable release from OpenFOAM Foundation)
- **Protocol**: Model Context Protocol (JSON-RPC 2.0 based)
- **Build System**: CMake with custom OpenFOAM library linking
- **Environment**: GitHub Codespaces + VS Code Desktop
- **Dependencies**: nlohmann/json, SQLite3, Boost libraries

### Key Design Principles
1. **OpenFOAM-Native**: Built as extension of OpenFOAM, not wrapper around it
2. **Physics-Aware**: Understands CFD concepts, validates physics, applies best practices
3. **Zero-Copy Integration**: Direct memory access to OpenFOAM objects (mesh, fields, etc.)
4. **Domain Expert System**: Embeds 20+ years of CFD knowledge and best practices
5. **Real-Time Capability**: Live monitoring and manipulation of running simulations

## OpenFOAM Integration Details

### Core OpenFOAM Classes Used
```cpp
// Primary OpenFOAM objects we work with:
Foam::Time          // Simulation time management
Foam::fvMesh        // Finite volume mesh
Foam::volScalarField // Scalar fields (pressure, temperature)
Foam::volVectorField // Vector fields (velocity)
Foam::dictionary    // OpenFOAM configuration files
Foam::boundaryMesh  // Boundary conditions
Foam::fvSolution    // Numerical solution settings
Foam::fvSchemes     // Discretization schemes
OpenFOAM Environment Variables
bashFOAM_SRC      # OpenFOAM source directory: /usr/lib/openfoam/openfoam12/src
FOAM_LIBBIN   # OpenFOAM compiled libraries: /usr/lib/openfoam/openfoam12/lib
FOAM_TUTORIALS # Example cases directory: /usr/share/openfoam/tutorials  
WM_PROJECT_DIR # OpenFOAM installation root: /usr/lib/openfoam/openfoam12
FOAM_ETC      # Configuration directory: /usr/lib/openfoam/openfoam12/etc
Compilation Requirements
cpp// Required compiler definitions for OpenFOAM compatibility:
#define WM_DP                // Double precision
#define WM_LABEL_SIZE=32     // Label size  
#define NoRepository         // Disable repository features

// Required includes for OpenFOAM integration:
#include "fvCFD.H"          // Main OpenFOAM header
#include "Time.H"           // Time management
#include "fvMesh.H"         // Mesh handling
#include "volFields.H"      // Field operations
MCP Protocol Implementation
MCP Server Capabilities
The server implements these MCP protocol features:
Tools (AI-callable functions)

create_case_from_physics - Physics-aware case generation
setup_incompressible_flow - Configure incompressible solvers
setup_compressible_flow - Configure compressible solvers
generate_mesh - Mesh generation and quality checking
run_simulation - Execute solvers with monitoring
extract_field_data - Get field values at points/patches
calculate_forces - Compute forces and coefficients
analyze_convergence - Assess simulation convergence
optimize_parameters - Parameter studies and optimization

Resources (AI-accessible data)

cases/ - Active simulation cases with metadata
templates/ - Physics-based case templates
results/ - Simulation results and visualizations
solvers/ - Solver selection guidance
schemes/ - Numerical scheme recommendations
validation/ - Benchmark cases and validation data

MCP JSON-RPC Message Examples
json// Tool call for case creation
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "create_case_from_physics",
    "arguments": {
      "physics_type": "incompressible",
      "geometry": "cylinder",
      "reynolds_number": 100,
      "boundary_conditions": {
        "inlet": {"type": "fixedValue", "value": [1, 0, 0]},
        "outlet": {"type": "zeroGradient"},
        "wall": {"type": "noSlip"}
      }
    }
  }
}

// Resource request for case data
{
  "jsonrpc": "2.0", 
  "method": "resources/read",
  "params": {
    "uri": "cases/cylinder_re100/results/forces.dat"
  }
}
CFD Domain Knowledge
Physics Models Supported

Incompressible Flow: Navier-Stokes equations for Ma << 0.3
Compressible Flow: Full compressible Navier-Stokes
Turbulent Flow: RANS models (k-ε, k-ω SST, Reynolds Stress)
Heat Transfer: Conjugate heat transfer, natural/forced convection
Multiphase: VOF, Eulerian-Eulerian, Lagrangian particle tracking

Solver Selection Logic
cpp// Physics-based solver selection
if (physics.mach_number < 0.3) {
    if (physics.is_transient) {
        return "pisoFoam";  // Unsteady incompressible
    } else {
        return "simpleFoam"; // Steady incompressible  
    }
} else if (physics.mach_number > 1.0) {
    return "rhoCentralFoam"; // Supersonic compressible
} else {
    return "rhoPimpleFoam";  // Subsonic compressible
}
Mesh Quality Criteria

Aspect Ratio: < 100 for most cases
Skewness: < 0.6 (< 0.4 preferred)
Non-orthogonality: < 70° (< 45° preferred)
y+ Values: < 1 for wall-resolved, 30-300 for wall functions

Boundary Condition Best Practices
cpp// Common boundary condition patterns
inlet_velocity = {
    "type": "fixedValue",
    "value": velocity_vector
};

outlet_pressure = {
    "type": "fixedValue", 
    "value": reference_pressure
};

wall_velocity = {
    "type": "noSlip"  // or "slip" for inviscid
};

symmetry_plane = {
    "type": "symmetryPlane"
};
Development Context
Project Structure
src/
├── mcp/
│   ├── server.hpp/cpp      # Core MCP server implementation
│   ├── protocol.hpp/cpp    # JSON-RPC message handling
│   └── transport.hpp/cpp   # HTTP/WebSocket transport
├── openfoam/
│   ├── case_manager.hpp/cpp    # OpenFOAM case manipulation
│   ├── solver_runner.hpp/cpp   # Solver execution and monitoring
│   ├── field_monitor.hpp/cpp   # Real-time field access
│   └── physics_validator.hpp/cpp # CFD physics validation
├── tools/
│   ├── incompressible_flow.cpp # Incompressible flow setup
│   ├── mesh_generation.cpp     # Mesh creation and quality
│   └── post_processing.cpp     # Results analysis
└── main.cpp                    # Application entry point
Build Configuration
cmake# Key CMake settings for OpenFOAM integration
find_library(OPENFOAM_LIB OpenFOAM PATHS /usr/lib/openfoam/openfoam12/lib)
find_library(FINITE_VOLUME_LIB finiteVolume PATHS /usr/lib/openfoam/openfoam12/lib)

include_directories(/usr/lib/openfoam/openfoam12/src)
include_directories(/usr/lib/openfoam/openfoam12/src/finiteVolume/lnInclude)

add_compile_definitions(WM_DP WM_LABEL_SIZE=32 NoRepository)
Common Development Tasks

Adding new MCP tools: Implement in src/tools/ with OpenFOAM integration
Physics validation: Use PhysicsValidator class for consistency checking
Error handling: OpenFOAM uses FatalError for critical errors
Field manipulation: Direct access to OpenFOAM field objects for zero-copy operations
Case templates: Store reusable case configurations in templates/

AI Assistant Guidelines
When Helping with This Project

Understand OpenFOAM Context: This is not generic C++ - it's specialized CFD software
Respect Physics: Always validate CFD physics and suggest realistic parameters
Use OpenFOAM Patterns: Follow OpenFOAM coding conventions and object lifecycle
Consider Performance: OpenFOAM deals with large datasets - memory efficiency matters
Think Like CFD Engineer: Provide domain-expert level guidance

Code Generation Guidelines
cpp// Good OpenFOAM integration pattern:
class IncompressibleFlowTool {
    autoPtr<Time> runTime_;
    autoPtr<fvMesh> mesh_;
    
public:
    void setupCase(const FlowParameters& params) {
        // Validate physics first
        if (params.mach_number > 0.3) {
            FatalError << "Incompressible assumption violated" << exit(FatalError);
        }
        
        // Use OpenFOAM objects directly
        runTime_ = autoPtr<Time>::New(/* args */);
        mesh_ = autoPtr<fvMesh>::New(/* args */);
        
        // Apply CFD best practices
        setupBoundaryConditions(params);
        setupNumericalSchemes(params);
        validateMeshQuality();
    }
};
Areas Where AI Assistance is Most Valuable

Architecture Design: Help structure MCP tools and OpenFOAM integration
Physics Validation: Review CFD setups for correctness
Performance Optimization: Suggest efficient OpenFOAM usage patterns
Error Handling: Debug OpenFOAM compilation and runtime issues
Code Generation: Create boilerplate for new MCP tools
Best Practices: Apply OpenFOAM and MCP development patterns
Documentation: Generate API docs and usage examples

Success Metrics
Technical Goals

Zero-copy field access: Direct OpenFOAM object manipulation
Real-time monitoring: Sub-second response to simulation queries
Physics validation: Catch invalid configurations before simulation
Expert guidance: Provide CFD engineer-level recommendations
Scalability: Handle large meshes (1M+ cells) efficiently

User Experience Goals

Natural language: "Set up turbulent flow over airfoil at Re=1M"
Intelligent defaults: Automatically select appropriate solvers/schemes
Error prevention: Validate physics before wasting compute time
Performance optimization: Suggest improvements for faster simulation

Competitive Advantages

First-mover: No existing OpenFOAM MCP servers
Native integration: Impossible to replicate with external processes
Domain expertise: 20+ years of CFD knowledge embedded
Performance: 10-100x faster than subprocess approaches
AI-native design: Built specifically for AI interaction patterns


This project represents the convergence of artificial intelligence and computational fluid dynamics - enabling AI systems to understand and manipulate the physics of fluid flow with unprecedented sophistication.

### **File 3: `.devcontainer/devcontainer.json`**
Create `.devcontainer/devcontainer.json`:

```json
{
  "name": "OpenFOAM MCP Development",
  "image": "ubuntu:22.04",
  "runArgs": [
    "--privileged"
  ],
  "features": {
    "ghcr.io/devcontainers/features/common-utils:2": {},
    "ghcr.io/devcontainers/features/git:1": {},
    "ghcr.io/devcontainers/features/github-cli:1": {}
  },
  "customizations": {
    "vscode": {
      "extensions": [
        "ms-vscode.cpptools-extension-pack",
        "ms-vscode.cmake-tools",
        "ms-vscode.makefile-tools",
        "github.vscode-github-actions",
        "ms-vscode.hexeditor"
      ],
      "settings": {
        "C_Cpp.default.includePath": [
          "/usr/lib/openfoam/openfoam12/src/**",
          "${workspaceFolder}/src/**"
        ],
        "C_Cpp.default.defines": [
          "WM_DP",
          "WM_LABEL_SIZE=32",
          "NoRepository"
        ],
        "C_Cpp.default.compilerPath": "/usr/bin/g++",
        "C_Cpp.default.cppStandard": "c++20",
        "files.associations": {
          "*.H": "cpp",
          "*.C": "cpp"
        }
      }
    }
  },
  "postCreateCommand": "bash .devcontainer/setup.sh",
  "forwardPorts": [8080, 3000],
  "mounts": [
    "source=openfoam-cases,target=/home/openfoam/cases,type=volume"
  ]
}