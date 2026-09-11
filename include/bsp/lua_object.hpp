#pragma once
// The LuaObject API the datatable loaders share. See docs/LUA_OBJECT_API.md.
// Addresses: 00b65f50, 00b660a0, 00b66420, 00b669a0, 00b66bd0, 00b67080,
//            00b67190, 00b67700, 00b67800, 00b67980, 00b69d40, 00b6a020.
//
// Every name below is a hypothesis, not a recovered symbol. The live reference
// side of the API is already reconstructed as bsp::GuiLuaHost in
// include/bsp/gui_lua_reader.hpp and is reused here rather than duplicated;
// this header adds the native object layout, the state-owner lifetime, and the
// three call shapes every loader repeats (scan a table, read one field,
// release the temporary).

#include "bsp/gui_lua_reader.hpp"
#include "bsp/lua_numeric.hpp"

#include <cstdint>
#include <string>

namespace bsp {

// ---------------------------------------------------------------------------
// The 14h LuaObject, from the default constructor 00b65f50
// ---------------------------------------------------------------------------
//
// 00b65f50 is __thiscall(ECX = object), RET, and writes four of the five words:
//   00b65f54  [ECX+00] = 0          state
//   00b65f56  [ECX+04] = 0          kind
//   00b65f59  [ECX+08] = 0xffffffff reference
//   00b65f60  [ECX+10] = 0 (byte)   flag
// +0Ch is left as it was found, so a stack-built object carries whatever was
// in that slot. Every reader that matters writes +0Ch before reading it.

enum class LuaObjectKind : std::int32_t {
    // 00b66420 is exactly `kind == 0`, and 00b67190 leaves the pair in this
    // state when lua_next reports the end of a table.
    Unbound = 0,
    // A borrowed stack or pseudo index. 00b660a0 refuses to type-test one:
    // only kind 2 reaches lua_type. See include/bsp/gui_lua_runtime.hpp on the
    // globals pseudo-index alias 00b67980 builds.
    StackIndex = 1,
    // A tracked registry reference, released through 00b66de0 by the
    // destructor 00b67700. This is what 00b67800 and the iterators produce.
    Reference = 2,
};

struct LuaObject {
    const void* state{nullptr};                 // +00 lua_State*
    LuaObjectKind kind{LuaObjectKind::Unbound}; // +04
    std::int32_t reference{-1};                 // +08 registry ref, -1 when none
    std::uint32_t uninitialised_0c{0};          // +0C not written by 00b65f50
    bool flag_10{false};                        // +10 byte
};

// 00b65f50, __thiscall(ECX = object), RET.
void lua_object_construct_00b65f50(LuaObject& object) noexcept;

// 00b66420, __thiscall(ECX ignored, [esp+4] = object), RET 4, bool in AL.
// The ECX this is genuinely unused; both call sites in 00740840 pass the table
// they are iterating, which the routine never touches.
bool lua_object_is_unbound_00b66420(const LuaObject& object) noexcept;

// 00b660a0, __thiscall(ECX = object), RET, bool in AL. An unbound object is
// false without a library call; any kind other than 2 is false as well; kind 2
// calls lua_type (00a675a0) and compares it with 4, LUA_TSTRING. `type_of` is
// only consulted for kind 2, so a caller that has no live state can pass
// GuiLuaType::None.
bool lua_object_is_string_00b660a0(const LuaObject& object, GuiLuaType type) noexcept;

// ---------------------------------------------------------------------------
// The state owner, from 00b66bd0 and 00b669a0
// ---------------------------------------------------------------------------
//
// 00b66bd0 is __thiscall(ECX = owner), RET, over a 4C8h object: the owns byte
// at +00, the state at +04, +0Ch and +10h, then fifty 18h-byte slots starting
// at +28h (00b66be8 with the 18h stride and the 0x31 counter), and +4C4h.
// Only the first two fields decide the lifetime, which is what this models;
// the fifty slots are the owner's callback table and belong to a later packet.
// PcStorageLuaOwner in include/bsp/lua_state_owner.hpp is the concrete
// stock-Lua owner; this is the rule the native destructor path applies.

struct LuaStateOwnerLifetime {
    bool owns_state{false}; // +00, set when the owner created the state
    void* state{nullptr};   // +04
};

// 00b669a0, __thiscall(ECX = owner), RET. lua_close (00a68a90) runs only when
// the state is non-null AND the owns byte is set; the state word is cleared
// either way, so an owner that borrowed its state forgets it without closing
// it. The owns byte is NOT cleared, which is why a second close is harmless
// only because the state is already null.
bool lua_state_owner_should_close_00b669a0(const LuaStateOwnerLifetime& owner) noexcept;

struct LuaStateCloseHost {
    virtual ~LuaStateCloseHost() = default;
    // 00a68a90 lua_close.
    virtual void lua_close(void* state) = 0;
};

void lua_state_owner_close_00b669a0(LuaStateOwnerLifetime& owner, LuaStateCloseHost& host);

// ---------------------------------------------------------------------------
// The three call shapes every datatable loader repeats
// ---------------------------------------------------------------------------

// The native table walk: 00b67080 to restart (lua_pushnil then lua_next),
// 00b67190 to advance (lua_pushvalue then lua_next), and 00b66420 on the
// VALUE as the end test. 00740840 tests the value, not the key, at 007409a8
// and 00740d6d; a loader that tests the key instead would stop one element
// early on a table whose last key is unbound-shaped. Both temporaries are
// released by 00b67700 after the loop (00740d7a, 00740d8b).
class LuaTableScan {
public:
    LuaTableScan(GuiLuaHost& host, GuiLuaRef table);
    ~LuaTableScan();
    LuaTableScan(const LuaTableScan&) = delete;
    LuaTableScan& operator=(const LuaTableScan&) = delete;

    bool at_end() const noexcept { return at_end_; }
    const GuiLuaRef& key() const noexcept { return key_; }
    const GuiLuaRef& value() const noexcept { return value_; }
    void advance();

private:
    GuiLuaHost& host_;
    GuiLuaRef table_{};
    GuiLuaRef key_{};
    GuiLuaRef value_{};
    bool at_end_{true};
};

// One field read: 00b67800 (lua_gettop, lua_pushlstring, lua_gettable) into a
// temporary LuaObject, one accessor on the temporary, then 00b67700. The
// native sequence never tests presence, so a missing key reaches the accessor
// as nil: the number forms return 0.0f, the string form returns the empty
// string where the native returns the null lua_tolstring gives it.
float lua_field_number_00b66270(GuiLuaHost& host, const GuiLuaRef& table, const char* key);
std::int32_t lua_field_integer_00b66290(GuiLuaHost& host, const GuiLuaRef& table,
    const char* key, const bool& crt_sse2_conversion);
std::string lua_field_string_00b662b0(GuiLuaHost& host, const GuiLuaRef& table, const char* key);

// 00b67980 then 00b67800 then 00b67700, as 0074092f..00740965 runs it: take the
// globals pseudo-index object, look one name up in it, release the globals
// object and keep the result. The caller owns the returned reference.
GuiLuaRef lua_global_by_name_00b67980(GuiLuaHost& host, const char* name);

} // namespace bsp
