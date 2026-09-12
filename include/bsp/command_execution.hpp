#pragma once
// Projection of the unit command controller's execution engine: the ten-slot
// command queue at controller+54h, the one-slot override at +188h, and the
// begin/step/clear sequences that move commands through them.
//
// Packet cc2_director_commands. docs/COMMAND_EXECUTION.md and
// docs/COMMAND_CLASSES.md carry the addresses and the evidence per claim.
// Every descriptive name here is a hypothesis, not a recovered symbol, except
// `InternalClearPrimaryCommand`, which is the literal trace string 00720850
// passes to 004254B0.
//
// The controller object itself (250h bytes at unit+738h, constructor 008366D0)
// is include/bsp/weapon_director.hpp's; the 26 command classes are
// include/bsp/entity_orders.hpp's (EntityOrderCommandClass). Nothing here
// duplicates a type or constant from either header.
#include <cstddef>
#include <cstdint>

#include "bsp/entity_orders.hpp"
#include "bsp/weapon_director.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The command parameter record (18h bytes) and the slot that holds it
// ---------------------------------------------------------------------------
// Slot layout is fixed by 00720850's shift loop, which moves exactly these
// eight fields one slot down (piVar5[-4] = piVar5[3] .. piVar5[2] = piVar5[9]).
// The same 18h-byte shape is built on the stack by 00720CD0 at 00720D2A..
// 00720D68 and copied whole into the override record by 0071E7F0.

inline constexpr std::size_t kCommandParamsOffHasTargetEntity = 0x00; // 00720D42
inline constexpr std::size_t kCommandParamsOffHasPosition = 0x01;     // 00720850
inline constexpr std::size_t kCommandParamsOffTargetEntityId = 0x02;  // 00720D4E
inline constexpr std::size_t kCommandParamsOffTargetEntity = 0x04;    // 00720D31
inline constexpr std::size_t kCommandParamsOffPositionX = 0x08;       // 00720D08
inline constexpr std::size_t kCommandParamsOffPositionY = 0x0c;       // 00720D16
inline constexpr std::size_t kCommandParamsOffPositionZ = 0x10;       // 00720D24
inline constexpr std::size_t kCommandParamsOffTrailing = 0x14;        // 00720D68
inline constexpr std::size_t kCommandParamsSize = 0x18;

// Offsets inside a command slot. The command pointer is the slot's +0h; the
// parameter record starts at +4h, so slot stride (1Ch, weapon_director.hpp's
// kDirectorCommandSlotStride) is 4 + kCommandParamsSize.
inline constexpr std::size_t kCommandSlotOffCommand = 0x00;
inline constexpr std::size_t kCommandSlotOffParams = 0x04;

// The 18h-byte record a command carries. Field meanings come from the writers
// (00720CD0, 00720850, 0071E610, 0071E7F0), not from any reader.
struct CommandParams {
    bool has_target_entity{false};  // +0h
    bool has_position{false};       // +1h
    std::uint16_t target_entity_id{0};   // +2h, the entity's +174h
    std::uint32_t target_entity{0};      // +4h
    float position_x{0.0f};              // +8h
    float position_y{0.0f};              // +0Ch
    float position_z{0.0f};              // +10h
    std::uint32_t trailing{0};           // +14h, zero at every producer read
};

// One queue slot, override record or previous-command record. All three have
// the same shape; `command` is one of the 26 singleton addresses in
// 00E08EF8..00E08FC7 (entity_orders.hpp's kEntityOrderCommandFirstAddress), or
// 0 for an empty slot.
struct CommandSlot {
    std::uint32_t command{0};
    CommandParams params{};
};

// ---------------------------------------------------------------------------
// Controller offsets this packet established
// ---------------------------------------------------------------------------
// kDirectorOffCommandSlots (54h), kDirectorCommandSlotStride (1Ch) and
// kDirectorCommandSlotCount (10) are weapon_director.hpp's and are reused.

