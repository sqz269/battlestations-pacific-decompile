# DamageableClass back-row selection Source

This is ordinary, unbound Win32 Source for `0087CE88..0087CEBA`: 51 bytes /
15 Native instructions in parent `0087CA80`. It selects the raw row address
following the already completed row-begin fragment. `0087CEBB`, all row fields,
iteration, production binding and the complete parent remain excluded.

The source is `src/native_damageable_class_back_row_selection_fragment.cpp`;
its explicit borrowing contract is in the corresponding public header. The
complete machine-readable receipt is
`reports/cc12_damageable_class_back_row_selection_source.json`.

## Retained Native evidence

The admitted readiness is the only Native window used. A fresh replay of the
same installed parent bytes, saved listing and retained live listing verifies
3,238 bytes, all 858 instruction starts and parent SHA-256
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
All 51 selected bytes and 15 instructions match retained SHA-256
`0b5b7fb9484dc4c5b30ef97bcf62886907a7d10f11cc22455ecf15f0492442e0`.
No live query, new Native child/data/handler/body window or Ghidra mutation was
made. Existing 37-state parent evidence supplies the enclosing state13 contract.

## Actual storage and providers

The caller supplies the same retained EDI header identity established as actual
descriptor+18h before append, the actual writable four-byte S+DCh slot, the
successful live state13 iterator owner and its unchanged actual scratch. S is
ESP after the parent's E4h locals/four saves. The prior row-begin run must have
completed normally, including its state14 temporary cleanup back to state13.
The references document existing ownership; this function neither inspects nor
changes the owner and does not create a guard or replacement container.

The supplied `NativeDamageableSectionVectorAccess` is the same actual access
used for append. Its genuine `SingletonLifetimeCallbacks` service/context is
borrowed directly. Every required failure call invokes
`invalid_parameters.invalid_parameter(invalid_parameters.context)` exactly as
the existing vector Source does. All three calls can return or throw. There is
no callback cast, null fallback, no-op, invented service or change of policy.
The access and service bindings must remain stable and calls must not reenter.
Callback writes to actual header contents and S+DCh are permitted and retained.

The current complete vector/provider sources and the inherited admission
receipts are pinned. The genuine zero-argument BF6713 Source wrapper exists,
but has a different signature from the existing context service; this fragment
does not silently replace that service with the wrapper. Its Native policy and
the concrete DamageableClass application service binding remain unproved.
The readiness's bounded searches and their qualified interpretation remain the
binding evidence; another subsystem's real service is not this receiver's binding.

All native field accesses use explicit raw x86 reads/stores. Header and cursor
storage need not contain live C++ integer subobjects. Header+0/capacity+Ch are
untouched. Stable aligned raw extents are disjoint from one another and Source
locals, binding objects and owner storage. There is no row dereference or
pointer subtraction outside an established allocation. SUB computes unsigned
32-bit wraparound; the ordinary API returns that raw address only on success.

## Preserved read, publication and callback order

| Native site | Source action |
| --- | --- |
| CE88 | Capture E0 from current header+8 exactly once. |
| CE8B..CE90 | Read current begin; call the real service if begin > E0. |
| CE95..CE98 | Derive candidate = E0-30h modulo 2^32; compare it with current end. |
| CE9B | Publish E0 to actual S+DCh after saving that comparison. |
| CEA2..CEA9 | Above-end skips the next begin read and calls the service; otherwise read current begin and call iff candidate < begin. |
| CEAE | After any second call returns, derive the raw row from captured E0 again. |
| CEB1..CEB6 | Read current end again; call the service iff row >= end. |
| CEBB excluded | Return the captured-end-derived address after any third call returns. |

The saved comparison is materialized by SETA before the raw cursor store. The
store therefore cannot change the comparison or move before its end read.
The begin load stays conditional. A separate inline-assembly SUB after the
second call prevents the compiler from using an earlier row derivation there.
All four/five scheduled header reads remain fresh and in the same order.

The fragment makes exactly one possible cursor write. Failure at the first
call precedes it; failure at either later call preserves it and provider writes.
Callback replacements of S+DCh are not overwritten again or used as the row.
There is no retry, repair, clamp, validity assertion or rollback. Enclosing
ordinary C++ owners handle a propagated exception while remaining at state13;
the already destroyed state14 temporary is never retried here.

## Complete compiled-object review

Both compilations use MSVC x86 with `/O2 /Ob2 /MD /W4 /WX /fp:strict /EHsc
/std:c++17` and Release definitions. The current generated Release project has
no explicit `/Oy-` setting. The ordinary Release-default object and a separate
explicit `/Oy-` audit object are retained and reviewed individually; their
different stack addressing is not claimed to be identical code.

| Object | Physical sections | Complete executable bytes / instructions |
| --- | --- | --- |
| `local/back_row_selection_release.obj` | 6 | 190 / 61 |
| `local/back_row_selection_fragment.obj` (`/Oy-`) | 5 | 167 / 64 |

Each has one code section (physical section4) and one function definition.
There are no code relocations, direct project external calls, unresolved project
symbols, compiler helpers or new EH handlers/unwind maps. The Release object
has one section-relative relocation in `.debug$F` to its own function, symbol13;
the frame audit has none. `.bss` contains the existing MSVC header's unreferenced
`__Avx2WmemEnabledWeakValue` artifact, not an invented Native global or service.
Directive, debug, checksum, raw symbol/aux records and all section bytes are
retained as well as full raw object bytes.

| Observable operation | Release offset | `/Oy-` offset |
| --- | --- | --- |
| Capture E0 once | 21h | 20h |
| First callback | 3Eh | 39h |
| Current-end comparison / saved SETA | 52h / 55h | 4Ah / 4Dh |
| Raw S+DCh publication | 62h | 57h |
| Above-end branch / conditional begin read | 69h / 6Fh | 5Dh / 62h |
| Second callback | 85h | 75h |
| Row subtraction after second callback | 8Eh | 7Dh |
| Fresh final end read | 99h | 86h |
| Unsigned below-end branch / third callback | A8h / AFh | 92h / 99h |

Every service call pushes the real context, loads the current function pointer
from the same stable service binding and uses the ordinary cdecl Source ABI.
The final row is preserved across a returning third callback. The saved boolean
uses the compiler's incoming argument home byte after the scratch pointer's last
use; this is private Source ABI storage, not S+DCh or a write through scratch.
The ordinary return is EAX, whereas the original continuation retains ESI.
Register identity, fault locations, private spills and original stack/FH3/SEH
transport are therefore explicitly outside the Source claim.

The report freezes commands, full compiler/include logs, actual include closure,
whole indexed COFF including every auxiliary symbol and relocation, both complete
disassemblies, complete candidate/provider Source and baseline Git/artifact pins.
The five bounded current Source/build queries retain complete outputs. The
current candidate has no CMake registration in this worker packet.

## Validation and remaining boundary

Both strict object compilations succeeded without warnings or errors. The whole
functions and callback branches were inspected, and the retained Native replay
passed. No test, executable probe or normal build was run: Root owns later
registration, complete link/build checks, admission and integration. Prior
readiness and Source receipts are historical context, not new execution credit.

This adds one ordinary Source fragment and no complete Native function. Actual
application receiver/header/cursor/service binding, Native ABI/FH3/SEH/longjmp,
failure/fault identity, whole-loop/parent completion, startup and gameplay remain
held. CMake, providers, ledgers, existing receipts and Ghidra remain unchanged.
