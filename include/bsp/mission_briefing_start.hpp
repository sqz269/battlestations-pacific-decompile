// From the mission tree, through the briefing surface, to the request pair the
// load path consumes.
//
// Addresses: 005C5600 0051DCE0 004CC460 00626930 00580940 005806A0 005C27E0
//            005C3850 005C3BE0 0058C010 005922F0 0058BDF0 0058D9D0 0051B7B0
//
// docs/MISSION_BRIEFING_START.md carries the evidence. Every name here is a
// hypothesis, not a recovered symbol, and no struct is binary compatible: the
// offsets are comments and k*Offset constants.
//
// The shape of the path, established by this packet:
//
//   005C57D0  the mission tree's accept action, gated on the unlock test
//   005C5600  sets the pending scene, publishes the loading text, resets the
//             per-mission statistics, and then branches:
//               side block +4h empty -> 00439020 straight away
//               side block +4h set   -> 0051DCE0 fills the briefing screen
//                                       object and 004CC460 raises interface 1
//   0058C010  the main-menu screen's page builder mirrors the mission-tree
//             selection into 00E194D8/00E194DC and shows MissionDetail
//   005922F0  the play action of that page: with a MovieName it plays the
//             movie and hands 0058D9D0 the completion, otherwise it calls
//             0058BDF0 directly
//   0058BDF0  re-sets the pending scene from the mirrored selection and calls
//             00439020, which is what bsp/mission_load_path.hpp consumes
//
// The briefing screen class's own play action, 0051B7B0, is in the image but
// unreachable; see kBriefingActivatePlayIsUnreferenced below.

#ifndef BSP_MISSION_BRIEFING_START_HPP
#define BSP_MISSION_BRIEFING_START_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "bsp/mission_tree_data.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Field offsets this packet adds
// ---------------------------------------------------------------------------

// Mission record, 434h. Both are already named in kMissionRecordSchema; the
// constants exist because this path reads them off the record directly.
inline constexpr std::size_t kMissionRecordDebriefingTextOffset = 0x38; // 00626970
inline constexpr std::size_t kMissionRecordMovieNameOffset = 0x58;     // 00592320

// The front-end manager at 00E198AC. +58h is the main-menu screen (005902E0,
// interface id 1), +5Ch the mission-tree screen, +64h the briefing screen.
inline constexpr std::size_t kFrontEndManagerMainMenuScreenOffset = 0x58;   // 0058BF64
inline constexpr std::size_t kFrontEndManagerMissionTreeScreenOffset = 0x5C; // 0058C06B
inline constexpr std::size_t kFrontEndManagerMoviePlayerOffset = 0x50;      // 00592339

// Main-menu screen, 578h. The byte at +5Ch gates two things in 0058BDF0: the
// player-difficulty inheritance and the checkpoint write. Its meaning is not
// recovered, so it is carried as the raw flag it is.
inline constexpr std::size_t kMainMenuScreenFlag5cOffset = 0x5C; // 0058BF48

// Profile block, game+650h. 007F8D60 looks the record up in the container at
// profile+94h and compares the value it finds with the dword at profile+98h.
inline constexpr std::size_t kProfileCheckpointMapOffset = 0x94;   // 007F8D72
inline constexpr std::size_t kProfileCheckpointValueOffset = 0x98; // 007F8D66

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------

// The main menu's own copy of the mission-tree selection. 0058C010 writes both
// (0058C071, 0058C08D) and 005806A0 reads both (005806B3, 005806E6), so the
// mission-detail page and everything it starts run off these, not off the
// mission-tree screen's +0Ch/+10h.
inline constexpr std::uint32_t kMainMenuSelectedGroupAddress = 0x00E194D8;
inline constexpr std::uint32_t kMainMenuSelectedMissionAddress = 0x00E194DC;
// 0058099x: the page dword and the sub-selection 00580940 resets to -1.
inline constexpr std::uint32_t kMainMenuPageAddress = 0x00E08874;
inline constexpr std::uint32_t kMainMenuSubSelectionAddress = 0x00E08878;

