#pragma once

// The producer of the vehicle class's buoyancy element list at class+52Ch.
//
// docs/SHIP_HYDRO_FORCES.md recorded that "the buoyancy element list's producer was not
// found, so its field roles are a hypothesis reconciled between its two readers". This
// header carries the producer.
//
// It is `0082D040`, body `0082D040-0082D6FD`,
// `void __thiscall(descriptor, const std::vector<Vec3>* bottom_points,
//                  const std::vector<Vec3>* deck_points, float water_line_ratio)`,
// `RET 0Ch` at `0082D6FB`. Its only caller is `0082FE30` (body `0082FE30-00831801`), the
// ship class descriptor's model-binding virtual, which occupies slot 8 (`vtable+20h`) of
// all eight ship-kind vtables and is reached once per class after the Lua load.
//
// The elements are generated, not authored per ship. `0082FE30` asks the class's loaded
// model (`descriptor+50h`) for two named point sets, `"deckline"` (the literal at
// `00D09A0C`) and `"bottomline"` (`00D09A00`), each a polyline of hull-local points.
// `0082D040` sorts both by z, walks `Hull.Segments` stations evenly spaced over `Length`,
// samples both polylines at each station's z, and emits TWO elements per station, one at
// `+Width/2` and one at `-Width/2`. So the list holds `2 * Hull.Segments` records and the
// hull's cross-section is never read: only the deck line, the keel line, the beam and the
// length shape it.
//
// The identity that makes the ship float: the coefficient at record `+00h` is
// `Mass * 10 * 0.5 / draught / Hull.Segments`, so `coefficient * draught` is
// `5 * Mass / Hull.Segments` for every record and the sum over all `2 * Hull.Segments`
// records is exactly `10 * Mass`. `00937C90`'s displacement sum multiplies that by
// `Gravitacio / 10` and subtracts `Gravitacio * Mass`, so the hull's reserve buoyancy at
// `controller+84h` is exactly zero whenever the material exponent leaves the shape factor
// at 1. Checked numerically against the installed DeRuyter data in
// docs/SHIP_BUOYANCY_ELEMENTS.md.
//
// Every descriptive name here is a hypothesis, not a recovered symbol, with two
// exceptions that are recovered from the image: the node names `deckline` and
// `bottomline` are string literals, and record `+14h` is the DRAUGHT because
// `008936A0`, the `luaMW_GetDraught` binding, returns `max(element[+14h])` over the list.
//
// Built on, never redefining: bsp/world_ocean.hpp (OceanVec3),
// bsp/ship_hydro_forces.hpp (ShipBuoyancyElement, the 24h-byte record as its readers see
// it). The corrected reading of that record's fields is in the accessors below and in the
// Corrections section of docs/SHIP_BUOYANCY_ELEMENTS.md; the shared struct keeps the
// reader-side member names so nothing downstream breaks.

#include <cstddef>
#include <vector>

#include "bsp/ship_hydro_forces.hpp"
#include "bsp/world_ocean.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Constants the producer reads
// ---------------------------------------------------------------------------

// The two model node names, ASCII literals at 00D09A0C (length 8, pushed at 0082FE66)
// and 00D09A00 (length 10, pushed at 0082FE82).
inline constexpr const char* kShipBuoyancyDeckLineNode = "deckline";
inline constexpr const char* kShipBuoyancyBottomLineNode = "bottomline";

// The third argument 00718000 takes at both call sites, EBX = 0 (0082FEAC, 0082FEC2). The
// callee compares it against record+24h, so it selects the record kind.
inline constexpr int kShipBuoyancyProfileNodeKind = 0;

// The double at 00CE3DC0, the same constant 00937F19 divides the displacement sum by.
inline constexpr float kShipBuoyancyMassToDisplacement = 10.0f;

// The double at 00D7A280. It halves the length into the station span (0082D1B2) and the
// width into the lateral offset (0082D50D, 0082D66C).
inline constexpr float kShipBuoyancyHalfExtent = 0.5f;

// 0082C9CA advances _Mylast by 24h; 00932C4B and five further sites divide by the same.
inline constexpr std::size_t kShipBuoyancyElementStride = 0x24;

