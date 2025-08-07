cmake_minimum_required(VERSION 3.16)
project(openfoam-mcp-server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find OpenFOAM libraries (installed in /opt/openfoam12)
find_library(OPENFOAM_LIB OpenFOAM PATHS /opt/openfoam12/platforms/linux64GccDPInt32Opt/lib)
find_library(FINITE_VOLUME_LIB finiteVolume PATHS /opt/openfoam12/platforms/linux64GccDPInt32Opt/lib)
find_library(MESH_TOOLS_LIB meshTools PATHS /opt/openfoam12/platforms/linux64GccDPInt32Opt/lib)

if(NOT OPENFOAM_LIB)
    message(FATAL_ERROR "OpenFOAM library not found. Please install openfoam12 package.")
endif()

# Include OpenFOAM headers
include_directories(/opt/openfoam12/src)
include_directories(/opt/openfoam12/src/OpenFOAM/lnInclude)
include_directories(/opt/openfoam12/src/OSspecific/POSIX/lnInclude)
include_directories(/opt/openfoam12/src/finiteVolume/lnInclude)
include_directories(/opt/openfoam12/src/meshTools/lnInclude)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Find dependencies
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json QUIET)
find_package(SQLite3 QUIET)
find_package(Boost COMPONENTS system thread QUIET)

# Compiler flags for OpenFOAM compatibility
add_compile_definitions(WM_DP WM_LABEL_SIZE=32 NoRepository)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Source files
file(GLOB_RECURSE SOURCES 
    "src/mcp/*.cpp"
    "src/openfoam/*.cpp" 
    "src/tools/*.cpp"
    "src/utils/*.cpp"
    "src/physics/cantera_wrapper.cpp"
)

# Exclude test files from main sources
list(FILTER SOURCES EXCLUDE REGEX ".*test_.*")

# Main executable
add_executable(openfoam-mcp-server
    src/main.cpp
    ${SOURCES}
)

target_link_libraries(openfoam-mcp-server
    ${OPENFOAM_LIB}
    ${FINITE_VOLUME_LIB}
    ${MESH_TOOLS_LIB}
    pthread
    dl
)

# Add nlohmann_json if found
if(nlohmann_json_FOUND)
    target_link_libraries(openfoam-mcp-server nlohmann_json::nlohmann_json)
endif()

# Add SQLite3 if found
if(SQLite3_FOUND)
    target_link_libraries(openfoam-mcp-server SQLite::SQLite3)
endif()

# Add Boost if found
if(Boost_FOUND)
    target_link_libraries(openfoam-mcp-server Boost::system Boost::thread)
endif()

# Physics test executable
add_executable(test_cantera_wrapper
    src/physics/test_cantera_wrapper.cpp
    src/physics/cantera_wrapper.cpp
)

target_link_libraries(test_cantera_wrapper
    pthread
)

# Add nlohmann_json to physics test if found
if(nlohmann_json_FOUND)
    target_link_libraries(test_cantera_wrapper nlohmann_json::nlohmann_json)
else()
    # Try to use system nlohmann/json header-only library
    find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp 
        PATHS /usr/include /usr/local/include /opt/homebrew/include
    )
    if(NLOHMANN_JSON_INCLUDE_DIR)
        target_include_directories(test_cantera_wrapper PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR})
        message(STATUS "Using system nlohmann/json from ${NLOHMANN_JSON_INCLUDE_DIR}")
    else()
        message(WARNING "nlohmann/json not found. Consider installing with: sudo apt-get install nlohmann-json3-dev")
    endif()
endif()

# Tests (optional)
option(BUILD_TESTS "Build test suite" OFF)
if(BUILD_TESTS)
    enable_testing()
    find_package(Catch2 3 QUIET)
    if(Catch2_FOUND)
        file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
        add_executable(openfoam-mcp-tests ${TEST_SOURCES} ${SOURCES})
        target_link_libraries(openfoam-mcp-tests 
            ${OPENFOAM_LIB}
            ${FINITE_VOLUME_LIB}
            ${MESH_TOOLS_LIB}
            Catch2::Catch2WithMain
        )
        
        include(CTest)
        include(Catch)
        catch_discover_tests(openfoam-mcp-tests)
    endif()
endif()

# Installation
install(TARGETS openfoam-mcp-server 
    RUNTIME DESTINATION bin
)