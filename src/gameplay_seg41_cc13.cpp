// Segment 41 reference models: the LandConvoy damage pooling, the land fort
// destruction and death-smoke rules, the landing ship's ramp anchor and the Lua
// mission triggers' checks. Packet cc13-gpr41; docs/CC13_GPR_41.md. Read from
// the installed executable's bytes (Capstone disasm-raw) and the Ghidra
// pseudocode; the listing decides every comparison. Main-thread gameplay runs
// with the x87 at 24-bit precision (docs/CC13_FPU_PRECISION.md), so each x87
// result is a float; the models compute in double and round with f32() at each
// FSTP, which is exact for these single operations.

#include "bsp/gameplay_seg41_cc13.hpp"

#include "bsp/entity_identity.hpp"  // compare_insensitive_00438e10

#include <cstring>

namespace bsp {
namespace gameplay_seg41_cc13 {

namespace {

constexpr float kZero_00d7a218 = 0.0f;
constexpr float kOne_00d7a24c = 1.0f;
constexpr float kMasterKillDamage_00ce4970 = 1.0e10f;
constexpr float kShortLength_00ce5444 = 80.0f;
constexpr float kMovingSpeed_00ce3854 = 3.0f;
constexpr double kLosRecheck_00d7a280 = 0.5;  // qword

constexpr const char* kCommandSetTarget_00cf8b0c = "settarget";
constexpr const char* kCommandClearOrders_00cf8af4 = "clearorders";
constexpr const char* kCommandClearTarget_00cf8b00 = "cleartarget";

inline float f32(double v) { return static_cast<float>(v); }

}  // namespace

// ---------------------------------------------------------------------------
// 1. LandConvoy
// ---------------------------------------------------------------------------

// 00741140: MOV [ECX+368h], arg; RET 4. Caller luaMW_LandConvoySetSpeed 00895850.
void set_speed_00741140(ConvoyState& c, float speed) { c.speed_368 = speed; }

// 00741160: MOV [ECX+344h], arg; RET 4. Caller the Lua native 008959F0.
void set_field_344_00741160(ConvoyState& c, std::int32_t value) { c.field_344 = value; }

// 0074CD30-0074CD8B, MLandVehicle vtable[1ACh] (00CFFDE0+1ACh).
void add_damage_parent_aware_0074cd30(ConvoyState* convoy, std::uint32_t unit, float amount,
                                      ConvoyHost& host) {
    // 0074CD3B / 0074CD44: no convoy or the convoy's reentry byte set takes the
    // base. 0074CD4E UCOMISS, LAHF, TEST AH,44h, JP: only an ordered equality
    // with 0.0 takes it too, so a NaN or a nonzero +364h skips the base.
    if (convoy == nullptr || convoy->reentry_390 || convoy->field_364 == kZero_00d7a218) {
        host.base_add_damage_0095da00(unit, amount);  // 0074CD63
    }
    // 0074CD68..0074CD84: re-read +738h, then the convoy unless reentering.
    if (convoy != nullptr && !convoy->reentry_390) {
        on_member_damaged_00742330(*convoy, amount, host);
    }
}

// 00742330-007423F1, reached only from 0074CD30.
void on_member_damaged_00742330(ConvoyState& c, float amount, ConvoyHost& host) {
    // 0074233C COMISS [00D7A218], JBE: 0 < +364h, ordered.
    if (c.field_364 > kZero_00d7a218) {
        c.reentry_390 = true;                                   // 00742349
        if (c.members_398.empty()) {                            // 0074235A..00742365
            host.invalid_iterator_00bf6713();
            return;
        }
        host.member_add_damage(c.members_398.front(), amount);  // 00742384 vtable[1ACh]
        c.reentry_390 = false;                                  // 00742386
        // 007423B9 UCOMISS, JP: the lead's +370h equal to 0.0 kills the roster.
        if (host.member_health_370(c.members_398.front()) == kZero_00d7a218) {
            host.kill_members_00742210(0);                      // 007423CA
        }
    }
    // 007423CF..007423D9: FLD +3C0h, FADD amount, FSTP float.
    c.collected_damage_3c0 = f32(static_cast<double>(c.collected_damage_3c0) + amount);
    c.last_hit_3c4 = host.fixed_step_clock();                   // 007423E7
}

// 00742940-00742A09, convoy vtable[110h].
float health_00742940(const ConvoyState& c, ConvoyHost& host) {
    // 0074294F COMISS, JBE: 0 < +364h (ordered) returns the lead's health.
    if (c.field_364 > 0.0f) {
        if (c.members_398.empty()) {
            host.invalid_iterator_00bf6713();
            return 0.0f;
        }
        return host.member_get_health_00923be0(c.members_398.front());  // 0074297C JMP
    }
    // 00742996..007429F2: sum = f(health + sum) over the non-null members.
    float sum = 0.0f;
    for (std::uint32_t m : c.members_398) {
        if (m == 0) continue;                                    // 007429C7
        sum = f32(static_cast<double>(host.member_get_health_00923be0(m)) + sum);  // 007429DD
    }
    // 007429F9 FIDIV [+3B8h], 00742A02 FSTP float. InitialSize 0 gives inf or
    // NaN with the masked exceptions, as here.
    return f32(static_cast<double>(sum) / static_cast<double>(c.initial_size_3b8));
}

// 00741F20, convoy vtable[0F4h].
void set_invincible_00741f20(const ConvoyState& c, bool on, ConvoyHost& host) {
    host.base_set_invincible_0042ed80(on);
    for (std::uint32_t m : c.members_398) {
        if (m != 0) host.member_set_invincible(m, on);  // member vtable[0F4h]
    }
}

// ---------------------------------------------------------------------------
// 2. MLandFort / MCommandBuilding
// ---------------------------------------------------------------------------

// 00743F50-00743F8D. Each 00438E10 call returns 0 on a match.
bool supports_command_00743f50(const char* command) {
    if (compare_insensitive_00438e10(command, kCommandSetTarget_00cf8b0c) == 0) return true;
    if (compare_insensitive_00438e10(command, kCommandClearOrders_00cf8af4) == 0) return true;
    return compare_insensitive_00438e10(command, kCommandClearTarget_00cf8b00) == 0;
}

// 00744B30-00744BDB, vtable[16Ch] of 00CFF3F8 and 00CFB028.
bool may_follow_target_00744b30(const char* command, std::int32_t self_party_54,
                                const FollowTargetView& target, FortCommandHost& host) {
    // 00744B3A: this->vtable[168h](command); both classes install 00743F50.
    if (!supports_command_00743f50(command)) return false;
    if (!target.present) {
        // 00744B57..00744B6C: NEG AL; SBB; ADD 1, so the answer is slot8 == false.
        return !host.command_object_slot8(command);
    }
    // 00744B72..00744B9D: kind 5, then 18h, then 1Ah (vtable[5Ch]).
    if (!(target.kind_05 || target.kind_18 || target.kind_1a)) return false;
    if (target.dead_5d) return false;                                          // 00744B9F
    if (host.party_relative_to_00803510(self_party_54, target.party_54) == 0) return false;  // 00744BAB
    if (!target.kind_08) return true;                                          // 00744BC1
    return host.target_above_depth_charge_depth_00852820();                    // 00744BC5
}

// 00744A20-00744A84.
SetMasterResult set_master_00744a20(bool master_present, std::int32_t self_party_54,
                                    std::int32_t master_party_54, float self_max_health_36c,
                                    float self_health_370) {
    SetMasterResult r;
    if (!master_present) return r;                         // 00744A3A
    r.apply = true;
    // 00744A3C..00744A48: this party >= 2 and the master's < 2 (signed JL/JGE).
    r.plan.adopt_master_party = self_party_54 >= 2 && master_party_54 < 2;
    r.plan.master_max_health_36c = self_max_health_36c;  // 00744A65..00744A6C
    r.plan.master_health_370 = self_health_370;          // 00744A74..00744A7D, 00877B90
    return r;
}

// 00747400-007474DB, MLandFort vtable[1A8h].
FortDieOutcome die_00747400(bool dead_5d, bool inferior_71c, bool field_50,
                            const std::vector<bool>& chain_dead_5d) {
    if (dead_5d) return FortDieOutcome::already_dead;            // 0074741E
    if (inferior_71c) return FortDieOutcome::inferior_failure;   // 00747426..00747456
    if (!field_50) return FortDieOutcome::not_eligible;          // 00747493
    // 007474A0 CMP [EAX+5Dh],0; JE 007474CC: one live element ends the rule.
    for (bool dead : chain_dead_5d) {
        if (!dead) return FortDieOutcome::chain_alive;
    }
    return FortDieOutcome::die;                                  // 007474B0..007474C9
}

// 007465E0-00746811.
void start_death_fire_007465e0(FortTimers& t, const FortClassRow& cls, LandFortHost& host) {
    // 007465FD..00746645: a burning effect is stopped and dropped first.
    if (t.fire_740) {
        host.stop_fire_effect_740();
        t.fire_740 = false;
    }
    if (t.smoke_744) {
        // 00746785..007467E4: draw, then keep +748h when it is larger
        // (FCOMIP with JBE, so a tie or NaN takes the draw).
        const float d = host.uniform(0, cls.fire_duration_min_168, cls.fire_duration_max_16c);
        if (!(t.time_748 > d)) t.time_748 = d;
        return;
    }
    if (!cls.death_smoke_definition_160) return;                 // 00746681
    t.smoke_744 = host.create_death_smoke_effect();              // 007466D0..007466E0
    // 0074672B: the draw happens whether or not the effect came back.
    t.time_748 = host.uniform(0, cls.fire_duration_min_168, cls.fire_duration_max_16c);
    if (t.smoke_744) host.send_effect_message_7(t.time_748);    // 0074675A, 0074676E
}

// 00745F1E-00745F9D.
void smoke_frame_00745f1e(FortTimers& t, float dt, LandFortHost& host) {
    if (!t.smoke_744) return;                                    // 00745F36
    t.time_748 = f32(static_cast<double>(t.time_748) - dt);     // 00745F47 FSUBP, 00745F4B
    t.clock_750 = f32(static_cast<double>(t.clock_750) + dt);   // 00745F5F FADDP, 00745F63
    // 00745F69 FLDZ; FCOMIP ST1; JB skips: the smoke stops when 0 >= time (ordered).
    if (kZero_00d7a218 >= t.time_748) {
        host.stop_smoke_effect_744();
        t.smoke_744 = false;
    }
}

// 00747170-007473FA, vtable[7Ch] of 00CFF3F8 and 00CFB028.
void on_destroyed_00747170(const FortDestroyInput& in, FortDestroyHost& host) {
    // 0074718D CMP [+70h],1; 007471A2 COMISS clock, [00D7A24C] = 1.0, JBE.
    if (in.state_70 == 1 && in.clock > kOne_00d7a24c) {
        host.set_damaged_gfx_level(in.self, host.last_damage_state_index_0072ab10(in.self));
        if (in.part_list_19c) host.rebuild_part_render_list();     // 007471C1..00747261
        if (in.master_738) {
            host.master_add_damage(kMasterKillDamage_00ce4970);    // 00747271..00747283
            if (host.master_die_gate()) host.master_die();          // 0074728B..007472AD
        }
        host.create_death_effect();                                // 007472AF..00747330
        // 0074733E COMISS class+164h, [00D7A218], JBE.
        if (in.smoke_fire_chance_mul_164 > kZero_00d7a218) host.start_death_fire();
        // 0074735A..007473B3: each chain element takes its own last level
        // (inline 0072AB10: +360h vector count - 1, or 0).
        for (std::uint32_t e : in.chain_48) {
            host.set_damaged_gfx_level(e, host.last_damage_state_index_0072ab10(e));
        }
    }
    host.base_on_destroyed_00959450();                             // 007473E3
}

// ---------------------------------------------------------------------------
// 3. MLandingShip
// ---------------------------------------------------------------------------

// 0074A420-0074A4B4.
void lower_ramp_0074a420(LandingShipAnchor& a, const Vec3& node_world, const float pose_74[16]) {
    a.ramp_down_1188 = true;   // 0074A426
    a.anchored_1189 = true;    // 0074A42C
    a.ramp_node_11b4 = node_world;  // 0074A494..0074A4A4
    std::memcpy(a.pose_11c0, pose_74, sizeof(a.pose_11c0));  // 0074A4AC 004134F0
}

// 0074AC30-0074AC86. Only InitAll (0074BEFB) and the save load (0074C347)
// clear +1189h, so once lowered the ship holds the captured pose.
void place_step_pose_0074ac30(const LandingShipAnchor& a, float dt, LandingShipPoseHost& host) {
    if (!a.anchored_1189) {
        host.run_controller_update_00811ab0(dt);  // 0074AC80
        return;
    }
    host.set_pose_74(a.pose_11c0);    // 0074AC49
    host.clear_pose_valid_flags();    // 0074AC4E, 0074AC55
    host.invalidate_child_chain();    // 0074AC5C..0074AC72
}

Vec3 get_ramp_point_0074bca0(const Vec3& point_1214, const Vec3& point_1220, bool alternate) {
    return alternate ? point_1214 : point_1220;
}

// 0074BBD0-0074BC04: COMISS 80.0 vs class+A0h (JBE), then FCOMIP speed vs 3.0 (JBE).
bool small_and_moving_0074bbd0(float class_length_a0, float speed) {
    if (!(kShortLength_00ce5444 > class_length_a0)) return false;
    return speed > kMovingSpeed_00ce3854;
}

bool slot240_00749810(bool on_pad_1200, bool base_008128e0) {
    return on_pad_1200 ? false : base_008128e0;
}

// ---------------------------------------------------------------------------
// 4. Lua mission triggers
// ---------------------------------------------------------------------------

float trigger_radius_squared(float radius) {
    return f32(static_cast<double>(radius) * radius);  // 0074FF4E / 0075014C
}

// 00750000-00750089.
bool proximity_check_00750000(const Vec3& watched, const Vec3& target, float radius_sq_5c) {
    const float dx = f32(static_cast<double>(watched.x) - target.x);  // 00750046
    const float dz = f32(static_cast<double>(watched.z) - target.z);  // 00750052
    // 0075005E..0075006C: (dx*dx + 0*0) + dz*dz, one FSTP float.
    const float d2 = f32((static_cast<double>(dx) * dx + 0.0) + static_cast<double>(dz) * dz);
    return radius_sq_5c > d2;  // 00750077 FCOMIP, JBE skips
}

// 00750230-00750420.
bool line_of_sight_check_00750230(LineOfSightTrigger& t, float dt, const Vec3& watched,
                                  const Vec3& target, std::uint32_t watched_ignore,
                                  std::uint32_t target_44, LineOfSightHost& host) {
    const float dx = f32(static_cast<double>(watched.x) - target.x);  // 007502CA
    const float dy = f32(static_cast<double>(watched.y) - target.y);  // 007502D6
    const float dz = f32(static_cast<double>(watched.z) - target.z);  // 007502E2
    // 007502F2..00750302: (dx*dx + dy*dy) + dz*dz.
    const float d2 = f32((static_cast<double>(dx) * dx + static_cast<double>(dy) * dy)
                         + static_cast<double>(dz) * dz);
    if (!(t.range_sq_48 > d2)) return false;                    // 0075030D, JBE
    t.timer_4c = f32(static_cast<double>(t.timer_4c) - dt);     // 0075031A, 00750329 FST
    if (!(0.0f > t.timer_4c)) return false;                     // 0075032E FCOMIP, JBE
    t.timer_4c = f32(static_cast<double>(t.timer_4c) + kLosRecheck_00d7a280);  // 00750336
    std::uint32_t hit = 0;
    if (!host.cast_0084b6b0(watched, target, watched_ignore, hit)) return false;  // 007503D9
    return hit == target_44;                                     // 007503E6
}

SquadOrdnanceCheck squad_ordnance_check_007504f0(bool squadron_present_30,
                                                 bool any_can_drop_007ed790) {
    if (!squadron_present_30) return SquadOrdnanceCheck::target_gone;  // 007504FA vtable[10h]
    return any_can_drop_007ed790 ? SquadOrdnanceCheck::wait : SquadOrdnanceCheck::fire;
}

}  // namespace gameplay_seg41_cc13
}  // namespace bsp
