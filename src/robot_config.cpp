#include "bsp/robot_config.hpp"
#include "bsp/lua_numeric.hpp"

#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
struct OwnedRef {
    GuiLua51Host& host;
    GuiLuaRef ref;
    ~OwnedRef() { host.release(ref); }
    OwnedRef(GuiLua51Host& h, GuiLuaRef r) : host(h), ref(r) {}
    OwnedRef(const OwnedRef&) = delete;
    OwnedRef& operator=(const OwnedRef&) = delete;
};
struct OwnedString {
    NativeStringStorage& storage;
    NativeString value;
    ~OwnedString() { destroy_native_string_header_0041dd20(&value, storage); }
};
float float_bits(std::uint32_t bits) noexcept {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

// Direct descriptor virtual+4 bodies call the canonical Lua numeric accessors.
// Rows retain native lookup order, repeated array-parent lookups and exact
// float-default bits. Addresses identify the individual lookup callsites.
enum class RobotReadKind { Float, FloatDefault, Int, Scope };
struct RobotReadRow {
    const char* key;
    RobotReadKind kind;
    std::int32_t array_index;
    std::uint32_t offset;
    std::uint32_t fallback_bits;
    std::uint32_t lookup_address;
};
constexpr RobotReadRow kAAFlakBotRows[] = {
    {"GoodRatio", RobotReadKind::Float, 0, 0xc, 0x00000000u, 0x008fc6fdu},
    {"AngleErrMin", RobotReadKind::Float, 0, 0x10, 0x00000000u, 0x008fc739u},
    {"AngleErrMin", RobotReadKind::Float, 0, 0x14, 0x00000000u, 0x008fc769u},
    {"AngleErrBad", RobotReadKind::Float, 0, 0x18, 0x00000000u, 0x008fc799u},
    {"DistErrMin", RobotReadKind::Float, 0, 0x1c, 0x00000000u, 0x008fc7c9u},
    {"DistErrMax", RobotReadKind::Float, 0, 0x20, 0x00000000u, 0x008fc7f9u},
    {"DistErrBad", RobotReadKind::Float, 0, 0x24, 0x00000000u, 0x008fc829u},
    {"BulletThrowMul", RobotReadKind::Float, 0, 0x28, 0x00000000u, 0x008fc859u},
};

constexpr RobotReadRow kTailGunnerBotRows[] = {
    {"AimPeriodMin", RobotReadKind::FloatDefault, 0, 0x10, 0x3e4ccccdu, 0x008fca3du},
    {"AimPeriodMax", RobotReadKind::FloatDefault, 0, 0x14, 0x3f000000u, 0x008fca84u},
    {"AngleError", RobotReadKind::FloatDefault, 0, 0xc, 0x3fc00000u, 0x008fcabeu},
    {"ShootRange", RobotReadKind::FloatDefault, 0, 0x18, 0x44480000u, 0x008fcaf8u},
    {"BulletThrowMul", RobotReadKind::FloatDefault, 0, 0x1c, 0x3f800000u, 0x008fcb32u},
    {"SectionTargetChance", RobotReadKind::Float, 0, 0x20, 0x00000000u, 0x008fcb68u},
    {"EngineRoomWeight", RobotReadKind::Float, 0, 0x24, 0x00000000u, 0x008fcb98u},
    {"MagazineWeight", RobotReadKind::Float, 0, 0x28, 0x00000000u, 0x008fcbc8u},
    {"FueltankWeight", RobotReadKind::Float, 0, 0x2c, 0x00000000u, 0x008fcbf8u},
};

constexpr RobotReadRow kAAGunnerBotRows[] = {
    {"AngleDiffErrorRatio", RobotReadKind::Float, 0, 0xc, 0x00000000u, 0x008fcd8du},
    {"Dist2AngleErrRatio", RobotReadKind::Float, 0, 0x10, 0x00000000u, 0x008fcdc9u},
    {"ConstAngleError", RobotReadKind::Float, 0, 0x14, 0x00000000u, 0x008fcdf9u},
    {"BulletThrowMul", RobotReadKind::Float, 0, 0x18, 0x00000000u, 0x008fce29u},
};

constexpr RobotReadRow kPilotBotRows[] = {
    {"DiveBombTargetHError", RobotReadKind::Float, 0, 0x3c, 0x00000000u, 0x009973e4u},
    {"DiveBombTargetVError", RobotReadKind::Float, 0, 0x40, 0x00000000u, 0x0099742cu},
    {"DiveBombCalcTargetPosError", RobotReadKind::Float, 0, 0x38, 0x00000000u, 0x00997462u},
    {"DiveBombReleaseAlt", RobotReadKind::Float, 1, 0x44, 0x00000000u, 0x00997498u},
    {"DiveBombReleaseAlt", RobotReadKind::Float, 2, 0x48, 0x00000000u, 0x009974f5u},
    {"DiveBombNewReleaseMul", RobotReadKind::FloatDefault, 0, 0x4c, 0x3f19999au, 0x00997552u},
    {"DiveBombMaxPowerCtrl", RobotReadKind::Float, 0, 0x50, 0x00000000u, 0x00997592u},
    {"DiveBombMinPowerCtrl", RobotReadKind::Float, 0, 0x54, 0x00000000u, 0x009975c8u},
    {"DiveBombMaxBrakeCtrl", RobotReadKind::Float, 0, 0x58, 0x00000000u, 0x009975feu},
    {"DiveBombMinBrakeCtrl", RobotReadKind::Float, 0, 0x5c, 0x00000000u, 0x00997634u},
    {"DiveBombAimPitchRatio", RobotReadKind::Float, 0, 0x60, 0x00000000u, 0x0099766au},
    {"DiveBombTargetPointSelectPrec", RobotReadKind::Float, 0, 0x64, 0x00000000u, 0x009976a0u},
    {"DiveBombAimPrecDist", RobotReadKind::Float, 0, 0x68, 0x00000000u, 0x009976d6u},
    {"DiveBombAimPrecMul", RobotReadKind::Float, 0, 0x6c, 0x00000000u, 0x0099770cu},
    {"DiveBombAimPrecPullPlus", RobotReadKind::Float, 0, 0x70, 0x00000000u, 0x00997742u},
    {"DiveBombAimPrecPullMinus", RobotReadKind::Float, 0, 0x74, 0x00000000u, 0x00997778u},
    {"DiveBombThrowMul", RobotReadKind::Float, 0, 0x78, 0x00000000u, 0x009977aeu},
    {"DiveBombSectionDamageChance", RobotReadKind::Float, 0, 0x7c, 0x00000000u, 0x009977e4u},
    {"DiveBombEngineRoomWeight", RobotReadKind::Float, 0, 0x80, 0x00000000u, 0x0099781au},
    {"DiveBombMagazineWeight", RobotReadKind::Float, 0, 0x84, 0x00000000u, 0x00997853u},
    {"DiveBombFueltankWeight", RobotReadKind::Float, 0, 0x88, 0x00000000u, 0x0099788cu},
    {"KamikazeSectionDamageChance", RobotReadKind::Float, 0, 0x8c, 0x00000000u, 0x009978c5u},
    {"KamikazeEngineRoomWeight", RobotReadKind::Float, 0, 0x90, 0x00000000u, 0x009978feu},
    {"KamikazeMagazineWeight", RobotReadKind::Float, 0, 0x94, 0x00000000u, 0x00997937u},
    {"KamikazeFueltankWeight", RobotReadKind::Float, 0, 0x98, 0x00000000u, 0x00997970u},
    {"KamikazeTargetPrecision", RobotReadKind::Float, 0, 0x9c, 0x00000000u, 0x009979a9u},
    {"KamikazeTargetHError", RobotReadKind::Float, 0, 0xa0, 0x00000000u, 0x009979e2u},
    {"KamikazeTargetVError", RobotReadKind::Float, 0, 0xa4, 0x00000000u, 0x00997a1bu},
    {"KamikazeTargetProjTimeError", RobotReadKind::Float, 0, 0xa8, 0x00000000u, 0x00997a54u},
    {"KamikazeManeuverPrecisionTimer", RobotReadKind::Float, 0, 0xb4, 0x00000000u, 0x00997a8du},
    {"KamikazeManeuverPrecisionMul", RobotReadKind::Float, 1, 0xac, 0x00000000u, 0x00997ac6u},
    {"KamikazeManeuverPrecisionMul", RobotReadKind::Float, 2, 0xb0, 0x00000000u, 0x00997b26u},
    {"TorpReleaseAlt", RobotReadKind::Float, 0, 0xc, 0x00000000u, 0x00997b86u},
    {"TorpReleaseDistNear", RobotReadKind::Float, 0, 0x10, 0x00000000u, 0x00997bbcu},
    {"TorpReleaseDistFar", RobotReadKind::Float, 0, 0x14, 0x00000000u, 0x00997bf2u},
    {"TorpReleaseDropCloserMul", RobotReadKind::FloatDefault, 0, 0x18, 0x3f19999au, 0x00997c28u},
    {"TorpFlikFlakTime", RobotReadKind::Float, 1, 0x1c, 0x00000000u, 0x00997c68u},
    {"TorpFlikFlakTime", RobotReadKind::Float, 2, 0x20, 0x00000000u, 0x00997cc5u},
    {"TorpCalcTargetPosError", RobotReadKind::Float, 0, 0x24, 0x00000000u, 0x00997d22u},
    {"TorpTargetHError", RobotReadKind::Float, 0, 0x28, 0x00000000u, 0x00997d58u},
    {"TorpTargetVError", RobotReadKind::Float, 0, 0x2c, 0x00000000u, 0x00997d8eu},
    {"TorpTargetPointSelectPrec", RobotReadKind::Float, 0, 0x30, 0x00000000u, 0x00997dc4u},
    {"TorpThrowMul", RobotReadKind::Float, 0, 0x34, 0x00000000u, 0x00997dfau},
    {"LevelBombTargetHError", RobotReadKind::Float, 0, 0xb8, 0x00000000u, 0x00997e30u},
    {"LevelBombTargetVError", RobotReadKind::Float, 0, 0xbc, 0x00000000u, 0x00997e69u},
    {"LevelBombCalcTargetPosError", RobotReadKind::Float, 0, 0xc0, 0x00000000u, 0x00997ea2u},
    {"LevelBombTargetPointSelectPrec", RobotReadKind::Float, 0, 0xc4, 0x00000000u, 0x00997edbu},
    {"LevelBombThrowMul", RobotReadKind::Float, 0, 0xc8, 0x00000000u, 0x00997f14u},
    {"DepthChargeTargetHError", RobotReadKind::Float, 0, 0xcc, 0x00000000u, 0x00997f4du},
    {"DepthChargeTargetVError", RobotReadKind::Float, 0, 0xd0, 0x00000000u, 0x00997f86u},
    {"DepthChargeCalcTargetPosError", RobotReadKind::Float, 0, 0xd4, 0x00000000u, 0x00997fbfu},
    {"DepthChargeCancelTargetDist", RobotReadKind::Float, 0, 0xd8, 0x00000000u, 0x00997ff8u},
    {"DepthChargeTargetPointSelectPrec", RobotReadKind::Float, 0, 0xdc, 0x00000000u, 0x00998031u},
    {"DepthChargeThrowMul", RobotReadKind::Float, 0, 0xe0, 0x00000000u, 0x0099806au},
    {"AimDistortAngle", RobotReadKind::Float, 1, 0x22c, 0x00000000u, 0x009980a3u},
    {"AimDistortAngle", RobotReadKind::Float, 2, 0x230, 0x00000000u, 0x00998103u},
    {"AimDistortChangeSpeed", RobotReadKind::Float, 0, 0x234, 0x00000000u, 0x00998166u},
    {"AimShootDistance", RobotReadKind::Float, 0, 0x23c, 0x00000000u, 0x009981a5u},
    {"AimDontShootArea", RobotReadKind::Float, 0, 0x238, 0x00000000u, 0x009981e4u},
    {"AimShootTime", RobotReadKind::Float, 1, 0x244, 0x00000000u, 0x00998220u},
    {"AimShootTime", RobotReadKind::Float, 2, 0x240, 0x00000000u, 0x00998289u},
    {"AimShootDelayTime", RobotReadKind::Float, 1, 0x24c, 0x00000000u, 0x009982efu},
    {"AimShootDelayTime", RobotReadKind::Float, 2, 0x248, 0x00000000u, 0x0099835bu},
    {"AimBulletThrowMul", RobotReadKind::FloatDefault, 0, 0x250, 0x3f800000u, 0x009983c7u},
    {"StrafeTooCloseDistance", RobotReadKind::Float, 0, 0xe4, 0x00000000u, 0x0099840cu},
    {"StrafeGoAwayDistance", RobotReadKind::Float, 0, 0xe8, 0x00000000u, 0x0099844bu},
    {"StrafeAttackAngle", RobotReadKind::Float, 0, 0xec, 0x00000000u, 0x0099848au},
    {"StrafeTargetPointSelectPrec", RobotReadKind::Float, 0, 0xf0, 0x00000000u, 0x009984c9u},
    {"StrafeSectionDamageChance", RobotReadKind::Float, 0, 0xf4, 0x00000000u, 0x00998508u},
    {"StrafeEngineRoomWeight", RobotReadKind::Float, 0, 0xf8, 0x00000000u, 0x00998547u},
    {"StrafeMagazineWeight", RobotReadKind::Float, 0, 0xfc, 0x00000000u, 0x00998586u},
    {"StrafeFueltankWeight", RobotReadKind::Float, 0, 0x100, 0x00000000u, 0x009985c5u},
    {"StrikeTooClose", RobotReadKind::Float, 0, 0x104, 0x00000000u, 0x00998604u},
    {"StrikeGoAwayDistance", RobotReadKind::Float, 0, 0x108, 0x00000000u, 0x00998643u},
    {"StrikeAttackAngle", RobotReadKind::Float, 0, 0x10c, 0x00000000u, 0x00998682u},
    {"StrikeTargetPrec", RobotReadKind::Float, 0, 0x110, 0x00000000u, 0x009986c1u},
    {"StrikeFireAngle", RobotReadKind::Float, 0, 0x11c, 0x00000000u, 0x00998700u},
    {"StrikeHomingAngle", RobotReadKind::Float, 0, 0x114, 0x00000000u, 0x0099873fu},
    {"StrikeRepeatTime", RobotReadKind::Float, 0, 0x118, 0x00000000u, 0x0099877bu},
    {"StrikeFireDist", RobotReadKind::Float, 1, 0x120, 0x00000000u, 0x009987b7u},
    {"StrikeFireDist", RobotReadKind::Float, 2, 0x124, 0x00000000u, 0x0099881au},
    {"StrikeFireDist", RobotReadKind::Float, 3, 0x128, 0x00000000u, 0x00998883u},
    {"StrikeThrowMul", RobotReadKind::Float, 0, 0x12c, 0x00000000u, 0x009988e9u},
    {"StrikeSectionDamageChance", RobotReadKind::Float, 0, 0x130, 0x00000000u, 0x00998928u},
    {"StrikeEngineRoomWeight", RobotReadKind::Float, 0, 0x134, 0x00000000u, 0x00998967u},
    {"StrikeMagazineWeight", RobotReadKind::Float, 0, 0x138, 0x00000000u, 0x009989a6u},
    {"StrikeFueltankWeight", RobotReadKind::Float, 0, 0x13c, 0x00000000u, 0x009989e5u},
    {"DogfightFollowDist", RobotReadKind::Float, 0, 0x218, 0x00000000u, 0x00998a24u},
    {"DogfightBoringTime", RobotReadKind::Float, 0, 0x21c, 0x00000000u, 0x00998a63u},
    {"DogfightAvoidTime", RobotReadKind::Float, 0, 0x220, 0x00000000u, 0x00998aa2u},
    {"DogfightTurnAfterChance", RobotReadKind::Float, 0, 0x224, 0x00000000u, 0x00998ae1u},
    {"DogfightManeuverChangeTime", RobotReadKind::Float, 0, 0x228, 0x00000000u, 0x00998b20u},
    {"RocketCheckTime", RobotReadKind::Float, 1, 0x140, 0x00000000u, 0x00998b5fu},
    {"RocketCheckTime", RobotReadKind::Float, 2, 0x144, 0x00000000u, 0x00998bcbu},
    {"Rocket_SmallPlane", RobotReadKind::Scope, 0, 0x0, 0x00000000u, 0x00998c4cu},
    {"RocketChance", RobotReadKind::Float, 0, 0x148, 0x00000000u, 0x00998c87u},
    {"AttackDist", RobotReadKind::Float, 0, 0x14c, 0x00000000u, 0x00998cc5u},
    {"AttackAngle", RobotReadKind::Float, 0, 0x170, 0x00000000u, 0x00998d03u},
    {"AttackTime", RobotReadKind::Float, 0, 0x164, 0x00000000u, 0x00998d41u},
    {"HomingAngle", RobotReadKind::Float, 0, 0x174, 0x00000000u, 0x00998d7fu},
    {"NumRockets", RobotReadKind::Int, 0, 0x168, 0x00000000u, 0x00998dbdu},
    {"RocketDelay", RobotReadKind::Float, 0, 0x16c, 0x00000000u, 0x00998e00u},
    {"Rocket_LargePlane", RobotReadKind::Scope, 0, 0x0, 0x00000000u, 0x00998e3cu},
    {"RocketChance", RobotReadKind::Float, 0, 0x17c, 0x00000000u, 0x00998e77u},
    {"AttackDist", RobotReadKind::Float, 0, 0x184, 0x00000000u, 0x00998eb5u},
    {"AttackAngle", RobotReadKind::Float, 0, 0x1a4, 0x00000000u, 0x00998ef3u},
    {"AttackTime", RobotReadKind::Float, 0, 0x198, 0x00000000u, 0x00998f31u},
    {"HomingAngle", RobotReadKind::Float, 0, 0x1a8, 0x00000000u, 0x00998f6fu},
    {"NumRockets", RobotReadKind::Int, 0, 0x19c, 0x00000000u, 0x00998fadu},
    {"RocketDelay", RobotReadKind::Float, 0, 0x1a0, 0x00000000u, 0x00998ff0u},
    {"Rocket_Ship", RobotReadKind::Scope, 0, 0x0, 0x00000000u, 0x0099902cu},
    {"RocketChance", RobotReadKind::Float, 0, 0x1b0, 0x00000000u, 0x00999067u},
    {"AttackDist", RobotReadKind::Float, 1, 0x1bc, 0x00000000u, 0x009990a5u},
    {"AttackDist", RobotReadKind::Float, 2, 0x1c0, 0x00000000u, 0x00999110u},
    {"AttackDist", RobotReadKind::Float, 3, 0x1c4, 0x00000000u, 0x0099917bu},
    {"AttackAngle", RobotReadKind::Float, 0, 0x1d8, 0x00000000u, 0x009991e6u},
    {"AttackTime", RobotReadKind::Float, 0, 0x1cc, 0x00000000u, 0x00999224u},
    {"HomingAngle", RobotReadKind::Float, 0, 0x1dc, 0x00000000u, 0x00999262u},
    {"NumRockets", RobotReadKind::Int, 0, 0x1d0, 0x00000000u, 0x009992a0u},
    {"RocketDelay", RobotReadKind::Float, 0, 0x1d4, 0x00000000u, 0x009992e3u},
    {"Rocket_Landfort", RobotReadKind::Scope, 0, 0x0, 0x00000000u, 0x0099931fu},
    {"RocketChance", RobotReadKind::Float, 0, 0x1e4, 0x00000000u, 0x0099935au},
    {"AttackDist", RobotReadKind::Float, 0, 0x1fc, 0x00000000u, 0x00999398u},
    {"AttackAngle", RobotReadKind::Float, 0, 0x20c, 0x00000000u, 0x009993d6u},
    {"AttackTime", RobotReadKind::Float, 0, 0x200, 0x00000000u, 0x00999414u},
    {"HomingAngle", RobotReadKind::Float, 0, 0x210, 0x00000000u, 0x00999452u},
    {"NumRockets", RobotReadKind::Int, 0, 0x204, 0x00000000u, 0x00999490u},
    {"RocketDelay", RobotReadKind::Float, 0, 0x208, 0x00000000u, 0x009994d3u},
};

constexpr RobotReadRow kArtillerySubDirectorBotRows[] = {
    {"MaxErrorRadius", RobotReadKind::Float, 0, 0xc, 0x00000000u, 0x008fcf5du},
    {"ErrorRangeMul", RobotReadKind::Float, 0, 0x10, 0x00000000u, 0x008fcf9au},
    {"MinErrorMul", RobotReadKind::Float, 0, 0x14, 0x00000000u, 0x008fcfcau},
    {"ApproachMulMin", RobotReadKind::Float, 0, 0x18, 0x00000000u, 0x008fcffau},
    {"ApproachMulMax", RobotReadKind::Float, 0, 0x1c, 0x00000000u, 0x008fd02au},
    {"DeviationMul", RobotReadKind::Float, 0, 0x20, 0x00000000u, 0x008fd05au},
    {"AngleChange", RobotReadKind::Float, 0, 0x24, 0x00000000u, 0x008fd08au},
    {"ErrorDistIncStartTime", RobotReadKind::Float, 0, 0x28, 0x00000000u, 0x008fd0bau},
    {"ErrorDistIncFullTime", RobotReadKind::Float, 0, 0x2c, 0x00000000u, 0x008fd0eau},
    {"BulletThrowMul", RobotReadKind::Float, 0, 0x30, 0x00000000u, 0x008fd11au},
};

constexpr RobotReadRow kArtilleryGunnerBotRows[] = {
    {"MaxAngleError", RobotReadKind::Float, 0, 0xc, 0x00000000u, 0x008fd39du},
    {"Power", RobotReadKind::Float, 0, 0x10, 0x00000000u, 0x008fd3e0u},
    {"TargetPointRefreshTime", RobotReadKind::Float, 0, 0x14, 0x00000000u, 0x008fd410u},
    {"SectionTargetChance", RobotReadKind::Float, 0, 0x18, 0x00000000u, 0x008fd440u},
    {"EngineRoomWeight", RobotReadKind::Float, 0, 0x1c, 0x00000000u, 0x008fd470u},
    {"MagazineWeight", RobotReadKind::Float, 0, 0x20, 0x00000000u, 0x008fd4a0u},
    {"FueltankWeight", RobotReadKind::Float, 0, 0x24, 0x00000000u, 0x008fd4d0u},
};

constexpr RobotReadRow kTorpedoBotRows[] = {
    {"AngleErrMin", RobotReadKind::Float, 0, 0xc, 0x00000000u, 0x008fd66du},
    {"AngleErrMax", RobotReadKind::Float, 0, 0x10, 0x00000000u, 0x008fd6aau},
    {"FireTargetAccuracy", RobotReadKind::Float, 0, 0x14, 0x00000000u, 0x008fd6dau},
    {"AnyTargetAccuracy", RobotReadKind::Float, 0, 0x18, 0x00000000u, 0x008fd70au},
    {"BulletThrowMul", RobotReadKind::Float, 0, 0x1c, 0x00000000u, 0x008fd73au},
};

constexpr RobotReadRow kDepthChargeBotRows[] = {
    {"AttackDist", RobotReadKind::Float, 0, 0xc, 0x00000000u, 0x008fd8adu},
    {"BulletThrowMul", RobotReadKind::Float, 0, 0x10, 0x00000000u, 0x008fd8eau},
    {"ContinuousFireTime", RobotReadKind::FloatDefault, 0, 0x14, 0x3f000000u, 0x008fd91au},
    {"FireDelay", RobotReadKind::FloatDefault, 1, 0x18, 0x00000000u, 0x008fd954u},
    {"FireDelay", RobotReadKind::FloatDefault, 2, 0x1c, 0x3f000000u, 0x008fd9abu},
};

constexpr RobotReadRow kNavigatorBotRows[] = {
    {"SubAttackDistMultiplier", RobotReadKind::Float, 0, 0xc, 0x00000000u, 0x009d53deu},
    {"TorpedoPredict", RobotReadKind::Float, 1, 0x10, 0x00000000u, 0x009d541bu},
    {"TorpedoPredict", RobotReadKind::Float, 2, 0x14, 0x00000000u, 0x009d546cu},
    {"TorpedoObservation", RobotReadKind::FloatDefault, 1, 0x1c, 0x3f800000u, 0x009d54bdu},
    {"TorpedoObservation", RobotReadKind::FloatDefault, 2, 0x20, 0x40800000u, 0x009d5514u},
    {"TorpedoObservationSubAddon", RobotReadKind::FloatDefault, 0, 0x24, 0x40000000u, 0x009d556fu},
    {"TorpedoSpdErr", RobotReadKind::FloatDefault, 1, 0x28, 0x00000000u, 0x009d55a9u},
    {"TorpedoSpdErr", RobotReadKind::FloatDefault, 2, 0x2c, 0x00000000u, 0x009d5600u},
    {"TorpedoPredictReferenceLength", RobotReadKind::FloatDefault, 0, 0x18, 0x42c80000u, 0x009d5657u},
};

template<std::size_t N>
void read_rows(RobotDescriptor& object, GuiLua51Host& host, GuiLuaRef root,
    std::uint32_t level, std::uint32_t stride, const RobotReadRow (&rows)[N],
    const bool& crt_mode) {
    OwnedRef scope(host, {});
    bool has_scope = false;
    auto* const bytes = reinterpret_cast<std::byte*>(&object) + level * stride;
    for (const auto& row : rows) {
        if (row.kind == RobotReadKind::Scope) {
            OwnedRef next(host, host.get_by_name(root, row.key));
            //00B67690: release the old retained scope before copying the new
            // temporary. Keep that temporary alive across the copy operation.
            host.release(scope.ref);
            scope.ref = {};
            scope.ref = host.copy_ref_00b66fa0(next.ref);
            has_scope = true;
            continue;
        }
        OwnedRef value(host, host.get_by_name(has_scope ? scope.ref : root, row.key));
        const auto store = [&](GuiLuaRef ref) {
            if (row.kind == RobotReadKind::Int) {
                const auto number = lua_object_integer_00b66290(host, ref, crt_mode);
                std::memcpy(bytes + row.offset, &number, sizeof(number));
            } else {
                //00B66330 accepts a NUMBER in a tracked LuaObject. Numeric
                // strings use the compiled default; ordinary00B66270 coerces.
                const float number = row.kind == RobotReadKind::FloatDefault &&
                    host.type_of(ref) != GuiLuaType::Number ? float_bits(row.fallback_bits) :
                    lua_object_number_00b66270(host, ref);
                std::memcpy(bytes + row.offset, &number, sizeof(number));
            }
        };
        if (row.array_index != 0) {
            OwnedRef component(host, host.get_by_index(value.ref, row.array_index));
            store(component.ref);
            // Native destroys the component before its freshly looked-up parent.
        } else {
            store(value.ref);
        }
    }
}

void read_robot_descriptor(RobotDescriptor& object, GuiLua51Host& host,
    GuiLuaRef level_table, std::uint32_t level, const bool& crt_mode) {
    // Reached after each level-table Lua lookup: do not cache this dispatch
    // across Lua metamethods. The vtable word is actual native storage.
    switch (object.native_vtable_00) {
    case 0x00d17decu: return read_rows(object, host, level_table, level, 0x20, kAAFlakBotRows, crt_mode);
    case 0x00d17e50u: return read_rows(object, host, level_table, level, 0x24, kTailGunnerBotRows, crt_mode);
    case 0x00d17edcu: return read_rows(object, host, level_table, level, 0x10, kAAGunnerBotRows, crt_mode);
    case 0x00d17de0u: return read_rows(object, host, level_table, level, 0x248, kPilotBotRows, crt_mode);
    case 0x00d17f20u: return read_rows(object, host, level_table, level, 0x28, kArtillerySubDirectorBotRows, crt_mode);
    case 0x00d17fc4u: return read_rows(object, host, level_table, level, 0x1c, kArtilleryGunnerBotRows, crt_mode);
    case 0x00d17ff8u: return read_rows(object, host, level_table, level, 0x14, kTorpedoBotRows, crt_mode);
    case 0x00d18038u: return read_rows(object, host, level_table, level, 0x14, kDepthChargeBotRows, crt_mode);
    case 0x00d217dcu: return read_rows(object, host, level_table, level, 0x24, kNavigatorBotRows, crt_mode);
    default: throw std::invalid_argument("unrecovered robot descriptor read vtable");
    }
}

// The validators mix COMISS and x87 FCOMI, including unordered branches,
// explicit spills and resident double constants. Keep those game instructions
// rather than substituting C++ comparisons with different NaN/x87 behavior.
const std::uint32_t kOneD7A24C = 0x3f800000u;
const std::uint64_t kFourHundredCE3D90 = 0x4079000000000000ull;
const std::uint64_t kHundredD7A220 = 0x4059000000000000ull;
const std::uint64_t kAngleCE3828 = 0x401921fb60000000ull;
// Full008fc930 through008fc9b9; ECX=this, result AL.
bool validate_008fc930(RobotDescriptor& object) {
    unsigned char result;
    __asm {
        mov ecx, object
        push ecx // 008fc930
        xorps xmm1, xmm1 // 008fc931
        movss xmm2, dword ptr [kOneD7A24C] // 008fc934
        xor edx, edx // 008fc93c
        add ecx, 0x14 // 008fc93e
    robot_008fc941:
        movss xmm0, dword ptr [ecx - 8] // 008fc941
        comiss xmm0, xmm1 // 008fc946
        jb robot_008fc9b6 // 008fc949
        comiss xmm2, xmm0 // 008fc94b
        jb robot_008fc9b6 // 008fc94e
        fld dword ptr [ecx] // 008fc950
        fstp dword ptr [esp] // 008fc952
        fld dword ptr [esp] // 008fc955
        fld dword ptr [ecx + 4] // 008fc958
        fcomip st(0), st(1) // 008fc95b
        jb robot_008fc9b4 // 008fc95d
        fld dword ptr [ecx - 4] // 008fc95f
        fstp dword ptr [esp] // 008fc962
        fld dword ptr [esp] // 008fc965
        fxch st(1) // 008fc968
        fcomip st(0), st(1) // 008fc96a
        fstp st(0) // 008fc96c
        jb robot_008fc9b6 // 008fc96e
        movss xmm0, dword ptr [esp] // 008fc970
        comiss xmm0, xmm1 // 008fc975
        jb robot_008fc9b6 // 008fc978
        fld dword ptr [ecx + 0xc] // 008fc97a
        fstp dword ptr [esp] // 008fc97d
        fld dword ptr [esp] // 008fc980
        fld dword ptr [ecx + 0x10] // 008fc983
        fcomip st(0), st(1) // 008fc986
        jb robot_008fc9b4 // 008fc988
        fld dword ptr [ecx + 8] // 008fc98a
        fstp dword ptr [esp] // 008fc98d
        fld dword ptr [esp] // 008fc990
        fxch st(1) // 008fc993
        fcomip st(0), st(1) // 008fc995
        fstp st(0) // 008fc997
        jb robot_008fc9b6 // 008fc999
        movss xmm0, dword ptr [esp] // 008fc99b
        comiss xmm0, xmm1 // 008fc9a0
        jb robot_008fc9b6 // 008fc9a3
        add edx, 1 // 008fc9a5
        add ecx, 0x20 // 008fc9a8
        cmp edx, 6 // 008fc9ab
        jl robot_008fc941 // 008fc9ae
        mov al, 1 // 008fc9b0
        pop ecx // 008fc9b2
        mov result, al
        jmp done_008fc930
    robot_008fc9b4:
        fstp st(0) // 008fc9b4
    robot_008fc9b6:
        xor al, al // 008fc9b6
        pop ecx // 008fc9b8
        mov result, al
        jmp done_008fc930
    done_008fc930:
    }
    return result != 0;
}

// Full008fcca0 through008fcd07; ECX=this, result AL.
bool validate_008fcca0(RobotDescriptor& object) {
    unsigned char result;
    __asm {
        mov ecx, object
        xorps xmm0, xmm0 // 008fcca0
        fld qword ptr [kFourHundredCE3D90] // 008fcca3
        sub esp, 8 // 008fcca9
        xor edx, edx // 008fccac
        add ecx, 0x10 // 008fccae
    robot_008fccb1:
        movss xmm1, dword ptr [ecx - 4] // 008fccb1
        comiss xmm1, xmm0 // 008fccb6
        jb robot_008fcd00 // 008fccb9
        movss xmm1, dword ptr [ecx] // 008fccbb
        comiss xmm1, xmm0 // 008fccbf
        movss dword ptr [esp + 4], xmm1 // 008fccc2
        jb robot_008fcd00 // 008fccc8
        movss xmm1, dword ptr [ecx + 4] // 008fccca
        comiss xmm1, xmm0 // 008fcccf
        movss dword ptr [esp], xmm1 // 008fccd2
        jb robot_008fcd00 // 008fccd7
        fld dword ptr [esp + 4] // 008fccd9
        fld dword ptr [esp] // 008fccdd
        fcomip st(0), st(1) // 008fcce0
        fstp st(0) // 008fcce2
        jb robot_008fcd00 // 008fcce4
        fld dword ptr [ecx + 8] // 008fcce6
        fcomip st(0), st(1) // 008fcce9
        jb robot_008fcd00 // 008fcceb
        add edx, 1 // 008fcced
        add ecx, 0x24 // 008fccf0
        cmp edx, 6 // 008fccf3
        jl robot_008fccb1 // 008fccf6
        mov al, 1 // 008fccf8
        fstp st(0) // 008fccfa
        add esp, 8 // 008fccfc
        mov result, al
        jmp done_008fcca0
    robot_008fcd00:
        xor al, al // 008fcd00
        fstp st(0) // 008fcd02
        add esp, 8 // 008fcd04
        mov result, al
        jmp done_008fcca0
    done_008fcca0:
    }
    return result != 0;
}

// Full008fcea0 through008fcedd; ECX=this, result AL.
bool validate_008fcea0(RobotDescriptor& object) {
    unsigned char result;
    __asm {
        mov ecx, object
        xorps xmm0, xmm0 // 008fcea0
        xor eax, eax // 008fcea3
        add ecx, 0x14 // 008fcea5
        jmp robot_008fceb0 // 008fcea8
        lea ebx, [ebx] // 008fceaa
    robot_008fceb0:
        movss xmm1, dword ptr [ecx - 8] // 008fceb0
        comiss xmm1, xmm0 // 008fceb5
        jb robot_008fcedb // 008fceb8
        movss xmm1, dword ptr [ecx - 4] // 008fceba
        comiss xmm1, xmm0 // 008fcebf
        jb robot_008fcedb // 008fcec2
        movss xmm1, dword ptr [ecx] // 008fcec4
        comiss xmm1, xmm0 // 008fcec8
        jb robot_008fcedb // 008fcecb
        add eax, 1 // 008fcecd
        add ecx, 0x10 // 008fced0
        cmp eax, 6 // 008fced3
        jl robot_008fceb0 // 008fced6
        mov al, 1 // 008fced8
        mov result, al
        jmp done_008fcea0
    robot_008fcedb:
        xor al, al // 008fcedb
        mov result, al
        jmp done_008fcea0
    done_008fcea0:
    }
    return result != 0;
}

// Full008fd230 through008fd314; ECX=this, result AL.
bool validate_008fd230(RobotDescriptor& object) {
    unsigned char result;
    __asm {
        mov ecx, object
        fld qword ptr [kHundredD7A220] // 008fd230
        xorps xmm0, xmm0 // 008fd236
        movss xmm2, dword ptr [kOneD7A24C] // 008fd239
        fld qword ptr [kAngleCE3828] // 008fd241
        sub esp, 8 // 008fd247
        xor edx, edx // 008fd24a
        add ecx, 0x14 // 008fd24c
    robot_008fd24f:
        movss xmm1, dword ptr [ecx - 8] // 008fd24f
        comiss xmm1, xmm0 // 008fd254
        jb robot_008fd30b // 008fd257
        movss xmm1, dword ptr [ecx - 4] // 008fd25d
        comiss xmm1, xmm0 // 008fd262
        jb robot_008fd30b // 008fd265
        comiss xmm2, xmm1 // 008fd26b
        jb robot_008fd30b // 008fd26e
        movss xmm1, dword ptr [ecx] // 008fd274
        comiss xmm1, xmm0 // 008fd278
        jb robot_008fd30b // 008fd27b
        comiss xmm2, xmm1 // 008fd281
        jb robot_008fd30b // 008fd284
        movss xmm1, dword ptr [ecx + 4] // 008fd28a
        comiss xmm1, xmm0 // 008fd28f
        jb robot_008fd30b // 008fd292
        comiss xmm2, xmm1 // 008fd294
        jbe robot_008fd30b // 008fd297
        movss xmm1, dword ptr [ecx + 8] // 008fd299
        comiss xmm1, xmm0 // 008fd29e
        jb robot_008fd30b // 008fd2a1
        comiss xmm2, xmm1 // 008fd2a3
        jbe robot_008fd30b // 008fd2a6
        movss xmm1, dword ptr [ecx + 0xc] // 008fd2a8
        comiss xmm1, xmm0 // 008fd2ad
        movss dword ptr [esp], xmm1 // 008fd2b0
        jb robot_008fd30b // 008fd2b5
        fld dword ptr [esp] // 008fd2b7
        fxch st(2) // 008fd2ba
        fcomi st(0), st(2) // 008fd2bc
        fstp st(2) // 008fd2be
        jbe robot_008fd30b // 008fd2c0
        movss xmm1, dword ptr [ecx + 0x14] // 008fd2c2
        comiss xmm1, xmm0 // 008fd2c7
        jb robot_008fd30b // 008fd2ca
        movss xmm1, dword ptr [ecx + 0x18] // 008fd2cc
        comiss xmm1, xmm0 // 008fd2d1
        jb robot_008fd30b // 008fd2d4
        movss xmm1, dword ptr [ecx + 0x10] // 008fd2d6
        comiss xmm1, xmm0 // 008fd2db
        movss dword ptr [esp + 4], xmm1 // 008fd2de
        jb robot_008fd30b // 008fd2e4
        fld dword ptr [esp + 4] // 008fd2e6
        fxch st(1) // 008fd2ea
        fcomi st(0), st(1) // 008fd2ec
        fstp st(1) // 008fd2ee
        jb robot_008fd30b // 008fd2f0
        add edx, 1 // 008fd2f2
        add ecx, 0x28 // 008fd2f5
        cmp edx, 6 // 008fd2f8
        jl robot_008fd24f // 008fd2fb
        fstp st(1) // 008fd301
        mov al, 1 // 008fd303
        fstp st(0) // 008fd305
        add esp, 8 // 008fd307
        mov result, al
        jmp done_008fd230
    robot_008fd30b:
        fstp st(1) // 008fd30b
        xor al, al // 008fd30d
        fstp st(0) // 008fd30f
        add esp, 8 // 008fd311
        mov result, al
        jmp done_008fd230
    done_008fd230:
    }
    return result != 0;
}

// Full008fd580 through008fd5ea; ECX=this, result AL.
bool validate_008fd580(RobotDescriptor& object) {
    unsigned char result;
    __asm {
        mov ecx, object
        movss xmm2, dword ptr [kOneD7A24C] // 008fd580
        xorps xmm1, xmm1 // 008fd588
        xor edx, edx // 008fd58b
        lea eax, [ecx + 0x1c] // 008fd58d
    robot_008fd590:
        movss xmm0, dword ptr [eax - 0xc] // 008fd590
        comiss xmm0, xmm2 // 008fd595
        jb robot_008fd5e8 // 008fd598
        movss xmm0, dword ptr [eax - 0x10] // 008fd59a
        comiss xmm0, xmm1 // 008fd59f
        jb robot_008fd5e8 // 008fd5a2
        movss xmm0, dword ptr [eax] // 008fd5a4
        comiss xmm0, xmm1 // 008fd5a8
        jb robot_008fd5e8 // 008fd5ab
        movss xmm0, dword ptr [eax + 4] // 008fd5ad
        comiss xmm0, xmm1 // 008fd5b2
        jb robot_008fd5e8 // 008fd5b5
        movss xmm0, dword ptr [eax + 8] // 008fd5b7
        comiss xmm0, xmm1 // 008fd5bc
        jb robot_008fd5e8 // 008fd5bf
        movss xmm0, dword ptr [eax - 4] // 008fd5c1
        comiss xmm0, xmm1 // 008fd5c6
        jb robot_008fd5e8 // 008fd5c9
        comiss xmm2, xmm0 // 008fd5cb
        jb robot_008fd5e8 // 008fd5ce
        movss xmm0, dword ptr [eax - 8] // 008fd5d0
        comiss xmm0, xmm1 // 008fd5d5
        jb robot_008fd5e8 // 008fd5d8
        add edx, 1 // 008fd5da
        add eax, 0x1c // 008fd5dd
        cmp edx, 6 // 008fd5e0
        jl robot_008fd590 // 008fd5e3
        mov al, 1 // 008fd5e5
        mov result, al
        jmp done_008fd580
    robot_008fd5e8:
        xor al, al // 008fd5e8
        mov result, al
        jmp done_008fd580
    done_008fd580:
    }
    return result != 0;
}

// Full008fd7d0 through008fd820; ECX=this, result AL.
bool validate_008fd7d0(RobotDescriptor& object) {
    unsigned char result;
    __asm {
        mov ecx, object
        push ecx // 008fd7d0
        xorps xmm1, xmm1 // 008fd7d1
        xor edx, edx // 008fd7d4
        add ecx, 0x14 // 008fd7d6
        lea esp, [esp] // 008fd7d9
    robot_008fd7e0:
        movss xmm0, dword ptr [ecx - 8] // 008fd7e0
        comiss xmm0, xmm1 // 008fd7e5
        movss dword ptr [esp], xmm0 // 008fd7e8
        jb robot_008fd81d // 008fd7ed
        fld dword ptr [esp] // 008fd7ef
        fld dword ptr [ecx - 4] // 008fd7f2
        fcomip st(0), st(1) // 008fd7f5
        fstp st(0) // 008fd7f7
        jb robot_008fd81d // 008fd7f9
        movss xmm0, dword ptr [ecx] // 008fd7fb
        comiss xmm0, xmm1 // 008fd7ff
        jb robot_008fd81d // 008fd802
        movss xmm0, dword ptr [ecx + 4] // 008fd804
        comiss xmm0, xmm1 // 008fd809
        jb robot_008fd81d // 008fd80c
        add edx, 1 // 008fd80e
        add ecx, 0x14 // 008fd811
        cmp edx, 6 // 008fd814
        jl robot_008fd7e0 // 008fd817
        mov al, 1 // 008fd819
        pop ecx // 008fd81b
        mov result, al
        jmp done_008fd7d0
    robot_008fd81d:
        xor al, al // 008fd81d
        pop ecx // 008fd81f
        mov result, al
        jmp done_008fd7d0
    done_008fd7d0:
    }
    return result != 0;
}

// Full008fda60 through008fdab0; ECX=this, result AL.
bool validate_008fda60(RobotDescriptor& object) {
    unsigned char result;
    __asm {
        mov ecx, object
        push ecx // 008fda60
        xorps xmm1, xmm1 // 008fda61
        xor edx, edx // 008fda64
        add ecx, 0x18 // 008fda66
        lea esp, [esp] // 008fda69
    robot_008fda70:
        movss xmm0, dword ptr [ecx - 0xc] // 008fda70
        comiss xmm0, xmm1 // 008fda75
        jbe robot_008fdaad // 008fda78
        movss xmm0, dword ptr [ecx - 4] // 008fda7a
        comiss xmm0, xmm1 // 008fda7f
        jb robot_008fdaad // 008fda82
        movss xmm0, dword ptr [ecx] // 008fda84
        comiss xmm0, xmm1 // 008fda88
        movss dword ptr [esp], xmm0 // 008fda8b
        jb robot_008fdaad // 008fda90
        fld dword ptr [esp] // 008fda92
        fld dword ptr [ecx + 4] // 008fda95
        fcomip st(0), st(1) // 008fda98
        fstp st(0) // 008fda9a
        jb robot_008fdaad // 008fda9c
        add edx, 1 // 008fda9e
        add ecx, 0x14 // 008fdaa1
        cmp edx, 6 // 008fdaa4
        jl robot_008fda70 // 008fdaa7
        mov al, 1 // 008fdaa9
        pop ecx // 008fdaab
        mov result, al
        jmp done_008fda60
    robot_008fdaad:
        xor al, al // 008fdaad
        pop ecx // 008fdaaf
        mov result, al
        jmp done_008fda60
    done_008fda60:
    }
    return result != 0;
}

// Full009999f0 through009999f2; ECX=this, result AL.
bool validate_009999f0(RobotDescriptor& object) {
    (void)object;
    return true; // MOV AL,1; RET
}

// Full009d56a0 through009d56a2; ECX=this, result AL.
bool validate_009d56a0(RobotDescriptor& object) {
    (void)object;
    return true; // MOV AL,1; RET
}

template<class T>
RobotDescriptor* construct_descriptor(RobotDescriptorStorage& storage,
    const char* name, std::uint32_t derived_vtable) {
    // Default-initialize only: native constructors leave +4 and every parameter
    // byte untouched. Actual storage ownership remains with the caller/registry.
    void* const block = storage.allocate(static_cast<std::uint32_t>(sizeof(T)));
    if (!block) throw std::bad_alloc(); // allocator's explicit nonnull contract
    auto* const object = ::new (block) T;
    object->descriptor.native_vtable_00 = kRobotBaseVtable;
    object->descriptor.name_08 = duplicate_00438e40(name);
    object->descriptor.native_vtable_00 = derived_vtable;
    return &object->descriptor;
}

void run_robot_script(PcStorageLuaOwner& lua, LuaScriptRuntime& scripts,
    NativeStringStorage& strings, const char* path) {
    OwnedString name{strings, {}};
    // Both native literals are 29 bytes. Resize then memcpy includes the NUL.
    name.value.resize_0041dd40(strings, 0x1d, false);
    std::memcpy(name.value.data(), path, 0x1e);
    (void)scripts.run_file(lua.storage_lua_38(), name.value.data(), false);
}
} // namespace

