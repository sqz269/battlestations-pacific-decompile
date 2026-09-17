#pragma once
#include "bsp/native_surface_owner.hpp"

namespace bsp {
using NativeTextureSurfaceReferenceIncrement = long (__stdcall*)(volatile long*);
struct NativeTextureSurfaceGetterContext {
    NativeSurfaceOwnerContext& surfaces;
    // The actual Win32 IAT target cell, captured once at function entry.
    NativeTextureSurfaceReferenceIncrement const volatile& actual_increment_00ce221c;
};
struct NativeTextureSurfaceGetterAcquired {
    enum class Phase { fresh, running, complete, failed };
    Phase phase{Phase::fresh};
    int unwind_state{-1};
    // Original first stack word is BOTH the incoming mip and COM output slot.
    // The routine sets this to the incoming mip bit pattern before COM writes.
    IDirect3DSurface9* volatile surface_output{};
    HRESULT get_surface_result{};
    void* raw_slot{};
    NativeSurfaceOwnerStorage* owner{};
    bool raw_slot_returned{};
};

// Full B3FD80..B3FE87, 264 bytes. Original ECX actual texture owner; two stack
// words (mip and unused), EAX retained surface owner, RET8. Searches the captured
// signed count/data and retains a matching owner BEFORE the native null test.
// Miss: real current COM GetSurfaceLevel, canonical surface pool, full B3F630,
// creator COM release, and optional retained cache entry unless current flags&1.
// Only raw-slot return is armed across construction; no COM or cache rollback.
// Context must use the same canonical pool/string/support/renderer domains as
// the texture. New source interface and C++ cleanup projection; native ABI/FH3/
// SEH and private-stack aliases are not established. No failed-HRESULT fallback.
NativeSurfaceOwnerStorage* get_native_texture_surface_00b3fd80(void* actual_texture,
    std::uint32_t mip, std::uint32_t unused,
    NativeTextureSurfaceGetterContext&, NativeTextureSurfaceGetterAcquired&);
} // namespace bsp