// The two strings 00626930 publishes: the mission's Lua `name` and its
// `debriefingText`. Nothing in the image reads either by a direct reference.
inline constexpr std::uint32_t kMissionStatsMissionNameAddress = 0x00E19798;
inline constexpr std::uint32_t kMissionStatsDebriefingTextAddress = 0x00E197A0;

// The metrics gate and sink of 0058BDF0's tail.
inline constexpr std::uint32_t kMissionStartMetricsEnabledAddress = 0x00E1AED4;
// 0058BFD6: (page == Page08) ? 5 : 1.
inline constexpr int kMissionStartMetricsCodeDefault = 1;
inline constexpr int kMissionStartMetricsCodePage08 = 5;

// The movie player singleton pointer 005922F0 drives.
inline constexpr std::uint32_t kMoviePlayerSingletonAddress = 0x00E18D48;
// 0059236F: the float 004F8A20 takes as its third argument.
inline constexpr std::uint32_t kMissionMovieFadeConstantAddress = 0x00CEFCB8;
// The completion routine 004F8970 is handed at 0059239C.
inline constexpr std::uint32_t kMissionMovieCompletionAddress = 0x0058D9D0;

// 0051B7B0 has no reference of any kind in the image: no relative CALL or JMP
// anywhere in .text reaches 0051B7B0..0051B88C, and the four bytes B0 B7 51 00
// do not occur in the file, so no vtable, table or immediate holds its address
// either. It is compiled-in but uncalled; the reachable play action is 005922F0
// on the main-menu screen.
inline constexpr bool kBriefingActivatePlayIsUnreferenced = true;

// ---------------------------------------------------------------------------
// 00626930, the per-mission statistics reset
// ---------------------------------------------------------------------------

// How each container in the block at 00E196CC..00E19797 is cleared. The three
// callees are distinguished only by address; what they erase is not recovered
// beyond "the whole container", which is what the (begin, end) pair the caller
// builds from the container's own _Myfirst/_Mylast means.
enum class MissionStatsContainerKind {
    EraseAll004954F0,  // erase(begin, end) on a vector, 004954F0
    Reset005F6190,     // a 14h object reset in place, 005F6190
    EraseAll006226F0,  // erase(begin, end) on a vector of another element, 006226F0
};

struct MissionStatsContainer {
    std::uint32_t address;
    MissionStatsContainerKind kind;
};

// In the order 00626930 runs them.
inline constexpr std::array<MissionStatsContainer, 12> kMissionStatsContainers{{
    {0x00E196CC, MissionStatsContainerKind::EraseAll004954F0}, // sweep from 006269A9
    {0x00E196DC, MissionStatsContainerKind::EraseAll004954F0}, // 00626A31
    {0x00E196EC, MissionStatsContainerKind::EraseAll004954F0}, // 00626A77
    {0x00E196FC, MissionStatsContainerKind::Reset005F6190},    // 00626A81
    {0x00E19710, MissionStatsContainerKind::Reset005F6190},    // 00626A8B
    {0x00E19724, MissionStatsContainerKind::Reset005F6190},    // 00626A95
    {0x00E19738, MissionStatsContainerKind::EraseAll006226F0},
    {0x00E19748, MissionStatsContainerKind::EraseAll006226F0},
    {0x00E19758, MissionStatsContainerKind::EraseAll006226F0},
    {0x00E19768, MissionStatsContainerKind::EraseAll004954F0},
    {0x00E19778, MissionStatsContainerKind::EraseAll004954F0},
    {0x00E19788, MissionStatsContainerKind::EraseAll004954F0},
}};

// Integration boundary for 00626930, one method per native call site.
struct MissionStatsResetHost {
    virtual ~MissionStatsResetHost() = default;
    // 0062694D / 00626968: the string at 00E19798 takes record+08h.
    virtual void set_stats_mission_name(std::string_view name) = 0;
    // 00626985 / 006269A1: the string at 00E197A0 takes record+38h.
    virtual void set_stats_debriefing_text(std::string_view text) = 0;
    // 004954F0, 005F6190 and 006226F0 in the table order above.
    virtual void clear_stats_container(const MissionStatsContainer& container) = 0;
};

