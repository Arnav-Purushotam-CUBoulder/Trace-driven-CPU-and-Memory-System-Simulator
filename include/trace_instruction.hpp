#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace sim {

enum class InstructionType {
    Alu,
    Load,
    Store,
    Branch,
};

struct TraceInstruction {
    std::uint64_t sequence = 0;
    std::uint64_t pc = 0;
    InstructionType type = InstructionType::Alu;
    std::optional<std::uint64_t> address;
    std::optional<bool> branch_taken;
};

std::string to_string(InstructionType type);

}  // namespace sim
