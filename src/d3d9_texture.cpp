#include "bsp/d3d9_texture.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/texture_load_policy.hpp"
#include <d3dx9tex.h>

namespace bsp {
D3D9RetainedTexture2D::~D3D9RetainedTexture2D() {
    // 00b3f2e0 releases retained source before its COM texture. Other native
    // destructor duties (registry, surfaces, base/pool) remain unimplemented.
    source_.reset();
    release_com();
}

void D3D9RetainedTexture2D::assign_source_00b23640_fragment(
    const std::shared_ptr<MemoryStream>& source) {
    if (source_.get() == source.get()) return;
    source_ = source;
}

void D3D9RetainedTexture2D::release_com() noexcept {
    if (texture_) texture_->Release();
    texture_ = nullptr;
}

HRESULT D3D9RetainedTexture2D::initialize_00b2c2d0_fragment(IDirect3DDevice9& device,
    CreateTextureFromMemory create, const std::shared_ptr<MemoryStream>& source,
    const TextureLoadPolicy& policy) {
    if (!source || !source->fully_initialized() || source_ || texture_) return D3DERR_INVALIDCALL;
    const MemoryTextureOptions requested{policy.requested_width, policy.requested_height,
        policy.requested_mip_levels, options_.format};
    const HRESULT result = texture_create_from_retained_memory_00b3e190(device, create,
        source->data_00bef610(), static_cast<UINT>(source->size_00bef600()), requested, texture_);
    // Native creates a logical wrapper whenever the resulting COM pointer exists,
    // then retains its source. Report HRESULT without assuming pointer/HR parity.
    if (texture_) {
        // 00b3f930 stores actual level-zero Width/Height/Format at +28/+2C/+18;
        // policy saved dimensions instead go to native +34/+38. They can differ
        // after D3DX conversion. +3C receives requested mips after construction.
        D3DSURFACE_DESC description{};
        const HRESULT described = texture_->GetLevelDesc(0, &description);
        if (FAILED(described)) {
            // Explicit host failure boundary: do not retain unusable metadata.
            release_com();
            return FAILED(result) ? result : described;
        }
        options_ = {description.Width, description.Height,
            policy.requested_mip_levels, description.Format};
        assign_source_00b23640_fragment(source);
    }
    return result;
}

HRESULT D3D9RetainedTexture2D::recreate_00b3e190(IDirect3DDevice9& device,
    CreateTextureFromMemory create) {
    // Explicit typed boundary: native does not protect an uninitialized tail
    // after a short source read. Never feed such bytes to the decoder here.
    if (!source_ || !source_->fully_initialized()) return D3DERR_INVALIDCALL;
    return texture_create_from_retained_memory_00b3e190(device, create,
        source_->data_00bef610(), static_cast<UINT>(source_->size_00bef600()), options_, texture_);
}

HRESULT texture_create_from_retained_memory_00b3e190(IDirect3DDevice9& device,
    CreateTextureFromMemory create, const void* bytes, UINT size,
    const MemoryTextureOptions& options, IDirect3DTexture9*& output) {
    if (!create || output) return D3DERR_INVALIDCALL;
    // CMP +18h,14h; request A8R8G8B8 for R8G8B8, otherwise UNKNOWN.
    const auto format = options.format == D3DFMT_R8G8B8
        ? D3DFMT_A8R8G8B8 : D3DFMT_UNKNOWN;
    // Exact native stack at 00b3e1b4..00b3e1e5: managed pool, usage zero,
    // filter 70004h, mip filter FFFFFFFFh, zero color key and optional outputs.
    return create(&device, bytes, size, options.width, options.height,
        options.mip_levels, 0, format, D3DPOOL_MANAGED, 0x70004u, 0xffffffffu,
        0, nullptr, nullptr, &output);
}

HRESULT load_retained_texture_2d_00b2c2d0_fragment(IDirect3DDevice9& device,
    ReadImageInfoFromMemory read_info, CreateTextureFromMemory create,
    const std::shared_ptr<MemoryStream>& source, const TextureLoadNameView& name,
    std::uint32_t mip_reduction, std::unique_ptr<D3D9RetainedTexture2D>& output) {
    if (!read_info || !create || !source || !source->fully_initialized() || output)
        return D3DERR_INVALIDCALL;
    D3DXIMAGE_INFO info{};
    const HRESULT inspected = read_info(source->data_00bef610(),
        static_cast<UINT>(source->size_00bef600()), &info);
    if (FAILED(inspected)) return inspected;
    if (info.ResourceType != D3DRTYPE_TEXTURE) return D3DERR_INVALIDCALL;
    TextureLoadPolicy policy;
    if (!select_initial_texture_load_policy_00b2c405(name,
        {info.Width, info.Height, info.MipLevels}, mip_reduction, policy))
        return D3DERR_INVALIDCALL;
    auto owner = std::make_unique<D3D9RetainedTexture2D>(MemoryTextureOptions{
        policy.saved_width, policy.saved_height, policy.requested_mip_levels, info.Format});
    const HRESULT result = owner->initialize_00b2c2d0_fragment(device, create, source, policy);
    if (owner->texture()) output = std::move(owner);
    return result;
}
}
