// cc13_moveto1: the kind-7 moveto task construction, its keep test and the
// attack-class pick. Packet cc13-moveto1; docs/CC13_MOVETO1.md carries the
// evidence, the host comparison and the structure-target answer.
// Names are hypotheses. Build-tested only; nothing here is wired.

#include "bsp/cc13_moveto1.hpp"

namespace bsp {
namespace cc13_moveto1 {

// ---------------------------------------------------------------------------
// 009C2DF0 BSP_BotTaskMoveTo_ConstructApproachBlock (009C2DF0-009C2ECF).
// __thiscall(ECX = approach, task+3F8h)(bot [ESP+4], target [ESP+8]), RET 8,
// returns the approach. EH frame states 0..4 only guard the members.
// Layout (approach-relative): +0 the 009C1C30 approach, +70h the state
// registry, +84h the moveto state (inline, 18h bytes), +9Ch the follow state,
// +134h the circle state. Task-relative those are +468h, +47Ch, +494h, +52Ch.
// ---------------------------------------------------------------------------
Handle moveto_approach_block_construct_009c2df0(Handle approach, Handle bot, Handle target,
                                                MoveToTaskCalls& calls)
{
    // 009C2E06-009C2E1F: 009C1C30(approach, [bot+50h] the unit, target).
    const Handle unit = calls.read_u32(bot + 0x50u);
    calls.approach_moveto_construct_009c1c30(approach, unit, target);
    // 009C2E24-009C2E2F: the registry at +70h.
    const Handle registry = approach + 0x70u;
    calls.state_registry_construct_00411e20(registry);
    // 009C2E3A / 009C2E40: the block's own vtable, then the registry's.
    calls.write_u32(approach, kVtApproachBlock);
    calls.write_u32(registry, kVtApproachRegistry);
    // 009C2E46-009C2E55: the moveto state at +84h, field stores in image order,
    // vtable last.
    const Handle moveto_state = approach + 0x84u;
    calls.write_u32(moveto_state + 0x04u, approach);
    calls.write_u32(moveto_state + 0x0Cu, 0u);
    calls.write_u32(moveto_state + 0x10u, 0u);
    calls.write_u32(moveto_state + 0x14u, 0u);
    calls.write_u32(moveto_state + 0x08u, 0u);
    calls.write_u32(moveto_state, kVtStateMoveTo);
    // 009C2E5B-009C2E73: 009C2980(+9Ch)(approach, [00CE3D08] = 100.0f).
    const Handle follow_state = approach + 0x9Cu;
    calls.state_follow_construct_009c2980(follow_state, approach, kFollowStateParam);
    // 009C2E78-009C2E84: 009C25D0(+134h)(approach).
    const Handle circle_state = approach + 0x134u;
    calls.state_circle_construct_009c25d0(circle_state, approach);
    // 009C2E89-009C2EB6: register moveto, follow, circle in that order.
    calls.state_registry_add_00411e70(registry, kStrMoveToMoveTo, moveto_state);
    calls.state_registry_add_00411e70(registry, kStrFollowMoveTo, follow_state);
    calls.state_registry_add_00411e70(registry, kStrCircleMoveTo, circle_state);
    return approach; // 009C2EC0
}

// ---------------------------------------------------------------------------
// 009C3000 BSP_BotTaskMoveTo_Construct (009C3000-009C30F1).
// __thiscall(ECX = task)(bot [ESP+4], target [ESP+8]), RET 8, returns task.
// No range is read or stored here: approach+6Ch (task+464h) is first written
// by the activation 009C3650 (009C36B7, 0.0) and then by the approach update
// 009C35CB while the squadron's command is moveto.
// ---------------------------------------------------------------------------
Handle moveto_task_construct_009c3000(Handle task, Handle bot, Handle target,
                                      MoveToTaskCalls& calls)
{
    // 009C301E-009C3027: the base, kind 7.
    calls.bot_task_construct_base_0099c6f0(task, bot, kMoveToTaskKind);
    // 009C302C-009C3042: the approach block at +3F8h.
    const Handle approach = task + 0x3F8u;
    moveto_approach_block_construct_009c2df0(approach, bot, target, calls);
    // 009C305F-009C306E: the three vtables, then 009C307C +548h = [00D7A24C].
    calls.write_u32(task, kVtMoveToTask);
    calls.write_u32(approach, kVtMoveToApproach);
    calls.write_u32(task + 0x468u, kVtMoveToRegistry468);
    calls.write_f32(task + 0x548u, kStateRulePeriod);
    // 009C3047-009C308B: +54Ch = -U(0, 1) on stream 1 (FLDZ lo, FLD1 hi, FCHS).
    const float stagger = calls.random_uniform_00bd2f10(kRandomStream, 0.0f, 1.0f);
    calls.write_f32(task + 0x54Cu, -stagger);
    // 009C3091-009C30BF: the start state. A flight leader starts in circle when
    // the approach already counts as arrived (+454h = approach+5Ch), else in
    // moveto; any other plane starts in follow.
    Handle state;
    if (calls.unit_is_flight_leader_007b8ad0(calls.read_u32(task + 0x3FCu))) {
        state = calls.read_u8(task + 0x454u) != 0 ? task + 0x52Cu : task + 0x47Cu;
    } else {
        state = task + 0x494u;
    }
    calls.write_u32(task + 0x310u, state);       // 009C30BF
    calls.state_vcall_enter_04(state);           // 009C30C5-009C30CA
    calls.approach_bind_to_task_009f9980(approach, task); // 009C30CC-009C30CF
    calls.free_00bf65ac(target);                 // 009C30D4-009C30D5, no destructor call
    return task;                                 // 009C30E2
}

// ---------------------------------------------------------------------------
// 009C3B10 BSP_BotTaskMoveToForAttackMove_Construct (009C3B10-009C3B79).
// __thiscall(ECX = task)(bot [ESP+4], target [ESP+8]), RET 8, returns task.
// The variant differs from 00D20B68 only in the destructor and the class
// getter +3Ch (009C3B80, which answers attackmove 00E08F78); its keep test
// +40h is the same 009C31B0, which answers 0 while the command is attackmove.
// ---------------------------------------------------------------------------
Handle moveto_task_attackmove_construct_009c3b10(Handle task, Handle bot, Handle target,
                                                 MoveToTaskCalls& calls)
{
    moveto_task_construct_009c3000(task, bot, target, calls); // 009C3B39
    const Handle approach = task + 0x3F8u;
    calls.write_u32(task, kVtAttackMoveTask);                  // 009C3B4D
    calls.write_u32(approach, kVtAttackMoveApproach);          // 009C3B53
    calls.write_u32(task + 0x468u, kVtAttackMoveRegistry468);  // 009C3B59
    calls.approach_bind_to_task_009f9980(approach, task);      // 009C3B63, a second bind
    return task;
}

// ---------------------------------------------------------------------------
// 009BE310 BSP_MoveToTarget_CreateFromCommand (009BE310-009BE3DF).
// ECX = the command descriptor {byte kind, byte has_point, ..., +8 point};
// plain RET. An entity descriptor (kind != 0) builds the 14h entity target
// from the resolved object; otherwise a 10h point target holding the
// descriptor's point, or the zero vector 00F87574 when it has none.
// ---------------------------------------------------------------------------
Handle moveto_target_create_from_command_009be310(Handle descriptor, MoveToTaskCalls& calls)
{
    if (calls.read_u8(descriptor) != 0) {                          // 009BE329-009BE32D
        const Handle self = calls.operator_new_00bf681b(kEntityTargetSize); // 009BE330
        if (self == 0u) return 0u;                                  // 009BE34A -> 009BE36C
        const Handle entity = calls.command_target_resolve_00521ea0(descriptor); // 009BE34E
        return calls.moveto_target_construct_from_entity_009be0f0(self, entity); // 009BE356
    }
    const Handle self = calls.operator_new_00bf681b(kPointTargetSize); // 009BE37F
    if (self == 0u) return 0u;                                      // 009BE38B -> 009BE3CE
    // 009BE38D-009BE396: the source point.
    const Handle source = calls.read_u8(descriptor + 1u) != 0 ? descriptor + 8u : kZeroVector;
    // 009BE39B-009BE3B9: the three loads, then the vtable, then the stores.
    const float x = calls.read_f32(source);
    const float y = calls.read_f32(source + 4u);
    const float z = calls.read_f32(source + 8u);
    calls.write_u32(self, kVtMoveToPointTarget);
    calls.write_f32(self + 4u, x);
    calls.write_f32(self + 8u, y);
    calls.write_f32(self + 0x0Cu, z);
    return self;
}

// ---------------------------------------------------------------------------
// 009C3BE0 BSP_BotTask_MakeMoveTo (009C3BE0-009C3C3B) and 009C3C40
// BSP_BotTask_MakeMoveToForAttackMove (009C3C40-009C3C9B). ECX = bot,
// EDX = descriptor, plain RET. The target is built only after the 550h
// allocation succeeds; a failed allocation returns 0 with no target built.
// ---------------------------------------------------------------------------
Handle make_moveto_009c3be0(Handle bot, Handle descriptor, MoveToTaskCalls& calls)
{
    const Handle task = calls.operator_new_00bf681b(kMoveToTaskSize); // 009C3BF9-009C3C02
    if (task == 0u) return 0u;                                         // 009C3C18
    const Handle target = moveto_target_create_from_command_009be310(descriptor, calls); // 009C3C1C
    return moveto_task_construct_009c3000(task, bot, target, calls);  // 009C3C25
}

Handle make_moveto_for_attackmove_009c3c40(Handle bot, Handle descriptor,
                                           MoveToTaskCalls& calls)
{
    const Handle task = calls.operator_new_00bf681b(kMoveToTaskSize); // 009C3C59-009C3C62
    if (task == 0u) return 0u;                                         // 009C3C78
    const Handle target = moveto_target_create_from_command_009be310(descriptor, calls); // 009C3C7C
    return moveto_task_attackmove_construct_009c3b10(task, bot, target, calls); // 009C3C85
}

// ---------------------------------------------------------------------------
// 009C33A0, task vtable +0Ch (bytes 8B 44 24 04 D9 81 40 04 00 00 ...).
// out = {task+440h, +444h, +448h} (the approach point +48h..+50h), valid = 1.
// The valid byte is written between the x and y copies, as the image does.
// ---------------------------------------------------------------------------
void moveto_task_point_009c33a0(Handle task, MoveToTaskPoint* out, ImageMemory& mem)
{
    out->x = mem.read_f32(task + 0x440u);
    out->valid = 1;
    out->y = mem.read_f32(task + 0x444u);
    out->z = mem.read_f32(task + 0x448u);
}

// ---------------------------------------------------------------------------
// 009C31B0 BSP_BotTaskMoveTo_IsCommandStillValid (009C31B0-009C3293).
// __thiscall(ECX = task), plain RET, EAX = 0, 1 or 2.
// 0099A4C0 keeps a lone head task only when this answers exactly 1; the
// arrival hook 009C3100 ends the moveto command when it answers nonzero.
// ---------------------------------------------------------------------------
int moveto_task_command_still_current_009c31b0(Handle task, MoveToTaskCalls& calls)
{
    // 009C31B6-009C31BE: [task+404h] = approach+0Ch, the squadron (unit+9D4h).
    const Handle squadron = calls.read_u32(task + 0x404u);
    if (squadron == 0u) return 2;                                    // 009C328A
    const Handle block = calls.squadron_command_block_vcall_114(squadron); // 009C31CD
    if (block == 0u) return 0;                                       // 009C31D3
    if (calls.director_current_command_0071be40(block) != kCmdMoveTo) return 0; // 009C31E0
    const Handle entity =
        calls.command_target_resolve_00521ea0(calls.director_active_descriptor_0071eb60(block));
    if (entity != 0u) {
        // 009C31FD-009C320C: the approach's target (+43Ch = approach+44h).
        const Handle mine = calls.read_u32(task + 0x43Cu);
        if (mine == 0u) return 0;
        return mine == entity ? 1 : 0;
    }
    // 009C3216-009C3231: the task's point through vtable +0Ch; the image tests
    // the out record's valid byte (NEG/SBB/TEST against 00F872D4).
    MoveToTaskPoint mine;
    calls.task_vcall_point_0c(task, &mine);
    if (mine.valid == 0) return 0;
    // 009C3233-009C3241: the descriptor's point (a fresh 0071EB60 read).
    float there[3];
    calls.descriptor_resolve_position_006f7dd0(calls.director_active_descriptor_0071eb60(block),
                                               there);
    // 009C3246-009C3272: dx, dz stored to float, then dz*dz + dx*dx to float.
    const float dx = there[0] - mine.x;
    const float dz = there[2] - mine.z;
    const float dz2 = dz * dz;
    const float dx2 = dx * dx;
    const float d2 = dz2 + dx2;
    // 009C3276-009C3280: FLD [00CE3D64]; FCOMIP; JA keep. NaN takes the 0 arm.
    return kPointKeepRadiusSq > d2 ? 1 : 0;
}

// ---------------------------------------------------------------------------
// 007EEC00 BSP_Unit_AttackCommandAppliesWithLoadout (007EEC00-007EEC44).
// __thiscall(ECX = unit)(class, target, surface, out), RET 10h, AL.
// ---------------------------------------------------------------------------
bool unit_attack_command_applies_with_flag_007eec00(Handle unit, Handle command_class,
                                                    Handle target, std::uint8_t surface,
                                                    Handle* out, AttackChoiceCalls& calls)
{
    // 007EEC00-007EEC19: (unit+369h && byte 00E17BF2) ? 1 : 0.
    std::uint32_t flag = 0u;
    if (calls.read_u8(unit + 0x369u) != 0 && calls.read_u8(kCarrierArmByte) != 0) flag = 1u;
    // 007EEC1B-007EEC2C: 007EE8F0(class, target, surface, flag).
    if (!calls.unit_attack_command_applies_007ee8f0(unit, command_class, target, surface, flag)) {
        return false;                                                // 007EEC41
    }
    *out = command_class;                                            // 007EEC39
    return true;
}

// ---------------------------------------------------------------------------
// 007EEC50 BSP_Unit_ChooseAttackCommand (007EEC50-007EEDBD).
// __thiscall(ECX = unit)(target, prefer_ordnance, allow_guns), RET 0Ch.
// The image reuses the target slot as the ordnance result and the
// prefer_ordnance slot as the guns result; both are cleared before use.
// ---------------------------------------------------------------------------
Handle unit_choose_attack_command_007eec50(Handle unit, Handle target,
                                           std::uint8_t prefer_ordnance,
                                           std::uint8_t allow_guns, AttackChoiceCalls& calls)
{
    // 007EEC54-007EEC64: no plane in slot 0 (+3D0h) answers moveto.
    if (calls.read_u32(unit + 0x3D0u) == 0u) return kCmdMoveTo;
    if (target == 0u) return 0u;                                     // 007EEC6E
    // 007EEC70-007EEC92: the side gate. A target of another side that is not
    // side 2 goes on. Otherwise only a CommandBuilding (1Ch) of another side
    // goes on.
    const std::uint32_t target_side = calls.read_u32(target + 0x54u);
    if (calls.read_u32(unit + 0x54u) == target_side || target_side == 2u) {
        if (!calls.entity_vcall_is_kind_of_5c(target, 0x1C)) return 0u; // 007EEC8A
        if (calls.read_u32(target + 0x54u) == calls.read_u32(unit + 0x54u)) return 0u; // 007EEC92
    }
    // 007EEC9C-007EECB2: surface first, then airborne.
    const std::uint8_t surface = calls.entity_is_surface_target_00922dc0(target) ? 1 : 0;
    Handle ordnance = 0u;                                            // 007EECAA
    if (!calls.entity_is_airborne_target_00922b10(target)) {
        // 007EECC6-007EED62: the seven ordnance classes, first that applies.
        static constexpr Handle kOrder[] = {kCmdLevelBomb, kCmdDropKamikaze, kCmdDiveBomb,
                                            kCmdTorpedo,   kCmdRocket,       kCmdKamikaze,
                                            kCmdDepthCharge};
        for (const Handle candidate : kOrder) {
            if (unit_attack_command_applies_with_flag_007eec00(unit, candidate, target, surface,
                                                               &ordnance, calls)) {
                break;
            }
        }
    }
    // 007EED6B-007EED74: an ordnance class wins when ordnance is preferred.
    if (prefer_ordnance != 0 && ordnance != 0u) return ordnance;
    // 007EED76-007EEDA3: dogfight, then strafe.
    Handle guns = 0u;                                                // 007EED84
    if (!unit_attack_command_applies_with_flag_007eec00(unit, kCmdDogfight, target, surface,
                                                        &guns, calls)) {
        unit_attack_command_applies_with_flag_007eec00(unit, kCmdStrafe, target, surface, &guns,
                                                       calls);
    }
    // 007EEDA8-007EEDB7: a guns class only when guns are allowed.
    if (allow_guns != 0 && guns != 0u) return guns;
    return 0u;
}

} // namespace cc13_moveto1
} // namespace bsp
