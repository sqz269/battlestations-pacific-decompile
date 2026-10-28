// bsp_game.exe milestone 2f: the load past the renderer-owner hosts, the
// mission state entry and the headless in-mission frame.
//
// Two things in this file are the executable's own decisions and are marked as
// such where they are made:
//
// 1. The load is driven by walking bsp::mission_load_host_steps() in its own
//    order rather than by entering bsp::run_mission_scene_load. That driver's
//    first world step hands the caller a bsp::GlobalSubsystemInvocation, which
//    is a bundle of references to twenty configuration contexts that nothing in
//    the repository builds yet, so the driver cannot be entered from here. The
//    order the walk follows is still the reconstruction's recovered table; what
//    the walk does not carry are the driver's own session-mode arms, and those
//    steps are reported as skipped arms rather than as performed.
// 2. The frame runs with no world. Every world container the reconstructions
//    walk is empty, so a routine such as 00481640 or 006dc1a0 runs to
//    completion over nothing. That is a real run of the recovered routine and
//    it is not a claim that the game's world was ticked.
//
// See include/bsp/game_hosts_mission_frame.hpp for the address list.

#include "bsp/game_hosts_mission_frame.hpp"
#include <stdexcept>

#include "bsp/game_hosts.hpp"
#include "bsp/game_hint_passes.hpp"
#include "bsp/plane_follow_law.hpp"
#include "bsp/sw1b_switch_reach.hpp"
#include "bsp/game_profile_owner.hpp"
#include "bsp/cc13_hitq_switches.hpp"   // packet cc13-hitq2
#include "bsp/game_observer_runtime.hpp"
#include "bsp/game_hosts_fixed_step.hpp"
#include "bsp/game_hosts_hud.hpp"
#include "bsp/game_hosts_hud_world.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_menu.hpp"
#include "bsp/game_hosts_message_manager.hpp"
#include "bsp/game_hosts_global_subsystems.hpp"
#include "bsp/warning_owner.hpp"
#include "bsp/game_hosts_ready.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_hosts_mission_result.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/game_hosts_ship_ai.hpp"
#include "bsp/game_hosts_trajectory.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/mission_events.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_hosts_world.hpp"
#include "bsp/award_trackers.hpp"
#include "bsp/dive_bomb_task.hpp"
#include "bsp/game_dynamics_list.hpp"
#include "bsp/game_frame_control.hpp"
#include "bsp/in_mission_subsystem_tick.hpp"
#include "bsp/game_settings.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/lua_binding_mission_2.hpp"
#include "bsp/mission_load_path.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/mission_state_entry.hpp"
#include "bsp/ocean_spectrum.hpp"
#include "bsp/mission_state_frame.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_missionlua_cc13.hpp"
#include "bsp/plane_squadron_host.hpp"
#include "bsp/render_tail.hpp"
#include "bsp/session_polls.hpp"
#include "bsp/session_participant_pools.hpp"
#include "bsp/simulation_gate.hpp"
#include "bsp/world_entities.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/recon_sensor_pass.hpp"
#include "bsp/world_ocean.hpp"
#include "bsp/unit_gunnery_pass.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/warning_messages.hpp"
#include "bsp/warning_reports_cc13.hpp"
#include "bsp/cc13_powerup.hpp"  // packet cc13-powerup
#include "bsp/game_hosts_ai.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/vfs_mounts.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <sstream>

namespace bsp::game {
// Packet cc9_authored_command_order: see the call sites below.
// ON since the JM08 / USN01 / USN04 / USN02 pairs (SENTITY_INIT_ATTACH_ORDER.md 23.4).
constexpr bool kAuthoredCommandsAfterLoadWalkBound = true;

namespace {

// 004e53ad's argument: the global clock in milliseconds, 00f876a4 times the
// double 1000.0 at 00ce47a0.
constexpr float kMillisecondsPerSecond = 1000.0f;

// DIAGNOSTIC, packet cc9_ship_motion_hydro: BSP_HULL_ATTITUDE_TRACE=1 turns on
// the per-ship "hull attitude" lines after each tenth motion step; =2 after every
// motion step (packet cc9_hull_kick_size). Returns the interval, 0 when off.
unsigned hull_attitude_trace_interval() {
    static const unsigned interval = [] {
        char* text = nullptr;
        std::size_t bytes = 0;
        unsigned value = 0;
        if (_dupenv_s(&text, &bytes, "BSP_HULL_ATTITUDE_TRACE") == 0 && text != nullptr)
            value = text[0] == '1' ? 10u : (text[0] == '2' ? 1u : 0u);
        std::free(text);
        return value;
    }();
    return interval;
}

// The two profiler slots the in-mission frame brackets itself with are held at
// 0109db08 and 0109db14, both filled at run time by the counter registration
// 00408720 and therefore zero in the image. The executable picks its own, the
// same substitution milestone 2c recorded for the application frame's slot.
constexpr int kGameBlockSlot = 2;
constexpr int kRenderBlockSlot = 3;

// Milestone 2g. How many frames the exit path is given after the mission leaves
// game state 0Dh before the run gives up on it. The recovered path needs two:
// one to dispatch request 10h and lower the suspension the teardown arm raised,
// and one to dispatch the request 04h the state 11h arm enqueued. The bound
// exists so a path that stalls ends the run instead of spinning.
constexpr unsigned long long kMaxExitFrames = 8;

void format_address(std::uint32_t value, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(value));
}

}  // namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

// Packet cc9_warning_manager_tick (docs/WARNING_MANAGER.md). True: the
// warning manager's per-frame calls from 00987590 run as the image's routines
// under their own names (00982540 the input channel pump, 0096D540 the prompt
// poll, 00968550 the deadlines), the 4 s accumulator takes the director's own
// argument (0098759E FLD [EBP+8]), and 00977990 is reached as a named record.
// False: the three calls are records under the old MissionEvents names and the
// accumulator reads the world state's never-written delta.
constexpr bool kWarningManagerTickBound = true;

// Packet cc15-wB4. True: the global_subsystems step loads the manager+108h
// message table as 009870A0 BSP_WarningManager_Init does (its only caller is
// 004DC6A0): Scripts/datatables/Warnings.lua through 00bdf4c0/00bdf310, run in
// a Lua state of its own (009870A0 opens one with 00B6A020 mask 1; the
// repository's Lua 5.1.1 stands in, as for decals.lua), then
// Warnings.escapecharacters.{entity,playerunit_section} and Warnings.messages
// through load_warning_tables_009870a0. message_id_known answers 0096DBB0 over
// that table. The rest of Init (roster 00973F20, the two environment effects,
// the hooks) is not run. Messages.lua (007065E0, load_warning_bank) is the
// MessageManager's voice bank, not this table. The same switch runs 009813A0's
// default kill warning, 00981E07..00982096, after the loss entry's `kill`
// channel record: its gates from the host's own state (recon level and
// 00803CE0 through the recon sensor pass, read only), 009744C0's id, and
// 009763E0 into the pending queue, and 00977690's accepted torpedo report
// builds its 00974150 "torpedo" warning and reports it the same way. An
// accepted warning runs its vtable[3] activate (0097F970 kill, 0097F490
// torpedo, 0097FA90 hit; see warning_activate_cc15): the 00979B50 variant
// pick, whose one draw is a SUBSTITUTION (warning_pick_variant_draw), the
// 00803CE0 relation test and the 00976B00 clip. The MessageManager
// [game+21DCh] is not hosted, so 00705E00 and 0097D650's 0097B5E0 step are
// counted as not evaluated. voice_ready 005B71D0 and 00974070 stay empty, so
// an accepted warning is Queued and never applied. False: no table, no kill
// block, and every id is unknown.
// ON (cc9-lua69, 2026-10-18): USN01 pair with steps a, b, hit and 3a, gameplay identical
// (CC15_WARNING_APPLY_GAP.md closing note).
constexpr bool kWarningMessageBankCc15Bound = true;

// cc15-ob1 (docs/CC15_OBJECT_MESSAGEMANAGER.md). True: the MessageManager
// [game+21DCh] exists as the image builds it. The construct_world step runs
// 00707480 on a fresh 70h block (004DFA1A..004DFA4B; a later load destroys the
// previous one through 007071F0 first, as 004D2BB0 does at 004D2D04), and the
// load_warning_bank step runs 004DFB70's main arm 004E1374..004E17E9: 007065E0(1)
// over Scripts/datatables/Messages.lua (the spoken-language sweep's mode 2 and
// the FileBlocks are seams, see the doc). 00976B00's 00705E00 then resolves the
// clip's record from the manager (records_resolved / record_misses replace
// records_not_evaluated). False: no manager, as before.
constexpr bool kMessageManagerCc15Bound = true;

// cc15-ob1 (docs/CC15_OBJECT_GLOBALSUBSYSTEMS.md). True: the global_subsystems
// step builds four of 004DC6A0's owners in its order through their typed bodies:
// traffic 004A43C0 + 0049D690 [game+21D0h], panel 00452660 + 0044FA30
// [game+21E4h], the warning owner 0098A020 [game+21E0h], then weather 00445B10 +
// 00444D20 [game+21C4h]. The power-up manager 008EDC60 between panel and warnings
// is cc13-powerup's (kPowerupGrantBound) and keeps its own path. The warning
// owner becomes the warning manager this file runs (Impl::warning() and
// warning_tables()): 009870A0's Warnings.lua fragment loads into its +108h
// tables, once per owner, and its scalar resets run; its roster, effects and
// hooks do not. A later load first tears the set down in 004D2BB0's order. False:
// no owners; the manager is the host's own two fields, as before.
constexpr bool kGlobalSubsystemsCc15Bound = true;

// Packet cc9_construct_world part 2 (docs/CONSTRUCT_WORLD.md section 9). True:
// the load step construct_world 004DE610 sets the world-active byte
// [game+19CCh]+4ACh as 004CB030 does at 004CB098, held here as
// world.world_gate.enabled. The fixed-step fan-out's gate 00875E69..00875E7F
// and the gate of 00481640 at 0048164B read it, so rows 9..13 of the fan-out
// and the TrafficConfig walk (game+21D0h, [+8]->vtable[1] = 00487270) are
// reached as named records. The rest of 004DE610 (scene nodes, ocean, sky)
// stays the load record; the 97 lists are the units host's own. The teardown
// 004D2BB0 clears the byte through the destructor (00904C67); this host runs
// no teardown, so the byte stays set to the end of the run. False: the byte is
// never set and both gates stay closed. ON by the verdict: the USN04 and
// USN02 pairs moved only the predicted record rows (docs section 10).
constexpr bool kWorldActiveByteBound = true;

// Packet cc9_construct_world_p3 (docs/CONSTRUCT_WORLD.md section 11). True:
// 00977990 ScanProximity runs as the image's walk. For each entity of world
// list 6 (the units host's registry, [game+19CCh]+64h) that is live (+5Ch set,
// +5Dh, +60h, +5Eh clear) and has more than one part descriptor (+348h..+34Ch),
// it walks list 24 ([game+19CCh]+13Ch, plane squadrons) for an entity of
// another side that 00803CE0 rates enemy-identified and whose +3D0h leader
// is within 2000 m (squared 4.0e6, 00D09FE8). The per-entity record
// 00975D00 (manager+184h, fields +4h deadline and +8h effect) is created
// either way; a hit spawns the point effect 0096C070 and holds it 2.0 s
// (00D7A308); a miss past the deadline with a live effect stops it
// (00867B10). Two labelled stand-ins: the part test answers true for every
// list-6 entity (no units-host size entry for +348h yet), and list 24 is empty
// (no host registrar fills it). False: one record per call, as before. ON by
// the verdict: USN04 and USN02 pairs identical but for the scan row (section 12).
constexpr bool kScanProximityBound = true;

// Packet cc9_scan_units_entries (docs/CONSTRUCT_WORLD.md section 18). True: the
// two stand-ins above are retired with the units host's entries from
// cc9_units_contracts. The part test is the image's 009779FB..00977A03,
// (+34Ch - +348h) >> 2 >= 2, through unit_part_descriptor_count; list 24 is
// world_list_entry(24, i), one node per plane squadron, whose own live bytes,
// side and 00803CE0 rating are tested (00977A36..00977A74) and whose leader
// [+3D0h] (squadron_list_24_leader) gives the position (00977A76); the squared
// distance is summed in the x87 order, (dx^2 + dy^2) + dz^2, and stored to a
// float before the double compare (00977AC8..00977AEA). False: the stand-ins.
// ON by the verdict: USN04 444 hits over 10,576 list-24 nodes, USN02 identical,
// gameplay identical on both (docs/CONSTRUCT_WORLD.md section 18).
constexpr bool kScanProximityUnitsEntriesBound = true;

// Packet p20_squadron_helm (docs/OCEAN_WAVE_FIELD.md section 79). True: a helm
// `moveto` whose unit is a plane squadron member goes through
// GameScriptOrdersHost::player_plane_moveto (the members' 00E08F68 moveto and
// the 0099A170 install at delivery). False: the ship path, whose command row a
// plane's bot never reads, so the squadron keeps its scene moveto (BSM09 Donald).
constexpr bool kHelmPlaneMovetoBound = true;   // ON: OCEAN_WAVE_FIELD 79 (BSM09 pair)

// Packet cc9_construct_world_p8 (docs/CONSTRUCT_WORLD.md section 15). True:
// 00481640's call is the walk 00487270 itself: the std::list whose sentinel is
// group+10h (group = [TrafficConfig+8h], the 20h object 0049D690 builds), each
// node's payload +8h taking its slot 1 with the delta. The list's only producer
// is the scene-traffic commit 004A5620 -> 004A50D0 (one 170h runtime per `item`
// of the scene's `traffic` block; docs/SCENE_TRAFFIC_BLOCK.md). This host does
// not load that block (SceneContents::load_traffic_block stays a record), so
// the walk visits nothing. SUBSTITUTION, exact only for a scene whose traffic
// block has no item, as USN04 (usn_19_coralus.scn line 2567) and USN02
// (usn_2_java.scn line 1760) are. False: the walk is the named record. ON by
// the verdict: USN04 and USN02 pairs identical but for this row (section 15).
constexpr bool kTrafficWalkBound = true;

// Packet cc9_scaled_delta_write (docs/SCALED_DELTA_WRITE.md). True: the frame
// writes game+21F0h, the scaled delta 004C6E30 stores at 004E4D45, into the
// world tick state every frame, so each world-tick reader of +21F0h sees it.
// False: that field is never written and reads 0.
constexpr bool kScaledDeltaWriteBound = true;

struct GameMissionFrameHost::Impl {
    Impl(GameHostLog& log_in, GameVfsHost& vfs_in, GameMissionLuaHost& lua_in,
        bsp::SessionParticipantPools& participants_in,
        GameFrameProfiler* profiler_in, std::string language_in)
        : log(log_in), vfs(vfs_in), lua(lua_in), participants(participants_in), profiler(profiler_in),
          language(std::move(language_in)) {
        // Both take references to members declared below, which are already
        // default constructed when this body runs.
        fixed_step = std::make_unique<GameFixedStepHost>(log, dynamics);
        result = std::make_unique<GameMissionResultHost>(log, control);
        // Milestone 2m: the six fan-out rows whose reconstructions are on main.
        step_subsystems = std::make_unique<GameStepSubsystemsHost>(log);
        fixed_step->attach_subsystems(step_subsystems.get());
        // Packet cc9_sentity_init_all: row 12's pending list is the Lua host's.
        fixed_step->attach_entity_init(&lua);
        // Packet cc9_run_extra_fixed_step: GenerateObject's 00874D00.
        lua.attach_extra_fixed_step(fixed_step.get());
        // Packet cc13-fix20: the Scoring_SetMissionCompleted and PlayBinkMovie rows.
        lua.attach_mission_result(result.get());
    }

    ~Impl() {
        release_units();
        // Not in release_units: the scene load calls that before the scripts run,
        // and the result object lives for the whole mission.
        lua.attach_mission_result(nullptr);
        // Packet cc13-powerup: 004D2BB0 deletes the manager at 004D2D84..004D2D96,
        // and the next load builds a new one. The runtime's keys go with the
        // frame's world, which holds the active slot's record values.
        if constexpr (bsp::cc13_powerup::kPowerupGrantBound) {
            if (powerup_binding != nullptr) {
                bsp::cc13_powerup::PowerupRuntime& powerups =
                    bsp::cc13_powerup::powerup_runtime();
                if (powerups.host == powerup_binding.get()) {
                    bsp::cc13_powerup::destroy_powerup_manager_004d2d84(powerups);
                    // Packet cc13-powerup-bind: the unit view goes with the lists.
                    bsp::game_powerups::publish_powerup_unit_resolver(nullptr);
                }
            }
        }
    }

    void release_units() noexcept {
        // HUD persists across frames; Lua persists until after this frame dies.
        // Release every outward borrow before destroying the canonical slots.
        if (hud != nullptr && units != nullptr) hud->detach_world_2k();
        lua.attach_script_orders(nullptr);
        lua.attach_units_hooks(nullptr);
        if (step_subsystems != nullptr) step_subsystems->attach_units(nullptr);
        if (units != nullptr) units->set_ship_ai(nullptr);
        ship_ai.reset();
        script_orders.reset();
        world_host.reset();
        units.reset();
    }

    GameHostLog& log;
    GameVfsHost& vfs;
    GameMissionLuaHost& lua;
    GameObserverRuntime* observer_runtime{nullptr};
    bsp::SessionParticipantPools& participants; // persistent GameMissionHost owner
    GameFrameProfiler* profiler{};
    std::string language;
    // cc15-ob1: [game+21DCh], built by the construct_world step (kMessageManagerCc15Bound).
    std::unique_ptr<GameMessageManagerHost> messages;

    GameMissionLoadRunSummary load{};
    GameMissionEntrySummary entry{};
    GameMissionFrameRunSummary frames{};

    // The parts of the game object this milestone owns.
    bsp::MissionSceneLoadState scene_state{};
    bsp::MissionStateEntryState entry_state{};
    bsp::DeviceWaitLatch device_latch{};
    bsp::MissionFrameState frame_state{};
    bsp::WorldTickState world{};
    bsp::InputTickState input{};
    bsp::GameFrameControlState control{};
    bsp::MenuInterfaceState menus{};
    bsp::SoundRequestQueue sound_requests{};
    bsp::ParticleClock particle_clock{};
    bsp::MissionCounterState mission_counters{};
    bsp::AudioSettings audio{};
    bsp::InputBindingDeviceGroups device_groups{};
    std::vector<bsp::HintCooldown> hint_cooldowns{};
    // cc15-hint2 (kHintTriggerPassesCc15Bound): this frame's reads, bound into
    // the tracker [00E198D0] while the frame host lives.
    std::unique_ptr<HintFrameQueries> hint_frame;
    bool main_menu_manager_released{false};   // 00e198ac
    bool cinematic_hidden{false};             // game+634h
    bool cinematic_allow_simulation{false};   // game+635h
    int script_reentry_depth{0};              // game+644h
    // kEngineMovieEntryBound: the load took the mode-9 arm, so the first
    // in-mission frame owes 0045F600's tail (the player's skip).
    bool engine_movie_skip_pending{false};
    std::int32_t published_mission_id{0};     // [00f8a2fc]+48h
    float world_clock{0.0f};                  // 00f876a4
    // Packet cc9_warning_manager_tick: the manager's fields this process holds.
    bsp::WarningManagerState warning_host_state{};
    std::map<std::size_t, float> warning_effect_deadline;  // 00975D00 record +0h, per unit
    // The same 00975D00 record's +4h deadline and +8h effect, which 00977990
    // uses; held apart from +0h because the torpedo path keeps its own map.
    struct ProximityRecord {
        float deadline_04{-1.0e10f};  // 00CE4ADC
        bool effect_live{false};      // +8h non-null
    };
    std::map<std::size_t, ProximityRecord> proximity_records;
    unsigned long long proximity_ships_scanned{0};
    // Packet cc9_scan_units_entries.
    unsigned long long proximity_part_rejects{0};
    unsigned long long proximity_list24_nodes{0};
    unsigned long long proximity_list24_no_leader{0};
    unsigned long long proximity_records_created{0};
    unsigned long long proximity_hits{0};
    unsigned long long proximity_expiries{0};
    unsigned long long warning_scans{0};
    unsigned long long warning_torpedo_calls{0};
    unsigned long long warning_torpedo_accepted{0};
    unsigned long long warning_effect_calls{0};
    unsigned long long warning_effects{0};
    unsigned long long warning_deadline_expiries{0};
    // Packet cc9_loss_report_entry: 009813A0 calls, and its guard's passes by side.
    unsigned long long warning_loss_calls{0};
    unsigned long long warning_loss_side0{0};
    unsigned long long warning_loss_side1{0};
    unsigned long long warning_loss_other{0};
    // Packet cc9_loss_warning: the text keys chosen, the pending records the
    // cancel removed, the proximity records a ship's loss erased, and the
    // manager+15Ch stamp per unit (now + 2.0, 00976FBE..00976FE8).
    unsigned long long warning_loss_text_uslost{0};
    unsigned long long warning_loss_text_japanlost{0};
    unsigned long long warning_loss_cancelled{0};
    unsigned long long warning_loss_proximity_erased{0};
    std::map<std::size_t, float> warning_loss_stamp_15c{};
    // Packet cc15-wB4: manager+108h..+144h and the 0096DBB0 answers per id.
    bsp::WarningMessageTables warning_host_tables{};
    // cc15-ob1, kGlobalSubsystemsCc15Bound: 004DC6A0's owners. Once the warning
    // owner [game+21E0h] exists, it is the manager: warning() is its +D0h..+19Ch
    // state and warning_tables() its +108h tables. Without it, the two fields above.
    std::unique_ptr<GameGlobalSubsystemsHost> global_subsystems;
    bsp::WarningOwner* warning_owner_21e0() const {
        return global_subsystems ? global_subsystems->warnings() : nullptr;
    }
    bsp::WarningManagerState& warning() {
        bsp::WarningOwner* owner = warning_owner_21e0();
        return owner != nullptr ? owner->state : warning_host_state;
    }
    bsp::WarningMessageTables& warning_tables() {
        bsp::WarningOwner* owner = warning_owner_21e0();
        return owner != nullptr ? *owner->tables_108 : warning_host_tables;
    }
    bool warning_tables_loaded{false};
    std::string warning_tables_error{};
    std::map<std::string, std::pair<unsigned long long, unsigned long long>> warning_id_queries{};
    // The kill block of 009813A0 (00981E07..009820AC) under the same switch.
    struct KillWarningCounts {
        unsigned long long entries{0}, override_104{0}, suppressed_d0{0}, slot{0}, clock{0},
            suppress_77edf0{0}, recon_not_evaluated{0}, recon_below{0}, plane_throttled{0},
            filtered{0}, reports{0};
    } kill_warning{};
    // The hit block of 00988510 (009895D8..00989992) under the same switch.
    struct HitWarningCounts {
        unsigned long long calls{0}, suppressed_d0{0}, clock{0}, flags{0},
            recon_not_evaluated{0}, recon_below{0}, stamp_throttled{0},
            player_controlled{0}, own_shooter{0}, plane_count{0},
            enemy_payload_not_evaluated{0}, neutral{0}, not_reportable{0},
            command_building_not_evaluated{0}, reports{0};
    } hit_warning{};
    bool kill_plane_throttle_init{false};      // 00F8A0D0 bit 0
    float kill_plane_throttle_last{0.0f};      // 00F8A0CC
    std::map<std::string, std::array<unsigned long long, 4>> warning_report_outcomes{};
    std::map<std::string, unsigned long long> warning_lua_callbacks{};
    // Step 3a: the activates' variant use tables (manager +144h/+150h), the
    // warning-local generator of the SUBSTITUTION draw, and their counts.
    bsp::cc13_seg49_64_78::WarningVariantTable warning_variants{};
    std::mt19937 warning_variant_rng{};
    struct WarningActivateCounts {
        unsigned long long activations{0}, single_variant{0}, draws{0},
            sound_keys_untouched{0}, sound_keys_not_evaluated{0}, relation_blocked{0},
            clips{0}, records_not_evaluated{0}, records_resolved{0}, record_misses{0},
            sound_keys_resolved{0}, sound_keys_unmatched{0};
    } warning_activate{};
    // Packet cc13-powerup: the binding published with the power-up registry.
    std::unique_ptr<bsp::cc13_powerup::PowerupManagerHost> powerup_binding{};
    // Packet cc13-powerup-bind: the unit view 008E4680 reads, published with it.
    std::unique_ptr<bsp::game_powerups::PowerupUnitResolver> powerup_unit_resolver{};
    bool device_edge_injected{false};         // the executable's own injection
    // Packet cc_mission_tick's reconstruction of the four-call opener of every
    // simulated frame, and the dynamics list behind its fourth call.
    bsp::FixedStepClock fixed_clock{};        // 00f876a1..00f876b8
    bsp::GameDynamicsState dynamics{};        // game+30h
    bool unit_lists_built{false};             // game+193Ch
    unsigned long long fixed_steps{0};
    // Milestone 2g: the fixed step's own body (00875cc0, 00875e0c, 00875670,
    // 00875fd1) and the mission result object at game+7188h with the path out
    // of state 0Dh. Both are constructed in the body above.
    std::unique_ptr<GameFixedStepHost> fixed_step;
    std::unique_ptr<GameMissionResultHost> result;
    // Milestone 2m: the six fan-out rows' own reconstructions.
    std::unique_ptr<GameStepSubsystemsHost> step_subsystems;
    // Milestone 2h: 004d4df0, the scene contents pass. Built on the load walk's
    // own row, because the "2_" file block it opens belongs to that step.
    std::unique_ptr<GameSceneContentsHost> scene_contents;
    // Milestone 2i: the created units and the world chain the walk reads. Both
    // are built on the load's own load_scene_contents row, because that is the
    // step that creates the instances.
    std::unique_ptr<GameUnitsHost> units;
    std::unique_ptr<GameWorldHost> world_host;
    // Milestone 2m: the host the eight reconstructed binding bodies run over.
    // Built with the units, because every one of the eight addresses an entity.
    std::unique_ptr<GameScriptOrdersHost> script_orders;
    // Milestone 2n: one ship AI controller per created unit, and the weapon
    // director's automatic target selector beside it. Built with the units,
    // because the order slot the controller publishes lives on the unit.
    std::unique_ptr<GameShipAiHost> ship_ai;
    long order_frame{-1};             // --order-frame N
    // Milestone 2o, --ai-drive <name>=<throttle>,<rudder>, engaged on the same
    // frame as the player order.
    std::string ai_drive_unit;
    float ai_drive_throttle{0.0f};
    float ai_drive_rudder{0.0f};
    float order_throttle{0.0f};
    float order_rudder{0.0f};
    float order_speed{0.0f};
    bool order_speed_set{false};
    bool order_issued{false};
    // Milestone 2l: --order <command>[:<entity>], the same frame, issued
    // through the recovered command path instead of the order ring.
    std::string order_command;
    std::string order_command_target;
    std::string order_command_unit;   // milestone 2n, --order-unit <name>
    // Packet cc9_scripted_helm_orders: --helm-orders <file>.
    struct HelmOrder {
        int line{0};
        long frame{0};
        bool takehelm{false};      // packet cc9_helm_orders_helm_route
        float throttle{0.0f};      // takehelm
        float repeat_seconds{0.0f};  // moveto ... repeat N
        bool select{false};        // packet cc9_player_order_capture_row
        bool release_gun{false};   // packet cc13-releasegun
        bool attack{false};        // packet cc9_player_attack_row
        bool squadron_target{false};   // packet cc9_player_squadron_target
        bool launch{false};        // packet cc9_helm_orders_launch
        // Packet cc9_player_air_ops_order_line: `order <base> <slot> <1|2|3>
        // [target...]`, the Support Manager's 82h order to a held slot.
        bool air_order{false};
        int air_order_slot{0};     // 1-based
        int air_order_kind{0};     // 1 moveto, 2 recall, 3 attack
        int launch_slot{0};        // launch: 1-based slot, 0 = 006C7210's pick
        int launch_class{0};       // launch: VehicleClass id
        int launch_count{0};       // launch: planes requested
        int launch_loadout{-1};    // launch: `loadout <n>`, -1 = the class default
        // `build <shipyard> class <id>`: the strategic map's shipyard purchase
        // 00673A10 through GameGunneryHost::shipyard_order (cc9-ships38).
        bool build{false};
        int build_class{0};
        // Packet cc9_player_fire_group_line: `fire <ship> group <g> [release]`,
        // the fire key 99h held (or let go) with weapon group g selected:
        // 005484F0's message 79h through GameGunneryHost::player_fire_weapon_group.
        bool fire_group{false};
        int fire_group_id{0};
        bool fire_held{true};
        // Packet cc13-firehold: `input <action> [hold <frames>]`, an input action
        // held through the menu host's action records (inject_input_action_hold),
        // so 005484F0 reads the fire key 99h (and the group keys 9Ch..9Fh) as it
        // reads a real press.
        bool input_hold{false};
        int input_action{0};
        unsigned input_frames{1};
        // Packet cc13-helmaim: `aim <yaw> <pitch>`, degrees, the ship camera
        // mover's +384h / +388h reached through the look axes (inject_view_aim).
        bool view_aim{false};
        float aim_yaw{0.0f};
        // Packet cc13-aimunit: `aim unit <name...> [pitch]`, a per-frame track of
        // the named unit (inject_view_aim_track); `unit` holds the name.
        bool aim_track{false};
        bool aim_track_has_pitch{false};
        float aim_pitch{0.0f};
        // Packet cc9_player_bomb_release: `release <unit> [on <target...>
        // [within <m>] [until <frame>]]`, the plane screen's bomb fire 006082D0.
        bool bomb_release{false};
        float release_within{25.0f};   // release ... on: the sight radius, metres
        long release_until{-1};        // release ... on: last frame tried
        bool release_have_last{false}; // the target's previous sample, for its velocity
        float release_last[3]{};
        float release_last_seconds{0.0f};
        float release_best{-1.0f};     // the nearest predicted miss seen
        std::string target;        // attack / target: the target entity's name
        float stop_radius{-1.0f};  // takehelm ... stop R; negative: none
        unsigned applications{0};
        std::string unit;
        std::string point_name;   // empty for an x/z point
        float x{0.0f};
        float z{0.0f};
        bool applied{false};
    };
    std::vector<HelmOrder> helm_orders;
    // A `torpedo <ship> at <target>` press held until its `release` line: the held
    // 79h is re-sent every mission frame, as 005484F0 sends one per frame.
    struct HelmTorpedoHold {
        std::size_t unit{0};
        std::string target;
        int line{0};
        unsigned long long frames{0};
    };
    std::vector<HelmTorpedoHold> helm_torpedo_holds;
    std::string helm_orders_path;
    struct HelmMarker {
        std::string name;
        float position[3]{};
    };
    std::vector<HelmMarker> helm_markers;   // the scene's markers, by name
    unsigned long long helm_orders_applied{0};
    unsigned long long helm_orders_refused{0};
    // Packet cc13-mvsel (log only): moveto lines issued to a unit that was not
    // the controlled unit ([00E188D8]) at issue time, which a player cannot do
    // without the select first (SCRIPTED_HELM 8.2 difference 2).
    unsigned long long helm_moveto_unselected{0};
    unsigned long long helm_role_forced{0};   // packet cc13-rolef
    // Packet cc9_player_order_capture_row: the pending `stop R` of the last
    // applied takehelm, checked every step until it fires once.
    bool helm_stop_armed{false};
    std::size_t helm_stop_unit{0};
    float helm_stop_radius{0.0f};
    float helm_stop_x{0.0f};
    float helm_stop_z{0.0f};
    int helm_stop_line{0};
    float mission_frame_seconds{0.0f};  // --mission-frame-seconds S
    // Milestone 2j, --trajectory-csv <path>: one row per unit per fixed step.
    std::string trajectory_csv_path;
    bool trajectory_csv_tried{false};
    GameTrajectoryCsv trajectory_csv;
    // Milestone 2h: the in-mission HUD, owned by GameMenuHost because its
    // screens register into that object's copy of the registry at 00e18b60.
    GameHudHost* hud{nullptr};
    long complete_frame{-1};                  // --mission-complete-frame N
    bool complete_injected{false};
    unsigned long long exit_frames{0};        // frames run after state 0Dh ended
    bool exit_path_finished{false};
    bool drain_skip_reported{false};

    // Packet cc9_sunk_ship_flush: 00903670 over the unit slots, in unit order
    // (the image walks [world+4h] along +38h). 00903680..00903697: +5Eh set,
    // +6Ch clear, and no parent (a unit slot has none here), then 00922FD0:
    // +5Eh = 1, +5Dh = 1, +5Ch = 0, +6Ch = 1 (00922FDE..00922FE8), the children
    // at +48h (none here) and vt[84h] (ship 00819880, plane 007CC580).
    std::size_t unit_kills{0};
    void flush_unit_kills_00903670() {
        if (units == nullptr || step_subsystems == nullptr) return;
        for (std::size_t unit = 0; unit < units->count(); ++unit) {
            bsp::SceneNodeFlags flags;
            if (!units->unit_scene_node_flags(unit, flags)) continue;
            if (!flags.destroyed || step_subsystems->killed_counter_006c(unit) != 0) continue;
            const void* identity = units->unit_identity(unit);
            if (identity == nullptr) continue;
            flags.destroyed = true;
            flags.torn_down = true;
            flags.active = false;
            units->store_scene_node_flags(identity, flags);
            step_subsystems->mark_killed_006c(unit);
            ++unit_kills;
            record("Entities::kill_vtable84", 0x00923010u);
        }
    }
    void record(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.unimplemented(method, text);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.implemented(method, text);
    }
};

namespace {
// Packet cc9_warning_manager_tick: the manager of the live mission frame host,
// for the two report entries the ship AI's warning timer calls.
GameMissionFrameHost::Impl* g_warning_owner = nullptr;
}  // namespace

// ---------------------------------------------------------------------------
// bsp::InputTickHost, for the load's 00a92c40 and the device wait's own call
// ---------------------------------------------------------------------------

namespace {

class InputTickBinding final : public bsp::InputTickHost {
public:
    explicit InputTickBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}

    void backend_update(float seconds) override {
        static_cast<void>(seconds);
        // 00f8bbf4 vtable +4h. The input backend is the input owner's; this
        // process has the reconstructed action records but no backend object.
        owner_.record("InputTick::backend_update", 0x00f8bbf4u);
    }
    bool take_backend_bindings_dirty() override { return false; }  // backend+D4h
    const bsp::InputBindingDeviceGroups& binding_device_groups() const override {
        return owner_.device_groups;
    }
    void post_update_hook() override {}  // 00f8bbfc, null in this process

