# RDE Expert Gap Analysis & Implementation Resources

## Executive Summary

Critical analysis revealed significant gaps between educational framework and expert-level RDE simulation capabilities. This document provides actionable resources to bridge these gaps based on latest research (2023-2024).

## Critical Gaps Identified

### 1. Missing Cellular Detonation Physics
- **Current**: No cellular structure implementation
- **Risk**: Cannot predict mesh requirements or wave stability
- **Impact**: Unreliable simulation results

### 2. Oversimplified Chemistry
- **Current**: Basic polynomial fits
- **Need**: Stiff chemistry solvers with detailed mechanisms
- **Impact**: Inaccurate detonation predictions

### 3. Inadequate Numerics
- **Current**: Standard CFD solvers
- **Need**: Shock-capturing schemes for discontinuities
- **Impact**: Failed wave propagation capture

### 4. No Experimental Validation
- **Current**: Zero validation data
- **Need**: NASA/Purdue experimental comparisons
- **Impact**: Unverified accuracy

### 5. Incomplete Safety Analysis
- **Current**: Basic TNT equivalency only
- **Need**: Complete explosion safety framework
- **Impact**: Potential safety hazards

## Implementation Resources

### Cellular Detonation Physics

#### detonationFoam Solver (2024)
- **Repository**: https://github.com/JieSun-pku/detonationFoam
- **Publication**: Computer Physics Communications 292 (2023) 108859
- **Contact**: Jie Sun, Peking University
- **Features**:
  - Cellular structure capture
  - HLLC-P Riemann solver
  - Mixture-averaged transport
  - Validated implementation

#### Cellular Structure Research
- "Experimental characterization of the cell cycle for multicellular detonations" - Combustion and Flame (2024)
- "Dynamics of detonation cellular structure in linear and nonlinear instability regimes" - Combustion and Flame (2024)
- Machine learning approaches for automatic cell size extraction

### Advanced Chemistry Integration

#### BYCFoam (2024)
- **Publication**: Energies 17(4), 769 (2024)
- **Features**:
  - CVODE integration with Cantera
  - Detailed chemistry for RDE simulations
  - Excellent agreement with Chemkin
  - Real gas effects capability

#### RSDFoam (2023)
- Real gas shock and detonation simulation
- OpenFOAM-Cantera coupling with CVODE
- High-pressure detonation capabilities

#### Validated Reaction Mechanisms

**Hydrogen (H2-air)**:
- 9 species, 21 reactions
- Temperature: 298-2700 K
- Pressure: 0.05-87 atm

**Methane (CH4-air)**:
- GRI-Mech 3.0 (325 reactions, 53 species)
- Source: http://combustion.berkeley.edu/gri-mech/version30/text30.html
- Alternatives: Lu and Law 19, BFER2, MP1, DRM19

**Propane (C3H8-air)**:
- Validated for 1250-1700 K, 0.6-10 atm
- Available through Cantera library

### Shock-Capturing Numerics

#### WENO Implementations
- **BYCFoam WENO**: AMR with dynamic load balancing
- **OpenFOAM WENO Library**: SoftwareX 12 (2020) 100611
- **Memory Optimization**: 90% reduction through stencil reuse

#### Advanced Solvers
- **detonationFoam AMR**: Temperature-based adaptation
- **Base grid**: 200 μm, finest: 25 μm (3-level refinement)
- **Adaptation criteria**: Temperature threshold 0.04

### Experimental Validation Resources

#### NASA Glenn Research Center
- **Lead**: Daniel Paxson
- **Achievement**: 4,000+ pounds thrust for nearly one minute (2022)
- **Collaboration**: Venus Aerospace, Marshall Space Flight Center
- **Focus**: CFD assessment of RDE performance

#### Purdue University
- **Facility**: "Detonation Rig for Optical, Non-intrusive Experimental measurements (DRONE)"
- **Lead**: Distinguished Professor Steve Heister
- **System**: Liquid-cooled RDE design (Panther system)
- **Partnership**: Direct collaboration with NASA Marshall

#### Naval Research Laboratory (NRL)
- **Focus**: Fuel consumption reduction for ships
- **Research**: RDE operational mechanisms understanding

#### Validation Techniques (2024)
- Multi-diagnostic: Schlieren, NO-PLIF, OH-PLIF, Rayleigh scattering
- Machine learning: Automated cell structure extraction
- Statistical methods: 5 MHz simultaneous imaging analysis

### Safety Analysis Standards

#### TNT Equivalency
- **Standard**: 1 gram TNT = 4,184 J
- **Testing Methods**:
  1. Plate dent tests
  2. Reaction velocity measurements
  3. Air blast analysis

#### Safety Protocols
- **Containment Design**: TNT equivalent determines polycarbonate thickness
- **Fragment Analysis**: Critical for full risk assessment
- **R.E. Factor**: Relative effectiveness factor (TNTe/kg)

