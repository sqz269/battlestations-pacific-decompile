#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

#include "bsp/lua_binding_core.hpp"

// The fourteen `AI*` handlers of the mission Lua binding table 00E0B7B8.
// docs/LUA_BINDING_AI.md carries the evidence; every descriptive name below is a
// hypothesis, not a recovered symbol. Addresses: 00A37310 AICreate, 00A37400
// AIEnable, 00A37650 AIEnableGrouping, 00A37790 AIMergeGroups, 00A378C0
// AIGetGroupInfo, 00A37A00 AISetCommand, 00A37B30 AISetHintWeight, 00A37D50
// AISetDefendResourcePercent, 00A37EB0 AISetSpawnSceneUnitsWeightMul, 00A38010
// AISetQuickSpawnTargetPos, 00A38200 AIGetTargetWeight, 00A38430
// AISetTargetWeight, 00A38960 AIReloadGlobals, 00A38A50 AICreateGroup.
//
// Every handler shares the prologue and epilogue of docs/LUA_BINDING_CORE.md:
// 00B66C00 borrowed owner, 00B679B0 call frame, 00B66400 result count, 00B669A0
// close, plus a one-time `luakod` log-category static-local that has no per-call
// effect and is not modelled. Argument objects come from 00B677E0, which is
// __thiscall(ECX = frame)(LuaObject* out, int index), RET 8, and returns `out`
// in EAX; several handlers build that object directly in an outgoing by-value
// argument slot (SUB ESP,14h) for a callee that pops it with RET 14h.
//
// No struct, enum or k* constant declared here is declared by another header in
// include/bsp. The argument readers and result writer of lua_binding_core.hpp
// are reused rather than redeclared.
namespace bsp {

// ---------------------------------------------------------------------------
// The per-party AI globals block
// ---------------------------------------------------------------------------

// 00A374C4..00A374D1 computes `0xF8A8C8 + 28 * index` as LEA ESI,[EAX*8+0]; SUB
// ESI,EAX; LEA ESI,[ESI*4+0xF8A8C8], so the record array at 00F8A8C8 has a 1Ch
// stride and is indexed by AIEnable's first argument. AISetQuickSpawnTargetPos
// reaches the same array through `index * 0x1C` at 00A38187.
inline constexpr std::uint32_t kAiPartyRecordArrayAddress = 0x00F8A8C8u;
inline constexpr std::size_t kAiPartyRecordStride = 0x1C;

// Field offsets inside one record, each from the store that writes it.
inline constexpr std::size_t kAiPartyEnabledOffset = 0x00;            // 00A37507
inline constexpr std::size_t kAiPartyAttackRatioOffset = 0x04;        // 00A375xx
inline constexpr std::size_t kAiPartyAggressiveRatioOffset = 0x08;    // 00A375xx
inline constexpr std::size_t kAiPartyQuickSpawnValidOffset = 0x0C;    // 00A38187
inline constexpr std::size_t kAiPartyQuickSpawnPosOffset = 0x10;      // three floats

// The defend-resource-percent array is a separate global at 00F8A8BC, written as
// MOVSS [ESI*4+0xF8A8BC] at 00A37E4B with ESI from 009FFC80. See the doc: the
// index range 009FFC80 can answer (0..6) overruns the twelve bytes that separate
// 00F8A8BC from the record array above. The overlap is recorded, not resolved.
inline constexpr std::uint32_t kAiDefendResourcePercentArrayAddress = 0x00F8A8BCu;

// 00E0E35C, the single global AISetSpawnSceneUnitsWeightMul writes (00A37F5x).
inline constexpr std::uint32_t kAiSpawnSceneUnitsWeightMulAddress = 0x00E0E35Cu;

// The three float constants the numeric handlers clamp against.
inline constexpr float kAiRatioDefault = 0.5f;        // 00CE3800, 3F000000h
inline constexpr float kAiWeightMulMinimum = 0.001f;  // 00D7A23C, 3A83126Fh
inline constexpr float kAiUnitClamp = 1.0f;           // 00D7A24C, 3F800000h

// Offsets inside the 5660h-byte AI group object (size from the operator new at
// 00A38CAE). Each is cited from the instruction that touches it.
inline constexpr std::size_t kAiGroupSize = 0x5660;
inline constexpr std::size_t kAiGroupMemberListOffset = 0x563C;  // 00A2D9D8
inline constexpr std::size_t kAiGroupPopulationOffset = 0x5644;  // 00A2DB9E, 00A2EEFE gate
inline constexpr std::size_t kAiGroupGroupingEnabledOffset = 0x5648; // 00A37731
inline constexpr std::size_t kAiGroupCommandOffset = 0x564C;     // 00A2BD03

// An entity's back-pointer to its AI group, read at 00A37277 and written at
// 00A2D8F5. docs/LUA_BINDING_AI.md derives it from 00A37250's body.
inline constexpr std::size_t kEntityAiGroupOffset = 0x16C;

// The vehicle-class name table AISetTargetWeight searches when an argument is a
// string: 97 char* entries at 00E0CD80, entry 0 being "NULL" (00D0E694). The
// loop bound 61h is at 00A385E9. A miss answers -1.
inline constexpr std::uint32_t kAiClassNameTableAddress = 0x00E0CD80u;
inline constexpr int kAiClassNameTableCount = 0x61;
inline constexpr int kAiClassNameNotFound = -1;

// ---------------------------------------------------------------------------
// Pure argument-decoding rules
// ---------------------------------------------------------------------------

// AIEnable reads the table argument only when the flag it just stored is true
// AND the frame holds more than two arguments (00A37515 CMP [ESI],0; 00A37527
// CMP EAX,3 / JL). Both conditions, in that order.
bool ai_enable_reads_ratios(bool enabled, int argument_count) noexcept;

// AISetDefendResourcePercent clamps to [0, 1]: FLDZ/FCOMIP at 00A37E24 sends a
// negative value to zero, then COMISS against 00D7A24C clamps the top.
float ai_clamp_defend_resource_percent(float value) noexcept;

// AISetSpawnSceneUnitsWeightMul is NOT a clamp of the same shape: 00A37F2x
// stores the 0.001f default first and only overwrites it when the argument is
// at least 0.001f, so an argument below the minimum (the scripts pass 0) lands
// on 0.001f rather than on the argument.
float ai_spawn_scene_units_weight_mul(float value) noexcept;

// AIGetTargetWeight's two optional arguments are read by two independent
// equality tests on the argument count, not by a range test (00A38340 CMP EAX,3
// and 00A38380 CMP EAX,4). With exactly four arguments the third argument is
// therefore never read and the integer stays 0.
struct AiTargetWeightQuery {
    int mode{0};          // argument 2, only when the count is exactly 3
    bool flag{false};     // argument 3, only when the count is exactly 4
};
AiTargetWeightQuery ai_target_weight_query(int argument_count,
                                           int argument_two,
                                           bool argument_three) noexcept;

// AISetTargetWeight's sixth..eighth arguments are read only when the count is at
// least 5 (00A387xx CMP EAX,5 / JL), and the weight is then argument 6 instead
// of argument 3. `extended` is that flag.
int ai_set_target_weight_weight_slot(int argument_count) noexcept;

// The record AISetTargetWeight assembles before 00A32500. The field order is the
// order the handler writes it; the exact byte layout inside 00A32500's 20h-stride
// table is NOT established, because that body was not read in full.
struct AiTargetWeightRule {
    int subject_class{kAiClassNameNotFound};
    int target_class{kAiClassNameNotFound};
    bool subject_was_integer{false};
    bool target_was_integer{false};
    bool flag{false};       // argument 2, always a boolean
    float weight{0.0f};
    bool extended{false};   // set when the frame held five or more arguments
    int extra_a{0};         // argument 3 when extended
    int extra_b{0};         // argument 4 when extended
    bool extra_flag{false}; // argument 5 when extended
};

// ---------------------------------------------------------------------------
// Host boundary: one virtual per native call site
// ---------------------------------------------------------------------------

// Reader extension for the two argument shapes lua_binding_core.hpp does not
// cover: a named field of a table argument (00B67800 + 00B66xxx behind
// 00A3753F/00A375A0), and the vector-3 decode at 00888760.
struct LuaBindingAiReader {
    virtual ~LuaBindingAiReader() = default;
    // 00B67800 field fetch followed by the float read with a default; AIEnable
    // passes 00CE3800 (0.5f) as that default for both ratios.
    virtual float table_field_number(int index, const char* field, float fallback) = 0;
    // 00888760 at 00A38137, __fastcall(ECX = out vec3, EDX = LuaObject*). Body
    // not read; the three floats are copied from its returned pointer.
    virtual void read_vector3(int index, float out_xyz[3]) = 0;
    // 00888D20 at 00A37C20, __fastcall(ECX = LuaObject*). Body read in full: it
    // fetches the field named "Ptr" (00CFAD08) through 00B67800 and converts it
    // with 00B662D0. This is the handle behind an entity table.
    virtual void* lua_table_ptr_field(int index) = 0;
};

// Each method is exactly one native call site of the fourteen handlers. Nothing
// here has a default implementation: none of it stands in for game behaviour.
struct LuaBindingAiHost {
    virtual ~LuaBindingAiHost() = default;

