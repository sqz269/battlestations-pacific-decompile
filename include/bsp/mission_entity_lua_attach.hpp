#pragma once
// Mission entity Lua self-table attach. docs/MISSION_ENTITY_LUA_ATTACH.md.
//
// Addresses: 00928A00 00928C80 00928F50 0077E830 00779AF0 00951F30 00CE6290.
//
// Every descriptive name here is a hypothesis, not a recovered symbol, with two
// exceptions that are read out of the image: the `M...` class names listed in
// docs/VEHICLE_CLASS_DESCRIPTORS.md, and the Lua field names `ID`, `Dead`,
// `Ptr`, `LastPosition`, `x`, `y`, `z`, `Race`, `Party`, which are the string
// literals the routines push.
//
// The unit of reconstruction is a virtual slot, not a free function. The root
// entity vtable 00CE6290 holds, at three fixed indices:
//
//   slot 11 (+2Ch)  00928F50   set party and race, mirrored into the self table
//   slot 32 (+80h)  00928C80   on-killed: null `Ptr`, record `LastPosition`
//   slot 39 (+9Ch)  00928A00   attach or refresh the self table
//
// 0077E830 is an override of slot 39 that calls the base 00928A00 first; the
// nine functions that call 0077E830 are themselves slot-39 overrides in derived
// classes. So "which entities get a self table" is answered by the slot, not by
// a call graph: every class whose vtable carries any of these at +9Ch does.

#include <cstddef>
#include <cstdint>
#include <string>

#include "bsp/lua_object.hpp"
#include "bsp/mission_lua_bindings.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Virtual slot indices
// ---------------------------------------------------------------------------

// Read from the root entity vtable 00CE6290 and confirmed against 21 derived
// vtables (docs/MISSION_ENTITY_LUA_ATTACH.md, "The slot is uniform"): every
// attach installation sits at vtable+9Ch, every party/race at vtable+2Ch and
// every on-killed at vtable+80h.
inline constexpr std::size_t kEntityVtableAttachLuaSelfSlot = 39;  // +9Ch
inline constexpr std::size_t kEntityVtableOnKilledSlot = 32;       // +80h
inline constexpr std::size_t kEntityVtableSetPartyRaceSlot = 11;   // +2Ch

// ---------------------------------------------------------------------------
// Entity fields these three routines read
// ---------------------------------------------------------------------------

// Byte offsets taken from the listings named in each comment. This is not the
// full entity layout; only the fields the attach, kill and party/race paths
// touch are projected. kEntityIdOffset (174h) and kEntityKeyStringOffset (178h)
// are already declared in bsp/mission_lua_bindings.hpp and are reused, not
// restated.
inline constexpr std::size_t kEntityDescriptorOffset = 0xC0;    // 00928AA4, 0077E839
inline constexpr std::size_t kEntityDescriptorKindOffset = 0x04;  // 00928AB4, 0077E843
inline constexpr std::size_t kEntityDescriptorOwnerOffset = 0x08; // 0077E84C, 0077E87E
inline constexpr std::size_t kEntityPartyOffset = 0x54;          // 00929034
inline constexpr std::size_t kEntityRaceOffset = 0x58;           // 00928FC7
inline constexpr std::size_t kEntityPoseStaleOffset = 0xC8;      // 00928D86, byte
inline constexpr std::size_t kEntityPoseXOffset = 0xFC;          // 00928DA4 FLD
inline constexpr std::size_t kEntityPoseYOffset = 0x100;         // 00928D9C MOVSS
inline constexpr std::size_t kEntityPoseZOffset = 0x104;         // 00928DB4 MOVSS
inline constexpr std::size_t kEntityResourceUsageOffset = 0x304; // 0077E864

// The descriptor kind at descriptor+4h. Only the two values the packet read are
// named; the descriptor is the object 00964790 hands back for a unit class, so
// other kinds exist and are deliberately left unnamed.
enum class EntityDescriptorKind : std::int32_t {
    // 0077E843 CMP dword ptr [EAX + 0x4],0x1. Gates the ResourceUsage property
    // read that 0077E830 performs after the base attach.
    ResourceOwner = 1,
    // 00928AB4 CMP dword ptr [EAX + 0x4],0x3, JZ 00928BEE. Gates out the whole
    // table reset in 00928A00.
    ScriptOwned = 3,
};

