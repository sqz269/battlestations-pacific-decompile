# Input settings vector storage

Addresses: 006A0DB0, 00492210, 0049DF50, 0049E050, 006A6350, 006A79A0,
006A4710, 0069EEA0, 006A6EE0; supporting copy, relocation and insertion
helpers are listed in `reports/native_input_settings_vector_storage.json`.

The settings parser now uses seven concrete resize operations over its actual
native allocations. Settings destruction and constructor unwind use two concrete
nested-range destructors. This removes nine required callbacks. Six table-tree
calls and four lifetime tree-erasure calls still require providers, along with
the separately declared keyboard library services.

## Recovered storage contracts

Ordinary checked vectors have four DWORDs: opaque, begin, end, capacity end.
The source preserves opaque words and uses the native element widths: DWORD 4,
pooled string 8, nested vector 16, descriptor 20. Descriptor copies retain all
five DWORDs, including the high three bytes surrounding the flag. Growing a
vector first makes an independent fill-value copy, even with spare capacity.
Insufficient capacity grows to the greater of the requested count and 1.5 times
the old capacity, subject to the native element limit.

Nested fill values are deeply copied with exact-size backing, using the behavior
of 004F6210, 00557BC0 and 0055B150. Existing nested elements relocated during
outer growth instead transfer their buffers. The 006A2310, 006A3200 and 006A6D20
helpers construct empty destination headers and swap begin, end and capacity
in that order. Their source headers become empty; buffer identities and spare
capacities survive. Scalar, descriptor and string relocation uses copies.
Completed old elements are destroyed forward before freeing the old backing.
DWORD vectors publish begin, capacity and end; other instantiations publish
capacity, end and begin. Shrink destroys the removed suffix and retains capacity.

The packed-bit vector at 0049DF50 has a different 20-byte layout: bit count at
0, opaque at 4, and underlying DWORD-vector pointers at 8/12/16. Growth resizes
the word storage, publishes the bit count, then fills the new bits. Only the low
byte of the supplied value determines true or false. Shrink truncates used word
storage and masks unused high bits in the final word. This is not a byte array.

The by-value string/nested-vector arguments own their allocations and are
consumed on return, including equal-size and shrink calls. Callers must supply
independent values as native copy construction would. Range destructors capture
the endpoints, visit elements forward, release each current nested backing, and
clear pointer words 4/8/12 while preserving opaque word 0. The outer backing is
released separately by its owner.

## ABI and boundaries

All seven resize entries receive the header in ECX. Stack arguments are count
and the fill value: descriptor five DWORDs (RET 18h), DWORD/bit one DWORD (RET 8),
string two DWORDs (RET 0Ch), or nested header four DWORDs (RET 14h). The two range
destructors receive first in ECX and last in EDX, with two unused stack words
(RET 8). These are new explicit-service C++ APIs, not original register or STL
ABI replacements. Descriptive function names remain hypotheses.

Valid, consistently owned storage is required. Allocation callbacks may throw
but must not structurally mutate headers or source values. Source cleanup frees
completed newly constructed elements on supported C++ unwind; nested transfers
already performed remain visible on failure. Original FH3 execution, CRT error
transport, malformed ranges, private-stack aliases and hardware-fault behavior
are unvalidated. General count insertion away from the end is not implemented.

## Validation

The retained isolated Win32 fixture compares actual copied original bodies with
the source parser and lifetime, using the existing native tree providers. It
matches 122,545 settings-lifetime DWORDs and the prior 335 string-storage DWORDs.
One focused scenario adds 1,448 matching DWORDs for all seven resize operations,
nonempty consumed values, growth/shrink/equal counts, packed bits across DWORD
boundaries, and two populated range destructors. Pooled allocation and release
counts match and no pooled strings remain. Source constructor-failure cleanup
and the actual raw manager drain also pass. These fixtures do not exercise game
devices or interact with the running game.

The initial comparison caught a source error: deep-copying existing nested
elements on outer growth reduced their capacity. Assembly and helper pseudocode
confirmed the pointer swaps; the corrected source passes without weakening the
comparison. Strict MSVC Win32 compilation and both existing CTests pass. Native
byte hashes, call ownership checks, saved Ghidra annotations, exact source and
artifact hashes are recorded in the report. Remaining tree providers,
application integration and gameplay validation are still pending.

All 22 checked native byte envelopes match the live program and installed file.
The live audit checks 180 direct CALLs at their actual saved function owners.
Four decoded exception rethrows (006A0106, 006A5DE2, 006A7822, 006A3EBB) remain
outside saved catch ownership and are explicitly excluded from passed checks.
Returning-free flow gaps were repaired under the write lock without changing
callee no-return flags. Prior names, signatures and comments are retained in the
annotation receipt; original engine signatures are unchanged.


## Part-vector provider, batch AB

Correction from `docs/NATIVE_UNIT_PART_VECTOR.md`: The shared DWORD storage mechanics now accept an explicit publication order. The existing00492210 entry keeps begin/capacity/end; the unit-part pointer specialization0087B460 uses capacity/end/begin. Eight original-pointer/source cases also check the existing DWORD provider. The valid-storage and source exception boundaries above remain unchanged.
