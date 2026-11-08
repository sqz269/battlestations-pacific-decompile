#pragma once
// Private to the game_hosts_units*.cpp files: GameUnitsHost::Impl, GameUnitSlot and the helpers they
// share. Split out of game_hosts_units.cpp; an anonymous namespace there is `units_local` here.
// bsp_game.exe milestone 2i: the units the instantiate pass created, ticked.
//
// Milestone 2h created 32 `DestroyerGen` instances and reported that every unit
// pass of the frame and of the fixed step ticked 0 of them, because nothing
// walked them. This file is the other half of the fix: one state object per
// created unit, over which the reconstructions on main run.
//
// What runs here is recovered: 008255b0's twelve steps, 00813020's ring tick,
// 00825f20's gates and command chain with 0092d300, 0092e8c0 and 00937440,
// 0092be80's controller step and 004c0890's controlled-unit bind. What this
// file supplies, and labels, is listed in include/bsp/game_hosts_units.hpp.

#include "bsp/game_hosts_units.hpp"
#include "bsp/game_host_renderer_listener.hpp"
#include "bsp/unit_instance_subobjects.hpp"
#include "bsp/local_player_unit_lists.hpp"
#include "bsp/entity_local_matrix.hpp"
#include "bsp/cc13_left8_seg8.hpp"
#include "bsp/cc13_sweepA.hpp"
#include "bsp/game_native_geometry_globals.hpp"
#include "bsp/gameplay_settings.hpp"
#include "bsp/camera_decomposition.hpp"
#include "bsp/native_mlandfort_kind.hpp"
#include "bsp/native_mcommandbuilding_kind.hpp"
#include "bsp/cc13_seg41_land.hpp"
#include "bsp/cc13_seg49.hpp"
#include "bsp/cc13_left1.hpp"
#include "bsp/cc13_dyn1.hpp"
#include "bsp/game_unit_group_storage.hpp"
#include "bsp/native_unit_group_lookup.hpp"
#include "bsp/cc13_seg67b.hpp"
#include "bsp/cc13_seg67n.hpp"
#include "bsp/native_unit_group_direct_views.hpp"
#include "bsp/game_hosts_session_messages.hpp"
#include "bsp/plane_rack_census_admission.hpp"
#include "bsp/gameplay_seg38_cc13.hpp"
#include "bsp/view_seg14_cc13.hpp"
#include "bsp/pose_derived.hpp"
#include "bsp/material_effect_plane.hpp"
#include "bsp/scene_terrain_forwarders.hpp"
#include "bsp/gameplay_seg33_cc13.hpp"
#include "bsp/wire_units_host_switches.hpp"  // packet wire-units: kWireUnitsHostBound
#include "bsp/native_air_ops_holder_geometry.hpp"  // chain 73: 006BCC90
#include "bsp/oct18_tail_b.hpp"        // chain 73: 007B8A90 typed overload
#include "bsp/cc13_seg47.hpp"          // chain 67: 007B5BE0, 007C4830 typed overloads
#include "bsp/cc13_sweepC.hpp"         // chain 84: 006C77E0 typed overload
#include "bsp/cc15_seg48.hpp"          // chain 84: 007F1CA0 typed overload
#include "bsp/cc13_seg47b.hpp"         // chain 73: 007C1570, 007C2090 typed overloads
#include "bsp/cc13_seg48.hpp"          // chain 67: 007D7A40 typed overload
#include "bsp/cc13_seg67b.hpp"         // chain 67: 0099BB00 typed overload
#include "bsp/gameplay_seg61_62_cc13.hpp"
#include "bsp/gameplay_seg53_cc13.hpp"            // chain 369: 00852410's typed body
#include "bsp/frontend_screens_cc13.hpp"  // chain 67: 006049F0 typed overload
#include "bsp/cc13_sweepB.hpp"         // chain 67: 00604A20 typed overload
#include "bsp/native_unit_role_permission.hpp"  // chain 67: 0059BBD0 typed overload
#include "bsp/native_frame_transpose_inverse.hpp"  // chain 67: 0085DEA0
#include "bsp/strike_release_ver_cc13.hpp"
#include "bsp/native_scene_command_queue.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_scene_command_record_destroy.hpp"
#include "bsp/native_scene_command_list_clear.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/gameplay_seg41_cc13.hpp"
#include "bsp/cc13_seg29U.hpp"
#include "bsp/cc13_seg29U_trap_host.hpp"
#include "bsp/band_006b_cc13.hpp"            // packet wire-units: 006BA620
#include "bsp/options_hardware_cc13.hpp"     // packet wire-units: 00977050, 009771E0, 00977370
#include "bsp/cc13_x87_model.hpp"  // pc24w1-gunnery-2
#include "bsp/pc24_gunnery_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/aim_lead_body_axis_cc13.hpp"
#include "bsp/torpedo_candidate_gyro_heading_cc13.hpp"
#include "bsp/plane_issue_threshold_cc13.hpp"
#include "bsp/stream_set2_cc13.hpp"
#include "bsp/pc24_unitshost_cc13.hpp"
#include "bsp/hull_aim_shared_stream_cc13.hpp"  // packet cc13-hullaimrng
#include "bsp/set_speed_ctl_axis_cc13.hpp"  // packet cc13-setspeed
#include "bsp/ship_ai_switch_reach.hpp"
#include "bsp/game_hosts_ready.hpp"  // kSunkShipFlushBound, packet cc13-jm08
#include "bsp/hull_terrain_contact.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_hudgui_cc13.hpp"
#include "bsp/entity_orders.hpp"  // packet cc13-fix1: the command class category
#include "bsp/avoid_zone_draft_bodies.hpp"
#include "bsp/game_avoid_zone_runtime.hpp"
#include "bsp/submarine_model.hpp"  // packet cc9_set_submarine_depth_level
#include "bsp/cc13_sub_dive.hpp"    // packet cc13-subdive
#include "bsp/cc13_sinkrng.hpp"     // packet cc13-sinkrng
#include "bsp/cc13_leak92.hpp"      // packet cc13-leak92
#include "bsp/unit_motion.hpp"      // unit_step_towards_0042ac60, packet cc9_periscope_out
#include "bsp/game_hosts_powerups.hpp"  // packet cc13-powerup-bind
#include "bsp/game_hosts_scene_contents.hpp"  // packet cc9_submarine_seabed
#include "bsp/airfield_taxi.hpp"                 // packet cc9_land_park_taxi
#include "bsp/game_hosts_avoid_zones.hpp"
#include "bsp/game_hosts_hud_world.hpp"  // kHelmPrepareMapUpGateCc13Bound, packet cc13-helmmapgate
#include "bsp/band_007a_cc13.hpp"
#include "bsp/gameplay_seg48_cc13.hpp"
#include "bsp/plane_flight.hpp"
#include "bsp/plane_ground_ops.hpp"
#include "bsp/plane_water_law.hpp"
#include "bsp/plane_wanderer.hpp"
#include "bsp/plane_death_modes.hpp"
#include "bsp/gun_aim_terms.hpp"
#include "bsp/plane_pose_commit.hpp"
#include "bsp/camera_path_sampler.hpp"      // packet cc9_land_convoy_movement
#include "bsp/return_to_base.hpp"           // packet cc9_squadron_land_task
#include "bsp/cc13_kz_latch.hpp"            // packet cc13-kzlatch
#include "bsp/land_and_structures.hpp"      // packet cc9_land_convoy_movement
#include "bsp/cc15_land_convoy.hpp"         // packet cc13-convoy
#include "bsp/land_convoy_dead_cc13.hpp"    // packet cc13-dead
#include "bsp/cc13_seg41_land_null_host.hpp"  // packet cc13-convoy (0074CD10, 00742210)
#include "bsp/mission_camera.hpp"           // kWorldUp00f8758c
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/plane_advance_pose.hpp"
#include "bsp/plane_angular_velocity.hpp"
#include "bsp/plane_control_rate.hpp"
#include "bsp/pilot_plan_slots.hpp"
#include "bsp/dogfight_task.hpp"
#include "bsp/cc13_fire_envelope.hpp"           // packet cc13-fire (009FC7C0's envelope)
#include "bsp/dogfight_ver_cc13.hpp"
#include "bsp/near_field_probe.hpp"
#include "bsp/camera_position_modes.hpp"
#include "bsp/plane_attitude_angles.hpp"
#include "bsp/plane_ai_control.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/dive_bomb_aimdive_tail.hpp"
#include "bsp/dive_bomb_task.hpp"
#include "bsp/dive_bomb_goaway_turn.hpp"
#include "bsp/move_to_glide.hpp"
#include "bsp/target_disabled_byte_5d_cc13.hpp"
#include "bsp/leave_countdown_all_squadrons_cc13.hpp"
#include "bsp/gameplay_seg47_cc13.hpp"
#include "bsp/strafe_target_fallback_70_cc13.hpp"
#include "bsp/strafe_aim_point_freeze_cc13.hpp"
#include "bsp/torpedo_aim_tick.hpp"
#include "bsp/plane_cached_speed_b1c_cc13.hpp"
#include "bsp/follow_sight_draw_cc13.hpp"
#include "bsp/native_land_follow_parameters.hpp"
#include "bsp/torpedo_goaway_tick.hpp"
#include "bsp/plane_fly_to_solver.hpp"
#include "bsp/cc13_seg68.hpp"
#include "bsp/cc13_steer_gate.hpp"           // packet cc13-steergate (009FABE0, 009FCCA0)
#include "bsp/cc13_seg67n_switches.hpp"   // packet cc13-seg67n3 (009FAAD0 per install)
#include "bsp/torpedo_approach_update.hpp"
#include "bsp/torpedo_first_release.hpp"
#include "bsp/follow_to_body_pc24_cc13.hpp"  // packet cc13-followbody
#include "bsp/units_host_deviations_cc13.hpp"  // packet cc13-unitsdev
#include "bsp/cc13_release_timer.hpp"            // packet cc13-reltimer (009FA3A0 stubs)
#include "bsp/level_bomb_bay_cc13.hpp"           // packet cc13-lbbay (009B8450, 007CC8E0 bay test)
#include "bsp/band_0099_cc13.hpp"                // 009D49E0 / 009C8290 (cc13-reltimer2)
#include "bsp/torpedo_issue_timing.hpp"
#include "bsp/pilot_order_bindings.hpp"
#include "bsp/plane_squadron_host.hpp"
#include "bsp/plane_retreat_task.hpp"         // packet cc9_plane_retreat_task
#include "bsp/ship_nav_ver_cc13.hpp"           // packet cc9_ship_exit_zone (00826D70)
#include "bsp/command_execution.hpp"           // packet cc9_ship_exit_zone (command classes)
#include "bsp/classes_seg7_cc13.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_particle_axial_loading.hpp"
#include "bsp/plane_level_bomb_task.hpp"      // packet cc9_plane_level_bomb_task
#include "bsp/cc13_devwalk.hpp"               // packet cc13-devwalk (007C0D90 per rack)
#include "bsp/cc13_seg67b_switches.hpp"       // packet cc13-seg67b (009A7932)
#include "bsp/gameplay_seg46_cc13.hpp"        // packet cc13-para (006E3F90, 006E541F)
#include "bsp/cc13_paratrooper_round.hpp"     // packet cc13-para2 (the MParatrooper round)
#include "bsp/cc13_takeoff_notify.hpp"        // packet cc13-takeoff (007C17D0, +AA0h)
#include "bsp/squadron_spent_ordnance.hpp"    // packet cc9_squadron_spent_ordnance_rtb
#include "bsp/scene_record_side_blocks.hpp"
#include "bsp/pc24_unitshost_cc13.hpp"       // packet pc24w1-units-A (kPc24UnitsHostCc13Bound)
#include "bsp/plane_follow_law.hpp"
#include "bsp/plane_follow_hold.hpp"
#include "bsp/plane_formation.hpp"
#include "bsp/torpedo_release_orders.hpp"
#include "bsp/torpedo_task_arm.hpp"
#include "bsp/attack_commands.hpp"
#include "bsp/strafe_task.hpp"
#include "bsp/airops_ver_cc13.hpp"  // packet cc9_l57_moveto_self
#include "bsp/strike_approach_ver_cc13.hpp"  // packet cc9_l57_ctl_cruise_cap
#include "bsp/kamikaze_task_cc13.hpp"   // packet cc9_lua52_kamikaze_wiring
#include "bsp/kamikaze_task_host.hpp"
#include "bsp/plane_land_chain_cc13.hpp"  // packet cc13-landc
#include "bsp/plane_crash_sweep.hpp"      // packet cc13-cool9dc
#include "bsp/frontier_wiring_cc13.hpp"  // packet cc13-frontier
#include "bsp/cc13_lua_natives_wire.hpp"  // packet cc13-luawire 7b
#include "bsp/depth_charge_task_cc13.hpp"
#include "bsp/ordnance_kinds.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_observer_runtime.hpp"
#include "bsp/observer_edges.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/squadron_free_attack.hpp"            // packet p15_free_attack_tick
#include "bsp/recon_sensor_pass.hpp"               // packet cc13-fapick
#include "bsp/squadron_stophead_cc13.hpp"          // packet cc13-stophead
#include "bsp/squadron_leash_cc13.hpp"             // packet cc13-leash
#include "bsp/squadron_step_tail_cc13.hpp"         // packet cc13-steptail
#include "bsp/racked_round_entity_cc13.hpp"        // packet cc13-rackround
#include "bsp/squadron_ammo_type_rack_round_cc13.hpp"  // packet cc13-ammort
#include "bsp/squadron_ammo_dummy_kinds_cc13.hpp"  // packet cc13-ammodk
#include "bsp/approach_aim_freeze_cc13.hpp"  // packet cc13-aimfreeze3
#include "bsp/approach_ref_freeze_planes_cc13.hpp"  // packet cc13-aimfreezekz
#include "bsp/director_update_arms.hpp"            // packet cc13-leash: 0080DC70
#include "bsp/director_target_gate.hpp"
#include "bsp/plane_squadron_entity.hpp"
#include "bsp/world_map_bounds.hpp"
#include "bsp/gunnery_tables.hpp"  // GunneryCategory, packet cc9_plane_device_walk
#include "bsp/game_hosts_ai.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/torpedo_release_spawn.hpp"
#include "bsp/bot_tasks.hpp"
#include "bsp/moveonpath_task_cc13.hpp"
#include "bsp/leg_altitude_manager_cc13.hpp"   // packet cc13-legmgr1
#include "bsp/game_hosts_ship_ai.hpp"
#include "bsp/session_participant_pools.hpp"
#include "bsp/unit_hull_extents.hpp"

#include "bsp/camera_affine.hpp"
#include "bsp/geometry_helpers.hpp"
#include "bsp/vector_helpers.hpp"
#include "bsp/camera_projection.hpp"
#include "bsp/controlled_unit.hpp"
#include "bsp/cruise_speed_setting.hpp"
#include "bsp/dyn_world_settings.hpp"
#include "bsp/lua_binding_navigator.hpp"
#include "bsp/ocean_height.hpp"
#include "bsp/ocean_wave_field.hpp"
#include "bsp/ocean_spectrum.hpp"
#include "bsp/ship_hydro_forces.hpp"
#include "bsp/ship_buoyancy_elements.hpp"
#include "bsp/unit_death_sink.hpp"
#include "bsp/gun_fire_points.hpp"
#include "bsp/geom_mesh_resource.hpp"  // GeomMeshResourcePayload, cc9_ship_section_points
#include "bsp/opportunistic_rocket_cc13.hpp"
#include "bsp/rocket_task_cc13.hpp"         // packet cc9-rocket1, the rocket task 007B6A40
#include "bsp/rocket_target_stop_cc13.hpp"  // cc13-rocketstop, task+38Ch per task
#include "bsp/free_attack_accept_test_cc13.hpp"  // cc13-tranche2 S18
#include "bsp/plane_task_reach_sw3.hpp"
#include "bsp/wingmen_recount_countdown_cc13.hpp"  // cc13-tranche2 S19
#include "bsp/near_field_probe_cached_list_cc13.hpp"  // cc13-tranche2 S22
#include "bsp/attack_target_classify.hpp"
#include <array>
#include "bsp/pose_refresh.hpp"
#include "bsp/rigid_body_integration.hpp"
#include "bsp/ship_ai_path_cursor.hpp"
#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/ship_ai_nav_block_ctor.hpp"
#include "bsp/ship_ai_wake_trail.hpp"
#include "bsp/native_unit_wake_copy.hpp"
#include "bsp/ship_ai_rudder_hop.hpp"
#include "bsp/unit_group_slot_swap.hpp"
#include "bsp/ship_ai_path_corridor.hpp"  // ShipAiUnitGroupMember, the 34h record
#include "bsp/ship_ai_formation.hpp"      // ship_ai_formation_reshape (0070EFD0)
#include "bsp/ship_class_fields.hpp"
#include "bsp/ship_hull_body.hpp"
#include "bsp/mmod_hull_convex_box.hpp"
#include "bsp/ship_motion.hpp"
#include "bsp/ship_motion_ver_cc13.hpp"         // packet cc9_tboat_rudder_torque (00937440)
#include "bsp/unit_controller.hpp"
#include "bsp/unit_forces.hpp"
#include "bsp/air_operations.hpp"
#include "bsp/unit_instance.hpp"
#include "bsp/unit_instance_layout.hpp"
#include "bsp/unit_kind_query.hpp"
#include "bsp/unit_world_registration.hpp"
#include "bsp/unit_generic_input_phase.hpp"
#include "bsp/tick_element_overrides.hpp"
#include "bsp/unit_order_record.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/unit_rudder_curve.hpp"
#include "bsp/unit_instance_step11.hpp"
#include "bsp/unit_state_message.hpp"
#include "bsp/approach_target_ref.hpp"
#include "bsp/vehicle_class.hpp"
#include "bsp/world_ocean.hpp"
#include "bsp/world_construct.hpp"
#include "bsp/plane_class_fields.hpp"
#include "bsp/cc13_jm08_switches.hpp"
#include "bsp/cc13_torp_goaway.hpp"           // packet cc13-goaway (009D0D90 draws, 00901610)
#include "bsp/follow_release_seams_cc13.hpp"
#include "bsp/sw1b_switch_reach.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_shipai_cc13.hpp"           // pc24w1-sweep2-3 (0074F490)
#include "bsp/pc24_flightrender_cc13.hpp"     // pc24w1-sweep1-3 (007DB4D0)
#include "bsp/pc24_planetasks_cc13.hpp"       // pc24w1-sweep3-5 (sites at each body)
#include "bsp/flyabove_lead_bearing_pc24_cc13.hpp"  // cc13-flybearing
#include "bsp/dive_flyabove_avoid_increment_cc13.hpp"  // cc13-flyavoid, 009C6D59
#include "bsp/session_context_cc13.hpp"  // cc13-mp1 (MP2a)

#include <cstddef>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <cctype>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsp::game {
inline namespace units_local {

// Goal 4 chain 467. 009F7D68 calls 004684F0 with the command building resolved
// by this live squadron's target. The constructor reads only its identity/ID
// and F87574/78/7C, then produces the descriptor issued at 009F7D75. Borrow the
// actual host identity; it is never interpreted as a native unit allocation.
inline void construct_host_unit_reference(bsp::SceneCommandTarget& target,
    const void* identity, std::uint16_t object_id) {
    struct Inputs final : bsp::cc13_sweepA::UnitReferenceHost {
        const void* identity;
        std::uint16_t object_id;
        std::array<float, 3> origin;

        Inputs(const void* unit, std::uint16_t id) : identity(unit), object_id(id) {
            // Read the process's canonical mutable cells at the call boundary,
            // including any published changes; do not substitute constant zero.
            const auto& bits = game_native_geometry_globals().zero_vector_00f87574();
            static_assert(sizeof(origin) == sizeof(bits));
            std::memcpy(origin.data(), bits.data(), sizeof(origin));
        }
        void* global_address(std::uint32_t address) override {
            if (address != 0x00f87574u)
                throw std::logic_error("unit reference requested an unrelated global");
            return origin.data();
        }
        std::uint16_t unit_object_id_174(const void* unit) override {
            if (unit != identity)
                throw std::logic_error("unit reference requested an unrelated identity");
            return object_id;
        }
    } inputs(identity, object_id);
    static_assert(sizeof(bsp::SceneCommandTarget) == 0x18);
    static_assert(offsetof(bsp::SceneCommandTarget, object_id) == 2);
    static_assert(offsetof(bsp::SceneCommandTarget, object) == 4);
    static_assert(offsetof(bsp::SceneCommandTarget, position) == 8);
    static_assert(offsetof(bsp::SceneCommandTarget, trailing) == 0x14);
    bsp::cc13_sweepA::unit_reference_construct_004684f0(
        {reinterpret_cast<std::uint8_t*>(&target)}, const_cast<void*>(identity), inputs);
}

// Chain 273: projections of the host's existing rack fields, not a second rack
// simulation. Only the fields read/written by these two leaf bodies are bound.
// Evidence: 006E3550..006E3567 is FLD +494h / FADD delay / FSTP +494h,
// with +498h set; 006E3CD0..006E3CD6 returns that same busy byte.
template <class Busy>
void fire_host_single_rack(float& to_repeat, Busy& dropping, float delay) {
    bsp::gameplay_seg33_cc13::SingleRackState rack;
    rack.to_repeat_494 = to_repeat;
    rack.drop_bombs_498 = dropping != 0;
    bsp::gameplay_seg33_cc13::rack_fire_006e3550(rack, delay);
    to_repeat = rack.to_repeat_494;
    dropping = static_cast<Busy>(rack.drop_bombs_498);
}

inline bool host_single_rack_busy(bool dropping) {
    bsp::gameplay_seg33_cc13::SingleRackState rack;
    rack.drop_bombs_498 = dropping;
    return bsp::gameplay_seg33_cc13::rack_is_busy_006e3cd0(rack);
}

constexpr double kPi = 3.14159265358979323846;

// Packet cc9_party_change_gunnery_side (SHIP_AI 213), OFF until paired: after a
// party write (set_unit_side_0054), re-read unit+54h into the unit's gunnery row
// so the gunnery side matches the image's single unit field.
constexpr bool kPartyChangeRefreshesGunneryBound = true;  // ON: SHIP_AI 213, cc9-ships44\local\s44_pr{off,on}_* gameplay-identical; faithful by construction (the image reads +54h live)

// SUBSTITUTION, labelled: Pilot/Torpedo/ReferenceSpeed, the divisor of
// 009F9D30's run-profile speed ratio. The PilotBot tuning block is not
// reachable from this host, so the value is the authored one recorded against
// offset +440h in include/bsp/bot_tasks.hpp:266, KMH(300).
constexpr float kTorpedoReferenceSpeedAuthored = 83.333336f;

// Packet cc13-powerup-bind: 0080FC30's scale, 0080FC4F's product over category
// 4 (SHIP_SPEED) behind the gate at 0080FC31..0080FC4A, else the 1.0f at
// 00D7A24C (0080FC6A). The body multiplies max_speed by it once (0080FC5F).
inline float unit_reference_speed_scale_0080fc30(std::size_t index) {
    return game_powerups::powerup_site_product(4, index).value_or(
        bsp::kUnitReferenceSpeedUnscaled);
}


// Dispatch coverage only; direct_ship_body retains the existing partial
// 00825F20 reconstruction described in SHIP_MOTION.md.
enum class UnitMotionCoverage {
    unresolved,
    generic_guarded,
    direct_ship_body,
    ship_base_fragment,
};

struct UnitMotionDispatch {
    std::uint32_t creator{};
    std::uint32_t tick_vtable{};
    std::uint32_t entry{};
    UnitMotionCoverage coverage{UnitMotionCoverage::unresolved};

    bool runs_ship_base() const noexcept {
        return coverage == UnitMotionCoverage::direct_ship_body
            || coverage == UnitMotionCoverage::ship_base_fragment;
    }
};

// Actual leaf constructor stores at unit+310h, then that table's +8h slot.
// Select through the existing descriptor allocator identity, not MaxSpeed or
// the navigation-controller classifier. These are evidence addresses, not
// callable vtables in this process. See SHIP_MOTION_NONFINITE_ORIGIN.md.
constexpr UnitMotionDispatch kUnitMotionDispatches[] = {
    {0x006fe590, 0x00cfc38c, 0x00825f20, UnitMotionCoverage::direct_ship_body},
    {0x006fb430, 0x00cfb6f0, 0x00825f20, UnitMotionCoverage::direct_ship_body},
    {0x0074be00, 0x00cff9ec, 0x00749b20, UnitMotionCoverage::ship_base_fragment},
    {0x006eb290, 0x00cfa730, 0x00825f20, UnitMotionCoverage::direct_ship_body},
    {0x006dfef0, 0x00cf9068, 0x00825f20, UnitMotionCoverage::direct_ship_body},
    {0x008531a0, 0x00d0bf3c, 0x00855420, UnitMotionCoverage::ship_base_fragment},
    {0x00857e20, 0x00d0c604, 0x00825f20, UnitMotionCoverage::direct_ship_body},
    {0x00758d30, 0x00d015e8, 0x00758270, UnitMotionCoverage::ship_base_fragment},
    {0x008091d0, 0x00d0002c, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x0084ca50, 0x00d0ba3c, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x0074e540, 0x00d002c4, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x007ddae0, 0x00d068dc, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x00956390, 0x00d19ce4, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x009564e0, 0x00d19fbc, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x00956240, 0x00d1a294, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x007d7850, 0x00d065f4, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x006d3110, 0x00cf8bc0, 0x006d2510, UnitMotionCoverage::unresolved},
    {0x00848380, 0x00d0b728, 0x00846320, UnitMotionCoverage::unresolved},
    {0x0074df10, 0x00cffd9c, 0x00953cc0, UnitMotionCoverage::generic_guarded},
    {0x00747000, 0x00cff3b4, 0x00953cc0, UnitMotionCoverage::generic_guarded},
    {0x006f5c10, 0x00cfafe0, 0x00953cc0, UnitMotionCoverage::generic_guarded},
};

inline UnitMotionDispatch unit_motion_dispatch(const bsp::VehicleClassDescriptorRow* descriptor) {
    if (descriptor != nullptr) {
        for (const UnitMotionDispatch& dispatch : kUnitMotionDispatches) {
            if (dispatch.creator == descriptor->allocate_instance) return dispatch;
        }
    }
    return {};
}

// The stationary prop, which has no vehicle-class descriptor and so no creator
// to key on. 004F0FB0 takes it at 004F0FFE when the scene property `Stationary`
// is set: 00748C40 allocates 1ACh bytes and 00748A40 constructs them, storing
// 00CFF678 at this+0 and 00CFF65C at this+10h (00748A64 and 00748A6A). Those are
// the same two slots the keyed rows in src/native_unit_observer_endpoint.cpp
// take, which 00745940 shows for LandFort at 0074597D and 00745983 against its
// row {00747000, 00CFF3F8, 00CFF3E0}. The prop is 1ACh bytes with no slot at
// 310h, so unlike a fort it carries no tick vtable and takes no motion dispatch.
//
// This lives here rather than beside the keyed rows because that file and its
// header are Codex-lineage and we do not edit them.
// docs/SCENE_STATIONARY_UNITS.md.
inline void publish_stationary_prop_observer_tables_00748a40(
    bsp::NativeUnitObserverPrefixStorage& unit) noexcept {
    unit.observed_00.native_vtable_00 = 0x00cff678u;
    unit.callback_10.native_vtable_00 = 0x00cff65cu;
}

inline const char* unit_motion_coverage_name(UnitMotionCoverage coverage) {
    switch (coverage) {
    case UnitMotionCoverage::direct_ship_body: return "direct_ship_body";
    case UnitMotionCoverage::ship_base_fragment: return "ship_base_fragment";
    case UnitMotionCoverage::generic_guarded: return "generic_guarded";
    default: return "unresolved";
    }
}

inline float heading_degrees_of(const bsp::ShipMotionState& state) {
    return static_cast<float>(std::atan2(static_cast<double>(state.pose_row2[0]),
                                  static_cast<double>(state.pose_row2[2]))
        * 180.0 / kPi);
}

// `<x>,<z>`, the point form of `--order moveto:<target>`. Both halves must be
// numbers and nothing may follow them, so an entity name can never be mistaken
// for a point. An optional leading `@` is accepted for readability.
inline bool parse_order_position(const std::string& text, float& x, float& z) {
    std::size_t begin = 0;
    if (begin < text.size() && text[begin] == '@') ++begin;
    const std::size_t comma = text.find(',', begin);
    if (comma == std::string::npos || comma == begin) return false;
    const std::string first = text.substr(begin, comma - begin);
    const std::string second = text.substr(comma + 1);
    if (second.empty()) return false;
    char* end = nullptr;
    const double parsed_x = std::strtod(first.c_str(), &end);
    if (end == nullptr || *end != '\0') return false;
    const double parsed_z = std::strtod(second.c_str(), &end);
    if (end == nullptr || *end != '\0') return false;
    x = static_cast<float>(parsed_x);
    z = static_cast<float>(parsed_z);
    return true;
}

}  // namespace units_local  // namespace

// ---------------------------------------------------------------------------
// One created unit
// ---------------------------------------------------------------------------

// How many stand-in buoyancy elements a hull gets. The native count is
// (class+530h - class+52Ch) / 24h and is authored data this process does not
// have; eight is the probe's own --hydro-elements default, so the two sides of
// the comparison use the same number.
constexpr int kBuoyancyElementCount = 8;

// Packet cc13-luawire 7b, under kLuaSetCheatTurboBound: 0099D479-0099D4F2 on the
// plane's CheatTurbo float +340h (0 when the switch is off, which makes every
// result below the identity). dtScale = float(+340h x 0.4) (00CE65D0, a double),
// raised to 1.0 when 1.0 is greater (FCOMI/JBE, so a NaN is kept);
// [ESP+28h] = float(1 / dtScale) (FDIV at 24-bit precision); the dt argument slot
// becomes float([ESP+28h] x dt) (0099D4EE).
inline float cheat_turbo_value_340(float stored) {
    return bsp::kLuaSetCheatTurboBound ? stored : 0.0f;
}
inline float cheat_turbo_dt_scale_0099d479(float stored) {
    const float t = static_cast<float>(
        static_cast<double>(cheat_turbo_value_340(stored)) * 0.4000000059604645);
    return 1.0f > t ? 1.0f : t;
}
inline float cheat_turbo_inverse_0099d4b3(float stored) {
    return 1.0f / cheat_turbo_dt_scale_0099d479(stored);
}
inline float cheat_turbo_scaled_dt_0099d4ee(float stored, float dt) {
    return cheat_turbo_inverse_0099d4b3(stored) * dt;
}

struct GameUnitSlot;

// Persistent list/node storage for the existing mission world registry.
// These triples reproduce the fields consumed by00484540 and009F1877;
// the enclosing C++ owner is not a raw4BCh native world object.
struct GameUnitWorldNode {
    GameUnitWorldNode* previous{};
    GameUnitWorldNode* next{};
    GameUnitSlot* unit{};
};
struct GameUnitWorldList {
    std::uint32_t count{}; //004B7EC0 clears all three words
    GameUnitWorldNode* head{};
    GameUnitWorldNode* tail{};
};
static_assert(sizeof(GameUnitWorldNode) == 0x0c);
static_assert(offsetof(GameUnitWorldNode, unit) == 8);
static_assert(sizeof(GameUnitWorldList) == 0x0c);
struct GameUnitWorldLists {
    GameUnitWorldList entries[bsp::kWorldSlotCount];
    GameUnitWorldLists() = default;
    GameUnitWorldLists(const GameUnitWorldLists&) = delete;
    GameUnitWorldLists& operator=(const GameUnitWorldLists&) = delete;
    // Process resource cleanup. This does not claim native world teardown,
    // observer notification or entity detach callback ordering.
    ~GameUnitWorldLists() {
        for (GameUnitWorldList& list : entries) {
            while (list.head != nullptr) {
                GameUnitWorldNode* node = list.head;
                list.head = node->next;
                delete node;
            }
        }
    }
};

// Packet cc9_squadron_ordnance_state: one bomb platform of an aircraft, in
// BSPGun order: the projectile class id it carries (0 when not a bomb-family
// type), single or multi, and its index among the single racks.
struct PlaneOrdnanceRack {
    int class_id{0};
    int platform_key{-1};     // identity of the default-device metadata
    int device_class{-1};
    bool multi{false};
    int single_index{-1};
    int authored_ammo{-1};   // Equipments[DefaultEquipment][key].Ammo, -1 absent
    // Packet cc9_ai_plane_loadout_arm: Equipments[1][key].Ammo, -1 absent. The
    // scene generator writes Equipment 1 when a squadron authors none and the
    // class lists any (0094BD34), so this is a scene squadron's loadout.
    int generator_ammo{-1};
};

// Adapter for the single-threaded landing-assignment snapshot. The body can
// only report an invalid index: there is no host lock at block+8h. All other
// segment services trap rather than provide unrelated placeholder behavior.
class LandingAssignmentLookupHost : public bsp::cc13_seg29U::Seg29UTrapHost {
};

// Goal 2 chain 209 (kWireGoal2Chain209Bound): the squadron block's byte +3AEh.
// Producers in the image (a disp-3AEh byte scan of .text): 0 from the memset of
// the block's new(414h); the Lua native 008A28B0 SquadronSetForceRelease, MOV
// [ESI+3AEh], BL at 008A29FC (lua_squadron_set_force_release). The two other
// byte writers, 0079C23E and 007A91D1, are MovCam's own +3AEh. Readers:
// 009B5E3C / 009B63EE (aim tick 009B5C80), 009B7B22 (009B7B10), 009B7B4C
// (009B7B40). Lifetime: the host slot's, as the block's (created with the
// squadron, never cleared). OFF: an empty base, so the slot's layout is unchanged.
template <bool Bound> struct SquadronForceReleaseByte {
    // Only the ON arms call these; they are discarded statements here.
    bool force_release_3ae() const { return false; }
    void set_force_release_3ae(bool) {}
};
template <> struct SquadronForceReleaseByte<true> {
    bool force_release_3ae() const { return sq_force_release_3ae; }
    void set_force_release_3ae(bool on) { sq_force_release_3ae = on; }
    bool sq_force_release_3ae{false};   // +3AEh, memset 0 at new(414h)
};

// The factory requests the image's full 520h allocation; only the typed task
// projection lives at its start. Pair scalar allocation with unsized deletion.
struct GameMoveOnPathTaskDeleter {
    void operator()(bsp::MoveOnPathTask* task) const noexcept {
        task->~MoveOnPathTask();
        ::operator delete(task);
    }
};
using GameMoveOnPathTaskOwner = std::unique_ptr<bsp::MoveOnPathTask, GameMoveOnPathTaskDeleter>;

struct GameUnitSlot : SquadronForceReleaseByte<true> {
    GameUnitSlot() {
        // Partial projections of the two actual bases, not a whole unit ctor.
        bsp::initialize_unit_observed_prefix_00925cff(observer_prefix);
        bsp::initialize_unit_callback_prefix_00925d13(observer_prefix);
        bsp::publish_scene_observer_tables_00925d44(observer_prefix);
        //00928713..00928748: current assignments, not the +188h policy table.
        for (std::int32_t& role : current_roles_01ac) role = bsp::kUnitRoleTableFill;
        // 00928630 calls instance->vtable[148h](1FFh, 9) after +180h = 9: every
        // role opens to PLAYER_ANY (docs/UNIT_INSTANCE_LAYOUT.md).
        for (std::int32_t& p : role_permission_0188) p = 9;
    }

    GameUnitRow row;
    bsp::NativeUnitObserverPrefixStorage observer_prefix;
    bool observer_prefix_ready{false};
    // 00925CE0 constructs +BDh as zero; 00923840 publishes one after +A0
    // initialization. This is not the independently mutable active byte.
    std::uint8_t message_ready_00bd{0};
    // The unit took 004F0FB0's stationary arm: no vehicle-class descriptor, so
    // no creator to key the observer tables on, and no motion dispatch either
    // because the 1ACh prop has no slot at 310h.
    // docs/SCENE_STATIONARY_UNITS.md.
    bool stationary_prop{false};
    UnitMotionDispatch motion_dispatch;
    GameUnitWorldLists* world_parent_0030{};
    std::size_t process_index{}; // metadata, not a native unit field
    // This unit owns the canonical current roles for every process consumer.
    // Actual 4Bh receive-side assignment and its side effects remain pending.
    std::int32_t current_roles_01ac[bsp::kUnitRoleTableEntries];
    // Packet cc9_player_role_bookkeeping (docs/SCRIPTED_HELM.md): the
    // permission words unit+188h + role*4 (00927D20) and unit+184h, which only
    // an accepted role-1 take sets (00780214).
    std::int32_t role_permission_0188[bsp::kUnitRoleTableEntries];
    bool role_player_0184{false};

    // Only constructor-established cells used by 00953CC0 / 0095DC40.
    // The role and gate members of the pure routine's view are not owners.
    std::int32_t generic_notify_528{-9};       //0095CDC1
    std::int32_t generic_flag_634{0};          //0095CE0B
    float generic_timer_6f8{0.0f};             //0095CF50
    float generic_timer_6fc{0.0f};             //0095CF58
    bool generic_suppress_520{false};         //0095CDD7
    // Chain 794: the +2 byte of BOTH plane +9C0h stride-8 publications.
    // 007CDCD0 copies the real +520h to the write parity at the end of the
    // plane step; 0099C270 reads the opposite parity. The host pass counter
    // chooses the two slots; their absolute numbering is immaterial. Both
    // start with the constructor's false +520h (0095CDD7 / 007CDFFF).
    std::array<std::uint8_t, 2> plane_published_suppress_9c2{};

    // Plane control state. 007CFD20 zeroes unit+900h (XOR EBX,EBX); the
    // free-flight arm needs 7, which 007C6340's fall-through sets at 007C6481
    // alongside unit+908h = 3600. All three inputs of select_motion_arm_007ce040
    // derive from this one field: the gate (*(unit+72Ch))->vtable[+38h] is
    // BSP_PlaneControlMode_IsFreeFlight, `+1D4h == 7`, and 310h+41Ch+1D4h = 900h.
    // docs/PLANE_FLIGHT_CORE_LAW.md, docs/PLANE_UNIT_TICK.md.
    std::int32_t plane_control_mode_900{0};
    float plane_airborne_908{0.0f};
    bool plane_airborne_frozen_9e0{false};
    // Packet cc9_plane_touchdown (kPlaneTouchdownBound). 007CFD20 leaves +C04h
    // 0.0 (007D0006, XORPS at 007CFF94), +BF4h null, +BF8h/+C02h clear (007D01A7-
    // 007D01B3), +BFCh 1000.0 (007D01EB, 00CE3804) and +C18h 5 (007D000E).
    float plane_site_timer_c04{0.0f};         // the 006C0840 re-probe countdown
    std::size_t plane_contact_deck_bf4{0};    // the holder, as a landing deck index + 1
    bool plane_ground_contact_bf8{false};     // 006BC530: over the runway rectangle
    float plane_contact_height_bfc{1000.0f};  // 006BC530's local height
    bool plane_near_site_c02{false};          // the site key below 1000.0
    bool plane_landed_904{false};             // 007CB6xx: airborne clock above 5.0
    // Packet cc9_plane_ground_roll: classDesc+1E0h WheelBrake, dyn+7Ch..84h (the
    // body acceleration the core law copies to ctl+60h..68h), and counters.
    float plane_wheel_brake_1e0{0.0f};
    // classDesc+200h GroundPitch, written with WheelHeight (007D29B8-007D2AC6).
    float plane_ground_pitch_200{0.0f};
    unsigned long long ground_level_steps{0};
    float plane_body_accel_7c[3]{0.0f, 0.0f, 0.0f};
    unsigned long long ground_arm_steps{0};
    unsigned long long ground_law_steps{0};
    unsigned long long ground_free_steps{0};
    unsigned long long ground_band_steps{0};
    unsigned long long ground_hold_down_steps{0};
    unsigned long long ground_contact_steps{0};
    unsigned long long ground_liftoff_requests{0};
    float ground_last_brake{0.0f};
    float ground_last_friction{0.0f};
    // Packet cc13-takeoffdiag, log-only: the last ground-band step's lateral term
    // and thrust (007DBEB3 / 007DB744), and the takeoff decel log's state.
    float ground_last_lateral{0.0f};
    float ground_last_thrust{0.0f};
    float takeoff_decel_prev_v{-1.0f};
    std::uint32_t takeoff_decel_lines{0};
    float ground_first_pos[3]{0.0f, 0.0f, 0.0f};
    unsigned long long ground_steer_steps{0};   // cc9_plane_ground_steering
    float ground_steer_last_f2{0.0f};
    float ground_stop_time{-1.0f};
    float ground_stop_heading{0.0f};            // cc9_plane_ground_pose
    float ground_min_bfc{1000.0f};
    std::int32_t plane_c18{5};
    float plane_wheel_height_1fc{0.0f};       // classDesc+1FCh WheelHeight
    float plane_min_water_spd_198{0.0f};      // classDesc+198h MinWaterSpd
    float plane_max_water_spd_19c{0.0f};      // classDesc+19Ch MaxWaterSpd
    // unit+911h, the "splash" latch: set once a dead aircraft has raised
    // "splash" (007CC418 in the free-flight arm, 007CBBE0 at the water
    // surface), so it raises it once. Goal 5 chain 645.
    bool plane_splashed_911{false};
    // DIAGNOSTIC counters for the probe and the touchdown test.
    unsigned long long td_probes{0}, td_contact_steps{0}, td_low_steps{0};
    unsigned long long td_refused_airborne{0}, td_refused_vy{0}, td_touchdowns{0};
    float td_min_height{1000.0f};
    double td_at{-1.0};
    // Packet cc9_carrier_landing_deck_part2 (kCarrierDeckParentBound): the scene
    // parent unit+3Ch after 007C71E0's re-parent to a carrier (slot index + 1,
    // 0 = none), and the pose kept in the carrier's frame between steps
    // (unit+74h under the parent): position, the three pose rows, and the
    // carrier-relative velocity (ctl+18h after 007D9CE0), all in carrier axes.
    std::size_t deck_parent_plus_one{0};
    float deck_local_pos[3]{0.0f, 0.0f, 0.0f};
    float deck_local_rows[9]{};
    float deck_local_vel[3]{0.0f, 0.0f, 0.0f};
    bool deck_stop_logged{false};
    // ctl+ACh (unit+B5Ch), the arrestor wire: seeded by 007DB630 at a carrier
    // touchdown, stepped by 007DBEEE-007DC088, consumed at 007DC1A7.
    float plane_wire_ac{0.0f};
    // Free-flight motion. 007DB680 is an ACCUMULATOR pass, not an integrator:
    // it leaves four accumulators for 007D8470 to fold, and the caller applies
    // the result. The integration below is therefore the host's, not a
    // reconstruction of native code. docs/PLANE_FREE_FLIGHT_PHYSICS.md.
    float plane_world_velocity[3]{0.0f, 0.0f, 0.0f};
    float plane_lost_drag_timer_c3c{0.0f};
    bool plane_velocity_seeded{false};
    // Packet cc9_submarine_dive: the submarine's depth state (00853630 seeds
    // it; 00936DC0 steps it). bands are +1200h..+120Ch.
    bool sub_dive_ready{false};
    // Packet cc13-subdive: the class test of 00853630 stage 1 and 008528B0
    // (class+510h KamikazeDamage or +514h KamikazeBlastDamage above 0), read
    // once at the seed under kCc13SubDiveKamikazeBound; false otherwise.
    bool sub_kamikaze_class{false};
    float sub_bands[4]{0.0f, -20.0f, -40.0f, -80.0f};
    float sub_up_speed{1.2f};
    float sub_down_speed{1.2f};
    float sub_stop_time{5.0f};
    float sub_accel{0.25f};
    bool sub_seed_126c{false};
    bool sub_at_depth_1278{false};
    float sub_sink_1270{0.0f};
    float sub_sink_1274{0.0f};
    float sub_pitch_398{0.0f};   // controller+398h
    unsigned long long sub_dive_steps{0};
    unsigned long long sub_trace_counter{0};
    // Packet cc9_submarine_air: +127Ch air, +1281h needAir, +1280h unlimitedAir,
    // +1284h the crush accumulator, and the class rates +838h/+83Ch.
    bsp::SubmarineAirState sub_air{};
    bsp::SubmarineAirRates sub_air_rates{};
    float sub_crush_1284{0.0f};
    unsigned long long sub_crush_pulses{0};
    // Packet cc9_periscope_out: +122Ch periscopeState (mirrored from the ship AI),
    // the node test +1214h, the class's +81Ch, the mast's local Y above periscopeY
    // (+1230h, the node's own Y at attach, 00853CB4), and +1234h periscopeOut.
    int sub_periscope_state_122c{0};
    bsp::SubmarinePeriscopeRepair sub_periscope_repair{}; // +1254h/+1258h/+125Ch
    bool sub_periscope_node_1214{false};
    float sub_periscope_move_range{10.0f};  // 00CE38B8, NumberOr's default
    float sub_mast_offset{0.0f};
    bool sub_periscope_out_1234{false};
    unsigned long long sub_periscope_out_steps{0};
    unsigned long long sub_periscope_raises{0};
    bool sub_drowned{false};
    // Packet cc9_submarine_seabed: unit+1238h..+1250h, and the class sizes.
    bsp::SubmarineSeabedScan sub_scan{};
    float sub_class_width{0.0f};
    float sub_class_length{0.0f};
    float sub_class_height{0.0f};
    float sub_class_max_speed{1.0f};
    unsigned long long sub_scan_samples{0};
    unsigned long long sub_clamp_steps{0};
    // The controller's body angular velocity, ctl+48h pitch, +4Ch yaw, +50h
    // roll. 007DA710 writes these three and 007D9C80 rotates them into world
    // for 0085E4D0 to turn the pose with; plane_angular_velocity.hpp records
    // the same offsets as kAngularBody, recovered from the other end.
    //
    // Nothing writes them yet. 007DA710 drives each toward a target built from
    // the latched control inputs at ctl+BB0h/BB4h/BB8h, and no path from a bot
    // task to those latches is reconstructed, so every target is zero, the law
    // holds the axis at zero, and no plane turns. That is the open link, not a
    // simplification: docs/PLANE_CONTROL_RATE_LAW.md and
    // docs/PLANE_BOT_CONTROL_WRITEBACK.md.
    float plane_body_angular[3]{0.0f, 0.0f, 0.0f};
    // unit+9E4h yaw, +9E8h pitch, +9ECh roll - the live pilot control block,
    // and unit+BB0h/+BB4h/+BB8h, the previous-step snapshot 007B9770 latches
    // from it. plane_flight.hpp owns the offsets and carries the correction
    // note for the axis order; plane_advance_pose.hpp had it swapped until
    // packet cc7_plane_control_targets.
    float plane_live_controls[3]{0.0f, 0.0f, 0.0f};
    // unit+9F0h and +9F4h. 007CFEB0 sets the throttle to 1.0f at construction
    // and 007CFEA4 zeroes the air brake.
    // unit+C64h pitch, +C68h bank, +C6Ch heading, all from 007C18B0 (its 007C1900
    // block) over the live pose. Slot state rather than locals because the native
    // leaves the heading and bank untouched when the forward axis is near vertical.
    float plane_pitch_angle_c64{0.0f};
    // unit+C7Ch, the world velocity's elevation (007C1C4B..007C1CA8), written by
    // update_pose_derived_state_007c18b0 (all three of its host callers); kept when
    // |v| <= 0.1. Starts at 0 (the constructor's value was not read; labelled).
    float plane_velocity_elevation_c7c{0.0f};
    // Goal 5 chain 657: the rates 007C18B0 derives with dt > 0. +C70h the heading
    // rate (only while |d| < 0.5), +C74h pitch, +C78h bank, +C80h the velocity
    // elevation's and +C88h the path pitch's.
    float plane_heading_rate_c70{0.0f};
    float plane_pitch_rate_c74{0.0f};
    float plane_bank_rate_c78{0.0f};
    float plane_velocity_elevation_rate_c80{0.0f};
    float plane_path_pitch_rate_c88{0.0f};
    // Packet cc9_torpedo_run_pitch_profile: the measured angle the last pitch
    // demand used (diagnostic).
    float diag_held_pitch{0.0f};
    // Packet cc13-ratetracebank (diagnostic): pilot+2BCh as the task arm left
    // it (before 009FC7C0, the rocket and 009A17D0) and as 0099D300 entered.
    float diag_pt2bc_task{0.0f};
    float diag_pt2bc_think{0.0f};
    // Packet cc13-ratetraceband (diagnostic): cmd[1] after 0099BC00, before the
    // 0099BF30 band repair, and the avoidance counters as 009A17D0 was entered.
    float diag_cmd1_pre{0.0f};
    int diag_va_plane_ticks0{0};
    int diag_va_ship_ticks0{0};
    int diag_tr_bands0{0};
    int diag_ga_flagged_ticks0{0};
    float plane_bank_angle_c68{0.0f};
    float plane_heading_c6c{0.0f};
    float plane_class_turn_roll_spd{0.0f};   // desc+1C8h TurnRollSpd
    float plane_class_turn_roll{0.0f};       // desc+25Ch TurnRoll
    float plane_class_turn_roll_leader{0.0f};  // desc+260h TurnRollLeader
    // DIAGNOSTIC (packet cc9_hull_inertia): the largest angle between the hull's
    // up row and world up seen by publish_pose, in degrees. Read-only.
    float diag_hull_tilt_max_deg{0.0f};
    // The plan's non-slot fields, reset by 0099B450 on every think.
    bsp::PilotPlanState plan_state;
    // The range to the commanded target the first time the yaw arm planned for
    // this unit, and the last. Two numbers, so the run can say whether an
    // ordered aircraft actually closed on what it was ordered at.
    float attack_range_first{-1.0f};
    float attack_range_last{-1.0f};
    // The absolute heading error at the first plan and the last. This is the
    // measurement that says whether the yaw law steers: the range can grow for
    // reasons the yaw arm does not control, but the heading error is exactly
    // what it is trying to shrink.
    float attack_hdg_err_first{-1.0f};
    float attack_hdg_err_last{-1.0f};
    float attack_pitch_last{0.0f};
    // The dive-bomb bot task (kind 8) this ordered aircraft runs, when its
    // class carries general bomb ordnance (kind 2Ah, 007ED7E0 -> 007B9320) and
    // the command arm at 007EE9C5 chose class 00E08F20. docs/DIVE_BOMB_TASK.md.
    // The class 007EEC50 chose, stored by the PilotSetTarget path. 0 means no
    // attack order, which is what every aircraft in IJN01 and USN01 has.
    unsigned int attack_command_class{0};
    bool dive_bomb_task_installed{false};
    // kTaskProfileActivationOnlyBound: set at each task install, cleared by the
    // profile call it stands for (0099AEB3 -> task vt[54h]).
    bool db_profile_activation_pending{false};
    bool lb_profile_activation_pending{false};
    bool moveto_profile_activation_pending{false};
    bool land_profile_activation_pending{false};
    bsp::DiveBombState dive_bomb_state{bsp::DiveBombState::kNone};
    // Packet cc9_dogfight_task: the dogfight task (kind 2), installed when
    // 007EEC50 chose class 00E08F58. docs/DOGFIGHT_TASK.md.
    bool dogfight_task_installed{false};
    // Packet cc13-depthcharge1: the depth-charge task 009A5240, behind
    // bsp::kDepthChargeTaskBound. docs/CC13_DEPTH_CHARGE_TASK.md 7.
    bool depth_charge_task_installed{false};
    std::size_t dc_target_plus_one{0};
    // 009A6970's 7BCh allocation backs the task actually ticked below. The
    // semantic C++ state has no native layout/ABI claim. Use unsized delete
    // because the allocation is larger than this typed projection.
    struct DepthChargeTaskDelete {
        void operator()(bsp::DepthChargeTask* task) const noexcept {
            task->~DepthChargeTask();
            ::operator delete(task);
        }
    };
    using DepthChargeTaskOwner = std::unique_ptr<bsp::DepthChargeTask, DepthChargeTaskDelete>;
    DepthChargeTaskOwner dc;
    unsigned long long dc_transitions{0};
    unsigned long long dc_releases{0};
    // Packet cc9_l65_dc_census: arm ticks per DepthChargeState, the ticks with an
    // aim point (+75h) and the ticks with depth charges aboard (+74h).
    int dc_state_ticks[9]{};
    int dc_arm_ticks{0};
    int dc_target_ticks{0};
    int dc_armed_ticks{0};
    float dc_min_range_48{1.0e9f};
    // Packet cc9_strafe_arm (cc9-lua30): the strafe task (kind 0Ah, 6FCh bytes),
    // installed when 007EEC50 chose class 00E08F40, behind bsp::kStrafeTaskBound.
    // Approach fields by their 009CA4A0 offsets (approach = task+3F8h), state
    // fields by the state's own offsets. docs/SQUADRON_LAND_TASK.md 5ca.
    bool strafe_task_installed{false};
    bsp::StrafeState strafe_state{bsp::StrafeState::kNone};
    std::size_t st_target_plus_one{0};
    // Packet cc13-strafe70: approach+70h (target ref sub+18h) dropped by 009FADA0
    // (009FAE11) once the kind-5 target's +5Dh is set. Read only under
    // bsp::kStrafeTargetFallback70Cc13Bound.
    bool st_ref70_dropped{false};
    // Packet cc13-strafeaimfreeze (bsp::kStrafeAimPointFreezeCc13Bound): sub+1Ch frozen
    // by 009FADA0's head drop or 009FAF90, and the census of each path.
    bool st_aim_frozen{false};
    int st_aim_freeze_5d{0};
    int st_aim_freeze_notify{0};
    int st_aim_frozen_refreshes{0};
    bsp::StrafeApproachUpdate st_ap{};      // +44h, +CCh, +D0h, +50h
    float st_speed_ratio_24{1.0f};
    float st_glide_angle_2c{0.0f};
    float st_glide_tan_30{1.0f};
    float st_goaway_dist_34{0.0f};
    float st_shoot_dist_38{0.0f};
    float st_too_close_3c{0.0f};
    float st_glide_dist_40{0.0f};
    float st_run_speed_48{0.0f};
    float st_speed_4c{0.0f};
    float st_aim_ratio_9c{0.0f};
    float st_aim_point[3]{0.0f, 0.0f, 0.0f};   // approach vt[0] 009CA680
    // Packet cc13-oprocket1: task+38Ch, 009FD0E0, behind bsp::kOpportunisticRocketBound.
    bsp::OpportunisticRocketState orx{};
    int orx_owner{0};   // 1 dogfight, 2 strafe: whose Stop clears it
    // The rocket platforms 007C1EB0 walks, single (BombPlatform) AND multiple
    // (MultiBombPlatform: every Corsair, Raiden, Shinden and Kikka rocket set),
    // in BSPGun order; the per-rack vectors above hold single racks only.
    // Kind: 1 small non-AA, 2 small AA, 3 IgnitionDelay > 0 (cc9-gunnery39).
    std::vector<int> orx_rack_kind, orx_rack_bullet, orx_rack_ammo;
    unsigned long long orx_decisions{0}, orx_salvos{0}, orx_releases{0};
    // The 0080AA20 request held for the release (cc9-gunnery39, GUNNERY 202).
    std::size_t orx_pending_rack_plus_one{0}, orx_pending_target_plus_one{0};
    std::array<float, 3> orx_pending_point{};
    float orx_pending_angle{0.0f};
    unsigned long long orx_spawned{0}, orx_spawn_refused{0};
    float st_gt_dist_18{800.0f};           // gotowards +18h
    float st_gt_horiz_1c{800.0f};          // gotowards +1Ch
    bool st_gt_aligned_20{false};          // gotowards +20h
    bool st_aim_close_1c{false};           // aim +1Ch
    bool st_aim_off_1d{false};             // aim +1Dh
    float st_ga_prob_18{0.3f};             // goaway +18h
    float st_ga_bank_1c{0.0f};             // goaway +1Ch (009CB780)
    float st_ga_pitch_20{0.0f};            // goaway +20h (009CB650)
    bool st_ga_done_24{false};             // goaway +24h
    float st_ga_hdg_timer_30{0.0f};        // goaway +30h, re-armed U(+28h 2, +2Ch 5)
    float st_ga_pitch_timer_3c{0.0f};      // goaway +3Ch, re-armed U(+34h 2, +38h 5)
    float st_ga_countdown_44{0.0f};        // goaway +44h against +40h 1.0
    bool st_evading{false};                // census edge of goaway's evasive gate
    // Packet cc9_strafe_breakoff (bsp::kStrafeBreakoffBound): the manoeuvre
    // goaway front-pushes; 0 none, 1 tightturn 009BC030, 2 flikflak 009BC0A0.
    int st_mv_kind{0};
    // Packet cc9-gunnery38 O1: the strafe task is (again) the head, so its
    // cruise profile 009CD020 runs once (0099AEB3); set by the install and by a
    // manoeuvre's retire, consumed by the arm.
    bool st_profile_activation_pending{false};
    int st_profile_activations{0};
    int st_profile_seeds{0};
    bool st_mv_side{false};                // tightturn +408h, flikflak +400h
    float st_mv_point[3]{0.0f, 0.0f, 0.0f};// the end condition's point +1Ch..+24h
    float st_mv_dist2{0.0f};               // flikflak's condition +28h
    float st_mv_ref_3fc{0.0f};             // tightturn +3FCh (009BAFC0)
    float st_mv_timer_3fc{0.0f};           // flikflak +3FCh
    int st_mv_tightturns{0};
    int st_mv_flikflaks{0};
    int st_mv_ticks{0};
    float st_ar_period_18{0.4f};           // attackrun +18h
    float st_ar_countdown_1c{0.0f};        // attackrun +1Ch
    float st_ar_offset_20{0.0f};           // attackrun +20h
    bsp::PilotAttackMode st_attack_mode_370{bsp::PilotAttackMode::kForced};
    // (approach+1Ch)+5Ch..+64h and +28h as the aim state writes them for the gun
    // controller 009FC7C0 in the same think; the gun's tail resets +28h to -1.
    float st_gun_point_5c[3]{0.0f, 0.0f, 0.0f};
    bool st_gun_point_armed_28{false};
    int st_arm_ticks{0};
    int st_state_ticks[7]{};
    int st_transitions{0};
    int st_transition_logs{0};
    // Packet cc9_lua52_kamikaze_wiring (bsp::kKamikazeTaskBound,
    // bsp::kDropKamikazeTaskBound; docs/CC13_KAMIKAZE_TASK.md): the kamikaze task
    // (kind 0Ch, factory 009AF720) and the drop-kamikaze task (kind 5, 009AEBE0),
    // one value each per unit, plus what the host keeps beside them.
    bool kamikaze_task_installed{false};
    std::size_t kz_target_plus_one{0};
    bsp::KamikazeTask kz;
    bsp::PilotAttackMode kz_attack_mode_370{bsp::PilotAttackMode::kForced};
    float kz_alt_394{0.0f};                // ctl+394h as the cruise profile 009AF480 left it
    float kz_alt_398{0.0f};                // ctl+398h
    float kz_moveto_near{0.0f};            // 009AED20: BotStateMoveTo(+4Ch, +4Ch, +48h)
    float kz_moveto_speed_range{0.0f};
    // Packet cc9-rocket1 (bsp::kRocketTaskBound; docs/CC13_ROCKET_TASK.md 7): the
    // rocket task (kind 0Bh, 007B6A40), one value per unit, the moveto ranges the
    // arm hands 009BDE80 each think, and a log-only census.
    bool rocket_task_installed{false};
    std::size_t rk_target_plus_one{0};
    bsp::RocketTask rk{};
    float rk_moveto_near{0.0f}, rk_moveto_far{0.0f}, rk_moveto_speed{0.0f};
    unsigned long long rk_installs{0}, rk_transitions{0}, rk_releases{0};
    unsigned long long rk_spawned{0}, rk_spawn_refused{0}, rk_hit_resets{0};
    int rk_transition_logs{0};
    int rk_arm_ticks{0}, rk_rocket_ticks{0};
    int rk_state_ticks[7]{};   // moveto follow prepare gotowards aim goaway attackrun
    // The aimed rocket's class row (read_rocket_bullet_row), cached by Bullets id.
    int rk_cls_bullet{-2};
    float rk_cls_ign{0.0f}, rk_cls_vmax{0.0f}, rk_cls_acc{0.0f};
    bool rk_cls_aa{false};
    bool drop_kamikaze_task_installed{false};
    bool torpedo_install_refused_class{false};   // packet cc9_dkz_spent_rtb, logged once
    std::size_t dkz_target_plus_one{0};
    std::size_t db_task_target_plus_one{0};        // task+440h, kAttackTaskTargetRetireBound
    std::size_t torpedo_task_target_plus_one{0};   // task+4C4h, kAttackTaskTargetRetireBound
    bsp::DropKamikazeTask dkz;
    bsp::PilotAttackMode dkz_attack_mode_370{bsp::PilotAttackMode::kForced};
    float dkz_alt_398{0.0f};
    float dkz_moveto_near{0.0f};           // 009BDE80 at 009AE8C9: (alt - 100, alt, startDist)
    float dkz_moveto_far{0.0f};
    float dkz_moveto_speed_range{0.0f};
    int dkz_rack{-1};                      // the rack whose bullet is a DummyKamikazePlane
    bool dkz_c25{false};                   // unit+C25h stand-in: a round left this think
    // plan+2E4h, the boost request (-> cmd+14h -> unit+A10h, docs/PILOT_PLAN_SLOT_PIPELINE.md);
    // 0099B56C clears it every think. Its reader 007BB7B9 is hosted only under
    // kKamikazeBoostTurboBound (packet cc13-fix89 U2, docs/CC13_FIX89.md).
    bool plan_boost_2e4{false};
    // Packet cc13-fix89 U2: the boost byte unit+9F8h (007BB6E0), its latched copy
    // +BC8h (007B978F), the charge +E84h and its lockout +E88h, and the class's
    // TurboTime / TurboRechargingTime / TurboStrength (desc+5FCh / +600h / +604h,
    // 007D3CD0-007D3D50, TurboTime zeroed at 007D3DB5 unless the strength > 1).
    bool plane_boost_9f8{false};
    bool plane_boost_latched_bc8{false};
    float plane_boost_charge_e84{0.0f};
    bool plane_boost_lockout_e88{false};
    float plane_turbo_time{0.0f};
    float plane_turbo_recharging_time{0.0f};
    float plane_turbo_strength{1.0f};
    float plane_turbo_control_limit{1.0f};   // desc+608h TurboControlLimit, 007C0FDE
    // task+2E4h, the per-think avoidance mask 009A17D0 tests (bit 4 gunfire
    // detect 009A18BA, bit 2 vehicle 009A1951, bit 1 terrain 009A1974): 0FFh at
    // 00999944 before the arm; the kamikaze arms store their band (009AF54A,
    // 0 in aim) and the drop arm its avoid flag. Every other task leaves 0FFh.
    int task_mask_2e4{0xFF};
    // Diagnostics: thinks with each order class (both builds), and the arms' census.
    int kz_ordered_thinks{0};
    int dkz_ordered_thinks{0};
    int kz_arm_ticks{0};
    int kz_state_ticks[5]{};               // moveto, follow, prepare, gotowards, aim
    int kz_installs{0};
    int kz_aim_entries{0};
    int kz_locks{0};
    int kz_boost_ticks{0};
    int kz_transition_logs{0};
    float kz_min_target_dist{-1.0f};
    int dkz_arm_ticks{0};
    int dkz_state_ticks[7]{};              // moveto, follow, attackrun, prepare, done, release, goAway
    int dkz_installs{0};
    int dkz_shape_writes[2]{};             // squadron+3E4h = 1 (009BEDDA) / = 3 (009AD1A5)
    int dkz_releases{0};
    int dkz_transition_logs{0};
    int st_aim_entries{0};
    int st_evasive_gaps{0};
    int st_gun_point_ticks{0};
    int st_gun_point_fires{0};
    int st_hit_resets{0};
    // Packet cc9_plane_level_bomb_task (bsp::kPlaneLevelBombTaskBound): the
    // levelbomb task (kind 4, 6F8h bytes, 009B9030 -> 009B7990), installed when
    // 007EEC50 chose class 00E08F28. Approach fields by their 009B75E0 / 009B44F0
    // offsets, state fields by the state's own offsets. docs/SQUADRON_LAND_TASK.md
    // section 5ee and include/bsp/plane_level_bomb_task.hpp.
    bool level_bomb_task_installed{false};
    bsp::LevelBombState level_bomb_state{bsp::LevelBombState::kNone};
    std::size_t lb_target_plus_one{0};
    bsp::LevelBombApproach lb_ap{};
    float lb_aim[3]{0.0f, 0.0f, 0.0f};        // approach->vtable[0], this tick
    // squadron+370h, the leader's value copied to the members (the same labelled
    // ownership hole as db_attack_mode_370), seeded as the order leaves it.
    bsp::PilotAttackMode lb_attack_mode_370{bsp::PilotAttackMode::kForced};
    float lb_mv_frac_3c{0.0f};                // moveto +3Ch
    float lb_mv_tol_40{1.0f};                 // moveto +40h
    float lb_mv_period_44{1.0f};              // moveto +44h
    float lb_mv_countdown_48{0.0f};           // moveto +48h
    float lb_ar_period_18{0.4f};              // attackrun +18h
    float lb_ar_countdown_1c{0.0f};           // attackrun +1Ch
    float lb_ar_offset_20{0.0f};              // attackrun +20h
    float lb_ar_frac_24{0.0f};                // attackrun +24h
    bool lb_aim_18{false};                    // aim +18h, release now
    bool lb_aim_19{false};                    // aim +19h, abort the run
    bsp::LevelBombReleaseState lb_release{};  // release +18h..+20h
    float lb_ga_standoff_18{0.0f};            // goaway +18h
    float lb_ga_side_1c{1.0f};                // goaway +1Ch
    bool lb_prepare_a0{false};                // prepare +A0h
    float lb_prepare_98{-1.0f};               // prepare +98h
    float lb_prepare_8c{1.0f};                // prepare +8Ch (kLevelBombPrepareTickBound)
    float lb_prepare_9c{0.0f};                // prepare +9Ch
    int lb_prepare_wait_ticks{0};             // 009B6670 wait-arm ticks
    int lb_prepare_drop_ticks{0};             // 009B6670 drop-arm ticks
    int lb_prepare_arms{0};                   // 009B8100 calls
    int lb_prepare_releases{0};               // 009B67E9 releases
    bool lb_break_off_now{false};             // 009B8D80's last answer (007EE6B0)
    // The level bomber's racks as 007C0D90's level branch fires them: each
    // single rack's own toRepeatTime (+494h) and dropBombs (+498h).
    std::vector<float> lb_rack_to_repeat;
    std::vector<char> lb_rack_dropping;
    float lb_release_delay_1fc{0.0f};         // 007C1FB0, max RepeatTime x (rounds - 1)
    float lb_rack_repeat_e0{0.0f};            // the racks' RepeatTime, desc+E0h
    int lb_installs{0};
    int lb_arm_ticks{0};
    int lb_state_ticks[7]{};
    int lb_transitions{0};
    int lb_transition_logs{0};
    int lb_releases{0};
    int lb_aim_entries{0};
    int lb_aim_aborts{0};
    int lb_break_offs{0};
    int lb_rack_issues{0};
    int lb_rack_fired{0};
    int lb_issue_tail_calls{0};      // 007C0ED9 tail runs (kLevelBomberIssueTailBound)
    int lb_pending_424{0};           // task+424h (kLevelBombReleaseOrderSpendBound)
    int lb_orders_spent{0};          // 0099AF9B offers taken by 009B83D0
    int lb_orders_cleared{0};        // 0099AFC2 zeroings
    int lb_passthrough_releases{0};  // 009B8C38 007BBBA0 calls
    int lb_issue_tail_raised{0};     // members 007EEF30 gave orders to
    int lb_issue_tail_gate_refused{0};  // 007EEF50 gate (ctl+390h > ctl+374h) refused
    int lb_rack_drops{0};
    int lb_rack_gate_refused{0};
    int lb_rack_bay_refused{0};   // 007CC8F4-007CC928 bay test (packet cc13-lbbay)
    float lb_first_release_at{-1.0f};
    float lb_first_drop_at{-1.0f};
    float lb_min_impact_miss{-1.0f};          // the aim tick's smallest planar miss
    // Packet cc9_pilot_moveto_task: the kind-7 moveto task (009C3BE0 -> 009C3000,
    // vtable 00D20B68). The three states live in the sub-object at task+3F8h and
    // register their names through 00411E70 (009C2E96-009C2EB6):
    //   +47Ch "moveto (moveto)"  (00D20998), vtable 00D20A80
    //   +494h "follow (moveto)"  (00D20988), 009C2980 BSP_BotStateFollow_Construct
    //   +52Ch "circle (moveto)"  (00D20978), 009C25D0
    // 009C3097-009C30BF picks the start: a flight leader (007B8AD0) enters
    // +52Ch when task+454h (the sub-object's +5Ch) is set, else +47Ch; a wingman
    // enters +494h. 009C1D0A clears +5Ch at construction, so a leader starts in
    // moveto; 009C3636 in 009C3570 sets it later.
    enum class MoveToTaskState : int { kNone = 0, kMoveTo = 0x47C, kFollow = 0x494, kCircle = 0x52C };
    bool moveto_task_installed{false};
    // Packet cc13-moveonpath1: the kind-6 moveonpath task (009BCE50), kMoveOnPathTaskBound.
    bool moveonpath_task_installed{false};
    GameMoveOnPathTaskOwner moveonpath_task;
    unsigned long long moveonpath_ticks{0};
    bool moveto_stage_logged{false};   // cc9_moveto_arrival_end_command, one note per member
    MoveToTaskState moveto_state{MoveToTaskState::kNone};
    bool moveto_arrived_454{false};        // task+454h, cleared by 009C1D0A
    float moveto_range{0.0f};              // the order's descriptor +14h
    float moveto_approach_6c{0.0f};        // approach+6Ch, 009C35CB (cc9_moveto_command_range)
    float moveto_timer_548{1.0f};          // 009C307C, 1.0 (00D7A24C)
    float moveto_stagger_54c{-0.5f};       // 009C3084/009C3089: -U(0, 1), the midpoint
    // Part 2, the approach at task+3F8h (009C1C30 on 009F9CE0) and two state
    // fields, named by their offset in that object.
    std::size_t moveto_target_plus_one{0};   // approach+44h, the order's target
    // Chain468: actual 14h entity-target storage, retained with this task's
    // owner. Slot allocations stay stable for the mission's unit lifetime.
    std::optional<bsp::MoveToEntityTargetStorage> moveto_entity_target;
    GameUnitSlot* moveto_entity_44{nullptr}; // approach's borrowed entity, from target slot0
    bool moveto_entity_refresh_pending{false}; // retarget must refresh before either steering state
    float moveto_alt_offset_2c{0.0f};        // U(0, tuning+540h MaxAltOffset), 009C1C30
    // approach+68h: 50.0 (00CEB4D4) from 009C1C30, redrawn U(FollowDist/1,
    // FollowDist/2) by the activation's 009C3650 (kMoveToActivationDrawsBound).
    float moveto_follow_dist_68{50.0f};
    float moveto_refresh_58{0.0f};           // countdown; period +54h = 0.5 (009C1CE0)
    float moveto_dwell_60{0.0f};             // the arrival dwell
    float moveto_distance_64{0.0f};          // planar distance, 009BEBA0
    // Packet p16_moveto_cruise: approach+28h and whether the leg manager holds a
    // record for this unit through the moveto task.
    float moveto_leg_weight_28{-1.0f};
    bool moveto_leg_registered{false};
    float moveto_point[3]{0.0f, 0.0f, 0.0f}; // +48h..+50h, the steer point
    float moveto_follow_stagger_74{0.0f};    // follow state +74h, -U(0, 0.6), 009C2980
    // Packet cc13-fix89 U1 (kFollowSightSearchBound, docs/CC13_FIX89.md): the
    // follow state's sight latch +84h and its point +78h..+80h (in the leader's
    // frame), written by 009C211D-009C22F5 and read by the hold arm 009BF0FD.
    // fs_countdown_74 is +74h for the follow ticks that run through
    // run_follow_tick_009c1fd0 (the moveto follow keeps moveto_follow_stagger_74).
    bool fs_sight_84{false};
    float fs_sight_78[3]{0.0f, 0.0f, 0.0f};
    float fs_countdown_74{0.0f};
    float fs_think_dt{0.0f};                 // this think's elapsed, 009C1FD0's dt
    // The think count, and the think of the last follow tick: a follow tick
    // with no follow tick on the previous think is an entry (009BED80).
    unsigned long long fs_thinks{0};
    unsigned long long fs_last_follow_think{0};
    bsp::FollowParameterSnapshot fs_parameters;
    // unit+914h: -1.0 from the plane constructor (007CFDDF, 00D7A260); 1.0 once
    // a follow sight search finds a contact (009C230B). Nothing resets it.
    float plane_formation_sight_914{-1.0f};
    bool moveto_circle_flag_18{false};       // circle state +18h, U(0,1) >= 0.5, 009C25D0
    // Part 3, the follow state: +70h the search period (0.6, 00CE3D30, set in 009C2980),
    // +74h its countdown, drawn at construction.
    unsigned long long moveto_follow_ticks{0};
    unsigned long long moveto_follow_no_station{0};
    // The wanderer at plane+810h (007C4560 constructs it: +30h = 1.0, +34h = 1):
    // plane+840h WandererMul and plane+844h WandererEnabled (the Lua names at
    // 00D1047C / 00D10460, SquadronSetWandererMul / SquadronSetWandererEnabled).
    // Written by the follow trail arm 009C207C (kFollowTrailArmBound); read by
    // 007BE060 BSP_PlaneWanderer_FixedStep, hosted as wanderer_fixed_step_007be060
    // under kPlaneWandererBound (ON). Also written by the gunfire exempt arm 0099F197
    // (kGunfireExemptWandererMulBound).
    // plane+810h, the wanderer (packet cc9_plane_wanderer): +30h is plane+840h
    // WandererMul, +34h plane+844h WandererEnabled.
    bsp::PlaneWandererState wanderer;
    bool wanderer_off_state{false};   // in land/final, turndown, aimdive or flyabove
    unsigned long long wanderer_steps{0}, wanderer_active{0};
    float wanderer_offset_max_seen{0.0f};
    double wanderer_displacement{0.0};
    // Part 4, the circle state 009C26D0 (kMoveToCircleSteerBound): tick count and
    // the planar distance to the steer point that 009FBB20 measured, min / max / last.
    unsigned long long moveto_circle_ticks{0};
    unsigned long long moveto_circle_no_heading{0};   // 009FBBF8 early exit
    float moveto_circle_min_d{-1.0f};
    float moveto_circle_max_d{-1.0f};
    float moveto_circle_last_d{-1.0f};
    unsigned long long moveto_ticks{0};
    unsigned long long moveto_state_changes{0};
    unsigned long long moveto_arrivals{0};
    float moveto_min_distance{-1.0f};
    // Packet cc9_takeoff_task_bind (kBaseLaunchChainBound, docs/SQUADRON_LAND_TASK.md
    // 5bd): the `takeoff` task, kind 0Dh (009CFF40 -> 009CF8E0, size 4D4h). Its
    // four states sit in the approach at task+3F8h and register their names in
    // 009CF710 (strings, not invented): +448h "takeoff/prepare", +470h
    // "takeoff/Takeoff", +490h "takeoff/SlowTakeoff", +4B8h "takeoff/parking".
    enum class TakeoffTaskState : int { kNone = 0, kPrepare = 0x448, kTakeoff = 0x470,
        kSlowTakeoff = 0x490, kParking = 0x4B8 };
    bool takeoff_task_installed{false};
    // Packet cc9_takeoff_task_head: a land install that waits for the takeoff
    // task to retire (kTakeoffTaskHeadBound).
    bool land_install_waits_for_takeoff{false};
    // Packet cc9_ground_retakeoff: the tick tail's pushes for this plane.
    unsigned long long retakeoff_pushes{0};
    TakeoffTaskState takeoff_state{TakeoffTaskState::kNone};
    // approach+2Ch, 009CFA80's altitude floor; +30h the margin (15.0 00CE5380,
    // 25.0 00CE89CC for IsKindOf(10h)); +34h the plane's y at the install when
    // it has a holder (+BF4h), else 0 (009CD420).
    float takeoff_floor_2c{0.0f};
    float takeoff_margin_30{15.0f};
    float takeoff_base_34{0.0f};
    // takeoff/prepare (approach+50h): +18h the site permission cache, +19h done,
    // +1Ch the time in the state, +20h the wobble phase, +24h its period.
    bool takeoff_prep_permit_18{false};
    bool takeoff_prep_done_19{false};
    float takeoff_prep_t_1c{0.0f};
    float takeoff_prep_phase_20{0.0f};
    float takeoff_prep_period_24{0.0f};
    // takeoff/SlowTakeoff (approach+98h): +18h..+20h the start, +24h the yaw.
    float takeoff_slow_start_18[3]{0.0f, 0.0f, 0.0f};
    float takeoff_slow_yaw_24{0.0f};
    // takeoff/Takeoff (approach+78h): +18h and +1Ch as 009CE270 sets them.
    float takeoff_run_18{0.0f};
    float takeoff_run_1c{0.0f};
    // Packet cc13-takeoff (kPlaneTakeoffStartNotifyCc13Bound): 007C17D0's C3h
    // {+20h = the current +900h, +21h = 4}, posted at the Takeoff enter and
    // delivered after the plane's own step (the image: row 9).
    bool takeoff_notify_c3h_pending{false};
    std::int32_t takeoff_notify_c3h_current{0};
    // Packet cc13-takeoff (kPlaneShipyardLaunchAa0Cc13Bound): unit+AA0h. 007D5D20
    // stores 0.0, then 5.0 / -1.0 by the ShipYardLaunch key; 007B83F0 stores 5.0
    // or -1.0. Read only with the switch on.
    float plane_shipyard_launch_aa0{0.0f};
    unsigned long long takeoff_installs{0}, takeoff_ticks{0}, takeoff_prep_ticks{0};
    unsigned long long takeoff_slow_ticks{0}, takeoff_run_refused{0};
    // Packet cc9_takeoff_step_bind: takeoff/Takeoff's step 009CE2C0.
    // approach+38h as the step writes it (3, FFh or 0; no reader here), the
    // step's census and the first aligned and first full-throttle times.
    int takeoff_run_38{0};
    unsigned long long takeoff_run_ticks{0}, takeoff_run_aligned{0}, takeoff_run_speed{0};
    unsigned long long takeoff_run_low_land{0}, takeoff_run_no_holder{0};
    unsigned long long takeoff_run_hold{0}, takeoff_run_no_deck{0};
    float takeoff_run_first_aligned{-1.0f}, takeoff_run_last_throttle{0.0f};
    float takeoff_installed_at{-1.0f}, takeoff_prep_left_at{-1.0f};
    float takeoff_slow_left_at{-1.0f}, takeoff_prep_min_member_d{-1.0f};
    const char* takeoff_prep_exit{""};
    // Packet cc9_plane_retreat_task (bsp::kPlaneRetreatTaskBound): the `retreat`
    // task, kind 9 (009CA2B0 -> 009C9D00, size 564h), its approach at task+3F8h
    // (009C9BB0 -> 009C91B0) and the plane's leave-map countdown 007C6C30
    // (unit+6ECh/+6F0h). include/bsp/plane_retreat_task.hpp; the state names
    // are the approach's own registered strings.
    bool retreat_task_installed{false};
    bsp::RetreatState retreat_state{bsp::RetreatState::kNone};
    bsp::RetreatApproach retreat_approach{};
    float retreat_arm_560{0.0f};            // task+560h, -U(0, 1) at 009C9D7E
    float retreat_moveto_timer_18{0.0f};    // moveto state+18h (task+490h), 009C92F0
    // 007EE2E0's retreat arm reads the SQUADRON's current command; set on every
    // member when the order went to the squadron (its fused flight leader).
    bool retreat_squadron_ordered{false};
    float retreat_exit_6f0{0.0f};           // unit+6F0h
    int retreat_exit_state_6ec{0};          // unit+6ECh: 1 inside, 2 counting, 3 expired
    // Packet cc13-leavecd: the first counting step of a plane that is not retreating
    // was logged (bsp::kLeaveCountdownAllSquadronsCc13Bound).
    bool leave_counting_logged{false};
    // Packet cc9_ship_exit_zone: a ship reuses +6ECh / +6F0h above (00826D70 writes the
    // same unit fields); +100Ch is its warning repeat timer.
    float ship_exit_warning_100c{0.0f};     // unit+100Ch
    bool ship_exit_counting_logged{false};
    unsigned long long retreat_ticks{0}, retreat_moveto_ticks{0}, retreat_enter_ticks{0},
        retreat_leave_ticks{0}, retreat_follow_ticks{0}, retreat_refreshes{0},
        retreat_zone_misses{0}, retreat_outside_ticks{0};
    float retreat_installed_at{-1.0f}, retreat_outside_at{-1.0f};
    // Packet cc9_land_task_reach (kSquadronLandTaskBound): the `land` task,
    // kind 3 (009B41C0 -> 009B3240, size 670h). The eight states sit in the
    // approach at task+3F8h and register these names through 00411E70 in
    // 009AF9A0 (strings 00D1FE1C..00D1FDC0), so the names are recovered, not
    // invented: +4C4h "moveto (land)", +500h "follow (land)", +598h
    // "land/line", +5B8h "land/standby", +5D8h "land/begin", +5F8h
    // "land/final", +620h "land/park", +64Ch "land/abort".
    enum class LandTaskState : int { kNone = 0, kMoveTo = 0x4C4, kFollow = 0x500,
        kLine = 0x598, kStandby = 0x5B8, kBegin = 0x5D8, kFinal = 0x5F8,
        kPark = 0x620, kAbort = 0x64C };
    bool land_task_installed{false};
    LandTaskState land_state{LandTaskState::kNone};
    // approach+30h, the block's +7Ch owner (the land target), as a unit index + 1;
    // approach+2Ch, the air-ops deck, as a deck index + 1 (0 = cleared by 009B34D0).
    std::size_t land_site_plus_one{0};
    std::size_t land_deck_plus_one{0};
    // Packet cc9_pilot_land_native: the task came from an explicit `land`
    // command (PilotLand 008A47B0 -> 0077D600), not from 007F16D0's answer, so
    // 009B34D0 checks the plane's own command rather than rtb_census.
    bool land_explicit_site{false};
    float land_angle_54{0.0f};     // U(0.9, 1.1) * tuning+4E4h ApproachAngle, 009AFEF5
    float land_pitch_58{0.0f};     // U(0.9, 1.1) * tuning+4E0h ApproachPitch, 009AFF25
    float land_speed_5c{0.0f};     // class+18Ch TravelSpeed * 0.75 (00CEC9D8), 009AFF4A
    float land_alt_60{0.0f};       // 009AFF45, the approach altitude offset
    float land_timer_64{10.0f};    // 009AFF50, 00CE38B8
    float land_request_ac{0.5f};   // 009AFF77, 00CE3800, the request period
    float land_request_b0{0.0f};   // -U(0, 0.5), 009AFF84..009AFF8B
    float land_follow_stagger_74{0.0f};  // follow state +74h, -U(0, 0.6), 009C2980
    int land_mode_50{1};           // approach+50h, 009AFEC7; 006C54C0 writes it
    // Packet cc9_land_internal_records: 006C54C0's queue, lookup and tail ran
    // on a built deck this request (landing_request_006c54c0 past its guards).
    bool land_request_ran{false};
    // Packet cc9_landing_sequencer: 006C54C0's found arm (006C5516-006C552C)
    // copies the record's +8h to approach+4Ch and its side +0Dh to approach+44h
    // (out+0Ch). No bound state reads either; they are the observable result.
    float land_spacing_4c{1.0f};
    bool land_side_44{false};
    // Packet cc9_land_standby_state: 006C54C0's common tail (006C55FF-006C562A),
    // approach+38h..+40h the circle point 006C5380 and +48h the radius 006C3E50,
    // and the standby state's counters.
    float land_circle_38[3]{0.0f, 0.0f, 0.0f};
    float land_radius_48{1.0f};
    unsigned long long land_standby_ticks{0};
    unsigned long long land_standby_entries{0};
    float land_standby_last_d{-1.0f};
    // Packet cc9_land_line_state: land/line's counters and the last distance
    // to the squadron head (009B04F3).
    unsigned long long land_line_ticks{0};
    unsigned long long land_line_entries{0};
    float land_line_last_head_d{-1.0f};
    // Packet cc9_land_begin_state: 009AFAF0's approach geometry and land/begin.
    // approach+98h..+A0h the path point 006BCA80, +A4h its heading, +A8h the
    // horizontal distance, +8Ch/+90h/+94h the lateral offset, the height above
    // the point and the along-track distance; the state's done byte +18h.
    float land_path_98[3]{0.0f, 0.0f, 0.0f};
    float land_heading_a4{0.0f};
    float land_dist_a8{0.0f};
    float land_geo_8c[3]{0.0f, 0.0f, 0.0f};
    bool land_done_18{false};
    unsigned long long land_begin_ticks{0};
    unsigned long long land_begin_entries{0};
    unsigned long long land_begin_abort_refused{0};
    unsigned long long land_begin_final_refused{0};
    unsigned long long land_begin_hold_unapplied{0};
    // Packet cc9_landing_descent_2: the timed direction hold dyn+B4h/+C0h
    // (docs/PLANE_DYN_TIMED_HOLD.md, docs/SQUADRON_LAND_TASK.md 5o), world
    // direction as 007C07A0 normalised it, and its counters.
    float plane_dir_hold[3]{0.0f, 0.0f, 0.0f};
    float plane_dir_hold_seconds{0.0f};
    // Packet cc9_plane_rate_floor (kPlaneSpeedFloorBound): dyn+C8h, the band-repair
    // speed-floor timer (007D7A40 arms 1.2f, 007D81B0 counts it down and parks it
    // at -1.0f outside free flight), and its counters.
    float plane_speed_floor_timer_c8{0.0f};
    unsigned long long speed_floor_arms{0};
    unsigned long long speed_floor_raises{0};
    unsigned long long rate_floor_steps{0};
    unsigned long long bank_yaw_steps{0};
    // kPlaneDeckClampBound: dyn+A4h, and the clamp's counters.
    bool plane_contact_a4{false};
    // Chain 254: the actual per-plane contact normal and flag written when
    // the deck clamp raises this plane; the integrator consumes/clears +A4h.
    bsp::gameplay_seg48_cc13::GroundContactUp plane_ground_contact{};
    unsigned long long deck_clamp_lifts{0};
    double deck_clamp_lift_total{0.0};
    // kPlanePoseFirstBound: ctl+7Ch and the levelling mode the last core law left.
    float plane_forward_speed_7c{0.0f};
    // Chain 254: the fields written by 007D80C0, retained per live controller.
    // +44h and the old +7Ch are borrowed below from this step and its existing
    // latch; +78h holds the reconstructed rate instead of discarding it.
    bsp::band_007a_cc13::FlightControllerRateFields plane_controller_rate{};
    bool plane_pose_ground_prev{false};
    unsigned long long pose_first_steps{0};
    double bank_yaw_abs_angle{0.0};
    unsigned long long dir_hold_arms{0};
    unsigned long long dir_hold_blend_steps{0};
    float dir_hold_max_seconds{0.0f};
    unsigned long long land_begin_young_bot{0};
    float land_begin_last_a8{-1.0f};
    // Packet cc9_land_final_state: land/final's touched latch +20h and counters.
    bool land_touched_20{false};
    unsigned long long land_final_ticks{0};
    unsigned long long land_final_entries{0};
    unsigned long long land_final_abort_refused{0};
    unsigned long long land_final_park_refused{0};
    unsigned long long land_final_park_entered{0};
    unsigned long long land_final_ground_refused{0};
    unsigned long long land_final_ground_ticks{0};   // cc9_plane_ground_roll
    // Packet cc9_land_abort_state: land/abort +20h (to standby) and +21h (to park).
    bool land_abort_standby_20{false};
    bool land_abort_park_21{false};
    unsigned long long land_abort_entries{0}, land_abort_ticks{0}, land_abort_to_standby{0};
    unsigned long long land_abort_ground_refused{0}, land_abort_park_refused{0};
    unsigned long long land_abort_ground_ticks{0};   // 009B0E74's arm run (packet cc9_land_abort_ground_arm)
    // Packet cc9_land_park_taxi: land/park (+620h, vtable 00D1FF60) and the plane
    // bytes its tick writes. docs/SQUADRON_LAND_TASK.md 5s.
    float land_park_1c{0.0f};             // park +1Ch, 009B21A0
    float land_park_target_20{0.0f};      // park +20h: the taxi target x (airfield frame)
    float land_park_target_24{0.0f};      // park +24h: the taxi target z
    float land_park_timer_28{0.0f};       // park +28h, 3.0 at the enter (the carrier arm's clock)
    unsigned long long land_park_carrier_ticks{0};   // packet cc9_carrier_elevator
    bool plane_taxi_queue_910{false};     // plane+910h: inside the hangar queue band
    bool plane_in_hangar_c00{false};      // plane+C00h, 007B96C0
    // 007C5F60's plane+9DCh store (007C605D 3.0 / 007C60EA 5.0) and how many times
    // it ran; read by the gunnery crash sweep (packet cc13-cool9dc).
    std::uint32_t launch_spot_arms_9dc{0};
    float launch_spot_cooldown_9dc{0.0f};
    // [unit+360h]+184h cleared: 00951F40(0) -> 00710B80 took the collision part
    // out of the spatial index (007B96C0, 006FC250); 00951F40(1) (006FC0D0)
    // puts it back. Packet cc9_hit_index_detach, SQUADRON_LAND_TASK 5as.
    bool plane_hit_node_detached{false};
    // Packet cc13-luawire 7b. unit+340h, the CheatTurbo float only 008A62A0
    // writes (no constructor store, so 0); read under kLuaSetCheatTurboBound.
    float cheat_turbo_340{0.0f};
    // squadron+3A4h, the avoidance mask: 0FFh from the squadron constructor
    // (007F2C13), changed only by the SquadronEnable*Avoidance natives. Bit 2 is
    // read at 009A1963 under kLuaSquadronEnableVehicleAvoidanceBound.
    std::uint32_t sq_avoid_3a4{0xFFu};
    std::uint8_t plane_c01{0};            // plane+C01h, 007B9000 stores 2
    // 007CE46A's +C01h = 2 stores (below a Landscape), counted (goal 5 chain 640).
    unsigned long long plane_below_landscape_stores{0};
    unsigned long long land_park_entries{0}, land_park_ticks{0}, land_park_done{0};
    unsigned long long land_park_joins{0}, land_park_leaves{0}, land_park_spot_refused{0};
    // Packet cc9_land_park_done_test: which arm of 009B21D0 set done - the timer
    // (0 > +28h), no contact (+BF8h), no deck (+BF4h), the target behind
    // (t.z < pz), the heading past pi/2. Print-only.
    unsigned long long land_park_done_why[5]{0, 0, 0, 0, 0};
    unsigned long long land_park_hangar{0}, land_park_no_hangar{0}, land_park_from_abort{0};
    unsigned long long land_park_retarget{0};
    double land_park_hangar_at{-1.0};
    float land_park_last[4]{0.0f, 0.0f, 0.0f, 0.0f};   // x, z, speed request, yaw request
    // DIAGNOSTIC (p21_stow): the last 0099D300 demand inputs.
    float diag_thr_meas{0.0f}, diag_thr_scale{1.0f};
    int diag_thr_skip{-1};
    unsigned long long land_found_assignments{0};
    double land_installed_at{-1.0};
    unsigned long long land_ticks{0};
    unsigned long long land_requests{0};
    unsigned long long land_empty_assignments{0};
    unsigned long long land_moveto_ticks{0};
    unsigned long long land_follow_ticks{0};
    unsigned long long land_follow_no_station{0};
    unsigned long long land_state_changes{0};
    unsigned long long land_refused_states{0};
    float land_first_distance{-1.0f};
    float land_min_distance{-1.0f};
    float land_last_distance{-1.0f};
    double land_min_distance_at{-1.0};
    bsp::DogfightState dogfight_state{bsp::DogfightState::kNone};
    int df_state_ticks[bsp::kDogfightStateCount]{};
    int df_transitions{0};
    int df_within_attack_dist_ticks{0};
    float df_min_target_range{-1.0f};
    // Packet cc9_dogfight_engaged: the approach object's dogfight fields
    // (task+3F8h + N) and the engaged states'. docs/DOGFIGHT_ENGAGED.md.
    bsp::DogfightPilotRow df_row{};         // approach+14h, SPNormal substitute
    bool df_target_squadron{false};         // approach+CCh != 0 (task+4C4h)
    std::size_t df_target_plus_one{0};      // approach+B4h
    bool df_latch_d0{false};                // approach+D0h (task+4C8h)
    float df_distance_d4{9999.0f};          // approach+D4h
    float df_horizontal_d8{0.0f};           // approach+D8h
    float df_reselect_timer_e4{0.0f};       // approach+E4h
    float df_local_ec[3]{0.0f, 0.0f, 0.0f}; // approach+ECh..F4h
    float df_tan_f8[2]{0.0f, 0.0f};         // approach+F8h/FCh
    float df_aim[3]{0.0f, 0.0f, 0.0f};      // approach+48h (substitute: target origin)
    float df_projtime_70{0.0f};             // approach+70h (kDogfightAimLeadBound)
    int df_aim_lead_ticks{0};               // ticks with approach+70h > 0
    double df_aim_lead_shift_sum{0.0};      // sum of |+48h - target origin|
    // DIAGNOSTIC, packet cc9_kate_engagement: the pitch arm's last floored
    // target and demand, for the chase trace.
    float diag_floored_2bc{0.0f};
    // Packet cc9_kate_engagement: thinks with a positive turn numerator.
    int turn_numerator_ticks{0};
    float diag_pitch_demand{0.0f};
    bsp::DogfightAimState df_aim_state{};   // task+67Ch + 18h..25h
    float df_maneuver_range_34{0.0f};       // task+6D8h
    float df_avoid_timer{0.0f};             // task+720h / +748h
    float df_avoid_sign{1.0f};
    // Packet cc9_dogfight_maneuver_bodies (docs/DOGFIGHT_MANEUVER_BODIES.md).
    // The maneuver state task+6A4h: +18h mode, +1Ch bank, +20h/+24h heading
    // vector, +28h/+2Ch re-aim period and countdown, +30h pitch, +38h timer,
    // +3Ch pursue latch (+34h is df_maneuver_range_34).
    int mv_mode_18{0};
    float mv_bank_1c{0.0f};
    float mv_vec_20{0.0f}, mv_vec_24{1.0f};
    float mv_period_28{1.0f};
    float mv_count_2c{-0.5f};
    float mv_pitch_30{0.0f};
    float mv_timer_38{0.0f};
    bool mv_latch_3c{false};
    // The two avoid states (+708h / +730h): +1Ch rate or bank, +20h, +24h latch.
    float av_1c{0.0f};
    float av_20{0.0f};
    bool av_latch_24{false};
    int mv_mode_ticks[3]{0, 0, 0};
    int mv_enters{0}, mv_expires{0}, av_latched_ticks{0}, av_turn_picks{0}, av_roll_picks{0};
    int df_latch_sets{0};
    int df_latched_ticks{0};
    int df_reselects{0};
    int df_target_changes{0};
    int df_gun_window_ticks{0};
    bsp::DogfightGunState df_gun{};         // task+314h, packet cc9_dogfight_gun
    // Packet cc9_fighter_gun_lead. The target's acceleration for 00954650's arc
    // arm: the image reads the target's own unit+654h..65Ch, a per-fixed-step
    // finite difference of its velocity (007CEE05-007CEE5B); this host keeps a
    // per-fighter difference of the target's world velocity over the gun tick's
    // dt instead (SUBSTITUTION, labelled; zero on the first tick of a target).
    std::size_t gl_target_plus_one{0};
    float gl_prev_target_v[3]{0.0f, 0.0f, 0.0f};
    // Packet cc9_plane_substitution_sweep: 007CEE05-007CEE5B per fixed step,
    // unit+648h = (position - previous position) / dt, unit+654h = (+648h -
    // +6BCh) / dt with +6BCh the previous step's +648h. Zero until two steps
    // have run.
    float fd_prev_pos[3]{0.0f, 0.0f, 0.0f};
    float fd_vel_648[3]{0.0f, 0.0f, 0.0f};
    float fd_accel_654[3]{0.0f, 0.0f, 0.0f};
    int fd_steps{0};
    // DIAGNOSTIC, packet cc9_fighter_accel_friendly_fire: per fighter, the
    // target's +654h against the gun tick's own velocity difference.
    double ga_a654_sum{0.0}, ga_aold_sum{0.0}, ga_dt_sum{0.0}, ga_sep_sum{0.0};
    float ga_a654_max{0.0f}, ga_aold_max{0.0f};
    int ga_acc_n{0}, ga_arc_654{0}, ga_arc_old{0};
    // Packet cc9_fighter_accel_friendly_fire: the unit+C50h object's +70h
    // friendly-aircraft list (007E11D0) with its own copy of the +7Ch clock,
    // the +80h re-check clock (period +90h = 1.0) and the cached +C8h answer
    // of 007DEDB0.
    std::vector<std::size_t> nb_friend_70;
    float nb_clock_70{4.5f};
    float nb_clock_80{1.5f};
    bool nb_busy_c8{false};
    int ff_busy_ticks{0}, ff_checks{0};
    // Packet cc13-fire (docs/CC13_FIRE_ENVELOPE.md), census under
    // kDogfightFireEnvelopeBound: 007BA760 blocks on an envelope tick, re-aims
    // (009FCDFC), hold-ahead points (E) and PC24/53-bit envelope disagreements.
    int fe_blocked_ticks{0}, fe_reaims{0}, fe_hold_ahead_ticks{0}, fe_pc53_disagree_ticks{0};
    // 009FA248: plan+2D4h = 0 from the fine aim 009F9FC0. 0099E756 then skips
    // the planner's yaw arm 0099E81A. Reset by the planner, as 0099B450 seeds 1.
    bool gl_yaw_mode_2d4_zero{false};
    int gl_lead_ticks{0};
    int gl_arc_ticks{0};
    int gl_fine_aim_ticks{0};
    // Packet cc9_fighter_aim_wiring: fine-aim ticks that added 009FA7E0's
    // distortion, and ticks whose sticks took 009F9FC0's damped cap.
    int gl_aim_distortion_ticks{0};
    int gl_aim_damped_ticks{0};
    double gl_shift_sum{0.0};
    float gl_shift_max{0.0f};
    float gl_muzzle{0.0f};
    // unit+C50h (packet cc9_plane_gunfire): the enemy-aircraft list +50h, the
    // refresh clock +7Ch and the finder clock +84h (U(0,P)+P at midpoints),
    // and the finder's cached choice +B0h.
    std::vector<std::size_t> nb_enemy_50;
    float nb_clock_7c{4.5f};
    float nb_clock_84{3.0f};
    std::size_t nb_found_plus_one{0};
    std::size_t gun_target_74_plus_one{0};   // gun+74h (009FC917), the finder's result
    int df_gun_adopts{0};                    // 009A79A9 calls (kDogfightAimAdoptsGunTargetBound)
    int nb_refreshes{0};
    int nb_scans{0};
    // Packet cc9_near_field_probe census.
    int nf_calls{0};
    // Packet cc9_plane_gun_pass.
    float df_dc{0.0f};                 // approach+DCh
    float df_e0{1.0f};                 // approach+E0h
    int df_early_edges{0};
    int df_early_asks{0};
    bool plane_gun_fire_bc9{false};    // unit+BC9h, the latched gunFire
    int pg_trigger_ticks{0};
    int tg_ticks{0};    // census: 009FC7C0 ticks outside the dogfight arm
    int tg_enemy_ticks{0};   // census: of those, ticks with a non-empty +50h list
    int tg_untasked_ticks{0};   // census: eligible thinks with no modelled task
    int tg_cone_ticks{0};       // census: gun ticks with a non-zero cone
    int tg_steer_ticks{0};      // census: fine-aim steers outside dogfight
    float gun_cone_40{0.0f};    // (approach+1Ch)+40h outside dogfight, this think
    // Packet cc13-steergate (bsp::kBotCommandedDirection68Bound): (approach+1Ch)+68h..+70h,
    // the direction 009FABE0 stores after the cone; zeroed with the cone (009FCE5A-009FCE64).
    // All zero reads as "no state wrote it", and the gate then takes the forward (009FC857).
    float gun_direction_68[3]{0.0f, 0.0f, 0.0f};
    std::uint32_t sg_site_68{0};   // the 009FABE0 call that wrote it (census key)
    int sg_direction_writes{0};    // census: stores
    int sg_heading_fallbacks{0};   // census: 009A1A20 with plan+2CCh != 2 (0099D0A0 arm)
    int sg_gate_dir_ticks{0};      // census: gates measured against a stored direction
    int sg_flip_to_pass{0};        // census: stored direction passes, forward would not
    int sg_flip_to_block{0};       // census: forward would pass, stored direction does not
    int pg_trigger_rises{0};
    int df_head_on_ticks{0};
    int pc_air_brake_overrides{0};
    // Packet cc9_dogfight_moveto.
    float df_station_world[3]{0.0f, 0.0f, 0.0f};   // follow state +30h..38h
    bool df_station_valid{false};
    float df_leader_heading{0.0f};                 // followed unit vtable[50h]
    bool plane_pilot_fires_c24{true};              // unit+C24h PilotFires
    bool plane_pilot_fires_c24_authored{false};    // chain 301: class projection supplied 0/1
    int df_moveto_speed_commands{0};
    float df_moveto_speed_min{1.0e9f};
    float df_moveto_speed_max{0.0f};
    float df_head_on_throttle_min{1.0f};
    int nf_hits{0};
    int nf_attackrun_weaves{0};
    float nf_attackrun_max_offset{0.0f};
    int nf_flyover_slot_writes{0};
    int nf_goaway_hits{0};
    float nf_flyover_slot_min{0.0f};
    float nf_flyover_slot_max{0.0f};
    int dive_bomb_state_ticks[10]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int dive_bomb_arm_ticks{0};
    int dive_bomb_transitions{0};
    int dive_bomb_releases{0};
    int dive_bomb_rounds_pending{0};   // task+424h
    int dive_bomb_rounds_remaining{0};  // 007C1DB0(unit)
    // The approach fields 009C7A80 and the seed 009C3EA0 produce, mirrored so
    // the arm, the two release rules and the census read one set.
    float db_dive_alt_a8{0.0f};      // approach+A8h, the release floor
    // Packet cc9_dive_hit_clock: approach+C4h (task+4BCh), seeded 3600.0
    // (00CFDEB0) at 009C3FD2, advanced += dt at 009C7A8C, zeroed by the hit
    // notice 009C7900. 009C4AA4 reads it (1.0 > C4h: the forced goaway re-roll).
    float db_approach_clock_c4{3600.0f};
    unsigned long long db_hit_clock_resets{0};
    // Packet cc9_difficulty. pilot_skill_index is unit+390h and the pilot bot's
    // bot+34h (0095CCCC's default 1; 009565A0 / 007B8AE0 set it). The approach
    // captures it once, at 009F9D22, as its PilotBot row: db_skill_row_14 is
    // approach+14h held as the row index rather than the row's address.
    int pilot_skill_index{1};
    int db_skill_row_14{1};
    float db_begin_alt_ac{0.0f};     // approach+ACh = ctl+398h, BeginAltRange/1
    float db_alt_span_b0{0.0f};      // approach+B0h = BeginAltRange/2 - /1
    float db_attack_dist_b4{0.0f};   // approach+B4h, the moveto speed argument
    float db_in_range_b8{0.0f};      // approach+B8h == task+4B0h
    float db_planar_bc{0.0f};        // approach+BCh, the planar range
    // 009C8B14-009C8B3B: what 009C8A90's range arm actually measures, which is
    // NOT approach+BCh. The far point is the approach's own aim point, fetched
    // through vtable slot 0 (00D20E10 -> 009C40A0, which copies +4Ch/+50h/+54h),
    // the near point is the unit, and 0042B2F0 takes the full three-vector
    // length. docs/BOMBER_AFTER_TASK.md 10.10, docs/DIVE_BOMB_APPROACH.md 7.
    // Packet cc8_hull_aim_point. The target-reference sub-object every attack
    // approach embeds (approach+30h dive bomb, approach+B4h torpedo), bound in
    // include/bsp/approach_target_ref.hpp. One per ordered target: the body
    // frame offset is drawn once, because slot +104h is 0042BB20 on every class
    // that samples a hull and so never refuses the point it already has.
    bsp::ApproachTargetRefState hull_aim_ref{};
    std::size_t hull_aim_target_plus_one{0};
    std::uint32_t hull_aim_seed{0};
    // Packet cc13-aimfreeze3 (include/bsp/approach_aim_freeze_cc13.hpp): the task whose
    // aim_point_freeze_009fada0 call latched hull_aim_ref (0 none, 1 dive, 2 level bomb,
    // 3 torpedo, 4 kamikaze, 5 rocket, 6 depth charge), and the last point returned
    // unlatched (sub+1Ch with its lead, which 009FAF90 transforms again).
    std::uint8_t aim_freeze_task{0};
    std::array<float, 3> aim_freeze_last_point{{0.0f, 0.0f, 0.0f}};
    // Packet cc9_hull_axis: the target the in-range print last fired for, so
    // it fires once per (attacker, target). Diagnostic only, behind the switch.
    std::size_t hull_aim_range_printed_plus_one{0};
    float db_aim_point_3d{0.0f};
    float db_bearing_c0{0.0f};       // approach+C0h
    float db_aim_point_height_50{0.0f};   // approach+50h
    float db_release_range_d4{0.0f};  // approach+D4h
    float db_lead_high_5c{0.0f};     // (approach+14h)->+5Ch
    float db_gain_high_60{0.0f};     // (approach+14h)->+60h
    bool db_in_range_d0{false};      // approach+D0h == task+4C8h
    bool db_has_bomb_d1{false};      // approach+D1h == task+4C9h
    // squadron+370h, the attack mode. Its faithful owner is the squadron
    // (`plane+9D4h`), not the unit: 0099B740 writes it through `bot+2FCh` and
    // 009C83F8 reads it through `task+404h`, which is the same object. This
    // host keeps a per-slot copy that the flight leader broadcasts to its
    // squadron's members every think, so every member holds the value the
    // leader wrote. Labelled: the copy is a hole in OWNERSHIP only, the value
    // is the leader's. Seeded kForced because the scene's attack order reaches
    // 008A4C41 -> 007ED430(2); the leader's first think then sets it to 1.
    bsp::PilotAttackMode db_attack_mode_370{bsp::PilotAttackMode::kForced};
    // sqn+3D0h[0] == this unit, the test at 0099B757. Only the leader's think
    // writes the mode.
    bool db_is_flight_lead{false};
    bool db_flight_lead_resolved{false};
    int db_mode_ticks[3]{0, 0, 0};
    int db_mode_changes{0};
    int db_mode_first_set_tick{-1};
    // Cached inputs of the two predicates, sampled where they are computed so
    // the exit log can print what actually decided the edge.
    bool db_dbg_engaged{false};
    bool db_dbg_break_off{false};
    float db_dbg_break_off_distance{0.0f};
    float db_dbg_break_off_threshold{0.0f};
    // Where a spent bomber ends up, and how low it gets afterwards.
    int db_spent_exit_logs{0};
    int db_post_done_ticks{0};
    float db_post_done_min_alt{-1.0f};
    int db_approach_returns{0};
    int db_approach_return_tick{-1};
    // The per-state fields the two aim states and flyabove carry.
    float db_aim_rearm_1c{0.0f};     // aimdive/aimglide state+1Ch
    float db_glide_travel_20{0.0f};  // aimglide state+20h
    bool db_aim_alive_19{false};     // aimdive state+19h
    bool db_aim_pull_out_18{false};  // aimdive state+18h
    bool db_flyabove_ready_19{false};
    bool db_flyabove_can_dive_18{false};
    bool db_flyabove_leave_1a{false};
    float db_turn_roll_18{0.0f};     // turndown state+18h, 009C7800's output
    // The attackrun state's own fields, 009C4220.
    float db_attackrun_timer_1c{0.0f};
    float db_attackrun_period_18{1.0f};
    float db_attackrun_offset_20{0.0f};
    int db_attackrun_ticks{0};
    int db_attackrun_rerolls{0};
    int db_latch_closed_tick{-1};
    float db_attackrun_heading_last{0.0f};
    float db_attackrun_throttle_last{0.0f};
    float db_attackrun_alt_last{0.0f};
    float db_range_at_second[16]{};
    int db_range_samples{0};
    bool db_turndown_latch_1c{false};  // turndown state+1Ch, 009C465D
    int db_turndown_ticks{0};
    int db_turndown_roll_writes{0};
    int db_turndown_pitch_writes{0};
    int db_turndown_latched_tick{-1};
    float db_turndown_bank_last{0.0f};
    float db_turndown_roll_last{0.0f};
    float db_turndown_pitch_last{0.0f};
    // The two pose angles 009C7EA0 reads, sampled over the turndown: the gate
    // out of it needs pose+C64h under the -1.3 at 00D1F98C, or under -1.0 with
    // |pose+C68h| past the 135 degrees at 00D20E80.
    float db_turndown_pose_c64_last{0.0f};
    float db_turndown_pose_c64_min{0.0f};
    // Census.
    float db_dive_entry_alt{-1.0f};
    float db_dive_entry_pitch{0.0f};
    float db_release_alt{-1.0f};
    float db_release_speed{-1.0f};
    float db_release_range{-1.0f};
    float db_aim_error_last{0.0f};
    // 009C6493's height and the span 009C65FD draws from it, the one pair all
    // three flyabove flags key on.
    float db_flyabove_height{0.0f};
    float db_flyabove_span{0.0f};
    // 009C6385-009C63E6: the range and the bearing to the aim point predicted
    // three seconds ahead, which is what the fly-above's flags and its heading
    // arm all read. Packet cc8_dive_heading.
    float db_flyabove_lead_range{0.0f};
    float db_flyabove_lead_bearing{0.0f};
    // The aimdive trace. Endpoint values cannot tell a dive that never pointed
    // at the target from one that pointed and was too slow, so the aim states
    // sample their own geometry the way the run-in samples its range.
    // 009C62B0's and 009C5180's heading arms.
    int db_flyabove_tick_ticks{0};
    int db_aimglide_tick_ticks{0};
    // 009C18C0 and 009C1FD0, the two approach states the arm's virtual tick
    // dispatch at 009C884C reaches and this host never ran.
    // docs/DIVE_BOMB_APPROACH.md.
    int db_moveto_tick_ticks{0};
    int db_follow_tick_ticks{0};
    // 009C7240 and 009C7270, the dive-bomb done/prepare state, packet
    // cc8_done_state. `plan_mode_26c` is the value 009C1FE2 stamps into the
    // pilot command block at the head of every follow tick. Nothing in this
    // host reads it, and docs/BOMBER_AFTER_TASK.md section 6c says why nothing
    // in the IMAGE acts on it for a flight leader either: the only reader,
    // 0099D300's gate A, needs three further conditions that a leader's follow
    // tick never produces. It is carried to be measured, not to be acted on.
    int db_done_entries{0};
    int db_done_tick_ticks{0};
    int db_done_placed_ticks{0};
    float db_done_entry_alt{0.0f};
    float db_done_last_alt{0.0f};
    float db_done_last_hdg{0.0f};
    int plan_mode_26c{0};
    // 009FBA50's terms, sampled through the run-in.
    float db_cruise_span[8]{};
    float db_cruise_gain[8]{};
    float db_cruise_scale[8]{};
    float db_cruise_base[8]{};
    float db_cruise_clamped[8]{};
    int db_cruise_samples{0};
    int db_goaway_tick_ticks{0};
    float db_goaway_pitch_last{0.0f};
    // Packet cc8_dive_goaway: the climb census. `pitch_last` on its own cannot
    // tell a state that never commanded a climb from one whose command was
    // eaten later, so the altitude the aircraft actually reaches goes beside it,
    // and so do the two operands of 009C7F00's second arm - the ceiling
    // min(ctl+398h, approach+ACh + approach+50h) and the deficit ceiling - Y,
    // which 009C4B36 is about to be shown to feed to the climb curve as well.
    float db_goaway_pitch_max{0.0f};
    float db_goaway_alt_first{-1.0f};
    float db_goaway_alt_last{0.0f};
    float db_goaway_alt_max{-1.0f};
    float db_goaway_ceiling_last{0.0f};
    float db_goaway_deficit_last{0.0f};
    int db_goaway_complete_ticks{0};
    // The goaway state's OWN +20h. 009C7F00's completion rule reads the goaway
    // state, not the aimglide's, and conflating them made goaway finish on its
    // first tick.
    float db_goaway_travel_20{0.0f};
    // Packet cc9_goaway_turn: the goaway state's own +18h..+2Ch, persisting
    // across entries as the native object does (the enter 009C4950 rewrites
    // +18h/+20h/+24h only). docs/DIVE_BOMB_GOAWAY_TURN.md.
    bsp::DiveBombGoAwayTurnState db_goaway_turn{};
    int db_goaway_enters{0};
    // Packet cc13-goaway: 007B5BE0's goaway target term (the target's longest
    // category 6/5/1 range) at the dive / torpedo / level goaway enters,
    // measured whether or not a target-term switch is on. Log only.
    const char* ga_term_kind{nullptr};
    int ga_term_enters{0};
    int ga_term_targets{0};      // a target with a gunnery row
    int ga_term_moved{0};        // term > base: the switch would move +20h/+18h
    float ga_term_aa_max{0.0f};
    float ga_term_base_min{0.0f};
    float ga_term_base_max{0.0f};
    int db_goaway_flag0_ticks{0};       // ticks that reached 009C4CBA
    int db_goaway_countdown_ticks{0};   // 009C4A86 ran
    int db_goaway_rerolls{0};           // 009C4CF1 arm ran
    int db_goaway_first_reroll_tick{-1};
    int db_goaway_bank_ticks{0};        // 009C4DAF arm
    int db_goaway_heading_ticks{0};     // 009C4E05 arm
    int db_goaway_first_heading_tick{-1};
    int db_goaway_side_writes{0};       // 009FD570 changed +18h
    float db_goaway_bank_min{0.0f};
    float db_goaway_bank_max{0.0f};
    float db_goaway_heading_last{0.0f};
    float db_goaway_heading_err_max{0.0f};  // |heading - own heading|, heading arm
    int db_flyabove_heading_writes{0};
    float db_flyabove_heading_last{0.0f};
    // Packet cc8_dive_flyover: flyabove+1Ch, the roll-in LATCH of 009C6919, and
    // the bank arm around it. The latch is per-state - 009C629E clears it on the
    // fly-over's enter - and once set it both skips the bank evaluation
    // (009C6865) and suppresses the heading write (009C6DDA).
    // Packet cc8_dive_flyover: the aimglide's pull-out latch, state+18h =
    // whole-object +76Ch. 009C57FF sets it, 009C4F0C clears it on the enter,
    // and 009C86B2/009C86BF read it to send the state to goaway - which is the
    // only way an aircraft that aborted its dive gets a second attack run.
    bool db_aimglide_pull_out_76c{false};
    float db_glide_bearing_abs_18{0.0f};  // [ESP+18h], the arm's input
    float db_glide_bearing_max{0.0f};     // the census arm A needs either way
    int db_aimglide_pull_outs{0};
    bool db_flyabove_bank_latch_1c{false};
    int db_flyabove_side_20{0};   // flyabove+20h = task+798h, 0 / +1 / -1
    float db_flyabove_dead_band_t{0.0f};
    float db_flyabove_along_track{0.0f};
    float db_flyabove_cross_track{0.0f};
    int db_flyabove_bl_ticks{0};        // ticks with BL set at 009C67BF
    int db_flyabove_latched_ticks{0};   // ticks the latch suppressed the heading
    int db_flyabove_latch_tick{-1};     // the arm tick the latch first closed on
    // Packet cc8_dive_entry. Where the dive-entry altitude comes from is a
    // question about the altitude at each HAND-OVER, and no existing census
    // carries it: db_dive_entry_alt is the aimdive entry only, and the geo
    // trace prints one slot and starts at the turndown. One record per state
    // change, for every dive bomber.
    struct DbTransition {
        int tick{0};
        signed char from{-1};
        signed char to{-1};
        float alt{0.0f};
        float range{0.0f};
        float cmd_alt{0.0f};
        // Packet cc8_dive_heading: the three flyabove numbers that decide the
        // race, latched at the hand-over so the arm that fired can be named
        // instead of inferred. `span` is 009C65FD's, `f18` is 009C680E's
        // can-dive and `f19` is 009C67B0's roll-in permission.
        float span{0.0f};
        float b_height{0.0f};
        signed char f18{-1};
        signed char f19{-1};
    };
    static constexpr int kDbTransitions = 12;
    DbTransition db_transitions[kDbTransitions]{};
    int db_transition_count{0};
    // 009C6E10-009C6F91, the flyabove's altitude arm.
    int db_fa_alt_calls{0};
    int db_fa_level_ticks{0};
    float db_fa_limit_c{0.0f};
    float db_fa_band{0.0f};
    float db_fa_target_last{0.0f};
    float db_fa_ref_last{0.0f};
    float db_fa_err_last{0.0f};
    float db_fa_err_first{0.0f};
    float db_fa_pitch_last{0.0f};
    // The geometry when the turndown starts, which is what decides where the
    // dive begins.
    float db_turndown_entry_range{-1.0f};
    float db_turndown_entry_bearing{0.0f};
    float db_aimdive_entry_range{-1.0f};
    float db_aimdive_entry_bearing{0.0f};
    float db_aimdive_min_range{-1.0f};
    float db_aim_trace_range[12]{};
    float db_aim_trace_error[12]{};
    float db_aim_trace_bank[12]{};
    float db_aim_trace_bearing[12]{};
    int db_aim_trace_samples{0};
    int db_aim_state_ticks{0};
    // 009C58D0's steering census.
    int db_aimdive_steer_ticks{0};
    float db_aimdive_pitch_last{0.0f};
    float db_aimdive_roll_last{0.0f};
    float db_aimdive_bearing_last{0.0f};
    float db_aim_heading_last{0.0f};   // 009C4F80's result, the aim heading
    // The closest the aim error came to the 25.0 m gate at 00CE3880 while an aim
    // state owned the tick, with the geometry at that sample. -1 means never.
    float db_aim_error_abs_min{-1.0f};
    float db_aim_error_min_range{-1.0f};
    float db_aim_error_min_alt{-1.0f};
    // Packet cc8_dive_geometry: a per-tick window over the split-S and the
    // start of the dive, to settle which manoeuvre the host flies against the
    // one 009C44F0 commands. Additive; nothing here feeds a command.
    static constexpr int kDbGeoSamples = 140;
    struct DbGeoSample {
        int tick;
        int state;
        float pitch_c64;
        float bank_c68;
        float heading_c6c;
        float altitude;
        float range;
        float bearing_err;
        float aim_heading;
        float roll_input;
        float roll_cmd;
        float pitch_cmd;
        int pitch_mode_2d0;
        int heading_mode_2cc;
    };
    DbGeoSample db_geo[kDbGeoSamples]{};
    int db_geo_samples{0};
    int db_blocked_no_latch{0};      // ticks with approach+D0h clear
    int db_blocked_no_bomb{0};       // ticks with approach+D1h clear
    // Packet cc8_dive_release, item 1: which of 009C8634-009C8682's three arms
    // ends the aimdive, with the three flags as the transition read them (that
    // is, BEFORE the same arm's tick can rewrite them). Additive census only.
    int db_aimdive_entries{0};
    int db_aimdive_exits{0};
    int db_aimdive_exit_alive{0};    // 009C8645 taken: +19h was 0
    int db_aimdive_exit_nobomb{0};   // 009C8668 taken: approach+D1h was 0
    int db_aimdive_exit_pullout{0};  // 009C8671 not taken: +18h was set
    int db_aimdive_exit_other{0};
    int db_aimdive_run_ticks{0};     // ticks in the current aimdive run
    int db_aimdive_first_run{-1};    // ticks the first aimdive run lasted
    int db_aimdive_last_run{-1};
    // approach+D8h/+DCh/+E0h and the two quantities the aimdive tick takes from
    // them: the second sqrt at 009C5A40 ([ESP+5Ch]) and the bearing 009C5AF1
    // leaves in [ESP+18h].
    float db_run_in_origin[3]{};
    float db_impact_fall_time{0.0f};
    float db_impact_planar_5c{-1.0f};
    float db_impact_bearing_18{0.0f};
    // Packet cc8_dive_glide. The aimglide tick builds a THIRD planar vector
    // from the same point, `impactPoint - unit` at 009C5207-009C522E, and its
    // magnitude at 009C52C7-009C5303 is the [ESP+14h] the 120 m gate and the
    // lead read. The aimdive never needs it, which is why no slot carried it.
    float db_impact_throw_14{0.0f};
    // The aimglide release census, packet cc8_dive_glide: how far down the
    // 009C5689-009C5755 chain each call got, and the lead when it was computed.
    int db_glide_calls{0};
    int db_glide_gate_reached[7]{};
    float db_glide_lead_last{0.0f};
    float db_glide_ratio_last{0.0f};  // aimglide [ESP+24h], cc9_dive_throttle
    float db_glide_pitch_last{0.0f};  // aimglide cmd+2BCh, cc9_aimglide_pitch
    float df_moveto_cmd_alt_last{0.0f};  // cc9_dive_modes, generic moveto
    int db_glide_yaw_ticks{0};        // aimglide yaw arm ticks, cc9_aimglide_pitch
    float db_glide_lead_min{0.0f};      // the most negative lead seen
    float db_glide_bearing_min{-1.0f};
    int db_glide_releases{0};
    int db_glide_rounds{0};
    // Packet cc8_dive_glide: the bomb side of the release, which until now had
    // no fields of its own. The summary's `bombs_spawned` column was printing
    // torpedo_drops_spawned - a different field with a different producer.
    int db_bay_requests_accepted{0};
    int db_bay_requests_refused{0};
    int db_bombs_spawned{0};
    // The 009C5B01-009C5B48 abort's own operands the first time it fires.
    int db_abort_fires{0};
    int db_abort_first_tick{-1};
    float db_abort_d4{0.0f};
    float db_abort_h14{0.0f};
    float db_abort_range{0.0f};
    float db_abort_pitch{0.0f};
    // The torpedo bot task (kind Eh) this ordered aircraft runs, when its
    // class carries torpedo ordnance (kind 2Bh). docs/TORPEDO_TASK_ARM.md.
    // The task object itself is src/bot_tasks.cpp's; what lives here is the
    // per-slot state the arm 009D4850 reads and writes.
    bool torpedo_task_installed{false};
    bsp::TorpedoState torpedo_state{bsp::TorpedoState::kNone};
    // 00999AA0 -> 009D3270 resets of approach+134h (packet cc9_plane_hit_task_notify).
    unsigned long long torpedo_hit_clock_resets{0};
    int torpedo_state_ticks[8]{0, 0, 0, 0, 0, 0, 0, 0};
    int torpedo_arm_ticks{0};
    // Packet cc9_l60_torpedo_disarmed: 009D29E0 runs (leader and target known),
    // its close arm (cmd+2C8h written), the least ratio and the last +8Ch.
    int torpedo_disarmed_ticks{0};
    int torpedo_command_writes{0};   // kTorpedoCommandWritesBound: binding writes applied
    int torpedo_done_enters_2530{0};   // kTorpedoDoneFormationBound: 009D2530 runs
    int torpedo_3e8_republishes{0};    // kTorpedoDoneFormationBound: 009BFDA6 writes
    int torpedo_disarmed_close_ticks{0};
    float torpedo_disarmed_min_ratio{1.0e30f};
    float torpedo_disarmed_blend_8c{1.0f};
    int torpedo_releases{0};
    // Packet cc9_torpedo_release_counter: the task-level releases above that
    // came from an aircraft the gunnery host already has dead. Such a release
    // spawns nothing (the image refuses it at 007CEA1C), so the drop count is
    // `summary mission gunnery torpedo_drop drops`, not torpedo_releases.
    // Diagnostic only: torpedo_releases keeps its value, which the task reads.
    int torpedo_releases_dead{0};
    int torpedo_transitions{0};
    // task+424h, the manual-release budget the arm's passthrough spends.
    int torpedo_rounds_pending{0};
    // prepare+98h (task+7D8h), the countdown 009D49A0 raises.
    float torpedo_drop_timer{-1.0f};
    // done+98h, the done state's own countdown (kTorpedoStateEnterTimersBound).
    // 009D49A0 writes prepare+98h only, so the image's done timer stays at the
    // -1.0 its enter 009D2530 stores.
    float torpedo_done_timer_98{-1.0f};
    // Packet p21_torp_enter counters. armed: 009D49A0 wrote the timer since the
    // last done/prepare enter; a release from the done/prepare tick that read
    // a positive timer without that arm is a stale-timer drop.
    bool torpedo_timer_armed_since_enter{false};
    float torpedo_last_drop_timer_read{-1.0f};
    int torpedo_stale_timer_drops{0};
    int torpedo_done_state_drops{0};
    int torpedo_aim_enters{0};
    int torpedo_aim_enters_complete_stale{0};   // +2Ch already set at an aim enter
    // Packet p22_los_007df360: 007DF360 calls, screened answers, terrain 1s cleared.
    int torpedo_los_calls{0};
    int torpedo_los_screened{0};
    int torpedo_los_clears{0};
    int torpedo_los_zero_length{0};   // a screening ship with class+A0h == 0
    int torpedo_cruise_passes{0};     // packet p22_cruise_pass: 009D4BFC dt = 0 passes
    // The values 009D3420 produces. docs/TORPEDO_APPROACH_UPDATE.md reads that
    // routine; the reconstruction in torpedo_approach_update.hpp runs here and
    // these mirror its outputs so the arm and the census read one set.
    bool torpedo_aim_flag_529{false};    // approach+131h
    bool torpedo_attack_flag_52a{false};  // approach+132h
    float torpedo_engage_range_8c{0.0f};  // task+484h == approach+8Ch
    float torpedo_engage_limit_90{0.0f};  // task+488h == approach+90h
    int torpedo_blocked_by_engaged{0};
    // Packet cc9_jm05_blocked_engaged: which clause of 009D3210 refused, print-only:
    // [0] no engage target (009D3222), [1] a wing member while the mode is not 2
    // (009D3245), [2] the leader's range test 009D325B.
    int torpedo_blocked_clause[3]{0, 0, 0};
    int torpedo_blocked_by_arm{0};
    // Packet cc9_l57_torpedo_wingmen: 0099B6CD's 007BBBA0 through the 009D49DA
    // fallthrough, and the members 007EEFDD gave one order (leader block).
    int torpedo_fallthrough_releases{0};
    int torpedo_leader_block_raised{0};
    // unit+C58h, the queued release-order count BSP_PilotBot_Tick 0099ACD0
    // spends at 0099AF81 by offering it to each task's vtable +24h. Its raiser
    // was not found: contract: unread, so it stays at zero here.
    int torpedo_release_orders_c58{0};
    int torpedo_arm_offers{0};
    int torpedo_arm_blocked_no_order_0099af53{0};
    // unit+C25h, the byte 007C0EE2 raises before it calls the issuer, and the
    // issue path's own bookkeeping. docs/TORPEDO_RELEASE_ORDERS.md.
    bool torpedo_release_pending_c25{false};
    // Packet cc9_plane_device_walk. The weapon controller's device list
    // (unit+974h, count unit+994h) holds every child answering IsKindOf(25h):
    // MBombPlatform (25h, `Type` "BombPlatform") and MMultipleBombPlatform
    // (26h, "MultiBombPlatform"). Only a single rack can answer busy
    // (006E3CD0 reads its +498h; 006E4360 is XOR AL,AL), and +498h is set by
    // the fire slot 006E3550 and cleared only by the constructor 006E3C00.
    bool rack_census_done{false};
    int rack_single_count{0};
    int rack_multi_count{0};
    int rack_single_fired{0};          // racks whose +498h is set
    int rack_device_walks{0};          // 007CEB00 walks (cleanup arm only)
    int rack_device_walks_busy{0};
    // Packet cc9_release_issue_stage. The rack's own fields (serializer 006E4A60
    // names them): +484h `ammo`, +494h `toRepeatTime`, +498h `dropBombs`. The host
    // keeps one rack state per aircraft (its single racks taken together).
    bool rack_dropping{false};         // dropBombs +498h
    float rack_to_repeat{0.0f};        // toRepeatTime +494h
    int rack_ammo{-1};                 // ammo +484h, -1 until the census
    // Packet cc9_squadron_ordnance_state: the racks 007EDAD0 walks, read once.
    bool ordnance_racks_read{false};
    std::vector<PlaneOrdnanceRack> ordnance_racks;
    int rack_drops{0};                 // 006E4D50 drops (host spawns)
    int rack_gate_refused{0};          // 007CC8E0 / 007C7600 said no
    int rack_requests_deferred{0};     // 007BBBA0 requests whose spawn waits
    int rack_bomb_drops{0};            // packet cc9_release_issue_stage_vals
    int player_bomb_releases{0};       // packet cc9_player_bomb_release, 006082D0
    int rack_rounds_authored{0};       // packet cc9_dive_bomb_carried_rounds
    // Packet cc9_plane_scene_equipment: the record's bag `Equipment`, which
    // 007CDF20 stores at [unit+C54h]; -1 when the creator carried none.
    int bag_equipment{-1};
    // Packet cc9_mavis_rack_drops: each single rack's own ammo +484h, in census
    // order (the child-list order is unread), and the rack the last issue fired.
    std::vector<int> rack_authored_per_rack;  // authored Ammo per single rack, -1 absent
    std::vector<int> rack_ammo_per_rack;      // seeded with rack_ammo
    int rack_active{-1};
    // Packet cc9_rack_bullet_kind: each single rack's projectile class (the
    // descriptor 007C0D90 asks vtable[8] of: 2Ah bomb, 2Bh torpedo, 2Ch depth
    // charge, 33h rocket) and its RepeatTime (desc+E0h), in census order.
    std::vector<int> rack_bullet_class_per_rack;
    // Chain 324: identities captured by the equipment-resolved ammo census.
    std::vector<SingleRackCensusIdentity> rack_identity_per_rack;
    // Packet cc9_lua53_kamikaze_impact: the rack's Bullet[1].Bullet id itself (the
    // vector above holds the descriptor's kind), census order. -1 when absent.
    std::vector<int> rack_bullet_id_per_rack;
    // Packet cc13-para (bsp::kLevelBombParatrooperDropBound): each single rack's
    // 006E3F90 state, census order: the flag rack+4F8h, the point +4FCh..+504h and
    // the velocity +508h..+510h.
    std::vector<bsp::gameplay_seg46_cc13::RackParatrooperTarget> lb_para_rack;
    unsigned long long lb_para_points_set{0};      // 006E3F90 calls
    unsigned long long lb_para_drop_targets{0};    // 006E541F blocks run
    // Packet cc13-para2 (bsp::kParatrooperRoundBound): each 31h rack's hung round
    // (006E3E70 -> 007AB8C0), census order; `lb_para_hung_set` is 1 while one hangs.
    std::vector<bsp::gameplay_seg46_cc13::ParatrooperState> lb_para_hung;
    std::vector<unsigned char> lb_para_hung_set;
    bool lb_para_hung_seeded{false};
    std::vector<float> rack_repeat_per_rack;
    // Packet cc9_rack_device_reload: each single rack's ReloadTime +48Ch (the
    // equipment entry's, 006E3A20) and its countdown +490h, census order.
    std::vector<float> rack_reload_time_per_rack;
    std::vector<float> rack_reload_left_per_rack;
    int rack_device_reloads{0};
    int rack_drops_unspawned{0};       // drops whose host torpedo spawn returned false
    // GUNNERY 133.3 / 137, a diagnostic (no behaviour): single-rack drops since
    // the last issue set dropBombs, drops after the first in that episode (the
    // ones the unmodelled CanFire gates and RepeatTime would pace), and drops
    // that left rounds on the rack that dropped.
    int rack_episode_drops{0};
    int rack_repeat_drops{0};
    int rack_drops_leaving_rounds{0};
    // GUNNERY 138, a diagnostic: the single-rack tick's calls and early returns.
    int rack_tick_calls{0}, rack_ret_census{0}, rack_ret_dead{0}, rack_ret_idle{0};
    int rack_ret_level{0}, rack_ticks_after_drop{0};
    float rack_last_drop_t{-1.0f}, rack_last_tick_t{-1.0f};
    // Chain 414: publish 007CE90D's existing sample for the HUD altitude gate.
    // Until that native writer runs, do not substitute world Y or a sea height.
    float plane_height_above_surface_9b4{};
    bool plane_height_above_surface_ready{false};
    float plane_height_rate_9b8{0.0f}; // unit+9B8h, packet cc9_units_contracts
    float hull_heading_1050{0.0f};     // unit+1050h, written at 00826C56
    float occupant_timer_1158{0.0f};   // unit+1158h, 0 from 00823C30
    int db_bomb_requests_logged{0};    // diagnostic line cap
    int torpedo_orders_issued{0};
    int torpedo_orders_issue_ticks{0};
    int torpedo_peak_release_orders_c58{0};
    // The 007CE9FD stage's own two fields. docs/TORPEDO_ISSUE_TIMING.md.
    // unit+C28h, the interval countdown; 007D5D20 leaves it at the
    // constructor's zero, so the first fixed step expires it at once.
    float torpedo_issue_interval_c28{0.0f};
    // unit+C20h, the pending release-request count. 007BBC00 raises it by one
    // at the tail of 007BBBA0 BSP_Unit_RequestOrdnanceRelease; 007CEA82 spends
    // one per issue. 007D625B seeds it to zero (EBX from 007D619A XOR).
    int torpedo_issue_requests_c20{0};
    int torpedo_issue_stage_ticks{0};
    int torpedo_issue_stage_guard_blocked{0};
    // The first of the five 007CE9FD guards that held: 0 = 007CEA02 app
    // state, 1 = 007CEA0F unit+9E0h, 2 = 007CEA1C unit+C3Ah, 3 = 007CEA29
    // unit+5Dh, 4 = 007CEA33-4B unit+900h outside 4..7. Diagnostic only.
    int torpedo_issue_stage_guard_by[5]{};
    int torpedo_issue_stage_last_mode_900{-1};
    int torpedo_issue_stage_waiting{0};
    int torpedo_issue_stage_issues{0};
    int torpedo_issue_stage_cleanups{0};
    int torpedo_issue_first_issue_tick{-1};
    int torpedo_release_requests_007bbba0{0};
    // unit+DECh, the plane's actuator block. Packet cc8_torpedo_release_spawn;
    // docs/TORPEDO_RELEASE_SPAWN.md. Built at 007EABC0 by
    // BSP_Plane_ReadPropertyBag and stepped by 007DE3A0. Channel C is the
    // ordnance bay 007BBBA0 drives.
    //
    // SUBSTITUTION, labelled: the native writer of the channel's enabled byte
    // at +60h and of its rate at +6Ch is unread, so channel C is enabled here
    // with a nominal 1.0 per second travel. The request forces the value to
    // 1.0f on the same frame regardless, so the rate only governs how long the
    // bay takes to close again.
    bsp::PlaneActuatorBlock actuator_block_dec{};
    // Packet cc9_wing_fold (kPlaneWingFoldElevatorBound): channel +44h of the
    // block is PartAnims.Wings (the host's channel_a). The class record is read
    // once; the last +900h state the 007C11E0 stand-in has seen (-1: none yet).
    bool wing_fold_read{false};
    std::int32_t wing_notified_state_900{-1};
    int torpedo_bay_requests_accepted{0};   // 007BBBA0 moved the channel
    int torpedo_bay_requests_refused{0};    // a guard rejected it
    int torpedo_drops_spawned{0};           // a round actually left the plane
    // ctl+370h, the pilot control block's attack mode. 0099B740 raises it to
    // kAttack on the flight leader's cruise-profile tick; 009D3F60 row 1 holds
    // a task in prepare while it is kHold.
    bsp::PilotAttackMode torpedo_attack_mode_370{bsp::PilotAttackMode::kHold};
    int torpedo_attack_mode_raised_tick{-1};
    // ctl+370h's whole history. docs/TORPEDO_ATTACK_MODE.md: only three sites
    // in the image write this field on the pilot control block, and neither
    // route to 0 is one a torpedo task runs.
    int torpedo_mode_ticks[3]{0, 0, 0};       // ticks spent at hold/attack/forced
    int torpedo_mode_changes{0};
    int torpedo_mode_hold_with_engaged{0};    // ticks at 0 while engaged
    int torpedo_mode_lowered_009a285e{0};     // the closetoship countdown
    int torpedo_mode_message_007f0068{0};     // the BCh message arm
    int torpedo_prepare_entries{0};
    int torpedo_prepare_first_tick{-1};
    int torpedo_blocked_no_order_first_tick{-1};
    // The closetoship task's +550h/+43Ch pair and the BCh message, neither of
    // which this host produces. Held so the bound rules are exercised the
    // moment a producer appears.
    bool torpedo_closetoship_task_installed{false};
    float torpedo_closetoship_timer_550{0.0f};
    bool torpedo_closetoship_latch_43c{false};
    bool torpedo_pending_mode_message_bc{false};
    unsigned char torpedo_mode_message_payload{0};
    bool torpedo_is_flight_lead{false};
    // The sector scan of 009D3420, once it has a sampler.
    int torpedo_scans_run{0};
    int torpedo_clear_sectors_last{-1};
    int torpedo_home_sector_last{-1};
    float torpedo_turn_offset_last{0.0f};
    // The issue gate 007EEF40 compares, as the host last evaluated it.
    float torpedo_issue_threshold_390{0.0f};
    float torpedo_armed_fraction_374{0.0f};
    bool torpedo_issue_gate_open{false};
    // The approach object embedded at task+3F8h.
    bsp::TorpedoApproachState torpedo_approach{};
    // Packet cc13-torpranges. The torpedo moveto state (task+544h) +30h/+34h/+38h,
    // stored by 009BDE80 at 009D48CF every arm tick (kTorpedoMoveToRangesBound).
    float torpedo_moveto_alt_30{0.0f};
    float torpedo_moveto_alt_34{0.0f};
    float torpedo_moveto_range_38{0.0f};
    // Packet cc9_aim_error_draw: set once 009C3DA0 / 009D02A0 have written the
    // sub-object; dive approach+C8h, the projtime time error.
    bool aim_error_drawn{false};
    float db_aim_time_error_c8{0.0f};
    // Packet cc9_planner_heading_writes: the torpedo attackrun state's +18h,
    // +1Ch and +20h. 009D06D0 seeds +1Ch = -U(0,1); the midpoint, as every
    // other draw in this host. 009D0790 resets +18h and +20h on each entry.
    float torpedo_ar_period_18{1.0f};
    float torpedo_ar_countdown_1c{-0.5f};
    float torpedo_ar_offset_20{0.0f};
    int torpedo_ar_heading_writes{0};
    int torpedo_ar_rerolls{0};
    int torpedo_moveto_heading_writes{0};
    // Packet cc9_attackrun_squadron_probe: the mode-0 probe on the re-roll arm.
    int torpedo_ar_probe_calls{0};
    int torpedo_ar_probe_nonzero{0};
    float torpedo_ar_probe_max_raw{0.0f};   // |product| * pi/6, radians
    int torpedo_approach_ticks{0};
    int torpedo_approach_no_target_ticks{0};
    int torpedo_approach_replans{0};
    int torpedo_aim_ticks{0};
    // Packet cc9_torpedo_release_orders, observation only: aim ticks on which
    // each flag of the 009D202C-009D2233 chain was true (state, lead, alt,
    // bank, cone, all five), and the aim-tick extremes of the inputs.
    int torpedo_aim_gate_ticks[6]{0, 0, 0, 0, 0, 0};
    float torpedo_aim_min_alt{1.0e9f};
    float torpedo_aim_min_f14{1.0e9f};
    float torpedo_aim_last_fall_lead{0.0f};
    float torpedo_aim_last_bank{0.0f};
    // The first tick on which the engaged pair went non-zero, or -1.
    int torpedo_first_engaged_tick{-1};
    float torpedo_range_min{-1.0f};
    float torpedo_range_last{0.0f};
    // cc8_torpedo_retire item 1. Every input of 009D4C10's binding, latched on
    // the FIRST tick it returns true, so the arm that retires the bomber is read
    // from a run instead of inferred. Additive: nothing here feeds behaviour.
    int torpedo_breakoff_first_tick{-1};      // slot arm tick, or -1
    // Packet cc9_squadron_spent_ordnance_rtb: the last answer of the task's
    // break-off slot +1Ch (009D4C10), which 007EE6B0 asks.
    bool torpedo_breakoff_now{false};
    int torpedo_breakoff_true_ticks{0};
    int torpedo_breakoff_arm{0};              // 2 = target arm, 5 = range arm
    int torpedo_breakoff_state{0};
    int torpedo_breakoff_target_plus_one{0};
    int torpedo_breakoff_approach_ticks{0};
    int torpedo_breakoff_no_target_ticks{0};
    bool torpedo_breakoff_has_target{false};
    bool torpedo_breakoff_target_marked{false};
    bool torpedo_breakoff_in_attack_state{false};
    bool torpedo_breakoff_attack_flag_52a{false};
    bool torpedo_breakoff_has_ordnance_132{false};
    float torpedo_breakoff_range_90{0.0f};    // what the binding fed as distance
    float torpedo_breakoff_limit_90{0.0f};    // task+488h's twin, the image's cell
    float torpedo_breakoff_prev_range{0.0f};  // range_90 one transition tick back
    float torpedo_breakoff_safe_dist{0.0f};   // safe_distance_438() on that tick
    float torpedo_prev_range_90{0.0f};
    // cc8_torpedo_retire item 5: the crossing angle at ATTACKRUN ENTRY, to be
    // set against the 5 to 12 degrees the closest-approach census measures at
    // the end of the swim. The target's number is the hull POSE heading,
    // atan2(pose_row2.x, pose_row2.z), which is the same quantity the gunnery
    // census differences the round's track against. It is NOT the ship-ai
    // step heading: docs/TORPEDO_AFTER_THE_DROP.md section 13 measures that
    // control-block field 0.7 to 0.9 rad away from the hull's pose.
    float torpedo_attackrun_yaw_c6c{0.0f};
    float torpedo_attackrun_own_pose{0.0f};
    float torpedo_attackrun_target_pose{0.0f};
    float torpedo_attackrun_crossing{-1.0f};   // -1 = attackrun never entered
    int torpedo_attackrun_entries{0};
    float torpedo_aim_heading_last{0.0f};
    float torpedo_aim_throttle_last{0.0f};
    int torpedo_attackrun_altitude_commands{0};
    // 009D236E, the aim-complete byte state+2Ch, and the clause values on the
    // tick it first went true. docs/TORPEDO_AIM_TICK.md.
    bool torpedo_aim_complete_2c{false};
    int torpedo_aim_complete_tick{-1};
    int torpedo_aim_complete_clause{0};   // 1 = range, 2 = turn, 3 = both
    float torpedo_aim_f34_threshold{0.0f};
    float torpedo_aim_f14_range{0.0f};
    float torpedo_aim_f18_turn{0.0f};
    float torpedo_aim_f0c_time{0.0f};
    float torpedo_aim_ramp{0.0f};
    float torpedo_aim_floor{0.0f};
    // 009D0D90, the goaway enter: the break-off distance 009D3150 compares the
    // range against, and the alternating side byte. docs/TORPEDO_GOAWAY_RELEASE.md.
    float torpedo_goaway_distance_24{0.0f};
    float torpedo_goaway_side_2c{1.0f};
    int torpedo_goaway_enters{0};
    bool torpedo_goaway_done_last{false};
    float torpedo_goaway_range_peak{-1.0f};
    // 009D0F10's own state object, the rest of task+6D8h. The five fields above
    // stay because the report and 009D3150 read them; this carries +18h, +1Ch,
    // +20h, +28h, +30h and +34h, which only the tick uses.
    bsp::TorpedoGoAwayRuntime torpedo_goaway_runtime{};
    // Packet cc9_fly_to_obstacles: 009FD570's cache on each goaway state.
    // [0] the torpedo goaway (009D0C54), [1] the dive-bomb goaway (009C4810).
    // The countdown starts at 0, as 009C74E8 leaves the dive state's +8h.
    int fly_to_countdown[2]{0, 0};
    std::vector<bsp::FlyToObstacle> fly_to_cache[2];
    int torpedo_goaway_ticks{0};
    int torpedo_goaway_arm_ticks[5]{0, 0, 0, 0, 0};  // index 1..4 = the four arms
    int torpedo_goaway_heading_ticks{0};             // arms that publish +2C0h
    float torpedo_goaway_alt_cmd_last{0.0f};
    float torpedo_goaway_alt_min{1e9f};
    float torpedo_goaway_alt_max{-1e9f};
    int torpedo_aim_run_time_updates{0};   // 009D19A4
    int torpedo_aim_release_arms{0};       // 009D2287
    float torpedo_aim_timer{0.0f};         // 009D2027, 009FA3A0(state+18h, dt)
    // The rest of the state+18h release timer. 009D2287 enables it with the
    // countdown at zero, 009FA3A0 counts it down and calls 007BBBA0 at
    // 009FA3D0. docs/TORPEDO_FIRST_RELEASE.md.
    bool torpedo_aim_timer_enabled_0c{false};
    int torpedo_aim_timer_fires{0};
    int torpedo_aim_timer_blocked_007bb110{0};
    int torpedo_aim_timer_first_fire_tick{-1};
    // unit+DF0h, 007BB150 from the pilot think 0099B055 (packet cc13-reltimer2,
    // written only with kReleaseBayOpenCc13Bound).
    std::uint8_t pilot_step_flag_df0{0};
    // timer+0Dh, set at 009FA405 (packet cc13-reltimer, written only with
    // kReleaseTimerNotify0DhBound). The image has no reader.
    bool torpedo_aim_timer_notify_0d{false};
    // plan+2C0h / +2CCh. docs/PILOT_TASK_HEADING_ARM.md: the arm writes the
    // bearing and mode 2, and a bot state's tick may overwrite the pair. The
    // torpedo aim tick does at 009D1D16 / 009D1D1E.
    float plan_heading_2c0{0.0f};
    int plan_heading_mode_2cc{0};
    bool plan_heading_2c0_written{false};
    float plane_live_throttle{1.0f};
    float plane_live_air_brake{0.0f};
    float plane_latched_controls[3]{0.0f, 0.0f, 0.0f};
    // The pilot bot's five plan slots (plan+274h, stride 0Ch) and the think
    // accumulator 0099ACD0 keeps at bot+70h. The tick gates on the accumulator
    // reaching 0.09 s and then passes the ACCUMULATED interval down, not the
    // frame delta - docs/PILOT_BOT_TICK_GATES.md.
    bsp::PilotPlanSlot plan_slots[5]{};
    float pilot_think_accumulator_70{0.0f};
    // unit+9FCh..+A10h, the pending command block, and the byte at unit+A14h
    // that 007B8C90 sets and 007BB920 clears.
    float pilot_command_block[5]{0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    bool pilot_command_pending_a14{false};
    // The unit this one's current command names as its target, plus one, or 0
    // for none. The gunnery host resolves the command row's target token against
    // the unit table and pushes it here, because the plane's control path needs
    // the same answer the weapon director already has rather than a second
    // resolution of its own.
    std::size_t command_target_plus_one{0};
    // Packet cc9_land_command_own_target: the `land` command's own target (its
    // vtable[178h] -> 00521EA0), plus one, stored where the land command is
    // installed. The refresh above never writes it.
    std::size_t land_command_target_plus_one{0};
    // The plane row's rate and acceleration keys, read once at creation.
    bsp::PlaneControlClass plane_class;
    // desc+184h StallSpd, the divisor the control authority ramp uses. The
    // PlaneFreeFlightClass default stands in when the row does not carry it.
    float plane_stall_spd{17.5f};
    // Packet p16_planner_laws: class+15Ch BombControlLimit (007C0FA6) and the
    // speed-error divisor plan+2B8h.
    float plane_bomb_control_limit_15c{0.0f};
    float plane_speed_corr_2b8{1.0f};
    // desc+174h XDrag and desc+170h YDrag, the two body-frame damping
    // coefficients 007DBD3A and 007DBD50 multiply the body lateral and body
    // vertical velocity by. PlaneFreeFlightClass declares them 0.0f and this
    // host never filled them, which switched off the ONLY term that turns a
    // plane's velocity onto its nose. docs/TORPEDO_RUN_IN_VELOCITY.md.
    float plane_x_drag{0.0f};
    float plane_y_drag{0.0f};
    // desc+188h MaxSpd, the numerator of 009F9D30's run-profile speed ratio,
    // and the scale on the torpedo aim tick's pitch denominator at 009D1E64.
    float plane_max_spd{0.0f};
    // Packet cc9_units_capture_accessors: unit+7A0h CaptureRange as 006F2780 stores
    // it (the scene record's dword, 500 when unauthored).
    std::int32_t capture_range_7a0{500};
    // Packet cc9_command_building_capture_bind: unit+7A4h CaptureValue (006F2780,
    // 1000 when unauthored).
    std::int32_t capture_value_7a4{1000};
    // Packet cc9_command_building_level: unit+770h Level (0 when unauthored) and
    // unit+76Ch LevelUpSeconds (10 when unauthored), as 006F2780 stores them.
    std::int32_t level_770{0};
    std::int32_t level_up_seconds_76c{10};
    // unit+7C4h LandingRange as 006F2780 stores it (006F285F; 500 when unauthored).
    std::int32_t landing_range_7c4{500};
    // Packet cc9_building_pad_model: unit+7CCh LandingPointRange (006F28B3; 500
    // when unauthored), the radius 006F5CC0 adopts landing pads within.
    std::int32_t landing_point_range_7cc{500};
    // Packet cc9_airfield_destruction_rule: unit+7C8h InferiorRange (006F288C; C8h
    // when unauthored), the radius 006F5CC0's first pass adopts within, and
    // whether the merged scene bag finds MinLevel (009554C1, which gates the
    // unit+724h clone that 006F5DCF requires).
    std::int32_t inferior_range_7c8{200};
    bool bag_min_level_724{false};
    std::int32_t min_level_724{0};   // the clone's MinLevel value (s57_garrison_min_level)
    // unit+71Ch as 006F5E9B writes it: 0 unresolved, -1 none, else building+1.
    int parent_link_71c{0};
    unsigned long long inferior_failure_steps{0};
    // Packet cc9_squadron_travel_alt: the squadron cruise block 0089F550 writes, kept on
    // the squadron's slot. The countdown +380h is held as the clock at which it goes
    // below zero (0.5 s after the call); -1.0 at construction means already expired.
    bool sq_alt_set{false};          // +394h holds a script altitude
    float sq_alt_394{0.0f};          // +394h
    bool sq_freeze_38d{false};       // +38Dh, countdown 1's freeze byte
    bool sq_lock_3a9{false};         // +3A9h
    double sq_timer_expiry_380{-1.0};
    // Packet l68_travel_speed: the fourth quadruple, 008A0630's stores (countdown 3
    // retained float countdown +388h, freeze +38Fh, +3A0h, lock +3ACh).
    // sq_speed_set marks a script override; sq_speed_refreshed marks an actual
    // 007ED5D0 class-speed store. Either publication feeds the movement readers.
    // Before either, the existing readers use the loaded class speed directly.
    bool sq_speed_set{false};
    bool sq_speed_refreshed{false};
    float sq_speed_3a0{0.0f};
    bool sq_freeze_38f{false};
    bool sq_lock_3ac{false};
    float sq_countdown_388{-1.0f};
    // A plane's head-task signature at its last think (the installed task kinds
    // and the command target), standing in for 0099AE92's compare of the bot's
    // head task before and after 0099A4C0.
    std::uint64_t head_task_sig_last{0};
    bool sq_ever_set{false};
    float sq_last_cruise{0.0f};
    // Packet cc9_squadron_attack_alt: the second block 008A22B0 writes (+37Ch
    // countdown 0 held as its expiry clock, +38Ch freeze, +398h, +3AAh lock).
    bool sq_attack_alt_set{false};
    float sq_alt_398{0.0f};
    // +390h: 007F2BD0 copies tuning+5F4h at construction (chain 564), then
    // 008A1F40 SquadronSetDropAllBombPercent can replace it. OFF retains the
    // old allocation-zero default until a script writes it.
    float sq_drop_all_bomb_percent_390{0.0f};
    bool sq_freeze_38c{false};
    bool sq_lock_3aa{false};
    // The one byte +3ADh both natives clear: true while it is 0 (a script
    // altitude live). 007F2C1A sets it to 1 at block init; the moveto and dive
    // profiles set it to 1 with their +394h / +398h writes.
    bool sq_lua_alt_3ad{false};
    double sq_timer_expiry_37c{-1.0};
    unsigned long long sq_attack_alt_reads{0};
    float sq_last_attack_alt{0.0f};
    // Packet cc9_dive_profile_draw: the value 009C8920's open gate last wrote
    // into +398h (BeginAltRange/1 + the 009C899B draw).
    bool sq_profile_398_valid{false};
    float sq_profile_398{0.0f};
    unsigned long long sq_profile_draws{0};
    unsigned long long sq_profile_writes{0};
    // Packet cc13_natives1: the third block 008A24E0 writes (+384h countdown 2
    // held as its expiry clock, +38Eh freeze, +39Ch release altitude, +3ABh lock).
    // sq_release_alt_set: +39Ch holds the script value, not 009C8A0E's 9999.
    bool sq_release_alt_set{false};
    float sq_alt_39c{0.0f};
    bool sq_freeze_38e{false};
    bool sq_lock_3ab{false};
    double sq_timer_expiry_384{-1.0};
    // desc+1ACh PitchSpd (DEG(30) on this installation's TBD). 007DA8EB uses it
    // as the pitch rate; 009D1E39 divides the nose-down angle by it to shallow
    // the aim tick's dive command as the dive steepens.
    float plane_pitch_spd{0.0f};
    // desc+18Ch TravelSpeed, the airspeed 007C6340 seeds a plane with.
    float plane_travel_speed{0.0f};
    // record+0h TorpReleaseAlt of the torpedo task's robots row, which
    // 009D4A70 reads through *(task+40Ch) = approach+14h. Packet
    // cc9_torpedo_reset_draws.
    float torpedo_release_alt_row{0.0f};
    float plane_bomb_delay_1f4{1.0f};   // class+1F4h BombDelay
    // desc+164h Accel, desc+208h GlideRate, desc+1D4h DragPitchRatio and
    // desc+1DCh AirBrakeDrag: the four authored fields the thrust 007D9050 and
    // the drag 007D9140 are built from. docs/PLANE_POSE_THROTTLE_ALTITUDE.md.
    float plane_accel{0.0f};
    float plane_glide_rate{0.0f};
    float plane_drag_pitch_ratio{0.0f};
    float plane_air_brake_drag{0.0f};
    // desc+1F0h DropAngle, 009FB800's dive gain and cap.
    float plane_drop_angle{0.0f};
    // plan+2B4h and plan+2D8h. 009C1850 writes the first at 009C189A and raises
    // the second at 009C18A7, and 0099D300's throttle arms read both. They live
    // on the slot rather than in PilotPlanState because that header belongs to
    // another packet. docs/PILOT_THROTTLE_CUT_RAISER.md.
    float plane_desired_speed_2b4{0.0f};
    int plane_air_brake_mode_2d8{0};
    // plan+264h, the start of 0099CAB0's pass-0 bank ramp. 0099D300 zeroes it
    // at 0099EB9D on every think; only the torpedo aim tick (009D1F36) writes
    // anything else. cc13-audit10, behind kBandShaperPlan264Bound.
    float plan_band_bank_264{0.0f};
    // plan+2B0h, a byte. Its only reader in the plane-AI range is 0099D924
    // `CMP byte [ESI+2B0h],0` in 0099D300's speed-demand arm: ZERO lets
    // 0099D92D-0099D970 raise the speed-error divisor plan+2B8h to
    // InterpolateClamped(-TrgSpeedCorrMinPitch, TrgSpeedCorrSpeedMul, 0, 1,
    // pitch) (singleton+5E8h/5ECh), non-zero skips that raise. The host's
    // throttle rule substitutes 1.0 for plan+2B8h, so nothing here reads it
    // yet; it is carried so the image's store sequence is complete.
    // docs/PLANE_FOLLOW_SPEED.md.
    std::uint8_t plane_trg_speed_corr_off_2b0{0};
    float plane_throttle_last{1.0f};
    float plane_think_dt{0.0f};   // 0099D300's argument, packet cc9_throttle_slot
    int plane_speed_commands{0};
    // desc+1E4h and desc+1ECh, the two 007C4850 derives with the 007D98F0
    // climb-angle solver at 007C4BE9 and 007C4C14. desc+1ECh is the climb arm's
    // gain, and it is 0.6 * desc+1E4h, NOT zero: docs/PLANE_FLIGHT.md read it as
    // having no producer because it is derived rather than authored.
    float plane_climb_angle_1e4{0.0f};
    float plane_climb_angle_1ec{0.0f};
    // desc+268h TurnCircleRadius, the scale both of the dive-bomb approach's
    // attack-distance draws multiply (009C3F86 and 009C3FB5, each dominated by
    // its own `MOV EBP,[ESI+8]`). Packet cc8_dive_race.
    float plane_turn_circle_radius{0.0f};
    // desc+194h SwimHeight, one of the two terms of the free-flight arm's water
    // line at 007CC4E8.
    float plane_swim_height{0.0f};
    int plane_water_contacts{0};
    // Packet cc9_plane_water_law (docs/WATER_SURFACE_LAW.md section 11): the
    // state-6 arm's own fields. unit+90Ch the submersion accumulator, ctl+94h the
    // byte 007D8180 sets for a dying plane at 007CBC02, and the arm's counters.
    float plane_water_accum_90c{0.0f};
    bool plane_ctl_94{false};
    // Packet p15_dying_spin: ctl+F0h..F8h, 007D7EA0's draw (007D7F90, 00439950),
    // taken from the plane's keyed stream at first use.
    float dying_spin_f0[3]{0.0f, 0.0f, 0.0f};
    bool dying_spin_seeded{false};
    int plane_water_law_steps{0};
    int plane_water_lift_offs{0};
    int plane_water_powerlost{0};
    // Packet cc9_plane_death_modes (docs/PLANE_DEATH_MODES.md): the death mode
    // 007CA8A0 chose, unit+C10h the explosion timer 007BBFA0 stored, the flags
    // 007D0B80 set (unit+C39h power lost, +C36h spinning, +C3Ah message seen),
    // and whether the aircraft has been killed and left the world.
    bsp::PlaneDeathMode plane_death_mode{bsp::PlaneDeathMode::None};
    // A live plane's mode raised through vtable[194h] 007BBFA0 rather than
    // chosen by SetHealth's 007CA8A0: "powerlost" on the water (007CB8DC /
    // 007CBE02, packet cc9_lua54_follow_altitude) and the +C01h latch's
    // "powerlost" or "explosion" (007CE790). The death-mode step takes it
    // without 007CA8A0's draw. None: no effect was raised.
    bsp::PlaneDeathMode plane_raised_death_mode{bsp::PlaneDeathMode::None};
    // Goal 5 chain 645: what that raise's own 007BBFA0 routed (0077C2A0 flags
    // 7), kept for the death-mode step, which delivers each copy to 007D0B80.
    // 007BBFA0 stores unit+C10h itself at the raise.
    std::vector<bsp::gameplay_seg47_cc13::FailureMessage> plane_raised_routed;
    float plane_death_timer_c10{-1.0f};
    bool plane_death_c39{false};
    bool plane_death_c36{false};
    // unit+C37h, 007D0DD7: set for rightspinning (the "rwing"/"aileron_r" arm),
    // clear for leftspinning (007D104F sets C38h instead). Read by R4.
    bool plane_spin_right_c37{false};
    // ctl+30h..38h (unit+AE0h), the low-passed relative velocity 007DC743 writes
    // at the core law's tail; unit+C84h, its flight-path angle (007C1D05).
    float plane_velocity_lp_30[3]{};
    bool plane_velocity_lp_valid{false};
    float plane_path_pitch_c84{0.0f};
    bool plane_death_c3a{false};
    bool plane_death_removed{false};
    // Goal 5 chain 654. unit+C14h, the dead-engine fire delay: -1.0f from the
    // constructor (docs/CC13_MISTAG.md), then 007BCAA0's 007BCABB draw
    // (DeadEngineFireDelay) when the plane is destroyed; 007CAF10's engine block
    // reads it and stores -1.0f again once the engine fire is raised.
    float plane_engine_fire_timer_c14{-1.0f};
    // The engine list at unit+C98h (+8h data, +0Ch count), as 007EC5A0 fills it
    // from the class at spawn (build_plane_engine_list). Empty for a
    // ship or a class without NumEngines. plane_num_engines_140 is the class's
    // NumEngines as 007D204D stores it at +140h (007BBFA0 reads the same key's
    // +7Ch copy).
    std::int32_t plane_num_engines_140{0};
    std::vector<std::unique_ptr<bsp::gameplay_seg48_cc13::PlaneEngineSlots>> plane_engines_c98;
    std::vector<bsp::gameplay_seg48_cc13::PlaneEngineSlots*> plane_engine_list_c98() const {
        std::vector<bsp::gameplay_seg48_cc13::PlaneEngineSlots*> list;
        list.reserve(plane_engines_c98.size());
        for (const auto& engine : plane_engines_c98) list.push_back(engine.get());
        return list;
    }
    // Packet cc13-limbo3: the squadron's live count (+3CCh, this plane included)
    // stamped by the stock return just before it clears the member seat; -1
    // otherwise. Read only by plane_squadron_live_count_3cc.
    int squadron_live_3cc_at_stock_return{-1};
    bool squadron_left_on_death{false};   // packet cc9_val_squadron_registry
    int db_done_law_ticks{0};             // packet cc9_plane_follow_law
    // Packet cc9_gunfire_avoidance (docs/ATTACKER_EVASION.md section 5): the
    // pilot's gunfire timers pilot+3E8h (avoid) and +3ECh (rest end), the
    // flag plan+26Ch bit 4, and the three band sets pilot+4h (yaw, cmd[0]),
    // +CCh (pitch, cmd[1]) and +194h (roll, cmd[2]), each one [lo, hi] because
    // the gunfire arm is
    // the only one bound and inserts once per think (0099B790 on an empty set).
    float ga_timer_3e8{0.0f};
    float ga_rest_3ec{0.0f};
    // Packet p20_ctor_seeds: task+308h, the 0099B740 phase 0099C76C seeds with
    // -U(0, 1). Seeded only; 009998A0's +304h/+308h accumulator is not hosted.
    float task_phase_308{0.0f};
    bool ga_flag_26c_4{false};
    // Packet cc9_pilot_vehicle_terrain_avoidance: each set as 0099B790 keeps
    // it, up to 24 merged [lo, hi] pairs from set+4h with the count at set+C4h
    // (pilot+C8h/+190h/+258h), because three arms now insert into the sets.
    int ga_band_n[3]{0, 0, 0};
    float ga_band[3][24][2]{};
    // pilot+25Ch, reset to 1.0 by 009A17EF, lowered by the vehicle and
    // terrain arms, read by 0099BF30's throttle arm.
    float av_throttle_25c{1.0f};
    // unit+BC4h, the yaw-target gain: 1.0 from 007D6167, max-latched by
    // 0099BB00 from the terrain arm, relaxed toward 1.0 by the rate law.
    float plane_yaw_gain_bc4{1.0f};
    int yaw_gain_raises{0};
    // The unit+C50h object's +30h list (007E11D0: kinds 6 and 0Fh within R)
    // with its own copy of the +7Ch clock; the finder keeps +50h separately.
    std::vector<std::size_t> nb_near_30;
    float nb_clock_30{4.5f};
    bool nb_clocks_seeded{false};   // kNeighbourClockDrawsBound: 007E1E20's draws taken
    // Counters (diagnostic census only).
    int va_plane_ticks{0};
    int va_ship_ticks{0};
    int va_bands{0};
    int va_throttle_sets{0};
    int va_dive_ticks{0};
    int tr_ticks{0};
    int tr_bands{0};
    int tr_throttle_sets{0};
    int tr_dive_ticks{0};
    int tr_water_ticks{0};
    float tr_min_margin{1.0e9f};
    float tr_toggle_3f0{0.0f};
    // Packet cc9_terrain_segment_consumers: pilot+3F0h, branch A's side byte, and
    // its census (00903BC0 calls, blocked first probes, fan exits by side).
    bool tr_side_3f0{false};
    // Packet cc9_bsm01_state_natives: controller+14h, set by DisablePhysics.
    bool controller_disabled_14{false};
    int tr_water_probes{0};
    int tr_water_blocked{0};
    int tr_water_fan_pos{0};
    int tr_water_fan_neg{0};
    int tr_water_fan_exhausted{0};
    int ga_detect_ticks{0};
    int ga_flagged_ticks{0};
    int ga_repairs{0};
    int ga_exempt_ticks{0};
    // Packet cc9_wing_achieved_speed, diagnostic: the last follow step's arm
    // (1 hold, 2 fly-to), station distance and the motion step it ran on.
    int fw_arm{0};
    float fw_station_dist{-1.0f};
    // Every follow-law tick (both arms); gates the `follow law` rows, which
    // keyed off the dive-bomb follow counter and never printed elsewhere.
    int fw_law_ticks{0};
    // Packet cc9_follow_turbo: plan+2E5h (the pilot's turbo request) and the
    // unit's turbo byte unit+9F9h, which 007DC84F copies to ctl+4h.
    bool plan_turbo_2e5{false};
    bool plane_turbo_9f9{false};
    unsigned long long fw_step{0};
    float fw_station[3]{0.0f, 0.0f, 0.0f};
    float fw_leader_heading{0.0f};
    float plane_death_seconds{-1.0f};
    // The altitude 009FBA50 was last commanded with, and the pitch 009FB800
    // answered, kept for the census only.
    float plane_commanded_altitude{-1.0f};
    float plane_commanded_pitch{0.0f};
    float plane_dive_probe_timer{0.0f};
    // unit+0BBCh and unit+0BB0h+10h, the latched throttle and air brake. The
    // latch 007B9783 copies the whole live block, not just the three stick
    // axes, and both of these are drag or thrust inputs.
    float plane_latched_throttle{1.0f};
    float plane_latched_air_brake{0.0f};
    // The union of this unit's guns' projectile descriptor answer sets, stored
    // by the gunnery host at load. docs/ORDNANCE_KIND_IDENTITY.md.
    std::uint64_t ordnance_mask{0};
    volatile float generic_input_63c{1.0f};   //0095CD9E

    // The pose the canonical projection borrows: +74h local, +C8h valid, +CCh
    // world, +10Ch derived-valid, with no parent because every entity of this
    // mission is top level (docs/GAME_EXECUTABLE.md milestone 2h, correction 8).
    bsp::CameraMatrix local{};
    bsp::CameraMatrix world{};
    // Produced by the position/matrix setters below, not the plane controller's
    // separate +608h history (007CED84, ESI=unit+310h). No initial value is
    // assumed while the host has not replayed either native store.
    std::optional<std::array<float, 3>> previous_position_6a4;
    // The inverse at unit+110h belongs to this same pose. Mutable because a
    // const frame query can lazily fill the cache through PoseRefreshView.
    mutable bsp::CameraMatrix derived{};
    std::uint8_t world_valid{1};
    std::uint8_t derived_valid{1};
    bsp::PoseRefreshView* parent{nullptr};
    std::unique_ptr<bsp::PoseRefreshView> pose;
    std::unique_ptr<bsp::UnitInstanceState> state;
    // Retained cells absent from UnitInstanceState. Its active/simulate cells
    // remain the only owners of+5C/+5D. Constructor00925CE0 stores BL=0 at
    //00925E11/+5E,00925E0B/+5F,00925E08/+60; later stores use this same slot.
    std::uint8_t scene_destroyed_005e{0};
    std::uint8_t scene_removed_005f{0};
    std::uint8_t scene_pending_destroy_0060{0};
    bool dead_plane_bot_gate_logged{false};  // packet cc9_dead_plane_bot_think
    // Packet cc9_sunk_ship_kill_depth: sinkTime unit+828h (advanced through
    // [EDI+518h], EDI = unit+310h, at 008263CE..008263DC) and the census of a
    // wreck's hull ends against KillDepth (008265EC..00826622).
    float sink_time_828{0.0f};
    float sink_under_82c{0.0f};   // packet s52_sink_draws: both hull ends below 0 (00826656)
    // Packet cc9_sunk_hull_shape_flag8: 00826410..0082643B cleared mask bit 8 (the terrain
    // group) on every hull shape once sinkTime passed 60 s (00C47F60).
    bool sunk_terrain_mask_cleared{false};
    // Packet cc9_avoid_zone_draft_bodies: the class bit the hull build ORs into every hull
    // shape's mask (009394E5..009395E2), taken at the first world contact phase, and
    // whether 0092BD70(0) has cleared it.
    std::uint32_t hull_class_bit{0};
    bool hull_class_bit_taken{false};
    bool hull_class_bit_cleared{false};
    // Packet cc9_ship_terrain_contact: +1010h / +1011h, the one-frame contact
    // latch 008255B0 rotates (00825824..0082583E); 009377E0 sets +1010h on a
    // kind-8 contact (the HUD's `ingame.warning_shallowwater`, 006830A5).
    bool ground_contact_1010{false};
    bool ground_contact_1011{false};
    bool terrain_contact_seen{false};
    unsigned long long terrain_contact_steps{0};
    float terrain_max_penetration{0.0f};
    bool wreck_seen{false};
    float wreck_first_t{-1.0f};
    float wreck_min_end_y{0.0f};
    float wreck_last_y{0.0f};
    float kill_depth_at{-1.0f};
    std::size_t world_nodes_unlinked{0};
    // Packet cc9_ship_sink_descent: the leak manager at unit+10D4h (constructor
    // 0074E7B0, init 0074F490 from 00823780). `leaks` holds its three parallel
    // arrays (+18h rates, +1Ch water, +20h points), count +14h = 6.
    bool leak_ready{false};
    std::array<bsp::UnitLeakEntry, 6> leaks{};
    float leak_rate_cap_08{0.0f};     // +08h
    float leak_capacity_0c{0.0f};     // +0Ch = controller+84h / 10
    float leak_rate_34{0.0f};         // +34h
    // Packet cc9_live_hull_leak: +38h (DamageToDeath, 200.0 from 0074E7B0 unless
    // the class authors both keys), +2Ch (the total rate 0074F930 last summed)
    // and +04h (Length / 3.0, 0074F4F6), the three fields 0074F090 reads.
    float leak_damage_to_death_38{200.0f};
    float leak_total_rate_2c{0.0f};
    float flood_trace_timer{0.0f};    // DIAGNOSTIC, packet cc9_hull_flooding_trace
    float leak_station_04{0.0f};
    // Packet cc9_live_hull_repair: set by the gunnery host every step.
    float leak_health_fraction{1.0f};  // 00923BE0
    bool class_repair_00d0{false};     // class+D0h
    float controller_reserve_84{0.0f}; // controller+84h, 00937F74
    bool leak_redistributed{false};   // 0074EC50 ran (the wreck handler)
    float leak_first_water_t{-1.0f};
    bool scene_flags_available{false}; // process provenance, not a native byte
    bsp::UnitClassBlock class_block{};

    // The motion half.
    bsp::ShipMotionState motion{};
    bsp::ShipMotionClass motion_class{};
    // The wake trail at unit+0BD0h, appended by 00825F20's tail (00826CEE) and
    // read by 0070D290 for every formation follower's station. Packet
    // cc8_ship_follow, docs/SHIP_UNIT_GROUP_FOLLOW.md sections 5b and 5c.
    bsp::ShipAiWakeTrail wake{};
    // unit+284h, the unit group pointer 0070EF38 writes before any test, as an
    // index into Impl::formation_groups. -1 is a null pointer.
    std::int32_t formation_group{-1};
    // Packet cc13-jm08 (kDeadLeaderDirectorStateCopyBound), the 0071C150 handover
    // of this unit's kill. `ran`: already done (BeforePump, or the posted entry
    // delivered) for a non-ship, so 0077BD70 must not repeat it; cleared by
    // leave_group. A ship runs the whole leave instead and never sets it.
    // `posted`: queued (HitDrainAfterPump) and not yet run; whoever comes first
    // runs it and clears it - normally the posted entry (a Lua leave meeting it
    // is posted behind it, post_formation_leave_0077c980); as a backstop, a
    // synchronous leave_group of the unit or of a member of its group. `posted`
    // is set only by leader_death_handover_at_kill (cleared there on a failed
    // post); `ran` is set there at a BeforePump kill and also when the posted
    // entry is delivered. Slots are appended and never reused.
    bool director_handover_ran{false};
    bool director_handover_posted{false};
    bsp::UnitHullExtents hull_extents{};
    float class_width_00a4{};
    bsp::ShipClassFields fields{};
    // Packet cc9_unit_instance_step11: the cells step 11 of 008255B0 owns.
    bsp::UnitStep11State step11{};
    bsp::UnitOrderRing ring{};
    bsp::UnitOrderQueue queue{};
    bsp::UnitOrderRecordStorage scratch{};
    bsp::UnitControllerState controller{};

    // The hull's rigid body. `motion_state` is M = *(B+4h) and `body` is B, which
    // the unit's force-model controller holds at controller+2Ch. Milestone 2r
    // builds both through the tail of 00937C90 (docs/SHIP_HULL_BODY.md), so the
    // mass, the box inertia, the row-1 torque lock, both damping rates and both
    // speed clamps are the game's own; `hull_material` is the record 00937CF1
    // selects for this hull.
    bsp::DynMotionState motion_state{};
    bsp::DynBody body{};
    bsp::ShipPhysicsMaterial hull_material{bsp::ShipPhysicsMaterial::kShip};

    // Milestone 2s: what 009329C0 reads. The physics-material record is the row
    // settings+4E0h + material*38h that the hull body already selected, and the
    // element list is class+52Ch..+530h.
    //
    // The element list is a STAND-IN, not the game's own: its producer writes
    // class+528h..+534h and no function in the exported set does that, so
    // docs/SHIP_HYDRO_FORCES.md leaves the field roles as a hypothesis and its
    // follow-up `ship_buoyancy_element_producer` owns the question. The list
    // built here is the same one the probe builds under --hydro, from the class
    // row's own Length, Height and Mass, so the two sides are comparable.
    std::vector<bsp::ShipBuoyancyElement> buoyancy_elements;
    // unit+10FCh, the leak model's total accumulated water. 0074F930 writes it
    // and 009329C0 adds it to the hull mass at 00932B78; an undamaged hull
    // holds it at zero.
    float leak_water_mass_10fc{0.0f};
    // Packet cc9_scene_home_base_key. The scene row's `HomeBase` string as the
    // scene-contents owner hands it over (set_squadron_scene_home_base), and the
    // home base 007F1C00 stored at squadron+404h from it at pass C, by name.
    std::string scene_home_base_key;
    std::string scene_home_base_404;

    // unit+C4h is the most-derived instance class selected by VehicleClass.Type.
    // -1 means its identity is unresolved; it is not a native class-id stamp.
    int class_id{bsp::kVehicleClassKindUnknown};
    // Packet cc9_unit_class_lands_troops: class+809h `Rocketer`, which only the
    // landing-ship reader 0074C630 writes (MLandingShip); false elsewhere.
    bool landing_ship_is_rocketer_0809{false};
    // Milestone 2p: the two load latches the middle of 009F3F80 raises with the
    // inlined bodies of 009D4FB0 (unit+102Ch) and 009D4FE0 (unit+1034h). Their
    // consumers are not in this process; the fields exist so the raise is a
    // real store rather than a discarded call, and the report prints them.
    float turn_assist_load_102c{0.0f};
    float secondary_load_1034{0.0f};
    bool standing_order{false};
    float standing_throttle{0.0f};
    float standing_rudder{0.0f};
};

// ---------------------------------------------------------------------------
// Chain468: borrow the same pose cells used by placement/rendering. Native
// 009BE0F0 tests C8 and calls 00414DB0 before reading world+30h (entity+FCh).
// No raw unit image or copied validity flag is manufactured for this producer.
class GameMoveToEntityPoseHost final : public bsp::MoveToEntityPoseHost {
public:
    bsp::MoveToEntityPoseView move_to_entity_pose(void* entity) override {
        auto& unit = *static_cast<GameUnitSlot*>(entity);
        return {unit.world_valid, unit.world.data() + 12};
    }
    void entity_pose_refresh_world_00414db0(void* entity) override {
        bsp::refresh_pose_00414db0(*static_cast<GameUnitSlot*>(entity)->pose);
    }
};

// Chain 412: the 009C62E8 fly-over read consumes the selected target's live
// velocity through 009FA2E0. This adapter borrows the real unit observer
// endpoint and motion cells; it does not construct a second unit or register
// another observer. The temporary ref below projects only +14h/+18h, the only
// cells the reader uses. Original class/stack ABI is not supplied.
class GameApproachTargetVelocityHost final : public bsp::cc13_seg68::ApproachTargetVelocityHost {
public:
    explicit GameApproachTargetVelocityHost(const GameUnitSlot& target) : target_(target) {}

    std::array<float, 3> entity_velocity_34(bsp::NativeObserverOwnerStorage* entity) override {
        // 009FA2EE..009FA313 dispatches slot +34h on the selected endpoint.
        // Only the borrowed target's real observed base is admitted here.
        if (entity != &target_.observer_prefix.observed_00) std::abort();
        if (target_.plane_velocity_seeded) {
            return {target_.plane_world_velocity[0], target_.plane_world_velocity[1],
                target_.plane_world_velocity[2]};
        }
        return {target_.motion.linear_velocity.x, target_.motion.linear_velocity.y,
            target_.motion.linear_velocity.z};
    }

    const std::array<float, 3>& static_zero_vector_00f87574() override {
        // 009FA31B..009FA340 copies the three words at 00F87574/78/7C.
        // They are in the image's zero-filled .data tail (009FA2E0 export).
        static const std::array<float, 3> zero{0.0f, 0.0f, 0.0f};
        return zero;
    }

private:
    const GameUnitSlot& target_;
};

// Packet cc8_hull_aim_point: the aim point both attack tasks steer to.
// ---------------------------------------------------------------------------
// Until this packet both tasks aimed at the target's ORIGIN. The image does
// not: 009FADA0 stores target_world_matrix(target+CCh) x body_frame_offset to
// the sub-object's +1Ch, and for a ship that offset is a point drawn inside the
// authored hull box by 00816650 (docs/HULL_AIM_POINT.md).
//
// A monotonic counter gives each newly ordered (attacker, target) pair its own
// draw, the way each approach sub-object gets its own in the image. It is
// deterministic because the order in which targets are assigned is.
inline std::uint32_t g_hull_aim_pick_counter = 0;
// Packet cc9_ship_section_points: picks 00816650 answered with a section
// (engine room, magazine, fuel tank), picks that had a positive chance and fell
// to the box, and picks whose target had a section taken out by 0093A570.
inline unsigned long long g_section_picks[3] = {0, 0, 0};
inline unsigned long long g_section_chance_box_picks = 0;
inline unsigned long long g_section_unavailable_picks = 0;

// 00816650 is reached only through the nine unit vtables that carry it; the
// plane, land-vehicle and land-fort classes carry 0042D810, the origin.
//
// CORRECTED after the first USN04 pair measured nothing. This tested
// `class_id == kVehicleClassIsShipKind` (6), but that constant is one of
// 00964790's KIND TESTS (include/bsp/vehicle_class.hpp:176), not the leaf
// class_id this host stores on the slot. The run's own rows settle it:
// York-class02 kind=10, Lexington-class01 kind=9, Fletcher-class01 kind=7 -
// never 6 - so the predicate was false for every ship and the feed was inert.
//
// The ship family is the one src/game_hosts_ship_ai.cpp:73 already enumerates
// as has_ship_navigation_class. That function is in another translation unit's
// anonymous namespace, so the switch is restated rather than called; keep the
// two in step. It is the right family on the evidence side too: planes, land
// vehicles and land forts are exactly the families carrying 0042D810.
inline bool hull_aim_target_samples_hull(const GameUnitSlot& target) {
    switch (static_cast<bsp::VehicleClassKind>(target.class_id)) {
    case bsp::VehicleClassKind::Destroyer:
    case bsp::VehicleClassKind::Submarine:
    case bsp::VehicleClassKind::MotherShip:
    case bsp::VehicleClassKind::Cruiser:
    case bsp::VehicleClassKind::Cargo:
    case bsp::VehicleClassKind::LandingShip:
    case bsp::VehicleClassKind::BattleShip:
    case bsp::VehicleClassKind::TorpedoBoat:
        return true;
    default:
        return false;
    }
}

// THE NAMED SWITCH, and it is OFF. With the offset live, USN04 dive-bomb drops
// fell from 23 to 8 (local\hullaim_after2_usn04.log against
// local\hullaim_before_usn04.log, same base ec14870c3, identical parameters).
// The reading in docs/HULL_AIM_POINT.md predicts the bombs SCATTER along the
// hull, not that two thirds of them stop being released, so that is a missed
// prediction and it is not understood yet. Until it is, this host keeps the
// target's origin, which is the configuration measured to be line-identical to
// the before on all 43 census lines.
//
// Turning this to `true` re-arms the binding; docs/HANDOFF_HULL_AIM_POINT.md
// section (f) lists the three candidates to check first. Nothing below this
// line is disabled - the pick still runs and the state still advances, so a
// successor can print the drawn offset per aircraft without re-arming the feed.
//
// Packet cc9_hull_axis (docs/HULL_AIM_AXIS.md) measured the first two and both
// are clean: one draw per aircraft, and the offset lies on row 2, which is the
// ship's course. Three more sites read the hull point in the image (009C6342,
// 009C59CD, 009C5278) that this host feeds the origin, but feeding them does
// not restore the releases. The loss scales with the horizontal offset (1% and
// 10% of it: 23 releases, 50%: 16, 100%: 8) and not with its height, so it
// stays OFF. With it on, `hull_aim draw` and `hull_aim inrange` lines print.
//
// Packet cc9_dive_aim_hull_point: ON. Those losses were measured before the
// rest of the dive was bound. Re-paired on the faithful dive set (USN04
// 4700/4500, USN01, USN13): one draw per attacker, inside 0.45 x Length/Width,
// and the releases hold (USN04 dive 1 -> 1, torpedo 4 -> 5; USN01 2 -> 2 with
// identical damage). docs/HULL_AIM_POINT.md section 9.
constexpr bool kHullAimOffsetEnabled = true;
// Packet cc9_hull_turndown: the per-tick turndown/aimdive/aimglide trace in
// update_dive_bomb_approach. Diagnostic only; off in the default build.
constexpr bool kHullAimTrace = false;

// Packet cc9_approach_target_lead (docs/TORPEDO_AIM_LEAD.md section 11):
// 009FADA0's tail 009FAF05-009FAF7D adds target->vtable[48h](projtime) minus
// the origin to the hull point. projtime (sub+44h) is written by the torpedo
// approach as approach+F8h (its engagement estimate) and by the dive-bomb
// approach as approach+74h (009C7E3C). One switch per writer; both paired ON.
constexpr bool kTorpedoAimLeadBound = true;   // ON: TORPEDO_AIM_LEAD 11.3
constexpr bool kDiveAimLeadBound = true;      // ON: TORPEDO_AIM_LEAD 11.4
// Packet cc9_aim_error_draw (docs/TORPEDO_AIM_LEAD.md section 12): 009C3DA0 and
// 009D02A0 draw the aim error from the robots row - the body-frame bias, the
// hull-point spread, the dive's section chance and weights, and the time error
// on projtime (dive approach+C8h, torpedo approach+9Ch). Paired ON.
constexpr bool kAimErrorDrawBound = true;     // ON: TORPEDO_AIM_LEAD 12.2
// cc13-audit10 (docs/CC13_AUDIT_TORPEDO_BOMBER.md finding 4): the aim tick's
// plan stores this host drops - plan+2E8h = 1.4f (009D1D02), throttle 1.0 and
// air brake 0.0 with their bytes (009D1F42-009D1F64), plan+2D8h = 0 (009D1F6B)
// and the plan+264h fade (009D1F36). Behaviour-changing; pair before ON.
constexpr bool kTorpedoAimPlanWritesBound = true;  // GUNNERY 193: ON by pair verdict
// cc13-audit10 finding 5: desc+25Ch TurnRoll (009D1C51), desc+A4h Width
// (009D1C84) and desc+268h TurnCircleRadius (009D1962) into the aim tick, in
// place of the 1.0 / 1.0 / 0 stand-ins. Behaviour-changing; pair before ON.
constexpr bool kTorpedoAimClassInputsBound = true;  // GUNNERY 193: ON by pair verdict
// cc13-audit10 finding 3: 0099CAB0's pass-0 ramp starts at pilot+264h, which
// 0099D300 zeroes at 0099EB9D on every think; the 0.12 stored at 0099EB95 is
// pilot+260h. Only the torpedo aim tick (009D1F36) writes +264h otherwise.
constexpr bool kBandShaperPlan264Bound = true;  // GUNNERY 193: ON by pair verdict
// Packet cc9_ship_section_points (docs/TORPEDO_AIM_LEAD.md section 14): the pick
// 009FA260 hands 00816650 the TARGET ship's section records, the engine room /
// magazine / fuel tank points 0081F980 builds from the model's GeomMesh
// elements of kinds 5 / 8 / 6, and 0093A570's answer on the repair task's
// failure vector (ids 5 / 8 / 6). Only the dive rows 009C3DA0 stores (skill 2-5,
// chance 0.5-1.0) reach them. SUBSTITUTION, labelled: 0093A570 matches the
// record id; this host asks GameGunneryHost::unit_failure_active by the
// ShipGlobals.Failures names of those kinds (EngineJam / Explosion / Fire),
// which is the same answer while 0093BED0 (id = kind) is the host's only
// producer of those names. OFF: empty records, the hull box every time.
// Packet cc9_torpedo_goaway_aim (docs/TORPEDO_AIM_LEAD.md section 15): the
// goaway geometry 009D0C10 takes its break-off point from approach->vtable[0],
// 009D0670, which copies approach+D0h = sub+1Ch: the hull point plus the lead
// that 009FADA0 wrote at 009D34BB earlier in the same arm tick (009D486F runs
// 009D3420 before the state tick at 009D48F6). ON: the goaway asks
// aim_point_009fada0 for it, as approach_target_point does. OFF: the ordered
// target's origin.
constexpr bool kTorpedoGoAwayAimPointBound = true;
// Packet cc9_fly_to_obstacles (docs/TORPEDO_AIM_LEAD.md section 16): 009FD570's
// obstacle cache (cache = the calling state; +8h countdown, +0Ch list). Every
// 21st call (009FD5FF ADD [EDI+8],-1 / JNS / MOV [EDI+8],14h) it walks
// [[00E188A8]+19CCh]+64h, world list 6, the ships, and keeps each unit whose
// team +54h differs from the flyer's (009FD656) and whose XZ distance to the
// lead point is under max(unit+434h, +444h, +448h) + 100 (009FD66F-009FD6F9):
// the longest AAMACHINEGUN / FLAK / LIGHTARTILLERYFLAK range (00956C20's
// unit+430h + cat*4). The weight is unit+464h + +474h + +478h, the same three
// categories' 00956C20 damage sums (unit+460h + cat*4). So the goaway steers
// round enemy AA envelopes. SUBSTITUTIONS, labelled: +460h is rebuilt here
// from the gunnery host's gun rows (mean of the bullet class's DamageMin and
// DamageMax per gun with a bullet class), without 00956C20's
// kind-6-in-category-6 second ammo record (+48h); the cache lives on the unit
// slot, one per goaway state, so it is not reset when a new task is built.
// OFF: the empty list, as before.
constexpr bool kFlyToObstacleListBound = true;    // ON: TORPEDO_AIM_LEAD 16.1
// Packet cc9_torpedo_reset_draws (docs/TORPEDO_AIM_LEAD.md section 19): the
// reset 009D0380's stream-1 draws, which this host held as row values or 0.
// +70h = U(0.9, 1.1) * min(desc+18Ch TravelSpeed, 0.75 * 007BCE20) (009D0449),
// the planned attack speed the engagement estimate adds to the closing speed;
// +74h = 67.0 (009D0457); +78h = U(0, 0.25) * TorpReleaseAlt (009D0475), where
// the host stored the whole row value; +7Ch/+80h each moved by U(-0.1, 0.5) of
// their gap (009D05ED, 009D0625). SUBSTITUTION, labelled: every draw has its
// own keyed stand-in stream (name#t70, #t78, #t7c, #t80), so the draws the
// host already makes keep their sequence. OFF: the row values, as before.
constexpr bool kTorpedoResetDrawsBound = true;          // ON: TORPEDO_AIM_LEAD 19.6
// Packet cc9_torpedo_reset_engage_draws (docs/TORPEDO_AIM_LEAD.md section 23): the
// reset's two remaining draws in 009D0380. +88h = U(1.25, 1.5) * desc+268h
// TurnCircleRadius + +80h (009D04A6-009D050B), and +8Ch = +90h = +88h * 1.3
// (009D0517-009D053F), the engage range 009D4AC4 then clamps up to AttackDist;
// +12Ch = -U(0, 1) (009D0581-009D0590), the first replan delay. SUBSTITUTION,
// labelled: keyed stand-in streams name#t88 and name#t12c. OFF: +88h is the
// engage range, +90h is 0 until the first tick and +12Ch is 0.
constexpr bool kTorpedoResetEngageDrawsBound = true;   // ON: TORPEDO_AIM_LEAD 23.4
// Packet cc9_torpedo_reset_draws: 009D0160, called at 009D0632 after the aim
// error draw, seeds +98h, which the engagement estimate adds into +F8h, the
// torpedo projtime. The host binds no other +98h writer (009D1360 at 009D19A4
// and 009D27D1 is a counter here), so the seed stands for the whole task.
// It reads +70h, +78h and +7Ch, so it is meant to run with the draws ON.
// OFF: +98h stays 0.
constexpr bool kTorpedoResetRunTimeSeedBound = true;    // ON: TORPEDO_AIM_LEAD 19.6
// Packet cc9_torpedo_run_time_update (docs/TORPEDO_AIM_LEAD.md section 21):
// 009D1360 at its two callers, the aim tick (009D19A4) and the done/prepare
// tick's running countdown (009D27D1), writes +A0h (the fall lead, read by the
// aim release gate 009D2052) and +98h (the run time the engagement estimate
// adds into +F8h). The reset stores +A0h = 10.0 (009D04E9). OFF: +A0h stays
// 0 and +98h keeps the reset seed.
constexpr bool kTorpedoRunTimeUpdateBound = true;   // ON: TORPEDO_AIM_LEAD 21.4
constexpr bool kApproachSectionPointsBound = true;   // ON: TORPEDO_AIM_LEAD 14.4
// Packet cc9_aimdive_response: the aimdive tick's yaw, throttle and air-brake
// tail 009C5DB8-009C6080 (include/bsp/dive_bomb_aimdive_tail.hpp), read whole.
// OFF: bound, USN04 releases fell 23 -> 4 with the hull switch off
// (local/boff_usn04.log). The host's aim error saturates the pitch command, so
// the tail holds MinPowerCtrl 0.2 and MaxBrakeCtrl 0.5 through the dive and
// the aircraft arrives slow and shallow. docs/AIMDIVE_RESPONSE.md section 4.
// Re-measured with the throttle fix (cc9_dive_throttle, docs/DIVE_THROTTLE.md 4): the
// swing settles and the tail lifts the brake, but releases stay 0 because the
// aimglide pitch target 009C5522-009C55DF is unbound. Still OFF.
constexpr bool kAimDiveTailBound = true;
//  * kDiveAimHeadingBlendBound: packet cc9_dive_aim_heading (docs/GUNNERY_OPEN_ITEMS.md
//    178). 009C4F80 has a third arm the host lacked (009C508A..009C516C): for a
//    pitch between -60 and -40 degrees the body-axis bearing B is blended toward the
//    inverted-corrected Euler heading E, B - t wrap(B - E), t rising 0 -> 1 from -60 to
//    -40 degrees. OFF: B for every pitch at or below -40 degrees, as before.
constexpr bool kDiveAimHeadingBlendBound = true;  // ON: GUNNERY 178.1
// Packet cc9_dive_flight_response: class+164h Accel as the image's reader leaves
// it (007D20F3-007D2127). docs/DIVE_FLIGHT_RESPONSE.md. OFF, measured: main 32 -> 24
// releases (local/S1fr_9000.log); the throttle-fix pair lacks its control.
constexpr bool kPlaneAccelCheatScaleBound = true;
// Packet cc9_dive_throttle: goaway's throttle and air-brake commands on both
// sides of the nose-down split, 009C4C0C-009C4CA7 and 009C4CBA-009C4CE1.
// docs/DIVE_THROTTLE.md section 1.
constexpr bool kGoawayThrottleBound = true;
// Packet cc9_dive_throttle: the aimglide tick's throttle and air brake
// 009C55E5-009C5679 and its cmd+2D8h = 0 at 009C567F, which this host lacked
// (so aimglide ran in the per-think speed mode). docs/DIVE_THROTTLE.md 2.
// ON with kGoawayThrottleBound, measured (local/G0_9000.log vs C0): the drop
// rows are unchanged, and only the post-release glide and climb-out paths move.
constexpr bool kAimGlideThrottleBound = true;
// Packet cc9_aimglide_pitch: the aimglide pitch target 009C5484-009C55DF
// (cmd+2BCh, cmd+2D0h = 2). docs/AIMGLIDE_PITCH.md section 1.
// OFF, measured with the throttle fix and the tail (U3, local/U3_9000.log):
// releases 16 against the control's 29 (8 against 17 at 4500). With the fix off
// it is unmeasured, so it lands off with the flip.
constexpr bool kAimGlidePitchBound = true;
// Packet cc9_aimglide_pitch: the aimglide yaw arm 009C53E7-009C542A, taken when
// the planar miss is under 140 m; the host ran only the heading arm.
// OFF with kAimGlidePitchBound, for the same measurement.
constexpr bool kAimGlideYawBound = true;
// Packet cc9_flyover_speed: the flyabove desired-speed arm 009C6F97-009C6FFB,
// and approach+50h fed with the aim point's height. docs/FLYOVER_SPEED.md.
// ON: USN04 moves only through the approach+50h feed on the two Yorktown
// squadrons (local/v1 = local/v4, the height-only run); the speed arm itself
// moves no census line there.
constexpr bool kFlyoverSpeedBound = true;

// Packet cc9_hull_axis, diagnostic only. The drawn body-frame offset, the
// world point, the target origin and heading, and the world point resolved
// back onto the target's row 2 (along) and row 0 (across). Row 2 is the axis
// 004142E0 multiplies body z by and the forward axis 00826866 and 0092D300 use.
inline void hull_aim_print(GameHostLog& log, const char* why, const GameUnitSlot& shooter,
                    const GameUnitSlot& target, std::size_t target_plus_one,
                    const std::array<float, 3>& w) {
    const bsp::ApproachTargetRefState& st = shooter.hull_aim_ref;
    const float* m = target.world.data();
    const double d[3] = {static_cast<double>(w[0]) - m[12],
                         static_cast<double>(w[1]) - m[13],
                         static_cast<double>(w[2]) - m[14]};
    // pc24-lint: allow diagnostic print only (cc9_hull_axis); nothing reads these values back
    const double r2n = std::sqrt(static_cast<double>(m[8]) * m[8] +
                                 static_cast<double>(m[9]) * m[9] +
                                 static_cast<double>(m[10]) * m[10]);
    // pc24-lint: allow diagnostic print only
    const double r0n = std::sqrt(static_cast<double>(m[0]) * m[0] +
                                 static_cast<double>(m[1]) * m[1] +
                                 static_cast<double>(m[2]) * m[2]);
    // pc24-lint: allow diagnostic print only
    const double along = (d[0] * m[8] + d[1] * m[9] + d[2] * m[10]) / (r2n > 0 ? r2n : 1);
    // pc24-lint: allow diagnostic print only
    const double across = (d[0] * m[0] + d[1] * m[1] + d[2] * m[2]) / (r0n > 0 ? r0n : 1);
    const double vx = target.motion.linear_velocity.x;
    const double vz = target.motion.linear_velocity.z;
    log.notef("hull_aim %s shooter=%s target=%s(#%u) seed=%u body=(%.2f %.2f %.2f)"
              " L=%.1f W=%.1f world=(%.1f %.1f %.1f) origin=(%.1f %.1f %.1f)"
              " pos=(%.1f %.1f %.1f) row2=(%.3f %.3f %.3f) row0=(%.3f %.3f %.3f)"
              " along=%.2f across=%.2f vel_heading=%.3f",
              why, shooter.row.name.c_str(), target.row.name.c_str(),
              static_cast<unsigned>(target_plus_one), shooter.hull_aim_seed,
              st.body_offset_28[0], st.body_offset_28[1], st.body_offset_28[2],
              target.motion_class.hull_length, target.class_width_00a4,
              w[0], w[1], w[2], m[12], m[13], m[14],
              target.motion.position[0], target.motion.position[1],
              target.motion.position[2], m[8], m[9], m[10], m[0], m[1], m[2],
              along, across, std::atan2(vx, vz));
}

// The world aim point for `shooter` against `target`. Returns false when the
// target supplies no hull, in which case the caller keeps the origin it had.
//
// kHullAimSharedStreamCc13Bound: 00816650's draws on the shared stream 1, in its count
// and order (include/bsp/hull_aim_shared_stream_cc13.hpp). A target that does not sample
// its hull has 0042D810 at vtable[100h], which draws nothing.
inline bsp::ShipLeadRandomDraws hull_aim_shared_stream_draws_00816650(
    const bsp::ApproachTargetRefState& st, bool samples, float section_total,
    GameGunneryHost& stream, std::size_t shooter) {
    bsp::ShipLeadRandomDraws d{};
    if (!samples) return d;
    bool sections = false;
    if (st.section_chance_64 > 0.0f) {                                   // 00816659, JBE
        d.section_roll = stream.hull_aim_point_draw_00816650(shooter, 0.0f, 1.0f);  // 0081667C
        sections = st.section_chance_64 > d.section_roll;                // 00816685, JBE
    }
    // 00816769..00816775: FLD m64 [00D7A268] = 9.999999747378752e-05 (1e-4f widened),
    // FLD the total, FCOMI, JBE.
    constexpr double kSectionEpsilon = static_cast<double>(1.0e-4f);
    if (sections && static_cast<double>(section_total) > kSectionEpsilon) {
        // 0081677B FSUBRP (total - epsilon at PC24), 00816785 FSTP float.
        const float high = static_cast<float>(
            bsp::cc13x87::x87_sub24(section_total, kSectionEpsilon));
        d.pick = stream.hull_aim_point_draw_00816650(shooter, 0.0f, high);  // 00816796
        return d;
    }
    // 00816820: the box, sub+48h; the low ends are -0.0 [00D7A208] SUBSS the extent.
    d.box_x = stream.hull_aim_point_draw_00816650(shooter, -0.0f - st.spread_48[0],
                                                  st.spread_48[0]);       // 00816883
    d.box_y = stream.hull_aim_point_draw_00816650(shooter, 0.0f, st.spread_48[1]);  // 008168A0
    d.box_z = stream.hull_aim_point_draw_00816650(shooter, -0.0f - st.spread_48[2],
                                                  st.spread_48[2]);       // 008168D5
    return d;
}

inline bool hull_aim_world_point(GameUnitSlot& shooter, const GameUnitSlot& target,
                          std::size_t target_plus_one, float out[3],
                          GameHostLog* log = nullptr,
                          const bsp::ApproachTargetSections* sections = nullptr,
                          GameHostLog* section_log = nullptr,
                          GameGunneryHost* stream = nullptr,
                          std::size_t shooter_index = 0) {
    if (shooter.hull_aim_target_plus_one != target_plus_one) {
        // A new ordered target means a new sub-object: 009FB200.
        // kAimErrorDrawBound: the image builds the sub-object once, in the
        // approach constructor, and 009C3DA0/009D02A0 write into it; this host
        // builds it on the first query, so the drawn fields are carried over.
        const bsp::ApproachTargetRefState before = shooter.hull_aim_ref;
        shooter.hull_aim_ref =
            bsp::approach_target_ref_construct_009fb200(0.0f);
        if (kAimErrorDrawBound && shooter.aim_error_drawn) {
            bsp::ApproachTargetRefState& st = shooter.hull_aim_ref;
            st.bias_34 = before.bias_34;
            st.spread_48 = before.spread_48;
            st.section_chance_64 = before.section_chance_64;
            st.weight_68 = before.weight_68;
            st.weight_6c = before.weight_6c;
            st.weight_70 = before.weight_70;
            st.projtime_44 = before.projtime_44;
        }
        shooter.hull_aim_target_plus_one = target_plus_one;
        shooter.hull_aim_seed = ++g_hull_aim_pick_counter;
    }
    bsp::ApproachTargetRefState& st = shooter.hull_aim_ref;
    const bool samples = hull_aim_target_samples_hull(target);
    if (st.dirty_41) {
        // 009FAEB0-009FAEB8, the dirty byte the constructor set at 009FB272.
        bsp::LeadAimHullExtents hull;
        hull.length = target.motion_class.hull_length;  // class+A0h `Length`
        hull.width = target.class_width_00a4;           // class+A4h `Width`
        hull.height = target.motion_class.hull_height;  // class+A8h `Height`
        const std::array<float, 4> unit =
            bsp::approach_target_ref_unit_draws_substitute(shooter.hull_aim_seed);
        const float section_total = (samples && sections != nullptr)
            ? bsp::approach_target_ref_section_total(st, *sections) : 0.0f;
        bsp::ShipLeadRandomDraws draws{};
        if constexpr (bsp::kHullAimSharedStreamCc13Bound) {
            // LABELLED: with no gunnery host (a bare units host) there is no shared
            // stream, and the pick keeps the counter-hash values.
            if (stream != nullptr) {
                draws = hull_aim_shared_stream_draws_00816650(st, samples, section_total,
                                                              *stream, shooter_index);
            } else {
                draws = bsp::approach_target_ref_draws_from_unit(unit, st.spread_48,
                                                                 section_total);
            }
        } else {
            draws = bsp::approach_target_ref_draws_from_unit(unit, st.spread_48, section_total);
        }
        bsp::approach_target_ref_pick_009fa260(st, hull, draws, samples,
                                               samples ? sections : nullptr);
        if (samples && sections != nullptr && st.section_chance_64 > draws.section_roll) {
            // The census of 0081667C..008167EB's outcome, recomputed.
            const bsp::ApproachTargetSections& sc = *sections;
            if (!sc.engine_room_available || !sc.magazine_available
                || !sc.fuel_tank_available) {
                ++g_section_unavailable_picks;
            }
            if (section_total > bsp::kInterceptCoefficientEpsilon) {
                const float w0 = (st.weight_68 > 0.0f && sc.sections.engine_room.present
                    && sc.engine_room_available) ? st.weight_68 : 0.0f;
                const float w1 = (st.weight_6c > 0.0f && sc.sections.magazine.present
                    && sc.magazine_available) ? st.weight_6c : 0.0f;
                const int which = w0 > draws.pick ? 0 : (w0 + w1) > draws.pick ? 1 : 2;
                ++g_section_picks[which];
                if (section_log != nullptr && g_section_picks[0] + g_section_picks[1]
                        + g_section_picks[2] <= 40) {
                    static const char* const kNames[3] = {"engine_room", "magazine",
                                                          "fuel_tank"};
                    section_log->notef("section aim %s -> %s: %s body=(%.1f %.1f %.1f) "
                        "(00816650, packet cc9_ship_section_points)",
                        shooter.row.name.c_str(), target.row.name.c_str(), kNames[which],
                        static_cast<double>(st.body_offset_28[0]),
                        static_cast<double>(st.body_offset_28[1]),
                        static_cast<double>(st.body_offset_28[2]));
                }
            } else {
                ++g_section_chance_box_picks;
            }
        }
        if (kHullAimOffsetEnabled && samples && log != nullptr) {
            // Packet cc9_hull_axis: one line per draw, so a re-draw shows up
            // as a second line for the same (attacker, target).
            std::array<float, 3> w{};
            bsp::transform_point_004142e0(st.body_offset_28, target.world, w);
            hull_aim_print(*log, "draw", shooter, target, target_plus_one, w);
        }
    }
    if (!samples || !kHullAimOffsetEnabled) return false;
    // 009FAED0-009FAF00.
    bsp::approach_target_ref_store_world_point_009faeea(st, target.world);
    out[0] = st.world_point_1c[0];
    out[1] = st.world_point_1c[1];
    out[2] = st.world_point_1c[2];
    return true;
}

// Packet cc9_land_convoy_movement: one LandConvoy's element state and its Path.
// Offsets name the native fields each member stands for.
struct LandConvoyMotionState {
    std::string name;
    bsp::LandConvoyFormation formation;          // +354h..+368h, +3A8h, +3A9h
    float live_arc_3ac{0.0f};                     // seeded by "Offset"
    float committed_arc_3b0{0.0f};
    float odometer_3b4{0.0f};                     // zeroed at 00742C58
    std::vector<std::size_t> member_units;        // +398h, one per slot; npos = null
    std::string path_name;
    std::vector<bsp::CameraPathKnotWords> knots;  // the Path's knot records
    std::vector<void*> knot_pointers;             // path +8h..+0Ch
    void** knots_begin_08{nullptr};
    void** knots_end_0c{nullptr};
    void* path_parent_14{nullptr};                // unused by the local sampler
    std::uint8_t closed_24{0};
    bsp::CameraPathKnotTotals totals;
    float path_local_74[16]{};                    // [path+14h]+74h, 007B03C0
    std::size_t parent_scene_id{0};               // convoy +3A4h
    float parent_world_cc[16]{};                  // parent +CCh..+108h
    std::shared_ptr<const SceneTerrainHeightField> terrain;  // parent +3D0h
    bool world_pose_law_3c8{false};
    std::size_t steps{0};
    std::size_t placements{0};
    std::size_t range_errors{0};
};

// Packet cc13-convoy, under kLandConvoyBodiesBound: what cc15's bodies read and
// write on one convoy (include/bsp/cc15_land_convoy.hpp). The bodies take this
// object's address as the convoy pointer and unit index + 1 as a member pointer;
// neither is dereferenced by them.
struct LandConvoyLifeState {
    std::string name;
    std::vector<std::size_t> slots;  // +394h vector (+398h..+39Ch); npos = a null slot
    std::int32_t live_3bc{0};        // +3BCh
    bool killed_5f{false};           // +5Fh, set by 00926D90 on the convoy
    bool pending_destroy_60{false};  // +60h; stays false under either switch: kLandConvoyDeadWired models only 00929800's write, not 00926C80's +60h store
    std::int32_t cause_70{0};        // +70h
    bool kill_queued{false};         // on 00F899B4; its +080h (00742180) not yet run
    struct Member {
        std::size_t unit{0};
        LandConvoyLifeState* convoy_738{nullptr};  // member +738h; 00742180 clears it
        bool on_killed_run{false};                 // its +080h (0074CD10) has run
    };
    std::vector<Member> members;
};

inline namespace units_local {
// pc24w1-sweep3-5: 0042B2F0 BSP_Vector3_LengthFloatThreshold at PC24, the ON arm of
// kPc24PlaneTasksCc13Bound for this file's copies (avoid_len and 009BFE19's call).
// 0042B2F9..0042B307: (y*y + x*x) + z*z, each op rounded, FSTP float (0042B309);
// 0042B30C..0042B319: +0 at or below the double 1e-10 (00CE3820, unordered included),
// else 00BF7030's FSQRT at PC24 and FSTP float.
inline float length_0042b2f0_pc24(const float v[3]) {
    namespace pc24 = bsp::cc13x87;
    const float d2 = static_cast<float>(pc24::x87_add24(
        pc24::x87_add24(pc24::x87_mul24(v[1], v[1]), pc24::x87_mul24(v[0], v[0])),
        pc24::x87_mul24(v[2], v[2])));
    return static_cast<double>(d2) > 1e-10   // 00CE3820 qword
               ? static_cast<float>(pc24::x87_sqrt24(d2))
               : 0.0f;
}

// cc13-pc24bear, the ON arms of kPc24PlaneTasksCc13Bound for the dive-bomb approach
// feed's planar pairs. Each image chain loads two floats, FSUBs one float operand
// and FSTPs the difference as float (009C7B34..009C7B56, 009C5950..009C596F,
// 009C5207..009C522E), so the differences are rounded before use.
inline float dive_bomb_diff_pc24(float a, float b) {
    return static_cast<float>(bsp::cc13x87::x87_sub24(a, b));
}
// 009C7B5A..009C7B8F (+BCh), 009C5A1C..009C5A4D (+5Ch), 009C52C7..009C52F8 (+14h):
// each square and the sum round to 24 bits, FSTP float; the 1e-10 qword (00CE3820)
// floor with JBE, then 00BF7030's FSQRT at PC24 and FSTP float.
inline float dive_bomb_planar_pc24(float dx, float dz) {
    namespace pc24 = bsp::cc13x87;
    const float d2 = static_cast<float>(
        pc24::x87_add24(pc24::x87_mul24(dz, dz), pc24::x87_mul24(dx, dx)));
    return static_cast<double>(d2) > 1e-10   // 00CE3820 qword
               ? static_cast<float>(pc24::x87_sqrt24(d2))
               : 0.0f;
}
// 009C7BB2..009C7BD9 (+C0h) and 009C5AB4..009C5ADB ([ESP+18h]): 00BF701A's atan2
// is FSTP'd as float, 009C7BBF's FSUBR qword pi/2 (00CE3830) rounds to 24 bits and
// is FSTP'd, and a negative takes FADD qword 2pi (00CE3828) and another FSTP.
// SUBSTITUTION, labelled (rev9 N1): std::atan2 in double, then the float cast, stands
// for 00BF701A LIBCRT_atan2 (MOV EDX,00E15C20; JMP 00C07EE0 __cintrindisp2). Its
// table entry's byte +0Eh is 10h, not 5, so __trandisp2 loads control word 133Fh
// (00C081DE..00C081E9: PC = 64-bit, round to nearest) and FPATAN runs at extended
// precision, not at the caller's PC24; the caller's FSTP then rounds once. The host
// rounds through double, so a last-bit difference is possible in rare cases. (00BF7030's
// sqrt, by contrast, keeps the caller's PC bits at 00C083A5, so its FSQRT is PC24.)
inline float dive_bomb_bearing_pc24(float dx, float dz) {
    namespace pc24 = bsp::cc13x87;
    const float a = static_cast<float>(
        std::atan2(static_cast<double>(dz), static_cast<double>(dx)));
    float b = static_cast<float>(pc24::x87_sub24(bsp::dive_bomb_constant::kHalfPi, a));
    if (b < 0.0f) {
        b = static_cast<float>(pc24::x87_add24(b, bsp::dive_bomb_constant::kTwoPi));
    }
    return b;
}
}  // namespace units_local  // namespace

inline namespace units_local {
// Only the command-list projection of 0046F160's database is owned here.
// Its three DWORDs are zeroed by that constructor at +14Ch/+150h/+154h.
struct UnitSceneCommandRecordImage {
    GameUnitSlot* owner;
    std::uint32_t command_length;
    const char* command;
    std::uint32_t target_length;
    const char* target;
};
struct UnitSceneCommandNodeImage {
    UnitSceneCommandNodeImage* previous;
    UnitSceneCommandNodeImage* next;
    UnitSceneCommandRecordImage* record;
};
struct UnitSceneCommandDatabaseImage {
    std::uint8_t unused[0x14c];
    std::uint32_t count;
    UnitSceneCommandNodeImage* head;
    UnitSceneCommandNodeImage* tail;
};
static_assert(sizeof(UnitSceneCommandRecordImage) == 0x14);
static_assert(offsetof(UnitSceneCommandRecordImage, command_length) == 4);
static_assert(offsetof(UnitSceneCommandRecordImage, command) == 8);
static_assert(offsetof(UnitSceneCommandRecordImage, target_length) == 0xc);
static_assert(offsetof(UnitSceneCommandRecordImage, target) == 0x10);
static_assert(sizeof(UnitSceneCommandNodeImage) == 0xc);
static_assert(offsetof(UnitSceneCommandNodeImage, previous) == 0);
static_assert(offsetof(UnitSceneCommandNodeImage, next) == 4);
static_assert(offsetof(UnitSceneCommandNodeImage, record) == 8);
static_assert(offsetof(UnitSceneCommandDatabaseImage, count) == 0x14c);
static_assert(offsetof(UnitSceneCommandDatabaseImage, head) == 0x150);
static_assert(offsetof(UnitSceneCommandDatabaseImage, tail) == 0x154);
static_assert(sizeof(UnitSceneCommandDatabaseImage) == 0x158);

class UnitSceneCommandQueue final {
public:
    UnitSceneCommandQueue() : strings_(game_native_string_process().raw_context()) {}
    ~UnitSceneCommandQueue() {
        std::uint32_t saved_this;
        bsp::clear_native_scene_command_list_0046a9f0(
            &database_.count, &saved_this, strings_);
    }
    void enqueue(GameUnitSlot& owner, const GameSceneEntityRecord& entity) {
        // 004E6B30 supplies the creator's entity and the carried property-bag
        // strings. 00469680 RET 0Ch confirms all three incoming stack words.
        const std::uint32_t parameters[3] = {
            reinterpret_cast<std::uint32_t>(&owner),
            reinterpret_cast<std::uint32_t>(entity.command.c_str()),
            reinterpret_cast<std::uint32_t>(entity.command_target.c_str())};
        std::uint32_t saved_allocation;
        std::uint32_t constructor_parameters[3];
        std::uint32_t constructor_saved_record;
        // Explicit native cells: P+4 incoming arguments, P-10h allocation,
        // P-20h constructor arguments, P-34h child saved record. P-18h is
        // reused by the list argument, aliasing the outgoing target word.
        const bsp::NativeSceneCommandQueueFrame frame{parameters, &saved_allocation,
            constructor_parameters, &constructor_saved_record, &constructor_parameters[2]};
        bsp::queue_native_scene_command_00469610(&database_, frame, strings_);
    }
    void take(GameUnitSlot& owner) {
        // The actual queued record feeds the existing 0046AAB0/0077D600
        // consumer at authored-command issue, after every entity was created.
        // Generated-object issue consumes its own record by owner identity.
        for (auto* node = database_.head; node != nullptr; node = node->next) {
            const auto* record = node->record;
            if (record == nullptr || record->owner != &owner) continue;
            owner.row.command.assign(record->command != nullptr ? record->command : "",
                record->command_length);
            owner.row.command_target.assign(record->target != nullptr ? record->target : "",
                record->target_length);
            return;
        }
    }
    void retire(GameUnitSlot& owner) {
        // 0046AAB0 issues first; its 0046A9F0 tail releases the record only
        // after the consumer returns. Keep that lifetime for each host issue.
        for (auto* node = database_.head; node != nullptr; node = node->next) {
            if (node->record == nullptr || node->record->owner != &owner) continue;
            std::uint32_t saved_this;
            bsp::delete_native_scene_command_record_pointer_0046a090(
                &node->record, &saved_this, strings_);
            if (node->previous != nullptr) node->previous->next = node->next;
            else database_.head = node->next;
            if (node->next != nullptr) node->next->previous = node->previous;
            else database_.tail = node->previous;
            --database_.count;
            bsp::singleton_lifetime_free(node);
            return;
        }
    }
private:
    bsp::NativeStringRawPoolContext& strings_;
    UnitSceneCommandDatabaseImage database_{};
};
}  // namespace units_local // namespace

struct GameUnitsHost::Impl {
    Impl(GameHostLog& log_in, GameMissionLuaHost& lua_in)
        : log(log_in), lua(lua_in), commands(log_in) {}
    ~Impl() {
        // Source mission retirement: the process singleton outlives these
        // slot+1 unit identities. Retire their records before slots are freed;
        // clear() also admits a shared drain that already destroyed the owner.
        leg_manager_cc13_.clear();
    }

    GameHostLog& log;
    GameMissionLuaHost& lua;
    GameObserverRuntime* observer_runtime{nullptr}; // application-owned, borrowed
    // Milestone 2l: the weapon director of every created unit, and the three
    // message hops between the authored `Command` token and its command slot.
    GameCommandsHost commands;
    // 0046F160 before scene creation; 004E6BAB queues until 0046ED1E's
    // resolve pass, or the Lua generated-object drain. 0046A9F0 frees any
    // unconsumed records before this mission owner is retired. Constructed
    // only in the chain-51 ON arm; borrows the canonical process string pool.
    std::unique_ptr<UnitSceneCommandQueue> scene_commands;
    // Packet cc9_land_convoy_movement, under kLandConvoyMovementBound.
    std::vector<LandConvoyMotionState> land_convoys;
    // Packet cc13-convoy, under kLandConvoyBodiesBound: one per roster built.
    std::vector<std::unique_ptr<LandConvoyLifeState>> land_convoy_lives;
    // Packet cc9_air_ops_squadron_registry.
    unsigned long long departed_block_reads{0};
    // Packet cc9_squadron_land_task, under kSquadronReturnToBaseResolveBound.
    struct ReturnToBaseCensus {
        std::string squadron;
        unsigned long long resolutions{0};
        int last_arm{-1};
        std::string last_site;
        std::string last_note;
    };
    std::vector<ReturnToBaseCensus> rtb_census;
    unsigned long long rtb_arm_counts[4]{};
    // Packet cc13-luawire 7b. The air-ops block lists at +B4h that 006CA730 fills
    // (SquadronSetBaseUnsupported), as (deck name, squadron record name) pairs.
    // 006C4790 answers false for a listed squadron; read under
    // kLuaSquadronSetBaseUnsupportedBound.
    std::set<std::pair<std::string, std::string>> base_unsupported_b4;
    bool squadron_excluded_006c4790(const std::string& deck,
                                    const bsp::PlaneSquadronHostRecord& sq) const {
        if constexpr (bsp::kLuaSquadronSetBaseUnsupportedBound) {
            return base_unsupported_b4.count({deck, sq.name}) != 0;
        } else {
            static_cast<void>(deck);
            static_cast<void>(sq);
            return false;
        }
    }
    unsigned long long base_unsupported_refusals{0};
    void record_return_to_base_007f16d0(std::size_t unit_index);
    // Packet cc9_land_task_reach, under kSquadronLandTaskBound: one row per
    // squadron whose members received a `returntobase` placement.
    struct LandTaskCensus {
        std::string squadron;
        unsigned long long placements{0};
        unsigned long long installs{0};
        unsigned long long kept{0};            // 009B3560 answers 1: same land, same site
        unsigned long long refused_arm{0};     // 007F16D0 did not answer `land at site`
        unsigned long long refused_unread{0};  // the resolution carries an unread input
        unsigned long long refused_ground{0};  // not airborne: land/park, unbound
        unsigned long long ground_parked{0};   // not airborne: started in land/park (ON)
        unsigned long long refused_dead{0};
        std::string last_refusal;
        double first_install_at{-1.0};
    };
    std::vector<LandTaskCensus> land_census;

    // Packet cc9_plane_retreat_task: the retreat task's census.
    unsigned long long retreat_orders{0}, retreat_installs{0}, retreat_refused{0},
        retreat_retired{0}, retreat_exits{0}, retreat_exit_members{0},
        retreat_exit_gated{0}, retreat_latches_361{0}, retreat_rtb_installs{0};
    std::vector<std::string> retreat_latched_361;   // squadron+361h, by squadron name
    // Packet cc13-leavecd (bsp::kLeaveCountdownAllSquadronsCc13Bound): squadron+36Ch,
    // the 00982120 warning countdown, by squadron name (a plane with no squadron
    // record by its own name), and the census.
    std::map<std::string, float> leave_warn_36c;
    unsigned long long leave_countdown_calls{0}, leave_counting_steps{0},
        leave_counting_not_retreat{0};
    // Goal 5 chain 668: the 007C6D65 warnings (00982120(plane, 0)) and the 007CA790
    // ExitZoneParty stores (made / without a border zone at the plane / no self slot).
    unsigned long long leave_warnings_00982120{0}, exit_zone_party_stores{0},
        exit_zone_party_no_zone{0}, exit_zone_party_no_slot{0};
    // BSP_MissionEntity_Kill's cause (00926D90's argument) for the kills this
    // host passes one other than 1; 00929800 names it from 00E0CF04.
    std::vector<int> kill_cause_by_unit;
    // Packet cc9_squadron_spent_ordnance_rtb: per squadron, the latch
    // [+38h]+3Ch, the stage-2 ending and the B5 issue, by squadron name.
    struct SpentOrdnanceRtb {
        bool latch_3c{false};
        bool ended{false};
        std::size_t retest_target{0};   // the +3Dh latch's target (009F8412), 0 = clear
        bool issued{false};
        float latch_at{-1.0f}, ended_at{-1.0f}, issued_at{-1.0f};
        std::uint32_t command{0};
        std::size_t placed{0};
        std::uint32_t census_unlatched_class{0xFFFFFFFFu};   // BSP_SPENT_RTB_CENSUS only
        // Packet cc9_l57_moveto_self: the command the retest ended at stage 2, and
        // whether 0084E595's moveto-self is the squadron's current order.
        std::uint32_t ended_stage2_command{0};
        bool moveto_self{false};
        // cc13-fix1 (kSquadronCommandBuildingMoveToBound): 009F7D75 queued its moveto
        // into the same director before that end, so 0071BE60() is 2, not 1.
        bool building_moveto_queued{false};
        // Packet cc9_p12_land_target_gone: the land target (unit index + 1) whose
        // loss latched +3Ch, so one lost site latches once.
        std::size_t land_gone_site{0};
        // Packet cc9_p12_fighter_spent_3d: the command a spent fighter squadron's
        // +3Dh ended, so one command is ended once.
        std::uint32_t fighter_ended_command{0};
    };
    unsigned long long moveto_self_issues{0}, moveto_self_placed{0}, moveto_self_wings{0};
    std::map<std::string, SpentOrdnanceRtb> spent_rtb;
    // 009F75F4 initializes auto-target+28h to zero. It survives the native
    // clear-latches operation 009F6DD0, so keep it outside SpentOrdnanceRtb:
    // the host resets that record when a landing target disappears.
    std::map<std::string, bool> squadron_rockets_previous_28;
    // Packet cc13-steptail (kSquadronGeneralStepTailBound): the head the general
    // tail last issued on, so it issues once per head.
    struct StepTailGuard {
        bool issued{false};
        std::uint32_t head{0};
        bsp::SceneCommandTarget head_target{};
    };
    std::map<std::string, StepTailGuard> step_tail_guard;
    unsigned long long step_tail_movetos{0}, step_tail_rtbs{0};
    // Packet p15_free_attack_tick (docs/OCEAN_WAVE_FIELD.md section 57). True: each
    // squadron runs 009F8470's own part every step after 009F8160 / 009F7C90 (cc13's
    // reconstruction over src/squadron_free_attack.cpp's arena): a pick sends the
    // override 0071D980 to every live member's director and builds that class's
    // task (0099A170); when the override is cleared the head command's task is
    // built again. With BSP_FREE_ATTACK_CENSUS=1 the tick runs and is logged while
    // the switch is false, and nothing is sent. False: nothing runs.
    static constexpr bool kSquadronFreeAttackTickBound = kSquadronFreeAttackOverrideBound;
    struct FreeAttack {
        std::uint32_t override_command{0};   // the class this host sent, 0 = none live
        std::size_t override_target{0};      // unit + 1
        bool override_squadron{false};       // the target is a squadron record
        std::uint32_t head_command{0};       // slot 0 when it was sent
        int last_stop{-1};
        std::size_t last_target{0};
        // Packets cc13-fapick / cc13-fapick3, log-only: pick lines printed (cap 8)
        // and the last printed pick, (unit + 1) * 2 + squadron.
        int pick_lines{0};
        std::size_t last_printed_pick{0};
    };
    std::map<std::string, FreeAttack> free_attack;
    unsigned long long free_attack_ticks{0}, free_attack_stops[8]{}, free_attack_sends{0},
        free_attack_installed{0}, free_attack_tasks{0}, free_attack_stage_raises{0},
        free_attack_cleared{0}, free_attack_restored{0}, free_attack_moveto_self{0};
    unsigned long long spent_rtb_latches{0}, spent_rtb_ends{0}, spent_rtb_issues{0};
    unsigned long long rack_device_reloads{0};   // packet cc9_rack_device_reload
    unsigned long long torpedo_install_refused_class{0};   // packet cc9_dkz_spent_rtb
    // Packet cc9_dkz_spent_rtb: 009AE1B0, the drop-kamikaze task's break-off
    // (task vtable +1Ch), asked by 007EE6B0 for 009F7C90's ending. Inputs as the
    // torpedo feed takes them: 0099C230 true (no in-mission interface manager),
    // the target the unit's command resolves to and its +5Dh (state->simulate),
    // reload as ctl+369h (1) and [00E17BF2], the task's state and +4B9h.
    bool drop_kamikaze_breaks_off_009ae1b0(const GameUnitSlot& u) const {
        bsp::DropKamikazeBreakOffInputs bi;
        bi.base_allows = true;
        const GameUnitSlot* tgt = nullptr;
        if (u.command_target_plus_one != 0 && u.command_target_plus_one - 1 < slots.size())
            tgt = slots[u.command_target_plus_one - 1].get();
        bi.has_target_440 = tgt != nullptr;
        bi.target_marked_5d = tgt != nullptr && tgt->state != nullptr && tgt->state->simulate != 0;
        bi.reload_enabled = kLuaDeviceReloadEnabledBound && lua_device_reload_enabled_00e17bf2();
        bi.current = u.dkz.state;
        bi.has_ordnance_4b9 = u.dkz.ap.has_ordnance_c1;
        return bsp::drop_kamikaze_should_break_off_009ae1b0(bi);
    }
    unsigned long long retest_dead_targets{0}, retest_ends{0};   // kSquadronDeadTargetRetestBound
    unsigned long long attack_task_target_retires{0};   // kAttackTaskTargetRetireBound
    unsigned long long touchdown_human_pilot{0};   // 007CB75F census (SQUADRON_LAND_TASK 5hw.4)
    // Packet cc9_ship_exit_zone (docs/SHIP_AI_OPEN_ITEMS.md 232; cc13-ver7 Finding 1,
    // docs/CC13_VER_7.md). 00826D70 is vtable slot +1D8h of every ship class, which
    // 00953CC0 (called at 00825FAC, the head of the ship motion tick) runs at 00953D98
    // while unit+520h is clear. Outside the map and held by 00811F50(1) the timer +6F0h
    // counts down; past Retreat.ExitDist or below zero 00811E80 routes MT_UNITEXITED and
    // the ship's handler 0081BEB0 kills it with reason 4 ("exitzone"). True: the host
    // runs it for every live ship. False: no ship ever leaves the map, as before.
    // ON by mechanism (cc9-ships47, SHIP_AI 232): wins identical, the JM06 probe exits.
    static constexpr bool kShipOutOfMapExitBound = true;
    // Packet g49_exit_zone_role_gate (GUNNERY 266.3, cc13-sweepB adoption note 2).
    // 0081BEDF..0081BF01: 0081BEB0 sends the exitzone channel 00982120(unit, 1) only
    // when 00927C50(unit, [game+18ECh]) answers true: one of the nine role words
    // unit+188h.. is 9 (open) or the local player's slot. True: that gate, the local
    // slot being kLocalPlayerSlot (0) as elsewhere in this host. False: every exit is
    // sent, as before. Kill(4) runs either way.
    // ON (GUNNERY 266.5): JM06 and USN12 gameplay identical; every ship that left
    // the map on them had an open role word, so the gate never refused.
    static constexpr bool kShipExitZoneRoleGateBound = true;
    unsigned long long ship_exit_role_gated{0};
    unsigned long long ship_exit_calls{0}, ship_exit_counting{0}, ship_exit_warnings{0},
        ship_exit_kills{0};
    void note_kill_cause(std::size_t unit, int cause) {
        if (kill_cause_by_unit.size() <= unit) kill_cause_by_unit.resize(unit + 1, 1);
        kill_cause_by_unit[unit] = cause;
        // Packet cc13-killx (kPlaneFunnelKillPathTagCc13Bound): every caller here
        // stands for a 00926D90 Kill, except a site that raised
        // funnel_path_tag_suppressed around its kill_unit_00926d90 call.
        if (!funnel_path_tag_suppressed) {
            if (funnel_path_tag_by_unit.size() <= unit) funnel_path_tag_by_unit.resize(unit + 1, 0);
            funnel_path_tag_by_unit[unit] = 1;
        }
    }
    // Packet cc13-killx: the unit died through a 00926D90 stand-in (not SetHealth).
    // Grown per slot index at the kill, read by the plane death-mode block one
    // step later; nothing clears it (slots are never freed).
    std::vector<std::uint8_t> funnel_path_tag_by_unit;
    // Raised by the host sites that call kill_unit_00926d90 for a direct
    // vtable[70h](1), which is not 00926D90; only through FunnelPathTagSuppressScope.
    bool funnel_path_tag_suppressed = false;
    struct FunnelPathTagSuppressScope {
        bool& flag;
        bool previous;
        explicit FunnelPathTagSuppressScope(Impl& host)
            : flag(host.funnel_path_tag_suppressed), previous(host.funnel_path_tag_suppressed) {
            flag = true;
        }
        ~FunnelPathTagSuppressScope() { flag = previous; }
        FunnelPathTagSuppressScope(const FunnelPathTagSuppressScope&) = delete;
        FunnelPathTagSuppressScope& operator=(const FunnelPathTagSuppressScope&) = delete;
    };
    unsigned long long land_retired_invalid{0};
    unsigned long long land_retired_own_target_ok{0};   // packet cc9_land_command_own_target
    // Packet cc9_unit_yaw_rate_forward_speed: the accessor's calls and non-zero answers.
    unsigned long long yaw_rate_calls_00811940{0};
    unsigned long long yaw_rate_nonzero_00811940{0};
    unsigned long long land_profile_calls{0};
    unsigned long long land_profile_writes{0};
    LandTaskCensus& land_census_for(const std::string& squadron);
    void install_land_task_0099a3dd(std::size_t unit_index);
    // Packet cc9_moveto_arrival_end_command (docs/PILOT_MOVETO_TASK.md, "The
    // arrival ends the command"). True: once approach+5Ch is set, 009C3647's
    // approach vtable[8] = 009C3100 runs: when the task's vtable[40h] 009C31B0
    // answers non-zero, the squadron's moveto (00E08F68) ends through 0071E430
    // (terminal 0), and each member's 0099A4C0 retires its kind-7 task. False:
    // the call is a record and the flight keeps its moveto task.
    static constexpr bool kMoveToArrivalEndCommandBound = true;  // ON: stage-only, six rows identical (docs/PILOT_MOVETO_TASK.md)
    // Packet cc9_moveto_command_range (docs/PILOT_MOVETO_TASK.md, "The command block's
    // range"). 009C3594-009C35CB: while the squadron's +348h block (squadron vtable[114h],
    // 007ECFD0) has the moveto class 00E08F68 in its queue head slot (+54h), approach+6Ch
    // takes the slot's trailing parameter (+6Ch = slot +18h), which PilotMoveToRange's
    // third argument writes (008A46DC). The circle state's radius is then
    // max(approach+6Ch, TurnCircleRadius) (009C27A6). True: the unit's issued moveto
    // range stands for the block's head. False: 0 stands in, so r = TurnCircleRadius.
    static constexpr bool kMoveToCommandRangeBound = true;  // ON: stage-only, twelve rows identical (docs/PILOT_MOVETO_TASK.md)
    void moveto_arrival_end_command_009c3100(GameUnitSlot& unit);
    unsigned long long moveto_end_calls{0}, moveto_end_no_squadron{0}, moveto_end_not_moveto{0};
    unsigned long long moveto_end_commands{0}, moveto_end_retired{0}, moveto_end_promoted{0};
    unsigned long long moveto_end_stage_only{0};
    bool install_land_task_core_009b41c0(std::size_t unit_index,
        const bsp::PlaneSquadronHostRecord& sq, std::size_t owner, std::size_t deck,
        const std::string& site_name, bool explicit_site);
    bool land_command_still_valid_009b34d0(const GameUnitSlot& unit) const;
    // Packet cc9_landing_sequencer (kLandingSequencerBound): each deck's landing
    // holder (block+80h), queue (block+98h) and assignment vector (block+A8h),
    // indexed as bsp::air_ops_decks(). docs/SQUADRON_LAND_TASK.md section 5c.
    struct LandingAssignment {           // 14h bytes at block+A8h
        std::size_t plane{0};            // +0h
        float path_4{0.0f};              // +4h, 006C6020
        float spacing_8{1.0f};           // +8h, 006C3F80
        bool held_0c{false};             // +0Ch, never set after the insert
        bool side_0d{false};             // +0Dh
        int mode_10{1};                  // +10h, 006C7960
    };
    struct LandingQueueEntry {           // 14h bytes at block+98h, 006C0B50
        std::size_t squadron{0};         // +0h, the registry record index
        int n{0};                        // +8h
        float countdown_0c{0.0f};        // +0Ch
        float interval_10{1.0f};         // +10h
    };
    // Packet cc9_carrier_elevator: the mother-ship site's platform at site+44h
    // (006FC2A0, set up by 006FC380 at 006CFD42) and the site's own +18h/+78h/
    // +9Ch. P+0Ch..+14h the lift point, +18h the lift's depth below it, +1Ch
    // depth x 0.1, +34h the platform plane, +38h..+40h its offset, +44h
    // MotherShip.ElevatorSpeed, +48h ElevatorDepth, +4Ch 1.0, +50h the mode
    // (0 still, 1 up, 2 down).
    struct CarrierElevator {
        bool built{false};
        bool refused{false};
        float lift[3]{0.0f, 0.0f, 0.0f};
        float p18{0.0f}, p1c{0.0f}, speed_44{0.0f}, depth_48{0.0f}, p4c{1.0f};
        int mode_50{0};
        std::size_t plane_34{static_cast<std::size_t>(-1)};
        float off_38[3]{0.0f, 0.0f, 0.0f};
        float timer_9c{0.0f};
        unsigned long long ticks{0}, intakes{0}, hides{0}, releases{0}, empty_up{0};
        unsigned long long top_releases{0}, unfed_relaunch{0}, lane_waits{0};
    };
    struct LandingDeck {
        bool built{false};
        bool refused{false};             // a mother-ship deck, or no authored runway
        std::size_t owner{0};
        bsp::CameraMatrix frame_8{};     // holder+8h, the owner's world matrix
        bsp::CameraMatrix inverse_48{};  // holder+48h, 0085DEA0
        float runway_heading_88{0.0f};   // holder+88h
        float t_a4[3]{0.0f, 0.0f, 0.0f}; // holder+A4h, 006BC960
        float width_b0{0.0f};
        float length_b4{0.0f};
        // Packet cc9_carrier_landing_deck: a mother-ship holder (007593D0's),
        // re-framed from the moving ship at each use (00758E80 -> 006BEE40).
        bool mother_ship{false};
        CarrierElevator elevator;   // packet cc9_carrier_elevator
        // The launch site at block+3Ch (packet cc9_plane_touchdown): +40h, which
        // 006CF100 stamps now - 99999.0 (00CF89D0) and 006CE230 restamps, and the
        // occupancy vector +34h/+38h that 006CED90 appends to.
        float site_stamp_40{-99999.0f};
        std::vector<std::size_t> site_occupants_34;
        // site+18h, the ready plane (006CF190 stores it under the observer pair
        // at site+4h), read by both readiness slots (006CDF60, 006CFF40) and by
        // the deck arms. Packet cc9_base_launch_task (kBaseLaunchChainBound).
        std::size_t site_ready_18{static_cast<std::size_t>(-1)};
        unsigned long long site_ready_sets{0};
        std::vector<LandingQueueEntry> queue;
        std::vector<LandingAssignment> assignments;
        unsigned long long lookups{0}, found{0}, passes{0}, inserts{0}, releases{0};
        unsigned long long hit_passes{0}, outside_passes{0}, spacing_mode4_refused{0};
        unsigned long long spacing_mode4_site{0}, landed_arm_hits{0};
        unsigned long long spacing_site_busy{0};   // cc13-carrierdeck G2, log-only
        unsigned long long slot_tail_unapplied{0}, mode_counts[5]{0, 0, 0, 0, 0};
    };
    std::vector<LandingDeck> landing_decks;
    unsigned long long landing_refused_decks{0};
    // Packet cc9_rtb_site_key (docs/SQUADRON_LAND_TASK.md 5ad). True: the
    // 007F16D0 record path gives each 006C0840 candidate its ordering key
    // (006C09FE-006C0A9B) from the head plane, through the airfield holder
    // (landing_deck_006c0750) or a carrier holder frame built as 00758E80's
    // refresh leaves it (006BEE40 on 007593D0's holder). False: every key is
    // unknown, and two or more candidates make the resolution `site-key-unread`.
    static constexpr bool kReturnToBaseSiteKeyBound = true;  // ON: 5ag.2 (JM05 recall faithful, SHIP_AI_OPEN_ITEMS 59)
    // Packet cc9_carrier_landing_deck, part 1 (docs/SQUADRON_LAND_TASK.md 5ae).
    // True: landing_deck_006c0750 builds a mother-ship deck's holder as
    // 007593D0 -> 006C0D20 -> 006C0750 does and re-frames it from the carrier's
    // pose at each use (00758E80's per-update 006BEE40), and 006BA620 answers
    // RunwayLength x 0.3 on it. False: a mother-ship deck is refused.
    static constexpr bool kCarrierLandingDeckBound = true;  // ON: 5ag.2 (part 2 carries the landed plane)
    unsigned long long carrier_decks_built{0}, carrier_deck_refreshes{0};
    // Packet cc9_carrier_landing_deck_part2 (docs/SQUADRON_LAND_TASK.md 5ag).
    // True: a touchdown on a mother-ship deck re-parents the plane to the
    // carrier as 007C71E0 does (007D9CE0's relative velocity, the local pose,
    // the 007C72D3 arrestor seed through 007DB630); the ground roll carries
    // the parented plane with the carrier, runs the wire band 007DBEEE and its
    // consumer 007DC1A7, and scales the wheel friction by the parent's
    // velocity (007DC0A2). False: the plane stays at its world touchdown
    // point and the wire and surface factor are 0 and 1.0.
    static constexpr bool kCarrierDeckParentBound = true;  // ON: 5ag.2 (13 of 13 carrier landings stop on deck)
    void carrier_deck_reparent_007c71e0(GameUnitSlot& p, const LandingDeck& d);
    void carrier_deck_carry_in(GameUnitSlot& p);
    void carrier_deck_capture(GameUnitSlot& p);
    // Packet cc9_carrier_elevator (kCarrierElevatorBound).
    CarrierElevator* carrier_elevator_ready(LandingDeck& d);
    std::array<float, 3> carrier_local_point(const LandingDeck& d, const float* world) const;
    float carrier_elevator_nose_distance_006cfe90(const LandingDeck& d,
        const GameUnitSlot& p) const;
    bool carrier_elevator_intake_006cff70(const LandingDeck& d, const GameUnitSlot& p) const;
    bool carrier_elevator_lift_free_006d0150(const LandingDeck& d, const GameUnitSlot& p) const;
    bool carrier_elevator_lane_006d0390(const LandingDeck& d, const GameUnitSlot& p) const;
    void carrier_elevator_carry_006fc0d0(LandingDeck& d);
    void carrier_elevator_release_006fc250(LandingDeck& d);
    void carrier_elevator_take_006fc720(LandingDeck& d, GameUnitSlot& p);
    void carrier_elevator_site_tick_006d0600(LandingDeck& d, float dt);
    void carrier_elevator_platform_tick_006fc480(LandingDeck& d, float dt);
    void plane_enter_state_two_007cc7a0(GameUnitSlot& p);
    unsigned long long carrier_elevator_state_two{0};
    // Packet cc9_base_launch_inside_start (kBaseLaunchChainBound).
    void plane_enter_state_one_007cc820(GameUnitSlot& p, std::size_t base_index);
    unsigned long long base_launch_inside_entries{0};
    unsigned long long base_launch_inside_squadrons{0};
    unsigned long long base_launch_inside_no_base{0};
    // Packet cc9_base_launch_task (kBaseLaunchChainBound).
    void base_launch_task_tick_007f1f00(bsp::PlaneSquadronHostRecord& sq, float dt);
    unsigned long long base_launch_tasks_built{0}, base_launch_tasks_ended{0};
    unsigned long long base_launch_ticks{0}, base_launch_not_ready{0};
    unsigned long long base_launch_sends{0}, base_launch_owner_dead{0};
    unsigned long long base_launch_no_deck{0};
    // Packet cc9_base_launch_deck_arms (kBaseLaunchChainBound), piece 3.
    void run_airfield_destruction_rule_006d40f0(std::size_t index);
    void ensure_parent_link_71c_006f5cc0(std::size_t index);
    const bsp::game::ScenePathEntry* landing_hangar_path_006d2780_006d2640(
        const LandingDeck& d, bool entry) const;
    void airfield_site_tick_006cf980(std::size_t deck_index, LandingDeck& d);
    bool place_on_launch_spot_007c5f60(GameUnitSlot& p, std::size_t deck_index,
                                       LandingDeck& d, bool airfield);
    void ground_state_from_locked_007ca3f0(GameUnitSlot& p, LandingDeck& d, bool five);
    // Packet cc9_takeoff_task_bind (kBaseLaunchChainBound), piece 4 part 2a.
    void install_takeoff_task_0099a4a0(GameUnitSlot& p);
    void push_ground_retakeoff_0099b107(GameUnitSlot& p, const char* head);
    // Packet cc13-takeoff (kPlaneTakeoffStartNotifyCc13Bound).
    void plane_notify_takeoff_start_007c17d0(GameUnitSlot& p);
    void deliver_takeoff_notify_c3h_007cd02c(GameUnitSlot& p);
    unsigned long long takeoff_notify_calls{0}, takeoff_notify_posts{0};
    unsigned long long takeoff_notify_arm_7cb5f0{0}, takeoff_notify_arm_7c7430{0};
    unsigned long long takeoff_notify_arm_7ca3f0{0}, takeoff_notify_5_to_4{0};
    // Packet cc13-takeoff (kPlaneShipyardLaunchAa0Cc13Bound).
    unsigned long long shipyard_launch_aa0_planes{0}, shipyard_launch_aa0_slow_pushes{0};
    unsigned long long shipyard_launch_aa0_leaves{0};
    // Packet cc9_base_launch_liftoff (kBaseLaunchChainBound), piece 5.
    void plane_liftoff_c6h_007c6f50(GameUnitSlot& p, const char* why);
    unsigned long long base_launch_liftoffs{0}, base_launch_liftoff_unparented{0};
    unsigned long long base_launch_liftoff_site_leaves{0}, base_launch_c01_sets{0};
    // Packet cc9_stowed_plane_stock_return (kStowedPlaneStockReturnBound).
    void plane_stock_return_c7h_007cc8b0(GameUnitSlot& p);
    unsigned long long stock_return_sends{0}, stock_return_adds{0}, stock_return_no_deck{0};
    unsigned long long stock_return_no_squadron{0}, stock_return_slot_returns{0};
    unsigned long long stock_return_site_leaves{0}, stock_return_destroys{0};
    // Goal 4 chain 356: C7h copies made, and returns whose holder fell back to
    // the +BF4h index (no owner id, or no block bound to the copy's id).
    unsigned long long stock_return_c7h_copies{0}, stock_return_c7h_index_holder{0};
    // Packet cc13-landc.
    unsigned long long flight_state_notifies{0}, stock_return_destroy_draws{0};
    unsigned long long combat_death_destroy_draws{0};
    // Goal 5 chain 654: the plane engine lists (007EC5A0) and their calls.
    unsigned long long plane_engine_lists_built{0}, plane_engines_built{0};
    unsigned long long plane_engine_stop_alls{0}, plane_engine_fires_raised{0};
    unsigned long long plane_engines_destroyed{0};
    unsigned long long ground_state_dead_tail_skips{0};
    unsigned long long funnel_kill_destroy_only{0};
    // planepartclasses.lua DeadEngineFireDelay, which 004A9BD0 stores at
    // [00E18718] / [00E18714] (004AA9BD / 004AAA57); read once per host.
    bool dead_engine_fire_delay_read{false};
    float dead_engine_fire_delay[2]{3.0f, 5.0f};
    // The destroy flush's vtable[7Ch] for a plane is 007BCAA0 (00D19D28+7Ch);
    // its first act is 00BD2F10(ECX = 0, [00E18718], [00E18714]) at 007BCABB,
    // stored at +C14h (007BCAC0), which 007CAF10's engine block reads (goal 5
    // chain 654). Takes that stream-0 draw through the gunnery host; false
    // without one.
    bool plane_on_destroyed_draw_007bcabb(GameUnitSlot& plane, std::size_t index) {
        if (gunnery == nullptr) return false;
        if (!dead_engine_fire_delay_read) {
            dead_engine_fire_delay_read = true;
            lua.read_global_number_pair("DeadEngineFireDelay", dead_engine_fire_delay[0],
                dead_engine_fire_delay[1]);
        }
        plane.plane_engine_fire_timer_c14 = gunnery->death_mode_draw_00bd2f10(0, index,
            dead_engine_fire_delay[0], dead_engine_fire_delay[1]);
        return true;
    }
    // Goal 5 chain 654: the engine list 007EC5A0 builds at unit+C98h for a new
    // plane (BSP_Plane_ReadPropertyBag, 007D6327). game_hosts_units.cpp.
    void build_plane_engine_list(GameUnitSlot& plane);
    unsigned long long stock_return_queue_erases{0}, stock_return_queue_misses{0};
    unsigned long long squadron_kill_queue_kills{0}, squadron_kill_queue_erases{0};
    unsigned long long land_retire_leader_calls{0}, land_retire_queue_erases{0};
    unsigned long long land_retire_assignment_erases{0}, land_retire_slot_states{0};
    // Packet cc9_takeoff_own_site: 006C0840's own-site answers.
    unsigned long long takeoff_own_site_answers{0};
    // Packet cc9_base_launch_brake: 006C5B70's releases of block+38h.
    unsigned long long base_launch_brake_releases{0}, base_launch_brake_gone{0}; unsigned long long base_launch_brake_attach_holds{0};  // cc13-fix7 B
    void release_launch_brake_006c5b70();
    void takeoff_prepare_enter_009cdd50(GameUnitSlot& p);
    void takeoff_slow_enter_009ce1d0(GameUnitSlot& p);
    bool takeoff_permission_009cdd10(GameUnitSlot& p);
    bool carrier_site_permission_006d01c0(LandingDeck& d, const GameUnitSlot& p);
    unsigned long long takeoff_installs{0}, takeoff_parking_refused{0};
    unsigned long long takeoff_done{0}, takeoff_to_slow{0}, takeoff_to_run{0};
    // Packet cc13-takeoffdiag (log-only): the takeoff decel lines printed, the
    // planes that printed one, and the lines the caps held back.
    unsigned long long takeoff_decel_lines{0}, takeoff_decel_planes{0},
        takeoff_decel_suppressed{0};
    // Packet cc9_takeoff_task_head.
    unsigned long long land_installs_deferred{0}, land_installs_after_takeoff{0};
    // Packet cc9_ground_retakeoff (kGroundRetakeoffBound).
    unsigned long long retakeoff_pushes{0}, retakeoff_parking{0}, retakeoff_takeoff{0};
    unsigned long long retakeoff_land_head{0}, retakeoff_other_head{0}, retakeoff_state6{0};
    // Packet cc9_throttle_dead_band (kThrottleDeadBandBound): 0099D300's demand
    // arm, increments applied and skipped by the dead band.
    unsigned long long throttle_band_applied{0}, throttle_band_skipped{0};
    unsigned long long takeoff_permission_asks{0}, takeoff_permission_denied{0};
    bool carrier_elevator_blocked_006d02f0(const LandingDeck& d) const;
    bool carrier_site_has_free_runway_006d0530(const LandingDeck& d) const;
    void carrier_elevator_send_empty_down_006fc640(LandingDeck& d);
    void carrier_elevator_raise_plane_006fc810(LandingDeck& d, GameUnitSlot& p);
    // Goal 5 chain 628. 006CFFF0(flag, plane), the reconstructed body run by
    // HostSessionMessages::queue_mothership_elevator_move_87 through the 87h
    // message's local copy, then the carrier's receiver 006D0050 on the copy.
    // `plane` is a slot index or SIZE_MAX for null.
    void route_carrier_elevator_message_87(LandingDeck& d, bool flag, std::size_t plane);
    unsigned long long base_launch_lift_down{0}, base_launch_lift_up{0};
    unsigned long long base_launch_lift_top{0}, base_launch_lift_blocked{0};
    unsigned long long base_launch_placed{0}, base_launch_place_refused{0};
    unsigned long long base_launch_ground_entries{0};
    unsigned long long carrier_deck_parented{0}, carrier_deck_wire_seeded{0};
    unsigned long long carrier_deck_carry_steps{0}, carrier_deck_wire_steps{0};
    unsigned long long carrier_deck_wire_resets{0}, carrier_deck_edge_takeoffs{0};
    unsigned long long carrier_deck_stops{0};
    float carrier_deck_wire_max{0.0f};
    // 00759120 (MMothership model bind, 00759237-00759265): class+814h..+81Ch =
    // the first point of the model's ("runwaycenter", 0) Aux group; 00759590:
    // class+820h RunwayWidth, +824h RunwayLength. One entry per class.
    struct CarrierRunwayClass {
        bool ok{false};
        std::string reason;
        float center[3]{0.0f, 0.0f, 0.0f};
        float width{0.0f};
        float length{0.0f};
        // 00759167-007591C2: class+808h..+810h, the first point of the model's
        // ("liftexitpoint", 2) Aux group (packet cc9_carrier_elevator).
        bool lift_ok{false};
        float lift[3]{0.0f, 0.0f, 0.0f};
    };
    std::map<int, CarrierRunwayClass> carrier_runway_classes;
    const CarrierRunwayClass& carrier_runway_class_00759120(const GameUnitSlot& s);
    bool carrier_holder_frame_006bee40(std::size_t owner, LandingDeck& d);
    bool landing_site_key_006c09fe(std::size_t deck_index, std::size_t owner,
        const GameUnitSlot& head, bsp::LandingSiteCandidate& c);
    unsigned long long rtb_site_keys{0}, rtb_site_keys_carrier{0}, rtb_site_keys_missing{0};
    LandingDeck* landing_deck_006c0750(std::size_t deck_index, std::size_t owner);
    // The unit row a deck name / bag `HomeBase` resolves to (bsp::air_ops_owner_row:
    // the first row by name, or under kAirOpsOwnerByInstanceBound the last live one).
    std::size_t air_ops_owner_unit(const std::string& name) const {
        return bsp::air_ops_owner_row(slots.size(), name,
            [&](std::size_t u) -> const std::string* {
                return slots[u] ? &slots[u]->row.name : nullptr;
            },
            [&](std::size_t u) { return gunnery != nullptr && gunnery->unit_dead(u); });
    }
    unsigned long long landing_deck_owner_rebuilds{0};
    bool landing_request_006c54c0(GameUnitSlot& plane);
    float landing_sequencer_006cc9f0(LandingDeck& d, std::size_t squadron);
    std::vector<std::size_t> landing_members(std::size_t squadron) const;
    std::size_t landing_squadron_of(std::size_t plane) const;
    int landing_spawn_index(std::size_t plane) const;
    std::array<float, 3> landing_local_006bcc90(const LandingDeck& d, const float* pos) const;
    // Packet cc9_plane_touchdown (docs/SQUADRON_LAND_TASK.md section 5k).
    bool landing_over_runway_006bc530(const LandingDeck& d, const float* pos, float* height) const;
    std::size_t plane_landing_site_006c0840(const GameUnitSlot& p, float& dist);
    void plane_site_probe_007c5ac0(GameUnitSlot& p, float step);
    bool plane_touchdown_007cc440(GameUnitSlot& p);
    bool deliver_plane_touchdown(GameUnitSlot& p);
    std::array<float, 3> landing_t_world_006bca40(const LandingDeck& d) const;
    float landing_run_length_006ba620(const LandingDeck& d) const;
    // Packet p19_fwd_speed (docs/OCEAN_WAVE_FIELD.md section 74). True:
    // 007D99C0's deck term joins the host's forward speed ctl+44h for a plane
    // with a scene parent (unit+3Ch, here deck_parent_plus_one): the parent's
    // velocity (its vtable[34h]) in the plane's frame (0042D0D0 through the
    // inverse world +110h), z, times 0.1 (00D7A2F0) when z >= 0.0 (00D7A218) and
    // 0.8 (00CE74F8) below or unordered (COMISS / JC). False: ctl+44h alone.
    static constexpr bool kPlaneForwardSpeedDeckTermBound = true;
    float plane_forward_speed_007d99c0(const GameUnitSlot& p, float ctl_44) const;
    mutable unsigned long long forward_speed_deck_terms{0};
    float landing_t_distance(const LandingDeck& d, const GameUnitSlot& p) const;
    bool landing_outside_standby_006c46b0(const LandingDeck& d, const GameUnitSlot& p) const;
    bool landing_released_006c45c0(const LandingDeck& d, const GameUnitSlot& p) const;
    bool landing_corridor_006c3b10(const LandingDeck& d, const GameUnitSlot& p) const;
    bool landing_on_circle_006c5c40(const LandingDeck& d, const GameUnitSlot& p) const;
    // DIAGNOSTIC, packet cc9_land_head_chatter: 006C5C40's parts for a head in
    // land/standby at the leader pass of 006C7960.
    void landing_head_circle_diag(const LandingDeck& d, const GameUnitSlot& p, int old_mode,
                                  int new_mode);
    unsigned long long lhc_n{0}, lhc_on{0}, lhc_far{0}, lhc_side_moved{0}, lhc_heading{0};
    unsigned long long lhc_own_ok{0}, lhc_own_far{0}, lhc_leave{0}, lhc_trace{0};
    unsigned long long lfm_trace{0};  // cc9-physics28: land final mode-loss lines
    unsigned long long lsp_trace{0};  // cc9-physics29: land spacing-zero lines
    // cc9-physics29 (diagnostic): the OFF census of kUnitsHostDeviationsCc13Bound, the
    // calls where the ON arm would give a different moveto distance or pass-1 pitch.
    unsigned long long uhd_moveto_calls{0}, uhd_moveto_differs{0};
    unsigned long long uhd_pitch_calls{0}, uhd_pitch_differs{0}, uhd_pitch_lowspeed{0};
    double uhd_pitch_maxdiff{0.0};
    double lhc_dd_r_sum{0.0}, lhc_delta_fail_sum{0.0}, lhc_r_moved_sum{0.0};
    double lhc_point_moved_sum{0.0}, lhc_own_d_r_sum{0.0};
    bool landing_turn_in_006c5e20(const LandingDeck& d, const GameUnitSlot& p) const;
    std::array<float, 3> landing_circle_point_006c5380(const LandingDeck& d,
        const GameUnitSlot& p, bool side) const;
    float landing_radius_006c3e50(const LandingDeck& d, const GameUnitSlot& p) const;
    bool landing_deck_usable_006bed60(std::size_t deck_index) const;
    void landing_mode_006c7960(LandingDeck& d, std::size_t deck_index, LandingAssignment& rec,
        bool leader_pass, int head_mode, float head_dist);
    void landing_path_006c6020(LandingDeck& d, LandingAssignment& rec);
    void landing_spacing_006c3f80(LandingDeck& d, LandingAssignment& rec);
    // Packet cc9_land_park_taxi: the airfield launch-site calls land/park makes.
    bool landing_hangar_point_006cf420_006cf520(const LandingDeck& d, bool entry,
        std::array<float, 3>& out) const;
    bool landing_blocks_006cdf70(const GameUnitSlot& a, const GameUnitSlot& b) const;
    bool landing_queue_clear_006ce610(const LandingDeck& d, const GameUnitSlot& a) const;
    bool landing_spot_clear_006cf5b0(const LandingDeck& d, const GameUnitSlot& p, bool enable,
        float x, float z) const;
    void plane_four_to_five_007c16f0(GameUnitSlot& p);
    void plane_five_to_four_007c1680(GameUnitSlot& p);
    void log_ground_contact_loss(const GameUnitSlot& p);
    std::vector<std::unique_ptr<GameUnitSlot>> slots;
    // Packet cc9_prcp03_phase_progress: scene marker id -> authored position.
    std::map<std::uint32_t, std::array<float, 3>> scene_marker_positions;
    std::map<std::uint32_t, std::array<float, 16>> scene_marker_frames;  // cc9_spawn_new_shipyard
    // Packet cc8_ship_follow: the 508h-byte unit groups 0070DB20 allocates. A
    // unit points at one through GameUnitSlot::formation_group (unit+284h).
    struct FormationGroup {
        std::size_t leader{0};                               // group+14h
        std::vector<bsp::ShipAiUnitGroupMember> members;     // group+18h, 34h each
        // Stable actual callback prefix, also used by the existing member
        // read projections. FormationGroup itself moves as the vector grows.
        mutable std::unique_ptr<GameUnitGroupStorage> native_members{
            std::make_unique<GameUnitGroupStorage>()};
        bool observes_units{false};                         // native callback-owner edges live
        std::int32_t column{0};                              // group+500h, the
                                                             // pattern index the
                                                             // constructor leaves 0
        std::int32_t type_04fc{6};                           // group+4FCh, 6 for a
                                                             // ship leader (0070D82F)
    };
    std::vector<FormationGroup> formation_groups;
    void register_formation_observer(FormationGroup& group, std::size_t unit) {
        if (observer_runtime == nullptr || !observer_runtime->has_live_dispatch_owner()
            || !slots.at(unit)->observer_prefix_ready)
            throw std::logic_error("formation join requires the live unit observer endpoint");
        bsp::register_observer_pair_00694a60(slots[unit]->observer_prefix.observed_00,
            group.native_members->observer_owner(), observer_runtime->lifetime());
        group.observes_units = true;
    }
    void unregister_formation_observer(FormationGroup& group, std::size_t unit) {
        if (!group.observes_units) return;
        observer_runtime->lifetime().unregister_pair_006952a0(
            slots.at(unit)->observer_prefix.observed_00, group.native_members->observer_owner());
    }
    unsigned long long formation_joins{0};
    unsigned long long formation_creates{0};
    unsigned long long formation_rejoins{0};
    // Packet cc9_formation_join_follow: the recursion depth of formation_join_0077f940
    // (the follow is the top-level join's), and the follows it issued / that answered.
    int formation_join_depth{0};
    unsigned long long formation_join_follows{0};
    unsigned long long formation_join_follows_pushed{0};
    // Packet cc9_dead_member_group_removal.
    unsigned long long formation_death_leaves{0};
    unsigned long long pitch_mode_two_steps{0};   // packet cc9_torpedo_run_pitch_profile
    unsigned long long formation_death_handovers{0};
    unsigned long long formation_groups_emptied{0};
    // Packet cc13-jm08: leader handovers 0071C150 run (in leave_group, at the
    // kill, or by the posted entry), posted at a kill, and posted entries that
    // found theirs already run by a leave_group.
    unsigned long long director_handovers_run{0}, director_handovers_posted{0},
        director_handovers_skipped{0};
    // Lua leaves posted behind a pending handover (post_formation_leave_0077c980).
    unsigned long long director_lua_leaves_posted{0};
    // Whole 0077BD70 leaves run at a mapped kill or its delivered entry (ships).
    unsigned long long director_whole_leaves_at_kill{0};

    bool unit_torn_down_005d(std::size_t index) const {
        return index < slots.size() && slots[index]->state != nullptr
            && slots[index]->state->simulate != 0;
    }
    // Packet cc13-jm08 (kDeadLeaderDirectorStateCopyBound). 0077BDDE..0077BE02:
    // the leaving unit's director (vtable[114h]) gets vtable[5Ch](successor) =
    // 0071C150, which runs only while the unit still leads (00778890 at
    // 0071C156) and a successor exists (0071C163): the successor's director
    // vtable[4Ch] = 00836680 copies it (00720450, then the fire-target tail).
    // Three hosts hold that director. True when it copied.
    bool director_handover_0071c150(std::size_t index, std::size_t successor) {
        if (index >= slots.size() || successor >= slots.size()) return false;
        const std::int32_t g = slots[index]->formation_group;
        if (g < 0 || static_cast<std::size_t>(g) >= formation_groups.size()
            || formation_groups[static_cast<std::size_t>(g)].leader != index) return false;
        if (!commands.copy_director_state_00720450(successor, index)) return false;
        if (gunnery != nullptr) gunnery->copy_director_allow_bytes_00720450(successor, index);
        if (ship_ai != nullptr) ship_ai->copy_fire_target_00836680(successor, index);
        ++director_handovers_run;
        done("WeaponDirector::leader_handover_0071c150", 0x0071c150u);
        return true;
    }
    // 0070D8D0(group, exclude), read whole. A follower's exclude answers the
    // current leader; with no exclude, no leader or the leader excluded (and a
    // group that is not type 18h) it resets the column (group+500h) and answers
    // the living member (entity set, +5Dh clear, not `exclude`) with the
    // smallest axial[0] (record+20h), the first on a tie, stopping at the
    // leader's record. SIZE_MAX for none.
    std::size_t group_successor_0070d8d0(FormationGroup& fg, std::size_t exclude) {
        const std::size_t none = static_cast<std::size_t>(-1);
        const std::size_t leader = fg.leader < slots.size() ? fg.leader : none;
        if (!((exclude == none || leader == none || exclude == leader) && fg.type_04fc != 0x18)) {
            return leader;
        }
        fg.column = 0;                                        // group+500h
        std::size_t best = none;
        float best_key = 0.0f;
        for (const bsp::ShipAiUnitGroupMember& m : fg.members) {
            if (m.entity == 0) continue;
            const std::size_t u = static_cast<std::size_t>(m.entity) - 1u;
            if (unit_torn_down_005d(u) || u == exclude) continue;
            if (u == leader) break;
            if (best == none || m.axial[0] < best_key) {
                best = u;
                best_key = m.axial[0];
            }
        }
        return best;
    }
    // 00815E20(new, old): the new leader takes a copy of the old one's wake ring
    // (00815680 on old+0BD0h). +FA0h/+FA4h/+FA8h have no host field: recorded.
    void inherit_leader_wake_00815e20(std::size_t new_leader, std::size_t old_leader) {
        if (new_leader >= slots.size() || old_leader >= slots.size()) return;
        {
            bsp::ShipAiWakeTrail& destination = slots[new_leader]->wake;
            const bsp::ShipAiWakeTrail& source = slots[old_leader]->wake;
            // 00815EAA calls 00815680 on the new/old leaders' inline rings.
            // These are the authoritative samples filled at spawn and updated
            // by append, also read by the formation/follower queries. Preserve
            // the whole original head-first x87 copy, including self-copy.
            bsp::copy_native_unit_wake_00815680(&destination, nullptr, &source);
            // Preserve the existing outer handover projection for fields the
            // primitive does not touch. Full 00815E20 pose/residual rebasing
            // remains unbound; this does not claim that larger native owner.
            destination.leg_shrank = source.leg_shrank;
            for (std::size_t i = 0; i < 3; ++i) destination.residual[i] = source.residual[i];
            destination.appends = source.appends;
            destination.advances = source.advances;
            destination.merges = source.merges;
            destination.written = source.written;
            done("UnitGroup::copy_leader_wake_00815680", 0x00815680u);
        }
        done("UnitGroup::inherit_leader_wake_00815e20", 0x00815e20u);
        record("UnitGroup::leader_offset_fa0_00815e20", 0x00815e7cu);
        ++formation_death_handovers;
    }
    bool ship_unit(std::size_t u) const;
    unsigned long long formation_clamped{0};
    unsigned long long formation_columns_unmeasurable{0};
    // A flat copy of the rows, rebuilt on demand so units() can hand the caller
    // one contiguous table without exposing the slots.
    mutable std::vector<GameUnitRow> rows;
    GameUnitsSummary summary{};
    // Legacy index/presence view of 00E188D8, retained for existing readers
    // and the chain-499-off path. The publication below borrows the same slot.
    bool controlled_bound{false};
    std::size_t controlled_index{0};
    // Chain 499: the actual selected slot's stable identity, published by the
    // 004C0890 host callback before any class/listener query. Initial null is
    // the image's unbound state; clearing control publishes null again.
    void* volatile controlled_unit_00e188d8{nullptr};
    // 00424c40 is a singleton, so the curve block is one table shared by every
    // ship in the mission, not a per-unit or per-class tuning. It is filled once
    // by load_gameplay_settings_0083b5e0; until then it carries the zeroes a
    // freshly allocated settings object has, which is why the load runs first.
    bsp::UnitRudderCurveSettings rudder_curve{};
    bool rudder_curve_loaded{false};
    // Packet cc9_unit_instance_step11: settings+5BCh + i*24h + 8h.
    bsp::EngineSoundSmoothRates engine_sound_rates{};
    // Milestone 2p: the seven AutoThrust keys of the same singleton, which
    // 009EC7C0 reads at +6CCh..+6ECh. Loaded beside the turn multipliers.
    bsp::ShipAiAutoThrustSettings auto_thrust{};
    bool auto_thrust_loaded{false};

    // The world fields 00c41550 and 00c5b1b0 read: gravity at world+04h..+0Ch,
    // the two sleep speed thresholds at world+3Ch/+40h and the countdown reload
    // at world+44h.
    //
    // Milestone 2r left these at zero on the reading that none of them had a
    // recovered producer. They do: 004DDB90 builds the world descriptor on its
    // own stack and 00C41AD0 copies it into the world, and packet
    // `dyn_world_settings` read both whole (docs/DYN_WORLD_SETTINGS.md). The
    // values are the game's own: gravity (0, -10, 0) from the double at
    // 00CE6848, both sleep speeds the zero 004DE1C7 / 004DE1CD store, so no
    // body ever sleeps.
    //
    // Gravity is switched on here and not earlier because it only balances once
    // 009329C0 runs: the buoyancy the element list produces at the hull's draft
    // is exactly mass * 10, which is what cancels 00C41550's gravity * dt. The
    // two belong to the same step and neither is correct alone.
    bsp::DynWorldStepConstants physics_world{
        bsp::dyn_world_step_constants(bsp::dyn_world_settings_game())};

    // The notes are logged once per kind, not once per unit tick.
    bool logged_ocean{false};
    bool logged_ocean_live{false};       // packet cc9_physics1_sea
    bool logged_ocean_live_1000{false};
    bool logged_scale{false};
    bool logged_curve{false};
    bool logged_integrator{false};
    bool logged_cruise{false};
    bool logged_hydro{false};

    // Milestone 2s: the world registry's per-class unit lists, the 97 triples
    // at [[00E188A8]+19CCh] + 18h + id*0Ch that 004CB076's vector-constructor
    // iterator builds inside 004CB030 BSP_World_Construct. Each triple is
    // {count, head, tail};00484540 appends stable nodes holding direct unit
    // identities. The registry remains the same mission-owned object.
    // Only the ids a unit's +130h override joins are ever non-empty here.
    static constexpr int kWorldListCount = static_cast<int>(bsp::kWorldSlotCount);
    GameUnitWorldLists world_lists;

    // 00484540, __thiscall void(list, void* value), RET 4. The node is
    // {prev, next, value}; the empty branch at 00484586 writes the head and the
    // other at 00484572 chains from the tail, and both set the tail and
    // ADD dword ptr [ESI],1. Consumers can retain a node and reload its next
    // link after a callback, as the native candidate loop does.
    void world_list_push_back_00484540(GameUnitWorldLists& parent,
        std::uint32_t offset, GameUnitSlot& unit) {
        if (offset < bsp::kWorldSlotArrayOffset
            || (offset - bsp::kWorldSlotArrayOffset) % bsp::kWorldSlotStride != 0
            || (offset - bsp::kWorldSlotArrayOffset) / bsp::kWorldSlotStride
                >= bsp::kWorldSlotCount) {
            throw std::logic_error("invalid recovered world list offset");
        }
        GameUnitWorldList& list = parent.entries[
            (offset - bsp::kWorldSlotArrayOffset) / bsp::kWorldSlotStride];
        //00484546 allocates12;54..59 zero words,66 payload,6C previous;
        //75 old-tail next or86 head, then tail/next/count stores.
        auto* node = new GameUnitWorldNode{};
        node->unit = &unit;
        node->previous = list.tail;
        if (list.count != 0) list.tail->next = node;
        else list.head = node;
        list.tail = node;
        node->next = nullptr;
        ++list.count;
        ++summary.world_list_pushes;
        done("UnitList::push_back", 0x00484540u);
    }

    // Packet cc9_sunk_ship_kill_depth. The ship's vt[134h] (Battleship 006E0060,
    // Cruiser 006FB510, Destroyer 006FE670, Cargo 006EB460, Mothership
    // 00758FE0), called at 00928F2C from the on-killed base 00928C80 when
    // unit+30h is set. Each walks its lists 1, 2, 4 (006D3620 via 00928570),
    // 5, 6 (006DFFC0) and its class list, and erases (004837D0) the first node
    // whose payload is the unit: the mirror of the +130h registrar. The host
    // erases the first such node of every list, which is the same set.
    void world_lists_unlink_unit_00928f24(GameUnitSlot& unit) {
        for (GameUnitWorldList& list : world_lists.entries) {
            for (GameUnitWorldNode* node = list.head; node != nullptr; node = node->next) {
                if (node->unit != &unit) continue;
                if (node->previous != nullptr) node->previous->next = node->next;
                else list.head = node->next;
                if (node->next != nullptr) node->next->previous = node->previous;
                else list.tail = node->previous;
                --list.count;
                delete node;
                ++unit.world_nodes_unlinked;
                ++world_nodes_unlinked;
                break;
            }
        }
        done("UnitList::erase_004837d0", 0x004837d0u);
    }

    // Packet cc9_buoyancy_elements. The class's element list as 0082FE30 builds
    // it once per class after the Lua load: 00718000 finds the Aux point sets
    // "deckline" (00D09A0C, at 0082FEBA) and "bottomline" (00D09A00, at
    // 0082FECA), kind 0, in the class model descriptor+50h (the row's `Mesh`),
    // and 0082D040 (called at 0082FEE3 with Hull.WaterLineRatio +71Ch) emits
    // two records per Hull.Segments station. The original dereferences a null
    // node record; the host refuses instead and the caller keeps the stand-in.
    struct ClassBuoyancyList {
        bool ok{false};
        std::string reason;
        std::string mesh;
        float ratio{0.0f};
        int segments{0};
        std::vector<bsp::ShipBuoyancyElement> elements;
    };
    struct BuoyancyProfileHost final : bsp::ShipBuoyancyElementHost {
        const std::vector<bsp::GunFirePointItem>* items{nullptr};
        bool find_model_profile_points(const char* node_name, int kind,
            std::vector<bsp::OceanVec3>& out) override {
            const bsp::GunFirePointItem* item = bsp::find_named_point_group_00718870(
                *items, node_name, static_cast<std::uint32_t>(kind));
            if (item == nullptr) return false;
            out.clear();
            for (const auto& p : item->points) out.push_back(bsp::OceanVec3{p[0], p[1], p[2]});
            return true;
        }
    };
    const ClassBuoyancyList& class_buoyancy_list_0082fe30(const GameUnitSlot& s) {
        const int type_id = s.row.type_id;
        auto found = class_buoyancy_lists.find(type_id);
        if (found != class_buoyancy_lists.end()) return found->second;
        ClassBuoyancyList& entry = class_buoyancy_lists[type_id];
        entry.mesh = lua.read_vehicle_class_string(type_id, "Mesh");
        entry.ratio = lua.read_vehicle_class_nested_number(type_id, "Hull", "WaterLineRatio",
            0.0f);
        entry.segments = lua.read_vehicle_class_integer(type_id, "Hull", "Segments", 0);
        std::vector<std::uint8_t> bytes;
        std::vector<bsp::GunFirePointItem> items;
        std::string error;
        if (entry.mesh.empty()) {
            entry.reason = "no Mesh string";
        } else if (!lua.read_resource_file(entry.mesh, bytes)) {
            entry.reason = "model did not open";
        } else if (!bsp::read_mmod_aux_point_items_0071b3e0(bytes, items, error)
                   && items.empty()) {
            entry.reason = "model Aux read failed: " + error;
        } else {
            BuoyancyProfileHost profile;
            profile.items = &items;
            bsp::ShipBuoyancyHullInputs hull{};
            hull.length = s.motion_class.hull_length;   // descriptor+A0h
            hull.width = s.class_width_00a4;            // descriptor+A4h
            hull.mass = s.motion_class.hull_mass;       // descriptor+B0h
            hull.hull_water_line_ratio = entry.ratio;   // descriptor+71Ch
            hull.hull_segments = entry.segments;        // descriptor+720h
            if (!bsp::ship_buoyancy_build_from_model_0082fea9(profile, hull, entry.elements)) {
                entry.reason = "model lacks deckline or bottomline";
            } else if (entry.elements.empty()) {
                entry.reason = "Hull.Segments not positive";
            } else {
                entry.ok = true;
                entry.reason = "image list";
            }
        }
        float draught = 0.0f;  // 008936A0 luaMW_GetDraught: max(+14h)
        float coef_draught = 0.0f;
        for (const bsp::ShipBuoyancyElement& e : entry.elements) {
            draught = std::max(draught, e.unread_14);
            coef_draught += e.coefficient * e.unread_14;
        }
        log.notef("buoyancy elements class=%d \"%s\" mesh=%s ratio=%.4f segments=%d "
            "status=%s records=%zu max_draught=%.3f sum_coef_draught=%.1f (10*Mass=%.1f) "
            "(0082D040 at 0082FEE3, packet cc9_buoyancy_elements, used=%d)",
            type_id, s.row.name.c_str(), entry.mesh.c_str(), static_cast<double>(entry.ratio),
            entry.segments, entry.reason.c_str(), entry.elements.size(),
            static_cast<double>(draught), static_cast<double>(coef_draught),
            static_cast<double>(s.motion_class.hull_mass * 10.0f),
            kShipBuoyancyElementsBound && entry.ok ? 1 : 0);
        return entry;
    }

    // Packet cc9_ship_sink_descent. controller+84h (00937DBB..00937F74) and the
    // leak manager's init 0074F490 (called from 00823780 in 00822C20).
    void build_leak_model_0074f490(GameUnitSlot& s) {
        // 0081F15F constructs unit+10D4h before 0074F490 initializes its
        // zones. Use that body's binary defaults for the live model rather
        // than independently supplying DamageToDeath/TimeToDeath below.
        // This is its typed construction projection, not a raw unit overlay.
        bsp::UnitLeakManagerConstruction constructed;
        bsp::construct_unit_leak_manager_0074e7b0(constructed);
        // 00937D8E..00937DB8: Mass < 100.0 (00D7A220) -> 1, else the unit's
        // IsKindOf(8) -> 2, else 0.
        const float mass = s.motion_class.hull_mass;             // class+B0h
        const bsp::ShipPhysicsMaterial index =
            (static_cast<double>(mass) < 100.0) ? bsp::ShipPhysicsMaterial::kTBoat
            : bsp::unit_is_kind_of(s.class_id, 8) ? bsp::ShipPhysicsMaterial::kSubmarine
            : bsp::ShipPhysicsMaterial::kShip;
        const bsp::ShipPhysicsMaterialRecord rec = bsp::ship_physics_material_shipped(index);
        // Per record: frac = |(+0Ch - +08h) / (+04h - +0Ch)| (00937E41..00937E80),
        // shaped by settings+50Ch (the Kitevo ladder, previous value kept),
        // times |+04h - +0Ch| (00937EDC..00937EFD), times +00h and settings
        // +510h, over the double 10.0 (00937F06..00937F1F), each stored float.
        float sum = 0.0f;
        float shaped = 0.0f;  // [ESP+10h], carried between records
        for (const bsp::ShipBuoyancyElement& e : s.buoyancy_elements) {
            const float frac = std::fabs(
                (e.level_base - e.level_draft) / (e.level_top - e.level_base));
            shaped = bsp::ship_hydro_depth_factor_00932fb5(frac, rec.depth_exponent, shaped);
            shaped = std::fabs(e.level_top - e.level_base) * shaped;
            float term;
            if constexpr (bsp::kPc24ShipAiCc13Bound) {
                // pc24w1-sweep2-3: 00937F0B FMUL, 00937F12 FMUL settings+510h, 00937F19
                // FDIV 10.0, each rounded at PC24.
                term = static_cast<float>(bsp::cc13x87::x87_div24(bsp::cc13x87::x87_mul24(
                    bsp::cc13x87::x87_mul24(e.coefficient, shaped), rec.gravity), 10.0));
            } else {
                term = static_cast<float>(static_cast<double>(e.coefficient)
                    * static_cast<double>(shaped) * static_cast<double>(rec.gravity) / 10.0);
            }
            sum = term + sum;
        }
        // 00937F46..00937F74: controller+84h = sum - settings+510h * Mass.
        s.controller_reserve_84 = sum - rec.gravity * mass;
        // 0074F490: count 6 (+14h); capacity +0Ch = (float)(0092BEB0() / 10.0).
        s.leak_capacity_0c = static_cast<float>(
            static_cast<double>(s.controller_reserve_84) / 10.0);
        // 0074F51B..0074F56D: IsKindOf(0Eh), or IsKindOf(0Ch) with Mass below
        // 500.0 (00CE3840), triples the rate cap (the double 3.0 at 00D7A2B0).
        float cap = kMaxLeakPercentSubstitute * s.leak_capacity_0c;
        if (bsp::unit_is_kind_of(s.class_id, 0x0E) ||
            (bsp::unit_is_kind_of(s.class_id, 0x0C) && static_cast<double>(mass) < 500.0)) {
            cap = static_cast<float>(static_cast<double>(cap) * 3.0);
        }
        s.leak_rate_cap_08 = cap;
        // 0074F490 performs the same two memsets as 0074E830, water first
        // then rates. Reuse the existing body for those initialization stores.
        // The host interleaves the manager's three arrays in UnitLeakEntry;
        // project the actual six zones and publish both cleared arrays back.
        bsp::gameplay_seg41_cc13::LeakManagerZones zones;
        zones.water_1c.reserve(s.leaks.size());
        zones.leak_18.reserve(s.leaks.size());
        for (const auto& leak : s.leaks) {
            zones.water_1c.push_back(leak.water);
            zones.leak_18.push_back(leak.rate);
        }
        bsp::gameplay_seg41_cc13::clear_all_zones_0074e830(zones);
        for (std::size_t i = 0; i < s.leaks.size(); ++i) {
            s.leaks[i].water = zones.water_1c[i];
            s.leaks[i].rate = zones.leak_18[i];
        }
        // The six points: side -1 then +1 (local_8), three stations each at
        // z = (L / 2) * i - L * 0.5, x = Width * side, y = 0.
        const float length = s.motion_class.hull_length;       // class+A0h
        const float width = s.class_width_00a4;                 // class+A4h
        std::size_t k = 0;
        for (int side = -1; side < 2; side += 2) {
            for (int i = 0; i < 3; ++i) {
                bsp::UnitLeakEntry& leak = s.leaks[k++];
                leak.point = bsp::OceanVec3{width * static_cast<float>(side), 0.0f,
                    (length / 2.0f) * static_cast<float>(i) - length * 0.5f};
            }
        }
        // 0074F6A4..0074F6D5: +38h/+3Ch take class DamageToDeath (+54Ch) and
        // TimeToDeath (+550h) when both are >= 0; else the constructor's
        // 200.0 and 60.0 (0074E7B0). +34h = 2 * settings+404h * capacity / +3Ch,
        // and the cap is raised to it.
        float damage_to_death = constructed.tuning_38;
        float time_to_death = constructed.tuning_3c;
        const float d = lua.read_vehicle_class_number(s.row.type_id, "DamageToDeath", -1.0f);
        const float t = lua.read_vehicle_class_number(s.row.type_id, "TimeToDeath", -1.0f);
        if (0.0f <= d && 0.0f <= t) {
            damage_to_death = d;
            time_to_death = t;
        }
        s.leak_damage_to_death_38 = damage_to_death;  // +38h, read by 0074F090
        s.leak_station_04 = static_cast<float>(
            static_cast<double>(length) / 3.0);         // +04h, 0074F4F0..0074F4FC
        const float r34 = kEnnyiVizEsKeszPercentSubstitute * s.leak_capacity_0c;
        s.leak_rate_34 = (r34 + r34) / time_to_death;
        if (s.leak_rate_cap_08 < s.leak_rate_34) s.leak_rate_cap_08 = s.leak_rate_34;
        s.leak_ready = true;
        ++leak_models_built;
        done("LeakManager::init_0074f490", 0x0074f490u);
    }

    // Packet cc13-leak92: the 92h messages 0074EC50 routed and the pump has not
    // delivered yet, in post order. Filled only with the switch ON.
    struct PendingShipSink92h {
        GameUnitSlot* slot{nullptr};
        std::uint32_t count{0};          // msg+1Ch, the copy's count
        std::array<float, 6> rates{};    // msg+20h
    };
    std::vector<PendingShipSink92h> pending_ship_sink_92h;
    // 0076C600 -> 0082220D -> 0074E860 for each: the message's rates overwrite
    // the manager's (+18h), so a leak 0074F440 added since the post is lost.
    // bsp.py show 00821e80 --asm: 0082220D MOV EDX,[ESI+1Ch]; 00822210 LEA
    // ECX,[ESI+20h]; PUSH ECX; PUSH EDX; LEA ECX,[EDI+10D4h]; CALL 0074E860.
    // The slot's `leaks` are the manager's parallel arrays as one struct per
    // zone, so the +18h array is projected into the body's view and back.
    std::size_t deliver_ship_sink_92h_0082220d() {
        if (pending_ship_sink_92h.empty()) return 0;
        std::vector<PendingShipSink92h> batch;
        batch.swap(pending_ship_sink_92h);
        for (const PendingShipSink92h& m : batch) {
            bsp::gameplay_seg41_cc13::LeakManagerZones zones;
            zones.leak_18.resize(m.slot->leaks.size());
            for (std::size_t i = 0; i < m.slot->leaks.size(); ++i)
                zones.leak_18[i] = m.slot->leaks[i].rate;
            if (!bsp::gameplay_seg41_cc13::load_zone_array_0074e860(
                    zones, static_cast<std::int32_t>(m.count), m.rates.data())) {
                // The image has no bound; a count past +14h would write past
                // the manager's arrays. queue_ship_sink_92 posts the zone count.
                throw std::logic_error("92h leak-rate count exceeds the leak manager's zones");
            }
            for (std::size_t i = 0; i < m.slot->leaks.size(); ++i)
                m.slot->leaks[i].rate = zones.leak_18[i];
            done("LeakManager::load_zone_array_0074e860", 0x0074e860u);
            done("ShipMessage::sink_92h_0082220d", 0x0082220du);
        }
        return batch.size();
    }

    // 00824FE5..00825086, the wreck handler's sink block, for one ship.
    void ship_wreck_sink_block_00824fe5(GameUnitSlot& s) {
        if (!s.leak_ready || s.leak_redistributed) return;
        s.leak_redistributed = true;
        // 0074EC50: with lobby mode [00E188A8]+1FE4h != 2 (single player), each
        // leak draws U(0, 1) from 00BD2F10 (ECX = 1). SUBSTITUTION, labelled: the
        // draws come from this host's keyed release stream, one key per ship.
        // Packet cc13-sinkrng: with kSinkLeakDrawStreamCc13Bound they come from the
        // gunnery generator, the host's main-thread stream 1 (0074ECA5, row 15).
        float draws[6]{};
        float sum_draws = 0.0f;
        for (std::size_t i = 0; i < s.leaks.size(); ++i) {
            float r = 0.0f;
            if constexpr (bsp::kSinkLeakDrawStreamCc13Bound) {
                r = gunnery != nullptr
                    ? gunnery->sink_leak_draw_0074eca5(s.process_index, 0.0f, 1.0f)
                    : release_altitude_draw_00bd2f10(s.row.name + "#leak", 0.0f, 1.0f);
            } else {
                r = release_altitude_draw_00bd2f10(s.row.name + "#leak", 0.0f, 1.0f);
            }
            sum_draws = r + sum_draws;                    // 0074ECBD
            draws[i] = r + s.leaks[i].water;              // 0074ECC5
        }
        // 0074ECD6..0074ED18: rate' = draw * (cap + cap) / (total_water + sum) + rate.
        float total_water = 0.0f;
        for (const bsp::UnitLeakEntry& leak : s.leaks) total_water += leak.water;
        if constexpr (bsp::kLeakMessageRouterDeliveryCc13Bound) {
            // Packet cc13-leak92: the rates go into the 92h message (0074ED21
            // 0075B430(92h)), not the manager, and 0074ED7B routes it. The pump
            // delivers it (units_deliver_ship_sink_92h_0082220d).
            PendingShipSink92h message{&s, {}};
            for (std::size_t i = 0; i < s.leaks.size(); ++i) {
                const float twice = s.leak_rate_cap_08 + s.leak_rate_cap_08;
                message.rates[i] = (draws[i] * twice) / (total_water + sum_draws)
                    + s.leaks[i].rate;
            }
            {
                // Goal 4 chain 343: 0074ED18..0074ED62 (0074EAB0's stores) and
                // 0074ED7B (flags 7) queue 0076E520's copy (0074EB20 write,
                // 00768530 case 92h 00762130, 0074EB80 read); 00780120 asks
                // the copy's 0074EB00, and the pump's 0082220D hands the copy's
                // count and rates to 0074E860.
                const bsp::HostSessionMessages::ShipSinkCopy copy =
                    commands.session_messages().queue_ship_sink_92(message.rates.data(),
                        static_cast<std::uint32_t>(s.leaks.size()));
                if (copy.sink_92) {
                    for (std::size_t i = 0; i < message.rates.size(); ++i)
                        message.rates[i] = i < copy.count ? copy.rates[i] : 0.0f;
                    message.count = copy.count;
                    pending_ship_sink_92h.push_back(message);
                }
            }
            done("LeakManager::route_ship_sink_92h", 0x0077c2a0u);
        } else {
            for (std::size_t i = 0; i < s.leaks.size(); ++i) {
                const float twice = s.leak_rate_cap_08 + s.leak_rate_cap_08;
                s.leaks[i].rate = (draws[i] * twice) / (total_water + sum_draws) + s.leaks[i].rate;
            }
            // The rates travel in message 92h through 0077C2A0 (route 7, 0074ED7B);
            // its arm 0082220D -> 0074E860 loads them. SUBSTITUTION, labelled: applied
            // at once, not through the session router (kLeakMessageRouterDeliveryCc13Bound).
            record("LeakManager::route_ship_sink_92h", 0x0077c2a0u);
        }
        done("LeakManager::redistribute_0074ec50", 0x0074ec50u);
        // 00824FF0..00825074 on the hull body: inertia x2 (00D7A308), angular
        // damping 2.5f (00CF87C8), linear damping 0.5f (00CE3800).
        s.body.motion = &s.motion_state;
        const bsp::OceanVec3 inv = s.motion_state.inverse_inertia_body;
        const bsp::OceanVec3 twice{
            inv.x == 0.0f ? 0.0f : static_cast<float>(2.0 * static_cast<double>(1.0f / inv.x)),
            inv.y == 0.0f ? 0.0f : static_cast<float>(2.0 * static_cast<double>(1.0f / inv.y)),
            inv.z == 0.0f ? 0.0f : static_cast<float>(2.0 * static_cast<double>(1.0f / inv.z))};
        bsp::dyn_body_set_inertia_00c37e70(s.body, twice);
        bsp::dyn_body_set_angular_damping_00c37de0(s.body, 2.5f);
        bsp::dyn_body_set_linear_damping_00c37e00(s.body, 0.5f);
        // 0082507E / 00825086.
        s.sink_time_828 = 0.0f;
        s.sink_under_82c = 0.0f;
        s.sunk_terrain_mask_cleared = false;
        ++leak_redistributions;
        done("UnitInstance::wreck_sink_block_00824fe5", 0x00824fe5u);
    }

    //009288F1 dispatches the actual leaf+130h registrar after placement.
    // All21 recovered creators first join parent list1 through00928560;
    // their remaining lists differ. Reload the actual parent at every push.
    void register_in_world_lists(GameUnitSlot& unit) {
        const auto* dispatch = bsp::unit_world_registration_for_creator(
            unit.motion_dispatch.creator);
        if (dispatch == nullptr) {
            ++summary.world_registration_unavailable;
            record_slot("UnitInstance::unresolved_world_registration", "unit/vtable+130h");
            return;
        }
        class Calls final : public bsp::UnitWorldRegistrationHost {
        public:
            explicit Calls(Impl& owner) : owner_(owner) {}
            void* parent_0030(void* value) override {
                return static_cast<GameUnitSlot*>(value)->world_parent_0030;
            }
            void push_back_00484540(void* parent, std::uint32_t offset,
                void* value) override {
                if (parent == nullptr) throw std::logic_error("unplaced unit registration");
                owner_.world_list_push_back_00484540(
                    *static_cast<GameUnitWorldLists*>(parent), offset,
                    *static_cast<GameUnitSlot*>(value));
            }
        private:
            Impl& owner_;
        } calls(*this);
        if (bsp::register_unit_world_lists_for_creator(calls,
                unit.motion_dispatch.creator, &unit)) {
            ++summary.world_registrations;
            done("GameEntity::register_in_parent_entity_list", 0x00928560u);
            // Keep the actual registrar in the method key: logging otherwise
            // groups all derived routines under the first routine's address.
            char name[72];
            std::snprintf(name, sizeof(name), "UnitInstance::world_registration_%08lx",
                static_cast<unsigned long>(dispatch->native_entry));
            done(name, dispatch->native_entry);
            log.notef("unit world registration: unit=%s creator=%08lx primary=%08lx entry=%08lx lists=%zu",
                unit.row.name.c_str(), static_cast<unsigned long>(dispatch->creator),
                static_cast<unsigned long>(dispatch->primary_vtable),
                static_cast<unsigned long>(dispatch->native_entry), dispatch->list_count);
        }
    }
    bool logged_gate{false};
    bool logged_precision{false};

    // Milestone 2n: the ship AI controller that publishes into each unit's own
    // 84-byte AI order slot, which the motion head at 00825f2c promotes.
    GameShipAiHost* ship_ai{nullptr};

    // Packet cc13-para2 (bsp::kParatrooperRoundBound): the dropped MParatrooper
    // rounds, SoldierGlobals (0084CB70) and the Paratrooper class rows by bullet id.
    bsp::cc13_paratrooper::RoundWorld para_world;
    bsp::soldier_traffic_cc13::SoldierGlobals para_globals;
    bool para_globals_tried{false};
    std::vector<std::pair<int, bsp::gameplay_seg46_cc13::ParatrooperClassFields>> para_class_rows;
    bool para_soldier_globals_0084cb70();
    const bsp::gameplay_seg46_cc13::ParatrooperClassFields& para_class_fields(int bullet_id);
    bsp::cc13_paratrooper::ChuteLengths para_chute_lengths(int bullet_id);
    void para_round_step(float step_seconds);

    // Milestone 2t: the gun chain, one object per run. It owns the gunnery pass
    // at unit+6DCh of every created unit, the guns the authored `Platforms`
    // table produces, the projectiles in flight and the hit path behind them.
    std::unique_ptr<GameGunneryHost> gunnery;
    // docs/AI_COORDINATOR_TICK.md. 00A32350 BSP_AiController_Create is called
    // from BSP_Game_LoadMissionScene at 004E1838, and its 00A31730 constructor
    // registers the tick element into fixed-step group 0 at 00A31766.
    std::unique_ptr<GameAiCoordinatorHost> ai;

    void record(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.unimplemented(method, text);
    }
    void record_motion_phase(const char* phase, std::uint32_t entry) {
        // GameHostLog aggregates by method name, so retain each native entry
        // in that key as well as in the evidence-address column.
        char method[96];
        std::snprintf(method, sizeof(method), "UnitMotion::%s_%08lx", phase,
            static_cast<unsigned long>(entry));
        record(method, entry);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.implemented(method, text);
    }
    // Packet cc13-reltimer. 007CD9A0 as 007CDC70 publishes it at unit+9C3h: the
    // first rack whose platform authors UseBayDoor answers vtable[210h](2Ah, 0);
    // none answers 0. Live, not one step late; host rack order.
    // docs/CC13_RELEASE_TIMER.md.
    bool bay_rack_holds_round_9c3(GameUnitSlot& s) {
        const std::vector<PlaneOrdnanceRack>& racks = ordnance_racks(s);
        const std::vector<bool> bay =
            bsp::cc13_release_timer::bomb_rack_bay_doors(lua, s.row.type_id);
        for (std::size_t i = 0; i < racks.size() && i < bay.size(); ++i) {
            if (bay[i]) return plane_rack_holds_round(s, racks[i]);
        }
        return false;
    }
    // Packet cc13-reltimer2, kReleaseBayOpenCc13Bound (docs/CC13_RELEASE_TIMER.md
    // section 10): the image's bay opener, run once per fixed step before the
    // block step. 0099AFCC-0099B055 (the pilot think's unit+DF0h) then
    // 007C6A8D-007C6B2E (the plane tick's channel C target). SUBSTITUTIONS,
    // labelled: DF0h is refreshed every fixed step, not on the pilot think's
    // own timer; 9C1h/9C3h are read live, not one step late; the task's
    // vtable[+50h] is modelled for the torpedo (009D49E0) and dive-bomb
    // (009C8290) tasks only, and a plane with neither task is left alone.
    unsigned long long bay_open_target_writes{0}, bay_open_steps{0};
    unsigned long long lb_bay_opener_steps{0}, lb_bay_opener_mode2{0};   // packet cc13-lbbay
    unsigned long long lb_bay_opener_shadowed{0};   // level task live under a torpedo/dive task
    void bay_open_step_cc13(GameUnitSlot& s, std::size_t index) {
        namespace rt = bsp::cc13_release_timer;
        if (!bsp::unit_is_kind_of(s.class_id, 0x0F)) return;
        int mode = 1;
        if constexpr (bsp::kLevelBombBayOpenerCc13Bound) {
            // The host's torpedo -> dive -> level order stands for the image's
            // first task in bot+58h/+5Ch whose +50h is not 1; count the overlap.
            if (s.level_bomb_task_installed
                && ((s.torpedo_task_installed && s.torpedo_state != bsp::TorpedoState::kNone)
                    || (s.dive_bomb_task_installed
                        && s.dive_bomb_state != bsp::DiveBombState::kNone))) {
                ++lb_bay_opener_shadowed;
            }
        }
        if (s.torpedo_task_installed && s.torpedo_state != bsp::TorpedoState::kNone) {
            const bsp::TorpedoState t = s.torpedo_state;
            const bool listed = t == bsp::TorpedoState::kAim || t == bsp::TorpedoState::kDone
                || t == bsp::TorpedoState::kGoAway || t == bsp::TorpedoState::kAttackRun
                || t == bsp::TorpedoState::kPrepare;
            mode = bsp::band0099::torpedo_step_result_009d49e0(
                listed, s.torpedo_approach.aim_solution_130);
        } else if (s.dive_bomb_task_installed
                   && s.dive_bomb_state != bsp::DiveBombState::kNone) {
            const bsp::DiveBombState d = s.dive_bomb_state;
            const bool attacking = d == bsp::DiveBombState::kAimDive
                || d == bsp::DiveBombState::kAimGlide || d == bsp::DiveBombState::kDone
                || d == bsp::DiveBombState::kFlyAbove || d == bsp::DiveBombState::kGoAway
                || d == bsp::DiveBombState::kPrepare || d == bsp::DiveBombState::kAttackRun
                || d == bsp::DiveBombState::kTurnDown;               // 009C7910
            mode = bsp::band0099::dive_bomb_step_result_009c8290(
                attacking, d == bsp::DiveBombState::kAimDive);
        } else {
            if constexpr (bsp::kLevelBombBayOpenerCc13Bound) {
                // 009B8450 (packet cc13-lbbay): the level-bomb task's vtable[+50h],
                // close_cc = task+4C4h (approach+CCh, "bayDoorState"). A task with
                // no state answers 1 (009B8496), and 007C6A8D still reads C20h.
                if (!s.level_bomb_task_installed) return;
                const bsp::LevelBombState l = s.level_bomb_state;
                const bool attacking = l == bsp::LevelBombState::kAttackRun
                    || l == bsp::LevelBombState::kAim || l == bsp::LevelBombState::kPrepare
                    || l == bsp::LevelBombState::kRelease
                    || l == bsp::LevelBombState::kGoAway;            // 009B845C-009B8486
                mode = bsp::level_bomb_step_result_009b8450(attacking, s.lb_ap.close_cc);
                ++lb_bay_opener_steps;
                if (mode == 2) ++lb_bay_opener_mode2;
            } else {
                return;
            }
        }
        ++bay_open_steps;
        const bool own_9c3 = bay_rack_holds_round_9c3(s);
        // 0099B01C-0099B039: [unit+9D4h]+3D0h, the squadron's first member.
        // With kSquadronRemovesDeadBound the death path clears a dead leader's
        // seat to kPlaneSquadronNoUnit before 007F3970's erase moves the next
        // wingman up; the image keeps the old pointer in that window, so the
        // first occupied seat (flight_leader()) stands for +3D0h there.
        bool has_leader = false, leader_9c1 = false;
        if (mode == 1) {
            if (const bsp::PlaneSquadronHostRecord* r =
                    bsp::plane_squadron_registry().find_by_member_unit(index)) {
                const std::size_t m = kSquadronRemovesDeadBound ? r->flight_leader()
                    : r->member_units.empty() ? bsp::kPlaneSquadronNoUnit
                                              : r->member_units[0];
                if (m != bsp::kPlaneSquadronNoUnit && m < slots.size() && slots[m]
                    && m != index) {
                    const bsp::PlaneActuatorChannel& lc =
                        slots[m]->actuator_block_dec.channel_c;
                    has_leader = true;
                    leader_9c1 = rt::step_byte_9c1_007cdc84(lc.enabled, lc.value);
                }
            }
        }
        {
            // 007BB150 (0099B055): the typed body stores flag ? +9C3h : 0 at +DF0h, fed
            // the 0099B003-0099B04D flag (own_9c3 = true leaves the fused helper's
            // flag whole) and this unit's 9C3h; OFF keeps the fused helper.
            const std::uint8_t flag_0099b04d =
                rt::pilot_step_flag_df0_0099b055(mode, has_leader, leader_9c1, true);
            s.pilot_step_flag_df0 = bsp::cc13_seg47::plane_publish_slot_byte_df0_007bb150(
                flag_0099b04d, [own_9c3] { return static_cast<std::uint8_t>(own_9c3 ? 1 : 0); });
        }
        const bool before = s.actuator_block_dec.channel_c.target;
        const bsp::OrdnanceKindSet set{s.ordnance_mask};
        if (rt::plane_bay_target_007c6a8d(s.actuator_block_dec, s.generic_suppress_520,
                bsp::ordnance_has_drop_kamikaze_2fh(set), s.pilot_step_flag_df0 != 0,
                s.torpedo_release_pending_c25, s.torpedo_issue_requests_c20 != 0,
                own_9c3)) {
            ++bay_open_target_writes;
            log.notef("bay target: unit=%s t=%.2f target=%d->%d mode=%d df0=%d c25=%d "
                "c20=%d (0099B055, 007C6A8D)", s.row.name.c_str(),
                static_cast<double>(summary.simulated_seconds), before ? 1 : 0,
                s.actuator_block_dec.channel_c.target ? 1 : 0, mode,
                s.pilot_step_flag_df0, s.torpedo_release_pending_c25 ? 1 : 0,
                s.torpedo_issue_requests_c20);
        }
        done("Plane::bay_target_007c6a8d", 0x007c6a8du);
    }
    // An indirect dispatch has no call-site address of its own; the slot is the
    // evidence, so the report carries `<vtable>+vtableNN` for it.
    // 007BBBA0 BSP_Unit_RequestOrdnanceRelease. Two call sites in this host
    // reach it - the torpedo task's manual passthrough and the aim state's
    // release timer - and both must raise unit+C20h, because 007BBC00 does so
    // on every path of that routine. docs/TORPEDO_ISSUE_TIMING.md,
    // docs/TORPEDO_FIRST_RELEASE.md.
    void release_ordnance_007bbba0(GameUnitSlot& slot) {
        // 007BBBA0 itself: the three guards and the writes onto channel C of the
        // block at unit+DECh. It is a bay-open command, not a spawn; nothing in
        // the block or its tick 007DE3A0 reads ordnance or makes a projectile.
        // docs/TORPEDO_RELEASE_SPAWN.md.
        //
        // The third guard, the per-slot byte at unit+9C3h[[00F876B8]*8], is the
        // caller's because it lives on the unit rather than the block. This host
        // has no such byte, so it is NOT applied and the refusal count below
        // therefore under-counts what the image would refuse.
        if (!slot.actuator_block_dec.channel_c.enabled) {
            ++bay_channel_disabled_requests;
            if constexpr (!kBayRequestNoForceEnableBound) {
                slot.actuator_block_dec.channel_c.enabled = true;
                slot.actuator_block_dec.channel_c.rate = 1.0f;
            }
        }
        const bool accepted =
            bsp::ordnance_release_request_007bbba0(slot.actuator_block_dec);
        done("Unit::request_ordnance_release_007bbba0", 0x007bbba0u);
        if (accepted) {
            ++slot.torpedo_bay_requests_accepted;
            // SUBSTITUTION, labelled. The native chain from here is
            // 007BBBA0 -> unit+C20h -> 007CE040's stage at 007CEA82 -> 007C0D90
            // -> BSP_PilotControl_IssueReleaseOrders 007EEF30 ->
            // BSP_Unit_SetQueuedReleaseOrders 007BCBE0 -> unit+C58h, which
            // BSP_PilotBot_Tick spends one of at 0099AFB6 after a bot task's
            // vtable[24h] answers. That task's release slot, and the bomb
            // platform's own release in vtable 00CFE308, are UNREAD, so the
            // spawn is not reproduced from its own site. What IS established is
            // that a bomb platform is a Gun subclass (00730B80 calls
            // BSP_Gun_Construct at 00730B88), so the round is handed to the gun
            // spawn 0072F830 the gunnery host already implements.
            if constexpr (kReleaseIssueStageBound) {
                // The spawn waits for the rack (run_rack_tick_006e56f0).
                ++slot.rack_requests_deferred;
                record("Plane::release_spawn_deferred_to_rack", 0x006e4d50u);
            } else if (gunnery != nullptr) {
                const std::size_t index = index_of_slot(slot);
                // Packet cc9_plane_death_modes: a dead aircraft releases
                // nothing. 007D1314 sets unit+C3Ah when the death message is
                // handled, and the release stage refuses at 007CEA1C (C3Ah) and
                // 007CEA29 (+5Dh). This spawn bypasses that stage, so the
                // refusal is applied here, on the gunnery host's death.
                if (kPlaneDeathModesBound && index < slots.size()
                    && gunnery->unit_dead(index)) {
                    ++summary.dead_releases_refused;
                    log.notef("dead release refused: unit=%s torpedo (007CEA1C / "
                        "007CEA29, packet cc9_plane_death_modes)", slot.row.name.c_str());
                    done("Plane::issue_block_c3a", 0x007cea1cu);
                } else if (index < slots.size() && gunnery->release_ordnance_drop(index)) {
                    ++slot.torpedo_drops_spawned;
                }
            }
        } else {
            ++slot.torpedo_bay_requests_refused;
        }
        // The release instant, which is the sample the water-entry arithmetic
        // is built on. Printed for the first four releases only, like the
        // gunnery host's own drop line.
        if (slot.torpedo_releases < 4) {
            float vn[3];
            velocity_versus_nose(slot, vn);
            log.notef("release census: unit=%s alt=%.1f m |v|=%.2f m/s "
                "angle_to_nose=%.1f deg body_fwd_0092d730=%.2f m/s "
                "travel_spd=%.2f x_drag=%.2f y_drag=%.2f max_spd=%.2f",
                slot.row.name.c_str(),
                static_cast<double>(slot.motion.position[1]),
                static_cast<double>(vn[0]),
                static_cast<double>(vn[1]) * 180.0 / kPi,
                static_cast<double>(vn[2]),
                static_cast<double>(slot.plane_travel_speed),
                static_cast<double>(slot.plane_x_drag),
                static_cast<double>(slot.plane_y_drag),
                static_cast<double>(slot.plane_max_spd));
        }
        ++slot.torpedo_releases;
        if (gunnery != nullptr) {
            const std::size_t dead_index = index_of_slot(slot);
            if (dead_index < slots.size() && gunnery->unit_dead(dead_index)) {
                ++slot.torpedo_releases_dead;
            }
        }
        ++slot.torpedo_release_requests_007bbba0;
        slot.torpedo_issue_requests_c20 =
            bsp::release_request_raise_007bbc00(slot.torpedo_issue_requests_c20);
    }

    // ---- the dive-bomb task's host side. docs/DIVE_BOMB_TASK.md ----
    // docs/GAME_TUNING_SINGLETON.md rows +4C0h..+4D8h.
    static constexpr float kPilotDiveBombCruisingAlt = 1300.0f;
    static constexpr float kPilotDiveBombAttackDist = 1100.0f;
    static constexpr float kPilotDiveBombBeginAltRange1 = 1000.0f;
    static constexpr float kPilotDiveBombBeginAltRange2 = 1200.0f;
    // scripts/datatables/robots.lua, the SPNormal row, the fields
    // include/bsp/robot_config.hpp names from the same Lua keys.
    // docs/GAME_TUNING_SINGLETON.md +4D8h, `Pilot/DiveBomb/ReferenceSpeed`,
    // authored KMH(280) and read by 009C3EA0 - the divisor of task+41Ch's
    // max(1.0, MaxSpd / ReferenceSpeed). Logged, not yet bound.
    static constexpr float kPilotDiveBombReferenceSpeed = 280.0f / 3.6f;
    static constexpr float kDiveBombReleaseAlt1 = 350.0f;   // row+38h
    static constexpr float kDiveBombReleaseAlt2 = 450.0f;   // row+3Ch
    // These four are (approach+14h)->+5Ch, +60h, +64h and +68h. 009F9D1E sets
    // approach+14h to `00F8A30C + index * 248h + 0Ch`, so the record is a
    // 0x248-stride robots row viewed 0xCh in: ->+5Ch is row+68h, ->+60h is
    // row+6Ch, ->+64h is row+70h and ->+68h is row+74h. include/bsp/
    // robot_config.hpp names those four dive_bomb_aim_prec_dist_068,
    // _mul_06c, _pull_plus_070 and _pull_minus_074, and this installation's
    // scripts/datatables/robots.lua SPNormal row authors them. So the first
    // two are no longer an assumption and the last two are recovered.
    static constexpr float kDiveBombAimPrecDist = 70.0f;    // row+68h, ->+5Ch
    static constexpr float kDiveBombAimPrecMul = 0.3f;      // row+6Ch, ->+60h
    // "tavolsagtol fuggoen mennyire huzza a pitch-t, ha nem pontos a celzas" -
    // how much it pulls the pitch when the aim is off - and its push twin.
    static constexpr float kDiveBombAimPrecPullPlus = 0.018f;   // row+70h, ->+64h
    static constexpr float kDiveBombAimPrecPullMinus = 0.025f;  // row+74h, ->+68h
    // row+4Ch, ->+40h. "ha nem leboritott manoverrel bombaz, csak siman
    // rarepulve, akkor a fenti ReleaseAlt erteket ennyivel megszorozva
    // hasznalja" - bombing without the wingover, it uses ReleaseAlt times this.
    static constexpr float kDiveBombNewReleaseMul = 0.6f;       // row+4Ch, ->+40h

    // Packet cc9_difficulty. The six PilotBot rows the approach can capture,
    // the dive-bomb fields only, named as include/bsp/robot_config.hpp's
    // PilotBotParameters names them. SUBSTITUTION, labelled: the rows are
    // PilotBotConfig.levels[0..5] (00F8A30C + 0Ch), which 00901610 fills from
    // robots.lua. This process cannot build the context 004DC6A0 hands that
    // reader (src/game_hosts_mission_frame.cpp records the step), so the values
    // are this installation's scripts/datatables/robots.lua PilotBot blocks,
    // copied by line: Stun :1259-1273, SPNormal :568-582, SPVeteran :707-721,
    // MPNormal :845-859, MPVeteran :983-997, Elite :1121-1135. The level index
    // is 00901610's (register_robot_config_009013d0's table): Stun 0,
    // SPNormal 1, SPVeteran 2, MPNormal 3, MPVeteran 4, Elite 5.
    struct PilotDiveBombRow {
        float release_alt_1_044;
        float release_alt_2_048;
        float new_release_mul_04c;
        float max_power_ctrl_050;
        float min_power_ctrl_054;
        float max_brake_ctrl_058;
        float min_brake_ctrl_05c;
        float aim_pitch_ratio_060;
        float aim_prec_dist_068;
        float aim_prec_mul_06c;
        float aim_prec_pull_plus_070;
        float aim_prec_pull_minus_074;
    };
    static constexpr PilotDiveBombRow kPilotDiveBombRows[6] = {
        {350.0f, 450.0f, 0.6f, 0.6f, 0.1f, 0.5f, 0.0f, 3.0f, 150.0f, 0.75f, 0.03f, 0.06f},   // Stun
        {350.0f, 450.0f, 0.6f, 0.7f, 0.2f, 0.5f, 0.0f, 3.0f, 70.0f, 0.3f, 0.018f, 0.025f},   // SPNormal
        {250.0f, 300.0f, 0.6f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},        // SPVeteran
        {250.0f, 300.0f, 0.6f, 0.85f, 0.5f, 0.25f, 0.0f, 3.0f, 50.0f, 0.24f, 0.014f, 0.018f}, // MPNormal
        {250.0f, 300.0f, 0.6f, 1.0f, 0.8f, 0.1f, 0.0f, 3.0f, 20.0f, 0.20f, 0.012f, 0.016f},  // MPVeteran
        {250.0f, 300.0f, 0.6f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},        // Elite
    };
    static_assert(kPilotDiveBombRows[1].release_alt_1_044 == kDiveBombReleaseAlt1
                  && kPilotDiveBombRows[1].aim_prec_dist_068 == kDiveBombAimPrecDist
                  && kPilotDiveBombRows[1].aim_prec_mul_06c == kDiveBombAimPrecMul
                  && kPilotDiveBombRows[1].aim_prec_pull_plus_070 == kDiveBombAimPrecPullPlus
                  && kPilotDiveBombRows[1].aim_prec_pull_minus_074 == kDiveBombAimPrecPullMinus
                  && kPilotDiveBombRows[1].new_release_mul_04c == kDiveBombNewReleaseMul,
                  "row 1 is the SPNormal row the host used before the binding");
    // The captured row. Switch off: SPNormal, the old behaviour. An index
    // outside 0..5 would read past the native's array; the host takes row 1.
    static int dive_bomb_row_index(const GameUnitSlot& slot) noexcept {
        const int i = slot.db_skill_row_14;
        return (!kSkillLevelBound || i < 0 || i > 5) ? 1 : i;
    }
    static const PilotDiveBombRow& dive_bomb_row(const GameUnitSlot& slot) noexcept {
        const int i = slot.db_skill_row_14;
        if (!kSkillLevelBound || i < 0 || i > 5) return kPilotDiveBombRows[1];
        return kPilotDiveBombRows[i];
    }
    // Packet cc9_strafe_arm. The strafe fields of the six PilotBot rows, which
    // the strafe approach reads through approach+14h (the row viewed 0Ch in),
    // named as include/bsp/robot_config.hpp names them. SUBSTITUTION, labelled,
    // as for the dive-bomb rows above: this installation's
    // scripts/datatables/robots.lua (mtime 2025-06-01 23:03 UTC), by line:
    // SPNormal :605/:667-674, SPVeteran :743/:763-770, MPNormal :881/:901-908,
    // MPVeteran :1019/:1039-1046, Elite :1157/:1177-1184, Stun :1295/:1357-1364.
    // DEG(33) and DEG(35) are written in radians.
    struct PilotStrafeRow {
        float too_close_distance_0e4;
        float go_away_distance_0e8;
        float attack_angle_0ec;
        float target_point_select_prec_0f0;
        float section_damage_chance_0f4;
        float engine_room_weight_0f8;
        float magazine_weight_0fc;
        float fueltank_weight_100;
        float aim_shoot_distance_23c;
    };
    static constexpr PilotStrafeRow kPilotStrafeRows[6] = {
        {350.0f, 1000.0f, 0.575958653f, 1.0f, 0.0f, 1.0f, 0.1f, 0.1f, 800.0f},   // Stun
        {240.0f, 1000.0f, 0.575958653f, 0.9f, 0.0f, 0.5f, 0.5f, 0.5f, 850.0f},   // SPNormal
        {200.0f, 900.0f, 0.610865238f, 0.5f, 1.0f, 0.5f, 1.0f, 1.0f, 950.0f},    // SPVeteran
        {160.0f, 900.0f, 0.610865238f, 0.7f, 0.5f, 1.0f, 0.5f, 0.5f, 900.0f},    // MPNormal
        {200.0f, 900.0f, 0.610865238f, 0.55f, 0.8f, 1.0f, 1.0f, 1.0f, 900.0f},   // MPVeteran
        {200.0f, 900.0f, 0.610865238f, 0.5f, 1.0f, 0.5f, 1.0f, 1.0f, 950.0f},    // Elite
    };
    static const PilotStrafeRow& strafe_row(const GameUnitSlot& slot) noexcept {
        const int i = slot.pilot_skill_index;
        if (!kSkillLevelBound || i < 0 || i > 5) return kPilotStrafeRows[1];
        return kPilotStrafeRows[i];
    }
    // Packet cc13-dogfight1. AimShootTime and AimShootDelayTime of the six
    // PilotBot rows, which 009FC7C0 reads through 0099C210 (00F8A30C +
    // level * 248h + 0Ch): view +234h/+238h/+23Ch/+240h are robot_config.hpp's
    // aim_shoot_time_2_240/_1_244 and aim_shoot_delay_time_2_248/_1_24c, so the
    // draw's low end is the Lua table's second element. SUBSTITUTION, labelled,
    // as above: this installation's scripts/datatables/robots.lua (mtime
    // 2025-06-01 23:03 UTC), by line: SPNormal :607-608, SPVeteran :745-746,
    // MPNormal :883-884, MPVeteran :1021-1022, Elite :1159-1160, Stun :1297-1298.
    struct PilotAimShootRow {
        float time_lo_234;
        float time_hi_238;
        float delay_lo_23c;
        float delay_hi_240;
    };
    static constexpr PilotAimShootRow kPilotAimShootRows[6] = {
        {2.0f, 3.0f, 1.0f, 2.0f},     // Stun
        {3.0f, 4.5f, 0.7f, 1.6f},     // SPNormal
        {8.0f, 10.0f, 0.0f, 0.1f},    // SPVeteran
        {4.0f, 5.5f, 0.9f, 1.4f},     // MPNormal
        {5.0f, 7.0f, 0.5f, 1.0f},     // MPVeteran
        {8.0f, 10.0f, 0.0f, 0.1f},    // Elite
    };
    static const PilotAimShootRow& aim_shoot_row(const GameUnitSlot& slot) noexcept {
        const int i = slot.pilot_skill_index;
        if (!kSkillLevelBound || i < 0 || i > 5) return kPilotAimShootRows[1];
        return kPilotAimShootRows[i];
    }

    // Packet cc9_ship_motion_tail. ShipGlobals.Formacio.UpdateInterval (0083F0AD
    // stores it at settings+430h); 10.0 in this installation's shipglobals.lua.
    float formacio_update_interval_430() {
        if (formacio_update_interval < 0.0f) {
            const char chunk[] =
                "local g = type(ShipGlobals) == 'table' and ShipGlobals.Formacio or nil\n"
                "if type(g) ~= 'table' then return -1 end\n"
                "return tonumber(g.UpdateInterval) or -1\n";
            const int top = lua.lua_gettop();
            float value = 0.0f;
            if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                    "bsp_formacio_interval") == 0 &&
                lua.lua_pcall(0, 1, 0) == 0) {
                const std::string text = lua.lua_tolstring_at_top();
                if (!text.empty()) value = static_cast<float>(std::atof(text.c_str()));
            }
            lua.lua_settop(top);
            formacio_update_interval = value > 0.0f ? value : 0.0f;
            log.notef("ship motion tail: Formacio.UpdateInterval = %.3f (settings+430h)",
                static_cast<double>(formacio_update_interval));
        }
        return formacio_update_interval;
    }
    // Packet cc9_l66_putto. ShipGlobals.Formacio.FormationShipDist (0083F06B stores
    // it at settings+42Ch), the spacing 0070EFD0 reads at 0070EFFE.
    float formacio_ship_dist_42c() {
        if (formacio_ship_dist < 0.0f) {
            const char chunk[] =
                "local g = type(ShipGlobals) == 'table' and ShipGlobals.Formacio or nil\n"
                "if type(g) ~= 'table' then return -1 end\n"
                "return tonumber(g.FormationShipDist) or -1\n";
            const int top = lua.lua_gettop();
            float value = 0.0f;
            if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                    "bsp_formacio_ship_dist") == 0 &&
                lua.lua_pcall(0, 1, 0) == 0) {
                const std::string text = lua.lua_tolstring_at_top();
                if (!text.empty()) value = static_cast<float>(std::atof(text.c_str()));
            }
            lua.lua_settop(top);
            formacio_ship_dist = value > 0.0f ? value : 0.0f;
            log.notef("PutFormationTo: Formacio.FormationShipDist = %.3f (settings+42Ch)",
                static_cast<double>(formacio_ship_dist));
        }
        return formacio_ship_dist;
    }
    // Packet s53_wreck_debris. ShipGlobals.DebrisStruct, as 0083B5E0's fragment
    // stores it (0084124E boolean, default false; 00841294 number, default 0.3).
    bool debris_enabled_settings_680() {
        if (debris_enabled_680 < 0) {
            const char chunk[] =
                "local g = type(ShipGlobals) == 'table' and ShipGlobals.DebrisStruct or nil\n"
                "if type(g) ~= 'table' then return '0 0.3' end\n"
                "return (g.DebrisEnabled == true and '1 ' or '0 ') .. "
                "tostring(tonumber(g.DebrisNumPerMeter) or 0.3)\n";
            const int top = lua.lua_gettop();
            int enabled = 0;
            float per_meter = 0.3f;
            if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                    "bsp_debris_struct") == 0 &&
                lua.lua_pcall(0, 1, 0) == 0) {
                const std::string text = lua.lua_tolstring_at_top();
                if (text.size() >= 2) {
                    enabled = text[0] == '1' ? 1 : 0;
                    per_meter = static_cast<float>(std::atof(text.c_str() + 2));
                }
            }
            lua.lua_settop(top);
            debris_enabled_680 = enabled;
            debris_num_per_meter_684 = per_meter;
            log.notef("sinking wreck debris: DebrisStruct.DebrisEnabled = %d, DebrisNumPerMeter = "
                "%.3f (settings+680h / +684h, packet s53_wreck_debris)", enabled,
                static_cast<double>(per_meter));
        }
        return debris_enabled_680 == 1;
    }
    // 0070DB60 BSP_UnitGroup_SwapSlotsByDistance. A record until packet
    // cc9_ship_motion_tail's part 8b binds the reconstruction.
    void run_formation_slot_swap_0070db60(std::int32_t group) {
        if constexpr (!kFormationSlotSwapBound) {
            record("UnitGroup::swap_slots_by_distance_0070db60", 0x0070db60u);
            return;
        } else {
            if (group < 0 || static_cast<std::size_t>(group) >= formation_groups.size()) return;
            const std::size_t g = static_cast<std::size_t>(group);
            if (!slot_swap_membership_logged) {
                slot_swap_membership_logged = true;
                for (std::size_t k = 0; k < formation_groups.size(); ++k) {
                    std::string names;
                    for (const auto& m : formation_groups[k].members) {
                        if (m.entity == 0 || m.entity > slots.size()) continue;
                        if (!names.empty()) names += ",";
                        names += slots[m.entity - 1]->row.name;
                    }
                    log.notef("formation group %zu: leader=%s type=%d column=%d members=[%s]", k,
                        formation_groups[k].leader < slots.size()
                            ? slots[formation_groups[k].leader]->row.name.c_str() : "-",
                        formation_groups[k].type_04fc, formation_groups[k].column, names.c_str());
                }
            }
            if (slot_swap_runs_by_group.size() < formation_groups.size()) {
                slot_swap_runs_by_group.resize(formation_groups.size(), 0);
                slot_swaps_by_group.resize(formation_groups.size(), 0);
                slot_swap_gated_by_group.resize(formation_groups.size(), 0);
            }
            FormationGroup& fg = formation_groups[g];
            struct Host final : bsp::UnitGroupSlotSwapHost {
                Host(Impl& o, std::size_t leader) : owner(o), leader_(leader) {}
                std::uint32_t member_class_0538(std::uint32_t entity) override {
                    if (entity == 0 || entity > owner.slots.size()) return 0;
                    return static_cast<std::uint32_t>(owner.slots[entity - 1]->row.type_id);
                }
                bool decompose_against_leader_00811180(std::uint32_t entity, float& across,
                                                       float& along) override {
                    if (entity == 0 || entity > owner.slots.size() ||
                        leader_ >= owner.slots.size()) return false;
                    const float* p = owner.slots[entity - 1]->world.data() + 12;  // +0FCh
                    const float world[3] = {p[0], p[1], p[2]};
                    const bsp::ShipAiWakeDecomposition d =
                        bsp::ship_ai_wake_decompose_00811180(owner.slots[leader_]->wake, world);
                    across = d.across;
                    along = d.along;
                    return d.valid;
                }
                Impl& owner;
                std::size_t leader_;
            } host_(*this, fg.leader);
            const std::uint32_t controlled =
                controlled_bound ? static_cast<std::uint32_t>(controlled_index + 1) : 0u;
            const bsp::UnitGroupSlotSwapResult r = bsp::unit_group_swap_slots_by_distance_0070db60(
                fg.members, fg.column, fg.type_04fc, controlled, host_);
            ++slot_swap_runs_by_group[g];
            slot_swaps_by_group[g] += static_cast<unsigned long long>(r.swaps);
            if (r.gated) slot_swap_gated_by_group[g] = 1;
            slot_swap_pairs += static_cast<unsigned long long>(r.pairs_compared);
            slot_swap_pairs_invalid += static_cast<unsigned long long>(r.pairs_skipped_invalid);
            done("UnitGroup::swap_slots_by_distance_0070db60", 0x0070db60u);
            if (r.swaps > 0) {
                log.notef("formation slot swap: group=%zu swaps=%d t=%.2f", g, r.swaps,
                    static_cast<double>(summary.simulated_seconds));
            }
        }
    }

    // Packet cc9_units_contracts. The count of the class's Damage.Sections, the
    // records 0087CA80 appends (memset 30h at 0087CE00, pushed at 0087CE48) into
    // the descriptor vector +18h (+1Ch..+20h). 0087BCC0 sizes the unit's part
    // table unit+344h (+348h..+34Ch) to that count (0087BD20-0087BD4F) and
    // fills one pointer per record (0087BDBC), so (+34Ch - +348h) >> 2 is it.
    // Every table entry is counted, as the image's iterator does; -1 when the
    // class row is absent.
    int damage_section_count(int class_id) {
        const auto it = part_count_cache.find(class_id);
        if (it != part_count_cache.end()) return it->second;
        char chunk[512];  // the chunk is about 270 bytes; 256 truncated it
        // 004B7F70 is the image's cdecl _vsprintf_s(dst, 200h, fmt, va)
        // wrapper (bsp.py show 004b7f70). Use the existing body on this
        // actual query buffer; the loaded class id and fixed Lua source
        // are the same inputs as the OFF arm. All substituted values are
        // decimal int32s, so the complete query fits even at INT_MIN.
        bsp::cc13_left8::format_string_512_004b7f70(chunk,
            "local c = type(VehicleClass) == 'table' and VehicleClass[%d] or nil\n"
            "if type(c) ~= 'table' then return -1 end\n"
            "local d = c.Damage\n"
            "if type(d) ~= 'table' or type(d.Sections) ~= 'table' then return 0 end\n"
            "local n = 0\n"
            "for _, v in pairs(d.Sections) do n = n + 1 end\n"
            "return n\n", class_id);
        const int top = lua.lua_gettop();
        int value = -2;   // -2: the chunk did not run; -1: no class row
        if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                "bsp_damage_sections") == 0 &&
            lua.lua_pcall(0, 1, 0) == 0) {
            const std::string text = lua.lua_tolstring_at_top();
            if (!text.empty()) value = std::atoi(text.c_str());
        }
        lua.lua_settop(top);
        part_count_cache[class_id] = value;
        return value;
    }

    // Packet cc9_units_contracts. 007F10B0 (squadron vtable 00D087C0 slot
    // +130h, called after placement) pushes the squadron into list 1
    // (00928560), list 2 (+30h) and list 24 (+138h). This host fuses a
    // squadron with a member slot (squadron_unit, else its first member), which
    // the plane registrar already put in its own lists, so only list 24 is
    // pushed here, with that slot as the node. SUBSTITUTION, labelled: the push
    // happens at the first units step after the record has a unit, not inside
    // the creation call (at most one frame later).
    void sync_squadron_world_list_24() {
        auto& records = bsp::plane_squadron_registry().records();
        if (squadron_list24_pushed.size() < records.size()) {
            squadron_list24_pushed.resize(records.size(), 0);
        }
        for (std::size_t i = 0; i < records.size(); ++i) {
            if (squadron_list24_pushed[i] != 0) continue;
            const bsp::PlaneSquadronHostRecord& r = records[i];
            std::size_t node = r.squadron_unit;
            if (node == bsp::kPlaneSquadronNoUnit) {
                for (const std::size_t m : r.member_units) {
                    if (m != bsp::kPlaneSquadronNoUnit) { node = m; break; }
                }
            }
            if (node == bsp::kPlaneSquadronNoUnit || node >= slots.size()) continue;
            squadron_list24_pushed[i] = 1;
            // 007F10C7-007F10CE: ECX = [squadron+30h] + 138h, the id 24 list.
            world_list_push_back_00484540(world_lists,
                static_cast<std::uint32_t>(bsp::kWorldSlotArrayOffset +
                                           bsp::kWorldSlotStride * 24),
                *slots[node]);
            ++squadron_list24_pushes;
            done("PlaneSquadron::register_in_world_lists_007f10b0", 0x007f10b0u);
            log.notef("squadron world list 24 push: squadron=%s node=%s t=%.2f",
                r.name.c_str(), slots[node]->row.name.c_str(),
                static_cast<double>(summary.simulated_seconds));
        }
    }

    // VehicleClass[class].Equipments[DefaultEquipment or 1][platform].Ammo, the
    // authored round count of one platform (the equipment reader 00961F57 stores
    // Platform, Ammo and ReloadTime per entry). -1 when any level is absent.
    // ASSUMED: the aircraft carries its DefaultEquipment; a mission that picks
    // another equipment is not modelled.
    // `equipment` 0 keeps the default above; packet cc9_ai_plane_loadout_arm
    // passes 1 for the scene generator's loadout.
    int read_equipment_ammo(int class_id, int platform_key, int equipment = 0) {
        if (platform_key < 0) return -1;
        char chunk[512];
        char index[48];
        if (equipment > 0) {
            std::snprintf(index, sizeof(index), "%d", equipment);
        } else {
            std::snprintf(index, sizeof(index), "c.DefaultEquipment or 1");
        }
        // Same bounded formatter and real loadout query as damage_section_count.
        // index is either one decimal int32 or the fixed default expression.
        bsp::cc13_left8::format_string_512_004b7f70(chunk,
            "local c = type(VehicleClass) == 'table' and VehicleClass[%d] or nil\n"
            "if type(c) ~= 'table' or type(c.Equipments) ~= 'table' then return -1 end\n"
            "local e = c.Equipments[%s]\n"
            "if type(e) ~= 'table' or type(e[%d]) ~= 'table' then return -1 end\n"
            "return tonumber(e[%d].Ammo) or -1\n", class_id, index, platform_key, platform_key);
        const int top = lua.lua_gettop();
        int value = -1;
        if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                "bsp_equipment_ammo") == 0 &&
            lua.lua_pcall(0, 1, 0) == 0) {
            const std::string text = lua.lua_tolstring_at_top();
            if (!text.empty()) value = std::atoi(text.c_str());
        }
        lua.lua_settop(top);
        return value;
    }

    // Packet cc9_plane_scene_equipment (kPlaneSceneEquipmentBound): a rack's
    // authored Ammo from the plane's own equipment index. 007CDFDE..007CDFF8:
    // Equipments[n] for n > 0, else a null entry, so 0095A880 attaches nothing
    // and the rack holds no round (-1). A slot whose record carried no bag
    // value keeps the class's `DefaultEquipment or 1` (LABELLED, as before).
    int rack_equipment_ammo(const GameUnitSlot& s, int class_id, int platform_key) {
        if constexpr (bsp::kPlaneSceneEquipmentBound) {
            if (s.bag_equipment >= 0) {
                ++plane_equipment_reads;
                if (s.bag_equipment == 0) {
                    ++plane_equipment_none;
                    return -1;
                }
                return read_equipment_ammo(class_id, platform_key, s.bag_equipment);
            }
            ++plane_equipment_default;
        }
        return read_equipment_ammo(class_id, platform_key);
    }
    // Packet cc9_rack_device_reload: Equipments[e][platform].ReloadTime, the value
    // the equipment reader 00961F57 stores per entry and 0095A880 hands the rack's
    // vtable[1C0h] (006E3A20). -1.0 when any level is absent; the equipment index
    // is chosen as rack_equipment_ammo chooses it.
    float rack_equipment_reload_time(const GameUnitSlot& s, int class_id, int platform_key) {
        if (platform_key < 0) return -1.0f;
        int equipment = 0;
        if constexpr (bsp::kPlaneSceneEquipmentBound) {
            if (s.bag_equipment == 0) return -1.0f;
            if (s.bag_equipment > 0) equipment = s.bag_equipment;
        }
        char chunk[512];
        char index[48];
        if (equipment > 0) {
            std::snprintf(index, sizeof(index), "%d", equipment);
        } else {
            std::snprintf(index, sizeof(index), "c.DefaultEquipment or 1");
        }
        bsp::cc13_left8::format_string_512_004b7f70(chunk,
            "local c = type(VehicleClass) == 'table' and VehicleClass[%d] or nil\n"
            "if type(c) ~= 'table' or type(c.Equipments) ~= 'table' then return -1 end\n"
            "local e = c.Equipments[%s]\n"
            "if type(e) ~= 'table' or type(e[%d]) ~= 'table' then return -1 end\n"
            "return tonumber(e[%d].ReloadTime) or -1\n", class_id, index, platform_key, platform_key);
        const int top = lua.lua_gettop();
        float value = -1.0f;
        if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                "bsp_equipment_reload_time") == 0 &&
            lua.lua_pcall(0, 1, 0) == 0) {
            const std::string text = lua.lua_tolstring_at_top();
            if (!text.empty()) value = static_cast<float>(std::atof(text.c_str()));
        }
        lua.lua_settop(top);
        return value;
    }
    // Packet cc9_lua45_rack_equipment_device: Equipments[e][platform].Platform,
    // -1 when any level is absent. `equipment` 0 reads `DefaultEquipment or 1`.
    int read_equipment_platform(int class_id, int platform_key, int equipment) {
        if (platform_key < 0) return -1;
        char chunk[512];
        char index[48];
        if (equipment > 0) {
            std::snprintf(index, sizeof(index), "%d", equipment);
        } else {
            std::snprintf(index, sizeof(index), "c.DefaultEquipment or 1");
        }
        bsp::cc13_left8::format_string_512_004b7f70(chunk,
            "local c = type(VehicleClass) == 'table' and VehicleClass[%d] or nil\n"
            "if type(c) ~= 'table' or type(c.Equipments) ~= 'table' then return -1 end\n"
            "local e = c.Equipments[%s]\n"
            "if type(e) ~= 'table' or type(e[%d]) ~= 'table' then return -1 end\n"
            "return tonumber(e[%d].Platform) or -1\n", class_id, index, platform_key, platform_key);
        const int top = lua.lua_gettop();
        int value = -1;
        if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                "bsp_equipment_platform") == 0 &&
            lua.lua_pcall(0, 1, 0) == 0) {
            const std::string text = lua.lua_tolstring_at_top();
            if (!text.empty()) value = std::atoi(text.c_str());
        }
        lua.lua_settop(top);
        return value;
    }
    // The rack's device under kRackEquipmentDeviceBound; `class_dev` otherwise.
    int rack_equipment_device(const GameUnitSlot& s, int class_id, int platform_key,
                              int class_dev) {
        if constexpr (!kRackEquipmentDeviceBound) {
            (void)s; (void)class_id; (void)platform_key;
            return class_dev;
        } else {
            int equipment = 0;
            if constexpr (bsp::kPlaneSceneEquipmentBound) {
                if (s.bag_equipment == 0) return class_dev;
                if (s.bag_equipment > 0) equipment = s.bag_equipment;
            }
            const int dev = read_equipment_platform(class_id, platform_key, equipment);
            if (dev >= 0) ++rack_equipment_devices;
            return dev >= 0 ? dev : class_dev;
        }
    }
    // 006E0242: 00B66330("RepeatTime") with the default [00CE3D30] = 0.6f.
    static constexpr float kRackRepeatTimeDefault = 0.6f;
    float rack_repeat_default() const noexcept {
        return kRackEquipmentDeviceBound ? kRackRepeatTimeDefault : 0.0f;
    }
    unsigned long long rack_equipment_devices{0};
    unsigned long long plane_equipment_reads{0};
    unsigned long long plane_equipment_none{0};
    unsigned long long plane_equipment_default{0};
    unsigned long long plane_equipment_squadrons{0};

    // Packet cc9_squadron_ordnance_state (kSquadronOrdnanceReaderBound). The
    // projectile class a rack carries: DeviceClass[dev].Bullet[1].Bullet ->
    // Bullets[b].Type, as the entity class id the descriptor answers
    // vtable[8] with (docs/ORDNANCE_KIND_IDENTITY.md, docs/ENTITY_CLASS_IDS.md):
    // Bomb 2Ah, Torpedo 2Bh, Depthcharge 2Ch, DummyKamikazePlane 2Fh,
    // Paratrooper 31h, Rocket 33h. Any other type answers 0.
    // Packet cc9_lua53_kamikaze_impact: DeviceClass[device].Bullet[1].Bullet, the
    // bullet class id (-1 when absent).
    int read_device_bullet_id(int device) {
        if (device < 0) return -1;
        char chunk[384];
        std::snprintf(chunk, sizeof(chunk),
            "local d = type(DeviceClass) == 'table' and DeviceClass[%d] or nil\n"
            "if type(d) ~= 'table' or type(d.Bullet) ~= 'table' or type(d.Bullet[1]) ~= 'table' then return -1 end\n"
            "return tonumber(d.Bullet[1].Bullet) or -1\n", device);
        const int top = lua.lua_gettop();
        int id = -1;
        if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                "bsp_device_bullet_id") == 0 &&
            lua.lua_pcall(0, 1, 0) == 0) {
            id = std::atoi(lua.lua_tolstring_at_top().c_str());
        }
        lua.lua_settop(top);
        return id;
    }
    // Packet cc13-oprocket1: what 007C1EB0 asks of a rocket rack's bullet class:
    // IgnitionDelay (+E0h) > 0 is the big rocket; AntiAir (+E4h) picks the target.
    // 0 not a rocket, 1 small non-AA, 2 small AA, 3 IgnitionDelay > 0.
    int read_device_rocket_kind(int device) {
        if (device < 0) return 0;
        char chunk[640];
        std::snprintf(chunk, sizeof(chunk),
            "local d = type(DeviceClass) == 'table' and DeviceClass[%d] or nil\n"
            "if type(d) ~= 'table' or type(d.Bullet) ~= 'table' or type(d.Bullet[1]) ~= 'table' then return '0' end\n"
            "local b = type(Bullets) == 'table' and Bullets[d.Bullet[1].Bullet] or nil\n"
            "if type(b) ~= 'table' or b.Type ~= 'Rocket' then return '0' end\n"
            "if (tonumber(b.IgnitionDelay) or 0) > 0 then return '3' end\n"
            "return b.AntiAir and '2' or '1'\n", device);
        const int top = lua.lua_gettop();
        std::string kind;
        if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                "bsp_device_rocket_kind") == 0 &&
            lua.lua_pcall(0, 1, 0) == 0) {
            kind = lua.lua_tolstring_at_top();
        }
        lua.lua_settop(top);
        return kind.empty() ? 0 : kind[0] - '0';
    }
    // Packet cc9-rocket1: the class values the rocket task reads off its aimed
    // rocket (007C1EB0 / 00809DE0 / 00809E10): IgnitionDelay classDesc+E0h, VMax
    // +D8h, Acceleration +DCh and AntiAir +E4h, by Bullets id.
    struct RocketBulletRow {
        bool found{false};
        float ignition_delay{0.0f}, vmax{0.0f}, acceleration{0.0f};
        bool anti_air{false};
    };
    RocketBulletRow read_rocket_bullet_row(int bullet) {
        RocketBulletRow r;
        if (bullet < 0) return r;
        char chunk[512];
        std::snprintf(chunk, sizeof(chunk),
            "local b = type(Bullets) == 'table' and Bullets[%d] or nil\n"
            "if type(b) ~= 'table' then return '' end\n"
            "return string.format('%%.17g %%.17g %%.17g %%d', tonumber(b.IgnitionDelay) or 0,\n"
            "  tonumber(b.VMax) or 0, tonumber(b.Acceleration) or 0, b.AntiAir and 1 or 0)\n",
            bullet);
        const int top = lua.lua_gettop();
        std::string text;
        if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                "bsp_rocket_bullet_row") == 0 &&
            lua.lua_pcall(0, 1, 0) == 0) {
            text = lua.lua_tolstring_at_top();
        }
        lua.lua_settop(top);
        double ign = 0.0, vmax = 0.0, acc = 0.0;
        int aa = 0;
        if (std::sscanf(text.c_str(), "%lf %lf %lf %d", &ign, &vmax, &acc, &aa) == 4) {
            r.found = true;
            r.ignition_delay = static_cast<float>(ign);
            r.vmax = static_cast<float>(vmax);
            r.acceleration = static_cast<float>(acc);
            r.anti_air = aa != 0;
        }
        return r;
    }
    // Packet cc9-rocket1, the PilotSetTarget feed: 007ED970 -> 007B9480 on one
    // plane, some rack whose descriptor is kind 33h with !(0 >= IgnitionDelay).
    // Once the plane's rack census ran, its rocket platforms (orx_rack_*, kind 3
    // = IgnitionDelay > 0) with a round left answer; before it (the mission
    // script orders during luaStageInit) the same platforms are walked from the
    // class with their authored rounds. SUBSTITUTION, labelled: a rack's rounds
    // stand for its attached child (rack vt[220h] answers null once spent).
    bool plane_has_big_rocket_007b9480(const GameUnitSlot& u) {
        if (!bsp::unit_is_kind_of(u.class_id, 0x0F)) return false;
        if (u.rack_census_done) {
            for (std::size_t i = 0; i < u.orx_rack_kind.size() && i < u.orx_rack_ammo.size(); ++i) {
                if (u.orx_rack_kind[i] == 3 && u.orx_rack_ammo[i] > 0) return true;
            }
            return false;
        }
        const int type_id = u.row.type_id;
        const int platforms = lua.read_vehicle_class_integer(type_id, "BSPGun", "n", 0);
        for (int p = 1; p <= platforms && p <= 64; ++p) {
            char key[32];
            std::snprintf(key, sizeof(key), "p%d_cat", p);
            if (lua.read_vehicle_class_integer(type_id, "BSPGun", key, -1) !=
                static_cast<int>(bsp::GunneryCategory::kBombPlatform)) continue;
            std::snprintf(key, sizeof(key), "p%d_dev", p);
            const int class_dev = lua.read_vehicle_class_integer(type_id, "BSPGun", key, -1);
            std::snprintf(key, sizeof(key), "p%d_key", p);
            const int pkey = lua.read_vehicle_class_integer(type_id, "BSPGun", key, -1);
            const int dev = rack_equipment_device(u, type_id, pkey, class_dev);
            const std::string type = dev >= 0 ? lua.read_device_class_string(dev, "Type")
                                              : std::string();
            if (type != "BombPlatform" && type != "MultiBombPlatform") continue;
            if (read_device_rocket_kind(dev) == 3 && rack_equipment_ammo(u, type_id, pkey) > 0)
                return true;
        }
        return false;
    }
    int read_device_bullet_class_id(int device) {
        if (device < 0) return 0;
        char chunk[512];
        std::snprintf(chunk, sizeof(chunk),
            "local d = type(DeviceClass) == 'table' and DeviceClass[%d] or nil\n"
            "if type(d) ~= 'table' or type(d.Bullet) ~= 'table' or type(d.Bullet[1]) ~= 'table' then return '' end\n"
            "local b = type(Bullets) == 'table' and Bullets[d.Bullet[1].Bullet] or nil\n"
            "if type(b) ~= 'table' then return '' end\n"
            "return tostring(b.Type or '')\n", device);
        const int top = lua.lua_gettop();
        std::string type;
        if (lua.luaL_loadbuffer(chunk, static_cast<int>(std::strlen(chunk)),
                                "bsp_device_bullet_type") == 0 &&
            lua.lua_pcall(0, 1, 0) == 0) {
            type = lua.lua_tolstring_at_top();
        }
        lua.lua_settop(top);
        if (type == "Bomb") return 0x2A;
        if (type == "Torpedo") return 0x2B;
        if (type == "Depthcharge") return 0x2C;
        if (type == "DummyKamikazePlane") return 0x2F;
        if (type == "Paratrooper") return 0x31;
        if (type == "Rocket") return 0x33;
        if constexpr (bsp::kSquadronAmmoDummyKindsCc13Bound) {
            // Packet cc13-ammodk: the descriptor ids of docs/ORDNANCE_KIND_IDENTITY.md.
            if (type == "DummyTarget") return 0x2E;
            if (type == "DummySubmarine") return 0x30;
            if (type == "WaterMine") return 0x34;
            return 0;
        } else {
            return 0;
        }
    }

    // The racks of one aircraft in BSPGun order, the order the census walks:
    // the class id each carries, single or multi, and its index among the
    // single racks (rack_ammo_per_rack's index).
    const std::vector<PlaneOrdnanceRack>& ordnance_racks(GameUnitSlot& s) {
        if (s.ordnance_racks_read) return s.ordnance_racks;
        s.ordnance_racks_read = true;
        const int type_id = s.row.type_id;
        const int platforms = lua.read_vehicle_class_integer(type_id, "BSPGun", "n", 0);
        int singles = 0;
        for (int p = 1; p <= platforms && p <= 64; ++p) {
            char key[32];
            std::snprintf(key, sizeof(key), "p%d_cat", p);
            if (lua.read_vehicle_class_integer(type_id, "BSPGun", key, -1)
                != static_cast<int>(bsp::GunneryCategory::kBombPlatform)) continue;
            std::snprintf(key, sizeof(key), "p%d_dev", p);
            const int dev = lua.read_vehicle_class_integer(type_id, "BSPGun", key, -1);
            const std::string type = dev >= 0 ? lua.read_device_class_string(dev, "Type")
                                              : std::string();
            PlaneOrdnanceRack r;
            r.class_id = read_device_bullet_class_id(dev);
            std::snprintf(key, sizeof(key), "p%d_key", p);
            const int platform_key = lua.read_vehicle_class_integer(type_id, "BSPGun", key, -1);
            r.platform_key = platform_key;
            r.device_class = dev;
            r.authored_ammo = rack_equipment_ammo(s, type_id, platform_key);
            r.generator_ammo = read_equipment_ammo(type_id, platform_key, 1);
            if (type == "BombPlatform") {
                r.single_index = singles++;
                r.authored_ammo = rack_positive_bag_round(s, r.authored_ammo);
            } else if (type == "MultiBombPlatform") {
                r.multi = true;
            } else {
                continue;
            }
            s.ordnance_racks.push_back(r);
        }
        return s.ordnance_racks;
    }

    // slot->vtable[220h](1): 006E4060 (single) answers the hanging child's
    // descriptor while a round is attached. With live [00E17BF2] set, it may
    // instead return its configured bomb-family descriptor; SP missions can
    // enable this. This helper projects ATTACHED rounds only; the first-slot
    // descriptor query handles the reload fallback separately. 006E4640
    // (multi) also answers its +514h entries. LABELLED: a rack holds a round
    // while its ammo +484h is above 0 - for a single rack the host's own count
    // (rack_ammo_per_rack, else rack_ammo) once the issue has seeded it, before
    // that the default equipment's authored Ammo; a rack the default equipment
    // does not load (no entry) holds none. A multi rack's +514h entries are not
    // modelled, so it answers its authored Ammo.
    bool rack_holds_round(const GameUnitSlot& s, const PlaneOrdnanceRack& r) const {
        if (!r.multi) {
            const std::size_t i = static_cast<std::size_t>(r.single_index);
            if (i < s.rack_ammo_per_rack.size()) return s.rack_ammo_per_rack[i] > 0;
            if (s.rack_ammo >= 0 && s.rack_single_count <= 1) return s.rack_ammo > 0;
        }
        return r.authored_ammo > 0;
    }

    // One rack's answer to vtable[210h](2Ah, flag) as plane_holds_rack_round_007b9140
    // takes it: the unseeded fallbacks below, then rack_holds_round. Shared by
    // GetPlaneIsReloading's 007B92E0 (packet cc13-fix16), whose vtable[20Ch]
    // 006E3570 is the same count read the other way (vtable[21Ch](2Ah) + ammo == 0).
    bool plane_rack_holds_round(const GameUnitSlot& s, const PlaneOrdnanceRack& r) const {
        // LABELLED: rack_holds_round's unseeded fallback is the DefaultEquipment
        // Ammo. A scene squadron with no authored Equipment flies Equipments[1]
        // (0094BD34), so a rack the default leaves empty (a Zero's, whose class
        // has DefaultEquipment 0) takes that loadout's Ammo until the release
        // issue seeds the host's own count.
        PlaneOrdnanceRack g = r;
        // With kPlaneSceneEquipmentBound and a carried index, authored_ammo is
        // already the plane's own loadout and an empty one stays empty.
        const bool own_index = bsp::kPlaneSceneEquipmentBound && s.bag_equipment >= 0;
        if (g.authored_ammo < 0 && !own_index) g.authored_ammo = g.generator_ammo;
        if constexpr (kRackConstructorRoundBound) {
            // 006E3350 with +C54h <= 0: no loadout Ammo, the constructor's one
            // round attached. LABELLED: only bag 0; a positive bag whose
            // loadout has no entry for a rack also keeps the constructor's
            // round in the image, but the release seed gives that rack none.
            // The unread 006E33ED `[+C0h]+4 == 3` skip is taken as not met.
            if (own_index && s.bag_equipment == 0 && g.authored_ammo < 0 && !g.multi)
                g.authored_ammo = 1;
        }
        return rack_holds_round(s, g);
    }

    // Packet cc13-fix16: 007C1D80 BSP_Plane_ReloadBombPlatforms on one plane. It
    // walks the device list (+48h, next +44h) and calls vtable[200h] on every
    // device answering IsKindOf(25h). A single rack's slot is 006E3410
    // (BSP_MBombPlatform_Rearm): +490h = +48Ch (the reload countdown back to
    // ReloadTime), then ammo +484h = orgAmmo - 1 when vtable[210h](2Ah, 0) finds
    // a round attached, else ammo = orgAmmo and the tail-jump vtable[224h]
    // attaches the next one. In this host's terms (a rack's count includes the
    // attached round, as rack_device_reload_slice_006e57a4 keeps it) both arms
    // leave the rack holding orgAmmo rounds.
    // SUBSTITUTIONS, labelled:
    //  - orgAmmo is the value the host seeded the rack with (authored ? max(Ammo,
    //    0) : 1), as the device reload slice and the issue seed take it.
    //  - a plane whose racks the host has not seeded (rack_ammo < 0) holds every
    //    authored round already, so only the countdowns are reset.
    //  - no round entity is created (006E3E70 is not hosted), as in the slice.
    //  - a multi rack (26h) keeps no per-round state here; rack_holds_round
    //    answers its authored Ammo, which a reload does not change.
    //  - the rocket racks' orx_rack_ammo (packet cc13-oprocket1) is not
    //    restored: its seeded value is not kept apart from the count.
    //  - a refilled torpedo rack (bullet class 2Bh) puts back the kind-2Bh bit
    //    the gunnery host clears at a drop (game_hosts_gunnery.cpp, packet
    //    cc8_torpedo_breakoff), since that bit is this host's stand-in for
    //    the torpedo rack holding a round; 007B93F0 then answers again, as the
    //    image's does once 006E3410 has refilled +484h.
    // Returns the number of single racks whose count changed.
    int plane_reload_bomb_platforms_007c1d80(GameUnitSlot& s) {
        done("Plane::reload_bomb_platforms_007c1d80", 0x007c1d80u);
        const std::size_t n = std::min(s.rack_reload_left_per_rack.size(),
                                       s.rack_reload_time_per_rack.size());
        for (std::size_t i = 0; i < n; ++i)
            s.rack_reload_left_per_rack[i] = s.rack_reload_time_per_rack[i];   // 006E3419
        if (s.rack_ammo < 0) return 0;
        const bool authored = kDiveBombCarriedRoundsBound && s.rack_rounds_authored > 0;
        int changed = 0;
        bool torpedo_refilled = false;
        const std::size_t m = std::min(s.rack_ammo_per_rack.size(),
                                       s.rack_authored_per_rack.size());
        for (std::size_t i = 0; i < m; ++i) {
            const int a = s.rack_authored_per_rack[i];
            const int org = authored ? (a > 0 ? a : 0) : 1;
            if (org > 0 && i < s.rack_bullet_class_per_rack.size()
                && s.rack_bullet_class_per_rack[i] == 0x2B) {
                torpedo_refilled = true;
            }
            if (s.rack_ammo_per_rack[i] == org) continue;
            s.rack_ammo += org - s.rack_ammo_per_rack[i];
            s.rack_ammo_per_rack[i] = org;                                      // 006E3431 / 006E343B
            ++changed;
        }
        // store_unit_ordnance's rule: an unequipped bag (0) carries no rack kind.
        const bool unequipped = kRackUnequippedEmptyBound && bsp::kPlaneSceneEquipmentBound
            && s.bag_equipment == 0;
        if (torpedo_refilled && !unequipped) {
            const std::uint64_t torpedo_bit = std::uint64_t(1) << (0x2B - 0x08);
            if ((s.ordnance_mask & torpedo_bit) == 0) {
                s.ordnance_mask |= torpedo_bit;
                ++changed;
            }
        }
        return changed;
    }

    // Chain 324: 007C1F60's read-only census over the owned rack records. The
    // counts are the live combined attached/reserve counts: the release seed,
    // per-rack drops and reload slice maintain them. Do not split them merely
    // to call 006E3500, whose separate attached-child input is not available.
    int droppable_round_count_or_stand_in(GameUnitSlot* unit) {
        if (unit == nullptr) return 0;
        const auto& racks = ordnance_racks(*unit);
        bool complete = unit->rack_census_done && !racks.empty()
            && racks.size() == unit->rack_ammo_per_rack.size()
            && racks.size() == unit->rack_identity_per_rack.size()
            && racks.size() == unit->rack_bullet_class_per_rack.size()
            && unit->rack_multi_count == 0;
        for (const auto& rack : racks) {
            // Multi-rack attached lists are absent. Torpedo/rocket direct
            // release paths also have separate counters, so keep their old
            // query until every producer updates this per-rack owner.
            if (!complete) break;
            if (rack.multi || rack.single_index < 0
                || static_cast<std::size_t>(rack.single_index) >= unit->rack_ammo_per_rack.size()) {
                complete = false;
                break;
            }
            const auto index = static_cast<std::size_t>(rack.single_index);
            const auto& resolved = unit->rack_identity_per_rack[index];
            // The ammo census resolves Equipments[e][key].Platform; the older
            // ordnance metadata above uses BSPGun's default device. Validate
            // both identities and the resolved kind, not just the vector size.
            complete = resolved.projectile_kind == unit->rack_bullet_class_per_rack[index]
                && admits_single_rack_round_census(
                    {rack.platform_key, rack.device_class, rack.class_id}, resolved,
                    unit->rack_ammo_per_rack[index]);
        }
        if (complete) {
            struct Binding final : bsp::cc13_seg47::DroppableRoundCountHost {
                using Raw = bsp::cc13_seg47::Raw;
                Impl& owner;
                GameUnitSlot& unit;
                const std::vector<PlaneOrdnanceRack>& racks;
                Binding(Impl& o, GameUnitSlot& u, const std::vector<PlaneOrdnanceRack>& r)
                    : owner(o), unit(u), racks(r) {}
                Raw handle(const PlaneOrdnanceRack* rack) {
                    return bsp::cc13_seg47::raw(const_cast<PlaneOrdnanceRack*>(rack));
                }
                Raw first_round_device(Raw) override { return handle(racks.data()); }
                Raw next_round_device(Raw device) override {
                    const auto* rack = reinterpret_cast<const PlaneOrdnanceRack*>(device.p);
                    const std::size_t next = static_cast<std::size_t>(rack - racks.data()) + 1;
                    return handle(next < racks.size() ? &racks[next] : nullptr);
                }
                bool entity_vcall_is_kind_5c(Raw, std::int32_t kind) override {
                    return kind == 0x25; // every record came from a BombPlatform
                }
                bool rack_vcall_holds_kind_210(Raw device, std::int32_t kind,
                                               std::int32_t flag) override {
                    const auto& rack = *reinterpret_cast<const PlaneOrdnanceRack*>(device.p);
                    // 2Ah/2Ch/31h all answer 2Ah. Reuse the host's live held-round
                    // query; do not synthesize a native child or attached count.
                    return kind == 0x2A && flag == 0 && owner.plane_rack_holds_round(unit, rack);
                }
                std::int32_t bomb_platform_round_count_006e3500(Raw device) override {
                    const auto& rack = *reinterpret_cast<const PlaneOrdnanceRack*>(device.p);
                    return unit.rack_ammo_per_rack[static_cast<std::size_t>(rack.single_index)];
                }
            } binding(*this, *unit, racks);
            // 007C1F61..007C1F9C: filter kind 25h, then holds(2Ah,0), then sum.
            // BSPGun order replaces the unread native +44h child order only for
            // this read-only integer sum; the original body remains the owner.
            return bsp::cc13_seg47::unit_count_droppable_rounds_007c1f60(
                binding, bsp::cc13_seg47::raw(unit));
        }

        // Existing substitution for racks whose complete live state is absent.
        const bsp::OrdnanceKindSet set{unit->ordnance_mask};
        if (bsp::ordnance_has_torpedo_2bh(set)) {
            const int n = 1 - unit->torpedo_releases;
            return n < 0 ? 0 : n;
        }
        if (kReleaseIssueStageValsBound && bsp::ordnance_has_general_bomb_2ah(set)) {
            return unit->rack_ammo >= 0 ? unit->rack_ammo
                : (kDiveBombCarriedRoundsBound && unit->rack_rounds_authored > 0)
                    ? unit->rack_rounds_authored : unit->rack_single_count;
        }
        return 0;
    }

    // Packet cc13-rackround (A, moved): PlaneBinding::plane_rack_census, the rack
    // census once per unit, hoisted here so one implementation serves every caller.
    // The body is the original's, re-indented; `owner_` names this Impl.
    void plane_rack_census(GameUnitSlot& unit_) {
        Impl& owner_ = *this;
        if (unit_.rack_census_done) return;
        unit_.rack_census_done = true;
        const int type_id = unit_.row.type_id;
        const int platforms =
            owner_.lua.read_vehicle_class_integer(type_id, "BSPGun", "n", 0);
        for (int p = 1; p <= platforms && p <= 64; ++p) {
            char key[32];
            std::snprintf(key, sizeof(key), "p%d_cat", p);
            const int cat = owner_.lua.read_vehicle_class_integer(
                type_id, "BSPGun", key, -1);
            if (cat != static_cast<int>(bsp::GunneryCategory::kBombPlatform)) continue;
            std::snprintf(key, sizeof(key), "p%d_dev", p);
            const int class_dev = owner_.lua.read_vehicle_class_integer(
                type_id, "BSPGun", key, -1);
            // The platform's key, which also indexes the
            // equipment entry (packet cc9_lua45_rack_equipment_device).
            std::snprintf(key, sizeof(key), "p%d_key", p);
            const int pkey = owner_.lua.read_vehicle_class_integer(
                type_id, "BSPGun", key, -1);
            const int dev = owner_.rack_equipment_device(unit_, type_id, pkey, class_dev);
            const std::string type = dev >= 0
                ? owner_.lua.read_device_class_string(dev, "Type") : std::string();
            if (type == "BombPlatform") {
                ++unit_.rack_single_count;
                // The platform's authored rounds (packet
                // cc9_dive_bomb_carried_rounds). -1 when absent.
                const int ammo = owner_.rack_positive_bag_round(unit_,
                    owner_.rack_equipment_ammo(unit_, type_id, pkey));
                // 006E3530 writes ammo+484h and orgAmmo+488h from the same
                // equipment entry. Both outputs feed the existing load seed:
                // the total carried count and this rack's original capacity.
                // -1 is this reader's missing-entry sentinel, not native ammo.
                if (ammo >= 0) {
                    bsp::gameplay_seg33_cc13::SingleRackState rack;
                    bsp::gameplay_seg33_cc13::rack_set_ammo_006e3530(rack, ammo);
                    if (rack.ammo_484 > 0) unit_.rack_rounds_authored += rack.ammo_484;
                    unit_.rack_authored_per_rack.push_back(rack.org_ammo_488);
                } else {
                    unit_.rack_authored_per_rack.push_back(ammo);
                }
                // Packet cc9_rack_bullet_kind. LABELLED: the class's
                // default Bullet[1]; an equipment bag that changes
                // the bullet is not read.
                unit_.rack_bullet_class_per_rack.push_back(
                    owner_.read_device_bullet_class_id(dev));
                unit_.rack_identity_per_rack.push_back(
                    {pkey, dev, unit_.rack_bullet_class_per_rack.back()});
                unit_.rack_bullet_id_per_rack.push_back(
                    owner_.read_device_bullet_id(dev));
                // 006E0242 00B66330 default [00CE3D30] = 0.6f.
                unit_.rack_repeat_per_rack.push_back(
                    owner_.lua.read_device_class_number(dev, "RepeatTime",
                        owner_.rack_repeat_default()));
                // Packet cc9_rack_device_reload: 006E3C00 stores
                // 30.0 (00CE38C8); an entry's ReloadTime replaces it
                // through 006E3A20 (below 1.0 becomes 1.0), in both
                // +48Ch and +490h.
                float reload = 30.0f;
                const float authored_reload =
                    owner_.rack_equipment_reload_time(unit_, type_id, pkey);
                // 006E3A20..006E3A5D: both the configured period and live
                // countdown take the clamped equipment ReloadTime. Preserve
                // the existing missing-entry guard and constructor default.
                bsp::gameplay_seg33_cc13::SingleRackState rack;
                rack.reload_time_48c = reload;
                rack.reload_left_490 = reload;
                if (authored_reload >= 0.0f)
                    bsp::gameplay_seg33_cc13::rack_set_reload_time_006e3a20(
                        rack, authored_reload);
                unit_.rack_reload_time_per_rack.push_back(rack.reload_time_48c);
                unit_.rack_reload_left_per_rack.push_back(rack.reload_left_490);
            } else if (type == "MultiBombPlatform") {
                ++unit_.rack_multi_count;
            }
            if constexpr (bsp::kOpportunisticRocketBound) {
                // Packet cc13-oprocket1 (cc9-gunnery39): every rocket
                // platform, with its own authored rounds.
                const int rk = (type == "BombPlatform" || type == "MultiBombPlatform")
                    ? owner_.read_device_rocket_kind(dev) : 0;
                if (rk > 0) {
                    unit_.orx_rack_kind.push_back(rk);
                    unit_.orx_rack_bullet.push_back(owner_.read_device_bullet_id(dev));
                    unit_.orx_rack_ammo.push_back(
                        owner_.rack_equipment_ammo(unit_, type_id, pkey));
                }
            }
        }
        if constexpr (GameUnitsHost::Impl::kRackUnequippedEmptyBound &&
                      bsp::kPlaneSceneEquipmentBound) {
            // 0095A880(null, 1, 0) hands no rack a round (5ey).
            if (unit_.bag_equipment == 0 && unit_.rack_ammo < 0) {
                unit_.rack_ammo = 0;
                unit_.rack_ammo_per_rack.assign(
                    unit_.rack_authored_per_rack.size(), 0);
            }
        }
        if constexpr (GameUnitsHost::Impl::kRackBulletKindBound) {
            if (bsp::unit_is_kind_of(unit_.class_id, 0x10)
                && !unit_.rack_bullet_class_per_rack.empty()) {
                std::string k;
                for (std::size_t i = 0; i < unit_.rack_bullet_class_per_rack.size(); ++i) {
                    char b[48];
                    std::snprintf(b, sizeof(b), "%s%Xh/%.2fs", i ? " " : "",
                        unit_.rack_bullet_class_per_rack[i],
                        static_cast<double>(unit_.rack_repeat_per_rack[i]));
                    k += b;
                }
                owner_.log.notef("  rack bullet kinds %s (level bomber): [%s] "
                    "(007C0D90, packet cc9_rack_bullet_kind)",
                    unit_.row.name.c_str(), k.c_str());
            }
        }
    }

    // Packet cc13-rackround (A, moved): TorpedoReleaseOrderBinding's first-issue
    // seed at 007C0DE3-007C0DEF (the rack ammo, once: it runs only while
    // rack_ammo < 0). The block is the original's, re-indented.
    void rack_release_issue_seed_007c0de3(GameUnitSlot& slot_) {
        if (slot_.rack_ammo < 0) {
            const bool authored =
                GameUnitsHost::Impl::kDiveBombCarriedRoundsBound &&
                slot_.rack_rounds_authored > 0;
            slot_.rack_ammo = authored
                    ? slot_.rack_rounds_authored
                    : slot_.rack_single_count;
            if constexpr (GameUnitsHost::Impl::kRackRoundsPerRackBound) {
                // Each rack's own +484h: its authored Ammo,
                // or one round per rack where the census
                // found none (the pool's own fallback).
                slot_.rack_ammo_per_rack.clear();
                for (const int a : slot_.rack_authored_per_rack) {
                    slot_.rack_ammo_per_rack.push_back(
                        authored ? (a > 0 ? a : 0) : 1);
                }
            }
        }
    }

    // 007EDAD0 BSP_PlaneSquadron_AmmoType (007EDAD0-007EDB7C, __fastcall(squadron),
    // RET): for each of the +3CCh planes at +3D0h, in turn 007B93F0 (2Bh) -> 2,
    // 007B94F0 (2Ch) -> 3, 007B9400 (33h) -> 4, 007B9500 (31h) -> 5, 007B93E0
    // (2Fh) -> 6, 007B9320 (2Ah and none of 2Ch, 31h, 2Bh, 33h, 2Dh) -> 1; each
    // is 007B91C0's walk of the plane's +994h racks at +974h with loadout 1.
    // No plane answering: 0.
    int squadron_ammo_type_007edad0(const bsp::PlaneSquadronHostRecord& sq) {
        auto ancestry_has = [](int id, int kind) {
            if constexpr (bsp::kSquadronAmmoDummyKindsCc13Bound) {
                // Packet cc13-ammodk: the descriptor vt[8] census; 2Dh is answered
                // by DummyKamikazePlane (006EA830) only.
                if (id == kind) return true;
                if (kind == 0x2A) {
                    return id == 0x2B || id == 0x2C || id == 0x2E || id == 0x2F || id == 0x30
                        || id == 0x31 || id == 0x33 || id == 0x34;
                }
                if (kind == 0x2D) return id == 0x2F;
                return false;
            } else {
                if (id == kind) return true;
                if (kind == 0x2A) return id == 0x2B || id == 0x2C || id == 0x31 || id == 0x33;
                if (kind == 0x2D) return id == 0x2F;
                return false;
            }
        };
        for (const std::size_t m : sq.member_units) {
            if (m == bsp::kPlaneSquadronNoUnit || m >= slots.size()) continue;
            GameUnitSlot& s = *slots[m];
            const std::vector<PlaneOrdnanceRack>& racks = ordnance_racks(s);
            // kSquadronAmmoTypeRackRoundCc13Bound: 007B91C0 walks the same rack
            // slots as 007B9140, so ON takes plane_rack_holds_round's answer.
            auto holds = [&](const PlaneOrdnanceRack& r) {
                if constexpr (bsp::kSquadronAmmoTypeRackRoundCc13Bound) {
                    return plane_rack_holds_round(s, r);
                } else {
                    return rack_holds_round(s, r);
                }
            };
            auto has = [&](int kind) {
                for (const PlaneOrdnanceRack& r : racks) {
                    if (r.class_id != 0 && holds(r) && ancestry_has(r.class_id, kind)) {
                        return true;
                    }
                }
                return false;
            };
            if (has(0x2B)) return 2;
            if (has(0x2C)) return 3;
            if (has(0x33)) return 4;
            if (has(0x31)) return 5;
            if (has(0x2F)) return 6;
            for (const PlaneOrdnanceRack& r : racks) {
                if (r.class_id == 0 || !holds(r)) continue;
                const int id = r.class_id;
                if (ancestry_has(id, 0x2A) && !ancestry_has(id, 0x2C) && !ancestry_has(id, 0x31)
                    && !ancestry_has(id, 0x2B) && !ancestry_has(id, 0x33)
                    && !ancestry_has(id, 0x2D)) {
                    return 1;
                }
            }
        }
        return 0;
    }

    // The class stand-in the AI host used before this reader (its
    // squadron_ammo_type_stand_in): TorpedoBomber 11h -> 2, DiveBomber 12h -> 1,
    // LevelBomber 10h -> 5, Kamikaze 17h -> 6, anything else 0, from the leader.
    int squadron_ammo_type_stand_in(const bsp::PlaneSquadronHostRecord& sq) const {
        const std::size_t lead = sq.flight_leader();
        if (lead >= slots.size()) return 0;
        const int c = slots[lead]->class_id;
        if (bsp::unit_is_kind_of(c, 0x11)) return 2;
        if (bsp::unit_is_kind_of(c, 0x12)) return 1;
        if (bsp::unit_is_kind_of(c, 0x10)) return 5;
        if (bsp::unit_is_kind_of(c, 0x17)) return 6;
        return 0;
    }

    // DIAGNOSTIC census: each squadron's 007EDAD0 answer, logged when it changes.
    std::vector<int> ordnance_census_last;
    void squadron_ordnance_census() {
        const auto& recs = bsp::plane_squadron_registry().records();
        if (ordnance_census_last.size() < recs.size()) ordnance_census_last.resize(recs.size(), -1);
        for (std::size_t i = 0; i < recs.size(); ++i) {
            if (recs[i].member_units.empty()) continue;
            const int a = squadron_ammo_type_007edad0(recs[i]);
            if (a == ordnance_census_last[i]) continue;
            log.notef("squadron ordnance %s: ammo type %d -> %d (stand-in %d) at %.2f s "
                "(007EDAD0, packet cc9_squadron_ordnance_state)", recs[i].name.c_str(),
                ordnance_census_last[i], a, squadron_ammo_type_stand_in(recs[i]),
                static_cast<double>(summary.simulated_seconds));
            ordnance_census_last[i] = a;
        }
    }

    // 007C1DB0 over the complete owned single-rack census, with no held-kind
    // filter. An absent answer keeps the existing host calculation below.
    std::optional<int> complete_single_rack_round_count(GameUnitSlot& unit) {
        // Same admission as the 007C1F60 binding: only the actual single-rack
        // owner whose release/reload producers maintain every counter. The
        // integer census may walk BSPGun order because this sum has no effects.
        const auto& racks = ordnance_racks(unit);
        if (!unit.rack_census_done || racks.empty() || unit.rack_multi_count != 0
            || racks.size() != unit.rack_ammo_per_rack.size()
            || racks.size() != unit.rack_identity_per_rack.size()
            || racks.size() != unit.rack_bullet_class_per_rack.size()) return std::nullopt;
        for (const auto& rack : racks) {
            if (rack.multi || rack.single_index < 0
                || static_cast<std::size_t>(rack.single_index) >= unit.rack_ammo_per_rack.size())
                return std::nullopt;
            const auto index = static_cast<std::size_t>(rack.single_index);
            const auto& resolved = unit.rack_identity_per_rack[index];
            if (resolved.projectile_kind != unit.rack_bullet_class_per_rack[index]
                || !admits_single_rack_round_census(
                    {rack.platform_key, rack.device_class, rack.class_id}, resolved,
                    unit.rack_ammo_per_rack[index])) return std::nullopt;
        }
        struct Binding final : bsp::cc13_seg47::RackRoundCountHost {
            using Raw = bsp::cc13_seg47::Raw;
            GameUnitSlot& unit;
            const std::vector<PlaneOrdnanceRack>& racks;
            Binding(GameUnitSlot& u, const std::vector<PlaneOrdnanceRack>& r)
                : unit(u), racks(r) {}
            Raw handle(const PlaneOrdnanceRack* rack) {
                return bsp::cc13_seg47::raw(const_cast<PlaneOrdnanceRack*>(rack));
            }
            Raw first_round_device(Raw) override { return handle(racks.data()); }
            Raw next_round_device(Raw device) override {
                const auto* rack = reinterpret_cast<const PlaneOrdnanceRack*>(device.p);
                const std::size_t next = static_cast<std::size_t>(rack - racks.data()) + 1;
                return handle(next < racks.size() ? &racks[next] : nullptr);
            }
            bool entity_vcall_is_kind_5c(Raw, std::int32_t kind) override {
                return kind == 0x25; // census category is BombPlatform
            }
            std::int32_t bomb_platform_round_count_006e3500(Raw device) override {
                const auto& rack = *reinterpret_cast<const PlaneOrdnanceRack*>(device.p);
                return unit.rack_ammo_per_rack[static_cast<std::size_t>(rack.single_index)];
            }
        } binding(unit, racks);
        // Original 007C1DB0 sums every kind-25h child's live remaining count;
        // unlike 007C1F60 it does not additionally ask holds(2Ah,0).
        return bsp::cc13_seg47::unit_count_remaining_ordnance_rounds_007c1db0(
            binding, bsp::cc13_seg47::raw(&unit));
    }

    // 007C1DB0: the device list at unit+48h, summing 006E3500 over every device
    // whose vtable[+5Ch] answers 25h. The gunnery host owns that list; the
    // count here is the aircraft's bomb platforms, one round each, minus what
    // it has already dropped.
    int dive_bomb_rounds_remaining(GameUnitSlot& slot) {
        if constexpr (kDiveBombCarriedRoundsBound) {
            // 007C1DB0 over the census's single racks. An aircraft whose census
            // found no single rack, or no authored Ammo, keeps the substitute.
            if (slot.rack_census_done && slot.rack_single_count > 0 &&
                slot.rack_rounds_authored > 0) {
                done("Unit::count_remaining_rounds_007c1db0", 0x007c1db0u);
                return slot.rack_ammo < 0 ? slot.rack_rounds_authored : slot.rack_ammo;
            }
        }
        if constexpr (kRackPoolRoundsRemainingBound) {
            // The same 007C1DB0 over the pool's fallback (one round per single
            // rack, 0x007C0DE3's seed). LABELLED: a MultiBombPlatform keeps the
            // substitute, its count being unread.
            if (slot.rack_census_done && slot.rack_single_count > 0 && slot.rack_multi_count == 0) {
                done("Unit::count_remaining_rounds_007c1db0", 0x007c1db0u);
                return slot.rack_ammo < 0 ? slot.rack_single_count : slot.rack_ammo;
            }
        }
        const int dropped = slot.torpedo_drops_spawned;
        const int carried = slot.dive_bomb_task_installed
            ? slot.dive_bomb_rounds_remaining + dropped
            : kDiveBombCarriedRoundsSubstitute;
        const int left = carried - dropped;
        return left > 0 ? left : 0;
    }
    // SUBSTITUTION, labelled: 006E3500's per-device round count is unread, so
    // the host gives a bomb-carrying aircraft one salvo's worth. The aimglide
    // release at 009C5777 caps its loop against this number, so it decides how
    // many bombs a single glide drop puts out.
    static constexpr int kDiveBombCarriedRoundsSubstitute = 2;

    // 009C7A80's outputs, the producer of every state-machine input. The body
    // is 009C7A80-009C7E9E and its tail 009C7C5B-009C7E9E is read for its
    // outputs only, so this is a PARTIAL binding of the rules that were read.
    void update_dive_bomb_approach(GameUnitSlot& slot, float dt,
                                   bool diving = false) {
        // 009C7A83-009C7A96: approach+C4h = dt + approach+C4h (FLD, FADD, FSTP).
        slot.db_approach_clock_c4 = static_cast<float>(
            static_cast<double>(dt) + static_cast<double>(slot.db_approach_clock_c4));
        if constexpr (kDiveAimLeadBound) {
            // 009C7A9F: 009FADA0 on approach+30h reads sub+44h = approach+74h,
            // last tick's 009C7E3C store.
            slot.hull_aim_ref.lead_projtime = slot.hull_aim_ref.projtime_44;
        }
        // 009C7AFE: approach+D1h = BSP_WeaponController_HasGeneralBombOrdnance.
        const bsp::OrdnanceKindSet set{slot.ordnance_mask};
        slot.db_has_bomb_d1 = bsp::ordnance_has_general_bomb_2ah(set) &&
                              dive_bomb_rounds_remaining(slot) > 0;
        // Read before the +398h block below consumes it (0099AEB3).
        const bool db_profile_activation = slot.db_profile_activation_pending;
        // 009C7A96: approach+ACh = ctl+398h, which the cruise profile 009C8920
        // holds at Pilot/DiveBomb/BeginAltRange/1.
        slot.db_begin_alt_ac = kPilotDiveBombBeginAltRange1;
        if constexpr (kDiveProfileDrawBound) {
            // Packet cc9_dive_profile_draw. 009C8920 runs its block only for a
            // unit with nothing at +9D8h (009C8931 007B8AD0, JE 009C8A1B): the
            // flight leader. It draws first (009C897C..009C899B: 00BD2F10(0.0,
            // [00CE5380] = 15.0), ECX = 1), adds BeginAltRange/1 (009C89A0 FADD
            // [EBP+4CCh]), then tests +38Ch, +37Ch < 0 and +3AAh before the write
            // at 009C89CE and clears +3AAh at 009C89DD. The block is the
            // squadron's ([task+404h]), so every member's 009C7A96 copy reads the
            // leader's write. LABELLED: this per-tick approach update stands for
            // the profile's call, and a plane with no squadron keeps the block on
            // its own slot.
            GameUnitSlot* sq = squadron_slot_of(slot.process_index);
            bool leader = unit_is_flight_leader_007b8ad0(slot.process_index);
            if constexpr (kDepartedWingmanTaskBlockBound) {
                // Packet cc9_air_ops_squadron_registry: a member 007F3970 removed
                // keeps its task's [task+404h] (the squadron) and its +9D8h.
                const bsp::PlaneSquadronRegistry& registry = bsp::plane_squadron_registry();
                if (registry.find_by_member_unit(slot.process_index) == nullptr) {
                    for (const bsp::PlaneSquadronHostRecord& r : registry.records()) {
                        for (std::size_t k = 0; k < r.departed_units.size(); ++k) {
                            if (r.departed_units[k] != slot.process_index) continue;
                            if (r.squadron_unit < slots.size()) sq = slots[r.squadron_unit].get();
                            leader = k < r.departed_index_9d8.size()
                                && r.departed_index_9d8[k] == 0;       // 007B8AD0
                            ++departed_block_reads;
                        }
                    }
                }
            }
            GameUnitSlot& block = sq != nullptr ? *sq : slot;
            bool profile_runs = true;
            if constexpr (kTaskProfileActivationOnlyBound) {
                // 0099AEB3: 009C8920 runs once, when the task becomes the head.
                profile_runs = slot.db_profile_activation_pending;
                slot.db_profile_activation_pending = false;
            }
            if constexpr (kDiveCruiseProfileGateBound) {
                // Packet p23_dive_profile_gate. 009C894F-009C8971, block 1 on the
                // squadron's [task+404h]+37Ch, inside the 009C8938 leader branch:
                // unless +38Dh, when +380h < 0 and +3A9h is clear, +394h =
                // tuning+4C0h and +3ADh = 1; then +3A9h = 0 either way. As at the
                // moveto (009C3650) gate, clearing sq_alt_set stands for the
                // tuning write: strike_ctl_cruising_alt_394 then answers the
                // dive tuning+4C0h for this leader's task.
                if (leader && profile_runs && !block.sq_freeze_38d) {
                    if (summary.simulated_seconds >= block.sq_timer_expiry_380
                        && !block.sq_lock_3a9) {
                        if (block.sq_alt_set) ++dive_profile_travel_alt_resets;
                        block.sq_alt_set = false;
                        block.sq_lua_alt_3ad = false;           // 009C896A +3ADh = 1
                    }
                    block.sq_lock_3a9 = false;                  // 009C8971
                }
                // 009C8A66-009C8A74: task+4BCh (approach+C4h) = 3600.0 [00CFDEB0]
                // on every profile call, for every member (past the 009C8A1B
                // join), then 009C8A7C runs 009C7A80 with dt 0.0. LABELLED: the
                // advance at the top of this function stands for the tick's own
                // 009C7A80, which follows the head-change profile call, so the
                // clock leaves this tick at 3600 + dt.
                if (profile_runs) {
                    if (slot.db_approach_clock_c4 < 3600.0f) ++dive_profile_clock_reseeds;
                    slot.db_approach_clock_c4 = static_cast<float>(
                        static_cast<double>(dt) + 3600.0);
                }
            }
            if (leader && profile_runs) {
                const float value = static_cast<float>(
                    static_cast<double>(stream_set2_draw_00bd2f10(slot,
                        slot.row.name + "#dp", 0.0f, 15.0f))
                    + static_cast<double>(kPilotDiveBombBeginAltRange1));
                ++block.sq_profile_draws;
                bool open = true;
                if (kSquadronAttackAltBound && block.sq_attack_alt_set) {
                    const double now = summary.simulated_seconds;
                    if (block.sq_freeze_38c) {
                        open = false;                           // 009C89A6 JNE
                    } else if (now >= block.sq_timer_expiry_37c && !block.sq_lock_3aa) {
                        block.sq_attack_alt_set = false;       // the script value is replaced
                    } else {
                        open = false;
                        block.sq_lock_3aa = false;              // 009C89DD
                    }
                }
                if (open) {
                    block.sq_profile_398 = value;               // 009C89CE
                    block.sq_profile_398_valid = true;
                    block.sq_lua_alt_3ad = false;               // 009C89D6 +3ADh = 1
                    ++block.sq_profile_writes;
                }
            }
            if (kSquadronAttackAltBound && block.sq_attack_alt_set) {
                slot.db_begin_alt_ac = block.sq_alt_398;
                ++block.sq_attack_alt_reads;
                block.sq_last_attack_alt = slot.db_begin_alt_ac;
            } else if (block.sq_profile_398_valid) {
                slot.db_begin_alt_ac = block.sq_profile_398;    // 009C7A96
            }
        } else if constexpr (kSquadronAttackAltBound) {
            // Packet cc9_squadron_attack_alt: 009C8920's gate on the squadron's
            // second block (009C89A6 +38Ch, 009C89B6 +37Ch < 0, 009C89BF +3AAh;
            // the write 009C89CE and the lock clear 009C89DD). LABELLED: this
            // per-tick copy stands for the profile's call, and the profile's
            // own value stays the tuning constant (its 00BD2F10 draw at 009C899B
            // is not modelled, as before).
            GameUnitSlot* sq = squadron_slot_of(slot.process_index);
            if (sq != nullptr && sq->sq_attack_alt_set) {
                const double now = summary.simulated_seconds;
                if (sq->sq_freeze_38c) {
                    slot.db_begin_alt_ac = sq->sq_alt_398;
                } else if (now >= sq->sq_timer_expiry_37c && !sq->sq_lock_3aa) {
                    sq->sq_attack_alt_set = false;   // the profile overwrites +398h
                } else {
                    sq->sq_lock_3aa = false;         // 009C89DD
                    slot.db_begin_alt_ac = sq->sq_alt_398;
                }
                ++sq->sq_attack_alt_reads;
                sq->sq_last_attack_alt = slot.db_begin_alt_ac;
            }
        }
        // 009C7A9E-009C7AB4, reconstructed. ctl+39Ch is the 9999.0f at
        // 00CE4C04 the same profile writes, so the min leaves +A8h alone below
        // about 9523 and this is a ceiling, not a decay, at these altitudes.
        // Packet cc13_natives1: 009C8920's third block, run for the flight leader
        // only (009C8931 007B8AD0, JE 009C8A1B) before its 009C7A80 call. +38Eh set:
        // skip (009C89EF). Countdown +384h expired and +3ABh clear: +39Ch = 9999
        // (009C8A0E) and +3ADh = 1 (009C8A13). Then +3ABh = 0 (009C8A17). LABELLED:
        // this per-tick update stands for the profile's call; the leader test is
        // 007B8AD0 without the departed-wingman case.
        float control_alt_39c = bsp::dive_bomb_constant::kCruisingAltitudeThird;
        if (GameUnitSlot* rb = squadron_slot_of(slot.process_index);
            rb != nullptr && rb->sq_release_alt_set) {
            // kDiveReleaseAltActivationOnlyBound: the block is 009C8920's (009C89E4-
            // 009C8A17, inside the 009C8938 leader branch), so with
            // kTaskProfileActivationOnlyBound it runs at activation only, as the
            // +398h block above does.
            const bool profile_call = !kDiveReleaseAltActivationOnlyBound
                || !kTaskProfileActivationOnlyBound || db_profile_activation;
            {   // sw3c reach, log only: the block's effect on a tick the ON arm skips.
                const bool on_call = !kTaskProfileActivationOnlyBound || db_profile_activation;
                const bool block = unit_is_flight_leader_007b8ad0(slot.process_index)
                    && !rb->sq_freeze_38e;
                const bool changes = block && (rb->sq_lock_3ab
                    || summary.simulated_seconds >= rb->sq_timer_expiry_384);
                bsp::plane_task_reach_count(bsp::g_plane_task_reach_sw3.dive_release_alt,
                                            !on_call && changes, block);
            }
            if (profile_call && unit_is_flight_leader_007b8ad0(slot.process_index)
                && !rb->sq_freeze_38e) {
                if (summary.simulated_seconds >= rb->sq_timer_expiry_384 && !rb->sq_lock_3ab) {
                    rb->sq_release_alt_set = false;
                    rb->sq_lua_alt_3ad = false;   // 009C8A13 +3ADh = 1
                }
                rb->sq_lock_3ab = false;
            }
            if (rb->sq_release_alt_set) control_alt_39c = rb->sq_alt_39c;
        }
        slot.db_dive_alt_a8 = bsp::dive_bomb_decay_dive_altitude_009c7a94(
            slot.db_dive_alt_a8, control_alt_39c);
        // 009C8A5E, the cruise profile's clamp on task+4B0h == approach+B8h.
        {
            const float wanted = kPilotDiveBombAttackDist * 1.0f;
            if (wanted > slot.db_in_range_b8) slot.db_in_range_b8 = wanted;
        }
        if (slot.command_target_plus_one == 0) {
            slot.db_in_range_d0 = false;   // 009C7B0A
            return;
        }
        const std::size_t ti = slot.command_target_plus_one - 1;
        if (ti >= slots.size()) { slot.db_in_range_d0 = false; return; }
        // Packet cc8_hull_aim_point. This was the target's ORIGIN; 009FADA0 on
        // approach+30h stores the hull point instead. On a target whose vtable
        // slot +100h is 0042D810 the helper leaves the origin in place, which
        // is what that class does.
        float tp_hull[3] = {slots[ti]->motion.position[0],
                            slots[ti]->motion.position[1],
                            slots[ti]->motion.position[2]};
        if constexpr (bsp::kDiveAimPointFreezeCc13Bound) {
            // Packet cc13-aimfreeze3: 009FADA0's death head, then 009C7B03 CMP [ESI+48h],0
            // / JZ 009C7E91: with sub+18h cleared, +D0h = 0 and the body returns.
            // approach+50h is sub+20h, so it reads the frozen height.
            (void)aim_point_freeze_009fada0(slot, slots[ti].get(), ti + 1, tp_hull, 1, &log);
            if (slot.hull_aim_ref.frozen && slot.aim_freeze_task == 1) {
                if (kFlyoverSpeedBound) slot.db_aim_point_height_50 = tp_hull[1];
                ++aim_freeze_early_outs_[1];
                slot.db_in_range_d0 = false;
                return;
            }
        } else {
            aim_point_009fada0(slot, *slots[ti], ti + 1, tp_hull, &log);
        }
        const float* const tp = tp_hull;
        // Packet cc9_flyover_speed: approach+50h is sub+20h, the aim point's
        // height that 009FADA0 stores at 009FAEF5 every tick. The enter-time
        // zero near 009C4F00 stands for the ticks before the first update.
        if (kFlyoverSpeedBound) slot.db_aim_point_height_50 = tp[1];
        // 009C7B4F-009C7B80: the planar distance, components 0 and 2 only.
        const double dx = static_cast<double>(tp[0]) -
                          static_cast<double>(slot.motion.position[0]);
        const double dz = static_cast<double>(tp[2]) -
                          static_cast<double>(slot.motion.position[2]);
        const double d2 = dx * dx + dz * dz;
        // kPc24PlaneTasksCc13Bound (cc13-pc24bear): ON rounds per op as
        // 009C7B34..009C7BD9 does, for +BCh here and +C0h below. db_aim_point_3d
        // (009C8B14, another function) keeps the double dx/dz/d2 in both arms.
        if constexpr (kPc24PlaneTasksCc13Bound) {
            slot.db_planar_bc = dive_bomb_planar_pc24(
                dive_bomb_diff_pc24(tp[0], slot.motion.position[0]),
                dive_bomb_diff_pc24(tp[2], slot.motion.position[2]));
        } else {
            slot.db_planar_bc = (d2 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                ? 0.0f : static_cast<float>(std::sqrt(d2));
        }
        // Packet cc9_hull_axis: once per (attacker, target), when the range to
        // the aim point first falls inside approach+B8h, the attackrun ->
        // fly-over hand-over range. Diagnostic only, behind the switch.
        if (kHullAimOffsetEnabled && slot.db_planar_bc < slot.db_in_range_b8 &&
            slot.hull_aim_range_printed_plus_one != ti + 1) {
            slot.hull_aim_range_printed_plus_one = ti + 1;
            hull_aim_print(log, "inrange", slot, *slots[ti], ti + 1,
                           {tp[0], tp[1], tp[2]});
            log.notef("hull_aim inrange shooter=%s planar_to_aim=%.1f "
                      "planar_to_origin=%.1f shooter_pos=(%.1f %.1f %.1f)",
                      slot.row.name.c_str(), static_cast<double>(slot.db_planar_bc),
                      std::sqrt(std::pow(static_cast<double>(slots[ti]->motion.position[0]) -
                                             slot.motion.position[0], 2) +
                                std::pow(static_cast<double>(slots[ti]->motion.position[2]) -
                                             slot.motion.position[2], 2)),
                      slot.motion.position[0], slot.motion.position[1],
                      slot.motion.position[2]);
        }
        // 009C8B14-009C8B3B, the break-off's own range. The image subtracts all
        // THREE components of `aimPoint - unitPosition` and takes 0042B2F0's
        // length; approach+BCh is planar and measured to the target entity, so
        // feeding it made the break-off fire late and low. The aim point is
        // tp_hull, 009FADA0's hull point above (the same point +BCh and +C0h
        // are measured to), so the x and z are the same two differences and
        // only the vertical term is new.
        {
            const double dy = static_cast<double>(tp[1]) -
                              static_cast<double>(slot.motion.position[1]);
            const double d3 = d2 + dy * dy;
            slot.db_aim_point_3d =
                (d3 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                    ? 0.0f : static_cast<float>(std::sqrt(d3));
        }
        // 009C7B8A-009C7BB0: pi/2 - atan2, wrapped into [0, 2pi).
        if constexpr (kPc24PlaneTasksCc13Bound) {
            slot.db_bearing_c0 = dive_bomb_bearing_pc24(
                dive_bomb_diff_pc24(tp[0], slot.motion.position[0]),
                dive_bomb_diff_pc24(tp[2], slot.motion.position[2]));
        } else {
            float b = static_cast<float>(bsp::dive_bomb_constant::kHalfPi -
                                         std::atan2(dz, dx));
            if (b < 0.0f) {
                b += static_cast<float>(bsp::dive_bomb_constant::kTwoPi);
            }
            slot.db_bearing_c0 = b;
        }
        // 009C7BFB-009C7C31, reconstructed.
        bsp::DiveBombRangeLatchInputs lin;
        lin.latched = slot.db_in_range_d0;
        lin.planar_distance = slot.db_planar_bc;
        lin.in_range_distance = slot.db_in_range_b8;
        lin.control_flag_369 = kLuaDeviceReloadEnabledBound;  // squadron ReloadEnabled, default 1 (007F2D09)
        lin.global_e17bf2 = lua_device_reload_enabled_00e17bf2();
        // 009C7C31-009C7CFE, the spent-member arm. A bomber with no bombs left
        // that is not its flight leader has its latch ANDed with "the leader is
        // within approach+B8h of this aircraft's aim point" - a second,
        // independent mechanism taking a spent squadron out of its attack, and
        // it bites in exactly the situation docs/BOMBER_AFTER_TASK.md section 10
        // is about. `[sqn+3D0h]` is member_units[0]: 0099B757 compares the unit
        // against that same dword to decide the flight lead, and 009C7C7C reads
        // it for a position at +FCh/+104h.
        lin.has_bomb_ordnance_d1 = slot.db_has_bomb_d1;
        {
            auto* const sqn = bsp::plane_squadron_registry().find_by_member_unit(
                slot.process_index);
            if (sqn != nullptr) {
                for (const std::size_t member : sqn->member_units) {
                    if (member == bsp::kPlaneSquadronNoUnit) continue;
                    lin.is_flight_leader = (member == slot.process_index);
                    if (!lin.is_flight_leader && member < slots.size()) {
                        const float* const lp = slots[member]->motion.position;
                        // SUBSTITUTION, labelled: the aim point is this host's
                        // commanded-target position, the same stand-in +BCh and
                        // +C0h run on, so this is |target.xz - leader.xz|.
                        const double lx = static_cast<double>(tp[0]) -
                                          static_cast<double>(lp[0]);
                        const double lz = static_cast<double>(tp[2]) -
                                          static_cast<double>(lp[2]);
                        // 00414C60 BSP_Vector2f_LengthWithCutoff carries the
                        // same 00CE3820 cutoff the other two sqrt guards use.
                        const double l2 = lx * lx + lz * lz;
                        lin.leader_to_aim_point =
                            (l2 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                                ? 0.0f : static_cast<float>(std::sqrt(l2));
                        lin.leader_known = true;
                    }
                    break;
                }
            }
        }
        slot.db_in_range_d0 = bsp::dive_bomb_in_range_latch_009c7c31(lin);
        // 009C7D04-009C7E33, packet cc8_dive_release item 2. approach+D8h/+DCh/
        // +E0h is rewritten HERE, every tick, not latched: see the header note on
        // dive_bomb_impact_point_009c7d71. Before the aircraft is diving and
        // before the range latch it is the raw unit position (009C7D27); from
        // then on it is the predicted bomb impact point (009C7D71). The aim
        // point is tp_hull, 009FADA0's hull point, the same point +BCh/+C0h
        // above are measured to.
        if (!diving && !slot.db_in_range_d0) {
            slot.db_run_in_origin[0] = slot.motion.position[0];
            slot.db_run_in_origin[1] = tp[1];
            slot.db_run_in_origin[2] = slot.motion.position[2];
            slot.db_impact_fall_time = 0.0f;
        } else {
            // bsp.py show 009c7a80 --asm: 009C7D71..009C7D78 passes
            // approach+48h (the resolved target) to the pilot plan at
            // approach+18h. ti names that same live target above; the
            // frozen/missing-target return has already excluded it.
            bsp::bot_task_set_owner_25c_0099b650(
                slot.plan_state.owner_25c,
                static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(slots[ti].get())));
            bsp::DiveBombImpactPointInputs ip;
            ip.unit_position[0] = slot.motion.position[0];
            ip.unit_position[1] = slot.motion.position[1];
            ip.unit_position[2] = slot.motion.position[2];
            ip.unit_velocity[0] = slot.motion.linear_velocity.x;
            ip.unit_velocity[1] = slot.motion.linear_velocity.y;
            ip.unit_velocity[2] = slot.motion.linear_velocity.z;
            ip.aim_point_y = tp[1];
            const bsp::DiveBombImpactPoint r =
                bsp::dive_bomb_impact_point_009c7d71(ip);
            slot.db_run_in_origin[0] = r.point[0];
            slot.db_run_in_origin[1] = r.point[1];
            slot.db_run_in_origin[2] = r.point[2];
            slot.db_impact_fall_time = r.fall_time;
        }
        if constexpr (kDiveAimLeadBound) {
            // 009C7D65 / 009C7E3C-009C7E85: approach+74h. approach+C8h is
            // 009C3DA0's Uniform(-row+2Ch, row+2Ch); this host draws none of
            // 009C3DA0 (FOLLOWER_ATTACK_HANDOVER section 6: the [approach+14h]
            // base is unread), so SUBSTITUTION, labelled: the draw's mean, 0.
            // A re-target re-constructs hull_aim_ref and zeroes it; the image
            // keeps the sub-object, so a re-target leads one tick late here.
            slot.hull_aim_ref.projtime_44 = bsp::dive_bomb_projtime_009c7e3c(
                diving || slot.db_in_range_d0, slot.db_impact_fall_time,
                kAimErrorDrawBound ? slot.db_aim_time_error_c8 : 0.0f);
        }
        // 009C5950/009C5960 then 009C5A40 and 009C5AF1: the aim error's range
        // and bearing are taken from that point, not from the aircraft.
        // kPc24PlaneTasksCc13Bound (cc13-pc24bear): ON rounds per op as
        // 009C5950..009C596F, 009C5A1C..009C5A4D and 009C5AB4..009C5ADB do.
        if constexpr (kPc24PlaneTasksCc13Bound) {
            const float idx = dive_bomb_diff_pc24(tp[0], slot.db_run_in_origin[0]);
            const float idz = dive_bomb_diff_pc24(tp[2], slot.db_run_in_origin[2]);
            slot.db_impact_planar_5c = dive_bomb_planar_pc24(idx, idz);
            slot.db_impact_bearing_18 = dive_bomb_bearing_pc24(idx, idz);
        } else {
            const double idx = static_cast<double>(tp[0]) -
                               static_cast<double>(slot.db_run_in_origin[0]);
            const double idz = static_cast<double>(tp[2]) -
                               static_cast<double>(slot.db_run_in_origin[2]);
            const double id2 = idx * idx + idz * idz;
            slot.db_impact_planar_5c =
                (id2 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                    ? 0.0f : static_cast<float>(std::sqrt(id2));
            float ib = static_cast<float>(bsp::dive_bomb_constant::kHalfPi -
                                          std::atan2(idz, idx));
            if (ib < 0.0f) {
                ib += static_cast<float>(bsp::dive_bomb_constant::kTwoPi);
            }
            slot.db_impact_bearing_18 = ib;
        }
        // 009C5207-009C522E then 009C52C7-009C5303: the aimglide's throw, the
        // planar distance from the aircraft to that same predicted impact
        // point. Built with the same epsilon the other two sqrt guards use
        // (00CE3820, 1e-10), which 009C52DB compares the squared sum against.
        // kPc24PlaneTasksCc13Bound (cc13-pc24bear): ON rounds per op as
        // 009C5207..009C522E and 009C52C7..009C52F8 do.
        if constexpr (kPc24PlaneTasksCc13Bound) {
            slot.db_impact_throw_14 = dive_bomb_planar_pc24(
                dive_bomb_diff_pc24(slot.db_run_in_origin[0], slot.motion.position[0]),
                dive_bomb_diff_pc24(slot.db_run_in_origin[2], slot.motion.position[2]));
        } else {
            const double wx = static_cast<double>(slot.db_run_in_origin[0]) -
                              static_cast<double>(slot.motion.position[0]);
            const double wz = static_cast<double>(slot.db_run_in_origin[2]) -
                              static_cast<double>(slot.motion.position[2]);
            const double w2 = wx * wx + wz * wz;
            slot.db_impact_throw_14 =
                (w2 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                    ? 0.0f : static_cast<float>(std::sqrt(w2));
        }
        // Packet cc9_hull_turndown, diagnostic only: one line per tick of the
        // turndown, aimdive and aimglide, every quantity relative to the fed
        // aim point `tp` so the offset itself cancels between builds. The
        // commands are the ones the state ticks wrote on the previous tick.
        if (kHullAimTrace &&
            (slot.dive_bomb_state == bsp::DiveBombState::kTurnDown ||
             slot.dive_bomb_state == bsp::DiveBombState::kAimDive ||
             slot.dive_bomb_state == bsp::DiveBombState::kAimGlide)) {
            log.notef("hull_trace %s t=%d st=%x rel=(%.2f %.2f %.2f) "
                      "vel=(%.2f %.2f %.2f) hdg=%.4f pitch=%.4f bank=%.4f "
                      "cmd_pitch=%.4f bank_tgt=%.4f hdg_tgt=%.4f "
                      "ccip_rel=(%.2f %.2f) rng=%.2f latch=%d err=%.2f "
                      "ccip_d=%.2f brg_c0=%.4f brg_18=%.4f abort=%d rel_n=%d "
                      "yaw=%.3f thr=%.3f brk=%.3f roll=%.3f aimhdg=%.4f live_roll=%.3f",
                      slot.row.name.c_str(), slot.dive_bomb_arm_ticks,
                      static_cast<unsigned>(slot.dive_bomb_state),
                      slot.motion.position[0] - tp[0], slot.motion.position[1] - tp[1],
                      slot.motion.position[2] - tp[2],
                      slot.plane_world_velocity[0], slot.plane_world_velocity[1],
                      slot.plane_world_velocity[2], slot.plane_heading_c6c,
                      slot.plane_pitch_angle_c64, slot.plane_bank_angle_c68,
                      slot.plane_commanded_pitch, slot.plan_state.bank_target_2c4,
                      slot.plan_heading_2c0,
                      slot.db_run_in_origin[0] - tp[0], slot.db_run_in_origin[2] - tp[2],
                      slot.db_planar_bc, slot.db_in_range_d0 ? 1 : 0,
                      slot.db_aim_error_last, slot.db_impact_planar_5c,
                      slot.db_bearing_c0, slot.db_impact_bearing_18,
                      slot.db_abort_fires, slot.dive_bomb_releases,
                      slot.plan_slots[bsp::kPilotSlotYaw].desired,
                      slot.plan_slots[bsp::kPilotSlotThrottle].desired,
                      slot.plan_slots[bsp::kPilotSlotAirBrake].desired,
                      static_cast<double>(slot.db_aimdive_roll_last),
                      static_cast<double>(slot.db_aim_heading_last),
                      static_cast<double>(slot.plane_live_controls[2]));
        }
    }

    bsp::DiveBombTransitionInputs dive_bomb_transition_inputs(GameUnitSlot& slot) {
        bsp::DiveBombTransitionInputs in;
        in.current = slot.dive_bomb_state;
        in.engaged.in_range_latch_4c8 = slot.db_in_range_d0;
        // squadron+370h, the attack mode 009C83F8 reads through task+404h.
        // The constant 2 makes `engaged` true for as long as a target is
        // latched, which makes BOTH of 009C83E0's return-to-approach edges
        // (009C8419-009C845E in the attacking half, 009C873E-009C8783 in the
        // other) unreachable. The real producer is the flight leader's 0099B740
        // tick, which SETS it to 1 every think; `slot.db_attack_mode_370` now
        // carries that value and is correct (docs/BOMBER_AFTER_TASK.md 10.6).
        //
        // NOT WIRED, and the measurement says why. Feeding the real mode here
        // was measured on USN04, same binary apart from this one line
        // (docs/BOMBER_AFTER_TASK.md 10.9): it does take the leader out of the
        // sea -- `movieval` loses its `plane water contact` and its 303 done
        // ticks, and the approach edge fires for the first time -- but it also
        // takes `movieval`'s releases from 2 to 0 and the mission's dive-bomber
        // water contacts from 1 to 7. With `engaged` collapsed to the latch,
        // entry into the attack waits for `d < approach+B8h` = 1100 m, and this
        // host's `moveto` approach state does not fly an attack profile, so the
        // bombers arrive low and exit aimdive to `goaway` at ~240 m instead of
        // `aimglide` at ~600 m. The gate is faithful; the approach state behind
        // it is not yet. Re-wire this when moveto/follow are real, not before.
        //
        // STILL NOT WIRED after packet cc8_dive_approach, and the new
        // measurement says the blocker has MOVED. `moveto` is now real: the arm
        // dispatches 009C18C0 for kMoveTo and it commands the glide from
        // BeginAltRange/1 above the target down to that altitude at
        // approach+B4h. Runs A2 (unwired) and B (this line reading
        // db_attack_mode_370), same binary apart from this line:
        //
        //   dive entry altitude   651 / 626 / 677 m  ->  1044 / 1040 / 1045 m
        //   aim error 009C5C9B    -24.9 / -33.1 / -18.0 m -> +12.9 / -3.7 / +17.3 m
        //   mission water contacts             1    ->  0
        //   total dive-bomb releases           5    ->  0
        //
        // So the approach defect 10.9 diagnosed is FIXED: the bombers now enter
        // the dive at the authored BeginAltRange/1 instead of 200-400 m below
        // it, all three aim errors move inside the 25 m gate, and nobody
        // ditches. What the wiring costs is the five releases, and they are not
        // lost to this change: A2's releases all came from the AIMDIVE gate
        // reached from a dive entry 350 m too low, and with the faithful entry
        // every bomber pulls out into AIMGLIDE instead -- where the release gate
        // passes ZERO times in both runs (9 and 11 aimglide rows, `passed=0`
        // and `releases=0` on every one, `rearm` blocking 343 of 344 calls).
        // The aimglide re-arm timer is frozen in this tree: only the aimdive
        // input builder counts it down (line ~1544, 009C58E9); the aimglide
        // builder below reads it and never decrements it. Packet
        // cc8_dive_entry reports fixing exactly that on agent/cc8-dive-entry,
        // which is not in this tree.
        //
        // Re-wire this once that fix is merged and B is re-run; the experiment
        // is one line and one window. docs/DIVE_BOMB_APPROACH.md section 13.
        //
        // RE-TAKEN on the merged base dd5364d6d, where approach+B8h is the
        // image's draw 2080.0 instead of main's old 1100.0, and the verdict
        // CHANGED SIGN on the criterion that mattered. Runs A2' and B', same
        // binary apart from this line:
        //
        //   total dive-bomb releases          19  ->  20   (up)
        //   movieval / #1.1 releases        2 / 2  ->  2 / 2
        //   mission water contacts             0  ->  2
        //   movieval approach_returns          -  ->  0, done_ticks=609
        //
        // The releases held and rose, but movieval NEVER leaves done, and on
        // the OLD base with R = 1100 it left done entirely (no done ticks at
        // all). The reason is this line's own gate: engaged = latch || (mode ==
        // 2 && target), the mode is 1, so engaged IS the latch, and the latch
        // disengages only past approach+B8h + 100. movieval ends at
        // approach+BCh = 494.9 m against B8h = 2080.0, so the latch can never
        // clear and 009C8483's `CMP EDI,EBX / JZ ret` parks it. Raising B8h
        // from 1100 to 2080 widened the hysteresis past the whole engagement,
        // which is what removes the benefit this feed had at R = 1100.
        //
        // So the two ditchers are downstream of that: D3A Val #1.1|.-2 had
        // ALREADY released both bombs and descends out of done (alt 274.5 ->
        // 31.3 unwired, 270.6 -> 0.0 wired - the same descent, 31 m of margin
        // in one and none in the other), and #3.1|.-2 dives with an aim error
        // of -135 m and flies in. Neither is a moveto defect; both are the
        // done-state descent and the dive aim, reached more often because the
        // latch holds.
        // FED, packet cc8_follow_enter run D, under the integrator's explicit
        // grant of 2026-09-19 (this line is the integrator's pin, not a
        // worker's). It was pinned at 2 on two earlier measurements in which
        // wiring the real squadron attack mode was a net regression
        // (docs/BOMBER_AFTER_TASK.md 10.9 and docs/DIVE_BOMB_APPROACH.md's B'
        // run): the leader sets the mode to 1 every think via
        // 0099B740 -> 007ED3F0, so `engaged` collapses to the in-range latch at
        // R = approach+B8h = 2080 m.
        //
        // Both verdicts were taken when EVERY aircraft answered 007B8AD0 as a
        // leader and flew its own moveto, with no follow state to fall back to.
        // This packet removed that premise, so the verdict was RE-TAKEN as run
        // D. It regressed again, for a third reason, and the pin STAYS. USN04,
        // 4800 mission frames, placement ON in every run:
        //
        //   run                     follow law  releases  mutual kills  deaths
        //   A  before                        0        35             4      14
        //   B  predicate fed                 0        35             0      10
        //   D  B + this line fed            32        26             0       6
        //
        // D is the first run in this chain in which the follow LAW executes at
        // all, and it holds B's gains (the four mutual torpedo kills stay gone,
        // deaths fall further). But nine releases are lost, and the state that
        // loses them is FLYABOVE. Six dive-bomber wing members end the run like
        //
        //   divebomb movieval|.-2 arm_ticks=1812 transitions=1
        //       states[follow=1546 flyabove=266] releases=0 rounds_left=2
        //
        // - one transition, all of it: they hold formation for 1100-1550 ticks,
        // leave follow into flyabove, and the run ends before the approach
        // sequence (aimdive -> turndown -> attackrun) can complete. They never
        // reach `done` either, so criterion (c) becomes untestable rather than
        // passing. The aircraft are not lost to a bad command; they are lost to
        // holding station until there is no mission left to fly.
        //
        // What that means is that this pin is NOT only propping up the missing
        // follow state, as this packet expected. Something downstream of follow
        // is too slow - most likely the flyabove arm, or the moment the member's
        // own latch is allowed to set. Feeding this line honestly needs that
        // read first; it is not blocked on the follow entry any more.
        // PACKET cc8_follow_attack. The pin STAYS, but for a different reason
        // than the two paragraphs above give, and both of those are now wrong.
        //
        // (1) "They are lost to holding station until there is no mission left
        // to fly" is FALSE. Run B has no `plane water contact` line; run D has
        // four, and two are those wing members, at alt=-1.34 and -1.66 with the
        // last flyabove altitude sample at err=-209.4 against target=210.0.
        // Of the six: two drowned, two are in a fly-over/go-away limit cycle
        // (transitions=6 with goaway=1071, and transitions=32), and only the
        // #5.1 pair is the mission window - and that pair released 0 in run B.
        //
        // (2) The pin is NOT the faithful value. docs/TORPEDO_ATTACK_MODE.md's
        // writer census gives mode 2 exactly two producers image-wide, 008A4C41
        // inside the Lua binding 008A4B10 (callsite_census 007ED430 = total 1)
        // and pilot-control message BCh; an AI-ordered bomber reaches neither,
        // so 0099B740 leaves it at 1. What the pin actually buys is `attackrun`,
        // which 009C8310 can only return when `engaged` is true with the latch
        // CLEAR - impossible at mode 1, where 009C83F8 collapses `engaged` to
        // the latch.
        //
        // (3) The blocker is THIS HOST'S placement substitution, not anything
        // downstream of follow: see the note on
        // place_wing_member_on_station_007f23a0 below.
        //
        // (4) And the pinned baseline is not safe either. Run E1 (pinned, USN04
        // at 9000 mission frames) reproduces run B and then the `done` state
        // flies everything into the sea - every dive-bomb done row descends
        // without stopping (alt 278.5 -> 0.1, 274.5 -> 0.0) and the log carries
        // SIXTEEN water contacts against run D's four. 4800 frames merely ended
        // the mission first. Dive-bomb releases pinned at 9000 are 30.
        //
        // So the pin is held here only because the change it blocks has not
        // been measured yet, not because it is right. Run E2 measures it; see
        // docs/FOLLOWER_ATTACK_HANDOVER.md sections 8 and 9.
        in.engaged.control_mode_370 = 2;
        if constexpr (kDiveAttackModeFedBound) {
            in.engaged.control_mode_370 = static_cast<int>(slot.db_attack_mode_370);
        }
        in.engaged.has_latched_target_440 = slot.command_target_plus_one != 0;
        in.entry.control_mode_370 = in.engaged.control_mode_370;
        in.entry.has_bomb_ordnance_4c9 = slot.db_has_bomb_d1;
        in.entry.control_flag_369 = kLuaDeviceReloadEnabledBound;  // squadron ReloadEnabled, default 1 (007F2D09)
        in.entry.global_e17bf2 = lua_device_reload_enabled_00e17bf2();
        in.entry.in_range_latch_4c8 = slot.db_in_range_d0;
        // FED, packet cc8_follow_enter, replacing a hardcoded `true`. The field
        // name follows the ledger's misreading; 007B8AD0 is the flight-leader
        // test (see unit_is_flight_leader_007b8ad0). 009C8419-009C845E and
        // 009C873E-009C8783 are the two `!engaged` edges and both make the same
        // choice the constructor 009C7777 made: AL != 0 -> +4F0h moveto,
        // AL == 0 -> +52Ch follow. This is what lets the host enter follow at
        // all; with the hardcode every aircraft took moveto and the follow tick
        // never ran.
        in.unit_lacks_follow_target =
            unit_is_flight_leader_007b8ad0(slot.process_index);
        {
            bsp::DiveBombBreakOffInputs b;
            b.base_0099c230 = true;
            b.has_latched_target = slot.command_target_plus_one != 0;
            b.has_bomb_ordnance_4c9 = slot.db_has_bomb_d1;
            // CORRECTED, packet cc8_dive_approach. This was db_planar_bc, which
            // is planar and measured to the target entity; 009C8B14-009C8B3B
            // measures the 3-D `aimPoint - unitPosition`. Both errors pushed the
            // same way, so the break-off used to fire later and lower than the
            // image's. docs/BOMBER_AFTER_TASK.md 10.10.
            b.distance_to_target = slot.db_aim_point_3d;
            b.speed_ratio_41c = 1.0f;
            if constexpr (kDiveBreakOffReloadFeedBound) {
                // 009C8ABE-009C8ACD: ctl+369h (ReloadEnabled, default 1) and [00E17BF2].
                b.control_flag_369 = kLuaDeviceReloadEnabledBound;
                b.global_e17bf2 = lua_device_reload_enabled_00e17bf2();
            }
            in.should_break_off = bsp::dive_bomb_should_break_off_009c8a90(b);
            // 009C8B40-009C8B51: the range arm is `[tuning+4C8h] * [task+41Ch]`
            // against the 3-D range. NOTE the id-space collision: that +4C8h is
            // Pilot/DiveBomb/SafeDist on the tuning singleton 0042E740, a
            // different object from the task's +4C8h in-range latch above.
            slot.db_dbg_break_off = in.should_break_off;
            slot.db_dbg_break_off_distance = b.distance_to_target;
            slot.db_dbg_break_off_threshold = b.safe_distance * b.speed_ratio_41c;
        }
        slot.db_dbg_engaged = bsp::dive_bomb_engaged_009c83f8(in.engaged);
        // 009C62B0 is defined and its two decisive flags are recovered, so
        // the geometry stand-in is gone. docs/DIVE_BOMB_TASK.md.
        //
        // All three flyabove flags key on ONE height: B at 009C6493, the
        // aircraft's Y less out[1] of the approach's vtable[0] (009C40A0), and
        // the same frame slot reaches 009C67C7's can-dive test. This host's aim
        // point is the commanded target's own position, so out[1] is its Y.
        {
            const float* const own_p = slot.motion.position;
            float target_p[3] = {own_p[0], own_p[1], own_p[2]};
            float target_v[3] = {0.0f, 0.0f, 0.0f};
            // Packet cc13-aimfreeze3: a latched ref answers its frozen point with or
            // without the target, and 009FA2E0 then finds sub+14h and sub+18h both null
            // and returns the vector at 00F87574 (taken as zero, LABELLED), the default
            // target_v already holds.
            bool frozen_point = false;
            if constexpr (bsp::kDiveAimPointFreezeCc13Bound) {
                if (const float* fp = aim_freeze_point(slot, slot.command_target_plus_one, 1)) {
                    for (int i = 0; i < 3; ++i) target_p[i] = fp[i];
                    frozen_point = true;
                }
            }
            if (!frozen_point && slot.command_target_plus_one != 0) {
                const std::size_t ti = slot.command_target_plus_one - 1;
                if (ti < slots.size()) {
                    const GameUnitSlot& tgt = *slots[ti];
                    target_p[0] = tgt.motion.position[0];
                    target_p[1] = tgt.motion.position[1];
                    target_p[2] = tgt.motion.position[2];
                    // 009C6342: the fly-over lead point starts from approach->vtable[0],
                    // the fed aim point (the origin while kHullAimOffsetEnabled is
                    // false). Packet cc9_hull_turndown.
                    aim_point_009fada0(slot, tgt, ti + 1, target_p);
                    // 009FA2E0 reaches vtable[34h] on the object at
                    // approach+44h, else approach+48h; a plane slot carries its
                    // world velocity separately from the rigid body.
                    // The command names a whole unit, not an interior
                    // section. 009FB1CC..009FB1D6 puts that same target in
                    // +14h only for IsKindOf(5); +18h is always the target.
                    // Keep the former path for targets whose creator has
                    // not published its native observer endpoint yet.
                    if (tgt.observer_prefix_ready) {
                        bsp::cc13_seg68::NativeApproachTargetRef ref;
                        ref.target_18 = &slots[ti]->observer_prefix.observed_00;
                        if (bsp::unit_is_kind_of(tgt.class_id, 5))
                            ref.vehicle_14 = ref.target_18;
                        GameApproachTargetVelocityHost velocity_host(tgt);
                        const auto velocity = bsp::cc13_seg68::approach_ref_target_velocity_009fa2e0(
                            velocity_host, ref);
                        target_v[0] = velocity[0];
                        target_v[2] = velocity[2];
                    } else if (tgt.plane_velocity_seeded) {
                        target_v[0] = tgt.plane_world_velocity[0];
                        target_v[2] = tgt.plane_world_velocity[2];
                    } else {
                        target_v[0] = tgt.motion.linear_velocity.x;
                        target_v[2] = tgt.motion.linear_velocity.z;
                    }
                }
            }
            const float height_above = own_p[1] - target_p[1];
            slot.db_flyabove_height = height_above;
            // 009C62D1-009C63E6, BOUND (packet cc8_dive_heading): the fly-over
            // does NOT measure its range and its bearing to the aim point. It
            // measures them to where the aim point will be relative to the
            // aircraft in THREE SECONDS.
            //
            //   009C62CF  vtable[34h] on [[ESI+4]+4]        the own velocity
            //   009C62E8  009FA2E0 on approach+30h          the target velocity
            //   009C62ED/009C6305  (dx,dz) = v_own - v_tgt
            //   009C6320/009C6328  the qword 3.0 at 00D7A2B0
            //   009C6336/009C633E  3*dx, 3*dz
            //   009C6342  vtable[0] on the approach          the aim point
            //   009C6346/009C6351  aim - 3*(dx,dz)
            //   009C635D/009C636B  less the entity pose +FCh/+104h
            //   009C6385-009C63A6  R = sqrt(x*x + z*z), the 1e-10 floor at
            //                      00CE3820 collapsing it to 0
            //   009C63BF-009C63E6  bearing = wrap(pi/2 - atan2(z,x)), the +2pi
            //                      at 00CE3828
            //
            // Integrating, in 3 s the aircraft moves 3*v_own and the aim point
            // moves 3*v_tgt, so `aim - pos - 3*(v_own - v_tgt)` is exactly the
            // predicted separation. vtable[34h] is the velocity getter, the
            // same slot 009C7D71 multiplies by 007BCC80's fall time to build
            // the predicted impact point. R and the bearing move together,
            // which is why both are bound here rather than one of them.
            const float rel_vx =
                slot.plane_world_velocity[0] - target_v[0];
            const float rel_vz =
                slot.plane_world_velocity[2] - target_v[2];
            const float lead_x = target_p[0] - 3.0f * rel_vx - own_p[0];
            const float lead_z = target_p[2] - 3.0f * rel_vz - own_p[2];
            float lead_range = 0.0f;
            if constexpr (bsp::kPc24UnitsHostCc13Bound) {
                // pc24w2-units: 009C637D FMUL ST0,ST0 (x*x), 009C637F..81 z*z,
                // 009C6383 FADDP, each at PC24, stored float at 009C6385; FCOMI
                // against double [00CE3820] 1e-10, JBE (NaN too) -> 0 at
                // 009C63B1, else 00BF7030 sqrt stored float (009C639E/009C63A6).
                namespace x8 = bsp::cc13x87;
                const float lead_d2 = static_cast<float>(
                    x8::x87_add24(x8::x87_mul24(lead_x, lead_x), x8::x87_mul24(lead_z, lead_z)));
                if (static_cast<double>(lead_d2) > bsp::dive_bomb_constant::kDistanceEpsilonSq) {
                    lead_range = static_cast<float>(x8::x87_sqrt24(lead_d2));
                }
            } else {
            const double lead_d2 = static_cast<double>(lead_x) * lead_x +
                                   static_cast<double>(lead_z) * lead_z;
            lead_range =
                (lead_d2 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                    ? 0.0f : static_cast<float>(std::sqrt(lead_d2));
            }
            float lead_bearing = 0.0f;
            if constexpr (bsp::kFlyaboveLeadBearingPc24Cc13Bound) {
                // cc13-flybearing (rev7 PC24 debt): 009C63C4 FSTP dword stores the
                // 00BF701A atan2 as a float, 009C63CC FSUBR qword [00CE3830] (pi/2,
                // the float widened) rounds at PC24, 009C63D2 FSTP dword. ABI:
                // inline in 009C62B0. FPATAN ignores PC (00BF701A is a CRT trandisp
                // stub that sets its own control word), so the image's atan2 result is
                // rounded to float once at 009C63C4. Residual: CRT double atan2 vs x87
                // FPATAN last-ulp.
                const float atan2_f = static_cast<float>(
                    std::atan2(static_cast<double>(lead_z), static_cast<double>(lead_x)));
                lead_bearing = static_cast<float>(bsp::cc13x87::x87_sub24(
                    bsp::dive_bomb_constant::kHalfPi, atan2_f));
            } else {
            lead_bearing = static_cast<float>(
                bsp::dive_bomb_constant::kHalfPi -
                std::atan2(static_cast<double>(lead_z),
                           static_cast<double>(lead_x)));
            }
            if (lead_bearing < 0.0f) {
                lead_bearing +=
                    static_cast<float>(bsp::dive_bomb_constant::kTwoPi);
            }
            slot.db_flyabove_lead_range = lead_range;
            slot.db_flyabove_lead_bearing = lead_bearing;
            const float lead_bearing_error =
                bsp::wrapped_angle_subtract_00438b10(lead_bearing,
                                                     slot.plane_heading_c6c);
            // CORRECTION, packet cc8_dive_heading, edited under the
            // integrator's hunk arbitration of 2026-09-19: 009C65DB's FSUBP
            // takes the PLANAR RANGE off the stack, not the height, so the
            // span is `max(R - S, 0)`, and `R` is the three-second lead range
            // built above, not `db_planar_bc` and not the height.
            const bsp::DiveBombFlyAboveSpan span =
                bsp::dive_bomb_flyabove_span_009c65fd(height_above, lead_range);
            slot.db_flyabove_span = span.span;
            if constexpr (bsp::dive_bomb_constant::kDiveFlyAboveCarriedFlagsBound) {
            // +18h/+19h/+1Ah are state bytes, not per-think predicates
            // (cc13-audit9, GUNNERY 192). 009C83E0 reads them at
            // 009C84FB/009C8508/009C85E8 before this think's flyabove tick
            // runs, so the transition sees what the PREVIOUS tick left; the
            // tick writes them in dive_bomb_flyabove_flags_009c659f and the
            // enter 009C6270 seeds them. The OFF arm below recomputes them
            // from the current pose: one think early, +18h on every high tick
            // (the image clears it at 009C659F and sets it only at 009C680E),
            // and `ready` beating the 009C66E7 clear.
            in.flyabove_can_dive_790 = slot.db_flyabove_can_dive_18;
            in.flyabove_ready_791 = slot.db_flyabove_ready_19;
            in.flyabove_leave_792 = slot.db_flyabove_leave_1a;
            } else {
            // +18h at 009C680E: dive once higher above the aim point than
            // approach+D4h, which is now the real 675.0 m.
            in.flyabove_can_dive_790 = bsp::dive_bomb_flyabove_can_dive_009c680e(
                height_above, slot.db_release_range_d4);
            slot.db_flyabove_can_dive_18 = in.flyabove_can_dive_790;
            // +19h at 009C67B0, BOUND. The first arm is the 1.6 rad bearing
            // test at 00CE3D48; the second is `span <= 0` (009C67A9 with
            // 009C67AE the byte 72, JC). RETRACTED by packet cc8_dive_heading:
            // "with the 0.7/200.0 pair that is height <= 666.7 m, the same gate
            // approach+D4h's 675.0 m expresses" was an artefact of feeding the
            // span the height. The second arm is `R <= 0.7 * max(B,100) + 200`,
            // a range-to-go test against a glide slope, and it has nothing to
            // do with +D4h.
            in.flyabove_ready_791 = bsp::dive_bomb_flyabove_roll_in_009c67b0(
                lead_bearing_error, span.span);
            slot.db_flyabove_ready_19 = in.flyabove_ready_791;
            // +1Ah at 009C66E3, BOUND: leave when the folded bearing error
            // beats a tolerance opening from 20 degrees at span 0 to pi at
            // approach+B4h * 0.8 - S. 009C66E7 clears +19h on the same edge,
            // which the transition rule already models by taking `ready` first.
            in.flyabove_leave_792 = bsp::dive_bomb_flyabove_leave_009c66e3(
                lead_bearing_error, span, slot.db_attack_dist_b4);
            }
            // Packet cc9_dive_modes, diagnostic only: the fly-over's roll-in
            // inputs per tick, to tell 009C67B0's two arms apart.
            if (kHullAimTrace && slot.dive_bomb_state == bsp::DiveBombState::kFlyAbove) {
                const double own_v = std::sqrt(
                    static_cast<double>(slot.plane_world_velocity[0]) * slot.plane_world_velocity[0] +
                    static_cast<double>(slot.plane_world_velocity[2]) * slot.plane_world_velocity[2]);
                log.notef("fa_trace %s t=%d h=%.1f rng=%.1f lead_R=%.1f lead_brg=%.4f hdg=%.4f "
                          "brg_err=%.4f span=%.1f thr=%.1f v_planar=%.2f tv=(%.2f %.2f) "
                          "can_dive=%d ready=%d leave=%d",
                          slot.row.name.c_str(), slot.dive_bomb_arm_ticks,
                          static_cast<double>(height_above), static_cast<double>(slot.db_planar_bc),
                          static_cast<double>(lead_range), static_cast<double>(lead_bearing),
                          static_cast<double>(slot.plane_heading_c6c),
                          static_cast<double>(lead_bearing_error),
                          static_cast<double>(span.span), static_cast<double>(span.threshold),
                          own_v, static_cast<double>(target_v[0]), static_cast<double>(target_v[2]),
                          in.flyabove_can_dive_790 ? 1 : 0, in.flyabove_ready_791 ? 1 : 0,
                          in.flyabove_leave_792 ? 1 : 0);
            }
        }
        in.flyabove_turn_side_798 = 0;
        if constexpr (kDiveTurnSideLatchBound) {
            in.flyabove_turn_side_798 = slot.db_flyabove_side_20;  // 009C8515
        }
        in.aimdive_alive_74d = slot.db_aim_alive_19;
        in.aimdive_pull_out_74c = slot.db_aim_pull_out_18;
        // 009C86B2/009C86BF. Packet cc8_dive_flyover, the ONE line the
        // integrator granted in this function under the hunk arbitration of
        // 2026-09-19: the aimglide state's +76Ch is now kept (009C57FF sets it,
        // 009C4F0C clears it) instead of being hardcoded false, which made the
        // aimglide terminal for any aircraft still holding a bomb.
        in.aimglide_pull_out_76c = slot.db_aimglide_pull_out_76c;
        // 009C7850: !HasGeneralBombOrdnance.
        // 009C7850: the typed body asks 007B9320(0) now, as the image does, over the
        // host's racks (the ordnance kinds and the rounds left, the +D1h producer's
        // test); OFF keeps the +D1h value the approach tick cached.
        in.aimglide_out_of_bombs = bsp::bot_state_dive_bomb_aim_glide_out_of_bombs_009c7850(
            [&](std::int32_t) {
                return bsp::ordnance_has_general_bomb_2ah(
                           bsp::OrdnanceKindSet{slot.ordnance_mask}) &&
                       dive_bomb_rounds_remaining(slot) > 0;
            });
        // 009C7EA0, reconstructed.
        // 009C7EA0 reads pose+C64h first and pose+C68h second. The slot's
        // own comment makes +C64h the pitch and +C68h the bank, so the pair
        // goes in offset order, not axis-name order.
        in.turndown_complete = bsp::dive_bomb_turndown_complete_009c7ea0(
            slot.plane_pitch_angle_c64, slot.plane_bank_angle_c68);
        // 009C7F00 is PARTIAL; its first rule is d = state+20h * 0.9 against
        // the planar range, and with the travel accumulator at 0 that answers
        // as soon as the aircraft has opened at all.
        // CORRECTION: this read db_glide_travel_20, the AIMGLIDE state's +20h.
        // 009C7F00 is the goaway state's own completion rule and reads the
        // goaway state's +20h; the two are different objects. Once the aimglide
        // seed was recovered as max(arg, 5.0) the conflation made goaway finish
        // on its very first tick, which is the 1-tick goaway in every run so
        // far. The goaway's own accumulator starts at zero here, so the rule
        // stays the labelled PARTIAL it was - but it is no longer fed a value
        // that belongs to another state.
        {
            bsp::DiveBombGoAwayCompleteInputs g;
            g.planar_distance_bc = slot.db_planar_bc;
            g.travel_20 = slot.db_goaway_travel_20;
            g.altitude = slot.motion.position[1];
            // ctl+398h is what approach+ACh is refreshed from every tick, so
            // this host has one value for both.
            g.cruise_altitude_398 = slot.db_begin_alt_ac;
            g.begin_altitude_ac = slot.db_begin_alt_ac;
            g.aim_point_height_50 = slot.db_aim_point_height_50;
            g.has_bomb_ordnance_d1 = slot.db_has_bomb_d1;
            g.control_flag_369 = kLuaDeviceReloadEnabledBound;  // squadron ReloadEnabled, default 1 (007F2D09)
            g.global_e17bf2 = lua_device_reload_enabled_00e17bf2();
            in.goaway_complete = bsp::dive_bomb_goaway_complete_009c7f00(g);
            if (in.goaway_complete) {
                ++slot.db_goaway_complete_ticks;
            }
        }
        in.unit_bank_c68 = slot.plane_bank_angle_c68;
        in.bank_high_00ce398c = 0.0f;
        in.bank_low_00d1fbc0 = 0.0f;
        if constexpr (kDiveTurnSideBankBound) {
            in.bank_high_00ce398c = 0.3490658700466156f;   // 00CE398C, 20 deg
            in.bank_low_00d1fbc0 = -0.3490658700466156f;   // 00D1FBC0
        }
        // The coin is drawn lazily in write_turn_direction (kDiveTurnSideCoinBound),
        // so it is drawn only when 009C7800 runs on the coin arm, as in the image.
        in.random_turn_side = 1;
        return in;
    }

    bsp::DiveBombAimDiveReleaseInputs dive_bomb_aimdive_inputs(GameUnitSlot& slot,
                                                               float dt) {
        // 009C58E9 counts state+1Ch down by dt.
        if (slot.db_aim_rearm_1c >= 0.0f) slot.db_aim_rearm_1c -= dt;
        bsp::DiveBombAimDiveReleaseInputs in;
        in.altitude = slot.motion.position[1];
        in.dive_altitude_a8 = slot.db_dive_alt_a8;
        in.rearm_timer_1c = slot.db_aim_rearm_1c;
        // SUBSTITUTION, labelled: 009C6114 draws U(0.5, 1.0) (00CE3800 0.5,
        // FLD1) from 00BD2F10 with ECX = 1 after each release and stores it
        // to state+1Ch; this host takes the low end and draws nothing. It
        // gates only a second release within one aimdive (cc13 plane-bot
        // audit 7a, GUNNERY 181.2).
        in.rearm_draw = bsp::dive_bomb_constant::kAimDiveRearmLow;
        // 009C59BA-009C5C9B, reconstructed.
        bsp::DiveBombAimErrorInputs e;
        float target_y = slot.motion.position[1];
        const float* frozen_59cd = nullptr;
        if constexpr (bsp::kDiveAimPointFreezeCc13Bound) {
            // Packet cc13-aimfreeze3: approach->vtable[0] answers the frozen sub+1Ch.
            frozen_59cd = aim_freeze_point(slot, slot.command_target_plus_one, 1);
            if (frozen_59cd != nullptr) target_y = frozen_59cd[1];
        }
        if (frozen_59cd == nullptr && slot.command_target_plus_one != 0) {
            const std::size_t ti = slot.command_target_plus_one - 1;
            if (ti < slots.size()) {
                // 009C59CD: the aimdive height [ESP+14h] is unit+100h less
                // approach->vtable[0].out[1], the fed aim point. Packet
                // cc9_hull_turndown.
                float hp[3] = {slots[ti]->motion.position[0],
                               slots[ti]->motion.position[1],
                               slots[ti]->motion.position[2]};
                aim_point_009fada0(slot, *slots[ti], ti + 1, hp);
                target_y = hp[1];
            }
        }
        e.height_above_target = slot.motion.position[1] - target_y;
        e.dive_altitude_a8 = slot.db_dive_alt_a8;
        e.begin_altitude_ac = slot.db_begin_alt_ac;
        e.aim_point_height_50 = slot.db_aim_point_height_50;
        e.lead_at_high_5c = slot.db_lead_high_5c;
        e.gain_at_high_60 = slot.db_gain_high_60;
        // 009C5AF1 is the SAME shape as the roll input at 009C5AA3: the
        // 009C4F80 heading is arg0 at [ESP] and the bearing to the target is
        // arg1 at [ESP+4], both at the base frame 0x58 (walked forward over the
        // whole body with local/x87_walk.py, no depth conflict in this range).
        // This stood as (bearing - pose+C6Ch) here, which is off by pi for the
        // whole dive because the turndown ends inverted: cos(pi) = -1 turned
        // `along_track` into MINUS the range, which is what pinned the pitch
        // command at -1.0 for all 344 aimdive ticks of local/usn04_geo2.log.
        // The image's bearing here is drawn from the approach+D8h/+E0h point
        // rather than the aircraft, and so is this host's: db_impact_bearing_18
        // (below, packet cc8_dive_release item 2). The old SUBSTITUTION label
        // was stale (cc13-s20 re-read 009C5950-009C5AF9: [ESP+40h]/[ESP+48h] =
        // vt[0] aim point - approach+D8h/+E0h, the same arithmetic as 009C7B8A).
        bsp::DiveBombAimHeadingInputs ahin;
        ahin.mid_pitch_blend = kDiveAimHeadingBlendBound;  // GUNNERY 178
        ahin.pitch_c64 = slot.plane_pitch_angle_c64;
        ahin.bank_c68 = slot.plane_bank_angle_c68;
        ahin.heading_c6c = slot.plane_heading_c6c;
        ahin.body_up_x = slot.motion.pose_row1[0];
        ahin.body_up_z = slot.motion.pose_row1[2];
        // CORRECTED, packet cc8_dive_release item 2. The aim error's two
        // geometric inputs are 009C5AF1's bearing ([ESP+18h]) and the second
        // sqrt at 009C5A40 ([ESP+5Ch]), and BOTH are measured from
        // approach+D8h/+E0h, not from the aircraft: 009C593E makes EDI the
        // approach for 009C5950/009C5960 and only 009C5966 `MOV EDI,[EBP+4]`
        // rebases it to the unit, for 009C598C/009C5999 - so the first sqrt
        // ([ESP+1Ch]) and 009C5AA3's bearing ([ESP+24h]) are the live pair and
        // this one is not. And approach+D8h is the predicted impact point the
        // approach update rewrites every tick, so this is a CCIP solution:
        // passing the live range here made the error track the range exactly.
        e.bearing_error = bsp::wrapped_angle_subtract_00438b10(
            bsp::dive_bomb_aim_heading_009c4f80(ahin), slot.db_impact_bearing_18);
        e.planar_distance = slot.db_impact_planar_5c >= 0.0f
                                ? slot.db_impact_planar_5c : slot.db_planar_bc;
        const bsp::DiveBombAimError err = bsp::dive_bomb_aim_error_009c5c9b(e);
        slot.db_aim_error_last = err.error;
        // The release gate 00CE3880 tests the magnitude, so the census keeps the
        // closest approach to it rather than only the last sample: the last one
        // is taken after the aircraft has overflown and says nothing about the
        // dive. Sampled only while the aim states own the tick.
        if (slot.dive_bomb_state == bsp::DiveBombState::kAimDive ||
            slot.dive_bomb_state == bsp::DiveBombState::kAimGlide) {
            const float mag = err.error < 0.0f ? -err.error : err.error;
            if (slot.db_aim_error_abs_min < 0.0f || mag < slot.db_aim_error_abs_min) {
                slot.db_aim_error_abs_min = mag;
                slot.db_aim_error_min_range = slot.db_planar_bc;
                slot.db_aim_error_min_alt = slot.motion.position[1];
            }
            if (slot.db_aimdive_min_range < 0.0f ||
                slot.db_planar_bc < slot.db_aimdive_min_range) {
                slot.db_aimdive_min_range = slot.db_planar_bc;
            }
            const float bearing_error = bsp::wrapped_angle_subtract_00438b10(
                slot.db_bearing_c0, slot.plane_heading_c6c);
            if (slot.db_aimdive_entry_range < 0.0f) {
                slot.db_aimdive_entry_range = slot.db_planar_bc;
                slot.db_aimdive_entry_bearing = bearing_error;
            }
            ++slot.db_aim_state_ticks;
            // One sample every 30 aim ticks, about 2.7 s.
            if (slot.db_aim_trace_samples < 12 &&
                slot.db_aim_state_ticks >= (slot.db_aim_trace_samples + 1) * 30) {
                const int i = slot.db_aim_trace_samples;
                slot.db_aim_trace_range[i] = slot.db_planar_bc;
                slot.db_aim_trace_error[i] = err.error;
                slot.db_aim_trace_bank[i] = slot.plane_bank_angle_c68;
                slot.db_aim_trace_bearing[i] = bearing_error;
                ++slot.db_aim_trace_samples;
            }
        }
        // 009C60C1 reloads [ESP+5Ch], the same slot 009C5C9B writes - but the
        // 009C5BDB gate jumps over 009C5C9B, and the last writer before it is
        // the second sqrt at 009C5A4D/009C5A58. So a dive shallower than 30
        // degrees nose-down brings the LATCHED PLANAR DISTANCE to the 25 m
        // window, not the aim error. Same condition as the steer arm below;
        // the image evaluates it once because it is one function.
        in.aim_error =
            (slot.plane_pitch_angle_c64 >
             bsp::dive_bomb_constant::kAimDiveSteepGateAngle)
                ? (slot.db_impact_planar_5c >= 0.0f ? slot.db_impact_planar_5c
                                                    : slot.db_planar_bc)
                : err.error;
        // 009C5B01-009C5B48, the dive abort: clearing +19h is what sends the
        // state to aimglide on the next transition.
        bsp::DiveBombDiveAbortInputs ab;
        ab.release_range_d4 = slot.db_release_range_d4;
        ab.aim_point_height_50 = slot.db_aim_point_height_50;
        // CORRECTED: [ESP+14h] is the height above the aim point, the same
        // quantity `e.height_above_target` above carries, not a second copy of
        // the planar range. With the range here, 009C5B3E reduced to
        // `0.3*range + 150 > range` and aborted the dive at any range under
        // 214 m - which is where usn04_geo3.log lost it, at 204.1 m.
        ab.height_above_target_14 = e.height_above_target;
        ab.aim_point_distance = slot.db_planar_bc;
        ab.unit_attitude_c64 = slot.plane_pitch_angle_c64;
        if (bsp::dive_bomb_dive_abort_009c5b43(ab)) {
            if (slot.db_abort_first_tick < 0) {
                slot.db_abort_first_tick = slot.dive_bomb_arm_ticks;
                slot.db_abort_d4 = ab.release_range_d4;
                slot.db_abort_h14 = ab.height_above_target_14;
                slot.db_abort_range = ab.aim_point_distance;
                slot.db_abort_pitch = ab.unit_attitude_c64;
            }
            ++slot.db_abort_fires;
            slot.db_aim_alive_19 = false;
            slot.db_aim_pull_out_18 = false;
        }
        return in;
    }

    bsp::DiveBombAimGlideReleaseInputs dive_bomb_aimglide_inputs(GameUnitSlot& slot,
                                                                 float dt) {
        bsp::DiveBombAimGlideReleaseInputs in;
        // 009C5188-009C51A5, packet cc8_dive_entry. The aimglide tick counts
        // state+1Ch down by its OWN dt at the top of every tick, exactly as
        // 009C58E9 does for the aimdive, and 009C519B (byte 72, JC) against the
        // 0.0f at 00D7A218 keeps it from running below zero:
        //   009c5188 MOVSS XMM0,[ESI+1Ch] / 009c518d COMISS XMM0,[00D7A218]
        //   009c519b JC 009c51a8 / 009c519d FLD [ESP+28h] / 009c51a1 FSUB dt
        //   009c51a5 FSTP [ESI+1Ch]
        // This host decremented the timer only in dive_bomb_aimdive_inputs,
        // which runs only while the state is kAimDive, so in the glide the
        // timer sat at the 0.0 its enter writes and gate 1 at 009C5689
        // (`state+1Ch < 0`) refused every call: local/entry_before.log has
        // `blocked[rearm=629 bearing=0 ceiling=1 ...] passed=0` out of 630 for
        // D3A Val #1.1, where docs/DIVE_BOMB_TASK.md's table from the glide
        // packet's own tree reads `rearm=0 bearing=523 ceiling=107`.
        if (slot.db_aim_rearm_1c >= 0.0f) slot.db_aim_rearm_1c -= dt;
        // Packet cc8_dive_glide: every slot behind 009C5689-009C5755 is now
        // traced to its producer and bound from the geometry this host already
        // keeps. The `four frame slots not traced` note is withdrawn.
        //
        // [ESP+18h], 009C5357-009C53CA. CORRECTED: this was |pitch|, and the
        // slot is not an attitude at all - it is the horizontal angle between
        // the bearing to the aim point (approach+C0h's own construction, taken
        // from the AIRCRAFT here, unlike the aimdive's, which 009C5AF1 takes
        // from the impact point) and 009C4F80's aim heading, then abs'd. The
        // wrapped subtract is even and so is the 009C5708 cosine, so the
        // argument order is immaterial to both consumers.
        in.rearm_timer_1c = slot.db_aim_rearm_1c;
        {
            bsp::DiveBombAimHeadingInputs ahin;
            ahin.mid_pitch_blend = kDiveAimHeadingBlendBound;  // GUNNERY 178
            ahin.pitch_c64 = slot.plane_pitch_angle_c64;
            ahin.bank_c68 = slot.plane_bank_angle_c68;
            ahin.heading_c6c = slot.plane_heading_c6c;
            ahin.body_up_x = slot.motion.pose_row1[0];
            ahin.body_up_z = slot.motion.pose_row1[2];
            const float be = bsp::wrapped_angle_subtract_00438b10(
                bsp::dive_bomb_aim_heading_009c4f80(ahin), slot.db_bearing_c0);
            in.bearing_error_18 = be < 0.0f ? -be : be;
            if (slot.db_glide_bearing_min < 0.0f ||
                in.bearing_error_18 < slot.db_glide_bearing_min) {
                slot.db_glide_bearing_min = in.bearing_error_18;
            }
            // Packet cc8_dive_flyover: the same [ESP+18h] the pull-out arm at
            // 009C57CA reads, kept so the tick's tail can test it.
            slot.db_glide_bearing_abs_18 = in.bearing_error_18;
        }
        // Both RECOVERED this packet; see include/bsp/dive_bomb_task.hpp. The
        // height is measured against the aim point, as 009C5281 builds it, and
        // the ceiling is row+4Ch times the drawn release altitude - 0.6 * 350.0
        // = 210.0, so with the 50.0 margin the glide release opens below 260 m.
        {
            float target_y = slot.motion.position[1];
            const float* frozen_5278 = nullptr;
            if constexpr (bsp::kDiveAimPointFreezeCc13Bound) {
                // Packet cc13-aimfreeze3: approach->vtable[0] answers the frozen sub+1Ch.
                frozen_5278 = aim_freeze_point(slot, slot.command_target_plus_one, 1);
                if (frozen_5278 != nullptr) target_y = frozen_5278[1];
            }
            if (frozen_5278 == nullptr && slot.command_target_plus_one != 0) {
                const std::size_t ti = slot.command_target_plus_one - 1;
                if (ti < slots.size()) {
                    // 009C5278: the aimglide height is taken to approach->vtable[0],
                    // the fed aim point (the origin while kHullAimOffsetEnabled is
                    // false). Packet cc9_hull_turndown.
                    float hp[3] = {slots[ti]->motion.position[0],
                                   slots[ti]->motion.position[1],
                                   slots[ti]->motion.position[2]};
                    aim_point_009fada0(slot, *slots[ti], ti + 1, hp);
                    target_y = hp[1];
                }
            }
            in.height_above_aim_point = slot.motion.position[1] - target_y;
        }
        in.glide_release_ceiling =
            dive_bomb_row(slot).new_release_mul_04c * slot.db_dive_alt_a8;
        // CORRECTED: both of these were slot.db_planar_bc, one quantity standing
        // in for two different ones, which made 009C56C2's gate `0 < 120` and
        // 009C5704's lead `range * (1 - cos)`, never negative - so the salvo at
        // 009C5777 could not fire at any altitude, angle or travel. [ESP+14h]
        // is the throw to the predicted impact point and [ESP+10h] is the range
        // to the aim point; see the header for the three-vector walk.
        in.lateral_a = slot.db_impact_throw_14;
        in.lateral_b = slot.db_planar_bc;
        in.travel_accumulator_20 = slot.db_glide_travel_20;
        // The cap is EBP, the literal 2 at 009C53DD - a proof, not the
        // kDiveBombCarriedRoundsSubstitute the two happened to share.
        in.rounds_cap = bsp::dive_bomb_constant::kGlideSalvoCap;
        in.rearm_draw = bsp::dive_bomb_constant::kAimGlideRearmLow;
        // RECOVERED, packet cc8_dive_glide - this was a labelled 0.0f, and a
        // zero here freezes the release window at the 009C4F50 seed. A scan of
        // 009C3E00-009CA000 for `[reg+0A4h]` returns exactly one writer among
        // six touches, 009C3F16 in the constructor FUN_009C3EA0:
        //   009C3EF4 MOV ECX,[ESI+8]        ; the plane class descriptor
        //   009C3EF7 FLD  [ECX+188h]        ; MaxSpd, already named in this repo
        //   009C3F00 FMUL qword [00CEFFB0]  ; 0.95
        //   009C3F16 FSTP [ESI+0A4h]
        // so approach+A4h is 0.95 * MaxSpd, set once and never rewritten. The
        // three following instructions settle approach+A8h in the same breath:
        // 009C3F1C/009C3F23 push (approach+14h)->+3Ch and ->+38h into
        // BSP_Random_UniformFloatRange and 009C3F2E stores the draw, which is
        // the authored uniform(350, 450) this host pins at 350.
        in.drift_rate_a4 = static_cast<float>(
            bsp::dive_bomb_constant::kApproachDriftScaleA4) * slot.plane_max_spd;
        return in;
    }

    // Packet cc8_dive_glide. The dive-bomb twin of release_ordnance_007bbba0
    // above, kept separate rather than sharing it: that one spawns through
    // release_ordnance_drop, which selects torpedo rows and clears kind 2Bh, and
    // a bomb needs the kind 2Ah selection instead. The bay-open command itself
    // is the same 007BBBA0 the image calls at 009C60F1 and at 009C5777.
    //
    // The point handed to the spawn is slot.db_run_in_origin, which is
    // approach+D8h/+DCh/+E0h and which 009C7A80 has already rewritten this tick
    // through 009C7D71 - the predicted impact point of a bomb released now. So
    // it is the prediction to score the round against, not a latched origin.
    void release_bomb_007bbba0(GameUnitSlot& slot, int rounds) {
        if (!slot.actuator_block_dec.channel_c.enabled) {
            ++bay_channel_disabled_requests;
            if constexpr (!kBayRequestNoForceEnableBound) {
                slot.actuator_block_dec.channel_c.enabled = true;
                slot.actuator_block_dec.channel_c.rate = 1.0f;
            }
        }
        // The bay request is raised ONCE per release even when the salvo is two
        // rounds, and that is not a shortcut. 007BBBA0 is a latch: 007BBBB2
        // leaves when channel C is already at the top, so the image's own
        // 009C5771-009C5784 loop gets one acceptance and one refusal from a
        // two-round salvo. The bay is a bay; what the image counts per round is
        // approach+2Ch, which the loop decrements whether or not 007BBBA0
        // accepted - the call's result is never tested at 009C5777.
        const bool accepted =
            bsp::ordnance_release_request_007bbba0(slot.actuator_block_dec);
        done("Unit::request_ordnance_release_007bbba0", 0x007bbba0u);
        if (accepted) {
            ++slot.db_bay_requests_accepted;
        } else {
            ++slot.db_bay_requests_refused;
        }
        // Diagnostic, both builds: the request instant per Val, so a pair can
        // compare the request with the drop. First eight requests per Val.
        if (slot.db_bomb_requests_logged < 8) {
            ++slot.db_bomb_requests_logged;
            log.notef("val bomb request: unit=%s t=%.2f rounds=%d alt=%.1f "
                "pitch=%.4f bank=%.4f state=%x",
                slot.row.name.c_str(), static_cast<double>(summary.simulated_seconds),
                rounds, static_cast<double>(slot.motion.position[1]),
                static_cast<double>(slot.plane_pitch_angle_c64),
                static_cast<double>(slot.plane_bank_angle_c68),
                static_cast<unsigned>(slot.dive_bomb_state));
        }
        // Packet cc9_release_issue_stage_vals: the spawn waits for the rack
        // (run_rack_tick_006e56f0). unit+C20h is NOT raised here: the task's
        // request_ordnance_release hook already routes each 007BBBA0 call
        // (009C60F1, 009C5777 per round) through release_ordnance_007bbba0,
        // whose 007BBC00 raise is the image's one per call. A raise here doubled
        // it (the first ON run, local\RV_ON1_4500.log of dadc05b84).
        // Only an aircraft whose census found a single rack takes this route;
        // a multi rack (26h, never busy) is not bound, so such an aircraft keeps
        // the request spawn below. The death refusal is the stage's guard.
        if constexpr (kReleaseIssueStageBound && kReleaseIssueStageValsBound) {
            if (slot.rack_census_done && slot.rack_single_count > 0) {
                for (int i = 0; i < rounds; ++i) {
                    ++slot.rack_requests_deferred;
                    record("Plane::release_spawn_deferred_to_rack", 0x006e4d50u);
                }
                return;
            }
        }
        // SUBSTITUTION, labelled, and the reason the spawn does not hang off
        // `accepted`: the image's chain from the bay to a round is 007BBBA0 ->
        // unit+C20h -> 007CE040 -> 007C0D90 -> 007EEF30 -> 007BCBE0 ->
        // unit+C58h, and the bot task's release slot at the far end is UNREAD.
        // Tying the spawn to the latch would drop the second round of every
        // salvo, which is a property of the bay, not of the ordnance. One round
        // per counted round is the smallest model that keeps the count the
        // image keeps.
        if (gunnery == nullptr) return;
        const std::size_t index = index_of_slot(slot);
        if (index >= slots.size()) return;
        // Packet cc9_plane_death_modes: the same dead-aircraft refusal as the
        // torpedo spawn (007D1314 sets unit+C3Ah; 007CEA1C / 007CEA29 refuse).
        if (kPlaneDeathModesBound && gunnery->unit_dead(index)) {
            summary.dead_releases_refused += static_cast<unsigned long long>(rounds);
            log.notef("dead release refused: unit=%s bombs=%d (007CEA1C / 007CEA29, "
                "packet cc9_plane_death_modes)", slot.row.name.c_str(), rounds);
            done("Plane::issue_block_c3a", 0x007cea1cu);
            return;
        }
        for (int i = 0; i < rounds; ++i) {
            // Packet cc8_dive_aim item 2, edited under the integrator's hunk
            // arbitration of 2026-09-19: db_impact_fall_time is 009C7D71's tf
            // as of THIS tick, so carrying it here is the one sample that is
            // provably a release-tick one.
            if (!gunnery->release_bomb_drop(index, slot.db_run_in_origin,
                                            slot.db_impact_fall_time)) break;
            ++slot.db_bombs_spawned;
            // NO decrement of dive_bomb_rounds_remaining here. The task's own
            // `spend_round` already does it, and a second one took the stock
            // from 2 to 0 on the first bomb: db_has_bomb_d1 went false, the
            // aimdive ended 15 ticks early and `movieval` dropped from
            // releases=2 to releases=1. Measured in local\bomb_spawn.log before
            // this line came out.
        }
    }

    // `rounds` is 1 for the aimdive, whose 009C60F1 raises one request per gate,
    // and r.rounds_released for the aimglide, whose 009C5771-009C5784 loop calls
    // 007BBBA0 once per round of the salvo.
    void note_dive_bomb_release(GameUnitSlot& slot, int rounds = 1) {
        ++slot.dive_bomb_releases;
        // Packet cc8_dive_glide: the release was only ever COUNTED here.
        release_bomb_007bbba0(slot, rounds);
        if (slot.db_release_alt < 0.0f) {
            slot.db_release_alt = slot.motion.position[1];
            slot.db_release_speed = slot.plane_travel_speed;
            slot.db_release_range = slot.db_planar_bc;
        }
    }

    // Packet cc9_player_bomb_release (docs/SCRIPTED_HELM.md section 14). The
    // plane screen's 006082D0 (__thiscall(screen, param), body 006082D0-00608895,
    // screen+B8h the plane): the bomb mode +BDh, then on IC_PLANE_BOMBFIRE (A8h,
    // 0060877F) with the latch +BCh clear, 007BC7A0, 007B9500 and 007BB110, and
    // message C4h (00605270 PUSH 0C4h) routed at 0060882B to BSP_Plane_HandleMessage,
    // whose 007CD0B4 arm calls 007BBBA0 with no further gate. LABELLED: the bomb
    // switch input (+1F78h/+1F74h) is taken as held and the mode as entered on an
    // earlier frame (its entry frame sets +BCh); the latch is one press per line;
    // the HUD timer +CCh and 0042ECA0 are display.
    // Borrow the same retained fields as the native bomb-mode gate. No native
    // aircraft buffer, auxiliary attitude state, or fallback tuning is made.
    class PlayerBombModeFields final : public bsp::cc13_seg47::PlaneBombModeFields {
    public:
        PlayerBombModeFields(Impl& owner, GameUnitSlot& slot)
            : owner_(owner), slot_(slot) {}
        bool bay_blocks_release() override {
            const bsp::PlaneActuatorChannel& bay = slot_.actuator_block_dec.channel_c;
            return bay.enabled && !(bay.value == 1.0f) && owner_.bay_rack_holds_round_9c3(slot_);
        }
        float bank_c68() override { return slot_.plane_bank_angle_c68; }
        float pitch_c64() override { return slot_.plane_pitch_angle_c64; }
        float height_above_surface_9b4() override { return slot_.plane_height_above_surface_9b4; }
        float level_bomb_angle_max_550() override {
            return owner_.lua.plane_globals().pilot_general_level_bomb_angle_max;
        }
        float paratrooper_min_altitude_354() override {
            return owner_.lua.plane_globals().paratrooper_drop_min_altitude;
        }
        float paratrooper_max_altitude_358() override {
            return owner_.lua.plane_globals().paratrooper_drop_max_altitude;
        }
    private:
        Impl& owner_;
        GameUnitSlot& slot_;
    };

    GameUnitsHost::PlayerBombRelease player_bomb_release_006082d0(std::size_t index) {
        GameUnitsHost::PlayerBombRelease r;
        if (index >= slots.size()) { r.reason = "no such unit"; return r; }
        GameUnitSlot& slot = *slots[index];
        if (!bsp::unit_is_kind_of(slot.class_id, 0x0F)) {
            r.reason = "not a plane (the plane screen only)";
            return r;
        }
        if (kPlaneDeathModesBound && slot.plane_death_c3a) {
            r.reason = "dead (unit+C3Ah)";
            return r;
        }
        // 006082F6 007B9140(0) and 0060830C IsKindOf(17h): a torpedo bomber
        // has no bomb mode. 007B9140's rack walk (vtable[210h](2Ah, 0)) is the
        // ordnance mask and the rack's rounds here.
        if (bsp::unit_is_kind_of(slot.class_id, 0x17)) {
            r.reason = "006082D0: a torpedo bomber (17h) has no bomb mode";
            return r;
        }
        const bsp::OrdnanceKindSet set{slot.ordnance_mask};
        bool has_paratrooper_round = false;
        {
            // 007B9140 asks the racks for 2Ah; a 31h paratrooper also answers
            // 2Ah. The old general-bomb-only test excluded its entire HUD arm.
            // Keep the other specialised ordnance outside this host's scope.
            const bool para = bsp::ordnance_has_paratrooper_31h(set);
            // Do not open the new paratrooper path on the older salvo-count
            // substitute: its actual single-rack census must own the rounds.
            if (para && (!slot.rack_census_done || slot.rack_single_count <= 0 ||
                         slot.rack_multi_count != 0 || slot.rack_rounds_authored <= 0)) {
                r.reason = "paratrooper single-rack Ammo census unavailable";
                return r;
            }
            if (para) {
                // The loadout mask retains ancestry after ammunition is spent.
                // Match kind 31h against the current rack counts, so an empty
                // paratrooper rack cannot put other ordnance through 007C4D90.
                const std::size_t n = std::min(slot.rack_bullet_class_per_rack.size(),
                                               slot.rack_ammo_per_rack.size());
                for (std::size_t i = 0; i < n; ++i) {
                    if (slot.rack_bullet_class_per_rack[i] == 0x31 &&
                        slot.rack_ammo_per_rack[i] > 0) {
                        has_paratrooper_round = true;
                        break;
                    }
                }
            }
            if ((!bsp::ordnance_has_general_bomb_2ah(set) && !has_paratrooper_round) ||
                dive_bomb_rounds_remaining(slot) <= 0) {
                r.reason = "007B9140: no bomb or paratrooper rack has rounds";
                return r;
            }
        }
        // 00608338 (unit+72Ch)->vtable[38h], free flight.
        if (slot.plane_control_mode_900 != 7) {
            r.reason = "00608338: not in free flight (unit+900h != 7)";
            return r;
        }
        bool needs_dive_gate = true;
        if (bsp::unit_is_kind_of(slot.class_id, 0x10)) {
            if (!lua.plane_globals_loaded()) {
                r.reason = "007C7750: GameTuning has not loaded";
                return r;
            }
            PlayerBombModeFields fields(*this, slot);
            // bsp.py show 006082d0 --asm: 00608357 PUSH 0, 0060835B
            // CALL 007C7750. Bomb-mode entry checks attitude without the
            // bay-door gate; the later release path checks the bay itself.
            if (!bsp::cc13_seg47::plane_level_bomb_attitude_gate_007c7750(fields, 0)) {
                r.reason = "007C7750: bank or pitch above LevelBombAngleMax";
                return r;
            }
            done("Plane::level_bomb_attitude_gate_007c7750", 0x007c7750u);
            needs_dive_gate = false;
        }
        const float pitch = slot.plane_pitch_angle_c64;
        const float bank = slot.plane_bank_angle_c68;
        if (needs_dive_gate) {
        if (lua.plane_globals_loaded() && !set.contains(0x33)) {
            const bsp::GameTuningBlock& g = lua.plane_globals();
            const bsp::cc13_seg47::PlaneDiveDropTuning tuning{
                g.pilot_general_dive_bomb_roll_angle_min,
                g.pilot_general_dive_bomb_roll_angle_max,
                g.pilot_general_dive_bomb_roll_angle_min_pitch,
                g.pilot_general_dive_bomb_roll_angle_max_pitch,
                g.pilot_general_dive_bomb_pitch_angle_min,
                g.pilot_general_dive_bomb_pitch_angle_max};
            // 00608357 PUSH 0 -> 00608362 CALL 007C7600: no bay test.
            // The actual loadout mask excludes kind 33h here, so its first
            // bomb class cannot be a rocket. Mixed rocket loadouts and an
            // unloaded tuning owner retain the existing host path below.
            if (!bsp::cc13_seg47::dive_drop_attitude_gate_007c7600(
                    pitch, bank, tuning, nullptr, nullptr)) {
                r.reason = pitch > tuning.pitch_max_568
                    ? "007C7600: pitch above DiveBombPitchAngleMax"
                    : "007C7600: bank outside the roll curve";
                return r;
            }
            done("Plane::dive_drop_attitude_gate_007c7600", 0x007c7600u);
        } else {
            // 00608367 007C7600, the same attitude gate the rack tick applies
            // (run_rack_tick_006e56f0). The bay byte unit+9C3h is not modelled.
            float pmin = 1.0471976f, pmax = 1.5707964f;
            float rmin = 1.0471976f, rmax = 1.5707964f;
            float rmin_p = -1.3962634f, rmax_p = 1.5707964f;
            if (lua.plane_globals_loaded()) {
                const bsp::GameTuningBlock& g = lua.plane_globals();
                pmin = g.pilot_general_dive_bomb_pitch_angle_min;
                pmax = g.pilot_general_dive_bomb_pitch_angle_max;
                rmin = g.pilot_general_dive_bomb_roll_angle_min;
                rmax = g.pilot_general_dive_bomb_roll_angle_max;
                rmin_p = g.pilot_general_dive_bomb_roll_angle_min_pitch;
                rmax_p = g.pilot_general_dive_bomb_roll_angle_max_pitch;
            }
            if (pitch > pmax) {
                r.reason = "007C7600: pitch above DiveBombPitchAngleMax";
                return r;
            }
            if (!(pmin > pitch) && std::fabs(bank) > bsp::clamped_interpolate_00419010(
                                       rmax_p, rmax, rmin_p, rmin, pitch)) {
                r.reason = "007C7600: bank outside the roll curve";
                return r;
            }
        }
        }
        // 007BC7A0 is the later release-site gate. Keep it outside the
        // dive-only 007C7600 scope so level bombers still reach it too.
        if (!set.contains(0x33)) {
            // The live rack-kind mask excludes every rocket here, so the
            // actual first bomb class cannot answer IsKindOf(33h). Null
            // represents that observed branch, not a fabricated +E0h.
            // The angles are this plane's flight-state +C64h/+C68h.
            if (!bsp::cc13_left1::bomb_release_attitude_gate_007bc7a0(
                    pitch, bank, nullptr)) {
                r.reason = "007BC7A0: bank above 80 deg - |pitch|";
                return r;
            }
            done("Plane::bomb_release_attitude_gate_007bc7a0", 0x007bc7a0u);
        } else {
            // A mixed paratrooper/rocket loadout can reach this older
            // path. Its first-rack descriptor +E0h is not retained here.
            if (pitch > -0.87266463f) {
                float limit = 1.3962635f - std::fabs(pitch);
                if (limit < 0.0f) limit = 0.0f;
                if (std::fabs(bank) > limit) {
                    r.reason = "007BC7A0: bank above 80 deg - |pitch|";
                    return r;
                }
            }
        }
        // 006087DC 007BB110: channel C of unit+DECh idle or at the top passes;
        // otherwise the per-slot byte unit+9C3h, which this host does not have
        // (the same omission as release_ordnance_007bbba0), so it passes.
        if (has_paratrooper_round) {
            // 00608388 and 00608806 both call 007C4D90 for kind 31h.
            // This host represents one press while already in bomb mode;
            // enforce the release-site test on the last fixed-step sample.
            if (!lua.plane_globals_loaded() || !slot.plane_height_above_surface_ready) {
                r.reason = "007C4D90: waiting for GameTuning and the 007CE90D height sample";
                return r;
            }
            PlayerBombModeFields fields(*this, slot);
            if (!bsp::cc13_seg47::plane_in_paratrooper_drop_altitude_007c4d90(fields)) {
                r.reason = "007C4D90: outside ParatrooperDrop altitude range";
                return r;
            }
            done("Plane::paratrooper_drop_altitude_007c4d90", 0x007c4d90u);
        } else {
            // 006087DC CALL 007BB110. Unlike the paratrooper arm at
            // 00608806, the newly admitted level bomber must wait for the
            // live bay latch. The older dive-bomber substitution above is
            // unchanged; its bomb-mode entry/latch remains outside this packet.
            if (!needs_dive_gate) {
                const bsp::PlaneActuatorChannel& bay = slot.actuator_block_dec.channel_c;
                if (!bsp::cc13_release_timer::unit_can_release_007bb110(
                        bay.enabled, bay.value, bay_rack_holds_round_9c3(slot))) {
                    r.reason = "007BB110: bomb bay is still opening";
                    return r;
                }
            }
            done("Unit::can_release_007bb110", 0x007bb110u);
        }
        // 0060882B -> 007CD0B4 -> 007BBBA0.
        ++slot.player_bomb_releases;
        log.notef("player bomb release: unit=%s t=%.2f alt=%.1f pitch=%.4f bank=%.4f "
            "(006082D0 -> C4h -> 007BBBA0)", slot.row.name.c_str(),
            static_cast<double>(summary.simulated_seconds),
            static_cast<double>(slot.motion.position[1]), static_cast<double>(pitch),
            static_cast<double>(bank));
        release_ordnance_007bbba0(slot);
        r.accepted = true;
        return r;
    }

    // No slot carries its own index, and the release request is rare, so the
    // lookup is a scan rather than a new field on every slot.
    std::size_t index_of_slot(const GameUnitSlot& slot) const {
        for (std::size_t i = 0; i < slots.size(); ++i) {
            if (slots[i].get() == &slot) return i;
        }
        return slots.size();
    }

    void record_slot(const char* method, const char* text) { log.unimplemented(method, text); }

    // Packet cc9_squadron_travel_alt: the slot holding a plane's squadron block, the
    // registry squadron unit (the fused leader), else the plane's own slot.
    GameUnitSlot* squadron_slot_of(std::size_t unit) {
        if (unit >= slots.size()) return nullptr;
        if (const bsp::PlaneSquadronHostRecord* r =
                bsp::plane_squadron_registry().find_by_member_unit(unit)) {
            if (r->squadron_unit < slots.size()) return slots[r->squadron_unit].get();
        }
        return slots[unit].get();
    }

    // Packet cc9_l57_ctl_cruise_cap (docs/CC13_VER_9.md finding 1, CC13_VER_11.md):
    // 009FBA88-009FBABF caps 009FBA50's command at min(Ceiling - 50, ctl+394h)
    // whenever [state+4]+0Ch, the squadron block from unit+9D4h, is non-null.
    // ctl+394h by writer: the script block 0089F550 (sq_alt_394 while sq_alt_set),
    // else the flight leader's task profile (009D4A70 tuning+430h, 009C8920
    // tuning+4C0h; both leader-only, into the shared block), else the block init
    // 007F2BE8 (tuning+53Ch). LABELLED: the leader's installed task stands for the
    // profile that last wrote, and every plane is taken to have the block.
    unsigned long long strike_ctl_cap_feeds{0};
    void apply_strike_ctl_cruise_cap_009fba9b(std::size_t unit,
                                              bsp::PlaneCruiseAltitudeInputs& cin) {
        if constexpr (bsp::kStrikeCtlCruiseCapBound) {
            const GameUnitSlot* sq = squadron_slot_of(unit);
            if (sq == nullptr) return;
            std::size_t leader = unit;
            if (const bsp::PlaneSquadronHostRecord* r =
                    bsp::plane_squadron_registry().find_by_member_unit(unit)) {
                for (const std::size_t m : r->member_units) {
                    if (m == bsp::kPlaneSquadronNoUnit || m >= slots.size()) continue;
                    leader = m;   // +3D0h[0], 007B8AD0's flight leader
                    break;
                }
            }
            const GameUnitSlot& l = *slots[leader];
            bsp::StrikeCtlCruiseInputs ctl;
            ctl.script_alt_set = sq->sq_alt_set;
            ctl.script_alt_394 = sq->sq_alt_394;
            ctl.task = l.torpedo_task_installed ? bsp::StrikeCruiseTask::kTorpedo
                : (l.dive_bomb_task_installed ? bsp::StrikeCruiseTask::kDiveBomb
                                              : bsp::StrikeCruiseTask::kNone);
            if (lua.plane_globals_loaded()) {
                const bsp::GameTuningBlock& g = lua.plane_globals();
                ctl.general_cruising_alt_53c = g.pilot_general_cruising_alt;
                ctl.dive_cruising_alt_4c0 = g.pilot_dive_bomb_cruising_alt;
                ctl.torpedo_cruising_alt_430 = g.pilot_torpedo_cruising_alt;
            }
            cin.has_squadron = true;                                 // 009FBA99 JE not taken
            cin.squadron_limit = bsp::strike_ctl_cruising_alt_394(ctl);   // 009FBA9B
            ++strike_ctl_cap_feeds;
        } else {
            (void)unit;
            (void)cin;
        }
    }

    bool is_controlled(const GameUnitSlot& slot) const {
        return controlled_bound && controlled_index < slots.size()
            && slots[controlled_index].get() == &slot;
    }
    // unit+184h as the director step (00836978/00836AAD/00836E45) and cruise
    // (009E11C8) read it: the unit's own byte under kPlayerRoleBookkeepingBound
    // (declared further down), else the controlled-unit stand-in.
    bool player_flag_0184(const GameUnitSlot& slot) const;

    // The +CCh pose block and the motion state are two views of the same rows;
    // the motion path writes them, the pose refresh reads them.
    static void publish_pose(GameUnitSlot& slot) {
        {
            double uy = slot.motion.pose_row1[1];
            if (uy > 1.0) uy = 1.0;
            if (uy < -1.0) uy = -1.0;
            const float tilt = static_cast<float>(std::acos(uy) * 57.29577951308232);
            if (tilt > slot.diag_hull_tilt_max_deg) slot.diag_hull_tilt_max_deg = tilt;
        }
        {
            // 00C33650..00C336B9 (bsp.py show 00c33650 --asm): expand the
            // SAME twelve live motion floats into the published entity matrix.
            // Native callers include 0092F993 (body -> controller pose) and
            // 00932C28 (hydrodynamics). This host stores those body rows in
            // motion, rather than a native physics allocation. The result is
            // the world/local pose consumed by rendering and subsequent ticks.
            // The existing body retains FLD/FSTP signalling-NaN handling and
            // the binary's homogeneous-column constants; there is no float
            // arithmetic added here (pc24).
            const float compact[12] = {
                slot.motion.pose_row0[0], slot.motion.pose_row0[1], slot.motion.pose_row0[2],
                slot.motion.pose_row1[0], slot.motion.pose_row1[1], slot.motion.pose_row1[2],
                slot.motion.pose_row2[0], slot.motion.pose_row2[1], slot.motion.pose_row2[2],
                slot.motion.position[0], slot.motion.position[1], slot.motion.position[2]};
            bsp::cc13_dyn1::transform_expand_3x4_to_4x4_00c33650(compact, slot.world.data());
        }
        slot.local = slot.world;
        // Motion has published new +74h/+CCh rows. Invalidate the SAME
        // +10Ch byte that 00414E10 tests, including the first publication
        // during creation; no inverse exists before that publication.
        slot.derived_valid = 0;
    }

    // Replay the represented tail of 009583C0/009584D0/009585B0. Evidence:
    // bsp.py show 009583c0 --asm, 009585b0 --asm: +6A4h is written only by
    // position/matrix setters; all three clear C8h/10Ch. Keep the produced
    // +6A4h value separate from the plane controller's finite-difference history.
    // These instances have no child PoseRefreshViews. The native +310h/0Ch
    // and D8h notifications retain the host's existing substitutions; this
    // does not claim their native callback/physics ABI. The enclosing placement
    // routines refresh C8h before their wake/formation work (008193A0/008196B0).
    static void publish_pose_set_plan(GameUnitSlot& slot,
        const bsp::gameplay_seg61_62_cc13::PoseSetPlan& plan) {
        if (plan.set_previous_6a4) {
            slot.previous_position_6a4 = std::array<float, 3>{
                slot.motion.position[0], slot.motion.position[1], slot.motion.position[2]};
        }
        publish_pose(slot);
        slot.world_valid = 0;
        slot.derived_valid = 0;
        bsp::refresh_pose_00414db0(*slot.pose);
    }

    // 009BFD70, the first half of the follow tick 009C1FD0. The plane wing's
    // formation, which is NOT the ship unit group: see docs/PLANE_FORMATION.md
    // and the header note on why 0077C8D0 cannot reach a plane.
    //
    // WHAT IS BOUND: 009BFD70 takes the squadron from `[[task+4]+0Ch]`, refuses
    // when the unit IS the flight leader (009BFD92 `CMP EAX,[ECX+4]` / JZ
    // 009BFEB1), reads the unit's own `plane+9D0h` and asks 007F23A0 for the
    // station (009BFDBE-009BFDCC). 007ED260 and 007F23A0 are reconstructed in
    // src/plane_formation.cpp from the listing.
    //
    // WHAT IS NOT, stated as a hole rather than dressed as a proof: 009BFEE0
    // (1795 instructions, 009BFEE0-009C1846, 55 calls) is the law that FLIES a
    // member to its station - the throttle, the heading and the GoodPosition /
    // WaitForHdg gates of the whole `Pilot/Follow/*` tuning block. It is not
    // reconstructed. What runs here instead PLACES the member on its station.
    // The geometry is the image's; the path by which a member reaches it is not.
    //
    // The switch below exists so that the BEFORE half of this packet's
    // measurement is the same binary as the AFTER apart from the placement: the
    // station and the wing-geometry report are produced either way. It ships
    // true.
    // Packet `cc8_follow_regimes` replaces the placement with the law itself.
    // 009BFEE0's steer-point producer is now read far enough to fly a member:
    // the LEAD-PURSUIT regime (009C107B-009C123C) end to end, the abeam regime
    // (009C15C0-009C16CF), the 009C1328 regime and the tail's altitude band,
    // all in src/plane_follow_law.cpp against 009BEE30's already-bound fly-to
    // arm.  TWO SUBSTITUTIONS remain and they are named, not hidden:
    //
    //   * the REGIME SELECTOR is an output of the unread Phase A
    //     (009C0251-009C0EE0), because BL is rewritten there - see the header.
    //     This host always takes lead pursuit, whose D -> 0 limit is
    //     `station + FollowedPointDist * U`, i.e. steering along the leader's
    //     lagged track at the station, which is formation flight.
    //   * the GOOD-POSITION gate selects 009BEE30's hold arm
    //     (009BEE56-009BF9E5, ~700 instructions, unread), so this host runs the
    //     fly-to arm unconditionally.  At d_station = 0 that arm's own ramps
    //     return the leader's speed and stationY, so this is the fly-to law's
    //     limit standing in for the hold law, not an invented hold law.
    //   CORRECTION (packet cc13-tranche2, 2026-10-28): both substitutions are
    //   closed. kFollowPhaseABound (below) reads Phase A 009C0251-009C0EE0 and
    //   takes its regime; kPlaneFollowLawBound runs 009BEE30's hold arm
    //   (009BEE56-009BF9E5) while the +85h latch is set. The text above
    //   describes the OFF paths only.
    //
    // MEASURED 2026-09-19, and the result is a NULL that must not be read as a
    // success.  With placement false and the law true, USN04 gave mean wing
    // pairwise separation 2.5 m against the placement's 213.3 m - which looks
    // like a formation holding tightly and is nothing of the kind.  The law
    // never ran: `follow law` appears ZERO times in local/after_follow_law.log,
    // and the last `plane formation geometry` line is still the spawn clump
    // (pairwise 0.00 / 3.46 / 3.46).  The reason is three comments below at the
    // `once` gate: this host NEVER ENTERS the follow state - every plane of
    // every USN04 squadron reports `states[attackrun=...]` with
    // `prepare_entries=0` - so 009C1FD0's tick, where the law is wired, does
    // not execute, while the placement that DID run was the member's first
    // step.  Turning placement off therefore removed the only thing that put a
    // member on its station and replaced it with nothing.
    //
    // So the two flags are NOT a pair to alternate yet.  Placement stays true
    // because it is the only station-keeping this host actually reaches; the
    // law stays true because it is correct where it is wired and costs nothing
    // until 009C1FD0 becomes reachable.  The blocker for a real before/after is
    // no longer the reading of 009BFEE0 - it is that nothing in this host
    // enters BotStateFollow.  See docs/HANDOFF_PLANE_FOLLOW_REGIMES.md.
    // 2026-09-26, packet cc9_placement_off: OFF by the integrator's ruling. The
    // kind-7 task (docs/PILOT_MOVETO_TASK.md parts 1-4) now puts the escort Zeros
    // in a follow state, and the placement pair there (FP_OFF against CS_ON) shows
    // the follow law holding wingmen at the 95.5 m station with deaths, releases
    // and Zero deaths in band. The image spawns the wing stacked
    // (docs/SQUADRON_SPAWN_SEATS.md section 6, dated update).
    static constexpr bool kPlaneFormationPlacementEnabled = false;
    static constexpr bool kPlaneFollowLawEnabled = true;
    // Packet cc9_plane_follow_law (docs/PLANE_FOLLOW_LAW.md section 14): the
    // image's 009C1FD0 body at all three follow seams - the torpedo follow tick
    // (009D2731 -> 009C1FD0), the dive-bomb follow tick and the dive-bomb done
    // tick (009C7278 -> 009C1FD0): 009BFD70's station and +85h latch, then
    // 009BEE30's HOLD arm (009BEE56-009BF9E5) while latched and its fly-to arm
    // otherwise. The per-tick station PLACEMENT at those seams is the OFF path.
    static constexpr bool kPlaneFollowLawBound = true;
    // Packet cc9_follow_approach_arm (docs/PLANE_FOLLOW_PHASE_A.md section 8):
    // outside the +85h latch, 009BFEE0's Phase A (009C0251-009C0EE0) picks the
    // fly-to regime - lead pursuit, abeam, or the turn-circle lead-in
    // 009C124D-009C15BB - and the lead distance, where the host flew lead
    // pursuit always; with it the tail's leaderY - 120 floor (009C1734).
    // OFF: lead pursuit always, and Phase A runs as a shadow for the
    // summary line only.
    static constexpr bool kFollowPhaseABound = true;  // ON: fidelity, PLANE_FOLLOW_PHASE_A 8.8
    // Packet cc9_follow_turbo (docs/PLANE_FOLLOW_PHASE_A.md section 9): the
    // follow command step 009BEE30 raises the pilot's turbo request plan+2E5h
    // (009BEE42, both arms; 0099B572 clears it in every think's reseed).
    // 0099BF0F copies it into the command buffer +15h (unit+A11h), 007BB8D6
    // into the turbo byte unit+9F9h at the commit, 007DC84F into ctl+4h each
    // step, and 007D9062 multiplies the thrust by tuning+330h
    // Dynamics/SpdMultipliers/TurboMultiplier.  The host had none of the
    // chain, so a wing member could never out-run its full-throttle leader.
    static constexpr bool kFollowTurboBound = true;  // ON: along error -65..-93% (PLANE_FOLLOW_PHASE_A 9.4)
    // Packet cc13-fix89 U1 (docs/CC13_FIX89.md, CC13_VERIFY15 U1): the follow
    // tick's tail 009C211D-009C232A. Every 0.6 s (+70h) while in position (+85h)
    // and the flight leader is firing (007B8EE0, leader+C40h) or the sight is
    // already held, it searches the side's contacts ahead of the LEADER's nose
    // and latches +84h / +80h and unit+914h; every tick it then sets the gun
    // controller's hold timer (approach+1Ch)+4Ch to 2.0 or 0.0. The hold arm
    // steers at the sighted point (009BF0FD-009BF295) and a held timer makes
    // 009FC7C0 fire on any target in range. False: the sight stays inactive
    // and the hold timer is left alone.
    // cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 102); reached on
    // BSM09 only (98 searches), which still wins earlier; other rows identical.
    static constexpr bool kFollowSightSearchBound = true;
    unsigned long long follow_turbo_steps{0};
    // Packet cc13-fix89 U2 (docs/CC13_FIX89.md, CC13_VERIFY15 U2): the boost
    // request plan+2E4h (the kamikaze states 009AC74C / 009ACD66) reaches cmd+14h
    // (0099BF18-0099BF1E) and 007BB6E0's throttle arm 007BB79E-007BB834: for an
    // IsKindOf(17h) plane without PilotFires (the rocket-like kamikaze, the Ohka)
    // the boost byte unit+9F8h latches while the charge unit+E84h lasts and the
    // throttle is binary, 9F8h ? 1.0 : 0.0; for any other plane a request forces
    // the throttle to 1.0 and sets 9F8h while charged and not locked out. 007D9077
    // multiplies the thrust by desc+604h TurboStrength while 9F8h is set, and
    // 007CEB31-007CEC28 drains the charge by step / TurboTime while the latched
    // +BC8h is set (lockout +E88h at empty) and otherwise recharges it by step /
    // TurboRechargingTime up to 1.0 (never for the rocket-like plane). False:
    // the request is recorded only, and every plane's throttle is analogue.
    static constexpr bool kKamikazeBoostTurboBound = true;
    unsigned long long boost_turbo_steps_{0};
    // Packet cc13-PL3: the image's operation order inside the drag 007D9140 and
    // its coefficient 007C4990. 007C4990-007C4992 forms desc+50Ch as
    // (Accel / MaxSpd) / MaxSpd (DE FA, DE F9); 007D91D2-007D91EC forms
    // KB = (1 + AirBrakeDrag * brake) * (k * coef) and 007D928D-007D9295 returns
    // ((-sgn(v) * v) * v) * KB; 007D924E-007D9266 keeps the speed unless the
    // floor compares greater, so a NaN on either side keeps the speed. Every
    // product rounds to float on the 24-bit main thread. False: the host's
    // Accel / MaxSpd^2, its left-to-right product and its NaN-to-floor max.
    static constexpr bool kPlaneDragOrderCc13Bound = true;  // ON by bundle pair (cc9-sweep1b)
    // Packet cc13-PL3: 007DA380's water arm 007DA3D0-007DA53F reads the up value
    // as the dot ((DCh * 0) + E0h) + (E4h * 0) and gates the throttled arm on
    // unit+9F0h > 0 (COMISS / JBE, so NaN fails). Finite inputs are identical
    // either way. False: E0h alone and the helper's !(thr <= 0) gate.
    static constexpr bool kWaterRateFactorsNanOrderCc13Bound = true;  // ON (cc9-sweep2b, 2026-10-28): bundle b2 pairs on d3ad11c39; see commit
    // kPlaneDeadMeatRollBound / kPlaneWreckForwardDragBound step counts.
    unsigned long long dead_meat_roll_steps{0};
    unsigned long long dead_meat_spin_steps{0};
    unsigned long long wreck_drag_steps{0};
    // kPlaneVelocityLowpassBound: filter steps, mode-2 holds read, corrections.
    unsigned long long velocity_lowpass_steps{0};
    unsigned long long velocity_lowpass_c84_reads{0};
    unsigned long long velocity_lowpass_corrections{0};
    double velocity_lowpass_correction_abs{0.0};
    // Packet p16_planner_laws.
    unsigned long long planner_mode_one_tuned{0};
    unsigned long long carrier_landing_leads{0};
    unsigned long long carrier_touchdown_brakes{0};
    unsigned long long moveto_cruise_by_ordnance{0};
    unsigned long long moveto_leg_separations{0};
    unsigned long long speed_corr_raises{0};
    unsigned long long speed_corr_above_one_thinks{0};
    mutable unsigned long long bomb_load_authority_scaled{0};
    // Packet cc9_follow_catchup_speed (docs/PLANE_FOLLOW_LAW.md section 15):
    // the fly-to arm's two class terms as the image reads them - cruise =
    // 007C47F0 (LevelFlight x StallSpd) x 0.9 at 009BFC41/009BFC49 and the
    // leader-speed floor classDesc+188h MaxSpd at 009BFC7D - and the hold
    // arm's steer point from 009BFEE0's latched path 009BFEFC-009C0021.
    // Before: max speed x 0.9, the stall speed, and the fly-to geometry.
    static constexpr bool kPlaneFollowCatchupBound = true;
    // Packet cc13-fix5 (docs/CC13_FIX5.md, finding R3 of docs/CC13_VERIFY15.md):
    // the band tail's floor as the image builds it. 009BED80 (follow Enter)
    // copies the Pilot/Follow block (009BE150, singleton+380h) and latches
    // state+88h = block+08h = singleton+388h Pilot/Follow/SafeAlt
    // (009BED9A-009BEDBB); 009C16E5-009C175A takes
    // L = max(min(leaderY + LeaderFollowAlt, state+88h), leaderY - 120.0)
    // on every path into the tail. True: state+88h = SafeAlt and the -120 bound
    // at the fly-to, hold and catch-up sites. False: state+88h = 1e30 (floor
    // leaderY + LeaderFollowAlt), -120 only with Phase A at the fly-to.
    static constexpr bool kFollowBandSafeAltFloorBound = true;  // ON 2026-10-17 (OCEAN_WAVE_FIELD 67)
    // Packet cc9_plane_follow_law_drift (docs/PLANE_FOLLOW_LAW.md section 17).
    // True: 009BFEE0's leader turn rate is 007D7DA0 (read whole) at the fly-to
    // and the hold steer point, instead of 0.0. False: 0.0.
    // Packet cc9_landing_approach_bit (docs/SQUADRON_LAND_TASK.md section 5d):
    // the block+20h bit 1 that 006C0840 tests, from its producers. False: every
    // deck gets the bit and a class 10h/16h head's resolution is flagged unread.
    static constexpr bool kLandingApproachBitBound = true;
    // Packet cc9_land_standby_state (docs/SQUADRON_LAND_TASK.md section 5e):
    // land/standby's enter/tick/exit (009B0230/009B0FE0/009B0240), the rule's
    // standby arm and 006C54C0's common tail (circle point and radius). Needs
    // kLandingSequencerBound. False: every entry to standby is refused.
    static constexpr bool kLandStandbyStateBound = true;  // ON: pairs held (docs/SQUADRON_LAND_TASK.md 5e)
    // Packet cc9_land_line_state (docs/SQUADRON_LAND_TASK.md section 5g):
    // land/line's enter/tick/exit (009B02E0/009B0300/009B02F0) and the rule's
    // line arm (a wing member at mode 2, 009B3E81). Needs kLandStandbyStateBound.
    // False: every entry to land/line is refused.
    static constexpr bool kLandLineStateBound = true;  // ON: pairs held (docs/SQUADRON_LAND_TASK.md 5g)
    // Packet cc9_land_begin_state (docs/SQUADRON_LAND_TASK.md section 5h):
    // land/begin's enter/tick (009B13E0/009B1D70 with 009B1420), the rule's
    // mode-4 arm and its begin arm, 009AFAF0's geometry (both arms) and the
    // approach update's onGround arm. final and abort stay refused. Needs
    // kLandLineStateBound. False: every entry to land/begin is refused.
    static constexpr bool kLandBeginStateBound = true;  // ON: joint pair with the direction hold (docs/SQUADRON_LAND_TASK.md 5o)
    // Packet cc9_land_begin_state: 006C3F80's k=0 mode-4 arm (006C42E9-006C4405)
    // over the airfield's launch site at block+3Ch. False: the arm is refused and
    // +8h keeps its value. docs/SQUADRON_LAND_TASK.md section 5i.
    // Packet cc9_land_final_state (docs/SQUADRON_LAND_TASK.md section 5j):
    // land/final's enter (009B1E60), its airborne tick (009B1ED0 to 009B1FE5)
    // and the rule's begin -> final and final arms. The on-ground half, park
    // and abort stay refused. Needs kLandBeginStateBound. False: final refused.
    static constexpr bool kLandFinalStateBound = true;  // ON: 5o
    // Packet cc9_land_final_direction_40 (docs/SQUADRON_LAND_TASK.md 5ac): land/final's
    // [approach+1Ch]+40h = 0.0 (009B1EED) stores the value the gun controller's cone
    // already holds (begin's 009B1DDA and 009FC7C0's tail 009FCE69 store only 0.0, and
    // 009B3DA7 is the only switch into final). True: the record is performed (done).
    static constexpr bool kLandFinalDirection40Bound = true;  // ON: 5ac
    // Packet cc9_torpedo_flight_lead (docs/TORPEDO_RELEASE_ORDERS.md 9): 0099B740's
    // leader test (0099B757 CMP EAX,[ECX+3D0h]) is per squadron, and ctl+370h lives
    // on that squadron's pilot control block. True: each torpedo task is its
    // squadron's leader when it is element 0 of its own squadron, re-read every
    // think, and the leader's mode reaches its own squadron only. False: the first
    // torpedo task installed in the mission leads, and its mode reaches every
    // torpedo task.
    static constexpr bool kTorpedoFlightLeadPerSquadronBound = true;  // ON: TORPEDO_RELEASE_ORDERS 9
    // Packet cc9_task_gun_controller_all_tasks (docs/DOGFIGHT_GUN.md 8): 009998A0's
    // slow path ticks the task gun controller 009FC7C0 (task+314h) at 00999979 after
    // EVERY task arm while unit+C24h (PilotFires) is set, not only the dogfight's.
    // The fast path (0099C270: the published unit+520h, not an AI plane) skips it.
    // True: every AI PilotFires plane flying a modelled non-dogfight task runs it.
    // False: only the dogfight arm does.
    static constexpr bool kTaskGunControllerAllTasksBound = true;  // ON: DOGFIGHT_GUN 8
    // Packet cc9_task_gun_cones (docs/DOGFIGHT_GUN.md 9): the states' stores to the gun
    // controller's cone (approach+1Ch)+40h outside dogfight: Angle_MoveTo at 009C1B2D,
    // 009C2584, 009C2831, 009BFD2B, 009C4443, 009D0AD4, 009D11BA; Angle_GoAway at
    // 009C4E2C, 009D1121. The gun tail zeroes it (009FCE69). True: the cone feeds the
    // finder and the fine-aim steer gate. False: 0 outside dogfight.
    static constexpr bool kTaskGunConeBound = true;  // ON: DOGFIGHT_GUN 9
    // Same packet: 0099A170 builds a task for every command class (009C3C40 for none),
    // so every AI PilotFires plane's bot ticks 009FC7C0. True: planes with no modelled
    // task tick it too, with cone 0. False: only the modelled tasks.
    static constexpr bool kTaskGunUntaskedPlanesBound = true;  // ON: DOGFIGHT_GUN 9
    // Routed from cc9-planes1 (docs/DIVE_BOMB_APPROACH.md 19): 009C18C0 measures
    // the planar separation from the +2Ch entity's pose ORIGIN (009C18EC-009C1913)
    // and steers at that origin (009C1B1C). True: the dive-bomb moveto tick feeds
    // that separation to 009C1850 and the glide, and heads with 009F9E40's law.
    // False: db_planar_bc and db_bearing_c0 stand in.
    // cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 102); moves
    // USN01/BSM09/USN21 dive releases, every win holds in both envs.
    static constexpr bool kDiveBombMoveToOriginBound = true;
    // Packet cc9_landing_descent_2 (docs/SQUADRON_LAND_TASK.md section 5o): the
    // timed direction hold dyn+B4h/+C0h. The land steer's arm 009B1B1C-009B1C79
    // and land/abort's clear 009B0A3B through 007C07A0; the core law's commit
    // 007DC6C5 on (plane+72Ch)->vtable[38h] (unit+900h == 7); the consumer
    // 007D88CB-007D8C5E in 007D8470; the decay 007D902F; and the 0.6 x seconds
    // term 007D9AFC in the control authority. The launch arm 007C705C (0.8 s at
    // BeginFlying) is NOT bound. False: the arms are counted and the field stays 0.
    static constexpr bool kPlaneDirectionHoldBound = true;  // ON: 5o
    // Packet cc9_plane_rate_floor, V1 (docs/CC13_GPR_48.md 8.3): 0099BF30 calls
    // 007D7A40 (dyn+C8h = 1.2f, 00CE3814) when its band repair rewrites command[1]
    // (0099C032) or command[0] (0099C0ED) and the +72Ch predicate holds (unit+900h
    // == 7); 007D81B0 subtracts the step from a positive timer at the top of
    // 007DB680 and parks it at -1.0f (00D7A260) outside free flight; 007D87C7-
    // 007D87F1 then raises the body forward velocity dyn+6Ch to dyn+CCh while the
    // timer is positive. dyn+CCh is float(007C47F0() * 1.05) (007DB2C0, double
    // 00CF2510), taken here from bot_desired_speed_007c47f0 at the step rather than
    // stored at spawn (SUBSTITUTION, labelled; the inputs are per class). False: no
    // timer and no floor, as before. 0099B1B0, the other caller, has no reference.
    static constexpr bool kPlaneSpeedFloorBound = true;  // ON: CC13_GPR_48 9
    // Packet cc9_plane_rate_floor, R1: 007DA710's crossing floor per axis when
    // 007DA380's byte is set: pitch (|w|*1.5)*sp, yaw |w|*sp, roll none, with sp =
    // Interp(StallRangeMin,1,StallRangeMax,0, fwd/StallSpd) x Interp(StallOffPitch,
    // 1,StallOnPitch,0, unit+C64h) (007DAABD-007DAB36). False: 1.5|w| on all three.
    static constexpr bool kPlaneRateLawCrossingFloorBound = true;  // ON: CC13_GPR_48 9
    // Packet cc9_plane_bank_yaw, D2 (docs/CC13_GPR_48.md 7.2, 10): 007D9F60's
    // bank-driven yaw 007DA00C-007DA117, between the axis-angle rotation and the
    // levelling. angle = -(sin(unit+C68h) * cos(unit+C64h) * desc+1C8h TurnRollSpd
    // * Interp(3.0 00CE3854, 0, 5.0 00CE3850, 1, ctl+7Ch) * difficulty * t), built
    // by 00B646E0 and post-multiplied by 00413920. ctl+7Ch is the body forward
    // speed (007D80C0). difficulty is 008E6430(0Dh, unit) behind [00E0C978] and
    // [[00F88C30]+124h], else 1.0f (00D7A24C); this host's modifier list is empty,
    // so 1.0f. False: no bank yaw, as before.
    static constexpr bool kPlaneBankYawBound = true;
    // Packet cc9_plane_deck_clamp, D1 and V4 (docs/CC13_GPR_48.md 7.2, 8.3, 10):
    // 007DA2B1-007DA338, the tail of 007D9F60. With ctl+FCh == 1 (the ground
    // law), unit+BF4h set and a parent, h = 006BC530's local height of the
    // translated position over the holder, d = float(h - (desc+1FCh WheelHeight
    // + 0.01 [00D7A358, double])); when 0 > d the position's y gains -d and
    // 007D7D70 sets dyn+98h..A0h = (0,1,0) and dyn+A4h = 1. The next integrate
    // then zeroes body vy at |vz| < 1e-5 (007D87F6) and projects the world
    // velocity off the normal (007D8E28) with dyn+A4h set even when dyn+94h is 0,
    // and clears it. SUBSTITUTIONS, labelled: the parent (unit+3Ch) is taken as
    // present whenever the holder is, and the lift is applied to world y (the
    // image lifts the parent-local y; equal for a level deck). False: no clamp,
    // and the contact terms key on dyn+94h > 0 alone, as before.
    static constexpr bool kPlaneDeckClampBound = true;
    // Packet cc9_plane_pose_first, D4 (docs/CC13_GPR_48.md 7.2, 10): the order
    // within a step. The image runs the +4h pose slot 007D9F60 (rotate with
    // ctl+24h, bank yaw, level with ctl+8Ch, translate with ctl+18h and unit+810h)
    // in wave 1, before the +8h physics 007DB680 in wave 3, so the pose moves with
    // the previous step's velocities and the physics runs on the moved pose. True:
    // the core law starts with that pose step, using the velocity, forward speed
    // (ctl+7Ch) and levelling mode the previous core law left, and does not move
    // the pose at its end. SUBSTITUTION, labelled: the step runs at the head of
    // the core law, after the arm's attitude refresh, think and water sample,
    // which in the image read the moved pose. False: physics first, then the
    // pose with the new velocities, as before.
    static constexpr bool kPlanePoseFirstBound = true;
    // Packet cc9_plane_wreck_roll, R4 (docs/CC13_GPR_48.md 8.3, docs/OCEAN_WAVE_FIELD.md
    // 27): 007DA7E4-007DA8D5, the DeadMeat roll terms of the rate law. For a dying
    // plane (unit+5Dh, here plane_death_c3a) the roll target is multiplied by
    // 00419010(0, 1, RollMulTime, RollMul, C3Ch); a spinning one (C36h) then gains
    // +/- min(4*C3Ch, 1) * SpinRollSpd * max(0.35, RollSpd), plus for rightspinning
    // (C37h). SUBSTITUTION, labelled: C37h needs the model's wing node
    // ((unit+360h)+160h), taken as present. False: the target ignores the death.
    static constexpr bool kPlaneDeadMeatRollBound = true;
    // Packet cc9_plane_wreck_roll, C1: 007DBC77-007DBD34 in the drag path. With
    // ctl+94h and unit+C3Ch > 1.0f, dyn+0Ch -= Interp(-8, 10, -1, 1, alt) * fwd *
    // min(1, C3Ch - 1). SUBSTITUTION, labelled: ctl+94h is the host's water-step
    // latch (007CBBE7); the in-air producer 007CE0DB-007CE1D9 (a point of the
    // class list desc+5BCh below y -1 under 15 m) is not carried. False: no term.
    static constexpr bool kPlaneWreckForwardDragBound = true;
    // Packet cc9_plane_velocity_lowpass, C2 (docs/CC13_GPR_48.md 8.3,
    // docs/OCEAN_WAVE_FIELD.md 28). 007DC743-007DC80B: at the core law's tail,
    // ctl+30h = ((0.25 - step) * ctl+30h + step * ctl+18h) * 4 when step < 0.25,
    // else ctl+18h; 007D9CE0, 007DB1F0 and 007DB2C0 copy ctl+18h into it. Two
    // readers: 007C18B0 sets unit+C84h = atan2(lp.y, |lp.xz|) when |ctl+18h| > 0.1
    // (double 00D7A3A0), which pitch mode 2 holds (0099DD0A); and 0099DA97-0099DAC8
    // subtracts (unit+B1Ch - |unit+AE0h|) * 20.0 * pending from the throttle error,
    // B1Ch being ctl+6Ch = |ctl+18h| (007D807C). SUBSTITUTIONS, labelled: the
    // filter seeds from the velocity at a slot's first core step (the image seeds
    // at spawn and at 007DB1F0's control-state message); the third reader,
    // 0099D558's |B1Ch - |AE0h|| into [ESP+48h], is not carried. False: the
    // live velocity in both readers and no correction, as before.
    static constexpr bool kPlaneVelocityLowpassBound = true;
    // Packet cc9_plane_ground_roll (docs/SQUADRON_LAND_TASK.md section 5q): the
    // ground-roll arm 007CBFA0 for unit+900h 4 and 5: the pilot pass (0099AE5F
    // admits 4..7), the probe 007C5AC0, and 007DCCF0: the core law in mode 1
    // with dyn+94h, the ground band 007DBEB3-007DC200, the landed hold-down
    // 007D8CE9 and the contact projection 007D8E28. False: a landed plane is
    // held where it touched down and its land task stops ticking.
    static constexpr bool kPlaneGroundRollBound = true;  // ON: stops on the runway (5q)
    // Packet cc9_plane_water_law (docs/WATER_SURFACE_LAW.md sections 10-11): the
    // state-6 arm 007CBA50 with the water law 007DCDD0, the core law's water band
    // 007DC205 and 007DA380's mode-2 factors. False: a state-6 aircraft is frozen
    // where it touched the water.
    static constexpr bool kPlaneWaterSurfaceLawBound = true;  // ON: USN01 / ESMP08 gameplay identical
    // Packet cc9_plane_water_law (docs/WATER_SURFACE_LAW.md section 13): the depth kill
    // 007CE313-007CE3AC runs at the top of every tick in every state, so a sinking wreck
    // reaches 00926D90 BSP_MissionEntity_Kill(unit, 1) at -30 m. A shot-down plane has
    // +5Fh clear (only the explosion path 007D0CFD kills it), so Kill sets +5Fh and queues
    // it on 00F899B4 for the on-killed dispatch: the wreck leaves the world, as an
    // exploded plane does here (plane_death_removed). False: the wreck sinks without limit.
    static constexpr bool kPlaneWreckDepthRemovalBound = true;  // ON: ESMP08 gameplay identical
    // Packet cc9_leader_turn_roll: 0099DF8B-0099DFAA picks TurnRollLeader (class+260h)
    // when 007B95E0 says the plane leads a squadron with active wingmen, else
    // TurnRoll (+25Ch). False: TurnRoll for every aircraft.
    static constexpr bool kPlaneLeaderTurnRollBound = true;  // ON by ruling (a): faithful; USN01 re-plan
    // Packet cc9_wing_fold (docs/PLANE_INPUT_CHECKS_CC9.md, "the binding"). True:
    // the +DECh block's channel +44h (PartAnims.Wings, the host's channel_a) is
    // enabled per class as 007EABC0 does (class+5E4h, rate 1/class+5E0h, value 0),
    // its target follows 007C11E0 (state 4..7 spread), the ground roll 007CBFA0
    // extends it off the deck and folds it (007CC18E) while the holder's site
    // holds the plane (006CFFC0: 20.0 > 006CFE90 on a carrier), the elevator carry
    // zeroes it (006FC15E) and the elevator takes a landed plane only at value
    // 0.0 (006D06E8, 007B8D40). False: the channel stays disabled and 007B8D40
    // answers true, as before.
    static constexpr bool kPlaneWingFoldElevatorBound = true;
    // Packet cc9_wing_fold, the takeoff half (requires the switch above): the
    // takeoff prepare step holds while the wings are not at 1.0 (009CDEED) and
    // the takeoff run extends them (009CF2F8) and runs its aligned arm only once
    // they are (009CF2ED). False: 007B8D10 answers true in both, as before.
    static constexpr bool kPlaneWingSpreadTakeoffBound = true;
    unsigned long long wing_fold_classes{0}, wing_fold_extends{0}, wing_fold_retracts{0};
    unsigned long long wing_fold_resets{0}, wing_fold_elevator_waits{0};
    unsigned long long wing_spread_prep_holds{0}, wing_spread_run_holds{0};
    // Packet cc15-act1: the +DECh block's +44h channel (007B8D10 reads [plane+DECh]+44h)
    // runs band_007a_cc13.cpp's 007B8D10/007B8D40, 007B8DA0/007B8DC0 and 007B8DE0 over
    // a copy of the channel, and the dying-plane byte runs gameplay_seg48_cc13.cpp's
    // 007D8180. Byte and UCOMISS logic only: no evaluation width changes.
    static constexpr bool kActuatorFlagCc13BodiesBound = true;  // ON 2026-10-18 (OCEAN_WAVE_FIELD 75)
    static bsp::band_007a_cc13::ActuatorChannel cc13_channel(const bsp::PlaneActuatorChannel& c) {
        return {static_cast<std::uint8_t>(c.enabled), static_cast<std::uint8_t>(c.target), c.value,
                static_cast<std::uint8_t>(c.moving)};
    }
    static void store_cc13_channel(bsp::PlaneActuatorBlock& b,
                                   const bsp::band_007a_cc13::ActuatorChannel& c,
                                   std::uint8_t block_dirty) {
        b.channel_a.target = c.target != 0;
        b.channel_a.value = c.value;
        b.channel_a.moving = c.dirty != 0;
        b.moving_any = block_dirty != 0;
    }
    // 007B8DC0 / 007B8DA0: the target to 1 / 0, the moving byte and +11h raised.
    static void wing_target_007b8dc0(bsp::PlaneActuatorBlock& b, bool spread) {
        if constexpr (kActuatorFlagCc13BodiesBound) {
            bsp::band_007a_cc13::ActuatorChannel c = cc13_channel(b.channel_a);
            std::uint8_t dirty = static_cast<std::uint8_t>(b.moving_any);
            bsp::band_007a_cc13::actuator_channel_set_target_007b8da0(c, spread ? 1 : 0, dirty);
            store_cc13_channel(b, c, dirty);
        } else {
            bsp::PlaneActuatorChannel& c = b.channel_a;
            if (c.target != spread) {
                c.target = spread;
                c.moving = true;
            }
            if (c.moving) b.moving_any = true;
        }
    }
    // 007B8D10 / 007B8D40: true when the channel is disabled or its value is
    // exactly 1.0 / 0.0 (UCOMISS, so a NaN answers false).
    static bool wing_at_007b8d10(const bsp::PlaneActuatorBlock& b, float end) {
        if constexpr (kActuatorFlagCc13BodiesBound) {
            return bsp::band_007a_cc13::actuator_channel_at_value_007b8d10(cc13_channel(b.channel_a), end);
        } else {
            return !b.channel_a.enabled || b.channel_a.value == end;
        }
    }
    void wing_fold_class_007eabc0(GameUnitSlot& u);
    void wing_fold_notify_007c11e0(GameUnitSlot& u);
    // Packet cc13-landc (kPlaneFlightStateNotifyCc13Bound): 007C11E0(0) itself,
    // at the transition sites that call it, on the state already stored.
    void flight_state_notify_007c11e0(GameUnitSlot& p);
    bool wing_fold_site_holds_006cffc0(const GameUnitSlot& u) const;
    // Packet cc9_plane_touchdown (docs/SQUADRON_LAND_TASK.md section 5k): the
    // free-flight arm's site probe 007C5AC0 (006C0840 with its key, 007B8E80,
    // 006BC530 into plane+BF8h/+BFCh), the touchdown test 007CC440-007CC4CA and
    // the C5h -> 007C71E0 -> 007C1570(4) -> C3h -> 007CB5F0 chain into state 4,
    // with the site's occupancy add 006CED90 and restamp 006CE230. False: no
    // probe, plane+BF8h stays clear and a plane never leaves state 7 by landing.
    static constexpr bool kPlaneTouchdownBound = true;  // ON: gameplay identical, contact exercised (docs/SQUADRON_LAND_TASK.md 5k, 5l)
    // Packet cc9_land_abort_state (docs/SQUADRON_LAND_TASK.md section 5m):
    // land/abort's enter/exit/tick (009B0980/009B09A0/009B09C0, the airborne
    // arm), the rule's abort arm (+66Ch -> standby) and the begin/final -> abort
    // edges. Reached only with kLandBeginStateBound. False: entries refused.
    static constexpr bool kLandAbortStateBound = true;  // ON: 5o
    // Packet cc9_squadron_ordnance_state: squadron_ammo_type_007edad0 answers
    // 007EDAD0 from the planes' racks (the kind each carries, rounds left), and
    // a census logs every change. False: it answers the leader-class stand-in
    // the AI host used, and nothing is logged.
    static constexpr bool kSquadronOrdnanceReaderBound = true;  // ON: census held, gameplay identical (docs/SQUADRON_ORDNANCE_STATE.md 4)
    static constexpr bool kLandingSiteSpacingBound = true;  // ON: pair gameplay identical (docs/SQUADRON_LAND_TASK.md 5i)
    // Packet cc13-carrierdeck (G2). 006C43CB-006C43D3 (reached only when site >=
    // 0.4 FDT, 006C43C9 JA): the k=0 mode-4 arm's (block+3Ch)->vtable[30h]
    // is the mother-ship site's 006D0530 BSP_MotherShipSite_HasFreeRunway (vtable
    // 00CF8A58 +30h) on a carrier deck: false when an occupant of site+34h has
    // 006BCC90's z below Pilot/Landing/MinFreeRunwayLength (tuning+520h). True: a
    // carrier deck answers 006D0530. False: true, the airfield's 006CF3F0, as before.
    static constexpr bool kCarrierSiteHasFreeRunwayCc13Bound = true;
    static_assert(!kCarrierSiteHasFreeRunwayCc13Bound ||
        (kLandingSiteSpacingBound && kCarrierLandingDeckBound),
        "006D0530 needs the site spacing arm and the carrier holder");
    // Packet cc9_p12_queue_compact (docs/OCEAN_WAVE_FIELD.md section 39): 006C77E0,
    // 006CDC70's compaction of the landing records at block+A8h. False: a record
    // stays until its squadron head is released (006C7540), so a dead plane's
    // record keeps its last mode and path and holds the survivors' spacing at 0.
    static constexpr bool kAirOpsQueueCompactBound = true;  // ON: mechanism (docs/OCEAN_WAVE_FIELD.md 39.1)
    // Packet cc9_p12_land_target_gone (docs/OCEAN_WAVE_FIELD.md section 41): 009F8160's
    // land arm (009F8295-009F82D5). False: a squadron whose land target sank keeps
    // its land command and circles the wreck.
    // Packet cc9_p12_fighter_spent_3d (docs/OCEAN_WAVE_FIELD.md section 43): 009F8160's
    // fighter arm. False: a fighter squadron with an ordnance-class command and no
    // rack round keeps the command (the host did nothing for fighters).
    static constexpr bool kFighterSpentEndBound = true;  // ON: mechanism (docs/OCEAN_WAVE_FIELD.md 43.1)
    // Packet cc13-fix1 (docs/CC13_FIX1.md, docs/CC13_VERIFY11.md R3): 009F7C90's +3Dh
    // arm issues moveto 00E08F68 on a CommandBuilding target before its 0071E430
    // (009F7D17-009F7D75). Used by the fighter arm and the dead-target retest arm.
    // False: no moveto; the command only ends.
    static constexpr bool kSquadronCommandBuildingMoveToBound = true;
    static constexpr bool kSquadronLandTargetGoneBound = true;  // ON: mechanism (docs/OCEAN_WAVE_FIELD.md 41.1)
    // Packet cc9_landing_follower_spacing (docs/SQUADRON_LAND_TASK.md section 5r):
    // the landed byte plane+904h in the sequencer. 006C7960's first arm (mode 4,
    // +4h = -1.0 (00D7A260), +8h = 1.0, before the airborne test), 006C3B10's
    // 2-or-4 refusal only while it is clear, 006C5C40 false for a landed plane,
    // and 006C5534's lookup-miss mode 904h ? 4 : 1. False: the byte is taken
    // clear there, so a landed head gets mode 2 and its followers mode 1.
    // Packet cc9_plane_ground_steering (docs/SQUADRON_LAND_TASK.md section 5t): the
    // rate law in controller mode 1 under the ground law 007DCCF0 (ctl+FCh = 1 at
    // 007DCD24): the roll term zeroed (007DA8D9), no bank-yaw coupling, and
    // 007DA380's mode-1 factors 007DA542-007DA6E3. False: the ground law runs the
    // rate law's free-flight arm.
    // ON since packet cc9_plane_ground_pose (5u): with the flat floor carried, stopped
    // planes hold their heading.
    static constexpr bool kPlaneGroundSteeringBound = true;
    // Packet cc9_ground_speed_hold (docs/SQUADRON_LAND_TASK.md 5y): 007D9F60's up
    // levelling 007DA11C-007DA20C in the pose advance. 007DCCF0 stores ctl+80h..8Ch
    // = (0, cos GroundPitch, sin GroundPitch, RunwaySmoothStrength) on every ground
    // step and the free-flight step 007DC830 zeroes +8Ch (007DCCD3), so the rate is
    // live only under the ground law. Row 1 moves by min(rate x step, 1) x (world up
    // - axis x M), then 0085DAD0 re-orthonormalises up first. False: the pose keeps
    // whatever roll and pitch the touchdown left, and gravity's body-x share pushes
    // a stopped plane sideways (the B-25 creep).
    // ON (5y.1): LOMP10 9200/9000 moved as predicted, the B-25s stay on the strip
    // (min_bfc 1.5 against -954 / -1778); USN04, USN01, JM05, USN13 gameplay-identical.
    static constexpr bool kPlaneGroundLevellingBound = true;
    // Packet cc9_land_park_taxi (docs/SQUADRON_LAND_TASK.md section 5s): land/park.
    // The rule's arms into it (009B3D38 for +900h 4 or 5, 009B3E38 from abort's
    // +66Dh) and out of it (009B3770, the done byte -> abort), its enter/exit
    // 009B21A0/009B21C0 and tick 009B22C0 (the airfield arm; the carrier arm is
    // refused with the mother-ship holders), the done test 009B21D0, the site
    // calls 006CF420/006CF520/006CF5B0 with 006CE610/006CDF70, the transitions
    // 007C16F0/007C1680 and 007B96C0/007B9000. False: park is refused as before.
    // Packet cc9_dive_hit_clock: 009C4AA4 reads the carried approach+C4h instead
    // of the 3600.0 floor, so a hit (009C7900) arms the forced goaway re-roll for
    // one second. OFF until paired with the hit notice's caller.
    // cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 102); moves
    // USN01/USN21 dive releases, BSM09 identical, every win holds.
    static constexpr bool kDiveHitClockBound = true;
    // ON: paired in 5ar.1, every death table identical. The airfield park <->
    // abort loop is NOT the image's (SQUADRON_LAND_TASK 5bp): the image pushes a
    // takeoff task at the first ground abort (0099B0BE), and the host's loopers
    // are planes that lost +BF8h and fell below the terrain (a host
    // ground-support artefact). Kept ON for the carrier lift and the hangar hold.
    static constexpr bool kLandParkStateBound = true;
    // Packet cc9-physics29 landpark (docs/CC13_LAND_PARK.md, OCEAN_WAVE_FIELD 125).
    // 009B3240 at 009B32AA-009B32B8: a plane that is not airborne (plane+72Ch
    // vtable[38h], read here as +900h != 7) gets the land task like any other, with
    // every construction draw, but starts in land/park (task+620h): the state's enter
    // 009B21A0 is called directly (+1Ch = 0, +18h = 0, +28h = 3.0), with no 009B3680
    // switch, no exit and no 007B8AD0 leader test. False: the host refuses the task
    // (the plane keeps no land task; a recalled grounded squadron flies on).
    // ON (cc9-physics29, OCEAN_WAVE_FIELD 125): census 0 on every WINS plane row; the USNOS
    // s63_os_g pair parks recalled grounded planes, which then retake off and land.
    static constexpr bool kLandTaskGroundParkStartBound = true;
    // Packet cc9_carrier_elevator (docs/SQUADRON_LAND_TASK.md 5ao). True, on a
    // mother-ship deck: land/park's carrier arm (009B23E4's vtable[5Ch](9)
    // branches: the lift targets 006D0120/006D00E0, ParkVelocity, the +28h
    // clock, no path, the 12 m steer, the lift-lane test 006D0390/006D0150 and
    // the speed clamp 009B291C-009B298D), the done test's intake exemption
    // (009B2228, 006CFF70), and the elevator: the site tick 006D0600, the
    // platform tick 006FC480 (006CFE40), the intake 006FC720 with the state-2
    // request 007C2090 -> 007CC7A0, the carry 006FC0D0 and the hide/release
    // 007B96C0/006FC250. False: park on a carrier takes the airfield arm.
    static constexpr bool kCarrierElevatorBound = true;  // ON, staged behind park: JM05 9000 stows 8 of 8 (5ao.1)
    // Packet cc9_stowed_plane_stock_return (docs/SHIP_AI_OPEN_ITEMS.md 200). True: a
    // plane whose stowed byte +C00h is set (the elevator's 007B96C0, or the hangar
    // taxi-in) runs 007CE6CA's C7h send on its next fixed step: 007CC8B0 flight
    // state 1, 007F1CA0's stock add 006CA770(class, 1) on the holder's block, the
    // last plane's slot return 006C5950, and Kill(plane, 5). False: the stowed
    // plane stays alive on the site vector and the elevator re-takes it.
    static constexpr bool kStowedPlaneStockReturnBound = true;   // ON by its pairs (SHIP_AI 200.2)
    // Packet cc9_p14_stock_return_destroy (docs/OCEAN_WAVE_FIELD.md 50). True: the
    // stock return's Kill(plane, 5) sets the entity bytes the image's Kill sets at
    // once: 00926DF3 +5Fh = 1, and with +60h clear vtable[70h] 0077D1A0 -> 0077D243
    // 00926C80 Destroy -> 00926CD5 +60h = 1; the flush's +5Dh store (0092639B) is
    // taken in the same step, as the C3Ah death path does. A retired plane then
    // leaves every in-world test (the 007E11D0 neighbour refresh of the vehicle
    // avoidance 007DF4F0 among them). False: the retired plane keeps +5Fh/+60h
    // clear and stays listed as a neighbour.
    static constexpr bool kStockReturnDestroyBytesBound = true;   // ON: mechanism held, outcome miss recorded (OCEAN_WAVE_FIELD 50.6)
    // Packet cc9_stock_return_queue_erase (docs/SHIP_AI_OPEN_ITEMS.md 204). True: the
    // last plane's slot return 006C5950 first erases the squadron's landing-queue
    // record at the block (006C59A4-006C59DC: find at +98h, 14h stride, count +9Ch;
    // 006BF7F0 on +84h moves the last record into its place). False: the record stays
    // and later squadrons queue behind it (n = largest + 1).
    static constexpr bool kStockReturnQueueEraseBound = true;   // ON by its pairs (SHIP_AI 204.1)
    // Packet cc9_squadron_kill_queue_erase (docs/SHIP_AI_OPEN_ITEMS.md 208). True: when
    // 007F3970 leaves a squadron with no member (+3CCh == 0 at 007F3A16), its Kill
    // (00926D90) queues the entity, and the flush's removal notification (009273A0 ->
    // 00696330, callback slot+4) reaches every landing queue that observes it: the
    // queue object at block+84h (vtable 00CF868C, slot+4 006C6B40 -> 006C4B30) removes
    // all of that squadron's records in order (006C06A0) and unregisters (006952A0).
    // LABELLED: the host erases at the leave sweep, one frame boundary before the image's
    // flush. False: a shot-down squadron's record stays and keeps the largest n.
    static constexpr bool kSquadronKillQueueEraseBound = true;   // ON by its pairs (SHIP_AI 208.1)
    // Packet cc9_land_task_retire_queue_erase (docs/SHIP_AI_OPEN_ITEMS.md 208.6). True: a
    // land task retired while its plane lives (the retreat install's 0099A4C0 retire, or
    // 009B34D0's invalid command) runs the destructor's 009B3F50 tail: for the squadron's
    // flight leader (007B8AD0), 006C8800(plane+9D4h) walks every block, and 006C7680, at
    // a block that queues the squadron, erases that record (006BF7F0, the last moved into
    // its place), every +A8h record whose plane belongs to the squadron (006C7540, in
    // order), and sends each launch slot holding it to state 3 (+0Ch = +8h from state 2,
    // timer 0, or 5.0 with +34h cleared). 006BF150's slot publication is not carried.
    // False: the retire leaves the deck's records as they were.
    static constexpr bool kLandTaskRetireQueueEraseBound = true;   // ON by its pairs (SHIP_AI 208.6.1)
    // Packet cc9_land_command_own_target (docs/SHIP_AI_OPEN_ITEMS.md 210). 009B34D0
    // compares the squadron's current command's own target (vtable[178h] at 009B3507,
    // 00521EA0 at 009B3516) with approach+30h. True: the host reads the land command's
    // own target, stored at the land install. False: it reads command_target_plus_one,
    // which the 0071EBF0 refresh rewrites from a still-current category 1/2 row, so a
    // recalled squadron with a current attack row loses its land task (USNOS j:
    // Enterprise_sqn05 at 1406.10 s, 4.4 s after the recall).
    static constexpr bool kLandCommandOwnTargetBound = true;   // ON by its pairs (SHIP_AI 210.1)
    // Packet cc9_carrier_lift_height (docs/SHIP_AI_OPEN_ITEMS.md 210.3). 00759120's tail
    // 0075927C-00759287, after the runwaycenter read and its free (code Ghidra had
    // truncated): class+80Ch, the lift exit point's height, = class+818h, the runway
    // centre's. True: the host's lift[1] takes the runway centre's height. False: it
    // keeps the model's liftexitpoint height.
    static constexpr bool kCarrierLiftHeightFromRunwayBound = true;   // ON by its pairs (SHIP_AI 210.3)
    // Packet cc9_land_abort_ground_arm: land/abort's on-ground arm 009B0E74-009B0F93
    // (+21h = 1, the pitch hold class+1ECh x 0.5, a yaw on the runway-axis error).
    // False: the arm is refused and only +21h acts. OFF: paired with park on, its
    // yaw (heading - runway, opposite to park's sense) rests at pi/2 under the
    // host's park <-> abort loop; re-pair with park (docs/SQUADRON_LAND_TASK.md 5v.2).
    static constexpr bool kLandAbortGroundArmBound = true;   // ON: OCEAN_WAVE_FIELD 78 (bytes match; USN01 identical)
    // Packet p20_land_final_park (docs/OCEAN_WAVE_FIELD.md section 78). 009B3DD9-009B3DF7:
    // land/final with mode +448h = 4 calls 009B3370 BSP_BotStateLandFinal_ParkRule (final
    // +20h touched && !(plane+72Ch)->vt[38h]) and on true switches to land/park (+620h)
    // through 009B3680. True: the edge enters park. False: it is refused, as before.
    // LABELLED: vt[38h] is read as "airborne", the host's +900h == 7.
    static constexpr bool kLandFinalParkEdgeBound = true;
    // Packet p20_finder_noise (docs/OCEAN_WAVE_FIELD.md section 78). 007E2184-007E21B3:
    // for each candidate whose 007DEEC0 score is > 0 and when the noise argument is
    // > 0 ([00D7A218]), 007E2090 adds BSP_Random_UniformFloatRange(1, 0, noise) (00BD2F10,
    // ECX = 1). True: a keyed stream-1 draw (unit name + "#fnoise") per such candidate.
    // False: the constant midpoint noise / 2, as before. RNG-coupled: judge per entity.
    static constexpr bool kFinderNoiseDrawBound = true;   // ON: OCEAN_WAVE_FIELD 78 (BSM09, USN04 identical)
    // Packet p20_df_avoid_timer (docs/OCEAN_WAVE_FIELD.md section 80). 009A7D52-009A7D77: on
    // the latched arm of 009A7A50, +18h > 2.0 ([00CE3958], COMISS/JBE) is redrawn as
    // 00BD2F10(ECX = 1, 1.0, 2.0). True: a keyed stream-1 draw (unit name + "#dfat").
    // False: the constant midpoint 1.5, as before.
    static constexpr bool kDogfightAvoidTimerDrawBound = true;   // ON: OCEAN_WAVE_FIELD 80 (bytes; USN04/JM02 identical)
    // Packet p20_df_early_clear (section 80). 009AAC5D-009AAC63: when 009AAA80's gate
    // fails (+4D4h <= 0.0 at 009AAA9C, or +4D8h >= 1.0 at 009AAABB, NaN failing both),
    // 007B9740(ECX = [task+3FCh]) runs before the false return: with unit+C50h set it
    // tail-jumps to 007E2070, which unregisters the cached target at neighbours+B0h
    // (6952A0) and zeroes it. True: the host's cache (nb_found_plus_one) is cleared.
    // False: the gate returns false and keeps the cache, as before. LABELLED: +C50h,
    // the neighbours block 007E1E20 builds, is taken to exist for every fighter.
    static constexpr bool kDogfightEarlyEdgeClearBound = true;   // ON: OCEAN_WAVE_FIELD 80 (USN04 pair, 210 clears)
    unsigned long long df_avoid_timer_redraws{0}, df_early_edge_clears{0};
    // Packet p20_land_min_speed_owner (section 80). 009B1309-009B137C: with approach+30h
    // set, 009B1300 adds dot(owner velocity (vt[34h], 009B1362), the plane's unit
    // velocity (vt[34h] / 0042B2F0, 009B1315-009B1359)) to 007C47F0 before the max with
    // 007C4810. True: the land target's (land_site_plus_one) linear velocity enters, so a
    // carrier's speed along the plane's heading raises the minimum. False: 0, as before.
    static constexpr bool kLandMinSpeedOwnerVelocityBound = true;   // ON: OCEAN_WAVE_FIELD 80 (USN01 pair, 2332 terms)
    mutable unsigned long long land_min_speed_owner_terms{0};
    // Packet p20_gunfire_exempt_wanderer (section 80). 0099F197-0099F1A8: the exempt arm
    // of 0099EC40 (kinds 10h / 16h, or a flight leader whose squadron +3CCh > 1) writes
    // unit+840h WandererMul = tuning+614h (Pilot/Avoidance/Gunfire/BomberVSGunfire)
    // before returning. True: wanderer.mul takes the value. False: only counted.
    static constexpr bool kGunfireExemptWandererMulBound = true;
    // Packet p20_follow_exit (section 80). 009BDE40 BSP_BotStateFollow_Exit (RET): 007B8A90
    // clears the turbo byte unit+9F9h (009BDE49), then [bot+1Ch]+4Ch, the gun hold timer
    // the sight tick sets, = 0.0 (009BDE57). It is the follow state's exit in the kind-7
    // task and the level-bomb prepare's exit (009B84A0). True: both stores. False:
    // recorded only, as before.
    static constexpr bool kFollowExitClearsBound = true;
    // Packet D5 (cc13): OFF until cc9 pairs it. 009C3374-009C3396: on a state change
    // the moveto task's rule calls the old state's vtable+8 (exit), stores task+310h,
    // then the new state's vtable+4 (enter). Only follow (00D20AB8) has a non-stub exit,
    // 009BDE40; moveto 00D20A80 and circle 00D20A9C have 007B3DC0 (RET). ON: a follow ->
    // moveto/circle change (a wingman promoted to leader) runs follow_exit_009bde40.
    // OFF: no exit call, as before. The follow enter (009BED80) stays with the follow
    // tick's entry detection.
    static constexpr bool kMoveToFollowExitCc13Bound = true;
    // Packet cc13-D5. 009B1420's x87 chains at the main thread's 24-bit precision, each
    // FMUL/FADD/FSUB rounded: the c1 softening (009B1538-009B1550), the asin
    // denominator (009B1713-009B1728), the glide height H and G and the raise
    // (009B1842-009B1925), the deck test (009B19BA-009B19D2), the pitch-arm products
    // lvl x w (009B1A59), spd x 3 (009B1A7C) and length x 1.4 (009B1AA2), and the hold's
    // lateral -X x 1.25 x u (009B1B76-009B1B82); and the caller 009B1D70's speed
    // +2B4h = mc + (a+5Ch - mc) x a+4Ch (009B1DAB-009B1DCE). True: per-op float in the
    // listing's order. False: the double chains rounded once. ON (cc9-physics29,
    // OCEAN_WAVE_FIELD 124): six WINS rows paired, BSM01 l72_b1_k1 moved with its end kept.
    static constexpr bool kLandBeginSteerPc24Cc13Bound = true;
    // Packet p20_strafe_horizontal (section 80). 009CAD27-009CAD58: dx and dz are FSTP'd
    // to floats (009CAD30, 009CAD3E) and d2 = (dx dx + 0 0) + dz dz is stored as a float
    // (009CAD58), main-thread 24-bit per op. True: float per op. False: double dx, dz.
    static constexpr bool kStrafeHorizontalFloatOpsBound = true;
    // Packet p20_ctor_seeds (section 81). The bot-task base constructors 0099C6F0 /
    // 0099C8B0 seed task+308h = -U(0, 1) (ECX = 1, 0099C765 FCHS FSTP 0099C76C),
    // +3E8h = U(-1, 0) (ECX = 0, 0099C7BA FSTP 0099C7BF) and +3ECh = -1.0 (0099C7D3), on
    // every task construction; the follow state 009C2980 seeds +74h = -U(0, 0.6)
    // (009C2A0A), the dogfight maneuver 009A84E0 +2Ch = -U(0, 1) (009A853A), the torpedo
    // attackrun 009D06D0 +1Ch = -U(0, 1). True: those values from keyed per-(unit,
    // purpose) streams at each host task install (one host stream without
    // BSP_GUNNERY_RNG_STREAMS, not the image's stream 0/1 order). False: the per-unit
    // values persist across tasks (ga 0 / 0, +74h 0, maneuver and torpedo -0.5).
    // Not hosted, recorded: the depth-charge attackrun 009A36A0 +1Ch and the dogfight
    // attackrun 009A70E0 +1Ch (no host countdown field; the latter is unreachable).
    static constexpr bool kBotTaskCtorSeedsBound = true;
    unsigned long long bot_task_ctor_seeds{0};
    // Packet cc13-seg67n3 (docs/CC13_SEG67N.md, include/bsp/cc13_seg67n_switches.hpp).
    // 0099C790 / 0099C950: every task constructor builds a fresh gun block at task+314h
    // with 009FAAD0; this host keeps one per unit (df_gun and its sibling fields).
    // ON: the modelled fields take 009FAAD0's stores, in its order. The reset draws
    // nothing, so its place between the +308h and +3E8h draws does not matter here.
    unsigned long long task_gun_block_resets{0};
    void task_gun_block_construct_009faad0(GameUnitSlot& unit) {
        if constexpr (bsp::kTaskGunBlockReconstructBound) {
            ++task_gun_block_resets;
            unit.st_gun_point_armed_28 = false;     // 009FAB14: +28h = -1.0 (00D7A260)
            unit.df_gun.fire_48 = false;            // 009FAB1C: byte +48h = 0
            unit.df_gun.burst_4b = false;           // 009FAB22: byte +4Bh = 0
            unit.gun_target_74_plus_one = 0;        // 009FAB25: +74h = 0
            unit.gun_cone_40 = 0.0f;                // 009FAB41: +40h = 0.0
            unit.df_gun.hold_4c = 0.0f;             // 009FAB4B: +4Ch = 0.0
            if constexpr (bsp::kTaskGunBlockBurstTimerResetBound)
                unit.df_gun.burst_timer_50 = 0.0f;  // 009FAB50: +50h = 0.0
        } else {
            (void)unit;
        }
    }
    // cc13-sightdraw (kFollowSightCountdownDrawCc13Bound, ON only): 009C2980's +74h,
    // -U(0.0f, 0.6f) from stream 1 (009C29E0-009C2A19, FCHS exact), drawn from the
    // shared stream-1 generator once per 009C2980 the task's approach constructor
    // makes (`constructs`, in call order). The host keeps one +74h per unit, so the
    // first state's value is returned and the later states' draws only advance the
    // stream (labelled). Without a gunnery host the keyed draw answers, once.
    unsigned long long follow_stagger_shared_draws{0};
    unsigned long long follow_stagger_shared_misses{0};
    float follow_stagger_shared_009c2a0a(const GameUnitSlot& unit, const std::string& key,
                                         int constructs = 1) {
        float v = 0.0f;
        if (gunnery != nullptr) {
            for (int i = 0; i < constructs; ++i) {
                ++follow_stagger_shared_draws;
                const float d = -gunnery->ship_ai_draw(unit.process_index, 0.0f, 0.6f);
                if (i == 0) v = d;
            }
        } else {
            ++follow_stagger_shared_misses;
            v = -release_altitude_draw_00bd2f10(key, 0.0f, 0.6f);
        }
        return v;
    }
    // cc13-stream2 (include/bsp/stream_set2_cc13.hpp): a per-unit keyed draw that the
    // image takes from the shared stream 1 (00BD2F10 with ECX = 1). Bound (one of
    // kStreamSet2SharedDrawsCc13Bound / kStreamSet2WandererCc13Bound): the shared
    // generator in call order, as the issue-interval and rack-delay draws do. Otherwise,
    // or without a gunnery host, the unit's keyed draw, as before.
    template <bool Bound>
    float stream_set2_draw_00bd2f10(const GameUnitSlot& unit, const std::string& key,
                                    float lo, float hi) {
        if constexpr (Bound) {
            if (gunnery != nullptr)
                return gunnery->death_mode_draw_00bd2f10(1, index_of_slot(unit), lo, hi);
        }
        return release_altitude_draw_00bd2f10(key, lo, hi);
    }
    float stream_set2_draw_00bd2f10(const GameUnitSlot& unit, const std::string& key,
                                    float lo, float hi) {
        return stream_set2_draw_00bd2f10<bsp::kStreamSet2SharedDrawsCc13Bound>(unit, key,
                                                                                lo, hi);
    }
    // `follow_constructs` (cc13-sightdraw, read only under
    // kFollowSightCountdownDrawCc13Bound): how many 009C2980 calls the task's
    // approach constructor makes (rel32 callers of 009C2980, construct paths):
    // dogfight 009A94E0, strafe 009CC020, level bomb 009B75E0 and kamikaze 009AED20
    // 2; drop kamikaze 009AD720, depth charge 009A4DC0, dive bomb 009C73A0 and
    // torpedo 009D2DA0 3; moveonpath 009BCC60 1.
    void bot_task_ctor_seeds_0099c6f0(GameUnitSlot& unit, bool follow_state,
                                      int follow_constructs = 1) {
        task_gun_block_construct_009faad0(unit);    // 0099C790
        // cc13-rocketstop, kRocketTargetStopHandlersCc13Bound (include/bsp/
        // rocket_target_stop_cc13.hpp): the new task's own task+38Ch block. The old
        // task's Stop (009AA9E0 / 009CD0D0 / 007B7BA0) clears its block and 00999EE0
        // deletes it; 0099C79C (0099C95C) runs 009FCEF0 on the new one. ABI: none (host
        // per-plane block). Uncertain: the image's new block leaves +18h unset; the host
        // keeps the Stop's zero vector there (overwritten by 007B7870 before use).
        if constexpr (bsp::kOpportunisticRocketBound) {
            // Census (cc9-nondet1, log only): installs that find the block off its
            // 009FCEF0 reset state, which the switch would reset.
            const bsp::OpportunisticRocketState& o = unit.orx;
            ++rocket_stop_installs_;
            if (o.class_00 != bsp::OpportunisticRocketClass::kNone || o.salvo_left_0c != 0
                || o.check_timer_10 != bsp::opportunistic_rocket::kTimerSeed
                || o.delay_timer_14 != bsp::opportunistic_rocket::kTimerSeed
                || o.target_plus_one_38 != 0 || o.target_is_plane_3c
                || o.rocket_plus_one_58 != 0)
                ++rocket_stop_not_reset_;
        }
        if constexpr (bsp::kRocketTargetStopHandlersCc13Bound && bsp::kOpportunisticRocketBound) {
            bsp::opportunistic_rocket_clear_target(unit.orx);        // the Stop
            bsp::opportunistic_rocket_construct_009fcef0(unit.orx);  // 0099C79C
            unit.orx_owner = 0;
        }
        if constexpr (kBotTaskCtorSeedsBound) {
            ++bot_task_ctor_seeds;
            const std::string& n = unit.row.name;
            unit.task_phase_308 = -stream_set2_draw_00bd2f10(unit, n + "#tb308", 0.0f, 1.0f);
            unit.ga_timer_3e8 = release_altitude_draw_00bd2f10(n + "#tb3e8", -1.0f, 0.0f);
            unit.ga_rest_3ec = -1.0f;
            if constexpr (bsp::kFollowSightCountdownDrawCc13Bound) {
                if (follow_state)
                    unit.fs_countdown_74 = follow_stagger_shared_009c2a0a(unit, n + "#tb74",
                                                                          follow_constructs);
            } else {
            if (follow_state)
                unit.fs_countdown_74 = -release_altitude_draw_00bd2f10(n + "#tb74", 0.0f, 0.6f);
            }
        } else {
            (void)unit;
            (void)follow_state;
        }
    }
    // Packet p20_cadence (section 82). 009998A0's slow path runs 0099B740 only when
    // dt >= task+308h (00999912-00999926 FCOMI / JC): then +308h = (+304h - dt) + +308h
    // (0099992C FSUBR, 00999934 FADDP, +304h = 1.0 from 0099C75D) and 0099993C CALL
    // 0099B740; otherwise +308h -= dt (009999B5-009999BB) and 0099B740 is skipped. The
    // task arm runs either way. True: the host's 0099B740 sites take that gate on
    // task_phase_308. False: 0099B740 on every think, as before.
    static constexpr bool kTaskAttackModeCadenceBound = true;
    unsigned long long attack_mode_cadence_fires{0};
    unsigned long long attack_mode_cadence_skips{0};
    bool attack_mode_cadence_009998a0(GameUnitSlot& unit, float dt) {
        if constexpr (kTaskAttackModeCadenceBound) {
            const float t = unit.task_phase_308;
            if (dt < t) {
                unit.task_phase_308 = t - dt;
                ++attack_mode_cadence_skips;
                return false;
            }
            unit.task_phase_308 = (1.0f - dt) + t;
            ++attack_mode_cadence_fires;
            return true;
        } else {
            (void)unit;
            (void)dt;
            return true;
        }
    }
    unsigned long long follow_exits{0};
    void follow_exit_009bde40(GameUnitSlot& unit) {
        ++follow_exits;
        if constexpr (kFollowExitClearsBound) {
            // 007B8A90: the typed body clears the plane's turbo byte +9F9h; OFF keeps
            // the direct store.
            bsp::oct18_tail_b::plane_clear_turbo_007b8a90(unit.plane_turbo_9f9);
            unit.df_gun.hold_4c = 0.0f;        // 009BDE57
        }
    }
    static constexpr bool kLandingLandedArmBound = true;  // ON: all ten LOMP10 planes land (docs/SQUADRON_LAND_TASK.md 5r)
    static constexpr bool kFollowLeaderTurnRateBound = true;  // ON: mechanism held, spread miss recorded (docs/PLANE_FOLLOW_LAW.md 17.5)
    // True: 009BFC58/009BFCC3's leader vtable[38h] (007B8E60, unit+B1Ch, the
    // controller's forward speed) is the leader's live |v|, as the hold arm
    // reads it. False: the leader's authored TravelSpeed.
    static constexpr bool kFollowLeaderLiveSpeedBound = true;  // ON: re-paired on HEAD (docs/PLANE_FOLLOW_LAW.md 17.7)

    // 007D7DA0 (007D7DA0-007D7E91, __thiscall(ctl = unit+AB0h), RET, ST0).
    static float leader_turn_rate_007d7da0(const GameUnitSlot& u) {
        const float cp = std::cos(std::fabs(u.plane_pitch_angle_c64));
        const float cb = std::cos(u.plane_bank_angle_c68);
        const float sb = std::sin(u.plane_bank_angle_c68);
        const float yaw_spd = u.plane_class.yaw_spd;                 // desc+1B0h
        float rate;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // pc24w1-sweep3-5: each op rounded at PC24 in the listing's order.
            // x = c1 * pitch_spd + roll_spd * cp (007D7E1C..007D7E39); y = x +
            // ((cp * cb) * slide) * yaw_spd (007D7E4B..007D7E5F); z = y * sb (007D7E6B);
            // z + (cb * c0) * yaw_spd (007D7E7D..007D7E83: cb * c0 is formed first).
            namespace pc24 = bsp::cc13x87;
            const float x = static_cast<float>(pc24::x87_add24(
                pc24::x87_mul24(u.plane_live_controls[1], u.plane_class.pitch_spd),
                pc24::x87_mul24(u.plane_class_turn_roll_spd, cp)));
            const float y = static_cast<float>(pc24::x87_add24(
                pc24::x87_mul24(pc24::x87_mul24(pc24::x87_mul24(cp, cb), u.plane_class.slide_ratio),
                                yaw_spd),
                x));
            const float z = static_cast<float>(pc24::x87_mul24(y, sb));
            rate = static_cast<float>(pc24::x87_add24(
                z, pc24::x87_mul24(pc24::x87_mul24(cb, u.plane_live_controls[0]), yaw_spd)));
        } else {
        const float x = static_cast<float>(
            static_cast<double>(u.plane_live_controls[1]) * u.plane_class.pitch_spd +  // +9E8h, +1ACh
            static_cast<double>(u.plane_class_turn_roll_spd) * cp);                    // +1C8h
        const float y = static_cast<float>(static_cast<double>(x) +
            static_cast<double>(cp) * cb * u.plane_class.slide_ratio * yaw_spd);       // +1B8h
        const float z = static_cast<float>(static_cast<double>(y) * sb);
        rate = static_cast<float>(static_cast<double>(z) +
            static_cast<double>(yaw_spd) * cb * u.plane_live_controls[0]);             // +9E4h
        }
        return rate;
    }
    static float leader_live_speed_007b8e60(const GameUnitSlot& u) {
        const float* v = u.plane_world_velocity;
        if constexpr (bsp::kPlaneCachedSpeedB1cCc13Bound) {
            // 007B8E60 FLD [unit+B1Ch], as 007D8020 caches it (per op, 1e-10 floor).
            return bsp::plane_cached_speed_b1c_007d8020(v);
        } else {
        return static_cast<float>(std::sqrt(static_cast<double>(v[0]) * v[0] +
            static_cast<double>(v[1]) * v[1] + static_cast<double>(v[2]) * v[2]));
        }
    }
    // Packet cc9_wing_achieved_speed (docs/PLANE_FOLLOW_LAW.md section 16): the
    // torpedo and dive-bomb moveto speed slot 009C1850 as the image forms it,
    // 009BECD0(a = [approach+0Ch]+3A0h, b = 007C47F0, sep). squadron+3A0h is
    // the squadron class's desc+190h TravelSpeed x NewTravelSpeedMul
    // (007F2020-007F202A, 007ED596-007ED5AE, refreshed from 0099ACD0 through
    // 007ED5D0), and 009BECD0 blends from b toward it by max(the wait-distance
    // ramp, 007EF2C0's wingmen value): the leader waits for a wing that is out
    // of position. Before: 007C47F0 alone, the leader always at LevelFlight x
    // StallSpd.
    static constexpr bool kMovetoSpeedBlendBound = true;
    // Packet cc13-mvspeed (census row 20, docs/CC13_MVSPEED.md). 009C18C0 forms
    // its separation at 009C18EC-009C1984 from the moveto TARGET'S ORIGIN
    // ([state+2Ch]+FCh/+104h less the unit's), in float at PC24: dx, dz and
    // d2 = dx*dx + dz*dz each FSTP float, d2 above double 1e-10 (00CE3820) or
    // 0, then 00BF7030's square root, FSTP float; 0 when +2Ch is null. It then
    // calls the speed slot 009C1850(sep) at 009C198A before any early return,
    // and passes the same sep to the glide (009C1A71 denominator, 009C1B01
    // rangeHigh). True: every 009C1850 site forms sep that way, and the torpedo
    // tick (which used approach+90h, the range to the 009FADA0 hull aim point,
    // and returned before the speed write when it was not positive) takes the
    // origin sep for the speed and the glide and writes the speed first. The
    // dive site keeps its own switch, kDiveBombMoveToOriginBound, for the
    // choice of point. False: the double-precision sep and the old torpedo tick.
    static constexpr bool kMovetoSpeedShapingCc13Bound = true;    // ON (p25: USN21 unexercised)
    static_assert(!kMovetoSpeedShapingCc13Bound || kMovetoSpeedBlendBound,
                  "the 009C1850 sep fix shapes the bound 009BECD0 blend");
    // 009C18EC-009C1984, the moveto tick's planar separation (see above).
    static float moveto_sep_009c18ec(const GameUnitSlot& unit, const GameUnitSlot* target) noexcept {
        if (target == nullptr) return 0.0f;                    // 009C18D0/009C18D6
        const float dx = target->motion.position[0] - unit.motion.position[0];   // 009C1936
        const float dz = target->motion.position[2] - unit.motion.position[2];   // 009C1944
        const float d2 = dx * dx + dz * dz;                    // 009C1958 FSTP float
        if (!(static_cast<double>(d2) > 1e-10)) return 0.0f;   // 009C196A JBE
        return std::sqrt(d2);                                  // 009C196C, 009C1971
    }
    // Packet cc9_gunfire_avoidance (docs/ATTACKER_EVASION.md section 5): the
    // firing list (007C74A0/007C75A0 on lastGunState +C35h), 009A17D0's gunfire
    // arm (0099EC40 with its timers and exemptions, bands through 0099B790)
    // and 0099BF30's band repair of the published roll/pitch/yaw command.
    // Before: none of the three ran. The vehicle (0099B670) and terrain
    // (0099F1C0) arms of 009A17D0 are the two switches below.
    static constexpr bool kPilotGunfireAvoidanceBound = true;
    // Packet cc13-fix2 (docs/CC13_FIX2.md, finding R2 of CC13_VERIFY15):
    // 0099EFA5-0099F029, a kind-13h (MPlaneFighter) evader scales the drawn
    // gunfire timers by f = float(1 - [[unit+DF4h]+34h] x double 0.3f), floored
    // at 0.4f (double [00CE65D0] compare, JBE keeps f and NaN; float [00CE7804]):
    // +3E8h *= f, +3ECh /= f. SPNormal: evade 0.7x, rest 1.43x. Before: unscaled.
    static constexpr bool kPilotFighterGunfireScaleBound = true;  // ON 2026-10-17 (OCEAN_WAVE_FIELD 67)
    // Packet cc9_pilot_vehicle_terrain_avoidance (docs/ATTACKER_EVASION.md
    // section 6): 009A17D0's vehicle arm 0099B670 -> 007DF4F0 over the
    // unit+C50h object's +30h list, and its terrain arm 0099F1C0 with
    // 0099CAB0, both inserting through the merged 0099B790 sets; 0099BF30's
    // pilot+25Ch throttle arm. Before: neither arm ran.
    static constexpr bool kPilotVehicleAvoidanceBound = true;
    static constexpr bool kPilotTerrainAvoidanceBound = true;
    // Packet cc9_player_role_bookkeeping (docs/SCRIPTED_HELM.md section 2):
    // the permission words from SetRoleAvailable (0077F360 -> 00927D20), the
    // 4Bh role arm of 00780120 with 0059BBD0, HUD page 27h's role-0 take on
    // the controlled unit (0067BB50), and unit+184h set only by an accepted
    // role-1 take. Before: unit+184h was "the unit 004c0890 bound".
    static constexpr bool kPlayerRoleBookkeepingBound = true;
    // Where 0067BB50 runs: true keeps the once-per-fixed-step call before the
    // unit loop; false leaves it to the HUD pump through the public
    // GameUnitsHost::role_screen_update_0067bb50 (twice per mission frame,
    // docs/SHIP_SCREEN_UPDATE.md 21). False since packet cc9_screen_2eh_group
    // (section 26): the pump's slot 27h calls it under
    // bsp::kHudRoleScreenPumpBound. Keep the two opposite.
    static constexpr bool kRoleScreenFixedStepCall = false;
    // Packet cc9_plane_substitution_sweep (docs/PLANE_SUBSTITUTION_SWEEP.md):
    // the torpedo done/prepare tick's 009D1500 answer (was 0), unit+BC4h's yaw
    // gain from the terrain arm (was held at 1.0), and the fighter lead's
    // target acceleration from the fixed-step difference (was the gun tick's).
    static constexpr bool kTorpedoArmTimeToTargetBound = true;
    static constexpr bool kPlaneYawGainBc4Bound = true;
    // Packet cc9_plane_yaw_gain_raw, R2 (docs/CC13_GPR_48.md 8.3, 10): where
    // unit+BC4h enters. 007DA926-007DA94B forms yaw_raw = (YawSpd * latched yaw)
    // * BC4h, and yaw_raw feeds both the target -(yaw_raw*f2 - slide) and the
    // coupling roll += (yaw_raw*f1)*1B4h (007DAA5C-007DAA78). False: the host
    // multiplies the finished yaw target by BC4h, which scales the slide term too
    // and leaves BC4h out of the coupling. Needs kPlaneYawGainBc4Bound.
    static constexpr bool kPlaneYawGainRawBound = true;
    static constexpr bool kFighterLeadAccel654Bound = true;
    // Packet cc9_airfield_destruction_rule (docs/GUNNERY_OPEN_ITEMS.md 119): 006D40F0,
    // the airfield's vtable[1A8h], which 006D2510 runs every fixed step. When no
    // listed hangar has +370h > 0, an airfield adopted by a CommandBuilding
    // (+71Ch, 006F5E9B) takes the InferiorFailure arm (message 7Dh, +720h, which
    // 00895E51 makes IsReadyToSendPlanes answer false); any other dies through
    // vtable[70h](0) = 0077D1A0. ON by the pairs of 2026-10-06 (GUNNERY 121.5).
    // False: the rule does not run.
    static constexpr bool kAirfieldDestructionRuleBound = true;
    // Packet cc9_fighter_accel_friendly_fire (docs/FIGHTER_GUN_LEAD.md 5):
    // 007B96D0 -> 007DEDB0, the fighter gun's friendly-in-line hold. The
    // answer feeds DogfightGunInputs::finder_busy at the fighter gun call site.
    static constexpr bool kFighterFriendlyInLineBound = true;
    // Packet cc9_dogfight_maneuver_bodies (docs/DOGFIGHT_MANEUVER_BODIES.md):
    // the maneuver enter/tick/expire 009A87F0/009A8B20/009A86F0 with 009A85B0;
    // the avoid enters, 009A7A50 and both avoid ticks; 009A9970's roll/turn
    // choice with the weights 009A7F70/009A83B0; and 0099DE8A's gate, the yaw
    // base term only on heading mode 2. Draws at their midpoints, as every
    // other dogfight draw in this host.
    static constexpr bool kDogfightManeuverBodyBound = true;
    // Packet cc13-D5 (spot-check gap rows). 009A85B0 (009A85DF-009A8607): when the
    // target's published byte [T + 9C0h + 8*[00F876B8]] is set (its own 007B9140(T, 0),
    // published by 007CDC70; read live here, as kReleaseTimerNotify0DhBound does), the
    // plane must answer 007B9140(plane, 0) too, else the threat test is false
    // (009A86DF). True: that clause. False: skipped, as before. SUBSTITUTION, labelled:
    // the image reads the byte 007CDC70 published for the step slot [00F876B8]; True
    // computes 007B9140(T, 0) live at the call, which can differ when T's racks change
    // within the step.
    static constexpr bool kDfThreatOrdnanceClauseCc13Bound = true;
    // Packet cc13-D5. The cone arm 009A8682-009A86C6 at 24-bit precision:
    // ShootDistance x 1.5 (FMUL qword 00CE3D78) is rounded before the FCOMIP,
    // x / z and y / z are FSTP'd as floats, and 0042B2F0 forms (x*x + y*y) + z*z
    // per op in float. True: those roundings. False: the 53-bit double chain.
    static constexpr bool kDfThreatConePc24Cc13Bound = true;
    // Packet cc13-D5. 009B4D00's spacing distances, own_d (009B4FE4-009B5054) and each
    // peer's (009B50EE-009B5154): dx and dz FSTP'd as floats, d2 = (dx*dx + 0) + dz*dz
    // per op in float, sqrt (00BF7030) only above 1e-10 (00CE3820), else 0; as 0042B2F0
    // with y = 0. True: gun_length3_0042b2f0<false>. False: the 53-bit double sqrt.
    static constexpr bool kLevelBombSpacingDistPc24Cc13Bound = true;
    static constexpr bool kDogfightAvoidBodiesBound = true;
    static constexpr bool kDogfightAvoidPickBound = true;
    // Packet cc13-dogfight1 (docs/CC13_DOGFIGHT_DRAWS.md): three of those draws
    // taken on keyed host streams (release_altitude_draw_00bd2f10, unit name +
    // suffix) instead of at their midpoints: 009A8712 the maneuver-expiry roll
    // against TurnAfterChance, 009A99C6 the avoid pick, 009FCED1 the gun burst
    // clock from the pilot's skill row. False: the midpoints, as before.
    static constexpr bool kDogfightTurnAfterDrawBound = true;   // GUNNERY 197: ON, USN01 win gameplay-identical
    static constexpr bool kDogfightAvoidPickDrawBound = true;   // GUNNERY 198: ON by mechanism (lead ruling a)
    static constexpr bool kDogfightGunBurstDrawBound = true;   // GUNNERY 198: ON by mechanism (lead ruling a)
    // Packet cc9_l60_df_enter_draws (cc13-confirm A): the remaining dogfight
    // state-enter draws, each 00BD2F10 on ECX = 1, taken on keyed host streams
    // instead of at their midpoints: 009A857E the maneuver pursuit range
    // U(0.3, 1.1) x row+230h; 009A75DD-009A7635 the aim enter U(0.8, 1.4) x
    // row+210h and U(0.5, 0.8) x row+20Ch x approach+24h; 009A7DE0/009A8020 the
    // avoid enters' timer U(0.75, 1.5) x row+214h, then the roll side
    // U(0.8, 1.0) / U(-0.8, -1.0) or the turn's +20h U(0.75, 1.0) and +1Ch
    // U(70, 100 deg) / U(-70, -100 deg) by the bank sign (+C68h > 0).
    // False: the midpoints, as before.
    static constexpr bool kDogfightEnterDrawsBound = true;   // ON: SQUADRON_LAND_TASK 5hg.1
    // Packet cc9_l60_torpedo_disarmed (cc13-confirm B): 009D2720's disarmed arm
    // 009D29E0-009D2CF1 (every done tick: the enter leaves +98h = -1.0), run
    // through bsp::torpedo_done_disarmed_arm_009d29e0 when the squadron has a
    // first member and the task a target. Close (leader below 300 m, ratio below
    // 3.5) it writes the bank cap cmd+2C8h = IP(1 -> 0.3, 2.2 -> 1.5; ratio).
    // The aim point approach->vtable[0] is taken as the target's position
    // (substitution); approach+A4h = 3 and [approach+1Ch]+40h = 0 have no host
    // cell; state+8Ch (republished to squadron+3E8h by 009BFDA6) is kept on the
    // slot for the log only, no host reader. False: the arm is skipped, as before.
    static constexpr bool kTorpedoDoneDisarmedArmBound = true;   // ON: SQUADRON_LAND_TASK 5hn
    // Packet cc9_l60_formation_shapes (cc13-confirm C): the drop-kamikaze
    // prepare/done enter 009AD180 writes squadron+3E4h = 3 (009AD1A5) and
    // re-runs 007ED260 after its follow enter 009BED80, which writes 1
    // (009BEDDA). True: both writes reach the registry's formation_shape_3e4
    // (007ED260 runs inside the placement seam, as for the dive bomber), so the
    // wingmen fly 007F23A0's shape-3 station. False: the shape stays as seeded.
    static constexpr bool kDropKamikazeFormationShapeBound = true;   // ON: SQUADRON_LAND_TASK 5hn
    // Packet cc9_l60_torpedo_command_writes (lead, after SQUADRON_LAND_TASK 5hh). The
    // torpedo arm's command-block writes reach TorpedoArmBinding's write_command_*
    // (torpedo_task_arm.cpp): the committed branch's cmd+2C8h = 0.3, the bank cap
    // (009D2761-009D2794), and the hold / steer arms' cmd+2BCh = 0, +2D0h = 2, +2C4h = 0,
    // +2CCh = 1 (009D295F-009D29B7). True: they land on the planner state (plan_state
    // bank_limit_2c8 / pitch_target_2bc / pitch_mode_2d0 / bank_target_2c4 and
    // plan_heading_mode_2cc). False: every write is dropped, as before.
    static constexpr bool kTorpedoCommandWritesBound = true;   // ON: SQUADRON_LAND_TASK 5hr
    // Packet p21_torp_enter (cc13 CC13_SEG67B 3.51). The done/prepare enter
    // 009D2530 stores state+98h = -1.0 (009D253B, 00D7A260) on every entry, and
    // done (task+310h) and prepare (task+740h) each own a +98h; 009D49A0 arms
    // prepare's only. True: the done tick reads and writes its own timer and
    // each enter resets the entered state's timer to -1. False: one shared
    // timer, reset only at install, so done can release on prepare's stale arm.
    static constexpr bool kTorpedoStateEnterTimersBound = true;   // ON: OCEAN_WAVE_FIELD 85
    // The aim state's enter 009D15D0 (vtable 00D212FC +4h): approach+A8h = 1
    // (009D15DE), state+24h = 0 (009D15E7, the release timer 009FA3A0's +0Ch
    // enable) and state+2Ch = 0 (009D15EA, aim complete). Its exit 009D1350
    // (+8h) stores approach+A8h = 0 (009D1353). +A8h gates the replan's
    // over-land writers 009D3688. True: those stores land on
    // torpedo_aim_timer_enabled_0c, torpedo_aim_complete_2c and
    // torpedo_approach.scan_enabled_a8. False: +2Ch stays set once set, the
    // enable carries over and +A8h stays 0.
    static constexpr bool kTorpedoAimEnterStoresBound = true;   // ON: OCEAN_WAVE_FIELD 85
    // Packet p22_los_007df360 (OCEAN_WAVE_FIELD 87). The replan's second
    // over-land writer: when unit+C50h is set (007D61C0-007D621F: built when
    // [game+1FE4h] is 0 or 1 and unit+54h != the scene's DummyAIEnabled),
    // 009D36EE 007DF360(ECX = unit+C50h, [approach+CCh], aim point), RET 8,
    // AL, and 009D36F3 stores AL to +A9h outright. 007DF360 walks the +30h
    // list and answers 1 for the first IsKindOf(6) member, not the target,
    // ahead of the aim point toward the plane (dot > 0) whose perpendicular
    // distance squared is below Length^2 * 0.22 ([class+A0h], double
    // [00D06BA0]). True: modelled over nb_near_30 with the plane's own pose
    // (unit+FCh/+104h; [[C50h+4]+28h] is taken to be the plane, labelled).
    // False: -1, the old "leave +A9h alone" answer.
    static constexpr bool kTorpedoLineOfSightBound = true;   // ON: OCEAN_WAVE_FIELD 87
    // Packet p22_cruise_pass (OCEAN_WAVE_FIELD 88). The torpedo cruise profile
    // 009D4A70 ends its install with the +12Ch flip (009D4BAF-C8), the plan
    // park (009D4BD0-E6), the clears of approach+60h (a float, 009D4BEE) and
    // +64h/+68h/+6Ch, then 009D3420(approach, 0.0) (009D4BD8/009D4BDB/009D4BFC)
    // and tail-JMP 0099B740. True: the host clears +60h, not +5Ch, and runs
    // that dt = 0 pass at install, so the first replan and 009FADA0's dirty
    // re-pick happen there. False: no pass; the first arm tick replans.
    static constexpr bool kTorpedoCruisePassBound = true;   // ON: OCEAN_WAVE_FIELD 88
    // Packet cc13-torpranges (census row 15, docs/CC13_TORPRANGES.md). The torpedo
    // arm stores A = approach+78h + approach+74h into the moveto state's +30h and
    // +34h and approach+7Ch/+80h (on +134h against 15.0) into +38h (009D48AC-
    // 009D48CF, 009BDE80). The moveto tick 009C18C0 reads them back as
    // base = max(+34h + targetY, +30h) (009C19F1-009C1A19, targetY = [state+2Ch]
    // +100h copied at 009C1905) and rangeLow = +38h (009C1B09). True: the arm
    // stores the three and the tick reads them, with the target lift. False: the
    // tick reads the approach fields directly and the base is A, which is low by
    // targetY whenever the target's origin sits above 0 (USN21: up to +1.3 m).
    static constexpr bool kTorpedoMoveToRangesBound = true;    // ON (p25: identity, USN21 census)
    // Packet p22_moveto_c25 (OCEAN_WAVE_FIELD 89). With [unit+C25h] set (a
    // pending release), the moveto state tick 009C2430 (009C2519-009C2559) and
    // the circle tick 009C26D0 (009C275A-009C2794) store cmd+2BCh = 0.0,
    // +2D0h = 2, +2C4h = 0.0 and +2CCh = 1, then RET 4, after the speed
    // stores. True: the host makes those stores before it returns. False: the
    // arm only records itself and the previous tick's commands stand.
    static constexpr bool kMoveToC25ArmBound = true;   // ON: OCEAN_WAVE_FIELD 89 (faithful, unreached)
    // Packet p22_moveto_head (OCEAN_WAVE_FIELD 90). The moveto state tick
    // 009C2430 stores [[state+4]+18h]+26Ch = 1 at 009C2440, first thing, and
    // takes its distance as float(dx*dx + dz*dz) (FSTP at 009C2496), zero at
    // or below double 1e-10 ([00CE3820], 009C249A-009C24A8 FCOMI/JBE), else
    // float(sqrt). True: that store and that distance. False: double d2 with
    // a 1e-6 threshold, and no +26Ch store.
    static constexpr bool kMoveToTickHeadBound = true;   // ON: OCEAN_WAVE_FIELD 90 (identical)
    // Packet p22_lb_goaway_order (OCEAN_WAVE_FIELD 90). The level-bomb goaway
    // enter 009B5680 loads approach+24h (009B5724, FSTP to a local at
    // 009B572A), draws at 009B5746, then 009B574B-009B5755 FLD the speed ratio,
    // FMUL the clamped base, FMULP the draw: +18h = draw * (ratio * base).
    // True: that order. False: (draw * base) * ratio.
    static constexpr bool kLevelBombGoAwayOrderBound = true;   // ON: OCEAN_WAVE_FIELD 90 (unreached)
    // Packet p22_strafe_draws (OCEAN_WAVE_FIELD 91). Two stream draws the host
    // skipped. (a) The strafe aim enter 009CB0F0 stores +18h = U(0.2, 0.8)
    // (ECX = 1, [00CE54A0] / [00CE74F8], 009CB116, FSTP 009CB11B); no tick
    // reads it. (b) Each tightturn (009BC030) or flikflak (009BC0A0) push
    // constructs a bot task through 0099C6F0, whose seeds draw +308h = -U(0, 1)
    // and +3E8h = U(-1, 0) (section 81). True: those draws are taken under the
    // keys the host already uses (`#sta18`, `#tb308`, `#tb3e8`) and their
    // values are dropped (the pushed task's own +308h / +3E8h are not hosted).
    // False: no draw.
    static constexpr bool kStrafeDrawCountBound = true;   // ON: OCEAN_WAVE_FIELD 91 (identical)
    // Packet p22_land_blend_ops (OCEAN_WAVE_FIELD 92). The land standby 009B0FE0
    // (009B11E9-009B1205) and final 009B1ED0 (009B1FC6-009B1FDF) blend
    // +2B4h = a + w * (b - a) on the x87 at the main thread's 24-bit precision:
    // FSUBP, FMULP by approach+4Ch and FADDP each round to float. True: float per
    // op. False: (b - a) in float, then a double product and sum rounded once.
    static constexpr bool kLandBlendFloatOpsBound = true;   // ON: OCEAN_WAVE_FIELD 92 (identical)
    // Packet p23_dive_profile_gate (OCEAN_WAVE_FIELD 96). The dive cruise profile
    // 009C8920's block 1 (009C894F-009C8971: the squadron travel-altitude gate on
    // +38Dh / +380h / +3A9h, the tuning+4C0h write and the +3A9h clear) and its
    // 009C8A74 re-seed of task+4BCh = 3600.0. True: both, on the profile call.
    // False: the dive host never resets an expired script altitude, never clears
    // +3A9h, and carries only the 009C3FD2 seed.
    static constexpr bool kDiveCruiseProfileGateBound = true;   // ON: OCEAN_WAVE_FIELD 96 (unexercised)
    unsigned long long dive_profile_travel_alt_resets{0};
    unsigned long long dive_profile_clock_reseeds{0};
    static float land_blend_2b4(float a, float w, float b) {
        if constexpr (kLandBlendFloatOpsBound) {
            const float diff = b - a;      // FSUBP
            const float prod = w * diff;   // FMULP by +4Ch
            return a + prod;               // FADDP, FSTP +2B4h
        } else {
            return static_cast<float>(static_cast<double>(a) +
                                      static_cast<double>(w) * (b - a));
        }
    }
    unsigned long long strafe_draw_count_takes{0};
    unsigned long long moveto_c25_arm_applied{0};
    void moveto_c25_arm(GameUnitSlot& unit) {
        ++moveto_c25_arm_applied;
        unit.plan_state.pitch_target_2bc = 0.0f;   // 009C252A / 009C2769
        unit.plan_state.pitch_mode_2d0 = 2;        // 009C2532 / 009C2771
        unit.plan_state.bank_target_2c4 = 0.0f;    // 009C2543 / 009C2782
        unit.plan_heading_mode_2cc = 1;            // 009C254B / 009C278A
        unit.plan_heading_2c0_written = false;
    }
    // Packet cc9_l61_torpedo_done_formation (SQUADRON_LAND_TASK 5ho item 2). The
    // done/prepare enter 009D2530 writes squadron+3E4h = 2 (009D254E, after
    // 009BED80's 1) and state+8Ch = 1.0 (009D2565); every wingman's follow
    // station update 009BFD70 republishes its state's +8Ch into squadron+3E8h
    // (009BFDA6-009BFDB2), the scale 007F23A0 reads, before placing itself. The
    // done tick's disarmed arm 009D29E0 moves +8Ch to 1-2.5 after that, so the
    // station uses the previous tick's value. True: the enter writes shape 2 and
    // resets the slot's +8Ch, the torpedo follow base tick republishes it, and the
    // plain follow tick 009C1FD0 republishes the follow state's 1.0 (009BED80's
    // +8Ch). False: shape and +3E8h stay as they were.
    static constexpr bool kTorpedoDoneFormationBound = true;   // ON: SQUADRON_LAND_TASK 5hr
    // Packet cc9_l60_level_issue_tail (cc13-confirm D). 007C0D90's rack walk ends
    // at 007C0ED3 and falls to 007C0ED9 for the level bomber too: when the walk
    // armed a rack (the byte 007C0E23 raises) the tail sets unit+C25h = 1
    // (007C0EE2) and, unless the last rack's descriptor answers 33h (007C0EF0),
    // calls 007EEF30 on the squadron (007C0EFA-007C0F01), which hands each other
    // member 999 release orders (unit+C58h). True: the level-bomber branch runs
    // that tail through the torpedo path's binding. False: it returns after the
    // walk, as before. The per-rack 31h 007BCCF0 -> 006E3F90 point
    // (007C0E98-007C0EC9) is modelled under bsp::kLevelBombParatrooperDropBound
    // (OFF; packet cc13-para); JM08's C-47 racks answer 31h.
    static constexpr bool kLevelBomberIssueTailBound = true;   // ON: SQUADRON_LAND_TASK 5hj.2/5hj.3
    // Packet cc9_l60_level_order_spend (CC13_VER_6 findings 2 and 5, with
    // kLevelBomberIssueTailBound). 0099AF53-0099AFC2 in the pilot bot's tick, before
    // its task ticks (0099AFCC): while unit+C58h > 0, with the plane in free flight
    // ((unit+72Ch)->vt[38h] = 0074E210, unit+900h == 7), unit+5Ch set and 007B9140
    // true, one order is offered to the task's vt[24h]; the level-bomb task's
    // 009B83D0 raises task+424h when approach+CEh (task+4C6h) is set and the plane is
    // in free flight, and always answers 1, so one order is spent; any failed guard
    // zeroes C58h. 009B8BD0-009B8C3D at the end of the arm: with +424h > 0, free
    // flight and the state none of attackrun/aim/prepare/release (+5CCh/+5F4h/+610h/
    // +6B4h), 007BBBA0 and +424h -= 1. True: both run on the level-bomb task. NOT
    // MODELLED: 009B83D0's prepare arm 009B8100 (prepare's own tick 009B6670 is not
    // hosted; no row reaches prepare). False: neither runs.
    static constexpr bool kLevelBombReleaseOrderSpendBound = true;   // ON: SQUADRON_LAND_TASK 5hj.2/5hj.3
    // Packet cc9_l61_level_prepare (SQUADRON_LAND_TASK 5hs). The level-bomb prepare
    // state: the enter 009B6600 (+A0h = 0, +98h = -1, +8Ch = 1, +9Ch = class+188h x
    // 0.9, squadron+3E4h = 3 after 009BED80's 1), 009B83D0's prepare arm 009B8100
    // (+98h = 5, +9Ch = max(vt[38h], 007C47F0), approach+A8h = 007C1FB0 x 0.85) and
    // the tick 009B6670 (bsp::level_bomb_prepare_tick_009b6670: the wait arm's +8Ch
    // station scale before the follow tick, the 300 m pitch/speed hold; the drop arm's
    // countdown and release). True: all three run. False: the follow tick stands in
    // and only +A0h/+98h are reset on the enter, as before.
    static constexpr bool kLevelBombPrepareTickBound = true;   // ON: SQUADRON_LAND_TASK 5hs.1
    // Packet cc9_l63_lb_target_cleared (SQUADRON_LAND_TASK 5hv). 009B81F0's target
    // arm: the task stays valid only while +440h is 0 or the command's resolved target
    // is +440h (009B8238..009B824F). When 009F8160's dead-target retest clears a
    // member's command target, the image's next validity check returns 0 and the task
    // goes. True: a level-bomb task whose command target is cleared retires. False:
    // the arm returns early and the task freezes in its last state (USNOS wave 2).
    static constexpr bool kLevelBombTargetClearedRetireBound = true;   // ON: SQUADRON_LAND_TASK 5hv.6
    // Packet cc9_l64_attack_task_retire (SQUADRON_LAND_TASK 5hw). The same vt[40h]
    // rule in the dive-bomb (009C8060, +440h), torpedo (009D3EF0, +4C4h) and
    // drop-kamikaze (009AE140, +440h) tasks: valid only while the command's resolved
    // target is the task's own; 0099A4C0 (0099AE7E, every think) pops a task that
    // answers 0 and 0099A170 builds a new one for the current command, which needs a
    // target for all three classes. True: a cleared target retires the task (no new
    // one), and a changed target retires it and installs a fresh one. False: the
    // arms return early on a cleared target (frozen) and keep the old task on a new
    // target (dive bomb, torpedo).
    static constexpr bool kAttackTaskTargetRetireBound = true;   // ON: SQUADRON_LAND_TASK 5hw.3
    // Packet cc9_l63_lb_stick_delay (SQUADRON_LAND_TASK 5hv.3). 007C1FB0's tail: the
    // stick time is the longest rack's 006E4170 span plus BombDelay (class+1F4h) x
    // max(0, n - 1) over the n kind-25h racks. True: the term is added. False: the
    // longest span alone, as before (equal for one-rack planes).
    static constexpr bool kLevelBombStickBombDelayBound = true;   // ON: SQUADRON_LAND_TASK 5hv.6
    // Packet cc9-gunnery38 (docs/CC13_AUDIT13_RECENT_FLIPS.md O1). The strafe
    // cruise profile 009CD020, run once per activation (the install and each
    // time the strafe task resurfaces after a tightturn or flikflak retires),
    // re-seeds the leader's glide at 009CD0AA (009CA3B0: 009CA41E U(0.8, 1.2),
    // 009CA459 U(-10, 40)) inside its 009CD035 leader branch. True: the arm
    // makes that call on the activation think. False: only the install's and
    // the go-towards enters' seeds, as before. The profile's +394h/+398h
    // gates and its every-unit 009CCED0(0) call are not hosted here.
    static constexpr bool kStrafeProfileActivationSeedBound = true;   // GUNNERY 198: ON by mechanism (lead ruling a)
    // ON since packet cc9_planner_heading_writes: with the torpedo moveto and
    // attackrun heading writes bound (kPilotStateHeadingWritesBound) the Kates
    // close again; pair PHW0/PHW1, docs/PLANNER_HEADING_WRITES.md section 6.
    static constexpr bool kPlannerYawBaseModeGateBound = true;
    unsigned long long planner_yaw_base_zeroed{0};
    // DIAGNOSTIC, packet cc9_planner_heading_writes (docs/PLANNER_HEADING_WRITES.md):
    // per task/state, the planner ticks with a target, split by +2CCh on
    // entry to 0099DE8A (index 0, 1, 2; 3 = any other value).
    static constexpr bool kPlannerModeCensusDiag = true;
    // DIAGNOSTIC, packet cc9_climbout_speed_gate: once a second, every live
    // dogfight fighter below 400 m, its state, speed and pitch chain.
    static constexpr bool kFighterLowTraceDiag = false;
    // DIAGNOSTIC, packet cc9_fighter_roll_trace: once a second, every live A6M
    // Zero, its task, controls and attitude (the two undamaged Zero losses).
    static constexpr bool kZeroTraceDiag = false;
    // DIAGNOSTIC, packet cc9_kate_engagement: every two seconds, every live
    // dogfight fighter with a target, the chase geometry and speeds.
    static constexpr bool kFighterChaseTraceDiag = false;
    // Packet cc9_kate_engagement (docs/KATE_ENGAGEMENT.md): 007DA710 reads the
    // unit's pitch unit+C64h and bank unit+C68h for its slide term (007DAA3B-
    // 007DAA58, yaw -= SlideRatio * YawSpd * sin(bank) * cos(pitch)) and the
    // state-6 bank factor. This host passed zero for both, so a banked plane
    // got no bank-induced yaw: once the planner's yaw base term fades above
    // its bank limit (0099DFFB), a fighter held at 60-70 degrees of bank
    // stopped turning toward its target. OFF: both angles zero.
    static constexpr bool kRateLawAttitudeTermsBound = true;
    // Packet cc9_flight_integrator (docs/FLIGHT_INTEGRATOR.md): 007D8470's
    // velocity step in place of the host's Euler step. OFF: v += a * step.
    // ON with kDogfightThrottleBound on the pair FI0/FI1 (section 4).
    static constexpr bool kFlightIntegratorBound = true;
    // Packet cc9_terrain_arm_us_fighters (docs/TERRAIN_ARM_US_FIGHTERS.md):
    // 009A17D0's side gate compares unit+54h with the scene's DummyAIEnabled
    // (record+908h, default 3), not with the player's side. OFF: the
    // controlled unit's side stands in, which skips every friendly plane.
    static constexpr bool kAvoidanceDummyAiGateBound = true;
    // Packet cc9_stall_target_validity (docs/STALL_TARGET_VALIDITY.md): an
    // emptied target squadron clears the dogfight target, as 009AA630 hands
    // 009A7650 the null that 007F3970 leaves in +3D0h[0]. OFF: the host keeps
    // the last target, dead or not.
    static constexpr bool kDogfightEmptySquadronClearBound = true;
    // Packet cc9_lua54_follow_altitude (docs/SQUADRON_LAND_TASK.md 5gf):
    // 007CB8DC and 007CBE02 raise "powerlost" (00D059F8) through vtable[194h]
    // 007BBFA0 on a plane that reaches the water past 007BC5B0, and 007D0B80's
    // tail destroys it. OFF: the effect is a contract and a live plane floats
    // in state 6, still its squadron's leader. ON by verdict (5gf: USNOS and
    // USN12 pairs, mechanism matches).
    static constexpr bool kPlaneWaterPowerLostBound = true;
    // Packet cc9_taskless_plan_arms (docs/TASKLESS_PLAN_ARMS.md): 0099D300 runs
    // its arms for a plane with no commanded target. OFF: the host returns
    // before any arm, which froze a task-less Zero after an avoidance repair.
    static constexpr bool kPlannerTasklessArmsBound = true;
    // Packet cc9_planner_heading_writes (docs/PLANNER_HEADING_WRITES.md): the
    // torpedo states' own heading writes the host never reproduced. The moveto
    // tick 009C18C0 steers at its +2Ch target through 009F9E40 (009C1B23,
    // mode 2 at 009F9EC1); the attackrun tick 009D07B0 writes
    // AddWrapped(approach+94h, state+20h) with mode 2 (009D0927/009D092D).
    static constexpr bool kPilotStateHeadingWritesBound = true;
    // Packet cc9_attackrun_squadron_probe (docs/ATTACKRUN_SQUADRON_PROBE.md):
    // 007F0280 at 009D0857 runs in mode 0 with ECX = [approach+0Ch], the plane
    // squadron, so it walks the squadron's +3D0h member array (+3CCh entries,
    // 007F0481), skipping the unit itself and with no kind filter. The host's
    // copy of that array is the squadron registry's member_units. OFF: the
    // probe product is 0 (the planner-heading-writes substitution). ON on the
    // pair ASP0/ASP1 (docs/ATTACKRUN_SQUADRON_PROBE.md section 4).
    static constexpr bool kAttackRunSquadronProbeBound = true;
    struct PlannerModeCount { unsigned long long n[4]{}; };
    std::map<std::string, PlannerModeCount> planner_mode_census;
    // DIAGNOSTIC: every kFighterAccelTraceEvery lead ticks one line per
    // fighter comparing the two accelerations; 0 = off in the landed build.
    static constexpr int kFighterAccelTraceEvery = 0;
    // game+18ECh, the local player's slot: 0 in this single-player host.
    static constexpr std::int32_t kLocalPlayerSlot = 0;
    std::size_t role_screen_unit_plus_one{0};   // page 27h's tracked unit, +30h
    int role_takes{0}, role_releases{0}, role_refusals{0}, role_permission_writes{0};
    // A take for a role this slot already holds: 00780346's holder test fails
    // (a human slot, not 8 and not AI-held), nothing is written and neither
    // 0077C7B0 nor 0077FE80 runs (docs/SHIP_SCREEN_UPDATE.md section 39).
    int role_held_retakes{0};
    void role_message_4b_00780162(std::size_t index, std::uint32_t mask,
                                  std::int32_t player_slot, int take);
    void role_screen_update_0067bb50();
    // Packet cc9_scripted_helm_option (docs/SCRIPTED_HELM.md section 7): the
    // harness option BSP_PLAYER_HELM=<throttle>[,<rudder>], default off. A
    // MEASUREMENT SCENARIO, never a reference: it opens EROLF_PILOT on the
    // controlled unit to PLAYER_ANY through 0077F360 as a script call would,
    // performs 0064B870's role-1 transfer, and issues the helm every step.
    bool helm_option_read{false};
    bool helm_option_on{false};
    float helm_throttle{0.0f}, helm_rudder{0.0f};
    bool helm_opened{false};
    float helm_lever_24{0.0f}, helm_lever_28{0.0f};   // HUD +24h thrust, +28h turn
    int helm_transfers{0}, helm_issues{0};
    // Packet cc9_helm_orders_helm_route: --helm-orders `takehelm`.
    bool helm_route_on{false};
    bool helm_route_arrived{false};
    float helm_route_x{0.0f}, helm_route_z{0.0f};
    // Packet cc13-helmmapgate: set when the HUD host's map occludes screen 46h.
    // The counters are fixed steps on which prepare and issue would run with the
    // map up, and steps on which the gate skipped them.
    bool helm_screen_occluded_by_map{false};
    unsigned long long helm_map_up_prepares{0}, helm_map_up_issues{0};
    unsigned long long helm_map_up_skipped_prepares{0}, helm_map_up_skipped_issues{0};
    void player_helm_prepare_0064b870();
    void set_role_availability_00927d20_impl(std::size_t index, std::uint32_t mask,
                                             std::int32_t value);
    void player_helm_issue_0064b870(GameUnitSlot& slot);
    static constexpr bool kPilotAvoidanceUpdateBound = kPilotGunfireAvoidanceBound ||
        kPilotVehicleAvoidanceBound || kPilotTerrainAvoidanceBound;
    // DIAGNOSTIC, packet cc9_wing_achieved_speed: every kWingTraceEvery motion
    // steps, one line per member of the squadron named kWingTraceSquadron.
    // Off (0) in the landed build.
    static constexpr int kWingTraceEvery = 0;
    static constexpr const char* kWingTraceSquadron = "B5N Kate #4.1";
    // The fly-to arm's last two plan stores, 009BFD15 plan+2B0h = 0 and
    // 009BFD1C plan+2D8h = 1, after the desired speed at 009BFD0F. Without
    // +2D8h = 1 an airborne plane (flight state 7) never enters 0099D300's
    // speed-demand arm, so the follow law's desired speed was never read.
    // Packet cc9_follow_speed, docs/PLANE_FOLLOW_SPEED.md.
    static constexpr bool kPlaneFollowFlyToSpeedStores = true;
    // The fly-to arm's pitch through 009F9ED0 (packet cc9_follow_pitch) rather
    // than the 009FB800 substitute, which this binding fed the commanded
    // altitude as its dimensionless `reference` and so ran bang-bang at the
    // +0.698 / -1.047 caps. docs/PLANE_FOLLOW_PITCH.md.
    static constexpr bool kPlaneFollowFlyToPitch = true;
    // The pitch mode 009FB800 writes (cmd+2D0h = 2) at the three 009FBA50 seams
    // that left it out. Packet cc9_pitch_callers, docs/PITCH_COMMAND_CALLERS.md.
    static constexpr bool kPitchCommandCallersBound = true;
    // Packet cc9_torpedo_run_pitch_profile (docs/PLANE_BODY_RATES.md section 2).
    // True: 0099DC9E-0099DD5A, the planner's pitch-mode-2 arm. At a cached speed
    // of 10 km/h ([00D1F3D8] 2.7778) or less, mode 2 demotes to 1. Otherwise the
    // held angle is unit+C84h, the flight-path angle 007C18B0 derives from the
    // velocity at +AE0h (atan2 of the vertical over the horizontal speed);
    // inc = 00419010(007C4810 min control speed, DEG(5), TravelSpeed +18Ch,
    // DEG(20), speed); when the live pitch C64h exceeds held + inc, the pitch
    // target +2BCh becomes held + inc and the measured angle is the live pitch,
    // else the measured angle is the flight-path angle. Mode 1 measures the live
    // pitch. False: the live pitch in every mode, no limit, as before.
    // ON by the pairs at b2e64ab63: USN04 and USN01 move, USN02 identical.
    static constexpr bool kPlanePitchModeTwoBound = true;
    // The dogfight task's skeleton (packet cc9_dogfight_task): install, the
    // 009AAFA0 unengaged arm (moveto leader / follow wing), the generic follow
    // tick, and a labelled moveto stand-in. docs/DOGFIGHT_TASK.md.
    static constexpr bool kDogfightTaskBound = true;
    // Packet cc9_dogfight_engaged: 009AAC70's selection and latch, all of
    // 009AAFA0, the aim state, and labelled maneuver/avoid stand-ins.
    // docs/DOGFIGHT_ENGAGED.md.
    static constexpr bool kDogfightEngagedBound = true;
    // Packet cc9_dogfight_gun: the task gun controller 009FC7C0 (fire decision
    // and burst clock, census only: the gunFire consumer is not established),
    // the head-on and maneuver throttle 007B4ED0. docs/DOGFIGHT_GUN.md.
    // Packet cc9_plane_gunfire: 0099B450's per-think re-seed of plan+2B4h,
    // +2B0h and +2D8h. docs/PLANE_GUNFIRE.md.
    static constexpr bool kPilotPlanReseedBound = true;
    static constexpr bool kDogfightGunBound = true;
    // Packet cc9_plane_gunfire: the unit+C50h enemy-aircraft list (007E11D0,
    // 3 s refresh) and its finder 007E2090/007DEEC0 as the gun's +74h target.
    static constexpr bool kPlaneFinderBound = true;
    // Packet cc9_near_field_probe: 007F0280 at the dive-bomb attack run
    // (009C42B8) and the fly-over speed slot (009C6B3A). docs/NEAR_FIELD_PROBE.md.
    static constexpr bool kNearFieldProbeBound = true;
    // cc13-audit10 (docs/CC13_AUDIT_PLANNER_ROLL.md finding 1): the roll arm's
    // h is the yaw base numerator [ESP+6Ch] (0099DF87), not the raw heading
    // error. 0099E127 loads it for the deadband and the bank ask, and 0099E02B
    // for the pitch-floor ramp. Behaviour-changing; pair before ON.
    static constexpr bool kRollArmBaseNumeratorBound = true;  // GUNNERY 193: ON by pair verdict
    // Packet cc9_plane_gun_pass: 009AAA80's early maneuver -> aim edge, and the
    // latched gunFire (unit+BC9h) the plane's fixed step hands to every gun's
    // SetTriggerHeld (007CE9A0-007CE9F4). docs/PLANE_GUN_PASS.md.
    static constexpr bool kDogfightEarlyEdgeBound = true;
    static constexpr bool kPlaneGunfireBound = true;
    // Packet cc9_plane_flight_natives. docs/PLANE_FLIGHT_NATIVES.md.
    // The aim tick's head-on test reads vtable[34h] = 007BBB70, unit+AC8h, the
    // world linear velocity, for both aircraft (not the forward rows).
    static constexpr bool kDogfightHeadOnVelocityBound = true;
    // 007CE9FD's release-issue predicates: session mode, +C3Ah, +5Dh, BombDelay.
    static constexpr bool kPlaneFixedStepPredicatesBound = true;
    // Packet cc9_l66_issue_draw (docs/CC13_VERIFY1.md A.1). True: after each
    // 007C0D90 issue the next-issue interval is U(0.9, 1.1) x BombDelay (007CEAB3,
    // one keyed stream-1 draw per issue). False: always 0.9 x BombDelay, no draw.
    static constexpr bool kReleaseIssueIntervalDrawBound = true;  // ON: SQUADRON_LAND_TASK 5if.1
    unsigned long long release_issue_interval_draws = 0;
    // Packet cc9_plane_device_walk, docs/PLANE_DEVICE_WALK.md: 007CEB00-007CEB1D,
    // the cleanup arm's walk over the rack list asking vtable[1FCh], and the
    // busy byte the fire slot vtable[1F0h] (006E3550) raises at 007C0E17.
    // ON since the pair (identical rows; the walk is not reached here). OFF: the
    // walk stays the record Plane::device_busy_1fc and nothing is busy.
    static constexpr bool kPlaneDeviceWalkBound = true;
    // Packet cc9_release_issue_stage, docs/RELEASE_ISSUE_STAGE.md: a torpedo leaves
    // the rack, not the request. 007BBBA0 only opens the bay and counts unit+C20h;
    // the stage's issue 007C0D90 arms the first idle rack (006E3550: toRepeatTime
    // += delay, dropBombs = 1); the rack tick 006E56F0 drops through 007CC8E0 /
    // 007C7600 and the fire message into 006E4D50, and clears dropBombs once its
    // ammo is spent. OFF: the torpedo spawns at the request (the old stand-in).
    // Torpedoes only; the dive bomber's bombs keep their spawn at the request.
    // ON since the E2 9000 / USN04 4500 pairs (docs/RELEASE_ISSUE_STAGE.md).
    static constexpr bool kReleaseIssueStageBound = true;
    // Packet p19_bay_force (docs/OCEAN_WAVE_FIELD.md section 76; section 69's
    // MINOR a). True: a disabled channel C stays disabled at the request, as
    // 007BBBA6-007BBBB0 skips the bay writes and still reaches 007BBC00's +C20h
    // raise; the rack stage spends +C20h either way. False: the host enables
    // channel C (rate 1.0) before the request, so the bay opens.
    // With it, wing_fold_class_007eabc0 enables channel C from PartAnims.BayDoor
    // as 007EABC0 does (class+5F4h), so only a class without bay doors keeps it shut.
    static constexpr bool kBayRequestNoForceEnableBound = true;  // ON 2026-10-18 (OCEAN_WAVE_FIELD 76)
    unsigned long long bay_channel_disabled_requests{0}, bay_door_classes{0};
    // Packet cc9_release_issue_stage_vals, docs/RELEASE_ISSUE_STAGE.md section
    // "Vals": the dive bomber's bomb leaves the same rack through the same stage.
    // 007BBBA0 raises unit+C20h once per round (the aimglide's 009C5771-009C5784
    // loop calls it per round); the stage spends one request per reloaded
    // interval into 007C0D90, which arms the D3A's single BombPlatform (device
    // class 87, Ammo 1 in this installation); the rack tick drops the bomb
    // ballistically (006E4D50 carries no aim point for a dive bomber: the target
    // point setter 006E3F90 is called only from the level bomber's branch
    // 007C0EC9). The roll curve of 007C7600 (00419010 over tuning +554h..+560h)
    // is bound under this switch too. OFF: the bomb spawns at the request.
    // ON since the USN04 9000 / 4500 pairs: every behaviour row identical, the
    // dead Vals' requests held by the stage's guard. The live Val's drop has no
    // run-time evidence yet (no mission here has a live Val release).
    static constexpr bool kReleaseIssueStageValsBound = true;
    // Packet cc9_dive_bomb_carried_rounds, docs/RELEASE_ISSUE_STAGE.md section
    // "Carried rounds": 007C1DB0 sums 006E3500 (loaded round + ammo +484h) over
    // the aircraft's IsKindOf(25h) racks. ON: the host's count is the census's
    // single racks' authored rounds (this installation's
    // VehicleClass[id].Equipments[DefaultEquipment][platform key].Ammo) until the
    // issue's first check, then the rack's own ammo, which only the rack drop
    // spends. OFF: kDiveBombCarriedRoundsSubstitute (2), spent at the request.
    // ON since the USN04 9000 / 4500 pairs: only the dead Val #1.1|.-4's rows
    // moved (its two-round glide became one), deaths and hits identical.
    static constexpr bool kDiveBombCarriedRoundsBound = true;
    // Packet cc9_avoid_zone_registry, docs/AVOID_ZONE_REGISTRY.md: 0041BC20
    // samples the avoid-zone layer at squadron+34Ch (0041DF40(1.5, true), the
    // scene `.nav`'s tan(70 deg) layer) at the fixed step's unit+9B4h writer
    // 007CE8FF, AvoidTerrain's 0099F9F3 / 009A05DF / 009A0E1A and the torpedo
    // approach's 009D36A9 / 009D386D / 009D3A42. OFF: the water surface stands in.
    // ON since the USN04 9000 / 4500 and USN02 pairs: the Yorktown's fighters
    // now fly terrain avoidance over the eastern block; no death, hit or release
    // moved, and USN02's ship rows are identical.
    static constexpr bool kAvoidZoneLayerSampleBound = true;
    // Packet cc9_terrain_avoid_forward_speed (docs/SQUADRON_LAND_TASK.md 5da):
    // AvoidTerrain's unit vtable[204h] (0099F27E, 0099CC98) is ctl+7Ch, the body
    // forward speed 007D80C0 stores, where the host passed |v|. OFF: |v|.
    // ON since the 5da.1 pairs: USNRM01's Kates keep full throttle through the
    // slip and drop at Pennsylvania; USN02 identical; USN04 an RNG cascade.
    static constexpr bool kTerrainAvoidForwardSpeedBound = true;
    // Packet cc9_units_contracts, docs/AVOID_ZONE_REGISTRY.md section "Units
    // contracts": (1) unit+9B8h, the fixed step's second 0041BC20 sample
    // (007CE92A), stored per plane; no host path reads it (its readers are the
    // unreferenced 007B9620, the state-message arms 007C2B6B / 007D18CE and the
    // alternate pose 007DA24F); (2) 007F10B0's push of a squadron into world
    // list 24 (+138h), the squadron's +130h registrar. OFF: neither.
    // ON since the USN04 4500 / USN02 pairs: gameplay identical, list 24 = 21
    // squadrons on USN04, unit+9B8h stored with no reader.
    static constexpr bool kUnitsContractsBound = true;
    // Packet cc9_ship_motion_tail, docs/SHIP_POST_MOTION.md section 8a: the tail
    // 00826C34..00826D69 of 00825F20 (ship_motion_tail_00826c34) replaces the
    // host's wake-append block. It stores unit+1050h, feeds the wake the image's
    // yaw rate 00811890(unit, [unit+984h]) (the ORDERED rudder, ring+14Ch)
    // instead of the provisional steering rate, and runs the leader's
    // unit+1158h timer (Formacio.UpdateInterval) whose expiry calls 0070DB60,
    // a record until kFormationSlotSwapBound. OFF: the old wake block.
    // ON since the USN02 / USN04 9000 pairs: every moved row inside its band.
    static constexpr bool kShipMotionTailBound = true;
    unsigned long long tail_slot_swap_calls = 0;
    unsigned long long tail_expiry_calls = 0;
    float formacio_update_interval = -1.0f;   // -1 until read
    float formacio_ship_dist = -1.0f;         // packet cc9_l66_putto; -1 until read
    unsigned long long putto_station_places = 0;   // packet cc9_l66_putto
    unsigned long long putto_headings = 0;
    unsigned long long putto_reshapes = 0;
    // Packet s53_wreck_debris: ShipGlobals.DebrisStruct (0084124E / 00841294 store
    // settings+680h / +684h; defaults false and 0.3). -1 until read.
    int debris_enabled_680 = -1;
    float debris_num_per_meter_684 = 0.3f;
    // Packet cc9_ship_motion_tail part 8b: 0070DB60 on the group the expired
    // leader timer names. OFF: a record.
    // ON since the USN02 / USN04 pairs: identical; both missions gate every call.
    static constexpr bool kFormationSlotSwapBound = true;
    // Packet cc9_formation_join_follow (docs/SHIP_UNIT_GROUP_FOLLOW.md section 5g).
    // 0077F940's tail 0077FA81..0077FABF: once per brought unit, after 0070EF30,
    // ordered->vtable[114h]() (the director) and, when non-null, director->vtable[58h]
    // = 00720CD0 with [ESP+80h], the target group's leader (0077F9E6): clear the ten
    // slots and push `follow` 00E08F60. ESI is the ORDERED unit on every pass, so the
    // brought members get nothing. OFF: the follow-up is skipped (counted).
    static constexpr bool kFormationJoinFollowBound = true;  // ON: pairs held (docs/SHIP_UNIT_GROUP_FOLLOW.md 5g)
    // Packet cc9_ship_motion_tail part 8c: 00826B84's unit->vtable[1ECh](dt) is
    // 008160B0 on every ship class, the repair tick whose work (0093CA20) the
    // gunnery host's run_damage_control already runs, after the projectile pass.
    // ON: the row evaluates 008160B0's entity gate (+5Ch set; +5Dh, +60h, +5Eh
    // clear) and is concrete; the class flag +D0h has no host field and is taken
    // as set. OFF: the record. Bookkeeping only; the repair order is not moved.
    // ON since the USN02 / USN04 pairs: identical gameplay.
    static constexpr bool kShipPostMotionRepairOrder = true;
    unsigned long long post_motion_gate_passes = 0;
    // Packet cc9_ground_height_hunks, docs/SCENE_CONTENTS_HOSTS.md section 7:
    // the torpedo aim tick's 00903860 (009D16D1 / 009D2265) and the torpedo
    // approach's 00903BC0 probe (009D39D3) answer from the scene's world queries
    // (-1000 over no terrain, the Landscape sweep). OFF: 0.0 and the water
    // surface as the ground.
    // ON since the USN01 / USN04 pairs: identical gameplay (0 blocked probes both sides).
    static constexpr bool kGroundHeightHunksBound = true;
    // Packet cc9_terrain_segment_consumers, docs/SCENE_CONTENTS_HOSTS.md section 12:
    // 0099F1C0's branch A (009A1420..009A17A1), a live plane on the water (unit+900h
    // == 6): the alternating +-30 degree heading probe through 00903BC0 at y = 0.1
    // and, when it is blocked, the 15-degree fan 3,-3,4,-4..12,-12,13, the band
    // 0099B790 on pilot+4h and the throttle pilot+25Ch. OFF: branch A returns before
    // probing (the earlier labelled substitution).
    // ON since the USN01 / USN04 / USN02 pairs: identical, and branch A made 0
    // probes on all three (no live plane in mode 6), so it is bound but unexercised.
    static constexpr bool kAvoidTerrainWaterProbeBound = true;
    // Packet cc9_mavis_rack_drops, docs/RELEASE_ISSUE_STAGE.md section "Mavis rack
    // drops": 007C0D90 fires ONE rack per issue (007C0E17 vtable[1F0h] = 006E3550
    // on the first IsKindOf(25h) child holding 2Ah and not busy, then returns for
    // any aircraft that is not a level bomber 10h), and that rack's tick 006E56F0
    // clears its dropBombs once ITS ammo [ESI+174h] = rack+484h is spent
    // (006E58D8-006E58E1). ON: the host keeps one ammo count per single rack and
    // the issue fires the first rack with rounds left, so one issue is one drop
    // for a rack of one round. OFF: the single racks' rounds are one pool, and one
    // issue drops until the pool is empty (an H6K Mavis' four racks: four drops).
    // ON since the USN01 / USN04 pairs (identical) and a USN01 pair with the layer
    // sample OFF on both sides: Mav1 and Mav4 drop once instead of four times,
    // the same two torpedoes spawn, gameplay identical.
    static constexpr bool kRackRoundsPerRackBound = true;
    // Packet cc9_lua46_rack_pool_rounds (SQUADRON_LAND_TASK 5ex). 007C1DB0 sums
    // 006E3500 (the loaded round plus ammo +484h) over the single racks. True: an
    // aircraft whose census found single racks but no authored Ammo (IJN01's A7M,
    // class 150 DefaultEquipment 0, bag Equipment 0) answers the rack pool the
    // release spends (one round per rack before the first issue), as an authored
    // one already does. False: it answers the task's substitute 2, which the
    // salvo's request spends whole, so one drop zeroes it with a rack still loaded.
    // ON by its IJN01 pair (5ex): every A7M_2 / A7M_4 member drops both racks.
    static constexpr bool kRackPoolRoundsRemainingBound = true;
    // Packet cc9_lua46_unequipped_racks (SQUADRON_LAND_TASK 5ey). 007CDF20's kind-1
    // arm (a squadron's planes share its cloned bag, 007F48FD / 00922DE0) calls
    // 0095A880(null, 1, 0) when the bag's Equipment is <= 0: only the default
    // devices are added and no rack is handed Ammo (vtable[1BCh]) or a round, so
    // 006E4060 / 006E3FE0 answer no rack kind in single player. True: a plane
    // whose carried bag Equipment is 0 (kPlaneSceneEquipmentBound) stores no
    // rack-borne kind (2Ah 2Bh 2Ch 2Dh 31h 33h) and its rack pool seeds empty.
    // False: such a plane's racks take the pool's one-round fallback.
    // REFUTED, stays OFF (5ey): the rack's constructor 006E3C00 stores ammo +484h
    // = orgAmmo +488h = 1 (006E3C20..006E3C3D), so a default rack 0095A880's flag
    // pass adds holds one round; the pool's fallback is the image's default.
    static constexpr bool kRackUnequippedEmptyBound = true;
    // Packet p13_bag0_round (docs/OCEAN_WAVE_FIELD.md 46). 007B9140's rack walk
    // asks each rack vtable[210h](2Ah), which answers while a round is attached.
    // A rack's owner bind 006E3350 (vtable 9Ch) skips the loadout walk when the
    // plane's +C54h is <= 0 (006E3386 JLE 006E33E3) and still tail-jumps
    // vtable[224h] 006E3E70, which attaches a round from the constructor's ammo
    // +484h = 1 (006E3C20..006E3C37). True: an unseeded single rack of a plane
    // whose carried bag Equipment is 0 answers one round, as the release path's
    // pool fallback already seeds it. False: it answers none (authored -1), so
    // 009F8160 finds such a squadron spent before it has dropped anything.
    // ON: OCEAN_WAVE_FIELD 46.1 (ESMP08 dive-bomb releases 1 -> 100, every
    // +3Dh end after the squadron's drops; USN04 gameplay identical).
    static constexpr bool kRackConstructorRoundBound = true;
    // Packet cc9_p14_bag_rack_round (docs/OCEAN_WAVE_FIELD.md 54). True: a single
    // rack (BombPlatform) of a plane whose carried bag is positive, with no entry for
    // the rack's platform key in that loadout, keeps its constructor round: 006E3350
    // walks 009552E0's entries and calls SetAmmo (vt[1BCh]) only on a key match
    // (006E33AA-006E33B6), then attaches the next round (006E33FE vt[224h]) with the
    // ammo 006E3C00 stored, 1. Both the 007B9140 answer and the release seed see it.
    // False: such a rack holds nothing. LABELLED: single racks only, as for bag 0.
    static constexpr bool kRackPositiveBagRoundBound = true;   // ON by its pairs (OCEAN_WAVE_FIELD 54.5)
    // Goal 5 chain 657: 007DC830 itself runs at both native sites (PlaneBinding::
    // free_flight_step_007dc830): the stall block (packet cc9_p14_stall_force,
    // docs/OCEAN_WAVE_FIELD.md 55), the dying spin and the wreck lift (packet
    // p15_dying_spin, section 58). Counters for the report. LABELLED: the dying spin
    // ctl+F0h..F8h is 007D7EA0's construction draw, U(+250h, +254h) (007D7F90; the
    // loader fills both from Dynamics/DeadMeat/RotationMin, RotationMax is never
    // read) times 00439950's direction (z = U(-1, 1), phi = U(0, 2 pi)), all on
    // stream 1, drawn here from the plane's keyed stream (`name#ds_m`, `#ds_z`,
    // `#ds_p`) at its first dying step, not at construction.
    unsigned long long stall_force_steps{0}, stall_force_kicks{0};
    unsigned long long dying_spin_steps{0}, dying_spin_seeds{0}, wreck_lift_kicks{0};
    unsigned long long rack_positive_bag_rounds{0};
    int rack_positive_bag_round(const GameUnitSlot& s, int ammo) {
        if constexpr (kRackPositiveBagRoundBound && bsp::kPlaneSceneEquipmentBound) {
            if (s.bag_equipment > 0 && ammo < 0) {
                ++rack_positive_bag_rounds;
                return 1;   // 006E3C20 MOV EAX,1, kept by 006E3350's walk
            }
        }
        return ammo;
    }
    // Packet p13_neighbour_clock_draws (docs/OCEAN_WAVE_FIELD.md 47). The unit+C50h
    // constructor 007E1E20 starts its three clocks with stream-1 draws (00BD2F10,
    // ECX = 1, at 007E1F94 / 007E1FB9 / 007E1FE1): +7Ch = U(0, 3) + 3,
    // +84h = U(0, 2) + 2, +80h = U(0, 1) + 1 (the periods +88h, +8Ch, +90h added
    // by FADD). True: each plane's clocks take those draws from its keyed stream
    // (`name#nb7c` / `#nb84` / `#nb80`) at its first neighbour use, and every
    // host copy of +7Ch takes the one value. False: the midpoints 4.5 / 3.0 / 1.5.
    // LABELLED: seeded at first use, not at 007D621F's construction; the sum is
    // taken in float, the image's in x87 before the FSTP.
    // ON: OCEAN_WAVE_FIELD 47.1 (ESMP08 / USN04 move from the first fighter contact).
    static constexpr bool kNeighbourClockDrawsBound = true;
    void nb_seed_clocks(GameUnitSlot& u) {
        if constexpr (kNeighbourClockDrawsBound) {
            if (u.nb_clocks_seeded) return;
            u.nb_clocks_seeded = true;
            const float c7c = stream_set2_draw_00bd2f10(u, u.row.name + "#nb7c", 0.0f, 3.0f) + 3.0f;
            const float c84 = stream_set2_draw_00bd2f10(u, u.row.name + "#nb84", 0.0f, 2.0f) + 2.0f;
            const float c80 = stream_set2_draw_00bd2f10(u, u.row.name + "#nb80", 0.0f, 1.0f) + 1.0f;
            u.nb_clock_30 = c7c;
            u.nb_clock_70 = c7c;
            u.nb_clock_7c = c7c;
            u.nb_clock_84 = c84;
            u.nb_clock_80 = c80;
            ++nb_clock_seeds;
        } else {
            (void)u;
        }
    }
    unsigned long long nb_clock_seeds{0};
    // Packet cc9_rack_bullet_kind (SQUADRON_LAND_TASK 5eq, GUNNERY 138). True:
    // 007C0D90 decides per rack from the rack's projectile descriptor (a level
    // bomber 10h fires its plain-bomb racks together; 2Bh/2Ch/33h, or any rack
    // of a non-10h plane, takes the single-drop arm), the drop spawns by the
    // fired rack's own class, and 006E58AA re-arms toRepeatTime from the
    // rack's RepeatTime. False: both arms key on the unit's ordnance mask,
    // whose torpedo bit the gunnery host clears after one drop, and the single
    // rack's toRepeatTime is 0. ON (SQUADRON_LAND_TASK 5fa): gameplay identical to
    // kGunneryRackEquipmentDeviceBound alone on USN13 9000, USNOS and LOMP10,
    // with the 2Ah/0.05s racks found by the gunnery rows (refusals=0).
    static constexpr bool kRackBulletKindBound = true;
    // Packet cc9_lua45_rack_live_ordnance (SQUADRON_LAND_TASK 5eu). The image's
    // loadout tests ask each device live: 007B9320 / 007B91C0 call the rack's
    // vtable[220h] / [210h] (MBombPlatform 006E4060 / 006E3FE0, vtable 00CF96A8),
    // which answer from the rack's attached child round (+48h list, IsKind via
    // child vtable[5Ch], descriptor child+314h), or, only when the owner's
    // vtable[1E4h] (006D1F40: the global byte 00E17BF2, 00 in single player) and
    // the loadout flag are both set, from the rack's bullet descriptor. So an
    // emptied rack stops answering. True: unit_ordnance() drops each single-rack
    // kind whose racks are all empty once the rack ammo is seeded, and the
    // general-bomb bit 2Ah when no rack of the bomb family (2Ah, 2Bh, 2Ch, 2Dh,
    // 31h, 33h; their descriptors answer 2Ah too, 007B9320's exclusions) has a
    // round left. LABELLED: "ammo > 0" stands for "a round is attached"; planes
    // with a MultiBombPlatform or an unknown rack kind keep the static mask.
    // False: the static OR of the device kinds, as before. ON by verdict (5eu):
    // IJN01 moves through the two A7M squadrons that emptied both racks.
    static constexpr bool kRackLiveOrdnanceMaskBound = true;
    // Packet cc9_rack_device_reload (docs/SQUADRON_LAND_TASK.md 5go). True: the
    // single rack's in-flight reload, 006E56F0's slice 006E57A4-006E57D2, runs
    // while SetDeviceReloadEnabled's byte [00E17BF2] is set: an empty rack gets
    // its rounds back ReloadTime seconds (the equipment entry's, default 30, floor
    // 1) after it emptied. False: an emptied rack stays empty, as before.
    static constexpr bool kRackDeviceReloadBound = true;   // ON: SQUADRON_LAND_TASK 5go.1
    // Packet cc9_rack_device_reload, part 2 (5go). True: both feeds of the dive-bomb
    // break-off 009C8A90 pass ctl+369h and [00E17BF2], so with device reload set
    // (step 4, 009C8ABE-009C8ACD) a live target never breaks the attack off, as
    // the torpedo feed (cc9_device_reload_feeds) already does. False: both read
    // false, so a spent dive bomber breaks off to `done` whatever the flag.
    static constexpr bool kDiveBreakOffReloadFeedBound = true;   // ON: SQUADRON_LAND_TASK 5go.1
    // Packet cc9_dkz_spent_rtb (docs/CC13_GPR_38.md 1.4-1.6, SQUADRON_LAND_TASK 5gq).
    // True: the torpedo task is installed only under the torpedo command 00E08F18
    // (0099A170's class chain), and 009F7C90's ending asks a drop-kamikaze member
    // its 009AE1B0. So a spent BettyOhka squadron reaches 0084E010's B5 and is
    // sent home. False: every plane with 2Bh ordnance and a target also gets a
    // torpedo task, whose break-off answers for the Betty.
    static constexpr bool kDropKamikazeSpentRtbBound = true;   // ON: SQUADRON_LAND_TASK 5gq.1
    // Packet cc9_l57_moveto_self (docs/CC13_VER_10.md D1, model src/airops_ver_cc13.cpp):
    // 0084E010's tail sends a squadron to 0084E595, `0071ECF0(00E08F68, 00465080(
    // squadron, 0.0))`, a moveto on the squadron itself, from three arms: the idle
    // arm b4 (a command ended at stage 2 without the latch), the resolve gate
    // (+369h and [00E17BF2] both set) and a null 007F16D0 answer. A moveto whose
    // target is the squadron stays at stage 1 (0084E48A), so the flight loiters on
    // itself. False: those squadrons get no order (the host's B5 only).
    // ON (SQUADRON_LAND_TASK 5hm): unit+184h is the player flag, so an AI squadron's
    // moveto-self is raised to stage 2 at once (0084E0A4) and the image churns too.
    static constexpr bool kSquadronMoveToSelfBound = true;
    // Packet s65_moveto_self_wing_follow (SHIP_AI 329.2, 330). 0084E5A8 puts the
    // moveto-self into the squadron's own director, and each wing's bot takes it
    // through 0099A170's member arm: the kind-7 moveto task (009C3000) runs the
    // follow state 009C1FD0 on the leader for every non-leader member. SUBSTITUTION,
    // labelled, as 009F8615's stop head (cc13-stophead): the host stands each member in
    // for the director, so the moveto-self also goes to every live wing member once it
    // is placed. False: the leader's row only, and the wings, whose rows
    // retire_ended_command_rows stripped, idle on the director tail and fly straight.
    static constexpr bool kSquadronMoveToSelfWingFollowBound = true;
    // Packet cc9_lua45_rack_equipment_device (GUNNERY 144, SQUADRON_LAND_TASK 5ev).
    // True: (a) an absent RepeatTime reads as 006E0242's default, 00B66330 with
    // [00CE3D30] = 0.6f, at both rack readers; (b) a rack's device is the plane's
    // own equipment entry, VehicleClass[c].Equipments[unit+C54h][slot].Platform
    // (C54h 1-based, the equipment reader 00961F57 stores Platform per entry), and
    // the class default p%d_dev only when C54h is 0 or the entry has no Platform.
    // A slot with no recorded bag keeps `DefaultEquipment or 1`, as the ammo
    // reader does. False: the class default device and a 0.0 RepeatTime. ON (5ev):
    // identical on USN13 9000, USNOS and LOMP10 with 138 OFF.
    static constexpr bool kRackEquipmentDeviceBound = true;
    // Packet cc9_sunk_ship_kill_depth, docs/CONSTRUCT_WORLD.md section 24: while
    // +5Dh is set, 00825F20 advances sinkTime +828h by dt (008263C1..008263DC)
    // and, once both hull ends y +/- forward.y * 0.5 * class+A0h lie below
    // GameSettings+3F4h (KillDepth), calls Kill 00926D90(1) at 00826628 and
    // returns (00826642). The kill list's drain then removes the entity
    // (009263C0) and calls vt[80h] 00951FB0 -> 00928C80, whose 00928F24 calls
    // vt[134h], the ship's world-list deregistration (the mirror of the +130h
    // registrar). ON: all of that, the removal and the unlink applied at the
    // kill (SUBSTITUTION, labelled: the host's row-15 kill list cannot take a
    // host entity). OFF: nothing; the census of hull ends prints in both builds.
    // ON since the USN02 and E2 9200/9000 pairs: identical gameplay. Host wrecks do
    // reach KillDepth (docs/CC13_VER_5.md section 5).
    static constexpr bool kSunkShipKillDepthBound = true;
    // Packet cc9_ship_terrain_contact (docs/SHIP_AI_OPEN_ITEMS.md section 87).
    // The image stops a hull on the terrain in the Dyn library's contact phase
    // (00C5BB5F..00C5C455, an LCP solver, unread); its callback 009377E0 only
    // latches +1010h on a kind-8 contact. SUBSTITUTION, labelled: after the
    // position phase, three keel points of the hull box (bow, middle, stern at
    // the box's min y, through the pose) are tested against the terrain height
    // 00903860; a point below the ground is a contact (the latch, both builds).
    // True: at a point whose penetration the step deepened, the step's
    // horizontal displacement and the linear velocity lose their uphill
    // component (the terrain gradient), so a hull cannot climb onto land and
    // slides along a shore; a hull already resting in contact moves freely on
    // the level or away. False: the latch and the census only; hulls cross land.
    // ON by section 87.5: the landers beach and lower their ramps 2 s after the first
    // contact; the no-contact rows are gameplay identical.
    static constexpr bool kShipTerrainContactBound = true;
    // SUBSTITUTION, labelled: GameSettings+3F4h is not loaded into this host
    // (the reader 0083EA71 is in the Lua host's settings load, not bound).
    // This installation's scripts/datatables/shipglobals.lua line 377
    // (2024-07-13): VizbeomlesDolgok.KillDepth = -200.0.
    static constexpr float kKillDepthSubstitute = -200.0f;
    // Packet cc9_units_push_pending, docs/CONSTRUCT_WORLD.md section 26:
    // create_units pushes each instance it constructs onto the pending list, as
    // 00928760 does. OFF: only the Lua routes and the load walk push.
    // ON since the USN04 4700/4500 and USN02 9200/9000 pairs: every counter as
    // predicted, InitAll rows and gameplay identical.
    static constexpr bool kUnitsPendingPushBound = true;
    unsigned long long construction_pushes = 0;
    unsigned long long squadron_pass_a_hook_calls = 0;  // packet cc9_squadron_pass_hooks
    unsigned long long scene_home_contract_calls = 0;   // packet cc9_scene_home_base_contract
    // Packet cc9_wing_construction: the held-back wings, by leader index.
    std::map<std::size_t, std::vector<GameSceneEntityRecord>> staged_wings;
    unsigned long long wing_records_staged = 0;
    unsigned long long wing_hook_builds = 0;
    unsigned long long wing_planes_built = 0;
    unsigned long long squadron_pass_c_hook_calls = 0;
    // Packet cc9_squadron_member_placement: 007F2920 at pass C (007F4DB0, an
    // airborne squadron: +408h clear). OFF: the wing stays where 007F4580 put it.
    // ON (docs/SQUADRON_MEMBER_PLACEMENT.md section 5): USN04 4500, LOMP10 9000.
    static constexpr bool kSquadronPassCPlacementBound = true;
    unsigned long long squadron_member_place_calls = 0;
    unsigned long long squadron_members_placed = 0;
    unsigned long long squadron_member_place_unseeded = 0;
    unsigned long long squadron_member_place_no_station = 0;
    unsigned long long squadron_member_place_pass_c = 0;
    // Packet cc9_squadron_initial_command, docs/CONSTRUCT_WORLD.md section 27:
    // the squadron's pass C default order at 007F4E9E. OFF: none.
    // ON since the USN04 4700/4500, E2 9200/9000 and USN02 pairs: 4 movetos (12
    // member orders) on USN04, gameplay identical on all three.
    static constexpr bool kSquadronInitialCommandBound = true;
    unsigned long long initial_command_movetos = 0;
    unsigned long long initial_command_stops = 0;
    unsigned long long initial_command_member_orders = 0;
    unsigned long long initial_command_skipped_current = 0;
    unsigned long long initial_command_no_home = 0;
    // Packet cc9_ship_sink_descent, docs/CONSTRUCT_WORLD.md section 25. ON: the
    // leak manager at unit+10D4h is built (0074F490 with controller+84h from the
    // displacement sum 00937DBB..00937F74 over the hull's element list), the
    // wreck handler's sink block 00824FE5..00825086 runs at the row-15 flush
    // (0074EC50's rate redistribution, inertia x2, angular damping 2.5, linear
    // damping 0.5) and the hydrodynamics' leak tick 0074F930 and heel torque
    // 0074F2E0 run over the six leaks, so a wreck takes on water, the flooding
    // weight 00933A3A pulls it down and the KillDepth kill can fire. OFF: the
    // leak model has no entries (today's empty list).
    // ON since the USN02 / E2 9200/9000 pairs: 17 USN02 wrecks pass -200 m
    // (91.6..98.6 s after +5Dh) and are killed, unlinked and released; E2 identical.
    static constexpr bool kShipSinkDescentBound = true;
    // SUBSTITUTIONS, labelled: GameSettings is not loaded into this host, so
    // +400h MaxLeakPercent and +404h EnnyiVizEsKeszPercent are this
    // installation's scripts/datatables/shipglobals.lua lines 383 and 381
    // (2024-07-13): 0.02 and 0.2.
    static constexpr float kMaxLeakPercentSubstitute = 0.02f;
    static constexpr float kEnnyiVizEsKeszPercentSubstitute = 0.2f;
    // Packet cc9_buoyancy_elements, docs/CONSTRUCT_WORLD.md section 28. ON: the
    // hull's element list at class+52Ch is the image's own, built by 0082D040
    // (called at 0082FEE3 from the ship class's model-binding virtual 0082FE30)
    // from the class Mesh's "deckline" / "bottomline" Aux point sets, Length,
    // Width, Mass, Hull.WaterLineRatio (+71Ch) and Hull.Segments (+720h):
    // 2 x Segments records. Everything that walks the list follows it: the
    // hydrodynamics 009329C0 on every live hull and the reserve buoyancy
    // controller+84h (00937DBB..00937F74) the leak manager's capacity derives
    // from. OFF: the eight-element stand-in (still reachable, and still the
    // fallback ON for a class whose model lacks either node or whose Segments
    // is not positive; those are counted as `fallbacks`).
    // ON since the USN02 / E2 9200/9000 pairs: every hull on both missions
    // takes the image list (no fallback); capacity = Mass r / (1 - r) exactly;
    // USN02 wrecks reach -200 m 106..117 s after +5Dh (91..98 s on the
    // stand-in); E2 gameplay moves through live flotation (51 vs 52 deaths).
    static constexpr bool kShipBuoyancyElementsBound = true;
    // Packet cc9_live_hull_leak, docs/UNIT_MESSAGE_ARMS.md ("90h, bound"). ON:
    // add_leak_0074f440 runs 0074F090 on the leak manager for a hit's 90h
    // message (008221A7: amount = (float)(uint32)n * 10.0). OFF: it counts
    // the call and does nothing. The sender is the gunnery host's R10 / R11c
    // (0082755F, and the inline build at 00827663..), a follow-up there.
    // ON since the USN02 / USN04 pairs (27cc522b2): identical, because the
    // Repair substitution (0 for every class) refuses every live hit.
    static constexpr bool kLiveHullLeakBound = true;
    // SUBSTITUTION, labelled: GameSettings +40Ch VizbeomlesDolgok.DologSzorzo
    // is this installation's scripts/datatables/shipglobals.lua line 388
    // (2024-07-13): 2. The loader default is 4.0 (0083EEBE).
    static constexpr float kDologSzorzoSubstitute = 2.0f;
    // Packet cc9_live_hull_repair, docs/UNIT_MESSAGE_ARMS.md ("90h: the Repair
    // byte and the health"). ON: 0074F090's live gate reads the class Repair
    // byte and 0074F930's live cap reads the unit's health fraction, both as
    // the gunnery host hands them over. OFF: Repair 0 and health 1.0, the
    // labelled substitutions of packets cc9_live_hull_leak and
    // cc9_ship_sink_descent.
    // ON since the USN02 / USN04 pairs (f27fb079a): live hulls flood to their
    // cap; one-hit wrecks sink slower (0074EC50 weights by water).
    static constexpr bool kLiveHullRepairBound = true;
    unsigned long long hull_leak_calls = 0;
    unsigned long long hull_leak_no_model = 0;
    unsigned long long hull_leak_gated_live = 0;
    unsigned long long hull_leak_applied = 0;
    unsigned long long hull_leak_applied_wreck = 0;
    double hull_leak_rate_added = 0.0;

    // Packet cc9_scene_home_base_key, docs/CONSTRUCT_WORLD.md section 29. ON:
    // squadron pass C (007F4BA0) takes a scene row's `HomeBase` (00CF8820, read
    // at 007F4C43): a non-empty name resolves through 00925A90 (the host: the
    // air-ops deck and the unit of that name), 007F1C00 at 007F4CFA stores it
    // at +404h and pushes the squadron onto that deck's campaign queue
    // (006CC7B0 at 007F1C69), and the initial command's moveto arm then targets
    // it. OFF: only an air-ops launch has a home base (the deck search).
    // ON since the USN04 4700/4500 and USN02 9200/9000 pairs: identical (no
    // reference row authors a HomeBase and the contract is not wired yet).
    static constexpr bool kSceneHomeBaseBound = true;
    // Packet cc9_ijn08_home_base_measure (docs/CONSTRUCT_WORLD.md section 32). ON:
    // the scene HomeBase name resolves the way 00925A90's matcher 009251F0 does:
    // case-insensitively (0092521E, 00438E10), and a "Parent\Child" path by its
    // child (0092523A _strchr on 5Ch, then the children). SUBSTITUTION, labelled:
    // the host keeps no scene tree, so the parent segments are not checked and
    // the last segment is matched. OFF: an exact, case-sensitive whole-name match.
    // ON since the pairs (docs/CONSTRUCT_WORLD.md 32, "Pairs and verdict").
    static constexpr bool kSceneHomeBaseQualifiedNameBound = true;
    unsigned long long scene_home_keys_set = 0;
    unsigned long long scene_home_resolved = 0;
    unsigned long long scene_home_unresolved = 0;
    unsigned long long scene_home_not_airbase = 0;
    unsigned long long scene_home_queue_pushes = 0;
    std::map<int, ClassBuoyancyList> class_buoyancy_lists;
    unsigned long long buoyancy_lists_image = 0;
    // Packet cc9_hull_inertia (docs/GUNNERY_OPEN_ITEMS.md 47.3 step 3, 49.7, 49.10):
    // the hull body's collision box, the union 00C55FC0 of the root-listed
    // ConvexObjects' boxes (00C57C40, widened by 0.02), which 00939A89 reads back
    // into the inertia block 00939A80..00939C10. read_mmod_hull_convex_box over
    // the class Mesh model, cached per type_id. shape_count 0 leaves the zero
    // box, which is the native result (49.10). The periscope shape
    // (009396BA..009399BF) is added behind bsp::kHullPeriscopeShapeBound, which
    // is ON (GUNNERY_OPEN_ITEMS 72.1). OFF: the box stays zero, so the hull
    // body has zero inertia and never rotates under a torque.
    static constexpr bool kHullInertiaFromShapesBound = true;  // ON: capsize re-pair held (GUNNERY_OPEN_ITEMS 56.5)
    struct ClassHullBox {
        bool ok{false};
        std::string reason;
        bsp::MmodHullConvexBox box{};
    };
    std::map<int, ClassHullBox> class_hull_boxes;
    const ClassHullBox& class_hull_box(const GameUnitSlot& s) {
        const int type_id = s.row.type_id;
        auto found = class_hull_boxes.find(type_id);
        if (found != class_hull_boxes.end()) return found->second;
        ClassHullBox& entry = class_hull_boxes[type_id];
        const std::string mesh = lua.read_vehicle_class_string(type_id, "Mesh");
        std::vector<std::uint8_t> bytes;
        std::string error;
        if (mesh.empty()) {
            entry.reason = "no Mesh string";
        } else if (!lua.read_resource_file(mesh, bytes)) {
            entry.reason = "model did not open";
        } else if (!bsp::read_mmod_hull_convex_box(bytes, entry.box, error)) {
            entry.reason = "hull box read failed: " + error;
        } else {
            entry.ok = true;
            entry.reason = entry.box.shape_count > 0 ? "root and Note-owner ConvexObjects"
                                                     : "no root or Note-owner ConvexObject";
            // Packet cc9_hull_periscope_shape (GUNNERY_OPEN_ITEMS 68): 009396BA..009399BF
            // adds the `periszkop` node's ConvexObject when the class is not a kamikaze
            // (+510h KamikazeDamage, +514h KamikazeBlastDamage both <= 0).
            if constexpr (bsp::kHullPeriscopeShapeBound) {
                bsp::mmod_hull_convex_box_add_periscope(entry.box,
                    lua.read_vehicle_class_number(type_id, "KamikazeDamage", 0.0f),
                    lua.read_vehicle_class_number(type_id, "KamikazeBlastDamage", 0.0f));
            }
            if (entry.box.periscope_node) {
                log.notef("hull periscope %s (type %d): shape=%d merged=%d points=%u "
                    "min=(%.3f %.3f %.3f) max=(%.3f %.3f %.3f) kamikaze=%.1f/%.1f "
                    "(cc9_hull_periscope_shape, 009396BA..009399BF)",
                    s.row.name.c_str(), type_id, entry.box.periscope_shape ? 1 : 0,
                    entry.box.periscope_merged ? 1 : 0,
                    static_cast<unsigned>(entry.box.periscope_points),
                    static_cast<double>(entry.box.periscope_min.x),
                    static_cast<double>(entry.box.periscope_min.y),
                    static_cast<double>(entry.box.periscope_min.z),
                    static_cast<double>(entry.box.periscope_max.x),
                    static_cast<double>(entry.box.periscope_max.y),
                    static_cast<double>(entry.box.periscope_max.z),
                    static_cast<double>(lua.read_vehicle_class_number(type_id, "KamikazeDamage", 0.0f)),
                    static_cast<double>(lua.read_vehicle_class_number(type_id, "KamikazeBlastDamage", 0.0f)));
            }
        }
        log.notef("hull shapes %s (type %d, %s): %s shapes=%u points=%u extent=(%.2f %.2f %.2f) "
            "min=(%.3f %.3f %.3f) (cc9_hull_inertia, 00938F61..0093918C)",
            s.row.name.c_str(), type_id, mesh.c_str(), entry.reason.c_str(),
            static_cast<unsigned>(entry.box.shape_count), static_cast<unsigned>(entry.box.point_count),
            static_cast<double>(entry.box.max.x - entry.box.min.x),
            static_cast<double>(entry.box.max.y - entry.box.min.y),
            static_cast<double>(entry.box.max.z - entry.box.min.z),
            static_cast<double>(entry.box.min.x), static_cast<double>(entry.box.min.y),
            static_cast<double>(entry.box.min.z));
        return entry;
    }
    // Packet cc9_hull_fort_contact (docs/GUNNERY_OPEN_ITEMS.md 96): a fort's shapes are every
    // ConvexObject of its class Mesh model, whatever its node (007482B0 walks instance+4Ch
    // with no filter), cached per type_id; its static body takes the world matrix of the
    // first step the fort is placed (007482B0 runs once, at creation).
    std::map<int, ClassHullBox> class_fort_boxes;
    const ClassHullBox& class_fort_box(const GameUnitSlot& s) {
        const int type_id = s.row.type_id;
        auto found = class_fort_boxes.find(type_id);
        if (found != class_fort_boxes.end()) return found->second;
        ClassHullBox& entry = class_fort_boxes[type_id];
        const std::string mesh = lua.read_vehicle_class_string(type_id, "Mesh");
        std::vector<std::uint8_t> bytes;
        std::string error;
        if (mesh.empty()) {
            entry.reason = "no Mesh string";
        } else if (!lua.read_resource_file(mesh, bytes)) {
            entry.reason = "model did not open";
        } else if (!bsp::read_mmod_hull_convex_box(bytes, entry.box, error)) {
            entry.reason = "model read failed: " + error;
        } else {
            entry.ok = true;
            entry.reason = "every ConvexObject";
        }
        std::size_t points = 0;
        for (const std::vector<bsp::OceanVec3>& shape : entry.box.all_shape_points) {
            points += shape.size();
        }
        log.notef("fort shapes %s (type %d, %s): %s shapes=%zu points=%zu "
            "(007482B0, packet cc9_hull_fort_contact)", s.row.name.c_str(), type_id,
            mesh.c_str(), entry.reason.c_str(), entry.box.all_shape_points.size(), points);
        return entry;
    }
    std::map<std::size_t, bsp::FortWorldEntry> fort_bodies;
    unsigned long long buoyancy_lists_stand_in = 0;
    unsigned long long buoyancy_lists_fallbacks = 0;
    unsigned long long leak_models_built = 0;
    unsigned long long leak_redistributions = 0;
    unsigned long long kill_depth_tests = 0;
    std::size_t kill_depth_kills = 0;
    // Packet cc9_sunk_hull_shape_flag8: hulls whose terrain mask bit was cleared, and the
    // hull-terrain contact steps such a wreck still had (only possible with the switch OFF).
    std::size_t sunk_mask_cleared_units = 0;
    unsigned long long sunk_wreck_terrain_steps = 0;
    // Packet cc9_torpedo_boat_kind: motion-tick queries of IsKindOf(0Eh) that a
    // TorpedoBoat answered true (counted both ways), and the sink steps a torpedo
    // boat spent between 20 s and 60 s (the arm OFF never takes).
    unsigned long long torpedo_boat_trait_queries = 0;
    unsigned long long torpedo_boat_sink_20s_steps = 0;
    // The arms the probe opens: boost steps (and those with an order kind), and
    // the righting steps with the summed |row-0 rate change|.
    unsigned long long torpedo_boat_boost_steps = 0;
    unsigned long long torpedo_boat_boost_ordered_steps = 0;
    unsigned long long torpedo_boat_righting_steps = 0;
    double torpedo_boat_righting_abs = 0.0;
    void ship_terrain_contact(GameUnitSlot& slot, const float before[3]);
    unsigned long long terrain_contact_steps = 0;
    unsigned long long terrain_contact_stops = 0;
    std::size_t terrain_contact_logged = 0;
    // Packet cc9_hull_terrain_contact_solver (docs/GUNNERY_OPEN_ITEMS.md 84): the Dyn contact
    // phase for hull-terrain pairs, and its per-unit census {contact steps, max depth, max
    // upward velocity change, max pseudo-velocity}.
    bsp::HullTerrainContactSolver hull_terrain;
    // Packet cc9_avoid_zone_draft_bodies: the draft-body build and the 0092BD70 calls.
    bsp::AvoidZoneDraftBuildCensus draft_build{};
    unsigned long long hull_class_bit_clears{0}, hull_class_bit_sets{0};
    struct HullTerrainUnit {
        unsigned long long steps{0};
        float max_depth{0.0f};
        float max_up_dv{0.0f};
        float max_horizontal_dv{0.0f};
        float first_t{-1.0f};
    };
    std::map<std::size_t, HullTerrainUnit> hull_terrain_units;
    // Packet cc9_hull_hull_contact (docs/GUNNERY_OPEN_ITEMS.md 91): the ticks waiting for the
    // world phase (kDynWorldContactPhaseBound), and the hulls of this step for the OFF
    // census of hull pairs.
    struct WorldContactPending {
        std::size_t index{0};
        float before[3]{};
        bsp::ShipMotionStepResult result{};
    };
    std::vector<WorldContactPending> world_contact_pending;
    std::vector<bsp::HullWorldEntry> hull_census;
    void record_hull_terrain_result(GameUnitSlot& slot, std::size_t index,
        const bsp::HullTerrainContactStepResult& r);
    void finish_motion_tick(GameUnitSlot& slot, const float before[3],
        const bsp::ShipMotionStepResult& result, float step_seconds);
    void run_world_contact_phase(float step_seconds);
    unsigned long long world_nodes_unlinked = 0;
    // Packet cc9_ground_height_hunks: the torpedo approach's 009D39D3 probe.
    unsigned long long segment_probes = 0;
    unsigned long long segment_probes_blocked = 0;
    std::vector<unsigned long long> slot_swap_runs_by_group;
    std::vector<unsigned long long> slot_swaps_by_group;
    std::vector<unsigned char> slot_swap_gated_by_group;
    unsigned long long slot_swap_pairs = 0, slot_swap_pairs_invalid = 0;
    bool slot_swap_membership_logged = false;
    unsigned long long h9b8_writes = 0;
    double h9b8_min = 0.0, h9b8_max = 0.0;
    std::vector<unsigned char> squadron_list24_pushed;
    int squadron_list24_pushes = 0;
    std::map<int, int> part_count_cache;
    std::size_t part_count_primed = 0;
    // Diagnostic census of both heights at every sample, in both builds.
    unsigned long long az_samples = 0;
    unsigned long long az_samples_layer_nonzero = 0;
    double az_layer_max = 0.0;
    double az_sea_min = 0.0, az_sea_max = 0.0;
    double az_diff_abs_max = 0.0;
    double az_diff_abs_sum = 0.0;
    // Packet cc9_plane_death_modes: 007CA8A0's death mode, 007CAF10's dead-step
    // terms, the kill that takes the aircraft out of the world, and the release
    // refusal of a dead aircraft (007CEA1C). docs/PLANE_DEATH_MODES.md.
    static constexpr bool kPlaneDeathModesBound = true;
    // Packet cc9_plane_death_flags: the destroy that 007D0B80's tail calls
    // (007D12FC, vtable[70h](1) = 0077D1A0 -> 00926C80) sets unit+60h at
    // 00926CD5, and the destroy-list flush's 00926390 sets unit+5Dh and +60h at
    // 0092639B/0092639E. Every liveness gate (00A2DDE0's eviction, 0043F080)
    // then reads the aircraft as gone. OFF: a dead aircraft keeps both bytes
    // clear. docs/PLANE_DEATH_MODES.md section 6.
    static constexpr bool kPlaneDeathFlagsBound = true;
    // Packet cc9_dead_plane_bot_think (docs/PLANE_DEATH_MODES.md section 7).
    // BSP_PilotBot_Tick 0099ACD0 returns before its think accumulator when the
    // unit's +5Dh (0099ACE1), +60h (0099ACEB) or +61h (0099ACF5) byte is set,
    // or its squadron's +61h (0099AD05, through unit+9D4h). A dead aircraft
    // carries +60h from the destroy (00926CD5) and +5Dh from the flush
    // (0092639B), so from its death on no task arm, planner or 007B8C90 command
    // runs, and 007BB920 finds no pending block: the live controls hold what
    // was last committed. OFF: a dead aircraft keeps thinking and steering.
    static constexpr bool kDeadPlaneBotThinkBound = true;   // ON by its pairs (PLANE_DEATH_MODES 7.5)
    // Packet cc9_pilot_surface_climbout: a dead plane leaves its squadron
    // (007BCAA0 -> 007F3970), so no member is placed on a dead leader's station.
    // docs/PILOT_SURFACE_CLIMBOUT.md.
    static constexpr bool kSquadronRemovesDeadBound = true;
    // 007DA710 (free-flight arm, read) and 007BB920's air-brake override.
    static constexpr bool kPlaneControlRateLawBound = true;
    static constexpr bool kPlaneCommitCommandBound = true;
    // Packet cc9_dogfight_moveto: the dogfight moveto's speed slot 009C1BC0
    // (009BECD0 / 007EF2C0 / 009BE3E0), and PilotFires (unit+C24h, 007CD930)
    // loaded from the authored Platforms[].PilotFires. docs/DOGFIGHT_MOVETO.md.
    static constexpr bool kDogfightMovetoSpeedBound = true;
    // Packet cc9_dive_modes: the dogfight moveto runs the generic 009C18C0
    // glide with its fixed ranges (500, 100, 1000) instead of the stand-in.
    // OFF, measured (docs/DIVE_MODES.md 5): with the throttle fix the leader holds
    // 1475 m / 82.2 m/s (was 1547 m / 34 m/s), but on main alone (local/G_9000.log)
    // it moves 46 dive-bomb rows through the fighters, not yet explained.
    static constexpr bool kDogfightMovetoGenericBound = true;
    static constexpr bool kPilotFiresBound = true;
    // Packet cc9_throttle_slot: a per-think trace of the throttle slot for the
    // Yorktown flight (dogfight) and D3A Val #1.1 (dive-bomb). Diagnostic, off.
    static constexpr bool kThrottleSlotTrace = false;
    // DIAGNOSTIC (p21_stow): 009B22C0's carrier park inputs within 8 m of the target.
    static constexpr bool kParkStowTrace = false;
    // Packet cc9_throttle_slot: 0099D300's speed-hold multiplier is dt in speed
    // mode (not the slot's pending distance). docs/PILOT_THROTTLE_SLOT.md.
    // OFF, measured: USN04 bomb drops 29 -> 0 (local/S0_9000.log vs
    // local/S1_9000.log). The speed hold now works in flyabove and turndown and
    // cuts the throttle to 0.001; aimdive (kAimDiveTailBound off) and goaway
    // (009C4C0C-009C4CA7 unmodelled) write no throttle in this host, so the Val
    // dives unpowered at 53 m/s instead of 134. Binding the aimdive tail too
    // (local/S1A_9000.log) still drops nothing. Bind those writers first.
    static constexpr bool kPilotThrottleSlotBound = true;
    // Packet p16_carrier_landing (docs/OCEAN_WAVE_FIELD.md section 64; cc13 verify12
    // R2). True: 009AFAF0's on-ground arm leads the path point. 009AFC71-009AFD1A:
    // e = |00605070(A4 - unit+C6Ch)|, A8 += min(class+268h x 0.5 x e, A8 x 0.18
    // (00D05AA0)), stored; 009AFD20-009AFD40 t = min(A8 / unit vtable[38h] (+B1Ch,
    // |velocity|), 10.0); 009AFD52 006BCA80(out, t) adds the owner's velocity
    // (006BCBF8-006BCC51, owner vtable[34h]) x t to T. LABELLED: holder+8Ch is 0
    // (written only at construction), so the turn by +8Ch x t is the identity.
    // False: the point is T and only 009AFDA4's floor of 1.0 is kept.
    static constexpr bool kCarrierLandingVelocityLeadBound = true;  // ON 2026-10-17 (OCEAN_WAVE_FIELD 66)
    // Packet p19_lead_ops (docs/OCEAN_WAVE_FIELD.md section 73). True: the lead's
    // arithmetic rounds per op as 006BCC0A-006BCC51 stores it: v[k] x t to a float
    // (006BCC1D/006BCC26/006BCC2D), then that float + T[k] to a float (006BCC39..).
    // Exact at the main thread's 24-bit PC; at 53 bits the image can differ by a
    // double rounding of the product. False: one double v x t + T, rounded once.
    static constexpr bool kCarrierLandingLeadFloatOpsBound = true;  // ON 2026-10-18 (OCEAN_WAVE_FIELD 73)
    // Same packet (R3). True: 009B201B-009B2074, the land-final ground half's air
    // brake is 1.0 (00D7A24C) when the land target answers vtable[5Ch](9) (a mother
    // ship), else 0.2 (00CE54A0). False: 0.2 always.
    static constexpr bool kCarrierTouchdownBrakeBound = true;  // ON 2026-10-17, unreached (OCEAN_WAVE_FIELD 66)
    // Packet p16_moveto_cruise (docs/OCEAN_WAVE_FIELD.md section 63; cc13 verify12
    // R1). True: the kind-7 moveto's cruise altitude ctl+394h takes 009C3650's
    // ordnance arms, the same as 009BD6E0's (moveonpath_cruise_profile_009bd6e0):
    // torpedo (007B93F0, +43Ch) +430h, depth charge (007B94F0, +4B4h) +4A4h, a
    // general bomb (007B9320) LevelBomb (0047B850, +454h) +444h or DiveBomb (+4D4h)
    // +4C0h, each + 0.6 x TravelAltRandom (009C36DD-009C36F2); otherwise 009C38FD's
    // Small/LargePlaneTravelAlt with no random term. LABELLED: every member computes
    // it at the 0.5 s refresh from its own ordnance (the image's leader writes the
    // squadron block once, at activation). False: Small/Large + 0.6 x random for all.
    static constexpr bool kMoveToCruiseByOrdnanceBound = true;  // ON 2026-10-17 (OCEAN_WAVE_FIELD 66)
    // Same packet (R4). True, with bsp::kLegAltitudeManagerBound: 009BEC85 runs
    // 009FD050(+64h, point) in the moveto refresh, registering the plane with the
    // converging-leg manager inside 3000 m and dropping it outside, and while the
    // weight +28h > 0, 009BECA9-009BECBF write 009FC260(unit, point, w) to the steer
    // point's height +4Ch. Slot +58h (009C32A0, 009C32B8 009FD050(-1)) drops it when
    // the task goes. LABELLED: the drop is taken on the first think after the host
    // clears moveto_task_installed. False: only moveonpath planes register.
    static constexpr bool kMoveToLegAltitudeBound = true;  // ON 2026-10-17, unexercised (OCEAN_WAVE_FIELD 66)
    // Packet p16_planner_laws (docs/OCEAN_WAVE_FIELD.md section 62; cc13 verify12
    // R5). True: 0099D300's mode-1 roll arm (0099E26E-0099E39D) runs on the image's
    // inputs: plan+2C4h clamped in place to +-plan+2C8h when that is below pi
    // (0099E27B-0099E2B5, 00415690), the band/gain/rate EBX+40h/44h/48h =
    // SoftRollCtrl, SoftRollMul and singleton+580h, and the servo limit t =
    // class+1A8h^2 / class+1BCh / WaggleLimit (0099E344-0099E367), as the mode-2
    // arm already takes them. False: band 0, gain 1, rate 0, limit 1, no clamp.
    static constexpr bool kPilotModeOneRollTuningBound = true;  // ON 2026-10-17 (OCEAN_WAVE_FIELD 62.1)
    // Same packet (verify12 R6). True: the speed-error divisor plan+2B8h has its
    // image producer: 0099D756-0099D79A decay above 1.0 by TrgSpeedCorrMulDecay x
    // dt, and 0099D924-0099D970 raise it on the demand arm while plan+2B0h is clear
    // to max(+2B8h, Interp(-TrgSpeedCorrMinPitch, TrgSpeedCorrSpeedMul, 0, 1,
    // pitch)). LABELLED: the start value is 1.0 (no store to the plan block's
    // +2B8h was found; 1.0 is the decay floor and the divisor's identity). False:
    // the divisor is 1.0.
    static constexpr bool kPilotSpeedCorrDivisorBound = true;  // ON 2026-10-17 (OCEAN_WAVE_FIELD 62.1)
    // Same packet (R6, the measured speed). True: 0099D99E's measured speed is
    // 007D99C0 (unit+AB0h), the body forward speed, taken as the world velocity on
    // the pose's forward row as control_authority takes it. LABELLED: 007D99C0's
    // carrier-deck parent term is dropped. False: |world velocity|.
    static constexpr bool kPilotThrottleForwardSpeedBound = true;  // ON 2026-10-17 (OCEAN_WAVE_FIELD 62.1)
    // Same packet (R7). True: control authority 007D9A70 is 007C0F40(unit) x t^2:
    // the first weapon slot whose vtable[210h](2Ah, 0) answers (a rack holding a
    // round) gives Interp(0, 1, 0.8, class+15Ch BombControlLimit, its remaining
    // fraction vtable[214h]). LABELLED: the holding test is rack_holds_round; a
    // single rack's fraction (006E4130) is the host's rounds left over the
    // authored Ammo (orgAmmo +488h); a multi rack (006E3700) answers 1.0 while it
    // holds, its +514h entries not being modelled; the turbo arm (unit+BC8h,
    // class+608h) is never taken by an AI plane here. False: the factor is 1.0.
    static constexpr bool kPlaneBombLoadAuthorityBound = true;  // ON 2026-10-17 (OCEAN_WAVE_FIELD 62.1)
    // 007B4ED0 wiring (the head-on arm and the maneuver tail's full throttle).
    // OFF, measured: run F1 drowned Yorktown-class01_sqn02 and its .-2 at |v| 51
    // after one maneuver tick left the throttle slot active in mode 0. The image
    // re-seeds the command block every think (0099B4E8); this host does not, so
    // the write outlives the state. docs/DOGFIGHT_GUN.md section 6.
    // Packet cc9_throttle_slot corrects that cause: the stall was the speed-hold
    // multiplier (kPilotThrottleSlotBound). With both on (local/S1T_9000.log) the
    // pair no longer stalls but still reaches the water at |v| 100-106, in aim,
    // chasing Vals that the same switch sends gliding into the sea. Still OFF.
    // ON since packet cc9_flight_integrator: pair FI0/FI1 with the integrator
    // bound (docs/FLIGHT_INTEGRATOR.md section 4); the low-speed stall is the
    // image's own regime (docs/FREEFLIGHT_STALL_LAW.md).
    static constexpr bool kDogfightThrottleBound = true;
    // Packet cc9_aimglide_pitch: approach+A8h is Uniform(row+38h, row+3Ch) drawn
    // by 00BD2F10 on stream ECX = 1 in the approach constructor 009C3EA0
    // (009C3F09-009C3F2E), once per dive-bomb task. This host pinned it to the
    // low end. SUBSTITUTION, labelled: the image's stream is the one process-wide
    // generator the gunnery host models, but that generator is private to
    // src/game_hosts_gunnery.cpp, so this draws from a copy of its algorithm and
    // seed (one sequence across these draws). With BSP_GUNNERY_RNG_STREAMS=1 the
    // draw is keyed per unit name instead, as the gunnery option keys its draws.
    // OFF with kAimGlidePitchBound (docs/AIMGLIDE_PITCH.md section 5).
    static constexpr bool kReleaseAltitudeDrawBound = true;
    // Packet cc9_fighter_gunfire_rate: approach+B4h = uniform(0.6, 0.8) * class+268h
    // (009C3F63-009C3F97), pinned at 0.6 before. OFF: unmeasured (runs blocked).
    static constexpr bool kAttackDistDrawBound = true;
    // Packet cc9_l57_dive_mode_fed: 009C83F8 reads squadron+370h through task+404h.
    // Its producers are the block init 007F2DC1 (MOV [ESI+370h],EDI, EDI = 1 from
    // 007F2D41) and the flight leader's 0099B740 (a store of 1); mode 2 comes only
    // from the Lua binding 008A4B10 and message BCh. True: the dive arm reads
    // slot.db_attack_mode_370 (the leader's value, copied to the flight) instead
    // of the pinned 2, so a member's `engaged` is its in-range latch.
    static constexpr bool kDiveAttackModeFedBound = true;  // ON: SQUADRON_LAND_TASK 5gx.1
    // Packet cc9_l57_dive_items (docs/CC13_VER_9.md finding 3): +B8h = +BCh =
    // uniform(1.6, 1.8) * class+268h (009C3FB0 CALL 00BD2F10, stream 1 per cc13-ver9, after the
    // +B4h draw; 009C3FE8 FST / 009C3FF5 FSTP). False: pinned at 1.6.
    static constexpr bool kInRangeDrawBound = true;  // ON: SQUADRON_LAND_TASK 5gx.1
    // Packet cc9_l57_dive_items (docs/CC13_VER_6.md finding 4): 009C6EB1/009C6F17
    // divide by frame [ESP+28h] = R, the 3-second lead range 009C63A6 writes (the
    // only writer in 009C62B0-009C6F97), not |aim - pos|. False: db_planar_bc.
    static constexpr bool kFlyAboveLeadDivisorBound = true;  // ON: SQUADRON_LAND_TASK 5gx.1
    // Packet cc13-draws1 (docs/CC13_UNLABELLED_DRAWS_BATCH.md). Each is OFF until
    // its pair runs. The draws use release_altitude_draw_00bd2f10 with a per-site
    // key, never stream 0; the image draws all four on stream 1 (ECX = 1).
    // 009C6CAE-009C6D3B: the fly-over latches its roll side +20h (task+798h).
    static constexpr bool kDiveTurnSideLatchBound = true;  // GUNNERY 193: ON by pair verdict
    // 009C8531/009C855B: the bank thresholds +-20 deg (00CE398C, 00D1FBC0).
    static constexpr bool kDiveTurnSideBankBound = true;  // GUNNERY 193: ON by pair verdict
    // 009C8587-009C85B6: the coin trunc(U(0, 1024)) >= 512 -> +1, else -1.
    static constexpr bool kDiveTurnSideCoinBound = true;  // GUNNERY 193: ON by pair verdict
    // 009C7839: the turndown roll magnitude U(0.8, 1.0), pinned at 0.8 before.
    static constexpr bool kDiveTurnRollDrawBound = true;  // GUNNERY 193: ON by pair verdict
    // 009C579E: the aimglide re-arm U(0.2, 0.5), pinned at 0.2 before.
    static constexpr bool kAimGlideRearmDrawBound = true;  // GUNNERY 193: ON by pair verdict
    // 009C6114: the aimdive re-arm U(0.5, 1.0), pinned at 0.5 before.
    static constexpr bool kAimDiveRearmDrawBound = true;  // GUNNERY 193: ON by pair verdict
    // Packet cc9_fighter_gun_lead (docs/FIGHTER_GUN_LEAD.md): the gun's lead
    // point +5Ch from 009FC931-009FC9F6 (two passes of the target's vtable[48h]
    // 00954650, both arms, t = d / 007C2610), the envelope against it, and the
    // fine aim 009F9FC0 (009FCCA0-009FCDB2) writing the yaw and pitch slots.
    // Before: the target's current position, unled. ON, measured (E2 9000 L0/L1b):
    // fighter hits 16 -> 66, two Vals shot down before their dive, releases 5 -> 2.
    static constexpr bool kFighterGunLeadBound = true;
    // Packet cc9_l58_df_aim_lead (docs/CC13_VER_22.md D1). The dogfight aim tick
    // 009AAC70 takes d (+D4h) from the aim point +48h it built on the previous
    // tick (009AAD34 vt[0]), latches, stores the time to target
    // clamp(d / 007C2610(), 0, 30) at approach+70h (0 unlatched, 009AAE22) and
    // rebuilds +48h through 009FADA0 (009AAE35), whose tail 009FAF05 adds the
    // target's vt[48h](projtime) - origin (00954650 for a plane). The local frame
    // +ECh is then taken from the new point (009AAE70-009AAEA7). True: d is taken
    // from 00954650(target, last tick's projtime) (LABELLED: the image's point
    // is last tick's prediction from last tick's target state), the latch gets
    // 007C2610, and df_aim = 00954650(target, this tick's projtime). False: the
    // target's origin and a muzzle speed of 1.0 (the time to target discarded).
    static constexpr bool kDogfightAimLeadBound = true;  // ON: SQUADRON_LAND_TASK 5gz.1
    // Packet cc9_l58_df_adopt_gun (docs/CC13_VER_22.md D2). 009A797F-009A79A9, the
    // tail of the aim tick 009A76E0: the gun's auto target gun+74h (stored
    // unconditionally at 009FC917 from the 007B96F0 search, null included) is
    // handed to 009A7650 when it is not null, not approach+B4h, has a squadron
    // (+9D4h) and that squadron is approach+CCh. True: the host's finder result
    // (df_finder_scan_007e2090; 007B96F0 only forwards to 007E2090 on
    // unit+C50h, and the gun's arguments match, SQUADRON_LAND_TASK 5he) as last
    // left by the gun tick, against the order's squadron (the approach+CCh
    // stand-in of df_approach_update_009aac70); 009A7650's effects as
    // df_select_009aa630 runs them (target, opportunistic rocket target).
    // False: the aim state keeps its pick until the reselect.
    static constexpr bool kDogfightAimAdoptsGunTargetBound = true;  // ON: SQUADRON_LAND_TASK 5he.2
    // Packet cc9_fighter_aim_wiring (docs/GUN_AIM_TERMS.md): the fine aim's
    // distortion 009FA7E0 and 009F9FC0's gun+4Ah damped cap, under the lead.
    static constexpr bool kFighterAimTermsBound = true;
    std::uint32_t db_release_rng{0x9E3779B9u};
    std::map<std::uint64_t, std::uint32_t> db_release_rng_by_key;
    static bool release_rng_streams_enabled() {
        static const bool on = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            bool value = false;
            if (_dupenv_s(&text, &bytes, "BSP_GUNNERY_RNG_STREAMS") == 0 && text != nullptr)
                value = text[0] == '1';
            std::free(text);
            return value;
        }();
        return on;
    }
    float release_altitude_draw_00bd2f10(const std::string& unit_name, float low, float high) {
        std::uint32_t* state = &db_release_rng;
        if (release_rng_streams_enabled()) {
            std::uint64_t key = 0xcbf29ce484222325ull;   // FNV-1a of the name
            for (const char ch : unit_name) {
                key = (key ^ static_cast<unsigned char>(ch)) * 0x100000001b3ull;
            }
            auto it = db_release_rng_by_key.find(key);
            if (it == db_release_rng_by_key.end()) {
                std::uint64_t z = key + 0x9E3779B97F4A7C15ull;
                z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
                z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
                z ^= z >> 31;
                it = db_release_rng_by_key.emplace(key, static_cast<std::uint32_t>(z)).first;
            }
            state = &it->second;
        }
        *state = *state * 1664525u + 1013904223u;
        const float unit = static_cast<float>((*state >> 8) & 0xFFFFFFu)
            / static_cast<float>(0x1000000u);
        return low + (high - low) * unit;
    }
    // Diagnostic period for the `follow trace` row below; 0 compiles it out.
    // Packet cc9_follow_speed ran it at 25 (docs/PLANE_FOLLOW_SPEED.md section 5).
    static constexpr int kFollowTraceEvery = 0;
    // 007B8AD0, four instructions at 007b8ad0-007b8adb, bytes
    // `33 c0 39 81 d8 09 00 00 0f 94 c0 c3`:
    //   XOR EAX,EAX / CMP [ECX+9D8h],EAX / SETE AL / RET
    // `__fastcall(ECX = unit) -> bool`, i.e. `return unit->+9D8h == 0`.
    //
    // CORRECTED, packet cc8_follow_enter. The ledger name
    // `BSP_Unit_LacksFollowTarget` and docs/BOT_TASK_STATES.md read `+9D8h` as
    // a follow-target pointer, with `contract: unread` on the writer. It is not
    // a pointer. Every writer stores an INDEX - a census of both store forms
    // (`89 ?? d8 09 00 00` -> seven sites, `c7 ?? d8 09 00 00` -> two, and
    // those two are in FUN_00A414D0/FUN_00A415B0, far outside unit code):
    //   007CFE72  BSP_PlaneUnitInstance_Construct stamps -1; 007CFD69 is
    //             `OR EDI,0FFFFFFFFh` and EDI is written nowhere between that
    //             and the store (whole-listing filter, and EDI is non-volatile
    //             across the intervening calls)
    //   007F4B43  the spawn attach stamps squadron+3CCh, the count before the
    //             append; 007CDF7C and 007ED220 are two further append helpers
    //             carrying the same +9D4h=squadron / +9D8h=index pair
    //   007ED292  BSP_PlaneSquadron_AssignFormationIndices rewrites it to the
    //             live walk index, and 007F4BFA runs that in the spawn tail, so
    //             it equals the array position from the first assignment on
    // 007ED610 BSP_PlaneSquadron_PromoteFlightLeader rotates the promoted
    // member to the FRONT of +3D0h and only then re-indexes, which is what
    // makes slot 0 mean "leader". So the predicate asks "am I my squadron's
    // flight leader": the leader takes moveto (009C777E -> +4F0h,
    // 009D30BE -> +544h) and every wing member takes follow (+52Ch, +580h).
    //
    // SUBSTITUTION, labelled: a unit with no squadron record answers TRUE, the
    // leader answer, which is this host's previous hardcode. The image's own
    // no-squadron state is the constructor's -1, which is NOT slot 0 and would
    // take follow - but "this host's registry has no record" is not evidence of
    // "the image would have left -1 here". The registry holds the planes spawned
    // through PlaneSquadronGen/007F4580 only. (Correction, packet
    // cc9_val_squadron_registry: that includes every SpawnNew squadron, since
    // 0046db4b runs 007F4580's plan for class 18h, so USN04's Vals, Kates and
    // Zeros are all registered; the "one squadron of three" this said was the
    // scene-authored movieval only.) An aircraft outside it reaches a task by a
    // route this host does not model as a squadron, so answering false for it would
    // put aircraft whose membership is simply unknown into a follow state that
    // 009BFD70 then declines to produce a station for - commanded nothing, on no
    // evidence. Keeping the leader answer confines this packet's change to the
    // aircraft whose slot the host actually knows, which is what makes the
    // before/after separable.
    // Packet cc9_val_squadron_registry: 009273A0 -> 00926390 -> vtable[7Ch]
    // 007BCAA0 -> 007F3970 at a plane's death. The host has no destroy flush,
    // so the death is read where this host records it: the death mode chosen
    // at 007CA8A0 (plane_death_c3a), a removal, or the gunnery host's dead
    // flag (health <= 0 or kill_unit, the image's vtable[70h] trigger). The
    // image leaves on the flush after the lethal step; this sweep runs at the
    // head of the next motion step, which is the same frame boundary.
    int squadron_leaves_{0};
    int squadron_promotions_{0};
    // Packet cc13-legmgr1 (docs/CC13_LEG_ALTITUDE_MANAGER.md), kLegAltitudeManagerBound:
    // the process-wide converging-leg manager 009BD400 (one per host here) and its
    // two unit reads. A record's unit is slot index + 1. unit+5Dh stands as the
    // host's death predicate (plane_death_c3a / removed / gunnery dead), a
    // LABELLED substitution; [unit+538h]+268h is plane_turn_circle_radius.
    struct LegUnits final : bsp::LegAltitudeUnitHost {
        explicit LegUnits(const Impl& h) : h_(h) {}
        const GameUnitSlot* slot(bsp::LegUnitId id) const {
            return id != 0 && id - 1 < h_.slots.size() ? h_.slots[id - 1].get() : nullptr;
        }
        bool out_of_action_5d(bsp::LegUnitId id) const override {
            const GameUnitSlot* s = slot(id);
            return s == nullptr || s->plane_death_c3a || s->plane_death_removed ||
                (h_.gunnery != nullptr && h_.gunnery->unit_dead(s->process_index));
        }
        float turn_circle_radius_268(bsp::LegUnitId id) const override {
            const GameUnitSlot* s = slot(id);
            return s != nullptr ? s->plane_turn_circle_radius : 0.0f;
        }
        const Impl& h_;
    };
    // 009BD400 supplies the SAME raw10h owner to all live aircraft/goal
    // consumers. The shared singleton drain runs 009FAA90 and clears F8A720;
    // this mission-side view owns no second manager or record array.
    bsp::LegAltitudeManager leg_manager_cc13_{bsp::LegAltitudeSingletonDomain{
        game_native_string_process().manager_01090aa0(),
        game_native_string_process().path_leg_manager_00f8a720()}};
    LegUnits leg_units_cc13_{*this};
    void squadron_leave_on_death_007bcaa0() {
        for (const auto& owned : slots) {
            GameUnitSlot& s = *owned;
            if (s.squadron_left_on_death) continue;
            const bool dead = s.plane_death_c3a || s.plane_death_removed ||
                (gunnery != nullptr && gunnery->unit_dead(s.process_index));
            if (!dead) continue;
            bsp::PlaneSquadronHostRecord* rec =
                bsp::plane_squadron_registry().find_by_member_unit(s.process_index);
            if (rec == nullptr) {
                // kSquadronRemovesDeadBound cleared the unit; the name remains.
                rec = bsp::plane_squadron_registry().find_by_member_name(s.row.name);
            }
            if (rec == nullptr) continue;
            const std::size_t old_leader = rec->flight_leader();
            if (!rec->remove_member_unit_007f3970(s.process_index, s.row.name)) continue;
            s.squadron_left_on_death = true;
            ++squadron_leaves_;
            const std::size_t leader = rec->flight_leader();
            if (old_leader == s.process_index) ++squadron_promotions_;
            log.notef("plane squadron leave: unit=%s squadron=%s t=%.2f live=%d leader %s -> %s "
                "(007BCAA0 -> 007F3970, packet cc9_val_squadron_registry)",
                s.row.name.c_str(), rec->name.c_str(),
                static_cast<double>(summary.simulated_seconds), rec->live_count(),
                old_leader < slots.size() ? slots[old_leader]->row.name.c_str() : "-",
                leader < slots.size() ? slots[leader]->row.name.c_str() : "-");
            if (rec->live_count() == 0) squadron_kill_queue_erase_006c4b30(*rec);
        }
    }

    // 007F3A16 -> 00926D90 (Kill) -> the flush's slot+4 notification -> 006C4B30 on
    // each observing queue (block+84h): every record of the squadron goes, the rest
    // keep their order (006C06A0 is a remove, not 006BF7F0's swap with the last).
    void squadron_kill_queue_erase_006c4b30(const bsp::PlaneSquadronHostRecord& rec) {
        ++squadron_kill_queue_kills;
        const std::size_t sqi = static_cast<std::size_t>(
            &rec - bsp::plane_squadron_registry().records().data());
        for (std::size_t di = 0; di < landing_decks.size(); ++di) {
            std::vector<LandingQueueEntry>& q = landing_decks[di].queue;
            std::size_t held = 0;
            for (const LandingQueueEntry& e : q) held += e.squadron == sqi ? 1u : 0u;
            if (held == 0) continue;
            log.notef("  squadron kill: %s's %zu landing-queue record(s) at deck %zu %s "
                "(%zu queued; 00926D90 -> 009273A0 slot+4 -> 006C4B30, packet "
                "cc9_squadron_kill_queue_erase)", rec.name.c_str(), held, di,
                kSquadronKillQueueEraseBound ? "erased" : "kept, unbound", q.size());
            if constexpr (kSquadronKillQueueEraseBound) {
                std::size_t w = 0;
                for (std::size_t r = 0; r < q.size(); ++r) {
                    if (q[r].squadron != sqi) q[w++] = q[r];
                }
                q.resize(w);
                squadron_kill_queue_erases += held;
            }
        }
    }

    // 009B3F50's tail at a live retire: 007B8AD0, then 006C8800 -> 006C7680 per block.
    void land_task_retire_006c8800(const GameUnitSlot& unit, const char* site) {
        if (!unit_is_flight_leader_007b8ad0(unit.process_index)) return;   // 009B3F9E
        bsp::PlaneSquadronHostRecord* rec =
            bsp::plane_squadron_registry().find_by_member_unit(unit.process_index);
        if (rec == nullptr) return;                                          // 006C8803
        ++land_retire_leader_calls;
        const std::size_t sqi = static_cast<std::size_t>(
            rec - bsp::plane_squadron_registry().records().data());
        bsp::AirOpsDeckRegistry& air_decks = bsp::air_ops_decks();
        for (std::size_t di = 0; di < landing_decks.size(); ++di) {
            LandingDeck& d = landing_decks[di];
            std::size_t k = 0;
            while (k < d.queue.size() && d.queue[k].squadron != sqi) ++k;   // 006C76B6
            if (k == d.queue.size()) continue;
            std::size_t assignments = 0;
            for (const LandingAssignment& a : d.assignments) {
                if (bsp::plane_squadron_registry().find_by_member_unit(a.plane) == rec) ++assignments;
            }
            log.notef("  land task retire: %s (%s) leads %s; deck %zu queue record %s, %zu +A8h "
                "record(s) (009B3FB3 -> 006C8800 -> 006C7680, packet "
                "cc9_land_task_retire_queue_erase)", unit.row.name.c_str(), site,
                rec->name.c_str(), di, kLandTaskRetireQueueEraseBound ? "erased" : "kept, unbound",
                assignments);
            if constexpr (kLandTaskRetireQueueEraseBound) {
                d.queue[k] = d.queue.back();                                 // 006BF7F0
                d.queue.pop_back();
                ++land_retire_queue_erases;
                std::size_t w = 0;                                           // 006C7540
                for (std::size_t r = 0; r < d.assignments.size(); ++r) {
                    if (bsp::plane_squadron_registry().find_by_member_unit(d.assignments[r].plane)
                            == rec) continue;
                    d.assignments[w++] = d.assignments[r];
                }
                land_retire_assignment_erases += d.assignments.size() - w;
                d.assignments.resize(w);
                if (rec->squadron_unit == bsp::kPlaneSquadronNoUnit) continue;
                const std::uint32_t entity = static_cast<std::uint32_t>(rec->squadron_unit + 1u);
                if (bsp::AirOpsDeck* deck = air_decks.mutable_at(di)) {
                    for (bsp::AirOpsSlot& slot : deck->slots) {              // 006C7710..006C779D
                        if (slot.launched_squadron != entity) continue;
                        if (slot.state == bsp::AirOpsSlotState::kLaunching) {
                            slot.requested_count = slot.assigned_count;
                        }
                        slot.state = bsp::AirOpsSlotState::kLaunched;
                        slot.timer = 0.0f;
                        if (slot.launch_requested) {
                            slot.timer = bsp::kAirOpsSlotCooldownSeconds;
                            slot.launch_requested = false;
                        }
                        ++land_retire_slot_states;
                    }
                }
            }
        }
    }

    bool unit_is_flight_leader_007b8ad0(std::size_t process_index) const {
        const bsp::PlaneSquadronHostRecord* const sqn =
            bsp::plane_squadron_registry().find_by_member_unit(process_index);
        if (sqn == nullptr) return true;
        for (const std::size_t member : sqn->member_units) {
            if (member == bsp::kPlaneSquadronNoUnit) continue;
            return member == process_index;   // +3D0h[0], the flight leader
        }
        return true;   // a record with no live member: the same unknown case
    }

    // `apply_position` false computes the station and hands it to the caller
    // WITHOUT teleporting the member onto it. Packet cc8_follow_attack: the
    // teleport writes `motion.position` only, while `plane_world_velocity` goes
    // on integrating from its own accelerations in the plane step
    // (`plane_world_velocity[i] += world_accel[i] * step`), so a member held
    // in follow by placement carries a velocity that never had to match the
    // motion the teleport displayed. The fly-over inherits that velocity at the
    // hand-over. docs/FOLLOWER_ATTACK_HANDOVER.md section 7.
    bool place_wing_member_on_station_007f23a0(
        GameUnitSlot& unit, bool once,
        bsp::PlaneFormationStation* station_out = nullptr,
        const GameUnitSlot** leader_out = nullptr,
        bool apply_position = true,
        bool report = true) {
        bsp::PlaneSquadronHostRecord* const squadron =
            bsp::plane_squadron_registry().find_by_member_unit(unit.process_index);
        if (squadron == nullptr) return false;
        // +3D0h in array order, skipping slots whose plane never became a unit,
        // which is live_count()'s own rule.
        std::vector<std::size_t> wing;
        for (const std::size_t member : squadron->member_units) {
            if (member != bsp::kPlaneSquadronNoUnit) wing.push_back(member);
        }
        if (wing.size() < 2u) return false;
        // 007EDA91 reads slot 0 whatever the count, and 009BFD92 refuses when
        // the leader is the unit itself.
        if (wing.front() == unit.process_index) return false;

        // 007ED260. `tools/callsite_census.py` gives it nine call sites where
        // `ghidra callers` gives four, and the one that decides the schedule is
        // 007F4BFA inside the spawn tail 007F4580, just past the 007F4B43
        // attach: the image assigns the indices AT SPAWN, and again on every
        // promote (007ED645), leave (007F3A11) and follow entry (009BEDE4).
        // This host runs the same rule once per squadron at the member's first
        // step, because at 007F4580 time its members are still plans with no
        // unit and no pose to transform against.
        if (!squadron->formation_indices_assigned
            || squadron->member_formation_index.size() != wing.size()) {
            squadron->member_formation_index.assign(wing.size(), 0);
            std::vector<std::int32_t> stamps(wing.size(), 0);
            bsp::plane_formation_assign_indices_007ed260(
                squadron->member_formation_index.data(), stamps.data(),
                static_cast<int>(wing.size()));
            squadron->formation_indices_assigned = true;
            std::string indices;
            for (std::size_t i = 0; i < wing.size(); ++i) {
                if (i != 0) indices += " ";
                indices += std::to_string(squadron->member_formation_index[i]);
            }
            log.notef("plane formation: squadron %s assigned 007ED260 indices [%s] over "
                "%zu live members; shape +3E4h=%d morale +3E8h=%.3f",
                squadron->name.c_str(), indices.c_str(), wing.size(),
                static_cast<int>(squadron->formation_shape_3e4),
                static_cast<double>(squadron->morale_3e8));
        }
        std::size_t seat = wing.size();
        for (std::size_t i = 0; i < wing.size(); ++i) {
            if (wing[i] == unit.process_index) { seat = i; break; }
        }
        if (seat >= squadron->member_formation_index.size()) return false;

        const std::size_t leader_index = wing.front();
        if (leader_index >= slots.size() || slots[leader_index] == nullptr) return false;
        const GameUnitSlot& leader = *slots[leader_index];

        // The triple is the authored one or nothing: without the installation's
        // own `Pilot/Follow/*` block there is no spacing to place anyone at, and
        // inventing one is exactly what this packet must not do.
        if (!lua.plane_globals_loaded()) return false;
        const bsp::GameTuningBlock& tuning = lua.plane_globals();

        bsp::PlaneFormationStationInputs in;
        in.formation_index = squadron->member_formation_index[seat];
        in.shape = squadron->formation_shape_3e4;
        in.morale = squadron->morale_3e8;
        // 007F23F0 `84 c0 75 6c` and 007F2403 `84 c0 75 59`, both JNZ into the
        // bomber arm: the LEADER's class chain decides for the whole wing.
        // `unit_is_kind_of` is the same 0074E400 model slot 5Ch uses.
        const bool bomber = bsp::plane_formation_uses_bomber_displacement(
            bsp::unit_is_kind_of(leader.class_id, 0x10),
            bsp::unit_is_kind_of(leader.class_id, 0x14));
        in.displacement[0] = bomber ? tuning.pilot_follow_bomber_displacement
                                    : tuning.pilot_follow_small_plane_displacement;
        in.displacement[1] = bomber ? tuning.pilot_follow_bomber_displacement_2
                                    : tuning.pilot_follow_small_plane_displacement_2;
        in.displacement[2] = bomber ? tuning.pilot_follow_bomber_displacement_3
                                    : tuning.pilot_follow_small_plane_displacement_3;
        in.symmetrical_position = tuning.pilot_follow_symmetrical_position;
        in.symmetrical_altitude = tuning.pilot_follow_symmetrical_altitude;
        // `[[squadron+35Ch]+A4h]`: squadron+35Ch is the plane class descriptor
        // and class+A4h is the Lua `Width` this host already carries.
        in.leader_class_width = leader.class_width_00a4;
        // The small-plane arm transforms by the leader's whole world matrix,
        // leader+0CCh (007F2456); the bomber arm rebuilds a yaw-only frame from
        // `leader->vtable[50h]()` with the same translation (007F24AB,
        // 007F24E4-007F2508). This host has the leader's published pose, which
        // IS leader+0CCh; for a bomber leader the roll and pitch rows are
        // therefore carried where the image would have flattened them, and that
        // difference is labelled, not hidden. On USN04 the leaders are
        // TorpedoBombers (11h), which take the small-plane arm, so it does not
        // arise there.
        in.leader_frame = leader.world;

        const bsp::PlaneFormationStation station =
            bsp::plane_formation_station_007f23a0(in);
        if (!station.produced) return false;
        // Handed out before the placement gate below, so the follow law can use
        // the station whether or not the placement that used to stand in for it
        // is enabled.
        if (station_out != nullptr) *station_out = station;
        if (leader_out != nullptr) *leader_out = &leader;

        // Reporting, from the first non-leader seat only so one squadron gives
        // one line. This runs whether or not the placement below is enabled,
        // which is what makes a before/after on the same binary possible.
        if (report && seat == 1u) {
            const std::int32_t tick = squadron->formation_report_ticks++;
            if (tick == 0 || (tick % 400) == 0) {
                std::string pairs;
                for (std::size_t a = 0; a < wing.size(); ++a) {
                    for (std::size_t b = a + 1u; b < wing.size(); ++b) {
                        if (wing[a] >= slots.size() || wing[b] >= slots.size()) continue;
                        const GameUnitSlot& ua = *slots[wing[a]];
                        const GameUnitSlot& ub = *slots[wing[b]];
                        float d2 = 0.0f;
                        for (int i = 0; i < 3; ++i) {
                            const float dv = ua.motion.position[i] - ub.motion.position[i];
                            d2 += dv * dv;
                        }
                        if (!pairs.empty()) pairs += " ";
                        pairs += std::to_string(a) + "-" + std::to_string(b) + "=";
                        pairs += std::to_string(static_cast<double>(std::sqrt(d2)));
                    }
                }
                log.notef("plane formation geometry: squadron %s tick=%d wing=%zu "
                    "pairwise=[%s] seat1 index=%d local=(%.1f %.1f %.1f) disp=(%.1f %.1f %.1f) "
                    "bomber_triple=%d",
                    squadron->name.c_str(), static_cast<int>(tick), wing.size(),
                    pairs.c_str(), in.formation_index,
                    static_cast<double>(station.local[0]),
                    static_cast<double>(station.local[1]),
                    static_cast<double>(station.local[2]),
                    static_cast<double>(in.displacement[0]),
                    static_cast<double>(in.displacement[1]),
                    static_cast<double>(in.displacement[2]),
                    bomber ? 1 : 0);
                // Every seat's station, not only seat 1's (packet
                // cc9_torpedo_approach_leader_speed): the formation index,
                // 007F23A0's local offset and the member's distance to it.
                std::string seats;
                for (std::size_t k = 1; k < wing.size(); ++k) {
                    if (wing[k] >= slots.size() || slots[wing[k]] == nullptr) continue;
                    if (k >= squadron->member_formation_index.size()) continue;
                    bsp::PlaneFormationStationInputs sin = in;
                    sin.formation_index = squadron->member_formation_index[k];
                    const bsp::PlaneFormationStation st = bsp::plane_formation_station_007f23a0(sin);
                    if (!st.produced) continue;
                    const GameUnitSlot& um = *slots[wing[k]];
                    float d2 = 0.0f;
                    for (int i = 0; i < 3; ++i) {
                        const float dv = um.motion.position[i] - st.world[i];
                        d2 += dv * dv;
                    }
                    char buf[128];
                    std::snprintf(buf, sizeof buf, "%s%zu:index=%d local=(%.1f %.1f %.1f) dist=%.1f",
                        seats.empty() ? "" : " ", k, sin.formation_index,
                        static_cast<double>(st.local[0]), static_cast<double>(st.local[1]),
                        static_cast<double>(st.local[2]), static_cast<double>(std::sqrt(d2)));
                    seats += buf;
                }
                const GameUnitSlot& ul = *slots[wing[0]];
                // pc24-lint: allow log-only speed for the formation-seats note; no state reads it
                const double lspd = std::sqrt(
                    static_cast<double>(ul.plane_world_velocity[0]) * ul.plane_world_velocity[0] +
                    static_cast<double>(ul.plane_world_velocity[1]) * ul.plane_world_velocity[1] +
                    static_cast<double>(ul.plane_world_velocity[2]) * ul.plane_world_velocity[2]);
                log.notef("plane formation seats: squadron %s tick=%d leader 2b4=%.2f speed=%.2f [%s]",
                    squadron->name.c_str(), static_cast<int>(tick),
                    static_cast<double>(ul.plane_desired_speed_2b4), lspd, seats.c_str());
            }
        }

        // The placement that stands in for 009BFEE0. `false` here is the BEFORE
        // half of this packet's measurement and is not a shipped configuration.
        if (!kPlaneFormationPlacementEnabled) return false;
        // Packet cc8_follow_attack: the station and the leader are already in
        // the out-params above, so the follow law still gets its geometry.
        if (!apply_position) return false;
        if (once) {
            // The image separates the wing inside the follow state and holds it
            // there. This host never enters that state - every plane of every
            // USN04 squadron reports `states[attackrun=...]` and
            // `prepare_entries=0` - so the only moment left is the member's
            // first step. Applied once, the member then flies its own run from
            // its own station, which is the geometry the run-in starts from.
            if (squadron->member_station_applied.size() != wing.size()) {
                squadron->member_station_applied.assign(wing.size(), 0u);
            }
            if (squadron->member_station_applied[seat] != 0u) return false;
            squadron->member_station_applied[seat] = 1u;
        }
        for (int i = 0; i < 3; ++i) unit.motion.position[i] = station.world[i];
        publish_pose(unit);
        return true;
    }

    // 007F2920 (body 007F2920-007F2BC9, __fastcall(squadron), RET), read whole from
    // the listing. With +3CCh members: +408h = 0 (007F293C); the leader's speed
    // vt[38h] (007F294A, 007B8E60) is kept; member 0 takes the squadron's +CCh
    // matrix through vt[88h] (007F298B) and runs 007C6340, vt[3Ch](speed),
    // 007C18B0(0), vt[D8h] and [+310h]+0Ch. Then for every member k >= 1
    // (007F2A50-007F2BB8): the matrix is the LEADER's +74h block (007F2A68 REP
    // MOVSD), its translation is 007F23A0(member+9D0h) (007F2A78), and the height
    // is floored: a station below 10.0 ([00CE38B8]) under a leader above it takes
    // min(leaderY, 10.0) ([00CE3DC0], 007F2A83-007F2AEC). vt[88h](matrix)
    // (007F2B23), 007C6340 (007F2B27), vt[3Ch](the leader's speed) (007F2B3C),
    // 007C18B0(0), vt[D8h], [+310h]+0Ch and the physics body sync (+804h).
    // Callers (tools/callsite_census.py): 007F2E8F (squadron vt[118h] 007F2E20,
    // PutTo's position set, after the base 00489760), 007F2F74, 007F2FC6 and
    // 007F4DB0 (pass C 007F4BA0, skipped when +408h is set).
    // SUBSTITUTIONS (labelled): the host fuses the squadron with member 0, so the
    // member-0 pose is the squadron's own placement and is not repeated;
    // 007C18B0, vt[D8h], [+310h]+0Ch and the +804h sync retain their substitutes.
    // OFF keeps flight state; ON applies 007C6340's typed selector below.
    // vt[3Ch] is set_forward_speed and is
    // skipped for a plane whose velocity is not seeded yet (its seed is the same
    // class TravelSpeed along the nose the leader is seeded with).
    // The selector's typed projection consumes the actual placed pose/class and
    // updates this instance; its remaining native callbacks keep their existing
    // host substitutions (controller reset, attachment, and presentation).
    void apply_squadron_spawn_placement(GameUnitSlot& unit);

    std::size_t place_squadron_members_007f2920(std::size_t squadron_unit, const char* site) {
        if (squadron_unit >= slots.size() || slots[squadron_unit] == nullptr) return 0;
        const bsp::PlaneSquadronHostRecord* squadron = nullptr;
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == squadron_unit) {
                squadron = &r;
                break;
            }
        }
        if (squadron == nullptr) return 0;
        ++squadron_member_place_calls;
        GameUnitSlot& leader = *slots[squadron_unit];
        publish_pose(leader);   // the position the squadron was just given
        const float speed = leader_live_speed_007b8e60(leader);   // 007F294A vt[38h]
        // 007F2929 skips an empty member array. 007F299E then places
        // member zero using the squadron pose just published above.
        if (!squadron->member_units.empty()) apply_squadron_spawn_placement(leader);
        std::size_t placed = 0, unseeded = 0;
        std::string where;
        for (const std::size_t member : squadron->member_units) {
            if (member == bsp::kPlaneSquadronNoUnit || member == squadron_unit) continue;
            if (member >= slots.size() || slots[member] == nullptr) continue;
            GameUnitSlot& unit = *slots[member];
            bsp::PlaneFormationStation station;
            const GameUnitSlot* station_leader = nullptr;
            place_wing_member_on_station_007f23a0(unit, false, &station, &station_leader,
                false, false);
            if (!station.produced || station_leader != &leader) {
                ++squadron_member_place_no_station;
                continue;
            }
            float y = station.world[1];
            if (10.0f > y) {                                      // 007F2A8B [00CE38B8]
                const float leader_y = leader.motion.position[1];   // 007F2AB2 +100h
                if (leader_y > y) y = leader_y > 10.0 ? 10.0f : leader_y;   // [00CE3DC0]
            }
            for (int i = 0; i < 3; ++i) {                         // 007F2A68 the +74h rows
                unit.motion.pose_row0[i] = leader.motion.pose_row0[i];
                unit.motion.pose_row1[i] = leader.motion.pose_row1[i];
                unit.motion.pose_row2[i] = leader.motion.pose_row2[i];
            }
            unit.motion.position[0] = station.world[0];
            unit.motion.position[1] = y;
            unit.motion.position[2] = station.world[2];
            publish_pose(unit);                                   // 007F2B23 vt[88h]
            apply_squadron_spawn_placement(unit);               // 007F2B27
            if (unit.plane_velocity_seeded) {                     // 007F2B3C vt[3Ch]
                const float* fwd = unit.motion.pose_row2;
                const float len = std::sqrt(fwd[0] * fwd[0] + fwd[1] * fwd[1] + fwd[2] * fwd[2]);
                if (len > 1e-6f) {
                    for (int i = 0; i < 3; ++i) {
                        unit.plane_world_velocity[i] = speed * fwd[i] / len;
                        unit.plane_body_angular[i] = 0.0f;
                    }
                    unit.motion.linear_velocity = bsp::OceanVec3{unit.plane_world_velocity[0],
                        unit.plane_world_velocity[1], unit.plane_world_velocity[2]};
                }
            } else {
                ++unseeded;
            }
            ++placed;
            char buf[96];
            std::snprintf(buf, sizeof buf, " %s=(%.1f %.1f %.1f)", unit.row.name.c_str(),
                static_cast<double>(unit.motion.position[0]), static_cast<double>(y),
                static_cast<double>(unit.motion.position[2]));
            where += buf;
        }
        squadron_members_placed += placed;
        squadron_member_place_unseeded += unseeded;
        log.notef("squadron members placed: squadron=%s site=%s leader=(%.1f %.1f %.1f) speed=%.2f "
            "members=%zu unseeded=%zu%s (007F2920, packet cc9_squadron_member_placement)",
            leader.row.name.c_str(), site, static_cast<double>(leader.motion.position[0]),
            static_cast<double>(leader.motion.position[1]),
            static_cast<double>(leader.motion.position[2]), static_cast<double>(speed),
            placed, unseeded, where.c_str());
        done("PlaneSquadron::place_members_007f2920", 0x007f2920u);
        return placed;
    }

    void refresh_row(GameUnitSlot& slot);
    void issue_into_ring(GameUnitSlot& slot, float throttle, float rudder);

    // The heading 00835ac0 latches. The unit virtual at primary slot 50h is a
    // RET 0 getter with no reconstruction, so this is the executable's own
    // value: atan2 over pose row 2, the same convention the trajectory dump and
    // the run log print in degrees.
    // 007C47F0(approach+8h): tuning+24Ch Dynamics/SpdMultipliers/LevelFlight
    // times classDesc+184h StallSpd. Both halves are real here: the tuning row
    // comes from the PlaneGlobals mirror and the stall speed from the unit's own
    // vehicle-class row, which src/game_hosts_lua.cpp loads and the free-flight
    // arm already reads at 007DB760. docs/BOT_SPEED_CLASS_ROWS.md.
    // Packet cc9_dogfight_moveto: VehicleClass[id].BSPPilotFires = 1 when any
    // platform with at least one gun authors PilotFires = true, else 0.
    // tuning+66Ch..678h, Pilot/AutoStrafeAngle (radians), 0 without the globals.
    float strafe_cone_tuning(int off) const {
        if (!lua.plane_globals_loaded()) return 0.0f;
        const bsp::GameTuningBlock& g = lua.plane_globals();
        switch (off) {
            case 0x66C: return g.pilot_auto_strafe_angle_angle_prepare;
            case 0x670: return g.pilot_auto_strafe_angle_angle_move_to;
            case 0x674: return g.pilot_auto_strafe_angle_angle_go_away;
            default: return g.pilot_auto_strafe_angle_angle_strafe;
        }
    }
    // Packet cc13-steergate (docs/CC13_STEER_GATE.md). 0099B630
    // BSP_PilotCommand_GetPitchTargetOrLive, __thiscall(cmd), ST0, plain RET:
    // cmd+2D0h != 0 ? cmd+2BCh : (cmd+2F0h)->+C64h.
    static float commanded_pitch_0099b630(const GameUnitSlot& u) {
        return u.plan_state.pitch_mode_2d0 != 0 ? u.plan_state.pitch_target_2bc
                                                 : u.plane_pitch_angle_c64;
    }
    // 009A1A20 BSP_BotTask_CommandedHeading, __fastcall(ECX = cmd), ST0: cmd+2CCh == 2
    // ? cmd+2C0h : 00438AA0(unit->vtable[50h]() (the heading, unit+C6Ch),
    // 0099D0A0(cmd, unit+C68h)). The 0099D0A0 inputs are the ones the planner's roll
    // arm passes at 0099E0C1 (pilot_plan_roll), with the raw bank as the argument and
    // the class's own TurnRoll (0099D19E class+25Ch; the leader's +260h is the
    // planner's choice at 0099DF9A, not 0099D0A0's). plan+2F0h is never null here.
    float commanded_heading_009a1a20(GameUnitSlot& u) const {
        if (u.plan_heading_mode_2cc == 2) return u.plan_heading_2c0;
        ++u.sg_heading_fallbacks;
        bsp::PilotHeadingDiffInputs in;
        in.bank_request = u.plane_bank_angle_c68;
        in.pitch_angle = u.plane_pitch_angle_c64;
        {
            // 0099B260 (bsp.py show --asm): byte +8 selects float +4 or +0.
            // PilotPlanSlot is the retained live plan slot, not a fabricated
            // input buffer. This replaces the same inline selection below;
            // 0099D0A0 consumes its result as the commanded pitch.
            static_assert(offsetof(bsp::PilotPlanSlot, current) == 0);
            static_assert(offsetof(bsp::PilotPlanSlot, desired) == 4);
            static_assert(offsetof(bsp::PilotPlanSlot, active) == 8);
            static_assert(sizeof(bsp::PilotPlanSlot) == 0x0c);
            in.pitch_command = bsp::pilot_bot_override_or_value_0099b260(
                &u.plan_slots[bsp::kPilotSlotPitch]);
        }
        in.unit_is_null = false;
        in.caps_rate_at_one = bsp::unit_is_kind_of(u.class_id, 0x10) ||
                              bsp::unit_is_kind_of(u.class_id, 0x16);
        in.roll_spd = u.plane_class.roll_spd;
        in.pitch_spd = u.plane_class.pitch_spd;
        in.yaw_spd = u.plane_class.yaw_spd;
        in.slide_ratio = u.plane_class.slide_ratio;
        in.roll_accel = u.plane_class.roll_accel;
        in.turn_roll_spd = u.plane_class_turn_roll_spd;
        in.turn_roll = u.plane_class_turn_roll;
        if (lua.plane_globals_loaded()) {
            const bsp::GameTuningBlock& g = lua.plane_globals();
            in.hdg_diff_calc_limit_1 = g.pilot_general_hdg_diff_calc_limit_1;
            in.hdg_diff_calc_limit_2 = g.pilot_general_hdg_diff_calc_limit_2;
            in.hdg_diff_calc_min_pitch = g.pilot_general_hdg_diff_calc_min_pitch;
            in.hdg_diff_calc_min_roll = g.pilot_general_hdg_diff_calc_min_roll;
        }
        return bsp::wrapped_angle_add_00438aa0(u.plane_heading_c6c,
                                               bsp::pilot_heading_diff_0099d0a0(in));
    }
    // 009FABE0(ECX = [approach+1Ch], heading, pitch) at the call `site`.
    struct SteerGateSiteCensus {
        int writes{0}, gate_ticks{0}, to_pass{0}, to_block{0};
    };
    std::map<std::uint32_t, SteerGateSiteCensus> steer_gate_sites_;
    void gun_direction_009fabe0(GameUnitSlot& u, std::uint32_t site, float heading,
                                float pitch) {
        if constexpr (bsp::kBotCommandedDirection68Bound) {
            const std::array<float, 3> d = bsp::bot_commanded_direction_009fabe0(heading, pitch);
            u.gun_direction_68[0] = d[0];
            u.gun_direction_68[1] = d[1];
            u.gun_direction_68[2] = d[2];
            u.sg_site_68 = site;
            ++u.sg_direction_writes;
            ++steer_gate_sites_[site].writes;
        }
    }
    // The common shape: 009FABE0(009A1A20(cmd), 0099B630(cmd)); the image calls
    // 0099B630 first, and neither has a side effect.
    void gun_direction_from_plan_009fabe0(GameUnitSlot& u, std::uint32_t site) {
        if constexpr (bsp::kBotCommandedDirection68Bound) {
            const float pitch = commanded_pitch_0099b630(u);
            gun_direction_009fabe0(u, site, commanded_heading_009a1a20(u), pitch);
        }
    }
    bool pilot_fires_flattened{false};
    void ensure_pilot_fires_flattened() {
        if (pilot_fires_flattened) return;
        pilot_fires_flattened = true;
        static const char chunk[] =
            "if type(VehicleClass) == 'table' then\n"
            "  for id, row in pairs(VehicleClass) do\n"
            "    if type(row) == 'table' then\n"
            "      local pf = 0\n"
            "      if type(row.Platforms) == 'table' then\n"
            "        for k = 1, 64 do\n"
            "          local p = row.Platforms[k]\n"
            "          if type(p) == 'table' and type(p.Gun) == 'table'\n"
            "             and type(p.Gun[1]) == 'number' and p.PilotFires == true then\n"
            "            pf = 1\n"
            "          end\n"
            "        end\n"
            "      end\n"
            "      row.BSPPilotFires = pf\n"
            "    end\n"
            "  end\n"
            "end\n";
        if (lua.luaL_loadbuffer(chunk, static_cast<int>(sizeof(chunk) - 1), "bsp_pilot_fires") != 0) {
            log.note("pilot fires: flatten chunk did not compile; PilotFires stays true");
            lua.lua_settop(-2);
            return;
        }
        if (lua.lua_pcall(0, 0, 0) != 0) {
            log.note("pilot fires: flatten chunk failed; PilotFires stays true");
            lua.lua_settop(-2);
            return;
        }
        done("Plane::compute_pilot_fires_007cd930", 0x007cd930u);
    }

    float bot_desired_speed_007c47f0(const GameUnitSlot& slot) const {
        float level_flight = 1.8f;                    // tuning+24Ch
        if (lua.plane_globals_loaded()) {
            level_flight = lua.plane_globals().dynamics_spd_multipliers_level_flight;
        }
        const float stall = slot.plane_stall_spd > 0.0f ? slot.plane_stall_spd : 17.5f;
        return level_flight * stall;
    }

    // Hoisted out of the dive-bomb arm class, packet cc9_follow_package, so
    // that the torpedo seam (follow_base_tick_009c1fd0, 009D2731) CAN call it:
    // 009D2731 reaches 009C1FD0 and so 009BEE30 exactly as the dive-bomb
    // follow state does (docs/PLANE_FOLLOW_HOLD_ARM.md section 11). The torpedo
    // seam does not call it yet. Text move only; behaviour unchanged.
    // 009C2068 CALL 009BFEE0 then 009C2077 CALL 009BEE30, in
    // that order and with the station 009BFD70 produced, which
    // is the tick's own order (section 2 of
    // docs/PLANE_FOLLOW_LAW.md).
    // 0099B790 (body 0099B790-0099B935): clamp (lo <= -1 -> [00CFBC84] -5.0,
    // hi >= 1 -> [00CE3850] 5.0), then merge with every pair it overlaps, the
    // running union widening as the walk goes; the union replaces the first
    // overlapped pair and the others are removed by moving the last pairs in.
    // With no overlap it appends while fewer than 24 (18h) pairs are held.
    static void ga_insert_0099b790(GameUnitSlot& u, int set, float lo, float hi) {
        if (lo <= -1.0f) lo = -5.0f;
        if (1.0f <= hi) hi = 5.0f;
        auto& b = u.ga_band[set];
        int& n = u.ga_band_n[set];
        int hit[24];
        int nh = 0;
        for (int i = 0; i < n; ++i) {
            if (lo < b[i][0]) {
                if (b[i][0] <= hi) {
                    if (hi < b[i][1]) hi = b[i][1];
                    hit[nh++] = i;
                }
            } else if (lo <= b[i][1]) {
                lo = b[i][0];
                if (hi < b[i][1]) hi = b[i][1];
                hit[nh++] = i;
            }
        }
        if (nh > 0) {
            b[hit[0]][0] = lo;
            b[hit[0]][1] = hi;
            // Removing the other overlapped pairs: set order does not change
            // what 0099B940 finds, because the held pairs are disjoint.
            for (int k = nh - 1; k >= 1; --k) {
                const int i = hit[k];
                b[i][0] = b[n - 1][0];
                b[i][1] = b[n - 1][1];
                --n;
            }
            return;
        }
        if (n < 24) {
            b[n][0] = lo;
            b[n][1] = hi;
            ++n;
        }
    }
    // 0099B940: true keeps (possibly moved to the nearer edge of the first
    // pair holding it), false when that pair covers all of [-1, 1].
    static bool ga_band_search_0099b940(const GameUnitSlot& u, int set, float& v) {
        const auto& b = u.ga_band[set];
        for (int i = 0; i < u.ga_band_n[set]; ++i) {
            const float lo = b[i][0], hi = b[i][1];
            // 0099B977 FCOMI / JBE and 0099B97F FCOMIP / JA: an unordered pair
            // is skipped, as v <= lo || hi <= v would not (NaN only).
            if (!(v > lo) || !(hi > v)) continue;
            if (lo < -1.0f && 1.0f < hi) return false;
            v = (hi - v < v - lo) ? hi : lo;
            return true;
        }
        return true;
    }
    void vehicle_avoidance_007df4f0(GameUnitSlot& u, float dt);
    static bool nb_in_world(const GameUnitSlot& o);
    void nb_near_30_tick_007e2010(GameUnitSlot& u, float dt);
    bool nb_has_object_c50(const GameUnitSlot& u) const;
    void nb_near_30_fixed_step(float step);
    bool friendly_in_line_007b96d0(GameUnitSlot& u, float dt);
    void friendly_in_line_clock_007e2010(GameUnitSlot& u, float dt);
    bool friendly_in_line_eval_007dedb0(GameUnitSlot& u);
    void terrain_avoidance_0099f1c0(GameUnitSlot& u, float dt);
    void terrain_avoidance_water_009a1420(GameUnitSlot& u);
    float avoid_surface_height(float x, float z);
    static bool avoid_in_dive(const GameUnitSlot& u);

    // 009A17D0's gunfire arm and 0099EC40, for one plane, per think.
    void gunfire_avoidance_009a17d0(GameUnitSlot& u, float dt) {
        // 009A17E9 zeroes the whole +26Ch dword; bit 4 is the only bit the
        // host carries (no other +26Ch bit is modelled or read).
        u.ga_flag_26c_4 = false;                                   // 009A17E9
        u.av_throttle_25c = 1.0f;                                  // 009A17EF [00D7A24C]
        for (int& n : u.ga_band_n) n = 0;                          // +C8h/+190h/+258h
        // 009A17F8: an AI plane (+520h clear), live (+5Dh clear), in free
        // flight (+72Ch vtable[38h], mode 7) or mode 6, whose party is not the
        // local player's ([[00E188A8]+5FCh]+908h; the controlled unit's party
        // stands in, labelled).
        if (u.generic_suppress_520) return;
        if (u.state == nullptr || u.state->simulate != 0) return;
        const bool free_flight = u.plane_control_mode_900 == 7;
        if (!free_flight && u.plane_control_mode_900 != 6) return;
        if (!(controlled_bound && controlled_index < slots.size())) return;
        if constexpr (kAvoidanceDummyAiGateBound) {
            // 009A184A-009A1871: skip when [[00E188A8]+5FCh] (the scene record)
            // is null, or when its +908h equals unit+54h. +908h is the scene's
            // `DummyAIEnabled` (004F20EE-004F2101), 3 when absent (004F2109,
            // and 004DA4CF in the constructor); unit+54h is the side. So in a
            // mission that does not author the key no side is skipped.
            // SUBSTITUTION, labelled: the key is not plumbed from the scene
            // header to this host, so the default 3 stands in.
            // docs/TERRAIN_ARM_US_FIGHTERS.md.
            if (u.row.party == bsp::kSceneDefaultDummyAi) return;
        } else {
            if (slots[controlled_index]->row.party == u.row.party) return;
        }
        // 009A1865/009A194F/009A1972: +2E4h bits 4, 2, 1 and the squadron's
        // +3A4h bits. +2E4h is the task's per-tick mask (0FFh at 009C87A3 /
        // 009D4865, copied at 009C8855 / 009D48FF, 0FFh at 00999944 before
        // the task arm); no literal store clears a bit of +4C4h or +49Ch, so
        // the mask stays 0FFh through the dive and torpedo tasks. The
        // squadron's +3A4h is 0FFh from its constructor: 007F2CA2 passes
        // ECX = squadron+37Ch to 007F2BD0, whose 007F2C13 stores [ECX+28h] =
        // 0FFh. Only the SquadronEnable*Avoidance bindings (0089FE50 and
        // siblings) change it, and no USN04 script calls them. Confirmed, not
        // a substitution (docs/PLANE_SUBSTITUTION_SWEEP.md). BSM09 calls
        // SquadronEnableVehicleAvoidance(false) on its five D3A Val squadrons;
        // the vehicle bit is read below under kLuaSquadronEnableVehicleAvoidanceBound.
        if constexpr (kPilotGunfireAvoidanceBound) {
            if (!free_flight) {
                u.ga_timer_3e8 = -1.0f;                            // 009A193F [00D7A260]
            } else {
                if (u.ga_rest_3ec <= u.ga_timer_3e8) u.ga_timer_3e8 -= dt;  // 009A18A8
                // Packet cc9_lua52_kamikaze_wiring: 009A18BA TEST +2E4h, 4.
                if ((u.ga_timer_3e8 > 0.0f || u.ga_timer_3e8 < u.ga_rest_3ec)
                    && (u.task_mask_2e4 & 4) != 0) {
                    gunfire_detect_0099ec40(u);
                }
                if (!u.ga_flag_26c_4 && u.ga_timer_3e8 > 0.0f) {   // 009A190F
                    u.ga_timer_3e8 = -0.0f - u.ga_timer_3e8;
                }
            }
        }
        // Packet cc9_lua52_kamikaze_wiring: 009A1951 / 009A1974 TEST +2E4h, 2 / 1
        // (the squadron's +3A4h stays 0FFh, as above, until 008A0000 changes it).
        if constexpr (kPilotVehicleAvoidanceBound) {
            // Packet cc13-luawire 7b: 009A1959-009A1969, pilot+2FCh (the squadron)
            // null, or its +3A4h bit 2 set. The squadron is this host's squadron
            // slot (the plane's own slot when it has no record, whose mask is the
            // constructor's 0FFh, as a null squadron passes).
            bool squadron_allows = true;
            if constexpr (bsp::kLuaSquadronEnableVehicleAvoidanceBound) {
                const GameUnitSlot* sq = squadron_slot_of(u.process_index);
                squadron_allows = sq == nullptr || (sq->sq_avoid_3a4 & 2u) != 0;
            }
            if ((u.task_mask_2e4 & 2) != 0 && squadron_allows) {
                // bsp.py show 0099B670 --asm: only [pilot+2F4h] and
                // [unit+C50h] are read before the nullable dispatch. Borrow
                // those two cells; the neighbour identity is this live
                // unit's retained +30h list, refreshed by 007E2010. Presence
                // uses the same 007D621F allocation predicate as that refresh.
                // These are call-scoped views, not full native unit/task
                // objects, and their unobserved padding is never consumed.
                struct UnitView {
                    std::byte unobserved[0xc50];
                    void* neighbours;
                } unit_view{{}, nb_has_object_c50(u) ? &u.nb_near_30 : nullptr};
                struct PilotView {
                    std::byte unobserved[0x2f4];
                    UnitView* unit;
                } pilot_view{{}, &unit_view};
                static_assert(offsetof(UnitView, neighbours) == 0xc50);
                static_assert(offsetof(PilotView, unit) == 0x2f4);
                class Calls final : public bsp::PilotVehicleAvoidanceCalls {
                public:
                    Calls(Impl& owner, GameUnitSlot& unit, float dt)
                        : owner_(owner), unit_(unit), dt_(dt) {}
                    void vehicle_avoidance_007df4f0(void*, void*) override {
                        // The borrowed identities above bind this one unit;
                        // the existing geometry/band implementation reads and
                        // writes its real state. No second collision pass.
                        owner_.vehicle_avoidance_007df4f0(unit_, dt_);
                    }
                private:
                    Impl& owner_;
                    GameUnitSlot& unit_;
                    float dt_;
                } calls(*this, u, dt);
                bsp::pilot_bot_avoid_vehicles_0099b670(calls, &pilot_view);
            }
        }
        if constexpr (kPilotTerrainAvoidanceBound) {
            if ((u.task_mask_2e4 & 1) != 0) {
                terrain_avoidance_0099f1c0(u, dt);                 // 009A1990
            }
        }
        // 009A1995: pilot+260h = 0; it has no host producer.
    }

    void gunfire_detect_0099ec40(GameUnitSlot& u) {
        ++u.ga_detect_ticks;
        double acc_x = 0.0, acc_y = 0.0;                           // [ESP+0Ch]/[+10h]
        const bsp::PlaneSquadronHostRecord* own_sq =
            bsp::plane_squadron_registry().find_by_member_unit(u.process_index);
        for (const auto& owned : slots) {
            const GameUnitSlot& s = *owned;
            // [00F87278]: planes whose lastGunState +C35h is set (007C74A0 /
            // 007C75A0 from 007CC345). The host's gunFire byte +BC9h stands in
            // for +C35h, labelled.
            if (&s == &u || !s.plane_gun_fire_bc9) continue;
            // DIAGNOSTIC, env-gated (BSP_GUN_LIST_TRACE=1): a killed plane (C3Ah) that
            // still holds the gunFire stand-in, once per shooter, and whether the
            // +5Dh test below drops it. Prints nothing when unset.
            static const bool gun_list_trace = [] {
                char* v = nullptr;
                std::size_t n = 0;
                const bool on = _dupenv_s(&v, &n, "BSP_GUN_LIST_TRACE") == 0 && v != nullptr;
                std::free(v);
                return on;
            }();
            if (gun_list_trace && s.plane_death_c3a) {
                static std::vector<const GameUnitSlot*> traced;
                if (std::find(traced.begin(), traced.end(), &s) == traced.end()) {
                    traced.push_back(&s);
                    log.notef("  gun list trace %s: killed at %.2f s, gunFire set, "
                        "+5Dh=%d (%s) at %.2f s", s.row.name.c_str(),
                        static_cast<double>(s.plane_death_seconds),
                        s.state != nullptr ? static_cast<int>(s.state->simulate) : -1,
                        (s.state == nullptr || s.state->simulate != 0) ? "dropped" : "COUNTED",
                        static_cast<double>(summary.simulated_seconds));
                }
            }
            if (s.state == nullptr || s.state->simulate != 0) continue;
            // 0099EC73: own squadron skipped (+9D4h).
            if (own_sq != nullptr &&
                bsp::plane_squadron_registry().find_by_member_unit(s.process_index) == own_sq) {
                continue;
            }
            float l[3];
            follow_to_body(s, u.motion.position, false, l);        // 004142E0, shooter +110h
            if (!(1.0f < l[2]) || !(l[2] < 800.0f)) continue;       // [00D7A24C], double [00CE3948]
            float v[3];
            follow_to_body(s, u.plane_world_velocity, true, v);    // vtable[34h], 0042D0D0
            bool outside_cone = false;
            if constexpr (bsp::kPc24GunneryCc13Bound) {
                // PC24 (pc24w1-gunnery-2): 0099ED64..0099ED86 v + l stored as floats;
                // the 15 band on the stored sum, each arm FSUB/FADD/FSUBR stored
                // (0099ED8A..0099EDCE); 0099EDD6/0099EDE0 FDIV by l.z stored; then
                // ry*ry + rx*rx (0099EDF6..0099EDFC) against [00D1F3F0], JBE skips
                // (unordered too).
                using namespace bsp::cc13x87;
                auto band15 = [](float a) {
                    float r = 0.0f;
                    if (a > 15.0) r = static_cast<float>(x87_sub24(a, 15.0));
                    else if (-15.0 > a) r = static_cast<float>(x87_add24(a, 15.0));
                    else r = static_cast<float>(x87_sub24(a, a));   // FSUBR ST0,ST0
                    return r;
                };
                const float fx = band15(static_cast<float>(x87_add24(v[0], l[0])));
                const float fy = band15(static_cast<float>(x87_add24(v[1], l[1])));
                const float rx = static_cast<float>(x87_div24(fx, l[2]));
                const float ry = static_cast<float>(x87_div24(fy, l[2]));
                outside_cone = !(0.014399999752640724 >
                                 x87_add24(x87_mul24(ry, ry), x87_mul24(rx, rx)));
            } else {
                auto band15 = [](double a) {                       // doubles [00CF3F20] 15, [00CE3D58] -15
                    if (a > 15.0) return a - 15.0;
                    if (a >= -15.0) return 0.0;
                    return a + 15.0;
                };
                const double fx = band15(static_cast<double>(l[0]) + v[0]);
                const double fy = band15(static_cast<double>(l[1]) + v[1]);
                const double rx = fx / l[2], ry = fy / l[2];
                outside_cone = !(rx * rx + ry * ry < 0.014399999752640724);   // double [00D1F3F0]
            }
            if (outside_cone) continue;
            u.ga_flag_26c_4 = true;                                // 0099EE11
            // 0099EE1F-0099EF02: the point (0, 0, z) on the shooter's line,
            // world (+CCh pose), minus the own world velocity, in the own frame.
            const float* m = s.world.data();
            float w[3];
            if constexpr (bsp::kPc24GunneryCc13Bound) {
                // PC24: 004142E0 on (0, 0, z) as ((0*up + 0*right) + z*forward) + origin,
                // each op rounded, stored; 0099EE67..0099EE90 minus the velocity, stored.
                using namespace bsp::cc13x87;
                for (int i = 0; i < 3; ++i) {
                    const float point = static_cast<float>(x87_add24(
                        x87_add24(x87_add24(x87_mul24(0.0, m[4 + i]), x87_mul24(0.0, m[i])),
                                  x87_mul24(l[2], m[8 + i])),
                        m[12 + i]));
                    w[i] = static_cast<float>(x87_sub24(point, u.plane_world_velocity[i]));
                }
            } else {
                for (int i = 0; i < 3; ++i) {
                    w[i] = static_cast<float>(m[12 + i] + static_cast<double>(l[2]) * m[8 + i]) -
                           u.plane_world_velocity[i];
                }
            }
            float o[3];
            follow_to_body(u, w, false, o);
            if constexpr (bsp::kPc24GunneryCc13Bound) {
                // 0099EECA..0099EF02: the sums are float locals ([ESP+14h], [ESP+18h]).
                acc_x = static_cast<float>(bsp::cc13x87::x87_add24(o[0], acc_x));
                acc_y = static_cast<float>(bsp::cc13x87::x87_add24(o[1], acc_y));
            } else {
                acc_x += o[0];
                acc_y += o[1];
            }
        }
        if (!u.ga_flag_26c_4) return;
        ++u.ga_flagged_ticks;
        if (u.ga_timer_3e8 <= 0.0f && u.ga_timer_3e8 != 0.0f) {      // 0099EF2D
            float lo = 2.0f, hi = 3.0f, rlo = 5.0f, rhi = 8.0f;
            if (lua.plane_globals_loaded()) {
                const bsp::GameTuningBlock& g = lua.plane_globals();
                lo = g.pilot_avoidance_gunfire_avoid_time_1;
                hi = g.pilot_avoidance_gunfire_avoid_time_2;
                rlo = g.pilot_avoidance_gunfire_wait_time_1;
                rhi = g.pilot_avoidance_gunfire_wait_time_2;
            }
            u.ga_timer_3e8 = stream_set2_draw_00bd2f10(u, u.row.name + "#ga", lo, hi);
            u.ga_rest_3ec = -stream_set2_draw_00bd2f10(u, u.row.name + "#gr", rlo, rhi);
            // 0099EFA5-0099F029: a kind-13h plane (MPlaneFighter) scales both by
            // its pilot level factor. Switch OFF: not applied (the old label:
            // no USN04 attacker is kind 13h in this host's class table).
            if constexpr (kPilotFighterGunfireScaleBound) {
                if (bsp::unit_is_kind_of(u.class_id, 0x13)) {             // 0099EFB8 vtable[5Ch]
                    // [[unit+DF4h]+34h], read live (009F9D08 reads the same index
                    // for the approach row). The f = 1.0f arm for a null unit or
                    // +DF4h (0099EFC0-0099EFE0) has no host counterpart.
                    const int level = kSkillLevelBound ? u.pilot_skill_index : 1;
                    // 0099EFE2-0099EFEF FILD, FMUL double [00CE3DC8] 0.30000001192,
                    // FLD1, FSUBRP, FSTP float; 24-bit order (equal at 53 for 0..5).
                    const float p = static_cast<float>(static_cast<double>(level) * 0.30000001192092896);
                    float f = static_cast<float>(1.0 - static_cast<double>(p));
                    // 0099EFF7-0099F00B: double [00CE65D0] 0.4f widened, FCOMPI, JBE
                    // keeps f (NaN too); else f = float [00CE7804] 0.4f.
                    if (0.4f > f) f = 0.4f;
                    u.ga_timer_3e8 = u.ga_timer_3e8 * f;                   // 0099F01B
                    u.ga_rest_3ec = u.ga_rest_3ec / f;                     // 0099F023 FDIVR
                }
            }
        }
        // 0099F03C-0099F081: kinds 10h and 16h, and a squadron flight leader
        // with 2+ members, take the 0099F197 arm (unit+840h = BomberVSGunfire).
        const bool exempt = bsp::unit_is_kind_of(u.class_id, 0x10) ||
            bsp::unit_is_kind_of(u.class_id, 0x16) ||
            (unit_is_flight_leader_007b8ad0(u.process_index) && own_sq != nullptr &&
             own_sq->live_count() > 1);
        if (exempt) {
            ++u.ga_exempt_ticks;
            if constexpr (kGunfireExemptWandererMulBound) {
                // 0099F197 0042E740, 0099F19C FLD [tuning+614h], 0099F1A8 FSTP [unit+840h].
                if (lua.plane_globals_loaded())
                    u.wanderer.mul = lua.plane_globals().pilot_avoidance_gunfire_bomber_vs_gunfire;
            }
            return;
        }
        // 0099F087 004F2F40: the 2-D threat sum normalised, then x 0.8
        // (double [00CE3D40]).
        double nx = 0.0, ny = 0.0;
        if constexpr (bsp::kPc24GunneryCc13Bound) {
            // PC24, 004F2F40: s = x*x + y*y stored; len = sqrt(s) (00BF7030) stored when
            // s > 1e-10 [00CE3820], else 1e-5 [00CE3C70] stored; both components are
            // divided by len (no zero arm).
            using namespace bsp::cc13x87;
            const float s2 = static_cast<float>(
                x87_add24(x87_mul24(acc_y, acc_y), x87_mul24(acc_x, acc_x)));
            float len = static_cast<float>(1.0e-5);
            if (s2 > 1.0e-10) len = static_cast<float>(std::sqrt(static_cast<double>(s2)));
            nx = static_cast<float>(x87_div24(acc_x, len));
            ny = static_cast<float>(x87_div24(acc_y, len));
        } else {
            const double len = std::sqrt(acc_x * acc_x + acc_y * acc_y);
            if (len > 1e-10) { nx = acc_x / len; ny = acc_y / len; }
        }
        const float x = static_cast<float>(nx * 0.800000011920929);
        const float y = static_cast<float>(ny * 0.800000011920929);
        const float x8 = static_cast<float>(static_cast<double>(x) * 0.800000011920929);
        if (x > 0.0f) {
            ga_insert_0099b790(u, 0, -x, 0.8f);                    // 0099F0D3, +4h
            ga_insert_0099b790(u, 2, -x8, 0.71f);                  // 0099F13A, +194h
        } else {
            ga_insert_0099b790(u, 0, -0.8f, -x);                   // 0099F10D
            ga_insert_0099b790(u, 2, -0.71f, -x8);                 // 0099F13A
        }
        if (y > 0.0f) ga_insert_0099b790(u, 1, -y, 0.7f);          // 0099F172, +CCh
        else ga_insert_0099b790(u, 1, -0.7f, -y);                  // 0099F18C
    }

    // 0099BF30 BSP_PilotBot_RepairCommandBands over 0099BC00's six-dword
    // buffer, in its order (kPilotCmdYaw 0, Pitch 1, Roll 2), then the
    // bot+258h (pilot+25Ch) throttle arm.
    void gunfire_repair_0099bf30(GameUnitSlot& u) {
        const bool free_flight = u.plane_control_mode_900 == 7;   // +72Ch vtable[38h]
        float* c = u.pilot_command_block;
        auto finish = [&](float& slot, float v) {
            if (slot != v) {
                slot = v < -1.0f ? -1.0f : (v > 1.0f ? 1.0f : v);
                ++u.ga_repairs;
                // 0099C011-0099C032 / 0099C0CC-0099C0ED: inside the changed arm,
                // 007D7A40 when the +72Ch predicate holds (packet cc9_plane_rate_floor).
                if constexpr (GameUnitsHost::Impl::kPlaneSpeedFloorBound) {
                    if (free_flight) {
                        // 007D7A40: cc13_seg48_pilot_plan_reset_command_timer_007d7a40 arms
                        // dyn+C8h (plane_speed_floor_timer_c8, the plane's [ctl+10h] block,
                        // never null here); OFF keeps the inline 1.2f store.
                        bsp::cc13_seg48_pilot_plan_reset_command_timer_007d7a40(
                            &u.plane_speed_floor_timer_c8);
                        ++u.speed_floor_arms;
                    }
                }
            }
        };
        if (free_flight) {                                        // cmd[1], the bot+C8h set
            float v = c[bsp::kPilotCmdPitch];
            if (!ga_band_search_0099b940(u, 1, v)) {
                const float ab = std::fabs(u.plane_bank_angle_c68);
                v = ab <= 1.5707963705062866f ? 1.1f : -1.1f;      // [00CE3830], [00CE6448]/[00D06BB0]
            }
            finish(c[bsp::kPilotCmdPitch], v);
        }
        {                                                         // cmd[0], the bot+0 set
            float v = c[bsp::kPilotCmdYaw];
            if (!ga_band_search_0099b940(u, 0, v)) {
                v = u.plane_bank_angle_c68 <= 0.0f ? -1.1f : 1.1f;
            }
            finish(c[bsp::kPilotCmdYaw], v);
        }
        if (free_flight) {                                        // cmd[2], the bot+190h set
            float v = c[bsp::kPilotCmdRoll];
            const float before = v;
            if (!ga_band_search_0099b940(u, 2, v)) v = before < 0.0f ? -1.0f : 1.0f;  // 0099C129
            if (c[bsp::kPilotCmdRoll] != v) {
                c[bsp::kPilotCmdRoll] = v;
                ++u.ga_repairs;
            }
        }
        // 0099C130-0099C1FF: the pilot+25Ch throttle arm (bot+258h, bot =
        // pilot+4h). Below 1.0: when not positive, throttle 0.01 ([00D7A238])
        // and air brake max(cmd[4], -limit); otherwise throttle min(cmd[3],
        // limit). Inert at 1.0, which is all the gunfire arm leaves.
        if (u.av_throttle_25c < 1.0f) {
            const float lim = u.av_throttle_25c;
            if (lim <= 0.0f) {
                c[bsp::kPilotCmdThrottle] = 0.01f;
                if (c[bsp::kPilotCmdAirBrake] <= -lim) c[bsp::kPilotCmdAirBrake] = -lim;
            } else if (!(c[bsp::kPilotCmdThrottle] < lim)) {
                c[bsp::kPilotCmdThrottle] = lim;
            }
        }
        // 007D7A40 after a changed pitch or yaw: unread, not modelled.
    }

    // Packet cc9_moveto_target_speed (docs/PILOT_MOVETO_TASK.md, "The target-speed
    // override"; GAMEPLAY_GAP_RANKING #8). True: the kind-7 moveto state and its
    // circle (009C24E0, 009C26F2) take 009BECD0's first argument from 009C23B0,
    // which answers the target's speed instead of squadron+3A0h when the target
    // is within TurnCircleRadius + approach+68h (50.0) and faster than 007C47F0.
    // False: squadron+3A0h always (a record).
    static constexpr bool kMoveToTargetSpeedOverrideBound = true;  // ON: five rows identical (PILOT_MOVETO_TASK)
// Packet cc13-cadence3 (docs/CC13_TASK_PROFILE_CADENCE.md section 10). The
// moveto task's activation (0099AEB3 -> vt[54h] 009C3650) draws approach+68h =
// U(FollowDist/1, FollowDist/2) at 009C367C and approach+2Ch = U(0,
// TravelAltRandom) at 009C36A3, both stream 1, then refreshes (009C36BF).
// True: the install makes both draws on "#mt" keys after the construction
// draws and refreshes again. False: +68h stays 50.0 and +2Ch keeps its
// construction draw U(0, MaxAltOffset).
static constexpr bool kMoveToActivationDrawsBound = true;  // GUNNERY 193: ON by pair verdict
    unsigned long long ts_override_calls{0}, ts_override_target{0};
    unsigned long long ts_override_within{0}, ts_override_applied{0};
    float ts_override_max_speed{0.0f};
    // kMovetoTargetSpeedClassCc13Bound census: calls with a target, calls whose class speed
    // differs from |linear_velocity| by > 1e-3 (or has no live leader), and calls whose
    // returned speed differs between the two arms.
    unsigned long long tsc_calls{0}, tsc_speed_differs{0}, tsc_answer_differs{0};

    // 009C23B0 (009C23B0-009C2429, __thiscall(approach), RET, ST0). a =
    // [approach+0Ch]+3A0h (squadron+3A0h, `base`); with a target (+44h) and a
    // class (+8h): when approach+64h < class+268h TurnCircleRadius + approach+68h
    // (009C23D5-009C23F8; +68h = 50.0 from 009C1C30, 00CEB4D4) and the target's
    // vtable[38h] speed > 007C47F0 (009C23FA-009C2412), a = that speed.
    // LABELLED: the target's vtable[38h] is its motion.linear_velocity length, as
    // the 009D3D01 site takes it; approach+64h is the host's moveto_distance_64.
    // kMovetoTargetSpeedClassCc13Bound (OFF) dispatches the slot by class instead.
    // Switch pairing: this switch alone decides this site. ON, its plane and
    // squadron-leader arms take bsp::plane_cached_speed_b1c_007d8020 over
    // plane_world_velocity unconditionally (the image's vt[38h] is 007B8E60 for every
    // plane), whatever kPlaneCachedSpeedB1cCc13Bound says; that switch governs only its own
    // listed readers. So the pairs are: this switch ON vs OFF (all arms move together), and
    // kPlaneCachedSpeedB1cCc13Bound ON vs OFF separately; the two need no joint pair.
    static constexpr bool kMovetoTargetSpeedClassCc13Bound = true;
    float moveto_target_speed_009c23b0(const GameUnitSlot& unit, float base) {
        ++ts_override_calls;
        if (unit.moveto_entity_44 == nullptr) return base;
        ++ts_override_target;
        const std::size_t target = [&]() -> std::size_t {
            // 009C23B0 reads approach+44h, which 009C1C30 produced from
            // the entity target's slot0 (009BE140), once at construction.
            return unit.moveto_entity_44->process_index;
        }();
        const GameUnitSlot& t = *slots[target];
        // The OFF expression, moved verbatim into this lambda: the same arithmetic as before
        // (|motion.linear_velocity| in double, rounded once), not textually identical.
        const auto linear_speed = [](const GameUnitSlot& s) {
            const bsp::OceanVec3& v = s.motion.linear_velocity;
            return static_cast<float>(std::sqrt(static_cast<double>(v.x) * v.x
                + static_cast<double>(v.y) * v.y + static_cast<double>(v.z) * v.z));
        };
        // The ON speed, evaluated on both arms so that the OFF arm can count where it
        // differs (census for the flip, cc9-physics29); `on_base` marks the
        // no-live-leader arm, which answers `base` on the ON arm.
        bool on_base = false;
        const auto class_speed = [&]() -> float {
            // 009C23DB-009C23E9: MOV EDX,[ECX]; CALL [EDX+38h] on the target, made before the
            // reach test, FSTP float. Slot 38h by class, first match, as
            // run_get_entity_speed_008c0820 dispatches it:
            //   PlaneSquadronGen 007ED470: JMP the leader's ([squadron+3D0h]) vtable[38h];
            //   ships 0080E0F0: ECX = [unit+1018h], JMP 0092D730, the signed body forward
            //   speed over the inputs GameUnitsHost::unit_forward_speed_0092d730 reads;
            //   planes 007B8E60: FLD [unit+B1Ch].
            // The plane arm and the squadron-leader arm read +B1Ch as
            // bsp::plane_cached_speed_b1c_007d8020 over plane_world_velocity (007D8020's
            // per-op chain with its 1e-10 floor; LABELLED there: a recompute, not the cache).
            // LABELLED: other classes' slot-38h bodies were not read and keep
            // |motion.linear_velocity|; a squadron without a live leader faults in the image
            // and answers `base` here.
            const auto plane_speed = [](const GameUnitSlot& s) {
                const float v[3] = {s.plane_world_velocity[0], s.plane_world_velocity[1],
                                    s.plane_world_velocity[2]};
                return bsp::plane_cached_speed_b1c_007d8020(v);   // 007B8E60 FLD [unit+B1Ch]
            };
            // 0092D74E-0092D763: FMUL, FMUL, FADDP, FMUL, FADDP, each to 24 bits, then FSTP
            // float at 0092D765, in the order of the kSubmarineSeabedSpeedPc24Cc13Bound arm.
            // Of the host's copies of this speed, this arm and that one are the PC24-faithful
            // ones; bsp::unit_forward_speed_0092d730 (unit_motion.cpp) and
            // GameUnitsHost::unit_forward_speed_0092d730 sum in double and round once, which
            // stays the OFF behaviour at their other callers.
            const auto ship_speed = [](const GameUnitSlot& s) {
                const float vy_ry = s.motion.linear_velocity.y * s.motion.pose_row2[1];
                const float vx_rx = s.motion.linear_velocity.x * s.motion.pose_row2[0];
                const float sum_yx = vy_ry + vx_rx;
                const float vz_rz = s.motion.linear_velocity.z * s.motion.pose_row2[2];
                return sum_yx + vz_rz;
            };
            const bsp::PlaneSquadronHostRecord* squadron = nullptr;
            for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
                if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == target) {
                    squadron = &r;
                    break;
                }
            }
            if (squadron != nullptr) {
                const std::size_t leader = squadron->flight_leader();   // 007ED470 +3D0h
                if (leader == bsp::kPlaneSquadronNoUnit || leader >= slots.size() || !slots[leader]) {
                    on_base = true;
                    return base;
                }
                return plane_speed(*slots[leader]);                     // leader's 007B8E60
            }
            if (bsp::unit_is_kind_of(t.class_id, bsp::kUnitKindQueryShip)) {
                return ship_speed(t);                                   // 0080E0F0 -> 0092D730
            }
            if (bsp::unit_is_kind_of(t.class_id, bsp::kUnitKindQueryPlane)) {
                return plane_speed(t);                                  // 007B8E60
            }
            return linear_speed(t);                                     // LABELLED, unread class
        };
        const float on_speed = class_speed();
        const float off_speed = linear_speed(t);
        const float reach = unit.plane_turn_circle_radius + unit.moveto_follow_dist_68;  // 009C23E3 FSTP
        const bool within = reach > unit.moveto_distance_64;
        const double desired = static_cast<double>(bot_desired_speed_007c47f0(unit));  // pure
        const auto answer = [&](float s) {
            return within && static_cast<double>(s) > desired ? s : base;
        };
        ++tsc_calls;
        if (on_base || std::fabs(on_speed - off_speed) > 1e-3f) ++tsc_speed_differs;
        const float on_answer = on_base ? base : answer(on_speed);
        if (on_answer != answer(off_speed)) ++tsc_answer_differs;
        if (kMovetoTargetSpeedClassCc13Bound && on_base) return base;
        const float speed = kMovetoTargetSpeedClassCc13Bound ? on_speed : off_speed;
        if (!within) return base;                                            // 009C23F8 JBE
        ++ts_override_within;
        if (!(static_cast<double>(speed) > desired)) return base;           // 009C2412
        ++ts_override_applied;
        if (speed > ts_override_max_speed) ts_override_max_speed = speed;
        return speed;
    }

    // 009C1850 BSP_BotStateMoveTo_SetDesiredSpeed's speed for a torpedo or
    // dive-bomb moveto: 009BECD0(squadron+3A0h, 007C47F0(), sep). With
    // `via_009c23b0` (the kind-7 moveto and circle states) the first argument
    // is 009C23B0's.
    float moveto_speed_009c1850(const GameUnitSlot& unit, float sep,
                                bool via_009c23b0 = false) {
        float w1 = 3000.0f, w2 = 5000.0f;
        float dont_wait = 0.87266463f, wait = 1.7453293f, good = 100.0f, nearby = 200.0f;
        float travel_mul = 1.6f;
        if (lua.plane_globals_loaded()) {
            const bsp::GameTuningBlock& g = lua.plane_globals();
            w1 = g.pilot_general_wingmen_wait_dist_1;
            w2 = g.pilot_general_wingmen_wait_dist_2;
            dont_wait = g.pilot_follow_dont_wait_for_hdg_diff;
            wait = g.pilot_follow_wait_for_hdg_diff;
            good = g.pilot_follow_good_position_dist;
            nearby = g.pilot_follow_nearby_dist;
            travel_mul = g.dynamics_spd_multipliers_new_travel_speed_mul;
        }
        // squadron+3A0h = [squadron+35Ch]+190h, the class's TravelSpeed x
        // tuning+334h NewTravelSpeedMul (007D23F5/007D2406). The squadron's
        // class is its members' class, so the unit's own row stands in.
        float travel_scaled = unit.plane_travel_speed * travel_mul;
        travel_scaled = squadron_speed_3a0_live(unit.process_index, travel_scaled);
        if constexpr (kMoveToTargetSpeedOverrideBound) {
            if (via_009c23b0) travel_scaled = moveto_target_speed_009c23b0(unit, travel_scaled);
        }
        const bsp::PlaneSquadronHostRecord* sq =
            bsp::plane_squadron_registry().find_by_member_unit(unit.process_index);
        float wingmen = 1.0f;
        if (sq != nullptr) {
            std::vector<float> values;
            for (const std::size_t m : sq->member_units) {
                float v = -1.0f;   // 007BCC20: dead, leader, no pilot
                if (m != bsp::kPlaneSquadronNoUnit && m < slots.size() &&
                    m != unit.process_index) {
                    const GameUnitSlot& o = *slots[m];
                    const bool following = member_follows_007bcc20(o);
                    if (following) {
                        const float d[3] = {o.motion.position[0] - o.fw_station[0],
                                            o.motion.position[1] - o.fw_station[1],
                                            o.motion.position[2] - o.fw_station[2]};
                        v = bsp::follow_wait_value_009be3e0(o.fw_arm == 1, d,
                            o.fw_leader_heading, dont_wait, wait, good, nearby);
                    }
                }
                values.push_back(v);
            }
            wingmen = bsp::squadron_wingmen_value_007ef2c0(
                values.data(), static_cast<int>(values.size()), sq->formation_shape_3e4);
        }
        ++moveto_blend_calls_;
        last_blend_a_ = travel_scaled;
        last_blend_b_ = bot_desired_speed_007c47f0(unit);
        last_blend_wingmen_ = sq != nullptr ? wingmen : 1.0f;
        last_blend_near_ = sq != nullptr && sep < w1;
        last_blend_w1_ = w1;
        last_blend_w2_ = w2;
        last_blend_has_sq_ = sq != nullptr;
        return bsp::dogfight_moveto_speed_009becd0(travel_scaled,
            bot_desired_speed_007c47f0(unit), sep, w1, w2, sq != nullptr, wingmen);
    }
    // 007BCC20's non-negative case: a member whose first task answers vtable[4Ch]
    // with 009BE3E0 (a follow or prepare state), so it publishes a wait value in
    // [0, 1] at unit+9C4h; every other member publishes -1.0.
    bool member_follows_007bcc20(const GameUnitSlot& o) const {
                    // 00999AE0: the first task answering vtable[4Ch]. The
                    // dive-bomb task's is 009C8260 and the torpedo task's
                    // 009D4970 (vtables 00D20E18 / 00D213C8, slot +4Ch; both
                    // +34h are 0099B700 XOR AL,AL): when task+310h is the
                    // follow state (+52Ch / +580h) or the prepare state
                    // (+5C4h / +740h) they tail-jump to 009BE3E0 on it, else
                    // to 0099B720, which returns [00D7A260] = -1.0f. The
                    // follow-state values come from the follow law's last
                    // tick (docs/PLANE_FOLLOW_LAW.md section 16).
                    const bool answers =
                        o.torpedo_state == bsp::TorpedoState::kFollow ||
                        o.torpedo_state == bsp::TorpedoState::kPrepare ||
                        o.dive_bomb_state == bsp::DiveBombState::kFollow ||
                        o.dive_bomb_state == bsp::DiveBombState::kPrepare ||
                        // Packet cc9_land_task_reach: the land task's +4Ch is
                        // 009B3750, 009BE3E0 on the follow (land) state +500h.
                        (kSquadronLandTaskBound && o.land_task_installed &&
                         o.land_state == GameUnitSlot::LandTaskState::kFollow) ||
                        // Packet cc9_strafe_arm: the strafe task's +4Ch is 009CC8D0,
                        // 009BE3E0 on the follow state +51Ch only (not prepare).
                        (bsp::kStrafeTaskBound && o.strafe_task_installed &&
                         o.strafe_state == bsp::StrafeState::kFollow) ||
                        // Packet cc9-rocket1: the rocket task's +4Ch 007B7270 takes
                        // 009BE3E0 on the follow state +520h only.
                        (bsp::kRocketTaskBound && o.rocket_task_installed &&
                         o.rk.state == bsp::RocketState::kFollow) ||
                        // Packet cc9_plane_level_bomb_task: 009B8420 answers
                        // 009BE3E0 on follow +534h and prepare +610h.
                        (bsp::kPlaneLevelBombTaskBound && o.level_bomb_task_installed &&
                         (o.level_bomb_state == bsp::LevelBombState::kFollow ||
                          o.level_bomb_state == bsp::LevelBombState::kPrepare));
                    return answers && o.fw_step + 2 >= summary.motion_steps;
    }
    // Packet cc9_leader_turn_roll (docs/CC13_AUDIT_PLANNER_ROLL.md finding 3):
    // 007B95E0 is true for the flight leader (+9D8h == 0) of a squadron whose
    // +3BCh > 0. +3BCh is 007EE790's recount: members 1..count-1 whose +9C4h
    // (007BCC20) is non-negative. SUBSTITUTION, labelled: the image recounts when
    // 007EE790's countdown expires and reads the previous buffer index; this host
    // counts at the call.
    // CORRECTION (cc13-tranche2 S19): under bsp::kWingmenRecountCountdownCc13Bound the
    // count is 007EE790's, held per squadron and retaken on the 007F3C3F countdown
    // (squadron_wingmen_recount_007f3c3f, include/bsp/wingmen_recount_countdown_cc13.hpp).
    int count_following_wingmen_007ee790(const bsp::PlaneSquadronHostRecord& sq) const {
        int active = 0;
        for (std::size_t i = 1; i < sq.member_units.size(); ++i) {
            const std::size_t m = sq.member_units[i];
            if (m == bsp::kPlaneSquadronNoUnit || m >= slots.size() || !slots[m]) continue;
            if (member_follows_007bcc20(*slots[m])) ++active;
        }
        return active;
    }
    // Goal 5 chain 628: the predicate is the reconstructed 007B95E0 body
    // (src/band_007a_cc13.cpp; CMP [ECX+9D8h],0 / JNE, TEST squadron / JE,
    // CMP [EAX+3BCh],0 / JLE). Its three inputs come from this host's squadron
    // record: +9D4h is the record found for the unit; +9D8h is the unit's live
    // seat among the occupied members, so the first occupied seat,
    // flight_leader(), is index 0; +3BCh is the 007EE790 recount described above.
    // Evidence for +9D8h: bsp.py show 007ed260 --asm, 007ED290..007ED2B4 walks
    // +3D0h for +3CCh members with 007ED292 MOV [ESI+9D8h],EAX, EAX the walk
    // index; 007F4BFA runs it in the spawn tail and 007ED610 after promoting the
    // leader to slot 0. So PlaneSquadronHostRecord::member_spawn_index (the
    // 007F4B43 stamp, which this host never rewrites) is not +9D8h once 007ED260
    // has run. 007F3970 calls 007ED260 at 007F3A11 immediately after compaction,
    // so 007ED292 publishes the new live seats before removal returns.
    bool plane_is_leader_with_wingmen_007b95e0(const GameUnitSlot& unit) const {
        const bsp::PlaneSquadronRegistry& registry = bsp::plane_squadron_registry();
        const bsp::PlaneSquadronHostRecord* sq =
            registry.find_by_member_unit(unit.process_index);
        std::int32_t seat_9d8 = -1;      // 007CFE72's constructor stamp: no squadron
        std::int32_t wingmen_3bc = 0;
        if (sq != nullptr) {
            std::int32_t live = 0;
            for (const std::size_t m : sq->member_units) {
                if (m == bsp::kPlaneSquadronNoUnit) continue;
                if (m == unit.process_index) {
                    seat_9d8 = live;
                    break;
                }
                ++live;
            }
            if constexpr (bsp::kWingmenRecountCountdownCc13Bound) {
                const auto it = wingmen_recount_.find(sq->name);
                wingmen_3bc = it != wingmen_recount_.end() ? it->second.count_3bc : 0;  // +3BCh
            } else {
                wingmen_3bc = count_following_wingmen_007ee790(*sq);
            }
        }
        return bsp::band_007a_cc13::plane_is_leader_with_wingmen_007b95e0(
            seat_9d8, sq != nullptr, wingmen_3bc);
    }
    // squadron+3C4h / +3BCh, packet cc13-tranche2 S19.
    // CORRECTION (rev4 FIX 1): +3C4h is not seeded -1.0. 007F2D30..007F2D6A store
    // -U(0, 1) from 00BD2F10 on stream 1 (FCHS at 007F2D60); the default below is only
    // the value before that draw is taken (squadron_wingmen_recount_seed).
    struct WingmenRecount {
        float countdown_3c4 = bsp::kSquadronRecountCountdownSeed;
        int count_3bc = 0;
    };
    std::map<std::string, WingmenRecount> wingmen_recount_;
    // 007F2D30..007F2D6A at the squadron's construction: +3C4h = -U(0, 1), stream 1.
    // STREAM-ORDER MOVER, LABELLED: the host draws when it first sees the record, at the
    // start of the next units' fixed step (the registry has four creators in other
    // hosts), not at the constructor's own place in the stream-1 sequence. The index is
    // the flight leader's (0 with none); it keys the stream only under the measurement
    // option. With no gunnery host the midpoint -0.5 stands in.
    void squadron_wingmen_recount_seed() {
        for (const bsp::PlaneSquadronHostRecord& sq : bsp::plane_squadron_registry().records()) {
            if (wingmen_recount_.count(sq.name) != 0) continue;
            WingmenRecount& r = wingmen_recount_[sq.name];
            const std::size_t lead = sq.flight_leader();
            const std::size_t key = lead != bsp::kPlaneSquadronNoUnit ? lead : 0u;
            r.countdown_3c4 = gunnery != nullptr
                ? -gunnery->death_mode_draw_00bd2f10(1, key, 0.0f, 1.0f)   // 007F2D5B, FCHS
                : -0.5f;
        }
    }
    // 007F3C3F..007F3C5B, once per fixed step for every squadron. Each float op is
    // stored as a float, which is the image's per-op PC24 result.
    void squadron_wingmen_recount_007f3c3f(float step) {
        for (const bsp::PlaneSquadronHostRecord& sq : bsp::plane_squadron_registry().records()) {
            const auto it = wingmen_recount_.find(sq.name);
            if (it == wingmen_recount_.end()) continue;   // created this step: next step
            WingmenRecount& r = it->second;
            // 007F3C3F FCOMI step, t / JC: below or unordered (a NaN) takes t -= step.
            if (!(step >= r.countdown_3c4)) {                              // 007F3C41 JC
                r.countdown_3c4 -= step;                                   // 007F3D24
            } else {
                const float reload = bsp::kSquadronRecountPeriod3c0 - step; // 007F3C47
                r.countdown_3c4 = reload + r.countdown_3c4;                // 007F3C53
                r.count_3bc = count_following_wingmen_007ee790(sq);        // 007F3C5B
            }
        }
    }
    int moveto_blend_calls_{0};
    int leader_turn_roll_uses{0};
    // DIAGNOSTIC, packet cc9_land_holding_speed: the last 009C1850 call's
    // a (squadron+3A0h stand-in), b (007C47F0), 007EF2C0 and sep < WaitDist1.
    float last_blend_a_{0.0f}, last_blend_b_{0.0f}, last_blend_wingmen_{1.0f};
    bool last_blend_near_{false};
    // DIAGNOSTIC, packet cc13-mvspeed (ON only): the last call's wait
    // distances and squadron test, and the torpedo sites' old-versus-new sep.
    float last_blend_w1_{3000.0f}, last_blend_w2_{5000.0f};
    bool last_blend_has_sq_{false};
    unsigned long long mvs_n{0}, mvs_speed_changed{0}, mvs_no_range{0};
    double mvs_dsep_sum{0.0}, mvs_dsep_max{0.0}, mvs_dspeed_max{0.0};
    unsigned long long lhs_n{0}, lhs_floor{0}, lhs_wait{0}, lhs_near{0};
    unsigned long long lhs_slow{0}, lhs_slow_floor{0}, lhs_sep300{0};
    double lhs_a_sum{0.0}, lhs_b_sum{0.0}, lhs_want_sum{0.0}, lhs_wingmen_sum{0.0};
    double lhs_v_sum{0.0}, lhs_sep_sum{0.0};

    // 009BEE30's hold arm, 009BEE56-009BF9E5, through
    // bsp::plane_follow_hold_command_009bee56 (docs/PLANE_FOLLOW_HOLD_ARM.md).
    void run_follow_hold_arm_009bee56(GameUnitSlot& unit,
                                      const bsp::PlaneFormationStation& station,
                                      const GameUnitSlot& leader,
                                      const bsp::GameTuningBlock& gt) {
        ++follow_hold_ticks_;
        bsp::PlaneFollowHoldInputs in;
        in.own = follow_hold_attitude(unit);
        in.leader = follow_hold_attitude(leader);
        in.leader_present = true;
        // leader+9C2h[...], the published +520h: clear for an AI leader
        // (docs/BOMBER_AFTER_TASK.md 6c, provisional), so the lock never runs.
        in.leader_published_520 = false;
        // S for 009BFEE0's latched path (009BFF3B): the station as 009BFD70
        // wrote it this tick (009C1FEA), before the tail clamps it.
        float station_local_pre_band[3];
        follow_to_body(unit, station.world, false, station_local_pre_band);
        bsp::PlaneFollowBand band{};
        if constexpr (kFollowBandSafeAltFloorBound) {
            // 009C16D2-009C17B9, the band of 009BFEE0's tail (009C2068), with
            // state+88h = SafeAlt (009BEDBB).  009C17C6-009C17F1 clamps the
            // station Y (state+34h) into it before 009BEE30 (009C2077) reads
            // the station at 009BEE92, on the latched and hold paths alike.
            band = bsp::plane_follow_band_009c16d2(
                leader.motion.position[1], gt.pilot_follow_leader_follow_alt,
                unit.fs_parameters.safe_alt(), gt.dynamics_ceiling, true, false);
            const float station_banded[3] = {
                station.world[0],
                bsp::plane_follow_band_clamp_009c17c6(station.world[1], band),
                station.world[2]};
            follow_to_body(unit, station_banded, false, in.station_local);   // 009BEE92
        } else {
            for (int i = 0; i < 3; ++i) in.station_local[i] = station_local_pre_band[i];   // 009BEE92
        }
        follow_to_body(unit, leader.motion.pose_row2, true,
                       in.leader_forward_local);                        // 009BEEED
        // SUBSTITUTION, labelled: state+84h/+78h come from 009C1FD0's search
        // loop 009C216D-009C22F5, read only in part; the sight stays inactive.
        in.sight_active_84 = false;
        if constexpr (kFollowSightSearchBound) {
            // 009BF0FD: +84h. 009BF13D / 009BF177: +78h..+80h through the
            // leader's pose, then the member's inverse. SUBSTITUTION, labelled:
            // the pose product is taken in double, as follow_to_body does.
            in.sight_active_84 = unit.fs_sight_84;
            if (in.sight_active_84) {
                const float* m = leader.world.data();
                const float* q = unit.fs_sight_78;
                float w[3];
                for (int i = 0; i < 3; ++i) {
                    if constexpr (kPc24PlaneTasksCc13Bound) {
                        // pc24w1-sweep3-5: 004142E0's order, each op rounded at PC24:
                        // ((x * m[i] + y * m[4 + i]) + z * m[8 + i]) + m[12 + i]
                        // (00414304..00414328 for row 0; 0041432A..0041433D row 1).
                        namespace pc24 = bsp::cc13x87;
                        w[i] = static_cast<float>(pc24::x87_add24(
                            pc24::x87_add24(pc24::x87_add24(pc24::x87_mul24(q[0], m[i]),
                                                            pc24::x87_mul24(q[1], m[4 + i])),
                                            pc24::x87_mul24(q[2], m[8 + i])),
                            m[12 + i]));
                    } else {
                    w[i] = static_cast<float>(
                        static_cast<double>(q[0]) * m[i] + static_cast<double>(q[1]) * m[4 + i] +
                        static_cast<double>(q[2]) * m[8 + i] + m[12 + i]);
                    }
                }
                follow_to_body(unit, w, false, in.sight_local);
            }
        }
        if constexpr (kPlaneFollowCatchupBound) {
            // 009BFEFC-009C0021, 009BFEE0's latched path: S = the station in
            // the member's frame (009BFF3B 004142E0), Lf = the leader's forward
            // row in the member's frame (009BFF97 0042D0D0, normalize 0),
            // k = (block+00h FollowedPointDist - S.z) / Lf.z (009BFF9C-009BFFAB),
            // state+44h = station + k * the leader's world forward
            // (009BFFAF-009C001E), then JMP 009C16D2, the band tail.
            const float lfz = in.leader_forward_local[2];
            float steer[3];
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // pc24w1-sweep3-5: k = (dist - S.z) / Lf.z, 009BFFA1 FSUB, 009BFFA8 FDIV,
                // stored through float (009BFFAB); each k * forward product is stored
                // (009BFFE5..009BFFF9) and added to the station (009C0004..009C001B),
                // each op rounded at PC24.
                namespace pc24 = bsp::cc13x87;
                const float k = lfz != 0.0f
                    ? static_cast<float>(pc24::x87_div24(
                          pc24::x87_sub24(gt.pilot_follow_followed_point_dist,
                                          station_local_pre_band[2]),
                          lfz))
                    : 0.0f;
                for (int i = 0; i < 3; ++i) {
                    const float step = static_cast<float>(pc24::x87_mul24(k, leader.motion.pose_row2[i]));
                    steer[i] = static_cast<float>(pc24::x87_add24(step, station.world[i]));
                }
            } else {
            const double k = lfz != 0.0f
                ? (static_cast<double>(gt.pilot_follow_followed_point_dist) -
                   station_local_pre_band[2]) / lfz
                : 0.0;
            for (int i = 0; i < 3; ++i) {
                steer[i] = static_cast<float>(static_cast<double>(station.world[i]) +
                                              k * leader.motion.pose_row2[i]);
            }
            }
            // 009C16D2-009C1846 on the steer Y, the arithmetic of
            // plane_follow_geometry_009bfee0's tail (state+88h taken as huge,
            // as at the fly-to call).
            if constexpr (kFollowBandSafeAltFloorBound) {
                // 009C17F6-009C183B on the steer Y (state+48h): L > Y -> L
                // without testing the cap, else Y > cap -> cap, else Y.
                steer[1] = bsp::plane_follow_band_clamp_009c17c6(steer[1], band);
            } else {
                const float floor_v = leader.motion.position[1] + gt.pilot_follow_leader_follow_alt;
                const float ceil_c = static_cast<float>(
                    static_cast<double>(leader.motion.position[1]) + 120.0);
                const float ceil_v = gt.dynamics_ceiling < ceil_c ? gt.dynamics_ceiling : ceil_c;
                if (steer[1] < floor_v) steer[1] = floor_v;
                if (steer[1] > ceil_v) steer[1] = ceil_v;
            }
            follow_to_body(unit, steer, false, in.steer_local);   // 009BF57F
        } else {
        // SUBSTITUTION, labelled: state+44h in the hold arm is 009BFEE0's own
        // hold-geometry point (009BFEEE's latched path), unread. The fly-to
        // geometry's steer point stands in; the blend reads it only while
        // |leader bank| > 0.5 (009BF531).
            bsp::PlaneFollowGeometryInputs gin;
            for (int i = 0; i < 3; ++i) {
                gin.own_pos[i] = unit.motion.position[i];
                gin.station[i] = station.world[i];
                gin.leader_forward[i] = leader.motion.pose_row2[i];
                gin.leader_pos[i] = leader.motion.position[i];
            }
            gin.own_heading = unit.plane_heading_c6c;
            gin.leader_heading = leader.plane_heading_c6c;
            if constexpr (kFollowLeaderTurnRateBound) {
                gin.leader_turn_rate = leader_turn_rate_007d7da0(leader);
            }
            gin.followed_point_dist = gt.pilot_follow_followed_point_dist;
            gin.leader_heading_time_1 = gt.pilot_follow_leader_heading_spd_time_1;
            gin.leader_heading_time_2 = gt.pilot_follow_leader_heading_spd_time_2;
            gin.leader_heading_dist_1 = gt.pilot_follow_leader_heading_spd_dist_1;
            gin.leader_heading_dist_2 = gt.pilot_follow_leader_heading_spd_dist_2;
            gin.band_floor_offset = gt.pilot_follow_leader_follow_alt;
            gin.band_ceiling_210 = gt.dynamics_ceiling;
            gin.state_88 = 1.0e30f;
            if constexpr (kFollowBandSafeAltFloorBound) {
                gin.state_88 = unit.fs_parameters.safe_alt();   // 009BEDBB
                gin.band_floor_image = true;
            }
            gin.band_inputs_available = true;
            const bsp::PlaneFollowGeometry geo = bsp::plane_follow_geometry_009bfee0(
                gin, bsp::PlaneFollowRegime::kLeadPursuit);
            follow_to_body(unit, geo.steer_point, false, in.steer_local);   // 009BF57F
        }
        // 007C47F0 on [approach+8], 009BF90D/009BF92E.
        in.level_flight_speed = bot_desired_speed_007c47f0(unit);
        in.gains.yf_hdg_rad = gt.pilot_follow_yf_hdg_rad;
        in.gains.yf_yaw_v_rad_per_sec = gt.pilot_follow_yf_yaw_v_rad_per_sec;
        in.gains.yf_sidepos_meter = gt.pilot_follow_yf_sidepos_meter;
        in.gains.yf_sidedir = gt.pilot_follow_yf_sidedir;
        in.gains.pf_pitch_rad = gt.pilot_follow_pf_pitch_rad;
        in.gains.pf_pitch_v_rad_per_sec = gt.pilot_follow_pf_pitch_v_rad_per_sec;
        in.gains.pf_vertpos_meter = gt.pilot_follow_pf_vertpos_meter;
        in.gains.pf_vertdir = gt.pilot_follow_pf_vertdir;
        in.gains.rf_roll_rad = gt.pilot_follow_rf_roll_rad;
        in.gains.rf_roll_v_rad_per_sec = gt.pilot_follow_rf_roll_v_rad_per_sec;
        in.gains.rf_hdg_rad = gt.pilot_follow_rf_hdg_rad;
        in.gains.rf_hdg_v_rad_per_sec = gt.pilot_follow_rf_hdg_v_rad_per_sec;
        in.gains.pwr_back_meter = gt.pilot_follow_pwr_back_meter;
        in.gains.pwr_spd_meter_per_sec = gt.pilot_follow_pwr_spd_meter_per_sec;
        const bsp::PlaneFollowHoldCommand c = bsp::plane_follow_hold_command_009bee56(in);
        done("BotStateFollow::hold_arm", 0x009bee56u);
        if (c.locked) return;   // 009BF0B8-009BF0E8, never for an AI leader
        if constexpr (kFollowSightSearchBound) {
            if (c.clears_sight_84) unit.fs_sight_84 = false;   // 009BF295
        }
        // 009BF6F4-009BF70B: pitch desired, slot 3 active, plan+2D0h = 0.
        unit.plan_slots[bsp::kPilotSlotPitch].desired = c.pitch_29c;
        unit.plan_slots[bsp::kPilotSlotPitch].active = 1;
        unit.plan_state.pitch_mode_2d0 = 0;
        // 009BF716-009BF730: yaw desired, slot 1 active, plan+2D4h = 0 (0099E756
        // then skips the planner's yaw arm).
        unit.plan_slots[bsp::kPilotSlotYaw].desired = c.yaw_284;
        unit.plan_slots[bsp::kPilotSlotYaw].active = 1;
        unit.gl_yaw_mode_2d4_zero = true;
        if (c.writes_roll_290) {
            // 009BF743-009BF752: roll desired, slot 2 active, plan+2CCh = 0.
            unit.plan_slots[bsp::kPilotSlotRoll].desired = c.roll_290;
            unit.plan_slots[bsp::kPilotSlotRoll].active = 1;
            unit.plan_heading_mode_2cc = 0;
            unit.plan_heading_2c0_written = false;
        } else if (c.writes_bank_target_2c4) {
            // 009BF783/009BF789: bank target, plan+2CCh = 1.
            unit.plan_state.bank_target_2c4 = c.bank_target_2c4;
            unit.plan_heading_mode_2cc = 1;
            unit.plan_heading_2c0_written = false;
        }
        // 009BF9AA-009BF9C8: throttle and air brake slots, plan+2D8h = 0.
        unit.plan_slots[bsp::kPilotSlotThrottle].desired = c.throttle_278;
        unit.plan_slots[bsp::kPilotSlotThrottle].active = 1;
        unit.plan_slots[bsp::kPilotSlotAirBrake].desired = c.air_brake_2a8;
        unit.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
        unit.plane_air_brake_mode_2d8 = 0;
    }

    // 004142E0 with unit+110h (the inverse of the world matrix): a world point
    // in the unit's body frame; rows 0/1/2 of the pose are right/up/forward.
    // With `direction` it is 0042D0D0(normalize = 0): the rotation only.
    // cc13-followbody: 00B63D50 per op at PC24, the inverse the image caches at
    // unit+110h from the pose at unit+CCh (u.world). Each row's squared length
    // n = (a*a + b*b) + c*c with every square stored float (00B63D7D..00B63E39);
    // column r of the 3x3 is row r / n (00B63E47 FDIVP, 00B63E50 FDIV, 00B63E58
    // FDIVR m32, each stored); translation r = ((-inv[r] * t0) - t1 * inv[4+r])
    // - t2 * inv[8+r] (00B63E9A..00B63F03, FSUBP DE E9 = ST1 - ST0); the last
    // column is 0, 0, 0, 1.
    static void follow_inverse_00b63d50(const float* s, float inv[16]) {
        namespace x8 = bsp::cc13x87;
        float n[3];
        for (int r = 0; r < 3; ++r) {
            const float* row = s + 4 * r;
            const float aa = static_cast<float>(x8::x87_mul24(row[0], row[0]));
            const float bb = static_cast<float>(x8::x87_mul24(row[1], row[1]));
            const float cc = static_cast<float>(x8::x87_mul24(row[2], row[2]));
            n[r] = static_cast<float>(x8::x87_add24(x8::x87_add24(aa, bb), cc));
        }
        for (int r = 0; r < 3; ++r) {
            inv[r] = static_cast<float>(x8::x87_div24(s[4 * r], n[r]));
            inv[4 + r] = static_cast<float>(x8::x87_div24(s[4 * r + 1], n[r]));
            inv[8 + r] = static_cast<float>(x8::x87_div24(s[4 * r + 2], n[r]));
        }
        for (int r = 0; r < 3; ++r) {
            inv[12 + r] = static_cast<float>(x8::x87_sub24(
                x8::x87_sub24(x8::x87_mul24(-inv[r], s[12]), x8::x87_mul24(s[13], inv[4 + r])),
                x8::x87_mul24(s[14], inv[8 + r])));
        }
        inv[3] = inv[7] = inv[11] = 0.0f;
        inv[15] = 1.0f;
    }
    // cc13-followbody: 004142E0 (point) / 0042D0D0 with flag 0 (direction)
    // by that inverse of the pose m, per op: out[j] = ((p0 * inv[j] + p1 *
    // inv[4+j]) + p2 * inv[8+j]) (+ inv[12+j] for the point), each output stored
    // float (00414328/00414340/0041435B; 0042D11A/0042D131/0042D14A).
    // The input is staged first (004142E3..004142FA), so out may alias p.
    // Shared by follow_to_body and the dogfight df_local (pc24w2-units).
    static void to_body_image_004142e0(const float* m, const float p[3], bool direction,
                                       float out[3]) {
        namespace x8 = bsp::cc13x87;
        float inv[16];
        follow_inverse_00b63d50(m, inv);
        const float q[3] = {p[0], p[1], p[2]};
        for (int j = 0; j < 3; ++j) {
            double sum = x8::x87_add24(
                x8::x87_add24(x8::x87_mul24(q[0], inv[j]), x8::x87_mul24(q[1], inv[4 + j])),
                x8::x87_mul24(q[2], inv[8 + j]));
            if (!direction) sum = x8::x87_add24(sum, inv[12 + j]);
            out[j] = static_cast<float>(sum);
        }
    }
    static void follow_to_body(const GameUnitSlot& u, const float p[3], bool direction,
                               float out[3]) {
        // bsp.py show 00414e10 --asm: +10Ch gates the retained +110h
        // output; the dirty path refreshes +CCh before 00B63D50. Borrow
        // the canonical unit pose and its cache, never a fresh pose or a
        // separately guessed inverse. The old path below also reads
        // u.world, so publication timing is unchanged.
        bsp::PoseDerivedView view{*u.pose, u.derived};
        const bsp::CameraMatrix& inverse = bsp::get_pose_derived_00414e10(view);
        if (direction) {
            // The follow law passes normalize=0 to 0042D0D0. Reuse its
            // existing native-order kernel on the same cached inverse.
            bsp::transform_native_effect_direction_0042d0d0_no_normalize(
                out, p, inverse.data());
        } else {
            std::array<float, 3> result{};
            bsp::transform_point_004142e0({p[0], p[1], p[2]}, inverse, result);
            for (std::size_t i = 0; i < result.size(); ++i) out[i] = result[i];
        }
    }
    static bsp::PlaneFollowHoldAttitude follow_hold_attitude(const GameUnitSlot& u) {
        bsp::PlaneFollowHoldAttitude a;
        a.pitch = u.plane_pitch_angle_c64;
        a.bank = u.plane_bank_angle_c68;
        a.heading = u.plane_heading_c6c;
        // unit+C70h, 007C18B0's heading rate (goal 5 chain 657).
        a.turn_c70 = u.plane_heading_rate_c70;
        // vtable+38h 007B8E60 BSP_Unit_GetCachedSpeed: the world speed.
        if constexpr (bsp::kPlaneCachedSpeedB1cCc13Bound) {
            // unit+B1Ch as 007D8020 caches it (per op, 1e-10 floor).
            a.speed = bsp::plane_cached_speed_b1c_007d8020(u.plane_world_velocity);
        } else {
        a.speed = static_cast<float>(std::sqrt(
            static_cast<double>(u.plane_world_velocity[0]) * u.plane_world_velocity[0] +
            static_cast<double>(u.plane_world_velocity[1]) * u.plane_world_velocity[1] +
            static_cast<double>(u.plane_world_velocity[2]) * u.plane_world_velocity[2]));
        }
        // SUBSTITUTION, labelled: ctl+A0h/A4h/A8h (unit+B50h..B58h), which the
        // gains pair with roll, pitch and yaw rate, are not carried; the body
        // angular velocity ctl+48h pitch / +4Ch yaw / +50h roll stands in. Of
        // the three only the pitch rate has a non-zero gain (pf_pitchV
        // 1/DEG(100)); yf_yawV and rf_rollV are authored 0.
        a.rate_a0 = u.plane_body_angular[2];
        a.rate_a4 = u.plane_body_angular[0];
        a.rate_a8 = u.plane_body_angular[1];
        return a;
    }
    int follow_hold_ticks_{0};
    int follow_flyto_ticks_{0};
    int follow_torpedo_follow_ticks_{0};
    // 009C1FD0 with kPlaneFollowLawBound: 009BFD70's station from the live
    // slot 0 (no placement), then 009BFEE0/009BEE30. False when the member has
    // no station (it is its squadron's leader, or alone).
    // Packet cc9_follow_trail_arm (docs/PILOT_MOVETO_TASK.md, "The follow trail
    // arm"; GAMEPLAY_GAP_RANKING #14). True: 009C1FD0's arm after 009BEE30 runs.
    // False: a record.
    // Packet cc9_plane_wanderer (docs/PLANE_WANDERER.md). True: the plane fixed step
    // runs 007BE060 at 007CE0D2 and the position takes 007D8230's offset term.
    // False: no wanderer (the offset stays zero).
    static constexpr bool kPlaneWandererBound = true;  // ON: mechanism held on five rows (PLANE_WANDERER 8)
    unsigned long long wanderer_planes{0}, wanderer_steps_total{0}, wanderer_active_total{0};
    unsigned long long wanderer_timer_draws{0}, wanderer_off_transitions{0};
    float wanderer_offset_max{0.0f}, wanderer_speed_max_seen{0.0f}, wanderer_accel_max_seen{0.0f};
    double wanderer_displacement_total{0.0};
    // Diagnostic, both sides: the follow law's station error (009BFD70's distance).
    double follow_station_err_sum{0.0};
    unsigned long long follow_station_err_n{0};
    float follow_station_err_max{0.0f};
    // Diagnostic, both sides (packet cc9_follow_error_split): the station
    // error split in the leader's frame (its horizontal forward, not the
    // lagged track), and the speeds, over every follow-law tick.
    double fes_along_sum{0.0}, fes_cross_abs_sum{0.0}, fes_dy_abs_sum{0.0};
    unsigned long long fes_n{0}, fes_behind{0}, fes_ahead{0}, fes_beside{0};
    double fes_speed_diff_sum{0.0};   // own |v| - leader |v|
    double fes_behind_speed_diff_sum{0.0};
    double fes_flyto_cmd_minus_own_sum{0.0};
    // ... and the throttle slots and speed ceilings behind the deficit.
    double fes_thr_member_sum{0.0}, fes_thr_leader_sum{0.0};
    double fes_member_v_sum{0.0}, fes_leader_v_sum{0.0};
    double fes_member_max_sum{0.0}, fes_leader_max_sum{0.0}, fes_leader_want_sum{0.0};
    unsigned long long fes_member_full{0}, fes_leader_full{0};
    unsigned long long fes_flyto_n{0};
    // Diagnostic, both sides: Phase A's regime per fly-to tick (applied with
    // kFollowPhaseABound, else the shadow of what it would pick).
    unsigned long long follow_phase_a_lead{0}, follow_phase_a_abeam{0}, follow_phase_a_circle{0};
    // Diagnostic, both sides (packet cc9_follow_cross_track): the leader-frame
    // error split by leader manoeuvre (0 straight, 1 turning: |007D7DA0| above
    // 0.05 rad/s) and by the follow branch (0 latched hold, 1 lead, 2 abeam,
    // 3 circle). cross_turn is cross signed + toward the turn's outside.
    double fct_along[2][4]{}, fct_cross_abs[2][4]{}, fct_cross_turn[2][4]{}, fct_err[2][4]{};
    unsigned long long fct_n[2][4]{}, fct_beside[2][4]{};
    // ... and by task: 0 no land task, 1 the member has the land task installed.
    double fct_task_err[2]{}, fct_task_cross_abs[2]{}, fct_task_along[2]{}, fct_task_speed[2]{};
    unsigned long long fct_task_n[2]{}, fct_task_slow[2]{};
    void fct_add(int turn, int branch, double along, double cross, double err, float rate) {
        fct_n[turn][branch] += 1;
        fct_along[turn][branch] += along;
        fct_cross_abs[turn][branch] += std::fabs(cross);
        fct_err[turn][branch] += err;
        // cross = d . (f.z, -f.x) is + to the leader's left of travel for a
        // y-up frame; a positive rate is recorded as is, so the sign is only
        // compared between buckets, not interpreted absolutely.
        fct_cross_turn[turn][branch] += rate >= 0.0f ? cross : -cross;
        if (std::fabs(cross) > 50.0 && std::fabs(cross) > std::fabs(along)) fct_beside[turn][branch] += 1;
    }
    double follow_phase_a_time_sum{0.0};
    static constexpr bool kFollowTrailArmBound = true;  // ON: stage-only, five rows identical (PILOT_MOVETO_TASK)
    // Packet p20_follow_release_arm (docs/OCEAN_WAVE_FIELD.md section 78). 009C1FD0's
    // release arm: after the station 009BFD70, a pending release (unit+C25h, 009C1FFD)
    // or a queued request (unit+C20h > 0, 009C2006) takes 009C200F-009C2063 and returns
    // BEFORE the follow law (009C2068 009BFEE0, 009C2077 009BEE30): cmd+2D0h = 2,
    // +2BCh = 0.0, +2C4h = 0.0, +2CCh = 1, +2B4h = [bot+8]+190h, +2B0h = 0, +2D8h = 1.
    // True: every 009C1FD0 seam takes the arm. False: the plain follow tick runs the
    // law anyway and the moveto follow returns without the stores, as before.
    static constexpr bool kFollowReleaseArmBound = true;   // ON: OCEAN_WAVE_FIELD 78 (USN01 pair)
    unsigned long long follow_release_pending_ticks{0}, follow_release_arm_applied{0};
    // 009C200F-009C2059. +2B4h is [[bot+4]+8]+190h, the class's second TravelSpeed
    // slot (TravelSpeed x NewTravelSpeedMul, 007D2406; docs/PLANE_CLASS_FIELDS.md).
    // LABELLED: bot+8 is taken to be the plane's own class descriptor.
    void follow_release_arm_009c200f(GameUnitSlot& unit) {
        ++follow_release_arm_applied;
        unit.plan_state.pitch_mode_2d0 = 2;                          // 009C2015
        unit.plan_state.pitch_target_2bc = 0.0f;                     // 009C201B
        unit.plan_state.bank_target_2c4 = 0.0f;                      // 009C2029
        unit.plan_heading_mode_2cc = 1;                              // 009C2036
        unit.plan_heading_2c0_written = false;
        unit.plane_desired_speed_2b4 = retreat_squadron_speed_3a0(unit);  // 009C204C
        unit.plane_trg_speed_corr_off_2b0 = 0;                       // 009C2052
        unit.plane_air_brake_mode_2d8 = 1;                           // 009C2059
        ++unit.plane_speed_commands;
    }
    // Packet cc13-follow (bsp::kFollowReleaseSeamsBound, OFF): the five host
    // renditions of 009C1FD0 that run 007F23A0 and the law directly. Called
    // after the station has been produced, as 009C1FFD/009C2006 follow
    // 009C1FEA; true means the arm ran and the caller skips the law.
    bool follow_release_seam_arm_009c1ffd(GameUnitSlot& unit) {
        if constexpr (bsp::kFollowReleaseSeamsBound) {
            if (unit.torpedo_release_pending_c25 || unit.torpedo_issue_requests_c20 > 0) {
                ++follow_release_pending_ticks;
                follow_release_arm_009c200f(unit);
                return true;
            }
        } else {
            (void)unit;
        }
        return false;
    }
    unsigned long long trail_arm_calls{0}, trail_arm_raised{0};
    unsigned long long trail_arm_enabled{0}, trail_arm_disabled{0};
    float trail_arm_min_mul{1.0f};

    // Packet cc13-fix89 U1: 009C211D-009C232A, the follow tick's tail after the
    // trail arm (docs/CC13_FIX89.md). __thiscall(state)(dt) inline in 009C1FD0.
    //  * 009C211D-009C213B / 009C22BA: +74h -= dt while dt is below it (JB, so an
    //    unordered compare decrements too); else +74h = (+70h - dt) + +74h, +70h
    //    = 0.6f (00CE3D30, 009C2A05), and the search runs.
    //  * 009C213E-009C2167: only with +85h (009BFD70's latch, this tick) and
    //    either +84h already set or 007B8EE0(squadron+3D0h), the leader's byte
    //    +C40h (in the firing list 00F87278 through 007C74A0 / 007C75A0).
    //  * 009C216D-009C22E8: +78h = +7Ch = 0; the walk of [008053C0(unit+54h)]
    //    +DE8h; each IsKindOf(5) entity's position (+FCh) in the leader's frame
    //    (414E10, 414D10); z > 1.0f (D7A24C, COMISS/JBE), z < gun+34h * 1.2
    //    (00CEC160 is the double of 1.2f), 0042B2F0 |(x/z, y/z, 0)| < gun+38h *
    //    1.5 (00CE3D78); the smallest such length wins and its z goes to +80h.
    //  * 009C22F5-009C230B: +84h = found (0 when +85h is clear or the gate
    //    fails); found -> unit+914h = 1.0f.
    //  * 009C2313-009C232A, every tick: gun+4Ch = +84h ? 2.0f (00CE3958) : 0.0f,
    //    gun = [task+1Ch], the 009FC7C0 controller (unit.df_gun here).
    // SUBSTITUTIONS, labelled: the recon contact list is every live slot of
    // another party (recon levels are not hosted; docs/AA_TARGETING.md uses the
    // same stand-in), the leader's +C40h is its gunFire byte +BC9h (the host's
    // stand-in for the firing list, gunfire_detect_0099ec40), gun+34h is
    // df_row.aim_shoot_distance and gun+38h the controller's 0.09 default, as
    // df_gun_tick_009fc7c0 has them. x87 chains are modelled at 24-bit precision
    // (float per operation); a 53-bit thread could differ in the last bit.
    // Not hosted: unit+914h's reader 0072C5FE (squadron-head substitution in the
    // gun code). 0072C128 is plane_shot_owner_squadron_head_0072c119 (cc13-s24). 009BED80's +84h clear: follow_sight_enter_009bed80.
    static constexpr float kFollowSightPeriod70 = 0.6f;   // 00CE3D30 = 3F19999Ah
    // 009BED80, the follow Enter: of the sight block it clears only +84h
    // (009BEDC3) and +85h (009BEE1C; the host recomputes +85h every tick).
    // +74h, +78h..+80h are untouched (009BED92's 009BE150 copies the
    // parameters into [state+6Ch], not into the state). SUBSTITUTION, labelled:
    // the host has no single enter for the follow states, so a follow tick
    // with no follow tick on the previous think counts as an entry; a re-entry
    // with no think between is missed, and a follow state whose tick skips a
    // think would be cleared where the image keeps +84h.
    void follow_sight_enter_009bed80(GameUnitSlot& unit) {
        retain_follow_parameters(unit);
        if (unit.fs_last_follow_think + 1 != unit.fs_thinks) {
            unit.fs_sight_84 = false;                                     // 009BEDC3
        }
        unit.fs_last_follow_think = unit.fs_thinks;
    }
    void retain_follow_parameters(GameUnitSlot& unit) {
        if (lua.plane_globals_loaded() &&
            unit.fs_parameters.observe_follow_think(lua.plane_globals(), unit.fs_thinks)) {
            done("BotStateFollow::copy_parameters", 0x009be150u);
        }
    }
    void follow_sight_tick_009c211d(GameUnitSlot& unit, const GameUnitSlot& leader,
                                    float dt, float& countdown_74) {
        const float c = countdown_74;
        if (!(dt >= c)) {
            countdown_74 = c - dt;                                        // 009C22BA
        } else {
            countdown_74 = (kFollowSightPeriod70 - dt) + c;               // 009C2134-009C213B
            bool found = false;                                           // 009C2137
            const bool in_position_85 =
                unit.fw_arm == 1 && unit.fw_step == summary.motion_steps;
            if (in_position_85 && (unit.fs_sight_84 || leader.plane_gun_fire_bc9)) {
                unit.fs_sight_78[0] = 0.0f;                               // 009C2178
                unit.fs_sight_78[1] = 0.0f;                               // 009C2173
                float best = 0.0f;                                        // 009C2186
                const float reach = static_cast<float>(
                    static_cast<double>(unit.df_row.aim_shoot_distance) * 1.2000000476837158);
                const float cone = static_cast<float>(
                    static_cast<double>(bsp::DogfightGunInputs{}.lateral_cap_38) * 1.5);
                for (const auto& owned : slots) {
                    const GameUnitSlot& o = *owned;
                    if (o.state == nullptr || o.state->active == 0 || o.state->simulate != 0 ||
                        o.scene_destroyed_005e != 0 || o.scene_pending_destroy_0060 != 0) {
                        continue;
                    }
                    if (o.row.party == unit.row.party) continue;
                    if (!bsp::unit_is_kind_of(o.class_id, 5)) continue;  // 009C21D7
                    float v[3];
                    follow_to_body(leader, o.motion.position, false, v);
                    const float z = v[2];
                    if (!(z > 1.0f)) continue;                            // 009C2254
                    if (!(reach > z)) continue;                           // 009C226F
                    const float xz = v[0] / z;                            // 009C2278
                    const float yz = v[1] / z;                            // 009C2288
                    const float sq = xz * xz + yz * yz;                   // 0042B2F0
                    const float len = static_cast<double>(sq) > 1e-10
                        ? static_cast<float>(std::sqrt(static_cast<double>(sq))) : 0.0f;
                    if (!(cone > len)) continue;                          // 009C22A8
                    if (!found || best > len) {                           // 009C22AA-009C22B6
                        best = len;
                        unit.fs_sight_78[2] = z;                          // 009C22D5
                    }
                    found = true;                                         // 009C22DD
                }
                ++follow_sight_searches_;
            }
            unit.fs_sight_84 = found;                                     // 009C22F5
            if (found) {
                unit.plane_formation_sight_914 = 1.0f;                    // 009C230B
                ++follow_sight_found_;
            }
        }
        unit.df_gun.hold_4c = unit.fs_sight_84 ? 2.0f : 0.0f;              // 009C232A
        if (unit.fs_sight_84) ++follow_sight_held_ticks_;
    }
    unsigned long long follow_sight_searches_{0}, follow_sight_found_{0};
    unsigned long long follow_sight_held_ticks_{0};

    // 009C207C-009C211C, in 009C1FD0 after 009BEE30. With the good-position
    // flag state+85h (009BFD70's, this tick) and a leader (state+2Ch): b =
    // |leader+C68h| (the bank, -0.0f idiom 00D7A208); when b > 0.5 (00CE3800) or
    // the leader's published byte leader[9C2h + word[00F876B8]*8] is set:
    // plane+844h = 0, plane+840h = 0.0 (009C2102-009C2115); otherwise +844h = 1
    // and +840h = 1.0 - 2b (009C20D5-009C20FA, FADD ST0,ST0 then FSUBRP from
    // FLD1, through a float store). Without the flag or a leader, nothing.
    // LABELLED: the published byte is clear for an AI leader, as in 009BEF09
    // (plane_follow_hold.hpp); +85h is the host's latch from
    // run_follow_law_009bfee0_009bee30 (fw_arm 1 on this motion step).
    void follow_trail_arm_009c207c(GameUnitSlot& unit, const GameUnitSlot& leader) {
        ++trail_arm_calls;
        if (!(unit.fw_arm == 1 && unit.fw_step == summary.motion_steps)) return;
        ++trail_arm_raised;
        const float c68 = leader.plane_bank_angle_c68;
        const float b = c68 > 0.0f ? c68 : -0.0f - c68;
        const bool leader_published_520 = false;
        if (b > 0.5f || leader_published_520) {
            unit.wanderer.enabled = false;
            unit.wanderer.mul = 0.0f;
            ++trail_arm_disabled;
            return;
        }
        unit.wanderer.enabled = true;
        unit.wanderer.mul = static_cast<float>(1.0 - (static_cast<double>(b) + b));
        ++trail_arm_enabled;
        if (unit.wanderer.mul < trail_arm_min_mul) {
            trail_arm_min_mul = unit.wanderer.mul;
        }
    }

    // Packet cc9_plane_retreat_task: 009FB800(altitude, 1.0) as the retreat
    // states call it (009C9820, 009C98FD, 009C9A09).
    void retreat_pitch_009fb800(GameUnitSlot& unit, float altitude) {
        bsp::PlanePitchCommandInputs pin;
        pin.desired_altitude = altitude;
        pin.reference = 1.0f;
        pin.unit_world_y = unit.motion.position[1];
        if (lua.plane_globals_loaded()) {
            const bsp::GameTuningBlock& g = lua.plane_globals();
            pin.ceiling = g.dynamics_ceiling;
            pin.climb_dist = g.pilot_general_climb_dist;
            pin.drop_dist = g.pilot_general_drop_dist;
        }
        pin.class_climb_angle = unit.plane_climb_angle_1ec;
        pin.class_drop_angle = unit.plane_drop_angle;
        unit.plane_commanded_altitude = altitude;
        unit.plane_commanded_pitch = bsp::pitch_command_009fb800(pin);
        unit.plan_state.pitch_target_2bc = unit.plane_commanded_pitch;
        if constexpr (kPitchCommandCallersBound) unit.plan_state.pitch_mode_2d0 = 2;
    }
    // squadron+3A0h = class+190h, TravelSpeed x NewTravelSpeedMul (as
    // moveto_speed_009c1850 takes it; the members' class stands in).
    float retreat_squadron_speed_3a0(const GameUnitSlot& unit) const {
        float travel_mul = 1.6f;
        if (lua.plane_globals_loaded())
            travel_mul = lua.plane_globals().dynamics_spd_multipliers_new_travel_speed_mul;
        return unit.plane_travel_speed * travel_mul;
    }
    // Packet l68_travel_speed: squadron+3A0h as the four readers 009C185C,
    // 009C23BE, 009BC909 and 009C97E4 load it. Off, or with no script speed held,
    // the class value `class_value` (the members' class stands in).
    float squadron_speed_3a0_live(std::size_t unit, float class_value) {
        if constexpr (kLuaSquadronSetTravelSpeedBound) {
            const GameUnitSlot* sq = squadron_slot_of(unit);
            if (sq != nullptr && (sq->sq_speed_set || sq->sq_speed_refreshed)) {
                if (sq->sq_speed_set) ++travel_speed_script_reads;
                return sq->sq_speed_3a0;
            }
        }
        return class_value;
    }
    // 007ED5D0 BSP_PlaneSquadron_RefreshTravelSpeed, from BSP_PilotBot_Tick
    // 0099AEA3 when the member's head task changed and unit+9D4h is set.
    // The retained +388h float is decremented only at the subsequent group-3
    // phase. The squadron is fused with wing 0 (the registry's type_class_id);
    // its loaded class supplies the class speed to the single body.
    unsigned long long travel_speed_refreshes{0};
    unsigned long long travel_speed_reverts{0};
    unsigned long long travel_speed_script_reads{0};
    void refresh_travel_speed_007ed5d0(std::size_t unit) {
        // The native call tests member+9D4h. A standalone plane's fallback
        // block in squadron_slot_of is not a squadron for this operation.
        const auto* record = bsp::plane_squadron_registry().find_by_member_unit(unit);
        if (record == nullptr || record->squadron_unit >= slots.size()) return;
        GameUnitSlot* sq = slots[record->squadron_unit].get();
        if (sq == nullptr) return;
        ++travel_speed_refreshes;
        struct RefreshView final : bsp::PlaneSquadronTravelSpeedView {
            RefreshView(Impl& owner, GameUnitSlot& sq)
                : PlaneSquadronTravelSpeedView(sq.sq_freeze_38f, sq.sq_lock_3ac,
                    sq.sq_speed_3a0), owner(owner), sq(sq) {}
            float countdown_388() const override {
                return bsp::PlaneSquadronTravelSpeedHostCountdown{
                    sq.sq_countdown_388, sq.sq_freeze_38f}.read();
            }
            const float& class_speed_190() const override {
                if (!owner.lua.plane_globals_loaded())
                    throw std::logic_error("Squadron travel-speed refresh requires PlaneGlobals");
                // This is the fused wing-0 plane's genuine descriptor class:
                // PlaneSquadronHostRecord::type_class_id constructs all wings.
                cached_class_speed = owner.retreat_squadron_speed_3a0(sq);
                return cached_class_speed;
            }
            Impl& owner;
            GameUnitSlot& sq;
            mutable float cached_class_speed{};
        } view{*this, *sq};
        if (bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view)) {
            sq->sq_speed_set = false;
            sq->sq_speed_refreshed = true;
            ++travel_speed_reverts;
            log.notef("squadron travel speed: %s back to the class value at %.2f s "
                "(007ED5D0, packet l68_travel_speed)", sq->row.name.c_str(),
                static_cast<double>(summary.simulated_seconds));
        }
    }
    static std::uint64_t head_task_signature(const GameUnitSlot& u) {
        std::uint64_t s = 0;
        const bool kinds[] = {u.dive_bomb_task_installed, u.dogfight_task_installed,
            u.depth_charge_task_installed, u.strafe_task_installed,
            u.kamikaze_task_installed, u.drop_kamikaze_task_installed,
            u.level_bomb_task_installed, u.moveto_task_installed,
            u.moveonpath_task_installed, u.takeoff_task_installed,
            u.retreat_task_installed, u.land_task_installed, u.torpedo_task_installed,
            u.torpedo_closetoship_task_installed};
        for (std::size_t i = 0; i < sizeof(kinds) / sizeof(kinds[0]); ++i) {
            if (kinds[i]) s |= std::uint64_t{1} << i;
        }
        return s | (static_cast<std::uint64_t>(u.command_target_plus_one) << 16);
    }
    // The state transition 009C9F70: the old state's exit (007B3DC0, empty for
    // all four) then the new state's enter; only moveto's 009C92F0 writes.
    void retreat_enter_state(GameUnitSlot& unit, bsp::RetreatState s) {
        if (unit.retreat_state == s) return;
        unit.retreat_state = s;
        if (s == bsp::RetreatState::kMoveTo) {
            unit.retreat_moveto_timer_18 = bsp::retreat_moveto_timer_009c92f0(
                unit.plane_turn_circle_radius, unit.plane_travel_speed);
        }
        log.notef("  retreat %s -> %s at %.2f s (009C9F70, packet cc9_plane_retreat_task)",
            unit.row.name.c_str(),
            s == bsp::RetreatState::kMoveTo ? "moveto (retreat)"
                : s == bsp::RetreatState::kEnterZone ? "enterzone (retreat)"
                : s == bsp::RetreatState::kLeave ? "leave (retreat)" : "follow (retreat)",
            static_cast<double>(summary.simulated_seconds));
    }
    // 009C8D40 then 009C8F40 over the host's world+7134h and world box.
    void retreat_refresh_approach(GameUnitSlot& unit) {
        const bsp::BorderZoneSet* zones = lua.world_border_zones();
        const bsp::WorldMapBounds* bounds = lua.world_border_bounds();
        if (zones == nullptr || bounds == nullptr) return;
        const float* p = unit.motion.position;
        ++unit.retreat_refreshes;
        if (!bsp::retreat_refresh_zone_009c8d40(unit.retreat_approach, *zones,
                {p[0], p[1], p[2]}, unit.row.party,
                unit.plane_turn_circle_radius)) {
            ++unit.retreat_zone_misses;   // the image dereferences null here
        }
        bsp::retreat_refresh_target_009c8f40(unit.retreat_approach, *bounds, *zones, p[0], p[2]);
    }
    // 0099A45B's arm: 009CA2B0 -> 009C9D00 on one plane. LABELLED refusals: a
    // dead plane, a plane not in free flight (the image builds the task on any
    // bot; a parked plane's retreat is not modelled), and a run without the
    // world's border zones.
    bool install_retreat_task_009c9d00(std::size_t m, bool squadron_ordered) {
        if (m >= slots.size()) return false;
        GameUnitSlot& unit = *slots[m];
        if (unit.plane_death_c3a || unit.plane_control_mode_900 != 7
            || lua.world_border_zones() == nullptr || lua.world_border_bounds() == nullptr) {
            ++retreat_refused;
            log.notef("retreat task REFUSED: plane \"%s\": %s at %.2f s (packet "
                "cc9_plane_retreat_task)", unit.row.name.c_str(),
                unit.plane_death_c3a ? "dead" : unit.plane_control_mode_900 != 7
                    ? "not in free flight" : "no border zones",
                static_cast<double>(summary.simulated_seconds));
            return false;
        }
        record("Bot::install_command_task_retreat_arm", 0x0099a45bu);
        // 0099A4C0's retire: the old task goes, whatever it was.
        unit.dive_bomb_task_installed = false;
        unit.dive_bomb_state = bsp::DiveBombState::kNone;
        unit.dogfight_task_installed = false;
        unit.dogfight_state = bsp::DogfightState::kNone;
        unit.torpedo_task_installed = false;
        unit.torpedo_state = bsp::TorpedoState::kNone;
        unit.moveto_task_installed = false;
        unit.moveto_state = GameUnitSlot::MoveToTaskState::kNone;
        if (unit.land_task_installed) land_task_retire_006c8800(unit, "retreat install");
        unit.land_task_installed = false;
        unit.land_state = GameUnitSlot::LandTaskState::kNone;
        unit.strafe_task_installed = false;
        unit.rocket_task_installed = false;
        unit.level_bomb_task_installed = false;
        unit.level_bomb_state = bsp::LevelBombState::kNone;
        unit.attack_command_class = bsp::kRetreatCommandClass;
        // 009C9D26 base, 009C9BB0 -> 009C91B0: the draws, in order.
        const std::string& n = unit.row.name;
        float cruising = 0.0f;
        if (lua.plane_globals_loaded()) cruising = lua.plane_globals().pilot_general_cruising_alt;
        bsp::RetreatApproach& a = unit.retreat_approach;
        a = bsp::RetreatApproach{};
        a.altitude_2c = stream_set2_draw_00bd2f10(unit, n + "#r2c", bsp::kRetreatAltitudeDrawLow,
                                                  bsp::kRetreatAltitudeDrawHigh) + cruising;
        a.margin_30 = stream_set2_draw_00bd2f10(unit, n + "#r30", bsp::kRetreatMarginDrawLow,
                                                bsp::kRetreatMarginDrawHigh);
        a.period_34 = bsp::kRetreatRefreshPeriod;
        a.countdown_38 = -stream_set2_draw_00bd2f10(unit, n + "#r38", 0.0f, 1.0f);
        retreat_refresh_approach(unit);                       // 009C9282, 009C9289
        // 009C9C63 009C2980, the follow state's construction draw.
        float retreat_follow_74;
        if constexpr (bsp::kFollowSightCountdownDrawCc13Bound) {
            retreat_follow_74 = follow_stagger_shared_009c2a0a(unit, n + "#rf");
        } else {
        retreat_follow_74 = -release_altitude_draw_00bd2f10(n + "#rf", 0.0f, 0.6f);
        }
        (void)retreat_follow_74;
        unit.retreat_arm_560 = -stream_set2_draw_00bd2f10(unit, n + "#r560", 0.0f, 1.0f);
        unit.retreat_squadron_ordered = squadron_ordered;
        unit.retreat_exit_6f0 = 0.0f;
        unit.retreat_exit_state_6ec = 0;
        unit.retreat_installed_at = summary.simulated_seconds;
        unit.retreat_task_installed = true;
        bot_task_ctor_seeds_0099c6f0(unit, false);
        if constexpr (kBotTaskCtorSeedsBound) unit.fs_countdown_74 = retreat_follow_74;
        unit.retreat_state = bsp::RetreatState::kNone;
        // 009C9DA0-009C9DC6: leader -> moveto, member -> follow, then its enter.
        retreat_enter_state(unit, unit_is_flight_leader_007b8ad0(m)
            ? bsp::RetreatState::kMoveTo : bsp::RetreatState::kFollow);
        ++retreat_installs;
        const bsp::RetreatApproach& r = unit.retreat_approach;
        log.notef("retreat task install: plane \"%s\" squadron_ordered=%d alt=%.1f margin=%.1f "
            "zone=%d edge=(%.1f,%.1f)-(%.1f,%.1f) target=(%.1f,%.1f) enter=(%.1f,%.1f) "
            "dir=(%.3f,%.3f) lined=%d leave=%d at %.2f s (0099A45B -> 009CA2B0, packet "
            "cc9_plane_retreat_task)", n.c_str(), squadron_ordered ? 1 : 0,
            static_cast<double>(r.altitude_2c), static_cast<double>(r.margin_30),
            r.zone_3c != nullptr ? 1 : 0, static_cast<double>(r.edge_40[0]),
            static_cast<double>(r.edge_40[1]), static_cast<double>(r.edge_40[2]),
            static_cast<double>(r.edge_40[3]), static_cast<double>(r.target_64[0]),
            static_cast<double>(r.target_64[1]), static_cast<double>(r.enter_point_50[0]),
            static_cast<double>(r.enter_point_50[1]), static_cast<double>(r.direction_58[0]),
            static_cast<double>(r.direction_58[1]), r.lined_up_61 ? 1 : 0,
            r.leave_now_60 ? 1 : 0, static_cast<double>(summary.simulated_seconds));
        done("BotTaskRetreat::construct", 0x009c9d00u);
        return true;
    }
    // Packet cc13-leavecd: 007EE2E0 BSP_PlaneSquadron_IsLeavingOnPurpose(sq,
    // check_leader) for plane m's squadron (include/bsp/leave_countdown_all_squadrons_cc13.hpp).
    // LABELLED: a plane with no squadron record is its own one-plane squadron (the image
    // instead skips 007EE2E0 for a null +9D4h, 007C6C6D JZ 007C6C80, and counts down
    // whenever the plane is outside the map; moot for bot planes); the squadron's own
    // director (+174h/+178h/[sq+348h]) is read from its flight leader's, as the spent-RTB
    // path reads it; the retreat arm is the plane's own proxy from
    // run_retreat_task_tick_009ca130, so retreats count as before.
    bool leaving_on_purpose_007ee2e0(std::size_t m, bool check_leader) {
        if (m >= slots.size()) return false;
        const bsp::PlaneSquadronHostRecord* sq =
            bsp::plane_squadron_registry().find_by_member_unit(m);
        const std::size_t leader = sq != nullptr ? sq->flight_leader() : m;
        const bool leader_ok = leader != bsp::kPlaneSquadronNoUnit && leader < slots.size();
        if (check_leader && leader_ok) {
            const GameUnitSlot& l = *slots[leader];
            if (l.current_roles_01ac[1] == kLocalPlayerSlot) return true;   // 00927F30(leader, 1)
            if (l.role_player_0184) return true;                           // leader+184h
        }
        // The retreat arm (00E08F90).
        const GameUnitSlot& u = *slots[m];
        if (u.retreat_task_installed && u.retreat_squadron_ordered) return true;
        const bsp::WorldMapBounds* b = lua.world_border_bounds();
        if (!leader_ok || b == nullptr) return false;
        const std::uint32_t command = commands.current_command_0071be40(leader);   // vcall +174h
        if (command == bsp::kCommandAttackMove || command == bsp::kCommandMoveTo) {
            // vcall +178h: the target's +1h, then its point +8h.
            bsp::SceneCommandTarget target;
            int mode = 0;
            commands.active_command_descriptor_0071eb60(leader, target, mode);
            if (target.position_valid != 0 && bsp::point_outside_world_map_0071c4f0(*b,
                    {target.position[0], target.position[1], target.position[2]})) {
                return true;
            }
        }
        if (command == bsp::kCommandMoveOnPath) {
            // 0071BFF0(0), 007B1350: the path's last point when its +0Ch byte is set.
            float last[3]{};
            if (commands.path_final_point_007b1350(leader, last)
                && bsp::point_outside_world_map_0071c4f0(*b, {last[0], last[1], last[2]})) {
                return true;
            }
        }
        return false;
    }
    // Packet cc13-leavecd: 007C6C30 for one plane, from its fixed step (00953D98).
    void leave_countdown_007c6c30(GameUnitSlot& unit, float step) {
        GameGunneryHost* const gun = gunnery.get();
        const bsp::WorldMapBounds* b = lua.world_border_bounds();
        if (gun == nullptr || b == nullptr || !lua.plane_globals_loaded()) return;
        const std::size_t m = unit.process_index;
        if (m >= slots.size()) return;
        ++leave_countdown_calls;
        const bsp::GameTuningBlock& g = lua.plane_globals();
        bsp::gameplay_seg47_cc13::ExitCountdownTuning t;
        t.margin_348 = g.retreat_exit_dist;
        t.period_34c = g.retreat_exit_time;
        t.warn_period_350 = g.retreat_warning_repeat_time;
        const bsp::PlaneSquadronHostRecord* sq =
            bsp::plane_squadron_registry().find_by_member_unit(m);
        float& warn = leave_warn_36c[sq != nullptr ? sq->name : unit.row.name];
        bsp::gameplay_seg47_cc13::ExitCountdownState s;
        s.timer_6f0 = unit.retreat_exit_6f0;
        s.state_6ec = unit.retreat_exit_state_6ec;
        s.has_squadron_9d4 = true;
        s.flag_9d8 = !unit_is_flight_leader_007b8ad0(m);   // +9D8h != 0: not slot 0
        s.squadron_warn_36c = warn;
        const float* p = unit.motion.position;
        bool send = false;
        bsp::gameplay_seg47_cc13::ExitCountdownHost h;
        h.outside_map = [&] {
            return bsp::point_outside_world_map_0071c4f0(*b, {p[0], p[1], p[2]});
        };
        h.squadron_leaving = [&] { return leaving_on_purpose_007ee2e0(m, true); };
        h.beyond_margin = [&](float margin) {
            return bsp::point_beyond_map_margin_0059c9b0(*b, p[0], p[2], margin);
        };
        h.local_player_role = [&] { return unit.current_roles_01ac[0] == kLocalPlayerSlot; };
        // 007C6D5C..007C6D65: 00982120([00F8A0C4], plane, 0). 00982120 first takes
        // entity->vtable[140h](), 007B97E0 for a plane (+9D4h, the squadron), so the
        // hook gets the squadron's unit, the same entity the exit (plane, 1) events
        // carry (dispatch_exit_zone_listeners_00982120), and the warning tail and the
        // exitzone listeners (exited 0) run on it. A plane whose squadron has no unit
        // slot is the plane itself there too.
        h.warn_exit_zone = [&] {
            if (sq == nullptr) {
                record("Plane::out_of_action_notice_00982120", 0x00982120u);
                return;
            }
            lua.note_exit_zone_00982120(
                sq->squadron_unit != bsp::kPlaneSquadronNoUnit ? sq->squadron_unit : m, false);
            ++leave_warnings_00982120;
            done("Plane::out_of_action_notice_00982120", 0x00982120u);
        };
        h.send_exit_zone = [&] { send = true; };
        bsp::gameplay_seg47_cc13::out_of_action_countdown_007c6c30(s, gun->unit_dead(m), step, t, h);
        unit.retreat_exit_6f0 = s.timer_6f0;
        unit.retreat_exit_state_6ec = s.state_6ec;
        warn = s.squadron_warn_36c;
        if (s.state_6ec >= 2) {
            ++leave_counting_steps;
            if (unit.retreat_task_installed) {
                ++unit.retreat_outside_ticks;
                if (unit.retreat_outside_at < 0.0f)
                    unit.retreat_outside_at = summary.simulated_seconds;
            } else {
                ++leave_counting_not_retreat;
                if (!unit.leave_counting_logged) {
                    unit.leave_counting_logged = true;
                    log.notef("leave countdown: plane \"%s\" squadron \"%s\" counting, not "
                        "retreating, at %.2f s pos=(%.1f, %.1f) timer=%.2f (00953D98 -> "
                        "007C6C30 -> 007EE2E0, packet cc13-leavecd)", unit.row.name.c_str(),
                        sq != nullptr ? sq->name.c_str() : "-",
                        static_cast<double>(summary.simulated_seconds),
                        static_cast<double>(p[0]), static_cast<double>(p[2]),
                        static_cast<double>(s.timer_6f0));
                }
            }
        }
        if (send) retreat_leave_map_007ee410(m);   // 007C6DC1
        done("Plane::out_of_action_countdown_007c6c30", 0x007c6c30u);
    }
    // 007EE410's live gate and border-party lookup, implemented in
    // game_hosts_units_motion.cpp; 007CA790 now calls the reconstructed
    // 007F31A0 leave-map body over this squadron's member/latch owners.
    void retreat_leave_map_007ee410(std::size_t m);
    // 007CA790 BSP_Plane_OnExitZone, __thiscall(plane, party), RET 4, the handler of
    // message 50h (bsp.py disasm-raw 007ca790): under the mission entity lock
    // (004C1570's +4h critical section, 007CA7AD..007CA7CE), the plane's Lua
    // self.ExitZoneParty = party (00927B40 at 007CA815, 00B67460 at 007CA82B, an
    // integer), then 007CA862..007CA86E 007F31A0(ECX = plane+9D4h, plane, party) when
    // +9D4h is set, which it is on every path here (007EE410 itself runs with ECX =
    // plane+9D4h). The lock has no host counterpart: this host runs the mission on
    // one thread.
    void run_exit_zone_007ca790(std::size_t unit, std::int32_t party,
                                const std::function<void(std::int32_t)>& left_map) {
        bsp::gameplay_seg47_cc13::ExitZoneHost h;
        h.set_lua_exit_zone_party = [&](std::int32_t p) {
            store_exit_zone_party_007ca790(unit, p);
        };
        h.squadron_left_map = left_map;
        bsp::gameplay_seg47_cc13::exit_zone_007ca790(true, party, h);
        done("Plane::exit_zone_party_007ca790", 0x007ca790u);
    }
    // 007CA7D2..007CA82B: the key "ExitZoneParty" (00D05E00, copied into a native
    // string through 0041DD40), 00927B40's thisTable slot of the plane (the units-host
    // index plus one, kMissionLuaEntityKeyFormat), and 00B67460 stores the party there
    // as an integer. A plane with no slot stores nothing.
    void store_exit_zone_party_007ca790(std::size_t unit, std::int32_t party);
    bool run_follow_tick_009c1fd0(GameUnitSlot& unit, float state_8c = 1.0f) {
        if constexpr (kFollowSightSearchBound) follow_sight_enter_009bed80(unit);
        // 009BFDA6-009BFDB2: the calling state's +8Ch. The follow state's is
        // 009BED80's 1.0 and nothing in its tick moves it (packet
        // cc9_l61_torpedo_done_formation); the level-bomb prepare passes its own.
        if constexpr (kTorpedoDoneFormationBound) {
            if (bsp::PlaneSquadronHostRecord* const sq =
                    bsp::plane_squadron_registry().find_by_member_unit(unit.process_index)) {
                if (!unit_is_flight_leader_007b8ad0(unit.process_index)) sq->morale_3e8 = state_8c;
            }
        }
        bsp::PlaneFormationStation station;
        const GameUnitSlot* leader = nullptr;
        place_wing_member_on_station_007f23a0(unit, false, &station, &leader, false);
        if (!station.produced || leader == nullptr) return false;
        // 009C1FFD-009C2063: a pending or queued release takes the level arm and
        // returns before the law.
        if (unit.torpedo_release_pending_c25 || unit.torpedo_issue_requests_c20 > 0) {
            ++follow_release_pending_ticks;
            if constexpr (kFollowReleaseArmBound) {
                follow_release_arm_009c200f(unit);
                return true;
            }
        }
        run_follow_law_009bfee0_009bee30(unit, station, *leader);
        if constexpr (kFollowTrailArmBound) {
            // 009C1FFD-009C2006: a pending or queued release returns first.
            if (!(unit.torpedo_release_pending_c25 || unit.torpedo_issue_requests_c20 > 0))
                follow_trail_arm_009c207c(unit, *leader);
        }
        if constexpr (kFollowSightSearchBound) {
            // 009C211D-009C232A, behind the same release test (009C1FFD-009C2006
            // returns before it). +74h is seeded at each task install with
            // kBotTaskCtorSeedsBound (bot_task_ctor_seeds_0099c6f0, the retreat and
            // land installs) as 009C2980's -U(0, 0.6) (009C2A0A). SUBSTITUTION,
            // labelled: drawn from the host's keyed LCG, not stream 1; under
            // kFollowSightCountdownDrawCc13Bound from the shared stream-1 generator.
            if (!(unit.torpedo_release_pending_c25 || unit.torpedo_issue_requests_c20 > 0))
                follow_sight_tick_009c211d(unit, *leader, unit.fs_think_dt, unit.fs_countdown_74);
        }
        return true;
    }

    // 007DB4D0 (007DB4D0-007DB62F, __thiscall(class), RET, ST0), read whole in
    // packet cc9_land_begin_state (docs/SQUADRON_LAND_TASK.md): the turn rate
    // at TurnRollLeader, which it stores at class+270h (007DB616) before
    // returning class+26Ch = TravelSpeed / rate.  When the pitch share e
    // exceeds 0.6 (00CE3D30) the image also rewrites +260h as
    // atan(0.6 PitchSpd / c) (007DB5EB); this host does not keep that write.
    float plane_class_turn_rate_270_007db4d0(const GameUnitSlot& u) const {
        const float trl = u.plane_class_turn_roll_leader;                     // +260h
        const float sr = static_cast<float>(std::sin(static_cast<double>(trl)));
        const float cr = static_cast<float>(std::cos(static_cast<double>(trl)));
        const float a = static_cast<float>(
            static_cast<double>(u.plane_class_turn_roll_spd) * sr);           // +1C8h
        float y1 = 0.0f, y2 = 0.0f;
        if (lua.plane_globals_loaded()) {
            y1 = lua.plane_globals().pilot_general_yaw_turn_roll_range_1;
            y2 = lua.plane_globals().pilot_general_yaw_turn_roll_range_2;
        }
        const float bi = bsp::clamped_interpolate_00419010(y1, 1.0f, y2, 0.0f, trl);
        const float b = static_cast<float>(static_cast<double>(bi) * 0.20000000298023224);
        float rate;
        if constexpr (kPc24FlightRenderCc13Bound) {
            // pc24w1-sweep1-3: each op rounded at PC24. c = (sr * slide + b) * yaw_spd
            // (007DB566..007DB570), dd = a + c * cr (007DB584..007DB58C), e = (sr * c) /
            // (ps * cr) (007DB59E..007DB5AC), then (ps * e) * sr + dd (007DB603..007DB60A).
            const float ys = u.plane_class.yaw_spd;                           // +1B0h
            const float c = static_cast<float>(cc13x87::x87_mul24(
                cc13x87::x87_add24(cc13x87::x87_mul24(sr, u.plane_class.slide_ratio), b), ys));
            const float dd = static_cast<float>(cc13x87::x87_add24(a, cc13x87::x87_mul24(c, cr)));
            const float ps = u.plane_class.pitch_spd;                         // +1ACh
            float e = static_cast<float>(cc13x87::x87_div24(cc13x87::x87_mul24(sr, c),
                                                            cc13x87::x87_mul24(ps, cr)));
            if (e > 0.6000000238418579f) e = 0.6000000238418579f;
            rate = static_cast<float>(cc13x87::x87_add24(
                cc13x87::x87_mul24(cc13x87::x87_mul24(ps, e), sr), dd));
        } else {
        const double ys = u.plane_class.yaw_spd;                              // +1B0h
        const float c = static_cast<float>(
            (static_cast<double>(sr) * u.plane_class.slide_ratio + b) * ys);  // +1B8h
        const float dd = static_cast<float>(static_cast<double>(a) +
            static_cast<double>(c) * cr);
        const double ps = u.plane_class.pitch_spd;                             // +1ACh
        float e = static_cast<float>((static_cast<double>(sr) * c) /
            (static_cast<double>(cr) * ps));
        if (e > 0.6000000238418579f) e = 0.6000000238418579f;
        rate = static_cast<float>(static_cast<double>(dd) +
            ps * e * static_cast<double>(sr));
        }
        return rate;
    }

    void run_follow_law_009bfee0_009bee30(
        GameUnitSlot& unit,
        const bsp::PlaneFormationStation& station,
        const GameUnitSlot& leader) {
        if (!lua.plane_globals_loaded()) return;
        // Follow-derived task arms also enter this shared law directly. Their
        // projected entry uses the same think-gap rule as the sight hook; a
        // second visit in one think must not copy/run the native kernel again.
        retain_follow_parameters(unit);
        const bsp::GameTuningBlock& gt = lua.plane_globals();
        // DIAGNOSTIC (cc9_follow_cross_track): this tick's split, bucketed below.
        double fct_tick_along = 0.0, fct_tick_cross = 0.0, fct_tick_err = 0.0;
        bool fct_tick_valid = false;
        const float fct_rate = leader_turn_rate_007d7da0(leader);
        const int fct_turn = std::fabs(fct_rate) > 0.05f ? 1 : 0;
        if constexpr (kFollowTurboBound) {
            unit.plan_turbo_2e5 = true;   // 009BEE42, before the +85h branch
        }
        if constexpr (kPlaneFollowLawBound) {
            // 009BFD70's +85h latch, recomputed every tick with no hysteresis
            // (009BFDD4 clear, 009BFE19 distance, 009BFE62-7B heading dot):
            // within GoodPositionDist of the station AND leader-forward .
            // member-forward above GoodPositionDir.
            const double dx = static_cast<double>(unit.motion.position[0]) - station.world[0];
            const double dy = static_cast<double>(unit.motion.position[1]) - station.world[1];
            const double dz = static_cast<double>(unit.motion.position[2]) - station.world[2];
            const float* lf = leader.motion.pose_row2;
            const float* mf = unit.motion.pose_row2;
            double dist;
            double dot;
            if constexpr (kPc24PlaneTasksCc13Bound) {
                // pc24w1-sweep3-5: the bytes store d = member - station through float
                // (009BFDF8..009BFE15), take 0042B2F0 (009BFE19) and the dot
                // (y*y' + x*x') + z*z' (009BFE62..009BFE79, FSTP float 009BFE7B), each op
                // rounded at PC24.
                namespace pc24 = bsp::cc13x87;
                const float d[3] = {static_cast<float>(dx), static_cast<float>(dy),
                                    static_cast<float>(dz)};
                dist = length_0042b2f0_pc24(d);
                dot = static_cast<float>(pc24::x87_add24(
                    pc24::x87_add24(pc24::x87_mul24(lf[1], mf[1]), pc24::x87_mul24(lf[0], mf[0])),
                    pc24::x87_mul24(lf[2], mf[2])));
            } else {
            dist = std::sqrt(dx * dx + dy * dy + dz * dz);
            dot = static_cast<double>(lf[0]) * mf[0] +
                  static_cast<double>(lf[1]) * mf[1] +
                  static_cast<double>(lf[2]) * mf[2];
            }
            const bool latch_85 = gt.pilot_follow_good_position_dist > dist &&
                dot > gt.pilot_follow_good_position_dir;
            unit.fw_arm = latch_85 ? 1 : 2;
            unit.fw_station_dist = static_cast<float>(dist);
            ++unit.fw_law_ticks;
            {
                // DIAGNOSTIC (cc9_follow_error_split): along = d . f, cross =
                // d . (f.z, -f.x), f the leader's horizontal forward.
                double fx = lf[0], fz = lf[2];
                const double fl = std::sqrt(fx * fx + fz * fz);
                if (fl > 1e-6) {
                    // pc24-lint: allow diagnostic counters only (cc9_follow_error_split); no state reads them
                    fx /= fl;
                    // pc24-lint: allow diagnostic counters only (cc9_follow_error_split); no state reads them
                    fz /= fl;
                    const double along = dx * fx + dz * fz;
                    const double cross = dx * fz - dz * fx;
                    const float* vo = unit.plane_world_velocity;
                    const float* vl = leader.plane_world_velocity;
                    // pc24-lint: allow diagnostic counters only (cc9_follow_error_split); no state reads them
                    const double so = std::sqrt(static_cast<double>(vo[0]) * vo[0] +
                        static_cast<double>(vo[1]) * vo[1] + static_cast<double>(vo[2]) * vo[2]);
                    // pc24-lint: allow diagnostic counters only (cc9_follow_error_split); no state reads them
                    const double sl = std::sqrt(static_cast<double>(vl[0]) * vl[0] +
                        static_cast<double>(vl[1]) * vl[1] + static_cast<double>(vl[2]) * vl[2]);
                    fct_tick_along = along;
                    fct_tick_cross = cross;
                    fct_tick_err = dist;
                    fct_tick_valid = true;
                    {
                        const int k = unit.land_task_installed ? 1 : 0;
                        ++fct_task_n[k];
                        fct_task_err[k] += dist;
                        fct_task_cross_abs[k] += std::fabs(cross);
                        fct_task_along[k] += along;
                        // pc24-lint: allow diagnostic counters only (cc9_follow_error_split); no state reads them
                        fct_task_speed[k] += so;
                        if (so < 40.0) ++fct_task_slow[k];
                    }
                    ++fes_n;
                    fes_along_sum += along;
                    fes_cross_abs_sum += std::fabs(cross);
                    // pc24-lint: allow diagnostic counters only (cc9_follow_error_split); no state reads them
                    fes_dy_abs_sum += std::fabs(dy);
                    fes_speed_diff_sum += so - sl;
                    const float tm = unit.plan_slots[bsp::kPilotSlotThrottle].current;
                    const float tl = leader.plan_slots[bsp::kPilotSlotThrottle].current;
                    fes_thr_member_sum += tm;
                    fes_thr_leader_sum += tl;
                    if (tm >= 0.99f) ++fes_member_full;
                    if (tl >= 0.99f) ++fes_leader_full;
                    // pc24-lint: allow diagnostic counters only (cc9_follow_error_split); no state reads them
                    fes_member_v_sum += so;
                    // pc24-lint: allow diagnostic counters only (cc9_follow_error_split); no state reads them
                    fes_leader_v_sum += sl;
                    fes_member_max_sum += unit.plane_max_spd;
                    fes_leader_max_sum += leader.plane_max_spd;
                    fes_leader_want_sum += leader.plane_desired_speed_2b4;
                    if (along < -50.0 && std::fabs(along) >= std::fabs(cross)) {
                        ++fes_behind;
                        fes_behind_speed_diff_sum += so - sl;
                    } else if (along > 50.0 && std::fabs(along) >= std::fabs(cross)) {
                        ++fes_ahead;
                    } else if (std::fabs(cross) > 50.0) {
                        ++fes_beside;
                    }
                }
            }
            follow_station_err_sum += dist;
            ++follow_station_err_n;
            if (static_cast<float>(dist) > follow_station_err_max) {
                follow_station_err_max = static_cast<float>(dist);
            }
            unit.fw_step = summary.motion_steps;
            for (int i = 0; i < 3; ++i) unit.fw_station[i] = station.world[i];
            unit.fw_leader_heading = leader.plane_heading_c6c;
            if (latch_85) {
                if (fct_tick_valid) fct_add(fct_turn, 0, fct_tick_along, fct_tick_cross, fct_tick_err, fct_rate);
                run_follow_hold_arm_009bee56(unit, station, leader, gt);
                return;
            }
            ++follow_flyto_ticks_;
        }

        bsp::PlaneFollowGeometryInputs gin;
        for (int i = 0; i < 3; ++i) {
            gin.own_pos[i] = unit.motion.position[i];
            gin.station[i] = station.world[i];
        }
        gin.own_heading = unit.plane_heading_c6c;
        gin.leader_heading = leader.plane_heading_c6c;
        // CameraMatrix is the image's 16-float order, so row 2
        // at leader+0ECh is the forward basis and row 3 at
        // leader+0FCh the translation - the same two rows
        // 009C0F0D and 009C16EB read.
        for (int i = 0; i < 3; ++i) {
            gin.leader_forward[i] = leader.motion.pose_row2[i];
            gin.leader_pos[i] = leader.motion.position[i];
        }
        // SUBSTITUTION, labelled: 007D7DA0(leader+0AB0h) at
        // 009C0109 is the leader's turn rate and its body is
        // unread.  Zero makes `ref` the leader's instantaneous
        // heading, which is this law's own zero-turn-rate limit;
        // it costs the lag only while the leader is turning.
        gin.leader_turn_rate = 0.0f;
        if constexpr (kFollowLeaderTurnRateBound) {
            gin.leader_turn_rate = leader_turn_rate_007d7da0(leader);   // 009C0109
        }
        gin.followed_point_dist = gt.pilot_follow_followed_point_dist;
        gin.leader_heading_time_1 =
            gt.pilot_follow_leader_heading_spd_time_1;
        gin.leader_heading_time_2 =
            gt.pilot_follow_leader_heading_spd_time_2;
        gin.leader_heading_dist_1 =
            gt.pilot_follow_leader_heading_spd_dist_1;
        gin.leader_heading_dist_2 =
            gt.pilot_follow_leader_heading_spd_dist_2;
        // 009C16E8 reads block+04h = singleton+384h, which this
        // host already carries as Pilot/Follow/LeaderFollowAlt,
        // and 009C1789 reads singleton+210h = Dynamics/Ceiling.
        gin.band_floor_offset = gt.pilot_follow_leader_follow_alt;
        gin.band_ceiling_210 = gt.dynamics_ceiling;
        gin.state_88 = 1.0e30f;
        if constexpr (kFollowBandSafeAltFloorBound) {
            // 009BED84-009BEDBB: follow Enter latches state+88h from the
            // block copy's +08h = singleton+388h Pilot/Follow/SafeAlt; the
            // tail then bounds at leaderY - 120 on every path.
            gin.state_88 = unit.fs_parameters.safe_alt();
            gin.band_floor_image = true;
        }
        gin.band_inputs_available = true;

        // Phase A's inputs, 009C025E-009C0323: the leader's world velocity
        // (vtable +34h, 007BBB70 = unit+AC8h), the own vtable +5Ch(10h) /
        // +5Ch(16h) pair that picks LargePlaneTurnMul, the class turn rate
        // class+270h (007DB4D0) and TravelSpeed class+18Ch; the abeam
        // regime's nose is pose row 2 (009C15E2).
        for (int i = 0; i < 3; ++i) {
            gin.leader_velocity[i] = leader.plane_world_velocity[i];
            gin.own_forward[i] = unit.motion.pose_row2[i];
        }
        gin.own_large_turn_class = bsp::unit_is_kind_of(unit.class_id, 0x10) ||
                                   bsp::unit_is_kind_of(unit.class_id, 0x16);
        gin.small_plane_turn_mul = gt.pilot_follow_small_plane_turn_mul;
        gin.large_plane_turn_mul = gt.pilot_follow_large_plane_turn_mul;
        gin.class_turn_rate_270 = plane_class_turn_rate_270_007db4d0(unit);
        gin.class_travel_speed_18c = unit.plane_travel_speed;
        gin.run_phase_a = kFollowPhaseABound;
        const bsp::PlaneFollowGeometry geo =
            bsp::plane_follow_geometry_009bfee0(
                gin, bsp::PlaneFollowRegime::kLeadPursuit);
        {
            bsp::PlaneFollowGeometry shadow;
            if constexpr (!kFollowPhaseABound) {
                bsp::PlaneFollowGeometryInputs shadow_in = gin;
                shadow_in.run_phase_a = true;
                shadow = bsp::plane_follow_geometry_009bfee0(
                    shadow_in, bsp::PlaneFollowRegime::kLeadPursuit);
            }
            const bsp::PlaneFollowGeometry& pa = kFollowPhaseABound ? geo : shadow;
            if (pa.phase_a_bl & 1) ++follow_phase_a_lead;
            else if (pa.phase_a_bl & 8) ++follow_phase_a_circle;
            else ++follow_phase_a_abeam;
            if (fct_tick_valid) {
                const int branch = (pa.phase_a_bl & 1) ? 1 : ((pa.phase_a_bl & 8) ? 3 : 2);
                fct_add(fct_turn, branch, fct_tick_along, fct_tick_cross, fct_tick_err, fct_rate);
            }
            follow_phase_a_time_sum += pa.phase_a_time;
        }
        done("BotStateFollow::steer_point", 0x009bfee0u);

        bsp::PlaneFollowFlyToInputs fin;
        for (int i = 0; i < 3; ++i) {
            fin.station[i] = station.world[i];
            fin.steer_point[i] = geo.steer_point[i];
            fin.unit_pos[i] = unit.motion.position[i];
        }
        // The tail rewrote the station's Y (009C17F1), and that
        // rewritten value is what 009BEE30 blends from.
        fin.station[1] = geo.station_y;
        fin.unit_forward_x = static_cast<float>(
            std::sin(static_cast<double>(unit.plane_heading_c6c)));
        fin.unit_forward_z = static_cast<float>(
            std::cos(static_cast<double>(unit.plane_heading_c6c)));
        // 009BFC58 `CALL [[state+2Ch]]+38h`, the leader-speed
        // virtual: the leader's own travel speed.
        fin.leader_speed = leader.plane_travel_speed;
        if constexpr (kFollowLeaderLiveSpeedBound) {
            fin.leader_speed = leader_live_speed_007b8e60(leader);       // 007B8E60, labelled |v|
        }
        // SUBSTITUTIONS, labelled: 007C47F0
        // BSP_PlaneClass_LevelFlightSpeed (009BFC41, then
        // `FMUL double [00D7A390]` = 0.9) and the floor at
        // classDesc+188h (009BFC7D) are not carried by this
        // host.  §5.4 records that the alignment ramp can only
        // reach 0.39 of the way toward the first of them, and
        // the second only scales the catch-up end.
        if constexpr (kPlaneFollowCatchupBound) {
            // 009BFC41 CALL 007C47F0 (desc+184h StallSpd x tuning+24Ch
            // LevelFlight), 009BFC49 FMUL double [00D7A390] = 0.9.
            fin.level_flight_speed = static_cast<float>(
                static_cast<double>(bot_desired_speed_007c47f0(unit)) * 0.9);
            // 009BFC7D: the floor is classDesc+188h, MaxSpd.
            fin.class_min_speed = unit.plane_max_spd;
        } else {
            fin.level_flight_speed = unit.plane_max_spd * 0.9f;
            fin.class_min_speed = unit.plane_stall_spd;
        }
        fin.good_position_dist = gt.pilot_follow_good_position_dist;
        fin.align_ramp_lo = gt.pilot_follow_max_follow_spd_target_dir;
        fin.align_ramp_hi = gt.pilot_follow_min_follow_spd_target_dir;
        fin.catchup_speed_scale =
            gt.dynamics_spd_multipliers_turbo_multiplier;
        fin.min_command_dist = gt.pilot_follow_followed_point_dist;
        const bsp::PlaneFollowFlyToCommand cmd =
            bsp::plane_follow_flyto_command_009bee30(fin);
        if (!cmd.produced) return;

        // 009BF9EA-009BF9F0: the pilot is steered at the steer
        // point.  Same mode-2 pair the moveto and attackrun
        // ticks write; 009F9E40 read whole; heading_command_009f9e40
        // is its law (PLANE_FLIGHT, cc9_wingman_heading_to_point).
        unit.plan_heading_2c0 = bsp::heading_command_009f9e40(
            geo.steer_point[0], geo.steer_point[2],
            unit.motion.position[0], unit.motion.position[2]);
        unit.plan_heading_2c0_written = true;
        unit.plan_heading_mode_2cc = 2;
        done("BotStateFollow::steer_to_point", 0x009f9e40u);

        // 009BFC0C CALL 009F9ED0(cmdAlt - ownY, dist).  That
        // body is unread, so the commanded altitude is turned
        // into a pitch through the same 009FB800 every other
        // state uses, with the commanded altitude as its own
        // reference.  SUBSTITUTION, labelled.
        // CORRECTION (cc13-tranche2 S25, 2026-10-28): 009F9ED0 is read
        // (bsp::pitch_command_to_point_009f9ed0, packet cc9_follow_pitch) and runs under
        // kPlaneFollowFlyToPitch (true) below; the 009FB800 substitution above is the
        // OFF arm only.
        bsp::PlanePitchCommandInputs pin;
        pin.desired_altitude = cmd.commanded_altitude;
        pin.reference = cmd.commanded_altitude;
        pin.unit_world_y = unit.motion.position[1];
        pin.ceiling = gt.dynamics_ceiling;
        pin.climb_dist = gt.pilot_general_climb_dist;
        pin.drop_dist = gt.pilot_general_drop_dist;
        pin.class_climb_angle = unit.plane_climb_angle_1ec;
        pin.class_drop_angle = unit.plane_drop_angle;
        unit.plane_commanded_altitude = cmd.commanded_altitude;
        if constexpr (kPlaneFollowFlyToPitch) {
            // 009BFC03-009BFC21 CALL 009F9ED0(cmdAlt - ownY, max(dist,
            // FollowedPointDist)), read in packet cc9_follow_pitch: the
            // elevation angle of the steer altitude, capped at the class's
            // sustainable climb desc+1E4h, stored to plan+2BCh with the pitch
            // mode plan+2D0h = 2 (009F9F68 / 009F9F70). docs/PLANE_FOLLOW_PITCH.md.
            unit.plane_commanded_pitch = bsp::pitch_command_to_point_009f9ed0(
                cmd.altitude_error, cmd.command_distance, unit.plane_climb_angle_1e4);
            unit.plan_state.pitch_target_2bc = unit.plane_commanded_pitch;
            unit.plan_state.pitch_mode_2d0 = 2;
        } else {
            unit.plane_commanded_pitch = bsp::pitch_command_009fb800(pin);
            unit.plan_state.pitch_target_2bc = unit.plane_commanded_pitch;
        }

        // 009BFD0F-009BFD1C.
        unit.plane_desired_speed_2b4 = cmd.desired_speed_2b4;
        if constexpr (kPlaneFollowFlyToSpeedStores) {
            unit.plane_trg_speed_corr_off_2b0 = 0;   // 009BFD15, byte
            unit.plane_air_brake_mode_2d8 = 1;       // 009BFD1C, dword
        }
        done("BotStateFollow::command_step", 0x009bee30u);
        if constexpr (kTaskGunConeBound) {
            // 009BFD2B-009BFD36: the fly-to arm's cone = Angle_MoveTo.
            unit.gun_cone_40 = strafe_cone_tuning(0x670);
        }
        gun_direction_from_plan_009fabe0(unit, 0x009bfd59u);   // 009BFD3B-009BFD59

        {
            // DIAGNOSTIC (cc9_follow_error_split): commanded minus own speed.
            const float* vo = unit.plane_world_velocity;
            // pc24-lint: allow diagnostic counters only (cc9_follow_error_split); no state reads them
            const double so = std::sqrt(static_cast<double>(vo[0]) * vo[0] +
                static_cast<double>(vo[1]) * vo[1] + static_cast<double>(vo[2]) * vo[2]);
            // pc24-lint: allow diagnostic counters only (cc9_follow_error_split); no state reads them
            fes_flyto_cmd_minus_own_sum += static_cast<double>(cmd.desired_speed_2b4) - so;
            ++fes_flyto_n;
        }
        if ((unit.fw_law_ticks % 400) == 1) {
            log.notef("  follow law %-12s n=%d R=%.1f D=%.1f "
                "ref=%.3f V=%.1f along=%.1f A=%.3f lag=%.2f "
                "steer=(%.1f %.1f %.1f) cmdalt=%.1f ownY=%.1f "
                "spd=%.2f band=%d",
                unit.row.name.c_str(), unit.fw_law_ticks,
                static_cast<double>(geo.range_horizontal),
                static_cast<double>(geo.range_3d),
                static_cast<double>(geo.reference_heading),
                static_cast<double>(geo.cross_track),
                static_cast<double>(geo.along_track),
                static_cast<double>(geo.heading_error),
                static_cast<double>(geo.lag_time),
                static_cast<double>(geo.steer_point[0]),
                static_cast<double>(geo.steer_point[1]),
                static_cast<double>(geo.steer_point[2]),
                static_cast<double>(cmd.commanded_altitude),
                static_cast<double>(unit.motion.position[1]),
                static_cast<double>(cmd.desired_speed_2b4),
                geo.band_applied ? 1 : 0);
        }
        // DIAGNOSTIC, packet cc9_follow_speed: the throttle slot, the speed mode,
        // the pitch command and the measured speed, every kFollowTraceEvery ticks.
        if constexpr (kFollowTraceEvery > 0) {
        constexpr int kEvery = kFollowTraceEvery > 0 ? kFollowTraceEvery : 1;
        if ((unit.db_follow_tick_ticks % kEvery) == 1) {
            const bsp::PilotPlanSlot& th = unit.plan_slots[bsp::kPilotSlotThrottle];
            const float v2 = unit.plane_world_velocity[0] * unit.plane_world_velocity[0] +
                unit.plane_world_velocity[1] * unit.plane_world_velocity[1] +
                unit.plane_world_velocity[2] * unit.plane_world_velocity[2];
            log.notef("  follow trace %-12s n=%d ownY=%.1f cmdalt=%.1f pitch=%.3f "
                "v=%.2f want=%.2f mode2d8=%d thr=%.3f/%.3f act=%d state=%d",
                unit.row.name.c_str(), unit.db_follow_tick_ticks,
                static_cast<double>(unit.motion.position[1]),
                static_cast<double>(cmd.commanded_altitude),
                static_cast<double>(unit.plane_commanded_pitch),
                static_cast<double>(std::sqrt(v2)),
                static_cast<double>(unit.plane_desired_speed_2b4),
                unit.plane_air_brake_mode_2d8,
                static_cast<double>(th.current), static_cast<double>(th.desired),
                static_cast<int>(th.active), unit.plane_control_mode_900);
        }
        }
    }

    // The three numbers docs/TORPEDO_RELEASE_GEOMETRY.md section 3 asked for
    // and did not take: |v|, the angle between v and the forward pose row, and
    // the body-axis speed 0092D730 itself computes (the dot of the body's
    // linear velocity from 00C31F40 with row 3 of the axis matrix 00C32000).
    // out[0] = magnitude, out[1] = angle in radians, out[2] = body-axis speed.
    static void velocity_versus_nose(const GameUnitSlot& slot, float out[3]) {
        const float v[3] = {slot.motion.linear_velocity.x,
            slot.motion.linear_velocity.y, slot.motion.linear_velocity.z};
        const float* const fwd = slot.motion.pose_row2;
        const float mag = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        const float fwd_len =
            std::sqrt(fwd[0] * fwd[0] + fwd[1] * fwd[1] + fwd[2] * fwd[2]);
        const float dot = v[0] * fwd[0] + v[1] * fwd[1] + v[2] * fwd[2];
        out[0] = mag;
        out[2] = dot;   // 0092D730's own answer
        if (mag > 1e-6f && fwd_len > 1e-6f) {
            float c = dot / (mag * fwd_len);
            if (c > 1.0f) c = 1.0f;
            if (c < -1.0f) c = -1.0f;
            out[1] = static_cast<float>(std::acos(static_cast<double>(c)));
        } else {
            out[1] = 0.0f;
        }
    }

    // 009FADA0 with its tail: hull_aim_world_point, then the lead when the
    // shooter's sub-object carries a positive projtime. Defined after
    // UnitRudderBinding. docs/TORPEDO_AIM_LEAD.md section 11.
    bool aim_point_009fada0(GameUnitSlot& shooter, const GameUnitSlot& target,
                            std::size_t target_plus_one, float out[3],
                            GameHostLog* log_out = nullptr);
    // Packet cc13-aimfreeze3: aim_point_009fada0 with 009FADA0's death head and
    // 009FAF90's notify, for the switched approach ticks only (dive 009C7A9F, level bomb
    // 009B7CA1, torpedo 009D34BB; task 1 / 2 / 3), the one place the image runs 009FADA0.
    // `target` may be null: a latched ref still answers its frozen point. False when
    // there is no target and no latch (the caller's fallback stands).
    // aim_freeze_point is the approach->vtable[0] reader's side: the frozen sub+1Ch when
    // the shooter's ref is latched for `task` (and bound to `target_plus_one`, unless 0),
    // else null and the caller computes as before.
    const float* aim_freeze_point(GameUnitSlot& shooter, std::size_t target_plus_one,
                                  std::uint8_t task);
    bool aim_point_freeze_009fada0(GameUnitSlot& shooter, const GameUnitSlot* target,
                                   std::size_t target_plus_one, float out[3],
                                   std::uint8_t task, GameHostLog* log_out = nullptr);
    // cc13-aimfreeze3 / cc13-aimfreezekz census (ON only), indexed by task 1..6: death-head freezes,
    // 009FAF90 double transforms, frozen answers, approach early-outs.
    unsigned long long aim_freeze_heads_[8]{}, aim_freeze_doubles_[8]{},
        aim_freeze_reads_[8]{}, aim_freeze_early_outs_[8]{};
    // cc13-kzpick: 009FA260 alone on the shooter's hull aim ref (the pick
    // 009ABF60's tail makes at 009AC09E), with no world point taken and no lead.
    void hull_aim_pick_009fa260(GameUnitSlot& shooter, const GameUnitSlot& target,
                                std::size_t target_plus_one);
    unsigned long long aim_lead_applied_{0};
    // Census for kAimLeadBodyAxisCc13Bound (cc9-nondet1, log only): leads taken in
    // aim_point_009fada0's tail, and those whose ON (body axis) lead point lies more
    // than 1e-3 m from the OFF (level heading) one.
    unsigned long long aim_lead_axis_calls_{0}, aim_lead_axis_differs_{0};
    // Census for kRocketTargetStopHandlersCc13Bound (cc9-nondet1, log only): task
    // installs, and those whose rocket block is not at its reset state.
    unsigned long long rocket_stop_installs_{0}, rocket_stop_not_reset_{0};
    // Packet cc9-rocket1 (log only): the 007B9480 feed's asks and true answers.
    unsigned long long rocket_feed_queries_{0}, rocket_feed_true_{0};

    // Packet cc9_aim_error_draw: 009C3DA0 (dive) / 009D02A0 (torpedo). The
    // three draws go through release_altitude_draw_00bd2f10, the stream-1
    // stand-in the approach constructor's draws already use. Returns the time
    // error, which the caller stores on its approach (+C8h or +9Ch).
    float redraw_aim_error(GameUnitSlot& slot, const bsp::ApproachAimErrorRow& row) {
        const float h = stream_set2_draw_00bd2f10(slot, slot.row.name, -row.h_error, row.h_error);
        const float v = stream_set2_draw_00bd2f10(slot, slot.row.name, -row.v_error, row.v_error);
        bsp::approach_target_ref_apply_aim_error(slot.hull_aim_ref, row, h, v);
        // The pick 009FA260 re-runs on the dirty byte; a fresh stand-in seed
        // gives it fresh box draws, as the image's stream would.
        slot.hull_aim_seed = ++g_hull_aim_pick_counter;
        slot.aim_error_drawn = true;
        const float t = stream_set2_draw_00bd2f10(slot, slot.row.name, -row.time_error,
                                                  row.time_error);
        ++aim_error_draws_;
        if (aim_error_draws_ <= 40) {
            log.notef("aim error draw %s: bias=(%.2f 0 %.2f) spread=%.2f section=%.2f "
                      "time=%.2f", slot.row.name.c_str(), static_cast<double>(h),
                      static_cast<double>(v), static_cast<double>(row.select_prec),
                      static_cast<double>(row.sets_sections ? row.section_chance : -1.0f),
                      static_cast<double>(t));
        }
        return t;
    }
    unsigned long long aim_error_draws_{0};
    // Packet cc9_torpedo_goaway_aim: goaway ticks that took 009D0670's point,
    // and how far it sat from the target's origin (XZ).
    unsigned long long goaway_aim_points_{0};
    // Packet cc9_fly_to_obstacles.
    unsigned long long fly_to_rebuilds_[2]{0, 0};
    unsigned long long fly_to_kept_[2]{0, 0};
    unsigned long long fly_to_avoid_ticks_[2]{0, 0};
    unsigned long long fly_to_side_flips_[2]{0, 0};
    // Packet cc9_p12_fly_to_world_edge (docs/OCEAN_WAVE_FIELD.md section 42):
    // 009FD570's world-edge pull, 009FDD58-009FDE8B. 00681F40(lead, 500.0
    // [00CE397C]) gates it; 009FA510(lead, 500.0) / 60.0 is the gain and the
    // centre is 0.5 x (world min + max) in XZ (009FDD8F-009FDDC5), over
    // GGame+711Ch/7124h/7128h/7130h = the Lua host's NW.x / NW.z / SE.x / SE.z.
    // False: near_edge stays false, as every host site forced it.
    static constexpr bool kFlyToWorldEdgeBound = true;  // ON: identical where reached, near arm not reached yet (docs/OCEAN_WAVE_FIELD.md 42.1)
    unsigned long long fly_to_edge_calls_{0}, fly_to_edge_near_{0};
    void fly_to_world_edge_009fdd62(bsp::FlyToSolverInputs& fin) {
        fin.world_edge.near_edge = false;
        if constexpr (!kFlyToWorldEdgeBound) return;
        const bsp::WorldMapBounds* b = lua.world_border_bounds();
        if (b == nullptr) return;
        ++fly_to_edge_calls_;
        float lead[3];
        for (int i = 0; i < 3; ++i) {
            // 009FD5A8-009FD61F, as the solver forms it.
            lead[i] = fin.unit_position[i] + static_cast<float>(
                static_cast<double>(fin.unit_lead_vector[i]) *
                static_cast<double>(bsp::fly_to_solver::kLeadSeconds));
        }
        const float m = 500.0f;
        const float x_min = b->north_west[0], z_max = b->north_west[2];
        const float x_max = b->south_east[0], z_min = b->south_east[2];
        // 00681F40 (00681F40-00681FB7): true below +711Ch + m or above +7128h - m
        // in x, or above +7124h - m or below +7130h + m in z.
        const bool near_hit = lead[0] < m + x_min || !(lead[0] <= x_max - m) ||
            !(lead[2] <= z_max - m) || !(z_min + m <= lead[2]);
        if (!near_hit) return;
        ++fly_to_edge_near_;
        fin.world_edge.near_edge = true;
        const bsp::cc13_seg68::WorldEdges e{x_min, z_max, x_max, z_min};
        fin.world_edge.penetration = bsp::cc13_seg68::ggame_world_edge_overrun_009fa510(
            e, lead, m);
        const float sx = x_max + x_min;                              // 009FDD8F-009FDD9B
        const float sz = z_min + z_max;                              // 009FDD9F-009FDDAB
        fin.world_edge.centre[0] = static_cast<float>(static_cast<double>(sx) * 0.5);
        fin.world_edge.centre[1] = static_cast<float>(static_cast<double>(sz) * 0.5);
    }
    // 00956C20's unit+460h + cat*4 for one unit, from the gunnery host's rows.
    float unit_category_damage_460(std::size_t unit_index, int category) const {
        if (gunnery == nullptr) return 0.0f;
        const std::vector<std::size_t>* list =
            gunnery->unit_category_guns(unit_index, category);
        if (list == nullptr) return 0.0f;
        const auto& guns = gunnery->guns();
        float sum = 0.0f;
        for (std::size_t g : *list) {
            if (g >= guns.size()) continue;
            const GameBulletClassRow* b = gunnery->bullet_class_row(guns[g].bullet_class);
            if (b == nullptr || !b->found) continue;
            sum += (b->damage_max + b->damage_min) * 0.5f;   // 00956E94..00956E9B
        }
        return sum;
    }
    // 007BCE20 (MaxWaterHitVel, class +DCh) and 007BCFA0 (WaterTravelSpeed,
    // class +E4h): the minimum over the unit's devices (unit+974h, count
    // +994h) whose bullet class +8h is 0Ah, the torpedo, and FLT_MAX
    // (00D7A248) when none is. The devices are the gunnery host's gun rows.
    float torpedo_device_min_007bce20(const GameUnitSlot& slot, bool water_travel) const {
        float best = FLT_MAX;
        if (gunnery == nullptr) return best;
        for (const auto& g : gunnery->guns()) {
            if (g.unit_index != slot.process_index || g.bullet_sub_type != 0x0A) continue;
            const GameBulletClassRow* b = gunnery->bullet_class_row(g.bullet_class);
            if (b == nullptr || !b->found) continue;
            const float v = water_travel ? b->water_travel_speed : b->max_water_hit_vel;
            if (v <= best) best = v;                                 // FCOMI / JA
        }
        return best;
    }
    // 009D1360 for `slot`'s torpedo approach. site 0 is the aim tick
    // (009D19A4), 1 the done/prepare tick (009D27D1).
    void torpedo_run_time_009d1360(GameUnitSlot& slot, int site) {
        bsp::TorpedoApproachState& ap = slot.torpedo_approach;
        const float vx = slot.motion.linear_velocity.x;
        const float vy = slot.motion.linear_velocity.y;
        const float vz = slot.motion.linear_velocity.z;
        bsp::TorpedoRunTimeInputs in;
        if constexpr (bsp::kPlaneCachedSpeedB1cCc13Bound) {
            // 009D138C CALL [vtable+38h] = 007B8E60 on a plane: FLD [unit+B1Ch],
            // the speed 007D8020 caches from ctl+18h (plane_world_velocity).
            in.unit_speed = bsp::plane_cached_speed_b1c_007d8020(slot.plane_world_velocity);
        } else {
        // 009D138C unit->vtable[38h], the speed unit_speed_vtable38 takes.
        in.unit_speed = static_cast<float>(std::sqrt(static_cast<double>(vx) * vx +
            static_cast<double>(vy) * vy + static_cast<double>(vz) * vz));
        }
        // 009D137D FLD [EDI+100h], the altitude, then 007BCC80 at 009D139D.
        in.fall_time = bsp::weapon_fall_time_007bcc80(slot.motion.position[1], vy);
        in.run_speed = torpedo_device_min_007bce20(slot, true);    // 009D13BF
        const bsp::TorpedoRunTimeResult r = bsp::torpedo_run_time_009d1360(ap, in);
        ap.fall_lead_a0 = r.fall_lead_a0;                           // 009D13B9
        ap.run_time_98 = r.run_time_98;                             // 009D14E7
        ++torpedo_run_time_updates_[site];
        torpedo_run_time_sum_[site] += r.run_time_98;
        torpedo_fall_lead_sum_[site] += r.fall_lead_a0;
        if (torpedo_run_time_updates_[0] + torpedo_run_time_updates_[1] <= 24) {
            log.notef("torpedo run time %s site=%d: alt=%.1f speed=%.2f fall=%.3f "
                "run=%.2f range=%.1f +A0h=%.1f +98h=%.3f (009D1360)",
                slot.row.name.c_str(), site,
                static_cast<double>(slot.motion.position[1]),
                static_cast<double>(in.unit_speed), static_cast<double>(in.fall_time),
                static_cast<double>(in.run_speed), static_cast<double>(ap.range_90),
                static_cast<double>(r.fall_lead_a0), static_cast<double>(r.run_time_98));
        }
    }
    unsigned long long torpedo_run_time_updates_[2]{0, 0};
    double torpedo_run_time_sum_[2]{0.0, 0.0};
    double torpedo_fall_lead_sum_[2]{0.0, 0.0};
    unsigned long long torpedo_reset_draws_{0};
    unsigned long long torpedo_reset_seeds_{0};
    double torpedo_reset_seed_sum_{0.0};
    // Packet cc13-goaway: 007B5BE0 on a goaway target, as the dive (009C49B4),
    // torpedo (009D0DF2) and level (009B56DF) enters call it. Every host slot
    // answers kind 5; the fields are the gunnery row's category 6 / 5 / 1
    // ranges (unit+448h, +444h, +434h). False when there is no target or no
    // gunnery row.
    bool goaway_target_extent_007b5be0(std::size_t target_plus_one, float& extent) {
        if (gunnery == nullptr || target_plus_one == 0) return false;
        const std::size_t ti = target_plus_one - 1;
        if (ti >= slots.size() || !slots[ti]) return false;
        const std::size_t idx = slots[ti]->process_index;
        for (const GameGunneryUnitRow& r : gunnery->unit_rows()) {
            if (r.unit_index != idx) continue;
            // 007B5BE0: unit_largest_extent_007b5be0 over the target's gunnery row
            // ranges (+434h, +444h, +448h); OFF keeps the inline max.
            extent = bsp::cc13_seg47::unit_largest_extent_007b5be0(
                r.category_ranges[1], r.category_ranges[5], r.category_ranges[6]);
            return true;
        }
        return false;
    }
    // The same, at a goaway enter: measures the term against the enter's base
    // (dive 2 * approach+B4h, torpedo SafeDist +438h, level SafeDist +450h)
    // into the slot's ga_term_* counters, and returns what the enter feeds its
    // has_extent_target / target_extent when its switch is on.
    bool goaway_target_term(GameUnitSlot& flyer, const char* kind,
                            std::size_t target_plus_one, float base, float& extent) {
        const bool has = goaway_target_extent_007b5be0(target_plus_one, extent);
        if (flyer.ga_term_enters == 0 || base < flyer.ga_term_base_min) flyer.ga_term_base_min = base;
        if (flyer.ga_term_enters == 0 || base > flyer.ga_term_base_max) flyer.ga_term_base_max = base;
        flyer.ga_term_kind = kind;
        ++flyer.ga_term_enters;
        if (has) {
            ++flyer.ga_term_targets;
            if (extent > flyer.ga_term_aa_max) flyer.ga_term_aa_max = extent;
            if (extent > base) ++flyer.ga_term_moved;
        }
        return has;
    }
    // 009FD5FF-009FD743 for the cache `k` of `flyer`, with `lead` the solver's
    // lead point (unit+FCh + 3.0 * vtable[34h]).
    const std::vector<bsp::FlyToObstacle>& fly_to_obstacles_009fd63c(
            GameUnitSlot& flyer, int k, const float lead[3]) {
        if (--flyer.fly_to_countdown[k] >= 0) return flyer.fly_to_cache[k];  // JNS
        flyer.fly_to_countdown[k] = 20;                                      // 009FD630
        std::vector<bsp::FlyToObstacle>& out = flyer.fly_to_cache[k];
        out.clear();                                                         // 007B4500(0)
        ++fly_to_rebuilds_[k];
        if (gunnery == nullptr) return out;
        const auto& gunnery_rows = gunnery->unit_rows();
        for (const GameUnitWorldNode* node = world_lists.entries[6].head; node != nullptr;
             node = node->next) {
            const GameUnitSlot* u = node->unit;
            if (u == nullptr || u->row.party == flyer.row.party) continue;  // 009FD656
            const std::size_t idx = u->process_index;
            const GameGunneryUnitRow* row = nullptr;
            for (const auto& r : gunnery_rows) {
                if (r.unit_index == idx) { row = &r; break; }
            }
            if (row == nullptr) continue;
            const float dx = lead[0] - u->motion.position[0];
            const float dz = lead[2] - u->motion.position[2];
            const float d2 = dx * dx + 0.0f * 0.0f + dz * dz;               // 009FD6A3 FLDZ
            // 009FD6B1-009FD6D3: max(+448h, +444h) then against +434h.
            float m = std::max(row->category_ranges[6], row->category_ranges[5]);
            m = std::max(row->category_ranges[1], m);
            const float r = static_cast<float>(static_cast<double>(m) + 100.0);  // 00D7A220
            if (!(r * r > d2)) continue;                                     // 009FD6F9 JBE
            bsp::FlyToObstacle o;
            o.position[0] = u->motion.position[0];
            o.position[1] = u->motion.position[1];
            o.position[2] = u->motion.position[2];
            o.extent_max = m;
            o.extent_sum = unit_category_damage_460(idx, 5)
                + unit_category_damage_460(idx, 1) + unit_category_damage_460(idx, 6);
            out.push_back(o);
            ++fly_to_kept_[k];
        }
        return out;
    }
    double goaway_aim_offset_sum_{0.0};
    double goaway_aim_offset_max_{0.0};

    // Packet cc9_plane_desc_158. 007D473A, in BSP_PlaneClass_BindModelData_Provisional
    // (007D3E60, EBP = the class descriptor from 007D3E7F, never rewritten):
    // model = [desc+50h] (007D470E); FLD model+30h, FSTP float; FLD model+3Ch,
    // FSTP float, FLD, FADD the first, FMUL double 0.5 (00D7A280), FSTP float to
    // desc+158h. The model box is local min xyz, max xyz at +28h..+3Ch, from the
    // model's top-level `BoundingBox` (00B7F525..00B7F55C,
    // docs/UNIT_HULL_EXTENTS.md), so this is the box centre along z.
    // SUBSTITUTIONS, labelled: the class model is the vehicle class's `Mesh` file
    // (as the section points and the buoyancy list read it); a model without a
    // BoundingBox, or that does not open, gives 0.0.
    // Packet cc9_plane_water_law. The class fields the water law reads, once per
    // class: desc+180h WaterRotAccel, +184h StallSpd, +194h SwimHeight, +198h
    // MinWaterSpd, +1A0h WaterDecel, +204h WaterPitch from the vehicle class, and
    // desc+508h = -model min y - SwimHeight (007C4CF8-007C4D03; [desc+50h]+2Ch is
    // the class model's BoundingBox min y, read from the Mesh file as
    // plane_desc_158 reads it). SUBSTITUTION, labelled: a model without a
    // BoundingBox gives min y 0.
    std::map<int, bsp::PlaneWaterLawClass> plane_water_class_by_type_;
    const bsp::PlaneWaterLawClass& plane_water_class(const GameUnitSlot& p) {
        auto found = plane_water_class_by_type_.find(p.row.type_id);
        if (found != plane_water_class_by_type_.end()) return found->second;
        bsp::PlaneWaterLawClass& c = plane_water_class_by_type_[p.row.type_id];
        const int t = p.row.type_id;
        c.min_water_spd_198 = p.plane_min_water_spd_198;
        c.swim_height_194 = p.plane_swim_height;
        c.stall_spd_184 = p.plane_stall_spd;
        c.water_rot_accel_180 = lua.read_vehicle_class_number(t, "WaterRotAccel", 0.0f);
        c.water_decel_1a0 = lua.read_vehicle_class_number(t, "WaterDecel", 0.0f);
        c.water_pitch_204 = lua.read_vehicle_class_number(t, "WaterPitch", 0.0f);
        const std::string mesh = lua.read_vehicle_class_string(t, "Mesh");
        std::vector<std::uint8_t> bytes;
        std::array<float, 6> box{};
        const bool boxed = !mesh.empty() && lua.read_resource_file(mesh, bytes)
            && bsp::read_mmod_bounding_box(bytes, box);
        c.derived_508 = -(boxed ? box[1] : 0.0f) - c.swim_height_194;
        log.notef("plane water class %d %s: min_y=%.3f%s SwimHeight=%.3f desc508=%.3f "
            "MinWaterSpd=%.3f WaterRotAccel=%.3f WaterDecel=%.3f WaterPitch=%.4f "
            "StallSpd=%.3f (packet cc9_plane_water_law)", t, mesh.c_str(),
            static_cast<double>(box[1]), boxed ? "" : " (no box)",
            static_cast<double>(c.swim_height_194), static_cast<double>(c.derived_508),
            static_cast<double>(c.min_water_spd_198), static_cast<double>(c.water_rot_accel_180),
            static_cast<double>(c.water_decel_1a0), static_cast<double>(c.water_pitch_204),
            static_cast<double>(c.stall_spd_184));
        return c;
    }
    bsp::PlaneWaterLawTuning plane_water_tuning() {
        bsp::PlaneWaterLawTuning t;
        if (lua.plane_globals_loaded()) {
            const bsp::GameTuningBlock& g = lua.plane_globals();
            t.lift_depth_ratio = g.dynamics_water_lift_depth_ratio;
            t.lift_max = g.dynamics_water_lift_max;
            t.decel_speed = g.dynamics_water_decel_speed;
            t.side_drag_ratio = g.dynamics_water_side_drag_ratio;
            t.max_side_drag = g.dynamics_water_max_side_drag;
            t.min_drag_spd = g.dynamics_water_min_drag_spd;
            t.lift_begin_dive_mul = g.dynamics_water_lift_begin_dive_mul;
            t.lift_rotate_max = g.dynamics_water_lift_rotate_max;
            t.lift_rotate_depth_ratio = g.dynamics_water_lift_rotate_depth_ratio;
            t.lift_rotate_angle_ratio = g.dynamics_water_lift_rotate_angle_ratio;
            t.accel_cheat_mul = g.dynamics_accel_cheat_mul;
        }
        return t;
    }
    std::map<int, float> plane_desc_158_by_class_;
    float plane_desc_158(const GameUnitSlot& p) {
        if constexpr (!kPlaneDesc158Bound) {
            (void)p;
            return 0.0f;
        } else {
            const int type_id = p.row.type_id;
            auto found = plane_desc_158_by_class_.find(type_id);
            if (found != plane_desc_158_by_class_.end()) return found->second;
            float& value = plane_desc_158_by_class_[type_id];
            value = 0.0f;
            const std::string mesh = lua.read_vehicle_class_string(type_id, "Mesh");
            std::vector<std::uint8_t> bytes;
            std::array<float, 6> box{};
            const char* why = "ok";
            if (mesh.empty()) {
                why = "no Mesh string";
            } else if (!lua.read_resource_file(mesh, bytes)) {
                why = "model did not open";
            } else if (!bsp::read_mmod_bounding_box(bytes, box)) {
                why = "no BoundingBox";
            } else {
                const float min_z = box[2];                              // model+30h
                const float max_z = box[5];                              // model+3Ch
                value = static_cast<float>((static_cast<double>(max_z)
                    + static_cast<double>(min_z)) * 0.5);                // 007D4730-007D473A
            }
            log.notef("plane desc 158h: class %d %s z=[%.3f %.3f] -> %.3f (%s; 007D473A, "
                "packet cc9_plane_desc_158)", type_id, mesh.c_str(),
                static_cast<double>(box[2]), static_cast<double>(box[5]),
                static_cast<double>(value), why);
            return value;
        }
    }

    // Packet cc9_ship_section_points: 0081F980's section records per vehicle
    // class, from the class's Mesh model (the same read the buoyancy list and
    // the gunnery host's section points make).
    std::map<int, bsp::ShipLeadSections> section_points_by_class_;
    const bsp::ShipLeadSections& class_section_points_0081f980(int type_id) {
        auto found = section_points_by_class_.find(type_id);
        if (found != section_points_by_class_.end()) return found->second;
        bsp::ShipLeadSections& entry = section_points_by_class_[type_id];
        entry.engine_room.id = 5;
        entry.magazine.id = 8;
        entry.fuel_tank.id = 6;
        const std::string mesh = lua.read_vehicle_class_string(type_id, "Mesh");
        std::vector<std::uint8_t> bytes;
        if (mesh.empty() || !lua.read_resource_file(mesh, bytes)) return entry;
        std::vector<bsp::GeomMeshResourcePayload> meshes;
        std::string error;
        bsp::read_mmod_geom_meshes(bytes, meshes, error);
        entry = bsp::ship_section_points_0081f980(meshes);
        log.notef("section points class %d %s: engine_room %d (%.1f %.1f %.1f) "
            "magazine %d (%.1f %.1f %.1f) fuel_tank %d (%.1f %.1f %.1f) (0081F980)",
            type_id, mesh.c_str(),
            entry.engine_room.present ? 1 : 0,
            static_cast<double>(entry.engine_room.point[0]),
            static_cast<double>(entry.engine_room.point[1]),
            static_cast<double>(entry.engine_room.point[2]),
            entry.magazine.present ? 1 : 0,
            static_cast<double>(entry.magazine.point[0]),
            static_cast<double>(entry.magazine.point[1]),
            static_cast<double>(entry.magazine.point[2]),
            entry.fuel_tank.present ? 1 : 0,
            static_cast<double>(entry.fuel_tank.point[0]),
            static_cast<double>(entry.fuel_tank.point[1]),
            static_cast<double>(entry.fuel_tank.point[2]));
        return entry;
    }
    // The target's records plus 0093A570 per section. See
    // kApproachSectionPointsBound for the name substitution.
    bsp::ApproachTargetSections approach_target_sections(std::size_t target_index,
                                                         const GameUnitSlot& target) {
        bsp::ApproachTargetSections out;
        out.sections = class_section_points_0081f980(target.row.type_id);
        if (gunnery != nullptr) {
            out.engine_room_available = !gunnery->unit_failure_active(target_index, "EngineJam");
            out.magazine_available = !gunnery->unit_failure_active(target_index, "Explosion");
            out.fuel_tank_available = !gunnery->unit_failure_active(target_index, "Fire");
        }
        return out;
    }

    static float pose_heading_radians(const GameUnitSlot& slot) {
        return static_cast<float>(std::atan2(static_cast<double>(slot.motion.pose_row2[0]),
            static_cast<double>(slot.motion.pose_row2[2])));
    }
};

// The host represents the shared squadron director on each member. Queue one
// survivor order per live plane and retire its retreat task at delivery, so
// wing-0 death cannot strand the command on the fused squadron identity.
std::size_t queue_squadron_survivor_moveto(GameCommandsHost& commands, GameGunneryHost& gunnery,
    const std::vector<GameUnitSlot*>& members, std::size_t leaving_plane,
    const std::array<float, 3>& point);

inline namespace units_local {

// ---------------------------------------------------------------------------
// 0078cf20's two leaves, and the gameplay-modifier filter
// ---------------------------------------------------------------------------

// bsp::OceanHeightHost. The sampler itself is recovered and runs; what is not
// available is its receiver. 0078cf20 takes [[00e188a8]+19F0h] and hands
// [world+A8h] to both leaves, and that sub-object is the renderer/scene owner's
// (the foliage builder 00af0c50 takes its camera from the same pointer, and
// `construct_world` 004de610 is still a load record), so each leaf is a host
// record that answers the open-sea state docs/OCEAN_HEIGHT.md evidences: the
// wave field returns exactly 0.0f when field+F9h is set or field+24h is zero,
// and the coverage mask returns exactly 1.0f when no region covers the point.
// Packet cc9_ocean_waves, docs/OCEAN_WAVE_FIELD.md. True: both leaves run their
// reconstructed rules over the field state this process can establish: amplitude
// field+24h = 0.0f (00BA19C9; the per-frame store 00BA0B93 is bound separately by
// kWaveFieldLiveAmplitudeBound, OCEAN_WAVE_FIELD.md section 9), +B4h = 1/100 (00B9A4C0 over the
// 004CB420 default tile 100.0 at authored +30h), +F9h clear, no coverage regions
// (00BA0C00 / 00BA1370 build them from bitmaps this process does not load) and a
// labelled zero grid for the field+BCh spectrum. False: the records, as before.
constexpr bool kOceanWaveFieldBound = true;

class OceanFieldBinding final : public bsp::OceanHeightHost {
public:
    explicit OceanFieldBinding(GameUnitsHost::Impl& owner) : owner_(owner) {}

    float wave_height_0078c890(float x, float z) override {
        if (!owner_.logged_ocean) {
            owner_.logged_ocean = true;
            owner_.log.notef("ocean sampler 0078cf20 runs over [world+A8h]'s field: amplitude "
                "+24h 0.0f (00ba19c9, its only store), tile 100.0 (004cb420 default), +F9h "
                "clear, no coverage regions, a labelled zero grid for the +BCh spectrum; "
                "0078c890 answers h * 0.0f and the product 0078cf64 is 0.0f");
        }
        if (!kOceanWaveFieldBound) {
            owner_.record("ShipMotion::ocean_wave_field", 0x0078c890u);
            return 0.0f;
        }
        // Packet cc9_physics1_sea (docs/OCEAN_WAVE_FIELD.md section 9): the live
        // field the render step 00BA0090 keeps, amplitude +24h = HeightScale
        // (00BA0B93) over the stepped 00B95D30 grid. Off, or before a scene was
        // published, the flat sea below stands.
        if (bsp::ocean_live_field_ready()) {
            const bsp::OceanLiveField& live = bsp::ocean_live_field_process();
            if (!owner_.logged_ocean_live || (live.render_steps >= 1000u && !owner_.logged_ocean_live_1000)) {
                owner_.logged_ocean_live = true;
                if (live.render_steps >= 1000u) owner_.logged_ocean_live_1000 = true;
                owner_.log.notef("ocean live field: render steps %u amplitude +24h %.3f "
                    "tile %.1f +F9h %d clock %.3f sample(%.1f,%.1f) %.4f",
                    live.render_steps, static_cast<double>(live.field.amplitude_24),
                    static_cast<double>(1.0f / live.field.inv_tile_b4),
                    live.field.flat_f9 ? 1 : 0, static_cast<double>(live.clock_01090494),
                    static_cast<double>(x), static_cast<double>(z),
                    static_cast<double>(bsp::ocean_wave_field_sample_0078c890(live.field,
                        bsp::ocean_spectrum_grid(live.spectrum), x, z)));
            }
            owner_.done("ShipMotion::ocean_wave_field", 0x0078c890u);
            return bsp::ocean_wave_field_sample_0078c890(live.field,
                bsp::ocean_spectrum_grid(live.spectrum), x, z);
        }
        bsp::OceanWaveFieldState field;
        field.flat_f9 = false;        // [[game+5FCh]+0C20h]: no scene record here
        field.inv_tile_b4 = static_cast<float>(1.0 / 100.0);   // FLD1; FDIVRP at 00B9A4D2
        field.amplitude_24 = 0.0f;    // 00BA19C9
        const bsp::OceanWaveGridView zero_grid{};  // LABELLED: the spectrum is not run
        owner_.done("ShipMotion::ocean_wave_field", 0x0078c890u);
        return bsp::ocean_wave_field_sample_0078c890(field, zero_grid, x, z);
    }
    float coverage_mask_00b9cf50(float x, float z) override {
        if (!kOceanWaveFieldBound) {
            owner_.record("ShipMotion::ocean_coverage_mask", 0x00b9cf50u);
            return 1.0f;
        }
        owner_.done("ShipMotion::ocean_coverage_mask", 0x00b9cf50u);
        // Packet cc9_physics1_sea: the landscapes' wave-damp regions (008830AA ->
        // 00BA1370); 1.0 everywhere while the live sea is off.
        return bsp::ocean_live_field_coverage_mask(x, z);
    }

private:
    GameUnitsHost::Impl& owner_;
};

// Packet cc13-para. The host side of bsp::gameplay_seg46_cc13's rack models for a
// level bomber's 31h rack. paratrooper_drop_target_006e541f reaches only uniform()
// (its two stream-0 draws, 006E5479 / 006E549E); the rest of the interface is the
// MParatrooper round's, which the host does not spawn (recorded gap, packet
// cc13-para), so those slots record the call and return the type's zero.
class ParatrooperRackBinding final : public bsp::gameplay_seg46_cc13::ParatrooperHost {
public:
    using Vec3 = bsp::gameplay_seg46_cc13::Vec3;
    using State = bsp::gameplay_seg46_cc13::ParatrooperState;
    ParatrooperRackBinding(GameUnitsHost::Impl& owner, std::size_t unit_index)
        : owner_(owner), unit_index_(unit_index) {}
    // 00BD2F10(ECX = stream, lo, hi). Without bsp::kLevelBombParatrooperDropDrawBound
    // no draw is taken and the jitter is 0 (the target is the rack point).
    float uniform(int stream, float lo, float hi) override {
        if constexpr (bsp::kLevelBombParatrooperDropDrawBound) {
            if (owner_.gunnery) {
                return owner_.gunnery->death_mode_draw_00bd2f10(stream, unit_index_, lo, hi);
            }
        }
        owner_.record("Rack::paratrooper_drop_jitter_draw_untaken", 0x006e5479u);
        return 0.0f;
    }
    // game+1FE4h (007AB294 CMP [EAX+1FE4h], 1). Packet cc13-mp4 (MP2b, rev9 N1): from
    // the session context under the switch; a campaign's word is 0. Campaign
    // identity: the OFF arm is the old literal 0.
    int session_mode() const override {
        if constexpr (bsp::kSkirmishSessionCc13Bound) {
            return bsp::session_context().session_word_1fe4;
        } else {
            return 0;
        }
    }
    // game+218Ch from the session context (packet cc13-mp1). Campaign identity: the
    // OFF arm is the old literal false.
    bool session_flag_218c() const override {
        if constexpr (bsp::kSkirmishSessionCc13Bound) {
            return bsp::session_context().skirmish_218c;
        } else {
            return false;
        }
    }
    Vec3 world_position() override { return unreached(Vec3{}); }
    float ground_height_at(const Vec3&) override { return unreached(0.0f); }
    bool ground_normal_at(const Vec3&, Vec3&) override { return unreached(false); }
    void avoid_zone_nearest_boundary(float[2], float, float) override { unreached(0); }
    void bomb_advance_in_air(State&, float) override { unreached(0); }
    void extrapolate_position(State&, float) override { unreached(0); }
    void orthonormalize_upright() override { unreached(0); }
    bool chute_anim_present() override { return unreached(false); }
    void set_chute_anim(std::uint32_t, float) override { unreached(0); }
    void kill_self(int) override { unreached(0); }
    void kill_entity(std::uint32_t, int) override { unreached(0); }
    void on_killed_lua() override { unreached(0); }
    int building_side(std::uint32_t) override { return unreached(0); }
    float building_368(std::uint32_t) override { return unreached(0.0f); }
    void building_damage(std::uint32_t, float) override { unreached(0); }
    Vec3 building_position(std::uint32_t) override { return unreached(Vec3{}); }
    int party_relative(int, int) override { return unreached(0); }
    std::uint32_t find_enemy_building(const Vec3&, int) override { return unreached(0u); }
    void register_fixed_step_once() override { unreached(0); }
    void add_capturer(std::uint32_t) override { unreached(0); }
    void remove_capturer(std::uint32_t) override { unreached(0); }
    bool soldier_clip_visible(std::uint32_t, const Vec3&) override { return unreached(false); }
    void soldier_show(std::uint32_t) override { unreached(0); }
    void soldier_muzzle_effect(std::uint32_t) override { unreached(0); }
    void send_landing_message_71(const Vec3&) override { unreached(0); }
    bool dropper_from_owner(std::int32_t&) override { return unreached(false); }

private:
    template <typename T>
    T unreached(T zero) {
        owner_.record("MParatrooper::round_not_modelled", 0x007ab8c0u);
        return zero;
    }
    GameUnitsHost::Impl& owner_;
    std::size_t unit_index_;
};

// Packet cc13-para2: bsp::cc13_paratrooper::RoundServices over this host. The
// draw is 00BD2F10 by stream (the callers gate it on their sub-switches); the
// ground is 00903860 / 009038F0; the avoid zones are the ship AI host's runtime
// (layer 0, 004120D0(0)); the building list is world list 28 (the class 1Ch
// list [[game+19CCh]+16Ch], 006F2C30); armour and the +1ACh hit are the ship AI
// host's capture buildings, then 0095DA00 through the gunnery host, as the
// landed remainder bleed does. A handle is the unit index + 1.
class ParaServices final : public bsp::cc13_paratrooper::RoundServices {
public:
    using Vec3 = bsp::gameplay_seg46_cc13::Vec3;
    explicit ParaServices(GameUnitsHost::Impl& owner) : owner_(owner) {}
    // `key` (the round's or the hang's band, bsp::cc13_paratrooper) is read only
    // under BSP_GUNNERY_RNG_STREAMS=1; the default mode ignores it.
    float uniform(int stream, std::uint32_t key, float lo, float hi) override {
        if (!owner_.gunnery) return lo;
        return owner_.gunnery->death_mode_draw_00bd2f10(stream, key, lo, hi);
    }
    float ground_height_00903860(const Vec3& p) override {
        const float point[3] = {p.x, p.y, p.z};
        float out = -1000.0f;
        if (!world_ground_height_00903860(point, out)) return -1000.0f;
        return out;
    }
    bool ground_normal_009038f0(const Vec3& p, Vec3& normal) override {
        const float point[3] = {p.x, p.y, p.z};
        float n[3] = {0.0f, 1.0f, 0.0f};
        if (!world_ground_normal_009038f0(point, n)) return false;
        normal = Vec3{n[0], n[1], n[2]};
        return true;
    }
    bool avoid_zone_nearest_0041b840(float xz[2], float slack, float push) override {
        GameAvoidZoneRuntime* zones =
            owner_.ship_ai ? owner_.ship_ai->avoid_zone_runtime_mutable() : nullptr;
        if (zones == nullptr || !zones->ready()) return false;
        const std::uint32_t group = zones->group_for_layer(0);
        if (group == 0) return false;
        const std::array<float, 2> out = zones->nearest(group, {xz[0], xz[1]}, slack, push);
        xz[0] = out[0];
        xz[1] = out[1];
        return true;
    }
    std::size_t building_count() override {
        refresh();
        return list_.size();
    }
    bsp::cc13_paratrooper::BuildingView building(std::size_t index) override {
        refresh();
        bsp::cc13_paratrooper::BuildingView v;
        if (index >= list_.size()) return v;
        const GameUnitSlot& u = *owner_.slots[list_[index]];
        v.handle = static_cast<std::uint32_t>(list_[index] + 1);
        v.side_54 = u.row.party;
        v.position_fc = Vec3{u.motion.position[0], u.motion.position[1], u.motion.position[2]};
        v.inferior_range_7c8 = u.inferior_range_7c8;
        return v;
    }
    int building_side_54(std::uint32_t handle) override {
        const GameUnitSlot* u = slot(handle);
        return u ? u->row.party : 0;
    }
    float building_armour_368(std::uint32_t handle) override {
        float armour = 0.0f;
        if (owner_.ship_ai && slot(handle) != nullptr)
            owner_.ship_ai->command_building_armour_0368(handle - 1, armour);
        return armour;
    }
    void building_damage_1ac(std::uint32_t handle, float amount) override {
        if (slot(handle) == nullptr) return;
        const std::size_t index = handle - 1;
        if (owner_.ship_ai && !owner_.ship_ai->command_building_hit_gate_006f1f20(index, amount))
            return;
        if (owner_.gunnery) owner_.gunnery->apply_script_damage_0095da00(index, amount);
    }
    Vec3 building_position_fc(std::uint32_t handle) override {
        const GameUnitSlot* u = slot(handle);
        if (u == nullptr) return Vec3{};
        return Vec3{u->motion.position[0], u->motion.position[1], u->motion.position[2]};
    }
    void record(const char* name, std::uint32_t address) override { owner_.record(name, address); }
    void note(const std::string& line) override { owner_.log.notef("%s", line.c_str()); }

private:
    const GameUnitSlot* slot(std::uint32_t handle) const {
        if (handle == 0 || handle > owner_.slots.size()) return nullptr;
        return owner_.slots[handle - 1].get();
    }
    // World list 28 in list order, taken once per services object (one step).
    void refresh() {
        if (listed_) return;
        listed_ = true;
        for (const GameUnitWorldNode* node = owner_.world_lists.entries[28].head; node != nullptr;
             node = node->next) {
            if (node->unit != nullptr) list_.push_back(node->unit->process_index);
        }
    }
    GameUnitsHost::Impl& owner_;
    std::vector<std::size_t> list_;
    bool listed_{false};
};

// bsp::GameplayModifierHost. 008e6430 walks the list at manager+80h+category*0Ch
// and this process registers no modifier record, so the walk visits nothing and
// the filter is never reached; the product is the routine's own 1.0f at
// 00d7a24c rather than a literal a host returns.
class NoGameplayModifiers final : public bsp::GameplayModifierHost {
public:
    bool entry_matches_008e4680(const bsp::GameplayModifierEntry&) override { return false; }
};

// ---------------------------------------------------------------------------
// bsp::UnitInstanceHost, one method per native call site of 008255b0
// ---------------------------------------------------------------------------

// Packet cc9_unit_instance_step11 (docs/UNIT_INSTANCE_STEP11.md). ON runs
// 00815AA0, 0081C050, 008252C0, 00956600 and 00834E90 (with its tails 00834820,
// 00834CC0, 00834A70) against this host's units, which carry no scene nodes,
// emitters or effect handles; OFF keeps the six records. 008227E0 stays a record.
constexpr bool kUnitInstanceStep11Bound = true;

class UnitInstanceBinding final : public bsp::UnitInstanceHost {
public:
    UnitInstanceBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot)
        : owner_(owner), slot_(slot) {}

    // 00b6db70 on the scene node at +4A4h. 00928860, the placement that would
    // give a created unit a scene node, is a milestone 2h record, so the state
    // carries has_scene_node = false and this is unreachable; it is here because
    // the interface has no default.
    float refresh_scene_node_world_matrix() override {
        owner_.record("UnitInstance::refresh_scene_node", 0x00b6db70u);
        return 0.0f;
    }
    void release_submerged_effect(std::size_t) override {
        owner_.record("UnitInstance::release_submerged_effect", 0x00867b10u);
    }
    // The ocean object's own world Y, which step 2 of 008255b0 gates on. That is
    // the object's pose, not a sample of the sampler, and its producer is the
    // renderer/scene owner; a surface at y = 0 leaves the bubble block alone,
    // which is why it is a stated contract rather than a claim.
    bool ocean_exists() override { return true; }
    float ocean_world_y() override { return 0.0f; }
    float draw_bubble_interval() override {
        owner_.record("UnitInstance::draw_bubble_interval", 0x00bd2f10u);
        return 0.0f;
    }
    void spawn_bubble_effect() override {
        owner_.record("UnitInstance::spawn_bubble_effect", 0x008687c0u);
    }
    // 0092d730 on the controller at +1018h: the body-axis forward speed, which
    // is reconstructed, so this is the recovered value rather than a record.
    float controller_time_value() override {
        bsp::UnitBodyAxisSpeedInputs in{};
        in.velocity[0] = slot_.motion.linear_velocity.x;
        in.velocity[1] = slot_.motion.linear_velocity.y;
        in.velocity[2] = slot_.motion.linear_velocity.z;
        in.axis[0] = slot_.motion.pose_row2[0];
        in.axis[1] = slot_.motion.pose_row2[1];
        in.axis[2] = slot_.motion.pose_row2[2];
        owner_.done("UnitInstance::controller_body_axis_speed", 0x0092d730u);
        return bsp::unit_forward_speed_0092d730(in);
    }
    // 00815aa0 is reconstructed as publish_unit_effect_intensity_00815aa0, but
    // it publishes into the effect groups at unit+FFCh, and a created unit has
    // none: 006fe590, the instance the descriptor's vtable +28h allocates, is a
    // milestone 2h record and nothing fills those groups.
    void sub_update_00815aa0(float gate) override {
        if (!kUnitInstanceStep11Bound || slot_.state == nullptr) {
            owner_.record("UnitInstance::publish_effect_intensity", 0x00815aa0u);
            return;
        }
        // Run whole against a unit with no effect groups (header of
        // bsp/unit_instance_step11.hpp): only the latch at +9D0h moves.
        bsp::run_effect_intensity_00815aa0(*slot_.state, slot_.step11, step11_inputs(), gate);
        owner_.done("UnitInstance::publish_effect_intensity", 0x00815aa0u);
    }
    bsp::UnitAnchorPoint transform_bow_anchor() override {
        owner_.record("UnitInstance::transform_bow_anchor", 0x00413920u);
        return {};
    }
    bsp::UnitAnchorPoint transform_stern_anchor() override {
        owner_.record("UnitInstance::transform_stern_anchor", 0x00413920u);
        return {};
    }
    // The same 0078cf20 the motion tick's wave gate calls, run whole.
    float sample_ocean_height(float x, float z) override {
        OceanFieldBinding sea(owner_);
        const float height = bsp::ocean_water_height_0078cf20(x, z, sea);
        owner_.done("UnitInstance::ocean_height", 0x0078cf20u);
        return height;
    }
    void publish_bow_anchor(const bsp::UnitAnchorPoint&) override {
        owner_.record("UnitInstance::publish_bow_anchor", 0x004842c0u);
    }
    void publish_stern_anchor(const bsp::UnitAnchorPoint&) override {
        owner_.record("UnitInstance::publish_stern_anchor", 0x004842c0u);
    }
    // 0092be80 on the controller, whose whole recovered body is one RET
    // (docs/UNIT_CONTROLLER_UPDATE.md). Running it is the point: the negative
    // result is what the reconstruction records.
    void sub_update_0092be80(float scaled_delta) override {
        bsp::unit_controller_update_0092be80(slot_.controller, scaled_delta);
        owner_.done("UnitInstance::controller_update", 0x0092be80u);
    }
    void sub_update_0081c050() override {
        if (!kUnitInstanceStep11Bound) {
            owner_.record("UnitInstance::sub_update_0081c050", 0x0081c050u);
            return;
        }
        // The finished-effect prune over +FFCh, which this host never fills.
        bsp::run_prune_effects_0081c050();
        owner_.done("UnitInstance::prune_finished_effects", 0x0081c050u);
    }
    bool is_controlled_unit() override { return owner_.is_controlled(slot_); }
    bool global_intensity_override() override { return false; }  // 00f87152
    void smooth_intensity_008227e0(float scaled_delta) override {
        // Packet cc9_damage_smoke_draws (docs/GUNNERY_OPEN_ITEMS.md 127): the
        // gunnery host keeps the damage-smoke controller (unit+10A0h) and its
        // draws; it answers false while kDamageSmokeDrawsBound is off.
        GameGunneryHost* const gun = owner_.gunnery.get();
        if (gun != nullptr
            && gun->damage_smoke_tick_008227e0(slot_.process_index, scaled_delta)) {
            owner_.done("UnitInstance::smooth_intensity", 0x008227e0u);
            return;
        }
        owner_.record("UnitInstance::smooth_intensity", 0x008227e0u);
    }
    bool wake_enabled() override {
        // Packet s53_wreck_debris: 00825AF4 00424C40, 00825AF9 settings+680h.
        if (bsp::game::kSinkingWreckDebrisDrawsBound) {
            owner_.done("UnitInstance::wake_setting", 0x00424c40u);
            return owner_.debris_enabled_settings_680();
        }
        owner_.record("UnitInstance::wake_setting", 0x00424c40u);
        return false;
    }
    void spawn_wake() override {
        // Packet s53_wreck_debris: 00825B3E..00825BCF, ECX = unit+C4h and the
        // hull body's local box extents (00C31F90: B+38h min, B+44h max); arg4 is
        // the z extent. LABELLED: the class hull box the body was created from
        // stands for B+38h..+4Ch; with no box, the class hull length.
        GameGunneryHost* const gun = owner_.gunnery.get();
        if (!bsp::game::kSinkingWreckDebrisDrawsBound || gun == nullptr) {
            owner_.record("UnitInstance::spawn_wake", 0x00935540u);
            return;
        }
        const GameUnitsHost::Impl::ClassHullBox& hb = owner_.class_hull_box(slot_);
        const float z_extent = (hb.ok && hb.box.shape_count > 0)
            ? hb.box.max.z - hb.box.min.z : slot_.motion_class.hull_length;
        gun->sinking_wreck_debris_draws_00935540(slot_.process_index, slot_.state->class_id,
            z_extent, owner_.debris_num_per_meter_684);
        owner_.done("UnitInstance::spawn_wake", 0x00935540u);
    }
    bool prop_wash_threshold_passed(float) override {
        owner_.record("UnitInstance::prop_wash_threshold", 0x00ce69d0u);
        return false;
    }
    void stop_prop_wash() override {
        owner_.record("UnitInstance::stop_prop_wash", 0x00b6da70u);
    }
    void update_attachment(std::size_t, const bsp::UnitAnchorPoint&) override {
        owner_.record("UnitInstance::update_attachment", 0x008689c0u);
    }
    // The three timed sub-updates of step 11. docs/UNIT_TIMED_SUBUPDATES.md
    // analyses them; packet cc_unit_subupdates owns their reconstruction.
    void sub_update_008252c0(float scaled_delta) override {
        if (!kUnitInstanceStep11Bound) {
            owner_.record("UnitInstance::update_engine_audio", 0x008252c0u);
            return;
        }
        // The RPM smoother at +BC4h, with the authored smoothing rate; the
        // emitters it would feed are not built.
        bsp::run_engine_audio_008252c0(slot_.step11, step11_inputs(),
            owner_.engine_sound_rates, scaled_delta);
        owner_.done("UnitInstance::update_engine_audio", 0x008252c0u);
    }
    void sub_update_00956600(float scaled_delta) override {
        if (!kUnitInstanceStep11Bound || slot_.state == nullptr) {
            owner_.record("UnitInstance::update_unit_timers", 0x00956600u);
            return;
        }
        bsp::run_unit_timers_00956600(*slot_.state, slot_.step11, step11_inputs(),
            scaled_delta);
        owner_.done("UnitInstance::update_unit_timers", 0x00956600u);
    }
    void sub_update_00834e90(float scaled_delta) override {
        if (!kUnitInstanceStep11Bound) {
            owner_.record("UnitInstance::update_propellers", 0x00834e90u);
            return;
        }
        bsp::UnitStep11Inputs in = step11_inputs();
        in.controller_speed = controller_time_value(); // 00834EB7, 0092D730
        bsp::run_propellers_00834e90(slot_.step11, in, scaled_delta);
        owner_.done("UnitInstance::update_propellers", 0x00834e90u);
        owner_.done("UnitInstance::update_bow_wave", 0x00834820u);
        owner_.done("UnitInstance::update_stern_wave", 0x00834cc0u);
        owner_.done("UnitInstance::update_propeller_spray", 0x00834a70u);
    }
    void tick_part(std::size_t, float) override {
        owner_.record("UnitInstance::tick_part", 0x00815370u);
    }

private:
    // What the step-11 runners read off this unit (bsp/unit_instance_step11.hpp).
    bsp::UnitStep11Inputs step11_inputs() const {
        bsp::UnitStep11Inputs in{};
        in.class_id = slot_.class_id;
        in.gate_5d = slot_.state != nullptr && slot_.state->simulate != 0;
        in.throttle = slot_.row.throttle; // +980h
        in.steering = slot_.row.rudder;   // +984h
        in.alternate_inputs = false;      // +61h, no writer (unit_flag_0061)
        in.global_intensity_override = false; // 00f87152, as global_intensity_override()
        return in;
    }

    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
};

// ---------------------------------------------------------------------------
// bsp::UnitRudderHost, the yaw-rate curve 00811890 reads
// ---------------------------------------------------------------------------

class UnitRudderBinding final : public bsp::UnitRudderHost {
public:
    UnitRudderBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot)
        : owner_(owner), slot_(slot) {}

    // 0082e890 calls 00424c40 four times on the low branch and three on the
    // high one and reads +438h..+44Ch off each returned pointer. The settings
    // object is the singleton the Lua-driven loader 0083b5e0 filled, so this
    // hands back the one shared block rather than a per-unit copy.
    const bsp::UnitRudderCurveSettings& settings_00424c40() override {
        owner_.done("ShipMotion::rudder_curve_settings", 0x00424c40u);
        return owner_.rudder_curve;
    }
    // Packet cc13-powerup-bind, 008118A3..008118BA: the manager, EnablePowerups
    // and the category 5 (SHIP_TURNFACTOR) list size. All three answer the one
    // gate, which keeps the shut arm's unscaled 0082ECB0 result (mapped times
    // 1.0f is exact). Shut whenever kPowerUpSubsystemCc13Bound is OFF.
    bool scale_manager_present() override { return yaw_product().has_value(); }
    const bsp::ShipClassFields& ship_class() override { return slot_.fields; }
    bool gameplay_scale_enabled() override { return yaw_product().has_value(); }
    bool scale_manager_enabled() override { return yaw_product().has_value(); }
    float gameplay_scale_008e6430(int) override {
        const std::optional<float> product = yaw_product();
        if (product) {
            owner_.done("ShipMotion::gameplay_scale", 0x008e6430u);
            return *product;
        }
        NoGameplayModifiers modifiers;
        const float scale = bsp::gameplay_modifier_product_008e6430(nullptr, 0, modifiers);
        owner_.done("ShipMotion::gameplay_scale", 0x008e6430u);
        return scale;
    }
    float turn_efficiency() override { return slot_.motion.turn_efficiency; }
    float forward_speed_0092d730() override { return forward_speed; }
    float steering_command() override { return slot_.motion.to_turn; }

    float forward_speed{0.0f};

private:
    std::optional<float> yaw_product() const {
        std::optional<float> product;
        if constexpr (game_powerups::kPowerUpSubsystemCc13Bound) {
            product = game_powerups::powerup_site_product(5, owner_.index_of_slot(slot_));
        }
        return product;
    }

    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
};

// ---------------------------------------------------------------------------
// bsp::UnitOrderRecordIssueHost, 00816a40's network half
// ---------------------------------------------------------------------------

class UnitOrderIssueBinding final : public bsp::UnitOrderRecordIssueHost {
public:
    explicit UnitOrderIssueBinding(GameUnitsHost::Impl& owner) : owner_(owner) {}
    // game+1FE4h from the session context (packet cc13-mp2d): 0 single player, 1 the
    // skirmish host, 2 the client. This site acts only on 2 (unit_order_record.cpp:80,
    // 00816A8D cmp [..+1FE4h],2), so 0 and 1 behave alike. Campaign identity: the OFF
    // arm is the old literal 1.
    int session_mode() override {
        if constexpr (bsp::kSkirmishSessionCc13Bound) {
            return bsp::session_context().session_word_1fe4;
        } else {
            return 1;
        }
    }
    bsp::UnitOrderMessageStorage construct_message_0075b430(int) override {
        owner_.record("UnitOrder::construct_message", 0x0075b430u);
        return bsp::UnitOrderMessageStorage{};
    }
    void send_message_0077c2a0(const bsp::UnitOrderMessageStorage&, std::uint32_t,
        std::uint32_t) override {
        owner_.record("UnitOrder::send_message", 0x0077c2a0u);
    }

private:
    GameUnitsHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::ShipHydroHost, one method per call site of 009329C0
// ---------------------------------------------------------------------------

// The hull's hydrodynamics, run where 00937440 makes its last call, 00937622.
// docs/SHIP_HYDRO_FORCES.md.
//
// Eleven of the twelve methods are calls. The exceptions and the qualifications:
//
//  * unit_category_8_vtable5c is the only record. Its callee is the unit's own
//    vtable slot 5Ch and no packet has read that body
//    (docs/SHIP_HYDRO_FORCES.md follow-up `unit_vtable_5c`), so the answer is
//    substituted from the recovered IsKindOf table and the slot is reported as
//    the evidence instead of a call site;
//  * the leak model at unit+10D4h has no leak points, because nothing in this
//    process damages a hull, so 0074F930's tick and 0074F2E0's heeling torque
//    both run over an empty entry list. They are calls, not records: the
//    reconstruction in bsp/unit_forces.hpp is what answers them;
//  * the disabled path's three effects (00C37E50, 00C37E20 and the OR of bit 2
//    into body+50h) are wired to the body but never reached, because
//    controller+14h is clear on a live hull.
class ShipHydroBinding final : public bsp::ShipHydroHost {
public:
    ShipHydroBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot)
        : owner_(owner), slot_(slot) {}

    bsp::OceanVec3 body_linear_velocity_00c31f40() override {
        owner_.done("ShipHydro::body_linear_velocity", 0x00c31f40u);
        return slot_.motion.linear_velocity;
    }
    bsp::OceanVec3 body_angular_velocity_00c31f20() override {
        owner_.done("ShipHydro::body_angular_velocity", 0x00c31f20u);
        return slot_.motion.angular_velocity;
    }
    // 00C32000 hands back body+8h and 00C33650 expands that 3x4 into the 4x4
    // whose rows are the body axes. The pose the motion state carries is the
    // same one the position phase 00C5B1B0 wrote into the body last step.
    bsp::ShipHydroTransform body_world_transform_00c33650() override {
        bsp::ShipHydroTransform m{};
        m.row0 = {slot_.motion.pose_row0[0], slot_.motion.pose_row0[1],
            slot_.motion.pose_row0[2]};
        m.row1 = {slot_.motion.pose_row1[0], slot_.motion.pose_row1[1],
            slot_.motion.pose_row1[2]};
        m.row2 = {slot_.motion.pose_row2[0], slot_.motion.pose_row2[1],
            slot_.motion.pose_row2[2]};
        m.position = {slot_.motion.position[0], slot_.motion.position[1],
            slot_.motion.position[2]};
        owner_.done("ShipHydro::body_world_transform", 0x00c33650u);
        return m;
    }
    // 00932A42 and 00932E2F, the unit's vtable slot 5Ch with the literal 8. The
    // compiled predicate belongs to the loaded instance class, as at the
    // hull-body call 00937CFD. docs/GAME_UNIT_KIND_BINDING.md.
    bool unit_category_8_vtable5c() override {
        const bsp::UnitKindClassBody* body = bsp::unit_kind_body_for_class(slot_.class_id);
        if (body != nullptr) {
            // GameHostLog groups by method name; keep distinct vtable owners
            // distinct so the first unit cannot label every later call.
            char method[64];
            std::snprintf(method, sizeof(method), "ShipHydro::unit_category_8_class_%02x",
                static_cast<unsigned int>(slot_.class_id));
            owner_.done(method, body->test_address);
        } else {
            owner_.record("ShipHydro::unit_category_8_unresolved_identity", 0x00932a42u);
        }
        return bsp::unit_is_kind_of(slot_.class_id, bsp::kUnitForceSubmarineClassId);
    }
    float water_height_0078cf20(float x, float z) override {
        OceanFieldBinding sea(owner_);
        const float height = bsp::ocean_water_height_0078cf20(x, z, sea);
        owner_.done("ShipHydro::water_height", 0x0078cf20u);
        return height;
    }
    // 00933A01. An undamaged hull has no leak points, so the tick runs over a
    // count of zero and leaves the unit+10FCh it already read at zero.
    void leak_tick_0074f930(float dt) override {
        // Every scalar past `count` is read only inside the per-leak passes, so
        // with a count of zero the result is the zero the routine's own two
        // accumulators start at, whatever they are.
        if (GameUnitsHost::Impl::kShipSinkDescentBound && slot_.leak_ready) {
            // Packet cc9_ship_sink_descent: the six leaks. The health 00923BE0
            // is read only by the cap, which +5Dh suppresses; a live hull's rates
            // come from the 90h leaks cc9_live_hull_leak adds (0074F440); the
            // health is read only by the cap.
            const bool gate_5d = slot_.state != nullptr && slot_.state->simulate != 0;
            // Packet cc9_live_hull_repair: 00923BE0 under its switch, else 1.0.
            const float health = GameUnitsHost::Impl::kLiveHullRepairBound
                ? slot_.leak_health_fraction : 1.0f;
            const bsp::UnitLeakTickResult r = bsp::unit_leak_tick_0074f930(
                slot_.leaks.data(), static_cast<std::uint32_t>(slot_.leaks.size()),
                slot_.leak_rate_cap_08, slot_.leak_capacity_0c,
                GameUnitsHost::Impl::kEnnyiVizEsKeszPercentSubstitute, health, gate_5d, dt);
            leak_water_mass_10fc = r.total_water;
            slot_.leak_total_rate_2c = r.total_rate;  // +2Ch, 0074F984
            if (r.total_water > 0.0f && slot_.leak_first_water_t < 0.0f)
                slot_.leak_first_water_t = owner_.summary.simulated_seconds;
        } else {
            leak_water_mass_10fc = bsp::unit_leak_tick_0074f930(nullptr, 0u, 0.0f, 0.0f,
                0.0f, 1.0f, false, dt).total_water;
        }
        owner_.done("ShipHydro::leak_tick", 0x0074f930u);
    }
    // 00933A52, over the same empty list, so the heeling torque is the zero the
    // routine's own out vector starts at.
    bsp::OceanVec3 leak_heel_torque_0074f2e0() override {
        // The image's pose block at unit+CCh is four floats per row;
        // unit_leak_torque_0074f2e0 reads indices 0, 2, 4, 6, 8, 10 on that stride
        // (0074F35D), so the rows are padded to four (GUNNERY_OPEN_ITEMS 56.3).
        float rows[12] = {
            slot_.motion.pose_row0[0], slot_.motion.pose_row0[1], slot_.motion.pose_row0[2], 0.0f,
            slot_.motion.pose_row1[0], slot_.motion.pose_row1[1], slot_.motion.pose_row1[2], 0.0f,
            slot_.motion.pose_row2[0], slot_.motion.pose_row2[1], slot_.motion.pose_row2[2], 0.0f};
        const bool leaks = GameUnitsHost::Impl::kShipSinkDescentBound && slot_.leak_ready;
        const bsp::OceanVec3 torque = bsp::unit_leak_torque_0074f2e0(
            leaks ? slot_.leaks.data() : nullptr,
            leaks ? static_cast<std::uint32_t>(slot_.leaks.size()) : 0u, rows);
        owner_.done("ShipHydro::leak_heel_torque", 0x0074f2e0u);
        return torque;
    }
    // 00933B01 and 00933B38, the two flushes. They are the only reason the
    // velocity phase 00C41550 has anything to integrate.
    void add_force_00c35360(const bsp::OceanVec3& force) override {
        bsp::dyn_body_add_force_00c35360(slot_.body, force);
        ++owner_.summary.hydro_force_flushes;
        owner_.done("ShipHydro::add_force", 0x00c35360u);
    }
    void add_torque_00c35330(const bsp::OceanVec3& torque) override {
        bsp::dyn_body_add_torque_00c35330(slot_.body, torque);
        ++owner_.summary.hydro_torque_flushes;
        owner_.done("ShipHydro::add_torque", 0x00c35330u);
    }
    void set_linear_velocity_00c37e50(const bsp::OceanVec3& v) override {
        bsp::dyn_body_set_linear_velocity_00c37e50(slot_.body, v);
        slot_.motion.linear_velocity = v;
        owner_.done("ShipHydro::set_linear_velocity", 0x00c37e50u);
    }
    void set_angular_velocity_00c37e20(const bsp::OceanVec3& w) override {
        bsp::dyn_body_set_angular_velocity_00c37e20(slot_.body, w);
        slot_.motion.angular_velocity = w;
        owner_.done("ShipHydro::set_angular_velocity", 0x00c37e20u);
    }
    void set_body_no_gravity_flag_00932a16() override {
        slot_.body.flags |= bsp::kDynBodyFlagNoGravity;
        owner_.record("ShipHydro::set_body_no_gravity_flag", 0x00932a16u);
    }

    // unit+10FCh as the leak tick left it, for the next call's total mass.
    float leak_water_mass_10fc{0.0f};

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
};

// ---------------------------------------------------------------------------
// bsp::ShipMotionHost, one method per call site of 00825f20's motion path
// ---------------------------------------------------------------------------

// Packet cc9_ship_motion_tail: the host side of 00826C34..00826D69.
class ShipMotionTailBinding final : public bsp::ShipMotionTailHost {
public:
    ShipMotionTailBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot,
                          UnitRudderBinding& rudder, std::size_t index,
                          float forward_speed)
        : owner_(owner), slot_(slot), rudder_(rudder), index_(index),
          forward_speed_(forward_speed) {}
    // 00826C5C, 0092E5B0(controller, 0): everything is under controller+60h,
    // whose only writer is the network state message 0092F2E0; no mission run
    // here receives one, so the gate is closed (docs/SHIP_POST_MOTION.md s4).
    void controller_0092e5b0(int) override {
        owner_.done("UnitController::network_correction_0092e5b0", 0x0092e5b0u);
    }
    // 00826C75, 00811890(unit, [unit+984h]). The unit's forward speed is the
    // post-step one, which is what 00811890 reads when the tail runs.
    float yaw_rate_from_rudder_00811890(float ordered_rudder) override {
        rudder_.forward_speed = forward_speed_;
        owner_.done("UnitRudder::yaw_rate_00811890", 0x00826c75u);
        return bsp::unit_yaw_rate_00811890(ordered_rudder, rudder_);
    }
    // 00826C7A..00826CC5: the host's pose cache is always current.
    void refresh_world_matrix() override {}
    // 00826CDB, vtable[50h] = 006DFD60: FLD [unit+1050h], as the tail wrote it.
    float unit_heading_vtable50() override { return slot_.hull_heading_1050; }
    void append_wake_sample_00810190(const bsp::OceanVec3& position, float heading,
                                     float yaw_rate) override {
        const float p[3] = {position.x, position.y, position.z};
        bsp::ship_ai_wake_append_00810190(slot_.wake, p, heading, yaw_rate);
        owner_.done("UnitWake::append_sample", 0x00810190u);
    }
    // 00778890: [unit+284h] set and its +14h pointing back at this unit.
    bool occupant_owns_unit_00778890() override {
        const std::int32_t g = slot_.formation_group;
        if (g < 0 || static_cast<std::size_t>(g) >= owner_.formation_groups.size()) return false;
        // bsp.py show 00778890: test group presence, then compare its +14h
        // identity with the unit. These are the host's existing one-based
        // registry identities; this pure body never dereferences them.
        return bsp::ship_ai_unit_group_leads_00778890(
            static_cast<std::uint32_t>(index_ + 1u),
            static_cast<std::uint32_t>(g) + 1u,
            static_cast<std::uint32_t>(
                owner_.formation_groups[static_cast<std::size_t>(g)].leader + 1u));
    }
    // 00826D21..00826D26: settings+430h, Formacio.UpdateInterval.
    float occupant_timer_refill_00424c40_430() override {
        return owner_.formacio_update_interval_430();
    }
    // 00826D3E, 0070DB60([unit+284h]).
    void occupant_tick_0070db60() override {
        ++owner_.tail_slot_swap_calls;
        owner_.run_formation_slot_swap_0070db60(slot_.formation_group);
    }
    // 00826D5A, 0077A650(unit). The constructor stores 0 at unit+308h
    // (0077EFFE) and no host path sends message 51h, so it is not reached.
    void field_308_crossed_0077a650() override {
        ++owner_.tail_expiry_calls;
        owner_.record("UnitInstance::expire_at_scheduled_time_0077a650", 0x0077a650u);
    }

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
    UnitRudderBinding& rudder_;
    std::size_t index_;
    float forward_speed_;
};

class ShipMotionBinding final : public bsp::ShipMotionHost {
public:
    // Packet cc9_tboat_rudder_torque (docs/CC13_VER_3.md T1 and T2, SHIP_AI 233). True:
    // 00937440's roll torque runs for TorpedoBoats only, with settings+588h = 1500 (the
    // installed value, labelled), in the image's grouping; other ships skip it. False:
    // every surface ship runs the torque with a zero gain, as before.
    static constexpr bool kTorpedoBoatRudderTorqueBound = true;  // ON: USN06, BSM01, JM08 pairs (SHIP_AI 238)
    static constexpr float kTorpedoBoatRudderTorqueMulInstalled = 1500.0f;
    ShipMotionBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot,
        UnitRudderBinding& rudder)
        : owner_(owner), slot_(slot), rudder_(rudder) {}

    void tick_order_ring(float dt) override {
        bsp::tick_unit_order_ring_00813020(slot_.ring, dt, 1);
        slot_.motion.throttle = slot_.ring.current_param_a;
        slot_.motion.to_turn = slot_.ring.current_param_b;
        slot_.motion.order_kind = slot_.ring.current_kind;
        owner_.done("ShipMotion::order_ring_tick", 0x00813020u);
    }

    // 0078cf20 at 00826985, run whole: the wave field times the coverage mask,
    // formed at x87 precision and rounded once. Both leaves are records.
    float ocean_height(float x, float z) override {
        OceanFieldBinding sea(owner_);
        const float height = bsp::ocean_water_height_0078cf20(x, z, sea);
        owner_.done("ShipMotion::ocean_height", 0x0078cf20u);
        return height;
    }

    // 008e6430 at 00826a21. With its gate shut (no power-up store, or an empty
    // category 4 list) the product is the 1.0f at 00d7a24c.
    float gameplay_scale() override {
        if (!owner_.logged_scale) {
            owner_.logged_scale = true;
            if constexpr (game_powerups::kPowerUpSubsystemCc13Bound) {
                owner_.log.notef("gameplay scale 008e6430 at 00826a21: the category 4 product "
                    "from the power-up store when its gate is open, else 1.0f (00d7a24c)");
            } else {
                owner_.log.notef("gameplay scale 008e6430 runs over an empty category list: no "
                    "modifier record is registered in this process, so the product is the 1.0f "
                    "the accumulator starts at (00d7a24c) and the filter 008e4680 is never "
                    "reached");
            }
        }
        // Packet cc13-powerup-bind, 00826A06..00826A21: the category 4
        // (SHIP_SPEED) product behind its gate; the shut arm (00826A2C) is the
        // 1.0f the empty-list run below also returns.
        if constexpr (game_powerups::kPowerUpSubsystemCc13Bound) {
            const std::optional<float> product =
                game_powerups::powerup_site_product(4, owner_.index_of_slot(slot_));
            if (product) {
                owner_.done("ShipMotion::gameplay_scale", 0x008e6430u);
                return *product;
            }
        }
        NoGameplayModifiers modifiers;
        const float scale = bsp::gameplay_modifier_product_008e6430(nullptr, 0, modifiers);
        owner_.done("ShipMotion::gameplay_scale", 0x008e6430u);
        return scale;
    }

    // controller->vtable[0](dt), which for a surface ship is 00937440.
    //
    // Milestone 2r said of this torque that "nothing applies it, because the
    // hydrodynamic tail 009329c0 and the rigid-body solver are external". Both
    // halves are now false. 00937440 applies it itself: `python tools/bsp.py
    // ghidra xrefs 00c35330` reports `From 00937613 in
    // BSP_UnitController_ApplyShipForces`, nine instructions before the call
    // into 009329C0, so the torque goes onto the body exactly as the hydro
    // totals do. It is a call the executable makes and it is made here, and it
    // moves nothing for two separate reasons: the vector it carries is the zero
    // vector, because the gain is multiplied by settings+588h and that field
    // has no recovered producer, and the inverse inertia is zero anyway,
    // because the collision AABB's producer 00C5C940 is unread.
    //
    // Then the tail. 00937622 is a CALL into 009329C0 with the same dt
    // (00937618 FLD [ESP+40h], 0093761C PUSH ECX, 0093761D MOV ECX,EDI,
    // 0093761F FSTP [ESP]), followed by the epilogue at 00937627, so the
    // hydrodynamics happen here, inside the motion tick at 00826A6D and ahead
    // of 0092D300's velocity rewrite, which is the native order. What they
    // stage reaches the hull through 00C35360 AddForce and 00C35330 AddTorque,
    // and the velocity phase 00C41550 at the end of the step integrates it.
    bsp::OceanVec3 run_force_model(float dt) override {
        if (slot_.sub_dive_ready) {
            if (kSubmarineDiveBound) return run_submarine_dive_00936dc0(dt);
            trace_submarine_depth(nullptr);
        }
        bsp::UnitSteeringTorqueInputs in{};
        in.velocity = slot_.motion.linear_velocity;
        in.axis.x = slot_.motion.pose_row2[0];
        in.axis.y = slot_.motion.pose_row2[1];
        in.axis.z = slot_.motion.pose_row2[2];
        in.reference_speed = reference_speed();
        in.hull_mass = slot_.motion_class.hull_mass;
        in.steering = slot_.motion.to_turn;
        in.pose_row0_y = slot_.motion.pose_row0[1];
        in.pose_row2.x = slot_.motion.pose_row2[0];
        in.pose_row2.y = slot_.motion.pose_row2[1];
        in.pose_row2.z = slot_.motion.pose_row2[2];
        in.settings_rudder_torque = 0.0f;  // settings+588h, not recovered
        if constexpr (kTorpedoBoatRudderTorqueBound) {
            // Packet cc9_tboat_rudder_torque (docs/CC13_VER_3.md T1/T2, SHIP_AI 233).
            // 00937440 is slot 0 only of the controller vtable 00D196F0, which 00939F20
            // builds for 00857C20, the TorpedoBoat vtable's +230h; every other ship's
            // controller (base 00D19630) runs 009329C0 alone. SUBSTITUTION, labelled:
            // settings+588h Physics.TBoatNyomatekSzorzo is this installation's
            // shipglobals.lua:553 value 1500, not the loaded field.
            if (!bsp::unit_is_kind_of(slot_.class_id, 0x0e)) {
                run_hydrodynamics_00937622(dt);
                return bsp::OceanVec3{};
            }
            bsp::TorpedoBoatRudderTorqueInputs tb;
            tb.velocity = slot_.motion.linear_velocity;
            for (int i = 0; i < 3; ++i) tb.row2[i] = slot_.motion.pose_row2[i];
            tb.reference_speed = in.reference_speed;
            tb.pose_row0_y = slot_.motion.pose_row0[1];
            tb.to_turn = slot_.motion.to_turn;
            tb.torque_mul = kTorpedoBoatRudderTorqueMulInstalled;
            tb.hull_mass = slot_.motion_class.hull_mass;
            owner_.done("ShipMotion::force_model", 0x00937440u);
            const bsp::OceanVec3 tboat = bsp::tboat_rudder_torque_00937440(tb);
            if (slot_.state == nullptr || !slot_.state->simulate) {
                bsp::dyn_body_add_torque_00c35330(slot_.body, tboat);
                owner_.done("ShipMotion::add_rudder_torque", 0x00c35330u);
            }
            run_hydrodynamics_00937622(dt);
            return tboat;
        }
        owner_.done("ShipMotion::force_model", 0x00937440u);
        const bsp::OceanVec3 torque = bsp::unit_steering_torque_00937440(in);
        // 00937449 CMP byte ptr [EAX+5Dh],0 / 0093744D JNZ 00937618: a set byte
        // skips the whole torque block and lands one instruction before the
        // hydrodynamic call, so the hydrodynamics run either way and only the
        // torque is gated. EAX is [controller+1Ch], the unit, and the byte is
        // the one bsp/unit_instance.hpp names `simulate`, held clear here.
        if (slot_.state == nullptr || !slot_.state->simulate) {
            bsp::dyn_body_add_torque_00c35330(slot_.body, torque);
            owner_.done("ShipMotion::add_rudder_torque", 0x00c35330u);
        }
        run_hydrodynamics_00937622(dt);
        return torque;
    }

    // Packet cc9_submarine_dive. 00936DC0, the submarine controller's slot-0
    // force callback: velocities read first (00936DD0/00936DDD), the
    // hydrodynamics 009329C0 (00936DF8), then the dive law and the write-back
    // (0093739A, 009373A7). The rudder torque of 00937440 is not part of it.
    // SUBSTITUTIONS (labelled): needAir (+1281h), 008522C0 (answered since goal 4
    // chain 369) and the kamikaze test read false; there are no seabed samples, so the clearance never clamps and
    // the gain stays 1.0; the ring writes at +844h/+848h (slot +0Ch/+10h, 2.0 and
    // -1.0, 00936E11/00936E2B) and the unused vtable[34h] read (009371E5) are not
    // made; the reference speed is this binding's 0080FC30.
    bsp::OceanVec3 run_submarine_dive_00936dc0(float dt) {
        bsp::SubmarineDiveInputs in;
        in.linear[0] = slot_.motion.linear_velocity.x;
        in.linear[1] = slot_.motion.linear_velocity.y;
        in.linear[2] = slot_.motion.linear_velocity.z;
        in.angular[0] = slot_.motion.angular_velocity.x;
        in.angular[1] = slot_.motion.angular_velocity.y;
        in.angular[2] = slot_.motion.angular_velocity.z;
        owner_.done("SubmarineController::read_velocities", 0x00c31f40u);
        run_hydrodynamics_00937622(dt);
        const bool dead = owner_.gunnery != nullptr
            && owner_.gunnery->unit_dead(slot_.process_index);
        bsp::SubmarineEffectiveBandInputs band;
        const int level = slot_.row.submarine_depth_level;
        band.commanded = static_cast<bsp::SubmarineDepthBand>(level < 0 ? 0 : (level > 3 ? 3 : level));
        band.dead = dead;
        band.need_air = kSubmarineAirBound && slot_.sub_air.need_air;  // +1281h
        // Goal 4 chain 369: 00936E53 008522C0 on the boat, over its catapult
        // devices and its catapult launch record (GameGunneryHost's adapter).
        band.catapult_wants_surface = owner_.gunnery != nullptr
            && owner_.gunnery->submarine_catapult_wants_surface(slot_.process_index);
        // 00936E5C..00936E80: the same class test exempts a kamikaze boat from
        // the surfacing override (packet cc13-subdive; OFF false, LABELLED).
        band.kamikaze_class = bsp::cc13_sub_dive::kCc13SubDiveKamikazeBound
            && slot_.sub_kamikaze_class;
        in.effective = bsp::submarine_effective_depth_band_00936dc0(band);
        // 00936EA3..00936F06: the clamp needs the bot's owner helm slot (role 1,
        // [[unit+740h]+50h]+1B0h) to be 8 or AI-held (00927F10). SUBSTITUTION,
        // labelled: the bot's owner is the boat itself, and a slot 0..7 reads as
        // player-held.
        const bool clamp_allowed = kSubmarineSeabedBound
            && slot_.current_roles_01ac[1] == 8;
        const bsp::SubmarineDepthTarget target = bsp::submarine_depth_target_00936dc0(
            slot_.sub_bands[static_cast<int>(in.effective)], slot_.sub_scan.clearance,
            clamp_allowed);
        if (target.clamped_by_seabed) ++slot_.sub_clamp_steps;
        in.target_y = target.target_y;
        in.gain = target.gain;
        in.hull_y = slot_.motion.position[1];
        for (int i = 0; i < 3; ++i) {
            in.row0[i] = slot_.motion.pose_row0[i];
            in.row2[i] = slot_.motion.pose_row2[i];
        }
        in.up_speed = slot_.sub_up_speed;
        in.down_speed = slot_.sub_down_speed;
        in.stop_time = slot_.sub_stop_time;
        in.accel = slot_.sub_accel;
        in.reference_speed = reference_speed();
        in.dt = dt;
        in.seed_126c = slot_.sub_seed_126c;
        in.at_depth_1278 = slot_.sub_at_depth_1278;
        in.sink_1270 = slot_.sub_sink_1270;
        in.pitch_398 = slot_.sub_pitch_398;
        const bsp::SubmarineDiveResult r = bsp::submarine_dive_step_00936dc0(in);
        slot_.sub_seed_126c = r.seed_126c;
        slot_.sub_at_depth_1278 = r.at_depth_1278;
        slot_.sub_sink_1270 = r.sink_1270;
        if (r.seed_1274_written) slot_.sub_sink_1274 = r.sink_1274;
        slot_.sub_pitch_398 = r.pitch_398;
        const bsp::OceanVec3 lin{r.linear[0], r.linear[1], r.linear[2]};
        const bsp::OceanVec3 ang{r.angular[0], r.angular[1], r.angular[2]};
        bsp::dyn_body_set_linear_velocity_00c37e50(slot_.body, lin);
        slot_.motion.linear_velocity = lin;
        bsp::dyn_body_set_angular_velocity_00c37e20(slot_.body, ang);
        slot_.motion.angular_velocity = ang;
        owner_.done("SubmarineController::step_depth_physics", 0x00936dc0u);
        ++slot_.sub_dive_steps;
        trace_submarine_depth(&r);
        if (kSubmarineSeabedBound) {
            // 00936E00..00936E2B and 00937069..00937107: the bounds at the ring's
            // write cursor, slot +0Ch / +10h.
            const bsp::SubmarineThrottleBounds bounds = bsp::submarine_throttle_bounds_00936dc0(
                target.gain > 1.0f, slot_.sub_scan.front_rate, slot_.sub_scan.rear_rate);
            const int cursor = slot_.ring.write_cursor;
            if (cursor >= 0 && cursor < static_cast<int>(bsp::kUnitOrderRingSlotCount)) {
                slot_.ring.slot[cursor].param_a_high = bounds.high;
                slot_.ring.slot[cursor].param_a_low = bounds.low;
            }
            run_submarine_seabed_scan(dt);
        }
        if (kSubmarineAirBound) run_submarine_air_and_crush(dt, dead);
        if (kSubmarinePeriscopeOutBound) run_submarine_periscope_00854650(dt);
        // Packet s53_sub_wake_draws: 00854650's 008546DF..008546FC pick on the hull y.
        if (owner_.gunnery != nullptr) {
            owner_.gunnery->submarine_wake_draws_008546df(slot_.process_index,
                slot_.motion.position[1]);
        }
        return bsp::OceanVec3{};
    }

    // Packet cc9_submarine_seabed. 00855420's body before its tail: publish at state
    // 0, one footprint point, every Landscape provider's height at it, advance.
    // SUBSTITUTIONS (labelled): it runs after the force callback, not after
    // 00825F20's motion update; the body-axis speed 0092D730 is dot(linear, row2)
    // of the velocity the dive law wrote; the point is transformed by the pose rows
    // (004142E0 against unit+CCh).
    void run_submarine_seabed_scan(float dt) {
        (void)dt;
        bsp::SubmarineSeabedScan& scan = slot_.sub_scan;
        bsp::submarine_scan_publish_00855440(scan, slot_.sub_bands[3]);
        float speed = 0.0f;
        {
            // Reach counter (cc9-sweep2b, log-only): both forms of 0092D730.
            const float vy_ry = slot_.motion.linear_velocity.y * slot_.motion.pose_row2[1];
            const float vx_rx = slot_.motion.linear_velocity.x * slot_.motion.pose_row2[0];
            const float vz_rz = slot_.motion.linear_velocity.z * slot_.motion.pose_row2[2];
            const float per_op = (vy_ry + vx_rx) + vz_rz;
            // pc24-lint: allow reach counter mirrors the OFF arm's single rounding
            const float once = static_cast<float>(
                static_cast<double>(slot_.motion.linear_velocity.y) * slot_.motion.pose_row2[1]
                + static_cast<double>(slot_.motion.linear_velocity.x) * slot_.motion.pose_row2[0]
                + static_cast<double>(slot_.motion.linear_velocity.z) * slot_.motion.pose_row2[2]);
            bsp::ShipAiSwitchReach& reach = bsp::ship_ai_switch_reach();
            ++reach.seabed_speed_calls;
            if (!bsp::ship_ai_reach_same_bits(per_op, once)) ++reach.seabed_speed_differs;
        }
        if constexpr (kSubmarineSeabedSpeedPc24Cc13Bound) {
            // 0092D74E-0092D763: FMUL, FMUL, FADDP, FMUL, FADDP, each to 24 bits.
            const float vy_ry = slot_.motion.linear_velocity.y * slot_.motion.pose_row2[1];
            const float vx_rx = slot_.motion.linear_velocity.x * slot_.motion.pose_row2[0];
            const float sum_yx = vy_ry + vx_rx;
            const float vz_rz = slot_.motion.linear_velocity.z * slot_.motion.pose_row2[2];
            speed = sum_yx + vz_rz;
        } else {
            speed = static_cast<float>(
                static_cast<double>(slot_.motion.linear_velocity.y) * slot_.motion.pose_row2[1]
                + static_cast<double>(slot_.motion.linear_velocity.x) * slot_.motion.pose_row2[0]
                + static_cast<double>(slot_.motion.linear_velocity.z) * slot_.motion.pose_row2[2]);
        }
        const bsp::SubmarineScanPoint p = bsp::submarine_scan_point_00855420(scan.state,
            slot_.sub_class_width, slot_.sub_class_length, speed,
            slot_.sub_class_max_speed, slot_.ring.current_param_b);
        if (p.reset_state) scan.state = 0;
        const float time = bsp::submarine_scan_time_00855420(p, slot_.sub_class_length, speed);
        const float hull_y = slot_.motion.position[1];
        float world[3];
        for (int i = 0; i < 3; ++i) {
            world[i] = slot_.motion.position[i] + p.x * slot_.motion.pose_row0[i]
                + p.z * slot_.motion.pose_row2[i];
        }
        const bsp::game::SceneWorldClassLists& lists = bsp::game::scene_world_class_lists();
        for (std::size_t index : lists.list(bsp::game::kSceneLandscapeClassId)) {
            const bsp::game::SceneWorldObject& object = lists.objects()[index];
            if (!object.terrain) continue;
            // 00855420's seabed scan calls 0087FA50: world XZ through the
            // loaded Landscape's +3D0h slot 28h. Borrow the same terrain.
            SceneTerrainForwarders terrain(*object.terrain);
            const float sample = bsp::landscape_lua_seg55_argon::landscape_forward_height_at_0087fa50(
                terrain.landscape(), world[0], world[2]);
            bsp::submarine_scan_sample_00855420(scan, sample, slot_.sub_class_height, hull_y, time);
            ++slot_.sub_scan_samples;
        }
        bsp::submarine_scan_advance_00855420(scan);
        owner_.done("SubmarineUnit::seabed_scan", 0x00855420u);
    }

    // Packet cc9_periscope_out. 00854650's periscope arm, 00854AF8..00855057:
    // 00854B00 clears +1234h and skips the arm when +1214h (the "periszkop" node)
    // is null. The repair (00854E44) and the auto-raise (00854ED7, gated by
    // +1235h) come first in the image. Chain479 supplies the common break stores
    // and repair; auto-raise still has no reflection setter. Then
    // 00854F52: state 1 steps the mast's local Y toward +81Ch + periscopeY at
    // dt * 5.0 (00D7A370), any other state toward periscopeY at dt * 3.0
    // (00D7A2B0), both through 0042AC60; only the extending arm reaches 00855045,
    // which sets +1234h when the new Y is at least +81Ch + periscopeY - 1.0.
    // SUBSTITUTIONS (labelled): it runs once per force step after the air step, not
    // in the unit update; the mast is held as its offset above periscopeY, so
    // periscopeY is 0 in the threshold (the image adds the node's attach Y to both
    // sides); the node's pose write (vtable[2Ch]) and the +1210h shape move
    // (00C357B0) are not made.
    void run_submarine_periscope_00854650(float dt) {
        const bool was_out = slot_.sub_periscope_out_1234;
        slot_.sub_periscope_out_1234 = false;                              // 00854B00
        if (!slot_.sub_periscope_node_1214) return;                         // 00854B06
        // 00854E44..00854ECB: repair precedes the ordinary mast step.
        // Read the live task priority and the loaded +3D0h, rather than
        // substituting the installed multiplier or a new repair setting.
        if (slot_.sub_periscope_state_122c == 2) {
            const auto* settings = owner_.lua.gameplay_tuning_settings();
            int priority;
            if (settings != nullptr && owner_.gunnery != nullptr
                && owner_.gunnery->periscope_repair_priority(owner_.index_of_slot(slot_), priority)) {
                const auto repaired = bsp::submarine_step_periscope_repair_00854650(
                    slot_.sub_periscope_repair, dt, priority == 1,
                    settings->failure_repair_multiplier);
                slot_.sub_periscope_repair = repaired.repair;
                if (repaired.completed) {
                    // 00854ECF stores EBX = 0 after 0092BEC0. No native
                    // mast/shape handles are represented by this adapter.
                    slot_.sub_periscope_state_122c = 0;
                }
            }
        }
        const float range = slot_.sub_periscope_move_range;
        if (slot_.sub_periscope_state_122c == 1) {                         // 00854F52
            slot_.sub_mast_offset = bsp::unit_step_towards_0042ac60(
                slot_.sub_mast_offset, range, dt * bsp::kSubPeriscopeRaiseRate);
            slot_.sub_periscope_out_1234 = bsp::submarine_periscope_is_out_00855057(
                slot_.sub_mast_offset, 0.0f, range);                         // 00855045
        } else {                                                           // 00855063
            slot_.sub_mast_offset = bsp::unit_step_towards_0042ac60(
                slot_.sub_mast_offset, 0.0f, dt * bsp::kSubPeriscopeLowerRate);
        }
        if (slot_.sub_periscope_out_1234) ++slot_.sub_periscope_out_steps;
        owner_.done("SubmarineUnit::periscope_out_00855045", 0x00855045u);
        if (was_out != slot_.sub_periscope_out_1234) {
            owner_.log.notef("submarine periscope out: unit=%s out=%d mast=%.3f range=%.2f "
                "t=%.2f (00854650, packet cc9_periscope_out)", slot_.row.name.c_str(),
                slot_.sub_periscope_out_1234 ? 1 : 0,
                static_cast<double>(slot_.sub_mast_offset), static_cast<double>(range),
                owner_.summary.simulated_seconds);
        }
    }

    // Packet cc9_submarine_air. 00855420's tail: 00855250 (0085591C) then 008551C0
    // (0085592B), both on the unit with the step's dt.
    // SUBSTITUTIONS (labelled): they run here, after the force callback of the same
    // step, not after 00825F20's motion update; the warnings 00977050/009771E0/
    // 00977370 are logged, not reported; the settings are the installed
    // shipglobals.lua values (SubmarineDepthSettings' defaults).
    // Packet wire-units: the WarningReportHost 00977050 / 009771E0 / 00977370 report through.
    // LABELLED: this host has no warning manager, so no +24h lock, +D0h never suppresses and the
    // +D4h map never holds a byte; the local slot is kLocalPlayerSlot; the entity's +5Ch is set
    // (a live unit), +5Dh is unit_torn_down_005d, +5Eh/+60h read clear; 00974150 is a log line.
    struct SubmarineWarningReportBinding final : bsp::WarningReportHost {
        GameUnitsHost::Impl& host;
        const GameUnitSlot& unit;
        SubmarineWarningReportBinding(GameUnitsHost::Impl& h, const GameUnitSlot& u)
            : host(h), unit(u) {}
        void lock_24() override {}
        void unlock_24() noexcept override {}
        bool suppressed_d0() override { return false; }
        std::int32_t local_slot_18ec() override { return GameUnitsHost::Impl::kLocalPlayerSlot; }
        float mission_clock_f876a4() override {
            return static_cast<float>(host.summary.simulated_seconds);
        }
        bsp::WarningEntityFlags entity_flags(bsp::WarningEntity) override {
            bsp::WarningEntityFlags f;
            f.present_5c = true;
            f.flag_5d = host.unit_torn_down_005d(unit.process_index);
            return f;
        }
        bool entity_suppressed_0077edf0(bsp::WarningEntity) override { return false; }
        bool is_player_controlled_00965810(bsp::WarningEntity) override {
            return host.is_controlled(unit);
        }
        bool is_kind_5c(bsp::WarningEntity, std::int32_t) override { return false; }  // unread: no excluded kind
        void report_entity_warning_00974150(bsp::WarningEntity, std::string_view id) override {
            host.log.notef("submarine warning: unit=%s id=%.*s t=%.2f (00974150, packet wire-units)",
                unit.row.name.c_str(), static_cast<int>(id.size()), id.data(),
                static_cast<double>(host.summary.simulated_seconds));
        }
    };

    void run_submarine_air_and_crush(float dt, bool dead) {
        const bsp::SubmarineDepthSettings settings{};
        const float hull_y = slot_.motion.position[1];
        const bsp::SubmarineAirStepResult air = bsp::submarine_step_air_00855250(
            slot_.sub_air, dt, hull_y, slot_.sub_bands[1], slot_.sub_air_rates, settings, dead);
        const bool was_need = slot_.sub_air.need_air;
        slot_.sub_air = air.state;
        owner_.done("SubmarineUnit::air_step", 0x00855250u);
        // ON 009771E0 at 0085533B / 00977050 at 00855375: report_submarine_air_critical_009771e0 or
        // report_submarine_air_low_00977050 through SubmarineWarningReportBinding; OFF only logs.
        if constexpr (kWireUnitsHostBound) {
            if (air.warning != bsp::SubmarineAirWarning::none) {
                SubmarineWarningReportBinding reports(owner_, slot_);
                if (air.warning == bsp::SubmarineAirWarning::critical) {
                    bsp::report_submarine_air_critical_009771e0(reports, &slot_);
                } else {
                    bsp::report_submarine_air_low_00977050(reports, &slot_);
                }
            }
        }
        if (air.warning != bsp::SubmarineAirWarning::none || was_need != air.state.need_air) {
            owner_.log.notef("submarine air: unit=%s air=%.3f need_air=%d warning=%s y=%.2f "
                "(packet cc9_submarine_air)", slot_.row.name.c_str(),
                static_cast<double>(air.state.air), air.state.need_air ? 1 : 0,
                air.warning == bsp::SubmarineAirWarning::critical ? "critical"
                    : air.warning == bsp::SubmarineAirWarning::low ? "low" : "none",
                static_cast<double>(hull_y));
        }
        if (air.drowned && !slot_.sub_drowned && owner_.gunnery != nullptr) {
            // 008553A1..008553B8: vtable[70h](1), 0077D1A0 -> 00926C80.
            slot_.sub_drowned = true;
            // Packet cc13-killx: a direct vtable[70h](1), not 00926D90, so no
            // funnel path tag.
            const GameUnitsHost::Impl::FunnelPathTagSuppressScope no_tag(owner_);
            owner_.gunnery->kill_unit_00926d90(slot_.process_index, 1);
            owner_.log.notef("submarine air: unit=%s drowned (vtable[70h](1), packet "
                "cc9_submarine_air)", slot_.row.name.c_str());
        }
        const bsp::SubmarineCrushStepResult crush = bsp::submarine_step_crush_008551c0(
            slot_.sub_crush_1284, dt, hull_y, settings);
        slot_.sub_crush_1284 = crush.accumulator;
        owner_.done("SubmarineUnit::crush_step", 0x008551c0u);
        // ON 00977370 at 0085523C: report_submarine_depth_damage_00977370 through
        // SubmarineWarningReportBinding when 008551C0 reports; OFF reports nothing.
        if constexpr (kWireUnitsHostBound) {
            if (crush.report) {
                SubmarineWarningReportBinding reports(owner_, slot_);
                bsp::report_submarine_depth_damage_00977370(reports, &slot_);
            }
        }
        if (crush.damage > 0.0f && owner_.gunnery != nullptr) {
            // vtable[1ACh] of 00D0BF80 is 0095DA00 (the dword at 00D0C12C).
            owner_.gunnery->apply_script_damage_0095da00(slot_.process_index, crush.damage);
            ++slot_.sub_crush_pulses;
            if (slot_.sub_crush_pulses <= 3 || (slot_.sub_crush_pulses % 30u) == 0u) {
                owner_.log.notef("submarine crush: unit=%s y=%.2f damage=%.2f pulses=%llu "
                    "(008551C0 -> 0095DA00, packet cc9_submarine_air)", slot_.row.name.c_str(),
                    static_cast<double>(hull_y), static_cast<double>(crush.damage),
                    slot_.sub_crush_pulses);
            }
        }
    }

    // One line per submarine every 200 force steps, in both switch states, so a
    // pair compares depths directly.
    void trace_submarine_depth(const bsp::SubmarineDiveResult* r) {
        ++slot_.sub_trace_counter;
        if ((slot_.sub_trace_counter % 200u) != 1u) return;
        owner_.log.notef("submarine depth trace: unit=%s step=%llu level=%d y=%.2f vy=%.3f "
            "sink=%.3f pitch=%.4f rate=%.3f dir=%d air=%.3f need=%d clear=%.2f front=%.3f rear=%.3f clamps=%llu bound=%d x=%.1f z=%.1f (packet cc9_submarine_dive)",
            slot_.row.name.c_str(), slot_.sub_trace_counter,
            static_cast<int>(slot_.row.submarine_depth_level),
            static_cast<double>(slot_.motion.position[1]),
            static_cast<double>(slot_.motion.linear_velocity.y),
            static_cast<double>(slot_.sub_sink_1270), static_cast<double>(slot_.sub_pitch_398),
            r != nullptr ? static_cast<double>(r->commanded_rate) : 0.0,
            r != nullptr ? r->direction : 0, static_cast<double>(slot_.sub_air.air),
            slot_.sub_air.need_air ? 1 : 0, static_cast<double>(slot_.sub_scan.clearance),
            static_cast<double>(slot_.sub_scan.front_rate),
            static_cast<double>(slot_.sub_scan.rear_rate), slot_.sub_clamp_steps,
            kSubmarineDiveBound ? 1 : 0,
            static_cast<double>(slot_.motion.position[0]),
            static_cast<double>(slot_.motion.position[2]));
    }

    // 00937622, 00937440's last call. 009329C0 is slot 0 of the controller vtable
    // 00D19630 and 00937440 forwards its own dt into it.
    void run_hydrodynamics_00937622(float dt) {
        if (slot_.buoyancy_elements.empty()) return;
        ShipHydroBinding hydro_host(owner_, slot_);
        hydro_host.leak_water_mass_10fc = slot_.leak_water_mass_10fc;

        bsp::ShipHydroInputs in{};
        // controller+14h. Only DisablePhysics 00891380 sets it (packet
        // cc9_bsm01_state_natives); clear, the live path at 00932A1F is taken.
        in.disabled = slot_.controller_disabled_14;
        in.material = slot_.hull_material;
        in.record = bsp::ship_physics_material_shipped(slot_.hull_material);
        in.class_mass = slot_.motion_class.hull_mass;      // class+B0h
        in.class_length = slot_.motion_class.hull_length;  // class+A0h
        // Raw class+A4 Width from00960368; a model-derived unit+9CC can
        // differ, so this class-field consumer retains the Lua value.
        in.class_width = slot_.class_width_00a4;
        in.leak_water_mass = slot_.leak_water_mass_10fc;
        // unit+5Dh, the byte docs/UNIT_INSTANCE.md names `simulate`. The list
        // filter requires it clear on a live ship, and clear is what opens the
        // planing branch at 00933639, not what shuts it.
        in.suppress_planing = slot_.state != nullptr && slot_.state->simulate != 0;
        in.elements = slot_.buoyancy_elements.data();
        in.element_count = static_cast<int>(slot_.buoyancy_elements.size());

        const bsp::ShipHydroResult result
            = bsp::ship_hydro_apply_forces_009329c0(in, dt, hydro_host);
        owner_.done("ShipMotion::hydrodynamics", 0x009329c0u);
        slot_.leak_water_mass_10fc = hydro_host.leak_water_mass_10fc;
        // DIAGNOSTIC (packet cc9_hull_flooding_trace, BSP_HULL_FLOODING_TRACE=1):
        // every 0.5 s per ship, the leak water unit+10FCh, the total rate +2Ch,
        // alive or wreck (+5Dh), the hull's world height and up row, and each
        // 0074F2E0 leak with a rate or water (its hull-space point). Read-only.
        {
            static const bool trace = [] {
                char* v = nullptr;
                std::size_t n = 0;
                const bool on = _dupenv_s(&v, &n, "BSP_HULL_FLOODING_TRACE") == 0
                    && v != nullptr && v[0] == '1';
                std::free(v);
                return on;
            }();
            if (trace) {
                slot_.flood_trace_timer -= dt;
                if (slot_.flood_trace_timer <= 0.0f) {
                    slot_.flood_trace_timer += 0.5f;
                    if (slot_.flood_trace_timer <= 0.0f) slot_.flood_trace_timer = 0.5f;
                    const bool wreck = slot_.state != nullptr && slot_.state->simulate != 0;
                    const float* up = slot_.motion.pose_row1;
                    owner_.log.notef("hull flooding %s t=%.2f %s water=%.3f rate=%.4f leak_ready=%d "
                        "y=%.2f up=(%.4f %.4f %.4f) height=%.2f",
                        slot_.row.name.c_str(),
                        static_cast<double>(owner_.summary.simulated_seconds),
                        wreck ? "wreck" : "alive",
                        static_cast<double>(slot_.leak_water_mass_10fc),
                        static_cast<double>(slot_.leak_total_rate_2c), slot_.leak_ready ? 1 : 0,
                        static_cast<double>(slot_.motion.position[1]),
                        static_cast<double>(up[0]), static_cast<double>(up[1]),
                        static_cast<double>(up[2]),
                        static_cast<double>(slot_.motion_class.hull_height));
                    for (std::size_t i = 0; i < slot_.leaks.size(); ++i) {
                        const bsp::UnitLeakEntry& e = slot_.leaks[i];
                        if (!(e.rate > 0.0f) && !(e.water > 0.0f)) continue;
                        owner_.log.notef("  hull leak %s #%zu local=(%.2f %.2f %.2f) rate=%.4f "
                            "water=%.3f", slot_.row.name.c_str(), i,
                            static_cast<double>(e.point.x), static_cast<double>(e.point.y),
                            static_cast<double>(e.point.z), static_cast<double>(e.rate),
                            static_cast<double>(e.water));
                    }
                }
            }
        }
        ++slot_.row.hydro_calls;
        ++owner_.summary.hydro_calls;
        owner_.summary.hydro_element_steps
            += static_cast<unsigned long long>(in.element_count);
        owner_.summary.hydro_submerged_steps
            += static_cast<unsigned long long>(result.submerged_elements);
        slot_.row.hydro_elements = in.element_count;
        slot_.row.hydro_submerged = result.submerged_elements;
        slot_.row.hydro_force[0] = result.force.x;
        slot_.row.hydro_force[1] = result.force.y;
        slot_.row.hydro_force[2] = result.force.z;
        slot_.row.hydro_torque[0] = result.torque.x;
        slot_.row.hydro_torque[1] = result.torque.y;
        slot_.row.hydro_torque[2] = result.torque.z;
        if (!owner_.logged_hydro) {
            owner_.logged_hydro = true;
            owner_.log.notef("hydrodynamics 009329c0 runs from 00937440's last call at "
                "00937622, over %d buoyancy elements of \"%s\" and physics material %d. "
                "When packet cc9_buoyancy_elements is bound and the class model has "
                "its deckline and bottomline, the list is the image's (0082D040); "
                "otherwise it is a STAND-IN built from the "
                "class row's own Length %.1f, Height %.1f and Mass %.1f the way "
                "bsp_ship_motion_probe.exe --hydro builds it, and the buoyancy is solved "
                "so the hull displaces its own weight at its draft. The world's gravity "
                "is now the game's own (0, %.1f, 0) from 004DDB90; before this milestone "
                "it was zero, and the two only balance together. The first flush staged "
                "force (%.2f, %.2f, %.2f) and torque (%.2f, %.2f, %.2f) with %d of %d "
                "elements submerged",
                static_cast<int>(slot_.buoyancy_elements.size()), slot_.row.name.c_str(),
                static_cast<int>(slot_.hull_material),
                static_cast<double>(slot_.motion_class.hull_length),
                static_cast<double>(slot_.motion_class.hull_height),
                static_cast<double>(slot_.motion_class.hull_mass),
                static_cast<double>(owner_.physics_world.gravity.y),
                static_cast<double>(result.force.x), static_cast<double>(result.force.y),
                static_cast<double>(result.force.z),
                static_cast<double>(result.torque.x), static_cast<double>(result.torque.y),
                static_cast<double>(result.torque.z), result.submerged_elements,
                in.element_count);
        }
    }

    float reference_speed() override {
        float scale = bsp::kUnitReferenceSpeedUnscaled;
        if constexpr (game_powerups::kPowerUpSubsystemCc13Bound) {
            scale = unit_reference_speed_scale_0080fc30(owner_.index_of_slot(slot_));
        }
        return bsp::unit_reference_speed_0080fc30(slot_.motion.max_speed, scale);
    }

    bsp::OceanVec3 body_linear_velocity() override { return slot_.motion.linear_velocity; }

    bsp::ShipBodyBasis body_basis() override {
        bsp::ShipBodyBasis basis{};
        for (int i = 0; i < 3; ++i) {
            basis.row0[i] = slot_.motion.pose_row0[i];
            basis.row1[i] = slot_.motion.pose_row1[i];
            basis.row2[i] = slot_.motion.pose_row2[i];
        }
        return basis;
    }

    float forward_acceleration() override {
        return bsp::unit_forward_acceleration_00825ec0(slot_.motion_class.retardation,
            slot_.motion.acceleration_boost, 0.0f);
    }

    void body_set_linear_velocity(const bsp::OceanVec3& value) override {
        slot_.motion.linear_velocity = value;
    }

    bsp::OceanVec3 body_angular_velocity() override { return slot_.motion.angular_velocity; }

    float yaw_rate_target(float smoothed_rudder) override {
        rudder_.forward_speed = forward_speed();
        // 0092e950 then the FCHS at 0092e955.
        const float rate = -bsp::unit_yaw_rate_00811890(smoothed_rudder, rudder_);
        // 0092e966: settings+220h/+224h were not recovered; with a zero
        // propeller load the routine returns the rate unchanged whatever they
        // are, which is the state a created unit is in here.
        return bsp::unit_propeller_turn_assist_00825de0(rate, slot_.motion.propeller_load,
            0.0f, 0.0f);
    }

    // IsKindOf(0Eh), TorpedoBoat: 00826A78 (the boost block) and 0092E9D1 (the
    // righting term). A destroyer's IsKindOf answers {0,1,2,4,5,6,7} plus its own
    // class id, so only a TorpedoBoat leaf answers true. Packet cc9_torpedo_boat_kind:
    // with bsp::kShipTorpedoBoatKindBound false every unit answers false, as before.
    bool unit_trait_0e() override {
        const bool kind = bsp::unit_is_kind_of(slot_.class_id, 0x0e);
        if (kind) ++owner_.torpedo_boat_trait_queries;
        if constexpr (bsp::kShipTorpedoBoatKindBound) {
            return kind;
        } else {
            return false;
        }
    }

    void body_set_angular_velocity(const bsp::OceanVec3& value) override {
        slot_.motion.angular_velocity = value;
    }

    void controller_step(float dt) override {
        bsp::unit_controller_update_0092be80(slot_.controller, dt);
        owner_.done("ShipMotion::controller_step", 0x0092be80u);
    }
    void unit_post_motion(float) override {
        if constexpr (GameUnitsHost::Impl::kShipPostMotionRepairOrder) {
            // 008160B0: class+D0h (taken as set), then the four entity bytes.
            const bool live = slot_.state != nullptr && slot_.state->active != 0 &&
                              slot_.state->simulate == 0 && slot_.scene_destroyed_005e == 0 &&
                              slot_.scene_pending_destroy_0060 == 0;
            if (live) ++owner_.post_motion_gate_passes;   // 008160D7 would call 0093CA20
            owner_.done("ShipMotion::unit_post_motion", 0x00826b84u);
        } else {
            owner_.record("ShipMotion::unit_post_motion", 0x00826b84u);
        }
    }

    float forward_speed() const {
        bsp::UnitBodyAxisSpeedInputs in{};
        in.velocity[0] = slot_.motion.linear_velocity.x;
        in.velocity[1] = slot_.motion.linear_velocity.y;
        in.velocity[2] = slot_.motion.linear_velocity.z;
        in.axis[0] = slot_.motion.pose_row2[0];
        in.axis[1] = slot_.motion.pose_row2[1];
        in.axis[2] = slot_.motion.pose_row2[2];
        return bsp::unit_forward_speed_0092d730(in);
    }

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
    UnitRudderBinding& rudder_;
};

// ---------------------------------------------------------------------------
// 004c0890, the controlled-unit bind
// ---------------------------------------------------------------------------

class ControlledUnitQueryBinding final : public bsp::ControlledUnitQuery {
public:
    explicit ControlledUnitQueryBinding(int class_id) : class_id_(class_id) {}
    bool unit_is_kind_of(int query) override {
        return bsp::unit_is_kind_of(class_id_, query);
    }
    bool has_driven_sub_unit() override { return false; }   // unit+3D0h
    bool sub_unit_is_kind_of(int) override { return false; }

private:
    int class_id_;
};

class SetControlledUnitBinding final : public bsp::SetControlledUnitHost {
public:
    explicit SetControlledUnitBinding(GameUnitsHost::Impl& owner) : owner_(owner) {}
    void store_controlled_unit(bool unit_present) override {
        // bsp.py show 004c0890: 004C0893 publishes ECX before testing it.
        // The caller sets controlled_index from its validated live slot
        // before entering this callback; the null arm never indexes slots.
        owner_.controlled_unit_00e188d8 = unit_present
            ? owner_.slots[owner_.controlled_index].get() : nullptr;
        owner_.controlled_bound = unit_present;
        owner_.done("ControlledUnit::store_global", 0x004c0893u);
    }
    bool driven_listener_handle() override {
        queried_driven_listener_ = true;
        owner_.record("ControlledUnit::driven_listener_handle", 0x004c08f7u);
        return false;
    }
    void publish_listener(bool) override {
        // 004C090C..004C0924 is an exact null operand when resolution found
        // no listener target (including 004C0890(nullptr)). Never convert the
        // unresolved driven-unit virtual+18 result's bool into a native pointer.
        if (!queried_driven_listener_ && clear_game_native_controlled_listener()) {
            owner_.done("ControlledUnit::publish_listener", 0x00b0d7b0u);
            return;
        }
        owner_.record("ControlledUnit::publish_listener", 0x00b0d7b0u);
    }

private:
    bool queried_driven_listener_{false};
    GameUnitsHost::Impl& owner_;
};

// Packet cc13-para2: the Lua side of 0084CB70. LABELLED SUBSTITUTION: the image
// runs Scripts\global\luaMW_init.lua and Scripts\datatables\SoldierGlobals.lua
// in a private state; this host has one Lua state, so it reads the datatable's
// text and takes each `SoldierGlobals["Key"] = number` assignment (the file is
// exactly those six lines in this installation). An absent key reads 0.0, as
// 00B66270 does for nil.
class SoldierGlobalsTextHost final : public bsp::soldier_traffic_cc13::SoldierGlobalsLuaHost {
public:
    explicit SoldierGlobalsTextHost(GameMissionLuaHost& lua) : lua_(lua) {}
    void open_state_00b6a020(std::uint32_t) override {}
    void run_script_00b69d40(const char* path) override {
        if (std::strstr(path, "SoldierGlobals") == nullptr) return;
        std::vector<std::uint8_t> bytes;
        if (!lua_.read_resource_file(path, bytes)) return;
        read_ = true;
        text_.assign(bytes.begin(), bytes.end());
    }
    double soldier_globals_number_00b66270(const char* key) override {
        const std::string needle = std::string("SoldierGlobals[\"") + key + "\"]";
        const std::size_t at = text_.find(needle);
        if (at == std::string::npos) return 0.0;
        const std::size_t eq = text_.find('=', at + needle.size());
        if (eq == std::string::npos) return 0.0;
        return std::strtod(text_.c_str() + eq + 1, nullptr);
    }
    void close_state_00b669a0() override {}
    bool read() const noexcept { return read_; }

private:
    GameMissionLuaHost& lua_;
    std::string text_;
    bool read_{false};
};

}  // namespace units_local

// ---------------------------------------------------------------------------
// Packet cc9_pilot_vehicle_terrain_avoidance (docs/ATTACKER_EVASION.md 6):
// 009A17D0's vehicle arm 007DF4F0 and terrain arm 0099F1C0.
// ---------------------------------------------------------------------------

inline namespace units_local {

struct AvoidBox {
    float mn[3];
    float mx[3];
};

// class+50h's local box at +28h..+3Ch (min xyz, then max xyz). This host has
// no model box (UnitHullExtentClassInputs.model_local_box stays null), so the
// class Width/Height/Length (+A4h/+A8h/+A0h) stand in as a box symmetric about
// the origin. SUBSTITUTION, labelled.
inline AvoidBox avoid_box(const GameUnitSlot& s) {
    const float hx = 0.5f * s.class_width_00a4;
    const float hy = 0.5f * s.motion_class.hull_height;
    const float hz = 0.5f * s.motion_class.hull_length;
    return AvoidBox{{-hx, -hy, -hz}, {hx, hy, hz}};
}

// class+500h, 007C4CDC-007C4D78: the largest of max(-min, max) over x, y, z.
inline float avoid_radius_500(const GameUnitSlot& s) {
    const AvoidBox b = avoid_box(s);
    float r = 0.0f;
    for (int i = 0; i < 3; ++i) {
        const float e = (-b.mn[i] < b.mx[i]) ? b.mx[i] : -b.mn[i];
        if (r < e) r = e;
    }
    return r;
}

// Packet p21_thrcut (docs/OCEAN_WAVE_FIELD.md section 84). A plane's vtable[34h] is
// 007BBB70, a copy of unit+AC8h (ctl+18h). While the plane is parented, 007D9CE0 keeps
// that vector in the PARENT'S LOCAL frame (section 75). True: a parented plane answers
// deck_local_vel, the carrier-frame vector, as the bytes do. False: it answers the
// host's world-axes rotation of the same relative velocity (plane_world_velocity).
inline constexpr bool kAvoidParentLocalVelocityBound = true;   // ON: OCEAN_WAVE_FIELD 84 (USN01 pair)

// vtable[34h], the world velocity (for a parented plane, see above).
inline void avoid_velocity(const GameUnitSlot& s, float out[3]) {
    if (bsp::unit_is_kind_of(s.class_id, 0x0F)) {
        if constexpr (kAvoidParentLocalVelocityBound) {
            if (s.deck_parent_plus_one != 0) {
                out[0] = s.deck_local_vel[0];
                out[1] = s.deck_local_vel[1];
                out[2] = s.deck_local_vel[2];
                return;
            }
        }
        out[0] = s.plane_world_velocity[0];
        out[1] = s.plane_world_velocity[1];
        out[2] = s.plane_world_velocity[2];
    } else {
        out[0] = s.motion.linear_velocity.x;
        out[1] = s.motion.linear_velocity.y;
        out[2] = s.motion.linear_velocity.z;
    }
}

inline float avoid_len(const float v[3]) {   // 0042B2F0
    float len;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        len = length_0042b2f0_pc24(v);   // pc24w1-sweep3-5, with the 1e-10 floor
    } else {
    len = static_cast<float>(std::sqrt(static_cast<double>(v[0]) * v[0] +
                                       static_cast<double>(v[1]) * v[1] +
                                       static_cast<double>(v[2]) * v[2]));
    }
    return len;
}

inline float avoid_interp(float x0, float y0, float x1, float y1, float x) {   // 00419010
    return bsp::dive_bomb_interpolate_clamped_00419010(x0, y0, x1, y1, x);
}

}  // namespace units_local

// Packet cc9_plane_body_rate_check: the one plane BSP_PLANE_RATE_TRACE names
// (empty when unset). Diagnostic only.
inline const std::string& plane_rate_trace_name() {
    static const std::string value = [] {
        char* text = nullptr;
        std::size_t length = 0;
        std::string v;
        if (_dupenv_s(&text, &length, "BSP_PLANE_RATE_TRACE") == 0 && text != nullptr) {
            v = text;
        }
        std::free(text);
        return v;
    }();
    return value;
}

// 0099CAB0, __thiscall(pilot, pass, lo, hi, dLo, dHi, R, bankF), RET 1Ch,
// body 0099CAB0-0099D03B: shapes one blocked run of the terrain fan into a
// band and inserts it into pilot+4h + pass * C8h.
inline void terrain_shape_band_0099cab0(GameUnitsHost::Impl& host, GameUnitSlot& u, int pass,
                                        float lo, float hi, float dlo, float dhi, float r,
                                        float bank_f, float level_flight_speed, float speed,
                                        bool trace = false) {
    // DIAGNOSTIC (cc9-physics27, BSP_PLANE_RATE_TRACE): one "plane tr shape" line per
    // call naming the exit taken; log-only.
    auto shape_note = [&](const char* exit, bool cov, float mm, float bs, float dn, float ee) {
        if (!trace) return;
        host.log.notef("plane tr shape %s t=%.2f pass=%d exit=%s covers=%d m=%.4f base=%.4f "
            "den=%.2f e=%.4f lo=%.4f hi=%.4f dlo=%.2f dhi=%.2f r=%.2f",
            u.row.name.c_str(), static_cast<double>(host.summary.simulated_seconds), pass, exit,
            cov ? 1 : 0, static_cast<double>(mm), static_cast<double>(bs), static_cast<double>(dn),
            static_cast<double>(ee), static_cast<double>(lo), static_cast<double>(hi),
            static_cast<double>(dlo), static_cast<double>(dhi), static_cast<double>(r));
    };
    if (!(lo < hi)) { shape_note("lo>=hi", false, 0.0f, 0.0f, 0.0f, 0.0f); return; }   // 0099CABC
    // 0099CAD4-0099CB19; x0 is pilot+264h (0099CAF2). 0099D300 stores 0.0 there
    // at 0099EB9D (XORPS 0099EB89) every think; the 0.12 ([00CE81A8]) stored at
    // 0099EB95 goes to pilot+260h. The 0.12 below is the old reading, kept
    // until the switch is paired (cc13-audit10 finding 3).
    const float bank_lo = kBandShaperPlan264Bound ? u.plan_band_bank_264 : 0.12f;
    const float g = pass == 0 ? avoid_interp(bank_lo, 0.0f, 0.35f, 1.0f, bank_f)   // [00CF6560]
                              : avoid_interp(1.3f, 1.0f, 1.7f, 0.0f, bank_f);   // [00CEB4B4] [00D1F3C4]
    if (1.0f > g) {
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // pc24w1-sweep3-5: 0099CB3B..0099CB47 and 0099CB51..0099CB55, each op
            // rounded at PC24 against the qword [00CE3DF0] = 1.100000023841858 (float 1.1
            // widened).
            namespace pc24 = bsp::cc13x87;
            constexpr double kFade = 1.100000023841858;   // [00CE3DF0]
            lo = static_cast<float>(pc24::x87_add24(pc24::x87_mul24(pc24::x87_sub24(lo, kFade), g), kFade));
            hi = static_cast<float>(pc24::x87_sub24(pc24::x87_mul24(pc24::x87_add24(hi, kFade), g), kFade));
        } else {
        lo = static_cast<float>((static_cast<double>(lo) - 1.1) * g + 1.1);     // double [00CE3DF0]
        hi = static_cast<float>((static_cast<double>(hi) + 1.1) * g - 1.1);
        }
        if (!(lo < hi)) { shape_note("fade", false, 0.0f, 0.0f, 0.0f, g); return; }   // 0099CB67
    }
    const float ratio = dlo / dhi;                                   // 0099CB6D-0099CBB8
    if (static_cast<double>(ratio) > 1.8) {                          // double [00D049A8]
        dlo = static_cast<float>(dhi * 1.8);
    } else if (0.56f > ratio) {                                      // [00D1F3C0]
        dhi = static_cast<float>(dlo / 0.56);                        // double [00D1F3B8]
    }
    const bool covers = !(0.0f < lo) && !(hi < 0.0f);
    float m = 0.0f;
    bool range_test = false;
    if (covers) {
        m = (hi > -lo) ? -lo : hi;
        if (pass == 1) {
            if (m > 0.3f) {                                          // 0099CC11 [00CE69C8]
                const float tgt = avoid_interp(0.3f,
                    static_cast<float>(u.plane_max_spd * 1.8), 0.7f,
                    static_cast<float>(level_flight_speed + 20.0), m);   // [00CE3E18], double [00CE3D88]
                const float diff = static_cast<float>(static_cast<double>(tgt) - speed);
                if (30.0 > diff) {                                   // double [00CE7630]
                    const float tl = avoid_interp(-20.0f, -1.0f, 30.0f, 1.0f, diff);
                    if (tl < u.av_throttle_25c) u.av_throttle_25c = tl;   // 00415510 min
                    ++u.tr_throttle_sets;
                }
            }
            range_test = true;
        } else {
            if (static_cast<double>(m) > 0.4) {                      // 0099CD07 double [00CE65D0]
                // 0099BB00: unit+BC4h = max(unit+BC4h, 2 (m - 0.4) sin|bank| + 1),
                // with the unfolded |bank| from unit+C68h.
                if constexpr (GameUnitsHost::Impl::kPlaneYawGainBc4Bound) {
                    const float ab = std::fabs(u.plane_bank_angle_c68);
                    float v;
                    if constexpr (kPc24PlaneTasksCc13Bound) {
                        // pc24w1-sweep3-5: FSIN stored through float (0099CD4A), then
                        // (m - 0.4) * sin, doubled, + 1.0 (0099CD52..0099CD65), each op
                        // rounded at PC24.
                        namespace pc24 = bsp::cc13x87;
                        const float s = static_cast<float>(std::sin(static_cast<double>(ab)));
                        // [00CE65D0] is float 0.4 widened.
                        const double a = pc24::x87_mul24(pc24::x87_sub24(m, 0.4000000059604645), s);
                        v = static_cast<float>(pc24::x87_add24(a + a, 1.0));
                    } else {
                    v = static_cast<float>(
                        2.0 * static_cast<double>(static_cast<float>(
                            (static_cast<double>(m) - 0.4) * std::sin(ab))) + 1.0);
                    }
                    // 0099BB00: plane_latch_yaw_rate_multiplier_0099bb00 latches unit+BC4h
                    // (plane_yaw_gain_bc4) at max(cur, v); OFF keeps the inline latch.
                    const bool raises = !(u.plane_yaw_gain_bc4 > v);
                    bsp::plane_latch_yaw_rate_multiplier_0099bb00(u.plane_yaw_gain_bc4, v);
                    if (raises) ++u.yaw_gain_raises;
                }
            }
            r = static_cast<float>(r * 0.7);                         // double [00CEFFA0]
        }
    } else {
        if (pass == 1) range_test = true;
        else r = static_cast<float>(r * 0.7);
    }
    if (range_test) {                                                // 0099CDC9
        if (dlo > r && dhi > r) { shape_note("range", covers, m, 0.0f, 0.0f, 0.0f); return; }
        r = static_cast<float>(r * 1.1);
    }
    const float base = covers ? avoid_interp(0.0f, 0.3f, 0.08f, 0.5f, m) : 0.3f;   // [00D05B50] [00CE3800]
    const float den = (200.0 > r) ? r : 200.0f;                      // double [00CE4D70]
    const float e_trace = dhi / den - base;                          // DIAGNOSTIC copy of e
    if (lo > -1.0f) {                                                // 0099CD96
        const float e = dhi / den - base;                            // dHi for both edges
        if (0.0f > e) {
            lo = covers ? avoid_interp(0.0f, 0.001f, 0.1f, 1.1f, m) * e + lo : lo - 0.001f;
        } else {
            lo = e * e + lo;
        }
    }
    if (1.0f > hi) {                                                 // 0099CEE5
        const float e = dhi / den - base;
        if (0.0f > e) {
            hi = covers ? hi - avoid_interp(0.0f, 0.001f, 0.1f, 1.1f, m) * e : hi + 0.001f;
        } else {
            hi = hi - e * e;
        }
    }
    if (!(lo < hi)) { shape_note("final_lo>=hi", covers, m, base, den, e_trace); return; }   // 0099D016
    host.ga_insert_0099b790(u, pass, lo, hi);                        // 0099D025
    ++u.tr_bands;
    shape_note("inserted", covers, m, base, den, e_trace);
}

inline namespace units_local {

// ---------------------------------------------------------------------------
// Milestone 2q: bsp::CruiseSpeedSettingHost, the five call sites of 00822C20's
// property-bag arm (0082356C..008235FB).
// ---------------------------------------------------------------------------
//
// 00822C20 is slot 0A0h of the game-unit vtable family and the executable
// reaches it where the native does, from BSP_SEntity_InitAll's slot call at
// 00926110, which in this process is the per-entity pass create_units makes
// over the records the instantiate pass left. The two finds are answered from
// the merged property bag the scene-contents host kept on the entity record
// (docs/CRUISE_SPEED_SETTING.md); the three setters are the reconstructions
// already on main.
class StartSpeedSeedBinding final : public bsp::CruiseSpeedSettingHost {
public:
    StartSpeedSeedBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot,
                          const GameSceneEntityRecord& entity)
        : owner_(owner), slot_(slot), entity_(entity) {}

    bool find_shipyard_launch_00823576(std::uint32_t, const char*) override {
        owner_.done("SceneStartSpeed::find_shipyard_launch", 0x00823576u);
        return entity_.shipyard_launch;
    }

    bsp::SceneStartSpeedProperty find_start_speed_00823590(std::uint32_t,
                                                           const char*) override {
        owner_.done("SceneStartSpeed::find_start_speed", 0x00823590u);
        bsp::SceneStartSpeedProperty record;
        record.present = entity_.start_speed_present;
        record.type = static_cast<bsp::ScenePropertyType>(entity_.start_speed_type);
        record.value_int = entity_.start_speed_int;
        record.value_float = entity_.start_speed_float;
        return record;
    }

    float unit_reference_speed_0080fc30(std::uint32_t) override {
        // 008235BA and 008235DC, the same callee at two sites. unit+9C0h is
        // the class `MaxSpeed` 00822C4F/00822C65 set earlier in this same
        // routine, which create_units copies out of the VehicleClass row.
        ++reference_calls;
        owner_.done("SceneStartSpeed::unit_reference_speed", 0x0080fc30u);
        float scale = bsp::kUnitReferenceSpeedUnscaled;
        if constexpr (game_powerups::kPowerUpSubsystemCc13Bound) {
            // Packet cc13-powerup-bind phase 2a: this runs inside create_units
            // before the slot joins the table, so 008E4680 reads the slot itself
            // (a command building granted earlier in the pass can already have
            // filled category 4). The caller (00822C20's arm) is noexcept, so
            // this path must not reach the resolver's throw.
            const std::size_t index = owner_.index_of_slot(slot_);
            if (index < owner_.slots.size()) {
                scale = unit_reference_speed_scale_0080fc30(index);
            } else {
                game_powerups::PowerupUnitView view;
                // 0043F080: +5Ch set, +5Dh, +60h and +5Eh clear.
                view.live = slot_.scene_flags_available && slot_.state != nullptr
                    && slot_.state->active != 0 && slot_.state->simulate == 0
                    && slot_.scene_pending_destroy_0060 == 0 && slot_.scene_destroyed_005e == 0;
                view.party = slot_.row.party;                              // +54h
                // LABELLED: the id the slot takes when create_units appends it.
                view.id = static_cast<std::uint16_t>(owner_.slots.size() + 1);
                view.slot = ai_scene_unit_slot_009ffd20(slot_.row.party, slot_.row.name);
                for (int axis = 0; axis < 3; ++axis) {
                    view.position[axis] = slot_.motion.position[axis];
                }
                scale = game_powerups::powerup_site_product_for_view(4, view)
                    .value_or(bsp::kUnitReferenceSpeedUnscaled);
            }
        }
        return bsp::unit_reference_speed_0080fc30(slot_.motion.max_speed, scale);
    }

    void set_order_ring_throttle_0080d9b0(std::uint32_t, float throttle) override {
        // 008235D5 with ECX = unit+838h. The reconstruction fills the pending
        // span and writes the live field, which is ring+148h.
        bsp::set_unit_order_ring_param_a_0080d9b0(slot_.ring, throttle);
        owner_.done("SceneStartSpeed::set_order_ring_throttle", 0x0080d9b0u);
    }

    void set_controller_axial_speed_0092d770(std::uint32_t, float speed) override {
        // 008235F7 with ECX = [unit+1018h]. The body axis is the pose's row 2,
        // the same forward vector 0092D730 projects onto.
        bsp::OceanVec3 axis{};
        axis.x = slot_.motion.pose_row2[0];
        axis.y = slot_.motion.pose_row2[1];
        axis.z = slot_.motion.pose_row2[2];
        slot_.motion.linear_velocity
            = bsp::unit_set_axial_speed_0092d770(slot_.motion.linear_velocity, axis, speed);
        slot_.motion_state.linear_velocity = slot_.motion.linear_velocity;
        owner_.done("SceneStartSpeed::set_controller_axial_speed", 0x0092d770u);
    }

    int reference_calls{0};

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
    const GameSceneEntityRecord& entity_;
};

}  // namespace units_local  // namespace

// ---------------------------------------------------------------------------
// GameUnitsHost
// ---------------------------------------------------------------------------

inline namespace units_local {
// Packet cc13-leak92: the units host the session pump delivers 92h to, as
// script_orders_drain_loopback_0076c600 finds its host. Set only with the switch ON.
inline GameUnitsHost::Impl* g_leak92_units = nullptr;
}  // namespace units_local

inline namespace units_local {
inline bool land_name_equal(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i]))
            != std::tolower(static_cast<unsigned char>(b[i]))) return false;
    }
    return true;
}

// 007B03C0's path half: the local sampler's record lookups against the knot
// storage this convoy owns. The pose resolver is never entered by 007AFE80.
class LandConvoyPathHost final : public bsp::CameraPathHost {
public:
    LandConvoyPathHost(LandConvoyMotionState& c, bsp::CameraPathView& view, GameHostLog& log)
        : c_(c), view_(view), log_(log) {}
    bsp::CameraPathView& resolve_path(void*) override { return view_; }
    float* resolve_path_knot_words(void* actual_knot) override {
        return static_cast<float*>(actual_knot);
    }
    void range_error_00bf6713() override { ++c_.range_errors; }
    bsp::PoseRefreshView& resolve_pose(void*) override {
        log_.unimplemented("LandConvoy::path_pose", "007b03c0");
        throw std::runtime_error("the convoy path sampler asked for a pose");
    }

private:
    LandConvoyMotionState& c_;
    bsp::CameraPathView& view_;
    GameHostLog& log_;
};
}  // namespace units_local  // namespace

// A borrowed view of the actual convoy member's retained +74h/C8h/10Ch cells.
// This host flattens scene parents into world coordinates at creation and in
// convoy placement; its PoseRefreshViews have no child links (create_units).
// Check that domain before writing instead of fabricating a native hierarchy.
class ConvoyMemberLocalMatrixBinding final : public bsp::EntityLocalMatrixHost {
public:
    ConvoyMemberLocalMatrixBinding(GameUnitsHost::Impl& host, std::uint32_t member)
        : member_(member), slot_(*host.slots.at(member - 1u)) {
        if (!slot_.pose || slot_.parent != nullptr)
            throw std::logic_error("convoy local-matrix binding requires a flat member pose");
        for (const auto& other : host.slots) {
            if (other && other->parent == slot_.pose.get())
                throw std::logic_error("convoy local-matrix binding has an unrepresented child list");
        }
    }
    void copy_local_matrix(std::uint32_t entity, const float source[16]) override {
        require_member(entity);
        bsp::copy_native_camera_matrix_004134f0(slot_.local.data(), nullptr, source);
    }
    void clear_pose_cache_flags(std::uint32_t entity) override {
        require_member(entity);
        slot_.world_valid = 0;
        slot_.derived_valid = 0;
    }
    std::uint32_t first_child(std::uint32_t entity) override {
        require_member(entity);
        return 0; // The actual parent links were checked above; the child set is empty.
    }
    std::uint32_t next_sibling(std::uint32_t) override {
        throw std::logic_error("flat convoy member has no child sibling");
    }
    void invalidate_subtree_pose(std::uint32_t) override {
        throw std::logic_error("flat convoy member has no child subtree");
    }
    void notify_local_matrix_owner(std::uint32_t) override {
        // The base setter never takes 006E00A0's embedded-owner arm.
        throw std::logic_error("convoy base matrix setter has no embedded-owner notification");
    }
private:
    void require_member(std::uint32_t entity) const {
        if (entity != member_)
            throw std::logic_error("convoy matrix setter addressed a different member");
    }
    std::uint32_t member_;
    GameUnitSlot& slot_;
};

// The placement host for 00742400. Member handles are unit index + 1.
class LandConvoyPlacementBinding final : public bsp::LandConvoyPlacementHost {
public:
    LandConvoyPlacementBinding(GameUnitsHost::Impl& host, LandConvoyMotionState& c)
        : host_(host), c_(c) {}
    int member_count() override { return static_cast<int>(c_.member_units.size()); }
    std::uintptr_t member_at(int index) override {
        const std::size_t u = c_.member_units[static_cast<std::size_t>(index)];
        return u == static_cast<std::size_t>(-1) ? 0 : static_cast<std::uintptr_t>(u) + 1u;
    }
    std::uintptr_t convoy_parent() override { return c_.parent_scene_id; }
    // 007B03C0: 007AFE80 in path space, then (p, 1.0f) through the path
    // entity's local matrix with the homogeneous divide, and the direction
    // through 0042D0D0 with normalise = 0 (no divide).
    void sample_path_007b03c0(float arc, bsp::LandVec3* position, bsp::LandVec3* tangent) override {
        bsp::CameraPathView view{c_.knots_begin_08, c_.knots_end_0c, c_.path_parent_14,
                                 c_.closed_24};
        LandConvoyPathHost path_host(c_, view, host_.log);
        std::array<float, 3> p{};
        std::array<float, 3> d{};
        bsp::sample_camera_path_local_007afe80(view, arc, p, &d, 0u, path_host);
        const float source[4] = {p[0], p[1], p[2], 1.0f};        // 00D7A24C
        float out[4] = {};
        bsp::transform_native_vector4_00b62d10(source, out, c_.path_local_74);
        const float inverse = static_cast<float>(1.0 / static_cast<double>(out[3]));
        position->x = inverse * out[0];
        position->y = out[1] * inverse;
        position->z = inverse * out[2];
        bsp::AdvanceMatrix m{};
        std::memcpy(m.m, c_.path_local_74, sizeof(m.m));
        float direction[3] = {};
        bsp::transform_direction_0042d0d0(direction, d.data(), m);
        tangent->x = direction[0];
        tangent->y = direction[1];
        tangent->z = direction[2];
    }
    void orthonormalize_frame_0085dc80(bsp::LandPoseMatrix* matrix) override {
        bsp::orthonormalize_pose_matrix_0085dc80(matrix->m.data());
    }
    bsp::LandVec3 world_up_00f8758c() override {
        return {bsp::kWorldUp00f8758c[0], bsp::kWorldUp00f8758c[1], bsp::kWorldUp00f8758c[2]};
    }
    // The parent's world frame is the composed scene frame; nothing to refresh.
    void refresh_world_pose_00414db0(std::uintptr_t) override {}
    float terrain_height_0087fa20(std::uintptr_t, float x, float z) override {
        SceneTerrainForwarders terrain(*c_.terrain);
        return bsp::landscape_lua_seg55_argon::landscape_forward_local_height_0087fa20(
            terrain.landscape(), x, z);
    }
    float parent_height_origin_00742792(std::uintptr_t) override {
        return c_.parent_world_cc[13];                  // parent+100h
    }
    bsp::LandVec3 terrain_normal_0087fb90(std::uintptr_t, float x, float z) override {
        SceneTerrainForwarders terrain(*c_.terrain);
        bsp::landscape_lua_seg55_argon::Vec3 normal;
        bsp::landscape_lua_seg55_argon::landscape_forward_local_normal_0087fb90(
            terrain.landscape(), &normal, x, z);
        return {normal.x, normal.y, normal.z};
    }
    void orthonormalize_from_normal_0085dad0(bsp::LandPoseMatrix* matrix,
                                             const bsp::LandVec3& normal) override {
        bsp::AdvanceMatrix a{};
        std::memcpy(a.m, matrix->m.data(), sizeof(a.m));
        a.m[4] = normal.x;
        a.m[5] = normal.y;
        a.m[6] = normal.z;
        bsp::orthonormalize_up_first_0085dad0(a);
        std::memcpy(matrix->m.data(), a.m, sizeof(a.m));
    }
    // The terrain law: member+74h is the local matrix under the convoy's
    // parent; the host's pose rows are the world frame, local x parent.
    void write_member_local_matrix(std::uintptr_t member,
                                   const bsp::LandPoseMatrix& matrix) override {
        bsp::AdvanceMatrix local{};
        bsp::AdvanceMatrix parent{};
        bsp::AdvanceMatrix world{};
        std::memcpy(local.m, matrix.m.data(), sizeof(local.m));
        std::memcpy(parent.m, c_.parent_world_cc, sizeof(parent.m));
        bsp::multiply_00413920(world, local, parent);
        // 00431410..00431444 (bsp.py show 00431410 --asm): copy the
        // complete matrix, clear C8h/10Ch, invalidate the actual children.
        // The generated convoy members are LandFort instances (00747000,
        // vtable 00CFF3F8), whose +88h slot is this base setter; see
        // ENTITY_LOCAL_MATRIX.md. 00742400's terrain arm previously wrote
        // the same pose directly here. Keep the existing parent flattening
        // above, then set the member's real canonical local matrix and
        // consume its lazy-refreshed world matrix in motion/row publication.
        // No second pose owner, shadow matrix or float arithmetic is added.
        const auto entity = static_cast<std::uint32_t>(member);
        GameUnitSlot& slot = *host_.slots.at(static_cast<std::size_t>(member - 1u));
        ConvoyMemberLocalMatrixBinding binding(host_, entity);
        bsp::set_entity_local_matrix_00431410(binding, entity, world.m);
        bsp::refresh_pose_00414db0(*slot.pose);
        for (int i = 0; i < 3; ++i) {
            slot.motion.pose_row0[i] = slot.world[i];
            slot.motion.pose_row1[i] = slot.world[4 + i];
            slot.motion.pose_row2[i] = slot.world[8 + i];
            slot.motion.position[i] = slot.world[12 + i];
        }
        // Retain publish_pose's existing diagnostic verbatim. The setter
        // and refresh now own matrix publication; do not regenerate the
        // same matrices a second time through write_world/publish_pose.
        // pc24: this copied double acos chain only observes diagnostic tilt.
        double uy = slot.motion.pose_row1[1];
        if (uy > 1.0) uy = 1.0;
        if (uy < -1.0) uy = -1.0;
        const float tilt = static_cast<float>(std::acos(uy) * 57.29577951308232);
        if (tilt > slot.diag_hull_tilt_max_deg) slot.diag_hull_tilt_max_deg = tilt;
        host_.refresh_row(slot);
        ++c_.placements;
        host_.done("LandConvoy::set_member_local_matrix_00431410", 0x00431410u);
    }
    void invalidate_member_subtree_0042ed50(std::uintptr_t) override {}
    void set_member_world_pose_00741e90(std::uintptr_t member,
                                        const bsp::LandPoseMatrix& matrix) override {
        write_world(member, matrix.m.data());
    }
    void notify_member_scene_node(std::uintptr_t) override {}

private:
    void write_world(std::uintptr_t member, const float* m) {
        GameUnitSlot& slot = *host_.slots[static_cast<std::size_t>(member - 1u)];
        for (int i = 0; i < 3; ++i) {
            slot.motion.pose_row0[i] = m[i];
            slot.motion.pose_row1[i] = m[4 + i];
            slot.motion.pose_row2[i] = m[8 + i];
            slot.motion.position[i] = m[12 + i];
        }
        GameUnitsHost::Impl::publish_pose(slot);
        host_.refresh_row(slot);
        ++c_.placements;
    }

    GameUnitsHost::Impl& host_;
    LandConvoyMotionState& c_;
};

// Packet cc13-convoy, under kLandConvoyBodiesBound: the host side of cc15's
// three land convoy bodies and cc13's 00742210 / 0074CD10. A convoy pointer is
// its LandConvoyLifeState, a member pointer is unit index + 1.
inline namespace units_local {
inline LandConvoyLifeState& convoy_life(std::uint8_t* convoy) {
    return *reinterpret_cast<LandConvoyLifeState*>(convoy);
}
inline std::uint8_t* convoy_handle(LandConvoyLifeState& life) {
    return reinterpret_cast<std::uint8_t*>(&life);
}
inline std::uint8_t* member_handle(std::size_t unit) {
    return reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(unit) + 1u);
}
inline std::size_t member_unit(std::uint8_t* member) {
    return static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(member) - 1u);
}

// The member walk every body shares (LandConvoyMemberSlots).
template <class Seam>
class LandConvoySlotsAdapter : public Seam {
public:
    explicit LandConvoySlotsAdapter(GameUnitsHost::Impl& host) : host_(host) {}
    std::size_t member_slot_count(std::uint8_t* convoy) override {
        return convoy_life(convoy).slots.size();
    }
    std::uint8_t* member_slot(std::uint8_t* convoy, std::size_t i) override {
        const std::size_t unit = convoy_life(convoy).slots[i];
        return unit == static_cast<std::size_t>(-1) ? nullptr : member_handle(unit);
    }
    // The image reports and carries on; the index model never reaches it.
    void invalid_parameter_noinfo_00bf6713() override {
        host_.log.notef("  LandConvoy: checked-iterator report 00BF6713 (packet cc13-convoy)");
    }
    // MOV dword [slot], 0: the one vector +398h, which the motion state's
    // placement walk (00742400, land_and_structures.cpp) reads too.
    void null_slot(std::uint8_t* convoy, std::size_t i) {
        LandConvoyLifeState& life = convoy_life(convoy);
        life.slots[i] = static_cast<std::size_t>(-1);
        for (LandConvoyMotionState& c : host_.land_convoys) {
            if (c.name != life.name) continue;
            if (i < c.member_units.size()) c.member_units[i] = static_cast<std::size_t>(-1);
            break;
        }
    }
    // 00926D90 on a member: returns at once when +5Fh is set, else the kill. The
    // host's one death route is the gunnery funnel, as for the Lua Kill victims.
    // SUBSTITUTION (labelled): it kills now; the image queues on 00F899B4.
    void kill_member(std::uint8_t* member, std::int32_t cause) {
        const std::size_t unit = member_unit(member);
        GameGunneryHost* gunnery = host_.gunnery.get();
        if (gunnery == nullptr || unit >= host_.slots.size() || gunnery->unit_dead(unit)) return;
        gunnery->kill_unit_00926d90(unit, cause);
    }
    // 00926D90 on the convoy: +5Fh, +70h, and the kill list. Its +080h
    // (00742180) runs in the same 009273A0 flush, on the loop's next pass
    // (land_convoy_flush_009273a0).
    void kill_convoy(std::uint8_t* convoy, std::int32_t cause) {
        LandConvoyLifeState& life = convoy_life(convoy);
        if (life.killed_5f) return;                                  // 00926DE1
        const std::int32_t stored = cause == 7 ? 2 : cause;          // 00926DAD..00926DBC
        life.killed_5f = true;                                       // 00926DF3
        if (!life.pending_destroy_60) {                              // 00926DEF
            life.cause_70 = stored;                                  // 00926E02
            // 00926E05 vtable+70h(1) is 0077D1A0 for a convoy, whose 00926C80
            // Destroy (0077D243) queues it on the destroy list 00F899A8. With
            // kLandConvoyDeadWired, that chain's end, 00929800's Lua `Dead` /
            // `KillReason` write, is modelled in land_convoy_flush_009273a0.
            // With it off, nothing of the call is modelled. Either way the rest
            // (broadcast, FireEntityKilled, child loop, 00925C90, 0077D270's
            // stores, 00927F60 / 00928100) is OMITTED (labelled): the list is
            // in land_convoy_dead_cc13.hpp.
        }
        if (cause == 7) life.cause_70 = stored;                      // 00926E07..00926E0C
        // OMITTED (labelled): 00926E0F..00926E23, 00926D90(cause) on each child of
        // the +48h / +44h chain; a convoy marker here has no child entities.
        life.kill_queued = true;                                     // 00F899B4
        host_.log.notef("  LandConvoy \"%s\": 00926D90 cause %d at live count %d, 00742180 "
            "queued for the flush loop's next pass (packet cc13-convoy)", life.name.c_str(),
            static_cast<int>(cause), static_cast<int>(life.live_3bc));
    }

protected:
    GameUnitsHost::Impl& host_;
};

// 00742180 (convoy +080h).
class LandConvoyKillBinding final
    : public LandConvoySlotsAdapter<bsp::cc15_land_convoy::LandConvoyKillHost> {
public:
    using LandConvoySlotsAdapter::LandConvoySlotsAdapter;
    void clear_member_convoy_738(std::uint8_t* member) override {
        const std::size_t unit = member_unit(member);
        for (auto& life : host_.land_convoy_lives) {
            for (LandConvoyLifeState::Member& m : life->members) {
                if (m.unit == unit) m.convoy_738 = nullptr;
            }
        }
    }
    std::int32_t convoy_kill_cause_70(std::uint8_t* convoy) override {
        return convoy_life(convoy).cause_70;
    }
    void mission_entity_kill_00926d90(std::uint8_t* entity, std::int32_t cause) override {
        kill_member(entity, cause);
    }
    // 00779AF0 -> 00928C80: the Lua on-killed release. RESIDUAL (labelled): the
    // convoy is a scene marker here; nothing releases its Lua table, and nothing
    // sets its `Dead` field (docs/MISSION_ENTITY_LUA_ATTACH.md, open).
    void mission_entity_on_killed_00779af0(std::uint8_t* convoy) override {
        host_.log.notef("  LandConvoy \"%s\": 00742180 ran, tail 00779AF0 not modelled for "
            "a marker (packet cc13-convoy)", convoy_life(convoy).name.c_str());
    }
};

// 007422A0, reached from 0074CD10.
class LandConvoyRemoveBinding final
    : public LandConvoySlotsAdapter<bsp::cc15_land_convoy::LandConvoyRemoveHost> {
public:
    using LandConvoySlotsAdapter::LandConvoySlotsAdapter;
    void null_member_slot(std::uint8_t* convoy, std::size_t i) override {
        null_slot(convoy, i);
    }
    std::int32_t live_count_3bc(std::uint8_t* convoy) override {
        return convoy_life(convoy).live_3bc;
    }
    void set_live_count_3bc(std::uint8_t* convoy, std::int32_t count) override {
        convoy_life(convoy).live_3bc = count;
    }
    void mission_entity_kill_00926d90(std::uint8_t* entity, std::int32_t cause) override {
        kill_convoy(entity, cause);
    }
};

// 00742210, from the Lua Kill binding. The members it hands to 00926D90 go
// back to the caller with the cause the body computed (0074226A..00742271),
// and the caller kills them as it kills any Lua Kill victim.
class LandConvoyKillVectorBinding final
    : public LandConvoySlotsAdapter<bsp::cc13_seg41::LandConvoyKillVectorHost> {
public:
    LandConvoyKillVectorBinding(GameUnitsHost::Impl& host,
                                std::vector<GameUnitsHost::LandConvoyVictim>& victims)
        : LandConvoySlotsAdapter(host), victims_(victims) {}
    void mission_entity_kill_00926d90(std::uint8_t* entity, std::int32_t cause) override {
        victims_.push_back({member_unit(entity), static_cast<int>(cause)});
    }
    void null_member_slot(std::uint8_t* convoy, std::size_t i) override {
        null_slot(convoy, i);
    }

private:
    std::vector<GameUnitsHost::LandConvoyVictim>& victims_;
};

// 0074CD10 (member +080h). Its only reads are member +738h and the two calls;
// the member is a 73Ch-byte shadow carrying +738h, every other hook is unreached.
class LandVehicleOnDeathBinding final : public bsp::cc13_seg41::LandVehicleNullHost {
public:
    LandVehicleOnDeathBinding(GameUnitsHost::Impl& host, std::size_t unit)
        : host_(host), unit_(unit) {}
    void unreached(const char* hook, std::uint32_t) override {
        host_.log.notef("  LandVehicle 0074CD10: unexpected hook %s (packet cc13-convoy)", hook);
    }
    void convoy_member_died_007422a0(std::uint8_t* convoy, std::uint8_t*) override {
        LandConvoyRemoveBinding remove(host_);
        LandConvoyLifeState& life = convoy_life(convoy);
        const std::int32_t before = life.live_3bc;
        const std::size_t slot = bsp::cc15_land_convoy::land_convoy_remove_member_007422a0(
            convoy, member_handle(unit_), remove);
        host_.log.notef("  LandConvoy \"%s\": member unit %zu died, 007422A0 %s, live %d -> %d "
            "(packet cc13-convoy)", life.name.c_str(), unit_,
            slot == bsp::cc15_land_convoy::kMemberNotFound ? "found no slot" : "nulled its slot",
            static_cast<int>(before), static_cast<int>(life.live_3bc));
        host_.done("LandConvoy::remove_member", 0x007422a0u);
    }
    // JMP 00951FB0: the unit's own death route, which the gunnery funnel ran.
    void unit_on_death_00951fb0(std::uint8_t*) override {}

private:
    GameUnitsHost::Impl& host_;
    std::size_t unit_;
};

// 007420B0 (convoy +0A0h), at the host's bind. The property bag, its "Path"
// property and the registry lookup are answered from the record the host
// already resolved; the skill broadcast goes to the members' 006D1EB0.
class LandConvoyBindBinding final
    : public LandConvoySlotsAdapter<bsp::cc15_land_convoy::LandConvoyBindHost> {
public:
    LandConvoyBindBinding(GameUnitsHost::Impl& host, GameUnitsHost& units,
                          const GameSceneEntityRecord& convoy, const GameSceneEntityRecord* path)
        : LandConvoySlotsAdapter(host), units_(units), convoy_(convoy), path_(path) {}
    // 0077F0E0: the host registered the convoy's marker at its creation.
    void unit_owner_activate_and_register_0077f0e0(std::uint8_t*) override {}
    // SUBSTITUTION (labelled): the record that carries the roster keys stands for
    // the bag at +C0h, taken as state 1 (bind_land_convoy_motion returns before
    // this for a record without them).
    void* property_bag_c0(std::uint8_t*) override {
        return const_cast<GameSceneEntityRecord*>(&convoy_);
    }
    std::int32_t property_bag_state(void*) override { return 1; }
    void* property_bag_records(void* bag) override { return bag; }
    void* scene_property_bag_find_008f2260(void*, const char*) override {
        return const_cast<std::string*>(&convoy_.convoy_path);
    }
    const char* scene_property_string_008f0df0(void* property) override {
        return static_cast<const std::string*>(property)->c_str();
    }
    void* entity_registry_30(std::uint8_t*) override { return nullptr; }
    // SUBSTITUTION (labelled): bind_land_convoy_motion's qualified match stands
    // for 00925A90.
    void* entity_registry_find_by_name_00925a90(void*, const char*) override {
        return const_cast<GameSceneEntityRecord*>(path_);
    }
    void* entity_path_interface_for_kind_007ac9d0(void* entity) override { return entity; }
    void set_path_interface_344(std::uint8_t*, void* path_interface) override {
        path_interface_ = path_interface;
    }
    std::int32_t property_bag_get_skill_00927a80(std::uint8_t*) override {
        return convoy_.bag_skill;
    }
    void member_set_skill_vt128(std::uint8_t* member, std::int32_t skill) override {
        if (units_.set_unit_skill_index_006d1eb0(member_unit(member), skill)) ++skilled_;
    }
    void* path_interface() const { return path_interface_; }
    std::size_t skilled() const { return skilled_; }

private:
    GameUnitsHost& units_;
    const GameSceneEntityRecord& convoy_;
    const GameSceneEntityRecord* path_;
    void* path_interface_{nullptr};
    std::size_t skilled_{0};
};

inline LandConvoyLifeState* find_convoy_life(GameUnitsHost::Impl& host, const std::string& name) {
    for (auto& life : host.land_convoy_lives) {
        if (life->name == name) return life.get();
    }
    return nullptr;
}
}  // namespace units_local

inline namespace units_local {
// 00743060 (+4h, RET 4): when +3A8h is clear, live += t * speed * dir and wrap
// with two loops, odometer += t * |speed|; then 00742400 in every case.
inline void land_convoy_advance_and_place(GameUnitsHost::Impl& host, LandConvoyMotionState& c,
                                   float t) {
    const bsp::LandConvoyArcStep step = bsp::land_convoy_advance_arc_00743060(
        c.formation, c.live_arc_3ac, c.odometer_3b4, t);
    c.live_arc_3ac = step.live_arc;
    c.odometer_3b4 = step.odometer;
    LandConvoyPlacementBinding binding(host, c);
    bsp::land_convoy_place_members_00742400(c.formation, c.live_arc_3ac,
                                            c.world_pose_law_3c8, binding);
}
}  // namespace units_local

// ---------------------------------------------------------------------------
// Packet cc9_landing_sequencer (kLandingSequencerBound): the deck's landing
// holder (block+80h), queue (block+98h), assignment vector (block+A8h) and the
// sequencer that fills it. Every body was read from the listing, and each
// 00419010 call was traced through its pushes: docs/SQUADRON_LAND_TASK.md
// sections 5b and 5c. The x87 intermediates are kept at the widths the listing
// stores (float spills, double constants); the extended-precision internals of
// a single expression are computed in double. Airfield owners only.
// ---------------------------------------------------------------------------

inline namespace units_local {

inline std::array<float, 3> landing_xform_004142e0(const bsp::CameraMatrix& m, const float* p) {
    std::array<float, 3> out{};
    bsp::transform_point_004142e0(std::array<float, 3>{p[0], p[1], p[2]}, m, out);
    return out;
}

inline float landing_len2_00414c60(float x, float z) {
    return bsp::length_2d_00414c60(std::array<float, 2>{x, z});
}

inline bool landing_airborne(const GameUnitSlot& p) {
    return p.plane_control_mode_900 == 7;   // (plane+72Ch)->vtable[38h], as the land task reads it
}

}  // namespace units_local

inline namespace units_local {
// Adapter for 006BCC90's holder: the two image cells it reads, the inverse
// (holder+48h, 0085DEA0's 4x4) and T (holder+A4h..+ACh, 006BC960), copied from
// the host's LandingDeck into a query-local image so the native body reads them
// at their offsets. Nothing else in the holder is read by 006BCC90..006BCCD9.
struct LandingHolderLocalImage {
    std::uint8_t unread_00[0x48];
    bsp::CameraMatrix inverse_48;
    std::uint8_t unread_88[0x1c];
    float t_a4[3];
};
static_assert(sizeof(bsp::CameraMatrix) == 0x40);
static_assert(offsetof(LandingHolderLocalImage, inverse_48) == 0x48);
static_assert(offsetof(LandingHolderLocalImage, t_a4) == 0xa4);
}  // namespace units_local

inline namespace units_local {

// 0047B850: vtable[5Ch](10h) or vtable[5Ch](16h).
inline bool park_bomber_class_0047b850(const GameUnitSlot& u) {
    return bsp::unit_is_kind_of(u.class_id, 0x10) || bsp::unit_is_kind_of(u.class_id, 0x16);
}

// The entity's derived affine inverse (00414E10) applied to a world point: the
// point in the unit's own frame, the world rows being its basis.
inline std::array<float, 3> park_point_in_frame(const GameUnitSlot& u, const float* w) {
    const float d[3] = {w[0] - u.motion.position[0], w[1] - u.motion.position[1],
                        w[2] - u.motion.position[2]};
    const float* r[3] = {u.motion.pose_row0, u.motion.pose_row1, u.motion.pose_row2};
    std::array<float, 3> l{};
    for (int i = 0; i < 3; ++i) l[i] = r[i][0] * d[0] + r[i][1] * d[1] + r[i][2] * d[2];
    return l;
}

}  // namespace units_local

// Packet cc9_carrier_landing_deck_part2 (kCarrierDeckParentBound), 5ag.
// The carrier's world matrix: point = l0 R0 + l1 R1 + l2 R2 + T, rows at
// [0..2], [4..6], [8..10], T at [12..14] (publish_pose's layout).
inline namespace units_local {
inline void deck_to_local(const bsp::CameraMatrix& w, const float* v, bool point, float* out) {
    float d[3] = {v[0], v[1], v[2]};
    if (point) {
        d[0] -= w[12];
        d[1] -= w[13];
        d[2] -= w[14];
    }
    for (int i = 0; i < 3; ++i) {
        const std::size_t r = static_cast<std::size_t>(4 * i);
        out[i] = w[r] * d[0] + w[r + 1] * d[1] + w[r + 2] * d[2];
    }
}
inline void deck_to_world(const bsp::CameraMatrix& w, const float* l, bool point, float* out) {
    for (int c = 0; c < 3; ++c) {
        const std::size_t k = static_cast<std::size_t>(c);
        out[c] = l[0] * w[k] + l[1] * w[4 + k] + l[2] * w[8 + k] + (point ? w[12 + k] : 0.0f);
    }
}
}  // namespace units_local

// ---------------------------------------------------------------------------
// Packet cc9_carrier_elevator (kCarrierElevatorBound), docs/SQUADRON_LAND_TASK.md
// 5ao. Every body was read from the disk listing.
// ---------------------------------------------------------------------------

// planeDesc+158h (007D473A: (model+3Ch + model+30h) x 0.5, the model's box
// centre along z). With kPlaneDesc158Bound off it is 0.0, the plane's origin.
// Read through Impl::plane_desc_158 (kPlaneDesc158Bound, include/bsp/game_hosts_units.hpp).
inline namespace units_local { constexpr float kPlaneDesc158 = 0.0f; }  // namespace units_local

// 006CF980 (the airfield launch site's tick, 006CF980-006CF9DB, RET 4): +1Ch set
// runs 006CEDD0 (not read; this host never sets +1Ch, as in 006D0600's note);
// with a ready plane +18h, a message built by 006BDAC0(plane) is routed to the
// airfield (site+44h) through 0077C2A0(msg, 5, 0), whose handler reaches the
// site's vtable[14h] 006CF9F0 BSP_AirOpsSite_PlacePlaneOnSpot (AIRFIELD_TAXI 7).
// LABELLED: delivered in the same step. 006CF9F0 (006CF9F0-006CFA5D, RET 4):
// with the airfield (site+44h) and its block owner (+7A8h) alive (+5Dh),
// 007C5F60(plane, airfield+72Ch), 007C3C90(plane, 1), 0042ED50 and
// (plane+310h)->vtable[8](0.0) (recorded); then, on both paths, the observer
// at site+4h is unregistered and site+18h = 0 (006CFA43-006CFA54).
inline namespace units_local {
// Chain 73: 007C3C90 BSP_Plane_RequestGroundState(flag) (007C3C90-007C3CA7) is
// 007C1570(4 + flag, 0). The typed 007C1570 body reads +900h and the session
// test (game+1FE4h == 2, the session context). Its C3h message reaches 007CCFA0,
// whose requested-4/5 arm is this host's 007CA3F0 (delivered at once, as the OFF
// call is). Its mode-2 arm calls 007C1430 directly: set_flight_state_007c1430's
// effects on the slot, then 007C11E0 when the state changed (007C14C5).
// A template only so that the OFF build, which never names it, has no
// unreferenced internal function.
template <class ImplT, class SlotT, class DeckT>
void request_ground_state_007c3c90(ImplT& h, SlotT& p, DeckT& d, bool flag) {
    bsp::cc13_seg47b::plane_request_flight_state_007c1570(p.plane_control_mode_900,
        bsp::session_context().session_word_1fe4 == 2, 4 + (flag ? 1 : 0), false,
        [&h, &p, &d, flag](std::uint8_t current, std::uint8_t requested) {
            // 007C1570's C3h (current, 4 + flag) goes through 0076E520's
            // local-copy codec before the existing immediate delivery.
            // This callback only posts states 4/5, whose copied selector
            // chooses the same 007CA3F0 flag as the former direct call.
            const auto copy = h.commands.session_messages().queue_plane_change_state_c3(
                current, requested);
            h.ground_state_from_locked_007ca3f0(p, d, copy.requested == 5);
        },
        [&h, &p](std::int32_t wanted) {
            bsp::PlaneSetStateInputs si;
            si.requested_state = wanted;
            si.current_state = p.plane_control_mode_900;
            si.airborne_clock = p.plane_airborne_908;
            si.class_min_water_spd = p.plane_min_water_spd_198;
            si.landed_after_flight = p.plane_landed_904;
            const bsp::PlaneSetStateEffects fx = bsp::set_flight_state_007c1430(si);
            p.plane_control_mode_900 = fx.state;
            if (fx.write_state_change_stamp) p.plane_site_timer_c04 = -1.0f;
            if (fx.write_landed_after_flight) p.plane_landed_904 = fx.landed_after_flight;
            if (fx.write_state_enter_marker) p.plane_c18 = 3;
            if constexpr (bsp::kPlaneFlightStateNotifyCc13Bound) {
                if (fx.changed) h.flight_state_notify_007c11e0(p);
            }
        });
}
}  // namespace units_local

inline namespace units_local {
// Chain 28 adapters (kWireGoal2Chain28Bound). The host keeps a plane's pose rows
// in world (pose_row_i is body axis i in world) and keeps the parent-relative
// velocity of a deck-parented plane in world axes as well (LABELLED at 007C71E0).
//
// ctl+0B0h as 007D9C10 reads it: 0085DEA0 rebuilds it from unit+74h as the
// transpose, so column i of the 3x3 block is body axis i. 0042D0D0 reads only
// that block (no translation row), so the translation row is left zero.
inline bsp::AdvanceMatrix host_pose_world_to_body(const GameUnitSlot& u) {
    bsp::AdvanceMatrix m{};
    const float* const rows[3] = {u.motion.pose_row0, u.motion.pose_row1, u.motion.pose_row2};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) m.m[4 * j + i] = rows[i][j];
    }
    m.m[15] = 1.0f;
    return m;
}

// 007D9CE0's frame callbacks in host axes. The image rotates by the old
// parent's +CCh and the new parent's +110h; the host stores the relative
// velocity in world axes, so both frame changes are the identity here. The
// parent velocities are vtable[34h] (007BBB70 for a unit): the parent slot's
// motion.linear_velocity, the term the OFF arms add and subtract.
inline bsp::gameplay_seg48_cc13::Vec3 host_unit_velocity(const GameUnitSlot* u) {
    if (u == nullptr) return {};
    return {u->motion.linear_velocity.x, u->motion.linear_velocity.y,
            u->motion.linear_velocity.z};
}
inline bsp::gameplay_seg48_cc13::ReparentCallbacks host_reparent_callbacks(
    const GameUnitSlot* old_parent, const GameUnitSlot* new_parent) {
    using bsp::gameplay_seg48_cc13::Vec3;
    bsp::gameplay_seg48_cc13::ReparentCallbacks cb;
    cb.old_parent_to_world = [](const Vec3& v) { return v; };
    cb.old_parent_velocity = [old_parent] { return host_unit_velocity(old_parent); };
    cb.world_to_new_parent = [](const Vec3& v) { return v; };
    cb.new_parent_velocity = [new_parent] { return host_unit_velocity(new_parent); };
    return cb;
}
}  // namespace units_local

// Packet cc9_squadron_spent_ordnance_rtb (bsp::kSquadronSpentOrdnanceRtbBound),
// docs/SQUADRON_LAND_TASK.md 5eb: 009F8160's ordnance latch, 009F7C90's
// ending and 0084E010's B5 per squadron, once per simulation step.
// SUBSTITUTIONS, labelled: the squadron's current command is its flight
// leader's (the host fuses them); the queue holds one command, so
// 0071BE60() < 2 holds; sq+368h is 0 (007F2C60 never writes it) and sq+369h is
// kLuaDeviceReloadEnabledBound's default 1; the class's fighter test is the
// leader's IsKindOf(13h); chain419 binds 009F7C90's rocket arm (007ED920)
// below, retaining the same class/reload providers; 007EE6B0 reads each member's
// dive-bomb (009C8A90) or torpedo (009D4C10) break-off answer, a member with
// neither task counting as broken off (007EE6E0's empty-list skip); B5 issues
// `returntobase` on the squadron (issue_return_to_base_007f16d0), whose
// 007F16D0 resolution and bot intake install what the image issues directly.
// POSITION, labelled: the image runs this inside each squadron's 0071F290; the
// host runs it at the head of the landing queue's step.
inline namespace units_local {
constexpr std::size_t kInvalidFreeAttackUnit = ~static_cast<std::size_t>(0);

inline bool free_attack_census_enabled() {
    static const bool value = [] {
        char* text = nullptr;
        std::size_t bytes = 0;
        bool on = false;
        if (_dupenv_s(&text, &bytes, "BSP_FREE_ATTACK_CENSUS") == 0 && text != nullptr)
            on = text[0] == '1';
        std::free(text);
        return on;
    }();
    return value;
}

inline const char* free_attack_stop_name(bsp::FreeAttackStop stop) {
    switch (stop) {
        case bsp::FreeAttackStop::kModeNotQueueHead: return "mode";
        case bsp::FreeAttackStop::kGateClosed: return "gate";
        case bsp::FreeAttackStop::kNoTarget: return "no_target";
        case bsp::FreeAttackStop::kRefused0071df70: return "refused_0071df70";
        case bsp::FreeAttackStop::kSameTarget: return "same_target";
        case bsp::FreeAttackStop::kNotAirTarget: return "not_air";
        case bsp::FreeAttackStop::kOrdnanceToOrdnance: return "ordnance_to_ordnance";
        case bsp::FreeAttackStop::kOverrideSent: return "override";
    }
    return "?";
}
}  // namespace units_local  // namespace

// Packet p15_free_attack_tick: what the tick's callees ask of a unit.
class FreeAttackWorld final : public bsp::SquadronFreeAttackWorld {
public:
    FreeAttackWorld(GameUnitsHost& self, GameUnitsHost::Impl& host, GameGunneryHost& gun)
        : self_(self), host_(host), gun_(gun), bounds_(host.lua.world_border_bounds()) {}

    bool is_kind_of(std::size_t unit, int kind) override { return self_.unit_is_kind_of(unit, kind); }
    std::array<float, 3> position(std::size_t unit) override {
        std::array<float, 3> p{};
        self_.unit_position_00fc(unit, p[0], p[1], p[2]);
        return p;
    }
    bool untouchable_1d4(std::size_t unit) override {
        return bsp::game::lua_unit_untouchable_1d4(unit);
    }
    bool outside_map_0071c4f0(std::size_t unit) override {
        if (bounds_ == nullptr) return false;
        const std::array<float, 3> p = position(unit);
        return bsp::point_outside_world_map_0071c4f0(*bounds_, {p[0], p[1], p[2]});
    }
    // 00811F50 with check_player = 0 (009F74C9 PUSH 0): the player tests are
    // skipped, the current command's arms are ShipOutOfMapBinding's.
    bool command_blocks_auto_action_00811f50(std::size_t unit) override {
        const std::uint32_t command = host_.commands.current_command_0071be40(unit);
        bool target_outside = false;
        bool path_outside = false;
        if (bounds_ != nullptr && (command == bsp::kCommandAttackMove || command == bsp::kCommandMoveTo)) {
            bsp::SceneCommandTarget target;
            int mode = 0;
            host_.commands.active_command_descriptor_0071eb60(unit, target, mode);
            target_outside = target.position_valid != 0
                && bsp::point_outside_world_map_0071c4f0(*bounds_,
                    {target.position[0], target.position[1], target.position[2]});
        } else if (bounds_ != nullptr && command == bsp::kCommandMoveOnPath) {
            float last[3]{};
            path_outside = host_.commands.path_final_point_007b1350(unit, last)
                && bsp::point_outside_world_map_0071c4f0(*bounds_, {last[0], last[1], last[2]});
        }
        return bsp::command_blocks_auto_action(command, target_outside, path_outside);
    }
    // 007EE8B0: outside the map and +361h (the retreat exit latch).
    bool squadron_outside_map_detached_007ee8b0(std::size_t leader) override {
        if (!outside_map_0071c4f0(leader)) return false;
        const bsp::PlaneSquadronHostRecord* sq =
            bsp::plane_squadron_registry().find_by_member_unit(leader);
        if (sq == nullptr) return false;
        for (const std::string& s : host_.retreat_latched_361) if (s == sq->name) return true;
        return false;
    }
    bool sensors_detect_00804350(std::size_t observer, std::size_t target,
                                 std::uint32_t mask) override {
        return gun_.sensors_detect_unit_00804350(observer, target, mask);
    }
    std::uint32_t choose_attack_command_007eec50(std::size_t leader, std::size_t target,
        std::uint8_t a, int b) override {
        return script_orders_choose_attack_class_007eec50(leader, target, a != 0, b != 0);
    }

private:
    GameUnitsHost& self_;
    GameUnitsHost::Impl& host_;
    GameGunneryHost& gun_;
    const bsp::WorldMapBounds* bounds_;
};

// Packet cc9_scene_unit_skill (docs/SCENE_UNIT_SKILL.md): a scene-placed unit
// starts at its bag skill instead of 1. ON: the USN04, USN01, USN13 and JM05
// 9200/9000 pairs are gameplay-identical; the only live values in reach are
// JM05's two carriers (2), which do not fire in the row.
constexpr bool kSceneUnitSkillBound = true;

inline namespace units_local {
inline const bsp::PlaneSquadronHostRecord* squadron_record_of(std::size_t unit_index) {
    bsp::PlaneSquadronRegistry& reg = bsp::plane_squadron_registry();
    for (const bsp::PlaneSquadronHostRecord& r : reg.records()) {
        if (r.squadron_unit == unit_index) return &r;
    }
    return reg.find_by_member_unit(unit_index);
}
}  // namespace units_local

// cc9-lua67 (SQUADRON_LAND_TASK 5ij). The fan-out below keys on find_by_member_unit,
// so a call on a member PLANE re-skills its siblings too. The image dispatches
// vtable[128h] on the entity: the squadron's 007ECF80 (007ECF86-007ECFB9) loops
// i < +3CCh over [+3D0h+4i]->vtable[128h](skill) and stores nothing on itself; a
// plane or ship takes 006D1EB0 / 007B8AE0 on itself only. True: the registry
// record whose fused slot (squadron_unit) is this unit re-skills its members, and
// any other unit re-skills itself only (as run_set_crew_level_00894f20 does).
// False: the member-keyed fan-out. ON (5ij.1): USN04 and BSM09 pairs, both arms live, gameplay identical.
inline constexpr bool kSkillLevelSquadronFanOutBound = true;

// Packet cc9_ship_exit_zone: the hooks of 00826D70 (bsp::ShipOutOfMapHost) for one ship.
class ShipOutOfMapBinding final : public bsp::ShipOutOfMapHost {
public:
    ShipOutOfMapBinding(GameUnitsHost& self, GameUnitsHost::Impl& host, std::size_t index,
        const bsp::WorldMapBounds& bounds)
        : self_(self), host_(host), index_(index), slot_(*host.slots[index]), bounds_(bounds) {}

    bool point_outside_map_0071c4f0() override {
        const float* p = slot_.motion.position;
        return bsp::point_outside_world_map_0071c4f0(bounds_, {p[0], p[1], p[2]});
    }
    // 00811F50 with check_player = 1 (00826DA9 PUSH 1), read whole (00811F50-0081204F).
    bool command_blocks_auto_action_00811f50() override {
        // 00811F5C 00927F30(0) and 00811F6D unit+184h, then the same two tests on
        // 007788D0's formation leader.
        if (local_player(index_) || self_.unit_player_controlled_0184(index_)) return true;
        const std::size_t leader = self_.formation_leader_0014(slot_.formation_group);
        if (slot_.formation_group >= 0 && leader < host_.slots.size()
            && (local_player(leader) || self_.unit_player_controlled_0184(leader)))
            return true;
        // 00811FA5 vtable[174h] = 0080E1B0 -> 0071BE40.
        const std::uint32_t command = host_.commands.current_command_0071be40(index_);
        bool target_outside = false;
        bool path_outside = false;
        if (command == bsp::kCommandAttackMove || command == bsp::kCommandMoveTo) {
            // 00811FEE vtable[178h] = 00812CD0 (0071EB60's descriptor): +1h set and the
            // position outside the map (0059BD20, 0071C4F0).
            bsp::SceneCommandTarget target;
            int mode = 0;
            host_.commands.active_command_descriptor_0071eb60(index_, target, mode);
            target_outside = target.position_valid != 0
                && bsp::point_outside_world_map_0071c4f0(bounds_,
                    {target.position[0], target.position[1], target.position[2]});
        } else if (command == bsp::kCommandMoveOnPath) {
            // 00812020 0071BFF0(0), 00812029 007B1350: the path's last point, when valid.
            float last[3]{};
            path_outside = host_.commands.path_final_point_007b1350(index_, last)
                && bsp::point_outside_world_map_0071c4f0(bounds_, {last[0], last[1], last[2]});
        }
        return bsp::command_blocks_auto_action(command, target_outside, path_outside);
    }
    bool point_beyond_map_margin_0059c9b0(float margin) override {
        const float* p = slot_.motion.position;
        return bsp::point_beyond_map_margin_0059c9b0(bounds_, p[0], p[2], margin);
    }
    bool formation_follower_007788b0() override {
        return self_.unit_is_formation_follower_007788b0(index_);
    }
    bool local_player_role_00927f30() override { return local_player(index_); }
    void dispatch_exit_zone_warning_00982120() override {
        // 00826E8F 00982120([00F8A0C4], unit, 0): the exitzone channel, exited = 0.
        ++host_.ship_exit_warnings;
        host_.lua.note_exit_zone_00982120(index_, false);
        host_.done("ShipUnit::exit_zone_warning_00982120", 0x00982120u);
    }
    // SUBSTITUTION, labelled: 00811E80 routes MT_UNITEXITED (50h) and 0081BEB0 runs on
    // its delivery; the host runs the handler's body here: 0081BF0C 00982120(unit, 1),
    // the exitzone channel (cc13's dispatcher; its game+18ECh gate 00927C50 under
    // kShipExitZoneRoleGateBound), then Kill(4). The Lua ExitZoneParty store is a
    // record. Labelled: the game-mode-9 return (0081BED6) is not taken (004BCA50
    // reports 8 in a single-player session unless game+614h is already 9, which this
    // host takes as never), and 0092BD30 on [unit+1018h] (0081BF4C) is not modelled.
    void route_unit_exited_00811e80() override {
        GameGunneryHost* const gun = host_.gunnery.get();
        if (gun == nullptr || gun->unit_dead(index_)) return;
        ++host_.ship_exit_kills;
        host_.log.notef("ship exit zone: unit=\"%s\" t=%.2f pos=(%.1f, %.1f) command=%s "
            "(00826D70 -> 00811E80 -> 0081BEB0 Kill(4), packet cc9_ship_exit_zone)",
            slot_.row.name.c_str(), static_cast<double>(host_.summary.simulated_seconds),
            static_cast<double>(slot_.motion.position[0]),
            static_cast<double>(slot_.motion.position[2]),
            host_.commands.command_name_of(host_.commands.current_command_0071be40(index_)));
        bool role_open = false;   // 00927C50(unit, game+18ECh)
        for (int i = 0; i < bsp::kUnitRoleTableEntries; ++i) {
            const std::int32_t w = slot_.role_permission_0188[i];
            if (w == 9 || w == GameUnitsHost::Impl::kLocalPlayerSlot) { role_open = true; break; }
        }
        if (!role_open) {
            ++host_.ship_exit_role_gated;
            host_.log.notef("ship exit zone: unit=\"%s\" has no open role word; 0081BEB0 %s the "
                "exitzone channel (00927C50 at 0081BEFA)", slot_.row.name.c_str(),
                GameUnitsHost::Impl::kShipExitZoneRoleGateBound ? "skips" : "would skip");
        }
        if (role_open || !GameUnitsHost::Impl::kShipExitZoneRoleGateBound) {
            host_.lua.note_exit_zone_00982120(index_, true);
            host_.done("ShipUnit::exit_zone_channel_0081bf0c", 0x0081bf0cu);
        }
        host_.note_kill_cause(index_, 4);
        gun->kill_unit_00926d90(index_, 4);
        host_.record("ShipUnit::exit_zone_party_lua_0081beb0", 0x0081beb0u);
    }

private:
    // SUBSTITUTION, labelled: 00927F30(0), the local player's role, is the host's
    // controlled unit.
    bool local_player(std::size_t unit) const {
        return host_.controlled_bound && host_.controlled_index == unit;
    }
    GameUnitsHost& self_;
    GameUnitsHost::Impl& host_;
    std::size_t index_;
    GameUnitSlot& slot_;
    const bsp::WorldMapBounds& bounds_;
};

inline namespace units_local {
// Read-only receiver adapter for 00927CC0 / 00927C90 (__thiscall, RET 4).
// bsp.py show 00927cc0 / 00927c90: only +188h..+1A8h are read. Copy the
// canonical table initialized to 9 by GameUnitSlot and changed by
// set_role_availability_00927d20_impl on every query. The prefix is never
// read; this is not another unit or a cached policy table.
struct UnitRolePermissionReadView {
    std::uint8_t unused[0x188];
    std::int32_t words[bsp::kUnitRoleTableEntries];
    explicit UnitRolePermissionReadView(const GameUnitSlot& slot) {
        std::memcpy(words, slot.role_permission_0188, sizeof words);
    }
    bsp::cc13_seg49::Raw raw() {
        return bsp::cc13_seg49::Raw{reinterpret_cast<std::uint8_t*>(this)};
    }
};
static_assert(bsp::kUnitRoleTableEntries == 9);
static_assert(offsetof(UnitRolePermissionReadView, words) == 0x188);
}  // namespace units_local

inline namespace units_local {
class ShipHeadingMatrixBinding final : public bsp::cc13_seg7::SceneNodeHeadingHost {
    using Object = bsp::cc13_seg7::Object;
public:
    void matrix_copy_004134f0(Object destination, Object source) override {
        bsp::copy_native_camera_matrix_004134f0(destination, nullptr, source);
    }
    Object matrix_build_rotation_y_00b646e0(Object out, const float* angle) override {
        // Immutable binary operands, 00D7A208 and 00D7A24C. The provider retains
        // B646E0's separate FSIN/FCOS spills and SSE negative-zero subtraction.
        static constexpr float negative_zero = -0.0f;
        static constexpr float one = 1.0f;
        return static_cast<Object>(bsp::build_native_particle_rotation_y_00b646e0(
            out, angle, &negative_zero, &one));
    }
    Object matrix_multiply_00413920(Object left, Object out, Object right) override {
        return static_cast<Object>(bsp::multiply_native_camera_matrices_00413920(
            left, nullptr, out, right));
    }
};

inline void set_live_ship_scene_heading(bsp::ShipMotionState& motion, float heading) {
    // bsp.py show 0047F200 --asm: 0047F24A..266 read only node+A4h..ACh;
    // 0047F2D4..2D7 publish the complete matrix at +74h. This borrowed view
    // carries the real current translation, not a second persistent node.
    alignas(4) std::uint8_t node[0xb4];
    std::memcpy(node + 0xa4, motion.position, sizeof(motion.position));
    ShipHeadingMatrixBinding binding;
    bsp::cc13_seg7::scene_node_set_heading_0047f200(binding, node, heading);
    std::memcpy(motion.pose_row0, node + 0x74, sizeof(motion.pose_row0));
    std::memcpy(motion.pose_row1, node + 0x84, sizeof(motion.pose_row1));
    std::memcpy(motion.pose_row2, node + 0x94, sizeof(motion.pose_row2));
    std::memcpy(motion.position, node + 0xa4, sizeof(motion.position));
}
}  // namespace units_local

}  // namespace bsp::game
