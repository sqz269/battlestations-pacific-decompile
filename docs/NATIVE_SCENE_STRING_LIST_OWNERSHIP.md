# Actual scene string-list ownership

This packet reconstructs the complete ordinary successful normal-flow bodies
of append `0094F270`, clear `0094F3E0`, and retire `0094F540`. The APIs take the
actual raw12 list and actual raw8 record addresses plus an explicit genuine
`NativeStringRawPoolContext&`. Names are descriptive Source hypotheses.
Independent review, registration and a dependent Main build remain Root gates.

## Whole Original and interface boundary

| Original body | Whole span | Bytes / instructions | Measured native interface |
| --- | --- | --- | --- |
| Append | `[0094F270,0094F2ED)` | 125 / 38 | ECX=list12; stacked record and DWORD flags; low BYTE read; EAX=new count; RET8; ESI preserved; private FS0 registration |
| Clear | `[0094F3E0,0094F441)` | 97 / 42 | ECX=list12; no stacked input; RET; no semantic EAX result; EBX/EBP saved, ESI/EDI saved on positive-count path |
| Retire | `[0094F540,0094F55F)` | 31 / 12 | ECX=list12; no stacked input; RET; ESI preserved; no semantic EAX result |

The accepted immutable planning family records disk/live/listing and typed
whole-body agreement. This phase reuses that evidence by explicit pins and
performs no Ghidra reads, writes or metadata repair. Source uses ordinary C++
calling conventions and an extra context argument. No physical native ABI,
Original instruction identity, private CRT/EH or game compatibility is claimed.

## Normal ownership schedule

The raw12 layout is slots at +0, DWORD count at +4 and capacity at +8. No host
container, NativeString object or fake list is constructed by these functions.
Volatile field access retains actual reads and stores; it adds no synchronization.

Append reads count then capacity and grows only when equal. It tests the flag
low byte after growth: zero copies, every value1..255 adopts, high24 bits are
ignored. Adoption stores the supplied pointer itself, including null, and
transfers unique nonnull ownership on successful publication. It performs no
pool operation. Copy allocates exactly `object,8,8` through
`singleton_lifetime_allocate`, then calls the accepted raw-context
`copy_construct_native_string_header_00426060` on that actual allocation and
the borrowed actual source header. After all reached providers, append reloads
count then slots, publishes the resulting pointer, performs count read/add/write,
then rereads and returns count. Capacity is changed only by existing growth.

Clear observes signed count. Its observed <=0 branch writes count0; negative
owned counts are outside admission. Each positive iteration reloads slots,
captures the record and exact slot address, and skips null records. For nonnull
records it calls raw `destroy_native_string_header_0041dd20`, frees that same
captured raw8 through `singleton_lifetime_free`, then zeros the captured slot.
It rereads current signed count after reached providers on every iteration and
sets count0 on normal completion. Capacity, slot allocation and spare slots
remain unchanged. The compiler may reorder the private index increment with
the final volatile count load; this has no observable difference in the
admitted serialized, nonfaulting domain.

Retire calls clear, then reloads current slots. It frees nonnull slots through
the same canonical current heap and only then zeros slots. It leaves capacity
and the actual raw12 header allocation/lifetime to the caller. Append/growth
after retirement requires the genuine constructor to initialize storage again.

## Genuine providers and admission domain

The caller prepares the exact retained
`bsp::game::game_native_string_process().raw_context()` before entry. Its
noncopyable process owner is retained through a function-static heap pointer;
the context borrows permanent real pool01090AA8, gate01090AA4 and
manager01090AA0 cells and owns no buffers. These functions neither call the
process getter implicitly nor invent publications, callbacks, a host, globals,
TLS, an allocator, a pool generation counter or an FS protocol.

Raw destruction captures data first. Null data skips length/getter work;
nonnull data captures length+1 and resolves `00419CC0` before every `BD1510`
return, including large blocks and disabled small returns. The genuine gate is
preserved; there is no cached pool and no noexcept storage adapter. Clear and
retire remain potentially throwing. Existing raw-copy memmove/nonzero-copy
policy and canonical current CRT allocation/free remain explicit Source bindings.

Storage constructor/growth Source commit `87736664b93c` is already integrated,
registered, independently/primarily accepted and has passed its required Main
build/archive gate. The passed Main report is separately frozen in this family;
the older authoring-baseline readiness text that said pending is superseded by
that gate. Provider source/header bytes equal this worktree's versions. This
packet does not modify or duplicate either storage body.

- List storage is actual aligned writable raw12 initialized by genuine
  `0094EFB0` and accepted growth, with initialized reached slots,
  `0 <= count <= capacity <= 0x3fffffff`, capacity at least64 and a multiple64,
  and its genuine nonnull slot allocation of exactly `4*capacity` bytes.
- Full append requires old capacity at most `0x3fffffbf`. Count+1, index*4,
  all addresses and positive requested sizes remain accessible/nonwrapping.
  No negative-count ownership, overflow, saturated allocation or memory-pressure
  branch is admitted.
