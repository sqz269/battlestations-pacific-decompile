#pragma once

#include "bsp/native_texture_loading_cache.hpp"

namespace bsp {

// B3F5D0's 0Ch producer: the COM word is borrowed; only the name owns storage.
struct NativeTextureReloadDiagnostic {
    IDirect3DTexture9* borrowed_texture{};
    NativeString name;
};
static_assert(sizeof(NativeTextureReloadDiagnostic) == 12);
static_assert(offsetof(NativeTextureReloadDiagnostic, name) == 4);

// Explicit native stack output preimages. Both native HRESULTs are ignored.
// Providers may leave fields untouched on failure, so the caller must supply
// the intended initialized preimage for any unwritten field that is consumed.
// Successful real D3DX/GetLevelDesc calls supply their own complete outputs.
// These two objects must be disjoint from owner/name/acquired/service storage.
struct NativeTextureReloadOutputs {
    D3DXImageInfo& image_info;
    D3DSURFACE_DESC& level_zero;
};

// Diagnostic state, never an additional resource owner. A failed invocation
// cannot be replayed: the native body has no source/COM rollback or stream EH
// cleanup. Retain this state until externally resolving any acquired stream.
struct NativeTextureReloadAcquired {
    enum class Phase { not_started, running, complete, failed };
    Phase phase{Phase::not_started};
    std::uint32_t native_site{};
    void* owner{};
    const void* name{};
    void* source{};
    void* memory{};
    bool source_release_started{}, memory_release_started{};
    bool old_com_release_started{}, diagnostic_constructed{}, diagnostic_released{};
    IDirect3DDevice9* captured_device{};
    HRESULT image_info_result{}, create_result{}, level_desc_result{};
    NativeTextureReloadDiagnostic diagnostic;
};

// Complete B3F5D0..B3F620: ECX output, stack pointer-to-COM/name, EAX output,
// RET8. Initialize the borrowed word and actual8h header, resize, then overlap-
// safe BF7680 copy. No AddRef, support call or cleanup before constructor return.
void* construct_native_texture_reload_diagnostic_00b3f5d0(void* actual_output,
    IDirect3DTexture9* const* borrowed_texture, const void* actual_name,
    ActualNativeStringPoolStorage&);

// Complete B3FA90..B3FD7A source control flow. Original ECX actual D61948 owner,
// RET; this new C++ interface borrows the SAME loading/cache/VFS/string/owner
// domains and caller-provided output preimages. Numeric profiles select concrete
// source functions only; D3D uses the actual COM interface/import providers.
// Identity and canonical companion remain unchanged. No retained-source +4C,
// flags, requested-mips +3C or surface-cache modification occurs in this body.
// No ABI, original-code differential, application binding or game claim.
void reload_native_texture_00b3fa90(void* actual_owner,
    NativeTextureLoadingContext&, NativeTextureReloadOutputs,
    NativeTextureReloadAcquired&);

} // namespace bsp
