#!/usr/bin/env python3
"""
Performance Benchmark Tests for OpenFOAM MCP Server

Measures tool execution times and resource usage.
"""

import sys
import json
import subprocess
import time
import psutil
from pathlib import Path

def benchmark_pipe_flow_tool():
    """Benchmark pipe flow tool performance"""
    print("🚀 Benchmarking pipe flow tool...")
    
    test_cases = [
        {"name": "Small Case", "diameter": 0.01, "length": 0.1, "velocity": 1.0},
        {"name": "Medium Case", "diameter": 0.05, "length": 0.5, "velocity": 5.0},
        {"name": "Large Case", "diameter": 0.1, "length": 1.0, "velocity": 10.0}
    ]
    
    results = []
    
    for case in test_cases:
        print(f"  Testing {case['name']}...")
        
        # Prepare test input
        test_input = {
            "diameter": case["diameter"],
            "length": case["length"],
            "velocity": case["velocity"],
            "fluid": "water",
            "temperature": 298.15,
            "roughness": 0.000045
        }
        
        # Run benchmark
        start_time = time.time()
        memory_before = psutil.Process().memory_info().rss / 1024 / 1024  # MB
        
        success = run_tool_benchmark("analyze_pipe_flow", test_input)
        
        end_time = time.time()
        memory_after = psutil.Process().memory_info().rss / 1024 / 1024  # MB
        
        execution_time = end_time - start_time
        memory_usage = memory_after - memory_before
        
        results.append({
            "case": case["name"],
            "success": success,
            "time": execution_time,
            "memory": memory_usage
        })
        
        status = "✅" if success else "❌"
        print(f"    {status} {execution_time:.2f}s, {memory_usage:.1f}MB")
    
    return results

def benchmark_heat_transfer_tool():
    """Benchmark heat transfer tool performance"""
    print("🚀 Benchmarking heat transfer tool...")
    
    test_cases = [
        {
            "name": "Forced Convection",
            "analysis_type": "forced_convection",
            "geometry": {"type": "plate", "length": 0.5, "width": 0.3, "thickness": 0.01},
            "fluid": {"type": "air", "temperature": 298.15, "velocity": 5.0, "pressure": 101325},
            "thermal": {"wall_temperature": 373.15, "heat_flux": 1000}
        },
        {
            "name": "Natural Convection",
            "analysis_type": "natural_convection",
            "geometry": {"type": "vertical_plate", "height": 1.0, "width": 0.5, "thickness": 0.01},
            "fluid": {"type": "air", "temperature": 298.15, "pressure": 101325},
            "thermal": {"wall_temperature": 323.15, "ambient_temperature": 298.15}
        }
    ]
    
    results = []
    
    for case in test_cases:
        print(f"  Testing {case['name']}...")
        
        # Run benchmark
        start_time = time.time()
        memory_before = psutil.Process().memory_info().rss / 1024 / 1024  # MB
        
        success = run_tool_benchmark("analyze_heat_transfer", case)
        
        end_time = time.time()
        memory_after = psutil.Process().memory_info().rss / 1024 / 1024  # MB
        
        execution_time = end_time - start_time
        memory_usage = memory_after - memory_before
        
        results.append({
            "case": case["name"],
            "success": success,
            "time": execution_time,
            "memory": memory_usage
        })
        
        status = "✅" if success else "❌"
        print(f"    {status} {execution_time:.2f}s, {memory_usage:.1f}MB")
    
    return results

