#pragma once

#include <vector>

#include "simulator_config.hpp"
#include "stats.hpp"
#include "trace_instruction.hpp"

namespace sim {

class CoreSimulator {
  public:
    explicit CoreSimulator(SimulatorConfig config);

    [[nodiscard]] SimulationStats run(const std::vector<TraceInstruction>& trace) const;

  private:
    SimulatorConfig config_;
};

}  // namespace sim
