#pragma once

#include "bsp/native_render_resource_record.hpp"
#include "bsp/native_startup_shader_modes.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Platform renderer activation requires MSVC Win32.
#endif

namespace bsp {
struct NativeVfsDateRouteContext;
struct NativeStringRawPoolContext;

// Required native call boundaries. No success/no-op defaults: the canonical
// renderer owner must bind the real resource reload and material providers.
// A callback can change the current publications, row resource or renderer
// table; each caller preserves the native captures/reloads described below.
struct NativeRendererActivationBindings {
    virtual ~NativeRendererActivationBindings() = default;
    virtual void file_date_00bdd340(void* actual_manager,
        std::uint32_t* actual_five_word_output, const void* actual_name_header) = 0;
    virtual void resource_reload_slot_08(void* actual_resource,
        std::uint32_t current_native_target) = 0;
    virtual void refresh_material_textures_00b19000(void* actual_material) = 0;
};

// Application adapter for the already reconstructed raw date route. Derive
// only to bind the two resource/material leaves above. Borrow the SAME VFS
// date context and raw string-pool cells used by the renderer loading graph.
class NativeRendererActivationVfsBindings : public NativeRendererActivationBindings {
public:
    NativeRendererActivationVfsBindings(NativeVfsDateRouteContext&,
        NativeStringRawPoolContext&) noexcept;
    void file_date_00bdd340(void*, std::uint32_t*, const void*) final;
private:
    NativeVfsDateRouteContext& dates_;
    NativeStringRawPoolContext& strings_;
};

struct NativeRendererActivationContext {
    const NativeStartupShaderModes& modes;
    void* const volatile& actual_vfs_0109ceec;
    NativeRendererActivationBindings& calls;
};

// Complete normal B22030/B21F70 loops, respectively effect and texture
// registries. Capture DWORD count+8 BEFORE data+4 and the wrapping 2Ch-row end
// once. For each row query CURRENT CEEC, compare five unsigned date words,
// write a strictly newer date BEFORE reloading resource+28 and its slot+8.
// No null-resource repair, rescan after mutation, or implicit owner retention.
void refresh_native_effect_registry_dates_00b22030(void* actual_registry,
    NativeRendererActivationContext&);
void refresh_native_texture_registry_dates_00b21f70(void* actual_registry,
    NativeRendererActivationContext&);

// Complete normal B24DD0 loop. Capture initial renderer+1A9C cursor; after
// each B19000 reload count+1AA0 then data+1A9C to form the current end, while
// advancing the ORIGINAL cursor by2Ch. Callback-driven relocation of the old
// array must preserve readability just as the original native caller requires.
void refresh_native_renderer_material_textures_00b24dd0(void* actual_renderer,
    NativeRendererActivationBindings&);

// Complete normal control body B24FB0 for current slot120 target B24DD0.
// Sample canonical108D4BB once; capture actual_renderer once, including across
// callback changes to globalF8D394. Effects scan, CURRENT receiver table+120,
// then textures scan. Unknown current slot targets throw at that boundary;
// the explicit source interface is not a native ABI/general-vtable substitute.
void refresh_native_renderer_activation_00b24fb0(void* actual_renderer,
    NativeRendererActivationContext&);

// Recovered raw-domain focus chain from historical84c6fe880, reverified here.
// Existing RenderNodeRootList is a host reference view, not native root+0C.
// Supply actual service/root/node storage from the same production resources.
void unlink_native_raw_root_node_00b72220(void* actual_root, void* actual_node) noexcept;
// Partial B6D890 projection: ONLY requested_root=0, as fixed by B4ECC0.
// Nonzero-root registration/scene branches B6D8C6..B6D915 are excluded.
void clear_native_raw_node_root_00b6d890_null(void* actual_node) noexcept;
void mark_native_render_batch_dirty_00b50010(void* actual_batch) noexcept;
void invalidate_native_render_root_chain_00b4ecc0(void* actual_owner) noexcept;
// Complete B0D1E0 normal body: capture service, check+1C4, call B50010 on+20
// when nonnull, recheck+1C4, then current+30 and tail B4ECC0 when nonnull.
void refresh_native_render_service_focus_00b0d1e0(void* actual_service) noexcept;
} // namespace bsp
