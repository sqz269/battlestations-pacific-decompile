#include "bsp/native_renderer_lua_owner.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <exception>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer Lua owner requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t base_vtable_00d5e5a4 = 0x00d5e5a4u;
constexpr std::uint32_t derived_vtable_00d5e5a8 = 0x00d5e5a8u;
struct NativeGuard {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(NativeGuard) == 8 && offsetof(NativeGuard, section_04) == 4);

volatile std::uint32_t& depth(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}

// FH3 state 0 for both B1BA30 and B1BAD0 calls this exact 412430 provider.
struct ProfileUnwindCleanup {
    NativeRendererLuaOwnerStorage& owner;
    bool armed = true;
    ~ProfileUnwindCleanup() noexcept {
        if (armed) destroy_native_generic_singleton_base_00412430(&owner);
    }
};

// FH3 state 1 uses the captured eight-byte guard, not a current manager reload.
struct GuardUnwindCleanup {
    NativeGuard& guard;
    bool armed = true;
    ~GuardUnwindCleanup() noexcept {
        if (armed) {
            try { destroy_native_singleton_guard_00411ee0(&guard); }
            catch (...) { std::terminate(); }
        }
    }
};

struct BaseOwnerUnwindCleanup {
    NativeRendererLuaOwnerStorage& owner;
    NativeRendererLuaOwnerContext& context;
    bool armed = true;
    ~BaseOwnerUnwindCleanup() noexcept {
        if (armed) {
            try { destroy_native_renderer_lua_base_00b1bad0(owner, context); }
            catch (...) { std::terminate(); }
        }
    }
};

struct LuaUnwindCleanup {
    NativeLuaStateStorage& lua;
    bool armed = true;
    ~LuaUnwindCleanup() noexcept {
        if (armed) {
            try { close_native_lua_state_00b669a0(lua); }
            catch (...) { std::terminate(); }
        }
    }
};

NativeGuard enter_first_section(void* manager) {
    auto* const section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        depth(section) = depth(section) + 1u;
    }
    return guard;
}

void leave_first_section(GuardUnwindCleanup& cleanup) {
    auto* const section = cleanup.guard.section_04;
    if (section) {
        depth(section) = depth(section) - 1u;
        LeaveCriticalSection(section);
    }
    cleanup.armed = false;
}
} // namespace

NativeRendererLuaOwnerStorage* construct_native_renderer_lua_base_00b1ba30(
    void* fresh, NativeRendererLuaOwnerContext& context) {
    auto* const owner = ::new (fresh) NativeRendererLuaOwnerStorage;
    ProfileUnwindCleanup profile_cleanup{*owner}; // state 0 before first 415350
    owner->vtable_00 = base_vtable_00d5e5a4;

    void* const first_manager = get_native_singleton_manager_00415350(
        context.manager_publication_01090aa0);
    NativeGuard guard = enter_first_section(first_manager);
    GuardUnwindCleanup guard_cleanup{guard}; // state 1 after enter/depth
    context.owner_publication_00f8d434 = owner;
    // Resolve the manager first; only then reload the current publication.
    void* const manager = get_native_singleton_manager_00415350(
        context.manager_publication_01090aa0);
    void* const current = context.owner_publication_00f8d434;
    register_native_singleton_object_00bd0c30(manager, nullptr, current);
    leave_first_section(guard_cleanup);
    profile_cleanup.armed = false;
    return owner;
}

NativeRendererLuaOwnerStorage* construct_native_renderer_lua_owner_00b1bb90(
    void* fresh, NativeRendererLuaOwnerContext& context) {
    auto* const owner = construct_native_renderer_lua_base_00b1ba30(fresh, context);
    BaseOwnerUnwindCleanup base_cleanup{*owner, context}; // B1BB90 state 0
    owner->vtable_00 = derived_vtable_00d5e5a8;
    construct_native_lua_state_00b66bd0(&owner->lua_04);
    LuaUnwindCleanup lua_cleanup{owner->lua_04}; // B1BB90 state 1
    open_native_lua_state_00b6a020(
        owner->lua_04, 1u, context.strings, context.bootstrap);
    lua_cleanup.armed = false;
    base_cleanup.armed = false;
    return owner;
}

void destroy_native_renderer_lua_base_00b1bad0(
    NativeRendererLuaOwnerStorage& owner, NativeRendererLuaOwnerContext& context) {
    owner.vtable_00 = base_vtable_00d5e5a4;
    ProfileUnwindCleanup profile_cleanup{owner}; // B1BAD0 state 0
    void* const first_manager = get_native_singleton_manager_00415350(
        context.manager_publication_01090aa0);
    NativeGuard guard = enter_first_section(first_manager);
    GuardUnwindCleanup guard_cleanup{guard}; // B1BAD0 state 1
    // Native B1BB26 gets the manager a second time; B1BB2B reloads F8D434.
    void* const manager = get_native_singleton_manager_00415350(
        context.manager_publication_01090aa0);
    void* const current = context.owner_publication_00f8d434;
    unregister_native_singleton_object_00bcfca0(manager, nullptr, current);
    context.owner_publication_00f8d434 = nullptr;
    leave_first_section(guard_cleanup);
    destroy_native_generic_singleton_base_00412430(&owner);
    profile_cleanup.armed = false;
}

void destroy_native_renderer_lua_owner_00b1bbf0(
    NativeRendererLuaOwnerStorage& owner, NativeRendererLuaOwnerContext& context) {
    owner.vtable_00 = derived_vtable_00d5e5a8;
    BaseOwnerUnwindCleanup base_cleanup{owner, context}; // B1BBF0 state 0
    close_native_lua_state_00b669a0(owner.lua_04);
    base_cleanup.armed = false; // state -1 before explicit base destructor
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
