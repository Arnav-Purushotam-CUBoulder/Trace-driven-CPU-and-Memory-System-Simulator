#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "core_simulator.hpp"
#include "trace_reader.hpp"

namespace {

enum class OutputFormat {
    Text,
    Json,
};

std::size_t parse_size(const std::string& token, const std::string& argument_name) {
    try {
        std::size_t consumed = 0;
        const auto value = std::stoull(token, &consumed, 0);
        if (consumed != token.size()) {
            throw std::invalid_argument("trailing characters");
        }
        return value;
    } catch (const std::exception&) {
        throw std::runtime_error("Failed to parse " + argument_name + ": " + token);
    }
}

void print_usage() {
    std::cout
        << "Trace-driven CPU and memory-system simulator\n"
        << "Usage:\n"
        << "  td_sim --trace <path> [options]\n\n"
        << "Options:\n"
        << "  --width <n>                    pipeline width for fetch/issue/commit\n"
        << "  --rob <n>                      reorder buffer entries\n"
        << "  --cache-size <bytes>           total data-cache capacity\n"
        << "  --cache-line-size <bytes>      cache line size\n"
        << "  --cache-assoc <n>              1 for direct-mapped, >1 for set-associative\n"
        << "  --cache-hit-latency <cycles>   cache hit latency\n"
        << "  --cache-miss-penalty <cycles>  extra miss latency\n"
        << "  --predictor <name>             always_taken | always_not_taken | one_bit | two_bit\n"
        << "  --bp-entries <n>               predictor table size for dynamic predictors\n"
        << "  --mispredict-penalty <cycles>  frontend recovery penalty\n"
        << "  --format <text|json>           output format\n"
        << "  --json                         shorthand for --format json\n"
        << "  --help                         print this message\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        sim::SimulatorConfig config;
        std::filesystem::path trace_path;
        OutputFormat output_format = OutputFormat::Text;

        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            const auto require_value = [&](const std::string& name) -> std::string {
                if (index + 1 >= argc) {
                    throw std::runtime_error("Missing value for " + name);
                }
                return argv[++index];
            };

            if (argument == "--trace") {
                trace_path = require_value(argument);
            } else if (argument == "--width") {
                config.width = parse_size(require_value(argument), argument);
            } else if (argument == "--rob") {
                config.rob_size = parse_size(require_value(argument), argument);
            } else if (argument == "--cache-size") {
                config.cache_size_bytes = parse_size(require_value(argument), argument);
            } else if (argument == "--cache-line-size") {
                config.cache_line_size = parse_size(require_value(argument), argument);
            } else if (argument == "--cache-assoc") {
                config.cache_associativity = parse_size(require_value(argument), argument);
            } else if (argument == "--cache-hit-latency") {
                config.cache_hit_latency =
                    parse_size(require_value(argument), argument);
            } else if (argument == "--cache-miss-penalty") {
                config.cache_miss_penalty =
                    parse_size(require_value(argument), argument);
            } else if (argument == "--predictor") {
                config.predictor_kind =
                    sim::parse_branch_predictor_kind(require_value(argument));
            } else if (argument == "--bp-entries") {
                config.branch_predictor_entries =
                    parse_size(require_value(argument), argument);
            } else if (argument == "--mispredict-penalty") {
                config.branch_mispredict_penalty =
                    parse_size(require_value(argument), argument);
            } else if (argument == "--format") {
                const auto value = require_value(argument);
                if (value == "text") {
                    output_format = OutputFormat::Text;
                } else if (value == "json") {
                    output_format = OutputFormat::Json;
                } else {
                    throw std::runtime_error("Unknown output format: " + value);
                }
            } else if (argument == "--json") {
                output_format = OutputFormat::Json;
            } else if (argument == "--help" || argument == "-h") {
                print_usage();
                return 0;
            } else {
                throw std::runtime_error("Unknown argument: " + argument);
            }
        }

        if (trace_path.empty()) {
            throw std::runtime_error("A trace file is required. Use --trace <path>.");
        }

        const auto trace = sim::read_trace(trace_path);
        const sim::CoreSimulator simulator(config);
        const auto stats = simulator.run(trace);

        if (output_format == OutputFormat::Json) {
            std::cout << stats.to_json(config, trace_path.filename().string()) << '\n';
        } else {
            std::cout << stats.to_text(config, trace_path.filename().string());
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
