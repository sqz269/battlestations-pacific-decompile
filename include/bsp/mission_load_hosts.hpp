#pragma once

// Seven mission-load steps that docs/GAME_EXECUTABLE.md still reports as
// unimplemented MissionSceneLoadHost methods. Packet cc2_mission_load_hosts,
// worktree agent/cc2-mission-load-hosts. Ghidra was read-only for this packet;
// every name below is a hypothesis, not a recovered symbol.
//
// Evidence and per-step field tables: docs/MISSION_LOAD_HOSTS.md.
// Driver and host interface: include/bsp/mission_scene_load.hpp.
//
// Two of the seven already have reconstructions and are NOT repeated here:
//   release_deferred_dynamics      00447060 -> release_all_dynamics_00447060
//   check_multiplayer_player_count 004D87B0 -> run_mission_player_count_check_004d87b0
// both in include/bsp/mission_state_frame.hpp. In a local single-player session
// 004D87B0 returns at 004D87C9 because game+1FE4h is 0, so its outcome there is
// MissionPlayerCountOutcome::kNotRun and the step has no local effect.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// Step 1. 004C3840 BSP_Game_AssignPartyPlayerSlots
// __thiscall void(GGame* /*ECX*/, char skipMarkedCandidates), RET 4,
// body 004C3840..004C3A6x. The load path calls it at 004E044D with argument 0
// and only when game+1FE4h == 1.
// ---------------------------------------------------------------------------

// 004C395F CMP ECX,7 / JA: an owner slot index is valid only below 8.
inline constexpr std::uint32_t kPlayerSlotLimit = 8;
// The roster frame is three counts at ESP+20h, three cursors at ESP+2Ch and a
// 3x8 int table at ESP+5Ch (004C38BF LEA EAX,[EDX+EAX*8], 004C38C5).
inline constexpr std::size_t kPartyCount = 3;
inline constexpr std::size_t kPartyRosterStride = 8;
// 004C39A2 / 004C39A0: the two words pushed into the entity's vtable +148h when
// the unit's party has no bound player at all. Tokens, not slot indices.
inline constexpr int kUnboundOwnerTokenA = 0x1FF;
inline constexpr int kUnboundOwnerTokenB = 8;
// 00CE75E4, the only literal in the routine. 004C39D7 ADD ESP,14h proves five
// dwords: the format plus name, party, old slot, new slot.
inline constexpr const char* kPartyReassignLogFormat =
    "%s(party %d player %d)=>player %d";
// 004C3914 / 004C3923, the two vtable +5Ch (IsKindOf) class ids that exempt a
// candidate from the marker skip. Only consulted when the argument is non-zero.
inline constexpr int kPartyReassignExemptKindA = 0x1C;
inline constexpr int kPartyReassignExemptKindB = 0x09;
// 0075B430 with 53h at 004C39DA: the session message kind the reassignment
// routes. The new slot lands at message+20h (004C3A04).
inline constexpr int kOwnerChangeMessageKind = 0x53;

// One entry of the eight-pointer array at game+18CCh, as this routine reads it.
// The party id at +28h is the same field docs/OBJECTIVE_UNIT_LIST.md and
// 004D87B0's side walk read, and it indexes a three-slot frame array unchecked.
struct MissionSlotBinding {
    bool present_08{};  // +8h,  004C38A2
    bool flag_09{};     // +9h,  004C38A8
    bool flag_0a{};     // +0Ah, 004C38AE
    int party_28{};     // +28h, 004C38B4
};

// 004C38A2..004C38B2. A slot is enrolled in its party roster when +8h is set and
// either +9h is clear or +0Ah is set. 004C396F..004C3983 is the exact negation,
// so "bound" and "vacant" partition the eight slots with no third case.
bool mission_slot_is_bound(const MissionSlotBinding& slot) noexcept;

// The stack frame 004C3896..004C38D3 builds, in slot order.
struct MissionPartyRoster {
    int member_count[kPartyCount]{};
    int cursor[kPartyCount]{};
    int members[kPartyCount][kPartyRosterStride]{};
};

