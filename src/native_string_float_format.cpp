#define _CRT_SECURE_NO_WARNINGS
#include "bsp/native_string_float_format.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstdio>
#include <cstring>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native float string construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(NativeString) == 8 && alignof(NativeString) == 4);
static_assert(std::is_nothrow_default_constructible_v<NativeString>);
static_assert(std::is_trivially_destructible_v<NativeString>);
static_assert(sizeof(void*) == 4 && sizeof(float) == 4 && sizeof(double) == 8);

template<class T> T read_field(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(header) + offset, sizeof(value));
    return value;
}

// A concrete CURRENT-CRT service. char* prevents secure array-overload
// selection; no count, forced locale, clipping or error replacement is added.
void format_current_crt(char* scratch, double widened) {
    (void)std::sprintf(scratch, "%f", widened);
}

void return_captured(char* data, std::uint32_t length,
    NativeStringRawPoolContext& strings) {
    const auto bytes = length + 1u;
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,
        strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, bytes,
        strings.actual_small_returns_disabled_01090aa4);
}
} // namespace

void* construct_native_string_float_header_0043bdf0(void* actual_fresh_header,
    NativeStringRawPoolContext& strings, float value) {
    double widened;
    // Original43BE08/13: load the actual float32 and materialize its double
    // promotion BEFORE the output stores, never forward the original Lua double.
    __asm {
        fld value
        fstp widened
        mov eax, actual_fresh_header
        xor edx, edx
        mov dword ptr [eax], edx
        mov dword ptr [eax + 4], edx
    }
    //52B is the observed prefix up to the next Original saved-FS frame object.
    // The result including NUL must fit. The Original declared extent is unknown.
    char scratch[52];
    format_current_crt(static_cast<char*>(scratch), widened);

    // Construction and first copy are BEFORE the sole completed-temp guard.
    // NativeString is genuine8B storage with a trivial, non-owning destructor.
    NativeString temporary;
    const auto length = static_cast<std::uint32_t>(std::strlen(scratch));
    resize_native_string_header_0041dd40(&temporary, strings, length, true);
    char* const captured_data = read_field<char*>(&temporary, 4);
    const auto captured_length = read_field<std::uint32_t>(&temporary, 0);
    if (captured_data != nullptr)
        std::memmove(captured_data, scratch, captured_length + 1u);

    // This scope represents Original state0. It owns only temporary cleanup,
    // including current fields changed during output work; never the output.
    try {
        if (actual_fresh_header != static_cast<void*>(&temporary)) {
            resize_native_string_header_0041dd40(actual_fresh_header, strings,
                captured_length, true);
            if (captured_length != 0) {
                const auto current_length = read_field<std::uint32_t>(actual_fresh_header, 0);
                void* const current_data = read_field<void*>(actual_fresh_header, 4);
                // Preserve the established omission of native zero-byte copies.
                if (current_length != 0)
                    std::memmove(current_data, captured_data, current_length);
            }
        }
    } catch (...) {
        destroy_native_string_header_0041dd20(&temporary, strings);
        throw;
    }
    // Original state-1 precedes this CURRENT getter/captured-sized return.
    // A failure here cannot trigger a second temporary cleanup.
    if (captured_data != nullptr)
        return_captured(captured_data, captured_length, strings);
    return actual_fresh_header;
}
} // namespace bsp
