// Mission Lua bindings: the self table `thisTable` and the entity return convention.
//
// Packet cc_mission_natives, worktree agent/cc-mission-natives. Ghidra was read-only.
// Every name in this header is a hypothesis, not a recovered symbol.
//
// docs/MISSION_LUA_MACHINE.md recovered the 560-row binding table at 00E0B7B8 and showed that
// six installed mission scripts fail under a stubbed probe because they index the result of a
// binding. This header records what those results actually are.
//
// The short version, all addresses evidenced in docs/MISSION_LUA_SELF_TABLE.md and
// docs/LUA_BINDING_ENTITY.md:
//
//   * A game entity reaches Lua as a plain **table** stored in the global `thisTable`, keyed by
//     the decimal-string form of the entity's 16-bit id at entity+174h. There is no userdata
//     proxy for the entity itself and no metatable on the table.
//   * The key string is cached on the entity at +178h as a NativeString (00928A23..00928A6F).
//   * 00928A00 creates the table and seeds `ID`, `Dead` and `Ptr`. `Ptr` is light userdata
//     holding the native entity pointer, which is what the already-reconstructed reverse
//     direction `object_from_lua_table_00888aa0` (include/bsp/object_handle_resolvers.hpp,
//     src/object_handle_resolvers.cpp) reads back. That function is reused, not restated here.
//   * Nineteen bindings return an entity by pushing `thisTable[key]` and nothing else.
//
// No struct, enum or k* constant declared here is declared by another header in include/bsp.
#ifndef BSP_MISSION_LUA_BINDINGS_HPP
#define BSP_MISSION_LUA_BINDINGS_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace bsp {

// ---------------------------------------------------------------------------
// The self table
// ---------------------------------------------------------------------------

// 00CE7494, the literal every one of the thirty-three referencing functions pushes.
// docs/MISSION_LUA_MACHINE.md's follow-up row called 00CE7494 a vtable; it is this string.
inline constexpr const char* kMissionSelfTableGlobal = "thisTable";

// 00CE74A0. BSP_Game_LoadMissionScene clears this global (004E0305 -> 00B67350, set-nil) on the
// same pass that creates `thisTable`, immediately after 005E2F00 publishes LobbySettings.
inline constexpr const char* kMissionReconGlobal = "recon";

// The three fields 00928A00 writes on a freshly created entity table, in the order it writes
// them. Everything else on an entity table is added later by a subsystem: 00440E10 adds `Class`.
inline constexpr const char* kEntitySelfFieldId = "ID";     // 00CE59B4, 00928B9C, a string
inline constexpr const char* kEntitySelfFieldDead = "Dead"; // 00CF829C, 00928BAA, a boolean
inline constexpr const char* kEntitySelfFieldPtr = "Ptr";   // 00CFAD08, 00928C12, light userdata
inline constexpr const char* kEntitySelfFieldClass = "Class"; // 00CE44C8, 00440ED2, a table

// The `Class` field is a row of a datatable-built global, not a native object, which is why
// `FindEntity(...).Class.Height` resolves in the shipped scripts. Which global depends on the
// entity kind, and each kind has its own setter:
//   * 009292B0 (00CE5880) sets `Class = VehicleClass[index]` -- ships and planes;
//   * 00440E10 (00CE44BC) sets `Class = DeviceClass[entity->[354h]->[6Ch]]` -- devices.
// Both reach the same 00B675D0 name-set-object on the object 00927B40 returns. Three further
// callers of 00B675D0 that push the literal "Class" were not read: 006E2E10, 004AED50 and the
// pair at 00742E76/00742EE4.
inline constexpr const char* kVehicleClassGlobal = "VehicleClass";
inline constexpr const char* kDeviceClassGlobal = "DeviceClass";

// Byte offsets on the native entity, from 00928A23 and 00928A36.
inline constexpr std::size_t kEntityIdOffset = 0x174;      // MOVZX EAX, word ptr [reg + 0x174]
inline constexpr std::size_t kEntityKeyStringOffset = 0x178; // LEA ESI, [EDI + 0x178]

// The key rule. 00884240 is `vsprintf_s(dst, 8, "%d", args)` over an 8-byte stack buffer and
// 004260B0 BSP_NativeString_FromInt is the NativeString form of the same conversion; both
// produce the decimal text of a zero-extended 16-bit id, so the key never exceeds five digits.
// Pure: no state, no allocation beyond the returned string.
std::string mission_entity_self_key(std::uint16_t entity_id);

