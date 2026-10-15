# Nested integer-tree subtree cleanup

`destroy_native_input_nested_int_subtree_0055af60` reconstructs the complete
ordinary 0055AF60..0055AFC5 body (102 bytes, 44 instructions).
`clear_native_input_scalar_tree_full_prefix_005572f0` reconstructs ONLY the
consumed full-path prefix 005572F0..0055736E (127 bytes, 49 instructions).
The ordinary 005572F0 range eraser remains incomplete: its diagnostics and
partial-erasure path are not new Source. New API names are hypotheses; retain
`STL_IntIntTreeTree_DestroySubtree_N20_0055AF60` and
`STL_IntScalarTree_EraseRange_N18_005572F0`, prior frontend host projections,
and private AV/provider-bearing implementations unchanged.

The two new APIs use accepted actual
`destroy_native_input_int_scalar_subtree_00554f10` from
`native_input_int_subtrees.hpp` and `singleton_lifetime_free`. There is no
NativeStringStorage, RawPoolContext, callback, getter, allocator or node overlay.

## Producer layout and exact parent selection

The complete normal 0055A290 producer listing/bytes request 20h bytes, write
links +0/+4/+8 and key word +Ch, invoke 00558570 to copy the nested tree into
+10h, then write color+1Ch and nil+1Dh=0. The nested header's +0 word is opaque;
head and count are at outer+14h/+18h. Accepted 00554F10 separately establishes
inner node18h/nil15h. This producer is context evidence only; its FH3 handler
C6DB11 and child copy-constructor are not reconstructed by this packet.

On every nonnil outer node, the parent first destroys right+8 recursively.
It then captures CURRENT nested head+14h, captures that head's left+0, captures
CURRENT outer left+0, and computes nested owner=outer+10h, in that order.
It passes private 8-byte output, first_owner=last_owner=nested owner,
first_node=captured head.left and last_node=captured head to 005572F0.
Between capture and helper selection, only stack argument/private-frame writes
occur. A valid actual owned node gives a nonnull nested owner and readable
owned head; stable exclusive storage makes the helper's initial head.left and
fresh head equal those captured arguments. Both owner tests and both range
comparisons therefore select the full branch without a diagnostic call.

The fragment explicitly requires those full-path outcomes. Raw x86 TEST/CMP
preserves first-owner TEST, initial head.left load, owner/first comparisons,
last-owner TEST, fresh head load, owner/last comparisons, then fresh root load.
There is no invented bad-path guard, abort, diagnostic callback, partial dispatch
or unsupported-path stub. The excluded 00557370..005573B8 branch uses iterator
005531F0 and historical `STL_xlen_throw_00556dc0`; its name does not establish a
throw-only function. The original body has two ranges; 0055736F is a NOP byte
outside both ranges. It is preserved in the raw evidence, not Source coverage.

## Actual cleanup and representation writes

The fragment reads CURRENT head.root+4 and uses the actual 00554F10 service, passing root
bits also in its unused EDX position as observed. It then reloads CURRENT head
and sets head.parent=head; reloads head BEFORE zeroing tree.count, writes
head.left=head; reloads head and writes head.right=head; reloads head.left and
captures the result BEFORE output.owner=tree then output.node=result. It returns
output. Parent and fragment preserve all current-head reloads, without caching
the selection head for cleanup.

The parent frees CURRENT nested head after the fragment returns, then zeros
outer+14h and +18h, frees the outer node, and only then tests the captured
outer-left nil+1Dh for the next iteration. ANY nonzero nil byte stops without
freeing that sentinel. Outer+10h opaque storage is never written. There is no
null/cycle/sharing repair, payload initialization, host container or broad
tree-range adapter. Raw byte/DWORD loads and stores avoid invented C++ overlays
and pointer arithmetic; no x87 or floating-point operation occurs.

## ABI, ownership and verification limits

Native 0055AF60 captures/forwards ECX without dereferencing it; EDX is unused,
one node word is stacked, RET4, no semantic EAX result. Source reserves ignored
EDX and forwards zero recursively. Native full-prefix 005572F0 has ECX tree,
unused EDX, five stacked words (output, first owner/node, last owner/node),
RET14h and EAXoutput. The new Source fragment keeps that input shape and result;
it is not a replacement for the complete ordinary entry.

Reached headers/nodes/sentinels must be readable, finite and uniquely owned;
nonnil allocations must belong to the matching current-CRT free domain.
Selection inputs remain stable; the private output remains writable for both
stores. Native owned scopes have no local EH registration/catch. Source adds
no catch; producer EH remains separate context. Private frames/output placement,
register/flag residue, original static-CRT heap identity, original binary ABI,
fault/SEH/stack exhaustion and game execution are unverified. This packet does
not promote whole InputSettings record, partial-range or ordinary 005572F0
readiness.

Full live/disk bytes and typed body ranges are retained in the report. The
ignored local audit verifies three complete native normal bodies/436 bytes,
167 current instruction starts and twelve transfer edges without masks.
Project/program were verified as `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`; no Ghidra mutation occurred.

## Exact native transfers and Source scope

Whole parent: `0055af7b CALL 0055af60`, `0055af95 CALL 005572f0`,
`0055af9e CALL 00bf65ac`, `0055afac CALL 00bf65ac`.
Admitted prefix: `0055733a CALL 00554f10`.
Prefix diagnostics excluded by the proved precondition:
`0055730a CALL 00bf6713`, `00557326 CALL 00bf6713`.
Excluded partial context: `0055737a CALL 00bf6713`,
`00557389 CALL 005531f0`, `00557397 CALL 00556dc0`.
Producer context only: `0055a2b3 CALL 00bf681b`, `0055a2f2 CALL 00558570`.

The exact MSVC Win32 Source build and three existing suites passed in 8.02
seconds. All twelve native transfer rows and twelve Markdown claims passed
with no unchecked claims or warnings. Root independently reviewed complete
Source/header against the native parent/full-prefix listings with no findings;
frozen file hashes are recorded. Static COFF inspection confirms the retained
comparison/capture/reload/store/free order and RET14h/RET4. Source prefix/parent
code bodies are 117/172 bytes, compared with native 127/102-byte bodies; this is not
binary equivalence. No new tests or native/game execution claims; existing
differential seeds do not execute these entries.
