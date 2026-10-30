# Locale slot allocation and base scalar integration

This batch registers the direct A9F0B0 allocator and CFEA5C slot-zero scalar
737010. It also refines only the existing E970 block initializer's stores.
No application caller, loaded locale or startup permission is added.

Allocator2c5a borrows the same canonical pool before native effects and calls
real Win32, matched heap and host-free E970 services without broad-host stubs.
It preserves first-free/capacity publication before allocation, captures actual
allocator results immediately, retains current table/count versus captured
destination and scan cursors, and performs the real WORD free-stack RMW.
Only normal Leave followed by healthy private frame state issues a normal result.

The nonmoving caller frame records actual slab/table/node/section identities.
Every escape poisons metadata and rethrows without unlock, free, restoration,
rollback or retry. An unpublished slab/table can survive only in that retained
frame. Pool startup health is not allocation quiescence or safe-exit permission;
a future caller must retain and settle the whole failed graph separately.
Returned slot storage does not construct its payload, strings, links or map.

Fresh slab allocation exposed E970's typed field-store assumption. Root's narrow
fix writes count WORD first, then each reverse-stack WORD before the node slab
DWORD, using fenced byte copies. It touches no payload/link/padding and leaves
the shared access template/API/other functions unchanged. Exact whole CPP/HPP
inverse projections and strict Win32 object compilations passed.

The 30-byte base scalar calls real7365E0 before reading the actual caller-owned
DWORD's low byte, conditionally frees the saved receiver and returns address
bits without a post-free read. It supplies neither full derived cleanup nor
canonical profile wiring; the independent Source/ASM review is separate from
the common build.

The combined current-main MSVC Win32 Release build passed both existing checks
in12.98s. No tests or probes were added; no new body/game was run for qualification.
Current heap throw/OOM/import, raw backing, external serialization, retained
failure and ordinary C++/Native FH3/SEH/ABI limits remain explicit. Ghidra
comments were preserved, saved and exported with the updated Source evidence.

All nine raw Text choices remain FALSE. Canonical F8BC4C owner/profile wiring,
payload/map construction, raw loading and actual startup remain open. Frozen
ac60 retains its Source hold. No runtime, Original ABI or new-native-body credit.

Exact Source/build/peer evidence: reports/codex_locale_allocation_integration.json.
