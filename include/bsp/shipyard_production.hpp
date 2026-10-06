#pragma once

// Packet cc9_shipyard_production: the shipyard's production queue (MShipyard).
// docs/GUNNERY_OPEN_ITEMS.md sections 134 and 136 have the evidence.
//
// The image keeps three vectors on the shipyard unit: the stock records at
// +774h (stride 20h), the hangars at +780h (stride 10h) and the queue entries at
// +794h (stride 4Ch). 00849A30 builds the stock and sizes the entries from the
// scene bag; 00849F70 (the deferred mode-1 pass) fills the hangars and the
// authored slots; AddShipyardStock (00896CC0 -> 0084ACB0) adds stock; the
// strategic map's purchase (00673A10) sends A7h / A8h / A9h through
// 00847030; the tick 00846320 releases hangars and builds through 00844FC0.
//
// This file holds the state and the pure parts. The gunnery host owns the walk
// (it can see units, paths and the class table); unit creation belongs to the
// SpawnNew lane and is reached through the seam at the bottom.
//
// Names are hypotheses, not recovered symbols.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// Packet cc9_shipyard_production. True: the gunnery host completes 00849F70 for
// every registered shipyard, runs 00846320's walk each fixed step and accepts
// the strategic map's purchase (GameGunneryHost::shipyard_order). False: none
// of it; the registry may still be filled by the scene, and nothing reads it.
inline constexpr bool kShipyardProductionBound = true;  // ON: GUNNERY_OPEN_ITEMS 146

inline constexpr std::size_t kShipyardNone = static_cast<std::size_t>(-1);

// Entry +4. 0 idle, 1 class chosen (A7h, or a scene `UnitClass`), 2 ordered
// (A9h), 3 building (00844FC0 at 0084500E), 4 launched (00846320's release, or
// a scene `Slot %d` that names a live unit).
enum class ShipyardEntryState : int {
    Idle = 0,
    Chosen = 1,
    Ordered = 2,
    Building = 3,
    Launched = 4,
};

// +774h, 008485F0: +4 the class (compared by class+70h, its id), +8 the count,
// +10h..+14h the names (stride 1Ch, the argument split on ',' by 0094EDC0, the
// delimiter at 00CE4BFC), +1Ch the next name. 0094EDC0 was not read: how it
// treats an empty string or an empty token is unknown.
struct ShipyardStockRecord {
    std::uint32_t class_id{0};
    int count{0};
    std::vector<std::string> names;
    std::uint32_t next_name{0};
};

// +780h, 00848150 from 00849F70: +4 the `Object` entity, +8 the `Path` entity
// (the path interface less 1E4h), +0Ch the unit it launched (0 = free). This
// process holds the two by their authored names.
struct ShipyardHangar {
    std::string object;
    std::string path;
    std::size_t launched_unit{kShipyardNone};
};

// +794h, the default 00849A30 pushes at 00849E8B..00849ECF: state 0, class 0,
// +0Ch = -1, the rest clear.
struct ShipyardEntry {
    ShipyardEntryState state{ShipyardEntryState::Idle};
    std::uint32_t class_id{0};          // +8
    int equipment{-1};                  // +0Ch, the class's Equipments index (A8h)
    std::string name;                   // +10h, taken from the stock at A9h
    std::size_t hangar{kShipyardNone};  // +2Ch
    std::size_t unit{kShipyardNone};    // +30h
    std::size_t order_target{kShipyardNone};  // +48h, the observed order object
};

