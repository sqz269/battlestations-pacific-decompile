// Entity lookup by name (0088B1B0) and the two mission bindings that return an entity table:
// 00898E30 FindEntity and 00944FD0 GenerateObject.
//
// Addresses: 0088b1b0 00898e30 00944fd0 00944e0a 004c6ba0 004c6be0 00888760 0088b840
// Evidence:  docs/LUA_BINDING_ENTITY_LOOKUP.md, docs/LUA_BINDING_GENERATE_OBJECT.md
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The lookup is modelled as
// a pure rule over an injected view of the world's bucket lists; the two bindings are modelled as
// sequence routines over a host with one virtual method per native call site. Lua 5.1.1, the STL
// and the CRT are contracts, not ports.
#ifndef BSP_LUA_BINDING_ENTITY_LOOKUP_HPP
#define BSP_LUA_BINDING_ENTITY_LOOKUP_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace bsp {

// ---------------------------------------------------------------------------
// The world's bucket lists
// ---------------------------------------------------------------------------

// 0088B1B0 walks `[[00E188A8]+19CCh]`, the world node of docs/GAME_WORLD_CONSTRUCT.md, through an
// array of intrusive list bases. The base layout is the producer's, 00484540 push-back:
// {count +0h, head +4h, tail +8h}, node {prev +0h, next +4h, value +8h}.
// 0088B203 reads `[world + kind*0Ch + 1Ch]`, which is the *head* field of the base at
// `world + 18h + (kind + 6)*0Ch`.
inline constexpr std::uint32_t kWorldBucketArrayBase = 0x18;  // 00484540 call sites, e.g. 006FE62C
inline constexpr std::uint32_t kWorldBucketStride = 0x0C;     // 0088B2C7 ADD EBP, 0xC
inline constexpr std::uint32_t kWorldBucketHeadField = 0x04;  // 0088B203 vs. the base offsets
inline constexpr std::int32_t kWorldBucketFirstKind = -6;     // 0088B1D8 MOV EBX, 0xFFFFFFFA
inline constexpr std::int32_t kWorldBucketLastKind = 0x5A;    // 0088B2CA CMP EBX, 0x5B / JL

// Byte offset of a kind's list base on the world node. Pure arithmetic.
std::uint32_t world_bucket_base_offset(std::int32_t kind) noexcept;

// Byte offset of a kind's head pointer, the dword 0088B203 actually loads.
std::uint32_t world_bucket_head_offset(std::int32_t kind) noexcept;

// ---------------------------------------------------------------------------
// The kind jump table at 0088B310
// ---------------------------------------------------------------------------

// 0088B1E0 `CMP EBX,0x47 / JA` is unsigned, so every kind outside 0..47h skips without touching
// the byte table: the six negative kinds and 48h..5Ah are searched by nothing. The byte table at
// 0088B318 selects target 0 (0088B1F7, walk) or 1 (0088B2C4, skip) for the remaining 72 kinds.
// The fourteen walk kinds are listed in kSearchedEntityKinds.
inline constexpr std::int32_t kEntityKindJumpTableLimit = 0x47;  // 0088B1E0
inline constexpr std::size_t kSearchedEntityKindCount = 14;
extern const std::int32_t kSearchedEntityKinds[kSearchedEntityKindCount];

// The predicate the jump table implements.
bool entity_kind_is_searched(std::int32_t kind) noexcept;

// ---------------------------------------------------------------------------
// The four flag bytes
// ---------------------------------------------------------------------------

// 0088B213..0088B235, in the native's order. +5Ch must be set, the other three must be clear.
// Meanings from the producers: 00922FD0 (the release marker) writes +5Eh = 1, +5Dh = 1, +5Ch = 0
// and +6Ch = 1 down the child chain; 00903670 queues a node whose +5Eh is set and +6Ch clear;
// 00904BF0 gates the per-frame update on +5Ch. +60h has no producer read in this packet.
inline constexpr std::uint32_t kEntityFlagActive = 0x5C;         // required non-zero
inline constexpr std::uint32_t kEntityFlagReleased = 0x5D;       // required zero
inline constexpr std::uint32_t kEntityFlagReleaseRequested = 0x5E;  // required zero
inline constexpr std::uint32_t kEntityFlagUnreadGate = 0x60;     // required zero, meaning unread

