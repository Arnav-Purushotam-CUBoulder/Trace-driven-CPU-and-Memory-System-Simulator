#include <vector>

#include "core_simulator.hpp"
#include "test_framework.hpp"

namespace {

sim::TraceInstruction make_alu(const std::uint64_t sequence, const std::uint64_t pc) {
    sim::TraceInstruction instruction;
    instruction.sequence = sequence;
    instruction.pc = pc;
    instruction.type = sim::InstructionType::Alu;
    return instruction;
}

sim::TraceInstruction make_load(
    const std::uint64_t sequence,
    const std::uint64_t pc,
    const std::uint64_t address
) {
    sim::TraceInstruction instruction;
    instruction.sequence = sequence;
    instruction.pc = pc;
    instruction.type = sim::InstructionType::Load;
    instruction.address = address;
    return instruction;
}

sim::TraceInstruction make_branch(
    const std::uint64_t sequence,
    const std::uint64_t pc,
    const bool taken
) {
    sim::TraceInstruction instruction;
    instruction.sequence = sequence;
    instruction.pc = pc;
    instruction.type = sim::InstructionType::Branch;
    instruction.branch_taken = taken;
    return instruction;
}

}  // namespace

TEST_CASE("simulator_sustains_simple_alu_throughput") {
    sim::SimulatorConfig config;
    config.width = 2;
    config.rob_size = 8;

    const std::vector<sim::TraceInstruction> trace = {
        make_alu(0, 0x1000),
        make_alu(1, 0x1004),
        make_alu(2, 0x1008),
        make_alu(3, 0x100c),
    };

    const sim::CoreSimulator simulator(config);
    const auto stats = simulator.run(trace);

    EXPECT_EQ(stats.fetched_instructions, 4ULL);
    EXPECT_EQ(stats.committed_instructions, 4ULL);
    EXPECT_EQ(stats.cycles, 4ULL);
    EXPECT_NEAR(stats.ipc(), 1.0, 1e-9);
}

TEST_CASE("simulator_reports_cache_and_branch_behavior") {
    sim::SimulatorConfig config;
    config.width = 1;
    config.rob_size = 4;
    config.cache_size_bytes = 64;
    config.cache_line_size = 16;
    config.cache_associativity = 1;
    config.cache_hit_latency = 1;
    config.cache_miss_penalty = 5;
    config.predictor_kind = sim::BranchPredictorKind::AlwaysNotTaken;
    config.branch_mispredict_penalty = 3;

    const std::vector<sim::TraceInstruction> trace = {
        make_load(0, 0x2000, 0x0),
        make_load(1, 0x2004, 0x0),
        make_branch(2, 0x2008, true),
        make_alu(3, 0x200C),
    };

    const sim::CoreSimulator simulator(config);
    const auto stats = simulator.run(trace);

    EXPECT_EQ(stats.cache_accesses, 2ULL);
    EXPECT_EQ(stats.cache_hits, 1ULL);
    EXPECT_EQ(stats.cache_misses, 1ULL);
    EXPECT_EQ(stats.branch_instructions, 1ULL);
    EXPECT_EQ(stats.branch_mispredictions, 1ULL);
    EXPECT_EQ(stats.frontend_branch_stall_cycles, 3ULL);
}
