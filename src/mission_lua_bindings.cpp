// Mission Lua bindings: the self table and the entity return convention.
//
// Packet cc_mission_natives. Ghidra read-only; every name is a hypothesis. The routines here
// are the pure rules of include/bsp/mission_lua_bindings.hpp. Nothing in this file touches a
// lua_State: the live plumbing belongs to the LuaObject layer (docs/LUA_OBJECT_API.md) and to
// the probe, which is the only consumer that owns a real state.
#include "bsp/mission_lua_bindings.hpp"

#include <cstdio>
#include <cstring>

namespace bsp {

// 00884240, `__cdecl void(char* dst, const char* fmt, ...)` calling _vsprintf_s(dst, 8, fmt, va)
// -- the 8 is the literal PUSH 0x8 at 00884250, not a sizeof. Its one caller
// 00885DA0 BSP_MissionLua_PushArgumentRecord passes "%d" (00CE3A34) and a 16-bit field, and
// 004260B0 BSP_NativeString_FromInt performs the same conversion on the binding side
// (00898FA7, 008989D6, 00928A2F). Both produce the decimal text of a zero-extended id, so the
// two paths agree on the key and at most five digits are ever written.
std::string mission_entity_self_key(std::uint16_t entity_id)
{
    char buffer[kSelfKeyBufferBytes];
    const int written = std::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(entity_id));
    if (written <= 0) {
        return std::string();
    }
    return std::string(buffer, static_cast<std::size_t>(written));
}

// The nineteen rows of kEntityReturningBindings. Linear because the table is short and the
// native has no lookup structure of its own: each handler simply ends in the same tail.
bool mission_binding_returns_entity(const char* name) noexcept
{
    if (name == nullptr) {
        return false;
    }
    for (std::size_t i = 0; i < kEntityReturningBindingCount; ++i) {
        if (std::strcmp(kEntityReturningBindings[i].name, name) == 0) {
            return kEntityReturningBindings[i].returns_entity;
        }
    }
    return false;
}

// 00B677B7..00B677D4, the non-kind-2 branch of 00B67720: the produced object is kind 2 with
// `slot = this->slot + index`. The frame object 00B679B0 builds carries slot 1, so index 0 is
// the first Lua argument. The kind-2 branch at 00B67735 is a genuine rawget-style table index
// and is deliberately not modelled here; no binding this packet read reaches it on a frame.
std::int32_t mission_binding_argument_slot(std::int32_t argument_index) noexcept
{
    return kBindingFirstArgumentSlot + argument_index;
}

// 00B66400: `lua_gettop(owner->state) - this->base`, where base is the entry top cached by
// 00B679B0 at +0Ch. A handler that pushed nothing returns 0, which is how the not-found path
// would report if it did not push nil first.
std::int32_t mission_binding_result_count(std::int32_t final_top,
                                          const MissionBindingCallFrame& frame) noexcept
{
    return final_top - frame.entry_top;
}

// 008878C7..008878D2: `EDI = arg1; if (EDI == 0) skip; if (*(int*)EDI == 0) skip`. The
// NativeString's size word at +0 is the test, not its data pointer -- a NativeString with a
// null data pointer but a non-zero size still pushes, and 008878ED substitutes the empty string
// at 00F87904 for the null pointer. Modelled with the same two conditions.
MissionNamedCallSelf mission_named_call_self(const std::string* self_key) noexcept
{
    MissionNamedCallSelf result;
    if (self_key == nullptr || self_key->empty()) {
        result.pushes_self = false;
        return result;
    }
    result.self_key = *self_key;
    result.pushes_self = true;
    return result;
}

// 004E0225 00B65FB0 over `globals["thisTable"]`, then the branch at 004E0249. 00B65FB0 answers
// true only for a bound object whose lua_type is LUA_TNIL (0): an unbound object yields
// LUA_TNONE (-1) and every other kind is hard-coded to LUA_TTABLE (5) at 00B65FE2, both false.
// So a table left over from a previous mission is kept, and 00928A00 clears slots one by one.
bool mission_self_table_needs_creation(bool global_is_nil) noexcept
{
    return global_is_nil;
}

} // namespace bsp
