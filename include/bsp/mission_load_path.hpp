#pragma once
// The mission load path: from the mission-tree request to a populated scene.
// Addresses: 005c5600, 004e2770, 004bf930, 004e1d70, 004c6890, 004bb160,
//            004bb440, 00439020, 004d7920, 004e4430, 004dfb70, 004db920,
//            004da6c0.
//
// Every name here is a hypothesis, not a recovered symbol. docs/MISSION_LOAD_PATH.md
// holds the evidence, the original ABI of each routine and the uncertainties.
//
// This header is the glue between reconstructions that already exist. The load
// itself is bsp/mission_scene_load.hpp (004dfb70), the 0Ch handler and the
// 0Ch->0Dh transition are bsp/mission_state_entry.hpp (004db920, 004da6c0), the
// .scn reader is bsp/scene_file.hpp (0046df00) and the class table is
// bsp/scene_entity_factory.hpp (004f2800). Nothing from those files is
// redefined here. What is added is the part in front of the load - what selects
// the mission, what queues the request, and what the record it selects supplies
// to the eight scene slot records - plus the frame-by-frame stage machine that
// ties all of it together.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/mission_scene_load.hpp"
#include "bsp/mission_state_entry.hpp"

namespace bsp {

struct AudioSettings;

// ---------------------------------------------------------------------------
// The scene record (004e1d70 allocates it, 004c6890 reads it)
// ---------------------------------------------------------------------------

// operator new size at 004e1f30 and 004e213d: 0x109c bytes. The fields the load
// path reads live at +0h..+8FFh (the side blocks), +90Ch (the scene path),
// +928h (the script table), +980h, +988h, +914h and +1098h. The last field is
// four bytes at +1098h, which is exactly 0x109c, so the record has no tail
// beyond the mission id.
inline constexpr std::size_t kSceneRecordSize = 0x109c;

// The side-block array at record+0h. 004c6890 walks it with stride 0x120 for
// record+988h entries and copies two dwords out of each one. 8 * 0x120 = 0x900,
// and the scene path at +90Ch starts 0xc bytes later, so the array is capped at
// eight entries by the record layout as well as by the slot array.
inline constexpr std::size_t kSceneRecordSideBlockStride = 0x120;
inline constexpr std::size_t kSceneRecordSideBlockCount = 8;
// Offsets inside one side block that 004c6890 reads (004c6a4e, 004c6a5b).
inline constexpr std::size_t kSceneSideBlockSelectorOffset = 0x4; // -> slot +28h
inline constexpr std::size_t kSceneSideBlockSecondaryOffset = 0x8; // -> slot +24h

// record+914h. 004e1d70 sets it from the VFS provider table at 0109ceec,
// virtual +8h, with the scene path; the reuse arm runs the header pass of the
// .scn reader only when it is set. The create arm does not test it.
inline constexpr std::size_t kSceneRecordFileExistsOffset = 0x914;

// The two dwords out of one side block, as data. Neither is interpreted here:
// the selector is the value that reaches mission_side_suffix through the
// participant record, and the meaning of the second dword is not established.
struct SceneRecordSideBlock {
    std::int32_t selector{0};  // +4h
    std::int32_t secondary{0}; // +8h
};

// The parts of the 0x109c record this path reads. The full record is filled by
// the .scn header pass (bsp/scene_file.hpp); only what 004c6890, 004dfb70 and
// 004e1d70 take out of it is modelled.
struct SceneRecord {
    std::string scene_path;                       // +90Ch/+910h
    bool file_present{false};                     // +914h
    std::vector<SceneRecordSideBlock> side_blocks; // +0h, count at +988h
    std::vector<std::string> script_names;        // +928h, stride 8
    std::string locale_table_list;                // +980h/+984h
    std::int32_t mission_id{0};                   // +1098h
};

// ---------------------------------------------------------------------------
// The scene record list (game+5F0h)
// ---------------------------------------------------------------------------

// game+5F0h is a three-word intrusive list header {int count; Node* head;
// Node* tail;} over nodes of {Node* prev; Node* next; SceneRecord* record;}.
// 004c6890 walks it forward through node+4h and takes node+8h; the manual
// append at 004e21ac and the container push_back 004c2c80 both build the same
// 0xc-byte node.
inline constexpr std::size_t kSceneRecordListCountOffset = 0x5f0;
inline constexpr std::size_t kSceneRecordListHeadOffset = 0x5f4;
inline constexpr std::size_t kSceneRecordListTailOffset = 0x5f8;
inline constexpr std::size_t kSceneRecordListNodeSize = 0xc;

// The selected record and its index, game+5FCh and game+60Ch.
inline constexpr std::size_t kSelectedSceneRecordOffset = 0x5fc;
inline constexpr std::size_t kSelectedSceneIndexOffset = 0x60c;

// ---------------------------------------------------------------------------
// The eight scene slot records (game+1008h)
// ---------------------------------------------------------------------------

// 004bb160 points game+18CCh..18E8h at eight embedded records based at
// game+1008h with stride 0x118 (1008, 1120, 1238, 1350, 1468, 1580, 1698,
// 17B0). The live participant array 004bb440 claims from is a second array of
// the same 0x118 record at game+748h, and 004bb440 writes the same offsets
// (+8h, +24h, +28h, +58h, +78h), so the two arrays share one layout.
inline constexpr std::size_t kSceneSlotRecordBase = 0x1008;
inline constexpr std::size_t kSceneSlotRecordStride = 0x118;
inline constexpr std::size_t kSceneSlotRecordCount = 8;
inline constexpr std::size_t kParticipantRecordBase = 0x748;

// Offsets inside one 0x118 record that this path writes. The names are roles
// read off the two writers, not recovered field names.
inline constexpr std::size_t kSlotRecordInUseOffset = 0x8;      // byte, 004c6a44 / 004bb47d
inline constexpr std::size_t kSlotRecordSecondaryOffset = 0x24; // dword, 004c6a63
inline constexpr std::size_t kSlotRecordSelectorOffset = 0x28;  // dword, 004c6a52
inline constexpr std::size_t kSlotRecordNameOffset = 0x50;      // char[], 004c6a72
inline constexpr std::size_t kSlotRecordSecondNameOffset = 0x70; // char[], 004c6a84

// Byte offset of slot `index` from the game object, for either array.
std::size_t scene_slot_record_offset(std::size_t index) noexcept;
std::size_t participant_record_offset(std::size_t index) noexcept;

// One scene slot record as 004c6890 leaves it. The two strings are strcpy'd
// from the empty literal at 00ce3a0c, so they are always cleared here; they are
// filled later by 004bb440 from its own arguments.
struct SceneSlotRecord {
    bool in_use{false};          // +8h
    std::int32_t selector{0};    // +28h, from side block +4h
    std::int32_t secondary{0};   // +24h, from side block +8h
    std::string name;            // +50h, cleared
    std::string second_name;     // +70h, cleared
};

// ---------------------------------------------------------------------------
// 004c6890 BSP_Game_SelectSceneRecord
// ---------------------------------------------------------------------------

// The fields 004c6890 writes outside the slot array.
inline constexpr std::size_t kGameMissionIdByteOffset = 0x2015; // low byte of record+1098h
inline constexpr std::size_t kGameResolvedScriptSlotOffset = 0x2028;

struct SelectSceneRecordResult {
    bool have_record{false};           // game+5FCh non-null
    std::int32_t selected_index{0};    // game+60Ch
    std::uint8_t mission_id_byte{0};   // game+2015h
    std::int32_t resolved_script_slot{0}; // game+2028h
    SceneSlotRecord slots[kSceneSlotRecordCount]{};
    std::size_t slots_filled{0};       // min(record+988h, 8)
    bool leaderboard_published{false}; // the 00f8a2fc +178h/+17Ch pair
    bool network_slot_refresh{false};  // 004bc890, session_mode != 0
};

// Inputs 004c6890 reads off the game object that are not in the record.
struct SelectSceneRecordInputs {
    std::int32_t session_mode{0};      // +1FE4h
    std::int32_t script_slot{0};       // +614h
    bool script_slot_forced{false};    // +61Ch
};

// 004c6890. Native __thiscall void(GGame* this, int index), ECX = game, one
// stack argument, RET 4 at 004c6afa.
//
// index is clamped to 0 when the list is empty, when it is negative, or when it
// is greater than the list count - note the comparison at 004c68a7 is `count <
// index`, so index == count is accepted and walks off the end of the list, in
// which case the walk yields a null record. The record found at that position
// becomes game+5FCh, the override string at game+600h/604h is emptied, and the
// eight slot records are refilled from the record's side blocks. Slots beyond
// record+988h have their in-use byte cleared and nothing else.
//
// The script-slot rule at 004c6944 is byte-identical to the one 004dfb70 uses
// at 004e087b; this reuses normalise_mission_script_slot from
// bsp/mission_scene_load.hpp rather than restating it.
SelectSceneRecordResult run_select_scene_record_004c6890(
    const std::vector<SceneRecord>& records, std::int32_t index,
    const SelectSceneRecordInputs& inputs);

// ---------------------------------------------------------------------------
// 004e2770 BSP_Game_SetPendingScene
// ---------------------------------------------------------------------------

// The empty literal at 00ce3a0c that 004e2770, 004c6890 and 004c6a66 all use to
// clear a string.
inline constexpr const char* kEmptyStringLiteral = ""; // 00ce3a0c

// One method per native call site of 004e2770 and the part of 004e1d70 that
// reaches outside the record. No default implementations.
struct SetPendingSceneHost {
    virtual ~SetPendingSceneHost() = default;

