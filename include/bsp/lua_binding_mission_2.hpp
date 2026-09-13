// Packet cc_lua_find_entity: what makes `FindEntity` answer.
//
// docs/LUA_BINDING_ENTITY_LOOKUP.md reconstructed both halves of the binding
// itself: `0088B1B0`, the walk over the world buckets, and `00898E30`, which
// turns the entity it found into `thisTable[<decimal of entity+174h>]`. Neither
// half explains why the binding answered `nil` for every one of the 38 calls
// `usn_2_java.lua` makes: the lookup and the tail are both correct, and the
// *slot they read* did not exist for the entity the mission asks for.
//
// docs/MISSION_ENTITY_LUA_ATTACH.md established that the slot is built by entity
// virtual 39 (`+9Ch`) and closed with "the host has to reach slot 39 for it",
// leaving open which native pass does. This header answers that: `00925F20`
// `BSP_SEntity_InitAll`, the routine whose own literal at `00D19244` is
// "SEntity::InitAll", walks the pending-entity list five times and calls slot 39
// on **every** node in its first pass, at `0092604E`, with no class filter.
//
// Addresses: 00925F20 0057C1A0 0077F090 00922F30 00922F80 00807A50 004B8490
//
// Hypotheses, not recovered symbols. Read from the stored Ghidra listing of
// `00925F20` (`00925F20..0092638A`) and from the shipped image's bytes for the
// class vtables. Not a drop-in binary replacement: this is a new C++ contract
// over the native call sites.

#ifndef BSP_LUA_BINDING_MISSION_2_HPP
#define BSP_LUA_BINDING_MISSION_2_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// BSP_SEntity_InitAll 00925F20
// ---------------------------------------------------------------------------
//
// `__fastcall void(char force_recon_refresh)`, `RET`, body 00925F20..0092638A.
// The argument is the low byte only (`00925F45 MOV byte ptr [ESP+8],CL`) and is
// read once, at the very end (`00926367 CMP byte ptr [ESP+0x10],0x0`).

// The pending list and its count. `[00F899D0]` is the sentinel of a circular
// list whose nodes are `{next +0h, prev +4h, value +8h}`; `[00F899D4]` is the
// count. 00926335..00926365 empties the list and zeroes the count, so a second
// InitAll with nothing pending returns at 00925F49 without a pass.
inline constexpr std::uint32_t kSEntityPendingListAddress = 0x00F899D0u;   // 00925FA9
inline constexpr std::uint32_t kSEntityPendingCountAddress = 0x00F899D4u;  // 00925F3B
// Set before pass B and cleared before pass D (00926082, 0092623B). Nothing in
// this packet reads it; it is recorded as a write with an unread meaning.
inline constexpr std::uint32_t kSEntityInitActiveFlagAddress = 0x00F899A5u;
// The one-time NativeString init at 00925F5F, guarded by bit 0 of 00F899F8.
inline constexpr const char* kSEntityInitAllLabel = "SEntity::InitAll";  // 00D19244
// 00926089, formatted by 004B8490 (a vsprintf_s wrapper) with the count.
inline constexpr const char* kSEntityInitEnumFormat = "INIT,ENUM:%d";  // 00D19234

// The entity virtual slots the routine drives. `+9Ch` is the Lua self-table
// attach docs/MISSION_ENTITY_LUA_ATTACH.md names slot 39; `+A0h` and `+A4h` are
// the two slots after it, called by the second and third passes, and neither
// body was read by this packet.
inline constexpr std::size_t kEntityNameVtableSlot = 0x10;         // 00925FFB
inline constexpr std::size_t kEntityAttachLuaVtableSlot = 0x9C;    // 0092604E
inline constexpr std::size_t kEntityInitSecondVtableSlot = 0xA0;   // 0092610A
inline constexpr std::size_t kEntityInitThirdVtableSlot = 0xA4;    // 00926194
inline constexpr std::size_t kEntityEnableVtableSlot = 0x68;       // 009261C9
inline constexpr std::size_t kEntityDisableVtableSlot = 0x6C;      // 00926200
inline constexpr std::size_t kEntitySessionGateVtableSlot = 0x5C;  // 00926283