// One candidate as 0088B210..0088B242 sees it: the four gate bytes plus the name the entity's own
// vtable slot +10h returns. `name == nullptr` is the native's null name; the accessor is the same
// virtual docs/SCENE_ENTITY_FACTORY.md reads a parent's name through.
struct MissionEntityCandidate {
    const void* entity = nullptr;  // the list node's value at +8h, what the routine returns
    const char* name = nullptr;    // (*(*entity + 10h))(), 0088B23D..0088B242
    bool active = false;           // byte +5Ch
    bool released = false;         // byte +5Dh
    bool release_requested = false;  // byte +5Eh
    bool unread_gate = false;      // byte +60h
};

// 0088B213..0088B235 as a predicate.
bool mission_entity_candidate_passes_flags(const MissionEntityCandidate& candidate) noexcept;

// The name test, 0088B244..0088B2B7. A null wanted name matches a null *or empty* candidate name
// (0088B24C..0088B25B measures the candidate and requires length zero). A non-null wanted name
// matches a null candidate name only when the NativeString's length field is zero
// (0088B29E CMP dword ptr [ESP+0x24], EAX with EAX == 0), and otherwise compares case-insensitively.
bool mission_entity_name_matches(const char* wanted,
                                 std::int32_t wanted_length,
                                 const char* candidate_name) noexcept;

// ---------------------------------------------------------------------------
// The lookup itself
// ---------------------------------------------------------------------------

// The injected view of the world's lists: one sequence of candidates per kind, in list order.
struct MissionEntityListView {
    virtual ~MissionEntityListView() = default;
    // Number of nodes on the bucket for `kind`; 0 for a kind the caller does not populate.
    virtual std::size_t bucket_size(std::int32_t kind) const = 0;
    // Node `index` of that bucket, head first, following `next` at +4h.
    virtual MissionEntityCandidate bucket_entry(std::int32_t kind, std::size_t index) const = 0;
};

// 0088B1B0 as a rule: the first candidate, in kind order then list order, that passes the flags
// and matches the name. Returns nullptr when nothing matches, exactly as the native's 0088B2FD.
// The native also frees the NativeString argument it was given by value; that ownership transfer
// is described in the doc and is not part of the rule.
const void* mission_entity_find_by_name(const MissionEntityListView& lists,
                                        const char* wanted,
                                        std::int32_t wanted_length) noexcept;

// ---------------------------------------------------------------------------
// The two bindings, as sequences over one host per native call site
// ---------------------------------------------------------------------------

// Which creator 00944FD0 reaches, decided at 00945193..0094522E.
enum class GenerateObjectForm {
    NamedOnly,     // 00945308, 004C6BA0 -> 0046D930: no position, no heading
    NamedPlaced,   // 009452CA, 004C6BE0 -> 0046DC10: position table and heading
};

// The ordered call sites of 00898E30 FindEntity, argument decode through the entity tail.
enum class FindEntityStep {
    OpenStateOwner,   // 00898EF0  00B66C00
    OpenCallFrame,    // 00898F09  00B679B0, kind 3, slot base 1
    OpenArgument0,    // 00898F20  00B677E0
    ReadArgument0,    // 00898F2F  00B662B0, lua_tolstring; a non-string yields nothing, no error
    FindByName,       // 00898F8B  0088B1B0, the NativeString passed by value and freed by it
    PushEntityTable,  // 00898F98..00899045, EntityPushStep of mission_lua_bindings.hpp
    PushNil,          // 0089903C  00B66430, taken when 0088B1B0 returned zero
};

// The ordered call sites of 00944FD0 GenerateObject.
enum class GenerateObjectStep {
    OpenStateOwner,      // 00944FD0 prologue -> 00B66C00
    OpenCallFrame,       // 00945076  00B679B0
    ReadObjectName,      // 009450A0  00B662B0 on argument 0
    TestArgument1String, // 009450E6  00B660A0 lua_type == LUA_TSTRING
    ReadSecondName,      // 00945136  00B662B0 on argument 1, else a copy of argument 0
    TestPositionTable,   // 009451B7  0088B840, "argument 1 is a table of three numbers"
    ReadHeading,         // 00945265  00B66270, the optional number after the position table
    ReadPosition,        // 009452BC  00888760, three floats out of the table into a Vector3
    CreatePlaced,        // 009452CA  004C6BE0 -> 0046DC10
    CreateNamedOnly,     // 00945308  004C6BA0 -> 0046D930
    PumpDeferredWork,    // 00945311  00874D00, with CL = 1
    PushEntityTable,     // 00945316..00945367, the same tail as FindEntity
    ResolveDeferred,     // 009453BB  0046AAB0
};

