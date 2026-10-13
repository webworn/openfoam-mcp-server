cmake_minimum_required(VERSION 3.16)
project(openfoam-mcp-server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# OpenFOAM is invoked as an external application. Build the MCP server without
# binding it to one OpenFOAM release's C++ ABI or installation path.
message(STATUS "Building OpenFOAM MCP server with runtime OpenFOAM discovery")
# Find dependencies
find_package(nlohmann_json QUIET)
find_package(SQLite3 QUIET)
find_package(Boost COMPONENTS system thread QUIET)

# The server only needs the OpenFOAM command line tools at runtime. This keeps
# the MCP executable buildable against both OpenFOAM Foundation and OpenCFD.

# MCP tools and runtime case management
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
    "src/tools/run_case_tool.cpp"
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

# System libraries
set(SYSTEM_LIBS pthread dl m stdc++)

# Link all libraries
target_link_libraries(openfoam-mcp-server
    ${SYSTEM_LIBS}
)

# Link test executable with system libraries only
target_link_libraries(openfoam-mcp-server-test
    ${SYSTEM_LIBS}
)

# Report linking status
message(STATUS "System libraries: ${SYSTEM_LIBS}")

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

message(STATUS "MCP server build configured; source OpenFOAM's etc/bashrc before running simulations")

# Installation
install(TARGETS openfoam-mcp-server 
    RUNTIME DESTINATION bin
)
