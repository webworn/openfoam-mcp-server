# OpenFOAM Universe: Complete CFD Scenarios & Capabilities

## 🌊 Physics Categories & User Scenarios

### 1. **Incompressible Flow** (Ma < 0.3)

#### **External Aerodynamics**
- **Cars**: "Analyze drag reduction on electric vehicle", "Optimize spoiler design"
- **Aircraft**: "Study wing performance at takeoff/landing", "Analyze propeller efficiency"
- **Buildings**: "Calculate wind loads on skyscraper", "Study pedestrian wind comfort"
- **Sports**: "Optimize cyclist position", "Design Formula 1 front wing"
- **Ships**: "Hull resistance analysis", "Propeller wake interaction"

#### **Internal Flows**
- **Pipes & Ducts**: "Calculate pressure drop in HVAC system", "Optimize pipe network"
- **Pumps & Turbines**: "Analyze centrifugal pump performance", "Design impeller geometry"
- **Valves**: "Study flow through control valve", "Optimize valve opening profile"
- **Heat Exchangers**: "Shell-and-tube thermal performance", "Plate heat exchanger optimization"

#### **Free Surface Flows**
- **Wave Energy**: "Design wave energy converter", "Study breaking waves"
- **Sloshing**: "Fuel tank dynamics", "LNG carrier sloshing analysis"
- **Dam Break**: "Flood modeling", "Tsunami propagation"
- **Ship Hydrodynamics**: "Hull wave resistance", "Ship seakeeping"

---

### 2. **Compressible Flow** (Ma > 0.3)

#### **Supersonic/Hypersonic**
- **Aerospace**: "Scramjet inlet design", "Hypersonic vehicle heating"
- **Propulsion**: "Rocket nozzle optimization", "Jet engine analysis"
- **Defense**: "Missile aerodynamics", "Shock wave mitigation"

#### **Transonic Flows**
- **Aircraft**: "Transonic wing design", "Shock-boundary layer interaction"
- **Turbomachinery**: "Compressor blade analysis", "Turbine cascade flows"

#### **Gas Dynamics**
- **Industrial**: "Gas pipeline blowdown", "Pressure vessel rupture"
- **Safety**: "Gas explosion modeling", "Shock tube analysis"

---

### 3. **Heat Transfer**

#### **Natural Convection**
- **Buildings**: "Room air circulation", "Natural ventilation design"
- **Electronics**: "CPU cooling without fans", "LED thermal management"
- **Industrial**: "Furnace heat distribution", "Solar collector analysis"

#### **Forced Convection**
- **HVAC**: "Air conditioning system", "Heat pump optimization"
- **Automotive**: "Engine cooling", "Radiator design"
- **Electronics**: "Server room cooling", "Power electronics thermal management"

#### **Conjugate Heat Transfer**
- **Gas Turbines**: "Turbine blade cooling", "Combustor liner analysis"
- **Nuclear**: "Reactor vessel cooling", "Fuel element thermal analysis"
- **Process**: "Chemical reactor heating", "Heat exchanger design"

#### **Radiation**
- **Solar**: "Solar panel thermal analysis", "Concentrated solar power"
- **Furnaces**: "Industrial furnace design", "Glass melting furnaces"
- **Space**: "Spacecraft thermal control", "Satellite thermal management"

---

### 4. **Turbulent Flow**

#### **RANS Applications**
- **Industrial**: "Mixing tank optimization", "Separator design"
- **Environmental**: "Pollution dispersion", "Urban wind patterns"
- **Energy**: "Wind farm optimization", "Turbine wake analysis"

#### **LES Applications**
- **Research**: "Fundamental turbulence studies", "DNS validation"
- **Acoustics**: "Jet noise prediction", "Fan noise analysis"
- **Mixing**: "Combustor mixing", "Chemical reactor optimization"

#### **Transition Modeling**
- **Aerospace**: "Laminar flow wing design", "Natural laminar flow"
- **Turbomachinery**: "Blade boundary layer transition", "Loss prediction"

