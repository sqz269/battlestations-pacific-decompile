#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/in_mission_subsystem_tick.hpp"

// The eight local-player unit lists at game+1964h..+19B8h, built once per
// mission scene load by 004C3CB0.
//
// docs/LOCAL_PLAYER_UNIT_LISTS.md carries the evidence.
// bsp/in_mission_subsystem_tick.hpp already declares the guard side
// (UnitListsGate, local_player_unit_lists_run_004c3cb0,
// kUnitListsBuiltLatchOffset, kLocalPlayerSlotArrayOffset,
// kLocalPlayerSlotSelectorOffset, kUnitListsSlotBound, kUnitListHeadOffsets);
// those are reused here, never redefined. This header adds the body
// 004C3D06..004C4098: the list ABI, the three walks, the kind codes each walk
// tests and the merge tail.
//
// bsp/unit_instance.hpp already declares unit_is_kind_of_006fe530, the
// MDestroyer implementation of the same vtable slot 5Ch this file queries, and
// kUnitDestroyerAncestry. The class ids below extend that space; see the
// Corrections section of the doc for the three constants in other headers that
// this packet's evidence contradicts.
//
// Every name below is a hypothesis, not a recovered symbol, except the M*
// class names, which are string constants in the image.

namespace bsp {

// ---------------------------------------------------------------------------
// Class ids: the argument of the unit's vtable slot 5Ch, bool IsKindOf(int)
// ---------------------------------------------------------------------------
// Only the eight 004C3CB0 passes are named as constants. kUnitClassTable below
// carries every id this packet recovered a name or a role for.

inline constexpr int kUnitClassShipBase = 0x06;            // 009635E0 = [6,5,4]
inline constexpr int kUnitClassPlaneBase = 0x0f;           // 007CFD00 = [15,5,4]
inline constexpr int kUnitClassPlaneSquadron = 0x18;       // 007EFB00 = [24,2,1,0]
inline constexpr int kUnitClassLandFort = 0x1b;            // 00749030 -> "MLandFort"
inline constexpr int kUnitClassOrdnanceBase = 0x2a;        // 006EA290 -> "MBomb"
inline constexpr int kUnitClassDummyTargetVehicle = 0x35;  // 009600A0 -> "MDummyTarget"
inline constexpr int kUnitClassAirfield = 0x45;            // 0095FF30 -> "MAirfield"
inline constexpr int kUnitClassShipyard = 0x46;            // 0095FFA0 -> "MShipyard"

// The vehicle root every VehicleClass descriptor chain ends with ([.., 5, 4]).
// 008DDF90 asks for it to decide whether a unit is a vehicle at all.
inline constexpr int kUnitClassVehicleRoot = 0x05;         // 00749010 = [5,4]

// One row of the recovered id space. `is_kind_of` is the implementation the id
// was read from, `descriptor_vtable` the vtable whose +0Ch name getter produced
// `name` (0 when the class has no name getter). `name` is nullptr for the
// unnamed roots, which are described by what derives from them.
struct UnitClassRow {
    int class_id;
    const char* name;
    std::uint32_t is_kind_of;
    std::uint32_t descriptor_vtable;
};
extern const UnitClassRow kUnitClassTable[41];
inline constexpr std::size_t kUnitClassTableCount = 41;

// Linear search over kUnitClassTable; nullptr when the id is not in it.
const UnitClassRow* unit_class_row(int class_id) noexcept;

// ---------------------------------------------------------------------------
// The intrusive list ABI. 00484540, 004BF8E0 and 004C2BE0 all work on the same
// {count, head, tail} head and 0Ch node.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitListCountOffset = 0x00; // 0048456E, 004BF8E3
inline constexpr std::size_t kUnitListHeadOffset = 0x04;  // 00484586, 004BF8E8
inline constexpr std::size_t kUnitListTailOffset = 0x08;  // 00484569, 004BF910
inline constexpr std::size_t kUnitListHeadSize = 0x0c;    // the stride of kUnitListHeadOffsets

inline constexpr std::size_t kUnitListNodePrevOffset = 0x00;  // 0048456C
inline constexpr std::size_t kUnitListNodeNextOffset = 0x04;  // 00484575
inline constexpr std::size_t kUnitListNodeValueOffset = 0x08; // 00484566
inline constexpr std::size_t kUnitListNodeSize = 0x0c;        // 00484542 PUSH 0Ch

// A unit instance pointer, kept opaque. The lists store it directly; the world
// lists store a wrapper whose +4h is the unit (kWorldListEntryUnitOffset).
using UnitRef = std::uint32_t;

// The eight heads, in the address order of kUnitListHeadOffsets. The names are
// what each list ends up holding, from the tests that fill it.
enum class LocalPlayerUnitList : int {
    kWalk0Ships = 0,   // game+1964h
    kWalk0Rest = 1,    // game+1970h, everything in walk 0 that is not ordnance
    kShips = 2,        // game+197Ch
    kSquadrons = 3,    // game+1988h
    kAirfields = 4,    // game+1994h
    kShipyards = 5,    // game+19A0h
    kLandForts = 6,    // game+19ACh
    kMerged = 7,       // game+19B8h, the merge target of the tail
};
inline constexpr std::size_t kLocalPlayerUnitListCount = 8;

// The offset of one list head inside GGame; the same values as
// kUnitListHeadOffsets, indexed by the enum.
std::size_t local_player_unit_list_offset(LocalPlayerUnitList list) noexcept;

// 00484540, __thiscall void(List*, void* value), RET 4. push_back.
void unit_list_push_back_00484540(std::vector<UnitRef>& list, UnitRef unit);

// 004BF8E0, __thiscall void(List*), RET. Erases and frees every node: the loop
// at 004BF8E8..004BF922 repeats while count != 0. It is not a single pop.
void unit_list_clear_004bf8e0(std::vector<UnitRef>& list) noexcept;

// 004C2BE0, __thiscall void(List* dst, List* src), RET 4. Appends every value
// of src to dst and leaves src untouched.
void unit_list_append_all_004c2be0(std::vector<UnitRef>& dst,
                                   const std::vector<UnitRef>& src);

// The eight lists as one object, indexed by LocalPlayerUnitList.
using LocalPlayerUnitLists = std::array<std::vector<UnitRef>, kLocalPlayerUnitListCount>;

// 004BFDF0, __thiscall void(GGame*): 004BF8E0 on each of the eight heads in
// address order, the last as a tail call.
void clear_all_unit_lists_004bfdf0(LocalPlayerUnitLists& lists) noexcept;

// ---------------------------------------------------------------------------
// The source: the local player's unit registry at [[game+18CCh + slot*4]+30h]
// ---------------------------------------------------------------------------
// Five {count, head, tail} triples. The producer 008050E0 zeroes exactly
// +DD8h..+E10h (0080518D..008051DB); 008073C0 clears the same five through
// 008042B0. 004C3CB0 walks the heads of triples 0, 1 and 3.
inline constexpr std::size_t kUnitRegistryOffset = 0x30;            // 004C3CF5
inline constexpr std::size_t kUnitRegistryTripleOffsets[5] = {
    0xdd8, 0xde4, 0xdf0, 0xdfc, 0xe08,
};
// The head of an entry in one of those lists points at a wrapper, not the unit.
inline constexpr std::size_t kWorldListEntryUnitOffset = 0x04;      // 004C3D09

// The three walks, in body order. The value is the index into
// kUnitRegistryTripleOffsets, which is what the head offsets 004C3CF8,
// 004C3D61 and 004C3EB4 resolve to.
enum class UnitRegistryWalk : int {
    kWalk0 = 0, // head +DDCh, 004C3CF8
    kWalk1 = 1, // head +DE8h, 004C3D61
    kWalk2 = 3, // head +E00h, 004C3EB4
};

// ---------------------------------------------------------------------------
// The per-unit filter, identical in all three walks
// ---------------------------------------------------------------------------
// 004C3D0C..004C3D1E, 004C3D76..004C3D94, 004C3EC8..004C3EE6.
struct UnitListFilterFlags {
    bool active_5c{};   // must be set; the world tick's gate byte
    bool flag_5d{};     // must be clear
    bool flag_60{};     // must be clear
    bool flag_5e{};     // must be clear
};
bool unit_passes_list_filter(const UnitListFilterFlags& flags) noexcept;

// ---------------------------------------------------------------------------
// The host: one method per native call site of 004C3CB0
// ---------------------------------------------------------------------------
struct LocalPlayerUnitListsHost {
    virtual ~LocalPlayerUnitListsHost() = default;