inline constexpr std::size_t kControllerOffCommandMode = 0x30;        // 0071BE40
inline constexpr std::size_t kControllerOffQueueAccepted = 0x44;      // 0071F600
inline constexpr std::size_t kControllerOffQueueStage = 0x48;         // 00836A81
inline constexpr std::size_t kControllerOffOverrideAccepted = 0x4c;   // 0071F600
inline constexpr std::size_t kControllerOffOverrideStage = 0x50;      // 0071E7F0
inline constexpr std::size_t kControllerOffPreviousCommand = 0x16c;   // 00720850
inline constexpr std::size_t kControllerOffOverrideCommand = 0x188;   // 0071E7F0
inline constexpr std::size_t kControllerOffOverrideParams = 0x18c;    // 0071E7F0
inline constexpr std::size_t kControllerOffPathObjects = 0x1a4;       // 0071DEE0
inline constexpr std::size_t kControllerPathObjectSize = 0x50;        // 00720B1B

// Director vtable slots this packet used, beyond weapon_director.hpp's set.
inline constexpr std::size_t kDirectorVtableSlotSlotHasActiveOrder = 0x20; // 0071DEE0
inline constexpr std::size_t kDirectorVtableSlotNormalizeSelfTarget = 0x14; // 00836040
inline constexpr std::size_t kDirectorVtableSlotOnCommandChanged = 0x6c;   // 00835BF0
inline constexpr std::size_t kDirectorVtableSlotBeginCommand = 0x78;       // 00835C70
inline constexpr std::size_t kDirectorVtableSlotStep = 0x7c;               // 00836920

// +30h. 0071BE40 returns the queue head for 1, the override for 2, 0 otherwise.
enum class CommandMode : int {
    kIdle = 0,
    kQueueHead = 1,
    kOverride = 2,
};

// +48h / +50h, raised by 0071D810 and 0071D9E0 (docs/UNIT_COMMANDED_SPEED.md,
// the 009E1170 stage ladder). Only the three values the controller writes.
inline constexpr int kCommandStageFresh = 0;    // what a clear leaves behind
inline constexpr int kCommandStageStarted = 1;  // 00835DA0, 00835DA8
inline constexpr int kCommandStageDone = 2;     // 00836A7A, 00836AD2, 00836D12

// The 26 singleton addresses this engine names by identity. They are the
// entity_orders.hpp table's `object_address` values; these aliases exist so the
// dispatch below reads the way the listing does.
inline constexpr std::uint32_t kCommandFollow = 0x00e08f60u;
inline constexpr std::uint32_t kCommandMoveTo = 0x00e08f68u;
inline constexpr std::uint32_t kCommandCruise = 0x00e08f70u;
inline constexpr std::uint32_t kCommandAttackMove = 0x00e08f78u;
inline constexpr std::uint32_t kCommandMoveOnPath = 0x00e08f80u;
inline constexpr std::uint32_t kCommandStop = 0x00e08f88u;
inline constexpr std::uint32_t kCommandLeave = 0x00e08fb0u;
inline constexpr std::uint32_t kCommandDisband = 0x00e08fb8u;

// ---------------------------------------------------------------------------
// The controller's command state
// ---------------------------------------------------------------------------
struct CommandQueueState {
    CommandMode mode{CommandMode::kIdle};              // +30h
    bool queue_accepted{false};                        // +44h
    int queue_stage{kCommandStageFresh};               // +48h
    bool override_accepted{false};                     // +4Ch
    int override_stage{kCommandStageFresh};            // +50h
    CommandSlot slots[kDirectorCommandSlotCount]{};    // +54h..+16Bh
    CommandSlot previous{};                            // +16Ch..+187h
    CommandSlot override_slot{};                       // +188h..+1A3h
    std::uint32_t path_objects[kDirectorCommandSlotCount]{}; // +1A4h..+1CBh
};

// 0071BE40 BSP_WeaponDirector_CurrentCommand, __fastcall(controller), RET 0.
std::uint32_t command_queue_current_command(const CommandQueueState& state) noexcept;

// 0071D780 BSP_WeaponDirector_CommandCount. Counts slots up to the first empty
// one, except that a moveonpath slot with a non-empty point list counts as its
// waypoint count. `path_point_counts[i]` is (end - begin) / 0Ch for slot i, or 0
// when the slot has no path object or its vtable[4] answers false.
int command_queue_count(const CommandQueueState& state,
                        const int (&path_point_counts)[kDirectorCommandSlotCount]) noexcept;

// 0071DEE0, director vtable +20h: does slot `index` hold an order worth running.
// False for an empty slot and for stop, cruise, Leave and disband; for
// moveonpath only when the slot's path object reports points.
bool command_slot_has_active_order(const CommandQueueState& state, int index,
                                   int path_point_count) noexcept;

