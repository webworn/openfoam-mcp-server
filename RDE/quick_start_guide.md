# Cellular Detonation Framework - Quick Start Guide

## 5-Minute Setup

### 1. Build the Framework

```bash
# Build core framework
cd /workspaces/openfoam-mcp-server
mkdir build && cd build
cmake ..
make -j$(nproc)

# Build validation tests
cd ../tests
make validate
```

### 2. Run Basic Example

```bash
# Compile and run usage examples
cd ../examples
g++ -std=c++17 -I../src -o cellular_demo cellular_detonation_usage.cpp
./cellular_demo
```

### 3. Validate Installation

```bash
# Run validation suite
cd ../tests
./test_cellular_model
```

## Core Usage Patterns

### Basic Cellular Analysis

```cpp
#include "cellular_detonation_model.hpp"

CellularDetonationModel cellularModel;
RDEChemistry chemistry;
chemistry.fuelType = "hydrogen";
chemistry.chamberPressure = 101325.0;
chemistry.useCellularModel = true;

double cellSize = cellularModel.calculateCellSize(chemistry);
double meshSize = cellularModel.calculateRequiredMeshSize(cellSize);

std::cout << "Cell Size: " << cellSize * 1000 << " mm" << std::endl;
std::cout << "Required Mesh: " << meshSize * 1000 << " mm" << std::endl;
```

### Expert RDE Analysis

```cpp
#include "rotating_detonation_engine.hpp"

RotatingDetonationEngine rdeEngine;
RDEAnalysisRequest request;

// Configure for cellular physics
request.settings.solverType = "detonationFoam";
request.settings.enableCellularTracking = true;
request.settings.autoMeshSizing = true;

auto result = rdeEngine.analyzeRDE(request);
```

### Validation Testing

```cpp
#include "rde_validation_framework.hpp"

RDEValidationFramework validator;
auto result = validator.runValidation(
    RDEValidationFramework::ValidationMode::FAST
);

std::cout << "Accuracy: " << result.overallAccuracy * 100 << "%" << std::endl;
```

## Common Tasks

### Multi-Fuel Comparison

```cpp
std::vector<std::string> fuels = {"hydrogen", "methane", "propane"};
for (const auto& fuel : fuels) {
    chemistry.fuelType = fuel;
    double cellSize = cellularModel.calculateCellSize(chemistry);
    std::cout << fuel << ": " << cellSize * 1000 << " mm" << std::endl;
}
```

### Mesh Validation

```cpp
double cellSize = 0.001;  // 1 mm
double meshSize = 0.00005;  // 0.05 mm
bool valid = cellularModel.validateMeshResolution(meshSize, cellSize);
std::cout << "Mesh valid: " << (valid ? "Yes" : "No") << std::endl;
```

### Pressure Scaling

```cpp
std::vector<double> pressures = {101325, 200000, 500000};  // 1, 2, 5 atm
for (double pressure : pressures) {
    chemistry.chamberPressure = pressure;
    double cellSize = cellularModel.calculateCellSize(chemistry);
    std::cout << pressure/1000 << " kPa: " << cellSize*1000 << " mm" << std::endl;
}
```

## Expected Results

### Validation Success Rates

- **Fast Mode**: 100% success (3/3 cases)
- **Standard Mode**: 100% success (2/2 cases)
- **Comprehensive Mode**: 100% success (5/5 cases)

### Typical Cell Sizes

| Fuel     | 1 atm   | 2 atm   | 5 atm   |
|----------|---------|---------|---------|
| Hydrogen | 1.0 mm  | 0.7 mm  | 0.4 mm  |
| Methane  | 10.0 mm | 7.1 mm  | 4.5 mm  |
| Propane  | 20.0 mm | 14.1 mm | 8.9 mm  |

## Troubleshooting

### Build Issues

```bash
# Missing dependencies
sudo apt-get install cmake build-essential

# OpenFOAM not found
source $FOAM_ROOT/etc/bashrc
```

### Runtime Errors

```bash
# Permission denied
chmod +x external/detonationFoam_V2.0/Allwmake

# Library not found
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
```

## Next Steps

1. **Explore Examples**: Run all 6 examples in `cellular_detonation_usage.cpp`
2. **Read Documentation**: Review `cellular_detonation_integration.md`
3. **Run Validation**: Execute comprehensive validation suite
4. **Customize Cases**: Modify examples for your specific needs

## Getting Help

- **Documentation**: See `RDE/cellular_detonation_integration.md`
- **Examples**: Study `examples/cellular_detonation_usage.cpp`
- **Validation**: Check `tests/test_cellular_model.cpp`
- **Issues**: Report problems via GitHub issues

---

*This guide gets you started with the cellular detonation framework in under 5 minutes. For detailed information, see the full documentation.*