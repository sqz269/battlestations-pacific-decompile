#include "bsp/native_input_conflict_vector_tidy.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This reconstruction requires MSVC Win32 raw DWORD accesses.
#endif

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));

void __fastcall destroy_native_input_conflict_vector_header_range_0069eea0(
    void* first, void* last, std::uint32_t, std::uint32_t) {
    std::uint32_t current = reinterpret_cast<std::uint32_t>(first);
    const std::uint32_t captured_last = reinterpret_cast<std::uint32_t>(last);
    while (current != captured_last) {
        std::uint32_t captured_begin;
        __asm {
            mov eax, current
            mov eax, dword ptr [eax + 4]
            mov captured_begin, eax
        }
        if (captured_begin != 0u) {
            singleton_lifetime_free(reinterpret_cast<void*>(captured_begin));
        }
        __asm {
            mov eax, current
            xor edx, edx
            mov dword ptr [eax + 4], edx
            mov dword ptr [eax + 8], edx
            mov dword ptr [eax + 0ch], edx
        }
        current += 0x10u;
    }
}

void __fastcall tidy_native_input_conflict_vector_006a1420(
    void* header, std::uint32_t) {
    const std::uint32_t actual_header = reinterpret_cast<std::uint32_t>(header);
    std::uint32_t captured_begin;
    __asm {
        mov eax, actual_header
        mov eax, dword ptr [eax + 4]
        mov captured_begin, eax
    }
    if (captured_begin != 0u) {
        std::uint32_t captured_end;
        __asm {
            mov eax, actual_header
            mov eax, dword ptr [eax + 8]
            mov captured_end, eax
        }
        // Native pushes (header, saved incoming-ECX/header bits). Both
        // metadata words are ignored; the second is normalized to 0.
        destroy_native_input_conflict_vector_header_range_0069eea0(
            reinterpret_cast<void*>(captured_begin),
            reinterpret_cast<void*>(captured_end), actual_header, 0u);
        std::uint32_t current_begin;
        __asm {
            mov eax, actual_header
            mov eax, dword ptr [eax + 4]
            mov current_begin, eax
        }
        // Native makes this free unconditionally in the initial-nonnull arm.
        singleton_lifetime_free(reinterpret_cast<void*>(current_begin));
    }
    __asm {
        mov eax, actual_header
        xor edx, edx
        mov dword ptr [eax + 4], edx
        mov dword ptr [eax + 8], edx
        mov dword ptr [eax + 0ch], edx
    }
}
} // namespace bsp
