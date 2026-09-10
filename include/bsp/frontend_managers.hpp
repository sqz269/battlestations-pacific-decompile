#pragma once
// The three front-end manager singletons BSP_Game_EnterFrontEndShell builds
// into 00E198B8, 00E198AC and 00E198B4, and the base class they share with the
// attract screen at 00E198BC.
// Addresses: 00689800, 006898C0, 00689A10, 00689820, 00686170, 00686380,
//            00686C90, 00685820, 006887E0, 00689540, 006888E0, 00689510,
//            00687800, 00687320, 00687330, 00684E10, 00684FA0, 00684700,
//            00683AA0, 00683A90, 00684600, 004CC460, 00683E90, 006840F0,
//            004DA650, 004DB190, 004BAC20, 004BFC70, 00688AD0, 00688B20.
// Every name below is a hypothesis, not a recovered symbol. Evidence and the
// call-by-call recovery are in docs/FRONTEND_MANAGERS.md. Nothing here is
// binary compatible with the original: the vtables, the pooled strings, the
// checked-iterator containers and the intrusive refcounts are not reproduced.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// Interface ids
// ---------------------------------------------------------------------------

// 00E08CD8 is a table of 45 string pointers indexed by the interface id that
// every manager stores at +04h and +20h. 00688AD0 prints two of its entries as
// "GVMultiMenu::PushRequestInterface() req:%s set:%s", which is what fixes the
// table as an id-to-name map rather than an unrelated string array. Ids 00h..1Fh
// are front-end interfaces; 20h and above are in-session HUD interfaces, and
// 00683E90 splits the two ranges at exactly that boundary.
inline constexpr std::size_t kInterfaceNameCount = 45;
inline constexpr int kFirstInGameInterface = 0x20;      // 00683EA2
inline constexpr int kMovieCameraNewInterface = 0x2C;   // 00683E9D, the one exempt id

// The ids this packet establishes from a store or a compare, named after their
// 00E08CD8 entry.
inline constexpr int kInterfaceNone = 0x00;
inline constexpr int kInterfaceMainMenu = 0x01;         // 004E4257, 006868BC
inline constexpr int kInterfaceRewards = 0x04;          // 004E4250, 006868B8
inline constexpr int kInterfaceOptions = 0x0C;          // 006899DA
inline constexpr int kInterfaceMultiMainMenu = 0x0F;    // 006897CF, 004BFCFC
inline constexpr int kInterfaceMultiModeSelector = 0x10; // 004BFD1F

// The 00E08CD8 table itself, in index order. Returns nullptr outside the table.
const char* front_end_interface_name(int interface_id) noexcept;

// ---------------------------------------------------------------------------
// The base class, 00684E10 / 00684FA0 / 00684700 / 00683AA0 / 00684600
// ---------------------------------------------------------------------------

// 00684E10 lays out 40h bytes: the vtable at +00h, then two identical 1Ch-byte
// records at +04h and +20h, then the active byte at +3Ch. Each record is an
// interface id followed by an embedded object whose vtable is 00CE3CF0 (fields
// +0Ch..+18h of the first record, +28h..+34h of the second, all zeroed except a
// byte set to 1) and a refcounted payload pointer. Only the id and the payload
// are used by anything this packet traced, so only those two are modelled; the
// embedded object is left as an opaque hole in the layout comments.
struct InterfaceRequestRecord {
    int interface_id{0};      // +04h in `applied`, +20h in `pending`
    void* payload{nullptr};   // +1Ch in `applied`, +38h in `pending`
};

struct FrontEndManagerBase {
    // +04h..+1Fh. What the manager has already put on screen.
    InterfaceRequestRecord applied{};
    // +20h..+3Bh. What was last asked for. 006840F0 compares the two records
    // field by field and drives the manager's virtual +10h when they differ.
    InterfaceRequestRecord pending{};
    // +3Ch. Set by 00684700, cleared by 00683AA0, tested by 006840FE, by the
    // OnMove drain tail at 004E4873 and by the blocking-screen gate.
    bool active{false};
};

