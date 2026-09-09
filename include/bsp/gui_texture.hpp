#pragma once
#include "bsp/texture_atlas.hpp"

namespace bsp {
// Supplied services replace native globals/vtables, not their implementation.
// All callbacks are required when their corresponding path is taken.
struct GuiTextureCallbacks {
    std::function<const TextureAtlasItem*(std::string_view)> find_atlas_item;
    std::function<void*(std::string_view, std::uint32_t)> load_texture;
    std::function<std::uint32_t(void*)> width;
    std::function<std::uint32_t(void*)> height;
    std::function<void(void*)> retain;
};

// 00aa2660: native stdcall(filename string*, float4*, float2*, float), RET10h.
// New API, not binary compatible. On atlas match copies float UVs preserving
// input flip signs, optionally sets size when both components are zero, and
// retains the returned texture once. On a miss calls load_texture(name,0),
// preserving UV/size and adding no extra retain to that callback's result.
// The caller owns the returned reference according to its supplied services.
void* resolve_gui_texture_00aa2660(std::string_view name,
    std::array<float, 4>& uv, std::array<float, 2>& size, float scale,
    const GuiTextureCallbacks& callbacks);
}
