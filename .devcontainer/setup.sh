#!/bin/bash
set -e

echo "🚀 Setting up OpenFOAM MCP development environment..."

# Update system and install basic dependencies
sudo apt-get update && sudo apt-get install -y \
    software-properties-common \
    apt-transport-https \
    ca-certificates \
    gnupg \
    curl \
    wget

# Add OpenFOAM Foundation repository
curl -s https://dl.openfoam.org/gpg.key | sudo apt-key add -
sudo add-apt-repository http://dl.openfoam.org/ubuntu

# Update package list and install OpenFOAM 12
sudo apt-get update && sudo apt-get install -y \
    openfoam12 \
    openfoam12-doc \
    openfoam12-dev

# Install build tools and dependencies
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    libssl-dev \
    nlohmann-json3-dev \
    libsqlite3-dev \
    libcurl4-openssl-dev \
    git \
    curl \
    python3 \
    python3-pip

# Install Claude Code (Anthropic's AI coding assistant)
echo "🤖 Installing Claude Code..."

# Check if already installed to avoid reinstallation
if [ ! -f "$HOME/.claude-code/bin/claude-code" ]; then
    echo "Installing Claude Code for the first time..."
    curl -fsSL https://storage.googleapis.com/anthropic-claude-code/install.sh | bash
    
    # Verify installation
    if [ -f "$HOME/.claude-code/bin/claude-code" ]; then
        echo "✅ Claude Code installed successfully"
    else
        echo "⚠️  Claude Code installation failed - check network connectivity"
    fi
else
    echo "✅ Claude Code already installed, skipping..."
fi

# Ensure PATH is set (idempotent)
if ! grep -q "claude-code/bin" ~/.bashrc; then
    echo 'export PATH="$HOME/.claude-code/bin:$PATH"' >> ~/.bashrc
fi

# Create Claude config directory if it doesn't exist
mkdir -p "$HOME/.claude"

# Preserve existing configuration if present
if [ -f "$HOME/.claude.json" ]; then
    echo "📝 Preserving existing Claude configuration..."
    cp "$HOME/.claude.json" "$HOME/.claude.json.backup"
fi

# Set up OpenFOAM environment
echo 'source /usr/lib/openfoam/openfoam12/etc/bashrc' >> ~/.bashrc
echo 'export OPENFOAM_MCP_ROOT=/workspaces/openfoam-mcp-server' >> ~/.bashrc
echo 'export PATH="$HOME/.claude-code/bin:$PATH"' >> ~/.bashrc

# Claude Code shortcuts for OpenFOAM development
echo '# Claude Code + OpenFOAM shortcuts' >> ~/.bashrc
echo 'alias claude-openfoam="claude-code --context openfoam"' >> ~/.bashrc
echo 'alias claude-mcp="claude-code --context mcp-development"' >> ~/.bashrc

# Create project structure
mkdir -p /workspaces/openfoam-mcp-server/{src,tests,examples}
mkdir -p /workspaces/openfoam-mcp-server/src/{mcp,openfoam,tools}

# Source OpenFOAM environment for this session
source /usr/lib/openfoam/openfoam12/etc/bashrc

# Create CMakeLists.txt
cat > /workspaces/openfoam-mcp-server/CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.16)
project(openfoam-mcp-server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenFOAM libraries (Ubuntu package installation)
find_library(OPENFOAM_LIB OpenFOAM PATHS /usr/lib/openfoam/openfoam12/lib)
find_library(FINITE_VOLUME_LIB finiteVolume PATHS /usr/lib/openfoam/openfoam12/lib)

# Include OpenFOAM headers
include_directories(/usr/lib/openfoam/openfoam12/src)
include_directories(/usr/lib/openfoam/openfoam12/src/finiteVolume/lnInclude)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSON nlohmann_json)

# Compiler flags for OpenFOAM compatibility
add_compile_definitions(WM_DP WM_LABEL_SIZE=32 NoRepository)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")

# Main executable
add_executable(openfoam-mcp-server
    src/main.cpp
)

target_link_libraries(openfoam-mcp-server
    ${OPENFOAM_LIB}
    ${FINITE_VOLUME_LIB}
    pthread
    dl
)
EOF

