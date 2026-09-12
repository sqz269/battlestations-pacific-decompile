// The unit command controller at unit+738h. docs/WEAPON_DIRECTOR.md carries the
// evidence for every claim here; the addresses in the comments are the native
// call sites the sequences follow.
#include "bsp/weapon_director.hpp"

namespace bsp {
namespace {

// The class ids 008363E0 asks the session endpoint at +34h before defaulting
// the two permissions. What they name is unread.
constexpr int kEndpointClassTestA = 0x0b; // first test
constexpr int kEndpointClassTestB = 0x09; // second test

constexpr int kSessionRouteFlags = 0; // the third argument at every route site

} // namespace

// 008366D0 and its base 008363E0. Order: the endpoint first (008366F9), the base
// constructor (00836705), the two vtables (00836717, 0083671D), the six named
// fields (00836724-00836745), the owner (0083674D), the 40h subobject (0083676A).
WeaponDirectorState construct_director_008366d0(WeaponDirectorHost& host, NativeHandle unit) {
    const NativeHandle endpoint = host.unit_session_endpoint(unit);
    host.construct_command_array(endpoint);

    WeaponDirectorState state{};
    // 008363E0: the two permissions default to true only when the endpoint is
    // of neither tested class; otherwise they keep the array constructor's zero.
    const bool tested_class = host.endpoint_class_test(endpoint, kEndpointClassTestA) ||
                              host.endpoint_class_test(endpoint, kEndpointClassTestB);
    state.allow_fire = !tested_class;
    state.allow_move = !tested_class;
    state.fire_target = 0;         // 0083642F
    state.target_change_gate = false;

    state.torpedo_avoidance = true;        // 00836724
    state.ship_collision_avoidance = true; // 0083672A
    state.land_collision_avoidance = true; // 00836730
    state.cruise_is_heading = false;       // 00836736
    state.cruise_steer_or_heading = 0.0f;  // 0083673D
    state.cruise_thrust = 0.0f;            // 00836745
    state.owner_unit = unit;               // 0083674D

    host.construct_subobject_38h(); // 0083676A, stored at +38h
    return state;
}

// 0071D560, decoded from 0071D560-0071D57A: CMP 1, CMP 2, else zero.
bool director_stance_allows_fire_0071d560(FireStance stance) noexcept {
    return stance == FireStance::FreeFire || stance == FireStance::FreeAttack;
}

// 0071D580, decoded from 0071D580-0071D59A: CMP 3, CMP 2, else zero.
bool director_stance_allows_move_0071d580(FireStance stance) noexcept {
    return stance == FireStance::MoveOnly || stance == FireStance::FreeAttack;
}

// 0071BE80. Both questions are asked before either answer is applied: the fire
// answer is parked in a stack byte at 0071BE91 while 0071BE9D runs.
void set_fire_stance_0071be80(WeaponDirectorHost& host, FireStance stance) {
    const bool allow_fire = host.director_stance_allows_fire(stance); // 0071BE8F
    const bool allow_move = host.director_stance_allows_move(stance); // 0071BE9D
    host.send_allow_fire(allow_fire);                                 // 0071BEAF
    host.send_allow_move(allow_move);                                 // 0071BEBD
}

void hold_fire_0071bed0(WeaponDirectorHost& host) {
    set_fire_stance_0071be80(host, FireStance::HoldFire); // literal 0 at 0071BEDB
}

void free_fire_0071bf20(WeaponDirectorHost& host) {
    set_fire_stance_0071be80(host, FireStance::FreeFire); // literal 1 at 0071BF2B
}

void free_attack_0071bf70(WeaponDirectorHost& host) {
    set_fire_stance_0071be80(host, FireStance::FreeAttack); // literal 2 at 0071BF7B
}

// 0071DA50 and its nine siblings. Each writes the same header and differs only
// in the sub-kind dword at message +20h.
DirectorCommandMessage director_command_message_0071da50(DirectorCommandSubKind sub_kind,
                                                         bool value) {
    DirectorCommandMessage message{};
    message.base_kind = kSessionMessageKindWeaponEnable; // 5Ah pushed at 0071DA6B
    message.vtable = kDirectorCommandMessageVtable;      // 0071DA92
    message.header_dword = 1;                            // 0071DA86
    message.sub_kind = static_cast<int>(sub_kind);       // 0071DA9A
    message.value = value ? 1u : 0u;                     // 0071DA9E, MOVZX of the byte argument
    return message;
}

// 00836210, decoded from 00836210-0083623A. The store comes first; clearing the
// target happens only when fire is being forbidden.
void store_allow_fire_00836210(WeaponDirectorHost& host,
                               WeaponDirectorState& state,
                               bool allowed) {
    state.allow_fire = allowed; // 00836216
    if (!allowed && state.fire_target != 0) {
        host.release_target_reference(state.fire_target); // 0083622B
        state.fire_target = 0;                            // 00836230
    }
}

// 0071D5E0: MOV AL,[ESP+4]; MOV [ECX+3Dh],AL; RET 4.
void store_allow_move_0071d5e0(WeaponDirectorState& state, bool allowed) noexcept {
    state.allow_move = allowed;
}

// 00835640 (derived, raw bytes 00835640-0083568E) then 0071C1E0 (base). The
// derived override handles the three avoidance sub-kinds and tail-jumps to the
// base at 0083568A for everything else.
void apply_command_message_00835640(WeaponDirectorHost& host,
                                    WeaponDirectorState& state,
                                    const DirectorCommandMessage& message) {
    const bool flag = message.value != 0;
    switch (static_cast<DirectorCommandSubKind>(message.sub_kind)) {
    case DirectorCommandSubKind::TorpedoAvoidance: // 00835653
        state.torpedo_avoidance = flag;
        return;
    case DirectorCommandSubKind::ShipCollisionAvoidance: // 00835668
        state.ship_collision_avoidance = flag;
        return;
    case DirectorCommandSubKind::LandCollisionAvoidance: // 0083567D
        state.land_collision_avoidance = flag;
        return;
    // 0071C1E0 from here on.
    case DirectorCommandSubKind::AllowFire:
        store_allow_fire_00836210(host, state, flag); // through vtable[64h]
        return;
    case DirectorCommandSubKind::AllowMove:
        store_allow_move_0071d5e0(state, flag); // through vtable[68h]
        return;
    case DirectorCommandSubKind::SubKind2Dword:
        state.sub_kind_2_value = message.value; // the only sub-kind that keeps the dword
        return;
    case DirectorCommandSubKind::ArtilleryEnable: // 0071C231
        state.artillery_flag = flag;
        return;
    case DirectorCommandSubKind::SubKind4: // 0071C246
        state.sub_kind_4_flag = flag;
        return;
    case DirectorCommandSubKind::TorpedoEnable: // 0071C25B
        state.torpedo_flag = flag;
        return;
    case DirectorCommandSubKind::SubKind6: // 0071C270
        state.sub_kind_6_flag = flag;
        return;
    }
    // 0071C1E0 falls through without writing anything for an unknown sub-kind.
}

// 008364E0.
NativeHandle fire_target_008364e0(const WeaponDirectorState& state) noexcept {
    return state.fire_target;
}

// The gate at the head of 00835860.
bool accepts_fire_target_00835860(const WeaponDirectorState& state, bool force) noexcept {
    return force || !state.target_change_gate || state.fire_target == 0;
}

// 00835740, __thiscall(msg)(target, force), RET 8.
DirectorTargetMessage build_target_message_00835740(WeaponDirectorHost& host,
                                                    NativeHandle target,
                                                    bool force) {
    DirectorTargetMessage message{};
    message.base_kind = kDirectorTargetMessageKind; // 5Eh pushed at 0083575A
    message.vtable = kDirectorTargetMessageVtable;  // 00835782
    message.header_dword = 1;                       // 0083576E
    message.target_id = target != 0 ? host.entity_id(target) : std::uint16_t{0}; // 0083578A
    message.target_passes_class_test =
        target != 0 && host.target_class_test(target, kDirectorTargetClassTestId); // 008357A4
    message.force = force; // 008357B7
    return message;
}

// 00835860. Nothing is written locally: the target crosses the session as an id.
void set_fire_target_00835860(WeaponDirectorHost& host,
                              const WeaponDirectorState& state,
                              NativeHandle target,
                              bool force) {
    if (!accepts_fire_target_00835860(state, force)) {
        return;
    }
    const DirectorTargetMessage message = build_target_message_00835740(host, target, force);
    host.session_route_target_message(message, kSessionRouteChannel, kSessionRouteFlags);
}

// 00720CD0 (vtable[58h]). The clear walks index 9 down to 0 over slots of 1Ch
// starting at +54h, then one command is issued with the target's id and the
// three floats at 00F87574.
void issue_target_command_00720cd0(WeaponDirectorHost& host,
                                   NativeHandle target,
                                   const float position[3]) {
    for (int index = kDirectorCommandSlotCount - 1; index >= 0; --index) {
        if (host.command_slot_occupied(index)) {
            host.clear_command_slot(index); // 00720850
        }
    }
    const bool has_target = target != 0;
    const std::uint16_t target_id = has_target ? host.entity_id(target) : std::uint16_t{0};
    host.issue_command(kDirectorAttackCommandDescriptor, has_target, target_id, position);
}

} // namespace bsp
