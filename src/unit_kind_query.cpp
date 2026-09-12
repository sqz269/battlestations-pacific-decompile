// The entity kind predicate at vtable slot 5Ch, as data: the accepted-literal
// set decoded from each of the 88 compiled class-test bodies, and the
// catalogue of query literals the engine asks with.
//
// docs/UNIT_KIND_QUERY.md. Every accept list below was decoded from the body
// at `test_address` in the shipped image's .text: the run of `CMP EAX,imm`
// (root compare emitted as `TEST EAX,EAX`) that precedes the single
// `CMP EAX,[ECX+0C4h]`. Lists are in ascending order, not the compiled compare
// order, which varies (006D20E0 puts its own id last).
#include "bsp/unit_kind_query.hpp"

namespace bsp {
namespace {

constexpr int kAccept00[] = {0x00};
constexpr int kAccept01[] = {0x00, 0x01};
constexpr int kAccept02[] = {0x00, 0x01, 0x02};
constexpr int kAccept03[] = {0x00, 0x01, 0x03};
constexpr int kAccept04[] = {0x00, 0x01, 0x02, 0x04};
constexpr int kAccept05[] = {0x00, 0x01, 0x02, 0x04, 0x05};
constexpr int kAccept06[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06};
constexpr int kAccept07[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x07};
constexpr int kAccept08[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x08};
constexpr int kAccept09[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x09};
constexpr int kAccept0A[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x0A};
constexpr int kAccept0B[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x0B};
constexpr int kAccept0C[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x0C};
constexpr int kAccept0D[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x0D};
constexpr int kAccept0E[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x0E};
constexpr int kAccept0F[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x0F};
constexpr int kAccept10[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x0F, 0x10};
constexpr int kAccept11[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x0F, 0x11};
constexpr int kAccept12[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x0F, 0x12};
constexpr int kAccept13[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x0F, 0x13};
constexpr int kAccept14[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x0F, 0x14};
constexpr int kAccept15[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x0F, 0x14, 0x15};
constexpr int kAccept16[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x0F, 0x14, 0x16};
constexpr int kAccept17[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x0F, 0x17};
constexpr int kAccept18[] = {0x00, 0x01, 0x02, 0x18};
constexpr int kAccept19[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x19};
constexpr int kAccept1A[] = {0x00, 0x01, 0x02, 0x1A};
constexpr int kAccept1B[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x1B};
constexpr int kAccept1C[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x1B, 0x1C};
constexpr int kAccept1D[] = {0x00, 0x01, 0x1D};
constexpr int kAccept1E[] = {0x00, 0x01, 0x02, 0x04, 0x1E};
constexpr int kAccept20[] = {0x00, 0x01, 0x02, 0x04, 0x1E, 0x20};
constexpr int kAccept21[] = {0x00, 0x01, 0x02, 0x04, 0x1E, 0x20, 0x21};
constexpr int kAccept22[] = {0x00, 0x01, 0x02, 0x04, 0x1E, 0x20, 0x22};
constexpr int kAccept23[] = {0x00, 0x01, 0x02, 0x04, 0x1E, 0x20, 0x22, 0x23};
constexpr int kAccept24[] = {0x00, 0x01, 0x02, 0x04, 0x1E, 0x20, 0x22, 0x24};
constexpr int kAccept25[] = {0x00, 0x01, 0x02, 0x04, 0x1E, 0x20, 0x25};
constexpr int kAccept26[] = {0x00, 0x01, 0x02, 0x04, 0x1E, 0x20, 0x25, 0x26};
constexpr int kAccept27[] = {0x00, 0x01, 0x02, 0x04, 0x1E, 0x20, 0x22, 0x24, 0x27};
constexpr int kAccept28[] = {0x00, 0x01, 0x02, 0x04, 0x1E, 0x20, 0x28};
constexpr int kAccept29[] = {0x00, 0x29};
constexpr int kAccept2A[] = {0x00, 0x01, 0x02, 0x2A};
constexpr int kAccept2B[] = {0x00, 0x01, 0x02, 0x2A, 0x2B};
constexpr int kAccept2C[] = {0x00, 0x01, 0x02, 0x2A, 0x2C};
constexpr int kAccept2D[] = {0x00, 0x01, 0x02, 0x2A, 0x2D};
constexpr int kAccept2E[] = {0x00, 0x01, 0x02, 0x2A, 0x2D, 0x2E};
constexpr int kAccept2F[] = {0x00, 0x01, 0x02, 0x2A, 0x2D, 0x2F};
constexpr int kAccept30[] = {0x00, 0x01, 0x02, 0x2A, 0x2D, 0x30};
constexpr int kAccept31[] = {0x00, 0x01, 0x02, 0x2A, 0x31};
constexpr int kAccept32[] = {0x00, 0x29, 0x32};
constexpr int kAccept33[] = {0x00, 0x01, 0x02, 0x2A, 0x33};
constexpr int kAccept34[] = {0x00, 0x01, 0x02, 0x2A, 0x34};
constexpr int kAccept35[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x35};
constexpr int kAccept36[] = {0x00, 0x36};
constexpr int kAccept37[] = {0x00, 0x37};
constexpr int kAccept38[] = {0x00, 0x37, 0x38};
constexpr int kAccept39[] = {0x00, 0x37, 0x39};
constexpr int kAccept3A[] = {0x00, 0x37, 0x3A};
constexpr int kAccept3B[] = {0x00, 0x37, 0x3B};
constexpr int kAccept3C[] = {0x00, 0x37, 0x3C};
constexpr int kAccept3D[] = {0x00, 0x37, 0x3D};
constexpr int kAccept3E[] = {0x00, 0x37, 0x3E};
constexpr int kAccept3F[] = {0x00, 0x37, 0x3F};
constexpr int kAccept40[] = {0x00, 0x37, 0x40};
constexpr int kAccept41[] = {0x00, 0x01, 0x41};
constexpr int kAccept42[] = {0x00, 0x01, 0x42};
constexpr int kAccept43[] = {0x00, 0x01, 0x43};
constexpr int kAccept44[] = {0x00, 0x01, 0x44};
constexpr int kAccept45[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x45};
constexpr int kAccept46[] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x46};
constexpr int kAccept47[] = {0x00, 0x01, 0x47};
constexpr int kAccept48[] = {0x00, 0x48};
constexpr int kAccept49[] = {0x00, 0x01, 0x02, 0x49};
constexpr int kAccept4A[] = {0x00, 0x01, 0x4A};
constexpr int kAccept4B[] = {0x00, 0x4B};
constexpr int kAccept4C[] = {0x00, 0x4C};
constexpr int kAccept4D[] = {0x00, 0x01, 0x02, 0x4D};
constexpr int kAccept4E[] = {0x00, 0x4C, 0x4E};
constexpr int kAccept4EB[] = {0x00, 0x4C, 0x4E};
constexpr int kAccept4F[] = {0x00, 0x4C, 0x4F, 0x56};
constexpr int kAccept50[] = {0x00, 0x4C, 0x50, 0x56};
constexpr int kAccept51[] = {0x00, 0x4C, 0x51};
constexpr int kAccept52[] = {0x00, 0x4C, 0x52};
constexpr int kAccept53[] = {0x00, 0x4C, 0x53};
constexpr int kAccept54[] = {0x00, 0x4C, 0x54};
constexpr int kAccept55[] = {0x00, 0x4C, 0x55};
constexpr int kAccept56[] = {0x00, 0x4C, 0x56};
constexpr int kAccept60[] = {0x00, 0x01, 0x02, 0x60};

}  // namespace

