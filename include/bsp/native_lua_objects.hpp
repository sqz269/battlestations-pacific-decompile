#pragma once
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
struct lua_State;

namespace bsp {
struct NativeLuaStateStorage;
// Actual14h object. This owns a position on the Lua STACK, not a registry ref.
// Addresses remain stable while tracked. No implicit cleanup or initialization.
struct NativeLuaObjectStorage {
    NativeLuaStateStorage* owner_00;
    std::int32_t kind_04,index_08;
    std::uint32_t opaque_0c;
    std::uint8_t tracked_10;
    std::byte padding_11[3];
};
struct NativeLuaTrackedSlot {
    NativeLuaObjectStorage* references_00[5];
    std::int32_t count_14;
};
// Full4C8h owner layout. The constructor clears the50 COUNTS at28+18*i;
// their five reference pointers begin at14+18*i and retain their preimage.
struct NativeLuaStateStorage {
    std::uint8_t owns_00;
    std::byte padding_01[3];
    lua_State* state_04;
    std::uint32_t opaque_08;
    std::int32_t stack_offset_0c;
    std::uint32_t opaque_10;
    NativeLuaTrackedSlot slots_14[50];
    std::int32_t high_water_4c4;
};
static_assert(sizeof(NativeLuaObjectStorage)==0x14);
static_assert(offsetof(NativeLuaObjectStorage,tracked_10)==0x10);
static_assert(sizeof(NativeLuaTrackedSlot)==0x18);
static_assert(sizeof(NativeLuaStateStorage)==0x4c8);
static_assert(offsetof(NativeLuaStateStorage,slots_14)==0x14);
static_assert(offsetof(NativeLuaStateStorage,high_water_4c4)==0x4c4);

// Full original bodies, explicit C++ interfaces over actual storage. The
// linked Lua5.1.1 API is a library dependency, not reimplemented game code.
NativeLuaObjectStorage* construct_native_lua_object_00b65f50(void* fresh) noexcept;
NativeLuaStateStorage* construct_native_lua_state_00b66bd0(void* fresh) noexcept;
// B66C00, ECX fresh owner, stack borrowed lua_State*, EAX same, RET4.
// Native installs00B69E00 as global DoFile with ZERO upvalues. Supply that
// application's real callback; no default/no-op/VFS substitution is provided.
NativeLuaStateStorage* construct_native_lua_borrowed_state_00b66c00(
    void* fresh,lua_State&,int (*do_file_00b69e00)(lua_State*));
// B66C60: luaL_loadstring of name.data (empty if null), pcall(0,-1,0) on
// successful load only. Preserve all result/error stack values; return status.
int execute_native_lua_string_00b66c60(NativeLuaStateStorage&,const NativeString&);
// No blanket reference invalidation. Close only nonnull state when owns!=0,
// then clear state04 unconditionally, leaving ownership/tracking metadata stale.
void close_native_lua_state_00b669a0(NativeLuaStateStorage&);
NativeLuaObjectStorage* native_lua_globals_00b67980(NativeLuaStateStorage&,void* fresh);
// B67800 always uses the supplied owner, even for kind0. Kind3 reads globals;
// all other kinds use current index08 AFTER pushing the C-string key. Output
// is a tracked kind2 object at actual top. Unchecked slots<50/refs<5 are caller
// contracts; lua_checkstack's return is ignored just as in the original.
// This raw entry retains Lua's nonlocal error transfer, including calls inside
// existing lua_pcall callbacks. It does not convert Lua errors to C++ values.
NativeLuaObjectStorage* native_lua_get_by_name_00b67800(
    NativeLuaObjectStorage& table,void* fresh,const char* key);
// Source error transport for the explicit C++ reader boundary below. This is
// a linked Lua status, not the original CRT exception or retained error value.
struct NativeLuaOperationError final : std::exception {
    explicit NativeLuaOperationError(int value) noexcept : status(value) {}
    const char* what() const noexcept override { return "native Lua named lookup failed"; }
    int status;
};
// Explicit C++ reader adapter: protects checkstack/push/gettable in the SAME Lua C
// frame. On Lua error it restores the entry stack, publishes no output, and
// throws NativeLuaOperationError after Lua restores the frame. Inherits the
// current error handler; consumes its error value. Caller objects/interpreter
// must remain stable, including across metamethod/error-handler execution.
// Do not use inside a Lua C callback that expects its caller's lua_pcall to
// receive the Lua error. It is selected only by the base/Particle C++ readers.
// Other primitives still have their existing Lua nonlocal-transfer contract.
NativeLuaObjectStorage* native_lua_get_by_name_protected(
    NativeLuaObjectStorage& table,void* fresh,const char* key);
NativeLuaObjectStorage* native_lua_get_by_string_00b68100(
    NativeLuaObjectStorage& table,void* fresh,const NativeString& key);
bool native_lua_is_number_00b66050(const NativeLuaObjectStorage&);
float native_lua_number_00b66270(const NativeLuaObjectStorage&);
std::int32_t native_lua_integer_00b66290(const NativeLuaObjectStorage&,const bool& crt_sse2_conversion);
// Tracked byte0 is a no-op even with a closed/null owner. Otherwise remove
// first matching pointer by last-swap. A zero refcount can pop/remove index,
// shift subsequent slot records and decrement EVERY moved object's index.
// High-water and stale pointer cells are NOT cleared/decremented.
void release_native_lua_tracked_object_00b66de0(
    NativeLuaStateStorage*,NativeLuaObjectStorage&,std::int32_t index,std::uint8_t remove_stack);
// Kind0 skips everything; otherwise release using captured owner/index and
// then clear CURRENT kind04 only. The other16 bytes remain unchanged.
void destroy_native_lua_object_00b67700(NativeLuaObjectStorage&);
// B67690, ECX destination, stack source, EAX destination, RET4. Release a
// bound destination BEFORE reading source; copy owner/kind/index/tracked only.
// Register the destination's actual address when tracked. No self-copy guard:
// self-assignment can clear kind and re-register stale tracked metadata.
NativeLuaObjectStorage* assign_native_lua_object_00b67690(
    NativeLuaObjectStorage& destination,const NativeLuaObjectStorage& source);
// Full native predicates/getters over the actual stack object. B661B0 treats
// every nonzero kind other than 2 as a table without accessing the interpreter.
bool native_lua_is_boolean_00b66000(const NativeLuaObjectStorage&);
// B65FB0 returns false for unbound/non-reference kinds, even unbound kind0.
bool native_lua_is_nil_00b65fb0(const NativeLuaObjectStorage&);
// B66250 always calls lua_toboolean at the current owner/index; no kind gate.
bool native_lua_boolean_00b66250(const NativeLuaObjectStorage&);
bool native_lua_is_table_00b661b0(const NativeLuaObjectStorage&);
bool native_lua_is_integer_number_00b66a60(const NativeLuaObjectStorage&);
const char* native_lua_string_00b662b0(const NativeLuaObjectStorage&);
std::uint8_t native_lua_boolean_or_00b662f0(const NativeLuaObjectStorage&,std::uint8_t fallback);
bool native_lua_is_unbound_00b66420(const NativeLuaObjectStorage&) noexcept;
// ECX table; stack key/value; RET8. Existing value is released before key.
// Results keep word0C/padding unchanged and register their actual addresses.
// Next consumes a top key in place, or copies then removes a non-top key.
void native_lua_iterate_first_00b67080(NativeLuaObjectStorage& table,
    NativeLuaObjectStorage& key,NativeLuaObjectStorage& value);
void native_lua_iterate_next_00b67190(NativeLuaObjectStorage& table,
    NativeLuaObjectStorage& key,NativeLuaObjectStorage& value);
// Exact NUMBER or the full signed fallback DWORD; unlike integer coercion,
// numeric strings take the fallback. ECX object, stack fallback, EAX, RET4.
std::int32_t native_lua_integer_or_00b66380(const NativeLuaObjectStorage&,
    std::int32_t fallback,const bool& crt_sse2_conversion);
// Exact STRING or supplied C-string fallback, then construct fresh8h output.
// ECX object, stack output/fallback, EAX output, RET8; no prior-output cleanup.
NativeString* native_lua_string_or_00b685c0(const NativeLuaObjectStorage&,
    void* fresh,const char* fallback,NativeStringStorage&);
bool native_lua_is_string_00b660a0(const NativeLuaObjectStorage&);
// Kind2/exact NUMBER yields a float32-rounded result; every other value
// returns the supplied float. Original ECX object, stack float, ST0, RET4.
float native_lua_number_or_00b66330(const NativeLuaObjectStorage&,float fallback);
// B679B0: kind3, start index1, top in opaque0C, untracked; RET4/EAXoutput.
NativeLuaObjectStorage* native_lua_call_frame_00b679b0(NativeLuaStateStorage&,void* fresh);
// B67720: kind2 table lookup pushes integer key and registers actual output;
// every other kind creates untracked kind2 at current base index+offset.
NativeLuaObjectStorage* native_lua_get_by_index_00b67720(NativeLuaObjectStorage&,void* fresh,std::int32_t);
// Full32-byte wrapper, RET8/EAXoriginal output pointer.
NativeLuaObjectStorage* native_lua_argument_at_00b677e0(NativeLuaObjectStorage&,void* fresh,std::int32_t);
} // namespace bsp
