#!/usr/bin/env python3
"""
MCP Protocol Integration Tests

Validates JSON-RPC 2.0 protocol compliance and MCP server behavior.
"""

import sys
import json
import subprocess
import time
from pathlib import Path

def test_mcp_server_startup():
    """Test MCP server starts and responds to initialization"""
    print("🧪 Testing MCP server startup...")
    
    try:
        # Start MCP server
        mcp_cmd = ["/workspaces/openfoam-mcp-server/build/openfoam-mcp-server"]
        process = subprocess.Popen(
            mcp_cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        # Send initialize request
        init_request = {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "initialize",
            "params": {
                "protocolVersion": "2024-11-05",
                "capabilities": {
                    "tools": {}
                },
                "clientInfo": {
                    "name": "test-client",
                    "version": "1.0.0"
                }
            }
        }
        
        stdout, stderr = process.communicate(json.dumps(init_request), timeout=10)
        
        if process.returncode != 0:
            print(f"  ❌ Server failed to start: {stderr}")
            return False
        
        # Parse response
        response = json.loads(stdout)
        
        # Validate response structure
        assert "jsonrpc" in response, "Missing jsonrpc field"
        assert response["jsonrpc"] == "2.0", "Invalid jsonrpc version"
        assert "id" in response, "Missing id field"
        assert response["id"] == 1, "Invalid response id"
        assert "result" in response, "Missing result field"
        
        # Validate server capabilities
        result = response["result"]
        assert "capabilities" in result, "Missing capabilities"
        assert "tools" in result["capabilities"], "Missing tools capability"
        
        print("  ✅ MCP server startup test passed")
        return True
        
    except subprocess.TimeoutExpired:
        print("  ❌ Server startup timeout")
        return False
    except Exception as e:
        print(f"  ❌ Server startup failed: {e}")
        return False

def test_tools_list():
    """Test tools/list endpoint returns available tools"""
    print("🧪 Testing tools/list endpoint...")
    
    try:
        # Build the MCP server if not already built
        build_cmd = ["cmake", "--build", "/workspaces/openfoam-mcp-server/build"]
        subprocess.run(build_cmd, check=True, capture_output=True)
        
        # Create JSON-RPC request
        request = {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/list",
            "params": {}
        }
        
        # Run MCP server
        mcp_cmd = ["/workspaces/openfoam-mcp-server/build/openfoam-mcp-server"]
        process = subprocess.Popen(
            mcp_cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        # Send request and get response
        stdout, stderr = process.communicate(json.dumps(request), timeout=10)
        
        if process.returncode != 0:
            print(f"  ❌ Server failed: {stderr}")
            return False
        
        # Parse response
        response = json.loads(stdout)
        
        # Validate response structure
        assert "jsonrpc" in response, "Missing jsonrpc field"
        assert response["jsonrpc"] == "2.0", "Invalid jsonrpc version"
        assert "id" in response, "Missing id field"
        assert "result" in response, "Missing result field"
        
        # Validate tools list
        result = response["result"]
        assert "tools" in result, "Missing tools array"
        tools = result["tools"]
        
        # Expected tools
        expected_tools = [
            "analyze_pipe_flow",
            "analyze_heat_transfer", 
            "analyze_external_flow"
        ]
        
        tool_names = [tool["name"] for tool in tools]
        
        for expected_tool in expected_tools:
            assert expected_tool in tool_names, f"Missing tool: {expected_tool}"
        
        print(f"  ✅ Found {len(tools)} tools: {tool_names}")
        return True
        
    except subprocess.TimeoutExpired:
        print("  ❌ Tools list timeout")
        return False
    except Exception as e:
        print(f"  ❌ Tools list failed: {e}")
        return False

def test_pipe_flow_tool():
    """Test pipe flow tool execution"""
    print("🧪 Testing pipe flow tool...")
    
    try:
        # Build the MCP server if not already built
        build_cmd = ["cmake", "--build", "/workspaces/openfoam-mcp-server/build"]
        subprocess.run(build_cmd, check=True, capture_output=True)
        
        # Create JSON-RPC request
        request = {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "analyze_pipe_flow",
                "arguments": {
                    "diameter": 0.1,
                    "length": 1.0,
                    "velocity": 1.0,
                    "fluid": "water",
                    "temperature": 298.15,
                    "roughness": 0.000045
                }
            }
        }
        
        # Run MCP server
        mcp_cmd = ["/workspaces/openfoam-mcp-server/build/openfoam-mcp-server"]
        process = subprocess.Popen(
            mcp_cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        # Send request and get response
        stdout, stderr = process.communicate(json.dumps(request), timeout=30)
        
        if process.returncode != 0:
            print(f"  ❌ Tool execution failed: {stderr}")
            return False
        
        # Parse response
        response = json.loads(stdout)
        
        # Validate response structure
        assert "jsonrpc" in response, "Missing jsonrpc field"
        assert response["jsonrpc"] == "2.0", "Invalid jsonrpc version"
        assert "id" in response, "Missing id field"
        assert "result" in response, "Missing result field"
        
        # Validate tool result
        result = response["result"]
        assert "content" in result, "Missing content array"
        
        content = result["content"]
        assert len(content) > 0, "Empty content array"
        
        # Check for expected result structure
        found_resource = False
        for item in content:
            if item.get("type") == "resource":
                found_resource = True
                break
        
        assert found_resource, "No resource content found"
        
        print("  ✅ Pipe flow tool test passed")
        return True
        
    except subprocess.TimeoutExpired:
        print("  ❌ Pipe flow tool timeout")
        return False
    except Exception as e:
        print(f"  ❌ Pipe flow tool failed: {e}")
        return False

def test_invalid_request():
    """Test server handles invalid JSON-RPC requests properly"""
    print("🧪 Testing invalid request handling...")
    
    try:
        # Build the MCP server if not already built
        build_cmd = ["cmake", "--build", "/workspaces/openfoam-mcp-server/build"]
        subprocess.run(build_cmd, check=True, capture_output=True)
        
        # Create invalid JSON-RPC request (missing jsonrpc field)
        request = {
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "nonexistent_tool",
                "arguments": {}
            }
        }
        
        # Run MCP server
        mcp_cmd = ["/workspaces/openfoam-mcp-server/build/openfoam-mcp-server"]
        process = subprocess.Popen(
            mcp_cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        # Send request and get response
        stdout, stderr = process.communicate(json.dumps(request), timeout=10)
        
        # Parse response
        response = json.loads(stdout)
        
        # Should return error response
        assert "jsonrpc" in response, "Missing jsonrpc field"
        assert response["jsonrpc"] == "2.0", "Invalid jsonrpc version"
        assert "id" in response, "Missing id field"
        assert "error" in response, "Missing error field for invalid request"
        
        error = response["error"]
        assert "code" in error, "Missing error code"
        assert "message" in error, "Missing error message"
        
        print("  ✅ Invalid request handling test passed")
        return True
        
    except subprocess.TimeoutExpired:
        print("  ❌ Invalid request test timeout")
        return False
    except Exception as e:
        print(f"  ❌ Invalid request test failed: {e}")
        return False

def main():
    """Run all MCP protocol integration tests"""
    print("🔗 OpenFOAM MCP Server - Protocol Integration Tests")
    print("=" * 60)
    
    # Test suite
    tests = [
        ("MCP Server Startup", test_mcp_server_startup),
        ("Tools List", test_tools_list),
        ("Pipe Flow Tool", test_pipe_flow_tool),
        ("Invalid Request", test_invalid_request)
    ]
    
    # Run tests
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        try:
            if test_func():
                passed += 1
        except Exception as e:
            print(f"  ❌ {test_name} failed with exception: {e}")
    
    # Results
    print(f"\n📊 Integration Test Results: {passed}/{total} passed")
    
    if passed == total:
        print("✅ All MCP protocol integration tests passed!")
        return 0
    else:
        print(f"❌ {total - passed} integration tests failed!")
        return 1

if __name__ == "__main__":
    sys.exit(main())