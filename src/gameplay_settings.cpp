// The gameplay tuning settings loader 0083B5E0, as a sequence over an injected row view.
// docs/GAMEPLAY_SETTINGS.md.
//
// Routines read for this file:
//   00424A10, 00424A10..00424C15 (the constructor, in full)
//   00424C40, 00424C40..00424CFF (the lazy singleton, in full)
//   0083B5E0, 0083B5E0..00842951 (the loader listing, swept by script for its stores)
//
// Nothing here is a binary-compatible replacement.

#include "bsp/gameplay_settings.hpp"

namespace bsp {

void store_gameplay_artillery_throw_rate_0083da26(float duration, float& rate) noexcept {
    // 00D7A3A0: 000000A09999B93F; the comparison double is widened0.1f.
    // 00D7A2F0: CDCCCC3D; replacement is the same float32 value.
    static constexpr float duration_floor = 0.1f;
    static constexpr double duration_floor_compare = static_cast<double>(duration_floor);
    float* destination = &rate;
    __asm {
        fld duration
        fstp duration
        fld duration
        fld duration_floor_compare
        fcomip st(0),st(1)
        fstp st(0)
        jbe keep_duration
        movss xmm0,duration_floor
        jmp selected_duration
    keep_duration:
        movss xmm0,duration
    selected_duration:
        movss duration,xmm0
        fld duration
        fld1
        fdivrp st(1),st(0)
        mov eax,destination
        fstp dword ptr [eax]
    }
}

void apply_constructor_defaults_00424a10(GameplayTuningSettings& out) noexcept {
    // 00424A8D..00424BBA, in store order. The addresses in the comments are the constants.
    out.dofparams_dist = 1.0f;             // 00424AC5, 00D7A24C
    out.dofparams_range1 = 1.0f;           // 00424ACD, 00D7A24C
    out.dofparams_range2 = 1000.0f;        // 00424AA5, 00CE3804
    out.dofparams_min_amount = 1.0f;       // 00424AD5, 00D7A24C
    out.dofparams_max_amount = 0.5f;       // 00424AB5, 00CE3800
    out.torpedo_hit_life_time = 1.2f;       // 00424ADD, 00CE3814
    out.torpedo_hit_upthrust = 2000000.0f;  // 00424AED, 00CE3810
    out.torpedo_hit_up_dist = 1.5f;         // 00424AFD, 00CE380C
    out.torpedo_hit_mass_ratio = 150.0f;    // 00424B05, 00CE3808
    out.flag_need_flag = true;              // 00424B9B
    out.flag_wind_power_min = 8.0f;         // 00424B4B, 00CE3918
    out.flag_wind_power_max = 9.0f;         // 00424B5B, 00CE3914
    out.flag_wind_power_change = 0.04f;     // 00424B7B, 00CE3910
    out.flag_wind_dir_change = 0.01f;       // 00424B8B, 00D7A238
    out.flag_wind_dir_min = -1.0f;          // 00424BA2, 00D7A260
    out.flag_wind_dir_max = 1.0f;           // 00424B6B, 00D7A24C
    out.flag_dt_mul = 1.5f;                 // 00424BAA, 00CE380C
    out.flag_dt_step = 0.016666668f;        // 00424BB2, 00CE390C
    out.flag_num_iteration = 10;            // 00424BBA
}

void load_gameplay_tuning_settings(GameplayTuningRowView& rows,
                                   GameplayTuningSettings& out) {
    rows.enter("Sinking");
    out.sinking_drag_multiplier = rows.number_or("DragMultiplier", 1.0f);  // 0083d6ab
    out.sinking_drag_multiplier_z = rows.number_or("DragMultiplierZ", 1.0f);  // 0083d6f0
    out.sinking_leak_size = rows.number_or("LeakSize", 1.0f);  // 0083d735
    out.sinking_drag_power = rows.number_or("DragPower", 1.0f);  // 0083d666
    out.sinking_start_angular_velocity = rows.number_or("StartAngularVelocity", 30.0f);  // 0083d605
    out.sinking_section_num = rows.integer_or("SectionNum", 4);  // 0083d5bc
    rows.enter("Debris");
    out.debris_splash_speed_1 = rows.number_or("SplashSpeed", 1, 5.0f);  // 0083d7b8
    out.debris_splash_speed_2 = rows.number_or("SplashSpeed", 2, 20.0f);  // 0083d82e
    out.debris_splash_fxid_1 = rows.number_or("SplashFXID", 1, 9.0f);  // 0083d8a4
    out.debris_splash_fxid_2 = rows.number_or("SplashFXID", 2, 52.0f);  // 0083d91a
    rows.enter("PlayerArtilleryThrow");
    out.player_artillery_throw_after_shot_fire_time = rows.number_or("AfterShot_FireTime", 3.0f);  // 0083d998
    out.player_artillery_throw_after_shot_wait_time = rows.number_or("AfterShot_WaitTime", 3.0f);  // 0083d9de
    store_gameplay_artillery_throw_rate_0083da26(
        rows.number_or("ThrowIncrementTime_HasTarget", 12.0f),
        out.player_artillery_throw_throw_increment_time_has_target);  // 0083da64
    store_gameplay_artillery_throw_rate_0083da26(
        rows.number_or("ThrowIncrementTime_NoTarget", 8.0f),
        out.player_artillery_throw_throw_increment_time_no_target);  // 0083dad9
    store_gameplay_artillery_throw_rate_0083da26(
        rows.number_or("ThrowDecrementTime", 10.0f),
        out.player_artillery_throw_throw_decrement_time);  // 0083db4e
    rows.enter("PipeSightParams");
    out.pipe_sight_params_pipesight_enabled = rows.boolean("pipesight_enabled");  // 0083db94
    out.pipe_sight_params_blur_heavy_add = rows.number_or("blur_heavy_add", 0.3f);  // 0083dca5
    out.pipe_sight_params_blur_medium_add = rows.number_or("blur_medium_add", 0.3f);  // 0083dceb
    out.pipe_sight_params_blur_light_add = rows.number_or("blur_light_add", 0.3f);  // 0083dd31
    out.pipe_sight_params_blur_aa_add = rows.number_or("blur_aa_add", 0.3f);  // 0083dd77
    out.pipe_sight_params_blur_spring = rows.number_or("blur_spring", 1.0f);  // 0083dc19
    out.pipe_sight_params_blur_drag = rows.number_or("blur_drag", 0.9f);  // 0083dbd7
    out.pipe_sight_params_blur_rate = rows.number_or("blur_rate", 0.4f);  // 0083dc5f
    out.pipe_sight_params_zoom_heavy_add = rows.number_or("zoom_heavy_add", 0.3f);  // 0083de8b
    out.pipe_sight_params_zoom_medium_add = rows.number_or("zoom_medium_add", 0.3f);  // 0083ded1
    out.pipe_sight_params_zoom_light_add = rows.number_or("zoom_light_add", 0.3f);  // 0083df17
    out.pipe_sight_params_zoom_aa_add = rows.number_or("zoom_aa_add", 0.3f);  // 0083df5d
    out.pipe_sight_params_zoom_spring = rows.number_or("zoom_spring", 1.0f);  // 0083ddff
    out.pipe_sight_params_zoom_drag = rows.number_or("zoom_drag", 0.9f);  // 0083ddbd
    out.pipe_sight_params_zoom_rate = rows.number_or("zoom_rate", 0.4f);  // 0083de45
    rows.enter("AttackMoveDirector");
    out.attack_move_director_my_damage_weight = rows.number_or("MyDamageWeight", 10.0f);  // 0083c96c
    out.attack_move_director_ideal_dist_weight = rows.number_or("IdealDistWeight", 4.0f);  // 0083c9b8
    out.attack_move_director_nearby_enemy_weight = rows.number_or("NearbyEnemyWeight", 3.0f);  // 0083ca01
    out.attack_move_director_nearby_enemy_reference = rows.number_or("NearbyEnemyReference", 1000.0f);  // 0083ca47
    out.attack_move_director_nearest_move_dir_weight = rows.number_or("NearestMoveDirWeight", 1.0f);  // 0083ca8c
    out.attack_move_director_prev_move_dir_weight = rows.number_or("PrevMoveDirWeight", 0.25f);  // 0083cad8
    out.attack_move_director_prev_move_dir_range = rows.number_or("PrevMoveDirRange", 1.0472f);  // 0083cb24
    rows.enter("ShipAvoidance");
    out.ship_avoidance_collect_timer_1 = rows.number("CollectTimer", 1);  // 0083b7ad
    out.ship_avoidance_collect_timer_2 = rows.number("CollectTimer", 2);  // 0083b810
    out.ship_avoidance_collect_dist = rows.number("CollectDist");  // 0083b85d
    out.ship_avoidance_collect_hit_time = rows.number("CollectHitTime");  // 0083b899
    out.ship_avoidance_nearby_ship_arrive_time_min = rows.number_or("NearbyShip_ArriveTimeMin", 3.0f);  // 0083b8df
    out.ship_avoidance_nearby_ship_arrive_dist_min = rows.number_or("NearbyShip_ArriveDistMin", 0.25f);  // 0083b925
    out.ship_avoidance_nearby_ship_pos_speed_corrig = rows.number_or("NearbyShip_PosSpeedCorrig", 2.0f);  // 0083b96b
    out.ship_avoidance_nearby_ship_est_pos_dist_limit_mul = rows.number_or("NearbyShip_EstPos_DistLimitMul", 0.7f);  // 0083b9b1
    out.ship_avoidance_nearby_ship_est_pos_min_ship_length = rows.number_or("NearbyShip_EstPos_MinShipLength", 100.0f);  // 0083b9f7
    out.ship_avoidance_nearby_ship_est_pos_ship_length_limit_mul = rows.number_or("NearbyShip_EstPos_ShipLengthLimitMul", 1.8f);  // 0083ba3d
    out.ship_avoidance_nearby_ship_my_min_spd_ratio = rows.number_or("NearbyShip_MyMinSpdRatio", 0.6f);  // 0083ba83
    out.ship_avoidance_nearby_ship_est_pos_ship_spd_mul = rows.number_or("NearbyShip_EstPos_ShipSpdMul", 0.75f);  // 0083bac9
    out.ship_avoidance_nearby_ship_est_pos_size_dec_mul = rows.number_or("NearbyShip_EstPos_SizeDecMul", 0.3f);  // 0083bb0f
    out.ship_avoidance_nearby_ship_est_pos_size_dec_min = rows.number_or("NearbyShip_EstPos_SizeDecMin", 0.1f);  // 0083bb55
    out.ship_avoidance_nearby_ship_next_corner_reach_dist_add_on = rows.number_or("NearbyShip_NextCornerReachDistAddOn", 100.0f);  // 0083bb9b
    out.ship_avoidance_nearby_ship_move_path_line_check_threshold = rows.number_or("NearbyShip_MovePathLineCheckThreshold", 200.0f);  // 0083bbe1
    out.ship_avoidance_nearby_ship_go_away_spd_add = rows.number_or("NearbyShip_GoAwaySpdAdd", 3.0f);  // 0083bcb3
    out.ship_avoidance_nearby_ship_way_clear_check_time = rows.number_or("NearbyShip_WayClearCheckTime", 0.5f);  // 0083bc27
    out.ship_avoidance_hit_detector_last_hit_dist_add_on = rows.number_or("HitDetector_LastHitDistAddOn", 30.0f);  // 0083bc6d
    rows.enter("TorpedoAvoidance");
    out.torpedo_avoidance_collect_timer_1 = rows.number("CollectTimer", 1);  // 0083bf3f
    out.torpedo_avoidance_collect_timer_2 = rows.number("CollectTimer", 2);  // 0083bfa8
    rows.enter("LandAvoidance");
    out.land_avoidance_check_move_pos_zone_time_1 = rows.number("CheckMovePosZoneTime", 1);  // 0083c032
    out.land_avoidance_check_move_pos_zone_time_2 = rows.number("CheckMovePosZoneTime", 2);  // 0083c09b
    out.land_avoidance_check_ship_pos_zone_time_1 = rows.number("CheckShipPosZoneTime", 1);  // 0083c104
    out.land_avoidance_check_ship_pos_zone_time_2 = rows.number("CheckShipPosZoneTime", 2);  // 0083c16d
    out.land_avoidance_check_travel_zone_time_1 = rows.number("CheckTravelZoneTime", 1);  // 0083c1d6
    out.land_avoidance_check_travel_zone_time_2 = rows.number("CheckTravelZoneTime", 2);  // 0083c23f
    out.land_avoidance_collect_timer_1 = rows.number("CollectTimer", 1);  // 0083c2ab
    out.land_avoidance_collect_timer_2 = rows.number("CollectTimer", 2);  // 0083c31a
    out.land_avoidance_yturn_dir_diff_1 = rows.number_or("YTurnDirDiff", 1, 1.8f);  // 0083c393
    out.land_avoidance_yturn_dir_diff_2 = rows.number_or("YTurnDirDiff", 2, 2.1f);  // 0083c40c
    out.land_avoidance_way_checker_update_time = rows.number_or("WayCheckerUpdateTime", 1.5f);  // 0083c46c
    rows.enter("AvoidanceCheat");
    out.avoidance_cheat_turn_spd_limit = rows.number_or("TurnSpdLimit", 5.0f);  // 0083c4d9
    out.avoidance_cheat_turn_spd_mul = rows.number_or("TurnSpdMul", 0.6f);  // 0083c531
    out.avoidance_cheat_stop_spd_mul = rows.number_or("StopSpdMul", 2.0f);  // 0083c57d
    rows.enter("Retreat");
    out.retreat_exit_dist = rows.number("ExitDist");  // 0083c6f2
    out.retreat_exit_time = rows.number("ExitTime");  // 0083c734
    out.retreat_warning_repeat_time = rows.number("WarningRepeatTime");  // 0083c776
    rows.enter("Hack");
    out.hack_rotation_add = rows.number_or("RotationAdd", 0.0f);  // 0083c5fa
    out.hack_height_add = rows.number_or("HeightAdd", 0.0f);  // 0083c642
    rows.enter("ColliDolgok");
    out.colli_dolgok_colli_damage_multiplier = rows.number_or("ColliDamageMultiplier", 0.0f);  // 0083dfd4
    rows.enter();
    out.fire_tick_damage = rows.number_or("FireTickDamage", 0.0f);  // 0083e1f5
    out.water_tick_damage = rows.number_or("WaterTickDamage", 0.0f);  // 0083e1b3
    out.body_repair_tick_percentage = rows.number_or("BodyRepairTickPercentage", 0.2f);  // 0083e23e
    out.gun_repair_tick_percentage = rows.number_or("GunRepairTickPercentage", 2.0f);  // 0083e290
    out.fire_failure_chance = rows.number_or("FireFailureChance", 1.0f);  // 0083e2de
    out.fire_failure_damage_duration = rows.number_or("FireFailureDamageDuration", 1.0f);  // 0083e32c
    out.explosion_damage_percentage = rows.number_or("ExplosionDamagePercentage", 1.0f);  // 0083e374
    out.pump_repair_multiplier = rows.number_or("PumpRepairMultiplier", 2.0f);  // 0083e3c6
    out.fire_repair_multiplier = rows.number_or("FireRepairMultiplier", 2.0f);  // 0083e412
    out.failure_repair_multiplier = rows.number_or("FailureRepairMultiplier", 2.0f);  // 0083e45e
    out.body_repair_multiplier = rows.number_or("BodyRepairMultiplier", 2.0f);  // 0083e4aa
    out.gun_repair_multiplier = rows.number_or("GunRepairMultiplier", 2.0f);  // 0083e4f6
    out.failure_chance = rows.number_or("FailureChance", 5.0f);  // 0083e542
    out.failure_damage_threshold = rows.number_or("FailureDamageThreshold", 100.0f);  // 0083e594
    rows.enter("VizbeomlesDolgok");
    out.vizbeomles_dolgok_kill_depth = rows.number("KillDepth");  // 0083ea71
    out.vizbeomles_dolgok_ptboat_leak_size = rows.number("PTBoatLeakSize");  // 0083eaad
    out.vizbeomles_dolgok_leak_per_hp = rows.number_or("LeakPerHP", 0.1f);  // 0083edda
    out.vizbeomles_dolgok_max_leak_percent = rows.number_or("MaxLeakPercent", 0.01f);  // 0083ee26
    out.vizbeomles_dolgok_ennyi_viz_es_kesz_percent = rows.number_or("EnnyiVizEsKeszPercent", 0.8f);  // 0083ee72
    out.vizbeomles_dolgok_water_section_num = rows.number_or("WaterSectionNum", 6.0f);  // 0083ed8e
    out.vizbeomles_dolgok_dolog_szorzo = rows.number_or("DologSzorzo", 4.0f);  // 0083eebe
    rows.enter("Formacio");
    out.formacio_heading_mul = rows.number("HeadingMul");  // 0083ef21
    out.formacio_heading_power = rows.number("HeadingPower");  // 0083ef63
    out.formacio_thrust_mul = rows.number("ThrustMul");  // 0083efa5
    out.formacio_follower_speed_mul = rows.number("FollowerSpeedMul");  // 0083efe7
    out.formacio_formation_max_count = rows.number("FormationMaxCount");  // 0083f0ef
    out.formacio_follower_max_dist = rows.number("FollowerMaxDist");  // 0083f029
    out.formacio_formation_ship_dist = rows.number("FormationShipDist");  // 0083f06b
    out.formacio_update_interval = rows.number("UpdateInterval");  // 0083f0ad
    out.formacio_player_follower_max_dist = rows.number("PlayerFollowerMaxDist");  // 0083f131
    rows.enter("Navigator", "TurnMultipliers");
    out.navigator_turn_multipliers_turn_multiplier_max_speed_2 = rows.number("TurnMultiplierMaxSpeed", 2);  // 0083d104
    out.navigator_turn_multipliers_turn_multiplier_max_speed_1 = rows.number("TurnMultiplierMaxSpeed", 1);  // 0083d095
    out.navigator_turn_multipliers_turn_multiplier_min_speed_2 = rows.number("TurnMultiplierMinSpeed", 2);  // 0083cf48
    out.navigator_turn_multipliers_turn_multiplier_min_speed_1 = rows.number("TurnMultiplierMinSpeed", 1);  // 0083ced9
    out.navigator_turn_multipliers_turn_multiplier_med_speed_2 = rows.number("TurnMultiplierMedSpeed", 2);  // 0083d026
    out.navigator_turn_multipliers_turn_multiplier_med_speed_1 = rows.number("TurnMultiplierMedSpeed", 1);  // 0083cfb7
    rows.enter("ShipCamera");
    out.ship_camera_zoom_offset = rows.number("ZoomOffset");  // 0083f194
    out.ship_camera_length_mult = rows.number("LengthMult");  // 0083f1d6
    out.ship_camera_min_camera_angle = rows.number_or("MinCameraAngle", -89.0f);  // 0083f222
    out.ship_camera_max_camera_angle = rows.number_or("MaxCameraAngle", 89.0f);  // 0083f26e
    out.ship_camera_follow_cam_delay = rows.number_or("FollowCamDelay", 0.3f);  // 0083f2ba
    rows.enter("Repair");
    out.repair_repair_script_interval = rows.number("RepairScriptInterval");  // 0083f3e3
    out.repair_repair_step_value = rows.number("RepairStepValue");  // 0083f425
    out.repair_repair_team_count = rows.number("RepairTeamCount");  // 0083f31d
    out.repair_repair_team_bonus = rows.number("RepairTeamBonus");  // 0083f35f
    out.repair_repair_team_penalty = rows.number("RepairTeamPenalty");  // 0083f3a1
    out.repair_repair_underwater_modifier = rows.number("RepairUnderwaterModifier");  // 0083f467
    out.repair_repair_zone_multiplier = rows.number("RepairZoneMultiplier");  // 0083f511
    out.repair_torpedo_restock_time = rows.number("TorpedoRestockTime");  // 0083f54d
    rows.enter("Submarine");
    out.submarine_submarine_periscope_level = rows.number("SubmarinePeriscopeLevel");  // 0083f5ef
    out.submarine_submarine_medium_level = rows.number("SubmarineMediumLevel");  // 0083f631
    out.submarine_submarine_deep_level = rows.number("SubmarineDeepLevel");  // 0083f673
    out.submarine_submarine_torpedo_range = rows.number("SubmarineTorpedoRange");  // 0083f5ad
    out.submarine_submarine_depth_damage = rows.number("SubmarineDepthDamage");  // 0083f81c
    out.submarine_submarine_damage_depth = rows.number("SubmarineDamageDepth");  // 0083f85e
    out.submarine_submarine_depth_speed_mul = rows.number("SubmarineDepthSpeedMul");  // 0083f8a0
    out.submarine_submarine_air_warning_limit = rows.number("SubmarineAirWarningLimit");  // 0083f8e2
    out.submarine_submarine_air_need_limit = rows.number("SubmarineAirNeedLimit");  // 0083f924
    out.submarine_submarine_air_enough_limit = rows.number("SubmarineAirEnoughLimit");  // 0083f966
    out.submarine_periscope_repair_time = rows.number_or("PeriscopeRepairTime", 30.0f);  // 0083f9b2
    rows.enter("SubAttack");
    out.sub_attack_max_torpedo_range = rows.number("MaxTorpedoRange");  // 0083f6d6
    out.sub_attack_too_close_dist = rows.number("TooCloseDist");  // 0083f718
    out.sub_attack_far_enough_dist = rows.number("FarEnoughDist");  // 0083f75a
    out.sub_attack_submarine_lost_time = rows.number("SubmarineLostTime");  // 0083f79c
    rows.enter("MotherShip");
    out.mother_ship_elevator_speed = rows.number("ElevatorSpeed");  // 0083fb4c
    out.mother_ship_elevator_depth = rows.number("ElevatorDepth");  // 0083fb8e
    rows.enter("Physics");
    out.physics_tboat_nyomatek_szorzo = rows.number_or("TBoatNyomatekSzorzo", 30000.0f);  // 0083fddf
    out.physics_tboat_motor_max_szog = rows.number_or("TBoatMotorMaxSzog", 6.0f);  // 0083fe28
    out.physics_torpedo_force = rows.number_or("TorpedoForce", 1.0f);  // 0083fe7c
    out.physics_torpedo_force_power = rows.number_or("TorpedoForcePower", 2.0f);  // 0083fec8
    rows.enter("DOFParams");
    out.dofparams_dist = rows.number_or("Dist", 1.0f);  // 0083fc08
    out.dofparams_range1 = rows.number_or("Range1", 1.0f);  // 0083fc4a
    out.dofparams_range2 = rows.number_or("Range2", 1.0f);  // 0083fc8f
    out.dofparams_min_amount = rows.number_or("MinAmount", 1.0f);  // 0083fcd7
    out.dofparams_max_amount = rows.number_or("MaxAmount", 0.0f);  // 0083fd1f
    rows.enter("TorpedoHit");
    out.torpedo_hit_life_time = rows.number("LifeTime");  // 0083fa1a
    out.torpedo_hit_upthrust = rows.number("Upthrust");  // 0083fa5f
    out.torpedo_hit_up_dist = rows.number("UpDist");  // 0083faa3
    out.torpedo_hit_mass_ratio = rows.number("MassRatio");  // 0083fae7
    rows.enter("Sounds");
    out.sounds_ship_dead_meat_sound_time_min = rows.number_or("ShipDeadMeatSoundTimeMin", 3.0f);  // 008407ba
    out.sounds_ship_dead_meat_sound_time_max = rows.number_or("ShipDeadMeatSoundTimeMax", 6.0f);  // 00840800
    out.sounds_plane_dead_meat_freq0 = rows.number_or("PlaneDeadMeatFreq0", 0.15f);  // 00840849
    out.sounds_plane_dead_meat_freq1000 = rows.number_or("PlaneDeadMeatFreq1000", 1.0f);  // 00840891
    rows.enter("Physics");
    out.physics_wreck_chance = rows.number_or("WreckChance", 0.0f);  // 0083fd99
    rows.enter("DebrisStruct");
    out.debris_struct_debris_enabled = rows.boolean_or("DebrisEnabled", false);  // 0084124e
    out.debris_struct_debris_num_per_meter = rows.number_or("DebrisNumPerMeter", 0.3f);  // 00841294
    rows.enter("Flag");
    out.flag_need_flag = rows.boolean("NeedFlag");  // 0084091a
    out.flag_wind_power_min = rows.number("WindPowerMin");  // 00840945
    out.flag_wind_power_max = rows.number("WindPowerMax");  // 00840981
    out.flag_wind_power_change = rows.number("WindPowerChange");  // 008409c0
    out.flag_wind_dir_change = rows.number("WindDirChange");  // 00840a86
    out.flag_wind_dir_min = rows.number("WindDirMin");  // 00840a02
    out.flag_wind_dir_max = rows.number("WindDirMax");  // 00840a44
    out.flag_gravity = rows.number("Gravity");  // 00840bd5
    out.flag_drag = rows.number("Drag");  // 00840b93
    out.flag_dt_mul = rows.number("DtMul");  // 00840ac8
    out.flag_dt_step = rows.number("DtStep");  // 00840b0a
    out.flag_num_iteration = static_cast<std::int32_t>(rows.number("NumIteration"));  // 00840b4c
    rows.enter("Navigator", "AutoThrust");
    out.navigator_auto_thrust_steer_value_min_slow = rows.number("SteerValueMin_Slow");  // 0083cba8
    out.navigator_auto_thrust_steer_value_max_slow = rows.number("SteerValueMax_Slow");  // 0083cbea
    out.navigator_auto_thrust_hdg_diff_value_min_slow = rows.number("HdgDiffValueMin_Slow");  // 0083cc2c
    out.navigator_auto_thrust_hdg_diff_value_max_slow = rows.number("HdgDiffValueMax_Slow");  // 0083cc6e
    out.navigator_auto_thrust_thrust_min_slow = rows.number("ThrustMin_Slow");  // 0083ccb0
    out.navigator_auto_thrust_steer_value_min_fast = rows.number("SteerValueMin_Fast");  // 0083ccf2
    out.navigator_auto_thrust_steer_value_max_fast = rows.number("SteerValueMax_Fast");  // 0083cd34
    out.navigator_auto_thrust_hdg_diff_value_min_fast = rows.number("HdgDiffValueMin_Fast");  // 0083cd76
    out.navigator_auto_thrust_hdg_diff_value_max_fast = rows.number("HdgDiffValueMax_Fast");  // 0083cdb8
    out.navigator_auto_thrust_thrust_min_fast = rows.number("ThrustMin_Fast");  // 0083cdfa
    out.navigator_auto_thrust_hdg_diff_danger_mul = rows.number("HdgDiffDangerMul");  // 0083ce3c
    rows.enter("Navigator", "PathFinderParams");
    out.navigator_path_finder_params_length_modifier_dir_diff_min = rows.number_or("LengthModifier_DirDiffMin", 0.261799f);  // 0083d4bf
    out.navigator_path_finder_params_length_modifier_dir_diff_max = rows.number_or("LengthModifier_DirDiffMax", 1.5708f);  // 0083d50b
    out.navigator_path_finder_params_length_modifier_length_addon = rows.number_or("LengthModifier_LengthAddon", 1500.0f);  // 0083d557
    rows.enter("Navigator");
    out.navigator_island_attack_turn_circle_multiplier = rows.number("IslandAttackTurnCircleMultiplier");  // 0083d15a
    out.navigator_flock_predict_time = rows.number("FlockPredictTime");  // 0083d19d
    out.navigator_flock_separation_strength = rows.number("FlockSeparationStrength");  // 0083d1df
    out.navigator_flock_follow_strength = rows.number("FlockFollowStrength");  // 0083d221
    out.navigator_flock_separation_min_dist_mult = rows.number("FlockSeparationMinDistMult");  // 0083d263
    out.navigator_flock_separation_max_dist_mult = rows.number("FlockSeparationMaxDistMult");  // 0083d2a5
    out.navigator_flock_move_treshold = rows.number("FlockMoveTreshold");  // 0083d2e7
    out.navigator_flock_thrust_treshold = rows.number("FlockThrustTreshold");  // 0083d329
    out.navigator_flock_back_steer_speed = rows.number("FlockBackSteerSpeed");  // 0083d36b
    out.navigator_flock_vector_multiplier = rows.number("FlockVectorMultiplier");  // 0083d3ad
    out.navigator_flock_leader_max_hdg = rows.number("FlockLeaderMaxHdg");  // 0083d3ef
    out.navigator_flock_leader_max_side_move = rows.number("FlockLeaderMaxSideMove");  // 0083d431
    rows.enter("VizbeomlesDolgok");
    out.vizbeomles_dolgok_sink_effect_ennyi_meterenkent = rows.number_or("SinkEffectEnnyiMeterenkent", 5.0f);  // 0083ed04
    out.vizbeomles_dolgok_sink_effect_ilyen_gyakran = rows.integer_or("SinkEffectIlyenGyakran", 1000);  // 0083ed45
    rows.enter("Fire");
    out.fire_fire_damage_per_fire_tick = rows.number("FireDamagePerFireTick");  // 008419d8
    rows.enter("AAGunnerErrorModifier");
    out.aagunner_error_modifier_versus_ai = rows.number_or("VersusAI", 2.0f);  // 00841a88
    out.aagunner_error_modifier_versus_player = rows.number_or("VersusPlayer", 1.4f);  // 00841a3f
    out.aagunner_error_modifier_calc_target_pos_time_add_fix = rows.number_or("CalcTargetPosTimeAddFix", 0.0f);  // 00841ad0
    out.aagunner_error_modifier_calc_target_pos_time_add_mul = rows.number_or("CalcTargetPosTimeAddMul", 0.0f);  // 00841b18
    out.aagunner_error_modifier_turn_off_aagun_throw = rows.boolean_or("TurnOffAAGunThrow", false);  // 00841b5c
    rows.enter();
    out.sub_torpedo_delay = rows.number_or("SubTorpedoDelay", 1.5f);  // 00842608
    out.ship_torpedo_delay = rows.number_or("ShipTorpedoDelay", 1.0f);  // 0084264a
}

}  // namespace bsp