const UnitKindClassBody kUnitKindClassBodies[] = {
    {0x00, 0x0042b8f0u, 0x00d19120u, nullptr, kAccept00, 1},
    {0x01, 0x0047f190u, 0x00ce8bd0u, nullptr, kAccept01, 2},
    {0x02, 0x004f1750u, 0x00d03e80u, nullptr, kAccept02, 3},
    {0x03, 0x00888ea0u, 0x00d11138u, nullptr, kAccept03, 3},
    {0x04, 0x006d1610u, 0x00d0df70u, nullptr, kAccept04, 4},
    {0x05, 0x006d1650u, 0x00d1a698u, nullptr, kAccept05, 5},
    {0x06, 0x006dfe50u, 0x00d09678u, nullptr, kAccept06, 6},
    {0x07, 0x006fe530u, 0x00cfc3d0u, "MDestroyer", kAccept07, 7},
    {0x08, 0x00853050u, 0x00d0bf80u, "MSubmarine", kAccept08, 7},
    {0x09, 0x00758510u, 0x00d01630u, "MMothership", kAccept09, 7},
    {0x0A, 0x006fb3d0u, 0x00cfb738u, "MCruiser", kAccept0A, 7},
    {0x0B, 0x006eb230u, 0x00cfa778u, "MCargo", kAccept0B, 7},
    {0x0C, 0x0074bc60u, 0x00cffa30u, "MLandingShip", kAccept0C, 7},
    {0x0D, 0x006dfe90u, 0x00cf90b0u, "MBattleship", kAccept0D, 7},
    {0x0E, 0x00857dc0u, 0x00d0c648u, "MTorpedoBoat", kAccept0E, 7},
    {0x0F, 0x0074e400u, 0x00d05f20u, nullptr, kAccept0F, 6},
    {0x10, 0x007d77f0u, 0x00d06638u, "MPlaneBomber", kAccept10, 7},
    {0x11, 0x009535c0u, 0x00d1a000u, "MPlaneTorpedoBomber", kAccept11, 7},
    {0x12, 0x00953530u, 0x00d19d28u, "MPlaneDiveBomber", kAccept12, 7},
    {0x13, 0x007dda80u, 0x00d06920u, "MPlaneFighter", kAccept13, 7},
    {0x14, 0x0074e480u, 0x00d00070u, "MReconPlane", kAccept14, 7},
    {0x15, 0x0084c9f0u, 0x00d0ba80u, "MSmallReconPlane", kAccept15, 8},
    {0x16, 0x0074e4e0u, 0x00d00308u, "MLargeReconPlane", kAccept16, 8},
    {0x17, 0x009534a0u, 0x00d1a2d8u, "MPlaneKamikaze", kAccept17, 7},
    {0x18, 0x007efb00u, 0x00d087c0u, "PlaneSquadronGen", kAccept18, 4},
    {0x19, 0x0074dd90u, 0x00cffde0u, "MLandVehicle", kAccept19, 6},
    {0x1A, 0x004f2560u, 0x00cea570u, "LandConvoy", kAccept1A, 4},
    {0x1B, 0x006f5890u, 0x00cff3f8u, "MLandFort", kAccept1B, 6},
    {0x1C, 0x006f58e0u, 0x00cfb028u, "MCommandBuilding", kAccept1C, 7},
    {0x1D, 0x004e9620u, 0x00ce90e0u, "LandingPoint", kAccept1D, 3},
    {0x1E, 0x006e3d10u, 0x00cfdc58u, nullptr, kAccept1E, 5},
    {0x20, 0x006e3d50u, 0x00cfe0a8u, nullptr, kAccept20, 6},
    {0x21, 0x00730bd0u, 0x00cfe308u, "MRFSGun", kAccept21, 7},
    {0x22, 0x006fde40u, 0x00cfbd20u, nullptr, kAccept22, 7},
    {0x23, 0x00730ed0u, 0x00cfe548u, "MRTGun", kAccept23, 8},
    {0x24, 0x006fdf20u, 0x00cfbf58u, "MSTGun", kAccept24, 8},
    {0x25, 0x006e3e10u, 0x00cf96a8u, "MBombPlatform", kAccept25, 7},
    {0x26, 0x006e43b0u, 0x00cf9918u, "MMultipleBombPlatform", kAccept26, 8},
    {0x27, 0x006fe050u, 0x00cfc190u, "MDepthChargeLauncher", kAccept27, 9},
    {0x28, 0x006ec7b0u, 0x00cfaab8u, "MCatapult", kAccept28, 7},
    {0x29, 0x006e7c00u, 0x00cf9df0u, "MBullet", kAccept29, 2},
    {0x2A, 0x006e2790u, 0x00cf9438u, "MBomb", kAccept2A, 4},
    {0x2B, 0x00856260u, 0x00d0c3e8u, "MTorpedo", kAccept2B, 5},
    {0x2C, 0x006fca80u, 0x00cfba80u, "MDepthCharge", kAccept2C, 5},
    {0x2D, 0x006fe9f0u, 0x00cfc910u, nullptr, kAccept2D, 5},
    {0x2E, 0x00700ac0u, 0x00cfd018u, "MDummyTarget", kAccept2E, 6},
    {0x2F, 0x006fea30u, 0x00cfc698u, "MDummyKamikazePlane", kAccept2F, 6},
    {0x30, 0x006ff8e0u, 0x00cfcb48u, "MDummySubmarine", kAccept30, 6},
    {0x31, 0x007aba50u, 0x00d05060u, "MParatrooper", kAccept31, 5},
    {0x32, 0x0070cb80u, 0x00cfd5d0u, "MFlakBullet", kAccept32, 3},
    {0x33, 0x0080acb0u, 0x00d090e8u, "MRocket", kAccept33, 5},
    {0x34, 0x0085eb20u, 0x00d0d130u, "MWaterMine", kAccept34, 5},
    {0x35, 0x007004b0u, 0x00cfcd60u, nullptr, kAccept35, 6},
    {0x36, 0x00748b40u, 0x00cff678u, "Stationary", kAccept36, 2},
    {0x37, 0x00470be0u, 0x00ce6490u, nullptr, kAccept37, 2},
    {0x38, 0x00479f60u, 0x00ce6130u, nullptr, kAccept38, 3},
    {0x39, 0x00472870u, 0x00ce5b90u, nullptr, kAccept39, 3},
    {0x3A, 0x004af840u, 0x00ce7008u, nullptr, kAccept3A, 3},
    {0x3B, 0x004b1f40u, 0x00ce71a8u, "Wreck", kAccept3B, 3},
    {0x3C, 0x004740e0u, 0x00ce5cf0u, nullptr, kAccept3C, 3},
    {0x3D, 0x00476310u, 0x00ce5e68u, "Cloud", kAccept3D, 3},
    {0x3E, 0x00470c10u, 0x00ce5a20u, nullptr, kAccept3E, 3},
    {0x3F, 0x004a7bc0u, 0x00ce6ab0u, nullptr, kAccept3F, 3},
    {0x40, 0x004ac8e0u, 0x00ce6e10u, nullptr, kAccept40, 3},
    {0x41, 0x004e6970u, 0x00ce8550u, "NavPoint", kAccept41, 3},
    {0x42, 0x004e69c0u, 0x00ce86d8u, "MovieCamPos", kAccept42, 3},
    {0x43, 0x004e6a10u, 0x00ce8860u, "MovieCamLookat", kAccept43, 3},
    {0x44, 0x004f1360u, 0x00cea090u, "Landscape", kAccept44, 3},
    {0x45, 0x006d20e0u, 0x00cf8c08u, "MAirfield", kAccept45, 6},
    {0x46, 0x00846c00u, 0x00d0b770u, "MShipyard", kAccept46, 6},
    {0x47, 0x00480930u, 0x00ce6290u, "Path", kAccept47, 3},
    {0x48, 0x0080f9a0u, 0x00d092b8u, nullptr, kAccept48, 2},
    {0x49, 0x007b3320u, 0x00d054d0u, nullptr, kAccept49, 4},
    {0x4A, 0x004e6920u, 0x00ce8390u, "CameraPath", kAccept4A, 3},
    {0x4B, 0x00a31c60u, 0x00d231a0u, nullptr, kAccept4B, 2},
    {0x4C, 0x0042c010u, 0x00ce3b28u, nullptr, kAccept4C, 2},
    {0x4D, 0x004f1800u, 0x00cea218u, "SpawnPoint", kAccept4D, 4},
    {0x4E, 0x004351e0u, 0x00ce3e60u, nullptr, kAccept4E, 3},
    {0x4E, 0x00435360u, 0x00ce3fd0u, nullptr, kAccept4EB, 3},
    {0x4F, 0x006508b0u, 0x00cf6250u, nullptr, kAccept4F, 4},
    {0x50, 0x0064b720u, 0x00cf5ce8u, nullptr, kAccept50, 4},
    {0x51, 0x006051e0u, 0x00cf4238u, nullptr, kAccept51, 3},
    {0x52, 0x0078fac0u, 0x00d044b8u, nullptr, kAccept52, 3},
    {0x53, 0x005177d0u, 0x00cec1f8u, nullptr, kAccept53, 3},
    {0x54, 0x0079a380u, 0x00d04750u, nullptr, kAccept54, 3},
    {0x55, 0x0079d840u, 0x00d04b58u, nullptr, kAccept55, 3},
    {0x56, 0x00519380u, 0x00cec440u, nullptr, kAccept56, 3},
    {0x60, 0x007810f0u, 0x00d040b8u, nullptr, kAccept60, 4},
};

