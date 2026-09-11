#pragma once

#include "bsp/storage_backend.hpp"
#include <string>

namespace bsp {

// Explicit inputs to the recovered bootstrap. The byte string is the contents
// cached by 00884770/00b68340, not the path or a substitute fundamentals script.
// DoFile must implement 00b69e00's script-with-overrides contract on this state.
// It returns zero on success and may use lua_error; it must not throw C++.
struct LuaStateOwnerEnvironment {
    std::string fundamentals;
    int (*do_file)(lua_State*, void*) noexcept{};
    void* do_file_context{};
    bool x360comp{};
    std::string region;

    // Host adaptation for registry-backed readers: retire their references
    // while the interpreter remains valid. Native untracked globals need no
    // such callback. Observer/context must outlive this owner's open state.
    void (*before_close)(lua_State*, void*) noexcept{};
    void* close_context{};
};

// Concrete stock-Lua-5.1.1 implementation of the storage's owner boundary.
// New C++ interface, not the native 4C8h layout or register ABI. Native open
// 00b6a020 is __thiscall(mask), RET4; close00b65e80 takes ECX, RET.
class PcStorageLuaOwner final : public PcStorageLuaHost {
public:
    explicit PcStorageLuaOwner(LuaStateOwnerEnvironment);
    ~PcStorageLuaOwner() override;
    PcStorageLuaOwner(const PcStorageLuaOwner&) = delete;
    PcStorageLuaOwner& operator=(const PcStorageLuaOwner&) = delete;

    // Always opens base; remaining bits follow table00d62bb8 (see GuiLuaLibrary).
    // Requires a closed owner. Bootstrap failure closes and throws the Lua
    // error through a protected host boundary; the native path would panic.
    void open_storage_archive_00b6a020(std::uint32_t library_flags) override;
    lua_State* storage_lua_38() noexcept override { return state_; }
    void close_storage_archive_00b65e80() noexcept override;
    int initial_stack_top_08() const noexcept { return initial_stack_top_; }

private:
    static int bootstrap(lua_State*);
    static int do_file(lua_State*);
    LuaStateOwnerEnvironment environment_;
    lua_State* state_{};
    std::uint32_t library_flags_{};
    int initial_stack_top_{};
};

} // namespace bsp
