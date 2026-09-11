// The four spawn bindings of the mission Lua table, and the request record they build.
//
// Packet cc_lua_core, worktree agent/cc-lua-core. Ghidra was read-only for this packet.
// Every name here is a hypothesis, not a recovered symbol.
//
// docs/LUA_BINDING_TABLE.md listed these four among the twenty-four handlers that had no
// Ghidra function; the integrator defined and named them at 39F9528F. They split cleanly in
// two: `Spawn` creates a scene object immediately and hands the script its entity table, while
// the other three are a request queue on a separate manager singleton.
//
//   Spawn                 00944680  creates now, returns the entity table
//   SpawnNew              0094C480  thunk -> 00949750, queues a request
//   SpawnNewIDIsRequested 00946380  thunk -> 00945850, asks whether an id is queued
//   SpawnNewIDRemove      00946390  thunk -> 00945A20, drops every request with an id
//
// The three thunks are three instructions each: PUSH ECX (the lua_State the binding received
// in ECX), MOV ECX,[00F89B3C] (the manager singleton), CALL <method>, RET. So the lua_CFunction
// is `__fastcall(lua_State*)` on the outside and `__thiscall(manager, lua_State*)` inside, and
// the callee cleans the one stack argument.
//
// No struct, enum or k* constant declared here is declared by another header in include/bsp.
#ifndef BSP_LUA_BINDING_SPAWN_HPP
#define BSP_LUA_BINDING_SPAWN_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/lua_binding_core.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The manager singleton
// ---------------------------------------------------------------------------

// *(00F89B3C), loaded by all three thunks (0094C481, 00946381, 00946391).
// include/bsp/world_construct.hpp's `kTailConstructions` already records the producer: the last
// row of that table is `{0x0000, 0x10, 0x00945820, 0x00941310}` at 004DFA92, "stored to
// DAT_00F89B3C". So the object is 10h bytes and 00945820 is its constructor; the two fields
// below are what the three methods read, and they fit inside that size.
inline constexpr std::uint32_t kSpawnRequestManagerSingleton = 0x00F89B3Cu;
inline constexpr std::size_t kSpawnManagerListSentinelOffset = 0x4; // 00945934, 00945B02
inline constexpr std::size_t kSpawnManagerListCountOffset = 0x8;    // 00945BAA ADD [EDI+8],-1

// A list node, as 00945850 and 00945A20 walk it: next, prev, then the record pointer.
inline constexpr std::size_t kSpawnListNodeNextOffset = 0x0;
inline constexpr std::size_t kSpawnListNodePrevOffset = 0x4;
inline constexpr std::size_t kSpawnListNodeRecordOffset = 0x8;

// ---------------------------------------------------------------------------
// The request record
// ---------------------------------------------------------------------------

// 00949530 allocates DCh bytes through operator_new and constructs them with 00948CC0, then
// links the node onto the manager's list and increments its count. The offsets below come from
// 00948CC0, which is the producer, not from a consumer, per the producer-before-layout rule
// of docs/WORKER_VERIFICATION_CHECKLIST.md.
inline constexpr std::size_t kSpawnRequestRecordBytes = 0xDC;

// The two NativeStrings, each a {size, data} pair in the image's usual layout.
// The id pair is the one fact two independent routines agree on: 00948CC0 writes it, and both
// 00945850 (0094594B MOV ECX,[EAX+B8h]) and 00945A20 (00945B31) read exactly these two to
// match a script-supplied id, which is what makes the field the request id rather than a guess.
inline constexpr std::size_t kSpawnRequestIdSizeOffset = 0xB8;  // 00948DFE
inline constexpr std::size_t kSpawnRequestIdDataOffset = 0xBC;  // 00948E04

// The second NativeString, written from the same constructor's eleventh parameter. SpawnNew
// reads exactly two strings out of its request table, `id` and `callback`, so by elimination
// this is the callback name; no routine reads it back, so the attribution is provisional.
inline constexpr std::size_t kSpawnRequestCallbackSizeOffset = 0x84; // 00948D70
inline constexpr std::size_t kSpawnRequestCallbackDataOffset = 0x88; // 00948D76

// The scalar block 00948CC0 fills from its own parameters, in the order it writes them.
// Which Lua field of the request table feeds each one was not separated: SpawnNew stages four
// floats through x87 stores into the outgoing frame at 00949F67..00949F8C, and this packet did
// not follow each store to its slot. Marked provisional for that reason.
inline constexpr std::size_t kSpawnRequestScalarAOffset = 0x68; // 00948D2C, uint
inline constexpr std::size_t kSpawnRequestScalarBOffset = 0x6C; // 00948D32, int
inline constexpr std::size_t kSpawnRequestScalarCOffset = 0x70; // 00948D38, uint
inline constexpr std::size_t kSpawnRequestFloatAOffset = 0x74;  // 00948D41, float
inline constexpr std::size_t kSpawnRequestFloatBOffset = 0x78;  // 00948D47, float
inline constexpr std::size_t kSpawnRequestSerialOffset = 0x7C;  // 00948EC9
inline constexpr std::size_t kSpawnRequestScalarDOffset = 0x80; // 00948D4D

