#include "bsp/native_crt_secure_memmove.hpp"
#include "bsp/native_crt_memmove.hpp"
#include "bsp/native_crt_stricmp.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT secure memmove requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

std::int32_t __cdecl move_native_crt_secure_bytes_00bf67a7(
    void* destination, std::uint32_t destination_bytes, const void* source,
    std::uint32_t byte_count,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    NativeCrtStricmpCallees& actual_error_callees) {
    // Native67AA captures count before validation. Native argument-slot reloads
    // become value arguments here; original private stack/frame aliases are not
    // part of this new interface. No feature value is copied or sampled here.
    const std::uint32_t captured_count = byte_count;
    if (captured_count == 0U)
        return 0;

    if (destination == nullptr || source == nullptr) {
        volatile std::int32_t* const actual_errno =
            actual_error_callees.errno_cell_00bffb8b();
        const std::int32_t invalid_argument = 0x16;
        *actual_errno = invalid_argument;
        actual_error_callees.invalid_parameter_00bf66ef(0, 0, 0, 0, 0);
        return invalid_argument;
    }

    if (destination_bytes < captured_count) {
        volatile std::int32_t* const actual_errno =
            actual_error_callees.errno_cell_00bffb8b();
        const std::int32_t insufficient_capacity = 0x22;
        *actual_errno = insufficient_capacity;
        actual_error_callees.invalid_parameter_00bf66ef(0, 0, 0, 0, 0);
        return insufficient_capacity;
    }

    (void)move_native_crt_bytes_00bf87e0(destination, source, captured_count,
        actual_feature_word_0109eea4);
    return 0;
}
} // namespace bsp
