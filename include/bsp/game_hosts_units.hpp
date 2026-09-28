#pragma once
// bsp_game.exe milestone 2i: the created scene units, as process bindings.
//
// Addresses: 008255b0 (BSP_UnitInstance_Update, the entity vtable slot 0DCh the
// world walk calls), 00825f20 (BSP_UnitInstance_UpdateShipMotion, the separate
// motion virtual) with 00813020 (the order ring tick), 0092d300 / 0092e8c0 /
// 00937440 (the speed command, the steering half and the force model),
// 0092be80 (the controller step), 0092d730 (the body-axis forward speed),
// 00816a40 / 00815440 (the order record issue and its clamp) and 004c0890
// (BSP_Game_SetControlledUnit).
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::UnitInstanceHost, bsp::ShipMotionHost, bsp::UnitRudderHost,
// bsp::UnitOrderRecordIssueHost, bsp::ControlledUnitQuery or
// bsp::SetControlledUnitHost, satisfied either by a reconstruction already on
// main or by the explicit unimplemented policy in GameHostLog.
//
// Milestone 2j replaced the four stand-ins milestone 2i carried with the
// producers packet cc_ship_inputs recovered, which are the same four
// src/ship_motion_probe.cpp now uses:
//
//   0078cf20  the ocean sampler, run as the recovered product of the wave field
//             0078c890 and the coverage mask 00b9cf50. The two leaves are host
//             records: their receiver is [[game+19F0h]+A8h], which belongs to
//             the renderer/scene owner this process does not build, and their
//             flat-sea pair (wave 0.0f, mask 1.0f) is the evidenced open-sea
//             state of the real routine (docs/OCEAN_HEIGHT.md).
//   008e6430  the gameplay-modifier product, run over an empty category list,
//             which is the routine's own exact 1.0f rather than a literal.
//   00424c40()+438h..+44Ch  the rudder curve settings, read out of the live Lua
//             state through the reconstructed loader fragment 0083ce56 of
//             0083b5e0 over ShipGlobals["Navigator"]["TurnMultipliers"].
//   00c41550 / 00c5b1b0  the Dyn library's own two integration phases, in place
//             of the explicit Euler step. The hull body's mass, inertia and
//             damping have no recovered producer, so the body carries none and
//             the substep schedule 00c5c540 stays a record.
//
// Milestone 2l removed the last of those decisions. The authored
// `Command = E CommandType : Cruise` token is no longer turned into an
// order-ring order: it runs the recovered command path of
// include/bsp/game_hosts_commands.hpp, and `cruise` turns out to be a latch
// that captures the ring's ordered pair rather than an order that fills it.
// A ship whose ring is zero therefore holds zero, which is what the authored
// state of this mission's 32 destroyers is. `--order` still writes the
// controlled unit's ring through 00816a40, which is the player's own path.
//
// Evidence: docs/UNIT_INSTANCE_UPDATE.md, docs/SHIP_MOTION.md,
// docs/UNIT_CONTROLLER.md, docs/UNIT_CONTROLLER_UPDATE.md,
// docs/CONTROLLED_UNIT.md, docs/UNIT_STATE_MESSAGE.md,
// docs/UNIT_ORDER_RECORD.md, docs/SCENE_DEFERRED_REFS.md,
// docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "bsp/pilot_command_path.hpp"
#include "bsp/game_hosts_commands.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/hit_narrowphase.hpp"
#include "bsp/native_unit_observer_endpoint.hpp"
#include "bsp/native_scene_lifecycle_notify.hpp"
#include "bsp/ship_ai_obstacle_tables.hpp"