// What 00849A30 and 00849F70 read from the bag, in authored order.
struct ShipyardSceneStock {        // "Stock %d", 1..12 (00849E65 CMP EAX,0Ch)
    int count{0};                  // `Count` (00CE5710)
    std::uint32_t class_id{0};     // `Type` resolved; 0 when the value is <= 0
    std::string names;             // `Names` (00D0B974)
};
struct ShipyardSceneHangar {       // "Hangar %d", 1..12 (local_d0 < 0Dh)
    std::string object;            // `Object` (00CF8F00), last path component
    std::string path;              // `Path` (00CEA738)
};
struct ShipyardSceneSlot {         // "Slot %d", 1..NumSlots
    bool present{false};           // the block exists (type 6, non-null)
    std::string unit;              // `Unit` (00CF7C8C), a reference (type 5)
    bool unit_class_present{false};
    std::uint32_t unit_class{0};   // `UnitClass` (00D0B9B4) resolved
    int unit_class_raw{0};         // its +0Ch, tested > 0 (0084A2E8)
    int unit_equipment{-1};        // `UnitEquipment` (00D0B9A4) +0Ch
};
struct ShipyardSceneAuthored {
    int num_slots{0};              // `NumSlots` (00CE56FC)
    std::vector<ShipyardSceneStock> stocks;
    std::vector<ShipyardSceneHangar> hangars;
    std::vector<ShipyardSceneSlot> slots;
};

struct ShipyardState {
    std::string owner_name;
    int owner_entity_id{0};
    std::vector<ShipyardStockRecord> stock;
    std::vector<ShipyardHangar> hangars;
    std::vector<ShipyardEntry> entries;
    // 00849F70 runs after the attach; the bag is kept until the host completes it.
    ShipyardSceneAuthored authored;
    bool slots_completed{false};
};

// 00849A30's stock loop (00849D94..00849E6C) and the entry vector it sizes by
// `NumSlots` (00849E7B..00849ED6). A stock is kept when Count != 0 and the class
// resolved (00849E04, 00849E0A). The authored hangars and slots wait in
// `authored` for shipyard_complete_slots_00849f70.
ShipyardState shipyard_scene_attach_00849a30(const std::string& owner_name,
                                             const ShipyardSceneAuthored& authored);

// 00849F70's mode-1 arms. `unit_class` answers the class id (entity+538h) of a
// live entity by name, or 0 when no entity has that name. Answers the number of
// stock records a slot drew from; `missing_stock` counts slots whose class has
// no record (the image faults there, 0084A3D1).
struct ShipyardSlotFill {
    std::size_t hangars{0};
    std::size_t launched{0};   // state 4 from `Unit`
    std::size_t chosen{0};     // state 1 from `UnitClass`
    std::size_t missing_stock{0};
};
using ShipyardUnitClassFn = std::uint32_t (*)(const std::string& unit, void* context);
using ShipyardUnitIndexFn = std::size_t (*)(const std::string& unit, void* context);
ShipyardSlotFill shipyard_complete_slots_00849f70(ShipyardState& state,
    ShipyardUnitClassFn unit_class, ShipyardUnitIndexFn unit_index, void* context);

// 0084ACB0 AddShipyardStock(class id, count, names): the record whose class id
// matches gains `count`; otherwise a new record (008485F0, 008499A0). Answers
// the record's count after the add.
int shipyard_add_stock_0084acb0(ShipyardState& state, std::uint32_t class_id, int count,
                                const std::string& names);

// The class facts the handlers read: class+134h `DefaultEquipment` and the size
// of `Equipments` at class+128h (00951F10).
struct ShipyardClassFacts {
    virtual ~ShipyardClassFacts() = default;
    virtual int default_equipment_134(std::uint32_t class_id) const = 0;
    virtual int equipment_count_128(std::uint32_t class_id) const = 0;
};

// A7h 00844D60: hand the entry's class back to its stock record (+8 += 1), then
// step from that record (from -1 with `forward` on an empty entry, or from the
// end without it) to the next record with stock; take one, state 1, class,
// +0Ch = class+134h. None: state 0, class 0. Answers false on a bad entry.
bool shipyard_choose_class_00844d60(ShipyardState& state, std::size_t entry, bool forward,
                                    const ShipyardClassFacts& facts);

// A8h 008436F0: +0Ch = (+0Ch + step) % (N + 1), N = class+128h, step 1 with
// `forward`, else N. Answers false on a bad entry or an entry with no class
// (the image reads class+128h through a null class there).
bool shipyard_cycle_equipment_008436f0(ShipyardState& state, std::size_t entry, bool forward,
                                       const ShipyardClassFacts& facts);

