#pragma once

#include <cmath>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace test {

using TestFunction = void (*)();

struct TestCase {
    std::string name;
    TestFunction function;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(std::string name, TestFunction function) {
        registry().push_back(TestCase{std::move(name), function});
    }
};

class AssertionFailure : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

inline void fail(std::string message) {
    throw AssertionFailure(std::move(message));
}

inline void expect_true(
    const bool condition,
    const std::string_view expression,
    const std::string_view file,
    const int line
) {
    if (!condition) {
        std::ostringstream output;
        output << file << ':' << line << " expected true: " << expression;
        fail(output.str());
    }
}

template <typename Left, typename Right>
void expect_equal(
    const Left& left,
    const Right& right,
    const std::string_view left_expression,
    const std::string_view right_expression,
    const std::string_view file,
    const int line
) {
    if (!(left == right)) {
        std::ostringstream output;
        output << file << ':' << line << " expected " << left_expression << " == "
               << right_expression << " (" << left << " vs " << right << ')';
        fail(output.str());
    }
}

inline void expect_near(
    const double left,
    const double right,
    const double epsilon,
    const std::string_view left_expression,
    const std::string_view right_expression,
    const std::string_view file,
    const int line
) {
    if (std::fabs(left - right) > epsilon) {
        std::ostringstream output;
        output << file << ':' << line << " expected " << left_expression << " ~= "
               << right_expression << " (" << left << " vs " << right << ')';
        fail(output.str());
    }
}

}  // namespace test

#define TEST_CONCAT_IMPL(left, right) left##right
#define TEST_CONCAT(left, right) TEST_CONCAT_IMPL(left, right)

#define TEST_CASE(name)                                                           \
    static void TEST_CONCAT(test_case_, __LINE__)();                              \
    static ::test::Registrar TEST_CONCAT(registrar_, __LINE__)(                   \
        name,                                                                     \
        &TEST_CONCAT(test_case_, __LINE__)                                        \
    );                                                                            \
    static void TEST_CONCAT(test_case_, __LINE__)()

#define EXPECT_TRUE(expression) \
    ::test::expect_true((expression), #expression, __FILE__, __LINE__)

#define EXPECT_EQ(left, right) \
    ::test::expect_equal((left), (right), #left, #right, __FILE__, __LINE__)

#define EXPECT_NEAR(left, right, epsilon) \
    ::test::expect_near((left), (right), (epsilon), #left, #right, __FILE__, __LINE__)
