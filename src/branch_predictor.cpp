#include "branch_predictor.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace sim {

namespace {

std::string lowercase(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); }
    );
    return value;
}

}  // namespace

std::size_t SimulatorConfig::cache_set_count() const {
    return cache_size_bytes / (cache_line_size * cache_associativity);
}

void SimulatorConfig::validate() const {
    if (width == 0) {
        throw std::runtime_error("Pipeline width must be greater than zero");
    }
    if (rob_size == 0) {
        throw std::runtime_error("ROB size must be greater than zero");
    }
    if (cache_size_bytes == 0 || cache_line_size == 0 || cache_associativity == 0) {
        throw std::runtime_error("Cache size, line size, and associativity must be non-zero");
    }
    if ((cache_size_bytes % (cache_line_size * cache_associativity)) != 0) {
        throw std::runtime_error(
            "Cache size must be divisible by line size multiplied by associativity"
        );
    }
    if (cache_set_count() == 0) {
        throw std::runtime_error("Cache configuration produced zero sets");
    }
    if (cache_hit_latency == 0) {
        throw std::runtime_error("Cache hit latency must be greater than zero");
    }
    if ((predictor_kind == BranchPredictorKind::OneBit ||
         predictor_kind == BranchPredictorKind::TwoBit) &&
        branch_predictor_entries == 0) {
        throw std::runtime_error("Dynamic branch predictors require at least one table entry");
    }
}

std::string to_string(const BranchPredictorKind kind) {
    switch (kind) {
        case BranchPredictorKind::AlwaysTaken:
            return "always_taken";
        case BranchPredictorKind::AlwaysNotTaken:
            return "always_not_taken";
        case BranchPredictorKind::OneBit:
            return "one_bit";
        case BranchPredictorKind::TwoBit:
            return "two_bit";
    }
    throw std::runtime_error("Unhandled branch predictor kind");
}

BranchPredictorKind parse_branch_predictor_kind(const std::string& value) {
    const auto normalized = lowercase(value);
    if (normalized == "always_taken" || normalized == "always-taken") {
        return BranchPredictorKind::AlwaysTaken;
    }
    if (normalized == "always_not_taken" || normalized == "always-not-taken" ||
        normalized == "not_taken" || normalized == "not-taken") {
        return BranchPredictorKind::AlwaysNotTaken;
    }
    if (normalized == "one_bit" || normalized == "one-bit" || normalized == "1bit") {
        return BranchPredictorKind::OneBit;
    }
    if (normalized == "two_bit" || normalized == "two-bit" || normalized == "2bit" ||
        normalized == "bimodal" || normalized == "bimodal_2bit") {
        return BranchPredictorKind::TwoBit;
    }
    throw std::runtime_error("Unknown branch predictor: " + value);
}

BranchPredictor::BranchPredictor(const SimulatorConfig& config)
    : kind_(config.predictor_kind) {
    if (kind_ == BranchPredictorKind::OneBit) {
        table_.assign(config.branch_predictor_entries, 0);
    } else if (kind_ == BranchPredictorKind::TwoBit) {
        table_.assign(config.branch_predictor_entries, 1);
    }
}

bool BranchPredictor::predict(const std::uint64_t pc) const {
    switch (kind_) {
        case BranchPredictorKind::AlwaysTaken:
            return true;
        case BranchPredictorKind::AlwaysNotTaken:
            return false;
        case BranchPredictorKind::OneBit:
            return table_.at(index_for(pc)) != 0;
        case BranchPredictorKind::TwoBit:
            return table_.at(index_for(pc)) >= 2;
    }
    throw std::runtime_error("Unhandled branch predictor kind");
}

void BranchPredictor::update(const std::uint64_t pc, const bool taken) {
    if (kind_ == BranchPredictorKind::AlwaysTaken || kind_ == BranchPredictorKind::AlwaysNotTaken) {
        return;
    }

    auto& counter = table_.at(index_for(pc));
    if (kind_ == BranchPredictorKind::OneBit) {
        counter = taken ? 1 : 0;
        return;
    }

    if (taken) {
        counter = static_cast<std::uint8_t>(std::min<std::uint8_t>(3, counter + 1));
    } else {
        counter = static_cast<std::uint8_t>(counter == 0 ? 0 : counter - 1);
    }
}

std::size_t BranchPredictor::table_size() const {
    return table_.size();
}

std::size_t BranchPredictor::index_for(const std::uint64_t pc) const {
    return static_cast<std::size_t>((pc >> 2U) % table_.size());
}

}  // namespace sim
