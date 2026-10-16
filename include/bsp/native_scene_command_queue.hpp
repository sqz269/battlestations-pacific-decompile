#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Explicit cells at native entry ESP=P: incoming owner/command/target at P+4,
// saved allocation at P-10h, outgoing constructor owner/command/target at
// P-20h, its saved-record cell at P-34h, and outgoing list argument at P-18h.
// The list argument intentionally reuses the constructor target word. Source
// metadata is stable and disjoint from all reached raw accesses. These cell
// bindings preserve actual payload aliases, without supplying other compiler,
// allocator, helper, register-save, return-address or private EH stack cells.
struct NativeSceneCommandQueueFrame {
    const void* actual_parameters12;
    void* actual_saved_allocation4;
    void* actual_constructor_parameters12;
    void* actual_constructor_saved_record4;
    void* actual_list_parameter4;
};

// Whole 11B/5op action: CURRENT saved allocation -> genuine current CRT free.
// Ordinary explicit-cell mapping, not the original private EBP-frame entry.
void unwind_native_scene_command_queue_00c61b10(
    const void* actual_saved_allocation4);

// Whole 115B/34op 00469610, native ECX database, three stack words, RET0Ch.
// Save database in the native cell, then allocate14h before reading arguments;
// overwrite that cell with allocation and arm its cleanup before
// construction. Capture target/command, publish outgoing target, then capture
// owner and publish outgoing command/owner. Call the real raw constructor,
// publish its returned value as the list argument and disarm BEFORE insertion
// into current database+14Ch. No rollback if insertion throws. Existing
// current CRT allocation/free and ordinary C++ cleanup transport are used;
// Original heap/handler/FH3/SEH/private ABI/game equivalence remains unproved.
// The semantic result is void; no incidental native EAX result is promised.
void queue_native_scene_command_00469610(void* actual_database,
    const NativeSceneCommandQueueFrame& frame,
    NativeStringRawPoolContext& strings);
} // namespace bsp