// The spawn descriptor at entity+C0h, the same field 00928A00 tests for kind 3.
// Pass C takes its enable/disable arm only for kind 2 (00926131 MOV EBP,2, then
// 009261AD CMP dword ptr [EAX+4],EBP).
inline constexpr std::size_t kEntitySpawnDescriptorOffset = 0xC0;     // 009261A3
inline constexpr std::size_t kEntitySpawnDescriptorKindOffset = 0x04;  // 009261AD
inline constexpr int kEntityDescriptorKindInitialState = 2;            // 00926131
// `[descriptor+8h]`, and the byte at `+3Ch` of what that points at: set means
// "start enabled". 009261B2 / 009261B5.
inline constexpr std::size_t kEntityDescriptorPayloadOffset = 0x08;
inline constexpr std::size_t kEntityDescriptorStartEnabledByte = 0x3C;
// The argument pass D hands the `+5Ch` predicate (00926286 PUSH EBP, EBP = 2).
inline constexpr int kEntitySessionGateArgument = 2;

// The loading bar. `0057C1A0` is `__fastcall(ECX = phase, float fraction)`,
// `RET 4`: it forwards to `0057BF10` on the singleton at `[00E194B4]`, which
// sums the per-phase weights at `00E08764` below `phase` and adds
// `weights[phase] * fraction`, so `phase` selects a weighted band of the bar and
// `fraction` is the position inside it. Every InitAll site passes phase 3.
inline constexpr int kSEntityInitProgressPhase = 3;  // 00926005, 009260DD, 00926179

// The progress denominator: `00925FB3 LEA EAX,[EAX+EAX*2]`, three passes over
// `count` entities. Pass A clamps its counter at `count` (0092601E..0092603B
// divides the denominator by three with the 0x55555556 reciprocal and takes the
// minimum); passes B and C increment without a clamp.
std::int32_t sentity_init_progress_denominator(std::int32_t pending_count) noexcept;
float sentity_init_progress_fraction(std::int32_t step,
    std::int32_t denominator) noexcept;

// One native call site per method. There are no defaults: nothing here stands in
// for behaviour this packet did not recover.
struct SEntityInitAllHost {
    virtual ~SEntityInitAllHost() = default;

    // [00F899D4] at 00925F3B, and the list walk from [00F899D0]. The walk is
    // given as a snapshot because every pass restarts from the sentinel and the
    // native re-reads the head each time; no pass observed here adds a node.
    virtual std::int32_t pending_count_00f899d4() = 0;
    virtual std::vector<void*> pending_entities_00f899d0() = 0;

    // 00925FFB / 009260D3 / 0092616D / 0092629C, the name accessor at vtable
    // +10h. Its result is discarded at all four sites; the call is kept because
    // it is a call the native makes.
    virtual void entity_name_vcall_10(void* entity) = 0;

    // 00926019 / 009260F1 / 0092618D.
    virtual void loading_progress_report_0057c1a0(int phase, float fraction) = 0;

    // 0092604E, the Lua self-table attach. This is the call that gives an entity
    // the `thisTable` slot `00898E30` pushes.
    virtual void entity_attach_lua_self_vcall_9c(void* entity) = 0;
    // 0092610A and 00926194. Bodies not read; named by slot.
    virtual void entity_init_second_vcall_a0(void* entity) = 0;
    virtual void entity_init_third_vcall_a4(void* entity) = 0;