namespace bsp::game {

// Packet cc9_wing_construction (docs/CONSTRUCT_WORLD.md section 30). True: the
// script-orders creator batches (the air-ops launch 006C5050 and the
// GenerateObject/SpawnNew creator 0046DB4B) build a plane squadron's leader
// only and stage its wing records here; the squadron's InitAll pass A hook
// builds them through create_units, as 007F4580 constructs the wing in the
// squadron's slot-39 attach, so each plane is pushed during pass A. The Lua
// half (marking those pushes as wing members, retiring the wing append) is
// cc9-movie-camera's and flips with this one. False: the creator batches build
// the wing, as before.
// ON since the joint flip (docs/CONSTRUCT_WORLD.md 30.7), by the lead's ruling
// on 30.6's pairs.
inline constexpr bool kWingConstructionInPassABound = true;

// Packet cc9_dead_member_group_removal (docs/SHIP_AI_FORMATION.md, "A dead member
// leaves its group"). True: a destroyed ship leaves its formation group as the
// image's destroy 0077D1A0 -> 0077C980 message 77h -> 0077FE80 -> 0077BD70(null)
// makes it: a successor from 0070D8D0, 0070D0C0 SetLeader with 00815E20's wake
// hand-over, and 0070E4C0 DetachMember's compaction. LABELLED: applied at the
// row-15 flush that delivers the destroy (the image routes 77h to the next
// session pump); the director notice vtable[114h]->vtable[5Ch] (unread), the
// new leader's +FA0h/+FA8h offsets and the observer unregister are recorded
// only; 0070DA00's ceiling has no stored copy here, the host reduces members on
// demand. False: a dead ship stays in its group, as before.
inline constexpr bool kDeadMemberLeavesGroupBound = true;

// Packet cc9_land_convoy_members (docs/LAND_AND_STRUCTURES.md, "The LandConvoy
// roster, bound"). 00743450, the LandConvoy's attach (vtable +9Ch), creates one
// member per slot of its Rows x Columns map whose type is at least 1: the
// type's class instance (BSP_VehicleClass_GetOrCreate, vtable[28h](0)), posed
// at the convoy's own frame (vtable[98h](convoy+3Ch, convoy+30h, convoy+74h)),
// named "<convoy>-<slot+1>" (00742A70 on slot+1, 007438F6), with the convoy's
// party +54h and race +58h, and the back pointer +738h = convoy, +73Ch = slot.
// True: the load walk runs it for every generated LandConvoy (build_land_
// convoy_roster_00743450). False: the convoy's attach stays a record.
inline constexpr bool kLandConvoyMembersBound = true;  // ON: pairs held (docs/LAND_AND_STRUCTURES.md)
// Packet cc9_land_convoy_movement. The convoy element's three pose slots
// (00CEA528: +4h 00743060, +8h 007410C0, +0Ch 007410B0) over the Path knots
// 007AF150 derives, with 00742400 placing every member per wave-1 step through
// 007B03C0. True: each generated convoy with a roster resolves its Path, builds
// the knots and moves its members. False: the members stand at the convoy frame.
// docs/LAND_AND_STRUCTURES.md, "The convoy formation, bound".
// Packet cc9_squadron_land_task, first step. A `returntobase` (00E08F98)
// placed on a squadron's flight leader runs 007F16D0 over the squadron, with
// 006C0840 over the air-ops decks, and records the arm it takes. RECORD ONLY:
// the command is placed as before and the member's bot intake still drops it,
// because the flown `land` task 009B41C0 is not bound. True: the resolution
// line and summary. False: nothing. docs/CONTROLLED_UNIT.md.
inline constexpr bool kSquadronReturnToBaseResolveBound = true;  // ON: record only, pairs (docs/CONTROLLED_UNIT.md)
inline constexpr bool kLandConvoyMovementBound = true;  // ON: pairs held (docs/LAND_AND_STRUCTURES.md)

class GameHostLog;
class GameMissionLuaHost;
class GameObserverRuntime;
// Milestone 2n, defined in bsp/game_hosts_ship_ai.hpp. Held by pointer so this
// header stays independent of the ship AI types.
class GameShipAiHost;
// Milestone 2t, defined in bsp/game_hosts_gunnery.hpp. Owned by this host
// because the gunnery pass hangs off the unit's own tick element unit+310h.
class GameGunneryHost;

// One created scene unit as this process holds it, for the run log and the
// report. Positions are world units; the heading is degrees of
// atan2(row2.x, row2.z), which is what the probe prints.
struct GameUnitRow {
    std::string name;         // 0041dd40, the instance name the creator set
    std::string class_name;   // the scene class token, `DestroyerGen` here
    std::string type_symbol;  // the `Type = E ShipClasses : <symbol>` token
    int type_id{-1};          // the symbol resolved through the enum library
    int party{-1};
    std::string command;      // the authored `Command` token
    // Packet cc9_land_convoy_members: +738h (the convoy, by its scene name; the
    // host builds no convoy instance) and +73Ch (the slot). Empty / -1 outside
    // a convoy, as the constructor's memset leaves them.
    std::string land_convoy_738;
    int land_convoy_slot_73c{-1};
    std::string command_target;  // the authored `CommandTarget`, "" when unset
    // Milestone 2l: what the recovered command path did with that token. The
    // latched triple is the weapon director's +243h / +244h / +248h after
    // 00835c70's `cruise` arm ran 00835ac0 over the unit's live ring.
    bool command_current{false};
    bool command_latched{false};
    bool latch_is_heading{false};
    float latch_steer{0.0f};
    float latch_thrust{0.0f};
    bool class_row_found{false};  // VehicleClass[type_id] exists
    std::string class_row_name;
    float max_speed{0.0f};
    float max_rot_angle{0.0f};
    bool controlled{false};
    bool active{true};        // unit+5Ch
    float start[3]{};
    float position[3]{};
    float heading_degrees{0.0f};
    float forward_speed{0.0f};
    float throttle{0.0f};     // unit+980h, what the ring published
    float ordered_rudder{0.0f};  // unit+984h, what the ring published
    float rudder{0.0f};       // controller+80h, the slewed rudder
    float yaw_rate{0.0f};     // the angular velocity's world y
    // dot(angular velocity, pose row 1), the component about the hull's own up
    // axis, which is the one 0092e8c0 slews toward the commanded rate. It
    // equals the world y only while the hull is upright, so the trajectory dump
    // reports this one and not the world component.
    float yaw_rate_up_axis{0.0f};
    float distance{0.0f};     // straight-line, start to current
    float path_length{0.0f};  // summed per step
    unsigned long long instance_updates{0};
    unsigned long long motion_ticks{0};
    bool command_applied{false};  // the throttle gate's last answer
    // Milestone 2q: what 00822C20's property-bag arm seeded at creation, from
    // the entity's authored `StartSpeed` (docs/CRUISE_SPEED_SETTING.md).
    bool start_speed_authored{false};  // 00823597, the find returned a record
    float start_speed{0.0f};           // the authored value, m/s
    float start_speed_ratio{0.0f};     // 008235CA, the float32 ring throttle
    float start_speed_axial{0.0f};     // 008235EC, the hull's axial velocity
    // Packet cc9_submarine_depth_level (docs/SUBMARINE_MODEL.md section 11):
    // unit+1268h `depthLevel` as 00853630 seeds it on a submarine. Stage 1
    // (00853A31..00853A81) is the kamikaze test, stage 2 (00853B05..00853BAA)
    // the scene's `Dive` then `TargetDive`. LABELLED: nothing in this host
    // writes the level after the seed (008528B0's callers are not modelled),
    // stage 3 (the nearest band, non-scene units) is not modelled, and the
    // kamikaze test reads false: class+510h/+514h are not on the host's class
    // row. `submarine_depth_seeded` is false on every other class.
    bool submarine_depth_seeded{false};
    std::int32_t submarine_depth_level{0};
    // Milestone 2s: what 009329C0 staged on this unit's last substep, read back
    // off the two accumulators controller+378h and controller+384h before the
    // AddForce / AddTorque flush at 00933B01 / 00933B38.
    unsigned long long hydro_calls{0};
    int hydro_elements{0};      // (class+530h - class+52Ch) / 24h
    int hydro_submerged{0};     // elements whose depth was > 0 on the last call
    float hydro_force[3]{};     // controller+378h..+380h
    float hydro_torque[3]{};    // controller+384h..+38Ch
    // The trajectory the run measures against the bow: the angle between the
    // per-step displacement and the hull's own forward axis, and the speed
    // along that displacement. 0092D300 rewrites only the axial component, so
    // these two diverge from forward_speed exactly as far as the drag lets them.
    float drift_degrees{0.0f};
    float trajectory_speed{0.0f};
    float peak_drift_degrees{0.0f};
};

struct GameUnitsSummary {
    std::size_t units{0};
    std::size_t class_rows{0};
    std::size_t cruise_orders{0};
    // Milestone 2q: units whose creation ran the `StartSpeed` seed arm.
    std::size_t start_speed_seeds{0};
    bool controlled_bound{false};
    std::size_t controlled_index{0};
    std::string controlled_name;
    unsigned long long instance_updates{0};
    unsigned long long motion_ticks{0};
    unsigned long long motion_steps{0};   // fixed steps that ran the motion pass
    // The plane fixed step 007CE040 and which arm select_motion_arm_007ce040
    // chose. All three inputs derive from unit+900h, which 007CFD20 zeroes and
    // 007C6481 sets to 7. docs/PLANE_FLIGHT_CORE_LAW.md.
    unsigned long long plane_steps{0};
    unsigned long long plane_row_refreshes{0};  // cc9_controlled_plane_ai_moveto
    unsigned long long squadron_travel_alt_refreshes{0};  // cc9_squadron_travel_alt
    unsigned long long plane_arm_free_flight{0};
    unsigned long long dead_plane_bot_ticks_skipped{0};  // packet cc9_dead_plane_bot_think
    unsigned long long plane_arm_ground_roll{0};
    unsigned long long plane_arm_surface{0};
    unsigned long long plane_arm_none{0};
    // Packet cc9_plane_death_modes: releases refused because the aircraft was dead.
    unsigned long long dead_releases_refused{0};
    // Metres of forward travel summed over every free-flight step; zero means
    // the arm ran but the plane did not move.
    double plane_distance_moved{0.0};
    // 0085DC80s two observable outcomes. Both stay 0 while nothing rotates
    // a plane: the authored basis is already orthonormal, so the sweep is a
    // no-op. A non-zero right_reference means a row 1 came within 2.56
    // degrees of forward; a non-zero collapsed means a zero or NaN forward
    // silently zeroed a basis, which the native does not guard either.
    unsigned long long plane_pose_right_reference{0};
    unsigned long long plane_pose_collapsed{0};
    // 0085E4D0's two outcomes, and what they did to the heading. `rotations`
    // counts the steps where it wrote a pose at all; it returns false and
    // writes nothing through the 0085E871 exit when the angular velocity is too
    // short to normalise. `heading_change` sums |delta atan2(row2.x, row2.z)|
    // over every free-flight step, so it is the total turning a plane did
    // rather than the net - a plane that turns and turns back still shows it.
    // 0099ACD0's think gate firing, and 007BB920 committing a command block
    // into the live control axes. Both stay 0 while no plane is ticked.
    unsigned long long pilot_thinks{0};
    unsigned long long pilot_commits{0};
    // 0099D300's yaw arm writing a `desired` - only for a plane with a commanded
    // target, so 0 means no aircraft was ever ordered at anything.
    unsigned long long pilot_yaw_plans{0};
    unsigned long long plane_pose_rotations{0};
    double plane_heading_change{0.0};
    unsigned long long generic_tick_calls{0}; //00953CC0 with available live inputs
    unsigned long long generic_tick_unavailable{0};
    unsigned long long player_orders{0};
    float simulated_seconds{0.0f};
    float total_path_length{0.0f};
    float controlled_distance{0.0f};
    // Milestone 2s: the hydrodynamic callback 009329C0, run where 00937440 tail
    // calls it at 00937622. `element_steps` counts element iterations, not
    // calls, and `submerged_steps` the subset that produced drag.
    unsigned long long hydro_calls{0};
    unsigned long long hydro_element_steps{0};
    unsigned long long hydro_submerged_steps{0};
    unsigned long long hydro_force_flushes{0};   // 00C35360 AddForce
    unsigned long long hydro_torque_flushes{0};  // 00C35330 AddTorque
    // Milestone 2s: the world registry at [[00E188A8]+19CCh]. `registrations`
    // counts calls of the +130h virtual, `list_pushes` the 00484540 push-backs
    // those made, and `class_6_list` the length of the one list the ship AI's
    // brain pre-pass walks for neighbour candidates.
    unsigned long long world_registrations{0};
    unsigned long long world_registration_unavailable{0};
    unsigned long long world_list_pushes{0};
    std::size_t world_class_6_list{0};
};

// The units the instantiate pass of 004d4df0 created, owned for the whole run.
// Packet cc9_submarine_dive (docs/SUBMARINE_MODEL.md section 12). True: a seeded
// submarine's force callback is 00936DC0, the dive law (bsp/submarine_model.hpp,
// submarine_dive_step_00936dc0), after the hydrodynamics 009329C0 it calls itself,
// instead of the surface ship's 00937440. False: the ship force model, as before.
inline constexpr bool kSubmarineDiveBound = true;  // ON: pairs held (docs/SUBMARINE_MODEL.md section 12)

// Packet cc9_submarine_air (docs/SUBMARINE_MODEL.md section 13). True: each seeded
// submarine runs the air model 00855250 and the crush model 008551C0 once per force
// step (the tail of 00855420 in the image), needAir feeds the dive law's effective
// band, a drowned boat dies through 00926D90, the crush pulse applies 0095DA00, and
// SetUnlimitedAirSupply (00893C00) is routed. False: needAir reads false, no crush.
inline constexpr bool kSubmarineAirBound = true;  // ON: pairs held (docs/SUBMARINE_MODEL.md section 13)

// Packet cc9_submarine_seabed (docs/SUBMARINE_MODEL.md section 14). True: each
// seeded submarine runs 00855420's footprint scan over the Landscape (44h) terrain
// list, the dive law clamps its target to the published clearance (gain 1.5) for
// an AI-helmed boat, and the force callback's order-ring throttle bounds (2.0 /
// -1.0, or the plane curves under the clamp) are written. False: no scan, no
// clamp, no bounds write.
inline constexpr bool kSubmarineSeabedBound = true;  // ON: mechanism held, JM06 spread miss recorded (docs/SUBMARINE_MODEL.md section 14)

// Packet cc9_submarine_dive_teleport (docs/SUBMARINE_MODEL.md section 15). True: a
// submarine whose scene row authors `Dive` is placed at bands[Dive] at attach
// (00853B2B..00853B86: local Y +A8h, X and Z kept), before its hull body is built.
// False: the hull keeps its authored Y.
// Packet cc9_controlled_plane_ai_moveto (docs/CONTROLLED_UNIT.md, "Why a controlled plane
// reads 0.00 m"). The unit row's position is the host's copy of entity+FCh, the world
// translation row that 008A7C3C (GetPosition) reads. refresh_row runs only in the
// ship-motion loop, so a plane's row keeps its spawn position while the plane flies.
// True: after 007CE040's fixed step, a plane's row takes its motion position, and its
// moved distance follows. False: the row stays at the spawn position.
// Packet cc9_squadron_travel_alt (docs/LUA_BINDING_MISSION.md, "SquadronSetTravelAlt,
// bound"). 0089F550 writes the squadron's cruise block: +380h (countdown 1 of the
// 007F2BD0 timer block, init -1.0) = 0.5, +38Dh (its freeze byte) = force, +394h = the
// altitude, +3A9h = 1, +3ADh = 0. The cruise profile 009C3650 (task vtable +54h) returns
// at once while +38Dh is set, overwrites +394h only when +380h < 0 and +3A9h is clear, and
// otherwise clears +3A9h. True: the Lua row stores the block on the squadron's slot and the
// moveto refresh applies that gate. False: unimplemented; the refresh uses its own value.
// Packet cc9_squadron_attack_alt (docs/LUA_BINDING_MISSION.md, "SquadronSetAttackAlt,
// bound"). 008A22B0 stores the squadron's second cruise block: +37Ch = 0.5
// (countdown 0), +38Ch = force (its freeze byte), +398h = the attack altitude,
// +3AAh = 1 (the lock) and +3ADh = 0. The dive-bomb profile 009C8920 honours it
// through the same gate as 0089F550's (009C89A6..009C89DD), and the approach
// update 009C7A96 copies ctl+398h into approach+ACh. True: the block is kept and
// the dive-bomb approach reads it. False: unimplemented, the tuning value.
// Packet cc9_dive_profile_draw (docs/DIVE_BOMB_TASK.md, "The cruise profile's
// begin-altitude draw"). 009C8920, for a unit with no follow target (007B8AD0 at
// 009C8931), draws 00BD2F10(0, [00CE5380] = 15.0) with ECX = 1 at 009C899B
// before the +398h gate, and a gate that is open writes BeginAltRange/1 + draw
// into the squadron's +398h (009C89CE), which 009C7A96 copies into approach+ACh.
// True: the draw is made on every such profile call and the squadron's members
// read the drawn +398h. False: +398h stays BeginAltRange/1 with no draw.
// Packet cc9_air_ops_squadron_registry (docs/DIVE_BOMB_TASK.md, "Departed
// wingmen keep their task's squadron block"). A plane 007F3970 removed at its
// death keeps flying its bot task (the powerlost glide), and that task's
// [task+404h] is still the squadron, while its +9D8h keeps the index it had.
// True: in the dive profile, a departed plane resolves its block through the
// squadron it left and takes 007B8AD0 from its kept +9D8h. False: it falls back
// to its own slot and counts as a leader.
inline constexpr bool kDepartedWingmanTaskBlockBound = true;  // ON: pairs (docs/DIVE_BOMB_TASK.md)
inline constexpr bool kDiveProfileDrawBound = true;  // ON: pairs (docs/DIVE_BOMB_TASK.md)
inline constexpr bool kSquadronAttackAltBound = true;  // ON: pairs (docs/LUA_BINDING_MISSION.md)
inline constexpr bool kSquadronTravelAltBound = true;  // ON: pairs held (docs/LUA_BINDING_MISSION.md)

inline constexpr bool kPlaneRowPositionBound = true;  // ON: mechanism held, one premise miss (docs/CONTROLLED_UNIT.md)

inline constexpr bool kSubmarineDiveTeleportBound = true;  // ON: pairs held (docs/SUBMARINE_MODEL.md section 15)

class GameUnitsHost {
public:
    GameUnitsHost(GameHostLog& log, GameMissionLuaHost& lua);
    ~GameUnitsHost();
    GameUnitsHost(const GameUnitsHost&) = delete;
    GameUnitsHost& operator=(const GameUnitsHost&) = delete;

