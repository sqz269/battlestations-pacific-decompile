// 006FEC00 (the DummyKamikazePlane tick) and the round registry the hosts share.
// Packet cc9_lua52_kamikaze_wiring; evidence in include/bsp/kamikaze_task_host.hpp
// and docs/SQUADRON_LAND_TASK.md 5gb. Names are hypotheses.

#include "bsp/kamikaze_task_host.hpp"

#include <cmath>
#include <utility>

namespace bsp {

DummyKamikazeTickResult dummy_kamikaze_tick_006fec00(const DummyKamikazeTickInputs& in) noexcept {
    DummyKamikazeTickResult r;
    r.launch_time_9c = in.launch_time_9c;
    // 006FEC34-006FEC48: COMISS 0.0, [this+9Ch]; JBE skips, so only a negative
    // value is replaced.
    if (!in.attached && 0.0f > in.launch_time_9c) {
        r.stamps_launch = true;
        r.launch_time_9c = in.now;
    }
    // 006FECBE-006FED12. FCOMI with JBE: the spawn needs now - launch strictly
    // above OpenAfterTime.
    if (in.unit_made_b8 || in.network_client || in.attached) return r;
    const float since = in.now - r.launch_time_9c;           // 006FECFA, a float store
    r.spawns = since > in.open_after_time_d8;
    return r;
}

std::array<float, 16> dummy_kamikaze_frame_from_velocity(const std::array<float, 3>& position,
                                                         const std::array<float, 3>& velocity) {
    std::array<float, 16> m{};
    double f[3] = {velocity[0], velocity[1], velocity[2]};
    double n = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    if (!(n > 1e-6)) { f[0] = 0.0; f[1] = 0.0; f[2] = 1.0; n = 1.0; }
    for (double& c : f) c /= n;
    // right = (0,1,0) x f = (fz, 0, -fx); a vertical forward keeps world x.
    double r[3] = {f[2], 0.0, -f[0]};
    double rn = std::sqrt(r[0] * r[0] + r[2] * r[2]);
    if (!(rn > 1e-6)) { r[0] = 1.0; r[2] = 0.0; rn = 1.0; }
    r[0] /= rn; r[2] /= rn;
    // up = f x r.
    const double u[3] = {f[1] * r[2] - f[2] * r[1], f[2] * r[0] - f[0] * r[2],
                         f[0] * r[1] - f[1] * r[0]};
    for (int i = 0; i < 3; ++i) {
        m[i] = static_cast<float>(r[i]);
        m[4 + i] = static_cast<float>(u[i]);
        m[8 + i] = static_cast<float>(f[i]);
        m[12 + i] = position[static_cast<std::size_t>(i)];
    }
    m[15] = 1.0f;
    return m;
}

std::uint32_t DummyKamikazeRounds::push(DummyKamikazeRound round) {
    round.dummy_id = next_id_++;
    round.launch_time = -1.0f;
    round.unit_made = false;
    round.dead = false;
    rounds_.push_back(std::move(round));
    ++pushed_;
    return rounds_.back().dummy_id;
}

std::size_t DummyKamikazeRounds::live() const {
    std::size_t n = 0;
    for (const DummyKamikazeRound& r : rounds_) {
        if (!r.dead) ++n;
    }
    return n;
}

std::vector<DummyKamikazeSpawn> DummyKamikazeRounds::step(float step) {
    std::vector<DummyKamikazeSpawn> out;
    if (!(step > 0.0f)) return out;
    clock_ += step;
    for (DummyKamikazeRound& r : rounds_) {
        if (r.dead) continue;
        // The decision first, with the time stamp the previous step left: the
        // round leaves the rack on the release step and its first own tick is
        // the next one (006FEC34 stamps it there, attached = false from then on).
        DummyKamikazeTickInputs in;
        in.attached = false;
        in.unit_made_b8 = r.unit_made;
        in.now = clock_;
        in.launch_time_9c = r.launch_time;
        in.open_after_time_d8 = r.open_after_time;
        const DummyKamikazeTickResult d = dummy_kamikaze_tick_006fec00(in);
        if (d.stamps_launch) {
            r.launch_time = d.launch_time_9c;
            // 006FEC86: the frame turns to the velocity on the stamp.
            r.frame = dummy_kamikaze_frame_from_velocity(r.position, r.velocity);
        }
        // 006FECB9: 006E1300, here a gravity-only ballistic step (LABELLED).
        r.velocity[1] -= 9.81f * step;
        for (int i = 0; i < 3; ++i) r.position[static_cast<std::size_t>(i)] += r.velocity[static_cast<std::size_t>(i)] * step;
        if (d.spawns) {
            r.unit_made = true;                              // 006FEED3, this+B8h
            DummyKamikazeSpawn s;
            s.carrier = r.carrier;
            s.plane_class = r.plane_class;
            // The matrix the creator receives is this-370h: the stamped frame's
            // axes, at the position the bomb tick has just moved it to.
            s.world = r.frame;
            for (int i = 0; i < 3; ++i) s.world[12 + i] = r.position[static_cast<std::size_t>(i)];
            s.velocity = r.velocity;
            s.speed = static_cast<float>(std::sqrt(
                static_cast<double>(r.velocity[0]) * r.velocity[0] +
                static_cast<double>(r.velocity[1]) * r.velocity[1] +
                static_cast<double>(r.velocity[2]) * r.velocity[2]));
            s.target_plus_one = r.target_plus_one;
            s.party = r.party;
            s.race = r.race;
            s.skill = r.skill;
            s.owner_player = r.owner_player;
            s.dummy_id = r.dummy_id;
            out.push_back(s);
            ++spawned_;
        }
        if (r.position[1] <= 0.0f) {
            r.dead = true;
            ++drowned_;
        }
    }
    return out;
}

DummyKamikazeRounds& dummy_kamikaze_rounds() {
    static DummyKamikazeRounds rounds;
    return rounds;
}

}  // namespace bsp
