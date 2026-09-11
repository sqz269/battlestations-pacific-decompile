#pragma once

#include "bsp/gui_lua_reader.hpp"
#include <memory>
struct lua_State;
#include <string_view>

namespace bsp {

// Concrete host for the recovered reader, linked to the repository's Lua 5.1.1.
// Registry references are host handles, not the original 14h LuaObject ABI.
// A reader takes ownership of its root handle; destroy readers before this host.
// Globals use the untracked pseudo-index, as native00b67980 does. A balanced
// reader holding only globals may be destroyed after its borrowed Lua closes.
class GuiLua51Host final : public GuiLuaHost {
public:
    GuiLua51Host();
    explicit GuiLua51Host(lua_State& borrowed_state);
    ~GuiLua51Host() override;
    GuiLua51Host(const GuiLua51Host&) = delete;
    GuiLua51Host& operator=(const GuiLua51Host&) = delete;

    // Protected host entry for generated archive text. Opens no Lua libraries.
    // Returns the actual Lua load/runtime error; restores the interpreter stack.
    // This is not the native GUI loader's unprotected lua_call path.
    bool execute_archive(std::string_view text, std::string& error);
    // Host projection of an evaluated data table. Preserves lua_next string-key
    // order and shared subtable identity; owns strings/numbers after Lua closes.
    // Nil returns null. Rejects non-table roots, cycles, metatables, unsupported
    // value/key kinds and oversized/deep data instead of claiming raw-table parity.
    std::shared_ptr<const GuiTable> snapshot_table(const GuiLuaRef&);
    GuiLuaRef globals() override;
    GuiLuaRef get_by_name(const GuiLuaRef&, const char*) override;
    GuiLuaRef get_by_index(const GuiLuaRef&, std::int32_t) override;
    bool next(const GuiLuaRef&, GuiLuaRef&, GuiLuaRef&, bool restart) override;
    GuiLuaType type_of(const GuiLuaRef&) override;
    bool to_boolean(const GuiLuaRef&) override;
    double to_number(const GuiLuaRef&) override;
    const char* to_string(const GuiLuaRef&) override;
    void* to_userdata(const GuiLuaRef&) override;
    void release(const GuiLuaRef&) override;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace bsp
