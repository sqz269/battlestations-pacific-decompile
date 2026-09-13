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
#include <string>
#include <vector>

#include "bsp/game_hosts_commands.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/ship_ai_obstacle_tables.hpp"

namespace bsp::game {

class GameHostLog;
class GameMissionLuaHost;
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
    unsigned long long world_list_pushes{0};
    std::size_t world_class_6_list{0};
};

// The units the instantiate pass of 004d4df0 created, owned for the whole run.
class GameUnitsHost {
public:
    GameUnitsHost(GameHostLog& log, GameMissionLuaHost& lua);
    ~GameUnitsHost();
    GameUnitsHost(const GameUnitsHost&) = delete;
    GameUnitsHost& operator=(const GameUnitsHost&) = delete;

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

    // Milestone 2m. 00836920's stage ladder over every unit's weapon director,
    // once per fixed simulation step: the pre-pass, the `stop` arm and the idle
    // tail that re-issues a default command.
    void run_director_steps_00836920();

    // DAT_00F876A4 as this process advances it: the simulated seconds the fixed
    // step has accumulated. 00835c28 measures a commanded speed's age against it.
    float mission_clock() const noexcept;

    // The command path's own rows and counters, for the report.
    const GameCommandsHost& commands() const noexcept;

    // 004c0890 on one created unit, through bsp::set_controlled_unit_004c0890.
    void set_controlled_unit_004c0890(std::size_t index);

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
    // unit+184h, the player-controlled byte 009f3df3 and 009f5e06 read. In this
    // process the byte is the unit 004c0890 bound.
    bool unit_player_controlled_0184(std::size_t index) const;
    // Canonical current assignments, native unit+1ACh..+1CCh: 00928630
    // explicitly initializes all nine to 8 (unassigned). Separate from the
    // +188h policy table. No assignment receiver is represented yet; selecting
    // a controlled unit or issuing a sender request does not change these.
    // Missing unit or role outside 0..8 returns false and preserves out.
    bool unit_current_role_slot(std::size_t index, std::int32_t role_index,
        std::int32_t& out) const;
    // The controller's second and third gates, 009f50f2 and 009f50fc. Milestone
    // 2i holds unit+5Dh clear for a live ship; unit+61h has no writer anywhere
    // in .text outside the constructor (docs/UNIT_AUTOPILOT_PAIR.md).
    bool unit_flag_005d(std::size_t index) const;
    bool unit_flag_0061(std::size_t index) const;
    // 0092d730 over the unit's body axis and linear velocity, the same value the
    // trajectory dump's fwd_speed column carries.
    float unit_forward_speed_0092d730(std::size_t index) const;
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
    // 004142e0 BSP_Vector3f_TransformAffinePoint with the matrix at unit+0cch,
    // which is what 009dbcc0 carries the latched offset out through.
    void transform_by_unit_matrix_004142e0(std::size_t index, float in_x, float in_y,
        float in_z, float& out_x, float& out_y, float& out_z) const;
    // unit+54h, the side word 009f14db / 009f14e4 compare and 009e2588 copies.
    int unit_side_0054(std::size_t index) const;
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
    // +130h: 006FE620 BSP_UnitInstance_RegisterInWorldLists, whose body is
    // 00928560 (which pushes id 1) followed by five 00484540 push-backs onto
    // ids 2, 4, 5, 6 and 7 (ADD ECX,0x30/0x48/0x54/0x60/0x6C at
    // 006FE62C..006FE65C). docs/GAMEPLAY_LOOSE_ENDS_1.md section 2.
    //
    // Id 6 is the list BSP_ShipAi_BrainPrePass reads at 009F1877: it loads
    // [00E188A8], then +19CCh, then the head at +64h and the count at +60h,
    // and 0x60 == 0x18 + 6*0xC. This host builds and fills those lists; the
    // consumer that walks id 6 into the neighbour list at blk+608h lives in
    // GameShipAiHost and is not wired here.
    std::size_t world_list_size(int class_id) const noexcept;
    // The unit index at `position` of the class-`class_id` list, or the unit
    // count when the position is past the end. The order is 00484540's own:
    // appended at the tail, walked from the head.
    std::size_t world_list_entry(int class_id, std::size_t position) const noexcept;

    std::size_t count() const noexcept;
    bool unit_active(std::size_t index) const noexcept;
    // Instance vtable+5Ch dispatch using the class selected by VehicleClass.Type
    // and the compiled predicates in unit_kind_query.hpp. Missing/unrecognized
    // identity and invalid indices answer false. docs/GAME_UNIT_KIND_BINDING.md.
    bool unit_is_kind_of(std::size_t index, int class_id) const;
    // Native instance+C4h class id, or -1 for an unresolved identity/invalid index.
    int unit_class_id(std::size_t index) const noexcept;

    // ---- milestone 2k: what the two HUD world screens read off a unit ------
    // 0043f080 BSP_UnitInstance_IsAliveAndVisible, the four-byte filter both the
    // minimap walk (005c1628..005c164a) and the marker gate (006431a8) run.
    // bsp/unit_instance.hpp carries +5Ch and +5Dh; +5Eh and +60h have no field
    // there, so they are treated as the clear a created instance leaves them at.
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
    const GameUnitRow* unit_row(std::size_t index) const noexcept;
    bool controlled_bound() const noexcept;
    std::size_t controlled_index() const noexcept;

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
