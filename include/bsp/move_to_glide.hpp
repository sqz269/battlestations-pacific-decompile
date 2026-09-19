#pragma once

// `009C18C0 BSP_BotStateMoveTo_Tick` step 5, the move-to glide slope, as a pure rule over
// explicit inputs. docs/DIVE_BOMB_APPROACH.md carries the evidence.
//
// The body is `__thiscall(this, float dt)`, `RET 4`, Ghidra body 009C18C0-009C1BB3. `this+4h`
// is the approach, `this+2Ch` the target entity, and `this+30h`/`+34h`/`+38h` the three floats
// `009C2AC0` stores at construction and `009BDE80 BSP_BotStateMoveTo_SetRanges` refreshes.
// The state object is the one every bot task's move-to slot holds: `009C2AC0` writes vtable
// `00D20AEC`, whose `+0Ch` tick slot is this body and whose `+1Ch` speed slot is `009C1850`.
//
// Which task supplies which values is the caller's business; this file owns only the
// arithmetic, which is one function and therefore identical for every task that reaches it.
// The torpedo arm's inputs are in docs/TORPEDO_MOVETO_TICK.md, the dive bomb's in
// docs/DIVE_BOMB_APPROACH.md.
//
// Names are hypotheses, not recovered symbols. Nothing here is binary compatible.

namespace bsp {

// Every constant was read from the image at the width of the instruction that loads it:
// `FLD double ptr` for the five gates and `MOVSS`/`FLD float ptr` for the five values. The
// trailing address on each line is the data address, in the form tools/const_width_sweep.py
// audits; the load site is named in the prose after it.
namespace move_to_glide_constant {
inline constexpr double kDistanceEpsilonSq = 1e-10;  // 00CE3820 gate, FLD qword at 009C195C
inline constexpr float kMarginCeiling = 1400.0f;     // 00D1F8D0 FSUBR qword at 009C1A43
inline constexpr float kMarginFloor = 50.0f;         // 00CEB4D4 MOVSS at 009C1A3B, gate 00CE3938
inline constexpr float kDenomOffset = 1000.0f;       // 00CE47A0 FSUB qword at 009C1A75
// The denominator shares the margin's floor: 009C1A89's `JA` leaves XMM0 at the 50.0f
// 009C1A3B loaded, so one value floors both.
inline constexpr float kDenomFloor = 50.0f;          // 00CEB4D4, the same load
inline constexpr float kDenomCap = 2000.0f;          // 00CFFD60 MOVSS at 009C1AA7, gate 00CF0DD8
// 009C1ACC-009C1AF0, the four `00419010 BSP_Math_InterpolateClamped` endpoints.
inline constexpr float kScaleRatioLow = 0.05f;       // 00CE7638 FLD dword at 009C1AEA
inline constexpr float kScaleAtLow = 0.35f;          // 00CF6560 FLD dword at 009C1AE0
inline constexpr float kScaleRatioHigh = 0.4f;       // 00CE7804 FLD dword at 009C1AD6
inline constexpr float kScaleAtHigh = 1.6f;          // 00D06BB4 FLD dword at 009C1ACC
// The near range the dive-bomb task hands `009BDE80` is its far range less this. Also the
// in-range latch's hysteresis band.
inline constexpr float kNearRangeDrop = 100.0f;      // 00D7A220 FSUB qword at 009C8814, 009C7423
}  // namespace move_to_glide_constant

// The values 009C18C0 reads. `near_range_30`/`far_range_34`/`speed_range_38` are the state's
// own three floats; the rest it fetches from the two poses each tick.
struct MoveToGlideInputs {
    float near_range_30{0.0f};    // state+30h
    float far_range_34{0.0f};     // state+34h
    float speed_range_38{0.0f};   // state+38h, 009FBA50's rangeLow
    float target_world_y{0.0f};   // target+100h, the value 009C1904 parks at [ESP+20h]
    float unit_world_y{0.0f};     // unit+100h, 009C1A35
    float planar_distance{0.0f};  // step 1's result, 009FBA50's rangeHigh
};

// What step 5 hands `009FBA50 BSP_PilotBot_CommandCruiseAltitude` at 009C1B17, in its
// argument order. The gain (`class+518h`, `tan(DropAngle)`) and the ceiling belong to
// 009FBA50's own inputs and stay with the caller.
struct MoveToGlideCommand {
    float base{0.0f};        // arg0, 009C1A19/009C1A11
    float range_low{0.0f};   // arg1, 009C1B09 `FLD [EDI+38h]`
    float range_high{0.0f};  // arg2, the planar distance
    float scale{0.0f};       // arg3, 00419010's result
};

// 009C192C-009C1984, complete: the planar separation, zeroed at or below the epsilon.
// This is both the argument of the `+1Ch` speed slot (009C1999, unconditional) and
// 009FBA50's rangeHigh.
float move_to_planar_distance_009c1950(float dx, float dz);

// 009C19F1-009C1AB9, complete. `base = max(far + targetY, near)`, then the margin/denominator
// ratio and its interpolation.
MoveToGlideCommand move_to_glide_009c18c0(const MoveToGlideInputs& in);

}  // namespace bsp
