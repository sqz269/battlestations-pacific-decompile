#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "bsp/input_tick.hpp"
#include "bsp/main_menu_screens.hpp"

// The main-menu screen class itself: the 578h object the manager at 00E198AC
// holds at +58h, constructed by 005902E0 with screen id 1 (INTF_MAINMENU).
//
// docs/MAIN_MENU_SCREENS.md records the class from its vtable and its string
// sets. This header adds the behaviour of the seven virtuals in that row, and
// above all of the update virtual 00599DB0, read to the instruction from the
// listing at 00599DB0..0059A6E2.
//
// Nothing here is a binary-compatible layout. The offsets recorded below are
// only the ones a routine in this packet demonstrably reads or writes; every
// other byte of the 578h object is unrecovered. Descriptive names are
// hypotheses, not recovered symbols.

namespace bsp {

// ---------------------------------------------------------------------------
// The vtable row, 00CEFC5C
// ---------------------------------------------------------------------------

// Calling convention and RET size of each override, read off the listing. Every
// one is __thiscall with ECX = the screen; `stack_bytes` is the RET operand.
struct MainMenuScreenVirtual {
    FrontEndScreenSlot slot;
    std::uint32_t address;
    std::uint16_t stack_bytes;   // the RET operand, 0 for a bare RET
    std::string_view summary;
};

inline constexpr std::array<MainMenuScreenVirtual, 7> kMainMenuScreenVirtuals{{
    {FrontEndScreenSlot::Destroy, 0x00590D60, 4,
     "scalar deleting destructor: 00590600 then _free when bit 0 of the flag is set"},
    {FrontEndScreenSlot::Register, 0x00582F30, 0,
     "chains 004F71D0, calls its own slot +14h, measures two text heights into "
     "+2B0h/+2B4h, hides +470h, clears +564h/+565h, sets the presentation mode"},
    {FrontEndScreenSlot::BindLayout, 0x005861B0, 0,
     "binds FE_main and the widget set; fills the five mission-group handles at "
     "+310h..+320h, the two backdrop icons at +328h/+32Ch and the list box at +1B8h"},
    {FrontEndScreenSlot::Enter, 0x005987F0, 0,
     "opens the page the item dispatcher selected; sets the map zoom +130h to 1.0"},
    {FrontEndScreenSlot::Exit, 0x0058F560, 0,
     "hides +2F0h, resets the page to TopLevel unless the game state is 10 or the "
     "page is already 1/2/3, then 00583E50 and 0058D9E0(1)"},
    {FrontEndScreenSlot::Update, 0x00599DB0, 4,
     "the per-frame page machine; the one stack argument is the frame delta in "
     "seconds, read as a float at 0059A3BD and 0059A66B"},
    {FrontEndScreenSlot::FillList, 0x005905E0, 4,
     "ADD ECX,470h / PUSH ECX / MOV ECX,[ESP+8] / CALL 004D6790: passes the "
     "sub-object at +470h to 004D6790 with the stack argument in ECX"},
}};

const MainMenuScreenVirtual* main_menu_screen_virtual(FrontEndScreenSlot slot) noexcept;

// ---------------------------------------------------------------------------
// Field layout
// ---------------------------------------------------------------------------

// One recovered field of the 578h object. `offset` is the byte offset from the
// object base; `written_by` is the routine that establishes it.
struct MainMenuScreenField {
    std::uint16_t offset;
    std::string_view name;
    std::string_view evidence;
};

inline constexpr std::array<MainMenuScreenField, 26> kMainMenuScreenFields{{
    {0x000, "primary_vtable", "005902E0 stores 00CEFC5C"},
    {0x00C, "text_owner_subobject", "005902E0 stores 00CEFC24; 00599DD2 gates on the byte at +10h"},
    {0x010, "text_owner_has_queue", "00599DD2 CMP byte [ESI+10h],0 then 00A96F40(this+0Ch, delta)"},
    {0x070, "ten_widget_vectors", "005902E0 clears ten 10h-byte vectors at +70h..+10Ch"},
    {0x110, "active_mission_group", "00599DB0 writes one of +310h..+320h here every campaign frame"},
    {0x11C, "layout_extent_pair", "005861B0 stores the two dwords 00AA6740 returns"},
    {0x124, "backdrop_position", "005861B0 stores the three floats 00AA6750 returns; "
                                 "0059A68E passes this+124h to 00AA8240 on the bg_01_Icon at +328h"},
    {0x130, "map_zoom", "00582F30 leaves it at 0, 005987F0 sets 1.0, 00588C70 clamps to [0.5,1.5]"},
    {0x134, "mission_point_lists", "005902E0 runs the vector ctor iterator over five 10h-byte "
                                   "elements at +134h; 00588C70 indexes this+134h+i*10h"},
    {0x184, "map_anchor", "00588C70 stores the 005803E0 result into +184h/+188h/+18Ch"},
    {0x190, "map_base_offset", "00582F30 sets +190h = 0.2, +194h = 0.39, +198h = 0"},
    {0x1A0, "zoom_axis_latched", "0059A5E5 stores 1 when the zoom delta is non-zero"},
    {0x1A4, "map_offset", "00582F30 clears +1A4h/+1A8h/+1ACh; 00588C70 adds them to +190h"},
    {0x1B8, "top_level_list_box", "00584AE0 drives it through 00A9AC40/00A9BEC0/00A9C7C0; "
                                  "00588A80 reads the selection with 00A9C990"},
    {0x1D4, "page_animator", "0059A6AF passes this+1D4h to 00683820 with the +56Ch byte"},
    {0x238, "list_entry_pool", "00584AE0 allocates each entry with 00AAB4C0(this+238h, 0)"},
    {0x2F0, "exit_hide_target", "0058F560 calls its virtual +34h with 0"},
    {0x310, "missions_us_group", "005861B0 binds \"missions_US_Group\""},
    {0x314, "missions_jp_group", "005861B0 binds \"missions_JP_Group\""},
    {0x318, "missions_us_dlc_group", "005861B0 binds \"missions_US_DLC_Group\""},
    {0x31C, "missions_jp_dlc_group", "005861B0 binds \"missions_JP_DLC_Group\""},
    {0x320, "training_group", "005861B0 binds \"training_Group\""},
    {0x328, "backdrop_icon", "005861B0 binds \"bg_01_Icon\""},
    {0x354, "detail_animator", "0059A3F0 and 0059A41C pass this+354h to 00683820 with 1"},
    {0x500, "async_text_pending", "00590A00 sets it, 0059A02A..0059A07A clears it"},
    {0x56C, "pad_axis_override_off", "005978A1 writes it; 0059A532 gates the pad poll on it"},
}};

// Two std::string members in the VC8 secure-SCL layout (proxy at +0h, 16-byte
// buffer at +4h, size at +14h, capacity at +18h, 1Ch bytes total). 005902E0
// initialises both: buffer bytes at 508h/524h, sizes at 518h/534h, capacities
// 0Fh at 51Ch/538h. The page-10 arm passes their addresses, 504h and 520h.
inline constexpr std::uint16_t kMainMenuScreenStringA = 0x504;
inline constexpr std::uint16_t kMainMenuScreenStringB = 0x520;

// The two campaign selector bytes. 00582F30 clears both; 00599DB0 reads them on
// the mission-detail page to pick the mission group and the point list.
inline constexpr std::uint16_t kMainMenuScreenUsCampaignByte = 0x564;
inline constexpr std::uint16_t kMainMenuScreenDlcCampaignByte = 0x565;

// The highest field 005902E0 writes is +574h, which is what makes the 578h
// operator new argument of docs/FRONTEND_MANAGERS.md consistent.
inline constexpr std::uint32_t kMainMenuScreenObjectSize = 0x578;

// ---------------------------------------------------------------------------
// The five mission groups
// ---------------------------------------------------------------------------

// Index 0..4 is the value 00599DB0 keeps in EDI/EBP and hands to 00588C70 as
// its fifth argument. It selects both the widget group written to +110h and the
// point list at +134h + index*10h, so the two are one choice, not two.
enum class MissionGroup : int {
    Ijn = 0,       // +314h missions_JP_Group
    Usn = 1,       // +310h missions_US_Group
    Training = 2,  // +320h training_Group
    IjnDlc = 3,    // +31Ch missions_JP_DLC_Group
    UsnDlc = 4,    // +318h missions_US_DLC_Group
};

struct MissionGroupBinding {
    MissionGroup group;
    std::uint16_t handle_offset;  // the field copied into +110h
    std::uint16_t point_list_offset;  // +134h + index*10h
    std::string_view widget;      // the name 005861B0 passes to 00AA7E00
};

inline constexpr std::array<MissionGroupBinding, 5> kMissionGroupBindings{{
    {MissionGroup::Ijn, 0x314, 0x134, "missions_JP_Group"},
    {MissionGroup::Usn, 0x310, 0x144, "missions_US_Group"},
    {MissionGroup::Training, 0x320, 0x154, "training_Group"},
    {MissionGroup::IjnDlc, 0x31C, 0x164, "missions_JP_DLC_Group"},
    {MissionGroup::UsnDlc, 0x318, 0x174, "missions_US_DLC_Group"},
}};

const MissionGroupBinding& mission_group_binding(MissionGroup group) noexcept;

// The five pages 00599DB0 treats as a mission list, and the group each selects.
// Read off the compare chain at 0059A28D..0059A2B4 and the assignments at
// 0059A4B4..0059A511. This corrects the campaign-page naming of
// docs/MAIN_MENU_SCREENS.md, which was inferred from neighbouring title strings
// rather than from the widget each page binds.
bool is_mission_list_page_00599db0(MainMenuPage page) noexcept;
MissionGroup mission_group_for_page(MainMenuPage page) noexcept;

// The mission-detail page picks the same five groups from the two campaign
// bytes instead (0059A31C..0059A369). Training is unreachable this way.
MissionGroup mission_group_for_detail(bool us_campaign, bool dlc_campaign) noexcept;

// ---------------------------------------------------------------------------
// The front-end action query, 004D92B0
// ---------------------------------------------------------------------------

// 004D92B0 is __thiscall(ECX = the game at 00E188A8, int action). It scales the
// action by 30h into the record table of docs/GAME_INPUT_TICK.md and reports a
// fresh press or an auto-repeat, then raises one of three UI sound flags.
inline constexpr int kFrontEndActionAccept = 0x4A;
inline constexpr int kFrontEndActionBack = 0x4B;
inline constexpr int kFrontEndActionContext = 0x50;

// The action indices the update passes, in the order they are tested per page.
enum class FrontEndActionSound { None, Navigate, Select, Back };

// Sound slot on the request queue singleton 004C1B90: +19h navigate, +1Ah
// select, +1Bh back. Only action 4Bh reaches +1Bh, which is what identifies it.
FrontEndActionSound front_end_action_sound_004d92b0(int action) noexcept;

// The repeat clock. `game+64Ch` is the front-end time base; the per-action
// deadline lives in the map 004D6900 keys by action index.
inline constexpr float kFrontEndActionRepeatDelay = 0.4f;    // 00CE65D0, a double
inline constexpr float kFrontEndActionRepeatInterval = 0.1f; // 00D7A3A0, a double
inline constexpr float kFrontEndActionHoldFloor = 0.0f;      // 00D7A218

// True when the action fires this frame, either as a fresh press or as a repeat.
// `deadline` is updated in place exactly as the native body updates its map slot.
bool front_end_action_fired_004d92b0(const InputActionRecord& record,
                                     float front_end_clock,
                                     float& deadline) noexcept;

// ---------------------------------------------------------------------------
// The top-level menu items
// ---------------------------------------------------------------------------

// What 00588A80 does with the list-box selection it reads through 00A9C990 and
// caches in 00E194C4. `state_requests` are pushed onto the game+5D8h deque of
// docs/GAME_FRAME_CONTROL.md through BSP_Game_RequestState (004D7920); this
// corrects docs/MAIN_MENU_SCREENS.md, which reported that no screen in this
// range produces on that deque.
struct MainMenuItemAction {
    int index;
    std::string_view label;
    std::uint32_t routine;        // the call 00588A80 makes, 0 when there is none
    std::array<int, 2> state_requests;  // 0 terminates
    std::string_view summary;
};

inline constexpr std::array<MainMenuItemAction, kMainMenuItemCount> kMainMenuItemActions{{
    {0, "FE.main_singleplayer", 0x00584F50, {0, 0},
     "00588AB1: builds the single-player page and sets page 2"},
    {1, "FE.main_multiplayer", 0x005853C0, {0, 0},
     "00588ACD: builds the multiplayer page and sets page 3"},
    {2, "FE.main_tacticallibrary", 0x005886F0, {0, 0},
     "00588B39 runs the Lua chunk luaLoadControlFunctionNames() first, then "
     "00588B40 builds the tactical-library page and sets page 0Bh"},
    {3, "FE.main_options", 0, {6, 0x14},
     "00588AEF clears 00E19650, then 00588AF9/00588B06 enqueue 6 and 14h; the "
     "drain runs the manager virtual +0Ch and 004BAC20 BSP_Game_RaiseOptionsMenu"},
    {4, "globals.live", 0x00A4D44C, {0, 0},
     "00588B5C calls XShowGuideUI(0)"},
    {5, "FE_pc.main_marketplace", 0, {0, 0},
     "00588B71: only when 00585B40 and 00585810 both pass; then "
     "XShowMarketplaceUI(BSP_XenonSystemManager_GetActiveUserSlot(0))"},
    {6, "FE_pc.main_quit", 0x00531B00, {0, 0},
     "raises the FE_pc.main_quit_confirm modal with callback 0057D3B0, then "
     "00530C20 on the menu-command screen; request 13h is enqueued by the callback"},
}};

const MainMenuItemAction& main_menu_item_action(int index) noexcept;

// 00584AE0 is __thiscall(this, bool select_first) and ends RET 4. At 00584C2F it
// tests the argument: zero restores the cached selection 00E194C4, non-zero
// selects item 0. Both callers inside this packet pass zero.
int top_level_selection_00584ae0(bool select_first, int cached_selection) noexcept;

// The dword at 00E194C4 the constructor clears and 00588A80 rewrites on every
// activation. It is the menu's highlighted-item memory across page changes.
inline constexpr std::uint32_t kTopLevelSelectionGlobal = 0x00E194C4;

// ---------------------------------------------------------------------------
// The map zoom
// ---------------------------------------------------------------------------

// 00414130 blends the +1A8h map offset on the mission-detail page from the two
// floats at 00CEFF04 and 00D7A2F0. Its body was not read, so the operation is
// unidentified; the two operands are recorded and the call is a host method.
inline constexpr float kMissionDetailOffsetLow = -0.21f;  // 00CEFF04
inline constexpr float kMissionDetailOffsetHigh = 0.1f;   // 00D7A2F0
// The constant zoom delta the mission-detail page hands to 00588C70 (00CE54A0).
inline constexpr float kMissionDetailZoomDelta = 0.2f;
// The page the async-text arm owns. No site in the image writes it with an
// immediate, so it is reachable only through the register writers listed in
// docs/MAIN_MENU_SCREEN_UPDATE.md.
inline constexpr int kMainMenuPageAsyncText = 0x0A;

// The two analog sources the campaign pages read. 0059A51F and 0059A53B index
// the action record table at records+3654h and records+3684h, which is field
// +24h (the current hold amount) of records 121h and 122h.
inline constexpr int kMapZoomOutAction = 0x121;  // records+3654h
inline constexpr int kMapZoomInAction = 0x122;   // records+3684h

inline constexpr float kMapZoomMin = 0.5f;      // 00CE3800
inline constexpr float kMapZoomMax = 1.5f;      // 00CE380C
inline constexpr float kMapZoomIdleThreshold = 1.0f;  // 00D7A24C
inline constexpr float kMapZoomIdleDrift = -0.2f;     // 00CE69CC
inline constexpr float kMapZoomGain = 0.1f;           // 00D7A3A0, read as a double
inline constexpr float kMapOffsetDecay = 0.1f;        // 00D7A3A0 again, at 0059A606

// The zoom delta a campaign frame hands to 00588C70. `pad_axis` is the value the
// first device in the 00F8BBF4 vector returns from its virtual +24h with 0Ah; it
// is only consulted when the +56Ch byte is clear and the vector is non-empty. A
// negative pad value replaces the zoom-out hold, a positive one greater than
// 00D7A218 replaces the zoom-in hold (0059A590..0059A5BF).
struct MapZoomInput {
    float zoom_out_hold{0.0f};  // record 121h +24h
    float zoom_in_hold{0.0f};   // record 122h +24h
    bool pad_axis_valid{false};
    float pad_axis{0.0f};
};

float map_zoom_delta_0059a517(const MapZoomInput& input, bool pad_axis_override_off) noexcept;

// 00588C70 accumulates the delta into +130h and clamps. `param_7` non-zero or a
// non-zero carried value at +19Ch swaps the smoothing constant; only the clamp
// is reconstructed here because +19Ch has no writer in this packet.
float apply_map_zoom_00588c70(float zoom, float delta) noexcept;

// The +1A8h component of the map offset decays toward zero every frame,
// whichever page is showing (0059A5F2..0059A60E).
float decay_map_offset_0059a5f2(float offset) noexcept;

// ---------------------------------------------------------------------------
// The page machine
// ---------------------------------------------------------------------------

// The scene id the update hands to 004C1E90 before 00427190 at the shared
// epilogue 0059A6BC. Pages that never reach the epilogue return 0.
int page_scene_id_0059a6bc(MainMenuPage page) noexcept;

// The two content modals the update raises before the page machine runs.
inline constexpr std::string_view kContentDamagedMessage = "FE_xbox.xsm_dlcdamaged";     // 00CEFF34
inline constexpr std::string_view kContentDownloadedMessage = "FE_xbox.xsm_dlcdownloaded"; // 00CEFF18

// Sign-in states the downloaded-content gate accepts, read off 00599F39..00599F58.
inline constexpr int kSignInStateOnline = 2;
inline constexpr int kSignInStateLocal = 1;

// Integration boundary for the update. Each method is one native call site,
// listed in frame order, in the style of bsp::ApplicationFrameHost. There are no
// default implementations: nothing here stands in for unrecovered behaviour.
struct MainMenuScreenUpdateHost {
    virtual ~MainMenuScreenUpdateHost() = default;

