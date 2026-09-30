// The CommandBuilding landing-pad model: the pad vector at building+794h and the
// routines that fill, search and assign it.
//
// Packet cc9_building_pad_model, worker agent/cc9-ships20. Project
// C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra was
// read-only for this packet. Every descriptive name is a hypothesis, not a
// recovered symbol. docs/SHIP_AI_OPEN_ITEMS.md sections 74.5 and 75 carry the
// evidence.
//
// A pad is a scene `LandingPoint` (class 1Dh, factory 004E9D40, 0x224 bytes,
// construct 004E9520). The building keeps pointers to its pads in a vector
// `{begin +794h, count +798h, capacity +79Ch}` guarded by the critical section
// at +764h; the pad keeps its occupant unit behind an observer handle at +1E4h
// (target +1F8h) and its building at +220h. Here a pad is a host index into
// BuildingPadModel::pads and a unit or building is a units-host index; the
// critical section has no counterpart because this process runs the callers
// on one thread.
#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

#include "bsp/ship_ai_follow_land.hpp"

namespace bsp {

// 006F2780: `LandingPointRange` (key 00CFAE0C, 006F2895) stored at +7CCh
// (006F28B3), 1F4h when unauthored (006F28A8).
inline constexpr std::int32_t kCommandBuildingLandingPointRangeDefault = 500;

// 006F5CC0's third pass, 006F6603-006F6670: the squared world distance from the
// building (+0FCh..+104h) to the pad, summed (dy^2 + dx^2) + dz^2 and stored as
// float32 (006F665A), against FILD of the int32 product LandingPointRange *
// LandingPointRange (006F6609-006F6617 IMUL, wrapping); JA skips, so a pad at
// exactly the range is kept. x87 intermediates are extended precision; here
// they are double, rounded to float at the same store. Coverage: this compare.
bool building_pad_in_range_006f5cc0(const float building[3], const float pad[3],
                                    std::int32_t landing_point_range);

class BuildingPadModel {
public:
    struct Pad {
        int marker_id{0};       // the scene marker id the host gave the LandingPoint
        float position[3]{};    // pad+0FCh..+104h, the authored world translation
        int occupant{-1};       // pad+1F8h, a units-host index, -1 for null
        int owner{-1};          // pad+220h, the building's units-host index
        // 006AC260: the pad's forward row (+0ECh / +0F4h) normalised in xz
        // (length of (x, 0.0 [00D7A258], z); 0 when not positive).
        float facing_x{0.0f};
        float facing_z{0.0f};
        // pad+208h..+21Ch, 006AC5D0's approach cache. The construct 004E9520
        // writes +208h = -99 (0FFFFFF9Dh) and +218h = 1000.0f [00CE3804].
        ShipAiLandPadLine line{-99, 0.0f, 0.0f, 0.0f, 1000.0f, 800.0f};
    };

    // The landing ship's side of the link, 0074A990 (BSP_LandingShip_BeginLandingAtPad).
    struct Lander {
        int pad_1200{-1};           // ship+1200h
        int building_1204{-1};      // ship+1204h
        float landing_time_1210{0.0f};  // ship+1210h, read by 009E18D0 via slot 248h
        // The ramp latch of 0074AF50 (MLandingShip vtable 00CFFA30 slot 0DCh), seeded
        // by the construct at 0074C0CD..0074C108. Lua property names (0074C455..,
        // `{2,&field}`/`{0,name}` pairs): +11A8h "lastTalaj" (the last ground
        // contact time), +11ACh "nyitzarTimer" (the open/close timer).
        float last_ground_11a8{-10000000000.0f};  // 00CE4ADC
        float ramp_timer_11ac{0.0f};
        float ramp_delay_11b0{2.0f};              // 00CE3958, no other writer
        bool ramp_down_1188{false};               // set by 0074A420 (and the 0A6h handler)
        // The ramp animation and the unload of 0074AF50 (docs/SHIP_AI_OPEN_ITEMS.md
        // 103, 106), keys from the save routine 0074A1F0: +11A4h "rampaElfordulas",
        // +118Ah "partraszallas", +118Bh "gyorsPartraszallas", +118Ch
        // "partraszalltunk". InitAll 0074BEC0 zeroes the bytes and stores 2.0
        // (00CE3958) into +1190h; the construct seeds +11A4h = 0.
        float ramp_rotation_11a4{0.0f};
        float ramp_rotation_time_1190{2.0f};
        bool landing_118a{false};        // no .text writer but InitAll's zero (and Lua/save)
        bool fast_landing_118b{false};   // 0074B18F, the unload's one-shot latch
        bool landed_118c{false};         // 0074AD90
    };