    // 004C3CE3 -> 004BFDF0. Clears all eight lists before the walks start.
    virtual void clear_all_lists_004bfdf0() = 0;

    // The walk itself is inline code, not a call: the units of one registry
    // list in native order, each already resolved through +8h then +4h.
    virtual const std::vector<UnitRef>& registry_walk_units(UnitRegistryWalk walk) = 0;

    // 004C3D0C and its two copies: the four filter bytes of one unit.
    virtual UnitListFilterFlags unit_filter_flags(UnitRef unit) = 0;

    // The virtual at vtable+5Ch, bool IsKindOf(int), RET 4. One call per query;
    // the classify_walk* rules below make them in the native order and stop at
    // the first that answers true.
    virtual bool unit_is_kind_of(UnitRef unit, int class_id) = 0;

    // 004C3E90 -> 008DDF90 with ECX = [game+21A4h + [00E188A8+18ECh]*4], the
    // local player's SzurkeNyil set. True when the unit, or the plane at
    // unit+3D0h for a squadron, is in that set.
    virtual bool unit_in_local_objective_set_008ddf90(UnitRef unit) = 0;

    // 00484540 and the four copies MSVC inlined at 004C3DA9, 004C3EFB,
    // 004C3F5D and 004C3FBF.
    virtual void append_00484540(LocalPlayerUnitList list, UnitRef unit) = 0;