// 0094522E MOVSS XMM0,[00CE38B8]: the heading used when no number argument follows. The stored
// float is 10.0f, which is a sentinel rather than an angle: 0046DD4C compares the value against
// the double at 00CE3828 (2*pi) and 0046DD87 skips the rotation when the value is not smaller,
// so the default leaves the authored orientation alone. A value below 2*pi is a yaw in radians,
// applied through 00467050(frame, 0.0f, yaw, 0.0f) onto the placement frame.
inline constexpr float kGenerateObjectDefaultHeading = 10.0f;          // 00CE38B8
inline constexpr double kGenerateObjectHeadingSentinelFloor = 6.283185482025146;  // 00CE3828

// One virtual per native call site the two bindings make outside the shared Lua object plumbing.
// Contracts are the callee bodies', see the docs; the sequence routines below never interpret a
// return value the native does not interpret.
struct LuaBindingEntityLookupHost {
    virtual ~LuaBindingEntityLookupHost() = default;

    // 00B663F0 on the call frame: the number of Lua arguments. Index i exists iff i < count
    // (00945193 CMP EAX,1/JLE for index 1, 00945220 CMP EAX,2/JLE for index 2).
    virtual std::int32_t argument_count() = 0;
    // 00B662B0 BSP_LuaObject_GetString on argument `index`; empty when the slot is not a string.
    virtual std::string argument_string(std::int32_t index) = 0;
    // 00B660A0 BSP_LuaObject_IsString on argument `index`.
    virtual bool argument_is_string(std::int32_t index) = 0;
    // 0088B840 on argument `index`: a table whose iteration yields exactly three entries.
    virtual bool argument_is_position_table(std::int32_t index) = 0;
    // 00B66270 BSP_LuaObject_GetNumber on argument `index`.
    virtual float argument_number(std::int32_t index) = 0;

    // 0088B1B0 BSP_MissionEntity_FindByName.
    virtual const void* find_entity_by_name(const std::string& name) = 0;
    // 004C6BA0 -> 0046D930, __thiscall(game), RET 0Ch.
    virtual const void* create_named_object(const std::string& name,
                                            const std::string& second_name) = 0;
    // 004C6BE0 -> 0046DC10, __thiscall(game), RET 10h. `position_index` is the argument slot the
    // three floats come from; the heading is the native's fourth dword, a yaw in radians unless
    // it is at or above kGenerateObjectHeadingSentinelFloor.
    virtual const void* create_placed_object(const std::string& name,
                                             const std::string& second_name,
                                             std::int32_t position_index,
                                             float heading) = 0;
    // 00874D00 with CL = 1, the deferred-work pump.
    virtual void pump_deferred_work(bool flag) = 0;
    // 0046AAB0 BSP_SceneDatabase_ResolveDeferredReferences.
    virtual void resolve_deferred_references() = 0;

    // The entity tail, EntityPushStep of mission_lua_bindings.hpp: push thisTable[decimal id].
    virtual void push_entity_table(const void* entity) = 0;
    // 00B66430 lua_pushnil.
    virtual void push_nil() = 0;
    // 00B66400, lua_gettop(L) - frame.base.
    virtual std::int32_t result_count() = 0;
};

// 00898E30 in order. Returns what the native returns, the binding's result count.
std::int32_t lua_binding_find_entity(LuaBindingEntityLookupHost& host);

// 00944FD0 in order. `out_form` receives which creator the arguments selected.
std::int32_t lua_binding_generate_object(LuaBindingEntityLookupHost& host,
                                         GenerateObjectForm* out_form = nullptr);

// The argument-shape decision of 0094518A..00945229, exposed so it can be checked without a host.
// `argument_count` is 00B663F0 on the frame; index 0 is the first Lua argument.
GenerateObjectForm generate_object_form(std::int32_t argument_count,
                                        bool argument1_is_position_table) noexcept;

// The argument index the position table is read from, once the form is NamedPlaced.
// 00945210 / 00945229: 1 when argument 1 was the table, 2 when argument 1 was the second name.
std::int32_t generate_object_position_index(bool argument1_is_position_table) noexcept;

}  // namespace bsp

#endif  // BSP_LUA_BINDING_ENTITY_LOOKUP_HPP