    // 009261A3..009261B9, the descriptor reads pass C makes after +A4h.
    virtual bool entity_descriptor_kind_is_initial_state(void* entity) = 0;
    virtual bool entity_descriptor_start_enabled_3c(void* entity) = 0;
    // The two release flags 0088B1B0 also gates on (bsp/lua_binding_entity_lookup.hpp).
    virtual bool entity_flag_5e(void* entity) = 0;
    virtual bool entity_flag_5c(void* entity) = 0;
    virtual void entity_set_flag_5c(void* entity, bool value) = 0;
    // 009261D4 and 0092620B, both with a literal 0 argument.
    virtual void entity_enable_vcall_68(void* entity) = 0;
    virtual void entity_disable_vcall_6c(void* entity) = 0;
    // The child chain from +48h along +44h (kEntityChildHeadOffset /
    // kEntityChildNextOffset in bsp/world_entity_update.hpp), each node handed to
    // 00922F30 or 00922F80 with a literal 1.
    virtual void* entity_first_child_48(void* entity) = 0;
    virtual void* entity_next_child_44(void* child) = 0;
    virtual void scene_node_enable_00922f30(void* node) = 0;
    virtual void scene_node_disable_00922f80(void* node) = 0;

    // 00926283 and 009262AE. 0077F090's body was not read; it is named by
    // address. Its one callee is 0077EB50.
    virtual bool entity_session_gate_vcall_5c(void* entity, int argument) = 0;
    virtual void session_register_0077f090(void* entity) = 0;

    // 009262FE..00926319: when entity+C0h is non-null, its own vtable slot 0 is
    // called with a literal 1 and the field is nulled.
    virtual void entity_release_spawn_descriptor(void* entity) = 0;

    // 00926082 / 0092623B, and the 00926089 log line.
    virtual void set_init_active_flag_00f899a5(bool value) = 0;
    virtual void log_enum_count_004b8490(std::int32_t pending_count,
        std::int32_t denominator) = 0;

    // 00926335..00926365: the list is spliced to empty, the nodes freed and the
    // count zeroed, before the tail.
    virtual void clear_pending_list_00926335() = 0;
    // 00926370, reached only when the char argument was non-zero.
    virtual void recon_force_refresh_00807a50() = 0;
};

// The whole routine. Returns without touching anything when the count is zero
// (00925F49), which is the arm the fixed-step caller 00875BB0 takes on a frame
// with no pending entity.
void sentity_init_all_00925f20(bool force_recon_refresh, SEntityInitAllHost& host);

// ---------------------------------------------------------------------------
// Which scene classes get a self table, and which of those `FindEntity` answers
// ---------------------------------------------------------------------------
//
// Two independent facts per scene class, and the milestone 2l executable ran
// them together:
//
//  * a `thisTable` slot exists when the class vtable's slot 39 (`+9Ch`) is one
//    of the attach functions, because InitAll calls that slot on every entity;
//  * `FindEntity` answers only when the class registers on one of the 14 world
//    buckets `0088B1B0` walks (`entity_kind_is_searched`, in
//    bsp/lua_binding_entity_lookup.hpp).
//
// `vtable` and `attach` were read from the shipped image: each class creator in
// 004F2800's table was scanned for the `.rdata` immediates it stores, one call
// deep, and the dword at `vtable+9Ch` read. `world_kind` is the bucket column of
// docs/LUA_BINDING_ENTITY_LOOKUP.md, whose vtables agree with the scan for every
// class the two have in common.
struct SceneClassLuaIdentityRow {
    int class_id;               // the id of 004F2800's row
    const char* name;           // the scene file's class token
    std::uint32_t vtable;       // the vtable the creator installs
    std::uint32_t attach;       // the function at vtable+9Ch
    std::int32_t world_kind;    // the bucket it registers on, or -1 when unread
    bool findable_by_name;      // entity_kind_is_searched(world_kind)
};

inline constexpr std::size_t kSceneClassLuaIdentityCount = 11;
const SceneClassLuaIdentityRow* scene_class_lua_identity_table() noexcept;
const SceneClassLuaIdentityRow* find_scene_class_lua_identity(int class_id) noexcept;

}  // namespace bsp

#endif  // BSP_LUA_BINDING_MISSION_2_HPP
