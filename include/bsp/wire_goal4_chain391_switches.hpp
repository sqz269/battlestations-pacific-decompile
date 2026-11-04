#pragma once

namespace bsp {

// Goal 4 chain 391: the vehicle-class descriptor's Lua load (vtable +8h). Every
// kind's loader reaches the base reader 00960230 first, so the host's
// GameMissionLuaVehicleClassBinding::load_descriptor runs
// bsp::read_vehicle_class_base_fields_00960230 over the class row and keeps what
// it read per class. The descriptor's +0C0h linked class, the ship catapult block
// (class +C0h..+CCh) and the launched class's Cost (+124h) are then sourced from
// that load. OFF keeps the direct `VehicleClass[index]` reads.
inline constexpr bool kWireGoal4Chain391Bound = true;

} // namespace bsp