    Lander* mutable_lander(int ship);

    // 006F5CC0, CommandBuilding vtable 00CFB028 slot 0A4h (InitAll pass C, via
    // 006F6740), third pass only (006F6436-006F673E): every LandingPoint of the
    // world registry list 1Dh ([[00E188A8]+19CCh]+178h) in list order, kept by
    // building_pad_in_range_006f5cc0, is appended to the building's vector and
    // gets owner = building (006F6711). A pad adopted by two buildings keeps the
    // later owner, as the image's plain store does. Coverage: pass three of three;
    // passes one (garrison, list 5) and two (list 4Dh into +788h) are not modelled.
    void adopt_landing_pads_006f5cc0(int building, const float building_position[3],
                                     std::int32_t landing_point_range,
                                     const std::vector<Pad>& landing_points_in_list_order);

    // 006F2E60, __thiscall(building)(unit, char ignore_held), body 006F2E60-006F2FA2,
    // complete. unit == -1 answers -1 (006F2E6A). Walks the vector in order: a free
    // pad (occupant null) is a candidate by squared 3-D distance pad-to-unit with
    // the first strict minimum winning (seed FLT_MAX 00D7A248); a pad held by the
    // unit returns at once unless ignore_held. Otherwise the nearest free pad or -1.
    int pick_006f2e60(int building, int unit, const float unit_position[3],
                      bool ignore_held) const;

    // 006F2DE0 BSP_CommandBuilding_ReleaseUnitPads, __thiscall(building)(unit),
    // RET 4, body 006F2DE0-006F2E54, complete: every pad of this building held by
    // the unit gets 006AC490(0).
    void release_unit_pads_006f2de0(int building, int unit);

    // 006F2FB0, __thiscall(building)(unit, pad), body 006F2FB0-006F3009, complete:
    // a non-null pad already held by the unit returns at once; otherwise release
    // the unit's pads on this building, then 006AC490(unit) on the pad if non-null.
    void assign_006f2fb0(int building, int unit, int pad);

    // 006AC490, __thiscall(pad)(unit), RET 4, body 006AC490-006AC4C3, complete:
    // no-op when unchanged; else unregister the old occupant's observer pair
    // (006952A0), store, register the new one (00694A60).
    void set_occupant_006ac490(int pad, int unit);

    // The observer pair's effect when the occupant goes away: the handle at
    // pad+1E4h drops its target. SUBSTITUTION, labelled: the handle vtable
    // 00CE75CC's notification slot was not read; clearing the occupant is the
    // assumed behaviour of a weak reference.
    void forget_unit(int unit);

    // 006F3AF0 BSP_CommandBuilding_NearestPadStandoffPoint, first half
    // (006F3AF0-006F3C5A): the xz of the pad nearest `from` by squared 3-D
    // distance pad-minus-from over the whole vector, occupied or not (seed FLT_MAX,
    // strict <), or the building's own xz with no pad. The second half hands that
    // point to 00417E60 (avoid-zone push-out, unread) and is not covered.
    void nearest_pad_xz_006f3af0(int building, const float building_position[3],
                                 const float from[3], float out_xz[2]) const;

    // 0074A990 steps 1-3 (docs/SHIP_AI_OPEN_ITEMS.md 76.2): the link, the
    // occupant and the landing time (draw + SpawnPhase) * 1.5 [00CE3D78], where
    // `draw` is 00BD2F10's uniform(0, 0.75f [00CEE07C]) the caller made.
    void begin_landing_0074a990(int ship, int pad, int building, float draw,
                                std::int32_t spawn_phase_1208);
    // ship+1200h, -1 for null.
    int lander_pad_1200(int ship) const;
    const Lander* lander(int ship) const;
    Pad* mutable_pad(int index);

    const std::vector<int>& pads_of(int building) const;
    const Pad* pad(int index) const;
    std::size_t pad_count() const { return pads_.size(); }
    std::size_t building_count() const { return vectors_.size(); }
    void clear();

