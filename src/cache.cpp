#include "cache.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace sim {

Cache::Cache(const SimulatorConfig& config) : config_(config) {
    config_.validate();
    sets_.assign(config_.cache_set_count(), std::vector<Line>(config_.cache_associativity));
}

CacheAccessResult Cache::access(const std::uint64_t address) {
    ++timestamp_;
    ++accesses_;

    const auto line_address = address / config_.cache_line_size;
    const auto set_index = static_cast<std::size_t>(line_address % config_.cache_set_count());
    const auto tag = line_address / config_.cache_set_count();

    auto& set = sets_.at(set_index);
    for (std::size_t way = 0; way < set.size(); ++way) {
        auto& line = set[way];
        if (line.valid && line.tag == tag) {
            line.last_used = timestamp_;
            ++hits_;
            return CacheAccessResult{
                .hit = true,
                .latency = config_.cache_hit_latency,
                .set_index = set_index,
                .way_index = way,
            };
        }
    }

    ++misses_;

    std::size_t replacement_way = 0;
    std::uint64_t oldest_timestamp = std::numeric_limits<std::uint64_t>::max();

    for (std::size_t way = 0; way < set.size(); ++way) {
        if (!set[way].valid) {
            replacement_way = way;
            oldest_timestamp = 0;
            break;
        }
        if (set[way].last_used < oldest_timestamp) {
            oldest_timestamp = set[way].last_used;
            replacement_way = way;
        }
    }

    auto& replacement = set[replacement_way];
    replacement.valid = true;
    replacement.tag = tag;
    replacement.last_used = timestamp_;

    return CacheAccessResult{
        .hit = false,
        .latency = config_.cache_hit_latency + config_.cache_miss_penalty,
        .set_index = set_index,
        .way_index = replacement_way,
    };
}

std::uint64_t Cache::accesses() const {
    return accesses_;
}

std::uint64_t Cache::hits() const {
    return hits_;
}

std::uint64_t Cache::misses() const {
    return misses_;
}

std::size_t Cache::set_count() const {
    return sets_.size();
}

}  // namespace sim
