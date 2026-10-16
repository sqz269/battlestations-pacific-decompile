# Native DWORD-vector object fill

This packet supplies complete Source for `00698010..00698035`: 38 native
bytes and 18 current instruction starts. It preserves the established
`STL_FillObjects_00698010` library identity as a descriptive hypothesis, not a
recovered symbol. The Source directly uses the accepted complete assignment
entry; it does not use a callback, generic container, typed-array/count bridge
or a snapshot of the source header.

The new declaration is in `native_dword_vector_fill_objects.hpp`, with its
implementation in `native_dword_vector_fill_objects.cpp`. This packet does
not promote existing bounded storage fragments, the enclosing `00698400`
insertion body, or the historical `006987C0` region into complete Source.
Existing Source, fixtures and reconstruction records remain unchanged.

## Native contract

The original entry is cdecl with three physical stacked words:
`(first,last,source)`. It ends with plain RET. First and last are captured
before the equality check, even for empty input. Only nonempty input loads
the source-header pointer. That pointer is captured once and retained across
the loop; the pointed-to header contents are not copied or cached by this
wrapper.

Each iteration invokes actual `00697BD0` assignment with current destination
in ECX and the captured source pointer as its one stacked input. The child's
incoming EDX is unconsumed. The new Source passes zero for that dead slot and
does not claim original ambient EDX residue. Only after normal assignment
return does it advance the destination's DWORD address by `10h`. It repeats
until that wrapped address equals the captured last. No endpoint ordering,
alignment, null or count guard is added; typed pointer arithmetic is avoided.

The `10h` stride advances actual raw checked DWORD-vector headers, whose
layout and ownership are consumed by the accepted assignment implementation.
The wrapper does not invent or overlay a C++ header/iterator/allocator object.
Each assignment rereads CURRENT source-header contents. When the source
aliases a destination header, an earlier iteration can affect a later one.
The Source does not replace that schedule with a deep source copy.

The new interface returns void. Empty native input leaves incoming EAX
unconsumed; nonempty traversal leaves the last assignment's incidental EAX.
Both actual producer sites discard this value. Thus no coherent public EAX
result or recovered original C++ return type is claimed.

## Actual dependency, ownership and exceptions

The only direct child is
`assign_native_checked_dword_vector_00697bd0`, accepted at
`8be630f4ccc1d6ac4af2256100de1b994d2a8014`. It supplies the actual raw
shortening/capacity/growth paths, progressive copies, current header reloads,
free before Buy, and accepted current allocation/free/diagnostic/exception
services. Its Source and header fingerprints are frozen in the report.

This wrapper has no allocation, owned temporary, local FS/FH3 frame, catch,
cleanup or rollback. A child exception propagates through Current C++ without
an added noexcept barrier; previously completed assignments remain completed.
The `10h` advance happens after a normal child return. Parent insertion EH
states and private source temporaries visible at the producer sites belong
to that separate parent and are not reconstructed here.

## Fresh body and producer proof

| Verified transfer | Scope |
| --- | --- |
| `00698026 CALL 00697BD0` | Owned actual assignment |
| `006985ED CALL 00698010` | Bounded first insertion producer |
| `00698649 CALL 00698010` | Bounded second insertion producer |

Fresh complete typed metadata, original PE bytes, live bytes and every
selected instruction start agree. Each transfer has its current owning body,
default/effective target and REL32 bytes verified. The owned body is one
complete 38-byte range with 18 starts and one direct transfer, without local
EH or an external tail JMP.

The first producer span is `006985DE..006985F6`: 25 bytes/nine starts. It
forms the source address at parent EBP-28h, pushes source/last/first, writes
a parent EH state, invokes the wrapper, adjusts ESP by `0Ch`, then jumps to
the shared continuation. The second span is `0069863E..00698653`: 22 bytes/
nine starts. It forms the same private source address, computes last from
a stacked DWORD plus EBX, pushes source/last/first, invokes the wrapper,
adjusts ESP by `18h`, then overwrites EAX at the shared `00698651` continuation.
That second adjustment includes 12 bytes outside the immediate three-push
setup; it does not establish a six-input wrapper ABI. The first producer also
reaches this EAX overwrite before using the result.

These are bounded producer spans, not a whole parent reconstruction. Current
typed ownership places both sites in `00698400`; the older snapshot caller
list's catch labels are not used as ownership evidence. No body/flow repair
or Ghidra mutation was needed. The union is 85 bytes, 36 current starts and
three direct transfers; owned coverage remains 38 bytes/18 starts/one transfer.

## Verification boundary

The MSVC Win32 build and all three existing suites passed in 8.42 seconds.
All three JSON transfer rows and three documentation claims pass, without
failures or unchecked claims. No new tests or original native execution were
added. The existing suites cover established math, seeds and tools; they do
not establish this wrapper's game behavior.

Six focused Release COFF checks passed over the new Source's 41 bytes/22
instructions. Both endpoint loads precede the empty branch; source-pointer
capture remains after it. The only relocation is the actual assignment call,
with current destination, captured source and explicit unconsumed EDX zero.
The DWORD `10h` advance/equality loop and plain RET are verified. These Source
counts are separate from the original 38 bytes/18 starts. Frozen Source,
header, accepted child and object hashes accompany the primary's eight-check
precommit review in the report. Final frozen-blob acceptance remains separate.

The Source requires accessible reached raw headers and finite equality
termination, with DWORD wrapping retained. Original arbitrary aliases into
argument/private-frame storage, dead registers/flags, static CRT/heap state,
fault/SEH/FH3/RTTI/exception identity, binary ABI and gameplay remain unproved.
The accepted Current C++/CRT providers define the Source exception boundary.