// ---------------------------------------------------------------------------
// Completion rules, as pure functions with explicit inputs
// ---------------------------------------------------------------------------
// The generic arrival test at 00836A6C. `FLD double ptr [00D09FE8]` loads
// 4000000.0 and `FCOMIP`/`JBE` raises stage 2 only when the squared horizontal
// distance is strictly below it. The distance is built from x and z only
// (00836A40..00836A62 uses pfVar7[0] and pfVar7[2]).
inline constexpr double kCommandArrivalRadiusSquared = 4000000.0; // 00D09FE8

bool command_arrival_reached(float unit_x, float unit_z, float target_x, float target_z) noexcept;

// 00836A6C's full gate: the arrival test only applies with more than one command
// queued, the queue stage below 1, the head not category 1 or 2, and the last
// queued command category 1 or 2.
bool command_arrival_gate_open(int queued_count, int queue_stage, int head_category,
                               int last_category) noexcept;

// 00836A8E, the stop arm. `COMISS speed, dword ptr [00D7A218]` against 0.0f with
// `JC` skipping the raise: stop is done unless the speed is below zero.
inline constexpr float kStopCompletionSpeed = 0.0f; // 00D7A218

bool stop_command_complete(int queued_count, bool unit_flag_184h, float unit_speed) noexcept;

// 00836ADC, the follow arm. Stage 2 unless every clause holds.
bool follow_command_continues(bool unit_in_group, std::uint32_t group_leader,
                              std::uint32_t unit, std::uint32_t command_target,
                              int queued_count) noexcept;

// ---------------------------------------------------------------------------
// Host: one virtual method per native call site
// ---------------------------------------------------------------------------
// No default implementations: nothing here stands in for unrecovered game
// behaviour. The motion controller, the session endpoint, the observer registry,
// the entity pose cache, the path objects and the effects are contracts.
struct CommandExecutionHost {
    virtual ~CommandExecutionHost() = default;

    // 00720889: command->vtable[4](), the class name. "EmptyCommand" for a null
    // slot is the caller's literal, not this method's.
    virtual const char* command_name(std::uint32_t command) = 0;
    // 008358F0 / 0071F653: command->vtable[0Ch](), the category.
    virtual int command_category(std::uint32_t command) = 0;
    // 0071E825, 0071F686, 0071F750: 0071D6D0 BSP_WeaponDirector_CommandAcceptsTarget.
    virtual bool command_accepts_target(std::uint32_t command, const CommandParams& params) = 0;

    // 00835907, 0071E841, 00720916: 00521EA0, resolve a parameter record's target.
    virtual std::uint32_t resolve_target(const CommandParams& params) = 0;
    // 0072099B, 0071F6FA: 00414DB0, refresh a target's world pose when its +0C8h is 0.
    virtual void refresh_target_pose(std::uint32_t target) = 0;
    // 00720850's snap: target+0FCh/+100h/+104h.
    virtual void read_target_position(std::uint32_t target, float& x, float& y, float& z) = 0;

    // 0071E8EE: 00694A60, register the observer pair on a resolved target.
    virtual void register_target_observer(std::uint32_t target) = 0;
    // 0072092A, 0071E895, 0071E69C: 006952A0, unregister it.
    virtual void unregister_target_observer(std::uint32_t target) = 0;

    // 00720B36, 00720C72: 0071FB90 after operator_new(50h).
    virtual std::uint32_t create_path_object() = 0;
    // 00720AA1, 00720B85: pathObject->vtable[0](1), the scalar deleting destructor.
    virtual void destroy_path_object(std::uint32_t path_object) = 0;
    // 00835CE6, 00835CF9: 0071BDB0 then byte +18h set to 1.
    virtual void invalidate_path_object(std::uint32_t path_object) = 0;

    // 007208A3: sessionEndpoint->vtable[10h](), the value the clear trace prints.
    virtual std::uint32_t session_trace_value() = 0;
    // 007208B1: 004254B0, the varargs trace. Format is the caller's literal.
    virtual void trace_clear_primary_command(const char* command_name, int index, int mode,
                                             std::uint32_t session_value) = 0;

