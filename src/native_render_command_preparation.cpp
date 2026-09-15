#include "bsp/native_render_command_preparation.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render command preparation requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t current_word(const void* actual, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const std::byte*>(actual) + offset);
}
} // namespace

__declspec(naked) void __fastcall set_native_render_command_metadata_00b1bf50(
    void*, void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4] // 00b1bf50
        mov edx, dword ptr [eax] // 00b1bf54
        mov dword ptr [ecx + 0x1c], edx // 00b1bf56
        mov edx, dword ptr [eax + 4] // 00b1bf59
        mov dword ptr [ecx + 0x20], edx // 00b1bf5c
        mov eax, dword ptr [eax + 8] // 00b1bf5f
        mov dword ptr [ecx + 0x24], eax // 00b1bf62
        ret 4 // 00b1bf65
    }
}

void set_native_render_command_diagnostic_00b1d910(void* actual_command,
    const void* actual_source_header, NativeStringRawPoolContext& actual_pool) {
    auto* const destination = static_cast<std::byte*>(actual_command) + 0x14;
    if (destination == actual_source_header) return; // B1D919/B1D91B

    const auto requested = current_word(actual_source_header, 0); // B1D91D
    resize_native_string_header_0041dd40(destination, actual_pool, requested,
        true); // B1D924: captured length, preserve1
    if (current_word(actual_source_header, 0) == 0) return; // B1D929/B1D92C

    const auto count = current_word(destination, 0); // B1D92E
    const auto source_data = current_word(actual_source_header, 4); // B1D930
    const auto destination_data = current_word(destination, 4); // B1D933
    // BF7680 handles backward overlap. Keep all argument reads even when
    // count becomes zero; omit only the standard-library zero-byte call.
    if (count != 0) {
        std::memmove(reinterpret_cast<void*>(destination_data),
            reinterpret_cast<const void*>(source_data), count); // B1D939
    }
}
} // namespace bsp
