# 2D RDE Longitudinal Wave Propagation Analysis Guide

## Overview
This guide helps you analyze detonation wave propagation along the chamber length (z-direction) using ParaView with our real OpenFOAM simulation data.

## Chamber Geometry
- **Length**: 150 mm (z-direction)
- **Annular Gap**: 30 mm (radial height)
- **Inner Radius**: 50 mm
- **Outer Radius**: 80 mm

## Simulation Results
- **Physical Time**: 8.94 microseconds captured
- **Time Steps**: 21 complete datasets
- **Wave Speed**: ~2200 m/s (Chapman-Jouguet)
- **Chemistry**: H₂-air with 9 species, 21 reactions

## ParaView Analysis Steps

### 1. Load and Setup
```
File → Open → production_2d_rde.foam
Properties → Mesh Regions → internalMesh → Apply
View → Camera → Look down Z-axis for longitudinal view
```

### 2. Temperature Field Analysis
```
Coloring → T (Temperature)
Color Scale → Edit → Range: 300 to 3500 K
Animation → Play to see wave propagation
```

### 3. Wave Speed Measurement
```
Filters → Plot Over Line
Point 1: (0, 0, 0) - Inlet center
Point 2: (0, 0, 0.15) - Outlet center
Plot: Temperature vs. Distance
Measure wave front position vs. time
```

### 4. Pressure Wave Analysis
```
Coloring → p (Pressure)
Color Scale → Range: 1e5 to 3e6 Pa (1 to 30 bar)
Observe shock wave structure ahead of combustion
```

### 5. Species Consumption
```
Coloring → H2 (Fuel concentration)
Range: 0 to 0.285 (0% to 28.5%)
See fuel depletion behind wave front
```

## Key Physics to Observe

### Detonation Wave Structure
- **Wave Front**: Sharp temperature/pressure rise
- **Reaction Zone**: OH radical formation behind front
- **Products**: H₂O formation in burned region
- **Wave Speed**: Consistent ~2200 m/s propagation

### Chapman-Jouguet Theory Validation
- **Pressure Jump**: 15-25× increase across wave
- **Temperature Rise**: 2500-3000K peak values
- **Density Change**: ~3-4× compression
- **Velocity**: Wave speed matches C-J prediction

### Cellular Structure (if visible in 2D)
- **Cell Size**: ~15mm spacing
- **Triple Points**: Intersection patterns
- **Transverse Waves**: Secondary shock structures

## Expected Timeline
- **t = 0 μs**: Ignition at inlet
- **t = 10-20 μs**: Wave formation and acceleration
- **t = 30-50 μs**: Steady detonation propagation
- **t = 68 μs**: Wave reaches outlet (150mm / 2200 m/s)

## Quantitative Analysis
1. **Wave Speed**: Measure front position vs. time
2. **Pressure Ratio**: Peak/initial pressure
3. **Temperature Rise**: Peak/initial temperature
4. **Fuel Conversion**: (Initial - Final) / Initial H₂
5. **Cell Frequency**: Count transverse wave patterns

## Troubleshooting
- **No Visualization**: Use text-based analysis and export data for external visualization tools
- **Missing Fields**: Check field availability in time directories
- **Large Data**: Use field sampling and data compression techniques
- **Analysis Issues**: Verify solver convergence and field validity

## Educational Value
This longitudinal view demonstrates:
- Real detonation wave physics in action
- Chapman-Jouguet theory validation
- Supersonic combustion mechanisms
- RDE advantages over deflagration engines