// Walks slots 0..count-1 and appends each bound slot's index to its party's
// roster. count comes from the scene record ([[00E188A8]+5FCh]+988h, 004C3884),
// not from kPlayerSlotLimit. A party id outside [0,3) would index the frame out
// of bounds natively; this reconstruction drops such a slot instead, and that
// deviation is deliberate.
MissionPartyRoster build_mission_party_roster(const MissionSlotBinding* slots,
                                              std::size_t count) noexcept;

// One element of the intrusive list at 00F87198, as 004C3931..004C3983 reads it.
// The field meanings for +5Ch, +5Dh and +5Eh are docs/UNIT_INSTANCE_LAYOUT.md.
struct PartyOwnerCandidate {
    bool active_5c{};              // +5Ch, the live gate; must be set
    bool flag_5d{};                // +5Dh, must be clear
    bool flag_60{};                // +60h, must be clear
    bool dead_5e{};                // +5Eh, must be clear
    std::uint32_t owner_slot_188{};// +188h, the owning player slot
};

// 004C3931..004C3962, the four byte gates and the unsigned slot bound.
bool candidate_is_reassignable(const PartyOwnerCandidate& candidate) noexcept;

enum class PartyReassignAction : std::uint8_t {
    kSkip,          // a gate rejected the candidate; nothing happens
    kUnbindOwner,   // 004C3998, vtable +148h(1FFh, 8): the party has no player
    kReassign,      // 004C39B0, the round-robin message
};

struct PartyReassignDecision {
    PartyReassignAction action{PartyReassignAction::kSkip};
    int party{-1};
    std::uint32_t from_slot{};
    int to_slot{-1};
};

// The per-candidate body. A candidate whose owner slot is still bound is
// skipped; otherwise the vacated slot's own party (slot+28h) selects the
// roster. An empty roster unbinds the owner; a non-empty one hands out
// members[party][cursor] and advances the cursor modulo the member count
// (004C3A1D LEA EAX,[EBP+1] / CDQ / IDIV, a signed remainder).
PartyReassignDecision decide_party_reassignment(const PartyOwnerCandidate& candidate,
                                                const MissionSlotBinding* slots,
                                                std::size_t slot_count,
                                                MissionPartyRoster& roster) noexcept;

struct AssignPartyPlayerSlotsHost {
    virtual ~AssignPartyPlayerSlotsHost() = default;

    // 004C3884, [[00E188A8]+5FCh]+988h, the scene record's side block count.
    virtual std::size_t scene_slot_count() = 0;
    // 004C38A0, game+18CCh + index*4.
    virtual MissionSlotBinding slot(std::size_t index) = 0;

    // The 00F87198 list. Its producer was not found: the only referencing
    // functions are this routine, 004C3A80, 004D56E0, 0076F2B0, 0077C540,
    // 0077E4C0, 0077F0E0 and 0077F5E0, all of them readers. contract: unread.
    virtual std::size_t candidate_count() = 0;
    virtual PartyOwnerCandidate candidate(std::size_t index) = 0;

    // 004C38FA, [entity+304h] against the float at 00D7A218. Only consulted
    // when the routine's argument is non-zero; the load path passes 0.
    virtual bool candidate_marker_matches(std::size_t index) = 0;
    // 004C3918 / 004C3927, vtable +5Ch IsKindOf(classId), RET 4
    // (docs/LOCAL_PLAYER_UNIT_LISTS.md).
    virtual bool candidate_is_kind(std::size_t index, int class_id) = 0;

    // 004C39CA, vtable +10h, __thiscall with no stack argument; its result is
    // the %s of kPartyReassignLogFormat. The override was not read, so only
    // "it yields the name the log prints" is established. contract: unread.
    virtual const char* candidate_name(std::size_t index) = 0;
    // 004C39A9, vtable +148h(1FFh, 8), __thiscall, two stack arguments.
    // contract: unread.
    virtual void unbind_candidate_owner(std::size_t index, int token_a, int token_b) = 0;
    // 004C39D2, 004254B0, the scope-marker log. __cdecl varargs.
    virtual void log_reassignment(const char* format, const char* name, int party,
                                  std::uint32_t from_slot, int to_slot) = 0;
    // 004C39E0 0075B430(kOwnerChangeMessageKind) then 004C3A18 0077C2A0 with
    // ECX = the candidate and (message, 0, 0). In a local session 0077C2A0
    // forces the route flags to 1 and the only destination is the local
    // enqueue 0076E520 (docs/SESSION_MESSAGE_DISPATCH.md).
    virtual void route_owner_change(std::size_t index, int new_slot) = 0;
};