// What the three routines need from the entity. `descriptor_present` is the
// `!= 0` test at 00928AA4/0077E839, kept separate from the kind because a null
// descriptor and a descriptor of an unnamed kind take the same branch in
// 00928A00 but different branches in 0077E830.
struct MissionEntityLuaRecord {
    std::uint16_t network_id{0};          // +174h, the key source
    bool descriptor_present{false};       // +C0h != 0
    std::int32_t descriptor_kind{0};      // +C0h -> +4h
    const void* descriptor_owner{nullptr}; // +C0h -> +8h, ECX of both property calls
    bool key_string_empty{true};          // +178h size == 0 (00928C9B gate)
    bool pose_stale{false};               // +C8h byte == 0 means refresh first
    float pose_x{0.0F};                   // +FCh
    float pose_y{0.0F};                   // +100h
    float pose_z{0.0F};                   // +104h
    std::int32_t party{0};                // +54h
    std::int32_t race{0};                 // +58h
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 00928AAC..00928AB8. True when the entity keeps whatever Lua object it already
// owns: the stale-slot clear, the fresh-table assign, `ID` and `Dead` are all
// jumped over and only `Ptr` is rebound. A missing descriptor takes the normal
// path, which is why the null test is separate.
bool mission_entity_keeps_existing_self_table(bool descriptor_present,
                                              std::int32_t descriptor_kind) noexcept;

// 0077E839..0077E847. True when the 0077E830 override goes on to read the
// `ResourceUsage` property after the base attach has run.
bool mission_entity_reads_resource_usage(bool descriptor_present,
                                         std::int32_t descriptor_kind) noexcept;

// 0077E86A..0077E876. FLDZ / FUCOMIP / LAHF / TEST AH,0x44 / JNP is an ordered
// "not equal" over the value just stored at +304h, so a NaN also takes the
// call. Expressed as `!(value == 0)` rather than `value != 0` to keep that.
bool mission_entity_resource_usage_needs_apply(float resource_usage) noexcept;

// The decimal key. 00928A2F calls 004260B0 BSP_NativeString_FromInt on the
// zero-extended u16 at +174h; that is the same conversion the binding side
// already models, so this forwards to mission_entity_self_key rather than
// declaring a second rule.
std::string mission_entity_lua_key(std::uint16_t network_id);

// ---------------------------------------------------------------------------
// Host: one method per native call site
// ---------------------------------------------------------------------------

// No method has a default implementation and none stands in for unrecovered
// behaviour. Lua 5.1.1, the STL and the CRT are contracts here, not ports: the
// LuaObject operations below are the native 00B67xxx family described in
// docs/LUA_OBJECT_API.md, and the string work is NativeString.
struct MissionEntityLuaAttachHost {
    virtual ~MissionEntityLuaAttachHost() = default;

    // --- 00928A00, the base attach ---------------------------------------
    // 00928A1E, 00927050. Runs before the key is formatted; its body was not
    // read by this packet, so it is named by address in the doc's host table.
    virtual void prepare_00927050() = 0;
    // 00928A2F, 004260B0 BSP_NativeString_FromInt over the u16 at +174h.
    virtual std::string format_key(std::uint16_t network_id) = 0;
    // 00928A54/00928A6A, 0041DD40 + memcpy into the NativeString at +178h.
    virtual void cache_key_on_entity(const std::string& key) = 0;
    // 00928A9F, 00927B40, ECX = entity, [esp+4] = out. Returns the LuaObject
    // for globals["thisTable"][key]; every field write below targets it.
    virtual LuaObject& self_object() = 0;
    // 00928AC2, 00B65FB0 BSP_LuaObject_IsNil on that object.
    virtual bool self_object_is_nil(const LuaObject& self) = 0;
    // 00928ADC/00928AF2/00928AFF: globals (00B67980) -> "thisTable"
    // (00B67800) -> set the key to nil (00B67350).
    virtual void clear_stale_slot(const std::string& key) = 0;
    // 00928B30/00928B46/00928B53: the same walk, then 00B67580 new table.
    virtual void assign_fresh_table(const std::string& key) = 0;
    // 00928BA5, 00B67630 -> lua_pushstring. `ID` is the key text, a string.
    virtual void set_id_string(LuaObject& self, const std::string& key) = 0;
    // 00928BC7, 00B673A0 -> lua_pushboolean(0).
    virtual void set_dead_false(LuaObject& self) = 0;
    // 00928C2F, 00B67530 -> lua_pushlightuserdata(entity).
    virtual void set_ptr_lightuserdata(LuaObject& self, const void* entity) = 0;

