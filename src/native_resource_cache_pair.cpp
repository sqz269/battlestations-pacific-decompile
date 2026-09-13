#include "bsp/native_resource_cache_pair.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);

U word(const void* object, U offset = 0) noexcept {
    const auto address = reinterpret_cast<U>(object) + offset;
    return *reinterpret_cast<const volatile U*>(address);
}
void put(void* object, U offset, U value) noexcept {
    const auto address = reinterpret_cast<U>(object) + offset;
    *reinterpret_cast<volatile U*>(address) = value;
}
} // namespace

void* construct_native_resource_cache_pair_00b7f290(void* destination,
    void* resource, void* input_header, NativeStringRawPoolContext& strings) {
    // B7F2B0/B7F2BA/B7F2BE: these values precede the first destination write.
    void* const captured_resource = resource;
    char* const input_data = reinterpret_cast<char*>(word(input_header, 4));
    const U input_length = word(input_header);
    put(destination, 0, 0);
    put(destination, 4, 0);

    bool completed_pair = false;
    bool input_cleanup_armed = true; // native EH state 1, B7F2CD
    try {
        if (destination != input_header) {
            resize_native_string_header_0041dd40(
                destination, strings, input_length, true);
            if (input_length != 0) {
                const U length = word(destination);
                void* const output = reinterpret_cast<void*>(word(destination, 4));
                if (length != 0) std::memmove(output, input_data, length);
            }
        }
        put(destination, 8, reinterpret_cast<U>(captured_resource));
        completed_pair = true; // local flag at B7F2F8
        input_cleanup_armed = false; // state 0 BEFORE return, B7F300
        if (input_data != nullptr) {
            const U size = input_length + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8,
                strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, input_data, size,
                strings.actual_small_returns_disabled_01090aa4);
        }
    } catch (...) {
        // DFB1B8 state 1 -> CC2030: current by-value input header, not the
        // captured fields used by the normal return above.
        if (input_cleanup_armed)
            destroy_native_string_header_0041dd20(input_header, strings);
        // State 0 -> CC2038 clears the completed flag before B7E8F0, whose
        // actual name-return schedule is the complete raw 41DD20 contract.
        if (completed_pair) {
            completed_pair = false;
            destroy_native_string_header_0041dd20(destination, strings);
        }
        throw;
    }
    return destination;
}

} // namespace bsp