// The registry the base constructor joins and the base destructor leaves. The
// original is a std::map keyed by the manager pointer at 00E19898, whose head
// node pointer sits at 00E1989C; 00684700 walks it from the leftmost node and
// reads each node's +0Ch as the manager. Order is therefore by address, which
// is not reproduced here and does not matter: the walk only ever deactivates.
struct FrontEndManagerRegistry {
    std::vector<FrontEndManagerBase*> managers;
};

// 00E19894, read by 00683E90 and by the copy of it inlined at 0068461C. While
// the byte is set every in-session interface request (id >= 20h) is dropped and
// every front-end request (id < 20h) clears it; INTF_MOVIECAMERANEW passes
// without clearing. Writers outside this packet: 004DAB5D, 004D2C5F, 004BC493,
// 005CD1E9.
struct FrontEndInterfaceLock {
    bool engaged{false};
};

// 00683E90, __stdcall(int interfaceId), RET 4. Returns true when the caller
// must drop the request. Mutates the lock, which is why it is not const.
bool front_end_request_rejected_00683e90(FrontEndInterfaceLock& lock, int interface_id) noexcept;

// The two refcount helpers the records use: 00694A60 addref, 006952A0 release,
// plus 0042BCA0 for the temporary 004BEF00 builds on the stack. Modelled as a
// host because the original refcount is intrusive in an object this packet did
// not recover.
struct FrontEndPayloadHost {
    virtual ~FrontEndPayloadHost() = default;
    virtual void retain_payload(void* payload) = 0;   // 00694A60
    virtual void release_payload(void* payload) = 0;  // 006952A0
};

// 004CC460, __thiscall(this, int interfaceId, void* payload), RET 8. Records a
// request without applying it: writes `pending` only, so the next
// 006840F0 pass sees the mismatch. This is `PushRequestInterface`; the name
// comes from the 00688AD0 format string, which prints the manager's +20h before
// the call and the argument after it.
void push_interface_request_004cc460(FrontEndManagerBase& manager, FrontEndInterfaceLock& lock,
    int interface_id, void* payload, FrontEndPayloadHost& host);

// 00684600, __thiscall(this, int interfaceId, void* payload), RET 8, returns
// true when the request went through. Writes `pending` exactly as 004CC460 does
// and then copies it into `applied`, which is what marks the request serviced.
// It is the base class's own virtual +10h, so a manager that overrides +10h
// calls this first and only touches the screens when it returns true.
bool apply_interface_request_00684600(FrontEndManagerBase& manager, FrontEndInterfaceLock& lock,
    int interface_id, void* payload, FrontEndPayloadHost& host);

// The two screen-set calls the virtuals make. Both are varargs terminated by a
// zero id in the original: 004F8710(id..., 0) and 004D8C00(game, id..., 0).
struct FrontEndScreenSetHost {
    virtual ~FrontEndScreenSetHost() = default;
    // 004F8710. An empty list hides everything, which is what deactivate does.
    virtual void set_gui_interface_set(const int* ids, std::size_t count) = 0;
    // 004D8C00, ECX-free, first argument DAT_00E188A8, then the same shape.
    virtual void set_game_interface_set(const int* ids, std::size_t count) = 0;
};

// 00684700, __thiscall(this), RET. The mutual-exclusion rule: every other
// registered manager is deactivated through its own virtual +0Ch before this
// one is marked active. The in-mission interface manager at 00E198C4 is the
// single exemption from the replay at the end, tested at 0068477E; the attract
// screen at 00E198BC is not exempt.
void activate_front_end_manager_00684700(FrontEndManagerRegistry& registry,
    FrontEndManagerBase& manager, FrontEndManagerBase* replay_exempt,
    FrontEndInterfaceLock& lock, FrontEndPayloadHost& payloads, FrontEndScreenSetHost& screens);