    // --- head, 00599DD2..0059A018
    virtual bool text_owner_has_queue() = 0;                 // byte at this+10h
    virtual void dispatch_text_events(float seconds) = 0;    // 00A96F40(this+0Ch, delta)
    virtual bool content_set_dirty() = 0;                    // 00E0887C
    virtual void clear_content_set_dirty() = 0;              // 00599EF2
    virtual bool content_manager_present() = 0;              // 00F8A304 != 0
    virtual int content_entry_count() = 0;                   // 00F8A304[2]
    virtual bool content_entry_damaged(int index) = 0;       // byte at entry+12h, stride 18h
    virtual void show_message(std::string_view key) = 0;     // 00531B00(2, key, 2, 0, 0, ...)
    virtual bool content_download_ready() = 0;               // 00F8ABE8+30h
    virtual void clear_content_download_ready() = 0;         // 0059A01D
    virtual bool menu_command_screen_idle() = 0;             // 00425D10 result +5h == 0
    virtual bool has_selected_user() = 0;                    // 00A3E510
    virtual int selected_sign_in_state() = 0;                // 00A3EAD0
    virtual bool content_manager_ready() = 0;                // 00F8A304 virtual +18h
    virtual void content_manager_commit() = 0;               // 00F8A304 virtual +24h

