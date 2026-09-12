#pragma once
#include <cstdint>

#include "bsp/plane_ground_ops.hpp"  // PlaneControllerMode

// Two rules settled by packet cc2_loose_ends_2. docs/GAMEPLAY_LOOSE_ENDS_2.md carries the
// evidence, the coverage notes and the corrections to earlier docs.
//
// Every name here is a hypothesis, not a recovered symbol, and nothing in this header is a
// binary-compatible layout. The offset constants are the native ones.
namespace bsp {

// ---------------------------------------------------------------------------
// 1. What ctl+FCh gates.
//
// `ctl` is unit+AB0h. BSP_PlaneFlightController_Construct (007D7EA0-007D7FE7) leaves the
// field at 0 (007D7FDA writes EBX and the only EBX write in the body is 007D7EA7 XOR
// EBX,EBX). Each of the three laws then writes its own value: 007DC841 = 0 free flight,
// 007DCD24 = 1 ground roll, 007DCDDC = 2 water surface. No writer stores anything else, so
// the "other" row below is unreachable in the shipped image; it is kept because three
// readers have an explicit arm for it.
//
// The six readers found by a disp32 FCh scan over 007D7000-007DE000 (which returns eleven
// operand references and no others) fall into the five gates this struct records.
// ---------------------------------------------------------------------------
struct PlaneControllerModeGates {
    // 007DA8D9 CMP [ESI+FCh],1 / JNE 007DA8EB / MOVSS [ESP+18h],XMM2 in
    // BSP_PlaneFlight_ControlRateLaw. The store is reached only on equality, so the roll
    // accumulator survives in every mode BUT GroundRoll. (docs/PLANE_FLIGHT.md line 163 had
    // the polarity the other way round; see the corrections table.)
    bool roll_rate_survives{true};

    // 007DB6D1 CMP EDX,1 / JNE 007DB744 in BSP_PlaneFlight_CoreLaw: the plane
    // { -h, 0, 1, 0 } written to [ctl+10h]+94h..+A0h, with h = unit+BFCh - classDesc+1FCh.
    // The block is additionally gated on the core law's second stack argument being
    // non-zero (007DB6EA-007DB6F7); that is not a function of the mode.
    bool stores_ground_plane{false};

    // 007DA211 CMP [EBX+FCh],1, consumed at 007DA2B1 JNE 007DA33A in FUN_007D9F60. Also
    // needs unit+BF4h (the object stood on) and unit+3Ch (the visual node) to be non-null.
    bool applies_wheel_height_lift{false};

    // 007DBE0E TEST EAX,EAX / JNE 007DBEAA then CMP EAX,1 / JNE 007DC205: which of the core
    // law's three aerodynamic blocks runs. 0 -> 007DBE1C, 1 -> 007DBEB3, >= 2 -> 007DC205.
    std::uint32_t core_law_block{0x007DBE1Cu};

    // FUN_007DA380's arm for this mode, and the byte out-parameter it publishes. The byte
    // gates the bank-yaw coupling block at 007DA9FB-007DAA7A, which is where the roll
    // accumulator is finally consumed (007DAA70).
    std::uint32_t factor_arm{0x007DA6E6u};
    bool enables_bank_yaw_coupling{false};
};

// The rule table. `mode` is the raw ctl+FCh dword, so values outside the enum take the
// defensive tail arm at 007DA3AB.
PlaneControllerModeGates plane_controller_mode_gates(int mode) noexcept;
PlaneControllerModeGates plane_controller_mode_gates(PlaneControllerMode mode) noexcept;

// FUN_007DA380's tail arm writes this to both float out-parameters ([00D7A238]); the three
// live arms write a speed or 1.0f instead.
inline constexpr float kLooseEnds2ControllerFactorFallback = 0.01f;

// The undercarriage height reference, settled by two independent uses: the ground plane's
// distance (007DB70B FSUB [ECX+1FCh]) and the wheel-height lift (007DA30A-007DA318). It is
// also the reference the lift-off test subtracts.
inline constexpr int kLooseEnds2ClassWheelHeightReference = 0x1FC;

// The wheel-height lift compares against classDesc+1FCh plus this epsilon, the double at
// 00D7A358 (007DA310 FADD qword ptr [00D7A358]).
inline constexpr double kLooseEnds2WheelHeightEpsilon = 0.01;

// ---------------------------------------------------------------------------
// 2. 008637D0 BSP_UnitGunneryAi_AnyCategoryAcceptsTarget.
//
// __thiscall(unit, Entity* target), RET 4, AL. Ghidra body 008637D0-0086383C.
// 009F1BC0 ANDs the result with the director's aaEnabled byte at +221h, so together they
// read "this unit has an anti-aircraft weapon that will engage the target AND the
// director's anti-aircraft enable is set".
// ---------------------------------------------------------------------------

// The global weapon-category list the loop walks, 00E0A510, read one dword per step. The
// loop runs while the entry is below the terminator, so an entry of 0Ch or more ends it
// (008637D5 and 0086381A both CMP ...,0Ch).
inline constexpr std::uint32_t kLooseEnds2WeaponCategoryListAddress = 0x00E0A510u;
inline constexpr int kLooseEnds2WeaponCategoryTerminator = 0x0C;

// The per-category presence byte array on the unit, indexed by the category id
// (008637F2 CMP byte ptr [EAX+EDI+70h],0).
inline constexpr int kLooseEnds2UnitCategoryPresentByteBase = 0x70;

// The object at unit+60h whose vtable[4h] filters categories (007F9-00863802).
inline constexpr int kLooseEnds2UnitGunneryAiField = 0x60;
inline constexpr int kLooseEnds2GunneryAiCategoryFilterSlot = 0x04;

// Integration boundary. Each method is one native call site inside the loop; there are no
// default implementations, because nothing here stands in for unrecovered game behaviour.
struct WeaponCategoryAvailabilityHost {
    virtual ~WeaponCategoryAvailabilityHost() = default;

    // [00E0A510 + index*4]. Returns the terminator (or anything >= it) past the end.
    virtual int category_at(int index) = 0;

    // byte unit[category + 70h] != 0, the per-category presence flag (008637F2).
    virtual bool unit_has_category(int category) = 0;

    // [unit+60h]->vtable[4h](category) (008637F9-00863802). The callee is not read; the
    // slot is recorded above.
    virtual bool gunnery_ai_accepts_category(int category) = 0;

    // 008633D0 BSP_UnitGunneryAi_CategoryAcceptsTarget(unit, category, target), RET 8
    // (0086380E). The target is the routine's own argument and is not modelled here.
    virtual bool category_accepts_target(int category) = 0;
};

// The whole of 008637D0: true at the first category that passes all three tests.
bool any_weapon_category_accepts_target_008637d0(WeaponCategoryAvailabilityHost& host);

}  // namespace bsp