def benchmark_external_flow_tool():
    """Benchmark external flow tool performance"""
    print("🚀 Benchmarking external flow tool...")
    
    test_cases = [
        {
            "name": "Flat Plate",
            "geometry": {"type": "flat_plate", "length": 1.0, "width": 0.5, "thickness": 0.01},
            "flow_conditions": {"velocity": 10.0, "fluid": "air", "temperature": 298.15, "pressure": 101325, "turbulence_intensity": 0.01},
            "analysis_type": "drag_analysis"
        },
        {
            "name": "Cylinder",
            "geometry": {"type": "cylinder", "diameter": 0.1, "length": 1.0},
            "flow_conditions": {"velocity": 20.0, "fluid": "air", "temperature": 298.15, "pressure": 101325, "turbulence_intensity": 0.05},
            "analysis_type": "drag_analysis"
        }
    ]
    
    results = []
    
    for case in test_cases:
        print(f"  Testing {case['name']}...")
        
        # Run benchmark
        start_time = time.time()
        memory_before = psutil.Process().memory_info().rss / 1024 / 1024  # MB
        
        success = run_tool_benchmark("analyze_external_flow", case)
        
        end_time = time.time()
        memory_after = psutil.Process().memory_info().rss / 1024 / 1024  # MB
        
        execution_time = end_time - start_time
        memory_usage = memory_after - memory_before
        
        results.append({
            "case": case["name"],
            "success": success,
            "time": execution_time,
            "memory": memory_usage
        })
        
        status = "✅" if success else "❌"
        print(f"    {status} {execution_time:.2f}s, {memory_usage:.1f}MB")
    
    return results

def run_tool_benchmark(tool_name, input_params):
    """Execute a tool and return success status"""
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
                "name": tool_name,
                "arguments": input_params
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
        stdout, stderr = process.communicate(json.dumps(request), timeout=60)
        
        if process.returncode != 0:
            return False
        
        # Parse response
        response = json.loads(stdout)
        return "result" in response and "error" not in response
        
    except Exception:
        return False

def analyze_performance_results(all_results):
    """Analyze and report performance results"""
    print("\n📊 Performance Analysis")
    print("=" * 50)
    
    # Calculate statistics
    all_times = []
    all_memory = []
    successful_tests = 0
    total_tests = 0
    
    for tool_results in all_results:
        for result in tool_results:
            total_tests += 1
            if result["success"]:
                successful_tests += 1
                all_times.append(result["time"])
                all_memory.append(result["memory"])
    
    if all_times:
        avg_time = sum(all_times) / len(all_times)
        max_time = max(all_times)
        min_time = min(all_times)
        
        avg_memory = sum(all_memory) / len(all_memory)
        max_memory = max(all_memory)
        min_memory = min(all_memory)
        
        print(f"⏱️  Execution Time:")
        print(f"   Average: {avg_time:.2f}s")
        print(f"   Range: {min_time:.2f}s - {max_time:.2f}s")
        
        print(f"💾 Memory Usage:")
        print(f"   Average: {avg_memory:.1f}MB")
        print(f"   Range: {min_memory:.1f}MB - {max_memory:.1f}MB")
        
        # Performance thresholds
        time_threshold = 10.0  # seconds
        memory_threshold = 500.0  # MB
        
        print(f"\n🎯 Performance Thresholds:")
        if max_time <= time_threshold:
            print(f"   ✅ Time: {max_time:.2f}s ≤ {time_threshold}s")
        else:
            print(f"   ❌ Time: {max_time:.2f}s > {time_threshold}s")
        
        if max_memory <= memory_threshold:
            print(f"   ✅ Memory: {max_memory:.1f}MB ≤ {memory_threshold}MB")
        else:
            print(f"   ❌ Memory: {max_memory:.1f}MB > {memory_threshold}MB")
    
    success_rate = successful_tests / total_tests * 100 if total_tests > 0 else 0
    print(f"\n📈 Success Rate: {successful_tests}/{total_tests} ({success_rate:.1f}%)")
    
    return success_rate >= 90  # 90% success rate required

def main():
    """Run all performance benchmark tests"""
    print("🏎️  OpenFOAM MCP Server - Performance Benchmarks")
    print("=" * 60)
    
    # Run benchmarks
    all_results = []
    
    try:
        pipe_flow_results = benchmark_pipe_flow_tool()
        all_results.append(pipe_flow_results)
        
        heat_transfer_results = benchmark_heat_transfer_tool()
        all_results.append(heat_transfer_results)
        
        external_flow_results = benchmark_external_flow_tool()
        all_results.append(external_flow_results)
        
        # Analyze results
        performance_acceptable = analyze_performance_results(all_results)
        
        if performance_acceptable:
            print("\n🎉 All performance benchmarks passed!")
            return 0
        else:
            print("\n⚠️  Performance benchmarks failed!")
            return 1
            
    except Exception as e:
        print(f"\n💥 Benchmark execution failed: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())