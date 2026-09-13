#include "bsp/native_ambient_construction.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4, "Native ambient storage requires Win32");

void* construct_native_ambient_00b7c290(
    void* owner, const void* one_00d7a24c) noexcept {
    float spill[4];
    // Retain the actual MOVSS/DWORD roundtrips and nonascending tail stores.
    // These are bit copies: neither NaNs nor the borrowed constant normalize.
    __asm {
        xorps xmm0, xmm0
        mov eax, owner
        mov edx, one_00d7a24c
        movss xmm1, dword ptr [edx]
        xor ecx, ecx
        mov dword ptr [eax], 0ceb130h
        mov dword ptr [eax + 4], 1
        mov dword ptr [eax], 0d62f3ch
        mov dword ptr [eax + 8], ecx
        mov dword ptr [eax + 0ch], ecx
        mov dword ptr [eax + 10h], ecx
        movss dword ptr [eax + 14h], xmm1
        movss dword ptr [eax + 18h], xmm0
        movss dword ptr [eax + 1ch], xmm0
        movss dword ptr [eax + 20h], xmm0
        movss dword ptr [eax + 24h], xmm1
        movss dword ptr [spill], xmm0
        mov ecx, dword ptr [spill]
        mov dword ptr [eax + 38h], ecx
        movss dword ptr [spill + 4], xmm0
        mov edx, dword ptr [spill + 4]
        mov dword ptr [eax + 3ch], edx
        movss dword ptr [spill + 8], xmm0
        mov ecx, dword ptr [spill + 8]
        mov dword ptr [eax + 40h], ecx
        movss dword ptr [spill + 0ch], xmm1
        mov edx, dword ptr [spill + 0ch]
        mov dword ptr [eax + 44h], edx
        movss dword ptr [spill], xmm0
        mov ecx, dword ptr [spill]
        mov dword ptr [eax + 48h], ecx
        movss dword ptr [spill + 4], xmm0
        mov edx, dword ptr [spill + 4]
        mov dword ptr [eax + 4ch], edx
        movss dword ptr [spill + 8], xmm0
        mov ecx, dword ptr [spill + 8]
        mov dword ptr [eax + 50h], ecx
        movss dword ptr [spill + 0ch], xmm1
        mov edx, dword ptr [spill + 0ch]
        mov dword ptr [eax + 54h], edx
        movss dword ptr [spill], xmm0
        mov ecx, dword ptr [spill]
        mov dword ptr [eax + 58h], ecx
        movss dword ptr [spill + 4], xmm0
        mov edx, dword ptr [spill + 4]
        mov dword ptr [eax + 5ch], edx
        movss dword ptr [spill + 8], xmm0
        mov ecx, dword ptr [spill + 8]
        mov dword ptr [eax + 60h], ecx
        movss dword ptr [spill + 0ch], xmm1
        mov edx, dword ptr [spill + 0ch]
        mov dword ptr [eax + 64h], edx
        movss dword ptr [spill], xmm0
        mov ecx, dword ptr [spill]
        mov dword ptr [eax + 68h], ecx
        movss dword ptr [spill + 4], xmm0
        mov edx, dword ptr [spill + 4]
        mov dword ptr [eax + 6ch], edx
        movss dword ptr [spill + 8], xmm0
        mov ecx, dword ptr [spill + 8]
        mov dword ptr [eax + 70h], ecx
        movss dword ptr [spill + 0ch], xmm1
        mov edx, dword ptr [spill + 0ch]
        movss dword ptr [spill], xmm0
        mov ecx, dword ptr [spill]
        mov dword ptr [eax + 78h], ecx
        mov dword ptr [eax + 74h], edx
        movss dword ptr [spill + 4], xmm0
        mov edx, dword ptr [spill + 4]
        movss dword ptr [spill + 8], xmm0
        mov ecx, dword ptr [spill + 8]
        mov dword ptr [eax + 7ch], edx
        movss dword ptr [spill + 0ch], xmm1
        mov edx, dword ptr [spill + 0ch]
        mov dword ptr [eax + 80h], ecx
        movss dword ptr [spill], xmm0
        mov ecx, dword ptr [spill]
        mov dword ptr [eax + 84h], edx
        movss dword ptr [spill + 4], xmm0
        movss dword ptr [spill + 8], xmm0
        movss dword ptr [spill + 0ch], xmm1
        mov dword ptr [eax + 88h], ecx
        mov edx, dword ptr [spill + 4]
        mov ecx, dword ptr [spill + 8]
        mov dword ptr [eax + 8ch], edx
        mov edx, dword ptr [spill + 0ch]
        mov dword ptr [eax + 90h], ecx
        mov dword ptr [eax + 94h], edx
    }
    return owner;
}

} // namespace bsp