// The whole routine. Returns the number of candidates that produced a message.
// skip_marked_candidates is the char argument: the load path passes false.
std::size_t run_assign_party_player_slots_004c3840(bool skip_marked_candidates,
                                                   AssignPartyPlayerSlotsHost& host);

// ---------------------------------------------------------------------------
// Step 2. reset_network_slots, the 004DFC13 branch of 004DFB70
// ---------------------------------------------------------------------------

// 004DFC13 CMP dword [EDI+1FE4h],0 / JZ 004DFD16. The whole network-slot reset
// is the ELSE of the single-player participant setup, not an unconditional
// step: a local session runs 004BB160/004BB440 and none of the code below.
inline constexpr std::size_t kSessionSlotStride = 0x118;        // 004DFC9C
inline constexpr std::size_t kSessionSlotArrayOffset = 0x748;   // the array base
inline constexpr std::size_t kSessionSlotHeaderOffset = 0x758;  // 004DFC86, +10h in
inline constexpr std::uint16_t kSessionSlotUnassignedPeer = 0xFFFD; // 004DFC94
inline constexpr std::size_t kSessionSlotCount = 8;             // 004DFC8C MOV ECX,8
// 004DFD01 IMUL ECX,ECX,434h, the stride of the menu record vector walked at
// the end of the branch.
inline constexpr std::size_t kMenuMissionRecordStride = 0x434;

// One 118h-stride header as 004DFC91..004DFC99 writes it. The same three fields
// the load path sets on the local slot at step 23 of docs/MISSION_SCENE_LOAD.md.
struct SessionSlotHeader {
    bool ready_0e{};                                  // [EAX-2]
    std::uint16_t peer_id_10{kSessionSlotUnassignedPeer}; // word [EAX]
    bool flag_18{};                                   // [EAX+8]
};

SessionSlotHeader reset_session_slot_header() noexcept;

struct NetworkSlotResetHost {
    virtual ~NetworkSlotResetHost() = default;

    // 004DFD18, 004BB160: the local arm. Everything else below is skipped.
    virtual void reset_single_player_slots_004bb160() = 0;

    // 004DFC2D, 004CEC60 on [00E18A64]->+4h, then the four header writes at
    // 004DFC32..004DFC4C that relink an MSVC _Tree empty. The element type is a
    // native string (node+0Ch length, node+10h buffer); the producer that
    // inserts into this tree was not found. contract: unread.
    virtual void clear_string_tree_00e18a60() = 0;
    // 004DFC5E, the same shape with the eraser 004C1FF0 over 00E18A6C. Its
    // element type was not read. contract: unread.
    virtual void clear_record_tree_00e18a6c() = 0;
    // 004DFC80, 00E188BD = 0. 004D87B0's side-balance block uses the same byte
    // as its one-shot (include/bsp/mission_state_frame.hpp).
    virtual void clear_side_balance_latch_00e188bd() = 0;
    // 004DFC91..004DFCA4, eight headers at game+758h, stride 118h.
    virtual void write_slot_header(std::size_t index, const SessionSlotHeader& header) = 0;

    // 004DFCA6..004DFCB8: the local slot at game+18ECh selects the entry of
    // game+18CCh whose +28h party is handed to 00626930.
    virtual int local_slot_index() = 0;
    virtual int slot_party(int slot) = 0;
    // 004DFCCF, 005D7070 with ECX = game+60Ch and ESI = [00E198AC]+5Ch+24h,
    // the menu manager's mission record vector. contract: unread.
    virtual int select_menu_record_005d7070() = 0;
    // 004DFCD6..004DFCF5, ([ESI+8] - [ESI+4]) / 434h, bounds-checked with a
    // call to the CRT trap 00BF6713 on failure.
    virtual std::size_t menu_record_count() = 0;
    // 004DFD0A, 00626930 with ECX = record and EDX = party. contract: unread.
    virtual void apply_menu_record_00626930(int record_index, int party) = 0;
};