// Seven dwords copied verbatim from a caller-supplied block (00948DB2, `for (i = 7; ...)`),
// covering 9Ch..B7h. One float inside it, at ACh, is clamped to zero-or-greater and used as a
// radius by the constructor's own post-pass at 00948F1x.
inline constexpr std::size_t kSpawnRequestPlacementBlockOffset = 0x9C;
inline constexpr std::size_t kSpawnRequestPlacementBlockDwords = 7;
inline constexpr std::size_t kSpawnRequestPlacementRadiusOffset = 0xAC;

// The group-member vector the request owns, at the head of the record: a begin/end pair of
// 10h-byte elements, the size computed as `(end - begin) >> 4` at 00948EE7.
inline constexpr std::size_t kSpawnRequestGroupBeginOffset = 0x4;
inline constexpr std::size_t kSpawnRequestGroupEndOffset = 0x8;
inline constexpr std::size_t kSpawnRequestGroupElementBytes = 0x10;

// Trailing fields the constructor zeroes or copies last.
inline constexpr std::size_t kSpawnRequestFlagByteOffset = 0xC0;  // 00948E3B, always 0
inline constexpr std::size_t kSpawnRequestPartyOffset = 0xC4;     // 00948E42
inline constexpr std::size_t kSpawnRequestPlayerOffset = 0xC8;    // 00948E48

// The serial the request carries. 00949F2B loads the counter at 00E0CF74, 00949F30 compares
// it against 3E80h unsigned, 00949F37 replaces it with 1 when it is strictly greater, and
// 00949F44/00949F47 store back one more than the serial actually used. So the sequence runs
// 1..16000 and then restarts at 1, and 16001 is never used. The counter is global, not per
// mission, so two missions in one process do not restart it.
inline constexpr std::uint32_t kSpawnRequestSerialCounter = 0x00E0CF74u;
inline constexpr int kSpawnRequestSerialWrapAbove = 16000;
inline constexpr int kSpawnRequestSerialWrapTo = 1;

// ---------------------------------------------------------------------------
// The fields SpawnNew reads out of its request table
// ---------------------------------------------------------------------------

// Every name 00949750 passes to BSP_LuaObject_GetByName, in the order the listing reads them.
// The literals are at the addresses given; `id` and `callback` default to the empty string at
// 00CE3A0C when the field is absent (BSP_LuaObject_ConstructStringOrDefault).
enum class SpawnNewField {
    Party,                 // 0094981B, 00D14754 "party"
    Player,                // 0094985F, 00D199D0 "player"
    Callback,              // 0094989E, 00CE49C0 "callback"
    ExcludeRadiusOverride, // 00949954, 00D199B8 "excludeRadiusOverride"
    Id,                    // 00949972, 00CF16BC "id"
    GroupMembers,          // 009499B5, 00D199A8 "groupMembers"
    CamoColor,             // 00949AD6, 00D09980 "CamoColor"
    Area,                  // 00949B1A, 00D199A0 "area"
    RefPos,                // 00949B30, 00D19998 "refPos"
    AngleRange,            // 00949CC1, 00D1998C "angleRange"
    DistRange,             // 00949D50, 00D19980 "distRange"
    LookAt,                // 00949E2B, 00D19978 "lookAt"
};

inline constexpr std::size_t kSpawnNewFieldCount = 12;

// The literal spelling of each field, indexed by SpawnNewField. A script that misspells one
// gets the default rather than an error, because every read goes through an or-default helper.
const char* spawn_new_field_name(SpawnNewField field) noexcept;

// `refPos` accepts either an entity table or a bare position: 00949B51 asks 008889C0 first and
// takes 00888AA0 on yes (00949BE5) or 00888760 on no (00949B5E). Both `distRange` and
// `angleRange` are read as two indexed numbers, slot 0 then slot 1 (00949CF5/00949D2C and
// 00949DB1/00949DE7); `angleRange` keeps the defaults 200.0 and 2500.0 from 00D19908/00D1990C
// when the field is nil, and its first element is then clamped to at least 10.0 (00D198B8).
inline constexpr float kSpawnNewAngleRangeDefaultLow = 200.0f;  // 00D19908
inline constexpr float kSpawnNewAngleRangeDefaultHigh = 2500.0f; // 00D1990C
inline constexpr float kSpawnNewAngleRangeLowMinimum = 10.0f;    // 00CE38B8

