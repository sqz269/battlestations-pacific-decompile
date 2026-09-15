#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_mesh_scalar_fields.hpp"
#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <atomic>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mesh texture field requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p, U offset = 0) noexcept { return ptr(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void put(void* p, U offset, U value) noexcept { *static_cast<volatile U*>(at(p, offset)) = value; }
void return_name(void* data, U bytes, NativeStringRawPoolContext& strings) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, bytes, strings.actual_small_returns_disabled_01090aa4);
}
void unwind_name(NativeMeshTextureFieldAcquired& a, NativeStringRawPoolContext& strings) noexcept {
    a.name_cleanup_armed = false;
    void* const data = ptr(word(&a.name, 4));
    if (data) return_name(data, word(&a.name) + 1u, strings);
    a.name_returned = true;
}
void unwind_child(NativeMeshTextureFieldAcquired& a, NativeAdoptedSubstreamDispatch& streams) noexcept {
    a.child_cleanup_armed = false;
    release_native_structured_node_handle_00be9ed0(&a.child, streams);
}
} // namespace

void set_native_material_texture_unchecked_00b189f0(void* material, U index,
    void* texture, NativeRenderActualOwners& owners) {
    const auto count = *static_cast<const volatile std::int16_t*>(at(material, 0x34));
    if (static_cast<U>(static_cast<std::int32_t>(count)) <= index)
        *static_cast<volatile std::uint16_t*>(at(material, 0x34)) = static_cast<std::uint16_t>(index + 1u);
    void* const slot = at(material, 0x10u + index * 4u);
    void* const old = ptr(word(slot));
    if (old == texture) return;
    put(slot, 0, bits(texture));
    if (texture) static_cast<std::atomic<std::int32_t>*>(at(texture, 4))->fetch_add(1, std::memory_order_seq_cst);
    if (old) release_native_render_actual_owner(owners, old);
}

void read_native_mesh_texture_field_00b93d30(void* material, void* parent,
    NativeMeshTextureFieldContext& c, NativeMeshTextureFieldAcquired& a) {
    using Phase = NativeMeshTextureFieldAcquired::Phase;
    if (a.phase != Phase::empty || a.texture || a.child || a.name_completed ||
        a.cache.phase != NativeTextureCacheAcquired::Phase::not_started)
        throw std::logic_error("Native mesh texture field cannot replay an acquired operation");
    a.phase = Phase::name; a.native_site = 0x00b93d55;
    (void)read_native_resource_handle_string_00bea010(parent, &a.name, c.reads);
    a.name_completed = true;
    // Original receiver/table/slot capture precedes state0. Unsupported profile
    // capture is an explicit source boundary; do not add name rollback here.
    void* const renderer = const_cast<void*>(c.textures.textures.current_renderer_00f8d394);
    if (word(renderer) != 0x00d5f0a8u || !c.renderer_profile_00d5f0a8 ||
        c.renderer_profile_00d5f0a8[0x64 / 4] != 0x00b319b0u)
        throw std::logic_error("Native mesh texture field requires current renderer slot64");
    a.name_cleanup_armed = true;
    try {
        a.phase = Phase::texture; a.native_site = 0x00b93d74;
        a.texture = load_native_renderer_texture_00b319b0(renderer, &a.name, 0, c.textures, &a.cache);
        a.captured_texture = a.texture;
        a.phase = Phase::slot; a.native_site = 0x00b93d7a;
        a.slot = read_native_resource_node_control_dword_00be99f0(parent, c.reads);
        a.phase = Phase::assignment; a.native_site = 0x00b93d85;
        set_native_material_texture_unchecked_00b189f0(material, a.slot, a.texture, c.owners);
        while (native_resource_node_has_remaining_00715bf0(parent)) {
            a.phase = Phase::child; a.native_site = 0x00b93d9c;
            create_native_resource_child_00bea680(parent, &a.child, c.reads);
            a.child_cleanup_armed = true;
            try {
                a.phase = Phase::child_fields;
                const auto* const name = static_cast<const char*>(ptr(word(a.child, 0x14)));
                if (name && _stricmp(name, "TextureAddress") == 0) {
                    a.native_site = 0x00b93dcc;
                    (void)read_native_resource_node_control_dword_00be99f0(&a.child, c.reads);
                    a.native_site = 0x00b93dd5;
                    (void)read_native_resource_node_control_dword_00be99f0(&a.child, c.reads);
                    a.native_site = 0x00b93dde;
                    (void)read_native_resource_node_control_dword_00be99f0(&a.child, c.reads);
                } else {
                    a.native_site = 0x00b93de9;
                    skip_native_resource_node_00be9c40(&a.child, c.reads);
                }
            } catch (...) {
                if (a.child_cleanup_armed) unwind_child(a, c.reads.streams);
                throw;
            }
            a.child_cleanup_armed = false;
            a.phase = Phase::child_release; a.native_site = 0x00b93df7;
            release_native_structured_node_handle_00be9ed0(&a.child, c.reads.streams);
        }
        a.phase = Phase::texture_release; a.native_site = 0x00b93e0b;
        void* const texture = a.texture;
        a.texture = nullptr;
        a.cache.caller_acquired = false; // Consumed bookkeeping, no extra release.
        release_native_render_actual_owner(c.owners, texture); // Native requires nonnull.
        void* const captured_data = ptr(word(&a.name, 4));
        a.name_cleanup_armed = false;
        a.phase = Phase::name_return; a.native_site = 0x00b93e40;
        if (captured_data) return_name(captured_data, word(&a.name) + 1u, c.reads.strings);
        a.name_returned = true;
    } catch (...) {
        if (a.name_cleanup_armed) unwind_name(a, c.reads.strings);
        throw;
    }
    a.phase = Phase::complete;
}
} // namespace bsp
