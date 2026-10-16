#pragma once

#include <cstdint>

namespace bsp {

// Bound addresses of the five actual original DWORD cells. Source binding
// metadata stays stable and disjoint from all reached raw accesses; payloads
// may alias the descriptor, selected table and other actual backing when each
// reached access is valid. These are live cells, not snapshots of table state.
struct NativeCommandTargetHandleTables {
    const void* actual_split_00f89a10;
    const void* actual_low_base_00f89a0c;
    const void* actual_low_table_00f89a54;
    const void* actual_high_base_00f89a60;
    const void* actual_high_table_00f89aa8;
};

// Whole78B/23op00521EA0. Native ECX=actual descriptor, unused EDX,
// plain RET, EAX raw DWORD. A zero kind BYTE returns0 before cache/id/global
// reads. A nonzero cache is read again for return. Otherwise capture the WORD
// id, signed-compare against current split, subtract current selected base,
// load current table, scale16 modulo32 and load slot+C. Cache the raw result.
// The low branch returns its captured slot word; the high branch rereads the
// cache after its store. No type/null/index/table-capacity guard is introduced.
// Source arithmetic retains x86 DWORD wrap, without signed-overflow UB.
// No private register/fault/binary-entry or game validation is inferred.
std::uint32_t resolve_native_command_target_00521ea0(void* actual_descriptor,
    const NativeCommandTargetHandleTables& tables) noexcept;
} // namespace bsp