    std::uint8_t device_query_1c(bsp::InputDevice&, std::uint32_t) override { return 0; }
    std::uint8_t device_query_20(bsp::InputDevice&, std::uint32_t) override { return 0; }
    float device_value_24(bsp::InputDevice&, std::uint32_t) override { return 0.0f; }
    float crt_sqrt_00bf7030(float sum_of_squares) override {
        return sum_of_squares <= 0.0f ? 0.0f : std::sqrt(sum_of_squares);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::WorldTickHost. Every container the recovered walks read is empty, so the
// per-entity methods below are unreachable in this process and never record.
// ---------------------------------------------------------------------------

// 00803CE0(ECX = side, EDX = unit): 3 when the side's detection level of
// the unit is below 2 (00803CFE); else 0 own, 2 neutral, 1 enemy. The level
// is the host's recon sensor pass (ReconSensorPassState::level).
int warning_relation_00803ce0(GameMissionFrameHost::Impl& host, int side,
    std::size_t unit) {
    const GameGunneryHost* gunnery =
        host.units != nullptr ? host.units->gunnery() : nullptr;
    if (gunnery == nullptr) return 3;
    if (gunnery->recon_sensor_pass_state().level(side, unit)
            != bsp::ReconDetectionLevel::identified) {
        return 3;
    }
    const int other = host.units->unit_side_0054(unit);
    if (side == 2) return other == 2 ? 0 : 2;              // 00803D26
    if (side == other) return 0;                           // 00803D15
    return other == 2 ? 2 : 1;                             // 00803D18
}

// Packet cc15-wB4 step 3a: which vtable[3] activate the reported warning runs.
enum class WarningActivate {
    plain,    // 0097F490, the type-1 entity warning (torpedo)
    kill_6c,  // 0097F970, the kill warning: victim +6Ch, relation test
    hit_80,   // 0097FA90, the hit warning: entity +80h, relation test
};
void warning_activate_cc15(GameMissionFrameHost::Impl& host,
    bsp::cc13_seg49_64_78::WarningObject& warning, WarningActivate kind);

class WarningManagerBinding final : public bsp::WarningManagerHost {
public:
    explicit WarningManagerBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}
    // 009763E0 calls vtable[3] on the object it just queued; the record is
    // the queue's projection of `accepted`.
    WarningManagerBinding(GameMissionFrameHost::Impl& owner,
        bsp::cc13_seg49_64_78::WarningObject& accepted, WarningActivate kind)
        : owner_(owner), accepted_(&accepted), kind_(kind) {}
    void pump_input_channel_00982540() override {}
    void scan_proximity_00977990() override {}
    bool input_action_pressed(int) override {
        owner_.record("WarningManager::input_action_004c43c0", 0x004c43c0u);
        return false;
    }
    void raise_prompt_hud(int) override {
        owner_.record("WarningManager::raise_prompt_hud", 0x0096d540u);
    }
    void call_mission_lua(const std::string& callback) override {
        if constexpr (kWarningMessageBankCc15Bound) ++owner_.warning_lua_callbacks[callback];
        owner_.record("WarningManager::call_mission_lua", 0x00887e50u);
    }
    void stop_collision_sound() override {}
    bool message_id_known(const std::string& id) override {
        if constexpr (kWarningMessageBankCc15Bound) {
            const bool known = bsp::warning_message_id_known_0096dbb0(owner_.warning_tables(), id);
            auto& counts = owner_.warning_id_queries[id];
            ++(known ? counts.first : counts.second);
            owner_.done("WarningManager::message_id_known", 0x0096dbb0u);
            return known;
        } else {
            return false;
        }
    }
    void report_error(const char*) override {}
    void destroy_warning(std::size_t, bool) override {}
    void on_warning_accepted(std::size_t index) override {
        if constexpr (kWarningMessageBankCc15Bound) {
            if (accepted_ == nullptr) return;
            warning_activate_cc15(owner_, *accepted_, kind_);
            bsp::WarningRecord& record = owner_.warning().pending[index];
            record.state = accepted_->state;                    // +0Ch
            record.applied_at = accepted_->validity_ref_time;   // +10h
            record.clip_count = accepted_->messages.size();     // +14h
        }
    }
    bool voice_ready(std::size_t) override { return false; }
    void apply_warning_00974070(std::size_t) override {}

private:
    GameMissionFrameHost::Impl& owner_;
    bsp::cc13_seg49_64_78::WarningObject* accepted_{nullptr};
    WarningActivate kind_{WarningActivate::plain};
};

// Packet cc15-wB4: what 009744C0 asks of the victim and its killer, answered
// from the host's unit, gunnery (kill attribution, read only) and recon state.
// Host entities travel in the opaque WarningEntity as one-based unit indices.
bsp::WarningEntity warning_entity_of_unit(std::size_t unit) {
    return reinterpret_cast<bsp::WarningEntity>(static_cast<std::uintptr_t>(unit) + 1u);
}

class KillWarningQueryBinding final : public bsp::KillWarningQuery {
public:
    explicit KillWarningQueryBinding(GameMissionFrameHost::Impl& host) : host_(host) {}
    bool is_kind_5c(bsp::WarningEntity entity, std::int32_t kind) override {
        std::size_t unit = 0;
        return unit_of(entity, unit) && host_.units->unit_is_kind_of(unit, kind);
    }
    // 00965810's other arm reads +9D4h of a kind-0Fh plane; the host fuses a
    // squadron with its members, so no plane entity carries one (as at 00977690).
    bsp::WarningEntity plane_owner_9d4(bsp::WarningEntity) override { return nullptr; }
    bsp::WarningEntity controlled_unit_e188d8() override {
        return host_.units->controlled_bound()
            ? warning_entity_of_unit(host_.units->controlled_index()) : nullptr;
    }
    std::int32_t local_slot_18ec() override { return 0; }  // single player
    std::int32_t local_side_id() override { return local_player_party(); }
    std::int32_t side_54(bsp::WarningEntity entity) override {
        return host_.units->unit_side_0054(required_unit(entity));
    }
    bsp::WarningRelation relation_00803ce0(bsp::WarningEntity entity) override {
        return static_cast<bsp::WarningRelation>(
            warning_relation_00803ce0(host_, local_side_id(), required_unit(entity)));
    }
    bool is_player_controlled_00965810(bsp::WarningEntity entity) override {
        std::size_t unit = 0;
        if (!unit_of(entity, unit)) return false;  // 00965810 tests ECX for null
        return host_.units->controlled_bound() && host_.units->controlled_index() == unit;
    }
    // victim+2C4h/+2D8h as 0077CE60 left them (GameGunneryHost's kill attribution).
    bsp::WarningEntity killer_2c4(bsp::WarningEntity victim) override {
        const GameGunneryHost* gunnery = host_.units->gunnery();
        if (gunnery == nullptr) return nullptr;
        const std::size_t last = gunnery->kill_last_attacker(required_unit(victim));
        return last == 0 ? nullptr : warning_entity_of_unit(last - 1);
    }
    std::int32_t slot_2d8(bsp::WarningEntity victim) override {
        const GameGunneryHost* gunnery = host_.units->gunnery();
        return gunnery == nullptr ? 0 : gunnery->kill_credited_slot(required_unit(victim));
    }

private:
    bool unit_of(bsp::WarningEntity entity, std::size_t& unit) const {
        const auto raw = reinterpret_cast<std::uintptr_t>(entity);
        if (raw == 0 || raw > host_.units->count()) return false;
        unit = static_cast<std::size_t>(raw - 1u);
        return true;
    }
    std::size_t required_unit(bsp::WarningEntity entity) const {
        std::size_t unit = 0;
        // The native dereferences the pointer without a check.
        if (!unit_of(entity, unit)) throw std::logic_error("kill warning query on a null entity");
        return unit;
    }
    GameMissionFrameHost::Impl& host_;
};

// What 00974C70 asks of a hit. The image's attacker is [hit+4]->vtable[108h](),
// the shot's attribution object; the host's hit event carries its two fields
// this block reads as the shot's team (attacker+1Ch, attacker_player_index) and
// the firing unit (attacker+ACh, `shooter`), so the attacker travels as the
// shooter's entity. attacker+CCh, the weapon, is not in the event: weapon_cc
// answers null, so the kind-2Ah payload test is false. Its only reached use is
// the enemy-target arm, which the caller counts as not evaluated instead.
class HitWarningQueryBinding final : public bsp::HitWarningQuery {
public:
    HitWarningQueryBinding(GameMissionFrameHost::Impl& host, int attacker_player_index)
        : units_(host), attacker_player_index_(attacker_player_index) {}
    bool is_kind_5c(bsp::WarningEntity e, std::int32_t kind) override {
        return units_.is_kind_5c(e, kind);
    }
    bsp::WarningEntity plane_owner_9d4(bsp::WarningEntity e) override {
        return units_.plane_owner_9d4(e);
    }
    bsp::WarningEntity controlled_unit_e188d8() override { return units_.controlled_unit_e188d8(); }
    std::int32_t local_slot_18ec() override { return units_.local_slot_18ec(); }
    std::int32_t local_side_id() override { return units_.local_side_id(); }
    std::int32_t side_54(bsp::WarningEntity e) override { return units_.side_54(e); }
    bsp::WarningRelation relation_00803ce0(bsp::WarningEntity e) override {
        return units_.relation_00803ce0(e);
    }
    bool is_player_controlled_00965810(bsp::WarningEntity e) override {
        return units_.is_player_controlled_00965810(e);
    }
    std::int32_t slot_1c(bsp::WarningEntity) override { return attacker_player_index_; }
    bsp::WarningEntity carrier_ac(bsp::WarningEntity attacker) override { return attacker; }
    bsp::WarningEntity weapon_cc(bsp::WarningEntity) override { return nullptr; }
    // Command-building targets are counted out by the caller before 00974C70.
    bool is_base_78c(bsp::WarningEntity) override { return unreached<bool>(); }
    std::int32_t owner_slot_180(bsp::WarningEntity) override { return unreached<std::int32_t>(); }
    std::int32_t count_command_buildings_of_slot(std::int32_t) override {
        return unreached<std::int32_t>();
    }

private:
    template <typename T> static T unreached() {
        throw std::logic_error("00974C70 reached a command-building query");
    }
    KillWarningQueryBinding units_;
    int attacker_player_index_;
};

// The WarningHost calls 00973A90, 009744C0, 00974150 and 00974C70 make: the clock and the
// observer handles of the +3Ch/+54h/+6Ch slots. The host keeps no observer
// lists, so a link is the slot's pointer in the object. Nothing else is reached
// from those constructors.
class WarningObjectHostBinding final : public bsp::cc13_seg49_64_78::WarningHost {
public:
    explicit WarningObjectHostBinding(GameMissionFrameHost::Impl& host) : host_(host) {}
    float clock_00f876a4() override { return host_.world_clock; }
    void observer_link_00694a60(void*, bsp::cc13_seg49_64_78::WarningSlot) override {}
    void observer_unlink_006952a0(void*, bsp::cc13_seg49_64_78::WarningSlot) override {}
    void observer_destroy(bsp::cc13_seg49_64_78::WarningSlot) override {}
    bool equals_insensitive_00435c40(const std::string&, const std::string&) override {
        return unreached<bool>();
    }
    float entity_distance_0042b2f0(void*, void*) override { return unreached<float>(); }
    void entity_world_position(void*, float[3]) override { unreached<int>(); }
    // 00976B00's [[game+18CCh+4*[game+18ECh]]+28h], the local party.
    std::int32_t local_side_28() override { return local_player_party(); }
    std::int32_t relation_00803ce0(std::int32_t, void*) override {
        return unreached<std::int32_t>();
    }
    void* detector_for_side_210(void*, std::int32_t) override { return unreached<void*>(); }
    bool is_kind_of_5c(void*, std::int32_t) override { return unreached<bool>(); }
    void* plane_squadron_9d4(void*) override { return unreached<void*>(); }
    void* local_player_unit_00e188d8() override { return unreached<void*>(); }
    // 00976B00's 00705E00 reads the MessageManager [game+21DCh]. With
    // kMessageManagerCc15Bound it is the hosted manager's record map; a miss
    // (the image's "<name>" record, which needs the voice manager) stays null.
    // Without it the clip's record is left null and counted.
    const void* resolve_message_00705e00(const std::string& name) override {
        if constexpr (kMessageManagerCc15Bound) {
            if (host_.messages != nullptr && host_.messages->constructed()) {
                const void* record = host_.messages->resolve_record_00705e00(name);
                ++(record != nullptr ? host_.warning_activate.records_resolved
                                     : host_.warning_activate.record_misses);
                if (record == nullptr && host_.warning_activate.record_misses <= 8) {
                    host_.log.notef("message manager record miss: \"%s\" (00705E00)",
                        name.c_str());
                }
                return record;
            }
        }
        ++host_.warning_activate.records_not_evaluated;
        return nullptr;
    }
    void read_message_ref_00706060(bsp::cc13_seg49_64_78::EventArchiveReader&,
        bsp::cc13_seg49_64_78::VoiceClipRef&) override { unreached<int>(); }
    void write_message_ref_00703380(const bsp::cc13_seg49_64_78::VoiceClipRef&,
        bsp::cc13_seg49_64_78::EventArchiveWriter&) override { unreached<int>(); }

private:
    template <typename T> static T unreached() {
        throw std::logic_error("the warning constructors used here do not reach this WarningHost call");
    }
    GameMissionFrameHost::Impl& host_;
};

// SUBSTITUTION (packet cc15-wB4 step 3a, cc9 option ii). 00979B50 takes its
// draw at 00979D0C/00979D11 from 00BD2FC0 with ECX = 1, the per-thread
// generator the image's gunnery also draws from; the host draws from a
// warning-local std::mt19937 instead, so no gunnery sequence moves. This is
// the only call site; the image-exact shared draw (option i) is a sub-switch
// here (docs/RANDOM_STREAMS.md).
std::uint32_t warning_pick_variant_draw(GameMissionFrameHost::Impl& host) {
    ++host.warning_activate.draws;
    return static_cast<std::uint32_t>(host.warning_variant_rng());
}

class WarningVariantBinding final : public bsp::cc13_seg49_64_78::WarningVariantHost {
public:
    explicit WarningVariantBinding(GameMissionFrameHost::Impl& host) : host_(host) {}
    std::uint32_t random_word_00bd2fc0_stream1() override {
        return warning_pick_variant_draw(host_);
    }
    float clock_00f876a4() override { return host_.world_clock; }
    void invalid_parameter_00bf6713() override {
        throw std::logic_error("00979B50 picked no variant (00BF6713)");
    }
    void divide_error() override {
        throw std::logic_error("00979B50 divided by zero ties (00979D1A)");
    }

private:
    GameMissionFrameHost::Impl& host_;
};

// cc15-ob1 (kMessageManagerCc15Bound): 0097D650's key step for the entity
// `unit` on `text`, whose first byte of the separator set sits at pos.
//   tail   = text[pos, pos+1) (00469840(text, pos, 1))
//   prefix = tail + 00E0B080[00803CE0(local party, entity)] (OWN/ENEMY/NEUTRAL/UNKNOWN)
//   keys   = prefix + vtable[10h] name, prefix + vtable[28h] name and prefix +
//            00E0B590[(entity+170h)->vtable[0]()], each only when the local party's
//            recon level of the entity is >= 2 (00803CE0 answers 3 exactly when it is
//            below 2), then prefix + default_ship / default_plane /
//            default_planesquadron / default_landobject by vtable[5Ch] (6, 0Fh, 18h)
//            and 00922A90 (class ids 19h, 1Ah, 1Bh, 1Ch, 36h, 45h, 46h).
// 0097B5E0 then takes the keys in order; for each that the entity escape map
// (manager+11Ch) holds, it rebuilds the text from the picked copy with the
// separator replaced by the mapped value (0096EDD0) and stops at the first text
// the MessageManager's record map holds. Without a hit the text keeps the last
// rebuild, or stays as picked when no key was in the map.
// The two vtable names are not bound: the installed Warnings.lua's entity map
// has no unit-name keys (they are commented out), so neither can match there.
// The category is the +170h interface's slot 0, a constant per class (listed
// below from the constructors' vtable stores, docs/CC15_OBJECT_MESSAGEMANAGER.md);
// the base plane (unit+AACh) and squadron (unit+354h) read a field instead, and
// a class not listed answers false here (counted as not evaluated).
int warning_entity_category_cc15(int class_id) {
    switch (class_id) {
    case 0x07: return 1;   // MDestroyer, 006FE4D0
    case 0x08: return 13;  // MSubmarine, 00852FB0
    case 0x09: return 0;   // MMothership, 007581F0
    case 0x0A: return 4;   // MCruiser, 006FB370
    case 0x0B: return 5;   // MCargo, 006EB1D0
    case 0x0C: return 6;   // MLandingShip, 0074BBC0
    case 0x0D: return 3;   // MBattleship, 006DFDC0
    case 0x0E: return 2;   // MTorpedoBoat, 00857D60
    case 0x10: return 7;   // MPlaneBomber, 007D7790
    case 0x11: return 9;   // MPlaneTorpedoBomber, 00951CB0
    case 0x12: return 8;   // MPlaneDiveBomber, 00951BD0
    case 0x13: return 10;  // MPlaneFighter, 007DDA20
    case 0x14: return 11;  // MReconPlane, 0074E2B0
    case 0x19: return 14;  // MLandVehicle, 0074DD40
    case 0x1A: return 14;  // LandConvoy, 004F24E0
    case 0x1B: return 15;  // MLandFort, 00745A40
    case 0x1C: return 15;  // MCommandBuilding, 006F5830
    case 0x45: return 16;  // MAirfield, 006D1D20
    case 0x46: return 17;  // MShipyard, 00846B40
    default: return -1;
    }
}

bool resolve_entity_sound_keys_cc15(GameMissionFrameHost::Impl& host, std::string& text,
    std::size_t unit) {
    static constexpr const char* kRelationWords[] = {"OWN", "ENEMY", "NEUTRAL", "UNKNOWN"};
    static constexpr const char* kCategoryNames[] = {"mothership", "destroyer", "torpedoboat",
        "battleship", "cruiser", "cargo", "landingship", "levelbomber", "divebomber",
        "torpedobomber", "fighter", "reconplane", "kamikaze", "submarine", "landvehicle",
        "landfort", "airfield", "shipyard", "path", "other"};  // 00E0B590
    if (host.messages == nullptr || !host.messages->constructed() || host.units == nullptr) {
        return false;
    }
    const int class_id = host.units->unit_class_id(unit);
    const int relation = warning_relation_00803ce0(host, local_player_party(), unit);
    const int category = warning_entity_category_cc15(class_id);
    if (class_id < 0 || (relation != 3 && category < 0)) return false;
    const std::size_t pos = std::strcspn(text.c_str(), host.warning_tables().entity.prefixes.c_str());
    const std::string tail = text.substr(pos, 1);
    const std::string prefix = tail + kRelationWords[relation];
    std::vector<std::string> keys;
    if (relation != 3) keys.push_back(prefix + kCategoryNames[category]);
    if (host.units->unit_is_kind_of(unit, 6)) {
        keys.push_back(prefix + "default_ship");
    } else if (host.units->unit_is_kind_of(unit, 0x0f)) {
        keys.push_back(prefix + "default_plane");
    } else if (host.units->unit_is_kind_of(unit, 0x18)) {
        keys.push_back(prefix + "default_planesquadron");
    } else if (class_id == 0x19 || class_id == 0x1a || class_id == 0x1b || class_id == 0x36
               || class_id == 0x45 || class_id == 0x46 || class_id == 0x1c) {
        keys.push_back(prefix + "default_landobject");
    }
    const std::string picked = text;
    bool hit = false;
    for (const std::string& key : keys) {
        const auto mapped = host.warning_tables().entity.replacements.find(key);
        if (mapped == host.warning_tables().entity.replacements.end()) continue;
        text = picked.substr(0, pos) + mapped->second + picked.substr(pos + tail.size());
        if (host.messages->record_map_contains_00702300(text)) {
            hit = true;
            break;
        }
    }
    ++(hit ? host.warning_activate.sound_keys_resolved : host.warning_activate.sound_keys_unmatched);
    if (!hit && host.warning_activate.sound_keys_unmatched <= 8) {
        std::string tried;
        for (const std::string& key : keys) tried += (tried.empty() ? "" : ",") + key;
        host.log.notef("message manager sound keys unmatched: picked=\"%s\" text=\"%s\" "
            "class=%02X keys=%s (0097B5E0)", picked.c_str(), text.c_str(), class_id,
            tried.c_str());
    }
    return true;
}

// Packet cc15-wB4 step 3a: the vtable[3] activates of the warnings the host
// reports (src/cc13_seg62_seg65.cpp gives their Raw forms), all
// __fastcall(ECX = warning), RET. Common head: +0Ch = 1, +10h = [00F876A4].
// The key +4h is looked up in the manager's +108h map (008EC460; 009763E0
// has already found it through 0096DBB0) and 00979B50 picks a variant. The
// kill (+6Ch) and hit (+80h) forms then run 0097D650 on the copy for that
// entity and add no message when 00803CE0 of the local party and the entity
// is 2. 0097D650 returns at once on an empty text, a null separator set
// (manager+11Ch, the entity escape keys) or no separator in the text;
// otherwise its 0097B5E0 step tests candidate keys against the MessageManager
// map, which the host does not keep, so the text is left as picked and the
// call counted as not evaluated. Last, 00976B00 appends the clip.
void warning_activate_cc15(GameMissionFrameHost::Impl& host,
    bsp::cc13_seg49_64_78::WarningObject& warning, WarningActivate kind) {
    auto& counts = host.warning_activate;
    ++counts.activations;
    warning.state = 1;
    warning.validity_ref_time = host.world_clock;
    const auto entry = host.warning_tables().messages.find(warning.id);
    if (entry == host.warning_tables().messages.end()) {
        throw std::logic_error("an accepted warning id is missing from the +108h map");
    }
    const std::vector<std::string>& variants = entry->second;
    if (variants.size() == 1) ++counts.single_variant;
    WarningVariantBinding variant_host(host);
    const std::size_t picked = bsp::cc13_seg49_64_78::pick_message_variant_00979b50(
        host.warning_variants, variants, variant_host);
    std::string text = variants[picked];
    if (kind != WarningActivate::plain) {
        const std::uintptr_t entity = kind == WarningActivate::kill_6c
            ? static_cast<std::uintptr_t>(warning.word_6c)
            : reinterpret_cast<std::uintptr_t>(warning.entity);
        const std::string& set = host.warning_tables().entity.prefixes;
        if (text.empty() || set.empty()
            || std::strcspn(text.c_str(), set.c_str()) == text.size()) {
            ++counts.sound_keys_untouched;
        } else if (!(kMessageManagerCc15Bound && entity != 0
                       && resolve_entity_sound_keys_cc15(host, text, entity - 1u))) {
            ++counts.sound_keys_not_evaluated;
        }
        if (entity != 0 && warning_relation_00803ce0(host, local_player_party(),
                static_cast<std::size_t>(entity - 1u)) == 2) {
            ++counts.relation_blocked;
            return;
        }
    }
    WarningObjectHostBinding warning_host(host);
    bsp::cc13_seg49_64_78::warning_add_message_00976b00(warning, warning_host, text);
    ++counts.clips;
}

// Packet cc15-wB4: one 009763E0 outcome, counted under the warning's id.
void count_warning_report(GameMissionFrameHost::Impl& host, const std::string& id,
    const bsp::WarningReportResult& result) {
    auto& outcome = host.warning_report_outcomes[id];
    switch (result.outcome) {
    case bsp::WarningReportOutcome::Queued:
    case bsp::WarningReportOutcome::QueuedAndApplied: ++outcome[0]; break;
    case bsp::WarningReportOutcome::SuppressedDuplicate: ++outcome[1]; break;
    case bsp::WarningReportOutcome::RejectedUnknownId: ++outcome[2]; break;
    default: ++outcome[3]; break;
    }
}

// Packet cc15-wB4: 009813A0's default kill warning, 00981E07..009820AC, after
// the two `kill` channel passes (docs/CC13_EVENT_CHANNELS.md section 3). The
// report goes through 009763E0 into the manager's pending queue, which only
// 009763E0's own walks and 00976F10 read in this process.
void game_warning_kill_block_00981e07(GameMissionFrameHost::Impl& host, std::size_t unit) {
    auto& counts = host.kill_warning;
    ++counts.entries;
    // 00981E07..00981E0E: +104h is set only by 00979670 (Lua OverrideMessage)
    // from a listener the dispatcher ran inline; the dispatcher cleared it on
    // entry and the host runs no `kill` listener, so it is clear here.
    // 00981E3E..00981E45: +D0h.
    if (host.warning().suppressed) {
        ++counts.suppressed_d0;
        return;
    }
    // 00981E4B..00981E62: [game+18ECh] in [0, 7]; the single-player slot is 0.
    // 00981E68..00981E77: 00F876A4 above 4.0 (00CE3D34).
    if (!bsp::warning_report_clock_allows(host.world_clock)) {
        ++counts.clock;
        return;
    }
    // 00981E7D..00981E94: 0077EDF0 on manager+D4h for the victim; that map has
    // no entries in this process (as at 00977690).
    // 00981E9A..00981ECE: the victim's recon record for the local party,
    // unit+1E8h+party*34h (+8h when +10h is set, else +4h); below 2 ends the
    // block. ReconSensorPassState::level publishes the forced level the same way.
    const GameGunneryHost* gunnery = host.units->gunnery();
    if (gunnery == nullptr) {
        ++counts.recon_not_evaluated;
        return;
    }
    const int party = local_player_party();
    if (gunnery->recon_sensor_pass_state().level(party, unit)
            != bsp::ReconDetectionLevel::identified) {
        ++counts.recon_below;
        return;
    }
    KillWarningQueryBinding query(host);
    const bsp::WarningEntity victim = warning_entity_of_unit(unit);
    if (query.is_kind_5c(victim, bsp::kWarningKindPlane)) {
        // 00981EFC..00981F6E, x87: a function-local static (init bit 00F8A0D0,
        // last time 00F8A0CC, first -1e10 from 00CE4ADC) against game+64Ch, the
        // accumulator 004C6E30 writes (world.game.elapsed here). A last time
        // ahead of now resets; one within 8.0 (double 00CE3DB0) ends the block.
        if (!host.kill_plane_throttle_init) {
            host.kill_plane_throttle_init = true;
            host.kill_plane_throttle_last = -1.0e10f;
        }
        const float now = host.world.game.elapsed;  // 00981F22 FSTP to a float slot
        if (host.kill_plane_throttle_last > now) host.kill_plane_throttle_last = -1.0e10f;
        if (static_cast<double>(host.kill_plane_throttle_last)
                > static_cast<double>(now) - 8.0) {
            ++counts.plane_throttled;
            return;
        }
        host.kill_plane_throttle_last = now;
    }
    // 00981F76..00981F8A: 00803CE0(ECX = party, EDX = victim).
    const int relation = warning_relation_00803ce0(host, party, unit);
    // 00981F8F..00981FB0: +2C4h when it is a kind-2 object.
    bsp::WarningEntity killer = query.killer_2c4(victim);
    if (killer != nullptr && !query.is_kind_5c(killer, bsp::kWarningKindUnit)) killer = nullptr;
    // 00922A90: class id +C4h in {19h, 1Ah, 1Bh, 36h, 45h, 46h, 1Ch}.
    const auto land_family = [&]() {
        const int id = host.units->unit_class_id(unit);
        return id == 0x19 || id == 0x1a || id == 0x1b || id == 0x36 || id == 0x45
            || id == 0x46 || id == 0x1c;
    };
    bool report = false;
    if (relation == 0) {
        // 00981FB6..00981FF8: own ships, planes, squadrons and airfields.
        report = query.is_kind_5c(victim, bsp::kWarningKindShip)
            || query.is_kind_5c(victim, bsp::kWarningKindPlane)
            || query.is_kind_5c(victim, bsp::kWarningKindSquadron)
            || query.is_kind_5c(victim, bsp::kWarningKindAirfield);
    } else if (relation == 1) {
        // 0098201E..0098204B: an enemy plane needs a killer; an airfield
        // reports; any other land-family class does not.
        if (killer == nullptr && query.is_kind_5c(victim, bsp::kWarningKindPlane)) {
            report = false;
        } else if (query.is_kind_5c(victim, bsp::kWarningKindAirfield)) {
            report = true;
        } else {
            report = !land_family();
        }
    } else {
        // 00982071..00982074: neutral ends the block; any other code reports.
        report = relation != 2;
    }
    if (!report) {
        ++counts.filtered;
        return;
    }
    // 0098204D..00982096: operator_new(70h), 009744C0(victim), 009763E0.
    bsp::cc13_seg49_64_78::WarningObject warning;
    WarningObjectHostBinding warning_host(host);
    bsp::construct_kill_warning_009744c0(warning, warning_host, query, victim);
    bsp::WarningRecord record;
    record.message_id = warning.id;
    record.applied_at = warning.validity_ref_time;  // +10h
    record.clip_count = warning.messages.size();    // filled by vtable[3], 0097F970
    // +50h and +68h, the entities of the +3Ch and +54h handles.
    record.positional_speaker = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(warning.ambient));
    record.flat_speaker = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(warning.reporter));
    record.target_id = 0;           // a 70h kill warning has no +80h; 00976F10 tests kind 4 first
    record.kind = 3;                // vtable 00D1B2CC +10h, 00973D50
    record.priority = 0.0f;         // +18h, 0096F220
    record.pending_lifetime = 5.0f; // +1Ch, 0096F230: 00CE3850
    record.applied_lifetime = 15.0f; // +20h, 0096F240: 00CE5380
    WarningManagerBinding binding(host, warning, WarningActivate::kill_6c);
    const bsp::WarningReportResult result =
        bsp::report_warning_009763e0(host.warning(), binding, record, host.world_clock);
    ++counts.reports;
    count_warning_report(host, record.message_id, result);
    host.done("WarningManager::report_kill_warning", 0x00982096u);
}

// Packet cc15-wB4: 00977690's report, 0097777B..009777CE, past the guards the
// entry already runs: operator_new(84h), 00974150(entity, "torpedo" 00CFB414),
// 009763E0.
void game_warning_torpedo_report_0097777b(GameMissionFrameHost::Impl& host, std::size_t unit) {
    bsp::cc13_seg49_64_78::WarningObject warning;
    WarningObjectHostBinding warning_host(host);
    bsp::cc13_seg49_64_78::construct_entity_warning_with_id_00974150(warning, warning_host,
        const_cast<void*>(warning_entity_of_unit(unit)), "torpedo");
    bsp::WarningRecord record;
    record.message_id = warning.id;
    record.applied_at = warning.validity_ref_time;  // +10h
    record.clip_count = warning.messages.size();    // filled by vtable[3], 0097F490
    // +50h and +68h: an entity warning leaves the +3Ch and +54h handles empty.
    record.positional_speaker = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(warning.ambient));
    record.flat_speaker = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(warning.reporter));
    record.target_id = static_cast<std::uint32_t>(unit);  // +80h, the host's unit as 00976F10 compares it
    record.kind = 1;                // vtable 00D1B26C +10h, 00973C50
    record.priority = 0.0f;         // +18h, 0096F220
    record.pending_lifetime = 5.0f; // +1Ch, 0096F230: 00CE3850
    record.applied_lifetime = 15.0f; // +20h, 0096F240: 00CE5380
    WarningManagerBinding binding(host, warning, WarningActivate::plain);
    const bsp::WarningReportResult result =
        bsp::report_warning_009763e0(host.warning(), binding, record, host.world_clock);
    count_warning_report(host, record.message_id, result);
    host.done("WarningManager::report_torpedo_warning", 0x009763e0u);
}

// Packet cc13-powerup: 009789A0 (report_powerup_009789a0) as the power-up
// manager calls it. 009758E0 builds the 6Ch power-up warning (vtable 00D1B4B0,
// id "powerup" + name) and 009763E0 queues it; its activate 0097FF90 has the
// shape of the torpedo's 0097F490 (head, 008EC460, 00979B50, 00976B00), so it
// takes the plain arm. Record fields from 00D1B4B0: +10h 009758A0 returns 8,
// +18h/+1Ch/+20h are 0096F220/0096F230/0096F240 (0.0, 5.0, 15.0). Nothing else
// of the manager interface is reached from 009789A0.
class PowerupWarningReportBinding final : public bsp::cc13_seg49_64_78::WarningReportHost {
    using WarningObject = bsp::cc13_seg49_64_78::WarningObject;

public:
    explicit PowerupWarningReportBinding(GameMissionFrameHost::Impl& host) : host_(host) {}
    void lock_24() override {}    // the +24h critical section: one thread here
    void unlock_24() override {}
    bool suppressed_d0() override { return host_.warning().suppressed; }
    std::int32_t local_player_slot_18ec() override { return 0; }  // single player
    float clock_00f876a4() override { return host_.world_clock; }
    WarningObject* new_powerup_warning_009758e0(const std::string* name) override {
        WarningObjectHostBinding warning_host(host_);
        // [game+1FE4h] is 0 on a mission-tree launch (kAiCoordinatorCreatedAtLoad).
        bsp::construct_named_powerup_warning_009758e0(warning_, warning_host, *name, false);
        return &warning_;
    }
    void report_009763e0(WarningObject* warning) override {
        bsp::WarningRecord record;
        record.message_id = warning->id;
        record.applied_at = warning->validity_ref_time;  // +10h
        record.clip_count = warning->messages.size();    // filled by vtable[3], 0097FF90
        record.positional_speaker = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(warning->ambient));
        record.flat_speaker = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(warning->reporter));
        record.target_id = 0;
        record.kind = 8;                 // 009758A0
        record.priority = 0.0f;          // 0096F220
        record.pending_lifetime = 5.0f;  // 0096F230: 00CE3850
        record.applied_lifetime = 15.0f; // 0096F240: 00CE5380
        WarningManagerBinding binding(host_, *warning, WarningActivate::plain);
        const bsp::WarningReportResult result =
            bsp::report_warning_009763e0(host_.warning(), binding, record, host_.world_clock);
        count_warning_report(host_, record.message_id, result);
        host_.done("WarningManager::report_powerup", 0x009789a0u);
    }
    std::int32_t local_side_28() override { return unreached<std::int32_t>(); }
    bool entity_disabled_0077edf0(void*) override { return unreached<bool>(); }
    bool entity_byte(void*, std::uint32_t) override { return unreached<bool>(); }
    std::int32_t entity_side_54(void*) override { return unreached<std::int32_t>(); }
    WarningObject* new_confirm_warning_00975540(void*, std::uint32_t) override {
        return unreached<WarningObject*>();
    }
    WarningObject* new_new_unit_warning_009756a0() override { return unreached<WarningObject*>(); }
    WarningObject* new_string_list_warning_009795b0(const std::vector<std::string>&, void*,
        void*) override { return unreached<WarningObject*>(); }
    void set_byte_104() override { unreached<int>(); }
    void remove_from_list_0096c280(std::size_t, WarningObject*) override { unreached<int>(); }
    void delete_warning(WarningObject*) override { unreached<int>(); }
    std::vector<void*> waiting_payloads_e4() override { return unreached<std::vector<void*>>(); }
    void delete_waiting_payload(void*) override { unreached<int>(); }
    void clear_waiting_list_e4() override { unreached<int>(); }
    void* ingame_gui_00e198c4() override { return unreached<void*>(); }
    void voice_stop_005b83e0(void*) override { unreached<int>(); }

private:
    template <typename T> static T unreached() {
        throw std::logic_error("009789A0 does not reach this WarningReportHost call");
    }
    GameMissionFrameHost::Impl& host_;
    WarningObject warning_{};
};

