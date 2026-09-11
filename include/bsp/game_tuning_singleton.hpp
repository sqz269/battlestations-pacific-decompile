#pragma once
// The 6D0h-byte plane tuning singleton that 0042E740 caches in DAT_00F87440 and
// 007E2A20 builds. Field names are hypotheses derived from the recovered Lua key
// paths; no recovered symbol names this object or any of its fields. See
// docs/GAME_TUNING_SINGLETON.md for the evidence behind every offset.
#include <cstddef>
#include <cstdint>

namespace bsp {

// How 007E2A20 reads one key. The address is the native getter it calls.
enum class GameTuningValueKind {
    Number,          // 00B66270, no fallback
    Integer,         // 00B66290, one site: Dynamics/RotationLimit
    Boolean,         // 00B66250, seven sites
    NumberOrDefault, // 00B66330, 37 sites, fallback in the row
    NumberTriple,    // 00B67A80, two sites, fills three consecutive floats
};

// One row of the load order of 007E2A20. `path` is the key path inside the global
// PlaneGlobals table, '/' separated, with one-based indices for the array keys.
struct GameTuningKey {
    const char* path;
    std::uint16_t offset;
    GameTuningValueKind kind;
    float fallback;      // meaningful only for NumberOrDefault
    std::uint32_t read_site; // the native getter call site
};

inline constexpr std::size_t kGameTuningBlockSize = 0x6D0;
inline constexpr std::size_t kGameTuningKeyCount = 439;
extern const GameTuningKey kGameTuningKeys[kGameTuningKeyCount];

// The object. Every offset below is a store 007E2A20 makes; the trailing
// reserved bytes are the padding after the two boolean bytes at +3E8h.
struct GameTuningBlock {
    const void* vtable; // 00d08628, stored at 007E2A4C
    float close_to_camera_dist; // +4 CloseToCameraDist
    float move_detail_lod_error; // +8 MoveDetailLODError
    float move_detail_radius; // +c MoveDetailRadius
    float death_mode_chances_explosion; // +10 DeathModeChances/Explosion
    float death_mode_chances_explosion_delayed; // +14 DeathModeChances/Explosion_delayed
    float death_mode_chances_spinning; // +18 DeathModeChances/Spinning
    float death_mode_chances_powerloss; // +1c DeathModeChances/Powerloss
    float death_mode_chances_explodetoparts; // +20 DeathModeChances/Explodetoparts
    float sound_wind_vol_min_spd_ratio; // +24 Sound/WindVolMinSpdRatio
    float sound_wind_vol_max_spd_ratio; // +28 Sound/WindVolMaxSpdRatio
    float sound_wind_pitch_min_spd_ratio; // +2c Sound/WindPitchMinSpdRatio
    float sound_wind_pitch_max_spd_ratio; // +30 Sound/WindPitchMaxSpdRatio
    float sound_wind_pitch_min; // +34 Sound/WindPitchMin
    float sound_wind_pitch_max; // +38 Sound/WindPitchMax
    float sound_fall_vol_min_spd_ratio; // +3c Sound/FallVolMinSpdRatio
    float sound_fall_vol_max_spd_ratio; // +40 Sound/FallVolMaxSpdRatio
    float sound_fall_pitch_min_spd_ratio; // +44 Sound/FallPitchMinSpdRatio
    float sound_fall_pitch_max_spd_ratio; // +48 Sound/FallPitchMaxSpdRatio
    float sound_fall_pitch_min; // +4c Sound/FallPitchMin
    float sound_fall_pitch_max; // +50 Sound/FallPitchMax
    float plane_camera_z_rot_mul; // +54 PlaneCamera/ZRotMul
    float plane_camera_z_rot_smooth_rate; // +58 PlaneCamera/ZRotSmoothRate
    float plane_camera_x_dist_mul; // +5c PlaneCamera/XDistMul
    float plane_camera_x_dist_smooth_rate; // +60 PlaneCamera/XDistSmoothRate
    float plane_camera_y_dist_mul; // +64 PlaneCamera/YDistMul
    float plane_camera_y_dist_smooth_rate; // +68 PlaneCamera/YDistSmoothRate
    float plane_camera_y_dist_roll_mul; // +6c PlaneCamera/YDistRollMul
    float plane_camera_y_dist_roll_smooth_rate; // +70 PlaneCamera/YDistRollSmoothRate
    float plane_camera_z_dist_mul; // +74 PlaneCamera/ZDistMul
    float plane_camera_z_dist_smooth_rate; // +78 PlaneCamera/ZDistSmoothRate
    float plane_camera_ship_yard_dist; // +7c PlaneCamera/ShipYardDist
    float plane_camera_look_around_smooth_rate; // +80 PlaneCamera/LookAroundSmoothRate default 10
    float plane_camera_cockpit_fov_mul; // +84 PlaneCamera/CockpitFOVMul default 0.65
    float plane_camera_cockpit_smooth; // +88 PlaneCamera/CockpitSmooth default 0.5
    float plane_camera_cockpit_yaw_turn; // +8c PlaneCamera/CockpitYawTurn default 0.1
    float plane_camera_cockpit_pitch_turn; // +90 PlaneCamera/CockpitPitchTurn default 0.1
    float plane_camera_cockpit_roll_turn; // +94 PlaneCamera/CockpitRollTurn default 0.05
    float plane_camera_cockpit_roll_h_turn; // +98 PlaneCamera/CockpitRollHTurn default 0.05
    float plane_camera_cockpit_roll_v_turn; // +9c PlaneCamera/CockpitRollVTurn default 0.05
    float plane_camera_cockpit_view_h_max; // +a0 PlaneCamera/CockpitViewHMax default 1.5
    float plane_camera_cockpit_view_v_max; // +a4 PlaneCamera/CockpitViewVMax default 1
    float plane_camera_cockpit_view_v_min; // +a8 PlaneCamera/CockpitViewVMin default 0.7
    float plane_camera_cockpit_max_head_move_dist; // +ac PlaneCamera/CockpitMaxHeadMoveDist default 0.08
    float plane_camera_cockpit_gunfire_effect_size; // +b0 PlaneCamera/CockpitGunfireEffectSize default 0.008
    float plane_camera_cockpit_gunfire_effect_time; // +b4 PlaneCamera/CockpitGunfireEffectTime default 0.1
    float plane_camera_fov_min_speed; // +b8 PlaneCamera/FOVMinSpeed
    float plane_camera_fov_max_speed; // +bc PlaneCamera/FOVMaxSpeed
    float plane_camera_fov_min_spd_mul; // +c0 PlaneCamera/FOVMinSpdMul
    float plane_camera_fov_max_spd_mul; // +c4 PlaneCamera/FOVMaxSpdMul
    float plane_camera_fov_min_accel; // +c8 PlaneCamera/FOVMinAccel
    float plane_camera_fov_max_accel; // +cc PlaneCamera/FOVMaxAccel
    float plane_camera_fov_accel_mul; // +d0 PlaneCamera/FOVAccelMul
    std::uint8_t reserved_d4[0x4];
    float plane_camera_turbo_motion_blur_min_speed; // +d8 PlaneCamera/TurboMotionBlurMinSpeed default 80
    float plane_camera_turbo_motion_blur_max_speed; // +dc PlaneCamera/TurboMotionBlurMaxSpeed default 100
    float plane_camera_turbo_motion_blur_max_blur; // +e0 PlaneCamera/TurboMotionBlurMaxBlur default 0.1
    float bomb_camera_min_camera_alt; // +e4 BombCamera/MinCameraAlt
    float bomb_camera_camera_pos_smooth; // +e8 BombCamera/CameraPosSmooth
    float bomb_camera_camera_pos_smooth_2; // +ec BombCamera/CameraPosSmooth
    float bomb_camera_cam_vel_blender_acceleration; // +f0 BombCamera/CamVelBlenderAcceleration
    float bomb_camera_max_cam_vel_blender; // +f4 BombCamera/MaxCamVelBlender
    float bomb_camera_torpedo_near_alt_2; // +f8 BombCamera/TorpedoNearAlt/2
    float bomb_camera_torpedo_near_alt_1; // +fc BombCamera/TorpedoNearAlt/1
    float bomb_camera_torpedo_follow_dist_near; // +100 BombCamera/TorpedoFollowDistNear
    float bomb_camera_torpedo_follow_dist_far; // +104 BombCamera/TorpedoFollowDistFar
    float bomb_camera_rocket_near_alt_2; // +108 BombCamera/RocketNearAlt/2
    float bomb_camera_rocket_near_alt_1; // +10c BombCamera/RocketNearAlt/1
    float bomb_camera_rocket_follow_dist_near; // +110 BombCamera/RocketFollowDistNear
    float bomb_camera_rocket_follow_dist_far; // +114 BombCamera/RocketFollowDistFar
    float bomb_camera_bomb_near_alt_2; // +118 BombCamera/BombNearAlt/2
    float bomb_camera_bomb_near_alt_1; // +11c BombCamera/BombNearAlt/1
    float bomb_camera_bomb_follow_dist_near; // +120 BombCamera/BombFollowDistNear
    float bomb_camera_bomb_follow_dist_far; // +124 BombCamera/BombFollowDistFar
    float bomb_camera_bullet_near_alt_2; // +128 BombCamera/BulletNearAlt/2
    float bomb_camera_bullet_near_alt_1; // +12c BombCamera/BulletNearAlt/1
    float bomb_camera_bullet_follow_dist_near; // +130 BombCamera/BulletFollowDistNear
    float bomb_camera_bullet_follow_dist_far; // +134 BombCamera/BulletFollowDistFar
    float bomb_camera_final_dist; // +138 BombCamera/FinalDist
    float bomb_camera_bomb_sub_dist_limit; // +13c BombCamera/BombSubDistLimit
    float bomb_camera_bomb_sub_dist_min; // +140 BombCamera/BombSubDistMin
    float bomb_camera_bomb_sub_dist_mul; // +144 BombCamera/BombSubDistMul
    float bomb_camera_torpedo_sub_dist; // +148 BombCamera/TorpedoSubDist
    float bomb_camera_torpedo_sub_dist2; // +14c BombCamera/TorpedoSubDist2
    float bomb_camera_torpedo_sub_angle; // +150 BombCamera/TorpedoSubAngle
    float bomb_camera_torpedo_sub_angle2; // +154 BombCamera/TorpedoSubAngle2
    float multi_player_sync_send_mul; // +158 MultiPlayer/SyncSendMul default 1
    float multi_player_sync_upload_limit_k_bit_per_sec; // +15c MultiPlayer/SyncUploadLimitKBitPerSec default 1024
    float rotor_speed_base; // +160 Rotor/SpeedBase
    float rotor_still_multiplier; // +164 Rotor/StillMultiplier
    float rotor_speed_random; // +168 Rotor/SpeedRandom
    float rotor_still_only_speed; // +16c Rotor/StillOnlySpeed
    float rotor_blurred_only_speed; // +170 Rotor/BlurredOnlySpeed
    float rotor_idle_power_speed; // +174 Rotor/IdlePowerSpeed
    float rotor_max_power_speed; // +178 Rotor/MaxPowerSpeed
    float rotor_rotor_speed_change; // +17c Rotor/RotorSpeedChange
    float air_field_turn_multiplier; // +180 AirField/TurnMultiplier
    float air_field_move_spd; // +184 AirField/MoveSpd
    float air_field_min_turn_spd; // +188 AirField/MinTurnSpd
    float air_field_player_control_spd; // +18c AirField/PlayerControlSpd
    float air_field_plane_send_interval; // +190 AirField/PlaneSendInterval
    float camera_shake_roll_pitch_mult; // +194 CameraShake/RollPitchMult
    float camera_shake_power_mult; // +198 CameraShake/PowerMult
    float camera_shake_speed_mult; // +19c CameraShake/SpeedMult
    float camera_shake_limit; // +1a0 CameraShake/Limit
    float camera_shake_ratio; // +1a4 CameraShake/Ratio
    float camera_shake_pause_len_min; // +1a8 CameraShake/PauseLenMin
    float camera_shake_pause_len_max; // +1ac CameraShake/PauseLenMax
    float camera_shake_shake_len_min; // +1b0 CameraShake/ShakeLenMin
    float camera_shake_shake_len_max; // +1b4 CameraShake/ShakeLenMax
    float camera_shake_random_len_factor; // +1b8 CameraShake/RandomLenFactor
    float camera_shake_force_shake_limit; // +1bc CameraShake/ForceShakeLimit
    float wanderer_speed_range_1; // +1c0 Wanderer/SpeedRange/1
    float wanderer_speed_range_2; // +1c4 Wanderer/SpeedRange/2
    float wanderer_roll_change_chance; // +1c8 Wanderer/RollChangeChance
    float wanderer_roll_change_max; // +1cc Wanderer/RollChangeMax
    float wanderer_roll_change_decay; // +1d0 Wanderer/RollChangeDecay
    float wanderer_roll_change_speed; // +1d4 Wanderer/RollChangeSpeed
    float wanderer_time_range_1; // +1d8 Wanderer/TimeRange/1
    float wanderer_time_range_2; // +1dc Wanderer/TimeRange/2
    float wanderer_offset_max; // +1e0 Wanderer/OffsetMax
    float wanderer_change_mul; // +1e4 Wanderer/ChangeMul
    float wanderer_accel_max; // +1e8 Wanderer/AccelMax
    float wanderer_speed_max; // +1ec Wanderer/SpeedMax
    float wanderer_accel_decay_time; // +1f0 Wanderer/AccelDecayTime
    float wanderer_speed_decay_time; // +1f4 Wanderer/SpeedDecayTime
    float wanderer_offset_decay_time; // +1f8 Wanderer/OffsetDecayTime
    float wanderer_small_plane_decal_mul; // +1fc Wanderer/SmallPlaneDecalMul
    float wanderer_small_plane_roll_decay_mul; // +200 Wanderer/SmallPlaneRollDecayMul
    float wanderer_small_plane_accel_mul; // +204 Wanderer/SmallPlaneAccelMul
    float wanderer_small_plane_time_mul; // +208 Wanderer/SmallPlaneTimeMul
    float wanderer_small_plane_offset_mul; // +20c Wanderer/SmallPlaneOffsetMul
    float dynamics_ceiling; // +210 Dynamics/Ceiling
    float dynamics_ceiling_force; // +214 Dynamics/CeilingForce
    std::int32_t dynamics_rotation_limit; // +218 Dynamics/RotationLimit
    float dynamics_rotation_factors_a; // +21c Dynamics/RotationFactors/A
    float dynamics_rotation_factors_b; // +220 Dynamics/RotationFactors/B
    float dynamics_rotation_factors_c; // +224 Dynamics/RotationFactors/C
    float dynamics_drag_func_power; // +228 Dynamics/DragFuncPower
    float dynamics_spd_multipliers_stall_range_min; // +22c Dynamics/SpdMultipliers/StallRangeMin
    float dynamics_spd_multipliers_stall_range_max; // +230 Dynamics/SpdMultipliers/StallRangeMax
    float dynamics_spd_multipliers_stall_off_pitch; // +234 Dynamics/SpdMultipliers/StallOffPitch
    float dynamics_spd_multipliers_stall_on_pitch; // +238 Dynamics/SpdMultipliers/StallOnPitch
    float dynamics_spd_multipliers_control_range_min; // +23c Dynamics/SpdMultipliers/ControlRangeMin
    float dynamics_spd_multipliers_control_range_max; // +240 Dynamics/SpdMultipliers/ControlRangeMax
    float dynamics_spd_multipliers_drag_range_min; // +244 Dynamics/SpdMultipliers/DragRangeMin
    float dynamics_spd_multipliers_drag_range_max; // +248 Dynamics/SpdMultipliers/DragRangeMax
    float dynamics_spd_multipliers_level_flight; // +24c Dynamics/SpdMultipliers/LevelFlight
    float dynamics_dead_meat_rotation_min; // +250 Dynamics/DeadMeat/RotationMin
    float dynamics_dead_meat_rotation_min_2; // +254 Dynamics/DeadMeat/RotationMin
    float dynamics_dead_meat_spin_roll_spd; // +258 Dynamics/DeadMeat/SpinRollSpd
    float dynamics_dead_meat_roll_mul_time; // +25c Dynamics/DeadMeat/RollMulTime
    float dynamics_dead_meat_roll_mul; // +260 Dynamics/DeadMeat/RollMul
    float dynamics_dead_meat_lost_drag_time; // +264 Dynamics/DeadMeat/LostDragTime
    float dynamics_dead_meat_extra_gravity_mul; // +268 Dynamics/DeadMeat/ExtraGravityMul
    float dynamics_dead_meat_spin_stall_mul; // +26c Dynamics/DeadMeat/SpinStallMul
    float dynamics_dead_meat_spin_stall_mul_time; // +270 Dynamics/DeadMeat/SpinStallMulTime
    float dynamics_dead_meat_spin_stall_mul_on_pitch; // +274 Dynamics/DeadMeat/SpinStallMulOnPitch
    float dynamics_dead_meat_spin_stall_mul_off_pitch; // +278 Dynamics/DeadMeat/SpinStallMulOffPitch
    float dynamics_dead_meat_stall_mul; // +27c Dynamics/DeadMeat/StallMul
    float dynamics_dead_meat_stall_mul_time; // +280 Dynamics/DeadMeat/StallMulTime
    float dynamics_dead_meat_stall_mul_on_pitch; // +284 Dynamics/DeadMeat/StallMulOnPitch
    float dynamics_dead_meat_stall_mul_off_pitch; // +288 Dynamics/DeadMeat/StallMulOffPitch
    float derived_28c; // +28c derived, see the doc
    float dynamics_wheel_friction; // +290 Dynamics/WheelFriction
    float dynamics_wheel_friction_speed_1; // +294 Dynamics/WheelFrictionSpeed/1
    float dynamics_wheel_friction_speed_2; // +298 Dynamics/WheelFrictionSpeed/2
    float dynamics_wheel_friction_accel_1; // +29c Dynamics/WheelFrictionAccel/1
    float dynamics_wheel_friction_accel_2; // +2a0 Dynamics/WheelFrictionAccel/2
    float dynamics_runway_smooth_strength; // +2a4 Dynamics/RunwaySmoothStrength
    float dynamics_runway_yaw_turn_spd_limit_1; // +2a8 Dynamics/RunwayYawTurnSpdLimit/1
    float dynamics_runway_yaw_turn_spd_limit_1_2; // +2ac Dynamics/RunwayYawTurnSpdLimit/1
    float dynamics_runway_yaw_turn_spd_mul; // +2b0 Dynamics/RunwayYawTurnSpdMul
    float dynamics_water_max_v_spd; // +2b4 Dynamics/Water/MaxVSpd
    float dynamics_water_max_down_pitch; // +2b8 Dynamics/Water/MaxDownPitch
    float dynamics_water_max_up_pitch; // +2bc Dynamics/Water/MaxUpPitch
    float dynamics_water_max_roll; // +2c0 Dynamics/Water/MaxRoll
    float dynamics_water_yaw_control_factor; // +2c4 Dynamics/Water/YawControlFactor
    float dynamics_water_normal_yaw_control_spd; // +2c8 Dynamics/Water/NormalYawControlSpd
    float dynamics_water_max_yaw_control_spd; // +2cc Dynamics/Water/MaxYawControlSpd
    float dynamics_water_max_yaw_control; // +2d0 Dynamics/Water/MaxYawControl
    float dynamics_water_max_depth; // +2d4 Dynamics/Water/MaxDepth
    float dynamics_water_lift_depth_ratio; // +2d8 Dynamics/Water/LiftDepthRatio
    float dynamics_water_lift_max; // +2dc Dynamics/Water/LiftMax
    float dynamics_water_decel_speed; // +2e0 Dynamics/Water/DecelSpeed
    float dynamics_water_side_drag_ratio; // +2e4 Dynamics/Water/SideDragRatio
    float dynamics_water_max_side_drag; // +2e8 Dynamics/Water/MaxSideDrag
    float dynamics_water_max_ctrl_angle; // +2ec Dynamics/Water/MaxCtrlAngle
    float dynamics_water_min_ctrl_angle; // +2f0 Dynamics/Water/MinCtrlAngle
    float dynamics_water_take_off_max_length; // +2f4 Dynamics/Water/TakeOffMaxLength
    float dynamics_water_take_off_min_length; // +2f8 Dynamics/Water/TakeOffMinLength
    float dynamics_water_min_drag_spd; // +2fc Dynamics/Water/MinDragSpd
    float dynamics_water_lift_begin_dive_mul; // +300 Dynamics/Water/LiftBeginDiveMul
    float dynamics_water_lift_rotate_max; // +304 Dynamics/Water/LiftRotateMax
    float dynamics_water_lift_rotate_depth_ratio; // +308 Dynamics/Water/LiftRotateDepthRatio
    float dynamics_water_lift_rotate_angle_ratio; // +30c Dynamics/Water/LiftRotateAngleRatio
    float dynamics_max_drag_spd_mul; // +310 Dynamics/MaxDragSpdMul
    float dynamics_min_drag_spd_mul; // +314 Dynamics/MinDragSpdMul
    float dynamics_max_drag_pitch; // +318 Dynamics/MaxDragPitch
    float dynamics_accel_cheat_mul; // +31c Dynamics/AccelCheatMul
    float dynamics_accel_cheat_mul_mul; // +320 Dynamics/AccelCheatMulMul
    float dynamics_accel_cheat_fall_mul; // +324 Dynamics/AccelCheatFallMul
    float dynamics_accel_cheat_fall_pitch_range_1; // +328 Dynamics/AccelCheatFallPitchRange/1
    float dynamics_accel_cheat_fall_pitch_range_2; // +32c Dynamics/AccelCheatFallPitchRange/2
    float dynamics_spd_multipliers_turbo_multiplier; // +330 Dynamics/SpdMultipliers/TurboMultiplier
    float dynamics_spd_multipliers_new_travel_speed_mul; // +334 Dynamics/SpdMultipliers/NewTravelSpeedMul
    float dynamics_spd_multipliers_dive_bomb_slow_mul; // +338 Dynamics/SpdMultipliers/DiveBombSlowMul
    float dynamics_spd_multipliers_torpedo_bomb_slow_mul; // +33c Dynamics/SpdMultipliers/TorpedoBombSlowMul
    float dynamics_spd_multipliers_depth_charge_slow_mul; // +340 Dynamics/SpdMultipliers/DepthChargeSlowMul
    float dynamics_spd_multipliers_level_bomb_slow_mul; // +344 Dynamics/SpdMultipliers/LevelBombSlowMul
    float retreat_exit_dist; // +348 Retreat/ExitDist
    float retreat_exit_time; // +34c Retreat/ExitTime
    float retreat_warning_repeat_time; // +350 Retreat/WarningRepeatTime
    float paratrooper_drop_min_altitude; // +354 ParatrooperDrop/MinAltitude
    float paratrooper_drop_max_altitude; // +358 ParatrooperDrop/MaxAltitude
    float pilot_move_to_small_plane_travel_alt; // +35c Pilot/MoveTo/SmallPlaneTravelAlt
    float pilot_move_to_large_plane_travel_alt; // +360 Pilot/MoveTo/LargePlaneTravelAlt
    float pilot_move_to_travel_alt_random; // +364 Pilot/MoveTo/TravelAltRandom
    float pilot_move_to_follow_dist_1; // +368 Pilot/MoveTo/FollowDist/1
    float pilot_move_to_follow_dist_2; // +36c Pilot/MoveTo/FollowDist/2
    float pilot_move_to_closing_dist; // +370 Pilot/MoveTo/ClosingDist
    float pilot_move_to_switch_next_point_time; // +374 Pilot/MoveTo/SwitchNextPointTime
    float pilot_move_to_circle_alt_diff; // +378 Pilot/MoveTo/CircleAltDiff
    float pilot_move_to_reference_speed; // +37c Pilot/MoveTo/ReferenceSpeed
    float pilot_follow_followed_point_dist; // +380 Pilot/Follow/FollowedPointDist
    float pilot_follow_leader_follow_alt; // +384 Pilot/Follow/LeaderFollowAlt
    float pilot_follow_safe_alt; // +388 Pilot/Follow/SafeAlt
    float pilot_follow_small_plane_turn_mul; // +38c Pilot/Follow/SmallPlaneTurnMul
    float pilot_follow_large_plane_turn_mul; // +390 Pilot/Follow/LargePlaneTurnMul
    float pilot_follow_good_position_dir; // +394 Pilot/Follow/GoodPositionDir
    float pilot_follow_good_position_dist; // +398 Pilot/Follow/GoodPositionDist
    float pilot_follow_good_position_spd_treshold; // +39c Pilot/Follow/GoodPositionSpdTreshold
    float pilot_follow_good_position_spd_diff; // +3a0 Pilot/Follow/GoodPositionSpdDiff
    float pilot_follow_max_follow_spd_target_dir; // +3a4 Pilot/Follow/MaxFollowSpdTargetDir
    float pilot_follow_min_follow_spd_target_dir; // +3a8 Pilot/Follow/MinFollowSpdTargetDir
    float pilot_follow_dont_wait_for_hdg_diff; // +3ac Pilot/Follow/DontWaitForHdgDiff
    float pilot_follow_wait_for_hdg_diff; // +3b0 Pilot/Follow/WaitForHdgDiff
    float pilot_follow_nearby_dist; // +3b4 Pilot/Follow/NearbyDist
    float pilot_follow_tight_turn; // +3b8 Pilot/Follow/TightTurn
    float pilot_follow_leader_heading_spd_time_1; // +3bc Pilot/Follow/LeaderHeadingSpdTime/1
    float pilot_follow_leader_heading_spd_time_2; // +3c0 Pilot/Follow/LeaderHeadingSpdTime/2
    float pilot_follow_leader_heading_spd_dist_1; // +3c4 Pilot/Follow/LeaderHeadingSpdDist/1
    float pilot_follow_leader_heading_spd_dist_2; // +3c8 Pilot/Follow/LeaderHeadingSpdDist/2
    float derived_3cc; // +3cc derived, see the doc
    float pilot_follow_small_plane_displacement; // +3d0 Pilot/Follow/SmallPlaneDisplacement
    float pilot_follow_small_plane_displacement_2; // +3d4 Pilot/Follow/SmallPlaneDisplacement
    float pilot_follow_small_plane_displacement_3; // +3d8 Pilot/Follow/SmallPlaneDisplacement
    float pilot_follow_bomber_displacement; // +3dc Pilot/Follow/BomberDisplacement
    float pilot_follow_bomber_displacement_2; // +3e0 Pilot/Follow/BomberDisplacement
    float pilot_follow_bomber_displacement_3; // +3e4 Pilot/Follow/BomberDisplacement
    bool pilot_follow_symmetrical_position; // +3e8 Pilot/Follow/SymmetricalPosition
    bool pilot_follow_symmetrical_altitude; // +3e9 Pilot/Follow/SymmetricalAltitude
    std::uint8_t reserved_3ea[0x2];
    float pilot_follow_yf_hdg_rad; // +3ec Pilot/Follow/yf_hdg_rad
    float pilot_follow_yf_yaw_v_rad_per_sec; // +3f0 Pilot/Follow/yf_yawV_radPerSec
    float pilot_follow_yf_sidepos_meter; // +3f4 Pilot/Follow/yf_sidepos_meter
    float pilot_follow_yf_sidedir; // +3f8 Pilot/Follow/yf_sidedir
    float pilot_follow_pf_pitch_rad; // +3fc Pilot/Follow/pf_pitch_rad
    float pilot_follow_pf_pitch_v_rad_per_sec; // +400 Pilot/Follow/pf_pitchV_radPerSec
    float pilot_follow_pf_vertpos_meter; // +404 Pilot/Follow/pf_vertpos_meter
    float pilot_follow_pf_vertdir; // +408 Pilot/Follow/pf_vertdir
    float pilot_follow_rf_roll_rad; // +40c Pilot/Follow/rf_roll_rad
    float pilot_follow_rf_roll_v_rad_per_sec; // +410 Pilot/Follow/rf_rollV_radPerSec
    float pilot_follow_rf_hdg_rad; // +414 Pilot/Follow/rf_hdg_rad
    float pilot_follow_rf_hdg_v_rad_per_sec; // +418 Pilot/Follow/rf_hdgV_radPerSec
    float pilot_follow_pwr_back_meter; // +41c Pilot/Follow/pwr_back_meter
    float pilot_follow_pwr_spd_meter_per_sec; // +420 Pilot/Follow/pwr_spd_meterPerSec
    float pilot_close_to_ship_cruising_alt; // +424 Pilot/CloseToShip/CruisingAlt
    float pilot_close_to_ship_drop_alt; // +428 Pilot/CloseToShip/DropAlt
    float pilot_close_to_ship_reference_speed; // +42c Pilot/CloseToShip/ReferenceSpeed
    float pilot_torpedo_cruising_alt; // +430 Pilot/Torpedo/CruisingAlt
    float pilot_torpedo_attack_dist; // +434 Pilot/Torpedo/AttackDist
    float pilot_torpedo_safe_dist; // +438 Pilot/Torpedo/SafeDist
    std::int32_t pilot_torpedo_move_on_cruising_alt; // +43c Pilot/Torpedo/MoveOnCruisingAlt (00B66250 into a dword slot)
    float pilot_torpedo_reference_speed; // +440 Pilot/Torpedo/ReferenceSpeed
    float pilot_level_bomb_cruising_alt; // +444 Pilot/LevelBomb/CruisingAlt
    float pilot_level_bomb_drop_alt; // +448 Pilot/LevelBomb/DropAlt
    float pilot_level_bomb_attack_dist; // +44c Pilot/LevelBomb/AttackDist
    float pilot_level_bomb_safe_dist; // +450 Pilot/LevelBomb/SafeDist
    std::int32_t pilot_level_bomb_move_on_cruising_alt; // +454 Pilot/LevelBomb/MoveOnCruisingAlt (00B66250 into a dword slot)
    float pilot_level_bomb_reference_speed; // +458 Pilot/LevelBomb/ReferenceSpeed
    float pilot_kamikaze_rocket_like_cruising_alt; // +45c Pilot/Kamikaze/RocketLike/CruisingAlt default 1200
    float pilot_kamikaze_rocket_like_drop_dist; // +460 Pilot/Kamikaze/RocketLike/DropDist default 2000
    float pilot_kamikaze_rocket_like_attack_range; // +464 Pilot/Kamikaze/RocketLike/AttackRange default 1400
    float pilot_kamikaze_rocket_like_attack_alt; // +468 Pilot/Kamikaze/RocketLike/AttackAlt default 500
    float pilot_kamikaze_rocket_like_turbo_range; // +46c Pilot/Kamikaze/RocketLike/TurboRange default 1000
    float pilot_kamikaze_rocket_like_turbo_angle; // +470 Pilot/Kamikaze/RocketLike/TurboAngle default 0.2
    float pilot_kamikaze_rocket_like_reference_speed; // +474 Pilot/Kamikaze/RocketLike/ReferenceSpeed default 180
    float pilot_kamikaze_fighter_like_cruising_alt; // +478 Pilot/Kamikaze/FighterLike/CruisingAlt default 1200
    float pilot_kamikaze_fighter_like_drop_dist; // +47c Pilot/Kamikaze/FighterLike/DropDist default 1200
    float pilot_kamikaze_fighter_like_attack_range; // +480 Pilot/Kamikaze/FighterLike/AttackRange default 1200
    float pilot_kamikaze_fighter_like_attack_alt; // +484 Pilot/Kamikaze/FighterLike/AttackAlt default 600
    float pilot_kamikaze_fighter_like_turbo_range; // +488 Pilot/Kamikaze/FighterLike/TurboRange default 500
    float pilot_kamikaze_fighter_like_turbo_angle; // +48c Pilot/Kamikaze/FighterLike/TurboAngle default 0
    float pilot_kamikaze_fighter_like_reference_speed; // +490 Pilot/Kamikaze/FighterLike/ReferenceSpeed default 100
    float pilot_depth_charge_aim_alt_range_1; // +494 Pilot/DepthCharge/AimAltRange/1
    float pilot_depth_charge_aim_alt_range_2; // +498 Pilot/DepthCharge/AimAltRange/2
    float pilot_depth_charge_maneuver_alt_range_1; // +49c Pilot/DepthCharge/ManeuverAltRange/1
    float pilot_depth_charge_maneuver_alt_range_2; // +4a0 Pilot/DepthCharge/ManeuverAltRange/2
    float pilot_depth_charge_cruising_alt; // +4a4 Pilot/DepthCharge/CruisingAlt
    float pilot_depth_charge_fly_above_dist; // +4a8 Pilot/DepthCharge/FlyAboveDist
    float pilot_depth_charge_attack_dist; // +4ac Pilot/DepthCharge/AttackDist
    float pilot_depth_charge_safe_dist; // +4b0 Pilot/DepthCharge/SafeDist
    std::int32_t pilot_depth_charge_move_on_cruising_alt; // +4b4 Pilot/DepthCharge/MoveOnCruisingAlt (00B66250 into a dword slot)
    float pilot_depth_charge_reference_speed; // +4b8 Pilot/DepthCharge/ReferenceSpeed
    float pilot_depth_charge_target_lost_time; // +4bc Pilot/DepthCharge/TargetLostTime default 20
    float pilot_dive_bomb_cruising_alt; // +4c0 Pilot/DiveBomb/CruisingAlt
    float pilot_dive_bomb_attack_dist; // +4c4 Pilot/DiveBomb/AttackDist
    float pilot_dive_bomb_safe_dist; // +4c8 Pilot/DiveBomb/SafeDist
    float pilot_dive_bomb_begin_alt_range_1; // +4cc Pilot/DiveBomb/BeginAltRange/1
    float pilot_dive_bomb_begin_alt_range_2; // +4d0 Pilot/DiveBomb/BeginAltRange/2
    std::int32_t pilot_dive_bomb_move_on_cruising_alt; // +4d4 Pilot/DiveBomb/MoveOnCruisingAlt (00B66250 into a dword slot)
    float pilot_dive_bomb_reference_speed; // +4d8 Pilot/DiveBomb/ReferenceSpeed
    float pilot_take_off_prepare_time; // +4dc Pilot/TakeOff/PrepareTime
    float pilot_landing_approach_pitch; // +4e0 Pilot/Landing/ApproachPitch
    float pilot_landing_approach_angle; // +4e4 Pilot/Landing/ApproachAngle
    float pilot_landing_park_velocity; // +4e8 Pilot/Landing/ParkVelocity
    float pilot_landing_approach_dist; // +4ec Pilot/Landing/ApproachDist
    float pilot_landing_pos_behind; // +4f0 Pilot/Landing/PosBehind
    float pilot_landing_pos_alt; // +4f4 Pilot/Landing/PosAlt
    float pilot_landing_circle_multiplier_min; // +4f8 Pilot/Landing/CircleMultiplierMin
    float pilot_landing_circle_multiplier_max; // +4fc Pilot/Landing/CircleMultiplierMax
    float pilot_landing_standby_dist; // +500 Pilot/Landing/StandbyDist
    float pilot_landing_follow_dist_time; // +504 Pilot/Landing/FollowDistTime
    float pilot_landing_takeoff_dist; // +508 Pilot/Landing/TakeoffDist
    float pilot_landing_touch_down_dist; // +50c Pilot/Landing/TouchDownDist
    float pilot_landing_lift_delay; // +510 Pilot/Landing/LiftDelay
    float pilot_landing_cruising_alt; // +514 Pilot/Landing/CruisingAlt
    float pilot_landing_wire_rope; // +518 Pilot/Landing/WireRope
    float pilot_landing_max_wire_rope; // +51c Pilot/Landing/MaxWireRope
    float pilot_landing_min_free_runway_length; // +520 Pilot/Landing/MinFreeRunwayLength
    float pilot_landing_radius_change_1; // +524 Pilot/Landing/RadiusChange/1
    float pilot_landing_radius_change_2; // +528 Pilot/Landing/RadiusChange/2
    float pilot_landing_reference_speed; // +52c Pilot/Landing/ReferenceSpeed
    float unit_ai_stop_clear_speed; // +530 UnitAI/StopClearSpeed
    float unit_ai_stop_clear_alt; // +534 UnitAI/StopClearAlt
    float pilot_general_waggle_limit; // +538 Pilot/General/WaggleLimit
    float pilot_general_cruising_alt; // +53c Pilot/General/CruisingAlt
    float pilot_general_max_alt_offset; // +540 Pilot/General/MaxAltOffset
    float pilot_general_climb_dist; // +544 Pilot/General/ClimbDist
    float pilot_general_drop_dist; // +548 Pilot/General/DropDist
    float pilot_general_min_turn_circle; // +54c Pilot/General/MinTurnCircle
    float pilot_general_level_bomb_angle_max; // +550 Pilot/General/LevelBombAngleMax [also stored here by 3 other read(s)]
    float pilot_general_dive_bomb_roll_angle_min; // +554 Pilot/General/DiveBombRollAngleMin
    float pilot_general_dive_bomb_roll_angle_max; // +558 Pilot/General/DiveBombRollAngleMax
    float pilot_general_dive_bomb_roll_angle_min_pitch; // +55c Pilot/General/DiveBombRollAngleMinPitch
    float pilot_general_dive_bomb_roll_angle_max_pitch; // +560 Pilot/General/DiveBombRollAngleMaxPitch
    float pilot_general_dive_bomb_pitch_angle_min; // +564 Pilot/General/DiveBombPitchAngleMin
    float pilot_general_dive_bomb_pitch_angle_max; // +568 Pilot/General/DiveBombPitchAngleMax
    float pilot_general_soft_hdg_mul; // +56c Pilot/General/SoftHdgMul
    float pilot_general_soft_hdg_limit; // +570 Pilot/General/SoftHdgLimit
    float pilot_general_soft_hdg_zone; // +574 Pilot/General/SoftHdgZone default 0.01
    float pilot_general_soft_roll_ctrl; // +578 Pilot/General/SoftRollCtrl
    float pilot_general_soft_roll_mul; // +57c Pilot/General/SoftRollMul
    float derived_580; // +580 derived, see the doc
    float pilot_general_hdg_diff_calc_limit_1; // +584 Pilot/General/HdgDiffCalcLimit/1
    float pilot_general_hdg_diff_calc_limit_2; // +588 Pilot/General/HdgDiffCalcLimit/2
    float pilot_general_hdg_diff_calc_min_pitch; // +58c Pilot/General/HdgDiffCalcMinPitch
    float pilot_general_hdg_diff_calc_min_roll; // +590 Pilot/General/HdgDiffCalcMinRoll
    float pilot_general_guard_dist; // +594 Pilot/General/GuardDist
    float pilot_general_leave_alone_pwr; // +598 Pilot/General/LeaveAlonePwr
    float pilot_general_turn_roll_limit_small; // +59c Pilot/General/TurnRollLimitSmall
    float pilot_general_turn_roll_limit_large; // +5a0 Pilot/General/TurnRollLimitLarge
    float pilot_general_turn_roll_pitch_limit_pitch_1; // +5a4 Pilot/General/TurnRollPitchLimitPitch/1
    float pilot_general_turn_roll_pitch_limit_pitch_2; // +5a8 Pilot/General/TurnRollPitchLimitPitch/2
    float pilot_general_turn_roll_pitch_limit_roll_1; // +5ac Pilot/General/TurnRollPitchLimitRoll/1
    float pilot_general_turn_roll_pitch_limit_roll_2; // +5b0 Pilot/General/TurnRollPitchLimitRoll/2
    float pilot_general_yaw_turn_roll_range_1; // +5b4 Pilot/General/YawTurnRollRange/1
    float pilot_general_yaw_turn_roll_range_2; // +5b8 Pilot/General/YawTurnRollRange/2
    float pilot_general_yaw_turn_max_pitch; // +5bc Pilot/General/YawTurnMaxPitch
    float pilot_general_pitch_turn_max_pitch; // +5c0 Pilot/General/PitchTurnMaxPitch
    float pilot_general_pitch_turn_hdg_range_1; // +5c4 Pilot/General/PitchTurnHdgRange/1
    float pilot_general_pitch_turn_hdg_range_2; // +5c8 Pilot/General/PitchTurnHdgRange/2
    float pilot_general_wingmen_wait_dist_1; // +5cc Pilot/General/WingmenWaitDist/1
    float pilot_general_wingmen_wait_dist_2; // +5d0 Pilot/General/WingmenWaitDist/2
    float pilot_general_yaw_ctrl_set_time_mul; // +5d4 Pilot/General/YawCtrlSetTimeMul
    float pilot_general_pitch_ctrl_set_time_mul; // +5d8 Pilot/General/PitchCtrlSetTimeMul
    float pilot_general_move_circle_min_angle; // +5dc Pilot/General/MoveCircleMinAngle
    float pilot_general_move_circle_max_angle; // +5e0 Pilot/General/MoveCircleMaxAngle
    float pilot_general_move_circle_followed_dist; // +5e4 Pilot/General/MoveCircleFollowedDist
    float pilot_general_trg_speed_corr_min_pitch; // +5e8 Pilot/General/TrgSpeedCorrMinPitch
    float pilot_general_trg_speed_corr_speed_mul; // +5ec Pilot/General/TrgSpeedCorrSpeedMul
    float pilot_general_trg_speed_corr_mul_decay; // +5f0 Pilot/General/TrgSpeedCorrMulDecay
    float pilot_general_drop_all_equipment_percent; // +5f4 Pilot/General/DropAllEquipmentPercent
    float pilot_avoidance_gunfire_max_weight; // +5f8 Pilot/Avoidance/Gunfire/MaxWeight
    float pilot_avoidance_gunfire_weight_inc; // +5fc Pilot/Avoidance/Gunfire/WeightInc
    float pilot_avoidance_gunfire_weight_dec; // +600 Pilot/Avoidance/Gunfire/WeightDec
    float pilot_avoidance_gunfire_avoid_time_1; // +604 Pilot/Avoidance/Gunfire/AvoidTime/1
    float pilot_avoidance_gunfire_avoid_time_2; // +608 Pilot/Avoidance/Gunfire/AvoidTime/2
    float pilot_avoidance_gunfire_wait_time_1; // +60c Pilot/Avoidance/Gunfire/WaitTime/1
    float pilot_avoidance_gunfire_wait_time_2; // +610 Pilot/Avoidance/Gunfire/WaitTime/2
    float pilot_avoidance_gunfire_bomber_vs_gunfire; // +614 Pilot/Avoidance/Gunfire/BomberVSGunfire
    float pilot_avoidance_vehicle_max_weight; // +618 Pilot/Avoidance/Vehicle/MaxWeight
    float pilot_avoidance_vehicle_weight_inc; // +61c Pilot/Avoidance/Vehicle/WeightInc
    float pilot_avoidance_vehicle_weight_dec; // +620 Pilot/Avoidance/Vehicle/WeightDec
    float pilot_avoidance_vehicle_min_coll_time; // +624 Pilot/Avoidance/Vehicle/MinCollTime
    float pilot_avoidance_vehicle_avoid_spd_mul; // +628 Pilot/Avoidance/Vehicle/AvoidSpdMul
    float pilot_avoidance_vehicle_min_plane_spd; // +62c Pilot/Avoidance/Vehicle/MinPlaneSpd
    float pilot_avoidance_vehicle_use_roll_strength; // +630 Pilot/Avoidance/Vehicle/UseRollStrength
    float pilot_avoidance_vehicle_max_dist_multiplier; // +634 Pilot/Avoidance/Vehicle/MaxDistMultiplier
    float pilot_avoidance_vehicle_min_dist_multiplier; // +638 Pilot/Avoidance/Vehicle/MinDistMultiplier
    std::int32_t pilot_avoidance_vehicle_bomber_vs_small_plane; // +63c Pilot/Avoidance/Vehicle/BomberVSSmallPlane (00B66250 into a dword slot)
    float pilot_dogfight_cruising_alt; // +640 Pilot/Dogfight/CruisingAlt
    float pilot_dogfight_attack_dist; // +644 Pilot/Dogfight/AttackDist
    float pilot_dogfight_fighter_aim_mul_versus_ai; // +648 Pilot/Dogfight/FighterAimMulVersusAI default 1.8
    float pilot_dogfight_fighter_aim_mul_versus_player; // +64c Pilot/Dogfight/FighterAimMulVersusPlayer default 1.2
    float pilot_dogfight_reference_speed; // +650 Pilot/Dogfight/ReferenceSpeed
    float pilot_strafe_cruising_alt; // +654 Pilot/Strafe/CruisingAlt
    float pilot_strafe_attack_dist; // +658 Pilot/Strafe/AttackDist
    float pilot_strafe_reference_speed; // +65c Pilot/Strafe/ReferenceSpeed
    float pilot_strike_cruising_alt; // +660 Pilot/Strike/CruisingAlt
    float pilot_strike_attack_dist; // +664 Pilot/Strike/AttackDist
    float pilot_strike_reference_speed; // +668 Pilot/Strike/ReferenceSpeed
    float pilot_auto_strafe_angle_angle_prepare; // +66c Pilot/AutoStrafeAngle/Angle_Prepare
    float pilot_auto_strafe_angle_angle_move_to; // +670 Pilot/AutoStrafeAngle/Angle_MoveTo
    float pilot_auto_strafe_angle_angle_go_away; // +674 Pilot/AutoStrafeAngle/Angle_GoAway
    float pilot_auto_strafe_angle_angle_strafe; // +678 Pilot/AutoStrafeAngle/Angle_Strafe
    float plane_gui_pitch_yaw_zoom_control_limit; // +67c PlaneGUI/PitchYawZoomControlLimit
    float plane_gui_roll_zoom_control_limit; // +680 PlaneGUI/RollZoomControlLimit
    float plane_gui_altimeter_ceiling; // +684 PlaneGUI/AltimeterCeiling
    float plane_gui_mouse_input_multiplier; // +688 PlaneGUI/MouseInputMultiplier
    float plane_gui_mouse_input_dead_zone_min; // +68c PlaneGUI/MouseInputDeadZoneMin
    float plane_gui_mouse_input_dead_zone_max; // +690 PlaneGUI/MouseInputDeadZoneMax
    float plane_gui_mouse_input_smooth_treshold; // +694 PlaneGUI/MouseInputSmoothTreshold
    float plane_gui_mouse_input_smooth_multiplier; // +698 PlaneGUI/MouseInputSmoothMultiplier
    float plane_gui_mouse_input_exponencial_weight; // +69c PlaneGUI/MouseInputExponencialWeight
    float plane_gui_speed_display_multiplier; // +6a0 PlaneGUI/SpeedDisplayMultiplier
    float sound_engine_power_multiplier; // +6a4 Sound/EnginePowerMultiplier
    float sound_engine_speed_multiplier; // +6a8 Sound/EngineSpeedMultiplier
    float sound_idle_off_volume; // +6ac Sound/IdleOffVolume
    float sound_engine_min_freq; // +6b0 Sound/EngineMinFreq
    float sound_engine_max_freq; // +6b4 Sound/EngineMaxFreq
    float sound_pitch_change_rate; // +6b8 Sound/PitchChangeRate
    float sound_volume_change_rate; // +6bc Sound/VolumeChangeRate
    float sound_idle_safety_time; // +6c0 Sound/IdleSafetyTime
    float sound_engine_freq_limit_spd_mul; // +6c4 Sound/EngineFreqLimitSpdMul
    float sound_engine_freq_limit_max; // +6c8 Sound/EngineFreqLimitMax
    float sound_idle_and_engine_vol_max; // +6cc Sound/IdleAndEngineVolMax
};

static_assert(sizeof(GameTuningBlock) == kGameTuningBlockSize,
              "the tuning block is 6D0h bytes (operator new(0x6d0) at 0042E7A2)");
static_assert(offsetof(GameTuningBlock, vtable) == 0x0, "vtable");
static_assert(offsetof(GameTuningBlock, close_to_camera_dist) == 0x4, "close_to_camera_dist");
static_assert(offsetof(GameTuningBlock, move_detail_lod_error) == 0x8, "move_detail_lod_error");
static_assert(offsetof(GameTuningBlock, move_detail_radius) == 0xc, "move_detail_radius");
static_assert(offsetof(GameTuningBlock, death_mode_chances_explosion) == 0x10, "death_mode_chances_explosion");
static_assert(offsetof(GameTuningBlock, death_mode_chances_explosion_delayed) == 0x14, "death_mode_chances_explosion_delayed");
static_assert(offsetof(GameTuningBlock, death_mode_chances_spinning) == 0x18, "death_mode_chances_spinning");
static_assert(offsetof(GameTuningBlock, death_mode_chances_powerloss) == 0x1c, "death_mode_chances_powerloss");
static_assert(offsetof(GameTuningBlock, sound_idle_safety_time) == 0x6c0, "sound_idle_safety_time");
static_assert(offsetof(GameTuningBlock, sound_engine_freq_limit_spd_mul) == 0x6c4, "sound_engine_freq_limit_spd_mul");
static_assert(offsetof(GameTuningBlock, sound_engine_freq_limit_max) == 0x6c8, "sound_engine_freq_limit_max");
static_assert(offsetof(GameTuningBlock, sound_idle_and_engine_vol_max) == 0x6cc, "sound_idle_and_engine_vol_max");

// Offsets the doc calls out. The three AccelCheat offsets are the ones 007D1F70
// touches and the only ones any caller of 0042E740 writes.
inline constexpr std::size_t kGameTuningAccelCheatMul = 0x31C;
inline constexpr std::size_t kGameTuningAccelCheatMulMul = 0x320;
inline constexpr std::size_t kGameTuningNewTravelSpeedMul = 0x334;

// A read-only view of one Lua value handed back by the host. It carries no
// ownership and no stack index; the native code holds a LuaObject instead.
struct GameTuningLuaValue {
    bool nil{true};
    double number{0.0};
    bool boolean{false};
};

// Integration boundary for the Lua calls 007E2A20 makes. One method per distinct
// native callee, in the order the loader reaches them. There are no default
// implementations: nothing here stands in for unrecovered game behaviour. Lua
// 5.1.1, the STL string and the CRT are contracts, not ported code.
struct GameTuningLuaHost {
    using Handle = std::uintptr_t;
    virtual ~GameTuningLuaHost() = default;
    // 00B69D40 at 007E2AE9 and 007E2B5D, ECX = the private state owner.
    virtual void run_script(const char* path) = 0;
    // 00B67980 at 007E2B98; the globals table of the private state.
    virtual Handle globals() = 0;
    // 00B67800, 471 sites. ECX = parent, returns the fetched value.
    virtual Handle table_field(Handle parent, const char* key) = 0;
    // 00B67720, 50 sites. One-based index, as the native code pushes it.
    virtual Handle table_element(Handle parent, int one_based_index) = 0;
    // 00B66270 / 00B66290 / 00B66250 / 00B66330, and 00B67A80 for the triple.
    virtual GameTuningLuaValue value(Handle handle) = 0;
    virtual void number_triple(Handle handle, float out[3]) = 0;
    // 00B67700, 524 sites.
    virtual void release(Handle handle) = 0;
};

// The two script paths, in the order 007E2A20 runs them. The first defines the
// DEG and KMH helpers and the game's constants and contributes no key; every key
// in kGameTuningKeys comes from the global table the second defines.
inline constexpr const char* kGameTuningConstantsScript = "Scripts\\global\\luaMW_init.lua";
inline constexpr const char* kGameTuningDataScript = "Scripts\\datatables\\PlaneGlobals.lua";
inline constexpr const char* kGameTuningRootTable = "PlaneGlobals";

// 007E2A20. Runs both scripts through the host, then walks kGameTuningKeys in
// load order and writes each value into `block`. The block is NOT zeroed first:
// operator new(0x6d0) at 0042E7A2 hands back raw storage and the native loader
// writes only the vtable and the keys, so this matches the native behaviour and
// a caller that wants defined padding must clear the block itself.
void game_tuning_load_007e2a20(GameTuningLuaHost& host, GameTuningBlock& block);

// The four derived stores, as pure rules. Addresses are the store sites.
// 007E41DF: interpolate the control range, floor it with the scaled stall range,
// raise it to the scaled level-flight speed, then clamp it down to level flight.
float game_tuning_control_speed_007e41df(float control_range_min, float control_range_max,
                                         float stall_range_max, float level_flight,
                                         float lerp_k, float stall_k, float level_k) noexcept;
// 007E6FD4: (1 - a) * b over the two fields already at +57Ch and +578h.
float game_tuning_blend_007e6fd4(float a, float b) noexcept;
// 007E3BF0 and 007E3C45: the two Water control angles are stored as cosines.
float game_tuning_ctrl_angle_cos(float radians) noexcept;

// 007D20F3..007D2144 in BSP_PlaneClass_ReadLuaFields. Scales the row's Accel when
// AccelCheatMul is above 1.0f and otherwise clamps AccelCheatMul to exactly 1.0f.
// The clamp is the only write into the singleton from any of the 141 call sites of
// 0042E740; it is idempotent, and with the installed AccelCheatMul = 1.5 it is
// never reached. Returns the Accel the descriptor's +164h receives.
float game_tuning_apply_accel_cheat_007d20f3(GameTuningBlock& block, float accel) noexcept;

} // namespace bsp