    // --- AICreate --------------------------------------------------------
    // 00A32350 at 00A373AF, no arguments and no cleanup. Body read in full: it
    // allocates 1A8h bytes (00BF55BE), zeroes them (00BF79F0), constructs with
    // 00A31730, attaches through the vtable slot at +98h with the scene root
    // from game+19CCh and an identity 4x4 matrix built at 00A323A2..00A3241A,
    // activates through the slot at +A0h, then calls 00A03A60.
    virtual void ai_controller_create() = 0;

    // --- AIReloadGlobals -------------------------------------------------
    // 00A371C0 at 00A389xx. Body is three instructions: CALL 004C1C50, MOV
    // ECX,EAX, JMP 00A335D0. 00A335D0 is the AI globals loader and one of the
    // writers of 00F8A8BC; its body was not read.
    virtual void ai_reload_globals() = 0;

    // --- the group handlers ---------------------------------------------
    // 00A37250 at 00A3771F, 00A37849, 00A37864, 00A3797B and 00A37AB9. Body read
    // in full: RET 14h, so it takes one LuaObject by value; it calls 00888AA0 to
    // turn the entity table into a native entity and answers entity+16Ch. There
    // is no null test on 00888AA0's result, so a non-entity table faults.
    virtual void* ai_group_of_entity_argument(int index) = 0;

