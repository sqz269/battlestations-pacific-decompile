#pragma once

#include "bsp/unit_rudder.hpp"

// Who fills the rudder curve settings, and what the shipped data puts there.
//
// docs/UNIT_RUDDER_CURVE.md carries the addresses, the original ABI and the uncertainty.
// `UnitRudderCurveSettings` (the six fields 0082E890 reads) is declared in
// bsp/unit_rudder.hpp and is NOT redeclared here; this header only adds the producer and
// the gameplay-modifier product that shares the curve's host.
//
// The block lives on the gameplay settings singleton 00424C40 returns, at +438h..+44Ch.
// It is not a unit field and not a vehicle-class field. 00424A10, the singleton's
// constructor, does not touch it; the only writer in the image is the Lua-driven settings
// loader 0083B5E0, whose six stores are at 0083CEDE, 0083CF4D, 0083CFBC, 0083D02B,
// 0083D09A and 0083D109. Names are hypotheses, not recovered symbols.

namespace bsp {

// One authored control point: the throttle position and the turning-circle multiplier the
// vehicle class's radius is multiplied by there. The Hungarian comment on all three rows
// of the shipped table says exactly that ("Elso ertek a gazkar allasa, masodik a
// fordulokor szorzoja" - first value the throttle setting, second the turn-circle factor).
struct UnitTurnMultiplierPoint {
    float throttle{0.0f};
    float turn_circle_multiplier{0.0f};
};

// ShipGlobals["Navigator"]["TurnMultipliers"], the sub-table 0083CE6F selects.
struct UnitTurnMultiplierTable {
    UnitTurnMultiplierPoint min_speed{};  // key "TurnMultiplierMinSpeed"
    UnitTurnMultiplierPoint med_speed{};  // key "TurnMultiplierMedSpeed"
    UnitTurnMultiplierPoint max_speed{};  // key "TurnMultiplierMaxSpeed"
};

// The values the installed scripts/datatables/shipglobals.lua carries. Data, not code:
// a mission or a mod that replaces that file changes the curve.
inline constexpr UnitTurnMultiplierTable kShippedTurnMultipliers{
    UnitTurnMultiplierPoint{0.0f, 0.4f},
    UnitTurnMultiplierPoint{0.5f, 1.5f},
    UnitTurnMultiplierPoint{1.0f, 2.0f},
};

// The five Lua keys the loader fragment pushes, in the order it pushes them.
inline constexpr const char* kTurnMultipliersTableKey = "TurnMultipliers";        // 00D0B1A4
inline constexpr const char* kTurnMultiplierMinSpeedKey = "TurnMultiplierMinSpeed"; // 00D0B18C
inline constexpr const char* kTurnMultiplierMedSpeedKey = "TurnMultiplierMedSpeed"; // 00D0B174
inline constexpr const char* kTurnMultiplierMaxSpeedKey = "TurnMultiplierMaxSpeed"; // 00D0B15C

// The native reader calls, one pure virtual per call site in 0083CE56..0083D10D. Handles
// stand in for the 14h-byte LuaObject wrappers the fragment builds on its own stack; the
// same four helpers are modelled in full in bsp/gui_lua_reader.hpp, which this interface
// deliberately does not depend on.
struct UnitRudderCurveLoaderHost {
    virtual ~UnitRudderCurveLoaderHost() = default;
    // 00B67700 BSP_LuaObject_Destruct, __thiscall(LuaObject), RET. The fragment calls it
    // on the previous temporary before every lookup.
    virtual void release_temporary_00b67700(int handle) = 0;
    // 00B67800 BSP_LuaObject_GetByName, __thiscall(table, out, const char*), RET 8.
    virtual int get_by_name_00b67800(int table, const char* key) = 0;
    // 00B67690, __thiscall(dst, src) at 0083CE84: assigns the returned wrapper into the
    // fragment's current-table slot, which is what makes the three key lookups run
    // against the TurnMultipliers sub-table rather than its parent.
    virtual void assign_current_table_00b67690(int handle) = 0;
    // 00B67720 BSP_LuaObject_GetByIndex, __thiscall(table, out, int), RET 8. The fragment
    // uses index 1 for the throttle and index 2 for the multiplier.
    virtual int get_by_index_00b67720(int table, int index) = 0;
    // 00B66270 BSP_LuaObject_GetNumber, __thiscall(LuaObject), RET, ST0 rounded to float.
    virtual float get_number_00b66270(int handle) = 0;
    // The wrapper the fragment holds at [ESP+0BCh] on entry to the block, which is
    // ShipGlobals["Navigator"]: the immediately preceding reads are that table's
    // AutoThrust fields (0083CDD7 and 0083CE19 push "HdgDiffDangerMul" and its neighbour).
    virtual int current_table() = 0;
};

// 0083CE56..0083D10D of 0083B5E0, transcribed as a sequence over that host. Returns the
// six settings fields; the native code stores them straight into the singleton.
UnitRudderCurveSettings unit_rudder_curve_load_0083ce56(UnitRudderCurveLoaderHost& host);

// The same six fields from an already-decoded table, without a Lua host. The mapping is
// the loader's: index 1 of each key is the speed coordinate, index 2 the denominator.
UnitRudderCurveSettings unit_rudder_curve_settings(const UnitTurnMultiplierTable& table) noexcept;

// ---------------------------------------------------------------------------
// 008E6430, the gameplay-modifier product
// ---------------------------------------------------------------------------

// One registered modifier: the filter record 008E4680 tests and the factor 008E6430
// multiplies in. Only the factor is modelled here; the filter is the host's answer.
struct GameplayModifierEntry {
    // The record's own identity, so a host can answer the filter question per entry.
    int record_id{0};
    float factor{1.0f};  // record+1Ch, the float the product multiplies by
};

struct GameplayModifierHost {
    virtual ~GameplayModifierHost() = default;
    // 008E4680, __cdecl(record+8h, unit) -> bool. Its body is a six-clause reject chain
    // over the unit's kind, owner and index fields; it is NOT reconstructed here, so this
    // method is `contract: unread` beyond "does this record apply to this unit".
    virtual bool entry_matches_008e4680(const GameplayModifierEntry& entry) = 0;
};

// 008E6430, __thiscall(manager, int category, unit), RET 8, ST0 result. Walks the list at
// manager+80h + category*0Ch and multiplies the running float by every matching record's
// +1Ch. The accumulator starts at the 1.0f at 00D7A24C, so an empty or fully filtered list
// returns exactly 1.0f. The multiply order is list order.
float gameplay_modifier_product_008e6430(const GameplayModifierEntry* entries, int count,
                                         GameplayModifierHost& host);

// 0080FC30, __thiscall(unit), RET 0, ST0 result: unit+9C0h scaled by the category-4
// product, but only when the byte at 00E0C978 is set AND the manager's category-4 list is
// non-empty (the `[manager+B8h] != 0` test at 0080FC43 is that list's size). Otherwise the
// literal 1.0f at 00D7A24C is used, which is the same value an empty list would produce.
inline constexpr int kGameplayModifierCategoryReferenceSpeed = 4;

}  // namespace bsp
