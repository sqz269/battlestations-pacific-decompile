#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include <atomic>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource remap requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
static_assert(sizeof(std::atomic<std::int32_t>) == 4);
void* at(const void* owner, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
Word word(const void* location) noexcept {
    return *static_cast<const volatile Word*>(location);
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void put(void* location, void* value) noexcept {
    *static_cast<volatile Word*>(location) = static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void release_captured(void* service, Word offset, void* captured,
    NativeRenderRemapDecrement decrement, NativeRenderActualOwners& owners) {
    if (!captured) return;
    if (!decrement) throw std::invalid_argument("unbound current CE2220 decrement target");
    if (decrement(static_cast<volatile long*>(at(captured, 4))) == 0) {
        auto& reference = owners.resolve_actual(captured);
        auto* const count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(captured, 4)));
        if (&reference.reference_count != count)
            throw std::invalid_argument("remap companion must borrow captured actual+04");
        // Canonical companion validates current profile/slot0 and its refreshed
        // deleting slot, then performs the real terminal without a decrement.
        reference.release_zero_references();
    }
    put(at(service, offset), nullptr); // Also overwrites a returning callback write.
}
void* set_remap(void* service, void* name, Word offset, Word entry,
    NativeRenderResourcesRemapContext& c, NativeRenderResourcesRemapAcquired& a) {
    if (a.phase != NativeRenderResourcesRemapAcquired::Phase::fresh || a.failed ||
        a.cache.phase != NativeTextureCacheAcquired::Phase::not_started || a.cache.wrapper_started)
        throw std::logic_error("remap setter cannot replay an acquired operation");
    try {
        a.phase = NativeRenderResourcesRemapAcquired::Phase::release;
        a.captured_old = pointer(word(at(service, offset)));
        if (a.captured_old) {
            a.native_site = entry + 0x12;
            const auto decrement = c.decrement_iat_00ce2220;
            release_captured(service, offset, a.captured_old, decrement, c.owners);
        }
        a.phase = NativeRenderResourcesRemapAcquired::Phase::load;
        a.native_site = entry + 0x2e;
        void* const renderer = const_cast<void*>(c.textures.textures.current_renderer_00f8d394);
        if (!renderer || word(renderer) != 0x00d5f0a8u ||
            !c.renderer_profile_00d5f0a8 || c.renderer_profile_00d5f0a8[0x64 / 4] != 0x00b319b0u)
            throw std::invalid_argument("unsupported current remap renderer slot64");
        a.native_site = entry + 0x40;
        a.loaded = load_native_renderer_texture_00b319b0(renderer, name, 0, c.textures, &a.cache);
        put(at(service, offset), a.loaded);
        a.phase = NativeRenderResourcesRemapAcquired::Phase::complete;
        return a.loaded;
    } catch (...) { a.failed = true; throw; }
}
} // namespace

void* set_native_render_remap_texture0_00b0fd70(void* s, void* n,
    NativeRenderResourcesRemapContext& c, NativeRenderResourcesRemapAcquired& a) {
    return set_remap(s, n, 0x66c, 0x00b0fd70, c, a);
}
void* set_native_render_remap_texture1_00b0fdc0(void* s, void* n,
    NativeRenderResourcesRemapContext& c, NativeRenderResourcesRemapAcquired& a) {
    return set_remap(s, n, 0x670, 0x00b0fdc0, c, a);
}
void* set_native_render_remap_texture2_00b0fe10(void* s, void* n,
    NativeRenderResourcesRemapContext& c, NativeRenderResourcesRemapAcquired& a) {
    return set_remap(s, n, 0x674, 0x00b0fe10, c, a);
}
void* set_native_render_remap_texture3_00b0fe60(void* s, void* n,
    NativeRenderResourcesRemapContext& c, NativeRenderResourcesRemapAcquired& a) {
    return set_remap(s, n, 0x678, 0x00b0fe60, c, a);
}
void* native_post_effect_material_00b4cba0(const void* owner) noexcept {
    return pointer(word(at(owner, 0x14)));
}
void reset_native_render_remap_textures_00b0feb0(void* service,
    NativeRenderResourcesRemapContext& c) {
    for (const Word offset : {0x65cu, 0x658u, 0x654u, 0x650u}) {
        void* const owner = pointer(word(at(service, offset)));
        if (owner) {
            void* const material = native_post_effect_material_00b4cba0(owner);
            set_native_material_texture_unchecked_00b189f0(material, 2, nullptr, c.owners);
        }
    }
    void* const first = pointer(word(at(service, 0x66c)));
    const auto decrement = c.decrement_iat_00ce2220; // B0FF24, after first capture.
    release_captured(service, 0x66c, first, decrement, c.owners);
    for (const Word offset : {0x670u, 0x674u, 0x678u})
        release_captured(service, offset, pointer(word(at(service, offset))), decrement, c.owners);
}
} // namespace bsp
