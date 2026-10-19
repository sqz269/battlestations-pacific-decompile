// Packet cc13-para2: the dropped MParatrooper round as a host-side world. See
// include/bsp/cc13_paratrooper_round.hpp; the per-routine models are
// src/gameplay_seg46_cc13.cpp.

#include "bsp/cc13_paratrooper_round.hpp"

#include "bsp/bomb_torpedo_tick.hpp"   // bomb_air_velocity (006E1510)

#include <algorithm>
#include <cstdio>

namespace bsp {
namespace cc13_paratrooper {

namespace {

namespace seg46 = gameplay_seg46_cc13;

float f32(double v) noexcept { return static_cast<float>(v); }

constexpr int kKillCause = 2;                     // every PUSH 2 before 00926D90 in 007AA000-007AD000
constexpr std::uint32_t kSoldierHandleBase = 0x00500000u;   // host handles, never a unit index + 1

}  // namespace

int party_relative_00803510(int subject, int reference) noexcept {
    if (subject == 2) return reference != 2 ? 2 : 0;   // 00803510: -(ref != 2) & 2
    if (subject == reference) return 0;
    return reference == 2 ? 2 : 1;
}

std::uint32_t nearest_other_side_building_006f2c30(const Vec3& position, int side,
                                                   RoundServices& svc) {
    // 006F2C30(ECX = &pos, EDX = side, kind 0, &out): the list [game+19CCh]+16Ch
    // in its own order; an entry of the caller's side is skipped (+54h ==
    // EDX). d2 = dz^2 + dy^2 + dx^2, each a float; kind 0 reads the int +7C8h,
    // converted to float and squared; `d2 <= r^2` admits; the first admitted
    // entry is taken, a later one only when strictly nearer.
    std::uint32_t best = 0;
    float best_d2 = 0.0f;
    const std::size_t n = svc.building_count();
    for (std::size_t i = 0; i < n; ++i) {
        const BuildingView b = svc.building(i);
        if (b.side_54 == side) continue;
        const float dx = b.position_fc.x - position.x;
        const float dy = b.position_fc.y - position.y;
        const float dz = b.position_fc.z - position.z;
        const float d2 = dz * dz + dy * dy + dx * dx;
        const float r = static_cast<float>(b.inferior_range_7c8);
        if (!(d2 <= r * r)) continue;
        if (best != 0 && !(d2 < best_d2)) continue;
        best = b.handle;
        best_d2 = d2;
    }
    return best;
}

// The ParatrooperHost the segment-46 models call, bound to one round.
class RoundAdapter final : public seg46::ParatrooperHost {
public:
    RoundAdapter(RoundWorld& world, Round& round, RoundServices& svc, float now)
        : world_(world), round_(round), svc_(svc), now_(now) {}

