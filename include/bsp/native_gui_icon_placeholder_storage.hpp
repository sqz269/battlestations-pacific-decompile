#pragma once
#include <cstdint>

namespace bsp {
struct NativeGuiIconStorageContext;
struct NativeTextureCacheContext;
struct NativeTextureCacheAcquired;
struct NativeRenderActualOwnerReleaseObservation;

// Actual ordinary native-header/argument/status homes; no default values or
// owner. Prepare/retain before enclosing effects. Empty user-provided ctor
// prevents value-init from adding home stores; no implicit Native destructor.
struct NativeGuiIconPlaceholderStorageFrame {
    NativeGuiIconPlaceholderStorageFrame() {}
    NativeGuiIconPlaceholderStorageFrame(const NativeGuiIconPlaceholderStorageFrame&) = delete;
    NativeGuiIconPlaceholderStorageFrame& operator=(const NativeGuiIconPlaceholderStorageFrame&) = delete;
    NativeGuiIconPlaceholderStorageFrame(NativeGuiIconPlaceholderStorageFrame&&) = delete;
    NativeGuiIconPlaceholderStorageFrame& operator=(NativeGuiIconPlaceholderStorageFrame&&) = delete;
    std::uint32_t header[2], preserve_home, state_home;
};

struct NativeGuiIconPlaceholderStorageContext {
    NativeGuiIconStorageContext& icon;
    NativeTextureCacheContext& cache;
    NativeTextureCacheAcquired& load_acquired;
    NativeRenderActualOwnerReleaseObservation& old_release;
    const volatile std::uint32_t& feature_word_0109eea4;
    const void* transparent_literal_00d5c058;
};

// Whole216B/76row AB3B30 ordinary Source schedule; original ECX state/RET0 is
// changed to this three-argument Source API. Actual40B state, not GuiIconState.
// Stage preserveDWORD1; set3C=1/3D=0; zero genuine8B header; exact41DD40(41),
// then if CURRENTDATA nonnull BF7680 with CURRENTLEN+1 INCLUDING zero/wrap.
// Literal is authenticated original42B (41characters/NUL), never CString41E870.
// Only normal completed header construction arms Source name cleanup. On a
// later C++ escape call existing whole raw41DD20 then barethrow only if it
// returns. A secondary escape propagates under ordinary Source C++; no new
// terminate wrapper or Original FH3/SEH/double-exception equivalence is claimed.
// Normal cleanup captures DATA BEFORE state-1, then CURRENTLEN+1/freshgetter/
// realBD1510, no postcall header clear/re-read or action replay.
//
// CURRENT renderer/profile/+64 is captured after name copy as inert data.
// Reach this edge only in the genuine existing Source-created D5F0A8/B319B0
// class/custody domain with fresh distinct load_acquired. OTHER64 stays OPEN;
// a profile/target/count alone does not prove class/storage or grant a call.
// Original header/literal0 goes to real B319B0/SAMEactualcache/rawpool domains.
// Capture old state0 AFTER load. Equal skips publication/counters. Otherwise
// publish new first, increment its genuine atomic+04 if nonnull, then release
// captured old through real observed-owner release/currentzero services.
// old_release is genuinely fresh and caller-retained through all callbacks,
// normal/failed postcall/catch/disposition obligations; no local observation
// is substituted. No extra release of the loaded result or rollback is added.
// No state/pointee read follows a possibly terminal old release.
//
// Caller proves actual state/subobject/atomic/header/block/literal/context and
// canonical-owner/provider lifetimes; cache.strings uses SAME RawPool cells.
// Declared operand homes remain current; private controls/spills are separate.
// Retain current/partial/completed graph and outstanding pool/zero-frame debt
// on escape. Passive state/history is not Native Origin/cleanup/retirement
// authority. Existing rawpool/cache/SDK/observed-zero/secondary policies remain
// inherited. No new owner/callback/default/map/countcap/NoThrow/retry/issuer or
// OriginalABI/FLAGS/fault/runtime/activation/extra body credit is supplied.
void set_native_gui_icon_placeholder_storage_00ab3b30(void* actual_state40,
    NativeGuiIconPlaceholderStorageFrame&, NativeGuiIconPlaceholderStorageContext&);
} // namespace bsp