// ---------------------------------------------------------------------------
// cc15-hint2 (kHintTriggerPassesCc15Bound): the reads of the seven passes and
// TriggerHint, docs/CC15_OBJECT_PROFILE_HINTS.md section 6. Every input this
// process has no owner for is a record with the labelled neutral answer.
// ---------------------------------------------------------------------------

class HintFrameBinding final : public HintFrameQueries {
public:
    explicit HintFrameBinding(GameMissionFrameHost::Impl& host) : host_(host) {}

    int effective_game_mode_004bca50() override {
        return host_.scene_contents ? host_.scene_contents->summary().effective_game_mode : 0;
    }
    // SUBSTITUTION (labelled): the narrative screen 33h is not hosted, so its
    // +C0h fade is never running.
    bool narrative_blend_005b5d50() override {
        host_.record("HintPasses::narrative_blend", 0x005b5d50u);
        return false;
    }
    bool controlled_unit_present_e188d8() override {
        return host_.units != nullptr && host_.units->controlled_bound();
    }
    bool controlled_kind_of_5c(int category) override {
        return controlled_unit_present_e188d8()
            && host_.units->unit_is_kind_of(host_.units->controlled_index(), category);
    }
    // The +170h interface's slot 0 per class, the table the MessageManager
    // binding reads; -1 for a class not listed (no hint name matches it).
    int controlled_type_170() override {
        if (!controlled_unit_present_e188d8()) return -1;
        return warning_entity_category_cc15(host_.units->unit_class_id(host_.units->controlled_index()));
    }
    int controlled_payload_007edad0() override {
        host_.record("HintPasses::plane_payload", 0x007edad0u);
        return 0;
    }
    bool controlled_ohka_144() override {
        host_.record("HintPasses::kamikaze_ohka", 0x00692f40u);
        return false;
    }
    int plane_ordnance_007bca50(bool) override {
        host_.record("HintPasses::plane_ordnance", 0x007bca50u);
        return 0;
    }
    bool plane_machinegun_c24(bool) override {
        host_.record("HintPasses::plane_machinegun", 0x006928d0u);
        return false;
    }
    // The repair task unit+A20h is not hosted: no class flag, no water or fire.
    bool class_repair_d0() override {
        host_.record("HintPasses::class_repair", 0x00692604u);
        return false;
    }
    float repair_water_34() override {
        host_.record("HintPasses::repair_water", 0x00939f80u);
        return 0.0F;
    }
    float repair_fire_38() override {
        host_.record("HintPasses::repair_fire", 0x00939f70u);
        return 0.0F;
    }
    int submarine_state_122c() override {
        host_.record("HintPasses::submarine_state", 0x0069264cu);
        return 0;
    }
    bool engine_signal_9e5() override {
        host_.record("HintPasses::engine_signal", 0x0069266au);
        return false;
    }
    void controlled_position_fc(float& x, float& y, float& z) override {
        x = y = z = 0.0F;
        if (controlled_unit_present_e188d8()) {
            host_.units->unit_position_00fc(host_.units->controlled_index(), x, y, z);
        }
    }
    // World list 28 (head +16Ch): the command buildings, +7C4h LandingRange,
    // +7A0h CaptureRange (both ints natively), +54h the side.
    void command_building_zones_16c(std::vector<bsp::CaptureZoneSample>& zones) override {
        zones.clear();
        if (host_.units == nullptr) return;
        GameUnitsHost& units = *host_.units;
        constexpr int kCommandBuildingList = 0x1C;
        const std::size_t count = units.world_list_size(kCommandBuildingList);
        for (std::size_t i = 0; i < count; ++i) {
            const std::size_t unit = units.world_list_entry(kCommandBuildingList, i);
            if (unit >= units.units().size()) continue;
            bsp::CaptureZoneSample zone{};
            units.unit_position_00fc(unit, zone.x, zone.y, zone.z);
            zone.landing_radius = static_cast<std::int32_t>(units.command_building_landing_range_07c4(unit));
            zone.capture_radius = static_cast<std::int32_t>(units.command_building_capture_range_07a0(unit));
            zone.state = units.unit_side_0054(unit);
            zones.push_back(zone);
        }
    }
    bool local_slot_flag_19() override { return false; }   // single player slot 0
    // SUBSTITUTION (labelled): the slot record's nation +28h is the local
    // party (0 allied, 1 japanese).
    int local_side_nation_28() override { return local_player_party(); }
    int local_player_slot_18ec() override { return 0; }
    int weapon_group_44() override {
        host_.record("HintPasses::weapon_group", 0x006928dbu);
        return 0;
    }
    bool support_manager_available_0066ffa0() override {
        host_.record("HintPasses::support_manager_available", 0x0066ffa0u);
        return false;
    }
    bool support_manager_free_camera_00652a30() override {
        host_.record("HintPasses::support_manager_free_camera", 0x00652a30u);
        return false;
    }
    // SUBSTITUTION (labelled): 00F8898C as 008D41C0 sets it (2); this
    // installation's options.txt has no hints token.
    std::int32_t hints_setting_00f8898c() override { return 2; }
    bool pad_prompts_00f88a30() override { return false; }
    std::int32_t lua_run_string_006b8ad0(const std::string&) override {
        host_.record("HintPasses::lua_run_string", 0x006b8ad0u);
        return 0;
    }
    bool game_cinematic_hidden_634() override { return host_.cinematic_hidden; }
    void game_set_cinematic_004cd0f0(bool hide, bool allow_simulation, bool) override {
        const bsp::CinematicModeFlags flags = bsp::cinematic_flags_004cd0f0(hide, allow_simulation);
        host_.cinematic_hidden = flags.hud_hidden;
        host_.cinematic_allow_simulation = flags.simulate_while_hidden;
        host_.frame_state.cinematic_hidden = flags.hud_hidden;
        host_.frame_state.cinematic_allow_simulation = flags.simulate_while_hidden;
        host_.done("HintsScreen::set_cinematic", 0x004cd0f0u);
    }
    void record(const char* label, std::uint32_t address) override { host_.record(label, address); }
    double mission_seconds() override { return static_cast<double>(host_.world_clock); }

private:
    GameMissionFrameHost::Impl& host_;
};

// The tracker through 004E1CA0, with this frame's reads bound to it.
GameHintTracker* hint_tracker_for_frame(GameMissionFrameHost::Impl& host) {
    GameHintTrackerSlot* slot = GameHintTrackerSlot::process_slot();
    if (slot == nullptr) return nullptr;
    if (!host.hint_frame) host.hint_frame = std::make_unique<HintFrameBinding>(host);
    GameHintTracker& tracker = slot->get_004e1ca0();
    tracker.bind_frame(host.hint_frame.get());
    return &tracker;
}

// Packet cc13-powerup: what the power-up manager (src/cc13_powerup.cpp) reaches
// in this process. Published with the class registry at the global_subsystems
// step, as 004DC6A0 publishes 00F88C30.
class PowerupManagerBinding final : public bsp::cc13_powerup::PowerupManagerHost {
    using PowerupClassConfig = bsp::PowerupClassConfig;

public:
    explicit PowerupManagerBinding(GameMissionFrameHost::Impl& host) : host_(host) {}
    float clock_00f876a4() override { return host_.world_clock; }
    std::int32_t local_player_slot_18ec() override { return 0; }  // single player
    // A mission-tree launch has no session (kAiCoordinatorCreatedAtLoad).
    std::int32_t session_word_1fe4() override { return 0; }
    // 005E2F00 forces EnablePowerups to 1 in single player (005E2FAB).
    bool powerups_enabled_00e0c978() override { return true; }
    void report_powerup_009789a0(std::int32_t slot, const bsp::cc13_powerup::PowerupItem&,
        const std::string& name) override {
        // 009789A0 passes only the name on to 009758E0; the item is unused.
        PowerupWarningReportBinding report(host_);
        bsp::cc13_seg49_64_78::report_powerup_009789a0(report, slot, &name);
    }
    // The host builds no in-game power-up manager screen ([00E198C4]+C0h), so
    // its shown byte is never set and 0061B0F0 is not reached.
    bool powerup_screen_shown_00e198c4() override { return false; }
    void powerup_screen_rebuild_lists_0061b0f0() override {
        throw std::logic_error("0061B0F0 without a power-up screen");
    }
    // The award tracker 004E1CA0 is not hosted. SUBSTITUTION (labelled): the
    // hint counts as not yet shown, and its trigger is a record.
    bool hint_has_been_shown_0068e600(const std::string& name) override {
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (GameHintTracker* tracker = hint_tracker_for_frame(host_)) {
                host_.done("PowerUps::hint_has_been_shown", 0x0068e600u);
                return tracker->has_been_shown_0068e600(name);
            }
        }
        host_.record("PowerUps::hint_has_been_shown", 0x0068e600u);
        return false;
    }
    void trigger_hint_00690fd0(const std::string& name, std::int32_t forced) override {
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (GameHintTracker* tracker = hint_tracker_for_frame(host_)) {
                tracker->trigger_hint_00690fd0(name, forced != 0);
                host_.done("PowerUps::trigger_hint", 0x00690fd0u);
                return;
            }
        }
        host_.record("PowerUps::trigger_hint", 0x00690fd0u);
    }
    // 008E34F0 acts only on a group-1 (air support) class; its vehicle-class
    // calls are not hosted and are a record.
    void resolve_air_support_classes_008e34f0(PowerupClassConfig& cls) override {
        if (cls.group_08 == 1) {
            host_.record("PowerUps::resolve_air_support_classes", 0x008e34f0u);
        }
    }
    void send_powerup_message67(std::int32_t, const bsp::cc13_powerup::PowerupItem&,
        std::int32_t) override {
        throw std::logic_error("power-up message 67 without a session");
    }
    std::vector<bsp::PowerUpSlot>& active_slot_records() override {
        return host_.world.power_ups.player_slots;
    }
    void set_active_slot_notify_gate(bool inventory_nonempty) override {
        host_.world.power_ups.player_notify_enabled = inventory_nonempty;
    }

private:
    GameMissionFrameHost::Impl& host_;
};

// Packet cc13-powerup-bind: what 008E4680 reads off a unit, over the units host.
// LABELLED: this process has no entity ids, so a unit's +174h word is its index
// plus one (unique and non-zero, which is all 008E4680's two id compares and the
// centre lookup use).
class PowerupUnitResolverBinding final : public bsp::game_powerups::PowerupUnitResolver {
public:
    explicit PowerupUnitResolverBinding(GameMissionFrameHost::Impl& host) : host_(host) {}
    bool view_unit(std::size_t unit_index, bsp::game_powerups::PowerupUnitView& out) const override {
        const GameUnitsHost* units = host_.units.get();
        if (units == nullptr || unit_index >= units->count() || unit_index >= 0xFFFFu) return false;
        out.live = units->unit_alive_and_visible(unit_index);              // 0043F080
        out.party = units->unit_side_0054(unit_index);                     // +54h
        out.id = static_cast<std::uint16_t>(unit_index + 1);               // +174h
        out.slot = ai_unit_slot_009ffd20(*units, unit_index);              // 009FFD20
        units->unit_position_00fc(unit_index, out.position[0], out.position[1],
            out.position[2]);                                              // 00427EB0
        return true;
    }
    bool view_unit_by_id(std::uint16_t id, bsp::game_powerups::PowerupUnitView& out) const override {
        return id != 0 && view_unit(static_cast<std::size_t>(id) - 1, out);
    }

private:
    GameMissionFrameHost::Impl& host_;
};

// Packet cc13-powerup: 004DC6A0's 008EDC60 + 008ECEC0, the registry part, with
// Scripts/datatables/PowerupClasses.lua read out of the VFS as the warning table
// is. Runs at every load, as 004DC6A0 does from 004DFB70; the previous frame's
// ~Impl has already destroyed the previous manager.
void load_powerup_classes_cc13(GameMissionFrameHost::Impl& host) {
    bsp::cc13_powerup::PowerupRuntime& runtime = bsp::cc13_powerup::powerup_runtime();
    std::string text;
    std::string error;
    std::string resolved = "Scripts/datatables/PowerupClasses.lua";
    if (!host.vfs.ready() || !bsp::resolve_existing_resource_00bdf4c0_fragment(
            host.vfs.context(), host.vfs.search_registrations(), resolved)) {
        error = "cannot resolve PowerupClasses.lua";
    } else {
        bsp::VfsMemoryOpen opened = bsp::open_resource_memory_00bdf310_fragment(
            host.vfs.context(), resolved, 2);
        if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
            error = opened.error.empty() ? "cannot open PowerupClasses.lua" : opened.error;
        } else {
            text.assign(reinterpret_cast<const char*>(opened.stream->data_00bef610()),
                static_cast<std::size_t>(opened.stream->size_00bef600()));
        }
    }
    host.powerup_binding = std::make_unique<PowerupManagerBinding>(host);
    if (error.empty() && bsp::cc13_powerup::load_powerup_classes_004dc6a0(runtime,
            *host.powerup_binding, text, local_player_party())) {
        host.done("PowerUps::load_classes", 0x008ecec0u);
        host.powerup_unit_resolver = std::make_unique<PowerupUnitResolverBinding>(host);
        bsp::game_powerups::publish_powerup_unit_resolver(host.powerup_unit_resolver.get());
        host.log.notef("power-up classes loaded: records=%llu texture_requests=%llu "
            "(008ECEC0, packet cc13-powerup)", runtime.counts.class_records,
            runtime.counts.texture_requests);
    } else {
        if (error.empty()) error = runtime.load_error;
        host.record("PowerUps::load_classes", 0x008ecec0u);
        host.log.notef("power-up classes not loaded: %s (packet cc13-powerup)", error.c_str());
    }
}

// Packet cc15-wB4: the table-loading fragment of 009870A0 (see
// kWarningMessageBankCc15Bound). 009870EB runs the 31-byte path through the
// script runner, which reads it out of the VFS like decals.lua.
void load_warning_message_bank_009870a0(GameMissionFrameHost::Impl& host) {
    if (host.warning_tables_loaded) return;  // 004DC6A0 runs once per process
    std::string text;
    {
        std::string resolved = bsp::kWarningDataScript;
        if (!host.vfs.ready() || !bsp::resolve_existing_resource_00bdf4c0_fragment(
                host.vfs.context(), host.vfs.search_registrations(), resolved)) {
            host.warning_tables_error = "cannot resolve the warning table";
        } else {
            bsp::VfsMemoryOpen opened = bsp::open_resource_memory_00bdf310_fragment(
                host.vfs.context(), resolved, 2);
            if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
                host.warning_tables_error = opened.error.empty()
                    ? "cannot open the warning table" : opened.error;
            } else {
                text.assign(reinterpret_cast<const char*>(opened.stream->data_00bef610()),
                    static_cast<std::size_t>(opened.stream->size_00bef600()));
            }
        }
    }
    if (host.warning_tables_error.empty()) {
        bsp::GuiLua51Host lua;
        std::string error;
        if (!lua.execute_archive(text, error)) {
            host.warning_tables_error = error;
        } else {
            const bsp::GuiLuaRef globals = lua.globals();
            const bsp::GuiLuaRef warnings = lua.get_by_name(globals, "Warnings");
            lua.release(globals);  // 009871AF
            try {
                bsp::load_warning_tables_009870a0(host.warning_tables(), lua, warnings);
                host.warning_tables_loaded = true;
            } catch (const std::exception& failure) {
                host.warning_tables_error = failure.what();
            }
            lua.release(warnings);
        }
    }
    if (host.warning_tables_loaded) {
        host.done("WarningManager::load_message_tables", 0x009870a0u);
        host.log.notef("warning table %s: messages=%zu entity=%zu playerunit_section=%zu",
            bsp::kWarningDataScript, host.warning_tables().messages.size(),
            host.warning_tables().entity.replacements.size(),
            host.warning_tables().playerunit_section.replacements.size());
    } else {
        host.record("WarningManager::load_message_tables", 0x009870a0u);
        host.log.notef("warning table %s not loaded: %s", bsp::kWarningDataScript,
            host.warning_tables_error.c_str());
    }
}


