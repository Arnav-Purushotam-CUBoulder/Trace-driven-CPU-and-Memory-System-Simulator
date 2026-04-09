#include "core_simulator.hpp"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <stdexcept>

#include "branch_predictor.hpp"
#include "cache.hpp"

namespace sim {

namespace {

struct RobEntry {
    TraceInstruction instruction;
    bool issued = false;
    bool completed = false;
    bool predictor_updated = false;
    bool predicted_taken = false;
    std::uint64_t completion_cycle = 0;
};

bool is_memory_instruction(const TraceInstruction& instruction) {
    return instruction.type == InstructionType::Load || instruction.type == InstructionType::Store;
}

}  // namespace

CoreSimulator::CoreSimulator(SimulatorConfig config) : config_(std::move(config)) {}

SimulationStats CoreSimulator::run(const std::vector<TraceInstruction>& trace) const {
    config_.validate();
    if (trace.empty()) {
        throw std::runtime_error("Cannot simulate an empty trace");
    }

    Cache cache(config_);
    BranchPredictor predictor(config_);
    SimulationStats stats;
    std::deque<RobEntry> rob;

    std::size_t next_trace_index = 0;
    std::uint64_t cycle = 0;
    std::uint64_t fetch_stall_remaining = 0;

    while (next_trace_index < trace.size() || !rob.empty()) {
        for (auto& entry : rob) {
            if (entry.issued && !entry.completed && entry.completion_cycle <= cycle) {
                entry.completed = true;
                if (entry.instruction.type == InstructionType::Branch && !entry.predictor_updated) {
                    predictor.update(entry.instruction.pc, *entry.instruction.branch_taken);
                    entry.predictor_updated = true;
                }
            }
        }

        std::size_t committed_this_cycle = 0;
        while (committed_this_cycle < config_.width && !rob.empty() && rob.front().completed) {
            ++stats.committed_instructions;
            rob.pop_front();
            ++committed_this_cycle;
        }

        if (!rob.empty() && committed_this_cycle == 0) {
            ++stats.commit_stall_cycles;
            if (is_memory_instruction(rob.front().instruction)) {
                ++stats.head_memory_stall_cycles;
            }
        }

        std::size_t issued_this_cycle = 0;
        for (auto& entry : rob) {
            if (issued_this_cycle == config_.width) {
                break;
            }
            if (entry.issued) {
                continue;
            }

            std::uint64_t latency = 1;
            if (is_memory_instruction(entry.instruction)) {
                const auto access = cache.access(*entry.instruction.address);
                latency = access.latency;
                ++stats.cache_accesses;
                stats.total_memory_latency += latency;
                if (access.hit) {
                    ++stats.cache_hits;
                } else {
                    ++stats.cache_misses;
                    stats.total_miss_penalty_cycles += config_.cache_miss_penalty;
                }
            }

            entry.issued = true;
            entry.completion_cycle = cycle + latency;
            ++issued_this_cycle;
        }

        if (!rob.empty() && issued_this_cycle == 0) {
            ++stats.issue_stall_cycles;
        }

        if (next_trace_index < trace.size()) {
            if (fetch_stall_remaining > 0) {
                ++stats.frontend_branch_stall_cycles;
                --fetch_stall_remaining;
            } else if (rob.size() >= config_.rob_size) {
                ++stats.frontend_rob_stall_cycles;
            } else {
                std::size_t fetched_this_cycle = 0;
                while (fetched_this_cycle < config_.width &&
                       next_trace_index < trace.size() &&
                       rob.size() < config_.rob_size) {
                    RobEntry entry;
                    entry.instruction = trace.at(next_trace_index++);
                    ++stats.fetched_instructions;

                    switch (entry.instruction.type) {
                        case InstructionType::Alu:
                            ++stats.alu_instructions;
                            break;
                        case InstructionType::Load:
                            ++stats.load_instructions;
                            break;
                        case InstructionType::Store:
                            ++stats.store_instructions;
                            break;
                        case InstructionType::Branch:
                            ++stats.branch_instructions;
                            entry.predicted_taken = predictor.predict(entry.instruction.pc);
                            if (entry.predicted_taken != *entry.instruction.branch_taken) {
                                ++stats.branch_mispredictions;
                                fetch_stall_remaining = config_.branch_mispredict_penalty;
                            }
                            break;
                    }

                    rob.push_back(entry);
                    ++fetched_this_cycle;

                    if (entry.instruction.type == InstructionType::Branch &&
                        entry.predicted_taken != *entry.instruction.branch_taken) {
                        break;
                    }
                }
            }
        }

        stats.max_rob_occupancy =
            std::max<std::uint64_t>(stats.max_rob_occupancy, rob.size());
        ++cycle;
    }

    stats.cycles = cycle;
    return stats;
}

}  // namespace sim
