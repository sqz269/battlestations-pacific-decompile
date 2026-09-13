#include "bsp/native_renderer_frame_statistics.hpp"

#include <cstdint>

namespace bsp {

static_assert(sizeof(void*) == 4, "Native renderer storage requires Win32");

void clear_native_renderer_frame_statistics_bank_00b0cc10(void* bank) noexcept {
    // Explicit DWORD writes retain the two nonascending portions of the leaf.
    __asm {
        mov ecx, bank
        xor eax, eax
        mov dword ptr [ecx], eax
        mov dword ptr [ecx + 04h], eax
        mov dword ptr [ecx + 08h], eax
        mov dword ptr [ecx + 0ch], eax
        mov dword ptr [ecx + 10h], eax
        mov dword ptr [ecx + 14h], eax
        mov dword ptr [ecx + 18h], eax
        mov dword ptr [ecx + 1ch], eax
        mov dword ptr [ecx + 20h], eax
        mov dword ptr [ecx + 24h], eax
        mov dword ptr [ecx + 28h], eax
        mov dword ptr [ecx + 2ch], eax
        mov dword ptr [ecx + 30h], eax
        mov dword ptr [ecx + 34h], eax
        mov dword ptr [ecx + 38h], eax
        mov dword ptr [ecx + 3ch], eax
        mov dword ptr [ecx + 40h], eax
        mov dword ptr [ecx + 4ch], eax
        mov dword ptr [ecx + 44h], eax
        mov dword ptr [ecx + 48h], eax
        mov dword ptr [ecx + 50h], eax
        mov dword ptr [ecx + 54h], eax
        mov dword ptr [ecx + 58h], eax
        mov dword ptr [ecx + 60h], eax
        mov dword ptr [ecx + 64h], eax
        mov dword ptr [ecx + 68h], eax
        mov dword ptr [ecx + 6ch], eax
        mov dword ptr [ecx + 5ch], eax
        mov dword ptr [ecx + 70h], eax
        mov dword ptr [ecx + 74h], eax
        mov dword ptr [ecx + 78h], eax
        mov dword ptr [ecx + 7ch], eax
        mov dword ptr [ecx + 80h], eax
        mov dword ptr [ecx + 84h], eax
        mov dword ptr [ecx + 88h], eax
        mov dword ptr [ecx + 8ch], eax
        mov dword ptr [ecx + 90h], eax
        mov dword ptr [ecx + 94h], eax
        mov dword ptr [ecx + 98h], eax
        mov dword ptr [ecx + 9ch], eax
    }
}

void publish_native_renderer_frame_statistics_00b0ccb0(void* bank_pair) noexcept {
    // The compiler preserves ESI/EDI for this new C++ interface. As in native
    // B0CCC1, no CLD is added and the entire forward copy precedes the clear.
    __asm {
        mov eax, bank_pair
        lea edi, [eax + 0a0h]
        mov ecx, 28h
        mov esi, eax
        rep movsd
    }
    clear_native_renderer_frame_statistics_bank_00b0cc10(bank_pair);
}

void* construct_native_renderer_frame_statistics_00b0cce0(void* storage) noexcept {
    // Retain the receiver as a C++ local across calls; unlike the native leaf,
    // an arbitrary C++ callee is not required to preserve EDX.
    clear_native_renderer_frame_statistics_bank_00b0cc10(storage);
    clear_native_renderer_frame_statistics_bank_00b0cc10(
        static_cast<std::uint8_t*>(storage) + 0xa0);
    __asm {
        mov edx, storage
        xor eax, eax
        mov dword ptr [edx + 140h], eax
        mov dword ptr [edx + 144h], eax
        mov dword ptr [edx + 150h], eax
        mov dword ptr [edx + 154h], eax
        mov dword ptr [edx + 158h], eax
        mov dword ptr [edx + 15ch], eax
        mov dword ptr [edx + 160h], eax
        mov dword ptr [edx + 164h], eax
        mov dword ptr [edx + 168h], eax
        mov dword ptr [edx + 16ch], eax
        mov dword ptr [edx + 170h], eax
        mov dword ptr [edx + 174h], eax
        mov dword ptr [edx + 178h], eax
        mov dword ptr [edx + 148h], eax
        mov dword ptr [edx + 14ch], eax
    }
    return storage;
}

} // namespace bsp
