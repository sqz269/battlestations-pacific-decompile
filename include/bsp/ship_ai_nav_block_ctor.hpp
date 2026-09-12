// 009E4330, the ship AI navigation control block's constructor.
//
// Address: 009E4330, `__thiscall(blk)(unit*)` returning `blk`, `RET 4` at
// 009E46C0, body 009E4330-009E46C2, complete.  Its single caller is
// BSP_ShipAi_BrainRecordConstruct 009F1160 at 009F118D, with
// `LEA ECX,[ESI+8]` at 009F1180, so the block is the brain record's inline
// sub-object at `brain+8h` and is never allocated on its own.  The unit passed
// in is the brain's owner; 009F1192 stores the same pointer at `brain+0AA8h`.
//
// This is the producer of the five per-ship tuning fields the navigation arm
// tail reads and never writes (docs/SHIP_AI_NAVIGATION_ARM_TAIL.md):
// `blk+3C8h`, `blk+3CCh`, `blk+3D4h`, `blk+3D8h` and `blk+604h`, plus
// `blk+3D0h`, `blk+3E4h`, `blk+340h`, `blk+318h`, `blk+1B4h` and `blk+1B8h`.
//
// Evidence: the stored Ghidra listing of 009E4330 read instruction by
// instruction (the pseudocode alone is not usable here: five of the six calls
// take register inputs the decompiler drops, and every float is x87), the
// bodies of 0082E960, 0082E890, 009DFCB0, 00811A30 and 00810F60, and a
// byte-pattern scan of `.text` for every store form at the five displacements.
// Every name below is a hypothesis, not a recovered symbol.
// docs/SHIP_AI_NAV_BLOCK_CTOR.md; reports/ship_ai_nav_block_ctor.json.
#ifndef BSP_SHIP_AI_NAV_BLOCK_CTOR_HPP
#define BSP_SHIP_AI_NAV_BLOCK_CTOR_HPP

#include <cstdint>

#include "bsp/unit_rudder.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Literals, all read from the image.
// ---------------------------------------------------------------------------

// 009E44A0 `FLD1` and the float at 00CE3860.  The two throttle fractions the
// constructor evaluates the class turning-circle curve at.
inline constexpr float kShipAiNavBlockThrottleFull = 1.0f;   // 009E44A0
inline constexpr float kShipAiNavBlockThrottleCruise = 0.9f; // 00CE3860

// 009E44F3 and 009E4533, the double at 00CE3D78.  Seconds of travel at
// MaxSpeed that set the stop radius, and the ratio between the stop radius and
// the restart radius: the same constant is used for both.
inline constexpr double kShipAiNavBlockStopSeconds = 1.5; // 00CE3D78

// 009E4507, the double at 00CE65D0.  The share of the hull length that
// competes with the speed term for the stop radius.
inline constexpr double kShipAiNavBlockStopHullShare = 0.4; // 00CE65D0

// 009E44DB, the double at 00CF1748.  blk+3E4h, the half-hull scale the sector
// shapes divide (docs/SHIP_AI_SECTOR_SCAN.md calls it `E`).
inline constexpr double kShipAiNavBlockHullScale = 0.45; // 00CF1748

// 009E45E3, the double at 00D7A348.  Quarter of the squared hull length in the
// shoulder-offset square root.
inline constexpr double kShipAiNavBlockShoulderHullShare = 0.25; // 00D7A348

// 009E4587 compares against the double at 00D218B8 and 009E4593 substitutes the
// float at 00D218B0.  Both are 4 degrees per second; the class MaxRotAngle can
// never enter the block below it.
inline constexpr double kShipAiNavBlockYawFloorCompare = 0.06981317009776831; // 00D218B8
inline constexpr float kShipAiNavBlockYawFloor = 0.06981317f;                 // 00D218B0

// 009E461F compares against the double at 00CF8850 and 009E462B substitutes the
// float at 00CE77B0.  The floor under blk+318h.
inline constexpr double kShipAiNavBlockLookAheadFloorCompare = 250.0; // 00CF8850
inline constexpr float kShipAiNavBlockLookAheadFloor = 250.0f;        // 00CE77B0

// 009E4399, the float at 00D21528, stored into blk+158h and blk+15Ch.
inline constexpr float kShipAiNavBlockDeadlineSentinel = 1.0e11f; // 00D21528

// 009E4456, the float at 00D7A260, stored into blk+0A20h.
inline constexpr float kShipAiNavBlockLatchClear = -1.0f; // 00D7A260