// Returns true when the network branch ran. session_mode is game+1FE4h.
bool run_reset_network_slots_004dfc13(std::int32_t session_mode, NetworkSlotResetHost& host);

// ---------------------------------------------------------------------------
// Step 4. reset_objective_list, the 004E0754..004E07C2 block of 004DFB70
// ---------------------------------------------------------------------------
//
// Correction. This step touches no objective list. The objective sets live at
// game+21A4h + slot*4 (docs/OBJECTIVE_UNIT_LIST.md). What the block clears is a
// tree at game+5C8h and what it then rebuilds is the avoid-zone table: 004218E0
// is a lazy singleton getter (operator new(78h), constructor 00421500) and
// 00424D00 rescans the world for AvoidZone entities.

inline constexpr std::size_t kGameAvoidZoneCounterOffset = 0x648;  // 004E075A, = 0
inline constexpr std::size_t kGameAvoidZoneClockOffset = 0x64C;    // 004E0764, = 0.0f
inline constexpr std::size_t kGameSceneTreeOffset = 0x5C8;         // 004E0775
inline constexpr std::size_t kGameSceneTreeHeadOffset = 0x5CC;     // 004E076C
inline constexpr std::size_t kGameSceneTreeSizeOffset = 0x5D0;     // 004E07AC
inline constexpr std::size_t kAvoidZoneManagerSize = 0x78;         // 0042193A
// 00424D00's three literals. The world list it scans is [game+19CCh]+370h and
// an element's name is at element+154h (length) / +158h (buffer).
inline constexpr const char* kAvoidZonePrefix = "AvoidZone";
inline constexpr const char* kAvoidZoneGroupPrefix = "AvoidZoneG";
inline constexpr const char* kAvoidZoneGroupFormat = "AvoidZoneG %*s %d #%03d";
inline constexpr const char* kAvoidZoneMissingName = "<null name>";

struct AvoidZoneResetHost {
    virtual ~AvoidZoneResetHost() = default;
    virtual void clear_avoid_zone_counter_648h() = 0;       // 004E075A
    virtual void clear_avoid_zone_clock_64ch() = 0;         // 004E0764
    // 004E076C..004E07B4, an inlined _Tree::_Erase whose recursive half is
    // 004C18D0, followed by the empty relink. The element type has no value
    // destructor, so it is POD; its producer was not found. contract: unread.
    virtual void clear_scene_tree_5c8h() = 0;
    // 004E07B7 004218E0 then 004E07BE 00424D00 with ECX = the singleton. The
    // rebuild is a scan of the world entity list, so it is a contract here.
    virtual void rebuild_avoid_zones_00424d00() = 0;
};

void run_reset_avoid_zone_state_004e0754(AvoidZoneResetHost& host);

// ---------------------------------------------------------------------------
// Step 5. 004D30F0 BSP_Game_RefillScriptedNameList
// __fastcall void(GGame* /*ECX*/), RET, sole caller 004DFB70 at 004E08E4.
// ---------------------------------------------------------------------------
//
// The container is now identified: game+1930h is an MSVC _Tree (a set of native
// strings) with _Myhead at +1934h and _Mysize at +1938h. 004CEC60 is its node
// eraser, not a network-slot reset. What the set holds is the NAME of every Lua
// global whose value is a function at the moment the scene reaches state 0Ch.

inline constexpr std::size_t kGameScriptedNameSetOffset = 0x1930;     // 004D3136 base
inline constexpr std::size_t kGameScriptedNameSetHeadOffset = 0x1934; // 004D3116
inline constexpr std::size_t kGameScriptedNameSetSizeOffset = 0x1938; // 004D3136
inline constexpr std::size_t kGameLuaStateOwnerOffset = 0x1A0C;       // 004D315C
// 004D32A0's literal, appended to each name it nils at teardown.
inline constexpr const char* kLuaNilAssignmentSuffix = " = nil";

// One (key, value) pair of the Lua globals table as the walk sees it: the name
// from 00B662B0 GetString on the key and the predicate from 00B66200, which is
// lua_type(value) == 6, LUA_TFUNCTION (004D31A4, body 00B66200..00B6623E).
struct LuaGlobalEntry {
    std::string name{};
    bool is_function{};
};

