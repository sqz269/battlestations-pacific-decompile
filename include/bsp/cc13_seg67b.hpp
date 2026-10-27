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
    // 007BCE20: __thiscall(ECX = unit), ST0: the minimum MaxWaterHitVel over the unit's
    // torpedo devices (FLT_MAX with none).
    virtual float unit_min_torpedo_hit_velocity_007bce20(void* unit) = 0;
    // 009D02A0 / 009D0160: __thiscall(ECX = torpedo approach): the aim-error redraw (draws
    // from 00BD2F10) and the run-time seed.
    virtual void approach_torpedo_aim_error_draw_009d02a0(void* approach) = 0;
    virtual void approach_torpedo_reset_run_time_009d0160(void* approach) = 0;
    // 009FB200: __thiscall(ECX = approach target ref, target), RET 4.
    virtual void approach_target_ref_construct_009fb200(void* ref, void* target) = 0;
    // 007C2610: __thiscall(ECX = unit), ST0: the unit's minimum kind-21 component speed.
    virtual float unit_min_kind21_component_speed_007c2610(void* unit) = 0;
    // 009CA3B0: __thiscall(ECX = strafe approach): the glide seed (draws from 00BD2F10).
    virtual void bot_approach_strafe_seed_glide_009ca3b0(void* approach) = 0;
    // 009BC780: __thiscall(ECX = move-on-path approach, [bot+50h]), RET 4.
    virtual void bot_approach_move_on_path_construct_009bc780(void* block, void* arg) = 0;
    // 009A3390: __thiscall(ECX = depth-charge approach, [bot+50h], target), RET 8.
    virtual void bot_approach_depth_charge_construct_009a3390(void* block, void* arg,
                                                              void* target) = 0;
    // 006C0B50: __thiscall(ECX = air ops, arg), RET 4, EAX: queue an incoming plane.
    virtual std::uintptr_t air_ops_queue_incoming_006c0b50(void* air_ops, void* arg) = 0;
    // 009AFAF0: __thiscall(ECX = land approach, flag), RET 4: the approach geometry.
    virtual void bot_approach_land_geometry_009afaf0(void* approach, std::uint32_t flag) = 0;
    // 009AB920 BSP_BotApproachKamikaze_ConstructBase: __thiscall(ECX = kamikaze approach,
    // [bot+50h], target), RET 8 (draws through 009AB850 and the aim errors).
    virtual void bot_approach_kamikaze_construct_base_009ab920(void* block, void* arg,
                                                               void* target) = 0;
    // 009C9D00: __thiscall(ECX = task, bot), RET 4, EAX = task: the retreat task
    // constructor (size 564h).
    virtual void* bot_task_retreat_construct_009c9d00(void* task, void* bot) = 0;
    // 009FA260 BSP_BotApproachTargetRef_PickBodyFrameAimOffset: __thiscall(ECX = target
    // reference), RET.
    virtual void approach_ref_pick_aim_offset_009fa260(void* ref) = 0;
    // 009E5530 BSP_ShipAi_AttackMoveRingConstruct: __thiscall(ECX = nested, owner), RET 4.
    virtual void ship_ai_attackmove_ring_construct_009e5530(void* nested, void* owner) = 0;
    // 009DBDB0 BSP_ShipAiPathPlan_DestructBody: __thiscall(ECX = path plan), RET.
    virtual void ship_ai_path_plan_destruct_009dbdb0(void* plan) = 0;
    // 0072BBD0 BSP_GunBot_ConstructBase: __thiscall(ECX = bot, class descriptor), RET 4.
    virtual void gun_bot_construct_base_0072bbd0(void* bot, std::uint32_t descriptor) = 0;
    // 008FBC80 BSP_GunBot_Attach: __thiscall(ECX = bot, tick node), RET 4.
    virtual void gun_bot_attach_008fbc80(void* bot, void* node) = 0;
    // 009E4330 BSP_ShipAi_NavigationBlockConstruct: __thiscall(ECX = block, ship), RET 4.
    virtual void ship_ai_nav_block_construct_009e4330(void* block, void* ship) = 0;
    // 009F126B..009F1401, the tail of 009F1160: bind to
    // bsp::ship_ai_brain_produce_tail_009f126b (src/ship_ai_brain_producers.cpp). The goal
    // defaults, the seven stream-1 draws and the critical section at brain+4h.
    // The (void* brain) signature differs from bsp::ship_ai_brain_produce_tail_009f126b(
    // ShipAiBrainProducerView, const ShipAiBrainProducerContext&): whoever binds this hook
    // must build the view over the brain record and supply the context.
    virtual void ship_ai_brain_produce_tail_009f126b(void* brain) = 0;
    // task->vtable[+28h](), RET, AL (a bot task's own predicate slot).
    virtual std::uint8_t task_vcall_28_flag(void* task) = 0;
    // task->vtable[+3Ch](), RET, EAX.
    virtual std::uint32_t task_vcall_3c(void* task) = 0;
    // task->vtable[+40h](), RET, EAX.
    virtual std::uint32_t task_vcall_40(void* task) = 0;
    // 00BF55BE operator new[](size), cdecl, EAX.
    virtual void* operator_new_array_00bf55be(std::uint32_t size) = 0;
    // 00BF6989 operator delete[](p), cdecl.
    virtual void operator_delete_array_00bf6989(void* p) = 0;
    // 009FC7C0 BSP_BotTaskGun_Tick: __thiscall(ECX = gun (task+314h), float dt), RET 4.
    virtual void bot_task_gun_tick_009fc7c0(void* gun, float dt) = 0;
    // 009FD0E0 opportunistic rocket tick: __thiscall(ECX = task+38Ch, float dt), RET 4.
    virtual void opportunistic_rocket_tick_009fd0e0(void* rocket, float dt) = 0;
    // 007D7A40 BSP_PilotPlan_ResetCommandTimer: __fastcall(ECX = plane+AB0h), RET.
    virtual void pilot_plan_reset_command_timer_007d7a40(void* controller) = 0;
    // 0071BE40 BSP_WeaponDirector_CurrentCommand: __fastcall(ECX = director), EAX.
    virtual std::uint32_t weapon_director_current_command_0071be40(void* director) = 0;
    // 0071EB60 BSP_EntityCommand_ActiveTargetDescriptor: __fastcall(ECX = director), EAX.
    virtual void* entity_command_active_target_descriptor_0071eb60(void* director) = 0;
    // 006F7DD0 BSP_TargetDescriptor_ResolvePosition: __thiscall(ECX = descriptor,
    // float* scratch), RET 4, EAX = the position (the scratch, a stored point or 00F87574).
    virtual const float* target_descriptor_resolve_position_006f7dd0(void* descriptor,
                                                                    float* scratch) = 0;
    // 0042B2F0 BSP_Vector3_LengthFloatThreshold: __fastcall(ECX = vector), RET, ST0.
    virtual float vector_length_0042b2f0(const float* v) = 0;
    // task->vtable[+0Ch](out), RET 4: the task's aim record {x, y, z, byte valid}.
    virtual void task_vcall_0c_aim(void* task, void* out16) = 0;
    // command->vtable[+0Ch](), RET, EAX.
    virtual std::uint32_t command_vcall_0c(void* command) = 0;
    // 008FE660 BSP_GunBot_ReadBaseProperties: __thiscall(ECX = bot, archive), RET 4.
    virtual void gun_bot_read_base_properties_008fe660(void* bot, void* archive) = 0;
    // 0099C690 BSP_BotTaskFactory_FindByName: __fastcall(ECX = counted string {length,
    // buffer}), EAX = factory (body: band0099::bot_task_factory_find_0099c690).
    virtual void* bot_task_factory_find_0099c690(void* name) = 0;
    // factory->vtable[+4h](bot, archive), RET 8, EAX = the loaded task.
    virtual void* factory_vcall_04_load(void* factory, void* bot, void* archive) = 0;
    // entity->vtable[+34h](float* scratch), RET 4, EAX = the velocity vector.
    virtual const float* entity_vcall_34_velocity(void* entity, float* scratch) = 0;
    // entity->vtable[+38h](), RET, ST0 (the value is discarded by 0099C527). For a unit
    // this is 007B8E60 BSP_Unit_GetCachedSpeed (FLD [ECX+B1Ch]), body
    // native_plane_cached_speed_007b8e60; 009BA2F0 reads it as the speed.
    virtual float entity_vcall_38_float(void* entity) = 0;
    // 009BE3E0 BSP_BotStateFollow_WaitValue: __fastcall(ECX = follow state), ST0.
    virtual float bot_state_follow_wait_value_009be3e0(void* state) = 0;
    // 0099B720: the task base wait value (FLD [00D7A260] = -1.0; RET), ST0.
    virtual float bot_task_base_wait_value_0099b720(void* task) = 0;
    // 007EFB30 BSP_PilotControl_RaiseForceReleaseFlag: __fastcall(ECX = unit), RET.
    virtual void pilot_control_raise_force_release_flag_007efb30(void* unit) = 0;
    // 009BCE30 / 009BDB90: scalar deleting destructors, __thiscall(ECX, flags), RET 4, EAX.
    virtual void* scalar_deleting_dtor_009bce30(void* object, std::uint32_t flags) = 0;
    virtual void* scalar_deleting_dtor_009bdb90(void* object, std::uint32_t flags) = 0;
    // object->vtable[0](), no stack arguments, EAX.
    virtual void* object_vcall_00(void* object) = 0;
    // 004BEF00: __thiscall(ECX = observer, subject), RET 4: zeroes the observer and
    // registers the pair (BSP_Observer_RegisterPair).
    virtual void observer_construct_004bef00(void* observer, void* subject) = 0;
    // 009ABAB0: __thiscall(ECX = drop-kamikaze approach, [bot+50h], target), RET 8.
    virtual void bot_approach_drop_kamikaze_construct_base_009abab0(void* block, void* arg,
                                                                    void* target) = 0;
    // 009C3DA0: __thiscall(ECX = dive-bomb approach): the aim-error redraw (draws from
    // 00BD2F10).
    virtual void approach_dive_bomb_aim_error_draw_009c3da0(void* approach) = 0;
    // 007F0F80: __thiscall(ECX = observer member, subject), RET 4.
    virtual void observer_member_initialize_007f0f80(void* member, void* subject) = 0;
    // 009FA350: __thiscall(ECX = approach target ref): reset the aim offset.
    virtual void approach_target_ref_reset_aim_offset_009fa350(void* ref) = 0;
    // 009B4400: __thiscall(ECX = level-bomb approach): the aim-error redraw.
    virtual void approach_level_bomb_aim_error_draw_009b4400(void* approach) = 0;
    // 007C1FB0: __thiscall(ECX = unit), ST0: the longest rack stick time.
    virtual float unit_longest_rack_stick_time_007c1fb0(void* unit) = 0;
    // 006BCD20: __fastcall(ECX = entity, DL = flag), EAX: the entity's air-ops block
    // (0 for a null entity).
    virtual void* air_ops_get_block_006bcd20(void* entity, bool flag) = 0;
    // 009AF9A0: __thiscall(ECX = land approach), RET: registers the eight land states.
    virtual void land_approach_register_states_009af9a0(void* approach) = 0;
    // 00410D80 / 00410E00: __thiscall(ECX = time/frequency pair, 0Ch reader or writer
    // block by value): read or write the pair; the callee destroys the block.
    virtual void time_freq_pair_read_00410d80(void* pair, void* ref) = 0;
    virtual void time_freq_pair_write_00410e00(void* pair, void* ref) = 0;
    // 009DE050: __thiscall(ECX = nav block, const float* goal2d, char keep_mode,
    // unsigned char final_leg), RET 0Ch.
    virtual void ship_ai_set_navigation_goal_009de050(void* nav, const float* goal,
                                                      std::uint8_t keep_mode,
                                                      std::uint8_t final_leg) = 0;
    // 00605070: __thiscall(ECX = float*), wraps the angle in place.
    virtual void math_wrap_angle_in_place_00605070(float* angle) = 0;
    // 00F8A68C: the ship-AI debug static block (colour records and guards,
    // 00F8A68C-00F8A703, 78h bytes).
    virtual std::uint8_t* ship_ai_debug_static_block_00f8a68c() = 0;
    // 00411E20: __thiscall(ECX = registry), the state-registry constructor.
    virtual void bot_state_registry_construct_00411e20(void* registry) = 0;
    // 007293D0: __thiscall(ECX = object), the tick-subnode owner base destructor.
    virtual void tick_subnode_owner_destruct_base_007293d0(void* object) = 0;
    // 009C2EF0: __thiscall(ECX = task+3F8h, bot, archive), RET 8.
    virtual void move_to_construct_approach_block_from_stream_009c2ef0(void* block, void* bot,
                                                                       void* archive) = 0;
    // 007B9320: __thiscall(ECX = weapon controller, int), RET 4, AL.
    virtual bool weapon_controller_has_general_bomb_ordnance_007b9320(void* controller,
                                                                      std::int32_t arg) = 0;
    // 007D9C10: __fastcall(ECX = unit+AB0h), RET.
    virtual void plane_flight_controller_refresh_body_frame_007d9c10(void* controller) = 0;
    // 009C2AC0: __thiscall(ECX = state, approach, target, float near, float far, float third),
    // RET 14h: the move-to state constructor.
    virtual void bot_state_move_to_construct_009c2ac0(void* state, void* approach, void* target,
                                                      float near_, float far_,
                                                      float third_) = 0;
    // 006C8800: __fastcall(ECX = squadron), RET: retires the landing squadron's blocks.
    virtual void air_ops_retire_landing_squadron_006c8800(void* squadron) = 0;
    // 009B2C80: __fastcall(ECX = task+3F8h), RET: the landing approach destructor.
    virtual void landing_approach_destroy_009b2c80(void* block) = 0;
    // 009BCE50: __thiscall(ECX = task, bot), RET 4, EAX = task.
    virtual void* move_on_path_task_construct_009bce50(void* task, void* bot) = 0;
    // 009ADBF0 / 009AEEF0: __thiscall(ECX = task, bot, target), RET 8, EAX = task.
    virtual void* drop_kamikaze_task_construct_009adbf0(void* task, void* bot, void* target) = 0;
    virtual void* kamikaze_task_construct_009aeef0(void* task, void* bot, void* target) = 0;
    // 007DF4F0: __thiscall(ECX = neighbour list at unit+C50h, pilot), RET 4.
    virtual void vehicle_avoidance_007df4f0(void* neighbours, void* pilot) = 0;
    // 009F9CE0: __thiscall(ECX = approach, arg, float reference speed), RET 8.
    virtual void bot_approach_construct_speed_reference_009f9ce0(void* approach, void* arg,
                                                                 float reference) = 0;
    // 009FB3E0: __thiscall(ECX = target ref, reader block by value [0Ch]), RET 0Ch.
    virtual void approach_ref_construct_from_stream_009fb3e0(void* ref, void* reader) = 0;
    // 009FACC0: __thiscall(ECX = target ref, writer block by value [0Ch]), RET 0Ch.
    virtual void approach_ref_save_009facc0(void* ref, void* writer) = 0;
    // 009A31A0: __fastcall(ECX = depth-charge approach), RET.
    virtual void bot_approach_depth_charge_pick_aim_offset_009a31a0(void* approach) = 0;
    // 00411450 / 00411170: __thiscall(ECX = registry, state machine, archive), RET 8.
    virtual void bot_state_registry_write_current_00411450(void* registry, void* machine,
                                                           void* archive) = 0;
    virtual void bot_state_registry_read_current_00411170(void* registry, void* machine,
                                                          void* archive) = 0;
    // 009A5240: __thiscall(ECX = task, bot, target), RET 8, EAX = task.
    virtual void* depth_charge_task_construct_009a5240(void* task, void* bot, void* target) = 0;
    // 00F8A720: the path-leg manager singleton.
    virtual void* path_leg_manager_00f8a720() = 0;
    virtual void set_path_leg_manager_00f8a720(void* manager) = 0;
    // 00415350: the singleton lifetime manager (critical section pointer at +10h).
    virtual void* singleton_manager_00415350() = 0;
    // EnterCriticalSection / LeaveCriticalSection through the imports 00CE2218 / 00CE2210.
    virtual void enter_critical_section_ce2218(void* section) = 0;
    virtual void leave_critical_section_ce2210(void* section) = 0;
    // 009FAA40: __fastcall(ECX = memory), RET, EAX = the manager.
    virtual void* bot_path_leg_manager_construct_009faa40(void* memory) = 0;
    // 00BD0C30: __thiscall(ECX = lifetime manager; instance), RET 4.
    virtual void singleton_register_00bd0c30(void* manager, void* instance) = 0;
    // 009FD050: __thiscall(ECX = approach, float d, const float* point), RET 8.
    virtual void bot_approach_refresh_path_leg_009fd050(void* approach, float d,
                                                        void* point) = 0;
    // 009BD400: __cdecl(), RET, EAX: the lazily built path-leg manager.
    virtual void* bot_path_leg_manager_get_009bd400() = 0;
    // 009FC260: __thiscall(ECX = manager, unit, const float* point, float w), RET 0Ch, ST0.
    virtual float bot_path_leg_manager_altitude_009fc260(void* manager, void* unit, void* point,
                                                         float w) = 0;
    // 00BF65AC: _free (operator delete), __cdecl(p).
    virtual void crt_free_00bf65ac(void* p) = 0;
    // 009E48A0: the std::list destructor instantiation, __fastcall(ECX = owner+offset).
    virtual void std_list_destroy_009e48a0(void* object) = 0;
    // 0041CC80: __fastcall(ECX = owned critical section holder), RET.
    virtual void critical_section_destroy_owned_0041cc80(void* object) = 0;
    // 006E0860: __fastcall(ECX = observer member), RET.
    virtual void observer_member_9348_destruct_006e0860(void* object) = 0;
    // 009E0130: __fastcall(ECX = member array), RET (no flags pushed at 009E550F).
    virtual void member_array_destruct_009e0130(void* object) = 0;
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

    // ---- packet cc13-seg67C ----
    // object->vtable[+114h](), RET, EAX: a unit's weapon director (no stack arguments).
    virtual void* object_vcall_114(void* object) = 0;
    // object->vtable[+28h](), RET, EAX.
    virtual void* object_vcall_28(void* object) = 0;
    // 0071BFF0 BSP_WeaponDirector_CommandPathCursor: __thiscall(ECX = director, arg), RET 4.
    virtual void* weapon_director_command_path_cursor_0071bff0(void* director,
                                                               std::uint32_t arg) = 0;
    // 007ACA30 BSP_PathObject_GetEntity: __thiscall(ECX = path object), EAX.
    virtual void* path_object_get_entity_007aca30(void* path) = 0;
    // 0085FB30 BSP_DebugDraw_LineRgbaFloatsOneColour: __fastcall(ECX = from, EDX = to,
    // colour[4] by value), RET 10h.
    virtual void debug_line_0085fb30(const float* from, const float* to,
                                     const float* colour) = 0;
    // 0085F990 BSP_DebugDraw_LineRgbaFloats: __fastcall(ECX = from, EDX = to, colour0[4],
    // colour1[4] by value), RET 20h.
    virtual void line_rgba_floats_0085f990(const float* from, const float* to,
                                           const float* colour0, const float* colour1) = 0;
    // 00860BB0 BSP_DebugDraw_CircleAroundAxis: __fastcall(ECX = centre, EDX = axis; radius,
    // rgba*, segments), RET 0Ch.
    virtual void circle_around_axis_00860bb0(const float* centre, const float* axis,
                                             float radius, const float* rgba,
                                             std::uint32_t segments) = 0;
    // 00860BE0 BSP_DebugDraw_WireSphereMarker: __fastcall(ECX = centre, EDX = rgba; radius,
    // rgba*, detailed byte), RET 0Ch.
    virtual void wire_sphere_00860be0(const float* centre, const float* rgba_edx, float radius,
                                      const float* rgba_stack, std::uint8_t detailed) = 0;
    // 004166E0 BSP_AvoidZoneDebug_DrawSelectedSegments: __thiscall(ECX = list head, rgba*),
    // RET 4.
    virtual void avoid_zone_debug_draw_selected_segments_004166e0(void* list_head,
                                                                  const float* rgba) = 0;
    // [00E0E300]: the debug side filter (a dword; negative draws every side).
    virtual std::int32_t global_debug_side_00e0e300() = 0;
    // 009A1A90 BSP_BotTask_DescribeAvoidFlags (the base slot +18h debug name):
    // __thiscall(ECX = task; string* result), RET 4, EAX = the 1Ch-byte string.
    virtual void* bot_task_debug_name_009a1a90(void* task, void* result) = 0;
    // 00411430 BSP_BotStateRegistry_NameOrUndefined: __thiscall(ECX = registry; state),
    // RET 4, EAX.
    virtual const char* state_registry_name_or_undefined_00411430(void* registry,
                                                                  void* state) = 0;
    // 007B7E70 operator+(const string&, const char*): __cdecl(out, left, right), EAX = out.
    virtual void* std_string_plus_cstr_007b7e70(void* out, const void* left,
                                                const char* right) = 0;
    // 0099B6E0 (TRIV_body_0099b6e0, a lone RET): the base debug-draw slot, __fastcall(ECX = task).
    virtual void bot_task_debug_draw_base_0099b6e0(void* task) = 0;
    // 008608B0 BSP_DebugDraw_CircleWithHeading: __thiscall(ECX = float* centre; float radius,
    // float x, y, z), RET 10h. It passes the radius to 00860710 with EDX = the static rgba
    // 00F875A8 (initialised once to 1, 0.5, 0, 1 under guard 00F875B8).
    virtual void debug_draw_circle_with_heading_008608b0(const float* centre, float radius,
                                                         float x, float y, float z) = 0;
    // 0099B660 BSP_BotTask_UpdateCruiseProfileBase: __fastcall(ECX = task), RET; empty.
    virtual void bot_task_update_cruise_profile_base_0099b660(void* task) = 0;
    // 009AAC70 BSP_BotTaskDogfight_UpdateApproach: __thiscall(ECX = task+3F8h; float dt),
    // RET 4.
    virtual void dogfight_update_approach_009aac70(void* approach, float dt) = 0;
    // 0099B740 BSP_BotTask_AbandonIfStale: __fastcall(ECX = task), RET.
    virtual void bot_task_abandon_if_stale_0099b740(void* task) = 0;
    // 00438B10 SubtractWrappedAngle: __stdcall(float a, float b), RET 8, ST0 = wrapped a - b.
    virtual float wrapped_angle_subtract_00438b10(float a, float b) = 0;
    // approach->vtable[0](out): __thiscall(ECX = approach; float* out), RET 4, EAX = the
    // target point (three floats).
    virtual float* approach_vcall_00_target_point(void* approach, float* out) = 0;
    // 00860710 BSP_DebugDraw_Circle64: __fastcall(ECX = centre, EDX = rgba; float radius),
    // RET 4.
    virtual void debug_circle64_00860710(const float* centre, const float* rgba,
                                         float radius) = 0;
    // node->vtable[0](flags): a ship-AI path node's scalar deleting destructor, RET 4.
    virtual void path_node_vcall_00_scalar_delete(void* node, std::uint32_t flags) = 0;
    // object->vtable[4](): the add-reference slot a 00D05704 holder calls, RET.
    virtual void ref_vcall_04_add_ref(void* object) = 0;
    // 006C2400 BSP_AirOpsHolder_DebugDrawRunwayDistances: __fastcall(ECX = holder), RET.
    virtual void air_ops_debug_draw_runway_distances_006c2400(void* holder) = 0;
    // object->vtable[0](), RET, AL: a condition object's test.
    virtual bool condition_vcall_00_test(void* condition) = 0;
    // 0099B690 BSP_BotTask_Retire: __fastcall(ECX = task), RET.
    virtual void bot_task_retire_0099b690(void* task) = 0;
    // 007ED3C0 BSP_PilotControl_ClearReleaseOrders: __fastcall(ECX = control), RET.
    virtual void pilot_control_clear_release_orders_007ed3c0(void* control) = 0;
    // 007BCBE0 BSP_Unit_SetQueuedReleaseOrders: __thiscall(ECX = unit; count), RET 4.
    virtual void unit_set_queued_release_orders_007bcbe0(void* unit, std::int32_t count) = 0;
    // 00438E40 BSP_NativeString_Duplicate: __fastcall(ECX = text), RET, EAX = the heap
    // copy (0 for a null text).
    virtual void* string_duplicate_00438e40(const char* text) = 0;
    // 009DA4E0 BSP_ShipAi_ClearPathPlan: __fastcall(ECX = navigator block), RET. Body:
    // ship_ai_clear_path_plan_009da4e0 (ship_ai_state_steps.hpp).
    virtual void ship_ai_clear_path_plan_009da4e0(void* blk) = 0;
    // 004C7070 BSP_DiagnosticPrintf: cdecl(a, float b, float c, fmt, ...).
    virtual void diagnostic_printf_004c7070(std::uint32_t a, float b, float c,
                                            const char* fmt) = 0;
    // The dword at 00E188A8, read just before the 004C7070 call.
    virtual std::uint32_t global_dword_00e188a8() = 0;
    // state->vtable[18h](x, y): __thiscall(ECX = state; int x, int y), RET 8.
    virtual void state_vcall_18_debug_text(void* state, std::int32_t x, std::int32_t y) = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint: __thiscall(ECX = point; float* out,
    // const matrix*), RET 8.
    virtual void vector_transform_affine_point_004142e0(const float* point, float* out,
                                                        const void* matrix) = 0;
    // object->vtable[4](), RET, EAX: the released object's observed-entity getter.
    virtual void* object_vcall_04_get(void* object) = 0;
    // 00419010 BSP_Math_InterpolateClamped: __stdcall(x0, y0, x1, y1, x), RET 14h, ST0.
    virtual float interpolate_clamped_00419010(float x0, float y0, float x1, float y1,
                                               float x) = 0;
    // 004B8490 BSP_FormatString256: cdecl(char* dst, fmt, ...), a 100h-byte vsprintf_s.
    // The plain form has no variadic tail; the _f64_ints form passes one double then
    // `count` dwords, in that order.
    virtual void format_string256_004b8490(char* dst, const char* fmt) = 0;
    virtual void format_string256_f64_ints_004b8490(char* dst, const char* fmt, double value,
                                                    const std::int32_t* ints,
                                                    std::size_t count) = 0;
    // 007B7CC0 STL_String_AppendPtrCount: __thiscall(ECX = string; ptr, count), RET 8.
    virtual void string_append_ptr_count_007b7cc0(void* str, const char* text,
                                                  std::size_t count) = 0;
    // 009D1500 BSP_BotApproachTorpedo_TimeToTarget: __fastcall(ECX = approach), RET, ST0.
    virtual float torpedo_approach_time_to_target_009d1500(void* approach) = 0;
    // 0051F460 BSP_DebugFeatureOwner_GetSingleton: cdecl(), EAX (+2Ch is the overlay byte).
    virtual void* debug_feature_owner_0051f460() = 0;
    // 00423520 BSP_AvoidZone_CornerOffsetPoint: __thiscall(ECX = zone; float* out, int
    // index, float distance), RET 0Ch, EAX = out (two floats).
    virtual float* avoid_zone_corner_offset_point_00423520(void* zone, float* out,
                                                           std::int32_t index,
                                                           float distance) = 0;
    // 009E9910 / 009E9A50: __thiscall(ECX = brain; float horizon), RET 4, EAX: the fore
    // or aft tubes ready within the horizon.
    virtual std::int32_t sub_tubes_ready_fore_009e9910(void* brain, float horizon) = 0;
    virtual std::int32_t sub_tubes_ready_aft_009e9a50(void* brain, float horizon) = 0;
    // 004C7070 with the name and `count` dwords after the format, in push order reversed
    // (the first dword is the first conversion after the %s).
    virtual void diagnostic_printf_name_ints_004c7070(std::uint32_t a, float b, float c,
                                                      const char* fmt, const char* name,
                                                      const std::int32_t* ints,
                                                      std::size_t count) = 0;
    // 00415690 BSP_Math_ClampInPlace: __fastcall(ECX = value, EDX = low; high), RET 4.
    virtual void math_clamp_in_place_00415690(float* value, const float* low,
                                              const float* high) = 0;
    // 00408720 BSP_CharacterString_AssignCounted: __thiscall(ECX = string; text, count),
    // RET 8, EAX = string.
    virtual void* string_assign_counted_00408720(void* str, const char* text,
                                                 std::size_t count) = 0;
    // 004D0860 std::operator+(const string&, const string&): __cdecl(out, left, right),
    // EAX = out.
    virtual void* string_plus_string_004d0860(void* out, const void* left,
                                              const void* right) = 0;
    // 0075B430 BSP_SessionMessage_ConstructBase: __thiscall(ECX = message; type), RET 4.
    virtual void session_message_construct_base_0075b430(void* message,
                                                         std::uint32_t type) = 0;
    // 00BF701A LIBCRT_atan2 (_CIatan2): ST0 = atan2(ST1, ST0).
    virtual float crt_atan2_00bf701a(float y, float x) = 0;
    // 007F0280 BSP_Bot_NearFieldUnitAvoidanceProbe: __thiscall(ECX = control; unit,
    // const float* extents, float* out_a, float* out_b, const float* weights, byte mode),
    // RET 18h (docs/BOT_PROBE_007F0280.md section 0.1).
    virtual void bot_near_field_probe_007f0280(void* control, void* unit, const float* extents,
                                               float* out_a, float* out_b,
                                               const float* weights, bool mode) = 0;
    // 009FB800 BSP_PilotBot_CommandPitchFromAltitude: __thiscall(ECX = state+4h; float
    // altitude, float reference), RET 8. The law is pitch_command_009fb800 (plane_flight.cpp).
    virtual void pilot_bot_command_pitch_from_altitude_009fb800(void* approach_ref,
                                                                float altitude,
                                                                float reference) = 0;
    // 009FABE0 BSP_Bot_SetDirectionFromHeadingPitch: __thiscall(ECX = direction; float
    // heading, float pitch), RET 8. Body: bot_set_direction_from_heading_pitch_009fabe0
    // (cc13_seg68.cpp).
    virtual void bot_set_direction_009fabe0(void* direction, float heading, float pitch) = 0;
    // 007C4810 BSP_PlaneClass_MinControlSpeed: __thiscall(ECX = class), RET, ST0. The
    // law is plane_min_control_speed_007c4810 (dive_bomb_aimdive_tail.hpp): tuning+28Ch
    // x class+184h StallSpd, rounded to float.
    virtual float plane_class_min_control_speed_007c4810(void* plane_class) = 0;
    // path->vtable[8](out, index): __thiscall(ECX = command path), RET 8; writes the
    // index's point to out (009DB060 then zeroes out[1]).
    virtual void path_vcall_08_point(void* path, float* out, std::int32_t index) = 0;
    // path->vtable[0Ch](): __thiscall(ECX = command path), RET, EAX = the point count.
    // The same slot as Cc13SweepB4Host::path_vcall_0c_count (cc13_sweepB4.hpp).
    virtual std::int32_t path_vcall_0c_count(void* path) = 0;
    // 0085FFF0 BSP_DebugDraw_ArrowRgbaFloatsOneColour: __fastcall(ECX = from, EDX = to;
    // rgba[4] by value), RET 10h. Body: debug_draw_arrow_rgba_floats_one_colour_0085fff0
    // (cc13_big53.hpp).
    virtual void debug_arrow_0085fff0(const float* from, const float* to,
                                      const float* rgba) = 0;
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
// 009A71E0: __thiscall(ECX = state; float dt), RET 4. The attack run's tick (slot +0Ch).
void bot_state_dogfight_attack_run_tick_009a71e0(Cc13Seg67bHost& host, void* state, float dt);
void* bot_state_dogfight_maneuver_construct_009a84e0(Cc13Seg67bHost& host, void* state,
                                                     void* approach);
void* bot_state_level_bomb_attack_run_construct_009b49b0(Cc13Seg67bHost& host, void* state,
                                                         void* approach);
void* bot_state_move_on_path_circle_construct_009bc960(Cc13Seg67bHost& host, void* state,
                                                       void* approach);
// 009B9680: __fastcall(ECX = task), RET, AL.
bool bot_task_maneuver_roll_completed_009b9680(Cc13Seg67bHost& host, void* task);
// The same body without the (unused) host, for the leaf fixture.
bool bot_task_maneuver_roll_completed_009b9680(void* task);
// 009B9740 (barrelroll, vtable 00D206D0 slot +64h) and 009BA2F0 (gainaltitude, vtable
// 00D20480 slot +64h): __thiscall(ECX = task; float dt), RET 4, dt unread. 009BA2F0
// takes one stream-1 draw in its no-target arm.
void bot_task_barrel_roll_tick_009b9740(Cc13Seg67bHost& host, void* task, float dt);
void bot_task_gain_altitude_tick_009ba2f0(Cc13Seg67bHost& host, void* task, float dt);
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
// 009E5110 / 009E5130 / 009E5460 / 009E8C10: __fastcall(ECX = object), RET.
void ship_ai_list_owner_destruct_009e5110(Cc13Seg67bHost& host, void* o);
void ship_ai_planner_destruct_009e5130(Cc13Seg67bHost& host, void* o);
void ship_ai_base_destruct_009e5460(Cc13Seg67bHost& host, void* o);
void ship_ai_brain_destruct_009e8c10(Cc13Seg67bHost& host, void* brain);
// 009BEBA0: __fastcall(ECX = approach), RET.
void bot_approach_refresh_target_009beba0(Cc13Seg67bHost& host, void* a);
// 009BD400: __cdecl(), RET, EAX. (009BEBA0 reaches it through the host hook.)
void* bot_path_leg_manager_get_body_009bd400(Cc13Seg67bHost& host);
// 009A35D0: __thiscall(ECX = approach, arg, archive), RET 8, EAX = approach.
void* bot_approach_depth_charge_construct_from_stream_009a35d0(Cc13Seg67bHost& host, void* a,
                                                               void* arg, void* archive);
// 009A5E60 / 009A5ED0: __thiscall(ECX = approach / task, archive), RET 4.
void bot_approach_depth_charge_save_009a5e60(Cc13Seg67bHost& host, void* a, void* archive);
void bot_task_depth_charge_save_009a5ed0(Cc13Seg67bHost& host, void* task, void* archive);
// 009A67C0: __fastcall(ECX = task), RET.
void bot_task_depth_charge_destruct_body_009a67c0(Cc13Seg67bHost& host, void* task);
// 009A6820: __thiscall(ECX = task, bot, archive), RET 8, EAX = task.
void* bot_task_depth_charge_construct_from_stream_009a6820(Cc13Seg67bHost& host, void* task,
                                                           void* bot, void* archive);
// 009A6970: __fastcall(ECX = bot, EDX = target), RET, EAX.
void* bot_task_make_depth_charge_009a6970(Cc13Seg67bHost& host, void* bot, void* target);
// 009A69E0: __stdcall(bot, archive), RET 8, EAX.
void* bot_task_load_depth_charge_009a69e0(Cc13Seg67bHost& host, void* bot, void* archive);
// 009A19B0: __fastcall(ECX = plan), RET, ST0.
float pilot_command_heading_diff_from_live_plane_009a19b0(Cc13Seg67bHost& host, void* plan);
// 0099B670: __fastcall(ECX = pilot task), RET.
void pilot_bot_avoid_vehicles_0099b670(Cc13Seg67bHost& host, void* p);
// 009AB370 / 009AEB80 / 009AF610: __fastcall(ECX = task), RET.
void bot_task_dogfight_destruct_body_009ab370(Cc13Seg67bHost& host, void* task);
void bot_task_drop_kamikaze_destruct_body_009aeb80(Cc13Seg67bHost& host, void* task);
void bot_task_kamikaze_destruct_body_009af610(Cc13Seg67bHost& host, void* task);
// 009AB3D0 / 009AEAE0: __thiscall(ECX = task, bot, archive), RET 8, EAX = task.
void* bot_task_dogfight_construct_from_stream_009ab3d0(Cc13Seg67bHost& host, void* task,
                                                       void* bot, void* archive);
void* bot_task_drop_kamikaze_construct_from_bag_009aeae0(Cc13Seg67bHost& host, void* task,
                                                         void* bot, void* archive);
// 009AB630 / 009AEC50: __stdcall(bot, archive), RET 8, EAX.
void* bot_task_dogfight_factory_create_009ab630(Cc13Seg67bHost& host, void* bot, void* archive);
void* bot_task_make_drop_kamikaze_from_bag_009aec50(Cc13Seg67bHost& host, void* bot,
                                                    void* archive);
// 009AEBE0 / 009AF720: __fastcall(ECX = bot, EDX = target), RET, EAX.
void* bot_task_make_drop_kamikaze_009aebe0(Cc13Seg67bHost& host, void* bot, void* target);
void* bot_task_make_kamikaze_009af720(Cc13Seg67bHost& host, void* bot, void* target);
// 009A98F0: __fastcall(ECX = aim state), RET, EAX.
bool bot_state_dogfight_aim_is_bored_009a98f0(void* state);
// 009B1E30: __fastcall(ECX = land approach), RET, EAX.
bool land_target_is_kind9_009b1e30(Cc13Seg67bHost& host, void* o);
// 009B3F50 / 009B8E80 / 009BD9D0: __fastcall(ECX = task), RET.
void bot_task_land_destruct_body_009b3f50(Cc13Seg67bHost& host, void* task);
void bot_task_level_bomb_destruct_body_009b8e80(Cc13Seg67bHost& host, void* task);
void bot_task_move_on_path_destroy_009bd9d0(Cc13Seg67bHost& host, void* task);
// 009B4120 / 009B8EE0 / 009BDA30 / 009BA960 / 009BAAB0: __thiscall(ECX = task, bot,
// archive), RET 8, EAX = task.
void* bot_task_land_construct_from_stream_009b4120(Cc13Seg67bHost& host, void* task, void* bot,
                                                   void* archive);
void* bot_task_level_bomb_construct_from_stream_009b8ee0(Cc13Seg67bHost& host, void* task,
                                                         void* bot, void* archive);
void* bot_task_move_on_path_construct_from_stream_009bda30(Cc13Seg67bHost& host, void* task,
                                                           void* bot, void* archive);
void* bot_task_immelmann_construct_from_stream_009ba960(Cc13Seg67bHost& host, void* task,
                                                        void* bot, void* archive);
void* bot_task_gain_altitude_construct_from_stream_009baab0(Cc13Seg67bHost& host, void* task,
                                                            void* bot, void* archive);
// 009B4230 / 009B90A0 / 009BDC10: __stdcall(bot, archive), RET 8, EAX.
void* bot_task_land_factory_create_009b4230(Cc13Seg67bHost& host, void* bot, void* archive);
void* bot_task_level_bomb_factory_create_009b90a0(Cc13Seg67bHost& host, void* bot,
                                                  void* archive);
void* bot_task_move_on_path_create_from_stream_009bdc10(Cc13Seg67bHost& host, void* bot,
                                                        void* archive);
// 009B9030: __fastcall(ECX = bot, EDX = target), RET, EAX.
void* bot_task_make_level_bomb_009b9030(Cc13Seg67bHost& host, void* bot, void* target);
// 009BDBB0 / 009BADB0: __fastcall(ECX = bot), RET, EAX.
void* bot_task_move_on_path_create_009bdbb0(Cc13Seg67bHost& host, void* bot);
void* bot_task_make_stop_009badb0(Cc13Seg67bHost& host, void* bot);
// 009BACB0: __fastcall(ECX = bot, EDX = target; float alt), RET 4, EAX. 009BAD30:
// __fastcall(ECX = bot), RET, EAX. Both unreferenced; each takes the base's two draws.
void* bot_task_make_gain_altitude_009bacb0(Cc13Seg67bHost& host, void* bot, void* target,
                                           float alt);
void* bot_task_make_immelmann_009bad30(Cc13Seg67bHost& host, void* bot);
// 009BD300: __thiscall(ECX = approach, archive), RET 4.
void bot_approach_move_on_path_save_offsets_009bd300(Cc13Seg67bHost& host, void* a,
                                                     void* archive);
// 009BC7E0: __thiscall(ECX = approach, arg, archive), RET 8, EAX = approach.
void* bot_approach_move_on_path_construct_from_stream_009bc7e0(Cc13Seg67bHost& host, void* a,
                                                               void* arg, void* archive);
// 009B7A80 / 009B7B10 / 009B7B40: __fastcall(ECX = state), RET, EAX.
bool bot_state_level_bomb_prepare_is_waiting_009b7a80(void* state);
bool bot_state_level_bomb_aim_should_release_009b7b10(void* state);
bool bot_state_level_bomb_aim_should_abort_009b7b40(void* state);
// 009B6C10: __thiscall(ECX = state, approach, target, float near, float far, float third),
// RET 14h, EAX = state.
void* bot_state_level_bomb_move_to_construct_009b6c10(Cc13Seg67bHost& host, void* s,
                                                      void* approach, void* target, float near_,
                                                      float far_, float third_);
// 009B7480: __thiscall(ECX = state, approach, reader handle by value [0Ch]), RET 10h,
// EAX = state.
void* bot_state_level_bomb_run_construct_from_stream_009b7480(Cc13Seg67bHost& host, void* s,
                                                              void* approach, void* handle);
// 009C8B60: __fastcall(ECX = task), RET.
void bot_task_dive_bomb_destruct_body_009c8b60(Cc13Seg67bHost& host, void* task);
// 009C8C70: __fastcall(ECX = bot, EDX = target), RET, EAX.
void* bot_task_make_dive_bomb_009c8c70(Cc13Seg67bHost& host, void* bot, void* target);
// 009C2CA0: __thiscall(ECX = state, approach, target, float near, float far, float third),
// RET 14h, EAX = state.
void* bot_state_dogfight_move_to_construct_009c2ca0(Cc13Seg67bHost& host, void* s,
                                                    void* approach, void* target, float near_,
                                                    float far_, float third_);
// 009C2D00: __thiscall(ECX = state, approach, reader handle by value), RET 10h.
void* bot_state_move_to_construct_from_stream_009c2d00(Cc13Seg67bHost& host, void* s,
                                                       void* approach, void* handle);
// 009C3990: __thiscall(ECX = task, bot, archive), RET 8, EAX = task.
void* bot_task_move_to_construct_from_stream_009c3990(Cc13Seg67bHost& host, void* task,
                                                      void* bot, void* archive);
// 009C3CA0: __stdcall(bot, archive), RET 8, EAX.
void* bot_task_move_to_factory_create_009c3ca0(Cc13Seg67bHost& host, void* bot, void* archive);
// 009C4140: __thiscall(ECX = state, approach), RET 4, EAX = state.
void* bot_state_dive_bomb_attack_run_construct_009c4140(Cc13Seg67bHost& host, void* state,
                                                        void* approach);
// 009C7850: __fastcall(ECX = aim-glide state), RET, EAX.
bool bot_state_dive_bomb_aim_glide_out_of_bombs_009c7850(Cc13Seg67bHost& host, void* state);
// 009BE050: __thiscall(ECX = unit, const float v[3]), RET 4.
void plane_unit_copy_leader_velocity_009be050(Cc13Seg67bHost& host, void* unit,
                                              const float* v);
// 009BE0F0: __thiscall(ECX = target, entity), RET 4, EAX = target.
void* move_to_target_construct_from_entity_009be0f0(Cc13Seg67bHost& host, void* t,
                                                    void* entity);
// 009B7990 / 009C7710: __thiscall(ECX = task, bot, target), RET 8, EAX = task.
void* bot_task_level_bomb_construct_009b7990(Cc13Seg67bHost& host, void* task, void* bot,
                                             void* target);
void* bot_task_dive_bomb_construct_009c7710(Cc13Seg67bHost& host, void* task, void* bot,
                                            void* target);
struct ShipAiPathPlanBlock;

// 009CFF40: __fastcall(ECX = owner, EDX = flag), RET, EAX = task or 0.
void* pilot_bot_create_takeoff_task_009cff40(Cc13Seg67bHost& host, void* owner,
                                             std::uint32_t flag);
// 009D5AE0 / 009D5B90: __thiscall(ECX = plan block), RET, AL.
bool ship_ai_path_any_positive_side_ahead_009d5ae0(ShipAiPathPlanBlock& plan);
bool ship_ai_path_any_negative_side_ahead_009d5b90(ShipAiPathPlanBlock& plan);
// cc13-fixture8: the two walks decide an undecided node with their own inline copy of the
// direction rule (009D5B29-009D5B56 / 009D5BD9-009D5C06), whose JC takes +1 when the two
// route sums are unordered (a NaN cost, or +inf and -inf in one sum), as the image's 009D9E6C
// also does. The C++ shared rule (ship_ai_path_node_direction_009d9e6c, whose `<` deviates
// from the image) gives -1 there. OFF: the walks keep the C++ shared rule. ON: the inline
// rule, so an unordered pair picks link_plus. Live: game_hosts_ship_ai.cpp ShipAiApproachPoint arc
// centre (009E47D3/009E47E4), under kShipAiArcCentrePlanTestCc13Bound. Only unordered sums
// differ.
inline constexpr bool kShipAiPathSideWalkNanCc13Bound = false;
// The walks with the inline rule regardless of the switch (the fixture's ON arm).
bool ship_ai_path_any_positive_side_ahead_image_009d5ae0(ShipAiPathPlanBlock& plan);
bool ship_ai_path_any_negative_side_ahead_image_009d5b90(ShipAiPathPlanBlock& plan);
// 009CA1A0 / 009CD1F0 / 009CFEE0 / 009D4DB0: __thiscall(ECX = task), RET.
void bot_task_retreat_destruct_body_009ca1a0(Cc13Seg67bHost& host, void* task);
void bot_task_strafe_destruct_body_009cd1f0(Cc13Seg67bHost& host, void* task);
void bot_task_takeoff_destruct_body_009cfee0(Cc13Seg67bHost& host, void* task);
void bot_task_torpedo_destruct_body_009d4db0(Cc13Seg67bHost& host, void* task);
// 009F3E80: __thiscall(ECX = gun bot), RET.
void ship_ai_gun_bot_destruct_body_009f3e80(Cc13Seg67bHost& host, void* bot);
// 009C2EF0: __thiscall(ECX = block, bot, archive), RET 8, EAX = block.
void* bot_task_move_to_construct_approach_block_from_stream_009c2ef0(Cc13Seg67bHost& host,
                                                                     void* block, void* bot,
                                                                     void* archive);
struct ShipAiPathNode;

// 009D5A80: __thiscall(ECX = node), RET, AL.
bool ship_ai_path_node_subtree_complete_009d5a80(void* node);
// 009D9440: __thiscall(ECX = node), RET, ST0.
float ship_ai_path_node_run_heading_009d9440(ShipAiPathNode& node);
// 009DE280: __thiscall(ECX = nav block, float heading, const float* goal, float penalty), RET 0Ch.
void ship_ai_set_navigation_goal_with_penalty_009de280(Cc13Seg67bHost& host, void* nav,
                                                       float heading, const float* goal,
                                                       float penalty);
// 009D5E10 / 009D5FD0: __cdecl(), RET, EAX = the static colour record.
float* ship_ai_debug_colour_record_a_009d5e10(Cc13Seg67bHost& host);
float* ship_ai_debug_colour_record_b_009d5fd0(Cc13Seg67bHost& host);
// 009E5E30: __thiscall(ECX = nested), RET, AL.
bool ship_ai_approach_target_is_kind_6_009e5e30(Cc13Seg67bHost& host, void* nested);
// 009C1DC0: __thiscall(ECX = approach, arg, archive), RET 8, EAX = approach.
void* bot_approach_move_to_construct_from_stream_009c1dc0(Cc13Seg67bHost& host, void* a,
                                                          void* arg, void* archive);
// 009C33D0: __thiscall(ECX = approach, archive), RET 4.
void bot_approach_move_to_save_009c33d0(Cc13Seg67bHost& host, void* a, void* archive);
// 009BCD50: __thiscall(ECX = task+3F8h, bot, archive), RET 8, EAX = block.
void* bot_task_move_on_path_construct_approach_block_from_stream_009bcd50(Cc13Seg67bHost& host,
                                                                          void* block, void* bot,
                                                                          void* archive);
// 009B77C0: __thiscall(ECX = task+3F8h, bot, archive), RET 8, EAX = block.
void* bot_task_level_bomb_construct_approach_block_from_stream_009b77c0(Cc13Seg67bHost& host,
                                                                        void* block, void* bot,
                                                                        void* archive);
// 009B3040: __thiscall(ECX = task+3F8h, [bot+50h], archive), RET 8, EAX = block.
void* bot_task_land_construct_approach_block_from_stream_009b3040(Cc13Seg67bHost& host,
                                                                  void* block, void* arg,
                                                                  void* archive);
// 009A5000: __thiscall(ECX = task+3F8h, bot, archive), RET 8, EAX = block.
void* bot_task_depth_charge_construct_approach_block_from_stream_009a5000(Cc13Seg67bHost& host,
                                                                          void* block, void* bot,
                                                                          void* archive);
// 009A9670: __thiscall(ECX = task+3F8h, bot, archive), RET 8, EAX = block.
void* bot_task_dogfight_construct_approach_block_from_stream_009a9670(Cc13Seg67bHost& host,
                                                                      void* block, void* bot,
                                                                      void* archive);
// 009B75E0: __thiscall(ECX = task+3F8h, bot, target), RET 8, EAX = block.
void* bot_approach_level_bomb_construct_009b75e0(Cc13Seg67bHost& host, void* block, void* bot,
                                                 void* target);
// 009A94E0: __thiscall(ECX = task+3F8h, bot, arg), RET 8, EAX = block.
void* bot_task_dogfight_construct_approach_block_009a94e0(Cc13Seg67bHost& host, void* block,
                                                          void* bot, void* arg);
// 009C73A0: __thiscall(ECX = task+3F8h, bot, target), RET 8, EAX = block.
void* bot_task_dive_bomb_construct_approach_block_009c73a0(Cc13Seg67bHost& host, void* block,
                                                           void* bot, void* target);
// 009AD920: __thiscall(ECX = task+3F8h, bot, archive), RET 8, EAX = block.
void* bot_task_drop_kamikaze_construct_approach_block_from_stream_009ad920(Cc13Seg67bHost& host,
                                                                           void* block, void* bot,
                                                                           void* archive);
// 009AD720: __thiscall(ECX = task+3F8h, bot, target), RET 8, EAX = block.
void* bot_approach_drop_kamikaze_construct_009ad720(Cc13Seg67bHost& host, void* block, void* bot,
                                                    void* target);
// 009BCC60: __thiscall(ECX = task+3F8h, bot), RET 4, EAX = block.
void* bot_approach_move_on_path_construct_with_states_009bcc60(Cc13Seg67bHost& host, void* block,
                                                               void* bot);
// 009A4DC0: __thiscall(ECX = task+3F8h, bot, target), RET 8, EAX = block.
void* bot_approach_depth_charge_construct_with_states_009a4dc0(Cc13Seg67bHost& host,
                                                               void* block, void* bot,
                                                               void* target);
// 009B2E50: __thiscall(ECX = task+3F8h, arg1, arg2), RET 8, EAX = block.
void* bot_approach_land_construct_composite_009b2e50(Cc13Seg67bHost& host, void* block,
                                                     void* arg1, void* arg2);
// 009CC230 / 009D3050: __thiscall(ECX = task, bot, target), RET 8, EAX = task.
void* bot_task_strafe_construct_009cc230(Cc13Seg67bHost& host, void* task, void* bot,
                                         void* target);
void* bot_task_torpedo_construct_009d3050(Cc13Seg67bHost& host, void* task, void* bot,
                                          void* target);
// 009CF8E0: __thiscall(ECX = task, owner, flag byte), RET 8, EAX = task.
void* bot_task_takeoff_construct_009cf8e0(Cc13Seg67bHost& host, void* task, void* owner,
                                          std::uint32_t flag);
// 009CC020 / 009D2DA0: __thiscall(ECX = block, bot, target), RET 8, EAX = block.
void* strafe_construct_approach_block_009cc020(Cc13Seg67bHost& host, void* block, void* bot,
                                               void* target);
void* torpedo_construct_approach_block_009d2da0(Cc13Seg67bHost& host, void* block, void* bot,
                                                void* target);
// 009CF710: __thiscall(ECX = block, arg), RET 4, EAX = block.
void* takeoff_construct_approach_block_009cf710(Cc13Seg67bHost& host, void* block, void* arg);
// 009CB500 / 009CAC80 / 009D06D0: __thiscall(ECX = state, owner block), RET 4, EAX = state.
void* bot_state_strafe_go_away_construct_009cb500(Cc13Seg67bHost& host, void* state,
                                                  void* block);
void* bot_state_strafe_attack_run_construct_009cac80(Cc13Seg67bHost& host, void* state,
                                                     void* block);
void* bot_state_torpedo_attack_run_construct_009d06d0(Cc13Seg67bHost& host, void* state,
                                                      void* block);
// 009CD420: __thiscall(ECX = approach, plane), RET 4, EAX = approach.
void* bot_approach_takeoff_construct_009cd420(Cc13Seg67bHost& host, void* approach,
                                              void* plane);
// 009CA4A0: __thiscall(ECX = approach, arg, target), RET 8, EAX = approach.
void* bot_approach_strafe_construct_009ca4a0(Cc13Seg67bHost& host, void* approach, void* arg,
                                             void* target);
// 009D0380: __thiscall(ECX = approach, arg, target), RET 8, EAX = approach.
void* bot_approach_torpedo_construct_009d0380(Cc13Seg67bHost& host, void* approach, void* arg,
                                              void* target);
// 009C3EA0: __thiscall(ECX = approach, plane, target), RET 8, EAX = approach.
void* bot_approach_dive_bomb_construct_009c3ea0(Cc13Seg67bHost& host, void* approach,
                                                void* plane, void* target);
// 009B44F0: __thiscall(ECX = approach, plane, target), RET 8, EAX = approach.
void* bot_approach_level_bomb_construct_base_009b44f0(Cc13Seg67bHost& host, void* approach,
                                                      void* plane, void* target);
// 009A6C10: __thiscall(ECX = approach, bot, arg), RET 8, EAX = approach.
void* bot_approach_dogfight_construct_009a6c10(Cc13Seg67bHost& host, void* approach, void* bot,
                                               void* arg);
// 009ABBE0: __thiscall(ECX = approach, plane, archive), RET 8, EAX = approach.
void* bot_approach_drop_kamikaze_construct_base_from_bag_009abbe0(Cc13Seg67bHost& host,
                                                                  void* approach, void* plane,
                                                                  void* archive);
// 009AFE70: __thiscall(ECX = approach, plane, air ops), RET 8, EAX = approach.
void* bot_approach_land_construct_lower_009afe70(Cc13Seg67bHost& host, void* approach,
                                                 void* plane, void* air_ops);
// 009AFFF0: __thiscall(ECX = approach, plane, archive), RET 8, EAX = approach.
void* bot_approach_land_construct_from_stream_009afff0(Cc13Seg67bHost& host, void* approach,
                                                       void* plane, void* archive);
// 009A6D40: __thiscall(ECX = approach, bot, archive), RET 8, EAX = approach.
void* bot_approach_dogfight_construct_from_stream_009a6d40(Cc13Seg67bHost& host, void* approach,
                                                           void* bot, void* archive);
// 009B4690: __thiscall(ECX = approach, arg, archive), RET 8, EAX = approach.
void* bot_approach_level_bomb_construct_from_stream_009b4690(Cc13Seg67bHost& host,
                                                             void* approach, void* arg,
                                                             void* archive);
// 009BD0B0: __fastcall(ECX = approach), RET, EAX = the path's entity or 0.
void* bot_approach_move_on_path_path_entity_009bd0b0(Cc13Seg67bHost& host, void* approach);
// 009D7260: __fastcall(ECX = box), RET.
void ship_ai_debug_draw_box_lines_009d7260(Cc13Seg67bHost& host, void* box);
// 009D98C0: __fastcall(ECX = node), RET; recursive.
void ship_ai_debug_draw_path_tree_009d98c0(Cc13Seg67bHost& host, void* node);
// 009DD9C0: __fastcall(ECX = plan), RET.
void ship_ai_debug_draw_plan_009dd9c0(Cc13Seg67bHost& host, void* plan);
// 009DDCC0: __fastcall(ECX = ship AI), RET.
void ship_ai_debug_draw_009ddcc0(Cc13Seg67bHost& host, void* ai);

// Approach and task saves: __thiscall(ECX = approach or task, archive), RET 4.
void bot_approach_dogfight_save_009aa410(Cc13Seg67bHost& host, void* a, void* archive);
void bot_task_dogfight_save_009aa5c0(Cc13Seg67bHost& host, void* task, void* archive);
void bot_approach_drop_kamikaze_save_009ae2d0(Cc13Seg67bHost& host, void* a, void* archive);
void bot_task_drop_kamikaze_save_009ae4c0(Cc13Seg67bHost& host, void* task, void* archive);
void bot_approach_land_save_009b37f0(Cc13Seg67bHost& host, void* a, void* archive);
void bot_task_land_save_009b3b90(Cc13Seg67bHost& host, void* task, void* archive);
void bot_approach_level_bomb_save_009b84e0(Cc13Seg67bHost& host, void* a, void* archive);
void bot_task_level_bomb_save_009b87b0(Cc13Seg67bHost& host, void* task, void* archive);
// 0099A660: __thiscall(ECX = pilot bot, task), RET 4 (thunk 0099A870).
void pilot_bot_retire_task_0099a660(Cc13Seg67bHost& host, void* bot, void* task);
// 009AED20: __thiscall(ECX = kamikaze approach, bot, target), RET 8, EAX = approach.
void* bot_approach_kamikaze_construct_009aed20(Cc13Seg67bHost& host, void* block, void* bot,
                                               void* target);
// 009C1C30: __thiscall(ECX = approach, unit, target descriptor), RET 8, EAX = approach.
void* bot_approach_moveto_construct_009c1c30(Cc13Seg67bHost& host, void* a, void* unit,
                                             void* target);
// 009CA2B0: __fastcall(ECX = bot), RET, EAX = the new retreat task or 0.
void* bot_task_make_retreat_009ca2b0(Cc13Seg67bHost& host, void* bot);
// 009CA780: __fastcall(ECX = strafe approach), RET.
void bot_approach_strafe_rearm_target_point_009ca780(Cc13Seg67bHost& host, void* a);
// 009CD300: __fastcall(ECX = bot, EDX = target), RET, EAX = the new strafe task or 0.
void* bot_task_make_strafe_009cd300(Cc13Seg67bHost& host, void* bot, void* target);
// 009D4E30: __fastcall(ECX = bot, EDX = target), RET, EAX = the new torpedo task or 0.
void* bot_task_make_torpedo_009d4e30(Cc13Seg67bHost& host, void* bot, void* target);
// 009DDBA0: __thiscall(ECX = ship AI), RET: the EH cleanup of the two path plans at +60h.
void ship_ai_destruct_path_plans_009ddba0(Cc13Seg67bHost& host, void* ai);
// 009E4F90: __thiscall(ECX = sub_attack parent, brain), RET 4, EAX = parent.
void* ship_ai_sub_attack_construct_009e4f90(Cc13Seg67bHost& host, void* p, void* brain);
// 009E5CA0: __thiscall(ECX = approach sub-state, owner), RET 4, EAX = sub.
void* ship_ai_attackmove_approach_construct_009e5ca0(Cc13Seg67bHost& host, void* sub,
                                                     void* owner);
// 009E8450: __thiscall(ECX = attackmove state, owner), RET 4, EAX = state.
void* ship_ai_attackmove_construct_009e8450(Cc13Seg67bHost& host, void* s, void* owner);
// 00997340: __thiscall(ECX = bot task, float dt), RET 4.
void bot_task_tick_gun_and_rocket_00997340(Cc13Seg67bHost& host, void* task, float dt);
// 009995A0: __thiscall(ECX = pilot bot, tick node), RET 4.
void pilot_bot_attach_009995a0(Cc13Seg67bHost& host, void* bot, void* node);
// The {data, count, capacity} pointer arrays: 00999700 insert at (RET 8, EAX = the slot),
// 00999810 grow (RET), 00999CC0 push back (RET 4), 00999D40 insert at the front (RET 4).
std::uint32_t* ptr_array_insert_at_00999700(Cc13Seg67bHost& host, void* array,
                                            std::uint32_t* where, const std::uint32_t* value);
void ptr_array_grow_00999810(Cc13Seg67bHost& host, void* array);
void ptr_array_push_back_00999cc0(Cc13Seg67bHost& host, void* array,
                                  const std::uint32_t* value);
void ptr_array_push_front_00999d40(Cc13Seg67bHost& host, void* array,
                                   const std::uint32_t* value);
// ---- one-line table slots (packet cc13-seg67C, section 3.45) ----
// Constant returns, __thiscall(ECX = task, unused), RET, EAX (009B9140: ST0).
std::uint32_t bot_task_depth_charge_factory_record_009a54e0() noexcept;
std::uint32_t bot_task_dogfight_factory_record_009a9950() noexcept;
std::uint32_t bot_task_depth_charge_name_009a5500() noexcept;
std::uint32_t bot_task_drop_kamikaze_name_009adde0() noexcept;
std::uint32_t bot_task_kamikaze_name_009af080() noexcept;
std::uint32_t bot_task_land_factory_record_009b33a0() noexcept;
std::uint32_t bot_task_level_bomb_name_009b7bd0() noexcept;
float bot_task_pull_up_slot68_009b9140() noexcept;
std::uint32_t bot_task_immelmann_name_009ba920() noexcept;
std::uint32_t bot_task_moveonpath_factory_record_009bcfb0() noexcept;
std::uint32_t bot_task_dive_bomb_name_009c79c0() noexcept;
std::uint32_t bot_task_retreat_kind_009c9e60() noexcept;
// Bare RET slots: takeoff's exit 009CE290 and the torpedo go-away enter 009D0C00.
void bot_state_takeoff_exit_009ce290(void* state) noexcept;
void bot_state_torpedo_go_away_enter_009d0c00(void* state) noexcept;
// Field slots, __thiscall(ECX = task or state).
std::uint32_t bot_task_dogfight_slot48_value_009a9c80(const void* task);
bool bot_task_dogfight_in_go_away_009a9d30(const void* task);
void bot_task_leave_alone_flush_009b9480(Cc13Seg67bHost& host, void* task);
// Adjustor thunks to the scalar deleting destructors, (ECX, flags), RET 4, EAX.
void* bot_approach_moveonpath_registry_dtor_thunk_009bcd40(Cc13Seg67bHost& host, void* self,
                                                           std::uint32_t flags);
void* bot_task_moveonpath_approach_view_dtor_thunk_009bd090(Cc13Seg67bHost& host, void* self,
                                                            std::uint32_t flags);
void* bot_task_moveonpath_registry_view_dtor_thunk_009bd0a0(Cc13Seg67bHost& host, void* self,
                                                            std::uint32_t flags);
// Wait values (slot +4Ch), ST0: the follow state's when current, else 0099B720.
float bot_task_depth_charge_wait_value_009a5d50(Cc13Seg67bHost& host, void* task);
float bot_task_moveonpath_follow_wait_value_009bd2e0(Cc13Seg67bHost& host, void* task);
float bot_task_retreat_wait_value_009ca0a0(Cc13Seg67bHost& host, void* task);
// 009C44C0: byte plane+844h = 0, byte state+1Ch = 0.
void bot_state_dive_bomb_turn_down_enter_009c44c0(void* state);
// 009B1E90: byte plane+844h = 1 (plane = [[state+4h]+4h]); twins 009C44E0/009C58A0/009C6210 ledger-only.
void bot_state_land_final_exit_009b1e90(void* state);
// 009D1350: byte [[state+4h]+A8h] = 0.
void bot_state_torpedo_aim_exit_009d1350(void* state);
// Command record accessors.
void command_record_set_float40_009d4ea0(void* record, float value);
void command_record_set_float48_009d4ec0(void* record, float value);
float command_record_get_float48_009d4f00(const void* record);
// 009D4F70: EAX = ai + A98h + [ai+B40h] * 54h.
void* ship_ai_current_order_slot_009d4f70(void* ai);
// The unreferenced navigation-block setters.
void ship_ai_set_torpedo_avoidance_request_009dabb0(void* block, std::uint8_t value);
void ship_ai_set_ship_avoidance_party_filter_009dabc0(void* block, std::uint32_t value);
void ship_ai_set_land_avoidance_request_009dabd0(void* block, std::uint8_t value);
void ship_ai_set_drive_bypass_009dabe0(void* block, std::uint8_t value);
void ship_ai_set_speed_commanded_flag_009dabf0(void* block);
// 009F5260: [o] = 00D21B40, [o+4h] = arg; EAX = o.
void* auto_target_base_construct_009f5260(void* object, std::uint32_t arg);
// 0099AA20: __thiscall(ECX = pilot bot, archive), RET 4.
void pilot_bot_load_tasks_0099aa20(Cc13Seg67bHost& host, void* bot, void* archive);
// 0099C3C0: __thiscall(ECX = lead record {owner, float speed, byte, out time +0Ch, out
// point +10h}, target entity), RET 4.
void bot_lead_point_on_target_0099c3c0(Cc13Seg67bHost& host, void* lead, void* target);
// 0099BDB0: __fastcall(ECX = table element), RET, EAX = element.
void* pilot_plan_table_construct_0099bdb0(void* table);
// 0099B1B0: __fastcall(ECX = plane), RET.
void plane_command_band_repair_if_controlled_0099b1b0(Cc13Seg67bHost& host, void* plane);
// 0099B2A0: __thiscall(ECX = five 0Ch-byte plan slots, unit), RET 4.
void pilot_bot_seed_slot_array_0099b2a0(void* slots, const void* unit);
// 0099C2C0: __fastcall(ECX = bot task), RET, EAX = 0, 1 or 2.
std::uint32_t bot_task_is_command_still_valid_base_0099c2c0(Cc13Seg67bHost& host, void* task);
// 00999A10: __fastcall(ECX = pilot bot), RET, AL.
bool pilot_bot_head_task_blocks_switch_00999a10(Cc13Seg67bHost& host, void* bot);
// 00999A60: __fastcall(ECX = park predicate), RET, AL.
bool pilot_park_predicate_plane_byte184_zero_00999a60(const void* predicate);
// 00999A80: __thiscall(ECX = park predicate, flags), RET 4, EAX = predicate.
void* pilot_park_predicate_scalar_delete_00999a80(Cc13Seg67bHost& host, void* predicate,
                                                  std::uint32_t flags);
// 00999B30: __fastcall(ECX = pilot bot), RET, AL (see the body for the no-match value).
std::uint8_t pilot_bot_any_task_slot28_00999b30(Cc13Seg67bHost& host, void* bot);
// 00999C70: __fastcall(ECX = pilot bot), RET, EAX = task or 0.
void* pilot_bot_find_task_by_slot3c_00999c70(Cc13Seg67bHost& host, void* bot);
// 0099A630: __fastcall(ECX = node), RET, EAX = node.
void* pilot_bot_init_observer_node_0099a630(void* node);
// 0099A940 / 0099A980: __thiscall(ECX = park predicate, archive), RET 4.
void pilot_park_predicate_read_plane_id_0099a940(Cc13Seg67bHost& host, void* predicate,
                                                 void* archive);
void pilot_park_predicate_write_plane_id_0099a980(Cc13Seg67bHost& host, void* predicate,
                                                  void* archive);
// 0099B3D0 / 0099B3E0: __thiscall(ECX = plan, byte), RET 4.
void pilot_plan_set_byte_2dc_0099b3d0(void* plan, std::uint8_t value);
void pilot_plan_set_turbo_request_0099b3e0(void* plan, std::uint8_t value);
// 009F1160: __thiscall(ECX = brain, ship), RET 4, EAX = brain: the brain record.
void* ship_ai_brain_record_construct_009f1160(Cc13Seg67bHost& host, void* brain, void* ship);
// 009F39C0: __thiscall(ECX = brain (ai+58h), owner ship), RET 4, EAX = brain.
void* ship_ai_brain_construct_009f39c0(Cc13Seg67bHost& host, void* brain, void* ship);
// 009F3BA0: __thiscall(ECX = ship AI, ship), RET 4, EAX = ai.
void* ship_ai_gun_bot_construct_009f3ba0(Cc13Seg67bHost& host, void* ai, void* ship);
// 009F3F20: __fastcall(ECX = ship), RET, EAX = the new 2268h-byte ship AI or null.
void* ship_ai_allocate_brain_009f3f20(Cc13Seg67bHost& host, void* ship);

// ---- medium rows, batch 1 (docs/CC13_SEG67B.md 3.46) ----
// The nine bot-task Describe slots (+18h overrides): __thiscall(ECX = task;
// string* out), RET 4, EAX = out, SEH frame. Each differs only in its state registry.
void* bot_task_depth_charge_describe_009a68c0(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_kamikaze_describe_009af670(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_drop_kamikaze_describe_009af890(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_level_bomb_describe_009b8f80(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_move_on_path_describe_009bdb00(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_move_to_describe_009c3a60(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_dive_bomb_describe_009c8bc0(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_retreat_describe_009ca200(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_strafe_describe_009cd250(Cc13Seg67bHost& host, void* task, void* out);
// 009A79F0: __fastcall(ECX = dogfight avoid state), RET. One draw on stream 1.
void bot_state_dogfight_avoid_enter_009a79f0(Cc13Seg67bHost& host, void* state);
// 009AAF30: __fastcall(ECX = dogfight task), RET (tail JMP 0099B740).
void bot_task_dogfight_update_cruise_profile_009aaf30(Cc13Seg67bHost& host, void* task);
// 009B35E0: __fastcall(ECX = land task), RET.
void bot_task_land_debug_draw_009b35e0(Cc13Seg67bHost& host, void* task);
// 009B93D0: __thiscall(ECX = task; float dt, unread), RET 4.
void bot_task_conditioned_tick_009b93d0(Cc13Seg67bHost& host, void* task, float dt);
// 009B93F0: __fastcall(ECX = task), RET, ST0.
float bot_task_tight_turn_slot68_009b93f0(void* task);
// 009B9420: __fastcall(ECX = task), RET.
void bot_task_leave_alone_update_cruise_profile_009b9420(Cc13Seg67bHost& host, void* task);
// 009B9FD0 / 009BD380 / 009C3510: __thiscall(ECX = task, archive), RET 4.
void bot_task_tight_turn_save_009b9fd0(Cc13Seg67bHost& host, void* task, void* archive);
void bot_task_move_on_path_save_009bd380(Cc13Seg67bHost& host, void* task, void* archive);
void bot_task_move_to_save_009c3510(Cc13Seg67bHost& host, void* task, void* archive);
// 009CA0C0: __thiscall(ECX = retreat task; out), RET 4, EAX = out (3 floats + a byte).
void* bot_task_retreat_target_position_009ca0c0(void* task, void* out);
// 009CA480: __fastcall(ECX = entity or null), RET, EAX = entity or 0.
void* entity_as_kind41_009ca480(Cc13Seg67bHost& host, void* entity);
// 009CFA10: __fastcall(ECX = takeoff task), RET.
void bot_task_takeoff_debug_draw_009cfa10(Cc13Seg67bHost& host, void* task);
// 009A5AF0 / 009D4230: __fastcall(ECX = task), RET.
void bot_task_depth_charge_debug_draw_009a5af0(Cc13Seg67bHost& host, void* task);
void bot_task_torpedo_debug_draw_009d4230(Cc13Seg67bHost& host, void* task);
// 009DB060: __thiscall(ECX = this; two stack arguments, unread), RET 8.
void ship_ai_debug_draw_command_path_009db060(Cc13Seg67bHost& host, void* self);
// 009E53B0: __thiscall(ECX = node storage; a, b, std::uint32_t lifetime bits), RET 0Ch,
// EAX = node.
void* ship_ai_obstacle_node_construct_unregistered_009e53b0(void* node, void* a, void* b,
                                                            std::uint32_t lifetime_bits);
// 009E8900: __thiscall(ECX = ship-AI debug object; int x, int y), RET 8.
void ship_ai_debug_draw_state_label_009e8900(Cc13Seg67bHost& host, void* self,
                                             std::int32_t x, std::int32_t y);

// ---- medium rows, batch 2 (docs/CC13_SEG67B.md 3.47) ----
// 009B94E0: __thiscall(ECX = split s task; byte wrap), RET 4.
void bot_task_split_s_roll_step_009b94e0(Cc13Seg67bHost& host, void* task, std::uint8_t wrap);
// The same body with 00438B10 supplied directly (the leaf fixture passes
// bsp::wrapped_angle_subtract_00438b10).
void bot_task_split_s_roll_step_009b94e0(void* task, std::uint8_t wrap,
                                         float (*wrapped_angle_subtract_00438b10)(float, float));
// 009B95D0: __thiscall(ECX = stop task; float dt, unread), RET 4.
void bot_task_stop_tick_009b95d0(void* task, float dt);
// 009BAE10 / 009BAE80 / 009BAEF0: __stdcall(bot, archive), RET 8, EAX = task or 0.
void* bot_task_gain_altitude_factory_create_009bae10(Cc13Seg67bHost& host, void* bot,
                                                     void* archive);
void* bot_task_immelmann_factory_create_009bae80(Cc13Seg67bHost& host, void* bot, void* archive);
void* bot_task_stop_factory_create_009baef0(Cc13Seg67bHost& host, void* bot, void* archive);
// 009BC6B0: __fastcall(ECX = move-on-path approach), RET.
void bot_approach_move_on_path_register_state_names_009bc6b0(Cc13Seg67bHost& host,
                                                             void* approach);
// 009BC870 / 009BC8A0 / 009C6180: __thiscall(ECX = state; approach), RET 4, EAX = state.
void* bot_state_move_on_path_base_construct_009bc870(void* state, void* approach);
void* bot_state_move_on_path_path_construct_009bc8a0(void* state, void* approach);
void* bot_state_dive_bomb_fly_above_construct_009c6180(void* state, void* approach);
// 009BC9F0 / 009C2670: __fastcall(ECX = circle state), RET.
void bot_state_move_on_path_circle_debug_draw_009bc9f0(Cc13Seg67bHost& host, void* state);
void bot_state_move_to_circle_debug_draw_009c2670(Cc13Seg67bHost& host, void* state);
// 009BCF70: __fastcall(ECX = move-on-path approach), RET, EAX = cursor or 0.
void* bot_approach_move_on_path_command_path_cursor_009bcf70(Cc13Seg67bHost& host,
                                                             void* approach);
// 009BD1D0: __fastcall(ECX = move-on-path task), RET.
void bot_task_move_on_path_debug_draw_009bd1d0(Cc13Seg67bHost& host, void* task);
// 009BDED0: __thiscall(ECX = move-to approach; entity), RET 4.
void bot_approach_move_to_set_observed_target_009bded0(Cc13Seg67bHost& host, void* approach,
                                                       void* entity);
// 009C5870: __fastcall(ECX = aimdive state), RET.
void bot_state_dive_bomb_aim_dive_enter_009c5870(void* state);
// 009C6220: __fastcall(ECX = const float* a, EDX = const float* b), RET, ST0.
float math_clamp_symmetric_009c6220(const float* a, const float* b);
// 009D58A0: __fastcall(ECX = path node), RET.
void ship_ai_path_node_destruct_body_009d58a0(Cc13Seg67bHost& host, void* node);
// 009D59C0: __fastcall(ECX = path node), RET, EAX = &node->x of the run's end node.
const float* ship_ai_path_node_run_end_point_009d59c0(ShipAiPathNode& node);
// 009D5CF0: __fastcall(ECX = ship-AI task), RET, AL.
bool ship_ai_path_has_no_sided_node_009d5cf0(void* task);
// 009D5D60: __fastcall(ECX = target ref record), RET. One draw on stream 1.
void target_ref_record_reset_009d5d60(Cc13Seg67bHost& host, void* record);
// 009D6530: __thiscall(ECX = holder; object), RET 4, EAX = holder.
void* ref_holder_construct_009d6530(Cc13Seg67bHost& host, void* holder, void* object);

// ---------------------------------------------------------------------------
// Medium rows, batch 3 (docs/CC13_SEG67B.md section 3.48)
// ---------------------------------------------------------------------------
// 009DA950: __fastcall(ECX = path node record), RET, EAX = record. One draw on stream 1.
void* ship_ai_path_node_init_fields_009da950(Cc13Seg67bHost& host, void* record);
// 009DBD60: __fastcall(ECX = object), RET, EAX = object.
void* navigator_bot_named_object_construct_009dbd60(Cc13Seg67bHost& host, void* object);
// 009DE1B0: __thiscall(ECX = navigator block; float throttle), RET 4.
void ship_ai_reset_path_and_set_throttle_009de1b0(Cc13Seg67bHost& host, void* blk,
                                                  float throttle);
// 009DFB90: __thiscall(ECX = observer member; entity, const float* point), RET 8.
void ship_ai_observer_member_set_observed_with_point_009dfb90(Cc13Seg67bHost& host,
                                                              void* member, void* entity,
                                                              const float* point);
// 009E2C40: __thiscall(ECX = ship-AI bot; int x, int y), RET 8.
void ship_ai_bot_debug_draw_with_state_name_009e2c40(Cc13Seg67bHost& host, void* bot,
                                                     std::int32_t x, std::int32_t y);
// 009E2F10: __thiscall(ECX = goal record member; released), RET 4.
void ship_ai_goal_member_on_observed_released_009e2f10(Cc13Seg67bHost& host, void* member,
                                                       void* released);
// 009E69D0: __thiscall(ECX = object; float angle, float peak), RET 8.
void heading_error_ramp_set_34_009e69d0(Cc13Seg67bHost& host, void* object, float angle,
                                        float peak);
// 009E6A30: __thiscall(ECX = object; float angle, float span, float peak), RET 0Ch.
void heading_error_ramp_set_38_009e6a30(Cc13Seg67bHost& host, void* object, float angle,
                                        float span, float peak);
// 009F3CC0: __fastcall(ECX = ship-AI gun bot), RET, EAX = name.
const char* ship_ai_gun_bot_sub_state_name_009f3cc0(Cc13Seg67bHost& host, void* bot);

// ---------------------------------------------------------------------------
// Medium rows, batch 4 (docs/CC13_SEG67B.md section 3.49)
// ---------------------------------------------------------------------------
// 009AB220 / 009B3FF0 / 009CFDD0 / 009D4C90: __thiscall(ECX = task; string* out), RET 4,
// EAX = out.
void* bot_task_dogfight_describe_009ab220(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_land_describe_009b3ff0(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_takeoff_describe_009cfdd0(Cc13Seg67bHost& host, void* task, void* out);
void* bot_task_torpedo_describe_009d4c90(Cc13Seg67bHost& host, void* task, void* out);
// 009AC7C0: __fastcall(ECX = kamikaze state), RET (009ACD90 is its exact twin).
void bot_state_kamikaze_go_towards_debug_draw_009ac7c0(Cc13Seg67bHost& host, void* state);
// 009BB4A0: __fastcall(ECX = unit, EDX = float point[3]; float limit), RET 4.
void pilot_bot_tight_turn_toward_point_009bb4a0(Cc13Seg67bHost& host, void* unit,
                                                const float* point, float limit);
// 009BB530: __fastcall(ECX = unit, EDX = float point[3]; float threshold, float limit),
// RET 8.
void pilot_bot_tight_turn_if_off_heading_009bb530(Cc13Seg67bHost& host, void* unit,
                                                  const float* point, float threshold,
                                                  float limit);
// 009D9390: __thiscall(ECX = path node; float plus_distance, float minus_distance), RET 8,
// AL.
bool ship_ai_path_node_resolve_turn_point_009d9390(Cc13Seg67bHost& host, void* node,
                                                   float plus_distance, float minus_distance);
// 009E7FA0: __thiscall(ECX = brain; float heading, const float* goal, float penalty),
// RET 0Ch.
void ship_ai_set_navigation_goal_with_penalty_brain_009e7fa0(Cc13Seg67bHost& host, void* brain,
                                                             float heading, const float* goal,
                                                             float penalty);
// 009EAAD0: __thiscall(ECX = sub-attack state machine; int x, int y), RET 8.
void ship_ai_sub_attack_debug_text_009eaad0(Cc13Seg67bHost& host, void* sm, std::int32_t x,
                                            std::int32_t y);
// The three former false twins of section 3.48 (docs section 3.50).
// 009DFA60 / 009DFA90: __fastcall(ECX = owning pointer slot), RET.
void ship_ai_path_node_delete_009dfa60(Cc13Seg67bHost& host, void* slot);
void observed_owner_delete_009dfa90(Cc13Seg67bHost& host, void* slot);
// 009E5220: __fastcall(ECX = list), RET, EAX = list.
void* ship_ai_traffic_list_construct_009e5220(Cc13Seg67bHost& host, void* list);

// ---------------------------------------------------------------------------
// Medium rows, batch 5 (docs/CC13_SEG67B.md section 3.51)
// ---------------------------------------------------------------------------
// 009B8260 / 009C80D0: __fastcall(ECX = task), RET.
void bot_task_level_bomb_debug_draw_009b8260(Cc13Seg67bHost& host, void* task);
void bot_task_dive_bomb_debug_draw_009c80d0(Cc13Seg67bHost& host, void* task);
// 009D6E50: __thiscall(ECX = 65-byte profile; float* angle), RET 4, ST0.
float byte_profile65_sample_009d6e50(Cc13Seg67bHost& host, const void* profile, float* angle);
// 009D7780 / 009D7930: __thiscall(ECX = record; const float* point), RET 4, ST0.
float oriented_rect_distance_to_point_009d7780(const void* rect, const float* point);
float oriented_rect2_distance_to_point_009d7930(const void* rect, const float* point);

// ---------------------------------------------------------------------------
// Batch 6 (docs/CC13_SEG67B.md section 3.52)
// ---------------------------------------------------------------------------
// 009BC5C0: __thiscall(ECX = task; string* out), RET 4, EAX = out.
void* bot_task_leave_alone_describe_009bc5c0(Cc13Seg67bHost& host, void* task, void* out);
// 009BDD70: __fastcall(ECX = moveto approach), RET.
void bot_approach_move_to_register_state_names_009bdd70(Cc13Seg67bHost& host, void* approach);
// 009D6650: __thiscall(ECX = message; float a, float b, float c, float d, int e), RET 14h,
// EAX = message.
void* session_message_9d_construct_009d6650(Cc13Seg67bHost& host, void* message, float a,
                                            float b, float c, float d, std::int32_t e);
// 009DBDB0: __fastcall(ECX = path plan), RET.
void ship_ai_path_plan_destruct_body_009dbdb0(Cc13Seg67bHost& host, void* plan);

}  // namespace bsp
