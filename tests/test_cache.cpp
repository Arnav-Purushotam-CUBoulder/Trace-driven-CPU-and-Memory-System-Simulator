#include "cache.hpp"
#include "test_framework.hpp"

TEST_CASE("direct_mapped_cache_tracks_hits_and_conflict_misses") {
    sim::SimulatorConfig config;
    config.cache_size_bytes = 128;
    config.cache_line_size = 16;
    config.cache_associativity = 1;
    config.cache_hit_latency = 1;
    config.cache_miss_penalty = 10;

    sim::Cache cache(config);

    const auto first = cache.access(0x0);
    const auto second = cache.access(0x8);
    const auto conflict = cache.access(0x80);
    const auto eviction = cache.access(0x0);

    EXPECT_TRUE(!first.hit);
    EXPECT_EQ(first.latency, 11ULL);
    EXPECT_TRUE(second.hit);
    EXPECT_TRUE(!conflict.hit);
    EXPECT_TRUE(!eviction.hit);
    EXPECT_EQ(cache.accesses(), 4ULL);
    EXPECT_EQ(cache.hits(), 1ULL);
    EXPECT_EQ(cache.misses(), 3ULL);
}
