#pragma once
#include <cstdint>

namespace bsp {
struct NativeParticleTextureNamesRawContext;
struct NativeTextureCacheContext;
struct NativeTextureCacheAcquired;
struct GuiTextureCallbacks;
struct GuiIconStateTextureConstants;

// All borrows precede entry and survive entered escapes. Atlas RawPool and
// cache.strings use the SAME real pool/manager/gate cells. No manager, renderer,
// header, counter or bank is created/copied. empty_name is the authenticated
// original F8BC60 byte address, not an invented default string.
struct NativeGuiTextureResolverStorageContext {
    NativeParticleTextureNamesRawContext& atlas;
    NativeTextureCacheContext& cache;
    NativeTextureCacheAcquired& miss_acquired;
    const GuiTextureCallbacks& dimensions;
    const GuiIconStateTextureConstants& constants;
    const char* empty_name_00f8bc60;
};

// Whole AA2660 ordinary data/FP schedule,385B/128rows. Original stdcall four
// DWORD homes/RET10h become this two-argument Source API. Homes start as
// [engine8B-name-header,UVfloat4,sizefloat2,scaleF32bits]; width/height processing
// reuses HOME0 for integer/diff bits, HOME1 for widthfloat, HOME2 heightfloat.
// Capture name first; DATA+4 only, fallback then CURRENT atlas publication.
// Capture UV only AFTER lookup; capture size only AFTER all UV stores/swaps.
// Legal UV/size/home/item aliases preserve reached loads/stores, not snapshots.
//
// Both dimension calls use the genuine existing captured integer consumer,
// extended for reported D61948/+3C B3CE50 and/+40 B3CE60. It inherits its
// canonical actual2D/SAME+04/storage domain and OTHER rejection. Width runs at
// relative FPdepth0. Height receiver alone is reloaded from CURRENT item+8
// while widthST0 is live; FSTP width HOME1 FIRST, then load THAT receiver's
// CURRENT profile/target. Source carriers and C++ consumer follow at depth0.
// No Source77 saved-dimension/pre-spill-pair rule or C++ FP is substituted.
// Scale HOME3 is loaded late and retained in x87 across width output and
// vertical reads; no second scale-home read or value cache across callbacks.
// Late item+8 before actual+04 increment and after it for return are distinct.
//
// Miss inherits genuine B319B0 over ORIGINAL captured engine8 header/literal0.
// When reached, caller proves CURRENT renderer is the existing Source-created
// D5F0A8/B319B0 class with real generation/custody, cache and fresh distinct
// acquired record. Profile/target/count data ALONE do not prove that domain.
// Capture exact current receiver/profile/+64 as inert words; never call code
// tokens or silently dispatch an arbitrary OTHER target to B319B0. OTHER+64
// transport/full production binding remains OPEN outside this Source domain.
//
// Caller proves actual atlas/item/texture/atomic+04/string/vector/subobject and
// publication/module/constant lifetimes. Declared homes and payload aliases
// stay live; private Source controls/carriers/spills stay separate. Retain
// actual homes/context/acquired/current graph/providers on an escape. No local
// EH/cleanup, FP reset/restore, NoThrow/Origin/owner/default/phase/rollback is
// supplied. Existing rawAtlas/cache/SDK/secondary-exception policies remain
// inherited. Ambient hardwareFP/fault/FLAGS/SEH/FH3/OriginalABI and production/
// disposition/runtime/activation are unproved; zero extra body/vendor/CG credit.
void* resolve_native_gui_texture_storage_00aa2660(
    volatile std::uint32_t (&actual_homes)[4], NativeGuiTextureResolverStorageContext&);
} // namespace bsp