// 00626930. Native __fastcall void(MissionRecord* record), ECX only, RET.
// 005C5732 and 0058BF1D load EDX with the side index before the call and the
// body never reads EDX, so the `side` argument earlier documentation gives this
// routine does not exist.
void run_mission_stats_reset_00626930(
    const MissionRecordData& record, MissionStatsResetHost& host);

// ---------------------------------------------------------------------------
// 00580940, publishing the mission-tree selection to the main menu
// ---------------------------------------------------------------------------

struct MainMenuSelectionInputs {
    bool non_campaign_session{false}; // game+1FE4h; non-zero writes nothing
    std::uint32_t selected_group{};   // mission-tree screen +10h
    std::uint32_t mission_index{};    // 005C3850, the index of the record's id
    bool group_completed{false};      // 005C3BE0
    std::size_t side_index{};         // 005C27E0
};

struct MainMenuMissionSelection {
    bool published{false};          // false: game+1FE4h was set, nothing written
    std::uint32_t group_index{};    // 00E194D8
    std::uint32_t mission_index{};  // 00E194DC
    std::int32_t sub_selection{-1}; // 00E08878
    MainMenuPage page{MainMenuPage::TopLevel}; // 00E08874
};

// 00580940. Native __cdecl void(), RET. Order: the two indices, then the -1,
// then one of four page outcomes. Group 0 and a fully completed group return
// before the side index is read.
MainMenuMissionSelection publish_main_menu_mission_selection_00580940(
    const MainMenuSelectionInputs& inputs) noexcept;

// 005806A0. Native __cdecl MissionRecord*(), ECX dead: it indexes
// groups[00E194D8].missions[00E194DC] of the screen at [00E198AC]+5Ch with the
// iterator-debug bound checks that trap through 00BF6713. Returns nullptr where
// the native code would trap.
const MissionRecordData* selected_mission_005806a0(const MissionTreeTables& tables,
    std::uint32_t group_index, std::uint32_t mission_index) noexcept;

// 005C27E0. Native __fastcall bool(MissionRecord*), the whole body being
// `return record[0B8h] == 0`. The same rule mission_side_index already states
// for the screen-side record; this is the standalone routine the main-menu path
// calls, so it is spelled out against the same record.
std::size_t mission_side_index_005c27e0(const MissionRecordData& record) noexcept;

// 005C3BE0. Native __thiscall bool(MissionTreeScreen*, const string* id):
// resolves the id through 005C3470, then walks that group and returns true only
// when BSP_MissionProgress_IsCompleted holds for every mission in it. An empty
// group is true, because the walk never runs. `mission_completed` carries one
// flag per mission of the group, in the group's own order; a short vector is
// treated as "not completed", which is the conservative reading.
bool group_completed_005c3be0(const MissionGroupData& group,
    const std::vector<bool>& mission_completed) noexcept;

// ---------------------------------------------------------------------------
// 005922F0, the play action of the mission-detail page
// ---------------------------------------------------------------------------

struct MissionStartHost;