- Every nonnull adopted/owned raw8 is a genuine separately allocated current
  `object,8,8` heap header, with exclusive ownership of its nonnull data. Headers,
  data, list, slots and private frame are pairwise disjoint; no duplicate owners,
  stack/inline/synthetic adoption or owning aliases are admitted. A copy source
  may borrow a stable existing record and creates a distinct owned output.
- For nonnull data, nonwrapping length+1 is the exact original allocation size.
  Sizes1..149 have valid current canonical pool-generation/size-class provenance;
  sizes>=150 have matching current CRT provenance. Generation validity spans
  allocation through final return, including genuine publication/manager lifetime.
- All reached providers return normally with genuine positive storage available.
  There is no throw, fault, asynchronous SEH, concurrent mutation, reentry,
  new-handler mutation of these objects, trim, drain, teardown or generation/
  lifetime replacement. Flat Win32 addressing/ES and DF-clear provider conditions
  remain. These are caller preconditions, not newly added runtime repairs.

## Separate current-C++ cleanup policy

Root explicitly selected a copy-only fresh-header guard. After successful
allocation it owns only that actual new raw8. If current C++ copy construction
throws, the guard canonical-frees the captured header only. It does not release
copied data, an adopted header, slots or raw12. Source disarms after successful
slot/count publication and the return-count read. No public ownership function
is marked noexcept. Failure outcomes remain outside normal-flow admission.

Original append installs `CA8CCB`; that handler loads `DDCA34` then jumps
`BF6B43`. Metadata starts `19930522,1,DDCA2C`; the map contains
`FFFFFFFF,CA8CC0`. The11-byte/5-instruction cleanup reads private `[EBP+8]`
and calls `BF65AC`, without a data destructor. Exact private frame mapping,
FH3 dispatch, faults, partial publication and all unwind outcomes remain
unproved. No literal Original handler or handler stub is installed or called.

The actual current compiler emitted a separate guard local at EBP-20, a
state0 unwind map, a cleanup tailcall to the genuine guard destructor, current
`___CxxFrameHandler3`, current GS checks and current FS registration. Its44-byte
`.xdata$x` contains one8-byte unwind row and one36-byte FuncInfo with
`FI_EHS_FLAG`. This is current compiler output, with zero Original EH credit.
The optimizer removes normal guard disarm/destruction code because nothing
after the copy can throw a C++ exception; asynchronous faults remain excluded.

## Verification and remaining gates

Exactly one strict MSVC19.51.36244 Win32 compile passed with `/W4 /WX /EHsc
/O2 /Ob2 /Oy- /MD /Gy /GL- /fp:strict`. All105 compiler-declared inputs match
fresh own pre-pins;132 prospective files,71 tools,17 providers and3 build
configuration files were measured before that attempt. No compile was replayed.

The actual5332-byte COFF SHA-256 is
`1c0c048795f06c7cd4e3ddf09832a8223ecf90219392543c8ed6260ba09fc857`.
All21 sections,564 code bytes/230 decoded instructions,17 code relocations,
4 metadata relocations,48 primary symbols/21 auxiliary slots,1553-byte string
table, runtime data, CodeView, frame-debug records, directives, checksums and
complete file ranges are accounted. The230 includes five unreachable INT3
alignment bytes;225 are non-alignment decoded function instructions across
all emitted sections, including unused inline/template COMDAT helpers. This
does not claim all225 are reachable from the three public APIs.
The three public emitted bodies are182B/63,87B/39 and101B/41; retire contains
an inlined clear. Header-emitted4-byte weak COMDAT
`__Avx2WmemEnabledWeakValue` has no incoming relocation or initializer and is
not an ownership publication. There are no application static initializers.

Existing Main compiler-command records already contain `/O2 /Ob2 /Oy-` for
storage and the string/allocator providers. The imported Win32
`Platform.Common.props` sets `ClCompile.OmitFramePointers=false` when unset,
explaining the absent explicit project property. No additional ownership-only
frame-pointer option is required for the observed current toolchain. This was
checked from existing inputs only; no compile/build was added. Future actual
Main code still requires review, without silently normalizing differences.

Exactly one normal `scripts/build.ps1` run passed with2 existing checks
(10.14 seconds for CTest). This new translation unit remains unregistered
and absent from the generated core project. That build verifies the existing
repository baseline; the strict object verifies these new functions. No new
tests, probes, new-body execution, replay or game run occurred.

Root still must arrange independent review, registration, the dependent Main
build and archive closure. Actual `008F3270` output-header allocation, creation,
pass/retire/header-disposal and genuine incoming raw8 producer schedules remain
unresolved. `008F2B20`, comparator/sort `0094F180/0094F1B0`, and the `F89B75`
empty role remain separate. The earlier comparator metadata rollback's missing
pre-create complete symbol/comment state is preserved, not repaired or erased.

Evidence: `reports/native_scene_string_list_ownership.json` and ignored local
`WholeSceneStringListOwnershipSource01`; original accepted evidence remains in
`SceneStringKeyListOwnershipContract01`. Prior60/39/70 sealed families are
preserved. Only the owned header/source/document/report are committed.
