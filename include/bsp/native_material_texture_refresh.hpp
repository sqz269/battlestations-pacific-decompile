#pragma once

#include "bsp/native_material_effect_owner.hpp"
#include "bsp/native_renderer_lua_owner.hpp"
#include "bsp/native_texture_loading_cache.hpp"

namespace bsp {

// Both are native C-string addresses, not NativeString headers. F8D438 is
// loader-zero storage with no observed writer, NOT proven immutable at runtime;
// preserve its current bytes. CE3A0C is an empty RO literal.
// Borrow their application data bindings; no private Lua/global owner is made.
struct NativeMaterialTextureNameLiterals {
    const char* nil_00f8d438;
    const char* non_string_00ce3a0c;
};

struct NativeMaterialTextureNameOperation final {
    enum class Phase { fresh, running, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    NativeLuaObjectStorage globals, value;
    bool globals_live{}, value_live{}, output_started{}, output_returned{};
    NativeString* output{};
    NativeMaterialTextureNameOperation() noexcept {}
    ~NativeMaterialTextureNameOperation();
    NativeMaterialTextureNameOperation(const NativeMaterialTextureNameOperation&) = delete;
    NativeMaterialTextureNameOperation& operator=(const NativeMaterialTextureNameOperation&) = delete;
};

// Complete normal B1BC70..B1BD1C, original ECX owner, stack fresh output/key,
// RET8/EAX output. Reads actual owner+4 Lua globals, looks up key.data as a
// C string, destroys globals, constructs fresh output from a STRING or the
// explicit fallback, then destroys the tracked value. NIL reads the CURRENT
// F8D438 C-string bytes. Source lookup uses the existing same-frame
// protected Lua boundary; errors retain this frame, not native FH3 unwinding.
NativeString* resolve_native_renderer_lua_string_00b1bc70(
    NativeRendererLuaOwnerStorage& captured_owner, void* fresh_output,
    const NativeString& actual_key, NativeStringStorage&,
    const NativeMaterialTextureNameLiterals&, NativeMaterialTextureNameOperation&);

struct NativeMaterialTextureRefreshContext {
    NativeTextureCacheContext& cache;
    NativeRenderActualOwners& actual_owners;
    void* const volatile& current_lua_owner_00f8d434;
    const volatile std::uint32_t* renderer_profile_00d5f0a8;
    NativeMaterialTextureNameLiterals literals;
};

struct NativeMaterialTextureRefreshStep final {
    NativeString selected, resolved;
    NativeMaterialTextureNameOperation name;
    NativeTextureCacheAcquired texture;
    bool selected_live{}, resolved_live{}, returned_reference_live{};
    void* returned_texture{};
};
struct NativeMaterialTextureRefreshOperation final {
    enum class Phase { fresh, running, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{}, index{};
    // Address-stable locals/child frames, not additional texture owners or
    // references. A failed provider keeps its live data here; never replay it.
    std::array<NativeMaterialTextureRefreshStep,11> steps;
    ~NativeMaterialTextureRefreshOperation();
    NativeMaterialTextureRefreshOperation() = default;
    NativeMaterialTextureRefreshOperation(const NativeMaterialTextureRefreshOperation&) = delete;
    NativeMaterialTextureRefreshOperation& operator=(const NativeMaterialTextureRefreshOperation&) = delete;
};

// Complete normal B19000..B191CD over existing C4h material storage, original
// ECX material/RET0. Supported current renderer+64 is numeric D5F0A8/B319B0,
// dispatched through the real cache provider, never called as host code.
// Borrow cache.strings and cache.textures.current_renderer_00f8d394; actual
// owners must be the SAME canonical registry used by texture construction.
// Reload counts/publications at native sites. Empty selected names preserve
// existing textures; identity assignment skips its retain/old release but
// still releases the returned reference. Out-of-array indices are explicit
// source errors. Keep failed operations alive for diagnosis/reconciliation;
// neither destructor invents rollback, native FH3/SEH or a cleanup schedule.
void refresh_native_material_textures_00b19000(NativeMaterialEffectBaseStorage&,
    NativeMaterialTextureRefreshContext&, NativeMaterialTextureRefreshOperation&);
} // namespace bsp
