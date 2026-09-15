#pragma once
#include <cstdint>

namespace bsp {
// Complete008798F0..0087998C[157]. Original ECX first, EDX unsigned count,
// stack source plus three ignored DWORDs, RET10h. Constructs consecutive30h
// records with the real00878B40 source; borrows the actual D0DF04-equivalent
// table identity. Zero count reads no record; a null current address skips
// construction but still advances by30h with native32-bit arithmetic.
//
// On a C++ exception, destroy the completed prefix in forward order through
// each record's CURRENT actual vslot0(flags0), then rethrow. The incomplete
// record's placement cleanup00401130 is a verified no-op. A cleanup exception
// escapes this catch; there is no added terminate, free, or container rollback.
// Original FH3/SEH, original ABI/private spills and faults are not reproduced.
void fill_native_damageable_sections_008798f0(void* first,
    std::uint32_t count, const void* source,
    std::uint32_t actual_vtable_00d0df04);
} // namespace bsp
