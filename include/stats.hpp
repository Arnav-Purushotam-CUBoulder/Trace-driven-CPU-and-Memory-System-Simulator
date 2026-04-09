#pragma once

#include <cstdint>
#include <string>

#include "simulator_config.hpp"

namespace sim {

struct SimulationStats {
    std::uint64_t cycles = 0;
    std::uint64_t fetched_instructions = 0;
    std::uint64_t committed_instructions = 0;

    std::uint64_t alu_instructions = 0;
    std::uint64_t load_instructions = 0;
    std::uint64_t store_instructions = 0;
    std::uint64_t branch_instructions = 0;
    std::uint64_t branch_mispredictions = 0;

    std::uint64_t cache_accesses = 0;
    std::uint64_t cache_hits = 0;
    std::uint64_t cache_misses = 0;

    std::uint64_t frontend_branch_stall_cycles = 0;
    std::uint64_t frontend_rob_stall_cycles = 0;
    std::uint64_t issue_stall_cycles = 0;
    std::uint64_t commit_stall_cycles = 0;
    std::uint64_t head_memory_stall_cycles = 0;

    std::uint64_t max_rob_occupancy = 0;
    std::uint64_t total_memory_latency = 0;
    std::uint64_t total_miss_penalty_cycles = 0;

    [[nodiscard]] double ipc() const;
    [[nodiscard]] double cache_miss_rate() const;
    [[nodiscard]] double branch_mispredict_rate() const;
    [[nodiscard]] double branch_accuracy() const;
    [[nodiscard]] std::string to_text(
        const SimulatorConfig& config,
        const std::string& trace_name
    ) const;
    [[nodiscard]] std::string to_json(
        const SimulatorConfig& config,
        const std::string& trace_name
    ) const;
};

}  // namespace sim