bool RobotNameLess::operator()(const char* left, const char* right) const noexcept {
    //0043A610 operates on borrowed pointer keys, including the null ordering.
    if (left == right) return false;
    if (!left) return true;
    if (!right) return false;
    return ::_stricmp(left, right) < 0;
}

std::pair<RobotConfigRegistry::iterator, bool> insert_robot_config_00901210(
    RobotConfigRegistry& registry, const char* borrowed_name, RobotDescriptor* object) {
    return registry.emplace(borrowed_name, object);
}

RobotDescriptor* find_robot_config_00900af0(RobotConfigRegistry& registry, const char* name) {
    const auto found = registry.find(name);
    return found == registry.end() ? nullptr : found->second;
}

bool validate_robot_descriptor(RobotDescriptor& object) {
    switch (object.native_vtable_00) {
    case 0x00d17decu: return validate_008fc930(object);
    case 0x00d17e50u: return validate_008fcca0(object);
    case 0x00d17edcu: return validate_008fcea0(object);
    case 0x00d17de0u: return validate_009999f0(object);
    case 0x00d17f20u: return validate_008fd230(object);
    case 0x00d17fc4u: return validate_008fd580(object);
    case 0x00d17ff8u: return validate_008fd7d0(object);
    case 0x00d18038u: return validate_008fda60(object);
    case 0x00d217dcu: return validate_009d56a0(object);
    default: throw std::invalid_argument("unrecovered robot descriptor validate vtable");
    }
}

