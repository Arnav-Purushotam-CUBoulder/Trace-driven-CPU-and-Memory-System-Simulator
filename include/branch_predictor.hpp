#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "simulator_config.hpp"

namespace sim {

class BranchPredictor {
  public:
    explicit BranchPredictor(const SimulatorConfig& config);

    [[nodiscard]] bool predict(std::uint64_t pc) const;
    void update(std::uint64_t pc, bool taken);
    [[nodiscard]] std::size_t table_size() const;

  private:
    [[nodiscard]] std::size_t index_for(std::uint64_t pc) const;

    BranchPredictorKind kind_;
    std::vector<std::uint8_t> table_;
};

}  // namespace sim
