#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "test_framework.hpp"

int main() {
    std::vector<std::string> failures;
    std::size_t passed = 0;

    for (const auto& test_case : test::registry()) {
        try {
            test_case.function();
            ++passed;
            std::cout << "[PASS] " << test_case.name << '\n';
        } catch (const std::exception& error) {
            failures.push_back(test_case.name + ": " + error.what());
            std::cerr << "[FAIL] " << test_case.name << ": " << error.what() << '\n';
        }
    }

    std::cout << passed << " / " << test::registry().size() << " tests passed\n";
    if (!failures.empty()) {
        return 1;
    }
    return 0;
}
