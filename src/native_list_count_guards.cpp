#include "bsp/native_list_count_guards.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {

void increment_list_count(void* actual_list_header, std::uint32_t addition) {
    static_assert(sizeof(void*) == 4);
    auto* const count_address = static_cast<char*>(actual_list_header) + 8;
    std::uint32_t current;
    std::memcpy(&current, count_address, sizeof(current));

    // The subtraction wraps even when the prestate already exceeds the limit.
    // A separate current>limit check would change native accepted states.
    const std::uint32_t available = std::uint32_t{0x3fffffff} - current;
    if (available < addition) {
        throw std::length_error("list<T> too long");
    }

    const std::uint32_t result = current + addition;
    std::memcpy(count_address, &result, sizeof(result));
}

} // namespace

void __fastcall increment_native_list_count_00420140(
    void* actual_list_header, void*, std::uint32_t addition) {
    increment_list_count(actual_list_header, addition);
}

void __fastcall increment_native_list_count_0041fdd0(
    void* actual_list_header, void*, std::uint32_t addition) {
    increment_list_count(actual_list_header, addition);
}

} // namespace bsp
