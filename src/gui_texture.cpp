#include "bsp/gui_texture.hpp"
#include <cmath>
#include <utility>

namespace bsp {
namespace {
// 00aa2725..2757: signed FILD, optional +2^32, then FSTP float32.
float native_dimension(std::uint32_t value) {
#if defined(_MSC_VER) && defined(_M_IX86)
    const double exact_value = static_cast<double>(value);
    float result;
    // Every uint32 value is exact as double, so FLD gives the same numerical
    // value as native FILD/unsigned correction before the float32 store.
    __asm {
        fld qword ptr [exact_value]
        fstp dword ptr [result]
    }
    return result;
#else
    return static_cast<float>(value);
#endif
}

// 00aa275b..2789 and 00aa278b..27b3: difference is spilled to float32,
// absolute value clears its sign bit, then multiply/divide/scale remains x87
// until the final float32 store. Do not reassociate scale before the division.
float native_size(float end_value, float begin_value, float dimension, float scale, double divisor) {
#if defined(_MSC_VER) && defined(_M_IX86)
    float difference;
    float result;
    __asm {
        fld dword ptr [end_value]
        fsub dword ptr [begin_value]
        fstp dword ptr [difference]
        mov eax, dword ptr [difference]
        and eax, 07fffffffh
        mov dword ptr [difference], eax
        fld dword ptr [difference]
        fmul dword ptr [dimension]
        fdiv qword ptr [divisor]
        fmul dword ptr [scale]
        fstp dword ptr [result]
    }
    return result;
#else
    // Portable semantic fallback: double lacks x87 extended intermediates and
    // does not reproduce arbitrary ambient x87 precision/rounding settings.
    const float difference = static_cast<float>(static_cast<double>(end_value) - begin_value);
    const double product = static_cast<double>(std::fabs(difference)) * dimension;
    const double divided = product / divisor;
    return static_cast<float>(divided * scale);
#endif
}
}

void* resolve_gui_texture_00aa2660(std::string_view name,
    std::array<float, 4>& uv, std::array<float, 2>& size, float scale,
    const GuiTextureCallbacks& callbacks) {
    const auto* item = callbacks.find_atlas_item(name); // Native manager00f8c26c.
    if (!item) return callbacks.load_texture(name, 0); // Renderer vtable+64h.

    // Native FCOMI/JBE leaves flips false on unordered input. The signs of
    // finite float endpoint differences agree with these ordered comparisons;
    // no premature float32 subtraction can underflow a sign here.
    const bool flip_u = uv[2] < uv[0];
    const bool flip_v = uv[3] < uv[1];
    uv = item->uv;
    if (flip_u) std::swap(uv[0], uv[2]);
    if (flip_v) std::swap(uv[1], uv[3]);

    if (size[0] == 0.0f && size[1] == 0.0f) {
        // Order matches native: width query/store, height query/store, then
        // width output, height output, and finally reference retention.
        const float width = native_dimension(callbacks.width(item->texture));
        const float height = native_dimension(callbacks.height(item->texture));
        size[0] = native_size(uv[2], uv[0], width, scale, 960.0);
        size[1] = native_size(uv[3], uv[1], height, scale, 720.0);
    }
    callbacks.retain(item->texture); // Native InterlockedIncrement(texture+4).
    return item->texture;
}
}
