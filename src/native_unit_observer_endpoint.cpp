#include "bsp/native_unit_observer_endpoint.hpp"

namespace bsp {

void initialize_unit_observed_prefix_00925cff(NativeUnitObserverPrefixStorage& unit) noexcept {
    unit.observed_00.native_vtable_00 = 0x00ceccc8u;
    unit.observed_00.edges_04.data_00 = nullptr;
    unit.observed_00.edges_04.count_04 = 0;
    unit.observed_00.edges_04.capacity_08 = 0;
}

void initialize_unit_callback_prefix_00925d13(NativeUnitObserverPrefixStorage& unit) noexcept {
    unit.callback_10.native_vtable_00 = 0x00ce3cd4u;
    unit.callback_10.edges_04.data_00 = nullptr;
    unit.callback_10.edges_04.count_04 = 0;
    unit.callback_10.edges_04.capacity_08 = 0;
}

void publish_scene_observer_tables_00925d44(NativeUnitObserverPrefixStorage& unit) noexcept {
    unit.observed_00.native_vtable_00 = 0x00d19120u;
    unit.callback_10.native_vtable_00 = 0x00d19104u;
}

void publish_game_entity_observer_tables_00928662(NativeUnitObserverPrefixStorage& unit) noexcept {
    unit.observed_00.native_vtable_00 = 0x00d192e0u;
    unit.callback_10.native_vtable_00 = 0x00d192c8u;
}

namespace {
// Each row is a pair of observed MOV stores in one actual leaf constructor.
// Full store addresses, raw bytes and coverage are recorded in the report.
constexpr NativeUnitObserverTablePair tables[] = {
    {0x006fe590u, 0x00cfc3d0u, 0x00cfc3b8u}, // Destroyer
    {0x006fb430u, 0x00cfb738u, 0x00cfb71cu}, // Cruiser
    {0x0074be00u, 0x00cffa30u, 0x00cffa18u}, // LandingShip
    {0x006eb290u, 0x00cfa778u, 0x00cfa75cu}, // Cargo
    {0x006dfef0u, 0x00cf90b0u, 0x00cf9094u}, // BattleShip
    {0x008531a0u, 0x00d0bf80u, 0x00d0bf68u}, // Submarine
    {0x00857e20u, 0x00d0c648u, 0x00d0c630u}, // TorpedoBoat
    {0x00758d30u, 0x00d01630u, 0x00d01614u}, // MotherShip
    {0x008091d0u, 0x00d00070u, 0x00d00058u}, // ReconPlane
    {0x0084ca50u, 0x00d0ba80u, 0x00d0ba68u}, // SmallReconPlane
    {0x0074e540u, 0x00d00308u, 0x00d002f0u}, // LargeReconPlane
    {0x007ddae0u, 0x00d06920u, 0x00d06908u}, // Fighter
    {0x00956390u, 0x00d19d28u, 0x00d19d10u}, // DiveBomber
    {0x009564e0u, 0x00d1a000u, 0x00d19fe8u}, // TorpedoBomber
    {0x00956240u, 0x00d1a2d8u, 0x00d1a2c0u}, // Kamikaze
    {0x007d7850u, 0x00d06638u, 0x00d06620u}, // LevelBomber
    {0x006d3110u, 0x00cf8c08u, 0x00cf8becu}, // AirField
    {0x00848380u, 0x00d0b770u, 0x00d0b754u}, // Shipyard
    {0x0074df10u, 0x00cffde0u, 0x00cffdc8u}, // LandVehicle
    {0x00747000u, 0x00cff3f8u, 0x00cff3e0u}, // LandFort
    {0x006f5c10u, 0x00cfb028u, 0x00cfb00cu}, // CommandBuilding
};
} // namespace

const NativeUnitObserverTablePair* unit_observer_tables_for_creator(
    std::uint32_t creator) noexcept {
    for (const auto& row : tables) if (row.creator == creator) return &row;
    return nullptr;
}

bool publish_unit_leaf_observer_tables_for_creator(
    NativeUnitObserverPrefixStorage& unit, std::uint32_t creator) noexcept {
    const auto* row = unit_observer_tables_for_creator(creator);
    if (!row) return false;
    unit.observed_00.native_vtable_00 = row->observed_table;
    unit.callback_10.native_vtable_00 = row->callback_table;
    return true;
}
} // namespace bsp