    // 004bf930 with ECX = game+5F0h. Pops every node, calls the record's
    // virtual +0h with 1 (delete), unlinks and frees the node.
    virtual void destroy_scene_record(SceneRecord& record) = 0;

    // 004e1d70's file block: BSP_FileBlock_Construct at 004e1e0c with the name
    // built from the prefix at 00ce7f0c and derive_scene_short_name, destroyed
    // at 004e21de.
    virtual void enter_file_block(const std::string& name) = 0;
    virtual void leave_file_block() = 0;

    // 004e1d70 at 004e1fbd, the VFS provider table 0109ceec virtual +8h with
    // the scene path. Only the reuse arm stores the answer at record+914h.
    virtual bool scene_file_exists(const std::string& scene_path) = 0;

    // 0046df00 with (path, 0, 0, record, override, 0) - the header pass of
    // bsp/scene_file.hpp, which fills the record the walk just allocated.
    virtual void load_scene_header_pass(const std::string& scene_path,
        const std::string& weather_override, SceneRecord& record) = 0;

    // 004e21c6 then 004e21cd: 004e1ca0 returns the award tracker singleton and
    // 0068ea00 runs on it. Only the create arm reaches these.
    virtual void notify_award_tracker() = 0;
};

// Which arm of 004e1d70 ran. 004e2770 always passes 0, so the mission path
// always takes Create; the reuse arm belongs to the console and session callers
// (004e2200, 004e27e0, 0076e710, 0076fad0).
enum class SceneRecordBuildArm {
    Create,        // argument 2 == 0: allocate unconditionally, header pass
                   // unconditionally, append, notify the award tracker
    ReuseExisting, // argument 2 != 0: reject a duplicate scene path, probe the
                   // VFS into record+914h, header pass only when it is set
};

struct SetPendingSceneResult {
    std::size_t records_destroyed{0};
    bool record_created{false};     // false on the reuse arm's duplicate reject
    SelectSceneRecordResult selection{};
};

// 004e1d70. Native __thiscall int(GGame* this, const char* scene_path,
// char arm, int unused, const char* weather_override), ECX = game, four stack
// arguments, RET 10h. Returns 1 except on the reuse arm's duplicate reject,
// which returns 0. The third stack argument is pushed as -1 by 004e2770 and is
// never read by the body - see the Uncertainties section of the doc.
bool run_build_scene_record_004e1d70(std::vector<SceneRecord>& records,
    const std::string& scene_path, SceneRecordBuildArm arm,
    const std::string& weather_override, SetPendingSceneHost& host);

// 004e2770. Native __thiscall void(GGame* this, const char* scene_path,
// const char* weather_override), ECX = game, two stack arguments, RET 8 at
// 004e27d2. The second argument is a string pointer that reaches 0046df00 as
// its override-name argument, not a flag word; the mission-tree caller pushes
// null for it.
//
// Order: clear the record list, null game+5FCh, empty the override string,
// build the record with the Create arm, then select index 0.
SetPendingSceneResult run_set_pending_scene_004e2770(std::vector<SceneRecord>& records,
    const std::string& scene_path, const std::string& weather_override,
    const SelectSceneRecordInputs& inputs, SetPendingSceneHost& host);

// ---------------------------------------------------------------------------
// 00439020 BSP_Game_RequestMissionStart
// ---------------------------------------------------------------------------

// 00439020 is __cdecl void(), seven instructions: 004d7920(game, 6) then
// 004d7920(game, 0Ah). Both go onto the request queue of
// bsp/game_frame_control.hpp, and the drain re-reads the count, so both are
// serviced in one pass - the interface change first, then the load.
inline constexpr std::uint32_t kMissionStartInterfaceRequest = 0x06;
const std::uint32_t* mission_start_state_requests_00439020(std::size_t& count) noexcept;

// ---------------------------------------------------------------------------
// 005c5600 BSP_MissionTree_StartSelectedMission
// ---------------------------------------------------------------------------

// Offsets into the mission-tree record 005c3870 returns. Only what 005c5600
// reads is listed.
inline constexpr std::size_t kMissionLoadRecordTitleOffset = 0x0;    // native string pair
inline constexpr std::size_t kMissionRecordSubtitleOffset = 0x8; // native string pair
// The native string pair sits at +20h; 005c5670 loads its data pointer from
// +24h and passes that, so an empty string with a null buffer is substituted at
// 005c5677 by the empty literal below.
inline constexpr std::size_t kMissionRecordScenePathOffset = 0x20;
inline constexpr std::size_t kMissionRecordSideByteOffset = 0xb8;
inline constexpr std::size_t kMissionRecordSideBlockBase = 0xb8;
inline constexpr std::size_t kMissionRecordSideBlockStride = 0x154;
inline constexpr std::size_t kMissionRecordBriefingOffset = 0x4;  // inside the side block
inline constexpr std::size_t kMissionRecordHintsOffset = 0x64;    // inside the side block
inline constexpr std::size_t kMissionRecordLoadingByteOffset = 0x20c;
inline constexpr std::size_t kMissionRecordModeOffset = 0xb4;

// The fallback scene-path pointer at 005c5677 when the record's own pointer is
// null: the empty string at 00e1952f.
inline constexpr std::uint32_t kEmptyScenePathAddress = 0x00e1952f;

// The mode value at record+0B4h that means "inherit": 005c5772 substitutes
// game+6B0h for it before storing into game+6ACh.
inline constexpr std::int32_t kMissionModeInherit = 3;

// What the mission-tree record supplies. `side_flag` is the byte at +0B8h; the
// side index the routine computes is (side_flag == 0), so a zero byte selects
// side block 1.
struct MissionTreeSelection {
    std::string title;        // record+0h
    std::string subtitle;     // record+8h
    std::string scene_path;   // record+24h, empty when the pointer was null
    std::uint8_t side_flag{0};// record+0B8h
    bool briefing_present{false}; // side block +4h non-null
    std::int32_t mode{0};     // record+0B4h
    std::int32_t inherited_mode{0}; // game+6B0h, used when mode == 3
};

// What 005c5600 decides after the pending scene is set. Exactly one of the two
// booleans is true: a mission whose side block carries briefing data opens the
// briefing screen and stops, everything else queues the two requests.
struct MissionTreeStartDecision {
    std::int32_t side_index{0};     // (side_flag == 0)
    bool show_briefing{false};      // 0051dce0 then 004cc460(1, 0)
    bool queue_mission_start{false};// 00439020
    std::int32_t stored_mode{0};    // what lands in game+6ACh
};

// 005c5600. Native __thiscall void(MissionTreeScreen* this), ECX only, RET with
// no immediate, SEH frame (handler 00c72dd8); Ghidra renders it __cdecl(void).
// The routine copies the record title to game+2198h and game+6B8h, sets the
// pending scene with a null override, hands the loading screen its text, runs
// 00626930 with the side, and then branches.
MissionTreeStartDecision decide_mission_tree_start_005c5600(
    const MissionTreeSelection& selection) noexcept;

// ---------------------------------------------------------------------------
// The stage machine
// ---------------------------------------------------------------------------

// Where the load has got to, in the order the frames reach it. These are not
// native values: game+5D4h only distinguishes the last three.
enum class MissionLoadStage {
    Idle,          // no pending scene
    SceneSelected, // 004e2770 ran, nothing queued yet
    Requested,     // 6 and 0Ah are on the queue, the drain has not run
    Loading,       // the drain is inside 004dfb70
    SceneReady,    // game+5D4h == 0Ch, waiting on 004db920
    InMission,     // game+5D4h == 0Dh, 004da6c0 finished
};
const char* mission_load_stage_name(MissionLoadStage stage) noexcept;

// 004db920 is dispatched from the frame loop only while game+5D4h is 0Ch, and
// 004da6c0 writes 0Dh; these two are the only values of game+5D4h the load path
// leaves behind.
MissionLoadStage mission_load_stage_for_state(std::uint32_t game_state,
    bool requests_pending, bool have_pending_scene) noexcept;

// What one frame of the path did. `stage` is where the frame left it.
struct MissionLoadFrameResult {
    MissionLoadStage stage{MissionLoadStage::Idle};
    bool ran_scene_load{false};   // 004dfb70
    bool ran_device_wait{false};  // 004db920
    bool entered_mission{false};  // 004da6c0 wrote 0Dh
};

// Everything the path carries between frames.
struct MissionLoadPathState {
    std::vector<SceneRecord> records;     // game+5F0h
    SelectSceneRecordResult selection{};  // game+5FCh and the slot array
    std::vector<std::uint32_t> requests;  // game+5D8h, front at index 0
    bool have_pending_scene{false};
    DeviceWaitLatch device_wait_latch{};
};

// Everything one frame needs to run the load. The three sub-drivers are the
// existing reconstructions; this only sequences them.
struct MissionLoadPathFrame {
    MissionSceneLoadState* scene_load{nullptr};
    MissionSceneLoadHost* scene_load_host{nullptr};
    MissionStateEntryState* state_entry{nullptr};
    const AudioSettings* audio_settings{nullptr};
    MissionStateEntryHost* state_entry_host{nullptr};
    MissionDeviceWaitHost* device_wait_host{nullptr};
    MissionDeviceWaitInputs device_wait_inputs{};
};

// Runs 005c5600's decision and, when it says so, queues the two requests. This
// is the only entry point into the path from the front end.
MissionTreeStartDecision start_mission_from_tree(MissionLoadPathState& state,
    const MissionTreeSelection& selection, const SelectSceneRecordInputs& inputs,
    SetPendingSceneHost& host);

// One frame. Drains the pending requests the path owns (6 is handed back to the
// caller through the result of start_mission_from_tree, 0Ah and 0Bh run
// run_mission_scene_load), then services state 0Ch. It deliberately does not
// implement the rest of the drain at 004e4430: a request this path does not own
// is left on the queue for the frame-control driver.
MissionLoadFrameResult advance_mission_load_path(MissionLoadPathState& state,
    MissionLoadPathFrame& frame);

// ---------------------------------------------------------------------------
// The host-method inventory
// ---------------------------------------------------------------------------

// Who has to supply a host method before the rebuilt executable can run the
// load. PureLogic means the reconstruction already decides it and the method
// only records the effect; the rest name the subsystem whose owner has to
// provide real behaviour.
enum class MissionLoadOwner {
    PureLogic,
    Vfs,
    Renderer,
    SceneGraph,
    World,
    Lua,
    Audio,
    Input,
    Gui,
    Session,
};
const char* mission_load_owner_name(MissionLoadOwner owner) noexcept;

// One host method the rebuilt executable must implement, in the order the load
// reaches it. `host` names the interface the method belongs to, `method` the
// member, and `address` the native call site or callee the method stands for.
struct MissionLoadHostStep {
    std::uint32_t address;
    const char* host;
    const char* method;
    MissionLoadOwner owner;
    const char* note;
};
const MissionLoadHostStep* mission_load_host_steps(std::size_t& count) noexcept;
// Count of the steps whose owner is not PureLogic.
std::size_t mission_load_external_step_count() noexcept;

} // namespace bsp
