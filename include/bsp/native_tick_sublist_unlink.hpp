#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tick sublist unlink requires MSVC Win32.
#endif

namespace bsp {

// Complete owned 00874E60..00874EB2 schedule: 83 bytes / 29 instructions.
// The descriptive list/node name is provisional. Explicit fastcall binds ECX
// to the actual list, EDX to an unused placement word, and entry ESP+4 to the
// actual node. Capture that node once in EAX; return it through common RET 4.
// The explicit EDX argument does not request preservation of incoming EDX.
//
// Borrow actual raw list backing of at least 0Ch bytes and node backing of at
// least 10h bytes. List DWORDs +0/+4/+8 and node DWORDs +8/+C must permit the
// selected reads/writes; selected nonzero neighbors need their +C/+8 writable.
// Preserve the captured previous word, each fresh next/previous read after
// earlier writes, then clear node+C before node+8 and ADD current list+8,-1.
// The zero-link/signed-count>1 path skips all writes and the EDI spill.
// There is no count-zero guard, null/membership check, clamp, lock, free,
// profile lookup, callback, producer binding, extra EH frame, or cleanup.
//
// Mutation conditionally spills EDI at entry ESP-4; normal EDI restoration
// requires field writes not to overwrite that spill. Stack overlap can change
// later field reads, saved EDI, or return-address/argument memory. Callers must
// supply valid selected backing and intact control-stack memory for a normal
// return. Earlier writes need not be undone when a later access faults.
// Source compilation, Original placement/callers, virtual-slot identity,
// fault/concurrency equivalence, drop-in ABI and gameplay remain separate gates.
void* __fastcall unlink_native_tick_sublist_node_00874e60(
    void* actual_list,
    std::uint32_t unused_edx,
    void* actual_node_first_stack_word);

} // namespace bsp