void register_robot_config_009013d0(RobotConfigRegistry& registry, RobotDescriptor& object,
    GuiLua51Host& host, GuiLuaRef robots, RobotConfigContext& context) {
    const char* name = object.name_08;
    if (!name) name = "unnamed MBotClass";
    OwnedRef table(host, host.get_by_name(robots, name));
    object.no_target_time_until_rest_04 = float_bits(0x7f7fffffu);
    OwnedRef no_target(host, host.get_by_name(table.ref, "NoTargetTimeUntilRest"));
    if (host.type_of(no_target.ref) != GuiLuaType::Nil)
        object.no_target_time_until_rest_04 = lua_object_number_00b66270(host, no_target.ref);
    const struct { const char* key; std::uint32_t index; } levels[] = {
        {"SPNormal", 1}, {"MPNormal", 3}, {"SPVeteran", 2},
        {"MPVeteran", 4}, {"Elite", 5}, {"Stun", 0}
    };
    for (const auto& level : levels) {
        OwnedRef value(host, host.get_by_name(table.ref, level.key));
        read_robot_descriptor(object, host, value.ref, level.index, context.crt_sse2_conversion);
    }
    (void)validate_robot_descriptor(object); // native ignores AL
    name = object.name_08; // reload after all virtual/read callbacks
    if (!name) name = "unnamed MBotClass";
    (void)insert_robot_config_00901210(registry, name, &object);
    // Native borrowed key wrappers own no bytes. The two live Lua refs release
    // in this scope's reverse order, after the unique map insertion.
}