    // 00A2DB80 at 00A3786C, __thiscall(ECX = first group)(second group). Body
    // head read: it returns at once when the second group's +5644h is zero, so
    // an empty source group is a no-op.
    virtual void ai_group_merge(void* into, void* from) = 0;

    // 00B67930 at 00A3798B, __thiscall(ECX = frame)(LuaObject* out). Body read in
    // full: lua_createtable through 00A67D10, a reference through 00A673D0, and
    // the out object is filled with kind 2, the tracked-registry kind.
    virtual void new_lua_table() = 0;

    // 00A2EEE0 at 00A3799F, __thiscall(ECX = group)(LuaObject* table). Body head
    // read: it returns at once when the group's +5644h is zero, and otherwise
    // writes named fields starting with "leader" (00CFD760). The full field set
    // was not read.
    virtual void ai_group_write_info(void* group, /*out*/ void* lua_table) = 0;

    // 00A13340 at 00A37AD6, __thiscall(ECX = group)(LuaObject by value). Body
    // head read: it dispatches on the table's "commandType" field (00D22A88).
    // The command taxonomy was not read.
    virtual void* ai_command_from_table(void* group, int argument_index) = 0;

    // 00A2BD00 at 00A37ADE, __thiscall(ECX = group)(command), RET 4. Body read in
    // full: it deletes the previous command at group+564Ch through its vtable
    // slot 0 with the flag 1, then stores the new pointer. Passing the same
    // pointer twice would therefore free it and store a dangling value.
    virtual void ai_group_set_command(void* group, void* command) = 0;

    // --- AICreateGroup ---------------------------------------------------
    // 00A2D9D0 at 00A38CA5, __thiscall(ECX = the entity's current group)(entity,
    // 1). Body read: it removes the entity from the list at group+563Ch through
    // 0077BEA0 and clears entity+16Ch when it still points at that group.
    virtual void ai_group_remove_entity(void* group, void* entity) = 0;
    // operator new at 00A38CAE with the literal 5660h, then 00A2DFA0 at 00A38CCE,
    // __thiscall(ECX = the raw block)(first entity), answering the group in EAX.
    // A null allocation leaves the group null and the loop continues.
    virtual void* ai_group_construct(void* first_entity) = 0;
    // 00A2D8E0 at 00A38CF6, __thiscall(ECX = group)(entity). Body read: it calls
    // 009FE0B0 on the entity, sets entity+16Ch to the group and links the entity
    // into the group's list at group+10h.
    virtual void ai_group_add_entity(void* group, void* entity) = 0;

    // --- the weight handlers ---------------------------------------------
    // 00A07F60 at 00A37C27, __fastcall(ECX = the hint handle)(float weight),
    // RET 4. Body read in full: 00975C40 resolves a slot in the container at
    // 00F8A740 keyed by the handle, and the float is stored there.
    virtual void ai_hint_weight_set_global(void* hint, float weight) = 0;
    // 00A07F80 at 00A37C9x, __fastcall(ECX = hint handle, EDX = party)(float),
    // RET 4. Body read in full: the same 00975C40 lookup, but in the container
    // at `0xF8A750 + party * 12`, an array of per-party maps.
    virtual void ai_hint_weight_set_for_party(void* hint, int party, float weight) = 0;