    // --- the async-text arm, 0059A020..0059A080
    virtual bool async_text_manager_present() = 0;           // 00F8A2FC != 0
    virtual bool async_text_pending() = 0;                   // byte at this+500h
    virtual void set_async_text_pending(bool pending) = 0;
    virtual bool async_text_busy() = 0;                      // 00F8A2FC virtual +44h
    virtual int async_text_status() = 0;                     // 00F8A2FC virtual +3Ch, field +34h

    // --- the page-10 arm, 0059A085..0059A280
    virtual void async_text_read_title(std::uint16_t dst_offset) = 0;  // virtual +148h
    virtual void async_text_submit(std::uint16_t a, std::uint16_t b) = 0; // virtual +70h
    virtual void rebuild_top_level_after_text(bool flag) = 0;  // 00590A00(this, 1)
    virtual void request_game_state(int request) = 0;          // 004D7920
    virtual void set_page(MainMenuPage page) = 0;              // 00E08874

    // --- the page machine, 0059A288..0059A6BA
    virtual MainMenuPage page() = 0;                           // 00E08874
    virtual bool action_fired(int action) = 0;                 // 004D92B0(game, action)
    virtual void build_top_level_page(bool select_first) = 0;  // 00584AE0
    virtual void build_single_player_page() = 0;               // 00584F50
    virtual void build_mission_detail_page() = 0;              // 0058C010
    virtual void mission_detail_accept() = 0;                  // 00594BF0
    virtual void mission_detail_back() = 0;                    // 00599340
    virtual void open_tactical_library_with_selection() = 0;   // 005885D0
    virtual void open_tactical_library_mode2() = 0;            // 005806A0 then 005886C0
    virtual void set_active_mission_group(MissionGroup group) = 0;  // this+110h
    virtual void position_backdrop() = 0;                      // 00AA8240(this+328h, this+124h)
    virtual void drive_map(float seconds, MissionGroup group, float zoom_delta,
                           bool moved) = 0;                    // 00588C70
    virtual MapZoomInput map_zoom_input() = 0;
    virtual void animate_detail_group() = 0;                   // 00683820(this+354h, 1)
    virtual void animate_page_group(bool pad_axis_override_off) = 0; // 00683820(this+1D4h, +56Ch)
    virtual void update_scene(int scene_id) = 0;               // 004C1E90 then 00427190
    // 00414130 with ECX = &this+1A8h and the two operands above.
    virtual float blend_map_offset(float current, float low, float high) = 0;
};

// The mutable state the update owns on the screen object.
struct MainMenuScreenState {
    float map_zoom{1.0f};        // +130h
    float map_offset_y{0.0f};    // +1A8h
    bool zoom_axis_latched{false};  // +1A0h
    bool us_campaign{false};     // +564h
    bool dlc_campaign{false};    // +565h
    bool pad_axis_override_off{false};  // +56Ch
};

// Which arm of the page machine a frame took, so a caller can observe the
// sixteen exits of 00599DB0 without reproducing its control flow.
enum class MainMenuUpdateArm {
    None,             // the page matched no arm; 0059A6C8 without the epilogue
    TopLevel,         // page 1, and pages 2/3 after the back action
    TacticalLibrary,  // page 0Bh; returns at 0059A2DC without the epilogue
    MissionDetail,    // page 9
    Objectives,       // page 0Ch
    MissionList,      // pages 4, 5, 6, 7, 8
};

struct MainMenuUpdateResult {
    MainMenuUpdateArm arm{MainMenuUpdateArm::None};
    int scene_id{0};             // the argument the epilogue passed to 004C1E90
    bool reached_epilogue{false};
};

// The update virtual 00599DB0. Native ECX = the screen, one stack argument (the
// frame delta in seconds), RET 4, no return value. The result is returned here
// only so a host can see which arm ran; the native body returns nothing.
MainMenuUpdateResult run_main_menu_screen_update_00599db0(MainMenuScreenState& state,
                                                          MainMenuScreenUpdateHost& host,
                                                          float seconds);

}  // namespace bsp
