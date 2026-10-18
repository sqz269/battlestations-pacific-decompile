#pragma once

// cc13_seg67b: segment 67 (00996120-009F6060), the plane bot-task and pilot-bot
// framework, second pass (packet cc13-seg67A, docs/CC13_SEG67B.md; grant: cc9
// gameplay lead 2026-10-18 addendum, standalone, nothing wired). Named seg67b
// because cc13_seg67a.* already exists (the earlier lineage) and the names would
// collide on a case-insensitive file system.
//
// Every name is a hypothesis, not a recovered symbol. These are new C++
// interfaces over the native records, not drop-in replacements of the original
// ABI; each function's comment gives the native ABI. Native objects are raw
// byte blocks touched only at the offsets the image reads or writes, and every
// call the image makes goes through a named Cc13Seg67bHost method at the same
// point with the same arguments. Nothing here is wired into the game.
// Build-tested only.
//
// Precision: one x87 operation per float store unless a comment says
// otherwise. Products of a float and a float-representable qword constant are
// exact in 53 bits, so the store rounds once and the result does not depend on
// the precision control (24-bit on the main thread, 53-bit in some unit jobs).
//
// RNG: 00BD2F10 BSP_Random_UniformFloatRange(ECX = stream, lo, hi). Stream 1 is
// the shared process-wide generator; stream 0 is the second one. Draws are
// listed per function.

#include <cstddef>
#include <cstdint>

namespace bsp {

template <class T>
inline T& cc13_seg67b_at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

// The eight-byte {type, value} record the image builds on the stack for the
// archive slots +4h (begin section), +0Ch (write) and +10h (read). Types seen:
// 0 name string, 1 dword, 2 float, 3 byte, 4 object reference. For a read the
// payload is the address of the destination; for a write it is the value (a
// float write carries the float's bits).
struct Cc13Seg67bArchiveValue {
    std::int32_t type = 0;
    std::uintptr_t payload = 0;
};

// Vtables and string addresses the bodies store or pass (image addresses).
inline constexpr std::uint32_t kBotTaskFactoryVtable = 0x00D1F3E0u;
inline constexpr std::uint32_t kBotTaskVtable = 0x00D05708u;
inline constexpr std::uint32_t kBotTaskSubVtable30C = 0x00D05704u;
inline constexpr std::uint32_t kPureCallVtable = 0x00D05840u;
inline constexpr std::uint32_t kBotConditionTimedVtable = 0x00D1EA00u;
inline constexpr std::uint32_t kBotConditionTurnToVtable = 0x00D1EA40u;
inline constexpr std::uint32_t kStrAvoidFlags = 0x00D1F3A8u;   // "avoidFlags"
inline constexpr std::uint32_t kStrType = 0x00CE4AD0u;         // "type"
inline constexpr std::uint32_t kStrTimed = 0x00D1EA28u;        // "timed"
inline constexpr std::uint32_t kStrDecorated = 0x00D1EA1Cu;    // "decorated"
inline constexpr std::uint32_t kStrTimeLeft = 0x00CE7318u;     // "timeLeft"
inline constexpr std::uint32_t kStrSimpleTimed = 0x00D1EA10u;  // "simpletimed"
inline constexpr std::uint32_t kStrEntity = 0x00CE5818u;       // "entity"
inline constexpr std::uint32_t kStrEntityPair = 0x00D1EA88u;   // "entityPair"
inline constexpr std::uint32_t kStrTurnAway = 0x00D1EA7Cu;     // "turnAway"
inline constexpr std::uint32_t kStrTurnTo = 0x00D1EA74u;       // "turnTo"
inline constexpr std::uint32_t kStrTurnToHdg = 0x00D1EA68u;    // "turnToHdg"
inline constexpr std::uint32_t kStrFarAway = 0x00D1EA60u;      // "farAway"
inline constexpr std::uint32_t kStrPID = 0x00D1EA94u;          // "pID"
inline constexpr std::uint32_t kStrPos = 0x00CF8FA4u;          // "pos"
inline constexpr std::uint32_t kStrHeading = 0x00CFE758u;      // "heading"
inline constexpr std::uint32_t kStrAngle = 0x00D1EA98u;        // "angle"
inline constexpr std::uint32_t kStrDist2 = 0x00D1EAA0u;        // "dist2"
inline constexpr std::uint32_t kBotConditionSimpleTimedVtable = 0x00D1E9F0u;
inline constexpr std::uint32_t kBotConditionTurnToHdgVtable = 0x00D1EA30u;
inline constexpr std::uint32_t kTargetObserverVtable = 0x00CE3CF0u;

class Cc13Seg67bHost {
public:
    virtual ~Cc13Seg67bHost() = default;

