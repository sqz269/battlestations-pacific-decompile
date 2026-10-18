#pragma once

// Ship catapult launches for the mission host (packet p17_catapult_adapter,
// docs/OCEAN_WAVE_FIELD.md sections 37 and 67). The chain in the image:
// ShipUseCatapult 00891D50 -> message 7Bh -> 008141E0 (ready = ship +638h > 0 and
// every catapult device's vtable[1D0h](1), 006EBD30) -> 006EB680 -> the device tick
// 006ED1C0 -> 006EC8E0 MCatapult::Fire (004F0AD0 with the LaunchedClass bag, State
// 3) -> 006ED261's timeline: VEHICLE_ADD_LAUNCHED (00957450 on the ship: the first
// empty of ten child slots +550h, +630h = slot, +638h decremented) and the takeoff.
//
// LABELLED SUBSTITUTIONS (the host has no catapult device instances and no
// catapult-held plane state):
//  - one catapult per ship whose class carries Catapult.LaunchedClass >= 0, with
//    the device's own gate (006EBD30) reduced to its class part: fewer live
//    children than Catapult.MaxLaunchedPlanes; the device ammo (+480h) and reload
//    timer (+414h) are not modelled (the device-class key mapping is not read);
//  - the plane is created at once in flight (State 7, WingCount 1) above the ship,
//    along the ship's heading, instead of State 3 on the catapult node followed by
//    the takeoff impulse at turn_time + 0.5 s; the child registration and the
//    stock decrement of 00957450 happen at the same call, not 1 s later;
//  - a child slot reads empty once its plane is dead (the image's slot is an
//    observer handle that 00694A60 clears when the child goes);
//  - the points gate of 008141E0 (sessions only) is not taken.

#include "bsp/gameplay_seg61_62_cc13.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>

namespace bsp {

// False: ShipUseCatapult stays a record, GetNumCatapulted / SetCatapultStock stay
// unbound, GetLastCatapulted answers nil and GetCatapultStock reads the class value.
inline constexpr bool kShipCatapultLaunchBound = false;

// The ship class's Catapult block (00960230 reads it into class +C0h..+CCh).
struct ShipCatapultClass {
    std::int32_t launched_class = -1;   // class +C0h, LaunchedClass
    std::int32_t equipment = 0;         // class +C4h, Equipment
    std::int32_t launch_stock = 0;      // class +C8h, LaunchStock
    std::int32_t max_launched = 0;      // class +CCh, MaxLaunchedPlanes
};

struct ShipCatapultRecord {
    gameplay_seg61_62_cc13::LaunchedChildSlots slots;           // +550h..+638h; child_550 holds entity ids
    std::uint32_t launches = 0;
    std::uint32_t refusals = 0;
};

// What 008141E0's gate decided for one ShipUseCatapult call.
enum class ShipCatapultVerdict {
    kLaunch,
    kNoCatapult,        // no LaunchedClass: the device walk finds no IsKindOf(28h)
    kNoStock,           // +638h <= 0
    kTooManyInAir,      // 006EBD4C: live children >= MaxLaunchedPlanes
};

class ShipCatapultRegistry {
public:
    void clear() { records_.clear(); }
    // The record of `unit`, made on first use with +638h = LaunchStock (0095E5B0)
    // and +630h = -1.
    ShipCatapultRecord& record(std::size_t unit, const ShipCatapultClass& cls);
    ShipCatapultRecord* find(std::size_t unit);
    // Drops children for which `alive` answers false (the observer handles).
    static void prune(ShipCatapultRecord& r, const std::function<bool(std::uint32_t)>& alive);
    // 00953920: the non-empty slots.
    static std::int32_t count_00953920(const ShipCatapultRecord& r);
    // 00953A60: [+630h] < 0 answers 0, else the child in that slot.
    static std::uint32_t last_00953a60(const ShipCatapultRecord& r);
    static ShipCatapultVerdict gate_008141e0(const ShipCatapultRecord& r,
                                             const ShipCatapultClass& cls);
    // 00957450 with the child's entity id; returns the slot or -1.
    static int add_child_00957450(ShipCatapultRecord& r, std::uint32_t entity_id);

private:
    std::map<std::size_t, ShipCatapultRecord> records_;
};

// Process-wide: the mission Lua host owns the natives and the spawn, and a new
// mission clears it.
ShipCatapultRegistry& ship_catapult_registry();

}  // namespace bsp