RobotDescriptor* delete_robot_descriptor(RobotDescriptor* object, std::uint32_t flags,
    RobotDescriptorStorage& storage) {
    // These ten known vtables resolve to the same fully recovered scalar body.
    // Other vtables require their actual deleting implementation, not a callback
    // pretending that all robot subclasses have this lifetime.
    switch (object->native_vtable_00) {
    case kRobotBaseVtable: case 0x00d17decu: case 0x00d17e50u: case 0x00d17edcu:
    case 0x00d17de0u: case 0x00d17f20u: case 0x00d17fc4u: case 0x00d17ff8u:
    case 0x00d18038u: case 0x00d217dcu: break;
    default: throw std::invalid_argument("unrecovered robot descriptor deleting vtable");
    }
    object->native_vtable_00 = kRobotBaseVtable;
    char* const name = object->name_08;
    if (name) {
        release_duplicate_00438e40(name);
        object->name_08 = nullptr;
    }
    if ((flags & 1u) != 0) storage.release(object);
    return object; // native EAX is the original pointer even after free
}

void clear_robot_registry_00900bb0(RobotConfigRegistry& registry, RobotDescriptorStorage& storage) {
    for (auto it = registry.begin(); it != registry.end(); ++it)
        if (it->second) (void)delete_robot_descriptor(it->second, 1, storage);
    registry.clear(); // no key comparisons after borrowed names were destroyed
}

