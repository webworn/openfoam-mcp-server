#!/usr/bin/env python3
"""
Generate changelog from git history
"""

import subprocess
import sys
from datetime import datetime

def generate_changelog():
    """Generate changelog from git commits"""
    
    print("# OpenFOAM MCP Server - Changelog")
    print()
    print(f"Generated on: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print()
    
    try:
        # Get recent commits
        result = subprocess.run(
            ["git", "log", "--oneline", "-10", "--pretty=format:%h %s"],
            capture_output=True,
            text=True
        )
        
        if result.returncode == 0:
            commits = result.stdout.strip().split('\n')
            
            print("## Recent Changes")
            print()
            for commit in commits[:10]:  # Show last 10 commits
                if commit.strip():
                    print(f"- {commit}")
            print()
            
            print("## Features")
            print("- Complete OpenFOAM MCP Server implementation")
            print("- Pipe flow analysis with laminar/turbulent flow support")
            print("- Heat transfer analysis with forced/natural convection")
            print("- External flow analysis with aerodynamics calculations")
            print("- Comprehensive test suite with physics validation")
            print("- Docker containerization support")
            print("- GitHub Actions CI/CD pipeline")
            print()
            
            print("## Technical Details")
            print("- C++20 implementation with modern CMake")
            print("- JSON-RPC 2.0 protocol compliance")
            print("- OpenFOAM 12 integration")
            print("- Validated against analytical solutions")
            print("- Performance benchmarks and memory profiling")
            print()
            
    except Exception as e:
        print(f"Error generating changelog: {e}")
        print()
        print("## Manual Changelog")
        print("- OpenFOAM MCP Server implementation complete")
        print("- All CFD tools validated and tested")

def main():
    """Main entry point"""
    generate_changelog()
    return 0

if __name__ == "__main__":
    sys.exit(main())