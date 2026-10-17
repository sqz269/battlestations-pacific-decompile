#pragma once

// cc13_moveto1: the kind-7 pilot moveto task's construction and its keep test,
// the two factories 0099A170 calls for it, and the attack-class pick 007EEC50
// that decides whether an attack order on a structure becomes a moveto at all.
// Packet cc13-moveto1 (cc9 grant 2026-10-17 for 009C3000, the 0099A170 moveto
// arm and 007EEC50). Evidence and the adoption notes are in docs/CC13_MOVETO1.md.
//
// Every name is a hypothesis, not a recovered symbol. These are new C++
// interfaces, not ABI-compatible replacements; build-tested only, nothing is
// wired into the game. Every engine call the image makes is a named host hook
// invoked at the same point, named after the callee's address, and every load
// and store the image makes on a native object goes through the ImageMemory
// accessors at the image's offset and in the image's order. `Handle` is a
// native pointer value; 0 is null.
//
// 0099A170 itself is not re-implemented here: its whole caller flow, the moveto
// arms included, is already install_native_pilot_bot_command_task_0099a170
// (src/attack_commands.cpp), re-verified by this packet.
//
// Precision: the only x87 arithmetic here is 009C31B0's planar distance, three
// float subtractions/products and a sum stored to float. It runs in the bot
// think on the main thread (24-bit), so float arithmetic rounds each operation
// as the image does.

#include <cstdint>

