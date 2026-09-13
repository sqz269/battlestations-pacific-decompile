#include "bsp/unit_world_registration.hpp"
#include "bsp/unit_instance_layout.hpp"
#include "bsp/vehicle_class.hpp"

// Complete native registration schedules; docs/UNIT_WORLD_REGISTRATION.md.
namespace bsp {
namespace {

constexpr std::size_t kListsCruiser[] = {0x24, 0x30, 0x48, 0x54, 0x60, 0x90};
constexpr std::size_t kListsLandingShip[] = {0x24, 0x30, 0x48, 0x54, 0x60, 0xa8};
constexpr std::size_t kListsCargo[] = {0x24, 0x30, 0x48, 0x54, 0x60, 0x9c};
constexpr std::size_t kListsBattleShip[] = {0x24, 0x30, 0x48, 0x54, 0x60, 0xb4};
constexpr std::size_t kListsSubmarine[] = {0x24, 0x30, 0x48, 0x54, 0x60, 0x78};
constexpr std::size_t kListsTorpedoBoat[] = {0x24, 0x30, 0x48, 0x54, 0x60, 0xc0};
constexpr std::size_t kListsMotherShip[] = {0x24, 0x30, 0x48, 0x54, 0x60, 0x84};
constexpr std::size_t kListsReconPlane[] = {0x24, 0x30, 0x48, 0x54, 0xcc, 0x108};
constexpr std::size_t kListsSmallReconPlane[] = {0x24, 0x30, 0x48, 0x54, 0xcc, 0x114};
constexpr std::size_t kListsLargeReconPlane[] = {0x24, 0x30, 0x48, 0x54, 0xcc, 0x120};
constexpr std::size_t kListsFighter[] = {0x24, 0x30, 0x48, 0x54, 0xcc, 0xfc};
constexpr std::size_t kListsDiveBomber[] = {0x24, 0x30, 0x48, 0x54, 0xcc, 0xf0};
constexpr std::size_t kListsTorpedoBomber[] = {0x24, 0x30, 0x48, 0x54, 0xcc, 0xe4};
constexpr std::size_t kListsKamikaze[] = {0x24, 0x30, 0x48, 0x54, 0xcc, 0x12c};
constexpr std::size_t kListsLevelBomber[] = {0x24, 0x30, 0x48, 0x54, 0xcc, 0xd8};
constexpr std::size_t kListsAirField[] = {0x24, 0x30, 0x48, 0x54, 0x354};
constexpr std::size_t kListsShipyard[] = {0x24, 0x30, 0x48, 0x54, 0x360};
constexpr std::size_t kListsLandVehicle[] = {0x24, 0x30, 0x48, 0x54, 0x144};
constexpr std::size_t kListsLandFort[] = {0x24, 0x30, 0x48, 0x54, 0x15c};
constexpr std::size_t kListsCommandBuilding[] = {0x24, 0x30, 0x48, 0x54, 0x15c, 0x168};

const UnitWorldRegistration kRegistrations[] = {
    {0x006fe590u, 0x00cfc3d0u, 0x006fe620u,
     kUnitParentListOffsets, 6 }, // Destroyer
    {0x006fb430u, 0x00cfb738u, 0x006fb4c0u,
     kListsCruiser, 6 }, // Cruiser
    {0x0074be00u, 0x00cffa30u, 0x0074bcd0u,
     kListsLandingShip, 6 }, // LandingShip
    {0x006eb290u, 0x00cfa778u, 0x006eb410u,
     kListsCargo, 6 }, // Cargo
    {0x006dfef0u, 0x00cf90b0u, 0x006e0010u,
     kListsBattleShip, 6 }, // BattleShip
    {0x008531a0u, 0x00d0bf80u, 0x00853090u,
     kListsSubmarine, 6 }, // Submarine
    {0x00857e20u, 0x00d0c648u, 0x00857eb0u,
     kListsTorpedoBoat, 6 }, // TorpedoBoat
    {0x00758d30u, 0x00d01630u, 0x00758f90u,
     kListsMotherShip, 6 }, // MotherShip
    {0x008091d0u, 0x00d00070u, 0x0074e670u,
     kListsReconPlane, 6 }, // ReconPlane
    {0x0084ca50u, 0x00d0ba80u, 0x0084cae0u,
     kListsSmallReconPlane, 6 }, // SmallReconPlane
    {0x0074e540u, 0x00d00308u, 0x0074e700u,
     kListsLargeReconPlane, 6 }, // LargeReconPlane
    {0x007ddae0u, 0x00d06920u, 0x007ddb70u,
     kListsFighter, 6 }, // Fighter
    {0x00956390u, 0x00d19d28u, 0x00956300u,
     kListsDiveBomber, 6 }, // DiveBomber
    {0x009564e0u, 0x00d1a000u, 0x00956450u,
     kListsTorpedoBomber, 6 }, // TorpedoBomber
    {0x00956240u, 0x00d1a2d8u, 0x009561b0u,
     kListsKamikaze, 6 }, // Kamikaze
    {0x007d7850u, 0x00d06638u, 0x007d78e0u,
     kListsLevelBomber, 6 }, // LevelBomber
    {0x006d3110u, 0x00cf8c08u, 0x006d36d0u,
     kListsAirField, 5 }, // AirField
    {0x00848380u, 0x00d0b770u, 0x00846cf0u,
     kListsShipyard, 5 }, // Shipyard
    {0x0074df10u, 0x00cffde0u, 0x0074de10u,
     kListsLandVehicle, 5 }, // LandVehicle
    {0x00747000u, 0x00cff3f8u, 0x006f59b0u,
     kListsLandFort, 5 }, // LandFort
    {0x006f5c10u, 0x00cfb028u, 0x006f5a50u,
     kListsCommandBuilding, 6 }, // CommandBuilding
};

} // namespace

const UnitWorldRegistration* unit_world_registration_for_creator(
    std::uint32_t creator) noexcept {
    for (const UnitWorldRegistration& row : kRegistrations) {
        if (row.creator == creator) return &row;
    }
    return nullptr;
}

const UnitWorldRegistration* unit_world_registration_for_descriptor(
    const VehicleClassDescriptorRow* descriptor) noexcept {
    return descriptor != nullptr
        ? unit_world_registration_for_creator(descriptor->allocate_instance)
        : nullptr;
}

void register_parent_entity_list_00928560(UnitWorldRegistrationHost& host,
                                        void* unit) {
    void* parent = host.parent_0030(unit); // 00928561
    host.push_back_00484540(parent,
        static_cast<std::uint32_t>(kUnitParentListOffsets[0]), unit); // 00928567
}

bool register_unit_world_lists_for_creator(UnitWorldRegistrationHost& host,
                                          std::uint32_t creator, void* unit) {
    const UnitWorldRegistration* row = unit_world_registration_for_creator(creator);
    if (row == nullptr) return false;
    register_parent_entity_list_00928560(host, unit);
    for (std::size_t i = 1; i < row->list_count; ++i) {
        // Every native append reloads [ESI+30h], after the preceding call.
        // The list value is ESI itself, never a wrapper's +4h field.
        void* parent = host.parent_0030(unit);
        host.push_back_00484540(parent,
            static_cast<std::uint32_t>(row->list_offsets[i]), unit);
    }
    return true;
}

} // namespace bsp
