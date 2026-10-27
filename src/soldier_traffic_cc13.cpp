// Soldier / traffic behaviour 0048A010-0048C230 and the SoldierGlobals loader
// 0084CB70. Packet cc13-soldier1; docs/CC13_SOLDIER_TRAFFIC.md. Read from the
// installed executable's bytes (Capstone over the disk image, local/sold_all.txt)
// and the Ghidra pseudocode; the listing decides every comparison below.

#include "bsp/soldier_traffic_cc13.hpp"

#include <cmath>
#include <iterator>

namespace bsp {
namespace soldier_traffic_cc13 {

namespace {

constexpr float kDefaultRadius_00ce38b8 = 10.0f;
constexpr double kSqrtCutoff_00ce3820 = 1e-10;       // qword
constexpr float kFakeEffectDieRadius_00ce3918 = 8.0f;
constexpr float kDyingTimer_00ce3930 = 20.0f;
constexpr double kHoldFloor_00ce6648 = 0.2;           // qword
constexpr float kOne_00d7a24c = 1.0f;
constexpr float kZero_00d7a218 = 0.0f;
constexpr float kHoldLow_00ce3854 = 3.0f;
constexpr float kHoldHigh_00ce5380 = 15.0f;
constexpr float kGoneDieLow_00ce3918 = 8.0f;
constexpr float kGoneDieHigh_00ce6640 = 14.0f;
constexpr double kFadeSeconds_00d7a2b0 = 3.0;         // qword
constexpr float kLeavingTimer_00d7a260 = -1.0f;
constexpr double kPathEndInset_00ce6638 = 0.01;       // qword
constexpr float kIdleWindow_00ce3850 = 5.0f;
constexpr float kDyingFadeStart_00ce6630 = 6.0f;
constexpr double kDyingFadeOffset_00d7a210 = 1.0;     // qword
constexpr double kDyingFadeSpan_00ce6628 = 6.0;       // qword
constexpr double kLeavingFadeOffset_00d7a308 = 2.0;   // qword
constexpr double kLeavingFadeSpan_00ce3dc0 = 10.0;    // qword
constexpr float kLeavingHide_00ce3800 = 0.5f;
constexpr float kWithdrawLow_00ce38b8 = 10.0f;
constexpr float kWithdrawHigh_00ce3930 = 20.0f;
constexpr float kShipLeftFade_00ce6650 = 0.98f;
constexpr float kShipLeftLow_00ce3854 = 3.0f;
constexpr float kShipLeftHigh_00ce38b8 = 10.0f;
constexpr float kNegZero_00d7a208 = -0.0f;
constexpr float kHalfPi_00ce3c64 = 1.5707963705062866f;
constexpr float kNegHalfPi_00ce3ccc = -1.5707963705062866f;
constexpr float kPi_00d7a264 = 3.1415927410125732f;
constexpr double kVariantX_00ce3dc8 = 0.30000001192092896;   // qword
constexpr double kVariant1Y_00ce6620 = 0.3700000047683716;   // qword
constexpr double kVariant1Z_00ce6618 = 0.15000000596046448;  // qword
constexpr double kVariant2YZ_00ce3d10 = 0.20000000298023224; // qword
constexpr double kIslandFloor_00ce6658 = -1000.0;            // qword
constexpr double kLaneOne_00d7a210 = 1.0;                    // qword

float f32(double v) noexcept { return static_cast<float>(v); }

// Index draw: NextU32 % count. The image divides by the vector size with no
// zero test (DIV), so an empty vector faults there; this code skips the draw
// and the animation instead.
bool pick_index(SoldierTrafficRandom& rng, SoldierRngSite site, std::uint32_t count,
                std::uint32_t& index) {
    const std::uint32_t r = rng.next_u32_00bd2fc0(site);
    if (count == 0) {
        return false;
    }
    index = r % count;
    return true;
}

}  // namespace

// ---------------------------------------------------------------------------
// 0084CB70
// ---------------------------------------------------------------------------
float paratrooper_gradient_radians_0084cd50(double degrees) noexcept {
    return f32(degrees * 3.1415927410125732 / 180.0);
}

void load_soldier_globals_0084cb70(SoldierGlobals& g, SoldierGlobalsLuaHost& lua) {
    // 0084CB8E CMP byte [ESI+1Ch],0 / JNZ: loaded once; 0084CBA2 sets it first.
    if (g.loaded) {
        return;
    }
    g.loaded = true;
    lua.open_state_00b6a020(0x41);                                   // 0084CBAA
    lua.run_script_00b69d40("Scripts\\global\\luaMW_init.lua");      // 00CE7CBC
    lua.run_script_00b69d40("Scripts\\datatables\\SoldierGlobals.lua");  // 00D0BD24
    g.shadow_clip_distance = f32(lua.soldier_globals_number_00b66270("ShadowClipDistance"));
    g.fire_efx_clip_distance = f32(lua.soldier_globals_number_00b66270("FireEfxClipDistance"));
    g.clip_distance = f32(lua.soldier_globals_number_00b66270("ClipDistance"));
    g.max_paratrooper_gradient = paratrooper_gradient_radians_0084cd50(
        lua.soldier_globals_number_00b66270("MaxParatrooperGradient"));
    g.parachute_open_altitude_min =
        f32(lua.soldier_globals_number_00b66270("ParachuteOpenAltitudeMin"));
    g.parachute_open_altitude_max =
        f32(lua.soldier_globals_number_00b66270("ParachuteOpenAltitudeMax"));
    lua.close_state_00b669a0();
}

// ---------------------------------------------------------------------------
// 0048A010
// ---------------------------------------------------------------------------
float entity_clip_radius_0048a02e(const EntityClipInputs& in) noexcept {
    // 0048A02E..0048A053: [entity+178h] and [[+178h]+160h]+0Ch both non-null ->
    // vslot +48h, MOVSS [EAX+0Ch]; otherwise [00CE38B8].
    return in.model_radius_present ? in.model_radius : kDefaultRadius_00ce38b8;
}

EntityClipQuery entity_clip_query_0048a010(const EntityClipInputs& in) noexcept {
    EntityClipQuery q;
    const float r = entity_clip_radius_0048a02e(in);
    // 0048A060..0048A0FB: max = p + r, min = p - r, each rounded to float.
    q.box_max = {f32(static_cast<double>(in.position.x) + r),
                 f32(static_cast<double>(in.position.y) + r),
                 f32(static_cast<double>(in.position.z) + r)};
    q.box_min = {f32(static_cast<double>(in.position.x) - r),
                 f32(static_cast<double>(in.position.y) - r),
                 f32(static_cast<double>(in.position.z) - r)};
    // 0048A147..0048A189: s = float(sin(float(fov))); (length / r) * s in x87,
    // against ClipDistance + entity+17Ch; JBE fails when (clip + bias) <= product
    // or the compare is unordered. The main thread's x87 runs at 24-bit
    // precision, so each step rounds to float.
    const float fov = in.fov_aspect_product;
    const float s = f32(std::sin(static_cast<double>(fov)));
    const float product = f32(static_cast<double>(f32(static_cast<double>(in.length) / r)) * s);
    const float limit = f32(static_cast<double>(in.clip_distance) + in.clip_bias);
    q.distance_passes = limit > product;
    return q;
}

bool entity_clip_result_0048a010(const EntityClipInputs& in, const EntityClipQuery& q,
                                 bool box_in_view, EntityClipAction& action) noexcept {
    action = EntityClipAction::None;
    if (!in.camera_present) {
        return in.visible_flag;  // 0048A024 MOV AL,[EBX+5Ch]
    }
    if (q.distance_passes && box_in_view) {
        if (!in.visible_flag) {
            action = EntityClipAction::Show_vslot11c;  // 0048A1A7
        }
        return true;
    }
    if (in.visible_flag) {
        action = EntityClipAction::Hide_vslot118;  // 0048A1C2
    }
    return false;
}

bool entity_clip_0048a010(const Vec3& position, EntityClipHost& host) {
    if (!host.camera_present_1ed4()) {
        return host.visible_flag_5c();  // 0048A024
    }
    EntityClipInputs in;
    in.camera_present = true;
    in.model_radius_present = host.model_radius_vslot48(in.model_radius);  // 0048A02E..0048A04C
    in.position = position;
    // 0048A0FF..0048A110: the camera pose refresh when [camera+C8h] == 0.
    if (!host.camera_pose_valid_c8()) {
        host.refresh_camera_pose_00414db0();
    }
    in.camera_position = host.camera_position_fc();
    // 0048A115..0048A143: delta = position - camera, each component stored as float.
    const Vec3 delta{f32(static_cast<double>(position.x) - in.camera_position.x),
                     f32(static_cast<double>(position.y) - in.camera_position.y),
                     f32(static_cast<double>(position.z) - in.camera_position.z)};
    in.fov_aspect_product = host.fov_aspect_product_00b6fc30();  // 0048A147
    in.clip_distance = host.clip_distance_00485520();            // 0048A162, +0Ch read at 0048A17B
    in.length = host.length_0042b2f0(delta);                     // 0048A16D
    in.clip_bias = host.clip_bias_17c();
    in.visible_flag = host.visible_flag_5c();
    const EntityClipQuery q = entity_clip_query_0048a010(in);
    // 0048A189 JBE: 004B48E0 (ECX = game, &{min, max}) is asked only when the
    // distance test passes.
    const bool in_view = q.distance_passes && host.box_in_view_004b48e0(q.box_min, q.box_max);
    EntityClipAction action = EntityClipAction::None;
    const bool result = entity_clip_result_0048a010(in, q, in_view, action);
    if (action == EntityClipAction::Show_vslot11c) {
        host.show_vslot11c();
    } else if (action == EntityClipAction::Hide_vslot118) {
        host.hide_vslot118();
    }
    return result;
}

// ---------------------------------------------------------------------------
// 0048A1E0
// ---------------------------------------------------------------------------
MuzzleEffectPlan plan_muzzle_effect_0048a1e0(const MuzzleEffectInputs& in) noexcept {
    MuzzleEffectPlan p;
    if (!in.class_fire_point_valid) {
        return p;  // 0048A209
    }
    // 0048A210..0048A231: an existing effect that 00865FE0 does not report ready
    // ends the call.
    if (in.effect_present && !in.effect_ready) {
        return p;
    }
    // 0048A237..0048A251: an existing effect with +9 set is released (00484620).
    p.release_previous = in.effect_present && in.effect_flag9;
    if (!in.camera_present) {
        return p;  // 0048A264
    }
    // 0048A2DD..0048A315: (length * float(sin(float(fov)))) > FireEfxClipDistance
    // returns (JA); equal spawns, and so does an unordered compare (JA not
    // taken), as here. The product rounds to float (24-bit x87 precision).
    const float s = f32(std::sin(static_cast<double>(in.fov_aspect_product)));
    const float product = f32(static_cast<double>(in.distance) * s);
    if (product > in.fire_efx_clip_distance) {
        return p;
    }
    p.spawn = true;
    if (in.is_kind_3a) {
        // 0048A345: angles (0, -pi/2, 0); translation by variant; then Euler
        // (pi, 0, 0) through 0047C210.
        p.euler = {0.0f, kNegHalfPi_00ce3ccc, 0.0f};
        const Vec3& f = in.fire_point;
        if (in.variant == 1) {
            p.translation = {f32(f.x + kVariantX_00ce3dc8), f32(f.y + kVariant1Y_00ce6620),
                             f32(f.z + kVariant1Z_00ce6618)};
        } else if (in.variant == 2) {
            p.translation = {f32(f.x + kVariantX_00ce3dc8), f32(f.y - kVariant2YZ_00ce3d10),
                             f32(kVariant2YZ_00ce3d10 + f.z)};
        } else {
            p.translation = f;
        }
        p.post_rotate_y_pi = true;
        (void)kPi_00d7a264;
    } else {
        // 0048A4C4: angles (0, pi/2, 0), translation the fire point; 0048A574
        // marks the new effect +9.
        p.euler = {0.0f, kHalfPi_00ce3c64, 0.0f};
        p.translation = in.fire_point;
        p.set_effect_flag9 = true;
    }
    return p;
}

void muzzle_effect_0048a1e0(int variant, MuzzleEffectHost& host) {
    MuzzleEffectInputs in;
    in.class_fire_point_valid = host.class_fire_point_valid_24();
    if (!in.class_fire_point_valid) {
        return;  // 0048A20A
    }
    in.effect_present = host.effect_present_180();
    if (in.effect_present) {
        in.effect_ready = host.effect_finished_00865fe0();  // 0048A22A
        if (!in.effect_ready) {
            return;  // 0048A231
        }
        in.effect_flag9 = host.effect_flag9();  // 0048A247
        if (in.effect_flag9) {
            host.release_effect_00484620();     // 0048A251
        }
    }
    in.camera_present = host.camera_present_1ed4();
    if (!in.camera_present) {
        return;  // 0048A264
    }
    // 0048A26A..0048A285: camera pose, then entity pose (00414DB0 when +C8h == 0).
    if (!host.camera_pose_valid_c8()) {
        host.refresh_camera_pose_00414db0();
    }
    if (!host.entity_pose_valid_c8()) {
        host.refresh_entity_pose_00414db0();
    }
    const Vec3 e = host.entity_position_fc();
    const Vec3 c = host.camera_position_fc();
    const Vec3 delta{f32(static_cast<double>(e.x) - c.x), f32(static_cast<double>(e.y) - c.y),
                     f32(static_cast<double>(e.z) - c.z)};    // 0048A28A..0048A2C1
    in.fov_aspect_product = host.fov_aspect_product_00b6fc30();       // 0048A2DD
    in.fire_efx_clip_distance = host.fire_efx_clip_distance_00485520();  // 0048A2F8
    in.distance = host.length_0042b2f0(delta);                        // 0048A303
    // The distance test runs before vslot +5Ch is asked (0048A315 JA).
    const float s = f32(std::sin(static_cast<double>(in.fov_aspect_product)));
    if (f32(static_cast<double>(in.distance) * s) > in.fire_efx_clip_distance) {
        return;
    }
    in.is_kind_3a = host.is_kind_3a_vslot5c();  // 0048A324
    in.variant = variant;
    in.fire_point = host.class_fire_point_18();
    const MuzzleEffectPlan p = plan_muzzle_effect_0048a1e0(in);
    host.create_effect_008687c0(p.euler, p.translation, p.post_rotate_y_pi);
    if (p.set_effect_flag9) {
        host.set_new_effect_flag9();  // 0048A564..0048A574
    }
}

bool unreferenced_fire_roll_0048aec0(SoldierTrafficRandom& rng, float fire_chance, float dt) {
    const float r = rng.uniform_00bd2f10(SoldierRngSite::UnreferencedFireRoll_0048aed0, 0.0f, 1.0f);
    // 0048AEDB FLD [type+18h] / FMUL dt rounds to float (24-bit x87); 0048AEE4
    // FCOMIP r against it and 0048AEE8 JAE skips, so r < product or an
    // unordered compare calls 0048A1E0(0).
    const float product = f32(static_cast<double>(fire_chance) * dt);
    return !(r >= product);
}

void fire_roll_0048aec0(TroopMember& m, float dt, SoldierTrafficRandom& rng,
                        TroopMemberHost& host) {
    // [ESP+4] is the member payload: +0 the type, +4 the entity (ECX for 0048A1E0).
    if (unreferenced_fire_roll_0048aec0(rng, m.type->fire_chance, dt)) {
        host.muzzle_effect_0048a1e0(m, 0);  // 0048AEEF
    }
}

// ---------------------------------------------------------------------------
// 0048A650
// ---------------------------------------------------------------------------
TrafficViewGateResult traffic_view_gate_0048a650(const TrafficViewGateInputs& in) noexcept {
    TrafficViewGateResult r;
    r.side = in.side;
    if (!in.box_in_view) {
        return r;  // 0048A667
    }
    if (in.anchor_present) {
        if (in.anchor_side != in.side) {
            r.side = in.anchor_side;  // 0048A682
            r.remap_00487470 = true;
            return r;                 // 0048A68D false
        }
        r.tick = in.side < 2;         // 0048A691 CMP ECX,2 / JL
        return r;
    }
    // 0048A69A..0048A6C1: the local player's slot decides the side; remap; true.
    r.side = in.local_player_slot_28_zero ? 1 : 0;
    r.remap_00487470 = true;
    r.tick = true;
    return r;
}

bool traffic_view_gate_0048a650(TrafficViewGateHost& host) {
    TrafficViewGateInputs in;
    in.box_in_view = host.box_in_view_004b48e0();  // 0048A660
    if (in.box_in_view) {
        in.anchor_present = host.anchor_present_168();
        in.side = host.side_16c();
        if (in.anchor_present) {
            in.anchor_side = host.anchor_side_54();
        } else {
            in.local_player_slot_28_zero = host.local_player_slot_28_zero();
        }
    }
    const TrafficViewGateResult r = traffic_view_gate_0048a650(in);
    if (r.remap_00487470) {
        host.set_side_16c(r.side);  // 0048A682 / 0048A6B6, before the call
        host.remap_00487470();      // 0048A688 / 0048A6BC
    }
    return r.tick;
}

// ---------------------------------------------------------------------------
// 0048A5A0
// ---------------------------------------------------------------------------
void clear_traffic_members_0048a5a0(std::list<void*>& members, TrafficMemberHost& host) {
    if (members.empty()) {
        return;  // 0048A5A0 CMP [ECX+74h],0
    }
    for (void*& e : members) {
        host.release_and_kill(e);  // vslot +114h(0), 00926D90(2)
        e = nullptr;               // 0048A5FC
    }
    members.clear();
}

// ---------------------------------------------------------------------------
// 0048A6D0
// ---------------------------------------------------------------------------
void tick_fake_effects_0048a6d0(LandingTroopState& troop, float dt, FakeEffectHost& host,
                                SoldierTrafficRandom& rng) {
    for (auto& list : troop.lists) {
        for (TroopMember& m : list) {
            m.fake_effect_timer = f32(static_cast<double>(m.fake_effect_timer) - dt);
            if (!(m.fake_effect_timer < 0.0f)) {
                continue;
            }
            const SoldierTypeView& type = *m.type;
            if (type.fake_effect_count == 0 || m.state == TroopMemberState::Waiting) {
                continue;
            }
            // 0048A790..0048A7D7: weighted pick over FakeEffects[i].Weight.
            float sum = 0.0f;
            for (std::uint32_t i = 0; i < type.fake_effect_count; ++i) {
                sum = f32(static_cast<double>(type.fake_effects[i].weight) + sum);
            }
            float r = rng.uniform_00bd2f10(SoldierRngSite::FakeEffectPick_0048a7d7, 0.0f, sum);
            std::uint32_t index = 0;
            for (std::uint32_t i = 0; i < type.fake_effect_count; ++i) {
                r = f32(static_cast<double>(r) - type.fake_effects[i].weight);
                if (r <= 0.0f) {
                    break;
                }
                ++index;
            }
            // The image indexes past the end when no weight brings r to 0
            // (checked-iterator fault); clamp instead.
            if (index >= type.fake_effect_count) {
                index = type.fake_effect_count - 1;
            }
            const FakeEffectRow& fx = type.fake_effects[index];
            // 0048A86C..0048A880: U(MinTime +8h, MaxTime +4h).
            m.fake_effect_timer =
                rng.uniform_00bd2f10(SoldierRngSite::FakeEffectTimer_0048a880, fx.min_time,
                                     fx.max_time);
            // 0048A895..0048A95F: the member pose refresh runs before the camera
            // test at 0048A9A5.
            const Vec3 p = host.member_world_position(m);
            if (!host.camera_present()) {
                continue;
            }
            const Vec3 c = host.camera_world_position();
            const float dx = f32(static_cast<double>(p.x) - c.x);
            const float dy = f32(static_cast<double>(p.y) - c.y);
            const float dz = f32(static_cast<double>(p.z) - c.z);
            // 0048AAAD..0048AAEE: s = dz^2 + (dy^2 + dx^2), each step rounded to
            // float (24-bit x87); sqrt (00BF7030) when s > 1e-10.
            const float s = f32(static_cast<double>(f32(static_cast<double>(dz) * dz)) +
                                f32(static_cast<double>(f32(static_cast<double>(dy) * dy)) +
                                    f32(static_cast<double>(dx) * dx)));
            const float dist = static_cast<double>(s) > kSqrtCutoff_00ce3820
                                   ? f32(std::sqrt(static_cast<double>(s)))
                                   : 0.0f;
            const float sn = f32(std::sin(static_cast<double>(host.fov_aspect_product_00b6fc30())));
            // 0048AB2E FCOMPI / JBE: spawn when FireEfx > float(dist * s).
            if (!(host.fire_efx_clip_distance() > f32(static_cast<double>(dist) * sn))) {
                continue;
            }
            // 0048ABC5..0048AC47: two draws U(-r, r); the first scales the identity's
            // row 2 (z), the second row 0 (x).
            const float radius = fx.radius;
            const float low = f32(static_cast<double>(kNegZero_00d7a208) - radius);
            const float oz =
                rng.uniform_00bd2f10(SoldierRngSite::FakeEffectOffsetZ_0048ac00, low, radius);
            const float ox =
                rng.uniform_00bd2f10(SoldierRngSite::FakeEffectOffsetX_0048ac47, low, radius);
            host.spawn_fake_effect_008687c0(m, index, ox, oz);
            // 0048AD54 COMISS: radius < 8.0 arms the die roll.
            if (!(radius < kFakeEffectDieRadius_00ce3918)) {
                continue;
            }
            const float roll =
                rng.uniform_00bd2f10(SoldierRngSite::FakeEffectDieRoll_0048ad8b, 0.0f, 1.0f);
            // 0048AD90..0048ADA9: (dt / [path]+28h) * Speed * DieChance > roll,
            // each step rounded to float (24-bit x87).
            const float chance = f32(
                static_cast<double>(f32(static_cast<double>(
                    f32(static_cast<double>(dt) / m.path_length_scale)) * troop.view.speed)) *
                troop.view.die_chance);
            if (!(chance > roll)) {
                continue;
            }
            m.state = TroopMemberState::Dying;  // 0048ADB9
            m.timer = kDyingTimer_00ce3930;     // 0048ADD2
            std::uint32_t die = 0;
            if (pick_index(rng, SoldierRngSite::FakeEffectDieAnim_0048ae02, type.anim_die_count,
                           die)) {
                host.play_die_anim_004b0f80(m, die);  // 0048AE43 (.., 0, 0)
            }
        }
    }
}

// ---------------------------------------------------------------------------
// 0048AF00
// ---------------------------------------------------------------------------
void spawn_missing_members_0048af00(LandingTroopState& troop, MemberSpawnHost& host) {
    for (auto& list : troop.lists) {
        for (TroopMember& m : list) {
            if (m.entity != nullptr) {
                continue;  // 0048AF45
            }
            // 0048AF5A..0048AF8C: both arms call [[type+4]] vslot +8.
            m.entity = host.instantiate_class_vslot8(m);
            host.attach_to_pad_path_parent_vslot98(m);  // 0048B072
            host.set_owner_vslot114(m);                 // 0048B08D
            host.activate_vslota0(m);                   // 0048B0A3
            if (m.type->is_soldier) {
                // 0048B0C0..0048B0F0: AnimRun[0], random start, forced. The image
                // faults on an empty vector (checked iterator).
                if (m.type->anim_run_count != 0) {
                    host.play_anim_004b0f80(m, TroopAnimSet::Run, 0, true, true);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// 0048B120
// ---------------------------------------------------------------------------
void tick_troop_members_0048b120(LandingTroopState& troop, float dt, TroopMemberHost& host,
                                 SoldierTrafficRandom& rng) {
    const LandingTroopView& v = troop.view;
    // 0048B12C..0048B14A: lists 0 and 2 empty, sampled once on entry.
    const bool outer_lists_empty = troop.lists[0].empty() && troop.lists[2].empty();

    for (int li = 0; li < 3; ++li) {
        auto& list = troop.lists[static_cast<std::size_t>(li)];
        for (auto it = list.begin(); it != list.end();) {
            TroopMember& m = *it;
            const SoldierTypeView& type = *m.type;
            m.timer = f32(static_cast<double>(m.timer) - dt);  // 0048B1AB

            // 0048B1BE..0048B358: with no ship (+DCh == 0), a non-Defence member
            // outside Waiting/Dying/Leaving counts down +40h and dies below 1.0.
            if (m.state != TroopMemberState::Waiting && m.state != TroopMemberState::Leaving &&
                m.state != TroopMemberState::Dying && !troop.ship_attached && !type.defence) {
                if (!(kHoldFloor_00ce6648 < m.hold_timer)) {
                    m.hold_timer = rng.uniform_00bd2f10(SoldierRngSite::ShipGoneHold_0048b253,
                                                        kHoldLow_00ce3854, kHoldHigh_00ce5380);
                } else if (kOne_00d7a24c > m.hold_timer) {
                    m.state = TroopMemberState::Dying;  // 0048B283
                    m.timer = rng.uniform_00bd2f10(SoldierRngSite::ShipGoneDieTimer_0048b2ac,
                                                   kGoneDieLow_00ce3918, kGoneDieHigh_00ce6640);
                    if (type.is_soldier) {
                        std::uint32_t die = 0;
                        if (pick_index(rng, SoldierRngSite::ShipGoneDieAnim_0048b2fa,
                                       type.anim_die_count, die)) {
                            host.play_anim_004b0f80(m, TroopAnimSet::Die, die, false, false);
                        }
                    }
                } else {
                    m.hold_timer = f32(static_cast<double>(m.hold_timer) - dt);  // 0048B352
                }
            }

            // 0048B35B..0048B448: fade +44h while 0 < f < 1.
            if (kOne_00d7a24c > m.fade && m.fade > kZero_00d7a218) {
                m.fade = f32(static_cast<double>(m.fade) -
                             f32(static_cast<double>(dt) / kFadeSeconds_00d7a2b0));
                if (m.fade > kZero_00d7a218) {
                    host.set_visibility_00b6da70(m, m.fade, true);
                } else {
                    host.set_visibility_00b6da70(m, 0.0f, true);
                    m.state = TroopMemberState::Leaving;  // 0048B42F
                    m.timer = kLeavingTimer_00d7a260;
                }
            }

            bool erase = false;
            switch (m.state) {
            case TroopMemberState::Waiting:  // 0048B46A
                if (m.hold_timer > kZero_00d7a218) {
                    m.path_position = 0.0f;
                    m.hold_timer = f32(static_cast<double>(m.hold_timer) - dt);
                } else if (0.0f > m.hold_timer) {
                    m.state = TroopMemberState::Running;  // 0048B4CD
                    m.hold_timer = 0.0f;
                    if (type.is_soldier) {
                        std::uint32_t run = 0;
                        if (pick_index(rng, SoldierRngSite::OutingRunAnim_0048b52c,
                                       type.anim_run_count, run)) {
                            host.play_anim_004b0f80(m, TroopAnimSet::Run, run, true, false);
                        }
                    }
                } else {
                    // hold == 0.0 exactly: advance (0048B584).
                    m.path_position = f32(
                        static_cast<double>(f32(static_cast<double>(v.speed) * dt)) + m.path_position);
                }
                break;
            case TroopMemberState::Running:  // 0048B5A5
                m.path_position = f32(
                    static_cast<double>(f32(static_cast<double>(v.speed) * dt)) + m.path_position);
                if (m.path_position > m.path_end) {  // 0048B5DE FCOMPI / JBE
                    m.state = TroopMemberState::Arrived;
                    m.path_position = f32(static_cast<double>(m.path_end) - kPathEndInset_00ce6638);
                }
                break;
            case TroopMemberState::Firing:  // 0048BA76
                if (!host.entity_is_kind_3a(m)) {
                    const float roll = rng.uniform_00bd2f10(
                        SoldierRngSite::FiringMuzzleRoll_0048baaa, 0.0f, 1.0f);
                    // 0048BAB2..0048BABE: float product; JAE skips, unordered calls.
                    if (!(roll >= f32(static_cast<double>(type.fire_chance) * dt))) {
                        host.muzzle_effect_0048a1e0(m, 0);
                    }
                }
                if (!(0.0f < m.timer)) {
                    m.timer = rng.uniform_00bd2f10(SoldierRngSite::RunTimer_0048bb01,
                                                   v.run_min_time, v.run_max_time);
                    host.stop_entity_effect_00867b10(m);
                    // 0048BB57..0048BB7A: |end - position| < 5.0 -> idle at the end.
                    const float gap = f32(std::fabs(
                        static_cast<double>(f32(static_cast<double>(m.path_end) - m.path_position))));
                    if (kIdleWindow_00ce3850 > gap) {
                        if (type.anim_idle_count != 0) {
                            host.play_anim_004b0f80(m, TroopAnimSet::Idle, 0, true, false);
                        }
                        m.state = TroopMemberState::Idle;  // 0048B88E
                    } else {
                        std::uint32_t run = 0;
                        if (pick_index(rng, SoldierRngSite::ResumeRunAnim_0048bbf0,
                                       type.anim_run_count, run)) {
                            host.play_anim_004b0f80(m, TroopAnimSet::Run, run, true, false);
                        }
                        m.state = TroopMemberState::Running;  // 0048BC46
                    }
                }
                break;
            case TroopMemberState::Dying:  // 0048BC52
                if (kDyingFadeStart_00ce6630 > m.timer && m.timer > kOne_00d7a24c) {
                    host.set_visibility_00b6da70(
                        m,
                        f32(static_cast<double>(f32(static_cast<double>(m.timer) -
                                                    kDyingFadeOffset_00d7a210)) /
                            kDyingFadeSpan_00ce6628),
                        true);
                    host.set_entity_byte_184(m, true);
                } else if (kOne_00d7a24c > m.timer) {
                    host.set_visibility_00b6da70(m, 0.0f, false);
                }
                if (!(0.0f < m.timer)) {
                    if (!type.defence) {
                        host.release_and_kill_entity(m);  // 0048BDEE..0048BE0C
                        erase = true;
                    } else {
                        m.state = TroopMemberState::Waiting;  // 0048BD59
                        m.path_position = 0.0f;
                        m.hold_timer = rng.uniform_00bd2f10(
                            SoldierRngSite::DefenceRespawnDelay_0048bd9a, 0.0f, type.outing_delay);
                        host.set_visibility_00b6da70(m, 1.0f, false);
                        host.set_entity_byte_184(m, false);
                    }
                }
                break;
            case TroopMemberState::Arrived: {  // 0048B626
                const float t = f32(static_cast<double>(m.path_end) - kPathEndInset_00ce6638);
                host.place_at_path_end_0048b626(m, t, !type.is_soldier);
                m.state = TroopMemberState::Idle;  // 0048B898
                break;
            }
            case TroopMemberState::Idle:  // 0048B8A4
                if (!type.is_soldier) {
                    const float roll = rng.uniform_00bd2f10(
                        SoldierRngSite::VehicleMuzzleRoll_0048ba4d, 0.0f, 1.0f);
                    // 0048BA55..0048BA61: float product; JAE skips, unordered calls.
                    if (!(roll >= f32(static_cast<double>(type.fire_chance) * dt))) {
                        host.muzzle_effect_0048a1e0(m, 0);
                    }
                } else if (li == 1 && outer_lists_empty && !troop.ship_attached &&
                           troop.base_present && troop.base_landing_active != nullptr &&
                           !troop.base_landing_active(troop.base_ctx)) {
                    // 0048B8BB..0048B931: withdraw. 00428260 returns the payload,
                    // so +2Ch/+30h are the node's +34h/+38h.
                    m.state = TroopMemberState::Leaving;
                    m.timer = rng.uniform_00bd2f10(SoldierRngSite::WithdrawTimer_0048b91b,
                                                   kWithdrawLow_00ce38b8, kWithdrawHigh_00ce3930);
                } else if (!(0.0f < m.timer) && v.fire_max_time > 0.0f) {
                    m.state = TroopMemberState::Firing;  // 0048B96C
                    m.timer = rng.uniform_00bd2f10(SoldierRngSite::FireTimer_0048b98f,
                                                   v.fire_min_time, v.fire_max_time);
                    std::uint32_t fire = 0;
                    if (pick_index(rng, SoldierRngSite::FireAnim_0048b9bc, type.anim_fire_count,
                                   fire)) {
                        host.play_anim_004b0f80(m, TroopAnimSet::Fire, fire, true, false);
                        host.muzzle_effect_0048a1e0(m, static_cast<int>(fire) + 1);
                    }
                }
                break;
            case TroopMemberState::Leaving:  // 0048BE2E
                if (m.timer > kOne_00d7a24c) {
                    host.set_visibility_00b6da70(
                        m,
                        f32(static_cast<double>(f32(static_cast<double>(m.timer) -
                                                    kLeavingFadeOffset_00d7a308)) /
                            kLeavingFadeSpan_00ce3dc0),
                        true);
                    host.set_entity_byte_184(m, true);
                } else if (kLeavingHide_00ce3800 > m.timer) {
                    host.set_visibility_00b6da70(m, 0.0f, false);
                }
                if (!(0.0f < m.timer)) {
                    host.release_and_kill_entity(m);  // 0048BF06..0048BF24
                    erase = true;
                }
                break;
            default:
                break;  // state > 6: 0048B45D JA
            }

            if (erase) {
                it = list.erase(it);  // 00484F10, then re-enter at 0048B174
            } else {
                ++it;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// 0048BF80 / 0048C090 / 0048C190
// ---------------------------------------------------------------------------
void release_member_entity_0048bf80(LandingTroopState& troop, void* entity,
                                    float game_time_00f876a4, float& last_release_time_18,
                                    MemberPathHost& host) {
    if (entity != nullptr) {
        last_release_time_18 = game_time_00f876a4;  // 0048BF9D
        host.clear_owner_vslot114(entity);          // 0048BFAC
    }
    for (auto& list : troop.lists) {
        for (auto it = list.begin(); it != list.end(); ++it) {
            TroopMember& m = *it;
            if (m.entity != entity) {
                continue;
            }
            if (m.type->is_soldier && host.member_owns_path(m)) {
                host.delete_member_path(m);  // 0048C049
            }
            list.erase(it);  // 0048C058..0048C070: unlink, free, count - 1
            break;
        }
    }
}

void kill_all_members_0048c090(LandingTroopState& troop, MemberPathHost& host) {
    for (auto& list : troop.lists) {
        for (TroopMember& m : list) {
            if (m.entity == nullptr) {
                continue;
            }
            if (m.type->is_soldier && host.member_owns_path(m)) {
                host.delete_member_path(m);
            }
            host.clear_owner_vslot114(m.entity);
            host.kill_entity_00926d90(m);
            m.entity = nullptr;
        }
    }
}

void ship_left_0048c190(LandingTroopState& troop, SoldierTrafficRandom& rng) {
    troop.ship_attached = false;  // 0048C195
    for (auto& list : troop.lists) {
        for (TroopMember& m : list) {
            m.fade = kShipLeftFade_00ce6650;
            m.timer = rng.uniform_00bd2f10(SoldierRngSite::ShipLeftTimer_0048c1fd,
                                           kShipLeftLow_00ce3854, kShipLeftHigh_00ce38b8);
        }
    }
}

// ---------------------------------------------------------------------------
// 0048C230
// ---------------------------------------------------------------------------
LandingLaneResult rebuild_landing_lane_0048c230(const LandingLaneInputs& in,
                                                LandingLaneHost& host) {
    LandingLaneResult r;
    // 0048C253: lane+180h = 1 is the caller-visible latch; the host owns it.
    bool found = false;
    void* island = host.select_island_d8(in.landing_point, found);
    r.island_found = found;
    (void)kIslandFloor_00ce6658;  // the host's search applies height > -1000.0
    host.begin_new_path_copying_old(island);

    // 0048C3F7..0048C480: s1 = float(+184h - +18Ch); point = landing + forward * s1,
    // y replaced by the island height at (x, z).
    const float s1 = f32(static_cast<double>(in.start_184) - in.length_18c);
    Vec3 p1{f32(static_cast<double>(f32(static_cast<double>(s1) * in.ship_forward.x)) +
                in.landing_point.x),
            f32(static_cast<double>(f32(static_cast<double>(in.ship_forward.y) * s1)) +
                in.landing_point.y),
            f32(static_cast<double>(f32(static_cast<double>(s1) * in.ship_forward.z)) +
                in.landing_point.z)};
    p1.y = host.island_height_0087fa50(island, p1.x, p1.z);
    host.append_point_007ad330(p1);

    // 0048C485..0048C522: s2 = float((+190h - +184h) + 1.0).
    const float s2 = f32(static_cast<double>(in.end_190) - in.start_184 + kLaneOne_00d7a210);
    Vec3 p2{f32(static_cast<double>(f32(static_cast<double>(in.ship_forward.x) * s2)) +
                in.landing_point.x),
            f32(static_cast<double>(f32(static_cast<double>(in.ship_forward.y) * s2)) +
                in.landing_point.y),
            f32(static_cast<double>(f32(static_cast<double>(in.ship_forward.z) * s2)) +
                in.landing_point.z)};
    p2.y = host.island_height_0087fa50(island, p2.x, p2.z);
    host.append_point_007ad330(p2);

    // 0048C527..0048C558: +190h = length at the last point - 1.0; +28h += 2.
    const std::int32_t last = host.point_count() - 1;
    r.end_190 = f32(static_cast<double>(host.length_at_007b05e0(last)) - kLaneOne_00d7a210);
    r.last_index_28 = in.last_index_28 + 2;
    host.install_new_path();

    // 0048C55E..0048C5DC: box over points min(+24h,+28h)..max, inclusive.
    const std::int32_t lo = in.first_index_24 < r.last_index_28 ? in.first_index_24 : r.last_index_28;
    const std::int32_t hi = in.first_index_24 > r.last_index_28 ? in.first_index_24 : r.last_index_28;
    const Vec3 first = host.point_to_world_007af800(lo);
    r.aabb_min_19c = first;
    r.aabb_max_1a8 = first;
    for (std::int32_t i = lo + 1; i <= hi; ++i) {
        const Vec3 p = host.point_to_world_007af800(i);
        r.aabb_min_19c = {p.x < r.aabb_min_19c.x ? p.x : r.aabb_min_19c.x,
                          p.y < r.aabb_min_19c.y ? p.y : r.aabb_min_19c.y,
                          p.z < r.aabb_min_19c.z ? p.z : r.aabb_min_19c.z};
        r.aabb_max_1a8 = {p.x > r.aabb_max_1a8.x ? p.x : r.aabb_max_1a8.x,
                          p.y > r.aabb_max_1a8.y ? p.y : r.aabb_max_1a8.y,
                          p.z > r.aabb_max_1a8.z ? p.z : r.aabb_max_1a8.z};
    }
    // 0048C5DE..0048C679: e = float(((+190h - +184h) * +188h + 1.0) * +3Ch); each
    // axis grows by float(e + margin).
    const float e = f32(((static_cast<double>(r.end_190) - in.start_184) * in.scale_188 +
                         kLaneOne_00d7a210) *
                        in.width_3c);
    const float gx = f32(static_cast<double>(in.margin_138.x) + e);
    const float gy = f32(static_cast<double>(in.margin_138.y) + e);
    const float gz = f32(static_cast<double>(in.margin_138.z) + e);
    r.aabb_min_19c = {f32(static_cast<double>(r.aabb_min_19c.x) - gx),
                      f32(static_cast<double>(r.aabb_min_19c.y) - gy),
                      f32(static_cast<double>(r.aabb_min_19c.z) - gz)};
    r.aabb_max_1a8 = {f32(static_cast<double>(r.aabb_max_1a8.x) + gx),
                      f32(static_cast<double>(r.aabb_max_1a8.y) + gy),
                      f32(static_cast<double>(r.aabb_max_1a8.z) + gz)};
    host.clear_members_0048a5a0();  // 0048C67F
    return r;
}

}  // namespace soldier_traffic_cc13
}  // namespace bsp
