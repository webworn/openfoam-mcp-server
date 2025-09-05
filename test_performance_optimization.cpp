#include "src/physics/cantera_wrapper.hpp"
#include <iostream>
#include <chrono>

using namespace Foam::MCP::Physics;

int main() {
    std::cout << "=== Performance Optimization Test ===" << std::endl;
    
    try {
        CanteraWrapper wrapper;
        wrapper.setVerboseMode(true);
        
        // Test conditions
        double pressure = 101325.0;    // 1 atm
        double temperature = 300.0;    // 300 K
        std::string composition = "H2:2 O2:1 N2:3.76";
        std::string mechanism = "sandiego20161214_H2only.yaml";
        
        std::cout << "\n=== Phase 1: Performance without caching ===" << std::endl;
        
        // Test without caching - 5 identical calculations
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < 5; i++) {
            double result = wrapper.calculateCJSpeed(pressure, temperature, composition, mechanism);
            std::cout << "Calculation " << (i+1) << ": " << result << " m/s" << std::endl;
        }
        auto end = std::chrono::steady_clock::now();
        double noCacheTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "Total time without caching: " << noCacheTime << "ms" << std::endl;
        std::cout << "Average time per calculation: " << (noCacheTime / 5.0) << "ms" << std::endl;
        
        // Get performance metrics
        auto metrics = wrapper.getPerformanceMetrics();
        std::cout << "Performance metrics:" << std::endl;
        std::cout << "  Total executions: " << metrics.totalExecutions << std::endl;
        std::cout << "  Average execution time: " << metrics.averageExecutionTime << "ms" << std::endl;
        std::cout << "  Min execution time: " << metrics.minExecutionTime << "ms" << std::endl;
        std::cout << "  Max execution time: " << metrics.maxExecutionTime << "ms" << std::endl;
        
        std::cout << "\n=== Phase 2: Performance with caching ===" << std::endl;
        
        // Enable caching
        wrapper.enableCaching(true, "performance_test_cache.json");
        
        // Reset performance metrics
        wrapper.resetPerformanceMetrics();
        
        // Test with caching - same 5 calculations
        start = std::chrono::steady_clock::now();
        for (int i = 0; i < 5; i++) {
            double result = wrapper.calculateCJSpeed(pressure, temperature, composition, mechanism);
            std::cout << "Calculation " << (i+1) << ": " << result << " m/s" << std::endl;
        }
        end = std::chrono::steady_clock::now();
        double cacheTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "Total time with caching: " << cacheTime << "ms" << std::endl;
        std::cout << "Average time per calculation: " << (cacheTime / 5.0) << "ms" << std::endl;
        
        // Cache statistics
        std::cout << "\nCache statistics:" << std::endl;
        std::cout << "  Cache hits: " << wrapper.getCacheHits() << std::endl;
        std::cout << "  Cache misses: " << wrapper.getCacheMisses() << std::endl;
        std::cout << "  Cache hit rate: " << (wrapper.getCacheHitRate() * 100.0) << "%" << std::endl;
        
        // Performance improvement
        double speedup = noCacheTime / cacheTime;
        std::cout << "\n=== Performance Analysis ===" << std::endl;
        std::cout << "Speed improvement: " << speedup << "x faster" << std::endl;
        std::cout << "Time saved: " << (noCacheTime - cacheTime) << "ms (" 
                  << (((noCacheTime - cacheTime) / noCacheTime) * 100.0) << "% reduction)" << std::endl;
        
        // Get updated performance metrics
        auto cachedMetrics = wrapper.getPerformanceMetrics();
        std::cout << "\nCached performance metrics:" << std::endl;
        std::cout << "  Total executions: " << cachedMetrics.totalExecutions << std::endl;
        std::cout << "  Average execution time: " << cachedMetrics.averageExecutionTime << "ms" << std::endl;
        std::cout << "  Min execution time: " << cachedMetrics.minExecutionTime << "ms" << std::endl;
        std::cout << "  Max execution time: " << cachedMetrics.maxExecutionTime << "ms" << std::endl;
        
        std::cout << "\n=== Phase 3: Different conditions test ===" << std::endl;
        
        // Test different conditions (should miss cache)
        struct TestCase {
            double pressure;
            double temperature;
            std::string composition;
            std::string name;
        };
        
        std::vector<TestCase> testCases = {
            {101325.0, 300.0, "H2:2 O2:1 N2:3.76", "Standard H2-air"},
            {202650.0, 300.0, "H2:2 O2:1 N2:3.76", "High pressure H2-air"},
            {101325.0, 400.0, "H2:2 O2:1 N2:3.76", "High temperature H2-air"},
            {101325.0, 300.0, "H2:4 O2:1 N2:3.76", "Rich H2-air"},
            {101325.0, 300.0, "H2:1.2 O2:1 N2:3.76", "Lean H2-air"}
        };
        
        wrapper.resetPerformanceMetrics();
        
        for (const auto& testCase : testCases) {
            auto testStart = std::chrono::steady_clock::now();
            double result = wrapper.calculateCJSpeed(testCase.pressure, testCase.temperature, 
                                                   testCase.composition, mechanism);
            auto testEnd = std::chrono::steady_clock::now();
            double testTime = std::chrono::duration_cast<std::chrono::milliseconds>(testEnd - testStart).count();
            
            std::cout << testCase.name << ": " << result << " m/s (" << testTime << "ms)" << std::endl;
        }
        
        // Final cache statistics
        std::cout << "\nFinal cache statistics:" << std::endl;
        std::cout << "  Cache hits: " << wrapper.getCacheHits() << std::endl;
        std::cout << "  Cache misses: " << wrapper.getCacheMisses() << std::endl;
        std::cout << "  Cache hit rate: " << (wrapper.getCacheHitRate() * 100.0) << "%" << std::endl;
        
        // Save cache to file
        wrapper.saveCacheToFile();
        std::cout << "\nCache saved to performance_test_cache.json" << std::endl;
        
        std::cout << "\n=== Phase 4: Cache persistence test ===" << std::endl;
        
        // Create new wrapper and load cache
        CanteraWrapper newWrapper;
        newWrapper.setVerboseMode(true);
        newWrapper.enableCaching(true, "performance_test_cache.json");
        
        // Test with loaded cache
        auto persistStart = std::chrono::steady_clock::now();
        double persistResult = newWrapper.calculateCJSpeed(pressure, temperature, composition, mechanism);
        auto persistEnd = std::chrono::steady_clock::now();
        double persistTime = std::chrono::duration_cast<std::chrono::milliseconds>(persistEnd - persistStart).count();
        
        std::cout << "Loaded from persistent cache: " << persistResult << " m/s (" << persistTime << "ms)" << std::endl;
        std::cout << "Cache hits: " << newWrapper.getCacheHits() << std::endl;
        std::cout << "Cache misses: " << newWrapper.getCacheMisses() << std::endl;
        
        if (newWrapper.getCacheHits() > 0) {
            std::cout << "✓ Cache persistence working correctly!" << std::endl;
        } else {
            std::cout << "✗ Cache persistence not working" << std::endl;
        }
        
        std::cout << "\n=== Performance Optimization Test Completed ===" << std::endl;
        
        // Performance targets check
        if (speedup > 5.0) {
            std::cout << "🎯 TARGET ACHIEVED: >5x speed improvement!" << std::endl;
        } else if (speedup > 2.0) {
            std::cout << "✓ Good improvement: " << speedup << "x faster" << std::endl;
        } else {
            std::cout << "⚠ Improvement limited: " << speedup << "x faster" << std::endl;
        }
        
        if (wrapper.getCacheHitRate() > 0.8) {
            std::cout << "🎯 TARGET ACHIEVED: >80% cache hit rate!" << std::endl;
        } else {
            std::cout << "📊 Cache hit rate: " << (wrapper.getCacheHitRate() * 100.0) << "%" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}