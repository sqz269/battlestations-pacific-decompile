#include "bsp/native_material_texture_refresh.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
template<class T> T current(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p) + offset);
}
U count(const std::int16_t& value) noexcept {
    return static_cast<U>(static_cast<std::int32_t>(
        *reinterpret_cast<const volatile std::int16_t*>(&value)));
}
void extend_count(std::int16_t& value, U index) noexcept {
    if (index >= count(value))
        *reinterpret_cast<volatile std::uint16_t*>(&value) = static_cast<std::uint16_t>(index + 1u);
}
void retain_actual(void* identity) noexcept {
    auto* references = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    references->fetch_add(1, std::memory_order_seq_cst);
}
} // namespace

NativeMaterialTextureNameOperation::~NativeMaterialTextureNameOperation() {
    if (phase == Phase::running || phase == Phase::failed || globals_live || value_live)
        std::terminate();
}

NativeString* resolve_native_renderer_lua_string_00b1bc70(
    NativeRendererLuaOwnerStorage& owner, void* fresh_output, const NativeString& key,
    NativeStringStorage& strings, const NativeMaterialTextureNameLiterals& literals,
    NativeMaterialTextureNameOperation& a) {
    using Phase = NativeMaterialTextureNameOperation::Phase;
    if (a.phase != Phase::fresh)
        throw std::logic_error("material texture-name operation cannot replay");
    if (!literals.nil_00f8d438 || !literals.non_string_00ce3a0c)
        throw std::invalid_argument("material texture-name literals are required");
    a.phase = Phase::running;
    a.output = static_cast<NativeString*>(fresh_output);
    try {
        a.native_site = 0x00b1bc99;
        native_lua_globals_00b67980(owner.lua_04, &a.globals);
        a.globals_live = true;
        a.native_site = 0x00b1bcb2;
        // Same key.data capture and null -> empty C-string behavior as B68100.
        // Protect this existing B67800 provider without introducing a Lua frame.
        const char* const text = current<const char*>(&key, 4);
        native_lua_get_by_name_protected(a.globals, &a.value, text ? text : "");
        a.value_live = true;
        a.native_site = 0x00b1bcc0;
        destroy_native_lua_object_00b67700(a.globals);
        a.globals_live = false;
        a.native_site = 0x00b1bcc9;
        const bool nil = native_lua_is_nil_00b65fb0(a.value);
        a.output_started = true;
        if (nil) {
            a.native_site = 0x00b1bcdd;
            ::new (fresh_output) NativeString;
            a.output->assign_0041e870(strings, literals.nil_00f8d438);
        } else {
            a.native_site = 0x00b1bcee;
            native_lua_string_or_00b685c0(a.value, fresh_output,
                literals.non_string_00ce3a0c, strings);
        }
        a.native_site = 0x00b1bd04;
        destroy_native_lua_object_00b67700(a.value);
        a.value_live = false;
        a.output_returned = true;
        a.phase = Phase::complete;
        return a.output;
    } catch (...) {
        a.phase = Phase::failed;
        throw;
    }
}

NativeMaterialTextureRefreshOperation::~NativeMaterialTextureRefreshOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
    for (const auto& step : steps)
        if (step.selected_live || step.resolved_live || step.returned_reference_live)
            std::terminate();
}

void refresh_native_material_textures_00b19000(NativeMaterialEffectBaseStorage& effect,
    NativeMaterialTextureRefreshContext& c, NativeMaterialTextureRefreshOperation& a) {
    using Phase = NativeMaterialTextureRefreshOperation::Phase;
    if (a.phase != Phase::fresh)
        throw std::logic_error("material texture refresh cannot replay");
    if (&c.cache.strings != &c.cache.textures.strings || c.cache.textures.cache != &c.cache)
        throw std::invalid_argument("material texture refresh requires the canonical cache/string domain");
    a.phase = Phase::running;
    try {
        if (current<std::uint16_t>(&effect.name_count_94) != 0) {
            do {
                if (a.index >= effect.names_3c.size())
                    throw std::logic_error("material texture refresh index exceeds eleven actual slots");
                auto& step = a.steps[a.index];
                step.selected_live = true; // native zeroed8h selected local
                extend_count(effect.name_count_94, a.index);
                a.native_site = 0x00b19071;
                auto* const lua_owner = static_cast<NativeRendererLuaOwnerStorage*>(
                    c.current_lua_owner_00f8d434);
                resolve_native_renderer_lua_string_00b1bc70(*lua_owner,
                    &step.resolved, effect.names_3c[a.index], c.cache.strings, c.literals, step.name);
                step.resolved_live = true;
                if (current<U>(&step.resolved) != 0) {
                    a.native_site = 0x00b1908a;
                    copy_native_string_header_00be0a30_fragment(&step.selected,
                        c.cache.strings, &step.resolved);
                } else {
                    extend_count(effect.name_count_94, a.index);
                    a.native_site = 0x00b190c5;
                    copy_native_string_header_00be0a30_fragment(&step.selected,
                        c.cache.strings, &effect.names_3c[a.index]);
                }
                if (current<U>(&step.selected) != 0) {
                    auto* const renderer = const_cast<void*>(c.cache.textures.current_renderer_00f8d394);
                    a.native_site = 0x00b190fb;
                    if (!renderer || current<U>(renderer) != 0x00d5f0a8u ||
                        !c.renderer_profile_00d5f0a8 ||
                        c.renderer_profile_00d5f0a8[0x64 / 4] != 0x00b319b0u)
                        throw std::logic_error("material refresh requires current renderer64 B319B0");
                    void* const incoming = load_native_renderer_texture_00b319b0(
                        renderer, &step.selected, 0, c.cache, &step.texture);
                    step.returned_texture = incoming;
                    step.returned_reference_live = incoming != nullptr;
                    extend_count(effect.texture_count_38, a.index);
                    void* const old = current<void*>(&effect.textures_0c[a.index]);
                    if (old != incoming) {
                        *reinterpret_cast<void* volatile*>(&effect.textures_0c[a.index]) = incoming;
                        if (incoming) { a.native_site = 0x00b19122; retain_actual(incoming); }
                        if (old) {
                            a.native_site = 0x00b19130;
                            release_native_render_actual_owner(c.actual_owners, old);
                        }
                    }
                    if (incoming) {
                        a.native_site = 0x00b1914a;
                        release_native_render_actual_owner(c.actual_owners, incoming);
                        step.returned_reference_live = false;
                        step.texture.caller_acquired = false;
                    }
                }
                step.resolved_live = false;
                a.native_site = 0x00b1917d;
                destroy_native_string_header_0041dd20(&step.resolved, c.cache.strings);
                step.selected_live = false;
                a.native_site = 0x00b191a4;
                destroy_native_string_header_0041dd20(&step.selected, c.cache.strings);
                ++a.index;
            } while (a.index < count(effect.name_count_94));
        }
        a.phase = Phase::complete;
    } catch (...) {
        a.phase = Phase::failed;
        throw;
    }
}
} // namespace bsp