    // 004C2BE0, the five merge calls at 004C405B..004C4093, all with
    // ECX = game+19B8h.
    virtual void append_all_004c2be0(LocalPlayerUnitList dst, LocalPlayerUnitList src) = 0;
};

// The decision one walk reaches for one unit. Both appends are used only by
// walk 0, which puts a ship in kWalk0Ships and then in kWalk0Rest.
struct UnitWalkDecision {
    bool append_first{false};
    LocalPlayerUnitList first{LocalPlayerUnitList::kWalk0Ships};
    bool append_second{false};
    LocalPlayerUnitList second{LocalPlayerUnitList::kWalk0Rest};
};

// 004C3D0C..004C3D55. IsKindOf(2Ah) true drops the unit; otherwise it always
// reaches kWalk0Rest and additionally kWalk0Ships when IsKindOf(06h).
UnitWalkDecision classify_walk0_unit(LocalPlayerUnitListsHost& host, UnitRef unit);

// 004C3D76..004C3EA0, the full chain: 06h, 18h, 0Fh (excluded), 45h, 46h, 1Bh,
// 35h, then 008DDF90. At most one list.
UnitWalkDecision classify_walk1_unit(LocalPlayerUnitListsHost& host, UnitRef unit);

// 004C3EC8..004C403C, the same chain without 0Fh, 35h and the 008DDF90 call.
UnitWalkDecision classify_walk2_unit(LocalPlayerUnitListsHost& host, UnitRef unit);

// The merge tail 004C404C..004C4098, in native order. kWalk0Ships and
// kWalk0Rest are not merged, and the sources are not cleared.
inline constexpr LocalPlayerUnitList kUnitListMergeSources[5] = {
    LocalPlayerUnitList::kShips,
    LocalPlayerUnitList::kSquadrons,
    LocalPlayerUnitList::kAirfields,
    LocalPlayerUnitList::kShipyards,
    LocalPlayerUnitList::kLandForts,
};

// The whole of 004C3CB0: the guard of bsp/in_mission_subsystem_tick.hpp, the
// clear, the three walks in body order and the merge tail. Returns what
// local_player_unit_lists_run_004c3cb0 returned, so a caller can tell a run
// from the no-op every later frame takes.
bool build_local_player_unit_lists_004c3cb0(UnitListsGate& gate,
                                            LocalPlayerUnitListsHost& host);

// ---------------------------------------------------------------------------
// 008DDF90, the SzurkeNyil membership test
// ---------------------------------------------------------------------------
// The eight sets live at game+21A4h..+21C0h (docs/GAME_WORLD_CONSTRUCT.md row
// 20); the class string "SzurkeNyil" sits at 00D16100, immediately before its
// vtable 00D1610C.
inline constexpr std::size_t kObjectiveSetArrayOffset = 0x21a4;  // 004C3E88
inline constexpr std::size_t kObjectiveSetTreeOffset = 0x18;     // 008DDF09, 008DF931
inline constexpr std::size_t kObjectiveSetHeadOffset = 0x1c;     // 008DDF04, 008DF948
inline constexpr std::size_t kObjectiveSetSizeOffset = 0x20;     // 008DDF93
// A squadron is looked up by its plane instead of itself.
inline constexpr std::size_t kSquadronPlaneOffset = 0x3d0;       // 008DDFC2, 004C08C8

struct ObjectiveSetQuery {
    std::size_t set_size{0};       // [this+20h]
    bool unit_is_vehicle{false};   // IsKindOf(05h)
    bool unit_is_squadron{false};  // IsKindOf(18h)
    bool subject_in_set{false};    // the 008DDF00 lookup on the resolved subject
};
// 008DDF90..008DDFD9, __thiscall char(SzurkeNyil*, unit), RET 4.
bool objective_set_contains_008ddf90(const ObjectiveSetQuery& query) noexcept;

} // namespace bsp