// ---------------------------------------------------------------------------
// Spawn 00944680, the immediate form
// ---------------------------------------------------------------------------

// What the script passes, in slot order. Slots 8 and 9 are optional and the routine decides
// which slot holds the trailing boolean from the count alone (00944AC2 asks is-string, then
// 00944B32 asks is-nil): when slot 8 is a
// string or nil it is consumed as the instance name and the boolean is looked for at slot 9,
// otherwise the boolean is looked for at slot 8.
struct SpawnBindingArguments {
    std::string class_name;      // slot 0, 00944746 BSP_LuaObject_GetString
    int placement_mode{0};       // slot 1, 009447A0; compared against 1 at 009449E4
    bool origin_is_entity{false};// slot 2 through 008889C0 at 009447D4
    float origin[3]{0.0f, 0.0f, 0.0f};
    float span_x{0.0f};          // slot 3, 009448CD
    float span_y{0.0f};          // slot 4, 00944907
    float span_z{0.0f};          // slot 5, 00944941
    float angle_a_radians{0.0f}; // slot 6, 0094497B; degrees in the script
    float angle_b_radians{0.0f}; // slot 7, 009449BE; degrees in the script
    std::string instance_name;   // slot 8 when it is a string, 00944AF8
    bool has_instance_name{false};
    bool direct_placement{false};// the trailing boolean
};

// The slot the trailing boolean occupies for a given argument count and slot-8 shape.
// Returns -1 when the routine looks for no boolean at all.
int spawn_flag_slot(int argument_count, bool slot8_is_string_or_nil) noexcept;

// `placement_mode == 1` makes the two angles relative to the origin entity's own heading: the
// routine refreshes the entity pose, reads the fields at entity+ECh and entity+F4h, derives a
// yaw through the CRT helper at 00BF701A less the constant 1.5707963267948966 at 00CE3830, and
// adds that yaw to both angles (00944995..009449E8). The helper was not identified beyond its
// operand shape, so the yaw derivation is provisional; the addition itself is not.
inline constexpr std::size_t kSpawnOriginEntityHeadingOffsetA = 0xEC;
inline constexpr std::size_t kSpawnOriginEntityHeadingOffsetB = 0xF4;
inline constexpr std::size_t kSpawnOriginEntityPositionOffset = 0xFC; // three floats
inline constexpr std::size_t kSpawnOriginEntityPoseDirtyOffset = 0xC8; // 008947xx, byte

// ---------------------------------------------------------------------------
// The host
// ---------------------------------------------------------------------------

// One method per native call site the four routines reach. No defaults.
struct LuaBindingSpawnHost {
    virtual ~LuaBindingSpawnHost() = default;

    // --- Spawn 00944680 ---
    // 00414DB0 at 0094482D and again at 009449F9 and 00944A11, guarded each time by the byte
    // at entity+C8h (009449EE CMP byte [ESI+C8h],0). Already reconstructed: BSP_EntityPose_RefreshWorld.
    virtual void entity_refresh_world_pose(void* entity) = 0;
    // 00941FA0 at 00944C44, then 009426D0 at 00944C64, 004BC120 at 00944CC4 and two more
    // 009426D0 at 00944CEF and 00944D36: the placement search. None of the three bodies was read; they are named by address together because
    // the routine uses them as one step and discards everything but the resulting position and
    // the success byte. Contract: unread.
    virtual bool find_spawn_position(const SpawnBindingArguments& request, float out_position[3]) = 0;
    // 00942CB0 at 00944EF6, the direct-placement branch taken when the trailing boolean is set
    // (the compiler lays this block after the common tail, so its address is higher than the
    // creator call it feeds). It receives the origin, the three spans, the two angles and one
    // more byte, the result of 00438E10 at 00944EAD comparing a string against a literal. Contract: unread.
    virtual void place_spawn_directly(const SpawnBindingArguments& request, float out_position[3]) = 0;
    // 004C6BA0 at 00944D8A, __thiscall(*(00E188A8), class name, instance name, 0). Body read in
    // full, eight instructions: it forwards to 0046D930, the scene-database creator that
    // resolves a class by name (00468660 BSP_SceneDatabase_ClassIdToName is in its callee set),
    // and then calls BSP_Game_AssignPartyPlayerSlots(0) when game+1FE4h is non-zero. An absent
    // name is passed as the empty NativeString data at 00F89B40, not as null.
    virtual void* create_scene_object(const std::string& class_name,
                                      const std::string& instance_name) = 0;
    // 00874D00 at 00944D93, __fastcall over a byte. Body read only as far as its first two
    // steps (00888230, then 00929460 on the zero branch); what it settles is not established.
    // Contract: partial.
    virtual void after_scene_object_created() = 0;
    // The created object's own virtuals at vtable+118h (loaded 00944DCD, called 00944DDE, the
    // position) and +11Ch (loaded 00944DE6, called 00944DF2, one float). Not read: virtuals with no resolved concrete vtable. Named by slot.
    virtual void object_vcall_118(void* object, const float position[3]) = 0;
    virtual void object_vcall_11c(void* object, float value) = 0;
    // 0046AAB0 at 00944E9B, already named BSP_SceneDatabase_ResolveDeferredReferences.
    virtual void scene_resolve_deferred_references() = 0;
    // The entity id the routine turns into the self-table key: the low 16 bits of the dword at
    // object+174h (00944DF4 MOVZX EAX,word ptr [ESI+174h]), which is the same field
    // include/bsp/mission_lua_bindings.hpp already calls `kEntityIdOffset`.
    virtual std::uint16_t object_entity_id(void* object) = 0;

