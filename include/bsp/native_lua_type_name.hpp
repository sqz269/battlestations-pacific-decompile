#pragma once

#include <array>
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeLuaObjectStorage;

enum class NativeLuaTypeNameRole : std::uint32_t {
    none, nil, boolean, light_userdata, number, string, table, function,
    userdata, thread, unknown
};
struct NativeLuaTypeNameSpan {
    const char* data;
    std::uintptr_t original_address;
    std::uint32_t extent_including_nul;
    NativeLuaTypeNameRole role;
};

// Immutable metadata over the SAME actual raw cells and eleven actual RO
// spans from the caller's existing verified mapping. Role/address/extent and
// fixed pointer identity are checked; literal contents/provenance are NOT
// synthesized or proved here. No payload is read until selected real41E870.
class NativeLuaTypeNameContext final {
public:
    NativeLuaTypeNameContext(NativeStringRawPoolContext&,
        const std::array<NativeLuaTypeNameSpan, 11>& actual_spans);
    NativeLuaTypeNameContext(const NativeLuaTypeNameContext&) = delete;
    NativeLuaTypeNameContext& operator=(const NativeLuaTypeNameContext&) = delete;
    NativeLuaTypeNameContext(NativeLuaTypeNameContext&&) = delete;
    NativeLuaTypeNameContext& operator=(NativeLuaTypeNameContext&&) = delete;
private:
    friend void* construct_native_lua_type_name_00b68460(
        void*, std::int32_t, const NativeLuaTypeNameContext&);
    NativeStringRawPoolContext& strings_;
    const std::array<NativeLuaTypeNameSpan, 11> spans_;
};

// Fresh writable raw8, SAME real constructor/pool and caller-owned failure
// debt. Unsigned DWORD(type+1)<=9 selects -1..8; otherwise actual unknown.
// Actual D62C2C is "bool". No prezero, copied literal bank, Frame or cleanup.
// Ordinary Source ABI, not Original ECX/EDX/EAX/bare-RET/FH3/runtime proof.
void* construct_native_lua_type_name_00b68460(void* fresh8, std::int32_t type,
    const NativeLuaTypeNameContext&);

// Kind+4 ONCE:0=>-1;2=>CURRENTowner, state+4, index+8, linkedlua_type ONCE;
// all other kinds=>5 with no owner/state/index/interpreter query. Caller keeps
// genuine object/owner/rawLua/output/cells/RO/code alive, with legal disjoint
// aliases/no invalidating writers and actual Lua nonlocal/failure contracts.
void* construct_native_lua_object_type_name_00b68550(
    const NativeLuaObjectStorage&, void* fresh8, const NativeLuaTypeNameContext&);
} // namespace bsp
