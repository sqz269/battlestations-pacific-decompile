#pragma once
// The command controller's per-frame update (0071F290, controller vtable +0Ch)
// and the consumers of the controller's permission bytes.
//
// docs/DIRECTOR_UPDATE_ARMS.md holds the evidence. The short version:
//
//  * 0071F290 is the whole per-frame driver of the unit command controller
//    (the "weapon director" at unit+738h). Seven arms in order: a session-live
//    gate that abandons the frame, a reset of slot 0's path vector, the
//    auto-target hold countdown, an idle -> queue-head mode promotion, a
//    begin-command attempt for the override and then for the queue head (a
//    failed attempt terminates that command by raising its stage to 2), and a
//    tail, skipped in session mode 2, that ticks the auto-target object at
//    +38h with the frame delta and then runs the command step vtable[7Ch].
//  * The four "enable" bytes at +220h..+223h have recovered names: the
//    reflection visitor 008360C0 emits them as "artilleryEnabled", "aaEnabled",
//    "torpedoEnabled" and "depthChargeEnabled". +238h is "fireTargetID" and
//    +23Ch is "fireTargetIsPrimary". Those six strings are literals in the
//    image, so unlike everything else here they are recovered, not guessed.
//  * The permission bytes' consumers are 0080DC70 (allowFire && allowMove, two
//    call sites), 009F5610 (allowMove plus the slot-0 category), 008624C0 (the
//    weapon-panel mirror, the only consumer that says what each enable gates),
//    009F1BC0 (the ship AI's per-pass cache), 00720450 (the controller-to-
//    controller copy) and the network snapshot pair 00721280/007214C0 out and
//    00721890/00721980 back.
//
// Every descriptive name here is a hypothesis, not a recovered symbol, except
// the six property strings called out above.
#include <cstddef>
#include <cstdint>

#include "bsp/command_execution.hpp"      // CommandSlot, kDirectorCommandSlotCount
#include "bsp/director_target_gate.hpp"   // update_auto_target_hold, categories

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kControllerUpdateAddress = 0x0071F290u;        // vtable +0Ch
inline constexpr std::uint32_t kControllerUpdateEndAddress = 0x0071F3A4u;     // inclusive
inline constexpr std::uint32_t kControllerBeginCommandAddress = 0x0071F600u;  // vtable +78h base
inline constexpr std::uint32_t kControllerStepAddress = 0x00836920u;          // vtable +7Ch derived
inline constexpr std::uint32_t kControllerRaiseQueueStageAddress = 0x0071D810u;
inline constexpr std::uint32_t kControllerRaiseOverrideStageAddress = 0x0071D9E0u;
inline constexpr std::uint32_t kControllerCopyStateAddress = 0x00720450u;     // vtable +4Ch base
inline constexpr std::uint32_t kDirectorAllowsFireAndMoveAddress = 0x0080DC70u;
inline constexpr std::uint32_t kAutoTargetEnabledAddress = 0x009F5610u;
inline constexpr std::uint32_t kWeaponPanelSyncAddress = 0x008624C0u;
inline constexpr std::uint32_t kDirectorStoreFireTargetAddress = 0x00836240u;
inline constexpr std::uint32_t kDirectorPropertyVisitorAddress = 0x008360C0u;
inline constexpr std::uint32_t kOrphanFireForbiddenPredicateAddress = 0x009F6AD0u;

// ---------------------------------------------------------------------------
// Controller vtable slots this packet settled
// ---------------------------------------------------------------------------
// Base table 00CFDA40, derived director table 00D09F58; 32 slots each.
inline constexpr std::size_t kControllerVtableSlotUpdate = 0x0C;        // 0071F290, not overridden
inline constexpr std::size_t kControllerVtableSlotCopyState = 0x4C;     // 00720450 / 00836680
inline constexpr std::size_t kControllerVtableSlotBeginCommand = 0x78;  // 0071F600 / 00835C70
inline constexpr std::size_t kControllerVtableSlotStep = 0x7C;          // 00BF698E / 00836920

// The one argument vtable[78h] takes, read from the two PUSHes at 0071F33E and
// 0071F360: 0 selects the override pair (+188h, +18Ch), non-zero the queue-head
// pair (+54h, +58h). 0071F600's ledger entry is the source of the mapping.
inline constexpr int kBeginCommandOverride = 0;
inline constexpr int kBeginCommandQueueHead = 1;

