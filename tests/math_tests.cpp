#include "bsp/math.hpp"
#include "bsp/native_string.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

namespace {
int failures = 0;
void check(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
bool equal(const bsp::Vec3d& a, const bsp::Vec3d& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

// A release that repeats the wrong size puts the block on the wrong free list
// of the native pool, so the sizes 0041dd40 passes are the risk worth pinning.
class RecordingStorage final : public bsp::NativeStringStorage {
public:
    char* allocate(std::uint32_t size) override {
        allocated.push_back(size);
        return new char[size];
    }
    void release(char* block, std::uint32_t size) noexcept override {
        released.push_back(size);
        delete[] block;
    }
    std::vector<std::uint32_t> allocated;
    std::vector<std::uint32_t> released;
};
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

    RecordingStorage storage;
    NativeString text;
    text.assign_0041e870(storage, "abcd");
    check(text.length() == 4 && std::strcmp(text.data(), "abcd") == 0, "assign copies the terminator");
    text.resize_0041dd40(storage, 2, true);
    check(text.length() == 2 && text.data()[0] == 'a' && text.data()[1] == 'b',
        "shrink preserves min(old, new) bytes");
    check(text.data()[2] == '\0', "resize writes the terminator at the new length");
    text.resize_0041dd40(storage, 2, true);
    text.release_to(storage);
    const std::vector<std::uint32_t> expected_allocated{5, 3};
    const std::vector<std::uint32_t> expected_released{5, 3};
    check(storage.allocated == expected_allocated, "each buffer is length + 1 bytes");
    check(storage.released == expected_released, "each release repeats the allocated size");

    if (!failures) std::cout << "Reconstructed math semantic tests passed (not binary equivalence).\n";
    return failures ? 1 : 0;
}
