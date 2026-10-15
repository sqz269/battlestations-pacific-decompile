#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This literal native entry requires MSVC Win32.
#endif

#include <cstdint>

namespace bsp {

// Whole native [008F0420,008F045A): ECX actual writable type-5 record;
// EDX unused; stack borrowed nullable target, opaque scalar, opaque kind.
// Returns the same actual record in EAX and removes 12 stack bytes.
// Record+1C must be null or sole-owned by the current canonical heap.
// Target must remain live, stable, NUL-terminated and disjoint from the
// retired name, record, outputs and active frame; its range/length+1 cannot
// wrap. The two full DWORD values are copied without filtering or defaults.
// Only +1C,+18,+08 are written. Retire records before their owning pools.
// Ordinary successful, nonthrowing, nonaliasing, nonreentrant, single-threaded
// current-provider domain only. No failure rollback, Original private CRT/EH,
// wider parser/class, full Original ABI or game-validation claim.
// This descriptive name is a hypothesis, not a recovered symbol.
void* __fastcall set_native_scene_property_reference_008f0420(
    void* actual_record_ecx, void* unused_edx, const char* borrowed_nullable_target,
    std::uint32_t scalar_bits, std::uint32_t reference_kind_bits);

} // namespace bsp
