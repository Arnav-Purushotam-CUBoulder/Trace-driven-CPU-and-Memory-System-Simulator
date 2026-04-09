#include <filesystem>
#include <fstream>

#include "test_framework.hpp"
#include "trace_reader.hpp"

TEST_CASE("trace_reader_parses_basic_trace_format") {
    const auto path =
        std::filesystem::temp_directory_path() / "trace_simulator_reader_test.trace";

    {
        std::ofstream output(path);
        output << "# demo trace\n"
               << "ALU 0x1000\n"
               << "LOAD 0x1004 0x2000\n"
               << "STORE 0x1008 8192\n"
               << "BRANCH 0x100c T\n"
               << "BRANCH 0x1010 N\n";
    }

    const auto trace = sim::read_trace(path);
    std::filesystem::remove(path);

    EXPECT_EQ(trace.size(), 5ULL);
    EXPECT_EQ(trace[0].sequence, 0ULL);
    EXPECT_TRUE(trace[1].type == sim::InstructionType::Load);
    EXPECT_EQ(*trace[1].address, 0x2000ULL);
    EXPECT_TRUE(trace[2].type == sim::InstructionType::Store);
    EXPECT_EQ(*trace[2].address, 8192ULL);
    EXPECT_TRUE(trace[3].type == sim::InstructionType::Branch);
    EXPECT_TRUE(*trace[3].branch_taken);
    EXPECT_TRUE(!*trace[4].branch_taken);
}