// Capacity of the 00884240 buffer, which is what bounds the key. Five digits plus the
// terminator fit; the routine is safe for every u16 only because the id is zero-extended.
inline constexpr std::size_t kSelfKeyBufferBytes = 8; // PUSH 0x8 at 00884250

// ---------------------------------------------------------------------------
// The binding-table row, as config/lua_bindings.json carries it
// ---------------------------------------------------------------------------

// One row of 00E0B7B8. Eight bytes in the image: a name pointer then a handler pointer.
// tools/lua_bindings_export.py reads them from the PE; this is the in-memory projection.
struct MissionLuaBindingRow {
    const char* name;          // the Lua global the row installs
    std::uint32_t handler;     // the native lua_CFunction address
    bool returns_entity;       // the handler ends in the thisTable tail below
};

// The nineteen handlers whose bodies end in the entity tail. Established mechanically: every
// binding-table row whose handler pushes 00CE7494 into 00B67910 and then reaches 00B663D0.
// Ordered by handler address. `Effect` (008A9730) is **not** among them, correcting this
// packet's brief: 008A9730 takes a parameter table (00B67080 scan) and returns no entity.
inline constexpr MissionLuaBindingRow kEntityReturningBindings[] = {
    {"GetLastCatapulted", 0x00892860u, true},
    {"LaunchAirBaseSlot", 0x00896750u, true},
    {"CreateScript", 0x00898750u, true},
    {"FindEntity", 0x00898E30u, true},
    {"FindEntityByID", 0x008990B0u, true},
    {"GetFormationLeader", 0x00899AF0u, true},
    {"GetFireTarget", 0x0089C360u, true},
    {"GetSquadronPlane", 0x0089CFE0u, true},
    {"GetPlaneSquadron", 0x0089D250u, true},
    {"GetSquadronLandedBase", 0x0089D470u, true},
    {"UnitGetAttackTarget", 0x008A6DE0u, true},
    {"GetPrimaryTarget", 0x008A9460u, true},
    {"GetSelectedUnit", 0x008AB070u, true},
    {"GetTargetInfoTarget", 0x008C2C50u, true},
    {"GetDevice", 0x008C3610u, true},
    {"GetKamikazeByDummy", 0x008C3880u, true},
    {"GetPayload", 0x008CEFC0u, true},
    {"GetGun", 0x008CF350u, true},
    {"GenerateObject", 0x00944FD0u, true},
};

inline constexpr std::size_t kEntityReturningBindingCount = 19;

// True when the named binding returns an entity table by the tail rule above. Pure lookup over
// the table; unknown names answer false rather than throwing, because the caller is a script.
bool mission_binding_returns_entity(const char* name) noexcept;

// ---------------------------------------------------------------------------
// The entity tail, as a rule rather than as code
// ---------------------------------------------------------------------------

// What every one of the nineteen handlers does once it has a native entity pointer, and what a
// probe has to reproduce to make the shipped scripts load. The steps are the call sequence of
// 00898E30 from 00898F98 to 00899045, which 00898750 repeats verbatim from 008989C7.
enum class EntityPushStep {
    ReadId,          // 00898F98  MOVZX EAX, word ptr [entity + 0x174]
    FormatKey,       // 00898FA7  004260B0, the decimal text of the zero-extended id
    TakeSelfTable,   // 00898FC4  00B67910 -> 00B67800 with kind 3, so LUA_GLOBALSINDEX
    IndexByKey,      // 00898FDA  00B678E0 -> 00B67800 on the table object
    PushValue,       // 00898FE9  00B663D0: lua_checkstack(L,1) then lua_pushvalue(L, slot)
    PushNilInstead,  // 0089903C  00B66430: lua_pushnil, taken when the lookup produced nothing
    ReturnCount,     // 00899045  00B66400: lua_gettop(L) - frame.base
};

// The call-frame LuaObject 00B679B0 builds at the top of every binding: kind 3, slot base 1,
// and the entry `lua_gettop` cached at +0Ch. Two consequences the probe depends on:
//   * a name lookup on a kind-3 object resolves against LUA_GLOBALSINDEX (00B67804 CMP .. 3,
//     00B67876 MOV EDX, 0xFFFFD8EE), which is why `frame["thisTable"]` is the global;
//   * the return value is `lua_gettop(L) - base`, so a handler returns however many values it
//     left above the arguments, never a hard-coded count.
struct MissionBindingCallFrame {
    std::int32_t first_argument_slot; // 00B679C1, always 1
    std::int32_t entry_top;           // 00B679DA, lua_gettop at entry == the argument count
};

