#pragma once
#include "bsp/d3d9_startup.hpp"
#include <memory>

namespace bsp {
class MemoryStream;
// Opaque because this call always passes null for optional image information.
struct D3DXImageInfo;
using CreateTextureFromMemory = HRESULT (WINAPI *)(IDirect3DDevice9*, const void*, UINT,
    UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, DWORD, DWORD, D3DCOLOR,
    D3DXImageInfo*, PALETTEENTRY*, IDirect3DTexture9**);

struct MemoryTextureOptions {
    UINT width{};              // native object+28h
    UINT height{};             // +2Ch
    UINT mip_levels{};         // +3Ch
    D3DFORMAT source_format{}; // +18h
};

// Semantic port of 00b3e190: ECX=native wrapper, no stack arguments, RET.
// The caller supplies the device and retained file's bytes instead of native
// singleton/file-service traversal. The imported function/module must be live.
// Output must be empty; caller owns the returned COM reference. Native code
// ignores HRESULT; this interface returns it. It does not release an old texture.
HRESULT texture_create_from_retained_memory_00b3e190(IDirect3DDevice9&,
    CreateTextureFromMemory, const void* bytes, UINT size,
    const MemoryTextureOptions&, IDirect3DTexture9*& output);

// Typed projection of 2D texture+4Ch retained stream and +10h COM owner.
// Native registry, cached surface owners, allocator and intrusive ABI omitted.
class D3D9RetainedTexture2D {
public:
    explicit D3D9RetainedTexture2D(MemoryTextureOptions options) : options_(options) {}
    ~D3D9RetainedTexture2D();
    D3D9RetainedTexture2D(const D3D9RetainedTexture2D&) = delete;
    D3D9RetainedTexture2D& operator=(const D3D9RetainedTexture2D&) = delete;
    // Identity skip; retain new stream before dropping old. This retains the
    // same stream wrapper, including its cursor, rather than cloning it.
    void assign_source_00b23640_fragment(const std::shared_ptr<MemoryStream>& source);
    HRESULT recreate_00b3e190(IDirect3DDevice9&, CreateTextureFromMemory);
    IDirect3DTexture9* texture() const noexcept { return texture_; }
    const std::shared_ptr<MemoryStream>& source() const noexcept { return source_; }
    // Host orchestration helper only, not a recovered complete reset callback.
    void release_com() noexcept;
private:
    MemoryTextureOptions options_;
    std::shared_ptr<MemoryStream> source_;
    IDirect3DTexture9* texture_{};
};
}