// 009E448A `PUSH 8`.  The class id the owner is tested for through vtable slot
// 5Ch; class 8 is `MSubmarine` (docs/ENTITY_CLASS_IDS.md).
inline constexpr int kShipAiNavBlockSubmarineClassId = 8; // 009E448A

// 009E468B.  The plan state blk+3F0h starts in.
inline constexpr int kShipAiNavBlockPlanStateInitial = 3; // 009E468B

// 009E43ED `LEA ECX,[EBX+0Bh]` with `SUB ECX,1` / `JNS`: twelve iterations.
// 009E43F9 `ADD EAX,2Ch`: the stride.  009E43E7 `LEA EAX,[ESI+808h]`: the base.
// The same twelve records 009E0270 rebuilds (docs/SHIP_AI_SECTOR_SCAN.md §7).
inline constexpr int kShipAiNavBlockSectorCount = 12;       // 009E43ED
inline constexpr int kShipAiNavBlockSectorStride = 0x2C;    // 009E43F9
inline constexpr int kShipAiNavBlockSectorBase = 0x808;     // 009E43E7

// 009E466F `PUSH 1`.  009E0270 takes a float; this call site pushes the raw
// dword 1, whose float reading is the denormal 1.4e-45, not 1.0f.
inline constexpr std::uint32_t kShipAiNavBlockSectorBuildArgumentRaw = 1u; // 009E466F

// ---------------------------------------------------------------------------
// Inputs.
// ---------------------------------------------------------------------------

// The three descriptor fields the constructor reads through `[unit+538h]`.
// `turn_radius_0520` is the existing `ShipClassAiDerivedMotion::turn_radius_0520`
// projection (include/bsp/ship_ai_throttle_ring.hpp), MaxSpeed / MaxRotAngle.
struct ShipAiNavBlockClassInputs {
    float max_rot_angle_04f8{0.0f}; // 009E4574, the Lua key MaxRotAngle
    float max_speed_0500{0.0f};     // 009E44ED, the Lua key MaxSpeed
    float turn_radius_0520{0.0f};   // read inside 0082E960 at 0082E970
    std::uint32_t reference_0570{0};// 009E44AE, copied to blk+168h
};

// The owner unit the caller passes.  `hull_length_09c8` is the field two
// routines produce (00810F60 at 0081106E and 0081F980 at 0081FA4D): twice the
// larger half-extent of the model box at `[class+50h]` along the long axis, or
// the descriptor's `+A0h` `Length` when the class carries no box.  It is a full
// hull length in metres, not a radius.
struct ShipAiNavBlockUnitInputs {
    bool present{true};                   // 009E4454 CMP EDI,EBX
    // Whatever the host uses to denote this unit.  The native fields blk+3FCh,
    // blk+3F8h, blk+260h and blk+2C8h hold the pointer itself.
    std::uint32_t handle{0};
    float hull_length_09c8{0.0f};         // 009E44D5
    ShipAiNavBlockClassInputs ship_class; // [unit+538h]
};

// ---------------------------------------------------------------------------
// The window 009DFCB0 seeds before the constructor overwrites it.
// ---------------------------------------------------------------------------

// 009DFCB0, `__fastcall(blk+1C4h)` returning its argument, body
// 009DFCB0-009DFEB4.  Called once, at 009E43CC with `ECX = blk+1C4h`
// (009E43A4 `LEA ECX,[ESI+1C4h]`), so its `param_1[0x80..0x88]` stores land on
// `blk+3C4h..blk+3E4h`.  PARTIAL: only that window is projected here; the rest
// of the body (blk+1C4h..blk+3C3h) is not.
//
// The constructor then overwrites +3C8h, +3CCh, +3D0h, +3D4h, +3D8h and +3E4h,
// and 009E0270 overwrites +3C4h, so only +3DCh and +3E0h survive as authored
// defaults for a ship that has just been constructed.
struct ShipAiNavBlockSteeringDefaults {
    float reference_speed_3c4{10.0f};      // 009DFE60, float 00CE38B8
    float turn_circle_full_3c8{400.0f};    // 009DFE88, float 00CFD710
    float turn_circle_cruise_3cc{400.0f};  // 009DFE90, float 00CFD710
    float yaw_rate_3d0{0.15f};             // 009DFE50, float 00CE7818
    float stop_radius_3d4{100.0f};         // 009DFE78, float 00CE3D08
    float start_radius_3d8{100.0f};        // 009DFE70, float 00CE3D08
    float margin_3dc{400.0f};              // 009DFEA0, never overwritten
    float cruise_distance_3e0{1000.0f};    // 009DFEA8, float 00CE3804, kept
    float hull_scale_3e4{50.0f};           // 009DFE98, float 00CEB4D4
};