// 00683AA0, __thiscall(this), RET. Clears the active byte and hides both screen
// sets. It keeps `applied` and `pending` intact, which is the whole reason the
// drain can lower a manager and raise it again without rebuilding it.
void deactivate_front_end_manager_00683aa0(FrontEndManagerBase& manager,
    FrontEndScreenSetHost& screens);

// 00683A90, the base virtual +4h: a bare RET. A manager with nothing to build
// inherits it, which is why the shell calls +4h unconditionally.
void front_end_manager_init_default_00683a90() noexcept;

// ---------------------------------------------------------------------------
// The three singletons
// ---------------------------------------------------------------------------

// 00E198B8, operator new(4Ch), constructor 00689800, vtable 00CF78C4.
// Init 006898C0 opens the "GVOptions" load block, publishes the global itself,
// builds three screens and commits INTF_OPTIONS.
inline constexpr std::size_t kOptionsMenuSizeBytes = 0x4C;
inline constexpr std::uint32_t kOptionsMenuVtable = 0x00CF78C4u;
inline constexpr std::string_view kOptionsMenuLoadBlock = "GVOptions"; // 00CF78DC

struct OptionsMenuManager {
    FrontEndManagerBase base{};
    // +40h 270h bytes (constructor 005F6030), +44h 30h bytes (00527D00),
    // +48h 250h bytes (00555770). Each gets its virtual +10h called right
    // after construction and its virtual +0Ch with flag 1 in the destructor.
    std::array<void*, 3> screens{};
};

// 00E198AC, operator new(78h), constructor 00686170, vtable 00CF774C.
inline constexpr std::size_t kMainMenuSizeBytes = 0x78;
inline constexpr std::uint32_t kMainMenuVtable = 0x00CF774Cu;
inline constexpr std::string_view kMainMenuLoadBlock = "GVMainMenu";   // 00CF7754
inline constexpr std::string_view kMainMenuTitleMusic = "sound/music/titlescreen.fsb";
inline constexpr std::string_view kMainMenuCreditsMusic = "sound/music/creditsfinal.fsb";
inline constexpr std::string_view kMainMenuLocaleTable = "globals";    // 00CF7778
// Both music paths are cached twice: the sibling built by replacing the last
// four characters with the literal at 00CF779C, at priority 32h, and the .fsb
// itself at priority 2.
inline constexpr std::string_view kMainMenuMusicSidecarExtension = ".def"; // 00CF779C
inline constexpr int kMainMenuSidecarCachePriority = 0x32;
inline constexpr int kMainMenuMusicCachePriority = 0x02;

// The loading-screen fractions the two reporting Init bodies pass, in order.
// GVMultiMenu::Init reports nothing.
inline constexpr std::array<float, 2> kOptionsMenuProgressSteps{
    0.35f,  // 00CF6560
    0.37f,  // 00CF78D8
};
inline constexpr std::array<float, 8> kMainMenuProgressSteps{
    0.40f,  // 00CE7804
    0.45f,  // 00CF2E4C
    0.50f,  // 00CE3800
    0.55f,  // 00CED6C4
    0.80f,  // 00CE74F8
    0.94f,  // 00CF7770
    0.96f,  // 00CF776C
    1.00f,  // 3F800000, an immediate
};

struct MainMenuManager {
    FrontEndManagerBase base{};
    // +40h and +48h are the two native strings above, each {length, data}.
    std::array<std::string_view, 2> music_paths{};
    // +50h and +54h. Refcounted handles the destructor releases with an
    // InterlockedDecrement on handle+4h followed by the handle's virtual +00h;
    // nothing in this packet writes them, so they are the streams the music
    // paths above are opened into. Provisional.
    std::array<void*, 2> music_handles{};
    // +58h..+70h, in construction order: 578h (005902E0), 34h (005CA880),
    // 260h (00626630), 138h (0051E4D0), 5Ch (0052FCE0), 16Ch (00563370),
    // 5D0h (005098B0). The shell reaches +58h directly at 004E4271.
    std::array<void*, 7> screens{};
    // +74h, zeroed by the constructor and not written by Init.
    bool flag_74{false};
};

