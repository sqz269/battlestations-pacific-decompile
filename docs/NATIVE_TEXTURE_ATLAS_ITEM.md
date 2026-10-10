# Actual atlas item: physical evidence, Source admission held

This packet preserves fresh installed-PE and live Ghidra evidence for the actual
30h item constructor `00AEE4D0`, cleanup `00AEE220` and scalar deleting wrapper
`00AEEAD0`. Source is **not admitted or implemented**: exact Ghidra body sets and
current flow properties remain unavailable, and the wrapper has a typed listing
gap. The report is `reports/native_texture_atlas_item_source.json`; its filename
does not imply a Source implementation.

Baseline is `f1155d165207566b3cf977304104902f007960a3`. The installed executable is
12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every live query uses the repository client verification for project `bsp`,
program `/battlestationspacific.exe`, x86 language and image base `00400000`.
Configuration names `C:/Users/sqz269/bsp.gpr`; the typed identity response does
not independently attest that absolute GPR filesystem path.

## Byte and listing evidence

| Selected physical span, end exclusive | Bytes | PE decode starts | Live listing / metadata starts | Typed exact-start result |
| --- | ---: | ---: | ---: | --- |
| `AEE4D0..AEE619` | 329 | 122 | 122 / 122 | All 122 exist |
| `AEE220..AEE296` | 118 | 38 | 38 / 38 | All 38 exist |
| `AEEAD0..AEEAEE` | 30 | 11 | 10 / 10 | `AEEAE5` has no instruction |

All bytes in these spans match the installed PE and live memory. The existing
constructor saved export has the same 122 starts as the live listing. No saved
cleanup or wrapper export existed in the inspected shared export directory;
this packet retains fresh live transcripts and does not claim independent old
preimages for those functions. Exact-start queries cover every physical start,
not just the wrapper gap. Matching counts and endpoints are not exact function
AddressSets and do not establish complete Ghidra body membership.

The wrapper physically calls `AEE220`, reads stack flags after cleanup, and
conditionally calls `BF65AC` with saved owner `ESI`. Its missing instruction is
`83 C4 04` (`ADD ESP,4`) at `AEEAE5`. Both returning branches reach
`MOV EAX,ESI; POP ESI; RET4`, so the physical return value is saved owner identity,
including when that backing was freed. Decompiler `extraout_EAX` cannot prove
this result. The exact typed listing jumps from call `AEEAE0` to `AEEAE8`.

The read-only flow probe returned HTTP 200 with `Script execution disabled`
and no property rows. Its full response and verified identities are retained.
Current call-site flow override, separate fallthrough override, default and
effective flow, callee NoReturn/thunk flags and exact function AddressSets were
not obtained. The gap alone establishes none of their values. No mutation,
repair, annotation, export refresh or save was attempted. The repository repair
tool refuses `--apply`; a mutating flow-clear dry-run is not an inspection route.

## Physical storage and failure contracts

The constructor takes actual item storage in ECX, then stack item-name header,
borrowed logical texture and descriptor-name header; it returns the item in EAX
with `RET0Ch`. Descriptor length/data at `+0/+4` are zeroed first, then item-name
length/data at `+C/+10`, before either source header is copied. Identity checks
occur after those stores. Each non-self copy resizes from the current source
length, rechecks source length and then copies current destination length using
current source/destination pointers. A source aliasing either item header must
observe the preceding zeroing and callbacks; semantic string snapshots change
this contract.

A local 8h dot needle is zeroed and resized to one character. Its resulting
pointer is captured separately, while the later copy and return sizes use the
current local length. Physical data `CE3A70` is `2E 00`. Reverse-find `467CF0`
uses limit `7FFFFFFF` and excludes position zero. The captured temporary pointer
is returned through the current string pool after search. It is not a separately
registered unwind object. A found suffix causes name resize; the original
conditional space-fill of any current-length growth must remain meaningful if
provider callbacks change the header. Texture `+8` is published only after those
operations return. No item stores touch `+14..2F`; UV/packed preimages belong to
the caller. No logical-texture reference count or COM lifetime action appears.

Cleanup has ECX item and plain `RET`, with no semantic result. It captures the
name pointer and returns a nonnull name with length `+C` plus one. Only afterward
does it read the **current** descriptor pointer and length `+0` plus one. It
preserves stale headers and any changes made by providers. It never clears the
item, releases the logical texture, or touches UV/packed data. Storage disposal
is the separate conditional step in `AEEAD0`, after member cleanup returns.

## Unwind evidence

Both complete 36-byte FuncInfo records, both maps, both 10-byte handlers and all
three action spans match PE/live bytes. Every physical handler/action instruction
also exists at its exact typed listing address. The handlers are listed code,
but `disassemble_function` found no enclosing function at their entry points;
no handler function body membership is inferred.

| Owner | Handler / FuncInfo / map | State transition and action |
| --- | --- | --- |
| Constructor | `CBA673 / DF2334 / DF2324` | State 1: `CBA668`, saved item `+C` to `41DD20`, then state 0 |
| Constructor | same | State 0: `CBA660`, saved item to `41DD20`, then -1 |
| Cleanup | `CBA618 / DF22CC / DF22C4` | State 0: `CBA610`, saved item to `41DD20`, then -1 |

The constructor records state 0 after descriptor zeroing and state 1 after name
zeroing, before any allocating copy. State 1 remains through dot creation,
search, temporary return and suffix resize. Thus an ordinary call failure there
selects name then current descriptor cleanup; the dot temporary has no extra
unwind entry. This is physical failure-schedule evidence, not executed Native
unwinding or a new rollback policy.

Cleanup sets state 0 before name return and state -1 before descriptor return.
A name-return/getter exception selects current descriptor destruction; failure
during descriptor return has no further local cleanup. Both FuncInfo records
have magic `19930522`, no try blocks/IP map/ES list, and EHFlags 1. Runtime fault
translation and failure inside an unwind action remain outside this packet.

## Existing Source providers and the next gate

Current `native_string.cpp/.hpp` provides the raw-pool overloads of
`resize_native_string_header_0041dd40` and
`destroy_native_string_header_0041dd20`; `native_physical_file_date.cpp` provides
`reverse_find_native_string_header_00467cf0`. The report pins these current files
to baseline Git blobs. The future item provider must borrow the application's
actual `01090AA8`, `01090AA4` and `01090AA0` cells through
`NativeStringRawPoolContext`. Its current getter is resolved on every allocation
and return, including large blocks and disabled small returns. Getter exceptions
can escape; the semantic noexcept storage adapter would change that boundary.
Existing provider Source/lifetime and overlap qualifications remain in force.

Root must first obtain actual body AddressSets and exact current flow preimages
through an attested read-only route. Any subsequently justified GPR repair must
use Root's write lock, retained metadata/comment preimages and a bounded
transaction. Post-state must establish the wrapper's 30 bytes and all 11 starts,
unchanged bytes, exact body membership and refreshed exports. A separately
justified fragment admission would need explicit review; this packet does not
silently substitute one for the held whole-body gate.

Only then can the constructor/cleanup/deletion Source packet proceed, with Root
handling CMake, ledger and annotations. No new C++ or tests were added, so no
build was run. Compile, COFF/EH, fixture, original ABI, runtime and gameplay
credit are all zero. Raw `AEE620`, `AEEAF0`, `AEF280`, `735F90`, manager ownership
and atlas startup wiring remain separate named dependencies.
