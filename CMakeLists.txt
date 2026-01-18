cmake_minimum_required(VERSION 3.16)
project(openfoam-mcp-server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Full OpenFOAM integration with proper Pstream library
message(STATUS "🔧 Building full OpenFOAM MCP server with dummy Pstream")

# OpenFOAM Configuration
set(OPENFOAM_DIR "/opt/openfoam12" CACHE PATH "OpenFOAM installation directory")
set(OPENFOAM_ARCH "linux64GccDPInt32Opt" CACHE STRING "OpenFOAM architecture")

# OpenFOAM Library Detection
set(OPENFOAM_LIB_DIR "${OPENFOAM_DIR}/platforms/${OPENFOAM_ARCH}/lib")
set(OPENFOAM_SRC_DIR "${OPENFOAM_DIR}/src")

# Core OpenFOAM Libraries
find_library(OPENFOAM_LIB OpenFOAM PATHS ${OPENFOAM_LIB_DIR} NO_DEFAULT_PATH)
find_library(FINITE_VOLUME_LIB finiteVolume PATHS ${OPENFOAM_LIB_DIR} NO_DEFAULT_PATH)
find_library(MESH_TOOLS_LIB meshTools PATHS ${OPENFOAM_LIB_DIR} NO_DEFAULT_PATH)

# Use dummy Pstream for single-processor mode (avoids MPI dependencies)
find_library(PSTREAM_DUMMY_LIB Pstream PATHS ${OPENFOAM_LIB_DIR}/dummy NO_DEFAULT_PATH)

# Find dependencies
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json QUIET)
find_package(SQLite3 QUIET)
find_package(Boost COMPONENTS system thread QUIET)

# Status reporting
if(OPENFOAM_LIB)
    message(STATUS "✅ Found OpenFOAM: ${OPENFOAM_LIB}")
else()
    message(FATAL_ERROR "❌ OpenFOAM library not found at ${OPENFOAM_LIB_DIR}")
endif()

if(PSTREAM_DUMMY_LIB)
    message(STATUS "✅ Found Pstream (dummy): ${PSTREAM_DUMMY_LIB}")
else()
    message(WARNING "⚠️  Dummy Pstream library not found")
endif()

# OpenFOAM Include Directories
if(EXISTS "${OPENFOAM_SRC_DIR}")
    include_directories(${OPENFOAM_SRC_DIR})
    include_directories(${OPENFOAM_SRC_DIR}/OpenFOAM/lnInclude)
    include_directories(${OPENFOAM_SRC_DIR}/OSspecific/POSIX/lnInclude)
    include_directories(${OPENFOAM_SRC_DIR}/finiteVolume/lnInclude)
    include_directories(${OPENFOAM_SRC_DIR}/meshTools/lnInclude)
    message(STATUS "✅ OpenFOAM headers: ${OPENFOAM_SRC_DIR}")
else()
    message(FATAL_ERROR "❌ OpenFOAM source directory not found: ${OPENFOAM_SRC_DIR}")
endif()

# OpenFOAM Compiler Configuration
add_compile_definitions(
    WM_DP                    # Double precision
    WM_LABEL_SIZE=32        # 32-bit labels
    NoRepository            # Disable template repository
    linux64
    WM_ARCH_OPTION=64
)

# Core Source Files (full OpenFOAM integration)
set(CORE_SOURCES 
    "src/mcp/json_rpc.cpp"
    "src/mcp/server.cpp"
    "src/openfoam/case_manager.cpp"
    "src/openfoam/external_flow.cpp"
    "src/openfoam/heat_transfer.cpp"
    "src/openfoam/multiphase_flow.cpp"
    "src/openfoam/pipe_flow.cpp"
    "src/openfoam/mesh_quality.cpp"
    "src/openfoam/stl_analyzer.cpp"
    "src/tools/external_flow_tool.cpp"
    "src/tools/heat_transfer_tool.cpp"
    "src/tools/multiphase_flow_tool.cpp"
    "src/tools/pipe_flow_tool.cpp"
    "src/tools/cfd_assistant_tool.cpp"
    "src/tools/cfd_assistant_registration.cpp"
    "src/tools/context_engine.cpp"
    "src/utils/logging.cpp"
    "src/utils/terminal_manager.cpp"
)

