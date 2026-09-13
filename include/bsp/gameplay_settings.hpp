#pragma once
// The gameplay tuning settings singleton behind 00424C40, its Lua-driven loader 0083B5E0
// and the key table that fills it. docs/GAMEPLAY_SETTINGS.md.
//
// The native object is 76Ch bytes, allocated at 00424C9F and constructed by 00424A10, whose
// tail calls the loader 0083B5E0 with ECX still holding the object. The loader opens its own
// Lua state, runs two scripts and reads the global table "ShipGlobals". Every field below is
// one store in that loader's listing; the comment gives the key path under ShipGlobals.
//
// This is a documented projection, not a binary-compatible replacement: the offsets are the
// native ones and are asserted, but the gaps hold sub-objects and containers this packet did
// not read, so the struct is only safe to use on its own instances.
//
// Names are hypotheses from the shipped Lua keys, not recovered symbols.

#include <cstddef>
#include <cstdint>

namespace bsp {

// The two scripts the loader runs before it reads the globals, in order
// (0083B63A and 0083B6AE build the paths from lengths 1Dh and 22h).
inline constexpr int kGameplayTuningScriptNameLength0 = 0x1D; // 0083B62C
inline constexpr int kGameplayTuningScriptNameLength1 = 0x22; // 0083B6A0

// 00424C9F: PUSH 76Ch into operator new.
inline constexpr std::size_t kGameplayTuningSettingsSize = 0x76C;

// 00424C55: the singleton pointer.
inline constexpr std::uint32_t kGameplayTuningSettingsSingletonAddress = 0x00F8753C;

// 00424A34: the constructor's vtable store.
inline constexpr std::uint32_t kGameplayTuningSettingsVTableAddress = 0x00CE3994;

// Ship-class slots of the two AvoidZoneDepths records at +80h and +F0h, from the stores at
// 00841BEE..008425A1. Each record is 70h bytes of int; two entries per class except the two
// submarine classes, which have five (surface plus four dive depths).
enum class AvoidZoneDepthSlot : int {
    kMotherShip = 0x00, kDestroyer = 0x08, kTBoat = 0x10, kSmallLandingShip = 0x18,
    kLargeLandingShip = 0x20, kBattleShip = 0x28, kCargoShip = 0x30, kLightCruiser = 0x38,
    kHeavyCruiser = 0x40, kMiniSub = 0x48, kSubmarine = 0x5C,
};
inline constexpr std::size_t kAvoidZoneDepthsRecordSize = 0x70;
inline constexpr std::size_t kAvoidZoneDepthsSingleOffset = 0x80;  // 00841B81
inline constexpr std::size_t kAvoidZoneDepthsMultiOffset = 0xF0;   // 00841B8E

struct GameplayTuningSettings {
    std::byte gap_000h[0x8];
    float          sinking_drag_multiplier;                       // +008h  Sinking.DragMultiplier
    float          sinking_drag_multiplier_z;                     // +00Ch  Sinking.DragMultiplierZ
    float          sinking_leak_size;                             // +010h  Sinking.LeakSize
    float          sinking_drag_power;                            // +014h  Sinking.DragPower
    float          sinking_start_angular_velocity;                // +018h  Sinking.StartAngularVelocity
    std::int32_t   sinking_section_num;                           // +01Ch  Sinking.SectionNum
    float          debris_splash_speed_1;                         // +020h  Debris.SplashSpeed.[1]
    float          debris_splash_speed_2;                         // +024h  Debris.SplashSpeed.[2]
    float          debris_splash_fxid_1;                          // +028h  Debris.SplashFXID.[1]
    float          debris_splash_fxid_2;                          // +02Ch  Debris.SplashFXID.[2]
    float          player_artillery_throw_after_shot_fire_time;   // +030h  PlayerArtilleryThrow.AfterShot_FireTime
    float          player_artillery_throw_after_shot_wait_time;   // +034h  PlayerArtilleryThrow.AfterShot_WaitTime
    // Legacy member names retain Lua duration keys; these three fields store
    // reciprocal rates after the native minimum-duration clamp, not seconds.
    float          player_artillery_throw_throw_increment_time_has_target; // +038h  rate from PlayerArtilleryThrow.ThrowIncrementTime_HasTarget
    float          player_artillery_throw_throw_increment_time_no_target; // +03Ch  rate from PlayerArtilleryThrow.ThrowIncrementTime_NoTarget
    float          player_artillery_throw_throw_decrement_time;   // +040h  rate from PlayerArtilleryThrow.ThrowDecrementTime
    bool           pipe_sight_params_pipesight_enabled;           // +044h  PipeSightParams.pipesight_enabled
    std::byte gap_045h[0x3];
    float          pipe_sight_params_blur_heavy_add;              // +048h  PipeSightParams.blur_heavy_add
    float          pipe_sight_params_blur_medium_add;             // +04Ch  PipeSightParams.blur_medium_add
    float          pipe_sight_params_blur_light_add;              // +050h  PipeSightParams.blur_light_add
    float          pipe_sight_params_blur_aa_add;                 // +054h  PipeSightParams.blur_aa_add
    float          pipe_sight_params_blur_spring;                 // +058h  PipeSightParams.blur_spring
    float          pipe_sight_params_blur_drag;                   // +05Ch  PipeSightParams.blur_drag
    float          pipe_sight_params_blur_rate;                   // +060h  PipeSightParams.blur_rate
    float          pipe_sight_params_zoom_heavy_add;              // +064h  PipeSightParams.zoom_heavy_add
    float          pipe_sight_params_zoom_medium_add;             // +068h  PipeSightParams.zoom_medium_add
    float          pipe_sight_params_zoom_light_add;              // +06Ch  PipeSightParams.zoom_light_add
    float          pipe_sight_params_zoom_aa_add;                 // +070h  PipeSightParams.zoom_aa_add
    float          pipe_sight_params_zoom_spring;                 // +074h  PipeSightParams.zoom_spring
    float          pipe_sight_params_zoom_drag;                   // +078h  PipeSightParams.zoom_drag
    float          pipe_sight_params_zoom_rate;                   // +07Ch  PipeSightParams.zoom_rate
    std::byte gap_080h[0xE0];
    float          attack_move_director_my_damage_weight;         // +160h  AttackMoveDirector.MyDamageWeight
    float          attack_move_director_ideal_dist_weight;        // +164h  AttackMoveDirector.IdealDistWeight
    float          attack_move_director_nearby_enemy_weight;      // +168h  AttackMoveDirector.NearbyEnemyWeight
    float          attack_move_director_nearby_enemy_reference;   // +16Ch  AttackMoveDirector.NearbyEnemyReference
    float          attack_move_director_nearest_move_dir_weight;  // +170h  AttackMoveDirector.NearestMoveDirWeight
    float          attack_move_director_prev_move_dir_weight;     // +174h  AttackMoveDirector.PrevMoveDirWeight
    float          attack_move_director_prev_move_dir_range;      // +178h  AttackMoveDirector.PrevMoveDirRange
    std::byte gap_17Ch[0x14];
    float          ship_avoidance_collect_timer_1;                // +190h  ShipAvoidance.CollectTimer.[1]
    float          ship_avoidance_collect_timer_2;                // +194h  ShipAvoidance.CollectTimer.[2]
    float          ship_avoidance_collect_dist;                   // +198h  ShipAvoidance.CollectDist
    float          ship_avoidance_collect_hit_time;               // +19Ch  ShipAvoidance.CollectHitTime
    float          ship_avoidance_nearby_ship_arrive_time_min;    // +1A0h  ShipAvoidance.NearbyShip_ArriveTimeMin
    float          ship_avoidance_nearby_ship_arrive_dist_min;    // +1A4h  ShipAvoidance.NearbyShip_ArriveDistMin
    float          ship_avoidance_nearby_ship_pos_speed_corrig;   // +1A8h  ShipAvoidance.NearbyShip_PosSpeedCorrig
    float          ship_avoidance_nearby_ship_est_pos_dist_limit_mul; // +1ACh  ShipAvoidance.NearbyShip_EstPos_DistLimitMul
    float          ship_avoidance_nearby_ship_est_pos_min_ship_length; // +1B0h  ShipAvoidance.NearbyShip_EstPos_MinShipLength
    float          ship_avoidance_nearby_ship_est_pos_ship_length_limit_mul; // +1B4h  ShipAvoidance.NearbyShip_EstPos_ShipLengthLimitMul
    float          ship_avoidance_nearby_ship_my_min_spd_ratio;   // +1B8h  ShipAvoidance.NearbyShip_MyMinSpdRatio
    float          ship_avoidance_nearby_ship_est_pos_ship_spd_mul; // +1BCh  ShipAvoidance.NearbyShip_EstPos_ShipSpdMul
    float          ship_avoidance_nearby_ship_est_pos_size_dec_mul; // +1C0h  ShipAvoidance.NearbyShip_EstPos_SizeDecMul
    float          ship_avoidance_nearby_ship_est_pos_size_dec_min; // +1C4h  ShipAvoidance.NearbyShip_EstPos_SizeDecMin
    float          ship_avoidance_nearby_ship_next_corner_reach_dist_add_on; // +1C8h  ShipAvoidance.NearbyShip_NextCornerReachDistAddOn
    float          ship_avoidance_nearby_ship_move_path_line_check_threshold; // +1CCh  ShipAvoidance.NearbyShip_MovePathLineCheckThreshold
    float          ship_avoidance_nearby_ship_go_away_spd_add;    // +1D0h  ShipAvoidance.NearbyShip_GoAwaySpdAdd
    float          ship_avoidance_nearby_ship_way_clear_check_time; // +1D4h  ShipAvoidance.NearbyShip_WayClearCheckTime
    float          ship_avoidance_hit_detector_last_hit_dist_add_on; // +1D8h  ShipAvoidance.HitDetector_LastHitDistAddOn
    std::byte gap_1DCh[0x10];
    float          torpedo_avoidance_collect_timer_1;             // +1ECh  TorpedoAvoidance.CollectTimer.[1]
    float          torpedo_avoidance_collect_timer_2;             // +1F0h  TorpedoAvoidance.CollectTimer.[2]
    float          land_avoidance_check_move_pos_zone_time_1;     // +1F4h  LandAvoidance.CheckMovePosZoneTime.[1]
    float          land_avoidance_check_move_pos_zone_time_2;     // +1F8h  LandAvoidance.CheckMovePosZoneTime.[2]
    float          land_avoidance_check_ship_pos_zone_time_1;     // +1FCh  LandAvoidance.CheckShipPosZoneTime.[1]
    float          land_avoidance_check_ship_pos_zone_time_2;     // +200h  LandAvoidance.CheckShipPosZoneTime.[2]
    float          land_avoidance_check_travel_zone_time_1;       // +204h  LandAvoidance.CheckTravelZoneTime.[1]
    float          land_avoidance_check_travel_zone_time_2;       // +208h  LandAvoidance.CheckTravelZoneTime.[2]
    float          land_avoidance_collect_timer_1;                // +20Ch  LandAvoidance.CollectTimer.[1]
    float          land_avoidance_collect_timer_2;                // +210h  LandAvoidance.CollectTimer.[2]
    float          land_avoidance_yturn_dir_diff_1;               // +214h  LandAvoidance.YTurnDirDiff.[1]
    float          land_avoidance_yturn_dir_diff_2;               // +218h  LandAvoidance.YTurnDirDiff.[2]
    float          land_avoidance_way_checker_update_time;        // +21Ch  LandAvoidance.WayCheckerUpdateTime
    float          avoidance_cheat_turn_spd_limit;                // +220h  AvoidanceCheat.TurnSpdLimit
    float          avoidance_cheat_turn_spd_mul;                  // +224h  AvoidanceCheat.TurnSpdMul
    float          avoidance_cheat_stop_spd_mul;                  // +228h  AvoidanceCheat.StopSpdMul
    float          retreat_exit_dist;                             // +22Ch  Retreat.ExitDist
    float          retreat_exit_time;                             // +230h  Retreat.ExitTime
    float          retreat_warning_repeat_time;                   // +234h  Retreat.WarningRepeatTime
    float          hack_rotation_add;                             // +238h  Hack.RotationAdd
    float          hack_height_add;                               // +23Ch  Hack.HeightAdd
    std::byte gap_240h[0x160];
    float          colli_dolgok_colli_damage_multiplier;          // +3A0h  ColliDolgok.ColliDamageMultiplier
    std::byte gap_3A4h[0x8];
    float          fire_tick_damage;                              // +3ACh  FireTickDamage
    float          water_tick_damage;                             // +3B0h  WaterTickDamage
    float          body_repair_tick_percentage;                   // +3B4h  BodyRepairTickPercentage
    float          gun_repair_tick_percentage;                    // +3B8h  GunRepairTickPercentage
    float          fire_failure_chance;                           // +3BCh  FireFailureChance
    float          fire_failure_damage_duration;                  // +3C0h  FireFailureDamageDuration
    float          explosion_damage_percentage;                   // +3C4h  ExplosionDamagePercentage
    float          pump_repair_multiplier;                        // +3C8h  PumpRepairMultiplier
    float          fire_repair_multiplier;                        // +3CCh  FireRepairMultiplier
    float          failure_repair_multiplier;                     // +3D0h  FailureRepairMultiplier
    float          body_repair_multiplier;                        // +3D4h  BodyRepairMultiplier
    float          gun_repair_multiplier;                         // +3D8h  GunRepairMultiplier
    float          failure_chance;                                // +3DCh  FailureChance
    float          failure_damage_threshold;                      // +3E0h  FailureDamageThreshold
    std::byte gap_3E4h[0x10];
    float          vizbeomles_dolgok_kill_depth;                  // +3F4h  VizbeomlesDolgok.KillDepth
    float          vizbeomles_dolgok_ptboat_leak_size;            // +3F8h  VizbeomlesDolgok.PTBoatLeakSize
    float          vizbeomles_dolgok_leak_per_hp;                 // +3FCh  VizbeomlesDolgok.LeakPerHP
    float          vizbeomles_dolgok_max_leak_percent;            // +400h  VizbeomlesDolgok.MaxLeakPercent
    float          vizbeomles_dolgok_ennyi_viz_es_kesz_percent;   // +404h  VizbeomlesDolgok.EnnyiVizEsKeszPercent
    float          vizbeomles_dolgok_water_section_num;           // +408h  VizbeomlesDolgok.WaterSectionNum
    float          vizbeomles_dolgok_dolog_szorzo;                // +40Ch  VizbeomlesDolgok.DologSzorzo
    float          formacio_heading_mul;                          // +410h  Formacio.HeadingMul
    float          formacio_heading_power;                        // +414h  Formacio.HeadingPower
    float          formacio_thrust_mul;                           // +418h  Formacio.ThrustMul
    float          formacio_follower_speed_mul;                   // +41Ch  Formacio.FollowerSpeedMul
    float          formacio_formation_max_count;                  // +420h  Formacio.FormationMaxCount
    float          formacio_follower_max_dist;                    // +424h  Formacio.FollowerMaxDist
    std::byte gap_428h[0x4];
    float          formacio_formation_ship_dist;                  // +42Ch  Formacio.FormationShipDist
    float          formacio_update_interval;                      // +430h  Formacio.UpdateInterval
    float          formacio_player_follower_max_dist;             // +434h  Formacio.PlayerFollowerMaxDist
    float          navigator_turn_multipliers_turn_multiplier_max_speed_2; // +438h  Navigator.TurnMultipliers.TurnMultiplierMaxSpeed.[2]
    float          navigator_turn_multipliers_turn_multiplier_max_speed_1; // +43Ch  Navigator.TurnMultipliers.TurnMultiplierMaxSpeed.[1]
    float          navigator_turn_multipliers_turn_multiplier_min_speed_2; // +440h  Navigator.TurnMultipliers.TurnMultiplierMinSpeed.[2]
    float          navigator_turn_multipliers_turn_multiplier_min_speed_1; // +444h  Navigator.TurnMultipliers.TurnMultiplierMinSpeed.[1]
    float          navigator_turn_multipliers_turn_multiplier_med_speed_2; // +448h  Navigator.TurnMultipliers.TurnMultiplierMedSpeed.[2]
    float          navigator_turn_multipliers_turn_multiplier_med_speed_1; // +44Ch  Navigator.TurnMultipliers.TurnMultiplierMedSpeed.[1]
    float          ship_camera_zoom_offset;                       // +450h  ShipCamera.ZoomOffset
    float          ship_camera_length_mult;                       // +454h  ShipCamera.LengthMult
    float          ship_camera_min_camera_angle;                  // +458h  ShipCamera.MinCameraAngle
    float          ship_camera_max_camera_angle;                  // +45Ch  ShipCamera.MaxCameraAngle
    float          ship_camera_follow_cam_delay;                  // +460h  ShipCamera.FollowCamDelay
    float          repair_repair_script_interval;                 // +464h  Repair.RepairScriptInterval
    float          repair_repair_step_value;                      // +468h  Repair.RepairStepValue
    float          repair_repair_team_count;                      // +46Ch  Repair.RepairTeamCount
    float          repair_repair_team_bonus;                      // +470h  Repair.RepairTeamBonus
    float          repair_repair_team_penalty;                    // +474h  Repair.RepairTeamPenalty
    float          repair_repair_underwater_modifier;             // +478h  Repair.RepairUnderwaterModifier
    std::byte gap_47Ch[0x18];
    float          repair_repair_zone_multiplier;                 // +494h  Repair.RepairZoneMultiplier
    float          repair_torpedo_restock_time;                   // +498h  Repair.TorpedoRestockTime
    float          submarine_submarine_periscope_level;           // +49Ch  Submarine.SubmarinePeriscopeLevel
    float          submarine_submarine_medium_level;              // +4A0h  Submarine.SubmarineMediumLevel
    float          submarine_submarine_deep_level;                // +4A4h  Submarine.SubmarineDeepLevel
    float          submarine_submarine_torpedo_range;             // +4A8h  Submarine.SubmarineTorpedoRange
    float          submarine_submarine_depth_damage;              // +4ACh  Submarine.SubmarineDepthDamage
    float          submarine_submarine_damage_depth;              // +4B0h  Submarine.SubmarineDamageDepth
    float          submarine_submarine_depth_speed_mul;           // +4B4h  Submarine.SubmarineDepthSpeedMul
    float          submarine_submarine_air_warning_limit;         // +4B8h  Submarine.SubmarineAirWarningLimit
    float          submarine_submarine_air_need_limit;            // +4BCh  Submarine.SubmarineAirNeedLimit
    float          submarine_submarine_air_enough_limit;          // +4C0h  Submarine.SubmarineAirEnoughLimit
    float          submarine_periscope_repair_time;               // +4C4h  Submarine.PeriscopeRepairTime
    float          sub_attack_max_torpedo_range;                  // +4C8h  SubAttack.MaxTorpedoRange
    float          sub_attack_too_close_dist;                     // +4CCh  SubAttack.TooCloseDist
    float          sub_attack_far_enough_dist;                    // +4D0h  SubAttack.FarEnoughDist
    float          sub_attack_submarine_lost_time;                // +4D4h  SubAttack.SubmarineLostTime
    float          mother_ship_elevator_speed;                    // +4D8h  MotherShip.ElevatorSpeed
    float          mother_ship_elevator_depth;                    // +4DCh  MotherShip.ElevatorDepth
    std::byte gap_4E0h[0xA8];
    float          physics_tboat_nyomatek_szorzo;                 // +588h  Physics.TBoatNyomatekSzorzo
    float          physics_tboat_motor_max_szog;                  // +58Ch  Physics.TBoatMotorMaxSzog
    float          physics_torpedo_force;                         // +590h  Physics.TorpedoForce
    float          physics_torpedo_force_power;                   // +594h  Physics.TorpedoForcePower
    float          dofparams_dist;                                // +598h  DOFParams.Dist
    float          dofparams_range1;                              // +59Ch  DOFParams.Range1
    float          dofparams_range2;                              // +5A0h  DOFParams.Range2
    float          dofparams_min_amount;                          // +5A4h  DOFParams.MinAmount
    float          dofparams_max_amount;                          // +5A8h  DOFParams.MaxAmount
    float          torpedo_hit_life_time;                         // +5ACh  TorpedoHit.LifeTime
    float          torpedo_hit_upthrust;                          // +5B0h  TorpedoHit.Upthrust
    float          torpedo_hit_up_dist;                           // +5B4h  TorpedoHit.UpDist
    float          torpedo_hit_mass_ratio;                        // +5B8h  TorpedoHit.MassRatio
    std::byte gap_5BCh[0x90];
    float          sounds_ship_dead_meat_sound_time_min;          // +64Ch  Sounds.ShipDeadMeatSoundTimeMin
    float          sounds_ship_dead_meat_sound_time_max;          // +650h  Sounds.ShipDeadMeatSoundTimeMax
    float          sounds_plane_dead_meat_freq0;                  // +654h  Sounds.PlaneDeadMeatFreq0
    float          sounds_plane_dead_meat_freq1000;               // +658h  Sounds.PlaneDeadMeatFreq1000
    float          physics_wreck_chance;                          // +65Ch  Physics.WreckChance
    std::byte gap_660h[0x20];
    bool           debris_struct_debris_enabled;                  // +680h  DebrisStruct.DebrisEnabled
    std::byte gap_681h[0x3];
    float          debris_struct_debris_num_per_meter;            // +684h  DebrisStruct.DebrisNumPerMeter
    bool           flag_need_flag;                                // +688h  Flag.NeedFlag
    std::byte gap_689h[0x3];
    float          flag_wind_power_min;                           // +68Ch  Flag.WindPowerMin
    float          flag_wind_power_max;                           // +690h  Flag.WindPowerMax
    float          flag_wind_power_change;                        // +694h  Flag.WindPowerChange
    float          flag_wind_dir_change;                          // +698h  Flag.WindDirChange
    float          flag_wind_dir_min;                             // +69Ch  Flag.WindDirMin
    float          flag_wind_dir_max;                             // +6A0h  Flag.WindDirMax
    float          flag_gravity;                                  // +6A4h  Flag.Gravity
    float          flag_drag;                                     // +6A8h  Flag.Drag
    float          flag_dt_mul;                                   // +6ACh  Flag.DtMul
    float          flag_dt_step;                                  // +6B0h  Flag.DtStep
    std::int32_t   flag_num_iteration;                            // +6B4h  Flag.NumIteration
    std::byte gap_6B8h[0xC];
    float          navigator_auto_thrust_steer_value_min_slow;    // +6C4h  Navigator.AutoThrust.SteerValueMin_Slow
    float          navigator_auto_thrust_steer_value_max_slow;    // +6C8h  Navigator.AutoThrust.SteerValueMax_Slow
    float          navigator_auto_thrust_hdg_diff_value_min_slow; // +6CCh  Navigator.AutoThrust.HdgDiffValueMin_Slow
    float          navigator_auto_thrust_hdg_diff_value_max_slow; // +6D0h  Navigator.AutoThrust.HdgDiffValueMax_Slow
    float          navigator_auto_thrust_thrust_min_slow;         // +6D4h  Navigator.AutoThrust.ThrustMin_Slow
    float          navigator_auto_thrust_steer_value_min_fast;    // +6D8h  Navigator.AutoThrust.SteerValueMin_Fast
    float          navigator_auto_thrust_steer_value_max_fast;    // +6DCh  Navigator.AutoThrust.SteerValueMax_Fast
    float          navigator_auto_thrust_hdg_diff_value_min_fast; // +6E0h  Navigator.AutoThrust.HdgDiffValueMin_Fast
    float          navigator_auto_thrust_hdg_diff_value_max_fast; // +6E4h  Navigator.AutoThrust.HdgDiffValueMax_Fast
    float          navigator_auto_thrust_thrust_min_fast;         // +6E8h  Navigator.AutoThrust.ThrustMin_Fast
    float          navigator_auto_thrust_hdg_diff_danger_mul;     // +6ECh  Navigator.AutoThrust.HdgDiffDangerMul
    float          navigator_path_finder_params_length_modifier_dir_diff_min; // +6F0h  Navigator.PathFinderParams.LengthModifier_DirDiffMin
    float          navigator_path_finder_params_length_modifier_dir_diff_max; // +6F4h  Navigator.PathFinderParams.LengthModifier_DirDiffMax
    float          navigator_path_finder_params_length_modifier_length_addon; // +6F8h  Navigator.PathFinderParams.LengthModifier_LengthAddon
    float          navigator_island_attack_turn_circle_multiplier; // +6FCh  Navigator.IslandAttackTurnCircleMultiplier
    float          navigator_flock_predict_time;                  // +700h  Navigator.FlockPredictTime
    float          navigator_flock_separation_strength;           // +704h  Navigator.FlockSeparationStrength
    float          navigator_flock_follow_strength;               // +708h  Navigator.FlockFollowStrength
    float          navigator_flock_separation_min_dist_mult;      // +70Ch  Navigator.FlockSeparationMinDistMult
    float          navigator_flock_separation_max_dist_mult;      // +710h  Navigator.FlockSeparationMaxDistMult
    float          navigator_flock_move_treshold;                 // +714h  Navigator.FlockMoveTreshold
    float          navigator_flock_thrust_treshold;               // +718h  Navigator.FlockThrustTreshold
    float          navigator_flock_back_steer_speed;              // +71Ch  Navigator.FlockBackSteerSpeed
    float          navigator_flock_vector_multiplier;             // +720h  Navigator.FlockVectorMultiplier
    float          navigator_flock_leader_max_hdg;                // +724h  Navigator.FlockLeaderMaxHdg
    float          navigator_flock_leader_max_side_move;          // +728h  Navigator.FlockLeaderMaxSideMove
    std::byte gap_72Ch[0x8];
    float          vizbeomles_dolgok_sink_effect_ennyi_meterenkent; // +734h  VizbeomlesDolgok.SinkEffectEnnyiMeterenkent
    std::int32_t   vizbeomles_dolgok_sink_effect_ilyen_gyakran;   // +738h  VizbeomlesDolgok.SinkEffectIlyenGyakran
    std::byte gap_73Ch[0x10];
    float          fire_fire_damage_per_fire_tick;                // +74Ch  Fire.FireDamagePerFireTick
    float          aagunner_error_modifier_versus_ai;             // +750h  AAGunnerErrorModifier.VersusAI
    float          aagunner_error_modifier_versus_player;         // +754h  AAGunnerErrorModifier.VersusPlayer
    float          aagunner_error_modifier_calc_target_pos_time_add_fix; // +758h  AAGunnerErrorModifier.CalcTargetPosTimeAddFix
    float          aagunner_error_modifier_calc_target_pos_time_add_mul; // +75Ch  AAGunnerErrorModifier.CalcTargetPosTimeAddMul
    bool           aagunner_error_modifier_turn_off_aagun_throw;  // +760h  AAGunnerErrorModifier.TurnOffAAGunThrow
    std::byte gap_761h[0x3];
    float          sub_torpedo_delay;                             // +764h  SubTorpedoDelay
    float          ship_torpedo_delay;                            // +768h  ShipTorpedoDelay
};
static_assert(sizeof(GameplayTuningSettings) == kGameplayTuningSettingsSize,
              "the native object is 76Ch bytes");
static_assert(offsetof(GameplayTuningSettings, sinking_drag_multiplier) == 0x008, "+008h");
static_assert(offsetof(GameplayTuningSettings, sinking_drag_multiplier_z) == 0x00C, "+00Ch");
static_assert(offsetof(GameplayTuningSettings, sinking_leak_size) == 0x010, "+010h");
static_assert(offsetof(GameplayTuningSettings, sinking_drag_power) == 0x014, "+014h");
static_assert(offsetof(GameplayTuningSettings, sinking_start_angular_velocity) == 0x018, "+018h");
static_assert(offsetof(GameplayTuningSettings, sinking_section_num) == 0x01C, "+01Ch");
static_assert(offsetof(GameplayTuningSettings, debris_splash_speed_1) == 0x020, "+020h");
static_assert(offsetof(GameplayTuningSettings, debris_splash_speed_2) == 0x024, "+024h");
static_assert(offsetof(GameplayTuningSettings, debris_splash_fxid_1) == 0x028, "+028h");
static_assert(offsetof(GameplayTuningSettings, debris_splash_fxid_2) == 0x02C, "+02Ch");
static_assert(offsetof(GameplayTuningSettings, player_artillery_throw_after_shot_fire_time) == 0x030, "+030h");
static_assert(offsetof(GameplayTuningSettings, player_artillery_throw_after_shot_wait_time) == 0x034, "+034h");
static_assert(offsetof(GameplayTuningSettings, player_artillery_throw_throw_increment_time_has_target) == 0x038, "+038h");
static_assert(offsetof(GameplayTuningSettings, player_artillery_throw_throw_increment_time_no_target) == 0x03C, "+03Ch");
static_assert(offsetof(GameplayTuningSettings, player_artillery_throw_throw_decrement_time) == 0x040, "+040h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_pipesight_enabled) == 0x044, "+044h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_blur_heavy_add) == 0x048, "+048h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_blur_medium_add) == 0x04C, "+04Ch");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_blur_light_add) == 0x050, "+050h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_blur_aa_add) == 0x054, "+054h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_blur_spring) == 0x058, "+058h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_blur_drag) == 0x05C, "+05Ch");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_blur_rate) == 0x060, "+060h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_zoom_heavy_add) == 0x064, "+064h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_zoom_medium_add) == 0x068, "+068h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_zoom_light_add) == 0x06C, "+06Ch");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_zoom_aa_add) == 0x070, "+070h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_zoom_spring) == 0x074, "+074h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_zoom_drag) == 0x078, "+078h");
static_assert(offsetof(GameplayTuningSettings, pipe_sight_params_zoom_rate) == 0x07C, "+07Ch");
static_assert(offsetof(GameplayTuningSettings, attack_move_director_my_damage_weight) == 0x160, "+160h");
static_assert(offsetof(GameplayTuningSettings, attack_move_director_ideal_dist_weight) == 0x164, "+164h");
static_assert(offsetof(GameplayTuningSettings, attack_move_director_nearby_enemy_weight) == 0x168, "+168h");
static_assert(offsetof(GameplayTuningSettings, attack_move_director_nearby_enemy_reference) == 0x16C, "+16Ch");
static_assert(offsetof(GameplayTuningSettings, attack_move_director_nearest_move_dir_weight) == 0x170, "+170h");
static_assert(offsetof(GameplayTuningSettings, attack_move_director_prev_move_dir_weight) == 0x174, "+174h");
static_assert(offsetof(GameplayTuningSettings, attack_move_director_prev_move_dir_range) == 0x178, "+178h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_collect_timer_1) == 0x190, "+190h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_collect_timer_2) == 0x194, "+194h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_collect_dist) == 0x198, "+198h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_collect_hit_time) == 0x19C, "+19Ch");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_arrive_time_min) == 0x1A0, "+1A0h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_arrive_dist_min) == 0x1A4, "+1A4h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_pos_speed_corrig) == 0x1A8, "+1A8h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_est_pos_dist_limit_mul) == 0x1AC, "+1ACh");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_est_pos_min_ship_length) == 0x1B0, "+1B0h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_est_pos_ship_length_limit_mul) == 0x1B4, "+1B4h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_my_min_spd_ratio) == 0x1B8, "+1B8h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_est_pos_ship_spd_mul) == 0x1BC, "+1BCh");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_est_pos_size_dec_mul) == 0x1C0, "+1C0h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_est_pos_size_dec_min) == 0x1C4, "+1C4h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_next_corner_reach_dist_add_on) == 0x1C8, "+1C8h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_move_path_line_check_threshold) == 0x1CC, "+1CCh");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_go_away_spd_add) == 0x1D0, "+1D0h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_nearby_ship_way_clear_check_time) == 0x1D4, "+1D4h");
static_assert(offsetof(GameplayTuningSettings, ship_avoidance_hit_detector_last_hit_dist_add_on) == 0x1D8, "+1D8h");
static_assert(offsetof(GameplayTuningSettings, torpedo_avoidance_collect_timer_1) == 0x1EC, "+1ECh");
static_assert(offsetof(GameplayTuningSettings, torpedo_avoidance_collect_timer_2) == 0x1F0, "+1F0h");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_check_move_pos_zone_time_1) == 0x1F4, "+1F4h");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_check_move_pos_zone_time_2) == 0x1F8, "+1F8h");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_check_ship_pos_zone_time_1) == 0x1FC, "+1FCh");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_check_ship_pos_zone_time_2) == 0x200, "+200h");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_check_travel_zone_time_1) == 0x204, "+204h");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_check_travel_zone_time_2) == 0x208, "+208h");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_collect_timer_1) == 0x20C, "+20Ch");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_collect_timer_2) == 0x210, "+210h");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_yturn_dir_diff_1) == 0x214, "+214h");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_yturn_dir_diff_2) == 0x218, "+218h");
static_assert(offsetof(GameplayTuningSettings, land_avoidance_way_checker_update_time) == 0x21C, "+21Ch");
static_assert(offsetof(GameplayTuningSettings, avoidance_cheat_turn_spd_limit) == 0x220, "+220h");
static_assert(offsetof(GameplayTuningSettings, avoidance_cheat_turn_spd_mul) == 0x224, "+224h");
static_assert(offsetof(GameplayTuningSettings, avoidance_cheat_stop_spd_mul) == 0x228, "+228h");
static_assert(offsetof(GameplayTuningSettings, retreat_exit_dist) == 0x22C, "+22Ch");
static_assert(offsetof(GameplayTuningSettings, retreat_exit_time) == 0x230, "+230h");
static_assert(offsetof(GameplayTuningSettings, retreat_warning_repeat_time) == 0x234, "+234h");
static_assert(offsetof(GameplayTuningSettings, hack_rotation_add) == 0x238, "+238h");
static_assert(offsetof(GameplayTuningSettings, hack_height_add) == 0x23C, "+23Ch");
static_assert(offsetof(GameplayTuningSettings, colli_dolgok_colli_damage_multiplier) == 0x3A0, "+3A0h");
static_assert(offsetof(GameplayTuningSettings, fire_tick_damage) == 0x3AC, "+3ACh");
static_assert(offsetof(GameplayTuningSettings, water_tick_damage) == 0x3B0, "+3B0h");
static_assert(offsetof(GameplayTuningSettings, body_repair_tick_percentage) == 0x3B4, "+3B4h");
static_assert(offsetof(GameplayTuningSettings, gun_repair_tick_percentage) == 0x3B8, "+3B8h");
static_assert(offsetof(GameplayTuningSettings, fire_failure_chance) == 0x3BC, "+3BCh");
static_assert(offsetof(GameplayTuningSettings, fire_failure_damage_duration) == 0x3C0, "+3C0h");
static_assert(offsetof(GameplayTuningSettings, explosion_damage_percentage) == 0x3C4, "+3C4h");
static_assert(offsetof(GameplayTuningSettings, pump_repair_multiplier) == 0x3C8, "+3C8h");
static_assert(offsetof(GameplayTuningSettings, fire_repair_multiplier) == 0x3CC, "+3CCh");
static_assert(offsetof(GameplayTuningSettings, failure_repair_multiplier) == 0x3D0, "+3D0h");
static_assert(offsetof(GameplayTuningSettings, body_repair_multiplier) == 0x3D4, "+3D4h");
static_assert(offsetof(GameplayTuningSettings, gun_repair_multiplier) == 0x3D8, "+3D8h");
static_assert(offsetof(GameplayTuningSettings, failure_chance) == 0x3DC, "+3DCh");
static_assert(offsetof(GameplayTuningSettings, failure_damage_threshold) == 0x3E0, "+3E0h");
static_assert(offsetof(GameplayTuningSettings, vizbeomles_dolgok_kill_depth) == 0x3F4, "+3F4h");
static_assert(offsetof(GameplayTuningSettings, vizbeomles_dolgok_ptboat_leak_size) == 0x3F8, "+3F8h");
static_assert(offsetof(GameplayTuningSettings, vizbeomles_dolgok_leak_per_hp) == 0x3FC, "+3FCh");
static_assert(offsetof(GameplayTuningSettings, vizbeomles_dolgok_max_leak_percent) == 0x400, "+400h");
static_assert(offsetof(GameplayTuningSettings, vizbeomles_dolgok_ennyi_viz_es_kesz_percent) == 0x404, "+404h");
static_assert(offsetof(GameplayTuningSettings, vizbeomles_dolgok_water_section_num) == 0x408, "+408h");
static_assert(offsetof(GameplayTuningSettings, vizbeomles_dolgok_dolog_szorzo) == 0x40C, "+40Ch");
static_assert(offsetof(GameplayTuningSettings, formacio_heading_mul) == 0x410, "+410h");
static_assert(offsetof(GameplayTuningSettings, formacio_heading_power) == 0x414, "+414h");
static_assert(offsetof(GameplayTuningSettings, formacio_thrust_mul) == 0x418, "+418h");
static_assert(offsetof(GameplayTuningSettings, formacio_follower_speed_mul) == 0x41C, "+41Ch");
static_assert(offsetof(GameplayTuningSettings, formacio_formation_max_count) == 0x420, "+420h");
static_assert(offsetof(GameplayTuningSettings, formacio_follower_max_dist) == 0x424, "+424h");
static_assert(offsetof(GameplayTuningSettings, formacio_formation_ship_dist) == 0x42C, "+42Ch");
static_assert(offsetof(GameplayTuningSettings, formacio_update_interval) == 0x430, "+430h");
static_assert(offsetof(GameplayTuningSettings, formacio_player_follower_max_dist) == 0x434, "+434h");
static_assert(offsetof(GameplayTuningSettings, navigator_turn_multipliers_turn_multiplier_max_speed_2) == 0x438, "+438h");
static_assert(offsetof(GameplayTuningSettings, navigator_turn_multipliers_turn_multiplier_max_speed_1) == 0x43C, "+43Ch");
static_assert(offsetof(GameplayTuningSettings, navigator_turn_multipliers_turn_multiplier_min_speed_2) == 0x440, "+440h");
static_assert(offsetof(GameplayTuningSettings, navigator_turn_multipliers_turn_multiplier_min_speed_1) == 0x444, "+444h");
static_assert(offsetof(GameplayTuningSettings, navigator_turn_multipliers_turn_multiplier_med_speed_2) == 0x448, "+448h");
static_assert(offsetof(GameplayTuningSettings, navigator_turn_multipliers_turn_multiplier_med_speed_1) == 0x44C, "+44Ch");
static_assert(offsetof(GameplayTuningSettings, ship_camera_zoom_offset) == 0x450, "+450h");
static_assert(offsetof(GameplayTuningSettings, ship_camera_length_mult) == 0x454, "+454h");
static_assert(offsetof(GameplayTuningSettings, ship_camera_min_camera_angle) == 0x458, "+458h");
static_assert(offsetof(GameplayTuningSettings, ship_camera_max_camera_angle) == 0x45C, "+45Ch");
static_assert(offsetof(GameplayTuningSettings, ship_camera_follow_cam_delay) == 0x460, "+460h");
static_assert(offsetof(GameplayTuningSettings, repair_repair_script_interval) == 0x464, "+464h");
static_assert(offsetof(GameplayTuningSettings, repair_repair_step_value) == 0x468, "+468h");
static_assert(offsetof(GameplayTuningSettings, repair_repair_team_count) == 0x46C, "+46Ch");
static_assert(offsetof(GameplayTuningSettings, repair_repair_team_bonus) == 0x470, "+470h");
static_assert(offsetof(GameplayTuningSettings, repair_repair_team_penalty) == 0x474, "+474h");
static_assert(offsetof(GameplayTuningSettings, repair_repair_underwater_modifier) == 0x478, "+478h");
static_assert(offsetof(GameplayTuningSettings, repair_repair_zone_multiplier) == 0x494, "+494h");
static_assert(offsetof(GameplayTuningSettings, repair_torpedo_restock_time) == 0x498, "+498h");
static_assert(offsetof(GameplayTuningSettings, submarine_submarine_periscope_level) == 0x49C, "+49Ch");
static_assert(offsetof(GameplayTuningSettings, submarine_submarine_medium_level) == 0x4A0, "+4A0h");
static_assert(offsetof(GameplayTuningSettings, submarine_submarine_deep_level) == 0x4A4, "+4A4h");
static_assert(offsetof(GameplayTuningSettings, submarine_submarine_torpedo_range) == 0x4A8, "+4A8h");
static_assert(offsetof(GameplayTuningSettings, submarine_submarine_depth_damage) == 0x4AC, "+4ACh");
static_assert(offsetof(GameplayTuningSettings, submarine_submarine_damage_depth) == 0x4B0, "+4B0h");
static_assert(offsetof(GameplayTuningSettings, submarine_submarine_depth_speed_mul) == 0x4B4, "+4B4h");
static_assert(offsetof(GameplayTuningSettings, submarine_submarine_air_warning_limit) == 0x4B8, "+4B8h");
static_assert(offsetof(GameplayTuningSettings, submarine_submarine_air_need_limit) == 0x4BC, "+4BCh");
static_assert(offsetof(GameplayTuningSettings, submarine_submarine_air_enough_limit) == 0x4C0, "+4C0h");
static_assert(offsetof(GameplayTuningSettings, submarine_periscope_repair_time) == 0x4C4, "+4C4h");
static_assert(offsetof(GameplayTuningSettings, sub_attack_max_torpedo_range) == 0x4C8, "+4C8h");
static_assert(offsetof(GameplayTuningSettings, sub_attack_too_close_dist) == 0x4CC, "+4CCh");
static_assert(offsetof(GameplayTuningSettings, sub_attack_far_enough_dist) == 0x4D0, "+4D0h");
static_assert(offsetof(GameplayTuningSettings, sub_attack_submarine_lost_time) == 0x4D4, "+4D4h");
static_assert(offsetof(GameplayTuningSettings, mother_ship_elevator_speed) == 0x4D8, "+4D8h");
static_assert(offsetof(GameplayTuningSettings, mother_ship_elevator_depth) == 0x4DC, "+4DCh");
static_assert(offsetof(GameplayTuningSettings, physics_tboat_nyomatek_szorzo) == 0x588, "+588h");
static_assert(offsetof(GameplayTuningSettings, physics_tboat_motor_max_szog) == 0x58C, "+58Ch");
static_assert(offsetof(GameplayTuningSettings, physics_torpedo_force) == 0x590, "+590h");
static_assert(offsetof(GameplayTuningSettings, physics_torpedo_force_power) == 0x594, "+594h");
static_assert(offsetof(GameplayTuningSettings, dofparams_dist) == 0x598, "+598h");
static_assert(offsetof(GameplayTuningSettings, dofparams_range1) == 0x59C, "+59Ch");
static_assert(offsetof(GameplayTuningSettings, dofparams_range2) == 0x5A0, "+5A0h");
static_assert(offsetof(GameplayTuningSettings, dofparams_min_amount) == 0x5A4, "+5A4h");
static_assert(offsetof(GameplayTuningSettings, dofparams_max_amount) == 0x5A8, "+5A8h");
static_assert(offsetof(GameplayTuningSettings, torpedo_hit_life_time) == 0x5AC, "+5ACh");
static_assert(offsetof(GameplayTuningSettings, torpedo_hit_upthrust) == 0x5B0, "+5B0h");
static_assert(offsetof(GameplayTuningSettings, torpedo_hit_up_dist) == 0x5B4, "+5B4h");
static_assert(offsetof(GameplayTuningSettings, torpedo_hit_mass_ratio) == 0x5B8, "+5B8h");
static_assert(offsetof(GameplayTuningSettings, sounds_ship_dead_meat_sound_time_min) == 0x64C, "+64Ch");
static_assert(offsetof(GameplayTuningSettings, sounds_ship_dead_meat_sound_time_max) == 0x650, "+650h");
static_assert(offsetof(GameplayTuningSettings, sounds_plane_dead_meat_freq0) == 0x654, "+654h");
static_assert(offsetof(GameplayTuningSettings, sounds_plane_dead_meat_freq1000) == 0x658, "+658h");
static_assert(offsetof(GameplayTuningSettings, physics_wreck_chance) == 0x65C, "+65Ch");
static_assert(offsetof(GameplayTuningSettings, debris_struct_debris_enabled) == 0x680, "+680h");
static_assert(offsetof(GameplayTuningSettings, debris_struct_debris_num_per_meter) == 0x684, "+684h");
static_assert(offsetof(GameplayTuningSettings, flag_need_flag) == 0x688, "+688h");
static_assert(offsetof(GameplayTuningSettings, flag_wind_power_min) == 0x68C, "+68Ch");
static_assert(offsetof(GameplayTuningSettings, flag_wind_power_max) == 0x690, "+690h");
static_assert(offsetof(GameplayTuningSettings, flag_wind_power_change) == 0x694, "+694h");
static_assert(offsetof(GameplayTuningSettings, flag_wind_dir_change) == 0x698, "+698h");
static_assert(offsetof(GameplayTuningSettings, flag_wind_dir_min) == 0x69C, "+69Ch");
static_assert(offsetof(GameplayTuningSettings, flag_wind_dir_max) == 0x6A0, "+6A0h");
static_assert(offsetof(GameplayTuningSettings, flag_gravity) == 0x6A4, "+6A4h");
static_assert(offsetof(GameplayTuningSettings, flag_drag) == 0x6A8, "+6A8h");
static_assert(offsetof(GameplayTuningSettings, flag_dt_mul) == 0x6AC, "+6ACh");
static_assert(offsetof(GameplayTuningSettings, flag_dt_step) == 0x6B0, "+6B0h");
static_assert(offsetof(GameplayTuningSettings, flag_num_iteration) == 0x6B4, "+6B4h");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_steer_value_min_slow) == 0x6C4, "+6C4h");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_steer_value_max_slow) == 0x6C8, "+6C8h");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_hdg_diff_value_min_slow) == 0x6CC, "+6CCh");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_hdg_diff_value_max_slow) == 0x6D0, "+6D0h");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_thrust_min_slow) == 0x6D4, "+6D4h");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_steer_value_min_fast) == 0x6D8, "+6D8h");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_steer_value_max_fast) == 0x6DC, "+6DCh");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_hdg_diff_value_min_fast) == 0x6E0, "+6E0h");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_hdg_diff_value_max_fast) == 0x6E4, "+6E4h");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_thrust_min_fast) == 0x6E8, "+6E8h");
static_assert(offsetof(GameplayTuningSettings, navigator_auto_thrust_hdg_diff_danger_mul) == 0x6EC, "+6ECh");
static_assert(offsetof(GameplayTuningSettings, navigator_path_finder_params_length_modifier_dir_diff_min) == 0x6F0, "+6F0h");
static_assert(offsetof(GameplayTuningSettings, navigator_path_finder_params_length_modifier_dir_diff_max) == 0x6F4, "+6F4h");
static_assert(offsetof(GameplayTuningSettings, navigator_path_finder_params_length_modifier_length_addon) == 0x6F8, "+6F8h");
static_assert(offsetof(GameplayTuningSettings, navigator_island_attack_turn_circle_multiplier) == 0x6FC, "+6FCh");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_predict_time) == 0x700, "+700h");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_separation_strength) == 0x704, "+704h");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_follow_strength) == 0x708, "+708h");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_separation_min_dist_mult) == 0x70C, "+70Ch");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_separation_max_dist_mult) == 0x710, "+710h");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_move_treshold) == 0x714, "+714h");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_thrust_treshold) == 0x718, "+718h");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_back_steer_speed) == 0x71C, "+71Ch");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_vector_multiplier) == 0x720, "+720h");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_leader_max_hdg) == 0x724, "+724h");
static_assert(offsetof(GameplayTuningSettings, navigator_flock_leader_max_side_move) == 0x728, "+728h");
static_assert(offsetof(GameplayTuningSettings, vizbeomles_dolgok_sink_effect_ennyi_meterenkent) == 0x734, "+734h");
static_assert(offsetof(GameplayTuningSettings, vizbeomles_dolgok_sink_effect_ilyen_gyakran) == 0x738, "+738h");
static_assert(offsetof(GameplayTuningSettings, fire_fire_damage_per_fire_tick) == 0x74C, "+74Ch");
static_assert(offsetof(GameplayTuningSettings, aagunner_error_modifier_versus_ai) == 0x750, "+750h");
static_assert(offsetof(GameplayTuningSettings, aagunner_error_modifier_versus_player) == 0x754, "+754h");
static_assert(offsetof(GameplayTuningSettings, aagunner_error_modifier_calc_target_pos_time_add_fix) == 0x758, "+758h");
static_assert(offsetof(GameplayTuningSettings, aagunner_error_modifier_calc_target_pos_time_add_mul) == 0x75C, "+75Ch");
static_assert(offsetof(GameplayTuningSettings, aagunner_error_modifier_turn_off_aagun_throw) == 0x760, "+760h");
static_assert(offsetof(GameplayTuningSettings, sub_torpedo_delay) == 0x764, "+764h");
static_assert(offsetof(GameplayTuningSettings, ship_torpedo_delay) == 0x768, "+768h");