const std::size_t kUnitKindClassBodyCount =
    sizeof(kUnitKindClassBodies) / sizeof(kUnitKindClassBodies[0]);

const UnitKindLiteralRow kUnitKindLiterals[] = {
    {0x00, 13, 87, "the root: every entity"},
    {0x01, 23, 61, "every entity except the effect/wreck branch under 37h and the 4Ch tool branch"},
    {0x02, 69, 52, "the commandable/simulated family: units, weapon devices, ordnance, squadrons, convoys"},
    {0x04, 32, 36, "a unit or a weapon device"},
    {0x05, 107, 25, "a unit: every ship, plane, land vehicle, fort, command building, airfield and shipyard"},
    {0x06, 200, 9, "a ship"},
    {0x07, 6, 1, "a destroyer"},
    {0x08, 110, 1, "a submarine; selects hull-body mode 2 at 00937CFD"},
    {0x09, 80, 1, "a carrier (MMothership); the carrier arm of the air-operations split"},
    {0x0A, 8, 1, "a cruiser"},
    {0x0B, 9, 1, "a cargo ship"},
    {0x0C, 21, 1, "a landing ship"},
    {0x0D, 9, 1, "a battleship"},
    {0x0E, 22, 1, "a torpedo boat"},
    {0x0F, 161, 9, "a plane"},
    {0x10, 55, 1, "a level bomber; the levelbomb/divebomb discriminator"},
    {0x11, 1, 1, "a torpedo bomber"},
    {0x12, 2, 1, "a dive bomber"},
    {0x13, 7, 1, "a fighter"},
    {0x14, 4, 3, "a recon plane, either size"},
    {0x16, 41, 1, "a large recon plane"},
    {0x17, 32, 1, "a kamikaze plane"},
    {0x18, 171, 1, "a plane squadron generator"},
    {0x19, 8, 1, "a land vehicle"},
    {0x1A, 17, 1, "a land convoy"},
    {0x1B, 47, 2, "a land structure: fort or command building"},
    {0x1C, 66, 1, "a command building"},
    {0x1E, 13, 10, "the weapon-device family root"},
    {0x1F, 3, 0, "nothing: no class test carries 1Fh and no constructor stamps it"},
    {0x20, 55, 9, "a gun or weapon platform"},
    {0x21, 4, 1, "a fixed gun (MRFSGun)"},
    {0x22, 10, 4, "a turning gun: RT, ST and the depth-charge launcher"},
    {0x23, 3, 1, "a rotating turret gun (MRTGun)"},
    {0x24, 13, 2, "an ST gun or a depth-charge launcher"},
    {0x25, 24, 2, "a bomb platform, single or multiple"},
    {0x26, 3, 1, "a multiple bomb platform"},
    {0x28, 4, 1, "a catapult"},
    {0x29, 8, 2, "a bullet, normal or flak"},
    {0x2A, 19, 10, "ordnance: the MBomb family, including torpedoes, rockets, mines and paratroopers"},
    {0x2B, 12, 1, "a torpedo"},
    {0x2C, 3, 1, "a depth charge"},
    {0x31, 4, 1, "a paratrooper"},
    {0x32, 1, 1, "a flak bullet"},
    {0x33, 5, 1, "a rocket"},
    {0x34, 5, 1, "a water mine"},
    {0x35, 3, 1, "an unnamed unit-family class, asked beside 06h in the surface-target test"},
    {0x36, 9, 1, "a Stationary scene object"},
    {0x3A, 4, 1, "an unnamed member of the 37h effect family"},
    {0x3D, 1, 1, "a cloud"},
    {0x41, 5, 1, "a nav point"},
    {0x44, 18, 1, "the landscape"},
    {0x45, 103, 1, "an airfield"},
    {0x46, 60, 1, "a shipyard"},
    {0x47, 6, 1, "a Path"},
    {0x48, 1, 1, "an unnamed root-derived class asked beside Path"},
    {0x49, 2, 1, "an unnamed 02h-derived class asked beside Path"},
    {0x4A, 3, 1, "a CameraPath"},
    {0x4E, 1, 1, "an unnamed 4Ch-family class"},
    {0x4F, 2, 1, "an unnamed 56h-derived class"},
    {0x50, 2, 1, "an unnamed 56h-derived class"},
    {0x51, 2, 1, "an unnamed 4Ch-derived class"},
    {0x54, 6, 1, "an unnamed 4Ch-derived class"},
    {0x55, 1, 1, "an unnamed 4Ch-derived class"},
};

