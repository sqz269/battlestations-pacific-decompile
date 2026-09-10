#pragma once
#include <cstddef>
#include <vector>

#include "bsp/frontend_states.hpp"

// Packet front_end_screen_sets. Addresses 004F8710, 004D8C00, 004F83B0,
// 00687330, with the supporting bodies 004F7620 (the recompute), 004F8530,
// 004F85D0, 004F8670, 004F87B0 (the sibling level setters), 004F7210
// (vector::size), 004F81D0 (vector::operator=) and 004C4300 (the input-context
// half). Everything here is a hypothesis reconstructed from the listing; no
// symbol was recovered.
namespace bsp {

// ---------------------------------------------------------------------------
// The layered screen-set stack, 00E18CF8..00E18D47
// ---------------------------------------------------------------------------

// Five 16-byte std::vector<int> objects, level 1 at 00E18CF8 through level 5 at
// 00E18D38. Each holds screen ids, which are indices into the 95-slot registry
// FrontEndScreenTable (00E18B60). 004F7620 walks them from level 5 down to
// level 1, so a higher level covers a lower one.
inline constexpr int kFrontEndScreenSetLevels = 5;
inline constexpr int kFrontEndScreenSetLowestLevel = 1;
inline constexpr int kFrontEndScreenSetHighestLevel = 5;

// One setter per level; each copies its varargs list into the level's vector,
// raises the dirty byte 00E18CDC and calls 004F7620.
inline constexpr unsigned kFrontEndScreenSetSetters[kFrontEndScreenSetLevels] = {
    0x004F8530u, // level 1, vector 00E18CF8
    0x004F85D0u, // level 2, vector 00E18D08
    0x004F8670u, // level 3, vector 00E18D18
    0x004F8710u, // level 4, vector 00E18D28; the one every manager calls
    0x004F87B0u, // level 5, vector 00E18D38
};

// 00E08310. Cleared to 1 at the head of every 004F7620 run and raised to the
// level of the first listed screen whose virtual +8h returns true. Screens
// below the resulting floor are not requested. BSP_Game_Render tests it against
// 1 at 004CA521, so 1 means "no front-end screen is covering the scene".
inline constexpr int kFrontEndOcclusionLevelNone = 1;

struct FrontEndScreenSetStack {
    // Index 0 is level 1. Ids are registry slots; out-of-range ids are stored as
    // the native does and rejected at recompute time.
    std::vector<int> levels[kFrontEndScreenSetLevels]{};
    int occlusion_level{kFrontEndOcclusionLevelNone}; // 00E08310
    bool dirty{false}; // 00E18CDC, raised by every setter, cleared by the pump
};

// The two per-screen predicates the recompute calls. Both are vtable slots of
// the screen base 00CEAE54 whose base implementations (004F7570 at +4h and
// 004F7580 at +8h) are `xor al,al; ret`, so a screen opts in by overriding.
struct FrontEndScreenSetHostBindings {
    virtual ~FrontEndScreenSetHostBindings() = default;
    // Vtable +4h. True keeps the screen's requested byte out of the recompute
    // entirely: neither pass writes it.
    virtual bool screen_manages_own_visibility(int slot) = 0;
    // Vtable +8h. True at level N raises the occlusion floor to N, which drops
    // every screen listed below N.
    virtual bool screen_occludes_lower_levels(int slot) = 0;
};

// 004F7620, __cdecl(void), RET. Recomputes FrontEndScreen::wanted for all 95
// slots from the five level vectors. Order matters and is preserved: the
// occlusion floor is raised while descending, and the clear pass runs to
// completion before the set pass begins.
void recompute_front_end_screen_requests_004f7620(FrontEndScreenSetStack& stack,
    FrontEndScreenTable& table, FrontEndScreenSetHostBindings& bindings);

// The body shared by 004F8530/004F85D0/004F8670/004F8710/004F87B0. `level` is
// 1..5. An empty list clears that level, which is how "hide everything" is
// expressed. The native call is __cdecl varargs terminated by a zero id, so a
// zero anywhere in `ids` ends the list; that truncation is reproduced here.
void set_front_end_screen_set_level(FrontEndScreenSetStack& stack, FrontEndScreenTable& table,
    FrontEndScreenSetHostBindings& bindings, int level, const int* ids, std::size_t count);

// 004F8710 itself: level 4. Every manager Activate and every +10h override goes
// through this one.
void set_front_end_screen_set_004f8710(FrontEndScreenSetStack& stack, FrontEndScreenTable& table,
    FrontEndScreenSetHostBindings& bindings, const int* ids, std::size_t count);

// ---------------------------------------------------------------------------
// The input-context half, 004D8C00 and 004C4300
// ---------------------------------------------------------------------------

// The game object (00E188A8) carries the same shape at +560h + level*10h: four
// 16-byte vectors for levels 1..4, level 4 at +5A0h. The ids are input-context
// indices in the input manager's level array (00A92290 reads
// [manager+10h][id], 00A933F0 writes it), not screen ids, and 004C4300 walks
// contexts 1..19h when it clears a level.
inline constexpr int kGameInputContextSetLevels = 4;
inline constexpr int kGameInputContextCount = 0x1A; // exclusive bound, ids 1..19h
inline constexpr unsigned kGameInputContextSetSetters[kGameInputContextSetLevels] = {
    0x004D8A50u, // level 1, game+570h
    0x004D8AE0u, // level 2, game+580h
    0x004D8B70u, // level 3, game+590h
    0x004D8C00u, // level 4, game+5A0h; the one every manager calls
};

struct GameInputContextSetStack {
    std::vector<int> levels[kGameInputContextSetLevels]{};
    // [manager+10h], indexed by context id. 004C4300 only ever raises a level.
    int context_level[kGameInputContextCount]{};
};

// 004C4300, __thiscall(game, int level), RET 4. For `level` down to 1: drop
// every context currently sitting at that level to 0, then raise every context
// listed at that level to it, but only when its current level is lower.
void apply_input_context_levels_004c4300(GameInputContextSetStack& stack, int level);

// 004D8C00, __cdecl(game, ids..., 0), RET. Stores the list at level 4 and runs
// 004C4300(game, 4). The backwards scan at 004D8C84 that looks for the topmost
// non-empty level leaves its result unused and is not reproduced.
void set_game_input_context_set_004d8c00(GameInputContextSetStack& stack,
    const int* ids, std::size_t count);

// ---------------------------------------------------------------------------
// 00687330, the multi-menu interface-id to screen-id map
// ---------------------------------------------------------------------------

// Jump table at 006873EC, 1Dh entries covering interface ids 3..1Fh; every other
// id falls to 006873E7 and returns 0. Id 0 is the empty screen set, so an
// unmapped id hides everything.
inline constexpr int kMultiMenuInterfaceIdFirst = 0x03;
inline constexpr int kMultiMenuInterfaceIdLast = 0x1F;
inline constexpr int kFrontEndScreenIdNone = 0;

// 00687330. RET 4 with the id at [ESP+4]; 00687838 also loads ECX with the
// manager, so this is more likely a __thiscall member that ignores `this` than
// a true __stdcall free function.
int multi_menu_screen_id_00687330(int interface_id) noexcept;

// The identity map the main menu override 00685820 uses for interface ids
// 1..0Bh, and the +1 map the options override 00689820 uses for 0Ch..0Eh.
inline constexpr int kMainMenuInterfaceIdFirst = 0x01;
inline constexpr int kMainMenuInterfaceIdLast = 0x0B;
inline constexpr int kOptionsMenuInterfaceIdFirst = 0x0C;
inline constexpr int kOptionsMenuInterfaceIdLast = 0x0E;
int main_menu_screen_id_00685820(int interface_id) noexcept;
int options_menu_screen_id_00689820(int interface_id) noexcept;

// ---------------------------------------------------------------------------
// 004F83B0, the per-screen commit
// ---------------------------------------------------------------------------

// One method per native call site of 004F83B0. The screen's virtual +24h fills
// a heap vector of child pointers; each non-null child gets its own virtual
// +34h called with the screen's applied byte.
struct FrontEndScreenCommitHost {
    virtual ~FrontEndScreenCommitHost() = default;
    // Vtable +24h. The base 004F75D0 is `ret 4`, so a screen with no children
    // returns the list untouched.
    virtual void collect_screen_children(int slot, std::vector<void*>& children) = 0;
    // Child vtable +34h, the GUI element visibility setter.
    virtual void set_child_visible(void* child, bool visible) = 0;
};

// 004F83B0, __thiscall(screen), RET, SEH handler 00C684D8. Pushes the screen's
// applied byte down to its children.
void commit_front_end_screen_visibility_004f83b0(const FrontEndScreen& screen, int slot,
    FrontEndScreenCommitHost& host);

// ---------------------------------------------------------------------------
// The manager-side sequence
// ---------------------------------------------------------------------------

// What a +10h override does once 00684600 has accepted the request: map the
// interface id to a screen id, publish the one-element screen set at level 4,
// then publish the one-element input-context set {1} at level 4. Screen id 0
// publishes the empty set, which is the same thing 00683AA0 does on deactivate.
inline constexpr int kFrontEndManagerInputContext = 0x01; // 0068584D and friends

void publish_front_end_manager_screen_set(FrontEndScreenSetStack& screens,
    FrontEndScreenTable& table, FrontEndScreenSetHostBindings& bindings,
    GameInputContextSetStack& contexts, int screen_id);

// 00683AA0's tail: 004F8710(0) and 004D8C00(game, 0), both empty.
void clear_front_end_manager_screen_set(FrontEndScreenSetStack& screens,
    FrontEndScreenTable& table, FrontEndScreenSetHostBindings& bindings,
    GameInputContextSetStack& contexts);

} // namespace bsp
