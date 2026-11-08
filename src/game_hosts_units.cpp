#include "bsp/game_hosts_units_internal.hpp"
#include "bsp/entity_mission_seg60_argon.hpp"

extern "C" {
#include <lua.h>   // goal 5 chain 668: 007CA790's thisTable store
}

#include "bsp/cc13_front3.hpp"   // goal 5 chain 650: 007C11E0's PlaneNotifyFlightStateHost
#include "bsp/path_movcam_seg46_argon.hpp"   // goal 5 chain 696: 007AC780's block
#include "bsp/flak_note_seg36_37_argon.hpp"
#include "bsp/pool_pool01_cc13.hpp"
#include "bsp/airfield_taxi.hpp"

namespace bsp::game {  // namespace

// Packet cc13-para2: 0084CB70 once per run. False when the datatable did not read.
bool GameUnitsHost::Impl::para_soldier_globals_0084cb70() {
    if (para_globals_tried) return para_globals.loaded;
    para_globals_tried = true;
    SoldierGlobalsTextHost text(lua);
    bsp::soldier_traffic_cc13::load_soldier_globals_0084cb70(para_globals, text);
    if (!text.read()) para_globals.loaded = false;
    log.notef("paratrooper soldier globals: read=%d gradient=%.6f open_altitude=%.1f..%.1f "
        "(0084CB70, packet cc13-para2)", text.read() ? 1 : 0,
        static_cast<double>(para_globals.max_paratrooper_gradient),
        static_cast<double>(para_globals.parachute_open_altitude_min),
        static_cast<double>(para_globals.parachute_open_altitude_max));
    return para_globals.loaded;
}

// Packet cc13-para2: the BombClass fields 007AC780 reads for a Paratrooper bullet
// row (bulletclasses.lua keys), cached by bullet id. step_scale +5Ch is the
// constructor default (bsp::level_bomb_paratrooper::kParatrooperStepScale). The
// SoldierClass / SoldierAnim handles stay 0: the soldier is not a host entity.
// Goal 5 chain 696: the fields are 007AC780's own, read once into the row's
// class descriptor by the gunnery host (GameGunneryHost::paratrooper_class,
// 006EA910's reader call), where they were read here key by key before. A
// row that is not a Paratrooper, or a call before the gunnery host exists,
// keeps the constructor's zeros (006EA720 leaves +D8h..+FCh as allocated).
const bsp::gameplay_seg46_cc13::ParatrooperClassFields&
GameUnitsHost::Impl::para_class_fields(int bullet_id) {
    for (const auto& row : para_class_rows)
        if (row.first == bullet_id) return row.second;
    bsp::gameplay_seg46_cc13::ParatrooperClassFields f;
    f.step_scale_5c = bsp::level_bomb_paratrooper::kParatrooperStepScale;
    if (gunnery == nullptr) {
        // Not cached: the class is read once the gunnery host (created with the
        // first unit batch) can build its descriptor.
        static bsp::gameplay_seg46_cc13::ParatrooperClassFields unavailable;
        unavailable = f;
        log.notef("paratrooper class: bullet=%d before the gunnery host exists; constructor "
            "zeros (007AC780 not yet read)", bullet_id);
        return unavailable;
    }
    const bsp::path_movcam_seg46_argon::ParatrooperClassResidue* const read =
        bullet_id >= 0 ? gunnery->paratrooper_class(bullet_id) : nullptr;
    if (read != nullptr) {
        f.capture_power_d8 = read->capture_power_d8;
        f.slow_factor_closed_dc = read->slow_factor_closed_dc;
        f.slow_factor_opened_e0 = read->slow_factor_opened_e0;
        f.open_duration_e4 = read->open_duration_e4;
        f.close_duration_e8 = read->close_duration_e8;
        f.capture_duration_f8 = read->capture_duration_f8;
        f.damage_fc = read->damage_fc;
    }
    para_class_rows.emplace_back(bullet_id, f);
    log.notef("paratrooper class: bullet=%d power=%.2f slow=%.2f/%.2f open=%.2f close=%.2f "
        "capture=%.2f damage=%.2f (007AC780 keys, packet cc13-para2)", bullet_id,
        static_cast<double>(f.capture_power_d8), static_cast<double>(f.slow_factor_closed_dc),
        static_cast<double>(f.slow_factor_opened_e0), static_cast<double>(f.open_duration_e4),
        static_cast<double>(f.close_duration_e8), static_cast<double>(f.capture_duration_f8),
        static_cast<double>(f.damage_fc));
    return para_class_rows.back().second;
}

// LABELLED (bsp/cc13_paratrooper_round.hpp): the chute animation lengths are the
// class OpenDuration / CloseDuration, and 1.0 for the fall.
bsp::cc13_paratrooper::ChuteLengths GameUnitsHost::Impl::para_chute_lengths(int bullet_id) {
    const auto& f = para_class_fields(bullet_id);
    bsp::cc13_paratrooper::ChuteLengths lengths;
    lengths.open = f.open_duration_e4;
    lengths.close = f.close_duration_e8;
    return lengths;
}

// One fixed step of the dropped rounds (callback list, waves 1 and 3, +DCh).
void GameUnitsHost::Impl::para_round_step(float step_seconds) {
    if constexpr (bsp::kParatrooperRoundBound) {
        if (para_world.live() == 0) return;
        ParaServices svc(*this);
        para_world.step(step_seconds, static_cast<float>(summary.simulated_seconds), para_globals,
                        bsp::kParatrooperSoldierAnimDrawBound, svc);
    } else {
        (void)step_seconds;
    }
}  // namespace

// cc13-kzpick (kKamikazeEnterPickCc13Bound): the pick only. hull_aim_world_point
// is the host's 009FADA0 body: it re-constructs the ref on a new target, picks
// on the dirty byte (approach_target_ref_pick_009fa260, clearing it) and then
// stores the world point. That store lands in hull_aim_ref.world_point_1c,
// which nothing reads before the next 009FADA0 rewrites it, and the point
// itself is discarded here; the lead tail of aim_point_009fada0 is not run.
// The sections are the ones aim_point_009fada0 passes.
void GameUnitsHost::Impl::hull_aim_pick_009fa260(GameUnitSlot& shooter,
                                                 const GameUnitSlot& target,
                                                 std::size_t target_plus_one) {
    bsp::ApproachTargetSections sections;
    const bsp::ApproachTargetSections* sections_in = nullptr;
    if (kApproachSectionPointsBound && shooter.hull_aim_ref.section_chance_64 > 0.0f
        && hull_aim_target_samples_hull(target)
        && target_plus_one != 0 && target_plus_one - 1 < slots.size()) {
        sections = approach_target_sections(target_plus_one - 1, target);
        sections_in = &sections;
    }
    float discard[3] = {target.motion.position[0], target.motion.position[1],
                        target.motion.position[2]};
    (void)hull_aim_world_point(shooter, target, target_plus_one, discard, nullptr,
                               sections_in, &log, gunnery.get(), index_of_slot(shooter));
}

// Packet cc9_approach_target_lead: 009FADA0's tail 009FAF05-009FAF7D.
bool GameUnitsHost::Impl::aim_point_009fada0(GameUnitSlot& shooter,
                                             const GameUnitSlot& target,
                                             std::size_t target_plus_one,
                                             float out[3], GameHostLog* log_out) {
    bsp::ApproachTargetSections sections;
    const bsp::ApproachTargetSections* sections_in = nullptr;
    // Only a positive sub+64h reads the records (00816659), so a class model
    // is opened only for a shooter whose dive row set one.
    if (kApproachSectionPointsBound && shooter.hull_aim_ref.section_chance_64 > 0.0f
        && hull_aim_target_samples_hull(target)
        && target_plus_one != 0 && target_plus_one - 1 < slots.size()) {
        sections = approach_target_sections(target_plus_one - 1, target);
        sections_in = &sections;
    }
    const bool hull = hull_aim_world_point(shooter, target, target_plus_one, out, log_out,
                                           sections_in, &log, gunnery.get(),
                                           index_of_slot(shooter));
    const float projtime = shooter.hull_aim_ref.lead_projtime;
    // 009FAF05-009FAF11: JBE past the tail unless projtime > 0.0. The nine
    // hull-sampling vtables carry 008120E0 at +48h; every other target class
    // (00954650 for planes) is left unled here.
    if (!(projtime > 0.0f) || !hull_aim_target_samples_hull(target)) return hull;
    if (target_plus_one == 0 || target_plus_one - 1 >= slots.size()) return hull;
    GameUnitSlot& ts = *slots[target_plus_one - 1];
    bsp::ShipPredictInputs pin;
    for (int i = 0; i < 3; ++i) pin.position[i] = ts.motion.position[i];
    // vtable[38h] = 0080E0F0 -> 0092D730 on +1018h.
    bsp::UnitBodyAxisSpeedInputs axis{};
    axis.velocity[0] = ts.motion.linear_velocity.x;
    axis.velocity[1] = ts.motion.linear_velocity.y;
    axis.velocity[2] = ts.motion.linear_velocity.z;
    for (int i = 0; i < 3; ++i) axis.axis[i] = ts.motion.pose_row2[i];
    pin.speed = bsp::unit_forward_speed_0092d730(axis);
    // vtable[34h] = 00812090: the body axis +94h..9Ch times that speed.
    // ON (bsp::kAimLeadBodyAxisCc13Bound, packet cc13-aimaxis): 008120A3-008120CB,
    // row.x * s, row.y * s, s * row.z, each one float product. OFF: the
    // SUBSTITUTION, labelled, the one neighbour_world_velocity makes: the hull's
    // heading direction, level.
    const std::array<float, 3> axis_velocity{axis.axis[0] * pin.speed,
        axis.axis[1] * pin.speed, pin.speed * axis.axis[2]};
    const double hd = static_cast<double>(pose_heading_radians(ts));
    const std::array<float, 3> level_velocity{static_cast<float>(std::sin(hd)) * pin.speed,
        0.0f, static_cast<float>(std::cos(hd)) * pin.speed};
    if constexpr (bsp::kAimLeadBodyAxisCc13Bound) {
        pin.velocity = axis_velocity;
    } else {
        pin.velocity = level_velocity;
    }
    // 008120F8: 00811890(unit, unit+984h), which is 00811940.
    UnitRudderBinding rudder(*this, ts);
    if constexpr (kUnitYawRateForwardSpeedBound) rudder.forward_speed = pin.speed;
    pin.yaw_rate = bsp::unit_current_yaw_rate_00811940(rudder);
    {
        // Census (cc9-nondet1, log only): the same tail with each velocity.
        auto lead_point = [&](const std::array<float, 3>& velocity) {
            bsp::ShipPredictInputs alt = pin;
            alt.velocity = velocity;
            bsp::ApproachTargetRefState alt_lead{};
            alt_lead.world_point_1c = {out[0], out[1], out[2]};
            bsp::approach_target_ref_lead_tail_009faf05(alt_lead,
                bsp::ship_predict_position_008120e0(alt, projtime), alt.position);
            return alt_lead.world_point_1c;
        };
        const auto on = lead_point(axis_velocity);
        const auto off = lead_point(level_velocity);
        const double dx = static_cast<double>(on[0]) - off[0];
        const double dy = static_cast<double>(on[1]) - off[1];
        const double dz = static_cast<double>(on[2]) - off[2];
        ++aim_lead_axis_calls_;
        if (std::sqrt(dx * dx + dy * dy + dz * dz) > 1e-3) ++aim_lead_axis_differs_;
    }
    const std::array<float, 3> predicted =
        bsp::ship_predict_position_008120e0(pin, projtime);
    bsp::ApproachTargetRefState lead{};
    lead.world_point_1c = {out[0], out[1], out[2]};
    bsp::approach_target_ref_lead_tail_009faf05(lead, predicted, pin.position);
    if ((++aim_lead_applied_ % 400) == 1) {
        log.notef("aim lead %s -> %s projtime=%.2f lead=(%.1f %.1f) yaw=%.4f v=%.2f",
                  shooter.row.name.c_str(), ts.row.name.c_str(),
                  static_cast<double>(projtime),
                  static_cast<double>(lead.world_point_1c[0] - out[0]),
                  static_cast<double>(lead.world_point_1c[2] - out[2]),
                  static_cast<double>(pin.yaw_rate), static_cast<double>(pin.speed));
    }
    for (int i = 0; i < 3; ++i) out[i] = lead.world_point_1c[i];
    return hull;
}

// Packet cc13-aimfreeze3 (include/bsp/approach_aim_freeze_cc13.hpp).
const float* GameUnitsHost::Impl::aim_freeze_point(GameUnitSlot& shooter,
                                                   std::size_t target_plus_one,
                                                   std::uint8_t task) {
    const bsp::ApproachTargetRefState& st = shooter.hull_aim_ref;
    if (!st.frozen || shooter.aim_freeze_task != task) return nullptr;
    if (target_plus_one != 0 && target_plus_one != shooter.hull_aim_target_plus_one)
        return nullptr;
    ++aim_freeze_reads_[task & 7];
    return st.world_point_1c.data();
}

bool GameUnitsHost::Impl::aim_point_freeze_009fada0(GameUnitSlot& shooter,
                                                    const GameUnitSlot* target,
                                                    std::size_t target_plus_one,
                                                    float out[3], std::uint8_t task,
                                                    GameHostLog* log_out) {
    bsp::ApproachTargetRefState& st = shooter.hull_aim_ref;
    const bool bound = target != nullptr && target_plus_one != 0
        && shooter.hull_aim_target_plus_one == target_plus_one;
    // sub+18h clear: 009FADA0 does nothing more and sub+1Ch stays as it is.
    if (const float* fp = aim_freeze_point(shooter, target != nullptr ? target_plus_one : 0,
                                           task)) {
        for (int i = 0; i < 3; ++i) out[i] = fp[i];
        return true;
    }
    if (target == nullptr) return false;
    if (bound && !st.frozen) {
        const bool marked_5d = target->state != nullptr && target->state->simulate != 0;
        const bool destroyed_5e = target->scene_destroyed_005e != 0;
        if (destroyed_5e) {
            // 009FAF90 (observer slot +4h), LABELLED: the drain's notify reached the ref
            // before any tick saw +5Dh. 009FAFA6..: sub+1Ch = M x sub+1Ch, the last point
            // with its lead transformed again; 009FAFF4/009FAFF7 clear sub+18h/sub+14h,
            // sub+40h and sub+41h are cleared too.
            std::array<float, 3> w{};
            bsp::transform_point_004142e0(shooter.aim_freeze_last_point, target->world, w);
            st.world_point_1c = w;
            st.tracking = false;
            st.has_target_40 = false;
            st.dirty_41 = false;
            st.frozen = true;
            shooter.aim_freeze_task = task;
            ++aim_freeze_doubles_[task & 7];
            for (int i = 0; i < 3; ++i) out[i] = st.world_point_1c[i];
            return true;
        }
        if (marked_5d) {
            // 009FADAE..009FAE18: one last sample, M(+CCh) x sub+28h, with no lead, then
            // sub+14h / sub+18h / sub+41h cleared. A target whose slot +100h is 0042D810
            // keeps its origin here, as hull_aim_world_point leaves it (LABELLED: the
            // image transforms the bias-only offset for that class too).
            if (hull_aim_target_samples_hull(*target) && kHullAimOffsetEnabled) {
                bsp::approach_target_ref_freeze_009fadae(st, target->world);
            } else {
                st.world_point_1c = {target->motion.position[0], target->motion.position[1],
                                     target->motion.position[2]};
                st.tracking = false;
                st.dirty_41 = false;
                st.frozen = true;
            }
            shooter.aim_freeze_task = task;
            ++aim_freeze_heads_[task & 7];
            for (int i = 0; i < 3; ++i) out[i] = st.world_point_1c[i];
            return true;
        }
    }
    (void)aim_point_009fada0(shooter, *target, target_plus_one, out, log_out);
    shooter.aim_freeze_last_point = {out[0], out[1], out[2]};
    return true;
}

bool GameUnitsHost::Impl::avoid_in_dive(const GameUnitSlot& u) {
    using S = bsp::DiveBombState;
    const S s = u.dive_bomb_state;
    return s == S::kAimDive || s == S::kAimGlide || s == S::kFlyAbove || s == S::kTurnDown;
}

// The water surface stands in for the two heights this host lacks: the
// avoid-zone layer at squadron+34Ch (0041BC20) and, subtracted from the
// altitude, the height above ground at unit+9B4h. SUBSTITUTION, labelled.
float GameUnitsHost::Impl::avoid_surface_height(float x, float z) {
    // Packet cc9_avoid_zone_registry: 0041BC20 on the registry's squadron+34Ch
    // layer. Both heights are computed in both builds for the census; the
    // switch picks which one the plane reads. Every plane is sampled here,
    // while the image's 007CE87F skips unit+9B4h's update for a plane whose
    // unit+9D4h is null (its old value stands). Labelled.
    OceanFieldBinding sea(*this);
    const float sea_h = bsp::ocean_water_height_0078cf20(x, z, sea);
    const GameAvoidZoneRegistry& reg = GameAvoidZoneRegistry::instance();
    const bsp::TerrainGridLayerRecord* layer = reg.layer(reg.squadron_layer_34c());
    const float layer_h = layer != nullptr ? avoid_zone_sample_0041bc20(*layer, x, z) : 0.0f;
    if (az_samples == 0) {
        az_sea_min = az_sea_max = sea_h;
    } else {
        if (sea_h < az_sea_min) az_sea_min = sea_h;
        if (sea_h > az_sea_max) az_sea_max = sea_h;
    }
    ++az_samples;
    if (layer_h != 0.0f) ++az_samples_layer_nonzero;
    if (layer_h > az_layer_max) az_layer_max = layer_h;
    // pc24-lint: allow census counter only; 0041BC20 itself is narrowed in game_hosts_avoid_zones.cpp (pc24w1-sweep3-3)
    const double d = std::fabs(static_cast<double>(layer_h) - sea_h);
    // pc24-lint: allow census counter only
    az_diff_abs_sum += d;
    if (d > az_diff_abs_max) az_diff_abs_max = d;
    if constexpr (kAvoidZoneLayerSampleBound) {
        done("AvoidZoneLayer::sample_0041bc20", 0x0041bc20u);
        return layer_h;
    } else {
        return sea_h;
    }
}

// Goal 5 chain 654. 007EC5A0 BSP_PlaneFxComponent_LoadFromClass, called from
// BSP_Plane_ReadPropertyBag at 007D6327 (ECX = unit+C98h, plane, active 0): the
// engine list (007ECB41-007ECC85), one engine per class+140h NumEngines (007D204D:
// 00B66270, then the CRT float-to-int, a truncation), a base engine (00D08640)
// when class+144h JetEngines (007D208B, 00B662F0 with a false default) is set,
// else a rotor (00D0865C), each initialised through its slot 0 at 007ECC85.
// LABELLED, not modelled: the component's effect slots, levels and class loops
// (007EC5D9-007EC9CC) and the rotor's Init 007EBF10. Their effects are scene-node
// visibility and spin scalars no host code reads, and their draws (the speed
// spread 007ECA23 / 007ECA41 / 007ECADD and 007EBF10's 007EC10F, all stream 1)
// are the ones docs/CC13_RNG_STREAM_CENSUS.md section 11 reclassifies as
// PRESENTATION; they are not taken, so the shared stream is as before. A rotor
// therefore keeps operator new's state (007ECBC4-007ECBD6). Its Reset (007E1C60,
// reached from Restart and from 007B97F0's ResetAll, neither of which this host
// runs) would draw 007E1CCF on stream 1; that draw has no host site, so it throws.
// The base engine's Init 007E1BA0 reads none of its arguments and runs.
namespace {
namespace s48 = bsp::gameplay_seg48_cc13;

// The two engine objects 007EC5A0 pushes: the base engine (0Ch bytes, vtable
// 00D08640) and the rotor (40h bytes, vtable 00D0865C), over the slot bodies in
// src/gameplay_seg48_cc13.cpp. operator new's stores 007ECBC4-007ECBD6: +4h = 0,
// +9h = 0, +8h = 0. The interface's burning_8 / stopped_9 are the object's own
// +8h / +9h; stopped_9 is copied from the body state after each slot call, and
// burning_8 is written only by 007E23E0 through the interface.
class PlaneBaseEngine final : public s48::PlaneEngineSlots {
public:
    // Slot 0, 007E1BA0 (the call at 007ECC85; its four arguments are unread).
    void init() noexcept { s48::engine_base_init_007e1ba0(state_); sync(); }
    void restart(bool running) override {
        s48::engine_base_restart_007ddeb0(state_, running);
        sync();
    }
    void reset() override { s48::engine_base_reset_007e1c00(state_); sync(); }
    void stop_engine() override { s48::engine_base_mark_stopped_007ddf00(state_); sync(); }
    void step_rpm(float target, float dt) override { s48::engine_step_rpm_noop_007e23b0(target, dt); }
    void apply_spin(float dt) override { s48::engine_apply_spin_noop_007e23c0(dt); }
    void blur_blend() override { s48::engine_blur_blend_noop_007e23d0(); }

private:
    void sync() noexcept { stopped_9 = state_.stopped_9; }
    s48::PlaneEngineBase state_;
};

// The rotor. The node visibility its slots return is dropped: this host has no
// rotor scene nodes ("rotor<i>", "rotor_still<i>", "rotor_still_dam<i>").
class PlaneRotorEngine final : public s48::PlaneEngineSlots {
public:
    PlaneRotorEngine(const s48::RotorTuning& tuning, s48::Rng rng)
        : tuning_(tuning), rng_(std::move(rng)) {}
    void restart(bool running) override {
        static_cast<void>(s48::rotor_restart_007ddec0(rotor_, running, tuning_, rng_));
        sync();
    }
    void reset() override {
        static_cast<void>(s48::rotor_reset_007e1c60(rotor_, rng_));
        sync();
    }
    void stop_engine() override {
        static_cast<void>(s48::rotor_stop_engine_007ddf10(rotor_));
        sync();
    }
    void step_rpm(float target, float dt) override {
        s48::rotor_step_rpm_007ec180(rotor_, target, dt, tuning_);
    }
    void apply_spin(float dt) override { static_cast<void>(s48::rotor_apply_spin_007ec1f0(rotor_, dt)); }
    void blur_blend() override { static_cast<void>(s48::rotor_blur_blend_007ec3b0(rotor_, tuning_)); }

private:
    void sync() noexcept { stopped_9 = rotor_.stopped_9; }
    s48::PlaneRotor rotor_;
    s48::RotorTuning tuning_;
    s48::Rng rng_;
};
}  // namespace

void GameUnitsHost::Impl::build_plane_engine_list(GameUnitSlot& plane) {
    plane.plane_engines_c98.clear();
    const int type = plane.row.type_id;
    const std::int32_t count = static_cast<std::int32_t>(
        lua.read_vehicle_class_number(type, "NumEngines", 0.0f));
    plane.plane_num_engines_140 = count;
    const bool jet_engines = lua.read_vehicle_class_boolean(type, "JetEngines", false);
    s48::RotorTuning tuning;
    if (lua.plane_globals_loaded()) {
        const bsp::GameTuningBlock& g = lua.plane_globals();
        tuning.speed_base_160 = g.rotor_speed_base;
        tuning.speed_random_168 = g.rotor_speed_random;
        tuning.still_only_speed_16c = g.rotor_still_only_speed;
        tuning.blurred_only_speed_170 = g.rotor_blurred_only_speed;
        tuning.rotor_speed_change_17c = g.rotor_rotor_speed_change;
    }
    s48::Rng no_reset_draw;
    no_reset_draw.uniform = [](int, float, float) -> float {
        throw std::logic_error("007E1C60: the rotor Reset's 007E1CCF draw has no host site");
    };
    no_reset_draw.next_u32 = [](int) -> std::uint32_t {
        throw std::logic_error("007E1C60: the rotor Reset draws no NextU32");
    };
    for (std::int32_t i = 0; i < count; ++i) {
        if (jet_engines) {
            auto engine = std::make_unique<PlaneBaseEngine>();
            engine->init();   // 007ECC85 -> 007E1BA0
            plane.plane_engines_c98.push_back(std::move(engine));
        } else {
            plane.plane_engines_c98.push_back(
                std::make_unique<PlaneRotorEngine>(tuning, no_reset_draw));
        }
    }
    ++plane_engine_lists_built;
    plane_engines_built += plane.plane_engines_c98.size();
}

// 007E2010 / 007E11D0 for the +30h copy: the +7Ch clock advanced by dt and, when it
// reaches +88h = 3.0, members at or beyond R = 1080 dropped and every world entity
// answering IsKindOf(6) or (0Fh) within R and not yet listed appended. "In the world
// list" is a slot with a live state object not marked destroyed (wrecks stay listed,
// as the refresh has no death test). Labelled, as for kPlaneFinderBound.
bool GameUnitsHost::Impl::nb_in_world(const GameUnitSlot& o) {
    return o.state != nullptr && o.state->active != 0 && o.scene_destroyed_005e == 0 &&
           o.scene_pending_destroy_0060 == 0;
}

void GameUnitsHost::Impl::nb_near_30_tick_007e2010(GameUnitSlot& u, float dt) {
    const bsp::PlaneNeighbourRadii radii = bsp::plane_neighbour_radii_007e11d0(3.0f);
    const float* p0 = u.world.data() + 12;                          // unit+FCh..+104h
    auto dist2 = [&](const GameUnitSlot& o) {
        const double dx = static_cast<double>(o.world[12]) - p0[0];
        const double dy = static_cast<double>(o.world[13]) - p0[1];
        const double dz = static_cast<double>(o.world[14]) - p0[2];
        return dx * dx + dy * dy + dz * dz;
    };
    nb_seed_clocks(u);
    u.nb_clock_30 += dt;
    if (u.nb_clock_30 >= 3.0f) {
        const double lim = static_cast<double>(radii.near_any) * radii.near_any;
        auto& v = u.nb_near_30;
        for (std::size_t k = 0; k < v.size();) {
            if (v[k] >= slots.size() || !nb_in_world(*slots[v[k]]) || dist2(*slots[v[k]]) >= lim) {
                v[k] = v.back();
                v.pop_back();
            } else {
                ++k;
            }
        }
        for (std::size_t j = 0; j < slots.size(); ++j) {
            const GameUnitSlot& o = *slots[j];
            if (&o == &u || !nb_in_world(o)) continue;
            if (!bsp::unit_is_kind_of(o.class_id, 0x06) && !bsp::unit_is_kind_of(o.class_id, 0x0F)) {
                continue;
            }
            if (!(dist2(o) < lim)) continue;
            bool listed = false;
            for (const std::size_t e : v) listed = listed || e == j;
            if (!listed) v.push_back(j);
        }
        u.nb_clock_30 -= 3.0f;
    }
}

// cc13-tranche2 S22 (include/bsp/near_field_probe_cached_list_cc13.hpp): the unit+C50h
// object is a tick element (007E2010 is its vtable 00D06B88 slot +0Ch), so its +30h
// list advances with the fixed step. 007D61C0..007D621F builds it when [game+1FE4h] is
// 0 or 1 and unit+54h != [[game]+5FCh]+908h; kSceneDefaultDummyAi stands in for +908h,
// as at 009A184A. LABELLED: run at the start of the units' fixed step, while the plane
// is in the world.
// CORRECTION (rev7 S22 NIT 1): not a step lead. 007E2010 runs in wave 2 as a plane
// sub-node, after the pose step; measuring before the host's pose move is the existing
// kPlanePoseFirstBound substitution.
// LABELLED split (rev7 S22 NIT 3): the image's one +7Ch clock refreshes +30h, +50h and
// +70h together, and 007E2010's early return clears all three. Under ON only the +30h
// copy (nb_clock_30) moves to the fixed step; the +70h copy (nb_clock_70, with the +80h
// clock, friendly_in_line_clock_007e2010) and the +50h list stay on the think, so their
// phases can drift from +30h's.
bool GameUnitsHost::Impl::nb_has_object_c50(const GameUnitSlot& u) const {
    return u.row.party != bsp::kSceneDefaultDummyAi;
}

void GameUnitsHost::Impl::nb_near_30_fixed_step(float step) {
    for (const auto& slot : slots) {
        if (!slot) continue;
        GameUnitSlot& u = *slot;
        if (!bsp::unit_is_kind_of(u.class_id, 0x0F) || !nb_has_object_c50(u) || !nb_in_world(u)) {
            continue;
        }
        nb_near_30_tick_007e2010(u, step);
    }
}

// 0099B670 -> 007DF4F0 (__thiscall(neighbours = unit+C50h, pilot), RET 4,
// body 007DF4F0-007E073E), for one plane per think. The unit+C50h object is
// built for AI planes not of the local player's party (007D621F), which
// 009A17D0's own gate already requires.
void GameUnitsHost::Impl::vehicle_avoidance_007df4f0(GameUnitSlot& u, float dt) {
    namespace pc24 = bsp::cc13x87;   // kPc24PlaneTasksCc13Bound arms (pc24w1-sweep3-5)
    // 007E2010 / 007E11D0: the +30h list, refreshed when the +7Ch clock
    // reaches +88h = 3.0: members at or beyond R = 1080 are dropped, then
    // every world entity answering IsKindOf(6) or (0Fh) within R and not yet
    // listed is appended. The object's tick cadence stands in as the think
    // interval, and "in the world list" is a slot with a live state object
    // not marked destroyed (wrecks stay listed, as the refresh has no death
    // test). Labelled, as for kPlaneFinderBound.
    const float* p0 = u.world.data() + 12;                          // unit+FCh..+104h
    // cc13-tranche2 S22: under bsp::kNearFieldProbeCachedListCc13Bound the refresh runs
    // once per fixed step for every plane (nb_near_30_tick_007e2010), not here.
    if constexpr (!bsp::kNearFieldProbeCachedListCc13Bound) nb_near_30_tick_007e2010(u, dt);
    if (u.nb_near_30.empty()) return;                               // 007DF53B this+34h

    const bool free_flight = u.plane_control_mode_900 == 7;
    // 007DF5E0-007DF959: the own frame M. When speed > 10 ([00CE38B8]) and
    // |unit+B04h| / speed > 0.1 (double [00D7A3A0]) the image builds it from
    // normalize(velocity + unit+B04h); otherwise it is the inverse pose
    // unit+110h. unit+B04h has no literal writer in the image (a store census
    // over every FSTP/MOVSS/MOV/MOVUPS form finds only this reader, 007DF62D),
    // so it stays at its zeroed allocation and the inverse pose is the frame.
    // Confirmed, with the usual block-copy caveat.
    const AvoidBox ob = avoid_box(u);
    const float hx = static_cast<float>(
        1.2000000476837158 * ((-ob.mn[0] < ob.mx[0]) ? ob.mx[0] : -ob.mn[0]));   // double [00CEC160]
    const float hy = static_cast<float>(
        1.5 * ((-ob.mn[1] < ob.mx[1]) ? ob.mx[1] : -ob.mn[1]));                  // double [00CE3D78]
    const float own_r = avoid_radius_500(u);
    float v0[3];
    avoid_velocity(u, v0);
    const float own_speed = avoid_len(v0);                          // vtable[38h], labelled
    // desc+1ACh PitchSpd; a class without one keeps DEG(30), labelled.
    const float pitch_spd = u.plane_pitch_spd > 0.0f ? u.plane_pitch_spd : 0.5235988f;
    const float h9b4 = p0[1] - avoid_surface_height(p0[0], p0[2]);  // unit+9B4h, labelled
    const bool dive = avoid_in_dive(u);
    bool plane_hit = false, ship_hit = false;

    // 007DF96C: the list walk. pilot+260h (skipped at 007DF990) has no host
    // producer and 009A1995 clears it every tick, so nothing is skipped.
    const std::vector<std::size_t> list = u.nb_near_30;
    for (const std::size_t idx : list) {
        if (idx >= slots.size()) continue;
        const GameUnitSlot& o = *slots[idx];
        if (&o == &u) continue;
        const float* p1 = o.world.data() + 12;
        float v1[3];
        avoid_velocity(o, v1);
        if (bsp::unit_is_kind_of(o.class_id, 0x0F)) {
            // 007DF99C-007DFE85, an aircraft.
            float lp[3], lv[3];
            follow_to_body(u, p1, false, lp);                       // 004142E0
            const float rv[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
            follow_to_body(u, rv, true, lv);                        // 0042D0D0
            float dot;
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // pc24w1-sweep3-5: (x*x' + y*y') + z*z', 007DFA8C..007DFAA6, each op
                // rounded at PC24, FSTP float (007DFAA8).
                dot = static_cast<float>(pc24::x87_add24(
                    pc24::x87_add24(pc24::x87_mul24(lp[0], lv[0]), pc24::x87_mul24(lp[1], lv[1])),
                    pc24::x87_mul24(lp[2], lv[2])));
            } else {
            dot = static_cast<float>(static_cast<double>(lp[0]) * lv[0] +
                                     static_cast<double>(lp[1]) * lv[1] +
                                     static_cast<double>(lp[2]) * lv[2]);
            }
            if (!(dot < 0.0f)) continue;                            // 007DFAB6, closing only
            const float d = avoid_len(lp);
            const float rr = avoid_radius_500(o) + own_r;           // class+500h twice
            float s = avoid_len(lv);
            s = s < 2.0f ? 2.0f : (s > 80.0f ? 80.0f : s);         // 00415620 [00CE3958] [00CE5444]
            if (d > rr) {
                if (9.999999747378752e-05 > s) continue;            // double [00D7A268]
                if constexpr (kPc24PlaneTasksCc13Bound) {
                    // 007DFB50 FSUB, 007DFB52 FDIV, each rounded at PC24, then the
                    // 007DFB5C compare against the double 5.0.
                    if (pc24::x87_div24(pc24::x87_sub24(d, rr), s) > 5.0) continue;
                } else {
                if ((static_cast<double>(d) - rr) / s > 5.0) continue;   // double [00D7A370]
                }
            }
            float c[3] = {lp[0], lp[1], lp[2]};
            float dca = d, t = 0.0f;
            if (d > rr) {                                           // 007DFBA9
                if constexpr (kPc24PlaneTasksCc13Bound) {
                    // 007DFBB1 FMULP s * s, 007DFBB3 FDIVR, each rounded; FCHS exact.
                    t = static_cast<float>(-pc24::x87_div24(dot, pc24::x87_mul24(s, s)));
                } else {
                t = static_cast<float>(-(static_cast<double>(dot) / (static_cast<double>(s) * s)));
                }
                for (int i = 0; i < 3; ++i) c[i] = lp[i] + t * lv[i];
                dca = avoid_len(c);
            }
            float w = avoid_interp(0.8f, 1.25f, 1.6f, 0.0f, dca / rr) *   // [00CE74F8] [00CF29A8] [00D06BB4]
                      avoid_interp(0.5f, 1.6f, 5.0f, 0.0f, t);            // [00CE3800] [00CE3850]
            if (0.0f > lp[2]) w = static_cast<float>(w - 0.5);            // double [00D7A280]
            if (!(w > 0.0f)) continue;
            if (rr > dca) dca = rr;
            for (float& e : c) e /= dca;
            const float xlo = c[0] - w, xhi = c[0] + w, ylo = c[1] - w, yhi = c[1] + w;
            if (xlo > 0.0f || 0.0f > xhi || (!(0.0f < ylo) && !(yhi < 0.0f))) {
                ga_insert_0099b790(u, 0, xlo, xhi);                 // 007DFD75, pilot+4h
                ++u.va_bands;
                plane_hit = true;
            }
            if (ylo > 0.0f || 0.0f > yhi || (!(0.0f < xlo) && !(xhi < 0.0f))) {
                ga_insert_0099b790(u, 1, ylo, yhi);                 // 007DFDC4, pilot+CCh
                ++u.va_bands;
                plane_hit = true;
                if (!plane_rate_trace_name().empty() && u.row.name == plane_rate_trace_name()) {
                    // Packet cc13-ratetraceband: diagnostic, env-gated.
                    log.notef("plane rate band %s t=%.2f branch=aircraft_007dfdc4 other=%s "
                        "class=%d mode900=%d lp=(%.1f %.1f %.1f) d=%.1f rr=%.1f t_ca=%.2f "
                        "w=%.3f band=(%.4f %.4f)",
                        u.row.name.c_str(), static_cast<double>(summary.simulated_seconds),
                        o.row.name.c_str(), static_cast<int>(o.class_id), o.plane_control_mode_900,
                        static_cast<double>(lp[0]), static_cast<double>(lp[1]),
                        static_cast<double>(lp[2]), static_cast<double>(d),
                        static_cast<double>(rr), static_cast<double>(t),
                        static_cast<double>(w), static_cast<double>(ylo),
                        static_cast<double>(yhi));
                }
            }
            // 007DFDD3-007DFE7F: both bands reach into the central +/-0.3
            // ([00D06888]/[00CE69C8]), closing under 35 ([00CE4D90]), ahead,
            // within 2 s: pilot+25Ch = interp(3, 1, 15, -1, s), a plain store.
            if (xhi > -0.3f && 0.3f > xlo && yhi > -0.3f && 0.3f > ylo && 35.0f > s &&
                lp[2] > 0.0f && 2.0f > t) {
                u.av_throttle_25c = avoid_interp(3.0f, 1.0f, 15.0f, -1.0f, s);
                ++u.va_throttle_sets;
            }
            continue;
        }
        // 007DFE8A-007E070B, any other listed entity (the ships).
        const float dx[3] = {p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};
        const float dv[3] = {v0[0] - v1[0], v0[1] - v1[1], v0[2] - v1[2]};
        float clos;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // pc24w1-sweep3-5: (x*x' + y*y') + z*z', 007DFF35..007DFF58, each op rounded.
            clos = static_cast<float>(pc24::x87_add24(
                pc24::x87_add24(pc24::x87_mul24(dv[0], dx[0]), pc24::x87_mul24(dv[1], dx[1])),
                pc24::x87_mul24(dv[2], dx[2])));
        } else {
        clos = static_cast<float>(static_cast<double>(dv[0]) * dx[0] +
                                  static_cast<double>(dv[1]) * dx[1] +
                                  static_cast<double>(dv[2]) * dx[2]);
        }
        if (!(clos > 0.0f)) continue;                               // 007DFF68
        const AvoidBox b = avoid_box(o);
        double box_top;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 007DFF83 FSUB, 007DFF86 FADD 5.0, rounded at PC24 before the compare.
            box_top = pc24::x87_add24(b.mx[1] - b.mn[1], 5.0);
        } else {
            box_top = static_cast<double>(b.mx[1] - b.mn[1]) + 5.0;
        }
        if (static_cast<double>(h9b4) > box_top) {                  // 007DFF74-007DFF92
            continue;
        }
        // 00816410: the point where the ray from the ship's origin toward P0
        // leaves its box; |P1 - Q| is that ray's length.
        float q[3];
        follow_to_body(o, p0, false, q);
        const float bx = (-b.mn[0] < b.mx[0]) ? b.mx[0] : -b.mn[0];
        const float bz = (-b.mn[2] < b.mx[2]) ? b.mx[2] : -b.mn[2];
        const float ax = std::fabs(q[0]), ay = std::fabs(q[1]), az = std::fabs(q[2]);
        float kq = 1.0f;
        if (bx < ax && bx / ax <= 1.0f) kq = bx / ax;
        if (!(ay <= b.mx[1])) {
            const float ky = b.mx[1] / ay;
            if (ky <= kq) kq = ky;
        }
        if (bz < az && bz / az <= kq) kq = bz / az;
        const float surf = kq * avoid_len(q);
        float gap;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // pc24w1-sweep3-5: 007DFFB7 FADD 5.0 then 007DFFC6 FSUBP, each rounded.
            gap = static_cast<float>(pc24::x87_sub24(avoid_len(dx), pc24::x87_add24(own_r, 5.0)));
        } else {
        gap = static_cast<float>(static_cast<double>(avoid_len(dx)) -
                                 (static_cast<double>(own_r) + 5.0));   // 007DFFB7
        }
        gap = static_cast<float>(static_cast<double>(gap) - surf);        // 007E0010
        if (1.0f > gap) gap = 1.0f;                                   // 007E001C
        const float rel = avoid_len(dv);
        const float tc = static_cast<float>(static_cast<double>(gap) / (rel > 1.0f ? rel : 1.0f));
        double tc_limit;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            tc_limit = pc24::x87_div24(1.5, pitch_spd);   // 007E0080 FDIVR, rounded at PC24
        } else {
            tc_limit = 1.5 / pitch_spd;
        }
        if (static_cast<double>(tc) > tc_limit) continue;            // 007E0088 double [00CE3D78]
        float l[3];
        follow_to_body(u, p1, false, l);                              // 00414D10
        auto dead = [](float v, float h) {                            // 007E00B5-007E012E
            if (v > h) return v - h;
            if (-h > v) return v + h;
            return 0.0f;
        };
        l[0] = dead(l[0], hx);
        l[1] = dead(l[1], hy);
        const float* om = o.world.data();
        float rgt[3], upv[3], fwd[3], vo[3];
        follow_to_body(u, om + 0, true, rgt);                         // 0042D7E0 rows, 0042D0D0
        follow_to_body(u, om + 4, true, upv);
        follow_to_body(u, om + 8, true, fwd);
        follow_to_body(u, v1, true, vo);
        const float kk0 = static_cast<float>(static_cast<double>(tc) * 1.25);   // double [00CF87C0]
        const float kk = kk0 < 0.7f ? 0.7f : (kk0 > 2.0f ? 2.0f : kk0);          // [00CE3E18] [00CE3958]
        float cmax[2], cmin[2];
        for (int i = 0; i < 2; ++i) {
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // pc24w1-sweep3-5: the listing sums (b0 * right + b1 * up) + b2 * forward
                // (007E030B..007E0351 for the +0Ch box corner, 007E03F2..007E043E for
                // +00h), each term stored as float, then times kk.
                cmax[i] = kk * ((b.mx[0] * rgt[i] + b.mx[1] * upv[i]) + b.mx[2] * fwd[i]);
                cmin[i] = kk * ((b.mn[0] * rgt[i] + b.mn[1] * upv[i]) + b.mn[2] * fwd[i]);
            } else {
            cmax[i] = kk * (b.mx[2] * fwd[i] + b.mx[1] * upv[i] + b.mx[0] * rgt[i]);
            cmin[i] = kk * (b.mn[2] * fwd[i] + b.mn[1] * upv[i] + b.mn[0] * rgt[i]);
            }
        }
        float zz;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 007E0479 FDIVR 0.5 / PitchSpd, 007E047F FMUL gap, each rounded at PC24.
            zz = static_cast<float>(pc24::x87_mul24(pc24::x87_div24(0.5, pitch_spd), gap));
        } else {
        zz = static_cast<float>(0.5 / pitch_spd * gap);               // double [00D7A280]
        }
        if (u.plane_control_mode_900 == 6) {
            zz = static_cast<float>(zz / 1.399999976158142);          // double [00D045F0]
        }
        float xlo = (vo[0] + l[0] + cmax[0]) / zz, xhi = (l[0] + cmin[0]) / zz;
        if (xlo > xhi) std::swap(xlo, xhi);
        float ylo = (vo[1] + l[1] + cmax[1]) / zz, yhi = (l[1] + cmin[1]) / zz;
        if (ylo > yhi) std::swap(ylo, yhi);
        if (-1.1f > xlo) xlo = -1.1f;                                 // [00D06BB0]
        if (xhi > 1.1f) xhi = 1.1f;                                   // [00CE6448]
        if (-1.1f > ylo) ylo = -1.1f;
        if (yhi > 1.1f) yhi = 1.1f;
        if (u.plane_control_mode_900 == 6 && 0.0f > xlo && xhi > 0.0f) {   // 007E05A0
            if (xlo > -0.2f) xlo = -0.2f;                             // double [00D06BA8]
            if (0.2f > xhi) xhi = 0.2f;                               // double [00CE3D10]
            u.av_throttle_25c = avoid_interp(3.0f, 1.0f, 8.0f, -1.0f, own_speed);   // [00CE3918]
            ++u.va_throttle_sets;
        }
        if (xhi > -1.0f && 1.0f > xlo && 0.0f > ylo && yhi > 0.0f) {
            ga_insert_0099b790(u, 0, xlo, xhi);                       // 007E0693
            ++u.va_bands;
            ship_hit = true;
        }
        if (free_flight && yhi > -1.0f && 1.0f > ylo && 0.0f > xlo && xhi > 0.0f) {
            ga_insert_0099b790(u, 1, ylo, yhi);                       // 007E0706
            ++u.va_bands;
            ship_hit = true;
            if (!plane_rate_trace_name().empty() && u.row.name == plane_rate_trace_name()) {
                // Packet cc13-ratetraceband: diagnostic, env-gated.
                log.notef("plane rate band %s t=%.2f branch=ship_007e0706 other=%s "
                    "class=%d mode900=%d l=(%.1f %.1f %.1f) gap=%.1f tc=%.2f h9b4=%.1f "
                    "box_h=%.1f band=(%.4f %.4f)",
                    u.row.name.c_str(), static_cast<double>(summary.simulated_seconds),
                    o.row.name.c_str(), static_cast<int>(o.class_id), o.plane_control_mode_900,
                    static_cast<double>(l[0]), static_cast<double>(l[1]),
                    static_cast<double>(l[2]), static_cast<double>(gap),
                    static_cast<double>(tc), static_cast<double>(h9b4),
                    static_cast<double>(b.mx[1] - b.mn[1]), static_cast<double>(ylo),
                    static_cast<double>(yhi));
            }
        }
    }
    if (plane_hit) ++u.va_plane_ticks;
    if (ship_hit) ++u.va_ship_ticks;
    if ((plane_hit || ship_hit) && dive) ++u.va_dive_ticks;
}

// 009A1420..009A17A1 (0099F1C0's branch A, inside the body 0099F1C0-009A17CB),
// reached when (unit+72Ch)->vtable[38h] is false and unit+900h == 6: a live plane
// on the water. Packet cc9_terrain_segment_consumers, docs/SCENE_CONTENTS_HOSTS.md
// section 12. Evidence: disk listing 009A1420..009A17CA and the export's
// pseudocode (0099f1c0/decompiled.c lines 300-400).
// - Length L = 00419010(0, T+2F4h TakeOffMaxLength, 007C4810, T+2F8h
//   TakeOffMinLength, unit->vtable[38h]); 007C4810 (thiscall on pilot+2F8h, the
//   class) = T+28Ch * cls+184h. 007C4810 returns with a plain RET, so its two
//   stack floats are 00419010's arguments 4 and 5 (009A1454..009A147F).
// - p = (unit+FCh, 0.1 [00D7A2F0], unit+104h). pilot+3F0h picks the side and
//   flips: clear -> set, angle -30 deg [00CEC728]; set -> clear, +30 deg
//   [00CEC724]. q = p + L * 007BA2E0(00438AA0(vtable[50h], angle)), q.y = 0.1.
// - 00903BC0(p, q) clear: return (009A15C8). Blocked: pilot+3F0h = !pilot+3F0h,
//   then the fan idx = 3,-3,4,-4,..,12,-12,13 at idx/12.0*pi [00CE42D0]
//   [00CE3D28] off vtable[50h], length L, y offset L*0.0 [00D7A258]. The first
//   clear idx >= 0 inserts (-1.1 [00D06BB0], 0.9 [00CE3860]); a clear negative
//   idx or an exhausted fan inserts (-0.9 [00CED6C8], 1.1 [00CE6448]), both into
//   pilot+4h (set 0). Then pilot+25Ch = 00419010(1, 1, 8 [00CE3918], -1
//   [00D7A260], 007D99C0 forward speed).
// SUBSTITUTIONS, labelled: unit->vtable[38h] is the live velocity length (as in
// the free-flight path), vtable[50h] is the host's plane_heading_c6c, and the
// position is the host world row, with no 00414DB0 refresh.
void GameUnitsHost::Impl::terrain_avoidance_water_009a1420(GameUnitSlot& u) {
    if (!lua.plane_globals_loaded()) return;
    const bsp::GameTuningBlock& g = lua.plane_globals();
    float v_world[3];
    avoid_velocity(u, v_world);
    const float speed = avoid_len(v_world);                          // vtable[38h], labelled
    const float m28c = bsp::tuning_min_control_multiplier_007e41df(
        g.dynamics_spd_multipliers_control_range_min,
        g.dynamics_spd_multipliers_control_range_max,
        g.dynamics_spd_multipliers_stall_range_max,
        g.dynamics_spd_multipliers_level_flight);
    const float m = bsp::plane_min_control_speed_007c4810(m28c, u.plane_stall_spd);
    const float len = avoid_interp(0.0f, g.dynamics_water_take_off_max_length, m,
                                   g.dynamics_water_take_off_min_length, speed);  // 009A147F
    const float* pos = u.world.data() + 12;                          // unit+FCh..104h
    const float from[3] = {pos[0], 0.1f, pos[2]};                    // [00D7A2F0]
    const float heading = u.plane_heading_c6c;                       // vtable[50h], labelled
    float side_angle;
    if (u.tr_side_3f0) {                                             // 009A14DC
        side_angle = 0.5235988f;                                     // [00CEC724]
        u.tr_side_3f0 = false;
    } else {
        side_angle = -0.5235988f;                                    // [00CEC728]
        u.tr_side_3f0 = true;
    }
    // 007BA2E0: a = pi/2 - h (double, stored float), + 2 pi when negative;
    // (cos a, 0, sin a), each stored float. The loop inlines the same steps.
    auto direction = [](float h, float& dx, float& dz) {
        float a = static_cast<float>(1.5707963705062866 - static_cast<double>(h));   // [00CE3830]
        if (0.0f > a) a = static_cast<float>(static_cast<double>(a) + 6.2831854820251465);  // [00CE3828]
        dx = static_cast<float>(std::cos(static_cast<double>(a)));
        dz = static_cast<float>(std::sin(static_cast<double>(a)));
    };
    float dx = 0.0f, dz = 0.0f;
    direction(bsp::wrapped_angle_add_00438aa0(heading, side_angle), dx, dz);   // 009A150C, 009A1524
    float to[3] = {static_cast<float>(len * dx) + from[0], 0.1f,
                   static_cast<float>(len * dz) + from[2]};
    ++u.tr_water_probes;
    if (!world_segment_blocked_00903bc0(from, to)) return;           // 009A15C1, JE 009A17A1
    ++u.tr_water_blocked;
    u.tr_side_3f0 = !u.tr_side_3f0;                                  // 009A15CE..009A15DE
    const float y_offset = static_cast<float>(len * 0.0);            // [00D7A258] double 0.0
    int up = 3, down = -3, pass = 0;
    bool positive_exit = false;
    for (;;) {                                                       // 009A160D
        int idx;
        if (pass > 1) {
            ++up;
            --down;
            pass = 0;
            idx = up;
        } else {
            idx = pass == 0 ? up : down;
        }
        float off;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // pc24w1-sweep3-5: 009A1635 FDIV 12.0, 009A163B FMUL pi, each rounded at PC24.
            off = static_cast<float>(bsp::cc13x87::x87_mul24(
                bsp::cc13x87::x87_div24(static_cast<double>(idx), 12.0), 3.1415927410125732));
        } else {
        off = static_cast<float>(static_cast<double>(idx) / 12.0 * 3.1415927410125732);
        }
        direction(bsp::wrapped_angle_add_00438aa0(heading, off), dx, dz);   // 009A1657..009A1699
        to[0] = static_cast<float>(len * dx) + from[0];
        to[1] = y_offset + from[1];
        to[2] = static_cast<float>(len * dz) + from[2];
        ++u.tr_water_probes;
        if (!world_segment_blocked_00903bc0(from, to)) {             // 009A172E, JE 009A17A9
            positive_exit = 1 - 2 * pass > 0;                        // 009A17A9..009A17B5
            if (positive_exit) ++u.tr_water_fan_pos; else ++u.tr_water_fan_neg;
            break;
        }
        ++pass;
        if (!(down >= -12)) {                                        // 009A173A JGE
            ++u.tr_water_fan_exhausted;
            break;
        }
    }
    if (positive_exit) {
        ga_insert_0099b790(u, 0, -1.1f, 0.9f);                       // [00D06BB0] [00CE3860]
    } else {
        ga_insert_0099b790(u, 0, -0.9f, 1.1f);                       // [00CED6C8] [00CE6448]
    }
    ++u.tr_bands;
    const float* fr = u.world.data() + 8;                            // forward row
    const float fwd_speed = v_world[0] * fr[0] + v_world[1] * fr[1] + v_world[2] * fr[2];   // 007D99C0
    u.av_throttle_25c = avoid_interp(1.0f, 1.0f, 8.0f, -1.0f, fwd_speed);   // 009A1796, 009A179B
    ++u.tr_throttle_sets;
}

// 0099F1C0 (body 0099F1C0-009A17CB), __thiscall(pilot), for one plane per think.
void GameUnitsHost::Impl::terrain_avoidance_0099f1c0(GameUnitSlot& u, float /*dt*/) {
    if (u.plane_control_mode_900 != 7) {
        // 009A1420, branch A (unit+900h == 6, on the water): a +-30 degree
        // heading probe through 00903BC0 at y = 0.1. This host has no ground
        // model; with the water surface standing in for the ground both probe
        // ends are clear, and branch A returns at its first test (009A14xx),
        // before any band or +25Ch write. SUBSTITUTION, labelled.
        ++u.tr_water_ticks;
        if constexpr (kAvoidTerrainWaterProbeBound) {
            terrain_avoidance_water_009a1420(u);
        }
        return;
    }
    // 0099F1F3-0099F20D: the squadron (unit+9D4h) and its avoid-zone layer
    // ctl+34Ch. The layer's cell size layer+0Ch is the scene's first avoid-zone
    // layer's 100 m (docs/TORPEDO_RUN_IN_PATH.md), and its height is the water
    // surface. SUBSTITUTION, labelled.
    if (bsp::plane_squadron_registry().find_by_member_unit(u.process_index) == nullptr) return;
    float cell = 100.0f;                                             // min(layer+0Ch, 120.0)
    if constexpr (kAvoidZoneLayerSampleBound) {
        const GameAvoidZoneRegistry& reg = GameAvoidZoneRegistry::instance();
        if (const bsp::TerrainGridLayerRecord* layer = reg.layer(reg.squadron_layer_34c())) {
            cell = layer->cell_size < 120.0f ? layer->cell_size : 120.0f;
        }
    }

    const float pitch_spd = u.plane_pitch_spd > 0.0f ? u.plane_pitch_spd : 0.5235988f;
    float look_t = static_cast<float>(1.5 / pitch_spd);              // double [00CE3D78]
    if (3.0 > look_t) look_t = 3.0f;                                 // double [00D7A2B0]
    const float w_width = u.class_width_00a4;                        // cls+A4h
    float sa = static_cast<float>(w_width / 5.0);                    // double [00D7A370]
    bool inverted = false;
    float v_world[3];
    avoid_velocity(u, v_world);
    // Packet cc9_terrain_avoid_forward_speed (docs/SQUADRON_LAND_TASK.md 5da).
    // 0099F27E and 0099CC98 (inside 0099CAB0) both call unit vtable[204h], which
    // is 007B8E70 FLD [unit+B2Ch], the flight controller's ctl+7Ch. Its writer
    // 007D80C0 (called from the core law at 007DC81D after each step) stores
    // ctl+44h there, the BODY FORWARD speed, not |v| (that is ctl+6Ch, 007D807C,
    // vtable[38h]). Bound: dot(body z row, world velocity), the value the host's
    // free-flight step forms as ctl+44h. OFF keeps the |v| stand-in.
    float speed = avoid_len(v_world);                                // vtable[204h] stand-in, labelled
    if constexpr (kTerrainAvoidForwardSpeedBound) {
        const float* const fz = u.motion.pose_row2;
        speed = fz[0] * v_world[0] + fz[1] * v_world[1] + fz[2] * v_world[2];   // ctl+7Ch
    }
    float stall_range = 1.6f;                                        // tuning+230h StallRangeMax
    if (lua.plane_globals_loaded()) {
        stall_range = lua.plane_globals().dynamics_spd_multipliers_stall_range_max;
    }
    const float stall = u.plane_stall_spd > 0.0f ? u.plane_stall_spd : 17.5f;
    // 007C4830: plane_class_stall_range_speed_007c4830 over tuning+230h and the
    // class StallSpd +184h; OFF keeps the inline product.
    float v_stall;
    v_stall = bsp::cc13_seg47::plane_class_stall_range_speed_007c4830(stall_range, stall);
    const float v_look = speed > v_stall ? speed : v_stall;
    // unit+C7Ch: the elevation of the vector at unit+AC8h (007C1CA8), and
    // unit+AC8h is the world velocity vtable[34h] 007BBB70 returns
    // (docs/FIGHTER_GUN_LEAD.md). So the velocity's elevation is the term.
    float pitch = 0.0f;
    if constexpr (bsp::kUnitsHostDeviationsCc13Bound) {
        // cc13-unitsdev: the image reads unit+C7Ch, which 007C18B0/007C1900
        // writes only when |v| > 0.1 (otherwise it keeps its value). In the
        // image that write is at 007CEE9C, after the think (007CE865) and the
        // motion, so think sees the previous step's C7C. The host's
        // update_pose_derived_state_007c18b0 runs before pilot_think_and_commit in each
        // of its three arms (free flight, ground roll/deck, surface), from the
        // velocity the host holds at that point. In steady free flight that is
        // the velocity the previous step's motion left, so the values agree.
        // LABELLED, they differ when the velocity changes between the image's
        // write and the host's refresh: (a) 007CE040 runs the 007C0910 sweep
        // collision at 007CEF67, after the 007CEE9C write, so the image's C7C
        // predates a collision response the host sees; (b) on the ground roll
        // carrier_deck_carry_in rewrites plane_world_velocity under the
        // carrier's current matrix just before the refresh; (c) out-of-step
        // writers zero or replace the velocity (flight_state_notify,
        // plane_enter_state_two, airfield_site_tick,
        // carrier_elevator_send_empty_down, carrier_deck_capture), after which
        // the host keeps a C7C one step older than the image's.
        pitch = u.plane_velocity_elevation_c7c;
    } else {
    double horiz;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // pc24w1-sweep3-5: the C7Ch writer (007C1B85..007C1CA8) forms x*x + z*z at PC24,
        // stores it through float, floors it at the double 1e-10 and takes FSQRT, then
        // atan2(y, horizontal), stored as float.
        namespace pc24 = bsp::cc13x87;
        const float h2 = static_cast<float>(pc24::x87_add24(pc24::x87_mul24(v_world[0], v_world[0]),
                                                            pc24::x87_mul24(v_world[2], v_world[2])));
        horiz = static_cast<double>(h2) > 1e-10 ? static_cast<float>(pc24::x87_sqrt24(h2)) : 0.0f;
    } else {
    horiz = std::sqrt(static_cast<double>(v_world[0]) * v_world[0] +
                      static_cast<double>(v_world[2]) * v_world[2]);
    }
    pitch = avoid_len(v_world) > 1.0f
        ? static_cast<float>(std::atan2(static_cast<double>(v_world[1]), horiz))
        : u.plane_pitch_angle_c64;
    // DIAGNOSTIC (cc9-physics29): the ON arm would read C7Ch, which the writer in
    // update_pose_derived_state_007c18b0 now keeps in both modes (read only by this census
    // when OFF).
    ++uhd_pitch_calls;
    if (pitch != u.plane_velocity_elevation_c7c) {
        ++uhd_pitch_differs;
        if (!(avoid_len(v_world) > 1.0f)) ++uhd_pitch_lowspeed;
        const double dd = std::fabs(static_cast<double>(pitch)
            - static_cast<double>(u.plane_velocity_elevation_c7c));
        if (dd > uhd_pitch_maxdiff) uhd_pitch_maxdiff = dd;
    }
    }
    const float bank = u.plane_bank_angle_c68;
    float bank_f = std::fabs(bank);
    if (static_cast<double>(bank_f) > 1.5707963705062866) {          // double [00CE3830]
        bank_f = static_cast<float>(3.1415927410125732 - bank_f);   // double [00CE3D28]
        inverted = true;
        sa = static_cast<float>(sa * 8.0);                           // double [00CE3DB0]
    } else {
        sa = avoid_interp(0.1f, 1.0f, 1.2f, 8.0f, bank_f) * sa;      // [00D7A2F0] [00CE3814] [00CE3918]
    }
    const float sa_bank_trace = sa;                                  // DIAGNOSTIC
    if (pitch < 0.0f) sa = static_cast<float>(sa - w_width * pitch * 4.0);   // double [00D7A328]
    const float sa_pitch_trace = sa - sa_bank_trace;                 // DIAGNOSTIC
    const float* p_now = u.world.data() + 12;
    const float h9b4 = p_now[1] - avoid_surface_height(p_now[0], p_now[2]);   // unit+9B4h, labelled
    const float sa_h_trace = avoid_interp(80.0f, 0.0f, 460.0f, w_width + w_width, h9b4);   // DIAGNOSTIC
    sa = avoid_interp(80.0f, 0.0f, 460.0f, w_width + w_width, h9b4) + sa;     // [00CE5444] [00D1F404]
    const float* fr = u.world.data() + 8;                            // forward row
    const float fwd_speed = v_world[0] * fr[0] + v_world[1] * fr[1] + v_world[2] * fr[2];   // 007D99C0
    const float max_spd = u.plane_max_spd;                           // cls+188h
    sa = avoid_interp(static_cast<float>(max_spd * 1.4), 0.0f,       // double [00D045F0]
                      static_cast<float>(2.5 * max_spd),             // double [00CE3DE0]
                      static_cast<float>(w_width * 3.0), fwd_speed) + sa;
    const float sa_speed_trace = avoid_interp(static_cast<float>(max_spd * 1.4), 0.0f,   // DIAGNOSTIC
                      static_cast<float>(2.5 * max_spd), static_cast<float>(w_width * 3.0), fwd_speed);
    if (!(sa > 10.0f)) sa = 10.0f;                                   // [00CE38B8]
    const double look_d = static_cast<double>(v_look * look_t);      // 0099F470, a double

    // DIAGNOSTIC (cc9-physics27, BSP_PLANE_RATE_TRACE): "plane tr arm" per pass and
    // "plane tr run" per 0099CAB0 call for the named plane; log-only. The heights
    // are recomputed here without avoid_surface_height's census counters.
    const bool tr_trace = !plane_rate_trace_name().empty() && u.row.name == plane_rate_trace_name();
    auto tr_heights = [&](float x, float z, float& lh, float& sh) {
        OceanFieldBinding sea(*this);
        sh = bsp::ocean_water_height_0078cf20(x, z, sea);
        const GameAvoidZoneRegistry& reg = GameAvoidZoneRegistry::instance();
        const bsp::TerrainGridLayerRecord* layer = reg.layer(reg.squadron_layer_34c());
        lh = layer != nullptr ? avoid_zone_sample_0041bc20(*layer, x, z) : 0.0f;
    };
    float tr_fy = 0.0f, tr_wa = -1.0f, tr_wb = -1.0f, tr_w = -1.0f;   // DIAGNOSTIC

    float p0[3]{}, a[3]{}, mid[3]{}, b[3]{};
    float c0 = 0.1f;
    float r = 0.0f;
    const float* m = u.world.data();
    const bool dive = avoid_in_dive(u);
    const int bands_before = u.tr_bands;
    const float level_flight = bot_desired_speed_007c47f0(u);        // 007C47F0
    for (int pass = 0; pass < 2; ++pass) {
        if (pass == 0) {                                             // 0099F4A3-0099FAAF
            const float l[3] = {-m[0], -m[1], -m[2]};                // -row0
            float f[3] = {m[8], m[9], m[10]};
            const float sin_b = std::sin(bank);
            const float cos_p = std::cos(pitch);
            const float m_turn = avoid_interp(-0.15f, 0.0f, 0.2f, 2.5f, -pitch);   // [00CF87CC] [00CE54A0] [00CF87C8]
            const float c = m_turn * sin_b * cos_p * u.plane_class.slide_ratio * u.plane_class.yaw_spd;
            for (int i = 0; i < 3; ++i) f[i] = c * l[i] + f[i];      // 0099F62F's normalize is discarded
            tr_fy = f[1];                                            // DIAGNOSTIC
            r = static_cast<float>(avoid_interp(0.3f, 0.6f, 1.2f, 1.2f, bank_f) * look_d);   // [00CE69C8] [00CE3D30] [00CE3814]
            for (int i = 0; i < 3; ++i) {
                if constexpr (kPc24PlaneTasksCc13Bound) {
                    // pc24w1-sweep3-5: f * 10.0 is stored through float (0099F6BB..0099F6BF)
                    // before the subtraction, each op rounded at PC24.
                    p0[i] = static_cast<float>(bsp::cc13x87::x87_sub24(
                        p_now[i], static_cast<float>(bsp::cc13x87::x87_mul24(f[i], 10.0))));
                } else {
                p0[i] = static_cast<float>(p_now[i] - f[i] * 10.0);   // double [00CE3DC0]
                }
            }
            for (int i = 0; i < 3; ++i) {
                a[i] = static_cast<float>(r * (f[i] + l[i]) * 0.8) + p0[i];   // double [00CE3D40]
                mid[i] = f[i] * r + p0[i];
                b[i] = static_cast<float>(r * (f[i] - l[i]) * 0.8) + p0[i];
            }
            const float h0 = avoid_surface_height(p0[0], p0[2]);
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // pc24w1-sweep3-5: 0099FA81 FADD 10.0, 0099FA87 FSUBP, each rounded.
                c0 = static_cast<float>(bsp::cc13x87::x87_sub24(p0[1], bsp::cc13x87::x87_add24(h0, 10.0)));
            } else {
            c0 = static_cast<float>(p0[1] - (h0 + 10.0));
            }
            if constexpr (bsp::kUnitsHostDeviationsCc13Bound) {
                // cc13-unitsdev: 0099FA91 FLD double [00D7A3A0] = 0.10000000149011612
                // (0.1f widened), FCOMIP, JBE keeps c0; else [00D7A2F0] 0.1f. No float
                // lies between 0.1 and 0.1f, so the two compares agree on every input.
                if (0.10000000149011612 > c0) c0 = 0.1f;
            } else {
                if (0.1 > c0) c0 = 0.1f;                             // double [00D7A3A0]
            }
        } else {                                                     // 0099FAB4-0099FF49
            float f[3] = {m[8], m[9], m[10]};
            float up[3];
            for (int i = 0; i < 3; ++i) up[i] = static_cast<float>(m[4 + i] * 1.25);   // double [00CF87C0]
            if (inverted) {
                float k;
                if constexpr (kPc24PlaneTasksCc13Bound) {
                    // pc24w1-sweep3-5: both FCOS results stored through float, then
                    // 0099FB75 FMUL and 0099FB80 FMUL 0.15, each rounded at PC24.
                    k = static_cast<float>(bsp::cc13x87::x87_mul24(
                        bsp::cc13x87::x87_mul24(std::cos(pitch), std::cos(bank_f)), 0.15000000596046448));   // [00CE6618]
                } else {
                k = static_cast<float>(std::cos(bank_f) * std::cos(pitch) * 0.15);   // double [00CE6618]
                }
                for (int i = 0; i < 3; ++i) f[i] = k * up[i] + f[i];
                // 0099FBED (and 0099FCA0 below) call 00419510 with EDX = f but
                // ECX = an unused temporary ([ESP+468h] / [ESP+45Ch]); f is not
                // normalized and 0099FCA9 reads it as is (cc13-unitsdev, rev9).
            } else if (pitch < 0.0f) {
                const float wa = avoid_interp(10.0f, 0.0f, 50.0f, 1.0f, h9b4);   // [00CEB4D4]
                const float wb = avoid_interp(-0.4f, 0.0f, -0.15f, 1.0f, pitch); // [00D1F400]
                const float w = wb * wa;
                if (w > 0.0f) {
                    if constexpr (kPc24PlaneTasksCc13Bound) {
                        // pc24w1-sweep3-5: 0099FC8F FSUBRP 1 - w, 0099FC98 FMUL, rounded.
                        f[1] = static_cast<float>(bsp::cc13x87::x87_mul24(bsp::cc13x87::x87_sub24(1.0, w), f[1]));
                    } else {
                    f[1] = static_cast<float>((1.0 - w) * f[1]);
                    }
                }
                tr_wa = wa;                                          // DIAGNOSTIC
                tr_wb = wb;
                tr_w = w;
            }
            tr_fy = f[1];                                            // DIAGNOSTIC
            r = static_cast<float>(look_d * 1.1);                    // double [00CE3DF0]
            for (int i = 0; i < 3; ++i) {
                a[i] = static_cast<float>(r * (f[i] - up[i]) * 0.75) + p0[i];   // double [00CEC9D8]
                mid[i] = f[i] * r + p0[i];
                b[i] = static_cast<float>((f[i] + up[i]) * r * 0.75) + p0[i];
            }
        }
        // 0099FF50-009A01BA: extent over x and z, N steps.
        float ext = 0.0f;
        const float* pts[3] = {a, mid, b};
        for (const float* q : pts) {
            const float ex = std::fabs(p0[0] - q[0]), ez = std::fabs(p0[2] - q[2]);
            const float e = ex > ez ? ex : ez;
            if (e > ext) ext = e;
        }
        const int n_steps = static_cast<int>(ext / cell) + 1;
        const float n_f = static_cast<float>(n_steps);
        const float ds = 1.0f / n_f;
        float s = ds > 1.0f ? 1.0f : ds;
        if (tr_trace) {                                              // DIAGNOSTIC
            const GameAvoidZoneRegistry& reg = GameAvoidZoneRegistry::instance();
            const int li = static_cast<int>(reg.squadron_layer_34c());
            const bsp::TerrainGridLayerRecord* layer = reg.layer(reg.squadron_layer_34c());
            float lh_pos = 0.0f, sh_pos = 0.0f, lh_p0 = 0.0f, lh_a = 0.0f, lh_m = 0.0f,
                  lh_b = 0.0f, sh_x = 0.0f;
            tr_heights(p_now[0], p_now[2], lh_pos, sh_pos);
            tr_heights(p0[0], p0[2], lh_p0, sh_x);
            tr_heights(a[0], a[2], lh_a, sh_x);
            tr_heights(mid[0], mid[2], lh_m, sh_x);
            tr_heights(b[0], b[2], lh_b, sh_x);
            log.notef("plane tr arm %s t=%.2f pass=%d pos=(%.1f %.1f %.1f) layer=%d cell=%.1f "
                "half_ext=%.1f dim=%d layer_h=%.2f sea_h=%.2f h9b4=%.2f pitch=%.4f bank=%.4f "
                "bank_f=%.4f inv=%d speed=%.2f v_stall=%.2f look_t=%.3f look_d=%.2f r=%.2f "
                "sa=%.2f sa_parts=(%.2f %.2f %.2f %.2f) max_spd=%.1f width=%.2f fy=%.4f "
                "wa=%.3f wb=%.3f w=%.3f p0=(%.1f %.1f %.1f|%.2f) a=(%.1f %.1f %.1f|%.2f) "
                "mid=(%.1f %.1f %.1f|%.2f) b=(%.1f %.1f %.1f|%.2f) c0=%.2f n_steps=%d",
                u.row.name.c_str(), static_cast<double>(summary.simulated_seconds), pass,
                static_cast<double>(p_now[0]), static_cast<double>(p_now[1]),
                static_cast<double>(p_now[2]), li,
                layer ? static_cast<double>(layer->cell_size) : -1.0,
                layer ? static_cast<double>(layer->half_extent) : -1.0,
                layer ? layer->dimension : -1, static_cast<double>(lh_pos),
                static_cast<double>(sh_pos), static_cast<double>(h9b4),
                static_cast<double>(pitch), static_cast<double>(bank),
                static_cast<double>(bank_f), inverted ? 1 : 0, static_cast<double>(speed),
                static_cast<double>(v_stall), static_cast<double>(look_t), look_d,
                static_cast<double>(r), static_cast<double>(sa),
                static_cast<double>(sa_bank_trace), static_cast<double>(sa_pitch_trace),
                static_cast<double>(sa_h_trace), static_cast<double>(sa_speed_trace),
                static_cast<double>(max_spd), static_cast<double>(w_width),
                static_cast<double>(tr_fy), static_cast<double>(tr_wa),
                static_cast<double>(tr_wb), static_cast<double>(tr_w),
                static_cast<double>(p0[0]), static_cast<double>(p0[1]), static_cast<double>(p0[2]),
                static_cast<double>(lh_p0),
                static_cast<double>(a[0]), static_cast<double>(a[1]), static_cast<double>(a[2]),
                static_cast<double>(lh_a),
                static_cast<double>(mid[0]), static_cast<double>(mid[1]), static_cast<double>(mid[2]),
                static_cast<double>(lh_m),
                static_cast<double>(b[0]), static_cast<double>(b[1]), static_cast<double>(b[2]),
                static_cast<double>(lh_b), static_cast<double>(c0), n_steps);
        }
        auto run_note = [&](int j, float tt, float ss, const char* site, float lo_, float hi_,
                            float dlo_, float dhi_, const float x[3], float cc) {   // DIAGNOSTIC
            if (!tr_trace) return;
            float lh = 0.0f, sh = 0.0f;
            tr_heights(x[0], x[2], lh, sh);
            log.notef("plane tr run %s t=%.2f pass=%d j=%d tt=%.3f s=%.3f site=%s lo=%.4f "
                "hi=%.4f dlo=%.2f dhi=%.2f smp=(%.1f %.1f %.1f) c=%.2f layer_h=%.2f",
                u.row.name.c_str(), static_cast<double>(summary.simulated_seconds), pass, j,
                static_cast<double>(tt), static_cast<double>(ss), site,
                static_cast<double>(lo_), static_cast<double>(hi_), static_cast<double>(dlo_),
                static_cast<double>(dhi_), static_cast<double>(x[0]), static_cast<double>(x[1]),
                static_cast<double>(x[2]), static_cast<double>(cc), static_cast<double>(lh));
        };
        auto len_from_p0 = [&](const float x[3]) {
            float len;
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // pc24w1-sweep3-5: x - p0 stored through float (009A06D3..009A0702),
                // then 0042B2F0's inline form (009A0709..009A073E), each op at PC24.
                const float d[3] = {static_cast<float>(x[0] - p0[0]), static_cast<float>(x[1] - p0[1]),
                                    static_cast<float>(x[2] - p0[2])};
                len = length_0042b2f0_pc24(d);
            } else {
            const double dx = static_cast<double>(x[0]) - p0[0];
            const double dy = static_cast<double>(x[1]) - p0[1];
            const double dz = static_cast<double>(x[2]) - p0[2];
            const double d2 = dy * dy + dx * dx + dz * dz;
            len = d2 > 1e-10 ? static_cast<float>(std::sqrt(d2)) : 0.0f;   // double [00CE3820]
            }
            return len;
        };
        auto clearance = [&](const float x[3]) {
            return x[1] - (avoid_surface_height(x[0], x[2]) + sa);   // 009A0677
        };
        for (int j = 1; j <= n_steps; ++j) {
            const float t = static_cast<float>(j) / n_f;
            float e1[3], cc[3], cme[3];
            for (int i = 0; i < 3; ++i) {
                const float q = mid[i] + (a[i] - mid[i]) * s;
                e1[i] = p0[i] + t * (q - p0[i]);
                cc[i] = p0[i] + t * (mid[i] - p0[i]);
                cme[i] = cc[i] - e1[i];
            }
            float lo = -1.1f;                                        // [00D06BB0]
            int state = 0;
            float dlo = 0.0f, dhi = 0.0f, c_prev = 0.0f, c = 0.0f;
            bool blocked = false;
            float smp[3]{};
            const float ax = std::fabs(cme[0]), az = std::fabs(cme[2]);
            const float span = static_cast<float>((ax > az ? ax : az) * 1.2);   // double [00CEC160]
            const int n2 = static_cast<int>(span / cell) + 1;
            const float n2f = static_cast<float>(n2);
            for (int k = 0; k <= n2; ++k) {                          // E (-1) -> C (0)
                const float fk = static_cast<float>(k) / n2f;
                for (int i = 0; i < 3; ++i) smp[i] = e1[i] + fk * cme[i];
                c = clearance(smp);
                blocked = 0.0f > c;
                if (state == 0) {
                    state = blocked ? -1 : 1;
                    if (blocked) dlo = c0 / (c0 - c) * len_from_p0(smp);
                } else if (state > 0) {
                    if (blocked) {
                        const float rr = c_prev / (c_prev - c);
                        if constexpr (kPc24PlaneTasksCc13Bound) {
                            // pc24w1-sweep3-5: rr - 1, / n2, - 1 (stored), + fk, each op rounded at
                        // PC24 (009A07BB..009A07D2; 009A0EF7..009A0F0C has the form without - 1).
                            lo = static_cast<float>(bsp::cc13x87::x87_add24(fk, bsp::cc13x87::x87_sub24(
                                bsp::cc13x87::x87_div24(bsp::cc13x87::x87_sub24(rr, 1.0), n2f), 1.0)));
                        } else {
                        lo = static_cast<float>(fk + ((rr - 1.0) / n2f - 1.0));
                        }
                        const float fx = lo + 1.0f;
                        float x[3];
                        for (int i = 0; i < 3; ++i) x[i] = fx * cme[i] + e1[i];
                        dlo = len_from_p0(x);
                        state = -1;
                    }
                } else if (!blocked) {
                    const float rr = c_prev / (c_prev - c);
                    float hi;
                    if constexpr (kPc24PlaneTasksCc13Bound) {
                        hi = static_cast<float>(bsp::cc13x87::x87_add24(fk, bsp::cc13x87::x87_sub24(
                            bsp::cc13x87::x87_div24(bsp::cc13x87::x87_sub24(rr, 1.0), n2f), 1.0)));
                    } else {
                    hi = static_cast<float>(fk + ((rr - 1.0) / n2f - 1.0));
                    }
                    const float fx = hi + 1.0f;
                    float x[3];
                    for (int i = 0; i < 3; ++i) x[i] = cme[i] * fx + e1[i];
                    dhi = len_from_p0(x);
                    run_note(j, t, s, "first_half", lo, hi, dlo, dhi, smp, c);   // DIAGNOSTIC
                    terrain_shape_band_0099cab0(*this, u, pass, lo, hi, dlo, dhi, r, bank_f,
                                                level_flight, speed, tr_trace);   // 009A0AE0
                    state = 1;
                }
                c_prev = c;
            }
            float e2[3], c2[3], emc[3];
            for (int i = 0; i < 3; ++i) {
                const float q2 = mid[i] + s * (b[i] - mid[i]);
                e2[i] = p0[i] + (q2 - p0[i]) * t;
                c2[i] = p0[i] + (mid[i] - p0[i]) * t;
                emc[i] = e2[i] - c2[i];
            }
            for (int k = 1; k <= n2; ++k) {                          // C (0) -> E' (+1)
                const float fk = static_cast<float>(k) / n2f;
                for (int i = 0; i < 3; ++i) smp[i] = fk * emc[i] + c2[i];
                c = clearance(smp);
                blocked = 0.0f > c;
                if (state > 0) {
                    if (blocked) {
                        const float rr = c_prev / (c_prev - c);
                        if constexpr (kPc24PlaneTasksCc13Bound) {
                            // pc24w1-sweep3-5: rr - 1, / n2 (stored), + fk, each rounded
                            // (009A0EF5..009A0F0C).
                            lo = static_cast<float>(bsp::cc13x87::x87_add24(
                                fk, bsp::cc13x87::x87_div24(bsp::cc13x87::x87_sub24(rr, 1.0), n2f)));
                        } else {
                        lo = static_cast<float>(fk + (rr - 1.0) / n2f);
                        }
                        float x[3];
                        for (int i = 0; i < 3; ++i) x[i] = e2[i] + (1.0f - lo) * (c2[i] - e2[i]);
                        dlo = len_from_p0(x);
                        state = -1;
                    }
                } else if (!blocked) {
                    const float rr = c_prev / (c_prev - c);
                    float hi;
                    if constexpr (kPc24PlaneTasksCc13Bound) {
                        hi = static_cast<float>(bsp::cc13x87::x87_add24(
                            fk, bsp::cc13x87::x87_div24(bsp::cc13x87::x87_sub24(rr, 1.0), n2f)));
                    } else {
                    hi = static_cast<float>(fk + (rr - 1.0) / n2f);
                    }
                    float x[3];
                    for (int i = 0; i < 3; ++i) x[i] = e2[i] + (1.0f - hi) * (c2[i] - e2[i]);
                    dhi = len_from_p0(x);
                    run_note(j, t, s, "second_half", lo, hi, dlo, dhi, smp, c);   // DIAGNOSTIC
                    terrain_shape_band_0099cab0(*this, u, pass, lo, hi, dlo, dhi, r, bank_f,
                                                level_flight, speed, tr_trace);   // 009A1240
                    state = 1;
                }
                c_prev = c;
            }
            if (blocked) {                                           // 009A1296-009A138E
                dhi = c0 / (c0 - c) * len_from_p0(smp);
                run_note(j, t, s, "open_end", lo, 1.1f, dlo, dhi, smp, c);   // DIAGNOSTIC
                terrain_shape_band_0099cab0(*this, u, pass, lo, 1.1f, dlo, dhi, r, bank_f,
                                            level_flight, speed, tr_trace);
            }
            s = s + ds;                                              // 009A13AC
            if (s > 1.0f) s = 1.0f;
        }
    }
    if (u.tr_bands > bands_before) {
        ++u.tr_ticks;
        if (h9b4 < u.tr_min_margin) u.tr_min_margin = h9b4;
        if (dive) {
            ++u.tr_dive_ticks;
            if (u.tr_dive_ticks <= 3) {
                log.notef("terrain avoid in dive: unit=%s state=0x%X alt=%.1f pitch=%.3f "
                          "speed=%.1f sa=%.1f pitch_bands=%d first=[%.3f,%.3f] thr=%.3f",
                          u.row.name.c_str(), static_cast<unsigned>(u.dive_bomb_state),
                          static_cast<double>(h9b4), static_cast<double>(pitch),
                          static_cast<double>(speed), static_cast<double>(sa), u.ga_band_n[1],
                          u.ga_band_n[1] > 0 ? static_cast<double>(u.ga_band[1][0][0]) : 0.0,
                          u.ga_band_n[1] > 0 ? static_cast<double>(u.ga_band[1][0][1]) : 0.0,
                          static_cast<double>(u.av_throttle_25c));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Packet cc9_player_role_bookkeeping (docs/SCRIPTED_HELM.md section 2)
// ---------------------------------------------------------------------------

// 00780120's 4Bh arm (00780162-007803D9) with msg+20h == 0 and msg+30h == 0,
// the shape 0077C470 and 00927D20 send: mask at +24h, slot at +28h, take at +2Ch.
void GameUnitsHost::Impl::role_message_4b_00780162(std::size_t index, std::uint32_t mask,
                                                   std::int32_t player_slot, int take) {
    if (index >= slots.size()) return;
    GameUnitSlot& u = *slots[index];
    // 00927F10 on a slot other than 8: in this single-player host every slot
    // but the local player's is AI-held. SUBSTITUTION, labelled.
    auto ai_held = [](std::int32_t s) { return s != kLocalPlayerSlot; };
    // 0059BBD0(unit, role, slot): unit+188h + role*4 == 9 || == slot.
    auto open_to = [&](int role, std::int32_t s) {
        // 0059BBD0: native_unit_role_open_to_slot_0059bbd0 over the unit's +188h role
        // policy table (role_permission_0188); OFF keeps the inline test.
        return bsp::native_unit_role_open_to_slot_0059bbd0(u.role_permission_0188,
            static_cast<std::uint32_t>(role), static_cast<std::uint32_t>(s)) != 0;
    };
    // 009281C0 through vtable[154h] = 0077F480: role 0 given 8 clears +184h.
    auto set_role = [&](int role, std::int32_t s) {
        if (role == 0 && s == 8) u.role_player_0184 = false;   // 009281D6
        u.current_roles_01ac[role] = s;                         // 009281E2
    };
    if (take == 1) {
        if ((mask & 2u) != 0 && open_to(1, player_slot)) {     // 007801FE-00780214
            u.role_player_0184 = true;
        }
    } else if ((mask & 1u) != 0 && u.current_roles_01ac[0] == player_slot) {
        u.role_player_0184 = false;                             // 00780235
        // 0078023C vtable[5Ch](6), then 0080E290(unit) at 00780244 for a kind-6 unit.
        // kHelmReleaseGunsToRestCc13Bound (packet cc13-frontier): the gunnery host's
        // 0080E290 over this unit's guns. OFF: recorded, before the kind test as before.
        if constexpr (bsp::kHelmReleaseGunsToRestCc13Bound) {
            if (bsp::unit_is_kind_of(u.class_id, 6) && gunnery) {
                gunnery->aim_unit_turning_guns_to_rest_0080e290(index);
                done("RoleMessage::kind6_release_0080e290", 0x0080e290u);
            }
        } else {
            record("RoleMessage::kind6_release_0080e290", 0x0080e290u);
        }
    }
    std::uint32_t bit = 1;
    for (int i = 0; i < bsp::kUnitRoleTableEntries; ++i, bit <<= 1) {
        if ((mask & bit) == 0) continue;
        const std::int32_t holder = u.current_roles_01ac[i];
        if (take == 1) {
            // Role 0 also rebinds the player record's unit (+4Ch) and routes a
            // 1FFh release for a different live unit (007802A0-0078030A): the
            // host keeps its controlled unit through 004C0890 instead. Labelled.
            if ((holder == 8 || ai_held(holder)) && open_to(i, player_slot)) {
                set_role(i, player_slot);                       // 0078037C vtable[154h]
                ++role_takes;
            } else if (holder == player_slot) {
                ++role_held_retakes;                            // 0078035C, a no-op
            } else {
                ++role_refusals;
            }
        } else if (holder == player_slot) {
            set_role(i, 8);
            ++role_releases;
        }
        // 00780393..007803A8: a changed holder hands the message to the unit's
        // HandleMessage (0077FE80 -> vtable[164h] -> 00821E80 -> 0095ABE0),
        // whose 4Bh arm walks the devices. kRoleOwnerDeviceWalkBound.
        if (kRoleOwnerDeviceWalkBound && u.current_roles_01ac[i] != holder && gunnery) {
            gunnery->role_owner_device_walk_0095ac1d(index, mask, player_slot, take == 1);
        }
    }
    done("Session::entity_role_message_4b", 0x00780162u);
}

// 0067BB50, slot 20h of HUD page 27h (vtable 00CF7A38, constructor
// 0068A8A0), which the player's unit interfaces open (INTF_CAPTAIN and the
// rest, docs/IN_MISSION_INTERFACE_MANAGER.md). __thiscall(ECX page, float
// seconds), RET 4. The page byte +4h (BL, read at 0067BB5D) is always set
// here: the only caller is the pump's slot-20h call (vtable dword 00CF7A58 is
// the one reference), which skips a page whose +4h or +5h is clear
// (004F890C/004F8912), so the clear-byte release at 0067BC16..0067BC21 is
// unreachable and not modelled. game+18ECh, the local slot (set by
// 004B4740 from the session select-player paths), is kLocalPlayerSlot 0 in
// the host, so the [0, 7] gate at 0067BBC8..0067BBD4 always passes
// (cc9-ships50, SHIP_AI_OPEN_ITEMS 248).
bool GameUnitsHost::Impl::player_flag_0184(const GameUnitSlot& slot) const {
    if constexpr (kPlayerRoleBookkeepingBound) {
        return slot.role_player_0184;
    } else {
        return is_controlled(slot);
    }
}

void GameUnitsHost::Impl::role_screen_update_0067bb50() {
    const std::size_t controlled_plus_one =
        (controlled_bound && controlled_index < slots.size()) ? controlled_index + 1 : 0;
    auto holds = [&](std::size_t plus_one, int role) {   // 00927F30(unit, role)
        return plus_one != 0 &&
               slots[plus_one - 1]->current_roles_01ac[role] == kLocalPlayerSlot;
    };
    auto send = [&](std::size_t plus_one, std::uint32_t mask, int take) {   // 0077C470
        // 0077C470's gate: game+5D4h > 0Ch (in mission) and session mode 0.
        role_message_4b_00780162(plus_one - 1, mask, kLocalPlayerSlot, take);
    };
    // 00523580 (bsp.py show --asm): null receiver -> null; otherwise ask
    // vslot 5Ch for kind 2 and retain that same receiver on success. Bind
    // only this kind slot to the controlled unit's live class identity.
    struct UnitKind final : bsp::cc13_seg14::EntityKindHooks {
        bool object_is_kind_virtual_5c(void* object, std::uint32_t kind) override {
            return bsp::unit_is_kind_of(static_cast<GameUnitSlot*>(object)->class_id,
                static_cast<int>(kind));
        }
    } kinds;
    auto* const controlled_slot = controlled_plus_one != 0
        ? slots[controlled_plus_one - 1].get() : nullptr;
    auto* const role_owner = static_cast<GameUnitSlot*>(
        bsp::cc13_seg14::entity_cast_if_kind_00523580(controlled_slot, kinds));
    if (role_owner == nullptr) {
        if (role_screen_unit_plus_one != 0) {                    // 0067BC2C..0067BC37
            send(role_screen_unit_plus_one, 1u, 0);
            role_screen_unit_plus_one = 0;
        }
        return;
    }
    if (role_screen_unit_plus_one != controlled_plus_one && role_screen_unit_plus_one != 0 &&
        holds(role_screen_unit_plus_one, 0)) {
        send(role_screen_unit_plus_one, 1u, 0);                  // 0067BB96
    }
    role_screen_unit_plus_one = controlled_plus_one;
    // 0067BBC8: game+18ECh in [0, 7]; 0067BBE0: page byte +4h set (both always
    // hold on the pump path, see above).
    if (holds(controlled_plus_one, 0)) return;
    GameUnitSlot& c = *role_owner;
    // 0059BBD0: native_unit_role_open_to_slot_0059bbd0(role 0, the local slot) over the
    // unit's +188h policy table; OFF keeps the inline test.
    if (bsp::native_unit_role_open_to_slot_0059bbd0(c.role_permission_0188, 0u,
            static_cast<std::uint32_t>(kLocalPlayerSlot)) == 0) {
        return;
    }
    send(controlled_plus_one, 1u, 1);                            // 0067BC0B
}

// ---------------------------------------------------------------------------
// Packet cc9_scripted_helm_option (docs/SCRIPTED_HELM.md section 7)
// ---------------------------------------------------------------------------

// Once per fixed step, before the unit loop: read the option, open the pilot
// role once, then 0064B870's transfer block (0064B97A-0064B9BD).
void GameUnitsHost::Impl::player_helm_prepare_0064b870() {
    if (!helm_option_read) {
        helm_option_read = true;
        char* text = nullptr;
        std::size_t bytes = 0;
        if (_dupenv_s(&text, &bytes, "BSP_PLAYER_HELM") == 0 && text != nullptr) {
            float t = 0.0f, r = 0.0f;
            const int n = sscanf_s(text, "%f,%f", &t, &r);
            if (n >= 1) {
                helm_option_on = true;
                helm_throttle = t;
                helm_rudder = n >= 2 ? r : 0.0f;
                log.notef("scripted helm: BSP_PLAYER_HELM=%s -> throttle %.3f rudder %.3f. "
                    "A MEASUREMENT SCENARIO: EROLF_PILOT is opened to PLAYER_ANY on the "
                    "controlled unit through 0077F360, then 0064B870's role-1 transfer and "
                    "per-frame helm issue run with these lever values", text,
                    static_cast<double>(helm_throttle), static_cast<double>(helm_rudder));
            }
            std::free(text);
        }
    }
    if (!helm_option_on && !helm_route_on) return;
    if (!controlled_bound || controlled_index >= slots.size()) return;
    GameUnitSlot& unit = *slots[controlled_index];
    if (!helm_opened) {
        // SetRoleAvailable(unit, EROLF_PILOT, PLAYER_ANY): 008ABA51 in session
        // mode 0 calls vtable[148h] = 0077F360 -> 00927D20 with (2, 9). It runs
        // at the first mission step, after the script's own init calls
        // (usn_19_coralus.lua 458-459). Its kind-5 tail is not modelled.
        set_role_availability_00927d20_impl(controlled_index, 2u, 9);
        helm_opened = true;
    }
    auto holds = [&](int role) { return unit.current_roles_01ac[role] == kLocalPlayerSlot; };
    // 0064B940-0064B97A: the thrust axis |a| > 0.1 (double [00D7A3A0]) or the
    // held-control flag. The option's throttle stands in for the axis.
    const bool axis = static_cast<double>(std::fabs(helm_throttle)) > 0.10000000149011612;
    // unit+1130h has no host field; taken as 0 (0064B97D). Labelled.
    if (axis && holds(0) && !holds(1)) {
        role_message_4b_00780162(controlled_index, 2u, kLocalPlayerSlot, 1);   // 0064B9A6
        helm_lever_28 = unit.ring.current_param_b;                            // 0064B9AE, +984h
        helm_lever_24 = unit.ring.current_param_a;                            // 0064B9B7, +980h
        ++helm_transfers;
        log.notef("scripted helm transfer: 0077C470(\"%s\", 2, 1) -> role1=%d +184h=%d; "
            "levers seeded from +980h %.4f / +984h %.4f", unit.row.name.c_str(),
            unit.current_roles_01ac[1], unit.role_player_0184 ? 1 : 0,
            static_cast<double>(helm_lever_24), static_cast<double>(helm_lever_28));
    }
}

// 0064B9C0-0064BB16 for the controlled unit while the player holds role 1:
// the levers, quantized by 0064BAB5/0064BAEE, issued through 00816A40, whose
// single-player publication is 0080DAD0 (no 8Eh in session mode 0). Called
// where the standing order is refilled, once per fixed step.
void GameUnitsHost::Impl::player_helm_issue_0064b870(GameUnitSlot& slot) {
    if (!controlled_bound || controlled_index >= slots.size() ||
        slots[controlled_index].get() != &slot) {
        return;
    }
    if (slot.current_roles_01ac[1] != kLocalPlayerSlot) return;   // 00927F30(unit, 1)
    if (helm_route_on) {
        // Packet cc9_helm_orders_helm_route. A player's hand moves the turn
        // lever; the harness stands in for the hand with the image's own AI
        // law, 009DA250 (rudder = -error / (class+524h * 1.2), clamped, with its
        // low-speed ramp), on the error from the hull heading to the bearing of
        // the point. LABELLED: the law is the AI's, not a player's; the error is
        // wrapped to [-pi, pi] here in place of 00438B10.
        const double dx = static_cast<double>(helm_route_x) - slot.motion.position[0];
        const double dz = static_cast<double>(helm_route_z) - slot.motion.position[2];
        const double distance = std::sqrt(dx * dx + dz * dz);
        const float bearing = static_cast<float>(std::atan2(dx, dz));
        double error = static_cast<double>(bearing) - pose_heading_radians(slot);
        while (error > 3.141592653589793) error -= 6.283185307179586;
        while (error < -3.141592653589793) error += 6.283185307179586;
        struct Speed final : bsp::ShipAiRudderLawHost {
            float v{0.0f};
            float unit_body_axis_speed_0092d730() override { return v; }
        } speed;
        bsp::UnitBodyAxisSpeedInputs axis{};
        axis.velocity[0] = slot.motion.linear_velocity.x;
        axis.velocity[1] = slot.motion.linear_velocity.y;
        axis.velocity[2] = slot.motion.linear_velocity.z;
        axis.axis[0] = slot.motion.pose_row2[0];
        axis.axis[1] = slot.motion.pose_row2[1];
        axis.axis[2] = slot.motion.pose_row2[2];
        speed.v = bsp::unit_forward_speed_0092d730(axis);
        const bsp::ShipClassAiDerivedMotion derived = bsp::ship_class_ai_derived_motion_00828f20(
            slot.fields.max_rot_angle, slot.fields.max_rot_angle_change_ratio,
            slot.fields.max_speed);
        helm_rudder = bsp::ship_ai_rudder_from_heading_error_009da250(
            bsp::ShipAiThrottleDirection::Ahead, static_cast<float>(error),
            derived.yaw_authority_0524, speed);
        if (helm_issues % 600 == 0) {
            log.notef("helm route \"%s\": at (%.1f, %.1f) distance %.1f m heading %.4f "
                "bearing %.4f rudder %.3f speed %.2f", slot.row.name.c_str(),
                static_cast<double>(slot.motion.position[0]),
                static_cast<double>(slot.motion.position[2]), distance,
                static_cast<double>(pose_heading_radians(slot)), static_cast<double>(bearing),
                static_cast<double>(helm_rudder), static_cast<double>(speed.v));
        }
        if (!helm_route_arrived && distance < 500.0) {
            helm_route_arrived = true;
            log.notef("helm route arrived: \"%s\" within 500 m of (%.1f, %.1f) at %.1f m, "
                "after %d helm issues", slot.row.name.c_str(),
                static_cast<double>(helm_route_x), static_cast<double>(helm_route_z),
                distance, helm_issues);
        }
    }
    // The input integration (0064B9C6-0064BA90) moves the levers toward the
    // held axes; the scripted helm sets them to the option's values instead,
    // clamped to [-1, 1]; the bounds of 0064BA3C/0064BA67's clamp were not
    // read. SUBSTITUTION, labelled.
    helm_lever_24 = helm_throttle < -1.0f ? -1.0f : (helm_throttle > 1.0f ? 1.0f : helm_throttle);
    helm_lever_28 = helm_rudder < -1.0f ? -1.0f : (helm_rudder > 1.0f ? 1.0f : helm_rudder);
    // 0064BA97-0064BB12: floor(B / step + bias) * step, floor(4A + bias) / 4.
    const double step = 0.16666667163372039794921875;              // [00CF5E60]
    const double bias = 0.4900000095367431640625;                  // [00CF5E58]
    float bin = 0.0f;
    float ain = 0.0f;
    if constexpr (bsp::kPc24HudGuiCc13Bound) {
        // PC24 (packet pc24w1-hudgui-2): 0064BA99 FDIV and 0064BAD7 FMUL round to
        // 24 bits before the FADD of the bias (0064BAA4, 0064BADD).
        bin = static_cast<float>(bsp::cc13x87::x87_add24(
            bsp::cc13x87::x87_div24(helm_lever_28, step), bias));
        ain = static_cast<float>(bsp::cc13x87::x87_add24(
            bsp::cc13x87::x87_mul24(helm_lever_24, 4.0), bias));         // [00D7A328]
    } else {
        bin = static_cast<float>(helm_lever_28 / step + bias);
        ain = static_cast<float>(helm_lever_24 * 4.0 + bias);           // [00D7A328]
    }
    const float turn = static_cast<float>(
        static_cast<float>(std::floor(static_cast<double>(bin))) * step);
    const float thrust = static_cast<float>(
        static_cast<float>(std::floor(static_cast<double>(ain))) * 0.25);   // [00D7A348]
    issue_into_ring(slot, thrust, turn);                            // 00816A40, kind 0
    ++helm_issues;
    // 0064BB19: a held role 1 is released when game+19C4h is set; that byte
    // has no host producer and is taken clear. Labelled.
}

// ---------------------------------------------------------------------------
// Packet cc9_fighter_accel_friendly_fire (docs/FIGHTER_GUN_LEAD.md section 5)
// ---------------------------------------------------------------------------

// 007B96D0 (body 007B96D0-007B96EB): unit+C50h != 0 && 007DEDB0(unit+C50h).
// 007DEDB0 (__fastcall(neighbours)): while +80h <= +90h (1.0, 007E1EFF) it
// returns the cached +C8h; otherwise it clears +C8h, takes +90h off +80h and
// walks the +70h friendly list: a live (+5Eh clear) aircraft (IsKindOf 0Fh)
// whose position, in the owner's frame (00414E10 on [[this+4]+28h]), has
// z > 1.0 ([00D7A24C]) and x*x + y*y < +94h = 400.0 ([00CFD710], 007E1F13)
// sets +C8h. The +80h clock advances by dt in 007E2010. Its start
// U(0, 1) + 1.0 is taken at its midpoint 1.5, and the +70h list refresh runs
// on its own copy of the 3 s +7Ch clock at the think interval. Labelled.
// Packet cc13-fire: 009FC7C0 calls 007B96D0 only on envelope ticks, while
// 007E2010 runs every update, so the two halves are split; this keeps the
// old order for the callers that take both at once.
bool GameUnitsHost::Impl::friendly_in_line_007b96d0(GameUnitSlot& u, float dt) {
    friendly_in_line_clock_007e2010(u, dt);
    return friendly_in_line_eval_007dedb0(u);
}

void GameUnitsHost::Impl::friendly_in_line_clock_007e2010(GameUnitSlot& u, float dt) {
    const bsp::PlaneNeighbourRadii radii = bsp::plane_neighbour_radii_007e11d0(3.0f);
    const float* p0 = u.world.data() + 12;
    auto dist2 = [&](const GameUnitSlot& o) {
        const double dx = static_cast<double>(o.world[12]) - p0[0];
        const double dy = static_cast<double>(o.world[13]) - p0[1];
        const double dz = static_cast<double>(o.world[14]) - p0[2];
        return dx * dx + dy * dy + dz * dz;
    };
    auto in_world = [](const GameUnitSlot& o) {
        return o.state != nullptr && o.state->active != 0 && o.scene_destroyed_005e == 0 &&
               o.scene_pending_destroy_0060 == 0;
    };
    nb_seed_clocks(u);
    u.nb_clock_70 += dt;
    if (u.nb_clock_70 >= 3.0f) {                                     // 007E204D
        // 007E11D0: drop +70h members at or beyond the friendly radius, then
        // add same-party aircraft within it.
        const double lim = static_cast<double>(radii.friendly_plane) * radii.friendly_plane;
        auto& v = u.nb_friend_70;
        for (std::size_t k = 0; k < v.size();) {
            if (v[k] >= slots.size() || !in_world(*slots[v[k]]) || dist2(*slots[v[k]]) >= lim) {
                v[k] = v.back();
                v.pop_back();
            } else {
                ++k;
            }
        }
        for (std::size_t j = 0; j < slots.size(); ++j) {
            const GameUnitSlot& o = *slots[j];
            if (&o == &u || !in_world(o)) continue;
            if (!bsp::unit_is_kind_of(o.class_id, 0x0F)) continue;
            if (o.row.party != u.row.party) continue;
            if (!(dist2(o) < lim)) continue;
            bool listed = false;
            for (const std::size_t e : v) listed = listed || e == j;
            if (!listed) v.push_back(j);
        }
        u.nb_clock_70 -= 3.0f;
    }
    u.nb_clock_80 += dt;                                             // 007E2010
}

bool GameUnitsHost::Impl::friendly_in_line_eval_007dedb0(GameUnitSlot& u) {
    if (u.nb_clock_80 <= 1.0f) return u.nb_busy_c8;                  // 007DEDC4
    u.nb_busy_c8 = false;
    u.nb_clock_80 -= 1.0f;
    ++u.ff_checks;
    for (const std::size_t idx : u.nb_friend_70) {
        if (idx >= slots.size()) continue;
        const GameUnitSlot& o = *slots[idx];
        if (!bsp::unit_is_kind_of(o.class_id, 0x0F)) continue;       // 007DEE0C
        if (o.scene_destroyed_005e != 0) continue;                   // 007DEE16 +5Eh
        float l[3];
        follow_to_body(u, o.world.data() + 12, false, l);            // 004142E0
        if (l[2] > 1.0f && l[0] * l[0] + l[1] * l[1] < 400.0f) {
            u.nb_busy_c8 = true;
            break;
        }
    }
    return u.nb_busy_c8;
}

// ---------------------------------------------------------------------------
// Impl helpers
// ---------------------------------------------------------------------------

void GameUnitsHost::Impl::refresh_row(GameUnitSlot& slot) {
    GameUnitRow& row = slot.row;
    row.active = slot.state != nullptr && slot.state->active != 0;
    for (int i = 0; i < 3; ++i) row.position[i] = slot.motion.position[i];
    row.heading_degrees = heading_degrees_of(slot.motion);
    bsp::UnitBodyAxisSpeedInputs speed{};
    speed.velocity[0] = slot.motion.linear_velocity.x;
    speed.velocity[1] = slot.motion.linear_velocity.y;
    speed.velocity[2] = slot.motion.linear_velocity.z;
    speed.axis[0] = slot.motion.pose_row2[0];
    speed.axis[1] = slot.motion.pose_row2[1];
    speed.axis[2] = slot.motion.pose_row2[2];
    row.forward_speed = bsp::unit_forward_speed_0092d730(speed);
    row.throttle = slot.motion.throttle;
    row.ordered_rudder = slot.motion.to_turn;
    row.rudder = slot.motion.smoothed_rudder;
    row.yaw_rate = slot.motion.angular_velocity.y;
    // The component about the hull's own up axis, which is what 0092e8c0 slews.
    row.yaw_rate_up_axis = slot.motion.angular_velocity.x * slot.motion.pose_row1[0]
        + slot.motion.angular_velocity.y * slot.motion.pose_row1[1]
        + slot.motion.angular_velocity.z * slot.motion.pose_row1[2];
    const double dx = static_cast<double>(row.position[0]) - static_cast<double>(row.start[0]);
    const double dz = static_cast<double>(row.position[2]) - static_cast<double>(row.start[2]);
    row.distance = static_cast<float>(std::sqrt(dx * dx + dz * dz));
}

void GameUnitsHost::Impl::issue_into_ring(GameUnitSlot& slot, float throttle, float rudder) {
    // 00816a40 builds the 20h-byte record through 00815440, which clamps both
    // parameters into [-2,+2], and publishes it into the slot the write cursor
    // names. The queue and the ring are two projections of the same native
    // storage at unit+838h, so the published slot is copied into the ring the
    // tick reads; 0080dad0 clears +8h, which marks the slot authoritative.
    UnitOrderIssueBinding issue(*this);
    slot.queue.slot_index = slot.ring.write_cursor;
    bsp::issue_unit_order_record_00816a40(issue, slot.queue, slot.scratch, throttle, rudder, 0);
    const int index = slot.queue.slot_index;
    slot.ring.slot[index].param_a = slot.queue.slot[index].param_a;
    slot.ring.slot[index].param_b = slot.queue.slot[index].param_b;
    slot.ring.slot[index].kind = slot.queue.slot[index].kind;
    slot.ring.slot[index].predicted = slot.queue.slot_active[index];
    done("ShipMotion::issue_order_record", 0x00816a40u);
}  // namespace

std::size_t units_deliver_ship_sink_92h_0082220d() {
    if constexpr (bsp::kLeakMessageRouterDeliveryCc13Bound) {
        return g_leak92_units != nullptr ? g_leak92_units->deliver_ship_sink_92h_0082220d() : 0;
    } else {
        return 0;
    }
}

GameUnitsHost::GameUnitsHost(GameHostLog& log, GameMissionLuaHost& lua)
    : impl_(std::make_unique<Impl>(log, lua)) {
    if constexpr (bsp::kLeakMessageRouterDeliveryCc13Bound) g_leak92_units = impl_.get();
    // commands is owned by this Impl, so the pose provider outlives it.
    impl_->commands.bind_user_path_live_owner(this);
}

GameUnitsHost::~GameUnitsHost() {
    // Teardown callbacks may deliver commands; stop exposing live pose
    // before retiring unit state, so they retain the legacy fallback.
    impl_->commands.bind_user_path_live_owner(nullptr);
    if constexpr (bsp::kLeakMessageRouterDeliveryCc13Bound) {
        if (g_leak92_units == impl_.get()) g_leak92_units = nullptr;
    }
    Impl& host = *impl_;
    // Directors borrow units. Frame-level borrowers have already been released.
    host.gunnery.reset();
    if (host.observer_runtime == nullptr || host.slots.empty()) return;
    if (!host.observer_runtime->has_live_dispatch_owner()) {
        host.log.note("unit observer teardown requires the live application dispatch owner");
        std::terminate();
    }
    std::size_t destroyed = 0;
    // Groups borrow the same actual observed endpoints as directors. Retire
    // their stable callback bases before freeing any unit endpoint arrays.
    for (auto& group : host.formation_groups) {
        if (!group.observes_units) continue;
        host.observer_runtime->lifetime().destroy_callback_owner_00695870(
            group.native_members->observer_owner());
        group.observes_units = false;
    }
    for (const auto& slot : host.slots) {
        // Withdraw lookup before lifetime callbacks can re-enter this host.
        // Already borrowed aliases expire here; native array frees retain bytes.
        slot->observer_prefix_ready = false;
        // 0092589D precedes 009258AC: callback +10h, then observed +0h.
        // The slot and all other endpoint owners remain alive throughout.
        auto& lifetime = host.observer_runtime->lifetime();
        lifetime.destroy_callback_owner_00695870(slot->observer_prefix.callback_10);
        lifetime.destroy_observed_owner_00695760(slot->observer_prefix.observed_00);
        ++destroyed;
    }
    host.log.notef("unit observer teardown: units=%zu callback_then_observed=1 live_owner=1", destroyed);
}

void GameUnitsHost::bind_observer_runtime(GameObserverRuntime& runtime) {
    Impl& host = *impl_;
    if (!runtime.has_live_dispatch_owner())
        throw std::logic_error("unit observer binding requires a live dispatch owner");
    if (host.observer_runtime != nullptr && host.observer_runtime != &runtime)
        throw std::logic_error("unit observer runtime cannot change while the host lives");
    for (const auto& slot : host.slots) {
        if (!slot->observer_prefix_ready)
            throw std::logic_error("unit observer binding requires a known native creator");
    }
    host.observer_runtime = &runtime;
}

void GameUnitsHost::load_gameplay_settings_0083b5e0() {
    Impl& host = *impl_;
    // 007E2A20, the plane half of the same settings load. It is independent of
    // 0083B5E0 - a different singleton, its own two scripts, its own global -
    // and it runs before the rudder-curve guard because that guard is about
    // ShipGlobals and says nothing about whether the plane block is filled.
    // Idempotent: a second call re-runs the scripts and rewrites the block with
    // the same values.
    host.lua.load_plane_globals_007e2a20();
    // Packet cc13-goaway: 00901610 load_robot_config, which the image runs once
    // per process from 004DC6A0 (recorded, not run, by the mission frame). Here
    // it runs through the mission host's VFS script reader on its own Lua owner,
    // before any unit exists, and keeps the PilotBot rows ([00F8A30C]) for the
    // goaway enter's 009D0EA3 draw. A second call answers already_loaded.
    if constexpr (bsp::cc13_torp_goaway::kPilotBotConfigLoadBound) {
        bsp::cc13_torp_goaway::MissionScriptFiles files(host.lua);
        const bsp::cc13_torp_goaway::PilotBotLoadResult robots =
            bsp::cc13_torp_goaway::load_pilot_bot_config_00901610(files);
        if (!robots.already_loaded) {
            if (robots.loaded) {
                host.done("RobotConfig::load_00901610", 0x00901610u);
            } else {
                host.record("RobotConfig::load_00901610", 0x00901610u);
            }
            const auto* rows = bsp::cc13_torp_goaway::loaded_pilot_bot_rows();
            auto pair = [rows](int i, int k) {
                if (rows == nullptr) return 0.0;
                const bsp::PilotBotParameters& row = (*rows)[static_cast<std::size_t>(i)];
                return static_cast<double>(k == 1 ? row.torp_flik_flak_time_1_01c
                                                  : row.torp_flik_flak_time_2_020);
            };
            host.log.notef("pilot bot config (00901610, packet cc13-goaway): loaded=%d "
                "script reads=%zu misses=%zu error=%s; TorpFlikFlakTime by level "
                "Stun %.3g,%.3g SPNormal %.3g,%.3g SPVeteran %.3g,%.3g MPNormal %.3g,%.3g "
                "MPVeteran %.3g,%.3g Elite %.3g,%.3g",
                robots.loaded ? 1 : 0, robots.script_reads, robots.script_misses,
                robots.error.empty() ? "-" : robots.error.c_str(),
                pair(0, 1), pair(0, 2), pair(1, 1), pair(1, 2), pair(2, 1), pair(2, 2),
                pair(3, 1), pair(3, 2), pair(4, 1), pair(4, 2), pair(5, 1), pair(5, 2));
        }
    }
    if (host.rudder_curve_loaded) return;
    // 0083b5e0's head: run Scripts\datatables\ShipGlobals.lua and take the
    // `ShipGlobals` global. Then the fragment 0083ce56..0083d10d, which is what
    // fills +438h..+44Ch of the settings object 00424c40 hands out.
    const bool table = host.lua.load_ship_globals_0083b6e6();
    host.record("GameSettings::load_from_lua_globals", 0x0083b5e0u);
    if (!table) {
        host.log.notef("rudder curve: `ShipGlobals` did not load, so 00424c40()+438h..+44Ch "
            "keeps the zeroes a fresh settings object has and 0082e890 would divide "
            "MaxRotAngle by zero; the motion path is left with the curve unset");
        return;
    }
    // Milestone 2p: the AutoThrust half of the same loader, 0083cba8..0083ce3c,
    // read before the turn multipliers so an incomplete TurnMultipliers table
    // does not hide it. 009EC7C0 consumes seven of the eleven keys.
    if (host.lua.read_auto_thrust_0083cc2c(host.auto_thrust)) {
        host.auto_thrust_loaded = true;
        host.done("GameSettings::load_auto_thrust", 0x0083cc2cu);
        host.log.notef("auto thrust loaded from ShipGlobals[\"Navigator\"][\"AutoThrust\"]: "
            "hdg slow (%.4f, %.4f) thrust_min %.3f, hdg fast (%.4f, %.4f) thrust_min %.3f, "
            "danger mul %.3f; these are 00424c40()+6cch..+6ech, the block "
            "009ec7c0 BSP_UnitBot_ComputeThrottleCeiling interpolates over",
            static_cast<double>(host.auto_thrust.hdg_diff_value_min_slow),
            static_cast<double>(host.auto_thrust.hdg_diff_value_max_slow),
            static_cast<double>(host.auto_thrust.thrust_min_slow),
            static_cast<double>(host.auto_thrust.hdg_diff_value_min_fast),
            static_cast<double>(host.auto_thrust.hdg_diff_value_max_fast),
            static_cast<double>(host.auto_thrust.thrust_min_fast),
            static_cast<double>(host.auto_thrust.hdg_diff_danger_mul));
    } else {
        host.record("GameSettings::load_auto_thrust", 0x0083cc2cu);
        host.log.notef("auto thrust: ShipGlobals[\"Navigator\"][\"AutoThrust\"] is absent or "
            "incomplete, so 00424c40()+6cch..+6ech keeps the zeroes a fresh settings object "
            "has and 009ec7c0's five interpolation stages all run on zero endpoints");
    }

    // Packet cc9_unit_instance_step11: the engine-sound records 008252C0 reads.
    if (host.lua.read_engine_sound_smooth_rates_0083b5e0(host.engine_sound_rates.rate)) {
        host.engine_sound_rates.loaded = true;
        host.done("GameSettings::load_engine_sound_records", 0x008405d5u);
        host.log.notef("engine sound records loaded from ShipGlobals[\"Sounds\"]: "
            "EngineSoundSmoothRate Ship %.3f TBoat %.3f Submarine %.3f Plane %.3f "
            "(00424c40()+5c4h + i*24h, the rate 008252c0 steps +bc4h at)",
            static_cast<double>(host.engine_sound_rates.rate[0]),
            static_cast<double>(host.engine_sound_rates.rate[1]),
            static_cast<double>(host.engine_sound_rates.rate[2]),
            static_cast<double>(host.engine_sound_rates.rate[3]));
    } else {
        host.record("GameSettings::load_engine_sound_records", 0x008405d5u);
    }

    bsp::UnitRudderCurveSettings settings{};
    if (!host.lua.read_turn_multipliers_0083ce56(settings)) {
        host.log.notef("rudder curve: ShipGlobals[\"Navigator\"][\"TurnMultipliers\"] is "
            "incomplete, so the six fields stay unset");
        return;
    }
    host.rudder_curve = settings;
    host.rudder_curve_loaded = true;
    host.done("GameSettings::load_turn_multipliers", 0x0083ce56u);
    host.log.notef("rudder curve loaded from ShipGlobals[\"Navigator\"][\"TurnMultipliers\"]: "
        "min (%.3f, %.3f) med (%.3f, %.3f) max (%.3f, %.3f); index 1 is the throttle "
        "coordinate and index 2 the turn-circle multiplier 0082ecb0 divides MaxRotAngle by",
        static_cast<double>(settings.speed_0444), static_cast<double>(settings.value_0440),
        static_cast<double>(settings.speed_044c), static_cast<double>(settings.value_0448),
        static_cast<double>(settings.speed_043c), static_cast<double>(settings.value_0438));
}

// Packet cc9_land_convoy_members. 00743450 after its scene reads (the record
// carries them, src/game_hosts_scene_contents.cpp retain_land_convoy_roster):
// for each slot in order (00743799..00743B5C) a type below 1 pushes a null into
// +398h and makes nothing; otherwise one member. SUBSTITUTIONS (labelled): the
// member is created as a LandFort scene record whose `Type` is the
// LandVehicleClasses value, the route the load pass takes for a scene vehicle
// (creator 0074DF10); its property-bag copy 00922DE0 is not modelled; the
// convoy's live count +3BCh and member vector +398h are the returned count and
// the rows' back pointers.
std::size_t GameUnitsHost::build_land_convoy_roster_00743450(
    const GameSceneEntityRecord& convoy) {
    Impl& host = *impl_;
    if (!convoy.land_convoy_keys) return 0;
    std::vector<GameSceneEntityRecord> batch;
    std::vector<int> slots;
    for (std::size_t slot = 0; slot < convoy.convoy_slots.size(); ++slot) {
        const int type = convoy.convoy_slots[slot];
        if (type < 1) continue;                                   // 0074379C
        GameSceneEntityRecord member;
        member.name = convoy.name + "-" +  // 007438F6, 00742A70
            (bsp::kRomanMemberNamesBound
                ? bsp::roman_member_number_00742a70(static_cast<std::int32_t>(slot + 1))
                : std::to_string(slot + 1));
        member.class_name = "LandFort";
        member.class_id = 0x1b;
        member.type_table = "LandVehicleClasses";
        member.type_symbol = convoy.convoy_slot_symbols[slot];
        member.type_id = type;
        member.party = convoy.party;                              // +54h
        member.party_symbol = convoy.party_symbol;
        member.race = convoy.race;                                // +58h
        member.generated = true;
        member.created = true;
        std::memcpy(member.world, convoy.world, sizeof(member.world));   // vtable[98h]
        std::memcpy(member.local, convoy.local, sizeof(member.local));
        member.parent_scene_id = convoy.parent_scene_id;
        member.parent_name = convoy.parent_name;
        std::memcpy(member.parent_world, convoy.parent_world, sizeof(member.parent_world));
        batch.push_back(member);
        slots.push_back(static_cast<int>(slot));
    }
    const std::size_t before = host.slots.size();
    create_units(batch);
    const std::size_t made = host.slots.size() - before;
    for (std::size_t i = 0; i < made && i < slots.size(); ++i) {
        GameUnitRow& row = host.slots[before + i]->row;
        row.land_convoy_738 = convoy.name;                        // 00743A34
        row.land_convoy_slot_73c = slots[i];                      // +73Ch
    }
    if constexpr (kLandConvoyBodiesBound) {
        // Packet cc13-convoy: the member vector (a null per type below 1, as at
        // 0074379C), the live count (the members made, as above) and each
        // member's +738h, for cc15's 00742180 / 007422A0 and cc13's 00742210.
        // A second roster under a name already held (a roster built twice, or two
        // convoys sharing a name) is not tracked: every lookup here is by name, as
        // the motion state's is, and would only ever reach the first.
        bool duplicate = false;
        for (const auto& held : host.land_convoy_lives) duplicate = duplicate || held->name == convoy.name;
        if (duplicate) {
            host.log.notef("  LandConvoy roster \"%s\": a convoy of that name already has a "
                "member vector; this roster's %zu members are not tracked (packet cc13-convoy)",
                convoy.name.c_str(), made);
        } else {
            auto life = std::make_unique<LandConvoyLifeState>();
            life->name = convoy.name;
            life->slots.assign(convoy.convoy_slots.size(), static_cast<std::size_t>(-1));
            for (std::size_t i = 0; i < made && i < slots.size(); ++i) {
                life->slots[static_cast<std::size_t>(slots[i])] = before + i;
                life->members.push_back({before + i, life.get(), false});
            }
            life->live_3bc = static_cast<std::int32_t>(made);
            host.land_convoy_lives.push_back(std::move(life));
        }
    }
    host.log.notef("  LandConvoy roster 00743450: \"%s\" rows=%d columns=%d slots=%zu "
        "members=%zu path=\"%s\" speed=%.2f offset=%.2f (packet cc9_land_convoy_members)",
        convoy.name.c_str(), convoy.convoy_rows, convoy.convoy_columns,
        convoy.convoy_slots.size(), made, convoy.convoy_path.c_str(),
        static_cast<double>(convoy.convoy_speed), static_cast<double>(convoy.convoy_offset));
    for (std::size_t i = 0; i < made; ++i) {
        const GameUnitRow& row = host.slots[before + i]->row;
        host.log.notef("    member %s type %s (%d) slot %d pos=(%.1f, %.1f, %.1f)",
            row.name.c_str(), row.type_symbol.c_str(), row.type_id, row.land_convoy_slot_73c,
            static_cast<double>(row.position[0]), static_cast<double>(row.position[1]),
            static_cast<double>(row.position[2]));
    }
    host.done("LandConvoy::attach_and_build_roster", 0x00743450u);
    return made;
}

const std::string& GameUnitsHost::unit_land_convoy_738(std::size_t index) const {
    static const std::string none;
    if (index >= impl_->slots.size()) return none;
    return impl_->slots[index]->row.land_convoy_738;
}

int GameUnitsHost::unit_land_convoy_slot_73c(std::size_t index) const {
    if (index >= impl_->slots.size()) return -1;
    return impl_->slots[index]->row.land_convoy_slot_73c;
}  // namespace

// Packet cc9_land_convoy_movement. 007420B0 resolves "Path" through 00925A90
// (qualified, __strnicmp: "Landscape 01\<path>") and 007AC9D0; 007B34F0 then
// 007AF150 built the knots from the authored Point%002i Pos triples at the
// Path's load; 00742C10 sets +3C8h at 00742C70 from the two parents and zeroes
// the odometer at 00742C58. LABELLED: the qualified name is matched as
// "<parent name>\<leaf>" against the scene records, not walked through the
// registry's party lists; a convoy whose parent has no loaded height field is
// refused (the native would call through a null +3D0h).
bool GameUnitsHost::bind_land_convoy_motion(const GameSceneEntityRecord& convoy,
                                            const std::vector<GameSceneEntityRecord>& scene) {
    Impl& host = *impl_;
    if (!convoy.land_convoy_keys) return false;
    LandConvoyMotionState c;
    c.name = convoy.name;
    c.formation.rows = convoy.convoy_rows;
    c.formation.columns = convoy.convoy_columns;
    c.formation.row_gap = convoy.convoy_row_gap;
    c.formation.column_gap = convoy.convoy_column_gap;
    c.formation.speed = convoy.convoy_speed;
    c.formation.reverse = convoy.convoy_reverse;
    c.formation.stopped = false;                              // 00743450 clears +3A8h
    c.live_arc_3ac = convoy.convoy_offset;
    c.path_name = convoy.convoy_path;
    const std::size_t none = static_cast<std::size_t>(-1);
    c.member_units.assign(convoy.convoy_slots.size(), none);
    std::size_t members = 0;
    for (std::size_t u = 0; u < host.slots.size(); ++u) {
        const GameUnitRow& row = host.slots[u]->row;
        if (row.land_convoy_738 != convoy.name) continue;
        const int slot = row.land_convoy_slot_73c;
        if (slot < 0 || static_cast<std::size_t>(slot) >= c.member_units.size()) continue;
        c.member_units[static_cast<std::size_t>(slot)] = u;
        ++members;
    }
    const std::size_t split = c.path_name.find_last_of('\\');
    const std::string leaf = split == std::string::npos ? c.path_name : c.path_name.substr(split + 1);
    const std::string qualifier = split == std::string::npos ? std::string() : c.path_name.substr(0, split);
    const GameSceneEntityRecord* path = nullptr;
    for (const GameSceneEntityRecord& record : scene) {
        if (record.class_id != 0x47 || !land_name_equal(record.name, leaf)) continue;
        if (!qualifier.empty() && !land_name_equal(record.parent_name, qualifier)) continue;
        path = &record;
        break;
    }
    if constexpr (kLandConvoyBodiesBound) {
        // Packet cc13-convoy: 007420B0 itself, over the roster's member vector.
        if (LandConvoyLifeState* life = find_convoy_life(host, convoy.name)) {
            LandConvoyBindBinding bind(host, *this, convoy, path);
            bsp::cc15_land_convoy::land_convoy_bind_motion_007420b0(convoy_handle(*life), bind);
            host.log.notef("  LandConvoy \"%s\": 007420B0 path=%s skill=%d members re-skilled=%zu "
                "(006D1EB0, packet cc13-convoy)", convoy.name.c_str(),
                bind.path_interface() != nullptr ? "resolved" : "null", convoy.bag_skill,
                bind.skilled());
            host.done("LandConvoy::bind_motion_007420b0", 0x007420b0u);
        }
    }
    const char* refused = nullptr;
    if (members == 0) refused = "no members";
    else if (path == nullptr) refused = "Path does not resolve";
    else if (!path->path_points_retained) refused = "Path points not retained";
    if (refused == nullptr) {
        for (const auto& point : path->path_points_local) {
            bsp::CameraPathKnotWords k{};
            k[0] = point[0];
            k[1] = point[1];
            k[2] = point[2];
            c.knots.push_back(k);
        }
        c.totals = bsp::derive_camera_path_knots_007af150(c.knots, bsp::kWorldUp00f8758c);
        c.closed_24 = c.totals.closed_24 ? 1 : 0;
        std::memcpy(c.path_local_74, path->local, sizeof(c.path_local_74));
        c.parent_scene_id = convoy.parent_scene_id;
        std::memcpy(c.parent_world_cc, convoy.parent_world, sizeof(c.parent_world_cc));
        c.world_pose_law_3c8 = bsp::land_convoy_uses_world_pose_law_00742c70(
            path->parent_scene_id, convoy.parent_scene_id);
        c.formation.path_length = c.totals.length_28;
        if (!c.world_pose_law_3c8) {
            const SceneWorldClassLists& lists = scene_world_class_lists();
            for (const SceneWorldObject& object : lists.objects()) {
                if (object.scene_id == convoy.parent_scene_id && object.terrain) {
                    c.terrain = object.terrain;
                    break;
                }
            }
            if (!c.terrain) refused = "parent has no height field";
        }
        if (!(c.formation.path_length > 0.0f)) refused = "Path length is not positive";
    }
    host.log.notef("  LandConvoy motion: \"%s\" path=\"%s\" knots=%zu closed=%d length=%.3f "
        "law=%s members=%zu speed=%.2f reverse=%d%s%s (007AF150/00742C10, packet "
        "cc9_land_convoy_movement)", c.name.c_str(), c.path_name.c_str(), c.knots.size(),
        static_cast<int>(c.closed_24), static_cast<double>(c.totals.length_28),
        c.world_pose_law_3c8 ? "world" : "terrain", members,
        static_cast<double>(c.formation.speed), c.formation.reverse ? 1 : 0,
        refused != nullptr ? " REFUSED: " : "", refused != nullptr ? refused : "");
    if (refused != nullptr) return false;
    host.land_convoys.push_back(std::move(c));
    LandConvoyMotionState& kept = host.land_convoys.back();
    kept.knot_pointers.clear();
    for (bsp::CameraPathKnotWords& k : kept.knots) kept.knot_pointers.push_back(k.data());
    kept.knots_begin_08 = kept.knot_pointers.data();
    kept.knots_end_0c = kept.knot_pointers.data() + kept.knot_pointers.size();
    host.done("LandConvoy::bind_motion", 0x007af150u);
    return true;
}  // namespace

// Packet cc13-fix5 (docs/CC13_FIX_5.md section 6). 00741120 MOV byte [ECX+3A8h],0;
// 00741130 MOV byte [ECX+3A8h],1; 00741140 MOV [ECX+368h],arg; RET 4.
bool GameUnitsHost::set_land_convoy_stopped_3a8(const std::string& convoy, bool stopped) {
    for (LandConvoyMotionState& c : impl_->land_convoys) {
        if (c.name != convoy) continue;
        c.formation.stopped = stopped;
        return true;
    }
    return false;
}

bool GameUnitsHost::set_land_convoy_speed_368(const std::string& convoy, float speed) {
    for (LandConvoyMotionState& c : impl_->land_convoys) {
        if (c.name != convoy) continue;
        // 00741140 calls the recovered +368h setter; OFF keeps the direct host store.
        {
            // Typed projection: the body only writes speed_368. The argument is
            // Lua argument 2 from run_land_convoy_native (008959A3 CALL, RET 4).
            bsp::gameplay_seg41_cc13::ConvoyState view;
            bsp::gameplay_seg41_cc13::set_speed_00741140(view, speed);
            c.formation.speed = view.speed_368;
            impl_->done("LandConvoy::set_speed", 0x00741140u);
        }
        return true;
    }
    return false;
}

// Packet cc13-convoy. 009273A0's kill-list pass calls each killed entity's
// +080h (0092751F): the convoy's 00742180, a member's 0074CD10. The flush loops
// until both queues are empty (009275BA JMP 009273C0; the head at 009273C2 /
// 009273CA re-tests 00F899B0 / 00F899BC), so an entity killed by a +080h call
// is handled in the same flush, one pass later. Each pass here takes the
// convoys queued and the members newly dead at its start, runs the convoys'
// 00742180, then the members' 0074CD10, and the loop ends when a pass finds
// neither. So a member's 007422A0 that empties its convoy has the convoy's
// 00742180 run on the next pass, and members that 00742180 kills have their
// 0074CD10 (with +738h already cleared) on the pass after.
// SUBSTITUTION (labelled): a member's kill is read from the gunnery host's dead
// flag at the flush, not from the kill list; the host's on_killed_run flag
// stands in for 009274C8's +5Eh test (each entity's +080h runs once).
void GameUnitsHost::land_convoy_flush_009273a0() {
    if constexpr (kLandConvoyBodiesBound) {
        Impl& host = *impl_;
        GameGunneryHost* gunnery = host.gunnery.get();
        if (gunnery == nullptr) return;
        for (;;) {
            std::vector<LandConvoyLifeState*> convoys;
            std::vector<LandConvoyLifeState::Member*> members;
            for (auto& life : host.land_convoy_lives) {
                if (life->kill_queued) convoys.push_back(life.get());
                for (LandConvoyLifeState::Member& m : life->members) {
                    if (m.on_killed_run || m.unit >= host.slots.size() || !gunnery->unit_dead(m.unit)) continue;
                    members.push_back(&m);
                }
            }
            if (convoys.empty() && members.empty()) break;  // 009273D0 JE 009275BF
            if constexpr (kLandConvoyDeadWired) {
                // Packet cc13-dead: in the same 009273A0 pass, the whole destroy list
                // (0092747F) runs before any of the kill list's +080h (0092751F); each
                // convoy's 00926D90 queued its destroy at 00926E05 (0077D1A0 ->
                // 00926C80), whose +7Ch chain ends in 00929800. So every queued
                // convoy's Dead is written before the first 00742180. The rest of
                // the destroy chain is OMITTED: see land_convoy_dead_cc13.hpp.
                for (LandConvoyLifeState* life : convoys)
                    host.lua.publish_land_convoy_dead_00929800(life->name, life->cause_70);
            }
            for (LandConvoyLifeState* life : convoys) {
                life->kill_queued = false;
                LandConvoyKillBinding kill(host);
                bsp::cc15_land_convoy::land_convoy_kill_members_00742180(convoy_handle(*life), kill);
                host.done("LandConvoy::kill_members_00742180", 0x00742180u);
            }
            for (LandConvoyLifeState::Member* m : members) {
                m->on_killed_run = true;
                // The member: a shadow whose only word is +738h, all 0074CD10 reads.
                alignas(4) std::uint8_t shadow[0x73C] = {};
                std::uint8_t* const convoy =
                    m->convoy_738 != nullptr ? convoy_handle(*m->convoy_738) : nullptr;
                std::memcpy(shadow + 0x738, &convoy, sizeof(convoy));
                LandVehicleOnDeathBinding on_death(host, m->unit);
                bsp::cc13_seg41::land_vehicle_on_death_0074cd10(shadow, on_death);
            }
        }
    }
}

// Packet cc13-convoy. 008AC740 (IsKindOf 1Ah) -> 008AC74E 00742210(hard); the
// JMP at 008AC753 skips 008AC756's 00926D90, so the convoy is not killed.
bool GameUnitsHost::land_convoy_kill_member_vector_00742210(const std::string& convoy, bool hard,
                                                            std::vector<LandConvoyVictim>& victims) {
    if constexpr (!kLandConvoyBodiesBound) {
        static_cast<void>(convoy);
        static_cast<void>(hard);
        static_cast<void>(victims);
        return false;
    } else {
        Impl& host = *impl_;
        LandConvoyLifeState* life = find_convoy_life(host, convoy);
        if (life == nullptr) return false;
        LandConvoyKillVectorBinding binding(host, victims);
        bsp::cc13_seg41::land_convoy_kill_member_vector_00742210(convoy_handle(*life), hard, binding);
        host.log.notef("  LandConvoy \"%s\": 00742210 hard=%d killed and nulled %zu members, "
            "+738h kept, live count %d, convoy not killed (packet cc13-convoy)",
            convoy.c_str(), hard ? 1 : 0, victims.size(), static_cast<int>(life->live_3bc));
        host.done("LandConvoy::kill_member_vector_00742210", 0x00742210u);
        return true;
    }
}

void GameUnitsHost::land_convoy_step_waves(float step) {
    Impl& host = *impl_;
    for (LandConvoyMotionState& c : host.land_convoys) {
        land_convoy_advance_and_place(host, c, step);               // wave 1, +4h
        c.committed_arc_3b0 = bsp::land_convoy_commit_arc_007410b0(c.live_arc_3ac);  // +0Ch
        c.live_arc_3ac = bsp::land_convoy_restore_arc_007410c0(      // wave 3, +8h
            c.formation, c.committed_arc_3b0, step);
        ++c.steps;
        if (c.steps == 1 || c.steps % 600 == 0) {
            const std::size_t u = c.member_units.empty() ? static_cast<std::size_t>(-1)
                                                         : c.member_units.front();
            float x = 0.0f, y = 0.0f, z = 0.0f;
            if (u < host.slots.size()) {
                x = host.slots[u]->motion.position[0];
                y = host.slots[u]->motion.position[1];
                z = host.slots[u]->motion.position[2];
            }
            host.log.notef("LandConvoy step %zu \"%s\": arc=%.3f committed=%.3f odometer=%.3f "
                "placements=%zu slot0=(%.2f, %.2f, %.2f) range_errors=%zu (00743060/007410C0, "
                "packet cc9_land_convoy_movement)", c.steps, c.name.c_str(),
                static_cast<double>(c.live_arc_3ac), static_cast<double>(c.committed_arc_3b0),
                static_cast<double>(c.odometer_3b4), c.placements, static_cast<double>(x),
                static_cast<double>(y), static_cast<double>(z), c.range_errors);
        }
    }
    if (!host.land_convoys.empty()) host.done("LandConvoy::element_waves", 0x00743060u);
}

void GameUnitsHost::land_convoy_interpolation_wave(float leftover) {
    Impl& host = *impl_;
    for (LandConvoyMotionState& c : host.land_convoys) {
        land_convoy_advance_and_place(host, c, leftover);
    }
}

// Packet cc9_squadron_land_task. The squadron intake 007F1940's returntobase
// arm (007F1AF9): 007F16D0 with ECX = the squadron. The host's AI hands the
// order to each member plane (fan-out substitution, src/game_hosts_ai.cpp), so
// the resolution runs once per order, at the flight leader's. LABELLED inputs:
//  - the class descriptor sq+35Ch is the head's VehicleClass row (MinWaterSpd);
//  - sq+369h is ReloadEnabled, default 1 at 007F2D09; the authored key is not
//    read here, so it is taken as 1;
//  - head+C24h is not carried (read only for a kamikaze head);
//  - the home arm's 006BCD20/006C4790/006BED30 are unread: a squadron with a
//    scene HomeBase is recorded as "home arm unread", not issued;
//  - 006C0840: the head's scene parent (00923810(1)) is not carried by the unit
//    rows, so the own-site branch is not taken; every air-ops deck is one node
//    with owner +5Dh clear; 006BC530 and the ordering key are unread, so with
//    two or more decks past the filters the answer is flagged "key unread";
//    block+20h bit 1 is unread, so a head of class 10h/16h leaves the deck
//    undetermined;
//  - the retreat arm takes the nearest zone of the squadron's side from the
//    Lua host's world+7134h records, at the squadron's position.
void GameUnitsHost::Impl::record_return_to_base_007f16d0(std::size_t unit_index) {
    const bsp::PlaneSquadronHostRecord* sq =
        bsp::plane_squadron_registry().find_by_member_unit(unit_index);
    if (sq == nullptr) return;
    const std::size_t head = sq->flight_leader();
    if (head != unit_index || head >= slots.size()) return;
    const GameUnitRow& head_row = slots[head]->row;
    std::string note;
    bsp::ReturnToBaseInputs in;
    const GameVehicleClassRow cls = lua.read_vehicle_class_row(head_row.type_id);
    in.class_descriptor_35c = cls.found;
    in.min_water_spd_198 = cls.min_water_spd;
    in.head_present = true;
    in.head_is_kamikaze_17 = bsp::unit_is_kind_of(slots[head]->class_id, 0x17);
    in.head_c24 = bsp::cc13_kz_latch::kRtbHeadC24PilotFiresCc13Bound
        && slots[head]->plane_pilot_fires_c24;   // 007F171A, packet cc13-kzlatch
    // sq+369h ReloadEnabled: 007F1FE0 from the scene bag over the constructor's 1
    // (007F2D09); no scene in this installation authors it, so the substitution
    // shared with the other +369h readers (cc13-gpr38b).
    in.has_deck_369 = kLuaDeviceReloadEnabledBound;
    const bool home_authored = sq->squadron_unit < slots.size()
        && !slots[sq->squadron_unit]->scene_home_base_key.empty();
    in.home_arm_issues = false;
    // Packet cc9_rtb_home_arm: 007F1732-007F176E. squadron+404h is the deck owner
    // the air-ops bag names (`HomeBase`, 006C5050 -> 007F1C00) or the scene row's
    // HomeBase; 006BCD20(+404h, 1) answers its block (kind 9 +1188h, kind 45h
    // +72Ch) unless the owner is remote (+5Dh, never here); 006C4790 tests the
    // block+B4h list; 006BED30 answers true for block+1Ch, +1Dh, no owner or a
    // remote owner. LABELLED: a sunk home owner is taken as gone from +404h (the
    // image keeps the pointer). The +B4h exclusion list is the host's
    // base_unsupported_b4 under kLuaSquadronSetBaseUnsupportedBound, and empty
    // otherwise (packet cc13-luawire 7b).
    std::string home_name;
    std::size_t home_owner = slots.size();
    std::size_t home_deck = 0;
    if constexpr (bsp::kReturnToBaseHomeArmBound) {
        home_name = sq->from_air_ops_launch ? sq->bag_home_base : std::string();
        if (home_name.empty() && home_authored) {
            home_name = slots[sq->squadron_unit]->scene_home_base_key;
        }
        bsp::AirOpsDeckRegistry& hdecks = bsp::air_ops_decks();
        for (std::size_t i = 0; !home_name.empty() && i < hdecks.size(); ++i) {
            if (hdecks.name_at(i) != home_name) continue;
            home_owner = air_ops_owner_unit(home_name);   // kAirOpsOwnerByInstanceBound
            const bsp::AirOpsDeck* hd = hdecks.mutable_at(i);
            const bool sunk = home_owner < slots.size() && gunnery != nullptr
                && gunnery->unit_dead(home_owner);
            // 007F1748: 006C4790(block, squadron) before 006BED30.
            const bool excluded = squadron_excluded_006c4790(home_name, *sq);
            if (excluded) ++base_unsupported_refusals;
            if (home_owner < slots.size() && !sunk && hd->owner_present && !excluded
                && !hd->runway_failure && !hd->hangar_failure) {
                home_deck = i;
                in.home_arm_issues = true;
            }
            break;
        }
    } else if (home_authored) {
        note += " home-arm-unread";
    }
    // 006C0840(side = sq+54h, head, 0, 0047B850(head), 1).
    bsp::NearestLandingSiteInputs site_in;
    site_in.side = sq->party;
    site_in.head_present = true;
    site_in.head_control_9d4 = true;
    site_in.multiplayer_927c90 = false;
    site_in.need_approach_bit = bsp::unit_is_kind_of(slots[head]->class_id, 0x10)
        || bsp::unit_is_kind_of(slots[head]->class_id, 0x16);
    site_in.local_only = true;
    std::vector<std::size_t> dead;
    if (gunnery != nullptr) {
        for (const GameGunneryUnitRow& r : gunnery->unit_rows()) if (r.sunk) dead.push_back(r.unit_index);
    }
    std::vector<bsp::LandingSiteCandidate> list;
    bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
    bool bit_undetermined = false;
    for (std::size_t i = 0; i < decks.size(); ++i) {
        const bsp::AirOpsDeck* deck = decks.mutable_at(i);
        bsp::LandingSiteCandidate c;
        c.node = i + 1u;
        c.block_present = true;
        c.owner_present = deck->owner_present;
        const std::size_t owner = air_ops_owner_unit(decks.name_at(i));   // kAirOpsOwnerByInstanceBound
        c.owner_dead_5e = owner < slots.size()
            && std::find(dead.begin(), dead.end(), owner) != dead.end();
        c.owner_remote_5d = false;
        c.approach_bit_20 = true;
        if constexpr (kLandingApproachBitBound) {
            // Packet cc9_landing_approach_bit: block+20h is 3 from 006CAC00
            // (006CAC3D) and stays 3 for an airfield; 00758550 stores 1 for a
            // mother ship, so bit 1 is set only for an airfield owner.
            c.approach_bit_20 = deck->is_airfield;
        } else {
            if (site_in.need_approach_bit) bit_undetermined = true;
        }
        c.owner_side_54 = deck->owner_party;
        c.accepts_6bc530 = false;
        c.key_known = false;
        if constexpr (kReturnToBaseSiteKeyBound) {
            if (owner < slots.size()) landing_site_key_006c09fe(i, owner, *slots[head], c);
        }
        list.push_back(c);
    }
    const bsp::NearestLandingSiteResult site = bsp::nearest_landing_site_006c0840(site_in, list);
    if (site.key_unknown) note += " site-key-unread";
    if (bit_undetermined && site.node != 0) note += " approach-bit-20-unread";
    in.site_node = site.node;
    in.site_block_present = site.node != 0;
    in.site_block_refuses_6bc120 = false;   // owner present, +5Dh clear
    const bsp::BorderZoneSet* zones = lua.world_border_zones();
    float sx = 0.0f, sy = 0.0f, sz = 0.0f;
    const std::size_t at = sq->squadron_unit < slots.size() ? sq->squadron_unit : head;
    sx = slots[at]->motion.position[0];
    sy = slots[at]->motion.position[1];
    sz = slots[at]->motion.position[2];
    if (zones != nullptr) {
        const bsp::BorderZoneHit hit =
            bsp::closest_border_zone_004c7730(*zones, {sx, sy, sz}, sq->party);
        if (hit.zone != nullptr) {
            in.zone_found = true;
            const bsp::BorderZoneRecord& zr = *hit.zone;
            const float corners[8] = {zr.a[0], zr.a[2], zr.b[0], zr.b[2],
                                      zr.c[0], zr.c[2], zr.d[0], zr.d[2]};
            std::copy(corners, corners + 8, in.zone_corners_xz);
        }
    } else {
        note += " no-zones";
    }
    // Packet cc13-kzlatch, ON only: the retreat arm's 004C7730 result.
    if constexpr (bsp::cc13_kz_latch::kRtbHeadC24PilotFiresCc13Bound) {
        if (zones != nullptr) note += in.zone_found ? " zone-hit" : " zone-miss";
    }
    const bsp::ReturnToBaseResult r = bsp::resolve_return_to_base_007f16d0(in);
    const int arm = static_cast<int>(r.arm);
    ++rtb_arm_counts[arm];
    std::string site_name;
    if (r.arm == bsp::ReturnToBaseArm::kLandAtSite && site.node != 0) {
        site_name = decks.name_at(static_cast<std::size_t>(site.node - 1u));
    } else if (r.arm == bsp::ReturnToBaseArm::kLandAtHome && home_owner < slots.size()) {
        site_name = decks.name_at(home_deck);                       // 007F1000(+404h)
    }
    ReturnToBaseCensus* entry = nullptr;
    for (ReturnToBaseCensus& e : rtb_census) if (e.squadron == sq->name) entry = &e;
    if (entry == nullptr) {
        rtb_census.push_back(ReturnToBaseCensus{});
        entry = &rtb_census.back();
        entry->squadron = sq->name;
    }
    ++entry->resolutions;
    const bool changed = entry->last_arm != arm || entry->last_site != site_name
        || entry->last_note != note;
    entry->last_arm = arm;
    entry->last_site = site_name;
    entry->last_note = note;
    if (changed) {
        static const char* const kArm[4] = {"null", "land at home", "land at site", "retreat"};
        log.notef("returntobase 007F16D0: squadron \"%s\" head \"%s\" side=%d -> %s%s%s "
            "(MinWaterSpd=%.4f sites=%zu passed=%d%s; retreat point (%.1f, %.1f)) at %.2f s "
            "RECORD ONLY (packet cc9_squadron_land_task)", sq->name.c_str(),
            head_row.name.c_str(), sq->party, kArm[arm], site_name.empty() ? "" : " ",
            site_name.c_str(), static_cast<double>(in.min_water_spd_198), list.size(),
            site.candidates_passed, note.c_str(), static_cast<double>(r.retreat_x),
            static_cast<double>(r.retreat_z), static_cast<double>(summary.simulated_seconds));
    }
    // Packet cc9_land_task_reach: where the head is against the site when the
    // resolution changes (a record in both switch positions).
    if (changed && !site_name.empty()) {
        std::size_t owner = slots.size();
        for (std::size_t u = 0; u < slots.size(); ++u) {
            if (slots[u]->row.name == site_name) { owner = u; break; }
        }
        if (owner < slots.size()) {
            const float* hp = slots[head]->motion.position;
            const float* op = slots[owner]->motion.position;
            const double dx = static_cast<double>(op[0]) - hp[0];
            const double dz = static_cast<double>(op[2]) - hp[2];
            log.notef("returntobase 007F16D0 geometry: squadron \"%s\" head at (%.1f, %.1f, %.1f) "
                "site %s at (%.1f, %.1f, %.1f) planar %.1f m at %.2f s (packet cc9_land_task_reach)",
                sq->name.c_str(), static_cast<double>(hp[0]), static_cast<double>(hp[1]),
                static_cast<double>(hp[2]), site_name.c_str(), static_cast<double>(op[0]),
                static_cast<double>(op[1]), static_cast<double>(op[2]),
                std::sqrt(dx * dx + dz * dz), static_cast<double>(summary.simulated_seconds));
        }
    }
    record("Squadron::resolve_return_to_base_007f16d0", 0x007f16d0u);
}

// Packet cc9_land_task_reach (kSquadronLandTaskBound). docs/SQUADRON_LAND_TASK.md.
GameUnitsHost::Impl::LandTaskCensus& GameUnitsHost::Impl::land_census_for(
    const std::string& squadron) {
    for (LandTaskCensus& c : land_census) if (c.squadron == squadron) return c;
    land_census.push_back(LandTaskCensus{});
    land_census.back().squadron = squadron;
    return land_census.back();
}

// The member plane's bot intake for the squadron's `land`. In the image the
// squadron's 007F1940 issues the resolution (land, 00465080(site owner)) on
// the squadron's director; each member's bot retires its attack task once the
// task's predicate sees the command change (0099A4C0) and 0099A170 installs
// from the current command. The host places the order on every member plane
// (the fan-out substitution) and installs at that delivery, one bot tick early
// (docs/SENTITY_INIT_ATTACH_ORDER.md 22.7, the standing substitution).
//  - 0099A3DD-0099A415, the land arm: 006BCD20(target, 1) is the site's deck,
//    then 006C4790(deck, squadron) (006C4790-006C47EC): false for a null
//    squadron or when the squadron is on the deck's list at +B4h. The list is
//    base_unsupported_b4 under kLuaSquadronSetBaseUnsupportedBound (its producer
//    0089FAF0 -> 006CA730); otherwise it is taken as empty and the gate is true.
//  - A squadron that already flies `land` at the same site keeps its task:
//    009B3560 answers 1 for `land` with the same target, so 0099A4C0 never
//    retires it and a repeated SELLING placement installs nothing.
//  - REFUSED, counted: a resolution that carries an unread input (B-25 01's
//    approach bit 20h), a plane that is not airborne (its task would start in
//    land/park, 009B32BA, which is unbound), and a dead plane.
void GameUnitsHost::Impl::install_land_task_0099a3dd(std::size_t unit_index) {
    if (unit_index >= slots.size()) return;
    if constexpr (kTakeoffTaskHeadBound) {
        // Packet cc9_takeoff_task_head. The takeoff task is the head of bot+58h,
        // so 0099A170 builds no command task until it retires (0099A4C0 ->
        // 0099A5EB). SUBSTITUTION, labelled: the delivery is remembered and the
        // install runs at the takeoff task's done arm, from the command current
        // then (the delivery was `returntobase`; a later non-land order that
        // arrives in between is not re-checked).
        if (slots[unit_index] && slots[unit_index]->takeoff_task_installed) {
            if (!slots[unit_index]->land_install_waits_for_takeoff) ++land_installs_deferred;
            slots[unit_index]->land_install_waits_for_takeoff = true;
            return;
        }
    }
    const bsp::PlaneSquadronHostRecord* sq =
        bsp::plane_squadron_registry().find_by_member_unit(unit_index);
    if (sq == nullptr) return;
    const ReturnToBaseCensus* entry = nullptr;
    for (const ReturnToBaseCensus& e : rtb_census) if (e.squadron == sq->name) entry = &e;
    if (entry == nullptr) {
        // A member placed before its leader: the squadron's intake resolves
        // once per order, over the squadron, so resolve it now at the leader.
        record_return_to_base_007f16d0(sq->flight_leader());
        for (const ReturnToBaseCensus& e : rtb_census) if (e.squadron == sq->name) entry = &e;
        if (entry == nullptr) return;
    }
    LandTaskCensus& c = land_census_for(sq->name);
    ++c.placements;
    GameUnitSlot& unit = *slots[unit_index];
    auto refuse = [&](unsigned long long& counter, const std::string& why) {
        ++counter;
        if (c.last_refusal != why) {
            c.last_refusal = why;
            log.notef("land task REFUSED: squadron \"%s\" plane \"%s\": %s at %.2f s "
                "(packet cc9_land_task_reach)", sq->name.c_str(), unit.row.name.c_str(),
                why.c_str(), static_cast<double>(summary.simulated_seconds));
        }
    };
    // Packet cc9_plane_retreat_task: 007F16D0's retreat arm (007F1896) issues
    // `retreat` on the squadron; each member's bot takes 0099A45B.
    if (bsp::kPlaneRetreatTaskBound && entry->last_arm == 3) {
        if (install_retreat_task_009c9d00(unit_index, true)) ++retreat_rtb_installs;
        return;
    }
    // Packet cc9_rtb_home_arm: the home arm's `land` (007F1000) names the home
    // base itself, which this host installs like a site.
    const bool home_arm = bsp::kReturnToBaseHomeArmBound && entry->last_arm == 1;
    if (entry->last_arm != 2 && !home_arm) {
        refuse(c.refused_arm, "007F16D0 did not answer land at site");
        return;
    }
    if (entry->last_note.find("-unread") != std::string::npos) {
        refuse(c.refused_unread, "the resolution carries an unread input:" + entry->last_note);
        return;
    }
    bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
    std::size_t deck = decks.size();
    for (std::size_t i = 0; i < decks.size(); ++i) {
        if (decks.name_at(i) == entry->last_site) { deck = i; break; }
    }
    std::size_t owner = slots.size();
    for (std::size_t u = 0; u < slots.size(); ++u) {
        if (slots[u]->row.name == entry->last_site) { owner = u; break; }
    }
    if (deck >= decks.size() || owner >= slots.size()) {
        refuse(c.refused_arm, "the site has no deck or no unit row");
        return;
    }
    install_land_task_core_009b41c0(unit_index, *sq, owner, deck, entry->last_site, false);
}

// The part of the land arm after the site is known: the host refusals, the
// same-target keep (009B3560), 0099A4C0's retire and 009B41C0's construction.
// Shared by the 007F16D0 intake above and the explicit PilotLand entry.
bool GameUnitsHost::Impl::install_land_task_core_009b41c0(std::size_t unit_index,
    const bsp::PlaneSquadronHostRecord& sq_ref, std::size_t owner, std::size_t deck,
    const std::string& site_name, bool explicit_site) {
    const bsp::PlaneSquadronHostRecord* sq = &sq_ref;
    LandTaskCensus& c = land_census_for(sq->name);
    GameUnitSlot& unit = *slots[unit_index];
    auto refuse = [&](unsigned long long& counter, const std::string& why) {
        ++counter;
        if (c.last_refusal != why) {
            c.last_refusal = why;
            log.notef("land task REFUSED: squadron \"%s\" plane \"%s\": %s at %.2f s "
                "(packet cc9_land_task_reach)", sq->name.c_str(), unit.row.name.c_str(),
                why.c_str(), static_cast<double>(summary.simulated_seconds));
        }
    };
    // 0099A404 006C4790(deck, squadron), right after 006BCD20's deck: a squadron
    // on the deck's +B4h list gets no task (0099A40B). Packet cc13-luawire 7b.
    if (squadron_excluded_006c4790(site_name, *sq)) {
        ++base_unsupported_refusals;
        refuse(c.refused_arm, "006C4790: the squadron is on the deck's +B4h list "
            "(SquadronSetBaseUnsupported)");
        return false;
    }
    if (unit.plane_death_c3a) {
        refuse(c.refused_dead, "the plane is dead");
        return false;
    }
    if (unit.land_task_installed && unit.land_site_plus_one == owner + 1u
        && unit.land_deck_plus_one == deck + 1u) {
        ++c.kept;   // 009B3560: `land`, same target
        unit.land_explicit_site = explicit_site;
        return false;
    }
    // 009B32AA-009B32B8: (plane+72Ch)->vtable[38h], this host's free flight.
    const bool ground_start = unit.plane_control_mode_900 != 7;
    if (ground_start && !kLandTaskGroundParkStartBound) {
        refuse(c.refused_ground, "not airborne: the task would start in land/park (unbound)");
        return false;
    }
    record("Bot::install_command_task_land_arm", 0x0099a3ddu);
    record("AirOpsBlock::squadron_not_excluded_006c4790", 0x006c4790u);
    // 0099A4C0's retire: the old task goes, whatever it was.
    unit.dive_bomb_task_installed = false;
    unit.dive_bomb_state = bsp::DiveBombState::kNone;
    unit.dogfight_task_installed = false;
    unit.dogfight_state = bsp::DogfightState::kNone;
    unit.torpedo_task_installed = false;
    unit.torpedo_state = bsp::TorpedoState::kNone;
    unit.moveto_task_installed = false;
    unit.moveto_state = GameUnitSlot::MoveToTaskState::kNone;
    unit.level_bomb_task_installed = false;
    unit.level_bomb_state = bsp::LevelBombState::kNone;
    unit.attack_command_class = 0x00E08FA0u;        // `land`
    unit.command_target_plus_one = owner + 1u;      // 00465080(block+7Ch, 0.0)
    unit.land_command_target_plus_one = owner + 1u;
    // 009B41C0 -> 009B3240 -> 009B2E50 -> 009AFE70. The draws, in order.
    float angle = 0.20943952f, pitch = 0.10471976f;   // DEG(12), DEG(6)
    if (lua.plane_globals_loaded()) {
        angle = lua.plane_globals().pilot_landing_approach_angle;
        pitch = lua.plane_globals().pilot_landing_approach_pitch;
    }
    const std::string& n = unit.row.name;
    unit.land_angle_54 = stream_set2_draw_00bd2f10(unit, n + "#l54", 0.9f, 1.1f) * angle;
    unit.land_pitch_58 = stream_set2_draw_00bd2f10(unit, n + "#l58", 0.9f, 1.1f) * pitch;
    unit.land_speed_5c = static_cast<float>(static_cast<double>(unit.plane_travel_speed) * 0.75);
    unit.land_alt_60 = 0.0f;
    unit.land_timer_64 = 10.0f;
    unit.land_request_ac = 0.5f;
    unit.land_request_b0 = -stream_set2_draw_00bd2f10(unit, n + "#lb0", 0.0f, 0.5f);
    // 009B2EE4 009C2980, the follow state's construction draw.
    if constexpr (bsp::kFollowSightCountdownDrawCc13Bound) {
        unit.land_follow_stagger_74 = follow_stagger_shared_009c2a0a(unit, n + "#l74");
    } else {
    unit.land_follow_stagger_74 = -release_altitude_draw_00bd2f10(n + "#l74", 0.0f, 0.6f);
    }
    unit.land_mode_50 = 1;                          // 009AFEC7
    unit.land_site_plus_one = owner + 1u;
    unit.land_deck_plus_one = deck + 1u;
    unit.land_explicit_site = explicit_site;
    unit.land_task_installed = true;
    bot_task_ctor_seeds_0099c6f0(unit, false);
    if constexpr (kBotTaskCtorSeedsBound) unit.fs_countdown_74 = unit.land_follow_stagger_74;
    unit.land_profile_activation_pending = true;    // 0099AEB3
    unit.land_installed_at = summary.simulated_seconds;
    // 009B32C2-009B32DD: a flight leader (007B8AD0) starts in moveto (land),
    // a wing member in follow (land); then the state's enter, vtable[4].
    bool leader = false;
    if (ground_start) {
        // 009B32BA: land/park (task+620h), then its enter 009B21A0 (vtable[4]) directly:
        // +1Ch = 0.0, +18h = 0, +28h = 3.0 (00CE3854). The taxi target +20h/+24h is not
        // written by the image's constructor (allocation contents); the host keeps 0.0
        // (LABELLED: the first off-holder tick may count one retarget).
        unit.land_state = GameUnitSlot::LandTaskState::kPark;
        unit.land_park_1c = 0.0f;
        unit.land_done_18 = false;
        unit.land_park_timer_28 = 3.0f;
        ++c.ground_parked;
    } else {
        leader = unit_is_flight_leader_007b8ad0(unit_index);
        unit.land_state = leader ? GameUnitSlot::LandTaskState::kMoveTo
                                 : GameUnitSlot::LandTaskState::kFollow;
    }
    ++c.installs;
    if (c.first_install_at < 0.0) c.first_install_at = summary.simulated_seconds;
    const float* hp = unit.motion.position;
    const float* op = slots[owner]->motion.position;
    const double dx = static_cast<double>(op[0]) - hp[0];
    const double dz = static_cast<double>(op[2]) - hp[2];
    log.notef("land task install: plane \"%s\" squadron \"%s\" -> %s at site %s planar %.1f m "
        "alt %.1f angle54=%.4f pitch58=%.4f b0=%.3f at %.2f s (0099A3DD -> 009B41C0, "
        "packet cc9_land_task_reach)", n.c_str(), sq->name.c_str(),
        ground_start ? "park (land, not airborne)" : leader ? "moveto (land)" : "follow (land)",
        site_name.c_str(),
        std::sqrt(dx * dx + dz * dz), static_cast<double>(hp[1]),
        static_cast<double>(unit.land_angle_54), static_cast<double>(unit.land_pitch_58),
        static_cast<double>(unit.land_request_b0),
        static_cast<double>(summary.simulated_seconds));
    if (explicit_site) log.notef("  land task install source: explicit `land` command "
        "(PilotLand 008A47B0, packet cc9_pilot_land_native)");
    done("BotTaskLand::construct", 0x009b3240u);
    return true;
}

// 009B34D0 (009B34D0-009B3551), run first by the approach update 009B3900. The
// site stays only while the squadron's (approach+0Ch, plane+9D4h) current
// command, vtable[174h], is `land` (00E08FA0) whose target (vtable[178h] ->
// 00521EA0) is approach+30h, the block's owner exists with +5Dh clear, and
// 006C4790(block, squadron) passes; otherwise +2Ch/+30h/+34h are cleared. The
// squadron's current command is the host's last 007F16D0 answer for it.
// LABELLED: +5Dh is the owner's network-remote byte, clear in a single-player
// mission. 006C4790's list is base_unsupported_b4 under
// kLuaSquadronSetBaseUnsupportedBound, else empty (above). 009B3533 tests it
// last; the test has no side effect, so it is taken first here.
bool GameUnitsHost::Impl::land_command_still_valid_009b34d0(const GameUnitSlot& unit) const {
    if (unit.land_deck_plus_one == 0 || unit.land_site_plus_one == 0) return false;
    const bsp::PlaneSquadronHostRecord* sq =
        bsp::plane_squadron_registry().find_by_member_unit(unit.process_index);
    if (sq == nullptr) return false;   // 006C4790: false for a null squadron
    if (unit.land_deck_plus_one - 1u < bsp::air_ops_decks().size()
        && squadron_excluded_006c4790(
               bsp::air_ops_decks().name_at(unit.land_deck_plus_one - 1u), *sq)) {
        return false;                  // 009B3537 006C4790: listed at +B4h
    }
    if (unit.land_explicit_site) {
        // The plane's current command is still `land` with this site as its
        // target (vtable[174h] / vtable[178h] -> 00521EA0).
        const std::size_t target = kLandCommandOwnTargetBound
            ? unit.land_command_target_plus_one : unit.command_target_plus_one;
        return unit.land_site_plus_one - 1u < slots.size()
            && unit.attack_command_class == 0x00E08FA0u
            && target == unit.land_site_plus_one;
    }
    const ReturnToBaseCensus* entry = nullptr;
    for (const ReturnToBaseCensus& e : rtb_census) if (e.squadron == sq->name) entry = &e;
    // Packet cc9_rtb_home_arm: the home arm's `land` (007F1000) targets the home
    // base, so it keeps the task the same way.
    const bool home_arm = bsp::kReturnToBaseHomeArmBound && entry != nullptr
        && entry->last_arm == 1;
    if (entry == nullptr || (entry->last_arm != 2 && !home_arm)) return false;
    const std::size_t owner = unit.land_site_plus_one - 1u;
    if (owner >= slots.size()) return false;
    return slots[owner]->row.name == entry->last_site;
}

std::size_t GameUnitsHost::land_at_site_0099a3dd(std::size_t unit_index,
                                                 std::size_t site_index) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || site_index >= host.slots.size()) return 0;
    const std::string& site_name = host.slots[site_index]->row.name;
    // 0099A3E9 006BCD20(target, 1): the command target's deck. No deck, no task.
    bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
    std::size_t deck = decks.size();
    for (std::size_t i = 0; i < decks.size(); ++i) {
        if (decks.name_at(i) == site_name) { deck = i; break; }
    }
    // 0099A404 006C4790(deck, plane+9D4h): false for a null squadron.
    const bsp::PlaneSquadronHostRecord* sq =
        bsp::plane_squadron_registry().find_by_member_unit(unit_index);
    if (deck >= decks.size() || sq == nullptr) {
        host.log.notef("land command: unit \"%s\" -> \"%s\": %s; no task (0099A3F2 / "
            "0099A40B, packet cc9_pilot_land_native)",
            host.slots[unit_index]->row.name.c_str(), site_name.c_str(),
            deck >= decks.size() ? "the target has no deck" : "the plane has no squadron");
        return 0;
    }
    // The command reaches the planes: a squadron (this host fuses it with its
    // flight leader) through its director's fan-out to every member, a wing
    // plane alone through its own. Each member's bot then takes 0099A3DD.
    std::vector<std::size_t> planes;
    if (unit_index == sq->flight_leader()) {
        for (const std::size_t m : sq->member_units) {
            if (m != bsp::kPlaneSquadronNoUnit && m < host.slots.size()) planes.push_back(m);
        }
    } else {
        planes.push_back(unit_index);
    }
    std::size_t installed = 0;
    for (const std::size_t m : planes) {
        GameUnitSlot& plane = *host.slots[m];
        plane.attack_command_class = 0x00E08FA0u;          // `land`
        plane.command_target_plus_one = site_index + 1u;   // the command target
        plane.land_command_target_plus_one = site_index + 1u;
        ++host.land_census_for(sq->name).placements;
        if (host.install_land_task_core_009b41c0(m, *sq, site_index, deck, site_name, true)) {
            ++installed;
        }
    }
    host.log.notef("land command: unit \"%s\" -> \"%s\": %zu plane(s), %zu task(s) "
        "installed (008A47B0 -> 0099A3DD, packet cc9_pilot_land_native)",
        host.slots[unit_index]->row.name.c_str(), site_name.c_str(), planes.size(),
        installed);
    return installed;
}

// Packet cc9_plane_retreat_task. PilotRetreat 008A4300's world query: 004C7730
// over world+7134h with the unit's side (unit+54h), then 008A443E's corner mean.
bool GameUnitsHost::retreat_point_008a4300(std::size_t unit_index, float out[3]) const {
    const Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return false;
    const bsp::BorderZoneSet* zones = host.lua.world_border_zones();
    if (zones == nullptr) return false;
    const float* p = host.slots[unit_index]->motion.position;
    const bsp::BorderZoneHit hit = bsp::closest_border_zone_004c7730(
        *zones, {p[0], p[1], p[2]}, host.slots[unit_index]->row.party);
    if (hit.zone == nullptr) return false;
    bsp::PilotRetreatZoneCorners corners;
    const std::array<float, 3>* c[4] = {&hit.zone->a, &hit.zone->b, &hit.zone->c, &hit.zone->d};
    for (int i = 0; i < 4; ++i)
        for (int k = 0; k < 3; ++k) corners.corner[i][k] = (*c[i])[static_cast<std::size_t>(k)];
    bsp::pilot_retreat_position_008a443e(corners, out);
    return true;
}

// Packet cc9_plane_retreat_task. The bots' intake of `retreat` (00E08F90):
// 0099A170's arm 0099A45B -> 009CA2B0 on every member of a squadron order
// (the fan-out substitution land_at_site_0099a3dd states), or on the one plane.
std::size_t GameUnitsHost::retreat_0099a45b(std::size_t unit_index, const std::string& source) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return 0;
    ++host.retreat_orders;
    bsp::PlaneSquadronRegistry& reg = bsp::plane_squadron_registry();
    const bsp::PlaneSquadronHostRecord* sq = reg.find_by_member_unit(unit_index);
    bool whole = false;
    if (sq == nullptr) {
        for (const bsp::PlaneSquadronHostRecord& r : reg.records()) {
            if (r.squadron_unit == unit_index) { sq = &r; whole = true; break; }
        }
    } else {
        whole = unit_index == sq->flight_leader() || unit_index == sq->squadron_unit;
    }
    std::vector<std::size_t> planes;
    if (sq != nullptr && whole) {
        for (const std::size_t m : sq->member_units) {
            if (m != bsp::kPlaneSquadronNoUnit && m < host.slots.size()) planes.push_back(m);
        }
    } else {
        planes.push_back(unit_index);
    }
    std::size_t installed = 0;
    for (const std::size_t m : planes) {
        if (host.install_retreat_task_009c9d00(m, sq != nullptr && whole)) ++installed;
    }
    host.log.notef("retreat command: unit \"%s\" squadron \"%s\" whole=%d: %zu plane(s), %zu "
        "task(s) installed (%s -> 0099A45B, packet cc9_plane_retreat_task)",
        host.slots[unit_index]->row.name.c_str(), sq != nullptr ? sq->name.c_str() : "",
        whole ? 1 : 0, planes.size(), installed, source.c_str());
    return installed;
}  // namespace

std::size_t GameUnitsHost::Impl::landing_squadron_of(std::size_t plane) const {
    // plane+9D4h: the registry record holding the plane.
    const bsp::PlaneSquadronRegistry& reg = bsp::plane_squadron_registry();
    const bsp::PlaneSquadronHostRecord* r = reg.find_by_member_unit(plane);
    if (r == nullptr || reg.records().empty()) return static_cast<std::size_t>(-1);
    return static_cast<std::size_t>(r - reg.records().data());
}

std::vector<std::size_t> GameUnitsHost::Impl::landing_members(std::size_t squadron) const {
    // squadron+3D0h..+3E0h in array order, ending at the fifth. Never-created
    // entries are skipped, which is live_count()'s rule; a destroyed plane is
    // erased from the registry list by its own path.
    std::vector<std::size_t> out;
    const bsp::PlaneSquadronRegistry& reg = bsp::plane_squadron_registry();
    if (squadron >= reg.records().size()) return out;
    for (const std::size_t m : reg.records()[squadron].member_units) {
        if (m == bsp::kPlaneSquadronNoUnit || m >= slots.size()) continue;
        out.push_back(m);
        if (out.size() == 5u) break;
    }
    return out;
}

int GameUnitsHost::Impl::landing_spawn_index(std::size_t plane) const {
    // plane+9D8h, the member slot stamped at 007F4B43.
    const bsp::PlaneSquadronHostRecord* r =
        bsp::plane_squadron_registry().find_by_member_unit(plane);
    if (r == nullptr) return 0;
    for (std::size_t i = 0; i < r->member_units.size(); ++i) {
        if (r->member_units[i] == plane) {
            return i < r->member_spawn_index.size() ? r->member_spawn_index[i]
                                                    : static_cast<int>(i);
        }
    }
    return 0;
}

// 006C0D20 -> 006C0750 (holder C0h bytes), 006BEE40(0), 006BF0D0 and 006BC960,
// as 006D3C10 builds them for an airfield. Built once: an airfield does not move.
GameUnitsHost::Impl::LandingDeck* GameUnitsHost::Impl::landing_deck_006c0750(
    std::size_t deck_index, std::size_t owner) {
    bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
    if (landing_decks.size() < decks.size()) landing_decks.resize(decks.size());
    if (deck_index >= landing_decks.size() || owner >= slots.size()) return nullptr;
    LandingDeck& d = landing_decks[deck_index];
    if constexpr (bsp::kAirOpsOwnerByInstanceBound) {
        // 007593D0 builds each mother-ship instance its own holder and elevator.
        // A deck whose holder belongs to a dead instance is rebuilt for the live
        // one under its name. LABELLED: only when the stored owner is dead and
        // the new one is not, because planes pass the owner they were assigned
        // (land_site_plus_one), and an older one must not flap the holder back.
        if (d.built && d.owner != owner && gunnery != nullptr && gunnery->unit_dead(d.owner)
                && !gunnery->unit_dead(owner)) {
            d.built = false;
            d.refused = false;
            d.mother_ship = false;
            d.elevator = CarrierElevator{};
            d.site_stamp_40 = -99999.0f;
            d.site_occupants_34.clear();
            d.site_ready_18 = static_cast<std::size_t>(-1);
            d.queue.clear();
            d.assignments.clear();
            ++landing_deck_owner_rebuilds;
            log.notef("landing sequencer: deck %s holder rebuilt for owner unit %zu (the "
                "previous owner %zu is dead; 007593D0 per instance, packet cc13-fix7 A)",
                decks.name_at(deck_index).c_str(), owner, d.owner);
        }
    }
    if (d.built) {
        if constexpr (kCarrierLandingDeckBound) {
            // 00758E80: the carrier's update re-frames its holder (006BEE40).
            if (d.mother_ship && !d.refused && carrier_holder_frame_006bee40(d.owner, d)) {
                ++carrier_deck_refreshes;
            }
        }
        return d.refused ? nullptr : &d;
    }
    d.built = true;
    d.owner = owner;
    const bsp::AirOpsDeck* deck = decks.mutable_at(deck_index);
    if constexpr (kCarrierLandingDeckBound) {
        // 007593D0 (007593F1-00759493): 006C0D20(&class+814h, +820h, +824h) ->
        // 006C0750, then 006BEE40 and 006BC960's MotherShip arm.
        if (deck != nullptr && !deck->is_airfield && carrier_holder_frame_006bee40(owner, d)) {
            d.mother_ship = true;
            ++carrier_decks_built;
            const std::array<float, 3> tw = landing_t_world_006bca40(d);
            log.notef("landing sequencer: deck %s owner %s MOTHER-SHIP holder built: "
                "RunwayWidth=%.2f RunwayLength=%.2f heading=%.4f T local (%.2f, %.2f, %.2f) "
                "world (%.1f, %.1f, %.1f) (007593D0, 006C0750, 006BEE40, 006BC960; packet "
                "cc9_carrier_landing_deck)", decks.name_at(deck_index).c_str(),
                slots[owner]->row.name.c_str(), static_cast<double>(d.width_b0),
                static_cast<double>(d.length_b4), static_cast<double>(d.runway_heading_88),
                static_cast<double>(d.t_a4[0]), static_cast<double>(d.t_a4[1]),
                static_cast<double>(d.t_a4[2]), static_cast<double>(tw[0]),
                static_cast<double>(tw[1]), static_cast<double>(tw[2]));
            return &d;
        }
    }
    if (deck == nullptr || !deck->is_airfield || !deck->runway_from_scene) {
        d.refused = true;
        ++landing_refused_decks;
        log.notef("landing sequencer: deck %s REFUSED (%s), packet cc9_landing_sequencer",
            decks.name_at(deck_index).c_str(),
            deck == nullptr ? "no deck"
                : (!deck->is_airfield ? "a mother-ship holder is refreshed from the moving "
                                        "ship; that refresh is unread"
                                      : "the scene authored no RunwayWidth/RunwayLength"));
        return nullptr;
    }
    d.width_b0 = deck->runway_width;     // 006BF0D0: holder+B0h
    d.length_b4 = deck->runway_length;   // holder+B4h
    // 006BEE40(0): +8h..+47h = the owner's world matrix (+CCh); the zero offset
    // at +98h (006D3C10) is added to the translation row.
    const GameUnitSlot& o = *slots[owner];
    d.frame_8 = o.world;
    // 0085DEA0: copy, transpose the 3x3, translation = -0.0 - (t through the
    // transposed rotation, 0042D0D0 without normalising).
    // 0085DEA0: transpose_inverse_native_frame_0085dea0 builds holder+48h from the +8h
    // frame (both image-shaped 4x4 float matrices) with the -0.0 cell 00D7A208
    // (kNegativeZero_00d7a208, plane_pose_commit.hpp);
    // OFF keeps the inline transpose and negated translation.
    bsp::transpose_inverse_native_frame_0085dea0(d.inverse_48.data(), d.frame_8.data(),
                                                 &bsp::kNegativeZero_00d7a208);
    // +88h = owner->vtable[50h]() = 006D2040: atan2 over local row 2 (x, z); this
    // host's airfield is top level, so its world rows are the local rows.
    d.runway_heading_88 = static_cast<float>(std::atan2(static_cast<double>(d.frame_8[8]),
        static_cast<double>(d.frame_8[10])));
    // 006BC960, the airfield arm: block vtable[10h] = 006CA5A0 = 35.0 (00CE4D90).
    // 006BC960: the typed body computes T from the holder's +B0h/+B4h, the owner's
    // IsKindOf(9) and the airfield block's vtable[10h] (006CA5A0's constant 35.0,
    // 00CE4D90); OFF keeps the inline airfield arm.
    {
        bsp::cc13_seg29U::AirOpsHolderTouchdownInputs in{};
        in.length_b4 = d.length_b4;
        in.owner_is_kind_9 = bsp::unit_is_kind_of(o.class_id, bsp::kUnitKindQueryCarrier);
        if (!in.owner_is_kind_9) {
            in.width_b0 = d.width_b0;
            in.block_vslot_10 = 35.0;   // 006CA5A0 BSP_AirOpsBlock_RunwayInset35
        }
        bsp::cc13_seg29U::air_ops_holder_refresh_touchdown_006bc960(d.t_a4, in);
    }
    const std::array<float, 3> tw = landing_t_world_006bca40(d);
    log.notef("landing sequencer: deck %s owner %s holder built: RunwayWidth=%.2f "
        "RunwayLength=%.2f heading=%.4f T local (%.2f, %.2f, %.2f) world (%.1f, %.1f, %.1f) "
        "owner pos (%.1f, %.1f, %.1f) (006C0750, 006BEE40, 006BC960)",
        decks.name_at(deck_index).c_str(), o.row.name.c_str(),
        static_cast<double>(d.width_b0), static_cast<double>(d.length_b4),
        static_cast<double>(d.runway_heading_88), static_cast<double>(d.t_a4[0]),
        static_cast<double>(d.t_a4[1]), static_cast<double>(d.t_a4[2]),
        static_cast<double>(tw[0]), static_cast<double>(tw[1]), static_cast<double>(tw[2]),
        static_cast<double>(o.motion.position[0]), static_cast<double>(o.motion.position[1]),
        static_cast<double>(o.motion.position[2]));
    return &d;
}  // namespace

// 006BCC90 (RET 8): the position through the inverse (+48h), less T.
std::array<float, 3> GameUnitsHost::Impl::landing_local_006bcc90(const LandingDeck& d,
    const float* pos) const {
    // 006BCC90: the native body (004142E0 through +48h, then FSUB of +A4h..+ACh)
    // over the deck's image cells; OFF keeps the host's transform and subtraction.
    LandingHolderLocalImage holder{};
    holder.inverse_48 = d.inverse_48;
    holder.t_a4[0] = d.t_a4[0];
    holder.t_a4[1] = d.t_a4[1];
    holder.t_a4[2] = d.t_a4[2];
    const float source[3] = {pos[0], pos[1], pos[2]};
    std::array<float, 3> out{};
    bsp::native_air_ops_holder_local_less_t_006bcc90(&holder, nullptr, out.data(), source);
    return out;
}

// ---------------------------------------------------------------------------
// Packet cc9_plane_touchdown (kPlaneTouchdownBound), docs/SQUADRON_LAND_TASK.md
// section 5k. Every body below was read from the listing.
// ---------------------------------------------------------------------------

// 006BC530 (006BC530-006BC5CE, __thiscall(holder, pos, float* height), RET 8):
// the position through the inverse (+48h), NOT less T; *height = local y; true
// when |x| < float(B0h x 0.5) and |z| < float(B4h x 0.5) (the double 0.5 at
// 00D7A280, each product stored as a float before the compare).
bool GameUnitsHost::Impl::landing_over_runway_006bc530(const LandingDeck& d, const float* pos,
    float* height) const {
    const float hw = static_cast<float>(static_cast<double>(d.width_b0) * 0.5);
    const float hl = static_cast<float>(static_cast<double>(d.length_b4) * 0.5);
    const std::array<float, 3> l = landing_xform_004142e0(d.inverse_48, pos);
    if (height != nullptr) *height = l[1];
    return std::fabs(l[0]) < hw && std::fabs(l[2]) < hl;
}

// 006C0840 as 007C5AC0 calls it (007C5BAD-007C5BB9): ECX = plane+54h, EDX = the
// plane, stack (&dist, 0, airborne or on the water). The key, 006C09FE-006C0A9B:
// l = 006BCC90(pos) with y zeroed; an accepting holder (006BC530) keys on
// |00438B10(holder+88h, plane vtable[50h])| (0042BE90); any other on
// 00427E30(l) with l.z x 0.3 (00CE3DC8) when -1500 < l.z < 800 (00CF86A8,
// 00CE3948). 006C0AFF-006C0B2A: *dist = 0.0 for an accepting winner, else
// sqrt(key) (00BF7030); no winner leaves *dist alone. LABELLED: the plane's
// scene parent is not carried, so a flying plane never takes the own-site arm;
// a mother-ship holder is refused (landing_deck_006c0750); vtable[50h] is
// plane_heading_c6c.
std::size_t GameUnitsHost::Impl::plane_landing_site_006c0840(const GameUnitSlot& p, float& dist) {
    bsp::NearestLandingSiteInputs in;
    in.side = p.row.party;
    in.head_present = true;
    in.head_control_9d4 =
        bsp::plane_squadron_registry().find_by_member_unit(p.process_index) != nullptr;
    in.multiplayer_927c90 = false;
    in.need_approach_bit = false;
    in.local_only = p.plane_control_mode_900 == 7 || p.plane_control_mode_900 == 6;
    bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
    // 006C086D-006C0964, the own-site arm (kTakeoffOwnSiteBound): the plane's
    // scene parent (00923810(1)) answering IsKindOf(45h) (holder +7ACh) or else
    // IsKindOf(9) (holder +1208h), with its +5Eh clear, sets *dist = 0.0 and
    // returns its holder when side > 1 (unsigned), the parent's +54h is the side,
    // or single-player and the parent's side is 2; otherwise 0, with no key
    // compared. A dead parent falls through to the scan. LABELLED: the scene
    // parent is deck_parent_plus_one; the parent's own parent (+5Eh test at
    // 006C08A3 / 006C090A) is never carried; a refused holder answers 0.
    if constexpr (bsp::kTakeoffOwnSiteBound) {
        if (in.head_control_9d4 && p.deck_parent_plus_one != 0
            && p.deck_parent_plus_one <= slots.size()) {
            const std::size_t parent = p.deck_parent_plus_one - 1u;
            const GameUnitSlot& o = *slots[parent];
            const bool parent_dead = gunnery != nullptr && gunnery->unit_dead(parent);
            if (!parent_dead && (bsp::unit_is_kind_of(o.class_id, 0x45)
                    || bsp::unit_is_kind_of(o.class_id, 0x09))) {
                dist = 0.0f;
                const unsigned side = static_cast<unsigned>(in.side);
                if (side > 1u || static_cast<unsigned>(o.row.party) == side || o.row.party == 2) {
                    for (std::size_t i = 0; i < decks.size(); ++i) {
                        if (decks.name_at(i) != o.row.name) continue;
                        if (landing_deck_006c0750(i, parent) == nullptr) break;
                        ++takeoff_own_site_answers;
                        return i + 1u;
                    }
                }
                return 0;
            }
        }
    }
    std::vector<bsp::LandingSiteCandidate> list;
    for (std::size_t i = 0; i < decks.size(); ++i) {
        const std::size_t owner = air_ops_owner_unit(decks.name_at(i));   // kAirOpsOwnerByInstanceBound
        if (owner >= slots.size()) continue;
        const LandingDeck* d = landing_deck_006c0750(i, owner);
        if (d == nullptr) continue;
        const bsp::AirOpsDeck* deck = decks.mutable_at(i);
        bsp::LandingSiteCandidate c;
        c.node = i + 1u;
        c.block_present = true;
        c.owner_present = deck->owner_present;
        c.owner_dead_5e = gunnery != nullptr && gunnery->unit_dead(owner);
        c.owner_remote_5d = false;
        c.approach_bit_20 = deck->is_airfield;
        c.owner_side_54 = deck->owner_party;
        c.accepts_6bc530 = landing_over_runway_006bc530(*d, p.motion.position, nullptr);
        c.key_known = true;
        if (c.accepts_6bc530) {
            c.key = std::fabs(bsp::wrapped_angle_subtract_00438b10(d->runway_heading_88,
                p.plane_heading_c6c));
        } else {
            std::array<float, 3> l = landing_local_006bcc90(*d, p.motion.position);
            l[1] = 0.0f;
            if (l[2] > -1500.0f && 800.0 > static_cast<double>(l[2])) {
                l[2] = static_cast<float>(static_cast<double>(l[2]) * 0.30000001192092896);
            }
            if constexpr (bsp::kPc24UnitsHostCc13Bound) {
                // pc24w2-units: 006C0840 00427E30 (006C0A96): x*x (00427E3B), y*y (00427E3F),
                // FADDP, z*z (00427E45 FMUL ST0), FADDP, each at PC24; stored float (00427E49).
                namespace x8 = bsp::cc13x87;
                c.key = static_cast<float>(x8::x87_add24(
                    x8::x87_add24(x8::x87_mul24(l[0], l[0]), x8::x87_mul24(l[1], l[1])),
                    x8::x87_mul24(l[2], l[2])));
            } else {
            c.key = static_cast<float>(static_cast<double>(l[0]) * l[0]
                + static_cast<double>(l[1]) * l[1] + static_cast<double>(l[2]) * l[2]);
            }
        }
        list.push_back(c);
    }
    const bsp::NearestLandingSiteResult r = bsp::nearest_landing_site_006c0840(in, list);
    if (r.node != 0) {
        dist = r.best_accepts ? 0.0f
            : static_cast<float>(std::sqrt(static_cast<double>(r.best_key)));
    }
    return static_cast<std::size_t>(r.node);
}

// Packet cc9_rtb_site_key. 00759120 at 00759237-00759265 takes the FIRST point
// of the LAST Aux group named "runwaycenter" with index 0 (00718000); an empty
// group is the image's invalid-parameter path (00BF6713), refused here.
const GameUnitsHost::Impl::CarrierRunwayClass& GameUnitsHost::Impl::carrier_runway_class_00759120(
    const GameUnitSlot& s) {
    const int type_id = s.row.type_id;
    auto found = carrier_runway_classes.find(type_id);
    if (found != carrier_runway_classes.end()) return found->second;
    CarrierRunwayClass& e = carrier_runway_classes[type_id];
    e.width = lua.read_vehicle_class_number(type_id, "RunwayWidth", 0.0f);    // +820h
    e.length = lua.read_vehicle_class_number(type_id, "RunwayLength", 0.0f);  // +824h
    const std::string mesh = lua.read_vehicle_class_string(type_id, "Mesh");
    std::vector<std::uint8_t> bytes;
    std::vector<bsp::GunFirePointItem> items;
    std::string error;
    if (mesh.empty()) {
        e.reason = "no Mesh string";
    } else if (!lua.read_resource_file(mesh, bytes)) {
        e.reason = "model did not open";
    } else if (!bsp::read_mmod_aux_point_items_0071b3e0(bytes, items, error) && items.empty()) {
        e.reason = "model Aux read failed: " + error;
    } else {
        const bsp::GunFirePointItem* item =
            bsp::find_named_point_group_00718870(items, "runwaycenter", 0u);
        if (item == nullptr || item->points.empty()) {
            e.reason = "no runwaycenter point";
        } else {
            for (int k = 0; k < 3; ++k) e.center[k] = item->points.front()[static_cast<std::size_t>(k)];
            e.ok = true;
            e.reason = "runwaycenter";
        }
        if constexpr (kCarrierElevatorBound) {
            const bsp::GunFirePointItem* lift =
                bsp::find_named_point_group_00718870(items, "liftexitpoint", 2u);
            if (lift != nullptr && !lift->points.empty()) {
                for (int k = 0; k < 3; ++k) {
                    e.lift[k] = lift->points.front()[static_cast<std::size_t>(k)];
                }
                e.lift_ok = true;
            }
            log.notef("carrier lift class %d \"%s\": %s lift=(%.3f, %.3f, %.3f) (00759167, "
                "packet cc9_carrier_elevator)", type_id, s.row.name.c_str(),
                e.lift_ok ? "liftexitpoint" : "no liftexitpoint point",
                static_cast<double>(e.lift[0]), static_cast<double>(e.lift[1]),
                static_cast<double>(e.lift[2]));
            // 0075927C-00759287: class+80Ch = class+818h.
            if (e.lift_ok && e.ok) {
                log.notef("carrier lift class %d \"%s\": lift height %.3f, runway centre "
                    "height %.3f, %s (0075927C, packet cc9_carrier_lift_height)", type_id,
                    s.row.name.c_str(), static_cast<double>(e.lift[1]),
                    static_cast<double>(e.center[1]),
                    kCarrierLiftHeightFromRunwayBound ? "replaced" : "kept, unbound");
                if constexpr (kCarrierLiftHeightFromRunwayBound) e.lift[1] = e.center[1];
            }
        }
    }
    log.notef("carrier runway class %d \"%s\" mesh=%s: %s center=(%.3f, %.3f, %.3f) "
        "RunwayWidth=%.2f RunwayLength=%.2f (00759120 / 00759590, packet cc9_rtb_site_key)",
        type_id, s.row.name.c_str(), mesh.c_str(), e.reason.c_str(),
        static_cast<double>(e.center[0]), static_cast<double>(e.center[1]),
        static_cast<double>(e.center[2]), static_cast<double>(e.width),
        static_cast<double>(e.length));
    return e;
}

// The carrier holder at [unit+EF8h] as 00758E80 leaves it every update:
// 006BEE40 (006BEE40-006BEEBC, __thiscall, one unused float, RET 4) copies the
// owner's world matrix (+CCh) to +8h, adds 0042D0D0(offset +98h, frame, 0) -
// the offset through the frame's rotation rows, not normalised - to the
// translation (+38h..+40h), builds the inverse at +48h (0085DEA0) and stores
// owner vtable[50h] = 006DFD60, FLD [unit+1050h], at +88h. 006C0750 took
// +98h = class+814h.. and +B0h/+B4h = class+820h/+824h (007593D0 at
// 007593F1-00759493); 006BC960's MotherShip arm (IsKindOf(9) at 006BC985)
// sets T = (0, 0.5 [00CE3800], float(float(-B4h x 0.5 [00D7A280]) + 10.0
// [00CE3DC0])). The x87 sums are taken in double here (LABELLED: not
// extended precision).
bool GameUnitsHost::Impl::carrier_holder_frame_006bee40(std::size_t owner, LandingDeck& d) {
    if (owner >= slots.size()) return false;
    const GameUnitSlot& o = *slots[owner];
    const CarrierRunwayClass& cls = carrier_runway_class_00759120(o);
    if (!cls.ok) return false;
    d.width_b0 = cls.width;
    d.length_b4 = cls.length;
    d.frame_8 = o.world;
    const bsp::CameraMatrix& m = d.frame_8;
    const double ox = cls.center[0], oy = cls.center[1], oz = cls.center[2];
    float wx, wy, wz;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // pc24w1-sweep3-5: 0042D0D0's (v0c0 + v1c1) + v2c2 with each FMUL/FADD
        // rounded at PC24.
        namespace pc24 = bsp::cc13x87;
        auto row = [&](int i) {
            return static_cast<float>(pc24::x87_add24(
                pc24::x87_add24(pc24::x87_mul24(m[4 + i], oy), pc24::x87_mul24(m[i], ox)),
                pc24::x87_mul24(m[8 + i], oz)));
        };
        wx = row(0);
        wy = row(1);
        wz = row(2);
    } else {
    wx = static_cast<float>(m[4] * oy + m[0] * ox + m[8] * oz);
    wy = static_cast<float>(m[5] * oy + m[1] * ox + m[9] * oz);
    wz = static_cast<float>(m[6] * oy + m[2] * ox + m[10] * oz);
    }
    d.frame_8[12] = static_cast<float>(static_cast<double>(d.frame_8[12]) + wx);
    d.frame_8[13] = static_cast<float>(static_cast<double>(wy) + d.frame_8[13]);
    d.frame_8[14] = static_cast<float>(static_cast<double>(wz) + d.frame_8[14]);
    // 0085DEA0, as landing_deck_006c0750 builds it for an airfield.
    // 0085DEA0: transpose_inverse_native_frame_0085dea0 builds holder+48h from the +8h
    // frame (both image-shaped 4x4 float matrices) with the -0.0 cell 00D7A208
    // (kNegativeZero_00d7a208, plane_pose_commit.hpp);
    // OFF keeps the inline transpose and negated translation.
    bsp::transpose_inverse_native_frame_0085dea0(d.inverse_48.data(), d.frame_8.data(),
                                                 &bsp::kNegativeZero_00d7a208);
    // 006BEE40 obtains the carrier's vtable[50h], the same 006DFD60 field getter
    // used by unit_heading_vtable50. Borrow the carrier's motion-produced value.
    d.runway_heading_88 = bsp::pool_pool01_cc13::unit_instance_get_hull_heading_006dfd60(
        o.hull_heading_1050);
    // 006BC960: the typed body computes T from the class runway length (+B4h) and
    // the owner's IsKindOf(9); on that arm the body reads neither +B0h nor the
    // block's vtable[10h]. OFF keeps the inline MotherShip arm.
    {
        bsp::cc13_seg29U::AirOpsHolderTouchdownInputs in{};
        in.length_b4 = d.length_b4;
        in.owner_is_kind_9 = bsp::unit_is_kind_of(o.class_id, bsp::kUnitKindQueryCarrier);
        if (in.owner_is_kind_9) {
            bsp::cc13_seg29U::air_ops_holder_refresh_touchdown_006bc960(d.t_a4, in);
        } else {
            // LABELLED: an owner that is not IsKindOf(9) would take the body's
            // other arm through its own block's vtable[10h], which this host
            // does not model; the host's MotherShip arm is kept for it.
            const float half_len = static_cast<float>(-static_cast<double>(d.length_b4) * 0.5);
            d.t_a4[0] = 0.0f;
            d.t_a4[1] = 0.5f;
            d.t_a4[2] = static_cast<float>(static_cast<double>(half_len) + 10.0);
        }
    }
    return true;
}

// 006C09FE-006C0A9B for one candidate, with the record path's head plane:
// an accepting holder (006BC530 over the runway) keys on
// |00438B10(holder+88h, head vtable[50h])|; any other on the squared
// holder-local offset from T with y zeroed and z x 0.3 (00CE3DC8) inside
// (-1500, 800) (00CF86A8, 00CE3948), as plane_landing_site_006c0840 does.
// A candidate whose holder cannot be built keeps key_known = false.
bool GameUnitsHost::Impl::landing_site_key_006c09fe(std::size_t deck_index, std::size_t owner,
    const GameUnitSlot& head, bsp::LandingSiteCandidate& c) {
    const bsp::AirOpsDeck* deck = bsp::air_ops_decks().mutable_at(deck_index);
    if (deck == nullptr) return false;
    LandingDeck carrier;
    const LandingDeck* d = nullptr;
    if (deck->is_airfield) {
        d = landing_deck_006c0750(deck_index, owner);
    } else if (carrier_holder_frame_006bee40(owner, carrier)) {
        d = &carrier;
        ++rtb_site_keys_carrier;
    }
    if (d == nullptr) {
        ++rtb_site_keys_missing;
        return false;
    }
    c.accepts_6bc530 = landing_over_runway_006bc530(*d, head.motion.position, nullptr);
    c.key_known = true;
    if (c.accepts_6bc530) {
        c.key = std::fabs(bsp::wrapped_angle_subtract_00438b10(d->runway_heading_88,
            head.plane_heading_c6c));
    } else {
        std::array<float, 3> l = landing_local_006bcc90(*d, head.motion.position);
        l[1] = 0.0f;
        if (l[2] > -1500.0f && 800.0 > static_cast<double>(l[2])) {
            l[2] = static_cast<float>(static_cast<double>(l[2]) * 0.30000001192092896);
        }
        if constexpr (bsp::kPc24UnitsHostCc13Bound) {
            // pc24w2-units: the 006C09FE block of 006C0840, 00427E30 (006C0A96): x*x (00427E3B), y*y (00427E3F),
            // FADDP, z*z (00427E45 FMUL ST0), FADDP, each at PC24; stored float (00427E49).
            namespace x8 = bsp::cc13x87;
            c.key = static_cast<float>(x8::x87_add24(
                x8::x87_add24(x8::x87_mul24(l[0], l[0]), x8::x87_mul24(l[1], l[1])),
                x8::x87_mul24(l[2], l[2])));
        } else {
        c.key = static_cast<float>(static_cast<double>(l[0]) * l[0]
            + static_cast<double>(l[1]) * l[1] + static_cast<double>(l[2]) * l[2]);
        }
    }
    ++rtb_site_keys;
    return true;
}

// Packet cc9_moveto_arrival_end_command. 009C3100 (009C3100-009C3135, RET, ECX =
// approach = task+3F8h), read whole:
//   if (task->vtable[40h]() != 0)                       // 009C3103-009C3116
//       0071E430(squadron->vtable[114h](), 00E08F68, 0) // 009C3118-009C312E
// approach+0Ch = task+404h is the plane's squadron (unit+9D4h, 009F9CE0), whose
// vtable[114h] (007ECFD0) is its +348h command block. 009C31B0 (task vt[40h],
// 009C31B0-009C3288) answers 2 with no squadron, else 1 when the block's current
// command (0071BE40) is the moveto and either its target object (0071EB60 ->
// 00521EA0) is task+43Ch (approach+44h, the task's target) or, with no object,
// its point (006F7DD0) lies within 100 m (planar, [00CE3D64] = 10000.0) of the
// task's point (vtable[0Ch]); 0 otherwise. 0099A4C0, run when the command
// changes, keeps a single task only while its vtable[40h] answers 1. With
// terminal 0 the pairs show 0071E430 taking its queue-stage arm: the stage is
// raised and the moveto STAYS in slot 0, so 009C31B0 still answers 1 and the
// task is kept. Only a completion that leaves another command (or none) in
// slot 0 retires the kind-7 task. LABELLED:
//  - the block is this host's per-member command rows (the fan-out stand-in), so
//    009C31B0 is "the member's command class is still moveto": the task and the
//    command share their target by construction, and a re-issued moveto
//    re-installs the task with the new target;
//  - 0071E430 runs on each member's director, where the image runs it once on
//    the shared block;
//  - a squadron-less plane is refused (the image would read a null +0Ch);
//  - a promoted next command is counted, and its task is not installed here.
void GameUnitsHost::Impl::moveto_arrival_end_command_009c3100(GameUnitSlot& unit) {
    ++moveto_end_calls;
    const bsp::PlaneSquadronHostRecord* sq =
        bsp::plane_squadron_registry().find_by_member_unit(unit.process_index);
    if (sq == nullptr) {
        ++moveto_end_no_squadron;
        record("BotApproachMoveTo::arrived_vtable8", 0x009c3647u);
        return;
    }
    if (unit.attack_command_class != bsp::kPilotOrderClassMoveTo) {  // 009C31E0
        ++moveto_end_not_moveto;
        return;
    }
    ++moveto_end_commands;
    for (const std::size_t m : sq->member_units) {
        if (m == bsp::kPlaneSquadronNoUnit || m >= slots.size()) continue;
        GameUnitSlot& member = *slots[m];
        if (member.attack_command_class != bsp::kPilotOrderClassMoveTo) continue;
        const bool player = kPlayerRoleBookkeepingBound
            ? member.role_player_0184 : (controlled_bound && controlled_index == m);
        const GameCommandCompletion completion = commands.end_command_0071e430(m,
            bsp::kPilotOrderClassMoveTo, false, player, member.ring,
            pose_heading_radians(member));
        if (completion.promoted_command == bsp::kPilotOrderClassMoveTo) {
            // The stage arm: the command stays, 009C31B0 still answers 1.
            ++moveto_end_stage_only;
            if (!member.moveto_stage_logged) {
                member.moveto_stage_logged = true;
                log.notef("  moveto task %s: the arrival of %s raised squadron \"%s\"'s moveto "
                    "stage (009C3100 -> 0071E430 arm %s, stage %d); the command stays and so "
                    "does the task, at %.2f s", member.row.name.c_str(), unit.row.name.c_str(),
                    sq->name.c_str(), completion.arm.c_str(), completion.stage_after,
                    static_cast<double>(summary.simulated_seconds));
            }
            continue;
        }
        // 0099A4C0: 009C31B0 now answers 0, so the kind-7 task goes.
        member.moveto_task_installed = false;
        member.moveto_state = GameUnitSlot::MoveToTaskState::kNone;
        member.moveto_arrived_454 = false;
        member.attack_command_class = 0;
        ++moveto_end_retired;
        if (completion.promoted_command != 0) ++moveto_end_promoted;
        log.notef("  moveto task %s: the arrival of %s ended squadron \"%s\"'s moveto "
            "(009C3100 -> 0071E430 arm %s, next %08x) and retired the task (0099A4C0) at %.2f s",
            member.row.name.c_str(), unit.row.name.c_str(), sq->name.c_str(),
            completion.arm.c_str(), static_cast<unsigned>(completion.promoted_command),
            static_cast<double>(summary.simulated_seconds));
    }
    done("BotApproachMoveTo::arrival_end_command_009c3100", 0x009c3100u);
}

// ---------------------------------------------------------------------------
// Packet cc9_land_park_taxi (kLandParkStateBound), docs/SQUADRON_LAND_TASK.md
// section 5s. Every body below was read from the listing.
// ---------------------------------------------------------------------------

// 006CF420 (site vtable +2Ch, RET 8) and 006CF520 (+44h, RET 4): the last point
// of 006D2640's exit path (the hangar with the least airfield-frame Z among those
// whose object+370h > 0) or point 0 of 006D2780's entry path (the greatest Z, no
// condition gate), each through 007AF800 and the airfield's inverse (+110h, the
// same matrix as the holder's +48h here: the holder frame is the airfield's).
// Returns false when no hangar qualifies: 006CF420 then answers the plane's WORLD
// position (plane+FCh, not transformed; the caller handles it) and 006CF520 has
// no fallback (the image would read through a null path). SUBSTITUTIONS,
// labelled: object+370h > 0 is taken as "the hangar unit is not dead"; a path is
// the scene path registry's world points for the authored name.
// 006D2780 / 006D2640 over the deck's hangar records: the chosen hangar's entry
// or exit path (packet cc9_base_launch_deck_arms split this out of 006CF420 /
// 006CF520 so that 006CF730 can sample two points of the same path).
// Packet cc9_airfield_destruction_rule, docs/GUNNERY_OPEN_ITEMS.md 119. 006D40F0
// (inside Ghidra's 006D40D0, ends 006D4207 RET), __thiscall(unit), the airfield's
// vtable[1A8h], called by 006D2510 after 00953CC0 every fixed step.
// SUBSTITUTIONS, labelled: a hangar is the first unit named like its Object
// reference's last component, and "+370h > 0" is "not dead" (as
// landing_hangar_path_006d2780_006d2640); of message 7Dh only the +720h read at
// 00895E51 (AirOpsDeck::airfield_blocked) is modelled; 006D2980's park-slot
// rebuild (006D41F3) is not run, because the gunnery host's 006D3250 reads the
// hangar list live.
void GameUnitsHost::Impl::run_airfield_destruction_rule_006d40f0(std::size_t index) {
    GameUnitSlot& slot = *slots[index];
    if (gunnery == nullptr || gunnery->unit_dead(index)) return;  // 006D410B, +5Dh
    bsp::AirOpsDeck* deck = nullptr;
    for (std::size_t d = 0; d < bsp::air_ops_decks().size(); ++d) {
        if (bsp::air_ops_decks().name_at(d) == slot.row.name) {
            deck = bsp::air_ops_decks().mutable_at(d);
            break;
        }
    }
    // 006D4116-006D4158: a listed hangar with +370h > 0 returns at 006D41F8.
    if (deck != nullptr) {
        for (const bsp::AirOpsDeck::HangarNames& h : deck->hangars) {
            for (std::size_t u = 0; u < slots.size(); ++u) {
                if (slots[u]->row.name != h.object) continue;
                if (!gunnery->unit_dead(u)) return;
                break;
            }
        }
    }
    ensure_parent_link_71c_006f5cc0(index);
    if (slot.parent_link_71c > 0) {
        // 006D4167-006D41BB: "InferiorFailure" (00CF0B74) -> vtable[194h] 006D2210,
        // which matches neither RunwayFailure nor HangarFailure and falls to
        // 00953DA0: message 7Dh through 0077C2A0(msg, 7, 0); 0095AD0A sets +720h.
        if (deck != nullptr) deck->airfield_blocked = true;
        if (slot.inferior_failure_steps++ == 0) {
            log.notef("airfield destruction rule: unit=%s arm=inferior_failure t=%.2f "
                "(006D2210 -> 00953DA0 7Dh -> +720h, packet cc9_airfield_destruction_rule)",
                slot.row.name.c_str(), summary.simulated_seconds);
        }
        done("Airfield::destruction_rule_inferior_failure_006d40f0", 0x006d40f0u);
        return;
    }
    log.notef("airfield destruction rule: unit=%s arm=destroy t=%.2f (006D41E6 -> 0077D1A0, "
        "packet cc9_airfield_destruction_rule)", slot.row.name.c_str(),
        summary.simulated_seconds);
    gunnery->destroy_unit_0077d1a0(index, 0);  // 006D41EF, vtable[70h](0)
    done("Airfield::destruction_rule_destroy_006d40f0", 0x006d40f0u);
}

void GameUnitsHost::Impl::ensure_parent_link_71c_006f5cc0(std::size_t index) {
    GameUnitSlot& slot = *slots[index];
    if (slot.parent_link_71c == 0) {
        // 006F5CC0 pass 1, from every CommandBuilding's side: kind 45h here, the
        // building is not the entity, entity+724h set (006F5DCF), and the squared
        // distance (per-axis float differences, entity minus building; the sum
        // stored as a float) not above FILD(R * R) (006F5DC9). The last adopter wins.
        slot.parent_link_71c = -1;
        // 006F5D10 / 006F5D1F / 006F5D2E: only an entity answering IsKindOf 1Bh, 45h
        // or 46h is adopted (packet cc9_g51_inferior_failure; the airfield and
        // shipyard callers already met it).
        if (!bsp::unit_is_kind_of(slot.class_id, 0x1B) && !bsp::unit_is_kind_of(slot.class_id, 0x45)
            && !bsp::unit_is_kind_of(slot.class_id, 0x46)) return;
        if (slot.bag_min_level_724) {
            for (std::size_t c = 0; c < slots.size(); ++c) {
                const GameUnitSlot& cb = *slots[c];
                if (c == index || !bsp::unit_is_kind_of(cb.class_id, 0x1C)) continue;
                const float dx = slot.motion.position[0] - cb.motion.position[0];
                const float dy = slot.motion.position[1] - cb.motion.position[1];
                const float dz = slot.motion.position[2] - cb.motion.position[2];
                float d2;
                if constexpr (bsp::kPc24UnitsHostCc13Bound) {
                    // 006F5D97-006F5DB3: (dx*dx + dy*dy) + dz*dz, each op rounded.
                    using namespace bsp::cc13x87;
                    d2 = static_cast<float>(x87_add24(x87_add24(x87_mul24(dx, dx), x87_mul24(dy, dy)),
                                                      x87_mul24(dz, dz)));
                } else {
                    d2 = static_cast<float>(
                        (static_cast<double>(dx) * dx + static_cast<double>(dy) * dy)
                        + static_cast<double>(dz) * dz);
                }
                const std::int32_t r2 = static_cast<std::int32_t>(
                    static_cast<std::uint32_t>(cb.inferior_range_7c8)
                    * static_cast<std::uint32_t>(cb.inferior_range_7c8));
                if (static_cast<double>(d2) > static_cast<double>(r2)) continue;
                slot.parent_link_71c = static_cast<int>(c) + 1;
            }
        }
        log.notef("airfield destruction rule: unit=%s parent=%s min_level=%d t=%.2f "
            "(006F5CC0 pass 1 +71Ch, packet cc9_airfield_destruction_rule)",
            slot.row.name.c_str(),
            slot.parent_link_71c > 0
                ? slots[static_cast<std::size_t>(slot.parent_link_71c - 1)]->row.name.c_str()
                : "-",
            slot.bag_min_level_724 ? 1 : 0, summary.simulated_seconds);
    }
}

const bsp::game::ScenePathEntry* GameUnitsHost::Impl::landing_hangar_path_006d2780_006d2640(
    const LandingDeck& d, bool entry) const {
    const bsp::AirOpsDeck* deck = bsp::air_ops_decks().mutable_at(
        static_cast<std::size_t>(&d - landing_decks.data()));
    if (deck == nullptr || deck->hangars.empty()) return nullptr;
    std::vector<bsp::AirfieldHangarCandidate> cands(deck->hangars.size());
    std::vector<const bsp::game::ScenePathEntry*> paths(deck->hangars.size() * 2u, nullptr);
    for (std::size_t i = 0; i < deck->hangars.size(); ++i) {
        const bsp::AirOpsDeck::HangarNames& h = deck->hangars[i];
        std::size_t obj = slots.size();
        for (std::size_t u = 0; u < slots.size(); ++u) {
            if (slots[u]->row.name == h.object) { obj = u; break; }
        }
        bsp::AirfieldHangarCandidate& c = cands[i];
        if (obj >= slots.size()) continue;
        paths[2u * i] = bsp::game::scene_path_registry().find(h.entry_path);
        paths[2u * i + 1u] = bsp::game::scene_path_registry().find(h.exit_path);
        c.object_present = true;
        c.record.object = slots[obj].get();
        c.record.entry_path = paths[2u * i];
        c.record.exit_path = paths[2u * i + 1u];
        c.condition = (gunnery != nullptr && gunnery->unit_dead(obj)) ? 0.0f : 1.0f;
        c.local_z = landing_xform_004142e0(d.inverse_48, slots[obj]->motion.position)[2];
    }
    const bsp::HangarPathPick pick = bsp::pick_hangar_path_006d2780_006d2640(cands.data(),
        cands.size(), entry ? bsp::HangarPathKind::Entry : bsp::HangarPathKind::Exit);
    if (!pick.found || pick.path == nullptr) return nullptr;
    return static_cast<const bsp::game::ScenePathEntry*>(pick.path);
}

bool GameUnitsHost::Impl::landing_hangar_point_006cf420_006cf520(const LandingDeck& d,
    bool entry, std::array<float, 3>& out) const {
    const bsp::game::ScenePathEntry* path = landing_hangar_path_006d2780_006d2640(d, entry);
    if (path == nullptr || path->points_world.empty()) return false;
    const std::array<float, 3>* point = &path->points_world.front();
    if (!entry) {
        // 006CF472..006CF48B counts the selected exit path's point POINTERS,
        // then passes count-1 to 007AF800. The registry retains the sampler's
        // world-space outputs, not a native Path object. Borrow those SAME
        // points through a Win32 pointer-vector view for the existing index
        // slice; never pass the 12-byte coordinate-vector span as a 4-byte
        // pointer span, or transform the already-world-space points twice.
        static_assert(sizeof(void*) == 4);
        std::vector<const std::array<float, 3>*> points;
        points.reserve(path->points_world.size());
        for (const auto& world_point : path->points_world) points.push_back(&world_point);
        const int index = bsp::taxi_target_point_index_006cf420(
            reinterpret_cast<std::ptrdiff_t>(points.data()),
            reinterpret_cast<std::ptrdiff_t>(points.data() + points.size()));
        point = points[static_cast<std::size_t>(index)];
    }
    out = landing_xform_004142e0(d.inverse_48, point->data());
    return true;
}  // namespace

// 006CDF70 (006CDF70-006CE223, __stdcall(a, b), RET 8): true when b stands in
// a's way. A plane with +904h clear never yields to one with it set.
//   D = max(bomber(b) ? 12 : 2, (bomber(b) ? 5 : 2) x max(bomber(b) ? 1.8 : 0,
//       |v_a| - 4.1666667 (00CF8920))) + a.Length x 0.5
//   l = b's position in a's frame; R = D + b.Length x 0.5; z = l.z, forced to -1.0
//   when 0 <= z < 1 and a and b share a squadron with a's +9D8h below b's.
//   b landed, a not, a off any holder (006049F0): R = z + 10, x = max(0, |l.x - 10|)
//   (the zero guard 006CE16C compares against the double at 00CF8918, which is
//   a denormal ~5e-315, so it never takes the -1.0 arm).
//   true when z >= 0, R > z and |x| < b.Width x 0.8 + a.Width x 0.5.
// Length is class+A0h (motion_class.hull_length), Width class+A4h.
bool GameUnitsHost::Impl::landing_blocks_006cdf70(const GameUnitSlot& a,
    const GameUnitSlot& b) const {
    if (&a == &b) return false;
    if (!a.plane_landed_904 && b.plane_landed_904) return false;
    const float va = avoid_len(a.plane_world_velocity);                // vtable[38h]
    const float s1 = static_cast<float>(static_cast<double>(va) - 4.1666669845581055);
    const bool bb = park_bomber_class_0047b850(b);
    const float f = bb ? 1.7999999523162842f : 0.0f;                   // 00CF4848
    const float m = f > s1 ? f : s1;                                   // 00415550
    const float g = bb ? 5.0f : 2.0f;                                  // 00CE3850 / 00CE3958
    const float gm = g * m;
    const float h = bb ? 12.0f : 2.0f;                                 // 00CEB4B8 / 00CE3958
    const float mx = h > gm ? h : gm;
    const float D = static_cast<float>(static_cast<double>(mx) +
        static_cast<double>(a.motion_class.hull_length) * 0.5);
    const float hwa = static_cast<float>(0.5 * static_cast<double>(a.class_width_00a4));
    const std::array<float, 3> l = park_point_in_frame(a, b.motion.position);
    float R = static_cast<float>(static_cast<double>(b.motion_class.hull_length) * 0.5 +
        static_cast<double>(D));
    float z = l[2];
    float x = l[0];
    if (0.0f <= z && z < 1.0f && landing_squadron_of(a.process_index) ==
            landing_squadron_of(b.process_index)
        && landing_spawn_index(a.process_index) < landing_spawn_index(b.process_index)) {
        z = -1.0f;                                                     // 00D7A260
    }
    if (b.plane_landed_904 && !a.plane_landed_904 && a.plane_contact_deck_bf4 == 0) {
        R = static_cast<float>(static_cast<double>(z) + 10.0);
        const float e = std::fabs(static_cast<float>(static_cast<double>(l[0]) - 10.0));
        x = 0.0f > e ? 0.0f : e;
    }
    if (!(z >= 0.0f) || !(R > z)) return false;
    double lim;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // pc24w1-sweep3-6: 006CE200 FMUL 0.8, 006CE206 FADD the half width, each rounded
        // at PC24 before the 006CE20A compare.
        lim = bsp::cc13x87::x87_add24(bsp::cc13x87::x87_mul24(b.class_width_00a4, 0.800000011920929), hwa);
    } else {
        lim = static_cast<double>(b.class_width_00a4) * 0.800000011920929 + static_cast<double>(hwa);
    }
    return lim > static_cast<double>(std::fabs(x));
}

// 006CE610 (006CE610-006CE795, __thiscall(site, plane, ...)): false when an
// occupant o blocks the plane (006CDF70(plane, o)) and either does not yield
// back, or, when both block, o is ahead: its z' = L/2 - l.z (006BEFF0, the holder
// frame through +48h) is below the plane's by more than 1.5, or within 1.5 with
// |x'| no larger.
bool GameUnitsHost::Impl::landing_queue_clear_006ce610(const LandingDeck& d,
    const GameUnitSlot& a) const {
    const auto primed = [&](const float* w) {
        const std::array<float, 3> l = landing_xform_004142e0(d.inverse_48, w);
        return std::array<float, 2>{static_cast<float>(0.0 - l[0]),
            static_cast<float>(static_cast<double>(d.length_b4) * 0.5 - l[2])};
    };
    for (const std::size_t oi : d.site_occupants_34) {
        if (oi >= slots.size()) continue;
        const GameUnitSlot& o = *slots[oi];
        if (!landing_blocks_006cdf70(a, o)) continue;
        if (!landing_blocks_006cdf70(o, a)) return false;
        const std::array<float, 2> pa = primed(a.motion.position);
        const std::array<float, 2> po = primed(o.motion.position);
        if (po[1] < static_cast<float>(static_cast<double>(pa[1]) - 1.5)) return false;
        if (po[1] <= static_cast<float>(1.5 + static_cast<double>(pa[1]))
            && std::fabs(po[0]) <= std::fabs(pa[0])) {
            return false;
        }
    }
    return true;
}

// 006CF5B0 (006CF5B0-006CF728, site vtable +34h, RET 10h): with the enable byte,
// +904h set and +910h clear, a plane 15 to 35 + speed metres short of (x, z) in
// the airfield frame is refused when an occupant with +904h clear is within 30 m
// of z (the arrival traffic). Every other path ends in 006CE610.
bool GameUnitsHost::Impl::landing_spot_clear_006cf5b0(const LandingDeck& d,
    const GameUnitSlot& p, bool enable, float x, float z) const {
    (void)x;
    if (enable && p.plane_landed_904 && !p.plane_taxi_queue_910) {
        const std::array<float, 3> l = landing_xform_004142e0(d.inverse_48, p.motion.position);
        const float dz = l[2] - z;
        const float s = static_cast<float>(-35.0 - static_cast<double>(
            avoid_len(p.plane_world_velocity)));                    // 00CF8A50
        if (-15.0 > static_cast<double>(dz) && dz > s) {             // 00CE3D58
            for (const std::size_t oi : d.site_occupants_34) {
                if (oi >= slots.size() || oi == p.process_index) continue;
                const GameUnitSlot& o = *slots[oi];
                if (o.plane_landed_904) continue;
                const std::array<float, 3> ol =
                    landing_xform_004142e0(d.inverse_48, o.motion.position);
                if (30.0 > std::fabs(static_cast<double>(ol[2] - z))) return false;   // 00CE7630
            }
        }
    }
    return landing_queue_clear_006ce610(d, p);
}

// 007C16F0 / 007C1680: 4 -> 5 and 5 -> 4, +C04h = -1.0, and with classDesc+198h
// non-zero +904h = +908h > 5.0 and +C18h = 3. 007C11E0(0) (the actuator targets)
// is carried at 007C1680 under kPlaneFlightStateNotifyCc13Bound; 007C16F0's
// 0090F6C0 scoring arm is not carried.
void GameUnitsHost::Impl::plane_four_to_five_007c16f0(GameUnitSlot& p) {
    if (p.plane_control_mode_900 != 4) return;
    p.plane_control_mode_900 = 5;
    p.plane_site_timer_c04 = -1.0f;
    if (p.plane_min_water_spd_198 != 0.0f) {
        p.plane_landed_904 = p.plane_airborne_908 > 5.0f;
        p.plane_c18 = 3;
    }
    record("Plane::four_to_five_007c16f0", 0x007c16f0u);
}

void GameUnitsHost::Impl::plane_five_to_four_007c1680(GameUnitSlot& p) {
    if (p.plane_control_mode_900 != 5) return;
    p.plane_control_mode_900 = 4;
    p.plane_site_timer_c04 = -1.0f;
    if (p.plane_min_water_spd_198 != 0.0f) {
        p.plane_landed_904 = p.plane_airborne_908 > 5.0f;
        p.plane_c18 = 3;
    }
    if constexpr (bsp::kPlaneFlightStateNotifyCc13Bound) {
        flight_state_notify_007c11e0(p);                                // 007C16EA
        done("Plane::five_to_four_007c1680", 0x007c1680u);
    } else {
        record("Plane::five_to_four_007c1680", 0x007c1680u);
    }
}

// Diagnostic only (packet cc9_landing_follower_spacing): the first free-flight
// step of a plane in the ground-roll arm, with its runway-frame position against
// 006BC530's half extents and the deck the probe last chose.
void GameUnitsHost::Impl::log_ground_contact_loss(const GameUnitSlot& p) {
    const std::size_t deck = p.plane_contact_deck_bf4;
    float lx = 0.0f, ly = 0.0f, lz = 0.0f, hw = 0.0f, hl = 0.0f;
    if (deck != 0 && deck <= landing_decks.size()) {
        const LandingDeck& d = landing_decks[deck - 1u];
        const std::array<float, 3> l = landing_xform_004142e0(d.inverse_48, p.motion.position);
        lx = l[0];
        ly = l[1];
        lz = l[2];
        hw = static_cast<float>(static_cast<double>(d.width_b0) * 0.5);
        hl = static_cast<float>(static_cast<double>(d.length_b4) * 0.5);
    }
    // pc24-lint: allow host diagnostic (log_ground_contact_loss is log output only)
    const double v = std::sqrt(static_cast<double>(p.plane_world_velocity[0]) * p.plane_world_velocity[0]
        + static_cast<double>(p.plane_world_velocity[1]) * p.plane_world_velocity[1]
        + static_cast<double>(p.plane_world_velocity[2]) * p.plane_world_velocity[2]);
    log.notef("plane ground contact lost %s t=%.2f deck=%zu near=%d local=(%.2f %.2f %.2f) "
        "half=(%.2f %.2f) |v|=%.2f heading=%.4f state=%d (007C5F0A, 006BC530)",
        p.row.name.c_str(), static_cast<double>(summary.simulated_seconds), deck,
        p.plane_near_site_c02 ? 1 : 0, static_cast<double>(lx), static_cast<double>(ly),
        static_cast<double>(lz), static_cast<double>(hw), static_cast<double>(hl), v,
        static_cast<double>(p.plane_heading_c6c), p.plane_control_mode_900);
}

// 007C5AC0 (007C5AC0-007C5F5E, __thiscall(plane, float step), RET 4), the probe
// both motion arms run first. Coverage: complete except the +C1Ch gear request
// (007C5C11-007C5EEF, mission state 0Dh), which feeds the gear channel (+DECh
// +28h) this host does not carry; see plane_touchdown_007cc440.
void GameUnitsHost::Impl::plane_site_probe_007c5ac0(GameUnitSlot& p, float step) {
    // 007C5AC0-007C5AF1: +C04h -= min(step, 0.5); a negative step sets -1.0.
    if (step > 0.5f) {
        p.plane_site_timer_c04 = static_cast<float>(
            static_cast<double>(p.plane_site_timer_c04) - 0.5);
    } else if (0.0f > step) {
        p.plane_site_timer_c04 = -1.0f;
    } else {
        p.plane_site_timer_c04 = static_cast<float>(
            static_cast<double>(p.plane_site_timer_c04) - static_cast<double>(step));
    }
    const float pos[3] = {p.motion.position[0], p.motion.position[1], p.motion.position[2]};
    // 007C5B09-007C5B5E: only when +C04h < 0 and the unit is present (+5Ch) with
    // +5Dh, +60h and +5Eh clear (this host: not dead).
    const bool dead = p.plane_death_c3a || p.plane_death_removed ||
        (gunnery != nullptr && gunnery->unit_dead(p.process_index));
    if (0.0f > p.plane_site_timer_c04 && !dead) {
        ++p.td_probes;
        float dist = 0.0f;                                          // 007C5B73
        const std::size_t deck = plane_landing_site_006c0840(p, dist);
        p.plane_contact_deck_bf4 = deck;                            // 007B8E80
        if (deck != 0) {
            // 007C5BCE-007C5BF3: +C02h = 1000.0 (00CE47A0, double) > dist.
            p.plane_near_site_c02 = 1000.0 > static_cast<double>(dist);
        } else {
            // 007C5BF5-007C5C09.
            p.plane_ground_contact_bf8 = false;
            p.plane_near_site_c02 = false;
            p.plane_contact_height_bfc = 999.0f;                    // 00CF4888
        }
        // 007C5EF3-007C5F04: +C04h = (dist - 200.0) x 0.004 (00CE4D70, 00CEB160).
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // pc24w1-sweep3-6: 007C5EF8 FSUB, 007C5EFE FMUL, each rounded at PC24.
            p.plane_site_timer_c04 = static_cast<float>(
                bsp::cc13x87::x87_mul24(bsp::cc13x87::x87_sub24(dist, 200.0), 0.004000000189989805));
        } else {
            p.plane_site_timer_c04 = static_cast<float>(
                (static_cast<double>(dist) - 200.0) * 0.004000000189989805);
        }
    }
    // 007C5F0A-007C5F57, every step.
    if (p.plane_near_site_c02 && p.plane_contact_deck_bf4 != 0) {
        const LandingDeck& d = landing_decks[p.plane_contact_deck_bf4 - 1u];
        p.plane_ground_contact_bf8 =
            landing_over_runway_006bc530(d, pos, &p.plane_contact_height_bfc);
        return;
    }
    p.plane_near_site_c02 = false;
    p.plane_ground_contact_bf8 = false;
    p.plane_contact_height_bfc = 1000.0f;                           // 00CE3804
}

// 007CC440-007CC4CA, the live arm of 007CC2F0 after the probe, and what the C5h
// message does. Returns true when the plane touched down (the native returns at
// 007CC4CF and skips the water test).
//   gate: +BF8h, classDesc+1FCh >= +BFCh (007CC463 JB), +908h > 10.0
//   (00CE38B8), gear (+DECh)+28h clear or its value 1.0, and vtable[34h]
//   (007BBB70, the world velocity) y > -6.0 (00D05E30).
//   00762A60 builds C5h; 0077C2A0 routes it; 007CCFA0 (jump table 007CD1AC,
//   kind C5h) calls 007C71E0: +C49h clear, classDesc+198h != 0.0, +BF4h, the
//   gear again, the holder's owner not the scene parent -> re-parent,
//   007C1570(4, 0) -> C3h (current 7, requested 4) -> 007CCFA0 sub-kind 4 with
//   current 7 -> 007CB5F0: state 4, +C04h = -1.0, +904h = +908h > 5.0 and +C18h
//   = 3 (classDesc+198h != 0), 007C11E0(0), site vtable[24h] 006CED90 (append to
//   the occupancy vector unless present) and vtable[48h] 006CE230 (+40h = now).
// Goal 4 chain 338: both gear gates now borrow the live channel_b cells and
// call 007B8D70. Its class+5D4h animation producer is still unbound, so the
// existing channel retains the native constructor's disabled-byte default.
// SUBSTITUTIONS, labelled: +C49h is taken
// clear; both messages are delivered at once (the image drains the loopback
// queue at row 9 of the same fixed step); the re-parent is not carried (a
// static airfield); 0090F6C0 (scoring) and 007C11E0(0)'s actuator targets are
// contracts. After state 4 the ground-roll arm 007CBFA0 does not run in this
// host, so the plane is held where it touched down.
bool GameUnitsHost::Impl::plane_touchdown_007cc440(GameUnitSlot& p) {
    if (!p.plane_ground_contact_bf8) return false;
    if (++p.td_contact_steps == 1 && p.plane_contact_deck_bf4 != 0) {
        const std::array<float, 3> fl = landing_xform_004142e0(
            landing_decks[p.plane_contact_deck_bf4 - 1u].inverse_48, p.motion.position);
        log.notef("  plane ground contact first %s t=%.2f local x=%.2f y=%.2f z=%.2f vy=%.2f "
            "(006BC530, packet cc9_plane_touchdown)", p.row.name.c_str(),
            static_cast<double>(summary.simulated_seconds), static_cast<double>(fl[0]),
            static_cast<double>(fl[1]), static_cast<double>(fl[2]),
            static_cast<double>(p.plane_world_velocity[1]));
    }
    if (p.plane_contact_height_bfc < p.td_min_height) p.td_min_height = p.plane_contact_height_bfc;
    if (!(p.plane_wheel_height_1fc >= p.plane_contact_height_bfc)) return false;   // 007CC463 JB
    ++p.td_low_steps;
    if (!(p.plane_airborne_908 > 10.0f)) { ++p.td_refused_airborne; return false; }
    {
        // 007CC47F-007CC494 inline 007B8D70's gate before world velocity. These cells
        // belong to this plane's existing actuator block, which 007DE3A0 ticks.
        const auto& gear = p.actuator_block_dec.channel_b;
        if (bsp::cc13_seg47::plane_gear_is_down_007b8d70({
                static_cast<std::uint8_t>(gear.enabled), gear.value}) == 0) return false;
        if (!(p.plane_world_velocity[1] > -6.0f)) { ++p.td_refused_vy; return false; }
    }
    // 007CC4B6 posts C5h after these contact gates; 007CCFA0's C5h case
    // delivers 007C71E0. Dispatch the decoded local copy to this live plane.
    switch (commands.session_messages().queue_plane_touchdown_c5()) {
    case 0xc5: return deliver_plane_touchdown(p);
    default: return false;
    }
}

// The previous direct C5h delivery, extracted without changing its operations.
// Timing stays synchronous, as in the existing host; this is not the native
// row-9 FIFO or a reconstruction of the entire 007C71E0/007CB5F0 bodies.
bool GameUnitsHost::Impl::deliver_plane_touchdown(GameUnitSlot& p) {
    // 007C71E0 (007C71E6-007C721A).
    {
        // 007C71E6-007C723E: class+198h and holder precede the second gear
        // check. Re-read the receiving plane, rather than caching sender state.
        if (p.plane_min_water_spd_198 == 0.0f || p.plane_contact_deck_bf4 == 0) return false;
        const auto& gear = p.actuator_block_dec.channel_b;
        if (bsp::cc13_seg47::plane_gear_is_down_007b8d70({
                static_cast<std::uint8_t>(gear.enabled), gear.value}) == 0) return false;
    }
    LandingDeck& d = landing_decks[p.plane_contact_deck_bf4 - 1u];
    if constexpr (kCarrierDeckParentBound) {
        // 007C7244-007C7266: the holder's owner is not the scene parent. Only
        // a mother-ship owner is re-parented here (LABELLED: an airfield is a
        // static owner and its re-parent changes nothing in this host).
        if (d.mother_ship && d.owner < slots.size() && slots[d.owner]
            && p.deck_parent_plus_one != d.owner + 1u) {
            carrier_deck_reparent_007c71e0(p, d);
        }
    }
    // 007CB5F0, the authority arm (007CB6A6-007CB75D).
    const float airborne = p.plane_airborne_908;
    p.plane_control_mode_900 = 4;
    p.plane_site_timer_c04 = -1.0f;
    p.plane_landed_904 = airborne > 5.0f;                            // 00CE3850
    p.plane_c18 = 3;
    if constexpr (bsp::kPlaneFlightStateNotifyCc13Bound) {
        flight_state_notify_007c11e0(p);                             // 007CB733
    }
    if (std::find(d.site_occupants_34.begin(), d.site_occupants_34.end(), p.process_index)
        == d.site_occupants_34.end()) {
        d.site_occupants_34.push_back(p.process_index);              // 006CED90
    }
    d.site_stamp_40 = static_cast<float>(summary.simulated_seconds);   // 006CE230
    ++p.td_touchdowns;
    p.td_at = summary.simulated_seconds;
    const std::array<float, 3> l = landing_xform_004142e0(d.inverse_48, p.motion.position);
    log.notef("plane touchdown: unit=%s t=%.2f height=%.3f wheel=%.2f vy=%.2f |v|=%.2f "
        "local x=%.1f z=%.1f airborne=%.1f landed904=%d state 7 -> 4 (007CC4B6 C5h -> "
        "007C71E0 -> 007C1570(4) -> C3h -> 007CB5F0; packet cc9_plane_touchdown)",
        p.row.name.c_str(), summary.simulated_seconds,
        static_cast<double>(p.plane_contact_height_bfc),
        static_cast<double>(p.plane_wheel_height_1fc),
        static_cast<double>(p.plane_world_velocity[1]),
        static_cast<double>(avoid_len(p.plane_world_velocity)),
        static_cast<double>(l[0]), static_cast<double>(l[2]),
        static_cast<double>(airborne), p.plane_landed_904 ? 1 : 0);
    record("Plane::touchdown_007cb5f0", 0x007cb5f0u);
    // 007CB75F-007CB778: when the pilot role's holder (+1B0h) is not 8 and
    // 00927F10 says a human holds that party slot, the image calls 007C3EA0
    // (land command 00E08FA0 to the squadron, 0099A600, 0077C470(2, 0), message
    // 57h). CENSUS ONLY: this host gives role 1 to the local player only in the
    // scripted helm transfer, so no plane reaches it (SQUADRON_LAND_TASK 5hw.4).
    if (p.current_roles_01ac[1] != bsp::kUnitRoleTableFill
            && p.current_roles_01ac[1] == kLocalPlayerSlot) {
        ++touchdown_human_pilot;
        log.notef("plane touchdown: %s pilot role held by the local player: 007C3EA0 not carried "
            "(007CB778 census)", p.row.name.c_str());
    }
    return true;
}  // namespace

// The local pose (unit+74h under the parent) from the world pose.
void GameUnitsHost::Impl::carrier_deck_capture(GameUnitSlot& p) {
    if (p.deck_parent_plus_one == 0 || p.deck_parent_plus_one > slots.size()) return;
    const GameUnitSlot* o = slots[p.deck_parent_plus_one - 1u].get();
    if (o == nullptr) return;
    deck_to_local(o->world, p.motion.position, true, p.deck_local_pos);
    deck_to_local(o->world, p.motion.pose_row0, false, p.deck_local_rows + 0);
    deck_to_local(o->world, p.motion.pose_row1, false, p.deck_local_rows + 3);
    deck_to_local(o->world, p.motion.pose_row2, false, p.deck_local_rows + 6);
    deck_to_local(o->world, p.plane_world_velocity, false, p.deck_local_vel);
}

// The world pose from the local pose and the carrier's current world matrix:
// what the image's derived pose (00414E10, parent x local) gives at the step.
void GameUnitsHost::Impl::carrier_deck_carry_in(GameUnitSlot& p) {
    if (p.deck_parent_plus_one == 0 || p.deck_parent_plus_one > slots.size()) return;
    const GameUnitSlot* o = slots[p.deck_parent_plus_one - 1u].get();
    if (o == nullptr) return;
    deck_to_world(o->world, p.deck_local_pos, true, p.motion.position);
    deck_to_world(o->world, p.deck_local_rows + 0, false, p.motion.pose_row0);
    deck_to_world(o->world, p.deck_local_rows + 3, false, p.motion.pose_row1);
    deck_to_world(o->world, p.deck_local_rows + 6, false, p.motion.pose_row2);
    deck_to_world(o->world, p.deck_local_vel, false, p.plane_world_velocity);
    ++carrier_deck_carry_steps;
}

// The point in the carrier's model frame: what plane+A4h..+ACh holds under the
// carrier (007C71E0's re-parent), from the world point and the carrier's
// current matrix.
std::array<float, 3> GameUnitsHost::Impl::carrier_local_point(const LandingDeck& d,
    const float* world) const {
    std::array<float, 3> l{};
    if (d.owner < slots.size() && slots[d.owner]) {
        deck_to_local(slots[d.owner]->world, world, true, l.data());
    }
    return l;
}

// 006CFAF0 / 006FC380: the platform from the lift point, ElevatorSpeed (+44h),
// ElevatorDepth (+48h), +1Ch = depth x 0.1 (double 00D7A3A0), +18h = -+1Ch.
GameUnitsHost::Impl::CarrierElevator* GameUnitsHost::Impl::carrier_elevator_ready(
    LandingDeck& d) {
    CarrierElevator& e = d.elevator;
    if (e.built) return e.refused ? nullptr : &e;
    e.built = true;
    if (!d.mother_ship || d.owner >= slots.size() || !slots[d.owner]) {
        e.refused = true;
        return nullptr;
    }
    const CarrierRunwayClass& cls = carrier_runway_class_00759120(*slots[d.owner]);
    float speed = 0.0f, depth = 0.0f;
    const bool s1 = lua.read_global_nested_number("ShipGlobals", "MotherShip",
        "ElevatorSpeed", speed);
    const bool s2 = lua.read_global_nested_number("ShipGlobals", "MotherShip",
        "ElevatorDepth", depth);
    if (!cls.lift_ok || !s1 || !s2) {
        e.refused = true;
        log.notef("carrier elevator %s: REFUSED lift=%d speed=%d depth=%d (006CFAF0, packet "
            "cc9_carrier_elevator)", slots[d.owner]->row.name.c_str(), cls.lift_ok ? 1 : 0,
            s1 ? 1 : 0, s2 ? 1 : 0);
        return nullptr;
    }
    for (int k = 0; k < 3; ++k) e.lift[k] = cls.lift[k];
    e.speed_44 = speed;
    e.depth_48 = depth;
    e.p1c = static_cast<float>(static_cast<double>(depth) * 0.1);   // 006FC3D0
    e.p18 = -e.p1c;
    e.p4c = 1.0f;
    e.mode_50 = 0;
    log.notef("carrier elevator %s: lift=(%.3f, %.3f, %.3f) speed=%.2f depth=%.2f (006CFAF0 / "
        "006FC380, packet cc9_carrier_elevator)", slots[d.owner]->row.name.c_str(),
        static_cast<double>(e.lift[0]), static_cast<double>(e.lift[1]),
        static_cast<double>(e.lift[2]), static_cast<double>(speed), static_cast<double>(depth));
    return &e;
}

// 006CFE90 (RET 4): the plane's point (0, 0, planeDesc+158h) through its local
// matrix (unit+74h), then the x/z distance to the lift (P+0Ch/+14h, site+50h/
// +58h); 0.0 below the 1e-10 square (00CE3820).
float GameUnitsHost::Impl::carrier_elevator_nose_distance_006cfe90(const LandingDeck& d,
    const GameUnitSlot& p) const {
    // The point (0, 0, planeDesc+158h) in the plane's frame: its origin plus
    // +158h along its forward row (pose row 2, the frame unit+74h carries).
    const float c = const_cast<Impl*>(this)->plane_desc_158(p);
    const float nose[3] = {p.motion.position[0] + p.motion.pose_row2[0] * c,
        p.motion.position[1] + p.motion.pose_row2[1] * c,
        p.motion.position[2] + p.motion.pose_row2[2] * c};
    const std::array<float, 3> l = carrier_local_point(d, nose);
    const float dx = l[0] - d.elevator.lift[0];
    const float dz = l[2] - d.elevator.lift[2];
    float out;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // pc24w1-sweep3-6: 006CFEED..006CFEF3 squares and sum, FSTP float, the 1e-10
        // floor and 00BF7030's FSQRT, each op rounded at PC24.
        const float s = static_cast<float>(bsp::cc13x87::x87_add24(bsp::cc13x87::x87_mul24(dx, dx), bsp::cc13x87::x87_mul24(dz, dz)));
        out = static_cast<double>(s) > 1e-10 ? static_cast<float>(bsp::cc13x87::x87_sqrt24(s)) : 0.0f;
    } else {
        const float s = static_cast<float>(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
        out = static_cast<double>(s) > 1e-10 ? static_cast<float>(std::sqrt(static_cast<double>(s)))
                                             : 0.0f;
    }
    return out;
}

// 006CFF70 (site vtable +3Ch, RET 4): false for a null plane, else
// (3.0 (double 00D7A2B0) - vtable[38h] speed) > 006CFE90.
bool GameUnitsHost::Impl::carrier_elevator_intake_006cff70(const LandingDeck& d,
    const GameUnitSlot& p) const {
    const double v = avoid_len(p.plane_world_velocity);
    return 3.0 - v > static_cast<double>(carrier_elevator_nose_distance_006cfe90(d, p));
}

// 006D0150 (RET 4): the platform still (+94h, P+50h, zero) and at the top
// (0 > P+18h, site+5Ch), and no other occupant within 14.0 ([00E08FC8]) of the
// lift.
bool GameUnitsHost::Impl::carrier_elevator_lift_free_006d0150(const LandingDeck& d,
    const GameUnitSlot& p) const {
    const CarrierElevator& e = d.elevator;
    if (e.mode_50 != 0 || !(0.0f > e.p18)) return false;
    for (const std::size_t oi : d.site_occupants_34) {
        if (oi >= slots.size() || !slots[oi]) continue;
        if (14.0f > carrier_elevator_nose_distance_006cfe90(d, *slots[oi])
            && oi != p.process_index) return false;
    }
    return true;
}

// 006D0390 (site vtable +34h, RET 10h): 006CE610 first; then, with Lz the lift's
// z (class+810h) and pz the plane's (+ACh), true unless the plane is inside the
// lane Lz - (19 + 2 max(0, v - 1.0)) < pz < Lz - 12 (14.0 [00E08FC8], 2.0 and 5.0
// doubles 00D7A308 / 00D7A370, 1.0 00D7A210), where 006D0150 decides.
bool GameUnitsHost::Impl::carrier_elevator_lane_006d0390(const LandingDeck& d,
    const GameUnitSlot& p) const {
    if (!landing_queue_clear_006ce610(d, p)) return false;
    const double lz = d.elevator.lift[2];
    const double pz = carrier_local_point(d, p.motion.position)[2];
    double top;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // pc24w1-sweep3-6: 14.0 - 2.0 (006D03EE), then Lz less it (006D03F4), rounded.
        top = bsp::cc13x87::x87_sub24(lz, bsp::cc13x87::x87_sub24(14.0, 2.0));
    } else {
        top = lz - (14.0 - 2.0);
    }
    if (!(top > pz)) return true;                                        // 006D03FA
    float s = static_cast<float>(static_cast<double>(avoid_len(p.plane_world_velocity)) - 1.0);
    if (0.0f > s) s = 0.0f;
    double bound;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 006D0438 s + s, 006D0440 14.0 + 5.0, 006D0446 FADDP, 006D0448 FSUBP, each
        // rounded at PC24.
        bound = bsp::cc13x87::x87_sub24(lz, bsp::cc13x87::x87_add24(static_cast<double>(s) + s, bsp::cc13x87::x87_add24(14.0, 5.0)));
    } else {
        bound = lz - (static_cast<double>(s) + s + (14.0 + 5.0));
    }
    if (!(pz > bound)) return true;                                      // 006D0450
    return carrier_elevator_lift_free_006d0150(d, p);
}

// 006FC0D0: with a platform plane, its local position = the lift point less
// clamp(P+18h, 0, depth) in y plus its offset (+38h..+40h); 00951F40(1). The
// host writes the local pose and carries it to the world at once.
// 007EABC0 007EAD70-007EADB6 (the animated-model branch): channel +44h enabled
// from class+5E4h (PartAnims.Wings present), target 0, value 0, rate
// 1/class+5E0h (FLD1 / FDIV). SUBSTITUTION, labelled: read from the live
// VehicleClass table at the plane's first block step, and every plane is taken
// as having an animated model (model+130h != 0).
void GameUnitsHost::Impl::wing_fold_class_007eabc0(GameUnitSlot& u) {
    u.wing_fold_read = true;
    if (!bsp::unit_is_kind_of(u.class_id, 0x0f)) return;
    float first = 0.0f, second = 0.0f, third = 0.0f;
    bool has_third = false;
    if constexpr (kBayRequestNoForceEnableBound) {
        // 007EABC0's next arm: channel +60h (the host's channel_c) enabled from
        // class+5F4h (PartAnims.BayDoor present, 007D3B06), target 0, value 0,
        // rate 1/class+5F0h. Same substitution as the wings arm below.
        if (lua.read_vehicle_class_part_anim(u.row.type_id, "BayDoor", first, second, third,
                has_third)) {
            const float bay = has_third ? third
                : bsp::plane_class_part_anim_duration_007d3883(first, second);
            bsp::PlaneActuatorChannel& b = u.actuator_block_dec.channel_c;
            b.enabled = true;
            b.target = false;
            b.value = 0.0f;
            b.rate = 1.0f / bay;
            ++bay_door_classes;
        }
        first = second = third = 0.0f;
        has_third = false;
    }
    if (!lua.read_vehicle_class_part_anim(u.row.type_id, "Wings", first, second, third,
            has_third)) {
        return;
    }
    // 007D3990: |[1] - [2]| (007D3883), replaced by a numeric [3].
    const float duration = has_third ? third
                                     : bsp::plane_class_part_anim_duration_007d3883(first, second);
    bsp::PlaneActuatorChannel& c = u.actuator_block_dec.channel_a;
    c.enabled = true;
    c.target = false;
    c.value = 0.0f;
    c.rate = 1.0f / duration;
    ++wing_fold_classes;
    log.notef("  wing fold %s: class %d PartAnims.Wings {%.3f, %.3f%s} duration %.3f s "
        "(007EABC0, packet cc9_wing_fold)", u.row.name.c_str(), u.row.type_id,
        static_cast<double>(first), static_cast<double>(second), has_third ? ", [3]" : "",
        static_cast<double>(duration));
}

// Goal 4 chain 263: the existing scalar selector, called at 007F2920's two
// actual placement sites. No raw plane/class image is fabricated. The ctor's
// state and the live pose/class mirrors are the values the native body reads.
void GameUnitsHost::Impl::apply_squadron_spawn_placement(GameUnitSlot& unit) {
    const bsp::PlaneSpawnPlacement placement = bsp::choose_spawn_placement_007c6340({
        unit.plane_control_mode_900, unit.motion.position[1],
        unit.motion_class.hull_height, unit.plane_min_water_spd_198});
    if (!placement.launch_arm) {
        unit.plane_control_mode_900 = static_cast<int>(placement.state);
        unit.plane_airborne_908 = placement.airborne_clock;
        unit.plane_landed_904 = false;
        unit.plane_taxi_queue_910 = false;
        unit.plane_water_accum_90c = 0.0f;

        // 007C641A / 007C649F notify with argument 1: set BOTH actuator
        // targets and values instantly, even when the state did not change.
        // Use the existing typed 007B8860 body over the real channel, borrowing
        // only its four semantic cells rather than manufacturing a Raw image.
        if (!unit.wing_fold_read) wing_fold_class_007eabc0(unit);
        auto& block = unit.actuator_block_dec;
        const auto instant = [&](bsp::PlaneActuatorChannel& channel, bool target) {
            auto view = cc13_channel(channel);
            std::uint8_t dirty = static_cast<std::uint8_t>(block.moving_any);
            bsp::band_007a_cc13::actuator_channel_set_instant_007b8860(
                view, static_cast<std::uint8_t>(target), dirty);
            channel.target = view.target != 0;
            channel.value = view.value;
            channel.moving = view.dirty != 0;
            block.moving_any = dirty != 0;
        };
        // Both selected states (6, or 7 with +908h=3600) retract the gear;
        // both spread the wings (007C11E0's argument-1 branch).
        instant(block.channel_b, false);
        instant(block.channel_a, true);
        unit.wing_notified_state_900 = unit.plane_control_mode_900;
        done("PlaneActuatorBlock::set_gear_instant", 0x007b8860u);

        if (placement.state == bsp::PlaneFlightState::Water) {
            // 007C641F..007C644F: sample the current world field at this pose,
            // then add the class's authored SwimHeight with x87 PC24 rounding.
            OceanFieldBinding sea(*this);
            const float water = bsp::ocean_water_height_0078cf20(
                unit.motion.position[0], unit.motion.position[2], sea);
            unit.motion.position[1] = static_cast<float>(bsp::cc13x87::x87_add24(
                water, unit.plane_swim_height));
            publish_pose(unit);
        }
    }
    done("Plane::choose_spawn_placement", 0x007c6340u);
}

// 007C11E0 BSP_Plane_NotifyFlightState, channel +44h only: target = state in
// {4, 5, 6, 7}; with argument 1 the value is set at once. SUBSTITUTION,
// labelled: the image notifies at each of its 19 call sites when +900h changes;
// the host compares +900h with the last state seen here, once per fixed step
// before the block step, and treats the first state seen as the spawn chooser's
// 007C641A / 007C649F call with argument 1 (a deck spawn in state 1 or 2 passes
// 0, which gives the same 0.0 from the constructor).
void GameUnitsHost::Impl::wing_fold_notify_007c11e0(GameUnitSlot& u) {
    if (!u.wing_fold_read) wing_fold_class_007eabc0(u);
    if (!u.actuator_block_dec.channel_a.enabled) return;
    const std::int32_t s = u.plane_control_mode_900;
    if (s == u.wing_notified_state_900) return;
    const bool first = u.wing_notified_state_900 < 0;
    u.wing_notified_state_900 = s;
    const bool spread = s == 4 || s == 5 || s == 6 || s == 7;
    bsp::PlaneActuatorChannel& c = u.actuator_block_dec.channel_a;
    if (first) {
        const float v = spread ? 1.0f : 0.0f;
        if (c.target != spread || c.value != v) {
            c.value = v;
            c.target = spread;
            c.moving = true;
        }
        if (c.moving) u.actuator_block_dec.moving_any = true;
        return;
    }
    wing_target_007b8dc0(u.actuator_block_dec, spread);
}

// Goal 5 chain 650. 007C11E0 and its tail 007BC550 run as their bodies
// (bsp::cc13_front3_plane_notify_flight_state_007c11e0,
// bsp::cc13_seg47::plane_sync_elevator_attachment_for_state_007bc550) over a
// shadow of the plane image. The shadow holds only the offsets the two bodies
// read or write; it is filled from the slot before the call and written back
// after it, and every call the bodies make lands on the slot:
//  * +5Ch, the scene node's active byte: the slot's state->active (row.active
//    when no state object). 00922F30 / 00922F80 are the typed bodies on the
//    slot's SceneNodeFlags (+5Ch..+5Fh) when the host models them; otherwise
//    the active flags are written directly, as before.
//  * +900h / +908h: plane_control_mode_900 / plane_airborne_908.
//  * [+DECh]: the actuator block (+11h moving_any, +28h channel_b, +44h
//    channel_a, offsets as bsp::PlaneActuatorChannel lays them out).
//  * +9F0h: plane_live_throttle.
//  * +AB0h, the flight controller: +3Ch..+50h are the body-frame linear and
//    angular velocities the body zeroes from 00F87574; 007D9C80 is
//    bsp::body_to_world_007d9c80 through the live pose, written back to
//    plane_world_velocity (ctl+18h..+20h), plane_body_angular (ctl+48h..+50h)
//    and motion.linear_velocity. Its 007D8020 tail call is not modelled, as in
//    the motion step's own use of the body.
//  * 00951F40 is bsp::gameplay_seg61_62_cc13::plane_set_visible_00951f40's plan; its attach/detach of
//    the +360h part is plane_hit_node_detached.
// LABELLED, as the inline copy this replaces: +4A4h (the node 007BC550 and
// 00951F40 test) is taken as present; 007DDEA0 and 007EB380(0.0f) (the +C98h FX
// sub-block's timer and effects) and 00779C60(1.0f) are records, the host has
// none of those objects; +C0Ch has no host field (007BB9A0's gate derives it
// from +900h). Every host caller passes immediate = 0 (PUSH 0 at 007C16E8,
// 007CC7DE, 007CC853, 007CB6C0, 007CB72F, 007C6278; PUSH EBX after XOR EBX,EBX
// at 007C1465 in 007C1430), so the immediate arm (+72Ch slot +38h, 007EB270)
// is not reached from this host.
namespace {
class PlaneFlightStateNotifyBinding final : public bsp::PlaneNotifyFlightStateHost,
                                            public bsp::cc13_seg47::PlaneAttachmentSyncHost {
public:
    PlaneFlightStateNotifyBinding(GameUnitsHost::Impl& host, GameUnitSlot& p)
        : host_(host), p_(p) {}

    void run() {
        std::memset(plane_, 0, sizeof(plane_));
        std::memset(block_, 0, sizeof(block_));
        const bool active = p_.state != nullptr ? p_.state->active != 0 : p_.row.active;
        put<std::uint8_t>(plane_, 0x5C, active ? 1u : 0u);
        std::uint8_t* const node = plane_;   // +4A4h present (labelled above)
        std::memcpy(plane_ + 0x4A4, &node, sizeof(node));
        put<std::int32_t>(plane_, 0x900, p_.plane_control_mode_900);
        put<float>(plane_, 0x908, p_.plane_airborne_908);
        put<float>(plane_, 0x9F0, p_.plane_live_throttle);
        std::uint8_t* const block = block_;
        std::memcpy(plane_ + 0xDEC, &block, sizeof(block));
        const bsp::PlaneActuatorBlock& b = p_.actuator_block_dec;
        put<std::uint8_t>(block_, 0x11, b.moving_any ? 1u : 0u);
        put_channel(0x28, b.channel_b);
        put_channel(0x44, b.channel_a);

        bsp::cc13_front3_plane_notify_flight_state_007c11e0(plane_, false, *this);

        p_.plane_live_throttle = get<float>(plane_, 0x9F0);
        bsp::PlaneActuatorBlock& out = p_.actuator_block_dec;
        out.moving_any = get<std::uint8_t>(block_, 0x11) != 0;
        get_channel(0x28, out.channel_b);
        get_channel(0x44, out.channel_a);
    }

    // PlaneNotifyFlightStateHost.
    const float* static_vector_00f87574() override { return kZero; }
    std::uint32_t vcall_38_dword(void*) override {
        throw std::logic_error("007C11E0: the immediate arm (+72Ch slot +38h) has no host caller");
    }
    void plane_sync_elevator_attachment_007bc550(void* plane) override {
        bsp::cc13_seg47::plane_sync_elevator_attachment_for_state_007bc550(
            *this, bsp::cc13_seg47::raw(plane));
        host_.done("Plane::sync_elevator_attachment_007bc550", 0x007bc550u);
    }
    void flight_controller_body_to_world_velocity_007d9c80(void* controller) override {
        std::uint8_t* const ctl = static_cast<std::uint8_t*>(controller);
        bsp::ControllerVelocities v;
        for (int i = 0; i < 3; ++i) {
            v.linear_body[i] = get<float>(ctl, 0x3C + 4 * static_cast<std::size_t>(i));
            v.angular_body[i] = get<float>(ctl, 0x48 + 4 * static_cast<std::size_t>(i));
        }
        bsp::AdvanceMatrix live{};
        const float* const rows[3] = {p_.motion.pose_row0, p_.motion.pose_row1,
                                      p_.motion.pose_row2};
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) live.m[r * 4 + c] = rows[r][c];
            live.m[r * 4 + 3] = 0.0f;
            live.m[12 + r] = p_.motion.position[r];
        }
        live.m[15] = 1.0f;
        bsp::body_to_world_007d9c80(v, live);
        for (int i = 0; i < 3; ++i) {
            put<float>(ctl, 0x18 + 4 * static_cast<std::size_t>(i), v.linear_world[i]);
            put<float>(ctl, 0x24 + 4 * static_cast<std::size_t>(i), v.angular_world[i]);
            p_.plane_world_velocity[i] = v.linear_world[i];
            p_.plane_body_angular[i] = v.angular_body[i];
        }
        p_.motion.linear_velocity = bsp::OceanVec3{v.linear_world[0], v.linear_world[1],
                                                   v.linear_world[2]};
        host_.done("PlaneFlightController::body_to_world_velocity_007d9c80", 0x007d9c80u);
    }
    void plane_sub_block_reset_timer40_007ddea0(void*) override {
        host_.record("PlaneFx::reset_timer40_007ddea0", 0x007ddea0u);
    }
    void plane_fx_on_flight_state_007eb270(void*, std::uint32_t) override {
        throw std::logic_error("007C11E0: 007EB270 is on the immediate arm, which has no host caller");
    }
    void plane_fx_step_007eb380(void*, float) override {
        host_.record("PlaneFx::step_007eb380", 0x007eb380u);
    }

    // PlaneAttachmentSyncHost.
    void unit_instance_detach_00951f40(bsp::cc13_seg47::Raw unit, std::int32_t value) override {
        const bsp::gameplay_seg61_62_cc13::PlaneVisiblePlan plan =
            bsp::gameplay_seg61_62_cc13::plane_set_visible_00951f40(unit.ptr(0x4A4) != nullptr, value != 0);
        if (plan.acts) p_.plane_hit_node_detached = !plan.attach;   // 00710AD0 / 00710B80
    }
    void scene_node_disable_00922f80(bsp::cc13_seg47::Raw entity, std::int32_t) override {
        set_scene_node_active(entity, false);
    }
    void scene_node_enable_00922f30(bsp::cc13_seg47::Raw entity, std::int32_t) override {
        set_scene_node_active(entity, true);
    }
    void unit_update_owner_hud_state_00779c60(bsp::cc13_seg47::Raw, float) override {
        host_.record("Unit::fade_00779c60", 0x00779c60u);
    }

private:
    template <class T>
    static void put(std::uint8_t* base, std::size_t offset, T value) noexcept {
        std::memcpy(base + offset, &value, sizeof(T));
    }
    template <class T>
    static T get(const std::uint8_t* base, std::size_t offset) noexcept {
        T value;
        std::memcpy(&value, base + offset, sizeof(T));
        return value;
    }
    void put_channel(std::size_t at, const bsp::PlaneActuatorChannel& c) noexcept {
        put<std::uint8_t>(block_, at + 0x0, c.enabled ? 1u : 0u);
        put<std::uint8_t>(block_, at + 0x1, c.target ? 1u : 0u);
        put<float>(block_, at + 0x4, c.value);
        put<std::uint8_t>(block_, at + 0x8, c.moving ? 1u : 0u);
    }
    void get_channel(std::size_t at, bsp::PlaneActuatorChannel& c) const noexcept {
        c.target = get<std::uint8_t>(block_, at + 0x1) != 0;
        c.value = get<float>(block_, at + 0x4);
        c.moving = get<std::uint8_t>(block_, at + 0x8) != 0;
    }
    void set_scene_node_active(bsp::cc13_seg47::Raw entity, bool enable) {
        if (p_.scene_flags_available && p_.state != nullptr) {
            bsp::SceneNodeFlags flags{p_.state->active != 0, p_.state->simulate != 0,
                                      p_.scene_destroyed_005e != 0, p_.scene_removed_005f != 0};
            if (enable) {
                bsp::scene_node_enable_00922f30(flags);
            } else {
                bsp::scene_node_disable_00922f80(flags);
            }
            p_.state->active = flags.active ? 1 : 0;
            p_.row.active = flags.active;
        } else {
            if (p_.state != nullptr) p_.state->active = enable ? 1 : 0;
            p_.row.active = enable;
        }
        const bool active = p_.state != nullptr ? p_.state->active != 0 : p_.row.active;
        entity.at<std::uint8_t>(0x5C) = active ? 1u : 0u;
    }

    static constexpr float kZero[3] = {0.0f, 0.0f, 0.0f};   // 00F87574, zero BSS
    GameUnitsHost::Impl& host_;
    GameUnitSlot& p_;
    alignas(8) std::uint8_t plane_[0xDF0];
    alignas(8) std::uint8_t block_[0x70];
};
}  // namespace

void GameUnitsHost::Impl::flight_state_notify_007c11e0(GameUnitSlot& p) {
    // 007EABC0 runs at the plane's construction in the image, so the class read
    // the poll makes lazily is made first here, or it would later reset the
    // targets the notify writes.
    if (!p.wing_fold_read) wing_fold_class_007eabc0(p);
    ++flight_state_notifies;
    PlaneFlightStateNotifyBinding binding(*this, p);
    binding.run();
    // The poll stand-in's bookkeeping (wing_fold_notify_007c11e0), not the image's.
    if (p.wing_notified_state_900 >= 0) p.wing_notified_state_900 = p.plane_control_mode_900;
    done("Plane::flight_state_notify_007c11e0", 0x007c11e0u);
}

// Goal 5 chain 668: 007CA790's Lua half (see the declaration).
void GameUnitsHost::Impl::store_exit_zone_party_007ca790(std::size_t unit, std::int32_t party) {
    lua_State* const state = lua.mission_lua_state();
    if (state == nullptr || unit >= slots.size()) {
        ++exit_zone_party_no_slot;
        return;
    }
    const int base = lua_gettop(state);
    lua_getfield(state, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (lua_istable(state, -1)) {
        char key[16];
        std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat,
            static_cast<int>(unit + 1u));
        lua_getfield(state, -1, key);
        if (lua_istable(state, -1)) {
            lua_pushinteger(state, static_cast<lua_Integer>(party));
            lua_setfield(state, -2, "ExitZoneParty");
            lua_settop(state, base);
            ++exit_zone_party_stores;
            log.notef("exit zone: plane \"%s\" self.ExitZoneParty = %d at %.2f s (007CA790)",
                slots[unit]->row.name.c_str(), party,
                static_cast<double>(summary.simulated_seconds));
            return;
        }
    }
    lua_settop(state, base);
    ++exit_zone_party_no_slot;
}

// 007CC16C-007CC186: (+BF4h)->+4h->+3Ch vtable[38h](unit). A carrier's site is
// the elevator site (vtable 00CF8A58, slot 006CFFC0: 20.0f (00CE3930) >
// 006CFE90, the nose's distance to the lift, FCOMI so a NaN answers false);
// an airfield's launch site (vtable 00CF89F8, slot 006CEBF0) answers false.
// SUBSTITUTION, labelled: a mother-ship deck whose elevator this host has not
// built (no lift point) answers false, as it never takes a plane either.
bool GameUnitsHost::Impl::wing_fold_site_holds_006cffc0(const GameUnitSlot& u) const {
    const std::size_t hd = u.plane_contact_deck_bf4;
    if (hd == 0 || hd > landing_decks.size()) return false;
    const LandingDeck& d = landing_decks[hd - 1u];
    if (!d.mother_ship || !d.elevator.built) return false;
    return 20.0f > carrier_elevator_nose_distance_006cfe90(d, u);
}

void GameUnitsHost::Impl::carrier_elevator_carry_006fc0d0(LandingDeck& d) {
    CarrierElevator& e = d.elevator;
    if (e.plane_34 >= slots.size() || !slots[e.plane_34]) return;
    GameUnitSlot& p = *slots[e.plane_34];
    float y = e.p18;
    if (0.0f > y) y = 0.0f;
    else if (y > e.depth_48) y = e.depth_48;
    p.deck_local_pos[0] = e.lift[0] + e.off_38[0];
    p.deck_local_pos[1] = static_cast<float>(static_cast<double>(e.lift[1]) - y) + e.off_38[1];
    p.deck_local_pos[2] = e.lift[2] + e.off_38[2];
    // The velocity stop predates packet cc9_wing_fold and stays; 007B8DE0 at
    // 006FC15E is the wing reset: target 0 and value 0.0 at once.
    for (float& v : p.deck_local_vel) v = 0.0f;
    if constexpr (kPlaneWingFoldElevatorBound && kActuatorFlagCc13BodiesBound) {
        bsp::PlaneActuatorChannel& c = p.actuator_block_dec.channel_a;
        if ((c.target || c.value != 0.0f) && c.enabled && c.value != 0.0f) ++wing_fold_resets;
        bsp::band_007a_cc13::ActuatorChannel cc = cc13_channel(c);
        std::uint8_t dirty = static_cast<std::uint8_t>(p.actuator_block_dec.moving_any);
        bsp::band_007a_cc13::actuator_channel_reset_to_zero_007b8de0(cc, dirty);
        store_cc13_channel(p.actuator_block_dec, cc, dirty);
    } else if constexpr (kPlaneWingFoldElevatorBound) {
        bsp::PlaneActuatorChannel& c = p.actuator_block_dec.channel_a;
        if (c.target || c.value != 0.0f) {
            if (c.enabled && c.value != 0.0f) ++wing_fold_resets;
            c.value = 0.0f;
            c.target = false;
            c.moving = true;
        }
        if (c.moving) p.actuator_block_dec.moving_any = true;
    }
    carrier_deck_carry_in(p);
    p.plane_hit_node_detached = false;               // 00951F40(1) -> 00710AD0
}

// 006FC250: carry, drop the platform plane, and hide it (00951F40(0)) when the
// platform is still and at the bottom (P+18h > depth).
void GameUnitsHost::Impl::carrier_elevator_release_006fc250(LandingDeck& d) {
    CarrierElevator& e = d.elevator;
    carrier_elevator_carry_006fc0d0(d);
    const std::size_t pi = e.plane_34;
    e.plane_34 = static_cast<std::size_t>(-1);
    ++e.releases;
    // 006FC279-006FC28F: at the bottom and still, 00951F40(0) detaches the
    // plane's node (the hit queries honour it behind the gunnery host's
    // kHitIndexDetachBound). The site tick has already set +C00h through 007B96C0.
    if (pi < slots.size() && slots[pi]) slots[pi]->plane_hit_node_detached = true;
}

// 007CC7A0 BSP_Plane_EnterFlightStateTwo, reached from 007C2090's C3h message
// (single player: game+1FE4h is not 2, so no 007C78A0): +9F0h = 0; when +900h
// is not 2, +900h = 2, +C04h = -1.0 and 007C11E0(0) (carried under
// kPlaneFlightStateNotifyCc13Bound, which enables the parked plane);
// game+193Ch is not modelled.
void GameUnitsHost::Impl::plane_enter_state_two_007cc7a0(GameUnitSlot& p) {
    p.plane_live_throttle = 0.0f;
    if (p.plane_control_mode_900 == 2) return;
    p.plane_control_mode_900 = 2;
    p.plane_site_timer_c04 = -1.0f;
    ++carrier_elevator_state_two;
    if constexpr (bsp::kPlaneFlightStateNotifyCc13Bound) {
        flight_state_notify_007c11e0(p);                                // 007CC7F4
        done("Plane::enter_state_two_007cc7a0", 0x007cc7a0u);
    } else {
        record("Plane::enter_state_two_007cc7a0", 0x007cc7a0u);
    }
}

// 007C2130 BSP_Plane_RequestFlightStateOne (007C2130-007C21C3, __thiscall(plane),
// RET): skips a plane already in 1 (007C214B) and a client session (game+1FE4h ==
// 2, 007C2159; single player is not); otherwise a C3h message with the current
// state (007C2169) and requested 1 routed with 7 (007C21AD 0077C2A0), delivered
// here at once, as 007C2090's twin is. Its handler 007CC820
// BSP_Plane_EnterFlightStateOne (`RET`): 007C78A0 only on a client; when +900h
// is not 1, +900h = 1, +C04h = -1.0 (00D7A260) and 007C11E0(0); then the
// re-parent vtable[ACh] (00924F90) to the squadron's home base (+9D4h)->+404h
// (007CC86E-007CC88B), which every launched member has. The re-parent is the
// host's deck parent (the local pose under the base).
//
// 007C11E0 BSP_Plane_NotifyFlightState(0) in state 1 (packet
// cc9_base_launch_task): the actuator block +DECh bytes (not carried: no +DECh
// block here); a state outside 7/6/4/5/3 zeroes +9F0h (the throttle), stores
// the vector at 00F87574 into +AECh..+B00h (LABELLED: taken as the zero vector,
// not read), then BSP_PlaneFlightController_BodyToWorldVelocity, 007DDEA0 and
// 007EB380(0) (not carried beyond the zeroing); and in state 0 or 1 it calls
// 007BC550, which with the Lua self +4A4h set (every attached member) DISABLES
// the scene node (00922F80: +5Ch = 0) and detaches the spatial node
// (00951F40(0)). So a member Inside is a disabled entity: the world walk
// (00904C00) skips it and 006D1EF0 does not count it live, and 007EF010 sends
// only members whose +5Ch is clear. The host's +5Ch is state->active.
// +908h: the host's creation seed wrote 3600 for 007C6340, which 007F4DA9
// skips here, so it goes back to the constructor's zero (LABELLED: inferred
// from 007C6340 writing it on every placement arm).
void GameUnitsHost::Impl::plane_enter_state_one_007cc820(GameUnitSlot& p,
                                                        std::size_t base_index) {
    if (p.plane_control_mode_900 == 1) return;                        // 007C214B
    p.plane_control_mode_900 = 1;                                     // 007CC857
    p.plane_site_timer_c04 = -1.0f;                                   // 007CC861
    p.plane_airborne_908 = 0.0f;
    if constexpr (bsp::kPlaneFlightStateNotifyCc13Bound) {
        flight_state_notify_007c11e0(p);                                // 007CC869
    } else {
        // 007C11E0(0), state 1: +9F0h = 0 and the velocities zeroed.
        p.plane_live_throttle = 0.0f;
        for (int i = 0; i < 3; ++i) {
            p.plane_world_velocity[i] = 0.0f;
            p.plane_body_angular[i] = 0.0f;
        }
        p.motion.linear_velocity = bsp::OceanVec3{0.0f, 0.0f, 0.0f};
        // 007BC550: 00922F80 (+5Ch = 0) and 00951F40(0).
        if (p.state != nullptr) p.state->active = 0;
        p.row.active = false;
        p.plane_hit_node_detached = true;
    }
    if (base_index < slots.size() && slots[base_index]) {
        p.deck_parent_plus_one = base_index + 1u;                     // vtable[ACh]
        p.deck_stop_logged = false;
        carrier_deck_capture(p);
    }
    ++base_launch_inside_entries;
    done("Plane::enter_state_one_007cc820", 0x007cc820u);
}

// 007F1F00 BSP_SquadronLaunchTask_Tick (007F1F00-007F1FDB, __thiscall(task,
// float dt), RET 4), with 007EF010 BSP_PlaneSquadron_SendNextHangarMember
// (007EF010-007EF090, RET) and the site's readiness vtable[18h]: 006CDF60
// (airfield, site+18h == 0) or 006CFF40 (mother ship: platform mode P+50h == 0,
// 0.0 > P+18h (the lift at the top), site+18h == 0). task+1Ch counts down by
// dt and holds while above 0 (007F1F8D JA); when the site is ready it is
// re-armed to tuning+190h (AirField/PlaneSendInterval) and 007EF010 sends the
// first member (+3D0h order) with +5Ch clear and +900h == 1: its vtable[10h]
// (0042E950, the name getter; no effect), then, the base owner alive
// (block+7Ch non-null, +5Dh clear), 006CF190(site, member): site+18h =
// member. With the owner gone it kills the member (00926D90(1), counted and not
// carried: this host's air-ops owner is a live unit when the squadron is made).
// 007EF010 answering 1 (none left) unregisters the squadron and ends the task
// (007F1FB5-007F1FCF, vtable[10h]).
void GameUnitsHost::Impl::base_launch_task_tick_007f1f00(bsp::PlaneSquadronHostRecord& sq,
                                                        float dt) {
    if (!sq.launch_task_live) return;
    bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
    const std::size_t di = sq.launch_task_deck;
    const bsp::AirOpsDeck* deck = di < decks.size() ? decks.mutable_at(di) : nullptr;
    std::size_t owner = slots.size();
    if (deck != nullptr) {
        owner = air_ops_owner_unit(decks.name_at(di));   // kAirOpsOwnerByInstanceBound
    }
    // 007F1F5C-007F1F71: squadron, block and the block's owner alive.
    if (deck == nullptr || owner >= slots.size()) { ++base_launch_no_deck; return; }
    if (gunnery != nullptr && gunnery->unit_dead(owner)) return;
    ++base_launch_ticks;
    // 007F1F73-007F1F8D: +1Ch -= dt; hold while above 0.
    sq.launch_task_timer_1c -= dt;
    if (sq.launch_task_timer_1c > 0.0f) return;
    // 007F1F8F-007F1F9B: the site's vtable[18h].
    LandingDeck* d = landing_deck_006c0750(di, owner);
    if (d == nullptr) { ++base_launch_no_deck; return; }
    bool ready = d->site_ready_18 >= slots.size();
    if (!deck->is_airfield) {
        CarrierElevator* e = carrier_elevator_ready(*d);
        ready = ready && e != nullptr && e->mode_50 == 0 && 0.0f > e->p18;
    }
    if (!ready) { ++base_launch_not_ready; return; }
    // 007F1F9D-007F1FAB: +1Ch = tuning+190h.
    sq.launch_task_timer_1c = lua.plane_globals_loaded()
        ? lua.plane_globals().air_field_plane_send_interval : 2.0f;
    // 007EF010.
    std::size_t pick = slots.size();
    for (const std::size_t m : sq.member_units) {
        if (m == bsp::kPlaneSquadronNoUnit || m >= slots.size() || !slots[m]) continue;
        const GameUnitSlot& p = *slots[m];
        const bool enabled = p.state != nullptr && p.state->active != 0;   // +5Ch
        if (!enabled && p.plane_control_mode_900 == 1) { pick = m; break; }
    }
    if (pick >= slots.size()) {
        // 007EF041: 1, none left; 007F1FB7-007F1FCF end the task.
        sq.launch_task_live = false;
        ++base_launch_tasks_ended;
        log.notef("base launch task: squadron=%s ends at %.2f s after %d send(s) "
            "(007EF010 -> 1, 007F1FCF; packet cc9_base_launch_task)", sq.name.c_str(),
            static_cast<double>(summary.simulated_seconds), sq.launch_task_sends);
        done("SquadronLaunchTask::tick_007f1f00", 0x007f1f00u);
        return;
    }
    // 007EF067-007EF072: the owner of squadron+404h's block, alive. The tick's
    // own gate (007F1F6D) tested the same owner this step, so the kill arm
    // (00926D90(1)) cannot be reached here; base_launch_owner_dead stays 0.
    // 006CF190: site+18h = member.
    d->site_ready_18 = pick;
    ++d->site_ready_sets;
    ++sq.launch_task_sends;
    ++base_launch_sends;
    log.notef("base launch task: squadron=%s sends %s to %s %s at %.2f s (send %d; "
        "007F1F00 -> 007EF010 -> 006CF190 site+18h; packet cc9_base_launch_task)",
        sq.name.c_str(), slots[pick]->row.name.c_str(),
        deck->is_airfield ? "airfield" : "mother ship", decks.name_at(di).c_str(),
        static_cast<double>(summary.simulated_seconds), sq.launch_task_sends);
    done("SquadronLaunchTask::tick_007f1f00", 0x007f1f00u);
    done("Squadron::send_next_hangar_member_007ef010", 0x007ef010u);
}  // namespace

void GameUnitsHost::Impl::airfield_site_tick_006cf980(std::size_t deck_index, LandingDeck& d) {
    if (d.site_ready_18 >= slots.size() || !slots[d.site_ready_18]) return;
    // Goal 5 chain 628. 006CF9AD..006CF9C7: the ready plane goes out as
    // MT_AIRBASE_APPEARREADYPLANE (86h, 006BDAC0) through 0077C2A0(msg, 5, 0)'s
    // local copy, and the receiver's 86h arm (006CD788) hands 006CF9F0 the plane
    // the copy's +20h names (006BDBF0). +174h is this process's unit index + 1
    // (GameScriptOrdersHost::entity_object_id). LABELLED: an index the copy's
    // 12 bits cannot carry keeps the posted plane, as queue_gun_aim_79 keeps
    // its target id; delivery stays in this step, as before.
    std::size_t plane_index = d.site_ready_18;
    if (d.site_ready_18 + 1u <= 0xfffu) {
        const std::uint16_t copy = commands.session_messages().queue_airbase_appear_ready_plane_86(
            static_cast<std::uint16_t>(d.site_ready_18 + 1u));
        plane_index = copy == 0 ? slots.size() : static_cast<std::size_t>(copy) - 1u;
    }
    if (plane_index >= slots.size() || !slots[plane_index]) {
        d.site_ready_18 = static_cast<std::size_t>(-1);                // 006CFA54
        return;
    }
    GameUnitSlot& p = *slots[plane_index];
    const bool owner_alive = d.owner < slots.size() && slots[d.owner]
        && !(gunnery != nullptr && gunnery->unit_dead(d.owner));
    if (owner_alive && place_on_launch_spot_007c5f60(p, deck_index, d, true)) {
        // 007C3C90(plane, 1) -> 007C1570(5, 0): the typed body gates the C3h
        // message (above); OFF keeps the direct 007CA3F0 call.
        request_ground_state_007c3c90(*this, p, d, true);
        record("SceneNode::invalidate_subtree_pose_0042ed50", 0x0042ed50u);
        record("Plane::tick_node_advance_zero_006cfa41", 0x006cfa41u);
    } else {
        ++base_launch_place_refused;
    }
    d.site_ready_18 = static_cast<std::size_t>(-1);                    // 006CFA54
    done("AirOpsSite::place_plane_on_spot_006cf9f0", 0x006cf9f0u);
}

// 007C5F60 BSP_Plane_PlaceOnLaunchSpotLocked (007C5F60-007C632E, __thiscall(plane,
// block), RET 4). The pose: block+3Ch ->vtable[40h] = 006CF730 (006CF730-006CF972)
// samples points 0 and 1 of 006D2780's entry path in world, slides point 0 along
// their direction by (planeDesc+158h + 1.0) - plane+9D8h x 0.2 (double 00CE3D10),
// takes y from the airfield's [+7ACh]+9Ch (or the ground height without one) and
// returns the frame in the airfield's axes (x +110h). 007C5F60 then tilts the
// forward row to GroundPitch (classDesc+200h: x, z scaled by cos / |(x, z)|, y =
// sin) and orthonormalises; +9DCh = 5.0 (00CE3850, 007C60EA) when block+7Ch
// IsKindOf(45h) (AirField), else 3.0 (00CE3854, 007C605D); on both arms the local y
// gains WheelHeight (classDesc+1FCh, 007C6065 / 007C6135); the +DECh gear
// and flap writes are not carried (no +DECh block). vtable[ACh](block+7Ch)
// re-parents the plane to the base; 007EB270(0), 007C18B0(0) and vtable[D8h] are
// recorded; 007B8E80 attaches the holder block+80h and 007C5AC0(-1.0) probes the
// site; +9F0h = 0; +900h = 2 with +C04h = -1.0 and 007C11E0(0) when it was not 2;
// the velocities are zeroed (00F87574, LABELLED as the zero vector); 008073C0 is
// recorded; and 007BC550 ENABLES the plane in state 2 (00922F30: +5Ch = 1,
// 00951F40(1)).
// SUBSTITUTIONS, labelled: planeDesc+158h is 0.0 (kPlaneDesc158); the height is
// point 0's world y (the airfield's +7ACh object is not read); the airfield's
// axes are taken as the world's, so the local offsets are applied in world;
// +9DCh is kept as launch_spot_cooldown_9dc with the count launch_spot_arms_9dc,
// for the gunnery crash sweep (block+7Ch is the deck owner); the orthonormalisation
// 0085DC80 (called at 007C6029) is the host's own normalise-and-cross.
bool GameUnitsHost::Impl::place_on_launch_spot_007c5f60(GameUnitSlot& p,
    std::size_t deck_index, LandingDeck& d, bool airfield) {
    float pos[3] = {0.0f, 0.0f, 0.0f};
    float slide = 0.0f;
    std::int32_t index_9d8 = 0;
    const bsp::PlaneSquadronHostRecord* sq =
        bsp::plane_squadron_registry().find_by_member_unit(p.process_index);
    if (sq != nullptr) {
        for (std::size_t i = 0; i < sq->member_units.size(); ++i) {
            if (sq->member_units[i] == p.process_index && i < sq->member_spawn_index.size()) {
                index_9d8 = sq->member_spawn_index[i];
            }
        }
    }
    const float cp = std::cos(p.plane_ground_pitch_200);
    const float sp = std::sin(p.plane_ground_pitch_200);
    if (airfield) {
        const bsp::game::ScenePathEntry* path = landing_hangar_path_006d2780_006d2640(d, true);
        if (path == nullptr || path->points_world.size() < 2) return false;
        const std::array<float, 3>& a = path->points_world[0];
        const std::array<float, 3>& b = path->points_world[1];
        float dir[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
        const float len = std::sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
        const float inv = len > 0.0f ? 1.0f / len : 0.0f;
        for (float& c : dir) c *= inv;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // pc24w1-sweep3-6: 006CF730's 006CF86D FADD +158h, 006CF877 FMUL 0.2,
            // 006CF87D FSUBP, each rounded at PC24.
            slide = static_cast<float>(bsp::cc13x87::x87_sub24(bsp::cc13x87::x87_add24(plane_desc_158(p), 1.0),
                bsp::cc13x87::x87_mul24(static_cast<double>(index_9d8), 0.20000000298023224)));
        } else {
            slide = static_cast<float>((static_cast<double>(plane_desc_158(p)) + 1.0)
                - static_cast<double>(index_9d8) * 0.20000000298023224);
        }
        pos[0] = a[0] + dir[0] * slide;
        pos[1] = a[1];
        pos[2] = a[2] + dir[2] * slide;
        // 007C5F9E-007C5FDC: the forward row at GroundPitch.
        const float h = std::sqrt(dir[0] * dir[0] + dir[2] * dir[2]);
        float fwd[3] = {h > 0.0f ? dir[0] * (cp / h) : 0.0f, sp,
                        h > 0.0f ? dir[2] * (cp / h) : cp};
        const float fl = std::sqrt(fwd[0] * fwd[0] + fwd[1] * fwd[1] + fwd[2] * fwd[2]);
        // pc24-lint: allow single float op: c / fl is one float FDIV, already rounded per op (as at PC24); this |fwd| normalisation and the right/up crosses below are the host's stand-in for 0085DC80 (called at 007C6029), not modelled per op
        for (float& c : fwd) c /= fl;
        // right = up x forward, up = forward x right (the identity pose's handedness).
        float right[3] = {fwd[2], 0.0f, -fwd[0]};
        const float rl = std::sqrt(right[0] * right[0] + right[2] * right[2]);
        if (rl > 0.0f) { right[0] /= rl; right[2] /= rl; } else { right[0] = 1.0f; }
        const float up[3] = {fwd[1] * right[2] - fwd[2] * right[1],
                             fwd[2] * right[0] - fwd[0] * right[2],
                             fwd[0] * right[1] - fwd[1] * right[0]};
        pos[1] += p.plane_wheel_height_1fc;                             // 007C5FF8
        for (int i = 0; i < 3; ++i) {
            p.motion.position[i] = pos[i];
            p.motion.pose_row0[i] = right[i];
            p.motion.pose_row1[i] = up[i];
            p.motion.pose_row2[i] = fwd[i];
        }
        // vtable[ACh](block+7Ch): the base is the parent.
        p.deck_parent_plus_one = d.owner + 1u;
        p.deck_stop_logged = false;
    } else {
        // The mother-ship arm. Its site's vtable[40h] is 006D0930 (006D0930-,
        // __thiscall(site, out, plane)): identity axes, translation class+808h..
        // +810h (the lift point) with y less settings+4DCh and planeDesc+158h.
        // 007C5F60's mother-ship arm (007C6030-007C6090): +9DCh = 5.0 (00CE3850),
        // local x += +9D8h x (sin(GroundPitch) < 0 ? -1.0 : 1.0), y += WheelHeight,
        // z += 0.0 (double 00D7A258). 006D0050 then calls 006FC810, whose carry
        // replaces the translation with the lift point plus (0, WheelHeight,
        // -planeDesc+158h); so only the axes survive, and the host writes the
        // carried translation here (ORDER, LABELLED: the image probes the site
        // at 006D0930's point, this host at the lift point).
        CarrierElevator* e = carrier_elevator_ready(d);
        if (e == nullptr) return false;
        const float fl = std::sqrt(sp * sp + cp * cp);
        const float fwd[3] = {0.0f, sp / fl, cp / fl};
        const float right[3] = {1.0f, 0.0f, 0.0f};
        const float up[3] = {0.0f, fwd[2], -fwd[1]};
        for (int i = 0; i < 3; ++i) {
            p.deck_local_rows[0 + i] = right[i];
            p.deck_local_rows[3 + i] = up[i];
            p.deck_local_rows[6 + i] = fwd[i];
            p.deck_local_vel[i] = 0.0f;
        }
        float y = e->p18;
        if (0.0f > y) y = 0.0f;
        else if (y > e->depth_48) y = e->depth_48;
        p.deck_local_pos[0] = e->lift[0];
        p.deck_local_pos[1] = static_cast<float>(static_cast<double>(e->lift[1]) - y)
            + p.plane_wheel_height_1fc;
        p.deck_local_pos[2] = e->lift[2] + (-0.0f - plane_desc_158(p));
        p.deck_parent_plus_one = d.owner + 1u;                          // vtable[ACh]
        p.deck_stop_logged = false;
        carrier_deck_carry_in(p);
        for (int i = 0; i < 3; ++i) pos[i] = p.motion.position[i];
    }
    // 007B8E80(holder block+80h), then 007C5AC0(-1.0).
    p.plane_contact_deck_bf4 = deck_index + 1u;
    plane_site_probe_007c5ac0(p, -1.0f);
    p.plane_live_throttle = 0.0f;                                        // +9F0h
    if (p.plane_control_mode_900 != 2) {
        p.plane_control_mode_900 = 2;
        p.plane_site_timer_c04 = -1.0f;
        if constexpr (bsp::kPlaneFlightStateNotifyCc13Bound) {
            flight_state_notify_007c11e0(p);                            // 007C628A
        }
    }
    for (int i = 0; i < 3; ++i) {
        p.plane_world_velocity[i] = 0.0f;
        p.plane_body_angular[i] = 0.0f;
    }
    p.motion.linear_velocity = bsp::OceanVec3{0.0f, 0.0f, 0.0f};
    carrier_deck_capture(p);
    // 007BC550 in state 2: 00922F30 and 00951F40(1).
    if (p.state != nullptr) p.state->active = 1;
    p.row.active = true;
    p.plane_hit_node_detached = false;
    ++base_launch_placed;
    record("Recon::rebuild_slot_lists_008073c0", 0x008073c0u);
    log.notef("base launch placed: %s on %s at (%.1f %.1f %.1f) index=%d slide=%.2f "
        "pitch=%.3f at %.2f s, state 2 enabled (%s -> 007C5F60, 007BC550; packet "
        "cc9_base_launch_deck_arms)", p.row.name.c_str(),
        slots[d.owner]->row.name.c_str(), static_cast<double>(pos[0]),
        static_cast<double>(pos[1]), static_cast<double>(pos[2]), index_9d8,
        static_cast<double>(slide), static_cast<double>(p.plane_ground_pitch_200),
        static_cast<double>(summary.simulated_seconds),
        airfield ? "006CF9F0 / 006CF730" : "006D0050 / 006D0930");
    p.launch_spot_cooldown_9dc = bsp::unit_is_kind_of(slots[d.owner]->class_id, 0x45)
        ? bsp::kPlaneCrashAirfieldLaunchCooldown : bsp::kPlaneCrashLaunchCooldown;
    ++p.launch_spot_arms_9dc;
    done("Plane::place_on_launch_spot_locked_007c5f60", 0x007c5f60u);
    return true;
}

// 006D02F0 (__fastcall(site), RET): true when the platform holds a plane (site+78h
// = P+34h), or any site occupant's local z (+ACh, under the carrier) lies in
// (lift z - 14.0 x 2.0, lift z + 14.0): 00E08FC8 (14.0 in the image's .data,
// LABELLED: its writers were not censused), 00D7A308 (double 2.0). The lift z is
// class+810h, the host's lift point.
bool GameUnitsHost::Impl::carrier_elevator_blocked_006d02f0(const LandingDeck& d) const {
    const CarrierElevator& e = d.elevator;
    if (e.plane_34 < slots.size()) return true;
    const float z = e.lift[2];
    for (const std::size_t oi : d.site_occupants_34) {
        if (oi >= slots.size() || !slots[oi]) continue;
        const float lz = carrier_local_point(d, slots[oi]->motion.position)[2];
        if (lz < z + 14.0f && static_cast<double>(z) - 14.0 * 2.0 < static_cast<double>(lz)) {
            return true;
        }
    }
    return false;
}

// 006D0530 BSP_MotherShipSite_HasFreeRunway (006D0530-, the mother-ship site's
// vtable[30h], __thiscall(site), RET, AL): for each occupant of site+34h, the
// third component of 006BCC90 on the carrier's holder ([site+B8h]+1208h) at the
// occupant's world position (plane+FCh, after 00414DB0); false when
// tuning+520h (Pilot/Landing/MinFreeRunwayLength) is greater than it (006D05CE
// FCOMIP / JA, so a NaN z does not fail). LABELLED: the holder is read as the sequencer pass left
// it, not re-framed here; 006D0530 itself does not re-frame it.
bool GameUnitsHost::Impl::carrier_site_has_free_runway_006d0530(const LandingDeck& d) const {
    const float min_free = lua.plane_globals_loaded()
        ? lua.plane_globals().pilot_landing_min_free_runway_length : 0.0f;
    bool free_runway = true;
    for (const std::size_t oi : d.site_occupants_34) {
        if (oi >= slots.size() || !slots[oi]) continue;
        const float along = landing_local_006bcc90(d, slots[oi]->motion.position)[2];
        if (min_free > along) {
            free_runway = false;
            break;
        }
    }
    return free_runway;
}

// 006FC640 BSP_ElevatorPlatform_SendEmptyDown (006FC640-006FC6A7, RET): +50h = 0,
// +18h = -0.0 - +1Ch; a platform plane is carried, unregistered and dropped;
// +50h = 2; the platform sound +54h vtable[0Ch](1.0) is not carried.
void GameUnitsHost::Impl::carrier_elevator_send_empty_down_006fc640(LandingDeck& d) {
    CarrierElevator& e = d.elevator;
    e.mode_50 = 0;
    e.p18 = -0.0f - e.p1c;
    if (e.plane_34 < slots.size()) {
        carrier_elevator_carry_006fc0d0(d);
        e.plane_34 = static_cast<std::size_t>(-1);
    }
    e.mode_50 = 2;
    ++base_launch_lift_down;
}

// 006FC810 (006FC810-006FC8E5, __thiscall(platform, plane), RET 4): for a plane
// not already on it, +50h = 0 and +18h = depth + +1Ch (the bottom); an old
// platform plane is carried, detached (00951F40(0)) and dropped; the new one is
// taken (+34h under the observer at +20h) with the offset (0, WheelHeight,
// -0.0 - planeDesc+158h); the sound is not carried; 006FC0D0 carries it; +50h = 1.
void GameUnitsHost::Impl::carrier_elevator_raise_plane_006fc810(LandingDeck& d,
                                                                GameUnitSlot& p) {
    CarrierElevator& e = d.elevator;
    if (e.plane_34 == p.process_index) return;
    e.mode_50 = 0;
    e.p18 = e.depth_48 + e.p1c;
    if (e.plane_34 < slots.size()) {
        carrier_elevator_carry_006fc0d0(d);
        if (slots[e.plane_34]) slots[e.plane_34]->plane_hit_node_detached = true;
        e.plane_34 = static_cast<std::size_t>(-1);
    }
    e.plane_34 = p.process_index;
    e.off_38[0] = 0.0f;
    e.off_38[1] = p.plane_wheel_height_1fc;
    e.off_38[2] = -0.0f - plane_desc_158(p);
    carrier_elevator_carry_006fc0d0(d);
    e.mode_50 = 1;
    ++base_launch_lift_up;
    log.notef("  carrier elevator %s: raises %s at %.2f s (006D0050 -> 007C5F60 / 006FC810; "
        "packet cc9_base_launch_deck_arms)", slots[d.owner]->row.name.c_str(),
        p.row.name.c_str(), static_cast<double>(summary.simulated_seconds));
}

// 007C3C90 BSP_Plane_RequestGroundState(flag) (007C3C90-007C3CA7): 007C1570(4 +
// flag, 0), which in single player sends C3h (current, requested) routed with 7;
// 007CCFA0's requested-4/5 arms (007CD02C / 007CD06D) take 007CA3F0(flag) when the
// current state is not 7 and 0042A7E0 (state 4 or 5) is false, as for a plane
// Locked in state 2. 007CA3F0 (007CA3F0-007CA5E6): the current state is 1..5, so
// 00954580, the tick node's vtable[0Ch] and 007C5AC0(-1.0) run (probe carried,
// the other two recorded); with a holder (+BF4h) it takes 007CA4C4: 007C1430(4 +
// flag) (+900h, +C04h = -1.0, and with MinWaterSpd +904h = +908h > 5.0 and +C18h
// = 3), then +9F0h = 0, +9F4h = 1.0, +9E4h/+9E8h/+9ECh = 0; the plane live, in
// single player: the site's vtable[24h] 006CED90 (the occupancy vector), the
// velocity +AF8h..+B00h zeroed, and 0099A4A0 (the takeoff task: 009CFF40(dl 1)
// -> 00999F50; piece 4, recorded); +9D8h == 0 clears game+193Ch (not modelled).
void GameUnitsHost::Impl::ground_state_from_locked_007ca3f0(GameUnitSlot& p, LandingDeck& d,
                                                            bool five) {
    plane_site_probe_007c5ac0(p, -1.0f);
    if (p.plane_contact_deck_bf4 == 0) return;   // 007CA4xx: the explosion arm, not reached
    bool state_changed;
    // ON 007C1430 at 007CA4D3: set_flight_state_007c1430(4 + flag) applied to +900h/+C04h/
    // +904h/+C18h (nothing stored when unchanged); OFF keeps the host's unconditional stores.
    if constexpr (kWireUnitsHostBound) {
        bsp::PlaneSetStateInputs si;
        si.requested_state = five ? 5 : 4;
        si.current_state = p.plane_control_mode_900;
        si.airborne_clock = p.plane_airborne_908;
        si.class_min_water_spd = p.plane_min_water_spd_198;
        si.landed_after_flight = p.plane_landed_904;
        const bsp::PlaneSetStateEffects fx = bsp::set_flight_state_007c1430(si);
        state_changed = fx.changed;
        p.plane_control_mode_900 = fx.state;
        if (fx.write_state_change_stamp) p.plane_site_timer_c04 = -1.0f;
        if (fx.write_landed_after_flight) p.plane_landed_904 = fx.landed_after_flight;
        if (fx.write_state_enter_marker) p.plane_c18 = 3;
    } else {
    state_changed = p.plane_control_mode_900 != (five ? 5 : 4);  // 007C144C
    p.plane_control_mode_900 = five ? 5 : 4;                            // 007C1430
    p.plane_site_timer_c04 = -1.0f;
    if (p.plane_min_water_spd_198 != 0.0f) {
        p.plane_landed_904 = p.plane_airborne_908 > 5.0f;
        p.plane_c18 = 3;
    }
    }
    if constexpr (bsp::kPlaneFlightStateNotifyCc13Bound) {
        if (state_changed) flight_state_notify_007c11e0(p);             // 007C14C5
    } else {
        static_cast<void>(state_changed);
    }
    p.plane_live_throttle = 0.0f;                                        // 007CA4E7
    p.plane_live_air_brake = 1.0f;                                       // 007CA4EF
    for (float& c : p.plane_live_controls) c = 0.0f;
    if constexpr (bsp::kPlaneGroundStateDeadTailCc13Bound) {
        // 007CA4D8 CMP [ESI+5Dh],0 / 007CA50F JNZ 007CA5D5: a flushed plane
        // (+5Dh, the host's state->simulate) skips the rest, the game+193Ch
        // clear included.
        if (unit_torn_down_005d(p.process_index)) {
            ++ground_state_dead_tail_skips;
            done("Plane::ground_state_from_locked_007ca3f0", 0x007ca3f0u);
            return;
        }
    }
    if (std::find(d.site_occupants_34.begin(), d.site_occupants_34.end(), p.process_index)
        == d.site_occupants_34.end()) {
        d.site_occupants_34.push_back(p.process_index);                  // 006CED90
    }
    for (int i = 0; i < 3; ++i) p.plane_world_velocity[i] = 0.0f;
    p.motion.linear_velocity = bsp::OceanVec3{0.0f, 0.0f, 0.0f};
    ++base_launch_ground_entries;
    install_takeoff_task_0099a4a0(p);                                    // 0099A4A0
    log.notef("base launch ground state: %s 2 -> %d at %.2f s (007C3C90 -> 007C1570 -> C3h "
        "-> 007CA3F0; packet cc9_base_launch_deck_arms)", p.row.name.c_str(),
        p.plane_control_mode_900, static_cast<double>(summary.simulated_seconds));
    done("Plane::ground_state_from_locked_007ca3f0", 0x007ca3f0u);
}

// 0099A4A0 (0099A4A0-0099A4B4, RET): MOV DL,1; 009CFF40(bot, 1) = operator
// new(4D4h) and 009CF8E0; 00999F50 pushes it. 009CF8E0 builds the approach
// (009CF710 -> 009CD420) and, with flag 1, starts in prepare (+448h), unless the
// plane is landed (+904h) on the path (+900h == 5), which starts in parking
// (+4B8h, not bound: refused). 009CD420: +34h = plane y when +BF4h is set,
// else 0; +30h = 15.0 (00CE5380; the IsKindOf(13h) arm stores the same), 25.0
// (00CE89CC) for IsKindOf(10h); +2Ch = +30h + +34h. 00999F50's push over an
// existing task is not modelled: a second install restarts the one record.
void GameUnitsHost::Impl::install_takeoff_task_0099a4a0(GameUnitSlot& p) {
    using TS = GameUnitSlot::TakeoffTaskState;
    p.takeoff_base_34 = p.plane_contact_deck_bf4 != 0 ? p.motion.position[1] : 0.0f;
    p.takeoff_margin_30 = bsp::unit_is_kind_of(p.class_id, 0x10) ? 25.0f : 15.0f;
    p.takeoff_floor_2c = p.takeoff_margin_30 + p.takeoff_base_34;
    p.takeoff_task_installed = true;
            bot_task_ctor_seeds_0099c6f0(p, false);
    ++p.takeoff_installs;
    ++takeoff_installs;
    p.takeoff_installed_at = summary.simulated_seconds;
    if (p.plane_landed_904 && p.plane_control_mode_900 == 5) {
        p.takeoff_state = TS::kParking;
        ++takeoff_parking_refused;
        log.notef("  takeoff task %s: installed in takeoff/parking at %.2f s, NOT BOUND "
            "(009CF8E0 -> +4B8h; packet cc9_takeoff_task_bind)", p.row.name.c_str(),
            static_cast<double>(summary.simulated_seconds));
    } else {
        p.takeoff_state = TS::kPrepare;
        takeoff_prepare_enter_009cdd50(p);
        log.notef("  takeoff task %s: installed in takeoff/prepare at %.2f s state=%d "
            "floor=%.2f period=%.3f phase=%.3f (0099A4A0 -> 009CF8E0 -> 009CDD50; packet "
            "cc9_takeoff_task_bind)", p.row.name.c_str(),
            static_cast<double>(summary.simulated_seconds), p.plane_control_mode_900,
            static_cast<double>(p.takeoff_floor_2c),
            static_cast<double>(p.takeoff_prep_period_24),
            static_cast<double>(p.takeoff_prep_phase_20));
    }
    done("PilotBot::install_takeoff_task_0099a4a0", 0x0099a4a0u);
}

// Packet cc9_ground_retakeoff (kGroundRetakeoffBound), docs/SQUADRON_LAND_TASK.md
// 5bp. 0099B107-0099B113: 009CFF40(bot, DL = 0) = operator new(4D4h) and
// 009CF8E0, pushed at the head by 00999F50. 009CF8E0 builds the same approach
// as 0099A4A0's install (009CF710 -> 009CD420); with DL = 0 the start state is
// takeoff/parking (+4B8h) when +904h is set and +900h == 5, else
// takeoff/Takeoff (+470h) when unit+AA0h <= 0.0 (00D7A218), else
// takeoff/SlowTakeoff (+490h). SUBSTITUTION, labelled: +AA0h is carried as 0.0
// (docs/SQUADRON_LAND_TASK.md 5be), so SlowTakeoff is not chosen. Parking is not
// bound: the task is recorded in it and holds the plane (the land task below it
// is not ticked). takeoff/Takeoff's enter 009CE270 (009CE270-009CE290): +18h =
// -1.0 (00D7A260), +1Ch = 0, then 007C17D0(plane), recorded.
// Packet cc13-takeoff: with kPlaneShipyardLaunchAa0Cc13Bound the test reads the
// carried +AA0h (009CF96D MOVSS / 009CF975 COMISS [00D7A218] / 009CF982 JA:
// SlowTakeoff only for an ordered > 0.0); its enter 009CE1D0 draws 00BD2F10, so
// that arm also needs kPlaneShipyardLaunchSlowPushCc13Bound. With
// kPlaneTakeoffStartNotifyCc13Bound, 007C17D0 runs at the Takeoff enter and its
// C3h is delivered at the end of the push (LABELLED: the image delivers at row 9
// of the same fixed step; the plane's own Takeoff step runs on its next think).
void GameUnitsHost::Impl::push_ground_retakeoff_0099b107(GameUnitSlot& p, const char* head) {
    using TS = GameUnitSlot::TakeoffTaskState;
    p.takeoff_base_34 = p.plane_contact_deck_bf4 != 0 ? p.motion.position[1] : 0.0f;
    p.takeoff_margin_30 = bsp::unit_is_kind_of(p.class_id, 0x10) ? 25.0f : 15.0f;
    p.takeoff_floor_2c = p.takeoff_margin_30 + p.takeoff_base_34;
    p.takeoff_task_installed = true;
            bot_task_ctor_seeds_0099c6f0(p, false);
    ++p.takeoff_installs;
    ++takeoff_installs;
    p.takeoff_installed_at = summary.simulated_seconds;
    ++p.retakeoff_pushes;
    ++retakeoff_pushes;
    if (p.plane_control_mode_900 == 6) ++retakeoff_state6;
    const bool parking = p.plane_landed_904 && p.plane_control_mode_900 == 5;
    bool slow = false;
    if constexpr (bsp::kPlaneShipyardLaunchAa0Cc13Bound
                  && bsp::kPlaneShipyardLaunchSlowPushCc13Bound) {
        slow = !parking && p.plane_shipyard_launch_aa0 > 0.0f;          // 009CF982 JA
    }
    if (parking) {
        p.takeoff_state = TS::kParking;
        ++takeoff_parking_refused;
        ++retakeoff_parking;
    } else if (slow) {
        p.takeoff_state = TS::kSlowTakeoff;
        takeoff_slow_enter_009ce1d0(p);                                  // 009CE1D0
        ++shipyard_launch_aa0_slow_pushes;
    } else {
        p.takeoff_state = TS::kTakeoff;
        p.takeoff_run_18 = -1.0f;
        p.takeoff_run_1c = 0.0f;
        if constexpr (bsp::kPlaneTakeoffStartNotifyCc13Bound) {
            plane_notify_takeoff_start_007c17d0(p);                      // 009CE28B JMP
        } else {
            record("Plane::takeoff_enter_007c17d0", 0x007c17d0u);
        }
        ++retakeoff_takeoff;
    }
    if (p.retakeoff_pushes <= 5) {
        log.notef("  ground retakeoff %s: push %llu at %.2f s head=%s state=%d landed=%d "
            "bf4=%zu bf8=%d y=%.2f |v|=%.2f -> %s%s (0099B0BE -> 009CFF40(DL 0) -> 00999F50; "
            "packet cc9_ground_retakeoff)", p.row.name.c_str(), p.retakeoff_pushes,
            static_cast<double>(summary.simulated_seconds), head, p.plane_control_mode_900,
            p.plane_landed_904 ? 1 : 0, static_cast<std::size_t>(p.plane_contact_deck_bf4),
            p.plane_ground_contact_bf8 ? 1 : 0, static_cast<double>(p.motion.position[1]),
            std::sqrt(static_cast<double>(p.plane_world_velocity[0]) * p.plane_world_velocity[0]
                + static_cast<double>(p.plane_world_velocity[1]) * p.plane_world_velocity[1]
                + static_cast<double>(p.plane_world_velocity[2]) * p.plane_world_velocity[2]),
            parking ? "takeoff/parking" : slow ? "takeoff/SlowTakeoff" : "takeoff/Takeoff",
            parking ? ", NOT BOUND" : "");
    }
    if constexpr (bsp::kPlaneTakeoffStartNotifyCc13Bound) {
        if (p.takeoff_notify_c3h_pending) deliver_takeoff_notify_c3h_007cd02c(p);
    }
    done("PilotBot::push_ground_retakeoff_0099b107", 0x0099b107u);
}

// Packet cc13-takeoff (kPlaneTakeoffStartNotifyCc13Bound). 007C17D0
// BSP_Plane_NotifyTakeoffStart (007C17D0-007C18A9, __thiscall(ECX = plane), RET,
// SEH frame), reached only by the JMP at 009CE28B (takeoff/Takeoff's enter).
//   007C17EB..007C17F3: byte +904h clear -> return.
//   007C17F9..007C1820: [[00E188A8]+1FE4h] is the session mode. Single player
//     runs mode 0, so neither the (cur == 2, mode 2) return nor the client arm
//     (007C187F: 007C1430(4) for 7/6/4/5) is taken here.
//   007C1822..007C1869: 0075B430(&m, C3h), +18h word 0, +1Ah 0, +4h 1, +1Ch 0,
//     vtable 00D03504, +20h = the +900h byte, +21h = 4; 0077C2A0(plane; &m, 7,
//     0). Mode 0 sets EBX = 1 at 0077C333, so 0077C44D posts it to the loopback
//     queue (0076E520) and no peer send runs; 0076C600 delivers it at row 9.
// The standalone reconstruction is plane_notify_takeoff_start_007c17d0
// (src/cc13_seg47b.cpp), verified against these bytes; this is its single-player
// path over the host's slot.
void GameUnitsHost::Impl::plane_notify_takeoff_start_007c17d0(GameUnitSlot& p) {
    ++takeoff_notify_calls;
    // Goal 5 chain 650: the body (bsp::cc13_seg47b, typed view) over the slot's
    // +904h, +900h and the session context's game+1FE4h.
    struct Exits final : bsp::cc13_seg47b::PlaneTakeoffStartNotifyHost {
        GameUnitsHost::Impl& host;
        GameUnitSlot& p;
        bool posted{false};
        Exits(GameUnitsHost::Impl& h, GameUnitSlot& s) : host(h), p(s) {}
        void send_flight_state_c3(std::uint8_t current, std::uint8_t requested) override {
            // 0076E520 writes 00762940; its C3h factory copy is constructed with
            // 00762880 and read by 007629A0. The slot retains that copy until the
            // existing row-9 delivery, with its recipient identity kept out of band.
            const auto copy =
                host.commands.session_messages().queue_plane_change_state_c3(current, requested);
            p.takeoff_notify_c3h_pending = copy.requested == 4;
            p.takeoff_notify_c3h_current = copy.current;
            posted = true;
        }
        void set_flight_state_007c1430(std::int32_t wanted) override {
            // 007C1897, a client session only (game+1FE4h == 2): 007C1430 on the
            // slot, applied as 007C1570's direct arm applies it.
            bsp::PlaneSetStateInputs si;
            si.requested_state = wanted;
            si.current_state = p.plane_control_mode_900;
            si.airborne_clock = p.plane_airborne_908;
            si.class_min_water_spd = p.plane_min_water_spd_198;
            si.landed_after_flight = p.plane_landed_904;
            const bsp::PlaneSetStateEffects fx = bsp::set_flight_state_007c1430(si);
            p.plane_control_mode_900 = fx.state;
            if (fx.write_state_change_stamp) p.plane_site_timer_c04 = -1.0f;
            if (fx.write_landed_after_flight) p.plane_landed_904 = fx.landed_after_flight;
            if (fx.write_state_enter_marker) p.plane_c18 = 3;
            if constexpr (bsp::kPlaneFlightStateNotifyCc13Bound) {
                if (fx.changed) host.flight_state_notify_007c11e0(p);
            }
        }
    } exits(*this, p);
    bsp::cc13_seg47b::plane_notify_takeoff_start_007c17d0(p.plane_landed_904,
        p.plane_control_mode_900, bsp::session_context().session_word_1fe4 == 2, exits);
    if (!exits.posted) {
        done("Plane::takeoff_enter_007c17d0", 0x007c17d0u);
        return;
    }
    ++takeoff_notify_posts;
    if (takeoff_notify_posts <= 10) {
        log.notef("  takeoff start notify %s: +904h set, state=%d airborne=%.2f taxi910=%d "
            "bf8=%d at %.2f s: C3h (%d, 4) posted (007C17D0 -> 0077C2A0 -> 0076E520; packet "
            "cc13-takeoff)", p.row.name.c_str(), p.plane_control_mode_900,
            static_cast<double>(p.plane_airborne_908), p.plane_taxi_queue_910 ? 1 : 0,
            p.plane_ground_contact_bf8 ? 1 : 0, static_cast<double>(summary.simulated_seconds),
            p.plane_control_mode_900);
    }
    done("Plane::takeoff_enter_007c17d0", 0x007c17d0u);
}

// The delivery: 0076C600 -> 00780670 -> 007CCFA0 (kind C3h: byte table
// 007CD1D8, dword table 007CD1AC -> 007CCFEF; requested 4 -> 007CD22C ->
// 007CD02C). ECX there is the message's +20h (the state at the post).
//   007CD02C: +20h == 7 -> 007CB5F0, the touchdown authority arm. Not reached
//     here: every image writer of +900h = 7 clears +904h (007C7183, 007C6481,
//     007D6600, 007C70F2: the setter's state-7 arm) and so do the host's (the
//     creation seed leaves it false), so a posted C3h never carries 7. LABELLED:
//     counted and logged if it ever does, not run.
//   007CD042: 0042A7E0, the LIVE +900h is 4 or 5 -> 007C7430. Single player
//     (mode 0) skips 007C7444's 007C71E0; then +904h = 0 when set (007C7460),
//     +910h = 0 (007C746F), +908h = 0.0 (007C7475), and +900h = 4 when +BF8h is
//     clear and +900h is 5 (007C7488, a plain store: no 007C11E0).
//   007CD05C: otherwise 007CA3F0(0), the host's ground_state_from_locked_007ca3f0
//     (with no holder it stops after the probe, as that function does).
void GameUnitsHost::Impl::deliver_takeoff_notify_c3h_007cd02c(GameUnitSlot& p) {
    p.takeoff_notify_c3h_pending = false;
    const std::int32_t sent = p.takeoff_notify_c3h_current;
    if (sent == 7) {
        ++takeoff_notify_arm_7cb5f0;
        log.notef("  takeoff start notify %s: C3h (7, 4) reached 007CB5F0, NOT RUN "
            "(007CD02C; packet cc13-takeoff)", p.row.name.c_str());
        record("Plane::touchdown_007cb5f0", 0x007cd033u);
        return;
    }
    if (p.plane_control_mode_900 == 4 || p.plane_control_mode_900 == 5) {   // 0042A7E0
        ++takeoff_notify_arm_7c7430;
        // Goal 5 chain 650: 007C7430's body
        // (bsp::cc13_seg47::plane_remote_touchdown_to_state4_007c7430) over a shadow
        // of the five plane bytes it touches (+900h, +904h, +908h, +910h, +BF8h)
        // and of the game object's +1FE4h (the session context's word), written
        // back after the call. 007B8450 runs at its own site now (the plane
        // tick's 007CE5FA, bsp::plane_landed_flag_tick_007ce598).
        struct RemoteTouchdown final : bsp::cc13_seg47::PlaneRemoteTouchdownHost {
            std::uint8_t game[0x1FE8] = {};   // 4-aligned after the vptr; +1FE4h is a dword
            void* global_pointer(std::uint32_t address) override {
                if (address != 0x00E188A8u) {
                    throw std::logic_error("007C7430 reads only the game object [00E188A8]");
                }
                return game;
            }
            void plane_touchdown_007c71e0(bsp::cc13_seg47::Raw) override {
                // 007C7447..007C744F: only +900h other than 4 and 5 reaches 007C7451,
                // and this arm runs for 4 or 5 alone.
                throw std::logic_error("007C7430: 007C71E0 is unreachable from the 0042A7E0 arm");
            }
        } touchdown;
        const std::int32_t session = bsp::session_context().session_word_1fe4;
        std::memcpy(touchdown.game + 0x1FE4, &session, sizeof(session));
        alignas(8) std::uint8_t plane[0xBFC] = {};
        const std::int32_t before = p.plane_control_mode_900;
        std::memcpy(plane + 0x900, &before, sizeof(before));
        plane[0x904] = p.plane_landed_904 ? 1u : 0u;
        std::memcpy(plane + 0x908, &p.plane_airborne_908, sizeof(float));
        plane[0x910] = p.plane_taxi_queue_910 ? 1u : 0u;
        plane[0xBF8] = p.plane_ground_contact_bf8 ? 1u : 0u;
        bsp::cc13_seg47::plane_remote_touchdown_to_state4_007c7430(touchdown,
                                                                   bsp::cc13_seg47::raw(plane));
        std::memcpy(&p.plane_control_mode_900, plane + 0x900, sizeof(std::int32_t));
        p.plane_landed_904 = plane[0x904] != 0;
        std::memcpy(&p.plane_airborne_908, plane + 0x908, sizeof(float));
        p.plane_taxi_queue_910 = plane[0x910] != 0;
        if (before == 5 && p.plane_control_mode_900 == 4) ++takeoff_notify_5_to_4;
        done("Plane::remote_touchdown_to_state4_007c7430", 0x007c7430u);
        return;
    }
    ++takeoff_notify_arm_7ca3f0;
    log.notef("  takeoff start notify %s: C3h (%d, 4) with state %d -> 007CA3F0(0) "
        "(007CD05C; packet cc13-takeoff)", p.row.name.c_str(), sent, p.plane_control_mode_900);
    if (p.plane_contact_deck_bf4 != 0 && p.plane_contact_deck_bf4 <= landing_decks.size()) {
        ground_state_from_locked_007ca3f0(p, landing_decks[p.plane_contact_deck_bf4 - 1u], false);
    } else {
        plane_site_probe_007c5ac0(p, -1.0f);
        done("Plane::ground_state_from_locked_007ca3f0", 0x007ca3f0u);
    }
}  // namespace

// The ground roll's lift-off send (007CC212 / 007CC26E): 00762A00 builds message
// C6h and 0077C2A0(unit, msg, 1, 0) routes it; LABELLED: delivered at once.
// BSP_Plane_HandleMessage's case C6h (007CCFA0) calls 007C6F50(msg byte +20h),
// which 00762A00 does not write; the flag only adds 007D83D0's push for a
// parented plane, and is taken as 0 here (LABELLED, uncertainty in 5bf).
// 007C6F50 (007C6F50-007C70FD, RET 4), +C49h clear (not carried: clear):
// - parented (+3Ch): 007D9CE0(0) takes ctl+18h back to world by adding the
//   parent's velocity (the reverse of 007C71E0's 007C7277), 004134F0 the pose
//   from +CCh, 00924F90(0) drops the parent. The host keeps the world pose.
// - +900h != 7: C3h with sub-kind 7 (+21h), routed with 7 (0077C2A0; delivered
//   at once, LABELLED) -> 007CCFA0 -> 007C7110: with +900h 4 or 5 and a holder,
//   the site's vtable[28h] 006CF180 -> 006CEF80 erases the plane from site+34h;
//   then +900h = 7, +C04h = -1.0, +910h = 0, +904h = 0, +908h = 0 from 4 or 3
//   (else 3600.0, 00CFDEB0), and 007C11E0(0) (recorded).
void GameUnitsHost::Impl::plane_liftoff_c6h_007c6f50(GameUnitSlot& p, const char* why) {
    if (p.deck_parent_plus_one != 0 && p.deck_parent_plus_one <= slots.size()) {
        const GameUnitSlot* o = slots[p.deck_parent_plus_one - 1u].get();
        // 007D9CE0(0) (007C6F8D): reparent_velocity_007d9ce0 takes ctl+18h
        // from the deck parent back to world and sets +30h; the OFF arm keeps
        // the inline addition of the parent's velocity.
        bsp::gameplay_seg48_cc13::ReparentVelocity rv;
        rv.velocity_18 = {p.plane_world_velocity[0], p.plane_world_velocity[1],
                          p.plane_world_velocity[2]};
        rv.previous_30 = {p.plane_velocity_lp_30[0], p.plane_velocity_lp_30[1],
                          p.plane_velocity_lp_30[2]};
        const bsp::gameplay_seg48_cc13::ReparentVelocity out =
            bsp::gameplay_seg48_cc13::reparent_velocity_007d9ce0(
                rv, false, o != nullptr, false, host_reparent_callbacks(o, nullptr));
        p.plane_world_velocity[0] = out.velocity_18.x;
        p.plane_world_velocity[1] = out.velocity_18.y;
        p.plane_world_velocity[2] = out.velocity_18.z;
        p.deck_parent_plus_one = 0;
        ++base_launch_liftoff_unparented;
        if constexpr (kPlaneVelocityLowpassBound) {
            p.plane_velocity_lp_30[0] = out.previous_30.x;
            p.plane_velocity_lp_30[1] = out.previous_30.y;
            p.plane_velocity_lp_30[2] = out.previous_30.z;
            p.plane_velocity_lp_valid = true;   // 007D9CE0's tail
        }
        done("PlaneFlightController::reparent_velocity_007d9ce0", 0x007d9ce0u);
    }
    const int prev = p.plane_control_mode_900;
    if (prev == 7) return;
    if ((prev == 4 || prev == 5) && p.plane_contact_deck_bf4 != 0
        && p.plane_contact_deck_bf4 <= landing_decks.size()) {
        std::vector<std::size_t>& occ = landing_decks[p.plane_contact_deck_bf4 - 1u].site_occupants_34;
        const auto it = std::find(occ.begin(), occ.end(), p.process_index);
        if (it != occ.end()) {
            occ.erase(it);
            ++base_launch_liftoff_site_leaves;
        }
    }
    if (prev == 4 || prev == 5) {
        // DIAGNOSTIC (cc9_takeoff_site_permission_diag): a lift-off that leaves the
        // plane on some site's occupancy vector.
        for (std::size_t k = 0; k < landing_decks.size(); ++k) {
            const auto& o = landing_decks[k].site_occupants_34;
            if (std::find(o.begin(), o.end(), p.process_index) == o.end()) continue;
            log.notef("  lift-off leaves %s on deck %zu (%s) occupants; its +BF4h is %zu "
                "(packet cc9_takeoff_site_permission_diag)", p.row.name.c_str(), k + 1u,
                landing_decks[k].owner < slots.size() && slots[landing_decks[k].owner]
                    ? slots[landing_decks[k].owner]->row.name.c_str() : "?",
                static_cast<std::size_t>(p.plane_contact_deck_bf4));
        }
    }
    p.plane_control_mode_900 = 7;
    p.plane_site_timer_c04 = -1.0f;
    p.plane_taxi_queue_910 = false;
    p.plane_landed_904 = false;
    p.plane_airborne_908 = (prev == 4 || prev == 3) ? 0.0f : 3600.0f;
    // 007C11E0(0) after the +900h..+908h stores: 007C71C3 when the previous state is
    // neither 3 nor 4 (+908h = [00CFDEB0], 007C71BB), 007C71D9 when it is (+908h = 0,
    // 007C71D1). Both arms make the same call, so the host has one site.
    // Packet cc13-frontier: covered by the same switch as the other notify sites.
    if constexpr (bsp::kPlaneFlightStateNotifyCc13Bound) {
        flight_state_notify_007c11e0(p);
    } else {
        record("Plane::flight_state_notify_007c11e0", 0x007c11e0u);
    }
    ++base_launch_liftoffs;
    log.notef("base launch lift-off: %s %d -> 7 at %.2f s (%s) y=%.2f |v|=%.2f "
        "(007CC1B3 -> C6h -> 007C6F50 -> C3h 7 -> 007C7110; packet cc9_base_launch_liftoff)",
        p.row.name.c_str(), prev, static_cast<double>(summary.simulated_seconds), why,
        static_cast<double>(p.motion.position[1]),
        static_cast<double>(avoid_len(p.plane_world_velocity)));
    done("Plane::begin_flying_007c7110", 0x007c7110u);
}

// Packet cc9_stowed_plane_stock_return (docs/SHIP_AI_OPEN_ITEMS.md 200).
// 007CE6CA-007CE75B in the plane's fixed step (ESI = plane+310h): +9E0h clear,
// +520h clear and the stowed byte +C00h set build message C7h carrying the holder
// owner's handle ([[[plane+BF4h]+4]+7Ch]+174h), route it (0077C2A0 with 5;
// LABELLED: delivered at once) and clear +C00h. 007CCFA0 case C7h -> 007CC8B0:
// - 007CC820 EnterFlightStateOne: +900h = 1, +C04h = -1.0, 007C11E0(0) (recorded),
//   vtable[ACh] (not carried: the plane is retired in the same step);
// - with a squadron (+9D4h), 007F1CA0(holder, plane): 006BCD20 -> 006CA770
//   AddStock(plane+538h class, 1) on the holder's block; when +3CCh == 1 (the last
//   live plane) 006C5950 returns its slot (00922F80 / +193Ch / 00696350 are not
//   carried); then 00926D90 Kill(plane, 5).
// SUBSTITUTIONS, labelled: the holder's block is the deck registry entry indexed
// by +BF4h (landing_decks and air_ops_decks share the index); the squadron is the
// registry record holding the plane and its entity id squadron_unit + 1; the
// destroy flush is taken at once, so the plane leaves the site vector +34h (the
// observer pair 006CE910 / 006CEF80) and its squadron (007BCAA0 -> 007F3970) in
// this step. OnDestroyed's +C14h draw on 00BD2F10 (007BCABB) is taken only under
// kPlaneRetireOnDestroyedDrawCc13Bound, right after the Kill.
void GameUnitsHost::Impl::plane_stock_return_c7h_007cc8b0(GameUnitSlot& p) {
    if (!p.plane_in_hangar_c00 || p.plane_death_removed) return;
    if (p.plane_airborne_frozen_9e0 || p.generic_suppress_520) return;   // 007CE6CA / 007CE6D7
    if (gunnery != nullptr && gunnery->unit_dead(p.process_index)) return;
    p.plane_in_hangar_c00 = false;                                        // 007CE75B
    ++stock_return_sends;
    p.plane_control_mode_900 = 1;                                         // 007CC857
    p.plane_site_timer_c04 = -1.0f;
    if constexpr (bsp::kPlaneFlightStateNotifyCc13Bound) {
        flight_state_notify_007c11e0(p);                                  // 007CC869
    } else {
        record("Plane::flight_state_notify_007c11e0", 0x007c11e0u);
    }
    const std::size_t di = p.plane_contact_deck_bf4 != 0 ? p.plane_contact_deck_bf4 - 1u
                                                         : static_cast<std::size_t>(-1);
    bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
    bsp::AirOpsDeck* deck = nullptr;
    {
        // Goal 4 chain 356. 007CE6ED..007CE735 stores the holder owner's id
        // ([[[plane+BF4h]+4]+7Ch]+174h; a unit's +174h is its index + 1 here)
        // at +20h of the C7h message, and 0077C2A0 flags 5 queues 0076E520's
        // rebuilt copy (00762B20, 00762BE0, 00768530 case C7h 00762B20,
        // 00762C30). 007CCFA0 case C7h resolves the copy's +20h (007BA890) and
        // hands that holder to 007CC8B0, whose stock add reaches the holder's
        // block: the registry entry bound to that id. The holder is +BF4h's
        // landing deck; its owner index past 12 bits, or an id no block is bound
        // to, keeps the index lookup (counted).
        const std::size_t owner = di < landing_decks.size() ? landing_decks[di].owner
                                                            : static_cast<std::size_t>(-1);
        if (owner != static_cast<std::size_t>(-1) && owner + 1u <= 0xfffu) {
            const bsp::HostSessionMessages::PlaneDisappearCopy copy =
                commands.session_messages().queue_plane_disappear_c7(
                    static_cast<std::uint16_t>(owner + 1u));
            ++stock_return_c7h_copies;
            done("Plane::disappear_copy_00762c30", 0x00762c30u);
            if (copy.type == 0xc7u) {
                deck = decks.find_mutable_by_entity_id(static_cast<int>(copy.holder_id));
            }
        }
        if (deck == nullptr) {
            ++stock_return_c7h_index_holder;
            deck = di < decks.size() ? decks.mutable_at(di) : nullptr;
        }
    }
    bsp::PlaneSquadronHostRecord* sq =
        bsp::plane_squadron_registry().find_by_member_unit(p.process_index);
    int stock_after = -1;
    std::size_t slots_returned = 0;
    if (sq == nullptr) {
        ++stock_return_no_squadron;
    } else if (deck == nullptr) {
        ++stock_return_no_deck;
    } else {
        // 007F1CA0 (007CC8B0 -> 007F1CA0): the typed body orders the holder's stock add, the
        // +3CCh == 1 gate over the squadron's live members and, outside session mode 2
        // (game+1FE4h), the slot return. The +3B8h/+3ECh bytes, the name call, the +404h link,
        // 00922F80, game+193Ch and 00696350 are not carried (empty), and the Kill with its
        // flush runs right after, below, as the body's last carried step. OFF keeps the inline
        // stock add, live count and slot return.
        struct HostReturnPlaneToStock {
            Impl& h;
            GameUnitSlot& p;
            bsp::PlaneSquadronHostRecord& sq;
            bsp::AirOpsDeck& deck;
            std::size_t di;
            bsp::AirOpsDeckRegistry& decks;
            int& stock_after;
            std::size_t& slots_returned;
            bool plane_is_leader() const {
                return !sq.member_units.empty() && sq.member_units[0] == p.process_index;
            }
            void leader_3b8(bool) {}
            void name_10() {}
            void link_home_base() {}
            // The holder's block: the deck registry entry indexed by +BF4h.
            bsp::AirOpsDeck* block_006bcd20() { return &deck; }
            void add_stock_006ca770(bsp::AirOpsDeck* block) {
                bsp::AirOpsStockEntry created{};
                const bsp::AirOpsStockAddResult added = bsp::air_base_stock_add_006ca770(
                    block->stock.data(), static_cast<int>(block->stock.size()),
                    static_cast<int>(block->stock.size()) + 1,
                    static_cast<std::uint32_t>(p.row.type_id), 1, &created);   // 007F1D09
                if (added.created_entry) block->stock.push_back(created);
                stock_after = added.entry_count;
                ++h.stock_return_adds;
            }
            // +3CCh: the squadron's members still alive, this plane included.
            std::int32_t member_count_3cc() const {
                std::int32_t live = 0;
                for (const std::size_t m : sq.member_units) {
                    if (m == bsp::kPlaneSquadronNoUnit || m >= h.slots.size() || !h.slots[m]) continue;
                    if (h.slots[m]->plane_death_removed) continue;
                    if (h.gunnery != nullptr && h.gunnery->unit_dead(m)) continue;
                    ++live;
                }
                return live;
            }
            void deactivate_00922f80() {}
            void clear_game_193c() {}
            void notify_00696350() {}
            std::int32_t session_mode_1fe4() const {
                return bsp::session_context().session_word_1fe4;
            }
            void return_slot_006c5950(bsp::AirOpsDeck* block) {
                if (sq.squadron_unit == bsp::kPlaneSquadronNoUnit) return;
                if constexpr (Impl::kStockReturnQueueEraseBound) {
                    // 006C59A4-006C59DC, before the slot walk (as the OFF arm).
                    if (di < h.landing_decks.size()) {
                        auto& q = h.landing_decks[di].queue;
                        const std::size_t sqi = static_cast<std::size_t>(
                            &sq - bsp::plane_squadron_registry().records().data());
                        std::size_t k = 0;
                        while (k < q.size() && q[k].squadron != sqi) ++k;
                        if (k < q.size()) {
                            q[k] = q.back();
                            q.pop_back();
                            ++h.stock_return_queue_erases;
                            h.log.notef("  stock return: squadron %s's landing-queue record erased "
                                "at %s, %zu left (006C59A4 -> 006BF7F0, packet "
                                "cc9_stock_return_queue_erase)", sq.name.c_str(),
                                decks.name_at(di).c_str(), q.size());
                        } else {
                            ++h.stock_return_queue_misses;
                        }
                    }
                }
                slots_returned = bsp::air_ops_return_squadron_slot_006c5950(
                    *block, static_cast<std::uint32_t>(sq.squadron_unit + 1u));   // 007F1D4E
                h.stock_return_slot_returns += slots_returned;
            }
            void kill_00926d90(std::int32_t) {}
            void returned_3ec() {}
        };
        HostReturnPlaneToStock ops{*this, p, *sq, *deck, di, decks, stock_after, slots_returned};
        bsp::cc15_seg48_plane_squadron_return_plane_to_stock_007f1ca0(ops);
    }
    // 007F1D57 Kill(plane, 5), and the flush taken at once.
    if (di < landing_decks.size()) {
        std::vector<std::size_t>& occ = landing_decks[di].site_occupants_34;
        const auto it = std::find(occ.begin(), occ.end(), p.process_index);
        if (it != occ.end()) {
            occ.erase(it);
            ++stock_return_site_leaves;
        }
    }
    if (sq != nullptr) {
        // Packet cc13-limbo3. The image's 007F1CA0 never calls 007F3970 here, so
        // at the Kill's flush (00959450 -> 009595F4) +3CCh still counts this
        // plane. The host clears the seat first; the pre-clear count is kept.
        p.squadron_live_3cc_at_stock_return = sq->live_count();
        for (std::size_t& member : sq->member_units) {                   // 007F3970
            if (member != p.process_index) continue;
            member = bsp::kPlaneSquadronNoUnit;
            break;
        }
        sq->formation_indices_assigned = false;
    }
    note_kill_cause(p.process_index, 5);   // 007F1D53 PUSH 5: KillReason "landed" (00E0CF04)
    p.plane_death_removed = true;
    if (gunnery != nullptr) gunnery->kill_unit_00926d90(p.process_index, 5);
    if constexpr (bsp::kPlaneRetireOnDestroyedDrawCc13Bound) {
        if (plane_on_destroyed_draw_007bcabb(p, p.process_index)) ++stock_return_destroy_draws;
    }
    if constexpr (kStockReturnDestroyBytesBound) {
        // 00926D90 Kill: +5Fh = 1 (00926DF3); +60h clear, so vtable[70h] 0077D1A0
        // tail-calls 00926C80 Destroy, +60h = 1 (00926CD5). The flush's +5Dh
        // (0092639B) is taken at once, as the death path's is.
        if (p.scene_removed_005f == 0) {
            p.scene_removed_005f = 1;
            if (p.scene_pending_destroy_0060 == 0) {
                p.scene_pending_destroy_0060 = 1;
                if (p.state != nullptr) p.state->simulate = 1;
                ++stock_return_destroys;
            }
        }
    }
    log.notef("  stock return: %s back into %s's stock (class %d -> %d) at %.2f s, squadron %s, "
        "slots returned %zu (007CE6CA C7h -> 007CC8B0 -> 007F1CA0 -> Kill 5, packet "
        "cc9_stowed_plane_stock_return)", p.row.name.c_str(),
        deck != nullptr ? decks.name_at(di).c_str() : "?", p.row.type_id, stock_after,
        static_cast<double>(summary.simulated_seconds), sq != nullptr ? sq->name.c_str() : "-",
        slots_returned);
    done("Plane::stock_return_007cc8b0", 0x007cc8b0u);
}

// 009CDD50 (009CDD50-009CDE4C, RET), takeoff/prepare's enter: +19h = +18h = 0,
// +1Ch = 0; +20h = U(0, 2 pi) (00CE3D9C); T = tuning+4DCh PrepareTime, 1.5
// (00CE380C) when 1.5 (double 00CE3D78) > T; p = T / 2 pi (double 00CE3828),
// a float; a sign -1.0 (00D7A260) when U(0, 1) > 0.5 (00CE3800), else 1.0
// (00D7A24C); +24h = sign x U(p x 0.6, 0.95 x p) (doubles 00CEFF98, 00CEFFB0).
// The draws are 00BD2F10's, here on the per-unit streams.
void GameUnitsHost::Impl::takeoff_prepare_enter_009cdd50(GameUnitSlot& p) {
    p.takeoff_prep_done_19 = false;
    p.takeoff_prep_permit_18 = false;
    p.takeoff_prep_t_1c = 0.0f;
    p.takeoff_prep_phase_20 =
        stream_set2_draw_00bd2f10(p, p.row.name + "#t20", 0.0f, 6.2831854820251465f);
    float t = lua.plane_globals_loaded() ? lua.plane_globals().pilot_take_off_prepare_time
                                         : 0.0f;
    if (1.5 > static_cast<double>(t)) t = 1.5f;
    const float per = static_cast<float>(static_cast<double>(t) / 6.2831854820251465);
    const float u = stream_set2_draw_00bd2f10(p, p.row.name + "#t24s", 0.0f, 1.0f);
    const float sign = u > 0.5f ? -1.0f : 1.0f;
    const float hi = static_cast<float>(0.949999988079071 * static_cast<double>(per));
    const float lo = static_cast<float>(static_cast<double>(per) * 0.6000000238418579);
    p.takeoff_prep_period_24 = stream_set2_draw_00bd2f10(p, p.row.name + "#t24", lo, hi)
        * sign;
}

// 009CE1D0 (009CE1D0-009CE22A, RET), takeoff/SlowTakeoff's enter: +18h..+20h =
// the plane's position, +24h = U(-0.3, 0.3) (00D06888, 00CE69C8).
void GameUnitsHost::Impl::takeoff_slow_enter_009ce1d0(GameUnitSlot& p) {
    for (int i = 0; i < 3; ++i) p.takeoff_slow_start_18[i] = p.motion.position[i];
    p.takeoff_slow_yaw_24 = stream_set2_draw_00bd2f10(p, p.row.name + "#t524",
        -0.30000001192092896f, 0.30000001192092896f);
}

// 009CDD10 (009CDD10-009CDD46, RET): +18h caches the answer. Without a holder
// (+BF4h) it is 1; otherwise the holder's site (+4h -> +3Ch) vtable[20h](plane):
// the airfield 006CF400 answers 1, the mother ship 006D01C0 the deck order.
bool GameUnitsHost::Impl::takeoff_permission_009cdd10(GameUnitSlot& p) {
    if (p.takeoff_prep_permit_18) return true;
    bool ok = true;
    if (p.plane_contact_deck_bf4 != 0 && p.plane_contact_deck_bf4 <= landing_decks.size()) {
        LandingDeck& d = landing_decks[p.plane_contact_deck_bf4 - 1u];
        if (d.mother_ship) ok = carrier_site_permission_006d01c0(d, p);
    }
    ++takeoff_permission_asks;
    if (!ok) ++takeoff_permission_denied;
    p.takeoff_prep_permit_18 = ok;
    return ok;
}

// 006D01C0 (006D01C0-006D02E5, the mother-ship site's vtable[20h], RET 4): the
// plane's own deck coordinate starts at -(L x 0.5) (FCHS, double 0.5 00D7A280),
// the best other at 9999.0 (00CE4C04). For each occupant of site+34h,
// 006BEFF0 on the carrier's holder (+B8h -> +1208h) gives z' = L x 0.5 - l.z;
// the plane's own replaces the start, every other keeps the least (<=). The
// answer is 0 when the least other is below the plane's own. The holder is
// re-framed from the carrier first (landing_deck_006c0750), as its update does.
bool GameUnitsHost::Impl::carrier_site_permission_006d01c0(LandingDeck& d,
    const GameUnitSlot& p) {
    const std::size_t di = static_cast<std::size_t>(&d - landing_decks.data());
    if (landing_deck_006c0750(di, d.owner) == nullptr) return true;
    float own = static_cast<float>(-static_cast<double>(d.length_b4) * 0.5);
    float best = 9999.0f;
    std::size_t blocker = slots.size();   // DIAGNOSTIC (cc9_takeoff_site_permission_diag)
    for (const std::size_t oi : d.site_occupants_34) {
        if (oi >= slots.size() || !slots[oi]) continue;
        const std::array<float, 3> l =
            landing_xform_004142e0(d.inverse_48, slots[oi]->motion.position);
        const float z = static_cast<float>(static_cast<double>(d.length_b4) * 0.5 - l[2]);
        if (oi == p.process_index) {
            own = z;
        } else if (z <= best) {
            best = z;
            blocker = oi;
        }
    }
    const bool permit = !(best < own);
    // DIAGNOSTIC (cc9_takeoff_site_permission_diag): who holds a denied plane back.
    if (!permit && blocker < slots.size() && (takeoff_permission_denied % 2000u) == 0u) {
        const GameUnitSlot& b = *slots[blocker];
        log.notef("  takeoff site denied: %s own_z=%.1f by %s z=%.1f state900=%d deck_bf4=%zu "
            "occupants=%zu at %.2f s (006D01C0, packet cc9_takeoff_site_permission_diag)",
            p.row.name.c_str(), static_cast<double>(own), b.row.name.c_str(),
            static_cast<double>(best), b.plane_control_mode_900,
            static_cast<std::size_t>(b.plane_contact_deck_bf4), d.site_occupants_34.size(),
            static_cast<double>(summary.simulated_seconds));
    }
    return permit;
}

// 006FC720 (flag 0 of 006D0050 with a plane): the platform at the top and still,
// the old plane dropped, this one taken with its offset from the lift, 007C2090
// (state 2 unless already 2), then down (+50h = 2).
void GameUnitsHost::Impl::carrier_elevator_take_006fc720(LandingDeck& d, GameUnitSlot& p) {
    CarrierElevator& e = d.elevator;
    if (e.plane_34 == p.process_index) return;
    e.mode_50 = 0;
    e.p18 = -0.0f - e.p1c;
    if (e.plane_34 < slots.size()) {
        carrier_elevator_carry_006fc0d0(d);
        e.plane_34 = static_cast<std::size_t>(-1);
    }
    e.plane_34 = p.process_index;
    const std::array<float, 3> l = carrier_local_point(d, p.motion.position);
    for (int k = 0; k < 3; ++k) e.off_38[k] = l[k] - e.lift[k];
    // 007C2090: the typed body skips a plane already in state 2 and a client
    // session (game+1FE4h == 2, the session context) and otherwise sends C3h
    // (+900h, 2), whose handler 007CC820 -> 007CC7A0 this host delivers at once;
    // OFF keeps the direct 007CC7A0 call.
    bsp::cc13_seg47b::plane_request_flight_state_two_007c2090(p.plane_control_mode_900,
        [] { return bsp::session_context().session_word_1fe4 == 2; },
        [this, &p](std::uint8_t, std::uint8_t) { plane_enter_state_two_007cc7a0(p); });
    e.mode_50 = 2;
    ++e.intakes;
    log.notef("  carrier elevator %s: takes %s at %.2f s offset=(%.2f %.2f %.2f) "
        "(006FC720 -> 007C2090, packet cc9_carrier_elevator)",
        slots[d.owner]->row.name.c_str(), p.row.name.c_str(),
        static_cast<double>(summary.simulated_seconds), static_cast<double>(e.off_38[0]),
        static_cast<double>(e.off_38[1]), static_cast<double>(e.off_38[2]));
}

// 006D0600 (site vtable +4h, RET 4). The ready plane +18h is never set in this
// host (the relaunch feed 006C6540 is not reconstructed), so the arms that read
// it answer as for none; +1Ch's 006CEDD0 is not reached (taken clear).
void GameUnitsHost::Impl::carrier_elevator_site_tick_006d0600(LandingDeck& d, float dt) {
    CarrierElevator& e = d.elevator;
    if (dt == 0.0f) return;
    if (d.owner >= slots.size() || !slots[d.owner]) return;
    ++e.ticks;
    if (e.mode_50 != 0) return;                                       // 006D0640
    if (0.0f > e.p18) {                                               // 006D064A: at the top
        e.timer_9c = 0.0f;
        if (e.plane_34 < slots.size()) {
            // 006D065E-006D0684: a plane brought up (the relaunch) is released
            // and asked for ground state 4 (007C3C90(0)); not reached here.
            const std::size_t up = e.plane_34;
            carrier_elevator_release_006fc250(d);
            ++e.top_releases;
            if constexpr (kBaseLaunchChainBound) {
                // Packet cc9_base_launch_deck_arms: 006D0667-006D0678 clear the
                // ready plane +18h whatever it holds, and 006D067B-006D0684 ask
                // the released plane, alive (+5Dh), for ground state 4.
                d.site_ready_18 = static_cast<std::size_t>(-1);
                ++base_launch_lift_top;
                if (up < slots.size() && slots[up]
                    && !(gunnery != nullptr && gunnery->unit_dead(up))) {
                    slots[up]->plane_hit_node_detached = false;
                    // 007C3C90(plane, 0) -> 007C1570(4, 0) (006D0684): the typed body
                    // gates the C3h message; OFF keeps the direct 007CA3F0 call.
                    request_ground_state_007c3c90(*this, *slots[up], d, false);
                }
            }
        }
        std::size_t cand = static_cast<std::size_t>(-1);
        for (const std::size_t oi : d.site_occupants_34) {
            if (oi >= slots.size() || !slots[oi]) continue;
            const GameUnitSlot& o = *slots[oi];
            if (!o.plane_landed_904) continue;
            if (!carrier_elevator_intake_006cff70(d, o)) continue;
            if (!(1.3888889f > avoid_len(o.plane_world_velocity))) continue;   // 00CF8AAC
            // 007B8D40: channel +44h (the wings, not the gear) disabled or at
            // exactly 0.0. Unbound, the channel stays disabled and this is true.
            if constexpr (kPlaneWingFoldElevatorBound) {
                if (!wing_at_007b8d10(o.actuator_block_dec, 0.0f)) {
                    ++wing_fold_elevator_waits;
                    continue;
                }
            }
            cand = oi;
        }
        if (cand < slots.size()) {
            // 006D0703..006D071D: 006CFFF0(0, cand) -> 006D0050 -> 006FC720.
            route_carrier_elevator_message_87(d, false, cand);
        } else if constexpr (kBaseLaunchChainBound) {
            // Packet cc9_base_launch_deck_arms, 006D0707-006D071D: no landed
            // candidate, a ready plane +18h and 006D02F0 false: 006CFFF0(0, 0) ->
            // 006D0050 -> 006FC640, the empty platform goes down.
            if (d.site_ready_18 < slots.size()) {
                if (carrier_elevator_blocked_006d02f0(d)) {
                    ++base_launch_lift_blocked;
                } else {
                    route_carrier_elevator_message_87(d, false, static_cast<std::size_t>(-1));
                }
            }
        }
        return;
    }
    if (!(e.p18 > e.depth_48)) return;                                // 006D0738
    if (e.plane_34 < slots.size()) {
        GameUnitSlot& p = *slots[e.plane_34];
        // 007B96C0 (006D0600's call): the typed body sets +C00h and runs 00951F40(0),
        // this host's node detach, before 006FC250; OFF keeps the store alone.
        bsp::cc13_seg47::plane_detach_into_hangar_007b96c0(p.plane_in_hangar_c00,
            [&p](std::int32_t visible) { p.plane_hit_node_detached = visible == 0; });
        log.notef("  carrier elevator %s: stows %s at %.2f s (007B96C0 / 006FC250, packet "
            "cc9_carrier_elevator)", slots[d.owner]->row.name.c_str(), p.row.name.c_str(),
            static_cast<double>(summary.simulated_seconds));
        carrier_elevator_release_006fc250(d);
        ++e.hides;
        e.timer_9c = 0.0f;
    } else {
        e.timer_9c = e.timer_9c + dt;
    }
    const bsp::GameTuningBlock& g = lua.plane_globals();
    if (e.timer_9c > g.pilot_landing_lift_delay && e.plane_34 >= slots.size()) {
        // 006D0786..006D07A1: 006CFFF0(1, +18h), then +9Ch = 0. Packet
        // cc9_base_launch_deck_arms: with the ready plane, 006D0050's flag-1 arm
        // places and raises it; +18h stays set until the top (006D0667).
        std::size_t ready = static_cast<std::size_t>(-1);
        if constexpr (kBaseLaunchChainBound) {
            if (d.site_ready_18 < slots.size() && slots[d.site_ready_18]) ready = d.site_ready_18;
        }
        if (ready >= slots.size()) ++e.unfed_relaunch;
        route_carrier_elevator_message_87(d, true, ready);
        e.timer_9c = 0.0f;
    }
}

void GameUnitsHost::Impl::route_carrier_elevator_message_87(LandingDeck& d, bool flag,
                                                         std::size_t plane) {
    // 006CFFF0 (its body, cc13_sweepC, runs inside the queue call): 00758B90
    // (plane, flag) on the stack, 0077C2A0(msg, 5, 0) to the carrier
    // (site+B8h); the local bit sends it through 0076E520's copy, and the
    // carrier's kind switch runs 006D0050 on it. +174h is this process's
    // unit index + 1 (GameScriptOrdersHost::entity_object_id). LABELLED: the
    // copy is delivered in this step, as the direct calls it replaces were; an
    // index the copy's 12 bits cannot carry keeps the posted plane.
    bool copy_flag = flag;
    std::size_t copy_plane = plane < slots.size() ? plane : static_cast<std::size_t>(-1);
    if (copy_plane >= slots.size() || copy_plane + 1u <= 0xfffu) {
        const bool has_plane = copy_plane < slots.size();
        const bsp::HostSessionMessages::ElevatorMoveCopy copy =
            commands.session_messages().queue_mothership_elevator_move_87(flag, has_plane,
                static_cast<std::uint16_t>(has_plane ? copy_plane + 1u : 0u));
        copy_flag = copy.flag;
        // 00758CF0: the +20h entity when +23h, else null.
        copy_plane = copy.has_plane && copy.plane_id != 0
            ? static_cast<std::size_t>(copy.plane_id) - 1u : static_cast<std::size_t>(-1);
    }
    CarrierElevator& e = d.elevator;
    if (copy_plane >= slots.size() || !slots[copy_plane]) {
        if (!copy_flag) {
            carrier_elevator_send_empty_down_006fc640(d);              // 006FC640
        } else {
            // 006FC6B0: the empty platform goes up.
            e.p18 = e.depth_48 + e.p1c;
            e.mode_50 = 1;
            ++e.empty_up;
        }
    } else if (!copy_flag) {
        carrier_elevator_take_006fc720(d, *slots[copy_plane]);         // 006FC720
    } else {
        // 007C5F60(plane, carrier+1188h), then 006FC810 brings it up.
        GameUnitSlot& p = *slots[copy_plane];
        const std::size_t di = static_cast<std::size_t>(&d - landing_decks.data());
        if (place_on_launch_spot_007c5f60(p, di, d, false)) {
            carrier_elevator_raise_plane_006fc810(d, p);
        }
    }
    // 006D0050's tail 006FBCE0 runs only for a client (game+1FE4h == 2).
}

// 006FC480 (through 006CFE40, slot +8h, when dt != 0 and the owner exists).
void GameUnitsHost::Impl::carrier_elevator_platform_tick_006fc480(LandingDeck& d, float dt) {
    CarrierElevator& e = d.elevator;
    if (dt == 0.0f) return;
    if (d.owner >= slots.size() || !slots[d.owner]) return;
    const bool moving = e.mode_50 != 0;
    if (e.plane_34 < slots.size() && gunnery != nullptr && gunnery->unit_dead(e.plane_34)) {
        e.plane_34 = static_cast<std::size_t>(-1);                   // 006FC498 (+5Dh)
    }
    const float step = static_cast<float>(static_cast<double>(e.speed_44 * dt) * e.p4c);
    if (e.mode_50 == 1) {
        e.p18 = e.p18 - step;
        if (!(0.0f < e.p18) && -e.p1c > e.p18) {
            e.p18 = -e.p1c;
            e.mode_50 = 0;
        }
    } else if (e.mode_50 == 2) {
        e.p18 = step + e.p18;
        if (!(e.p18 < e.depth_48)) {
            const float bottom = e.depth_48 + e.p1c;
            if (!(e.p18 < bottom)) {
                e.p18 = bottom;
                e.mode_50 = 0;
            }
        }
    }
    if (e.plane_34 < slots.size()) carrier_elevator_carry_006fc0d0(d);
    if (!moving) e.p4c = 1.0f;                                        // 006FC5EA
}

// 007D99C0 (007D99C0-007D9A69, __thiscall(ctl), RET, ST0): ctl+44h, plus with
// a scene parent the parent velocity's z in the plane frame x (0.1 | 0.8).
// 007D9A4E-007D9A5F: FLD z, FMUL c, FADD ctl+44h, FSTP; per-op float rounding.
float GameUnitsHost::Impl::plane_forward_speed_007d99c0(const GameUnitSlot& p,
                                                        float ctl_44) const {
    if constexpr (!kPlaneForwardSpeedDeckTermBound) return ctl_44;
    if (p.deck_parent_plus_one == 0 || p.deck_parent_plus_one > slots.size()) return ctl_44;
    const GameUnitSlot* o = slots[p.deck_parent_plus_one - 1u].get();
    if (o == nullptr) return ctl_44;
    const bsp::OceanVec3& pv = o->motion.linear_velocity;
    const float* const r2 = p.motion.pose_row2;
    const float z = r2[0] * pv.x + r2[1] * pv.y + r2[2] * pv.z;
    const float c = z >= 0.0f ? 0.1f : 0.8f;
    const float t = static_cast<float>(static_cast<double>(z) * static_cast<double>(c));
    ++forward_speed_deck_terms;
    return static_cast<float>(static_cast<double>(t) + static_cast<double>(ctl_44));
}

// 007C71E0 (007C71E0-007C742F), the re-parent arm (007C726C-007C72C5) and the
// ship arm (007C72D3-007C7417), for a mother-ship holder.
void GameUnitsHost::Impl::carrier_deck_reparent_007c71e0(GameUnitSlot& p, const LandingDeck& d) {
    const GameUnitSlot& o = *slots[d.owner];
    // 007D9CE0 (007C7277): ctl+18h -= owner vtable[34h]. LABELLED: the owner's
    // velocity is its motion.linear_velocity; the host keeps it in world axes.
    const float vw[3] = {p.plane_world_velocity[0], p.plane_world_velocity[1],
        p.plane_world_velocity[2]};
    {
        // 007D9CE0 (007C7277): reparent_velocity_007d9ce0 moves ctl+18h from the
        // current parent (none, or another deck) to the carrier and sets +30h;
        // the OFF arm keeps the inline subtraction of the carrier's velocity.
        const GameUnitSlot* const old_parent =
            p.deck_parent_plus_one != 0 && p.deck_parent_plus_one <= slots.size()
                ? slots[p.deck_parent_plus_one - 1u].get() : nullptr;
        bsp::gameplay_seg48_cc13::ReparentVelocity rv;
        rv.velocity_18 = {p.plane_world_velocity[0], p.plane_world_velocity[1],
                          p.plane_world_velocity[2]};
        rv.previous_30 = {p.plane_velocity_lp_30[0], p.plane_velocity_lp_30[1],
                          p.plane_velocity_lp_30[2]};
        const bsp::gameplay_seg48_cc13::ReparentVelocity out =
            bsp::gameplay_seg48_cc13::reparent_velocity_007d9ce0(
                rv, p.deck_parent_plus_one == d.owner + 1u, old_parent != nullptr, true,
                host_reparent_callbacks(old_parent, &o));
        p.plane_world_velocity[0] = out.velocity_18.x;
        p.plane_world_velocity[1] = out.velocity_18.y;
        p.plane_world_velocity[2] = out.velocity_18.z;
        if constexpr (kPlaneVelocityLowpassBound) {
            p.plane_velocity_lp_30[0] = out.previous_30.x;
            p.plane_velocity_lp_30[1] = out.previous_30.y;
            p.plane_velocity_lp_30[2] = out.previous_30.z;
            p.plane_velocity_lp_valid = true;   // 007D9CE0's tail
        }
        done("PlaneFlightController::reparent_velocity_007d9ce0", 0x007d9ce0u);
    }
    // 007BA020 / vtable[ACh] (007C7287-007C72B1): the parent and the local pose.
    p.deck_parent_plus_one = d.owner + 1u;
    p.deck_stop_logged = false;
    carrier_deck_capture(p);
    ++carrier_deck_parented;
    // 007C72D3: the owner answers IsKindOf(9). 007C72DD-007C730E: the body
    // (ctl+3Ch, ctl+44h) after 007D9C10, above 3.3333 (00D05B48, a double).
    const float* const v = p.plane_world_velocity;
    float bx = 0.0f;
    float bz = 0.0f;
    {
        // 007D9C10 (007C72C5): world_to_body_007d9c10 rebuilds ctl+3Ch..44h from
        // ctl+18h through ctl+0B0h; the OFF arm keeps the two inline dot products.
        bsp::ControllerVelocities vel;
        for (int i = 0; i < 3; ++i) vel.linear_world[i] = v[i];
        bsp::world_to_body_007d9c10(vel, host_pose_world_to_body(p));
        bx = vel.linear_body[0];   // ctl+3Ch, read at 007C72DD
        bz = vel.linear_body[2];   // ctl+44h, read at 007C72E5
        done("PlaneFlightController::refresh_body_frame_007d9c10", 0x007d9c10u);
    }
    const float spd = landing_len2_00414c60(bx, bz);
    bool seeded = false;
    float diff = 0.0f, z = 0.0f, arg = 0.0f;
    if (static_cast<double>(spd) > 3.3333332538604736) {
        // 007C7314-007C7378: 006BCA80(holder, t 0).w = wrap(+88h + 0), against
        // plane vtable[50h] through 00438B10(heading, w); |.| < 1.0472 (00D05AAC).
        const float w = bsp::wrapped_angle_add_00438aa0(d.runway_heading_88, 0.0f);
        diff = std::fabs(bsp::wrapped_angle_subtract_00438b10(p.plane_heading_c6c, w));
        if (1.0471976f > diff) {
            // 007C737E-007C73C5: 006BCC90's z < 006BA620() x 2.8 (00D05A38).
            const std::array<float, 3> l = landing_local_006bcc90(d, p.motion.position);
            z = l[2];
            const double L = landing_run_length_006ba620(d);
            if (L * 2.799999952316284 > static_cast<double>(z)) {
                // 007C73C7-007C7417: 00415620 clamps z - 2.0 (00D7A308) into
                // [1.0, 16.0] (00D7A24C, 00CE6454); x 0.5 (00D7A280) -> 007DB630.
                float c = static_cast<float>(static_cast<double>(z) - 2.0);
                if (1.0f > c) c = 1.0f;
                else if (c > 16.0f) c = 16.0f;
                arg = static_cast<float>(static_cast<double>(c) * 0.5);
                // 007DB630: ctl+ACh = min(arg, tuning+51Ch MaxWireRope).
                const float max_wire = lua.plane_globals_loaded()
                    ? lua.plane_globals().pilot_landing_max_wire_rope : 100.0f;
                p.plane_wire_ac = max_wire < arg ? max_wire : arg;
                seeded = true;
                ++carrier_deck_wire_seeded;
            }
        }
    }
    log.notef("carrier deck parent: unit=%s carrier=%s t=%.2f world v=(%.2f %.2f %.2f) relative "
        "v=(%.2f %.2f %.2f) body |(vx,vz)|=%.2f local=(%.2f %.2f %.2f) heading diff=%.4f "
        "z-less-T=%.2f wire seed=%d wire=%.3f (007C71E0 007D9CE0 007C72D3 007DB630; packet "
        "cc9_carrier_landing_deck_part2)", p.row.name.c_str(), o.row.name.c_str(),
        summary.simulated_seconds, static_cast<double>(vw[0]), static_cast<double>(vw[1]),
        static_cast<double>(vw[2]), static_cast<double>(v[0]), static_cast<double>(v[1]),
        static_cast<double>(v[2]), static_cast<double>(spd),
        static_cast<double>(p.deck_local_pos[0]), static_cast<double>(p.deck_local_pos[1]),
        static_cast<double>(p.deck_local_pos[2]), static_cast<double>(diff),
        static_cast<double>(z), seeded ? 1 : 0, static_cast<double>(p.plane_wire_ac));
    record("Plane::touchdown_reparent_007c71e0", 0x007c71e0u);
}

// 006BCA40 (RET 4): T through the holder's matrix (+8h).
std::array<float, 3> GameUnitsHost::Impl::landing_t_world_006bca40(const LandingDeck& d) const {
    return landing_xform_004142e0(d.frame_8, d.t_a4);
}

// 006BA620 (006BA620-006BA65A, RET): RunwayLength x 0.4 (00CE65D0) for an
// airfield owner, x 0.3 (00CE3DC8) when the owner answers IsKindOf(9)
// (006BA62F-006BA63D); both through a float store.
float GameUnitsHost::Impl::landing_run_length_006ba620(const LandingDeck& d) const {
    // ON 006BA620: air_ops_approach_setback_006ba620(holder+B4h, IsKindOf(9)); OFF keeps the host copy below.
    if constexpr (kWireUnitsHostBound) {
        return bsp::band_006b_cc13::air_ops_approach_setback_006ba620(d.length_b4, d.mother_ship);
    } else {
    if (d.mother_ship) {
        return static_cast<float>(static_cast<double>(d.length_b4) * 0.30000001192092896);
    }
    return static_cast<float>(static_cast<double>(d.length_b4) * 0.4000000059604645);
    }
}

float GameUnitsHost::Impl::landing_t_distance(const LandingDeck& d, const GameUnitSlot& p) const {
    const std::array<float, 3> tw = landing_t_world_006bca40(d);
    return landing_len2_00414c60(p.motion.position[0] - tw[0], p.motion.position[2] - tw[2]);
}

// 006C46B0: the horizontal distance to T is above StandbyDist (tuning+500h).
bool GameUnitsHost::Impl::landing_outside_standby_006c46b0(const LandingDeck& d,
    const GameUnitSlot& p) const {
    return landing_t_distance(d, p) > lua.plane_globals().pilot_landing_standby_dist;
}

// 006C45C0: airborne and above StandbyDist x 1.2 (00CEC160) from T.
bool GameUnitsHost::Impl::landing_released_006c45c0(const LandingDeck& d,
    const GameUnitSlot& p) const {
    if (!landing_airborne(p)) return false;
    return static_cast<double>(landing_t_distance(d, p)) >
        static_cast<double>(lua.plane_globals().pilot_landing_standby_dist) * 1.2000000476837158;
}

// 006C3B10 (006C3B10-006C3E41, RET 4): the runway corridor.
bool GameUnitsHost::Impl::landing_corridor_006c3b10(const LandingDeck& d,
    const GameUnitSlot& p) const {
    const bsp::GameTuningBlock& g = lua.plane_globals();
    // +904h clear and +900h 2 or 4 answers false.
    if (!(kLandingLandedArmBound && p.plane_landed_904)
        && (p.plane_control_mode_900 == 2 || p.plane_control_mode_900 == 4)) return false;
    if (!landing_airborne(p)) return true;
    // 00923810(1), the parent link of an airborne plane: null here (LABELLED),
    // so the AirField/MotherShip parent arm never answers.
    const std::array<float, 3> l = landing_local_006bcc90(d, p.motion.position);
    const double L = landing_run_length_006ba620(d);
    float zp;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // pc24w1-sweep3-6: 006C3BD7 FMUL 1.5, 006C3BDD FSUBR, rounded at PC24.
        zp = static_cast<float>(bsp::cc13x87::x87_sub24(l[2], bsp::cc13x87::x87_mul24(L, 1.5)));
    } else {
        zp = static_cast<float>(static_cast<double>(l[2]) - L * 1.5);   // 00CE3D78
    }
    const double tan_a = static_cast<double>(static_cast<float>(
        std::tan(static_cast<double>(g.pilot_landing_approach_angle))));        // 00412E20
    double ceiling;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 006C3C01 FMUL zp (rounded), 006C3C05 doubled, 006C3C07 FSUBR 5.0 (rounded).
        const double tz = bsp::cc13x87::x87_mul24(tan_a, zp);
        ceiling = bsp::cc13x87::x87_sub24(5.0, tz + tz);
    } else {
        ceiling = 5.0 - 2.0 * (tan_a * zp);
    }
    if (static_cast<double>(l[1]) > ceiling) return false;                      // 00D7A370
    const float w = static_cast<float>(static_cast<double>(d.width_b0) * 0.5);
    float e;
    if (l[0] > 0.0f) {
        const float v = l[0] - w;
        e = v > 0.0f ? v : 0.0f;                                   // 00415550 max
    } else {
        const float v = l[0] + w;
        e = v < 0.0f ? v : 0.0f;                                   // 00415510 min
    }
    const float ev[3] = {e, l[1], zp};
    float over;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // pc24w1-sweep3-6: 006C3C85 FMUL 1.5, 006C3C8B FSUBR, rounded at PC24.
        over = static_cast<float>(bsp::cc13x87::x87_sub24(avoid_len(ev), bsp::cc13x87::x87_mul24(L, 1.5)));
    } else {
        over = static_cast<float>(static_cast<double>(avoid_len(ev)) - L * 1.5);
    }
    const float pb11 = static_cast<float>(
        static_cast<double>(g.pilot_landing_pos_behind) * 1.100000023841858);  // 00CE3DF0
    if (over > pb11) return false;
    const float h = p.plane_heading_c6c;                           // plane->vtable[50h]
    const std::array<float, 3> tw = landing_t_world_006bca40(d);
    float b = bsp::heading_angle_00414eb0(std::array<float, 2>{
        tw[0] - p.motion.position[0], tw[2] - p.motion.position[2]});   // 00427C90(T - pos)
    if (std::fabs(e) < 1.0f && static_cast<double>(zp) > -2.0 * L) b = d.runway_heading_88;
    const float d1 = std::fabs(bsp::wrapped_angle_subtract_00438b10(h, b));
    const float d2 = std::fabs(bsp::wrapped_angle_subtract_00438b10(h, d.runway_heading_88));
    if (!(1.5707963705062866 > static_cast<double>(d2))) return false;          // 00CE3830
    const float thr = bsp::clamped_interpolate_00419010(0.0f, 0.5235987901687622f,
        static_cast<float>(static_cast<double>(g.pilot_landing_pos_behind) * 1.25),  // 00CF87C0
        1.5707963705062866f, over);
    return thr > d1;
}

// 006C5380 (__thiscall(holder)(out, plane, side), RET 0Ch): the circle point.
std::array<float, 3> GameUnitsHost::Impl::landing_circle_point_006c5380(const LandingDeck& d,
    const GameUnitSlot& p, bool side) const {
    const bsp::GameTuningBlock& g = lua.plane_globals();
    // 006BEF70: the plane's squadron's queue n, or -1.
    int n = -1;
    const std::size_t sq = landing_squadron_of(p.process_index);
    for (const LandingQueueEntry& q : d.queue) {
        if (q.squadron == sq) { n = q.n; break; }
    }
    const float nf = static_cast<float>(n);
    float n60 = static_cast<float>(60.0 * static_cast<double>(nf));    // 00CE3D68
    const float n20 = static_cast<float>(static_cast<double>(nf) * 20.0);  // 00CE3D88
    const float r = landing_radius_006c3e50(d, p);
    const float sr = static_cast<float>(static_cast<double>(r) * (side ? 1 : -1));
    if (bsp::unit_is_kind_of(p.class_id, 0x10) || bsp::unit_is_kind_of(p.class_id, 0x16)) {
        n60 = static_cast<float>(static_cast<double>(n60) + 250.0);   // 00CF8850
    }
    const float local[3] = {d.t_a4[0] + sr, n20 + g.pilot_landing_pos_alt,
                            -g.pilot_landing_pos_behind - n60};
    return landing_xform_004142e0(d.frame_8, local);
}

// 006C3E50 (RET 4, returns ST0): f^2 x 007C6760(plane), f from the record count.
float GameUnitsHost::Impl::landing_radius_006c3e50(const LandingDeck& d,
    const GameUnitSlot& p) const {
    const bsp::GameTuningBlock& g = lua.plane_globals();
    const float f = bsp::clamped_interpolate_00419010(g.pilot_landing_radius_change_1, 1.0f,
        g.pilot_landing_radius_change_2, 2.5f, static_cast<float>(d.assignments.size()));
    // 006C3F28 calls 007C6760 for this assigned plane. Feed the class and
    // tuning mirrors already loaded by the scene; its result supplies the
    // circle radius used by the live landing geometry below.
    const bool kind_10 = bsp::unit_is_kind_of(p.class_id, 0x10);
    const bool kind_12_14_11 = !kind_10 &&
        (bsp::unit_is_kind_of(p.class_id, 0x12) ||
         bsp::unit_is_kind_of(p.class_id, 0x14) ||
         bsp::unit_is_kind_of(p.class_id, 0x11));
    const float turn = bsp::cc13_seg47::landing_circle_radius_007c6760(
        p.plane_turn_circle_radius, g.pilot_landing_circle_multiplier_min,
        g.pilot_landing_circle_multiplier_max, kind_10, kind_12_14_11);
    // 006C3F35/3F38: retain the caller's two PC24 multiplies by f.
    return static_cast<float>(bsp::cc13x87::x87_mul24(
        bsp::cc13x87::x87_mul24(turn, f), f));
}

// 006C5C40 (006C5C40-006C5E18, RET 4): on the landing circle.
bool GameUnitsHost::Impl::landing_on_circle_006c5c40(const LandingDeck& d,
    const GameUnitSlot& p) const {
    // Null or landed (+904h) answers false.
    if (kLandingLandedArmBound && p.plane_landed_904) return false;
    const std::array<float, 3> l = landing_local_006bcc90(d, p.motion.position);
    const bool side = l[0] > 0.0f;                               // 006C5C8F COMISS/JA
    const std::array<float, 3> c = landing_circle_point_006c5380(d, p, side);
    const float vx = c[0] - p.motion.position[0];
    const float vz = c[2] - p.motion.position[2];
    const float dd = landing_len2_00414c60(vx, vz);
    const float r = landing_radius_006c3e50(d, p);
    if (static_cast<double>(dd) > static_cast<double>(r) * 1.600000023841858) return false;
    const float off = side ? 1.5707963705062866f : -1.5707963705062866f;
    const float a = static_cast<float>(static_cast<double>(
        bsp::heading_angle_00414eb0(std::array<float, 2>{vx, vz})) - static_cast<double>(off));
    const float s = bsp::wrapped_angle_subtract_00438b10(a, p.plane_heading_c6c);
    const float delta = s > 0.0f ? s : -0.0f - s;                   // 00D7A208
    const float thr = bsp::clamped_interpolate_00419010(
        static_cast<float>(static_cast<double>(r) * 0.25), 0.7853981852531433f,
        static_cast<float>(static_cast<double>(r) * 0.800000011920929), 0.3490658700466156f, dd);
    return thr > delta;
}

void GameUnitsHost::Impl::landing_head_circle_diag(const LandingDeck& d, const GameUnitSlot& p,
                                                   int old_mode, int new_mode) {
    const std::array<float, 3> l = landing_local_006bcc90(d, p.motion.position);
    const bool side = l[0] > 0.0f;
    const std::array<float, 3> c = landing_circle_point_006c5380(d, p, side);
    const float vx = c[0] - p.motion.position[0];
    const float vz = c[2] - p.motion.position[2];
    const float dd = landing_len2_00414c60(vx, vz);
    const float r = landing_radius_006c3e50(d, p);
    const float off = side ? 1.5707963705062866f : -1.5707963705062866f;
    const float a = static_cast<float>(static_cast<double>(
        bsp::heading_angle_00414eb0(std::array<float, 2>{vx, vz})) - static_cast<double>(off));
    const float delta = std::fabs(bsp::wrapped_angle_subtract_00438b10(a, p.plane_heading_c6c));
    const float thr = bsp::clamped_interpolate_00419010(
        static_cast<float>(static_cast<double>(r) * 0.25), 0.7853981852531433f,
        static_cast<float>(static_cast<double>(r) * 0.800000011920929), 0.3490658700466156f, dd);
    // The same test against the point and side the standby steer flies (+38h, +44h).
    const float ox = p.land_circle_38[0] - p.motion.position[0];
    const float oz = p.land_circle_38[2] - p.motion.position[2];
    const float od = landing_len2_00414c60(ox, oz);
    const float ooff = p.land_side_44 ? 1.5707963705062866f : -1.5707963705062866f;
    const float oa = static_cast<float>(static_cast<double>(
        bsp::heading_angle_00414eb0(std::array<float, 2>{ox, oz})) - static_cast<double>(ooff));
    const float odelta = std::fabs(bsp::wrapped_angle_subtract_00438b10(oa, p.plane_heading_c6c));
    const float pmx = c[0] - p.land_circle_38[0];
    const float pmz = c[2] - p.land_circle_38[2];
    const float point_moved = landing_len2_00414c60(pmx, pmz);
    ++lhc_n;
    const bool beyond = static_cast<double>(dd) > static_cast<double>(r) * 1.600000023841858;
    const bool on = !beyond && thr > delta;
    if (on) ++lhc_on;
    else if (beyond) ++lhc_far;
    else {
        ++lhc_heading;
        lhc_delta_fail_sum += delta;
    }
    if (side != p.land_side_44) ++lhc_side_moved;
    if (odelta < 1.5707963705062866f) ++lhc_own_ok;
    if (static_cast<double>(od) > static_cast<double>(p.land_radius_48) * 1.6) ++lhc_own_far;
    lhc_dd_r_sum += r > 0.0f ? dd / r : 0.0f;
    lhc_own_d_r_sum += p.land_radius_48 > 0.0f ? od / p.land_radius_48 : 0.0f;
    lhc_r_moved_sum += std::fabs(r - p.land_radius_48);
    lhc_point_moved_sum += point_moved;
    if (old_mode == 3 && new_mode != 3) {
        ++lhc_leave;
        if (lhc_trace < 60) {
            ++lhc_trace;
            log.notef("  land head circle leave %s t=%.2f mode %d->%d side_now=%d side44=%d "
                "x_local=%.1f dd=%.1f r=%.1f r44=%.1f delta=%.3f thr=%.3f own_d=%.1f "
                "own_delta=%.3f point_moved=%.1f records=%zu (006C5C40, packet "
                "cc9_land_head_chatter diagnostic)", p.row.name.c_str(),
                static_cast<double>(summary.simulated_seconds), old_mode, new_mode,
                side ? 1 : 0, p.land_side_44 ? 1 : 0, static_cast<double>(l[0]),
                static_cast<double>(dd), static_cast<double>(r),
                static_cast<double>(p.land_radius_48), static_cast<double>(delta),
                static_cast<double>(thr), static_cast<double>(od),
                static_cast<double>(odelta), static_cast<double>(point_moved),
                d.assignments.size());
        }
    }
}

// 006C5E20 (RET 4): TRUE when 006C3B10 or 006C5C40 answers (006C6010); else the
// StandbyDist gate and the heading test against the circle point.
bool GameUnitsHost::Impl::landing_turn_in_006c5e20(const LandingDeck& d,
    const GameUnitSlot& p) const {
    if (landing_corridor_006c3b10(d, p) || landing_on_circle_006c5c40(d, p)) return true;
    const bsp::GameTuningBlock& g = lua.plane_globals();
    if (landing_t_distance(d, p) > g.pilot_landing_standby_dist) return false;
    const std::array<float, 3> l = landing_local_006bcc90(d, p.motion.position);
    const bool side = l[0] >= 0.0f;                              // 006C5EFA COMISS/JC
    const std::array<float, 3> c = landing_circle_point_006c5380(d, p, side);
    const float vx = c[0] - p.motion.position[0];
    const float vz = c[2] - p.motion.position[2];
    const float delta = std::fabs(bsp::wrapped_angle_subtract_00438b10(
        bsp::heading_angle_00414eb0(std::array<float, 2>{vx, vz}), p.plane_heading_c6c));
    const float thr = bsp::clamped_interpolate_00419010(g.pilot_landing_pos_behind,
        1.3962634801864624f,                                               // 00CF8858
        static_cast<float>(static_cast<double>(g.pilot_landing_standby_dist) * 0.6000000238418579),
        0.5235987901687622f, landing_len2_00414c60(vx, vz));               // 00CEC724
    return thr > delta;
}

// 006BED60: block+1Ch/+1Dh clear and the owner at +7Ch present with +5Dh
// clear (the network-remote byte, clear in a single-player mission).
bool GameUnitsHost::Impl::landing_deck_usable_006bed60(std::size_t deck_index) const {
    const bsp::AirOpsDeck* deck = bsp::air_ops_decks().mutable_at(deck_index);
    return deck != nullptr && !deck->runway_failure && !deck->hangar_failure;
}

// 006C7960 (__thiscall(block)(rec, leaderPass, headMode, headDist), RET 10h).
void GameUnitsHost::Impl::landing_mode_006c7960(LandingDeck& d, std::size_t deck_index,
    LandingAssignment& rec, bool leader_pass, int head_mode, float head_dist) {
    const GameUnitSlot& p = *slots[rec.plane];
    // 006C7970-006C798A: +904h (landed) first, on both passes.
    if (kLandingLandedArmBound && p.plane_landed_904) {
        rec.mode_10 = 4;
        rec.path_4 = -1.0f;                                       // 00D7A260
        rec.spacing_8 = 1.0f;                                     // 00D7A24C
        ++d.landed_arm_hits;
        return;
    }
    if (!landing_airborne(p) || rec.held_0c) {
        rec.mode_10 = leader_pass ? 2 : 1;
        rec.path_4 = 999999.0f;                                   // 00CF87D0
        rec.spacing_8 = 1.0f;
        return;
    }
    // Squadron +3B0h: only ever cleared, so false.
    int mode = 3;
    if (leader_pass) {
        if (landing_corridor_006c3b10(d, p)) {
            mode = (landing_deck_usable_006bed60(deck_index) && rec.spacing_8 > 0.0f) ? 4 : 3;
        } else if (landing_on_circle_006c5c40(d, p)) {
            mode = 3;
        } else {
            mode = landing_outside_standby_006c46b0(d, p) ? 1 : 2;
        }
    } else if (head_mode == 2) {
        mode = 1;
        if (!landing_outside_standby_006c46b0(d, p)) {
            const std::vector<std::size_t> members =
                landing_members(landing_squadron_of(rec.plane));
            if (!members.empty() && landing_turn_in_006c5e20(d, *slots[members[0]])) mode = 2;
        }
    } else if (head_mode == 1) {
        mode = 1;
    } else if (head_mode == 3) {
        mode = 3;
    } else {
        mode = (landing_deck_usable_006bed60(deck_index) && landing_corridor_006c3b10(d, p)
                && rec.spacing_8 > 0.0f) ? 4 : 3;
    }
    if (leader_pass && p.land_task_installed &&
        p.land_state == GameUnitSlot::LandTaskState::kStandby) {
        landing_head_circle_diag(d, p, rec.mode_10, mode);   // DIAGNOSTIC
    }
    if (rec.mode_10 == 4 && mode != 4 && p.land_task_installed && lfm_trace < 80
        && p.land_state == GameUnitSlot::LandTaskState::kFinal) {
        // DIAGNOSTIC (cc9-physics28): a plane on land/final losing mode 4 (its
        // final then aborts at 009B3DB2): this pass's inputs to the mode.
        ++lfm_trace;
        const std::array<float, 3> l = landing_local_006bcc90(d, p.motion.position);
        log.notef("  land final mode loss %s t=%.2f mode 4->%d leader=%d head_mode=%d "
            "corridor=%d usable=%d spacing=%.3f m900=%d local=(%.1f %.1f %.1f) hdg=%.4f "
            "rwy=%.4f (006C7960, cc9-physics28 diagnostic)", p.row.name.c_str(),
            static_cast<double>(summary.simulated_seconds), mode, leader_pass ? 1 : 0,
            head_mode, landing_corridor_006c3b10(d, p) ? 1 : 0,
            landing_deck_usable_006bed60(deck_index) ? 1 : 0,
            static_cast<double>(rec.spacing_8), p.plane_control_mode_900,
            static_cast<double>(l[0]), static_cast<double>(l[1]), static_cast<double>(l[2]),
            static_cast<double>(p.plane_heading_c6c), static_cast<double>(d.runway_heading_88));
    }
    rec.mode_10 = mode;
    landing_path_006c6020(d, rec);
    if (!leader_pass) {
        const float floor = static_cast<float>(static_cast<double>(head_dist) + 1.0);  // 00D7A210
        rec.path_4 = floor > rec.path_4 ? floor : rec.path_4;     // 00415550
    }
}

// 006C6020 (006C6020-006C64A4, RET 4): the path still to fly, +4h, and the side.
void GameUnitsHost::Impl::landing_path_006c6020(LandingDeck& d, LandingAssignment& rec) {
    const GameUnitSlot& p = *slots[rec.plane];
    const bsp::GameTuningBlock& g = lua.plane_globals();
    if (!landing_airborne(p) || rec.held_0c) {
        rec.path_4 = 999999.0f;
        return;
    }
    if (rec.mode_10 == 4 || rec.mode_10 == 3) {
        rec.side_0d = 0.0f <= landing_local_006bcc90(d, p.motion.position)[0];
    } else {
        float sum = 0.0f;
        for (const std::size_t m : landing_members(landing_squadron_of(rec.plane))) {
            if (!landing_airborne(*slots[m])) continue;
            sum = landing_local_006bcc90(d, slots[m]->motion.position)[0] + sum;
        }
        rec.side_0d = !(sum <= 0.0f);
    }
    const bool side = rec.side_0d;
    const float D = landing_t_distance(d, p);
    if (rec.mode_10 == 4) {
        rec.path_4 = D;
        return;
    }
    const float r = landing_radius_006c3e50(d, p);
    const std::array<float, 3> c = landing_circle_point_006c5380(d, p, side);
    const float px = p.motion.position[0] - c[0];
    const float pz = p.motion.position[2] - c[2];
    const float dp = landing_len2_00414c60(px, pz);
    const float m1 = dp > 1.0f ? dp : 1.0f;                        // 00415550(1.0, dP)
    const float off = side ? -1.5707963705062866f : 1.5707963705062866f;   // 00CE3CCC / 00CE3C64
    const float h1 = bsp::wrapped_angle_add_00438aa0(d.runway_heading_88, off);
    const float b = bsp::heading_angle_00414eb0(std::array<float, 2>{px, pz});
    const float ratio = static_cast<float>(static_cast<double>(r) / static_cast<double>(m1));
    const float clamped = ratio < 1.0f ? ratio : 1.0f;             // 00415510(1.0, ratio)
    const float a = static_cast<float>(std::acos(static_cast<double>(clamped)));  // 00BF9940
    float bp;
    float theta;
    if (side) {
        bp = bsp::wrapped_angle_add_00438aa0(b, a);
        theta = bsp::wrapped_angle_subtract_00438b10(h1, bp);
    } else {
        bp = bsp::wrapped_angle_subtract_00438b10(b, a);
        theta = bsp::wrapped_angle_subtract_00438b10(bp, h1);
    }
    const std::array<float, 2> dir = bsp::heading_to_direction_006bc0c0(bp);
    if (theta < -0.0872664675116539f && !landing_corridor_006c3b10(d, p)) {   // 00CF885C
        theta = static_cast<float>(static_cast<double>(theta) + 6.2831854820251465);
    }
    const float rm = r < m1 ? r : m1;                               // 00415510(r, m1)
    const double arc = static_cast<double>(rm) * static_cast<double>(theta);
    float path;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // pc24w1-sweep3-6: 006C63EB FMUL min(r, m1) * theta and 006C63FE FADD PosBehind,
        // each rounded at PC24.
        path = static_cast<float>(bsp::cc13x87::x87_add24(g.pilot_landing_pos_behind, bsp::cc13x87::x87_mul24(rm, theta)));
    } else {
        path = static_cast<float>(static_cast<double>(g.pilot_landing_pos_behind) + arc);
    }
    if (m1 > r) {
        const float qx = dir[0] * r + c[0];
        const float qz = r * dir[1] + c[2];
        const float leg = landing_len2_00414c60(p.motion.position[0] - qx,
                                                p.motion.position[2] - qz);
        path = static_cast<float>(static_cast<double>(leg) + static_cast<double>(path));
    }
    rec.path_4 = D > path ? D : path;
}

// 006C3F80 (006C3F80-006C45B7, RET 4): the spacing +8h, which gates mode 4.
void GameUnitsHost::Impl::landing_spacing_006c3f80(LandingDeck& d, LandingAssignment& rec) {
    const GameUnitSlot& p = *slots[rec.plane];
    if (!landing_airborne(p) || rec.held_0c) {
        rec.spacing_8 = 1.0f;
        return;
    }
    const bsp::GameTuningBlock& g = lua.plane_globals();
    const bool own4 = rec.mode_10 == 4;
    const std::size_t sq = landing_squadron_of(rec.plane);
    const std::vector<std::size_t> members = landing_members(sq);
    const std::size_t head = members.empty() ? static_cast<std::size_t>(-1) : members[0];
    const bool is_head = rec.plane == head;
    const bool own_only = !(own4 || rec.mode_10 == 3 || (is_head && rec.mode_10 == 2));
    const float own_path = rec.path_4;
    const int own_spawn = landing_spawn_index(rec.plane);
    float least = 999999.0f;
    float gap = 999999.0f;
    int k = 0;
    std::size_t gap_plane = static_cast<std::size_t>(-1);          // DIAGNOSTIC
    for (const LandingAssignment& o : d.assignments) {
        if (o.plane == rec.plane) continue;
        const GameUnitSlot& op = *slots[o.plane];
        if (!landing_airborne(op)) continue;
        const std::size_t osq = landing_squadron_of(o.plane);
        if (is_head && osq == sq) continue;
        if (o.held_0c || !(o.path_4 > 0.0f)) continue;
        if (own4) {
            if (o.mode_10 != 4) continue;
        } else if (own_only) {
            if (osq != sq) continue;
        } else if (o.mode_10 != 3 && o.mode_10 != 4) {
            continue;
        }
        const float gdiff = o.path_4 - own_path;
        bool count;
        if (!is_head && o.plane == head) {
            count = true;                                           // 006C41AF
        } else if (-70.0f > gdiff) {                                // 00E08E50
            count = true;
        } else if (gdiff > 70.0f) {
            count = false;
        } else {
            const int other_spawn = landing_spawn_index(o.plane);
            if (own_spawn != other_spawn) {
                count = own_spawn > other_spawn;                    // SETG
            } else if (rec.side_0d != o.side_0d) {
                count = rec.side_0d;
            } else {
                // SETL on the two plane pointers: the heap's order. LABELLED:
                // this host orders by unit index.
                count = rec.plane < o.plane;
            }
        }
        if (!count) continue;
        least = o.path_4 > least ? least : o.path_4;
        const float ng = -gdiff;
        const float m = ng > gap ? gap : ng;
        if (!(ng > gap)) gap_plane = o.plane;                       // DIAGNOSTIC
        gap = 0.0f > m ? 0.0f : m;
        ++k;
    }
    const float fdt = g.pilot_landing_follow_dist_time;
    if (k == 0) {
        if (!own4) {
            rec.spacing_8 = 1.0f;
            return;
        }
        if constexpr (kLandingSiteSpacingBound) {
            // 006C42F4-006C4405. tp = own +4h / plane->vtable[38h]; site =
            // max(0, [00F876A4] - site+40h); +8h = 00419010(0.75 FDT (00CEC9D8),
            // 0.01, FDT, 1.0, site + tp), zeroed when tp < 0.25 FDT (00D7A348)
            // and (site < 0.4 FDT (00CE65D0) or site->vtable[30h] is false).
            // The airfield site's vtable[30h] is 006CF3F0, RET true.
            // site+40h: the site constructor 006CF100 stamps now - 99999.0
            // (00CF89D0); the one restamp, site->vtable[48h] = 006CE230 (now),
            // is called from 007CB5F0 at 007CB75D, the touchdown. Without
            // kPlaneTouchdownBound nothing restamps it and the stamp stays the
            // mission-start value -99999.0 (labelled: the site's construction
            // time is taken as 0), so site is 99999 s plus the mission clock.
            const float speed = avoid_len(p.plane_world_velocity);      // plane->vtable[38h]
            const float tp = static_cast<float>(static_cast<double>(rec.path_4) /
                static_cast<double>(speed));
            float site = static_cast<float>(static_cast<double>(
                static_cast<float>(summary.simulated_seconds)) - static_cast<double>(d.site_stamp_40));
            if (0.0f > site) site = 0.0f;
            rec.spacing_8 = bsp::clamped_interpolate_00419010(
                static_cast<float>(static_cast<double>(fdt) * 0.75), 0.009999999776482582f,
                fdt, 1.0f, static_cast<float>(static_cast<double>(site) + tp));
            if (!(static_cast<double>(fdt) * 0.25 <= static_cast<double>(tp))) {
                // Packet cc13-carrierdeck (G2): a carrier deck's site answers
                // 006D0530; the airfield's 006CF3F0 is RET true. The answer is
                // evaluated in both modes (const, log-only when OFF).
                const bool deck_free = !d.mother_ship || carrier_site_has_free_runway_006d0530(d);
                bool site_ready = true;                                  // 006CF3F0
                if constexpr (kCarrierSiteHasFreeRunwayCc13Bound) {
                    site_ready = deck_free;
                }
                const bool site_late = static_cast<double>(fdt) * 0.4000000059604645 >
                    static_cast<double>(site);
                if (!site_late && !deck_free) ++d.spacing_site_busy;
                if (site_late || !site_ready) {
                    rec.spacing_8 = 0.0f;
                }
            }
            ++d.spacing_mode4_site;
            return;
        }
        // The launch-site object at block+3Ch (its +40h time and vtable[30h]) is
        // not in this host: REFUSED, +8h keeps its value.
        ++d.spacing_mode4_refused;
        if (d.spacing_mode4_refused == 1) {
            log.notef("  landing sequencer: REFUSED 006C3F80's k=0 mode-4 arm for %s at %.2f s "
                "(the launch-site object at block+3Ch is unread)", p.row.name.c_str(),
                static_cast<double>(summary.simulated_seconds));
        }
        return;
    }
    const float speed = avoid_len(p.plane_world_velocity);          // plane->vtable[38h]
    const float F = static_cast<float>(static_cast<double>(fdt) * 1.2000000476837158);
    const float t = static_cast<float>(static_cast<double>(gap) / static_cast<double>(speed));
    float s;
    if (own4) {
        const double floor_t = kPc24PlaneTasksCc13Bound
            ? bsp::cc13x87::x87_mul24(0.4000000059604645, F)   // pc24w1-sweep3-6: 006C4465 FMUL, rounded
            : 0.4000000059604645 * F;
        s = floor_t > static_cast<double>(t) ? 0.0f
            : bsp::clamped_interpolate_00419010(static_cast<float>(F * 0.699999988079071),
                  0.009999999776482582f, static_cast<float>(1.100000023841858 * F),
                  1.100000023841858f, t);
    } else {
        s = 0.5 * F > static_cast<double>(t) ? 0.0f
            : bsp::clamped_interpolate_00419010(static_cast<float>(F * 0.75),
                  0.009999999776482582f, static_cast<float>(1.25 * F), 2.0f, t);
    }
    float sum;
    float x;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // pc24w1-sweep3-6: (F * k) * speed + least (006C451B..006C4527, FSTP float), then
        // (own - sum) / own (006C4533..006C4535), each op rounded at PC24.
        sum = static_cast<float>(bsp::cc13x87::x87_add24(
            bsp::cc13x87::x87_mul24(bsp::cc13x87::x87_mul24(F, static_cast<double>(k)), speed), least));
        x = static_cast<float>(bsp::cc13x87::x87_div24(bsp::cc13x87::x87_sub24(own_path, sum), own_path));
    } else {
        sum = static_cast<float>(static_cast<double>(least) +
            static_cast<double>(F) * k * static_cast<double>(speed));
        x = static_cast<float>((static_cast<double>(own_path) - sum) /
            static_cast<double>(own_path));
    }
    const float last = bsp::clamped_interpolate_00419010(-0.15000000596046448f,
        0.009999999776482582f, 0.15000000596046448f, 2.0f, x);
    rec.spacing_8 = last > s ? s : last;
    if (own4 && p.land_task_installed && lsp_trace < 80 && !(rec.spacing_8 > 0.0f)
        && (p.land_state == GameUnitSlot::LandTaskState::kBegin
            || p.land_state == GameUnitSlot::LandTaskState::kFinal)) {
        // DIAGNOSTIC (cc9-physics29): a mode-4 plane on land/begin or land/final whose
        // spacing is 0 (006C7960 then gives a wingman mode 3 and 009B3DB2 aborts).
        ++lsp_trace;
        log.notef("  land spacing zero %s t=%.2f k=%d own_path=%.1f gap=%.1f behind=%s "
            "behind_path=%.1f least=%.1f speed=%.2f gap_s=%.3f F=%.3f s=%.3f last=%.3f "
            "(006C3F80, cc9-physics29 diagnostic)", p.row.name.c_str(),
            static_cast<double>(summary.simulated_seconds), k, static_cast<double>(own_path),
            static_cast<double>(gap),
            gap_plane < slots.size() ? slots[gap_plane]->row.name.c_str() : "-",
            [&] {
                for (const LandingAssignment& o : d.assignments)
                    if (o.plane == gap_plane) return static_cast<double>(o.path_4);
                return -1.0;
            }(),
            static_cast<double>(least), static_cast<double>(speed),
            static_cast<double>(gap) / static_cast<double>(speed), static_cast<double>(F),
            static_cast<double>(s), static_cast<double>(last));
    }
}

// 006CC9F0 (006CC9F0-006CCD77, __thiscall(block)(squadron, float), RET 8,
// returns ST0). The float argument is not read.
float GameUnitsHost::Impl::landing_sequencer_006cc9f0(LandingDeck& d, std::size_t squadron) {
    const std::size_t deck_index = static_cast<std::size_t>(&d - landing_decks.data());
    const std::vector<std::size_t> members = landing_members(squadron);
    if (members.empty()) return 1.0f;   // squadron+3D0h null: the image would fault. LABELLED
    const std::size_t head = members[0];
    ++d.passes;
    std::size_t hi = d.assignments.size();
    for (std::size_t i = 0; i < d.assignments.size(); ++i) {
        if (d.assignments[i].plane == head) { hi = i; break; }
    }
    if (hi < d.assignments.size()) {
        ++d.hit_passes;
        const int before = d.assignments[hi].mode_10;
        landing_mode_006c7960(d, deck_index, d.assignments[hi], true, 4, -1.0f);
        landing_spacing_006c3f80(d, d.assignments[hi]);
        const int head_mode = d.assignments[hi].mode_10;
        const float head_dist = d.assignments[hi].path_4;
        if (landing_released_006c45c0(d, *slots[head])) {
            // 006C7540: erase every record of the squadron.
            std::size_t erased = 0;
            for (std::size_t i = d.assignments.size(); i-- > 0;) {
                if (landing_squadron_of(d.assignments[i].plane) == squadron) {
                    d.assignments.erase(d.assignments.begin() + static_cast<std::ptrdiff_t>(i));
                    ++erased;
                }
            }
            ++d.releases;
            log.notef("  landing sequencer: squadron head %s released at %.2f s, %zu record(s) "
                "erased (006C45C0, 006C7540)", slots[head]->row.name.c_str(),
                static_cast<double>(summary.simulated_seconds), erased);
            return 1.0f;                                                // 00D7A24C
        }
        if (head_mode != before) {
            log.notef("  landing sequencer: %s (head) mode %d -> %d path=%.1f spacing=%.3f "
                "side=%d at %.2f s (006C7960)", slots[head]->row.name.c_str(), before, head_mode,
                static_cast<double>(head_dist), static_cast<double>(d.assignments[hi].spacing_8),
                d.assignments[hi].side_0d ? 1 : 0, static_cast<double>(summary.simulated_seconds));
        }
        if (head_mode >= 0 && head_mode < 5) ++d.mode_counts[head_mode];
        const float result = (head_mode == 3 || head_mode == 4) ? 0.25f : 1.0f;  // 00CE3868
        for (LandingAssignment& rec : d.assignments) {
            if (rec.plane == head) continue;
            if (landing_squadron_of(rec.plane) != squadron) continue;
            const int was = rec.mode_10;
            landing_mode_006c7960(d, deck_index, rec, false, head_mode, head_dist);
            landing_spacing_006c3f80(d, rec);
            if (rec.mode_10 >= 0 && rec.mode_10 < 5) ++d.mode_counts[rec.mode_10];
            if (rec.mode_10 != was) {
                log.notef("  landing sequencer: %s mode %d -> %d path=%.1f spacing=%.3f "
                    "at %.2f s (006C7960)", slots[rec.plane]->row.name.c_str(), was, rec.mode_10,
                    static_cast<double>(rec.path_4), static_cast<double>(rec.spacing_8),
                    static_cast<double>(summary.simulated_seconds));
            }
        }
        return result;
    }
    if (landing_outside_standby_006c46b0(d, *slots[head])) {
        ++d.outside_passes;
        return 1.0f;
    }
    // 006CCC52-006CCC89: the head's x through the inverse, less T.x.
    const bool side = 0.0f <= landing_xform_004142e0(d.inverse_48,
        slots[head]->motion.position)[0] - d.t_a4[0];
    const float standby = lua.plane_globals().pilot_landing_standby_dist;
    for (const std::size_t m : members) {
        LandingAssignment rec;
        rec.plane = m;
        rec.path_4 = standby;
        rec.spacing_8 = 1.0f;
        rec.held_0c = false;
        rec.side_0d = side;
        rec.mode_10 = m == head ? 3 : 1;
        d.assignments.push_back(rec);                               // 006CAA10
    }
    ++d.inserts;
    log.notef("  landing sequencer: squadron head %s inserted %zu record(s) at %.2f s, "
        "head at %.1f m from T, side=%d (006CC9F0 miss arm, 006CAA10)",
        slots[head]->row.name.c_str(), members.size(),
        static_cast<double>(summary.simulated_seconds),
        static_cast<double>(landing_t_distance(d, *slots[head])), side ? 1 : 0);
    return 0.0f;
}

// 006C54C0's queue and lookup, bound: 006BF060/006C0B50, then 006BD080. Returns
// true when the plane's record is found (and its +174h word is non-zero, which
// this host takes as always true).
bool GameUnitsHost::Impl::landing_request_006c54c0(GameUnitSlot& plane) {
    plane.land_request_ran = false;
    if (!lua.plane_globals_loaded()) return false;
    if (plane.land_deck_plus_one == 0 || plane.land_site_plus_one == 0) return false;
    const std::size_t sq = landing_squadron_of(plane.process_index);
    if (sq == static_cast<std::size_t>(-1)) return false;           // +9D4h null
    LandingDeck* d = landing_deck_006c0750(plane.land_deck_plus_one - 1u,
                                           plane.land_site_plus_one - 1u);
    if (d == nullptr) return false;
    plane.land_request_ran = true;
    bool queued = false;
    int next_n = 0;
    for (const LandingQueueEntry& q : d->queue) {
        if (q.squadron == sq) queued = true;
        if (q.n + 1 > next_n) next_n = q.n + 1;
    }
    if (!queued) {
        LandingQueueEntry q;
        q.squadron = sq;
        q.n = next_n;
        d->queue.push_back(q);
        // The tail's slot move to state 4 is not applied: this host's slot
        // squadron ids are not the registry's. Counted.
        const bsp::AirOpsDeck* deck = bsp::air_ops_decks().mutable_at(plane.land_deck_plus_one - 1u);
        if (deck != nullptr) {
            for (const bsp::AirOpsSlot& s : deck->slots) {
                if (s.launched_squadron != 0u) ++d->slot_tail_unapplied;
            }
        }
        log.notef("  landing sequencer: squadron %s queued n=%d at deck %s at %.2f s (006C0B50)",
            bsp::plane_squadron_registry().records()[sq].name.c_str(), q.n,
            bsp::air_ops_decks().name_at(plane.land_deck_plus_one - 1u).c_str(),
            static_cast<double>(summary.simulated_seconds));
    }
    ++d->lookups;
    bool found = false;
    // 006BD080 calls air_ops_find_landing_assignment_006bd080; OFF keeps the host's assignment scan.
    {
        struct PlaneView {
            std::uint8_t unused_000[0x174]{};
            std::uint16_t id_174{};
        };
        struct AssignmentView {
            void* plane_00;
            float path_04;
            float spacing_08;
            std::uint8_t held_0c;
            std::uint8_t side_0d;
            std::uint16_t unused_0e;
            std::int32_t mode_10;
        };
        static_assert(sizeof(AssignmentView) == 0x14);
        static_assert(offsetof(PlaneView, id_174) == 0x174);
        // One identity per live host slot, without preselecting a matching
        // record. These query-local images stay fixed until the body returns.
        std::vector<PlaneView> planes(slots.size());
        planes.at(plane.process_index).id_174 =
            static_cast<std::uint16_t>(plane.process_index + 1u);
        std::vector<AssignmentView> assignments;
        assignments.reserve(d->assignments.size());
        for (const LandingAssignment& rec : d->assignments) {
            assignments.push_back({&planes.at(rec.plane),
                rec.path_4, rec.spacing_8, static_cast<std::uint8_t>(rec.held_0c),
                static_cast<std::uint8_t>(rec.side_0d), 0, rec.mode_10});
        }
        alignas(4) std::uint8_t block[0xb0]{};
        alignas(4) std::uint8_t answer[0x14]{};
        // The host owns a single-threaded vector with no tracked critical
        // section. Only its begin/end cells and the record payload are exposed.
        using bsp::cc13_seg29U::at;
        if (!assignments.empty()) {
            at<void*>(block, 0xa8) = assignments.data();
            at<void*>(block, 0xac) = assignments.data() + assignments.size();
        }
        LandingAssignmentLookupHost lookup;
        found = bsp::cc13_seg29U::air_ops_find_landing_assignment_006bd080(
            block, &planes.at(plane.process_index), answer, lookup);
        if (found) {
            ++d->found;
            plane.land_spacing_4c = at<float>(answer, 0x08);
            plane.land_mode_50 = at<std::int32_t>(answer, 0x10);
            plane.land_side_44 = at<std::uint8_t>(answer, 0x0c) != 0;
        }
    }
    if constexpr (kLandStandbyStateBound) {
        if (!found) {
            // 006C5563-006C55FC: out+0Ch = (sum of the airborne members' x
            // through holder+48h, less holder+A4h) >= 0.0 (006C55DC JC).
            float sum = 0.0f;
            for (const std::size_t m : landing_members(sq)) {
                if (!landing_airborne(*slots[m])) continue;
                const std::array<float, 3> l =
                    landing_xform_004142e0(d->inverse_48, slots[m]->motion.position);
                sum = static_cast<float>(static_cast<double>(l[0] - d->t_a4[0]) + sum);
            }
            plane.land_side_44 = !(0.0f > sum);
        }
        // 006C55FF-006C562A, both arms: +48h = 006C3E50(plane), +38h..+40h =
        // 006C5380(side = out+0Ch).
        plane.land_radius_48 = landing_radius_006c3e50(*d, plane);
        const std::array<float, 3> c =
            landing_circle_point_006c5380(*d, plane, plane.land_side_44);
        for (int i = 0; i < 3; ++i) plane.land_circle_38[i] = c[i];
    }
    return found;
}

// Packet p15_free_attack_tick (Impl::kSquadronFreeAttackTickBound,
// docs/OCEAN_WAVE_FIELD.md section 57). The squadron's +348h block is its flight
// leader's director here (the host fuses them, kSquadronSetCommandBound); an
// override goes to every live member's director, as the intake's fan-out does.
// LABELLED: 00927F10 for a slot other than 8 answers player-held (no participant
// pools reach here); leader+C24h (PilotFires) has no reader and is clear; the
// clear-then-moveto-self arm for a `stop` head (009F85EB..009F8615) is counted,
// not issued, unless kSquadronStopHeadMoveToSelfBound; 009F8160's leash runs
// here under kSquadronDogfightLeashBound; 0071F290 arm 5's begin (0084DDA0 -> 0071F600) is not run, the send
// having just passed 0071D6D0.
void GameUnitsHost::run_squadron_free_attack_009f8470(const bsp::PlaneSquadronHostRecord& sq,
    std::size_t leader, const std::vector<std::size_t>& alive) {
    Impl& h = *impl_;
    GameGunneryHost* const gun = h.gunnery.get();
    if (gun == nullptr || leader >= h.slots.size()) return;
    const bool apply = Impl::kSquadronFreeAttackTickBound;
    GameCommandsHost& commands = h.commands;
    Impl::FreeAttack& fa = h.free_attack[sq.name];
    const float now = h.summary.simulated_seconds;
    // 0071E4C0 on the leader's director: a moveonpath head's final point (007B1350),
    // else the head descriptor's point (006F7DD0: its entity's, else its own).
    const auto path_point_0071e4c0 = [&](std::uint32_t head_command,
                                         const bsp::SceneCommandTarget& head, std::uint32_t r,
                                         bool& valid, std::array<float, 3>& point) {
        if (head_command == bsp::kCommandMoveOnPath) {
            float last[3]{};
            if (commands.path_final_point_007b1350(leader, last)) {
                valid = true;
                point = {last[0], last[1], last[2]};
            }
        } else if (r != 0u) {
            valid = true;
            unit_position_00fc(r - 1u, point[0], point[1], point[2]);
        } else if (head.position_valid != 0 || head.kind != 0) {
            valid = true;
            point = {head.position[0], head.position[1], head.position[2]};
        }
    };

    // The override this host sent: its target's removal (0071DDB0's override arm,
    // 0071EDD0 then 0071D9E0(2)). LABELLED: tested here per step on the gunnery
    // death rather than at the destroy flush; a squadron target whose leader died
    // moves to the squadron's next flight leader (0071EDD0 keeps the command on a
    // live member), anything else ends the override.
    if (apply && fa.override_command != 0 && fa.override_target != 0
            && gun->unit_dead(fa.override_target - 1u)) {
        std::size_t next = kInvalidFreeAttackUnit;
        if (fa.override_squadron) {
            const bsp::PlaneSquadronHostRecord* tsq =
                bsp::plane_squadron_registry().find_by_member_or_departed_unit(fa.override_target - 1u);
            const std::size_t lead2 = tsq != nullptr ? tsq->flight_leader() : bsp::kPlaneSquadronNoUnit;
            if (lead2 != bsp::kPlaneSquadronNoUnit && lead2 < h.slots.size() && !gun->unit_dead(lead2))
                next = lead2;
        }
        for (const std::size_t m : alive) {
            if (next != kInvalidFreeAttackUnit) {
                commands.send_override_command_0071d980(m, fa.override_command,
                    static_cast<std::uint32_t>(next + 1u));
                h.slots[m]->command_target_plus_one = next + 1u;
            } else {
                commands.raise_override_stage_0071d9e0(m, 2);
            }
        }
        h.log.notef("free attack: squadron \"%s\" override target %s gone at %.2f s: %s "
            "(0071DDB0 -> 0071EDD0, packet p15_free_attack_tick)", sq.name.c_str(),
            h.slots[fa.override_target - 1u]->row.name.c_str(), static_cast<double>(now),
            next != kInvalidFreeAttackUnit ? h.slots[next]->row.name.c_str() : "ended (0071D9E0(2))");
        if (next != kInvalidFreeAttackUnit) fa.override_target = next + 1u;
    }
    if constexpr (kSquadronDogfightLeashBound) {
        // Packet cc13-leash (include/bsp/squadron_leash_cc13.hpp): 009F8160 from
        // 009F82DA to 009F8407, before the tick (009F848A): the dogfight
        // out-of-map arm, then the leash. LABELLED: the leader's director stands for
        // the squadron's block. 009F8160's earlier arms are the host's retest, so the
        // seven ordnance classes and 00E08FA0, which return or leave for 009F8412
        // before 009F82DA, are excluded here.
        // 0071BE40 answers the override only in mode 2, and 0071EB60 then hands back
        // its +18Ch descriptor, so the slot-0 test at 009F83C4 can pass only in mode 2.
        const std::uint32_t current = commands.current_command_0071be40(leader);
        const bsp::EntityOrderCommandClass* klass =
            bsp::entity_order_command_class_by_address(current);
        bool early_arm = false;
        for (const std::uint32_t c : {0x00E08F28u, 0x00E08F30u, 0x00E08F20u, 0x00E08F18u,
                                      0x00E08F50u, 0x00E08F48u, 0x00E08F38u, 0x00E08FA0u})
            if (current == c) early_arm = true;
        bsp::SceneCommandTarget over{};
        int over_mode = 0;
        commands.active_command_descriptor_0071eb60(leader, over, over_mode);
        const std::uint32_t over_target = commands.resolve_command_target_00521ea0(over);
        const bool category_12 = klass != nullptr && (klass->category == 1 || klass->category == 2);
        // 009F82EE: a category 1/2 command with no target leaves for 009F8412;
        // 009F830C / 009F8313: so does one 007EE8F0 refuses. SUBSTITUTION (labelled):
        // "the target is not dead" stands for 007EE8F0.
        // CORRECTION (cc13-tranche2 S18): under bsp::kFreeAttackAcceptTestCc13Bound the
        // gate also asks 007EE8F0 itself (include/bsp/free_attack_accept_test_cc13.hpp),
        // for every category 1/2 class: a refusal skips the leash arm and the whole
        // 009F83AD..009F8407 tail below.
        const bool target_resolved = over_target != 0u && over_target - 1u < h.slots.size()
            && !gun->unit_dead(over_target - 1u);
        // Both arms (sw3b reach count); the ON arm only reads unit state.
        const auto target_live_arm = [&](auto accept_test) -> bool {
            if constexpr (decltype(accept_test)::value) {
                if (!(target_resolved && category_12)) return target_resolved;
                // 009F830C 007EE8F0(ECX = squadron; current, target, 00922DC0(target),
                // 007B58D0(squadron)). LABELLED: the flight leader stands for the
                // squadron's +54h and its slot-0 plane.
                const std::size_t t = over_target - 1u;
                bsp::EntityTargetFacts tf;
                tf.present = true;
                tf.not_engageable = unit_flag_005d(t);
                tf.is_plane = unit_is_kind_of(t, 0x0f);
                tf.is_plane_squadron = unit_is_kind_of(t, 0x18);
                tf.is_ship_family = unit_is_kind_of(t, 0x06);
                tf.is_submarine = unit_is_kind_of(t, 0x08);
                tf.is_airfield = unit_is_kind_of(t, 0x45);
                tf.is_shipyard = unit_is_kind_of(t, 0x46);
                tf.is_command_building = unit_is_kind_of(t, 0x1c);
                tf.is_dummy_target = unit_is_kind_of(t, 0x35);
                tf.is_land_fort = unit_is_kind_of(t, 0x1b);
                float pr[3], pu[3], pf[3], pt[3];
                if (unit_pose(t, pr, pu, pf, pt)) tf.world_y = pt[1];
                // 00922DC0: MOV DL,1 / JMP 00922C80. The 008DDF90 set arm as the
                // script-order chooser takes it: slot 0's set (LABELLED), then the
                // tail with DL = 1. Periscope level and FakedType keep their defaults.
                bool surface = false;
                switch (bsp::entity_is_surface_target_00922c80(tf)) {
                    case bsp::SurfaceTargetAnswer::kYes: surface = true; break;
                    case bsp::SurfaceTargetAnswer::kNo: surface = false; break;
                    case bsp::SurfaceTargetAnswer::kUnreadSetBranch: {
                        bool in_set = false;
                        if (unit_is_kind_of(t, 0x05)) {
                            const std::vector<std::size_t> set =
                                bsp::game::game_objective_sets().units_in_slot(0);
                            in_set = std::find(set.begin(), set.end(), t) != set.end();
                        }
                        surface = in_set || bsp::entity_surface_target_tail_00922c80(tf, true);
                        break;
                    }
                }
                bsp::AttackFeasibilityInputs in;
                in.unit_has_weapon_controller = true;               // 007EE8FC, a leader
                in.target_present = true;                           // 007EE8F6
                in.target_is_structure = tf.is_command_building;    // 007EE90A
                in.unit_side = unit_side_0054(leader);              // 007EE917
                in.target_side = unit_side_0054(t);                 // 007EE912
                in.target_is_surface = surface;                     // the byte argument
                in.target_is_air = bsp::entity_mission_seg60_argon::
                    entity_is_airborne_target_00922b10(tf) != 0;          // 007EEAF5
                in.target_is_strafe_fallback = unit_is_kind_of(t, 0x41);    // 007EEBA7
                in.guns_available = plane_pilot_fires_0c24(leader);         // 007EEB08/007EEBB7
                in.self_is_dogfight_excluded = unit_is_kind_of(leader, 0x16); // 007EEB1A
                in.guns_suppressed = unit_is_kind_of(leader, 0x10)          // 007EEB2C/007EEBC2,
                    || in.self_is_dogfight_excluded;                        // 0047B850
                // The category 1/2 classes reaching here are artillery 00E08F10, strafe
                // 00E08F40, attackmove 00E08F78 and dogfight 00E08F58. Attackmove is the
                // side prologue alone (007EE938); artillery has no arm and falls to
                // 007EEBD1 -> 007EEBF6 XOR AL,AL, so it is always refused. The ordnance
                // inputs and the loadout feed none of these four arms.
                return bsp::attack_command_applies(current, in);
            } else {
                return target_resolved;
            }
        };
        const bool live_on = target_live_arm(std::true_type{});
        const bool live_off = target_live_arm(std::false_type{});
        bsp::plane_task_reach_count(bsp::g_plane_task_reach_sw3.free_attack,
            live_on != live_off, target_resolved && category_12);
        const bool target_live = bsp::kFreeAttackAcceptTestCc13Bound ? live_on : live_off;
        FreeAttackWorld leash_world(*this, h, *gun);
        if (current == 0x00E08F58u && !early_arm && (!category_12 || target_live)
                && over_target != 0u && over_target - 1u < h.slots.size()) {
            // 009F8319..009F83A8: a dogfight whose target's squadron is retreating
            // and has left the map ends (0071E430(current, 1) at 009F83A8).
            const std::size_t t = over_target - 1u;
            bool end = false;
            const auto target_as_plane = [&]() {
                // 009F8420 calls 0071EA00 on the resolved target; the
                // helper's null test and vtable[5Ch](0Fh) decide whether
                // 009F842B proceeds to the out-of-map query (bsp.py show
                // 0071ea00 --asm / 009f8160 --asm). Its existing typed
                // body consumes the actual slot presence and the same
                // live class query that this host used here before.
                const bool present = h.slots[t] != nullptr;
                return bsp::gameplay_seg38_cc13::entity_as_kind_0f_0071ea00(
                    present, present && unit_is_kind_of(t, 0x0F));
            };
            // SUBSTITUTION, labelled: "the target is the squadron this host's own
            // override named" (fa.override_squadron with the same target) stands for
            // 00523560's squadron test on the resolved object. A squadron target the
            // host did not send itself takes the plane arm below.
            if (fa.override_squadron && over_target == fa.override_target) {
                // 00523560: the target is a squadron. vt+174h, its command, must be
                // retreat (009F8347); then every member at +3D0h (count +3CCh) is
                // tested with 00427EB0 -> 0071C4F0, and the dogfight ends unless one
                // is inside the map (009F8386 / 009F83A0). No members also ends it.
                // LABELLED: the target squadron's command is its flight leader's;
                // its members are the registry's live member_units; the image's
                // index > 4 reads a null member (009F836F), which is not reproduced.
                const bsp::PlaneSquadronHostRecord* tsq = [&]() {
                    // 009F8319 calls 00523560 on the resolved target before
                    // reading its command/members. This override retains the
                    // target's squadron identity; use its live registry object,
                    // never the member plane as the cast receiver. The existing
                    // leader-command projection below is still a substitution.
                    const auto* squadron = bsp::plane_squadron_registry()
                        .find_by_member_or_departed_unit(t);
                    struct SquadronKind final : bsp::cc13_seg14::EntityKindHooks {
                        explicit SquadronKind(const bsp::PlaneSquadronHostRecord* value)
                            : record(value) {}
                        bool object_is_kind_virtual_5c(void* object, std::uint32_t kind) override {
                            // 007EFB00..007EFB24 accepts 18h before any
                            // receiver-field read. The receiver here is a real
                            // PlaneSquadron registry object, not raw image storage.
                            return object == record && record != nullptr && kind == 0x18u;
                        }
                        const bsp::PlaneSquadronHostRecord* record;
                    } kinds(squadron);
                    return static_cast<const bsp::PlaneSquadronHostRecord*>(
                        bsp::cc13_seg14::entity_cast_if_kind_00523560(
                            const_cast<bsp::PlaneSquadronHostRecord*>(squadron), kinds));
                }();
                if (tsq != nullptr && commands.current_command_0071be40(t) == 0x00E08F90u) {
                    bool inside = false;
                    for (const std::size_t mm : tsq->member_units) {
                        if (mm == bsp::kPlaneSquadronNoUnit || mm >= h.slots.size()
                            || gun->unit_dead(mm)) continue;
                        if (!leash_world.outside_map_0071c4f0(mm)) inside = true;
                    }
                    end = !inside;
                }
            } else if (target_as_plane() && leash_world.outside_map_0071c4f0(t)) {
                // 009F841E..009F8463: 0071EA00, a plane outside the map whose +9D4h
                // squadron's command (vt+174h) is retreat. LABELLED: the squadron's
                // command is its flight leader's.
                const bsp::PlaneSquadronHostRecord* tsq =
                    bsp::plane_squadron_registry().find_by_member_unit(t);
                const std::size_t tl = tsq != nullptr ? tsq->flight_leader() : bsp::kPlaneSquadronNoUnit;
                end = tl != bsp::kPlaneSquadronNoUnit && tl < h.slots.size()
                    && commands.current_command_0071be40(tl) == 0x00E08F90u;
            }
            if (end) {
                // 009F83A8: 0071E430(current, 1) on the shared block; here on each
                // live member's director, which each hold the override.
                for (const std::size_t m : alive) end_command_0071e430(m, current, true);
                h.log.notef("free attack: squadron \"%s\" dogfight on %s ended at %.2f s: the "
                    "target is retreating outside the map (009F8319..009F83A8 -> 0071E430, "
                    "packet cc13-leash)", sq.name.c_str(), h.slots[t]->row.name.c_str(),
                    static_cast<double>(now));
            }
        }
        // After an end above, the image falls through (009F83AD) with EBP still the
        // old current, and re-reads slot 0 (009F83C4) and the +18Ch descriptor
        // (009F83DE) from the block. LABELLED: the host keeps current, its class and
        // target_live as the image does, re-reads slot 0, but keeps over / over_target
        // from before the end (stale for the rest of this tick when an end ran).
        bool fire = false, move = false;
        script_orders_squadron_permissions_3c3d(sq, fire, move);
        if (current != 0u && !early_arm && category_12                         // 009F83B7..009F83BF
                && target_live
                && commands.director_slot_command(leader, 0) != current        // 009F83C4
                && !bsp::director_allows_fire_and_move(fire, move)) {          // 009F83CB
            bsp::SquadronFreeAttackInput gin;
            bsp::SceneCommandTarget head{};
            commands.director_slot_target(leader, 0, head);                   // director+58h
            const std::uint32_t slot0 = commands.director_slot_command(leader, 0);
            path_point_0071e4c0(slot0, head, commands.resolve_command_target_00521ea0(head),
                                gin.path_point_valid, gin.path_point);
            gin.guard_dist_594 = h.lua.plane_globals_loaded()
                ? h.lua.plane_globals().pilot_general_guard_dist : 0.0f;
            bsp::SquadronGuardDescriptor g;
            g.entity = over.kind != 0;
            g.unit = over_target - 1u;
            // As the tick's slot0_target_squadron: a squadron target is its leader
            // here, with the arena's squadron object standing for the entity.
            g.squadron = fa.override_squadron && over_target == fa.override_target;
            g.position_valid = over.position_valid != 0;
            g.position = {over.position[0], over.position[1], over.position[2]};
            if (!bsp::squadron_within_guard_distance_009f7fe0(gin, g, leash_world,
                    kSquadronLeashGuardScale)) {                               // 009F83E7
                for (const std::size_t m : alive)
                    commands.raise_override_stage_0071d9e0(m, 2);              // 009F83F5
                // 009F8407's 009F7810 is not run: the leash fires only in mode 2,
                // so the same tick leaves at 009F849F for 009F8640, and the next
                // tick clears the pick at 009F8487 before any read
                // (squadron_leash_cc13.hpp).
                // Packet cc13-fapick3, log-only: the distance 009F7FE0 measured, from
                // the same point it read (the entity's, else descriptor+8h, else 0).
                float lx = 0.0f, ly = 0.0f, lz = 0.0f;
                if (g.entity) unit_position_00fc(g.unit, lx, ly, lz);
                else if (g.position_valid) { lx = g.position[0]; lz = g.position[2]; }
                const float ldx = lx - gin.path_point[0], ldz = lz - gin.path_point[2];
                h.log.notef("free attack: squadron \"%s\" leash at %.2f s: %08X on %s outside "
                    "%.0f m of the head point, override ended (009F83AD..009F83F5, packet "
                    "cc13-leash) dist=%.0f", sq.name.c_str(), static_cast<double>(now), current,
                    h.slots[over_target - 1u]->row.name.c_str(),
                    static_cast<double>(gin.guard_dist_594 * kSquadronLeashGuardScale),
                    std::sqrt(static_cast<double>(ldx) * ldx + static_cast<double>(ldz) * ldz));
            }
        }
    }
    // The override this host sent has been cleared (0071E610 left mode 1): 0099A4C0
    // retires its task and 0099A170 builds the head command's again. LABELLED: the
    // host's members each hold the override; when the leader's clears, the others'
    // are cleared too (the image has one shared block).
    if (apply && fa.override_command != 0 && commands.director_mode_0030(leader) != 2) {
        for (const std::size_t m : alive)
            if (commands.director_mode_0030(m) == 2) commands.raise_override_stage_0071d9e0(m, 2);
        const std::uint32_t head = commands.current_command_0071be40(leader);
        bsp::SceneCommandTarget head_target;
        int head_mode = 0;
        commands.active_command_descriptor_0071eb60(leader, head_target, head_mode);
        const std::uint32_t resolved = commands.resolve_command_target_00521ea0(head_target);
        const std::size_t head_unit = resolved != 0u ? resolved - 1u : kInvalidFreeAttackUnit;
        for (const std::size_t m : alive) {
            commands.retire_ended_command_rows(m, fa.override_command);
            GameUnitSlot& u = *h.slots[m];
            if (u.command_target_plus_one == fa.override_target) u.command_target_plus_one = 0;
            u.moveto_task_installed = false;
            u.moveto_state = GameUnitSlot::MoveToTaskState::kNone;
            u.moveto_arrived_454 = false;
            if (head != 0u && script_orders_install_command_task_0099a170(m, head, head_unit) != 0u)
                ++h.free_attack_restored;
            else
                u.attack_command_class = head;
        }
        ++h.free_attack_cleared;
        h.log.notef("free attack: squadron \"%s\" override %08X cleared at %.2f s, head %08X "
            "rebuilt (0071E610 -> 0099A170, packet p15_free_attack_tick)", sq.name.c_str(),
            fa.override_command, static_cast<double>(now), head);
        fa.override_command = 0;
        fa.override_target = 0;
        fa.override_squadron = false;
    }

    bsp::SquadronFreeAttackInput in;
    in.mode_30 = static_cast<std::uint32_t>(commands.director_mode_0030(leader));
    bool allow_fire = false, allow_move = false;
    script_orders_squadron_permissions_3c3d(sq, allow_fire, allow_move);
    in.allow_fire_3c = allow_fire;
    in.allow_move_3d = allow_move;
    in.slot0_command_54 = commands.director_slot_command(leader, 0);
    {
        bsp::SceneCommandTarget head;
        int mode = 0;
        if (commands.active_command_descriptor_0071eb60(leader, head, mode) && mode == 1) {
            const std::uint32_t r = commands.resolve_command_target_00521ea0(head);
            if (r != 0u) in.slot0_target = r - 1u;
            path_point_0071e4c0(in.slot0_command_54, head, r, in.path_point_valid, in.path_point);
        }
    }
    {
        int categories[bsp::kDirectorCommandSlotCount]{};
        const int n = commands.director_leading_slot_categories_0071df83(leader, categories,
            bsp::kDirectorCommandSlotCount);
        bool accepts = !bsp::auto_target_hold_active(commands.director_target_hold_0040(leader));
        for (int i = 0; i < n; ++i)
            if (bsp::command_category_owns_fire_target(categories[i])) accepts = false;
        in.accepts_new_target_0071df70 = accepts;
    }
    const GameUnitSlot& lead = *h.slots[leader];
    in.leader = leader;
    in.party_54 = unit_side_0054(leader);
    in.attack_mode_370 = static_cast<int>(lead.torpedo_attack_mode_370);
    in.leader_slot_1b0 = lead.current_roles_01ac[1];
    in.leader_slot_ai_held_00927f10 = false;
    in.leader_player_184 = lead.role_player_0184;
    // 009F6CB1 calls 007EDAC0 before its ordnance filters. The same
    // leader+C24h is read by 009F8470's hosted command checks. This field
    // is retained on the real leader slot, populated from the authored
    // platform PilotFires entries by the 007CD930 producer at scene load.
    // `leader` was selected from this squadron's live member list above.
    // A failed/missing class projection keeps the old false input; its
    // pre-existing true fallback is not evidence of a native +C24h value.
    if (lead.plane_pilot_fires_c24_authored) {
        in.leader_pilot_fires_c24 =
            bsp::cc13_seg48_plane_squadron_leader_flag_c24_007edac0(
                bsp::PlaneSquadronPilotFireView{lead.plane_pilot_fires_c24}) != 0;
    }
    unit_position_00fc(leader, in.squadron_position[0], in.squadron_position[1],
                       in.squadron_position[2]);
    {
        bsp::PlaneSquadronLeadPlaneFacts facts;
        facts.has_lead_plane = true;
        facts.lead_is_kamikaze_17 = unit_is_kind_of(leader, 0x17);
        // 007EDAAA tests this same byte after the leader's kind-17h test.
        facts.lead_pilot_fires_0c24 = in.leader_pilot_fires_c24;
        bsp::band009f::SquadronFilterInputs& f = in.filter_inputs;
        f.leader_uncommitted_kamikaze = bsp::plane_squadron_excluded_007eda90(facts);
        f.bomber_command_class = in.slot0_command_54 == bsp::kCommandMoveTo
            || in.slot0_command_54 == bsp::kCommandStop
            || in.slot0_command_54 == bsp::kCommandMoveOnPath;
        f.has_007edac0 = in.leader_pilot_fires_c24;
        {
            struct OrdnanceMembers final : bsp::Cc13SquadronOrdnanceHost {
                GameUnitsHost& units;
                Impl& owner;
                const std::vector<std::size_t>& members;
                OrdnanceMembers(GameUnitsHost& u, Impl& o, const std::vector<std::size_t>& m)
                    : units(u), owner(o), members(m) {}
                std::int32_t squadron_member_count(void*) override {
                    return static_cast<std::int32_t>(members.size());
                }
                void* squadron_member(void*, std::int32_t i) override {
                    return owner.slots[members[static_cast<std::size_t>(i)]].get();
                }
                bsp::OrdnanceKindSet kinds(void* member, std::uint32_t loadout) {
                    // 009F6C80 supplies literal 0 (current loadout) to all four
                    // predicates. This is the same live rack/round mask the
                    // former loop consumed, including its reload policy;
                    // it is not an independently seeded native object.
                    if (loadout != 0) throw std::logic_error("unbound ordnance loadout query");
                    return {units.unit_ordnance(static_cast<GameUnitSlot*>(member)->process_index)};
                }
                bool weapon_controller_has_general_bomb_ordnance_007b9320(
                    void* member, std::uint32_t loadout) override {
                    return bsp::ordnance_has_general_bomb_2ah(kinds(member, loadout));
                }
                bool weapon_controller_has_drop_kamikaze_ordnance_007b93e0(
                    void* member, std::uint32_t loadout) override {
                    return kinds(member, loadout).contains(0x2f);
                }
                bool weapon_controller_has_torpedo_ordnance_007b93f0(
                    void* member, std::uint32_t loadout) override {
                    return kinds(member, loadout).contains(0x2b);
                }
                bool weapon_controller_has_depth_charge_ordnance_007b94f0(
                    void* member, std::uint32_t loadout) override {
                    return kinds(member, loadout).contains(0x2c);
                }
            } ordnance(*this, h, alive);
            // 009F6C80 -> these four existing member-loop bodies. The exact
            // live membership snapshot and lower rack-kind queries are shared
            // with the OFF arm; their results drive the target filter below.
            f.has_bomb_ordnance = bsp::cc13_seg48_unit_has_general_bomb_ordnance_007ed7e0(&ordnance, 0, ordnance);
            f.has_torpedo_ordnance = bsp::cc13_seg48_unit_has_torpedo_ordnance_007ed8d0(&ordnance, 0, ordnance);
            f.has_depth_charge_ordnance = bsp::cc13_seg48_unit_has_depth_charge_ordnance_007eda10(&ordnance, 0, ordnance);
            f.has_drop_kamikaze_ordnance = bsp::cc13_seg48_unit_has_drop_kamikaze_ordnance_007ed880(&ordnance, 0, ordnance);
        }
        f.latch_3c = allow_fire;
        f.latch_3d = allow_move;
    }
    {
        // [0042E740()]+594h, Pilot/General/GuardDist.
        in.guard_dist_594 = h.lua.plane_globals_loaded()
            ? h.lua.plane_globals().pilot_general_guard_dist : 0.0f;
    }
    {
        std::vector<std::size_t> units;
        std::vector<bool> squadron_record;
        gun->recon_enemy_contacts(in.party_54, units, squadron_record);
        in.contacts.reserve(units.size());
        for (std::size_t i = 0; i < units.size(); ++i)
            in.contacts.push_back({units[i], squadron_record[i]});
    }
    constexpr int kPickLineCap = 8;   // packet cc13-fapick3
    in.trace_candidates = fa.pick_lines < kPickLineCap;   // packet cc13-fapick, log-only

    FreeAttackWorld world(*this, h, *gun);
    const bsp::SquadronFreeAttackDecision d = bsp::squadron_free_attack_tick_009f8470(in, world);
    ++h.free_attack_ticks;
    ++h.free_attack_stops[static_cast<int>(d.stop) & 7];
    const int stop = static_cast<int>(d.stop);
    if (stop != fa.last_stop || d.target + 1u != fa.last_target) {
        fa.last_stop = stop;
        fa.last_target = d.target + 1u;
        h.log.notef("free attack: squadron \"%s\" at %.2f s: %s filter=%d mode=%u head=%08X "
            "fire=%d move=%d slot1b0=%d contacts=%zu pick=%s%s command=%08X (009F8470, %s)",
            sq.name.c_str(), static_cast<double>(now), free_attack_stop_name(d.stop),
            static_cast<int>(d.filter), in.mode_30, in.slot0_command_54, allow_fire ? 1 : 0,
            allow_move ? 1 : 0, in.leader_slot_1b0, in.contacts.size(),
            d.target < h.slots.size() ? h.slots[d.target]->row.name.c_str() : "-",
            d.target_squadron ? " (squadron)" : "", d.command,
            apply ? "applied" : "census only");
    }
    const std::size_t pick_key = d.target < h.slots.size()
        ? (d.target + 1u) * 2u + (d.target_squadron ? 1u : 0u) : 0u;
    if (pick_key != 0u && fa.pick_lines < kPickLineCap
            && (fa.pick_lines == 0 || pick_key != fa.last_printed_pick)) {
        // Packets cc13-fapick / cc13-fapick3, log-only and always on: per squadron
        // name, at the first tick that leaves a target (the 0071D980 override
        // send's, else the selector's +08h pick when 0071DF70, the same-target or
        // the air-target test stopped the send), reason=first, then again whenever
        // the target differs from the last PRINTED one, reason=change, up to 8
        // lines. Until the cap every tick traces (one vector of contacts per tick,
        // printed or not). The line lists every recon contact 009F7810 visited and
        // the return it left by, then the enemy squadrons the recon list (+DE8h)
        // did not offer. Reads only: the traces are copies of what the loop already
        // computed (no predicate is run again); the rest are position reads, the
        // registry, unit_dead, the recon pass's published level and its observer
        // admission (recon_nearest_observer_distance).
        const char* const reason = fa.pick_lines == 0 ? "first" : "change";
        ++fa.pick_lines;
        fa.last_printed_pick = pick_key;
        float own_x = 0.0f, own_y = 0.0f, own_z = 0.0f;
        unit_position_00fc(leader, own_x, own_y, own_z);
        const auto distances = [&](std::size_t unit, double& d_own, double& d_head) {
            float ux = 0.0f, uy = 0.0f, uz = 0.0f;
            unit_position_00fc(unit, ux, uy, uz);
            const double ox = ux - own_x, oz = uz - own_z;
            d_own = std::sqrt(ox * ox + oz * oz);
            d_head = -1.0;
            if (in.path_point_valid) {
                const double hx = ux - in.path_point[0], hz = uz - in.path_point[2];
                d_head = std::sqrt(hx * hx + hz * hz);
            }
        };
        const auto& records = bsp::plane_squadron_registry().records();
        const auto owner_of = [&](std::size_t unit) -> const bsp::PlaneSquadronHostRecord* {
            for (const bsp::PlaneSquadronHostRecord& r : records) {
                if (r.squadron_unit == unit) return &r;
                for (const std::size_t m : r.member_units) if (m == unit) return &r;
            }
            return nullptr;
        };
        const auto unit_name = [&](std::size_t unit, bool squadron) -> const char* {
            if (squadron) {
                if (const bsp::PlaneSquadronHostRecord* r = owner_of(unit)) return r->name.c_str();
            }
            return unit < h.slots.size() ? h.slots[unit]->row.name.c_str() : "?";
        };
        float pick_score = 0.0f;
        for (const bsp::FreeAttackCandidateTrace& t : d.traces)
            if (t.unit == d.target && t.squadron == d.target_squadron) pick_score = t.score;
        constexpr std::size_t kCap = 24;
        std::size_t shown = 0, hidden = 0;
        std::string list;
        char buf[256];
        for (const bsp::FreeAttackCandidateTrace& t : d.traces) {
            if (shown == kCap) { ++hidden; continue; }
            // 009F7480 and 009F7876 drop before 009F7BA0's guard test; 009F7BA0
            // runs filter accept (the guard circle in C), then 00804350, then the rank.
            // Filter B scores with 009F6B10 (FLDZ) and never reaches 009F7BA0: scored_b,
            // neither test run.
            using N = bsp::cc13_seg68::SquadronCandidateNote;
            const N n = static_cast<N>(t.note);
            const bool filter_b = d.filter == bsp::band009f::SquadronTargetFilter::At14;
            const bool scored = !filter_b && (n == N::Scored || n == N::NoRank);
            const int guard_in = n == N::GuardOut || n == N::LeaderGuardOut ? 0
                : (scored || n == N::NoDetect) ? 1 : -1;
            const int detect = n == N::NoDetect ? 0 : scored ? 1 : -1;
            // t.unit is the tested unit: an 18h entry's is the arena's +3D0h leader.
            double d_own = 0.0, d_head = 0.0;
            distances(t.unit, d_own, d_head);
            std::snprintf(buf, sizeof buf, " [%s kind=%s %s rank=%d guard_in=%d detect=%d score=%.3f"
                " d=%.0f head_d=%.0f]",
                unit_name(t.unit, t.squadron), t.squadron ? "18h" : "unit",
                filter_b && n == N::Scored ? "scored_b" : bsp::free_attack_candidate_note_name(t.note),
                t.rank, guard_in, detect,
                static_cast<double>(t.score), d_own, d_head);
            list += buf;
            ++shown;
        }
        const bsp::ReconSensorPassState& recon = gun->recon_sensor_pass_state();
        for (const bsp::PlaneSquadronHostRecord& r : records) {
            if (r.party == in.party_54 || r.party < 0) continue;
            bool live = false, offered = false;
            for (const std::size_t m : r.member_units) {
                if (m == bsp::kPlaneSquadronNoUnit || m >= h.slots.size() || gun->unit_dead(m)) continue;
                live = true;
                for (const bsp::FreeAttackContact& c : in.contacts) if (c.unit == m) offered = true;
            }
            for (const bsp::FreeAttackContact& c : in.contacts) if (c.unit == r.squadron_unit) offered = true;
            if (!live || offered) continue;
            if (shown == kCap) { ++hidden; continue; }
            const std::size_t their_lead = r.flight_leader();
            const int level = recon.side_covered(in.party_54) && their_lead < h.slots.size()
                ? static_cast<int>(recon.level(in.party_54, their_lead)) : -1;
            const float min_obs = their_lead < h.slots.size()
                ? gun->recon_nearest_observer_distance(in.party_54, their_lead) : -1.0f;
            std::snprintf(buf, sizeof buf, " [%s not_in_recon level=%d min_obs_d=%.0f]",
                r.name.c_str(), level, static_cast<double>(min_obs));
            list += buf;
            ++shown;
        }
        if (hidden != 0) {
            std::snprintf(buf, sizeof buf, " (+%zu more)", hidden);
            list += buf;
        }
        // GameHostLog::notef truncates at 1024 bytes; 24 entries do not fit.
        std::snprintf(buf, sizeof buf, "free attack first pick: squadron=\"%s\" reason=%s t=%.2f "
            "filter=%d contacts=%zu traced=%zu pick=%s%s score=%.3f",
            sq.name.c_str(), reason, static_cast<double>(now), static_cast<int>(d.filter),
            in.contacts.size(), d.traces.size(), unit_name(d.target, d.target_squadron),
            d.target_squadron ? " (squadron)" : "", static_cast<double>(pick_score));
        h.log.note((buf + list).c_str());
    }
    if (!apply) return;
    if (d.raise_override_stage_2) {
        // 009F8640: mode 2 with +3Ch clear ends the override (0071D9E0(2)).
        for (const std::size_t m : alive) commands.raise_override_stage_0071d9e0(m, 2);
        ++h.free_attack_stage_raises;
    }
    if (d.stop != bsp::FreeAttackStop::kOverrideSent || d.target >= h.slots.size()) return;
    if (d.clear_then_moveto_self) ++h.free_attack_moveto_self;
    ++h.free_attack_sends;
    if constexpr (kSquadronStopHeadMoveToSelfBound) {
        if (d.clear_then_moveto_self) {
            // Packet cc13-stophead (include/bsp/squadron_stophead_cc13.hpp). The
            // `stop` head: 009F85ED 0071D880 (every slot), 009F8615 0071ECF0(moveto
            // 00E08F68 to the squadron's own position), then 009F8631 0071D980.
            // SUBSTITUTION (labelled): the clear and the moveto take the entity-order
            // hop (flags 1: 00816E30's 0081733E clear, then 0071ECF0), and the
            // host's synchronous 0071D980 runs as that chain's after-delivery, so
            // the image's post order clear, moveto, override holds in the drain.
            // The position is the arena's 00427EB0 (the leader's).
            bsp::SceneCommandTarget here{};
            here.kind = 0;
            here.position_valid = 1;
            for (int k = 0; k < 3; ++k) here.position[k] = in.squadron_position[k];
            Impl::FreeAttack* const record = &fa;
            const std::uint32_t command = d.command;
            const std::size_t target = d.target;
            const bool target_squadron = d.target_squadron;
            std::size_t deferred = 0;
            // free_attack_installed counts one per send with any member installed,
            // as the non-stop path does.
            const auto counted = std::make_shared<bool>(false);
            for (const std::size_t m : alive) {
                issue_script_command(m, 0x00E08F68u, here, 1,
                    "009F8615 free-attack moveto-self", "(own position)");
                std::function<void()> send = [this, record, counted, m, command, target,
                                              target_squadron]() {
                    Impl& host = *impl_;
                    if (m >= host.slots.size() || target >= host.slots.size()) return;
                    // 009F8631: 0071D980 -> 00721B7C -> 0071E7F0 on this member's director.
                    if (!host.commands.send_override_command_0071d980(m, command,
                            static_cast<std::uint32_t>(target + 1u))) return;
                    GameUnitSlot& u = *host.slots[m];
                    // 0099A4C0: the head's moveto task answers 0 under the override and goes.
                    u.moveto_task_installed = false;
                    u.moveto_state = GameUnitSlot::MoveToTaskState::kNone;
                    u.moveto_arrived_454 = false;
                    u.command_target_plus_one = target + 1u;
                    if (script_orders_install_command_task_0099a170(m, command, target) != 0u)
                        ++host.free_attack_tasks;
                    if (!*counted) {
                        *counted = true;
                        ++host.free_attack_installed;
                    }
                    record->override_command = command;
                    record->override_target = target + 1u;
                    record->override_squadron = target_squadron;
                    record->head_command = 0x00E08F68u;
                };
                if (commands.after_last_issue_delivery(send)) ++deferred;
                else send();
            }
            h.log.notef("free attack: squadron \"%s\" stop head: moveto own position, then "
                "override %08X on %s%s for %zu member(s), %zu after the moveto's delivery at "
                "%.2f s (009F85ED / 009F8615 / 009F8631, packet cc13-stophead)", sq.name.c_str(),
                d.command, h.slots[d.target]->row.name.c_str(),
                d.target_squadron ? " (squadron)" : "", alive.size(), deferred,
                static_cast<double>(now));
            return;
        }
    }
    std::size_t installed = 0;
    for (const std::size_t m : alive) {
        // 0071D980 -> 00721B7C -> 0071E7F0 on this member's director.
        if (!commands.send_override_command_0071d980(m, d.command,
                static_cast<std::uint32_t>(d.target + 1u))) continue;
        ++installed;
        GameUnitSlot& u = *h.slots[m];
        // 0099A4C0: the head's moveto task answers 0 under the override and goes.
        u.moveto_task_installed = false;
        u.moveto_state = GameUnitSlot::MoveToTaskState::kNone;
        u.moveto_arrived_454 = false;
        u.command_target_plus_one = d.target + 1u;
        if (script_orders_install_command_task_0099a170(m, d.command, d.target) != 0u)
            ++h.free_attack_tasks;
    }
    if (installed != 0) {
        ++h.free_attack_installed;
        fa.override_command = d.command;
        fa.override_target = d.target + 1u;
        fa.override_squadron = d.target_squadron;
        fa.head_command = in.slot0_command_54;
    }
    h.log.notef("free attack: squadron \"%s\" override %08X on %s%s to %zu of %zu member(s) "
        "at %.2f s (0071D980, packet p15_free_attack_tick)", sq.name.c_str(), d.command,
        h.slots[d.target]->row.name.c_str(), d.target_squadron ? " (squadron)" : "", installed,
        alive.size(), static_cast<double>(now));
}

void GameUnitsHost::run_spent_ordnance_rtb_0084e010() {
    static const bool census = [] {
        char* text = nullptr;
        std::size_t bytes = 0;
        bool value = false;
        if (_dupenv_s(&text, &bytes, "BSP_SPENT_RTB_CENSUS") == 0 && text != nullptr)
            value = text[0] == '1';
        std::free(text);
        return value;
    }();
    if (!bsp::kSquadronSpentOrdnanceRtbBound && !census) return;
    Impl& h = *impl_;
    GameGunneryHost* const gun = h.gunnery.get();
    if (gun == nullptr) return;
    const bool reload_open =
        !(kLuaDeviceReloadEnabledBound && lua_device_reload_enabled_00e17bf2());
    const float now = h.summary.simulated_seconds;
    std::vector<std::pair<std::size_t, std::string>> issue;
    // Packet cc13-fix1 (Impl::kSquadronCommandBuildingMoveToBound, docs/CC13_FIX1.md):
    // 009F7C90's +3Dh arm ahead of its 0071E430 (009F7D17-009F7D75). d =
    // 00521EA0([sq+4]+58h), slot 0's resolved object; d->vtable[5Ch](1Ch), an
    // MCommandBuilding (009F7D35); 0071BE40()->vtable[0Ch]() is 1 or 2 (009F7D4B /
    // 009F7D50); 0071BE60() == 1 (009F7D5D). Then 0071ECF0(00E08F68, 004684F0(d)): a
    // moveto with d as the object target (kind 1, id d+174h, position 00F87574 =
    // 0,0,0, +14h 0.0). Every failed test jumps to 009F7D7B, the end, which runs
    // either way, so the caller ends the command after this. LABELLED: [sq+4] is the
    // squadron's command block (0084D8BD-0084D8C1 pass ESI to 009F7590), the director
    // 0084E595's moveto-self issues into, so the moveto goes to the squadron unit as
    // there (flags 1 as there; 0071ECF0 takes none). The four reads take the leader's
    // director and resolved target, the proxies the host's end already uses for that
    // director. A null current command (the image would call through it) and a dead
    // squadron unit skip the issue; a dead target is not re-resolved (00521EA0's
    // handle table is not modelled), only its slot's class is tested.
    const auto command_building_moveto_009f7d17 = [&](const bsp::PlaneSquadronHostRecord& sq,
                                                      std::size_t leader) -> bool {
        const std::size_t t = h.slots[leader]->command_target_plus_one;
        if (t == 0 || !unit_is_kind_of(t - 1, 0x1C)) return false;
        const bsp::EntityOrderCommandClass* klass = bsp::entity_order_command_class_by_address(
            h.commands.current_command_0071be40(leader));
        if (klass == nullptr || (klass->category != 1 && klass->category != 2)) return false;
        int categories[bsp::kDirectorCommandSlotCount]{};
        if (h.commands.director_leading_slot_categories_0071df83(leader, categories,
                bsp::kDirectorCommandSlotCount) != 1) return false;
        if (sq.squadron_unit >= h.slots.size() || gun->unit_dead(sq.squadron_unit)) return false;
        const std::size_t d = t - 1;
        bsp::SceneCommandTarget target;
        // The existing registry identity and one-based uint16 ID are the
        // same inputs used by command resolution. Keep the constructed
        // descriptor itself on the actual 009F7D75 issue path below.
        construct_host_unit_reference(target, unit_identity(d),
            static_cast<std::uint16_t>(d + 1u));
        h.done("SquadronAutoTarget::unit_reference_004684f0", 0x004684f0u);
        const bool placed = issue_script_command(sq.squadron_unit, 0x00E08F68u, target, 1,
            "009F7D75 moveto command building", h.slots[d]->row.name) != nullptr;
        h.log.notef("command-building moveto: squadron \"%s\" -> \"%s\" (category %d, placed=%d) "
            "at %.2f s (009F7D75, packet cc13-fix1)", sq.name.c_str(),
            h.slots[d]->row.name.c_str(), klass->category, placed ? 1 : 0,
            static_cast<double>(now));
        return placed;
    };
    for (const bsp::PlaneSquadronHostRecord& sq : bsp::plane_squadron_registry().records()) {
        std::vector<std::size_t> alive;
        for (const std::size_t m : sq.member_units) {
            if (m != bsp::kPlaneSquadronNoUnit && m < h.slots.size() && !gun->unit_dead(m))
                alive.push_back(m);
        }
        if (alive.empty()) continue;
        const std::size_t leader = sq.flight_leader();
        if (leader == bsp::kPlaneSquadronNoUnit || leader >= h.slots.size()) continue;
        Impl::SpentOrdnanceRtb& st = h.spent_rtb[sq.name];
        if constexpr (Impl::kSquadronLandTargetGoneBound) {
            // 009F8160's land arm (009F8295-009F82D5): with the current command
            // `land` 00E08FA0, +3Ch = 1 unless the resolved target exists with
            // +5Eh and +5Dh clear and either the squadron's 00927C90(0) is false
            // or the target's +54h is the squadron's. LABELLED: the leader's installed land task
            // and its site stand for the current command `land` and its target
            // (the host keeps the class the order came from: an explicit land, a
            // recall, or the `land` 007F1940 issues from a returntobase
            // resolution); +5Dh is state->simulate or the gunnery death, +5Eh
            // scene_destroyed_005e. Chain 436 supplies 00927C90's actual role-0
            // predicate and the party check; OFF preserves the old false predicate.
            // One latch per lost site: the B5 that follows starts a fresh cycle
            // (the image's latch outlives it, its reset is unread).
            const GameUnitSlot& lead = *h.slots[leader];
            std::size_t site = h.slots.size();
            bool land = false;
            if (lead.land_task_installed && lead.land_site_plus_one != 0) {
                land = true;
                site = lead.land_site_plus_one - 1u;
            }
            if (land) {
                const GameUnitSlot* ts = site < h.slots.size() ? h.slots[site].get() : nullptr;
                const bool gone = [&] {
                    if (ts == nullptr || (ts->state != nullptr && ts->state->simulate != 0)
                        || ts->scene_destroyed_005e != 0 || gun->unit_dead(site)) return true;
                    // bsp.py show 009f8160 --asm, 009F82AD..009F82C8:
                    // ECX = [bot+8] (the squadron), NOT ESI (the target),
                    // for 00927C90(0). A player/open squadron may keep a
                    // live landing target only when their parties match.
                    // The host fuses the squadron with wing 0. Read that
                    // live slot's party so SetPartyRace changes are visible;
                    // without its receiver retain only the target-loss test.
                    if (sq.squadron_unit >= h.slots.size()
                        || h.slots[sq.squadron_unit] == nullptr) return false;
                    if (!unit_role_permission_is_player_or_open(sq.squadron_unit, 0)) return false;
                    return unit_side_0054(site) != unit_side_0054(sq.squadron_unit);
                }();
                const std::size_t key = site < h.slots.size() ? site + 1u : 0u;
                if (gone && st.land_gone_site != key) {
                    st = Impl::SpentOrdnanceRtb{};
                    st.land_gone_site = key;
                    st.latch_3c = true;
                    st.latch_at = now;
                    st.command = lead.attack_command_class;
                    ++h.spent_rtb_latches;
                    h.log.notef("land target gone: squadron \"%s\" (command %08X, site %s) latched "
                        "+3Ch at %.2f s (009F82D5, packet cc9_p12_land_target_gone)", sq.name.c_str(),
                        st.command, ts != nullptr ? ts->row.name.c_str() : "-",
                        static_cast<double>(now));
                }
            }
        }
        // 009F7CAD..009F7CD8: for a non-fighter, sample 007ED920(0),
        // latch +3Ch on true->false when device reload is disabled, then
        // store +28h regardless of reload. This does not depend on command
        // class: the existing 009F8160 arm below has a different gate.
        if (!unit_is_kind_of(leader, 0x13)) {
            struct RocketMembers final : bsp::Cc13SquadronRocketHost {
                Impl& owner;
                const std::vector<std::size_t>& members;
                RocketMembers(Impl& o, const std::vector<std::size_t>& m)
                    : owner(o), members(m) {}
                std::int32_t rocket_member_count(void*) override {
                    return static_cast<std::int32_t>(members.size());
                }
                void* rocket_member(void*, std::int32_t i) override {
                    return owner.slots[members[static_cast<std::size_t>(i)]].get();
                }
                bool weapon_controller_has_rocket_kind33_007b9400(
                    void* member, std::uint32_t loadout) override {
                    // 007B91C0 -> rack[220h](0): 006E4060 / 006E4640
                    // inspect held rounds, with no loadout fallback for 0.
                    // Use the actual rocket firing counters, not the class
                    // ordnance mask (multi-rack depletion never clears it).
                    // SUBSTITUTION: as in the rocket release hosts, positive
                    // attached/reserve rounds stand for the rack's child.
                    if (loadout != 0) throw std::logic_error("unbound rocket loadout query");
                    static_assert(bsp::kOpportunisticRocketBound);
                    auto& unit = *static_cast<GameUnitSlot*>(member);
                    owner.plane_rack_census(unit); // lazy authored equipment/round producer
                    for (std::size_t i = 0; i < unit.orx_rack_kind.size()
                            && i < unit.orx_rack_ammo.size(); ++i) {
                        // Census kind 1/2/3 all come from Bullets.Type=Rocket
                        // (descriptor kind 33h); both release hosts decrement
                        // this same ammo array after a successful launch.
                        if (unit.orx_rack_kind[i] > 0 && unit.orx_rack_ammo[i] > 0) return true;
                    }
                    return false;
                }
            } rockets(h, alive);
            const bool current = bsp::cc13_seg48_plane_squadron_any_has_rockets_007ed920(
                const_cast<bsp::PlaneSquadronHostRecord*>(&sq), 0, rockets);
            bool& previous = h.squadron_rockets_previous_28[sq.name];
            if (previous && !current && reload_open && !st.latch_3c) {
                st.latch_3c = true;
                st.latch_at = now;
                st.command = h.slots[leader]->attack_command_class;
                ++h.spent_rtb_latches;
                h.log.notef("spent rockets: squadron \"%s\" latched at %.2f s "
                    "(007ED920 / 009F7CD4, chain419)", sq.name.c_str(), static_cast<double>(now));
            }
            previous = current;
        }
        if (st.issued) continue;
        const std::uint32_t command = h.slots[leader]->attack_command_class;
        // 009F8160, the ordnance arm (009F81C8-009F8205).
        if (!st.latch_3c && bsp::spent_ordnance_command_class_009f8160(command)) {
            bool any = false;
            for (const std::size_t m : alive) {
                // 007B914A: an IsKindOf(17h) member answers 007B9140 before its racks
                // (packet cc13-kzlatch, docs/CC13_KZ_LATCH.md).
                if ((bsp::cc13_kz_latch::kKamikazeRackRoundLatchCc13Bound && unit_is_kind_of(m, 0x17))
                        || plane_holds_rack_round_007b9140(m)) { any = true; break; }
            }
            if (reload_open && !any && !unit_is_kind_of(leader, 0x13)) {
                st.latch_3c = true;
                st.latch_at = now;
                st.command = command;
                ++h.spent_rtb_latches;
                h.log.notef("spent ordnance: squadron \"%s\" latched (command %08X, %zu live) "
                    "at %.2f s (009F8205, packet cc9_squadron_spent_ordnance_rtb)",
                    sq.name.c_str(), command, alive.size(), static_cast<double>(now));
            }
        }
        if constexpr (Impl::kFighterSpentEndBound) {
            // 009F8160's ordnance arm for a fighter (009F81C8-009F81FC, 009F8412):
            // with the seven ordnance classes, ([+369h] clear or [00E17BF2] clear)
            // and 007ED790 false, a squadron whose class descriptor answers
            // IsKindOf(13h) sets +3Dh instead of +3Ch. 009F7C90's +3Dh arm
            // (009F7D01-009F7D45) then, with 007EE720 true, ends the current
            // command: 0071E430(current, 1). LABELLED, as the dead-target retest
            // above models the same arm: 007EE720 (each member's head task
            // vtable[20h]; the dive and torpedo tasks answer 1) is taken as true,
            // the kind-1Ch moveto re-issue is modelled only with
            // kSquadronCommandBuildingMoveToBound (cc13-fix1), and each live member's
            // rows are retired with its resolved target cleared. One end per
            // command class: a new order of the same class is ended again only
            // after a different class came between.
            if (!st.latch_3c && command != st.fighter_ended_command && reload_open
                    && bsp::spent_ordnance_command_class_009f8160(command)
                    && unit_is_kind_of(leader, 0x13)) {
                bool any = false;
                for (const std::size_t m : alive) {
                    // 007B914A: an IsKindOf(17h) member answers 007B9140 before its racks
                    // (packet cc13-kzlatch, docs/CC13_KZ_LATCH.md).
                    if ((bsp::cc13_kz_latch::kKamikazeRackRoundLatchCc13Bound && unit_is_kind_of(m, 0x17))
                            || plane_holds_rack_round_007b9140(m)) { any = true; break; }
                }
                if (!any) {
                    st.fighter_ended_command = command;
                    bool queued = false;
                    if constexpr (Impl::kSquadronCommandBuildingMoveToBound)
                        queued = command_building_moveto_009f7d17(sq, leader);   // 009F7D17-009F7D75
                    const GameCommandCompletion done = end_command_0071e430(leader, command, true);
                    if (done.ran) {
                        st.ended_stage2_command = command;
                        st.building_moveto_queued = queued;
                    }
                    for (const std::size_t m : alive) {
                        h.commands.retire_ended_command_rows(m, command);
                        h.slots[m]->command_target_plus_one = 0;
                    }
                    h.log.notef("fighter spent: squadron \"%s\" +3Dh, command %08X ended (%s, "
                        "stage %d) at %.2f s (009F81FC / 009F7D3A, packet cc9_p12_fighter_spent_3d)",
                        sq.name.c_str(), command, done.arm.empty() ? "-" : done.arm.c_str(),
                        done.stage_after, static_cast<double>(now));
                }
            } else if (command != st.fighter_ended_command && st.fighter_ended_command != 0
                       && !bsp::spent_ordnance_command_class_009f8160(command)) {
                st.fighter_ended_command = 0;
            }
        }
        // DIAGNOSTIC (BSP_SPENT_RTB_CENSUS=1): a squadron with every live member
        // spent that did not latch, logged once per leader command class.
        if (census && !st.latch_3c && command != st.census_unlatched_class) {
            bool any = false;
            for (const std::size_t m : alive) {
                // 007B914A: an IsKindOf(17h) member answers 007B9140 before its racks
                // (packet cc13-kzlatch, docs/CC13_KZ_LATCH.md).
                if ((bsp::cc13_kz_latch::kKamikazeRackRoundLatchCc13Bound && unit_is_kind_of(m, 0x17))
                        || plane_holds_rack_round_007b9140(m)) { any = true; break; }
            }
            if (!any) {
                st.census_unlatched_class = command;
                h.log.notef("spent ordnance census: squadron \"%s\" spent, not latched: leader %s "
                    "command %08X ordnance_class=%d fighter=%d reload_open=%d at %.2f s",
                    sq.name.c_str(), h.slots[leader]->row.name.c_str(), command,
                    bsp::spent_ordnance_command_class_009f8160(command) ? 1 : 0,
                    unit_is_kind_of(leader, 0x13) ? 1 : 0, reload_open ? 1 : 0,
                    static_cast<double>(now));
            }
        }
        // Packet cc9-gunnery38 (bsp::kSquadronDeadTargetRetestBound): 009F8160's
        // ordnance arm with ordnance left (009F81E5..009F8212: 007ED790 true, +3Dh
        // clear, the resolved target absent or failing 007EE8F0 -> 009F8412) and
        // 009F7C90's +3Dh arm (009F7D01..009F7D45; 007EE720 true for the torpedo and
        // dive tasks). LABELLED: the target is the leader's resolved command
        // target and only its death is tested (007EE8F0 is not re-asked, a null
        // target is not taken as gone); depthcharge's 009F81C6 branch is not
        // modelled, and 009F7D1D's kind-1Ch moveto re-issue only with
        // kSquadronCommandBuildingMoveToBound (cc13-fix1).
        if (bsp::kSquadronDeadTargetRetestBound && !st.latch_3c
                && bsp::spent_ordnance_command_class_009f8160(command)
                && command != bsp::kAttackCmdDepthCharge) {
            const std::size_t t = h.slots[leader]->command_target_plus_one;
            bool any = false;
            for (const std::size_t m : alive) {
                // 007B914A: an IsKindOf(17h) member answers 007B9140 before its racks
                // (packet cc13-kzlatch, docs/CC13_KZ_LATCH.md).
                if ((bsp::cc13_kz_latch::kKamikazeDeadTargetRetestCc13Bound && unit_is_kind_of(m, 0x17))
                        || plane_holds_rack_round_007b9140(m)) { any = true; break; }
            }
            if (any && t != 0 && t != st.retest_target && gun->unit_dead(t - 1)) {
                st.retest_target = t;
                ++h.retest_dead_targets;
                bool queued = false;
                if constexpr (Impl::kSquadronCommandBuildingMoveToBound)
                    queued = command_building_moveto_009f7d17(sq, leader);   // 009F7D17-009F7D75
                const GameCommandCompletion done = end_command_0071e430(leader, command, true);
                if (done.ran) ++h.retest_ends;
                if (done.ran) {
                    st.ended_stage2_command = command;   // packet cc9_l57_moveto_self
                    st.building_moveto_queued = queued;
                }
                // After 0071E430 the director answers no attack class, so 0071EBF0
                // falls to its neutral record (00E19BB4): the rows stop being current
                // and the members' resolved target is cleared. LABELLED: the host's
                // commands are per unit, so each live member's rows are retired.
                for (const std::size_t m : alive) {
                    h.commands.retire_ended_command_rows(m, command);
                    if (h.slots[m]->command_target_plus_one == t) h.slots[m]->command_target_plus_one = 0;
                }
                h.log.notef("squadron retest: \"%s\" target %s dead at %.2f s, command %08X "
                    "ended (%s, stage %d) (009F8412 / 009F7D3A, cc9-gunnery38)", sq.name.c_str(),
                    h.slots[t - 1]->row.name.c_str(), static_cast<double>(now), command,
                    done.arm.empty() ? "-" : done.arm.c_str(), done.stage_after);
            }
        }
        // 009F7C90 (009F7CE0-009F7CFC): every member's task breaks off.
        if (st.latch_3c && !st.ended) {
            bool all = true;
            for (const std::size_t m : alive) {
                const GameUnitSlot& u = *h.slots[m];
                if (u.dive_bomb_task_installed) all = all && u.db_dbg_break_off;
                else if (u.torpedo_task_installed) all = all && u.torpedo_breakoff_now;
                // Packet cc9_plane_level_bomb_task: its vtable[1Ch] 009B8D80.
                else if (bsp::kPlaneLevelBombTaskBound && u.level_bomb_task_installed)
                    all = all && u.lb_break_off_now;
                // Packet cc9_dkz_spent_rtb: the drop-kamikaze task's vtable[1Ch]
                // 009AE1B0 (docs/CC13_GPR_38.md 1.3 step 4).
                else if (Impl::kDropKamikazeSpentRtbBound && u.drop_kamikaze_task_installed)
                    all = all && h.drop_kamikaze_breaks_off_009ae1b0(u);
            }
            if (all) {
                st.ended = true;
                st.ended_at = now;
                ++h.spent_rtb_ends;
            }
        }
        // Packet p15_free_attack_tick: 009F8470's selection, after 009F8160 / 009F7C90
        // and before the step 0084E010 (0071F290 arm 7).
        if (Impl::kSquadronFreeAttackTickBound || free_attack_census_enabled())
            run_squadron_free_attack_009f8470(sq, leader, alive);
        // 0084E010's B5 (0084E4A4-0084E5C6).
        if (st.ended && reload_open) {
            st.issued = true;
            st.issued_at = now;
            ++h.spent_rtb_issues;
            issue.emplace_back(leader, sq.name);
            h.log.notef("spent ordnance: squadron \"%s\" B5 at %.2f s (latched %.2f, ended "
                "%.2f): 007F16D0 %s (0084E4A4, packet cc9_squadron_spent_ordnance_rtb)",
                sq.name.c_str(), static_cast<double>(now), static_cast<double>(st.latch_at),
                static_cast<double>(st.ended_at),
                bsp::kSquadronSpentOrdnanceRtbBound ? "issued" : "census only");
        }
    }
    if constexpr (bsp::kSquadronSpentOrdnanceRtbBound) {
        for (const auto& e : issue) {
            h.spent_rtb[e.second].placed = issue_return_to_base_007f16d0(e.first, "0084E010 B5");
        }
    }
    if constexpr (Impl::kSquadronMoveToSelfBound) {
        // 0084E4B1-0084E5C6 through squadron_step_issues_moveto_self. LABELLED: the
        // stage is 2 once the retest ended the command (0071E430) or B5 ran; the queue
        // holds that one command (0071BE60() = 1); +368h is 0 and +369h is 1 (007F2D09).
        const bool device_reload = kLuaDeviceReloadEnabledBound && lua_device_reload_enabled_00e17bf2();
        for (const bsp::PlaneSquadronHostRecord& sq : bsp::plane_squadron_registry().records()) {
            auto it = h.spent_rtb.find(sq.name);
            if (it == h.spent_rtb.end()) continue;
            Impl::SpentOrdnanceRtb& st = it->second;
            const std::size_t leader = sq.flight_leader();
            if (leader == bsp::kPlaneSquadronNoUnit || leader >= h.slots.size()) continue;
            if (gun->unit_dead(leader)) continue;
            const std::uint32_t current = h.slots[leader]->attack_command_class;
            if (st.moveto_self) {
                // A new order replaced the moveto-self: a fresh cycle.
                if (current != 0x00E08F68u) {
                    st.moveto_self = false;
                    st.ended_stage2_command = 0;
                    st.building_moveto_queued = false;
                }
                continue;
            }
            bsp::SquadronStepTailInputs in;
            in.latch_3c = st.latch_3c;
            in.command_54 = st.ended_stage2_command != 0 ? st.ended_stage2_command : st.command;
            in.reload_enabled_369 = true;
            in.device_reload_e17bf2 = device_reload;
            in.flag_368 = false;
            const bool stage2 = (st.ended_stage2_command != 0 && current == st.ended_stage2_command)
                || st.ended;
            in.stage_48 = stage2 ? 2 : 0;
            in.override_stage_50 = st.ended ? 2 : 0;
            // cc13-fix1: the command-building moveto queued behind the ended command
            // is a second filled slot, so 0084E4B8 and 0084E5A6 both refuse.
            in.filled_slots_71be60 = st.building_moveto_queued ? 2 : 1;
            in.head_arm = false;
            const bsp::SquadronStepTailPlan plan = bsp::squadron_step_tail_plan_0084e4b1(in);
            bool go = false;
            if (plan.outcome == bsp::SquadronStepTailOutcome::kMoveToSelfIfRoom) {
                go = bsp::squadron_step_moveto_self_0084e595(in.filled_slots_71be60, in.stage_48);
            } else if (plan.outcome == bsp::SquadronStepTailOutcome::kResolve
                       && st.issued && st.placed == 0) {
                go = bsp::squadron_step_moveto_self_0084e595(in.filled_slots_71be60,
                                                             in.stage_48);   // 0084E55B JE
            }
            if (!go) continue;
            if (sq.squadron_unit >= h.slots.size()) continue;
            bsp::SceneCommandTarget target;
            target.kind = 1;
            target.position_valid = 0;
            target.object_id = static_cast<std::uint16_t>(sq.squadron_unit + 1);
            target.object = const_cast<void*>(unit_identity(sq.squadron_unit));
            target.position[0] = target.position[1] = target.position[2] = 0.0f;
            target.trailing = 0.0f;
            // 0084E5A8-0084E5C6: ECX = the squadron's own command block (ESI), the
            // target 00465080([ESI+224h] = the squadron, 0.0), so the moveto goes
            // into the squadron's director once (cc9-lua59), not to each member.
            std::size_t placed = 0;
            if (!gun->unit_dead(sq.squadron_unit) &&
                issue_script_command(sq.squadron_unit, 0x00E08F68u, target, 1,
                                     "0084E595 moveto-self",
                                     h.slots[sq.squadron_unit]->row.name) != nullptr) {
                ++placed;
            }
            if constexpr (Impl::kSquadronMoveToSelfWingFollowBound) {
                std::size_t wings = 0;
                for (const std::size_t m : sq.member_units) {
                    if (placed == 0 || m == bsp::kPlaneSquadronNoUnit || m >= h.slots.size()
                        || m == sq.squadron_unit || gun->unit_dead(m)) continue;
                    if (issue_script_command(m, 0x00E08F68u, target, 1,
                            "0084E595 moveto-self wing",
                            h.slots[sq.squadron_unit]->row.name) != nullptr) {
                        ++wings;
                    }
                }
                h.moveto_self_wings += wings;
                h.log.notef("squadron moveto-self wings: \"%s\" at %.2f s wings=%zu (0099A170 "
                    "member arm, packet s65_moveto_self_wing_follow)", sq.name.c_str(),
                    static_cast<double>(now), wings);
            }
            st.moveto_self = true;
            ++h.moveto_self_issues;
            h.moveto_self_placed += placed;
            h.log.notef("squadron moveto-self: \"%s\" at %.2f s (latch=%d ended=%d stage2=%08X "
                "b4=%d b5=%d placed=%zu) (0084E595, packet cc9_l57_moveto_self)", sq.name.c_str(),
                static_cast<double>(now), st.latch_3c ? 1 : 0, st.ended ? 1 : 0,
                st.ended_stage2_command, plan.b4 ? 1 : 0, plan.b5 ? 1 : 0, placed);
        }
    }
    if constexpr (kSquadronGeneralStepTailBound) {
        // Packet cc13-steptail (include/bsp/squadron_step_tail_cc13.hpp): 0084E010's
        // tail for every squadron the spent_rtb path above does not hold.
        // Runs only past this function's early return, so only while
        // kSquadronSpentOrdnanceRtbBound is true (or the census is set).
        const bool device_reload = kLuaDeviceReloadEnabledBound && lua_device_reload_enabled_00e17bf2();
        for (const bsp::PlaneSquadronHostRecord& sq : bsp::plane_squadron_registry().records()) {
            // Membership, 0071F290's gates ahead of its vt[7Ch] call (0071F39E), held
            // the way 009F8470's host call above is: a live member and a flight leader.
            bool any_alive = false;
            for (const std::size_t m : sq.member_units) {
                if (m != bsp::kPlaneSquadronNoUnit && m < h.slots.size() && !gun->unit_dead(m)) {
                    any_alive = true;
                    break;
                }
            }
            if (!any_alive) continue;
            const std::size_t leader = sq.flight_leader();
            if (leader == bsp::kPlaneSquadronNoUnit || leader >= h.slots.size()) continue;
            // 0071F29F..0071F2C1: the owner entity's +5Ch set, +5Dh / +60h / +5Eh clear.
            // LABELLED: the host's squadron entity is the fused wing 0 (sq.squadron_unit),
            // so a wing 0 still in the hangar (007BC550 cleared its +5Ch) holds the tail
            // through take-off. The live gate tests wing 0's state and scene flags, not
            // gunnery death: a gunnery-dead but active wing 0 passes, and the moveto-self
            // below is then skipped; a destroyed wing 0 ends the tail for the whole
            // squadron, where the image's separate squadron entity would continue.
            // 0071F378 ([[00E188A8]+1FE4h] != 2) always passes here: the mode is != 2 in
            // every host (0 or 1; single player).
            if (sq.squadron_unit >= h.slots.size()) continue;
            {
                const GameUnitSlot& s = *h.slots[sq.squadron_unit];
                const bool live = s.state != nullptr && s.state->active != 0 &&
                                  s.state->simulate == 0 && s.scene_destroyed_005e == 0 &&
                                  s.scene_pending_destroy_0060 == 0;
                if (!live) continue;
            }
            // PROXY: 0084E043 tests +3D0h for null, not for death.
            if (gun->unit_dead(leader)) continue;
            const auto rtb_it = h.spent_rtb.find(sq.name);
            if (rtb_it != h.spent_rtb.end()) {
                const Impl::SpentOrdnanceRtb& st = rtb_it->second;
                if (st.latch_3c || st.ended || st.ended_stage2_command != 0 || st.moveto_self) continue;
            }
            const std::uint32_t head = h.commands.director_slot_command(leader, 0);
            bsp::SceneCommandTarget head_target{};
            h.commands.director_slot_target(leader, 0, head_target);
            Impl::StepTailGuard& guard = h.step_tail_guard[sq.name];
            if (guard.issued) {
                const bool same_head = head == guard.head
                    && head_target.kind == guard.head_target.kind
                    && head_target.object_id == guard.head_target.object_id
                    && std::memcmp(head_target.position, guard.head_target.position,
                                   sizeof head_target.position) == 0;
                // LABELLED: the squadron-unit issue is taken to become the leader's
                // slot 0, the spent_rtb path's established proxy. After it completes
                // the image re-issues (0071ECF0 re-arms the stage); the host stays at
                // stage 2.
                const bool own_moveto = head == 0x00E08F68u && head_target.kind == 1
                    && head_target.object_id == static_cast<std::uint16_t>(sq.squadron_unit + 1);
                if (same_head || own_moveto) continue;
                guard.issued = false;
            }
            const int filled = h.commands.director_command_count_0071be60(leader);
            const int stage = h.commands.director_stage_0048(leader);
            const bsp::EntityOrderCommandClass* klass =
                bsp::entity_order_command_class_by_address(head);
            bsp::SquadronStepTailInputs in;
            in.stage_48 = stage;
            in.override_stage_50 = h.commands.director_override_stage_0050(leader);
            in.filled_slots_71be60 = filled;
            in.latch_3c = false;
            in.command_54 = head;
            in.command_category_0c = klass != nullptr ? klass->category : 0;
            in.head_arm = head == 0u && filled == 0;                          // 0084E047-0084E05C
            in.reload_enabled_369 = true;
            in.device_reload_e17bf2 = device_reload;
            in.flag_368 = false;
            const bsp::SquadronStepTailPlan plan = bsp::squadron_step_tail_plan_0084e4b1(in);
            std::size_t rtb = 0;
            bool moveto = false;
            if (plan.outcome == bsp::SquadronStepTailOutcome::kResolve) {
                rtb = issue_return_to_base_007f16d0(leader, "0084E552 step tail resolve");
                if (rtb == 0) moveto = bsp::squadron_step_moveto_self_0084e595(filled, stage);  // 0084E55B
            } else if (plan.outcome == bsp::SquadronStepTailOutcome::kMoveToSelfIfRoom) {
                moveto = bsp::squadron_step_moveto_self_0084e595(filled, stage);
            }
            if (rtb == 0 && !moveto) continue;
            std::size_t placed = 0;
            if (moveto && sq.squadron_unit < h.slots.size() && !gun->unit_dead(sq.squadron_unit)) {
                // 0084E5A8-0084E5C6, as the spent_rtb path issues it.
                bsp::SceneCommandTarget target;
                target.kind = 1;
                target.position_valid = 0;
                target.object_id = static_cast<std::uint16_t>(sq.squadron_unit + 1);
                target.object = const_cast<void*>(unit_identity(sq.squadron_unit));
                target.position[0] = target.position[1] = target.position[2] = 0.0f;
                target.trailing = 0.0f;
                if (issue_script_command(sq.squadron_unit, 0x00E08F68u, target, 1,
                        "0084E595 step tail moveto-self",
                        h.slots[sq.squadron_unit]->row.name) != nullptr)
                    ++placed;
                ++h.step_tail_movetos;
            }
            if (rtb != 0) ++h.step_tail_rtbs;
            // LABELLED: set even when placed == 0 (the issue was refused), so the
            // host never retries this head; the image retries every step.
            guard.issued = true;
            guard.head = head;
            guard.head_target = head_target;
            h.log.notef("squadron step tail: \"%s\" at %.2f s head %08X stage %d slots %d "
                "b4=%d b5=%d -> %s (rtb placed %zu, moveto placed %zu) (0084E4B1..0084E5C6, "
                "packet cc13-steptail)", sq.name.c_str(), static_cast<double>(now), head, stage,
                filled, plan.b4 ? 1 : 0, plan.b5 ? 1 : 0,
                rtb != 0 ? "007F16D0 resolve" : "0084E595 moveto-self", rtb, placed);
        }
    }
}

void GameUnitsHost::run_landing_queue_006cd240(float dt) {
    run_spent_ordnance_rtb_0084e010();   // packet cc9_squadron_spent_ordnance_rtb
    if constexpr (!kLandingSequencerBound) {
        (void)dt;
        return;
    } else {
        Impl& h = *impl_;
        if (!h.lua.plane_globals_loaded()) return;
        if constexpr (kBaseLaunchChainBound) {
            // Packet cc9_base_launch_task: each squadron's launch task 007F1F00.
            // POSITION, LABELLED: the image ticks it from its task list
            // (00876020's registration), whose place in the frame is unread;
            // it runs here, before the site ticks that read site+18h.
            for (bsp::PlaneSquadronHostRecord& sq : bsp::plane_squadron_registry().records()) {
                h.base_launch_task_tick_007f1f00(sq, dt);
            }
            // Packet cc9_base_launch_deck_arms: 006CDC70's site slot +4h for an
            // airfield, 006CF980. LABELLED: run here, beside the mother ship's
            // 006D0600 below.
            for (std::size_t i = 0; i < h.landing_decks.size(); ++i) {
                Impl::LandingDeck& d = h.landing_decks[i];
                if (!d.built || d.refused || d.mother_ship) continue;
                h.airfield_site_tick_006cf980(i, d);
            }
        }
        if constexpr (Impl::kCarrierElevatorBound) {
            // 006CDC70's site slots +4h (006D0600) and +8h (006CFE40 -> 006FC480);
            // LABELLED: run here, before the queue, in that order.
            for (Impl::LandingDeck& d : h.landing_decks) {
                if (!d.built || d.refused || !d.mother_ship) continue;
                if (h.carrier_elevator_ready(d) == nullptr) continue;
                h.carrier_elevator_site_tick_006d0600(d, dt);
                h.carrier_elevator_platform_tick_006fc480(d, dt);
            }
        }
        if constexpr (Impl::kAirOpsQueueCompactBound) {
            // 006C77E0 (006C77E0-006C7951, __fastcall(block), RET), 006CDC70's third
            // sub-update: under the block's lock, erase every 14h record at +A8h whose
            // plane is null or fails 0043F080's test ([+5Ch] set, [+5Dh], [+60h] and
            // [+5Eh] clear; 006C788D-006C78B6), shifting the tail down. LABELLED: it
            // runs here, before the queue tick, where 006CDC70 runs it before the site
            // virtual, 006C64B0 and 006C6540 (none of which reads +A8h); +5Dh is the
            // host's death predicate (plane_death_c3a / removed / gunnery dead), as for
            // the leg manager above.
            // 006C77E0: the typed body's index walk and order-keeping erase over each deck's
            // assignment records, its 0043F080 test answered by the host's death predicate;
            // the host steps on one thread, so the block+8h lock pair is empty. OFF keeps
            // the inline walk below.
            struct LandingDeckAssignmentRecords {
                GameUnitsHost& units;
                Impl& h;
                Impl::LandingDeck& d;
                std::uint32_t size() const { return static_cast<std::uint32_t>(d.assignments.size()); }
                const GameUnitSlot* slot(std::uint32_t i) const {
                    const std::size_t pi = d.assignments[i].plane;
                    return pi < h.slots.size() ? h.slots[pi].get() : nullptr;
                }
                bool plane_is_null(std::uint32_t i) const { return slot(i) == nullptr; }
                // 0043F080's test: the host's death predicate.
                bool plane_fails_0043f080(std::uint32_t i) const {
                    const std::size_t pi = d.assignments[i].plane;
                    const GameUnitSlot* s = slot(i);
                    return !units.unit_alive_and_visible(pi) ||
                        s->plane_death_c3a || s->plane_death_removed ||
                        (h.gunnery != nullptr && h.gunnery->unit_dead(pi));
                }
                void erase(std::uint32_t i) {
                    const GameUnitSlot* s = slot(i);
                    h.log.notef("  landing sequencer: record of %s erased (mode %d path=%.1f) at "
                        "%.2f s (006C77E0, packet cc9_p12_queue_compact)",
                        s != nullptr ? s->row.name.c_str() : "?", d.assignments[i].mode_10,
                        static_cast<double>(d.assignments[i].path_4),
                        static_cast<double>(h.summary.simulated_seconds));
                    d.assignments.erase(d.assignments.begin() + static_cast<std::ptrdiff_t>(i));
                }
            };
            for (Impl::LandingDeck& d : h.landing_decks) {
                LandingDeckAssignmentRecords records{*this, h, d};
                bsp::cc13_sweepC::air_ops_block_compact_records_006c77e0(records, [] {}, [] {});
            }
        }
        for (Impl::LandingDeck& d : h.landing_decks) {
            if (!d.built || d.refused) continue;
            for (std::size_t i = 0; i < d.queue.size(); ++i) {
                Impl::LandingQueueEntry& q = d.queue[i];
                const float c = q.countdown_0c - dt;
                q.countdown_0c = c;
                if (!(0.0f > c)) continue;
                q.countdown_0c = q.interval_10;
                const float res = h.landing_sequencer_006cc9f0(d, q.squadron);
                q.interval_10 = res;
                q.countdown_0c = res > q.countdown_0c ? q.countdown_0c : res;
            }
        }
        h.done("AirOps::landing_queue_tick", 0x006cd240u);
        if constexpr (bsp::kBaseLaunchBrakeBound) h.release_launch_brake_006c5b70();
    }
}

// Packet cc9_base_launch_brake (docs/SQUADRON_LAND_TASK.md 5bj). 006C5B70
// (006C5B70-006C5C34, __thiscall(block, float), RET 4), which 006CDC70 runs after
// 006CD240 when block+1Ch and +1Dh are clear and the owner at block+7Ch is
// present with +5Dh clear (006CDCE1-006CDD04). With block+38h set and 007ED740
// true, it re-publishes every slot whose +28h is that squadron (006BF150, a
// slot notice: counted, not carried) and clears block+38h (006C5C05-006C5C0A).
// 007ED740 (007ED740-007ED782): +3CCh > 0 and 007B8BD0 true for each +3D0h
// member. 007B8BD0 (007B8BD0-007B8C15): true for state 7 (+72Ch vtable[38h] is
// 0074E210, `+1D4h == 7` on the sub-object, i.e. +900h), for state 6, and for
// state 4 with +BF8h set; false otherwise.
// SUBSTITUTIONS, labelled:
//  - a member whose scene node is torn down, destroyed or removed is skipped, as
//    the +3D0h compaction at death (007F3970) would drop it;
//  - the observer pair's own clear (0065AFF0 zeroes +14h when the observed
//    squadron notifies) is stood in for by a squadron whose resolved members are
//    all gone; its notice's sender is unread;
//  - a squadron whose members this host has not resolved yet holds the brake.
void GameUnitsHost::Impl::release_launch_brake_006c5b70() {
    bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
    for (std::size_t di = 0; di < decks.size(); ++di) {
        bsp::AirOpsDeck* deck = decks.mutable_at(di);
        if (deck == nullptr || deck->launch_in_progress == 0u) continue;
        if (deck->runway_failure || deck->hangar_failure) continue;
        if (!deck->owner_present || deck->owner_blocked) continue;
        // The entity id is the unit index plus one (007F4E9E's reader above).
        const std::size_t sq_unit = static_cast<std::size_t>(deck->launch_in_progress - 1u);
        const bsp::PlaneSquadronHostRecord* rec = nullptr;
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit == sq_unit) { rec = &r; break; }
        }
        if (rec == nullptr || rec->member_units.empty()) continue;
        if constexpr (bsp::kBaseLaunchBrakeAttachGateBound) {
            // 007ED740 `+3CCh > 0`: no member is attached before pass C's inside start.
            if (rec->from_air_ops_launch && rec->bag_state < 2 && !rec->home_launch_408) {
                ++base_launch_brake_attach_holds;
                continue;
            }
        }
        std::size_t resolved = 0, live = 0;
        bool every = true;
        {
            std::vector<bsp::PlaneSquadronFlightMemberView> members;
            members.reserve(rec->member_units.size());
            for (const std::size_t m : rec->member_units) {
                if (m == bsp::kPlaneSquadronNoUnit || m >= slots.size() || !slots[m]) continue;
                ++resolved;
                const GameUnitSlot& p = *slots[m];
                if (p.scene_flags_available && p.state != nullptr && (p.state->simulate != 0
                        || p.scene_destroyed_005e != 0 || p.scene_removed_005f != 0)) {
                    continue;
                }
                // 007ED740 calls 007B8BD0 with each attached plane (007ED755).
                // The host's +72Ch vslot[38h] answer is state 7 (0074E210).
                // +900h is produced by the flight-state transitions;
                // +BF8h by the deck/contact probe 007C5F0A.
                // Keep the existing dead-member compaction and observer-gone
                // handling outside the predicate; there are no synthetic members.
                members.push_back({p.plane_control_mode_900 == 7,
                    p.plane_control_mode_900, p.plane_ground_contact_bf8});
            }
            live = members.size();
            const bsp::PlaneSquadronFlightReadinessView view{
                members.data(), static_cast<std::int32_t>(members.size())};
            every = bsp::cc13_seg48_plane_squadron_all_free_flight_or_parked_007ed740(view);
            done("AirOps::squadron_all_free_flight_or_parked", 0x007ed740u);
        }
        if (resolved == 0) continue;
        const bool gone = live == 0;
        if (!gone && !every) continue;
        if (gone) ++base_launch_brake_gone; else ++base_launch_brake_releases;
        if (base_launch_brake_releases + base_launch_brake_gone <= 40) {
            log.notef("base launch brake: deck %s releases squadron %s at %.2f s (%s; "
                "006CDD04 -> 006C5B70 -> 006C5C0A, packet cc9_base_launch_brake)",
                decks.name_at(di).c_str(), rec->name.c_str(),
                static_cast<double>(summary.simulated_seconds),
                gone ? "no live member: the observer notice, labelled"
                     : "007ED740: every member in 7, 6 or 4 on the ground");
        }
        deck->launch_in_progress = 0u;
        ++deck->launch_brake_clears;
    }
    done("AirOps::republish_spotted_slot", 0x006c5b70u);
}

void GameUnitsHost::create_units(const std::vector<GameSceneEntityRecord>& entities) {
    Impl& host = *impl_;
    if (host.scene_commands == nullptr) {
        host.scene_commands = std::make_unique<UnitSceneCommandQueue>();
    }
    for (const GameSceneEntityRecord& entity : entities) {
        if (!entity.created) continue;
        auto slot = std::make_unique<GameUnitSlot>();
        if (entity.class_id == bsp::kUnitClassPlaneSquadron) {
            // bsp.py show 007F2BD0 --asm: 007F2D2B in the squadron ctor
            // passes squadron+37Ch; +14h is its +390h, NOT allocation zero.
            // load_gameplay_settings_0083b5e0 loads this real tuning block
            // before scene units are created. Do not run on its zero preimage.
            struct Tuning final : bsp::PilotBotSettingsInitHost {
                const GameMissionLuaHost& lua;
                explicit Tuning(const GameMissionLuaHost& source) : lua(source) {}
                const void* game_tuning_get_singleton_0042e740() override {
                    if (!lua.plane_globals_loaded())
                        throw std::logic_error("PilotBotSettings requires loaded PlaneGlobals");
                    return &lua.plane_globals();
                }
            } tuning(host.lua);
            static_assert(offsetof(bsp::GameTuningBlock, pilot_general_cruising_alt) == 0x53c);
            static_assert(offsetof(bsp::GameTuningBlock,
                pilot_general_drop_all_equipment_percent) == 0x5f4);
            bsp::PilotBotSettingsNative defaults;
            bsp::cc15_seg48_pilot_bot_settings_init_007f2bd0(&defaults, tuning);
            // The slot fields are the live block used by the existing Lua
            // setters, altitude/speed laws, avoidance and ordnance issue.
            slot->sq_timer_expiry_37c = defaults.countdowns_00[0];
            slot->sq_timer_expiry_380 = defaults.countdowns_00[1];
            slot->sq_timer_expiry_384 = defaults.countdowns_00[2];
            slot->sq_countdown_388 = defaults.countdowns_00[3];
            slot->sq_freeze_38c = defaults.frozen_10[0] != 0;
            slot->sq_freeze_38d = defaults.frozen_10[1] != 0;
            slot->sq_freeze_38e = defaults.frozen_10[2] != 0;
            slot->sq_freeze_38f = defaults.frozen_10[3] != 0;
            slot->sq_drop_all_bomb_percent_390 = defaults.drop_all_equipment_percent_14;
            slot->sq_alt_394 = defaults.travel_altitude_18;
            slot->sq_alt_398 = defaults.attack_altitude_1c;
            slot->sq_alt_39c = defaults.release_altitude_20;
            slot->sq_speed_3a0 = defaults.travel_speed_24;
            slot->sq_avoid_3a4 = defaults.avoidance_mask_28;
            slot->sq_lock_3a9 = defaults.locks_2d[0] != 0;
            slot->sq_lock_3aa = defaults.locks_2d[1] != 0;
            slot->sq_lock_3ab = defaults.locks_2d[2] != 0;
            slot->sq_lock_3ac = defaults.locks_2d[3] != 0;
            slot->sq_lua_alt_3ad = defaults.enable_altitude_randomization_31 == 0;
            slot->set_force_release_3ae(defaults.force_release_32 != 0);
            host.done("PlaneSquadron::pilot_settings_init", 0x007f2bd0u);
        }
        GameUnitRow& row = slot->row;
        row.name = entity.name;
        row.class_name = entity.class_name;
        row.type_symbol = entity.type_symbol;
        row.type_id = entity.type_id;
        row.party = entity.party;
        row.resource_usage_304 = entity.resource_usage_304;
        // unit+58h: 00927050's kind-1 arm stores the bag's Race (0092708F..
        // 0092709C) with no presence test; MCatapult::Fire reads it back.
        row.race_58 = entity.race;
        // unit+530h / +534h as 0095E5B0 reads them (0095E8F2, 0095E912).
        row.launch_class_530 = entity.launch_class_530;
        row.launch_equipment_534 = entity.launch_equipment_534;
        // Milestone 2l: the two strings 004f0520's last step handed 00469610.
        // 00469610 queues the authored command; OFF keeps the direct string copies.
        if (entity.command_present) {
            host.scene_commands->enqueue(*slot, entity);
            host.done("Units::queue_native_scene_command", 0x00469610u);
        }

        // The frame 0046cf40 composed for the gate is the instance's world 4x4;
        // rows 0..2 are the body axes and row 3 the position, which is the same
        // shape the motion path reads at unit+CCh..+108h.
        for (int i = 0; i < 3; ++i) {
            slot->motion.pose_row0[i] = entity.world[i];
            slot->motion.pose_row1[i] = entity.world[4 + i];
            slot->motion.pose_row2[i] = entity.world[8 + i];
            slot->motion.position[i] = entity.world[12 + i];
            row.start[i] = slot->motion.position[i];
            row.position[i] = slot->motion.position[i];
        }
        row.heading_degrees = heading_degrees_of(slot->motion);

        // The class descriptor. 00964790, the factory that would build one, is
        // a milestone 2h record; what this reads is the installed
        // `VehicleClass` global the recovered global-script step 00886900
        // loaded, indexed by the `Type = E ShipClasses : <symbol>` id the enum
        // library resolved.
        const GameVehicleClassRow lua_row = host.lua.read_vehicle_class_row(row.type_id);
        row.class_row_found = lua_row.found;
        row.class_row_name = lua_row.name;
        const bsp::VehicleClassDescriptorRow* kind = lua_row.found
            ? bsp::vehicle_class_kind_row(lua_row.type.c_str()) : nullptr;
        // 004F0FB0's stationary arm. The native reaches its own factory for
        // these: 00851CB0 fetches the globals, takes `StationaryClass` by name
        // and then the row by the type's own text, so the class lives in a
        // different table from `VehicleClass` and carries no `Type` literal for
        // 00964790's chain to match. A unit whose type resolves there is a
        // stationary prop, and it has no descriptor by construction rather than
        // by a gap in this process. docs/SCENE_STATIONARY_UNITS.md.
        if (kind == nullptr && host.lua.stationary_class_exists(row.type_symbol)) {
            slot->stationary_prop = true;
            // The same StationaryClass factory takes 00748C40 -> 00748A40;
            // bsp.py show 00748a40 --asm: 00748A91 stores 36h at unit+C4.
            // These props have no VehicleClass descriptor to publish an id
            // later. Keep the constructor's actual class stamp for all
            // existing kind-query consumers, not only the new native arm.
            slot->class_id = 0x36;
        }
        slot->motion_dispatch = unit_motion_dispatch(kind);
        if (slot->motion_dispatch.entry == 0x007ce040u) {
            // 007C6340's fall-through at 007C6481 sets unit+900h = 7 and
            // unit+908h = 3600 together, and one call satisfies both of the
            // free-flight arm's requirements. 007CFD20 leaves +900h at 0, which
            // selects no arm at all - which is why a freshly created plane is
            // correctly motionless until something seeds it.
            // docs/PLANE_FLIGHT_CORE_LAW.md.
            slot->plane_control_mode_900 = 7;
            slot->plane_airborne_908 = 3600.0f;
            if constexpr (bsp::kPlaneShipyardLaunchAa0Cc13Bound) {
                // Packet cc13-takeoff. 007D5D20: +AA0h = 0.0 (007D614B); a kind-1
                // holder whose bag has `ShipYardLaunch` (00D0659C, 008F2260 at
                // 007D644B) stores 5.0 (00CE3850) when its +0Ch byte is set, else
                // -1.0 (00D7A260) (007D6454..007D646C). LABELLED: the record keeps
                // only the set bit, so a present-but-clear key stays 0.0; every
                // reader tests > 0.0 or <= 0.0, where 0.0 and -1.0 agree.
                if (entity.shipyard_launch) {
                    slot->plane_shipyard_launch_aa0 = 5.0f;
                    ++host.shipyard_launch_aa0_planes;
                }
            }
            // 007D1F70's rate and acceleration keys for this row. They are what
            // the control targets and the rate law are built from, and they are
            // per-class rather than global, so a Zero and a Dauntless turn at
            // different speeds. Zero on a row that does not carry them, which
            // is the right answer for a ship.
            slot->plane_class.roll_spd = lua_row.roll_spd;
            slot->plane_class.pitch_spd = lua_row.pitch_spd;
            slot->plane_class.yaw_spd = lua_row.yaw_spd;
            slot->plane_class.yaw_roll_ratio = lua_row.yaw_roll_ratio;
            slot->plane_class.slide_ratio = lua_row.slide_ratio;
            slot->plane_class.roll_accel = lua_row.roll_accel;
            slot->plane_class.pitch_accel = lua_row.pitch_accel;
            slot->plane_class.yaw_accel = lua_row.yaw_accel;
            slot->plane_class.negative_pitch_ratio = lua_row.negative_pitch_ratio;
            slot->plane_class_turn_roll_spd = lua_row.turn_roll_spd;
            slot->plane_class_turn_roll = lua_row.turn_roll;
            slot->plane_class_turn_roll_leader = lua_row.turn_roll_leader;
            if (lua_row.plane_stall_spd > 0.0f) {
                // desc+184h. PlaneFreeFlightClass keeps 17.5f as its fallback;
                // an authored row wins.
                slot->plane_stall_spd = lua_row.plane_stall_spd;
            }
            slot->plane_bomb_control_limit_15c = host.lua.read_vehicle_class_number(
                row.type_id, "BombControlLimit", 0.0f);
            // desc+174h / desc+170h. Zero on a ship row, which is what a ship
            // should read; 7.0 on this installation's TBD Devastator row.
            slot->plane_x_drag = lua_row.x_drag;
            slot->plane_y_drag = lua_row.y_drag;
            slot->plane_max_spd = lua_row.max_spd;
            slot->plane_pitch_spd = lua_row.pitch_spd;
            slot->plane_travel_speed = lua_row.travel_speed;
            if constexpr (Impl::kPilotFiresBound) {
                // 007CD930 (from BSP_Plane_ReadPropertyBag): unit+C24h = any gun
                // whose platform ([class+94h][gun+38Ch]) has PilotFires set. The
                // authored table is VehicleClass[id].Platforms[k].PilotFires with
                // the platform's guns in .Gun; flattened once per class.
                host.ensure_pilot_fires_flattened();
                // The projection writes exactly 0/1 for an actual class
                // row. -1 only detects a missing row/failed Lua chunk and
                // is never passed to 007EDAC0. Preserve the old true
                // fallback for existing users of the retained flag.
                const float pilot_fires = host.lua.read_vehicle_class_number(
                    row.type_id, "BSPPilotFires", -1.0f);
                slot->plane_pilot_fires_c24_authored =
                    pilot_fires == 0.0f || pilot_fires == 1.0f;
                slot->plane_pilot_fires_c24 =
                    !slot->plane_pilot_fires_c24_authored || pilot_fires != 0.0f;
            }
            slot->plane_bomb_delay_1f4 = host.lua.read_vehicle_class_number(row.type_id, "BombDelay", 1.0f);   // class+1F4h BombDelay, 007D2318
            if constexpr (Impl::kKamikazeBoostTurboBound) {
                // 007D3CB6-007D3D50: TurboTime (FLDZ), TurboRechargingTime (FLDZ),
                // TurboStrength (FLD1); 007D3DA4-007D3DB8 zeroes TurboTime unless
                // TurboStrength > 1.0f. TurboControlLimit (+608h) is read by
                // 007C0F40's turbo arm (007C0FCF-007C0FE4, bomb_load_factor).
                slot->plane_turbo_control_limit =
                    host.lua.read_vehicle_class_number(row.type_id, "TurboControlLimit", 1.0f);
                slot->plane_turbo_time =
                    host.lua.read_vehicle_class_number(row.type_id, "TurboTime", 0.0f);
                slot->plane_turbo_recharging_time =
                    host.lua.read_vehicle_class_number(row.type_id, "TurboRechargingTime", 0.0f);
                slot->plane_turbo_strength =
                    host.lua.read_vehicle_class_number(row.type_id, "TurboStrength", 1.0f);
                if (!bsp::plane_class_turbo_enabled_007d3da4(slot->plane_turbo_strength)) {
                    slot->plane_turbo_time = 0.0f;
                }
                // 007D5D59-007D5D76 (BSP_Plane_ReadPropertyBag): +E84h = 1.0f when
                // TurboTime > 0 (COMISS / JBE), else 0.0f; +E88h = 0.
                slot->plane_boost_charge_e84 = slot->plane_turbo_time > 0.0f ? 1.0f : 0.0f;
                slot->plane_boost_lockout_e88 = false;
            }
            slot->plane_accel = lua_row.accel;
            // Packet cc9_dive_flight_response: the class reader scales Accel in
            // place (007D20F3-007D2127): when tuning+31Ch AccelCheatMul > 1.0,
            // class+164h = tuning+320h AccelCheatMulMul * AccelCheatMul * Accel,
            // before 007C4850 derives the drag coefficient +50Ch = +164h / MaxSpd^2
            // (007C4984-007C499C). This installation: 1.15 * 1.5 = 1.725. The host
            // kept the raw row value for thrust, drag and the climb angle.
            // docs/DIVE_FLIGHT_RESPONSE.md.
            if (kPlaneAccelCheatScaleBound && host.lua.plane_globals_loaded()) {
                const bsp::GameTuningBlock& g = host.lua.plane_globals();
                slot->plane_accel = bsp::plane_class_accel_007d20f3(
                    lua_row.accel, g.dynamics_accel_cheat_mul,
                    g.dynamics_accel_cheat_mul_mul).accel;
            }
            slot->plane_glide_rate = lua_row.glide_rate;
            slot->plane_drag_pitch_ratio = lua_row.drag_pitch_ratio;
            slot->plane_air_brake_drag = lua_row.air_brake_drag;
            slot->plane_drop_angle = lua_row.drop_angle;
            // Packet cc8_dive_race: desc+268h, read by the dive-bomb approach
            // constructor. The loader already parsed it (plane_class_fields.cpp
            // "TurnCircleRadius"); only the copy onto the slot was missing.
            slot->plane_turn_circle_radius = lua_row.turn_circle_radius;
            slot->plane_swim_height = lua_row.swim_height;
            // Packet cc9_plane_touchdown. desc+198h, and desc+1FCh WheelHeight:
            // 007D29B8-007D2AC6 write it only when WheelHeight and GroundPitch are
            // both present; otherwise the descriptor keeps the zero of
            // BSP_Memory_AllocZeroed (00470B80). This installation authors neither
            // for the P-40 or the P-38.
            slot->plane_min_water_spd_198 = lua_row.min_water_spd;
            slot->plane_max_water_spd_19c = lua_row.max_water_spd;   // goal 5 chain 645
            if constexpr (Impl::kPlaneTouchdownBound) {
                constexpr float kAbsent = -1.0e30f;
                const float wh = host.lua.read_vehicle_class_number(row.type_id, "WheelHeight", kAbsent);
                const float gp = host.lua.read_vehicle_class_number(row.type_id, "GroundPitch", kAbsent);
                slot->plane_wheel_height_1fc = (wh != kAbsent && gp != kAbsent) ? wh : 0.0f;
                slot->plane_ground_pitch_200 = (wh != kAbsent && gp != kAbsent) ? gp : 0.0f;
                // classDesc+1E0h WheelBrake, read by 007D22A6 (docs/PLANE_CLASS_FIELDS.md).
                slot->plane_wheel_brake_1e0 =
                    host.lua.read_vehicle_class_number(row.type_id, "WheelBrake", 0.0f);
            }
            // 007C4BC5-007C4C14. The probe speed of the first call is
            // tuning+24Ch LevelFlight times desc+184h StallSpd, which is the
            // same product the lift term caps its q at, and 007C4C0E scales the
            // answer by the double 0.6 at 00CEFF98 into desc+1ECh.
            {
                bsp::PlaneClimbAngleInputs cai;
                cai.accel = slot->plane_accel;
                cai.max_spd = slot->plane_max_spd;
                float level_flight = 1.8f;      // tuning+24Ch
                float accel_cheat_mul = 1.5f;   // tuning+31Ch
                if (host.lua.plane_globals_loaded()) {
                    const bsp::GameTuningBlock& g = host.lua.plane_globals();
                    level_flight = g.dynamics_spd_multipliers_level_flight;
                    accel_cheat_mul = g.dynamics_accel_cheat_mul;
                }
                cai.accel_cheat_mul = accel_cheat_mul;
                cai.probe_speed = level_flight * slot->plane_stall_spd;
                // Goal 4 chain 279: 007D98F0's predicate is 007D9360 itself, over
                // this slot's class fields and the loaded PlaneGlobals (the struct
                // defaults are the same fallbacks the flight law uses when the data
                // file did not load).
                bsp::gameplay_seg48_cc13::ClimbTrialSetup trial;
                trial.cls.speed_ref_184 = slot->plane_stall_spd;   // desc+184h StallSpd
                trial.cls.thrust_164 = slot->plane_accel;          // desc+164h Accel
                trial.cls.aero_x_174 = slot->plane_x_drag;         // desc+174h XDrag
                trial.cls.aero_y_170 = slot->plane_y_drag;         // desc+170h YDrag
                trial.drag.glide_rate_208 = slot->plane_glide_rate;
                trial.drag.drag_pitch_ratio_1d4 = slot->plane_drag_pitch_ratio;
                trial.drag.air_brake_drag_1dc = slot->plane_air_brake_drag;
                trial.drag.max_spd_188 = slot->plane_max_spd;
                // desc+50Ch, 007C4990-007C499C: (Accel / MaxSpd) / MaxSpd.
                trial.drag.coefficient_50c =
                    (slot->plane_accel / slot->plane_max_spd) / slot->plane_max_spd;
                trial.globals.level_flight_f8732c = level_flight;
                trial.globals.accel_cheat_mul_f873fc = accel_cheat_mul;
                trial.globals.climb_thrust_k_f87404 = 2.6f;       // tuning+324h
                trial.globals.climb_blend_x0_f87408 = 0.174533f;  // tuning+328h
                trial.globals.climb_blend_x1_f8740c = 1.047198f;  // tuning+32Ch
                if (host.lua.plane_globals_loaded()) {
                    const bsp::GameTuningBlock& g = host.lua.plane_globals();
                    trial.globals.climb_thrust_k_f87404 = g.dynamics_accel_cheat_fall_mul;
                    trial.globals.climb_blend_x0_f87408 =
                        g.dynamics_accel_cheat_fall_pitch_range_1;
                    trial.globals.climb_blend_x1_f8740c =
                        g.dynamics_accel_cheat_fall_pitch_range_2;
                    trial.drag_tuning.max_drag_spd_mul = g.dynamics_max_drag_spd_mul;
                    trial.drag_tuning.min_drag_spd_mul = g.dynamics_min_drag_spd_mul;
                    trial.drag_tuning.max_drag_pitch = g.dynamics_max_drag_pitch;
                    trial.aero.drag_func_power = g.dynamics_drag_func_power;
                    trial.aero.drag_range_min = g.dynamics_spd_multipliers_drag_range_min;
                    trial.aero.drag_range_max = g.dynamics_spd_multipliers_drag_range_max;
                }
                cai.trial_007d9360 = &bsp::gameplay_seg48_cc13::climb_trial_speed_left;
                cai.trial_context = &trial;
                slot->plane_climb_angle_1e4 =
                    bsp::max_sustainable_climb_angle_007d98f0(cai);
                slot->plane_climb_angle_1ec = slot->plane_climb_angle_1e4 * 0.6f;
            }
            // The spawn airspeed. docs/PLANE_FLIGHT_CORE_LAW.md measures the
            // seed at 141.67 m/s, and the acceptance test in tests/math_tests.cpp
            // pins lift against gravity at exactly that value and at
            // 1.8 * StallSpd.
            //
            // It is an AIRSPEED, so it goes along the plane's own forward axis,
            // not along a world axis. That axis is pose_row2 - the frame
            // 0046cf40 composed from the authored placement, which the loop
            // above already copied in, and which 00521374's camera path reads
            // the same way (src/system_camera_axes.cpp:387 takes world[8..10]
            // as forward). An earlier revision seeded world +Z instead, on the
            // unchecked assumption that a placement carries no rotation; that
            // flew every plane in the same arbitrary direction.
            {
                const float* const fwd = slot->motion.pose_row2;
                const float len = std::sqrt(fwd[0] * fwd[0] + fwd[1] * fwd[1] +
                                            fwd[2] * fwd[2]);
                // The magnitude is desc+18Ch TravelSpeed, which
                // docs/PLANE_FLIGHT_CORE_LAW.md already named as the field
                // 007C6340 seeds from - it was just stood in for by one row's
                // value. 141.666672 is the TravelSpeed of four rows in this
                // installation's vehicleclasses.lua; the torpedo bombers
                // author 61.111111 (TBD Devastator and TBF Avenger alike), and
                // running them at 141.67 is what put an air-dropped torpedo
                // into the water above MaxWaterHitVel no matter how low it was
                // released. The constant stays as the fallback for a row that
                // carries no TravelSpeed, which is every ship row.
                // docs/TORPEDO_RUN_IN_VELOCITY.md.
                const float seed_speed = slot->plane_travel_speed > 0.0f
                    ? slot->plane_travel_speed : 141.666672f;
                if (len > 1e-6f) {
                    for (int i = 0; i < 3; ++i) {
                        slot->plane_world_velocity[i] = seed_speed * fwd[i] / len;
                    }
                } else {
                    // A degenerate authored frame keeps the old world-+Z seed
                    // rather than propagating a NaN through every lift term.
                    slot->plane_world_velocity[2] = seed_speed;
                }
            }
            // The same mirror as the integration step: the body velocity field
            // 0092D730 reads has to hold the seed too, or the first frames of a
            // plane's life answer 0.0 m/s. docs/TORPEDO_RELEASE_GEOMETRY.md.
            slot->motion.linear_velocity = bsp::OceanVec3{
                slot->plane_world_velocity[0], slot->plane_world_velocity[1],
                slot->plane_world_velocity[2]};
            slot->plane_velocity_seeded = true;
            host.build_plane_engine_list(*slot);   // 007D6327
            host.log.notef("plane spawn: unit=%s heading=%.2f deg forward=(%.4f %.4f %.4f)"
                " seed=(%.2f %.2f %.2f)", row.name.c_str(),
                static_cast<double>(row.heading_degrees),
                static_cast<double>(slot->motion.pose_row2[0]),
                static_cast<double>(slot->motion.pose_row2[1]),
                static_cast<double>(slot->motion.pose_row2[2]),
                static_cast<double>(slot->plane_world_velocity[0]),
                static_cast<double>(slot->plane_world_velocity[1]),
                static_cast<double>(slot->plane_world_velocity[2]));
        }
        bsp::publish_game_entity_observer_tables_00928662(slot->observer_prefix);
        slot->observer_prefix_ready = bsp::publish_unit_leaf_observer_tables_for_creator(
            slot->observer_prefix, slot->motion_dispatch.creator);
        if (!slot->observer_prefix_ready && slot->stationary_prop) {
            // The prop's own pair, from its constructor rather than from a
            // creator row, because it has no descriptor to be keyed by.
            publish_stationary_prop_observer_tables_00748a40(slot->observer_prefix);
            slot->observer_prefix_ready = true;
        }
        if (host.observer_runtime != nullptr) {
            if (!host.observer_runtime->has_live_dispatch_owner())
                throw std::logic_error("unit creation requires the live bound observer owner");
            if (!slot->observer_prefix_ready) {
                // The message used to name neither the unit nor the creator,
                // which cost a whole survey a wrong cause: a zero creator and an
                // unmapped one are different failures and read identically.
                // Zero means no vehicle-class descriptor was found at all, which
                // is what a scene row typed from `StationaryTypes` produces: the
                // 220 rows of this installation's stationaryclasses.lua are
                // `StationaryClass`, not `VehicleClass`, and not one of them
                // carries the `Type` literal that 00964790's chain compares, so
                // the factory returns a null descriptor and there is nothing to
                // map. A non-zero creator means the class resolved but is absent
                // from the 22 rows in src/native_unit_observer_endpoint.cpp.
                // docs/SCENE_STATIONARY_REGISTRATION.md.
                char detail[192];
                std::snprintf(detail, sizeof(detail),
                    "unit observer creator projection is unavailable: unit=%s creator=%08lx (%s)",
                    row.name.c_str(),
                    static_cast<unsigned long>(slot->motion_dispatch.creator),
                    slot->motion_dispatch.creator == 0
                        ? "no vehicle-class descriptor; a StationaryClass row carries no Type"
                        : "resolved class with no observer table row");
                host.log.notef("%s", detail);
                throw std::logic_error(detail);
            }
        }
        host.log.notef("unit motion dispatch: unit=%s creator=%08lx tick_vtable=%08lx "
            "entry=%08lx coverage=%s", row.name.c_str(),
            static_cast<unsigned long>(slot->motion_dispatch.creator),
            static_cast<unsigned long>(slot->motion_dispatch.tick_vtable),
            static_cast<unsigned long>(slot->motion_dispatch.entry),
            unit_motion_coverage_name(slot->motion_dispatch.coverage));
        if (lua_row.found) {
            ++host.summary.class_rows;
            row.max_speed = lua_row.max_speed;
            row.max_rot_angle = lua_row.max_rot_angle;
            slot->motion.max_speed = lua_row.max_speed;   // 00822c20 seeds unit+9C0h
            slot->motion_class.max_accel = lua_row.max_accel;
            slot->motion_class.retardation = lua_row.retardation;
            slot->fields.max_speed = lua_row.max_speed;
            slot->fields.max_rot_angle = lua_row.max_rot_angle;
            slot->fields.max_rot_angle_change_ratio = lua_row.max_rot_angle_change_ratio;
            // Packet cc9_unit_class_lands_troops. class+78Ch holds the resolved
            // descriptor in the image; the host keeps the id when it resolves.
            slot->fields.landing_ship_class =
                lua_row.landing_ship_resolves ? lua_row.landing_ship_id : 0;
            slot->fields.landing_ship_amount = lua_row.landing_ship_amount;
            slot->landing_ship_is_rocketer_0809 = lua_row.landing_ship_is_rocketer;
            // 00964790 maps VehicleClass.Type to a descriptor whose +28h
            // allocator constructs the instance. Its constructor stamps +C4h:
            // e.g. 006FE590 -> 006FE460, store 7 at 006FE4B3. The recovered
            // descriptor kind and instance class id coincide for these leaves.
            if (kind != nullptr) slot->class_id = static_cast<int>(kind->kind);
        }
        if (slot->class_id == bsp::kVehicleClassKindUnknown) {
            host.log.notef("unit identity unresolved: unit=%s type_id=%d class_row_found=%d "
                "VehicleClass.Type=\"%s\"; class_id=-1, kind queries return false",
                row.name.c_str(), row.type_id, row.class_row_found ? 1 : 0,
                lua_row.type.c_str());
        }
        // Milestone 2r. class+A0h, class+A8h and class+B0h are the `Length`,
        // `Height` and `Mass` keys 00960230 writes into the descriptor, and the
        // installed `VehicleClass` row carries all three (DeRuyter row 20:
        // 171, 5, 7688). Milestone 2q left them at zero because the keel gate
        // passes either way on a flat sea; they are read now because
        // 00937C90's hull body needs the mass and 00937440 needs it squared,
        // and because 00826866 places the keel sample point half a hull length
        // astern and half a hull height below the pose. A missing key leaves
        // the zero the reader stores, except `Mass`, whose reader default is
        // the 1.0f at 0096043A.
        slot->motion_class.hull_length = lua_row.length;   // class+A0h
        slot->motion_class.hull_height = lua_row.height;   // class+A8h
        slot->class_width_00a4 = lua_row.width;            // class+A4h
        //00810FAF..00811073: this represented class has no model resource.
        // Copy its actual Lua Width/Length through the recovered no-model arm;
        // a missing class leaves the slot's prior extent fields untouched.
        const bsp::UnitHullExtentClassInputs extent_class{
            nullptr, lua_row.length, lua_row.width};
        bsp::produce_unit_hull_extents_00810faf(slot->hull_extents,
            lua_row.found ? &extent_class : nullptr);
        constexpr float kClassMassReaderDefault = 1.0f;  // 0096043A
        slot->motion_class.hull_mass
            = lua_row.mass > 0.0f ? lua_row.mass : kClassMassReaderDefault;
        slot->motion_class.boost_refill_time = 1.0f;
        slot->motion.thrust_mod = 1.0f;        // 00823714
        slot->motion.turn_efficiency = 1.0f;   // 0082371c
        slot->motion.class_id = slot->class_id;

        bsp::construct_unit_order_ring_00812d40(slot->ring);

        // Packet cc8_ship_station, edited under the integrator's hunk
        // arbitration of 2026-09-19. 00822C20's FIRST reconstructed step, before
        // the property-bag arm below: 00823508 calls 00818EA0 in straight-line
        // code, and 00818EA0's tail 00819367..00819381 fills the pose-history
        // ring at entity+0BD0h from the entity's cached world position
        // (&entity+0FCh) and the float its vtable slot +50h returns. That is what
        // gives a ship a trail before it has moved: forty samples 50 m apart
        // running 1950 m astern of the spawn heading. Without it a follower of a
        // leader that never moves measures its station along a ring of zeros and
        // steams for the world origin, which is what USN01 did
        // (docs/SHIP_UNIT_GROUP_FOLLOW.md section 5e).
        //
        // The heading is the same expression the motion tail uses for the append
        // at 00826CEE - an atan2 over world row 2, unit+1050h - because slot +50h
        // is that field's getter (docs/SHIP_AI_RUDDER_HOP.md).
        //
        // Not ship-gated: 00818EA0 is reached from the shared game-unit
        // SEntityInit, so every unit this host creates fills its ring.
        {
            const float spawn_heading = static_cast<float>(
                std::atan2(static_cast<double>(slot->motion.pose_row2[0]),
                           static_cast<double>(slot->motion.pose_row2[2])));
            bsp::ship_ai_wake_fill_00810020(slot->wake, slot->motion.position,
                spawn_heading);
            host.done("UnitPoseHistoryRing::fill_at_spawn", 0x00810020u);
        }

        // Packet cc9_units_capture_accessors: 006F2780's CaptureRange, unit+7A0h.
        slot->capture_range_7a0 = entity.capture_range_present ? entity.capture_range_raw : 500;
        // Packet cc9_command_building_capture_bind: 006F2780's CaptureValue, unit+7A4h.
        slot->capture_value_7a4 = entity.capture_value_present ? entity.capture_value_raw : 1000;
        // Packet cc9_command_building_level: 006F2780's Level and LevelUpSeconds.
        slot->level_770 = entity.level_present ? entity.level_raw : 0;
        slot->level_up_seconds_76c = entity.level_up_seconds_present
            ? entity.level_up_seconds_raw : 10;
        // 006F2780's LandingRange, unit+7C4h (006F285F).
        slot->landing_range_7c4 = entity.landing_range_present ? entity.landing_range_raw : 500;
        // Packet cc9_building_pad_model: 006F2780's LandingPointRange, unit+7CCh (006F28B3).
        slot->landing_point_range_7cc = entity.landing_point_range_present
            ? entity.landing_point_range_raw : 500;
        // Packet cc9_airfield_destruction_rule: 006F2780's InferiorRange, unit+7C8h
        // (006F288C), and 009554C1's MinLevel find.
        slot->inferior_range_7c8 = entity.inferior_range_present ? entity.inferior_range_raw : 200;
        slot->bag_min_level_724 = entity.bag_min_level;
        slot->min_level_724 = entity.min_level_present ? entity.min_level : 0;

        // Milestone 2q: 00926110, BSP_SEntity_InitAll's call of the entity's
        // vtable slot 0A0h, which for this class family is 00822C20. Only that
        // routine's property-bag arm 0082356C..008235FB is run here, and only
        // its kind-1 branch: 0046d5b0 built the holder over a cloned scene
        // property bag, so 00823537's tag is 1 and 0082353D takes the arm.
        // The seed has to happen here, before issue_authored_commands latches
        // the scene's queued `Cruise` at 00835E17, because the latch captures
        // the ring's live throttle (docs/CRUISE_SPEED_SETTING.md).
        if (slot->motion_dispatch.runs_ship_base()) {
            StartSpeedSeedBinding seed_host(host, *slot, entity);
            const bsp::SceneStartSpeedSeed seed = bsp::run_start_speed_arm_0082356c(
                seed_host, static_cast<std::uint32_t>(host.slots.size()) + 1u,
                static_cast<std::uint32_t>(host.slots.size()) + 1u,
                bsp::kSceneEntityBagRefKindPropertyBag);
            host.done("SceneStartSpeed::init_slot_00a0", 0x00926110u);
            row.start_speed_authored = seed.authored;
            row.start_speed = seed.start_speed;
            row.start_speed_ratio = seed.ring_throttle;
            row.start_speed_axial = seed.axial_speed;
            if (seed.authored) {
                ++host.summary.start_speed_seeds;
                // ring.current_param_a and motion.throttle are two projections
                // of the one native field unit+980h, which 0080D9E8 has just
                // written; the ring tick copies it every step, so this only
                // keeps the two in step before the first tick runs.
                slot->motion.throttle = slot->ring.current_param_a;
            }
        }

        // Packet cc9_submarine_depth_level: 00853630, the submarine's slot
        // 0A0h override, picks unit+1268h after 00822C20. Stage 1: 1 for a
        // kamikaze class, else 0 (the class read is packet cc13-subdive's
        // kCc13SubDiveKamikazeBound; OFF it reads false, LABELLED).
        // Stage 2, a kind-1 holder (every host unit is scene-sourced): `Dive`
        // then `TargetDive`, each only when found. The hull move to
        // bands[level] that `Dive` also makes (00853B44..00853B84) is
        // kSubmarineDiveTeleportBound below.
        if (bsp::unit_is_kind_of(slot->class_id, 0x08)) {
            row.submarine_depth_seeded = true;
            // Packet cc9_submarine_dive: the band table (00853A90..00853AC5), the
            // rate keys, and the one-shot +126Ch (00853A2A, with +1270h = 0).
            {
                bsp::SubmarineClassDepths keys;
                keys.periscope_depth = lua_row.sub_periscope_depth;
                keys.swim_depth_2 = lua_row.sub_swim_depth2;
                keys.swim_depth_3 = lua_row.sub_swim_depth3;
                const auto bands = bsp::submarine_build_depth_bands_00853630(keys);
                for (int i = 0; i < 4; ++i) slot->sub_bands[i] = bands[static_cast<std::size_t>(i)];
                slot->sub_up_speed = lua_row.sub_up_speed;
                slot->sub_down_speed = lua_row.sub_down_speed;
                slot->sub_stop_time = lua_row.sub_up_down_stop_time;
                slot->sub_accel = lua_row.sub_up_down_accel;
                slot->sub_air_rates.run_out_time = lua_row.sub_air_run_out_time;
                slot->sub_air_rates.reload_time = lua_row.sub_air_reload_time;
                // 00853AD5..00853AE5: +123Ch and +1248h from bands[3], +1244h zero.
                slot->sub_scan.clearance = bands[3];
                slot->sub_scan.clearance_acc = bands[3];
                slot->sub_class_width = lua_row.width;
                slot->sub_class_length = lua_row.length;
                slot->sub_class_height = lua_row.height;
                slot->sub_class_max_speed = lua_row.max_speed;
                slot->sub_seed_126c = true;
                slot->sub_sink_1270 = 0.0f;
                slot->sub_dive_ready = true;
                host.log.notef("submarine dive bands: unit=%s bands=(%.1f %.1f %.1f %.1f) "
                    "up=%.2f down=%.2f stop=%.2f accel=%.2f air=%.0f/%.0f bound=%d (packet cc9_submarine_dive)",
                    row.name.c_str(), static_cast<double>(bands[0]),
                    static_cast<double>(bands[1]), static_cast<double>(bands[2]),
                    static_cast<double>(bands[3]), static_cast<double>(slot->sub_up_speed),
                    static_cast<double>(slot->sub_down_speed),
                    static_cast<double>(slot->sub_stop_time),
                    static_cast<double>(slot->sub_accel),
                    static_cast<double>(slot->sub_air_rates.run_out_time),
                    static_cast<double>(slot->sub_air_rates.reload_time),
                    kSubmarineDiveBound ? 1 : 0);
            }
            // Stage 1 (00853A31..00853A81): +1268h = 1 for a kamikaze class, else 0;
            // two JA tests after COMISS with 0.0, so NaN reads false. Packet
            // cc13-subdive: the class read runs under kCc13SubDiveKamikazeBound;
            // OFF the test reads false (LABELLED).
            if constexpr (bsp::cc13_sub_dive::kCc13SubDiveKamikazeBound) {
                slot->sub_kamikaze_class =
                    host.lua.read_vehicle_class_number(row.type_id, "KamikazeDamage", 0.0f) > 0.0f
                    || host.lua.read_vehicle_class_number(row.type_id, "KamikazeBlastDamage", 0.0f) > 0.0f;
            }
            row.submarine_depth_level = slot->sub_kamikaze_class ? 1 : 0;
            if (entity.dive_present) row.submarine_depth_level = entity.dive_level;
            // Packet cc9_submarine_dive_teleport. 00853B3E stores the level, 00853B44
            // loads bands[level] and 00853B55 writes it to the local Y at +A8h (X and
            // Z rewritten unchanged at 00853B4D/00853B5D); +C8h and +10Ch are cleared
            // and every child is invalidated through 0042ED50. SUBSTITUTION, labelled:
            // local is world here (every host submarine is a top-level entity), and
            // the children have no host pose to invalidate.
            if (kSubmarineDiveTeleportBound && entity.dive_present) {
                const int lvl = entity.dive_level < 0 ? 0 : (entity.dive_level > 3 ? 3 : entity.dive_level);
                const float from = slot->motion.position[1];
                slot->motion.position[1] = slot->sub_bands[lvl];
                row.position[1] = slot->motion.position[1];
                host.log.notef("submarine dive teleport: unit=%s dive=%d y %.2f -> %.2f "
                    "(00853B55, packet cc9_submarine_dive_teleport)", row.name.c_str(),
                    lvl, static_cast<double>(from), static_cast<double>(slot->motion.position[1]));
            }
            if (entity.target_dive_present) {
                row.submarine_depth_level = entity.target_dive_level;
            }
            // Packet cc13-subdive (rev-convoy N1), kSubSeedNearestBandCc13Bound:
            // with neither key found (00853BE2/00853BE6), 00853BEB..00853C70 keep
            // +1268h unless a band 1..3 is nearer the hull's world Y than |Y|;
            // each step is one rounded float op. OFF: the stage is skipped (LABELLED).
            if (bsp::cc13_sub_dive::kSubSeedNearestBandCc13Bound
                    && !entity.dive_present && !entity.target_dive_present) {
                const float y = slot->motion.position[1];
                float best = std::fabs(y);
                for (int i = 1; i < 4; ++i) {
                    const float d = std::fabs(static_cast<float>(y - slot->sub_bands[i]));
                    if (best > d) {
                        best = d;
                        row.submarine_depth_level = i;
                    }
                }
                host.log.notef("submarine seed nearest band: unit=%s y=%.2f level=%d "
                    "(00853BE8..00853C70, packet cc13-subdive)", row.name.c_str(),
                    static_cast<double>(y), static_cast<int>(row.submarine_depth_level));
            }
            host.log.notef("submarine depth seed: unit=%s level=%d dive=%d:%d "
                "target_dive=%d:%d (00853630 stages 1-2, packet "
                "cc9_submarine_depth_level)", row.name.c_str(),
                static_cast<int>(row.submarine_depth_level),
                entity.dive_present ? 1 : 0, static_cast<int>(entity.dive_level),
                entity.target_dive_present ? 1 : 0,
                static_cast<int>(entity.target_dive_level));
        }

        // Milestone 2r: the hull body the game builds, replacing milestone 2h's
        // freshly constructed one and the 1.0e30f speed clamps that stood in
        // for M+18h / M+1Ch. 00939E2A calls 00937C90 last in the controller
        // constructor 00939CB0 and its tail 009399C0..00939C05 is the writer:
        // the descriptor default 009391C2, the mass at 009399F7, the zeroed
        // inertia diagonal, the angular damping 1.0f at 00939A2F, the row-1
        // torque lock when the mass is under 100.0 (00939A57), then
        // 00C5D580 CreateBody and 00C37E70 with the box inertia at 00939C05.
        // The linear damping stays the descriptor's own 0.0f (00939295); it is
        // not a gap, and docs/SHIP_HULL_BODY.md says so.
        //
        // The collision AABB is the one input this cannot supply: its producer
        // is the shape attach 00C5C940 behind 00937D3F..009399BF, which no
        // packet has read, so the span is zero and the box inertia with it,
        // which 00C37E70 turns into a zero inverse inertia. Nothing here
        // applies a torque, so that decides nothing this run measures; it is
        // the same default the probe takes.
        if (slot->motion_dispatch.runs_ship_base()) {
            bsp::ShipHullBodyInputs hull{};
            hull.mass = slot->motion_class.hull_mass;             // 009399F7
            // 00937CFD calls virtual slot 5Ch with category 8 (MSubmarine);
            // the accepting branch picks the third physics record at 00937D09.
            hull.unit_category_8 = bsp::unit_is_kind_of(
                slot->class_id, bsp::kUnitForceSubmarineClassId);
            for (int i = 0; i < 3; ++i) {
                hull.row0[i] = slot->motion.pose_row0[i];
                hull.row1[i] = slot->motion.pose_row1[i];
                hull.row2[i] = slot->motion.pose_row2[i];
                hull.position[i] = slot->motion.position[i];
            }
            if constexpr (Impl::kHullInertiaFromShapesBound) {
                const Impl::ClassHullBox& hb = host.class_hull_box(*slot);
                if (hb.ok && hb.box.shape_count > 0) {
                    hull.aabb_min = hb.box.min;
                    hull.aabb_max = hb.box.max;
                    hull.shape_count = hb.box.shape_count;
                }
            }
            bsp::ship_hull_body_create_00937c90(hull, slot->body, slot->motion_state);
            slot->hull_material = bsp::ship_hull_material_00937cf1(hull.unit_category_8,
                hull.mass);
            slot->body.motion = &slot->motion_state;
        }

        // Milestone 2s: the buoyancy element list at class+52Ch..+530h, which
        // 009329C0 walks. This is a STAND-IN and the doc says so: the producer
        // writes class+528h..+534h and no function in the exported set does,
        // so docs/SHIP_HYDRO_FORCES.md carries the four field roles as a
        // hypothesis reconciled between its only two readers and leaves the
        // writer to packet `ship_buoyancy_element_producer`.
        //
        // The list built here is the one bsp_ship_motion_probe.exe --hydro
        // builds, so the two sides of the comparison are the same list: the
        // elements are spread evenly along the class `Length` in the hull's own
        // Y = 0 plane, the draft is half the class `Height`, and the shared
        // coefficient is solved so that a hull floating at that draft displaces
        // exactly its own weight against the world gravity of 10. A class row
        // that carries no Length or Height produces no list at all, and the
        // hydrodynamic step is then skipped rather than run on invented data.
        // Packet cc9_buoyancy_elements: the image's own list (0082D040 through
        // 0082FE30's window 0082FEA9..0082FEE8), built once per class and read
        // in both builds so the census can compare it; used only ON.
        const Impl::ClassBuoyancyList* image_list = nullptr;
        if (slot->motion_dispatch.runs_ship_base()) {
            image_list = &host.class_buoyancy_list_0082fe30(*slot);
        }
        bool used_image_list = false;
        if constexpr (Impl::kShipBuoyancyElementsBound) {
            if (image_list != nullptr && image_list->ok) {
                slot->buoyancy_elements = image_list->elements;
                used_image_list = true;
                ++host.buoyancy_lists_image;
            } else if (image_list != nullptr) {
                ++host.buoyancy_lists_fallbacks;
            }
        }
        if (slot->motion_dispatch.runs_ship_base() && !used_image_list) {
            const float length = slot->motion_class.hull_length;
            const float height = slot->motion_class.hull_height;
            const float mass = slot->motion_class.hull_mass;
            if (length > 0.0f && height > 0.0f && mass > 0.0f) {
                ++host.buoyancy_lists_stand_in;
                const float draft = height * 0.5f;
                // ship_hydro_buoyancy_00932e44 at depth == draft, with the
                // surface mix of 0.5 the 00CE3800 float supplies.
                const float shape
                    = bsp::kShipHydroBuoyancyShapeMix * (draft / height)
                        + (1.0f - bsp::kShipHydroBuoyancyShapeMix);
                const float count = static_cast<float>(kBuoyancyElementCount);
                const float per_element = (mass * bsp::kShipHydroDragGravity) / count;
                const float coefficient = per_element / (draft * shape);
                slot->buoyancy_elements.reserve(kBuoyancyElementCount);
                for (int i = 0; i < kBuoyancyElementCount; ++i) {
                    bsp::ShipBuoyancyElement element{};
                    element.coefficient = coefficient;
                    element.level_base = 0.0f;
                    element.level_draft = draft;
                    element.level_top = height;
                    element.position.x = 0.0f;
                    element.position.y = 0.0f;
                    element.position.z = length
                        * ((static_cast<float>(i) + 0.5f) / count - 0.5f);
                    slot->buoyancy_elements.push_back(element);
                }
            }
        }

        if constexpr (Impl::kShipSinkDescentBound) {
            if (!slot->buoyancy_elements.empty()) host.build_leak_model_0074f490(*slot);
        }

        slot->parent = nullptr;
        slot->pose = std::make_unique<bsp::PoseRefreshView>(
            bsp::PoseRefreshView{bsp::PoseRefreshParentSlot(slot->parent), slot->local,
                slot->world_valid, slot->world, slot->derived_valid});
        slot->state = std::make_unique<bsp::UnitInstanceState>(
            bsp::UnitInstanceState{*slot->pose});
        slot->state->class_id = slot->class_id;
        // The represented kind1 scene path reaches00923840 through each
        // supported creator's primary+A0 initializer.00923855 writes+5C=1;
        // this is initialization, not the constructor's initial zero.
        //00925E14 clears+5D; type3 deadMeat delivery is not represented here.
        slot->state->active = 1;
        slot->state->simulate = 0;
        slot->scene_flags_available = slot->motion_dispatch.creator != 0;
        // bsp.py show 00923840: the SAME supported kind1 initializer
        // which publishes active above also writes entity+BDh = 1.
        if (slot->scene_flags_available) slot->message_ready_00bd = 1;
        row.active = slot->state->active != 0;
        slot->state->has_scene_node = false;  // +4A4h, 00928860 is a 2h record
        slot->state->part_count = 0;     // +A18h, the instance has no parts here
        // The actual descriptor creator selects the native+130h override;
        //009288F1 is the recovered dispatch site, not a missing caller.
        slot->process_index = host.slots.size();
        //00925906 stores the actual world/list owner at unit+30h. This
        // process already owns that registry; full placement/locking and
        // hierarchy attachment remain separate unreconstructed runtime work.
        slot->world_parent_0030 = &host.world_lists;
        host.register_in_world_lists(*slot);
        Impl::publish_pose(*slot);
        // Packet cc9_scene_unit_skill: each creator hands 00927A80's bag skill to
        // vtable[128h] (00822C20 at 008238C1..008238CB for the kind-1 scene arm,
        // 006D3CF0, 00748383, 007D65AA, 00849D71), which stores unit+390h. This
        // host kept the constructor's 1 (0095CCCC) until a script call. The line
        // prints in both builds for every unit whose bag gives other than 1.
        slot->bag_equipment = entity.bag_equipment;
        if (entity.class_id == 0x18 && entity.bag_equipment >= 0) {
            ++host.plane_equipment_squadrons;
            if (host.plane_equipment_squadrons <= 40) {
                host.log.notef("plane equipment: unit=%zu name=%s type=%d bag=%d "
                               "applied=%d (007CDF20 [unit+C54h], packet "
                               "cc9_plane_scene_equipment)",
                               slot->process_index, entity.name.c_str(), entity.type_id,
                               entity.bag_equipment, bsp::kPlaneSceneEquipmentBound ? 1 : 0);
            }
        }
        if (entity.bag_skill != 1) {
            host.log.notef("scene skill: unit=%zu name=%s class=%s source=%s level=%d "
                           "applied=%d (00927A80, packet cc9_scene_unit_skill)",
                           slot->process_index, entity.name.c_str(),
                           entity.class_name.c_str(), entity.bag_skill_source.c_str(),
                           entity.bag_skill, kSceneUnitSkillBound ? 1 : 0);
        }
        if constexpr (kSceneUnitSkillBound) {
            if (entity.bag_skill >= 0 && entity.bag_skill <= 5) {
                slot->pilot_skill_index = entity.bag_skill;
            }
        }
        host.slots.push_back(std::move(slot));
        if constexpr (game_powerups::kPowerUpSubsystemCc13Bound) {
            // Packet cc13-powerup-bind phase 2a: 006F2780's second call, 008ED720
            // on the command building's binding (unit+758h). SUBSTITUTION,
            // labelled: run once the slot is in the table (the grant reads the
            // building's party and 009FFD20 slot through the resolver), in the
            // same per-entity pass; 00822C20's reference-speed reads of this
            // entity come before it either way.
            if (bsp::unit_is_kind_of(host.slots.back()->class_id, 0x1C)) {
                game_powerups::command_building_powerup_init_008ed720(
                    host.slots.size() - 1, entity.powerup_class_id);
            }
        }
        if constexpr (kSceneHomeBaseContractBound) {
            // Packet cc9_scene_home_base_contract: a squadron built from a
            // PlaneSquadronGen row gets its row's HomeBase before its pass C.
            if (entity.class_id == 0x18 && entity.home_base_carried) {
                set_squadron_scene_home_base(host.slots.size() - 1, entity.home_base);
                ++host.scene_home_contract_calls;
            }
        }
        if constexpr (Impl::kUnitsPendingPushBound) {
            // Packet cc9_units_push_pending: the base constructor 00928630, at 00928760,
            // pushes every entity onto 00F899D0 through 00926BE0. The id is the
            // unit id (index + 1), the class the VehicleClass row. The Lua
            // host's dedup (docs/SENTITY_INIT_ATTACH_ORDER.md section 9) keeps
            // one node per id while the routes and the load walk still push.
            const GameUnitRow& made = host.slots.back()->row;
            host.lua.push_pending_entity_00926be0(static_cast<int>(host.slots.size()),
                made.name, made.type_id);
            ++host.construction_pushes;
            host.done("UnitInstance::construct_push_00926be0", 0x00928760u);
        }
    }
    host.summary.units = host.slots.size();
    // Milestone 2l: the entities 0046aab0's target lookup would find, and the
    // owners its records name. entity+174h is the executable's own id space,
    // because the two handle tables at 00f89a0c / 00f89a60 are not built here.
    std::vector<GameCommandUnit> command_units;
    command_units.reserve(host.slots.size());
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        GameCommandUnit unit;
        unit.index = index;
        unit.name = host.slots[index]->row.name;
        unit.object_id = static_cast<std::uint16_t>(index + 1);
        unit.class_id = host.slots[index]->class_id;   // +C4h, for kSquadronSetCommandBound
        for (int lane = 0; lane < 3; ++lane) {
            unit.position[lane] = host.slots[index]->motion.position[lane];
        }
        command_units.push_back(unit);
    }
    host.commands.register_units(std::move(command_units));
    host.log.notef("world units: %zu created instance(s) carried into the frame, %zu with a "
        "VehicleClass row out of the installed table", host.summary.units,
        host.summary.class_rows);
    // The plane-squadron member write-back, here because the slots now exist
    // and the AI host below reads the array as soon as it is constructed.
    // GameScriptOrdersHost owns this body but the free function needs no
    // scripts-orders host: it takes the process-wide registry and this host's
    // own count()/unit_row(). Without the call, a squadron that came from a
    // SCENE row still had an empty +3D0h array when build_squadrons asked, and
    // USN04 built 7 AI squadrons over 15 member planes where the mission has 5.
    // `only_unresolved` so the wipe inside it can never un-fill the inline
    // answer an air-ops launch already wrote; docs/PLANE_SQUADRON_HOST.md.
    bsp::game::resolve_plane_squadron_members(*this, &host.log, true);
    // Milestone 2t: the unit-side gunnery pass. 00810DD0's creation block puts a
    // 558h-byte object at unit+6DCh and attaches it to the unit's own tick
    // element unit+310h through its vtable +4h (00864BD0), which is why it runs
    // on the fixed step beside the motion pass. The guns themselves come from
    // the authored `VehicleClass[id].Platforms` table, because this process
    // builds no model hierarchy; include/bsp/game_hosts_gunnery.hpp says so.
    // Packet cc8_gunnery_host. This used to construct a FRESH gunnery host on
    // every create_units, which is every spawn batch: USN04 ran create_units 13
    // times in 4800 mission frames and 25 times in 9000 (four SpawnNew batches
    // at t=0, four air-ops launches at t=27..30 s, four more SpawnNew at
    // t=105 s, three single-unit creations at t=345 s and nine more air-ops
    // launches at t=366..381 s). Each one threw away the summary, the hit
    // records, the kill credits and every round, bomb and torpedo in flight.
    // The image creates the per-unit gun object once per unit - 00810DD0's
    // creation block puts the 558h-byte object at unit+6DCh and attaches it
    // through 00864BD0 - and has no per-batch refresh, so the host is the
    // mission's, not the batch's. Same class of artefact as the coordinator
    // below and the weapon directors cc8_ship_drive fixed in register_units.
    //
    // Packet cc8_gunnery_host measured the pair this guard is for, on one build
    // differing only by the guard (`if (true)` reproduces the old path exactly),
    // USN04, same parameters, 4800 mission frames: the summary stops being a
    // since-the-last-batch count and becomes the mission's. `first_hit` goes
    // 13.35 s -> 119.90 s, which is the SAME event relabelled - the last batch
    // built its host at mission 106.55 s, and 13.35 + 106.55 = 119.90.
    // `queued_hits` 151 -> 143 and `total_damage` 14042.2 -> 14607.3
    // (local/gh_off_usn04.log vs local/gh_on_usn04.log); `bomb_drops` 23,
    // `bomb_impacts` 20, `torpedo_drop drops` 12 and `deaths` 14 are unchanged,
    // because USN04 drops its first bomb after the last batch anyway and the
    // deaths that moved cancel.
    //
    // They do move, though, and that is the gameplay half of this. Ignoring the
    // clock relabel, the two runs are identical for 6274 census lines and
    // diverge at mission 224.80 s, where a torpedo that hits Fletcher-class02
    // unguarded swims past it guarded. Four sinkings flip: Lexington-class01
    // (the controlled carrier) took 6291 of 8000 and lived unguarded, takes the
    // full 8000 and SINKS at 230.26 s guarded; movieval dies; Fletcher-class02
    // and movieval|.-3 now survive. The cause is that build_guns ran
    // `state.health = state.max_health` over every unit on every batch, so a
    // unit was fully healed 12 times in this mission. USN01 makes one
    // create_units call and is bit-identical across the pair: 7140 census lines,
    // zero differences (local/gh_off_usn01.log vs local/gh_on_usn01.log).
    if (host.gunnery == nullptr) {
        host.gunnery = std::make_unique<GameGunneryHost>(host.log, *this, host.lua);
        host.gunnery->set_ship_ai(host.ship_ai);
        host.gunnery->attach_00864bd0();
    } else {
        host.gunnery->register_new_units_00864bd0();
    }
    // Packet cc8_ship_follow. This used to rebuild the coordinator on EVERY
    // spawn batch, which on USN04's twelve batches threw away its groups and its
    // counters eleven times: the AI summary reported available=0 refused=238
    // joins=0 while the units host, which survives, reported joins=17 for the
    // same run. The image creates one coordinator for the mission - 00A32350 is
    // a creation, not a per-batch refresh - and this is the same class of host
    // artefact as the director destruction cc8_ship_drive fixed in
    // register_units. New units need no registration here: the coordinator reads
    // the units host live and its next compose pass picks them up.
    // Packet cc8_ship_screen measured the pair this guard is for, on one build
    // differing only by the guard, USN04, same parameters: without it
    // `summary mission ai follow` reads requests=238 available=0 refused=238
    // joins=0 (local/coord_off_usn04.log) because the coordinator is rebuilt on
    // every spawn batch and the counters are the last batch's alone; with it,
    // requests=731 available=1 refused=730 joins=1 (local/screen_usn04.log), and
    // `total_path` goes 26985.10 -> 29455.68. The units host's own
    // `joins=25 clamped=9` is identical either way, which is what made the
    // artefact look harmless.
    if (host.ai == nullptr) {
        host.ai = std::make_unique<GameAiCoordinatorHost>(host.log, *this);
        host.ai->create_00a32350();
    }
}

void GameUnitsHost::issue_authored_commands() {
    Impl& host = *impl_;
    if (!host.logged_cruise) {
        host.logged_cruise = true;
        host.log.notef("milestone 2l: the authored `Command` token is no longer a stand-in "
            "order. docs/CRUISE_COMMAND.md recovered `cruise` as a latch that captures the "
            "ring's ordered pair (unit+980h / unit+984h) and the heading when it becomes "
            "the unit's current command and re-applies what it captured, so a ship whose "
            "ring is still zero holds zero. Milestone 2i wrote throttle 1 / rudder 0 into "
            "the ring for the same token, which is what the latch would produce for a ship "
            "already running at full throttle and is wrong for a ship at rest");
    }
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        issue_authored_command_at(index);
    }
}

bool GameUnitsHost::issue_authored_command_at(std::size_t index) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[index];
    if (host.scene_commands != nullptr) host.scene_commands->take(slot);
    // The token every DestroyerGen of this scene authors. An entity that
    // authored none is skipped, exactly as 004f0520's last step skips the
    // queue call when the property is absent.
    // 004E6B30 queues a present-empty key too; 0046AAB0's empty-name
    // failure still reaches its clear. Retire before the host early exit.
    if (slot.row.command.empty() && host.scene_commands != nullptr) {
        host.scene_commands->retire(slot);
    }
    if (slot.row.command.empty()) return false;
    const float heading = host.pose_heading_radians(slot);
    const GameCommandRow* row = host.commands.issue(index, slot.row.command,
        slot.row.command_target, slot.ring, heading);
    if (host.scene_commands != nullptr) host.scene_commands->retire(slot);
    if (row == nullptr) return false;
    slot.row.command_current = row->current;
    slot.row.command_latched = row->latched;
    slot.row.latch_is_heading = row->fields.is_heading;
    slot.row.latch_steer = row->fields.steer_or_heading;
    slot.row.latch_thrust = row->fields.thrust;
    // Nothing on this path writes an order ring, so no standing order is
    // armed: the ring stays at whatever 00812d40 constructed it with.
    slot.standing_order = false;
    ++host.summary.cruise_orders;
    return true;
}

bool GameUnitsHost::issue_generated_authored_command(std::size_t first_index,
    const std::string& name) {
    Impl& host = *impl_;
    // The slot create_units just made for a GenerateObject spawn: the first one
    // at or past the count before the call that carries the spawned name. A
    // respawn (kGenerateObjectEveryCallBound) makes a new slot under the same
    // name, so the search starts at the new slots rather than at index 0.
    for (std::size_t index = first_index; index < host.slots.size(); ++index) {
        if (host.slots[index]->row.name == name) return issue_authored_command_at(index);
    }
    return false;
}

bool GameUnitsHost::issue_player_command(const std::string& token,
    const std::string& target_token, const std::string& unit_name) {
    Impl& host = *impl_;
    std::size_t index = host.controlled_index;
    if (!unit_name.empty()) {
        index = host.slots.size();
        for (std::size_t candidate = 0; candidate < host.slots.size(); ++candidate) {
            if (host.slots[candidate]->row.name == unit_name) { index = candidate; break; }
        }
        if (index >= host.slots.size()) {
            host.log.notef("--order-unit \"%s\" names no created instance; the command was "
                "not issued", unit_name.c_str());
            return false;
        }
    } else if (!host.controlled_bound) {
        return false;
    }
    if (index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[index];
    const float heading = host.pose_heading_radians(slot);
    // Milestone 2s: a fixed point instead of a named entity. `--order
    // moveto:<x>,<z>` is the same order `NavigatorMoveToPos` issues from a
    // mission script: 0088A810's Vector3 branch builds a descriptor whose
    // position_valid byte is set, whose object is null and whose three floats
    // are the point, and 008A2BC0 hands that and the fixed command object to
    // 0077D600. A named entity has no comma in it on any scene of this game, so
    // a target token that parses as two numbers is the point form.
    //
    // Milestone 2r's section 6 is why this exists: `moveto:<unit>` names a ship
    // whose position the goal vector re-reads every frame, and a goal that
    // outruns the chaser cannot be reached, so the arrival latch could never be
    // exercised whatever the AI did.
    float point[3] = {0.0f, 0.0f, 0.0f};
    if (parse_order_position(target_token, point[0], point[2])) {
        bsp::SceneCommandTarget target{};
        target.kind = 0;              // 0088A8A6 clears the word
        target.position_valid = 1;    // 0088A8A9
        target.object = nullptr;      // 0088A8BE
        target.object_id = 0;
        target.position[0] = point[0];
        target.position[1] = point[1];
        target.position[2] = point[2];
        target.trailing = 0.0f;       // 0088A8C7
        char label[64];
        std::snprintf(label, sizeof(label), "(%.1f, %.1f)",
            static_cast<double>(point[0]), static_cast<double>(point[2]));
        const GameCommandRow* placed = host.commands.issue_command_object(index,
            bsp::kCommandObjectMoveTo, target, bsp::kNavigatorIssueFlags,
            std::string("player:--order"), std::string(label), slot.ring, heading);
        if (placed == nullptr) return false;
        slot.row.command = placed->command.empty() ? token : placed->command;
        slot.row.command_target = label;
        slot.row.command_current = placed->current;
        slot.row.command_latched = placed->latched;
        slot.row.latch_is_heading = placed->fields.is_heading;
        slot.row.latch_steer = placed->fields.steer_or_heading;
        slot.row.latch_thrust = placed->fields.thrust;
        ++host.summary.player_orders;
        const double dx = static_cast<double>(point[0]) - slot.motion.position[0];
        const double dz = static_cast<double>(point[2]) - slot.motion.position[2];
        host.log.notef("player command issued to \"%s\": token=\"%s\" resolved=\"%s\" a "
            "FIXED POINT %s through the command builder 0088a810 takes for "
            "NavigatorMoveToPos, not a named entity. The hull is at (%.1f, %.1f), so the "
            "point is %.2f m ahead. current=%d latched=%d",
            slot.row.name.c_str(), token.c_str(), placed->command.c_str(), label,
            static_cast<double>(slot.motion.position[0]),
            static_cast<double>(slot.motion.position[2]),
            std::sqrt(dx * dx + dz * dz), placed->current ? 1 : 0,
            placed->latched ? 1 : 0);
        if (!placed->blocked.empty()) host.log.notef("  %s", placed->blocked.c_str());
        return true;
    }
    const GameCommandRow* row = host.commands.issue(index, token,
        target_token, slot.ring, heading);
    if (row == nullptr) return false;
    slot.row.command = row->command.empty() ? token : row->command;
    slot.row.command_target = target_token;
    slot.row.command_current = row->current;
    slot.row.command_latched = row->latched;
    slot.row.latch_is_heading = row->fields.is_heading;
    slot.row.latch_steer = row->fields.steer_or_heading;
    slot.row.latch_thrust = row->fields.thrust;
    ++host.summary.player_orders;
    host.log.notef("player command issued to \"%s\": token=\"%s\" resolved=\"%s\" "
        "outcome=%s current=%d latched=%d (%s %.3f, thrust %.3f)", slot.row.name.c_str(),
        token.c_str(), row->command.c_str(), row->resolve_outcome.c_str(),
        row->current ? 1 : 0, row->latched ? 1 : 0,
        row->fields.is_heading ? "heading" : "rudder",
        static_cast<double>(row->fields.steer_or_heading),
        static_cast<double>(row->fields.thrust));
    if (!row->blocked.empty()) host.log.notef("  %s", row->blocked.c_str());
    return true;
}

const GameCommandRow* GameUnitsHost::issue_script_command(std::size_t unit_index,
    std::uint32_t command_object, const bsp::SceneCommandTarget& target, int flags,
    const std::string& source, const std::string& target_name) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return nullptr;
    GameUnitSlot& slot = *host.slots[unit_index];
    const float heading = host.pose_heading_radians(slot);
    if constexpr (kSquadronReturnToBaseResolveBound) {
        if (command_object == 0x00E08F98u) host.record_return_to_base_007f16d0(unit_index);
    }
    const GameCommandRow* row = host.commands.issue_command_object(unit_index,
        command_object, target, flags, source, target_name, slot.ring, heading);
    // Packet cc9_land_task_reach: the member's bot installs `land` at the delivery.
    if constexpr (kSquadronReturnToBaseResolveBound && kSquadronLandTaskBound) {
        if (command_object == 0x00E08F98u) host.install_land_task_0099a3dd(unit_index);
    }
    // The unit's own row keeps the token the scene authored: a scripted order is
    // a second command on the same director, not a replacement for the first,
    // and GameScriptOrdersHost reports it in its own table.
    return row;
}  // namespace

int GameUnitsHost::squadron_ammo_type_007edad0(std::size_t unit_index) {
    Impl& host = *impl_;
    const bsp::PlaneSquadronHostRecord* sq = squadron_record_of(unit_index);
    if (sq == nullptr) return 0;
    if constexpr (Impl::kSquadronOrdnanceReaderBound) {
        host.done("PlaneSquadron::ammo_type_007edad0", 0x007edad0u);
        return host.squadron_ammo_type_007edad0(*sq);
    } else {
        host.record("PlaneSquadron::ammo_type_007edad0", 0x007edad0u);
        return host.squadron_ammo_type_stand_in(*sq);
    }
}

bool GameUnitsHost::plane_holds_rack_round_007b9140(std::size_t unit_index) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || !host.slots[unit_index]) return false;
    GameUnitSlot& s = *host.slots[unit_index];
    // 007B915B-007B918F: the first part answering stops the walk.
    for (const PlaneOrdnanceRack& r : host.ordnance_racks(s)) {
        if (r.class_id == 0) continue;
        if (host.plane_rack_holds_round(s, r)) return true;
    }
    return false;
}

bool GameUnitsHost::plane_any_rack_empty_007b92e0(std::size_t unit_index) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || !host.slots[unit_index]) return false;
    GameUnitSlot& s = *host.slots[unit_index];
    // 007B92E0-007B931A: the +994h racks at +974h in order, each asked
    // vtable[20Ch]; the first true answers. 006E3570 (single rack) is
    // vtable[21Ch](2Ah) + ammo +484h == 0, the negation of the round test
    // plane_holds_rack_round_007b9140 makes per rack (same LABELLED counts).
    // A rack whose projectile class the host could not read (class 0) is
    // skipped, as plane_holds_rack_round_007b9140 skips it; the image asks it.
    for (const PlaneOrdnanceRack& r : host.ordnance_racks(s)) {
        if (r.class_id == 0) continue;
        if (!host.plane_rack_holds_round(s, r)) return true;
    }
    return false;
}

int GameUnitsHost::plane_first_rack_bullet_id(std::size_t unit_index) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || !host.slots[unit_index]) return -1;
    GameUnitSlot& unit = *host.slots[unit_index];
    const auto& racks = host.ordnance_racks(unit);
    if (racks.empty()) return -1; // 007B9230: +994h <= 0.
    // 007B9230 queries element zero even when a later rack holds a torpedo.
    // Retain the established BSPGun-order projection, and resolve THAT slot's
    // current Equipment rather than treating the first general gun as a rack.
    PlaneOrdnanceRack first = racks.front();
    const int device = host.rack_equipment_device(unit, unit.row.type_id,
        first.platform_key, first.device_class);
    const std::string type = host.lua.read_device_class_string(device, "Type");
    first.device_class = device;
    first.class_id = host.read_device_bullet_class_id(device);
    first.authored_ammo = host.rack_equipment_ammo(unit, unit.row.type_id, first.platform_key);
    if (type == "BombPlatform") {
        first.multi = false;
        first.authored_ammo = host.rack_positive_bag_round(unit, first.authored_ammo);
        host.plane_rack_census(unit);
        const auto found = std::find_if(unit.rack_identity_per_rack.begin(),
            unit.rack_identity_per_rack.end(), [&](const SingleRackCensusIdentity& identity) {
                return identity.platform_key == first.platform_key && identity.device_class == device;
            });
        if (found == unit.rack_identity_per_rack.end())
            throw std::logic_error("first rack has no current equipment identity");
        first.single_index = static_cast<int>(found - unit.rack_identity_per_rack.begin());
    } else if (type == "MultiBombPlatform") {
        first.multi = true;
        first.single_index = -1;
    } else {
        throw std::logic_error("first rack current equipment is not a supported rack");
    }
    const int bullet_id = host.read_device_bullet_id(device);
    if (host.plane_rack_holds_round(unit, first)) return bullet_id;
    // bsp.py show 006e4060 --asm, 006E4086..006E40D7: query flag 1 admits
    // the configured +3F8h/+34h descriptor when owner slot1E4 (006D1F40's
    // LIVE 00E17BF2) is set and descriptor vslot8 answers 2Ah. SP missions
    // can enable this flag (usn_19_coralus.lua:64); do not infer it from mode.
    const bool reload = lua_device_reload_enabled_00e17bf2();
    GameGunneryHost* const owner = reload ? gunnery() : nullptr;
    const bool bomb_family = owner != nullptr
        && owner->weapon_class_descriptor(bullet_id, 0x2a) != nullptr;
    return empty_rack_configured_bullet_id(reload, bullet_id, bomb_family);
}

int GameUnitsHost::plane_reload_bomb_platforms_007c1d80(std::size_t unit_index) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || !host.slots[unit_index]) return -1;
    return host.plane_reload_bomb_platforms_007c1d80(*host.slots[unit_index]);
}

bool GameUnitsHost::request_ordnance_release_007bbba0(std::size_t unit_index) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || !host.slots[unit_index]) return false;
    host.release_ordnance_007bbba0(*host.slots[unit_index]);
    return true;
}

int GameUnitsHost::plane_bag_equipment(std::size_t unit_index) const {
    const Impl& host = *impl_;
    if (unit_index >= host.slots.size() || !host.slots[unit_index]) return -1;
    return host.slots[unit_index]->bag_equipment;
}

std::size_t GameUnitsHost::issue_return_to_base_007f16d0(std::size_t unit_index,
    const std::string& source) {
    Impl& host = *impl_;
    const bsp::PlaneSquadronHostRecord* sq = squadron_record_of(unit_index);
    if (sq == nullptr) return 0;
    const std::vector<std::size_t> members = sq->member_units;   // the issue may edit the record
    bsp::SceneCommandTarget target;
    target.kind = 0;
    target.position_valid = 0;
    target.object_id = 0;
    target.object = nullptr;
    target.position[0] = target.position[1] = target.position[2] = 0.0f;
    target.trailing = 0.0f;
    std::size_t placed = 0;
    for (const std::size_t plane : members) {
        if (plane == bsp::kPlaneSquadronNoUnit || plane >= host.slots.size()) continue;
        if (issue_script_command(plane, 0x00E08F98u, target, 1 /* 0077D600 flags */, source,
                                 host.slots[plane]->row.name) != nullptr) {
            ++placed;
        }
    }
    return placed;
}

void GameUnitsHost::store_commanded_speed_00890e6f(std::size_t unit_index, float speed) {
    Impl& host = *impl_;
    host.commands.store_commanded_speed_00890e6f(unit_index, speed,
        host.summary.simulated_seconds);
}

// Packet cc9_squadron_set_speed. 007D9E80: ctl+3Ch/+40h = 0 and ctl+44h = speed
// (007D9E86..007D9E96), ctl+48h..+50h from 00F87574..00F8757C, the zero vector
// (007D9E9B..007D9EBD), then 007D9C80 body to world through unit+74h and the
// world linear velocity copied to ctl+30h..+38h (007D9EC7..007D9ED6).
// SUBSTITUTION, labelled: the body +Z axis in world is the pose's forward row,
// the same axis the spawn seed uses; the ctl+30h..+38h copy has no host field.
// kSetSpeedCtlAxisCc13Bound (include/bsp/set_speed_ctl_axis_cc13.hpp): ctl+30h is
// the host's plane_velocity_lp_30, and ON stores it and drops the normalisation.
bool GameUnitsHost::set_plane_forward_speed_007d9e80(std::size_t unit_index, float speed) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[unit_index];
    if (!slot.plane_velocity_seeded) return false;
    const float* const fwd = slot.motion.pose_row2;
    if constexpr (bsp::kSetSpeedCtlAxisCc13Bound) {
        // 007D9C80 -> 0042D0D0(ctl+3Ch = (0, 0, speed), unit+74h, w 0): each axis is
        // f32((0 * row1[i] + 0 * row0[i]) + speed * row2[i]) per x87 op
        // (0042D0F2..0042D11A for i = 0, then +4h / +8h). The zero products only
        // decide the sign of a zero result; no normalisation and no length guard.
        // The angular term is the zero vector.
        namespace pc24 = bsp::cc13x87;
        const float* const row0 = slot.motion.pose_row0;
        const float* const row1 = slot.motion.pose_row1;
        for (int i = 0; i < 3; ++i) {
            slot.plane_world_velocity[i] = static_cast<float>(pc24::x87_add24(
                pc24::x87_add24(pc24::x87_mul24(0.0f, row1[i]), pc24::x87_mul24(0.0f, row0[i])),
                pc24::x87_mul24(speed, fwd[i])));
            slot.plane_body_angular[i] = 0.0f;
        }
        // 007D9EC7..007D9ED6: ctl+30h..+38h, the low-passed velocity, = ctl+18h..+20h.
        if constexpr (Impl::kPlaneVelocityLowpassBound) {
            for (int i = 0; i < 3; ++i) slot.plane_velocity_lp_30[i] = slot.plane_world_velocity[i];
            slot.plane_velocity_lp_valid = true;
        }
    } else {
    const float len = std::sqrt(fwd[0] * fwd[0] + fwd[1] * fwd[1] + fwd[2] * fwd[2]);
    if (!(len > 1e-6f)) return false;
    for (int i = 0; i < 3; ++i) {
        slot.plane_world_velocity[i] = speed * fwd[i] / len;
        slot.plane_body_angular[i] = 0.0f;
    }
    }
    slot.motion.linear_velocity = bsp::OceanVec3{
        slot.plane_world_velocity[0], slot.plane_world_velocity[1],
        slot.plane_world_velocity[2]};
    host.done("PlaneFlightController::set_forward_speed", 0x007d9e80u);
    return true;
}

// Packet cc9_set_submarine_depth_level. 008528B0: clamp to 0..3 (008528CC..008528E0),
// 1 when class+510h or +514h is above 0 (008528E5..00852908), then, only when
// +1268h differs (0085290D JE), store it (0085291E) and post session message A2h
// with the level through 0077C7B0 (00852915..00852956).
// The hull move to bands[level] is not this function's: the dive controller
// 00936DC0 reads +1268h every tick (run_submarine_dive_00936dc0,
// kSubmarineDiveBound). Packet cc13-subdive (include/bsp/cc13_sub_dive.hpp,
// docs/CC13_SUB_DIVE.md): kCc13SubDiveKamikazeBound takes the class test from
// the slot (read at the seed), OFF it reads false (LABELLED). The A2h post stays
// a record: its 0077C7B0 gate (game+1FE4h == 1, 0077C7B6..0077C7C0) delivers
// nothing in single player, and the handler 00852C60 only re-calls 008528B0 on a
// remote peer.
bool GameUnitsHost::set_submarine_depth_level_008528b0(std::size_t unit_index, int requested) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return false;
    const GameUnitSlot& slot = *host.slots[unit_index];
    GameUnitRow& row = host.slots[unit_index]->row;
    if (!row.submarine_depth_seeded) return false;
    const bool kamikaze = bsp::cc13_sub_dive::kCc13SubDiveKamikazeBound && slot.sub_kamikaze_class;
    const int level = static_cast<int>(
        bsp::submarine_clamp_depth_command_008528b0(requested, kamikaze));
    host.done("SubmarineUnit::set_depth_level", 0x008528b0u);
    if (row.submarine_depth_level == level) return false;
    row.submarine_depth_level = level;
    host.record("SubmarineUnit::depth_message_a2", 0x0077c7b0u);
    return true;
}

void GameUnitsHost::set_submarine_periscope_state_122c(std::size_t unit_index, int state,
                                                       bool has_node, float move_range) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[unit_index];
    if (!slot.row.submarine_depth_seeded) return;
    slot.sub_periscope_node_1214 = has_node;
    slot.sub_periscope_move_range = move_range;
    if (slot.sub_periscope_state_122c == state) return;
    if (state == 1) ++slot.sub_periscope_raises;
    slot.sub_periscope_state_122c = state;
    host.log.notef("submarine periscope state: unit=%s state=%d node=%d range=%.2f t=%.2f "
        "bound=%d (+122Ch, packet cc9_periscope_out)", slot.row.name.c_str(), state,
        has_node ? 1 : 0, static_cast<double>(move_range), host.summary.simulated_seconds,
        kSubmarinePeriscopeOutBound ? 1 : 0);
}

int GameUnitsHost::submarine_periscope_state_122c(std::size_t unit_index) const {
    if (unit_index >= impl_->slots.size()) return 0;
    return impl_->slots[unit_index]->sub_periscope_state_122c;
}

bool GameUnitsHost::begin_submarine_periscope_failure(std::size_t unit_index, float clock) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[unit_index];
    if (!slot.row.submarine_depth_seeded || slot.sub_periscope_state_122c == 2) return false;
    const auto* settings = host.lua.gameplay_tuning_settings();
    if (settings == nullptr) return false;
    const auto& hull = host.class_hull_box(slot);
    // The existing model reader applies 009396BA..009399BF's periszkop and
    // kamikaze gates. This is its actual retained shape, not the AI's old
    // assumed-present node. Do not synthesize an absent shape to run a body.
    // bsp.py show 009373c0 --asm: ECX = the controller, ESI = [controller+1Ch]
    // the unit; 009373C4 CMP [EDI+394h],0 / JZ is the periscope-shape gate, then
    // the +122Ch/+125Ch/+1258h/+1254h stores. Its callers are 008132C0 (SetFailure)
    // and 009377E0 (contact). 009327F0 repeats those stores but nothing in the
    // image calls it, so the outer body runs here.
    const auto broken = bsp::gameplay_seg61_62_cc13::submarine_break_periscope_009373c0(
        hull.ok && hull.box.periscope_merged, clock, settings->submarine_periscope_repair_time);
    if (!broken.acts) return false;
    slot.sub_periscope_state_122c = broken.state_122c;
    slot.sub_periscope_repair = {broken.deadline_1254, broken.clock_1258, broken.clock_fast_125c};
    slot.sub_periscope_node_1214 = hull.box.periscope_node;
    // 00853630 NumberOr("PeriscopeMoveRange", 00CE38B8 = 10). A Lua
    // failure may precede the first AI writer that normally binds this value.
    slot.sub_periscope_move_range = host.lua.read_vehicle_class_number(
        slot.row.type_id, "PeriscopeMoveRange", 10.0f);
    host.done("SubmarineUnit::periscope_break_009373c0", 0x009373c0u);
    // The plan's other two effects need owners this host does not have: the
    // mast node hide (009373DF 00B6DA70(0.0, 0) on unit+1214h) and the shape's
    // collision bits (00937430 00C47FC0(2) on controller+394h).
    if (broken.hide_mast_node_1214)
        host.record("SubmarineUnit::periscope_mast_hide_00b6da70", 0x009373dfu);
    if (broken.shape_collision_c47fc0_2)
        host.record("SubmarineUnit::periscope_shape_collision_00c47fc0", 0x00937430u);
    return true;
}

bool GameUnitsHost::submarine_periscope_out_1234(std::size_t unit_index) const {
    const Impl& host = *impl_;
    if (!kSubmarinePeriscopeOutBound || unit_index >= host.slots.size()) return false;
    const GameUnitSlot& slot = *host.slots[unit_index];
    return slot.row.submarine_depth_seeded && slot.sub_periscope_out_1234;
}

bool GameUnitsHost::submarine_band_y(std::size_t unit_index, int band, float& y) const {
    const Impl& host = *impl_;
    if (unit_index >= host.slots.size() || band < 0 || band > 3) return false;
    const GameUnitSlot& slot = *host.slots[unit_index];
    if (!slot.row.submarine_depth_seeded) return false;
    y = slot.sub_bands[band];
    return true;
}

// Goal 4 chain 369. 00852410 (bsp show 00852410) on a seeded submarine, through
// its typed body: the fields are what the boat's dive law 00936DC0 steps here
// (run_submarine_dive_00936dc0), +100h the hull y, the rates the class keys the
// seed read (UpDownAccel, UpSpeed, DownSpeed), the body velocity 0092D6A0 the
// linear velocity the dive law last wrote, and 008522C0 the gunnery's adapter.
bool GameUnitsHost::predict_submarine_depth(std::size_t unit_index, float dt, float& y) const {
    const Impl& host = *impl_;
    if (unit_index >= host.slots.size() || host.slots[unit_index] == nullptr) return false;
    const GameUnitSlot& slot = *host.slots[unit_index];
    if (!slot.row.submarine_depth_seeded) return false;
    bsp::gameplay_seg53_cc13::SubmarineDepthFields fields;
    fields.world_y_100 = slot.motion.position[1];                       // +100h
    for (int band = 0; band < 4; ++band) fields.bands_1200[band] = slot.sub_bands[band];
    fields.clearance_123c = slot.sub_scan.clearance;                     // +123Ch
    fields.depth_level_1268 = slot.row.submarine_depth_level;            // +1268h
    fields.sink_1270 = slot.sub_sink_1270;                               // +1270h
    fields.need_air_1281 = kSubmarineAirBound && slot.sub_air.need_air;  // +1281h
    bsp::gameplay_seg53_cc13::SubmarineDiveRates rates;
    rates.accel_824 = slot.sub_accel;                                    // class+824h
    rates.up_speed_830 = slot.sub_up_speed;                              // class+830h
    rates.down_speed_834 = slot.sub_down_speed;                          // class+834h
    const GameGunneryHost* const gun = host.gunnery.get();
    const bool catapult = gun != nullptr
        && gun->submarine_catapult_wants_surface(slot.process_index);   // 008524AA
    y = bsp::gameplay_seg53_cc13::submarine_predict_depth_after_00852410(
        fields, rates, slot.motion.linear_velocity.y, catapult, dt);
    return true;
}

// Packet cc13-fix21. 008938E0's read of unit+127Ch (008939F6 FLD float): the
// value as 00855250 and 00893A60 last left it. The image reads any entity; a
// slot that is not a seeded submarine has no air state here.
bool GameUnitsHost::submarine_air_008938e0(std::size_t unit_index, float& air) const {
    const Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return false;
    const GameUnitSlot& slot = *host.slots[unit_index];
    if (!slot.sub_dive_ready) return false;
    air = slot.sub_air.air;
    return true;
}

// Packet cc9_submarine_air. 00893C00's store at unit+1280h.
// Packet cc9_squadron_attack_alt. 008A22B0: argument 0 through 00888AA0 (no null
// test in the image; a unit with no squadron is refused here), 008A2456..008A2489.
bool GameUnitsHost::set_squadron_attack_alt_008a22b0(std::size_t unit_index, float altitude,
                                                     bool force) {
    Impl& host = *impl_;
    GameUnitSlot* sq = host.squadron_slot_of(unit_index);
    if (sq == nullptr) return false;
    sq->sq_timer_expiry_37c = host.summary.simulated_seconds + 0.5;  // +37Ch = [00CE3800]
    sq->sq_freeze_38c = force;                                        // +38Ch
    sq->sq_alt_398 = altitude;                                        // +398h
    sq->sq_lock_3aa = true;                                           // +3AAh = 1
    sq->sq_attack_alt_set = true;                                     // +3ADh = 0
    sq->sq_lua_alt_3ad = true;                                        // 008A2489
    host.done("PlaneSquadron::set_attack_alt_008a22b0", 0x008a22b0u);
    return true;
}

bool GameUnitsHost::set_squadron_travel_alt_0089f550(std::size_t unit_index, float altitude,
                                                     bool force) {
    Impl& host = *impl_;
    GameUnitSlot* sq = host.squadron_slot_of(unit_index);
    if (sq == nullptr) return false;
    sq->sq_timer_expiry_380 = host.summary.simulated_seconds + 0.5;  // +380h = [00CE3800]
    sq->sq_freeze_38d = force;                                        // +38Dh
    sq->sq_alt_394 = altitude;                                        // +394h
    sq->sq_lock_3a9 = true;                                           // +3A9h = 1
    sq->sq_alt_set = true;                                            // +3ADh = 0
    sq->sq_lua_alt_3ad = true;                                        // 0089F729
    sq->sq_ever_set = true;
    host.done("PlaneSquadron::set_travel_alt_0089f550", 0x0089f550u);
    return true;
}

// Packet l68_travel_speed. 008A0630's four stores (008A07E2..008A0802).
bool GameUnitsHost::set_squadron_travel_speed_008a0630(std::size_t unit_index, float speed,
                                                       bool force) {
    Impl& host = *impl_;
    GameUnitSlot* sq = host.squadron_slot_of(unit_index);
    if (sq == nullptr) return false;
    bsp::PlaneSquadronTravelSpeedHostCountdown{
        sq->sq_countdown_388, sq->sq_freeze_38f}.reset_script_override(); // +388h = [00CE3800]
    sq->sq_freeze_38f = force;                                        // +38Fh
    sq->sq_speed_3a0 = speed;                                         // +3A0h
    sq->sq_lock_3ac = true;                                           // +3ACh = 1
    sq->sq_speed_set = true;
    host.done("PlaneSquadron::set_travel_speed_008a0630", 0x008a0630u);
    return true;
}

// Packet cc13-issuea0. 008A1F40 SquadronSetDropAllBombPercent: argument 0
// through 00888AA0 (008A203F; no null test in the image), argument 1 through
// 00B66270, then FSTP float [ESI+390h] (008A2091). There is no other write.
// squadron_slot_of falls back to the unit's own slot when it has no squadron,
// which is close to the image's untyped [ESI+390h] store on that object; only an
// out-of-range index is refused.
bool GameUnitsHost::set_squadron_drop_all_bomb_percent_008a1f40(std::size_t unit_index,
                                                                float percent) {
    Impl& host = *impl_;
    GameUnitSlot* sq = host.squadron_slot_of(unit_index);
    if (sq == nullptr) return false;
    sq->sq_drop_all_bomb_percent_390 = percent;
    host.done("PlaneSquadron::set_drop_all_bomb_percent_008a1f40", 0x008a1f40u);
    return true;
}

// Goal 2 chain 209. The store of 008A28B0 SquadronSetForceRelease
// (lua_squadron_set_force_release): argument 0 through 00888AA0, then
// MOV [ESI+3AEh], BL (008A29FC) with BL = 00B66250's boolean of argument 1. That
// body takes LuaArgs/ImageAccess adapters this host does not have, so the Lua
// glue resolves the entity and this setter makes the one store, as for 008A1F40
// above. squadron_slot_of maps the entity to the slot its planes read as
// [state+4]+0Ch (falling back to the entity's own slot, as 008A1F40 does).
bool GameUnitsHost::set_squadron_force_release(std::size_t unit_index, bool on) {
    Impl& host = *impl_;
    GameUnitSlot* sq = host.squadron_slot_of(unit_index);
    if (sq == nullptr) return false;
    sq->set_force_release_3ae(on);
    host.done("PlaneSquadron::set_force_release", 0x008a28b0u);
    return true;
}

// Packet cc13-luawire 7b. 00951F40(visible) on one entity: attach (1) or detach
// (0) part +4A4h's spatial node, which is this host's plane_hit_node_detached.
// The squadron fan-out and its null-member case belong to the 008A13D0 runner.
bool GameUnitsHost::set_plane_visible_00951f40(std::size_t unit_index, bool visible) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || !host.slots[unit_index]) return false;
    host.slots[unit_index]->plane_hit_node_detached = !visible;
    host.done("PlaneUnit::set_visible_00951f40", 0x00951f40u);
    return true;
}

// Packet cc13-luawire 7b. 008A0000 on the entity itself, with no class test:
// +3A4h = on ? v | 2 : v & 0x000000FD. The reader (009A1959) takes the plane's
// squadron slot, so only a write on the squadron's fused slot is ever read.
bool GameUnitsHost::set_squadron_vehicle_avoidance_008a0000(std::size_t unit_index, bool on) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || !host.slots[unit_index]) return false;
    std::uint32_t& v = host.slots[unit_index]->sq_avoid_3a4;
    v = on ? (v | 2u) : (v & 0x000000FDu);
    host.done("PlaneSquadron::enable_vehicle_avoidance_008a0000", 0x008a0000u);
    return true;
}

// Packet cc13-luawire 7b. 006CA730 appends the squadron to the base's block
// +B4h list (std::list push, so a repeat adds a second node; the set keeps one,
// which 006C4790's any-match answer cannot tell apart). The block is the base's
// air-ops deck, named by the base's row name; a base that has no deck is still
// recorded, and no reader ever asks for it. False: no squadron record for
// squadron_unit (its fused slot or a member) or no base slot.
bool GameUnitsHost::add_base_unsupported_006ca730(std::size_t squadron_unit,
                                                  std::size_t base_unit) {
    Impl& host = *impl_;
    if (base_unit >= host.slots.size() || !host.slots[base_unit]) return false;
    const bsp::PlaneSquadronHostRecord* rec = nullptr;
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit == squadron_unit) { rec = &r; break; }
    }
    if (rec == nullptr) rec = bsp::plane_squadron_registry().find_by_member_unit(squadron_unit);
    if (rec == nullptr) return false;
    host.base_unsupported_b4.insert({host.slots[base_unit]->row.name, rec->name});
    host.done("AirBase::add_unsupported_squadron_006ca730", 0x006ca730u);
    return true;
}

// Packet cc13-luawire 7b. 008A62A0's store of float +340h on one entity; the
// squadron fan-out (members 0..4, stopping at the first null) belongs to the
// runner. Read only under kLuaSetCheatTurboBound.
bool GameUnitsHost::set_cheat_turbo_340(std::size_t unit_index, float value) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || !host.slots[unit_index]) return false;
    host.slots[unit_index]->cheat_turbo_340 = value;
    host.done("PlaneUnit::set_cheat_turbo_008a62a0", 0x008a62a0u);
    return true;
}

bool GameUnitsHost::set_unlimited_air_00893c00(std::size_t unit_index, bool flag) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[unit_index];
    if (!slot.sub_dive_ready) return false;
    slot.sub_air.unlimited_air = flag;
    host.done("SubmarineUnit::set_unlimited_air", 0x00893c00u);
    return true;
}

// Packet cc13-fix17. 00893A60's store at unit+127Ch: the value as given, no
// clamp (00855250 clamps it on the next force step). The image writes any
// entity; a slot that is not a seeded submarine has no air state here.
bool GameUnitsHost::set_submarine_air_00893a60(std::size_t unit_index, float air) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[unit_index];
    if (!slot.sub_dive_ready) return false;
    slot.sub_air.air = air;
    host.done("SubmarineUnit::set_air_supply", 0x00893a60u);
    return true;
}

bsp::CruiseSpeedSetting GameUnitsHost::commanded_speed(std::size_t unit_index) const {
    return impl_->commands.commanded_speed(unit_index);
}

void GameUnitsHost::set_skill_level_007b8ae0(std::size_t unit_index, int level) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return;
    if constexpr (kSkillLevelSquadronFanOutBound) {
        const bsp::PlaneSquadronHostRecord* squadron = nullptr;
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == unit_index) {
                squadron = &r;
                break;
            }
        }
        if (squadron == nullptr) {
            set_unit_skill_index_006d1eb0(unit_index, level);     // 006D1EB0
            host.done("Unit::set_skill_index_006d1eb0", 0x006d1eb0u);
            return;
        }
        for (const std::size_t member : squadron->member_units) {  // 007ECFAB
            if (member == bsp::kPlaneSquadronNoUnit) continue;
            set_unit_skill_index_006d1eb0(member, level);
        }
        host.done("PlaneSquadron::set_skill_all_members_007ecf80", 0x007ecf80u);
        return;
    }
    host.slots[unit_index]->pilot_skill_index = level;
    // 007ECF80: a squadron forwards the call to every member's vtable[128h].
    // process_index is the slot index (see where slots are appended).
    const bsp::PlaneSquadronHostRecord* const sqn =
        bsp::plane_squadron_registry().find_by_member_unit(unit_index);
    if (sqn == nullptr) return;
    for (const std::size_t member : sqn->member_units) {
        if (member == bsp::kPlaneSquadronNoUnit || member >= host.slots.size()) continue;
        host.slots[member]->pilot_skill_index = level;
    }
}

bool GameUnitsHost::set_unit_skill_index_006d1eb0(std::size_t unit_index, int level) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || host.slots[unit_index] == nullptr) return false;
    namespace skill = bsp::gameplay_seg61_62_cc13;
    GameUnitSlot& slot = *host.slots[unit_index];
    std::vector<skill::RoleDevice> devices;
    std::vector<std::size_t> device_guns;
    bool has_pass = false;
    if (host.gunnery != nullptr) {
        has_pass = host.gunnery->has_unit_gunnery_pass(unit_index);
        // These rows are the actual gun children produced for this unit,
        // all IsKindOf(20h); non-gun children do not receive the skill call.
        const auto& guns = host.gunnery->guns();
        for (std::size_t gun_index = 0; gun_index < guns.size(); ++gun_index) {
            const GameGunRow& gun = guns[gun_index];
            if (gun.unit_index == unit_index) {
                devices.push_back({true, gun.category});
                device_guns.push_back(gun_index);
            }
        }
    }
    // 006D1EB0 forwards unit+38Ch's slot zero. The shared base body is
    // 009565A0: 009565A3 gates on unit+5Dh, 009565B9 stores +390h,
    // 009565C4 notifies the +6DCh pass, then 009565EA notifies gun children.
    // Evidence: bsp.py show 006d1eb0 / 009565a0. This host normalizes the
    // pass/gun/pilot scalar skill readers onto pilot_skill_index. Device
    // notifications also have non-scalar effects: 00727E70 reaches the
    // tail gunner's 008FBEF0, which rerolls +6Ch even at the same level.
    // Replay the plan against the corresponding actual gun rows below.
    // This is a typed host adapter, not the original subobject/vtable ABI.
    const auto plan = skill::unit_skill_apply_level_009565a0(
        slot.state->simulate != 0, level, has_pass, devices);
    if (!plan.applied) return false;
    slot.pilot_skill_index = plan.level_390;
    for (const std::size_t device : plan.devices) {
        host.gunnery->notify_gun_skill_level(device_guns.at(device), plan.level_390);
    }
    host.done("UnitSkill::apply_level_009565a0", 0x009565a0u);
    return true;
}

bool GameUnitsHost::set_unit_max_speed_09c0(std::size_t unit_index, float value) noexcept {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size() || host.slots[unit_index] == nullptr) return false;
    host.slots[unit_index]->motion.max_speed = value;  // 00890B51, unit+9C0h
    return true;
}

// 00999AA0 (00999AA0-00999ADB): the pilot bot's hit notice, which the plane hit
// handler 007BBCF0 (vtable[ECh]) calls on [unit+DF4h] before 008777D0. It walks
// the task list bot+58h/+5Ch and calls task->vtable[2Ch](hit) until one answers
// true. The slots read from disk bytes:
//  - torpedo (vtable 00D213C8) 009D3270: task+52Ch = approach+134h = 0.0, true.
//    That clock selects DistFar over DistNear once it passes 15 s.
//  - divebomb 009C7900: task+4BCh = approach+C4h = 0.0, true. Carried as
//    db_approach_clock_c4; 009C4AA4 reads it under kDiveHitClockBound.
//  - strafe 009CC400: task+43Ch = approach+44h = 0.0, true (bsp::kStrafeTaskBound).
//  - levelbomb and retreat answer false.
// Returns true when a task took the notice.
bool GameUnitsHost::plane_hit_task_notify_00999aa0(std::size_t plane_index) {
    Impl& host = *impl_;
    if (plane_index >= host.slots.size() || host.slots[plane_index] == nullptr) return false;
    GameUnitSlot& unit = *host.slots[plane_index];
    if (unit.torpedo_task_installed) {
        unit.torpedo_approach.elapsed_134 = 0.0f;   // 009D3273
        ++unit.torpedo_hit_clock_resets;
        host.record("Bot::hit_task_notify_torpedo_009d3270", 0x009d3270u);
        return true;
    }
    if (unit.dive_bomb_task_installed) {
        unit.db_approach_clock_c4 = 0.0f;           // 009C7903, task+4BCh
        ++unit.db_hit_clock_resets;
        host.record("Bot::hit_task_notify_divebomb_009c7900", 0x009c7900u);
        return true;
    }
    if (bsp::kStrafeTaskBound && unit.strafe_task_installed) {
        unit.st_ap.elapsed_44 = 0.0f;               // 009CC403, task+43Ch
        ++unit.st_hit_resets;
        host.record("Bot::hit_task_notify_strafe_009cc400", 0x009cc400u);
        return true;
    }
    if (bsp::kRocketTaskBound && unit.rocket_task_installed) {
        (void)bsp::rocket_on_hit_007b6d00(unit.rk, 0);   // approach+44h = 0, task+43Ch
        ++unit.rk_hit_resets;
        host.record("Bot::hit_task_notify_rocket_007b6d00", 0x007b6d00u);
        return true;
    }
    return false;
}

// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 7). 008A24E0
// SquadronSetReleaseAlt's five stores on the squadron block, 008A2692..008A26B9:
// +384h = [00CE3800] (0.5), +38Eh = the force flag, +39Ch = altitude, +3ABh = 1,
// +3ADh = 0. A unit with no squadron is refused (the image has no null test).
bool GameUnitsHost::set_squadron_release_alt_008a24e0(std::size_t unit_index, float altitude,
                                                      bool force) {
    Impl& host = *impl_;
    GameUnitSlot* sq = host.squadron_slot_of(unit_index);
    if (sq == nullptr) return false;
    sq->sq_timer_expiry_384 = host.summary.simulated_seconds + 0.5;  // +384h = [00CE3800]
    sq->sq_freeze_38e = force;                                        // +38Eh
    sq->sq_alt_39c = altitude;                                        // +39Ch
    sq->sq_lock_3ab = true;                                           // +3ABh = 1
    sq->sq_release_alt_set = true;                                    // +3ADh = 0
    sq->sq_lua_alt_3ad = true;                                        // 008A26B9
    host.done("PlaneSquadron::set_release_alt_008a24e0", 0x008a24e0u);
    return true;
}

int GameUnitsHost::skill_level(std::size_t unit_index) const {
    const Impl& host = *impl_;
    return unit_index < host.slots.size() ? host.slots[unit_index]->pilot_skill_index : 1;
}

void GameUnitsHost::note_kill_cause_00926e02(std::size_t unit_index, int cause) {
    impl_->note_kill_cause(unit_index, cause);
}

int GameUnitsHost::kill_cause_00926d90(std::size_t unit_index) const {
    const Impl& host = *impl_;
    return unit_index < host.kill_cause_by_unit.size() ? host.kill_cause_by_unit[unit_index] : 1;
}

std::vector<std::pair<std::size_t, float>> GameUnitsHost::destroyed_units() const {
    std::vector<std::pair<std::size_t, float>> out;
    const Impl& host = *impl_;
    if (host.gunnery == nullptr) return out;
    for (const GameGunneryUnitRow& row : host.gunnery->unit_rows()) {
        if (row.sunk) out.emplace_back(row.unit_index, row.sunk_seconds);
    }
    return out;
}

float GameUnitsHost::mission_clock() const noexcept {
    return impl_->summary.simulated_seconds;
}

void GameUnitsHost::run_director_steps_00836920() {
    Impl& host = *impl_;
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        GameUnitSlot& slot = *host.slots[index];
        if (!slot.state->active) continue;
        const float heading = host.pose_heading_radians(slot);
        const GameDirectorStepOutcome outcome = host.commands.director_step_00836920(
            index, host.player_flag_0184(slot), host.summary.simulated_seconds, slot.ring,
            heading);
        // Packet cc8_ship_moveonpath, edited under the integrator's hunk
        // arbitration of 2026-09-19. 00836BF0-00836D66 is the `moveonpath` arm
        // of the same step, and it is the only place the path cursor moves:
        // 009E59C0 reads the cursor and never writes it.
        {
            float px = 0.0f, py = 0.0f, pz = 0.0f;
            unit_position_00fc(index, px, py, pz);
            // Packet cc8_ship_drive, edited under the integrator's hunk
            // arbitration of 2026-09-19. 00835C70's begin runs before the arm:
            // 0071F600 builds the path object for the command that is the queue
            // head, so a `moveonpath` that was queued under another command
            // begins on the step the queue advances to it. Where in a frame
            // 00835C70 runs is not established (00D09FD0/00D09FD4 are its only
            // references), so this is the same choice milestone 2m already made
            // for the `cruise` begin: immediately, on the director step.
            host.commands.begin_current_command_00835c70(index, px, pz);
            // Packet s53_path_multileg: unit->vtable[50h] (006DFD60, unit+1050h)
            // and [unit+740h]->vtable[48h] for 007ADD70's heading and force.
            GameShipAiHost* const ai = host.ship_ai;
            const bool plane = unit_is_kind_of(index, 0x0f) || unit_is_kind_of(index, 0x18);
            // Packet s53_sqn_path. 0084E2B8..0084E317: class+26Ch is 007DB4D0's
            // answer, TravelSpeed / turn rate; 00415550 keeps the larger of it *
            // 1.2 and Pilot/MoveTo/ClosingDist; * 1.5 for a role-0 slot that is
            // neither 8 nor AI-held. LABELLED: this host fuses the squadron into
            // its flight leader, so the leader stands for [block+224h] (position)
            // and its +3D0h (heading, the pose yaw); AI-held is every slot but
            // the local player's (as 0077C470's role message does).
            float squadron_radius = 0.0f;
            float path_heading = slot.hull_heading_1050;
            if (bsp::kSquadronPathFollowStepBound && plane) {
                const float rate = host.plane_class_turn_rate_270_007db4d0(slot);
                const float r26c = static_cast<float>(
                    static_cast<double>(slot.plane_travel_speed) / static_cast<double>(rate));
                const float scaled = static_cast<float>(
                    static_cast<double>(r26c) * 1.2000000476837158);
                const float closing = host.lua.plane_globals_loaded()
                    ? host.lua.plane_globals().pilot_move_to_closing_dist : 0.0f;
                squadron_radius = scaled > closing ? scaled : closing;
                const std::int32_t role0 = slot.current_roles_01ac[0];
                if (role0 != 8 && role0 == Impl::kLocalPlayerSlot) {
                    squadron_radius = static_cast<float>(
                        static_cast<double>(squadron_radius) * 1.5);
                }
                path_heading = Impl::pose_heading_radians(slot);
            }
            host.commands.advance_path_cursor_00836bf0(index, px, pz,
                unit_hull_length_09c8(index), unit_class_turn_radius_0520(index),
                path_heading,
                [ai, index](float x, float z) {
                    return ai != nullptr && ai->goal_reached_009da590(index, x, z);
                },
                !plane, squadron_radius);
        }
        if (outcome.reissued == bsp::DirectorDefaultCommand::None) continue;
        // The idle tail's own choice becomes the unit's standing command when
        // the slot push took it; 0071be40 is what answers that.
        slot.row.command_current = slot.row.command_current
            || host.commands.holds_cruise(index);
    }
}

const GameCommandsHost& GameUnitsHost::commands() const noexcept { return impl_->commands; }

// Packet cc8_ship_moveonpath, edited under the integrator's hunk arbitration of
// 2026-09-19: the script-orders host reaches the command path only through this
// object, and the `moveonpath` path build writes to the director it owns.
GameCommandsHost& GameUnitsHost::commands() noexcept { return impl_->commands; }

void GameUnitsHost::set_controlled_unit_004c0890(std::size_t index) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[index];
    host.controlled_index = index;
    ControlledUnitQueryBinding query(slot.class_id);
    SetControlledUnitBinding binding(host);
    const bsp::ControlledUnitGlobals globals
        = bsp::set_controlled_unit_004c0890(true, query, binding);
    host.done("ControlledUnit::set_controlled_unit", 0x004c0890u);
    for (std::unique_ptr<GameUnitSlot>& other : host.slots) other->row.controlled = false;
    slot.row.controlled = true;
    host.summary.controlled_bound = globals.unit_present;
    host.summary.controlled_index = index;
    host.summary.controlled_name = slot.row.name;
    host.log.notef("controlled unit: 00e188d8 = \"%s\" (%s %s, party %d); 00e188dc %s; "
        "class_id=%d IsKindOf(0Fh)=%d IsKindOf(18h)=%d; "
        "listener handle/publication adapters remain unimplemented",
        slot.row.name.c_str(), slot.row.class_name.c_str(), slot.row.type_symbol.c_str(),
        slot.row.party, globals.listener_present ? "published a handle" : "was cleared",
        slot.class_id, query.unit_is_kind_of(0x0f) ? 1 : 0,
        query.unit_is_kind_of(0x18) ? 1 : 0);
}

void GameUnitsHost::clear_controlled_unit_004c0890() {
    Impl& host = *impl_;
    const std::string before = host.controlled_bound && host.controlled_index < host.slots.size()
        ? host.slots[host.controlled_index]->row.name : std::string("none");
    ControlledUnitQueryBinding query(-1);   // not consulted on the null arm
    SetControlledUnitBinding binding(host);
    const bsp::ControlledUnitGlobals globals
        = bsp::set_controlled_unit_004c0890(false, query, binding);
    host.done("ControlledUnit::clear_controlled_unit", 0x004c0890u);
    for (std::unique_ptr<GameUnitSlot>& other : host.slots) other->row.controlled = false;
    host.summary.controlled_bound = globals.unit_present;
    host.summary.controlled_name.clear();
    host.log.notef("controlled unit: 00e188d8 = null (was \"%s\"); 004C0890(null)",
        before.c_str());
}

void GameUnitsHost::issue_player_order(float throttle, float rudder) {
    Impl& host = *impl_;
    if (!host.controlled_bound || host.controlled_index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[host.controlled_index];
    slot.standing_order = true;
    slot.standing_throttle = throttle;
    slot.standing_rudder = rudder;
    host.issue_into_ring(slot, throttle, rudder);
    ++host.summary.player_orders;
    host.log.notef("player order issued to \"%s\": throttle=%.3f rudder=%.3f through "
        "00816a40, clamped into [-2,+2] by 00815440", slot.row.name.c_str(),
        static_cast<double>(throttle), static_cast<double>(rudder));
}

void GameUnitsHost::update_entity_008255b0(std::size_t index, float scaled_delta) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[index];
    UnitInstanceBinding binding(host, slot);
    bsp::update_unit_instance_008255b0(*slot.state, slot.class_block, binding, scaled_delta);
    ++slot.row.instance_updates;
    ++host.summary.instance_updates;
    host.done("World::update_entity", 0x008255b0u);
}

// Packet cc9_ship_terrain_contact (docs/SHIP_AI_OPEN_ITEMS.md section 87). The
// kind-8 contact of 009377E0 and the stop the Dyn contact phase gives it, as a
// labelled substitution (see kShipTerrainContactBound).
void GameUnitsHost::Impl::ship_terrain_contact(GameUnitSlot& slot, const float before[3]) {
    const ClassHullBox& hull = class_hull_box(slot);
    if (!hull.ok || hull.box.shape_count == 0) return;
    const float keel_y = hull.box.min.y;
    const float zs[3] = {hull.box.max.z, 0.5f * (hull.box.min.z + hull.box.max.z),
                         hull.box.min.z};
    const float* r1 = slot.motion.pose_row1;
    const float* r2 = slot.motion.pose_row2;
    const float* p = slot.motion.position;
    bool contact = false;
    float deepest = 0.0f;
    float dx = p[0] - before[0];
    float dz = p[2] - before[2];
    float vx = slot.motion.linear_velocity.x;
    float vz = slot.motion.linear_velocity.z;
    bool moved = false;
    for (int i = 0; i < 3; ++i) {
        const float w[3] = {p[0] + r1[0] * keel_y + r2[0] * zs[i],
                            p[1] + r1[1] * keel_y + r2[1] * zs[i],
                            p[2] + r1[2] * keel_y + r2[2] * zs[i]};
        float ground = 0.0f;
        if (!world_ground_height_00903860(w, ground)) continue;
        const float depth = ground - w[1];
        if (!(depth > 0.0f)) continue;
        contact = true;
        if (depth > deepest) deepest = depth;
        // Packet cc9_hull_terrain_contact_gate: the Dyn contact phase replaces the stand-in.
        if constexpr (!kShipTerrainContactBound || bsp::kHullTerrainContactSolverBound) continue;
        // Only a step that deepens the point's penetration is resisted: the same
        // keel point at the step's start position (the pose's rotation kept).
        const float w0[3] = {w[0] - (p[0] - before[0]), w[1] - (p[1] - before[1]),
                             w[2] - (p[2] - before[2])};
        float ground0 = 0.0f;
        if (world_ground_height_00903860(w0, ground0) && !(depth > ground0 - w0[1] + 0.001f)) {
            continue;
        }
        // The uphill direction at the point: central differences over 1 unit.
        float h[4] = {ground, ground, ground, ground};
        const float px[3] = {w[0] + 1.0f, w[1], w[2]};
        const float mx[3] = {w[0] - 1.0f, w[1], w[2]};
        const float pz[3] = {w[0], w[1], w[2] + 1.0f};
        const float mz[3] = {w[0], w[1], w[2] - 1.0f};
        world_ground_height_00903860(px, h[0]);
        world_ground_height_00903860(mx, h[1]);
        world_ground_height_00903860(pz, h[2]);
        world_ground_height_00903860(mz, h[3]);
        float ux = 0.5f * (h[0] - h[1]);
        float uz = 0.5f * (h[2] - h[3]);
        float len = std::sqrt(ux * ux + uz * uz);
        if (!(len > 1.0e-6f)) continue;   // flat: the step did not climb
        ux /= len;
        uz /= len;
        const float along = dx * ux + dz * uz;
        if (along > 0.0f) {
            dx -= along * ux;
            dz -= along * uz;
            moved = true;
        }
        const float vin = vx * ux + vz * uz;
        if (vin > 0.0f) {
            vx -= vin * ux;
            vz -= vin * uz;
            moved = true;
        }
    }
    if (!contact) return;
    // With kHullTerrainContactSolverBound the latch comes from the solver's contacts.
    if constexpr (!bsp::kHullTerrainContactSolverBound) slot.ground_contact_1010 = true;
    ++slot.terrain_contact_steps;
    ++terrain_contact_steps;
    if (deepest > slot.terrain_max_penetration) slot.terrain_max_penetration = deepest;
    if (!slot.terrain_contact_seen) {
        slot.terrain_contact_seen = true;
        if (terrain_contact_logged < 80) {
            ++terrain_contact_logged;
            log.notef("ship terrain contact: unit=%s first at t=%.2f pos=(%.1f %.1f %.1f) "
                "penetration=%.2f bound=%d", slot.row.name.c_str(),
                static_cast<double>(summary.simulated_seconds), static_cast<double>(p[0]),
                static_cast<double>(p[1]), static_cast<double>(p[2]),
                static_cast<double>(deepest), kShipTerrainContactBound ? 1 : 0);
        }
    }
    if (!moved) return;
    ++terrain_contact_stops;
    slot.motion.position[0] = before[0] + dx;
    slot.motion.position[2] = before[2] + dz;
    slot.motion.linear_velocity.x = vx;
    slot.motion.linear_velocity.z = vz;
    slot.body.position[0] = slot.motion.position[0];
    slot.body.position[2] = slot.motion.position[2];
    slot.motion_state.linear_velocity = slot.motion.linear_velocity;
}

// Packet cc9_hull_terrain_contact_solver: one hull's contact result into the kind-8 latch
// (+1010h) and the per-unit census. Moved out of motion_step_00825f20 unchanged by packet
// cc9_hull_hull_contact, which calls it from the world phase too.
void GameUnitsHost::Impl::record_hull_terrain_result(GameUnitSlot& slot, std::size_t index,
    const bsp::HullTerrainContactStepResult& r) {
    Impl& host = *this;
    if (r.contact) {
        if constexpr (bsp::kHullTerrainContactSolverBound) {
            slot.ground_contact_1010 = true;   // 009377E0 kind 8
            host.done("DynScene::hull_terrain_contact_phase", 0x00c53630u);
        }
        Impl::HullTerrainUnit& u = host.hull_terrain_units[index];
        if (u.first_t < 0.0f) {
            u.first_t = host.summary.simulated_seconds;
            host.log.notef("hull terrain contact: unit=%s first at t=%.2f "
                "pos=(%.1f %.1f %.1f) candidates=%d depth=%.2f bound=%d "
                "(00C53630 / 00C3F760 / 00403720)", slot.row.name.c_str(),
                static_cast<double>(u.first_t),
                static_cast<double>(slot.body.position[0]),
                static_cast<double>(slot.body.position[1]),
                static_cast<double>(slot.body.position[2]), r.candidates,
                static_cast<double>(r.max_depth),
                bsp::kHullTerrainContactSolverBound ? 1 : 0);
        }
        ++u.steps;
        if (r.max_depth > u.max_depth) u.max_depth = r.max_depth;
        if (r.delta_linear.y > u.max_up_dv) u.max_up_dv = r.delta_linear.y;
        const float hdv = std::sqrt(r.delta_linear.x * r.delta_linear.x
            + r.delta_linear.z * r.delta_linear.z);
        if (hdv > u.max_horizontal_dv) u.max_horizontal_dv = hdv;
    }
}

// The position phase 00C5B1B0 and the rest of one ship's motion tick, moved out of
// motion_step_00825f20 unchanged by packet cc9_hull_hull_contact.
void GameUnitsHost::Impl::finish_motion_tick(GameUnitSlot& slot, const float before[3],
    const bsp::ShipMotionStepResult& result, float step_seconds) {
    Impl& host = *this;
    bsp::dyn_body_integrate_position_00c5b1b0(slot.body, host.physics_world,
        step_seconds);
    host.done("ShipMotion::rigid_body_position_phase", 0x00c5b1b0u);
    slot.motion.linear_velocity = slot.motion_state.linear_velocity;
    slot.motion.angular_velocity = slot.motion_state.angular_velocity;
    for (int i = 0; i < 3; ++i) {
        slot.motion.pose_row0[i] = slot.body.row0[i];
        slot.motion.pose_row1[i] = slot.body.row1[i];
        slot.motion.pose_row2[i] = slot.body.row2[i];
        slot.motion.position[i] = slot.body.position[i];
    }
    host.ship_terrain_contact(slot, before);
    Impl::publish_pose(slot);
    const double dx = static_cast<double>(slot.motion.position[0]) - before[0];
    const double dy = static_cast<double>(slot.motion.position[1]) - before[1];
    const double dz = static_cast<double>(slot.motion.position[2]) - before[2];
    slot.row.path_length += static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz));
    // Milestone 2s: the drift angle and the trajectory speed, measured from
    // the step's own displacement rather than from the velocity, so they
    // report where the hull went and not what it was told. The drift is the
    // angle in the horizontal plane between that displacement and the
    // hull's forward axis (pose row 2), which is the quantity milestone 2r's
    // section 2 table reports for both the executable and the probe.
    {
        const double planar = std::sqrt(dx * dx + dz * dz);
        slot.row.trajectory_speed = (step_seconds > 0.0f)
            ? static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz)
                / static_cast<double>(step_seconds))
            : 0.0f;
        if (planar > 1.0e-6) {
            const double fx = slot.motion.pose_row2[0];
            const double fz = slot.motion.pose_row2[2];
            const double forward = std::sqrt(fx * fx + fz * fz);
            if (forward > 1.0e-6) {
                double cosine = (dx * fx + dz * fz) / (planar * forward);
                if (cosine > 1.0) cosine = 1.0;
                if (cosine < -1.0) cosine = -1.0;
                slot.row.drift_degrees
                    = static_cast<float>(std::acos(cosine) * 180.0 / 3.14159265358979323846);
                if (slot.row.drift_degrees > slot.row.peak_drift_degrees) {
                    slot.row.peak_drift_degrees = slot.row.drift_degrees;
                }
            }
        }
    }
    slot.row.command_applied = result.gate.command_applies;
    if (!host.logged_gate) {
        host.logged_gate = true;
        // 00826994: the command is suppressed when the keel sample point has
        // risen above half the local wave height. With the flat-sea stand-in
        // that is a test of the authored hull height against zero, so a
        // reader can tell a closed gate from a missing class row.
        host.log.notef("ship motion gate on \"%s\": keel=(%.2f, %.2f, %.2f) wave=%.2f "
            "applies=%d throttle=%.3f target_speed=%.3f engine_gate=%.1f",
            slot.row.name.c_str(), static_cast<double>(result.keel_point.x),
            static_cast<double>(result.keel_point.y),
            static_cast<double>(result.keel_point.z),
            static_cast<double>(result.wave_height),
            result.gate.command_applies ? 1 : 0,
            static_cast<double>(result.gate.throttle),
            static_cast<double>(result.target_speed),
            static_cast<double>(result.engine_gate));
    }
    ++slot.row.motion_ticks;
    ++host.summary.motion_ticks;
    host.done("World::unit_motion_tick", 0x00825f20u);
    host.refresh_row(slot);
}

// Packet cc9_hull_hull_contact (docs/GUNNERY_OPEN_ITEMS.md 91). 00C5C540 once for the world
// after every ship's tick and velocity phase: 00C5BB30's contact phase over every hull
// (HullTerrainContactSolver::world_step: ManifoldUpdate, the terrain narrow phase, the
// hull pairs with kHullHullContactBound, 00C4B610's groups, one solve per group), then each
// ship's position phase and the rest of its tick in unit order.
void GameUnitsHost::Impl::run_world_contact_phase(float step_seconds) {
    // Packet cc9_avoid_zone_draft_bodies: 00424DDF..00425487 builds the draft bodies at
    // mission load, after the zones; here once, at the first world phase after the ship
    // AI's avoid-zone geometry is loaded (LABELLED: the build time; the bodies are static).
    // The session is single player (game+1FE4h = 0, game_hosts_mission.cpp), so the
    // depths are AvoidZoneDepthsSingle's (00837DE0).
    if (!hull_terrain.draft_bodies_set() && ship_ai != nullptr) {
        if (const bsp::game::GameAvoidZoneRuntime* zones = ship_ai->avoid_zone_runtime()) {
            std::array<std::int32_t, 9> depth{};
            std::string error;
            if (!lua.read_avoid_zone_draft_depths(0, depth, error))
                throw std::runtime_error("Avoid-zone draft depths: " + error);
            const bsp::AvoidZoneDraftDepthInputs inputs{depth[0], depth[1], depth[2],
                depth[3], depth[4], depth[5], depth[6], depth[7], depth[8]};
            const std::vector<bsp::AvoidZoneDraftBody> built =
                bsp::avoid_zone_draft_bodies_00424ddf(zones->table(),
                    [zones](std::size_t g) {
                        return zones->native_group(zones->table().groups.at(g));
                    },
                    inputs, application_camera_axes_crt(), draft_build);
            std::vector<bsp::DraftWorldEntry> drafts;
            for (const bsp::AvoidZoneDraftBody& b : built) {
                bsp::DraftWorldEntry e;
                e.layer = b.layer;
                e.group = b.group;
                for (int k = 0; k < 12; ++k) e.frame[k] = b.frame[k];
                e.hull = b.hull;
                drafts.push_back(std::move(e));
            }
            hull_terrain.set_draft_bodies(std::move(drafts), bsp::kAvoidZoneDraftBodiesBound);
            log.notef("avoid-zone draft bodies: depths=%d,%d,%d,%d,%d,%d,%d,%d,%d pairs=%zu "
                "groups_missing=%zu zones=%zu zones_short=%zu pieces=%zu pieces_short=%zu "
                "hulls_rejected=%zu bodies=%zu bound=%d (00424DDF..00425487 / 00423C50, packet "
                "cc9_avoid_zone_draft_bodies)", depth[0], depth[1], depth[2], depth[3],
                depth[4], depth[5], depth[6], depth[7], depth[8], draft_build.pairs,
                draft_build.groups_missing, draft_build.zones, draft_build.zones_short,
                draft_build.pieces, draft_build.pieces_short, draft_build.hulls_rejected,
                draft_build.bodies, bsp::kAvoidZoneDraftBodiesBound ? 1 : 0);
        }
    }
    std::vector<bsp::HullWorldEntry> hulls;
    for (const WorldContactPending& pending : world_contact_pending) {
        GameUnitSlot& slot = *slots[pending.index];
        const ClassHullBox& hb = class_hull_box(slot);
        if (!hb.ok || hb.box.shape_points.empty()) continue;
        hulls.push_back(bsp::HullWorldEntry{pending.index, &slot.body, &hb.box.shape_points,
            bsp::ship_physics_material_shipped(slot.hull_material).friction, {},
            bsp::kSunkHullTerrainMaskBound && slot.sunk_terrain_mask_cleared});
        // 009394E5..009395E2: the class kind [[ctl+1Ch]+538h]->vtable[1Ch]() and, for a
        // landing ship, BigLandingShip (class+808h). LABELLED: a cruiser's +808h
        // (HeavyCruiser) is read as 0 (no host field); both cruiser bits are in the
        // single-player layer's mask 3EF0h, so no single-player pair depends on it.
        if (!slot.hull_class_bit_taken) {
            slot.hull_class_bit_taken = true;
            const bool big = slot.class_id == 0x0C && ship_ai != nullptr &&
                ship_ai->unit_big_landing_ship_0808(pending.index);
            slot.hull_class_bit = bsp::hull_class_bit_009394fb(slot.class_id, big);
        }
        hulls.back().class_bit = slot.hull_class_bit_cleared ? 0u : slot.hull_class_bit;
    }
    // Packet cc9_hull_fort_contact: every MLandFort and MCommandBuilding (kind 1Bh) with a
    // placed world matrix and a shape has one static body (bsp::kHullFortContactBound; OFF
    // the fort pairs are a census only).
    std::vector<bsp::FortWorldEntry> forts;
    for (std::size_t i = 0; i < slots.size(); ++i) {
        const GameUnitSlot& s = *slots[i];
        if (!bsp::unit_is_kind_of(s.class_id, bsp::kUnitKindQueryLandStructure)) continue;
        auto found = fort_bodies.find(i);
        if (found == fort_bodies.end()) {
            if (s.world_valid == 0) continue;
            const ClassHullBox& fb = class_fort_box(s);
            if (!fb.ok || fb.box.all_shape_points.empty()) continue;
            bsp::FortWorldEntry e;
            e.unit = i;
            e.shapes = &fb.box.all_shape_points;
            for (int k = 0; k < 3; ++k) {
                e.frame[k] = s.world[k];
                e.frame[3 + k] = s.world[4 + k];
                e.frame[6 + k] = s.world[8 + k];
                e.frame[9 + k] = s.world[12 + k];
            }
            found = fort_bodies.emplace(i, e).first;
        }
        forts.push_back(found->second);
    }
    hull_terrain.world_step(hulls, forts, step_seconds, bsp::kHullHullContactBound,
        bsp::kHullFortContactBound);
    for (const bsp::HullWorldEntry& e : hulls) {
        if (slots[e.unit]->sunk_terrain_mask_cleared && e.result.contact) ++sunk_wreck_terrain_steps;
        record_hull_terrain_result(*slots[e.unit], e.unit, e.result);
    }
    for (const WorldContactPending& pending : world_contact_pending) {
        finish_motion_tick(*slots[pending.index], pending.before, pending.result,
            step_seconds);
    }
    world_contact_pending.clear();
}

// ---------------------------------------------------------------------------
// Milestone 2n: what the ship AI controller reads off a unit
// ---------------------------------------------------------------------------

GameShipAiHost* GameUnitsHost::ship_ai() noexcept { return impl_->ship_ai; }

void GameUnitsHost::set_hull_class_bit_0092bd70(std::size_t unit_index, bool on) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return;
    // 0092BD70: the bit from the class kind (as at 009394FB), then 00C47F60(bit) or
    // 00C47F90(bit) over the hull's shape chain (00C31DC0, next +208h).
    host.slots[unit_index]->hull_class_bit_cleared = !on;
    ++(on ? host.hull_class_bit_sets : host.hull_class_bit_clears);
    host.done("Units::set_hull_class_bit", 0x0092bd70u);
}

void GameUnitsHost::set_ship_ai(GameShipAiHost* ai) noexcept {
    impl_->ship_ai = ai;
    if (impl_->gunnery != nullptr) impl_->gunnery->set_ship_ai(ai);
}

GameGunneryHost* GameUnitsHost::gunnery() noexcept { return impl_->gunnery.get(); }
const GameGunneryHost* GameUnitsHost::gunnery() const noexcept {
    return impl_->gunnery.get();
}

std::uint32_t GameUnitsHost::director_current_command_0071be40(std::size_t index) const {
    return impl_->commands.current_command_0071be40(index);
}

bool GameUnitsHost::director_avoidance(std::size_t index, GameDirectorAvoidance& out) const {
    return impl_->commands.director_avoidance(index, out);
}

bool GameUnitsHost::apply_director_avoidance_message_00835640(std::size_t index,
    const bsp::DirectorCommandMessage& message) {
    return impl_->commands.apply_director_avoidance_message_00835640(index, message);
}

std::size_t GameUnitsHost::report_command_event_00984300(std::size_t index,
    std::uint32_t command_object, const char* status) {
    return impl_->commands.report_command_event_00984300(index, command_object, status);
}

GameCommandCompletion GameUnitsHost::end_command_0071e430(std::size_t index,
    std::uint32_t command_object, bool terminal) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return GameCommandCompletion{};
    GameUnitSlot& slot = *host.slots[index];
    return host.commands.end_command_0071e430(index, command_object, terminal,
        unit_player_controlled_0184(index), slot.ring, Impl::pose_heading_radians(slot));
}

bool GameUnitsHost::unit_player_controlled_0184(std::size_t index) const {
    const Impl& host = *impl_;
    if constexpr (Impl::kPlayerRoleBookkeepingBound) {
        return index < host.slots.size() && host.slots[index]->role_player_0184;
    } else {
        return host.controlled_bound && host.controlled_index == index;
    }
}

void GameUnitsHost::role_screen_update_0067bb50() {
    if constexpr (Impl::kPlayerRoleBookkeepingBound) {
        impl_->role_screen_update_0067bb50();
    }
}

void GameUnitsHost::set_role_availability_00927d20(std::size_t index, std::uint32_t mask,
                                                   std::int32_t value) {
    impl_->set_role_availability_00927d20_impl(index, mask, value);
}

void GameUnitsHost::Impl::set_role_availability_00927d20_impl(std::size_t index,
                                                              std::uint32_t mask,
                                                              std::int32_t value) {
    Impl& host = *this;
    if (!Impl::kPlayerRoleBookkeepingBound || index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[index];
    // 00927D20: for each of the nine role bits, unit+188h + i*4 = value. A
    // kind-2 unit whose role is held (not 8) and is now closed to everyone but
    // the AI (value 8) routes a 4Bh release (take 0) naming the holder. The
    // route is delivered locally at once in single player (labelled).
    std::uint32_t bit = 1;
    for (int i = 0; i < bsp::kUnitRoleTableEntries; ++i, bit <<= 1) {
        if ((mask & bit) == 0) continue;
        slot.role_permission_0188[i] = value;
        ++host.role_permission_writes;
        const std::int32_t holder = slot.current_roles_01ac[i];
        if (bsp::unit_is_kind_of(slot.class_id, 2) && holder != 8 && value == 8) {
            host.role_message_4b_00780162(index, bit, holder, 0);
        }
    }
    // 00927DE1..: a value naming one player (not 8, not 9) also calls
    // unit->vtable[2Ch] on that player's record; 0077F360's tail sets a
    // message-suppress byte for kind-5 units. Neither is modelled (labelled).
    // 00927E29..00927E31: 00696350(unit, code 5) notifies the unit's observers.
    // Packet cc13-powerup-bind phase 2a: the power-up manager observes the
    // command buildings 008ED720 granted through (008ED620 regrants them).
    game_powerups::powerup_manager_observed_event_008ed620(index, 5);
}

bool GameUnitsHost::unit_current_role_slot(std::size_t index, std::int32_t role_index,
    std::int32_t& out) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size() || role_index < 0
        || role_index >= bsp::kUnitRoleTableEntries) return false;
    out = host.slots[index]->current_roles_01ac[role_index];
    return true;
}

bool GameUnitsHost::unit_role_permission(std::size_t index, std::int32_t role_index,
    std::int32_t& out) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size() || role_index < 0
        || role_index >= bsp::kUnitRoleTableEntries) return false;
    out = host.slots[index]->role_permission_0188[role_index];
    return true;
}

std::uint32_t GameUnitsHost::unit_role_permission_mask(std::size_t index,
    std::int32_t player_slot) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size() || host.slots[index] == nullptr) return 0;
    UnitRolePermissionReadView view{*host.slots[index]};
    return bsp::cc13_seg49::unit_role_mask_for_00927cc0(
        view.raw(), player_slot);
}

bool GameUnitsHost::unit_role_permission_is_player_or_open(std::size_t index,
    std::int32_t role_index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size() || host.slots[index] == nullptr
        || role_index < 0 || role_index >= bsp::kUnitRoleTableEntries) return false;
    UnitRolePermissionReadView view{*host.slots[index]};
    return bsp::cc13_seg49::unit_role_word_is_player_or_open_00927c90(view.raw(), role_index);
}

void GameUnitsHost::role_request_0077c470(std::size_t index, std::uint32_t mask, bool take) {
    // 0077C470's gate (game+5D4h > 0Ch or game+216Ch, session mode 0) holds
    // in a single-player mission from 004DA73C on, before any caller here.
    if constexpr (Impl::kPlayerRoleBookkeepingBound) {
        impl_->role_message_4b_00780162(index, mask, Impl::kLocalPlayerSlot, take ? 1 : 0);
    }
}

bool GameUnitsHost::add_hull_torque_message_93h(std::size_t index,
                                                const bsp::OceanVec3& torque) {
    if (index >= impl_->slots.size()) return false;
    GameUnitSlot& s = *impl_->slots[index];
    if (s.body.motion == nullptr) return false;
    const bsp::UnitHullTorqueMessage m{torque};
    return bsp::unit_handle_add_hull_torque_00822235(m, s.body);
}

void GameUnitsHost::store_unit_command_target(std::size_t index,
                                              std::size_t target_plus_one) noexcept {
    if (index >= impl_->slots.size()) return;
    impl_->slots[index]->command_target_plus_one = target_plus_one;
}

void GameUnitsHost::store_unit_attack_command_class(std::size_t index,
                                                    unsigned int cls) noexcept {
    if (index >= impl_->slots.size()) return;
    impl_->slots[index]->attack_command_class = cls;
}

bool GameUnitsHost::plane_order_view(std::size_t index, int& state_900, bool& landed_904,
                                     bool& enabled_5c, unsigned int& command_class) const noexcept {
    if (index >= impl_->slots.size() || !impl_->slots[index]) return false;
    const GameUnitSlot& s = *impl_->slots[index];
    state_900 = s.plane_control_mode_900;
    landed_904 = s.plane_landed_904;
    enabled_5c = s.row.active;
    command_class = s.attack_command_class;
    return true;
}

bool GameUnitsHost::plane_launch_spot_arm_9dc(std::size_t index, std::uint32_t& arms,
                                              float& cooldown) const noexcept {
    if (index >= impl_->slots.size() || !impl_->slots[index]) return false;
    const GameUnitSlot& s = *impl_->slots[index];
    arms = s.launch_spot_arms_9dc;
    cooldown = s.launch_spot_cooldown_9dc;
    return true;
}

// 0084BF00's own-base filter (0084C213-0084C278). EBP is the shot's owner
// (shot+CCh) when it IsKindOf(0Fh) (0084BF3F-0084BF66), and [ESP+44h] its
// squadron +9D4h. With a squadron, a hit and a hit entity: a plane whose +908h
// is under 2.0 (00CE3958), with a holder +BF4h and +C18h > 0, drops the hit
// when the entity is the holder's base owner ([[+BF4h]+4]+7Ch); then
// +C18h = 5. Otherwise (0084C251) +C18h > 0 and (+72Ch)->vtable[38h] (free
// flight: 0074E210 BSP_PlaneControlMode_IsFreeFlight, `cmp [ecx+1D4h],7`, and
// +72Ch + 1D4h = +900h, so +900h == 7) count it down by one.
// PROVISIONAL: [[+BF4h]+4]+7Ch is taken as landing_decks[+BF4h - 1].owner, the
// same holder -> block -> owner substitution the stock return uses
// ([[[plane+BF4h]+4]+7Ch]+174h, packet cc9_stowed_plane_stock_return).
// LABELLED: a landscape hit (hit with hit_unit == -1 here; hit_unit 0 on the
// projectile sweep) is treated as an entity present, so it takes the hit arm
// and never counts down. That matches the image only if 0098B370 returns a
// non-null hit entity for land, which is not shown.
int GameUnitsHost::plane_own_base_hit_filter_0084c213(std::size_t shooter, bool hit,
                                                      std::size_t hit_unit) noexcept {
    if (shooter >= impl_->slots.size() || !impl_->slots[shooter]) return 0;
    GameUnitSlot& s = *impl_->slots[shooter];
    if (!bsp::unit_is_kind_of(s.class_id, 0x0f)) return 0;
    const bool squadron =
        bsp::plane_squadron_registry().find_by_member_unit(shooter) != nullptr;
    if (squadron && hit) {
        // 0084C213-0084C24F (a hit entity that is not a unit never matches).
        if (hit_unit == static_cast<std::size_t>(-1)) return 0;
        if (!(2.0f > s.plane_airborne_908)) return 0;
        const std::size_t hi = s.plane_contact_deck_bf4;
        if (hi == 0 || hi > impl_->landing_decks.size()) return 0;
        if (s.plane_c18 <= 0) return 0;
        if (impl_->landing_decks[hi - 1u].owner != hit_unit) return 0;
        s.plane_c18 = 5;
        return 1;
    }
    // 0084C251-0084C273.
    if (s.plane_c18 <= 0 || s.plane_control_mode_900 != 7) return 0;
    --s.plane_c18;
    return 2;
}

void GameUnitsHost::store_unit_moveto_range(std::size_t index, float range) noexcept {
    if (index >= impl_->slots.size()) return;
    impl_->slots[index]->moveto_range = range;
}

void GameUnitsHost::store_unit_moveto_target(std::size_t index,
                                             std::size_t target_index) noexcept {
    if (index >= impl_->slots.size()) return;
    auto& unit = *impl_->slots[index];
    if (target_index >= impl_->slots.size()) {
        // Point orders use the point-target profile, whose slot0 returns
        // null. This host retains that existing point separately.
        unit.moveto_entity_target.reset();
        unit.moveto_entity_44 = nullptr;
        unit.moveto_entity_refresh_pending = false;
        return;
    }
    GameMoveToEntityPoseHost pose;
    auto& target = unit.moveto_entity_target.emplace();
    bsp::move_to_target_construct_from_entity_009be0f0(
        pose, &target, impl_->slots[target_index].get());
    // bsp.py show 009c1c30: target slot0 supplies the observed entity at
    // approach+44h, then target+4/8/C initialize approach+48/4C/50h.
    // Its immediate 009BEBA0 replaces Y with cruise/path altitude. A
    // reused task has no constructor refresh, so do not expose raw target
    // height here; force that full refresh before its next steering tick.
    unit.moveto_entity_44 = static_cast<GameUnitSlot*>(
        bsp::move_to_target_entity_009be140(&target));
    bsp::stage_move_to_entity_point(target, unit.moveto_point,
        unit.moveto_entity_refresh_pending);
}

void GameUnitsHost::store_unit_moveto_point(std::size_t index,
                                            const float world[3]) noexcept {
    if (index >= impl_->slots.size()) return;
    // x and z only: 009BEBDB sets the height from the cruise profile each refresh.
    impl_->slots[index]->moveto_point[0] = world[0];
    impl_->slots[index]->moveto_point[2] = world[2];
}

// Documented deviation (cc13-D3): 007C9540 -> 004134F0 copies the whole 4x4 to +74h
// and +674h (translation included), clears +C8h / +10Ch, invalidates the child chain
// (0042ED50) and re-syncs (plane+310h) vtable[0Ch]. This takes the three rotation
// rows and republishes the pose (the +310h re-sync folded in). The dropped
// translation is unobservable: every caller passes the unit's own position (the
// squadron arm loads each member's +FCh..+104h at 008A0D9B..008A0DB7; the
// single-entity caller sets m[12..14] to the position).
bool GameUnitsHost::set_unit_world_basis_007c9540(std::size_t index, const float right[3],
    const float up[3], const float forward[3]) noexcept {
    if (index >= impl_->slots.size()) return false;
    GameUnitSlot& slot = *impl_->slots[index];
    for (int i = 0; i < 3; ++i) {
        slot.motion.pose_row0[i] = right[i];
        slot.motion.pose_row1[i] = up[i];
        slot.motion.pose_row2[i] = forward[i];
    }
    Impl::publish_pose(slot);
    impl_->done("Plane::set_world_matrix_007c9540", 0x007c9540u);
    return true;
}

std::size_t GameUnitsHost::place_squadron_members_007f2920(std::size_t index) {
    return impl_->place_squadron_members_007f2920(index, "put_to");
}

void GameUnitsHost::store_unit_ordnance(std::size_t index, std::uint64_t mask) noexcept {
    if (index >= impl_->slots.size()) return;
    if constexpr (Impl::kRackUnequippedEmptyBound && bsp::kPlaneSceneEquipmentBound) {
        // 0095A880(null, 1, 0): no rack round, so no rack-borne kind (5ey).
        if (impl_->slots[index]->bag_equipment == 0) {
            const auto bit = [](int kind) { return std::uint64_t(1) << (kind - 0x08); };
            mask &= ~(bit(0x2A) | bit(0x2B) | bit(0x2C) | bit(0x2D) | bit(0x31) | bit(0x33));
        }
    }
    impl_->slots[index]->ordnance_mask = mask;
}

bool GameUnitsHost::unit_has_rocket_ordnance_007b9480(std::size_t index) {
    if (index >= impl_->slots.size() || !impl_->slots[index]) return false;
    const bool has = impl_->plane_has_big_rocket_007b9480(*impl_->slots[index]);
    ++impl_->rocket_feed_queries_;
    if (has) ++impl_->rocket_feed_true_;
    return has;
}

std::uint64_t GameUnitsHost::unit_ordnance(std::size_t index) const noexcept {
    if (index >= impl_->slots.size() || !impl_->slots[index]) return 0;
    const GameUnitSlot& u = *impl_->slots[index];
    std::uint64_t mask = u.ordnance_mask;
    if constexpr (Impl::kRackLiveOrdnanceMaskBound) {
        // 007B9320 / 007B91C0 -> rack vtable[220h], 007B9140 -> rack vtable[210h]
        // (007B9174, 006E3FE0): an emptied rack has no child round to answer with
        // (SQUADRON_LAND_TASK 5eu). CORRECTION: this paired 007B91C0 with [210h];
        // 007B91C0 calls [220h] at 007B91E4.
        const std::size_t n = u.rack_bullet_class_per_rack.size();
        // With EnableDeviceReload (00E17BF2) set and the squadron's ReloadEnabled
        // +369h (default 1), 006E4060's second arm answers from the rack's own
        // bullet descriptor whatever is attached, so the static kinds stand.
        if (!lua_device_reload_enabled_00e17bf2()
            && u.rack_census_done && u.rack_multi_count == 0 && u.rack_ammo >= 0 && n != 0
            && u.rack_ammo_per_rack.size() == n) {
            const auto bit = [](int kind) { return std::uint64_t(1) << (kind - 0x08); };
            const auto bomb_family = [](int kind) {
                if constexpr (bsp::kSquadronAmmoDummyKindsCc13Bound) {
                    // Packet cc13-ammodk: the 2Ah bit's reader is 007B9140's walk
                    // (007B917B PUSH 2Ah): rack vt[210h] = 006E3FE0 asks each child
                    // round entity vt[5Ch](2Ah) (006E3FF2), and DummyTarget's entity
                    // test 00700AC0 answers 2Eh/2Dh/2Ah (00700AC4..00700AD1), so a
                    // loaded 2Eh rack counts toward 2Ah. CORRECTION: not 007B9320,
                    // which rejects 2Bh/2Ch/31h/33h that this family counts.
                    return kind == 0x2A || kind == 0x2B || kind == 0x2C || kind == 0x2D
                        || kind == 0x2E || kind == 0x31 || kind == 0x33;
                } else {
                    return kind == 0x2A || kind == 0x2B || kind == 0x2C || kind == 0x2D
                        || kind == 0x31 || kind == 0x33;
                }
            };
            bool known = true;
            bool family_loaded = false;
            for (std::size_t i = 0; i < n; ++i) {
                const int k = u.rack_bullet_class_per_rack[i];
                if (k < 0x08 || k > 0x3F) { known = false; break; }
                if (u.rack_ammo_per_rack[i] > 0 && bomb_family(k)) family_loaded = true;
            }
            if (known) {
                for (std::size_t i = 0; i < n; ++i) {
                    const int k = u.rack_bullet_class_per_rack[i];
                    bool loaded = false;
                    for (std::size_t j = 0; j < n; ++j) {
                        if (u.rack_bullet_class_per_rack[j] == k && u.rack_ammo_per_rack[j] > 0)
                            loaded = true;
                    }
                    if (!loaded) mask &= ~bit(k);
                }
                if (!family_loaded) mask &= ~bit(0x2A);
            }
        }
    }
    return mask;
}

// Packet cc13-rackround (include/bsp/racked_round_entity_cc13.hpp). The racked
// rounds GetSquadTorpedoes(squadron, true) lists for one plane: 00856420's rounds
// stay on the world list +21Ch while parented to the rack (+48h), unlaunched
// (entity+458h, 008561F0), until the drop launches them or a Kill releases them.
std::size_t GameUnitsHost::racked_torpedo_rounds(std::size_t index, std::vector<int>& racks) {
    racks.clear();
    // rev6 N3: the listing reads rack_ammo_per_rack, which only the per-rack seed fills.
    static_assert(!bsp::kRackedRoundEntityCc13Bound || Impl::kRackRoundsPerRackBound);
    if constexpr (bsp::kRackedRoundEntityCc13Bound) {
        if (index < impl_->slots.size() && impl_->slots[index]) {
            GameUnitSlot& s = *impl_->slots[index];
            // Lua time, ON only: the census and the first-issue seed, each run once
            // (rack_census_done, rack_ammo < 0).
            impl_->plane_rack_census(s);
            impl_->rack_release_issue_seed_007c0de3(s);
            const std::size_t n = std::min(s.rack_ammo_per_rack.size(),
                                           s.rack_bullet_class_per_rack.size());
            for (std::size_t i = 0; i < n; ++i) {
                if (s.rack_bullet_class_per_rack[i] == 0x2B && s.rack_ammo_per_rack[i] > 0)
                    racks.push_back(static_cast<int>(i));
            }
        }
    }
    return racks.size();
}

// 008AC5C0 Kill -> 00926D90 on a racked round: the round leaves the rack and none
// is attached again (no single-rack observer; vtable[224h] only from 006E3350,
// 006E3410, the drop tail and two messages). LABELLED: the rack's +484h reserve
// is stored as 0 with it, since the host keeps one count per rack.
bool GameUnitsHost::kill_racked_round(std::size_t index, int rack, int& before) {
    bool killed = false;
    before = 0;
    if constexpr (bsp::kRackedRoundEntityCc13Bound) {
        const std::size_t i = static_cast<std::size_t>(rack);
        if (index < impl_->slots.size() && impl_->slots[index] && rack >= 0) {
            GameUnitSlot& s = *impl_->slots[index];
            if (i < s.rack_ammo_per_rack.size() && i < s.rack_bullet_class_per_rack.size()
                && s.rack_bullet_class_per_rack[i] == 0x2B && s.rack_ammo_per_rack[i] > 0
                && s.rack_ammo >= 0) {
                before = s.rack_ammo_per_rack[i];
                s.rack_ammo_per_rack[i] = 0;
                // Kept >= 0 so no `rack_ammo < 0` seed runs again over the kill.
                s.rack_ammo = std::max(0, s.rack_ammo - before);
                bool torpedo_left = false;
                for (std::size_t k = 0; k < s.rack_ammo_per_rack.size()
                                        && k < s.rack_bullet_class_per_rack.size(); ++k) {
                    if (s.rack_bullet_class_per_rack[k] == 0x2B && s.rack_ammo_per_rack[k] > 0)
                        torpedo_left = true;
                }
                // As a torpedo drop does (game_hosts_gunnery.cpp, cc8_torpedo_breakoff),
                // but unlike the drop, only when no 2Bh rack is left; the direct write
                // equals store_unit_ordnance (rev6 N2). 007C1D80's rearm sets it again.
                const std::uint64_t torpedo_bit = std::uint64_t(1) << (0x2B - 0x08);
                if (!torpedo_left) s.ordnance_mask &= ~torpedo_bit;
                killed = true;
                impl_->log.notef("  racked round killed: %s rack %d, rounds %d -> 0, rack ammo %d, "
                    "2Bh %s (008AC5C0 -> 00926D90, packet cc13-rackround)", s.row.name.c_str(),
                    rack, before, s.rack_ammo, torpedo_left ? "kept" : "cleared");
            }
        }
    }
    return killed;
}

bool GameUnitsHost::unit_flag_005d(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    return host.slots[index]->state->simulate != 0;
}

// ---- packet cc8_ship_follow: the unit group at entity+284h -----------------

std::int32_t GameUnitsHost::unit_formation_group_0284(std::size_t index) const noexcept {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return -1;
    return host.slots[index]->formation_group;
}

std::size_t GameUnitsHost::formation_leader_0014(std::int32_t group) const noexcept {
    const Impl& host = *impl_;
    if (group < 0 || static_cast<std::size_t>(group) >= host.formation_groups.size()) {
        return static_cast<std::size_t>(-1);
    }
    return host.formation_groups[static_cast<std::size_t>(group)].leader;
}

bool GameUnitsHost::place_at_world_position_008193a0(std::size_t index, const float pos[3]) {
    Impl& host = *impl_;
    if (index >= host.slots.size() || host.slots[index] == nullptr) return false;
    GameUnitSlot& slot = *host.slots[index];
    float at[3] = {pos[0], pos[1], pos[2]};
    bool station = false;
    FormationStation st{};
    if (unit_is_formation_follower_007788b0(index)) {
        // 0081941F-00819430: |pos|^2, summed z, x, y and stored as a float, against
        // the double 25.0 at 00CE3880; a far point places nothing at all.
        const float r2 = pos[2] * pos[2] + pos[0] * pos[0] + pos[1] * pos[1];
        if (!kPutToFormationBound || static_cast<double>(r2) > 25.0) {
            host.record("UnitPlace::follower_arms_008193a0", 0x0081945du);
            return false;
        }
        // 0081945D-0081950A: the station, 00810630 on the leader's wake at the
        // record's axial column, pushed along (-dir.z, dir.x) by its lateral
        // column, with y forced to 0 (008194C0).
        st = formation_station_0070d290(index, 1.0f, 1.0f);
        at[0] = st.x;
        at[1] = 0.0f;
        at[2] = st.z;
        station = true;
    }
    // 009583C0's ship arm suppresses ENTITY_RELOCATE regardless of the
    // session. Keep the previous non-ship substitution until its message
    // and device-notification consumers exist; do not invent those owners.
    if (unit_is_kind_of(index, 6)) {
        const auto& session = bsp::session_context();
        const auto plan = bsp::gameplay_seg61_62_cc13::unit_set_world_position_009583c0(
            unit_is_kind_of(index, 6), unit_is_kind_of(index, 0x0f),
            session.session_word_1fe4, static_cast<std::uint8_t>(session.skirmish_218c));
        // For ships, vtable[114h] is the side-effect-free +738h director
        // getter 0080E150 (00CF90B0+114h, also 00CFC3D0+114h). 00958443
        // discards its result, so call_vtable_114 requires no state change.
        if (plan.set_translation_a4) {
            for (int lane = 0; lane < 3; ++lane) slot.motion.position[lane] = at[lane];
        }
        Impl::publish_pose_set_plan(slot, plan);
    } else {
        for (int lane = 0; lane < 3; ++lane) slot.motion.position[lane] = at[lane];
    }
    const float heading = static_cast<float>(
        std::atan2(static_cast<double>(slot.motion.pose_row2[0]),
                   static_cast<double>(slot.motion.pose_row2[2])));
    bsp::ship_ai_wake_fill_00810020(slot.wake, slot.motion.position, heading);   // 00818EA0
    host.done("UnitPlace::set_world_position_008193a0", 0x008193a0u);
    if (station) {
        // 008195C1-00819628: vtable[11Ch](-0.0 - wrap_2pi(pi/2 - atan2(dir.z, dir.x)))
        // turns the follower along the wake direction at its station.
        // The two constants are float values widened to doubles (3FF921FB60000000,
        // 401921FB60000000).
        float angle = static_cast<float>(1.5707963705062866 -   // 00CE3830
            static_cast<double>(static_cast<float>(std::atan2(static_cast<double>(st.dir_z),
                                                              static_cast<double>(st.dir_x)))));
        if (angle < 0.0f) angle = static_cast<float>(static_cast<double>(angle) +
                                                     6.2831854820251465);   // 00CE3828
        ++host.putto_station_places;
        set_heading_008196b0(index, -0.0f - angle);                         // 00D7A208
        return true;
    }
    if (slot.formation_group >= 0 &&
        formation_leader_0014(slot.formation_group) == index) {
        if constexpr (kPutToFormationBound) {
            // 0081963E-00819694: every other member through vtable[118h](&00F87574).
            relocate_formation_members_00819672(index);
        } else {
            host.record("UnitPlace::group_snap_0081963e", 0x0081963eu);
        }
    }
    return true;
}

void GameUnitsHost::relocate_formation_members_00819672(std::size_t leader) {
    Impl& host = *impl_;
    const std::int32_t group = host.slots[leader]->formation_group;
    const std::int32_t count = formation_member_count(group);                 // group+4F8h
    static const float kOrigin[3] = {0.0f, 0.0f, 0.0f};                       // 00F87574
    for (std::int32_t i = 0; i < count; ++i) {
        const std::size_t m = formation_member_unit(group, i);                 // 0070D060
        if (m == leader || m >= host.slots.size() || host.slots[m] == nullptr) continue;
        place_at_world_position_008193a0(m, kOrigin);
    }
} // namespace

bool GameUnitsHost::set_heading_008196b0(std::size_t index, float heading) {
    Impl& host = *impl_;
    if (index >= host.slots.size() || host.slots[index] == nullptr) return false;
    GameUnitSlot& slot = *host.slots[index];
    // LABELLED: only a ship (IsKindOf 6) is turned; a plane's or ground unit's
    // orientation lives outside the motion rows here, so theirs stays a record.
    if (!unit_is_kind_of(index, 6)) {
        host.record("UnitPlace::set_heading_008196b0 (not a ship)", 0x008196b0u);
        return false;
    }
    {
        const auto& session = bsp::session_context();
        const auto plan = bsp::gameplay_seg61_62_cc13::unit_set_heading_009584d0(
            unit_is_kind_of(index, 6), unit_is_kind_of(index, 0x0f),
            session.session_word_1fe4, static_cast<std::uint8_t>(session.skirmish_218c));
        // 00958556 calls 0047F200. Chain 454 runs that existing body on the
        // current ship translation, with native matrix arithmetic providers.
        // The OFF arm retains the former CRT RotationY projection.
        const float yaw = -0.0f - heading;
        if (plan.rotate_heading_0047f200) {
            set_live_ship_scene_heading(slot.motion, heading);
        }
        Impl::publish_pose_set_plan(slot, plan);
        // 008196B0's 00818EA0: the wake ring refilled along the new heading.
        bsp::ship_ai_wake_fill_00810020(slot.wake, slot.motion.position, yaw);
    }
    ++host.putto_headings;
    host.done("UnitPlace::set_heading_008196b0", 0x008196b0u);
    // 0081968F..: an occupant owner (00778890: the group's leader) re-places the
    // others at the origin, so each goes to its station on the new wake.
    if (slot.formation_group >= 0 && formation_leader_0014(slot.formation_group) == index) {
        relocate_formation_members_00819672(index);
    }
    return true;
}

bool GameUnitsHost::put_rel_to_00819840(std::size_t index, std::size_t ref, const float rel[3]) {
    Impl& host = *impl_;
    if (index >= host.slots.size() || host.slots[index] == nullptr ||
        ref >= host.slots.size() || host.slots[ref] == nullptr) {
        return false;
    }
    if (!unit_is_kind_of(index, 6)) {
        host.record("UnitPlace::put_rel_to_00819840 (not a ship)", 0x00819840u);
        return false;
    }
    // 008AA6F5-008AA716: ref's world matrix copied (16 dwords from +0CCh), then
    // 004142E0 on the relative point: each lane ((m0*a + m1*b) + m2*c) + t in the
    // order 00414301..0041435B adds them.
    const auto& rm = host.slots[ref]->motion;
    const float x = rel[0];
    const float y = rel[1];
    const float z = rel[2];
    float at[3];
    at[0] = ((rm.pose_row1[0] * y + rm.pose_row0[0] * x) + rm.pose_row2[0] * z) + rm.position[0];
    at[1] = ((rm.pose_row0[1] * x + rm.pose_row1[1] * y) + rm.pose_row2[1] * z) + rm.position[1];
    at[2] = ((rm.pose_row0[2] * x + rm.pose_row1[2] * y) + rm.pose_row2[2] * z) + rm.position[2];
    if (!unit_is_kind_of(index, 8)) at[1] = 0.0f;                              // 008AA784
    float rows[3][3];
    for (int lane = 0; lane < 3; ++lane) {
        rows[0][lane] = rm.pose_row0[lane];
        rows[1][lane] = rm.pose_row1[lane];
        rows[2][lane] = rm.pose_row2[lane];
    }
    GameUnitSlot& slot = *host.slots[index];
    {
        const auto plan = bsp::gameplay_seg61_62_cc13::unit_set_world_matrix_009585b0();
        if (plan.copy_matrix_004134f0) {
            // The actual reference unit's matrix and transformed scene/script
            // point above supply all rows; publish_pose supplies the unchanged
            // homogeneous column of this rigid matrix.
            for (int lane = 0; lane < 3; ++lane) {
                slot.motion.pose_row0[lane] = rows[0][lane];
                slot.motion.pose_row1[lane] = rows[1][lane];
                slot.motion.pose_row2[lane] = rows[2][lane];
                slot.motion.position[lane] = at[lane];
            }
        }
        Impl::publish_pose_set_plan(slot, plan);
    }
    const float heading = static_cast<float>(
        std::atan2(static_cast<double>(slot.motion.pose_row2[0]),
                   static_cast<double>(slot.motion.pose_row2[2])));
    bsp::ship_ai_wake_fill_00810020(slot.wake, slot.motion.position, heading);   // 00818EA0
    host.done("UnitPlace::put_rel_to_00819840", 0x00819840u);
    return true;
}

std::size_t GameUnitsHost::reshape_formation_0070efd0(std::size_t index, int shape) {
    Impl& host = *impl_;
    if (index >= host.slots.size() || host.slots[index] == nullptr) return 0;
    const std::int32_t g = host.slots[index]->formation_group;
    if (g < 0 || static_cast<std::size_t>(g) >= host.formation_groups.size()) return 0;
    Impl::FormationGroup& group = host.formation_groups[static_cast<std::size_t>(g)];
    struct Settings final : bsp::ShipAiFormationReshapeHost {
        Impl& h;
        explicit Settings(Impl& impl) : h(impl) {}
        float game_settings_formation_ship_dist_00424c40() override {
            return h.formacio_ship_dist_42c();
        }
    } settings{host};
    const std::size_t written = bsp::ship_ai_formation_reshape(settings, group.members.data(),
        static_cast<std::int32_t>(group.members.size()),
        static_cast<std::uint32_t>(group.leader + 1u),
        static_cast<bsp::ShipAiFormationShape>(shape));
    ++host.putto_reshapes;
    host.done("UnitGroup::apply_formation_shape_0070efd0", 0x0070efd0u);
    return written;
}

bool GameUnitsHost::disable_physics_00891380(std::size_t index) {
    Impl& host = *impl_;
    if (index >= host.slots.size() || host.slots[index] == nullptr) return false;
    GameUnitSlot& slot = *host.slots[index];
    // unit+1018h is a ship's force controller; the host models it with the
    // hydrodynamics of a ship that has buoyancy elements.
    if (slot.buoyancy_elements.empty()) return false;
    slot.controller_disabled_14 = true;                          // 0089149E
    host.done("Unit::disable_physics_00891380", 0x00891380u);
    return true;
}

bool GameUnitsHost::local_matrix_0074(std::size_t index, float out[16]) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size() || host.slots[index] == nullptr) return false;
    for (int k = 0; k < 16; ++k) out[k] = host.slots[index]->local[static_cast<std::size_t>(k)];
    return true;
}

// 006E00A0 (slot 88h of the unit vtables, docs/ENTITY_LOCAL_MATRIX.md): 004134F0
// copies the matrix into +74h, +C8h and +10Ch are cleared, the children are
// invalidated through 0042ED50, then (entity+310h)->vtable[0Ch]() (006E00E3).
// The host's pose rows are the motion state, so the matrix goes there and is
// published. LABELLED: the +310h notification is unread and not reproduced.
bool GameUnitsHost::set_local_matrix_006e00a0(std::size_t index, const float m[16]) {
    Impl& host = *impl_;
    if (index >= host.slots.size() || host.slots[index] == nullptr) return false;
    GameUnitSlot& slot = *host.slots[index];
    {
        // 00C336C0..00C3370A (bsp.py show 00c336c0 --asm): 4x4 -> body rows,
        // with twelve ordered FLD/FSTP copies. 008A0E4F and 008A130F use it
        // for the scripted ship turns that reach this setter. The host's
        // physics and entity pose share motion, so consume the converted
        // basis/position there instead of inventing a second physics body.
        // This replaces the old extraction; no added float arithmetic (pc24).
        float compact[12];
        bsp::cc13_dyn1::transform_from_4x4_00c336c0(compact, m);
        for (int i = 0; i < 3; ++i) {
            slot.motion.pose_row0[i] = compact[i];
            slot.motion.pose_row1[i] = compact[3 + i];
            slot.motion.pose_row2[i] = compact[6 + i];
            slot.motion.position[i] = compact[9 + i];
        }
    }
    Impl::publish_pose(slot);
    host.done("Entity::set_local_matrix_006e00a0", 0x006e00a0u);
    host.record("Entity::local_matrix_notice_310_vtable0c", 0x006e00e3u);
    return true;
}

bool GameUnitsHost::set_formation_member_offset_0070d080(std::size_t leader,
    std::size_t member, int column, float lateral, float axial) {
    Impl& host = *impl_;
    if (leader >= host.slots.size() || column < 0
        || column >= static_cast<int>(bsp::kShipAiUnitGroupColumnCount)) return false;
    const std::int32_t g = host.slots[leader]->formation_group;
    if (g < 0 || static_cast<std::size_t>(g) >= host.formation_groups.size()) return false;
    Impl::FormationGroup& group = host.formation_groups[static_cast<std::size_t>(g)];
    // 0070D081..0070D0A1: a linear search of group+18h for the entity, stride 34h.
    const std::uint32_t handle = static_cast<std::uint32_t>(member + 1u);
    bsp::ShipAiUnitGroupMember* record = nullptr;
    if (group.native_members->capture_members(group.members)) {
        // bsp.py show 0070D080 and 00A11690 @00A1193E: the first
        // matching record controls the formation-offset write. Refresh
        // the live records, then map the returned record to its mutable
        // host owner; no registry identity is dereferenced as a pointer.
        const void* const found = bsp::native_unit_group_member_record_0070d080(
            group.native_members->data(), reinterpret_cast<const void*>(handle));
        if (found != nullptr) {
            const auto offset = static_cast<const std::byte*>(found)
                - static_cast<const std::byte*>(group.native_members->data()) - 0x18;
            record = &group.members[static_cast<std::size_t>(offset) / 0x34u];
        }
    } else {
        // Host-only groups beyond the native 24-record capacity retain
        // the existing vector search; never enter a raw body out of range.
        for (bsp::ShipAiUnitGroupMember& candidate : group.members) {
            if (candidate.entity == handle) { record = &candidate; break; }
        }
    }
    if (record == nullptr) return false;
    record->lateral[static_cast<std::size_t>(column)] = lateral;
    record->axial[static_cast<std::size_t>(column)] = axial;
    return true;
}

bool GameUnitsHost::Impl::ship_unit(std::size_t u) const {
    return u < slots.size() && slots[u]->motion_dispatch.runs_ship_base();
}

void GameUnitsHost::leave_group_on_destroy_0077bd70(std::size_t index) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[index];
    const std::int32_t g = slot.formation_group;
    if (g < 0 || static_cast<std::size_t>(g) >= host.formation_groups.size()) return;
    if constexpr (kDeadLeaderDirectorStateCopyBound) {
        // Packet cc13-jm08. A dead leader's destroy 77h (0077D1A0) is ahead of
        // any later leave in the image's FIFO (0076E520 / 0076C600; a Lua
        // 00899EB0 / 0089A020 leave posts through 0077C980 too), so its whole
        // 0077BD70 runs first, while it still leads. Here that leave is still
        // pending, and run first, only while `posted` (HitDrainAfterPump, its
        // entry not yet delivered). A Lua leave that meets it is posted behind it
        // (post_formation_leave_0077c980), so this is a backstop for the
        // synchronous leaves that remain: the ship_ai follower release and a Lua
        // leave whose post failed. The copy then runs before the successor's 5Dh,
        // unlike the image. A BeforePump ship has left at its kill; an Unmapped
        // kill's leave waits for the flush and is not pulled forward (see
        // DeadLeaderDeathPhase).
        const std::size_t lead = host.formation_groups[static_cast<std::size_t>(g)].leader;
        if (lead != index && lead < host.slots.size()
            && host.slots[lead]->formation_group == g
            && host.slots[lead]->director_handover_posted) {
            leave_group_on_destroy_0077bd70(lead);
        }
    }
    Impl::FormationGroup& fg = host.formation_groups[static_cast<std::size_t>(g)];
    const std::size_t none = static_cast<std::size_t>(-1);
    const std::size_t leader_before = fg.leader < host.slots.size() ? fg.leader : none;
    // 0077BDA5..: the successor, asked twice when it answers the unit itself.
    std::size_t successor = host.group_successor_0070d8d0(fg, index);
    if (successor == index) successor = host.group_successor_0070d8d0(fg, index);
    // vtable[114h]->vtable[5Ch](successor): the unit's director is told.
    host.record("UnitGroup::leave_director_notice_vtable114", 0x0077bde4u);
    if constexpr (kDeadLeaderDirectorStateCopyBound) {
        // 0071C150, before the SetLeader below. A kill that already ran it
        // (leader_death_handover_at_kill) is not repeated; one that only posted
        // it is run here, and the posted entry then finds nothing to do.
        if (slot.director_handover_ran) {
            slot.director_handover_ran = false;
        } else {
            slot.director_handover_posted = false;
            if (successor != none && fg.leader == index) {
                host.director_handover_0071c150(index, successor);
            }
        }
    }
    // 0077BE0x: 0070D0C0 SetLeader(ship(successor), ship(unit)) when it differs.
    if (successor != none && fg.leader != successor) {
        const std::size_t new_leader = host.ship_unit(successor) ? successor : none;
        const std::size_t old_leader = host.ship_unit(index) ? index : none;
        if (new_leader != old_leader) {
            if (old_leader != none && new_leader != none) {
                host.inherit_leader_wake_00815e20(new_leader, old_leader);
            }
            fg.leader = new_leader;                               // group+14h
        }
    }
    // 0070E4C0 DetachMember(group, unit, 0). Its own re-election runs only for a
    // unit that still leads a group of more than one.
    if (fg.leader == index && fg.members.size() > 1) {
        std::size_t s = host.group_successor_0070d8d0(fg, index);
        if (s == none) {
            for (const bsp::ShipAiUnitGroupMember& m : fg.members) {
                if (m.entity != 0 && static_cast<std::size_t>(m.entity) - 1u != index) {
                    s = static_cast<std::size_t>(m.entity) - 1u;
                    break;
                }
            }
        }
        const std::size_t old_ship = host.ship_unit(index) ? index : none;
        const std::size_t new_ship = (s != none && host.ship_unit(s)) ? s : none;
        if (new_ship != old_ship) {
            if (old_ship != none && new_ship != none) {
                host.inherit_leader_wake_00815e20(new_ship, old_ship);
            }
            fg.leader = new_ship;
        }
    }
    slot.formation_group = -1;                                    // 0070E54x, +284h
    host.unregister_formation_observer(fg, index);
    for (std::size_t i = 0; i < fg.members.size(); ++i) {       // 0070E591 compaction
        if (fg.members[i].entity == static_cast<std::uint32_t>(index + 1u)) {
            fg.members.erase(fg.members.begin() + static_cast<std::ptrdiff_t>(i));
            break;
        }
    }
    if (fg.members.empty()) {                                    // vtable[0](1)
        fg.leader = none;
        ++host.formation_groups_emptied;
    }
    if constexpr (kDeadLeaderRepublishesFollowerPairBound) {
        // Packet cc13-jm08. 007788B0 / 007788D0 read unit+284h and group+14h live
        // in the image; the commands host holds a pushed copy, refreshed here.
        // The leaving unit: 0070E54x cleared its +284h.
        host.commands.set_unit_formation(index, false, none);
        // Each remaining member: 007788B0 = (group+14h != unit), 007788D0 = group+14h.
        for (const bsp::ShipAiUnitGroupMember& m : fg.members) {
            if (m.entity == 0) continue;
            const std::size_t member = static_cast<std::size_t>(m.entity) - 1u;
            host.commands.set_unit_formation(member, fg.leader != member, fg.leader);
        }
    }
    ++host.formation_death_leaves;
    host.done("UnitGroup::leave_on_destroy_0077bd70", 0x0077bd70u);
    host.log.notef("formation leave: unit=%s group=%d t=%.2f leader %s -> %s members=%zu "
        "column=%d (0077D1A0 -> 77h -> 0077BD70 -> 0070E4C0)", slot.row.name.c_str(), g,
        static_cast<double>(host.summary.simulated_seconds),
        leader_before < host.slots.size() ? host.slots[leader_before]->row.name.c_str() : "-",
        fg.leader < host.slots.size() ? host.slots[fg.leader]->row.name.c_str() : "-",
        fg.members.size(), fg.column);
}

void GameUnitsHost::leader_death_handover_at_kill(std::size_t index,
                                                  bsp::DeadLeaderDeathPhase phase) {
    // Packet cc13-jm08. The image delivers 0077D1A0's 77h (the whole 0077BD70)
    // at the first pump after the kill, through the FIFO 0076E520 / 0076C600;
    // this host's leave otherwise runs at the flush. Called by the gunnery kill BEFORE
    // its 0071DDB0 delivery, so the successor's 5Dh lands where the image's does
    // (kDeadLeaderDirectorStateCopyBound has the phase table).
    if constexpr (!kDeadLeaderDirectorStateCopyBound) {
        static_cast<void>(index);
        static_cast<void>(phase);
    } else {
        Impl& host = *impl_;
        if (index >= host.slots.size() || phase == bsp::DeadLeaderDeathPhase::Unmapped) return;
        GameUnitSlot& slot = *host.slots[index];
        if (slot.formation_group < 0 || slot.director_handover_ran
            || slot.director_handover_posted) return;
        const auto run = [&host, index]() {
            const std::int32_t g = host.slots[index]->formation_group;
            if (g < 0 || static_cast<std::size_t>(g) >= host.formation_groups.size()) return;
            Impl::FormationGroup& fg = host.formation_groups[static_cast<std::size_t>(g)];
            // 0077BDA5..0077BDDC: the successor, asked twice when it answers the unit.
            std::size_t successor = host.group_successor_0070d8d0(fg, index);
            if (successor == index) successor = host.group_successor_0070d8d0(fg, index);
            const bool copied = successor != static_cast<std::size_t>(-1)
                && host.director_handover_0071c150(index, successor);
            host.log.notef("leader handover: %s -> %s t=%.2f copied=%d (0071C150 at the "
                "image's pump, packet cc13-jm08)", host.slots[index]->row.name.c_str(),
                successor < host.slots.size() ? host.slots[successor]->row.name.c_str() : "-",
                static_cast<double>(host.summary.simulated_seconds), copied ? 1 : 0);
        };
        // The image's 77h delivery is the whole 0077BD70: 0071C150, SetLeader
        // 0070D0C0 and DetachMember 0070E4C0. For a ship this host's row-15 flush
        // would deliver the destroy leave (kDeadMemberLeavesGroupBound); it runs
        // here instead, where the image's pump has it, and the flush's call then
        // finds +284h clear. Other units keep the copy alone, as before.
        const bool whole = kDeadMemberLeavesGroupBound && kSunkShipFlushBound
            && unit_is_kind_of(index, 6);
        if (phase == bsp::DeadLeaderDeathPhase::BeforePump) {
            // The pump copies before the flush's 0071DDB0: no 5Dh for the successor.
            if (whole) {
                ++host.director_whole_leaves_at_kill;
                leave_group_on_destroy_0077bd70(index);
                return;
            }
            slot.director_handover_ran = true;
            run();
            return;
        }
        // HitDrainAfterPump: the 77h waits for the next pump, queued ahead of the
        // successor's 5Dh that 0071DDB0 posts at this kill. A leave_group that
        // comes first runs it and clears `posted` (see there).
        slot.director_handover_posted = true;
        ++host.director_handovers_posted;
        const auto deliver = [this, &host, index, run, whole]() {
            GameUnitSlot& s = *host.slots[index];
            if (!s.director_handover_posted) {
                ++host.director_handovers_skipped;
                return;
            }
            s.director_handover_posted = false;
            if (whole) {
                ++host.director_whole_leaves_at_kill;
                leave_group_on_destroy_0077bd70(index);
                return;
            }
            s.director_handover_ran = true;
            run();
        };
        if (!host.commands.post_loopback_callback(index, deliver)) {
            slot.director_handover_posted = false;
            --host.director_handovers_posted;
        }
    }
}

bool GameUnitsHost::formation_leave_waits_for_handover(std::size_t index) const {
    if constexpr (!kDeadLeaderDirectorStateCopyBound || !kSetCommandQueueDelayBound) {
        // The loopback delivers posted callbacks only with the queue delay bound.
        static_cast<void>(index);
        return false;
    } else {
        const Impl& host = *impl_;
        if (index >= host.slots.size()) return false;
        const GameUnitSlot& slot = *host.slots[index];
        if (slot.director_handover_posted) return true;
        const std::int32_t g = slot.formation_group;
        if (g < 0 || static_cast<std::size_t>(g) >= host.formation_groups.size()) return false;
        const std::size_t lead = host.formation_groups[static_cast<std::size_t>(g)].leader;
        return lead != index && lead < host.slots.size()
            && host.slots[lead]->director_handover_posted
            && host.slots[lead]->formation_group == g;
    }
}

bool GameUnitsHost::post_formation_leave_0077c980(std::size_t index) {
    // Packet cc13-jm08. The image's Lua leaves (00899EB0, 0089A020) post a 77h
    // through 0077C980 -> 0077C2A0 -> 0076E520 and 0077BD70 runs at the next
    // pump (0076C600, row 9). After a HitDrainAfterPump kill that FIFO already
    // holds the kill's 77h, then the 5Dh 0071DDB0 posted for the successor; the
    // Lua entry comes third, so the successor's inherited head is popped before
    // it leaves. This host posts only while formation_leave_waits_for_handover;
    // every other Lua leave stays synchronous (before the pump, not at it).
    if constexpr (!kDeadLeaderDirectorStateCopyBound || !kSetCommandQueueDelayBound) {
        static_cast<void>(index);
        return false;
    } else {
        Impl& host = *impl_;
        if (index >= host.slots.size()) return false;
        if (!host.commands.post_loopback_callback(index,
                [this, index]() { leave_group_on_destroy_0077bd70(index); })) {
            return false;
        }
        {
            // Goal 4 chain 260: 0077C980 at 0077CA21 sends +20h = 0 (a Lua
            // leave names no other); 0076E520's copy is what the queue holds
            // (0075C860 write, 00768530 case 77h 0075A330, 0075C8B0 read), and
            // 0077FE80 arm 3 resolves its +20h for 0077BD70. This host's leave
            // models the null-other arm only, so a copy naming one is recorded.
            // Made once the post succeeded: a failed post leaves synchronously
            // through leave_formation_message_0077c980, which makes its own.
            const std::size_t other =
                commands_formation_member_copy_0076e520(0x77, kNoFormationMember);
            if (other != kNoFormationMember) {
                impl_->log.unimplemented("UnitGroup::leave_message_other", "0077fe80");
            }
        }
        ++host.director_lua_leaves_posted;
        return true;
    }
}

void GameUnitsHost::leave_formation_message_0077c980(std::size_t index) {
    // Goal 4 chain 260: the 77h copy as in post_formation_leave_0077c980, then
    // 0077BD70 at the call (the synchronous leave's substitution). 0077C980
    // builds the message only while [unit+284h] is set (0077C99C..0077C9A6).
    if (unit_formation_group_0284(index) >= 0) {
        const std::size_t other =
            commands_formation_member_copy_0076e520(0x77, kNoFormationMember);
        if (other != kNoFormationMember) {
            impl_->log.unimplemented("UnitGroup::leave_message_other", "0077fe80");
        }
    }
    leave_group_on_destroy_0077bd70(index);
}

bool GameUnitsHost::unit_is_formation_follower_007788b0(std::size_t index) const noexcept {
    // 007788B0 whole: g = [unit+284h]; g && [g+14h] != unit.
    const std::int32_t group = unit_formation_group_0284(index);
    if (group < 0) return false;
    return formation_leader_0014(group) != index;
}

std::int32_t GameUnitsHost::formation_member_count(std::int32_t group) const noexcept {
    const Impl& host = *impl_;
    if (group < 0 || static_cast<std::size_t>(group) >= host.formation_groups.size()) {
        return 0;
    }
    return static_cast<std::int32_t>(
        host.formation_groups[static_cast<std::size_t>(group)].members.size());
}

std::size_t GameUnitsHost::formation_member_unit(std::int32_t group,
                                                std::int32_t slot) const noexcept {
    // Packet cc9_ship_formation_speed. The record's +00h is the one-based handle
    // the join stores (record.entity = unit + 1).
    const Impl& host = *impl_;
    if (group < 0 || static_cast<std::size_t>(group) >= host.formation_groups.size()) {
        return static_cast<std::size_t>(-1);
    }
    const auto& members = host.formation_groups[static_cast<std::size_t>(group)].members;
    if (slot < 0 || static_cast<std::size_t>(slot) >= members.size()) {
        return static_cast<std::size_t>(-1);
    }
    auto& native = *host.formation_groups[static_cast<std::size_t>(group)].native_members;
    // bsp.py show 0070D060: return record[index].entity without a unit
    // dereference. Bounds above and capture's 24-record limit admit the
    // raw indexed load; membership is refreshed at this read boundary.
    const std::uint32_t entity = native.capture_members(members)
        ? reinterpret_cast<std::uint32_t>(bsp::native_unit_group_member_at_0070d060(
            native.data(), nullptr, static_cast<std::uint32_t>(slot)))
        : members[static_cast<std::size_t>(slot)].entity;
    return entity == 0u ? static_cast<std::size_t>(-1) : static_cast<std::size_t>(entity - 1u);
}

GameUnitsHost::FormationStation GameUnitsHost::formation_station_0070d290(
    std::size_t unit, float across_scale, float along_scale) const noexcept {
    FormationStation out{};
    const Impl& host = *impl_;
    if (unit >= host.slots.size()) return out;
    const std::int32_t group_index = host.slots[unit]->formation_group;

    // 0070D2A1 resolves the member record through 0070D080 and 0070D2A6/0070D2AF
    // take the leader branch when the unit IS the leader or has no record.
    const bsp::ShipAiUnitGroupMember* record = nullptr;
    bsp::ShipAiUnitGroupMember projected_record;
    std::size_t leader = unit;
    if (group_index >= 0
        && static_cast<std::size_t>(group_index) < host.formation_groups.size()) {
        const Impl::FormationGroup& group =
            host.formation_groups[static_cast<std::size_t>(group_index)];
        leader = group.leader;
        if (leader != unit) {
            const std::uint32_t handle = static_cast<std::uint32_t>(unit + 1u);
            // bsp.py show 0070D030 / 0070D070: captured signed count,
            // first matching identity, then +18h + index*34h. Copy the
            // current live records, including authored offset columns.
            // This composition supplies the same record as the native
            // 0070D2A1 call to 0070D080, with no added unit dereference.
            // These bodies never dereference the registry identity word.
            if (group.native_members->capture_members(group.members)) {
                const std::int32_t member_index = bsp::native_unit_group_member_index_0070d030(
                    group.native_members->data(), reinterpret_cast<const void*>(handle));
                if (member_index >= 0) {
                    const void* const native_record = bsp::native_unit_group_record_at_0070d070(
                        group.native_members->data(), nullptr, static_cast<std::uint32_t>(member_index));
                    std::memcpy(&projected_record, native_record, sizeof(projected_record));
                    record = &projected_record;
                }
            } else {
                // A host group beyond native capacity stays on its old
                // vector search rather than entering a raw body out of domain.
                for (const bsp::ShipAiUnitGroupMember& member : group.members) {
                    if (member.entity == handle) { record = &member; break; }
                }
            }
        }
        if (record != nullptr) {
            const std::size_t column = static_cast<std::size_t>(group.column);
            if (column < bsp::kShipAiUnitGroupColumnCount) {
                out.across = record->lateral[column] * across_scale;   // 0070D2BD
                out.along = record->axial[column] * along_scale;       // 0070D2CC
            }
        }
    }

    if (record != nullptr && leader < host.slots.size()) {
        // 0070D2EE: 00811150(leader, along) -> the wake point and its direction,
        // then out[0] = pos.x - across * dir.z and out[1] = pos.z + across * dir.x
        // (0070D342, 0070D348), the left normal this packet also decomposes on.
        const bsp::ShipAiWakePoint base = bsp::cc13_sweepC::unit_wake_point_at_distance_00811150(
            host.slots[leader]->wake, out.along);
        out.x = base.x - out.across * base.dir_z;
        out.z = base.z + out.across * base.dir_x;
        out.dir_x = base.dir_x;
        out.dir_z = base.dir_z;
        out.wake_yaw_rate = base.yaw_rate;
        out.wake_yaw_written = base.yaw_rate_written;
        out.valid = true;
        return out;
    }

    // 0070D362: the unit's own position and (cos, sin) of wrap_2pi(pi/2 - heading).
    out.is_leader_branch = true;
    const bsp::ShipMotionState& motion = host.slots[unit]->motion;
    out.x = motion.position[0];
    out.z = motion.position[2];
    const float heading = static_cast<float>(
        std::atan2(static_cast<double>(motion.pose_row2[0]),
                   static_cast<double>(motion.pose_row2[2])));
    float angle = 1.5707963705062866f - heading;                  // 00CE3830
    if (angle < 0.0f) angle += 6.2831854820251465f;               // 00CE3828
    out.dir_x = std::cos(angle);
    out.dir_z = std::sin(angle);
    out.across = 0.0f;                                            // 0070D3E1
    out.along = 0.0f;                                             // 0070D3E9
    out.wake_yaw_written = false;                                 // 0070D3F1 writes 0
    out.valid = true;
    return out;
}

bool GameUnitsHost::formation_join_0077f940(std::size_t follower, std::size_t leader) {
    Impl& host = *impl_;
    if (follower >= host.slots.size() || leader >= host.slots.size()) return false;
    if (follower == leader) return false;                 // 00779824, the identity arm
    // 0070ED30 consumes the settings produced by 0083B5E0. Refuse before
    // any detach when that producer failed; do not invent tuning values.
    if (host.lua.gameplay_tuning_settings() == nullptr) {
        host.record("UnitGroup::member_slots_settings_unavailable", 0x0070ed30u);
        return false;
    }
    if (host.observer_runtime == nullptr || !host.observer_runtime->has_live_dispatch_owner())
        throw std::logic_error("formation join requires the application observer runtime");

    // Packet cc8_ship_station: 0077F940's merge arm, read whole from the listing
    // (126 instructions, body 0077F940-0077FAC8, RET 4). USN01 reaches it - the
    // script orders Northampton, which already leads SaltLakeCity and Dunlap, to
    // join Enterprise - so the arm the previous packet left out is the one that
    // decides what those three ships do.
    //
    //   0077F96E  nothing happens at all when the ordered ship's group is the
    //             target's group: `iVar3 == 0 || iVar3 != iVar4`.
    //   0077F97x  `brought` is 1, or the ordered ship's own member count
    //             (group+4F8h) when it LEADS that group - it brings its whole
    //             formation with it.
    //   0077F9B4  the cap: (float)(target_count + brought) <= settings+420h,
    //             where target_count is 1 when the target has no group. On a
    //             refusal nothing at all happens - no create, no detach, no add.
    //   0077F9D9  0070DB20 creates a group around the target when it has none;
    //             0077F9E6 otherwise redirects the target to its group's leader.
    //   0077FA10  the list: MemberAt(i) over the ordered ship's group, the
    //             ordered ship itself at index 0 and every other member after it.
    //   0077FA40  each of those followers leaves through 0077BD70 with the
    //             ordered ship as the argument, and 0077FA51 the ordered ship
    //             itself leaves with a null one.
    //   0077FA81  0070EF30 adds each brought unit to the target group in that
    //             order, ECX = the group, so each one's column is measured
    //             against the NEW leader's wake at its own position.
    //
    // NOT modelled, and named: 0077BD70's own body (0070D0C0 SetLeader, 0070D8D0,
    // 0070E4C0 DetachMember) is unread, so a detach here empties the record and
    // clears entity+284h without promoting a new leader for the group left
    // behind; and the vtable[114h] / vtable[58h] follow-up at 0077FA8D is
    // unread, as the previous packet already recorded. The create/redirect is
    // also left where it already stood, after the detach rather than before it,
    // which can only differ when the two groups are the same - and that case
    // returns above.
    const std::int32_t ordered_group_before = host.slots[follower]->formation_group;
    const std::int32_t target_group_before = host.slots[leader]->formation_group;
    if (ordered_group_before >= 0 && ordered_group_before == target_group_before) {
        ++host.formation_rejoins;
        return false;                                      // 0077F96E
    }

    // 0077FA10's list, built BEFORE any detach. Only a unit that leads its group
    // brings anything; a follower ordered away brings only itself.
    std::vector<std::size_t> brought;
    brought.push_back(follower);
    if (ordered_group_before >= 0
        && host.formation_groups[static_cast<std::size_t>(ordered_group_before)].leader
               == follower) {
        for (const bsp::ShipAiUnitGroupMember& member :
             host.formation_groups[static_cast<std::size_t>(ordered_group_before)].members) {
            const std::size_t unit = static_cast<std::size_t>(member.entity) - 1u;
            if (unit != follower && unit < host.slots.size()) brought.push_back(unit);
        }
    }

    // 0077F9B4. FormationMaxCount is settings+420h; 24 is cited from
    // docs/SHIP_AI_FORMATION.md rather than re-read here.
    constexpr float kFormationMaxCount = 24.0f;
    const std::size_t target_count = (target_group_before >= 0)
        ? host.formation_groups[static_cast<std::size_t>(target_group_before)].members.size()
        : static_cast<std::size_t>(1);
    if (static_cast<float>(target_count + brought.size()) > kFormationMaxCount) {
        host.log.notef("formation join refused: ordered=%s target=%s brought=%zu "
            "target_count=%zu cap=%.0f (0077F9B4)",
            host.slots[follower]->row.name.c_str(), host.slots[leader]->row.name.c_str(),
            brought.size(), target_count, static_cast<double>(kFormationMaxCount));
        return false;
    }

    // 0077FA40 / 0077FA51: every brought unit leaves its old group first.
    for (const std::size_t unit : brought) {
        const std::int32_t old_group = host.slots[unit]->formation_group;
        if (old_group < 0) continue;
        std::vector<bsp::ShipAiUnitGroupMember>& members =
            host.formation_groups[static_cast<std::size_t>(old_group)].members;
        const std::uint32_t old_handle = static_cast<std::uint32_t>(unit + 1u);
        std::vector<bsp::ShipAiUnitGroupMember> kept;
        kept.reserve(members.size());
        for (const bsp::ShipAiUnitGroupMember& member : members) {
            if (member.entity != old_handle) kept.push_back(member);
        }
        host.unregister_formation_observer(
            host.formation_groups[static_cast<std::size_t>(old_group)], unit);
        members.swap(kept);
        host.slots[unit]->formation_group = -1;
    }

    std::int32_t group = host.slots[leader]->formation_group;
    if (group < 0) {
        // 0070DB20: the leader becomes member 0 with an all-zero relative
        // position, the count is 1, and shape stays 0 so column 0 is live.
        group = static_cast<std::int32_t>(host.formation_groups.size());
        host.formation_groups.emplace_back();
        Impl::FormationGroup& created = host.formation_groups.back();
        // 0070DB20's fresh group's existing whole 0070DAB0 producer.
        // No call through the constructor's original profile is admitted.
        created.native_members->initialize();
        // Publish this mission unit's existing one-based identity into
        // the fresh native group, then use its +14h result as the host
        // leader. Only the null-previous arm of 0070D0C0 is admitted.
        created.leader = created.native_members->initialize_first_leader(
            static_cast<std::uint32_t>(leader + 1u)) - 1u;
        bsp::ShipAiUnitGroupMember record;
        record.entity = static_cast<std::uint32_t>(leader + 1u);
        created.members.push_back(record);
        host.slots[leader]->formation_group = group;       // leader+284h = group
        host.register_formation_observer(created, leader);
        ++host.formation_creates;
    } else {
        // "otherwise redirects to other's leader": the group the leader already
        // belongs to is the one joined, whoever it is led by.
        group = host.slots[leader]->formation_group;
    }

    Impl::FormationGroup& target = host.formation_groups[static_cast<std::size_t>(group)];
    const std::uint32_t handle = static_cast<std::uint32_t>(follower + 1u);
    std::int32_t member_count = static_cast<std::int32_t>(target.members.size());
    // Supply writable record[count] without publishing it to the recovered
    // join's walkers. 0070EF8D, after slot production, owns the count change.
    target.members.resize(static_cast<std::size_t>(member_count) + 1u);
    {
        // bsp.py show 0070EF30: 0070EF85 calls the existing 0070ED30 body
        // with the JOIN index, before count changes. Its inputs below borrow
        // the actual mission settings, entity poses/cache and leader wake.
        // Registry words are identities only; no raw unit ABI is fabricated.
        struct MemberSlots final : bsp::ShipAiFormationSlotHost {
            Impl& h;
            const bsp::GameplayTuningSettings& settings;
            MemberSlots(Impl& owner, const bsp::GameplayTuningSettings& tuning)
                : h(owner), settings(tuning) {}
            GameUnitSlot& unit(std::uint32_t identity) {
                return *h.slots.at(static_cast<std::size_t>(identity - 1u));
            }
            void refresh_world_pose_00414db0(std::uint32_t identity) override {
                bsp::refresh_pose_00414db0(*unit(identity).pose);
            }
            void build_world_inverse_00b63d50(std::uint32_t identity) override {
                auto& u = unit(identity);
                bsp::PoseDerivedView view{*u.pose, u.derived};
                bsp::get_pose_derived_00414e10(view);
            }
            void world_position_0fc(std::uint32_t identity, float out[3]) override {
                const auto& world = unit(identity).world;
                for (std::size_t i = 0; i < 3; ++i) out[i] = world[12 + i];
            }
            void transform_by_leader_inverse_004142e0(std::uint32_t identity,
                const float in[3], float out[3]) override {
                auto& u = unit(identity);
                bsp::PoseDerivedView view{*u.pose, u.derived};
                const auto& inverse = bsp::get_pose_derived_00414e10(view);
                std::array<float, 3> result;
                bsp::transform_point_004142e0({in[0], in[1], in[2]}, inverse, result);
                std::memcpy(out, result.data(), sizeof(result));
            }
            void transform_by_leader_world_004142e0(std::uint32_t identity,
                const float in[3], float out[3]) override {
                auto& u = unit(identity);
                bsp::refresh_pose_00414db0(*u.pose);
                std::array<float, 3> result;
                bsp::transform_point_004142e0({in[0], in[1], in[2]}, u.world, result);
                std::memcpy(out, result.data(), sizeof(result));
            }
            float game_settings_follower_max_dist_00424c40() override {
                return settings.formacio_follower_max_dist;
            }
            float game_settings_formation_ship_dist_00424c40() override {
                return settings.formacio_formation_ship_dist;
            }
            void normalize_in_place_0042b260(float v[3]) override {
                std::array<float, 3> vector{v[0], v[1], v[2]};
                bsp::normalize_camera_basis_0042b260(vector);
                std::memcpy(v, vector.data(), sizeof(vector));
                ++h.formation_clamped;
            }
            void wake_decompose_00811180(std::uint32_t identity, const float point[3],
                float& across, float& along) override {
                const auto decomposition = bsp::ship_ai_wake_decompose_00811180(
                    unit(identity).wake, point);
                if (decomposition.valid) {
                    across = decomposition.across;
                    along = decomposition.along;
                } else {
                    // Preserve the existing unfilled-ring diagnostic. Normal
                    // creation fills the real wake at 00818EA0 -> 00810020.
                    ++h.formation_columns_unmeasurable;
                }
            }
        } binding{host, *host.lua.gameplay_tuning_settings()};
        struct Join final : bsp::ShipAiFormationJoinHost,
            bsp::flak_note_seg36_37_argon::UnitGroupSpeedCeilingView {
            Impl& h;
            GameUnitsHost& units;
            Impl::FormationGroup& target;
            std::int32_t& count;
            bsp::ShipAiFormationSlotHost& slots;
            std::uint32_t entity;
            Join(Impl& owner, GameUnitsHost& facade, Impl::FormationGroup& group,
                std::int32_t& published_count, bsp::ShipAiFormationSlotHost& slot_host,
                std::uint32_t member)
                : h(owner), units(facade), target(group), count(published_count),
                  slots(slot_host), entity(member) {}
            bool observer_pair_registered(std::uint32_t, std::uint32_t member) override {
                return bsp::observer_pair_registered_00694af0(
                    h.slots.at(member - 1u)->observer_prefix.observed_00,
                    target.native_members->observer_owner(), h.observer_runtime->lifetime());
            }
            void observer_register_pair(std::uint32_t, std::uint32_t member) override {
                h.register_formation_observer(target, member - 1u);
            }
            void set_entity_group_284(std::uint32_t member, std::uint32_t group) override {
                h.slots.at(member - 1u)->formation_group = static_cast<std::int32_t>(group - 1u);
            }
            void produce_member_slots_0070ed30(std::size_t index) override {
                bsp::ShipAiFormationMemberSlots produced;
                bsp::ship_ai_formation_produce_member_slots(slots,
                    static_cast<std::uint32_t>(target.leader + 1u), entity, index, produced);
                auto& record = target.members.at(index);
                std::memcpy(record.field_04.data(), produced.relative, sizeof(produced.relative));
                record.lateral = produced.columns.across;
                record.axial = produced.columns.along;
                h.done("UnitGroup::produce_member_slots", 0x0070ed30u);
            }
            void refresh_speed_ceiling_0070da00() override {
                bsp::flak_note_seg36_37_argon::UnitGroup_RefreshSpeedCeiling_0070da00(*this);
                target.native_members->capture_members(target.members);
            }
            float seed_00cfd6f4() const override { return bsp::kShipAiFormationSpeedSeed; }
            float unset_00cf4888() const override { return bsp::kShipAiFormationMemberSpeedUnset; }
            std::int32_t member_count_04f8() const override { return count; }
            bool member_present(std::int32_t index) const override {
                return target.members.at(static_cast<std::size_t>(index)).entity != 0;
            }
            float member_class_speed(std::int32_t index) const override {
                const auto member = target.members.at(static_cast<std::size_t>(index)).entity - 1u;
                return units.unit_is_kind_of(member, 6)
                    ? units.unit_class_max_speed_0500(member)
                    : units.plane_class_max_speed_0188(member);
            }
            void set_missing_member_speed_30(std::int32_t index, float value) override {
                bsp::ship_ai_formation_set_member_speed(
                    target.members.at(static_cast<std::size_t>(index)), value);
            }
            float ceiling_0504() const override { return target.native_members->speed_ceiling(); }
            void set_ceiling_0504(float value) override {
                target.native_members->publish_speed_ceiling(value);
            }
        } join{host, *this, target, member_count, binding, handle};
        // Evidence: bsp.py show 0070EF30. One complete call replaces the old
        // append/slot writes, using actual unit/group identities and the
        // application observer graph. Numeric identities are never dispatched.
        std::int32_t added;
        try {
            added = bsp::ship_ai_formation_add_member(join, target.members.data(), member_count,
                static_cast<std::uint32_t>(group + 1), handle);
        } catch (...) {
            target.members.resize(static_cast<std::size_t>(member_count));
            throw;
        }
        target.members.resize(static_cast<std::size_t>(member_count));
        if (added < 0) {
            ++host.formation_rejoins;
            return false;
        }
    }
    host.done("UnitGroup::add_member_0070ef30", 0x0070ef30u);
    ++host.formation_joins;
    // 007788B0 / 007788D0 are read by 00836920's idle re-issue, which lives in
    // the commands host and has no route to the group, so the pair is pushed.
    host.commands.set_unit_formation(follower, true, leader);

    // 0077FA81's loop over the rest of the list. Each one has already been
    // detached above, so it leads nothing and brings only itself: the recursion
    // is one level deep and terminates. The leader taken is the target group's
    // own leader, which is 0077F9E6's redirect.
    const std::size_t target_leader =
        host.formation_groups[static_cast<std::size_t>(group)].leader;
    ++host.formation_join_depth;
    for (std::size_t i = 1; i < brought.size(); ++i) {
        formation_join_0077f940(brought[i], target_leader);
    }
    --host.formation_join_depth;
    // Packet cc9_formation_join_follow. 0077FA8D..0077FAB8 inside the 0077FA81 loop,
    // so it runs brought.size() times with the same unit and the same target; the
    // recursion above models the loop's 0070EF30 calls only, so the follow belongs
    // to the top-level call alone.
    if (host.formation_join_depth == 0) {
        for (std::size_t i = 0; i < brought.size(); ++i) {
            ++host.formation_join_follows;
            if constexpr (Impl::kFormationJoinFollowBound) {
                if (host.commands.issue_follow_command_00720cd0(follower, target_leader)) {
                    ++host.formation_join_follows_pushed;
                }
            }
        }
    }
    return true;
}

bool GameUnitsHost::unit_flag_0061(std::size_t index) const {
    // docs/UNIT_AUTOPILOT_PAIR.md scanned .text for a writer of unit+61h at the
    // direct displacement and at the seven shifted unit bases and found none, so
    // nothing in this process or in the recovered load can set it. The byte is
    // clear, which is what lets the ship AI controller run at all.
    static_cast<void>(index);
    return false;
}

float GameUnitsHost::unit_forward_speed_0092d730(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const GameUnitSlot& slot = *host.slots[index];
    bsp::UnitBodyAxisSpeedInputs axis{};
    axis.velocity[0] = slot.motion.linear_velocity.x;
    axis.velocity[1] = slot.motion.linear_velocity.y;
    axis.velocity[2] = slot.motion.linear_velocity.z;
    axis.axis[0] = slot.motion.pose_row2[0];
    axis.axis[1] = slot.motion.pose_row2[1];
    axis.axis[2] = slot.motion.pose_row2[2];
    return bsp::unit_forward_speed_0092d730(axis);
}

GameUnitsHost::PlayerBombRelease GameUnitsHost::player_bomb_release_006082d0(std::size_t index) {
    return impl_->player_bomb_release_006082d0(index);
}

bool GameUnitsHost::unit_linear_velocity(std::size_t index, float out[3]) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    const GameUnitSlot& slot = *host.slots[index];
    out[0] = slot.motion.linear_velocity.x;
    out[1] = slot.motion.linear_velocity.y;
    out[2] = slot.motion.linear_velocity.z;
    return true;
}

bool GameUnitsHost::unit_plane_world_velocity(std::size_t index, float out[3]) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    const GameUnitSlot& slot = *host.slots[index];
    out[0] = slot.plane_world_velocity[0];
    out[1] = slot.plane_world_velocity[1];
    out[2] = slot.plane_world_velocity[2];
    return true;
}

bool GameUnitsHost::unit_plane_body_angular_rate(std::size_t index, float out[3]) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    const GameUnitSlot& slot = *host.slots[index];
    out[0] = slot.plane_body_angular[0];   // ctl+48h
    out[1] = slot.plane_body_angular[1];   // ctl+4Ch
    out[2] = slot.plane_body_angular[2];   // ctl+50h
    return true;
}

float GameUnitsHost::unit_retardation_0508(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->motion_class.retardation;
}

bool GameUnitsHost::helm_route_take(std::size_t index, float throttle, float x, float z) {
    Impl& host = *impl_;
    if constexpr (!Impl::kPlayerRoleBookkeepingBound) {
        host.log.notef("helm route refused: the role bookkeeping is not bound, so role 1 "
            "cannot be taken");
        return false;
    } else {
        if (!host.controlled_bound || index != host.controlled_index
            || index >= host.slots.size()) {
            host.log.notef("helm route refused: \"%s\" is not the controlled unit (00E188D8); "
                "a player can take only that ship's helm",
                index < host.slots.size() ? host.slots[index]->row.name.c_str() : "?");
            return false;
        }
        host.helm_route_on = true;
        host.helm_route_arrived = false;
        host.helm_throttle = throttle;
        host.helm_rudder = 0.0f;
        host.helm_route_x = x;
        host.helm_route_z = z;
        // Packet cc13-mapback (log only): map_up is the state
        // kHelmPrepareMapUpGateCc13Bound tests. map_up=1 means the take waits:
        // the prepare (role-1 transfer) is skipped, and so is the issue gate
        // further down, which also requires this unit to be the controlled
        // unit (controlled_index == index), as it is here.
        host.log.notef("helm route taken: \"%s\" throttle %.3f toward (%.1f, %.1f); role 1 "
            "through 0077C470(unit, 2, 1) at the next step, then the helm every step "
            "map_up=%d",
            host.slots[index]->row.name.c_str(), static_cast<double>(throttle),
            static_cast<double>(x), static_cast<double>(z),
            host.helm_screen_occluded_by_map ? 1 : 0);
        return true;
    }
}

float GameUnitsHost::unit_heading_radians(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return Impl::pose_heading_radians(*host.slots[index]);
}

float GameUnitsHost::unit_heading_vtable50(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size() || !host.slots[index]) return 0.0f;
    const GameUnitSlot& slot = *host.slots[index];
    const int id = slot.class_id;
    if (id >= 0x06 && id <= 0x0E) {
        // 006DFD60 is the ship vtable[50h] implementation. Its +1050h is this
        // live field, written by ship_motion_tail_00826c34 after every motion
        // step (00826C56); lend it directly to the reconstructed single load.
        return bsp::pool_pool01_cc13::unit_instance_get_hull_heading_006dfd60(
            slot.hull_heading_1050);
    }
    if (id >= 0x0F && id <= 0x17) return slot.plane_heading_c6c;   // 0074E260
    if (id == 0x45) {
        // 006D2040: atan2(+94h, +9Ch) of the local frame's row 2, FSTP float. The
        // host's airfield is top level, so its pose rows are the local rows (as at
        // runway_heading_88).
        return Impl::pose_heading_radians(slot);
    }
    return 0.0f;                                                   // 0042B8C0 FLDZ
}

float GameUnitsHost::unit_current_yaw_rate_00811940(std::size_t index) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    UnitRudderBinding rudder(host, *host.slots[index]);
    if constexpr (kUnitYawRateForwardSpeedBound) {
        // 00811940 -> 00811890 reads the body-axis speed 0092D730 for 0082ECB0.
        rudder.forward_speed = unit_forward_speed_0092d730(index);
    }
    ++host.yaw_rate_calls_00811940;
    const float rate = bsp::unit_current_yaw_rate_00811940(rudder);
    if (rate != 0.0f) ++host.yaw_rate_nonzero_00811940;
    return rate;
}

float GameUnitsHost::unit_class_yaw_rate_0082ecb0(std::size_t index, float rudder,
    float speed, float efficiency) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    // The binding supplies 0082E890's settings block (00424C40); the class
    // fields are the slot's copy of [unit+538h].
    UnitRudderBinding curve(host, *host.slots[index]);
    return bsp::unit_class_yaw_rate_0082ecb0(host.slots[index]->fields, rudder, speed,
        efficiency, curve);
}

bool GameUnitsHost::run_cruise_state_step_009e1170(std::size_t index,
    bsp::ShipAiControlBlock& blk, bsp::ShipAiSetterHost& setters) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[index];
    bsp::CruiseOrderedValues ordered{};
    return host.commands.cruise_step(index, host.player_flag_0184(slot),
        unit_forward_speed_0092d730(index),
        bsp::unit_reference_speed_0080fc30(slot.motion.max_speed,
            unit_reference_speed_scale_0080fc30(index)),
        ordered, &blk, &setters);
}

bool GameUnitsHost::run_cruise_state_step_009e1170(std::size_t index,
    bsp::ShipAiControlBlock& blk, bsp::ShipAiSetterHost& setters,
    bsp::ShipAiAvoidanceRequest& request,
    const bsp::ShipAiCruiseAvoidanceInputs& avoidance_inputs) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[index];
    bsp::CruiseOrderedValues ordered{};
    return host.commands.cruise_step(index, host.player_flag_0184(slot),
        unit_forward_speed_0092d730(index),
        bsp::unit_reference_speed_0080fc30(slot.motion.max_speed,
            unit_reference_speed_scale_0080fc30(index)),
        ordered, &blk, &setters, &request, &avoidance_inputs);
}

// ---------------------------------------------------------------------------
// Milestone 2o: the ring's write slot, which the tail of 009f3f80 reads and
// then writes back through 0080e170 / 0080e190.
// ---------------------------------------------------------------------------

float GameUnitsHost::unit_ring_write_slot_throttle(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const bsp::UnitOrderRing& ring = host.slots[index]->ring;
    return bsp::ship_ai_ring_write_slot_throttle(ring);
}

float GameUnitsHost::unit_ring_write_slot_rudder(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const bsp::UnitOrderRing& ring = host.slots[index]->ring;
    return bsp::ship_ai_ring_write_slot_rudder(ring);
}

void GameUnitsHost::unit_ring_set_write_slot_throttle_0080e170(std::size_t index,
    float value) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    bsp::ship_ai_ring_set_write_slot_throttle_0080e170(host.slots[index]->ring, value);
}

void GameUnitsHost::unit_ring_set_write_slot_rudder_0080e190(std::size_t index,
    float value) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    bsp::ship_ai_ring_set_write_slot_rudder_0080e190(host.slots[index]->ring, value);
}

float GameUnitsHost::unit_ring_current_throttle(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->ring.current_param_a;
}

float GameUnitsHost::unit_ring_current_rudder(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->ring.current_param_b;
}

void GameUnitsHost::set_helm_screen_occluded_by_map(bool occluded) {
    impl_->helm_screen_occluded_by_map = occluded;
}

bool GameUnitsHost::helm_screen_occluded_by_map() const noexcept {
    return impl_->helm_screen_occluded_by_map;
}

float GameUnitsHost::unit_ring_confirmed_throttle_0994(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->ring.confirmed_param_a;
}

float GameUnitsHost::unit_ring_confirmed_rudder_0998(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->ring.confirmed_param_b;
}

float GameUnitsHost::unit_yaw_authority_0524(std::size_t index, bool& derived) const {
    derived = false;
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const bsp::ShipClassFields& fields = host.slots[index]->fields;
    const bsp::ShipClassAiDerivedMotion out = bsp::ship_class_ai_derived_motion_00828f20(
        fields.max_rot_angle, fields.max_rot_angle_change_ratio, fields.max_speed);
    derived = out.derived;
    return out.yaw_authority_0524;
}

bool GameUnitsHost::unit_turn_lead_fields_00902290(std::size_t index, float& efficiency_9dc,
    float& rudder_984, float& throttle_980, float& radius_0520, float& speed_vt38) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    const GameUnitSlot& slot = *host.slots[index];
    efficiency_9dc = slot.motion.turn_efficiency;
    rudder_984 = slot.motion.to_turn;
    throttle_980 = slot.motion.throttle;
    // 00828F66 leaves +520h at the constructor's value when its gate fails;
    // LABELLED: that value is taken as 0.
    const bsp::ShipClassAiDerivedMotion derived = bsp::ship_class_ai_derived_motion_00828f20(
        slot.fields.max_rot_angle, slot.fields.max_rot_angle_change_ratio, slot.fields.max_speed);
    radius_0520 = derived.derived ? derived.turn_radius_0520 : 0.0f;
    speed_vt38 = unit_forward_speed_0092d730(index);
    return true;
}

bool GameUnitsHost::ship_firepower_control_factors(std::size_t index,
    float& engine_9d8, float& steering_9dc) const {
    if (index >= impl_->slots.size()) return false;
    const auto& motion = impl_->slots[index]->motion;
    engine_9d8 = motion.thrust_mod;
    steering_9dc = motion.turn_efficiency;
    return true;
}

float GameUnitsHost::unit_rudder_curve_0082e890(std::size_t index, float magnitude) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    UnitRudderBinding curve(host, *host.slots[index]);
    return bsp::unit_rudder_denominator_0082e890(magnitude, curve);
}
bool GameUnitsHost::enable_ai_drive(const std::string& unit_name, float throttle,
    float rudder) {
    Impl& host = *impl_;
    if (host.ship_ai == nullptr) return false;
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        if (host.slots[index]->row.name != unit_name) continue;
        host.ship_ai->set_ai_drive(index, throttle, rudder);
        return true;
    }
    host.log.notef("--ai-drive \"%s\" names no created instance; nothing was driven",
        unit_name.c_str());
    return false;
}

bool GameUnitsHost::unit_pose_valid_00c8(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    return host.slots[index]->world_valid != 0;
}

void GameUnitsHost::unit_position_00fc(std::size_t index, float& x, float& y,
    float& z) const {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    const bsp::ShipMotionState& motion = host.slots[index]->motion;
    x = motion.position[0];
    y = motion.position[1];
    z = motion.position[2];
}

std::size_t GameUnitsHost::unit_part_descriptor_count(std::size_t index) const {
    // (unit+34Ch - unit+348h) >> 2: the class's Damage.Sections count (see
    // Impl::damage_section_count). 0 for an index past the end.
    if (index >= impl_->slots.size()) return 0;
    const int n = impl_->damage_section_count(impl_->slots[index]->row.type_id);
    return n > 0 ? static_cast<std::size_t>(n) : 0;
}

int GameUnitsHost::plane_squadron_live_count_3cc(std::size_t plane) const noexcept {
    // Member ids are slot indices (process_index is the slot's position, 20376).
    // A stock-return stamp wins: that path cleared the seat before its Kill.
    if (plane < impl_->slots.size()) {
        const int stamped = impl_->slots[plane]->squadron_live_3cc_at_stock_return;
        if (stamped >= 0) return stamped;
    }
    const bsp::PlaneSquadronHostRecord* const sq =
        bsp::plane_squadron_registry().find_by_member_unit(plane);
    return sq != nullptr ? sq->live_count() : -1;
}

std::size_t GameUnitsHost::plane_shot_owner_squadron_head_0072c119(
    std::size_t owner) const noexcept {
    if (owner >= impl_->slots.size() || !impl_->slots[owner]) return owner;
    const GameUnitSlot& s = *impl_->slots[owner];
    if (!bsp::unit_is_kind_of(s.class_id, 0x0F)) return owner;           // 0072C11E
    if (!(s.plane_formation_sight_914 > 0.0f)) return owner;             // 0072C128 JBE
    const bsp::PlaneSquadronHostRecord* const r =
        bsp::plane_squadron_registry().find_by_member_unit(owner);
    if (r == nullptr) return owner;
    // [squadron+3D0h], the first seat, read as the pilot step reads it
    // (+9D4h at 0099B01F, +3D0h at 0099B029).
    const std::size_t head = Impl::kSquadronRemovesDeadBound ? r->flight_leader()
        : r->member_units.empty() ? bsp::kPlaneSquadronNoUnit : r->member_units[0];
    return head < impl_->slots.size() && impl_->slots[head] ? head : owner;
}

std::size_t GameUnitsHost::squadron_list_24_leader(std::size_t entry_unit) const {
    // [squadron+3D0h] for the squadron whose list-24 node is `entry_unit`: the
    // registry record fused with that slot, then its current flight leader.
    // The unit count when no record matches or the squadron has no live member.
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        std::size_t node = r.squadron_unit;
        if (node == bsp::kPlaneSquadronNoUnit) {
            for (const std::size_t m : r.member_units) {
                if (m != bsp::kPlaneSquadronNoUnit) { node = m; break; }
            }
        }
        if (node != entry_unit) continue;
        const std::size_t leader = r.flight_leader();
        return leader < impl_->slots.size() ? leader : impl_->slots.size();
    }
    return impl_->slots.size();
}

std::size_t GameUnitsHost::world_list_size(int class_id) const noexcept {
    if (class_id < 0 || class_id >= Impl::kWorldListCount) return 0;
    return impl_->world_lists.entries[class_id].count;
}

std::size_t GameUnitsHost::world_list_entry(int class_id, std::size_t position) const noexcept {
    if (class_id < 0 || class_id >= Impl::kWorldListCount) return impl_->slots.size();
    const GameUnitWorldList& list = impl_->world_lists.entries[class_id];
    if (position >= list.count) return impl_->slots.size();
    const GameUnitWorldNode* node = list.head;
    while (position-- != 0) node = node->next;
    return node->unit->process_index;
}

const void* GameUnitsHost::world_list_head(int class_id) const noexcept {
    if (class_id < 0 || class_id >= Impl::kWorldListCount) return nullptr;
    return impl_->world_lists.entries[class_id].head;
}

const void* GameUnitsHost::world_list_node_unit(const void* node) noexcept {
    return static_cast<const GameUnitWorldNode*>(node)->unit;
}

const void* GameUnitsHost::world_list_node_next(const void* node) noexcept {
    return static_cast<const GameUnitWorldNode*>(node)->next;
}

std::size_t GameUnitsHost::count() const noexcept { return impl_->slots.size(); }

bool GameUnitsHost::unit_active(std::size_t index) const noexcept {
    if (index >= impl_->slots.size()) return false;
    return impl_->slots[index]->state->active != 0;
}

bool GameUnitsHost::unit_message_ready(std::size_t index) const noexcept {
    return index < impl_->slots.size() && impl_->slots[index]->message_ready_00bd != 0;
}

const void* GameUnitsHost::unit_identity(std::size_t index) const noexcept {
    return index < impl_->slots.size() ? impl_->slots[index].get() : nullptr;
}

std::optional<bsp::NativeUnitObserverAlias> GameUnitsHost::observer_alias(
    const void* identity) noexcept {
    if (impl_->observer_runtime == nullptr ||
        !impl_->observer_runtime->has_live_dispatch_owner()) return std::nullopt;
    for (const auto& slot : impl_->slots) {
        if (slot.get() == identity && slot->observer_prefix_ready)
            return bsp::NativeUnitObserverAlias{slot.get(), slot->observer_prefix};
    }
    return std::nullopt;
}

std::optional<bsp::NativeSceneLifecycleView> GameUnitsHost::scene_lifecycle_view(
    const void* identity) noexcept {
    auto alias = observer_alias(identity);
    if (!alias) return std::nullopt;
    for (const auto& slot : impl_->slots) {
        if (slot.get() != identity) continue;
        if (!slot->scene_flags_available || slot->state == nullptr) return std::nullopt;
        return bsp::NativeSceneLifecycleView{*alias, slot->state->active,
            slot->state->simulate, slot->scene_destroyed_005e,
            slot->scene_removed_005f, slot->scene_pending_destroy_0060};
    }
    return std::nullopt;
}

const void* GameUnitsHost::unit_identity_from_observer(
    const bsp::NativeObserverOwnerStorage* endpoint) const noexcept {
    if (impl_->observer_runtime == nullptr ||
        !impl_->observer_runtime->has_live_dispatch_owner()) return nullptr;
    for (const auto& slot : impl_->slots) {
        if (slot->observer_prefix_ready &&
            (endpoint == &slot->observer_prefix.observed_00 ||
                endpoint == &slot->observer_prefix.callback_10)) return slot.get();
    }
    return nullptr;
}

bool GameUnitsHost::unit_scene_node_flags(std::size_t index,
    bsp::SceneNodeFlags& out) const noexcept {
    if (index >= impl_->slots.size()) return false;
    const GameUnitSlot& slot = *impl_->slots[index];
    if (!slot.scene_flags_available || slot.state == nullptr) return false;
    out = {slot.state->active != 0, slot.state->simulate != 0,
        slot.scene_destroyed_005e != 0, slot.scene_removed_005f != 0};
    return true;
}

bool GameUnitsHost::read_scene_node_flags(const void* identity,
    bsp::SceneNodeFlags& out) const noexcept {
    // Compare identities before dereferencing: pending queues must hand over
    // the same stable slot as the world registry, not an index-shaped token.
    for (std::size_t index = 0; index < impl_->slots.size(); ++index) {
        if (impl_->slots[index].get() == identity) return unit_scene_node_flags(index, out);
    }
    return false;
}

bool GameUnitsHost::store_scene_node_flags(const void* identity,
    const bsp::SceneNodeFlags& flags) noexcept {
    for (const auto& owned : impl_->slots) {
        if (owned.get() != identity) continue;
        GameUnitSlot& slot = *owned;
        if (!slot.scene_flags_available || slot.state == nullptr) return false;
        slot.state->active = flags.active;
        slot.state->simulate = flags.torn_down;
        slot.scene_destroyed_005e = flags.destroyed;
        slot.scene_removed_005f = flags.removed;
        slot.row.active = flags.active;
        return true;
    }
    return false;
}

// Packet cc9_squadron_pass_hooks: declared no-ops, counted, until the two
// bindings land (docs/CONSTRUCT_WORLD.md section 27).
void GameUnitsHost::on_squadron_pass_a_construct_wing(std::size_t squadron_index) {
    ++impl_->squadron_pass_a_hook_calls;
    if constexpr (!kWingConstructionInPassABound) {
        static_cast<void>(squadron_index);
    } else {
        // Packet cc9_wing_construction. 007F4580, the squadron's slot-39 attach,
        // constructs each wing plane through the vehicle class's vtable +28h
        // (007CFD20 for a plane Type) at 007F4811; each construction reaches
        // 00928760 CALL 00926BE0, so the planes join the pending list while
        // this pass A walks it. create_units makes the same push.
        Impl& host = *impl_;
        const auto staged = host.staged_wings.find(squadron_index);
        if (staged == host.staged_wings.end()) return;
        std::vector<GameSceneEntityRecord> wing = std::move(staged->second);
        host.staged_wings.erase(staged);
        const std::size_t before = host.slots.size();
        create_units(wing);
        const std::size_t made = host.slots.size() - before;
        ++host.wing_hook_builds;
        host.wing_planes_built += made;
        // 007F4B49: +3D0h[wing] names each plane; the registry record's
        // member_units is that array.
        if (bsp::PlaneSquadronHostRecord* squadron =
                bsp::plane_squadron_registry().find_by_member_unit(squadron_index)) {
            for (std::size_t i = 0; i < made; ++i) {
                const std::size_t slot = i + 1;
                if (slot < squadron->member_units.size()) {
                    squadron->member_units[slot] = before + i;
                }
            }
        }
        host.done("PlaneSquadron::pass_a_construct_wing_007f4580", 0x007f4580u);
    }
}

void GameUnitsHost::stage_squadron_wing(std::size_t leader_index,
                                        std::vector<GameSceneEntityRecord> wing) {
    Impl& host = *impl_;
    host.wing_records_staged += wing.size();
    host.staged_wings[leader_index] = std::move(wing);
}

void GameUnitsHost::on_squadron_pass_c_initial_command(std::size_t squadron_index) {
    ++impl_->squadron_pass_c_hook_calls;
    if constexpr (!Impl::kSquadronInitialCommandBound) {
        static_cast<void>(squadron_index);
    } else {
        // Packet cc9_squadron_initial_command, 007F4E06..007F4EA3 (docs/
        // CONSTRUCT_WORLD.md section 27). The host fuses the squadron with its
        // leader, so the leader's director stands in for the squadron's own
        // controller +348h (SUBSTITUTION, labelled).
        Impl& host = *impl_;
        if (squadron_index >= host.slots.size()) return;
        if constexpr (kBaseLaunchChainBound) {
            // Packet cc9_base_launch_inside_start (docs/SQUADRON_LAND_TASK.md
            // 5ax), 007F4C0B-007F4DB5 for an air-ops squadron: `State` (00CF8818)
            // is the bag's 1, so flag = State < 2; `HomeBase` (00CF8820) is the
            // deck owner. 007F1C00(base, flag): in this campaign session
            // ([00E188A8]+1FE4h == 0) 007F1C69 pushes 006CC7B0 (the host made
            // that push at the launch start, labelled in air_operations.cpp),
            // then 007F1C78 +408h = flag and 007F1C7E 007ED6E0, which sets +408h
            // again and calls 007C2130 on every member +3D0h[0..+3CCh). 007F4DA9
            // then skips 007F2920, the airborne placement, since +408h is set.
            bsp::PlaneSquadronHostRecord* rec =
                bsp::plane_squadron_registry().find_by_member_unit(squadron_index);
            if (rec != nullptr && rec->from_air_ops_launch && rec->bag_state < 2
                && !rec->home_launch_408) {
                // kAirOpsOwnerByInstanceBound: the live instance, as block+7Ch is.
                const std::size_t base = host.air_ops_owner_unit(rec->bag_home_base);
                if (base >= host.slots.size()) {
                    ++host.base_launch_inside_no_base;
                } else {
                    rec->home_launch_408 = true;                          // 007F1C78
                    std::size_t entered = 0;
                    for (const std::size_t m : rec->member_units) {       // 007ED700
                        if (m == bsp::kPlaneSquadronNoUnit || m >= host.slots.size()) continue;
                        if (!host.slots[m]) continue;
                        host.plane_enter_state_one_007cc820(*host.slots[m], base);
                        ++entered;
                    }
                    ++host.base_launch_inside_squadrons;
                    host.log.notef("base launch inside start: squadron=%s base=%s members=%zu "
                        "state 1 at (%.1f %.1f %.1f) (007F1C00 flag -> 007ED6E0 -> 007C2130 "
                        "-> 007CC820, 007F4DA9 skips 007F2920; packet "
                        "cc9_base_launch_inside_start)", rec->name.c_str(),
                        rec->bag_home_base.c_str(), entered,
                        static_cast<double>(host.slots[squadron_index]->motion.position[0]),
                        static_cast<double>(host.slots[squadron_index]->motion.position[1]),
                        static_cast<double>(host.slots[squadron_index]->motion.position[2]));
                    host.done("Squadron::enter_inside_007ed6e0", 0x007ed6e0u);
                    // Packet cc9_base_launch_task, 007F5390-007F5404: with +404h
                    // set, a holder kind ([+C0h]+4h) other than 2 (every host
                    // squadron is kind 1) and a base answering IsKindOf(9) or
                    // IsKindOf(45h), 38h bytes -> 007F1DE0(base+310h, squadron):
                    // +1Ch = 0, the squadron at +34h. ORDER, LABELLED: the image
                    // builds it after the command block (007F4E06-007F4EA3);
                    // nothing between reads it, and here the command block's
                    // early returns would otherwise skip it. A scene squadron with
                    // a HomeBase gets a task in the image too, whose first ready
                    // tick finds no state-1 member and ends it; it is not built here.
                    bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
                    for (std::size_t di = 0; di < decks.size(); ++di) {
                        if (decks.name_at(di) != rec->bag_home_base) continue;
                        rec->launch_task_live = true;
                        rec->launch_task_timer_1c = 0.0f;
                        rec->launch_task_deck = di;
                        rec->launch_task_sends = 0;
                        ++host.base_launch_tasks_built;
                        host.done("SquadronLaunchTask::construct_007f1de0", 0x007f1de0u);
                        break;
                    }
                }
            }
        }
        if constexpr (Impl::kSceneHomeBaseBound) {
            // Packet cc9_scene_home_base_key: 007F4C40..007F4CFA, before the
            // command test. `HomeBase` (00CF8820) of type 5 with a non-empty
            // name -> 00925A90 by name (007F4C85); no `SpawnPoint` override
            // (00CE56B8, 007F4C95), which no scene row in this installation
            // authors; the flag is `State` <= 1 (007F4C12..007F4C3C), and no
            // scene row authors `State`, so it is the default 7 -> 0. Then
            // 007F1C00(home, flag) at 007F4CFA.
            GameUnitSlot& squadron = *host.slots[squadron_index];
            const std::string& key = squadron.scene_home_base_key;
            if (!key.empty()) {
                // Packet cc9_ijn08_home_base_measure: 009251F0's matching rule.
                const auto name_matches = [&key](const std::string& candidate) {
                    if constexpr (!Impl::kSceneHomeBaseQualifiedNameBound) {
                        return candidate == key;
                    } else {
                        const std::size_t cut = key.find_last_of('\\');
                        const std::string leaf =
                            cut == std::string::npos ? key : key.substr(cut + 1);
                        if (leaf.size() != candidate.size()) return false;
                        for (std::size_t c = 0; c < leaf.size(); ++c) {
                            if (std::tolower(static_cast<unsigned char>(leaf[c])) !=
                                std::tolower(static_cast<unsigned char>(candidate[c])))
                                return false;
                        }
                        return true;
                    }
                };
                bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
                bsp::AirOpsDeck* deck = nullptr;
                std::string resolved_name;
                for (std::size_t d = 0; d < decks.size(); ++d) {
                    if (name_matches(decks.name_at(d))) {
                        deck = decks.mutable_at(d);
                        resolved_name = decks.name_at(d);
                        break;
                    }
                }
                bool named_unit = false;
                for (const auto& owned : host.slots) {
                    if (name_matches(owned->row.name)) {
                        named_unit = true;
                        if (resolved_name.empty()) resolved_name = owned->row.name;
                        break;
                    }
                }
                if (deck == nullptr && !named_unit) {
                    // 00925A90 answers 0: 007F1C00(0) keeps +404h null.
                    ++host.scene_home_unresolved;
                    host.log.notef("squadron scene home base: squadron=%s HomeBase=\"%s\" "
                        "names no entity; +404h stays null (00925A90 -> 0, packet "
                        "cc9_scene_home_base_key)", squadron.row.name.c_str(), key.c_str());
                } else if (deck == nullptr) {
                    // 007F1C40 006BCD20 answers null for an entity that is not
                    // an MMothership (9) or MAirfield (45h), and 007F1C69 then
                    // calls 006CC7B0 on it: the original faults. REFUSAL,
                    // labelled: the host keeps +404h null.
                    ++host.scene_home_not_airbase;
                    host.log.notef("squadron scene home base: squadron=%s HomeBase=\"%s\" "
                        "has no air-operations block; the image would fault in "
                        "006CC7B0, the host keeps +404h null (packet "
                        "cc9_scene_home_base_key)", squadron.row.name.c_str(), key.c_str());
                } else {
                    squadron.scene_home_base_404 = resolved_name;
                    ++host.scene_home_resolved;
                    // 007F1C4B: [00E188A8]+1FE4h is zero in this campaign
                    // process, so 006CC7B0 (007F1C69) queues the squadron on the
                    // block+74h list 006C58A0 drains. The flag is 0, so +408h is
                    // not written and 007ED6E0 is not called (007F1C74).
                    // SUBSTITUTION, labelled: +35Ch / +3CCh are the record's
                    // `Type` and its member count.
                    const bsp::PlaneSquadronHostRecord* rec =
                        bsp::plane_squadron_registry().find_by_member_unit(squadron_index);
                    const std::uint32_t vehicle_class = rec != nullptr
                        ? static_cast<std::uint32_t>(rec->type_class_id)
                        : static_cast<std::uint32_t>(squadron.row.type_id);
                    const std::int32_t count = rec != nullptr
                        ? static_cast<std::int32_t>(rec->member_units.size()) : 1;
                    bsp::air_ops_push_assign_queue_006cc7b0(*deck,
                        static_cast<std::uint32_t>(squadron_index + 1), vehicle_class, count);
                    ++host.scene_home_queue_pushes;
                    host.log.notef("squadron scene home base: squadron=%s HomeBase=\"%s\" "
                        "stored at +404h and queued on its deck (007F1C00 at 007F4CFA, "
                        "006CC7B0; packet cc9_scene_home_base_key)",
                        squadron.row.name.c_str(), key.c_str());
                    host.done("Squadron::set_home_air_base_007f1c00", 0x007f1c00u);
                }
            }
        }
        // Packet cc9_squadron_member_placement: 007F4DA9 tests +408h (set by the
        // base launch above) and 007F4DB0 calls 007F2920 for an airborne squadron.
        if constexpr (Impl::kSquadronPassCPlacementBound) {
            const bsp::PlaneSquadronHostRecord* placed_rec =
                bsp::plane_squadron_registry().find_by_member_unit(squadron_index);
            if (placed_rec != nullptr && !placed_rec->home_launch_408) {
                ++host.squadron_member_place_pass_c;
                host.place_squadron_members_007f2920(squadron_index, "pass_c");
            }
        }
        // 007F4E0C 0071BE40: a current command skips the block.
        if (director_current_command_0071be40(squadron_index) != 0u) {
            ++host.initial_command_skipped_current;
            return;
        }
        // 007F4E19..007F4E24: lobby mode 2 skips; single player is 0.
        const bsp::PlaneSquadronHostRecord* sq =
            bsp::plane_squadron_registry().find_by_member_unit(squadron_index);
        if (sq == nullptr || sq->member_units.empty()) return;
        std::vector<std::size_t> live;
        for (const std::size_t m : sq->member_units) {
            if (m == bsp::kPlaneSquadronNoUnit || m >= host.slots.size()) continue;
            if (!host.slots[m]->row.active) continue;
            live.push_back(m);
        }
        if (live.empty()) return;
        const std::size_t first = sq->member_units.front();
        const bool on_water = first < host.slots.size() &&
            host.slots[first]->plane_control_mode_900 == 6;   // 007F4E30
        bsp::SceneCommandTarget target;
        std::uint32_t command = 0;
        std::string target_name;
        if (on_water) {
            // 007F4E87 calls 007EF8F0 before 00468560 builds the stop target
            // (bsp.py show 007f4ba0 --asm, 007F4E5D..007F4E97). The host's
            // fused squadron/leader supplies +FCh; the Map load publishes the
            // selected world+711Ch (NW) and +7128h (SE) corners on the Lua host.
            const bsp::WorldMapBounds* bounds = host.lua.world_border_bounds();
            if (bounds == nullptr) {
                host.log.unimplemented("Squadron::initial_command missing world bounds", "007ef8f0");
                return;
            }
            const GameUnitSlot& leader = *host.slots[squadron_index];
            target.kind = 0;
            target.position_valid = 1;
            bsp::band_007a_cc13::world_clamp_point_to_map_bounds_007ef8f0(
                leader.motion.position,
                bounds->north_west[0], bounds->north_west[2],
                bounds->south_east[0], bounds->south_east[2], target.position);
            host.done("Squadron::initial_command clamp point", 0x007ef8f0u);
            command = 0x00E08F88u;                              // stop
            target_name = "(position)";
        } else {
            // 007F4E39..007F4E56: the home base +404h (007F1C00). The host
            // knows it for an air-ops launch: the deck whose slot launched this
            // squadron (entity id = index + 1) is owned by the home base.
            std::string home;
            if constexpr (Impl::kSceneHomeBaseBound) {
                // A scene row's `HomeBase`, stored by 007F1C00 above.
                home = host.slots[squadron_index]->scene_home_base_404;
            }
            const std::uint32_t id = static_cast<std::uint32_t>(squadron_index + 1);
            bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
            for (std::size_t d = 0; d < decks.size() && home.empty(); ++d) {
                const bsp::AirOpsDeck* deck = decks.mutable_at(d);
                if (deck == nullptr) continue;
                for (const auto& slot : deck->slots) {
                    if (slot.launched_squadron == id) {
                        home = decks.name_at(d);
                        break;
                    }
                }
            }
            if (home.empty()) {
                ++host.initial_command_no_home;
                return;
            }
            // kAirOpsOwnerByInstanceBound: the live instance of the home deck.
            const std::size_t home_index = host.air_ops_owner_unit(home);
            if (home_index >= host.slots.size()) {
                ++host.initial_command_no_home;
                return;
            }
            // 00465080(home, 0.0f, 1): an object descriptor.
            target.kind = 1;
            target.position_valid = 0;
            target.object_id = static_cast<std::uint16_t>(home_index + 1);
            target.object = const_cast<void*>(unit_identity(home_index));
            command = 0x00E08F68u;                              // moveto
            target_name = home;
        }
        // 007F4E9C 0077D600 on the squadron, flags 1; its MT_COMMAND fans out
        // to the members (007ECF80's shape), one order per live member here.
        std::size_t placed = 0;
        for (const std::size_t m : live) {
            if (issue_script_command(m, command, target, 1, "squadron_pass_c", target_name)
                    != nullptr) {
                ++placed;
            }
        }
        host.initial_command_member_orders += placed;
        if (on_water) ++host.initial_command_stops; else ++host.initial_command_movetos;
        host.log.notef("squadron initial command 007F4E9E: squadron=%s %s -> %s, %zu member "
            "order(s) (packet cc9_squadron_initial_command)", host.slots[squadron_index]->row.name.c_str(),
            on_water ? "stop" : "moveto", target_name.c_str(), placed);
        host.done("Squadron::initial_command_007f4e9e", 0x007f4e9eu);
    }
}

void GameUnitsHost::set_squadron_scene_home_base(std::size_t squadron_index,
    const std::string& home_base) {
    // Packet cc9_scene_home_base_key: the scene row's `HomeBase` value, held on
    // the squadron until its pass C reads it (the image keeps the row's bag at
    // squadron+C0h and reads the key at 007F4C43). Stored in both builds; only
    // the ON build acts on it.
    if (squadron_index >= impl_->slots.size()) return;
    impl_->slots[squadron_index]->scene_home_base_key = home_base;
    if (!home_base.empty()) ++impl_->scene_home_keys_set;
}

void GameUnitsHost::set_unit_leak_inputs(std::size_t index, float health_fraction,
    bool class_repair) {
    if (index >= impl_->slots.size()) return;
    GameUnitSlot& s = *impl_->slots[index];
    s.leak_health_fraction = health_fraction;
    s.class_repair_00d0 = class_repair;
}

bool GameUnitsHost::add_leak_0074f440(std::size_t index, std::uint32_t count,
    const float world_point[3]) {
    // Packet cc9_live_hull_leak. The 90h arm 008221A7 and its callee chain:
    //   008221A7  FILD [msg+1Ch], unsigned fixup (00CE3978), FMUL 10.0 (00CE3DC0)
    //   0074F440  refresh the owner pose, transform msg+20h by pose+CCh, call 0074F090
    // SUBSTITUTION, labelled: the sender takes the hit's world point into the
    // hull frame (00414E10 / the cached inverse at unit+110h) and 0074F440 takes
    // it back with pose+CCh; the host applies the message at once on one pose,
    // so the world point is passed through unchanged.
    Impl& host = *impl_;
    ++host.hull_leak_calls;
    if constexpr (!Impl::kLiveHullLeakBound) {
        static_cast<void>(index);
        static_cast<void>(count);
        static_cast<void>(world_point);
        host.record("LeakManager::add_leak_0074f440", 0x0074f440u);
        return false;
    } else {
        if (index >= host.slots.size()) return false;
        GameUnitSlot& s = *host.slots[index];
        if (!s.leak_ready) {
            ++host.hull_leak_no_model;
            return false;
        }
        const float amount = static_cast<float>(static_cast<double>(count) * 10.0);
        // 0074F09A..0074F0BF: a live unit (+5Ch set, +5Dh / +60h / +5Eh clear)
        // takes the leak only when its class `Repair` byte (+D0h) is set.
        // SUBSTITUTION, labelled: the units host does not load `Repair`, so every
        // class answers 0 here and no live hull floods; a wreck (+5Dh set) always
        // takes the leak. In the image the byte is 1 unless the class authors
        // Repair = false (00962DCF..00962E16: nil -> 1), so this holds back live
        // flooding on every class but Kaiten in this installation.
        const bool live = s.state != nullptr && s.state->active != 0 &&
                          s.state->simulate == 0 && s.scene_destroyed_005e == 0 &&
                          s.scene_pending_destroy_0060 == 0;
        const bool class_repair = Impl::kLiveHullRepairBound && s.class_repair_00d0;
        if (live && !class_repair) {
            ++host.hull_leak_gated_live;
            host.done("LeakManager::add_leak_0074f440", 0x0074f440u);
            return false;
        }
        // 0074F0D5..0074F102: d = point - pose translation (pose+FCh).
        const float d[3] = {world_point[0] - s.motion.position[0],
                            world_point[1] - s.motion.position[1],
                            world_point[2] - s.motion.position[2]};
        // 0074F10F..0074F170: along = forward . d (row 2 of pose+CCh, stored as a
        // float), plus Length * 0.5 (00D7A280).
        const float* f = s.motion.pose_row2;
        // pc24w2-units (kPc24UnitsHostCc13Bound): 0074F12F f1*d1, 0074F142 f0*d0,
        // 0074F146 FADDP, 0074F154 f2*d2, 0074F158 FADDP, each at PC24, stored
        // float at 0074F15A. OFF sums in double.
        namespace x8 = bsp::cc13x87;
        float along = 0.0f;
        if constexpr (bsp::kPc24UnitsHostCc13Bound) {
            along = static_cast<float>(x8::x87_add24(
                x8::x87_add24(x8::x87_mul24(f[1], d[1]), x8::x87_mul24(f[0], d[0])),
                x8::x87_mul24(f[2], d[2])));
        } else {
        along = static_cast<float>(static_cast<double>(f[1]) * d[1] +
            static_cast<double>(f[0]) * d[0] + static_cast<double>(f[2]) * d[2]);
        }
        const float shifted = static_cast<float>(static_cast<double>(along) +
            static_cast<double>(s.motion_class.hull_length) * 0.5);
        // 0074F174..0074F193: 00BF7420 truncates; below 0 -> 0; at or past
        // count/2 -> count/2 - 1.
        const int half = static_cast<int>(s.leaks.size()) >> 1;
        int leak = 0;
        if constexpr (bsp::kPc24UnitsHostCc13Bound) {
            // pc24w2-units: 0074F178 FDIV m32 at PC24, then 00BF7420 truncates
            // that 24-bit quotient.
            leak = s.leak_station_04 != 0.0f
                ? static_cast<int>(x8::x87_div24(shifted, s.leak_station_04)) : 0;
        } else {
        leak = s.leak_station_04 != 0.0f
            ? static_cast<int>(static_cast<double>(shifted) / s.leak_station_04) : 0;
        }
        if (leak < 0) leak = 0; else if (leak >= half) leak = half - 1;
        // 0074F196..0074F1AB: the +x side takes the second half.
        double side = 0.0;
        if constexpr (bsp::kPc24UnitsHostCc13Bound) {
            // pc24w2-units: 0074F196 d0*f2, 0074F198 f0*d2, 0074F19A FSUBP (DE E9,
            // ST1 - ST0), each at PC24; FCOMIP against 0 (JBE keeps the first half).
            side = x8::x87_sub24(x8::x87_mul24(d[0], f[2]), x8::x87_mul24(f[0], d[2]));
        } else {
        side = static_cast<double>(d[0]) * f[2] - static_cast<double>(f[0]) * d[2];
        }
        if (side > 0.0) leak += half;
        // 0074F4A1..0074F4EA: k = ln(sqrt(DologSzorzo)) / ln(2.0), stored as a float.
        float k = 0.0f;
        if constexpr (bsp::kPc24UnitsHostCc13Bound) {
            // pc24w2-units: 0074F4A1 00BF7030 sqrt (FSQRT at PC24) stored float,
            // 0074F4AE 00BFA910 ln stored float, 0074F4C5 ln(double [00D7A308] 2.0)
            // stored float, 0074F4D6 FDIVR m32 at PC24, stored at [00E1AECC].
            const float root = static_cast<float>(
                x8::x87_sqrt24(static_cast<float>(Impl::kDologSzorzoSubstitute)));
            const float ln_root = static_cast<float>(std::log(static_cast<double>(root)));
            const float ln_two = static_cast<float>(std::log(2.0));
            k = static_cast<float>(x8::x87_div24(ln_root, ln_two));
        } else {
        k = static_cast<float>(
            std::log(static_cast<double>(static_cast<float>(
                std::sqrt(static_cast<double>(Impl::kDologSzorzoSubstitute))))) /
            std::log(2.0));
        }
        // 0074F1AD..0074F22C: u = |D / R| ^ (1 / k), 0 when D / R is 0.
        const float ratio = static_cast<float>(
            static_cast<double>(s.leak_total_rate_2c) / s.leak_rate_34);
        const float inv_k = static_cast<float>(1.0 / static_cast<double>(k));
        // pc24w2-units: the inline power (0074F208..0074F21C and 0074F286..
        // 0074F29A): FYL2X y*log2(x), FRNDINT, FSUB ST1,ST0 at PC24 (the
        // fraction rounds to 24 bits), F2XM1, FLD1 FADDP at PC24, FSCALE, stored
        // float. FYL2X and F2XM1 are not narrowed by precision control; they are
        // modelled in double here (labelled: the image keeps 64-bit significands).
        auto pow_0074f208 = [](float x, float y) {
            const double t = static_cast<double>(y) * std::log2(static_cast<double>(x));
            const double n = std::nearbyint(t);
            const double frac = x8::x87_sub24(t, n);
            const double m = x8::x87_add24(std::expm1(frac * 0.69314718055994531), 1.0);
            return static_cast<float>(std::ldexp(m, static_cast<int>(n)));
        };
        float u = 0.0f;
        if (ratio != 0.0f) {
            if constexpr (bsp::kPc24UnitsHostCc13Bound) {
                u = pow_0074f208(std::fabs(ratio), inv_k);
            } else {
            u = static_cast<float>(std::exp2(static_cast<double>(inv_k) *
                std::log2(std::fabs(static_cast<double>(ratio)))));
            }
        }
        // 0074F22C..0074F24B: z = (T * u + amount) / T, T = +38h.
        const float t = s.leak_damage_to_death_38;
        const float tu = static_cast<float>(static_cast<double>(t) * u);
        float z = 0.0f;
        if constexpr (bsp::kPc24UnitsHostCc13Bound) {
            // pc24w2-units: 0074F245 FADD m32 (amount, stored float at [ESP+20h])
            // at PC24, then 0074F249 FDIVRP (DE F1, ST0 / ST1 = sum / T) at PC24,
            // stored float at 0074F24B.
            z = static_cast<float>(x8::x87_div24(x8::x87_add24(tu, amount), t));
        } else {
        z = static_cast<float>(
            (static_cast<double>(tu) + amount) / static_cast<double>(t));
        }
        // 0074F24F..0074F2A4: w = |z| ^ k, 0 when z is 0.
        float w = 0.0f;
        if (z != 0.0f) {
            if constexpr (bsp::kPc24UnitsHostCc13Bound) {
                w = pow_0074f208(std::fabs(z), k);
            } else {
            w = static_cast<float>(std::exp2(static_cast<double>(k) *
                std::log2(std::fabs(static_cast<double>(z)))));
            }
        }
        // 0074F2AA..0074F2D1: D' = w * R; rates[leak] += D' - D; +2Ch = D'.
        const float next = static_cast<float>(static_cast<double>(w) * s.leak_rate_34);
        const float delta = static_cast<float>(
            static_cast<double>(next) - s.leak_total_rate_2c);
        s.leak_total_rate_2c = next;
        s.leaks[static_cast<std::size_t>(leak)].rate = static_cast<float>(
            static_cast<double>(s.leaks[static_cast<std::size_t>(leak)].rate) + delta);
        ++host.hull_leak_applied;
        if (!live) ++host.hull_leak_applied_wreck;
        host.hull_leak_rate_added += delta;
        host.done("LeakManager::add_leak_0074f440", 0x0074f440u);
        return true;
    }
}

bool GameUnitsHost::ship_wreck_sink_00824fe5(const void* identity) {
    if constexpr (!Impl::kShipSinkDescentBound) {
        static_cast<void>(identity);
        return false;
    } else {
        for (const auto& owned : impl_->slots) {
            if (owned.get() != identity) continue;
            // Packet cc9_wreck_pre_sink: 0082524B, the handler's throttle cut.
            if constexpr (bsp::kWreckThrottleCutBound) bsp::wreck_throttle_cut_0082524b(owned->ring);
            if (!owned->leak_ready) return false;
            impl_->ship_wreck_sink_block_00824fe5(*owned);
            return true;
        }
        return false;
    }
}

bool GameUnitsHost::unit_pending_destroy_0060(std::size_t index, bool& out) const noexcept {
    if (index >= impl_->slots.size()) return false;
    const GameUnitSlot& slot = *impl_->slots[index];
    if (!slot.scene_flags_available) return false;
    out = slot.scene_pending_destroy_0060 != 0;
    return true;
}

bool GameUnitsHost::store_pending_destroy_0060(const void* identity, bool pending) noexcept {
    for (const auto& owned : impl_->slots) {
        if (owned.get() != identity) continue;
        if (!owned->scene_flags_available) return false;
        owned->scene_pending_destroy_0060 = pending;
        return true;
    }
    return false;
}

bool GameUnitsHost::unit_is_kind_of(std::size_t index, int class_id) const {
    if (index >= impl_->slots.size()) return false;
    const GameUnitSlot& slot = *impl_->slots[index];
    // The scene factory above publishes this canonical class_id from the
    // resolved VehicleClass.Type (or 36h for a StationaryClass prop).
    // Rebase a byte view WITHIN that live slot so receiver+C4 aliases the
    // original cell. No copied id, shadow receiver, or synthetic vtable is
    // needed. These four bodies read no other receiver bytes.
    // Evidence: bsp.py show 00748b40/0074dd90/006f5890/006f58e0 --asm:
    // fixed ancestor compares, then exactly one DWORD[ECX+C4] compare;
    // native ABI is ECX receiver, query at ESP+4, EAX 0/1, RET4. The fort
    // and building entries preserve that ABI through their fastcall bridge;
    // the other two expose the existing C++ byte-view interface. This is
    // solely a field-view adapter, not an original whole-unit ABI claim.
    static_assert(sizeof(slot.class_id) == sizeof(std::uint32_t));
    static_assert(offsetof(GameUnitSlot, class_id) >= 0xc4);
    const auto* const receiver = reinterpret_cast<const std::uint8_t*>(&slot)
        + offsetof(GameUnitSlot, class_id) - 0xc4;
    const auto query = static_cast<std::uint32_t>(class_id);
    switch (slot.class_id) {
    case 0x19:
        return bsp::cc13_seg41::land_vehicle_is_kind_of_0074dd90(receiver, query);
    case 0x1b:
        return bsp::native_mlandfort_is_kind_006f5890(receiver, nullptr, query) != 0;
    case 0x1c:
        return bsp::native_mcommandbuilding_is_kind_006f58e0(receiver, nullptr, query) != 0;
    case 0x36:
        return bsp::cc13_seg41::stationary_is_kind_of_00748b40(receiver, query);
    default:
        return bsp::unit_is_kind_of(slot.class_id, class_id);
    }
}

bool GameUnitsHost::inferior_failure_720(std::size_t index) const {
    if (index >= impl_->slots.size()) return false;
    return impl_->slots[index]->inferior_failure_steps != 0;
}

void GameUnitsHost::set_inferior_failure_720(std::size_t index) {
    if (index >= impl_->slots.size()) return;
    // 0095AD0A: MOV byte [ESI+720h], 1 on message 7Dh (00953DA0, "InferiorFailure").
    ++impl_->slots[index]->inferior_failure_steps;
}

void GameUnitsHost::clear_inferior_failure_720(std::size_t index) {
    if (index >= impl_->slots.size()) return;
    // 0095AD7F, message 7Eh in 0095ABE0 (vtable[19Ch], "InferiorFailure" stopped): +720h = 0 and
    // +728h = 0 (the timer lives in the gunnery host).
    impl_->slots[index]->inferior_failure_steps = 0;
}

std::size_t GameUnitsHost::parent_link_71c(std::size_t index) {
    if (index >= impl_->slots.size()) return impl_->slots.size();
    impl_->ensure_parent_link_71c_006f5cc0(index);
    const int link = impl_->slots[index]->parent_link_71c;
    return link > 0 ? static_cast<std::size_t>(link - 1) : impl_->slots.size();
}

GameUnitsHost::ShipDeathMessageClass GameUnitsHost::ship_death_message_class(
    std::size_t index) {
    ShipDeathMessageClass out;
    if (index >= impl_->slots.size()) return out;
    const GameUnitSlot& slot = *impl_->slots[index];
    out.ship = bsp::unit_is_kind_of(slot.class_id, bsp::kUnitGunneryKindShipBase);
    if (!out.ship) return out;
    out.mass = slot.motion_class.hull_mass;                                   // class+B0h
    const Impl::ClassHullBox& hull = impl_->class_hull_box(slot);
    out.front_back = hull.ok && hull.box.front_node && hull.box.back_node;
    const int type_id = slot.row.type_id;
    out.kamikaze = impl_->lua.read_vehicle_class_number(type_id, "KamikazeDamage", 0.0f) > 0.0f
        || impl_->lua.read_vehicle_class_number(type_id, "KamikazeBlastDamage", 0.0f) > 0.0f;
    return out;
}

bool GameUnitsHost::kill_and_remove_00814595(std::size_t index) {
    if (index >= impl_->slots.size()) return false;
    Impl& host = *impl_;
    GameUnitSlot& slot = *host.slots[index];
    if (slot.scene_removed_005f != 0) return false;
    // 00926D90(1): +5Fh, and Destroy (vt[70h]) when +60h is clear.
    slot.scene_removed_005f = 1;
    if (slot.scene_pending_destroy_0060 == 0) {
        slot.scene_pending_destroy_0060 = 1;
        host.record("Entity::destroy_at_kill_00926e05", 0x00926e05u);
    }
    // SUBSTITUTION, labelled, as at KillDepth: the kill list's row-15 drain
    // (009274A1..0092751F) applied here, the removal 009263C0 and the vt[80h] unlink.
    bsp::SceneNodeFlags flags;
    if (unit_scene_node_flags(index, flags) && !flags.destroyed) {
        bsp::scene_node_remove_009263c0(flags);
        slot.state->active = flags.active;
        slot.state->simulate = flags.torn_down;
        slot.scene_destroyed_005e = flags.destroyed;
        slot.scene_removed_005f = flags.removed;
        slot.row.active = flags.active;
        host.record("Entity::on_killed_lua_self_00928c80", 0x00928c80u);
        host.lua.on_entity_killed_00928c80(index);   // p16_truly_dead
        host.world_lists_unlink_unit_00928f24(slot);
    }
    host.log.notef("ship death message: unit=%s kamikaze Kill(1) and removal t=%.2f "
        "unlinked_nodes=%zu (00814595, packet cc9_ship_death_message)", slot.row.name.c_str(),
        static_cast<double>(host.summary.simulated_seconds), slot.world_nodes_unlinked);
    host.done("ShipDeath::kamikaze_kill_00814595", 0x00814595u);
    host.record("UnitController::clear_hull_shape_fields", 0x0092bd30u);   // 0081459B JMP
    return true;
}

void GameUnitsHost::run_airfield_destruction_rule_006d40f0(std::size_t index) {
    if (index >= impl_->slots.size()) return;
    if constexpr (Impl::kAirfieldDestructionRuleBound) {
        impl_->run_airfield_destruction_rule_006d40f0(index);
    }
}

// Packet cc9_unit_class_lands_troops. Slot 2Ch of the class descriptor vtables
// (disk bytes, kVehicleClassDescriptorTable vtables + 2Ch): 00827FB0 in the
// Destroyer, Cruiser, Cargo, BattleShip, Submarine, TorpedoBoat and MotherShip
// descriptors and in the ship base at 00D1ACC4 (eight .rdata xrefs), 00963C70 in
// MLandingShip's 00D1AD78 (the only dword 00963C70 in the image).
//   00827FB0: cmp [ecx+78Ch],0 / jnz / xor al,al; ret / cmp [ecx+790h],0; setnz al
//   00963C70: xor eax,eax / cmp [ecx+809h],al / sete al
// Plane, land and building descriptors were not read at slot 2Ch; they answer
// false here (uncertain; the known callers reach ship group members).
bool GameUnitsHost::unit_class_lands_troops_vtable_2c(std::size_t index) const {
    if (index >= impl_->slots.size()) return false;
    const GameUnitSlot& slot = *impl_->slots[index];
    switch (static_cast<bsp::VehicleClassKind>(slot.class_id)) {
    case bsp::VehicleClassKind::ShipBase:
    case bsp::VehicleClassKind::Destroyer:
    case bsp::VehicleClassKind::Submarine:
    case bsp::VehicleClassKind::MotherShip:
    case bsp::VehicleClassKind::Cruiser:
    case bsp::VehicleClassKind::Cargo:
    case bsp::VehicleClassKind::BattleShip:
    case bsp::VehicleClassKind::TorpedoBoat:
        return slot.fields.landing_ship_class != 0 && slot.fields.landing_ship_amount != 0;
    case bsp::VehicleClassKind::LandingShip:
        return !slot.landing_ship_is_rocketer_0809;
    default:
        return false;
    }
}

// Packet cc9_plane_in_flight_test. 007BB9A0's inputs (docs/PILOT_COMMAND_PATH.md,
// docs/IN_GAME_INTERFACE_SCREEN_SETS.md):
//  1. +C0Ch: its last writer on an airborne plane is 007C11E0 (the flight-state
//     notify of 007C1430/007C6340/007C7110 and the rest), which stores 1 when
//     +900h is 7, 6 or 3, or 4 with +904h clear (007C13CB..007C140C).
//     SUBSTITUTION, labelled: this host keeps no +C0Ch byte, so the rule is
//     applied to the current plane_control_mode_900; the other writers
//     (007B8C30 through 008A5BD0, 007C6760's launch, the ground roll 007CC0E5)
//     are not modelled. +904h (the landing byte) is not carried: taken clear.
//  2. +AA0h: 5.0f for a ShipYardLaunch (007D6355/007D645A), else -1.0f or the
//     0.0f of 007D614B. SUBSTITUTION, labelled: not carried, taken as 0.0f.
//  3. [+DECh]+44h/+48h: the actuator block's channel A (the host's block).
//  4. [+9D4h]+3B0h: only ever cleared (007F2DD3, 007EFB69): false.
//  5. 00604A20(unit): the same flight-state test, true outside those states.
//  6. +5Dh.
bool GameUnitsHost::plane_local_input_gate_007bb9a0(std::size_t index,
    bsp::PilotCmdLocalInputGate& gate) const {
    if (index >= impl_->slots.size()) return false;
    const GameUnitSlot& u = *impl_->slots[index];
    if (!bsp::unit_is_kind_of(u.class_id, 0x0f)) return false;
    const std::int32_t mode = u.plane_control_mode_900;
    constexpr bool kLanding904 = false;
    const bool active = mode == 7 || mode == 6 || mode == 3 || (mode == 4 && !kLanding904);
    gate.enable_byte_c0c = active;
    gate.gate_float_aa0 = 0.0f;
    if constexpr (bsp::kPlaneShipyardLaunchAa0Cc13Bound) {
        gate.gate_float_aa0 = u.plane_shipyard_launch_aa0;   // packet cc13-takeoff (007BB9B4)
    }
    gate.gate_block_present = u.actuator_block_dec.channel_a.enabled;
    gate.gate_block_value = u.actuator_block_dec.channel_a.value;
    gate.controller_present = false;
    gate.controller_blocks = false;
    // 00604A20: cc13_sweepB_interface_allows_input_00604a20 over the flight state +900h
    // and the landing byte +904h (plane_landed_904); OFF keeps !active, which takes
    // +904h clear.
    gate.global_block_00604a20 = bsp::cc13_sweepB_interface_allows_input_00604a20(
        mode, static_cast<std::uint8_t>(u.plane_landed_904 ? 1u : 0u));
    gate.unit_suppressed = unit_flag_005d(index);
    return true;
}

int GameUnitsHost::unit_class_id(std::size_t index) const noexcept {
    if (index >= impl_->slots.size()) return -1;
    return impl_->slots[index]->class_id;
}

bool GameUnitsHost::unit_hit_node_detached(std::size_t index) const noexcept {
    if (index >= impl_->slots.size() || !impl_->slots[index]) return false;
    return impl_->slots[index]->plane_hit_node_detached;
}

bool GameUnitsHost::unit_alive_and_visible(std::size_t index) const {
    bsp::SceneNodeFlags flags;
    bool pending_destroy;
    if (!unit_scene_node_flags(index, flags) ||
        !unit_pending_destroy_0060(index, pending_destroy)) return false;
    // 0043f080: [+5Ch] != 0 && [+5Dh] == 0 && [+60h] == 0 && [+5Eh] == 0.
    return flags.active && !flags.torn_down && !pending_destroy && !flags.destroyed;
}

bool GameUnitsHost::unit_pose(std::size_t index, float right[3], float up[3],
    float forward[3], float translation[3]) const {
    if (index >= impl_->slots.size()) return false;
    const bsp::CameraMatrix& world = impl_->slots[index]->world;
    for (std::size_t lane = 0; lane < 3; ++lane) {
        right[lane] = world[lane];
        up[lane] = world[4 + lane];
        forward[lane] = world[8 + lane];
        translation[lane] = world[12 + lane];
    }
    return true;
}

// ---------------------------------------------------------------------------
// Milestone 2p: what the brain pre-pass and the drive's middle read off a unit
// ---------------------------------------------------------------------------

bool GameUnitsHost::world_bounds_box_00e188a8(float& min_x, float& max_x, float& min_z,
    float& max_z) const {
    min_x = 0.0f;
    max_x = 0.0f;
    min_z = 0.0f;
    max_z = 0.0f;
    // The world object at [00E188A8] is never built here, so there is no box.
    // Reporting its absence is what lets 0071C4F0's own rule run at its call
    // site without four comparisons against zeroes.
    return false;
}

bool GameUnitsHost::active_command_descriptor_0071eb60(std::size_t index,
    bsp::SceneCommandTarget& out, int& mode) const {
    return impl_->commands.active_command_descriptor_0071eb60(index, out, mode);
}

void GameUnitsHost::register_scene_marker_position(int id, const float world[3]) {
    if (id <= 0) return;
    impl_->scene_marker_positions[static_cast<std::uint32_t>(id)] =
        std::array<float, 3>{world[0], world[1], world[2]};
}

void GameUnitsHost::register_scene_marker_frame(int id, const float world[16]) {
    if (id <= 0) return;
    std::array<float, 16> m{};
    for (int i = 0; i < 16; ++i) m[static_cast<std::size_t>(i)] = world[i];
    impl_->scene_marker_frames[static_cast<std::uint32_t>(id)] = m;
}

bool GameUnitsHost::scene_marker_frame(std::uint32_t id, float world[16]) const {
    const auto found = impl_->scene_marker_frames.find(id);
    if (found == impl_->scene_marker_frames.end()) return false;
    for (int i = 0; i < 16; ++i) world[i] = found->second[static_cast<std::size_t>(i)];
    return true;
}

bool GameUnitsHost::scene_marker_position(std::uint32_t id, float world[3]) const {
    const auto found = impl_->scene_marker_positions.find(id);
    if (found == impl_->scene_marker_positions.end()) return false;
    world[0] = found->second[0];
    world[1] = found->second[1];
    world[2] = found->second[2];
    return true;
}

std::uint32_t GameUnitsHost::resolve_command_target_00521ea0(
    const bsp::SceneCommandTarget& target) const {
    return impl_->commands.resolve_command_target_00521ea0(target);
}

void GameUnitsHost::transform_by_unit_matrix_004142e0(std::size_t index, float in_x,
    float in_y, float in_z, float& out_x, float& out_y, float& out_z) const {
    out_x = in_x;
    out_y = in_y;
    out_z = in_z;
    if (index >= impl_->slots.size()) return;
    const std::array<float, 3> source{in_x, in_y, in_z};
    std::array<float, 3> result{};
    bsp::transform_point_004142e0(source, impl_->slots[index]->world, result);
    out_x = result[0];
    out_y = result[1];
    out_z = result[2];
}

// Packet cc9_plane_gun_pass: unit+BC9h, the latched gunFire the plane's fixed
// step hands to each enabled gun's SetTriggerHeld (007CE9A0-007CE9F4).
bool GameUnitsHost::plane_gun_trigger_bc9(std::size_t index) const {
    if (index >= impl_->slots.size()) return false;
    return impl_->slots[index]->plane_gun_fire_bc9;
}

// unit+C24h PilotFires as 007CD930 stores it (00604A60 CMP byte [ESI+C24h],0);
// true when the unit is not a plane slot. The field defaults to true and is set
// only in the plane arm of the class read (motion entry 007CE040), so a
// non-plane slot answers true.
bool GameUnitsHost::plane_pilot_fires_0c24(std::size_t unit_index) const {
    if (unit_index >= impl_->slots.size()) return true;
    return impl_->slots[unit_index]->plane_pilot_fires_c24;
}

const std::vector<bsp::HullContactEvent>& GameUnitsHost::hull_contact_events() const {
    return impl_->hull_terrain.contact_events();
}

int GameUnitsHost::unit_side_0054(std::size_t index) const {
    if (index >= impl_->slots.size()) return -1;
    // unit+54h is the party the scene record authored, which milestone 2h's
    // party_class_marks pass already carries on the row.
    return impl_->slots[index]->row.party;
}

float GameUnitsHost::director_target_hold_0040(std::size_t index) const {
    return impl_->commands.director_target_hold_0040(index);
}

int GameUnitsHost::director_leading_slot_categories_0071df83(std::size_t index, int* out,
    int max_out) const {
    return impl_->commands.director_leading_slot_categories_0071df83(index, out, max_out);
}

std::uint32_t GameUnitsHost::director_slot_command(std::size_t index, int slot_index) const {
    return impl_->commands.director_slot_command(index, slot_index);
}

const char* GameUnitsHost::command_name_of(std::uint32_t command_object) const {
    return impl_->commands.command_name_of(command_object);
}

const bsp::ShipAiAutoThrustSettings& GameUnitsHost::auto_thrust_settings(bool& loaded) const {
    loaded = impl_->auto_thrust_loaded;
    return impl_->auto_thrust;
}

bsp::ShipAiThrottleCeilingInputs GameUnitsHost::throttle_ceiling_inputs(
    std::size_t index) const {
    bsp::ShipAiThrottleCeilingInputs in{};
    if (index >= impl_->slots.size()) return in;
    const GameUnitSlot& slot = *impl_->slots[index];
    // 009EC97B, |unit+980h|: the order ring's published throttle. The absolute
    // value is taken inside the reconstruction, so the raw field goes in.
    in.live_throttle = slot.row.throttle;
    // 009EC99C / 009EC9A4, *(unit+73Ch)+28h and +24h: the commanded-speed pair
    // milestone 2m put on the navigator parameter block.
    const bsp::CruiseSpeedSetting speed = impl_->commands.commanded_speed(index);
    in.commanded_speed = speed.speed;
    in.commanded_speed_enabled = speed.enable >= 0.0f;
    // 009EC9AB, blk+3C4h: the cached reference speed, the same 0080FC30 value
    // 009E12BD divides by.
    in.reference_speed = bsp::unit_reference_speed_0080fc30(slot.motion.max_speed,
        unit_reference_speed_scale_0080fc30(index));
    return in;
}

float GameUnitsHost::unit_half_width_09cc(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    // Historical API name:00811010/00811062 produce FULL width.
    return impl_->slots[index]->hull_extents.width_09cc;
}

float GameUnitsHost::unit_class_max_speed_0500(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->fields.max_speed;
}

float GameUnitsHost::command_building_capture_range_07a0(std::size_t unit_index) const {
    // 006F2780 -> unit+7A0h; 00A03760 FILDs it. 1Ch is MCommandBuilding.
    if (unit_index >= impl_->slots.size()) return 500.0f;
    if (!unit_is_kind_of(unit_index, 0x1c)) return 500.0f;
    return static_cast<float>(impl_->slots[unit_index]->capture_range_7a0);
}

std::int32_t GameUnitsHost::command_building_level_0770(std::size_t unit_index) const {
    // Packet cc9_command_building_level: 006F2780 -> unit+770h.
    if (unit_index >= impl_->slots.size()) return 0;
    if (!unit_is_kind_of(unit_index, 0x1c)) return 0;
    return impl_->slots[unit_index]->level_770;
}

std::int32_t GameUnitsHost::command_building_level_up_seconds_076c(std::size_t unit_index) const {
    // Packet cc9_command_building_level: 006F2780 -> unit+76Ch.
    if (unit_index >= impl_->slots.size()) return 10;
    if (!unit_is_kind_of(unit_index, 0x1c)) return 10;
    return impl_->slots[unit_index]->level_up_seconds_76c;
}

std::int32_t GameUnitsHost::command_building_inferior_range_07c8(std::size_t unit_index) const {
    // Packet cc9_command_building_garrison: 006F288C -> unit+7C8h.
    if (unit_index >= impl_->slots.size()) return 200;
    return impl_->slots[unit_index]->inferior_range_7c8;
}

bool GameUnitsHost::unit_inferior_bag_0724(std::size_t unit_index) const {
    // Packet cc9_command_building_garrison: 009554DF's unit+724h clone.
    return unit_index < impl_->slots.size() && impl_->slots[unit_index]->bag_min_level_724;
}

void GameUnitsHost::paratrooper_capturers_07dc(std::size_t building_unit,
                                               std::vector<std::pair<int, float>>& out) const {
    // Packet cc13-para2: building+7DCh in 006F39B0 order, (round +54h, class +D8h).
    out.clear();
    std::vector<bsp::cc13_paratrooper::Capturer> list;
    impl_->para_world.capturers(static_cast<std::uint32_t>(building_unit + 1), list);
    for (const auto& c : list) out.emplace_back(c.side_54, c.capture_power_d8);
}

std::int32_t GameUnitsHost::unit_inferior_min_level_0724(std::size_t unit_index) const {
    return unit_index < impl_->slots.size() ? impl_->slots[unit_index]->min_level_724 : 0;
}

std::int32_t GameUnitsHost::command_building_capture_value_07a4(std::size_t unit_index) const {
    // 006F2780 -> unit+7A4h; 006F6760 compares |+7A8h| with (float)+7A4h.
    if (unit_index >= impl_->slots.size()) return 1000;
    if (!unit_is_kind_of(unit_index, 0x1c)) return 1000;
    return impl_->slots[unit_index]->capture_value_7a4;
}

void GameUnitsHost::set_unit_side_0054(std::size_t index, int party) {
    // Packet cc9_command_building_capture_bind: 00928F50 SetPartyRace's party
    // write, from a CommandBuilding's neutralize or flip (006F4D10).
    if (index >= impl_->slots.size()) return;
    impl_->slots[index]->row.party = party;
    if constexpr (kPartyChangeRefreshesGunneryBound) {
        if (impl_->gunnery != nullptr) impl_->gunnery->refresh_unit_side(index);
    }
}

void GameUnitsHost::set_unit_race(std::size_t index, int race) {
    // bsp.py show 00923B80: +58h is stored alongside the existing +54h party.
    // This is the row all current +58h consumers borrow, not a second image.
    if (index >= impl_->slots.size()) return;
    impl_->slots[index]->row.race_58 = race;
}

float GameUnitsHost::command_building_landing_range_07c4(std::size_t unit_index) const {
    // 006F2780 stores the scene LandingRange dword (006F2847) at unit+7C4h
    // (006F285F), 500 when unauthored; 009E6E80's mode-4 arm (009E6F4E FILD) and
    // the latch 009F20A4 read it. 1Ch is MCommandBuilding.
    if (unit_index >= impl_->slots.size()) return 500.0f;
    if (!unit_is_kind_of(unit_index, 0x1c)) return 500.0f;
    return static_cast<float>(impl_->slots[unit_index]->landing_range_7c4);
}

std::int32_t GameUnitsHost::command_building_landing_point_range_07cc(
    std::size_t unit_index) const {
    // Packet cc9_building_pad_model: 006F2780 stores the scene LandingPointRange
    // dword (006F2895) at unit+7CCh (006F28B3), 500 when unauthored; 006F5CC0's
    // third pass squares it as an int32 (006F6617). 1Ch is MCommandBuilding.
    if (unit_index >= impl_->slots.size()) return 500;
    if (!unit_is_kind_of(unit_index, 0x1c)) return 500;
    return impl_->slots[unit_index]->landing_point_range_7cc;
}

float GameUnitsHost::plane_class_max_speed_0188(std::size_t unit_index) const {
    if (unit_index >= impl_->slots.size()) return 0.0f;
    if (unit_is_kind_of(unit_index, 0x18)) {
        // [squadron+35Ch] is the plane class descriptor (docs/PLANE_SQUADRON.md);
        // its members carry that class, so a member's value is the class value.
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit != unit_index) continue;
            for (std::size_t m : r.member_units) {
                if (m < impl_->slots.size() && impl_->slots[m]->plane_max_spd > 0.0f) {
                    return impl_->slots[m]->plane_max_spd;
                }
            }
            return 0.0f;
        }
        return 0.0f;
    }
    if (unit_is_kind_of(unit_index, 0x0f)) return impl_->slots[unit_index]->plane_max_spd;
    return 0.0f;
}

float GameUnitsHost::unit_class_max_rot_angle_04f8(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->fields.max_rot_angle;
}

float GameUnitsHost::unit_class_max_accel_0504(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_class.max_accel;
}

float GameUnitsHost::unit_class_turn_radius_0520(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    // class+520h, the field 00828F20 derives once per class at 00828F66 from
    // MaxSpeed and MaxRotAngle. 0082E970 is its second reader, beside 0082E850.
    const bsp::ShipClassFields& fields = impl_->slots[index]->fields;
    const bsp::ShipClassAiDerivedMotion out = bsp::ship_class_ai_derived_motion_00828f20(
        fields.max_rot_angle, fields.max_rot_angle_change_ratio, fields.max_speed);
    return out.derived ? out.turn_radius_0520 : 0.0f;
}

float GameUnitsHost::unit_class_turn_circle_radius_0082e960(std::size_t index,
    float throttle) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    GameUnitSlot& slot = *host.slots[index];
    bsp::ShipAiNavBlockClassInputs in{};
    in.max_rot_angle_04f8 = slot.fields.max_rot_angle;
    in.max_speed_0500 = slot.fields.max_speed;
    in.turn_radius_0520 = unit_class_turn_radius_0520(index);
    UnitRudderBinding rudder(host, slot);
    const float radius = bsp::ship_class_turn_circle_radius_0082e960(in, throttle, rudder);
    host.done("ShipAiNavBlock::class_turn_circle_0082e960", 0x0082e960u);
    return radius;
}

float GameUnitsHost::unit_hull_length_09c8(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->hull_extents.length_09c8;
}

bool GameUnitsHost::unit_ground_contact_1011(std::size_t index) const {
    if (index >= impl_->slots.size()) return false;
    return impl_->slots[index]->ground_contact_1011;
}

float GameUnitsHost::unit_hull_mass_00b0(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_class.hull_mass;
}

int GameUnitsHost::unit_hull_material(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0;
    return static_cast<int>(impl_->slots[index]->hull_material);
}

float GameUnitsHost::unit_hull_linear_damping(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_state.linear_damping;
}

float GameUnitsHost::unit_hull_angular_damping(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_state.angular_damping;
}

void GameUnitsHost::raise_turn_assist_load_102c(std::size_t index, float value) {
    if (index >= impl_->slots.size()) return;
    // 009D4FB0 inlined: a compare and a store, so the raise is the whole body.
    GameUnitSlot& slot = *impl_->slots[index];
    if (value > slot.turn_assist_load_102c) slot.turn_assist_load_102c = value;
}

void GameUnitsHost::raise_secondary_load_1034(std::size_t index, float value) {
    if (index >= impl_->slots.size()) return;
    GameUnitSlot& slot = *impl_->slots[index];
    if (value > slot.secondary_load_1034) slot.secondary_load_1034 = value;
}

float GameUnitsHost::turn_assist_load_102c(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->turn_assist_load_102c;
}

float GameUnitsHost::secondary_load_1034(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->secondary_load_1034;
}

void GameUnitsHost::unit_class_extents(std::size_t index, float& forward, float& right,
    float& up) const {
    forward = 0.0f;
    right = 0.0f;
    up = 0.0f;
    if (index >= impl_->slots.size()) return;
    const bsp::ShipMotionClass& motion_class = impl_->slots[index]->motion_class;
    forward = motion_class.hull_length;  // class+A0h
    up = motion_class.hull_height;       // class+A8h
    // class+A4h has no recovered Lua key, so it stays zero rather than being
    // guessed from either of the other two.
}

GameUnitsHost::VehicleClassLaunchKeys GameUnitsHost::vehicle_class_launch_keys(
    int type_id) {
    VehicleClassLaunchKeys out;
    const GameVehicleClassRow row = impl_->lua.read_vehicle_class_row(type_id);
    if (!row.found) return out;
    out.found = true;
    out.length_00a0 = row.length;
    out.width_00a4 = row.width;
    out.height_00a8 = row.height;
    out.landing_ship_078c = row.landing_ship_resolves ? row.landing_ship_id : 0;
    out.landing_ship_amount_0790 = row.landing_ship_amount;
    out.landing_ship_cool_down_0794 =
        impl_->lua.read_vehicle_class_integer(type_id, "LandingShipCoolDown", nullptr, 60);
    return out;
}

const GameUnitRow* GameUnitsHost::unit_row(std::size_t index) const noexcept {
    if (index >= impl_->slots.size()) return nullptr;
    GameUnitSlot& slot = *impl_->slots[index];
    // Lifecycle providers can write the borrowed byte without going through
    // store_scene_node_flags. Refresh this diagnostic snapshot at its reader.
    slot.row.active = slot.state != nullptr && slot.state->active != 0;
    return &slot.row;
}

bool GameUnitsHost::controlled_bound() const noexcept { return impl_->controlled_bound; }

void* const volatile& GameUnitsHost::controlled_unit_publication() const noexcept {
    return impl_->controlled_unit_00e188d8;
}

std::size_t GameUnitsHost::controlled_index() const noexcept { return impl_->controlled_index; }

const std::vector<GameUnitRow>& GameUnitsHost::units() const noexcept {
    // The rows live inside the slots; a flat copy is rebuilt on demand so the
    // caller sees one contiguous table.
    impl_->rows.clear();
    impl_->rows.reserve(impl_->slots.size());
    for (const std::unique_ptr<GameUnitSlot>& slot : impl_->slots) {
        slot->row.active = slot->state != nullptr && slot->state->active != 0;
        impl_->rows.push_back(slot->row);
    }
    return impl_->rows;
}

const GameUnitsSummary& GameUnitsHost::summary() const noexcept { return impl_->summary; }

void GameUnitsHost::log_controlled_trajectory(unsigned long long mission_frame) {
    Impl& host = *impl_;
    if (!host.controlled_bound || host.controlled_index >= host.slots.size()) return;
    const GameUnitRow& row = host.slots[host.controlled_index]->row;
    host.log.notef("  controlled unit frame %-4llu t=%7.2f  x=%10.2f z=%10.2f  "
        "heading=%8.3f  fwd=%7.3f  throttle=%6.3f rudder=%6.3f  yaw=%8.5f",
        mission_frame, static_cast<double>(host.summary.simulated_seconds),
        static_cast<double>(row.position[0]), static_cast<double>(row.position[2]),
        static_cast<double>(row.heading_degrees), static_cast<double>(row.forward_speed),
        static_cast<double>(row.throttle), static_cast<double>(row.rudder),
        static_cast<double>(row.yaw_rate));
}
}  // namespace bsp::game