// 00E198B4, operator new(68h), constructor 006887E0, vtable 00CF7820.
inline constexpr std::size_t kMultiMenuSizeBytes = 0x68;
inline constexpr std::uint32_t kMultiMenuVtable = 0x00CF7820u;
inline constexpr std::string_view kMultiMenuLoadBlock = "GVMultiMenu::Init"; // 00CF7834

struct MultiMenuManager {
    FrontEndManagerBase base{};
    // +40h..+54h: F8h (005E6D90), 100h (005EA8C0), 300h (005E3290),
    // 2D0h (00574240), 5Ch (005D2F90), 90h (005CFDA0).
    std::array<void*, 6> screens{};
    // +58h. Neither the constructor nor Init nor the destructor touches it.
    std::uint32_t unknown_58{0};
    // +5Ch..+64h, a vector zeroed by the constructor and freed by the
    // destructor. Nothing in this packet fills it; element type unknown.
    std::vector<void*> owned_58{};
};

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

// The four front-end singletons the destroy paths know about, in the order
// 004DA650 walks them.
struct FrontEndManagerSet {
    OptionsMenuManager* options{nullptr};   // 00E198B8
    MainMenuManager* main_menu{nullptr};    // 00E198AC
    MultiMenuManager* multi_menu{nullptr};  // 00E198B4
    // 00E198C4, the in-mission interface manager (docs/GAME_SIMULATION_GATE.md).
    // A fourth instance of the same base, and the one 00684700 exempts.
    FrontEndManagerBase* in_mission{nullptr};
};

// One method per native call site of the three constructors, their Init
// virtuals and their destructors. There are no default implementations:
// nothing here stands in for unrecovered game behaviour.
struct FrontEndManagerHost {
    virtual ~FrontEndManagerHost() = default;

    // 00BD1A60 then 00BE0AE0(id, 1) at the head of every Init, and
    // 00BDCB30 BSP_FileBlock_Destroy at its tail. GVMultiMenu opens its block
    // through 00BE0A30 BSP_FileBlock_Construct instead, with the same effect.
    virtual void open_load_block(std::string_view name) = 0;
    virtual void close_load_block() = 0;

    // 0057BEC0 BSP_LoadingScreen_ReportProgress. GVOptions reports three times,
    // GVMainMenu eight; GVMultiMenu reports not at all.
    virtual void report_loading_progress(float progress) = 0;

    // 00BF681B operator new followed by the screen constructor, then the
    // screen's virtual +10h. A null allocation is stored and then dereferenced
    // by the original; the reconstruction keeps the store and omits the fault.
    virtual void* create_screen(std::size_t size_bytes, std::uint32_t constructor_address) = 0;
    virtual void register_screen(void* screen) = 0;   // screen virtual +10h

    // The destructor's two passes over the screen list. The first hides every
    // screen whose byte +5 is set through its virtual +1Ch, clears bytes +4 and
    // +5 and calls 004F83B0; the second destroys it through virtual +0Ch(1).
    virtual void hide_screen(void* screen) = 0;
    virtual void destroy_screen(void* screen) = 0;

    // 004FC150 / 004FC1F0 / the FileStore cache and remove pair GVMainMenu uses
    // for its two .fsb paths and their ".fev" siblings.
    virtual void cache_music(std::string_view path) = 0;
    virtual void remove_music(std::string_view path) = 0;

    // BSP_Localization_RegisterTableName then BSP_Localization_ReloadTables(0),
    // GVMainMenu only.
    virtual void register_locale_table(std::string_view name) = 0;
    virtual void reload_locale_tables() = 0;

    // 004C12B0 BSP_GuiManager_GetOrCreate(1) then its enable setter, the last
    // thing GVMainMenu::Init does before closing its load block.
    virtual void set_gui_layer_enabled(int layer, bool enabled) = 0;

