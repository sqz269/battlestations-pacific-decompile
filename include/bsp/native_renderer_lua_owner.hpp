#pragma once

#include "bsp/native_lua_bootstrap.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// B32410 allocates exactly 4CCh and discards the returned pointer. The Lua
// owner is embedded at +4; its full 4C8h storage retains its native layout.
// Vtable DWORDs below are original identity values, not callable C++ vtables.
struct NativeRendererLuaOwnerStorage {
    std::uint32_t vtable_00;
    NativeLuaStateStorage lua_04;
};
static_assert(sizeof(NativeRendererLuaOwnerStorage) == 0x4cc);
static_assert(offsetof(NativeRendererLuaOwnerStorage, lua_04) == 4);

// Borrow the two distinct application publication cells and the real Lua
// providers. Both cells must outlive the owner. No private manager is created.
struct NativeRendererLuaOwnerContext {
    void* volatile& manager_publication_01090aa0;
    void* volatile& owner_publication_00f8d434;
    NativeStringStorage& strings;
    const NativeLuaBootstrapInputs& bootstrap;
};

// Original entry: ECX fresh 4CCh allocation, EAX same, plain RET. These new
// source interfaces explicitly pass the borrowed global/provider context.
NativeRendererLuaOwnerStorage* construct_native_renderer_lua_base_00b1ba30(
    void* fresh, NativeRendererLuaOwnerContext&);
NativeRendererLuaOwnerStorage* construct_native_renderer_lua_owner_00b1bb90(
    void* fresh, NativeRendererLuaOwnerContext&);

// B1BAD0/B1BBF0: ECX owner, plain RET, no semantic result. The base dtor
// unregisters the current publication, which need not still equal this.
// Normal C++ unwind invokes actual 411EE0 and 412430 cleanup providers;
// an exception escaping a cleanup during another unwind terminates.
void destroy_native_renderer_lua_base_00b1bad0(
    NativeRendererLuaOwnerStorage&, NativeRendererLuaOwnerContext&);
void destroy_native_renderer_lua_owner_00b1bbf0(
    NativeRendererLuaOwnerStorage&, NativeRendererLuaOwnerContext&);

// B1BB70/B1BC50: ECX owner, stack flags, EAX original pointer, RET4. Only
// low flag bit 0 requests the application's CRT free after destruction.
void* scalar_delete_native_renderer_lua_base_00b1bb70(
    NativeRendererLuaOwnerStorage*, std::uint8_t flags,
    NativeRendererLuaOwnerContext&);
void* scalar_delete_native_renderer_lua_owner_00b1bc50(
    NativeRendererLuaOwnerStorage*, std::uint8_t flags,
    NativeRendererLuaOwnerContext&);

} // namespace bsp