    float uniform(int stream, float lo, float hi) override { return svc_.uniform(stream, lo, hi); }
    // [[00E188A8]+1FE4h]: the host's session is single player (0).
    int session_mode() const override { return 0; }
    bool session_flag_218c() const override { return false; }
    Vec3 world_position() override { return round_.position; }
    float ground_height_at(const Vec3& point) override { return svc_.ground_height_00903860(point); }
    bool ground_normal_at(const Vec3& point, Vec3& normal) override {
        return svc_.ground_normal_009038f0(point, normal);
    }
    void avoid_zone_nearest_boundary(float xz[2], float search, float radius) override {
        if (!svc_.avoid_zone_nearest_0041b840(xz, search, radius))
            svc_.record("MParatrooper::avoid_zone_group_absent", 0x007abc9cu);
    }
    void bomb_advance_in_air(ParatrooperState& s, float dt) override {
        // 006E1510: the Paratrooper row sets no NoGravity (classDesc+20h), so
        // v.y -= 9.81f * dt and v += +348h * dt. The forward-row blend and
        // 0085DC80 that follow are the pose only (not modelled).
        const BombVector3 v{s.velocity_318.x, s.velocity_318.y, s.velocity_318.z};
        const BombVector3 a{round_.accel_348.x, round_.accel_348.y, round_.accel_348.z};
        const BombVector3 out = bomb_air_velocity(v, a, false, dt);
        s.velocity_318 = Vec3{out.x, out.y, out.z};
    }
    void extrapolate_position(ParatrooperState& s, float dt) override {
        // 006E2220: +A4h = +448h + t * v. +448h is the pose 006E1010 committed
        // last step, which is the position this host keeps.
        round_.position.x = f32(static_cast<double>(round_.position.x) +
                                static_cast<double>(dt) * s.velocity_318.x);
        round_.position.y = f32(static_cast<double>(round_.position.y) +
                                static_cast<double>(s.velocity_318.y) * dt);
        round_.position.z = f32(static_cast<double>(round_.position.z) +
                                static_cast<double>(dt) * s.velocity_318.z);
    }
    void orthonormalize_upright() override {}             // 0085DAD0: the pose only
    bool chute_anim_present() override { return true; }   // [[+464h]+0Ch]: the chute model
    void set_chute_anim(std::uint32_t, float) override {} // vslot +24h: presentation
    void kill_self(int cause) override {
        if (!round_.alive) return;
        round_.alive = false;
        ++world_.counters_.kills;
        // Name the arm from the state the model stopped in (all PUSH 2).
        const ParatrooperState& s = round_.s;
        const char* why = "capture_done";
        if (s.in_water_472) {
            why = "water";
            ++world_.counters_.kills_water;
        } else if (s.bad_landing_471) {
            why = "bad_landing";
            ++world_.counters_.kills_bad_landing;
        } else if (s.target_building_48c == 0) {
            why = "no_building";
            ++world_.counters_.kills_no_building;
        } else if (svc_.building_side_54(s.target_building_48c) == s.side_54) {
            why = "friendly_building";
            ++world_.counters_.kills_friendly_building;
        } else {
            ++world_.counters_.kills_capture_done;
        }
        round_.death = why;
        if (world_.counters_.kills <= 12) {
            char line[200];
            std::snprintf(line, sizeof(line), "paratrooper killed: id=%u cause=%d reason=%s t=%.2f "
                "pos=(%.1f %.1f %.1f) capture_timer=%.2f (00926D90, packet cc13-para2)",
                round_.id, cause, why, static_cast<double>(now_),
                static_cast<double>(round_.position.x), static_cast<double>(round_.position.y),
                static_cast<double>(round_.position.z), static_cast<double>(s.capture_timer_474));
            svc_.note(line);
        }
        // LABELLED: 00926D90 runs the entity's +7Ch (007AAE10) here, at once.
        seg46::on_killed_007aae10(round_.s, *this);
    }
    void kill_entity(std::uint32_t, int) override {
        // The soldier is not a host entity (see the header).
        svc_.record("MParatrooper::soldier_kill_not_modelled", 0x007aae2cu);
    }
    void on_killed_lua() override {
        // 0077D270: the round has no Lua name in any mission of this installation.
        svc_.record("MParatrooper::on_killed_lua_0077d270", 0x0077d270u);
    }
    int building_side(std::uint32_t building) override { return svc_.building_side_54(building); }
    float building_368(std::uint32_t building) override { return svc_.building_armour_368(building); }
    void building_damage(std::uint32_t building, float amount) override {
        ++world_.counters_.damage_calls;
        world_.counters_.damage_total += static_cast<double>(amount);
        svc_.building_damage_1ac(building, amount);
    }
    Vec3 building_position(std::uint32_t building) override {
        return svc_.building_position_fc(building);
    }
    int party_relative(int subject, int reference) override {
        return party_relative_00803510(subject, reference);
    }
    std::uint32_t find_enemy_building(const Vec3& pos, int side) override {
        const std::uint32_t b = nearest_other_side_building_006f2c30(pos, side, svc_);
        if (b != 0) ++world_.counters_.building_found;
        return b;
    }
    void register_fixed_step_once() override { round_.callback_registered = true; }
    void add_capturer(std::uint32_t building) override {
        // 006F39B0: append under the building's +764h lock.
        world_.capturer_list_.emplace_back(building, round_.id);
        ++world_.counters_.capturer_adds;
    }
    void remove_capturer(std::uint32_t building) override {
        // 006F34B0: erase this round from the building's array.
        auto& list = world_.capturer_list_;
        for (auto it = list.begin(); it != list.end(); ++it) {
            if (it->first == building && it->second == round_.id) {
                list.erase(it);
                ++world_.counters_.capturer_removes;
                return;
            }
        }
    }
    bool soldier_clip_visible(std::uint32_t, const Vec3&) override { return false; }  // 0048A010
    void soldier_show(std::uint32_t) override {}
    void soldier_muzzle_effect(std::uint32_t) override {}   // 0048A1E0: presentation
    void send_landing_message_71(const Vec3&) override {}   // session mode 1 only
    bool dropper_from_owner(std::int32_t&) override { return false; }

private:
    RoundWorld& world_;
    Round& round_;
    RoundServices& svc_;
    float now_;
};

ParatrooperState RoundWorld::construct(const soldier_traffic_cc13::SoldierGlobals& g, int side_54,
                                       const ChuteLengths& lengths, bool draw, RoundServices& svc) {
    ParatrooperState s{};
    ++counters_.constructs;
    if (draw) {
        // 007AB8C0 through the services' draw (stream 1, 007AB94A).
        struct DrawOnly final : public seg46::ParatrooperHost {
            explicit DrawOnly(RoundServices& v) : svc(v) {}
            RoundServices& svc;
            float uniform(int stream, float lo, float hi) override { return svc.uniform(stream, lo, hi); }
            int session_mode() const override { return 0; }
            bool session_flag_218c() const override { return false; }
            Vec3 world_position() override { return {}; }
            float ground_height_at(const Vec3&) override { return 0.0f; }
            bool ground_normal_at(const Vec3&, Vec3&) override { return false; }
            void avoid_zone_nearest_boundary(float*, float, float) override {}
            void bomb_advance_in_air(ParatrooperState&, float) override {}
            void extrapolate_position(ParatrooperState&, float) override {}
            void orthonormalize_upright() override {}
            bool chute_anim_present() override { return true; }
            void set_chute_anim(std::uint32_t, float) override {}
            void kill_self(int) override {}
            void kill_entity(std::uint32_t, int) override {}
            void on_killed_lua() override {}
            int building_side(std::uint32_t) override { return 0; }
            float building_368(std::uint32_t) override { return 0.0f; }
            void building_damage(std::uint32_t, float) override {}
            Vec3 building_position(std::uint32_t) override { return {}; }
            int party_relative(int, int) override { return 0; }
            std::uint32_t find_enemy_building(const Vec3&, int) override { return 0; }
            void register_fixed_step_once() override {}
            void add_capturer(std::uint32_t) override {}
            void remove_capturer(std::uint32_t) override {}
            bool soldier_clip_visible(std::uint32_t, const Vec3&) override { return false; }
            void soldier_show(std::uint32_t) override {}
            void soldier_muzzle_effect(std::uint32_t) override {}
            void send_landing_message_71(const Vec3&) override {}
            bool dropper_from_owner(std::int32_t&) override { return false; }
        } host(svc);
        seg46::construct_007ab8c0(s, g, host);
        ++counters_.construct_draws;
        // 007AC530 (+A0h): the animation handles are host tokens 1..3.
        seg46::load_animations_007ac530(s, 1u, lengths.fall, 2u, lengths.open, 3u, lengths.close, host);
    } else {
        // Without the draw the constructor's other stores still run; +468h is
        // the midpoint of the authored range (recorded, not drawn).
        s.open_altitude_468 = f32((static_cast<double>(g.parachute_open_altitude_min) +
                                   g.parachute_open_altitude_max) * 0.5);
        s.class_id_c4 = 0x31;
        s.dropper_4b8 = 8;
        s.anim_fall_49c = 1u;
        s.anim_open_4a4 = 2u;
        s.anim_close_4ac = 3u;
        s.fall_length_4a0 = lengths.fall;
        s.open_length_4a8 = lengths.open;
        s.close_length_4b0 = lengths.close;
        s.anim_current_498 = s.anim_fall_49c;
        svc.record("MParatrooper::construct_draw_untaken", 0x007ab94au);
    }
    // 006E3E70: the hung round takes the owner's +54h.
    s.side_54 = side_54;
    return s;
}

std::uint32_t RoundWorld::drop(const ParatrooperState& hung, const ParatrooperClassFields& cls,
                               const ReleaseState& release, bool has_landing, const Vec3& landing,
                               float now, RoundServices& svc) {
    Round r;
    r.id = next_id_++;
    r.s = hung;
    r.cls = cls;
    r.s.net_id_174 = static_cast<std::uint16_t>(r.id & 0xFFFFu);   // LABELLED: the host serial
    r.position = release.position;
    r.s.velocity_318 = release.velocity_318;
    r.accel_348 = Vec3{release.accel_348.x, 0.0f, release.accel_348.z};   // 006E1F00 zeroes y
    r.s.flying_458 = true;                                                 // 006E1FA3
    r.dropper_unit = release.dropper_unit;
    r.bullet_class = release.bullet_class;
    r.dropped_at = now;
    ++counters_.drops;
    rounds_.push_back(r);
    Round& placed = rounds_.back();
    if (has_landing) {
        RoundAdapter host(*this, placed, svc, now);
        seg46::set_landing_pos_007ab230(placed.s, landing, host);   // 006E54B2
    } else {
        svc.record("MParatrooper::drop_without_target_point", 0x006e541fu);
    }
    if (counters_.drops <= 8) {
        char line[260];
        std::snprintf(line, sizeof(line), "paratrooper drop: id=%u t=%.2f pos=(%.1f %.1f %.1f) "
            "v=(%.2f %.2f %.2f) open_alt=%.1f target=(%.1f %.1f %.1f) side=%d "
            "(006E4D50 -> 006E1F00 -> 007AB230, packet cc13-para2)",
            placed.id, static_cast<double>(now), static_cast<double>(placed.position.x),
            static_cast<double>(placed.position.y), static_cast<double>(placed.position.z),
            static_cast<double>(placed.s.velocity_318.x), static_cast<double>(placed.s.velocity_318.y),
            static_cast<double>(placed.s.velocity_318.z),
            static_cast<double>(placed.s.open_altitude_468), static_cast<double>(placed.s.target_47c.x),
            static_cast<double>(placed.s.target_47c.y), static_cast<double>(placed.s.target_47c.z),
            placed.s.side_54);
        svc.note(line);
    }
    return placed.id;
}

void RoundWorld::step(float dt, float now, const soldier_traffic_cc13::SoldierGlobals& g,
                      bool soldier_draw, RoundServices& svc) {
    if (rounds_.empty()) return;
    ++counters_.steps;
    // 00874DE0, the per-step callback list: 007AB590 for a round 007AC000
    // registered last step. The model clears +490h, so it runs once.
    for (Round& r : rounds_) {
        if (!r.alive || !r.callback_registered) continue;
        r.callback_registered = false;
        RoundAdapter host(*this, r, svc, now);
        const seg46::LandSoldierPlan plan = seg46::land_and_spawn_soldier_007ab590(r.s, r.cls, host);
        if (!plan.run) continue;
        // [[class+ECh]] vslot +8h makes the soldier (no null test follows, 007AB601).
        r.s.soldier_478 = kSoldierHandleBase + r.id;
        ++counters_.soldiers;
        // 007AB6AC 004B0F80(&class+F0h, random start 1, 0): U(0, duration) on
        // stream 0 at 004B102F when the soldier's +18Ch is set and the length is
        // positive. LABELLED: both are taken as true, and the bound is 1.0 (the
        // "fire2" length is not loaded); only the draw's place in stream 0 matters.
        if (soldier_draw) {
            svc.uniform(0, 0.0f, 1.0f);
            ++counters_.soldier_draws;
        } else {
            svc.record("MParatrooper::soldier_anim_draw_untaken", 0x004b102fu);
        }
        if (counters_.soldiers <= 8) {
            const Vec3 b = svc.building_position_fc(plan.building);
            char line[220];
            std::snprintf(line, sizeof(line), "paratrooper soldier: id=%u t=%.2f building=%u "
                "side=%d building_side=%d pos=(%.1f %.1f %.1f) building_at=(%.1f %.1f %.1f) "
                "(007AB590 -> 006F39B0, packet cc13-para2)",
                r.id, static_cast<double>(now), plan.building, r.s.side_54,
                svc.building_side_54(plan.building), static_cast<double>(r.position.x),
                static_cast<double>(r.position.y), static_cast<double>(r.position.z),
                static_cast<double>(b.x), static_cast<double>(b.y), static_cast<double>(b.z));
            svc.note(line);
        }
    }
    // Job wave 1: tick element +4h 006E1060 rescales the step by class+5Ch and
    // calls +194h (007AC000) while the shot query +2Ch answers 0.
    for (Round& r : rounds_) {
        if (!r.alive) continue;
        const bool was_landed = r.s.landed_470;
        const bool was_water = r.s.in_water_472;
        const bool was_bad = r.s.bad_landing_471;
        RoundAdapter host(*this, r, svc, now);
        seg46::step_landing_007ac000(r.s, r.cls, g, f32(static_cast<double>(r.cls.step_scale_5c) * dt),
                                     host);
        if (r.s.bad_landing_471 && !was_bad) ++counters_.bad_landings;
        if (r.s.in_water_472 && !was_water) ++counters_.in_water;
        if (r.s.landed_470 && !was_landed) {
            ++counters_.landed;
            r.landed_at = now;
            if (counters_.landed <= 8) {
                char line[220];
                std::snprintf(line, sizeof(line), "paratrooper landed: id=%u t=%.2f flight=%.2f "
                    "pos=(%.1f %.1f %.1f) target=(%.1f %.1f %.1f) bad=%d (007AC000, packet cc13-para2)",
                    r.id, static_cast<double>(now), static_cast<double>(now - r.dropped_at),
                    static_cast<double>(r.position.x), static_cast<double>(r.position.y),
                    static_cast<double>(r.position.z), static_cast<double>(r.s.target_47c.x),
                    static_cast<double>(r.s.target_47c.y), static_cast<double>(r.s.target_47c.z),
                    r.s.bad_landing_471 ? 1 : 0);
                svc.note(line);
            }
        }
    }
    // Job wave 3: tick element +8h, 007AB360 (unscaled; it scales its own +19Ch call).
    for (Round& r : rounds_) {
        if (!r.alive) continue;
        RoundAdapter host(*this, r, svc, now);
        seg46::update_chute_and_target_007ab360(r.s, r.cls, dt, host);
    }
    // +DCh, 007AB0B0 (LABELLED: once per fixed step, see the header).
    for (Round& r : rounds_) {
        if (!r.alive) continue;
        RoundAdapter host(*this, r, svc, now);
        seg46::anim_clock_007ab0b0(r.s, dt, host);
    }
    rounds_.erase(std::remove_if(rounds_.begin(), rounds_.end(),
                                 [](const Round& r) { return !r.alive; }),
                  rounds_.end());
}

void RoundWorld::capturers(std::uint32_t building, std::vector<Capturer>& out) const {
    out.clear();
    for (const auto& entry : capturer_list_) {
        if (entry.first != building) continue;
        for (const Round& r : rounds_) {
            if (r.id != entry.second) continue;
            out.push_back(Capturer{r.s.side_54, r.cls.capture_power_d8});
            break;
        }
    }
}

}  // namespace cc13_paratrooper
}  // namespace bsp
