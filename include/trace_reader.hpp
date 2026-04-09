#pragma once

#include <filesystem>
#include <vector>

#include "trace_instruction.hpp"

namespace sim {

std::vector<TraceInstruction> read_trace(const std::filesystem::path& path);

}  // namespace sim
