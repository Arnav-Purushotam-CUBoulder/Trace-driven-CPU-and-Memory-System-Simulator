#include "branch_predictor.hpp"
#include "test_framework.hpp"

TEST_CASE("one_bit_predictor_flips_on_update") {
    sim::SimulatorConfig config;
    config.predictor_kind = sim::BranchPredictorKind::OneBit;
    config.branch_predictor_entries = 8;

    sim::BranchPredictor predictor(config);

    EXPECT_TRUE(!predictor.predict(0x1000));
    predictor.update(0x1000, true);
    EXPECT_TRUE(predictor.predict(0x1000));
    predictor.update(0x1000, false);
    EXPECT_TRUE(!predictor.predict(0x1000));
}

TEST_CASE("two_bit_predictor_requires_sustained_direction_change") {
    sim::SimulatorConfig config;
    config.predictor_kind = sim::BranchPredictorKind::TwoBit;
    config.branch_predictor_entries = 4;

    sim::BranchPredictor predictor(config);

    EXPECT_TRUE(!predictor.predict(0x2000));
    predictor.update(0x2000, true);
    EXPECT_TRUE(predictor.predict(0x2000));
    predictor.update(0x2000, false);
    EXPECT_TRUE(!predictor.predict(0x2000));
}