---

### 5. **Multiphase Flow**

#### **VOF (Volume of Fluid)**
- **Marine**: "Ship wave breaking", "Offshore platform design"
- **Industrial**: "Coating flows", "Injection molding"
- **Energy**: "Hydroelectric turbine", "Pump-turbine operation"

#### **Eulerian-Eulerian**
- **Chemical**: "Fluidized bed reactors", "Bubble column reactors"
- **Oil & Gas**: "Pipeline flow", "Separator design"
- **Mining**: "Hydrocyclone separation", "Flotation processes"

#### **Lagrangian Particle Tracking**
- **Pharmaceutical**: "Inhaler drug delivery", "Spray drying"
- **Environmental**: "Particulate pollution", "Dust dispersion"
- **Combustion**: "Spray combustion", "Coal particle burning"

#### **Cavitation**
- **Marine**: "Propeller cavitation", "Hydrofoil design"
- **Hydraulic**: "Pump cavitation", "Valve cavitation"
- **Medical**: "Ultrasonic cavitation", "Blood pump design"

---

### 6. **Reactive Flows & Combustion**

#### **Premixed Combustion**
- **Automotive**: "Gasoline engine combustion", "Gas turbine combustors"
- **Industrial**: "Burner design", "Furnace optimization"
- **Safety**: "Explosion modeling", "Fire suppression"

#### **Non-premixed Combustion**
- **Power Generation**: "Coal-fired boiler", "Waste incineration"
- **Aerospace**: "Rocket engine combustion", "Scramjet analysis"
- **Industrial**: "Glass melting", "Steel reheating furnaces"

#### **Fire Modeling**
- **Safety**: "Building fire evacuation", "Wildfire spread"
- **Industrial**: "Oil refinery fires", "Warehouse fire suppression"

---

### 7. **Fluid-Structure Interaction**

#### **Aeroelasticity**
- **Aerospace**: "Wing flutter analysis", "Helicopter rotor dynamics"
- **Civil**: "Bridge wind oscillations", "Building vibrations"
- **Energy**: "Wind turbine blade fatigue", "Offshore platform motion"

#### **Vibration-Induced Flows**
- **Nuclear**: "Fuel rod vibration", "Steam generator tube bundles"
- **Heat Exchangers**: "Tube bundle vibration", "Flow-induced vibration"

---

### 8. **Specialized Applications**

#### **Magnetohydrodynamics (MHD)**
- **Energy**: "MHD power generation", "Plasma confinement"
- **Metallurgy**: "Aluminum smelting", "Steel continuous casting"
- **Space**: "Plasma propulsion", "Magnetosphere modeling"

#### **Electrochemistry**
- **Energy**: "Fuel cell modeling", "Battery thermal management"
- **Industrial**: "Electroplating", "Water electrolysis"

#### **Porous Media**
- **Oil & Gas**: "Reservoir simulation", "Enhanced oil recovery"
- **Environmental**: "Groundwater flow", "Soil contamination"
- **Biomedical**: "Drug delivery", "Tissue perfusion"

#### **Atmospheric & Environmental**
- **Weather**: "Microclimate modeling", "Urban heat islands"
- **Pollution**: "Smokestack dispersion", "Vehicle emission dispersion"
- **Natural Disasters**: "Hurricane modeling", "Tornado dynamics"

---

## 🔧 OpenFOAM Solvers by Application

### **Quick Reference: User Need → Solver**

