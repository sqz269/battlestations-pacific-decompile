#include "bsp/gui_text_geometry.hpp"

#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
std::uint32_t load(const void* address) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
void store_index(void* destination, std::uint16_t value) noexcept {
    __asm {
        mov eax, destination
        mov dx, value
        mov word ptr [eax], dx
    }
}
void zero_uv(void* destination) noexcept {
    __asm {
        mov eax, destination
        xorps xmm0, xmm0
        movss dword ptr [eax], xmm0
        movss dword ptr [eax + 4], xmm0
    }
}
bool matched_glyph(GuiTextLifetime& lifetime, std::uint16_t code_unit) {
    auto& fields = lifetime.fields();
    if (!fields.pointer_1b0 || !fields.pointer_1ac ||
        lifetime.content_binding().widget.extra_fields().layout_listener_dc ||
        fields.string_1a4.empty()) return false;
    for (const auto unit : fields.string_1a4) {
        if (unit == 0) break;
        if (unit == code_unit) return true;
    }
    return false;
}
} // namespace

GuiTextGlyphWriteResult write_gui_text_quad_00ab98f0_fragment(
    GuiTextLifetime& lifetime, const FontGlyphData& glyph,
    const FontGlyphPlacement& placement, std::uint32_t quad_index,
    std::uint16_t height, std::uint32_t first_vertex, void* stream,
    void* indices, const volatile float& vertical_scale) {
    FontGeometryParameters parameters;
    parameters.x = placement.x;
    parameters.y = placement.y;
    parameters.width_scale = lifetime.text().font_scale;
    parameters.vertical_scale = vertical_scale;
    parameters.height = height;
    parameters.quad_index = quad_index;
    FontGeometryLayout layout;
    layout.stride = load(at(stream, 0xc));
    layout.position_offset = load(at(stream, 0x10));
    layout.uv_offset = load(at(stream, 0x28));
    const auto color_bits = load(at(stream, 0x34));
    std::memcpy(&layout.packed_color_offset, &color_bits, sizeof(color_bits));
    if (layout.packed_color_offset < 0) {
        for (std::uint32_t i = 0; i < 4; ++i)
            layout.float_color_offsets[i] = load(at(stream, 0x38 + i * 4));
    }
    auto* const mapped = reinterpret_cast<std::uint8_t*>(load(at(stream, 8)));
    const auto bytes = static_cast<std::uint64_t>(load(at(stream, 0x64))) * layout.stride;
    std::array<std::uint16_t, 6> result_indices;
    if (!indices || bytes > std::numeric_limits<std::size_t>::max() ||
        !write_font_quad_00ab98f0_fragment(glyph, parameters, layout, mapped,
            static_cast<std::size_t>(bytes), first_vertex, result_indices))
        throw std::logic_error("Text glyph requires valid actual mapped stream capacity and writer fields");
    // The scalar kernel writes vertices directly into actual+08. Only the six
    // scalar index results are local, then published in original store order.
    for (const auto lane : {1u, 0u, 3u, 2u, 4u, 5u})
        store_index(at(indices, lane * 2u), result_indices[lane]);
    if (!matched_glyph(lifetime, placement.code_unit))
        return GuiTextGlyphWriteResult::complete;
    for (std::uint32_t i = 0; i < 4; ++i) {
        const auto stride = load(at(stream, 0xc));
        const auto base = load(at(stream, 8));
        const auto uv = load(at(stream, 0x28));
        zero_uv(reinterpret_cast<void*>((first_vertex + i) * stride + base + uv));
    }
    return GuiTextGlyphWriteResult::needs_glyph_child;
}
} // namespace bsp