    // DAT_00E198B0, set by the mission-end wait to (game+1EE1h == 0). When it
    // is 1 GVMainMenu::Init commits INTF_REWARDS instead of INTF_MAINMENU.
    virtual bool returning_from_mission() = 0;

    // *(game+1EE1h). When it is non-zero GVMainMenu::Init clears 00E198B0 and
    // raises 00E08874 before building anything.
    virtual bool mission_result_pending() = 0;
    virtual void clear_returning_from_mission() = 0;

    // 006B8AD0("collectgarbage(\"collect\")", 0, 0, 2), run by 004BAC20 and
    // 004BFC70 after a lazy construction when *(game+1A08h)+4h is set.
    virtual bool script_gc_enabled() = 0;
    virtual void run_script_gc() = 0;

    // game+5D4h, written by the two lazy-construction request handlers.
    virtual void set_game_state(int state) = 0;

    // operator new plus the constructor, for the managers themselves:
    // 4Ch/00689800, 78h/00686170 and 68h/006887E0. The original stores a null
    // allocation and then dereferences it; the reconstruction returns null and
    // the callers leave without building, which is the one place it diverges.
    virtual OptionsMenuManager* create_options_manager() = 0;
    virtual MainMenuManager* create_main_menu_manager() = 0;
    virtual MultiMenuManager* create_multi_menu_manager() = 0;

    // 00576B10, run twice by GVMultiMenu::Init around the two refreshes below.
    virtual void refresh_multiplayer_state() = 0;
    // DAT_00F88950 then 00687C00.
    virtual bool downloadable_content_present() = 0;
    virtual void refresh_downloadable_content() = 0;
    // 008D2F50.
    virtual void refresh_online_services() = 0;
    // The inline strlen of *(game+1FF0h) inside 00689540, then 0076F0E0 when
    // the stored profile name is empty.
    virtual bool profile_name_empty() = 0;
    virtual void create_default_profile() = 0;

    // *(00F8A2FC)+4Ch, tested at 004BFCF0 before the forced INTF_MULTIMAINMENU.
    virtual bool online_signed_in() = 0;
    // DAT_00E19610, cleared at 004BFD21 alongside the INTF_MULTIMODESELECTOR
    // push.
    virtual void clear_multiplayer_entry_reason() = 0;
    // 004D95F0(0) at the head of 004DA650, and 004D7970(0) plus 004DA780 at the
    // head of 004DB190. Both are outside this packet.
    virtual void reset_interface_stack() = 0;
    virtual void lower_in_mission_interface() = 0;
    // DAT_00E08874, raised at 004DB190 and by GVMainMenu::Init.
    virtual void raise_front_end_dirty_flag() = 0;
};

// 004E4171, 004E41B0 and 004E41F0, the shell's own construction order: the
// options manager, then the main menu, then the multiplayer menu, each
// allocated, registered and immediately driven through its virtual +4h. The
// shell writes each global after the Init virtual has already written it.
void construct_front_end_managers_004e4171(FrontEndManagerSet& set,
    FrontEndManagerRegistry& registry, FrontEndInterfaceLock& lock, FrontEndManagerHost& host,
    FrontEndPayloadHost& payloads, FrontEndScreenSetHost& screens);

// 006898C0, 00686380 and 00689540: the three Init virtuals, in the order their
// bodies run. Each publishes its own global before it builds anything, which is
// why the shell's write of the same pointer at 004E419E, 004E41DE and 004E421E
// is redundant.
void init_options_menu_006898c0(OptionsMenuManager& manager, FrontEndManagerSet& set,
    FrontEndInterfaceLock& lock, FrontEndManagerHost& host, FrontEndPayloadHost& payloads);
void init_main_menu_00686380(MainMenuManager& manager, FrontEndManagerSet& set,
    FrontEndInterfaceLock& lock, FrontEndManagerHost& host, FrontEndPayloadHost& payloads,
    FrontEndScreenSetHost& screens, FrontEndManagerRegistry& registry);