// The defaults 00424A10 writes before the loader runs. Every one of these offsets is written
// again by the loader, so they are the pre-load state, not the values the game runs with;
// they are kept because the loader's own defaults differ from them (+5A0h is 1000.0 here and
// the loader's default for DOFParams.Range2 is 1.0).
void apply_constructor_defaults_00424a10(GameplayTuningSettings& out) noexcept;

// One virtual per native getter call site in 0083B5E0. `enter` replaces the current-table
// wrapper (00B67800 on the globals, then 00B67690 to assign a sub-table into the slot); the
// typed reads are 00B66270 GetNumber, 00B66290 GetInteger, 00B66250 GetBoolean and the
// *OrDefault forms 00B66330, 00B66380 and 00B662F0. The one-based index argument is the
// 00B67720 GetByIndex step. There are no default implementations.
struct GameplayTuningRowView {
    virtual ~GameplayTuningRowView() = default;
    // 00B67800 on the globals table, then one per path segment; an empty call selects
    // ShipGlobals itself.
    virtual void enter() = 0;
    virtual void enter(const char* a) = 0;
    virtual void enter(const char* a, const char* b) = 0;
    virtual float number(const char* key) = 0;                       // 00B66270
    virtual float number(const char* key, int index) = 0;            // 00B67720 then 00B66270
    virtual float number_or(const char* key, float fallback) = 0;    // 00B66330
    virtual float number_or(const char* key, int index, float fallback) = 0;
    virtual std::int32_t integer(const char* key) = 0;               // 00B66290
    virtual std::int32_t integer(const char* key, int index) = 0;
    virtual std::int32_t integer_or(const char* key, std::int32_t fallback) = 0;  // 00B66380
    virtual std::int32_t integer_or(const char* key, int index, std::int32_t fallback) = 0;
    virtual bool boolean(const char* key) = 0;                       // 00B66250
    virtual bool boolean(const char* key, int index) = 0;
    virtual bool boolean_or(const char* key, bool fallback) = 0;     // 00B662F0
    virtual bool boolean_or(const char* key, int index, bool fallback) = 0;
};

