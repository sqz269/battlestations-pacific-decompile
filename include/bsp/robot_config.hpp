#pragma once
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/lua_script_runtime.hpp"
#include "bsp/lua_state_owner.hpp"
#include "bsp/native_string.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>

namespace bsp {
// Exact Win32 header. Construction leaves +4 and every level byte untouched.
// name_08 is the actual independently duplicated char buffer, not NativeString.
struct RobotDescriptor {
    std::uint32_t native_vtable_00;
    float no_target_time_until_rest_04;
    char* name_08;
};
static_assert(sizeof(RobotDescriptor) == 0x0c);
static_assert(offsetof(RobotDescriptor, name_08) == 8);
inline constexpr std::uint32_t kRobotBaseVtable = 0x00d17dc0;

// Reader 008fc6d0; parameter names are evidence-based hypotheses.
struct AAFlakBotParameters {
    float good_ratio_00c;
    float angle_err_min_010;
    float angle_err_min_014;
    float angle_err_bad_018;
    float dist_err_min_01c;
    float dist_err_max_020;
    float dist_err_bad_024;
    float bullet_throw_mul_028;
};
static_assert(sizeof(AAFlakBotParameters) == 0x20);
struct AAFlakBotConfig {
    RobotDescriptor descriptor;
    std::array<AAFlakBotParameters, 6> levels;
};
static_assert(offsetof(AAFlakBotConfig, levels) == 0x0c);
static_assert(sizeof(AAFlakBotConfig) == 0xcc);

// Reader 008fca10; parameter names are evidence-based hypotheses.
struct TailGunnerBotParameters {
    float angle_error_00c;
    float aim_period_min_010;
    float aim_period_max_014;
    float shoot_range_018;
    float bullet_throw_mul_01c;
    float section_target_chance_020;
    float engine_room_weight_024;
    float magazine_weight_028;
    float fueltank_weight_02c;
};
static_assert(sizeof(TailGunnerBotParameters) == 0x24);
struct TailGunnerBotConfig {
    RobotDescriptor descriptor;
    std::array<TailGunnerBotParameters, 6> levels;
};
static_assert(offsetof(TailGunnerBotConfig, levels) == 0x0c);
static_assert(sizeof(TailGunnerBotConfig) == 0xe4);

// Reader 008fcd60; parameter names are evidence-based hypotheses.
struct AAGunnerBotParameters {
    float angle_diff_error_ratio_00c;
    float dist2_angle_err_ratio_010;
    float const_angle_error_014;
    float bullet_throw_mul_018;
};
static_assert(sizeof(AAGunnerBotParameters) == 0x10);
struct AAGunnerBotConfig {
    RobotDescriptor descriptor;
    std::array<AAGunnerBotParameters, 6> levels;
};
static_assert(offsetof(AAGunnerBotConfig, levels) == 0x0c);
static_assert(sizeof(AAGunnerBotConfig) == 0x6c);

// Reader 009973b0; parameter names are evidence-based hypotheses.
struct PilotBotParameters {
    float torp_release_alt_00c;
    float torp_release_dist_near_010;
    float torp_release_dist_far_014;
    float torp_release_drop_closer_mul_018;
    float torp_flik_flak_time_1_01c;
    float torp_flik_flak_time_2_020;
    float torp_calc_target_pos_error_024;
    float torp_targeth_error_028;
    float torp_targetv_error_02c;
    float torp_target_point_select_prec_030;
    float torp_throw_mul_034;
    float dive_bomb_calc_target_pos_error_038;
    float dive_bomb_targeth_error_03c;
    float dive_bomb_targetv_error_040;
    float dive_bomb_release_alt_1_044;
    float dive_bomb_release_alt_2_048;
    float dive_bomb_new_release_mul_04c;
    float dive_bomb_max_power_ctrl_050;
    float dive_bomb_min_power_ctrl_054;
    float dive_bomb_max_brake_ctrl_058;
    float dive_bomb_min_brake_ctrl_05c;
    float dive_bomb_aim_pitch_ratio_060;
    float dive_bomb_target_point_select_prec_064;
    float dive_bomb_aim_prec_dist_068;
    float dive_bomb_aim_prec_mul_06c;
    float dive_bomb_aim_prec_pull_plus_070;
    float dive_bomb_aim_prec_pull_minus_074;
    float dive_bomb_throw_mul_078;
    float dive_bomb_section_damage_chance_07c;
    float dive_bomb_engine_room_weight_080;
    float dive_bomb_magazine_weight_084;
    float dive_bomb_fueltank_weight_088;
    float kamikaze_section_damage_chance_08c;
    float kamikaze_engine_room_weight_090;
    float kamikaze_magazine_weight_094;
    float kamikaze_fueltank_weight_098;
    float kamikaze_target_precision_09c;
    float kamikaze_targeth_error_0a0;
    float kamikaze_targetv_error_0a4;
    float kamikaze_target_proj_time_error_0a8;
    float kamikaze_maneuver_precision_mul_1_0ac;
    float kamikaze_maneuver_precision_mul_2_0b0;
    float kamikaze_maneuver_precision_timer_0b4;
    float level_bomb_targeth_error_0b8;
    float level_bomb_targetv_error_0bc;
    float level_bomb_calc_target_pos_error_0c0;
    float level_bomb_target_point_select_prec_0c4;
    float level_bomb_throw_mul_0c8;
    float depth_charge_targeth_error_0cc;
    float depth_charge_targetv_error_0d0;
    float depth_charge_calc_target_pos_error_0d4;
    float depth_charge_cancel_target_dist_0d8;
    float depth_charge_target_point_select_prec_0dc;
    float depth_charge_throw_mul_0e0;
    float strafe_too_close_distance_0e4;
    float strafe_go_away_distance_0e8;
    float strafe_attack_angle_0ec;
    float strafe_target_point_select_prec_0f0;
    float strafe_section_damage_chance_0f4;
    float strafe_engine_room_weight_0f8;
    float strafe_magazine_weight_0fc;
    float strafe_fueltank_weight_100;
    float strike_too_close_104;
    float strike_go_away_distance_108;
    float strike_attack_angle_10c;
    float strike_target_prec_110;
    float strike_homing_angle_114;
    float strike_repeat_time_118;
    float strike_fire_angle_11c;
    float strike_fire_dist_1_120;
    float strike_fire_dist_2_124;
    float strike_fire_dist_3_128;
    float strike_throw_mul_12c;
    float strike_section_damage_chance_130;
    float strike_engine_room_weight_134;
    float strike_magazine_weight_138;
    float strike_fueltank_weight_13c;
    float rocket_check_time_1_140;
    float rocket_check_time_2_144;
    float rocket_small_plane_rocket_chance_148;
    float rocket_small_plane_attack_dist_14c;
    std::array<std::byte, 0x14> unconsumed_150;
    float rocket_small_plane_attack_time_164;
    std::int32_t rocket_small_plane_num_rockets_168;
    float rocket_small_plane_rocket_delay_16c;
    float rocket_small_plane_attack_angle_170;
    float rocket_small_plane_homing_angle_174;
    std::array<std::byte, 0x4> unconsumed_178;
    float rocket_large_plane_rocket_chance_17c;
    std::array<std::byte, 0x4> unconsumed_180;
    float rocket_large_plane_attack_dist_184;
    std::array<std::byte, 0x10> unconsumed_188;
    float rocket_large_plane_attack_time_198;
    std::int32_t rocket_large_plane_num_rockets_19c;
    float rocket_large_plane_rocket_delay_1a0;
    float rocket_large_plane_attack_angle_1a4;
    float rocket_large_plane_homing_angle_1a8;
    std::array<std::byte, 0x4> unconsumed_1ac;
    float rocket_ship_rocket_chance_1b0;
    std::array<std::byte, 0x8> unconsumed_1b4;
    float rocket_ship_attack_dist_1_1bc;
    float rocket_ship_attack_dist_2_1c0;
    float rocket_ship_attack_dist_3_1c4;
    std::array<std::byte, 0x4> unconsumed_1c8;
    float rocket_ship_attack_time_1cc;
    std::int32_t rocket_ship_num_rockets_1d0;
    float rocket_ship_rocket_delay_1d4;
    float rocket_ship_attack_angle_1d8;
    float rocket_ship_homing_angle_1dc;
    std::array<std::byte, 0x4> unconsumed_1e0;
    float rocket_landfort_rocket_chance_1e4;
    std::array<std::byte, 0x14> unconsumed_1e8;
    float rocket_landfort_attack_dist_1fc;
    float rocket_landfort_attack_time_200;
    std::int32_t rocket_landfort_num_rockets_204;
    float rocket_landfort_rocket_delay_208;
    float rocket_landfort_attack_angle_20c;
    float rocket_landfort_homing_angle_210;
    std::array<std::byte, 0x4> unconsumed_214;
    float dogfight_follow_dist_218;
    float dogfight_boring_time_21c;
    float dogfight_avoid_time_220;
    float dogfight_turn_after_chance_224;
    float dogfight_maneuver_change_time_228;
    float aim_distort_angle_1_22c;
    float aim_distort_angle_2_230;
    float aim_distort_change_speed_234;
    float aim_dont_shoot_area_238;
    float aim_shoot_distance_23c;
    float aim_shoot_time_2_240;
    float aim_shoot_time_1_244;
    float aim_shoot_delay_time_2_248;
    float aim_shoot_delay_time_1_24c;
    float aim_bullet_throw_mul_250;
};
static_assert(sizeof(PilotBotParameters) == 0x248);
struct PilotBotConfig {
    RobotDescriptor descriptor;
    std::array<PilotBotParameters, 6> levels;
};
static_assert(offsetof(PilotBotConfig, levels) == 0x0c);
static_assert(sizeof(PilotBotConfig) == 0xdbc);

// Reader 008fcf30; parameter names are evidence-based hypotheses.
struct ArtillerySubDirectorBotParameters {
    float max_error_radius_00c;
    float error_range_mul_010;
    float min_error_mul_014;
    float approach_mul_min_018;
    float approach_mul_max_01c;
    float deviation_mul_020;
    float angle_change_024;
    float error_dist_inc_start_time_028;
    float error_dist_inc_full_time_02c;
    float bullet_throw_mul_030;
};
static_assert(sizeof(ArtillerySubDirectorBotParameters) == 0x28);
struct ArtillerySubDirectorBotConfig {
    RobotDescriptor descriptor;
    std::array<ArtillerySubDirectorBotParameters, 6> levels;
};
static_assert(offsetof(ArtillerySubDirectorBotConfig, levels) == 0x0c);
static_assert(sizeof(ArtillerySubDirectorBotConfig) == 0xfc);

// Reader 008fd370; parameter names are evidence-based hypotheses.
struct ArtilleryGunnerBotParameters {
    float max_angle_error_00c;
    float power_010;
    float target_point_refresh_time_014;
    float section_target_chance_018;
    float engine_room_weight_01c;
    float magazine_weight_020;
    float fueltank_weight_024;
};
static_assert(sizeof(ArtilleryGunnerBotParameters) == 0x1c);
struct ArtilleryGunnerBotConfig {
    RobotDescriptor descriptor;
    std::array<ArtilleryGunnerBotParameters, 6> levels;
};
static_assert(offsetof(ArtilleryGunnerBotConfig, levels) == 0x0c);
static_assert(sizeof(ArtilleryGunnerBotConfig) == 0xb4);

// Reader 008fd640; parameter names are evidence-based hypotheses.
struct TorpedoBotParameters {
    float angle_err_min_00c;
    float angle_err_max_010;
    float fire_target_accuracy_014;
    float any_target_accuracy_018;
    float bullet_throw_mul_01c;
};
static_assert(sizeof(TorpedoBotParameters) == 0x14);
struct TorpedoBotConfig {
    RobotDescriptor descriptor;
    std::array<TorpedoBotParameters, 6> levels;
};
static_assert(offsetof(TorpedoBotConfig, levels) == 0x0c);
static_assert(sizeof(TorpedoBotConfig) == 0x84);

// Reader 008fd880; parameter names are evidence-based hypotheses.
struct DepthChargeBotParameters {
    float attack_dist_00c;
    float bullet_throw_mul_010;
    float continuous_fire_time_014;
    float fire_delay_1_018;
    float fire_delay_2_01c;
};
static_assert(sizeof(DepthChargeBotParameters) == 0x14);
struct DepthChargeBotConfig {
    RobotDescriptor descriptor;
    std::array<DepthChargeBotParameters, 6> levels;
};
static_assert(offsetof(DepthChargeBotConfig, levels) == 0x0c);
static_assert(sizeof(DepthChargeBotConfig) == 0x84);

// Reader 009d53b0; parameter names are evidence-based hypotheses.
struct NavigatorBotParameters {
    float sub_attack_dist_multiplier_00c;
    float torpedo_predict_1_010;
    float torpedo_predict_2_014;
    float torpedo_predict_reference_length_018;
    float torpedo_observation_1_01c;
    float torpedo_observation_2_020;
    float torpedo_observation_sub_addon_024;
    float torpedo_spd_err_1_028;
    float torpedo_spd_err_2_02c;
};
static_assert(sizeof(NavigatorBotParameters) == 0x24);
struct NavigatorBotConfig {
    RobotDescriptor descriptor;
    std::array<NavigatorBotParameters, 6> levels;
};
static_assert(offsetof(NavigatorBotConfig, levels) == 0x0c);
static_assert(sizeof(NavigatorBotConfig) == 0xe4);

// Native unique map F89994 stores borrowed class-name keys and raw descriptor
// pointers. This standard-container projection preserves uniqueness and pointer
// ownership, not the native tree node/iterator ABI. Duplicate insertion retains
// the old entry and does not delete the newly supplied descriptor.
struct RobotNameLess { bool operator()(const char*, const char*) const noexcept; };
using RobotConfigRegistry = std::map<const char*, RobotDescriptor*, RobotNameLess>;

struct RobotDescriptorStorage {
    virtual ~RobotDescriptorStorage() = default;
    // Actual allocator: return a fresh block of exactly the requested size or throw.
    // The constructor does not fill or synthesize the unconsumed allocation bytes.
    virtual void* allocate(std::uint32_t bytes) = 0;
    virtual void release(void*) noexcept = 0;
};
struct RobotConfigContext {
    NativeStringStorage& strings;
    RobotDescriptorStorage& descriptors;
    const bool& crt_sse2_conversion; // live0109EEA4 alias; outlives every call
};
struct RobotConfigAliases {
    RobotDescriptor*& pilot_f8a30c;
    RobotDescriptor*& tail_gunner_e199a0;
    RobotDescriptor*& aa_flak_e1999c;
    RobotDescriptor*& aa_gunner_e19998;
    RobotDescriptor*& artillery_subdirector_e19994;
    RobotDescriptor*& artillery_gunner_e19990;
    RobotDescriptor*& torpedo_e1998c;
    RobotDescriptor*& depth_charge_e19988;
    RobotDescriptor*& navigator_f8a688;
};

// Full00901610 returns true in AL after both refs and temporary Lua owner close.
// Existing owner/runtime use stock Lua5.1.1 and C++ error transport, not native SEH.
bool load_robot_config_00901610(RobotConfigRegistry&, RobotConfigAliases,
    LuaStateOwnerEnvironment, LuaScriptRuntime&, RobotConfigContext&);
//009013D0 ECX descriptor, EDX Robots LuaObject, RET. Reloads each virtual slot.
void register_robot_config_009013d0(RobotConfigRegistry&, RobotDescriptor&,
    GuiLua51Host&, GuiLuaRef robots, RobotConfigContext&);
//00900AF0 ECX class-name C string; returns existing descriptor or null.
RobotDescriptor* find_robot_config_00900af0(RobotConfigRegistry&, const char*);
// Native unique insertion primitive; pair returns iterator and insertion flag.
std::pair<RobotConfigRegistry::iterator, bool> insert_robot_config_00901210(
    RobotConfigRegistry&, const char* borrowed_name, RobotDescriptor*);
// Six-level validators use actual descriptor storage and preserve native FP order.
bool validate_robot_descriptor(RobotDescriptor&);
// Canonical scalar-deleting wrappers share base reset, name free/null, flags&1
// block release, then return the original pointer even after releasing its block.
RobotDescriptor* delete_robot_descriptor(RobotDescriptor*, std::uint32_t flags,
    RobotDescriptorStorage&);
//00900BB0 deletes registered descriptors in map order then clears the map.
// Native does not clear the separate aliases, which become dangling.
void clear_robot_registry_00900bb0(RobotConfigRegistry&, RobotDescriptorStorage&);
} // namespace bsp