// ---------------------------------------------------------------------------
// The record, read the producer's way
// ---------------------------------------------------------------------------
//
// bsp/ship_hydro_forces.hpp's ShipBuoyancyElement names the fields the way its two
// readers used them. The producer settles what they are; these accessors are the bridge,
// and they are the names to reason with.
//
//   +00h  coefficient   5 * Mass / (Hull.Segments * draught)        (0082D64B)
//   +04h  level_top     the WATERLINE, lerp(deck -> bottom, ratio)  (0082D61A..0082D622)
//   +08h  level_draft   the DECK LINE sample at this station        (0082D585)
//   +0Ch  level_base    the BOTTOM LINE (keel) sample               (0082D5D8)
//   +10h  unread_10     deck - bottom, the section height           (0082D604..0082D608)
//   +14h  unread_14     waterline - bottom, the DRAUGHT             (0082D629..0082D630)
//   +18h  position.x    +/- Width/2                                 (0082D507, 0082D65D)
//   +1Ch  position.y    the same deck line sample as +08h           (0082D58A)
//   +20h  position.z    the station along the hull                  (0082D51C..0082D539)

// +04h. The level the element treats as the still waterline.
float ship_buoyancy_element_water_line(const ShipBuoyancyElement& element) noexcept;

// +08h. The deck line height at this station; also the element point's y.
float ship_buoyancy_element_deck_level(const ShipBuoyancyElement& element) noexcept;

// +0Ch. The bottom line (keel) height at this station.
float ship_buoyancy_element_bottom_level(const ShipBuoyancyElement& element) noexcept;

// +10h. deck - bottom. Cached by the producer; both readers recompute it instead.
float ship_buoyancy_element_section_height(const ShipBuoyancyElement& element) noexcept;

// +14h. waterline - bottom. Cached by the producer and read as the draught by
// 008936A0 `luaMW_GetDraught`, which returns the maximum over the list (008936A0's loop
// at the `+14h + i*24h` load).
float ship_buoyancy_element_draught(const ShipBuoyancyElement& element) noexcept;

// ---------------------------------------------------------------------------
// The class fields the producer reads
// ---------------------------------------------------------------------------

