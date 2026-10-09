# CC12 native tick sublist unlink Source candidate

This candidate implements the complete owned `00874E60..00874EB2` leaf through
an explicit MSVC Win32 naked fastcall interface. It is not yet build-reviewed or
admitted as Source. Primary integration owns build registration, the normal
build, whole emitted COFF/Core review, and current application-map status.

- Header: [`native_tick_sublist_unlink.hpp`](../include/bsp/native_tick_sublist_unlink.hpp).
- Implementation: [`native_tick_sublist_unlink.cpp`](../src/native_tick_sublist_unlink.cpp).
- Candidate report: [`cc12_native_tick_sublist_unlink_source.json`](../reports/cc12_native_tick_sublist_unlink_source.json).
- Accepted Native audit: [`CC12_PENDING_REGISTRY_TICK_SUBLIST_UNLINK_LEAF_ABI_READINESS.md`](CC12_PENDING_REGISTRY_TICK_SUBLIST_UNLINK_LEAF_ABI_READINESS.md).

The candidate baseline is `4148cb440ca8347365b320df94032f79ed6eb376`.
The accepted audit accounts for all 83 bytes, 29 instructions, 11 blocks,
15 edges, and 13 syntactic stack paths. Its body SHA-256 is
`a83a36e95a4ff5d860bd6334753ab50c03fc0c7b4c7b5aedaec5f97bd3a04652`.
That historical Native evidence remains in its original PE/Ghidra domain;
this candidate does not reread Native bytes or claim emitted-byte equality.

## Explicit entry and borrowed memory

`unlink_native_tick_sublist_node_00874e60` receives the actual list in ECX,
an explicit unused DWORD in EDX, and the actual node in the first stack word
at entry ESP+4. The second argument fixes Source fastcall placement and makes
no incoming-EDX preservation promise. The implementation captures the node
once in EAX and reaches its common `RET 4` with EAX still that captured node.
The definition has no compiler prologue/epilogue or C++ body outside inline
assembly; compiled confirmation of that property remains a primary gate.

The caller borrows actual raw list backing of at least 0xC bytes and node
backing of at least 0x10 bytes. List head/tail/count offsets are 0/4/8;
node previous/next offsets are 8/C. Those descriptive field names remain
hypotheses. Selected nonzero neighbors require writable previous+C or next+8
fields. Selected list/node accesses require valid readable/writable backing.
There is no copied list, invented node layout, allocator, lock, publication
cell, profile, callback, producer, or ownership transfer.

## Preserved instruction schedule

Every one of the 29 accepted instructions has a corresponding assembly line
marked with its Original address. All seven branches retain their Original
destinations; short branches are requested explicitly. The static checker
normalizes instruction spelling and resolves each Source label against the
accepted address table. This validates the source text, not compiler emission.

The leaf captures the previous field once. Only when the initial previous
and subsequent next guard reads are zero does it compare count as signed
against 1; a greater count returns without any field write or EDI spill.
Otherwise TEST flags survive the conditional PUSH EDI and choose the first
splice. It freshly reads next for that splice, reloads current next after the
first write, then freshly reads previous for the second splice. It clears
node+C before node+8, and only then uses the current count in the actual
`ADD DWORD[list+8],-1` read/modify/write. No count-zero guard, clamp, null check,
membership check, free, profile lookup, callback, or extra cleanup is inserted.

The mutation path pops its conditional EDI spill before the common return;
the skip path never pushes or pops EDI. The accepted arithmetic-flag formulas
remain those of CMP count,1 on the skip path and ADD count,FFFFFFFF on the
mutation path. ADD's carry/auxiliary-carry effects must not be replaced with
SUB or DEC. The complete formulas and register residuals are pinned in the
accepted audit and carried into the candidate report for primary review.

Aliasing can make an earlier splice change a later link read, or either clear
change the count before its read/modify/write. The conditional stack spill
can affect overlapping raw backing; field stores can overwrite saved EDI,
the return address, or the argument slot. Normal EDI restoration and return
require intact spill/control-stack memory. The node argument is not reread.
There is no owned fault recovery, catch, exception frame, or `noexcept`;
earlier writes need not be undone after a later fault. Concurrency, hardware
faults, Original placement/callers, and execution equivalence remain unproved.

## Validation and remaining admission

The static checker verifies the exact 29-operation sequence, branch-label
destinations, naked fastcall declaration/definition, single `RET 4`, current
source hashes, all 20 direct accepted-audit pins, all 18 accepted cleanup pins,
and all 44 constructor Source pins. Every pin records its current or historical
domain. Historical constructor artifact hashes are not current-build evidence.
The candidate report pins these three current Source/document files; its own
JSON is parsed and checked without introducing a recursive self-hash.

Only the four assigned candidate files change. No CMake, ledger, or Ghidra
mutation, caller migration, build, probe, new test, or runtime run is performed.
Source, Native, Original-ABI, integration, and gameplay credit remain false.
The current application map is not inspected or forced to retain this function.

Primary review must register the source and complete the normal MSVC Win32
build, inspect the complete emitted definition and unique Core member against
all 83 accepted Native bytes, account for branches/flags/stack and any padding,
and report the current application-map result. Build or byte equality alone
does not assign production callers or a virtual slot. The actual sublist node
producer, profile, and slot-zero deletion target remain open; the accepted
`00874F00` cleanup does not directly call this leaf, and its node/flag dispatch
inputs do not establish this leaf's list/node binding.