struct MissionBriefingPlayHost {
    virtual ~MissionBriefingPlayHost() = default;
    // 0059230D then 00592314: the award tracker singleton, then 00690CD0.
    virtual void flush_award_tracker() = 0;
    // 0059232F, ECX = this: 005830A0 on the main-menu screen.
    virtual void suspend_page_for_movie() = 0;
    // 0059233C, ECX = [00E198AC]+50h.
    virtual void stop_front_end_audio() = 0;
    // 0059234C: the manager's own vtable +0Ch.
    virtual void apply_pending_interface() = 0;
    // 0059235B / 0059235E: the two bytes at movie+4h and movie+5h.
    virtual void arm_movie_surface() = 0;
    // 00592361, ECX = [00E18D48]: BSP_FrontEndScreen_CommitVisibility.
    virtual void commit_movie_visibility() = 0;
    // 0059236D: the movie object's own vtable +18h.
    virtual void enter_movie_surface() = 0;
    // 00592383: 004F8A20(&record+58h, 1, [00CEFCB8], 0).
    virtual void play_mission_movie(std::string_view movie_name) = 0;
    // 0059238E: the byte at movie+30h.
    virtual void mark_movie_active() = 0;
    // 0059239C: 004F8970 with the address of 0058D9D0.
    virtual void set_movie_completion(std::uint32_t completion_address) = 0;
    // 005923B3: 0058BDF0.
    virtual void start_selected_mission() = 0;
    // 005923B8..0059244C: five empty strings through 0054B530.
    virtual void clear_help_line() = 0;
};

// 005922F0. Native __thiscall void(MainMenuScreen* this), ECX only, RET, SEH
// frame (handler 00C70948). Returns true when it took the movie arm, which
// leaves the start to the completion callback.
bool run_briefing_play_005922f0(
    const MissionRecordData& record, MissionBriefingPlayHost& host);

// 0058D9D0, three instructions: ECX = [00E198AC]+58h, then JMP 0058BDF0. The
// ECX it loads is dead in the callee; the routine exists only to give
// 004F8970 a plain function pointer.
void run_briefing_movie_finished_0058d9d0(
    const MissionRecordData& record, MissionStartHost& host);

// ---------------------------------------------------------------------------
// 0058BDF0, starting the selected mission
// ---------------------------------------------------------------------------

// What the routine leaves behind, for a caller that wants to check the decision
// rather than the host trace.
struct MissionStartOutcome {
    bool difficulty_written{false};  // game+6ACh was stored to
    std::int32_t effective_difficulty{0};
    bool checkpoint_written{false};  // 00437C70 ran
    bool metrics_reported{false};
    int metrics_code{kMissionStartMetricsCodeDefault};
};

struct MissionStartHost {
    virtual ~MissionStartHost() = default;
    // 0058BE40: the string at game+2198h takes record+00h, the Lua id.
    virtual void set_current_mission_key(std::string_view mission_id) = 0;
    // 0058BE70: 004E2770(scene, null). The second argument is the weather
    // descriptor override, and this caller passes none.
    virtual void set_pending_scene(
        std::string_view scene, std::string_view weather_override) = 0;
    // 0058BEDE: 0057D060 with the side block's +64h text vector, the record
    // name at +08h and the low byte of record+20Ch.
    virtual void publish_loading_config(const std::vector<std::string>& text,
        std::string_view mission_name, bool second_side_enabled) = 0;
    // 0058BEFA: the string at game+6B8h takes record+00h again.
    virtual void set_mission_key_mirror(std::string_view mission_id) = 0;
    // 0058BF21: 00626930.
    virtual void reset_mission_stats(const MissionRecordData& record) = 0;
    // The byte at main-menu screen +5Ch, read at 0058BF48, 0058BF67 and 0058BFC5.
    virtual bool main_menu_flag_5c() = 0;
    // game+6B0h, read at 0058BF52.
    virtual std::int32_t chosen_difficulty() = 0;
    // game+6ACh, written at 0058BF37 or 0058BF58.
    virtual void set_effective_difficulty(std::int32_t value) = 0;
    // 0058BF79: 007F8D60(ECX = game+650h, record). True when the profile's
    // checkpoint value for this record differs from the one at profile+98h.
    virtual bool checkpoint_differs(const MissionRecordData& record) = 0;
    // 0058BF82 then 0058BF89: the profile manager singleton, then 00437C70,
    // the BSP_Chk_Save write.
    virtual void write_checkpoint() = 0;
    // 0058BF8E: 00439020, the request pair bsp/mission_load_path.hpp consumes.
    virtual void request_mission_start() = 0;
    // 0058BF99 then 0058BFA0: 004BEC00 then 00A92C40 with 0.0f.
    virtual void reset_front_end_timer() = 0;
    // 00E1AED4, read at 0058BFA5.
    virtual bool metrics_enabled() = 0;
    // game+1FE4h, read at 0058BFB4.
    virtual bool non_campaign_session() = 0;
    // 00E08874, read at 0058BFCB.
    virtual MainMenuPage current_page() = 0;
    // 0058BFDE: 00753810(code, flag).
    virtual void report_mission_start_metrics(int code, bool flag) = 0;
    // 0058BFEF: 004D2A80 on the frame temporary.
    virtual void finish_start() = 0;
};