void init_multi_menu_00689540(MultiMenuManager& manager, FrontEndManagerSet& set,
    FrontEndInterfaceLock& lock, FrontEndManagerHost& host, FrontEndPayloadHost& payloads);

// 00689A10, 00686C90 and 006888E0: the three destructor bodies. Each clears its
// own global before it returns, so the caller's own clear is a second write.
void destroy_options_menu_00689a10(OptionsMenuManager& manager, FrontEndManagerSet& set,
    FrontEndManagerHost& host);
void destroy_main_menu_00686c90(MainMenuManager& manager, FrontEndManagerSet& set,
    FrontEndManagerHost& host);
void destroy_multi_menu_006888e0(MultiMenuManager& manager, FrontEndManagerSet& set,
    FrontEndManagerHost& host);

// 004DA650, __cdecl, no arguments, RET. Destroys all four singletons through
// their virtual +00h with the delete flag and clears each global. Order: main
// menu, multiplayer menu, options, in-mission interface manager.
void destroy_front_end_managers_004da650(FrontEndManagerSet& set, FrontEndManagerHost& host);

// 004DB190, __cdecl, no arguments, RET. The teardown that leaves the front end
// for a session: it lowers the in-mission interface manager first, then destroys the
// multiplayer menu, the options menu and the main menu in that order. The
// options and main menu order is the reverse of 004DA650's.
void teardown_front_end_managers_004db190(FrontEndManagerSet& set, FrontEndManagerHost& host);

// 006840F0 BSP_MenuInterface_ServicePendingRequests, __thiscall(this = an out
// byte), RET. Walks the main menu, the multiplayer menu and the options manager
// in that order and, for each that exists, is active and whose two records
// differ, drives its virtual +10h with the pending id and payload. Returns true
// when at least one manager was serviced, which is the byte the original writes
// through ECX.
bool service_pending_interface_requests_006840f0(FrontEndManagerSet& set,
    FrontEndInterfaceLock& lock, FrontEndPayloadHost& payloads, FrontEndScreenSetHost& screens);

// The drain requests that reach a manager. 06h, 09h and 16h each call one
// manager's virtual +0Ch; 07h and 14h build one lazily and raise it.
enum class FrontEndManagerRequest : std::uint32_t {
    kLowerMainMenu = 0x06,   // 00E198AC virtual +0Ch
    kRaiseMultiMenu = 0x07,  // 004BFC70
    kLowerMultiMenu = 0x09,  // 00E198B4 virtual +0Ch
    kRaiseOptions = 0x14,    // 004BAC20
    kLowerOptions = 0x16,    // 00E198B8 virtual +0Ch
};

// The three lowering requests. They deactivate; they do not destroy. Returns
// false when the named manager does not exist, which is the case the original
// does not guard: it loads the global and calls through it unconditionally.
bool run_front_end_manager_request(FrontEndManagerRequest request, FrontEndManagerSet& set,
    FrontEndScreenSetHost& screens);

// 004BAC20, __thiscall(this = the game), RET, the 14h handler. Builds the
// options manager if it is absent, runs its Init, collects Lua garbage, raises
// it and leaves the game in state 15h.
void raise_options_menu_004bac20(FrontEndManagerSet& set, FrontEndManagerRegistry& registry,
    FrontEndInterfaceLock& lock, FrontEndManagerHost& host, FrontEndPayloadHost& payloads,
    FrontEndScreenSetHost& screens);

// 004BFC70, __thiscall(this = the game), RET, the 07h handler and the routine
// the shell runs at 004E428A when 0067D6E0 is true. Same shape, plus the two
// PushRequestInterface calls at 004BFCFC and 004BFD1F and a replay of the
// pending record when the game's +218Ch byte is set. Leaves state 8.
void raise_multi_menu_004bfc70(FrontEndManagerSet& set, FrontEndManagerRegistry& registry,
    FrontEndInterfaceLock& lock, bool multiplayer_entry_pending, FrontEndManagerHost& host,
    FrontEndPayloadHost& payloads, FrontEndScreenSetHost& screens);

}  // namespace bsp