// Arithmetic/store projection of0083DA26..0083DA66, repeated for+3C/+40 at
// 0083DA9B..0083DADB and0083DB10..0083DB50. `duration` is the float32 value
// returned by the existing row getter; the native clamp retains unordered NaN.
// Uses the caller's x87 control word and writes one float32 reciprocal rate.
// No Lua wrapper lifecycle or native SEH state is represented by this API.
// Evidence: docs/GAMEPLAY_THROW_RATES.md. New C++ ABI, not a native replacement.
void store_gameplay_artillery_throw_rate_0083da26(float duration, float& rate) noexcept;

// The keyed projection of 0083B5E0, grouped by settings fields. The overall
// sequence is not the native loader's complete call/error order. Within the
// PlayerArtilleryThrow block, each read/store precedes the next read. The unkeyed parts of the routine
// (the four 58h sub-objects, the failure descriptor vector at +3E8h, the three physics
// material records at +4E0h, the per-class sound records at +5BCh and the effect-name reads)
// are not in this sequence; docs/GAMEPLAY_SETTINGS.md lists them by address range.
void load_gameplay_tuning_settings(GameplayTuningRowView& rows,
                                   GameplayTuningSettings& out);

// FailureDebug in the installed scripts/datatables/ScriptOptions.lua is false, so the second
// assignment block of shipglobals.lua (lines 746..800) does not run and the damage constants
// at +3ACh..+3E0h keep their first-block values.
inline constexpr bool kInstalledFailureDebug = false;

}  // namespace bsp
