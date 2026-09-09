#include "bsp/d3d9_texture.hpp"
#include "bsp/memory_stream.hpp"

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
    const auto format = options.source_format == D3DFMT_R8G8B8
        ? D3DFMT_A8R8G8B8 : D3DFMT_UNKNOWN;
    // Exact native stack at 00b3e1b4..00b3e1e5: managed pool, usage zero,
    // filter 70004h, mip filter FFFFFFFFh, zero color key and optional outputs.
    return create(&device, bytes, size, options.width, options.height,
        options.mip_levels, 0, format, D3DPOOL_MANAGED, 0x70004u, 0xffffffffu,
        0, nullptr, nullptr, &output);
}
}
