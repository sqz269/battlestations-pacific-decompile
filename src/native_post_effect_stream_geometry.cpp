#include "bsp/native_post_effect_stream_geometry.hpp"
#include <cstddef>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native post-effect stream geometry requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(NativePostEffectGeometryConstants) == 8);
static_assert(offsetof(NativePostEffectGeometryConstants, actual_one_00d7a24c) == 4);

void write_native_post_effect_stream_geometry_00b4d2d0(void* output,
    std::uint32_t first_record, std::uint32_t record_count, const void* input,
    const NativePostEffectGeometryConstants& constants) noexcept {
    const auto* cells = &constants;
    // Keep the whole access sequence in one block: SUB EDI sets the loop flags
    // before the remaining stores, which do not change them. FLD/FSTP duplicates
    // load the already-written destination, not an earlier captured input.
    __asm {
        mov esi, first_record
        imul esi, esi, 168
        add esi, output
        mov edi, record_count
        test edi, edi
        jbe bt_writer_done
        mov ecx, input
        mov edx, cells
        mov eax, dword ptr [edx]
        movss xmm0, dword ptr [eax]
        mov eax, dword ptr [edx + 4]
        movss xmm1, dword ptr [eax]
        add ecx, 8
        lea eax, [esi + 116]
    bt_writer_loop:
        movss xmm2, dword ptr [ecx - 8]
        movss xmm3, dword ptr [ecx + 4]
        movss dword ptr [esi], xmm2
        movss dword ptr [eax - 112], xmm3
        movss dword ptr [eax - 108], xmm0
        movss dword ptr [eax - 104], xmm1
        movss xmm2, dword ptr [ecx - 8]
        movss xmm3, dword ptr [ecx - 4]
        movss dword ptr [eax], xmm3
        movss dword ptr [eax + 4], xmm0
        movss dword ptr [eax + 8], xmm1
        movss dword ptr [eax - 4], xmm2
        movss dword ptr [eax - 88], xmm2
        fld dword ptr [eax]
        fstp dword ptr [eax - 84]
        add esi, 168
        fld dword ptr [eax + 4]
        add eax, 168
        fstp dword ptr [eax - 248]
        add ecx, 36
        sub edi, 1
        fld dword ptr [eax - 160]
        fstp dword ptr [eax - 244]
        movss xmm2, dword ptr [ecx - 36]
        movss xmm3, dword ptr [ecx - 32]
        movss dword ptr [eax - 196], xmm3
        movss dword ptr [eax - 192], xmm0
        movss dword ptr [eax - 188], xmm1
        movss dword ptr [eax - 200], xmm2
        movss dword ptr [eax - 228], xmm2
        fld dword ptr [eax - 196]
        fstp dword ptr [eax - 224]
        fld dword ptr [eax - 192]
        fstp dword ptr [eax - 220]
        fld dword ptr [eax - 188]
        fstp dword ptr [eax - 216]
        movss xmm2, dword ptr [ecx - 36]
        movss xmm3, dword ptr [ecx - 40]
        movss dword ptr [eax - 144], xmm2
        movss dword ptr [eax - 140], xmm3
        movss dword ptr [eax - 136], xmm0
        movss dword ptr [eax - 132], xmm1
        movss xmm2, dword ptr [ecx - 28]
        movss xmm3, dword ptr [ecx - 16]
        movss dword ptr [eax - 268], xmm2
        movss dword ptr [eax - 264], xmm3
        movss xmm2, dword ptr [ecx - 28]
        movss xmm3, dword ptr [ecx - 24]
        movss dword ptr [eax - 152], xmm3
        movss dword ptr [eax - 156], xmm2
        movss dword ptr [eax - 240], xmm2
        fld dword ptr [eax - 152]
        fstp dword ptr [eax - 236]
        movss xmm2, dword ptr [ecx - 20]
        movss xmm3, dword ptr [ecx - 16]
        movss dword ptr [eax - 180], xmm3
        movss dword ptr [eax - 184], xmm2
        movss dword ptr [eax - 212], xmm2
        fld dword ptr [eax - 180]
        fstp dword ptr [eax - 208]
        movss xmm2, dword ptr [ecx - 20]
        movss xmm3, dword ptr [ecx - 24]
        movss dword ptr [eax - 128], xmm2
        movss dword ptr [eax - 124], xmm3
        mov edx, dword ptr [ecx - 12]
        mov dword ptr [eax - 120], edx
        mov dword ptr [eax - 148], edx
        mov dword ptr [eax - 176], edx
        mov dword ptr [eax - 204], edx
        mov dword ptr [eax - 232], edx
        mov dword ptr [eax - 260], edx
        jnz bt_writer_loop
    bt_writer_done:
    }
}

namespace {
std::uint32_t word(const void* actual, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, actual
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
void require_slot(void* stream, NativePostEffectStreamGeometryContext& context,
    std::uint32_t slot, std::uint32_t callee) {
    if (!stream || word(stream) != 0x00d61d6cu || !context.actual_logical_profile_00d61d6c ||
        context.actual_logical_profile_00d61d6c[slot / 4u] != callee)
        throw std::logic_error("post-effect geometry requires its current concrete logical stream slot");
}
}

void populate_native_post_effect_stream_geometry_00b4d4c0(void* actual,
    const void* input, NativePostEffectStreamGeometryContext& context) {
    void* stream = pointer(word(actual, 0x18)); // B4D4C3, BEFORE count load.
    const std::uint32_t count = word(actual, 0x20); // B4D4C6.
    require_slot(stream, context, 0x10, 0x00b49980u);
    void* mapped = lock_native_logical_vertex_stream_00b49980(stream, context.mapping,
        count * 3u, 0, 0); // DWORD multiplication; no fallback or clamp.
    const std::uint32_t records = word(actual, 0x20) >> 1u; // B4D4DC: fresh after map.
    write_native_post_effect_stream_geometry_00b4d2d0(mapped, 0, records, input, context.constants);
    stream = pointer(word(actual, 0x18)); // B4D4ED: fresh after all writer stores.
    require_slot(stream, context, 0x14, 0x00b49a80u);
    unlock_native_logical_vertex_stream_00b49a80(stream, context.mapping);
}
} // namespace bsp
