#pragma once
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_vfs_name_resolution.hpp"

namespace bsp::gui_text_backing_detail {
// Retained typed C++ backing only. Opaque operation base extents do not replace
// the caller's full derived/transitive extent contract. No native owner lookup.
template<class Region, class Check>
void shader_texture_backing(const NativeTextureCacheAcquired& a, Check check) {
    check(Region{&a, sizeof(a)});
    if (a.resolution.operation) check(Region{a.resolution.operation.get(), sizeof(*a.resolution.operation)});
    if (a.fallback_resolution.operation)
        check(Region{a.fallback_resolution.operation.get(), sizeof(*a.fallback_resolution.operation)});
    if (a.fallback) shader_texture_backing<Region>(*a.fallback, check);
}
template<class Region, class Check>
void shader_cache_backing(const NativeMaterialEffectCacheAcquired& a, Check check) {
    check(Region{&a, sizeof(a)});
    if (a.key_resolution) {
        const auto& k = *a.key_resolution;
        check(Region{&k, sizeof(k)});
        if (k.changed_resolution.acquired)
            check(Region{k.changed_resolution.acquired.get(), sizeof(*k.changed_resolution.acquired)});
        if (k.original_resolution.acquired)
            check(Region{k.original_resolution.acquired.get(), sizeof(*k.original_resolution.acquired)});
    }
    const auto& loader = a.loader;
    if (loader.name_resolution.acquired)
        check(Region{loader.name_resolution.acquired.get(), sizeof(*loader.name_resolution.acquired)});
    if (loader.programs) check(Region{loader.programs.get(), sizeof(*loader.programs)});
    if (loader.constructor_texture) shader_texture_backing<Region>(*loader.constructor_texture, check);
    if (loader.fallback) shader_cache_backing<Region>(*loader.fallback, check);
}
} // namespace bsp::gui_text_backing_detail
