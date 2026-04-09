#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace sim {

enum class BranchPredictorKind {
    AlwaysTaken,
    AlwaysNotTaken,
    OneBit,
    TwoBit,
};

struct SimulatorConfig {
    std::size_t width = 4;
    std::size_t rob_size = 64;
    std::size_t cache_size_bytes = 16 * 1024;
    std::size_t cache_line_size = 64;
    std::size_t cache_associativity = 1;
    std::uint64_t cache_hit_latency = 1;
    std::uint64_t cache_miss_penalty = 50;
    BranchPredictorKind predictor_kind = BranchPredictorKind::TwoBit;
    std::size_t branch_predictor_entries = 1024;
    std::uint64_t branch_mispredict_penalty = 6;

    [[nodiscard]] std::size_t cache_set_count() const;
    void validate() const;
};

std::string to_string(BranchPredictorKind kind);
BranchPredictorKind parse_branch_predictor_kind(const std::string& value);

}  // namespace sim