inline constexpr std::int32_t kBindingFirstArgumentSlot = 1;     // 00B679C1
inline constexpr std::int32_t kLuaGlobalsPseudoIndex = -10002;   // 0xFFFFD8EE, 00B67876

// The frame's indexed accessor 00B67720 on a non-kind-2 object is `slot = base + index`
// (00B677B7..00B677D4), so index 0 is the first Lua argument. Pure arithmetic; exposed because
// the shipped bindings read arguments by index and the probe must agree on the base.
std::int32_t mission_binding_argument_slot(std::int32_t argument_index) noexcept;

// Number of results a binding reports, given the stack top it leaves and the frame it opened.
// 00B66400, `lua_gettop(L) - frame.base`. Pure.
std::int32_t mission_binding_result_count(std::int32_t final_top,
                                          const MissionBindingCallFrame& frame) noexcept;

// ---------------------------------------------------------------------------
// Named calls into the scripts
// ---------------------------------------------------------------------------

// 00887750 BSP_MissionLuaHost_CallNamed, __thiscall with RET 1Ch: seven stack arguments, the
// first of which is a NativeString self key. When that key is non-empty the routine pushes
// `thisTable[key]` as the call's first argument (008878D4..0088790A: getglobal "thisTable",
// pushstring key, gettable -2, remove -2) and starts the argument count at one.
//
// This is what `this` is inside a mission script entry point: the entity's own table, the same
// object the entity-returning bindings hand back. The shipped scripts turn it into the global
// `Mission` with `Mission = this` on the first line of `luaInit`, where `luaInit` was registered
// by `CreateScript("luaInit")` from `luaStageInit`.
struct MissionNamedCallSelf {
    std::string self_key;    // empty means no self argument is pushed
    bool pushes_self;        // 008878CD/008878D2: non-null NativeString with a non-zero size
};

// The rule at 008878C7..008878D2, as a predicate. A null or zero-length key pushes nothing.
MissionNamedCallSelf mission_named_call_self(const std::string* self_key) noexcept;

// The four-step self push, for a host that has to reproduce it. Ordered as the native emits it.
enum class NamedCallSelfStep {
    GetGlobalSelfTable, // 008878E1  006B8460 lua_getglobal("thisTable")
    PushKey,            // 008878F6  006B8120 lua_pushstring(key), empty string when null
    GetTable,           // 00887900  006B8470 lua_gettable(-2)
    RemoveTable,        // 0088790A  006B7EA0 lua_remove(-2)
};

// ---------------------------------------------------------------------------
// Creating the table
// ---------------------------------------------------------------------------

// What 00928A00 does for one entity, in order. __thiscall(ECX = entity), sole caller 0077E830.
// The stale-slot clear is unconditional on a non-nil slot: the routine never reuses a table.
enum class SelfTableCreateStep {
    FormatKey,        // 00928A2F  004260B0 over the u16 at +174h
    CacheKeyOnEntity, // 00928A54  the NativeString at entity+178h
    ReadExistingSlot, // 00928A9F  00927B40, thisTable[entity->key]
    ClearStaleSlot,   // 00928AFF  00B67350, thisTable[key] = nil, only when the slot was not nil
    AssignFreshTable, // 00928B53  00B67580, thisTable[key] = {}
    SetId,            // 00928BA5  00B67630, self.ID = key (a string, not a number)
    SetDead,          // 00928BC7  00B673A0, self.Dead = false
    SetPtr,           // 00928C2F  00B67530, self.Ptr = lightuserdata(entity)
};

// 00927B40, the canonical self-object getter with twenty-five callers:
// `globals(owner at [00E188A8]+1A0Ch)["thisTable"][entity->keyString]`. Recorded as the rule the
// probe reproduces; the live LuaObject plumbing is docs/LUA_OBJECT_API.md's, not restated.
inline constexpr std::uint32_t kMissionLuaStateOwnerField = 0x1A0C; // game+1A0Ch, 004DFB70

// The guard BSP_Game_LoadMissionScene applies before creating the table (004E0225 00B65FB0,
// which is `lua_type(...) == LUA_TNIL` for a bound object and false for an unbound one):
// the global is only assigned a fresh table when it is currently nil, so a table that survived
// a previous mission is kept and the per-entity slots are cleared one at a time instead.
bool mission_self_table_needs_creation(bool global_is_nil) noexcept;

} // namespace bsp

#endif // BSP_MISSION_LUA_BINDINGS_HPP
