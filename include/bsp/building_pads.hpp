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
    };

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

    const std::vector<int>& pads_of(int building) const;
    const Pad* pad(int index) const;
    std::size_t pad_count() const { return pads_.size(); }
    std::size_t building_count() const { return vectors_.size(); }
    void clear();

private:
    int intern_pad(const Pad& pad);
    std::vector<Pad> pads_;
    std::map<int, std::vector<int>> vectors_;  // building -> +794h vector
};

}  // namespace bsp