    // Borrow the application's actual observer manager until all units die.
    // An unbound source fixture may create units, but cannot expose endpoints.
    // Binding requires a live dispatch owner and resolved creator projections;
    // switching a bound host to another runtime is an explicit error.
    void bind_observer_runtime(GameObserverRuntime&);

    // Milestone 2j. The head of 0083b5e0 on the live Lua state: run
    // Scripts\datatables\ShipGlobals.lua and read
    // ShipGlobals["Navigator"]["TurnMultipliers"] through the reconstructed
    // loader fragment, which is the producer of the six curve fields at
    // 00424c40()+438h..+44Ch. Called once, before the units are created.
    void load_gameplay_settings_0083b5e0();

    // One record per entity the instantiate pass created, in scene order. The
    // class-descriptor floats come from the installed `VehicleClass` global that
    // the recovered global-script step 00886900 loaded.
    void create_units(const std::vector<GameSceneEntityRecord>& entities);

    // Milestone 2l: the authored `Command` token of each unit, run through the
    // recovered path 0046aab0 -> 0077d600 -> 00816e30 -> 0071ecf0 -> 00721a40
    // -> 008358d0 -> 0071e6c0 and, when the pushed command becomes current,
    // 00835c70's own arm. Nothing here writes an order ring.
    void issue_authored_commands();

    // --order <command name>: the same path, issued to the controlled unit on
    // --order-frame. Returns false when no unit is bound.
    // Milestone 2n: `unit_name` names the created instance the command goes to;
    // empty keeps the controlled unit, which is what milestones 2l and 2m used.
    bool issue_player_command(const std::string& token, const std::string& target_token,
        const std::string& unit_name = {});

