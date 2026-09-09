// Isolated original-byte comparison, enabled only with the locally audited
// reference header. No original addresses are mapped and no game is loaded.
#include "bsp/gui_geometry.hpp"
#include "bsp/font_geometry.hpp"
#include "bsp/d3d9_startup.hpp"
#include "gui_geometry_reference.hpp"
#include <cstring>
#include <cstdio>
#include <cmath>

bool probe_gui_geometry_reference() {
    constexpr std::size_t n = sizeof(gui_reference_bytes);
    auto* code = static_cast<unsigned char*>(VirtualAlloc(nullptr, n + 8,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!code) return false;
    std::memcpy(code, gui_reference_bytes, n);
    const float constants[]{1.0f, 0.75f};
    std::memcpy(code + n, constants, sizeof(constants));
    for (const auto& patch : gui_reference_patches) {
        const auto destination = reinterpret_cast<std::uintptr_t>(code)
            + (patch.target == 0xab1ed4 ? 0x674 : patch.target == 0xd7a24c ? n : n + 4);
        std::memcpy(code + patch.offset, &destination, 4);
    }
    for (unsigned i = 0; i < 5; ++i) {
        std::uint32_t entry{};
        std::memcpy(&entry, code + 0x674 + 4 * i, 4);
        entry = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(code)) + entry - 0xab1860;
        std::memcpy(code + 0x674 + 4 * i, &entry, 4);
    }
    DWORD old{};
    if (!VirtualProtect(code, n + 8, PAGE_EXECUTE_READ, &old)
        || !FlushInstructionCache(GetCurrentProcess(), code, n + 8)) {
        VirtualFree(code, 0, MEM_RELEASE); return false;
    }
    using Native = void (__thiscall *)(void*, void*, const float*, float, float, float, float, const float*);
    Native native{};
    static_assert(sizeof(native) == sizeof(code));
    std::memcpy(&native, &code, sizeof(native));
    bool exact = true;
    for (unsigned mode = 0; mode < 5; ++mode) {
        bsp::GuiQuadParameters p;
        p.mode = mode; p.ratio = 0.37f; p.width = 0.73f; p.height = 0.61f;
        p.uv = {0.11f,0.23f,0.87f,0.93f}; p.crop = {0.13f,0.19f,0.83f,0.91f};
        std::array<bsp::GuiQuadVertex,4> reference{}, projected{};
        static_assert(sizeof(bsp::GuiQuadVertex) == 20);
        std::uint32_t object[0x118 / 4]{};
        object[0x110 / 4] = mode;
        std::memcpy(&object[0x114 / 4], &p.ratio, 4);
        std::uint32_t stream[11]{};
        stream[2] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(reference.data()));
        stream[3] = 20; stream[4] = 0; stream[10] = 12;
        const float uv[]{p.uv.left,p.uv.top,p.uv.right,p.uv.bottom};
        const float size[]{p.width,p.height};
        native(object, stream, uv, p.crop.left, p.crop.top, p.crop.right, p.crop.bottom, size);
        bsp::gui_write_cropped_quad_00ab1860(p, projected);
        unsigned differences = 0; float maximum = 0;
        for (std::size_t i = 0; i < sizeof(reference) / 4; ++i) {
            float a{}, b{};
            std::memcpy(&a, reinterpret_cast<const char*>(reference.data()) + i * 4, 4);
            std::memcpy(&b, reinterpret_cast<const char*>(projected.data()) + i * 4, 4);
            if (std::memcmp(&a, &b, 4)) ++differences;
            if (std::fabs(a-b) > maximum) maximum = std::fabs(a-b);
        }
        std::printf("GUI native comparison: mode=%u differing_words=%u max_abs=%.9g\n", mode, differences, maximum);
        exact = exact && differences == 0;
    }
    VirtualFree(code, 0, MEM_RELEASE);
    return exact;
}

