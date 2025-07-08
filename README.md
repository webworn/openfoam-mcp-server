# OpenFOAM MCP Server

🚀 **The world's first native OpenFOAM Model Context Protocol server** - enabling AI assistants to perform computational fluid dynamics (CFD) analysis with expert-level guidance.

[![OpenFOAM](https://img.shields.io/badge/OpenFOAM-12-blue)](https://openfoam.org/)
[![C++](https://img.shields.io/badge/C++-20-green)](https://isocpp.org/)
[![MCP](https://img.shields.io/badge/MCP-1.0-purple)](https://modelcontextprotocol.io/)
[![License](https://img.shields.io/badge/License-Apache%202.0-red)](LICENSE)

## 🎯 Why This MCP? The CFD-AI Gap

### The Problem: CFD Expertise Barrier
- **Complex Setup**: OpenFOAM requires deep expertise in mesh generation, boundary conditions, and solver selection
- **Physics Knowledge**: CFD analysis demands understanding of fluid dynamics, heat transfer, and numerical methods
- **Time-Intensive**: Manual setup and analysis can take hours to days for complex cases
- **Error-Prone**: Incorrect boundary conditions or solver settings lead to non-physical results

### The Solution: AI-Native CFD
This MCP bridges the gap between AI assistance and professional CFD tools by:
- **Physics-Aware AI**: First MCP server that understands OpenFOAM solvers and CFD principles
- **Expert Guidance**: Provides thermal design recommendations and aerodynamic optimization advice
- **Automated Setup**: Intelligent case creation with validated boundary conditions and solver settings
- **Educational**: Explains the physics behind results with dimensionless numbers and engineering insights

### Market Impact
- **Democratizes CFD**: Makes professional fluid dynamics analysis accessible to non-experts
- **Accelerates Design**: Reduces CFD setup time from hours to minutes
- **Enhances Learning**: Provides physics explanations alongside numerical results
- **Professional Integration**: Maintains full OpenFOAM compatibility for advanced users

## 🛠️ Capabilities

### Core Analysis Tools

#### 1. 🔬 **Pipe Flow Analysis** (`run_pipe_flow`)
**Physics**: Internal flow in circular pipes with laminar/turbulent regime detection
- **Calculates**: Reynolds number, friction factor, pressure drop, wall shear stress
- **Applications**: Hydraulic systems, pipeline design, HVAC analysis
- **Solvers**: `simpleFoam`, `pimpleFoam` with automated turbulence model selection
- **Features**: Entrance effects, surface roughness, flow regime transitions

#### 2. ✈️ **External Flow Analysis** (`analyze_external_flow`)
**Physics**: Aerodynamics around vehicles, aircraft, and buildings
- **Calculates**: Drag coefficient (Cd), lift coefficient (Cl), aerodynamic forces
- **Applications**: 
  - **Automotive**: Car aerodynamics, fuel efficiency optimization
  - **Aerospace**: Aircraft design, wing analysis, UAV performance
  - **Civil**: Building wind loads, urban flow patterns
- **Solvers**: `simpleFoam`, `SRFSimpleFoam` with k-ω SST turbulence
- **Features**: Reynolds number scaling, boundary layer analysis, flow separation detection

#### 3. 🌡️ **Heat Transfer Analysis** (`analyze_heat_transfer`)
**Physics**: Conjugate heat transfer between fluid and solid regions
- **Calculates**: Temperature distributions, thermal resistance, heat transfer coefficients
- **Applications**:
  - **Electronics Cooling**: CPU/GPU thermal management, PCB heat spreading
  - **Heat Exchangers**: Shell-and-tube, plate heat exchangers
  - **Building Thermal**: HVAC design, thermal comfort analysis
  - **Engine Cooling**: Combustion engines, electric motor cooling
- **Solvers**: `chtMultiRegionFoam` with radiation modeling
- **Features**: Material property database, Nusselt number correlations, thermal violations

### Advanced Features

#### 🧮 **Physics-Based Calculations**
- **Dimensionless Numbers**: Reynolds, Prandtl, Nusselt, Rayleigh, Grashof
- **Material Properties**: Thermal conductivity, density, specific heat, viscosity
- **Validation**: Analytical comparisons and engineering rule checks
- **Error Detection**: Non-physical results and convergence warnings

#### 🎓 **Educational Content**
- **Physics Explanations**: Detailed description of flow phenomena
- **Engineering Guidance**: Design recommendations and optimization strategies
- **Application Context**: Industry-specific advice for automotive, aerospace, HVAC
- **Best Practices**: Mesh quality, boundary conditions, solver selection

#### 📊 **Visualization Integration**
- **ParaView Web Interface**: Real-time result visualization on port 8080
- **Automated Plots**: Velocity contours, pressure distributions, temperature fields
- **Streamlines**: Flow visualization with particle tracking
- **Result Export**: VTK format for advanced post-processing

## 📥 Installation

### Prerequisites
Ensure OpenFOAM 12 is installed on your system:
```bash
# Ubuntu/Debian
sudo apt-get install openfoam12

# Or using OpenFOAM Foundation packages
# Follow: https://openfoam.org/download/
```

### Build MCP Server

1. **Clone Repository**
```bash
git clone https://github.com/webworn/openfoam-mcp-server.git
cd openfoam-mcp-server
```

2. **Build with CMake**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

3. **Test Installation**
```bash
./openfoam-mcp-server --version
# Should output: OpenFOAM MCP Server v1.0.0
```

### Dependencies Auto-Detection
The build system automatically detects:
- ✅ **OpenFOAM Libraries**: `/opt/openfoam12/platforms/linux64GccDPInt32Opt/lib`
- ✅ **nlohmann/json**: JSON parsing for MCP protocol
- ✅ **Boost**: System and threading libraries
- ✅ **SQLite3**: Optional case database storage

## 🤖 Claude Code Integration

### MCP Configuration

Add to your Claude Code settings (`~/.config/claude-code/mcp_servers.json`):

```json
{
  "mcpServers": {
    "openfoam": {
      "command": "/path/to/openfoam-mcp-server/build/openfoam-mcp-server",
      "args": [],
      "env": {
        "FOAM_INST_DIR": "/opt/openfoam12",
        "WM_PROJECT_DIR": "/opt/openfoam12"
      }
    }
  }
}
```

### Usage Examples

#### Basic Pipe Flow Analysis
```
🧑‍💻 User: "Analyze water flow in a 0.1m diameter pipe at 2 m/s"

🤖 Claude: I'll run a pipe flow analysis for you.
*Uses run_pipe_flow tool*

📊 Results:
• Reynolds Number: 200,000 (Turbulent flow)
• Friction Factor: 0.0184 (Blasius correlation)
• Pressure Drop: 2.4 kPa per meter
• Wall Shear Stress: 4.8 Pa

💡 Recommendations:
• High pressure drop - consider 0.15m diameter for 50% reduction
• Turbulent mixing enhances heat transfer
• Consider L/D > 10 for fully developed flow
```

#### Automotive Aerodynamics
```
🧑‍💻 User: "Calculate drag coefficient for a sedan at highway speeds"

🤖 Claude: I'll analyze the external flow around a sedan.
*Uses analyze_external_flow tool*

🚗 Results:
• Drag Coefficient (Cd): 0.32
• Frontal Area: 2.2 m²
• Drag Force: 312 N at 100 km/h
• Fuel Impact: ~15% of energy at highway speeds

🎯 Optimization:
• Rear spoiler could reduce Cd by 0.02-0.05
• Underbody panels improve airflow management
• Side mirror design contributes 3-5% of total drag
```

#### Electronics Thermal Management
```
🧑‍💻 User: "Design cooling for a 50W CPU with air cooling"

🤖 Claude: I'll analyze the conjugate heat transfer.
*Uses analyze_heat_transfer tool*

🌡️ Results:
• Junction Temperature: 78°C (within 85°C limit)
• Thermal Resistance: 1.2 K/W (junction-to-ambient)
• Heat Transfer: 45W convection, 5W radiation
• Cooling Effectiveness: 92%

⚠️ Recommendations:
• Current design adequate for 50W
• For >60W, consider heat pipes or liquid cooling
• Improve thermal interface material (TIM)
• Add case ventilation for ambient cooling
```

### Workflow Integration

#### Engineering Design Process
1. **Conceptual Design**: Use MCP for initial sizing and feasibility
2. **Detailed Analysis**: Export OpenFOAM cases for advanced simulation
3. **Optimization**: Iterate with AI guidance on design improvements
4. **Validation**: Compare results with analytical correlations

#### Educational Use
1. **Physics Learning**: Understand CFD principles through practical examples
2. **Parameter Studies**: Explore effects of Reynolds number, geometry
3. **Best Practices**: Learn proper boundary conditions and solver selection
4. **Industry Applications**: Apply CFD to real-world engineering problems

## 🤝 Contributing

### Development Environment

**GitHub Codespaces** (Recommended):
1. Fork the repository
2. Open in Codespaces (auto-configures OpenFOAM 12)
3. VS Code Desktop integration with full IntelliSense
4. Cloud development with 4-32 core machines

**Local Development**:
```bash
# Install dependencies
sudo apt-get install cmake build-essential openfoam12 nlohmann-json3-dev

# Clone and build
git clone https://github.com/webworn/openfoam-mcp-server.git
cd openfoam-mcp-server
mkdir build && cd build
cmake .. && make -j$(nproc)
```

### Adding New Analysis Tools

#### 1. Create Analyzer Class
```cpp
// src/openfoam/your_analysis.hpp
class YourAnalyzer {
public:
    struct Input { /* physics parameters */ };
    struct Results { /* calculated outputs */ };
    
    Results analyze(const Input& input);
    bool validateInput(const Input& input);
    json toJson(const Results& results);
};
```

#### 2. Create MCP Tool Wrapper
```cpp
// src/tools/your_analysis_tool.hpp
class YourAnalysisTool {
public:
    static std::string getName() { return "your_analysis"; }
    static std::string getDescription() { return "Physics description"; }
    static json getInputSchema();
    ToolResult execute(const json& arguments);
};
```

#### 3. Register Tool
```cpp
// src/main.cpp
#include "tools/your_analysis_tool.hpp"

int main() {
    McpServer server;
    registerYourAnalysisTool(server);  // Add this line
    server.start();
}
```

### Physics Implementation Guidelines

#### OpenFOAM Integration
- **Case Templates**: Use `src/openfoam/case_templates/` for solver setup
- **Boundary Conditions**: Implement physics-appropriate BC selection
- **Mesh Generation**: Add automatic meshing with quality checks
- **Solver Selection**: Choose appropriate OpenFOAM solver for physics

#### Validation Requirements
- **Analytical Comparison**: Compare with known solutions (Poiseuille, Blasius, etc.)
- **Experimental Data**: Validate against published correlations
- **Convergence Studies**: Ensure mesh-independent results
- **Physical Bounds**: Check for non-physical results

#### Educational Content
- **Physics Explanations**: Describe governing equations and assumptions
- **Dimensionless Numbers**: Calculate and explain significance
- **Engineering Context**: Provide industry-relevant guidance
- **Design Recommendations**: Suggest optimization strategies

### Code Standards

#### C++ Best Practices
- **C++20 Standard**: Use modern C++ features appropriately
- **OpenFOAM Style**: Follow OpenFOAM coding conventions
- **Memory Management**: Use smart pointers, RAII principles
- **Error Handling**: Comprehensive exception handling

#### MCP Protocol Compliance
- **JSON-RPC 2.0**: Strict protocol adherence
- **Schema Validation**: Complete input parameter validation
- **Error Messages**: User-friendly error reporting
- **Resource Management**: Proper cleanup and case management

## 🏗️ Technical Architecture

### System Overview
```
┌─────────────────┐    JSON-RPC 2.0    ┌──────────────────┐
│   Claude Code   │ ◄──────────────── │  MCP Server      │
│   AI Assistant  │                   │  (C++ Native)    │
└─────────────────┘                   └──────────────────┘
                                               │
                                               ▼
                                    ┌──────────────────┐
                                    │   Tool Router    │
                                    │  (Registration)  │
                                    └──────────────────┘
                                               │
                         ┌─────────────────────┼─────────────────────┐
                         ▼                     ▼                     ▼
                ┌─────────────────┐   ┌─────────────────┐   ┌─────────────────┐
                │  Pipe Flow      │   │ External Flow   │   │ Heat Transfer   │
                │  Tool           │   │ Tool            │   │ Tool            │
                └─────────────────┘   └─────────────────┘   └─────────────────┘
                         │                     │                     │
                         ▼                     ▼                     ▼
                ┌─────────────────┐   ┌─────────────────┐   ┌─────────────────┐
                │ PipeFlow        │   │ ExternalFlow    │   │ HeatTransfer    │
                │ Analyzer        │   │ Analyzer        │   │ Analyzer        │
                └─────────────────┘   └─────────────────┘   └─────────────────┘
                         │                     │                     │
                         └─────────────────────┼─────────────────────┘
                                               ▼
                                    ┌──────────────────┐
                                    │  Case Manager    │
                                    │  (OpenFOAM)      │
                                    └──────────────────┘
                                               │
                                               ▼
                                    ┌──────────────────┐
                                    │    OpenFOAM      │
                                    │   Solvers        │
                                    │ (Native C++)     │
                                    └──────────────────┘
```

### Data Flow
1. **User Request**: Natural language query to Claude Code
2. **AI Processing**: Claude interprets request and calls appropriate MCP tool
3. **Parameter Validation**: Input schema validation and physics bounds checking
4. **OpenFOAM Setup**: Automated case creation with boundary conditions
5. **CFD Simulation**: Native OpenFOAM solver execution
6. **Result Processing**: Physics calculations and educational content generation
7. **Response**: Formatted results with explanations and recommendations

### Performance Features
- **Parallel Execution**: Multi-threaded OpenFOAM solvers
- **Case Caching**: Intelligent reuse of similar simulations
- **Mesh Templates**: Pre-validated meshes for common geometries
- **Async Processing**: Non-blocking simulation execution (planned)

## 📚 References

- **OpenFOAM Foundation**: [https://openfoam.org/](https://openfoam.org/)
- **Model Context Protocol**: [https://modelcontextprotocol.io/](https://modelcontextprotocol.io/)
- **CFD Theory**: "An Introduction to Computational Fluid Dynamics" by Versteeg & Malalasekera
- **Heat Transfer**: "Heat Transfer" by Cengel & Ghajar
- **Fluid Mechanics**: "Fluid Mechanics" by White

## 📄 License

Apache License 2.0 - see [LICENSE](LICENSE) for details.

## 🙏 Acknowledgments

- **OpenFOAM Foundation** for the world-class CFD toolkit
- **Anthropic** for Claude Code and MCP protocol
- **CFD Community** for validation data and best practices

## 🚀 Upcoming Features

*Interested in contributing? Join us in building the most comprehensive CFD-AI integration platform!*

### 🌊 **Expanded Physics Domains**

#### **Compressible Flow Analysis** (Coming Q2 2024)
- **Supersonic Aerodynamics**: Shock waves, expansion fans, nozzle design
- **Gas Dynamics**: Pressure wave propagation, transonic flows
- **Hypersonic Applications**: High-temperature effects, real gas properties

#### **Multiphase Flow Systems** (In Development)
- **Volume of Fluid (VOF)**: Free surface flows, dam breaks, sloshing tanks
- **Eulerian-Eulerian**: Bubble columns, fluidized beds, liquid-liquid extraction  
- **Lagrangian Particle Tracking**: Spray injection, aerosol transport, combustion

#### **Reacting Flows & Combustion** (Research Phase)
- **Premixed Combustion**: Flame propagation, explosion modeling, engine combustion
- **Non-Premixed**: Diffusion flames, jet flames, industrial flares
- **Solid Fuel**: Coal combustion, biomass burning, waste incineration

### 🏭 **Industry-Specific Tool Suites**

#### **🚗 Automotive Engineering**
```cpp
// Planned automotive tools
analyze_vehicle_aerodynamics(car_geometry, speed, crosswind_effects)
optimize_underhood_cooling(heat_sources, fan_design, airflow_management)
design_hvac_system(cabin_geometry, climate_conditions, comfort_criteria)
simulate_engine_combustion(cylinder_geometry, fuel_injection, emissions)
```

#### **✈️ Aerospace Engineering**
```cpp
// Planned aerospace tools  
analyze_airfoil_performance(airfoil_coordinates, mach_range, reynolds_range)
design_propulsion_system(engine_type, thrust_requirements, efficiency_targets)
simulate_hypersonic_vehicle(mach_number, altitude, thermal_protection)
optimize_wing_configuration(geometry_parameters, flight_conditions)
```

#### **⚡ Energy Systems**
```cpp
// Planned energy tools
design_wind_turbine(blade_geometry, wind_conditions, power_optimization)
analyze_solar_collector(collector_type, solar_irradiance, fluid_properties)
optimize_gas_turbine(blade_design, combustion_efficiency, emissions)
simulate_nuclear_reactor(coolant_flow, heat_generation, safety_systems)
```

#### **🏥 Biomedical Applications**
```cpp
// Planned biomedical tools
simulate_blood_flow(vessel_geometry, pulsatile_conditions, wall_properties)
analyze_drug_delivery(injection_site, transport_mechanisms, target_tissue)
design_medical_device(device_geometry, biocompatibility, flow_characteristics)
model_respiratory_system(lung_geometry, breathing_patterns, particle_deposition)
```

#### **🌊 Marine Engineering**
```cpp
// Planned marine tools
analyze_ship_resistance(hull_geometry, sea_conditions, propulsion_efficiency)
design_offshore_platform(platform_type, wave_loads, environmental_conditions)
simulate_propeller_cavitation(propeller_geometry, operating_conditions)
optimize_subsea_pipeline(pipeline_route, current_loads, installation_method)
```

### 🧠 **AI-Enhanced Capabilities**

#### **Smart Automation**
- **Auto-Mesh Generation**: AI-driven mesh adaptation based on physics
- **Intelligent Solver Selection**: Automatic solver and model selection
- **Convergence Prediction**: ML-based convergence behavior forecasting
- **Error Detection**: Automated identification of non-physical results

#### **Design Optimization**
- **Multi-Objective Optimization**: Pareto-optimal design exploration
- **Sensitivity Analysis**: Parameter importance ranking and correlation
- **Surrogate Modeling**: Reduced-order models for rapid design iteration
- **Shape Optimization**: Gradient-based and evolutionary algorithms

#### **Predictive Analytics**
- **Performance Prediction**: AI models trained on CFD databases
- **Failure Mode Analysis**: Prediction of critical operating conditions
- **Maintenance Scheduling**: Condition monitoring through CFD analysis
- **Real-Time Control**: CFD-informed process control systems

### 🔧 **Advanced Computational Features**

#### **High-Performance Computing**
- **Cloud Integration**: AWS, Azure, GCP deployment with auto-scaling
- **HPC Cluster Support**: SLURM, PBS job scheduling integration
- **GPU Acceleration**: CUDA/OpenCL solver acceleration
- **Distributed Computing**: Multi-node parallel execution

#### **Workflow Automation**
- **Parametric Studies**: Automated parameter sweeps and DOE
- **Uncertainty Quantification**: Monte Carlo and polynomial chaos methods
- **Data Management**: Version control for CFD cases and results
- **Report Generation**: Automated technical documentation

#### **Integration & Interoperability**
- **CAD Integration**: Direct CATIA, SolidWorks, Fusion 360 coupling
- **FEA Coupling**: Structural analysis with ANSYS, Abaqus, CalculiX
- **Control Systems**: MATLAB/Simulink integration for system analysis
- **Data Analytics**: Python/R integration for advanced post-processing

### 📊 **Comprehensive Solver Matrix** (Planned)

| **Domain** | **Solvers** | **Applications** | **Target Release** |
|------------|-------------|------------------|-------------------|
| **Compressible** | `rhoCentralFoam`, `sonicFoam` | Aerospace, turbomachinery | Q2 2024 |
| **Multiphase** | `interFoam`, `twoPhaseEulerFoam` | Process industry, marine | Q3 2024 |
| **Combustion** | `fireFoam`, `reactingFoam` | Power generation, automotive | Q4 2024 |
| **Electromagnetics** | `mhdFoam`, `electrostaticFoam` | Fusion, materials processing | Q1 2025 |
| **Particle Dynamics** | `DPMFoam`, `coalChemistryFoam` | Environmental, pharmaceutical | Q2 2025 |

### 🤝 **How to Contribute**

We welcome contributions across all domains! Here's how you can help:

#### **🔬 Physics Domain Experts**
- **Validate Physics Models**: Ensure accurate implementation of governing equations
- **Develop Test Cases**: Create validation cases against analytical/experimental data
- **Write Educational Content**: Explain physics concepts for educational tools

#### **💻 Software Developers** 
- **Implement New Tools**: Follow our architecture patterns for new analysis tools
- **Optimize Performance**: Improve computational efficiency and memory usage
- **Enhance UI/UX**: Develop better user interfaces and visualization tools

#### **🏭 Industry Professionals**
- **Define Use Cases**: Specify real-world engineering problems and requirements
- **Provide Test Data**: Share (anonymized) validation cases and benchmarks
- **Review Results**: Validate tool outputs against industrial standards

#### **📚 Educators & Students**
- **Create Tutorials**: Develop educational content and learning materials
- **Test Tools**: Identify usability issues and suggest improvements
- **Documentation**: Improve documentation, examples, and guides

### 🎯 **Contribution Roadmap**

#### **Immediate Opportunities** (Next 3 months)
1. **Multiphase Flow Tool**: Implement `interFoam` wrapper for free surface flows
2. **Validation Framework**: Develop automated validation against analytical solutions
3. **Performance Optimization**: Parallel execution and memory management improvements
4. **Documentation**: Enhanced tutorials and physics explanations

#### **Medium-term Goals** (6-12 months)
1. **Compressible Flow Suite**: Complete supersonic and transonic flow tools
2. **AI Integration**: Machine learning for mesh optimization and solver selection
3. **Cloud Deployment**: AWS/Azure integration with auto-scaling capabilities
4. **Industry Partnerships**: Collaborate with automotive and aerospace companies

#### **Long-term Vision** (1-2 years)
1. **Complete Physics Coverage**: All OpenFOAM solvers integrated with MCP
2. **Real-time Analysis**: Live simulation monitoring and control
3. **Digital Twin Integration**: IoT sensor data fusion with CFD models
4. **Autonomous Design**: AI-driven design optimization without human intervention

### 📬 **Get Involved**

Ready to contribute? Here's how to start:

1. **Join Our Community**: Star the repository and join discussions
2. **Pick an Issue**: Check our [GitHub Issues](https://github.com/webworn/openfoam-mcp-server/issues) for beginner-friendly tasks
3. **Fork & Develop**: Use GitHub Codespaces for instant development environment
4. **Submit PR**: Follow our contribution guidelines and submit your improvements

**Together, we're building the future of AI-powered CFD analysis!** 🌊🤖⚡

---

*Transform your engineering workflow with AI-powered CFD analysis. Get started today!*