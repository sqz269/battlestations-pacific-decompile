#include "bsp/local_player_unit_lists.hpp"

// 004C3CB0, __thiscall void(GGame*), body 004C3CB0..004C409D.
// docs/LOCAL_PLAYER_UNIT_LISTS.md carries the evidence for every line here.

namespace bsp {

// Each row: the id, the name its descriptor's +0Ch getter returns (nullptr when
// the class has no name getter), the IsKindOf implementation the id was read
// from, and the descriptor vtable that holds it (0 when the id was read from an
// instance vtable or from a family whose vtable base was not established).
const UnitClassRow kUnitClassTable[41] = {
    {0x05, nullptr, 0x00749010, 0x00cff7cc},              // the vehicle root, [5,4]
    {0x06, nullptr, 0x009635e0, 0x00d1acc4},              // the ship base, [6,5,4]
    {0x07, "MDestroyer", 0x00963b70, 0x00d1acf8},
    {0x08, "MSubmarine", 0x00963e10, 0x00d1ae38},
    {0x09, "MMothership", 0x00963f10, 0x00d1aebc},
    {0x0a, "MCruiser", 0x00963bf0, 0x00d1ad38},
    {0x0b, "MCargo", 0x00963d00, 0x00d1adbc},
    {0x0c, "MLandingShip", 0x00963c80, 0x00d1ad78},
    {0x0d, "MBattleship", 0x00963d80, 0x00d1adf8},
    {0x0e, "MTorpedoBoat", 0x00963e90, 0x00d1ae78},
    {0x0f, nullptr, 0x007cfd00, 0x00d05eac},              // the plane base, [15,5,4]
    {0x10, "MPlaneBomber", 0x00953470, 0x00d1a4f8},
    {0x11, "MPlaneTorpedoBomber", 0x00953620, 0x00d19f4c},
    {0x12, "MPlaneDiveBomber", 0x00953590, 0x00d19c70},
    {0x13, "MPlaneFighter", 0x00953650, 0x00d19c30},
    {0x14, "MReconPlane", 0x009536b0, 0x00d19bf4},
    {0x15, "MSmallReconPlane", 0x00953740, 0x00d1a5a8},
    {0x16, "MLargeReconPlane", 0x009537d0, 0x00d1a5ec},
    {0x17, "MPlaneKamikaze", 0x00953500, 0x00d1a224},
    {0x18, nullptr, 0x007efb00, 0x00000000},              // the squadron, instance vtable 00D087C0
    {0x19, "MLandVehicle", 0x00960030, 0x00d1aa18},
    {0x1b, "MLandFort", 0x00749030, 0x00cff790},
    {0x1c, "MCommandBuilding", 0x00953680, 0x00d1a538},
    {0x21, "MRFSGun", 0x00442db0, 0x00ce45e0},
    {0x23, "MRTGun", 0x00442e50, 0x00ce4614},
    {0x24, "MSTGun", 0x00442ee0, 0x00ce4648},
    {0x25, "MBombPlatform", 0x00442c70, 0x00ce4560},
    {0x26, "MMultipleBombPlatform", 0x00442d00, 0x00ce459c},
    {0x27, "MDepthChargeLauncher", 0x00442fa0, 0x00ce467c},
    {0x28, "MCatapult", 0x00443010, 0x00ce46c0},
    {0x29, "MBullet", 0x006e8400, 0x00000000},            // name getter 006E8410
    {0x2a, "MBomb", 0x006ea2a0, 0x00000000},              // name getter 006EA290
    {0x2b, "MTorpedo", 0x006ea550, 0x00000000},
    {0x2c, "MDepthCharge", 0x006ea3e0, 0x00000000},
    {0x2e, "MDummyTarget", 0x006ea6f0, 0x00000000},       // the decoy family, not 35h
    {0x31, "MParatrooper", 0x006ea760, 0x00000000},
    {0x33, "MRocket", 0x006ea370, 0x00000000},
    {0x34, "MWaterMine", 0x006ea620, 0x00000000},
    {0x35, "MDummyTarget", 0x009600a0, 0x00d1aa58},       // the vehicle family
    {0x45, "MAirfield", 0x0095ff30, 0x00d1a9a0},
    {0x46, "MShipyard", 0x0095ffa0, 0x00d1a9dc},
};

const UnitClassRow* unit_class_row(int class_id) noexcept
{
    for (const UnitClassRow& row : kUnitClassTable) {
        if (row.class_id == class_id) {
            return &row;
        }
    }
    return nullptr;
}

std::size_t local_player_unit_list_offset(LocalPlayerUnitList list) noexcept
{
    const std::size_t index = static_cast<std::size_t>(list);
    return kUnitListHeadOffsets[index];
}

// 00484540: operator new(0Ch), node->value = arg, link at the tail, ++count.
// The native routine dereferences a null allocation (00484560 XOR ECX,ECX then
// 00484566 MOV [ECX+8],EAX); the model cannot fail, so that path has no
// counterpart here.
void unit_list_push_back_00484540(std::vector<UnitRef>& list, UnitRef unit)
{
    list.push_back(unit);
}

// 004BF8E0: while count != 0, unlink the head and free it. The back edge is
// 004BF922 JNZ 004BF8E8; it is a clear, not a pop.
void unit_list_clear_004bf8e0(std::vector<UnitRef>& list) noexcept
{
    list.clear();
}

// 004C2BE0: walk src from its head, push_back each value onto dst. src keeps
// its nodes.
void unit_list_append_all_004c2be0(std::vector<UnitRef>& dst,
                                   const std::vector<UnitRef>& src)
{
    for (UnitRef unit : src) {
        dst.push_back(unit);
    }
}

void clear_all_unit_lists_004bfdf0(LocalPlayerUnitLists& lists) noexcept
{
    for (std::vector<UnitRef>& list : lists) {
        unit_list_clear_004bf8e0(list);
    }
}

// 004C3D0C..004C3D1E: one set byte and three clear bytes.
bool unit_passes_list_filter(const UnitListFilterFlags& flags) noexcept
{
    return flags.active_5c && !flags.flag_5d && !flags.flag_60 && !flags.flag_5e;
}

namespace {

UnitWalkDecision one(LocalPlayerUnitList list)
{
    UnitWalkDecision decision;
    decision.append_first = true;
    decision.first = list;
    return decision;
}

} // namespace

// 004C3D0C..004C3D55.
UnitWalkDecision classify_walk0_unit(LocalPlayerUnitListsHost& host, UnitRef unit)
{
    UnitWalkDecision decision;
    if (!unit_passes_list_filter(host.unit_filter_flags(unit))) {
        return decision; // 004C3D0F, 004C3D14, 004C3D19, 004C3D1E
    }
    if (host.unit_is_kind_of(unit, kUnitClassOrdnanceBase)) {
        return decision; // 004C3D2D JNZ: ordnance is dropped
    }
    if (host.unit_is_kind_of(unit, kUnitClassShipBase)) {
        decision.append_first = true;              // 004C3D45, game+1964h
        decision.first = LocalPlayerUnitList::kWalk0Ships;
    }
    decision.append_second = true;                 // 004C3D51, game+1970h
    decision.second = LocalPlayerUnitList::kWalk0Rest;
    return decision;
}

// 004C3D76..004C3EA0. The tests are in listing order and each is reached only
// when every earlier one failed.
UnitWalkDecision classify_walk1_unit(LocalPlayerUnitListsHost& host, UnitRef unit)
{
    UnitWalkDecision decision;
    if (!unit_passes_list_filter(host.unit_filter_flags(unit))) {
        return decision; // 004C3D79..004C3D94
    }
    if (host.unit_is_kind_of(unit, kUnitClassShipBase)) {
        return one(LocalPlayerUnitList::kShips);       // 004C3DA7
    }
    if (host.unit_is_kind_of(unit, kUnitClassPlaneSquadron)) {
        return one(LocalPlayerUnitList::kSquadrons);   // 004C3E09
    }
    if (host.unit_is_kind_of(unit, kUnitClassPlaneBase)) {
        return decision;                               // 004C3E23: planes are dropped
    }
    if (host.unit_is_kind_of(unit, kUnitClassAirfield)) {
        return one(LocalPlayerUnitList::kAirfields);   // 004C3E36
    }
    if (host.unit_is_kind_of(unit, kUnitClassShipyard)) {
        return one(LocalPlayerUnitList::kShipyards);   // 004C3E4D
    }
    if (host.unit_is_kind_of(unit, kUnitClassLandFort)) {
        return one(LocalPlayerUnitList::kLandForts);   // 004C3E64
    }
    if (host.unit_is_kind_of(unit, kUnitClassDummyTargetVehicle)) {
        return one(LocalPlayerUnitList::kMerged);      // 004C3E7B, game+19B8h
    }
    if (host.unit_in_local_objective_set_008ddf90(unit)) {
        return one(LocalPlayerUnitList::kMerged);      // 004C3E97
    }
    return decision;
}

// 004C3EC8..004C403C: the same chain without 0Fh, 35h and the 008DDF90 call.
UnitWalkDecision classify_walk2_unit(LocalPlayerUnitListsHost& host, UnitRef unit)
{
    UnitWalkDecision decision;
    if (!unit_passes_list_filter(host.unit_filter_flags(unit))) {
        return decision; // 004C3ECB..004C3EE6
    }
    if (host.unit_is_kind_of(unit, kUnitClassShipBase)) {
        return one(LocalPlayerUnitList::kShips);       // 004C3EF9
    }
    if (host.unit_is_kind_of(unit, kUnitClassPlaneSquadron)) {
        return one(LocalPlayerUnitList::kSquadrons);   // 004C3F5B
    }
    if (host.unit_is_kind_of(unit, kUnitClassAirfield)) {
        return one(LocalPlayerUnitList::kAirfields);   // 004C3FBD
    }
    if (host.unit_is_kind_of(unit, kUnitClassShipyard)) {
        return one(LocalPlayerUnitList::kShipyards);   // 004C401C
    }
    if (host.unit_is_kind_of(unit, kUnitClassLandFort)) {
        return one(LocalPlayerUnitList::kLandForts);   // 004C4033
    }
    return decision;
}

namespace {

void apply(LocalPlayerUnitListsHost& host, const UnitWalkDecision& decision, UnitRef unit)
{
    if (decision.append_first) {
        host.append_00484540(decision.first, unit);
    }
    if (decision.append_second) {
        host.append_00484540(decision.second, unit);
    }
}

} // namespace

bool build_local_player_unit_lists_004c3cb0(UnitListsGate& gate,
                                            LocalPlayerUnitListsHost& host)
{
    if (!local_player_unit_lists_run_004c3cb0(gate)) {
        return false; // 004C3CBD, 004C3CCB, 004C3CD4
    }

    host.clear_all_lists_004bfdf0(); // 004C3CE3

    for (UnitRef unit : host.registry_walk_units(UnitRegistryWalk::kWalk0)) {
        apply(host, classify_walk0_unit(host, unit), unit);
    }
    for (UnitRef unit : host.registry_walk_units(UnitRegistryWalk::kWalk1)) {
        apply(host, classify_walk1_unit(host, unit), unit);
    }
    for (UnitRef unit : host.registry_walk_units(UnitRegistryWalk::kWalk2)) {
        apply(host, classify_walk2_unit(host, unit), unit);
    }

    for (LocalPlayerUnitList source : kUnitListMergeSources) {
        host.append_all_004c2be0(LocalPlayerUnitList::kMerged, source); // 004C405B..004C4093
    }
    return true;
}

// 008DDF90: empty set, then vehicle or squadron, then the set lookup.
bool objective_set_contains_008ddf90(const ObjectiveSetQuery& query) noexcept
{
    if (query.set_size == 0) {
        return false; // 008DDF93
    }
    if (!query.unit_is_vehicle && !query.unit_is_squadron) {
        return false; // 008DDFC0 falls through to 008DDFD5
    }
    return query.subject_in_set; // 008DDFCB -> 008DDF00
}

} // namespace bsp