    // Milestone 2m. The mission script's navigator bindings hand 0077d600 a
    // fixed command object and the descriptor 0088a810 read, so the chain starts
    // there rather than at 0046aab0's registry walk. Returns the row the command
    // path produced, or null when the index is out of range.
    const GameCommandRow* issue_script_command(std::size_t unit_index,
        std::uint32_t command_object, const bsp::SceneCommandTarget& target,
        int flags, const std::string& source, const std::string& target_name);

    // Milestone 2m. The commanded-speed store 00890e6f makes on the navigator
    // parameter block at *(unit+73Ch), and the pair as it stands.
    void store_commanded_speed_00890e6f(std::size_t unit_index, float speed);
    bsp::CruiseSpeedSetting commanded_speed(std::size_t unit_index) const;
    // Packet cc9_squadron_set_speed. A plane's vtable[3Ch], 0074E1E0, is
    // 007D9E80(unit+AB0h, speed): the controller's body linear velocity becomes
    // (0, 0, speed), its body angular velocity the zero vector at 00F87574, then
    // 007D9C80 rotates both into world. False (nothing written) for a slot that
    // is not a seeded plane.
    bool set_plane_forward_speed_007d9e80(std::size_t unit_index, float speed);
    // Packet cc9_set_submarine_depth_level. 008528B0 on a seeded submarine row:
    // the level clamped to 0..3 (1 for a kamikaze class, LABELLED false here),
    // stored at +1268h only when it differs (0085290D). True when it was stored.
    bool set_submarine_depth_level_008528b0(std::size_t unit_index, int requested);
    // Packet cc9_submarine_ai_states: unit+1200h..+120Ch, bands[band] of a seeded
    // submarine (00853A90's table), and false for any other slot or band.
    bool submarine_band_y(std::size_t unit_index, int band, float& y) const;
    // Packet cc9_submarine_air. unit+1280h, 00893C00's store. False when the slot
    // is not a seeded submarine.
    bool set_unlimited_air_00893c00(std::size_t unit_index, bool flag);
    // Packet cc9_squadron_travel_alt: 0089F550's five stores on the squadron of
    // unit_index (its registry squadron unit when it is a member). False when the
    // unit has no slot.
    // Packet cc9_squadron_attack_alt: 008A22B0's five stores on the squadron's slot.
    bool set_squadron_attack_alt_008a22b0(std::size_t unit_index, float altitude, bool force);
    bool set_squadron_travel_alt_0089f550(std::size_t unit_index, float altitude, bool force);

    // Packet cc9_difficulty. SetSkillLevel's leaf, unit->vtable[128h]: 009565A0
    // stores unit+390h and 007B8AE0 sets the pilot bot's index (bot+34h). The
    // host keeps one index per slot and applies it to every live member of the
    // slot's plane squadron, 007ECF80's fan-out. Default 1, 0095CCCC.
    void set_skill_level_007b8ae0(std::size_t unit_index, int level);
    int skill_level(std::size_t unit_index) const;

    // Packet cc9_entity_dead. The units whose damage death has happened: a
    // health <= 0 hit reaches vtable[70h] (0077D1A0 -> 00926C80, cause 1), which
    // queues the unit on the destroy list 00F899A8. The gunnery host owns the
    // death (its kill_unit funnel), so this reads its per-unit rows. Each entry
    // is (unit index, the mission clock of the death).
    std::vector<std::pair<std::size_t, float>> destroyed_units() const;

    // Milestone 2m. 00836920's stage ladder over every unit's weapon director,
    // once per fixed simulation step: the pre-pass, the `stop` arm and the idle
    // tail that re-issues a default command.
    void run_director_steps_00836920();

    // DAT_00F876A4 as this process advances it: the simulated seconds the fixed
    // step has accumulated. 00835c28 measures a commanded speed's age against it.
    float mission_clock() const noexcept;

    // The command path's own rows and counters, for the report.
    const GameCommandsHost& commands() const noexcept;
    // Packet cc8_ship_moveonpath: the `moveonpath` path build and the follow
    // mode pair both write to the director this host owns.
    GameCommandsHost& commands() noexcept;

    // 004c0890 on one created unit, through bsp::set_controlled_unit_004c0890.
    void set_controlled_unit_004c0890(std::size_t index);
    // 004C0890(null): 004C0893 stores null before its TEST ECX,ECX, so the
    // global is left empty. Packet cc9_controlled_unit_observer: the HUD
    // root observer's destruction slot 00644A20 calls it at 00644A38.
    void clear_controlled_unit_004c0890();

    // --order throttle=<f>,rudder=<f>: one player order into the controlled
    // unit's ring, through the same 00816a40 the authored command takes.
    void issue_player_order(float throttle, float rudder);

    // The entity vtable slot 0DCh the world walk calls, which for a unit is
    // 008255b0. `index` is the chain position, which is the creation order.
    void update_entity_008255b0(std::size_t index, float scaled_delta);

    // 00825f20 for every active unit, once per fixed simulation step. The
    // virtual's own caller is not established (docs/SHIP_MOTION.md names three
    // call sites and reads none), so running it here is the executable's
    // decision and is recorded as one.
    void motion_step_00825f20(float step_seconds);

