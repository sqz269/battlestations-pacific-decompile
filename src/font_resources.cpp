#include "bsp/font_resources.hpp"
#include "bsp/texture_load_policy.hpp"
#include <limits>
#include <utility>

namespace bsp {
FontResources::~FontResources() {
    // 00ad53a0: payloads before the two image references. Host map also destroys
    // its nodes here; native destroys tree nodes after images and strings.
    data.glyphs.clear();
    gfx.reset();
    alpha.reset();
}

namespace {
bool read_source(const FontPhysicalResolver& resolver, const std::string& name,
    std::shared_ptr<MemoryStream>& source, std::string& error) {
    std::string path;
    if (!resolver(name, path, error)) return false;
    if (path.empty() || path.find('\0') != std::string::npos) {
        error = "Font physical resolver returned an empty or embedded-NUL path.";
        return false;
    }
    PhysicalFile file;
    DWORD failure{};
    if (!file.open_read_only_00bf52a0_fragment(path.c_str(), failure)) {
        error = "Cannot open font resource " + name + ": Win32 " + std::to_string(failure);
        return false;
    }
    auto loaded = std::make_shared<MemoryStream>();
    if (!memory_stream_from_physical_00bef750_fragment(file, *loaded, failure)) {
        error = "Cannot read font resource " + name + ": Win32 " + std::to_string(failure);
        return false;
    }
    if (!loaded->fully_initialized()) {
        error = "Font resource has an uninitialized short-read tail: " + name;
        return false;
    }
    source = std::move(loaded);
    return true;
}
}

bool load_font_resources_00ad4c30_fragment(IDirect3DDevice9& device,
    ReadImageInfoFromMemory read_info, CreateTextureFromMemory create,
    const FontPhysicalResolver& resolver, const FontDescriptor& descriptor,
    const std::string& prefix, const std::string& extra, std::uint32_t mip_reduction,
    std::unique_ptr<FontResources>& output, std::string& error) {
    error.clear();
    if (!read_info || !create || !resolver || output) {
        error = "Font resource load requires live imports, resolver and empty output.";
        return false;
    }
    for (const auto* text : {&prefix, &extra, &descriptor.data_file,
             &descriptor.gfx_file, &descriptor.alpha_texture}) {
        if (text->find('\0') != std::string::npos) {
            error = "Embedded-NUL font resource names are outside this projection.";
            return false;
        }
    }
    const std::string gfx_name = prefix + extra + descriptor.gfx_file;
    const std::string alpha_name = descriptor.alpha_texture.empty()
        ? "white.tga" : prefix + descriptor.alpha_texture;
    const std::string data_name = prefix + extra + descriptor.data_file;
    auto result = std::make_unique<FontResources>();
    const auto load_image = [&](const std::string& name,
                               std::shared_ptr<D3D9RetainedTexture2D>& image) {
        if (name.size() > std::numeric_limits<std::uint32_t>::max()) {
            error = "Font image logical name exceeds the native DWORD length.";
            return false;
        }
        std::shared_ptr<MemoryStream> source;
        if (!read_source(resolver, name, source, error)) return false;
        std::unique_ptr<D3D9RetainedTexture2D> texture;
        const HRESULT hr = load_retained_texture_2d_00b2c2d0_fragment(device,
            read_info, create, source,
            {static_cast<std::uint32_t>(name.size()), name.c_str()}, mip_reduction, texture);
        // Explicit successful-load domain: native also publishes a nonnull
        // texture accompanying a failed HRESULT; this adapter releases it.
        if (FAILED(hr) || !texture) {
            error = "Cannot create font image " + name + ": HRESULT " + std::to_string(hr);
            return false;
        }
        image = std::shared_ptr<D3D9RetainedTexture2D>(std::move(texture));
        return true;
    };
    if (!load_image(gfx_name, result->gfx) || !load_image(alpha_name, result->alpha))
        return false;
    std::shared_ptr<MemoryStream> data_source;
    if (!read_source(resolver, data_name, data_source, error) ||
        !decode_font_data_00ad4c30_fragment(*data_source, descriptor.scale_ratio,
            result->data, error)) return false;
    output = std::move(result);
    return true;
}
}
