// docs/ENTITY_CLASS_IDS.md. The table is decoded data, not a projection of a
// native routine: every row's class test address is a body in .text that Ghidra
// has no function for, read with `bsp.py ghidra bytes` and decoded linearly.
#include "bsp/entity_class_ids.hpp"

namespace bsp {

// id, parent, recovered name. The trailing address is the class test, the
// vtable slot 5Ch implementation whose compiled compare chain supplied the
// ancestor set this row's parent edge was derived from.
const EntityClassRow kEntityClassTable[kEntityClassIdCount] = {
    {0x00, kEntityClassNoParent, nullptr},  // 0042B8F0
    {0x01, 0x00, nullptr},  // 0047F190
    {0x02, 0x01, nullptr},  // 004F1750
    {0x03, 0x01, nullptr},  // 00888EA0
    {0x04, 0x02, nullptr},  // 006D1610
    {0x05, 0x04, nullptr},  // 006D1650
    {0x06, 0x05, nullptr},  // 006DFE50
    {0x07, 0x06, "MDestroyer"},  // 006FE530
    {0x08, 0x06, "MSubmarine"},  // 00853050
    {0x09, 0x06, "MMothership"},  // 00758510
    {0x0A, 0x06, "MCruiser"},  // 006FB3D0
    {0x0B, 0x06, "MCargo"},  // 006EB230
    {0x0C, 0x06, "MLandingShip"},  // 0074BC60
    {0x0D, 0x06, "MBattleship"},  // 006DFE90
    {0x0E, 0x06, "MTorpedoBoat"},  // 00857DC0
    {0x0F, 0x05, nullptr},  // 0074E400
    {0x10, 0x0F, "MPlaneBomber"},  // 007D77F0
    {0x11, 0x0F, "MPlaneTorpedoBomber"},  // 009535C0
    {0x12, 0x0F, "MPlaneDiveBomber"},  // 00953530
    {0x13, 0x0F, "MPlaneFighter"},  // 007DDA80
    {0x14, 0x0F, "MReconPlane"},  // 0074E480
    {0x15, 0x14, "MSmallReconPlane"},  // 0084C9F0
    {0x16, 0x14, "MLargeReconPlane"},  // 0074E4E0
    {0x17, 0x0F, "MPlaneKamikaze"},  // 009534A0
    {0x18, 0x02, "PlaneSquadronGen"},  // 007EFB00
    {0x19, 0x05, "MLandVehicle"},  // 0074DD90
    {0x1A, 0x02, "LandConvoy"},  // 004F2560
    {0x1B, 0x05, "MLandFort"},  // 006F5890
    {0x1C, 0x1B, "MCommandBuilding"},  // 006F58E0
    {0x1D, 0x01, "LandingPoint"},  // 004E9620
    {0x1E, 0x04, nullptr},  // 006E3D10
    {0x1F, kEntityClassNoParent, nullptr},  // no class test in the image
    {0x20, 0x1E, nullptr},  // 006E3D50
    {0x21, 0x20, "MRFSGun"},  // 00730BD0
    {0x22, 0x20, nullptr},  // 006FDE40
    {0x23, 0x22, "MRTGun"},  // 00730ED0
    {0x24, 0x22, "MSTGun"},  // 006FDF20
    {0x25, 0x20, "MBombPlatform"},  // 006E3E10
    {0x26, 0x25, "MMultipleBombPlatform"},  // 006E43B0
    {0x27, 0x24, "MDepthChargeLauncher"},  // 006FE050
    {0x28, 0x20, "MCatapult"},  // 006EC7B0
    {0x29, 0x00, "MBullet"},  // 006E7C00
    {0x2A, 0x02, "MBomb"},  // 006E2790
    {0x2B, 0x2A, "MTorpedo"},  // 00856260
    {0x2C, 0x2A, "MDepthCharge"},  // 006FCA80
    {0x2D, 0x2A, nullptr},  // 006FE9F0
    {0x2E, 0x2D, "MDummyTarget"},  // 00700AC0
    {0x2F, 0x2D, "MDummyKamikazePlane"},  // 006FEA30
    {0x30, 0x2D, "MDummySubmarine"},  // 006FF8E0
    {0x31, 0x2A, "MParatrooper"},  // 007ABA50
    {0x32, 0x29, "MFlakBullet"},  // 0070CB80
    {0x33, 0x2A, "MRocket"},  // 0080ACB0
    {0x34, 0x2A, "MWaterMine"},  // 0085EB20
    {0x35, 0x05, nullptr},  // 007004B0
    {0x36, 0x00, "Stationary"},  // 00748B40
    {0x37, 0x00, nullptr},  // 00470BE0
    {0x38, 0x37, nullptr},  // 00479F60
    {0x39, 0x37, nullptr},  // 00472870
    {0x3A, 0x37, nullptr},  // 004AF840
    {0x3B, 0x37, "Wreck"},  // 004B1F40
    {0x3C, 0x37, nullptr},  // 004740E0
    {0x3D, 0x37, "Cloud"},  // 00476310
    {0x3E, 0x37, nullptr},  // 00470C10
    {0x3F, 0x37, nullptr},  // 004A7BC0
    {0x40, 0x37, nullptr},  // 004AC8E0
    {0x41, 0x01, "NavPoint"},  // 004E6970
    {0x42, 0x01, "MovieCamPos"},  // 004E69C0
    {0x43, 0x01, "MovieCamLookat"},  // 004E6A10
    {0x44, 0x01, "Landscape"},  // 004F1360
    {0x45, 0x05, "MAirfield"},  // 006D20E0
    {0x46, 0x05, "MShipyard"},  // 00846C00
    {0x47, 0x01, "Path"},  // 00480930
    {0x48, 0x00, nullptr},  // 0080F9A0
    {0x49, 0x02, nullptr},  // 007B3320
    {0x4A, 0x01, "CameraPath"},  // 004E6920
    {0x4B, 0x00, nullptr},  // 00A31C60
    {0x4C, 0x00, nullptr},  // 0042C010
    {0x4D, 0x02, "SpawnPoint"},  // 004F1800
    {0x4E, 0x4C, nullptr},  // 004351E0
    {0x4F, 0x56, nullptr},  // 006508B0
    {0x50, 0x56, nullptr},  // 0064B720
    {0x51, 0x4C, nullptr},  // 006051E0
    {0x52, 0x4C, nullptr},  // 0078FAC0
    {0x53, 0x4C, nullptr},  // 005177D0
    {0x54, 0x4C, nullptr},  // 0079A380
    {0x55, 0x4C, nullptr},  // 0079D840
    {0x56, 0x4C, nullptr},  // 00519380
    {0x57, kEntityClassNoParent, nullptr},  // no class test in the image
    {0x58, kEntityClassNoParent, nullptr},  // no class test in the image
    {0x59, kEntityClassNoParent, nullptr},  // no class test in the image
    {0x5A, kEntityClassNoParent, nullptr},  // no class test in the image
    {0x5B, 0x01, "SimpleEffect"},  // no class test: vtable 00CE8BD0 slot 5Ch is 0047F190, class 01's
    {0x5C, 0x01, "PeriodicEffect"},  // no class test: vtable 00CE8D68 slot 5Ch is 0047F190
    {0x5D, kEntityClassNoParent, "FreeCamPos"},  // scene table only; no vtable read
    {0x5E, kEntityClassNoParent, nullptr},  // no class test in the image
    {0x5F, kEntityClassNoParent, nullptr},  // no class test in the image
    {0x60, 0x02, nullptr},  // 007810F0
};

const EntityClassRow* entity_class_row(int id) noexcept
{
    if (id < 0 || id > kEntityClassIdMax) {
        return nullptr;
    }
    return &kEntityClassTable[static_cast<std::size_t>(id)];
}

const char* entity_class_name(int id) noexcept
{
    const EntityClassRow* row = entity_class_row(id);
    return row ? row->name : nullptr;
}

int entity_class_parent(int id) noexcept
{
    const EntityClassRow* row = entity_class_row(id);
    return row ? row->parent : kEntityClassNoParent;
}

int entity_class_depth(int id) noexcept
{
    if (id == kEntityClassIdRoot) {
        return 0;
    }
    const EntityClassRow* row = entity_class_row(id);
    if (row == nullptr || row->parent == kEntityClassNoParent) {
        return -1;
    }
    int depth = 0;
    int walk = id;
    // The table is acyclic by construction; the bound keeps a hand-edited row
    // from spinning.
    for (std::size_t step = 0; step < kEntityClassIdCount; ++step) {
        const EntityClassRow* here = entity_class_row(walk);
        if (here == nullptr) {
            return -1;
        }
        if (here->id == kEntityClassIdRoot) {
            return depth;
        }
        if (here->parent == kEntityClassNoParent) {
            return -1;
        }
        walk = here->parent;
        ++depth;
    }
    return -1;
}

namespace {

// Walk from `id` to the root, reporting whether `query` is on the chain.
// 004B1F40 and every other class test compile exactly this set as a run of
// CMP EAX,imm, most derived to root, with the root's compare emitted as
// TEST EAX,EAX.
bool chain_contains(int id, int query) noexcept
{
    int walk = id;
    for (std::size_t step = 0; step < kEntityClassIdCount; ++step) {
        const EntityClassRow* here = entity_class_row(walk);
        if (here == nullptr) {
            return false;
        }
        if (here->id == query) {
            return true;
        }
        if (here->parent == kEntityClassNoParent) {
            return false;
        }
        walk = here->parent;
    }
    return false;
}

}  // namespace

bool entity_is_kind_of(int dynamic_id, int query) noexcept
{
    if (query == kEntityClassIdRoot) {
        return entity_class_row(dynamic_id) != nullptr;
    }
    return chain_contains(dynamic_id, query);
}

bool entity_is_kind_of_inherited(int owner_id, int dynamic_id, int query) noexcept
{
    if (query == dynamic_id && entity_class_row(dynamic_id) != nullptr) {
        return true;
    }
    return entity_is_kind_of(owner_id, query);
}

int entity_class_id_for_bucket(int bucket) noexcept
{
    const int id = bucket + kEntityKindBucketBias;
    return (id < 0 || id > kEntityClassIdMax) ? kEntityClassNoParent : id;
}

int entity_bucket_for_class_id(int id) noexcept
{
    return (id < 0 || id > kEntityClassIdMax) ? kEntityClassNoParent
                                              : id - kEntityKindBucketBias;
}

}  // namespace bsp
