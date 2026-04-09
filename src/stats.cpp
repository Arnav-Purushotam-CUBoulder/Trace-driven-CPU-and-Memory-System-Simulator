#include "stats.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace sim {

namespace {

std::string escape_json(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char character : value) {
        switch (character) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            default:
                escaped += character;
                break;
        }
    }
    return escaped;
}

std::string cache_description(const SimulatorConfig& config) {
    if (config.cache_associativity == 1) {
        return "direct-mapped";
    }
    return std::to_string(config.cache_associativity) + "-way set-associative";
}

}  // namespace

double SimulationStats::ipc() const {
    if (cycles == 0) {
        return 0.0;
    }
    return static_cast<double>(committed_instructions) / static_cast<double>(cycles);
}

double SimulationStats::cache_miss_rate() const {
    if (cache_accesses == 0) {
        return 0.0;
    }
    return static_cast<double>(cache_misses) / static_cast<double>(cache_accesses);
}

double SimulationStats::branch_mispredict_rate() const {
    if (branch_instructions == 0) {
        return 0.0;
    }
    return static_cast<double>(branch_mispredictions) / static_cast<double>(branch_instructions);
}

double SimulationStats::branch_accuracy() const {
    if (branch_instructions == 0) {
        return 1.0;
    }
    return 1.0 - branch_mispredict_rate();
}

std::string SimulationStats::to_text(
    const SimulatorConfig& config,
    const std::string& trace_name
) const {
    std::ostringstream output;
    output << std::fixed << std::setprecision(4);
    output << "Trace: " << trace_name << '\n';
    output << "Config:\n";
    output << "  width=" << config.width << ", rob=" << config.rob_size
           << ", predictor=" << to_string(config.predictor_kind)
           << ", cache=" << config.cache_size_bytes / 1024 << "KB "
           << cache_description(config) << ", line=" << config.cache_line_size
           << "B, sets=" << config.cache_set_count() << '\n';
    output << "Performance:\n";
    output << "  cycles=" << cycles << ", committed=" << committed_instructions
           << ", IPC=" << ipc() << '\n';
    output << "Branching:\n";
    output << "  branches=" << branch_instructions
           << ", mispredictions=" << branch_mispredictions
           << ", accuracy=" << (branch_accuracy() * 100.0) << "%\n";
    output << "Cache:\n";
    output << "  accesses=" << cache_accesses << ", hits=" << cache_hits
           << ", misses=" << cache_misses
           << ", miss_rate=" << (cache_miss_rate() * 100.0) << "%\n";
    output << "Stalls:\n";
    output << "  branch_frontend=" << frontend_branch_stall_cycles
           << ", rob_frontend=" << frontend_rob_stall_cycles
           << ", issue=" << issue_stall_cycles
           << ", commit=" << commit_stall_cycles
           << ", head_memory=" << head_memory_stall_cycles << '\n';
    return output.str();
}

std::string SimulationStats::to_json(
    const SimulatorConfig& config,
    const std::string& trace_name
) const {
    std::ostringstream output;
    output << std::fixed << std::setprecision(6);
    output << "{";
    output << "\"trace\":\"" << escape_json(trace_name) << "\",";
    output << "\"width\":" << config.width << ',';
    output << "\"rob_size\":" << config.rob_size << ',';
    output << "\"predictor\":\"" << to_string(config.predictor_kind) << "\",";
    output << "\"branch_predictor_entries\":" << config.branch_predictor_entries << ',';
    output << "\"branch_mispredict_penalty\":" << config.branch_mispredict_penalty << ',';
    output << "\"cache_size_bytes\":" << config.cache_size_bytes << ',';
    output << "\"cache_line_size\":" << config.cache_line_size << ',';
    output << "\"cache_associativity\":" << config.cache_associativity << ',';
    output << "\"cache_sets\":" << config.cache_set_count() << ',';
    output << "\"cache_hit_latency\":" << config.cache_hit_latency << ',';
    output << "\"cache_miss_penalty\":" << config.cache_miss_penalty << ',';
    output << "\"cycles\":" << cycles << ',';
    output << "\"fetched_instructions\":" << fetched_instructions << ',';
    output << "\"committed_instructions\":" << committed_instructions << ',';
    output << "\"ipc\":" << ipc() << ',';
    output << "\"alu_instructions\":" << alu_instructions << ',';
    output << "\"load_instructions\":" << load_instructions << ',';
    output << "\"store_instructions\":" << store_instructions << ',';
    output << "\"branch_instructions\":" << branch_instructions << ',';
    output << "\"branch_mispredictions\":" << branch_mispredictions << ',';
    output << "\"branch_accuracy\":" << branch_accuracy() << ',';
    output << "\"cache_accesses\":" << cache_accesses << ',';
    output << "\"cache_hits\":" << cache_hits << ',';
    output << "\"cache_misses\":" << cache_misses << ',';
    output << "\"cache_miss_rate\":" << cache_miss_rate() << ',';
    output << "\"frontend_branch_stall_cycles\":" << frontend_branch_stall_cycles << ',';
    output << "\"frontend_rob_stall_cycles\":" << frontend_rob_stall_cycles << ',';
    output << "\"issue_stall_cycles\":" << issue_stall_cycles << ',';
    output << "\"commit_stall_cycles\":" << commit_stall_cycles << ',';
    output << "\"head_memory_stall_cycles\":" << head_memory_stall_cycles << ',';
    output << "\"max_rob_occupancy\":" << max_rob_occupancy << ',';
    output << "\"total_memory_latency\":" << total_memory_latency << ',';
    output << "\"total_miss_penalty_cycles\":" << total_miss_penalty_cycles;
    output << "}";
    return output.str();
}

}  // namespace sim