// Five numbers, every one already declared elsewhere; this struct only gathers them so a
// caller need not carry two large descriptor structs. `length`, `width` and `mass` are
// VehicleClassBaseFields::length/width/mass (descriptor +A0h, +A4h, +B0h, written by
// 00960230); `hull_water_line_ratio` and `hull_segments` are ShipClassFields'
// members of the same names (descriptor +71Ch and +720h, the Lua keys `Hull.WaterLineRatio`
// and `Hull.Segments`, written by 00832D9A and 00832DE8).
struct ShipBuoyancyHullInputs {
    float length{0.0f};                // descriptor+A0h, read at 0082D193
    float width{0.0f};                 // descriptor+A4h, read at 0082D507 and 0082D65D
    float mass{1.0f};                  // descriptor+B0h, read at 0082D637
    float hull_water_line_ratio{0.0f}; // descriptor+71Ch, pushed at 0082FECF
    int hull_segments{0};              // descriptor+720h, read at 0082D4CE and 0082D681
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 0082A920, body 0082A920-0082AA7C, `float __stdcall(const vector* owner, const Vec3*
// first, const vector* owner, const Vec3* last, float z)`, RET 14h, result on the x87
// stack. Two of the five arguments are the `_SECURE_SCL` iterator's owning container and
// carry no value; the contract is the three that do.
//
// A clamped piecewise-linear sample of y over z along a polyline already sorted by
// ascending z:
//   z <  first->z   -> first->y                        (the tail at 0082AA6x)
//   z >= last[-1].z -> last[-1].y                      (the `param_4 - 8` return)
//   otherwise       -> lerp(p.y, q.y, (z - p.z) / (q.z - p.z)) for the span p..q holding z
// An empty or one-point range is a `_invalid_parameter_noinfo` in the original; here it
// returns 0.0f for empty and the single y for one point.
float ship_buoyancy_sample_profile_0082a920(const OceanVec3* first, const OceanVec3* last,
                                            float z) noexcept;

// 0082D21F..0082D365 and the identical 0082D380..0082D4C5. A selection sort: repeatedly
// take the first point with the strictly smallest z, append it to the output and erase it
// from the input by shifting the tail down (0082D341..0082D35E). Equivalent to a stable
// ascending sort by z, which is what the sampler above requires and what the shipped
// model data is NOT: the DeRuyter's bottom line is stored in descending z.
std::vector<OceanVec3> ship_buoyancy_sort_profile_by_z_0082d21f(std::vector<OceanVec3> points);

// 0082D500..0082D54E. The station's z for element index `i`:
//   segments < 2 -> 0.0f                                      (0082D542)
//   otherwise    -> i * length / (segments - 1) - length / 2   (0082D51C..0082D539)
// So the stations span the whole length symmetrically about the model origin.
float ship_buoyancy_station_z_0082d500(int index, int segments, float length) noexcept;

// 0082D5E4..0082D651 plus the two pushes at 0082D658 and 0082D67C. One station's pair of
// records, given the two profile samples at that station. `lateral` is `+Width/2` for the
// first push and `-Width/2` for the second; everything else is identical between the two,
// because the second push reuses the same stack record with only `+18h` rewritten
// (0082D65D..0082D675).
ShipBuoyancyElement ship_buoyancy_make_element_0082d5e4(float deck_level, float bottom_level,
                                                        float water_line_ratio, float lateral,
                                                        float station_z, float mass,
                                                        int segments) noexcept;

// 0082D4CA..0082D690, the whole emit loop over `Hull.Segments` stations. The two point
// lists arrive in the order the caller resolved them and are sorted here. Returns the
// `2 * Hull.Segments` records the class stores at `class+52Ch`, in push order: station 0
// starboard, station 0 port, station 1 starboard, ... A `Hull.Segments` of zero or less
// yields an empty list (the `JLE` at 0082D4DC), which is what leaves the vector empty and
// makes 00937C90's `elements[0]` bounds check at 00937DE9 fire.
std::vector<ShipBuoyancyElement> ship_buoyancy_build_list_0082d4ca(
    const ShipBuoyancyHullInputs& hull, const std::vector<OceanVec3>& deck_points,
    const std::vector<OceanVec3>& bottom_points);

// ---------------------------------------------------------------------------
// The host, one method per native call site the sequence below cannot own
// ---------------------------------------------------------------------------

struct ShipBuoyancyElementHost {
    virtual ~ShipBuoyancyElementHost() = default;

    // 00718000, body 00718000-007180D5, `void* __thiscall(model, const std::string* name,
    // int kind)`. Its body walks the model's record vector at `model+68h..+6Ch`, compares
    // each record's name (a `std::string` at `record+0Ch` with the length at `+1Ch` and
    // the short/long capacity switch at `+20h`) with 004B3FC0, requires an exact length
    // match and `record+24h == kind`, and returns the LAST match or null. The points the
    // producer wants are the `std::vector<Vec3>` at `record+44h`, which 0071F9F0 (a
    // vector assign) copies out at 0082D1FE and 0082D376.
    //
    // Call sites: 0082FEBA ("deckline") and 0082FECA ("bottomline"), both inside 0082FE30,
    // both with the model taken from `descriptor+50h` (0082FEA9, 0082FEBF) and kind 0.
    //
    // Return false when the model has no such record; the original would then dereference
    // null, so a false here is a refusal to reproduce that.
    virtual bool find_model_profile_points(const char* node_name, int kind,
                                           std::vector<OceanVec3>& out) = 0;
};

// ---------------------------------------------------------------------------
// The sequence, 0082FEA9..0082FEE8 inside 0082FE30
// ---------------------------------------------------------------------------

// Resolve the two named point sets and build the list. In the original the two lookups are
// 0082FEBA and 0082FECA and the build is the call at 0082FEE3; the string construction
// either side (00408720 at 0082FE7B and 0082FEA4, `_free` at 0082FEFB and 0082FF2C) is
// `std::string` lifetime and is not modelled.
//
// The argument order at the call site is (bottom_points, deck_points, ratio): 0082FED9
// pushes the DECK record's points as the third parameter and 0082FEDD the BOTTOM record's
// as the second. The parameters are un-swapped here so the names read straight.
bool ship_buoyancy_build_from_model_0082fea9(ShipBuoyancyElementHost& host,
                                             const ShipBuoyancyHullInputs& hull,
                                             std::vector<ShipBuoyancyElement>& out);

}  // namespace bsp