    // ---- milestone 2n: what the ship AI controller reads off a unit -------
    // The AI controller runs before the motion pass and its publish fills the
    // unit's own 84-byte order slot, which the motion's head at 00825f2c then
    // promotes. Both are the same object, so the host is attached here.
    void set_ship_ai(GameShipAiHost* ai) noexcept;
    // Packet cc9_usn02_deruyter_fire: the ship AI the script orders reach.
    GameShipAiHost* ship_ai() noexcept;
    // Milestone 2t: the gun chain this host owns, or null before create_units.
    GameGunneryHost* gunnery() noexcept;
    const GameGunneryHost* gunnery() const noexcept;
    // 0071be40 on the unit's own weapon director, which 009f3dd0 reads at
    // 009f3de6 to decide which AI state the controller should be in.
    std::uint32_t director_current_command_0071be40(std::size_t index) const;
    bool director_avoidance(std::size_t index, GameDirectorAvoidance& out) const;
    bool apply_director_avoidance_message_00835640(std::size_t index,
        const bsp::DirectorCommandMessage& message);
    // Milestone 2q: the two calls an AI state step makes when it decides its
    // command is finished, 009E595C and 009E5997 for `movetopos` and 009E88C1
    // for `attackmove`. Both go to the unit's own weapon director.
    std::size_t report_command_event_00984300(std::size_t index,
        std::uint32_t command_object, const char* status);
    GameCommandCompletion end_command_0071e430(std::size_t index,
        std::uint32_t command_object, bool terminal);
    // unit+184h, the player-controlled byte 009f3df3 and 009f5e06 read. With
    // kPlayerRoleBookkeepingBound it is the image's byte: set only by a role-1
    // take the 4Bh arm of 00780120 accepts (00780214). Otherwise the stand-in:
    // the unit 004c0890 bound.
    bool unit_player_controlled_0184(std::size_t index) const;
    // SetRoleAvailable's owner->vtable[148h] = 0077F360 -> 00927D20 on one
    // unit: the permission words unit+188h + role*4 for every mask bit.
    void set_role_availability_00927d20(std::size_t index, std::uint32_t mask,
                                        std::int32_t value);
    // 0067BB50, HUD page 27h's slot 20h (the role-0 take on the controlled
    // unit), for the HUD pump to call once per pump as the image does. While
    // GameUnitsHost::Impl::kRoleScreenFixedStepCall is true the units host
    // still calls it once per fixed step itself (docs/SCRIPTED_HELM.md 6.6).
    void role_screen_update_0067bb50();
    // Canonical current assignments, native unit+1ACh..+1CCh: 00928630
    // explicitly initializes all nine to 8 (unassigned). Separate from the
    // +188h policy table. No assignment receiver is represented yet; selecting
    // a controlled unit or issuing a sender request does not change these.
    // Missing unit or role outside 0..8 returns false and preserves out.
    bool unit_current_role_slot(std::size_t index, std::int32_t role_index,
        std::int32_t& out) const;
    // The permission word unit+188h + role*4 (9 PLAYER_ANY, 8 PLAYER_AI, or a
    // slot), as SetRoleAvailable (00927D20) writes it. Missing unit or role
    // outside 0..8 returns false and preserves out. Packet cc9_gunner_role_take.
    bool unit_role_permission(std::size_t index, std::int32_t role_index,
        std::int32_t& out) const;
    // 0077C470(mask, take) on a unit from the local player: the 4Bh role
    // message 00780162 delivered at once, as the 27h take does. Screen 2Eh's
    // 00545410 sends it when a weapon group is selected or dropped.
    void role_request_0077c470(std::size_t index, std::uint32_t mask, bool take);
    // The controller's second and third gates, 009f50f2 and 009f50fc. Milestone
    // 2i holds unit+5Dh clear for a live ship; unit+61h has no writer anywhere
    // in .text outside the constructor (docs/UNIT_AUTOPILOT_PAIR.md).
    bool unit_flag_005d(std::size_t index) const;
    // Packet cc9_plane_in_flight_test: 007BB9A0's six inputs for a plane slot
    // (include/bsp/pilot_command_path.hpp). False when the slot is not a
    // plane (IsKindOf(0Fh)). docs/IN_GAME_INTERFACE_SCREEN_SETS.md.
    bool plane_local_input_gate_007bb9a0(std::size_t index,
        bsp::PilotCmdLocalInputGate& gate) const;
    // The unit's ordnance inventory, as the 007ED7E0 family aggregates it over
    // the weapon controller's slots: the union of its guns' projectile
    // descriptor answer sets. docs/ORDNANCE_KIND_IDENTITY.md. The gunnery host
    // computes it at load and stores it here because it owns the guns, and the
    // script-order host reads it because it owns 007EEC50's inputs; neither can
    // see the other. Zero for a unit with no guns, which is also the correct
    // answer for one whose guns carry nothing the family asks about.
    void store_unit_ordnance(std::size_t index, std::uint64_t mask) noexcept;
    // The commanded target the gunnery host resolved for this unit, plus one,
    // or 0 for none. Pushed rather than pulled so the two hosts agree by
    // construction on which entity a command names.
    void store_unit_command_target(std::size_t index, std::size_t target_plus_one) noexcept;
    // The command class 007EEC50 chose for this unit, stored where 0099A170
    // turns it into a bot task. A unit with no attack order keeps 0.
    // docs/DIVE_BOMB_TASK.md, "The class gate".
    void store_unit_attack_command_class(std::size_t index, unsigned int cls) noexcept;
    // Packet cc9_pilot_moveto_task: the moveto order's range, descriptor +14h
    // (008A4708), which the kind-7 task's approach reads.
    void store_unit_moveto_range(std::size_t index, float range) noexcept;
    // Packet cc9_pilot_moveto_task part 2: the order's target, the object the
    // approach reads at +44h (009C1C30 from the descriptor, 009BEBA0 each
    // refresh). `target_index` past the unit count clears it.
    void store_unit_moveto_target(std::size_t index, std::size_t target_index) noexcept;
    // Packet cc9_pilot_move_to: 009BEBA0 copies [approach+44h]'s world x/z into
    // the steer point +48h/+50h for any entity, a NavPoint included. A marker is
    // not a slot here, so its authored position is stored once and kept while
    // moveto_target_plus_one is 0.
    void store_unit_moveto_point(std::size_t index, const float world[3]) noexcept;
    // Packet cc9_pilot_moveto_task part 1b: plane vtable[88h] = 007C9540, the
    // world-matrix setter EntityTurnToEntity's squadron arm calls per member
    // (008A0DD4). Rows 0..2 replace the pose; the position is the member's own
    // (008A0D9B-008A0DBF), so it is left alone. 007C9540 touches no velocity.
    // False for an index out of range.
    bool set_unit_world_basis_007c9540(std::size_t index, const float right[3],
        const float up[3], const float forward[3]) noexcept;
    std::uint64_t unit_ordnance(std::size_t index) const noexcept;
    bool unit_flag_0061(std::size_t index) const;
    // 0092d730 over the unit's body axis and linear velocity, the same value the
    // trajectory dump's fwd_speed column carries.
    float unit_forward_speed_0092d730(std::size_t index) const;
    // Packet cc9_aa_lethality_audit: the body linear velocity, which for a
    // plane is its world velocity (unit+AC8h, what vtable[34h] 007BBB70 copies;
    // the free-flight step mirrors plane_world_velocity into it).
    bool unit_linear_velocity(std::size_t index, float out[3]) const;
    // Packet cc9_aa_turn_average: the flight controller's BODY angular rate,
    // ctl+48h..+50h = unit+AF8h..+B00h (ctl = unit+AB0h), which 00901C20 reads at
    // 00901CE5..00901CF9 for a plane target. The same three floats the rate law
    // 007DA710 integrates. Zero for a unit that is not a flying plane. False only
    // for an unknown index.
    bool unit_plane_body_angular_rate(std::size_t index, float out[3]) const;
    // [unit+538h]+508h, `Retardation` out of the installed VehicleClass row,
    // which is the divisor 009ed8ec uses to build the stopping distance.
    float unit_retardation_0508(std::size_t index) const;
    // The unit's vtable[50h] heading, in radians, as atan2(row2.x, row2.z).
    float unit_heading_radians(std::size_t index) const;
    // 00811940 on the unit's live rudder state, the yaw rate 009ed9c1 folds into
    // the heading target.
    float unit_current_yaw_rate_00811940(std::size_t index);
    // 009e1170's AI arm for one unit, with the three desired-value setters
    // 009dbf90 / 009dffb0 / 009e0040 writing the control block the caller owns.
    // False when the unit holds no current `cruise`, or when it is the player's.
    bool run_cruise_state_step_009e1170(std::size_t index, bsp::ShipAiControlBlock& blk,
        bsp::ShipAiSetterHost& setters);
    // The live request overload. Slot inputs must come from their actual
    // owners; this host supplies unit+184h itself. Historical request member
    // flag_3fc is nav+3F4h; enable_3f4 is nav+3ECh, and side_filter is nav+3F0h.
    bool run_cruise_state_step_009e1170(std::size_t index, bsp::ShipAiControlBlock& blk,
        bsp::ShipAiSetterHost& setters, bsp::ShipAiAvoidanceRequest& request,
        const bsp::ShipAiCruiseAvoidanceInputs& avoidance_inputs);

    // ---- milestone 2o: the hop from the AI's desired pair into the ring ----
    // 009f3ff8..009f402e, the head of 009f3f80: the slot under the ring's write
    // cursor at unit+97ch, read before the body rewrites the two desired values
    // and slews them toward it. The two loads are independent
    // (009f400d reads +83ch, 009f4025 reads +838h).
    float unit_ring_write_slot_throttle(std::size_t index) const;
    float unit_ring_write_slot_rudder(std::size_t index) const;
    // 0080e170 at 009f4cfb and 0080e190 at 009f4ce8: the two five-instruction
    // setters that put the slewed pair back into that same slot. They touch no
    // cursor, no bound and neither live field.
    void unit_ring_set_write_slot_throttle_0080e170(std::size_t index, float value);
    void unit_ring_set_write_slot_rudder_0080e190(std::size_t index, float value);
    // ring+148h / +14ch, the live pair 00813020 steps toward the read slot and
    // 00825f20 reads. Only the run's own counting uses these.
    float unit_ring_current_throttle(std::size_t index) const;
    float unit_ring_current_rudder(std::size_t index) const;
    // The divisor 009da268 loads from [[blk+3fch]+538h]+524h. Packet
    // ship_ai_class_field_0524 found its producer: 00828f20 derives it as
    // 0.5 * MaxRotAngle (class+4f8h) / MaxRotAngleChangeRatio (class+4fch), and
    // both keys are already read out of the installed `VehicleClass` row here.
    // `derived` is 00828f20's own gate: false means one of the two keys was not
    // strictly positive and the field was never written.
    float unit_yaw_authority_0524(std::size_t index, bool& derived) const;
    // --ai-drive <name>=<throttle>,<rudder>: the diagnostic stand-in for the
    // state steps that produce no desired throttle. Returns false when no
    // created instance carries the name.
    bool enable_ai_drive(const std::string& unit_name, float throttle, float rudder);