#### Recent Standards
- "TNT equivalency testing for energetic materials" - AIP Conference Proceedings (2024)
- "Experimental determination of TNT equivalence values for binary energetic materials" - Scientific Reports (2025)

### Leading Research Institutions

#### Argonne National Laboratory
- **Tool**: Nek5000 flow solver
- **Partners**: NETL, AFRL, Convergent Science Inc.
- **Capabilities**: Massive supercomputer simulations

#### University of Michigan
- **Lead**: Professor Venkat Raman
- **Platform**: Summit supercomputer
- **Application**: GE Research collaboration

#### Commercial Tools
- **CONVERGE CFD**: https://convergecfd.com/
- **Capabilities**: 10-15 million cells with AMR
- **Features**: Detailed chemistry, LES

#### Open Source Solutions
- **RYrhoCentralFoam**: National University of Singapore
- **Validation**: Hydrogen-air small annular RDEs
- **Limitation**: Limited documentation

## Immediate Implementation Plan

### Phase 1: Cellular Physics (Week 1-2)
1. **Download detonationFoam**
   - Repository: https://github.com/JieSun-pku/detonationFoam
   - Study validation paper: Computer Physics Communications 292 (2023) 108859
   - Contact Jie Sun for implementation guidance

2. **Implement Cellular Structure Model**
   - Use λ = f(ΔI, MCJ, σ̇max) approach
   - Add mesh constraint enforcement (Δx < λ/10)
   - Validate against experimental cell sizes

### Phase 2: Chemistry Integration (Week 3-4)
1. **Implement BYCFoam**
   - Access through MDPI Energies 17(4), 769 (2024)
   - Setup CVODE-OpenFOAM coupling
   - Integrate Cantera library

2. **Download Reaction Mechanisms**
   - GRI-Mech 3.0 from Berkeley database
   - H2-air 9-species mechanism
   - C3H8-air mechanisms from Cantera

### Phase 3: Validation Setup (Week 5-6)
1. **Establish Collaborations**
   - Contact NASA Glenn (Daniel Paxson)
   - Reach out to Purdue (Steve Heister)
   - Access Caltech database

2. **Implement Validation Framework**
   - 1D detonation tube validation
   - 2D wedge geometry tests
   - Statistical uncertainty quantification

### Phase 4: Safety Implementation (Week 7-8)
1. **TNT Equivalency Calculator**
   - Follow AIP proceedings standards
   - Implement containment design calculations
   - Add fragment analysis capabilities

2. **Safety Protocol Development**
   - Explosion hazard assessment
   - Emergency procedures
   - Personnel protection guidelines

## Success Metrics

### Technical Accuracy
- ±5% C-J velocity prediction
- ±10% pressure accuracy
- ±20% cell size prediction
- >90% experimental agreement

### Safety Compliance
- Complete HAZOP analysis
- Containment design validation
- Emergency protocol implementation
- Fragment trajectory calculation

### Computational Performance
- <24 hours for production RDE simulations
- Automated mesh constraint satisfaction
- Real-time wave tracking capabilities
- Parallel computing efficiency

## Contact Information

### Key Researchers
- **Jie Sun**: Peking University (detonationFoam developer)
- **Daniel Paxson**: NASA Glenn Research Center
- **Steve Heister**: Purdue University (Distinguished Professor)
- **Venkat Raman**: University of Michigan

### Database Access
- **Caltech EDL**: https://shepherd.caltech.edu/EDL/PublicResources/
- **Berkeley Combustion**: http://combustion.berkeley.edu/gri-mech/
- **Cantera Library**: https://cantera.org/

### Commercial Support
- **Convergent Science**: https://convergecfd.com/
- **CONVERGE CFD**: Advanced RDE simulation capabilities

## Risk Assessment Matrix

| Gap Category | Current Risk | Impact | Priority | Timeline |
|--------------|-------------|--------|----------|----------|
| Cellular Physics | Critical | High | URGENT | Week 1-2 |
| Chemistry Models | High | High | URGENT | Week 3-4 |
| Experimental Validation | High | Critical | HIGH | Week 5-6 |
| Safety Analysis | Medium | Fatal | CRITICAL | Week 7-8 |
| Shock Numerics | Medium | High | HIGH | Week 3-4 |
| Multi-Physics | Low | Medium | MEDIUM | Week 9+ |

## Technology Readiness Level

- **Current TRL**: 2-3 (Educational framework)
- **Target TRL**: 5-6 (Component validation)
- **Path to TRL 7-8**: Complete physics + experimental validation

## Conclusion

The research compilation provides comprehensive resources to transform the educational RDE framework into an expert-level engineering tool. Implementation should prioritize cellular detonation physics, detailed chemistry integration, and experimental validation to meet industry standards.

Key success factors:
1. Immediate implementation of detonationFoam
2. CVODE-Cantera chemistry integration
3. NASA/Purdue validation partnerships
4. Complete safety analysis framework
5. Systematic validation against experimental data

This roadmap provides the necessary resources and contacts to bridge all identified gaps and achieve expert-level RDE simulation capabilities.