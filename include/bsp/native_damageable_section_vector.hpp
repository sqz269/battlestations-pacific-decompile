#pragma once

#include "bsp/singleton_lifetime.hpp"
#include <cstdint>

namespace bsp {

// Borrow actual 10h vector headers (+4 begin,+8 end,+C capacity end), 8-byte
// iterators (owner,position), and 30h records. Preserve header+0. The table is
// the actual stable D0DF04-equivalent identity, not an invented replacement.
struct NativeDamageableSectionVectorAccess {
    const std::uint32_t actual_vtable_00d0df04;
    const SingletonLifetimeCallbacks& invalid_parameters;
};

std::uint32_t size_native_damageable_section_vector_00744070(const void* header);
void* allocate_native_damageable_sections_00876a10(std::uint32_t count);
void* assign_native_damageable_section_00878790(void* destination, const void* source);
void destroy_native_damageable_section_range_00878ac0(void* first, const void* end);
void* copy_backward_native_damageable_sections_00878d60(
    const void* first, const void* end, void* destination_end);
void assign_fill_native_damageable_sections_00879390(
    void* first, const void* end, const void* source);
void* uninitialized_copy_native_damageable_sections_008794f0(
    const void* first, const void* end, void* destination, std::uint32_t actual_vtable);
void* copy_backward_native_damageable_sections_008798c0(
    const void* first, const void* end, void* destination_end);
[[noreturn]] void throw_native_damageable_section_length_0087a580();
void* uninitialized_fill_native_damageable_sections_0087ad20(
    void* first, std::uint32_t count, const void* source, std::uint32_t actual_vtable);
void* uninitialized_copy_native_damageable_sections_0087b610(
    const void* first, const void* end, void* destination, std::uint32_t actual_vtable);

// Complete ordinary and catch bodies. Count zero STILL copy-constructs and
// releases the private source snapshot. Iterator owner is unused by 87B950.
// Current row vslots and header reloads remain observable at callbacks. Native
// x87 transfers, alias order and wrapping32 arithmetic are retained. C++ EH,
// CRT allocation/length-error transport and invalid-parameter callbacks are
// explicit existing boundaries; no original FH3/SEH or binary ABI claim.
void insert_native_damageable_sections_0087b950(void* header,
    const void* iterator_owner, void* position, std::uint32_t count,
    const void* source, std::uint32_t actual_vtable);
void* insert_checked_native_damageable_section_0087c2d0(void* header,
    void* result_iterator, const void* iterator_owner, void* position,
    const void* source, const NativeDamageableSectionVectorAccess&);
void append_native_damageable_section_0087c870(void* header,
    const void* source, const NativeDamageableSectionVectorAccess&);

} // namespace bsp