    // --- 0077E830, the override tail --------------------------------------
    // 0077E857, 0048F5D0, ECX = descriptor+8h, one float default pushed and the
    // name "ResourceUsage". Returns the property value on the x87 stack.
    virtual float read_resource_usage_property(const void* descriptor_owner,
                                               float fallback) = 0;
    // 0077E886, 0043D870, ECX = descriptor+8h, the same literal. Callee body
    // unread by this packet; named by address in the doc.
    virtual void apply_resource_usage_property(const void* descriptor_owner) = 0;

    // --- 00928C80, on-killed ----------------------------------------------
    // 00928CAA 004C1570, then EnterCriticalSection through [00CE2218] at
    // 00928CC3, with the recursion counter at +18h bumped by hand (00928CC9).
    // Leave is [00CE2210] at 00928ED2, counter at 00928ECE.
    virtual void enter_entity_lock() = 0;
    virtual void leave_entity_lock() = 0;
    // 00928D04, 00B67530 with a literal 0 pushed instead of the entity.
    virtual void set_ptr_null(LuaObject& self) = 0;
    // 00928D49 00B67580 on "LastPosition", then 00928D81 00B67800 to bind it.
    virtual LuaObject& make_last_position_table(LuaObject& self) = 0;
    // 00928D97, 00414DB0, taken when the byte at +C8h is zero (00928D86).
    virtual void refresh_world_pose() = 0;
    // 00928DE6, 00928E32, 00928E7E: 00B67400 (number setter) on "x", "y", "z".
    virtual void set_number(LuaObject& table, const char* name, float value) = 0;
    // 00928F14, 004254B0 "Entity %u killed: %s (%s)". Outside the lock and
    // outside the self-table gate: it runs even when the entity has no key.
    virtual void log_entity_killed(std::uint16_t network_id) = 0;

    // --- 00928F50, party and race -----------------------------------------
    // 00928F7D, 00923B80 with the three forwarded arguments; the base
    // implementation this override extends. Body unread by this packet.
    virtual void base_set_party_race_00923b80(std::int32_t a, std::int32_t b,
                                              std::int32_t c) = 0;
    // 00928FD9 and 00929046: 00B67460 (integer setter) on "Race" and "Party".
    virtual void set_integer(LuaObject& self, const char* name,
                             std::int32_t value) = 0;
};

// ---------------------------------------------------------------------------
// The routines
// ---------------------------------------------------------------------------

// 00928A00, __thiscall(ECX = entity), RET, body 00928A00..00928C79, root entity
// vtable 00CE6290 slot 39. Coverage: complete.
void mission_entity_attach_lua_self_00928a00(const MissionEntityLuaRecord& record,
                                             const void* entity,
                                             MissionEntityLuaAttachHost& host);

// 0077E830, __thiscall(ECX = entity), RET, body 0077E830..0077E88D. Calls the
// base above, then the ResourceUsage step. Coverage: complete. `resource_usage`
// carries back what was stored at +304h so a caller can see it.
void mission_entity_attach_lua_self_0077e830(const MissionEntityLuaRecord& record,
                                             const void* entity,
                                             MissionEntityLuaAttachHost& host,
                                             float* resource_usage);

// 00928C80, __thiscall(ECX = entity), RET, body 00928C80..00928F44, root vtable
// slot 32. Coverage: partial. The Lua half is complete; the tail past the log
// line (the +30h virtual at vtable+134h and 00923050) is left to the caller
// because neither callee was read.
void mission_entity_on_killed_00928c80(const MissionEntityLuaRecord& record,
                                       MissionEntityLuaAttachHost& host);

// 00928F50, __thiscall(ECX = entity, three stack arguments), RET 0Ch, body
// 00928F50..00929090, root vtable slot 11, reached through the adjustor thunk
// 00951F30 in classes with a second vptr. Coverage: complete.
void mission_entity_set_party_race_00928f50(const MissionEntityLuaRecord& record,
                                            MissionEntityLuaAttachHost& host,
                                            std::int32_t a, std::int32_t b,
                                            std::int32_t c);

}  // namespace bsp
