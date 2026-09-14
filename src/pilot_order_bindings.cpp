#include "bsp/pilot_order_bindings.hpp"

// Reconstruction of the five mission-Lua pilot order bindings.
// docs/PILOT_ORDER_BINDINGS.md carries the evidence and the coverage table.
// The binding names are recovered from the registration table at 00E0BC90;
// every other name here is a hypothesis, not a recovered symbol.

namespace bsp {
namespace {

// The entity liveness quartet the bindings inline: +5Ch set, +5Dh, +60h and
// +5Eh clear. The identical test already lives in include/bsp/airfield_taxi.hpp
// as entity_is_usable_006d2730; repeated here only to keep the branch order of
// 008A4DAA-008A4DD2 visible, which tests +5Dh before +60h before +5Eh.
bool liveness_passes(const EntityLivenessBytes& bytes) {
    if (!bytes.present) return false;        // 008A4DB0 CMP byte [EDI+5Ch],0 / JZ
    if (bytes.out_of_action) return false;   // 008A4DBA CMP byte [EDI+5Dh],0 / JNZ
    if (bytes.flag_60) return false;         // 008A4DC4 CMP byte [EDI+60h],0 / JNZ
    if (bytes.flag_5e) return false;         // 008A4DCE CMP byte [EDI+5Eh],0 / JNZ
    return true;
}

// 008A472F-008A475A, and the single guarded copy at 008A4417-008A4424. The
// native unrolls the guard three times in PilotMoveToRange; 00414DB0 sets
// entity+C8h itself, so only the first body can run.
void ensure_world_pose(PilotOrderHost& host, void* entity) {
    if (!host.world_pose_valid(entity)) host.refresh_world_pose(entity);
}

}  // namespace

PilotAttackSelectorFlags pilot_attack_selector_flags_008a4e54(int attack_type) {
    PilotAttackSelectorFlags out;
    // 008A4E5C CMP EBP,2 / 008A4E5F SETNZ AL, pushed as 007EEC50's arg 2.
    out.prefer_ordnance = attack_type != kPilotAttackTypeGunOnly;
    // 008A4E54 CMP EBP,3 / 008A4E57 SETNZ BL, pushed as 007EEC50's arg 3.
    out.allow_guns = attack_type != kPilotAttackTypeBombOrTorpedo;
    return out;
}

int pilot_set_target_attack_type_008a4e0b(int argc, int argument_as_int) {
    // 008A4E14 CMP EAX,3 / 008A4E17 JNZ 008A4E4B: an exact match, not `>= 3`.
    if (argc != 3) return kPilotAttackTypeDefault;  // 008A4D59 MOV EBP,1
    return argument_as_int;                          // 008A4E33 CALL 00B66290
}

float pilot_move_to_range_008a46dc(int argc, float argument_as_float) {
    // 008A46E5 CMP EAX,3 / 008A46E8 JNZ 008A471C.
    if (argc != 3) return 0.0f;      // 0088A8C7 cleared the descriptor's +14h
    return argument_as_float;        // 008A4703 CALL 00B66270, 008A4708 FSTP
}

void pilot_retreat_position_008a443e(const PilotRetreatZoneCorners& zone, float out[3]) {
    // 008A4456-008A4488 seed the two accumulators from corner 0 through an
    // FLDZ add; 008A448C-008A4504 fold corners 1, 2 and 3 in.
    float sum_x = 0.0f;
    float sum_z = 0.0f;
    for (int i = 0; i < 4; ++i) {
        sum_x += zone.corner[i][0];  // FLD [EAX+10h/1Ch/28h/34h]
        sum_z += zone.corner[i][2];  // FLD [EAX+18h/24h/30h/3Ch]
    }
    // 008A450C FLD double [00D7A348] = 0.25, 008A4512 FMUL ST1, 008A451A FMUL.
    // The native multiplies in double and stores through FSTP float.
    out[0] = static_cast<float>(static_cast<double>(sum_x) * kPilotRetreatCornerWeight);
    out[1] = 0.0f;  // 008A4445 XORPS XMM0,XMM0 -> 008A4465 MOVSS [desc+0Ch]
    out[2] = static_cast<float>(static_cast<double>(sum_z) * kPilotRetreatCornerWeight);
}

PilotSetTargetOutcome pilot_set_target_gate_008a4da8(bool unit_live,
                                                     bool target_live,
                                                     bool first_selection,
                                                     bool second_selection) {
    if (!unit_live) return PilotSetTargetOutcome::kUnitNotLive;      // -> 008A4EB1
    if (!target_live) return PilotSetTargetOutcome::kTargetNotLive;  // -> 008A4EB1
    // 008A4E8C TEST EAX,EAX / 008A4E8E JNZ 008A4EA2.
    if (first_selection) return PilotSetTargetOutcome::kIssued;
    // 008A4E9E TEST EAX,EAX / 008A4EA0 JZ 008A4EB1.
    if (!second_selection) return PilotSetTargetOutcome::kNoCommandClass;
    return PilotSetTargetOutcome::kIssued;
}

PilotSetTargetOutcome pilot_set_target_008a4c90(PilotOrderHost& host) {
    // 008A4D6F-008A4D90: argument 0 through 00888AA0, the table's `Ptr`.
    void* unit = host.unit_from_argument(kPilotOrderUnitArgument);
    // 008A4DA8 CMP EDI,EBX, then the quartet. The unit gate runs before the
    // target argument is even read.
    if (unit == nullptr || !liveness_passes(host.liveness(unit)))
        return PilotSetTargetOutcome::kUnitNotLive;

    // 008A4DD8-008A4DF5: argument 1 through 0088A810.
    SceneCommandTarget target = host.command_target_from_argument(kPilotOrderTargetArgument);

    // 008A4E0B-008A4E38.
    const int attack_type = pilot_set_target_attack_type_008a4e0b(
        host.argument_count(), host.argument_as_int(kPilotOrderOptionArgument));
    const PilotAttackSelectorFlags flags = pilot_attack_selector_flags_008a4e54(attack_type);

    // 008A4E4B CALL 00521EA0 on the descriptor; the result is cached in +4h.
    void* object = host.resolve_target_object(target);

    // 008A4E6B, unconditional: the selector runs before the target is checked.
    void* command = host.choose_attack_command(unit, object, flags.prefer_ordnance,
                                               flags.allow_guns);

    // 008A4E70 TEST ESI,ESI then the same quartet on the resolved target.
    if (object == nullptr || !liveness_passes(host.liveness(object)))
        return PilotSetTargetOutcome::kTargetNotLive;

    if (command == nullptr) {
        // 008A4E90-008A4E99: the identical call again, same three arguments.
        // Faithful to the listing; see the doc's "the repeated 007EEC50 call".
        command = host.choose_attack_command(unit, object, flags.prefer_ordnance,
                                             flags.allow_guns);
        if (command == nullptr) return PilotSetTargetOutcome::kNoCommandClass;
    }

    // 008A4EA2-008A4EAC. The class is whatever 007EEC50 answered; unlike the
    // other four bindings this one never names a class literal.
    host.issue_command(unit,
                       static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(command)),
                       target, kPilotOrderIssueFlags);
    return PilotSetTargetOutcome::kIssued;
}

