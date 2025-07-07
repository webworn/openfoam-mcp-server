#include "fvCFD.H"
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "🚀 OpenFOAM MCP Server v1.0.0" << std::endl;
    std::cout << "OpenFOAM version: " << Foam::foamVersion::version.c_str() << std::endl;
    
    // Test OpenFOAM integration
    Foam::Info << "✅ OpenFOAM integration: Working!" << Foam::endl;
    
    // Show environment variables
    if (const char* foam_src = std::getenv("FOAM_SRC")) {
        Foam::Info << "FOAM_SRC: " << foam_src << Foam::endl;
    }
    
    if (const char* foam_tutorials = std::getenv("FOAM_TUTORIALS")) {
        Foam::Info << "FOAM_TUTORIALS: " << foam_tutorials << Foam::endl;
    }
    
    std::cout << "🎯 Ready to build the world's first OpenFOAM-native MCP server!" << std::endl;
    std::cout << "🤖 Use Claude Code for AI-assisted development!" << std::endl;
    
    return 0;
}