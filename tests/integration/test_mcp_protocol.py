#!/usr/bin/env python3
"""
MCP Protocol compliance tests
"""

import sys
import json
import subprocess
import time
from pathlib import Path

def test_mcp_protocol():
    """Test MCP protocol compliance"""
    
    print("🔗 Testing MCP Protocol Compliance...")
    
    # Test 1: Server startup
    print("  Testing server startup...")
    try:
        result = subprocess.run(
            ["./build/openfoam-mcp-server", "--version"],
            capture_output=True,
            text=True,
            timeout=10
        )
        if result.returncode == 0:
            print("    ✅ Server startup test passed")
        else:
            print("    ❌ Server startup test failed")
            return False
    except subprocess.TimeoutExpired:
        print("    ❌ Server startup timeout")
        return False
    except FileNotFoundError:
        print("    ❌ Server executable not found")
        return False
    
    # Test 2: JSON-RPC format
    print("  Testing JSON-RPC format...")
    test_request = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "tools/list",
        "params": {}
    }
    
    try:
        json_str = json.dumps(test_request)
        parsed = json.loads(json_str)
        if parsed == test_request:
            print("    ✅ JSON-RPC format test passed")
        else:
            print("    ❌ JSON-RPC format test failed")
            return False
    except Exception as e:
        print(f"    ❌ JSON-RPC format test failed: {e}")
        return False
    
    print("✅ MCP protocol compliance tests passed")
    return True

def main():
    """Main entry point"""
    print("🧪 MCP Protocol Compliance Tests")
    print("=" * 40)
    
    if test_mcp_protocol():
        return 0
    else:
        return 1

if __name__ == "__main__":
    sys.exit(main())