// The set 004D30F0 leaves behind, in insertion order with duplicates dropped.
std::vector<std::string> scripted_name_snapshot(const LuaGlobalEntry* globals,
                                                std::size_t count);

// 004D32A0, the reader, called only from BSP_Game_TeardownSessionState and only
// when the set is non-empty (004D32D2). Every global that is a function now and
// was NOT in the snapshot is assigned nil through BSP_LuaStateOwner_ExecuteString,
// so the snapshot is the pre-script baseline of the Lua global namespace.
std::vector<std::string> scripted_globals_to_nil(const LuaGlobalEntry* globals,
                                                 std::size_t count,
                                                 const std::vector<std::string>& snapshot);

struct ScriptedNameListHost {
    virtual ~ScriptedNameListHost() = default;
    // 004D3126 004CEC60 on [game+1934h]->+4h, then the empty relink at
    // 004D312B..004D3142.
    virtual void clear_scripted_name_set() = 0;
    // 004D3167 00B67980 on game+1A0Ch, then the 00B67080 / 00B67190 /
    // 00B66420 walk. The Lua state is a contract.
    virtual std::size_t lua_global_count() = 0;
    virtual LuaGlobalEntry lua_global(std::size_t index) = 0;
    // 004D320B, 004D0640 BSP_NativeStringSet_Insert.
    virtual void insert_scripted_name(const std::string& name) = 0;
};

// Returns the number of names inserted.
std::size_t run_rebuild_scripted_name_list_004d30f0(ScriptedNameListHost& host);

// ---------------------------------------------------------------------------
// Step 7. 004C9CA0 BSP_Game_ApplyInGameInterface, the load arm only
// __thiscall void(GGame* /*ECX*/, char loading), RET 4, body 004C9CA0..004C9EB5.
// ---------------------------------------------------------------------------
//
// The routine is already read in full and implemented by milestone 2j
// (docs/GAME_EXECUTABLE.md, src/game_hosts_hud.cpp). Only the load arm's
// loading element, which that reconstruction records as a call site rather than
// a sequence, is modelled here.

inline constexpr std::size_t kInGameInterfaceLoadingElementOffset = 0xD4; // 004C9CCD
inline constexpr std::size_t kLoadingElementSize = 0x34;                  // 004C9CD5
// 00636D90 installs 00CF569C at +0h and 00CF5684 at +8h over the
// BSP_FrontEndScreen base 004F7180.
inline constexpr std::size_t kLoadingElementActiveByteOffset = 0x4;  // 004C9D1F
inline constexpr std::size_t kLoadingElementEnterByteOffset = 0x5;   // 004C9D82
// vtable 00CF569C +10h is 00636F30 BSP_HudSceneInitScreen_Register, which loads
// this GUI page and binds Message_Text (docs/HUD_SCREEN_PAGES.md).
inline constexpr const char* kLoadingElementGuiPage = "FE_sceneinit";

struct LoadingElementHost {
    virtual ~LoadingElementHost() = default;
    // 004C9CCD, [00E198C4]+D4h non-null: an element already there is reused and
    // neither allocated nor initialised again.
    virtual bool loading_element_present() = 0;
    // 004C9CD7 operator new(34h) then 004C9CED 00636D90; a null allocation
    // stores null and skips the init (004C9CF4).
    virtual bool create_loading_element_00636d90() = 0;
    // 004C9D11, vtable +10h, __thiscall with no stack argument. GUI contract.
    virtual void loading_element_init_00636f30() = 0;
    // 004C9D1F, element+4h = 1, on both the created and the reused path.
    virtual void set_loading_element_active() = 0;
};

// Returns true when the network tail at 004C9D2F would run: the load arm stops
// at 004C9D23 for a local session (game+1FE4h == 0) and at 004C9D2F when
// game+218Ch is set.
bool run_apply_in_game_interface_load_arm_004c9ccd(std::int32_t session_mode,
                                                   bool suppress_flag_218c,
                                                   LoadingElementHost& host);

}  // namespace bsp