bool load_robot_config_00901610(RobotConfigRegistry& registry, RobotConfigAliases aliases,
    LuaStateOwnerEnvironment environment, LuaScriptRuntime& scripts, RobotConfigContext& context) {
    PcStorageLuaOwner lua(std::move(environment));
    lua.open_storage_archive_00b6a020(0x41);
    run_robot_script(lua, scripts, context.strings, "Scripts\\global\\luaMW_init.lua");
    run_robot_script(lua, scripts, context.strings, "Scripts\\datatables\\Robots.lua");
    {
        GuiLua51Host host(*lua.storage_lua_38());
        OwnedRef globals(host, host.globals());
        OwnedRef robots(host, host.get_by_name(globals.ref, "Robots"));
        auto add = [&](RobotDescriptor* object) {
            register_robot_config_009013d0(registry, *object, host, robots.ref, context);
        };
        add(construct_descriptor<AAFlakBotConfig>(context.descriptors, "AAFlakBot", 0x00d17dec));
        add(construct_descriptor<TailGunnerBotConfig>(context.descriptors, "TailGunnerBot", 0x00d17e50));
        add(construct_descriptor<AAGunnerBotConfig>(context.descriptors, "AAGunnerBot", 0x00d17edc));
        add(construct_descriptor<PilotBotConfig>(context.descriptors, "PilotBot", 0x00d17de0));
        add(construct_descriptor<ArtillerySubDirectorBotConfig>(context.descriptors, "ArtillerySubDirectorBot", 0x00d17f20));
        add(construct_descriptor<ArtilleryGunnerBotConfig>(context.descriptors, "ArtilleryGunnerBot", 0x00d17fc4));
        add(construct_descriptor<TorpedoBotConfig>(context.descriptors, "TorpedoBot", 0x00d17ff8));
        add(construct_descriptor<DepthChargeBotConfig>(context.descriptors, "DepthChargeBot", 0x00d18038));
        //009DBFE0 publishes Navigator before registration. Other aliases are all
        //looked up only after every registration (duplicates keep older entries).
        aliases.navigator_f8a688 = construct_descriptor<NavigatorBotConfig>(
            context.descriptors, "NavigatorBot", 0x00d217dc);
        add(aliases.navigator_f8a688);
        aliases.pilot_f8a30c = find_robot_config_00900af0(registry, "PilotBot");
        aliases.tail_gunner_e199a0 = find_robot_config_00900af0(registry, "TailGunnerBot");
        aliases.aa_flak_e1999c = find_robot_config_00900af0(registry, "AAFlakBot");
        aliases.aa_gunner_e19998 = find_robot_config_00900af0(registry, "AAGunnerBot");
        aliases.artillery_subdirector_e19994 = find_robot_config_00900af0(registry, "ArtillerySubDirectorBot");
        aliases.artillery_gunner_e19990 = find_robot_config_00900af0(registry, "ArtilleryGunnerBot");
        aliases.torpedo_e1998c = find_robot_config_00900af0(registry, "TorpedoBot");
        aliases.depth_charge_e19988 = find_robot_config_00900af0(registry, "DepthChargeBot");
    }
    lua.close_storage_archive_00b65e80();
    return true;
}
} // namespace bsp