    // The unloads landing_ship_ramp_unload_0074b109 fired, in order, for the
    // script host to publish `LandingStarted` / `LandingFinished` (the image
    // writes both inside the ship's own update). take_ empties the queue.
    void post_unload(int ship) { unloads_.push_back(ship); }
    std::vector<int> take_unloads() {
        std::vector<int> out;
        out.swap(unloads_);
        return out;
    }

private:
    std::vector<int> unloads_;
    int intern_pad(const Pad& pad);
    std::vector<Pad> pads_;
    std::map<int, std::vector<int>> vectors_;  // building -> +794h vector
    std::map<int, Lander> landers_;
};

// The zone queries 006AC5D0's cache refresh makes, in the image's argument
// order (toward, start/from, out).
struct PadLineZoneQueries {
    virtual ~PadLineZoneQueries() = default;
    // 004218E0 then 004120D0(layer): the layer's group, 0 for none.
    virtual std::uint32_t group_for_layer_004120d0(int layer) = 0;
    // 004178F0(group)(point): the first zone containing the point, 0 for none.
    virtual std::uint32_t zone_containing_004178f0(std::uint32_t group,
                                                   float x, float z) = 0;
    // 0041B4E0(group)(toward, from, out) -> AL.
    virtual bool group_segment_hit_0041b4e0(std::uint32_t group, const float toward[2],
                                            const float from[2], float out[2]) = 0;
    // 00416DD0(zone)(toward, start, out, &edge) -> AL.
    virtual bool zone_segment_hit_00416dd0(std::uint32_t zone, const float toward[2],
                                           const float start[2], float out[2]) = 0;
};

// 006AC5D0's first arm, 006AC5F2..006AC927: on a new layer key the pad's
// approach line is recast. Coverage: this arm; the per-call arm is
// ship_ai_land_pad_approach_point_006ac5d0. Returns true when it recast.
bool refresh_pad_line_006ac5d0(BuildingPadModel::Pad& pad, int layer,
                               PadLineZoneQueries& zones);

// The process's one pad model (the image's live building and pad entities). The
// script-orders host fills it after the scene load; the ship AI reads it.
BuildingPadModel& building_pad_model();

// 0074AF50's ramp latch, 0074AFCC..0074B0AC (packet cc9_landing_ramp_capture,
// docs/SHIP_AI_OPEN_ITEMS.md section 86); the caller has already checked the
// gates at 0074AF7B..0074AFC6 (+5Ch set, +5Dh/+60h/+5Eh clear, game mode not 2,
// a pad at +1200h). `ground_1011` is the one-frame contact latch (+1011h, a
// kind-8 physics contact rotated by 008255B0); `clock` is [00F876A4]; `dt` the
// update's argument. x87: held_before = +11A8h > (clock - 1) - dt (0074AFE9
// FCOMIP, JBE); a set latch stores clock into +11A8h (0074B006); held_now =
// +11A8h > clock - 1 (0074B01C). A change resets +11ACh to +11B0h (0074B02E);
// otherwise a positive +11ACh counts down by dt (0074B053..0074B05F) and, once it
// is <= 0 (0074B065 FLDZ/FCOMIP, JB skips), a raised ramp (+1188h clear) with
// held_now lowers: true means 0074A420 (+1188h = +1189h = 1) and the 0A6h route
// (00749AA0, 0077C2A0 class 4) run. Coverage: 0074AFCC-0074B0AC; 0074B0B0.. (the
// pad re-request, the ramp animation +11A4h and the unload) is not covered.
bool landing_ship_ramp_latch_0074afcc(BuildingPadModel::Lander& lander, bool ground_1011,
                                      float clock, float dt);

// 0074AF50's ramp animation and one-shot unload, 0074B109..0074B329 (section 106),
// run every frame after the latch (every latch path reaches 0074B0B0 and falls
// through). r = +11A4h at entry. Ramp down: r >= 1.0 [00D7A24C] (0074B133 COMISS,
// JBE) with +118Ah and +118Bh clear sets +118Bh (0074B18F) and, through 0074AD90,
// +118Ch, and returns true; r < 1.0 stores min(float(dt / +1190h + r), 1)
// (006F22B0 with ECX = 1 at 0074B146). Ramp up and r > 0.0 [00D7A218]: max(float(r -
// dt / +1190h), 0) (006F22F0, ECX = 0 at 0074B309). x87 intermediates are extended
// precision; here double, rounded once to float as the FSTP does. With dt 0.05 the
// ramp needs 41 steps, so the unload comes 2.05 s after the latch frame.
// Coverage: the flags and the rotation. Not modelled: the bow point handed to
// 006AC370 (the pad's troop paths), the `soldiers` note node of a small ship, the
// ramp-bone poses (presentation), and the `shipLanded` event 00986820 (a record in
// the caller). The Lua writes (`LandingStarted`, then 0074AD90's `LandingFinished`)
// are the caller's.
bool landing_ship_ramp_unload_0074b109(BuildingPadModel::Lander& lander, float dt);

}  // namespace bsp
