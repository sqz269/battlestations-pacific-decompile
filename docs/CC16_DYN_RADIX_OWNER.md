# Dyn radix object owner: 00C55D30

This packet reconstructs the complete normal body at
`00C55D30..00C55D93` inclusive, 100 bytes. The last instruction is `RET 0Ch`
at `00C55D91`. Its current Ghidra name is
`Dyn_SAPRadixBroadPhaseManager_vslot0`; its old ledger row is
`DYN_unreachable_00c55d30`, status
`metadata_only_no_function_body_verified_oct24`. The new C++ name,
`bsp::cc16_dyn_radix_owner::create_object_00c55d30`, is a descriptive
hypothesis, not a recovered symbol.

## Target, evidence and ABI

The configured target is project `bsp` at `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, `x86:LE:32:default`, image base `00400000`.
Read-only `bsp.py ghidra proto` and `ghidra xrefs` wrappers verify that target
before their live batches. The leaf packet already inspected this parent's
complete pseudocode and assembly in
`exports/bsp/functions/00c55d30/{decompiled.c,assembly.txt}`; this continuation
retains that evidence and rechecks its live body boundary. No Ghidra writes
were performed.

The native method receives the manager in ECX and copies it to ESI at
`00C55D34`, before allocation. After saving ESI and EDI, the first stack
argument is read as a dword from `[ESP+0Ch]`, the second as a source pointer
from `[ESP+10h]`, and the low byte of the third from `[ESP+14h]`. Both native
return paths pop 0Ch bytes. ESI and EDI are saved/restored, EBX and EBP are
unchanged, and EAX is the object pointer after the helper returns. These are
the original calling conventions; the source uses explicit C++ arguments.

## Complete body and source composition

| Native evidence | Preserved behavior |
| --- | --- |
| `00C55D32..00C55D3E` | Allocate exactly 24h bytes and test the result. |
| `00C55D42..00C55D50` | On a nonzero result, store the first argument at object+00h, then the third argument's low byte at +1Ch. |
| `00C55D53..00C55D76` | Copy six dwords, in ascending source order, to object+04h through +18h. Save the object in EDI and pass it to the helper. |
| `00C55D79..00C55D82` | Append the actual object through the unchanged ESI manager, then return that saved object. |
| `00C55D85..00C55D93` | On allocation failure, pass zero to the same helper, then return zero only if the helper returns. |

The complete direct callee set is `00BF681B` and `00C37180`:
`00C55D36 CALL 00BF681B`, `00C55D79 CALL 00C37180`, and
`00C55D88 CALL 00C37180`. The source combines the two identical helper
invocations in one tail after optional object initialization. This retains
their ordering and inputs; it adds no failure return before the helper.

The owner composes the previously recovered
`cc16_dyn_radix_entries::append_object_00c37180` directly with the same raw
manager and the newly allocated object. Its `Heap` requires the separate
native `00BF681B` allocation provider plus the leaf's `00BF55BE` allocation
and `00BF6989` free providers. There are no fake globals, callback defaults,
unresolved-call stubs or vendor-body implementations.

All field accesses use raw Win32 addresses and ordered volatile word/byte
accesses, preserving the store order even if supplied source storage aliases
the new object. No `memcpy`, value initialization or guessed object class is
introduced. The owner leaves bytes +1Dh..+1Fh and +20h..+23h untouched before
the helper. The helper writes only the low16 index at +20h, so bytes
+22h..+23h remain untouched after a successful call. The word at +00h, the
six copied words and the flag are deliberately not assigned domain meanings.

If object allocation returns null, the original helper is still reached with
that null object. It updates the manager/entry according to its own contract
and attempts to write object+20h. The source preserves that invalid-store
path without adding a null guard. Valid readable six-word source storage is
required only on the nonzero object path. Usable providers and writable
manager/entry/object storage are required for a successful completion. Native
register/stack adapters, exception transport and invalid-memory fault
equivalence are not claimed.

## Incoming path and activation boundary

The current bounded index query reports zero direct callers for `00C55D30`
and two descriptions of its reference at `00D7A118`: DATA and vtable slot 0
of `Dyn::SAPRadixBroadPhaseManager`. The live cross-reference query also
returns only `From 00d7a118 [DATA]`. No incoming live references were found
for `00C37210`.

The earlier census in `docs/CC13_DYN1.md` identifies the Ghidra-undefined
`00C37210` as this class's only constructor and reports it unreferenced.
Current bounded disk disassembly independently confirms its vtable write at
`00C37212` and its final return at `00C37236`. However the live xref query
for `00D7A118` reports none despite that literal vtable store. Therefore
Ghidra xref absence alone does not revalidate the old exhaustive census.
This packet supplies the recovered owner-to-leaf source edge and qualified
incoming evidence; a runtime parent or an accepted dead-chain conclusion
remains an integrator obligation. There is no runtime activation claim.

## Validation and handoff

The report records all three direct call sites. `verify_report_calls.py`
checked all three report rows and all three Markdown call claims with zero
failures, unchecked claims or warnings. Source/assembly review covers argument widths,
initialization order, untouched bytes, provider identity and the failed
allocation path. No new tests, compiler runs, probes or game runs are part of
this packet. The primary owns shared C50000 ledger changes, any annotations,
CMake registration, MSVC Win32 compilation and relevant existing checks.
The leaf files are unchanged by this continuation.