class WorldTickBinding final : public bsp::WorldTickHost {
public:
    explicit WorldTickBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}

    float world_clock() override { return owner_.world_clock; }

    void mission_events_pre_pass_00982540() override {
        if constexpr (kWarningManagerTickBound) {
            // 00982540: the `input` channel's subscriptions, each fired when an
            // action in its nested list was pressed this frame. The channel is
            // filled by 0097E360's parser from the mission's Lua event blocks,
            // which this process does not run, so it is empty here.
            owner_.done("WarningManager::pump_input_channel", 0x00982540u);
            owner_.record("WarningManager::input_channel_subscriptions", 0x0097e360u);
        } else {
            owner_.record("MissionEvents::pre_pass", 0x00982540u);
        }
    }
    void mission_events_periodic_00977990() override {
        if constexpr (kWarningManagerTickBound && kScanProximityBound) {
            ++owner_.warning_scans;
            scan_proximity_00977990_bound();
            owner_.done("WarningManager::scan_proximity", 0x00977990u);
        } else if constexpr (kWarningManagerTickBound) {
            // 00977990 walks the world lists [game+19CCh]+64h (list 6, ships)
            // and +13Ch (list 24, plane squadrons); see kScanProximityBound.
            ++owner_.warning_scans;
            owner_.record("WarningManager::scan_proximity", 0x00977990u);
        } else {
            owner_.record("MissionEvents::periodic", 0x00977990u);
        }
    }
    // 00977990 with kScanProximityBound. Units are host indices.
    void scan_proximity_00977990_bound() {
        if (owner_.units == nullptr) return;
        const GameUnitsHost& units = *owner_.units;
        // 009779A2 MOV EAX,[ECX+64h]: list 6 from its head, in 00484540's order.
        const std::size_t ships = units.world_list_size(6);
        for (std::size_t n = 0; n < ships; ++n) {
            const std::size_t ship = units.world_list_entry(6, n);
            if (ship >= units.count()) break;
            // 009779B8..009779EB: +5Ch set, +5Dh, +60h, +5Eh clear (0043F080's
            // four bytes), then +348h non-null and (+34Ch - +348h) >> 2 > 1.
            if (!units.unit_alive_and_visible(ship)) continue;
            if constexpr (kScanProximityUnitsEntriesBound) {
                // 009779E7..00977A03: +348h non-null and (+34Ch - +348h) >> 2
                // not below 2.
                if (units.unit_part_descriptor_count(ship) < 2) {
                    ++owner_.proximity_part_rejects;
                    continue;
                }
            }
            // With the switch off, SUBSTITUTION: every list-6 entity (the eight
            // ship kinds that register id 6) is taken to carry more than one part.
            ++owner_.proximity_ships_scanned;
            const int side = units.unit_side_0054(ship);
            bool hit = false;
            if constexpr (kScanProximityUnitsEntriesBound) {
                // 00977A25 MOV EBP,[ECX+13Ch]: list 24 from its head, one node
                // per plane squadron.
                const std::size_t nodes = units.world_list_size(24);
                for (std::size_t node = 0; node < nodes && !hit; ++node) {
                    const std::size_t squadron = units.world_list_entry(24, node);
                    if (squadron >= units.count()) break;
                    ++owner_.proximity_list24_nodes;
                    // 00977A36..00977A58: the node's own four live bytes.
                    if (!units.unit_alive_and_visible(squadron)) continue;
                    // 00977A5E..00977A64: the sides differ.
                    if (units.unit_side_0054(squadron) == side) continue;
                    // 00977A6A..00977A74: 00803CE0(ECX = ship side, EDX = node) == 1.
                    if (relation_00803ce0(side, squadron) != 1) continue;
                    // 00977A76 MOV ESI,[ESI+3D0h]: the flight leader.
                    const std::size_t leader = units.squadron_list_24_leader(squadron);
                    if (leader >= units.count()) {
                        // SUBSTITUTION: the image reads [+3D0h] unguarded; this
                        // process answers "none alive" and the node is skipped.
                        ++owner_.proximity_list24_no_leader;
                        continue;
                    }
                    float sx = 0.0f, sy = 0.0f, sz = 0.0f, qx = 0.0f, qy = 0.0f, qz = 0.0f;
                    units.unit_position_00fc(ship, sx, sy, sz);
                    units.unit_position_00fc(leader, qx, qy, qz);
                    // 00977A8C..00977AB8: each difference is stored to a float.
                    const float dx = sx - qx, dy = sy - qy, dz = sz - qz;
                    // 00977AC8..00977AD8: (dx*dx + dy*dy) + dz*dz on the x87,
                    // FSTP to a float.
                    float d2;
                    if constexpr (kPc24MissionLuaCc13Bound) {
                        // The main thread runs at PC24: each FMULP/FMUL ST0 and
                        // FADDP rounds to 24 bits.
                        d2 = static_cast<float>(cc13x87::x87_add24(
                            cc13x87::x87_add24(cc13x87::x87_mul24(dx, dx),
                                               cc13x87::x87_mul24(dy, dy)),
                            cc13x87::x87_mul24(dz, dz)));
                    } else {
                        const double wide = (static_cast<double>(dx) * dx
                            + static_cast<double>(dy) * dy) + static_cast<double>(dz) * dz;
                        d2 = static_cast<float>(wide);
                    }
                    // 00977AE0 FLD double [00D09FE8] / FCOMI / JA: 4.0e6 > d2.
                    if (4.0e6 > static_cast<double>(d2)) hit = true;
                }
            } else {
            // 00977A25 MOV EBP,[ECX+13Ch]: list 24, the plane squadrons.
            // SUBSTITUTION: no host registrar fills list 24, so the walk has no
            // entry (docs/CONSTRUCT_WORLD.md section 11, contract for the units
            // host). The per-candidate tests would be: live bytes, side
            // (+54h) differs, 00803CE0(side, squadron) == 1, and the squared
            // distance from ship+FCh to [squadron+3D0h]+FCh below 4.0e6.
            const std::vector<std::size_t> squadrons_list_24{};
            for (const std::size_t squadron : squadrons_list_24) {
                if (!units.unit_alive_and_visible(squadron)) continue;
                if (units.unit_side_0054(squadron) == side) continue;
                if (relation_00803ce0(side, squadron) != 1) continue;
                float sx = 0.0f, sy = 0.0f, sz = 0.0f, qx = 0.0f, qy = 0.0f, qz = 0.0f;
                units.unit_position_00fc(ship, sx, sy, sz);
                units.unit_position_00fc(squadron, qx, qy, qz);
                const float dx = sx - qx, dy = sy - qy, dz = sz - qz;
                const float d2 = dz * dz + dy * dy + dx * dx;
                if (static_cast<double>(d2) < 4.0e6) {  // 00977AE0 FLD double [00D09FE8]
                    hit = true;
                    break;
                }
            }
            }
            // 00977B04 ADD ECX,184h; CALL 00975D00: the record, created with the
            // default deadline -1.0e10 (00CE4ADC) when absent.
            auto found = owner_.proximity_records.find(ship);
            if (found == owner_.proximity_records.end()) {
                found = owner_.proximity_records.emplace(ship, GameMissionFrameHost::Impl::ProximityRecord{}).first;
                ++owner_.proximity_records_created;
            }
            auto& record = found->second;
            if (hit) {
                // 00977B1B 0096C070: the point effect from manager+194h, parented
                // to the ship's node +4A4h; render-side here.
                record.effect_live = true;
                record.deadline_04 = static_cast<float>(
                    static_cast<double>(owner_.world_clock) + 2.0);  // 00D7A308
                ++owner_.proximity_hits;
                owner_.record("WarningManager::proximity_effect_0096c070", 0x0096c070u);
            } else if (record.deadline_04 < owner_.world_clock && record.effect_live) {
                // 00977B52 00867B10, effect+9 = 1, release.
                record.effect_live = false;
                ++owner_.proximity_expiries;
                owner_.record("WarningManager::proximity_effect_stop_00867b10", 0x00867b10u);
            }
        }
    }
    // 00803CE0(ECX = side, EDX = unit): 3 when the side's detection level of
    // the unit is below 2 (00803CFE); else 0 own, 2 neutral, 1 enemy. The level
    // is the host's recon sensor pass (ReconSensorPassState::level).
    int relation_00803ce0(int side, std::size_t unit) const {
        return warning_relation_00803ce0(owner_, side, unit);
    }

    void mission_events_poll_0096d540() override {
        if constexpr (kWarningManagerTickBound) {
            // 0096D540: +19Ch, the pending prompt callback, has no writer in this
            // process, so the poll returns before the input test.
            WarningManagerBinding binding(owner_);
            bsp::poll_warning_prompt_0096d540(owner_.warning(), binding);
            owner_.done("WarningManager::poll_prompt", 0x0096d540u);
        } else {
            owner_.record("MissionEvents::poll_zones", 0x0096d540u);
        }
    }
    void mission_events_poll_00968550() override {
        if constexpr (kWarningManagerTickBound) {
            const bsp::WarningDeadlineResult r
                = bsp::update_warning_deadlines(owner_.warning(), owner_.world_clock);
            owner_.done("WarningManager::update_deadlines", 0x00968550u);
            if (r.air_raid_expired || r.collision_expired) ++owner_.warning_deadline_expiries;
            if (r.collision_expired) {
                owner_.record("WarningManager::stop_collision_sound", 0x00968550u);
            }
        } else {
            owner_.record("MissionEvents::poll_triggers", 0x00968550u);
        }
    }
    float mission_event_start_time(std::size_t) override { return 0.0f; }
    float mission_event_priority(std::size_t) override { return 0.0f; }
    bool mission_event_ready_005b71d0(std::size_t) override { return false; }
    void mission_event_destroy(std::size_t) override {}
    void mission_event_apply_00974070(std::size_t index) override {
        static_cast<void>(index);
        owner_.record("MissionEvents::apply", 0x00974070u);
        ++owner_.frames.mission_events_applied;
    }

    void bot_retarget_begin_0075b430(int) override {
        owner_.record("BotScheduler::retarget_begin", 0x0075b430u);
    }
    void bot_slot_prepare_00914390(std::size_t) override {
        owner_.record("BotScheduler::slot_prepare", 0x00914390u);
    }
    void bot_slot_dispatch_0076a9f0(std::uint32_t, std::uint32_t, std::size_t) override {
        owner_.record("BotScheduler::slot_dispatch", 0x0076a9f0u);
    }
    void bot_think_pass_a_00911e80(std::size_t) override {
        owner_.record("BotScheduler::think_a", 0x00911e80u);
    }
    void bot_think_pass_b_00912a60(std::size_t) override {
        owner_.record("BotScheduler::think_b", 0x00912a60u);
    }

    void marker_update(void*, float) override {
        owner_.record("Markers::update_marker", 0x006dc27fu);
    }
    bsp::MarkerColor marker_get_color(void*) override { return {}; }
    void marker_set_color(void*, const bsp::MarkerColor&) override {}
    void marker_set_scale_006dbac0(void*, int, int, float) override {}
    void marker_set_highlight_level(void*, float) override {}

    void entity_manager_update(float scaled_delta) override {
        static_cast<void>(scaled_delta);
        // 00481654..00481664: ECX = game+21D0h, the TrafficConfig
        // (004A43C0); [+8] is the 20h object 0049D690 builds (vtable
        // 00CE6600), and its slot 1 is 00487270, a std::list walk calling
        // each traffic group's slot 1. No unit is on that list. The groups
        // are not read (docs/CONSTRUCT_WORLD.md section 5): a named record.
        ++owner_.frames.traffic_walks;
        if constexpr (kTrafficWalkBound) {
            // 0048728C: the first node is the sentinel, so no element is called.
            owner_.done("TrafficConfig::group_walk_00487270", 0x00481664u);
        } else {
            owner_.record("TrafficConfig::group_walk_00487270", 0x00481664u);
        }
    }

    void power_ups_pre_pass_008eac80() override {
        owner_.record("PowerUps::pre_pass", 0x008eac80u);
        // Packet cc13-powerup-bind phase 2a: 008EB110's category sweep follows
        // 008EAC80 at 008EB139 with only the clock read between; the world
        // tick's own channels stay empty. SUBSTITUTION, labelled: run from this
        // hook so it keeps that place ahead of the ready pass.
        bsp::game_powerups::expire_due_modifiers_008eb110();
    }
    void power_up_expire_008e8c30(void*) override {
        owner_.record("PowerUps::expire", 0x008e8c30u);
    }
    void power_up_notify_ready_009789a0(std::size_t slot, std::size_t) override {
        // Packet cc13-powerup: past the 008EB260 size gate, 008EB27A..008EB28F
        // builds "pup_ready" (00D16228) and 008EB2ED calls 009789A0(slot, the
        // slot's last inventory item, name). Only grants fill
        // the records and the gate, so the switch off never reaches this arm.
        bsp::cc13_powerup::PowerupRuntime& runtime = bsp::cc13_powerup::powerup_runtime();
        if (bsp::cc13_powerup::kPowerupGrantBound && runtime.host != nullptr) {
            const auto& inventory = runtime.inventories.at(slot);
            runtime.host->report_powerup_009789a0(static_cast<std::int32_t>(slot),
                inventory.back(), "pup_ready");
            owner_.done("PowerUps::notify_ready", 0x009789a0u);
            return;
        }
        owner_.record("PowerUps::notify_ready", 0x009789a0u);
    }
    void power_ups_post_pass_00613760() override {
        owner_.record("PowerUps::post_pass", 0x00613760u);
    }

    void activate_entity_subtree_00922fd0(std::size_t) override {
        owner_.record("Entities::activate_subtree", 0x00922fd0u);
    }

    bool input_action_pressed(int action) override {
        // The 004c43c0 edge test. The executable's action table holds only the
        // press-start record, which the front end owns, so no in-mission action
        // is ever pressed here.
        static_cast<void>(action);
        return false;
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::FixedStepHost and bsp::GameDynamicsFrameHost, behind calls 1 and 4 of
// the in-mission subsystem tick. Every list they walk is empty here.
// ---------------------------------------------------------------------------

class FixedStepBinding final : public bsp::FixedStepHost {
public:
    explicit FixedStepBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}
    void advance_step_countdown_008079b0(float) override {
        owner_.record("Recon::service_periodic_refresh", 0x008079b0u);
    }
    void run_step_job_waves(std::uint8_t run_pass) override {
        // Milestone 2g: the three waves over the five 68h groups at 00f876c0.
        owner_.fixed_step->run_job_waves_00875cc0(run_pass);
        if constexpr (bsp::game::kLandConvoyMovementBound) {
            // Packet cc9_land_convoy_movement: the convoy element (00CEA528)
            // in waves 1 and 3.
            if (owner_.units != nullptr) {
                owner_.units->land_convoy_step_waves(bsp::kFixedSimulationStepFloat);
            }
        }
        if constexpr (kDeckTickInFixedStepBound) {
            // Packet cc9_deck_tick_in_step: 006CDC70 at the owner's motion
            // (0075828E / 006D254B), which the job waves reach before the
            // fan-out's row 12.
            if (owner_.script_orders != nullptr) {
                owner_.script_orders->run_air_ops_update_006cdc70(
                    bsp::kFixedSimulationStepFloat);
            }
        }
    }
    void run_step_subsystems(float step, bool world_active) override {
        // Milestone 2g: the sixteen per-step calls and their world gate. The
        // tail hook's own gate is tested once per step alongside them, because
        // 00875fd1 is the last thing the driver does with the same clock.
        owner_.fixed_step->run_subsystems_00875e0c(step, world_active);
        if constexpr (kLuaProximityTriggersBound) {
            // Packet cc13-fix19: row 15's pass K (009273A0) notifies the observers of
            // each entity Killed since the last drain (00696330 -> 0074FFE0). The rows
            // just ran, and this host's motion step and trigger walk (waves 1 and 2)
            // come next, so the notice lands after the previous step's walk and before
            // this one's, where the image puts it. SUBSTITUTION (labelled): drained
            // after row 16 instead of between rows 14 and 16.
            owner_.lua.target_gone_drain_009273a0();
        }
        // Milestone 2i: the ship motion virtual 00825f20. Its own caller is not
        // established - docs/SHIP_MOTION.md names three call sites (0085542f,
        // 00749b2c, 00644a38) and reads none - so the position in the frame is
        // the executable's decision. It is run here because 00825f20's order
        // ring is written for the fixed step's 0.05 s period
        // (kUnitStateMessageTickSeconds == kFixedSimulationStepFloat), which is
        // the step this call receives.
        if (owner_.units != nullptr) {
            // Milestone 2j, --trajectory-csv: the file is opened before the
            // first step so its step 0 block holds the pose the scene placed,
            // with no motion in it. A consumer that aligns on its first sample
            // would otherwise fold that step's displacement and rotation into
            // the alignment. Executable plumbing; it reads what the motion path
            // wrote and changes nothing.
            if (!owner_.trajectory_csv_path.empty() && !owner_.trajectory_csv_tried
                && owner_.units->count() > 0) {
                owner_.trajectory_csv_tried = true;
                if (owner_.trajectory_csv.open(owner_.trajectory_csv_path, owner_.log)) {
                    owner_.trajectory_csv.append_step(0, 0.0f, owner_.units->units());
                }
            }
            owner_.units->motion_step_00825f20(step);
            if constexpr (kLuaProximityTriggersBound || kLuaSquadOrdnanceDroppedTriggerBound) {
                // Packet cc13-fix10: wave 2's sub-list walk (00875B90 -> 008759B0)
                // tests the Lua proximity triggers (00750000) after wave 1's element
                // steps, and a hit's callback runs before the next step. SUBSTITUTION
                // (labelled): once per step after this host's motion step. Packet
                // cc13-fix19: the same walk runs the squad-ordnance triggers (007504F0).
                owner_.lua.run_proximity_triggers_008759b0(step);
            }
            if constexpr (bsp::kHitDeferredSameStepDrainCc13Bound) {
                // Packet cc13-hitq2: the image's row 6 (00875E44, 00926700) runs after
                // the job waves that push (the gunnery pass and the wave-2 trigger walk)
                // and drains them in the same step. This host ran its fan-out before
                // the motion step, so the same drain runs again here, after the pushes.
                // SUBSTITUTION (labelled): fan-out rows 1..5 (physics world, buoyancy,
                // spatial refresh, callbacks) ran before the pushes, not between the
                // pushes and this drain.
                if (owner_.step_subsystems != nullptr) {
                    owner_.step_subsystems->drain_deferred_entity_events_00926700();
                }
            }
            if (hull_attitude_trace_interval() != 0u
                && (owner_.units->summary().motion_steps % hull_attitude_trace_interval()) == 0u) {
                // DIAGNOSTIC, packet cc9_ship_motion_hydro (BSP_HULL_ATTITUDE_TRACE=1):
                // every ship's attitude each 10 motion steps. It reads the pose
                // the motion step published and changes nothing.
                const double deg = 180.0 / 3.14159265358979323846;
                const std::vector<GameUnitRow>& rows = owner_.units->units();
                for (std::size_t i = 0; i < rows.size(); ++i) {
                    if (!owner_.units->unit_is_kind_of(i, bsp::kUnitGunneryKindShipBase)) continue;
                    float r[3], u[3], f[3], p[3];
                    if (!owner_.units->unit_pose(i, r, u, f, p)) continue;
                    const auto clamp1 = [](float v) {
                        return v > 1.0f ? 1.0 : (v < -1.0f ? -1.0 : static_cast<double>(v));
                    };
                    // pc24-lint: allow diagnostic log line (BSP_HULL_ATTITUDE_TRACE); no image arithmetic
                    owner_.log.notef("hull attitude: t=%.2f unit=%s roll=%.2f pitch=%.2f "
                        "tilt=%.2f y=%.2f alive=%d",
                        static_cast<double>(owner_.units->summary().simulated_seconds),
                        rows[i].name.c_str(),
                        std::atan2(static_cast<double>(r[1]), static_cast<double>(u[1])) * deg,
                        std::asin(clamp1(f[1])) * deg, std::acos(clamp1(u[1])) * deg,
                        static_cast<double>(p[1]),
                        owner_.units->unit_alive_and_visible(i) ? 1 : 0);
                }
            }
            if (owner_.trajectory_csv.is_open()) {
                const GameUnitsSummary& units = owner_.units->summary();
                owner_.trajectory_csv.append_step(units.motion_steps,
                    units.simulated_seconds, owner_.units->units());
            }
        }
    }
    void run_interpolation_wave_00875670(float leftover, std::uint8_t run_pass) override {
        owner_.fixed_step->run_interpolation_wave_00875670(leftover, run_pass);
        if constexpr (bsp::game::kLandConvoyMovementBound) {
            if (owner_.units != nullptr && leftover > 0.0f) {
                owner_.units->land_convoy_interpolation_wave(leftover);
            }
        }
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

class DynamicsFrameBinding final : public bsp::GameDynamicsFrameHost {
public:
    explicit DynamicsFrameBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}
    void select_visible_model_group_00710bb0(std::uint32_t, std::size_t) override {
        owner_.record("Dynamics::select_model_group", 0x00710bb0u);
    }
    std::size_t model_group_count(std::uint32_t) override { return 0; }
    bsp::DynamicsTransform34 interpolated_body_transform_00c43ea0(
        std::uint32_t, float) override {
        owner_.record("Dynamics::interpolated_transform", 0x00c43ea0u);
        return {};
    }
    void refresh_node_world_matrix_00b6db70(std::uint32_t) override {
        owner_.record("Dynamics::refresh_node_matrix", 0x00b6db70u);
    }
    void node_world_matrix_column_y(std::uint32_t, float out[4]) override {
        for (int i = 0; i < 4; ++i) out[i] = 0.0f;
    }
    bsp::DynamicsVec3 body_linear_velocity_00c31f40(std::uint32_t) override {
        owner_.record("Dynamics::body_velocity", 0x00c31f40u);
        return {};
    }
    bsp::DynamicsSplashSettings splash_settings_00424c40() override {
        owner_.record("Dynamics::splash_settings", 0x00424c40u);
        return {};
    }
    void spawn_water_entry_effect(int, const bsp::DynamicsVec3&) override {
        owner_.record("Dynamics::spawn_water_entry_effect", 0x008685e0u);
    }
    void set_node_world_transform_vtable34(
        std::uint32_t, const bsp::DynamicsTransform34&) override {
        owner_.record("Dynamics::set_node_transform", 0x00448130u);
    }
    void unlink_and_release_node_00b6dfa0(std::uint32_t) override {
        owner_.record("Dynamics::release_node", 0x00b6dfa0u);
    }
    void queue_body_release_00c34f70(std::uint32_t) override {
        owner_.record("Dynamics::queue_body_release", 0x00c34f70u);
    }
    void set_node_visibility_00b6da70(std::uint32_t, float) override {
        owner_.record("Dynamics::set_node_visibility", 0x00b6da70u);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::InMissionTickHost, step 9 of the frame (004e5133 -> 004c40a0)
// ---------------------------------------------------------------------------

class InMissionTickBinding final : public bsp::InMissionTickHost {
public:
    explicit InMissionTickBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}

    void step_fixed_simulation_00875bb0(float scaled_delta) override {
        // The four-test gate at 00875bb1..00875c02.
        // Read the selected identity from the same pool as the entry and AI.
        // The existing +10h entry-ready projection remains outside that
        // pool's three-byte scope; it is not device ownership at +0Eh.
        bsp::FixedStepGate gate{};
        gate.game_present = true;
        bsp::ParticipantRecordId local_record;
        gate.local_player_slot_present = owner_.participants.active_record(0, local_record);
        gate.slot_ready_10h = static_cast<std::int16_t>(bsp::kLocalSlotReadyValue);
        gate.view_mode_1fe4 = owner_.scene_state.session_mode;
        gate.session_count_9c = 0;
        FixedStepBinding host(owner_);
        // The world gate of the fan-out, 00875e69..00875e7f: [[game+19CCh]+4ACh],
        // set by the construct_world load step when kWorldActiveByteBound.
        // Closed, rows 9..13 are skipped every step.
        const bool world_active = kWorldActiveByteBound && owner_.world.world_gate.enabled;
        const std::uint32_t steps = bsp::run_fixed_step_driver_00875bb0(
            owner_.fixed_clock, gate, scaled_delta, world_active, 0, host);
        owner_.fixed_steps += steps;
        // 00875fd1..00875ff7, the tail hook's four-test gate. The driver
        // reconstruction stops after the interpolation wave and carries no host
        // method for it, so the executable runs it here, at the driver's own
        // tail position, and only when the driver's gate let the body run.
        if (bsp::fixed_step_gate_open(gate)) {
            owner_.fixed_step->run_tail_00875fd1();
        }
        owner_.done("InMissionTick::fixed_step_driver", 0x00875bb0u);
    }

    void build_local_player_unit_lists_004c3cb0() override {
        // Milestone 2i: milestone 2f ran the guard and recorded the body,
        // because the eight list heads are filled from a registry no unit had
        // reached. The world host now runs the whole routine: the guard, the
        // clear, the three walks and the merge tail.
        if (owner_.world_host != nullptr) {
            owner_.world_host->build_local_player_unit_lists_004c3cb0();
            owner_.unit_lists_built = true;
            return;
        }
        bsp::UnitListsGate gate{};
        gate.already_built = owner_.unit_lists_built;
        gate.slot_index = 0;
        const bool ran = bsp::local_player_unit_lists_run_004c3cb0(gate);
        owner_.unit_lists_built = gate.already_built;
        owner_.done("InMissionTick::unit_lists_guard", 0x004c3cb0u);
        // The guard is the whole of this routine that is a rule; the eight list
        // heads it fills are the world's.
        if (ran) owner_.record("InMissionTick::build_unit_lists", 0x004bfdf0u);
    }

    void update_world_entities_00904bf0(float scaled_delta) override {
        // Milestone 2i: 004c40ce, the walk over [[world+4]] with the gate at
        // entity+5Ch and the sibling link at entity+38h, then 00904600.
        if (owner_.world_host != nullptr) {
            owner_.world_host->run_world_entity_update_00904bf0(scaled_delta);
            return;
        }
        owner_.record("InMissionTick::update_world_entities", 0x00904bf0u);
    }

    void update_game_dynamics_00447b80(float scaled_delta) override {
        DynamicsFrameBinding host(owner_);
        const float alpha = bsp::dynamics_interpolation_alpha(
            owner_.fixed_clock.interpolation_left, bsp::kFixedSimulationStepFloat);
        static_cast<void>(bsp::tick_game_dynamics_frame_00447b80(
            owner_.dynamics, scaled_delta, alpha, host));
        owner_.done("InMissionTick::update_game_dynamics", 0x00447b80u);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::MenuRequestServicer for 006840f0
// ---------------------------------------------------------------------------

class MenuServicerBinding final : public bsp::MenuRequestServicer {
public:
    explicit MenuServicerBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}
    void service_menu_channel(std::size_t, std::int32_t, std::int32_t) override {
        owner_.record("MenuRequests::service_channel", 0x006840f0u);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::GameFrameControlHost, for the mission completion poll 004d7ea0 only.
// Every method records when it is reached; the poll reaches exactly one.
// ---------------------------------------------------------------------------

class FrameControlBinding final : public bsp::GameFrameControlHost {
public:
    explicit FrameControlBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}

    void pre_tick_console_commands() override {
        owner_.record("FrameControl::pre_tick_console", 0x004e2200u);
    }
    void copy_local_player_slots(bsp::LocalPlayerSlot (&slots)[bsp::kLocalPlayerSlotCount])
        override {
        for (std::size_t i = 0; i < bsp::kLocalPlayerSlotCount; ++i) {
            bsp::ParticipantRecordId record;
            bsp::ParticipantRecordBytes bytes;
            if (!owner_.participants.active_record(i, record)
                || !owner_.participants.read_record_bytes(record, bytes)
                || !bytes.claimed_08.available || !bytes.ai_held_09.available) {
                throw std::logic_error("Frame participant +8/+9 view is unavailable");
            }
            slots[i] = {true, bytes.claimed_08.value != 0, bytes.ai_held_09.value != 0};
        }
    }
    float mission_time_filter_007713a0(float delta) override { return delta; }
    bool max_step_clamp_disabled() override { return false; }
    bool input_action_held(int) override { return false; }
    bool input_action_pressed(int) override { return false; }

    void request_02_004d8000() override { owner_.record("Drain::request_02", 0x004d8000u); }
    void request_04_004e4000() override {
        // The front-end shell entry, which after a mission is the debrief front
        // end. 004e4000 itself is reconstructed and milestone 2c runs it from
        // the main-menu path over the menu host's screen registry; this host
        // owns none of that state, so the dispatch is recorded and the run ends
        // here rather than re-entering another owner's shell.
        owner_.record("Drain::request_04_front_end_shell", 0x004e4000u);
        owner_.result->note_front_end_entered();
    }
    void request_06_notify_00e198ac() override {
        owner_.record("Drain::request_06", 0x00e198acu);
    }
    void request_07_004bfc70() override { owner_.record("Drain::request_07", 0x004bfc70u); }
    void request_09_notify_00e198b4() override {
        owner_.record("Drain::request_09", 0x00e198b4u);
    }
    void request_0a_0b_004dfb70() override { owner_.record("Drain::request_0a", 0x004dfb70u); }
    void request_0e_004c6b00() override { owner_.record("Drain::request_0e", 0x004c6b00u); }
    void request_0f_004d7970() override {
        // BSP_Game_EndScene. Milestone 2g owns the scoring records and the
        // mission progress object, so the handler runs for real.
        owner_.result->run_end_scene_004d7970(false);
    }
    bool request_10_teardown_004e458a(bsp::GameFrameControlState& state) override {
        return owner_.result->run_teardown_arm_004e458a(state);
    }
    void request_12_resume_004cd0f0() override {
        owner_.record("Drain::request_12", 0x004cd0f0u);
    }
    void request_14_004bac20() override { owner_.record("Drain::request_14", 0x004bac20u); }
    void request_16_notify_00e198b8() override {
        owner_.record("Drain::request_16", 0x00e198b8u);
    }
    bool network_session_active() override { return false; }
    void post_drain_00a95960(float) override {
        owner_.record("Drain::post_drain", 0x00a95960u);
    }
    void update_cutscene_playback_004c6b20(float) override {
        owner_.record("FrameControl::cutscene_playback", 0x004c6b20u);
    }
    void mission_hud_update(float) override {
        owner_.record("FrameControl::mission_hud_update", 0x00f88c20u);
    }
    void accumulate_frame_statistics_0053c510() override {
        owner_.record("FrameControl::frame_statistics", 0x0053c510u);
    }
    void update_presence_context_004c0170() override {
        owner_.record("FrameControl::presence_context", 0x004c0170u);
    }
    void update_device_wait_screen_004db920() override {
        owner_.record("FrameControl::device_wait", 0x004db920u);
    }
    void set_front_end_pending_flag(bool) override {}
    bsp::MissionResult mission_result() override {
        // game+7188h. Milestone 2f had nothing here, because 004dfe83 released
        // the previous result during the load; --mission-complete-frame builds
        // one through 004cd390, and this is what the poll then reads.
        owner_.done("MissionCompletion::mission_result", 0x004d7ea0u);
        return owner_.result->result_projection();
    }
    void world_final_tick(float) override {
        // [game+19CCh] vtable +Ch, which the image fills with 00904bf0.
        owner_.record("MissionCompletion::world_final_tick", 0x00904bf0u);
    }
    void world_post_tick() override {
        if constexpr (kSunkShipFlushBound) {
            // 004D7EDE / 004D7EFC call the same 00903670.
            owner_.flush_unit_kills_00903670();
            owner_.done("MissionCompletion::world_post_tick", 0x00903670u);
        } else {
            owner_.record("MissionCompletion::world_post_tick", 0x00903670u);
        }
    }
    void show_mission_result_gui(float local_z) override {
        // 004d7f1d..004d7f42: the result's +8h float, 004f8a20 with it as the
        // movie widget's local Z, then 004f8970 registering 004f89d0.
        owner_.record("MissionCompletion::play_end_movie", 0x004d7f32u);
        owner_.record("MissionCompletion::set_movie_completion", 0x004d7f42u);
        owner_.log.notef("mission completion poll: the end movie would play at local "
            "Z %.1f (result+8h); the movie player is the front-end owner's",
            static_cast<double>(local_z));
    }
    void close_mission_result() override {
        owner_.record("MissionCompletion::close_result_gui", 0x004d7f72u);
        owner_.result->release_result_004cc510();
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::MissionDeviceWaitHost (004db920)
// ---------------------------------------------------------------------------

class DeviceWaitBinding final : public bsp::MissionDeviceWaitHost {
public:
    DeviceWaitBinding(GameMissionFrameHost::Impl& owner, bool& entered)
        : owner_(owner), entered_(entered) {}

    bool any_dynamic_device_button_down() override {
        // 00a91020 with ECX = the device table 00f8bbf4. This process has no
        // input backend, so nothing would ever report a button down and the
        // single-player arm would wait forever. The executable therefore
        // injects one device-down sample here, which the recovered latch at
        // table+DDh turns into exactly one rising edge. **This is the
        // executable's own injection, not recovered behaviour**, and it is the
        // same substitution --press-start-frame makes for the title page.
        owner_.record("DeviceWait::any_button_down", 0x00a91020u);
        if (owner_.device_edge_injected) return false;
        owner_.device_edge_injected = true;
        owner_.log.notef("device-wait edge injected: the state 0Ch arm waits on "
            "00a91020, which needs an input backend this process does not build");
        return true;
    }
    bool menu_command_screen_busy() override { return false; }  // 00425d10 +25Ch
    bool gui_root_busy() override { return false; }             // 00f8abe8 +3E8h
    void update_input_manager(float delta_seconds) override {
        InputTickBinding input(owner_);
        bsp::update_input_manager_00a92c40(owner_.input, delta_seconds, input);
        owner_.done("DeviceWait::update_input_manager", 0x00a92c40u);
    }
    void dispatch_session_event(int event_tag) override {
        static_cast<void>(event_tag);
        owner_.record("DeviceWait::dispatch_session_event", 0x0075b430u);
    }
    void enter_mission_state() override { entered_ = true; }

private:
    GameMissionFrameHost::Impl& owner_;
    bool& entered_;
};

// ---------------------------------------------------------------------------
// bsp::MissionStateEntryHost (004da6c0)
// ---------------------------------------------------------------------------

class DynamicsReleaseBinding final : public bsp::DynamicsReleaseHost {
public:
    explicit DynamicsReleaseBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}
    void release_dynamics_handle_00c34f70(std::uint32_t) override {
        owner_.record("Dynamics::release_handle", 0x00c34f70u);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

class MissionEntryBinding final : public bsp::MissionStateEntryHost {
public:
    explicit MissionEntryBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}

    void enter_scope(const char* label) override {
        static_cast<void>(label);  // 004254b0, a no-op body in this build
    }
    void release_deferred_dynamics() override {
        bsp::GameDynamicsList list{};
        DynamicsReleaseBinding release(owner_);
        owner_.entry.dynamics_released
            = bsp::release_all_dynamics_00447060(list, release);
        owner_.done("MissionEntry::release_deferred_dynamics", 0x00447060u);
    }
    void mark_local_slot_ready(std::size_t slot_index, std::uint16_t value) override {
        static_cast<void>(slot_index);
        static_cast<void>(value);
        owner_.done("MissionEntry::mark_local_slot_ready", 0x004da71eu);
    }
    void set_audio_environment_level(float level, std::uint32_t bus_mask) override {
        static_cast<void>(level);
        static_cast<void>(bus_mask);
        owner_.record("MissionEntry::set_audio_environment_level", 0x00a7a440u);
    }
    void apply_in_game_interface(bool loading) override {
        // 004c9ca0 with 0 on this path. Milestone 2j runs the arm rather than
        // recording it: its front-end frame step is the committing 00518250 the
        // load's own row is not.
        owner_.entry.interface_request = loading ? "allocate (1)" : "tear down (0)";
        if (owner_.hud != nullptr) {
            owner_.hud->apply_in_game_interface_004c9ca0(loading);
        } else {
            owner_.record("MissionEntry::apply_in_game_interface", 0x004c9ca0u);
        }
    }
    void check_multiplayer_player_count() override {
        owner_.record("MissionEntry::check_player_count", 0x004d87b0u);
    }
    std::uint32_t game_state() override { return owner_.entry_state.game_state; }
    void set_cinematic_mode(bool hide, bool allow_simulation, bool third) override {
        static_cast<void>(third);
        const bsp::CinematicModeFlags flags
            = bsp::cinematic_flags_004cd0f0(hide, allow_simulation);
        owner_.cinematic_hidden = flags.hud_hidden;
        owner_.cinematic_allow_simulation = flags.simulate_while_hidden;
        owner_.entry.cinematic_cleared = !flags.hud_hidden && !flags.simulate_while_hidden;
        owner_.done("MissionEntry::set_cinematic_mode", 0x004cd0f0u);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::MissionFrameHost, the in-mission branch of 004e4a40
// ---------------------------------------------------------------------------

class MissionFrameBinding final : public bsp::MissionFrameHost {
public:
    explicit MissionFrameBinding(GameMissionFrameHost::Impl& owner)
        : owner_(owner), world_(owner), control_(owner), servicer_(owner) {}

    // --- the mission-start one-shot --------------------------------------
    bool session_counts_mission_start_004b6260() override {
        owner_.done("MissionFrame::session_counts_mission_start", 0x004b6260u);
        // No session object, so session+F4h is null and the routine answers no.
        return bsp::session_counts_mission_004b6260(false, false);
    }
    void adjust_mission_start_counters_004bcaa0(bool add) override {
        bsp::adjust_mission_counters_004bcaa0(owner_.mission_counters, add);
        owner_.done("MissionFrame::adjust_mission_counters", 0x004bcaa0u);
    }

    // --- the warning director and the input effect sets ------------------
    void update_warning_manager_00987590(float scaled_delta) override {
        WorldTickBinding& world = world_;
        // 0098759E: the accumulator adds the director's argument [EBP+8]. The
        // reconstruction reads it from the world state, which this host never
        // writes, so it is supplied for this call alone.
        // With kScaledDeltaWriteBound the frame has already written it, and
        // it is the same value the director receives.
        const float held = owner_.world.game.scaled_delta;
        if constexpr (kWarningManagerTickBound && !kScaledDeltaWriteBound) {
            owner_.world.game.scaled_delta = scaled_delta;
        } else {
            static_cast<void>(scaled_delta);
        }
        const bsp::MissionEventTickResult result
            = bsp::update_mission_events_00987590(owner_.world, world);
        if constexpr (!kScaledDeltaWriteBound) owner_.world.game.scaled_delta = held;
        static_cast<void>(result);
        owner_.done("MissionFrame::update_mission_events", 0x00987590u);
    }
    int run_input_effect_sets_004e4e6c(float expiry_reference) override {
        const int erased
            = bsp::run_input_effect_lists_004e4e6c(owner_.input, expiry_reference);
        owner_.frames.input_entries_erased += static_cast<unsigned long long>(erased);
        owner_.done("MissionFrame::run_input_effect_sets", 0x004e4e6cu);
        return erased;
    }

    // --- frame bookkeeping -----------------------------------------------
    void record_action_deadlines_004d8cd0() override {
        WorldTickBinding& world = world_;
        bsp::record_action_deadlines_004d8cd0(owner_.world, world);
        owner_.done("MissionFrame::record_action_deadlines", 0x004d8cd0u);
    }
    void multiplayer_tick_00778560(float raw_delta) override {
        static_cast<void>(raw_delta);
        owner_.record("MissionFrame::multiplayer_tick", 0x00778560u);
    }
    void begin_game_profile_block() override { begin_block(kGameBlockSlot, "game"); }
    void end_game_profile_block() override { end_block(kGameBlockSlot, "game"); }

    // --- the simulation gate ---------------------------------------------
    void register_game_block_label() override {
        owner_.record("MissionFrame::register_block_label", 0x0041e870u);
    }
    void update_in_mission_subsystems_004c40a0() override {
        // The fixed four-call opener of every simulated frame, reconstructed by
        // packet cc_mission_tick and merged during this packet's turn.
        InMissionTickBinding tick(owner_);
        bsp::run_in_mission_subsystem_tick_004c40a0(owner_.frame_state.scaled_delta, tick);
        owner_.done("MissionFrame::update_in_mission_subsystems", 0x004c40a0u);
        if (owner_.engine_movie_skip_pending) {
            // kEngineMovieEntryBound. LABELLED: 004C9E1D's latch (+1EE0h, no
            // checkpoint in this process) makes 004E5147 call 004CCE50 on the
            // first simulated frame, and the player skips the movie at once
            // (004C6B20 -> 0045F600). The tail of 0045F600 in its order:
            // 0046B730 (SetGameMode 8 at 0046B99B), 004518B0, luaPrecacheUnits
            // (0045F520), the class-table resets, collectgarbage, then under
            // block 4_<short name> luaStageInitMulti (0045F520) and
            // luaStageInit (0045F440). The resets and the GC stay records.
            owner_.engine_movie_skip_pending = false;
            owner_.record("EngineMovie::begin_004cce50", 0x004cce50u);
            // 0046B730's kill pass (0046B795-0046B91B): every root entity, and
            // every child of a kept root, whose class +C4h is not in the scene
            // database's +158h set (44h, 5Eh, 1Dh, 47h, 3Dh) is killed with
            // 00926D90(2) at 0046B906. CreateScript entities are class 3, so
            // every luaDelay timer luaEngineMovieInit made dies here.
            // LABELLED: the scene units' kill and re-create from the +28h
            // records (0046BA7B-0046BBCD) stays a record; on a skip at the first
            // frame nothing has moved. The ten 00874D00 calls (five CL=0 before
            // the mode-8 set, five CL=1 after) stay records too: 00874D00 does
            // not advance the mission clock, and its CL=0 think-list pass
            // (00929460) finds only the entities just killed.
            const std::size_t killed = owner_.lua.script_orders() != nullptr
                ? owner_.lua.script_orders()->kill_live_script_entities(2) : 0;
            owner_.log.notef("engine movie stop: 0046B730 killed %zu script entit%s "
                "(class 3, not in the scene database's +158h set)", killed,
                killed == 1 ? "y" : "ies");
            owner_.record("EngineMovieStop::scene_database_reset_0046b730", 0x0046b730u);
            owner_.lua.call_entry_point(bsp::kLuaPrecacheUnits, false);
            if constexpr (bsp::game::kVehicleClassPreloadCc15Bound) {
                // 0045F6C1, the first of the class-table resets.
                owner_.lua.census_vehicle_class_purge_0095ca70("0045F6C1",
                    bsp::game::kVehicleClassMovieStopPurgeCc15Bound);
            }
            owner_.record("EngineMovieStop::class_table_resets", 0x0045f6dfu);
            owner_.lua.call_entry_point(bsp::kLuaStageInitMulti, false);
            owner_.lua.call_entry_point(bsp::kLuaStageInit, true);
            owner_.log.notef("engine movie skipped: 0045F600's tail ran luaPrecacheUnits, "
                "luaStageInitMulti and luaStageInit on the first in-mission frame "
                "(kEngineMovieEntryBound)");
        }
    }
    void begin_engine_movie_004cce50() override {
        owner_.record("MissionFrame::begin_engine_movie", 0x004cce50u);
    }
    bsp::SimulationGateBranch pause_gate_branch_004e5153() override {
        bsp::PauseDecisionInputs inputs{};
        inputs.multiplayer_session = false;
        inputs.hud_hidden = owner_.cinematic_hidden;
        inputs.top_populated_level = 0;
        const bsp::SimulationGateBranch branch
            = bsp::decide_simulation_gate_branch(inputs);
        owner_.done("MissionFrame::pause_gate_branch", 0x004e5153u);
        return branch;
    }
    bool tutorial_hint_step_available() override { return false; }
    void advance_tutorial_hint_0054e440() override {
        owner_.record("MissionFrame::advance_tutorial_hint", 0x0054e440u);
    }
    void toggle_pause_menu_004db030() override {
        owner_.record("MissionFrame::toggle_pause_menu", 0x004db030u);
    }
    bool in_game_interface_active() override {
        // 00e198c4 and its +3Ch. Milestone 2h builds the manager, so the gate
        // answers from the object rather than from its absence.
        owner_.done("MissionFrame::in_game_interface_active", 0x004e524cu);
        return owner_.hud != nullptr && owner_.hud->manager_active();
    }
    void update_in_game_interface_0068c1f0() override {
        if (owner_.hud == nullptr) {
            owner_.record("MissionFrame::update_in_game_interface", 0x0068c1f0u);
            return;
        }
        // bsp::update_in_game_interface_0068c1f0, packet cc_hud_updates. Almost
        // everything it reads belongs to a screen class, to the camera at
        // game+19FCh or to the controlled unit at 00e188d8, none of which this
        // process owns, so the recovered control flow runs over records.
        owner_.hud->update_in_game_interface_0068c1f0();
        owner_.done("MissionFrame::update_in_game_interface", 0x0068c1f0u);
    }
    void update_interface_only_004c40f0() override {
        // Milestone 2h: the registry is no longer empty. The load's HUD step
        // registered the 42 in-mission screens into it, so this call services
        // the interface request Init pushed (006840f0 then 00684600 into the
        // manager's own 0068aca0) and then runs the pump 004f8830, which is
        // what enters the level-1 screens and commits their pages' visibility.
        ++owner_.frames.interface_updates;
        if (owner_.hud == nullptr) {
            owner_.record("MissionFrame::update_interface_only", 0x004c40f0u);
            return;
        }
        owner_.hud->apply_pending_interface_0068aca0();
        owner_.hud->update_interface_only_004c40f0(owner_.frame_state.raw_delta);
        owner_.done("MissionFrame::update_interface_only", 0x004c40f0u);
    }

    // --- the seven hint passes -------------------------------------------
    // kHintTriggerPassesCc15Bound: each pass runs on the tracker its own
    // 004E1CA0 call returns (src/game_hint_passes.cpp).
    void hint_tick_cooldowns_0068ec10(float raw_delta) override {
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (GameHintTracker* tracker = hint_tracker_for_frame(owner_)) {
                tracker->tick_cooldowns_0068ec10(raw_delta);
                owner_.done("MissionFrame::hint_tick_cooldowns", 0x0068ec10u);
                return;
            }
        }
        const std::size_t surviving = bsp::tick_hint_cooldowns_0068ec10(
            owner_.hint_cooldowns.empty() ? nullptr : owner_.hint_cooldowns.data(),
            owner_.hint_cooldowns.size(), raw_delta);
        owner_.hint_cooldowns.resize(surviving);
        owner_.done("MissionFrame::hint_tick_cooldowns", 0x0068ec10u);
    }
    void hint_drain_queued_00692b00() override {
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (GameHintTracker* tracker = hint_tracker_for_frame(owner_)) {
                tracker->drain_queued_00692b00();
                owner_.done("MissionFrame::hint_drain_queued", 0x00692b00u);
                return;
            }
        }
        bsp::QueuedHintDrainInputs inputs{};
        static_cast<void>(bsp::should_drain_queued_hint_00692b00(inputs));
        owner_.done("MissionFrame::hint_drain_queued", 0x00692b00u);
    }
    void hint_unit_class_00692b60() override {
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (GameHintTracker* tracker = hint_tracker_for_frame(owner_)) {
                tracker->update_unit_class_00692b60();
                owner_.done("MissionFrame::hint_unit_class", 0x00692b60u);
                return;
            }
        }
        owner_.record("MissionFrame::hint_unit_class", 0x00692b60u);
    }
    void hint_weapon_006926f0() override {
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (GameHintTracker* tracker = hint_tracker_for_frame(owner_)) {
                tracker->update_weapon_006926f0();
                owner_.done("MissionFrame::hint_weapon", 0x006926f0u);
                return;
            }
        }
        owner_.record("MissionFrame::hint_weapon", 0x006926f0u);
    }
    void hint_environment_00692580() override {
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (GameHintTracker* tracker = hint_tracker_for_frame(owner_)) {
                tracker->update_environment_00692580();
                owner_.done("MissionFrame::hint_environment", 0x00692580u);
                return;
            }
        }
        owner_.record("MissionFrame::hint_environment", 0x00692580u);
    }
    void hint_zone_first_get_00692fd0() override {
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (GameHintTracker* tracker = hint_tracker_for_frame(owner_)) {
                tracker->update_zone_00692fd0();
                owner_.done("MissionFrame::hint_zone", 0x00692fd0u);
                return;
            }
        }
        owner_.record("MissionFrame::hint_zone", 0x00692fd0u);
    }
    void hint_strategic_map_00692960() override {
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (GameHintTracker* tracker = hint_tracker_for_frame(owner_)) {
                tracker->update_strategic_map_00692960();
                owner_.done("MissionFrame::hint_strategic_map", 0x00692960u);
                return;
            }
        }
        owner_.record("MissionFrame::hint_strategic_map", 0x00692960u);
    }

    // --- the world tick ---------------------------------------------------
    void update_bot_scheduler_00914ef0(float scaled_delta) override {
        static_cast<void>(scaled_delta);
        WorldTickBinding& world = world_;
        bsp::update_bot_scheduler_00914ef0(owner_.world, world);
        owner_.done("MissionFrame::update_bot_scheduler", 0x00914ef0u);
    }
    void update_markers_006dc1a0(float scaled_delta) override {
        static_cast<void>(scaled_delta);
        WorldTickBinding& world = world_;
        bsp::update_markers_006dc1a0(owner_.world, world);
        owner_.done("MissionFrame::update_markers", 0x006dc1a0u);
    }
    void update_entity_manager_00481640(float scaled_delta) override {
        static_cast<void>(scaled_delta);
        WorldTickBinding& world = world_;
        bsp::update_entity_manager_00481640(owner_.world, world);
        owner_.done("MissionFrame::update_entity_manager", 0x00481640u);
    }
    void update_rain_descriptor_00865ab0() override {
        owner_.record("MissionFrame::update_rain_descriptor", 0x00865ab0u);
    }
    void update_ocean_00bbddd0(float) override {
        owner_.record("MissionFrame::update_ocean", 0x00bbddd0u);
    }
    void update_decals_00740e10(float scaled_delta) override {
        static_cast<void>(bsp::update_decal_manager_00740e10(owner_.world.decals,
            scaled_delta));
        owner_.done("MissionFrame::update_decals", 0x00740e10u);
    }
    void update_00f89b3c_0094c8f0(float) override {
        owner_.record("MissionFrame::update_00f89b3c", 0x0094c8f0u);
    }
    void update_power_ups_008eb110() override {
        WorldTickBinding& world = world_;
        bsp::update_power_ups_008eb110(owner_.world, world);
        owner_.done("MissionFrame::update_power_ups", 0x008eb110u);
    }
    void update_effect_manager_00867ee0(float) override {
        owner_.record("MissionFrame::update_effect_manager", 0x00867ee0u);
    }
    void flush_entity_activations_00903670() override {
        if constexpr (kSunkShipFlushBound) {
            owner_.flush_unit_kills_00903670();
        } else {
            WorldTickBinding& world = world_;
            bsp::flush_entity_activations_00903670(owner_.world, world);
        }
        owner_.done("MissionFrame::flush_entity_activations", 0x00903670u);
    }
    bool check_mission_completion_004d7ea0() override {
        FrameControlBinding& control = control_;
        const bool enqueued
            = bsp::check_mission_completion_004d7ea0(owner_.control, control);
        owner_.done("MissionFrame::check_mission_completion", 0x004d7ea0u);
        if (enqueued) {
            owner_.frames.completion_requested = true;
            // 004d7f67 latches the drain suspension with the enqueue, and the
            // only two writers that clear it are 004c7ed0, reached from the
            // completion 004d7f42 just registered, and the state 11h arm at
            // 004e506b, which this state cannot reach while 0Fh is still
            // queued. No clip is in flight here, so the executable runs the
            // registered completion itself; without it the drain would never
            // dispatch the request the poll just made.
            owner_.result->run_movie_completion_004f89d0();
        }
        return enqueued;
    }

    // --- the particle clock ----------------------------------------------
    void set_particle_clock_time_00b19a10(float time_milliseconds) override {
        bsp::set_particle_clock_time_00b19a10(owner_.particle_clock, time_milliseconds);
        owner_.done("MissionFrame::set_particle_clock_time", 0x00b19a10u);
    }

    // --- post-simulation ---------------------------------------------------
    void set_foliage_shader_time_00af0450(float) override {
        owner_.record("MissionFrame::set_foliage_shader_time", 0x00af0450u);
    }
    void build_foliage_visible_set_00af0c50() override {
        // The recovered builder takes the camera at [[game+19F0h]+A8h], which is
        // the renderer's.
        owner_.record("MissionFrame::build_foliage_visible_set", 0x00af0c50u);
    }
    void apply_gui_visibility_004c6c70() override {
        bsp::GuiVisibilityInputs inputs{};
        static_cast<void>(bsp::decide_gui_visibility_004c6c70(inputs));
        owner_.done("MissionFrame::apply_gui_visibility", 0x004c6c70u);
    }
    bool interface_manager_present() override {
        // 00e198ac, which the load released at 004dfd90.
        return !owner_.main_menu_manager_released;
    }
    void update_interface_music_00685c80(float) override {
        owner_.record("MissionFrame::update_interface_music", 0x00685c80u);
    }
    void update_multiplayer_interface_004d80d0() override {
        owner_.record("MissionFrame::update_multiplayer_interface", 0x004d80d0u);
    }
    bool service_pending_menu_requests_006840f0() override {
        MenuServicerBinding& servicer = servicer_;
        const bool serviced
            = bsp::service_pending_menu_requests_006840f0(owner_.menus, servicer);
        owner_.done("MissionFrame::service_pending_menu_requests", 0x006840f0u);
        return serviced;
    }
    void pump_peer_queues_00776230() override {
        owner_.record("MissionFrame::pump_peer_queues", 0x00776230u);
    }
    void apply_sound_requests_00941140() override {
        static_cast<void>(bsp::apply_sound_request_00941140(owner_.sound_requests, false));
        owner_.done("MissionFrame::apply_sound_requests", 0x00941140u);
    }
    void update_front_end_screens_004d8620() override {
        owner_.record("MissionFrame::update_front_end_screens", 0x004d8620u);
    }

    // --- render ------------------------------------------------------------
    void begin_render_profile_block() override { begin_block(kRenderBlockSlot, "render"); }
    void render_004ca440() override {
        owner_.record("MissionFrame::render", 0x004ca440u);
        // Packet cc9_physics1_sea: render job B 004B4820 -> 0068A0D0 -> 0078CFF0 ->
        // 00BA0090 steps the ocean after this frame's fixed steps; the time is
        // [00F876A4], the mission clock. A no-op while kWaveFieldLiveAmplitudeBound
        // is off.
        bsp::ocean_live_field_render_job_0068a0d0(owner_.fixed_clock.simulation_clock);
    }
    void end_render_profile_block() override { end_block(kRenderBlockSlot, "render"); }
    void finish_render_frame_004ca1f0() override {
        owner_.record("MissionFrame::finish_render_frame", 0x004ca1f0u);
    }

    // --- tail --------------------------------------------------------------
    bool frame_metrics_enabled() override { return false; }  // 00e1aed4
    void submit_frame_metrics_00757ce0(float) override {
        owner_.record("MissionFrame::submit_frame_metrics", 0x00757ce0u);
    }

private:
    void begin_block(int slot, const char* label) {
        static_cast<void>(label);
        if (owner_.profiler == nullptr) {
            owner_.record("MissionFrame::begin_profile_block", 0x00be3640u);
            return;
        }
        bsp::profiler_begin_frame_slot_00be3640(owner_.profiler->counters(), slot,
            *owner_.profiler);
        owner_.done("MissionFrame::begin_profile_block", 0x00be3640u);
    }
    void end_block(int slot, const char* label) {
        static_cast<void>(label);
        if (owner_.profiler == nullptr) {
            owner_.record("MissionFrame::end_profile_block", 0x00be3660u);
            return;
        }
        bsp::profiler_end_frame_slot_00be3660(owner_.profiler->counters(), slot,
            *owner_.profiler);
        owner_.done("MissionFrame::end_profile_block", 0x00be3660u);
    }

    GameMissionFrameHost::Impl& owner_;
    WorldTickBinding world_;
    FrameControlBinding control_;
    MenuServicerBinding servicer_;
};

// Packet cc_lua_find_entity. Not a native structure: this is the executable's
// carrier for the two facts a non-unit scene entity contributes, the `thisTable`
// slot and the authored pose `GetPosition` answers with.
struct GameSceneMarkerSeed {
    int id{0};
    std::string name;
    std::string class_name;
    int class_id{-1};
    bool findable{false};
    std::uint32_t attach{0};
    float position[3]{0.0f, 0.0f, 0.0f};
    float world[16]{};  // packet cc9_spawn_new_shipyard: the whole entity+CCh matrix
    int party{-1};  // packet cc9_init_identity_gaps: the record's authored Party
    int race{-1};   // packet cc9_scene_race_and_script_identity: the record's Race
};

// Packet cc_lua_find_entity: the scene entities that are not units but that
// BSP_SEntity_InitAll still hands to entity virtual slot 39. The class filter is
// bsp::find_scene_class_lua_identity, whose rows carry the vtable and the
// function at its +9Ch as read from the shipped image; a class with no row is
// one whose creator installs its vtable through a factory the scan does not
// follow, and those are the unit classes the unit host already owns.
// Packet cc9_mission_end. Disjoint from the unit ids (index + 1) and below the
// script entities' kScriptEntityIdBase (100000) in src/game_hosts_script_orders.cpp.
constexpr int kSceneMarkerIdBase = 50000;

std::vector<GameSceneMarkerSeed> collect_scene_markers(
    const std::vector<GameSceneEntityRecord>& entities, int first_id) {
    std::vector<GameSceneMarkerSeed> markers;
    int next = first_id;
    for (const GameSceneEntityRecord& entity : entities) {
        if (entity.name.empty()) continue;
        // `created` is false for these: their creators are records, so the
        // instance was never built. `generated` is the gate's own answer that
        // the instantiate pass takes the entity, which is what puts it on the
        // pending list InitAll walks.
        if (!entity.generated || entity.created) continue;
        const bsp::SceneClassLuaIdentityRow* row
            = bsp::find_scene_class_lua_identity(entity.class_id);
        if (row == nullptr) continue;
        GameSceneMarkerSeed seed;
        seed.id = next++;
        seed.name = entity.name;
        seed.class_name = entity.class_name;
        seed.class_id = entity.class_id;
        seed.findable = row->findable_by_name;
        seed.attach = row->attach;
        seed.party = entity.party;
        seed.race = entity.race;
        // 008A7C3C reads the world matrix translation row at entity+0FCh.
        seed.position[0] = entity.world[12];
        seed.position[1] = entity.world[13];
        seed.position[2] = entity.world[14];
        for (int i = 0; i < 16; ++i) seed.world[i] = entity.world[i];
        markers.push_back(seed);
    }
    return markers;
}

void report_scene_markers(GameHostLog& log,
    const std::vector<GameSceneMarkerSeed>& markers) {
    log.notef("scene markers: %zu non-unit scene entit%s reach entity virtual slot 39 "
        "through 00925f20's first pass at 0092604e, so each carries a `thisTable` slot "
        "and, when its world bucket is one of the fourteen 0088b1b0 walks, answers "
        "FindEntity. The creators themselves (004e99b0 for NavPoint) stay records: what "
        "the executable supplies is the slot and the authored pose, not the instance",
        markers.size(), markers.size() == 1 ? "y" : "ies");
    for (const GameSceneMarkerSeed& marker : markers) {
        log.notef("  scene marker %-20s class=%-16s id=%d findable=%d attach=%08lx "
            "pos=(%.1f,%.1f,%.1f)", marker.name.c_str(), marker.class_name.c_str(),
            marker.id, marker.findable ? 1 : 0,
            static_cast<unsigned long>(marker.attach),
            static_cast<double>(marker.position[0]),
            static_cast<double>(marker.position[1]),
            static_cast<double>(marker.position[2]));
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// GameMissionFrameHost
// ---------------------------------------------------------------------------

GameMissionFrameHost::GameMissionFrameHost(GameHostLog& log, GameVfsHost& vfs,
    GameMissionLuaHost& lua, bsp::SessionParticipantPools& participants,
    GameFrameProfiler* profiler, std::string language,
    GameHudHost* hud)
    : impl_(std::make_unique<Impl>(log, vfs, lua, participants, profiler, std::move(language))) {
    impl_->hud = hud;
    g_warning_owner = impl_.get();
}

GameMissionFrameHost::~GameMissionFrameHost() {
    if (g_warning_owner == impl_.get()) g_warning_owner = nullptr;
    if (impl_->hint_frame) {
        if (GameHintTrackerSlot* slot = GameHintTrackerSlot::process_slot()) {
            if (GameHintTracker* tracker = slot->built()) tracker->unbind_frame(impl_->hint_frame.get());
        }
    }
}

// ---------------------------------------------------------------------------
// Packet cc9_warning_manager_tick: the manager's host boundary and the two
// report entries 009DA8D0 calls.
// ---------------------------------------------------------------------------



void game_warning_report_torpedo_00977690(std::size_t unit) {
    GameMissionFrameHost::Impl* host = g_warning_owner;
    if (host == nullptr || host->units == nullptr) return;
    ++host->warning_torpedo_calls;
    // 009776E0..0097775F: +D0h clear, the local slot in [0,8) (0 here), the
    // clock past 4.0 (00CE3D34), the entity's +5Ch set and +5Dh/+60h/+5Eh
    // clear, its suppress byte (0077EDF0 on this+D4h, no entries here) clear,
    // 00965810 (the entity is the controlled unit, or a plane whose +9D4h is),
    // and not kind 8.
    const GameUnitsHost& units = *host->units;
    bsp::SceneNodeFlags flags;
    bool pending = false;
    if (host->warning().suppressed || !bsp::warning_report_clock_allows(host->world_clock)) return;
    if (!units.unit_scene_node_flags(unit, flags) || !units.unit_pending_destroy_0060(unit, pending)
        || !flags.active || flags.torn_down || pending || flags.destroyed) return;
    const bool controlled = units.controlled_bound() && units.controlled_index() == unit;
    if (!controlled) return;  // 00965810; a plane's +9D4h owner is not modelled
    if (units.unit_is_kind_of(unit, 8)) return;
    ++host->warning_torpedo_accepted;
    // 00974150 builds the "torpedo" warning and 009763E0 queues it for the
    // voice manager; with kWarningMessageBankCc15Bound both run, else neither.
    if constexpr (kWarningMessageBankCc15Bound) {
        game_warning_torpedo_report_0097777b(*host, unit);
    } else {
        host->record("WarningManager::report_torpedo_queue", 0x009763e0u);
    }
}

void game_warning_report_loss_009813a0(std::size_t unit) {
    GameMissionFrameHost::Impl* host = g_warning_owner;
    if (host == nullptr || host->units == nullptr) return;
    ++host->warning_loss_calls;
    if constexpr (kLossWarningBound) host->done("WarningManager::report_loss", 0x009813a0u);
    const GameUnitsHost& units = *host->units;
    // 009813C6..009813D2: [entity+54h] against 2, signed; a negative side passes.
    const int side = units.unit_side_0054(unit);
    if (!(side < 2)) return;
    // 009813D8..009813F4: [vt+5Ch](18h), then [vt+5Ch](6); neither answers for an
    // aircraft.
    if (!units.unit_is_kind_of(unit, 0x18) && !units.unit_is_kind_of(unit, 6)) return;
    if (side == 0) {
        ++host->warning_loss_side0;
    } else if (side == 1) {
        ++host->warning_loss_side1;
    } else {
        ++host->warning_loss_other;
    }
    if constexpr (!kLossWarningBound) {
        // 009813FA..00982110 as one record; docs/LOSS_WARNING.md has the plan.
        host->record("WarningManager::report_loss_body", 0x009813fau);
        return;
    }
    // 009813FA..00981602: the text. 009FFD20 picks the unit's player slot; with
    // a named slot the line is "[" + slot+78h + "] " + slot+58h, otherwise
    // globals.warn_uslost (side 0) or globals.warn_japanlost (side 1), then
    // globals.warn_<class>_lost from 00E0B630. SUBSTITUTION: no player slot is
    // resolved here, so the key is counted by side; the post is the record.
    if (side == 0) ++host->warning_loss_text_uslost;
    if (side == 1) ++host->warning_loss_text_japanlost;
    host->record("WarningManager::loss_text_005cf3d0", 0x00981602u);
    // 00981658..00981688: under the manager's lock, 00976F10 CancelByTarget with
    // the unit. 00976F34..00976FB9 walk the +E0h list and destroy every record
    // whose kind (vt[10h]) is 4 and whose +80h is the unit; 00976FBE..00976FE8
    // then store now + 2.0 (00F876A4 + double 00D7A308) under the unit in the
    // map at +15Ch, whose readers are unread.
    auto& pending = host->warning().pending;
    const std::uint32_t target = static_cast<std::uint32_t>(unit);
    for (auto it = pending.begin(); it != pending.end();) {
        if (it->kind == 4 && it->target_id == target) {
            it = pending.erase(it);
            ++host->warning_loss_cancelled;
        } else {
            ++it;
        }
    }
    host->warning_loss_stamp_15c[unit] = static_cast<float>(
        static_cast<double>(host->world_clock) + 2.0);
    host->done("WarningManager::cancel_by_target", 0x00976f10u);
    // 0098168D..009816C5: a ship (IsKindOf 6) loses its proximity record at
    // manager+184h: 00975D00 finds it (creating one if absent), 0096AE90 stops
    // a live effect (00867B10, effect+9 = 1) and releases it, 00975E30 erases.
    if (units.unit_is_kind_of(unit, 6)) {
        auto found = host->proximity_records.find(unit);
        if (found != host->proximity_records.end()) {
            if (found->second.effect_live) {
                host->record("WarningManager::proximity_effect_stop_00867b10", 0x00867b10u);
            }
            host->proximity_records.erase(found);
            ++host->warning_loss_proximity_erased;
        }
        host->done("WarningManager::proximity_record_erase", 0x00975e30u);
    }
    // 009816CA..009816D5: 006E6670 over unit+2B0h, not read.
    host->record("WarningManager::loss_handle_006e6670", 0x006e6670u);
    // 00981750..00981D6C: the `kill` channel twice (00980150, 0097B8C0), each
    // listener through 00887E50. Kept a record: the host fails a mission through
    // its own script-order path, which must not run twice.
    host->record("WarningManager::kill_channel_0097b8c0", 0x0097b8c0u);
    if constexpr (kWarningMessageBankCc15Bound) game_warning_kill_block_00981e07(*host, unit);
    // Teardown 0077EDF0, 006952A0, 00695870, 006E0860.
    host->record("WarningManager::loss_teardown", 0x0077edf0u);
}

void game_warning_report_hit_00989604(std::size_t victim, std::size_t shooter,
    int attacker_player_index) {
    if constexpr (kWarningMessageBankCc15Bound) {
        GameMissionFrameHost::Impl* host = g_warning_owner;
        if (host == nullptr || host->units == nullptr) return;
        const GameUnitsHost& units = *host->units;
        if (victim >= units.count()) return;
        auto& counts = host->hit_warning;
        ++counts.calls;
        KillWarningQueryBinding query(*host);
        const int party = local_player_party();
        const bsp::WarningEntity raw = warning_entity_of_unit(victim);
        // 00988546..0098859B: [esp+34h] = hitEntity->vtable[140h]() when it is a
        // kind-2 object, a plane's squadron (the plane registry, as the hit
        // listeners read it).
        std::size_t outer = victim;
        if (const bsp::PlaneSquadronHostRecord* squadron =
                bsp::plane_squadron_registry().find_by_member_unit(victim)) {
            if (squadron->squadron_unit != bsp::kPlaneSquadronNoUnit) outer = squadron->squadron_unit;
        }
        // 0098870B..00988734: [esp+17h], the shot's team +1Ch is the local slot.
        const bool local_attacker = attacker_player_index == query.local_slot_18ec();
        // 00988740..00988770: the relation to the hit entity. [esp+98h], a local
        // attacker on our own unit, only drops the 0077EDF0 tests below, which
        // have no entries here, so both arms read the same.
        const int relation = warning_relation_00803ce0(*host, party, victim);
        // 009895D8..009895E3: +104h is set only by 00979670 OverrideMessage,
        // which this process does not bind, so a listener cannot set it.
        // 00989604..0098964B / 009896CA..00989702: +D0h, slot 0, clock > 4.0.
        if (host->warning().suppressed) {
            ++counts.suppressed_d0;
            return;
        }
        if (!bsp::warning_report_clock_allows(host->world_clock)) {
            ++counts.clock;
            return;
        }
        // 00989651..0098967A: +5Ch set, +5Dh/+60h/+5Eh clear; without own fire
        // 0077EDF0 (no entries in this process, as at 00977690) as well.
        const auto live = [&](std::size_t unit) {
            bsp::SceneNodeFlags flags;
            bool pending = false;
            return units.unit_scene_node_flags(unit, flags)
                && units.unit_pending_destroy_0060(unit, pending)
                && flags.active && !flags.torn_down && !pending && !flags.destroyed;
        };
        if (!live(victim)) {
            ++counts.flags;
            return;
        }
        // 0098969C..00989742: the hit entity's recon record for the local party.
        const GameGunneryHost* gunnery = units.gunnery();
        if (gunnery == nullptr) {
            ++counts.recon_not_evaluated;
            return;
        }
        if (gunnery->recon_sensor_pass_state().level(party, victim)
                != bsp::ReconDetectionLevel::identified) {
            ++counts.recon_below;
            return;
        }
        // 00989748..00989784, x87: manager+15Ch[hit entity]. Ahead of the clock
        // and not a local attacker ends the block; the stamp becomes clock + 2.0
        // (double 00D7A308) either way, before the test's branch.
        float& stamp = host->warning_loss_stamp_15c[victim];
        const bool throttled = stamp > host->world_clock && !local_attacker;
        stamp = static_cast<float>(static_cast<double>(host->world_clock) + 2.0);
        if (throttled) {
            ++counts.stamp_throttled;
            return;
        }
        std::size_t target = victim;
        if (relation == 0) {
            if (local_attacker) {
                // 00989829..00989832: 00965810 on the hit entity.
                if (query.is_player_controlled_00965810(raw)) {
                    ++counts.player_controlled;
                    return;
                }
            } else {
                // 009897E2..009897FF: the firing unit (the image's [esp+2Ch]) is
                // on the local party.
                if (shooter < units.count() && units.unit_side_0054(shooter) == party) {
                    ++counts.own_shooter;
                    return;
                }
                // 00989805..00989822: a plane whose squadron (+9D4h) holds fewer
                // than two planes (+3CCh).
                if (query.is_kind_5c(raw, bsp::kWarningKindPlane)) {
                    const bsp::PlaneSquadronHostRecord* squadron =
                        bsp::plane_squadron_registry().find_by_member_unit(victim);
                    if (squadron == nullptr || squadron->live_count() < 2) {
                        ++counts.plane_count;
                        return;
                    }
                }
            }
            target = outer;  // 00989838 MOV EBP,[ESP+34h]
        } else if (relation == 1) {
            // 0098978A..009897C9, 00989843: only a payload hit (attacker+CCh kind
            // 2Ah) from a local-side squadron reports; the weapon is not in the
            // host's hit event.
            ++counts.enemy_payload_not_evaluated;
            return;
        } else if (relation == 2) {
            ++counts.neutral;
            return;
        }
        // 00989855..00989871: 009761D0 (the live bytes and 0077EDF0) without own
        // fire, 009681D0 (the live bytes) with it, on the reported entity.
        if (!live(target)) {
            ++counts.not_reportable;
            return;
        }
        // 00989877..0098994A: a command building of another slot (+180h) sends
        // the E6h session message first; its +78Ch/+180h are not in the host.
        if (query.is_kind_5c(warning_entity_of_unit(target), bsp::kWarningKindCommandBuilding)) {
            ++counts.command_building_not_evaluated;
            return;
        }
        // 0098994C..00989992: operator_new(88h), 00974C70(target, attacker), 009763E0.
        HitWarningQueryBinding hit_query(*host, attacker_player_index);
        const bsp::WarningEntity attacker =
            shooter < units.count() ? warning_entity_of_unit(shooter) : nullptr;
        bsp::cc13_seg49_64_78::WarningObject warning;
        WarningObjectHostBinding warning_host(*host);
        bsp::construct_hit_warning_00974c70(warning, warning_host, hit_query,
            warning_entity_of_unit(target), attacker);
        bsp::WarningRecord record;
        record.message_id = warning.id;
        record.applied_at = warning.validity_ref_time;  // +10h
        record.clip_count = warning.messages.size();    // filled by vtable[3], 0097FA90
        record.positional_speaker = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(warning.ambient));
        record.flat_speaker = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(warning.reporter));
        record.target_id = static_cast<std::uint32_t>(target);  // +80h
        record.kind = 4;                // vtable 00D1B308 +10h, 00973DC0
        record.priority = 0.0f;         // +18h, 0096F220
        record.pending_lifetime = 5.0f; // +1Ch, 0096F230: 00CE3850
        record.applied_lifetime = 15.0f; // +20h, 0096F240: 00CE5380
        WarningManagerBinding binding(*host, warning, WarningActivate::hit_80);
        const bsp::WarningReportResult result =
            bsp::report_warning_009763e0(host->warning(), binding, record, host->world_clock);
        ++counts.reports;
        count_warning_report(*host, record.message_id, result);
        host->done("WarningManager::report_hit_warning", 0x00989992u);
    } else {
        static_cast<void>(victim);
        static_cast<void>(shooter);
        static_cast<void>(attacker_player_index);
    }
}

void game_scoring_set_real_play_time_running_00905340(bool running) {
    GameMissionFrameHost::Impl* host = g_warning_owner;
    if (host == nullptr) return;
    host->world.bots.slots_active = running;  // 00905344 MOV [ECX+14A4h],AL
    host->done("MissionScoring::set_real_play_time_running", 0x00905340u);
}

void game_warning_torpedo_effect_00977820(std::size_t unit) {
    GameMissionFrameHost::Impl* host = g_warning_owner;
    if (host == nullptr) return;
    ++host->warning_effect_calls;
    // 0097784B..00977866: 00975D00's per-entity record, deadline = now + 30.0
    // (00CE7630) when it has passed.
    float& deadline = host->warning_effect_deadline[unit];
    if (!(deadline < host->world_clock)) return;
    deadline = host->world_clock + 30.0f;
    ++host->warning_effects;
    // BSP_PointEffect_CreateWithParentMatrix(this+190h, entity+4A4h, identity)
    // and effect+9 = 1: render-side.
    host->record("WarningManager::torpedo_effect_spawn", 0x00977820u);
}

void GameMissionFrameHost::bind_observer_runtime(GameObserverRuntime& runtime) {
    if (!runtime.has_live_dispatch_owner())
        throw std::logic_error("mission frame observer binding requires a live dispatch owner");
    if (impl_->observer_runtime != nullptr && impl_->observer_runtime != &runtime)
        throw std::logic_error("mission frame observer runtime cannot change while the host lives");
    if (impl_->units != nullptr) impl_->units->bind_observer_runtime(runtime);
    impl_->observer_runtime = &runtime;
}

const GameMissionLoadRunSummary& GameMissionFrameHost::load_summary() const noexcept {
    return impl_->load;
}
const GameMissionEntrySummary& GameMissionFrameHost::entry_summary() const noexcept {
    return impl_->entry;
}
const GameMissionFrameRunSummary& GameMissionFrameHost::frame_summary() const noexcept {
    return impl_->frames;
}
GameNativeMissionUnitSource GameMissionFrameHost::native_render_unit_source() const noexcept {
    return {impl_->units.get(), &impl_->lua,
        impl_->scene_contents ? &impl_->scene_contents->root_properties() : nullptr};
}

const GameSceneContentsSummary* GameMissionFrameHost::scene_contents_summary()
    const noexcept {
    return impl_->scene_contents ? &impl_->scene_contents->summary() : nullptr;
}

void GameMissionFrameHost::run_scene_load_004dfb70(const std::string& scene_path,
    const std::string& script_name, const std::string& locale_tables,
    std::int32_t mission_id, const bsp::SceneRecord* scene_record,
    const std::int32_t* participant_count) {
    Impl& host = *impl_;
    if (scene_record && !participant_count) {
        throw std::logic_error("Scene participant count is unavailable from its header properties");
    }
    const std::int32_t scene_slot_count = participant_count ? *participant_count : 0;
    if (scene_record && scene_slot_count > static_cast<std::int32_t>(bsp::kSceneSlotRecordCount)) {
        throw std::invalid_argument("Scene participant count exceeds the native eight-record pool");
    }
    host.load.ran = true;
    host.load.short_name = bsp::derive_scene_short_name(scene_path);
    host.load.script_name = script_name;
    host.load.mission_id = mission_id;

    host.scene_state.state = bsp::kMissionSceneLoadRequest;
    host.scene_state.scene_path = scene_path;
    host.scene_state.script_name = script_name;
    host.scene_state.locale_table_list = locale_tables;
    host.scene_state.mission_id = mission_id;
    host.scene_state.have_scene_record = scene_record != nullptr;
    host.load.participant_scene_present = scene_record != nullptr;
    host.load.participant_count_available = participant_count != nullptr;
    host.load.participant_scene_slots = scene_slot_count;

    // 004e087b picks the script slot; single player always resolves to 8, and
    // 004e0a50 reads the raw slot to choose the arm.
    host.load.script_slot = bsp::normalise_mission_script_slot(
        host.scene_state.script_slot, host.scene_state.script_slot_forced,
        host.scene_state.session_mode);
    host.load.engine_movie_arm = bsp::mission_uses_engine_movie(host.scene_state.script_slot);

    // The five numbered VFS file blocks the load opens around its phases.
    const bsp::MissionLoadPhase phases[] = {
        bsp::MissionLoadPhase::WorldConstruction, bsp::MissionLoadPhase::MissionScript,
        bsp::MissionLoadPhase::StageInit, bsp::MissionLoadPhase::EngineMovie,
        bsp::MissionLoadPhase::RendererHandoff,
    };
    for (const bsp::MissionLoadPhase phase : phases) {
        host.load.blocks.push_back(
            bsp::mission_load_phase_block_name(phase, host.load.short_name));
    }

    host.log.notef("mission load resumes past the renderer owners: scene=%s short=%s "
        "script=%s slot=%d engine_movie_arm=%d", scene_path.c_str(),
        host.load.short_name.c_str(), script_name.c_str(), host.load.script_slot,
        host.load.engine_movie_arm ? 1 : 0);

    std::size_t count = 0;
    const bsp::MissionLoadHostStep* steps = bsp::mission_load_host_steps(count);
    for (std::size_t i = 0; i < count; ++i) {
        const bsp::MissionLoadHostStep& step = steps[i];
        if (std::strcmp(step.host, "MissionSceneLoadHost") != 0) continue;
        ++host.load.steps;
        const std::string method(step.method);
        char address[16];
        format_address(step.address, address);
        char label[128];
        std::snprintf(label, sizeof(label), "MissionLoad::%s", step.method);

        // The arms the driver never takes on a single-player load. They are
        // reported as skipped rather than as performed or as missing.
        // `assign_party_player_slots` stays here on the evidence of
        // docs/MISSION_LOAD_HOSTS.md: 004e044d gates 004c3840 on
        // game+1FE4h == 1 and this session's field is 0, so the party rule never
        // runs on a local load, whatever its reconstruction can do.
        if (method == "detach_network_menu_manager"
            || method == "assign_party_player_slots" || method == "activate_slot"
            || method == "dispatch_session_ready_event") {
            ++host.load.skipped_arms;
            continue;
        }

        // Milestone 2m. The three rows packet cc2_mission_load_hosts
        // reconstructed and renamed. Both spellings are accepted because the
        // interface's own names are being corrected on main at the same time.
        // cc15-ob4 (kSceneDatabaseCc15Bound): 004F2800's 26 class registrations
        // into [00E18680]+34h (docs/CC15_OBJECT_SCENE_DATABASE.md).
        if constexpr (kSceneDatabaseCc15Bound) {
            if (method == "resolve_named_scene_objects"
                && scene_database_register_classes_004f2800(host.log)) {
                host.done(label, step.address);
                ++host.load.concrete;
                continue;
            }
        }
        if (method == "reset_network_slots" || method == "erase_native_string_set") {
            bool single_player_reset = false;
            run_load_session_slot_reset_004dfc13(host.log,
                host.scene_state.session_mode, single_player_reset);
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "reset_objective_list" || method == "rebuild_avoid_zone_table"
            || method == "reset_avoid_zone_state") {
            run_load_avoid_zone_state_004e0754(host.log, [&host]() {
                if (!host.scene_contents || !host.ship_ai)
                    throw std::logic_error("Avoid-zone load precedes scene/AI creation");
                host.ship_ai->load_avoid_zone_geometry(*host.scene_contents, host.lua,
                    host.scene_state.script_slot,
                    host.scene_state.script_slot_forced ? 1u : 0u,
                    host.scene_state.session_mode);
                // Packet cc9_get_closest_border_zone: the same Map block and
                // 004D5EDE selection, handed to the Lua host for world+7134h.
                for (const auto& block : host.scene_contents->root_properties().blocks) {
                    if (_stricmp(block.first.c_str(), "Map") != 0) continue;
                    bsp::WorldMapSettings settings;
                    if (bsp::read_world_map_settings_004e6c00(block.second, settings)) {
                        host.lua.set_world_border_zones(bsp::select_world_map_bounds_004d5ede(
                            settings, host.scene_state.script_slot,
                            host.scene_state.script_slot_forced ? 1u : 0u,
                            host.scene_state.session_mode));
                    }
                    break;
                }
            });
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "rebuild_scripted_name_list"
            || method == "record_script_function_baseline") {
            std::vector<std::string> names;
            const std::size_t inserted = run_load_scripted_name_baseline_004d30f0(
                host.log, host.lua, names);
            host.load.scripted_names = inserted;
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }

        if (method == "set_cinematic_mode") {
            const bsp::CinematicModeFlags flags = bsp::cinematic_flags_004cd0f0(false, false);
            host.cinematic_hidden = flags.hud_hidden;
            host.cinematic_allow_simulation = flags.simulate_while_hidden;
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "reset_single_player_slots") {
            // 004DFD18 -> 004DFD57 -> 004DFD5C: reset both pools, claim the
            // first player record, then publish current[0]. Neither service
            // writes +0E. The later 004E0851 store is skipped in mode 0.
            bsp::ParticipantRecordId claimed;
            const auto result = host.participants.reset_and_claim_local_flags(
                host.scene_state.have_scene_record,
                scene_slot_count, claimed);
            if (result != bsp::ParticipantClaimResult::Claimed) {
                throw std::logic_error("Local participant reset/claim is unavailable");
            }
            host.load.participant_local_claimed = true;
            host.load.slots_reset = bsp::kSceneSlotRecordCount;
            host.log.notef("participant pools: scene_present=%d scene_slots=%d "
                "current[0]=player[%zu] remaining=mission[1..7] device_bytes=retained "
                "mode=%d", host.scene_state.have_scene_record ? 1 : 0,
                scene_slot_count, claimed.index, host.scene_state.session_mode);
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "release_main_menu_manager") {
            host.main_menu_manager_released = true;
            // Milestone 2h: 004dfd96 calls the manager's vtable slot 0, which is
            // the deleting destructor 00687300; the destroy body 00686c90 runs
            // BSP_FrontEndManager_Deactivate 00683aa0, and that routine's tail
            // publishes the empty level-4 sets. Milestone 2f nulled the global
            // and stopped there, which is why the front-end pages stayed on
            // screen through the whole mission.
            if (host.hud != nullptr) host.hud->release_main_menu_manager_00686c90();
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "release_mission_result") {
            host.control.mission_result = bsp::MissionResult{};
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "create_hud_manager") {
            // 004e0452 / 004e046c / 0068cc70. The manager and its Init are not
            // reconstructed and stay records; what the HUD host runs is the part
            // of Init that goes through recovered code, the 42 screen
            // registrations, their pages and the INTF_SCENE3D request.
            if (host.hud != nullptr) {
                host.hud->build_manager_0068a990();
                ++host.load.concrete;
            } else {
                host.record(label, step.address);
                ++host.load.records;
            }
            continue;
        }
        if (method == "load_scene_contents") {
            host.release_units();
            // 004d4df0 at 004e03e5. Milestone 2f recorded this step; it now runs
            // the reconstruction of the routine, which runs both scene-file
            // passes over the selected mission's .scn. Every host method inside
            // it that needs the renderer, the scene graph, the world, the
            // terrain or sound keeps the unimplemented policy with its own
            // address, so this step being concrete is not a claim about those.
            if (!host.scene_contents) {
                host.scene_contents = std::make_unique<GameSceneContentsHost>(host.log,
                    host.vfs);
            }
            // Milestone 2m: 0095c640's three reads of the live VehicleClass
            // table, which the recovered global-script step already loaded.
            host.scene_contents->attach_lua(&host.lua);
            // game+614h and game+61Ch are the two fields 004bca50 reads as the
            // raw game mode and its forced flag; MissionSceneLoadState names the
            // same pair script_slot / script_slot_forced, because 004e087b picks
            // the mission script slot out of the same field.
            host.scene_contents->run_load_scene_contents_004d4df0(scene_path,
                host.scene_state.scene_override, host.scene_state.script_slot,
                host.scene_state.script_slot_forced,
                host.scene_state.session_mode != 0);
            host.done(label, step.address);
            ++host.load.concrete;
            // Milestone 2i: the instances the instantiate pass created become
            // one state object each, and the chain 009037f0 allocates is built
            // over them so the world walk has something to walk. Every unit's
            // authored `Command` token is then queued, and the first created
            // instance becomes the controlled unit through 004c0890.
            host.units = std::make_unique<GameUnitsHost>(host.log, host.lua);
            // Packet cc9_squadron_pass_hooks_calls: the InitAll walk's squadron hooks.
            host.lua.attach_units_hooks(host.units.get());
            if (host.observer_runtime != nullptr)
                host.units->bind_observer_runtime(*host.observer_runtime);
            // Milestone 2j: the gameplay settings singleton's rudder curve block
            // is filled before any unit exists, because 0083b5e0 runs from the
            // settings object's own construction and every ship reads the one
            // block. This process has one Lua state, so it runs the script into
            // the mission machine and says so.
            host.units->load_gameplay_settings_0083b5e0();
            host.units->create_units(host.scene_contents->entities());
            host.world_host = std::make_unique<GameWorldHost>(host.log, *host.units);
            host.world_host->build_entity_chains_009037f0();
            if constexpr (kBotSchedulerWritersBound) {
                // 0091B2E0 (from BSP_Game_OnInit 004E3AA0, once per process in
                // the image; here at the first world build) stores the two
                // Scoring.lua intervals, then 0091C560 (from construct_world
                // 004DE610) resets the scheduler for the new mission:
                // 0091C603 +1494h = +14ACh, 0091C61C +149Ch = +14A8h,
                // 0091C610 +14A4h = 0, 0091C628 +1498h = 1, 0091C62F +14A0h = 0.
                // +14A5h and +1424h (the local slot) have no field here.
                bsp::BotSchedulerState& bots = host.world.bots;
                float update = 1.0f;
                float recalc = 5.0f;
                const bool read = host.lua.read_scoring_intervals_0091b2e0(update, recalc);
                if (read) {
                    host.done("MissionScoring::load_intervals", 0x0091b2e0u);
                } else {
                    host.record("MissionScoring::load_intervals", 0x0091b2e0u);
                }
                bots.retarget_period = update;
                bots.think_period = recalc;
                bots.think_countdown = bots.think_period;
                bots.retarget_countdown = bots.retarget_period;
                bots.slots_active = false;
                bots.enabled = true;
                bots.active_time = 0.0f;
                host.done("MissionScoring::reset_for_new_mission", 0x0091c560u);
                // 0091C63B JMP 0090FA40, the reset's tail, not read.
                host.record("MissionScoring::reset_tail_0090fa40", 0x0090fa40u);
                host.log.notef("mission scoring: Scoring.lua %s, InGameScoreUpdateTimeInterval=%.3f "
                    "ReCalcTimeInterval=%.3f; 0091C560 reset: +1498h=1 +14A4h=0",
                    read ? "read" : "not reached (defaults)", static_cast<double>(update),
                    static_cast<double>(recalc));
            }
            // Packet cc9_authored_command_order (docs/SENTITY_INIT_ATTACH_ORDER.md
            // section 21): the scene read runs its InitAll at 0046ED0F and only
            // then resolves the deferred references at 0046ED1E, which issue each
            // unit's authored `Command` (0046AAB0). ON: the host issues them after
            // its load walk, so a load squadron's pass C (007F4E0C) sees no current
            // command. OFF: before the walk (the earlier labelled order).
            if constexpr (!kAuthoredCommandsAfterLoadWalkBound) {
                host.units->issue_authored_commands();
            }
            if constexpr (kInitialControlledUnitBound) {
                // Packet cc9_initial_controlled_unit(_load): no first-created-unit
                // stand-in. Scene load step 19 picks the controlled unit after
                // the HUD is attached, below.
            } else {
                if (host.units->count() > 0) host.units->set_controlled_unit_004c0890(0);
            }
            // Milestone 2l: one `thisTable` slot per created instance, which is
            // what 00928a00 builds for an entity and what the entity tail at
            // 0089903c pushes. Without it the mission's own script resolves
            // every FindEntity to nil and its order loops run over empty tables.
            std::vector<GameSceneMarkerSeed> markers;
            {
                std::vector<GameMissionLuaHost::SceneEntity> entities;
                entities.reserve(host.units->count());
                // Packet cc9_scene_race_and_script_identity: a scene unit's Race
                // by name, from the record 00927050 read it from.
                std::map<std::string, int> record_races;
                std::size_t races_fed = 0;
                if constexpr (kSceneRaceAndScriptIdentityBound) {
                    for (const GameSceneEntityRecord& record : host.scene_contents->entities()) {
                        if (!record.name.empty() && record.race >= 0) {
                            record_races.emplace(record.name, record.race);
                        }
                    }
                }
                for (std::size_t unit = 0; unit < host.units->count(); ++unit) {
                    const GameUnitRow* row = host.units->unit_row(unit);
                    if (row == nullptr || row->name.empty()) continue;
                    entities.push_back(GameMissionLuaHost::SceneEntity{row->name,
                        static_cast<int>(unit) + 1, row->type_id, true});
                    if constexpr (kSceneRaceAndScriptIdentityBound) {
                        const auto found = record_races.find(row->name);
                        if (found != record_races.end()) {
                            entities.back().race = found->second;  // 00928FD9
                            ++races_fed;
                        }
                    }
                }
                // Packet cc_lua_find_entity: the pending-entity pass 00925F20
                // calls entity virtual slot 39 on every node at 0092604E, not
                // only on units, so a scene entity of any class whose vtable
                // carries an attach there has a `thisTable` slot too. Without
                // these rows `FindEntity("EscapePoint")` resolved the name to
                // nothing and usn_2_java's phase-2 distance test raised.
                // Packet cc9_mission_end: the markers number from a base of their
                // own. They used to take the ids right after the scene's units,
                // which are exactly the ids a SpawnNew or air-ops unit takes later
                // (unit id = index + 1). On USN04 CarrierPath1-3 and IJNRetreat
                // shared keys 22-26 with the first spawned squadron, so its attach
                // overwrote CarrierPath1's `thisTable` slot and a FindEntity on
                // the others resolved to a plane. The image numbers every entity
                // from one handle counter, so the two never meet there.
                markers = collect_scene_markers(host.scene_contents->entities(),
                    kSceneMarkerIdBase);
                for (const GameSceneMarkerSeed& marker : markers) {
                    GameMissionLuaHost::SceneEntity seed{marker.name,
                        marker.id, -1, marker.findable};
                    seed.marker_class_id = marker.class_id;
                    seed.marker_authored_party = marker.party;
                    if constexpr (kSceneRaceAndScriptIdentityBound) {
                        if (marker.race >= 0) {
                            seed.race = marker.race;  // 0092814B, 00928100's Race
                            ++races_fed;
                        }
                    }
                    entities.push_back(seed);
                }
                host.log.notef("summary SceneLoad race identity bound=%d races_fed=%zu "
                    "(00927050 +58h -> 00928F50 / 00928100, packet "
                    "cc9_scene_race_and_script_identity)",
                    kSceneRaceAndScriptIdentityBound ? 1 : 0, races_fed);
                if constexpr (kLoadTimeInitAllBound) {
                    // Packet cc9_load_time_init_all: the scene read's InitAll
                    // (0046EB4B), over the load-time instances.
                    host.lua.run_scene_load_init_all_0046eb4b(entities);
                } else {
                    host.lua.attach_scene_entities_00928a00(entities);
                }
                if (!markers.empty()) report_scene_markers(host.log, markers);
                if constexpr (bsp::game::kLandConvoyMembersBound) {
                    // Packet cc9_land_convoy_members: the convoy marker's attach
                    // 00743450, in the load walk that reaches its slot 39.
                    std::size_t convoys = 0, members = 0;
                    for (const GameSceneEntityRecord& record : host.scene_contents->entities()) {
                        if (record.class_id != 0x1a || !record.generated) continue;
                        ++convoys;
                        if constexpr (bsp::game::kVehicleClassPreloadCc15Bound) {
                            // cc15-vcp7: 00743891 resolves each member type
                            // above 0 with 00964790(type, DL=1). For a convoy
                            // created at load that is InitAll pass A, before
                            // 004E0A9B, so the miss path marks ID and Got. A
                            // held-back convoy is not generated here; it is
                            // first resolved when GenerateObject spawns it.
                            if (record.land_convoy_keys) {
                                for (const int type : record.convoy_slots) {
                                    if (type < 1) continue;                  // 0074379C
                                    host.lua.mark_scene_vehicle_class_row_00964790(type,
                                        "00743891");
                                }
                            }
                        }
                        members += host.units->build_land_convoy_roster_00743450(record);
                    }
                    host.log.notef("summary LandConvoy roster convoys=%zu members=%zu "
                        "(00743450, packet cc9_land_convoy_members)", convoys, members);
                    if constexpr (bsp::game::kLandConvoyMovementBound) {
                        // Packet cc9_land_convoy_movement: 007420B0's Path, the
                        // knots 007AF150 built at the Path's load, and 00742C10.
                        std::size_t moving = 0;
                        for (const GameSceneEntityRecord& record : host.scene_contents->entities()) {
                            if (record.class_id != 0x1a || !record.generated) continue;
                            if (host.units->bind_land_convoy_motion(
                                    record, host.scene_contents->entities())) ++moving;
                        }
                        host.log.notef("summary LandConvoy motion convoys=%zu bound=1 "
                            "(007AF150, 00743060, 00742400, packet cc9_land_convoy_movement)",
                            moving);
                    }
                }
            }
            if constexpr (kAuthoredCommandsAfterLoadWalkBound) {
                host.units->issue_authored_commands();                  // 0046ED1E
            }
            host.log.notef("summary SceneLoad authored command order after_load_walk=%d "
                "(0046ED0F then 0046ED1E, packet cc9_authored_command_order)",
                kAuthoredCommandsAfterLoadWalkBound ? 1 : 0);
            // Milestone 2m: with the slots built, the eight binding bodies
            // src/lua_binding_navigator.cpp reconstructs can run over the
            // created instances instead of being counted as records.
            host.script_orders = std::make_unique<GameScriptOrdersHost>(host.log,
                *host.units);
            for (const GameSceneMarkerSeed& marker : markers) {
                // Packet cc9_building_pad_model: the class id lets the pad model
                // adopt the LandingPoint (1Dh) markers (docs/SHIP_AI_OPEN_ITEMS.md 75).
                host.script_orders->register_scene_marker(marker.id, marker.name,
                    marker.position, marker.class_id);
                host.units->register_scene_marker_frame(marker.id, marker.world);
                if (!host.helm_orders.empty()) {
                    Impl::HelmMarker named;
                    named.name = marker.name;
                    named.position[0] = marker.position[0];
                    named.position[1] = marker.position[1];
                    named.position[2] = marker.position[2];
                    host.helm_markers.push_back(named);
                }
            }
            host.lua.attach_script_orders(host.script_orders.get());
            // Milestone 2n: the ship AI controller family at 00d21598 over the
            // same created units. 009f50e0's three gates read unit+5Ch, +5Dh and
            // +61h, and 009f3dd0 reads the director 0071be40 answers for, so the
            // host is built after the authored commands were issued.
            host.ship_ai = std::make_unique<GameShipAiHost>(host.log, *host.units);
            host.ship_ai->bind_session_participants(host.participants);
            host.ship_ai->register_units(host.lua, host.scene_state.session_mode);
            host.units->set_ship_ai(host.ship_ai.get());
            // Milestone 2m: row 16 of the fan-out walks the entity chain this
            // step created, so the subsystem host learns about it here.
            if (host.step_subsystems != nullptr) {
                host.step_subsystems->attach_units(host.units.get());
            }
            // Milestone 2k: the two HUD screens that show the world read the same
            // created units. Once 004c0890 has bound one, the interface request
            // can carry it, which is what raises the markers screen and keeps the
            // minimap: 0068aca0's 20h arm is a unit-kind classifier with a
            // payload and publishes 25h INTF_CAPTAIN's level-1 set for a ship.
            if (host.hud != nullptr) {
                host.hud->attach_world_2k(*host.units, host.lua);
                if constexpr (kInitialControlledUnitBound) {
                    // Packet cc9_initial_controlled_unit_load: scene load step 19,
                    // 004E0565..004E05B7, before the first tick. 004E059B
                    // 008073C0 on the local slot, 004E05A2 game+193Ch = 0,
                    // 004E05A9 004C3CB0, then 004E05B7 006485A0 (run by the
                    // world host right after that build publishes the lists).
                    // Should the gunnery pass or the build not publish an own
                    // triple, the request stays armed for the first tick's build.
                    if (host.world_host != nullptr) {
                        host.world_host->request_scene_load_force_select_004e05b7();
                        GameGunneryHost* gunnery = host.units->gunnery();
                        if (gunnery != nullptr && gunnery->run_recon_pass_at_scene_load_004e059b()) {
                            host.done("SceneLoad::recon_rebuild_local_slot", 0x004e059bu);
                        } else {
                            host.record("SceneLoad::recon_rebuild_local_slot", 0x004e059bu);
                        }
                        // Packet cc13-reconload: 004E0546 00807A50's publication of
                        // the three slots this pass rebuilt (kReconLoadForceRefreshCc13Bound;
                        // OFF, a dry run counts what it would publish).
                        {
                            GameMissionLuaHost::ReconPublishCounts rc;
                            if constexpr (kReconLoadForceRefreshCc13Bound) {
                                rc = host.lua.publish_recon_slot_tables_00806b10(true);
                            } else {
                                rc = host.lua.publish_recon_slot_tables_00806b10(false);
                            }
                            host.log.notef("recon load refresh scene_load_004e0546: bound=%d "
                                "slots=%d published=%d entries=%zu (00807A50)",
                                kReconLoadForceRefreshCc13Bound ? 1 : 0, rc.slots,
                                rc.published, rc.entries);
                            if (host.script_orders != nullptr) {
                                host.script_orders->note_recon_load_entries(rc.entries);
                            }
                        }
                        host.world_host->build_local_player_unit_lists_004c3cb0();
                        host.done("SceneLoad::unit_lists_004c3cb0", 0x004e05a9u);
                    }
                }
                host.hud->request_scene_interface_for_unit_004cc460();
            }
            continue;
        }
        if (method == "apply_in_game_interface") {
            // 004c9ca0(1) at 004e1873, the load's own row: the arm that puts the
            // loading element up. It exists here so the entry's 004c9ca0(0) has
            // the element the native teardown reads without a null check.
            if (host.hud != nullptr) {
                host.hud->apply_in_game_interface_004c9ca0(true);
                ++host.load.concrete;
            } else {
                host.record(label, step.address);
                ++host.load.records;
            }
            continue;
        }
        if (method == "select_front_end_layout") {
            // Milestone 2i: 004c1ac0(3,0) then 00518250(3,0). Milestone 2h
            // reported the front-end frame as still on screen during the
            // mission and left it at milestone 2b's caveat; this row is what
            // the game does about it, and the answer is that it does not
            // release them, because the call does not commit.
            if (host.hud != nullptr) {
                host.hud->select_front_end_layout_00518250();
                ++host.load.concrete;
            } else {
                host.record(label, step.address);
                ++host.load.records;
            }
            continue;
        }
        if (method == "load_scene_file") {
            // The header pass already ran against the same bytes when milestone
            // 2e built the record; the load runs it again, this time against the
            // live database, which is the scene-graph owner's. The first run is
            // reported by the mission host.
            host.record(label, step.address);
            ++host.load.records;
            continue;
        }
        if (method == "sync_lobby_settings_from_lua") {
            host.lua.publish_lobby_settings_005e2f00();
            // 004e0305 is the next thing the same pass does: it creates the
            // `thisTable` self table and clears `recon`
            // (docs/MISSION_LUA_SELF_TABLE.md). The inventory has no row of its
            // own for it, so it runs here, where the listing runs it.
            host.lua.create_self_table_004e0305();
            // Packet cc_lua_find_entity: 004E0305 nils `recon`, and on this path
            // the native gets a table back through 00806B10's 006B8190 descent.
            // That routine runs off the recon slot lists, which this process does
            // not build, so the executable installs the shell 00803A40 builds and
            // says the publication is the record.
            host.lua.install_recon_tables_00803a40();
            ++host.load.concrete;
            continue;
        }
        if (method == "register_locale_table") {
            const std::vector<std::string> names
                = bsp::split_locale_table_list(locale_tables);
            host.load.locale_tables = names.size();
            for (const std::string& name : names) {
                host.log.notef("  locale table %s requested by the scene record", name.c_str());
            }
            host.record(label, step.address);
            ++host.load.records;
            continue;
        }
        if (method == "input_update") {
            InputTickBinding input(host);
            bsp::update_input_manager_00a92c40(host.input, 0.0f, input);
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "global_subsystems") {
            // 004dc6a0 is the world owner's and needs a GlobalSubsystemContext
            // this process cannot build, so the construction itself is a
            // record. One step inside it does run: 004dc72f calls 00886900,
            // and the mission chunk depends on the globals those two folders
            // leave behind (docs/MISSION_LUA_MACHINE.md, gap 1).
            host.record(label, step.address);
            ++host.load.records;
            if constexpr (kGlobalSubsystemsCc15Bound) {
                // 004DC6A0's owners in its order; the power-up manager keeps its
                // place between panel and warnings. A new warning owner has empty
                // +108h tables, so the fragment loads them again into it.
                if (!host.global_subsystems) {
                    host.global_subsystems = std::make_unique<GameGlobalSubsystemsHost>(
                        host.log, host.vfs);
                }
                host.global_subsystems->build_traffic_and_panel();
                if constexpr (bsp::cc13_powerup::kPowerupGrantBound) {
                    load_powerup_classes_cc13(host);
                }
                host.global_subsystems->build_warnings();
                host.warning_tables_loaded = false;
                host.warning_tables_error.clear();
                if constexpr (kWarningMessageBankCc15Bound) load_warning_message_bank_009870a0(host);
                host.global_subsystems->build_weather();
                const auto& owners = host.global_subsystems->summary();
                host.log.notef("global subsystems built: traffic=%d panel=%d warnings=%d "
                    "weather=%d loads traffic=%llu dialog=%llu weather=%llu failures=%llu%s%s "
                    "(004DC6A0 without 008EDC60)",
                    host.global_subsystems->traffic() ? 1 : 0,
                    host.global_subsystems->panel() ? 1 : 0,
                    host.global_subsystems->warnings() ? 1 : 0,
                    host.global_subsystems->weather() ? 1 : 0, owners.traffic_loaded,
                    owners.panel_loaded, owners.weather_loaded, owners.load_failures,
                    owners.last_error.empty() ? "" : " last_error=", owners.last_error.c_str());
                host.log.notef("global subsystems contents: %s",
                    host.global_subsystems->describe_contents().c_str());
            } else {
                if constexpr (kWarningMessageBankCc15Bound) load_warning_message_bank_009870a0(host);
                if constexpr (bsp::cc13_powerup::kPowerupGrantBound) load_powerup_classes_cc13(host);
            }
            if (!host.lua.started()) host.lua.start_machine_00884be0();
            host.lua.run_global_script_folders_00886900();
            continue;
        }
        if (method == "run_mission_script") {
            if (!host.lua.started()) host.lua.start_machine_00884be0();
            ++host.script_reentry_depth;  // game+644h, raised across the group
            host.lua.run_mission_script_008860b0(script_name);
            ++host.load.concrete;
            continue;
        }
        if (method == "lua_call_entry_point_a") {
            // kEngineMovieEntryBound: 004D4DF0 forced game+614h to 9, and
            // 004E0A50 reads the raw mode, so the precache and stage-init arm
            // (004E0A75..004E0E03) is skipped and _b runs luaEngineMovieInit.
            if (bsp::game::kEngineMovieEntryBound && host.scene_contents
                && host.scene_contents->summary().game_mode_forced_to_9) {
                host.load.engine_movie_arm = true;
                host.engine_movie_skip_pending = true;
                host.log.notef("engine movie entry: scene contents forced mode 9, so the load "
                    "takes 004E0A50's mode-9 arm (luaEngineMovieInit only); 0045F600's tail "
                    "runs on the first in-mission frame (kEngineMovieEntryBound, player skip)");
                ++host.load.concrete;
                continue;
            }
            host.lua.call_entry_point(bsp::kLuaPrecacheUnits, false);
            if constexpr (bsp::game::kVehicleClassPreloadCc15Bound) {
                // 004E0A9B, the first of the eleven calls after luaPrecacheUnits.
                host.lua.census_vehicle_class_purge_0095ca70("004E0A9B", true);
            }
            host.lua.call_entry_point(bsp::kLuaStageInitMulti, false);
            ++host.load.concrete;
            continue;
        }
        if (method == "lua_call_entry_point_b") {
            if (host.load.engine_movie_arm) {
                host.lua.call_entry_point(bsp::kLuaEngineMovieInit, true);
            } else {
                host.lua.call_entry_point(bsp::kLuaStageInit, true);
            }
            if (host.script_reentry_depth > 0) --host.script_reentry_depth;
            ++host.load.concrete;
            continue;
        }
        if (method == "construct_world") {
            if constexpr (kWorldActiveByteBound) {
                // 004DE651..004DE696 allocate, zero and construct the world;
                // 004CB098 MOV byte ptr [ESI+4ACh],1 is the one write of the
                // byte both gates read. The rest of 004DE610 stays the record
                // below.
                host.world.world_gate.enabled = true;
                host.done("World::active_byte_004cb098", 0x004cb098u);
            }
            if constexpr (kMessageManagerCc15Bound) {
                // 004DFA1A operator new(70h), 004DFA39 00707480, 004DFA4B the store
                // to game+21DCh. 008D57A0's voice language: see the doc (seam).
                if (host.messages == nullptr) {
                    host.messages = std::make_unique<GameMessageManagerHost>(host.log, host.vfs,
                        host.language);
                }
                host.messages->construct_00707480();  // logs its own native row
            }
        }
        if (method == "load_warning_bank" && kMessageManagerCc15Bound
            && host.messages != nullptr && host.messages->constructed()) {
            // 004DFB70's spoken warnings. 004E0FAD's sweep needs [004C1E90()]+8h == 2,
            // and the host keeps no profile-hints singleton, so only the arm every
            // load reaches runs: 004E1374 picks the bank FileBlock
            // ("Warnings_<language>_<Ally|Jp>", or "Warnings_englishauthentic_..."
            // when 007F8890 owns DL_Content_0000062; neither is built here),
            // 004E17E9 calls 007065E0(1), 004E17FB deletes the block.
            MessageManagerLoadInputs inputs;
            inputs.local_side_kind = local_player_party();
            // [game+5FCh]+1064h..+1088h come from the scene tree's "unittypes"
            // (00468B70); the host's scene read does not parse it (seam).
            inputs.filters_enabled = false;
            host.messages->load_messages_007065e0(true, inputs);
            const auto& messages = host.messages->summary();
            host.log.notef("message manager loaded Messages.lua: records=%zu scripts_opened=%llu "
                "scripts_failed=%llu voice_probe_hits=%llu voice_probe_misses=%llu "
                "sound_banks_not_loaded=%llu streamed_probes_not_evaluated=%llu language=%s%s%s",
                host.messages->record_count(), messages.scripts_opened, messages.scripts_failed,
                messages.voice_probe_hits, messages.voice_probe_misses,
                messages.sound_banks_not_loaded, messages.streamed_probes_not_evaluated,
                host.language.c_str(), messages.script_error.empty() ? "" : " error=",
                messages.script_error.c_str());
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "publish_mission_id") {
            host.published_mission_id = mission_id;
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }

        host.record(label, step.address);
        ++host.load.records;
    }

    // 004e086b, immediately after 0077f5e0.
    host.scene_state.state = bsp::kGameStateSceneReady;
    host.load.state_after = bsp::kGameStateSceneReady;
    host.entry_state.game_state = bsp::kGameStateSceneReady;
    host.log.notef("mission load finished: %zu steps, %zu concrete, %zu records, "
        "%zu arms not taken; game state = 0x%02X", host.load.steps, host.load.concrete,
        host.load.records, host.load.skipped_arms, host.load.state_after);
    host.log.notef("  numbered VFS blocks: %s %s %s %s %s", host.load.blocks[0].c_str(),
        host.load.blocks[1].c_str(), host.load.blocks[2].c_str(),
        host.load.blocks[3].c_str(), host.load.blocks[4].c_str());
}

bool GameMissionFrameHost::enter_mission_state_004da6c0() {
    Impl& host = *impl_;
    if (host.load.state_after != bsp::kGameStateSceneReady) return false;

    bsp::MissionDeviceWaitInputs inputs{};
    inputs.local_view_mode = host.scene_state.session_mode;
    inputs.session_flag_29c = false;
    inputs.game_field_624 = 0;
    bsp::MissionEntryPlayerSlot current_slots[bsp::kLocalPlayerSlotCount];
    host.entry.participant_view_available = host.participants.try_entry_slots(current_slots);
    host.entry.player_count_arm = bsp::mission_entry_uses_player_count_arm(
        inputs.local_view_mode, inputs.session_flag_29c);
    if (host.entry.participant_view_available) {
        inputs.unbound_player_slots = bsp::count_unbound_player_slots(current_slots);
        host.entry.unbound_player_slots = inputs.unbound_player_slots;
    } else if (host.entry.player_count_arm && inputs.local_view_mode == 1) {
        host.log.note("state 0Ch participant +9/+0E view is unavailable for the count arm");
        return false;
    }
    host.log.notef("mission participant entry: view_available=%d unbound=%zu "
        "player_count_arm=%d mode=%d", host.entry.participant_view_available ? 1 : 0,
        host.entry.unbound_player_slots, host.entry.player_count_arm ? 1 : 0,
        inputs.local_view_mode);

    bool entered = false;
    DeviceWaitBinding device_wait(host, entered);
    host.entry.device_wait_ran = true;
    const bool wait_entered
        = bsp::run_mission_device_wait(inputs, host.device_latch, device_wait);
    host.done("MissionState::device_wait", 0x004db920u);

    if (!entered && !wait_entered) {
        host.log.notef("state 0Ch did not enter: %zu player slot(s) still unbound",
            inputs.unbound_player_slots);
        return false;
    }

    host.entry_state.game_state = bsp::kGameStateSceneReady;
    host.entry_state.local_view_mode = host.scene_state.session_mode;
    host.entry_state.local_slot_index = 0;
    // game+1FE4h and game+1EE3h, the two fields 004d7970 and the drain's 10h
    // arm read on the way out of the mission.
    host.result->set_session_mode(host.scene_state.session_mode);
    host.result->set_session_networked(host.entry_state.one_shots.session_was_networked);
    MissionEntryBinding entry(host);
    const bool ran = bsp::run_mission_state_entry(host.entry_state, host.audio, entry);
    host.done("MissionState::enter_mission_state", 0x004da6c0u);
    host.entry.entered = ran
        && host.entry_state.game_state == static_cast<std::uint32_t>(bsp::GameStateId::kInMission);
    host.entry.state = host.entry_state.game_state;

    // The frame reads what the entry left behind.
    host.frame_state.state = host.entry_state.game_state;
    host.frame_state.scaled_delta = host.entry_state.scaled_frame_delta;
    host.frame_state.cinematic_hidden = host.cinematic_hidden;
    host.frame_state.cinematic_allow_simulation = host.cinematic_allow_simulation;
    host.frame_state.warning_manager_present = true;  // game+21E0h, the owned queue
    host.control.state = host.entry_state.game_state;

    host.log.notef("mission state entry: state=0x%02X entered=%d dynamics_released=%zu "
        "interface=%s cinematic_cleared=%d one_shots{aborted=%d end_latch=%d "
        "networked=%d dropped=%d not_enough_players=%d}",
        host.entry.state, host.entry.entered ? 1 : 0, host.entry.dynamics_released,
        host.entry.interface_request.empty() ? "(none)" : host.entry.interface_request.c_str(),
        host.entry.cinematic_cleared ? 1 : 0,
        host.entry_state.one_shots.scene_ended_by_abort ? 1 : 0,
        host.entry_state.one_shots.end_scene_body_done ? 1 : 0,
        host.entry_state.one_shots.session_was_networked ? 1 : 0,
        host.entry_state.one_shots.session_dropped ? 1 : 0,
        host.entry_state.one_shots.not_enough_players ? 1 : 0);

    // Milestone 2l: the mission's own orders. usn_2_java.lua's `luaStageInit`
    // calls CreateScript("luaInit"), and `luaInit` is where the mission issues
    // every order it issues: NavigatorMoveToRange, NavigatorAttackMove,
    // JoinFormation and the rest of the Navigator family. The script manager
    // that would call it is three records, so the executable calls it here,
    // once, on the frame the mission enters state 0Dh.
    // Packet cc_lua_binding_audit: 00898750 now runs its own body, and that body
    // calls the named global itself (0089898B / 009290A0), on the frame the mission
    // hands the name over. The stand-in above would then run `luaInit` a second
    // time, so it runs only while the binding is still a record.
    if (host.script_orders == nullptr || host.script_orders->timers().scripts_created == 0) {
        host.lua.run_created_scripts();
    } else {
        host.log.notef("script objects: CreateScript 00898750 ran its own body %zu time(s) "
            "and called each named global from inside it, so milestone 2l's one-shot "
            "stand-in is not run",
            host.script_orders->timers().scripts_created);
    }
    return host.entry.entered;
}

void GameMissionFrameHost::set_mission_complete_frame(long frame) noexcept {
    impl_->complete_frame = frame;
}

void GameMissionFrameHost::set_mission_key(std::string key) {
    // game+2198h, the key 009205e0 commits the scoring record under. The
    // mission start writes it (docs/MISSION_TREE_BRIEFING_SCREENS.md).
    impl_->result->set_mission_key(std::move(key));
}

void GameMissionFrameHost::set_player_order(long frame, float throttle,
    float rudder) noexcept {
    impl_->order_frame = frame;
    impl_->order_throttle = throttle;
    impl_->order_rudder = rudder;
}

void GameMissionFrameHost::set_player_commanded_speed(float speed) noexcept {
    impl_->order_speed = speed;
    impl_->order_speed_set = true;
}

void GameMissionFrameHost::set_player_command(std::string token, std::string target,
    std::string unit) {
    impl_->order_command = std::move(token);
    impl_->order_command_target = std::move(target);
    impl_->order_command_unit = std::move(unit);
}

void GameMissionFrameHost::set_helm_orders(const std::string& path) {
    // Packet cc9_scripted_helm_orders. One order per line:
    //   <mission frame> moveto <unit> <x> <z>
    //   <mission frame> moveto <unit> <navpoint name>
    // plus `... repeat <s>`, `<frame> takehelm <unit> <throttle> <point> [stop
    // <m>]` and `<frame> select <unit>` (docs/SCRIPTED_HELM.md sections 9, 10),
    // `attack` (11) and `target` (12).
    // Blank lines and lines starting with '#' are skipped. A malformed line is
    // refused here, with its number, and the rest are kept.
    Impl& host = *impl_;
    host.helm_orders_path = path;
    std::FILE* file = nullptr;
    if (fopen_s(&file, path.c_str(), "r") != 0 || file == nullptr) {
        host.log.notef("helm orders refused: cannot open \"%s\"", path.c_str());
        return;
    }
    char text[512];
    int line = 0;
    while (std::fgets(text, sizeof(text), file) != nullptr) {
        ++line;
        std::istringstream in(text);
        std::vector<std::string> words;
        std::string word;
        while (in >> word) words.push_back(word);
        if (words.empty() || words[0][0] == '#') continue;
        // cc9-ships38: a double-quoted run of words is one word, so a unit
        // name with spaces can stand where one word is read (the `launch` base,
        // e.g. "MainAirFieldEntity 01"). The quotes are removed.
        for (std::size_t w = 0; w < words.size(); ++w) {
            if (words[w].size() < 1 || words[w][0] != '"') continue;
            std::size_t last = w;
            while (last < words.size()
                   && (words[last].size() < (last == w ? 2u : 1u)
                       || words[last].back() != '"')) {
                ++last;
            }
            if (last >= words.size()) break;   // unclosed: left as typed
            std::string merged = words[w];
            for (std::size_t k = w + 1; k <= last; ++k) merged += ' ' + words[k];
            words[w] = merged.substr(1, merged.size() - 2);
            words.erase(words.begin() + static_cast<std::ptrdiff_t>(w) + 1,
                        words.begin() + static_cast<std::ptrdiff_t>(last) + 1);
        }
        Impl::HelmOrder order;
        order.line = line;
        char* end = nullptr;
        order.frame = std::strtol(words[0].c_str(), &end, 10);
        const bool frame_ok = end != nullptr && *end == '\0' && order.frame >= 0;
        const bool verb_ok = words.size() >= 2
            && (words[1] == "moveto" || words[1] == "takehelm" || words[1] == "select"
                || words[1] == "attack" || words[1] == "target" || words[1] == "launch"
                || words[1] == "release" || words[1] == "build"
                || words[1] == "depthcharge" || words[1] == "fire" || words[1] == "order"
            || words[1] == "torpedo" || words[1] == "input" || words[1] == "aim"
            || words[1] == "releasegun");
        // Packet cc13-releasegun: `<frame> releasegun <unit...>`. One line opens
        // the map for the rest of the run; a second line only logs
        // `map close-requested` (the close arm 0068B4E9 is not hosted).
        const bool releasegun_ok = verb_ok && words[1] == "releasegun" && words.size() >= 3;
        // `<frame> input <action> [hold <frames>]`.
        const bool input_ok = verb_ok && words[1] == "input"
            && (words.size() == 3 || (words.size() == 5 && words[3] == "hold"));
        // `<frame> aim <yaw> <pitch>`.
        // `<frame> aim unit <name...> [pitch]`.
        const bool aim_unit = verb_ok && words[1] == "aim" && words.size() >= 4
            && words[2] == "unit";
        const bool aim_ok = verb_ok && words[1] == "aim" && (aim_unit || words.size() == 4);
        // `<frame> order <base> <slot> <order> [target...]`.
        const bool air_order_ok = verb_ok && words[1] == "order" && words.size() >= 5;
        // `<frame> depthcharge <ship...>`.
        const bool depthcharge_ok = verb_ok && words[1] == "depthcharge" && words.size() >= 3;
        // `<frame> fire <ship...> group <g> [release]`: the ship name may carry
        // spaces, so the group is read from the end.
        const bool fire_release = verb_ok && words[1] == "fire" && words.back() == "release";
        const std::size_t fire_words = words.size() - (fire_release ? 1 : 0);
        const bool fire_ok = verb_ok && words[1] == "fire" && fire_words >= 5
            && words[fire_words - 2] == "group";
        // GUNNERY 152/155: `<frame> torpedo <ship...> at <target...>` presses the
        // torpedo group (79h group 4) aimed at a named unit; `<frame> torpedo
        // <ship...> release` lets it go. Both names may carry spaces; `at` splits.
        std::size_t torpedo_at = 0;
        if (verb_ok && words[1] == "torpedo") {
            for (std::size_t w = 3; w + 1 < words.size(); ++w) {
                if (words[w] == "at") { torpedo_at = w; break; }
            }
        }
        const bool torpedo_release = verb_ok && words[1] == "torpedo" && torpedo_at == 0
            && words.size() >= 4 && words.back() == "release";
        const bool torpedo_ok = torpedo_release || torpedo_at != 0;
        // `<frame> build <shipyard...> class <id>`: the shipyard name may carry
        // spaces, so the class is the last two words.
        const bool build_ok = verb_ok && words[1] == "build" && words.size() >= 5
            && words[words.size() - 2] == "class";
        // Packet cc9_player_order_capture_row: `<frame> select <unit>`.
        const bool select_ok = verb_ok && words[1] == "select" && words.size() == 3;
        const bool launch_ok = verb_ok && words[1] == "launch" && words.size() >= 7;
        // Packet cc9_player_bomb_release: `<frame> release <unit>` or
        // `<frame> release <unit> on <target...> [within <m>] [until <frame>]`.
        const bool release_ok = verb_ok && words[1] == "release"
            && (words.size() == 3 || (words.size() >= 5 && words[3] == "on"));
        if (!frame_ok || !verb_ok || (words[1] == "select" ? !select_ok
                : words[1] == "launch" ? !launch_ok
                : words[1] == "release" ? !release_ok
                : words[1] == "build" ? !build_ok
                : words[1] == "depthcharge" ? !depthcharge_ok
                : words[1] == "fire" ? !fire_ok
                : words[1] == "torpedo" ? !torpedo_ok
                : words[1] == "input" ? !input_ok
                : words[1] == "aim" ? !aim_ok
                : words[1] == "releasegun" ? !releasegun_ok
                : words[1] == "order" ? !air_order_ok : words.size() < 4)) {
            host.log.notef("helm order refused: line %d of \"%s\" is not `<frame> moveto "
                "<unit> <x> <z>|<navpoint> [repeat <s>]`, `<frame> takehelm <unit> "
                "<throttle> <x> <z>|<navpoint> [stop <m>]`, `<frame> select <unit>` or `<frame> "
                "attack|target <unit> <target> [repeat <s>]`, `<frame> launch <base> <slot> "
                "<class> <count> <target> [loadout <n>]` or `<frame> release <unit> [on <target> [within "
                "<m>] [until <frame>]]` or `<frame> build <shipyard> class <id>` or `<frame> "
                "fire <ship> group <g> [release]` or `<frame> torpedo <ship> at <target>|release` "
                "or `<frame> order <base> <slot> <1|2|3> [target]` or `<frame> input "
                "<action> [hold <frames>]` or `<frame> aim <yaw> <pitch>` or `<frame> aim unit <name> "
                "[pitch]` or `<frame> releasegun <unit>`", line, path.c_str());
            continue;
        }
        order.unit = words[2];
        if (aim_unit) {
            // The last word is the pitch (degrees) when it is a number and a name
            // remains before it; quote a name that ends in a number.
            char* pe = nullptr;
            const double p = words.size() >= 5
                ? std::strtod(words.back().c_str(), &pe) : 0.0;
            const bool has_pitch = words.size() >= 5 && pe != nullptr && *pe == '\0'
                && std::isfinite(p);
            order.unit.clear();
            for (std::size_t w = 3; w < words.size() - (has_pitch ? 1 : 0); ++w) {
                if (!order.unit.empty()) order.unit += ' ';
                order.unit += words[w];
            }
            constexpr double kDegToRad = 3.141592653589793 / 180.0;
            order.view_aim = true;
            order.aim_track = true;
            order.aim_track_has_pitch = has_pitch;
            order.aim_pitch = static_cast<float>(p * kDegToRad);
            host.helm_orders.push_back(order);
            continue;
        }
        if (aim_ok) {
            char* ye = nullptr;
            char* pe = nullptr;
            const double y = std::strtod(words[2].c_str(), &ye);
            const double p = std::strtod(words[3].c_str(), &pe);
            if (ye == nullptr || *ye != '\0' || pe == nullptr || *pe != '\0'
                || !std::isfinite(y) || !std::isfinite(p)) {
                host.log.notef("helm order refused: line %d of \"%s\": aim needs a yaw and a "
                    "pitch in degrees", line, path.c_str());
                continue;
            }
            constexpr double kDegToRad = 3.141592653589793 / 180.0;
            order.view_aim = true;
            order.aim_yaw = static_cast<float>(y * kDegToRad);
            order.aim_pitch = static_cast<float>(p * kDegToRad);
            host.helm_orders.push_back(order);
            continue;
        }
        if (input_ok) {
            char* ae = nullptr;
            char* fe = nullptr;
            // Decimal, or hex with a 0x / 0X prefix (never octal: "0153" is 153).
            const std::string& at = words[2];
            const bool hex = at.size() > 2 && at[0] == '0' && (at[1] == 'x' || at[1] == 'X');
            const long a = std::strtol(at.c_str(), &ae, hex ? 16 : 10);
            const long n = words.size() == 5 ? std::strtol(words[4].c_str(), &fe, 10) : 1;
            if (ae == nullptr || *ae != '\0' || a < 0 || a == 0x4E
                || (words.size() == 5 && (fe == nullptr || *fe != '\0')) || n < 1) {
                host.log.notef("helm order refused: line %d of \"%s\": input needs an action "
                    "number other than 4Eh (decimal or 0x hex) and a hold of at least one "
                    "frame", line, path.c_str());
                continue;
            }
            order.input_hold = true;
            order.input_action = static_cast<int>(a);
            order.input_frames = static_cast<unsigned>(n);
            host.helm_orders.push_back(order);
            continue;
        }
        if (air_order_ok) {
            char* se = nullptr;
            char* oe = nullptr;
            const long s = std::strtol(words[3].c_str(), &se, 10);
            const long o = std::strtol(words[4].c_str(), &oe, 10);
            if (se == nullptr || *se != '\0' || s < 1 || oe == nullptr || *oe != '\0'
                || o < 1 || o > 3) {
                host.log.notef("helm order refused: line %d of \"%s\": order needs a 1-based "
                    "slot and an order of 1 (moveto), 2 (recall) or 3 (attack)", line,
                    path.c_str());
                continue;
            }
            order.air_order = true;
            order.air_order_slot = static_cast<int>(s);
            order.air_order_kind = static_cast<int>(o);
            for (std::size_t w = 5; w < words.size(); ++w) {
                if (!order.target.empty()) order.target += ' ';
                order.target += words[w];
            }
            host.helm_orders.push_back(order);
            continue;
        }
        if (fire_ok) {
            char* ge = nullptr;
            const long g = std::strtol(words[fire_words - 1].c_str(), &ge, 10);
            if (ge == nullptr || *ge != '\0' || g < 0) {
                host.log.notef("helm order refused: line %d of \"%s\": fire needs a "
                    "non-negative group number", line, path.c_str());
                continue;
            }
            order.fire_group = true;
            order.fire_group_id = static_cast<int>(g);
            order.fire_held = !fire_release;
            for (std::size_t w = 3; w + 2 < fire_words; ++w) order.unit += ' ' + words[w];
            host.helm_orders.push_back(order);
            continue;
        }
        if (words[1] == "torpedo") {
            order.fire_group = true;
            order.fire_group_id = 4;
            order.fire_held = !torpedo_release;
            const std::size_t ship_end = torpedo_release ? words.size() - 1 : torpedo_at;
            for (std::size_t w = 3; w < ship_end; ++w) order.unit += ' ' + words[w];
            if (!torpedo_release) {
                for (std::size_t w = torpedo_at + 1; w < words.size(); ++w) {
                    if (!order.target.empty()) order.target += ' ';
                    order.target += words[w];
                }
            }
            host.helm_orders.push_back(order);
            continue;
        }
        if (depthcharge_ok) {
            // `depthcharge <ship>` is now a one-frame press of the fire key with
            // group 5: `fire <ship> group 5` on its frame and the release on the
            // next one. (cc9-ships38's 009E2B60 stand-in is retired.)
            for (std::size_t w = 3; w < words.size(); ++w) order.unit += ' ' + words[w];
            order.fire_group = true;
            order.fire_group_id = 5;
            order.fire_held = true;
            host.helm_orders.push_back(order);
            order.frame += 1;
            order.fire_held = false;
            host.helm_orders.push_back(order);
            continue;
        }
        if (build_ok) {
            char* ne = nullptr;
            const long v = std::strtol(words.back().c_str(), &ne, 10);
            if (ne == nullptr || *ne != '\0' || v <= 0) {
                host.log.notef("helm order refused: line %d of \"%s\": build needs a positive "
                    "class id", line, path.c_str());
                continue;
            }
            order.build = true;
            order.build_class = static_cast<int>(v);
            order.unit.clear();
            for (std::size_t w = 2; w + 2 < words.size(); ++w) {
                if (!order.unit.empty()) order.unit += ' ';
                order.unit += words[w];
            }
            host.helm_orders.push_back(order);
            continue;
        }
        if (select_ok) {
            order.select = true;
            host.helm_orders.push_back(order);
            continue;
        }
        if (releasegun_ok) {
            order.release_gun = true;
            order.unit.clear();
            for (std::size_t w = 2; w < words.size(); ++w) {
                if (!order.unit.empty()) order.unit += ' ';
                order.unit += words[w];
            }
            host.helm_orders.push_back(order);
            continue;
        }
        if (release_ok) {
            order.bomb_release = true;
            bool numbers_ok = true;
            while (words.size() >= 7 && (words[words.size() - 2] == "within"
                                         || words[words.size() - 2] == "until")) {
                char* ne = nullptr;
                if (words[words.size() - 2] == "within") {
                    order.release_within = std::strtof(words.back().c_str(), &ne);
                    if (ne == nullptr || *ne != '\0' || !(order.release_within > 0.0f)) {
                        numbers_ok = false;
                    }
                } else {
                    order.release_until = std::strtol(words.back().c_str(), &ne, 10);
                    if (ne == nullptr || *ne != '\0' || order.release_until < order.frame) {
                        numbers_ok = false;
                    }
                }
                words.resize(words.size() - 2);
            }
            if (!numbers_ok) {
                host.log.notef("helm order refused: line %d of \"%s\": release needs `within` "
                    "a positive number of metres and `until` a frame not before its own",
                    line, path.c_str());
                continue;
            }
            for (std::size_t w = 4; w < words.size(); ++w) {
                if (!order.target.empty()) order.target += ' ';
                order.target += words[w];
            }
            host.helm_orders.push_back(order);
            continue;
        }
        // Packet cc9_helm_orders_launch: `<frame> launch <base> <slot> <class>
        // <count> <target...>`, the Support Manager screen's launch (00675C40,
        // 0067A57D..0067A5E6) through cc9-lua41's script_orders_player_air_ops_launch.
        if (launch_ok) {
            int* fields[3] = {&order.launch_slot, &order.launch_class, &order.launch_count};
            bool numbers_ok = true;
            for (int k = 0; k < 3; ++k) {
                char* ne = nullptr;
                const long v = std::strtol(words[3 + k].c_str(), &ne, 10);
                if (ne == nullptr || *ne != '\0' || v < 0) numbers_ok = false;
                *fields[k] = static_cast<int>(v);
            }
            if (!numbers_ok) {
                host.log.notef("helm order refused: line %d of \"%s\": launch needs a slot, a "
                    "class and a count as non-negative integers", line, path.c_str());
                continue;
            }
            order.launch = true;
            // `... <target...> loadout <n>`: the Support Manager's loadout ring
            // choice (screen+334h -> slot+10h at 0067485D); harness input.
            std::size_t target_end = words.size();
            if (words.size() >= 9 && words[words.size() - 2] == "loadout") {
                char* le = nullptr;
                const long v = std::strtol(words.back().c_str(), &le, 10);
                if (le == nullptr || *le != '\0' || v < 0) {
                    host.log.notef("helm order refused: line %d of \"%s\": launch loadout "
                        "needs a non-negative integer", line, path.c_str());
                    continue;
                }
                order.launch_loadout = static_cast<int>(v);
                target_end -= 2;
            }
            for (std::size_t w = 6; w < target_end; ++w) {
                if (!order.target.empty()) order.target += ' ';
                order.target += words[w];
            }
            host.helm_orders.push_back(order);
            continue;
        }
        // Packet cc9_player_attack_row: `<frame> attack <unit> <target...>
        // [repeat <s>]`; the target is the rest of the line (names such as
        // "Coastal Gun 01" carry spaces). Packet cc9_player_squadron_target:
        // `<frame> target <unit> <target...> [repeat <s>]` has the same shape.
        if (words[1] == "attack" || words[1] == "target") {
            if (words.size() >= 6 && words[words.size() - 2] == "repeat") {
                char* re = nullptr;
                order.repeat_seconds = std::strtof(words.back().c_str(), &re);
                if (re == nullptr || *re != '\0' || !(order.repeat_seconds > 0.0f)) {
                    host.log.notef("helm order refused: line %d of \"%s\": repeat needs a "
                        "positive number of seconds", line, path.c_str());
                    continue;
                }
                words.resize(words.size() - 2);
            }
            order.attack = words[1] == "attack";
            order.squadron_target = words[1] == "target";
            for (std::size_t w = 3; w < words.size(); ++w) {
                if (!order.target.empty()) order.target += ' ';
                order.target += words[w];
            }
            host.helm_orders.push_back(order);
            continue;
        }
        // Packet cc9_helm_orders_helm_route: `takehelm` carries a throttle
        // before the point, and `moveto` may end in `repeat <seconds>`.
        if (words[1] == "takehelm") {
            char* te = nullptr;
            order.takehelm = true;
            order.throttle = std::strtof(words[3].c_str(), &te);
            if (te == nullptr || *te != '\0') {
                host.log.notef("helm order refused: line %d of \"%s\": %s is not a throttle",
                    line, path.c_str(), words[3].c_str());
                continue;
            }
            words.erase(words.begin() + 3);
            // Packet cc9_player_order_capture_row: `... stop <metres>`.
            if (words.size() >= 6 && words[words.size() - 2] == "stop") {
                char* se = nullptr;
                order.stop_radius = std::strtof(words.back().c_str(), &se);
                if (se == nullptr || *se != '\0' || !(order.stop_radius > 0.0f)) {
                    host.log.notef("helm order refused: line %d of \"%s\": stop needs a "
                        "positive number of metres", line, path.c_str());
                    continue;
                }
                words.resize(words.size() - 2);
            }
        } else if (words.size() >= 6 && words[words.size() - 2] == "repeat") {
            char* re = nullptr;
            order.repeat_seconds = std::strtof(words.back().c_str(), &re);
            if (re == nullptr || *re != '\0' || !(order.repeat_seconds > 0.0f)) {
                host.log.notef("helm order refused: line %d of \"%s\": repeat needs a "
                    "positive number of seconds", line, path.c_str());
                continue;
            }
            words.resize(words.size() - 2);
        }
        if (words.size() < 4) {
            host.log.notef("helm order refused: line %d of \"%s\" has no point", line,
                path.c_str());
            continue;
        }
        if (words.size() == 5) {
            char* xe = nullptr;
            char* ze = nullptr;
            order.x = std::strtof(words[3].c_str(), &xe);
            order.z = std::strtof(words[4].c_str(), &ze);
            if (xe == nullptr || *xe != '\0' || ze == nullptr || *ze != '\0') {
                host.log.notef("helm order refused: line %d of \"%s\": %s %s is not an x z "
                    "pair", line, path.c_str(), words[3].c_str(), words[4].c_str());
                continue;
            }
        } else if (words.size() == 4) {
            order.point_name = words[3];
        } else {
            host.log.notef("helm order refused: line %d of \"%s\" has %zu words", line,
                path.c_str(), words.size());
            continue;
        }
        host.helm_orders.push_back(order);
    }
    std::fclose(file);
    if (!host.helm_orders.empty()) {
        host.log.notef("helm orders: %zu read from \"%s\"", host.helm_orders.size(),
            path.c_str());
    }
}

void GameMissionFrameHost::set_ai_drive(std::string unit, float throttle, float rudder) {
    impl_->ai_drive_unit = std::move(unit);
    impl_->ai_drive_throttle = throttle;
    impl_->ai_drive_rudder = rudder;
}

void GameMissionFrameHost::set_trajectory_csv(std::string path) {
    impl_->trajectory_csv_path = std::move(path);
}

void GameMissionFrameHost::set_mission_frame_seconds(float seconds) noexcept {
    impl_->mission_frame_seconds = seconds;
}

bool GameMissionFrameHost::in_mission_phase() const noexcept {
    return impl_->control.state
        == static_cast<std::uint32_t>(bsp::GameStateId::kInMission);
}

bool GameMissionFrameHost::exit_path_finished() const noexcept {
    return impl_->exit_path_finished;
}

bool GameMissionFrameHost::run_mission_frame_004e4a40(float raw_delta_in) {
    Impl& host = *impl_;
    if (!host.entry.entered) return false;

    // Milestone 2i, --mission-frame-seconds S. Executable plumbing, the same
    // kind as --frames: it replaces the wall-clock frame interval so a headless
    // run accumulates simulated time deterministically and the fixed-step
    // driver's own clock at 00f876a4 does not depend on the frame rate.
    const float raw_delta
        = host.mission_frame_seconds > 0.0f ? host.mission_frame_seconds : raw_delta_in;

    host.world_clock += raw_delta;
    host.frame_state.raw_delta = raw_delta;
    host.frame_state.scaled_delta = raw_delta;  // no time dilation in this process
    host.frame_state.global_time = host.world_clock;
    host.world.game.elapsed = host.world_clock;
    // 004E4D45: 004C6E30 stores the scaled delta at game+21F0h, after the
    // request drain (004E4D02, which reads no delta) and before the fixed steps
    // and Think (004E5133). frame_state.scaled_delta is that value here: with
    // no fast-forward, turbo, slow-motion or cinematic input and a step under
    // the single-view threshold, 004C6E30 returns the delta unchanged.
    if constexpr (kScaledDeltaWriteBound) host.world.game.scaled_delta = host.frame_state.scaled_delta;
    host.result->set_mission_clock(host.world_clock);

    // 004e4d02, the first thing OnMove does after the console pre-tick: the
    // request drain, skipped while game+5ECh is set. Milestone 2f ran only the
    // in-mission branch and never drained, because nothing could enqueue.
    {
        FrameControlBinding control(host);
        if (host.control.drain_suspended) {
            if (!host.drain_skip_reported) {
                host.drain_skip_reported = true;
                host.log.notef("the request drain is suspended at game+5ECh, so 004e4430 "
                    "does not run this frame");
            }
        } else {
            bsp::drain_state_requests_004e4430(host.control, control);
        }
        // 004e504b, the state 11h arm of the frame bookkeeping: it lowers the
        // suspension the teardown arm raised and asks for the front end.
        const std::size_t queued = host.control.requests.count;
        const bool waiting = host.control.state
            == static_cast<std::uint32_t>(bsp::GameStateId::kMissionEndWait);
        bsp::update_mission_end_wait(host.control, control);
        if (waiting) {
            const bool enqueued = host.control.requests.count > queued;
            host.result->note_mission_end_wait(enqueued);
            host.log.notef("mission end wait: state 11h, game+7184h clear, so 004e506b "
                "cleared the drain suspension and %s request 04h",
                enqueued ? "enqueued" : "did not enqueue");
        }
    }

    // The mission left state 0Dh: the in-mission branch of 004e4a40 no longer
    // runs, and what is left is the exit path's own frames.
    if (host.control.state
        != static_cast<std::uint32_t>(bsp::GameStateId::kInMission)) {
        ++host.exit_frames;
        host.frames.exit_frames = host.exit_frames;
        host.log.notef("  mission exit frame %llu: game state 0x%02X, requests queued=%zu",
            host.exit_frames, host.control.state, host.control.requests.count);
        const bool front_end = host.control.state
            == static_cast<std::uint32_t>(bsp::GameStateId::kFrontEnd);
        if (front_end || host.exit_frames >= kMaxExitFrames) {
            host.exit_path_finished = true;
            host.frames.exit_completed = front_end;
            return false;
        }
        return true;
    }

    // Packet cc9_scripted_helm_orders: every --helm-orders line whose mission
    // frame has come. HARNESS PATH (docs/SCRIPTED_HELM.md section 8): the order
    // is the player's moveto command form, 005F9B20's MoveTo object 00E08F68
    // with a point descriptor and flags 1 into 0077D600, but it is addressed to
    // the named unit directly instead of [00E188D8], and no HUD, selection or
    // camera step runs.
    if (!host.helm_orders.empty() && host.units != nullptr) {
        const unsigned long long now = host.frames.frames + 1;
        // cc9-ships50: 005484F0 builds one 79h per frame from the held fire key
        // (docs/PLAYER_GUN_SEAT.md section 1), and a surface ship's tubes only
        // pull the trigger once trained within 5 degrees of the snapped bearing
        // (0095A2FC), so a held torpedo press is re-sent every frame until its
        // release line. Before, one message at the press and one at the release
        // trained the tubes but never fired them (trigger_ons 0).
        for (std::size_t h = 0; h < host.helm_torpedo_holds.size();) {
            Impl::HelmTorpedoHold& hold = host.helm_torpedo_holds[h];
            GameGunneryHost* gunnery = host.units->gunnery();
            std::string why;
            bool sent = false;
            if (!host.units->controlled_bound() || host.units->controlled_index() != hold.unit) {
                why = "not the controlled unit [00E188D8] any more, which 005484F0 requires";
            } else if (gunnery == nullptr) {
                why = "no gunnery host";
            } else {
                sent = gunnery->player_fire_torpedo_group(hold.unit, hold.target, 0.0f, true, why);
            }
            if (sent) {
                ++hold.frames;
                ++h;
                continue;
            }
            host.log.notef("helm torpedo hold dropped: line %d (at mission frame %llu) after %llu "
                "held frame(s): %s", hold.line, now, hold.frames, why.c_str());
            host.helm_torpedo_holds.erase(host.helm_torpedo_holds.begin()
                + static_cast<std::ptrdiff_t>(h));
        }
        for (Impl::HelmOrder& order : host.helm_orders) {
            if (order.applied || static_cast<unsigned long long>(order.frame) > now) continue;
            if (order.bomb_release) {
                // Packet cc9_player_bomb_release (docs/SCRIPTED_HELM.md section 14):
                // the player's bomb fire, IC_PLANE_BOMBFIRE (action A8h,
                // scripts\datatables\Inputs.lua) read by the plane screen's
                // 006082D0 at 0060877F, whose message C4h (00605270) reaches
                // BSP_Plane_HandleMessage 007CD0B4 -> 007BBBA0. The units host
                // applies 006082D0's gates. `on <target>` stands for the player
                // watching the sight: the press is the first frame whose bomb,
                // released now, is predicted (009C7D71's prediction) to fall
                // within <m> of the target's position advanced by its own
                // velocity over the fall time. LABELLED: the lead and the radius
                // are the file's, not a person's eye; the bomb switch is held.
                const std::size_t count = host.units->count();
                const std::size_t idx = harness_target_row(*host.units, order.unit);
                const std::size_t target_idx = order.target.empty()
                    ? count : harness_target_row(*host.units, order.target);
                const float seconds = host.units->summary().simulated_seconds;
                const char* why = "";
                if (idx >= count) {
                    why = " (no created unit has that name)";
                } else if (!host.units->controlled_bound() || host.units->controlled_index() != idx) {
                    why = " (not the controlled unit [00E188D8]: 006082D0 reads the plane "
                          "screen's own unit)";
                } else if (!order.target.empty() && (target_idx >= count
                           || !host.units->unit_alive_and_visible(target_idx))) {
                    why = " (the target is not a live created unit)";
                }
                if (*why == '\0' && !order.target.empty()) {
                    // The sight. The target's velocity is its last frame's
                    // displacement; the first frame only samples it.
                    float t[3];
                    host.units->unit_position_00fc(target_idx, t[0], t[1], t[2]);
                    const float dt = seconds - order.release_last_seconds;
                    float tv[3] = {0.0f, 0.0f, 0.0f};
                    const bool have = order.release_have_last && dt > 0.0f;
                    if (have) {
                        for (int i = 0; i < 3; ++i) tv[i] = (t[i] - order.release_last[i]) / dt;
                    }
                    for (int i = 0; i < 3; ++i) order.release_last[i] = t[i];
                    order.release_last_seconds = seconds;
                    order.release_have_last = true;
                    bsp::DiveBombImpactPointInputs ip;
                    host.units->unit_position_00fc(idx, ip.unit_position[0], ip.unit_position[1],
                        ip.unit_position[2]);
                    host.units->unit_linear_velocity(idx, ip.unit_velocity);
                    ip.aim_point_y = t[1];
                    const bsp::DiveBombImpactPoint p = bsp::dive_bomb_impact_point_009c7d71(ip);
                    const float lead_x = t[0] + tv[0] * p.fall_time;
                    const float lead_z = t[2] + tv[2] * p.fall_time;
                    const float miss = std::sqrt((p.point[0] - lead_x) * (p.point[0] - lead_x)
                        + (p.point[2] - lead_z) * (p.point[2] - lead_z));
                    const bool expired = order.release_until >= 0
                        && now > static_cast<unsigned long long>(order.release_until);
                    if (have && (order.release_best < 0.0f || miss < order.release_best)) {
                        order.release_best = miss;
                    }
                    if (!have || !(p.fall_time > 0.0f) || miss > order.release_within) {
                        if (!expired) continue;   // not yet on the sight: try next frame
                        order.applied = true;
                        ++host.helm_orders_refused;
                        host.log.notef("helm order refused: line %d frame %ld (at mission frame "
                            "%llu) release %s on %s: the sight never came within %.1f m "
                            "(nearest %.1f m)", order.line, order.frame, now,
                            order.unit.c_str(), order.target.c_str(),
                            static_cast<double>(order.release_within),
                            static_cast<double>(order.release_best));
                        continue;
                    }
                    host.log.notef("helm order sight: line %d at mission frame %llu t=%.2f "
                        "%s on %s: predicted (%.1f %.1f) lead (%.1f %.1f) miss %.1f m tf %.2f s "
                        "alt %.1f", order.line, now, static_cast<double>(seconds),
                        order.unit.c_str(), order.target.c_str(),
                        static_cast<double>(p.point[0]), static_cast<double>(p.point[2]),
                        static_cast<double>(lead_x), static_cast<double>(lead_z),
                        static_cast<double>(miss), static_cast<double>(p.fall_time),
                        static_cast<double>(ip.unit_position[1]));
                }
                order.applied = true;
                bool accepted = false;
                std::string reason;
                if (*why == '\0') {
                    const GameUnitsHost::PlayerBombRelease r =
                        host.units->player_bomb_release_006082d0(idx);
                    accepted = r.accepted;
                    reason = r.reason;
                } else {
                    reason = why + 2;   // drop the leading " ("
                    if (!reason.empty() && reason.back() == ')') reason.pop_back();
                }
                if (accepted) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) "
                    "release %s t=%.2f through 0060877F A8h -> 00605270 C4h -> 007CD0B4 -> "
                    "007BBBA0%s%s", accepted ? "applied" : "refused", order.line,
                    order.frame, now, order.unit.c_str(), static_cast<double>(seconds),
                    reason.empty() ? "" : ": ", reason.c_str());
                continue;
            }
            order.applied = true;
            const long due = order.frame;
            if (order.input_hold) {
                // Packet cc13-firehold: the action is held from the next frame's
                // input prologue (GameMenuHost::frame), the way 00a92aa0 starts a
                // real press there; every reader of 004c43c0 / 004C5090 sees it.
                const bool accepted = inject_input_action_hold(order.input_action,
                                                               order.input_frames);
                if (accepted) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) input "
                    "0x%02X hold %u frame(s) through the action record (00a92aa0 / 00a919f0 / "
                    "00a92370 -> 004c43c0)", accepted ? "applied" : "refused", order.line, due,
                    now, static_cast<unsigned>(order.input_action), order.input_frames);
                continue;
            }
            if (order.aim_track) {
                // Packet cc13-aimunit: re-aim every frame at the unit's position
                // through the same look axes, until the next `aim` line.
                const std::size_t row = harness_target_row(*host.units, order.unit);
                if (row >= host.units->count()) {
                    ++host.helm_orders_refused;
                    std::size_t earlier = 0;
                    bool earlier_pitch = false;
                    float earlier_pitch_value = 0.0f;
                    if (injected_view_aim_track(earlier, earlier_pitch, earlier_pitch_value)) {
                        host.log.notef("helm order refused: line %d frame %ld (at mission frame "
                            "%llu) aim unit \"%s\": no such unit; the earlier track (row %zu) "
                            "keeps steering", order.line, due, now, order.unit.c_str(), earlier);
                    } else {
                        host.log.notef("helm order refused: line %d frame %ld (at mission frame "
                            "%llu) aim unit \"%s\": no such unit; the look keeps its last aim",
                            order.line, due, now, order.unit.c_str());
                    }
                    continue;
                }
                inject_view_aim_track(row, order.aim_track_has_pitch, order.aim_pitch);
                ++host.helm_orders_applied;
                if (order.aim_track_has_pitch) {
                    host.log.notef("helm order applied: line %d frame %ld (at mission frame %llu) "
                        "aim unit \"%s\" (row %zu) pitch %.4f rad, yaw tracked every frame through "
                        "the look axes (0051F050)", order.line, due, now, order.unit.c_str(), row,
                        static_cast<double>(order.aim_pitch));
                } else {
                    host.log.notef("helm order applied: line %d frame %ld (at mission frame %llu) "
                        "aim unit \"%s\" (row %zu), yaw and pitch tracked every frame through the "
                        "look axes (0051F050)", order.line, due, now, order.unit.c_str(), row);
                }
                continue;
            }
            if (order.view_aim) {
                // Packet cc13-helmaim: the target is reached through the look
                // axes 0051F050 reads, from the next ship-view input (0064A400).
                inject_view_aim(order.aim_yaw, order.aim_pitch);
                ++host.helm_orders_applied;
                host.log.notef("helm order applied: line %d frame %ld (at mission frame %llu) aim "
                    "yaw %.4f pitch %.4f rad through the look axes (0051F050 -> mover +384h / "
                    "+388h)", order.line, due, now, static_cast<double>(order.aim_yaw),
                    static_cast<double>(order.aim_pitch));
                continue;
            }
            if (order.select) {
                // Packet cc9_player_order_capture_row: the player's unit click as
                // the HUD root update commits it (00649860, 0064A00E..0064A101):
                // the four byte filters (+5Ch set, +5Dh/+60h/+5Eh clear, the
                // 0043F080 cells), then 00645060, 00645600 and the 20h push, which
                // is 00647300's body (docs/HUD_CENTRAL_UPDATES.md). LABELLED: no
                // click writes HUD root+78h; the commit runs here, before the
                // fixed step, not from the HUD pump.
                std::size_t pick = harness_target_row(*host.units, order.unit);
                bool reached = false;
                bool accepted = false;
                const char* why = "";
                if (pick >= host.units->count()) {
                    why = " (no created unit has that name)";
                } else if (!host.units->unit_alive_and_visible(pick)
                           && bsp::plane_squadron_live_fused_slot(pick) == nullptr) {
                    why = " (0064A057..0064A079: the four byte filters fail)";
                } else {
                    accepted = bsp::game::hud_set_selected_unit_00647300(pick, reached);
                    if (!reached) why = " (no HUD is attached)";
                    else if (!accepted) why = " (00645060 rejects it)";
                }
                if (accepted) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) select "
                    "%s%s through 0064A00E -> 00645060 -> 00645600 -> 20h", accepted
                    ? "applied" : "refused", order.line, due, now, order.unit.c_str(), why);
                continue;
            }
            if (order.release_gun) {
                // Packet cc13-releasegun (kHarnessReleaseGunMapBound). The player
                // opens the map with C1h (IC_MAP_TOGGLE): 0068C1F0 reads the edge
                // at 0068C64C and 0068B470 -> 0068AC30 publishes the level-2 set,
                // whose opaque 4Ch exits screen 46h; 0064A490 releases the group
                // (005470C0) and roles 1..8 of the ship-view unit. LABELLED: the
                // press is the helm `input` line's held action record, and the
                // line arms the host's 0068B470 / 4Ch / 0064A490 wiring.
                const std::size_t idx = harness_target_row(*host.units, order.unit);
                bool reached = false;
                bool accepted = false;
                const char* why = "";
                if (!kHarnessReleaseGunMapBound) {
                    why = " (kHarnessReleaseGunMapBound is OFF)";
                } else if (idx >= host.units->count()) {
                    why = " (no created unit has that name)";
                } else if (!host.units->controlled_bound()
                           || host.units->controlled_index() != idx) {
                    why = " (not the controlled unit [00E188D8], whose ship view 46h holds it)";
                } else if (!bsp::game::hud_arm_release_gun_map_toggle(reached)) {
                    why = " (no HUD is attached)";
                } else {
                    accepted = inject_input_action_hold(0xC1, 1);
                    if (!accepted) why = " (the action record refused C1h)";
                }
                if (accepted) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) "
                    "releasegun %s%s: input C1h through 0068C64C -> 0068B470 -> 0068AC30",
                    accepted ? "applied" : "refused", order.line, due, now,
                    order.unit.c_str(), why);
                continue;
            }
            if (order.fire_group) {
                // Packet cc9_player_fire_group_line. In the image the player's
                // group key (9Fh, IC_GUNC_DEPTHCHARGE in this installation's
                // Inputs.lua, for group 5) selects the weapon group on screen 2Eh,
                // and holding the fire key 99h makes 005484F0 build message 79h
                // (00954A10) for [00E188D8] with the held byte at msg+34h; letting
                // go sends it again with the byte clear. 00959C20 jumps on group
                // 5 to 0095A441. The line calls cc9-gunnery30's
                // GameGunneryHost::player_fire_weapon_group, which sends that
                // message (kPlayerWeaponGroupFireBound) and refuses with its
                // reason otherwise. LABELLED: no group screen or role take runs.
                std::size_t idx = harness_target_row(*host.units, order.unit);
                GameGunneryHost* gunnery = host.units->gunnery();
                std::string why;
                bool fired = false;
                if (idx >= host.units->count()) {
                    why = "no created unit has that name";
                } else if (!host.units->controlled_bound()
                           || host.units->controlled_index() != idx) {
                    why = "not the controlled unit [00E188D8], which 005484F0 requires";
                } else if (gunnery == nullptr) {
                    why = "no gunnery host";
                } else {
                    if (order.fire_group_id == 4) {
                        // 0095A1CC: the torpedo group (GUNNERY 152/155).
                        fired = gunnery->player_fire_torpedo_group(idx, order.target, 0.0f,
                            order.fire_held, why);
                        auto& holds = host.helm_torpedo_holds;
                        for (std::size_t h = 0; h < holds.size(); ++h) {
                            if (holds[h].unit != idx) continue;
                            host.log.notef("helm torpedo hold ends: line %d held %llu re-sent "
                                "frame(s) before line %d", holds[h].line, holds[h].frames,
                                order.line);
                            holds.erase(holds.begin() + static_cast<std::ptrdiff_t>(h));
                            break;
                        }
                        if (fired && order.fire_held) {
                            Impl::HelmTorpedoHold hold;
                            hold.unit = idx;
                            hold.target = order.target;
                            hold.line = order.line;
                            holds.push_back(hold);
                        }
                    } else {
                        fired = gunnery->player_fire_weapon_group(idx, order.fire_group_id,
                            order.fire_held, why);
                    }
                }
                if (fired) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) "
                    "fire %s group %d %s: %s (99h -> 005484F0 79h -> 00959C20)",
                    fired ? "applied" : "refused", order.line, due, now, order.unit.c_str(),
                    order.fire_group_id, order.fire_held ? "held" : "release", why.c_str());
                continue;
            }
            if (order.build) {
                // The strategic map's shipyard purchase (00673A10: A7h until the
                // entry holds the class, then A9h), one unit per order, through
                // cc9-gunnery30's GameGunneryHost::shipyard_order. LABELLED: no
                // map screen runs; the file names the shipyard and the class.
                std::size_t yard = harness_target_row(*host.units, order.unit);
                GameGunneryHost* gunnery = host.units->gunnery();
                int chosen = 0;
                std::string reason;
                bool accepted = false;
                if (yard >= host.units->count()) {
                    reason = "no created unit has that name";
                } else if (gunnery == nullptr) {
                    reason = "no gunnery host";
                } else {
                    accepted = gunnery->shipyard_order(yard, 0, 1, chosen, reason,
                                                       order.build_class);
                }
                if (accepted) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) build "
                    "%s class %d: entry %d: %s (00673A10 A7h/A9h -> shipyard_order)",
                    accepted ? "applied" : "refused", order.line, due, now,
                    order.unit.c_str(), order.build_class, chosen, reason.c_str());
                continue;
            }
            if (order.launch) {
                // Packet cc9_helm_orders_launch: the player's Support Manager launch
                // (00675C40's 0067A57D stock check, 0067A5A7 006C0F00 slot fill,
                // 0067A5D1 82h slot message, order 3 = attack target). The entry
                // logs its own `player air ops launch:` line. LABELLED: no screen,
                // and the base need not be the controlled unit's carrier.
                // LABELLED (cc9-ships38): the screen offers only targets that pass
                // the marker gate 0043F080 (006431A8), so a target that is dead or
                // hidden is refused here before the entry runs; a file can then
                // list fallback targets for one slot on consecutive frames.
                std::size_t target_pick = harness_target_row(*host.units, order.target);
                if (target_pick < host.units->count()
                    && !host.units->unit_alive_and_visible(target_pick)) {
                    ++host.helm_orders_refused;
                    host.log.notef("helm order refused: line %d frame %ld (at mission frame %llu) "
                        "launch %s slot %d -> %s: the target fails 0043F080 (dead or hidden)",
                        order.line, due, now, order.unit.c_str(), order.launch_slot,
                        order.target.c_str());
                    continue;
                }
                const bsp::game::PlayerAirOpsLaunchResult r = bsp::game::script_orders_player_air_ops_launch(
                    order.unit, order.launch_slot, order.launch_class, order.launch_count,
                    order.target, order.launch_loadout);
                if (r.accepted) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) launch "
                    "%s slot %d class %d count %d -> %s: slot %d, %d plane(s)%s%s",
                    r.accepted ? "applied" : "refused", order.line, due, now,
                    order.unit.c_str(), order.launch_slot, order.launch_class,
                    order.launch_count, order.target.c_str(), r.slot, r.count,
                    r.reason.empty() ? "" : ": ", r.reason.c_str());
                continue;
            }
            if (order.air_order) {
                // Packet cc9_player_air_ops_order_line: the Support Manager's
                // order to a slot whose squadron is out or recalled, the 82h
                // message {slot, order, target} into 006CCDA0, through cc9-lua44's
                // script_orders_player_air_ops_order (kAirOpsHeldSlotOrdersBound).
                // LABELLED: no screen runs; the file names the base and the slot.
                const bsp::game::PlayerAirOpsOrderResult r
                    = bsp::game::script_orders_player_air_ops_order(order.unit,
                        order.air_order_slot, order.air_order_kind, order.target);
                if (r.accepted) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) order "
                    "%s slot %d order %d%s%s: answer %d, state after %d, %d plane(s)%s%s "
                    "(82h -> 006CCDA0)", r.accepted ? "applied" : "refused", order.line, due,
                    now, order.unit.c_str(), order.air_order_slot, order.air_order_kind,
                    order.target.empty() ? "" : " -> ", order.target.c_str(), r.answer,
                    r.state_after, r.planes, r.reason.empty() ? "" : ": ", r.reason.c_str());
                continue;
            }
            if (order.squadron_target) {
                // Packet cc9_player_squadron_target: the player's target pick
                // 00525250 on a controlled squadron. 00523130 picks the hostile
                // entity nearest the screen centre and returns settarget
                // (00E08EF8) for a unit that is not a ship; 005252ED issues it
                // through 0077D600 on [00E188D8] with a kind-1 descriptor and
                // flags = IsKindOf(18h). The squadron's command intake, vtable
                // 00D087C0 +160h = 007F1940, turns settarget into 007EEC50's
                // class and issues that to its members (007F1AD6..007F1B24), as
                // the AI's settarget to a squadron does (kAiSquadronSetTarget-
                // IntakeBound). LABELLED: the file names the target instead of
                // the screen-centre pick (00523130's relation, 00804350(5) and
                // 005220C0 tests are not run); this host fuses the squadron
                // into its flight leader, so "the controlled unit is a member of
                // a plane squadron" stands in for IsKindOf(18h); message
                // delivery is immediate.
                const std::size_t count = host.units->count();
                std::size_t idx = count;
                std::size_t target_idx = count;
                // LABELLED (cc9-ships38): `<t1> | <t2> | ...` names the player's
                // next picks in order; the first that passes 0043F080 (live and
                // visible) is taken, standing in for the player moving the
                // screen centre to the next target once one is gone.
                std::vector<std::string> picks;
                for (std::size_t from = 0;;) {
                    const std::size_t bar = order.target.find(" | ", from);
                    picks.push_back(order.target.substr(from, bar == std::string::npos
                        ? std::string::npos : bar - from));
                    if (bar == std::string::npos) break;
                    from = bar + 3;
                }
                idx = harness_target_row(*host.units, order.unit);
                for (const std::string& pick : picks) {
                    const std::size_t first = harness_target_row(*host.units, pick);
                    if (first < count && target_idx == count) target_idx = first;
                    if (first < count && host.units->unit_alive_and_visible(first)) {
                        target_idx = first;
                        break;
                    }
                }
                const GameUnitRow* picked_row = target_idx < count
                    ? host.units->unit_row(target_idx) : nullptr;
                const std::string picked = picked_row != nullptr ? picked_row->name : order.target;
                const bsp::PlaneSquadronHostRecord* squadron = idx < count
                    ? bsp::plane_squadron_registry().find_by_member_unit(idx) : nullptr;
                // CC13 item A (OFF): the fused slot's own plane has left +3D0h
                // at its death; the squadron is still the record that names it.
                if (squadron == nullptr && idx < count) {
                    squadron = bsp::plane_squadron_live_fused_slot(idx);
                }
                std::vector<std::size_t> members;
                if (squadron != nullptr) {
                    for (const std::size_t m : squadron->member_units) {
                        if (m < count && host.units->unit_alive_and_visible(m)) {
                            members.push_back(m);
                        }
                    }
                }
                const char* why = "";
                if (idx >= count) {
                    why = " (no created unit has that name)";
                } else if (order.applications > 0 && !host.units->unit_alive_and_visible(idx)) {
                    continue;   // a repeat stops when the unit is gone
                } else if (!host.units->controlled_bound()
                           || host.units->controlled_index() != idx) {
                    why = " (not the controlled unit [00E188D8])";
                } else if (squadron == nullptr || members.empty()) {
                    why = " (not a plane squadron: 00523130 issues settarget only on 007F1940)";
                } else if (target_idx >= count
                           || !host.units->unit_alive_and_visible(target_idx)) {
                    why = " (the target is not a live created unit)";
                }
                std::uint32_t chosen = 0u;
                if (*why == '\0') {
                    chosen = script_orders_squadron_intake_007f1940(members.front(), members,
                        target_idx);
                    if (chosen == 0u) why = " (007EEC50 chose no class: nothing issued)";
                }
                const bool issued = chosen != 0u;
                ++order.applications;
                if (issued && order.repeat_seconds > 0.0f) {
                    const float frame_seconds = host.mission_frame_seconds > 0.0f
                        ? host.mission_frame_seconds : 0.05f;
                    long step = static_cast<long>(order.repeat_seconds / frame_seconds + 0.5f);
                    if (step < 1) step = 1;
                    order.frame = static_cast<long>(now) + step;
                    order.applied = false;
                }
                if (issued) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) target "
                    "%s -> %s%s through 00525250: 0077D600(00E08EF8 settarget, kind 1, flags 1) "
                    "-> 007F1940 -> class %08lx to %zu member(s)", issued ? "applied" : "refused",
                    order.line, due, now, order.unit.c_str(), picked.c_str(), why,
                    static_cast<unsigned long>(chosen), members.size());
                continue;
            }
            if (order.attack) {
                // Packet cc9_player_attack_row: the order screen's page-1 attack,
                // 005FAAE0's ship arm (005FAAF5..005FABA6): [00E188D8] must be a
                // ship; 0077C470(unit, 2, 0) gives back role 1 (the helm); then
                // 0077D600(00E08F78 attackmove, 00465080(target, 0.0), flags 1).
                // LABELLED: the image attacks the director's fire target
                // (+238h, 008364E0) and the player's pick path is unread, so
                // the target is named here; message 57h is not reproduced.
                std::size_t idx = harness_target_row(*host.units, order.unit);
                const char* why = "";
                if (idx >= host.units->count()) {
                    why = " (no created unit has that name)";
                } else if (order.applications > 0 && !host.units->unit_alive_and_visible(idx)) {
                    continue;   // a repeat stops when the unit is gone
                } else if (!host.units->controlled_bound()
                           || host.units->controlled_index() != idx) {
                    why = " (not the controlled unit [00E188D8])";
                } else if (!host.units->unit_is_kind_of(idx, 0x06)) {
                    why = " (005FAB05: not a ship)";
                } else {
                    // Harness rule, not the image's (cc9-ships50): a named target that is
                    // a created unit but no longer live is refused, so a stale repeat in
                    // an order file cannot re-task the ship onto a wreck.
                    const std::size_t target_idx = harness_target_row(*host.units, order.target);
                    if (target_idx < host.units->count()
                        && !host.units->unit_alive_and_visible(target_idx)) {
                        why = " (refused: target dead)";
                    }
                }
                bool issued = false;
                if (*why == '\0') {
                    host.units->role_request_0077c470(idx, 2u, false);   // 005FAB38
                    issued = host.units->issue_player_command("attackmove", order.target,
                        order.unit);                                       // 005FABA1
                    if (!issued) why = " (the command path refused it)";
                }
                ++order.applications;
                if (issued && order.repeat_seconds > 0.0f) {
                    const float frame_seconds = host.mission_frame_seconds > 0.0f
                        ? host.mission_frame_seconds : 0.05f;
                    long step = static_cast<long>(order.repeat_seconds / frame_seconds + 0.5f);
                    if (step < 1) step = 1;
                    order.frame = static_cast<long>(now) + step;
                    order.applied = false;
                }
                if (issued) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) attack "
                    "%s -> %s%s through 005FAAE0: 0077C470(unit, 2, 0), 0077D600(00E08F78, "
                    "00465080(target), 1)", issued ? "applied" : "refused", order.line, due, now,
                    order.unit.c_str(), order.target.c_str(), why);
                continue;
            }
            float x = order.x;
            float z = order.z;
            if (!order.point_name.empty()) {
                const Impl::HelmMarker* found = nullptr;
                for (const Impl::HelmMarker& marker : host.helm_markers) {
                    if (marker.name == order.point_name) { found = &marker; break; }
                }
                if (found == nullptr) {
                    ++host.helm_orders_refused;
                    host.log.notef("helm order refused: line %d frame %ld moveto %s %s: no "
                        "scene marker has that name", order.line, order.frame,
                        order.unit.c_str(), order.point_name.c_str());
                    continue;
                }
                x = found->position[0];
                z = found->position[2];
            }
            std::size_t unit_index = harness_target_row(*host.units, order.unit);
            if (order.takehelm) {
                // Packet cc9_helm_orders_helm_route: the controlled unit's helm.
                // Packet cc13-rolef (log only): the helm step opens EROLF_PILOT to
                // PLAYER_ANY (game_hosts_units.cpp, 00927D20(2, 9)) before its role-1
                // take. A real player cannot open it, so a takehelm is tagged
                // role_forced=1 when the word it will replace, unit+188h + 1*4, is
                // neither 9 (PLAYER_ANY) nor the local slot 0 (game+18ECh in single
                // player), e.g. a script's PLAYER_AI (8). Read before the take.
                std::int32_t pilot_permission = 9;
                const bool permission_read = unit_index < host.units->count()
                    && host.units->unit_role_permission(unit_index, 1, pilot_permission);
                const bool taken = unit_index < host.units->count()
                    && host.units->helm_route_take(unit_index, order.throttle, x, z);
                const bool role_forced = taken && permission_read
                    && pilot_permission != 9 && pilot_permission != 0;
                if (taken) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                if (role_forced) ++host.helm_role_forced;
                // Packet cc13-mapback (log only): map_up is the state
                // kHelmPrepareMapUpGateCc13Bound tests (the map occludes 46h).
                host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) "
                    "takehelm %s throttle %.3f %s(%.1f, %.1f) stop %.1f%s map_up=%d",
                    taken ? "applied" : "refused", order.line, order.frame, now,
                    order.unit.c_str(), static_cast<double>(order.throttle),
                    order.point_name.empty() ? "" : (order.point_name + " ").c_str(),
                    static_cast<double>(x), static_cast<double>(z),
                    static_cast<double>(order.stop_radius), role_forced ? " role_forced=1" : "",
                    host.units->helm_screen_occluded_by_map() ? 1 : 0);
                // Packet cc9_player_order_capture_row: a later takehelm replaces
                // the pending stop, as a new helm order replaces the route.
                if (taken) {
                    host.helm_stop_armed = order.stop_radius > 0.0f;
                    host.helm_stop_unit = unit_index;
                    host.helm_stop_radius = order.stop_radius;
                    host.helm_stop_x = x;
                    host.helm_stop_z = z;
                    host.helm_stop_line = order.line;
                }
                continue;
            }
            if (order.applications > 0 && (unit_index >= host.units->count()
                    || !host.units->unit_alive_and_visible(unit_index))) {
                continue;   // a repeat stops when the unit is gone
            }
            // cc9-ships38 (SHIP_AI 190.1): the image's moveto 005F9B20 goes to
            // [00E188D8] (005F9B42), and a unit becomes [00E188D8] only through
            // 00645060 (00647317, 0064564D). A unit 00645060 would refuse can
            // therefore never take the player's moveto; the harness refuses it.
            // LABELLED: a selectable unit that is not the controlled one is still
            // ordered without the select (SCRIPTED_HELM 8.2 difference 2).
            // Packet cc13-mvsel: such an order is counted and its line tagged
            // `unselected=1`; the read is const and selects nothing.
            const bool unselected = !(host.units->controlled_bound()
                && host.units->controlled_index() == unit_index);
            if (unit_index < host.units->count() && unselected) {
                bool hud_reached = false;
                const bool selectable =
                    bsp::game::hud_unit_selectable_00645060(unit_index, hud_reached);
                if (hud_reached && !selectable) {
                    ++order.applications;
                    ++host.helm_orders_refused;
                    host.log.notef("helm order refused: line %d frame %ld (at mission frame "
                        "%llu) moveto %s: 00645060 rejects the unit, so it can never be "
                        "[00E188D8], the unit 005F9B20 orders", order.line, due, now,
                        order.unit.c_str());
                    continue;
                }
            }
            if constexpr (kHelmPlaneMovetoBound) {
                const int planes = host.script_orders != nullptr
                    ? host.script_orders->player_plane_moveto(unit_index, x, z) : -1;
                if (planes >= 0) {
                    const bool issued = planes > 0;
                    ++order.applications;
                    if (issued && order.repeat_seconds > 0.0f) {
                        const float frame_seconds = host.mission_frame_seconds > 0.0f
                            ? host.mission_frame_seconds : 0.05f;
                        long step = static_cast<long>(order.repeat_seconds / frame_seconds + 0.5f);
                        if (step < 1) step = 1;
                        order.frame = static_cast<long>(now) + step;
                        order.applied = false;
                    }
                    if (issued) ++host.helm_orders_applied; else ++host.helm_orders_refused;
                    if (issued && unselected) ++host.helm_moveto_unselected;
                    host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) "
                        "moveto %s %s(%.1f, %.1f) to %d squadron plane(s) (005F9B20 00E08F68 -> "
                        "0099A170, packet p20_squadron_helm)%s", issued ? "applied" : "refused",
                        order.line, due, now, order.unit.c_str(),
                        order.point_name.empty() ? "" : (order.point_name + " ").c_str(),
                        static_cast<double>(x), static_cast<double>(z), planes,
                        issued && unselected ? " unselected=1" : "");
                    continue;
                }
            }
            char point[64];
            std::snprintf(point, sizeof(point), "%.3f,%.3f", static_cast<double>(x),
                static_cast<double>(z));
            const bool issued = host.units->issue_player_command("moveto", point, order.unit);
            ++order.applications;
            if (issued && order.repeat_seconds > 0.0f) {
                // Packet cc9_helm_orders_helm_route: a player clicking again.
                const float frame_seconds = host.mission_frame_seconds > 0.0f
                    ? host.mission_frame_seconds : 0.05f;
                long step = static_cast<long>(order.repeat_seconds / frame_seconds + 0.5f);
                if (step < 1) step = 1;
                order.frame = static_cast<long>(now) + step;
                order.applied = false;
            }
            if (issued) ++host.helm_orders_applied; else ++host.helm_orders_refused;
            if (issued && unselected) ++host.helm_moveto_unselected;
            host.log.notef("helm order %s: line %d frame %ld (at mission frame %llu) "
                "moveto %s %s(%.1f, %.1f) through the player's moveto form (00E08F68, point "
                "descriptor, flags 1, 0077D600)%s", issued ? "applied" : "refused", order.line,
                due, now, order.unit.c_str(),
                order.point_name.empty() ? "" : (order.point_name + " ").c_str(),
                static_cast<double>(x), static_cast<double>(z),
                issued && unselected ? " unselected=1" : "");
        }
        // Packet cc9_player_order_capture_row: `stop R`, the player pulling the
        // throttle lever back once the helm unit is within R metres (x/z) of its
        // point. The route keeps role 1 and the rudder law keeps the bow on the
        // point while the hull coasts down. LABELLED: a hand on the lever in
        // the image; the radius is the harness's.
        if (host.helm_stop_armed && host.helm_stop_unit < host.units->count()) {
            float ux = 0.0f, uy = 0.0f, uz = 0.0f;
            host.units->unit_position_00fc(host.helm_stop_unit, ux, uy, uz);
            const double dx = static_cast<double>(host.helm_stop_x) - ux;
            const double dz = static_cast<double>(host.helm_stop_z) - uz;
            const double distance = std::sqrt(dx * dx + dz * dz);
            if (distance <= static_cast<double>(host.helm_stop_radius)) {
                host.helm_stop_armed = false;
                const bool stopped = host.units->helm_route_take(host.helm_stop_unit, 0.0f,
                    host.helm_stop_x, host.helm_stop_z);
                const GameUnitRow* row = host.units->unit_row(host.helm_stop_unit);
                host.log.notef("helm route stop %s: line %d \"%s\" at (%.1f, %.1f), %.1f m from "
                    "(%.1f, %.1f) at mission frame %llu: throttle 0", stopped ? "applied"
                    : "refused", host.helm_stop_line, row != nullptr ? row->name.c_str() : "?",
                    static_cast<double>(ux), static_cast<double>(uz), distance,
                    static_cast<double>(host.helm_stop_x), static_cast<double>(host.helm_stop_z),
                    now);
            }
        }
    }

    // --order-frame N with --order throttle=<f>,rudder=<f>: one player order to
    // the controlled unit, through the same 00816a40 the authored command
    // takes. A running game issues it from the HUD's own order path; no input
    // backend exists here, so the executable issues it and says so.
    if (host.order_frame >= 0 && !host.order_issued && host.units != nullptr
        && host.frames.frames + 1 >= static_cast<unsigned long long>(host.order_frame)) {
        host.order_issued = true;
        host.frames.player_order_issued = true;
        if (!host.order_command.empty()) {
            // Milestone 2l: the command form. 0046aab0 resolves the name
            // against the 26-row registry and the whole hop chain runs, which
            // is the path the authored scene command takes and not the ring.
            host.units->issue_player_command(host.order_command,
                host.order_command_target, host.order_command_unit);
        } else {
            host.units->issue_player_order(host.order_throttle, host.order_rudder);
        }
        // Milestone 2o: the diagnostic stand-in for the state step, engaged
        // after the order that chose the state, so the two take effect on the
        // same in-mission frame.
        if (!host.ai_drive_unit.empty()) {
            host.units->enable_ai_drive(host.ai_drive_unit, host.ai_drive_throttle,
                host.ai_drive_rudder);
        }
        if (host.order_speed_set && host.units->controlled_bound()) {
            // Milestone 2m: the store luaMW_SetShipSpeed 00890d30 makes. This
            // mission's script never calls it, and neither does
            // luaMW_NavigatorMoveOnPath 008a3600, so the switch is the only way
            // the executable can put a commanded speed on a unit.
            const std::size_t unit = host.units->controlled_index();
            host.units->store_commanded_speed_00890e6f(unit, host.order_speed);
            const bsp::CruiseSpeedSetting pair = host.units->commanded_speed(unit);
            host.log.notef("commanded speed issued to the controlled unit: "
                "navigatorParams+24h = %.3f m/s, +28h = %.3f (the mission clock). "
                "00836e59 now answers active, so the director's idle tail re-issues "
                "`cruise` instead of `stop`, and 009e12bd turns the speed into a "
                "throttle by dividing it by 0080fc30's reference speed",
                static_cast<double>(pair.speed), static_cast<double>(pair.enable));
        }
    }

    // --mission-complete-frame N: the script's own PlayBinkMovie(name, true),
    // injected on the frame the switch names. commandhelpers.lua's end helpers
    // do make that call (docs/CC13_VERIFY16.md R8), but the Lua row reaches the
    // same path only under kLuaPlayBinkMovieBound (packet cc13-fix20, OFF).
    if (host.complete_frame > 0 && !host.complete_injected
        && host.frames.frames + 1 >= static_cast<unsigned long long>(host.complete_frame)) {
        host.complete_injected = true;
        host.frames.complete_injected = true;
        host.log.notef("mission complete injected on frame %llu: the executable calls the "
            "path a script's PlayBinkMovie(name, true) takes", host.frames.frames + 1);
        host.result->play_bink_movie_0089a480(host.load.short_name, true);
    }

    MissionFrameBinding binding(host);
    const bsp::MissionFrameResult result
        = bsp::run_mission_frame(host.frame_state, binding);
    // Packet cc_lua_binding_audit: row 8 of the fixed-step fan-out, 00875E64 /
    // 00929460, over the script entities CreateScript made. The fan-out's own row
    // runs against an empty list because nothing else registers a think; this pass
    // is the same reconstructed rule with the mission's script entities in it, and
    // it is what re-enters `luaTimetable` when a luaDelay expires.
    if (result.simulated && host.script_orders != nullptr) {
        host.script_orders->run_script_timers(raw_delta);
    }
    ++host.frames.frames;
    if (result.simulated) ++host.frames.simulated;
    if (result.paused) ++host.frames.paused;
    host.frames.menu_drain_iterations
        += static_cast<unsigned long long>(result.menu_drain_iterations);
    host.frames.script_calls = host.lua.summary().native_calls;
    if (result.mission_completion_requested) host.frames.completion_requested = true;

    // Milestone 2i: what the world walk and the units did this frame.
    if (host.world_host != nullptr) {
        const GameWorldSummary& world_summary = host.world_host->summary();
        host.frames.world_walks = world_summary.walks;
        host.frames.entities_walked = world_summary.entities_walked;
        host.frames.entities_updated = world_summary.entities_updated;
        host.frames.merged_unit_list = world_summary.list_counts[7];
    }
    if (host.units != nullptr) {
        const GameUnitsSummary& units = host.units->summary();
        host.frames.units = units.units;
        host.frames.unit_motion_ticks = units.motion_ticks;
        host.frames.simulated_seconds = units.simulated_seconds;
        host.frames.controlled_distance = units.controlled_distance;
        host.frames.total_path_length = units.total_path_length;
        host.frames.controlled_unit = units.controlled_name;
    }

    host.log.notef("  mission frame %llu simulated=%d paused=%d units=%llu events=%llu "
        "script_calls=%llu erased=%d", host.frames.frames, result.simulated ? 1 : 0,
        result.paused ? 1 : 0, host.frames.units_ticked,
        host.frames.mission_events_applied, host.frames.script_calls,
        result.input_entries_erased);
    // The world walk's own per-frame line, and the controlled unit's trajectory
    // every ten in-mission frames.
    if (host.world_host != nullptr) host.world_host->log_frame(host.frames.frames);
    if (host.units != nullptr && host.frames.frames % 10 == 0) {
        host.units->log_controlled_trajectory(host.frames.frames);
    }
    // A frame that asked to leave state 0Dh is not the last frame any more: the
    // request it enqueued is dispatched by the next frame's drain, which is
    // where the exit path runs.
    return true;
}

