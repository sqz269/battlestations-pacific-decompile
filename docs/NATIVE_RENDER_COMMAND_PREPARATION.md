# Native render command preparation

This module reconstructs complete `00B1BF50..00B1BF67` (24 bytes) and
`00B1D910..00B1D945` (54 bytes). They prepare the actual `44h` command returned
by existing `00B1F1F0` in the directional-shadow job path. The parent queue,
shadow update and its EH actions remain separate work. Names are descriptive
hypotheses, not recovered application symbols.

## Actual storage and source interfaces

`NativeRenderCommandStorage` already has length/data at `+14/+18` and metadata
DWORDs at `+1C/+20/+24`. These entries operate on that storage without creating
an owner, retaining it, changing registration, or providing an allocator.
Command/header/buffer backing must remain valid for all reached accesses.

`set_native_render_command_metadata_00b1bf50` is a naked Win32 fastcall entry.
ECX is the command and its third source argument stays in the original public
stack slot; the second source argument explicitly occupies unused input EDX.
Its eight instructions preserve the native body: read the source pointer,
read/store word0, read/store word1, read/store word2, RET4. In particular,
source reads interleave destination stores. Source equal to command+18 observes
each preceding write in its later reads; copying an upfront triple would differ.
Incidental EAX returns the third word and EDX the second; the declared result is
void. There are no callees, conversions, validation or EH state.

`set_native_render_command_diagnostic_00b1d910` adds an explicit borrowed
`NativeStringRawPoolContext` through a new C++ ABI. The original is ECX=command,
one public stack source-header pointer, saved ESI/EDI, RET4, no semantic result.
Destination is the actual command+14 header. Exact source/destination header
identity returns before reading either header or invoking a provider.

For different headers, the entry captures source length and calls the existing
raw `resize_native_string_header_0041dd40` with preserve1. It then reloads source
length; zero returns. Otherwise it reads current destination length, current
source data, and current destination data **in that order**. The copy uses the
captured destination length, not source length or length+1. A zero-count standard
library call is omitted only after all three explicit x86 DWORD argument reads.
The read helper uses MOV on a byte-computed address, admitting unaligned raw
storage and pointer representations without typed C++ aliasing assumptions. No extra
header/null guard, terminator copy, default value, rollback or cleanup is added.

The raw resize already preserves current header reads across allocation/return
and performs the native terminator write on resize. Equal length can return
without touching data. Partial header aliases and valid buffer overlap retain
their native schedule. Native `00BF7680` has the correct `_memcpy` symbol but
explicit backward-overlap handling: `BF7694..769A` selects the backward path;
`BF785F..7862` executes STD/REP MOVSD/CLD. Consequently this source uses memmove.
The source does not claim CRT instruction/EFLAGS/fault parity, invalid memory
support, arbitrary concurrent modification or wrapping nonzero buffer ranges.

## Concrete provider and lifetime

The raw context borrows actual pool publication `01090AA8`, shutdown gate
`01090AA4`, and manager publication `01090AA0`. They must be the same cells/domain
used for the command diagnostic's construction and eventual destruction.
`RawStringPoolAccess` in `native_string.cpp` resolves the real `00419CC0` getter
before every reached allocation and return, then calls actual `BD1120/BD1510`.
It does not cache the pool or invent a semantic `SizedStoragePool`.

The existing command actual environment's `ActualNativeStringPoolStorage`
retains its noexcept returning-getter limitation for command cleanup. This
entry's raw-context resize permits source getter exceptions, propagating with
the stores actually reached. Neither entry owns the command or its caller's
temporary string. The existing semantic `BE0A30` copy fragment is not reused:
its interface, overlap domain and pointer-capture schedule differ here.

## Numeric native evidence

| Entry/call site | Target | Actual native preparation |
| --- | --- | --- |
| `A8EBD5` | `B1BF50` | ECX=EDI, the `B1F1F0` returned command; public pointer to local `(0,62h,1Eh)` record. Caller state0 already disarmed. |
| `A8EC1E` | `B1D910` | ECX=same EDI; public pointer to local actual8h header prepared by `41DD40(4,1)` and current-length+1 copy of `D5B5D0`, `SHBU\0`. Caller state1 armed. |
| `B1D924` | `41DD40` | ECX=command+14, stack captured source length then preserve1; RET8. Source composes the raw-context overload. |
| `B1D939` | `BF7680` | cdecl current destination data, current source data, current destination length; caller ADD ESP,0C. Source uses overlap-capable memmove. |

These are the only current direct callers of the two entries. A8EA20 itself is
called at A8F9CA by full A8F3B0. A8EA20's handler CB6523 references FuncInfo
DEC76C and map DEC75C: state0 -> -1 invokes CB6510, freeing raw `[EBP-38]`;
state1 -> -1 invokes CB651B, calling 41DD20 on local header `[EBP-34]`.
Thus failure in B1D910 while caller state1 is armed cleans that caller temporary,
not the command diagnostic or the already-created command. These EH bodies are
evidence only; they are neither changed nor reconstructed by this module.

The sealed EB admission supplies complete native hex/listings, all caller
preparations, original ABI, pool source pins, and PE/live byte equality.
Installed executable SHA256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Native B1BF50 SHA256:
`cf390856fca8f1387ad9b719d4db36a09ee56230b1bdde6ee6473d792641c683`.
Native B1D910 SHA256:
`c1f090a8c6ab43a9286b8477f5d8f3a51210c3114b7c98e2e270e3e0b0f7c178`.

## Validation status

Primary source review accepted the exact source and raw x86 read helper.
The canonical live call verifier checked all four numeric rows, with zero
failures. All eight native seed spans matched the installed image.

One clean MSVC Win32 build passed at exact commit
`d0481c6812af58a399d5759c33c0a8492346ef64`. Its 2,633 tracked build inputs plus
the verified seed header (2,634 total) stayed unchanged; HEAD and tracked tree
were unchanged/clean before and after. Both existing CTests passed:
`reconstructed_math` and `native_math_differential`. They are existing math
checks, not execution coverage of these preparation entries. No compiler or
linker warnings/errors were reported.

Retained `local/ec-final-build-stamp.json` pins all inputs, four libraries,
the exact1911-byte object and `local/ec-final-build.log`. Object SHA256:
`d8f4f8bc9970284ae3e9acb09cd988dfcf4218bca85e3decce9009acfe70be70`.
Final generated schedule/byte review is reserved for primary integration; this
worker does not claim completed generated-body identity review.

No new test, native fixture, queue/shadow execution or game validation was
performed. New C++ ABI, raw-provider exception restrictions, native FH3/SEH and
hardware-fault boundaries remain explicit. Final documentation/report changes
only record validation; the tested source and build artifacts remain frozen.