// One isolated prefix case: original bytes00ab990c..00ab9c5d, with a local
// stack/register wrapper. No original process, SEH setup, or child-UI calls.
// Finite inputs and the caller's current x87 control word are compared; this
// does not validate exception dispatch, optional postprocessing, or rendering.
bool probe_font_geometry_reference() {
    constexpr std::size_t n = sizeof(font_reference_bytes);
    auto* code = static_cast<unsigned char*>(VirtualAlloc(nullptr, n + 24,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!code) return false;
    std::memcpy(code, font_reference_bytes, n);
    const double horizontal = 960.0, vertical = 720.0;
    const float white = 1.0f, vertical_scale = 0.83f;
    std::memcpy(code + n, &horizontal, 8);
    std::memcpy(code + n + 8, &vertical, 8);
    std::memcpy(code + n + 16, &white, 4);
    std::memcpy(code + n + 20, &vertical_scale, 4);
    for (const auto& patch : font_reference_patches) {
        const std::size_t offset = patch.target == 0xcec380 ? 0
            : patch.target == 0xcef1b8 ? 8 : patch.target == 0xd7a24c ? 16 : 20;
        const auto destination = reinterpret_cast<std::uintptr_t>(code + n + offset);
        std::memcpy(code + patch.offset, &destination, 4);
    }
    DWORD old{};
    if (!VirtualProtect(code, n + 24, PAGE_EXECUTE_READ, &old)
        || !FlushInstructionCache(GetCurrentProcess(), code, n + 24)) {
        VirtualFree(code, 0, MEM_RELEASE); return false;
    }
    using Native = void (__thiscall *)(void*, const void*, const float*, void*,
        std::uint16_t*, std::uint32_t, std::uint32_t, std::uint32_t,
        std::uint32_t, std::uint32_t, std::uint32_t);
    Native native{};
    static_assert(sizeof(native) == sizeof(code));
    std::memcpy(&native, &code, sizeof(native));
    bsp::FontGlyphData glyph;
    glyph.fields_00_0c = {0.13f, 0.27f, 0.79f, 0.91f};
    glyph.field_10 = 0xfff9u; // -7 signed bearing.
    glyph.scaled_field_12 = 19; glyph.scaled_field_14 = 23;
    // Construct native payload explicitly: host structure padding is irrelevant.
    std::array<unsigned char, 32> payload{};
    std::memcpy(payload.data(), glyph.fields_00_0c.data(), 16);
    std::memcpy(payload.data() + 0x10, &glyph.field_10, 2);
    std::memcpy(payload.data() + 0x14, &glyph.scaled_field_14, 2);
    bsp::FontGeometryParameters p;
    p.x = 71.37f; p.y = 29.19f; p.width_scale = 0.73f;
    p.vertical_scale = vertical_scale; p.height = 31; p.quad_index = 5;
    bsp::FontGeometryLayout layout;
    layout.stride = 28; layout.position_offset = 0;
    layout.uv_offset = 12; layout.packed_color_offset = 20;
    constexpr std::uint32_t first_vertex = 1;
    std::array<unsigned char, 6 * 28> reference, projected;
    reference.fill(0xa5); projected.fill(0xa5);
    std::array<std::uint16_t, 6> reference_indices{}, projected_indices{};
    std::uint32_t context[0x1dc / 4]{};
    std::memcpy(&context[0x1d8 / 4], &p.width_scale, 4);
    std::uint32_t writer[0x48 / 4]{};
    writer[2] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(reference.data()));
    writer[3] = layout.stride; writer[4] = layout.position_offset;
    writer[0x28 / 4] = layout.uv_offset;
    writer[0x34 / 4] = static_cast<std::uint32_t>(layout.packed_color_offset);
    const float position[]{p.x, p.y};
    native(context, payload.data(), position, writer, reference_indices.data(),
        0, 0, p.quad_index, p.height, first_vertex, 0x41);
    const bool written = bsp::write_font_quad_00ab98f0_fragment(glyph, p, layout,
        projected.data(), projected.size(), first_vertex, projected_indices);
    const bool vertices_equal = reference == projected;
    const bool indices_equal = reference_indices == projected_indices;
    std::printf("Font native prefix comparison: vertex_bytes_equal=%d indices_equal=%d\n",
        vertices_equal, indices_equal);
    VirtualFree(code, 0, MEM_RELEASE);
    return written && vertices_equal && indices_equal;
}