// 0058BDF0. Native __thiscall void(MainMenuScreen* this), ECX dead, RET, SEH
// frame (handler 00C700C8). This is the second pass over 004E2770 in a briefing
// start: 005C5600 already built a scene record, and 004E2770 destroys it before
// building the new one.
MissionStartOutcome run_start_selected_mission_0058bdf0(
    const MissionRecordData& record, MissionStartHost& host);

// ---------------------------------------------------------------------------
// 0051B7B0, the unreferenced briefing play action
// ---------------------------------------------------------------------------

// The three difficulty widgets at briefing +9Ch, +0A0h and +0A4h, in the order
// 0051B7B0 tests them; the index of the match is the value stored.
inline constexpr std::size_t kBriefingDifficultyWidgetCount = 3;

struct BriefingActivatePlayHost {
    virtual ~BriefingActivatePlayHost() = default;
    // 0051B7BA, ECX = briefing+98h: the active widget.
    virtual const void* active_widget() = 0;
    // 0051B7C1 / 0051B7DB / 0051B7FD: briefing+9Ch, +0A0h, +0A4h.
    virtual const void* difficulty_widget(std::size_t index) = 0;
    // 0051B7D5 / 0051B7D8 and the two arms after: profile+60h then profile+5Ch,
    // which are game+6B0h and game+6ACh.
    virtual void set_difficulty_pair(std::int32_t value) = 0;
    // 0051B820: briefing+90h.
    virtual void clear_active_page() = 0;
    // 0051B82C / 0051B83A / 0051B846 / 0051B855: virtual +34h then +60h(0) on
    // briefing+98h, +60h(1) on briefing+6Ch and on briefing+124h.
    virtual void refresh_widget_visibility() = 0;
    // 0051B861: the byte at [briefing+124h]+77h.
    virtual void clear_item_disabled_byte() = 0;
    // 0051B866: 00439020.
    virtual void request_mission_start() = 0;
    // 0051B871: 0051B450(false, false).
    virtual void rebuild_help_line() = 0;
    // 0051B87C then 0051B883: 004BEC00 then 00A92C40 with 0.0f.
    virtual void reset_front_end_timer() = 0;
};

// 0051B7B0. Native __thiscall void(BriefingScreen* this, bool start_now),
// one stack argument, RET 4. The selector at 0051B857 is `CMP byte [ESP+0Ch],
// BL` against the two saved registers plus the return address, i.e. the stack
// argument: zero rebuilds the help line, non-zero queues the mission start.
// Reconstructed for the record only - nothing calls this routine.
void run_briefing_activate_play_0051b7b0(bool start_now, BriefingActivatePlayHost& host);

// ---------------------------------------------------------------------------
// What a rebuilt executable has to supply
// ---------------------------------------------------------------------------

struct MissionBriefingHostStep {
    std::uint32_t call_site;     // the native address of the call
    std::uint32_t callee;        // 0 when the site is a store, not a call
    std::string_view host;       // the interface that owns the method
    std::string_view method;     // the pure virtual
    std::string_view owner;      // which subsystem has to supply it
};

// Every host method this packet's routines need, in the order the native code
// runs them across 005922F0, 0058BDF0 and 00626930.
const MissionBriefingHostStep* mission_briefing_host_steps(std::size_t& count) noexcept;
}

#endif