    // --- the three queue methods ---
    // The manager's request list, as 00945850 (00945934 onwards) and 00945A20 (00945B07)
    // walk it. Returning the ids alone is enough for both: neither reads any field of a
    // record it is scanning other than the id pair at B8h/BCh (0094594B, 00945B31).
    virtual std::vector<std::string> queued_request_ids() = 0;
    // 00945A20's removal step: unlink the node, run the record destructor 009442A0, free the
    // node and decrement the count at manager+8h. Removes every match, not the first: the
    // thirteen bytes at 00945BA7 that Ghidra leaves undisassembled are
    // `ADD ESP,4; ADD dword [EDI+8],-1; JMP 00945B07`, a jump back into the scan.
    virtual std::size_t remove_requests_with_id(const std::string& id) = 0;
    // 00949530 at 00949F97: allocate DCh bytes, construct through 00948CC0, push the node and
    // increment the count (00949549 operator_new, 00949562 00948CC0, 0094958D 00943C00).
    virtual void enqueue_request(const std::string& id, std::uint32_t serial) = 0;
    // The post-incremented counter at 00E0CF74, wrapping as `kSpawnRequestSerialWrapAbove`
    // describes. A host that models it keeps one counter for the whole process.
    virtual std::uint32_t next_request_serial() = 0;
};

// ---------------------------------------------------------------------------
// The four routines
// ---------------------------------------------------------------------------

// 00944680. Creates the object and pushes `thisTable[id]`, the same entity tail
// docs/LUA_BINDING_ENTITY.md established for nineteen other handlers; this is the twentieth.
// Returns 1 on success and 1 with nil pushed when the placement search fails (00944F20).
//
// Coverage: partial. The argument marshalling, the entity tail, the creator call and the
// serial rule were read; the three placement helpers 00941FA0, 009426D0 and 004BC120 and the
// direct-placement helper 00942CB0 were not, and are the host's business.
int lua_binding_spawn(const SpawnBindingArguments& request,
                      LuaBindingResultWriter& results,
                      LuaBindingSpawnHost& host,
                      void** out_object);

// 00946380 -> 00945850. Reads one string, scans the list with a case-insensitive compare that
// short-circuits on the size first (00945922 compares the NativeString sizes before calling
// __stricmp), pushes the boolean and returns 1.
int lua_binding_spawn_new_id_is_requested(LuaBindingArgumentReader& args,
                                          LuaBindingResultWriter& results,
                                          LuaBindingSpawnHost& host);

// 00946390 -> 00945A20. Reads one string, removes every matching record, pushes nothing and
// returns 0. The result count is taken from the frame, so the value is 0 rather than a count.
int lua_binding_spawn_new_id_remove(LuaBindingArgumentReader& args, LuaBindingSpawnHost& host);

// 0094C480 -> 00949750. Reads the twelve fields of its table argument, builds the record and
// queues it; pushes nothing and returns 0.
//
// Coverage: partial. The field set, the defaults, the two-element range reads, the serial rule
// and the enqueue were read; the `groupMembers` element pass (00945C30, 00949610, 008F3710),
// the `area` and `lookAt` geometry (008F84D0, 008F8530, 008F8610, 008F8680, 008F84A0,
// 0085DC80) and the mapping from the staged floats to the record's scalar block were not.
int lua_binding_spawn_new(LuaBindingSpawnHost& host, const std::string& request_id);

// The case-insensitive id match both queue methods use, as a rule: sizes first, then a
// case-insensitive comparison, with two empty ids equal and an empty id never equal to a
// non-empty one (00945900..0094593x, the same shape in 00945B10..00945B4x).
bool spawn_request_id_matches(const std::string& queued, const std::string& wanted) noexcept;

} // namespace bsp

#endif // BSP_LUA_BINDING_SPAWN_HPP
