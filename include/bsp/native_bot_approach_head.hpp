#pragma once

#include "bsp/bot_tasks.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Caller-owned live prefix at an approach object's offset0 (normally task+3F8).
// No initialization, executable profile, allocation or enclosing lifetime here.
struct NativeBotApproachHeadStorage {
    std::uint32_t profile_00;
    const void* unit_04;
    const void* class_08;
    const void* pilot_0c;
    const void* descriptor_10;
    const void* descriptor_row_14;
    std::uint32_t field_18;
    std::uint32_t field_1c;
    std::uint32_t field_20;
    float speed_ratio_24;
    float field_28;
};
static_assert(sizeof(NativeBotApproachHeadStorage) == 0x2c);
static_assert(offsetof(NativeBotApproachHeadStorage, speed_ratio_24) == 0x24);

// SAME actual publication/constant cells; forming aliases has no side effects.
// Cells, reached unit/class/descriptor storage and writable head stay live.
struct NativeBotApproachHeadContext {
    const void* const volatile& descriptor_rows_00f8a30c;
    const volatile float& fallback_one_00d7a24c;
    const volatile float& minus_one_00d7a260;
};

// Complete ordinary009F9CE0 body. D21C74 is a recovered numeric stamp only.
// Preserve the fresh unit+538/DF4 reads, wrapping row arithmetic, native x87
// spill/FCOMIP/JBE and current constant loads. New C++ ABI, not RET8/ECX ABI.
NativeBotApproachHeadStorage* construct_native_bot_approach_head_009f9ce0(
    NativeBotApproachHeadStorage*, const void* unit, float reference_speed,
    const NativeBotApproachHeadContext&) noexcept;

// Adopt the existing host's natural complete constructor call. All other
// original host services remain required; no constructor phases or defaults.
class NativeBotApproachHeadHost : public BotTaskHost {
public:
    explicit NativeBotApproachHeadHost(const NativeBotApproachHeadContext&) noexcept;
    void construct_speed_reference(void*, const void*, float) final;
private:
    const NativeBotApproachHeadContext& head_context_;
};

} // namespace bsp
