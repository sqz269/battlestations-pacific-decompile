// Isolated original-byte comparison, enabled only with the locally audited
// reference header. No original addresses are mapped and no game is loaded.
#include "bsp/gui_geometry.hpp"
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
