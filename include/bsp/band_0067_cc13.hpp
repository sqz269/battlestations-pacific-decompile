#pragma once

// Leaf helpers from the 00670000-006CFFFF and 00700000-0073FFFF bands (cc13-sweep67,
// docs/CC13_BAND_0067_0073.md): HUD and front-end helpers, fish-school/flag cloth math, the
// fish-school and flag slot pools, plane helpers and the geom-mesh radix sort. New C++
// interfaces, not the native register ABI. Names are hypotheses, not recovered symbols; each
// declaration states the original address and ABI.
//
// x87 model (as in bsp/band_0060_cc13.hpp): the image runs with 53-bit precision, so an FSTP to
// a float slot is a float cast and unspilled intermediates are doubles.

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp::band_0067_cc13 {

struct Vec3 {
    float x{};
    float y{};
    float z{};
};

// 00709A60, __fastcall(ECX out, EDX a, b), RET 4: out = a + b, each component one FADD stored
// to a stack temporary first, so out may alias a or b.
Vec3 vec3_add_00709a60(const Vec3& a, const Vec3& b) noexcept;

// 00709AC0, same ABI as 00709A60: out = a - b.
Vec3 vec3_sub_00709ac0(const Vec3& a, const Vec3& b) noexcept;

// 00709B60, __fastcall(ECX v, EDX d, float s), RET 4: the three products d*s are stored to
// float temporaries first, then v += product, component by component.
void vec3_add_scaled_in_place_00709b60(Vec3& v, const Vec3& d, float s) noexcept;

// 00709BB0, __fastcall(ECX out, EDX a, b, float s), RET 8: out = a + float(b*s), through a
// temporary (alias-safe).
Vec3 vec3_add_scaled_00709bb0(const Vec3& a, const Vec3& b, float s) noexcept;

struct Plane {
    Vec3 normal;  // +0
    float d{};    // +0Ch
};

// 0073E780 BSP_Plane_FromNormalAndPoint, __thiscall(ECX plane, point, normal), RET 8: copies
// the normal, then d = float((point.y*n.y + point.x*n.x) + point.z*n.z).
void plane_from_normal_and_point_0073e780(Plane& plane, const Vec3& point, const Vec3& normal) noexcept;

// 0073E7C0 BSP_Plane_SignedDistance, __thiscall(ECX plane, point), RET 4: the dot product is
// stored to a float (FSTP [esp+4]) before d is subtracted; the result is stored as a float.
float plane_signed_distance_0073e7c0(const Plane& plane, const Vec3& point) noexcept;

// 0067DF40 BSP_DigitSpinner_StepDigit, __thiscall(ECX spinner, delta), RET 4. The spinner keeps
// the selected digit index at +ECh and the digits at +F0h. sum = digit + delta + 10; the digit
// becomes sum % 10 (IDIV) and the quotient sum / 10 is returned, so 0 means a borrow, 1 no
// carry and 2 a carry (for |delta| < 10).
int digit_spinner_step_0067df40(std::int32_t& digit, int delta) noexcept;

// 007018A0 BSP_Warning_RemapKind, __fastcall(ECX kind): 0 -> 1, 1 -> 2, anything else -> 3.
std::uint8_t warning_remap_kind_007018a0(int kind) noexcept;

// 0067C170 BSP_GuiListbox_IsRowActivatable, __thiscall(ECX list, row), RET 4: the byte arrays
// at +0Ch (enabled) and +10Ch (locked) are indexed by row without a bound check.
bool listbox_row_activatable_0067c170(const std::uint8_t* enabled_bytes, const std::uint8_t* locked_bytes,
                                      int row) noexcept;

// 00722210 / 00722400 / 007225F0 BSP_GeomMesh_RadixSortKey0/4/8, __fastcall(ECX records,
// EDX count, scratch). A stable LSD radix sort of 10h-byte records on the unsigned dword at
// +0/+4/+8 (key_index 0/1/2): one pass builds four interleaved 256-entry histograms on the
// stack, exclusive prefix sums follow, then four byte passes scatter records -> scratch ->
// records -> scratch -> records. A non-positive count returns without touching either array.
struct Record16 {
    std::array<std::uint32_t, 4> words{};
};
void radix_sort_records16_00722210(Record16* records, int count, Record16* scratch, int key_index) noexcept;

// The fixed-size slot pools of the fish-school model (pool vtable 00CFD410, two 18Ch-byte
// slots per 800-byte block, slot +188h holds the block index) and the flag model (vtable
// 00CFD480, 32 slots of 1A0h bytes per 3444h-byte block, slot +19Ch holds the block index).
// Pool fields: +0Ch critical section, +24h lock depth, +28h block array, +2Ch block count,
// +30h block capacity (grown to 2n+2), +34h lowest block with a free slot or -1. A block keeps
// a stack of free slot indices (+318h/+3400h, uint16) and its free count (+31Ch/+3440h).
// The critical section and the lock depth counter stay with the host; this models the slot
// bookkeeping only, and the block vector does not reproduce the native 2n+2 capacity policy.
class SlotPool {
public:
    SlotPool(std::size_t slot_size, std::uint16_t slots_per_block) noexcept;

    // 00708920 BSP_FishSchoolPool_Allocate / 0070B070 BSP_FlagPool_Allocate.
    struct Slot {
        std::size_t block{};
        std::uint16_t index{};
    };
    Slot allocate();

    // 007079A0 BSP_FishSchoolPool_Free / 0070ACB0 BSP_FlagPool_Free: pushes the slot index back
    // and lowers the first-free block (an unsigned compare, so -1 is always replaced).
    void free(const Slot& slot) noexcept;

    [[nodiscard]] std::uint32_t first_free_block() const noexcept { return first_free_; }
    [[nodiscard]] std::size_t block_count() const noexcept { return blocks_.size(); }
    [[nodiscard]] std::uint16_t free_count(std::size_t block) const noexcept;

private:
    struct Block {
        std::vector<std::uint16_t> free_stack;  // +318h / +3400h
        std::uint16_t free_count{};             // +31Ch / +3440h
    };
    // 007075D0 BSP_FishSchoolPoolBlock_Init / 00709810 BSP_FlagPoolBlock_Init: free stack
    // {N-1, ..., 0} so the first allocation takes slot 0, count N.
    [[nodiscard]] Block make_block() const;

    std::size_t slot_size_{};
    std::uint16_t slots_per_block_{};
    std::vector<Block> blocks_;
    std::uint32_t first_free_{0xFFFFFFFFu};
};

inline constexpr std::size_t kFishSchoolSlotSize = 0x18C;
inline constexpr std::uint16_t kFishSchoolSlotsPerBlock = 2;
inline constexpr std::size_t kFlagSlotSize = 0x1A0;
inline constexpr std::uint16_t kFlagSlotsPerBlock = 32;

}  // namespace bsp::band_0067_cc13
