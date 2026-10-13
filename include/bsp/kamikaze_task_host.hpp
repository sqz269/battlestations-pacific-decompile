#pragma once

// Host wiring of cc13's kamikaze tasks (include/bsp/kamikaze_task_cc13.hpp,
// docs/CC13_KAMIKAZE_TASK.md) and the DummyKamikazePlane tick that turns a
// dropped Ohka/Funryu round into a flying kamikaze unit. Packet
// cc9_lua52_kamikaze_wiring; the run record is docs/SQUADRON_LAND_TASK.md 5gb.
// Names are hypotheses, not recovered symbols.

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {

// True: a plane whose order is class 00E08F50 (kamikaze, kind 0Ch) runs the
// kamikaze task's arm 009AF510 each think, next to the strafe arm. False: the
// order installs nothing and the plane flies the reseeded plan, as before.
inline constexpr bool kKamikazeTaskBound = true;  // ON: USN19 pairs, mechanism matches (docs/SQUADRON_LAND_TASK.md 5gc)
// True: a plane ordered with class 00E08F30 (dropkamikaze, kind 5) runs the
// drop task's arm 009AE8A0, and each release (007BBBA0) launches a
// DummyKamikazePlane round that 006FEC00 turns into the class-156/370 unit.
// False: no arm, no round, as before.
inline constexpr bool kDropKamikazeTaskBound = true;  // ON: USNOS K4, 22 Ohka releases (docs/SQUADRON_LAND_TASK.md 5gc)

// ---------------------------------------------------------------------------
// 006FEC00 BSP_DummyKamikazePlane_TickSpawnKamikaze (body 006FEC00-006FEF07,
// RET 4, __thiscall with ECX = round+3E4h, the tick element; its only reference
// is the .rdata slot 00CFC618 of tick vtable 00CFC610). Offsets below are the
// tick element's (this+N); the round itself is this-3E4h.
//
//   006FEC21-006FEC32  attached = 00923810(round, 1): the parent at round+3Ch
//   006FEC34-006FEC4E  !attached and this+9Ch < 0: this+9Ch = now ([00F876A4])
//   006FEC50-006FECA7    |v|^2 (this-CCh..-C4h) > [00D7A238]: the matrix row
//                        round+(-370h+20h) = v, then 0085DC80 (orthonormalise)
//   006FECAC-006FECB9  006E1300(dt), the bomb tick (the round flies as a bomb)
//   006FECBE-006FED12  spawn when this+B8h == 0 (no unit made yet),
//                      [[00E188A8]+1FE4h] != 2 (not a network client),
//                      !attached and now - this+9Ch > desc+D8h (OpenAfterTime)
//   006FED18-006FEE74  the bag: Type = [desc+DCh]+70h (KamikazePlaneClass's
//                      class id), WingCount 1, Skill this+BCh, Race this-38Ch,
//                      Party this-390h, OwnerPlayer this-264h, State 7,
//                      VelocitySI |v| (0042B2F0), Vel3D v, Invincible 0.0,
//                      FireStance 2, AutoAttackTarget [this+98h]+174h when a
//                      target is set, DummyObjectID this+A0h
//   006FEE79-006FEE8D  004F0AD0(class, name [desc+DCh]+13Ch, 0, matrix
//                      this-370h, bag, 0): the PlaneSquadronGen creator
//   006FEEBE-006FEED3  entity+C0h = the bag ref; 006FE910 on this+A4h stores
//                      the entity at this+A4h+14h = this+B8h, which is the
//                      once-only latch the spawn test reads
//
// The property names are the pushed literals (00CE4780 Type, 00CF8840
// WingCount, 00CF8838 Skill, 00CE8EE0 Race, 00CE5804 Party, 00CF882C
// OwnerPlayer, 00CF8818 State, 00CF880C VelocitySI, 00CFC85C Vel3D, 00CF8800
// Invincible, 00CFC850 FireStance, 00CF87EC AutoAttackTarget, 00CFC840
// DummyObjectID).
// ---------------------------------------------------------------------------

struct DummyKamikazeTickInputs {
    bool attached = false;           // 00923810(round, 1) != 0
    bool network_client = false;     // [[00E188A8]+1FE4h] == 2
    bool unit_made_b8 = false;       // this+B8h != 0
    float now = 0.0f;                // [00F876A4]
    float launch_time_9c = -1.0f;    // this+9Ch, negative until stamped
    float open_after_time_d8 = 0.0f; // desc+D8h OpenAfterTime
};
struct DummyKamikazeTickResult {
    bool stamps_launch = false;      // 006FEC48: this+9Ch = now
    float launch_time_9c = -1.0f;    // after the stamp
    bool spawns = false;             // 006FED18 onwards
};
// The decision half of 006FEC00, without the bomb tick and the creator.
DummyKamikazeTickResult dummy_kamikaze_tick_006fec00(const DummyKamikazeTickInputs& in) noexcept;

// 006FEC86-006FECA7 then 0085DC80: forward row = v, the frame re-orthonormalised.
// SUBSTITUTION, labelled: 0085DC80 is not reconstructed; the frame is built
// from the forward row and world up (right = up x forward, up = forward x right),
// which is what an orthonormalisation of a level frame with that forward gives.
// Rows 0..2 right/up/forward, row 3 the position (the units host's layout).
std::array<float, 16> dummy_kamikaze_frame_from_velocity(const std::array<float, 3>& position,
                                                         const std::array<float, 3>& velocity);

// One released round. The host owns the flight (a ballistic stand-in for
// 006E1300) and asks the decision above each step.
struct DummyKamikazeRound {
    std::size_t carrier = 0;         // the releasing unit (units host index)
    int bullet_class = -1;           // the rack's DummyKamikazePlane bullet
    int plane_class = -1;            // desc+DCh KamikazePlaneClass
    float open_after_time = 0.0f;    // desc+D8h OpenAfterTime
    float launch_time = -1.0f;       // this+9Ch
    std::array<float, 3> position{};
    std::array<float, 3> velocity{};
    std::array<float, 16> frame{};   // this-370h, set at the launch stamp
    std::size_t target_plus_one = 0; // this+98h, the carrier's target
    int party = -1;                  // this-390h
    int race = -1;                   // this-38Ch
    int skill = 1;                   // this+BCh
    int owner_player = 9;            // this-264h
    std::uint32_t dummy_id = 0;      // this+A0h
    bool unit_made = false;          // this+B8h
    bool dead = false;               // the bomb tick's water/expiry kill
};

struct DummyKamikazeSpawn {
    std::size_t carrier = 0;
    int plane_class = -1;
    std::array<float, 16> world{};
    std::array<float, 3> velocity{};
    float speed = 0.0f;              // VelocitySI
    std::size_t target_plus_one = 0;
    int party = -1;
    int race = -1;
    int skill = 1;
    int owner_player = 9;
    std::uint32_t dummy_id = 0;
};

// Process-wide, like the squadron registry: the units host pushes a round at
// the release, the mission Lua host steps the rounds once per mission frame and
// makes the units (it owns the creator and the SEntity init route).
class DummyKamikazeRounds {
public:
    void clear() { rounds_.clear(); clock_ = 0.0f; next_id_ = 1; }
    std::uint32_t push(DummyKamikazeRound round);
    // Advances the clock and every live round by `step` (gravity only: the
    // bomb tick's drag and water arms are not modelled, LABELLED), then returns
    // the rounds whose 006FEC00 decision spawns this step. A round falls dead
    // at y <= 0 (the bomb tick's water kill, LABELLED stand-in).
    std::vector<DummyKamikazeSpawn> step(float step);
    float clock() const { return clock_; }
    std::size_t live() const;
    std::size_t pushed() const { return pushed_; }
    std::size_t spawned() const { return spawned_; }
    std::size_t drowned() const { return drowned_; }
private:
    std::vector<DummyKamikazeRound> rounds_;
    float clock_ = 0.0f;
    std::uint32_t next_id_ = 1;
    std::size_t pushed_ = 0, spawned_ = 0, drowned_ = 0;
};
DummyKamikazeRounds& dummy_kamikaze_rounds();

}  // namespace bsp
