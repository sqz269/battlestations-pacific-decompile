// The mission Lua bindings that order the mission's ships.
//
// Packet cc_lua_navigator. Every routine here is a sequence over the injected host of
// include/bsp/lua_binding_navigator.hpp; no native effect is invented and no global is
// fabricated. Addresses, ABI and uncertainty are in the header.

#include "bsp/lua_binding_navigator.hpp"

namespace bsp {
namespace {

// The two Lua argument indices every navigator binding uses, in
// mission_binding_argument_slot terms: index 0 is Lua slot 1.
constexpr int kArgumentSelf = 0;    // 008A31AD PUSH EBP (EBP = 0), 008A2FFD, 00899DEE
constexpr int kArgumentTarget = 1;  // 008A31E6 PUSH 0x1, 008A3036, 00899E27

// 008A30D0 and 008A2F20 differ only in this constant, so both go through one body.
int issue_navigator_command(LuaCommandTargetSource& targets, LuaBindingNavigatorHost& host,
                            std::uint32_t command_object)
{
    // 008A31CF / 008A301F: the acting entity, before the target is read.
    void* entity = host.argument_entity(kArgumentSelf);

    // 008A3204 / 008A3054: 0088A810 over Lua argument 1.
    const SceneCommandTarget target = lua_read_command_target(targets, kArgumentTarget);

    // 008A3273 / 008A3077. The native passes whatever 00888AA0 answered, including null: it
    // makes no null test between the resolve and the call, and 0077D600's own body starts by
    // copying the descriptor rather than by touching the entity. Reproducing that would mean
    // handing a null `this` to the host, so the reconstruction stops here instead and reports
    // the difference rather than pretending the native guards.
    if (entity != nullptr) {
        host.entity_issue_command(entity, command_object, target, kNavigatorIssueFlags);
    }

    // 00B66400 at 008A327C / 008A3080: the handler pushed nothing above its arguments.
    return 0;
}

}  // namespace

// ---------------------------------------------------------------------------
// 0088A810
// ---------------------------------------------------------------------------

SceneCommandTarget lua_read_command_target(LuaCommandTargetSource& source, int index)
{
    SceneCommandTarget target{};

    // 0088A83B fetches "ID"; 0088A84A tests it.
    if (!source.argument_id_field_is_nil(index)) {
        // 0088A855, then the zero vector at 00F87574 into the three position floats
        // (0088A85C..0088A87E) and the pointer into +4h (0088A886).
        void* object = source.argument_entity(index);
        target.position[0] = 0.0f;
        target.position[1] = 0.0f;
        target.position[2] = 0.0f;
        target.kind = 0;
        target.position_valid = 0;  // 0088A883 writes the whole word
        target.object = object;
        if (object != nullptr) {
            // 0088A88B / 0088A88E / 0088A895, then the JMP at 0088A899 skips the tail.
            target.kind = 1;
            target.object_id = source.entity_object_id(object);
            target.reserved = 0.0f;  // 0088A8C7 XORPS, reached by both paths
            return target;
        }
        // Null falls into the shared tail below with kind and object_id cleared again.
    } else {
        // 0088A8A1 reads the value as a Vector3; 0088A8A6 clears the word, 0088A8A9 sets
        // position_valid, 0088A8AD..0088A8BB store the floats, 0088A8BE clears the object.
        float position[3] = {0.0f, 0.0f, 0.0f};
        source.argument_vector3(index, position);
        target.kind = 0;
        target.position_valid = 1;
        target.position[0] = position[0];
        target.position[1] = position[1];
        target.position[2] = position[2];
        target.object = nullptr;
    }

    // 0088A8C1 and 0088A8C5: the shared tail both surviving paths reach.
    target.object_id = 0;
    target.kind = 0;
    target.reserved = 0.0f;  // 0088A8C7 XORPS / 0088A8D6
    return target;
}

// ---------------------------------------------------------------------------
// 008A30D0, 008A2F20 (and byte-identically 008A2BC0, 008A2D70)
// ---------------------------------------------------------------------------

int lua_binding_navigator_attack_move(LuaCommandTargetSource& targets,
                                      LuaBindingNavigatorHost& host)
{
    return issue_navigator_command(targets, host, kCommandObjectAttackMove);
}

int lua_binding_navigator_move_to(LuaCommandTargetSource& targets, LuaBindingNavigatorHost& host)
{
    return issue_navigator_command(targets, host, kCommandObjectMoveTo);
}

// ---------------------------------------------------------------------------
// 0077C8D0 and 00899D10
// ---------------------------------------------------------------------------

FormationJoinOutcome entity_join_formation(LuaBindingNavigatorHost& host, void* follower,
                                           void* leader)
{
    // 0077C8F8: the predicate decides everything. A false answer leaves the routine with no
    // effect at all (0077C902 jumps straight to the epilogue).
    if (!host.entity_command_is_available(follower, kFormationFollowCommandName, leader)) {
        return FormationJoinOutcome::kRefused;
    }

    // 0077C904..0077C91B. The bound is unsigned, so a negative slot is also out of range.
    const int slot = host.entity_route_slot(follower);
    if (static_cast<unsigned int>(slot) <= static_cast<unsigned int>(kEntityRouteSlotMax)) {
        host.slot_counter_increment(slot);
    }

    // 0077C926..0077C964: type 76h carrying the leader's uint16 id, routed from the follower.
    // 0077C92B reads entity+174h off the *stack argument*, which is the leader.
    const std::uint16_t leader_id = leader != nullptr ? host.entity_object_id(leader) : 0;
    host.session_route_formation_message(follower, leader_id);
    return FormationJoinOutcome::kRequested;
}

int lua_binding_join_formation(LuaBindingNavigatorHost& host)
{
    // 00899E10 then 00899E41: argument 0 is the follower, argument 1 the leader. The
    // direction is settled by 0077C8F8 asking whether the *follower* may `follow` the leader.
    void* follower = host.argument_entity(kArgumentSelf);
    void* leader = host.argument_entity(kArgumentTarget);

    // 00899E59 puts the follower in ECX. A null there would mean a null `this`, which this
    // reconstruction declines to synthesise; see the note in issue_navigator_command.
    if (follower != nullptr) {
        entity_join_formation(host, follower, leader);
    }
    return 0;  // 00B66400 at 00899E64
}

// ---------------------------------------------------------------------------
// 00895250
// ---------------------------------------------------------------------------

int lua_binding_set_skill_level(LuaBindingNavigatorHost& host)
{
    // 00895351 reads argument 1 before 00895381 reads argument 0; the order matters only for
    // the Lua-side error a bad argument would raise, which this reconstruction does not model.
    const int level = host.argument_integer(kArgumentTarget);
    void* entity = host.argument_entity(kArgumentSelf);

    // 0089539A / 008953A2. No clamp and no range test: the script's value reaches the virtual.
    if (entity != nullptr) {
        host.entity_set_skill_level(entity, level);
    }
    return 0;  // 00B66400 at 008953AC
}

// ---------------------------------------------------------------------------
// 008AD330
// ---------------------------------------------------------------------------

int lua_binding_repair_enable(LuaBindingNavigatorHost& host, RepairEnableArm& arm_out)
{
    // 008AD42E, then 008AD448 asks IsKindOf(6) *before* 008AD46D reads the boolean.
    void* entity = host.argument_entity(kArgumentSelf);
    if (entity == nullptr) {
        arm_out = RepairEnableArm::kLocalFieldWrite;
        return 0;
    }
    const bool routed = host.entity_is_kind_of(entity, kRepairEnableIsKindOfClassId);
    const bool enabled = host.argument_boolean(kArgumentTarget);

    // 008AD487 TEST BL,BL / JZ 008AD4E4: the IsKindOf answer selects the arm, and the two are
    // exclusive. True routes the change, false writes it locally.
    if (routed) {
        host.session_route_repair_enable_message(entity, enabled);  // 008AD494, 008AD4CD
        arm_out = RepairEnableArm::kRoutedMessage;
    } else {
        host.entity_set_repair_enabled_field(entity, enabled);  // 008AD4E8
        arm_out = RepairEnableArm::kLocalFieldWrite;
    }
    return 0;  // 00B66400 at 008AD4F2
}

// ---------------------------------------------------------------------------
// 008AB850
// ---------------------------------------------------------------------------

int lua_binding_set_role_available(LuaBindingNavigatorHost& host, SetRoleAvailableArm& arm_out)
{
    // 008AB953 uses 00888D20, not 00888AA0: the table's `Ptr` without the entity validation.
    void* owner = host.argument_ptr_field(kArgumentSelf);
    const int role = host.argument_integer(kArgumentTarget);   // 008AB984
    const int value = host.argument_integer(kArgumentTarget + 1);  // 008AB9B4

    if (owner == nullptr) {
        arm_out = SetRoleAvailableArm::kDirectCall;
        return 0;
    }

    // 008AB9D1: the session mode at [00E188A8]+1FE4h.
    if (host.game_session_mode() == kSetRoleAvailableLocalSessionMode) {
        host.role_owner_set_role_available(owner, role, value);  // 008ABA51
        arm_out = SetRoleAvailableArm::kDirectCall;
        return 0;
    }

    // 008AB9DE then 008AB9E3: the effective game mode splits the two networked arms.
    if (host.game_effective_game_mode() < kSetRoleAvailableRoutedModeLimit) {
        host.session_route_role_message(owner, role, value);  // 008AB9EE, 008ABA2F
        arm_out = SetRoleAvailableArm::kRoutedMessage;
        return 0;
    }

    host.role_owner_set_role_available(owner, role, value);  // 008ABA73
    if (value < kSetRoleAvailablePartyReassignLimit) {       // 008ABA57
        host.game_assign_party_player_slots(0);              // 008ABA60
    }
    arm_out = SetRoleAvailableArm::kDirectCallWithParty;
    return 0;  // 00B66400 at 008ABA79
}

}  // namespace bsp
