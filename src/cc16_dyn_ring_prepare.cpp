#include "bsp/cc16_dyn_ring_prepare.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered ambient-x87 body requires MSVC Win32.
#endif

namespace bsp::cc16_dyn_ring_prepare {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4, "The recovered layout requires Win32 pointers.");

U32 address(const void* value) noexcept { return reinterpret_cast<U32>(value); }
void* pointer(U32 value) noexcept { return reinterpret_cast<void*>(value); }
volatile U32& word(const void* base, U32 offset = 0) noexcept {
    return *static_cast<volatile U32*>(pointer(address(base) + offset));
}

// Keep the native memory/FLD ordering and all four FSTP-float epochs. The
// multiply/subtract result stays in x87 until FCOMIP; no control changes,
// fixed-precision emulation, SSE replacement or extra determinant spill.
bool orientation_above_zero(void* owner, U32 next_data, U32 anchor,
    U32& previous) noexcept {
    U32 next_x, next_y, anchor_x, anchor_y, previous_x, previous_y;
    U32 previous_index = 0;
    float anchor_dx, anchor_dy, next_dx, next_dy;
    std::uint8_t above = 0;
    __asm {
        mov esi, owner
        mov edi, anchor
        mov ebx, next_data
        mov eax, dword ptr [esi + 08h]
        mov eax, dword ptr [eax]
        mov ecx, dword ptr [ebx + edi * 4]
        mov edx, dword ptr [eax + ecx * 8]
        mov ecx, dword ptr [eax + ecx * 8 + 4]
        mov next_x, edx
        mov edx, dword ptr [eax + edi * 8]
        mov next_y, ecx
        mov ecx, dword ptr [eax + edi * 8 + 4]
        mov anchor_x, edx
        fld dword ptr anchor_x
        mov edx, dword ptr [esi + 18h]
        mov anchor_y, ecx
        mov ecx, dword ptr [edx + edi * 4]
        mov previous_index, ecx
        mov ebx, dword ptr [eax + ecx * 8]
        mov eax, dword ptr [eax + ecx * 8 + 4]
        mov previous_x, ebx
        fld dword ptr previous_x
        fld st(0)
        mov previous_y, eax
        fsubp st(2), st(0)
        fxch st(1)
        fstp dword ptr anchor_dx
        fld dword ptr anchor_y
        fld dword ptr previous_y
        fld st(0)
        fsubp st(2), st(0)
        fxch st(1)
        fstp dword ptr anchor_dy
        fld dword ptr next_x
        fsubrp st(2), st(0)
        fxch st(1)
        fstp dword ptr next_dx
        fsubr dword ptr next_y
        fstp dword ptr next_dy
        fld dword ptr next_dy
        fmul dword ptr anchor_dx
        fld dword ptr anchor_dy
        fmul dword ptr next_dx
        fsubp st(1), st(0)
        fldz
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        seta above
    }
    previous = previous_index;
    return above != 0;
}

// The three native append expansions differ only in their final value offset.
// Read that value after growth/free and only if the destination is nonzero.
void append_scratch(void* owner, U32 value_offset,
    const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 capacity = word(owner, 0x2c);
    if (word(owner, 0x28) == capacity) {
        const U32 next_capacity = capacity * 2u + 2u;
        word(owner, 0x2c) = next_capacity;
        void* grown = crt.operator_new_00bf55be(next_capacity * 4u);
        U32 index = 0;
        U32 output = address(grown);
        while (index < word(owner, 0x28)) {
            if (output != 0) {
                const U32 input = word(owner, 0x24) + index * 4u;
                word(pointer(output)) = word(pointer(input));
            }
            ++index;
            output += 4u;
        }
        void* old_storage = pointer(word(owner, 0x24));
        if (old_storage != nullptr) {
            crt.free_00bf6989(old_storage);
        }
        word(owner, 0x24) = address(grown);
    }
    const U32 index = word(owner, 0x28);
    const U32 output = word(owner, 0x24) + index * 4u;
    if (output != 0) {
        word(pointer(output)) = word(owner, value_offset);
    }
    word(owner, 0x28) = word(owner, 0x28) + 1u;
}
} // namespace

void prepare_chain_00c4d370(void* owner,
    const cc15_dyn_live::DynLiveCrt& crt, const QueryProvider& query) {
    const U32 next_data = word(owner, 0x0c);
    U32 anchor;
    for (;;) {
        anchor = word(owner, 0x04);
        U32 previous;
        if (orientation_above_zero(owner, next_data, anchor, previous)) {
            const U32 next = word(pointer(next_data), anchor * 4u);
            const U32 walk_start = word(pointer(next_data), next * 4u);
            const TriangleQuery inputs{walk_start, next, previous, previous, anchor};
            if (query.triangle_ring_clear_00c410e0(query.context, owner, inputs) != 0) {
                break;
            }
        }
        word(owner, 0x04) = word(pointer(next_data), anchor * 4u);
    }
    const U32 previous_data = word(owner, 0x18);
    word(owner, 0x34) = word(pointer(previous_data), anchor * 4u);
    word(owner, 0x30) = word(pointer(next_data), anchor * 4u);
    append_scratch(owner, 0x34, crt);
    append_scratch(owner, 0x04, crt);
    append_scratch(owner, 0x30, crt);
}

} // namespace bsp::cc16_dyn_ring_prepare