| User Scenario | Physics | Solver | Turbulence |
|---------------|---------|--------|------------|
| "Car drag analysis" | Incompressible, external | `simpleFoam` | kOmegaSST |
| "Pipe pressure drop" | Incompressible, internal | `simpleFoam` | kEpsilon |
| "Supersonic jet" | Compressible, high-speed | `rhoCentralFoam` | kOmegaSST |
| "Heat exchanger" | Heat transfer | `chtMultiRegionFoam` | kOmegaSST |
| "Free surface waves" | Multiphase VOF | `interFoam` | kOmegaSST |
| "Engine combustion" | Reactive flow | `reactingFoam` | kEpsilon |
| "Natural convection" | Buoyancy-driven | `buoyantPimpleFoam` | kEpsilon |
| "Mixing tank" | Unsteady turbulent | `pimpleFoam` | kOmegaSST |
| "Jet noise" | LES acoustics | `pimpleFoam` | LES |
| "Fluidized bed" | Eulerian multiphase | `twoPhaseEulerFoam` | kEpsilon |

### **Detailed Solver Categories**

#### **Incompressible Solvers**
```cpp
// Steady-state
simpleFoam           // General RANS, most common
potentialFoam        // Inviscid, very fast estimates
SRFSimpleFoam        // Rotating machinery
MRFSimpleFoam        // Multiple reference frames

// Transient
pimpleFoam           // General unsteady RANS
pisoFoam             // Unsteady RANS (legacy)
icoFoam              // Laminar flows
nonNewtonianIcoFoam  // Non-Newtonian fluids
```

#### **Compressible Solvers**
```cpp
// Density-based (high-speed)
rhoCentralFoam       // Supersonic/hypersonic
sonicFoam            // Transonic flows

// Pressure-based (subsonic)
rhoPimpleFoam        // General compressible unsteady
rhoSimpleFoam        // General compressible steady
```

#### **Heat Transfer Solvers**
```cpp
buoyantPimpleFoam    // Natural convection, unsteady
buoyantSimpleFoam    // Natural convection, steady
chtMultiRegionFoam   // Conjugate heat transfer
thermoFoam           // Pure heat conduction
laplacianFoam        // Diffusion-only problems
```

#### **Multiphase Solvers**
```cpp
// VOF Method
interFoam                    // Two immiscible fluids
multiphaseInterFoam         // Multiple immiscible fluids
compressibleInterFoam       // Compressible multiphase
cavitatingFoam              // Cavitation modeling

// Eulerian-Eulerian
twoPhaseEulerFoam           // Two-fluid model
reactingTwoPhaseEulerFoam   // Reacting two-phase
multiphaseEulerFoam         // Multiple Eulerian phases

// Specialized
driftFluxFoam               // Drift-flux model
twoLiquidMixingFoam         // Liquid-liquid mixing
```

#### **Combustion Solvers**
```cpp
reactingFoam            // General combustion
fireFoam                // Fire and explosion
XiFoam                  // Premixed combustion
PDRFoam                 // Explosion modeling
coldEngineFoam          // Engine cold flow
engineFoam              // Engine with combustion
```

#### **Specialized Solvers**
```cpp
// Electromagnetics
electrostaticFoam       // Electrostatic fields
magneticFoam            // Magnetic fields
mhdFoam                 // Magnetohydrodynamics

// Transport
scalarTransportFoam     // Passive scalar transport
boundaryFoam            // Boundary layer flows

// Solid mechanics
solidDisplacementFoam   // Structural analysis
solidEquilibriumFoam    // Static structural
```

---

## 🎯 MCP Tools by Industry & Application

### **Automotive Industry**
```cpp
analyze_vehicle_aerodynamics()    // External flow, drag/lift
analyze_engine_combustion()       // ICE performance
optimize_cooling_system()         // Thermal management
study_hvac_performance()          // Climate control
analyze_exhaust_flow()            // Emission systems
```

### **Aerospace Industry**
```cpp
analyze_aircraft_performance()    // Wing/fuselage aerodynamics
design_propulsion_system()        // Engines, nozzles
study_hypersonic_heating()        // Thermal protection
analyze_helicopter_rotor()        // Rotorcraft aerodynamics
optimize_spacecraft_thermal()     // Space vehicle thermal
```

### **Energy Sector**
```cpp
analyze_wind_turbine()            // Wind energy
optimize_gas_turbine()            // Power generation
study_nuclear_cooling()           // Nuclear safety
design_heat_exchanger()           // Thermal equipment
analyze_solar_collector()         // Solar thermal
```

