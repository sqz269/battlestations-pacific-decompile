#pragma once
// The weapon director's auto-target hold timer at director+40h, the gate that
// reads it (0071DF70 BSP_WeaponDirector_AcceptsNewTarget) and the override-command
// send that reads it the other way (0071D980).
//
// docs/DIRECTOR_TARGET_GATE.md holds the evidence. The short version, because it
// corrects docs/GAME_EXECUTABLE.md's milestone 2n:
//
//  * `director+40h` is a float countdown in seconds, not an unwritten field. The
//    command controller's base constructor 00720180 stores -1.0f into it at
//    00720225; the per-frame update 0071F290 (controller vtable +0Ch) subtracts
//    the frame delta while the value is >= 0.0f, so an expired hold settles just
//    below zero and the -1.0f sentinel never moves.
//  * The only site that arms it is the `cleartarget` arm of
//    00816E30 BSP_UnitInstance_ApplyEntityCommand: at 00817031 it stores 3.0f
//    (00CE3854) into `[unit+738h]+40h` after clearing the fire target.
//  * 0071DF70 rejects a new auto-target while the hold is **greater than** 0.0f.
//    The milestone-2n brief had the comparison inverted: `JBE` at 0071DF7C jumps
//    to the slot scan, and the fall-through at 0071DF7E is `XOR AL,AL; RET`. With
//    the constructed -1.0f the float half of the gate passes.
//
// Every descriptive name here is a hypothesis, not a recovered symbol.
#include <cstddef>
#include <cstdint>

#include "bsp/command_execution.hpp" // CommandSlot, kDirectorCommandSlotCount

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kDirectorAcceptsNewTargetAddress = 0x0071DF70u;
inline constexpr std::uint32_t kDirectorSendOverrideCommandAddress = 0x0071D980u;
inline constexpr std::uint32_t kDirectorUpdateAddress = 0x0071F290u;      // vtable +0Ch
inline constexpr std::uint32_t kCommandControllerConstructAddress = 0x00720180u;
inline constexpr std::uint32_t kDirectorHoldArmSiteAddress = 0x00817031u; // in 00816E30

// ---------------------------------------------------------------------------
// The field and its three constants
// ---------------------------------------------------------------------------
// +40h sits between the two stance bytes (+3Ch allowFire, +3Dh allowMove) and
// the queue-stage pair (+44h, +48h) of command_execution.hpp.
inline constexpr std::size_t kDirectorOffAutoTargetHold = 0x40;

// 00D7A260, the value 00720225 stores. Reads as "no hold": the update leaves a
// negative value alone, so this one never changes until something arms it.
inline constexpr float kAutoTargetHoldDisabled = -1.0f;

// 00D7A218, the right-hand side of the COMISS at 0071DF75 and at 0071F2FD.
inline constexpr float kAutoTargetHoldThreshold = 0.0f;

// 00CE3854, the value the `cleartarget` arm stores at 00817031.
inline constexpr float kClearTargetHoldSeconds = 3.0f;

// ---------------------------------------------------------------------------
// Command categories, `command->vtable[0Ch]()`
// ---------------------------------------------------------------------------
// The 0/1/2/3 split is what the binary proves; the labels are
// docs/SCENE_COMMAND_TYPES.md's reading. 0071DF70 and 008358D0 both treat 1 and
// 2 as one set: a command that owns the unit's fire target.
enum class DirectorCommandCategory : int {
    kNone = 0,       // cleartarget, clearorders, Leave, disband
    kGunnery = 1,    // settarget, artillery, strafe, dogfight
    kWeaponRun = 2,  // torpedo, divebomb, levelbomb, dropkamikaze, depthcharge,
                     // rocket, kamikaze, attackmove
    kMovement = 3,   // the remaining ten
};

// ---------------------------------------------------------------------------
// The message the override send builds
// ---------------------------------------------------------------------------
// 0071C830 BSP_GameUnitSetCommandMessage_Construct's fourth argument, the byte
// it stores at message+20h. 00721A40 BSP_WeaponDirector_ApplyGameUnitMessage
// branches on it at 00721B36: non-zero takes the queue path
// (director vtable +60h, 008358D0 SetCommand), zero takes the override path
// (0071E7F0 SetOverrideCommand).
inline constexpr std::uint8_t kSetCommandMessageOverride = 0;
inline constexpr std::uint8_t kSetCommandMessageQueue = 1;