const std::size_t kUnitKindLiteralCount =
    sizeof(kUnitKindLiterals) / sizeof(kUnitKindLiterals[0]);

const UnitKindClassBody* unit_kind_body_for_class(int class_id) noexcept {
    for (std::size_t i = 0; i < kUnitKindClassBodyCount; ++i) {
        if (kUnitKindClassBodies[i].class_id == class_id) {
            return &kUnitKindClassBodies[i];
        }
    }
    return nullptr;
}

bool unit_is_kind_of(int class_id, int literal) noexcept {
    const UnitKindClassBody* body = unit_kind_body_for_class(class_id);
    if (body == nullptr) {
        return false;
    }
    for (std::size_t i = 0; i < body->accept_count; ++i) {
        if (body->accepts[i] == literal) {
            return true;
        }
    }
    return false;
}

bool unit_kind_body_answers(int owner_class_id, int dynamic_class_id, int literal) noexcept {
    // The compiled body compares the query against the owning class's chain
    // first and then against the object's own id at +C4h, so an object of a
    // class that does not override slot 5Ch is still recognised by identity.
    if (unit_is_kind_of(owner_class_id, literal)) {
        return true;
    }
    return literal == dynamic_class_id;
}

std::size_t unit_kind_member_count(int literal) noexcept {
    std::size_t n = 0;
    for (std::size_t i = 0; i < kUnitKindClassBodyCount; ++i) {
        const UnitKindClassBody& b = kUnitKindClassBodies[i];
        if (b.test_address == 0x00435360u) {
            continue;  // the second body of class 4Eh, not a second class
        }
        for (std::size_t j = 0; j < b.accept_count; ++j) {
            if (b.accepts[j] == literal) {
                ++n;
                break;
            }
        }
    }
    return n;
}

const UnitKindLiteralRow* unit_kind_literal_row(int literal) noexcept {
    for (std::size_t i = 0; i < kUnitKindLiteralCount; ++i) {
        if (kUnitKindLiterals[i].literal == literal) {
            return &kUnitKindLiterals[i];
        }
    }
    return nullptr;
}

}  // namespace bsp
