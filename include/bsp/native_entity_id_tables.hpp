#pragma once

#include "bsp/object_handle_resolvers.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Opt-in raw native owner storage. Existing conceptual EntityIdTable stays separate.
// Owner +0 profile, +4 full DWORD first ID, +8 signed loop count, +4C slots,
// +50 free count. Free sentinels are +C/+1C; live sentinels are +2C/+3C.
struct alignas(4) NativeEntityIdTableStorage { std::byte bytes[0x54]; };
struct NativeEntityIdSlot {
    NativeEntityIdSlot* next_00;
    NativeEntityIdSlot* previous_04;
    std::uint16_t id_08;
    std::uint16_t untouched_0a;
    void* payload_0c;
};
struct NativeEntityIdTableProfile {
    // Typed Source counterpart of original D19B88 slot 0 -> 00951790.
    // This C++ signature is not an original thiscall function pointer.
    void* (*scalar_delete)(void*, std::uint32_t) noexcept;
};
const NativeEntityIdTableProfile& native_entity_id_table_profile_00d19b88() noexcept;

// Native ECX=owner, RET. Reset does not change +50 free count.
void reset_native_entity_id_table_00951560(void* owner) noexcept;
// Native ECX=owner, stack full DWORD first/count, EAX=owner, RET8.
// count*16 unsigned overflow requests FFFFFFFF bytes from the real allocator.
void* construct_native_entity_id_table_00951660(void* owner,
    std::uint32_t first_id, std::uint32_t count);
// Native ECX=owner, stack low-WORD ID, RET4. Unchecked raw slot arithmetic.
void release_native_entity_id_009516d0(void* owner, std::uint32_t id) noexcept;
// Native ECX=owner, RET. Tests slot i but relinks slot uint16(i)-first_id.
// Both independently selected slots and their links must be valid; no repair.
void sweep_native_entity_id_table_00951720(void* owner) noexcept;
// Native ECX=owner, stack flags, EAX=identity, RET4; always frees slots,
// optionally frees owner when flags bit0 is set. Does not clear dangling fields.
void* delete_native_entity_id_table_00951790(void* owner, std::uint32_t flags) noexcept;
// Native ECX=owner, stack requested ID/payload, RET8. A zero low WORD selects
// free-tail ID; otherwise EAX retains the full requested argument (caller uses AX).
std::uint32_t allocate_native_entity_id_009517c0(void* owner,
    std::uint32_t requested_id, void* payload) noexcept;
// Native CX=ID, RET, EAX=payload; no zero test, signed split, unchecked indexing.
// References identify genuine current owner fields; no replacement registry.
void* resolve_native_entity_id_00521e30(std::uint32_t id,
    const ObjectHandleTables& tables) noexcept;

} // namespace bsp
