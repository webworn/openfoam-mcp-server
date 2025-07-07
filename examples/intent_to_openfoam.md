# Intent-to-OpenFOAM Translation Examples

## 🎯 Core Concept: User Intent → OpenFOAM Structure

The MCP server understands what users want to achieve and translates this into proper OpenFOAM case setup.

---

## Example 1: "I want to analyze airflow around a car"

### **User Input (Natural Language):**
```json
{
  "description": "I want to analyze airflow around a car at highway speeds",
  "objectives": ["drag coefficient", "pressure distribution", "wake visualization"],
  "constraints": ["results within 2 hours", "engineering accuracy"]
}
```

### **MCP Understanding & Questions:**
```
🤖 MCP: "I understand you want external aerodynamics analysis. Let me confirm:
- Vehicle speed: ~100 km/h (highway speed detected)
- Primary goal: Drag analysis with flow visualization
- Time constraint: 2 hours
- Accuracy: Engineering level (5-10% acceptable)

Questions:
1. Do you have a CAD file of the car geometry?
2. Should I include ground effect and wheel rotation?
3. Any specific regions of interest (mirrors, spoilers)?
```

### **OpenFOAM Structure Generated:**
```cpp
// MCP automatically determines:
Solver: simpleFoam              // Steady, incompressible at Ma ~0.08
Physics: {
  Reynolds: 2.8e7                // High Re → turbulent flow
  Mach: 0.08                     // Incompressible assumption valid
  Turbulence: kOmegaSST          // Good for external aero
}

Mesh: {
  Target_y_plus: 30-300          // Wall function range
  Wake_refinement: 5 levels      // Capture wake accurately
  Total_cells: ~2M               // Balance accuracy/speed for 2hr target
}

Boundary_Conditions: {
  inlet: "fixedValue U=(27.8 0 0)"     // 100 km/h
  outlet: "zeroGradient"
  ground: "noSlip"                      // Include ground effect
  vehicle: "noSlip"
  sides: "slip"                         // Reduce boundary effects
}

Schemes: {
  convection: "bounded Gauss upwind"    // Stable for complex geometry
  gradient: "Gauss linear"
  laplacian: "Gauss linear corrected"
}

Forces: {
  patches: ["vehicle"]
  CofR: "(2 0 0.5)"                   // Car center of rotation
  reference: {
    velocity: 27.8,
    area: 2.5,                         // Typical car frontal area
    density: 1.225
  }
}
```

---

## Example 2: "Quick pipe flow pressure drop estimate"

### **User Input:**
```json
{
  "description": "What's the pressure drop in a 10cm diameter, 5m long pipe with water at 2 m/s?",
  "objective": "quick estimate",
  "accuracy": "within 20% is fine"
}
```

### **MCP Understanding:**
```
🤖 MCP: "I detect internal pipe flow analysis:
- Geometry: Circular pipe, D=0.1m, L=5m
- Fluid: Water at 2 m/s
- Reynolds number: ~200,000 (turbulent)
- Goal: Pressure drop estimation
- Speed priority: Quick result over accuracy

Setting up simplified turbulent pipe flow case..."
```

### **OpenFOAM Structure Generated:**
```cpp
Solver: simpleFoam              // Steady flow sufficient

Geometry: {
  pipe_diameter: 0.1
  pipe_length: 5.0
  inlet_profile: "uniform"      // Simplified for speed
}

Mesh: {
  cells_radial: 20              // Coarse for speed
  cells_axial: 100
  total_cells: ~50k             // Very fast simulation
  boundary_layers: 3            // Minimal near-wall resolution
}

Physics: {
  Reynolds: 200000              // Turbulent
  turbulence: "kEpsilon"        // Faster than kOmegaSST
  wall_functions: true          // Avoid fine mesh
}

Expected_result: {
  pressure_drop: "~8000 Pa"     // Analytical estimate
  simulation_time: "2 minutes"
  accuracy: "±20%"
}
```

---

## Example 3: "Heat exchanger design optimization"

### **User Input:**
```json
{
  "description": "Optimize tube spacing in shell-and-tube heat exchanger",
  "objectives": ["maximize heat transfer", "minimize pressure drop"],
  "constraints": ["manufacturing feasible", "research-grade accuracy"],
  "parameters": ["tube_spacing: 15-25mm", "shell_diameter: 300mm"]
}
```

### **MCP Understanding:**
```
🤖 MCP: "I detect multi-objective heat exchanger optimization:
- Design variables: Tube spacing (15-25mm range)
- Objectives: Heat transfer vs pressure drop trade-off
- Accuracy: Research grade (1-2% target)
- Method: Parametric study recommended

This requires conjugate heat transfer simulation with parameter sweep.
Estimated study: 10-20 cases, 4-8 hours total.
```

### **OpenFOAM Structure Generated:**
```cpp
Solver: chtMultiRegionFoam      // Conjugate heat transfer

Regions: {
  shell_side: "fluid"           // Hot fluid
  tube_side: "fluid"            // Cold fluid  
  tube_walls: "solid"           // Heat conduction
}

Parameter_sweep: {
  tube_spacing: [15, 17.5, 20, 22.5, 25] // mm
  total_cases: 5
}

Physics: {
  shell_Reynolds: ~10000        // Typical shell-side flow
  tube_Reynolds: ~5000          // Typical tube-side flow
  Prandtl: 0.7                  // Air properties
  heat_transfer: true
}

Mesh_per_case: {
  cells: ~500k                  // High resolution for heat transfer
  y_plus: 1                     // Resolve thermal boundary layer
  interface_matching: true      // Accurate heat transfer
}

Outputs: {
  heat_transfer_rate: "W"
  pressure_drop_shell: "Pa"
  pressure_drop_tube: "Pa" 
  effectiveness: "dimensionless"
  performance_index: "heat_transfer/pressure_drop"
}
```

---

## 🧠 Key Intelligence Features

### **1. Physics Validation**
```cpp
if (mach_number > 0.3) {
    warning("Incompressible assumption questionable at Ma > 0.3");
    suggest("Consider rhoPimpleFoam for compressible effects");
}

if (reynolds < 2300 && turbulence_model != "laminar") {
    warning("Turbulence model unnecessary at Re < 2300");
    suggest("Use laminar solver for faster solution");
}
```

### **2. Automatic Recommendations**
```cpp
// Based on physics and objectives
if (objective == "quick_estimate") {
    mesh_density = "coarse";
    schemes = "first_order_stable";
    convergence = "relaxed_criteria";
}

if (objective == "design_optimization") {
    mesh_density = "fine";
    schemes = "second_order_accurate";
    convergence = "tight_criteria";
}
```

### **3. User-Friendly Explanations**
```
🤖 MCP: "I selected simpleFoam because:
- Your Mach number (0.08) allows incompressible assumption
- Steady flow is sufficient for drag analysis
- This solver is robust and well-validated for external aero

Mesh has 2M cells because:
- Need to resolve boundary layer (y+ = 50)
- Wake region requires refinement for accurate drag
- Compromise between accuracy and your 2-hour time constraint"
```

---

## 🎯 Implementation Priority

This approach means our MCP tools should be organized around **user intent**, not OpenFOAM features:

1. **`analyze_external_flow`** - Cars, aircraft, buildings
2. **`analyze_internal_flow`** - Pipes, ducts, pumps  
3. **`optimize_heat_transfer`** - Heat exchangers, cooling
4. **`study_mixing`** - Reactors, combustors
5. **`estimate_forces`** - Quick drag/lift calculations

Each tool understands physics, asks the right questions, and sets up OpenFOAM properly!