void GameMissionFrameHost::report(long requested_frames) {
    Impl& host = *impl_;
    host.frames.requested = requested_frames;
    if constexpr (kHintTriggerPassesCc15Bound) {
        if (GameHintTrackerSlot* slot = GameHintTrackerSlot::process_slot()) {
            if (const GameHintTracker* tracker = slot->built()) tracker->report();
        }
    }
    const GameMissionExitSummary& exit = host.result->summary();
    // Milestone 2f reported this as unreachable because nothing built a result
    // object. What makes the path reachable is the producer, not the poll:
    // 0089a390 calls 004d7970 itself when the script asks for the debrief.
    host.frames.exit_reachable = exit.end_scene_ran || host.frames.completion_requested;
    if (host.frames.exit_completed) {
        host.frames.exit_note = "the mission ended through 004d7970: request 10h, the "
            "teardown arm 004e458a, state 11h and request 04h";
    } else if (exit.end_scene_ran) {
        host.frames.exit_note = "004d7970 ran but the drain did not reach request 04h";
    } else if (host.frames.completion_requested) {
        host.frames.exit_note = "004d7ea0 enqueued the debrief request 0Fh";
    } else {
        host.frames.exit_note = "no mission-result object at game+7188h, so nothing "
            "asked to leave state 0Dh; --mission-complete-frame builds one";
    }
    host.fixed_step->report();
    if (host.step_subsystems != nullptr) host.step_subsystems->report();
    host.result->report();
    if (host.hud != nullptr) host.hud->report();
    if (host.world_host != nullptr) host.world_host->report();
    if (host.script_orders != nullptr) host.script_orders->report();
    // Packet cc_lua_find_entity: the shipped script's own `Mission` table, which
    // is how far the run carried the mission rather than how far the host ran.
    host.lua.report_mission_script_state();
    if (host.ship_ai != nullptr) host.ship_ai->report();
    if (host.units != nullptr) host.units->report();
    if (!host.trajectory_csv_path.empty()) {
        host.log.notef("summary mission trajectory csv=%s rows=%llu",
            host.trajectory_csv_path.c_str(), host.trajectory_csv.rows());
        host.trajectory_csv.close();
    }
    if (!host.helm_orders.empty()) {
        // Packet cc13-mvsel. No helm summary line existed; this one is printed
        // only when --helm-orders read a line, so a run without one is unchanged.
        host.log.notef("summary mission helm orders applied=%llu refused=%llu "
            "helm_moveto_unselected=%llu helm_role_forced=%llu", host.helm_orders_applied,
            host.helm_orders_refused, host.helm_moveto_unselected, host.helm_role_forced);
    }
    if (host.scene_contents) {
        // Milestone 2h. The scene contents pass created unit records. The world
        // object construct_world 004de610 builds is still mostly a load record:
        // the 97 per-kind lists are the units host's own, and the one byte the
        // gates read, [game+19CCh]+4ACh, is set only when kWorldActiveByteBound
        // (docs/CONSTRUCT_WORLD.md). 00481640 runs on game+21D0h, the
        // TrafficConfig, not on game+21A0h, which is the scoring object.
        const GameSceneContentsSummary& scene = host.scene_contents->summary();
        host.log.notef("summary mission scene contents mode=%d entities=%zu generated=%zu "
            "rejected=%zu created=%zu registration_bodies=%zu party_class_marks=%zu "
            "property_groups=%zu enum_tables=%zu", scene.effective_game_mode,
            scene.instantiate_entities, scene.generated, scene.rejected, scene.created,
            scene.registration_bodies, scene.party_class_marks, scene.property_groups,
            scene.enum_tables);
        // Milestone 2i supersedes milestone 2h's "every unit pass ticked 0 of
        // them" for the world walk and the motion virtual: the executable owns
        // the chain header 009037f0 allocates, so 00904bf0 and 00825f20 now run
        // over the created units. The passes that still tick nothing are the
        // ones that read world fields this host does not model; the gates of
        // 00875e69 and 00481640 read only the byte +4ACh (see above).
        host.log.notef("summary mission unit passes: %zu unit record(s) exist; the world walk "
            "00904bf0 updated %llu of them and the motion virtual 00825f20 ticked %llu, "
            "while every pass that reads the world object itself still ticks none, because "
            "construct_world 004de610 is a load record", scene.created,
            host.frames.entities_updated, host.frames.unit_motion_ticks);
    }
    host.log.notef("summary mission circle tangent unordered (004F4840, sw1 reach): bound=%d "
        "calls=%llu nan_r=%llu pc24_bound=%d pc24_differs=%llu",
        bsp::circle_tangent_reach().bound ? 1 : 0, bsp::circle_tangent_reach().calls,
        bsp::circle_tangent_reach().nan_r, bsp::circle_tangent_reach().pc24_bound ? 1 : 0,
        bsp::circle_tangent_reach().pc24_differs);
    for (int k = 0; k < static_cast<int>(bsp::Sw1bReach::Count); ++k) {
        const auto id = static_cast<bsp::Sw1bReach>(k);
        host.log.notef("summary mission switch reach (sw1b): %s sites=%llu differs=%llu",
            bsp::sw1b_reach_name(id), bsp::sw1b_reach(id).sites, bsp::sw1b_reach(id).differs);
    }
    host.log.notef("summary mission world active_byte_bound=%d active=%d traffic_walks=%llu "
        "(packet cc9_construct_world, [game+19CCh]+4ACh)", kWorldActiveByteBound ? 1 : 0,
        host.world.world_gate.enabled ? 1 : 0, host.frames.traffic_walks);
    host.log.notef("summary mission proximity scan bound=%d scans=%llu ships=%llu records=%llu "
        "hits=%llu expiries=%llu units_entries=%d part_rejects=%llu list24_nodes=%llu "
        "list24_no_leader=%llu (packets cc9_construct_world_p3, cc9_scan_units_entries, 00977990)",
        kScanProximityBound ? 1 : 0, host.warning_scans, host.proximity_ships_scanned,
        host.proximity_records_created, host.proximity_hits, host.proximity_expiries,
        kScanProximityUnitsEntriesBound ? 1 : 0, host.proximity_part_rejects,
        host.proximity_list24_nodes, host.proximity_list24_no_leader);
    host.log.notef("summary mission unit kills 00903670 bound=%d kills=%zu (packet "
        "cc9_sunk_ship_flush)", kSunkShipFlushBound ? 1 : 0, host.unit_kills);
    host.log.notef("summary mission warning manager bound=%d scans=%llu torpedo_reports=%llu "
        "accepted=%llu effect_calls=%llu effects=%llu deadline_expiries=%llu loss_reports=%llu "
        "loss_side0=%llu loss_side1=%llu loss_other=%llu loss_bound=%d text_uslost=%llu "
        "text_japanlost=%llu cancelled=%llu proximity_erased=%llu (packet "
        "cc9_warning_manager_tick; 009813a0, cc9_loss_report_entry, cc9_loss_warning)",
        kWarningManagerTickBound ? 1 : 0,
        host.warning_scans, host.warning_torpedo_calls, host.warning_torpedo_accepted,
        host.warning_effect_calls, host.warning_effects, host.warning_deadline_expiries,
        host.warning_loss_calls, host.warning_loss_side0, host.warning_loss_side1,
        host.warning_loss_other, kLossWarningBound ? 1 : 0, host.warning_loss_text_uslost,
        host.warning_loss_text_japanlost, host.warning_loss_cancelled,
        host.warning_loss_proximity_erased);
    if constexpr (kWarningMessageBankCc15Bound) {
        unsigned long long known = 0, unknown = 0;
        for (const auto& [id, counts] : host.warning_id_queries) {
            known += counts.first;
            unknown += counts.second;
        }
        host.log.notef("summary mission warning table bound=1 loaded=%d messages=%zu ids=%zu "
            "known=%llu unknown=%llu (packet cc15-wB4; 009870a0, 0096dbb0)",
            host.warning_tables_loaded ? 1 : 0, host.warning_tables().messages.size(),
            host.warning_id_queries.size(), known, unknown);
        for (const auto& [id, counts] : host.warning_id_queries) {
            host.log.notef("summary mission warning id %s known=%llu unknown=%llu",
                id.c_str(), counts.first, counts.second);
        }
        const auto& kill = host.kill_warning;
        host.log.notef("summary mission kill warning entries=%llu override_104=%llu "
            "suppressed_d0=%llu slot=%llu clock=%llu suppress_77edf0=%llu "
            "recon_not_evaluated=%llu recon_below=%llu plane_throttled=%llu filtered=%llu "
            "reports=%llu (00981e07..00982096)", kill.entries, kill.override_104,
            kill.suppressed_d0, kill.slot, kill.clock, kill.suppress_77edf0,
            kill.recon_not_evaluated, kill.recon_below, kill.plane_throttled, kill.filtered,
            kill.reports);
        const auto& hit = host.hit_warning;
        host.log.notef("summary mission hit warning calls=%llu suppressed_d0=%llu clock=%llu "
            "flags=%llu recon_not_evaluated=%llu recon_below=%llu stamp_throttled=%llu "
            "player_controlled=%llu own_shooter=%llu plane_count=%llu "
            "enemy_payload_not_evaluated=%llu neutral=%llu not_reportable=%llu "
            "command_building_not_evaluated=%llu reports=%llu (009895d8..00989992)",
            hit.calls, hit.suppressed_d0, hit.clock, hit.flags, hit.recon_not_evaluated,
            hit.recon_below, hit.stamp_throttled, hit.player_controlled, hit.own_shooter,
            hit.plane_count, hit.enemy_payload_not_evaluated, hit.neutral, hit.not_reportable,
            hit.command_building_not_evaluated, hit.reports);
        for (const auto& [id, outcome] : host.warning_report_outcomes) {
            host.log.notef("summary mission warning report id %s queued=%llu duplicate=%llu "
                "unknown=%llu other=%llu", id.c_str(), outcome[0], outcome[1], outcome[2],
                outcome[3]);
        }
        host.log.notef("summary mission warning lua callbacks=%zu (00887e50)",
            host.warning_lua_callbacks.size());
        for (const auto& [name, calls] : host.warning_lua_callbacks) {
            host.log.notef("summary mission warning lua callback %s calls=%llu", name.c_str(),
                calls);
        }
        const auto& activate = host.warning_activate;
        host.log.notef("summary mission warning activate activations=%llu single_variant=%llu "
            "draws=%llu sound_keys_untouched=%llu sound_keys_not_evaluated=%llu "
            "relation_blocked=%llu clips=%llu records_not_evaluated=%llu (0097F490, 0097F970, "
            "0097FA90; draws are the SUBSTITUTION for 00BD2FC0 stream 1 at 00979D0C/00979D11)",
            activate.activations, activate.single_variant, activate.draws,
            activate.sound_keys_untouched, activate.sound_keys_not_evaluated,
            activate.relation_blocked, activate.clips, activate.records_not_evaluated);
        if constexpr (kMessageManagerCc15Bound) {
            if (host.messages != nullptr) {
                const auto& messages = host.messages->summary();
                host.log.notef("summary mission message manager records=%zu constructed=%llu "
                    "loads=%llu clip_records_resolved=%llu clip_record_misses=%llu "
                    "sound_keys_resolved=%llu sound_keys_unmatched=%llu key_tests=%llu "
                    "key_hits=%llu (00707480, 007065E0, 00705E00, 0097D650/0097B5E0)",
                    host.messages->record_count(), messages.constructed, messages.loads,
                    activate.records_resolved, activate.record_misses,
                    activate.sound_keys_resolved, activate.sound_keys_unmatched,
                    messages.key_tests, messages.key_hits);
            }
        }
    }
    host.log.notef("summary mission fixed steps=%llu at %.3f s each (00875bb0's own clock "
        "at 00f876a4/00f876ac)", host.fixed_steps,
        static_cast<double>(bsp::kFixedSimulationStepFloat));
    host.log.notef("summary mission frames requested=%ld ran=%llu simulated=%llu paused=%llu "
        "units=%llu events=%llu script_calls=%llu interface_updates=%llu",
        host.frames.requested, host.frames.frames, host.frames.simulated,
        host.frames.paused, host.frames.units_ticked, host.frames.mission_events_applied,
        host.frames.script_calls, host.frames.interface_updates);
    host.log.notef("summary mission exit frames=%llu injected=%d completed=%d state=0x%02X",
        host.frames.exit_frames, host.frames.complete_injected ? 1 : 0,
        host.frames.exit_completed ? 1 : 0, host.control.state);
    host.log.notef("summary mission exit reachable=%d: %s",
        host.frames.exit_reachable ? 1 : 0, host.frames.exit_note.c_str());
    if (host.world_host != nullptr && host.units != nullptr) {
        host.log.notef("summary mission world units=%zu walked=%llu updated=%llu "
            "motion_ticks=%llu simulated=%.2f s controlled=%s moved=%.2f total_path=%.2f",
            host.frames.units, host.frames.entities_walked, host.frames.entities_updated,
            host.frames.unit_motion_ticks,
            static_cast<double>(host.frames.simulated_seconds),
            host.frames.controlled_unit.empty() ? "(none)"
                                                : host.frames.controlled_unit.c_str(),
            static_cast<double>(host.frames.controlled_distance),
            static_cast<double>(host.frames.total_path_length));
    }
}

}  // namespace bsp::game