ShipAiNavBlockSteeringDefaults ship_ai_nav_block_steering_defaults_009dfcb0() noexcept;

// ---------------------------------------------------------------------------
// Pure rules.
// ---------------------------------------------------------------------------

// 0082E960: `__thiscall(descriptor)(float)`, `RET 4` at 0082E97F, ST0 result,
// body 0082E960-0082E981, complete.  `0082E96B` calls the rudder curve
// 0082E890 and `0082E970 FMUL float ptr [ESI+520h]` multiplies its result by
// the descriptor's derived turn radius.  The curve's x axis is the throttle
// setting and its y axis the turning-circle multiplier
// (docs/UNIT_RUDDER_CURVE.md quotes the authors' comment on the three Lua
// rows), so the product is the turning-circle radius in metres at that
// throttle.  00811A30 is the other caller and divides this by the gameplay
// unit scale.
//
// NOTE the call sites push the descriptor with `PUSH ECX` only to make room:
// `FSTP float ptr [ESP]` at 009E44C1 and 009E4552 overwrites that dword with
// the float argument, and ECX still holds the descriptor at the CALL.
float ship_class_turn_circle_radius_0082e960(const ShipAiNavBlockClassInputs& ship_class,
                                             float throttle_fraction,
                                             UnitRudderCurveHost& host);

// 009E4519, `FCOMIP` then `JBE`.  Returns the speed term on an unordered
// compare, which is what the native `JBE` does.
float ship_ai_nav_block_stop_radius_009e4537(float max_speed_0500,
                                             float hull_length_09c8) noexcept;

// 009E458D, `FCOMIP` then `JBE`.  The floor is compared as a double and
// substituted as a float; a NaN MaxRotAngle survives unchanged.
float ship_ai_nav_block_yaw_rate_009e45a9(float max_rot_angle_04f8) noexcept;

// 009E4625, `FCOMIP` then `JBE`, same shape.
float ship_ai_nav_block_look_ahead_009e4648(float twice_turn_circle_full) noexcept;

// ---------------------------------------------------------------------------
// What the constructor writes.
// ---------------------------------------------------------------------------

// Only the fields 009E4330 itself stores.  Offsets are relative to
// `blk` = `brain+8h`.  The two `_memset` runs are recorded as ranges rather
// than members; nothing below overlaps them.
struct ShipAiNavBlockFields {
    // 009E4354, the vtable at 00D21854 whose 009E46D0 entry is the deleting
    // destructor.
    std::uint32_t vtable{0x00D21854u};

    // 009E435F, set before the `_memset` at 009E4363 clears blk+4h..blk+44h,
    // so it survives.  009E4372, likewise, before blk+46h..blk+145h is cleared.
    bool flag_45{true};
    bool flag_146{true};

    float random_phase_148{0.0f}; // 009E4381 then 009E4669, `-uniform(0,1)`
    float value_14c{0.0f};        // 009E4389
    float value_170{0.0f};        // 009E4391
    float deadline_158{kShipAiNavBlockDeadlineSentinel}; // 009E43C4
    float deadline_15c{kShipAiNavBlockDeadlineSentinel}; // 009E43BC
    bool flag_160{false};         // 009E43AA
    std::uint32_t value_164{0};   // 009E43B0
    std::uint32_t class_reference_168{0}; // 009E43B6 then 009E44B4

    bool flag_3e8{false}; // 009E43E1
    bool flag_3e9{false}; // 009E43DB
    bool flag_3ea{false}; // 009E43D5

    // 009E43F0..009E43FF, twelve records of kShipAiNavBlockSectorStride bytes
    // starting at blk+808h: byte +0h = 1, byte +14h = 0, dword +24h = 0.
    struct Sector {
        bool flag_00{true};
        bool flag_14{false};
        std::uint32_t value_24{0};
    };
    Sector sectors[kShipAiNavBlockSectorCount];

    std::uint32_t value_a18{0};                     // 009E4471
    std::uint32_t value_a1c{0};                     // 009E4463
    float latch_a20{kShipAiNavBlockLatchClear};     // 009E4469

    // 009E4417..009E4449, three identical 20h-byte memo records at blk+0A24h,
    // blk+0A44h and blk+0A64h.  docs/SHIP_AI_CLEARANCE_PROFILE.md reads the
    // first two back through 009D57E0 and 00415D70.
    struct Memo {
        bool valid_00{true};      // 009E4417 / 009E4430 / 009E4449
        std::int32_t key_14{-1};  // 009E4411 / 009E442A / 009E4443
        std::uint32_t value_18{0};// 009E4401 / 009E441E / 009E4437
        std::uint32_t value_1c{0};// 009E4407 / 009E4424 / 009E443D
    };
    Memo memos[3];