// The stage 0071F34A and 0071F36E push. 0071D810/0071D9E0 raise monotonically
// and only stage 2 sends the completion message, so 2 is the terminal stage.
inline constexpr int kCommandStageTerminal = 2;

// The session-mode word at [*(00E188A8) + 1FE4h]. Value 2 is the arm 0071F373
// tests, and the same test guards the message sends inside 0071D810/0071D9E0.
inline constexpr std::size_t kGameSessionModeOffset = 0x1FE4;
inline constexpr int kSessionModeNoSimulation = 2;

// ---------------------------------------------------------------------------
// Controller fields this packet added or corrected
// ---------------------------------------------------------------------------
// The mode, accepted, stage, override and path-array offsets already live in
// command_execution.hpp (kControllerOffCommandMode, kControllerOffQueueAccepted,
// kControllerOffQueueStage, kControllerOffOverrideAccepted,
// kControllerOffOverrideStage, kControllerOffOverrideCommand,
// kControllerOffPathObjects); this packet only adds what they do not cover.
// The hold lives in director_target_gate.hpp as kDirectorOffAutoTargetHold.

// New from 00720180's tail: twenty dwords at +1D0h..+21Fh, every one written
// with the same bit pattern at 0072032E..007203A5. As a float it is about
// -1.004e10, i.e. a "no value" sentinel; the consumer was not found.
inline constexpr std::size_t kControllerOffSentinelBlock = 0x1D0;
inline constexpr int kControllerSentinelDwordCount = 20;
inline constexpr std::uint32_t kControllerSentinelPattern = 0xD01502F9u;

// 007202F3: the dword at +1CCh the base constructor seeds before the sub-kind 2
// message can overwrite it.
inline constexpr std::uint32_t kControllerSubKind2Default = 0x1FF;

// Offsets inside one of the ten 50h-byte path objects, from the update's own
// writes at 0071F2D9/0071F2E6/0071F2F3 and the constructor's at 007203CE..
inline constexpr std::size_t kPathObjectOffVectorX = 0x30;
inline constexpr std::size_t kPathObjectOffOwner = 0x04;
inline constexpr std::size_t kPathObjectSize = 0x50;
inline constexpr std::uint32_t kPathObjectVtable = 0x00CFDB24u;

// ---------------------------------------------------------------------------
// The permission bytes, with the names the image carries
// ---------------------------------------------------------------------------
// 008360C0 builds a {tag, value} record per field and hands the visitor at
// argument 1 a name pointer for each. The four booleans below are the strings
// at 00D09E0C, 00D09E00, 00D09DF0 and 00D09DDC; 00D09DCC and 00D09DB8 name the
// fire-target pair. This closes docs/WEAPON_DIRECTOR.md's open question about
// what sub-kinds 4 and 6 mean.
enum class DirectorWeaponEnable : int {
    kArtillery = 0,    // +220h "artilleryEnabled",   sub-kind 3
    kAntiAircraft = 1, // +221h "aaEnabled",          sub-kind 4
    kTorpedo = 2,      // +222h "torpedoEnabled",     sub-kind 5
    kDepthCharge = 3,  // +223h "depthChargeEnabled", sub-kind 6
};

// The whole permission set one consumer reads. Field order is the controller's.
struct DirectorPermissions {
    bool allow_fire{false};          // +3Ch, controller vtable +64h
    bool allow_move{false};          // +3Dh, controller vtable +68h
    bool artillery_enabled{true};    // +220h
    bool aa_enabled{true};           // +221h
    bool torpedo_enabled{true};      // +222h
    bool depth_charge_enabled{true}; // +223h
};

// 007201AB..00720322: the base constructor zeroes both stance bytes and sets
// all four weapon enables. 008363E0 then raises the two stance bytes when the
// object at +34h answers neither class test, so a plain unit ends up with
// everything permitted.
DirectorPermissions controller_constructed_permissions() noexcept;

// Index a DirectorPermissions weapon enable, in the order the sub-kinds use.
bool weapon_enable(const DirectorPermissions& permissions,
                   DirectorWeaponEnable which) noexcept;

