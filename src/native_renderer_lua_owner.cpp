#include "bsp/native_renderer_lua_owner.hpp"

#include "bsp/singleton_lifetime.hpp"
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer Lua owner requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t base_vtable_00d5e5a4 = 0x00d5e5a4u;
constexpr std::uint32_t derived_vtable_00d5e5a8 = 0x00d5e5a8u;
constexpr std::uint32_t final_base_vtable_00ce3818 = 0x00ce3818u;
} // namespace

NativeRendererLuaOwnerStorage* construct_native_renderer_lua_base_00b1ba30(
    void* fresh, NativeRendererLuaOwnerContext& context) {
    auto* const owner = ::new (fresh) NativeRendererLuaOwnerStorage;
    owner->vtable_00 = base_vtable_00d5e5a4;

    const SoundLifetimeAccess actual_manager(context.manager_publication_01090aa0);
    CapturedSoundLifetimeSection guard(actual_manager); // first 415350, section +10
    context.owner_publication_00f8d434 = owner;
    // Resolve the manager first; only then reload the current publication.
    auto manager = actual_manager.get_manager_00415350();
    void* const current = context.owner_publication_00f8d434;
    manager->register_object(current);
    return owner;
}

NativeRendererLuaOwnerStorage* construct_native_renderer_lua_owner_00b1bb90(
    void* fresh, NativeRendererLuaOwnerContext& context) {
    auto* const owner = construct_native_renderer_lua_base_00b1ba30(fresh, context);
    try {
        owner->vtable_00 = derived_vtable_00d5e5a8;
        construct_native_lua_state_00b66bd0(&owner->lua_04);
        try {
            open_native_lua_state_00b6a020(
                owner->lua_04, 1u, context.strings, context.bootstrap);
        } catch (...) {
            // Native FH3 state 1 is armed after B66BD0 and closes +4.
            close_native_lua_state_00b669a0(owner->lua_04);
            throw;
        }
    } catch (...) {
        // Native FH3 state 0 is armed after the base constructor returns.
        destroy_native_renderer_lua_base_00b1bad0(*owner, context);
        throw;
    }
    return owner;
}

void destroy_native_renderer_lua_base_00b1bad0(
    NativeRendererLuaOwnerStorage& owner, NativeRendererLuaOwnerContext& context) {
    owner.vtable_00 = base_vtable_00d5e5a4;
    const SoundLifetimeAccess actual_manager(context.manager_publication_01090aa0);
    {
        CapturedSoundLifetimeSection guard(actual_manager);
        // Native B1BB26 gets the manager a second time; B1BB2B reloads F8D434.
        auto manager = actual_manager.get_manager_00415350();
        void* const current = context.owner_publication_00f8d434;
        manager->unregister_object(current);
        context.owner_publication_00f8d434 = nullptr;
    }
    owner.vtable_00 = final_base_vtable_00ce3818;
}

void destroy_native_renderer_lua_owner_00b1bbf0(
    NativeRendererLuaOwnerStorage& owner, NativeRendererLuaOwnerContext& context) {
    owner.vtable_00 = derived_vtable_00d5e5a8;
    try {
        close_native_lua_state_00b669a0(owner.lua_04);
    } catch (...) {
        destroy_native_renderer_lua_base_00b1bad0(owner, context);
        throw;
    }
    destroy_native_renderer_lua_base_00b1bad0(owner, context);
}

void* scalar_delete_native_renderer_lua_base_00b1bb70(
    NativeRendererLuaOwnerStorage* owner, std::uint8_t flags,
    NativeRendererLuaOwnerContext& context) {
    destroy_native_renderer_lua_base_00b1bad0(*owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

void* scalar_delete_native_renderer_lua_owner_00b1bc50(
    NativeRendererLuaOwnerStorage* owner, std::uint8_t flags,
    NativeRendererLuaOwnerContext& context) {
    destroy_native_renderer_lua_owner_00b1bbf0(*owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

} // namespace bsp