# Main source file
set(MAIN_SOURCE "src/main.cpp")

# Combine all sources
set(ALL_SOURCES ${CORE_SOURCES})

list(LENGTH ALL_SOURCES SOURCE_COUNT)
message(STATUS "📦 Total source files: ${SOURCE_COUNT}")

# Project headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Compiler flags for OpenFOAM compatibility
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -fPIC")

# Main executable
add_executable(openfoam-mcp-server
    ${MAIN_SOURCE}
    ${ALL_SOURCES}
)

# Test executable with minimal dependencies
add_executable(openfoam-mcp-server-test
    "src/main_test.cpp"
    "src/mcp/json_rpc.cpp"
    "src/mcp/server.cpp"
    "src/tools/cfd_assistant_tool.cpp"
    "src/tools/cfd_assistant_registration.cpp"
    "src/tools/context_engine.cpp"
    "src/tools/mesh_quality_tool.cpp"
    "src/tools/stl_analyzer_tool.cpp"
    "src/tools/rde_wave_tool.cpp"
    "src/tools/rde_3d_geometry_tool.cpp"
    "src/tools/rde_3d_wave_tool.cpp"
    "src/tools/rde_3d_performance_tool.cpp"
    "src/tools/turbulent_flow_tool.cpp"
    "src/openfoam/mesh_quality.cpp"
    "src/openfoam/stl_analyzer.cpp"
    "src/openfoam/turbulence_utils.cpp"
    "src/openfoam/pipe_flow.cpp"
    "src/openfoam/case_manager.cpp"
    "src/tools/rde_wave_analyzer.cpp"
    "src/tools/rde_3d_geometry.cpp"
    "src/tools/rde_3d_wave_analyzer.cpp"
    "src/tools/rde_3d_performance.cpp"
    "src/utils/logging.cpp"
    "src/utils/terminal_manager.cpp"
)

# Core libraries (required)
set(REQUIRED_LIBS)
if(OPENFOAM_LIB)
    list(APPEND REQUIRED_LIBS ${OPENFOAM_LIB})
endif()
if(FINITE_VOLUME_LIB)
    list(APPEND REQUIRED_LIBS ${FINITE_VOLUME_LIB})
endif()
if(MESH_TOOLS_LIB)
    list(APPEND REQUIRED_LIBS ${MESH_TOOLS_LIB})
endif()
if(PSTREAM_DUMMY_LIB)
    list(APPEND REQUIRED_LIBS ${PSTREAM_DUMMY_LIB})
endif()

# System libraries
set(SYSTEM_LIBS pthread dl m stdc++)

# Set runtime library paths for OpenFOAM libraries
set_target_properties(openfoam-mcp-server PROPERTIES
    INSTALL_RPATH "${OPENFOAM_LIB_DIR};${OPENFOAM_LIB_DIR}/dummy"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

# Link all libraries
target_link_libraries(openfoam-mcp-server
    ${REQUIRED_LIBS}
    ${SYSTEM_LIBS}
)

# Link test executable with system libraries only
target_link_libraries(openfoam-mcp-server-test
    ${SYSTEM_LIBS}
)

# Report linking status
message(STATUS "🔗 Linking libraries:")
message(STATUS "   Required: ${REQUIRED_LIBS}")
message(STATUS "   System: ${SYSTEM_LIBS}")

# Add nlohmann_json if found
if(nlohmann_json_FOUND)
    target_link_libraries(openfoam-mcp-server nlohmann_json::nlohmann_json)
    message(STATUS "✅ Using system nlohmann_json")
endif()

# Add SQLite3 if found
if(SQLite3_FOUND)
    target_link_libraries(openfoam-mcp-server SQLite::SQLite3)
    message(STATUS "✅ Using SQLite3")
endif()

# Add Boost if found
if(Boost_FOUND)
    target_link_libraries(openfoam-mcp-server Boost::system Boost::thread)
    message(STATUS "✅ Using Boost")
endif()

message(STATUS "🎯 Full OpenFOAM build configured successfully")

# Installation
install(TARGETS openfoam-mcp-server 
    RUNTIME DESTINATION bin
)