#include "bsp/game_tuning_singleton.hpp"

#include <cmath>

namespace bsp {

// Load order of 007E2A20, one row per store into the object. Generated from the
// listing; see docs/GAME_TUNING_SINGLETON.md for how the scopes were resolved.
const GameTuningKey kGameTuningKeys[kGameTuningKeyCount] = {
    {"CloseToCameraDist", 0x004, GameTuningValueKind::Number, 0.0f, 0x007e2bed},
    {"MoveDetailLODError", 0x008, GameTuningValueKind::Number, 0.0f, 0x007e2c26},
    {"MoveDetailRadius", 0x00C, GameTuningValueKind::Number, 0.0f, 0x007e2c5f},
    {"DeathModeChances/Explosion", 0x010, GameTuningValueKind::Number, 0.0f, 0x007e428c},
    {"DeathModeChances/Explosion_delayed", 0x014, GameTuningValueKind::Number, 0.0f, 0x007e42c7},
    {"DeathModeChances/Spinning", 0x018, GameTuningValueKind::Number, 0.0f, 0x007e433d},
    {"DeathModeChances/Powerloss", 0x01C, GameTuningValueKind::Number, 0.0f, 0x007e4302},
    {"DeathModeChances/Explodetoparts", 0x020, GameTuningValueKind::Number, 0.0f, 0x007e4251},
    {"Sound/WindVolMinSpdRatio", 0x024, GameTuningValueKind::Number, 0.0f, 0x007e6811},
    {"Sound/WindVolMaxSpdRatio", 0x028, GameTuningValueKind::Number, 0.0f, 0x007e684f},
    {"Sound/WindPitchMinSpdRatio", 0x02C, GameTuningValueKind::Number, 0.0f, 0x007e688d},
    {"Sound/WindPitchMaxSpdRatio", 0x030, GameTuningValueKind::Number, 0.0f, 0x007e68cb},
    {"Sound/WindPitchMin", 0x034, GameTuningValueKind::Number, 0.0f, 0x007e6909},
    {"Sound/WindPitchMax", 0x038, GameTuningValueKind::Number, 0.0f, 0x007e6947},
    {"Sound/FallVolMinSpdRatio", 0x03C, GameTuningValueKind::Number, 0.0f, 0x007e6985},
    {"Sound/FallVolMaxSpdRatio", 0x040, GameTuningValueKind::Number, 0.0f, 0x007e69c3},
    {"Sound/FallPitchMinSpdRatio", 0x044, GameTuningValueKind::Number, 0.0f, 0x007e6a01},
    {"Sound/FallPitchMaxSpdRatio", 0x048, GameTuningValueKind::Number, 0.0f, 0x007e6a3f},
    {"Sound/FallPitchMin", 0x04C, GameTuningValueKind::Number, 0.0f, 0x007e6a7d},
    {"Sound/FallPitchMax", 0x050, GameTuningValueKind::Number, 0.0f, 0x007e6abb},
    {"PlaneCamera/ZRotMul", 0x054, GameTuningValueKind::Number, 0.0f, 0x007e43b6},
    {"PlaneCamera/ZRotSmoothRate", 0x058, GameTuningValueKind::Number, 0.0f, 0x007e43f1},
    {"PlaneCamera/XDistMul", 0x05C, GameTuningValueKind::Number, 0.0f, 0x007e442c},
    {"PlaneCamera/XDistSmoothRate", 0x060, GameTuningValueKind::Number, 0.0f, 0x007e4467},
    {"PlaneCamera/YDistMul", 0x064, GameTuningValueKind::Number, 0.0f, 0x007e44a2},
    {"PlaneCamera/YDistSmoothRate", 0x068, GameTuningValueKind::Number, 0.0f, 0x007e44dd},
    {"PlaneCamera/YDistRollMul", 0x06C, GameTuningValueKind::Number, 0.0f, 0x007e4518},
    {"PlaneCamera/YDistRollSmoothRate", 0x070, GameTuningValueKind::Number, 0.0f, 0x007e4553},
    {"PlaneCamera/ZDistMul", 0x074, GameTuningValueKind::Number, 0.0f, 0x007e458e},
    {"PlaneCamera/ZDistSmoothRate", 0x078, GameTuningValueKind::Number, 0.0f, 0x007e45c9},
    {"PlaneCamera/ShipYardDist", 0x07C, GameTuningValueKind::Number, 0.0f, 0x007e4604},
    {"PlaneCamera/LookAroundSmoothRate", 0x080, GameTuningValueKind::NumberOrDefault, 10.0f, 0x007e4649},
    {"PlaneCamera/CockpitFOVMul", 0x084, GameTuningValueKind::NumberOrDefault, 0.65f, 0x007e4691},
    {"PlaneCamera/CockpitSmooth", 0x088, GameTuningValueKind::NumberOrDefault, 0.5f, 0x007e46d9},
    {"PlaneCamera/CockpitYawTurn", 0x08C, GameTuningValueKind::NumberOrDefault, 0.1f, 0x007e4721},
    {"PlaneCamera/CockpitPitchTurn", 0x090, GameTuningValueKind::NumberOrDefault, 0.1f, 0x007e4769},
    {"PlaneCamera/CockpitRollTurn", 0x094, GameTuningValueKind::NumberOrDefault, 0.05f, 0x007e47b1},
    {"PlaneCamera/CockpitRollHTurn", 0x098, GameTuningValueKind::NumberOrDefault, 0.05f, 0x007e47f9},
    {"PlaneCamera/CockpitRollVTurn", 0x09C, GameTuningValueKind::NumberOrDefault, 0.05f, 0x007e4841},
    {"PlaneCamera/CockpitViewHMax", 0x0A0, GameTuningValueKind::NumberOrDefault, 1.5f, 0x007e4889},
    {"PlaneCamera/CockpitViewVMax", 0x0A4, GameTuningValueKind::NumberOrDefault, 1.0f, 0x007e4915},
    {"PlaneCamera/CockpitViewVMin", 0x0A8, GameTuningValueKind::NumberOrDefault, 0.7f, 0x007e48d1},
    {"PlaneCamera/CockpitMaxHeadMoveDist", 0x0AC, GameTuningValueKind::NumberOrDefault, 0.08f, 0x007e495d},
    {"PlaneCamera/CockpitGunfireEffectSize", 0x0B0, GameTuningValueKind::NumberOrDefault, 0.008f, 0x007e49a5},
    {"PlaneCamera/CockpitGunfireEffectTime", 0x0B4, GameTuningValueKind::NumberOrDefault, 0.1f, 0x007e49ed},
    {"PlaneCamera/FOVMinSpeed", 0x0B8, GameTuningValueKind::Number, 0.0f, 0x007e4a2b},
    {"PlaneCamera/FOVMaxSpeed", 0x0BC, GameTuningValueKind::Number, 0.0f, 0x007e4a69},
    {"PlaneCamera/FOVMinSpdMul", 0x0C0, GameTuningValueKind::Number, 0.0f, 0x007e4aa7},
    {"PlaneCamera/FOVMaxSpdMul", 0x0C4, GameTuningValueKind::Number, 0.0f, 0x007e4ae5},
    {"PlaneCamera/FOVMinAccel", 0x0C8, GameTuningValueKind::Number, 0.0f, 0x007e4b23},
    {"PlaneCamera/FOVMaxAccel", 0x0CC, GameTuningValueKind::Number, 0.0f, 0x007e4b61},
    {"PlaneCamera/FOVAccelMul", 0x0D0, GameTuningValueKind::Number, 0.0f, 0x007e4b9f},
    {"PlaneCamera/TurboMotionBlurMinSpeed", 0x0D8, GameTuningValueKind::NumberOrDefault, 80.0f, 0x007e4be7},
    {"PlaneCamera/TurboMotionBlurMaxSpeed", 0x0DC, GameTuningValueKind::NumberOrDefault, 100.0f, 0x007e4c2f},
    {"PlaneCamera/TurboMotionBlurMaxBlur", 0x0E0, GameTuningValueKind::NumberOrDefault, 0.1f, 0x007e4c77},
    {"BombCamera/MinCameraAlt", 0x0E4, GameTuningValueKind::Number, 0.0f, 0x007e4cf3},
    {"BombCamera/CameraPosSmooth", 0x0E8, GameTuningValueKind::Number, 0.0f, 0x007e4d31},
    {"BombCamera/CameraPosSmooth", 0x0EC, GameTuningValueKind::Number, 0.0f, 0x007e4d6c},
    {"BombCamera/CamVelBlenderAcceleration", 0x0F0, GameTuningValueKind::Number, 0.0f, 0x007e4da7},
    {"BombCamera/MaxCamVelBlender", 0x0F4, GameTuningValueKind::Number, 0.0f, 0x007e4de5},
    {"BombCamera/TorpedoNearAlt/2", 0x0F8, GameTuningValueKind::Number, 0.0f, 0x007e4ea4},
    {"BombCamera/TorpedoNearAlt/1", 0x0FC, GameTuningValueKind::Number, 0.0f, 0x007e4e3c},
    {"BombCamera/TorpedoFollowDistNear", 0x100, GameTuningValueKind::Number, 0.0f, 0x007e4ef0},
    {"BombCamera/TorpedoFollowDistFar", 0x104, GameTuningValueKind::Number, 0.0f, 0x007e4f2b},
    {"BombCamera/RocketNearAlt/2", 0x108, GameTuningValueKind::Number, 0.0f, 0x007e4fed},
    {"BombCamera/RocketNearAlt/1", 0x10C, GameTuningValueKind::Number, 0.0f, 0x007e4f82},
    {"BombCamera/RocketFollowDistNear", 0x110, GameTuningValueKind::Number, 0.0f, 0x007e503f},
    {"BombCamera/RocketFollowDistFar", 0x114, GameTuningValueKind::Number, 0.0f, 0x007e507d},
    {"BombCamera/BombNearAlt/2", 0x118, GameTuningValueKind::Number, 0.0f, 0x007e513f},
    {"BombCamera/BombNearAlt/1", 0x11C, GameTuningValueKind::Number, 0.0f, 0x007e50d4},
    {"BombCamera/BombFollowDistNear", 0x120, GameTuningValueKind::Number, 0.0f, 0x007e5191},
    {"BombCamera/BombFollowDistFar", 0x124, GameTuningValueKind::Number, 0.0f, 0x007e51cf},
    {"BombCamera/BulletNearAlt/2", 0x128, GameTuningValueKind::Number, 0.0f, 0x007e5291},
    {"BombCamera/BulletNearAlt/1", 0x12C, GameTuningValueKind::Number, 0.0f, 0x007e5226},
    {"BombCamera/BulletFollowDistNear", 0x130, GameTuningValueKind::Number, 0.0f, 0x007e52e3},
    {"BombCamera/BulletFollowDistFar", 0x134, GameTuningValueKind::Number, 0.0f, 0x007e5321},
    {"BombCamera/FinalDist", 0x138, GameTuningValueKind::Number, 0.0f, 0x007e535f},
    {"BombCamera/BombSubDistLimit", 0x13C, GameTuningValueKind::Number, 0.0f, 0x007e539d},
    {"BombCamera/BombSubDistMin", 0x140, GameTuningValueKind::Number, 0.0f, 0x007e53db},
    {"BombCamera/BombSubDistMul", 0x144, GameTuningValueKind::Number, 0.0f, 0x007e5419},
    {"BombCamera/TorpedoSubDist", 0x148, GameTuningValueKind::Number, 0.0f, 0x007e5457},
    {"BombCamera/TorpedoSubDist2", 0x14C, GameTuningValueKind::Number, 0.0f, 0x007e5495},
    {"BombCamera/TorpedoSubAngle", 0x150, GameTuningValueKind::Number, 0.0f, 0x007e54d3},
    {"BombCamera/TorpedoSubAngle2", 0x154, GameTuningValueKind::Number, 0.0f, 0x007e5511},
    {"MultiPlayer/SyncSendMul", 0x158, GameTuningValueKind::NumberOrDefault, 1.0f, 0x007e5593},
    {"MultiPlayer/SyncUploadLimitKBitPerSec", 0x15C, GameTuningValueKind::NumberOrDefault, 1024.0f, 0x007e55db},
    {"Rotor/SpeedBase", 0x160, GameTuningValueKind::Number, 0.0f, 0x007e5f09},
    {"Rotor/StillMultiplier", 0x164, GameTuningValueKind::Number, 0.0f, 0x007e5f47},
    {"Rotor/SpeedRandom", 0x168, GameTuningValueKind::Number, 0.0f, 0x007e5f85},
    {"Rotor/StillOnlySpeed", 0x16C, GameTuningValueKind::Number, 0.0f, 0x007e5fc3},
    {"Rotor/BlurredOnlySpeed", 0x170, GameTuningValueKind::Number, 0.0f, 0x007e6001},
    {"Rotor/IdlePowerSpeed", 0x174, GameTuningValueKind::Number, 0.0f, 0x007e603f},
    {"Rotor/MaxPowerSpeed", 0x178, GameTuningValueKind::Number, 0.0f, 0x007e607d},
    {"Rotor/RotorSpeedChange", 0x17C, GameTuningValueKind::Number, 0.0f, 0x007e60bb},
    {"AirField/TurnMultiplier", 0x180, GameTuningValueKind::Number, 0.0f, 0x007e6137},
    {"AirField/MoveSpd", 0x184, GameTuningValueKind::Number, 0.0f, 0x007e61b3},
    {"AirField/MinTurnSpd", 0x188, GameTuningValueKind::Number, 0.0f, 0x007e6175},
    {"AirField/PlayerControlSpd", 0x18C, GameTuningValueKind::Number, 0.0f, 0x007e61f1},
    {"AirField/PlaneSendInterval", 0x190, GameTuningValueKind::Number, 0.0f, 0x007e622f},
    {"CameraShake/RollPitchMult", 0x194, GameTuningValueKind::Number, 0.0f, 0x007e5657},
    {"CameraShake/PowerMult", 0x198, GameTuningValueKind::Number, 0.0f, 0x007e5695},
    {"CameraShake/SpeedMult", 0x19C, GameTuningValueKind::Number, 0.0f, 0x007e56d3},
    {"CameraShake/Limit", 0x1A0, GameTuningValueKind::Number, 0.0f, 0x007e5711},
    {"CameraShake/Ratio", 0x1A4, GameTuningValueKind::Number, 0.0f, 0x007e574f},
    {"CameraShake/PauseLenMin", 0x1A8, GameTuningValueKind::Number, 0.0f, 0x007e578d},
    {"CameraShake/PauseLenMax", 0x1AC, GameTuningValueKind::Number, 0.0f, 0x007e57cb},
    {"CameraShake/ShakeLenMin", 0x1B0, GameTuningValueKind::Number, 0.0f, 0x007e5809},
    {"CameraShake/ShakeLenMax", 0x1B4, GameTuningValueKind::Number, 0.0f, 0x007e5847},
    {"CameraShake/RandomLenFactor", 0x1B8, GameTuningValueKind::Number, 0.0f, 0x007e5885},
    {"CameraShake/ForceShakeLimit", 0x1BC, GameTuningValueKind::Number, 0.0f, 0x007e58c3},
    {"Wanderer/SpeedRange/1", 0x1C0, GameTuningValueKind::Number, 0.0f, 0x007e5958},
    {"Wanderer/SpeedRange/2", 0x1C4, GameTuningValueKind::Number, 0.0f, 0x007e59c3},
    {"Wanderer/RollChangeChance", 0x1C8, GameTuningValueKind::Number, 0.0f, 0x007e5a91},
    {"Wanderer/RollChangeMax", 0x1CC, GameTuningValueKind::Number, 0.0f, 0x007e5a15},
    {"Wanderer/RollChangeDecay", 0x1D0, GameTuningValueKind::Number, 0.0f, 0x007e5a53},
    {"Wanderer/RollChangeSpeed", 0x1D4, GameTuningValueKind::Number, 0.0f, 0x007e5acf},
    {"Wanderer/TimeRange/1", 0x1D8, GameTuningValueKind::Number, 0.0f, 0x007e5b26},
    {"Wanderer/TimeRange/2", 0x1DC, GameTuningValueKind::Number, 0.0f, 0x007e5b91},
    {"Wanderer/OffsetMax", 0x1E0, GameTuningValueKind::Number, 0.0f, 0x007e5c9d},
    {"Wanderer/ChangeMul", 0x1E4, GameTuningValueKind::Number, 0.0f, 0x007e5be3},
    {"Wanderer/AccelMax", 0x1E8, GameTuningValueKind::Number, 0.0f, 0x007e5c21},
    {"Wanderer/SpeedMax", 0x1EC, GameTuningValueKind::Number, 0.0f, 0x007e5c5f},
    {"Wanderer/AccelDecayTime", 0x1F0, GameTuningValueKind::Number, 0.0f, 0x007e5cdb},
    {"Wanderer/SpeedDecayTime", 0x1F4, GameTuningValueKind::Number, 0.0f, 0x007e5d19},
    {"Wanderer/OffsetDecayTime", 0x1F8, GameTuningValueKind::Number, 0.0f, 0x007e5d57},
    {"Wanderer/SmallPlaneDecalMul", 0x1FC, GameTuningValueKind::Number, 0.0f, 0x007e5d95},
    {"Wanderer/SmallPlaneRollDecayMul", 0x200, GameTuningValueKind::Number, 0.0f, 0x007e5dd3},
    {"Wanderer/SmallPlaneAccelMul", 0x204, GameTuningValueKind::Number, 0.0f, 0x007e5e11},
    {"Wanderer/SmallPlaneTimeMul", 0x208, GameTuningValueKind::Number, 0.0f, 0x007e5e4f},
    {"Wanderer/SmallPlaneOffsetMul", 0x20C, GameTuningValueKind::Number, 0.0f, 0x007e5e8d},
    {"Dynamics/Ceiling", 0x210, GameTuningValueKind::Number, 0.0f, 0x007e2d09},
    {"Dynamics/CeilingForce", 0x214, GameTuningValueKind::Number, 0.0f, 0x007e2d3d},
    {"Dynamics/RotationLimit", 0x218, GameTuningValueKind::Integer, 0.0f, 0x007e2d75},
    {"Dynamics/RotationFactors/A", 0x21C, GameTuningValueKind::Number, 0.0f, 0x007e3cb9},
    {"Dynamics/RotationFactors/B", 0x220, GameTuningValueKind::Number, 0.0f, 0x007e3cfb},
    {"Dynamics/RotationFactors/C", 0x224, GameTuningValueKind::Number, 0.0f, 0x007e3d3d},
    {"Dynamics/DragFuncPower", 0x228, GameTuningValueKind::Number, 0.0f, 0x007e2dad},
    {"Dynamics/SpdMultipliers/StallRangeMin", 0x22C, GameTuningValueKind::Number, 0.0f, 0x007e3daa},
    {"Dynamics/SpdMultipliers/StallRangeMax", 0x230, GameTuningValueKind::Number, 0.0f, 0x007e3de9},
    {"Dynamics/SpdMultipliers/StallOffPitch", 0x234, GameTuningValueKind::Number, 0.0f, 0x007e3e28},
    {"Dynamics/SpdMultipliers/StallOnPitch", 0x238, GameTuningValueKind::Number, 0.0f, 0x007e3e67},
    {"Dynamics/SpdMultipliers/ControlRangeMin", 0x23C, GameTuningValueKind::Number, 0.0f, 0x007e3ea6},
    {"Dynamics/SpdMultipliers/ControlRangeMax", 0x240, GameTuningValueKind::Number, 0.0f, 0x007e3ee5},
    {"Dynamics/SpdMultipliers/DragRangeMin", 0x244, GameTuningValueKind::Number, 0.0f, 0x007e3f24},
    {"Dynamics/SpdMultipliers/DragRangeMax", 0x248, GameTuningValueKind::Number, 0.0f, 0x007e3f63},
    {"Dynamics/SpdMultipliers/LevelFlight", 0x24C, GameTuningValueKind::Number, 0.0f, 0x007e3fa2},
    {"Dynamics/DeadMeat/RotationMin", 0x250, GameTuningValueKind::Number, 0.0f, 0x007e30e2},
    {"Dynamics/DeadMeat/RotationMin", 0x254, GameTuningValueKind::Number, 0.0f, 0x007e311b},
    {"Dynamics/DeadMeat/SpinRollSpd", 0x258, GameTuningValueKind::Number, 0.0f, 0x007e3400},
    {"Dynamics/DeadMeat/RollMulTime", 0x25C, GameTuningValueKind::Number, 0.0f, 0x007e318d},
    {"Dynamics/DeadMeat/RollMul", 0x260, GameTuningValueKind::Number, 0.0f, 0x007e3154},
    {"Dynamics/DeadMeat/LostDragTime", 0x264, GameTuningValueKind::Number, 0.0f, 0x007e31c6},
    {"Dynamics/DeadMeat/ExtraGravityMul", 0x268, GameTuningValueKind::Number, 0.0f, 0x007e31ff},
    {"Dynamics/DeadMeat/SpinStallMul", 0x26C, GameTuningValueKind::Number, 0.0f, 0x007e3238},
    {"Dynamics/DeadMeat/SpinStallMulTime", 0x270, GameTuningValueKind::Number, 0.0f, 0x007e3271},
    {"Dynamics/DeadMeat/SpinStallMulOnPitch", 0x274, GameTuningValueKind::Number, 0.0f, 0x007e32aa},
    {"Dynamics/DeadMeat/SpinStallMulOffPitch", 0x278, GameTuningValueKind::Number, 0.0f, 0x007e32e3},
    {"Dynamics/DeadMeat/StallMul", 0x27C, GameTuningValueKind::Number, 0.0f, 0x007e331c},
    {"Dynamics/DeadMeat/StallMulTime", 0x280, GameTuningValueKind::Number, 0.0f, 0x007e3355},
    {"Dynamics/DeadMeat/StallMulOnPitch", 0x284, GameTuningValueKind::Number, 0.0f, 0x007e338e},
    {"Dynamics/DeadMeat/StallMulOffPitch", 0x288, GameTuningValueKind::Number, 0.0f, 0x007e33c7},
    {"Dynamics/WheelFriction", 0x290, GameTuningValueKind::Number, 0.0f, 0x007e2de5},
    {"Dynamics/WheelFrictionSpeed/1", 0x294, GameTuningValueKind::Number, 0.0f, 0x007e2e33},
    {"Dynamics/WheelFrictionSpeed/2", 0x298, GameTuningValueKind::Number, 0.0f, 0x007e2e92},
    {"Dynamics/WheelFrictionAccel/1", 0x29C, GameTuningValueKind::Number, 0.0f, 0x007e2ef1},
    {"Dynamics/WheelFrictionAccel/2", 0x2A0, GameTuningValueKind::Number, 0.0f, 0x007e2f50},
    {"Dynamics/RunwaySmoothStrength", 0x2A4, GameTuningValueKind::Number, 0.0f, 0x007e2f99},
    {"Dynamics/RunwayYawTurnSpdLimit/1", 0x2A8, GameTuningValueKind::Number, 0.0f, 0x007e2fe7},
    {"Dynamics/RunwayYawTurnSpdLimit/1", 0x2AC, GameTuningValueKind::Number, 0.0f, 0x007e3046},
    {"Dynamics/RunwayYawTurnSpdMul", 0x2B0, GameTuningValueKind::Number, 0.0f, 0x007e308f},
    {"Dynamics/Water/MaxVSpd", 0x2B4, GameTuningValueKind::Number, 0.0f, 0x007e36a5},
    {"Dynamics/Water/MaxDownPitch", 0x2B8, GameTuningValueKind::Number, 0.0f, 0x007e36e4},
    {"Dynamics/Water/MaxUpPitch", 0x2BC, GameTuningValueKind::Number, 0.0f, 0x007e3723},
    {"Dynamics/Water/MaxRoll", 0x2C0, GameTuningValueKind::Number, 0.0f, 0x007e3762},
    {"Dynamics/Water/YawControlFactor", 0x2C4, GameTuningValueKind::Number, 0.0f, 0x007e37a1},
    {"Dynamics/Water/NormalYawControlSpd", 0x2C8, GameTuningValueKind::Number, 0.0f, 0x007e37e0},
    {"Dynamics/Water/MaxYawControlSpd", 0x2CC, GameTuningValueKind::Number, 0.0f, 0x007e381f},
    {"Dynamics/Water/MaxYawControl", 0x2D0, GameTuningValueKind::Number, 0.0f, 0x007e385e},
    {"Dynamics/Water/MaxDepth", 0x2D4, GameTuningValueKind::Number, 0.0f, 0x007e3968},
    {"Dynamics/Water/LiftDepthRatio", 0x2D8, GameTuningValueKind::Number, 0.0f, 0x007e38ea},
    {"Dynamics/Water/LiftMax", 0x2DC, GameTuningValueKind::Number, 0.0f, 0x007e3929},
    {"Dynamics/Water/DecelSpeed", 0x2E0, GameTuningValueKind::Number, 0.0f, 0x007e389d},
    {"Dynamics/Water/SideDragRatio", 0x2E4, GameTuningValueKind::Number, 0.0f, 0x007e39a7},
    {"Dynamics/Water/MaxSideDrag", 0x2E8, GameTuningValueKind::Number, 0.0f, 0x007e39e6},
    {"Dynamics/Water/MaxCtrlAngle", 0x2EC, GameTuningValueKind::Number, 0.0f, 0x007e3bde},
    {"Dynamics/Water/MinCtrlAngle", 0x2F0, GameTuningValueKind::Number, 0.0f, 0x007e3c33},
    {"Dynamics/Water/TakeOffMaxLength", 0x2F4, GameTuningValueKind::Number, 0.0f, 0x007e3a25},
    {"Dynamics/Water/TakeOffMinLength", 0x2F8, GameTuningValueKind::Number, 0.0f, 0x007e3a64},
    {"Dynamics/Water/MinDragSpd", 0x2FC, GameTuningValueKind::Number, 0.0f, 0x007e3aa3},
    {"Dynamics/Water/LiftBeginDiveMul", 0x300, GameTuningValueKind::Number, 0.0f, 0x007e3ae2},
    {"Dynamics/Water/LiftRotateMax", 0x304, GameTuningValueKind::Number, 0.0f, 0x007e3b21},
    {"Dynamics/Water/LiftRotateDepthRatio", 0x308, GameTuningValueKind::Number, 0.0f, 0x007e3b60},
    {"Dynamics/Water/LiftRotateAngleRatio", 0x30C, GameTuningValueKind::Number, 0.0f, 0x007e3b9f},
    {"Dynamics/MaxDragSpdMul", 0x310, GameTuningValueKind::Number, 0.0f, 0x007e3449},
    {"Dynamics/MinDragSpdMul", 0x314, GameTuningValueKind::Number, 0.0f, 0x007e3481},
    {"Dynamics/MaxDragPitch", 0x318, GameTuningValueKind::Number, 0.0f, 0x007e34bc},
    {"Dynamics/AccelCheatMul", 0x31C, GameTuningValueKind::Number, 0.0f, 0x007e34fa},
    {"Dynamics/AccelCheatMulMul", 0x320, GameTuningValueKind::Number, 0.0f, 0x007e3538},
    {"Dynamics/AccelCheatFallMul", 0x324, GameTuningValueKind::Number, 0.0f, 0x007e3576},
    {"Dynamics/AccelCheatFallPitchRange/1", 0x328, GameTuningValueKind::Number, 0.0f, 0x007e35cd},
    {"Dynamics/AccelCheatFallPitchRange/2", 0x32C, GameTuningValueKind::Number, 0.0f, 0x007e3638},
    {"Dynamics/SpdMultipliers/TurboMultiplier", 0x330, GameTuningValueKind::Number, 0.0f, 0x007e3fe1},
    {"Dynamics/SpdMultipliers/NewTravelSpeedMul", 0x334, GameTuningValueKind::Number, 0.0f, 0x007e4020},
    {"Dynamics/SpdMultipliers/DiveBombSlowMul", 0x338, GameTuningValueKind::Number, 0.0f, 0x007e405f},
    {"Dynamics/SpdMultipliers/TorpedoBombSlowMul", 0x33C, GameTuningValueKind::Number, 0.0f, 0x007e409e},
    {"Dynamics/SpdMultipliers/DepthChargeSlowMul", 0x340, GameTuningValueKind::Number, 0.0f, 0x007e40dd},
    {"Dynamics/SpdMultipliers/LevelBombSlowMul", 0x344, GameTuningValueKind::Number, 0.0f, 0x007e411c},
    {"Retreat/ExitDist", 0x348, GameTuningValueKind::Number, 0.0f, 0x007e6bfd},
    {"Retreat/ExitTime", 0x34C, GameTuningValueKind::Number, 0.0f, 0x007e6c3e},
    {"Retreat/WarningRepeatTime", 0x350, GameTuningValueKind::Number, 0.0f, 0x007e6c7f},
    {"ParatrooperDrop/MinAltitude", 0x354, GameTuningValueKind::Number, 0.0f, 0x007e6d01},
    {"ParatrooperDrop/MaxAltitude", 0x358, GameTuningValueKind::Number, 0.0f, 0x007e6d42},
    {"Pilot/MoveTo/SmallPlaneTravelAlt", 0x35C, GameTuningValueKind::Number, 0.0f, 0x007e82f2},
    {"Pilot/MoveTo/LargePlaneTravelAlt", 0x360, GameTuningValueKind::Number, 0.0f, 0x007e8333},
    {"Pilot/MoveTo/TravelAltRandom", 0x364, GameTuningValueKind::Number, 0.0f, 0x007e8374},
    {"Pilot/MoveTo/FollowDist/1", 0x368, GameTuningValueKind::Number, 0.0f, 0x007e83ce},
    {"Pilot/MoveTo/FollowDist/2", 0x36C, GameTuningValueKind::Number, 0.0f, 0x007e843c},
    {"Pilot/MoveTo/ClosingDist", 0x370, GameTuningValueKind::Number, 0.0f, 0x007e8491},
    {"Pilot/MoveTo/SwitchNextPointTime", 0x374, GameTuningValueKind::Number, 0.0f, 0x007e8513},
    {"Pilot/MoveTo/CircleAltDiff", 0x378, GameTuningValueKind::Number, 0.0f, 0x007e84d2},
    {"Pilot/MoveTo/ReferenceSpeed", 0x37C, GameTuningValueKind::Number, 0.0f, 0x007e8554},
    {"Pilot/Follow/FollowedPointDist", 0x380, GameTuningValueKind::Number, 0.0f, 0x007e8aa8},
    {"Pilot/Follow/LeaderFollowAlt", 0x384, GameTuningValueKind::Number, 0.0f, 0x007e8ae9},
    {"Pilot/Follow/SafeAlt", 0x388, GameTuningValueKind::Number, 0.0f, 0x007e8b2a},
    {"Pilot/Follow/SmallPlaneTurnMul", 0x38C, GameTuningValueKind::Number, 0.0f, 0x007e870c},
    {"Pilot/Follow/LargePlaneTurnMul", 0x390, GameTuningValueKind::Number, 0.0f, 0x007e874d},
    {"Pilot/Follow/GoodPositionDir", 0x394, GameTuningValueKind::Number, 0.0f, 0x007e87cf},
    {"Pilot/Follow/GoodPositionDist", 0x398, GameTuningValueKind::Number, 0.0f, 0x007e878e},
    {"Pilot/Follow/GoodPositionSpdTreshold", 0x39C, GameTuningValueKind::Number, 0.0f, 0x007e8810},
    {"Pilot/Follow/GoodPositionSpdDiff", 0x3A0, GameTuningValueKind::Number, 0.0f, 0x007e8851},
    {"Pilot/Follow/MaxFollowSpdTargetDir", 0x3A4, GameTuningValueKind::Number, 0.0f, 0x007e8892},
    {"Pilot/Follow/MinFollowSpdTargetDir", 0x3A8, GameTuningValueKind::Number, 0.0f, 0x007e891b},
    {"Pilot/Follow/DontWaitForHdgDiff", 0x3AC, GameTuningValueKind::Number, 0.0f, 0x007e89a7},
    {"Pilot/Follow/WaitForHdgDiff", 0x3B0, GameTuningValueKind::Number, 0.0f, 0x007e89e5},
    {"Pilot/Follow/NearbyDist", 0x3B4, GameTuningValueKind::Number, 0.0f, 0x007e8a26},
    {"Pilot/Follow/TightTurn", 0x3B8, GameTuningValueKind::Number, 0.0f, 0x007e8a67},
    {"Pilot/Follow/LeaderHeadingSpdTime/1", 0x3BC, GameTuningValueKind::Number, 0.0f, 0x007e8b84},
    {"Pilot/Follow/LeaderHeadingSpdTime/2", 0x3C0, GameTuningValueKind::Number, 0.0f, 0x007e8bef},
    {"Pilot/Follow/LeaderHeadingSpdDist/1", 0x3C4, GameTuningValueKind::Number, 0.0f, 0x007e8c5a},
    {"Pilot/Follow/LeaderHeadingSpdDist/2", 0x3C8, GameTuningValueKind::Number, 0.0f, 0x007e8cc5},
    {"Pilot/Follow/SmallPlaneDisplacement", 0x3D0, GameTuningValueKind::NumberTriple, 0.0f, 0x007e85db},
    {"Pilot/Follow/SmallPlaneDisplacement", 0x3D4, GameTuningValueKind::NumberTriple, 0.0f, 0x007e85db},
    {"Pilot/Follow/SmallPlaneDisplacement", 0x3D8, GameTuningValueKind::NumberTriple, 0.0f, 0x007e85db},
    {"Pilot/Follow/BomberDisplacement", 0x3DC, GameTuningValueKind::NumberTriple, 0.0f, 0x007e8635},
    {"Pilot/Follow/BomberDisplacement", 0x3E0, GameTuningValueKind::NumberTriple, 0.0f, 0x007e8635},
    {"Pilot/Follow/BomberDisplacement", 0x3E4, GameTuningValueKind::NumberTriple, 0.0f, 0x007e8635},
    {"Pilot/Follow/SymmetricalPosition", 0x3E8, GameTuningValueKind::Boolean, 0.0f, 0x007e868a},
    {"Pilot/Follow/SymmetricalAltitude", 0x3E9, GameTuningValueKind::Boolean, 0.0f, 0x007e86cb},
    {"Pilot/Follow/yf_hdg_rad", 0x3EC, GameTuningValueKind::Number, 0.0f, 0x007e8d17},
    {"Pilot/Follow/yf_yawV_radPerSec", 0x3F0, GameTuningValueKind::Number, 0.0f, 0x007e8d58},
    {"Pilot/Follow/yf_sidepos_meter", 0x3F4, GameTuningValueKind::Number, 0.0f, 0x007e8d99},
    {"Pilot/Follow/yf_sidedir", 0x3F8, GameTuningValueKind::Number, 0.0f, 0x007e8dda},
    {"Pilot/Follow/pf_pitch_rad", 0x3FC, GameTuningValueKind::Number, 0.0f, 0x007e8e1b},
    {"Pilot/Follow/pf_pitchV_radPerSec", 0x400, GameTuningValueKind::Number, 0.0f, 0x007e8e5c},
    {"Pilot/Follow/pf_vertpos_meter", 0x404, GameTuningValueKind::Number, 0.0f, 0x007e8e9d},
    {"Pilot/Follow/pf_vertdir", 0x408, GameTuningValueKind::Number, 0.0f, 0x007e8ede},
    {"Pilot/Follow/rf_roll_rad", 0x40C, GameTuningValueKind::Number, 0.0f, 0x007e8f1f},
    {"Pilot/Follow/rf_rollV_radPerSec", 0x410, GameTuningValueKind::Number, 0.0f, 0x007e8f60},
    {"Pilot/Follow/rf_hdg_rad", 0x414, GameTuningValueKind::Number, 0.0f, 0x007e8fa1},
    {"Pilot/Follow/rf_hdgV_radPerSec", 0x418, GameTuningValueKind::Number, 0.0f, 0x007e8fe2},
    {"Pilot/Follow/pwr_back_meter", 0x41C, GameTuningValueKind::Number, 0.0f, 0x007e9023},
    {"Pilot/Follow/pwr_spd_meterPerSec", 0x420, GameTuningValueKind::Number, 0.0f, 0x007e9064},
    {"Pilot/CloseToShip/CruisingAlt", 0x424, GameTuningValueKind::Number, 0.0f, 0x007e9237},
    {"Pilot/CloseToShip/DropAlt", 0x428, GameTuningValueKind::Number, 0.0f, 0x007e9278},
    {"Pilot/CloseToShip/ReferenceSpeed", 0x42C, GameTuningValueKind::Number, 0.0f, 0x007e92b9},
    {"Pilot/Torpedo/CruisingAlt", 0x430, GameTuningValueKind::Number, 0.0f, 0x007e933b},
    {"Pilot/Torpedo/AttackDist", 0x434, GameTuningValueKind::Number, 0.0f, 0x007e937c},
    {"Pilot/Torpedo/SafeDist", 0x438, GameTuningValueKind::Number, 0.0f, 0x007e93bd},
    {"Pilot/Torpedo/MoveOnCruisingAlt", 0x43C, GameTuningValueKind::Boolean, 0.0f, 0x007e93fe},
    {"Pilot/Torpedo/ReferenceSpeed", 0x440, GameTuningValueKind::Number, 0.0f, 0x007e9448},
    {"Pilot/LevelBomb/CruisingAlt", 0x444, GameTuningValueKind::Number, 0.0f, 0x007e94ca},
    {"Pilot/LevelBomb/DropAlt", 0x448, GameTuningValueKind::Number, 0.0f, 0x007e950b},
    {"Pilot/LevelBomb/AttackDist", 0x44C, GameTuningValueKind::Number, 0.0f, 0x007e954c},
    {"Pilot/LevelBomb/SafeDist", 0x450, GameTuningValueKind::Number, 0.0f, 0x007e958d},
    {"Pilot/LevelBomb/MoveOnCruisingAlt", 0x454, GameTuningValueKind::Boolean, 0.0f, 0x007e95ce},
    {"Pilot/LevelBomb/ReferenceSpeed", 0x458, GameTuningValueKind::Number, 0.0f, 0x007e9618},
    {"Pilot/Kamikaze/RocketLike/CruisingAlt", 0x45C, GameTuningValueKind::NumberOrDefault, 1200.0f, 0x007e982c},
    {"Pilot/Kamikaze/RocketLike/DropDist", 0x460, GameTuningValueKind::NumberOrDefault, 2000.0f, 0x007e9875},
    {"Pilot/Kamikaze/RocketLike/AttackRange", 0x464, GameTuningValueKind::NumberOrDefault, 1400.0f, 0x007e96bf},
    {"Pilot/Kamikaze/RocketLike/AttackAlt", 0x468, GameTuningValueKind::NumberOrDefault, 500.0f, 0x007e9708},
    {"Pilot/Kamikaze/RocketLike/TurboRange", 0x46C, GameTuningValueKind::NumberOrDefault, 1000.0f, 0x007e9751},
    {"Pilot/Kamikaze/RocketLike/TurboAngle", 0x470, GameTuningValueKind::NumberOrDefault, 0.2f, 0x007e979a},
    {"Pilot/Kamikaze/RocketLike/ReferenceSpeed", 0x474, GameTuningValueKind::NumberOrDefault, 180.0f, 0x007e97e3},
    {"Pilot/Kamikaze/FighterLike/CruisingAlt", 0x478, GameTuningValueKind::NumberOrDefault, 1200.0f, 0x007e9a63},
    {"Pilot/Kamikaze/FighterLike/DropDist", 0x47C, GameTuningValueKind::NumberOrDefault, 1200.0f, 0x007e9aac},
    {"Pilot/Kamikaze/FighterLike/AttackRange", 0x480, GameTuningValueKind::NumberOrDefault, 1200.0f, 0x007e98fa},
    {"Pilot/Kamikaze/FighterLike/AttackAlt", 0x484, GameTuningValueKind::NumberOrDefault, 600.0f, 0x007e9943},
    {"Pilot/Kamikaze/FighterLike/TurboRange", 0x488, GameTuningValueKind::NumberOrDefault, 500.0f, 0x007e998c},
    {"Pilot/Kamikaze/FighterLike/TurboAngle", 0x48C, GameTuningValueKind::NumberOrDefault, 0.0f, 0x007e99d1},
    {"Pilot/Kamikaze/FighterLike/ReferenceSpeed", 0x490, GameTuningValueKind::NumberOrDefault, 100.0f, 0x007e9a1a},
    {"Pilot/DepthCharge/AimAltRange/1", 0x494, GameTuningValueKind::Number, 0.0f, 0x007e9b58},
    {"Pilot/DepthCharge/AimAltRange/2", 0x498, GameTuningValueKind::Number, 0.0f, 0x007e9bc3},
    {"Pilot/DepthCharge/ManeuverAltRange/1", 0x49C, GameTuningValueKind::Number, 0.0f, 0x007e9c2e},
    {"Pilot/DepthCharge/ManeuverAltRange/2", 0x4A0, GameTuningValueKind::Number, 0.0f, 0x007e9c99},
    {"Pilot/DepthCharge/CruisingAlt", 0x4A4, GameTuningValueKind::Number, 0.0f, 0x007e9ceb},
    {"Pilot/DepthCharge/FlyAboveDist", 0x4A8, GameTuningValueKind::Number, 0.0f, 0x007e9d2c},
    {"Pilot/DepthCharge/AttackDist", 0x4AC, GameTuningValueKind::Number, 0.0f, 0x007e9d6d},
    {"Pilot/DepthCharge/SafeDist", 0x4B0, GameTuningValueKind::Number, 0.0f, 0x007e9dae},
    {"Pilot/DepthCharge/MoveOnCruisingAlt", 0x4B4, GameTuningValueKind::Boolean, 0.0f, 0x007e9e30},
    {"Pilot/DepthCharge/ReferenceSpeed", 0x4B8, GameTuningValueKind::Number, 0.0f, 0x007e9def},
    {"Pilot/DepthCharge/TargetLostTime", 0x4BC, GameTuningValueKind::NumberOrDefault, 20.0f, 0x007e9e84},
    {"Pilot/DiveBomb/CruisingAlt", 0x4C0, GameTuningValueKind::Number, 0.0f, 0x007e9f47},
    {"Pilot/DiveBomb/AttackDist", 0x4C4, GameTuningValueKind::Number, 0.0f, 0x007e9f06},
    {"Pilot/DiveBomb/SafeDist", 0x4C8, GameTuningValueKind::Number, 0.0f, 0x007e9f88},
    {"Pilot/DiveBomb/BeginAltRange/1", 0x4CC, GameTuningValueKind::Number, 0.0f, 0x007e9fe2},
    {"Pilot/DiveBomb/BeginAltRange/2", 0x4D0, GameTuningValueKind::Number, 0.0f, 0x007ea04d},
    {"Pilot/DiveBomb/MoveOnCruisingAlt", 0x4D4, GameTuningValueKind::Boolean, 0.0f, 0x007ea0e0},
    {"Pilot/DiveBomb/ReferenceSpeed", 0x4D8, GameTuningValueKind::Number, 0.0f, 0x007ea09f},
    {"Pilot/TakeOff/PrepareTime", 0x4DC, GameTuningValueKind::Number, 0.0f, 0x007ea717},
    {"Pilot/Landing/ApproachPitch", 0x4E0, GameTuningValueKind::Number, 0.0f, 0x007ea16b},
    {"Pilot/Landing/ApproachAngle", 0x4E4, GameTuningValueKind::Number, 0.0f, 0x007ea1ac},
    {"Pilot/Landing/ParkVelocity", 0x4E8, GameTuningValueKind::Number, 0.0f, 0x007ea26f},
    {"Pilot/Landing/ApproachDist", 0x4EC, GameTuningValueKind::Number, 0.0f, 0x007ea1ed},
    {"Pilot/Landing/PosBehind", 0x4F0, GameTuningValueKind::Number, 0.0f, 0x007ea332},
    {"Pilot/Landing/PosAlt", 0x4F4, GameTuningValueKind::Number, 0.0f, 0x007ea373},
    {"Pilot/Landing/CircleMultiplierMin", 0x4F8, GameTuningValueKind::Number, 0.0f, 0x007ea3b4},
    {"Pilot/Landing/CircleMultiplierMax", 0x4FC, GameTuningValueKind::Number, 0.0f, 0x007ea3f5},
    {"Pilot/Landing/StandbyDist", 0x500, GameTuningValueKind::Number, 0.0f, 0x007ea436},
    {"Pilot/Landing/FollowDistTime", 0x504, GameTuningValueKind::Number, 0.0f, 0x007ea22e},
    {"Pilot/Landing/TakeoffDist", 0x508, GameTuningValueKind::Number, 0.0f, 0x007ea2f1},
    {"Pilot/Landing/TouchDownDist", 0x50C, GameTuningValueKind::Number, 0.0f, 0x007ea2b0},
    {"Pilot/Landing/LiftDelay", 0x510, GameTuningValueKind::Number, 0.0f, 0x007ea477},
    {"Pilot/Landing/CruisingAlt", 0x514, GameTuningValueKind::Number, 0.0f, 0x007ea57b},
    {"Pilot/Landing/WireRope", 0x518, GameTuningValueKind::Number, 0.0f, 0x007ea4b8},
    {"Pilot/Landing/MaxWireRope", 0x51C, GameTuningValueKind::Number, 0.0f, 0x007ea4f9},
    {"Pilot/Landing/MinFreeRunwayLength", 0x520, GameTuningValueKind::Number, 0.0f, 0x007ea53a},
    {"Pilot/Landing/RadiusChange/1", 0x524, GameTuningValueKind::Number, 0.0f, 0x007ea5d5},
    {"Pilot/Landing/RadiusChange/2", 0x528, GameTuningValueKind::Number, 0.0f, 0x007ea644},
    {"Pilot/Landing/ReferenceSpeed", 0x52C, GameTuningValueKind::Number, 0.0f, 0x007ea695},
    {"UnitAI/StopClearSpeed", 0x530, GameTuningValueKind::Number, 0.0f, 0x007e6b3a},
    {"UnitAI/StopClearAlt", 0x534, GameTuningValueKind::Number, 0.0f, 0x007e6b7b},
    {"Pilot/General/WaggleLimit", 0x538, GameTuningValueKind::Number, 0.0f, 0x007e6e46},
    {"Pilot/General/CruisingAlt", 0x53C, GameTuningValueKind::Number, 0.0f, 0x007e6fec},
    {"Pilot/General/MaxAltOffset", 0x540, GameTuningValueKind::Number, 0.0f, 0x007e702d},
    {"Pilot/General/ClimbDist", 0x544, GameTuningValueKind::Number, 0.0f, 0x007e6e05},
    {"Pilot/General/DropDist", 0x548, GameTuningValueKind::Number, 0.0f, 0x007e6dc4},
    {"Pilot/General/MinTurnCircle", 0x54C, GameTuningValueKind::Number, 0.0f, 0x007e7131},
    {"Pilot/General/LevelBombAngleMax", 0x550, GameTuningValueKind::Number, 0.0f, 0x007e75b4},
    {"Pilot/General/LevelBombAngleMax", 0x550, GameTuningValueKind::Number, 0.0f, 0x007e75f5},
    {"Pilot/General/LevelBombAngleMax", 0x550, GameTuningValueKind::Number, 0.0f, 0x007e7636},
    {"Pilot/General/LevelBombAngleMax", 0x550, GameTuningValueKind::Number, 0.0f, 0x007e7677},
    {"Pilot/General/DiveBombRollAngleMin", 0x554, GameTuningValueKind::Number, 0.0f, 0x007e7352},
    {"Pilot/General/DiveBombRollAngleMax", 0x558, GameTuningValueKind::Number, 0.0f, 0x007e7393},
    {"Pilot/General/DiveBombRollAngleMinPitch", 0x55C, GameTuningValueKind::Number, 0.0f, 0x007e73d4},
    {"Pilot/General/DiveBombRollAngleMaxPitch", 0x560, GameTuningValueKind::Number, 0.0f, 0x007e7415},
    {"Pilot/General/DiveBombPitchAngleMin", 0x564, GameTuningValueKind::Number, 0.0f, 0x007e7456},
    {"Pilot/General/DiveBombPitchAngleMax", 0x568, GameTuningValueKind::Number, 0.0f, 0x007e7497},
    {"Pilot/General/SoftHdgMul", 0x56C, GameTuningValueKind::Number, 0.0f, 0x007e6f13},
    {"Pilot/General/SoftHdgLimit", 0x570, GameTuningValueKind::Number, 0.0f, 0x007e6e87},
    {"Pilot/General/SoftHdgZone", 0x574, GameTuningValueKind::NumberOrDefault, 0.01f, 0x007e6ed2},
    {"Pilot/General/SoftRollCtrl", 0x578, GameTuningValueKind::Number, 0.0f, 0x007e6f54},
    {"Pilot/General/SoftRollMul", 0x57C, GameTuningValueKind::Number, 0.0f, 0x007e6f95},
    {"Pilot/General/HdgDiffCalcLimit/1", 0x584, GameTuningValueKind::Number, 0.0f, 0x007e720d},
    {"Pilot/General/HdgDiffCalcLimit/2", 0x588, GameTuningValueKind::Number, 0.0f, 0x007e727b},
    {"Pilot/General/HdgDiffCalcMinPitch", 0x58C, GameTuningValueKind::Number, 0.0f, 0x007e72d0},
    {"Pilot/General/HdgDiffCalcMinRoll", 0x590, GameTuningValueKind::Number, 0.0f, 0x007e7311},
    {"Pilot/General/GuardDist", 0x594, GameTuningValueKind::Number, 0.0f, 0x007e706e},
    {"Pilot/General/LeaveAlonePwr", 0x598, GameTuningValueKind::Number, 0.0f, 0x007e70f0},
    {"Pilot/General/TurnRollLimitSmall", 0x59C, GameTuningValueKind::Number, 0.0f, 0x007e7172},
    {"Pilot/General/TurnRollLimitLarge", 0x5A0, GameTuningValueKind::Number, 0.0f, 0x007e71b3},
    {"Pilot/General/TurnRollPitchLimitPitch/1", 0x5A4, GameTuningValueKind::Number, 0.0f, 0x007e77ee},
    {"Pilot/General/TurnRollPitchLimitPitch/2", 0x5A8, GameTuningValueKind::Number, 0.0f, 0x007e785c},
    {"Pilot/General/TurnRollPitchLimitRoll/1", 0x5AC, GameTuningValueKind::Number, 0.0f, 0x007e78ca},
    {"Pilot/General/TurnRollPitchLimitRoll/2", 0x5B0, GameTuningValueKind::Number, 0.0f, 0x007e7938},
    {"Pilot/General/YawTurnRollRange/1", 0x5B4, GameTuningValueKind::Number, 0.0f, 0x007e76d1},
    {"Pilot/General/YawTurnRollRange/2", 0x5B8, GameTuningValueKind::Number, 0.0f, 0x007e773f},
    {"Pilot/General/YawTurnMaxPitch", 0x5BC, GameTuningValueKind::Number, 0.0f, 0x007e7794},
    {"Pilot/General/PitchTurnMaxPitch", 0x5C0, GameTuningValueKind::Number, 0.0f, 0x007e798d},
    {"Pilot/General/PitchTurnHdgRange/1", 0x5C4, GameTuningValueKind::Number, 0.0f, 0x007e79e7},
    {"Pilot/General/PitchTurnHdgRange/2", 0x5C8, GameTuningValueKind::Number, 0.0f, 0x007e7a55},
    {"Pilot/General/WingmenWaitDist/1", 0x5CC, GameTuningValueKind::Number, 0.0f, 0x007e74f1},
    {"Pilot/General/WingmenWaitDist/2", 0x5D0, GameTuningValueKind::Number, 0.0f, 0x007e755f},
    {"Pilot/General/YawCtrlSetTimeMul", 0x5D4, GameTuningValueKind::Number, 0.0f, 0x007e7aaa},
    {"Pilot/General/PitchCtrlSetTimeMul", 0x5D8, GameTuningValueKind::Number, 0.0f, 0x007e7aeb},
    {"Pilot/General/MoveCircleMinAngle", 0x5DC, GameTuningValueKind::Number, 0.0f, 0x007e7b2c},
    {"Pilot/General/MoveCircleMaxAngle", 0x5E0, GameTuningValueKind::Number, 0.0f, 0x007e7b6d},
    {"Pilot/General/MoveCircleFollowedDist", 0x5E4, GameTuningValueKind::Number, 0.0f, 0x007e7bae},
    {"Pilot/General/TrgSpeedCorrMinPitch", 0x5E8, GameTuningValueKind::Number, 0.0f, 0x007e7bef},
    {"Pilot/General/TrgSpeedCorrSpeedMul", 0x5EC, GameTuningValueKind::Number, 0.0f, 0x007e7c30},
    {"Pilot/General/TrgSpeedCorrMulDecay", 0x5F0, GameTuningValueKind::Number, 0.0f, 0x007e7c71},
    {"Pilot/General/DropAllEquipmentPercent", 0x5F4, GameTuningValueKind::Number, 0.0f, 0x007e70af},
    {"Pilot/Avoidance/Gunfire/MaxWeight", 0x5F8, GameTuningValueKind::Number, 0.0f, 0x007e7d40},
    {"Pilot/Avoidance/Gunfire/WeightInc", 0x5FC, GameTuningValueKind::Number, 0.0f, 0x007e7d7f},
    {"Pilot/Avoidance/Gunfire/WeightDec", 0x600, GameTuningValueKind::Number, 0.0f, 0x007e7dbe},
    {"Pilot/Avoidance/Gunfire/AvoidTime/1", 0x604, GameTuningValueKind::Number, 0.0f, 0x007e7e16},
    {"Pilot/Avoidance/Gunfire/AvoidTime/2", 0x608, GameTuningValueKind::Number, 0.0f, 0x007e7e82},
    {"Pilot/Avoidance/Gunfire/WaitTime/1", 0x60C, GameTuningValueKind::Number, 0.0f, 0x007e7eee},
    {"Pilot/Avoidance/Gunfire/WaitTime/2", 0x610, GameTuningValueKind::Number, 0.0f, 0x007e7f5a},
    {"Pilot/Avoidance/Gunfire/BomberVSGunfire", 0x614, GameTuningValueKind::Number, 0.0f, 0x007e7fad},
    {"Pilot/Avoidance/Vehicle/MaxWeight", 0x618, GameTuningValueKind::Number, 0.0f, 0x007e8028},
    {"Pilot/Avoidance/Vehicle/WeightInc", 0x61C, GameTuningValueKind::Number, 0.0f, 0x007e8067},
    {"Pilot/Avoidance/Vehicle/WeightDec", 0x620, GameTuningValueKind::Number, 0.0f, 0x007e80a6},
    {"Pilot/Avoidance/Vehicle/MinCollTime", 0x624, GameTuningValueKind::Number, 0.0f, 0x007e80e5},
    {"Pilot/Avoidance/Vehicle/AvoidSpdMul", 0x628, GameTuningValueKind::Number, 0.0f, 0x007e81e1},
    {"Pilot/Avoidance/Vehicle/MinPlaneSpd", 0x62C, GameTuningValueKind::Number, 0.0f, 0x007e8124},
    {"Pilot/Avoidance/Vehicle/UseRollStrength", 0x630, GameTuningValueKind::Number, 0.0f, 0x007e8220},
    {"Pilot/Avoidance/Vehicle/MaxDistMultiplier", 0x634, GameTuningValueKind::Number, 0.0f, 0x007e8163},
    {"Pilot/Avoidance/Vehicle/MinDistMultiplier", 0x638, GameTuningValueKind::Number, 0.0f, 0x007e81a2},
    {"Pilot/Avoidance/Vehicle/BomberVSSmallPlane", 0x63C, GameTuningValueKind::Boolean, 0.0f, 0x007e825f},
    {"Pilot/Dogfight/CruisingAlt", 0x640, GameTuningValueKind::Number, 0.0f, 0x007ea799},
    {"Pilot/Dogfight/AttackDist", 0x644, GameTuningValueKind::Number, 0.0f, 0x007ea7da},
    {"Pilot/Dogfight/FighterAimMulVersusAI", 0x648, GameTuningValueKind::NumberOrDefault, 1.8f, 0x007ea825},
    {"Pilot/Dogfight/FighterAimMulVersusPlayer", 0x64C, GameTuningValueKind::NumberOrDefault, 1.2f, 0x007ea870},
    {"Pilot/Dogfight/ReferenceSpeed", 0x650, GameTuningValueKind::Number, 0.0f, 0x007ea8b1},
    {"Pilot/Strafe/CruisingAlt", 0x654, GameTuningValueKind::Number, 0.0f, 0x007ea933},
    {"Pilot/Strafe/AttackDist", 0x658, GameTuningValueKind::Number, 0.0f, 0x007ea974},
    {"Pilot/Strafe/ReferenceSpeed", 0x65C, GameTuningValueKind::Number, 0.0f, 0x007ea9b5},
    {"Pilot/Strike/CruisingAlt", 0x660, GameTuningValueKind::Number, 0.0f, 0x007eaa37},
    {"Pilot/Strike/AttackDist", 0x664, GameTuningValueKind::Number, 0.0f, 0x007eaa78},
    {"Pilot/Strike/ReferenceSpeed", 0x668, GameTuningValueKind::Number, 0.0f, 0x007eaab9},
    {"Pilot/AutoStrafeAngle/Angle_Prepare", 0x66C, GameTuningValueKind::Number, 0.0f, 0x007e90f2},
    {"Pilot/AutoStrafeAngle/Angle_MoveTo", 0x670, GameTuningValueKind::Number, 0.0f, 0x007e9133},
    {"Pilot/AutoStrafeAngle/Angle_GoAway", 0x674, GameTuningValueKind::Number, 0.0f, 0x007e9174},
    {"Pilot/AutoStrafeAngle/Angle_Strafe", 0x678, GameTuningValueKind::Number, 0.0f, 0x007e91b5},
    {"PlaneGUI/PitchYawZoomControlLimit", 0x67C, GameTuningValueKind::Number, 0.0f, 0x007e62ab},
    {"PlaneGUI/RollZoomControlLimit", 0x680, GameTuningValueKind::Number, 0.0f, 0x007e62e9},
    {"PlaneGUI/AltimeterCeiling", 0x684, GameTuningValueKind::Number, 0.0f, 0x007e6327},
    {"PlaneGUI/MouseInputMultiplier", 0x688, GameTuningValueKind::Number, 0.0f, 0x007e6365},
    {"PlaneGUI/MouseInputDeadZoneMin", 0x68C, GameTuningValueKind::Number, 0.0f, 0x007e63a3},
    {"PlaneGUI/MouseInputDeadZoneMax", 0x690, GameTuningValueKind::Number, 0.0f, 0x007e63e1},
    {"PlaneGUI/MouseInputSmoothTreshold", 0x694, GameTuningValueKind::Number, 0.0f, 0x007e641f},
    {"PlaneGUI/MouseInputSmoothMultiplier", 0x698, GameTuningValueKind::Number, 0.0f, 0x007e645d},
    {"PlaneGUI/MouseInputExponencialWeight", 0x69C, GameTuningValueKind::Number, 0.0f, 0x007e649b},
    {"PlaneGUI/SpeedDisplayMultiplier", 0x6A0, GameTuningValueKind::Number, 0.0f, 0x007e64d9},
    {"Sound/EnginePowerMultiplier", 0x6A4, GameTuningValueKind::Number, 0.0f, 0x007e6555},
    {"Sound/EngineSpeedMultiplier", 0x6A8, GameTuningValueKind::Number, 0.0f, 0x007e6593},
    {"Sound/IdleOffVolume", 0x6AC, GameTuningValueKind::Number, 0.0f, 0x007e65d1},
    {"Sound/EngineMinFreq", 0x6B0, GameTuningValueKind::Number, 0.0f, 0x007e660f},
    {"Sound/EngineMaxFreq", 0x6B4, GameTuningValueKind::Number, 0.0f, 0x007e664d},
    {"Sound/PitchChangeRate", 0x6B8, GameTuningValueKind::Number, 0.0f, 0x007e668b},
    {"Sound/VolumeChangeRate", 0x6BC, GameTuningValueKind::Number, 0.0f, 0x007e66cc},
    {"Sound/IdleSafetyTime", 0x6C0, GameTuningValueKind::Number, 0.0f, 0x007e670d},
    {"Sound/EngineFreqLimitSpdMul", 0x6C4, GameTuningValueKind::Number, 0.0f, 0x007e674e},
    {"Sound/EngineFreqLimitMax", 0x6C8, GameTuningValueKind::Number, 0.0f, 0x007e678f},
    {"Sound/IdleAndEngineVolMax", 0x6CC, GameTuningValueKind::Number, 0.0f, 0x007e67d0},
};

namespace {
// Walk a '/' separated key path from the root table, releasing each intermediate.
// One-based numeric segments are array indices (00B67720), the rest are names
// (00B67800). The native loader keeps a section scope alive instead of re-walking
// from the root; the values reached are the same and the walk is the readable form.
GameTuningLuaHost::Handle resolve(GameTuningLuaHost& host, GameTuningLuaHost::Handle root,
                                  const char* path, GameTuningLuaHost::Handle* trail,
                                  int& depth) {
    GameTuningLuaHost::Handle cur = root;
    depth = 0;
    const char* p = path;
    while (*p != '\0') {
        char segment[64];
        int n = 0;
        while (*p != '\0' && *p != '/' && n < 63) {
            segment[n++] = *p++;
        }
        segment[n] = '\0';
        if (*p == '/') {
            ++p;
        }
        bool numeric = n > 0;
        for (int i = 0; i < n; ++i) {
            if (segment[i] < '0' || segment[i] > '9') {
                numeric = false;
            }
        }
        if (numeric) {
            int index = 0;
            for (int i = 0; i < n; ++i) {
                index = index * 10 + (segment[i] - '0');
            }
            cur = host.table_element(cur, index);
        } else {
            cur = host.table_field(cur, segment);
        }
        if (depth < 15) {
            trail[depth++] = cur;
        }
    }
    return cur;
}

float* slot(GameTuningBlock& block, std::uint16_t offset) {
    return reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(&block) + offset);
}
} // namespace

