// Segment 34 reference models (packet cc13-gpr34). Evidence and the adoption
// note are in include/bsp/gameplay_seg34_cc13.hpp and docs/CC13_GPR_34.md.
// Names are hypotheses. Not wired, not game-run.

#include "bsp/gameplay_seg34_cc13.hpp"

namespace bsp {
namespace gameplay_seg34_cc13 {

namespace {

// 00CF9058, a double holding float(9.81); the product with a float rounds to
// float at 24-bit precision.
constexpr float kGravity = 9.81f;
// 00D7A24C.
constexpr float kOne = 1.0f;
// 00CE3850 (float) and 00D7A370 (double), both 5.0.
constexpr float kFadeHp = 5.0f;
// 00CE3800, the arming countdown a released dummy submarine gets.
constexpr float kSubmarineArming = 0.5f;
// 00D7A328 (double 4.0), the upward kick on release.
constexpr float kSubmarineReleaseKick = 4.0f;

}  // namespace

bool dummy_target_vehicle_tick_advance_00700660(DummyTargetVehicleState& s,
                                                const DummyTargetVehicleClassFields& cls,
                                                float dt,
                                                const DummyTargetVehicleHost& host) {
    if (host.base_tick_advance) host.base_tick_advance(dt);  // 0070066C
    // 0070067D-007006BA: COMISS 1.0, open; JBE skips at 1.0 or above (and on
    // NaN). The sum is stored, reloaded and compared with FCOMIP/JB, so a NaN
    // sum is kept.
    if (kOne > s.open_state_730) {
        const float open = dt / cls.open_time_138 + s.open_state_730;  // 00700698, 0070069E
        s.open_state_730 = open;
        if (open >= kOne) s.open_state_730 = kOne;
    }
    // 007006C2-007006D8: vy -= dt * 9.81 before the drag.
    s.velocity_734.y = s.velocity_734.y - dt * kGravity;
    // 007006DE-0070070C: k = dt * open * SlowingFactor, clamped to 1 (FCOMIP/JBE
    // keeps k at or below 1 and on NaN), then s = 1 - k.
    float k = dt * s.open_state_730 * cls.slowing_factor_13c;
    if (k > kOne) k = kOne;
    const float keep = kOne - k;
    s.velocity_734.x = keep * s.velocity_734.x;  // 0070071D-00700727
    s.velocity_734.y = s.velocity_734.y * keep;  // 0070072D-00700735
    s.velocity_734.z = keep * s.velocity_734.z;  // 0070073B-00700741
    // 00700747: COMISS 0.0, [E+A8h]; JBE skips at zero or above.
    return 0.0f > s.hp_a8;
}

void dummy_target_vehicle_commit_pose_00700f70(const DummyTargetVehicleState& s, float dt,
                                               const DummyTargetVehiclePoseHost& host) {
    host.restore_step_matrix();         // 00700F86
    host.force_up_row();                // 00700F8B-00700FAF
    host.orthonormalize_from_row_y();   // 00700FB4
    // 00700FB9-00700FE5: the three products are stored as floats first.
    const float dx = s.velocity_734.x * dt;
    const float dy = s.velocity_734.y * dt;
    const float dz = s.velocity_734.z * dt;
    Vec3 t = host.read_translation();
    t.x = t.x + dx;  // 00700FE9-00700FF3
    t.y = t.y + dy;  // 00700FF9-00701003
    t.z = t.z + dz;  // 00701009-00701013
    host.write_translation(t);
    host.invalidate_pose();             // 00701019-00701048
}

void dummy_target_vehicle_update_00700770(const DummyTargetVehicleState& s, float dt,
                                          const DummyTargetVehicleUpdateHost& host) {
    if (host.base_update_timers) host.base_update_timers(dt);  // 0070077B
    // 00700780-007007C2: only while the chute is still opening.
    if (kOne > s.open_state_730) {
        const float t = host.chute_animation_length() * s.open_state_730;
        host.set_chute_animation_time(t);
    }
    // 007007C5-0070082D: COMISS 5.0, hp; below 5 HP the body fades.
    if (kFadeHp > s.hp_a8) {
        float x = (kFadeHp - s.hp_a8) / kFadeHp;  // FSUBRP then FDIVP, one store
        if (0.0f > x) x = 0.0f;                   // FCOMIP/JBE keeps x at 0 or above and on NaN
        host.set_visibility_factor(kOne - x);
    }
}

DummyTargetBulletStep dummy_target_bullet_fixed_step_00701460(DummyTargetBulletState& s,
                                                             const DummyTargetBulletClassFields& cls,
                                                             float dt,
                                                             const DummyTargetBulletHost& host) {
    DummyTargetBulletStep r;
    // 00701483-007014A6: the owner chain is asked only when the launch time is
    // still negative.
    if (0.0f > s.launch_time_480 && !host.attached()) {
        s.launch_time_480 = host.now_f876a4;
        r.stamped_launch = true;
    }
    host.reset_round_animation();       // 007014AE-007014C6
    host.force_up_row();                // 007014C8-007014F2
    host.orthonormalize_from_row_y();   // 007014F7
    host.bomb_tick(dt, s);              // 00701509
    // 0070150E-0070157A, in this order: network mode, owner chain, no vehicle
    // yet, now - launch > OpenAfterTime (FCOMIP/JBE), -OpenAtFallingSpeed > vy.
    if (host.network_client) return r;
    if (host.attached()) return r;
    if (s.spawned_vehicle_49c) return r;
    const float since = host.now_f876a4 - s.launch_time_480;
    if (!(since > cls.open_after_time_dc)) return r;
    if (!(-cls.open_at_falling_speed_d8 > s.velocity_318.y)) return r;
    // 00701580-00701617: Race, Party (inverted), DummyObjectID, OwnerPlayer 8,
    // Vel3D; then 0070161C-00701683 the create and the observer.
    DummyTargetSpawnRequest q;
    q.race = s.race_58;
    q.party = s.party_54 == 0 ? 1 : 0;  // 007015B2 SETZ
    q.dummy_object_id = s.dummy_object_id_484;
    q.owner_player = 8;
    q.vel3d = s.velocity_318;
    host.spawn_vehicle(q);
    s.spawned_vehicle_49c = true;
    r.spawned = true;
    return r;
}

DummySubmarineBulletStep dummy_submarine_fixed_step_006ffa40(
    DummySubmarineBulletState& s, const DummySubmarineBulletClassFields& cls, float dt,
    const DummySubmarineBulletHost& host) {
    DummySubmarineBulletStep r;
    if (!host.attached()) {  // 006FFA6B
        // 006FFA74-006FFADD: COMISS 0.0, launch; JBE skips at zero or above.
        if (0.0f > s.launch_time_480) {
            s.launch_time_480 = host.now_f876a4;
            s.arming_45c = kSubmarineArming;
            s.velocity_318.y = s.velocity_318.y + kSubmarineReleaseKick;  // FADD qword, FSTP float
            r.stamped_launch = true;
        }
    } else if (!s.launcher_4b4) {  // 006FFAE5
        host.bind_launcher(s);  // 006FFAEE-006FFB5D
    }
    host.bomb_tick(dt, s);  // 006FFB6F
    // 006FFB74-006FFBC8: network mode, no unit yet, owner chain, then
    // now - launch > OpenAfterTime (FCOMIP/JBE).
    if (host.network_client) return r;
    if (s.spawned_unit_49c) return r;
    if (host.attached()) return r;
    const float since = host.now_f876a4 - s.launch_time_480;
    if (!(since > cls.open_after_time_d8)) return r;
    // 006FFBCE-006FFD10: the property bag, in image order.
    DummySubmarineSpawnRequest q;
    q.type_id = cls.submarine_type_id_54;
    q.type = cls.submarine_type_70;
    q.skill = s.skill_4b8;
    q.race = s.race_58;
    q.party = s.party_54;
    q.owner_player = s.owner_player_180;
    q.vel3d = s.velocity_318;
    q.start_speed = host.length(s.velocity_318);  // 006FFC92
    q.invincible = 0.0f;
    q.has_auto_attack_target = s.has_attack_target_47c;  // 006FFCBD-006FFCDB
    q.auto_attack_target = s.attack_target_word_174;
    q.dummy_object_id = s.dummy_object_id_484;
    q.target_dive = 1;
    q.fire_stance = 2;
    // 006FFD74-006FFDAD: only a launcher that answers vtable[+5Ch](5).
    q.notify_launcher = s.launcher_4b4 && s.launcher_is_unit;
    host.spawn_submarine(q);  // 006FFD29 004F05F0, 006FFD6F 006FF7C0
    s.spawned_unit_49c = true;
    r.spawned = true;
    return r;
}

}  // namespace gameplay_seg34_cc13
}  // namespace bsp
