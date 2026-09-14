#pragma once
#include "bsp/native_legacy_sbo_string.hpp"

namespace bsp {
// 711C30: ECX named record, stack destination, RET4. The record embeds its
// existing 1Ch legacy SBO at+8; output+0 is untouched. Source objects stay owned
// by the caller. This is a new C++ ABI, not a native virtual entry.
NativeLegacySboStringStorage& copy_native_part_record_name_00711c30(
    const void* actual_record, NativeLegacySboStringStorage& destination);
// 4CDBE0: ECX source SBO; stack destination,offset,count; RET0C. Initialize
// the existing destination then use the canonical complete substring assign.
NativeLegacySboStringStorage& construct_native_part_substring_004cdbe0(
    const NativeLegacySboStringStorage&, NativeLegacySboStringStorage& destination,
    std::uint32_t offset, std::uint32_t count);

// 713270 with its actual 713380 caller's empty 10h by-value row. This supported
// invocation resizes a checked vector of checked DWORD vectors: header+0 is
// untouched, begin4/end8/capacityC, row stride10h. Growth transfers each old
// row's allocation by swapping its three pointer cells into a fresh empty row;
// it does not copy or recreate node owners. Shrink destroys the removed rows.
// Nonempty fill values and arbitrary insertion/erase positions in the generic
// 712FF0/712B80 templates are not exposed by this source interface.
// Actual well-formed, consistently owned storage is required. Allocation/free
// callbacks must not structurally mutate these containers. The source CRT owns
// all allocations and invalid-handler/exception state, not the original CRT.
void resize_native_part_group_rows_empty_00713270(void* actual_header,
    std::uint32_t requested_count);

// Complete 713380..7135BD: ECX existing part, no stack args, plain RET.
// Scan selected-set+7C's 8-byte records, copy record0's name, match six bytes
// "damage", parse suffix with the actual source CRT atol, resize to that
// unsigned number when needed and append record+4 to row(number-1).
// Reload current source/set bounds after calls; preserve case, duplicate
// appends, record order, zero/negative-number paths and owning string cleanup.
// Existing pointer-vector operations are reused after full-body/callee-byte
// equivalence checks. No name filter fallback, synthesized rows, node retain,
// default name or replacement hierarchy is supplied. Gameplay, native FH3 and
// fault-address delivery remain unproved.
void build_native_unit_part_groups_00713380(void* actual_part);
} // namespace bsp