void game_tuning_load_007e2a20(GameTuningLuaHost& host, GameTuningBlock& block) {
    host.run_script(kGameTuningConstantsScript);
    host.run_script(kGameTuningDataScript);
    const GameTuningLuaHost::Handle globals = host.globals();
    const GameTuningLuaHost::Handle root = host.table_field(globals, kGameTuningRootTable);
    host.release(globals);

    for (std::size_t i = 0; i < kGameTuningKeyCount; ++i) {
        const GameTuningKey& key = kGameTuningKeys[i];
        if (key.path == nullptr) {
            continue;
        }
        GameTuningLuaHost::Handle trail[16] = {};
        int depth = 0;
        const GameTuningLuaHost::Handle value = resolve(host, root, key.path, trail, depth);
        switch (key.kind) {
        case GameTuningValueKind::Number:
            *slot(block, key.offset) = static_cast<float>(host.value(value).number);
            break;
        case GameTuningValueKind::NumberOrDefault: {
            const GameTuningLuaValue v = host.value(value);
            *slot(block, key.offset) = v.nil ? key.fallback : static_cast<float>(v.number);
            break;
        }
        case GameTuningValueKind::Integer:
            *reinterpret_cast<std::int32_t*>(slot(block, key.offset)) =
                static_cast<std::int32_t>(host.value(value).number);
            break;
        case GameTuningValueKind::Boolean:
            *reinterpret_cast<bool*>(slot(block, key.offset)) = host.value(value).boolean;
            break;
        case GameTuningValueKind::NumberTriple:
            host.number_triple(value, slot(block, key.offset));
            break;
        }
        for (int d = depth - 1; d >= 0; --d) {
            host.release(trail[d]);
        }
    }
    host.release(root);
}

