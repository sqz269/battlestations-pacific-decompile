// Unit damage, invincibility and death. docs/UNIT_DAMAGE_AND_DEATH.md,
// reports/unit_damage.json. Packet cc2_unit_damage.
//
// Projections of the native routines named in each function's comment. The
// names are hypotheses. Nothing here is a drop-in binary replacement, and the
// session, effect and message calls are contracts on UnitDamageHost.
#include "bsp/unit_damage.hpp"

namespace bsp {
namespace {

// 0095DA22, 0087D73C, 0087908F, 00877C2F: the raw session mode is compared
// against 0, 1 and 2 and nothing else.
bool is_campaign(UnitSessionMode mode) noexcept {
    return mode == UnitSessionMode::campaign;
}

}  // namespace

// 00926DA1: MOV EDI,2 / CMP param,7 / CMOV-style select. A cause of 7 is stored
// as 2; every other value is stored unchanged.
int normalized_kill_cause_00926d90(int cause) noexcept {
    return cause == static_cast<int>(UnitKillCause::remapped_seven)
               ? static_cast<int>(UnitKillCause::hard)
               : cause;
}

// 008AC6DF..008AC71B: the default is 1, and a second Lua argument that reads
// true raises it to 2 (`cVar1 = (cVar1 != 0) + 1`).
UnitKillCause kill_cause_from_lua_008ac5c0(bool has_second_argument, bool hard) noexcept {
    if (!has_second_argument) {
        return UnitKillCause::normal;
    }
    return hard ? UnitKillCause::hard : UnitKillCause::normal;
}

// 00876260: FLD [ecx+370h] / FDIV [ecx+36Ch] / RET. No zero guard.
float health_fraction_00876260(const UnitHealth& health) noexcept {
    return health.current_health / health.max_health;
}

// 00923BE0: FLDZ and return when +5Dh is set, otherwise vtable[110h].
float entity_health_00923be0(const UnitHealth& health, bool released) noexcept {
    if (released) {
        return 0.0f;
    }
    return health_fraction_00876260(health);
}

// 00877C53..00877C77: FMUL qword [00D0DEE0] (256.0), __ftol2, clamp to [0,255].
int replicated_health_byte_00877b90(float health_fraction) noexcept {
    const int raw = static_cast<int>(static_cast<double>(health_fraction) * kUnitHealthByteScale);
    if (raw < 0) {
        return 0;
    }
    if (raw > kUnitHealthByteMax) {
        return kUnitHealthByteMax;
    }
    return raw;
}

// 008790E3..008790FD. The native sequence is FLD inv / FLD1 / FLD ST0 /
// FSUBRP ST2,ST0 / FXCH / FSTP [ESP+8] / FSUB [ESP+8] / FMUL [ESI+36Ch], so the
// intermediate 1.0f - inv is rounded through a float store before the second
// subtraction. Written the same way here on purpose: it is not inv * max.
float invincibility_floor_00879070(const UnitHealth& health) noexcept {
    const float one_minus = 1.0f - health.invincibility;
    return (1.0f - one_minus) * health.max_health;
}

// 00879070 body. Steps in the order of docs/UNIT_DAMAGE_AND_DEATH.md.
UnitDamageOutcome apply_damage_00879070(const UnitHealth& health,
                                        const UnitDamageGates& gates,
                                        float amount) noexcept {
    UnitDamageOutcome outcome{};
    outcome.clamped_amount = amount;
    outcome.new_health = health.current_health;

    // 00879085..008790A5: a client applies nothing, and a released entity is
    // refused before the sign of the amount is even looked at.
    if (gates.session_mode == UnitSessionMode::multiplayer_client || gates.release_requested) {
        outcome.refused = true;
        return outcome;
    }

    // 008790B4: only a positive amount runs the dead test and the floor. A
    // non-positive amount is a repair and skips both.
    if (amount > 0.0f) {
        // 008790C1: COMISS 0.0, health / JNC -> already dead, nothing to do.
        if (!(health.current_health > 0.0f)) {
            outcome.refused = true;
            return outcome;
        }
        // 008790D8: any positive invincibility installs a floor.
        if (health.invincibility > 0.0f) {
            const float floor = invincibility_floor_00879070(health);
            // 00879109: FCOMI floor, health / JNC -> floor >= health, refuse.
            if (!(floor < health.current_health)) {
                outcome.refused = true;
                return outcome;
            }
            const float headroom = health.current_health - floor;
            if (amount > headroom) {
                outcome.clamped_amount = headroom;
            }
        }
    }

    // 00879123..0087914B.
    outcome.new_health = health.current_health - outcome.clamped_amount;
    return outcome;
}

// 0095DA03..0095DA1B: CMP [EBX+71Ch],0 / JZ, then COMISS [EBX+728h], 0.0 / JA.
bool subobject_blocks_damage_0095da00(std::uint32_t owning_unit, float lockout) noexcept {
    return owning_unit != 0 && lockout > 0.0f;
}

// 0095DA51..0095DA7F and 0087D768..0087D79D share this shape.
UnitDifficultyScale scale_damage_by_difficulty(float amount, const float* table,
                                               std::size_t count, std::size_t level,
                                               bool enabled) noexcept {
    UnitDifficultyScale result{};
    result.amount = amount;
    if (!enabled) {
        return result;
    }
    if (table == nullptr || level >= count) {
        result.out_of_range = true;  // 0095DA6E / 0087D785 call the CRT range throw
        return result;
    }
    result.amount = table[level] * amount;
    return result;
}

// 00877B90 body.
UnitHealthWrite set_health_00877b90(const UnitHealth& health,
                                    float requested,
                                    UnitSessionMode session_mode,
                                    int last_replicated_byte,
                                    bool released) noexcept {
    UnitHealthWrite write{};
    write.stored_health = health.current_health;
    write.replicated_byte = last_replicated_byte;

    // 00877BB9: FUCOMIP / JNP -> an unchanged value returns without writing.
    if (health.current_health == requested) {
        return write;
    }

    // 00877BC7..00877BFA: clamp to [0, max].
    float stored = requested;
    if (!(0.0f <= requested)) {
        stored = 0.0f;
    } else if (requested > health.max_health) {
        stored = health.max_health;
    }

    write.wrote = true;
    write.stored_health = stored;

    // 00877C0C: the operand of the FSUB is the double 1.0 at 00D7A210.
    write.full_health_marker =
        static_cast<double>(stored) > static_cast<double>(health.max_health) - kUnitFullHealthEpsilon;

    // 00877C2F: the health-changed hook runs for everyone but a client.
    write.dispatch_health_changed = session_mode != UnitSessionMode::multiplayer_client;

    // 00877C48: only the host replicates the byte.
    if (session_mode == UnitSessionMode::multiplayer_host) {
        UnitHealth after = health;
        after.current_health = stored;
        const int byte = replicated_health_byte_00877b90(entity_health_00923be0(after, released));
        if (byte != last_replicated_byte) {
            write.replicated_byte_changed = true;
            write.replicated_byte = byte;
        }
    }
    return write;
}

// 00827AB0 and 00958DAA both test COMISS 0.0, [+370h] with a JC/JA that treats
// exactly zero as dead.
bool unit_is_dead(const UnitHealth& health) noexcept {
    return !(health.current_health > 0.0f);
}

// 008110F3..00811108.
bool sink_is_refused_008110f0(bool released, float invincibility) noexcept {
    return released || invincibility > 0.0f;
}

// 00891CAF..00891D3C. `corner` is the raw Lua integer after __ftol2.
UnitSinkPivot sink_pivot_00891b20(float descriptor_half_width,
                                  float descriptor_half_length,
                                  int corner) noexcept {
    // 00891CC2: the parity is a signed modulo 2, so a negative odd value keeps
    // its sign bit and still reads as non-zero here.
    const bool odd = (corner % 2) != 0;
    // 00891CD4: n < 2 -> 1, n < 4 -> 0, otherwise -1.
    int step = 0;
    if (corner < 2) {
        step = 1;
    } else {
        step = (corner < 4) ? 0 : -1;
    }
    UnitSinkPivot pivot{};
    pivot.x = static_cast<float>(descriptor_half_width * (odd ? 1.0f : -1.0f) *
                                 static_cast<float>(kUnitSinkPivotScale));
    pivot.y = 0.0f;
    pivot.z = static_cast<float>(descriptor_half_length * static_cast<float>(step) *
                                 static_cast<float>(kUnitSinkPivotScale));
    return pivot;
}

// ---------------------------------------------------------------------------
// Native routines as sequences over the host.
// ---------------------------------------------------------------------------

// 0095DA00, 00CFC3D0+1ACh. __thiscall(this, float), RET 4.
void unit_add_damage_0095da00(UnitDamageHost& host, std::uint32_t entity, float amount) {
    if (subobject_blocks_damage_0095da00(host.entity_subobject_owner(entity),
                                         host.entity_subobject_lockout(entity))) {
        return;  // 0095DA1B
    }
    float scaled = amount;
    if (is_campaign(host.session_mode()) && host.is_local_player_role(entity)) {
        // 0095DA36..0095DA7F: the level is read again inside the branch.
        scaled = host.unit_damage_multiplier(host.difficulty_level()) * scaled;
    }
    scale_damage_for_player_unit_0087d730(host, entity, scaled);  // 0095DA8D
}

// 0087D730. __thiscall(this, float), RET 4.
void scale_damage_for_player_unit_0087d730(UnitDamageHost& host, std::uint32_t entity, float amount) {
    float scaled = amount;
    // 0087D73C: multiplayer takes the multiplier unconditionally; in a campaign
    // only the current player's own unit does.
    if (!is_campaign(host.session_mode()) || host.is_current_player_unit(entity)) {
        scaled = host.player_damage_multiplier(host.difficulty_level()) * scaled;
    }
    unit_apply_damage_00879070(host, entity, scaled);  // 0087D7A3
}

// 00879070. __thiscall(this, float), RET 4.
void unit_apply_damage_00879070(UnitDamageHost& host, std::uint32_t entity, float amount) {
    const UnitHealth health = host.read_health(entity);
    UnitDamageGates gates{};
    gates.session_mode = host.session_mode();
    gates.release_requested = host.entity_release_requested(entity);

    const UnitDamageOutcome outcome = apply_damage_00879070(health, gates, amount);
    if (outcome.refused) {
        return;
    }

    unit_set_health_00877b90(host, entity, outcome.new_health);  // 0087914B

    // 00879150: the telemetry record only runs when the health really moved.
    if (!host.telemetry_enabled()) {
        return;
    }
    const UnitHealth after = host.read_health(entity);
    if (after.current_health == health.current_health) {
        return;
    }
    const char* kind = after.current_health < health.current_health ? "damage" : "repair";
    host.telemetry_event(entity, kind,
                         health.current_health / health.max_health,
                         after.current_health / health.max_health);  // 008791EB
}

// 00877B90. __thiscall(this, float), RET 4. The only writer of +370h.
void unit_set_health_00877b90(UnitDamageHost& host, std::uint32_t entity, float health_value) {
    const UnitHealth health = host.read_health(entity);
    const UnitHealthWrite write =
        set_health_00877b90(health, health_value, host.session_mode(),
                            host.last_replicated_health_byte(entity), host.entity_released(entity));
    if (!write.wrote) {
        return;
    }
    host.write_health(entity, write.stored_health);  // 00877C04
    if (write.full_health_marker) {
        host.write_full_health_marker(entity);  // 00877C20
    }
    if (!write.dispatch_health_changed) {
        return;
    }
    host.dispatch_health_changed(entity);  // 00877C40, vtable[1B0h]
    if (!write.replicated_byte_changed) {
        return;
    }
    host.write_replicated_health_byte(entity, write.replicated_byte);  // 00877C84
    host.send_health_message(entity, write.replicated_byte);           // 00877C8A + 00877C9E
}

// 00827A90, 00CFC3D0+1B0h. __fastcall(this), RET 0.
void ship_on_health_changed_00827a90(UnitDamageHost& host, std::uint32_t entity) {
    unit_react_to_health_change_00958a30(host, entity);  // 00827AAB, before the health test
    if (!unit_is_dead(host.read_health(entity))) {
        return;  // 00827AB9
    }
    const std::uint32_t descriptor = host.entity_class_descriptor(entity);
    if (host.descriptor_death_selector(descriptor) >= 0.0f) {
        // 00827B1A..00827B63: three availability tests and a random roll decide
        // whether the breakup message is built at all.
        if (!host.breakup_roll(entity)) {
            return;
        }
        host.send_breakup_message(entity);  // 00827B63 + 00827B77
        return;
    }
    host.send_death_message(entity, kUnitDeathSessionMessageId);  // 00827AE5 + 00827B77
}

// 00958A30. __fastcall(this), RET 0. Partial: only the root-unit branch
// 00958DA7-00958DCF is modelled; the subobject branch 00958A58-00958DA6 drives
// the per-part destruction sequence through vtable[1B4h], [19Ch] and [204h] and
// is not reconstructed.
void unit_react_to_health_change_00958a30(UnitDamageHost& host, std::uint32_t entity) {
    if (host.entity_subobject_owner(entity) != 0) {
        return;  // contract: unread, the subobject branch
    }
    if (!unit_is_dead(host.read_health(entity))) {
        return;  // 00958DB1
    }
    host.dispatch_destroy(entity, 1);  // 00958DBE, vtable[70h]
}

// 0042ED80, 00CE6290+F4h and 00CFC3D0+F4h. __thiscall(this, float), RET 4.
void entity_set_invincible_0042ed80(UnitDamageHost& host, std::uint32_t entity, float value) {
    host.write_invincibility(entity, value);  // 0042ED8F
    for (std::uint32_t child = host.first_child(entity); child != 0;
         child = host.next_sibling(child)) {
        // 0042EDB4: the stored float is re-read for each child, not the argument.
        host.dispatch_set_invincible(child, value);
    }
}

// 008110F0. __thiscall(this, const float pivot[3]), RET 4. The pivot argument is
// never read: no instruction in 008110F0-00811144 touches [ESP+8].
void unit_sink_008110f0(UnitDamageHost& host, std::uint32_t entity) {
    if (sink_is_refused_008110f0(host.entity_released(entity),
                                 host.read_health(entity).invincibility)) {
        return;  // 008110F7 / 00811108
    }
    host.dispatch_destroy(entity, 1);       // 00811111
    host.clear_sink_fields(entity);         // 00811116, 0081111E
    host.release_sink_attachment(entity);   // 00811135, only when +740h is non-null
}

// 00926C80, 00CE6290+70h. __thiscall(this, int recurse), RET 4.
void entity_destroy_00926c80(UnitDamageHost& host, std::uint32_t entity, int recurse) {
    host.enter_entity_lock();  // 00926C9C
    if (!host.entity_destroyed(entity)) {
        host.write_destroyed_flag(entity);  // 00926CD5
        // 00926CE1: the cause is only filled in when it is still 0, and it is
        // inherited from a hierarchy parent that is itself already destroyed.
        if (host.entity_death_cause(entity) == 0) {
            const std::uint32_t parent = host.hierarchy_parent(entity);
            const int cause = (parent != 0 && host.entity_destroyed(parent))
                                  ? host.entity_death_cause(parent)
                                  : static_cast<int>(UnitKillCause::normal);
            host.write_death_cause(entity, cause);
        }
        // 00926CF7 compares only the incoming stack argument's low byte.
        if (static_cast<std::uint8_t>(recurse) != 0) {
            for (std::uint32_t child = host.first_child(entity); child != 0;
                 child = host.next_sibling(child)) {
                if (host.destroy_child_predicate(child, entity)) {  // 00926D0E
                    // 00926D14 reloads the parent cause after the predicate;
                    // a still-zero cause clears this child's cause first.
                    if (host.entity_death_cause(entity) == 0)
                        host.write_death_cause(child, 0);           // 00926D1A
                    host.dispatch_destroy(child, 1);                // 00926D2A
                }
            }
        }
        host.queue_pending_destroy(entity);  // 00926D4E, the list at 00F899A8
    }
    host.leave_entity_lock();  // 00926D72
}

// 00926D90. __thiscall(this, int cause), RET 4.
void entity_kill_00926d90(UnitDamageHost& host, std::uint32_t entity, int cause) {
    const int stored_cause = normalized_kill_cause_00926d90(cause);
    host.enter_entity_lock();  // 00926DBE
    if (!host.entity_killed(entity)) {
        host.write_killed_flag(entity);  // 00926DEB
        if (!host.entity_destroyed(entity)) {
            host.write_death_cause(entity, stored_cause);
            host.dispatch_destroy(entity, 1);  // 00926E05, vtable[70h]
        }
        if (cause == static_cast<int>(UnitKillCause::remapped_seven)) {
            host.write_death_cause(entity, stored_cause);  // 00926E0F, after the destroy
        }
        for (std::uint32_t child = host.first_child(entity); child != 0;
             child = host.next_sibling(child)) {
            host.dispatch_kill(child, cause);  // 00926E19, the raw cause
        }
        host.queue_pending_kill(entity);  // 00926E40, the list at 00F899B4
    }
    host.leave_entity_lock();  // 00926E63
}

// 0077D1A0, 00CFC3D0+70h. __thiscall(this, int recurse), RET 4. No Ghidra
// function starts there; read from the raw bytes with capstone.
void unit_destroy_and_broadcast_0077d1a0(UnitDamageHost& host, std::uint32_t entity, int recurse) {
    if (host.destroy_broadcast_enabled() &&
        host.session_mode() != UnitSessionMode::multiplayer_client) {
        // 0077D1DB: the message carries the death cause already stored at +70h.
        host.send_destroy_message(entity, 0, recurse);  // 0077D1E4..0077D236
    }
    entity_destroy_00926c80(host, entity, recurse);  // 0077D243
    if (host.telemetry_enabled()) {
        host.telemetry_event(entity, "destroy", 0.0f, 0.0f);  // 0077D253, 00986480
    }
}

// 007ED380. __thiscall(group, int cause), RET 4. The members are the fixed array
// at +3D0h and the walk runs from count-1 down to 0.
void group_kill_members_array_007ed380(UnitDamageHost& host, std::uint32_t group, int cause) {
    const std::size_t count = host.group_member_count(group);
    for (std::size_t i = count; i-- > 0;) {
        host.dispatch_kill(host.group_member(group, i), cause);  // 007ED3A3
    }
}

// 00742210. __thiscall(group, char flag), RET 4.
void group_kill_members_vector_00742210(UnitDamageHost& host, std::uint32_t group, bool hard) {
    const int cause = hard ? static_cast<int>(UnitKillCause::hard)
                           : static_cast<int>(UnitKillCause::normal);
    const std::size_t count = host.group_vector_size(group);
    for (std::size_t i = 0; i < count; ++i) {
        const std::uint32_t member = host.group_vector_member(group, i);
        if (member == 0) {
            continue;
        }
        host.dispatch_kill(member, cause);          // 00742275
        host.clear_group_vector_member(group, i);   // 0074227A
    }
}

// 00935C70. __fastcall(parts), RET 0.
void parts_detach_all_live_00935c70(UnitDamageHost& host, std::uint32_t parts) {
    const std::size_t count = host.part_count(parts);
    for (std::size_t i = 0; i < count; ++i) {
        if (!(host.part_health(parts, i) > 0.0f)) {
            continue;  // 00935CC7
        }
        host.write_part_health(parts, i, kUnitPartDetachedHealth);  // 00935CEE
        host.detach_part(parts, i);                                 // 00935D16
    }
}

// 0092751F, inside BSP_EntityEventQueues_FlushPending 009273A0. The only
// on-killed dispatch found: ECX is the entity and there is no stack argument.
void flush_pending_kill_dispatch_0092751f(UnitDamageHost& host, std::uint32_t entity) {
    host.dispatch_on_killed(entity);
}

// ---------------------------------------------------------------------------
// The six Lua binding handlers. Each returns the Lua result count; only
// IsInvincible pushes a result.
// ---------------------------------------------------------------------------

// 0088E000.
int lua_add_damage_0088e000(UnitDamageHost& host) {
    const std::uint32_t entity = host.entity_from_lua_table(0);  // 0088E0FF
    const float amount = host.lua_number_argument(1);            // 0088E0DE
    host.dispatch_add_damage(entity, amount);                    // 0088E15B, vtable[1ACh]
    return 0;
}

// 008AC5C0.
int lua_kill_008ac5c0(UnitDamageHost& host) {
    const std::uint32_t entity = host.entity_from_lua_table(0);
    const bool has_second = host.lua_argument_count() > 1;  // 008AC6DF
    const bool hard = has_second && host.lua_boolean_argument(1);
    const int cause = static_cast<int>(kill_cause_from_lua_008ac5c0(has_second, hard));

    if (host.class_id_test(entity, kKillGroupArrayClassId)) {         // 008AC729
        group_kill_members_array_007ed380(host, entity, cause);       // 008AC732
    } else if (host.class_id_test(entity, kKillGroupVectorClassId)) { // 008AC740
        // 008AC74E pushes the boolean form, not the 1/2 cause.
        group_kill_members_vector_00742210(host, entity, cause == static_cast<int>(UnitKillCause::hard));
    } else {
        entity_kill_00926d90(host, entity, cause);  // 008AC756
    }
    return 0;
}

// 00891B20.
int lua_sink_00891b20(UnitDamageHost& host) {
    const std::uint32_t entity = host.entity_from_lua_table(0);
    const int corner = static_cast<int>(host.lua_number_argument(1));  // 00891C3E + __ftol2
    const std::uint32_t descriptor = host.entity_class_descriptor(entity);
    // The pivot is computed and passed, and 008110F0 never reads it.
    (void)sink_pivot_00891b20(host.descriptor_half_width(descriptor),
                              host.descriptor_half_length(descriptor), corner);
    unit_sink_008110f0(host, entity);  // 00891CFD
    return 0;
}

// 00897A50.
int lua_set_invincible_00897a50(UnitDamageHost& host) {
    const std::uint32_t entity = host.entity_from_lua_table(0);
    float value = kUnitInvincibleOff;
    if (host.lua_argument_is_boolean(1)) {  // 00897B6F
        value = host.lua_boolean_argument(1) ? kUnitInvincibleFull : kUnitInvincibleOff;
    } else {
        value = host.lua_number_argument(1);  // a fraction of maximum health
    }
    host.dispatch_set_invincible(entity, value);  // 00897C63, vtable[F4h]
    return 0;
}

// 00897CB0.
int lua_is_invincible_00897cb0(UnitDamageHost& host) {
    const std::uint32_t entity = host.entity_from_lua_table(0);
    // 00897DC6: COMISS 0.0, [ESI+150h]; the pushed boolean is inv > 0.
    host.lua_push_boolean(host.read_health(entity).invincibility > 0.0f);
    return 1;
}

// 0088E1B0.
int lua_explode_to_parts_0088e1b0(UnitDamageHost& host) {
    const std::uint32_t entity = host.entity_from_lua_table(0);
    parts_detach_all_live_00935c70(host, host.parts_object(entity));  // 0088E2C8 + 0088E2CF
    return 0;
}

}  // namespace bsp