# Create starter main.cpp
cat > /workspaces/openfoam-mcp-server/src/main.cpp << 'EOF'
#include "fvCFD.H"
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "🚀 OpenFOAM MCP Server v1.0.0" << std::endl;
    std::cout << "OpenFOAM version: " << Foam::foamVersion::version.c_str() << std::endl;
    
    // Test OpenFOAM integration
    Foam::Info << "✅ OpenFOAM integration: Working!" << Foam::endl;
    
    // Show we can access OpenFOAM environment
    Foam::Info << "FOAM_TUTORIALS: " << getenv("FOAM_TUTORIALS") << Foam::endl;
    
    std::cout << "🎯 Ready to build the world's first OpenFOAM-native MCP server!" << std::endl;
    
    return 0;
}
EOF

# Create VS Code tasks
mkdir -p /workspaces/openfoam-mcp-server/.vscode
cat > /workspaces/openfoam-mcp-server/.vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build MCP Server",
            "type": "shell",
            "command": "mkdir -p build && cd build && cmake .. && make -j$(nproc)",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always"
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Run MCP Server",
            "type": "shell",
            "command": "cd build && ./openfoam-mcp-server",
            "group": "test",
            "dependsOn": "Build MCP Server"
        },
        {
            "label": "Test OpenFOAM - Cavity Tutorial",
            "type": "shell",
            "command": "cp -r /usr/share/openfoam/tutorials/incompressible/icoFoam/cavity /tmp/ && echo '✅ Cavity tutorial ready at /tmp/cavity'",
            "group": "test"
        }
    ]
}
EOF

# Initial build to test everything
cd /workspaces/openfoam-mcp-server
mkdir -p build
cd build
cmake .. && make -j$(nproc)

# Test Claude Code installation
echo "🤖 Testing Claude Code installation..."
export PATH="$HOME/.claude-code/bin:$PATH"
if command -v claude-code &> /dev/null; then
    echo "✅ Claude Code installed successfully!"
    claude-code --version
    
    # Initialize Claude Code configuration if needed
    if [ ! -f "$HOME/.claude.json" ]; then
        echo "🔧 Initializing Claude Code configuration..."
        # Let Claude Code create its default config
        timeout 5s claude-code --help &>/dev/null || echo "Config initialization timeout (expected)"
    fi
else
    echo "⚠️  Claude Code installation pending - will be available after restart"
    echo "💡 Run 'source ~/.bashrc && claude-code --version' to test manually"
fi

# Create a persistence check script
cat > "$HOME/check-claude-persistence.sh" << 'EOF'
#!/bin/bash
echo "🔍 Checking Claude Code persistence..."
echo "Claude Code binary: $(which claude-code 2>/dev/null || echo 'NOT FOUND')"
echo "Claude config dir: $(ls -la ~/.claude 2>/dev/null || echo 'NOT FOUND')"
echo "Claude config file: $(ls -la ~/.claude.json 2>/dev/null || echo 'NOT FOUND')"
echo "Volume mounts:"
mount | grep "claude\|vscode" || echo "No Claude-related mounts found"
EOF
chmod +x "$HOME/check-claude-persistence.sh"

echo ""
echo "✅ Setup complete! Your OpenFOAM MCP development environment is ready!"
echo ""
echo "🤖 Claude Code Integration:"
echo "  claude-code 'Help me implement OpenFOAM case creation'"
echo "  claude-openfoam 'Generate MCP tool for turbulent flow'"
echo "  claude-mcp 'Review my JSON-RPC protocol implementation'"
echo ""
echo "📚 OpenFOAM 12 Environment:"
echo "  Source: /usr/lib/openfoam/openfoam12/src"
echo "  Libraries: /usr/lib/openfoam/openfoam12/lib"  
echo "  Tutorials: /usr/share/openfoam/tutorials"
echo ""
echo "Quick commands:"
echo "  Ctrl+Shift+P → 'Tasks: Run Task' → 'Build MCP Server'"
echo "  Ctrl+Shift+P → 'Tasks: Run Task' → 'Run MCP Server'"
echo ""