// The routing-flag override both director senders pass to 0077C2A0.
inline constexpr int kDirectorRouteFlags = 7;

// ---------------------------------------------------------------------------
// The rules, as pure functions
// ---------------------------------------------------------------------------

// 0071DF70 first gate, 0071DF70..0071DF80. True while the controller is holding
// off auto-acquisition. An unordered compare (a NaN hold) counts as not holding,
// because COMISS sets CF and ZF and the JBE is taken.
bool auto_target_hold_active(float hold) noexcept;

// 0071DFB0 / 0071DFB5. A queued gunnery or weapon-run command owns the fire
// target, so the auto-target think must not replace it.
bool command_category_owns_fire_target(int category) noexcept;

// 0071DF88..0071DF9E. The count of leading occupied slots: the scan stops at the
// first slot whose command pointer is null and never looks past it, so a gap
// hides every slot behind it from the category loop.
int occupied_command_slot_prefix(
    const CommandSlot (&slots)[kDirectorCommandSlotCount]) noexcept;

// 0071DF70 whole, as one predicate. `categories[i]` is what slot i's command
// answered from `vtable[0Ch]`; only the first
// `occupied_command_slot_prefix(slots)` entries are read.
bool director_accepts_new_target(
    float hold, const CommandSlot (&slots)[kDirectorCommandSlotCount],
    const int (&categories)[kDirectorCommandSlotCount]) noexcept;

// 0071F2F8..0071F316, the hold half of the controller's per-frame update. The
// subtraction runs only while the value is >= 0.0f, which is what keeps the
// -1.0f sentinel and an expired hold where they are.
float update_auto_target_hold(float hold, float frame_delta) noexcept;

// 0071D99E. The override send is skipped unless the hold is strictly below
// 0.0f, so the frame a hold reaches exactly zero still suppresses the send.
bool may_send_override_command(float hold) noexcept;

// ---------------------------------------------------------------------------
// 0071DF70 over an injected host
// ---------------------------------------------------------------------------
// One virtual per native call site of 0071DF70. The gate makes exactly one kind
// of call, `command->vtable[0Ch]()` at 0071DFAE, and makes it at most ten times.
struct DirectorTargetGateHost {
    virtual ~DirectorTargetGateHost() = default;

    // vtable[0Ch] at 0071DFAE, __thiscall(command), RET 0, returns int.
    virtual int command_category(std::uint32_t command) = 0;
};

// The same predicate, asking the host for each category in slot order and
// stopping at the first answer of 1 or 2, exactly as 0071DFA7..0071DFC2 does.
bool director_accepts_new_target(
    float hold, const CommandSlot (&slots)[kDirectorCommandSlotCount],
    DirectorTargetGateHost& host);

// ---------------------------------------------------------------------------
// 0071D980 over an injected host
// ---------------------------------------------------------------------------
// __thiscall(director, const CommandClass* command, const SceneCommandTarget*
// target), RET 8 at 0071D9DD. Body 0071D980..0071D9DD, one SEH frame
// (00C85108) around the stack-built message.
struct DirectorOverrideCommandRequest {
    float auto_target_hold{kAutoTargetHoldDisabled}; // [director+40h]
    std::uint32_t session{0};                        // [director+34h]
    std::uint32_t command{0};                        // argument 1
    std::uint32_t command_target{0};                 // argument 2
};

// One virtual per native call site of 0071D980.
struct DirectorOverrideCommandHost {
    virtual ~DirectorOverrideCommandHost() = default;

    // 0071C830 at 0071D9B4, on a stack message; returns the message.
    virtual std::uint32_t build_set_command_message(std::uint32_t command,
                                                    std::uint32_t command_target,
                                                    std::uint8_t flag) = 0;

    // 0077C2A0 at 0071D9C9, ECX = [director+34h], (message, routeFlags, out).
    virtual void route_message(std::uint32_t session, std::uint32_t message,
                               int route_flags, std::uint32_t out) = 0;
};

// Returns whether the message was sent. The native routine returns void; the
// bool is the reconstruction's way of reporting which branch ran.
bool send_override_command(const DirectorOverrideCommandRequest& request,
                           DirectorOverrideCommandHost& host);

} // namespace bsp