void pilot_move_to_008a4150(PilotOrderHost& host) {
    // 008A422D-008A4261, then 008A4266-008A4284. Straight line: no argument
    // count test, no liveness gate, no null check on the unit.
    void* unit = host.unit_from_argument(kPilotOrderUnitArgument);
    SceneCommandTarget target = host.command_target_from_argument(kPilotOrderTargetArgument);
    // 008A4299-008A42A7 PUSH 1 / PUSH desc / PUSH 00E08F68 / MOV ECX,ESI.
    host.issue_command(unit, kPilotOrderClassMoveTo, target, kPilotOrderIssueFlags);
}

void pilot_move_to_range_008a4590(PilotOrderHost& host) {
    // 008A466D-008A46A4 and 008A46A9-008A46C7, the same pair as PilotMoveTo.
    void* unit = host.unit_from_argument(kPilotOrderUnitArgument);
    SceneCommandTarget target = host.command_target_from_argument(kPilotOrderTargetArgument);

    // 008A4708 FSTP float [ESP+54h]: the descriptor's +14h, not a fourth
    // argument to 0077D600. include/bsp/scene_deferred_refs.hpp calls the
    // field `reserved` and said "always 0"; this binding writes it, so it
    // is now named `trailing` after the EntityOrderMessage field 0077D600
    // forwards it into.
    target.trailing = pilot_move_to_range_008a46dc(
        host.argument_count(), host.argument_as_float(kPilotOrderOptionArgument));

    // 008A471C-008A472A. Same class literal as PilotMoveTo.
    host.issue_command(unit, kPilotOrderClassMoveTo, target, kPilotOrderIssueFlags);

    // 008A472F-008A475A, after the issue, not before.
    ensure_world_pose(host, unit);
}

void pilot_retreat_008a4300(PilotOrderHost& host) {
    // 008A43DE-008A4412. The only argument.
    void* unit = host.unit_from_argument(kPilotOrderUnitArgument);

    // 008A441D MOV EDI,[ESI+54h] is issued before the guarded refresh.
    const int side = host.entity_side(unit);
    ensure_world_pose(host, unit);  // 008A4417-008A4424

    // 008A4429-008A4439: ECX = [00E188A8], (&unit+FCh, side, 0, 0).
    const PilotRetreatZoneCorners zone =
        host.retreat_zone(host.entity_world_position(unit), side);

    // 008A4451-008A447E: the descriptor is built by hand, not by 0088A810.
    SceneCommandTarget target;
    target.kind = 0;            // 008A4471 MOV byte [desc+0h],BL
    target.position_valid = 1;  // 008A445E MOV byte [desc+1h],1
    target.object_id = 0;       // 008A4475 MOV word [desc+2h],BX
    target.object = nullptr;    // 008A446D MOV dword [desc+4h],EBX
    target.trailing = 0.0f;     // 008A447E MOVSS [desc+14h],XMM0
    pilot_retreat_position_008a443e(zone, target.position);

    // 008A444B/008A44EA/008A44EF then 008A4532.
    host.issue_command(unit, kPilotOrderClassRetreat, target, kPilotOrderIssueFlags);
}

void pilot_land_008a47b0(PilotOrderHost& host) {
    // 008A488D-008A48C1 and 008A48C6-008A48E4. Byte-for-byte the shape of
    // PilotMoveTo apart from the class literal.
    void* unit = host.unit_from_argument(kPilotOrderUnitArgument);
    SceneCommandTarget target = host.command_target_from_argument(kPilotOrderTargetArgument);
    // 008A48F9-008A4907 PUSH 1 / PUSH desc / PUSH 00E08FA0 / MOV ECX,ESI.
    host.issue_command(unit, kPilotOrderClassLand, target, kPilotOrderIssueFlags);
}

}  // namespace bsp
