#pragma once

#include <cstdint>

namespace bsp {
// Complete 0070D030..0070D058 (41 bytes) and 0070D080..0070D0B5 (54 bytes).
// Native ECX is the actual group; the stack holds the actual entity pointer;
// both return full EAX and RET4. Capture signed group+4F8h once and search the
// entity DWORDs at group+18h, stride34h. Return the FIRST matching identity;
// a null query can match a null slot. Count<=0 performs no record reads.
// Borrow valid aligned 508h-byte Win32 backing: a positive count describes
// readable records within its 24-record capacity. There are no added guards,
// semantic handles, copies, callbacks, allocation or ownership operations.
// These are new C++ interfaces, not native class/register-ABI replacements.
// Native D030 advances ECX through the records; native D080 preserves ECX.
std::int32_t native_unit_group_member_index_0070d030(
    const void* actual_group, const void* actual_entity) noexcept;
void* native_unit_group_member_record_0070d080(
    void* actual_group, const void* actual_entity) noexcept;
} // namespace bsp
