# Actual shared string-vector lookup

Addresses: 005efba0, 005f4375, 005f43b2, 00bb4f54, 00be84c4, 00be90b3, 00bf7fbf

This packet reconstructs **one complete game-owned function,118 bytes**:
`005EFBA0..005EFC15`, proposed descriptive name
`BSP_NativeStringVector_FindCaseInsensitive`. The name is a hypothesis, not a
recovered symbol. It is a shared search of the game's custom string vector;
the surrounding BB4F40 STL-style search and all MPAK container algorithms remain
library contracts. No library algorithm is ported by this packet.

The original ABI is ECX actual12-byte vector, one stacked actual8-byte key
pointer, EAX signed index or -1, RET4. Source declaration:
`find_native_string_vector_005efba0(const NativeStringVectorStorage&, const void*)`
in `include/bsp/native_string_vector_lookup.hpp`. It reuses the existing native
vector and string layout declarations and adds no owner, allocation or pool.
The key is an actual length/pointer header; a null key is not accessed for an
empty or unsigned-rejected range. This source API is not the original ABI.

## Established behavior

The target-verifying live listing has53 instructions and no gaps. Its118 bytes
match the unchanged installed PE recorded in the report. Assembly establishes
the register argument, unsigned address comparisons, wrapping arithmetic and
the result's current-data reload:

1. Read vector count+4, then data+0. Capture the unsigned32-bit end as
   `data + count*8`, with DWORD wrap. Test cursor<end as unsigned before reading
   the key or any element. Capacity+8 is never read; no signed-count guard is
   added.
2. Load current row length, then current key length. Unequal recorded lengths
   skip the row without comparing text. Equal zero lengths match without
   accessing either data pointer. Equal nonzero lengths load key data then row
   data and call retained CRT `_stricmp` (`00BF7FBF`) at5EFBE5. Text comparison
   is NUL-terminated CRT behavior, not a length-bounded or ASCII-only rewrite.
3. On a match, subtract **current vector data+0** from the captured cursor in
   DWORD arithmetic, then arithmetic-shift the signed difference by3. This is
   the explicit reload at5EFC07 after the library call; do not return a cached
   iteration counter instead. Otherwise advance cursor by8 with DWORD wrap,
   retaining the original captured end. Return -1 after exhaustion.

The source uses volatile DWORD reads for these observation points and unsigned
integer addresses for native pointer arithmetic. It preserves the load schedule;
this does not make concurrent mutation safe or validate original CRT callbacks.
There is no allocation, mutation, local exception frame or cleanup in the body.
The existing CRT supplies `_stricmp`; its locale and invalid-input behavior are
not reimplemented here.

## Shared consumers and layout evidence

The existing actual vector declaration is `NativeStringVectorStorage`, an alias
of `NativeMeshWeightNamesStorage`: data/count/capacity at0/4/8 with8-byte
length/data rows. Existing426520 reserve,427110 resize and4CDC20 append establish
this storage; BA's directory copy BB6630 initializes the same three-word member
vector. It is distinct from MPAK's outer16-byte STL vector header.

All five live call sites were inspected, including their argument setup:

| Caller / call site | Setup |
| --- | --- |
| BB4F40 / BB4F54 | ECX=row+8; stack captured temporary-directory key pointer; consumes nonnegative result |
| BE84A0 / BE84C4 | ECX=owner+4; one caller-provided name pointer; converts result to Boolean while holding its captured lock |
| BE9060 / BE90B3 | ECX=owner+4; one caller-provided name pointer; negative result triggers existing append4CDC20 |
| 5F42D0 / 5F4375 | ECX=options owner+17Ch; one selected global entry header pointer |
| 5F42D0 / 5F43B2 | ECX=captured EBX vector; stack selected8-byte element pointer from its current data |

The five sites are owned by **four live functions**. Snapshot lookup also lists
BB4D60, but current live callers/xrefs do not contain it; this historical edge
is not claimed as a verified caller. The caller bodies are not reconstructed
or claimed complete by this packet. Their lock, append and option-control
operations remain outside its coverage.

## Validation and limits

The ordinary strict Win32 build passed. The fresh worktree initially enabled
only `reconstructed_math`; after verifying the eight original seed spans, the
incremental build passed both existing `reconstructed_math` and
`native_math_differential` CTests. All six live direct-call rows pass the audit.
The report records source/header/object/archive hashes and retained logs. The
new object appears once in the archive's member listing; its archived bytes
were not separately extracted and compared. No permanent or new native
comparison fixture was added. Neither existing math test executes this lookup,
and no original/source runtime equality is claimed. The primary must rebuild
and validate its final integrated commit.

The read-only basis is retained under
`local/mpak-binding-frontier-bb-evidence/`, including the original118 bytes,
complete listing, five caller-site excerpts and installed PE hash. Production
implementation is `src/native_string_vector_lookup.cpp`; Ghidra naming and
saved-project synchronization belong to the primary integrator after review.

The function is ready to supply the game predicate to a future compatible
BB4F40 library binding. It does not implement that library search, instantiate
`NativeMpakRuntime`, bind a checked STL vector, mount an installed archive,
prove startup reachability or establish gameplay behavior. The preceding
discovery `NATIVE_MPAK_BINDING_FRONTIER_BB.md` records those remaining contracts.