namespace bsp {
namespace cc13_moveto1 {

using Handle = std::uint32_t;

// Command-class singletons (include/bsp/attack_commands.hpp, docs/ATTACK_COMMANDS.md).
inline constexpr Handle kCmdTorpedo = 0x00E08F18u;
inline constexpr Handle kCmdDiveBomb = 0x00E08F20u;
inline constexpr Handle kCmdLevelBomb = 0x00E08F28u;
inline constexpr Handle kCmdDropKamikaze = 0x00E08F30u;
inline constexpr Handle kCmdDepthCharge = 0x00E08F38u;
inline constexpr Handle kCmdStrafe = 0x00E08F40u;
inline constexpr Handle kCmdRocket = 0x00E08F48u;
inline constexpr Handle kCmdKamikaze = 0x00E08F50u;
inline constexpr Handle kCmdDogfight = 0x00E08F58u;
inline constexpr Handle kCmdMoveTo = 0x00E08F68u;
inline constexpr Handle kCmdAttackMove = 0x00E08F78u;

// Vtables the constructors store.
inline constexpr Handle kVtMoveToTask = 0x00D20B68u;            // 009C305F
inline constexpr Handle kVtMoveToApproach = 0x00D20B58u;        // 009C3065
inline constexpr Handle kVtMoveToRegistry468 = 0x00D20B54u;     // 009C306E
inline constexpr Handle kVtAttackMoveTask = 0x00D20BE0u;        // 009C3B4D
inline constexpr Handle kVtAttackMoveApproach = 0x00D20BD4u;    // 009C3B53
inline constexpr Handle kVtAttackMoveRegistry468 = 0x00D20BD0u; // 009C3B59
inline constexpr Handle kVtApproachBlock = 0x00D20B48u;         // 009C2E3A
inline constexpr Handle kVtApproachRegistry = 0x00D20B44u;      // 009C2E40
inline constexpr Handle kVtStateMoveTo = 0x00D20A80u;           // 009C2E55
inline constexpr Handle kVtMoveToPointTarget = 0x00D209C8u;     // 009BE3A9

// Static data.
inline constexpr Handle kStrCircleMoveTo = 0x00D20978u;  // "circle (moveto)"
inline constexpr Handle kStrFollowMoveTo = 0x00D20988u;  // "follow (moveto)"
inline constexpr Handle kStrMoveToMoveTo = 0x00D20998u;  // "moveto (moveto)"
inline constexpr Handle kZeroVector = 0x00F87574u;        // 009BE396
inline constexpr Handle kCarrierArmByte = 0x00E17BF2u;    // 007EEC09, 00 in the image
inline constexpr float kFollowStateParam = 100.0f;        // 00CE3D08 = 00 00 C8 42
inline constexpr float kStateRulePeriod = 1.0f;           // 00D7A24C = 00 00 80 3F
inline constexpr float kPointKeepRadiusSq = 10000.0f;     // 00CE3D64 = 00 40 1C 46

inline constexpr std::uint32_t kMoveToTaskSize = 0x550u;   // 009C3BF9 / 009C3C59
inline constexpr std::uint32_t kEntityTargetSize = 0x14u;  // 009BE330
inline constexpr std::uint32_t kPointTargetSize = 0x10u;   // 009BE37F
inline constexpr int kMoveToTaskKind = 7;                  // 009C301E
inline constexpr int kRandomStream = 1;                    // 009C3058 MOV ECX,1

// Native memory, read and written at the image's addresses.
struct ImageMemory {
    virtual ~ImageMemory() = default;
    virtual std::uint8_t read_u8(Handle address) = 0;
    virtual std::uint32_t read_u32(Handle address) = 0;
    virtual float read_f32(Handle address) = 0;
    virtual void write_u8(Handle address, std::uint8_t value) = 0;
    virtual void write_u32(Handle address, std::uint32_t value) = 0;
    virtual void write_f32(Handle address, float value) = 0;
};

// The out record of task vtable +0Ch (009C33A0 for both kind-7 vtables).
struct MoveToTaskPoint {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    std::uint8_t valid{0};  // +0Ch
};

struct MoveToTaskCalls : ImageMemory {
    // 00BF681B operator new; 00BF65AC _free (bound CRT bodies).
    virtual Handle operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void free_00bf65ac(Handle block) = 0;
    // 0099C6F0 BSP_BotTask_ConstructBase, __thiscall(task)(bot, kind), RET 8.
    virtual void bot_task_construct_base_0099c6f0(Handle task, Handle bot, int kind) = 0;
    // 009C1C30 BSP_BotApproachMoveTo_Construct, __thiscall(approach)(unit, target), RET 8.
    virtual void approach_moveto_construct_009c1c30(Handle approach, Handle unit,
                                                    Handle target) = 0;
    // 00411E20 BSP_BotStateRegistry_Construct, __thiscall(registry).
    virtual void state_registry_construct_00411e20(Handle registry) = 0;
    // 00411E70 BSP_BotStateRegistry_Add, __thiscall(registry)(name, state), RET 8.
    virtual void state_registry_add_00411e70(Handle registry, Handle name, Handle state) = 0;
    // 009C2980 BSP_BotStateFollow_Construct, __thiscall(state)(approach, float).
    virtual void state_follow_construct_009c2980(Handle state, Handle approach,
                                                 float param) = 0;
    // 009C25D0 BSP_BotStateMoveToCircle_Construct, __thiscall(state)(approach).
    virtual void state_circle_construct_009c25d0(Handle state, Handle approach) = 0;
    // 00BD2F10 BSP_Random_UniformFloatRange, ECX = stream, (lo, hi), result in ST0.
    virtual float random_uniform_00bd2f10(int stream, float lo, float hi) = 0;
    // 007B8AD0 BSP_Unit_IsSquadronFlightLeader, ECX = unit, AL.
    virtual bool unit_is_flight_leader_007b8ad0(Handle unit) = 0;
    // CALL [[state]+4], the state's enter (009C30C5-009C30CA).
    virtual void state_vcall_enter_04(Handle state) = 0;
    // 009F9980 BSP_BotApproach_BindToTask, __thiscall(approach)(task), RET 4.
    virtual void approach_bind_to_task_009f9980(Handle approach, Handle task) = 0;
    // 00521EA0 BSP_CommandTarget_ResolveObject, ECX = descriptor.
    virtual Handle command_target_resolve_00521ea0(Handle descriptor) = 0;
    // 009BE0F0 BSP_MoveToTarget_ConstructFromEntity, __thiscall(self)(entity), returns self.
    virtual Handle moveto_target_construct_from_entity_009be0f0(Handle self,
                                                                Handle entity) = 0;
    // CALL [[squadron]+114h], the squadron's command block (007ECFD0 for squadrons).
    virtual Handle squadron_command_block_vcall_114(Handle squadron) = 0;
    // 0071BE40 BSP_WeaponDirector_CurrentCommand, ECX = block.
    virtual Handle director_current_command_0071be40(Handle block) = 0;
    // 0071EB60 BSP_EntityCommand_ActiveTargetDescriptor, ECX = block.
    virtual Handle director_active_descriptor_0071eb60(Handle block) = 0;
    // CALL [[task]+0Ch](out), RET 4 (009C33A0 for the kind-7 vtables).
    virtual void task_vcall_point_0c(Handle task, MoveToTaskPoint* out) = 0;
    // 006F7DD0 BSP_TargetDescriptor_ResolvePosition, ECX = descriptor, (out vec3), RET 4.
    virtual void descriptor_resolve_position_006f7dd0(Handle descriptor, float out[3]) = 0;
};

// 009C2DF0 BSP_BotTaskMoveTo_ConstructApproachBlock, __thiscall(approach = task+3F8h)
// (bot, target), RET 8. Returns approach.
Handle moveto_approach_block_construct_009c2df0(Handle approach, Handle bot, Handle target,
                                                MoveToTaskCalls& calls);

// 009C3000 BSP_BotTaskMoveTo_Construct, __thiscall(task)(bot, target), RET 8.
// Frees `target` (the 009BE310 object) after the approach has copied it.
Handle moveto_task_construct_009c3000(Handle task, Handle bot, Handle target,
                                      MoveToTaskCalls& calls);

// 009C3B10 BSP_BotTaskMoveToForAttackMove_Construct, __thiscall(task)(bot, target), RET 8.
Handle moveto_task_attackmove_construct_009c3b10(Handle task, Handle bot, Handle target,
                                                 MoveToTaskCalls& calls);

// 009BE310 BSP_MoveToTarget_CreateFromCommand, ECX = descriptor, plain RET.
Handle moveto_target_create_from_command_009be310(Handle descriptor, MoveToTaskCalls& calls);

// 009C3BE0 BSP_BotTask_MakeMoveTo, ECX = bot, EDX = descriptor, plain RET.
Handle make_moveto_009c3be0(Handle bot, Handle descriptor, MoveToTaskCalls& calls);

// 009C3C40 BSP_BotTask_MakeMoveToForAttackMove, ECX = bot, EDX = descriptor, plain RET.
Handle make_moveto_for_attackmove_009c3c40(Handle bot, Handle descriptor,
                                           MoveToTaskCalls& calls);

// 009C33A0, task vtable +0Ch of 00D20B68 and 00D20BE0 (no Ghidra function),
// __thiscall(task)(out), RET 4.
void moveto_task_point_009c33a0(Handle task, MoveToTaskPoint* out, ImageMemory& mem);

// 009C31B0 BSP_BotTaskMoveTo_IsCommandStillValid, task vtable +40h of both kind-7
// vtables, __thiscall(task), plain RET. 2 = no squadron, 1 = keep, 0 = over.
int moveto_task_command_still_current_009c31b0(Handle task, MoveToTaskCalls& calls);

struct AttackChoiceCalls : ImageMemory {
    // CALL [[entity]+5Ch](kind), RET 4, AL.
    virtual bool entity_vcall_is_kind_of_5c(Handle entity, int kind) = 0;
    // 00922DC0 BSP_Entity_IsSurfaceTarget (00922C80 with DL = 1), ECX = entity.
    virtual bool entity_is_surface_target_00922dc0(Handle entity) = 0;
    // 00922B10 BSP_Entity_IsAirborneTarget, ECX = entity.
    virtual bool entity_is_airborne_target_00922b10(Handle entity) = 0;
    // 007EE8F0 BSP_Unit_AttackCommandApplies, __thiscall(unit)(class, target,
    // surface, flag), RET 10h, AL. `surface` is the low byte of a dword whose
    // upper bytes are stale (007EEC50 passes EBP); the image tests it as a char.
    virtual bool unit_attack_command_applies_007ee8f0(Handle unit, Handle command_class,
                                                      Handle target, std::uint8_t surface,
                                                      std::uint32_t flag) = 0;
};

// 007EEC00 BSP_Unit_AttackCommandAppliesWithLoadout, __thiscall(unit)(class, target,
// surface, out), RET 10h. Writes *out = class only when it applies.
bool unit_attack_command_applies_with_flag_007eec00(Handle unit, Handle command_class,
                                                    Handle target, std::uint8_t surface,
                                                    Handle* out, AttackChoiceCalls& calls);

// 007EEC50 BSP_Unit_ChooseAttackCommand, __thiscall(unit)(target, prefer_ordnance,
// allow_guns), RET 0Ch. Returns a command class or 0.
Handle unit_choose_attack_command_007eec50(Handle unit, Handle target,
                                           std::uint8_t prefer_ordnance,
                                           std::uint8_t allow_guns, AttackChoiceCalls& calls);

} // namespace cc13_moveto1
} // namespace bsp