// ---------------------------------------------------------------------------
// The permission consumers, as pure rules
// ---------------------------------------------------------------------------

// 0080DC70, __thiscall(controller) -> bool in AL, RET, body 0080DC70..0080DC81.
// Two call sites: 008171F6 inside 00816E30's entity-command arms
// (docs/ENTITY_COMMAND_ARMS.md) and 009F83CB in the ship AI. Both reach it with
// ECX loaded from a controller, so the two stance bytes are read together and
// either one clear answers false.
bool director_allows_fire_and_move(bool allow_fire, bool allow_move) noexcept;

// 009F5610, __thiscall(autoTarget) -> bool, RET, body 009F5610..009F563C. The
// director arrives as [autoTarget+0Ch], written by 009F6A20 at 009F6AA5. The
// byte it reads is +3Dh, allowMove, not allowFire: a unit forbidden to move is
// also forbidden to pick its own target. `slot0_occupied` is [director+54h] and
// `slot0_category` the answer of that command's vtable[0Ch]; the scan stops at
// slot 0 and never walks the prefix 0071DF70 walks.
bool auto_target_enabled(bool allow_move, bool slot0_occupied,
                         int slot0_category) noexcept;

// 00836240 BSP_WeaponDirector_StoreFireTarget's entry gate, 00836240..00836257.
// __thiscall(director)(entity, force), RET 8, body 00836240..00836299. The
// receive-side twin of 00835860's send-side gate: an unforced store is dropped
// while fireTargetIsPrimary is set and a target is already held.
bool store_fire_target_accepted(bool force, bool fire_target_is_primary,
                                bool fire_target_held) noexcept;

// ---------------------------------------------------------------------------
// 008624C0, the weapon-panel mirror
// ---------------------------------------------------------------------------
// __thiscall(view)(char force), where view+0h is the panel record, view+4h the
// controller, view+8h a countdown and view+0Ch..+10h the previously mirrored
// bytes. Called from 00864FE0 BSP_UnitGunneryAi_Tick at 00865073 and twice from
// 00863A80. This is the only consumer that says what each enable gates.
inline constexpr int kWeaponPanelRefreshPeriod = 10; // 008624EB

struct WeaponPanelMirror {
    int countdown{0};                  // view+8h
    bool last_allow_fire{false};       // view+0Ch
    bool last_torpedo_enabled{false};  // view+0Dh
    bool last_artillery_enabled{false};// view+0Eh
    bool last_aa_enabled{false};       // view+0Fh
    bool last_depth_charge_enabled{false}; // view+10h
};

// What one call decided to do. Each flag is one native branch of 008624C0.
struct WeaponPanelSyncResult {
    bool allow_move_copied{true};  // always: panel+7Ch = controller+3Dh
    bool refresh_all{false};       // the countdown expired or `force` was set
    bool rewrote_fire_enables{false};   // the panel+70h..+7Bh block was rewritten
    bool set_fire_enables{false};       // true = all ones, false = cleared
    bool torpedo_pushed{false};    // 00861D70 ran
    bool artillery_pushed{false};  // 00861CD0 ran
    bool aa_pushed{false};         // 00861D20 ran
    bool depth_charge_pushed{false}; // 00861DC0 ran
};

// The decision half of 008624C0, with the panel writes left to the caller.
// `force` is the char argument. The countdown is decremented first, exactly as
// 008624E5 does, so a fresh mirror refreshes on its first call.
WeaponPanelSyncResult weapon_panel_sync(WeaponPanelMirror& mirror, bool force,
                                        const DirectorPermissions& permissions) noexcept;

// ---------------------------------------------------------------------------
// 0071F290 as a rule table
// ---------------------------------------------------------------------------

// Arm 1, 0071F294..0071F2C1. The session object at controller+34h must exist
// and its four lifecycle bytes must read set / clear / clear / clear, in that
// order. The identical test is inlined at 0077A564, 0077BD8E, 0077C9B5,
// 0077CE66 and 0077CEFB in the session code, which is what makes it the
// session's own predicate rather than anything the controller owns. What the
// four bytes individually mean is contract: unread.
struct SessionLiveFlags {
    bool flag_5c{false}; // must be set
    bool flag_5d{false}; // must be clear
    bool flag_60{false}; // must be clear
    bool flag_5e{false}; // must be clear
};