// A9h 00846D90's queue half: the stock record of the entry's class, state 2, the
// record's next name (cycled), and with `forward` the order object. Answers
// false when no record has the class or the record has no names (the image
// faults at 00846E81 / divides by zero at 00846F00 there).
bool shipyard_order_entry_00846d90(ShipyardState& state, std::size_t entry, bool forward,
                                   std::size_t order_target);

// 00844CE0: the first hangar whose object is usable (+5Ch set, +5Dh, +5Eh and
// +60h clear) and has launched nothing. `usable[i]` stands for those four bytes.
std::size_t shipyard_free_hangar_00844ce0(const ShipyardState& state,
                                          const std::vector<bool>& usable);

// 00844610(class id): the stock count of the class plus the entries in state 1
// that hold it.
int shipyard_available_00844610(const ShipyardState& state, std::uint32_t class_id);

// Which creator 00844FC0 takes, by the class's kind tests in order:
// vtable+18h(0Fh) plane; 0Dh, 0Ah, 07h, 0Bh destroyer; 09h mother ship; 08h
// submarine; 0Eh torpedo boat; 0Ch landing ship; otherwise none (00845416).
enum class ShipyardCreator : int {
    None = 0,
    PlaneSquadron,   // 004F0AD0, WingCount 1, VelocitySI 0, State 6, Equipment
    Destroyer,       // 004F0520
    MotherShip,      // 004F0860
    Submarine,       // 004F05F0, Dive 0
    TorpedoBoat,     // 004F06C0
    LandingShip,     // 004F0790
};
ShipyardCreator shipyard_creator_for_kind_00844fc0(int vehicle_class_kind);

// The unit 00844FC0 creates. The bag is `Type` (class+70h), `Skill` (the
// shipyard's vtable[12Ch]), `Race` (+58h), `Party` (+54h), `OwnerPlayer`
// (+188h), `ShipYardLaunch` = 1, and per creator `WingCount` 1, `VelocitySI` 0,
// `State` 6 and `Equipment` (when +0Ch >= 0) for a plane, `Dive` 0 for a
// submarine. The frame: rows (1,0,0), (0,1,0), p1 - p0 of the hangar's path,
// then 0085DC80; the translation is p0 with y = the water height there
// (0078CF20). After the creation 00844FC0 issues `moveonpath` (00E08F80) on the
// hangar's path (0084549E..008454CB, the path entity's +174h id).
struct ShipyardBuildRequest {
    std::string shipyard_name;
    std::size_t shipyard_unit{kShipyardNone};
    std::size_t entry{0};
    std::uint32_t class_id{0};
    ShipyardCreator creator{ShipyardCreator::None};
    int equipment{-1};
    std::string gui_name;    // entry +10h; no reader in 00844FC0 (labelled)
    std::string path_name;   // the hangar's `Path`, for `moveonpath`
    float frame[16]{};       // y of the translation still to be put on the water
    bool snap_to_water{true};
};

// The SpawnNew lane's creation entry. Answers the new unit's index, or
// kShipyardNone. Null until that lane registers it; the build is then refused
// and counted.
using ShipyardCreateUnitFn = std::size_t (*)(const ShipyardBuildRequest& request);
void shipyard_set_create_unit(ShipyardCreateUnitFn fn) noexcept;
ShipyardCreateUnitFn shipyard_create_unit() noexcept;

// Process-level registry, keyed by the shipyard's authored name, the shape
// bsp::air_ops_decks() has and for the same reason (the scene-contents, Lua and
// gunnery hosts each hold one end).
class ShipyardRegistry {
public:
    void clear() noexcept;
    void set(const std::string& unit_name, ShipyardState state);
    void bind_entity_id(int entity_id, const std::string& unit_name);
    ShipyardState* find_mutable(const std::string& unit_name) noexcept;
    ShipyardState* find_mutable_by_entity_id(int entity_id) noexcept;
    std::size_t size() const noexcept;
    ShipyardState* mutable_at(std::size_t index) noexcept;

private:
    std::vector<ShipyardState> yards_;
};

ShipyardRegistry& shipyards() noexcept;

}  // namespace bsp