### **Process Industries**
```cpp
optimize_mixing_process()         // Chemical reactors
analyze_separation_unit()         // Distillation, extraction
study_fluidized_bed()            // Catalytic processes
design_heat_recovery()            // Energy efficiency
analyze_pipeline_flow()           // Fluid transport
```

### **Marine & Offshore**
```cpp
analyze_ship_resistance()         // Hull optimization
study_propeller_performance()     // Propulsion
analyze_offshore_platform()       // Wave loading
optimize_underwater_vehicle()     // AUV/ROV design
study_wave_energy_converter()     // Renewable energy
```

### **Civil Engineering**
```cpp
analyze_building_wind_loads()     // Structural safety
study_urban_wind_flow()           // City planning
optimize_ventilation_system()     // Building HVAC
analyze_bridge_aerodynamics()     // Infrastructure safety
study_pedestrian_comfort()        // Urban design
```

### **Biomedical Applications**
```cpp
analyze_blood_flow()              // Cardiovascular
study_respiratory_flow()          // Pulmonary
optimize_drug_delivery()          // Pharmaceutical
analyze_medical_device()          // FDA validation
study_bioreactor_mixing()         // Biotechnology
```

### **Environmental Engineering**
```cpp
study_pollution_dispersion()      // Air quality
analyze_groundwater_flow()        // Contamination
optimize_wastewater_treatment()   // Water treatment
study_atmospheric_flow()          // Weather/climate
analyze_wildfire_spread()         // Emergency response
```

---

## 🔍 Analysis Types & Outputs

### **Force & Moment Analysis**
- Drag coefficient (Cd)
- Lift coefficient (Cl)
- Moment coefficients (Cm, Cn, Cl)
- Pressure distribution
- Wall shear stress
- Force breakdown (pressure vs viscous)

### **Flow Field Analysis**
- Velocity magnitude and components
- Pressure fields
- Streamlines and pathlines
- Vorticity and Q-criterion
- Lambda-2 criterion
- Turbulent kinetic energy
- Reynolds stresses

### **Heat Transfer Analysis**
- Heat transfer coefficients
- Nusselt numbers
- Temperature distributions
- Heat flux (wall and volumetric)
- Thermal efficiency
- Heat exchanger effectiveness

### **Performance Metrics**
- Pressure drop/loss coefficients
- Mass flow rates
- Power requirements
- Efficiency calculations
- Energy dissipation
- Mixing effectiveness

### **Specialized Outputs**
- Species concentrations (combustion)
- Phase distributions (multiphase)
- Particle trajectories (Lagrangian)
- Acoustic fields (LES)
- Electromagnetic fields (MHD)
- Structural deformation (FSI)

---

## 🎯 User Intent Categories

### **By Objective**
1. **Design Optimization**: "Find best design for minimum drag"
2. **Performance Analysis**: "How efficient is this heat exchanger?"
3. **Safety Assessment**: "What are the wind loads on this building?"
4. **Troubleshooting**: "Why is this pump cavitating?"
5. **Research Study**: "Validate new turbulence model"
6. **Quick Estimate**: "Rough pressure drop in this pipe?"

### **By Accuracy Level**
1. **Research Grade**: <1% error, publication quality
2. **Engineering Design**: 2-5% error, design decisions
3. **Conceptual Study**: 5-15% error, feasibility assessment
4. **Quick Estimate**: 10-30% error, order of magnitude

### **By Timeline**
1. **Real-time**: <1 minute (potential flow, correlations)
2. **Quick**: 1-30 minutes (coarse RANS)
3. **Standard**: 1-8 hours (typical engineering analysis)
4. **Detailed**: 1-7 days (LES, optimization studies)
5. **Research**: Weeks to months (DNS, extensive parametrics)

---

This comprehensive mapping enables the MCP server to understand ANY CFD request and translate it to the appropriate OpenFOAM setup! 🚀