#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "simulator_config.hpp"

namespace sim {

struct CacheAccessResult {
    bool hit = true;
    std::uint64_t latency = 0;
    std::size_t set_index = 0;
    std::size_t way_index = 0;
};

class Cache {
  public:
    explicit Cache(const SimulatorConfig& config);

    [[nodiscard]] CacheAccessResult access(std::uint64_t address);
    [[nodiscard]] std::uint64_t accesses() const;
    [[nodiscard]] std::uint64_t hits() const;
    [[nodiscard]] std::uint64_t misses() const;
    [[nodiscard]] std::size_t set_count() const;

  private:
    struct Line {
        bool valid = false;
        std::uint64_t tag = 0;
        std::uint64_t last_used = 0;
    };

    SimulatorConfig config_;
    std::vector<std::vector<Line>> sets_;
    std::uint64_t timestamp_ = 0;
    std::uint64_t accesses_ = 0;
    std::uint64_t hits_ = 0;
    std::uint64_t misses_ = 0;
};

}  // namespace sim
