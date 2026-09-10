#pragma once
#include <cstdint>

// Tail of cSkeletonAppMidway::Init (0073d410), from the "After InitGui"
// checkpoint to the closing RET 8 at 0073e52a. Two routines are modelled here:
// the game startup-mode entry BSP_Game_BeginStartupSequence (004e5540) and the
// closing lifetime registration BSP_FrameHook_ConstructAndRegister (007372a0).
// BSP_Settings_ApplyAll (008d5b50) is analysed in docs/APP_INIT_GAME_ENTRY.md
// but not reconstructed: it drives D3D9 presentation and XLive directly.
namespace bsp {

// Value written to game+5D4h. Only 1, 2 and 10 survive the initialize tail;
// 3 is written transiently by BSP_Game_OnInit (004e3aa0) and is immediately
// overwritten by the caller. These are recovered values, not recovered names.
enum class GameStartupState : std::uint32_t {
    kLogoSequence = 1, // 004e57c1, neither skipLogos nor .scn was given
    kTitleScreen = 2, // 004e5819 and 004c9a70, the front end
    kMissionInit = 3, // 004e3aa0 entry only, never observed by the frame loop
    kScenarioLoad = 10, // 004e578c, a .scn path was on the command line
};

// The four command-line switches BSP_Game_BeginStartupSequence writes. Each
// field is one global; the addresses are the native storage.
struct GameStartupFlags {
    bool skip_logos{false}; // 00e188ac, a byte
    bool skip_title{false}; // 00e198cc, a byte read back by 004c9a70
    bool skip_briefings{false}; // 00e18d91, a byte
    std::uint32_t lockit_mode{0}; // 00f8bc50, a dword: 0 default, 1 mark, 2 raw
};

// The switch test compiled at 004e55c9 and repeated seven times. The native
// code guards on a null command line, calls strstr, and then compares the
// offset of the hit against -1, which cannot hold for a non-null result. The
// dead comparison is reproduced so the recovered guard order stays visible.
bool startup_switch_present(const char* command_line, const char* token) noexcept;

// Integration boundary for the game systems this tail reaches that are not
// reconstructed. Every method is one native call site and they are declared in
// the order BSP_Game_BeginStartupSequence issues them. Nothing has a default
// implementation: none of these is a stand-in for unrecovered game behaviour.
struct GameStartupSystems {
    virtual ~GameStartupSystems() = default;

    // 004e555e: 00f8abec receives the address of 004ceb40.
    virtual void install_startup_callback() = 0;
    // 004e556a..004e559d: operator new(34h), constructor 004f8af0 into
    // 00e18d48, then the object's virtual slot +10h. A failed allocation
    // stores null and the native code still dereferences it.
    virtual void create_startup_controller() = 0;
    // 004e55a5: 004f8970 with ECX = 00e18d48 and the callback 004f89d0.
    virtual void register_startup_handler() = 0;
    // 004e55af: 0109cee8, nonzero when the full logo sequence is forced.
    virtual bool logo_sequence_forced() = 0;
    // game+7168h, null when no command line was captured.
    virtual const char* command_line() = 0;
    // 004e574c: BSP_Game_OnInitOnce (004dd5b0) with the char argument.
    virtual void on_init_once(bool first_time) = 0;
    // 004e5753: BSP_Game_OnInitTitle (004c9a70).
    virtual void on_init_title() = 0;
    // 004e5758: singleton getter 004dc200, then 008fafc0 on the result.
    virtual void notify_title_ready() = 0;
    // 004e5770: BSP_Game_OnInit (004e3aa0).
    virtual void on_init_mission() = 0;
    // 004e5777: BSP_Game_DrainStateRequests (004c88a0).
    virtual void drain_state_requests() = 0;
    // 004e5796: BSP_Game_EnqueueStateRequest (004d3ed0) on game+5D8h.
    virtual void enqueue_state_request(GameStartupState state) = 0;
    // 004e57cb..004e5802: operator new(80h), BSP_LogoSequence_Construct
    // (006851e0) into 00e198a4, then the object's virtual slot +4h.
    virtual void create_logo_sequence() = 0;
};

// BSP_Game_BeginStartupSequence (004e5540). __thiscall, ECX = the GGame object,
// no stack arguments. Returns the value it writes to game+5D4h, which is the
// state the frame loop observes on its first pass.
GameStartupState game_on_init(GameStartupSystems& systems, GameStartupFlags& flags);

// Interface for the singleton lifetime manager reached at 0073e4ea. The
// critical section and its recursion counter are held by the manager, so the
// lock is modelled as a scope rather than as a handle.
struct SingletonLifetimeManager {
    virtual ~SingletonLifetimeManager() = default;
    virtual void lock() = 0; // EnterCriticalSection on manager+10h, ++manager+10h[18h]
    virtual void unlock() = 0; // --manager+10h[18h], LeaveCriticalSection
    virtual void register_object(void* object) = 0; // 00bd0c30
};

// Projection of the 0Ch object BSP_Application_Initialize builds last. The
// native layout is a vptr at +0, a byte at +4 and a float at +8;
// BSP_FrameHook_ConstructAndRegister writes base vptr 00cfea68 and the caller
// then overwrites it with the derived 00cfea98.
struct FrameHook {
    std::uint32_t vtable{0};
    bool flag{false};
    float value{0.0F};
};

inline constexpr std::uint32_t kFrameHookBaseVtable = 0x00CFEA68U;
inline constexpr std::uint32_t kFrameHookDerivedVtable = 0x00CFEA98U;

// BSP_FrameHook_ConstructAndRegister (007372a0) followed by the derived
// constructor inlined at 0073e4ef, which is byte-identical to 00737830.
// published receives the object, modelling the store to 00f88c20.
FrameHook* frame_hook_construct(FrameHook& hook, SingletonLifetimeManager& manager,
    FrameHook** published);

} // namespace bsp