    // 0071F79B, 0071F8C0: 00984300 with the "start" literal, and 00836D51 with
    // "finished" (00D09FD8). `name` is that literal.
    virtual void send_command_message(const char* name, std::uint32_t command,
                                      const CommandParams& params) = 0;
    // 0071F7E8, 0071F909: 00984800 on the resolved target after the send.
    virtual void notify_command_target(std::uint32_t target) = 0;

    // 00835E12, 00836A7A, 00836AD2, 00836D12: 0071D810, the queue stage ladder.
    virtual void raise_queue_stage(int stage) = 0;
    // 00836920's FUN_0071D9E0(2): the override stage ladder.
    virtual void raise_override_stage(int stage) = 0;
    // 00835930, 00835E07: 00835860 BSP_WeaponDirector_SetFireTarget.
    virtual void set_fire_target(std::uint32_t target, bool forced) = 0;
    // 008364E0, director vtable +2Ch: the current fire target.
    virtual std::uint32_t fire_target() = 0;
    // 00720B56, and 0071E6B5 / 0071E90E: this->vtable[6Ch] = 00835BF0.
    virtual void on_command_changed(bool from_clear) = 0;

    // 00835D92: 007788B0 / 007788D0, the unit's AI group leader.
    virtual std::uint32_t group_leader() = 0;
    // 00835DC2: 00521E70, the attackmove begin gate.
    virtual bool attack_move_gate() = 0;
    // 00835DD2 / 00835DE4 (CALL EAX): BSP_Entity_ControllerBelongsToAnother.
    virtual bool controller_belongs_to_another() = 0;
    // 00835D33: 0071F600, the base begin. Returns its acceptance answer.
    virtual bool begin_command_base(bool use_queue_head) = 0;
    // 008358DF: 0071E6C0 BSP_WeaponDirector_PushCommandSlot.
    virtual bool push_command_slot(std::uint32_t command, const CommandParams& params) = 0;
    // 00720D6E: this->vtable[60h] = 008358D0.
    virtual bool set_command(std::uint32_t command, const CommandParams& params) = 0;

    // The three floats at 00F87574. Past .data's raw size, so zero at load.
    virtual void default_command_position(float& x, float& y, float& z) = 0;
    // Session mode at [00E188A8]+1FE4h.
    virtual int session_mode() = 0;
    // The owning unit at controller+24Ch and its +184h byte.
    virtual bool unit_flag_184h() = 0;
};

// ---------------------------------------------------------------------------
// The sequences
// ---------------------------------------------------------------------------

// 00720CD0 BSP_WeaponDirector_IssueTargetCommand, director vtable +58h,
// __thiscall(controller)(entity), RET 4 at 00720D76. Clears every occupied slot
// through 00720850, then issues `follow` (00E08F60) with a record naming the
// entity. Returns what vtable[60h] answered.
bool issue_target_command_00720cd0(CommandQueueState& state, std::uint32_t entity,
                                   std::uint16_t entity_id, CommandExecutionHost& host);

// 00720CA0, the clear-all loop on its own: index 9 down to 0.
void clear_all_command_slots_00720ca0(CommandQueueState& state, CommandExecutionHost& host);

// 00720850 InternalClearPrimaryCommand, __thiscall(controller)(index),
// body 00720850-00720C95. The only routine that removes a command.
void clear_command_slot_00720850(CommandQueueState& state, int index, CommandExecutionHost& host);

// 0071E610, the override clear 00720850 calls when the mode is 2.
void clear_override_command_0071e610(CommandQueueState& state, CommandExecutionHost& host);

// 0071E7F0, set the override command.
void set_override_command_0071e7f0(CommandQueueState& state, std::uint32_t command,
                                   const CommandParams& params, CommandExecutionHost& host);

// 008358D0 BSP_WeaponDirector_SetCommand, director vtable +60h,
// __thiscall(controller)(command, params), RET 8 at 0083593A.
bool set_command_008358d0(std::uint32_t command, const CommandParams& params,
                          CommandExecutionHost& host);

// 00835C70 BSP_WeaponDirector_BeginCurrentCommand, director vtable +78h,
// __thiscall(controller)(useQueueHead), RET 4. Returns the acceptance answer it
// stores at +44h or +4Ch.
bool begin_current_command_00835c70(CommandQueueState& state, bool use_queue_head,
                                    CommandExecutionHost& host);

}  // namespace bsp
