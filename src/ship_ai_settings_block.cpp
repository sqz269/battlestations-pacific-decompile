// Packet cc_ai_settings_block. See include/bsp/ship_ai_settings_block.hpp and
// docs/SHIP_AI_SETTINGS_BLOCK.md for the address, evidence, original ABI and
// uncertainty behind every rule and every table row.
#include "bsp/ship_ai_settings_block.hpp"

namespace bsp {
namespace {

// The seven .rdata constants 0081F214..0081F269 stores into the block.
constexpr float kSeedMyDamageWeight = 10.0f;         // 00CE38B8
constexpr float kSeedIdealDistWeight = 4.0f;         // 00CE3D34
constexpr float kSeedNearbyEnemyWeight = 3.0f;       // 00CE3854
constexpr float kSeedNearbyEnemyReference = 1000.0f; // 00CE3804
constexpr float kSeedNearestMoveDirWeight = 1.0f;    // 00D7A24C
constexpr float kSeedPrevMoveDirWeight = 0.25f;      // 00CE3868
constexpr float kSeedPrevMoveDirRange = 1.0471976f;  // 00D05AAC, DEG(60)
constexpr float kSeedUnset = -1.0f;                  // 00D7A260

// The ship AI's own keys in 0083B5E0, ordered by the loader's getter call site.
// `installed` is the value in the shipped scripts/datatables/shipglobals.lua.
constexpr ShipAiSettingsKeyRecord kKeys[] = {
    {0x190, "ShipAvoidance.CollectTimer", 1, ShipAiSettingsGetter::kNumber, 0.0f, 1.0f, 0x0083b7ad, 0x0083b7b2},
    {0x194, "ShipAvoidance.CollectTimer", 2, ShipAiSettingsGetter::kNumber, 0.0f, 2.0f, 0x0083b810, 0x0083b815},
    {0x198, "ShipAvoidance.CollectDist", 0, ShipAiSettingsGetter::kNumber, 0.0f, 450.0f, 0x0083b85d, 0x0083b862},
    {0x19C, "ShipAvoidance.CollectHitTime", 0, ShipAiSettingsGetter::kNumber, 0.0f, 10.0f, 0x0083b899, 0x0083b89e},
    {0x1A0, "ShipAvoidance.NearbyShip_ArriveTimeMin", 0, ShipAiSettingsGetter::kFloatOrDefault, 3.0f, 2.0f, 0x0083b8df, 0x0083b8e4},
    {0x1A4, "ShipAvoidance.NearbyShip_ArriveDistMin", 0, ShipAiSettingsGetter::kFloatOrDefault, 0.25f, 0.4f, 0x0083b925, 0x0083b92a},
    {0x1A8, "ShipAvoidance.NearbyShip_PosSpeedCorrig", 0, ShipAiSettingsGetter::kFloatOrDefault, 2.0f, 0.5f, 0x0083b96b, 0x0083b970},
    {0x1AC, "ShipAvoidance.NearbyShip_EstPos_DistLimitMul", 0, ShipAiSettingsGetter::kFloatOrDefault, 0.7f, 4.0f, 0x0083b9b1, 0x0083b9b6},
    {0x1B0, "ShipAvoidance.NearbyShip_EstPos_MinShipLength", 0, ShipAiSettingsGetter::kFloatOrDefault, 100.0f, 5.0f, 0x0083b9f7, 0x0083b9fc},
    {0x1B4, "ShipAvoidance.NearbyShip_EstPos_ShipLengthLimitMul", 0, ShipAiSettingsGetter::kFloatOrDefault, 1.8f, 4.0f, 0x0083ba3d, 0x0083ba42},
    {0x1B8, "ShipAvoidance.NearbyShip_MyMinSpdRatio", 0, ShipAiSettingsGetter::kFloatOrDefault, 0.6f, 0.2f, 0x0083ba83, 0x0083ba88},
    {0x1BC, "ShipAvoidance.NearbyShip_EstPos_ShipSpdMul", 0, ShipAiSettingsGetter::kFloatOrDefault, 0.75f, 1.0f, 0x0083bac9, 0x0083bace},
    {0x1C0, "ShipAvoidance.NearbyShip_EstPos_SizeDecMul", 0, ShipAiSettingsGetter::kFloatOrDefault, 0.3f, 0.3f, 0x0083bb0f, 0x0083bb14},
    {0x1C4, "ShipAvoidance.NearbyShip_EstPos_SizeDecMin", 0, ShipAiSettingsGetter::kFloatOrDefault, 0.1f, 0.2f, 0x0083bb55, 0x0083bb5a},
    {0x1C8, "ShipAvoidance.NearbyShip_NextCornerReachDistAddOn", 0, ShipAiSettingsGetter::kFloatOrDefault, 100.0f, 100.0f, 0x0083bb9b, 0x0083bba0},
    {0x1CC, "ShipAvoidance.NearbyShip_MovePathLineCheckThreshold", 0, ShipAiSettingsGetter::kFloatOrDefault, 200.0f, 150.0f, 0x0083bbe1, 0x0083bbe6},
    {0x1D4, "ShipAvoidance.NearbyShip_WayClearCheckTime", 0, ShipAiSettingsGetter::kFloatOrDefault, 0.5f, 0.25f, 0x0083bc27, 0x0083bc2c},
    {0x1D8, "ShipAvoidance.HitDetector_LastHitDistAddOn", 0, ShipAiSettingsGetter::kFloatOrDefault, 30.0f, 50.0f, 0x0083bc6d, 0x0083bc72},
    {0x1D0, "ShipAvoidance.NearbyShip_GoAwaySpdAdd", 0, ShipAiSettingsGetter::kFloatOrDefault, 3.0f, 2.0f, 0x0083bcb3, 0x0083bcb8},
    {0x1EC, "TorpedoAvoidance.CollectTimer", 1, ShipAiSettingsGetter::kNumber, 0.0f, 1.5f, 0x0083bf3f, 0x0083bf44},
    {0x1F0, "TorpedoAvoidance.CollectTimer", 2, ShipAiSettingsGetter::kNumber, 0.0f, 2.0f, 0x0083bfa8, 0x0083bfad},
    {0x1F4, "LandAvoidance.CheckMovePosZoneTime", 1, ShipAiSettingsGetter::kNumber, 0.0f, 2.5f, 0x0083c032, 0x0083c037},
    {0x1F8, "LandAvoidance.CheckMovePosZoneTime", 2, ShipAiSettingsGetter::kNumber, 0.0f, 3.0f, 0x0083c09b, 0x0083c0a0},
    {0x1FC, "LandAvoidance.CheckShipPosZoneTime", 1, ShipAiSettingsGetter::kNumber, 0.0f, 2.5f, 0x0083c104, 0x0083c109},
    {0x200, "LandAvoidance.CheckShipPosZoneTime", 2, ShipAiSettingsGetter::kNumber, 0.0f, 3.0f, 0x0083c16d, 0x0083c172},
    {0x204, "LandAvoidance.CheckTravelZoneTime", 1, ShipAiSettingsGetter::kNumber, 0.0f, 3.0f, 0x0083c1d6, 0x0083c1db},
    {0x208, "LandAvoidance.CheckTravelZoneTime", 2, ShipAiSettingsGetter::kNumber, 0.0f, 4.0f, 0x0083c23f, 0x0083c244},
    {0x20C, "LandAvoidance.CollectTimer", 1, ShipAiSettingsGetter::kNumber, 0.0f, 1.0f, 0x0083c2ab, 0x0083c2b0},
    {0x210, "LandAvoidance.CollectTimer", 2, ShipAiSettingsGetter::kNumber, 0.0f, 1.5f, 0x0083c31a, 0x0083c31f},
    {0x214, "LandAvoidance.YTurnDirDiff", 1, ShipAiSettingsGetter::kFloatOrDefault, 1.8f, 1.8f, 0x0083c393, 0x0083c398},
    {0x218, "LandAvoidance.YTurnDirDiff", 2, ShipAiSettingsGetter::kFloatOrDefault, 2.1f, 2.1f, 0x0083c40c, 0x0083c411},
    {0x160, "AttackMoveDirector.MyDamageWeight", 0, ShipAiSettingsGetter::kFloatOrDefault, 10.0f, 10.0f, 0x0083c96c, 0x0083c971},
    {0x164, "AttackMoveDirector.IdealDistWeight", 0, ShipAiSettingsGetter::kFloatOrDefault, 4.0f, 4.0f, 0x0083c9b8, 0x0083c9bd},
    {0x168, "AttackMoveDirector.NearbyEnemyWeight", 0, ShipAiSettingsGetter::kFloatOrDefault, 3.0f, 3.0f, 0x0083ca01, 0x0083ca06},
    {0x16C, "AttackMoveDirector.NearbyEnemyReference", 0, ShipAiSettingsGetter::kFloatOrDefault, 1000.0f, 1000.0f, 0x0083ca47, 0x0083ca4c},
    {0x170, "AttackMoveDirector.NearestMoveDirWeight", 0, ShipAiSettingsGetter::kFloatOrDefault, 1.0f, 1.0f, 0x0083ca8c, 0x0083ca91},
    {0x174, "AttackMoveDirector.PrevMoveDirWeight", 0, ShipAiSettingsGetter::kFloatOrDefault, 0.25f, 0.25f, 0x0083cad8, 0x0083cadd},
    {0x178, "AttackMoveDirector.PrevMoveDirRange", 0, ShipAiSettingsGetter::kFloatOrDefault, 1.0471976f, 1.0471976f, 0x0083cb24, 0x0083cb29},
    {0x6CC, "Navigator.AutoThrust.HdgDiffValueMin_Slow", 0, ShipAiSettingsGetter::kNumber, 0.0f, 0.43633232f, 0x0083cc2c, 0x0083cc31},
    {0x6D0, "Navigator.AutoThrust.HdgDiffValueMax_Slow", 0, ShipAiSettingsGetter::kNumber, 0.0f, 1.3089969f, 0x0083cc6e, 0x0083cc73},
    {0x6D4, "Navigator.AutoThrust.ThrustMin_Slow", 0, ShipAiSettingsGetter::kNumber, 0.0f, 0.5f, 0x0083ccb0, 0x0083ccb5},
    {0x6E0, "Navigator.AutoThrust.HdgDiffValueMin_Fast", 0, ShipAiSettingsGetter::kNumber, 0.0f, 0.78539816f, 0x0083cd76, 0x0083cd7b},
    {0x6E4, "Navigator.AutoThrust.HdgDiffValueMax_Fast", 0, ShipAiSettingsGetter::kNumber, 0.0f, 1.5707963f, 0x0083cdb8, 0x0083cdbd},
    {0x6E8, "Navigator.AutoThrust.ThrustMin_Fast", 0, ShipAiSettingsGetter::kNumber, 0.0f, 0.75f, 0x0083cdfa, 0x0083cdff},
    {0x6EC, "Navigator.AutoThrust.HdgDiffDangerMul", 0, ShipAiSettingsGetter::kNumber, 0.0f, 6.0f, 0x0083ce3c, 0x0083ce41},
    {0x6F0, "Navigator.PathFinderParams.LengthModifier_DirDiffMin", 0, ShipAiSettingsGetter::kFloatOrDefault, 0.261799f, 0.261799f, 0x0083d4bf, 0x0083d4c4},
    {0x6F4, "Navigator.PathFinderParams.LengthModifier_DirDiffMax", 0, ShipAiSettingsGetter::kFloatOrDefault, 1.5708f, 1.3962634f, 0x0083d50b, 0x0083d510},
    {0x6F8, "Navigator.PathFinderParams.LengthModifier_LengthAddon", 0, ShipAiSettingsGetter::kFloatOrDefault, 1500.0f, 1200.0f, 0x0083d557, 0x0083d55c},
    {0x3B0, "WaterTickDamage", 0, ShipAiSettingsGetter::kFloatOrDefault, 0.0f, 100.0f, 0x0083e1b3, 0x0083e1b8},
    {0x4D4, "SubAttack.SubmarineLostTime", 0, ShipAiSettingsGetter::kNumber, 0.0f, 30.0f, 0x0083f79c, 0x0083f7a1},
};

// Every settings read this packet resolved in the ship AI segment. The two rows
// with a base-relative read (009EAFC0, 009F0100) come from `ADD reg,180h`
// followed by an eight-bit displacement; the rest are `[EAX + disp]` directly
// after the getter call.
constexpr ShipAiSettingsReadSite kReadSites[] = {
    {0x004, 0x009ec770, 0x009ec790, 0x009ec795},
    {0x004, 0x009ef350, 0x009ef384, 0x009ef389},
    {0x004, 0x009f0ea0, 0x009f1075, 0x009f107a},
    {0x004, 0x009f1420, 0x009f1b71, 0x009f1b78},
    {0x190, 0x009f1160, 0x009f13c4, 0x009f13e2},
    {0x194, 0x009eaca0, 0x009ead43, 0x009ead58},
    {0x194, 0x009eb660, 0x009ebede, 0x009ebee3},
    {0x194, 0x009f0d20, 0x009f0dc8, 0x009f0dcd},
    {0x194, 0x009f1160, 0x009f13cb, 0x009f13e2},
    {0x198, 0x009f1420, 0x009f19b8, 0x009f19bd},
    {0x19C, 0x009f1420, 0x009f199d, 0x009f19ae},
    {0x1A0, 0x009eafc0, 0x009eb05f, 0x009eb18c},
    {0x1A4, 0x009eafc0, 0x009eb05f, 0x009eb0ba},
    {0x1A8, 0x009eae20, 0x009eaec5, 0x009eaeca},
    {0x1AC, 0x009eafc0, 0x009eb05f, 0x009eb340},
    {0x1B0, 0x009eafc0, 0x009eb05f, 0x009eb334},
    {0x1B4, 0x009eafc0, 0x009eb05f, 0x009eb350},
    {0x1B8, 0x009f0ea0, 0x009f0f62, 0x009f0f6d},
    {0x1BC, 0x009eafc0, 0x009eb05f, 0x009eb20f},
    {0x1C0, 0x009eafc0, 0x009eb05f, 0x009eb28f},
    {0x1C4, 0x009eafc0, 0x009eb05f, 0x009eb288},
    {0x1C8, 0x009f0100, 0x009f02bf, 0x009f06da},
    {0x1CC, 0x009f0100, 0x009f02bf, 0x009f072f},
    {0x1CC, 0x009f0100, 0x009f02bf, 0x009f098b},
    {0x1D0, 0x009eafc0, 0x009eb05f, 0x009eb167},
    {0x1D4, 0x009ef910, 0x009ef943, 0x009ef948},
    {0x1D8, 0x009eb660, 0x009eb681, 0x009eb686},
    {0x1EC, 0x009f1160, 0x009f1392, 0x009f13b0},
    {0x1F0, 0x009eaca0, 0x009ead43, 0x009ead4e},
    {0x1F0, 0x009f0ad0, 0x009f0ba7, 0x009f0bac},
    {0x1F4, 0x009eca20, 0x009ecec5, 0x009ecee3},
    {0x1FC, 0x009eca20, 0x009ecb47, 0x009ecb65},
    {0x204, 0x009eca20, 0x009ed01a, 0x009ed01f},
    {0x208, 0x009eca20, 0x009ed029, 0x009ed02e},
    {0x214, 0x009ef910, 0x009f0076, 0x009f007f},
    {0x218, 0x009ef910, 0x009f0076, 0x009f0089},
    {0x240, 0x009f1bc0, 0x009f2d62, 0x009f2d87},
    {0x4D4, 0x009f3670, 0x009f369f, 0x009f36a4},
    {0x4D4, 0x009f3670, 0x009f36e1, 0x009f36e9},
    {0x6CC, 0x009ec7c0, 0x009ec82a, 0x009ec850},
    {0x6D0, 0x009ec7c0, 0x009ec823, 0x009ec840},
    {0x6D4, 0x009ec7c0, 0x009ec8ca, 0x009ec8da},
    {0x6E0, 0x009ec7c0, 0x009ec892, 0x009ec8b8},
    {0x6E4, 0x009ec7c0, 0x009ec88b, 0x009ec8a8},
    {0x6E8, 0x009ec7c0, 0x009ec900, 0x009ec910},
    {0x6EC, 0x009ec7c0, 0x009ec7c4, 0x009ec7d4},
    {0x6F0, 0x009ec280, 0x009ec30b, 0x009ec310},
    {0x6F4, 0x009ec280, 0x009ec31e, 0x009ec323},
    {0x6F8, 0x009ec280, 0x009ec331, 0x009ec336},
    {0x3B0, 0x00a08460, 0x00a09624, 0x00a09629},
};

}  // namespace

void seed_navigator_params_0081f214(UnitNavigatorParamsBlock& block) noexcept
{
    block.my_damage_weight = kSeedMyDamageWeight;               // 0081F21C
    block.ideal_dist_weight = kSeedIdealDistWeight;             // 0081F228
    block.nearby_enemy_weight = kSeedNearbyEnemyWeight;         // 0081F235
    block.nearby_enemy_reference = kSeedNearbyEnemyReference;   // 0081F25C
    block.nearest_move_dir_weight = kSeedNearestMoveDirWeight;  // 0081F269, XMM1 from 0081F205
    block.prev_move_dir_weight = kSeedPrevMoveDirWeight;        // 0081F242
    block.prev_move_dir_range = kSeedPrevMoveDirRange;          // 0081F24F
    block.range_override = kSeedUnset;                          // 0081F26E
    block.force_move_close_to_target = 1;                       // 0081F27D
    block.commanded_speed = kSeedUnset;                         // 0081F273
    block.commanded_speed_time = kSeedUnset;                    // 0081F278
}

void apply_navigator_params_from_settings_00822b70(const GameplayTuningSettings& settings,
                                                   UnitNavigatorParamsBlock& block,
                                                   bool reset) noexcept
{
    if (!reset) {
        return;  // 00822B70 CMP byte [ESP+4],0 / 00822B78 JZ 00822C13
    }
    block.allow_max_depth = 1;  // 00822B84
    block.my_damage_weight = settings.attack_move_director_my_damage_weight;              // 00822BAD
    block.ideal_dist_weight = settings.attack_move_director_ideal_dist_weight;            // 00822B99
    block.nearby_enemy_weight = settings.attack_move_director_nearby_enemy_weight;        // 00822BC0
    block.nearby_enemy_reference = settings.attack_move_director_nearby_enemy_reference;  // 00822BD4
    block.nearest_move_dir_weight = settings.attack_move_director_nearest_move_dir_weight;  // 00822BE8
    block.prev_move_dir_weight = settings.attack_move_director_prev_move_dir_weight;      // 00822BFC
    block.prev_move_dir_range = settings.attack_move_director_prev_move_dir_range;        // 00822C10
}

int ship_avoidance_side_filter_009f1b78(bool avoid_all_ship_collision) noexcept
{
    // 009F1B76 XOR ECX,ECX / 009F1B78 CMP byte [EAX+4],CL / 009F1B82 SETNZ CL /
    // 009F1B93 LEA ECX,[ECX*4-1].
    const int set = avoid_all_ship_collision ? 1 : 0;
    return set * 4 - 1;
}

bool ship_avoidance_side_accepted_009ec79b(int query_side, int record_side_filter) noexcept
{
    if (query_side == kShipAvoidanceSideAll) {
        return true;  // 009EC79F CMP ECX,3 / 009EC7A2 JZ 009EC7B3
    }
    if (record_side_filter == kShipAvoidanceSideAll) {
        return true;  // 009EC7AA CMP EAX,3 / 009EC7AD JZ 009EC7B3
    }
    return query_side == record_side_filter;  // 009EC7AF CMP ECX,EAX / 009EC7B1 JNZ
}

bool weapon_hit_accuracy_category_008387b0(int weapon_kind,
                                           WeaponHitAccuracyCategory& out) noexcept
{
    switch (weapon_kind) {
    case 2:
    case 3:
    case 4:
    case 6:
        out = WeaponHitAccuracyCategory::Artillery;  // 008387D3 ADD ECX,240h
        return true;
    case 1:
    case 5:
        out = WeaponHitAccuracyCategory::AntiAir;  // 00838809 ADD ECX,298h
        return true;
    case 7:
        out = WeaponHitAccuracyCategory::Torpedo;  // 0083883A ADD ECX,2F0h
        return true;
    case 8:
    case 9:
        out = WeaponHitAccuracyCategory::DepthCharge;  // 00838870 ADD ECX,348h
        return true;
    default:
        return false;  // 00838891 FLD [ESP+0Ch] / RET 10h, the scale unchanged
    }
}

const ShipAiSettingsKeyRecord* ship_ai_settings_keys(std::size_t& count) noexcept
{
    count = sizeof(kKeys) / sizeof(kKeys[0]);
    return kKeys;
}

const ShipAiSettingsReadSite* ship_ai_settings_read_sites(std::size_t& count) noexcept
{
    count = sizeof(kReadSites) / sizeof(kReadSites[0]);
    return kReadSites;
}

}  // namespace bsp