    // ---- milestone 2o, second pass: what a state step reads off a unit -----
    // unit+0C8h, the pose-valid byte 009e14d7 and 009e579a test before they ask
    // for a refresh through 00414db0. Milestone 2h leaves it set.
    bool unit_pose_valid_00c8(std::size_t index) const;
    // unit+0FCh, the world translation of the pose. 009e14ec passes &unit+0FCh
    // to the world-bounds test, so the y at +100h is part of it; 009e57aa and
    // 009e57c2 read the x and z alone.
    void unit_position_00fc(std::size_t index, float& x, float& y, float& z) const;

    // [00E188A8] +711ch / +7124h / +7128h / +7130h, the box 0071c4f0 tests a
    // position against. False when this process has no world object, which it
    // does not: construct_world 004de610 is a load record.
    bool world_bounds_box_00e188a8(float& min_x, float& max_x, float& min_z,
        float& max_z) const;

    // ---- milestone 2p: what the brain pre-pass 009f1420 reads --------------
    // 0071eb60 on [brain+0ab8h], the unit's own weapon director. False is the
    // empty singleton at 00e19b98; `mode` is director+30h.
    bool active_command_descriptor_0071eb60(std::size_t index,
        bsp::SceneCommandTarget& out, int& mode) const;
    // 00521ea0 BSP_CommandTarget_ResolveObject on that descriptor. Returns a
    // one-based created-instance handle, or 0 when the descriptor names no
    // object or the object is not one of this process's instances.
    std::uint32_t resolve_command_target_00521ea0(const bsp::SceneCommandTarget& target) const;
    // Packet cc9_prcp03_phase_progress: a scene marker's authored world
    // position by its entity id (the NavPoints the handle tables hold and this
    // host carries as markers). False when the id names no marker.
    void register_scene_marker_position(int id, const float world[3]);
    bool scene_marker_position(std::uint32_t id, float world[3]) const;
    // Packet cc9_spawn_new_shipyard. A marker's whole authored world matrix
    // (entity+CCh, rows 0..2 the basis and row 3 the translation), which
    // 008F8680 hands SpawnNew when `refPos` is that marker.
    void register_scene_marker_frame(int id, const float world[16]);
    bool scene_marker_frame(std::uint32_t id, float world[16]) const;
    // 004142e0 BSP_Vector3f_TransformAffinePoint with the matrix at unit+0cch,
    // which is what 009dbcc0 carries the latched offset out through.
    void transform_by_unit_matrix_004142e0(std::size_t index, float in_x, float in_y,
        float in_z, float& out_x, float& out_y, float& out_z) const;
    // unit+54h, the side word 009f14db / 009f14e4 compare and 009e2588 copies.
    int unit_side_0054(std::size_t index) const;
    // unit+BC9h, the latched gunFire (packet cc9_plane_gun_pass). The gunnery
    // host's plane-gun hook reads it; docs/PLANE_GUN_PASS.md.
    bool plane_gun_trigger_bc9(std::size_t index) const;
    // 0071df70's two inputs on the unit's own director, forwarded.
    float director_target_hold_0040(std::size_t index) const;
    int director_leading_slot_categories_0071df83(std::size_t index, int* out,
        int max_out) const;
    std::uint32_t director_slot_command(std::size_t index, int slot_index) const;
    const char* command_name_of(std::uint32_t command_object) const;
    // The seven AutoThrust keys 009ec7c0 consumes, loaded once beside the turn
    // multipliers. `loaded` is false when the sub-table was absent, in which
    // case every field is the zero a fresh settings object carries.
    const bsp::ShipAiAutoThrustSettings& auto_thrust_settings(bool& loaded) const;
    // 009ec97b / 009ec9a4 / 009ec99c / 009ec9ab, the four live reads the
    // throttle ceiling makes outside the tuning block.
    bsp::ShipAiThrottleCeilingInputs throttle_ceiling_inputs(std::size_t index) const;
    // unit+9cch, the FULL width the danger ramp divides the clearance by, and
    // [[unit+538h]+500h] / +508h, the two class fields 009ef230 builds a
    // sector's braking distance from.
    float unit_half_width_09cc(std::size_t index) const;
    float unit_class_max_speed_0500(std::size_t index) const;
    // Packet cc9_units_capture_accessors (for 00A03760 BSP_AiCapture_UnitArrivalValue).
    // unit+7A0h, the CommandBuilding's CaptureRange: 006F2780 stores the scene
    // `CaptureRange` dword, or 500 when unauthored (006F27E5); 00A03760 reads it as
    // `(float)(int)target[+7A0h]` (00A037CF FILD). Answers 500 for a unit that is not a
    // CommandBuilding (kind 1Ch) or has no slot.
    float command_building_capture_range_07a0(std::size_t unit_index) const;
    // The plane class MaxSpd, class+188h (007D238A reads the Lua key). 00A03760
    // reads it for a PlaneSquadron (IsType 18h) as [unit+35Ch]+188h (00A03819 /
    // 00A0381F), the
    // squadron's plane class. A plane (kind 0Fh) answers its own class value, a
    // squadron its first member's with a nonzero value; anything else 0.
    float plane_class_max_speed_0188(std::size_t unit_index) const;
    // ---- milestone 2r: what 009e4330 reads to build the navigation block ----
    // [unit+538h]+4f8h, the Lua key `MaxRotAngle` 009e4574 loads for the yaw
    // floor at 009e45a9, and [unit+538h]+520h, `MaxSpeed / MaxRotAngle`, which
    // 0082e960 multiplies the rudder curve by at 0082e970.
    float unit_class_max_rot_angle_04f8(std::size_t index) const;
    // [unit+538h]+504h, the Lua key `MaxAccel` 009e054f loads for the contact
    // horizon.
    float unit_class_max_accel_0504(std::size_t index) const;
    float unit_class_turn_radius_0520(std::size_t index) const;
    // 0082e960 itself, `__thiscall(descriptor)(float throttle)`: the rudder
    // curve 0082e890 over the settings singleton this host already owns, times
    // class+520h. 009e44c4 asks for 1.0f and 009e4555 for 0.9f.
    float unit_class_turn_circle_radius_0082e960(std::size_t index, float throttle);
    // unit+9c8h, the full hull length 0081106e / 0081fa4d produce. This process
    // builds no model box at [class+50h], so the producers' fallback applies and
    // the field is the descriptor's own +a0h `Length`.
    float unit_hull_length_09c8(std::size_t index) const;
    // class+b0h `Mass` and the physics record 00937cf1 selects for this hull,
    // both settled when the body was built.
    float unit_hull_mass_00b0(std::size_t index) const;
    int unit_hull_material(std::size_t index) const;
    // M+b8h / M+bch and M+18h, read back off the body 00937c90's tail built.
    float unit_hull_linear_damping(std::size_t index) const;
    float unit_hull_angular_damping(std::size_t index) const;
    // unit+102ch and unit+1034h, the two load latches 009f3f80's middle raises
    // with the inlined bodies of 009d4fb0 and 009d4fe0.
    void raise_turn_assist_load_102c(std::size_t index, float value);
    void raise_secondary_load_1034(std::size_t index, float value);
    float turn_assist_load_102c(std::size_t index) const;
    float secondary_load_1034(std::size_t index) const;

