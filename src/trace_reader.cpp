#include "trace_reader.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace sim {

namespace {

std::string trim(std::string value) {
    auto not_space = [](unsigned char character) { return !std::isspace(character); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::string uppercase(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char character) { return static_cast<char>(std::toupper(character)); }
    );
    return value;
}

std::uint64_t parse_u64(const std::string& token, const std::string& field_name, std::size_t line) {
    try {
        std::size_t consumed = 0;
        const auto value = std::stoull(token, &consumed, 0);
        if (consumed != token.size()) {
            throw std::invalid_argument("trailing characters");
        }
        return value;
    } catch (const std::exception&) {
        throw std::runtime_error(
            "Failed to parse " + field_name + " on line " + std::to_string(line) + ": " + token
        );
    }
}

bool parse_branch_outcome(const std::string& token, std::size_t line) {
    const auto value = uppercase(token);
    if (value == "T" || value == "TAKEN" || value == "1") {
        return true;
    }
    if (value == "N" || value == "NT" || value == "NOTTAKEN" || value == "0") {
        return false;
    }
    throw std::runtime_error(
        "Failed to parse branch outcome on line " + std::to_string(line) + ": " + token
    );
}

}  // namespace

std::string to_string(const InstructionType type) {
    switch (type) {
        case InstructionType::Alu:
            return "ALU";
        case InstructionType::Load:
            return "LOAD";
        case InstructionType::Store:
            return "STORE";
        case InstructionType::Branch:
            return "BRANCH";
    }
    throw std::runtime_error("Unhandled instruction type");
}

std::vector<TraceInstruction> read_trace(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to open trace: " + path.string());
    }

    std::vector<TraceInstruction> trace;
    std::string raw_line;
    std::size_t line_number = 0;

    while (std::getline(input, raw_line)) {
        ++line_number;
        const auto comment_offset = raw_line.find('#');
        if (comment_offset != std::string::npos) {
            raw_line = raw_line.substr(0, comment_offset);
        }

        const auto line = trim(raw_line);
        if (line.empty()) {
            continue;
        }

        std::istringstream stream(line);
        std::string opcode_token;
        std::string pc_token;
        stream >> opcode_token >> pc_token;
        if (opcode_token.empty() || pc_token.empty()) {
            throw std::runtime_error("Malformed trace line " + std::to_string(line_number));
        }

        TraceInstruction instruction;
        instruction.sequence = trace.size();
        instruction.pc = parse_u64(pc_token, "pc", line_number);

        const auto opcode = uppercase(opcode_token);
        if (opcode == "ALU") {
            instruction.type = InstructionType::Alu;
        } else if (opcode == "LOAD") {
            std::string address_token;
            stream >> address_token;
            if (address_token.empty()) {
                throw std::runtime_error(
                    "LOAD is missing an address on line " + std::to_string(line_number)
                );
            }
            instruction.type = InstructionType::Load;
            instruction.address = parse_u64(address_token, "address", line_number);
        } else if (opcode == "STORE") {
            std::string address_token;
            stream >> address_token;
            if (address_token.empty()) {
                throw std::runtime_error(
                    "STORE is missing an address on line " + std::to_string(line_number)
                );
            }
            instruction.type = InstructionType::Store;
            instruction.address = parse_u64(address_token, "address", line_number);
        } else if (opcode == "BRANCH") {
            std::string outcome_token;
            stream >> outcome_token;
            if (outcome_token.empty()) {
                throw std::runtime_error(
                    "BRANCH is missing an outcome on line " + std::to_string(line_number)
                );
            }
            instruction.type = InstructionType::Branch;
            instruction.branch_taken = parse_branch_outcome(outcome_token, line_number);
        } else {
            throw std::runtime_error(
                "Unknown opcode on line " + std::to_string(line_number) + ": " + opcode_token
            );
        }

        trace.push_back(instruction);
    }

    if (trace.empty()) {
        throw std::runtime_error("Trace is empty: " + path.string());
    }

    return trace;
}

}  // namespace sim
