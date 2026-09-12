#pragma once

#include <cstdint>

namespace bsp {
// Where the ship AI's goal vector comes from, and how a goal becomes a path
// point. Semantic interfaces, not native object layouts. Names are hypotheses,
// not recovered symbols. Addresses, evidence, original ABI and uncertainty:
// docs/SHIP_AI_GOAL_VECTOR.md.
//
// The producer chain, in the order one AI tick runs it:
//   009F516C  the controller calls 009F1420 with the accumulated tick delta
//   009F146B  0071EB60(entity command slot) -> the active command descriptor
//   009F1473  009E2FB0 latches its position and resolved object on brain+0AF8h
//   009F156B  009DBCC0 turns that into a world position
//   009F1572  brain+0B2Ch / +0B30h / +0B34h, the goal vector
//   009E57B4  the movetopos state step reads it and hands it to 009DE050
//   009EE5C2  009ED3E0 hands the latched goal blk+1DCh to the path planner
//   009EE5F4  009E3C00 returns the next path point off the planned path
//   009EE66C  00815F30 publishes the lateral-offset bounds for that point
//
// This header owns the first six steps and the 009EE580..009EE670 tail.
// include/bsp/ship_ai_state_steps.hpp owns 009DE050 and the state steps;
// include/bsp/ship_ai_navigation.hpp owns the output block from 009EE671.

// ---------------------------------------------------------------------------
// The command descriptor 0071EB60 hands back
// ---------------------------------------------------------------------------
// 0071EB60 is __fastcall(slot) and returns slot+58h when slot+30h == 1,
// slot+18Ch when it is 2, and otherwise the lazily built empty singleton at
// 00E19B98, whose +1h byte and +14h handle are both zero. The two fields
// 009E2FB0 reads off it are the byte at +1h (009E2FBA) and the vec3 at +8h
// (009E2FBF). 00521EA0 BSP_CommandTarget_ResolveObject resolves the same
// descriptor to an object; its own reads are +0h, +2h and +4h.
struct ShipAiGoalCommandDescriptor {
    bool has_position{false};  // the byte at descriptor+1h
    float x{0.0f};             // descriptor+8h
    float y{0.0f};             // descriptor+0Ch
    float z{0.0f};             // descriptor+10h
    std::uint32_t target{0};   // what 00521EA0 resolves the descriptor to
};

// 009E2FC4 substitutes the 16 zero bytes at 00F87574 when has_position is
// clear. The address sits past the raw tail of `.data`, so the constant is the
// zero vector and every command without a position asks for the world origin.
inline constexpr float kShipAiGoalNoPositionX = 0.0f;  // 00F87574
inline constexpr float kShipAiGoalNoPositionY = 0.0f;
inline constexpr float kShipAiGoalNoPositionZ = 0.0f;

// ---------------------------------------------------------------------------
// The latched target record at brain+0AF8h
// ---------------------------------------------------------------------------
// 009F1465 LEA EBX,[EDI+0AF8h] is the only producer of the `this` both
// 009E2FB0 and 009DBCC0 take. Offsets below are relative to that record, with
// the brain displacement the listing shows in the comment.
struct ShipAiGoalTargetRecord {
    std::uint32_t target_object_14{0};  // +14h = brain+0B0Ch, 009E3004 / 009DBCC7
    float offset_x_18{0.0f};            // +18h = brain+0B10h, 009E3018 / 009DBCEA
    float offset_y_1c{0.0f};            // +1Ch = brain+0B14h
    float offset_z_20{0.0f};            // +20h = brain+0B18h
};

// ---------------------------------------------------------------------------
// The brain fields this packet owns
// ---------------------------------------------------------------------------
struct ShipAiGoalVectorState {
    // The goal vector itself. 009F1572 / 009F157B / 009F1584 are its only
    // writers outside the constructor 009F1160 (009F1290 / 009F12B9).
    float goal_x_0b2c{0.0f};
    float goal_y_0b30{0.0f};
    float goal_z_0b34{0.0f};
    // brain+0B38h, the byte 009F1457 clears at the head of every tick, the
    // cruise step sets at 009E11F2 and 009E13E4, and 009F4DAF reads to skip
    // the throttle ceiling. It is NOT the goal vector's valid flag.
    bool speed_commanded_0b38{false};
    // brain+0B20h, the raw command target; brain+0B24h, the same pointer after
    // vtable[5Ch](2). 009F1480 and 009F1499.
    std::uint32_t raw_target_0b20{0};
    std::uint32_t filtered_target_0b24{0};
    // brain+0B28h, the gate the refresh runs under: set unconditionally when
    // the period expires (009F14D2), then narrowed by the recon test.
    bool target_visible_0b28{true};
    // brain+0B54h, the refresh period; brain+0B58h, the countdown. The
    // constructor seeds the period from 00CE3958 (009F137A) and the countdown
    // from -00BD2F10(0, period) (009F138C), so ships stagger their refreshes.
    float refresh_period_0b54{2.0f};
    float refresh_countdown_0b58{0.0f};
};

inline constexpr float kShipAiGoalRefreshPeriod = 2.0f;  // 00CE3958, 009F1358
// 009F155C FLD1 / 009F155E FCOMIP: a stored goal shorter than one unit is
// treated as unset and forces a refresh even when the gate is closed.
inline constexpr float kShipAiGoalKeepLengthSq = 1.0f;  // 009F155C
// 009DB898 FLD qword [00CE3820]: the squared-distance floor under which
// 009DB820 answers exactly zero instead of taking a square root.
inline constexpr double kShipAiGoalDistanceEpsilonSq = 1e-10;  // 00CE3820

struct ShipAiGoalVectorHost {
    virtual ~ShipAiGoalVectorHost() = default;
    // 009F146B, 0071EB60 on [brain+0AB8h]: the entity's active command
    // descriptor, or the empty singleton. Body read: it is a three-way select
    // on slot+30h, it copies nothing and it allocates nothing.
    virtual ShipAiGoalCommandDescriptor active_command_0071eb60() = 0;
    // 009E2FE9, 00521EA0 BSP_CommandTarget_ResolveObject on that same
    // descriptor: the object the command's handle names, or zero.
    virtual std::uint32_t resolve_command_target_00521ea0() = 0;
    // 009E2FFD, 006952A0 BSP_Observer_UnregisterPair(target, record) and
    // 009E300D, 00694A60 BSP_Observer_RegisterPair(target, record): the record
    // observes the object it latched so it learns when the object dies.
    virtual void observer_unregister_006952a0(std::uint32_t target) = 0;
    virtual void observer_register_00694a60(std::uint32_t target) = 0;
    // 009F1491, target->vtable[5Ch](2): the kind filter the raw target passes
    // before the refresh will use it. Slot body not read; the argument is the
    // literal 2 at 009F148D.
    virtual bool target_is_kind_vtable_005c(std::uint32_t target, int kind) = 0;
    // 009F14DB / 009F14E4: the side word at +54h, read off the target and off
    // the unit at [brain+0AA8h].
    virtual int unit_side_0054() = 0;
    virtual int target_side_0054(std::uint32_t target) = 0;
    // 009F14EC, 008053C0 BSP_Recon_EnsureSlot(side) -> the recon slot, then
    // 009F14FA, 009DFBE0(slot)(target) -> nonzero when that side has the
    // target. 009DFBE0's body was not read; the call site takes the result as
    // a bool and stores it straight into brain+0B28h.
    virtual bool recon_knows_target_009dfbe0(int own_side, std::uint32_t target) = 0;
    // 009F1519, 00922DC0 BSP_Entity_IsSurfaceTarget on the raw target: a
    // surface target reopens the gate the recon test closed.
    virtual bool target_is_surface_00922dc0(std::uint32_t target) = 0;
    // 009DBCCE / 009DBCD9: the target's pose-valid byte at +0C8h and
    // 00414DB0 BSP_EntityPose_RefreshWorld when it is clear.
    virtual bool target_pose_valid_00c8(std::uint32_t target) = 0;
    virtual void refresh_target_pose_00414db0(std::uint32_t target) = 0;
    // 009DBCED, 004142E0 BSP_Vector3f_TransformAffinePoint with ECX = the
    // record's offset, the destination on the stack and the matrix at
    // target+0CCh: the stored offset read as a point in the target's frame.
    virtual void transform_by_target_matrix_004142e0(std::uint32_t target,
                                                     float in_x, float in_y, float in_z,
                                                     float& out_x, float& out_y,
                                                     float& out_z) = 0;
};

// 009E2FB0, `ShipAiGoalTargetRecord* __thiscall(record)(descriptor)`, RET 4.
// Latches the command's position and its resolved object on the record,
// re-registering the observer pair only when the object actually changed.
void ship_ai_latch_command_target_009e2fb0(const ShipAiGoalCommandDescriptor& command,
                                           std::uint32_t resolved_target,
                                           ShipAiGoalTargetRecord& record,
                                           ShipAiGoalVectorHost& host);

// 009DBCC0, `void __thiscall(record)(float out[3])`, RET 4. With no latched
// object the stored triple is already a world position; with one it is a point
// in that object's frame and the object's world matrix carries it out.
void ship_ai_resolve_goal_position_009dbcc0(const ShipAiGoalTargetRecord& record,
                                            ShipAiGoalVectorHost& host,
                                            float& out_x, float& out_y, float& out_z);

struct ShipAiGoalRefreshResult {
    bool timer_expired{false};   // the 009F14B6 branch went the long way
    bool goal_rewritten{false};  // 009F1572..009F1584 ran
};

// 009F1420's goal half, `void __thiscall(brain)(float seconds)`, RET 4, call
// site 009F516C. Covers 009F1420..009F158A only: the periodic threat scan from
// 009F158A to the 009F1BB8 return is not projected here.
ShipAiGoalRefreshResult ship_ai_refresh_goal_vector_009f1420(ShipAiGoalVectorState& state,
                                                             ShipAiGoalTargetRecord& record,
                                                             float seconds,
                                                             ShipAiGoalVectorHost& host);

// 009DB820, `float __thiscall(adapter)(void)`, RET, body 009DB820-009DB8CC.
// The planar distance from the unit's pose to the goal, with an exact zero
// under the epsilon. The caller refreshes the pose; this is the arithmetic.
float ship_ai_goal_planar_distance_009db820(float goal_x, float goal_z,
                                            float pose_x, float pose_z);

// ---------------------------------------------------------------------------
// 009EE580..009EE670: the goal becomes a path point
// ---------------------------------------------------------------------------
// The tail of BSP_ShipAi_ControlsStep's first half. It is reached only when the
// station-keeping arm at 009EDA28 did not run, because that arm leaves through
// 009EE57B JMP 009EF206 and never falls through.
struct ShipAiPathPickState {
    int steering_mode_1c4{0};   // blk+1C4h, compared against the literal 2
    bool astern{false};         // BL, 009ED7E9 SETZ on blk+1C4h == 3
    float pose_x_184{0.0f};     // blk+184h, the query position 009E3C00 takes
    float pose_z_188{0.0f};     // blk+188h
};

// What 009E3C00 leaves in the 22h-byte record at [ESP+78h]. Read-side layout:
// 009E3C00's body was not read, so the names are the consumer's reading and
// the follow-up packet `ship_ai_path_source` owns the producer.
struct ShipAiPathPointRecord {
    float query_x_00{0.0f};        // +00h in, 009EE5DF
    float query_z_04{0.0f};        // +04h in, 009EE5EB
    float point_x_08{0.0f};        // +08h out, 009EE671
    float point_z_0c{0.0f};        // +0Ch out, 009EE694
    float next_x_10{0.0f};         // +10h out, 009EE78B
    float next_z_14{0.0f};         // +14h out, 009EE79D
    std::uint32_t node_18{0};      // +18h out, 009EE5F9; zero means no point
    int direction_1c{0};           // +1Ch out, 009EE602
    bool more_path_20{false};      // +20h out, 009EE6D3 / 009EE776
    bool steer_enabled_21{false};  // +21h out, 009EE801
};

struct ShipAiPathPickResult {
    bool entered{false};        // the 009EE59B / 009EE59F gate opened
    bool point_found{false};    // node_18 was non-zero
    bool published{false};      // the 009EE66C publish ran
    float publish_low{0.0f};    // the third argument 00815F30 took
    float publish_high{0.0f};   // the fourth
    // 009EE586..009EE594 clear blk+388h, +389h and +38Ah before the gate is
    // tested, on every pass that reaches 009EE580. The caller writes them back.
    bool clears_flags_388_389_38a{true};
};

// 009EE5A5 stores this literal in blk+39Ch before the path refresh; the
// station-keeping arm computes the same slot instead.
inline constexpr float kShipAiPathPickSpeedScale = 1.25f;  // 00CF29A8
// 009EE60E: the floor of the lateral band 00815F30 is given.
inline constexpr float kShipAiPathPublishLowFloor = 30.0f;  // 00E0E304
// 009EE643 FMUL qword [00D21A88]: the node width scaled into the band's top.
inline constexpr double kShipAiPathPublishHighScale = 1.75;  // 00D21A88

struct ShipAiPathPickHost {
    virtual ~ShipAiPathPickHost() = default;
    // 009EE5C2, 009ED3E0(blk)(seconds). Body read: it hands the latched goal
    // blk+1DCh / +1E0h and the pose blk+184h to the planner 009E3780, keeps
    // the front path at blk+2F4h and the back path at blk+2F8h, and swaps them
    // once the back path's status word +1Ch passes 3.
    virtual void refresh_path_plan_009ed3e0(float seconds) = 0;
    // 009EE5F4, 009E3C00([blk+2F4h])(&record): the next path point for the
    // query position the record carries in. Body not read.
    virtual void next_path_point_009e3c00(ShipAiPathPointRecord& record) = 0;
    // 009EE61E, the float at [blk+2F4h]+8h, taken as a width the published
    // band's floor must clear. Read-side reading; producer not read.
    virtual float path_width_2f4_08() = 0;
    // 009EE66C, 00815F30(record)(xz, direction, low, high), RET 10h, where the
    // record is unit+0AECh - 54h * unit+0B40h. Contract from
    // docs/SHIP_AI_ORDER_CONSUMER.md: the two-slot lateral-offset memory.
    virtual void publish_lateral_offset_00815f30(std::uint32_t node, int direction,
                                                 float low, float high) = 0;
    // 009EE63A, the float at node+20h.
    virtual float path_node_width_20(std::uint32_t node) = 0;
};

// 009EE580..009EE670, inside BSP_ShipAi_ControlsStep's body 009ED6B0-009EF228.
ShipAiPathPickResult ship_ai_pick_path_point_009ee580(const ShipAiPathPickState& state,
                                                      float seconds,
                                                      ShipAiPathPointRecord& record,
                                                      float& blk_speed_scale_39c,
                                                      ShipAiPathPickHost& host);

// 007ADC30, `bool __thiscall(slot)(void)`, body 007ADC30-007ADC50, call site
// 009E59F8 in BSP_ShipAi_MoveOnPathStateStep. True when the slot carries no
// command, or the command's vtable[0Ch] leg count is not positive. It is the
// head of 007ADC60 BSP_EntityCommand_IsOnFinalLeg without that routine's
// cursor tests, so it answers "no legs at all", not "the last leg".
bool ship_ai_command_slot_has_no_legs_007adc30(bool slot_has_command, int leg_count);

}  // namespace bsp
