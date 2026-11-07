// GameUnitsHost::report, the units host's end-of-run summary (split out of game_hosts_units.cpp).
#include "bsp/game_hosts_units_internal.hpp"

namespace bsp::game {

void GameUnitsHost::report() {
    Impl& host = *impl_;
    if (host.slots.empty()) return;
    if (bsp::kParatrooperRoundBound || host.para_world.counters().constructs != 0) {
        const bsp::cc13_paratrooper::RoundCounters& c = host.para_world.counters();
        host.log.notef("summary paratrooper rounds bound=%d constructs=%llu construct_draws=%llu "
            "drops=%llu landed=%llu in_water=%llu bad_landings=%llu building_found=%llu "
            "soldiers=%llu soldier_draws=%llu capturer_adds=%llu capturer_removes=%llu "
            "damage_calls=%llu damage_total=%.2f kills=%llu (friendly_building=%llu "
            "no_building=%llu capture_done=%llu bad_landing=%llu water=%llu) live=%zu "
            "steps=%llu (007AB8C0 / 007AB230 / 007AC000 / 007AB360 / 007AB590, packet cc13-para2)",
            bsp::kParatrooperRoundBound ? 1 : 0, c.constructs, c.construct_draws, c.drops,
            c.landed, c.in_water, c.bad_landings, c.building_found, c.soldiers, c.soldier_draws,
            c.capturer_adds, c.capturer_removes, c.damage_calls, c.damage_total, c.kills,
            c.kills_friendly_building, c.kills_no_building, c.kills_capture_done,
            c.kills_bad_landing, c.kills_water, host.para_world.live(), c.steps);
    }
    {
        // Packet cc9_plane_level_bomb_task: one line per aircraft that flew the task.
        int flown = 0;
        for (const auto& sp : host.slots) {
            if (sp && sp->lb_installs > 0) ++flown;
        }
        if (bsp::kPlaneLevelBombTaskBound || flown != 0) {
            host.log.notef("summary plane level bomb task bound=%d aircraft=%d (009B9030 / "
                "009B8B50 / 007C0E67, packet cc9_plane_level_bomb_task)",
                bsp::kPlaneLevelBombTaskBound ? 1 : 0, flown);
            for (const auto& sp : host.slots) {
                if (!sp || sp->lb_installs == 0) continue;
                const GameUnitSlot& u = *sp;
                host.log.notef("  levelbomb %-16s installs=%d ticks=%d mv=%d fo=%d ar=%d aim=%d "
                    "pr=%d rel=%d ga=%d aims=%d aborts=%d breakoffs=%d releases=%d issues=%d "
                    "fired=%d drops=%d gate_refused=%d first_release=%.2f first_drop=%.2f "
                    "min_miss=%.1f state=%s", u.row.name.c_str(), u.lb_installs, u.lb_arm_ticks,
                    u.lb_state_ticks[0], u.lb_state_ticks[1], u.lb_state_ticks[2],
                    u.lb_state_ticks[3], u.lb_state_ticks[4], u.lb_state_ticks[5],
                    u.lb_state_ticks[6], u.lb_aim_entries, u.lb_aim_aborts, u.lb_break_offs,
                    u.lb_releases, u.lb_rack_issues, u.lb_rack_fired, u.lb_rack_drops,
                    u.lb_rack_gate_refused, static_cast<double>(u.lb_first_release_at),
                    static_cast<double>(u.lb_first_drop_at),
                    static_cast<double>(u.lb_min_impact_miss),
                    bsp::level_bomb_state_name(u.level_bomb_state));
                if constexpr (bsp::kLevelBombDropBayGateCc13Bound) {
                    host.log.notef("  levelbomb %-16s bay_refused=%d (007CC8F4, packet cc13-lbbay)",
                        u.row.name.c_str(), u.lb_rack_bay_refused);
                }
                if (u.lb_orders_spent + u.lb_orders_cleared + u.lb_passthrough_releases > 0) {
                    host.log.notef("  levelbomb %-16s orders 0099AF53/009B83D0: spent=%d cleared=%d "
                        "passthrough_releases=%d pending_424=%d", u.row.name.c_str(),
                        u.lb_orders_spent, u.lb_orders_cleared, u.lb_passthrough_releases,
                        u.lb_pending_424);
                }
                if (u.lb_prepare_wait_ticks + u.lb_prepare_drop_ticks + u.lb_prepare_arms > 0) {
                    host.log.notef("  levelbomb %-16s prepare 009B6670: wait=%d drop=%d arms_8100=%d "
                        "releases=%d last_8c=%.3f", u.row.name.c_str(), u.lb_prepare_wait_ticks,
                        u.lb_prepare_drop_ticks, u.lb_prepare_arms, u.lb_prepare_releases,
                        static_cast<double>(u.lb_prepare_8c));
                }
                if (u.lb_issue_tail_calls > 0) {
                    host.log.notef("  levelbomb %-16s issue tail 007C0ED9: calls=%d raised=%d "
                        "gate_refused=%d c25=%d c58=%d", u.row.name.c_str(),
                        u.lb_issue_tail_calls, u.lb_issue_tail_raised,
                        u.lb_issue_tail_gate_refused, u.torpedo_release_pending_c25 ? 1 : 0,
                        u.torpedo_release_orders_c58);
                }
            }
        }
    }
    host.log.notef("summary aim lead body axis census bound=%d leads=%llu differs=%llu "
        "(009FADA0 tail / 00812090, ON vs OFF lead > 1e-3 m)",
        bsp::kAimLeadBodyAxisCc13Bound ? 1 : 0, host.aim_lead_axis_calls_,
        host.aim_lead_axis_differs_);
    {
        // Packet cc13-aimfreeze3 (include/bsp/approach_aim_freeze_cc13.hpp), ON only.
        // Tasks 4..6: packet cc13-aimfreezekz (include/bsp/approach_ref_freeze_planes_cc13.hpp);
        // their early_outs are the orphaned-target ticks answered with the frozen point.
        static const char* const kFreezeTasks[7] = {"", "dive", "level_bomb", "torpedo",
                                                    "kamikaze", "rocket", "depth_charge"};
        const bool freeze_on[7] = {false, bsp::kDiveAimPointFreezeCc13Bound,
                                   bsp::kLevelBombAimPointFreezeCc13Bound,
                                   bsp::kTorpedoAimPointFreezeCc13Bound,
                                   bsp::kApproachRefFreezePlanesCc13Bound,
                                   bsp::kApproachRefFreezePlanesCc13Bound,
                                   bsp::kApproachRefFreezePlanesCc13Bound};
        for (int k = 1; k < 7; ++k) {
            if (!freeze_on[k]) continue;
            host.log.notef("summary aim freeze %s heads=%llu doubles=%llu frozen_reads=%llu "
                "early_outs=%llu (009FADAE death head / 009FAF90 notify, LABELLED; "
                "packet cc13-aimfreeze3)", kFreezeTasks[k], host.aim_freeze_heads_[k],
                host.aim_freeze_doubles_[k], host.aim_freeze_reads_[k],
                host.aim_freeze_early_outs_[k]);
        }
    }
    {
        const bsp::TorpedoGyroHeadingCensus& tg = bsp::torpedo_gyro_heading_census();
        host.log.notef("summary torpedo gyro heading census bound=%d track_reads=%llu "
            "track_differs=%llu threat_reads=%llu threat_differs=%llu (009DC0F9 / 00817535, "
            "+46Ch vs atan2(v) > 1e-3 rad)", bsp::kTorpedoCandidateGyroHeadingCc13Bound ? 1 : 0,
            tg.track_reads, tg.track_differs, tg.threat_reads, tg.threat_differs);
    }
    host.log.notef("summary rocket target stop census bound=%d installs=%llu not_reset=%llu "
        "(0099C79C 009FCEF0 reset at task install)",
        bsp::kRocketTargetStopHandlersCc13Bound ? 1 : 0, host.rocket_stop_installs_,
        host.rocket_stop_not_reset_);
    host.log.notef("summary ship exit zone bound=%d calls=%llu counting=%llu warnings=%llu "
        "kills=%llu (00826D70 / 00811F50 / 0081BEB0, packet cc9_ship_exit_zone)",
        GameUnitsHost::Impl::kShipOutOfMapExitBound ? 1 : 0, host.ship_exit_calls,
        host.ship_exit_counting, host.ship_exit_warnings, host.ship_exit_kills);
    if constexpr (bsp::kLeaveCountdownAllSquadronsCc13Bound) {
        host.log.notef("summary plane leave countdown calls=%llu counting=%llu "
            "counting_not_retreat=%llu (00953D98 -> 007C6C30, packet cc13-leavecd)",
            host.leave_countdown_calls, host.leave_counting_steps,
            host.leave_counting_not_retreat);
    }
    if constexpr (kSquadronReturnToBaseResolveBound) {
        host.log.notef("summary plane retreat task bound=%d orders=%llu installs=%llu "
            "rtb_installs=%llu refused=%llu retired=%llu exits=%llu exit_members=%llu "
            "exit_gated=%llu latches_361=%llu (009CA2B0 / 007C6C30 / 007F31A0, packet "
            "cc9_plane_retreat_task)", bsp::kPlaneRetreatTaskBound ? 1 : 0,
            host.retreat_orders, host.retreat_installs, host.retreat_rtb_installs,
            host.retreat_refused, host.retreat_retired, host.retreat_exits,
            host.retreat_exit_members, host.retreat_exit_gated, host.retreat_latches_361);
        if (bsp::kSquadronSpentOrdnanceRtbBound || host.spent_rtb_latches != 0) {
            host.log.notef("summary squadron spent ordnance rtb bound=%d latches=%llu ends=%llu "
                "issues=%llu (009F8160 / 009F7C90 / 0084E010, packet "
                "cc9_squadron_spent_ordnance_rtb)", bsp::kSquadronSpentOrdnanceRtbBound ? 1 : 0,
                host.spent_rtb_latches, host.spent_rtb_ends, host.spent_rtb_issues);
            host.log.notef("summary rack device reload bound=%d reloads=%llu flag=%d "
                "(006E57A4, packet cc9_rack_device_reload)",
                GameUnitsHost::Impl::kRackDeviceReloadBound ? 1 : 0, host.rack_device_reloads,
                lua_device_reload_enabled_00e17bf2() ? 1 : 0);
            host.log.notef("summary dkz spent rtb bound=%d torpedo_refused_class=%llu (0099A170 / 009AE1B0, "
                "packet cc9_dkz_spent_rtb)", GameUnitsHost::Impl::kDropKamikazeSpentRtbBound ? 1 : 0,
                host.torpedo_install_refused_class);
            host.log.notef("summary mission squadron retest bound=%d dead_targets=%llu ends=%llu "
                "(009F8160 / 009F7C90, cc9-gunnery38)", bsp::kSquadronDeadTargetRetestBound ? 1 : 0,
                host.retest_dead_targets, host.retest_ends);
            host.log.notef("summary attack task target retire bound=%d retires=%llu (009C8060 / "
                "009D3EF0 / 009AE140, packet cc9_l64_attack_task_retire)",
                GameUnitsHost::Impl::kAttackTaskTargetRetireBound ? 1 : 0, host.attack_task_target_retires);
            host.log.notef("summary plane touchdown human pilot census=%llu (007CB778 -> 007C3EA0, "
                "not carried)", host.touchdown_human_pilot);
            for (const auto& e : host.spent_rtb) {
                if (!e.second.latch_3c) continue;
                host.log.notef("  spent %-24s command=%08X latched=%.2f ended=%.2f issued=%.2f "
                    "placed=%zu", e.first.c_str(), e.second.command,
                    static_cast<double>(e.second.latch_at), static_cast<double>(e.second.ended_at),
                    static_cast<double>(e.second.issued_at), e.second.placed);
            }
        }
        for (const auto& slot : host.slots) {
            if (!slot || slot->retreat_ticks == 0) continue;
            host.log.notef("  retreat %-16s ticks=%llu moveto=%llu enterzone=%llu leave=%llu "
                "follow=%llu refreshes=%llu zone_misses=%llu outside=%llu installed_at=%.2f "
                "outside_at=%.2f", slot->row.name.c_str(), slot->retreat_ticks,
                slot->retreat_moveto_ticks, slot->retreat_enter_ticks, slot->retreat_leave_ticks,
                slot->retreat_follow_ticks, slot->retreat_refreshes, slot->retreat_zone_misses,
                slot->retreat_outside_ticks, static_cast<double>(slot->retreat_installed_at),
                static_cast<double>(slot->retreat_outside_at));
        }
        host.log.notef("summary squadron returntobase 007F16D0 null=%llu home=%llu site=%llu "
            "retreat=%llu squadrons=%zu RECORD ONLY (packet cc9_squadron_land_task)",
            host.rtb_arm_counts[0], host.rtb_arm_counts[1], host.rtb_arm_counts[2],
            host.rtb_arm_counts[3], host.rtb_census.size());
        host.log.notef("summary moveto arrival end command calls=%llu no_squadron=%llu "
            "not_moveto=%llu commands_ended=%llu stage_only=%llu tasks_retired=%llu "
            "next_promoted=%llu bound=%d (009C3100, packet cc9_moveto_arrival_end_command)",
            host.moveto_end_calls, host.moveto_end_no_squadron, host.moveto_end_not_moveto,
            host.moveto_end_commands, host.moveto_end_stage_only, host.moveto_end_retired,
            host.moveto_end_promoted,
            Impl::kMoveToArrivalEndCommandBound ? 1 : 0);
        host.log.notef("summary plane wanderer planes=%llu steps=%llu active=%llu timer_draws=%llu "
            "off_transitions=%llu offset_max=%.3f speed_max=%.3f accel_max=%.3f displacement=%.1f "
            "bound=%d (007BE060 / 007D8230, packet cc9_plane_wanderer)", host.wanderer_planes,
            host.wanderer_steps_total, host.wanderer_active_total, host.wanderer_timer_draws,
            host.wanderer_off_transitions, static_cast<double>(host.wanderer_offset_max),
            static_cast<double>(host.wanderer_speed_max_seen),
            static_cast<double>(host.wanderer_accel_max_seen), host.wanderer_displacement_total,
            Impl::kPlaneWandererBound ? 1 : 0);
        host.log.notef("summary follow station error n=%llu mean=%.2f max=%.2f (009BFD70 distance, "
            "packet cc9_plane_wanderer diagnostic)", host.follow_station_err_n,
            host.follow_station_err_n ? host.follow_station_err_sum / host.follow_station_err_n : 0.0,
            static_cast<double>(host.follow_station_err_max));
        {
            const double n = host.fes_n ? static_cast<double>(host.fes_n) : 1.0;
            host.log.notef("summary follow error split n=%llu along_mean=%.1f cross_abs_mean=%.1f "
                "dy_abs_mean=%.1f behind=%llu ahead=%llu beside=%llu speed_diff_mean=%.2f "
                "behind_speed_diff_mean=%.2f flyto_cmd_minus_own_mean=%.2f (leader frame, "
                "packet cc9_follow_error_split diagnostic)", host.fes_n,
                host.fes_along_sum / n, host.fes_cross_abs_sum / n, host.fes_dy_abs_sum / n,
                host.fes_behind, host.fes_ahead, host.fes_beside, host.fes_speed_diff_sum / n,
                host.fes_behind ? host.fes_behind_speed_diff_sum / static_cast<double>(host.fes_behind) : 0.0,
                host.fes_flyto_n ? host.fes_flyto_cmd_minus_own_sum /
                    static_cast<double>(host.fes_flyto_n) : 0.0);
            {
                for (int k = 0; k < 2; ++k) {
                    if (host.fct_task_n[k] == 0) continue;
                    const double d = static_cast<double>(host.fct_task_n[k]);
                    host.log.notef("summary follow cross track task=%s n=%llu err_mean=%.1f "
                        "along_mean=%.1f cross_abs_mean=%.1f own_v_mean=%.1f own_v_below_40=%llu "
                        "(packet cc9_follow_cross_track diagnostic)", k ? "land" : "other",
                        host.fct_task_n[k], host.fct_task_err[k] / d, host.fct_task_along[k] / d,
                        host.fct_task_cross_abs[k] / d, host.fct_task_speed[k] / d,
                        host.fct_task_slow[k]);
                }
                static const char* const kTurn[2] = {"straight", "turning"};
                static const char* const kBranch[4] = {"hold", "lead", "abeam", "circle"};
                for (int t = 0; t < 2; ++t) {
                    for (int b = 0; b < 4; ++b) {
                        const unsigned long long c = host.fct_n[t][b];
                        if (c == 0) continue;
                        const double d = static_cast<double>(c);
                        host.log.notef("summary follow cross track %s/%s n=%llu err_mean=%.1f "
                            "along_mean=%.1f cross_abs_mean=%.1f cross_turnsigned_mean=%.1f beside=%llu "
                            "(packet cc9_follow_cross_track diagnostic)", kTurn[t], kBranch[b], c,
                            host.fct_err[t][b] / d, host.fct_along[t][b] / d,
                            host.fct_cross_abs[t][b] / d, host.fct_cross_turn[t][b] / d,
                            host.fct_beside[t][b]);
                    }
                }
            }
            host.log.notef("summary plane dead meat roll steps=%llu spin=%llu bound=%d wreck drag "
                "steps=%llu bound=%d (007DA7E4 R4, 007DBC77 C1, packet cc9_plane_wreck_roll)",
                host.dead_meat_roll_steps, host.dead_meat_spin_steps,
                Impl::kPlaneDeadMeatRollBound ? 1 : 0, host.wreck_drag_steps,
                Impl::kPlaneWreckForwardDragBound ? 1 : 0);
            host.log.notef("summary plane velocity lowpass steps=%llu c84_reads=%llu "
                "corrections=%llu mean_abs_correction=%.5f bound=%d (007DC743, 007C18B0, 0099DAC8, "
                "packet cc9_plane_velocity_lowpass)", host.velocity_lowpass_steps,
                host.velocity_lowpass_c84_reads, host.velocity_lowpass_corrections,
                host.velocity_lowpass_corrections
                    ? host.velocity_lowpass_correction_abs
                        / static_cast<double>(host.velocity_lowpass_corrections) : 0.0,
                Impl::kPlaneVelocityLowpassBound ? 1 : 0);
            host.log.notef("summary planner laws mode1_tuned=%llu bound=%d speed_corr raises=%llu "
                "above_one_thinks=%llu bound=%d forward_speed bound=%d bomb_load scaled=%llu "
                "bound=%d (0099E26E, 0099D756/0099D924, 0099D99E, 007C0F40, packet "
                "p16_planner_laws)", host.planner_mode_one_tuned,
                Impl::kPilotModeOneRollTuningBound ? 1 : 0, host.speed_corr_raises,
                host.speed_corr_above_one_thinks, Impl::kPilotSpeedCorrDivisorBound ? 1 : 0,
                Impl::kPilotThrottleForwardSpeedBound ? 1 : 0, host.bomb_load_authority_scaled,
                Impl::kPlaneBombLoadAuthorityBound ? 1 : 0);
            host.log.notef("summary carrier landing leads=%llu bound=%d touchdown_brakes=%llu "
                "bound=%d (009AFAF0, 009B201B, packet p16_carrier_landing)",
                host.carrier_landing_leads, Impl::kCarrierLandingVelocityLeadBound ? 1 : 0,
                host.carrier_touchdown_brakes, Impl::kCarrierTouchdownBrakeBound ? 1 : 0);
            host.log.notef("summary forward speed deck terms=%llu bound=%d lead_float_ops=%d "
                "(007D99C0, 006BCC0A, packets p19_fwd_speed / p19_lead_ops)",
                host.forward_speed_deck_terms, Impl::kPlaneForwardSpeedDeckTermBound ? 1 : 0,
                Impl::kCarrierLandingLeadFloatOpsBound ? 1 : 0);
            host.log.notef("summary bay request disabled channel C=%llu bay_door_classes=%llu "
                "no_force_bound=%d (007BBBA6, 007EABC0, packet p19_bay_force)",
                host.bay_channel_disabled_requests, host.bay_door_classes,
                Impl::kBayRequestNoForceEnableBound ? 1 : 0);
            if constexpr (bsp::cc13_release_timer::kReleaseBayOpenCc13Bound) {
                host.log.notef("summary bay opener steps=%llu target_writes=%llu "
                    "(0099B055, 007C6A8D, packet cc13-reltimer2)",
                    host.bay_open_steps, host.bay_open_target_writes);
            }
            if constexpr (bsp::kLevelBombBayOpenerCc13Bound) {
                host.log.notef("summary level-bomb bay opener steps=%llu mode2=%llu "
                    "shadowed_by_torpedo_or_dive=%llu (009B8450, packet cc13-lbbay)",
                    host.lb_bay_opener_steps, host.lb_bay_opener_mode2,
                    host.lb_bay_opener_shadowed);
            }
            host.log.notef("summary moveto cruise by_ordnance=%llu bound=%d leg separations=%llu "
                "bound=%d (009C3650, 009BEC85, packet p16_moveto_cruise)",
                host.moveto_cruise_by_ordnance, Impl::kMoveToCruiseByOrdnanceBound ? 1 : 0,
                host.moveto_leg_separations, Impl::kMoveToLegAltitudeBound ? 1 : 0);
            host.log.notef("summary follow turbo steps=%llu bound=%d (009BEE42 -> 007D9062, "
                "packet cc9_follow_turbo)", host.follow_turbo_steps,
                Impl::kFollowTurboBound ? 1 : 0);
            host.log.notef("summary follow speed ceiling n=%llu member_v=%.2f leader_v=%.2f "
                "member_maxspd=%.2f leader_maxspd=%.2f leader_want=%.2f thr_member=%.3f "
                "thr_leader=%.3f member_full=%llu leader_full=%llu (throttle slot current, "
                "packet cc9_follow_speed_loss diagnostic)", host.fes_n,
                host.fes_member_v_sum / n, host.fes_leader_v_sum / n, host.fes_member_max_sum / n,
                host.fes_leader_max_sum / n, host.fes_leader_want_sum / n, host.fes_thr_member_sum / n,
                host.fes_thr_leader_sum / n, host.fes_member_full, host.fes_leader_full);
        }
        if (host.mvs_n != 0) {
            host.log.notef("summary torpedo moveto sep n=%llu mean_dsep=%.2f max_dsep=%.2f "
                "speed_changed=%llu max_dspeed=%.4f old_range_not_positive=%llu (009C18EC origin "
                "sep against approach+90h, packet cc13-mvspeed diagnostic)", host.mvs_n,
                host.mvs_dsep_sum / static_cast<double>(host.mvs_n), host.mvs_dsep_max,
                host.mvs_speed_changed, host.mvs_dspeed_max, host.mvs_no_range);
        }
        if (host.lhs_n != 0) {
            const double n = static_cast<double>(host.lhs_n);
            host.log.notef("summary land moveto speed n=%llu a=%.2f b=%.2f want=%.2f wingmen=%.3f "
                "v=%.2f sep=%.1f at_floor=%llu wingmen_below_1=%llu sep_below_wait1=%llu "
                "sep_below_300=%llu v_below_40=%llu v_below_40_at_floor=%llu (009C1850 in "
                "moveto (land), packet cc9_land_holding_speed diagnostic)", host.lhs_n,
                host.lhs_a_sum / n, host.lhs_b_sum / n, host.lhs_want_sum / n,
                host.lhs_wingmen_sum / n, host.lhs_v_sum / n, host.lhs_sep_sum / n,
                host.lhs_floor, host.lhs_wait, host.lhs_near, host.lhs_sep300, host.lhs_slow,
                host.lhs_slow_floor);
        }
        host.log.notef("summary follow phase-a lead=%llu abeam=%llu circle=%llu mean_time=%.3f "
            "applied=%d (009C0251-009C0EE0, packet cc9_follow_approach_arm; a shadow when OFF)",
            host.follow_phase_a_lead, host.follow_phase_a_abeam, host.follow_phase_a_circle,
            (host.follow_phase_a_lead + host.follow_phase_a_abeam + host.follow_phase_a_circle)
                ? host.follow_phase_a_time_sum / static_cast<double>(host.follow_phase_a_lead +
                      host.follow_phase_a_abeam + host.follow_phase_a_circle) : 0.0,
            Impl::kFollowPhaseABound ? 1 : 0);
        host.log.notef("summary follow trail arm calls=%llu raised=%llu enabled=%llu disabled=%llu "
            "min_mul=%.3f bound=%d (009C207C, packet cc9_follow_trail_arm)", host.trail_arm_calls,
            host.trail_arm_raised, host.trail_arm_enabled, host.trail_arm_disabled,
            static_cast<double>(host.trail_arm_min_mul), Impl::kFollowTrailArmBound ? 1 : 0);
        host.log.notef("summary dogfight avoid timer redraws=%llu bound=%d | early edge clears=%llu "
            "bound=%d (009A7D60 / 009AAC63, packets p20_df_avoid_timer / p20_df_early_clear)",
            host.df_avoid_timer_redraws, Impl::kDogfightAvoidTimerDrawBound ? 1 : 0,
            host.df_early_edge_clears, Impl::kDogfightEarlyEdgeClearBound ? 1 : 0);
        host.log.notef("summary follow exit 009BDE40 calls=%llu bound=%d (packet p20_follow_exit)",
            host.follow_exits, Impl::kFollowExitClearsBound ? 1 : 0);
        host.log.notef("summary attack mode cadence fires=%llu skips=%llu bound=%d (009998A0 "
            "+308h gate on 0099B740, packet p20_cadence)", host.attack_mode_cadence_fires,
            host.attack_mode_cadence_skips, Impl::kTaskAttackModeCadenceBound ? 1 : 0);
        host.log.notef("summary bot task ctor seeds=%llu bound=%d (0099C6F0 / 009C2980, packet "
            "p20_ctor_seeds)", host.bot_task_ctor_seeds, Impl::kBotTaskCtorSeedsBound ? 1 : 0);
        host.log.notef("summary task gun block resets=%llu bound=%d burst=%d (009FAAD0 per "
            "install, packet cc13-seg67n3)", host.task_gun_block_resets,
            bsp::kTaskGunBlockReconstructBound ? 1 : 0,
            bsp::kTaskGunBlockBurstTimerResetBound ? 1 : 0);
        if (host.strafe_draw_count_takes > 0) {
            host.log.notef("summary strafe draw-count takes=%llu (009CB116 / 0099C6F0 pushes, "
                "packet p22_strafe_draws)", host.strafe_draw_count_takes);
        }
        host.log.notef("summary land min speed owner terms=%llu bound=%d (009B1362, packet "
            "p20_land_min_speed_owner)", host.land_min_speed_owner_terms,
            Impl::kLandMinSpeedOwnerVelocityBound ? 1 : 0);
        host.log.notef("summary follow release arm pending=%llu applied=%llu bound=%d "
            "(009C1FFD-009C2063, packet p20_follow_release_arm)", host.follow_release_pending_ticks,
            host.follow_release_arm_applied, Impl::kFollowReleaseArmBound ? 1 : 0);
        host.log.notef("summary moveto target speed override calls=%llu target=%llu within=%llu "
            "applied=%llu max_speed=%.2f bound=%d (009C23B0, packet cc9_moveto_target_speed)",
            host.ts_override_calls, host.ts_override_target, host.ts_override_within,
            host.ts_override_applied, static_cast<double>(host.ts_override_max_speed),
            Impl::kMoveToTargetSpeedOverrideBound ? 1 : 0);
        host.log.notef("summary moveto target speed class census calls=%llu speed_differs=%llu "
            "answer_differs=%llu bound=%d (009C23DB, kMovetoTargetSpeedClassCc13Bound, cc9-physics29)",
            host.tsc_calls, host.tsc_speed_differs, host.tsc_answer_differs,
            Impl::kMovetoTargetSpeedClassCc13Bound ? 1 : 0);
        host.log.notef("summary carrier landing decks built=%llu refreshes=%llu bound=%d "
            "(007593D0 / 006BEE40 / 006BA620, packet cc9_carrier_landing_deck)",
            host.carrier_decks_built, host.carrier_deck_refreshes,
            Impl::kCarrierLandingDeckBound ? 1 : 0);
        host.log.notef("summary carrier deck parent parented=%llu wire_seeded=%llu carry_steps=%llu "
            "wire_steps=%llu wire_resets=%llu wire_max=%.2f stops=%llu edge_takeoff_requests=%llu "
            "bound=%d (007C71E0 / 007DB630 / 007DBEEE / 007DC0A2 / 007CC264, packet "
            "cc9_carrier_landing_deck_part2)", host.carrier_deck_parented,
            host.carrier_deck_wire_seeded, host.carrier_deck_carry_steps,
            host.carrier_deck_wire_steps, host.carrier_deck_wire_resets,
            static_cast<double>(host.carrier_deck_wire_max), host.carrier_deck_stops,
            host.carrier_deck_edge_takeoffs, Impl::kCarrierDeckParentBound ? 1 : 0);
        host.log.notef("summary squadron returntobase site keys computed=%llu carrier=%llu "
            "missing=%llu bound=%d (006C09FE, packet cc9_rtb_site_key)", host.rtb_site_keys,
            host.rtb_site_keys_carrier, host.rtb_site_keys_missing,
            Impl::kReturnToBaseSiteKeyBound ? 1 : 0);
    }
    host.log.notef("summary unit yaw rate 00811940 bound=%d calls=%llu nonzero=%llu "
        "(packet cc9_unit_yaw_rate_forward_speed)", kUnitYawRateForwardSpeedBound ? 1 : 0,
        host.yaw_rate_calls_00811940, host.yaw_rate_nonzero_00811940);
    if constexpr (!bsp::kUnitsHostDeviationsCc13Bound) {
        host.log.notef("summary units host deviations census (OFF, cc9-physics29): moveto "
            "calls=%llu differs=%llu | pass-1 pitch calls=%llu differs=%llu lowspeed=%llu "
            "maxdiff=%.6f rad (kUnitsHostDeviationsCc13Bound)", host.uhd_moveto_calls,
            host.uhd_moveto_differs, host.uhd_pitch_calls, host.uhd_pitch_differs,
            host.uhd_pitch_lowspeed, host.uhd_pitch_maxdiff);
    }
    if constexpr (kSquadronLandTaskBound) {
        unsigned long long installs = 0, kept = 0, refused = 0, empty = 0, reqs = 0, refused_states = 0;
        for (const Impl::LandTaskCensus& c : host.land_census) {
            installs += c.installs;
            kept += c.kept;
            refused += c.refused_arm + c.refused_unread + c.refused_ground + c.refused_dead;
            host.log.notef("summary land task squadron \"%s\": placements=%llu installs=%llu "
                "kept=%llu refused arm=%llu unread=%llu ground=%llu dead=%llu first_install=%.2f s "
                "(packet cc9_land_task_reach)", c.squadron.c_str(), c.placements, c.installs, c.kept,
                c.refused_arm, c.refused_unread, c.refused_ground, c.refused_dead,
                c.first_install_at);
            if (c.ground_parked > 0) {
                host.log.notef("summary land task squadron \"%s\": ground_parked=%llu "
                    "(started in land/park, kLandTaskGroundParkStartBound)", c.squadron.c_str(),
                    c.ground_parked);
            }
        }
        for (const std::unique_ptr<GameUnitSlot>& s : host.slots) {
            if (s->land_installed_at < 0.0) continue;
            empty += s->land_empty_assignments;
            reqs += s->land_requests;
            refused_states += s->land_refused_states;
            host.log.notef("summary land task plane \"%s\": installed=%.2f s now=%d state=%X "
                "ticks=%llu moveto=%llu follow=%llu no_station=%llu changes=%llu requests=%llu "
                "empty_assignments=%llu refused_states=%llu d first=%.1f min=%.1f at %.2f s last=%.1f "
                "alt=%.1f dead=%d", s->row.name.c_str(), s->land_installed_at,
                s->land_task_installed ? 1 : 0, static_cast<unsigned>(s->land_state),
                s->land_ticks, s->land_moveto_ticks, s->land_follow_ticks,
                s->land_follow_no_station, s->land_state_changes, s->land_requests,
                s->land_empty_assignments, s->land_refused_states,
                static_cast<double>(s->land_first_distance), static_cast<double>(s->land_min_distance),
                s->land_min_distance_at, static_cast<double>(s->land_last_distance),
                static_cast<double>(s->motion.position[1]), s->plane_death_c3a ? 1 : 0);
        }
        host.log.notef("summary land task installs=%llu kept=%llu refused=%llu requests=%llu "
            "empty_assignments=%llu refused_states=%llu retired_invalid=%llu profile calls=%llu "
            "writes=%llu (packet cc9_land_task_reach)", installs, kept, refused, reqs, empty,
            refused_states, host.land_retired_invalid, host.land_profile_calls,
            host.land_profile_writes);
    }
    if (host.lhc_n != 0) {
        const double n = static_cast<double>(host.lhc_n);
        host.log.notef("summary land head circle n=%llu on=%llu far=%llu heading_fail=%llu "
            "heading_fail_delta=%.3f side_moved=%llu own_heading_ok=%llu own_far=%llu dd_over_r=%.3f "
            "own_d_over_r=%.3f r_moved=%.1f point_moved=%.1f leaves=%llu (006C5C40 at the leader "
            "pass for heads in land/standby, packet cc9_land_head_chatter diagnostic)", host.lhc_n,
            host.lhc_on, host.lhc_far, host.lhc_heading,
            host.lhc_heading ? host.lhc_delta_fail_sum / static_cast<double>(host.lhc_heading) : 0.0,
            host.lhc_side_moved, host.lhc_own_ok, host.lhc_own_far, host.lhc_dd_r_sum / n,
            host.lhc_own_d_r_sum / n, host.lhc_r_moved_sum / n, host.lhc_point_moved_sum / n,
            host.lhc_leave);
    }
    if constexpr (Impl::kCarrierElevatorBound) {
        for (const Impl::LandingDeck& d : host.landing_decks) {
            const Impl::CarrierElevator& e = d.elevator;
            if (!d.mother_ship || !e.built || d.owner >= host.slots.size()) continue;
            host.log.notef("summary carrier elevator %s: refused=%d ticks=%llu intakes=%llu "
                "stowed=%llu releases=%llu empty_up=%llu unfed_relaunch=%llu top_releases=%llu "
                "mode=%d p18=%.2f state_two=%llu (006D0600 / 006FC480 / 006FC720, packet "
                "cc9_carrier_elevator)", host.slots[d.owner]->row.name.c_str(),
                e.refused ? 1 : 0, e.ticks, e.intakes, e.hides, e.releases, e.empty_up,
                e.unfed_relaunch, e.top_releases, e.mode_50, static_cast<double>(e.p18),
                host.carrier_elevator_state_two);
        }
    }
    host.log.notef("summary stowed plane stock return bound=%d sends=%llu adds=%llu no_deck=%llu "
        "no_squadron=%llu slot_returns=%llu site_leaves=%llu (007CE6CA -> 007CC8B0 -> 007F1CA0, "
        "packet cc9_stowed_plane_stock_return)", Impl::kStowedPlaneStockReturnBound ? 1 : 0,
        host.stock_return_sends, host.stock_return_adds, host.stock_return_no_deck,
        host.stock_return_no_squadron, host.stock_return_slot_returns,
        host.stock_return_site_leaves);
    host.log.notef("summary plane disappear C7h copies bound=%d copies=%llu index_holder=%llu "
        "(00762B20 / 00762BE0 / 00762C30, goal 4 chain 356)",
        1, host.stock_return_c7h_copies,
        host.stock_return_c7h_index_holder);
    host.log.notef("summary free flight stall force bound=%d steps=%llu kicks=%llu (007DC830 -> "
        "007DAFD0, packet cc9_p14_stall_force)", Impl::kFreeFlightStallForceBound ? 1 : 0,
        host.stall_force_steps, host.stall_force_kicks);
    host.log.notef("summary free flight dying spin bound=%d steps=%llu seeds=%llu wreck_lift_bound=%d "
        "wreck_lift_kicks=%llu (007DC830 007DCAF9 / 007DCB4B, packet p15_dying_spin)",
        Impl::kFreeFlightDyingSpinBound ? 1 : 0, host.dying_spin_steps, host.dying_spin_seeds,
        Impl::kFreeFlightWreckLiftBound ? 1 : 0, host.wreck_lift_kicks);
    host.log.notef("summary squadron free attack bound=%d census=%d ticks=%llu stops mode=%llu gate=%llu " 
        "no_target=%llu refused=%llu same=%llu not_air=%llu ordnance=%llu override=%llu sends=%llu installed=%llu " 
        "tasks=%llu stage_raises=%llu cleared=%llu restored=%llu moveto_self=%llu (009F8470, packet " 
        "p15_free_attack_tick)", Impl::kSquadronFreeAttackTickBound ? 1 : 0, free_attack_census_enabled() ? 1 : 0, 
        host.free_attack_ticks, host.free_attack_stops[0], host.free_attack_stops[1], host.free_attack_stops[2], 
        host.free_attack_stops[3], host.free_attack_stops[4], host.free_attack_stops[5], host.free_attack_stops[6], 
        host.free_attack_stops[7], host.free_attack_sends, host.free_attack_installed, host.free_attack_tasks, 
        host.free_attack_stage_raises, host.free_attack_cleared, host.free_attack_restored, host.free_attack_moveto_self);
    host.log.notef("summary rack positive bag round bound=%d rounds=%llu (006E3350 -> 006E3C00, "
        "packet cc9_p14_bag_rack_round)", Impl::kRackPositiveBagRoundBound ? 1 : 0,
        host.rack_positive_bag_rounds);
    host.log.notef("summary stock return destroy bytes bound=%d destroys=%llu (00926D90 -> "
        "0077D1A0 -> 00926C80, packet cc9_p14_stock_return_destroy)",
        Impl::kStockReturnDestroyBytesBound ? 1 : 0, host.stock_return_destroys);
    host.log.notef("summary plane land chain notify bound=%d notifies=%llu draw bound=%d "
        "destroy_draws=%llu (007C11E0 at 007C1680 / 007CC7A0 / 007CC820 / 007CB5F0 / "
        "007C5F60 / 007C1430; 007BCABB, packet cc13-landc)",
        bsp::kPlaneFlightStateNotifyCc13Bound ? 1 : 0, host.flight_state_notifies,
        bsp::kPlaneRetireOnDestroyedDrawCc13Bound ? 1 : 0, host.stock_return_destroy_draws);
    host.log.notef("summary plane combat death draw bound=%d draws=%llu (007BCABB after "
        "the death flush, packet cc13-landc)",
        bsp::kPlaneCombatDeathRetireDrawCc13Bound ? 1 : 0, host.combat_death_destroy_draws);
    host.log.notef("summary plane engines lists=%llu engines=%llu stop_alls=%llu "
        "fires_raised=%llu destroy_calls=%llu (007EC5A0 list; 007CAF10 engine block "
        "007DE600 / 007DE5B0 / \"enginefire\"; 007D0B80 -> 007EB0B0, goal 5 chain 654)",
        host.plane_engine_lists_built, host.plane_engines_built, host.plane_engine_stop_alls,
        host.plane_engine_fires_raised, host.plane_engines_destroyed);
    host.log.notef("summary plane ground state dead tail bound=%d skips=%llu "
        "(007CA50F, packet cc13-landc)",
        bsp::kPlaneGroundStateDeadTailCc13Bound ? 1 : 0, host.ground_state_dead_tail_skips);
    host.log.notef("summary plane funnel kill destroy only bound=%d planes=%llu (00926D90 -> "
        "0077D1A0, no 007CA8A0, packet cc13-landc)",
        bsp::kPlaneFunnelKillDestroyOnlyCc13Bound ? 1 : 0, host.funnel_kill_destroy_only);
    host.log.notef("summary stock return queue erase bound=%d erases=%llu misses=%llu "
        "(006C59A4 -> 006BF7F0, packet cc9_stock_return_queue_erase)",
        Impl::kStockReturnQueueEraseBound ? 1 : 0, host.stock_return_queue_erases,
        host.stock_return_queue_misses);
    host.log.notef("summary land task retire queue erase bound=%d leader_calls=%llu "
        "queue_erases=%llu assignment_erases=%llu slot_states=%llu (009B3FB3 -> 006C8800 -> "
        "006C7680, packet cc9_land_task_retire_queue_erase)",
        Impl::kLandTaskRetireQueueEraseBound ? 1 : 0, host.land_retire_leader_calls,
        host.land_retire_queue_erases, host.land_retire_assignment_erases,
        host.land_retire_slot_states);
    host.log.notef("summary land command own target bound=%d retired_invalid=%llu "
        "own_target_ok=%llu (009B34D0 vtable[178h], packet cc9_land_command_own_target)",
        Impl::kLandCommandOwnTargetBound ? 1 : 0, host.land_retired_invalid,
        host.land_retired_own_target_ok);
    host.log.notef("summary squadron kill queue erase bound=%d kills=%llu erases=%llu "
        "(00926D90 -> 009273A0 slot+4 -> 006C4B30, packet cc9_squadron_kill_queue_erase)",
        Impl::kSquadronKillQueueEraseBound ? 1 : 0, host.squadron_kill_queue_kills,
        host.squadron_kill_queue_erases);
    if constexpr (kBaseLaunchChainBound) {
        std::size_t inside_now = 0;
        for (const auto& s : host.slots) {
            if (s && s->plane_control_mode_900 == 1) ++inside_now;
        }
        host.log.notef("summary base launch inside start: squadrons=%llu entries=%llu "
            "no_base=%llu inside_now=%zu (007F1C00 / 007ED6E0 / 007CC820, packet "
            "cc9_base_launch_inside_start)", host.base_launch_inside_squadrons,
            host.base_launch_inside_entries, host.base_launch_inside_no_base, inside_now);
        host.log.notef("summary base launch task: built=%llu ended=%llu ticks=%llu "
            "not_ready=%llu sends=%llu no_deck=%llu (007F1DE0 / 007F1F00 / 007EF010 / "
            "006CF190, packet cc9_base_launch_task)", host.base_launch_tasks_built,
            host.base_launch_tasks_ended, host.base_launch_ticks, host.base_launch_not_ready,
            host.base_launch_sends, host.base_launch_no_deck);
        host.log.notef("summary base launch deck arms: placed=%llu refused=%llu "
            "ground_entries=%llu (006CF9F0 / 007C5F60 / 007CA3F0, packet "
            "cc9_base_launch_deck_arms)", host.base_launch_placed,
            host.base_launch_place_refused, host.base_launch_ground_entries);
        host.log.notef("summary base launch lift: down=%llu up=%llu top=%llu blocked=%llu "
            "(006FC640 / 006FC810 / 006D0667 / 006D02F0, packet cc9_base_launch_deck_arms)",
            host.base_launch_lift_down, host.base_launch_lift_up, host.base_launch_lift_top,
            host.base_launch_lift_blocked);
        host.log.notef("summary base launch lift-off: liftoffs=%llu unparented=%llu site_leaves=%llu "
            "c01_sets=%llu (007CC1B3 / 007C6F50 / 007C7110, packet cc9_base_launch_liftoff)",
            host.base_launch_liftoffs, host.base_launch_liftoff_unparented,
            host.base_launch_liftoff_site_leaves, host.base_launch_c01_sets);
        if constexpr (bsp::kTakeoffOwnSiteBound) {
            host.log.notef("summary takeoff own site: answers=%llu (006C086D-006C0964, packet "
                "cc9_takeoff_own_site)", host.takeoff_own_site_answers);
        }
        if constexpr (bsp::kBaseLaunchBrakeBound) {
            unsigned long long sets = 0, clears = 0, held = 0;
            bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
            for (std::size_t di = 0; di < decks.size(); ++di) {
                const bsp::AirOpsDeck* deck = decks.mutable_at(di);
                if (deck == nullptr) continue;
                sets += deck->launch_brake_sets;
                clears += deck->launch_brake_clears;
                if (deck->launch_in_progress != 0u) ++held;
            }
            host.log.notef("summary base launch brake: sets=%llu clears=%llu released=%llu "
                "gone=%llu held_at_end=%llu (006C5050 / 006C5B70, packet cc9_base_launch_brake)",
                sets, clears, host.base_launch_brake_releases, host.base_launch_brake_gone, held);
        }
        host.log.notef("summary base launch takeoff task: installs=%llu parking_refused=%llu "
            "to_slow=%llu to_run=%llu done=%llu permission_asks=%llu denied=%llu (0099A4A0 / "
            "009CFD70, packet cc9_takeoff_task_bind)", host.takeoff_installs,
            host.takeoff_parking_refused, host.takeoff_to_slow, host.takeoff_to_run,
            host.takeoff_done, host.takeoff_permission_asks, host.takeoff_permission_denied);
        host.log.notef("summary takeoff decel lines=%llu planes=%llu suppressed=%llu",
            host.takeoff_decel_lines, host.takeoff_decel_planes, host.takeoff_decel_suppressed);
        if constexpr (kTakeoffTaskHeadBound) {
            host.log.notef("summary takeoff task head: land_deferred=%llu land_after_takeoff=%llu "
                "(00999F50 / 0099AE89 / 0099A4C0 -> 0099A170, packet cc9_takeoff_task_head)",
                host.land_installs_deferred, host.land_installs_after_takeoff);
        }
        if constexpr (kThrottleDeadBandBound) {
            host.log.notef("summary throttle dead band: applied=%llu skipped=%llu "
                "(0099DB1F-0099DB56, packet cc9_throttle_dead_band)",
                host.throttle_band_applied, host.throttle_band_skipped);
        }
        if constexpr (bsp::kPlaneTakeoffStartNotifyCc13Bound) {
            host.log.notef("summary takeoff start notify: calls=%llu posts=%llu arm_7cb5f0=%llu "
                "arm_7c7430=%llu five_to_four=%llu arm_7ca3f0=%llu (007C17D0 -> C3h -> "
                "007CD02C, packet cc13-takeoff)", host.takeoff_notify_calls,
                host.takeoff_notify_posts, host.takeoff_notify_arm_7cb5f0,
                host.takeoff_notify_arm_7c7430, host.takeoff_notify_5_to_4,
                host.takeoff_notify_arm_7ca3f0);
        }
        if constexpr (bsp::kPlaneShipyardLaunchAa0Cc13Bound) {
            host.log.notef("summary shipyard launch aa0: planes=%llu slow_pushes=%llu "
                "leaves_60m=%llu (007D5D20 / 009CF8E0 / 009CFB60, packet cc13-takeoff)",
                host.shipyard_launch_aa0_planes, host.shipyard_launch_aa0_slow_pushes,
                host.shipyard_launch_aa0_leaves);
        }
        if constexpr (kGroundRetakeoffBound) {
            host.log.notef("summary ground retakeoff: pushes=%llu parking=%llu takeoff=%llu "
                "land_head=%llu other_head=%llu state6=%llu (0099B0BE -> 009CFF40 -> 00999F50, "
                "packet cc9_ground_retakeoff)", host.retakeoff_pushes, host.retakeoff_parking,
                host.retakeoff_takeoff, host.retakeoff_land_head, host.retakeoff_other_head,
                host.retakeoff_state6);
            for (const auto& s : host.slots) {
                if (!s || s->retakeoff_pushes == 0) continue;
                host.log.notef("summary ground retakeoff member %s: pushes=%llu",
                    s->row.name.c_str(), s->retakeoff_pushes);
            }
        }
        for (const auto& s : host.slots) {
            if (!s || s->takeoff_installs == 0) continue;
            host.log.notef("summary takeoff member %s: installs=%llu state=%X installed_at=%.2f "
                "prep_left=%.2f (%s) slow_left=%.2f ticks=%llu prep=%llu slow=%llu "
                "run_refused=%llu flight_state=%d run_ticks=%llu aligned=%llu taxi=%llu hold=%llu "
                "low_land=%llu no_holder=%llu first_aligned=%.2f last_thr=%.3f",
                s->row.name.c_str(), s->takeoff_installs,
                static_cast<unsigned>(s->takeoff_state),
                static_cast<double>(s->takeoff_installed_at),
                static_cast<double>(s->takeoff_prep_left_at), s->takeoff_prep_exit,
                static_cast<double>(s->takeoff_slow_left_at), s->takeoff_ticks,
                s->takeoff_prep_ticks, s->takeoff_slow_ticks, s->takeoff_run_refused,
                s->plane_control_mode_900, s->takeoff_run_ticks, s->takeoff_run_aligned,
                s->takeoff_run_speed, s->takeoff_run_hold, s->takeoff_run_low_land,
                s->takeoff_run_no_holder, static_cast<double>(s->takeoff_run_first_aligned),
                static_cast<double>(s->takeoff_run_last_throttle));
        }
        for (const Impl::LandingDeck& d : host.landing_decks) {
            if (!d.built || d.owner >= host.slots.size()) continue;
            host.log.notef("summary base launch site %s: ready_plane=%s sets=%llu",
                host.slots[d.owner]->row.name.c_str(),
                d.site_ready_18 < host.slots.size()
                    ? host.slots[d.site_ready_18]->row.name.c_str() : "(none)",
                d.site_ready_sets);
        }
    }
    if constexpr (kLandingSequencerBound) {
        const bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
        for (std::size_t i = 0; i < host.landing_decks.size(); ++i) {
            const Impl::LandingDeck& d = host.landing_decks[i];
            if (!d.built) continue;
            host.log.notef("summary landing sequencer deck %s: refused=%d queue=%zu records=%zu "
                "passes=%llu hits=%llu outside=%llu inserts=%llu releases=%llu lookups=%llu "
                "found=%llu modes 1=%llu 2=%llu 3=%llu 4=%llu spacing_mode4_refused=%llu site=%llu "
                "landed=%llu slot_tail_unapplied=%llu (packet cc9_landing_sequencer) "
                "site_busy=%llu bound=%d (006D0530, cc13-carrierdeck)",
                i < decks.size() ? decks.name_at(i).c_str() : "?", d.refused ? 1 : 0,
                d.queue.size(), d.assignments.size(), d.passes, d.hit_passes, d.outside_passes,
                d.inserts, d.releases, d.lookups, d.found, d.mode_counts[1], d.mode_counts[2],
                d.mode_counts[3], d.mode_counts[4], d.spacing_mode4_refused, d.spacing_mode4_site,
                d.landed_arm_hits, d.slot_tail_unapplied, d.spacing_site_busy,
                Impl::kCarrierSiteHasFreeRunwayCc13Bound ? 1 : 0);
            for (const Impl::LandingAssignment& a : d.assignments) {
                host.log.notef("summary landing record %s: mode=%d path=%.1f spacing=%.3f "
                    "side=%d", host.slots[a.plane]->row.name.c_str(), a.mode_10,
                    static_cast<double>(a.path_4), static_cast<double>(a.spacing_8),
                    a.side_0d ? 1 : 0);
            }
        }
        for (const std::unique_ptr<GameUnitSlot>& s : host.slots) {
            if (s->land_installed_at < 0.0) continue;
            host.log.notef("summary landing plane %s: found=%llu mode=%d spacing=%.3f side=%d "
                "standby entries=%llu ticks=%llu last_circle_d=%.1f r=%.1f "
                "line entries=%llu ticks=%llu last_head_d=%.1f "
                "begin entries=%llu ticks=%llu abort_refused=%llu final_refused=%llu "
                "hold_unapplied=%llu young_bot=%llu last_a8=%.1f "
                "final entries=%llu ticks=%llu abort_refused=%llu park_refused=%llu "
                "ground_refused=%llu touched=%d",
                s->row.name.c_str(), s->land_found_assignments, s->land_mode_50,
                static_cast<double>(s->land_spacing_4c), s->land_side_44 ? 1 : 0,
                s->land_standby_entries, s->land_standby_ticks,
                static_cast<double>(s->land_standby_last_d),
                static_cast<double>(s->land_radius_48),
                s->land_line_entries, s->land_line_ticks,
                static_cast<double>(s->land_line_last_head_d),
                s->land_begin_entries, s->land_begin_ticks, s->land_begin_abort_refused,
                s->land_begin_final_refused, s->land_begin_hold_unapplied,
                s->land_begin_young_bot, static_cast<double>(s->land_begin_last_a8),
                s->land_final_entries, s->land_final_ticks, s->land_final_abort_refused,
                s->land_final_park_refused, s->land_final_ground_refused,
                s->land_touched_20 ? 1 : 0);
            if constexpr (Impl::kPlaneDirectionHoldBound) {
                host.log.notef("summary plane direction hold %s: arms=%llu blend_steps=%llu "
                    "max_seconds=%.3f (packet cc9_landing_descent_2)", s->row.name.c_str(),
                    s->dir_hold_arms, s->dir_hold_blend_steps,
                    static_cast<double>(s->dir_hold_max_seconds));
            }
            if constexpr (Impl::kPlaneGroundRollBound) {
                const float dx = s->motion.position[0] - s->ground_first_pos[0];
                const float dz = s->motion.position[2] - s->ground_first_pos[2];
                host.log.notef("summary plane ground roll %s: arm_steps=%llu law_steps=%llu "
                    "free_steps=%llu band_steps=%llu hold_down=%llu contact=%llu liftoff_req=%llu "
                    "wheel_brake=%.2f last_brake=%.2f last_friction=%.3f stop_t=%.2f roll=%.1f min_bfc=%.3f "
                    "state=%d land_state=%d level_steps=%llu (packet cc9_plane_ground_roll)", s->row.name.c_str(),
                    s->ground_arm_steps, s->ground_law_steps, s->ground_free_steps,
                    s->ground_band_steps, s->ground_hold_down_steps, s->ground_contact_steps,
                    s->ground_liftoff_requests, static_cast<double>(s->plane_wheel_brake_1e0),
                    static_cast<double>(s->ground_last_brake),
                    static_cast<double>(s->ground_last_friction),
                    static_cast<double>(s->ground_stop_time),
                    s->ground_arm_steps > 0 ? static_cast<double>(std::sqrt(dx * dx + dz * dz)) : 0.0,
                    static_cast<double>(s->ground_min_bfc),
                    s->plane_control_mode_900, static_cast<int>(s->land_state), s->ground_level_steps);
            }
            if (s->ground_steer_steps > 0) {
                host.log.notef("summary plane ground steering %s: steps=%llu last_f2=%.4f "
                    "heading=%.4f stop_heading=%.4f yaw_rate=%.5f x=%.1f z=%.1f (packets cc9_plane_ground_steering, "
                    "cc9_plane_ground_pose)", s->row.name.c_str(),
                    s->ground_steer_steps, static_cast<double>(s->ground_steer_last_f2),
                    static_cast<double>(s->plane_heading_c6c),
                    static_cast<double>(s->ground_stop_heading),
                    static_cast<double>(s->plane_body_angular[1]),
                    static_cast<double>(s->motion.position[0]),
                    static_cast<double>(s->motion.position[2]));
            }
            if (s->land_park_entries > 0) {
                host.log.notef("summary land park %s: entries=%llu from_abort=%llu ticks=%llu "
                    "done=%llu joins=%llu leaves=%llu spot_refused=%llu retarget=%llu "
                    "no_hangar=%llu hangar=%llu hangar_at=%.2f q910=%d c00=%d last x=%.1f z=%.1f "
                    "spd=%.2f yaw=%.3f state=%d done_why timer=%llu contact=%llu deck=%llu "
                    "behind=%llu heading=%llu (packet cc9_land_park_taxi)", s->row.name.c_str(),
                    s->land_park_entries, s->land_park_from_abort, s->land_park_ticks,
                    s->land_park_done, s->land_park_joins, s->land_park_leaves,
                    s->land_park_spot_refused, s->land_park_retarget, s->land_park_no_hangar,
                    s->land_park_hangar, s->land_park_hangar_at,
                    s->plane_taxi_queue_910 ? 1 : 0, s->plane_in_hangar_c00 ? 1 : 0,
                    static_cast<double>(s->land_park_last[0]),
                    static_cast<double>(s->land_park_last[1]),
                    static_cast<double>(s->land_park_last[2]),
                    static_cast<double>(s->land_park_last[3]), s->plane_control_mode_900,
                    s->land_park_done_why[0], s->land_park_done_why[1], s->land_park_done_why[2],
                    s->land_park_done_why[3], s->land_park_done_why[4]);
            }
        }
        host.log.notef("summary landing sequencer refused decks=%llu", host.landing_refused_decks);
    }
    if constexpr (Impl::kPlaneTouchdownBound) {
        for (const std::unique_ptr<GameUnitSlot>& s : host.slots) {
            if (s->td_probes == 0) continue;
            host.log.notef("summary plane touchdown %s: probes=%llu contact_steps=%llu "
                "min_height=%.2f wheel=%.2f low_steps=%llu refused_airborne=%llu "
                "refused_vy=%llu touchdowns=%llu at=%.2f state=%d landed904=%d",
                s->row.name.c_str(), s->td_probes, s->td_contact_steps,
                static_cast<double>(s->td_min_height),
                static_cast<double>(s->plane_wheel_height_1fc), s->td_low_steps,
                s->td_refused_airborne, s->td_refused_vy, s->td_touchdowns, s->td_at,
                s->plane_control_mode_900, s->plane_landed_904 ? 1 : 0);
        }
    }
    if constexpr (Impl::kLandAbortStateBound) {
        for (const std::unique_ptr<GameUnitSlot>& s : host.slots) {
            if (s->land_abort_entries == 0) continue;
            host.log.notef("summary land abort %s: entries=%llu ticks=%llu to_standby=%llu "
                "ground_refused=%llu ground_ticks=%llu park_refused=%llu standby entries=%llu",
                s->row.name.c_str(), s->land_abort_entries, s->land_abort_ticks,
                s->land_abort_to_standby, s->land_abort_ground_refused,
                s->land_abort_ground_ticks,
                s->land_abort_park_refused, s->land_standby_entries);
        }
    }
    if (host.gunnery != nullptr) host.gunnery->report();
    if (host.ai != nullptr) host.ai->report();
    host.log.notef("unit motion: %llu motion step(s) of %llu unit tick(s) over %.2f s of "
        "simulated time, %llu instance update(s) of 008255b0",
        host.summary.motion_steps, host.summary.motion_ticks,
        static_cast<double>(host.summary.simulated_seconds), host.summary.instance_updates);
    // Packet cc8_ship_follow: the wake trail 0070D290 measures a formation
    // follower's station along. Nothing consumes it yet, so these counters are
    // the whole observable effect of binding 00810190.
    {
        std::size_t with_trail = 0;
        unsigned long long appends = 0;
        unsigned long long advances = 0;
        unsigned long long merges = 0;
        float longest = 0.0f;
        for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
            const bsp::ShipAiWakeTrail& trail = owned->wake;
            if (trail.appends == 0) continue;
            ++with_trail;
            appends += trail.appends;
            advances += trail.advances;
            merges += trail.merges;
            const float length = bsp::ship_ai_wake_trail_length(trail);
            if (length > longest) longest = length;
        }
        host.log.notef("summary unit wake ships=%llu appends=%llu advances=%llu merges=%llu "
            "longest_trail=%.2f m (00810190, 4 m gate, 50 m legs, 40 slots)",
            static_cast<unsigned long long>(with_trail), appends, advances, merges,
            static_cast<double>(longest));
    }
    // Packet cc8_ship_follow: the unit groups at entity+284h. Columns are NOT
    // produced yet (0070ED30 needs 00811180's across sign), so membership is all
    // this reports and no station may be computed from these records.
    host.log.notef("summary unit formation death leaves=%llu handovers=%llu emptied=%llu bound=%d "
        "(0077BD70 / 0070D8D0 / 0070D0C0 / 0070E4C0, packet cc9_dead_member_group_removal)",
        host.formation_death_leaves, host.formation_death_handovers,
        host.formation_groups_emptied, kDeadMemberLeavesGroupBound ? 1 : 0);
    host.log.notef("summary unit formation leader handover run=%llu posted=%llu skipped=%llu "
        "lua_leaves_posted=%llu whole_leaves=%llu bound=%d (0071C150 -> 00836680 -> 00720450, "
        "packet cc13-jm08)",
        host.director_handovers_run, host.director_handovers_posted,
        host.director_handovers_skipped, host.director_lua_leaves_posted,
        host.director_whole_leaves_at_kill,
        kDeadLeaderDirectorStateCopyBound ? 1 : 0);
    host.log.notef("summary unit formation join follow calls=%llu pushed=%llu bound=%d "
        "(0077FA8D -> 00720CD0, packet cc9_formation_join_follow)",
        host.formation_join_follows, host.formation_join_follows_pushed,
        Impl::kFormationJoinFollowBound ? 1 : 0);
    host.log.notef("summary unit formation groups=%llu joins=%llu creates=%llu rejoins=%llu "
        "clamped=%llu columns_unmeasurable=%llu (0070DB20 create, 0070EF30 join, "
        "0070ED30 column 0)",
        static_cast<unsigned long long>(host.formation_groups.size()),
        host.formation_joins, host.formation_creates, host.formation_rejoins,
        host.formation_clamped, host.formation_columns_unmeasurable);
    if constexpr (Impl::kReleaseIssueIntervalDrawBound) {
        host.log.notef("summary mission release issue interval draws=%llu (007CEAB3, packet "
            "cc9_l66_issue_draw)", host.release_issue_interval_draws);
    }
    if constexpr (kPutToFormationBound) {
        host.log.notef("summary mission putto formation: station_places=%llu headings=%llu "
            "reshapes=%llu (008193A0 / 008196B0 / 0070EFD0, packet cc9_l66_putto)",
            host.putto_station_places, host.putto_headings, host.putto_reshapes);
    }
    for (std::size_t g = 0; g < host.formation_groups.size(); ++g) {
        const Impl::FormationGroup& group = host.formation_groups[g];
        host.log.notef("  formation %zu leader=%s count=%zu column=%d", g,
            (group.leader < host.slots.size()) ? host.slots[group.leader]->row.name.c_str()
                                               : "?",
            group.members.size(), group.column);
        for (const bsp::ShipAiUnitGroupMember& member : group.members) {
            const std::size_t index = static_cast<std::size_t>(member.entity) - 1u;
            host.log.notef("    %-18s across=%9.2f along=%9.2f",
                (index < host.slots.size()) ? host.slots[index]->row.name.c_str() : "?",
                static_cast<double>(member.lateral[0]),
                static_cast<double>(member.axial[0]));
        }
    }
    // Milestone 2k added the ordered pair each unit is running under, which is
    // unit+980h / unit+984h as the ring published them. Milestone 2l adds the
    // authored command and what its latch captured, and the two columns now
    // disagree on purpose: `cruise` writes the director's +243h / +244h / +248h
    // and never the ring, so a ship under an authored command alone shows a
    // zero ordered pair and does not move.
    host.log.notef("  %-20s %-12s %5s %5s %8s %8s %-8s %7s %9s %9s %8s %5s", "unit", "type",
        "party", "class", "throttle", "rudder", "command", "latched", "x", "z", "moved",
        "gate");
    for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
        const GameUnitRow& row = owned->row;
        char latched[16];
        if (row.command_latched) {
            std::snprintf(latched, sizeof(latched), "%s%.2f",
                row.latch_is_heading ? "h" : "r",
                static_cast<double>(row.latch_steer));
        } else {
            std::snprintf(latched, sizeof(latched), "-");
        }
        host.log.notef("  %-20s %-12s %5d %5d %8.3f %8.3f %-8s %7s %9.1f %9.1f %8.2f "
            "%5d%s", row.name.c_str(), row.type_symbol.c_str(), row.party, owned->class_id,
            static_cast<double>(row.throttle), static_cast<double>(row.ordered_rudder),
            row.command.empty() ? "-" : row.command.c_str(), latched,
            static_cast<double>(row.position[0]), static_cast<double>(row.position[2]),
            static_cast<double>(row.distance), row.command_applied ? 1 : 0,
            row.controlled ? "  <- controlled" : "");
    }
    // Milestone 2q: what 00822C20's property-bag arm seeded at creation.
    // `ratio` is the float32 008235CA stored into ring+148h and `axial` the
    // float32 008235EC handed to 0092D770; `expected` is ratio times the class
    // reference speed times the seconds simulated, the straight-run distance a
    // cruise ship that never changes throttle should cover.
    host.log.notef("  %-20s %10s %10s %8s %9s %10s %10s %10s", "unit", "StartSpeed",
        "reference", "ratio", "axial", "expected", "moved", "delta");
    std::size_t seeded = 0;
    for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
        const GameUnitRow& row = owned->row;
        if (!row.start_speed_authored || row.start_speed == 0.0f) continue;
        ++seeded;
        // pc24-lint: allow host diagnostic (GameUnitsHost::report end-of-run log)
        const double expected = static_cast<double>(row.start_speed_ratio)
            * static_cast<double>(row.max_speed)
            * static_cast<double>(host.summary.simulated_seconds);
        // pc24-lint: allow host diagnostic (GameUnitsHost::report end-of-run log)
        host.log.notef("  %-20s %10.4f %10.4f %8.4f %9.4f %10.2f %10.2f %10.2f",
            row.name.c_str(), static_cast<double>(row.start_speed),
            static_cast<double>(row.max_speed),
            static_cast<double>(row.start_speed_ratio),
            static_cast<double>(row.start_speed_axial), expected,
            static_cast<double>(row.distance),
            static_cast<double>(row.distance) - expected);
    }
    host.log.notef("summary mission start speed seeds=%zu non_zero=%zu key=%s",
        host.summary.start_speed_seeds, seeded, bsp::kSceneUnitStartSpeedKey);
    // Milestone 2s: what the hydrodynamic callback did. `drift` is the angle
    // between the last step's displacement and the hull's bow, `peak` the worst
    // over the whole run, and `traj` the speed along that displacement against
    // `fwd`, the axial speed 0092D300 commands. A hull with no drag holds every
    // metre per second it acquires sideways, so traj runs far above fwd and the
    // drift runs toward ninety degrees; with the drag they converge.
    host.log.notef("  %-20s %5s %4s %5s %9s %9s %9s %10s %10s", "unit", "elem", "sub",
        "mat", "drift", "peak", "fwd", "traj", "force_y");
    for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
        const GameUnitRow& row = owned->row;
        if (row.hydro_calls == 0) continue;
        host.log.notef("  %-20s %5d %4d %5d %9.4f %9.4f %9.4f %10.4f %10.1f",
            row.name.c_str(), row.hydro_elements, row.hydro_submerged,
            static_cast<int>(owned->hull_material),
            static_cast<double>(row.drift_degrees),
            static_cast<double>(row.peak_drift_degrees),
            static_cast<double>(row.forward_speed),
            static_cast<double>(row.trajectory_speed),
            static_cast<double>(row.hydro_force[1]));
    }
    host.summary.world_class_6_list = host.world_lists.entries[6].count;
    {
        // The ids a unit's +130h override joins, and how long each list is.
        char ids[512];
        int written = 0;
        for (int id = 0; id < Impl::kWorldListCount; ++id) {
            if (host.world_lists.entries[id].count == 0) continue;
            written += std::snprintf(ids + written,
                sizeof(ids) - static_cast<std::size_t>(written), "%s%d=%zu",
                written == 0 ? "" : " ", id,
                static_cast<std::size_t>(host.world_lists.entries[id].count));
            if (written >= static_cast<int>(sizeof(ids)) - 1) break;
        }
        host.log.notef("summary mission world lists registrations=%llu pushes=%llu "
            "lists{%s} (actual leaf+130h through 00484540; id 6 is the one 009f1877 walks for "
            "ship AI neighbour candidates, and nothing walks it yet)",
            host.summary.world_registrations, host.summary.world_list_pushes, ids);
    }
    std::size_t nodes = 0;
    std::size_t invalid_lists = 0;
    for (const GameUnitWorldList& list : host.world_lists.entries) {
        const GameUnitWorldNode* previous = nullptr;
        const GameUnitWorldNode* node = list.head;
        std::uint32_t visited = 0;
        bool valid = true;
        for (; node != nullptr && visited < list.count; ++visited) {
            if (node->previous != previous || node->unit == nullptr
                || node->unit->world_parent_0030 != &host.world_lists) valid = false;
            previous = node;
            node = node->next;
        }
        nodes += visited;
        if (!valid || node != nullptr || visited != list.count || previous != list.tail)
            ++invalid_lists;
    }
    host.log.notef("world registration owner audit: unavailable=%llu nodes=%zu invalid_lists=%zu ships=%u planes=%u",
        host.summary.world_registration_unavailable, nodes, invalid_lists,
        host.world_lists.entries[6].count, host.world_lists.entries[15].count);
    {
        // Packet cc9_sunk_ship_kill_depth: every wreck the ship motion saw with
        // +5Dh set, its lower hull end and its last y.
        std::size_t wrecks = 0;
        double below_min = 0.0;
        for (const auto& slot : host.slots) {
            if (!slot->wreck_seen) continue;
            ++wrecks;
            if (wrecks == 1 || slot->wreck_min_end_y < below_min) below_min = slot->wreck_min_end_y;
            host.log.notef("  wreck %-16s first_5Dh_t=%.2f min_end_y=%.2f last_y=%.2f "
                "killed_at=%.2f unlinked_nodes=%zu", slot->row.name.c_str(),
                static_cast<double>(slot->wreck_first_t),
                static_cast<double>(slot->wreck_min_end_y),
                static_cast<double>(slot->wreck_last_y),
                static_cast<double>(slot->kill_depth_at), slot->world_nodes_unlinked);
            // Packet cc9_ship_sink_descent: the leak manager of the wreck.
            host.log.notef("  wreck %-16s leaks bound=%d built=%d redistributed=%d "
                "reserve_84=%.1f capacity=%.2f rate_cap=%.4f rate_34=%.4f water=%.2f "
                "first_water_t=%.2f mass=%.1f elements=%zu", slot->row.name.c_str(),
                Impl::kShipSinkDescentBound ? 1 : 0, slot->leak_ready ? 1 : 0,
                slot->leak_redistributed ? 1 : 0,
                static_cast<double>(slot->controller_reserve_84),
                static_cast<double>(slot->leak_capacity_0c),
                static_cast<double>(slot->leak_rate_cap_08),
                static_cast<double>(slot->leak_rate_34),
                static_cast<double>(slot->leak_water_mass_10fc),
                static_cast<double>(slot->leak_first_water_t),
                static_cast<double>(slot->motion_class.hull_mass),
                slot->buoyancy_elements.size());
        }
        host.log.notef("summary squadron pass hooks pass_a=%llu pass_c=%llu (no-op entries, "
            "packet cc9_squadron_pass_hooks)", host.squadron_pass_a_hook_calls,
            host.squadron_pass_c_hook_calls);
        host.log.notef("summary squadron member placement pass_c_bound=%d calls=%llu pass_c=%llu "
            "members=%llu unseeded=%llu no_station=%llu (007F2920, packet "
            "cc9_squadron_member_placement)", Impl::kSquadronPassCPlacementBound ? 1 : 0,
            host.squadron_member_place_calls, host.squadron_member_place_pass_c,
            host.squadron_members_placed, host.squadron_member_place_unseeded,
            host.squadron_member_place_no_station);
        host.log.notef("summary squadron wing construction bound=%d staged=%llu builds=%llu "
            "planes=%llu left_staged=%zu (007F4580 in pass A, packet cc9_wing_construction)",
            kWingConstructionInPassABound ? 1 : 0, host.wing_records_staged,
            host.wing_hook_builds, host.wing_planes_built, host.staged_wings.size());
        host.log.notef("summary squadron initial command bound=%d movetos=%llu stops=%llu "
            "member_orders=%llu skipped_current=%llu no_home=%llu (007F4E9E, packet "
            "cc9_squadron_initial_command)", Impl::kSquadronInitialCommandBound ? 1 : 0,
            host.initial_command_movetos, host.initial_command_stops,
            host.initial_command_member_orders, host.initial_command_skipped_current,
            host.initial_command_no_home);
        host.log.notef("summary squadron scene home base bound=%d keys=%llu resolved=%llu "
            "unresolved=%llu not_airbase=%llu queue_pushes=%llu (007F4C43 / 007F1C00, packet "
            "cc9_scene_home_base_key)", Impl::kSceneHomeBaseBound ? 1 : 0,
            host.scene_home_keys_set, host.scene_home_resolved, host.scene_home_unresolved,
            host.scene_home_not_airbase, host.scene_home_queue_pushes);
        host.log.notef("summary squadron scene home base contract bound=%d calls=%llu "
            "(create_units -> set_squadron_scene_home_base, packet "
            "cc9_scene_home_base_contract)", kSceneHomeBaseContractBound ? 1 : 0,
            host.scene_home_contract_calls);
        host.log.notef("summary plane scene equipment bound=%d squadrons=%llu reads=%llu "
            "none=%llu default=%llu (007CDF20 [unit+C54h], packet cc9_plane_scene_equipment)",
            bsp::kPlaneSceneEquipmentBound ? 1 : 0, host.plane_equipment_squadrons,
            host.plane_equipment_reads, host.plane_equipment_none,
            host.plane_equipment_default);
        host.log.notef("summary units construction push bound=%d pushes=%llu (00928760 -> "
            "00926BE0, packet cc9_units_push_pending)", Impl::kUnitsPendingPushBound ? 1 : 0,
            host.construction_pushes);
        host.log.notef("summary ship sink descent bound=%d leak_models=%llu "
            "redistributions=%llu (00824FE5 / 0074F490, packet cc9_ship_sink_descent)",
            Impl::kShipSinkDescentBound ? 1 : 0, host.leak_models_built,
            host.leak_redistributions);
        host.log.notef("summary live hull leak bound=%d calls=%llu no_model=%llu gated_live=%llu applied=%llu applied_wreck=%llu rate_added=%.3f (008221A7 / 0074F440 / 0074F090, packet cc9_live_hull_leak)", Impl::kLiveHullLeakBound ? 1 : 0,
            host.hull_leak_calls, host.hull_leak_no_model, host.hull_leak_gated_live,
            host.hull_leak_applied, host.hull_leak_applied_wreck, host.hull_leak_rate_added);
        {
            std::size_t flooded = 0;
            for (const auto& slot : host.slots) {
                if (!slot->leak_ready || slot->leak_water_mass_10fc <= 0.0f) continue;
                ++flooded;
                const bool dead = slot->state != nullptr && slot->state->simulate != 0;
                host.log.notef("  hull water %-18s water=%.1f capacity=%.1f health=%.4f "
                    "repair=%d dead=%d first_water_t=%.2f", slot->row.name.c_str(),
                    static_cast<double>(slot->leak_water_mass_10fc),
                    static_cast<double>(slot->leak_capacity_0c),
                    static_cast<double>(slot->leak_health_fraction),
                    slot->class_repair_00d0 ? 1 : 0, dead ? 1 : 0,
                    static_cast<double>(slot->leak_first_water_t));
            }
            host.log.notef("summary live hull repair bound=%d flooded_hulls=%zu (00962E16 / "
                "00923BE0, packet cc9_live_hull_repair)", Impl::kLiveHullRepairBound ? 1 : 0,
                flooded);
        }
        {
            std::size_t touched = 0;
            for (const std::unique_ptr<GameUnitSlot>& s : host.slots) {
                if (!s->terrain_contact_seen) continue;
                ++touched;
                host.log.notef("ship terrain contact census: unit=%s steps=%llu "
                    "max_penetration=%.2f", s->row.name.c_str(), s->terrain_contact_steps,
                    static_cast<double>(s->terrain_max_penetration));
            }
            host.log.notef("summary ship terrain contact bound=%d ships=%zu steps=%llu "
                "stops=%llu (009377E0 kind 8, packet cc9_ship_terrain_contact)",
                Impl::kShipTerrainContactBound ? 1 : 0, touched, host.terrain_contact_steps,
                host.terrain_contact_stops);
            for (const auto& [unit, u] : host.hull_terrain_units) {
                const char* name = unit < host.slots.size()
                    ? host.slots[unit]->row.name.c_str() : "?";
                host.log.notef("hull terrain contact census: unit=%s first=%.2f steps=%llu "
                    "max_depth=%.2f max_up_dv=%.3f max_horizontal_dv=%.3f", name,
                    static_cast<double>(u.first_t), u.steps, static_cast<double>(u.max_depth),
                    static_cast<double>(u.max_up_dv), static_cast<double>(u.max_horizontal_dv));
            }
            const bsp::HullTerrainContactSolver::Census& c = host.hull_terrain.census();
            host.log.notef("summary hull terrain contact bound=%d ships=%zu steps=%llu "
                "contact_steps=%llu candidates=%llu rejected_normal=%llu solves=%llu rows=%llu "
                "retired=%llu max_depth=%.2f dyn_hull=%d hull_shapes=%llu raw_points=%llu "
                "hull_vertices=%llu (00C53630 / 00C3F760 / 00C4B9B0 / 00403720, 00C5DEB0, "
                "packet cc9_hull_terrain_contact_solver)",
                bsp::kHullTerrainContactSolverBound ? 1 : 0, c.units_touched, c.steps,
                c.contact_steps, c.candidates, c.rejected_normal, c.solves, c.rows, c.retired,
                static_cast<double>(c.max_depth), bsp::kHullTerrainDynHullVerticesBound ? 1 : 0,
                c.hull_shapes, c.raw_points, c.hull_vertices);
            for (const auto& [pair, pc] : host.hull_terrain.hull_pairs()) {
                const char* a = pair.first < host.slots.size()
                    ? host.slots[pair.first]->row.name.c_str() : "?";
                const char* b = pair.second < host.slots.size()
                    ? host.slots[pair.second]->row.name.c_str() : "?";
                host.log.notef("hull pair contact census: a=%s b=%s first_step=%llu steps=%llu "
                    "max_depth=%.3f", a, b, pc.first_step, pc.steps,
                    static_cast<double>(pc.max_depth));
            }
            host.log.notef("summary hull hull contact world_phase=%d bound=%d world_steps=%llu "
                "pairs_near=%llu shape_tests=%llu hits=%llu pairs=%zu groups=%llu "
                "multi_hull_groups=%llu max_group_bodies=%d (00C535E0 / 00C4B610 / 00403720, "
                "packet cc9_hull_hull_contact)", bsp::kDynWorldContactPhaseBound ? 1 : 0,
                bsp::kHullHullContactBound ? 1 : 0, c.world_steps, c.hull_pairs_near,
                c.hull_shape_tests, c.hull_hits, host.hull_terrain.hull_pairs().size(),
                c.groups, c.multi_hull_groups, c.max_group_bodies);
            for (const auto& [pair, pc] : host.hull_terrain.draft_pairs()) {
                const char* b = pair.second < host.slots.size()
                    ? host.slots[pair.second]->row.name.c_str() : "?";
                host.log.notef("hull draft contact census: draft=%zu hull=%s first_step=%llu "
                    "steps=%llu max_depth=%.3f", pair.first, b, pc.first_step, pc.steps,
                    static_cast<double>(pc.max_depth));
            }
            host.log.notef("summary hull draft contact bound=%d drafts=%llu filtered=%llu "
                "pairs_near=%llu shape_tests=%llu hits=%llu hit_steps=%llu pairs=%zu "
                "max_depth=%.3f class_bit_clears=%llu sets=%llu (00423C50 / 00C535E0 / "
                "0092BD70, packet cc9_avoid_zone_draft_bodies)",
                bsp::kAvoidZoneDraftBodiesBound ? 1 : 0, c.drafts, c.draft_pairs_filtered,
                c.draft_pairs_near, c.draft_shape_tests, c.draft_hits, c.draft_hit_steps,
                host.hull_terrain.draft_pairs().size(),
                static_cast<double>(c.draft_max_depth), host.hull_class_bit_clears,
                host.hull_class_bit_sets);
            for (const auto& [pair, pc] : host.hull_terrain.fort_pairs()) {
                const char* a = pair.first < host.slots.size()
                    ? host.slots[pair.first]->row.name.c_str() : "?";
                const char* b = pair.second < host.slots.size()
                    ? host.slots[pair.second]->row.name.c_str() : "?";
                host.log.notef("hull fort contact census: fort=%s hull=%s first_step=%llu "
                    "steps=%llu max_depth=%.3f", a, b, pc.first_step, pc.steps,
                    static_cast<double>(pc.max_depth));
            }
            host.log.notef("summary hull fort contact bound=%d forts=%llu shapes=%llu "
                "over_reserve=%llu pairs_near=%llu shape_tests=%llu hits=%llu hit_steps=%llu "
                "pairs=%zu max_depth=%.3f (007482B0 / 00C535E0, packet cc9_hull_fort_contact)",
                bsp::kHullFortContactBound ? 1 : 0, c.forts, c.fort_shapes,
                c.forts_over_reserve, c.fort_pairs_near, c.fort_shape_tests, c.fort_hits,
                c.fort_hit_steps, host.hull_terrain.fort_pairs().size(),
                static_cast<double>(c.fort_max_depth));
        }
        host.log.notef("summary sunk ship kill depth bound=%d wrecks=%zu lowest_end_y=%.2f "
            "tests=%llu kills=%zu unlinked_nodes=%llu list6=%u kill_depth=%.1f (00826628, "
            "packet cc9_sunk_ship_kill_depth)", Impl::kSunkShipKillDepthBound ? 1 : 0,
            wrecks, below_min, host.kill_depth_tests, host.kill_depth_kills,
            host.world_nodes_unlinked, host.world_lists.entries[6].count,
            static_cast<double>(Impl::kKillDepthSubstitute));
        host.log.notef("summary sunk hull terrain mask bound=%d cleared_units=%zu "
            "wreck_terrain_steps=%llu (00826410..0082643B / 00C47F60, packet "
            "cc9_sunk_hull_shape_flag8)", bsp::kSunkHullTerrainMaskBound ? 1 : 0,
            host.sunk_mask_cleared_units, host.sunk_wreck_terrain_steps);
        std::size_t torpedo_boats = 0;
        for (const auto& owned : host.slots) {
            if (owned != nullptr && bsp::unit_is_kind_of(owned->class_id, 0x0e)) ++torpedo_boats;
        }
        host.log.notef("summary torpedo boat kind bound=%d units=%zu trait_queries=%llu "
            "sink_20s_steps=%llu (00826A78 / 0092E9D1 / 008263F0, packet "
            "cc9_torpedo_boat_kind)", bsp::kShipTorpedoBoatKindBound ? 1 : 0, torpedo_boats,
            host.torpedo_boat_trait_queries, host.torpedo_boat_sink_20s_steps);
        host.log.notef("summary torpedo boat arms boost_steps=%llu boost_ordered_steps=%llu "
            "righting_steps=%llu righting_abs=%.4f (00826A6F / 0092E9D7, packet "
            "cc9_torpedo_boat_kind)", host.torpedo_boat_boost_steps,
            host.torpedo_boat_boost_ordered_steps, host.torpedo_boat_righting_steps,
            host.torpedo_boat_righting_abs);
    }
    host.log.notef("summary mission hydrodynamics calls=%llu element_steps=%llu "
        "submerged_steps=%llu add_force=%llu add_torque=%llu gravity_y=%.1f "
        "elements_per_hull=%d (009329c0 from 00937440 at 00937622, its last call)",
        host.summary.hydro_calls, host.summary.hydro_element_steps,
        host.summary.hydro_submerged_steps, host.summary.hydro_force_flushes,
        host.summary.hydro_torque_flushes,
        static_cast<double>(host.physics_world.gravity.y), kBuoyancyElementCount);
    for (const std::unique_ptr<GameUnitSlot>& s : host.slots) {
        if (!s->motion_dispatch.runs_ship_base()) continue;
        double uy = s->motion.pose_row1[1];
        if (uy > 1.0) uy = 1.0;
        if (uy < -1.0) uy = -1.0;
        // pc24-lint: allow host diagnostic (GameUnitsHost::report end-of-run log)
        host.log.notef("summary hull tilt %s: max=%.2f final=%.2f deg (diagnostic, cc9_hull_inertia)",
            s->row.name.c_str(), static_cast<double>(s->diag_hull_tilt_max_deg),
            std::acos(uy) * 57.29577951308232);
    }
    {
        std::size_t classes_ok = 0;
        for (const auto& [type_id, entry] : host.class_buoyancy_lists) {
            if (entry.ok) ++classes_ok;
        }
        host.log.notef("summary ship buoyancy elements bound=%d classes=%zu classes_image=%zu "
            "hulls_image=%llu hulls_stand_in=%llu fallbacks=%llu (0082D040 at 0082FEE3, packet "
            "cc9_buoyancy_elements)", Impl::kShipBuoyancyElementsBound ? 1 : 0,
            host.class_buoyancy_lists.size(), classes_ok, host.buoyancy_lists_image,
            host.buoyancy_lists_stand_in, host.buoyancy_lists_fallbacks);
    }
    std::size_t generic_units = 0;
    std::size_t generic_input_one = 0;
    for (const auto& slot : host.slots) {
        if (slot->motion_dispatch.entry != 0x00953cc0u) continue;
        ++generic_units;
        if (slot->generic_input_63c == 1.0f) ++generic_input_one;
    }
    host.log.notef("summary mission generic tick: calls=%llu unavailable=%llu "
        "units=%zu input_one=%zu (00953cc0; current roles0/4=8, flag634=0)",
        host.summary.generic_tick_calls, host.summary.generic_tick_unavailable,
        generic_units, generic_input_one);
    {   // cc9-sweep3b reach counter for S18 (log only).
        const bsp::PlaneTaskReachPair& fa = bsp::g_plane_task_reach_sw3.free_attack;
        host.log.notef("summary mission plane task reach (sw3) free_attack 009F830C "
            "(kFreeAttackAcceptTestCc13Bound): calls=%llu differs=%llu inner=%llu",
            fa.calls.load(), fa.differs.load(), fa.inner.load());
        // cc9-sweep3c reach counters for the plane-logic OFF switches (log only).
        const bsp::PlaneTaskReachSw3& r = bsp::g_plane_task_reach_sw3;
        const struct { const char* name; const bsp::PlaneTaskReachPair* pair; } rows[] = {
            {"aim_speed_corr_off 009A7932 (kSeg67bDogfightAimSpeedCorrOffBound)",
             &r.aim_speed_corr_off},
            {"dive_release_alt 009C89E4 (kDiveReleaseAltActivationOnlyBound)",
             &r.dive_release_alt},
            {"moveto_lua_alt_order 009C375A (kMoveToActivationLuaAltOrderBound)",
             &r.moveto_lua_alt_order},
            {"ai_bag_equipment 00A04608 (kAiPlaneBagEquipmentBound)", &r.ai_bag_equipment},
            {"torpedo_fallthrough 009D49DA (kTorpedoArmFallthroughReleaseBound)",
             &r.torpedo_fallthrough},
        };
        for (const auto& row : rows) {
            host.log.notef("summary mission plane task reach (sw3) %s: calls=%llu differs=%llu "
                "inner=%llu", row.name, row.pair->calls.load(), row.pair->differs.load(),
                row.pair->inner.load());
        }
    }
    host.log.notef("summary mission plane step: steps=%llu free_flight=%llu "
        "ground_roll=%llu surface=%llu none=%llu (007ce040 arm selection)",
        host.summary.plane_steps, host.summary.plane_arm_free_flight,
        host.summary.plane_arm_ground_roll, host.summary.plane_arm_surface,
        host.summary.plane_arm_none);
    host.log.notef("summary mission plane row position bound=%d refreshes=%llu (entity+FCh "
        "copy for 008A7C3C, packet cc9_controlled_plane_ai_moveto)",
        kPlaneRowPositionBound ? 1 : 0, host.summary.plane_row_refreshes);
    host.log.notef("summary mission squadron travel alt bound=%d gated_refreshes=%llu "
        "(0089F550 -> 009C3650 gate, packet cc9_squadron_travel_alt)",
        kSquadronTravelAltBound ? 1 : 0, host.summary.squadron_travel_alt_refreshes);
    if (host.dive_profile_travel_alt_resets != 0 || host.dive_profile_clock_reseeds != 0) {
        host.log.notef("summary mission dive cruise profile gate bound=%d travel_alt_resets=%llu "
            "clock_reseeds=%llu (009C894F-009C8971, 009C8A74, packet p23_dive_profile_gate)",
            GameUnitsHost::Impl::kDiveCruiseProfileGateBound ? 1 : 0,
            host.dive_profile_travel_alt_resets, host.dive_profile_clock_reseeds);
    }
    for (const std::unique_ptr<GameUnitSlot>& s : host.slots) {
        if (!s->sq_ever_set) continue;
        host.log.notef("  squadron travel alt %s: alt=%.1f force=%d active=%d last_cruise=%.1f "
            "y=%.1f", s->row.name.c_str(), static_cast<double>(s->sq_alt_394),
            s->sq_freeze_38d ? 1 : 0, s->sq_alt_set ? 1 : 0,
            static_cast<double>(s->sq_last_cruise), static_cast<double>(s->motion.position[1]));
    }
    if constexpr (bsp::kStrikeCtlCruiseCapBound) {
        host.log.notef("summary strike ctl+394h cap feeds=%llu (009FBA9B, packet "
            "cc9_l57_ctl_cruise_cap)", host.strike_ctl_cap_feeds);
    }
    if constexpr (kDiveProfileDrawBound) {
        if constexpr (kDepartedWingmanTaskBlockBound) {
            host.log.notef("summary dive profile departed wingmen block_reads=%llu (007F3970 "
                "keeps [task+404h] and +9D8h, packet cc9_air_ops_squadron_registry)",
                host.departed_block_reads);
        }
        for (const std::unique_ptr<GameUnitSlot>& s : host.slots) {
            if (s->sq_profile_draws == 0) continue;
            host.log.notef("  dive profile draw %s: draws=%llu writes=%llu last_398=%.3f "
                "(009C8920 009C899B, packet cc9_dive_profile_draw)", s->row.name.c_str(),
                s->sq_profile_draws, s->sq_profile_writes,
                static_cast<double>(s->sq_profile_398));
        }
    }
    if constexpr (kSquadronAttackAltBound) {
        for (const std::unique_ptr<GameUnitSlot>& s : host.slots) {
            if (s->sq_timer_expiry_37c < 0.0) continue;
            host.log.notef("  squadron attack alt %s: alt=%.1f force=%d active=%d reads=%llu "
                "last_begin_alt=%.1f (008A22B0 -> 009C8920 gate, packet cc9_squadron_attack_alt)",
                s->row.name.c_str(), static_cast<double>(s->sq_alt_398), s->sq_freeze_38c ? 1 : 0,
                s->sq_attack_alt_set ? 1 : 0, s->sq_attack_alt_reads,
                static_cast<double>(s->sq_last_attack_alt));
        }
    }
    {
        // Packet cc9_plane_death_modes.
        std::size_t modes[6] = {0, 0, 0, 0, 0, 0};
        std::size_t removed = 0;
        std::size_t flagged = 0;
        for (const auto& slot : host.slots) {
            ++modes[static_cast<int>(slot->plane_death_mode)];
            if (slot->plane_death_removed) ++removed;
            if (slot->plane_death_mode != bsp::PlaneDeathMode::None && slot->state != nullptr
                && slot->state->simulate != 0 && slot->scene_pending_destroy_0060 != 0) ++flagged;
        }
        host.log.notef("summary mission plane death modes: explosion=%zu delayed=%zu "
            "powerlost=%zu spin=%zu removed=%zu dead_releases_refused=%llu bound=%d "
            "(007CA8A0 / 007CAF10 / 007CEA1C, packet cc9_plane_death_modes)",
            modes[1], modes[2], modes[3], modes[4] + modes[5], removed,
            host.summary.dead_releases_refused,
            GameUnitsHost::Impl::kPlaneDeathModesBound ? 1 : 0);
        host.log.notef("summary mission plane death flags: dead_with_5d_60=%zu bound=%d "
            "(00926C80 / 00926390, packet cc9_plane_death_flags)", flagged,
            GameUnitsHost::Impl::kPlaneDeathFlagsBound ? 1 : 0);
        host.log.notef("summary mission dead plane bot think: ticks_skipped=%llu bound=%d "
            "(0099ACE1 / 0099ACEB, packet cc9_dead_plane_bot_think)",
            host.summary.dead_plane_bot_ticks_skipped,
            GameUnitsHost::Impl::kDeadPlaneBotThinkBound ? 1 : 0);
        host.log.notef("summary mission leader turn roll: uses=%d bound=%d (007B95E0, packet "
            "cc9_leader_turn_roll)", host.leader_turn_roll_uses,
            GameUnitsHost::Impl::kPlaneLeaderTurnRollBound ? 1 : 0);
        {
            unsigned long long arms = 0, raises = 0, rsteps = 0, bysteps = 0;
            double byangle = 0.0;
            unsigned long long dclifts = 0;
            double dctotal = 0.0;
            unsigned long long pfsteps = 0;
            for (const auto& s : host.slots) {
                if (!s) continue;
                arms += s->speed_floor_arms;
                raises += s->speed_floor_raises;
                rsteps += s->rate_floor_steps;
                bysteps += s->bank_yaw_steps;
                dclifts += s->deck_clamp_lifts;
                dctotal += s->deck_clamp_lift_total;
                pfsteps += s->pose_first_steps;
                byangle += s->bank_yaw_abs_angle;
            }
            host.log.notef("summary mission plane bank yaw: steps=%llu abs_angle=%.3f rad "
                "bound=%d (007DA00C-007DA117, packet cc9_plane_bank_yaw)", bysteps, byangle,
                GameUnitsHost::Impl::kPlaneBankYawBound ? 1 : 0);
            host.log.notef("summary mission plane deck clamp: lifts=%llu lift_total=%.3f m bound=%d "
                "(007DA2B1-007DA338 / 007D7D70, packet cc9_plane_deck_clamp)", dclifts, dctotal,
                GameUnitsHost::Impl::kPlaneDeckClampBound ? 1 : 0);
            host.log.notef("summary mission plane pose first: steps=%llu bound=%d (007D9F60 "
                "before 007DB680, packet cc9_plane_pose_first)", pfsteps,
                GameUnitsHost::Impl::kPlanePoseFirstBound ? 1 : 0);
            host.log.notef("summary mission plane rate floor: speed_floor_arms=%llu "
                "speed_floor_raises=%llu rate_floor_steps=%llu bound=%d/%d (007D7A40 / "
                "007D87C7 / 007DA710, packet cc9_plane_rate_floor)", arms, raises, rsteps,
                GameUnitsHost::Impl::kPlaneSpeedFloorBound ? 1 : 0,
                GameUnitsHost::Impl::kPlaneRateLawCrossingFloorBound ? 1 : 0);
        }
        host.log.notef("summary mission wing fold: classes=%llu extends=%llu retracts=%llu "
            "resets=%llu elevator_waits=%llu prep_holds=%llu run_holds=%llu bound=%d/%d "
            "(007EABC0 / 007CC18E / 006D06E8 / 009CDEED / 009CF2ED, packet cc9_wing_fold)",
            host.wing_fold_classes, host.wing_fold_extends, host.wing_fold_retracts,
            host.wing_fold_resets, host.wing_fold_elevator_waits, host.wing_spread_prep_holds,
            host.wing_spread_run_holds, GameUnitsHost::Impl::kPlaneWingFoldElevatorBound ? 1 : 0,
            GameUnitsHost::Impl::kPlaneWingSpreadTakeoffBound ? 1 : 0);
    }
    host.log.notef("summary mission plane motion: distance_moved=%.2f m "
        "pose_right_reference=%llu pose_collapsed=%llu "
        "pose_rotations=%llu heading_change=%.3f rad thinks=%llu commits=%llu yaw_plans=%llu",
        host.summary.plane_distance_moved,
        host.summary.plane_pose_right_reference,
        host.summary.plane_pose_collapsed,
        host.summary.plane_pose_rotations,
        host.summary.plane_heading_change,
        host.summary.pilot_thinks,
        host.summary.pilot_commits,
        host.summary.pilot_yaw_plans);
    host.log.notef("summary unit plane pitch mode two bound=%d steps=%llu (0099DC9E..0099DD5A, "
        "packet cc9_torpedo_run_pitch_profile)", Impl::kPlanePitchModeTwoBound ? 1 : 0,
        host.pitch_mode_two_steps);
    {
        // What an ordered aircraft actually did about the order. The range is
        // measured at the first think that planned for it and at the last, so a
        // plane that flew straight past shows a small closure and one that
        // turned in shows a large one.
        std::size_t ordered = 0;
        double first_total = 0.0;
        double last_total = 0.0;
        double worst_closure = 0.0;
        for (const auto& slot : host.slots) {
            if (slot->attack_range_first < 0.0f) continue;
            ++ordered;
            first_total += slot->attack_range_first;
            last_total += slot->attack_range_last;
            const double closed = static_cast<double>(slot->attack_range_first) -
                                  static_cast<double>(slot->attack_range_last);
            if (ordered == 1 || closed < worst_closure) worst_closure = closed;
        }
        if (ordered > 0) {
            double err_first = 0.0;
            double err_last = 0.0;
            double pitch_last = 0.0;
            for (const auto& slot : host.slots) {
                if (slot->attack_range_first < 0.0f) continue;
                err_first += slot->attack_hdg_err_first;
                err_last += slot->attack_hdg_err_last;
                pitch_last += slot->attack_pitch_last;
            }
            const double n = static_cast<double>(ordered);
            // Per aircraft, because the mean hides the case that matters: a
            // plane pointing at its target and still losing ground because the
            // target is faster than it is.
            for (const auto& slot : host.slots) {
                if (slot->attack_range_first < 0.0f) continue;
                host.log.notef("  ordered %-12s range %8.1f -> %8.1f m  closed %8.1f m  "
                    "heading error %.3f -> %.3f rad",
                    slot->row.name.c_str(),
                    static_cast<double>(slot->attack_range_first),
                    static_cast<double>(slot->attack_range_last),
                    static_cast<double>(slot->attack_range_first - slot->attack_range_last),
                    static_cast<double>(slot->attack_hdg_err_first),
                    static_cast<double>(slot->attack_hdg_err_last));
            }
            // pc24-lint: allow host diagnostic (GameUnitsHost::report end-of-run log)
            host.log.notef("summary mission pilot attack: ordered=%zu "
                "range_first_mean=%.1f m range_last_mean=%.1f m closed_mean=%.1f m "
                "worst_closed=%.1f m | heading_error_first_mean=%.3f rad "
                "heading_error_last_mean=%.3f rad final_pitch_mean=%.3f rad",
                ordered, first_total / n, last_total / n,
                (first_total - last_total) / n, worst_closure,
                err_first / n, err_last / n, pitch_last / n);
        } else {
            host.log.notef("summary mission pilot attack: no unit was ever ordered "
                "at a target the yaw arm could plan for");
        }
        host.log.notef("summary mission approach section points bound=%d "
            "engine_room=%llu magazine=%llu fuel_tank=%llu chance_to_box=%llu "
            "with_unavailable=%llu classes=%zu (00816650 via 009FA260, packet "
            "cc9_ship_section_points)", kApproachSectionPointsBound ? 1 : 0,
            g_section_picks[0], g_section_picks[1], g_section_picks[2],
            g_section_chance_box_picks, g_section_unavailable_picks,
            host.section_points_by_class_.size());
        host.log.notef("summary mission fly-to obstacles bound=%d torpedo_goaway "
            "rebuilds=%llu kept=%llu avoid_ticks=%llu side_writes=%llu | dive_goaway "
            "rebuilds=%llu kept=%llu avoid_ticks=%llu side_writes=%llu (009FD570 cache, "
            "packet cc9_fly_to_obstacles)", kFlyToObstacleListBound ? 1 : 0,
            host.fly_to_rebuilds_[0], host.fly_to_kept_[0], host.fly_to_avoid_ticks_[0],
            host.fly_to_side_flips_[0], host.fly_to_rebuilds_[1], host.fly_to_kept_[1],
            host.fly_to_avoid_ticks_[1], host.fly_to_side_flips_[1]);
        host.log.notef("summary mission fly-to world edge bound=%d calls=%llu near=%llu "
            "(00681F40 / 009FA510, packet cc9_p12_fly_to_world_edge)",
            Impl::kFlyToWorldEdgeBound ? 1 : 0, host.fly_to_edge_calls_, host.fly_to_edge_near_);
        host.log.notef("summary mission torpedo run time bound=%d aim_updates=%llu "
            "done_updates=%llu mean_98_aim=%.3f mean_98_done=%.3f mean_a0_aim=%.1f "
            "mean_a0_done=%.1f (009D1360, packet cc9_torpedo_run_time_update)",
            kTorpedoRunTimeUpdateBound ? 1 : 0, host.torpedo_run_time_updates_[0],
            host.torpedo_run_time_updates_[1],
            host.torpedo_run_time_updates_[0] > 0 ? host.torpedo_run_time_sum_[0] /
                static_cast<double>(host.torpedo_run_time_updates_[0]) : 0.0,
            host.torpedo_run_time_updates_[1] > 0 ? host.torpedo_run_time_sum_[1] /
                static_cast<double>(host.torpedo_run_time_updates_[1]) : 0.0,
            host.torpedo_run_time_updates_[0] > 0 ? host.torpedo_fall_lead_sum_[0] /
                static_cast<double>(host.torpedo_run_time_updates_[0]) : 0.0,
            host.torpedo_run_time_updates_[1] > 0 ? host.torpedo_fall_lead_sum_[1] /
                static_cast<double>(host.torpedo_run_time_updates_[1]) : 0.0);
        host.log.notef("summary mission torpedo reset draws bound=%d seed_bound=%d "
            "draws=%llu seeds=%llu mean_run_time_98=%.3f s (009D0380, 009D0160, packet "
            "cc9_torpedo_reset_draws)", kTorpedoResetDrawsBound ? 1 : 0,
            kTorpedoResetRunTimeSeedBound ? 1 : 0, host.torpedo_reset_draws_,
            host.torpedo_reset_seeds_,
            host.torpedo_reset_seeds_ > 0
                ? host.torpedo_reset_seed_sum_ / static_cast<double>(host.torpedo_reset_seeds_)
                : 0.0);
        host.log.notef("summary mission torpedo goaway aim point bound=%d ticks=%llu "
            "mean_off_origin=%.1f m max_off_origin=%.1f m (009D0670 via 009D0C10, packet "
            "cc9_torpedo_goaway_aim)", kTorpedoGoAwayAimPointBound ? 1 : 0,
            host.goaway_aim_points_,
            host.goaway_aim_points_ > 0
                ? host.goaway_aim_offset_sum_ / static_cast<double>(host.goaway_aim_points_)
                : 0.0,
            host.goaway_aim_offset_max_);


        // The torpedo task census: per ordered aircraft the states its arm
        // The dive-bomb task, kind 8: every ordered aircraft the arm 009C8790
        // ran, its state occupancy, the geometry the in-range latch 009C7C31
        // keys on, and the releases 009C60F1 / 009C5777 issued.
        // docs/DIVE_BOMB_TASK.md.
        // Packet cc9_l65_dc_census: one line per plane that ran the depth-charge
        // task (bsp::kDepthChargeTaskBound), its state occupancy, the ticks with
        // an aim point in range (+75h) and with depth charges aboard (+74h).
        for (const auto& slot : host.slots) {
            if (!slot->depth_charge_task_installed) continue;
            static const char* const kDcStateNames[9] = {
                "moveto", "follow", "done", "attackrun", "goaway",
                "aim", "prepare", "leave", "turnto"};
            char states[192];
            int n = 0;
            states[0] = '\0';
            for (int i = 0; i < 9; ++i) {
                if (slot->dc_state_ticks[i] == 0) continue;
                n += std::snprintf(states + n,
                    (n < static_cast<int>(sizeof(states)))
                        ? sizeof(states) - static_cast<std::size_t>(n) : 0u,
                    "%s%s=%d", n > 0 ? " " : "", kDcStateNames[i], slot->dc_state_ticks[i]);
                if (n >= static_cast<int>(sizeof(states))) break;
            }
            host.log.notef("  depthcharge %-12s arm_ticks=%d transitions=%llu states[%s] "
                "releases=%llu in_range=%d armed=%d min_range=%.1f (009A66C0, packet "
                "cc9_l65_dc_census)", slot->row.name.c_str(), slot->dc_arm_ticks,
                slot->dc_transitions, states, slot->dc_releases, slot->dc_target_ticks,
                slot->dc_armed_ticks,
                static_cast<double>(slot->dc_min_range_48 < 1.0e8f ? slot->dc_min_range_48 : -1.0f));
            host.log.notef("  depthcharge %-12s bay 007BBBA0: accepted=%d refused=%d | rack "
                "006E56F0: single=%d deferred=%d drops=%d ammo=%d dropping=%d gate_refused=%d "
                "ret_census=%d ret_idle=%d | stage 007CE9FD: issues=%d guard_blocked=%d C20h=%d",
                slot->row.name.c_str(), slot->torpedo_bay_requests_accepted,
                slot->torpedo_bay_requests_refused, slot->rack_single_count,
                slot->rack_requests_deferred, slot->rack_bomb_drops, slot->rack_ammo,
                slot->rack_dropping ? 1 : 0, slot->rack_gate_refused,
                static_cast<int>(slot->rack_ret_census), static_cast<int>(slot->rack_ret_idle),
                slot->torpedo_issue_stage_issues, slot->torpedo_issue_stage_guard_blocked,
                slot->torpedo_issue_requests_c20);
        }
        // Packet cc9-rocket1 (log only): the rocket task 007B7C40 and its 007B9480 feed.
        {
            unsigned long long installs = 0, releases = 0, spawned = 0, refused = 0;
            for (const auto& slot : host.slots) {
                if (!slot) continue;
                installs += slot->rk_installs;
                releases += slot->rk_releases;
                spawned += slot->rk_spawned;
                refused += slot->rk_spawn_refused;
            }
            host.log.notef("summary rocket task: bound=%d installs=%llu releases=%llu "
                "spawned=%llu refused=%llu feed_asks=%llu has_rocket_ordnance=%llu "
                "(007B7C40, 007B9480, packet cc9-rocket1)", bsp::kRocketTaskBound ? 1 : 0,
                installs, releases, spawned, refused, host.rocket_feed_queries_,
                host.rocket_feed_true_);
            for (const auto& slot : host.slots) {
                if (!slot || slot->rk_installs == 0) continue;
                const int* s = slot->rk_state_ticks;
                host.log.notef("  rocket %-12s installs=%llu arm_ticks=%d rocket_ticks=%d "
                    "transitions=%llu states[moveto=%d follow=%d prepare=%d gotowards=%d aim=%d "
                    "goaway=%d attackrun=%d] releases=%llu spawned=%llu refused=%llu hits=%llu "
                    "mv_tight=%d mv_flik=%d state=%s", slot->row.name.c_str(), slot->rk_installs,
                    slot->rk_arm_ticks, slot->rk_rocket_ticks, slot->rk_transitions, s[0], s[1],
                    s[2], s[3], s[4], s[5], s[6], slot->rk_releases, slot->rk_spawned,
                    slot->rk_spawn_refused, slot->rk_hit_resets, slot->st_mv_tightturns,
                    slot->st_mv_flikflaks, bsp::rocket_state_name(slot->rk.state));
            }
        }
        // Packet cc13-goaway: 007B5BE0's goaway target term per slot (log only;
        // moved = enters whose target's AA range exceeded the enter's base).
        for (const auto& slot : host.slots) {
            if (!slot || slot->ga_term_enters == 0) continue;
            host.log.notef("  goaway target term %-7s %-12s: enters=%d targets=%d moved=%d "
                "aa_max=%.0f base=%.0f..%.0f (007B5BE0, packet cc13-goaway)",
                slot->ga_term_kind != nullptr ? slot->ga_term_kind : "-",
                slot->row.name.c_str(), slot->ga_term_enters, slot->ga_term_targets,
                slot->ga_term_moved, static_cast<double>(slot->ga_term_aa_max),
                static_cast<double>(slot->ga_term_base_min),
                static_cast<double>(slot->ga_term_base_max));
        }
        {
            static const char* const kDiveBombStateNames[10] = {
                "moveto", "follow", "prepare", "done", "goaway",
                "aimdive", "aimglide", "flyabove", "turndown", "attackrun"};
            std::size_t tasked = 0;
            int releases_total = 0;
            int spawned_total = 0;
            bool geo_trace_printed = false;
            for (const auto& slot : host.slots) {
                if (!slot->dive_bomb_task_installed) continue;
                ++tasked;
                releases_total += slot->dive_bomb_releases;
                spawned_total += slot->torpedo_drops_spawned;
                char states[192];
                int n = 0;
                states[0] = '\0';
                for (int i = 0; i < 10; ++i) {
                    if (slot->dive_bomb_state_ticks[i] == 0) continue;
                    n += std::snprintf(states + n,
                        (n < static_cast<int>(sizeof(states)))
                            ? sizeof(states) - static_cast<std::size_t>(n) : 0u,
                        "%s%s=%d", n > 0 ? " " : "", kDiveBombStateNames[i],
                        slot->dive_bomb_state_ticks[i]);
                    if (n >= static_cast<int>(sizeof(states))) break;
                }
                // CORRECTED, packet cc8_dive_glide: `bombs_spawned` was printing
                // slot->torpedo_drops_spawned, which is the TORPEDO drop's
                // counter and can only ever be zero for a dive bomber. It now
                // prints the bomb spawn's own field, with the bay requests
                // 007BBBA0 accepted and refused beside it.
                host.log.notef("  divebomb %-12s arm_ticks=%d transitions=%d "
                    "states[%s] releases=%d bombs_spawned=%d rounds_left=%d "
                    "| bay 007BBBA0: accepted=%d refused=%d (torpedo column "
                    "was %d)",
                    slot->row.name.c_str(), slot->dive_bomb_arm_ticks,
                    slot->dive_bomb_transitions, states,
                    slot->dive_bomb_releases, slot->db_bombs_spawned,
                    slot->dive_bomb_rounds_remaining,
                    slot->db_bay_requests_accepted,
                    slot->db_bay_requests_refused,
                    slot->torpedo_drops_spawned);
                // Packet cc9_release_issue_stage_vals: the Val's stage and rack,
                // printed in both builds so a pair diffs line for line.
                host.log.notef("  divebomb %-12s rack 006E56F0: single=%d multi=%d "
                    "authored=%d deferred=%d drops=%d ammo=%d dropping=%d gate_refused=%d "
                    "| stage 007CE9FD: issues=%d guard_blocked=%d C20h=%d C25h=%d "
                    "guard_by=%d/%d/%d/%d/%d mode900=%d",
                    slot->row.name.c_str(), slot->rack_single_count,
                    slot->rack_multi_count, slot->rack_rounds_authored,
                    slot->rack_requests_deferred, slot->rack_bomb_drops, slot->rack_ammo,
                    slot->rack_dropping ? 1 : 0, slot->rack_gate_refused,
                    slot->torpedo_issue_stage_issues,
                    slot->torpedo_issue_stage_guard_blocked,
                    slot->torpedo_issue_requests_c20,
                    slot->torpedo_release_pending_c25 ? 1 : 0,
                    slot->torpedo_issue_stage_guard_by[0], slot->torpedo_issue_stage_guard_by[1],
                    slot->torpedo_issue_stage_guard_by[2], slot->torpedo_issue_stage_guard_by[3],
                    slot->torpedo_issue_stage_guard_by[4], slot->torpedo_issue_stage_last_mode_900);
                // 009C7240 / 009C7270, packet cc8_done_state. `placed` counts
                // the ticks on which the placement stand-in moved the aircraft,
                // which is zero for a flight LEADER by 009BFD70's own refusal -
                // so the leader's row is the one that shows what the image's
                // done state commands on its own, and it is one inert byte.
                if (slot->db_done_entries > 0) {
                    host.log.notef("  divebomb %-12s done 009C7240/009C7270: "
                        "entries=%d ticks=%d placed=%d plan_mode_26c=%d "
                        "alt %.1f -> %.1f hdg=%.4f",
                        slot->row.name.c_str(), slot->db_done_entries,
                        slot->db_done_tick_ticks, slot->db_done_placed_ticks,
                        slot->plan_mode_26c,
                        static_cast<double>(slot->db_done_entry_alt),
                        static_cast<double>(slot->db_done_last_alt),
                        static_cast<double>(slot->db_done_last_hdg));
                }
                // The first gate check the packet asks for: approach+BCh
                // against approach+B8h in the latch at 009C7C31.
                if (slot->db_turndown_ticks > 0) {
                    host.log.notef("  divebomb %-12s turndown 009C44F0: ticks=%d "
                        "roll_writes=%d pitch_writes=%d latched_at_tick=%d "
                        "|bank|=%.4f rad roll=%.4f pitch=%.4f speed_2b4=%.1f "
                        "| 009C7EA0 window: pose_c64_last=%.4f pose_c64_min=%.4f "
                        "(needs < -1.3, or < -1.0 with |bank| > 2.356)",
                        slot->row.name.c_str(), slot->db_turndown_ticks,
                        slot->db_turndown_roll_writes,
                        slot->db_turndown_pitch_writes,
                        slot->db_turndown_latched_tick,
                        static_cast<double>(slot->db_turndown_bank_last),
                        static_cast<double>(slot->db_turndown_roll_last),
                        static_cast<double>(slot->db_turndown_pitch_last),
                        static_cast<double>(slot->plane_desired_speed_2b4),
                        static_cast<double>(slot->db_turndown_pose_c64_last),
                        static_cast<double>(slot->db_turndown_pose_c64_min));
                }
                // Packet cc8_dive_geometry: the per-tick window, first slot
                // only - the three USN04 bombers fly identical traces.
                if (slot->db_geo_samples > 0 && !geo_trace_printed) {
                    geo_trace_printed = true;
                    host.log.notef("  divebomb %-12s geo trace: state tick "
                        "pitch_c64 bank_c68 heading_c6c alt range bearing_err "
                        "aim_head roll_in roll_cmd pitch_cmd mode_2d0 mode_2cc",
                        slot->row.name.c_str());
                    for (int i = 0; i < slot->db_geo_samples; ++i) {
                        const GameUnitSlot::DbGeoSample& s = slot->db_geo[i];
                        host.log.notef("  geo %-9s %5d %8.4f %8.4f %8.4f "
                            "%8.1f %8.1f %8.4f %8.4f %8.4f %7.3f %7.3f %d %d",
                            kDiveBombStateNames[s.state], s.tick,
                            static_cast<double>(s.pitch_c64),
                            static_cast<double>(s.bank_c68),
                            static_cast<double>(s.heading_c6c),
                            static_cast<double>(s.altitude),
                            static_cast<double>(s.range),
                            static_cast<double>(s.bearing_err),
                            static_cast<double>(s.aim_heading),
                            static_cast<double>(s.roll_input),
                            static_cast<double>(s.roll_cmd),
                            static_cast<double>(s.pitch_cmd),
                            s.pitch_mode_2d0, s.heading_mode_2cc);
                    }
                }
                if (slot->db_attackrun_ticks > 0) {
                    char ranges[192];
                    int rn = 0;
                    ranges[0] = '\0';
                    for (int i = 0; i < slot->db_range_samples; ++i) {
                        rn += std::snprintf(ranges + rn,
                            (rn < static_cast<int>(sizeof(ranges)))
                                ? sizeof(ranges) - static_cast<std::size_t>(rn) : 0u,
                            "%s%.0f", i > 0 ? " " : "",
                            static_cast<double>(slot->db_range_at_second[i]));
                        if (rn >= static_cast<int>(sizeof(ranges))) break;
                    }
                    host.log.notef("  divebomb %-12s attackrun 009C4220: ticks=%d "
                        "rerolls=%d latch_closed_tick=%d heading=%.4f rad "
                        "throttle=%.3f alt_base=%.1f m | climb_1ec=%.4f rad "
                        "b4=%.1f b8=%.1f | range per second: %s",
                        slot->row.name.c_str(), slot->db_attackrun_ticks,
                        slot->db_attackrun_rerolls, slot->db_latch_closed_tick,
                        static_cast<double>(slot->db_attackrun_heading_last),
                        static_cast<double>(slot->db_attackrun_throttle_last),
                        static_cast<double>(slot->db_attackrun_alt_last),
                        // Packet cc8_dive_race: the two substitutions under test,
                        // printed so neither is argued from a source constant.
                        static_cast<double>(slot->plane_climb_angle_1ec),
                        static_cast<double>(slot->db_attack_dist_b4),
                        static_cast<double>(slot->db_in_range_b8), ranges);
                }
                // Packet cc8_dive_glide: the aimglide release chain
                // 009C5689-009C5755, per aircraft. blocked[] counts the calls
                // that stopped at each gate in order; the lead window is
                // -4*travel - 5.0 < lead < -5.0, so with the 009C4F50 seed of
                // 5.0 and approach+A4h held at zero it is -25.0 .. -5.0 m.
                if (slot->db_glide_calls > 0) {
                    host.log.notef("  divebomb %-12s aimglide 009C5689-009C5755: "
                        "calls=%d blocked[rearm=%d bearing=%d ceiling=%d "
                        "lateral=%d lead_hi=%d lead_lo=%d] passed=%d | "
                        "releases=%d rounds=%d | lead last=%.2f m min=%.2f m "
                        "(window -%.1f..-%.1f m) | bearing err min=%.4f rad "
                        "max=%.4f rad pull_outs=%d "
                        "(gate %.4f) | throw=%.1f m range=%.1f m travel=%.2f",
                        slot->row.name.c_str(), slot->db_glide_calls,
                        slot->db_glide_gate_reached[0], slot->db_glide_gate_reached[1],
                        slot->db_glide_gate_reached[2], slot->db_glide_gate_reached[3],
                        slot->db_glide_gate_reached[4], slot->db_glide_gate_reached[5],
                        slot->db_glide_gate_reached[6],
                        slot->db_glide_releases, slot->db_glide_rounds,
                        static_cast<double>(slot->db_glide_lead_last),
                        static_cast<double>(slot->db_glide_lead_min),
                        4.0 * static_cast<double>(slot->db_glide_travel_20) + 5.0,
                        5.0,
                        static_cast<double>(slot->db_glide_bearing_min),
                        static_cast<double>(slot->db_glide_bearing_max),
                        slot->db_aimglide_pull_outs,
                        static_cast<double>(
                            bsp::dive_bomb_constant::kGlideDiveAngleLimit),
                        static_cast<double>(slot->db_impact_throw_14),
                        static_cast<double>(slot->db_planar_bc),
                        static_cast<double>(slot->db_glide_travel_20));
                }
                // Packet cc8_dive_goaway. 009C4A40's climb-out and 009C7F00's
                // completion, side by side: the state exists to put the aircraft
                // back at its ceiling, so the question is only whether the pitch
                // it commands moves the altitude toward the ceiling.
                if (slot->db_goaway_tick_ticks > 0) {
                    host.log.notef("  divebomb %-12s goaway 009C4A40: ticks=%d "
                        "pitch last=%.4f max=%.4f rad | alt first=%.1f last=%.1f "
                        "max=%.1f m | ceiling=%.1f m deficit=%.1f m | "
                        "009C7F00 complete_ticks=%d travel_20=%.2f",
                        slot->row.name.c_str(), slot->db_goaway_tick_ticks,
                        static_cast<double>(slot->db_goaway_pitch_last),
                        static_cast<double>(slot->db_goaway_pitch_max),
                        static_cast<double>(slot->db_goaway_alt_first),
                        static_cast<double>(slot->db_goaway_alt_last),
                        static_cast<double>(slot->db_goaway_alt_max),
                        static_cast<double>(slot->db_goaway_ceiling_last),
                        static_cast<double>(slot->db_goaway_deficit_last),
                        slot->db_goaway_complete_ticks,
                        static_cast<double>(slot->db_goaway_travel_20));
                    // Packet cc9_goaway_turn: the evasive turn's arms.
                    const bsp::DiveBombGoAwayTurnState& gt = slot->db_goaway_turn;
                    host.log.notef("  divebomb %-12s goaway turn 009C4DAD: "
                        "enters=%d flag0=%d countdown=%d rerolls=%d "
                        "first_reroll=%d bank=%d heading=%d first_heading=%d "
                        "side_writes=%d | bank min=%.3f max=%.3f rad | "
                        "heading last=%.4f err_max=%.4f rad | side=%.1f "
                        "standoff=%.1f countdown=%.2f clock=%.2f window=%.2f",
                        slot->row.name.c_str(), slot->db_goaway_enters,
                        slot->db_goaway_flag0_ticks,
                        slot->db_goaway_countdown_ticks, slot->db_goaway_rerolls,
                        slot->db_goaway_first_reroll_tick,
                        slot->db_goaway_bank_ticks, slot->db_goaway_heading_ticks,
                        slot->db_goaway_first_heading_tick,
                        slot->db_goaway_side_writes,
                        static_cast<double>(slot->db_goaway_bank_min),
                        static_cast<double>(slot->db_goaway_bank_max),
                        static_cast<double>(slot->db_goaway_heading_last),
                        static_cast<double>(slot->db_goaway_heading_err_max),
                        static_cast<double>(gt.side_18),
                        static_cast<double>(gt.standoff_20),
                        static_cast<double>(gt.countdown_24),
                        static_cast<double>(gt.clock_28),
                        static_cast<double>(gt.window_2c));
                }
                host.log.notef("  divebomb %-12s gate 009C7C31: "
                    "approach+BCh=%.1f m approach+B8h=%.1f m latch_D0h=%d "
                    "bomb_D1h=%d | ticks without latch=%d without bombs=%d",
                    slot->row.name.c_str(),
                    static_cast<double>(slot->db_planar_bc),
                    static_cast<double>(slot->db_in_range_b8),
                    slot->db_in_range_d0 ? 1 : 0,
                    slot->db_has_bomb_d1 ? 1 : 0,
                    slot->db_blocked_no_latch, slot->db_blocked_no_bomb);
                // Packet cc8_dive_entry: the altitude at every hand-over, for
                // every dive bomber, outside the dive-entry guard below so the
                // aircraft that never dive are in it too. `cmd` is
                // plane_commanded_altitude, which only the run-in writes in this
                // host - 009C6E10-009C6F7D, the flyabove's own altitude command,
                // is unbound - so a cmd that stops moving across the flyabove is
                // that gap measured rather than argued.
                if (slot->db_transition_count > 0) {
                    char tr[1200];  // cc8_dive_heading widened the per-entry text
                    int tn = 0;
                    tr[0] = '\0';
                    for (int i = 0; i < slot->db_transition_count; ++i) {
                        const GameUnitSlot::DbTransition& t =
                            slot->db_transitions[i];
                        tn += std::snprintf(tr + tn,
                            (tn < static_cast<int>(sizeof(tr)))
                                ? sizeof(tr) - static_cast<std::size_t>(tn) : 0u,
                            "%s%s>%s@%d alt=%.0f rng=%.0f cmd=%.0f "
                            "span=%.0f b=%.0f f18=%d f19=%d",
                            i > 0 ? " | " : "",
                            (t.from >= 0 && t.from < 10)
                                ? kDiveBombStateNames[t.from] : "?",
                            (t.to >= 0 && t.to < 10)
                                ? kDiveBombStateNames[t.to] : "?",
                            t.tick, static_cast<double>(t.alt),
                            static_cast<double>(t.range),
                            static_cast<double>(t.cmd_alt),
                            static_cast<double>(t.span),
                            static_cast<double>(t.b_height),
                            static_cast<int>(t.f18), static_cast<int>(t.f19));
                        if (tn >= static_cast<int>(sizeof(tr))) break;
                    }
                    host.log.notef("  divebomb %-12s hand-overs: %s",
                        slot->row.name.c_str(), tr);
                }
                if (slot->db_fa_alt_calls > 0) {
                    host.log.notef("  divebomb %-12s flyabove altitude "
                        "009C6E10: calls=%d level_arm=%d | C=%.1f band=%.1f "
                        "target=%.1f ref=%.3f | err first=%.1f last=%.1f | "
                        "pitch=%.3f rad",
                        slot->row.name.c_str(), slot->db_fa_alt_calls,
                        slot->db_fa_level_ticks,
                        static_cast<double>(slot->db_fa_limit_c),
                        static_cast<double>(slot->db_fa_band),
                        static_cast<double>(slot->db_fa_target_last),
                        static_cast<double>(slot->db_fa_ref_last),
                        static_cast<double>(slot->db_fa_err_first),
                        static_cast<double>(slot->db_fa_err_last),
                        static_cast<double>(slot->db_fa_pitch_last));
                }
                if (slot->db_dive_entry_alt >= 0.0f ||
                    slot->db_release_alt >= 0.0f) {
                    host.log.notef("  divebomb %-12s dive entry alt=%.1f m "
                        "pitch=%.3f rad | release alt=%.1f m speed=%.1f m/s "
                        "range=%.1f m | aim error 009C5C9B=%.2f m "
                        "(gate 00CE3880 = 25.0 m) closest=%.2f m at range=%.1f m "
                        "alt=%.1f m | 009C58D0 steer: ticks=%d pitch=%.3f "
                        "roll=%.3f bearing=%.4f rad | flyabove B=%.1f m "
                        "span=%.1f m",
                        slot->row.name.c_str(),
                        static_cast<double>(slot->db_dive_entry_alt),
                        static_cast<double>(slot->db_dive_entry_pitch),
                        static_cast<double>(slot->db_release_alt),
                        static_cast<double>(slot->db_release_speed),
                        static_cast<double>(slot->db_release_range),
                        static_cast<double>(slot->db_aim_error_last),
                        static_cast<double>(slot->db_aim_error_abs_min),
                        static_cast<double>(slot->db_aim_error_min_range),
                        static_cast<double>(slot->db_aim_error_min_alt),
                        slot->db_aimdive_steer_ticks,
                        static_cast<double>(slot->db_aimdive_pitch_last),
                        static_cast<double>(slot->db_aimdive_roll_last),
                        static_cast<double>(slot->db_aimdive_bearing_last),
                        static_cast<double>(slot->db_flyabove_height),
                        static_cast<double>(slot->db_flyabove_span));
                // Packet cc8_dive_release item 1: which 009C8634 arm ends the
                // aimdive, and the 009C5B01 abort's own operands when it first
                // fired. MaxSpd/ReferenceSpeed is item 4's quotient, logged in
                // a run taken anyway.
                host.log.notef("  divebomb %-12s aimdive: entries=%d exits=%d "
                    "by[alive_19=%d nobomb_d1=%d pullout_18=%d other=%d] "
                    "first_run=%d last_run=%d | abort 009C5B43: fires=%d "
                    "first_tick=%d d4=%.1f h14=%.1f range=%.1f pitch=%.3f | "
                    "speed: max=%.2f ref=%.2f quotient=%.3f | impact 009C7D71: "
                    "tf=%.2f s range=%.1f m (live %.1f m)",
                    slot->row.name.c_str(),
                    slot->db_aimdive_entries, slot->db_aimdive_exits,
                    slot->db_aimdive_exit_alive, slot->db_aimdive_exit_nobomb,
                    slot->db_aimdive_exit_pullout, slot->db_aimdive_exit_other,
                    slot->db_aimdive_first_run, slot->db_aimdive_last_run,
                    slot->db_abort_fires, slot->db_abort_first_tick,
                    static_cast<double>(slot->db_abort_d4),
                    static_cast<double>(slot->db_abort_h14),
                    static_cast<double>(slot->db_abort_range),
                    static_cast<double>(slot->db_abort_pitch),
                    // desc+188h MaxSpd, NOT motion.max_speed: that one is the
                    // ship row and is 0.00 for every aircraft, which is what the
                    // first run of this packet measured.
                    static_cast<double>(slot->plane_max_spd),
                    // GAME_TUNING_SINGLETON +4D8h Pilot/DiveBomb/ReferenceSpeed
                    // = KMH(280); 009C3EA0 reads it.
                    static_cast<double>(280.0f / 3.6f),
                    static_cast<double>(slot->plane_max_spd /
                                        (280.0f / 3.6f)),
                    static_cast<double>(slot->db_impact_fall_time),
                    static_cast<double>(slot->db_impact_planar_5c),
                    static_cast<double>(slot->db_planar_bc));
                // Packet cc8_attack_mode. squadron+370h per aircraft, and what
                // the spent bomber did afterwards. mode_ticks is indexed by the
                // PilotAttackMode value: [0]=kHold, [1]=kAttack (what 0099B740
                // sets), [2]=kForced (what the Lua attack order raises to).
                host.log.notef("  divebomb %-12s attack mode: lead=%d "
                    "mode_370=%d ticks[hold=%d attack=%d forced=%d] changes=%d "
                    "first_set_tick=%d | after: done_ticks=%d done_min_alt=%.1f "
                    "approach_returns=%d first_return_tick=%d",
                    slot->row.name.c_str(),
                    slot->db_is_flight_lead ? 1 : 0,
                    static_cast<int>(slot->db_attack_mode_370),
                    slot->db_mode_ticks[0], slot->db_mode_ticks[1],
                    slot->db_mode_ticks[2], slot->db_mode_changes,
                    slot->db_mode_first_set_tick,
                    slot->db_post_done_ticks,
                    static_cast<double>(slot->db_post_done_min_alt),
                    slot->db_approach_returns, slot->db_approach_return_tick);
                // 009FBA50's own terms, to settle why correcting its arguments
                // changed nothing observable.
                if (slot->db_cruise_samples > 0) {
                    char ct[320];
                    int cn = 0;
                    ct[0] = '\0';
                    for (int i = 0; i < slot->db_cruise_samples; ++i) {
                        cn += std::snprintf(ct + cn,
                            (cn < static_cast<int>(sizeof(ct)))
                                ? sizeof(ct) - static_cast<std::size_t>(cn) : 0u,
                            "%s%.0f/%.3f/%.3f/%.0f/%.0f", i > 0 ? " " : "",
                            static_cast<double>(slot->db_cruise_span[i]),
                            static_cast<double>(slot->db_cruise_gain[i]),
                            static_cast<double>(slot->db_cruise_scale[i]),
                            static_cast<double>(slot->db_cruise_base[i]),
                            static_cast<double>(slot->db_cruise_clamped[i]));
                        if (cn >= static_cast<int>(sizeof(ct))) break;
                    }
                    host.log.notef("  divebomb %-12s 009FBA50 terms "
                        "span/gain/scale/base/clamped per 120 attackrun ticks: %s",
                        slot->row.name.c_str(), ct);
                }
                // The aim trace: what the endpoint numbers cannot say.
                if (slot->db_aim_state_ticks > 0) {
                    char trace[320];
                    int tn = 0;
                    trace[0] = '\0';
                    for (int i = 0; i < slot->db_aim_trace_samples; ++i) {
                        tn += std::snprintf(trace + tn,
                            (tn < static_cast<int>(sizeof(trace)))
                                ? sizeof(trace) - static_cast<std::size_t>(tn) : 0u,
                            "%s%.0f/%.0f/%.2f/%.2f", i > 0 ? " " : "",
                            static_cast<double>(slot->db_aim_trace_range[i]),
                            static_cast<double>(slot->db_aim_trace_error[i]),
                            static_cast<double>(slot->db_aim_trace_bank[i]),
                            static_cast<double>(slot->db_aim_trace_bearing[i]));
                        if (tn >= static_cast<int>(sizeof(trace))) break;
                    }
                    host.log.notef("  divebomb %-12s aim trace: 009C62B0 ticks=%d "
                        "heading writes=%d last=%.4f rad | turndown entry "
                        "range=%.1f m bearing=%.4f rad | aimdive entry "
                        "range=%.1f m bearing=%.4f rad | closest range=%.1f m | "
                        "ticks=%d | range/error/bank/bearing per 30 ticks: %s",
                        slot->row.name.c_str(),
                        slot->db_flyabove_tick_ticks,
                        slot->db_flyabove_heading_writes,
                        static_cast<double>(slot->db_flyabove_heading_last),
                        static_cast<double>(slot->db_turndown_entry_range),
                        static_cast<double>(slot->db_turndown_entry_bearing),
                        static_cast<double>(slot->db_aimdive_entry_range),
                        static_cast<double>(slot->db_aimdive_entry_bearing),
                        static_cast<double>(slot->db_aimdive_min_range),
                        slot->db_aim_state_ticks, trace);
                    // Packet cc8_dive_flyover: the bank arm's own census. `bl`
                    // is 009C67BF, `latch` the 009C6919 flyabove+1Ch, `sup` the
                    // ticks 009C6DDA skipped the heading write, `T` the dead
                    // band as it last reached 009C6A43, `along`/`cross` the two
                    // 009C68D4 / 009C6889 distances on the last bank tick.
                    host.log.notef("  divebomb %-12s flyabove bank: bl=%d "
                        "latch=%d@%d sup=%d T=%.4f rad along=%.1f m "
                        "cross=%.1f m turn_circle=%.1f m",
                        slot->row.name.c_str(),
                        slot->db_flyabove_bl_ticks,
                        slot->db_flyabove_bank_latch_1c ? 1 : 0,
                        slot->db_flyabove_latch_tick,
                        slot->db_flyabove_latched_ticks,
                        static_cast<double>(slot->db_flyabove_dead_band_t),
                        static_cast<double>(slot->db_flyabove_along_track),
                        static_cast<double>(slot->db_flyabove_cross_track),
                        static_cast<double>(slot->plane_turn_circle_radius));
                }
                }
            }
            if constexpr (bsp::kStrafeTaskBound) {
                // Packet cc9_strafe_arm.
                int planes = 0, trans = 0, aims = 0, gaps = 0, gpt = 0, gpf = 0, hits = 0;
                int k[7] = {0, 0, 0, 0, 0, 0, 0};
                for (const auto& slot : host.slots) {
                    if (slot->st_arm_ticks == 0) continue;
                    ++planes;
                    for (int i = 0; i < 7; ++i) k[i] += slot->st_state_ticks[i];
                    trans += slot->st_transitions;
                    aims += slot->st_aim_entries;
                    gaps += slot->st_evasive_gaps;
                    gpt += slot->st_gun_point_ticks;
                    gpf += slot->st_gun_point_fires;
                    hits += slot->st_hit_resets;
                    const int* s = slot->st_state_ticks;
                    host.log.notef("  strafe %-12s ticks=%d moveto=%d follow=%d prepare=%d "
                        "gotowards=%d aim=%d goaway=%d attackrun=%d transitions=%d aims=%d "
                        "gun_point_ticks=%d gun_point_fires=%d evasive_gaps=%d hit_resets=%d",
                        slot->row.name.c_str(), slot->st_arm_ticks, s[0], s[1], s[2], s[3],
                        s[4], s[5], s[6], slot->st_transitions, slot->st_aim_entries,
                        slot->st_gun_point_ticks, slot->st_gun_point_fires,
                        slot->st_evasive_gaps, slot->st_hit_resets);
                }
                host.log.notef("summary mission strafe task: planes=%d moveto=%d follow=%d "
                    "prepare=%d gotowards=%d aim=%d goaway=%d attackrun=%d transitions=%d "
                    "aims=%d gun_point_ticks=%d gun_point_fires=%d evasive_gaps=%d "
                    "hit_resets=%d (009CD170, packet cc9_strafe_arm)",
                    planes, k[0], k[1], k[2], k[3], k[4], k[5], k[6], trans, aims, gpt, gpf,
                    gaps, hits);
                {
                    int tt = 0, ff = 0, mt = 0, pa = 0, ps = 0;
                    for (const auto& slot : host.slots) {
                        tt += slot->st_mv_tightturns;
                        ff += slot->st_mv_flikflaks;
                        mt += slot->st_mv_ticks;
                        pa += slot->st_profile_activations;
                        ps += slot->st_profile_seeds;
                    }
                    host.log.notef("summary mission strafe breakoff bound=%d tightturns=%d "
                        "flikflaks=%d manoeuvre_ticks=%d (009BC030 / 009BC0A0, packet "
                        "cc9_strafe_breakoff)", bsp::kStrafeBreakoffBound ? 1 : 0, tt, ff, mt);
                    host.log.notef("summary mission strafe profile activations bound=%d "
                        "activations=%d leader_seeds=%d (009CD020 / 009CD0AA, cc9-gunnery38 O1)",
                        GameUnitsHost::Impl::kStrafeProfileActivationSeedBound ? 1 : 0, pa, ps);
                    if constexpr (bsp::kStrafeAimPointFreezeCc13Bound) {
                        int f5d = 0, fnt = 0, frr = 0;
                        for (const auto& slot : host.slots) {
                            f5d += slot->st_aim_freeze_5d;
                            fnt += slot->st_aim_freeze_notify;
                            frr += slot->st_aim_frozen_refreshes;
                        }
                        host.log.notef("summary mission strafe aim_freeze head_5d=%d notify_5e=%d "
                            "frozen_refreshes=%d (009FADA0 / 009FAF90, packet cc13-strafeaimfreeze)",
                            f5d, fnt, frr);
                    }
                }
            }
            {
                // Packet cc9_lua52_kamikaze_wiring.
                int kz_planes = 0, kz_arm = 0, kz_inst = 0, kz_aim = 0, kz_lock = 0, kz_boost = 0;
                int dkz_planes = 0, dkz_arm = 0, dkz_inst = 0, dkz_rel = 0;
                int k[5] = {0, 0, 0, 0, 0};
                int d[7] = {0, 0, 0, 0, 0, 0, 0};
                for (const auto& slot : host.slots) {
                    if (slot->kz_ordered_thinks > 0) ++kz_planes;
                    if (slot->dkz_ordered_thinks > 0) ++dkz_planes;
                    kz_arm += slot->kz_arm_ticks;
                    kz_inst += slot->kz_installs;
                    kz_aim += slot->kz_aim_entries;
                    kz_lock += slot->kz_locks;
                    kz_boost += slot->kz_boost_ticks;
                    for (int i = 0; i < 5; ++i) k[i] += slot->kz_state_ticks[i];
                    dkz_arm += slot->dkz_arm_ticks;
                    dkz_inst += slot->dkz_installs;
                    dkz_rel += slot->dkz_releases;
                    for (int i = 0; i < 7; ++i) d[i] += slot->dkz_state_ticks[i];
                    if (slot->kz_arm_ticks > 0) {
                        const int* s = slot->kz_state_ticks;
                        host.log.notef("  kamikaze %-12s ticks=%d moveto=%d follow=%d prepare=%d "
                            "gotowards=%d aim=%d installs=%d aims=%d locks=%d boosts=%d "
                            "min_dist=%.1f", slot->row.name.c_str(), slot->kz_arm_ticks, s[0], s[1],
                            s[2], s[3], s[4], slot->kz_installs, slot->kz_aim_entries,
                            slot->kz_locks, slot->kz_boost_ticks,
                            static_cast<double>(slot->kz_min_target_dist));
                    }
                }
                host.log.notef("summary mission kamikaze task bound=%d ordered_planes=%d arm_ticks=%d "
                    "installs=%d moveto=%d follow=%d prepare=%d gotowards=%d aim=%d aims=%d "
                    "locks=%d boosts=%d (009AF510, packet cc9_lua52_kamikaze_wiring)",
                    bsp::kKamikazeTaskBound ? 1 : 0, kz_planes, kz_arm, kz_inst, k[0], k[1], k[2],
                    k[3], k[4], kz_aim, kz_lock, kz_boost);
                if constexpr (Impl::kKamikazeBoostTurboBound) {
                    host.log.notef("summary mission boost turbo steps=%llu (007BB6E0 9F8h -> "
                        "007D9077 TurboStrength, packet cc13-fix89)", host.boost_turbo_steps_);
                }
                {
                    int shape_writes[2]{};
                    for (const auto& slot : host.slots) {
                        shape_writes[0] += slot->dkz_shape_writes[0];
                        shape_writes[1] += slot->dkz_shape_writes[1];
                    }
                    if (shape_writes[0] + shape_writes[1] > 0) {
                        host.log.notef("summary mission drop kamikaze formation shape writes "
                            "shape1=%d shape3=%d (009BEDDA / 009AD1A5)", shape_writes[0],
                            shape_writes[1]);
                    }
                }
                const bsp::DummyKamikazeRounds& rounds = bsp::dummy_kamikaze_rounds();
                host.log.notef("summary mission drop kamikaze task bound=%d ordered_planes=%d "
                    "arm_ticks=%d installs=%d moveto=%d follow=%d attackrun=%d prepare=%d done=%d "
                    "release=%d goaway=%d releases=%d rounds_pushed=%zu spawned=%zu drowned=%zu "
                    "(009AE8A0 / 006FEC00, packet cc9_lua52_kamikaze_wiring)",
                    bsp::kDropKamikazeTaskBound ? 1 : 0, dkz_planes, dkz_arm, dkz_inst, d[0], d[1],
                    d[2], d[3], d[4], d[5], d[6], dkz_rel, rounds.pushed(), rounds.spawned(),
                    rounds.drowned());
            }
            {
                // Packet cc9_dogfight_task.
                std::size_t df_aircraft = 0;
                int df_moveto = 0, df_follow = 0, df_near = 0;
                for (const auto& slot : host.slots) {
                    if (!slot->dogfight_task_installed) continue;
                    ++df_aircraft;
                    df_moveto += slot->df_state_ticks[static_cast<int>(bsp::DogfightState::kMoveTo)];
                    df_follow += slot->df_state_ticks[static_cast<int>(bsp::DogfightState::kFollow)];
                    df_near += slot->df_within_attack_dist_ticks;
                    host.log.notef("  dogfight %-12s moveto=%d follow=%d transitions=%d "
                        "min_target_range=%.1f within_attack_dist_ticks=%d",
                        slot->row.name.c_str(),
                        slot->df_state_ticks[static_cast<int>(bsp::DogfightState::kMoveTo)],
                        slot->df_state_ticks[static_cast<int>(bsp::DogfightState::kFollow)],
                        slot->df_transitions,
                        static_cast<double>(slot->df_min_target_range),
                        slot->df_within_attack_dist_ticks);
                    {
                        const auto& k = slot->df_state_ticks;
                        using S = bsp::DogfightState;
                        host.log.notef("  dogfight-engaged %-12s aim=%d maneuver=%d avoid=%d "
                            "attackrun=%d prepare=%d latch_sets=%d latched_ticks=%d "
                            "reselects=%d target_changes=%d gun_window_ticks=%d target=%s",
                            slot->row.name.c_str(), k[static_cast<int>(S::kAim)],
                            k[static_cast<int>(S::kManeuver)],
                            k[static_cast<int>(S::kAvoidRoll)] + k[static_cast<int>(S::kAvoidTurn)],
                            k[static_cast<int>(S::kAttackRun)], k[static_cast<int>(S::kPrepare)],
                            slot->df_latch_sets, slot->df_latched_ticks, slot->df_reselects,
                            slot->df_target_changes, slot->df_gun_window_ticks,
                            (slot->df_target_plus_one != 0 &&
                             slot->df_target_plus_one - 1 < host.slots.size())
                                ? host.slots[slot->df_target_plus_one - 1]->row.name.c_str()
                                : "-");
                        if constexpr (GameUnitsHost::Impl::kDogfightAimAdoptsGunTargetBound) {
                            host.log.notef("  dogfight gun adopt %-12s adopts=%d",
                                slot->row.name.c_str(), slot->df_gun_adopts);
                        }
                        if constexpr (GameUnitsHost::Impl::kDogfightAimLeadBound) {
                            host.log.notef("  dogfight aim lead %-12s lead_ticks=%d mean_shift=%.2f",
                                slot->row.name.c_str(), slot->df_aim_lead_ticks,
                                slot->df_aim_lead_ticks > 0
                                    ? slot->df_aim_lead_shift_sum / slot->df_aim_lead_ticks
                                    : 0.0);
                        }
                        host.log.notef("  dogfight bodies %-12s mv_enters=%d mv_expires=%d "
                            "mv_mode_ticks=%d/%d/%d avoid_roll=%d avoid_turn=%d "
                            "avoid_latched_ticks=%d (packet cc9_dogfight_maneuver_bodies)",
                            slot->row.name.c_str(), slot->mv_enters, slot->mv_expires,
                            slot->mv_mode_ticks[0], slot->mv_mode_ticks[1],
                            slot->mv_mode_ticks[2], slot->av_roll_picks, slot->av_turn_picks,
                            slot->av_latched_ticks);
                    }
                }
                {
                    {
                        int calls = 0, hits = 0, weaves = 0, slots = 0;
                        for (const auto& slot : host.slots) {
                            calls += slot->nf_calls;
                            hits += slot->nf_hits;
                            weaves += slot->nf_attackrun_weaves;
                            slots += slot->nf_flyover_slot_writes;
                            if (slot->nf_hits == 0) continue;
                            host.log.notef("  near field %-12s calls=%d hits=%d attackrun_weaves=%d "
                                "max_offset=%.3f flyover_slot_writes=%d slot=[%.3f %.3f] goaway_hits=%d",
                                slot->row.name.c_str(), slot->nf_calls, slot->nf_hits,
                                slot->nf_attackrun_weaves,
                                static_cast<double>(slot->nf_attackrun_max_offset),
                                slot->nf_flyover_slot_writes,
                                static_cast<double>(slot->nf_flyover_slot_min),
                                static_cast<double>(slot->nf_flyover_slot_max),
                                slot->nf_goaway_hits);
                        }
                        host.log.notef("summary mission near field probe: calls=%d hits=%d "
                            "attackrun_weaves=%d flyover_slot_writes=%d", calls, hits, weaves, slots);
                    }
                    if constexpr (Impl::kTaskGunControllerAllTasksBound) {
                        int planes = 0, ticks = 0, tb = 0, tf = 0, rises = 0, enemy = 0;
                        int untasked = 0, untasked_planes = 0, cone_ticks = 0, steer_ticks = 0;
                        for (const auto& slot : host.slots) {
                            untasked += slot->tg_untasked_ticks;
                            if (slot->tg_untasked_ticks > 0) ++untasked_planes;
                            cone_ticks += slot->tg_cone_ticks;
                            steer_ticks += slot->tg_steer_ticks;
                            if (slot->dogfight_task_installed || slot->tg_ticks == 0) continue;
                            ++planes;
                            ticks += slot->tg_ticks;
                            tb += slot->df_gun.bursts;
                            tf += slot->df_gun.fire_ticks;
                            rises += slot->pg_trigger_rises;
                            enemy += slot->tg_enemy_ticks;
                            if (slot->df_gun.bursts == 0) continue;
                            host.log.notef("  task gun %-12s ticks=%d bursts=%d fire_ticks=%d "
                                "trigger_rises=%d finder_scans=%d cone_ticks=%d steer_ticks=%d",
                                slot->row.name.c_str(),
                                slot->tg_ticks, slot->df_gun.bursts, slot->df_gun.fire_ticks,
                                slot->pg_trigger_rises, slot->nb_scans, slot->tg_cone_ticks,
                                slot->tg_steer_ticks);
                        }
                        host.log.notef("summary mission task gun (all tasks): planes=%d "
                            "ticks=%d enemy_list_ticks=%d bursts=%d fire_ticks=%d trigger_rises=%d "
                            "(00999979 -> 009FC7C0, packet cc9_task_gun_controller_all_tasks)",
                            planes, ticks, enemy, tb, tf, rises);
                        host.log.notef("summary mission task gun cones: cone_ticks=%d "
                            "steer_ticks=%d untasked_planes=%d untasked_ticks=%d "
                            "(packet cc9_task_gun_cones)", cone_ticks, steer_ticks,
                            untasked_planes, untasked);
                        if constexpr (bsp::kBotCommandedDirection68Bound) {
                            // Packet cc13-steergate: gun+68h stores and the gate verdicts
                            // that differ from the forward's (to_pass / to_block).
                            int writes = 0, fallbacks = 0, gates = 0, to_pass = 0, to_block = 0;
                            for (const auto& slot : host.slots) {
                                writes += slot->sg_direction_writes;
                                fallbacks += slot->sg_heading_fallbacks;
                                gates += slot->sg_gate_dir_ticks;
                                to_pass += slot->sg_flip_to_pass;
                                to_block += slot->sg_flip_to_block;
                                if (slot->sg_flip_to_pass + slot->sg_flip_to_block == 0) continue;
                                host.log.notef("  steer gate %-12s writes=%d gates=%d to_pass=%d "
                                    "to_block=%d steer_ticks=%d", slot->row.name.c_str(),
                                    slot->sg_direction_writes, slot->sg_gate_dir_ticks,
                                    slot->sg_flip_to_pass, slot->sg_flip_to_block,
                                    slot->tg_steer_ticks);
                            }
                            for (const auto& [site, c] : host.steer_gate_sites_) {
                                host.log.notef("  steer gate site %08X writes=%d gates=%d "
                                    "to_pass=%d to_block=%d", site, c.writes, c.gate_ticks,
                                    c.to_pass, c.to_block);
                            }
                            host.log.notef("summary mission steer gate: direction_writes=%d "
                                "heading_fallbacks=%d direction_gates=%d to_pass=%d to_block=%d "
                                "gate_reads_direction=%d (009FABE0 -> gun+68h, 009FCCA0; packet "
                                "cc13-steergate)", writes, fallbacks, gates, to_pass, to_block,
                                bsp::kSteerGateCommandedDirectionBound ? 1 : 0);
                        }
                    }
                    int bursts = 0, fire_ticks = 0;
                    for (const auto& slot : host.slots) {
                        if (!slot->dogfight_task_installed) continue;
                        bursts += slot->df_gun.bursts;
                        fire_ticks += slot->df_gun.fire_ticks;
                        host.log.notef("  fighter gun %-12s bursts=%d fire_ticks=%d "
                            "c50_refreshes=%d finder_scans=%d enemy_list=%zu trigger_ticks=%d "
                            "trigger_rises=%d early_edges=%d early_asks=%d head_on_ticks=%d "
                            "head_on_throttle_min=%.3f",
                            slot->row.name.c_str(), slot->df_gun.bursts, slot->df_gun.fire_ticks,
                            slot->nb_refreshes, slot->nb_scans, slot->nb_enemy_50.size(),
                            slot->pg_trigger_ticks, slot->pg_trigger_rises, slot->df_early_edges,
                            slot->df_early_asks, slot->df_head_on_ticks,
                            static_cast<double>(slot->df_head_on_throttle_min));
                        if constexpr (Impl::kFighterFriendlyInLineBound) {
                            host.log.notef("  fighter friendly-in-line %-12s checks=%d busy_ticks=%d "
                                "friends=%zu (007B96D0 -> 007DEDB0)", slot->row.name.c_str(),
                                slot->ff_checks, slot->ff_busy_ticks, slot->nb_friend_70.size());
                        }
                        if constexpr (bsp::kDogfightFireEnvelopeBound) {
                            host.log.notef("  fighter fire envelope %-12s blocked=%d reaims=%d "
                                "hold_ahead=%d pc53_disagree=%d (009FC7C0, cc13-fire)",
                                slot->row.name.c_str(), slot->fe_blocked_ticks, slot->fe_reaims,
                                slot->fe_hold_ahead_ticks, slot->fe_pc53_disagree_ticks);
                        }
                        if constexpr (Impl::kFighterAccelTraceEvery > 0) {
                            const int n = slot->ga_acc_n > 0 ? slot->ga_acc_n : 1;
                            host.log.notef("  fighter accel %-12s ticks=%d mean_dt=%.3f "
                                "mean|a654|=%.2f max|a654|=%.2f mean|aold|=%.2f max|aold|=%.2f "
                                "arc654=%d arcold=%d mean_lead_sep=%.2f", slot->row.name.c_str(),
                                slot->ga_acc_n, slot->ga_dt_sum / n, slot->ga_a654_sum / n,
                                static_cast<double>(slot->ga_a654_max), slot->ga_aold_sum / n,
                                static_cast<double>(slot->ga_aold_max), slot->ga_arc_654,
                                slot->ga_arc_old, slot->ga_sep_sum / n);
                        }
                        host.log.notef("  fighter gun lead %-12s lead_ticks=%d arc_ticks=%d "
                            "fine_aim_ticks=%d distortion_ticks=%d damped_ticks=%d "
                            "shift_mean=%.1f shift_max=%.1f muzzle=%.1f",
                            slot->row.name.c_str(), slot->gl_lead_ticks, slot->gl_arc_ticks,
                            slot->gl_fine_aim_ticks, slot->gl_aim_distortion_ticks,
                            slot->gl_aim_damped_ticks,
                            slot->gl_lead_ticks ? slot->gl_shift_sum / slot->gl_lead_ticks : 0.0,
                            static_cast<double>(slot->gl_shift_max),
                            static_cast<double>(slot->gl_muzzle));
                        host.log.notef("  dogfight moveto %-12s speed_commands=%d speed=[%.1f %.1f] pilot_fires=%d",
                            slot->row.name.c_str(), slot->df_moveto_speed_commands,
                            static_cast<double>(slot->df_moveto_speed_commands ? slot->df_moveto_speed_min : 0.0f),
                            static_cast<double>(slot->df_moveto_speed_max),
                            slot->plane_pilot_fires_c24 ? 1 : 0);
                    }
                    {
                        int done_law = 0, done_placed = 0;
                        for (const auto& sl : host.slots) {
                            done_law += sl->db_done_law_ticks;
                            done_placed += sl->db_done_placed_ticks;
                        }
                        host.log.notef("summary mission plane follow law hold_ticks=%d flyto_ticks=%d "
                            "torpedo_follow_ticks=%d done_law_ticks=%d done_placed_ticks=%d "
                            "(009BEE30 arms, packet cc9_plane_follow_law)", host.follow_hold_ticks_,
                            host.follow_flyto_ticks_, host.follow_torpedo_follow_ticks_,
                            done_law, done_placed);
                        if constexpr (Impl::kFollowSightSearchBound) {
                            host.log.notef("summary mission follow sight searches=%llu found=%llu "
                                "held_ticks=%llu (009C211D-009C232A, packet cc13-fix89)",
                                host.follow_sight_searches_, host.follow_sight_found_,
                                host.follow_sight_held_ticks_);
                        }
                    }
                    {
                        long long gd = 0, gf = 0, gr = 0, ge = 0;
                        for (const auto& sl : host.slots) {
                            gd += sl->ga_detect_ticks; gf += sl->ga_flagged_ticks;
                            gr += sl->ga_repairs; ge += sl->ga_exempt_ticks;
                            if (sl->ga_flagged_ticks > 0) {
                                host.log.notef("  gunfire avoid %-20s detect=%d flagged=%d "
                                    "exempt=%d repairs=%d", sl->row.name.c_str(),
                                    sl->ga_detect_ticks, sl->ga_flagged_ticks,
                                    sl->ga_exempt_ticks, sl->ga_repairs);
                            }
                        }
                        host.log.notef("summary mission gunfire avoidance detect=%lld flagged=%lld "
                            "exempt=%lld repairs=%lld (009A17D0/0099EC40/0099BF30, packet "
                            "cc9_gunfire_avoidance)", gd, gf, ge, gr);
                    }
                    {
                        long long vp = 0, vs = 0, vb = 0, vt = 0, vd = 0;
                        long long tt = 0, tb = 0, tth = 0, td = 0, tw = 0;
                        for (const auto& sl : host.slots) {
                            vp += sl->va_plane_ticks; vs += sl->va_ship_ticks; vb += sl->va_bands;
                            vt += sl->va_throttle_sets; vd += sl->va_dive_ticks;
                            tt += sl->tr_ticks; tb += sl->tr_bands; tth += sl->tr_throttle_sets;
                            td += sl->tr_dive_ticks; tw += sl->tr_water_ticks;
                            if (sl->va_plane_ticks + sl->va_ship_ticks + sl->tr_ticks > 0) {
                                host.log.notef("  vehicle/terrain avoid %-20s plane=%d ship=%d "
                                    "bands=%d thr=%d dive=%d | terrain=%d bands=%d thr=%d "
                                    "dive=%d water=%d min_margin=%.1f", sl->row.name.c_str(),
                                    sl->va_plane_ticks, sl->va_ship_ticks, sl->va_bands,
                                    sl->va_throttle_sets, sl->va_dive_ticks, sl->tr_ticks,
                                    sl->tr_bands, sl->tr_throttle_sets, sl->tr_dive_ticks,
                                    sl->tr_water_ticks, static_cast<double>(sl->tr_min_margin));
                            }
                        }
                        host.log.notef("summary mission vehicle avoidance plane_ticks=%lld "
                            "ship_ticks=%lld bands=%lld throttle=%lld dive_ticks=%lld "
                            "(007DF4F0, packet cc9_pilot_vehicle_terrain_avoidance)",
                            vp, vs, vb, vt, vd);
                        if constexpr (Impl::kNeighbourClockDrawsBound) {
                            host.log.notef("summary mission neighbour clock draws seeded=%llu "
                                "(007E1E20 007E1F94/007E1FB9/007E1FE1, packet "
                                "p13_neighbour_clock_draws)", host.nb_clock_seeds);
                        }
                        if constexpr (Impl::kPlayerRoleBookkeepingBound) {
                            if (host.helm_option_on) {
                                host.log.notef("summary mission scripted helm throttle=%.3f "
                                    "rudder=%.3f transfers=%d issues=%d (BSP_PLAYER_HELM, packet "
                                    "cc9_scripted_helm_option; a scenario, not a reference)",
                                    static_cast<double>(host.helm_throttle),
                                    static_cast<double>(host.helm_rudder), host.helm_transfers,
                                    host.helm_issues);
                            }
                            host.log.notef("summary mission player roles takes=%d releases=%d "
                                "refusals=%d held_retakes=%d permission_writes=%d (00780162/00927D20/0067BB50, "
                                "packet cc9_player_role_bookkeeping)", host.role_takes,
                                host.role_releases, host.role_refusals, host.role_held_retakes,
                                host.role_permission_writes);
                            host.log.notef("summary mission helm map-up gate bound=%d map_up=%d "
                                "prepares=%llu skipped=%llu issues=%llu skipped=%llu (0064B870 "
                                "from 46h's 0064D610, occluded by 4Ch; packet cc13-helmmapgate)",
                                kHelmPrepareMapUpGateCc13Bound ? 1 : 0,
                                host.helm_screen_occluded_by_map ? 1 : 0,
                                host.helm_map_up_prepares, host.helm_map_up_skipped_prepares,
                                host.helm_map_up_issues, host.helm_map_up_skipped_issues);
                            for (const auto& sl : host.slots) {
                                bool any = sl->role_player_0184;
                                for (int i = 0; i < bsp::kUnitRoleTableEntries; ++i) {
                                    any = any || sl->current_roles_01ac[i] != 8 ||
                                          sl->role_permission_0188[i] != 9;
                                }
                                if (!any) continue;
                                char roles[64] = {}, perms[64] = {};
                                int ro = 0, po = 0;
                                for (int i = 0; i < bsp::kUnitRoleTableEntries; ++i) {
                                    ro += std::snprintf(roles + ro, sizeof(roles) - ro, "%d",
                                                        sl->current_roles_01ac[i]);
                                    po += std::snprintf(perms + po, sizeof(perms) - po, "%d",
                                                        sl->role_permission_0188[i]);
                                }
                                host.log.notef("  player roles %-24s held=%s open=%s +184h=%d",
                                    sl->row.name.c_str(), roles, perms,
                                    sl->role_player_0184 ? 1 : 0);
                            }
                        }
                        {
                            long long raises = 0;
                            for (const auto& sl : host.slots) raises += sl->yaw_gain_raises;
                            host.log.notef("summary mission plane yaw gain bc4 raises=%lld "
                                "bound=%d (0099BB00 -> 007DA92C, packet "
                                "cc9_plane_substitution_sweep)", raises,
                                Impl::kPlaneYawGainBc4Bound ? 1 : 0);
                        }
                        {
                            long long tn = 0;
                            for (const auto& sl : host.slots) tn += sl->turn_numerator_ticks;
                            host.log.notef("summary mission planner turn numerator ticks=%lld "
                                "slide_term=%d (0099E68D / 007DAA3B, packet cc9_kate_engagement)",
                                tn, Impl::kRateLawAttitudeTermsBound ? 1 : 0);
                        }
                        host.log.notef("summary mission planner yaw base zeroed_ticks=%llu "
                            "gate=%d (0099DE8A on +2CCh != 2, packet cc9_dogfight_maneuver_bodies)",
                            host.planner_yaw_base_zeroed,
                            Impl::kPlannerYawBaseModeGateBound ? 1 : 0);
                        {
                            long long calls = 0, nonzero = 0, units = 0;
                            float max_raw = 0.0f;
                            for (const auto& sl : host.slots) {
                                if (sl->torpedo_ar_probe_calls == 0) continue;
                                ++units;
                                calls += sl->torpedo_ar_probe_calls;
                                nonzero += sl->torpedo_ar_probe_nonzero;
                                if (sl->torpedo_ar_probe_max_raw > max_raw) {
                                    max_raw = sl->torpedo_ar_probe_max_raw;
                                }
                            }
                            host.log.notef("summary mission torpedo attackrun squadron probe "
                                "units=%lld calls=%lld nonzero=%lld max_raw_offset=%.4f rad bound=%d "
                                "(007F0280 mode 0 at 009D0857, packet cc9_attackrun_squadron_probe)",
                                units, calls, nonzero, static_cast<double>(max_raw),
                                Impl::kAttackRunSquadronProbeBound ? 1 : 0);
                        }
                        if constexpr (Impl::kPlannerModeCensusDiag) {
                            for (const auto& kv : host.planner_mode_census) {
                                host.log.notef("summary mission planner mode census state=%s "
                                    "mode0=%llu mode1=%llu mode2=%llu other=%llu "
                                    "(0099DE8A entry +2CCh, packet cc9_planner_heading_writes)",
                                    kv.first.c_str(), kv.second.n[0], kv.second.n[1],
                                    kv.second.n[2], kv.second.n[3]);
                            }
                        }
                        host.log.notef("summary mission terrain avoidance ticks=%lld bands=%lld "
                            "throttle=%lld dive_ticks=%lld water_ticks=%lld (0099F1C0/0099CAB0, "
                            "packet cc9_pilot_vehicle_terrain_avoidance)", tt, tb, tth, td, tw);
                    }
                    host.log.notef("summary mission plane squadron leaves=%d promotions=%d "
                        "(007BCAA0 -> 007F3970 at death, packet cc9_val_squadron_registry)",
                        host.squadron_leaves_, host.squadron_promotions_);
                    {
                        // Packet cc9_avoid_zone_registry: both heights at every
                        // 0041BC20 site, whichever the switch picks.
                        const GameAvoidZoneRegistry& reg = GameAvoidZoneRegistry::instance();
                        const int li = reg.squadron_layer_34c();
                        const bsp::TerrainGridLayerRecord* layer = reg.layer(li);
                        host.log.notef("summary mission avoid-zone layer 0041BC20: bound=%d "
                            "registry_layers=%zu scene_loads=%zu squadron_34c=%d "
                            "(n=%d cell=%.1f scale=%.4f slope=%.6f) samples=%llu "
                            "layer_nonzero=%llu layer_max=%.2f sea=[%.3f %.3f] "
                            "|layer-sea| max=%.2f mean=%.4f",
                            GameUnitsHost::Impl::kAvoidZoneLayerSampleBound ? 1 : 0,
                            reg.layer_count(), reg.scene_loads(), li,
                            layer ? layer->dimension : 0,
                            layer ? static_cast<double>(layer->cell_size) : 0.0,
                            layer ? static_cast<double>(layer->file_scalar) : 0.0,
                            layer ? static_cast<double>(layer->slope_limit) : 0.0,
                            host.az_samples, host.az_samples_layer_nonzero,
                            host.az_layer_max, host.az_sea_min, host.az_sea_max,
                            host.az_diff_abs_max,
                            host.az_samples > 0 ? host.az_diff_abs_sum /
                                static_cast<double>(host.az_samples) : 0.0);
                    }
                    {
                        // Packet cc9_units_contracts: the three contracts' census,
                        // printed in both builds.
                        std::size_t ships = 0, multi = 0;
                        const std::size_t n6 = host.world_lists.entries[6].count;
                        for (const GameUnitWorldNode* node = host.world_lists.entries[6].head;
                             node != nullptr; node = node->next) {
                            ++ships;
                            if (host.damage_section_count(node->unit->row.type_id) > 1) ++multi;
                        }
                        host.log.notef("summary mission units contracts: bound=%d "
                            "list24_size=%zu list24_pushes=%d | list6=%zu with_parts>1=%zu | "
                            "9B8h writes=%llu range=[%.3f %.3f]",
                            GameUnitsHost::Impl::kUnitsContractsBound ? 1 : 0,
                            static_cast<std::size_t>(host.world_lists.entries[24].count),
                            host.squadron_list24_pushes, n6, multi,
                            host.h9b8_writes, host.h9b8_min, host.h9b8_max);
                        static_cast<void>(ships);
                    }
                    {
                        // Packet cc9_ship_motion_tail: the tail's census, both builds.
                        std::string per;
                        for (std::size_t k = 0; k < host.slot_swap_runs_by_group.size(); ++k) {
                            char b[80];
                            std::snprintf(b, sizeof(b), "%s%zu:runs=%llu,swaps=%llu%s",
                                per.empty() ? "" : " ", k, host.slot_swap_runs_by_group[k],
                                host.slot_swaps_by_group[k],
                                host.slot_swap_gated_by_group[k] ? ",gated" : "");
                            per += b;
                        }
                        host.log.notef("summary mission ship motion tail: bound=%d swap_bound=%d "
                            "slot_swap_calls=%llu expiry_calls=%llu pairs=%llu invalid=%llu "
                            "groups=%zu [%s] post_motion_bound=%d post_motion_gate=%llu",
                            GameUnitsHost::Impl::kShipMotionTailBound ? 1 : 0,
                            GameUnitsHost::Impl::kFormationSlotSwapBound ? 1 : 0,
                            host.tail_slot_swap_calls, host.tail_expiry_calls,
                            host.slot_swap_pairs, host.slot_swap_pairs_invalid,
                            host.formation_groups.size(), per.c_str(),
                            GameUnitsHost::Impl::kShipPostMotionRepairOrder ? 1 : 0,
                            host.post_motion_gate_passes);
                        host.log.notef("summary mission ground queries: torpedo approach "
                            "segment probes 009D39D3=%llu blocked=%llu",
                            host.segment_probes, host.segment_probes_blocked);
                        {
                            // Packet cc9_terrain_segment_consumers: every 00903BC0
                            // call over the run, and branch A's probes.
                            const SceneTerrainQueryCensus& qc = scene_terrain_query_census();
                            long long wp = 0, wb = 0, wpos = 0, wneg = 0, wex = 0;
                            for (const auto& sl : host.slots) {
                                wp += sl->tr_water_probes; wb += sl->tr_water_blocked;
                                wpos += sl->tr_water_fan_pos; wneg += sl->tr_water_fan_neg;
                                wex += sl->tr_water_fan_exhausted;
                            }
                            host.log.notef("summary mission world segment 00903BC0 calls=%llu "
                                "endpoint_blocks=%llu sweep_entries=%llu sweep_blocks=%llu | "
                                "water probe bound=%d probes=%lld blocked=%lld fan_pos=%lld "
                                "fan_neg=%lld fan_exhausted=%lld (009A1420, packet "
                                "cc9_terrain_segment_consumers)",
                                qc.segment_calls, qc.segment_endpoint_blocks,
                                qc.segment_sweep_entries, qc.segment_sweep_blocks,
                                GameUnitsHost::Impl::kAvoidTerrainWaterProbeBound ? 1 : 0,
                                wp, wb, wpos, wneg, wex);
                        }
                    }
                    if constexpr (bsp::kPilotMoveToTaskBound && bsp::kMoveToTaskTickBound) {
                        std::size_t tasks = 0;
                        unsigned long long ticks = 0, changes = 0, arrivals = 0;
                        for (const auto& sl : host.slots) {
                            if (!sl->moveto_task_installed) continue;
                            ++tasks;
                            ticks += sl->moveto_ticks;
                            changes += sl->moveto_state_changes;
                            arrivals += sl->moveto_arrivals;
                            host.log.notef("  moveto task %-24s state=%d ticks=%llu arrived=%d "
                                "follow_ticks=%llu no_station=%llu "
                                "min_d=%.1f last_d=%.1f dwell=%.2f",
                                sl->row.name.c_str(), static_cast<int>(sl->moveto_state),
                                sl->moveto_ticks, sl->moveto_arrived_454 ? 1 : 0,
                                sl->moveto_follow_ticks, sl->moveto_follow_no_station,
                                static_cast<double>(sl->moveto_min_distance),
                                static_cast<double>(sl->moveto_distance_64),
                                static_cast<double>(sl->moveto_dwell_60));
                            if constexpr (bsp::kMoveToCircleSteerBound) {
                                if (sl->moveto_circle_ticks > 0) {
                                    host.log.notef("  moveto circle %-24s ticks=%llu "
                                        "no_heading=%llu side=%d r=%.1f min_d=%.1f "
                                        "max_d=%.1f last_d=%.1f (009C26D0/009FBB20, part 4)",
                                        sl->row.name.c_str(), sl->moveto_circle_ticks,
                                        sl->moveto_circle_no_heading,
                                        sl->moveto_circle_flag_18 ? 1 : 0,
                                        static_cast<double>(sl->plane_turn_circle_radius),
                                        static_cast<double>(sl->moveto_circle_min_d),
                                        static_cast<double>(sl->moveto_circle_max_d),
                                        static_cast<double>(sl->moveto_circle_last_d));
                                }
                            }
                        }
                        host.log.notef("summary mission moveto task: tasks=%zu ticks=%llu "
                            "state_changes=%llu arrivals=%llu (009C3950, packet "
                            "cc9_pilot_moveto_task part 2)", tasks, ticks, changes, arrivals);
                    }
                    if (df_aircraft > 0) {
                        host.log.notef("summary mission fighter gun: bursts=%d fire_ticks=%d "
                            "rounds=0 (the gunFire consumer is not established; "
                            "docs/DOGFIGHT_GUN.md)", bursts, fire_ticks);
                    }
                }
                if (df_aircraft > 0) {
                    host.log.notef("summary mission dogfight task: aircraft=%zu "
                        "moveto_ticks=%d follow_ticks=%d within_attack_dist_ticks=%d "
                        "(engaged: docs/DOGFIGHT_ENGAGED.md)",
                        df_aircraft, df_moveto, df_follow, df_near);
                }
            }
            if (tasked > 0) {
                host.log.notef("summary mission dive-bomb task: aircraft=%zu "
                    "releases=%d bombs_spawned=%d",
                    tasked, releases_total, spawned_total);
                if (releases_total == 0) {
                    host.log.notef("summary mission dive-bomb task: no release. "
                        "The torpedo's blocker, unit+C58h at 0099AF53, does not "
                        "apply here: 009C60F1 and 009C5777 are inside state "
                        "ticks the arm reaches through state->vtable[+Ch] at "
                        "009C884C, not through the arming loop. Read the walk "
                        "off the state ticks above: the in-range latch "
                        "approach+D0h at 009C7C31 gates entry to flyabove, "
                        "009C7EA0 gates turndown to aimdive, and the release "
                        "itself needs the aim error 009C5C9B inside the 25.0 m "
                        "at 00CE3880. The per-aircraft lines carry all three.");
                }
            }
        }

        // 009D4850 entered with tick counts, and the release requests
        // 007BBBA0 it issued. docs/TORPEDO_TASK_ARM.md.
        {
            static const char* const kTorpedoStateNames[8] = {
                "moveto", "follow", "done", "attackrun",
                "goaway", "aim", "prepare", "?"};
            std::size_t tasked = 0;
            int releases_total = 0;
            int releases_dead = 0;
            int blocked_engaged = 0;
            int blocked_arm = 0;
            int blocked_clause[3] = {0, 0, 0};
            int wing_fallthrough = 0, wing_leader_block = 0;
            for (const auto& slot : host.slots) {
                if (!slot->torpedo_task_installed) continue;
                ++tasked;
                releases_total += slot->torpedo_releases;
                releases_dead += slot->torpedo_releases_dead;
                blocked_engaged += slot->torpedo_blocked_by_engaged;
                for (int k = 0; k < 3; ++k) blocked_clause[k] += slot->torpedo_blocked_clause[k];
                wing_fallthrough += slot->torpedo_fallthrough_releases;
                wing_leader_block += slot->torpedo_leader_block_raised;
                blocked_arm += slot->torpedo_blocked_by_arm;
                char states[192];
                int used = 0;
                states[0] = '\0';
                for (int k = 0; k < 7; ++k) {
                    if (slot->torpedo_state_ticks[k] == 0) continue;
                    const int n = std::snprintf(states + used,
                        sizeof(states) - static_cast<std::size_t>(used),
                        "%s%s=%d", used == 0 ? "" : " ",
                        kTorpedoStateNames[k], slot->torpedo_state_ticks[k]);
                    if (n <= 0) break;
                    used += n;
                    if (static_cast<std::size_t>(used) >= sizeof(states)) break;
                }
                if (used == 0) std::snprintf(states, sizeof(states), "(none)");
                host.log.notef("  torpedo %-12s arm_ticks=%d transitions=%d "
                    "states[%s] releases=%d",
                    slot->row.name.c_str(), slot->torpedo_arm_ticks,
                    slot->torpedo_transitions, states, slot->torpedo_releases);
                if (slot->torpedo_command_writes > 0) {
                    host.log.notef("  torpedo %-12s command writes applied=%d (009D2761 / "
                        "009D295F-009D29B7, packet cc9_l60_torpedo_command_writes)",
                        slot->row.name.c_str(), slot->torpedo_command_writes);
                }
                if (slot->torpedo_done_enters_2530 > 0) {
                    host.log.notef("  torpedo %-12s done/prepare enter 009D2530=%d 3E8h "
                        "republish=%d (009BFDA6, packet cc9_l61_torpedo_done_formation)",
                        slot->row.name.c_str(), slot->torpedo_done_enters_2530,
                        slot->torpedo_3e8_republishes);
                }
                if (slot->torpedo_aim_enters > 0 || slot->torpedo_stale_timer_drops > 0 ||
                    slot->torpedo_done_state_drops > 0) {
                    host.log.notef("  torpedo %-12s aim enters=%d (2Ch already set=%d) "
                        "done-state drops=%d stale-timer drops=%d (packet p21_torp_enter)",
                        slot->row.name.c_str(), slot->torpedo_aim_enters,
                        slot->torpedo_aim_enters_complete_stale,
                        slot->torpedo_done_state_drops, slot->torpedo_stale_timer_drops);
                }
                if (slot->torpedo_los_calls > 0) {
                    host.log.notef("  torpedo %-12s los 007DF360 calls=%d screened=%d "
                        "a9 clears=%d zero-length=%d (packet p22_los_007df360)",
                        slot->row.name.c_str(), slot->torpedo_los_calls,
                        slot->torpedo_los_screened, slot->torpedo_los_clears,
                        slot->torpedo_los_zero_length);
                }
                if (slot->torpedo_cruise_passes > 0) {
                    host.log.notef("  torpedo %-12s cruise pass 009D4BFC=%d replans=%d "
                        "(packet p22_cruise_pass)", slot->row.name.c_str(),
                        slot->torpedo_cruise_passes, slot->torpedo_approach_replans);
                }
                if (slot->torpedo_disarmed_ticks > 0) {
                    host.log.notef("  torpedo %-12s disarmed 009D29E0: ticks=%d close=%d "
                        "min_ratio=%.3f last_8c=%.3f",
                        slot->row.name.c_str(), slot->torpedo_disarmed_ticks,
                        slot->torpedo_disarmed_close_ticks,
                        static_cast<double>(slot->torpedo_disarmed_min_ratio),
                        static_cast<double>(slot->torpedo_disarmed_blend_8c));
                }
                // The engaged pair over time: approach+8Ch (the engage
                // distance 009D4A70 sets) against approach+90h (the range
                // 009D3420 writes every tick it has a target), and the first
                // tick on which both went non-zero.
                if (slot->torpedo_first_engaged_tick >= 0) {
                    host.log.notef("  torpedo %-12s approach 009D3420: "
                        "ticks=%d no_target=%d replans=%d aim_ticks=%d | "
                        "engage 8Ch=%.1f range 90h first non-zero at tick %d, "
                        "min=%.1f last=%.1f",
                        slot->row.name.c_str(), slot->torpedo_approach_ticks,
                        slot->torpedo_approach_no_target_ticks,
                        slot->torpedo_approach_replans, slot->torpedo_aim_ticks,
                        static_cast<double>(slot->torpedo_engage_range_8c),
                        slot->torpedo_first_engaged_tick,
                        static_cast<double>(slot->torpedo_range_min),
                        static_cast<double>(slot->torpedo_range_last));
                    host.log.notef("  torpedo %-12s scan 009D37AE: runs=%d "
                        "clear_sectors=%d of 36 home_sector=%d turn_5c=%.4f rad",
                        slot->row.name.c_str(), slot->torpedo_scans_run,
                        slot->torpedo_clear_sectors_last,
                        slot->torpedo_home_sector_last,
                        static_cast<double>(slot->torpedo_turn_offset_last));
                    host.log.notef("  torpedo %-12s orders: lead=%d "
                        "issue_ticks=%d raised=%d peak_C58h=%d left=%d "
                        "arm_offers=%d blocked_0099af53=%d attack_mode_370=%d "
                        "raised_at_tick=%d drop_timer_98=%.1f",
                        slot->row.name.c_str(),
                        slot->torpedo_is_flight_lead ? 1 : 0,
                        slot->torpedo_orders_issue_ticks,
                        slot->torpedo_orders_issued,
                        slot->torpedo_peak_release_orders_c58,
                        slot->torpedo_release_orders_c58,
                        slot->torpedo_arm_offers,
                        slot->torpedo_arm_blocked_no_order_0099af53,
                        static_cast<int>(slot->torpedo_attack_mode_370),
                        slot->torpedo_attack_mode_raised_tick,
                        static_cast<double>(slot->torpedo_drop_timer));
                    host.log.notef("  torpedo %-12s issue stage 007CE9FD: "
                        "stage_ticks=%d guard_blocked=%d waiting=%d issues=%d "
                        "cleanups=%d first_issue_at_arm_tick=%d "
                        "requests_007BBBA0=%d C20h_left=%d C28h=%.3f",
                        slot->row.name.c_str(),
                        slot->torpedo_issue_stage_ticks,
                        slot->torpedo_issue_stage_guard_blocked,
                        slot->torpedo_issue_stage_waiting,
                        slot->torpedo_issue_stage_issues,
                        slot->torpedo_issue_stage_cleanups,
                        slot->torpedo_issue_first_issue_tick,
                        slot->torpedo_release_requests_007bbba0,
                        slot->torpedo_issue_requests_c20,
                        static_cast<double>(slot->torpedo_issue_interval_c28));
                    if constexpr (GameUnitsHost::Impl::kPlaneDeviceWalkBound) {
                        host.log.notef("  torpedo %-12s device walk 007CEB00: single=%d multi=%d "
                            "fired=%d walks=%d busy=%d C25h=%d", slot->row.name.c_str(),
                            slot->rack_single_count, slot->rack_multi_count,
                            slot->rack_single_fired, slot->rack_device_walks,
                            slot->rack_device_walks_busy,
                            slot->torpedo_release_pending_c25 ? 1 : 0);
                        if constexpr (GameUnitsHost::Impl::kReleaseIssueStageBound) {
                            host.log.notef("  torpedo %-12s rack 006E56F0: deferred=%d drops=%d "
                                "ammo=%d dropping=%d gate_refused=%d repeat_drops=%d "
                                "drops_leaving_rounds=%d", slot->row.name.c_str(),
                                slot->rack_requests_deferred, slot->rack_drops,
                                slot->rack_ammo, slot->rack_dropping ? 1 : 0,
                                slot->rack_gate_refused, slot->rack_repeat_drops,
                                slot->rack_drops_leaving_rounds);
                            host.log.notef("  torpedo %-12s rack tick returns: calls=%d census=%d "
                                "dead=%d idle=%d level=%d ticks_after_drop=%d last_drop=%.2f "
                                "last_tick=%.2f (GUNNERY 138)", slot->row.name.c_str(),
                                slot->rack_tick_calls, slot->rack_ret_census, slot->rack_ret_dead,
                                slot->rack_ret_idle, slot->rack_ret_level,
                                slot->rack_ticks_after_drop,
                                static_cast<double>(slot->rack_last_drop_t),
                                static_cast<double>(slot->rack_last_tick_t));
                            // Packet cc9_mavis_rack_drops.
                            std::string per_rack;
                            for (const int a : slot->rack_ammo_per_rack) {
                                if (!per_rack.empty()) per_rack += ',';
                                per_rack += std::to_string(a);
                            }
                            host.log.notef("  torpedo %-12s rack rounds: per_rack=bound=%d [%s] "
                                "authored=%d active=%d drops_unspawned=%d spawned=%d",
                                slot->row.name.c_str(),
                                GameUnitsHost::Impl::kRackRoundsPerRackBound ? 1 : 0,
                                per_rack.c_str(), slot->rack_rounds_authored,
                                slot->rack_active, slot->rack_drops_unspawned,
                                slot->torpedo_drops_spawned);
                        }
                    }
                    host.log.notef("  torpedo %-12s issue gate 007EEF40: "
                        "ctl+390h=%.4f ctl+374h=%.4f open=%d",
                        slot->row.name.c_str(),
                        static_cast<double>(slot->torpedo_issue_threshold_390),
                        static_cast<double>(slot->torpedo_armed_fraction_374),
                        slot->torpedo_issue_gate_open ? 1 : 0);
                    // 009D22FF-009D236E, the aim-complete test. clause 1 is
                    // 009D235A (F=34h > F=14h - ramp), clause 2 is 009D2368
                    // (F=18h > F=0Ch). docs/TORPEDO_AIM_TICK.md.
                    host.log.notef("  torpedo %-12s aim tick 009D15F0: "
                        "aim_complete_2Ch=%d first_true_at_aim_tick=%d "
                        "clause=%s | F34=%.2f F14=%.2f ramp=%.2f F18=%.4f "
                        "F0C=%.4f floor=%.1f | run_time_009D1360=%d "
                        "release_arm_009D2287=%d timer_009FA3A0=%.2f "
                        "timer_on=%d timer_fires=%d timer_blocked_007BB110=%d "
                        "first_fire_at_aim_tick=%d",
                        slot->row.name.c_str(),
                        slot->torpedo_aim_complete_2c ? 1 : 0,
                        slot->torpedo_aim_complete_tick,
                        slot->torpedo_aim_complete_clause == 3 ? "both"
                            : slot->torpedo_aim_complete_clause == 2 ? "turn"
                            : slot->torpedo_aim_complete_clause == 1 ? "range"
                            : "none",
                        static_cast<double>(slot->torpedo_aim_f34_threshold),
                        static_cast<double>(slot->torpedo_aim_f14_range),
                        static_cast<double>(slot->torpedo_aim_ramp),
                        static_cast<double>(slot->torpedo_aim_f18_turn),
                        static_cast<double>(slot->torpedo_aim_f0c_time),
                        static_cast<double>(slot->torpedo_aim_floor),
                        slot->torpedo_aim_run_time_updates,
                        slot->torpedo_aim_release_arms,
                        static_cast<double>(slot->torpedo_aim_timer),
                        slot->torpedo_aim_timer_enabled_0c ? 1 : 0,
                        slot->torpedo_aim_timer_fires,
                        slot->torpedo_aim_timer_blocked_007bb110,
                        slot->torpedo_aim_timer_first_fire_tick);
                    if (slot->torpedo_aim_ticks > 0) {
                        host.log.notef("  torpedo %-12s aim gates: ticks=%d state=%d lead=%d "
                            "alt=%d bank=%d cone=%d all=%d min_alt=%.1f min_f14=%.1f "
                            "fall_lead_a0=%.1f last_c64=%.4f (packet cc9_torpedo_release_orders)",
                            slot->row.name.c_str(), slot->torpedo_aim_ticks,
                            slot->torpedo_aim_gate_ticks[0], slot->torpedo_aim_gate_ticks[1],
                            slot->torpedo_aim_gate_ticks[2], slot->torpedo_aim_gate_ticks[3],
                            slot->torpedo_aim_gate_ticks[4], slot->torpedo_aim_gate_ticks[5],
                            static_cast<double>(slot->torpedo_aim_min_alt),
                            static_cast<double>(slot->torpedo_aim_min_f14),
                            static_cast<double>(slot->torpedo_aim_last_fall_lead),
                            static_cast<double>(slot->torpedo_aim_last_bank));
                    }
                    // ctl+370h. docs/TORPEDO_ATTACK_MODE.md: 009D3F69 and
                    // 009D40CB send an engaged task to prepare only at 0.
                    host.log.notef("  torpedo %-12s attack mode ctl+370h: "
                        "now=%d raised_at_tick=%d changes=%d | ticks "
                        "hold=%d attack=%d forced=%d | prepare_entries=%d "
                        "lowered_009A285E=%d message_007F0068=%d",
                        slot->row.name.c_str(),
                        static_cast<int>(slot->torpedo_attack_mode_370),
                        slot->torpedo_attack_mode_raised_tick,
                        slot->torpedo_mode_changes,
                        slot->torpedo_mode_ticks[0],
                        slot->torpedo_mode_ticks[1],
                        slot->torpedo_mode_ticks[2],
                        slot->torpedo_prepare_entries,
                        slot->torpedo_mode_lowered_009a285e,
                        slot->torpedo_mode_message_007f0068);
                    // Whether the one tick the mode is still 0 is the same tick
                    // the release-order queue unit+C58h is empty, which is what
                    // would decide the drop. 0099AF53 is the empty-queue arm.
                    host.log.notef("  torpedo %-12s prepare window: "
                        "first_prepare_at_arm_tick=%d "
                        "first_blocked_no_order_at_arm_tick=%d coincide=%s",
                        slot->row.name.c_str(),
                        slot->torpedo_prepare_first_tick,
                        slot->torpedo_blocked_no_order_first_tick,
                        (slot->torpedo_prepare_first_tick >= 0 &&
                         slot->torpedo_prepare_first_tick ==
                             slot->torpedo_blocked_no_order_first_tick)
                            ? "yes" : "no");
                    // 009D0D90 / 009D3150, the goaway break-off.
                    host.log.notef("  torpedo %-12s goaway 009D0D90/009D3150: "
                        "enters=%d break_off_24h=%.1f side_2Ch=%+.0f "
                        "range_peak_in_goaway=%.1f done_last=%d "
                        "(SafeDist 0042E740+438h=%.1f)",
                        slot->row.name.c_str(),
                        slot->torpedo_goaway_enters,
                        static_cast<double>(slot->torpedo_goaway_distance_24),
                        static_cast<double>(slot->torpedo_goaway_side_2c),
                        static_cast<double>(slot->torpedo_goaway_range_peak),
                        slot->torpedo_goaway_done_last ? 1 : 0,
                        static_cast<double>(
                            host.lua.plane_globals_loaded()
                                ? host.lua.plane_globals().pilot_torpedo_safe_dist
                                : 0.0f));
                    // cc8_torpedo_retire item 1. 009D4C10's inputs on the FIRST
                    // tick its binding returned true. arm 2 = the target pair
                    // (`target == 0 || target->+5Dh`), arm 5 = the range test
                    // (`SafeDist * ratio <= task+488h`). Nothing else can return
                    // true once base_gate is proved.
                    host.log.notef("  torpedo %-12s break-off 009D4C10 first true: "
                        "arm=%d at_arm_tick=%d true_ticks=%d state=%s "
                        "has_target=%d target_plus_one=%d target_marked=%d "
                        "in_attack=%d flag_52a=%d ordnance_132=%d "
                        "range_90=%.1f prev_range_90=%.1f limit_90_488h=%.1f "
                        "safe_dist=%.1f threshold=%.1f "
                        "approach_ticks=%d no_target_ticks=%d",
                        slot->row.name.c_str(),
                        slot->torpedo_breakoff_arm,
                        slot->torpedo_breakoff_first_tick,
                        slot->torpedo_breakoff_true_ticks,
                        kTorpedoStateNames[
                            static_cast<std::size_t>(
                                slot->torpedo_breakoff_state) & 7u],
                        slot->torpedo_breakoff_has_target ? 1 : 0,
                        slot->torpedo_breakoff_target_plus_one,
                        slot->torpedo_breakoff_target_marked ? 1 : 0,
                        slot->torpedo_breakoff_in_attack_state ? 1 : 0,
                        slot->torpedo_breakoff_attack_flag_52a ? 1 : 0,
                        slot->torpedo_breakoff_has_ordnance_132 ? 1 : 0,
                        static_cast<double>(slot->torpedo_breakoff_range_90),
                        static_cast<double>(slot->torpedo_breakoff_prev_range),
                        static_cast<double>(slot->torpedo_breakoff_limit_90),
                        static_cast<double>(slot->torpedo_breakoff_safe_dist),
                        static_cast<double>(slot->torpedo_breakoff_safe_dist),
                        slot->torpedo_breakoff_approach_ticks,
                        slot->torpedo_breakoff_no_target_ticks);
                    // cc8_torpedo_retire item 5. The run-in geometry at
                    // ATTACKRUN ENTRY, for comparison with the crossing angle
                    // the closest-approach census reports at the far end of the
                    // swim. Both target numbers are hull POSE headings, so the
                    // two are commensurable; the ship-ai step heading is a
                    // different field and is deliberately absent here.
                    // pc24-lint: allow host diagnostic (GameUnitsHost::report end-of-run log)
                    host.log.notef("  torpedo %-12s run-in at attackrun entry: "
                        "entries=%d yaw_C6C=%.4f own_pose=%.4f "
                        "target_pose=%.4f crossing=%.4f rad (%.1f deg)",
                        slot->row.name.c_str(),
                        slot->torpedo_attackrun_entries,
                        static_cast<double>(slot->torpedo_attackrun_yaw_c6c),
                        static_cast<double>(slot->torpedo_attackrun_own_pose),
                        static_cast<double>(slot->torpedo_attackrun_target_pose),
                        static_cast<double>(slot->torpedo_attackrun_crossing),
                        static_cast<double>(slot->torpedo_attackrun_crossing) *
                            180.0 / kPi);
                    // cc8_torpedo_retire item 4, observation only: the speed
                    // ratio task+41Ch = approach+24h that 009F9CE0 writes as
                    // max(1.0f, classBlock->+188h / reference_speed). The
                    // divisor is the TORPEDO row and it is not a stand-in any
                    // more: 009D03A1-009D03B7 inside BSP_BotApproachTorpedo_Reset
                    // calls 0042E740 and pushes tuning+440h
                    // (Pilot/Torpedo/ReferenceSpeed, docs/BOT_TASKS.md:246) as
                    // 009F9CE0's third argument. The numerator is MaxSpd
                    // (src/plane_class_fields.cpp, .MaxSpd -> +188h), which this
                    // host already carries as plane_max_spd.
                    {
                        const float ref = host.lua.plane_globals_loaded()
                            ? host.lua.plane_globals().pilot_torpedo_reference_speed
                            : 0.0f;
                        const float ratio = (ref > 0.0f)
                            ? bsp::bot_task_speed_ratio(slot->plane_max_spd, ref)
                            : 1.0f;
                        host.log.notef("  torpedo %-12s speed ratio 009F9CE0: "
                            "max_spd_188h=%.2f reference_speed_440h=%.2f "
                            "ratio_41Ch=%.4f break_off_threshold=%.1f "
                            "(bound=1.0)",
                            slot->row.name.c_str(),
                            static_cast<double>(slot->plane_max_spd),
                            static_cast<double>(ref),
                            static_cast<double>(ratio),
                            static_cast<double>(
                                (host.lua.plane_globals_loaded()
                                     ? host.lua.plane_globals().pilot_torpedo_safe_dist
                                     : 0.0f) * ratio));
                    }
                    // 009D0F10, bound by packet cc8_flyto_solver_and_goaway.
                    // arms: 1 = window/below 20 m (climb to 1000 on +18h),
                    // 2 = window/above 20 m (roll), 3 = post-window/high
                    // (heading +18h), 4 = post-window/low (wings level).
                    host.log.notef("  torpedo %-12s goaway 009D0F10: ticks=%d "
                        "arms[win_low=%d win_high=%d post_high=%d post_low=%d] "
                        "heading_ticks=%d climb_1Ch=%.1f known=%d high_20h=%.1f "
                        "alt_cmd=%.1f alt_range=[%.1f,%.1f]",
                        slot->row.name.c_str(),
                        slot->torpedo_goaway_ticks,
                        slot->torpedo_goaway_arm_ticks[1],
                        slot->torpedo_goaway_arm_ticks[2],
                        slot->torpedo_goaway_arm_ticks[3],
                        slot->torpedo_goaway_arm_ticks[4],
                        slot->torpedo_goaway_heading_ticks,
                        static_cast<double>(
                            slot->torpedo_goaway_runtime.climb_altitude_1c),
                        slot->torpedo_goaway_runtime.climb_altitude_known ? 1 : 0,
                        static_cast<double>(
                            slot->torpedo_goaway_runtime.high_threshold_20),
                        static_cast<double>(slot->torpedo_goaway_alt_cmd_last),
                        static_cast<double>(slot->torpedo_goaway_ticks > 0
                            ? slot->torpedo_goaway_alt_min : 0.0f),
                        static_cast<double>(slot->torpedo_goaway_ticks > 0
                            ? slot->torpedo_goaway_alt_max : 0.0f));
                } else {
                    host.log.notef("  torpedo %-12s approach 009D3420: "
                        "ticks=%d no_target=%d replans=%d aim_ticks=%d | "
                        "engage pair stays zero: 8Ch=%.1f 90h=%.1f",
                        slot->row.name.c_str(), slot->torpedo_approach_ticks,
                        slot->torpedo_approach_no_target_ticks,
                        slot->torpedo_approach_replans, slot->torpedo_aim_ticks,
                        static_cast<double>(slot->torpedo_engage_range_8c),
                        static_cast<double>(slot->torpedo_engage_limit_90));
                }
            }
            if (tasked > 0) {
                host.log.notef("summary mission torpedo task: aircraft=%zu "
                    "releases=%d blocked_engaged_009d3210=%d blocked_arm_009d49a0=%d "
                    "engaged_refusals no_target=%d member=%d range=%d",
                    tasked, releases_total, blocked_engaged, blocked_arm,
                    blocked_clause[0], blocked_clause[1], blocked_clause[2]);
                if constexpr (bsp::kTorpedoArmFallthroughReleaseBound ||
                              bsp::kReleaseOrderLeaderBlockBound) {
                    host.log.notef("summary mission torpedo wingmen: fallthrough_releases=%d "
                        "(0099B6CD) leader_block_orders=%d (007EEFDD, packet "
                        "cc9_l57_torpedo_wingmen)", wing_fallthrough, wing_leader_block);
                }
                // Packet cc9_torpedo_release_counter: `releases` above is task-level
                // and counts dead aircraft, which spawn nothing. The rounds in the
                // water are the gunnery host's `torpedo_drop drops`.
                host.log.notef("summary mission torpedo task releases: task=%d live=%d "
                    "dead=%d (dead aircraft spawn nothing, 007CEA1C; rounds in the water are "
                    "'gunnery torpedo_drop drops'; packet cc9_torpedo_release_counter)",
                    releases_total, releases_total - releases_dead, releases_dead);
                if (releases_total == 0) {
                    // With 009D3420 reconstructed the engaged pair is live, so
                    // the gate is no longer the approach update itself. Name
                    // whichever of the two clauses of 009D3210 still refuses.
                    float worst_range = -1.0f;
                    float engage = 0.0f;
                    bool any_target = false;
                    for (const auto& slot : host.slots) {
                        if (!slot->torpedo_task_installed) continue;
                        engage = slot->torpedo_engage_range_8c;
                        if (slot->torpedo_range_min >= 0.0f &&
                            (worst_range < 0.0f ||
                             slot->torpedo_range_min < worst_range)) {
                            worst_range = slot->torpedo_range_min;
                        }
                        if (slot->torpedo_approach_no_target_ticks <
                            slot->torpedo_approach_ticks) {
                            any_target = true;
                        }
                    }
                    if (!any_target) {
                        host.log.notef("summary mission torpedo task: no release. "
                            "009D3420 took its 009D3506 early-out on every tick: "
                            "the approach has no target at approach+CCh "
                            "(task+4C4h), so 009D3210 returns 0 at 009D3223");
                    } else {
                        int arm_blocked = 0;
                        int arm_offers = 0;
                        int orders = 0;
                        for (const auto& s : host.slots) {
                            if (!s->torpedo_task_installed) continue;
                            arm_blocked += s->torpedo_arm_blocked_no_order_0099af53;
                            arm_offers += s->torpedo_arm_offers;
                            orders += s->torpedo_release_orders_c58;
                        }
                        if (arm_offers == 0) {
                            host.log.notef("summary mission torpedo task: no "
                                "release. 009D3420 and 009D3210 now pass and the "
                                "rule reaches prepare. The gate is unit+C58h at "
                                "0099AF53 in BSP_PilotBot_Tick: the queued "
                                "release-order count is %d on every one of %d "
                                "ticks, so the arming loop at 0099AF81 never "
                                "offers the task its vtable +24h (009D49A0), "
                                "prepare+98h stays at %.1f and 009D2720 never "
                                "reaches 007BBBA0. The raiser of unit+C58h is "
                                "the contract this packet leaves open",
                                orders, arm_blocked,
                                static_cast<double>(-1.0f));
                        } else if (worst_range >= 0.0f && worst_range >= engage) {
                            // 009D3210 admits 2.2 times the engage distance, so
                            // the rule leaves moveto, but the in-range latch
                            // itself needs the range INSIDE it.
                            host.log.notef("summary mission torpedo task: no "
                                "release. 009D3420, 009D3210, ctl+370h and "
                                "unit+C58h all pass and the rule reaches "
                                "attackrun. The gate is the in-range latch "
                                "approach+131h (task+529h) at 009D3774: it "
                                "closes only below approach+8Ch, and the closest "
                                "any aircraft came was %.1f against an engage "
                                "distance of %.1f. Without the latch 009D4030 "
                                "step 10 never promotes attackrun to aim, and "
                                "009D49A0 only arms prepare+98h in prepare, so "
                                "the %d offers at 0099AF9B spend nothing",
                                static_cast<double>(worst_range),
                                static_cast<double>(engage), arm_offers);
                        } else {
                            int aim_total = 0;
                            for (const auto& s : host.slots) {
                                if (!s->torpedo_task_installed) continue;
                                aim_total += s->torpedo_aim_ticks;
                            }
                            int aim_done = 0;
                            for (const auto& s2 : host.slots) {
                                if (!s2->torpedo_task_installed) continue;
                                if (s2->torpedo_aim_complete_2c) ++aim_done;
                            }
                            if (aim_done == 0) {
                                host.log.notef("summary mission torpedo task: no "
                                    "release. The range reached the engage "
                                    "distance (closest %.1f against 8Ch=%.1f), "
                                    "%d offers were made at 0099AF9B and the "
                                    "rule reached aim for %d ticks, but no "
                                    "aircraft ever set the aim-complete byte "
                                    "state+2Ch at 009D236E",
                                    static_cast<double>(worst_range),
                                    static_cast<double>(engage), arm_offers,
                                    aim_total);
                            } else {
                                int goaway_done = 0;
                                float peak = -1.0f;
                                float breakoff = 0.0f;
                                for (const auto& s3 : host.slots) {
                                    if (!s3->torpedo_task_installed) continue;
                                    if (s3->torpedo_goaway_done_last) ++goaway_done;
                                    if (s3->torpedo_goaway_range_peak > peak) {
                                        peak = s3->torpedo_goaway_range_peak;
                                    }
                                    breakoff = s3->torpedo_goaway_distance_24;
                                }
                                int hold_ticks = 0;
                                int prep = 0;
                                for (const auto& s4 : host.slots) {
                                    if (!s4->torpedo_task_installed) continue;
                                    hold_ticks += s4->torpedo_mode_ticks[0];
                                    prep += s4->torpedo_prepare_entries;
                                }
                                host.log.notef("summary mission torpedo task: no "
                                    "release. 009D15F0 writes the aim-complete "
                                    "byte (%d of %zu) and 009D3150 is computed "
                                    "from goaway+24h=%.1f (true for %d, furthest "
                                    "%.1f m). The gate is ctl+370h, the attack "
                                    "mode: 009D3F69 and 009D40CB send an engaged "
                                    "task to prepare only while it is 0, and "
                                    "009D49A0 arms prepare+98h only in prepare. "
                                    "0099B774 raised it to 1 and it stayed there "
                                    "for %d of %d arm ticks; prepare was entered "
                                    "%d times. Neither route back to 0 is one a "
                                    "torpedo task runs: 009A285E needs the "
                                    "closetoship task's countdown after a Lua "
                                    "PilotStopCloseToShip, and 007F0068 needs "
                                    "the BCh message on the control block. "
                                    "closest approach %.1f against 8Ch=%.1f, %d "
                                    "offers, aim ran %d ticks",
                                    aim_done, tasked,
                                    static_cast<double>(breakoff), goaway_done,
                                    static_cast<double>(peak),
                                    arm_offers - hold_ticks, arm_offers, prep,
                                    static_cast<double>(worst_range),
                                    static_cast<double>(engage), arm_offers,
                                    aim_total);
                            }
                        }
                    }
                }
            } else {
                host.log.notef("summary mission torpedo task: no ordered aircraft "
                    "carries torpedo ordnance (kind 2Bh), so 0099A170 builds no "
                    "kind Eh task");
            }
        }
    }
    host.commands.report();
}
}  // namespace bsp::game
