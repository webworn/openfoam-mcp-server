# Makefile for RDE Validation Testing
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
INCLUDES = -I../src -I../external

# Source files
SOURCES = test_rde_validation.cpp \
          ../src/validation/rde_validation_framework.cpp \
          ../src/openfoam/rotating_detonation_engine.cpp \
          ../src/openfoam/cellular_detonation_model.cpp \
          ../src/openfoam/case_manager.cpp \
          ../src/tools/rde_analysis_tool.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Target executable
TARGET = test_rde_validation

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Run validation tests
test: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf validation_results

# Create test directory structure
setup:
	mkdir -p validation_results

# Run comprehensive validation
validate: setup $(TARGET)
	./$(TARGET)

# Install dependencies (placeholder)
install-deps:
	@echo "Installing dependencies..."
	@echo "Note: This requires OpenFOAM and nlohmann/json libraries"

.PHONY: all test clean setup validate install-deps

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build the validation test program"
	@echo "  test         - Run the validation tests"
	@echo "  validate     - Run comprehensive validation suite"
	@echo "  clean        - Clean build artifacts"
	@echo "  setup        - Create test directory structure"
	@echo "  install-deps - Install required dependencies"
	@echo "  help         - Show this help message"