    // ---- milestone 2s: the world's per-class unit lists -------------------
    // [[00E188A8]+19CCh] holds 97 {count, head, tail} triples at
    // registry+18h + id*0Ch (004CB076's vector-constructor iterator inside
    // 004CB030). A created unit joins them through its entity virtual slot
    // +130h, selected through its actual descriptor creator. Every recovered
    // registrar starts with00928560 (id1), then joins its own ordered lists.
    // Ships join6, planes15; buildings use their own class lists. See
    // docs/UNIT_WORLD_REGISTRATION.md and UNIT_WORLD_REGISTRATION_LIVE.md.
    //
    // Id 6 is the list BSP_ShipAi_BrainPrePass reads at 009F1877: it loads
    // [00E188A8], then +19CCh, then the head at +64h and the count at +60h,
    // and 0x60 == 0x18 + 6*0xC. This host builds and fills those lists; the
    // consumer that walks id 6 into the neighbour list at blk+608h lives in
    // GameShipAiHost and is not wired here.
    std::size_t world_list_size(int class_id) const noexcept;
    // Packet cc9_units_contracts, docs/AVOID_ZONE_REGISTRY.md. The unit's part
    // table size, (unit+34Ch - unit+348h) >> 2: its class's Damage.Sections
    // count (0087BCC0 sizes the table to the descriptor's +18h vector).
    std::size_t unit_part_descriptor_count(std::size_t index) const;
    // List 24 (007F10B0, +138h) holds one node per plane squadron: the member
    // slot the host fuses with the squadron. This answers [squadron+3D0h], the
    // squadron's current flight leader, for such a node; the unit count when
    // it is not a squadron node or no member is alive.
    std::size_t squadron_list_24_leader(std::size_t entry_unit) const;
    // The unit index at `position` of the class-`class_id` list, or the unit
    // count when the position is past the end. The order is 00484540's own:
    // appended at the tail, walked from the head.
    std::size_t world_list_entry(int class_id, std::size_t position) const noexcept;
    // Stable nodes from the actual runtime registry. Payload is the direct
    // unit identity; next is read live, including after candidate callbacks.
    const void* world_list_head(int class_id) const noexcept;
    static const void* world_list_node_unit(const void* node) noexcept;
    static const void* world_list_node_next(const void* node) noexcept;

    std::size_t count() const noexcept;
    bool unit_active(std::size_t index) const noexcept;
    // Stable identity shared with world_list_node_unit; not a native address,
    // scene ID, Lua handle, or index+1 token. Valid for this host's lifetime.
    const void* unit_identity(std::size_t index) const noexcept;
    // Borrowed aliases into that same stable slot, never a native whole-unit
    // cast. Only known creator prefixes with a live bound runtime are exposed.
    // Tables identify native profiles; they are not executable process vtables.
    // The alias expires when its unit begins teardown; callers must finish all
    // endpoint operations before the frame/units owner is destroyed or replaced.
    std::optional<bsp::NativeUnitObserverAlias> observer_alias(
        const void* canonical_identity) noexcept;
    // Borrow the same unit's actual +5C..+60 byte cells. This additionally
    // requires the live observer alias above and resolved flag provenance.
    // The view expires before unit teardown, just like observer_alias; finish
    // operations before destruction/replacement. No callback providers bind here.
    std::optional<bsp::NativeSceneLifecycleView> scene_lifecycle_view(
        const void* canonical_identity) noexcept;
    const void* unit_identity_from_observer(
        const bsp::NativeObserverOwnerStorage* endpoint) const noexcept;
    // One canonical owner: UnitInstanceState holds+5C/+5D; the same slot holds
    // missing+5E/+5F/+60. SceneNodeFlags is a snapshot, never a second owner.
    // Only resolved creator slots have native constructor/init provenance.
    // Invalid/foreign/unresolved identities return false and preserve outputs.
    bool unit_scene_node_flags(std::size_t index, bsp::SceneNodeFlags& out) const noexcept;
    bool read_scene_node_flags(const void* identity, bsp::SceneNodeFlags& out) const noexcept;
    // Stores accept state produced by an actual caller/compiled helper; they
    // do not perform kill/remove callbacks, notify observers or enqueue.
    bool store_scene_node_flags(const void* identity, const bsp::SceneNodeFlags& flags) noexcept;
    bool unit_pending_destroy_0060(std::size_t index, bool& out) const noexcept;
    bool store_pending_destroy_0060(const void* identity, bool pending) noexcept;
    // Packet cc9_ship_sink_descent: the ship wreck handler 00824B60's sink block
    // 00824FE5..00825086 (the leak redistribution 0074EC50, inertia x2 and the
    // two dampings), run by the row-15 flush at vt[7Ch]. False when the switch is
    // off, the identity is not a unit, or its leak model was never built.
    bool ship_wreck_sink_00824fe5(const void* identity);
    // Packet cc9_dead_member_group_removal: 0077BD70(unit, null) on a destroyed
    // unit, once. No effect for a unit in no group.
    void leave_group_on_destroy_0077bd70(std::size_t index);
    // Packet cc9_live_hull_leak (docs/UNIT_MESSAGE_ARMS.md, "90h, bound"): the
    // 90h message's receiver 008221A7 -> 0074F440 -> 0074F090 on the unit's
    // leak manager. `count` is msg+1Ch, the sender's trunc(clamp(damage / 10,
    // 0, 63)); `world_point` is the hit's world point (hit+08h). CONTRACT for
    // the gunnery host: call it from ShipHitBinding::route_hull_impact_effect
    // (R10, 0082755F) and route_part_impact_effect (R11c) with the victim's
    // unit index. False when the switch is off, there is no leak model, or
    // 0074F090's live-unit gate refuses it.
    bool add_leak_0074f440(std::size_t index, std::uint32_t count,
        const float world_point[3]);
    // Packet cc9_live_hull_repair. The two unit inputs the leak manager reads that
    // live in the gunnery host: the class `Repair` byte (class+D0h, 00962E16;
    // 1 unless the class authors Repair = false) that 0074F090's live gate tests,
    // and the health fraction 00923BE0 (clamped current / maximum, 0 once
    // torn down) that 0074F930's live cap reads. The gunnery host sets both every
    // step; the units host uses them only under kLiveHullRepairBound.
    void set_unit_leak_inputs(std::size_t index, float health_fraction, bool class_repair);
    // Packet cc9_squadron_pass_hooks (docs/CONSTRUCT_WORLD.md section 27). Two
    // entries the Lua host's InitAll calls for a plane-squadron node, by unit
    // index of the squadron (its fused leader). Both are no-ops for now and
    // only count their calls:
    // - pass A (007F4580): construct the squadron's wing there, as the image
    //   does (docs/SENTITY_INIT_ATTACH_ORDER.md section 9.3);
    // - pass C (007F4BA0 at 007F4E9E): with no current command, a stop
    //   (00E08F88) at the first member's position when its +900h is 6, else a
    //   moveto (00E08F68) toward the home base +404h when one is set.
    void on_squadron_pass_a_construct_wing(std::size_t squadron_index);
    void on_squadron_pass_c_initial_command(std::size_t squadron_index);
    // Packet cc9_wing_construction: the wing records a creator batch holds back
    // for the squadron whose fused leader is `leader_index`, built by the pass A
    // hook above. Wing i+1 of the registry record's member_units is written
    // when it is built.
    void stage_squadron_wing(std::size_t leader_index,
                             std::vector<GameSceneEntityRecord> wing);
    // Packet cc9_scene_home_base_key (docs/CONSTRUCT_WORLD.md section 29).
    // CONTRACT for the owner that builds a plane squadron from a scene row
    // (PlaneSquadronGen, class 18h): call this once, before the squadron's
    // InitAll pass C, with the row's `HomeBase` property as authored (the
    // RFort's name string, "" when empty or absent). Pass C (007F4BA0) reads the
    // bag key `HomeBase` (00CF8820) at 007F4C43, resolves a non-empty name
    // through 00925A90 and hands it to 007F1C00. An air-ops launch does not need
    // this: its bag's `HomeBase` is the deck owner, which the host finds from
    // the deck slot that launched the squadron.
    void set_squadron_scene_home_base(std::size_t squadron_index, const std::string& home_base);
    // Packet cc9_land_convoy_members, under kLandConvoyMembersBound: 00743450 for
    // one generated LandConvoy record (its roster keys lifted by the scene pass).
    // Appends the members through create_units and answers how many it made.
    std::size_t build_land_convoy_roster_00743450(const GameSceneEntityRecord& convoy);
    // +738h: the convoy a unit belongs to ("" for none), and +73Ch its slot.
    const std::string& unit_land_convoy_738(std::size_t index) const;
    int unit_land_convoy_slot_73c(std::size_t index) const;
    // For cc9-ships5's 00A11690 wedge (docs/AI_CAUTIOUS_ROUTE.md section 10).
    // 0070D080 BSP_UnitGroup_FindMemberRecord (__thiscall(group)(entity), RET 4,
    // body 0070D080-0070D0B5) resolves the member's 34h record in group+18h under
    // the count +4F8h; the caller then stores record+10h+4*column (lateral) and
    // record+20h+4*column (axial). The wedge's stores are 00A11A57 (record+10h)
    // and 00A11A5C (record+20h), both column 0, values -0.0 - off (00D7A208).
    // False, and nothing written, when the leader has no group, the member is
    // not in it, or the column is outside 0..3. No caller yet: inert.
    bool set_formation_member_offset_0070d080(std::size_t leader, std::size_t member,
                                              int column, float lateral, float axial);
    // Packet cc9_land_convoy_movement, under kLandConvoyMovementBound. After the
    // roster: resolve the convoy's "Path" (007420B0), derive its knots
    // (007AF150), take the placement law (00742C70) and keep the convoy's arc
    // state. False when the convoy has no members or its Path does not resolve.
    bool bind_land_convoy_motion(const GameSceneEntityRecord& convoy,
                                 const std::vector<GameSceneEntityRecord>& scene);
    // One fixed step of the convoy element, in the waves' order: wave 1 runs
    // +4h 00743060(step) (which calls 00742400) then +0Ch 007410B0; wave 3 runs
    // +8h 007410C0(step).
    void land_convoy_step_waves(float step);
    // The interpolation wave 00875160: +4h 00743060(leftover), not committed.
    void land_convoy_interpolation_wave(float leftover);
    // Instance vtable+5Ch dispatch using the class selected by VehicleClass.Type
    // and the compiled predicates in unit_kind_query.hpp. Missing/unrecognized
    // identity and invalid indices answer false. docs/GAME_UNIT_KIND_BINDING.md.
    bool unit_is_kind_of(std::size_t index, int class_id) const;
    // Native instance+C4h class id, or -1 for an unresolved identity/invalid index.
    int unit_class_id(std::size_t index) const noexcept;