    // ---- globals ----
    // 00F8A314: head of the bot-task factory list (factory+0Ch is next).
    virtual void* bot_task_factory_head_00f8a314() = 0;
    virtual void set_bot_task_factory_head_00f8a314(void* factory) = 0;
    // 00F8A30C: base of the pilot robots table (rows of 248h, first at +0Ch).
    virtual std::uintptr_t robots_table_00f8a30c() = 0;
    // 00F876A4: the float clock the timed condition subtracts.
    virtual float global_clock_00f876a4() = 0;

    // ---- other packets' functions ----
    // 00419CC0 BSP_SizedStoragePool_GetSingleton: no arguments, EAX.
    virtual void* sized_storage_pool_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock: __thiscall(ECX = pool, block,
    // size, unused), RET 0Ch.
    virtual void sized_storage_pool_return_block_00bd1510(void* pool, void* block,
                                                          std::uint32_t size,
                                                          std::uint32_t unused) = 0;
    // 0099A870: __thiscall(ECX = pilot bot, task), RET 4 (the retire).
    virtual void pilot_bot_retire_task_0099a870(void* bot, void* task) = 0;
    // 0099D0A0 BSP_PilotBot_HeadingDiffCalc: __thiscall(ECX = plan, float), RET 4, ST0.
    virtual float pilot_heading_diff_0099d0a0(void* plan, float value) = 0;
    // 00438AA0 BSP_Math_AddWrappedAngle(float, float), RET 8, ST0.
    virtual float math_add_wrapped_angle_00438aa0(float a, float b) = 0;
    // 00438B10 BSP_Math_SubtractWrappedAngle(float, float), RET 8, ST0.
    virtual float math_subtract_wrapped_angle_00438b10(float a, float b) = 0;
    // 0099BE30: __thiscall(ECX = task+4, base), RET 4: the embedded base at task+4.
    virtual void bot_task_embedded_construct_0099be30(void* sub, void* base) = 0;
    // 00BD2F10 BSP_Random_UniformFloatRange: __thiscall(ECX = stream, lo, hi), RET 8, ST0.
    virtual float random_uniform_00bd2f10(std::uint32_t stream, float lo, float hi) = 0;
    // 009FAAD0 BSP_BotTaskGun_Construct: __thiscall(ECX = task+314h, task), RET 4.
    virtual void bot_task_gun_construct_009faad0(void* gun, void* task) = 0;
    // 009FCEF0 BSP_PilotBot_OpportunisticRocketConstruct: __thiscall(ECX = task+38Ch,
    // task), RET 4.
    virtual void opportunistic_rocket_construct_009fcef0(void* rocket, void* task) = 0;
    // 009FAB60 BSP_BotTaskGun_InitAimParameters: __thiscall(ECX = task+314h, a, b, c,
    // d), RET 10h.
    virtual void bot_task_gun_init_aim_009fab60(void* gun, float a, float b, float c,
                                                float d) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld: __thiscall(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 0042BCA0 BSP_TargetObserver_Destruct: __thiscall(ECX = observer).
    virtual void target_observer_destruct_0042bca0(void* observer) = 0;
    // 007B5C70 BSP_BotCondition_ConstructBase: __thiscall(ECX = condition, arg), RET 4.
    virtual void bot_condition_construct_base_007b5c70(void* condition, void* arg) = 0;

    // 00BF681B operator new(size), cdecl, EAX.
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    // 00BF7FBF _stricmp(const char*, const char*), cdecl, EAX. The second
    // argument is an image string address.
    virtual int crt_stricmp_00bf7fbf(const char* a, std::uint32_t b_string) = 0;
    // 00425850 BSP_NativeString_EqualsCStringInsensitive: __thiscall(ECX = native
    // string {length, buffer}, image C string), RET 4, AL.
    virtual bool native_string_equals_insensitive_00425850(void* string,
                                                           std::uint32_t cstring) = 0;
    // 00796BD0 BSP_EntityObserver_CopyConstructBase: __thiscall(ECX = dst, src), RET 4.
    virtual void entity_observer_copy_construct_00796bd0(void* dst, void* src) = 0;
    // 00694A60 BSP_Observer_RegisterPair / 006952A0 BSP_Observer_UnregisterPair:
    // __fastcall(ECX = entity, EDX = observer), RET.
    virtual void observer_register_pair_00694a60(void* entity, void* observer) = 0;
    virtual void observer_unregister_pair_006952a0(void* entity, void* observer) = 0;
    // 007B5D30: __thiscall(ECX = condition, arg, float x, y, z by value), RET 10h:
    // the off-nose condition constructor.
    virtual void bot_condition_point_off_nose_construct_007b5d30(void* condition, void* arg,
                                                                 float x, float y,
                                                                 float z) = 0;
    // 007B5E20: __thiscall(ECX = condition, arg, const float* point, float), RET 0Ch:
    // the distance condition constructor.
    virtual void bot_condition_point_distance_construct_007b5e20(void* condition, void* arg,
                                                                 const float* point,
                                                                 float distance) = 0;
    // 00410630: __thiscall(ECX = native string / reader), the destructor.
    virtual void native_string_destruct_00410630(void* object) = 0;
    // Member destructors the teardown bodies call, each __thiscall(ECX = member), RET.
    // 007B45F0: the bot-state base destructor.
    virtual void bot_state_base_destruct_007b45f0(void* state) = 0;
    // 006CDD70: the destructor of a follow-style state's +18h member.
    virtual void member_destruct_006cdd70(void* member) = 0;
    // 007B65E0: the moveto state destructor.
    virtual void bot_state_moveto_destruct_007b65e0(void* state) = 0;
    // 004116D0: the state-registry destructor (ECX may be null).
    virtual void state_registry_destruct_004116d0(void* registry) = 0;
    // 00695870 BSP_CallbackOwner_Destroy.
    virtual void callback_owner_destroy_00695870(void* owner) = 0;
    // 00653390: the destructor of the dogfight approach's +B8h member.
    virtual void member_destruct_00653390(void* member) = 0;
    // 007B4030: the bot-task base destructor.
    virtual void bot_task_destruct_base_007b4030(void* task) = 0;
    // 007B8A90 BSP_Plane_ClearTurboByte: __thiscall(ECX = unit), RET.
    virtual void plane_clear_turbo_byte_007b8a90(void* unit) = 0;
    // 0042E740 BSP_GameTuning_GetSingleton: EAX.
    virtual void* game_tuning_singleton_0042e740() = 0;
    // 004107B0 BSP_ArchiveWriterRef_Destroy: __fastcall(ECX = the 0Ch-byte by-value
    // writer handle {vtable, int* count, writer*}).
    virtual void archive_writer_ref_destroy_004107b0(void* handle) = 0;
    // 00E198C4: a global object pointer (its +8Ch holds {byte +4h, unit +34h}).
    virtual void* global_object_00e198c4() = 0;
    // 00F876B8: the WORD index of the local side (unit+9C2h + index*8 is a byte).
    virtual std::uint16_t global_side_index_00f876b8() = 0;
    // 00F87574: a float[3] global (the three floats the condition factory passes).
    virtual float* global_vector_00f87574() = 0;
    // object->vtable[38h](), RET, AL.
    virtual bool object_vcall_38(void* object) = 0;
    // object->vtable[34h](), RET, AL.
    virtual bool object_vcall_34(void* object) = 0;
    // object->vtable[4Ch](), RET, ST0.
    virtual float object_vcall_4c_float(void* object) = 0;
    // object->vtable[8](), no stack arguments.
    virtual void object_vcall_08_noargs(void* object) = 0;
    // 0064A610: __fastcall(ECX = observed-owner base), RET.
    virtual void observed_owner_base_destroy_0064a610(void* object) = 0;
    // 00411E70: __thiscall(ECX = state registry; name string, state), RET 8.
    virtual void state_registry_add_00411e70(void* registry, std::uint32_t name_string,
                                             void* state) = 0;
    // 009BAFC0: __thiscall(ECX = task, bot, condition), RET 8: the pull-up task base.
    virtual void bot_task_pull_up_construct_009bafc0(void* task, void* bot, void* condition) = 0;
    // 00999F50: __thiscall(ECX = bot, task), RET 4.
    virtual void pilot_bot_prepend_active_task_00999f50(void* bot, void* task) = 0;
    // 00BD4FE0: __thiscall(ECX = archive; out ref [0Ch], name record), RET 0Ch:
    // opens the named child section for reading into `out_ref`.
    virtual void* archive_reader_ref_open_00bd4fe0(void* archive, void* out_ref,
                                                   Cc13Seg67bArchiveValue name) = 0;
    // 00BD5010: the write twin of 00BD4FE0.
    virtual void* archive_writer_ref_open_00bd5010(void* archive, void* out_ref,
                                                   Cc13Seg67bArchiveValue name) = 0;
    // 009FB5E0: __thiscall(ECX = approach target ref, target), RET 4.
    virtual void approach_ref_set_target_009fb5e0(void* ref, void* target) = 0;
    // 009FA350: __fastcall(ECX = approach target ref), RET.
    virtual void approach_ref_reset_aim_offset_009fa350(void* ref) = 0;
    // 007B7870: __thiscall(ECX = opportunistic rocket, target, float[3] by value), RET 10h.
    virtual void opportunistic_rocket_set_target_007b7870(void* rocket, void* target,
                                                          const float point[3]) = 0;
    // 009A94E0: __thiscall(ECX = task+3F8h, bot, arg), RET 8: the dogfight approach block.
    virtual void dogfight_construct_approach_block_009a94e0(void* block, void* bot, void* arg) = 0;
    // 009F9980: __thiscall(ECX = approach, task), RET 4.
    virtual void bot_approach_bind_to_task_009f9980(void* approach, void* task) = 0;
    // 007B8AD0: __fastcall(ECX = unit), RET, AL.
    virtual bool unit_is_squadron_flight_leader_007b8ad0(void* unit) = 0;
    // state->vtable[4](), RET: the state's enter slot.
    virtual void state_vcall_04_enter(void* state) = 0;

    // ---- virtual slots (ECX = the object) ----
    // object->vtable[+8](archive), RET 4: an object's load slot.
    virtual void object_vcall_08_load(void* object, void* archive) = 0;
    // object->vtable[+5Ch](arg), RET 4, AL.
    virtual bool object_vcall_5c(void* object, std::uint32_t arg) = 0;
    // archive->vtable[+14h](name), RET 8, AL: does the named section exist.
    virtual bool archive_vcall_14_has(void* archive, Cc13Seg67bArchiveValue name) = 0;
    // object->vtable[+4](flags), RET 4: the scalar deleting destructor.
    virtual void object_vcall_04_delete(void* object, std::uint32_t flags) = 0;
    // entity->vtable[+50h](), ST0: the heading.
    virtual float entity_vcall_50_heading(void* entity) = 0;
    // archive->vtable[+4](name), RET 8: begin a named section.
    virtual void archive_vcall_04_begin(void* archive, Cc13Seg67bArchiveValue name) = 0;
    // archive->vtable[+8](): end the section.
    virtual void archive_vcall_08_end(void* archive) = 0;
    // archive->vtable[+0Ch](name, value) and [+10h](name, value), RET 10h.
    virtual void archive_vcall_0c_write(void* archive, Cc13Seg67bArchiveValue name,
                                        Cc13Seg67bArchiveValue value) = 0;
    virtual void archive_vcall_10_read(void* archive, Cc13Seg67bArchiveValue name,
                                       Cc13Seg67bArchiveValue value) = 0;
    // object->vtable[+0Ch](archive), RET 4: an object's save slot.
    virtual void object_vcall_0c_save(void* object, void* archive) = 0;
};

// ---- the bot-task factory list (00F8A314) ----
// 0099EBB0: __thiscall(ECX = factory, unread arg), RET 4, EAX = factory.
void* bot_task_factory_register_0099ebb0(Cc13Seg67bHost& host, void* factory);
// 0099EBE0: __thiscall(ECX = factory), RET.
void bot_task_factory_unregister_0099ebe0(Cc13Seg67bHost& host, void* factory);

// ---- the bot-task base ----
// 0099B630: __fastcall(ECX = plan), RET, ST0.
float pilot_command_pitch_target_or_live_0099b630(void* plan);
// 0099B650: __thiscall(ECX = plan, owner), RET 4.
void bot_task_set_owner_25c_0099b650(void* plan, std::uint32_t owner);
// 0099B690: __thiscall(ECX = task), RET (the pushed ECX is 0099A870's argument).
void bot_task_retire_0099b690(Cc13Seg67bHost& host, void* task);
// 009A1A20: __fastcall(ECX = plan), RET, ST0.
float bot_task_commanded_heading_009a1a20(Cc13Seg67bHost& host, void* plan);
// 0099C6F0: __thiscall(ECX = task, bot, kind), RET 8, EAX = task.
void* bot_task_construct_base_0099c6f0(Cc13Seg67bHost& host, void* task, void* bot,
                                       std::uint32_t kind);
// 0099C8B0: __thiscall(ECX = task, bot, archive, kind), RET 0Ch, EAX = task.
void* bot_task_construct_base_from_stream_0099c8b0(Cc13Seg67bHost& host, void* task,
                                                   void* bot, void* archive,
                                                   std::uint32_t kind);
// 0099D060: __thiscall(ECX = task, archive), RET 4.
void bot_task_save_base_0099d060(Cc13Seg67bHost& host, void* task, void* archive);

// ---- bot conditions ----
// 00996120: __thiscall(ECX = condition), RET: the timed condition's destructor body.
void bot_condition_timed_destruct_00996120(Cc13Seg67bHost& host, void* condition);
// 00996270: __fastcall(ECX = condition), RET, EAX (0/1).
bool bot_condition_turn_to_test_00996270(Cc13Seg67bHost& host, void* condition);
// 00996460: __thiscall(ECX = condition, archive), RET 4.
void bot_condition_timed_save_00996460(Cc13Seg67bHost& host, void* condition,
                                       void* archive);
// 009965D0: __thiscall(ECX = archive, name record, observer by value [18h]),
// RET 20h. `observer` is the callee-owned by-value copy (destroyed here).
void bot_target_condition_save_target_ref_009965d0(Cc13Seg67bHost& host, void* archive,
                                                   Cc13Seg67bArchiveValue name,
                                                   void* observer);
// 00996670 and 00996770: __thiscall(ECX = condition), RET: two identical
// target-condition destructor bodies (EH handlers 00CAD398 / 00CAD3D8).
void bot_target_condition_destruct_00996670(Cc13Seg67bHost& host, void* condition);
void bot_target_condition_destruct_00996770(Cc13Seg67bHost& host, void* condition);
// 00996720: __thiscall(ECX = condition, arg, f1, f2, f3, f4), RET 14h, EAX.
void* bot_condition_turn_to_construct_00996720(Cc13Seg67bHost& host, void* condition,
                                               void* arg, float x, float y, float z,
                                               float limit);
// 009968D0: __fastcall(ECX = archive), RET, EAX = the new condition (or null).
void* bot_condition_create_from_archive_009968d0(Cc13Seg67bHost& host, void* archive);
// 00996BB0: __stdcall(reader by value [0Ch]), RET 0Ch, EAX. `reader` is the
// callee-owned copy; 00410630 destroys it after 009968D0.
void* bot_condition_load_from_reader_00996bb0(Cc13Seg67bHost& host, void* reader);
// 00996C00: __thiscall(ECX = timed condition, archive), RET 4.
void bot_condition_timed_load_00996c00(Cc13Seg67bHost& host, void* condition, void* archive);
// 00996CA0: __thiscall(ECX = target condition, archive), RET 4.
void bot_target_condition_save_00996ca0(Cc13Seg67bHost& host, void* condition, void* archive);
// 00996D00 / 00996D70 / 00996E10 / 00996EB0: __thiscall(ECX = condition,
// archive), RET 4: the turnAway, turnToHdg, turnTo and farAway saves.
void bot_condition_turn_away_save_00996d00(Cc13Seg67bHost& host, void* condition, void* archive);
void bot_condition_turn_to_heading_save_00996d70(Cc13Seg67bHost& host, void* condition,
                                                 void* archive);
void bot_condition_turn_to_save_00996e10(Cc13Seg67bHost& host, void* condition, void* archive);
void bot_condition_far_away_save_00996eb0(Cc13Seg67bHost& host, void* condition, void* archive);
// 00996F50: __fastcall(ECX = archive, EDX = observer, name record by value), RET 8.
void bot_target_condition_load_target_00996f50(Cc13Seg67bHost& host, void* archive,
                                               void* observer, Cc13Seg67bArchiveValue name);
// 00996FB0 / 00997090 / 00997110 / 009971B0 / 00997250: __thiscall(ECX =
// condition, archive), RET 4: the target, turnAway, turnToHdg, turnTo and
// farAway loads.
void bot_target_condition_load_00996fb0(Cc13Seg67bHost& host, void* condition, void* archive);
void bot_condition_turn_away_load_00997090(Cc13Seg67bHost& host, void* condition, void* archive);
void bot_condition_turn_to_heading_load_00997110(Cc13Seg67bHost& host, void* condition,
                                                 void* archive);
void bot_condition_turn_to_load_009971b0(Cc13Seg67bHost& host, void* condition, void* archive);
void bot_condition_far_away_load_00997250(Cc13Seg67bHost& host, void* condition, void* archive);

// ---- states: the follow state and the shared bodies ----
// 009C2980: __thiscall(ECX = state, approach, float unread), RET 8, EAX = state.
// Draw: 009C2A0A stream 1 (shared) U(0, 0.6), negated into +74h.
void* bot_state_follow_construct_009c2980(Cc13Seg67bHost& host, void* state, void* approach,
                                          float unread);
// 009BDE40: __thiscall(ECX = follow state), RET.
void bot_state_follow_exit_009bde40(Cc13Seg67bHost& host, void* state);
// 009BDE80: __thiscall(ECX = moveto state, a, b, c), RET 0Ch.
void bot_state_moveto_set_ranges_009bde80(void* state, float a, float b, float c);

// 009BEAA0: __thiscall(ECX = moveto state, writer handle by value [0Ch]), RET 0Ch.
// `handle` is the callee-owned copy (004107B0 destroys it).
void bot_approach_save_altitude_leg_009beaa0(Cc13Seg67bHost& host, void* state, void* handle);
// 009C2B80: __thiscall(ECX = state, approach, reader handle by value [0Ch]), RET 10h,
// EAX = state: the moveto-style state's stream constructor.
void* bot_state_construct_base_009c2b80(Cc13Seg67bHost& host, void* state, void* approach,
                                        void* handle);
// 0099C210: __fastcall(ECX = task), RET, EAX = the pilot robots row.
std::uintptr_t bot_task_pilot_robots_row_0099c210(Cc13Seg67bHost& host, void* task);
// 0099C230: __fastcall(ECX = task), RET, AL.
bool bot_task_break_off_allowed_for_unit_0099c230(Cc13Seg67bHost& host, void* task);
// 0099C270: __fastcall(ECX = task), RET, EAX (0/1).
bool bot_task_tick_gate_passes_0099c270(Cc13Seg67bHost& host, void* task);

// ---- teardown bodies: __thiscall(ECX = object), RET, EH frames not modelled ----
// Follow-style state destructor bodies, all 006CDD70(s+18h) then 007B45F0(s):
// 009A3930, 009AD390, 009B55D0, 009C40F0, 009D0B20.
void bot_state_follow_destruct_009a3930(Cc13Seg67bHost& host, void* state);
void bot_state_follow_destruct_009ad390(Cc13Seg67bHost& host, void* state);
void bot_state_follow_destruct_009b55d0(Cc13Seg67bHost& host, void* state);
void bot_state_dive_bomb_destruct_members_009c40f0(Cc13Seg67bHost& host, void* state);
void bot_state_follow_destruct_009d0b20(Cc13Seg67bHost& host, void* state);
// Approach-block destructors (ECX = the block at task+3F8h).
void bot_task_moveto_destruct_approach_block_009c2880(Cc13Seg67bHost& host, void* block);
void bot_task_level_bomb_destruct_approach_block_009b6480(Cc13Seg67bHost& host, void* block);
void bot_task_dive_bomb_destruct_approach_block_009c70a0(Cc13Seg67bHost& host, void* block);
void bot_task_depth_charge_destruct_approach_block_009a4680(Cc13Seg67bHost& host, void* block);
void bot_task_drop_kamikaze_destruct_approach_block_009adae0(Cc13Seg67bHost& host, void* block);
void bot_task_dogfight_destruct_approach_block_009a93e0(Cc13Seg67bHost& host, void* block);
void bot_task_retreat_destruct_approach_block_009c9a80(Cc13Seg67bHost& host, void* block);
void bot_task_torpedo_destruct_approach_block_009d2390(Cc13Seg67bHost& host, void* block);
void bot_task_strafe_destruct_approach_block_009cbe90(Cc13Seg67bHost& host, void* block);
void bot_task_kamikaze_destruct_approach_block_009ace50(Cc13Seg67bHost& host, void* block);
void bot_task_takeoff_destruct_approach_block_009cf850(Cc13Seg67bHost& host, void* block);
void bot_approach_move_on_path_destroy_009bcb70(Cc13Seg67bHost& host, void* block);
// 009A6B90: the dogfight approach's own members.
void bot_approach_dogfight_destruct_body_009a6b90(Cc13Seg67bHost& host, void* block);
// Task bodies.
void bot_task_conditioned_destruct_body_009baf60(Cc13Seg67bHost& host, void* task);
void bot_task_moveto_destruct_body_009c2920(Cc13Seg67bHost& host, void* task);
// Ship-AI observer members.
void ship_ai_observer_member_destruct_body_009d60f0(Cc13Seg67bHost& host, void* member);
void ship_ai_path_node_observer_destruct_body_009d5eb0(Cc13Seg67bHost& host, void* member);
void ship_ai_observer_member_destruct_a_009db380(Cc13Seg67bHost& host, void* object);
void ship_ai_observer_member_destruct_b_009db3d0(Cc13Seg67bHost& host, void* object);
void ship_ai_observer_member_destruct_c_009db420(Cc13Seg67bHost& host, void* object);
// 009A7650: __thiscall(ECX = dogfight approach, target), RET 4.
void bot_approach_dogfight_set_target_009a7650(Cc13Seg67bHost& host, void* approach,
                                               void* target);
// 009B9EA0: __thiscall(ECX = task, archive), RET 4.
void bot_task_conditioned_save_009b9ea0(Cc13Seg67bHost& host, void* task, void* archive);
// 009B9F20: __thiscall(ECX = task, archive), RET 4.
void bot_task_pull_up_save_009b9f20(Cc13Seg67bHost& host, void* task, void* archive);
// 009BA1F0: __thiscall(ECX = task, archive), RET 4.
void bot_task_barrel_roll_save_009ba1f0(Cc13Seg67bHost& host, void* task, void* archive);
// 009BA270: __thiscall(ECX = task, archive), RET 4.
void bot_task_flik_flak_save_009ba270(Cc13Seg67bHost& host, void* task, void* archive);
// 009BA830: __thiscall(ECX = task, bot, archive), RET 8, EAX = task.
void* bot_task_conditioned_construct_from_stream_009ba830(Cc13Seg67bHost& host, void* task,
                                                          void* bot, void* archive);
// 009A9810: __thiscall(ECX = task, bot, arg), RET 8, EAX = task.
void* bot_task_dogfight_construct_009a9810(Cc13Seg67bHost& host, void* task, void* bot,
                                           void* arg);
// 009AB490: __fastcall(ECX = bot, EDX = unit), RET, EAX.
void* bot_task_dogfight_create_for_squadron_009ab490(Cc13Seg67bHost& host, void* bot,
                                                     void* unit);
// 009AB500: __fastcall(ECX = bot, EDX = arg), RET, EAX.
void* bot_task_dogfight_create_009ab500(Cc13Seg67bHost& host, void* bot, void* arg);
// 009AB570: __fastcall(ECX = bot, EDX = target), RET, EAX.
void* bot_task_make_dogfight_009ab570(Cc13Seg67bHost& host, void* bot, void* target);
// 009BA1E0: __thiscall(ECX = task, archive), RET 4 (JMP 009B9EA0).
void bot_task_leave_alone_save_009ba1e0(Cc13Seg67bHost& host, void* task, void* archive);
// 009A3090 / 009A6A60 / 009AB740 / 009B42D0: __fastcall(ECX = approach), RET.
void bot_task_depth_charge_register_state_names_009a3090(Cc13Seg67bHost& host, void* approach);
void bot_task_dogfight_register_state_names_009a6a60(Cc13Seg67bHost& host, void* approach);
void bot_task_drop_kamikaze_register_state_names_009ab740(Cc13Seg67bHost& host,
                                                          void* approach);
void bot_task_level_bomb_register_state_names_009b42d0(Cc13Seg67bHost& host, void* approach);
// 009A36A0 / 009A70E0 / 009A84E0 / 009B49B0 / 009BC960: __thiscall(ECX = state,
// approach), RET 4, EAX = state.
void* bot_state_depth_charge_attack_run_construct_009a36a0(Cc13Seg67bHost& host, void* state,
                                                           void* approach);
void* bot_state_dogfight_attack_run_construct_009a70e0(Cc13Seg67bHost& host, void* state,
                                                       void* approach);
void* bot_state_dogfight_maneuver_construct_009a84e0(Cc13Seg67bHost& host, void* state,
                                                     void* approach);
void* bot_state_level_bomb_attack_run_construct_009b49b0(Cc13Seg67bHost& host, void* state,
                                                         void* approach);
void* bot_state_move_on_path_circle_construct_009bc960(Cc13Seg67bHost& host, void* state,
                                                       void* approach);
// 009B9680: __fastcall(ECX = task), RET, AL.
bool bot_task_maneuver_roll_completed_009b9680(Cc13Seg67bHost& host, void* task);
// 009BB380: __fastcall(ECX = unit, EDX = float point[3]; float limit, byte flag), RET 8.
void pilot_bot_prepend_tight_turn_to_point_009bb380(Cc13Seg67bHost& host, void* unit,
                                                    const float* point, float limit,
                                                    std::uint8_t flag);
// 009C25D0: __thiscall(ECX = state, approach), RET 4, EAX = state.
void* bot_state_move_to_circle_construct_009c25d0(Cc13Seg67bHost& host, void* state,
                                                  void* approach);
// 009AFA50: __fastcall(ECX = task), RET, EAX = state.
void* bot_task_land_pick_cruise_state_009afa50(Cc13Seg67bHost& host, void* task);
// 0099B260: __fastcall(ECX = slot), RET, ST0.
float pilot_bot_override_or_value_0099b260(void* slot);
// 0099A9B0: __thiscall(ECX = plan, entity), RET 4.
void pilot_plan_set_observed_endpoint_0099a9b0(Cc13Seg67bHost& host, void* plan, void* entity);
// 0099A9E0: __thiscall(ECX = plan, const plan* src), RET 4.
void pilot_plan_copy_endpoint_0099a9e0(Cc13Seg67bHost& host, void* plan, void* src);
// 0099BA10: __fastcall(ECX = out, EDX = in), RET, EAX = out.
float* math_normalize_angle_to_pi_by_ref_0099ba10(float* out, const float* in);
// 009DA880: __fastcall(ECX = node), RET.
void ship_ai_path_node_destroy_009da880(Cc13Seg67bHost& host, void* node);
// 009DF8F0: __cdecl(), RET, EAX = node.
void* ship_ai_traffic_list_head_node_009df8f0(Cc13Seg67bHost& host);
// 00999AE0: __fastcall(ECX = bot), RET, ST0.
float pilot_bot_first_task_wait_value_00999ae0(Cc13Seg67bHost& host, void* bot);
// 0099BB00: __thiscall(ECX = unit, float v), RET 4.
void plane_latch_yaw_rate_multiplier_0099bb00(void* unit, float v);
// 009DB470: __fastcall(ECX = brain), RET.
void ship_ai_brain_destruct_members_009db470(Cc13Seg67bHost& host, void* brain);

}  // namespace bsp
