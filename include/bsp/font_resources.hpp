#pragma once
#include "bsp/d3d9_texture.hpp"
#include "bsp/font_data.hpp"
#include "bsp/font_registry.hpp"

namespace bsp {
// Host adapter from the exact native logical name to a physical ANSI filename.
// No normalization, mount selection, cache or implicit path component.
using FontPhysicalResolver = std::function<bool(const std::string& logical_name,
    std::string& physical_path, std::string& error)>;

struct FontResources {
    FontData data;
    // One font-level reference each (native embedded space fields +64/+68).
    // All glyph resource pointers alias these; material owners may retain them.
    std::shared_ptr<D3D9RetainedTexture2D> gfx;
    std::shared_ptr<D3D9RetainedTexture2D> alpha;
    ~FontResources();
    FontResources() = default;
    FontResources(const FontResources&) = delete;
    FontResources& operator=(const FontResources&) = delete;
};

// Successful initial-load projection of 00ad4c30; not native reload00ad51d0.
// Exact concatenation only: GFX/DAT=(prefix+extra)+name; alpha=prefix+name,
// except an empty alpha name loads unprefixed white.tga. Inputs must have no
// embedded NUL. Images load before DAT. Failure preserves empty output and
// cleans temporary ownership; native does not have this transactional boundary.
// Resolver/import/allocation exceptions propagate; imports must remain live.
bool load_font_resources_00ad4c30_fragment(IDirect3DDevice9&,
    ReadImageInfoFromMemory, CreateTextureFromMemory, const FontPhysicalResolver&,
    const FontDescriptor&, const std::string& prefix, const std::string& extra,
    std::uint32_t mip_reduction, std::unique_ptr<FontResources>& output,
    std::string& error);
}