    // ---- packet cc8_ship_follow: the unit group at entity+284h -------------
    // The 508h-byte object 0070DB20 creates and 0070EF30 joins, whose leader is
    // group+14h, member records start at group+18h, count at group+4F8h and
    // pattern index at group+500h. docs/SHIP_UNIT_GROUP_FOLLOW.md,
    // docs/SHIP_AI_FORMATION.md. A unit not in a group answers -1.
    std::int32_t unit_formation_group_0284(std::size_t index) const noexcept;
    // group+14h. Answers the unit count of the index, or SIZE_MAX when there is
    // no such group. 007788D0 BSP_Unit_FormationLeader is this on a unit.
    std::size_t formation_leader_0014(std::int32_t group) const noexcept;
    // 007788B0 BSP_Unit_IsFormationFollower: in a group and not its leader.
    bool unit_is_formation_follower_007788b0(std::size_t index) const noexcept;
    // Packet cc9_bsm01_think_natives. 008193A0, the unit vtable slot +118h
    // (SetWorldPosition, docs/SHIP_ESCORT_SCREEN.md section 1), for the arm a
    // unit that is neither a formation follower nor its group's leader takes:
    // the position store 009583C0 and the wake-ring refill 00818EA0 from the
    // new position along the unit's current heading. Answers false and places
    // nothing for a follower (0081942F refuses |pos|^2 > 25.0; the station
    // override 0081945D is not modelled) and does place a leader, whose group
    // snap 0081963E is not modelled. LABELLED: 0092D620's controller sync is
    // the motion state itself here, and vtable[0D8h] (00955970, the scene-node
    // matrix refresh) and the 1.25 at unit+0BCCh have no host counterpart.
    bool place_at_world_position_008193a0(std::size_t index, const float pos[3]);
    // Packet cc9_bsm01_state_natives. DisablePhysics 00891380: the force
    // controller's +14h byte, which 009329C9 tests every step; false for a unit
    // with no controller. The entity's local 4x4 (+74h) and its slot 88h setter
    // 006E00A0, which the matrix interpolator pass writes through.
    bool disable_physics_00891380(std::size_t index);
    bool local_matrix_0074(std::size_t index, float out[16]) const;
    bool set_local_matrix_006e00a0(std::size_t index, const float m[16]);
    // group+4F8h.
    std::int32_t formation_member_count(std::int32_t group) const noexcept;
    // Packet cc9_ship_formation_speed: the unit record slot names,
    // [group+18h + slot*34h], as a unit index; SIZE_MAX for an empty record or
    // an out-of-range slot. The walks 0070D140, 0070DA00 and 0070E3C0 read it.
    std::size_t formation_member_unit(std::int32_t group, std::int32_t slot) const noexcept;
    // 0077F940 BSP_UnitGroup_JoinOrMerge reduced to the arm a runtime join takes:
    // create the group around the leader when it has none (0070DB20), then append
    // the follower (0070EF30). Answers true when the membership changed.
    bool formation_join_0077f940(std::size_t follower, std::size_t leader);

    // What 0070D290 answers: the formation slot point and its direction. It
    // lives here because it needs both the member record (group+18h) and the
    // LEADER's wake ring (leader+0BD0h), and the units host owns both.
    struct FormationStation {
        float x{0.0f};
        float z{0.0f};
        float dir_x{0.0f};
        float dir_z{0.0f};
        float across{0.0f};       // out[5], record+10h * across_scale
        float along{0.0f};        // out[6], record+20h * along_scale
        float wake_yaw_rate{0.0f};
        bool wake_yaw_written{false};  // 00810630's along<=0 arm never writes it
        bool is_leader_branch{false};  // 0070D362, the unit is its own leader
        bool valid{false};
    };
    // 0070D290, __thiscall(group)(unit, out[7], across_scale, along_scale),
    // RET 10h. A unit that is its own leader, or has no record, takes the
    // 0070D362 branch and answers its own pose.
    FormationStation formation_station_0070d290(std::size_t unit, float across_scale,
                                                float along_scale) const noexcept;

    // ---- milestone 2k: what the two HUD world screens read off a unit ------
    // 0043f080 BSP_UnitInstance_IsAliveAndVisible, the four-byte filter both the
    // minimap walk (005c1628..005c164a) and the marker gate (006431a8) run.
    // Reads all four persistent cells; unavailable native state fails the gate.
    bool unit_alive_and_visible(std::size_t index) const;
    // The world matrix rows 0063a6c0 and 00427eb0 read: +CCh right, +DCh up,
    // +ECh forward and +FCh translation. False when the index is out of range.
    bool unit_pose(std::size_t index, float right[3], float up[3], float forward[3],
        float translation[3]) const;
    // [unit+538h] +A0h, +A4h and +A8h, the three class extents 0063a6c0 builds
    // its eight corners from. Only +A0h and +A8h have a recovered Lua key
    // (bsp::ShipMotionClass), and both are zero on this installation's ships;
    // +A4h has no recovered producer at all and is reported as zero.
    void unit_class_extents(std::size_t index, float& forward, float& right,
        float& up) const;
    // One row without rebuilding the flat table, for a per-frame reader.
    // active is refreshed from the canonical byte on each request. Request
    // again after lifecycle writes; neither row API lends an authoritative flag.
    const GameUnitRow* unit_row(std::size_t index) const noexcept;
    bool controlled_bound() const noexcept;
    std::size_t controlled_index() const noexcept;
    // Packet cc9_helm_orders_helm_route (docs/SCRIPTED_HELM.md section 9):
    // --helm-orders `takehelm`. The controlled unit takes role 1 through the
    // BSP_PLAYER_HELM transfer (0064B9A6 -> 0077C470(unit, 2, 1), +184h set),
    // holds `throttle` and steers toward (x, z) by 009DA250's rudder law.
    // False, with a log line, when `index` is not the controlled unit.
    bool helm_route_take(std::size_t index, float throttle, float x, float z);

    const std::vector<GameUnitRow>& units() const noexcept;
    const GameUnitsSummary& summary() const noexcept;

    // One line per sampled unit, for --order's trajectory log.
    void log_controlled_trajectory(unsigned long long mission_frame);
    // The end-of-run distance table.
    void report();

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