float game_tuning_control_speed_007e41df(float control_range_min, float control_range_max,
                                         float stall_range_max, float level_flight,
                                         float lerp_k, float stall_k, float level_k) noexcept {
    float v = (control_range_max - control_range_min) * lerp_k + control_range_min;
    const float stall = stall_range_max * stall_k;
    if (stall < v) {
        v = stall;
    }
    const float level = level_flight * level_k;
    if (v <= level) {
        v = level;
    }
    return level_flight < v ? level_flight : v;
}

float game_tuning_blend_007e6fd4(float a, float b) noexcept {
    return (1.0f - a) * b;
}

float game_tuning_ctrl_angle_cos(float radians) noexcept {
    return std::cos(radians);
}

float game_tuning_apply_accel_cheat_007d20f3(GameTuningBlock& block, float accel) noexcept {
    float* const factor = slot(block, kGameTuningAccelCheatMul);
    if (*factor > 1.0f) {
        // 007D2115..007D2127, no write into the tuning object on this branch.
        return *slot(block, kGameTuningAccelCheatMulMul) * *factor * accel;
    }
    // 007D2134/007D213C. Idempotent: the field can only become exactly 1.0f, only
    // from a value at or below 1.0f, and the row's Accel is returned unscaled.
    *factor = 1.0f;
    return accel;
}

} // namespace bsp