    bool flag_a90{false};   // 009E4477
    float value_a84{0.0f};  // 009E4673
    float value_a88{0.0f};  // 009E467B
    float value_a8c{0.0f};  // 009E4683

    std::uint32_t owner_3fc{0};     // 009E447D, the unit argument
    std::uint32_t submarine_3f8{0}; // 009E44A2, the unit or null

    // The five the arm tail reads.
    float turn_circle_full_3c8{0.0f};   // 009E44CF
    float turn_circle_cruise_3cc{0.0f}; // 009E4568
    float stop_radius_3d4{0.0f};        // 009E4537
    float start_radius_3d8{0.0f};       // 009E453F
    std::int32_t neighbour_count_604{0};// 009E4659

    float yaw_rate_3d0{0.0f};   // 009E45A9
    float hull_scale_3e4{0.0f}; // 009E44E1
    float look_ahead_340{0.0f}; // 009E45BB
    float shoulder_angle_1b4{0.0f};  // 009E45D1
    float shoulder_offset_1b8{0.0f}; // 009E4606
    float look_ahead_floor_318{0.0f};// 009E4648

    std::uint32_t owner_260{0}; // 009E4600
    std::uint32_t owner_2c8{0}; // 009E460C
    std::uint32_t value_400{0}; // 009E4653

    std::int32_t plan_state_3f0{kShipAiNavBlockPlanStateInitial}; // 009E468B
    bool flag_3ec{true};        // 009E4695
    bool flag_3f4{true};        // 009E469C
    bool early_out_3f5{false};  // 009E46A3
};

// ---------------------------------------------------------------------------
// The host the executable must implement, one method per native call site.
// ---------------------------------------------------------------------------

struct ShipAiNavBlockCtorHost {
    virtual ~ShipAiNavBlockCtorHost() = default;

    // 009E43CC, `ECX = blk+1C4h`.  009DFCB0 seeds the steering sub-block; only
    // its blk+3C4h..blk+3E4h window is projected, and every field of that
    // window except +3DCh and +3E0h is overwritten below.
    virtual ShipAiNavBlockSteeringDefaults seed_steering_009dfcb0() = 0;

    // 009E448E, `unit->vtable[5Ch](class_id)` with class_id 8.  Reached only
    // when the unit argument is non-null (009E4483).
    virtual bool unit_answers_class_5c(int class_id) = 0;

    // 009E44C4 and 009E4555, `ECX = [unit+538h]`, one float argument.
    virtual float class_turn_circle_radius_0082e960(float throttle_fraction) = 0;

    // 009E45F3, the CRT square-root helper 00BF7030.  No domain guard: the
    // argument is negative whenever the hull is longer than twice the cruise
    // turning circle.
    virtual float sqrt_00bf7030(float value) = 0;

    // 009E465F, BSP_Random_UniformFloatRange 00BD2F10 with 0.0f and 1.0f
    // pushed in that order; 009E4664 `FCHS` negates the result.
    virtual float uniform_float_00bd2f10(float low, float high) = 0;

    // 009E46A9, `ECX = blk`, the raw dword at 009E466F as the stack argument.
    // 009E0270 rewrites blk+168h, blk+3C4h and the twelve sector shapes, and
    // calls 009DE2F0, which is what gives blk+19Ch and blk+1A0h their first
    // values.  docs/SHIP_AI_SECTOR_SCAN.md §7 owns its body.
    virtual void build_sector_shapes_009e0270(ShipAiNavBlockFields& fields,
                                              std::uint32_t raw_argument) = 0;
};

// 009E4330 in native order.  Returns the block; the native routine returns
// `this` in EAX at 009E46B2.
//
// UNCERTAINTY: the null test at 009E4483 guards only the vtable call.
// 009E44A8 and 009E44BA dereference the same pointer unconditionally, so a
// null unit faults in the native routine.  This projection reads the class
// inputs from `unit.ship_class` whatever `unit.present` says and records the
// null case only in `submarine_3f8`.
ShipAiNavBlockFields ship_ai_nav_block_ctor_009e4330(const ShipAiNavBlockUnitInputs& unit,
                                                     ShipAiNavBlockCtorHost& host);

} // namespace bsp

#endif // BSP_SHIP_AI_NAV_BLOCK_CTOR_HPP
