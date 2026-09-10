#include "bsp/app_bootstrap.hpp"
#include "bsp/math.hpp"
#include <cmath>
#include <iostream>
#include <limits>

namespace {
int failures = 0;
void check(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
bool equal(const bsp::Vec3d& a, const bsp::Vec3d& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}
}

int main() {
    using namespace bsp;
    check(abs_00401170(-3.5f) == 3.5f, "absolute value");
    check(!std::signbit(abs_00401170(-0.0f)), "negative zero becomes positive");
    check(std::isinf(abs_00401170(-std::numeric_limits<float>::infinity())), "infinity");
    check(std::isnan(abs_00401170(std::numeric_limits<float>::quiet_NaN())), "quiet NaN");
    Vec3d out{};
    cross_reversed_00401c20(out, {1, 0, 0}, {0, 1, 0});
    check(equal(out, {0, 0, -1}), "cross product handedness is b cross a");
    cross_reversed_00401c20(out, {1, 2, 3}, {4, 5, 6});
    check(equal(out, {3, -6, 3}), "cross product fixture");
    Vec3d alias{1, 2, 3};
    cross_reversed_00401c20(alias, alias, {4, 5, 6});
    check(equal(alias, {3, 6, 9}), "original sequential writes under aliasing");
    subtract_reversed_00401cb0(out, {1, 2, 3}, {4, 8, 12});
    check(equal(out, {3, 6, 9}), "subtraction operand order");
    subtract_reversed_00401cb0(out, out, {4, 8, 12});
    check(equal(out, {1, 2, 3}), "subtraction aliasing");
    scale_00401cd0(out, -2);
    check(equal(out, {-2, -4, -6}), "in-place scaling");
    check(length_squared_00401cf0({3, 4, 12}) == 169, "squared length fixture");
    check(length_squared_00401cf0({0, 0, 0}) == 0, "zero squared length");
    // 0073ce20: "auto" advances the token iterator itself, so the two tokens
    // that follow it are consumed and must not be classified on their own.
    const CommandLineOptions parsed =
        parse_command_line_0073ce20("nozip auto mpak classes memlimit");
    check(!parsed.zip_enabled, "nozip clears the zip flag");
    check(parsed.auto_task == AutoTask::package_classes, "auto mpak classes");
    check(parsed.file_access_log, "auto mpak also raises the file access log");
    check(parsed.memory_limit.has_value() && *parsed.memory_limit,
        "the token after the auto group is still parsed");
    check(parsed.unrecognized.empty(), "auto follower tokens are not left over");
    if (!failures) std::cout << "Reconstructed math semantic tests passed (not binary equivalence).\n";
    return failures ? 1 : 0;
}