    // 009FFC80 at 00A37DF6, no arguments. Body read in full: it reads the game
    // object at 00E188A8, dispatches on 004BCA50's answer through the jump table
    // at 009FFCFC and answers 0..6, with 0 on every failure path.
    virtual int ai_current_party_slot() = 0;

    // 00964790 at 00A382C9 and 00A38303, __fastcall(class id in ECX, DL = 1).
    // The vehicle-class descriptor factory, already reconstructed in
    // docs/VEHICLE_CLASS_DESCRIPTORS.md; a null answer skips the weight query.
    virtual void* vehicle_class_descriptor(int class_id) = 0;

    // 00A08460 at 00A383C4, __fastcall(ECX = subject class, EDX = mode)(target
    // class, flag), returning the weight in ST0. Body not read: 1354
    // instructions. contract: unread beyond its ABI.
    virtual float ai_target_weight_query(void* subject,
                                         void* target,
                                         int mode,
                                         bool flag) = 0;

    // 00A32500 at 00A3890E, __fastcall(ECX = the assembled rule, DL = 1)(-1).
    // Body head read only: DL selects the insert branch, which walks a 20h-stride
    // table addressed by the globals 00F8AB5C and 00F8AB60. coverage: partial.
    virtual void ai_target_weight_rule_apply(const AiTargetWeightRule& rule) = 0;

    // --- the global stores ------------------------------------------------
    // The four writes into the globals block, each named by its store site.
    virtual void store_party_enabled(int index, bool enabled) = 0;          // 00A37507
    virtual void store_party_ratios(int index, float attack, float aggressive) = 0;
    virtual void store_party_quick_spawn(int index, bool valid, const float xyz[3]) = 0;
    virtual void store_defend_resource_percent(int slot, float percent) = 0; // 00A37E4B
    virtual void store_spawn_scene_units_weight_mul(float value) = 0;        // 00E0E35C

    // 00B66480 at 00A383D9, the float result push. This is a separate entry from
    // the 00B664B0 integer push that LuaBindingResultWriter models: the value is
    // handed over as a 4-byte float through FSTP, so it is declared here rather
    // than widening that interface.
    virtual void push_number_float(float value) = 0;
};

// ---------------------------------------------------------------------------
// One sequence per handler. Each returns the handler's result count.
// ---------------------------------------------------------------------------

int lua_binding_ai_create(LuaBindingAiHost& host);                       // 00A37310
int lua_binding_ai_enable(LuaBindingArgumentReader& args,
                          LuaBindingAiReader& tables,
                          LuaBindingAiHost& host);                       // 00A37400
int lua_binding_ai_enable_grouping(LuaBindingArgumentReader& args,
                                   LuaBindingAiHost& host);              // 00A37650
int lua_binding_ai_merge_groups(LuaBindingAiHost& host);                 // 00A37790
int lua_binding_ai_get_group_info(LuaBindingAiHost& host);               // 00A378C0
int lua_binding_ai_set_command(LuaBindingAiHost& host);                  // 00A37A00
int lua_binding_ai_set_hint_weight(LuaBindingArgumentReader& args,
                                   LuaBindingAiReader& tables,
                                   LuaBindingAiHost& host);              // 00A37B30
int lua_binding_ai_set_defend_resource_percent(LuaBindingArgumentReader& args,
                                               LuaBindingAiHost& host);  // 00A37D50
int lua_binding_ai_set_spawn_scene_units_weight_mul(LuaBindingArgumentReader& args,
                                                    LuaBindingAiHost& host); // 00A37EB0
int lua_binding_ai_set_quick_spawn_target_pos(LuaBindingArgumentReader& args,
                                              LuaBindingAiReader& tables,
                                              LuaBindingAiHost& host);   // 00A38010
int lua_binding_ai_get_target_weight(LuaBindingArgumentReader& args,
                                     LuaBindingAiHost& host);            // 00A38200
int lua_binding_ai_set_target_weight(LuaBindingArgumentReader& args,
                                     LuaBindingAiHost& host);            // 00A38430
int lua_binding_ai_reload_globals(LuaBindingAiHost& host);               // 00A38960
int lua_binding_ai_create_group(LuaBindingAiHost& host,
                                void* const* entities,
                                std::size_t entity_count);               // 00A38A50

// The class-name lookup AISetTargetWeight performs, as a rule over the caller's
// table. Answers the index or -1; the native search is a strcmp walk over the
// 97 entries at 00E0CD80 that stops at the first match.
int ai_class_index_from_name(const std::string& name,
                             const char* const* table,
                             int table_count) noexcept;

} // namespace bsp