bool session_is_live(const SessionLiveFlags& flags) noexcept;

// Arm 4, 0071F317..0071F329. Idle plus an occupied slot 0 becomes mode 1; any
// other mode is left alone, and an idle controller with an empty slot 0 stays
// idle. Returns the new mode.
int promote_mode_to_queue_head(int mode, bool slot0_occupied) noexcept;

// Arms 5 and 6, 0071F32A..0071F372. One arm per command kind: while the
// command exists and has not accepted its target, ask vtable[78h] once; a
// refusal terminates that command by raising its stage to the terminal 2.
struct BeginCommandArm {
    bool already_accepted{false}; // +4Ch for the override, +44h for the queue
    bool command_present{false};  // +188h for the override, +54h for the queue
};

// True when the arm should call vtable[78h] at all.
bool begin_command_arm_runs(const BeginCommandArm& arm) noexcept;

// ---------------------------------------------------------------------------
// 0071F290 over an injected host
// ---------------------------------------------------------------------------

// The controller state 0071F290 reads. Everything it writes is reported back in
// CommandControllerUpdateTrace instead of being mutated in place, except the
// hold and the mode, which the routine genuinely updates.
struct CommandControllerUpdateState {
    bool session_present{false};          // [+34h] != 0
    SessionLiveFlags session_flags{};
    bool path_object0_present{false};     // [+1A4h] != 0
    float auto_target_hold{-1.0f};        // +40h
    int mode{0};                          // +30h
    bool slot0_occupied{false};           // [+54h] != 0
    bool override_command_present{false}; // [+188h] != 0
    bool queue_accepted{false};           // +44h
    bool override_accepted{false};        // +4Ch
    int session_mode{0};                  // [*(00E188A8) + 1FE4h]
    bool auto_target_present{false};      // [+38h] != 0
};

// One virtual per native call site of 0071F290. The routine makes exactly four
// kinds of call, and the path-vector reset is a plain store rather than a call,
// so it is a host method only to keep the write out of the pure rules.
struct CommandControllerUpdateHost {
    virtual ~CommandControllerUpdateHost() = default;

    // 0071F2D1..0071F2F3: store the shared default vector (00F87574..7C) into
    // path object 0's +30h, +34h, +38h.
    virtual void reset_path_vector() = 0;

    // vtable[78h] at 0071F340 and 0071F364, __thiscall(controller)(int which),
    // RET 4, returns bool in AL. Base body 0071F600.
    virtual bool begin_command(int which) = 0;

    // 0071D9E0 at 0071F34A, __thiscall(controller)(int stage), RET 4.
    virtual void raise_override_stage(int stage) = 0;

    // 0071D810 at 0071F36E, __thiscall(controller)(int stage), RET 4.
    virtual void raise_queue_stage(int stage) = 0;

    // [controller+38h]->vtable[4](float dt) at 0071F395. On the director this is
    // 009F5DA0, the auto-target think, because 009F6A20 installs vtable
    // 00D21B48 whose slot +4h is 009F5DA0.
    virtual void step_auto_target(float frame_delta) = 0;

    // vtable[7Ch] at 0071F39E, __thiscall(controller), RET 0. 00BF698E on the
    // base (a no-op), 00836920 BSP_WeaponDirector_Step on the director.
    virtual void step_commands() = 0;
};

// Which arms of 0071F290 ran, in the order the routine visits them.
struct CommandControllerUpdateTrace {
    bool session_gate_passed{false};
    bool path_vector_reset{false};
    bool hold_decremented{false};
    float auto_target_hold{-1.0f};
    int mode{0};
    bool mode_promoted{false};
    bool override_begin_attempted{false};
    bool override_terminated{false};
    bool queue_begin_attempted{false};
    bool queue_terminated{false};
    bool auto_target_stepped{false};
    bool commands_stepped{false};
};

// 0071F290 whole, __thiscall(controller)(float frameDelta), RET 4 at 0071F3A2,
// body 0071F290..0071F3A4.
CommandControllerUpdateTrace run_command_controller_update(
    const CommandControllerUpdateState& state, float frame_delta,
    CommandControllerUpdateHost& host);

} // namespace bsp
