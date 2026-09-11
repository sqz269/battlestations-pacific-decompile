#pragma once
#include <cstddef>
#include <cstdint>

// The countdown at 00F874B8 that 008079B0 maintains, and the three-slot recon
// refresh it drives.
//
// docs/FIXED_STEP_COUNTDOWN.md carries the evidence. 008079B0 is the first of
// the sixteen per-step calls of 00875BB0; bsp/in_mission_subsystem_tick.hpp
// declares that call as FixedStepHost::advance_step_countdown_008079b0 and
// kFixedSimulationStepFloat (0.05f, the argument). Neither is redefined here.
//
// Every name below is a hypothesis, not a recovered symbol. The four strings
// "recon", "own", "enemy", "neutral" and "unknown" are constants in the image.

namespace bsp {

// ---------------------------------------------------------------------------
// Constants, all read from the image
// ---------------------------------------------------------------------------

// 00D7A2B0, the double 3.0 the expired countdown is reloaded with. It is the
// image's shared 3.0 literal; 008079CE is the read that matters here.
inline constexpr double kReconRefreshPeriodSeconds = 3.0;

// 00D08E6C, stored into the countdown by 00807A5E and 00803B88 to make the next
// step expire it.
inline constexpr float kReconRefreshExpireNow = -1.0e-4f;

// The globals, as addresses: the countdown and the three-slot pointer table.
// The loop bound at 00807A30 is 00F874C8, so the table is exactly three slots.
inline constexpr std::uint32_t kReconCountdownAddress = 0x00f874b8;   // 008079B0
inline constexpr std::uint32_t kReconSlotTableAddress = 0x00f874bc;   // 008079F7
inline constexpr std::uint32_t kReconSlotTableEndAddress = 0x00f874c8; // 00807A30
inline constexpr std::size_t kReconSlotCount = 3;

// One slot object. 008053C0 allocates it, 008050E0 constructs it, 00805240
// destroys it and clears its own table entry through the index at +28h.
inline constexpr std::size_t kReconSlotSize = 0x12a0;          // 008053E4 PUSH 12A0h
inline constexpr std::size_t kReconSlotDirtyOffset = 0x25;     // 00807A0D, 00803BAB
inline constexpr std::size_t kReconSlotIndexOffset = 0x28;     // 00805272
// Three arrays of 61h elements of 0Ch bytes, built by the MSVC array-ctor
// helper 00BF7CD1 at 00805144, 00805163 and 00805182.
inline constexpr std::size_t kReconSlotArrayOffsets[3] = {0x34, 0x4c0, 0x94c};
inline constexpr std::size_t kReconSlotArrayLength = 0x61;     // 0080512A PUSH 61h
inline constexpr std::size_t kReconSlotArrayStride = 0x0c;     // 0080512C PUSH 0Ch
// The five {count, head, tail} triples 008050E0 zeroes at 0080518D..008051DB
// and 008073C0 clears at 008073D0..00807416. bsp/local_player_unit_lists.hpp
// declares the same five as kUnitRegistryTripleOffsets for the registry
// 004C3CB0 walks; they are not redeclared here.

// ---------------------------------------------------------------------------
// The countdown rule, 008079B0..008079ED
// ---------------------------------------------------------------------------

struct ReconCountdown {
    float remaining{0.0f}; // 00F874B8
};

// Subtract the step, then decide. Returns true when the pass runs, in which
// case `remaining` has been reloaded to max(remaining + 3.0f, 0.0f); returns
// false and leaves the decremented value alone otherwise.
//
// The native order is exactly this: the subtraction is stored back before the
// comparison (008079BA), the 3.0 is added in the x87 register and rounded to
// float on the way to the stack slot (008079CE, 008079D4), and the clamp
// compares that rounded value against 0.0f (008079DC). A NaN countdown takes
// the early exit, because JBE is taken on unordered.
bool step_recon_countdown_008079b0(ReconCountdown& countdown, float step) noexcept;

// 00803B80..00803B90 and 00807A5E: arm the pass for the next step.
void arm_recon_refresh_00803b80(ReconCountdown& countdown) noexcept;

// 006F23C0..006F23CB and 006F4E33: the same arming, one step later, because a
// zero countdown only goes negative after the next subtraction.
void clear_recon_countdown_006f23c0(ReconCountdown& countdown) noexcept;

// ---------------------------------------------------------------------------
// The pass, 008079ED..00807A3A
// ---------------------------------------------------------------------------

struct ReconRefreshHost {
    virtual ~ReconRefreshHost() = default;

    // The slot pointer at 00F874BC + index*4. Null slots are skipped whole
    // (00807A02 TEST ESI,ESI).
    virtual bool slot_present(std::size_t index) = 0;

    // The byte at slot+25h, read at 00807A0D and cleared at 00807A29 whatever
    // it was.
    virtual bool slot_dirty(std::size_t index) = 0;
    virtual void clear_slot_dirty(std::size_t index) = 0;

    // 00807A08 -> 008073C0, ECX = the slot. Clears the slot's five
    // {count, head, tail} triples and rebuilds them.
    virtual void rebuild_slot_008073c0(std::size_t index) = 0;

    // 00807A24 -> 00806B10, ECX = the slot, one stack argument. Rewrites the
    // Lua subtree recon[slot+28h] -> {own, enemy, neutral, unknown} through
    // BSP_LuaInstance_PushGlobalTableIfNil, BSP_ReconTableScope_ConstructIndexed
    // and BSP_LuaInstance_PopTable. docs/RECON_VALUES.md has the table shape.
    // Only reached for a dirty slot.
    virtual void publish_recon_report_00806b10(std::size_t index,
                                               std::uint32_t lua_instance) = 0;

    // [[00E188A8 + 1A08h] + 4], read at the call site (00807A13..00807A21): the
    // Lua instance whose +4h holds the lua_State*, the same owner
    // docs/RECON_VALUES.md resolves for install_recon_values_00803a40.
    virtual std::uint32_t recon_lua_instance() = 0;
};

// 008079B0 whole: the countdown rule, then the three slots in index order.
// Returns true when the pass ran this step.
bool run_recon_refresh_008079b0(ReconCountdown& countdown, float step,
                                ReconRefreshHost& host);

// 00807A50..00807A6B: force the pass now. Sets the countdown to
// kReconRefreshExpireNow and runs 008079B0 with a zero step, so the pass always
// executes and the countdown comes back at just under the full period.
void force_recon_refresh_00807a50(ReconCountdown& countdown, ReconRefreshHost& host);

// ---------------------------------------------------------------------------
// 008053C0, the lazy slot creation
// ---------------------------------------------------------------------------
// __fastcall void(int index): create the slot only when its table entry is
// null. Returns true when this call created it, which is what the caller of a
// reconstruction needs to know; the native routine returns nothing.
bool ensure_recon_slot_008053c0(bool slot_present) noexcept;

} // namespace bsp
