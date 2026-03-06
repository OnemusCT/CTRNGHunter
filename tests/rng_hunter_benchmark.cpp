#include <chrono>
#include <fstream>
#include <filesystem>
#include <print>

#include "rng_hunter.h"

int main() {
    std::filesystem::path tmp_dir = std::filesystem::temp_directory_path() / "rng_hunter_bench";
    std::filesystem::create_directories(tmp_dir);
    auto route_path = tmp_dir / "bench_route.txt";

    // Just enough actions to require a meaningful amount of time
    std::ofstream f(route_path);
    f << "load\n";
    f << "room 3\n";
    f << "battle_with_rng 1A\n";
    f << "heal 5\n";
    f << "battle_with_crits 10,20,30 2 5\n";
    f << "battle_with_rng 1A\n";
    f << "battle_with_rng 1B\n";
    f.close();

    std::print("Starting RNGHunter Benchmark...");

    RNGHunter hunter(/*max_seeds=*/1000, /*pool_size=*/1);
    if (!hunter.parseFile(route_path.string())) {
        std::print(stderr, "Failed to parse benchmark route file.");
        return 1;
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    hunter.findSeeds(0, 100000000, 2, 2, RNGSim::LogLevel::NONE);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::print("Benchmark Complete!");
    std::print("Execution time (100,000,000 seeds, 1 thread): {} ms", duration_ms.count());

    std::filesystem::remove_all(tmp_dir);
    return 0;
}
