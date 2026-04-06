#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

std::vector<TestCase>& tests() {
    static std::vector<TestCase> t;
    return t;
}

struct Register {
    Register(const std::string& name, std::function<void()> fn) { tests().push_back({name, std::move(fn)}); }
};

#define TEST(name) \
    void name();    \
    static Register reg_##name(#name, name); \
    void name()

#define ASSERT_TRUE(x)                                                                              \
    do {                                                                                            \
        if (!(x)) {                                                                                 \
            throw std::runtime_error(std::string("assertion failed: ") + #x + " line=" + std::to_string(__LINE__)); \
        }                                                                                           \
    } while (0)

#include "test_selection.cpp"
#include "test_priority_store.cpp"
#include "test_client.cpp"

int main() {
    std::size_t failed = 0;
    for (const auto& test : tests()) {
        try {
            test.fn();
            std::cout << "PASS " << test.name << '\n';
        } catch (const std::exception& ex) {
            ++failed;
            std::cerr << "FAIL " << test.name << " - " << ex.what() << '\n';
        }
    }

    if (failed > 0) {
        std::cerr << failed << " tests failed\n";
        return 1;
    }
    std::cout << "All tests passed: " << tests().size() << '\n';
    return 0;
}
