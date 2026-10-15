#include "bsp/native_string_copy_construct.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native string scalar construction requires MSVC Win32 x87 operations.
#endif

#include "bsp/native_pooled_string_substring.hpp"

namespace bsp {
namespace {

// Do not replace this pair with a DWORD copy or a C++ float assignment: native
// FLD/FSTP observes the caller's x87 state and can quiet a signaling NaN. There
// is no extra FWAIT, control-word change, or exception translation here.
void copy_scalar_x87(void* output, const void* input) noexcept {
    __asm {
        mov eax, input
        mov edx, output
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}

} // namespace

void* copy_construct_native_string_with_scalar_00444070(void* actual_destination,
    const void* actual_source_header, const void* actual_scalar,
    NativeStringRawPoolContext& context) {
    // 00444070..004440AA matches the complete 00426060 header body, including
    // the post-resize reads and the concrete pool/overlap-safe copy boundary.
    copy_construct_native_string_header_00426060(actual_destination,
        actual_source_header, context);
    copy_scalar_x87(static_cast<char*>(actual_destination) + 8, actual_scalar);
    return actual_destination;
}

void* copy_construct_native_string_with_scalar_004440c0(void* actual_destination,
    const void* actual_source, NativeStringRawPoolContext& context) {
    // The scalar address may be formed early, but its contents must be read
    // only after the header construction and all of its allocation/return work.
    copy_construct_native_string_header_00426060(actual_destination,
        actual_source, context);
    copy_scalar_x87(static_cast<char*>(actual_destination) + 8,
        static_cast<const char*>(actual_source) + 8);
    return actual_destination;
}

} // namespace bsp
