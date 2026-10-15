# Integer-map full cleanup paths

This packet adds provider-free Source for two admitted full-path fragments and
one complete ordinary destructor. New descriptive API names are hypotheses.

| Native scope | New Source API | Bytes/instructions |
| --- | --- | --- |
| 004cef40..004cefbe only | clear_native_input_int_only_tree_full_prefix_004cef40 | 127/49 |
| 0055b230..0055b2ae only | clear_native_input_nested_int_tree_full_prefix_0055b230 | 127/49 |
| Complete 0055b490..0055b4c3 | destroy_native_input_nested_int_map_0055b490 | 52/23 |

Ordinary 004CEF40 and 0055B230 remain incomplete: their diagnostics and partial
paths are not implemented here. Retain library-shaped names
`STL_IntOnlyTree_EraseRange_N14_004CEF40`,
`STL_IntIntTreeTree_EraseRange_N20_0055B230`,
`STL_IntScalarTreeMap_Destroy_N20_0055B490` and all prior projections/records.
The older guarded `clear_native_input_int_only_full_range_004cef40`,
provider-bearing `erase_native_input_scale_tree_range_0055b230`, and guarded,
cached-head `destroy_native_input_scale_map_0055b490` remain unchanged. The new
whole destructor has a separate identity and preserves CURRENT head free and
defined EAX0; it does not add a null/repeated-destruction guard.

## Closure and producer-backed storage

Actual owning services are accepted `destroy_native_input_int_only_subtree_004c1890`
(node14h/nil11h), `destroy_native_input_nested_int_subtree_0055af60`
(outer20h/nil1Dh owning inner18h/nil15h), and `singleton_lifetime_free`.
There is no NativeStringStorage, RawPoolContext, callback, allocator or getter.
Private helpers share ordered raw field operations, without a runtime callee
abstraction. The compiled prefixes inline them and bind their actual callees.

Complete current producer/constructor bodies 004C2700/004C4DF0 establish the
14h node/head and 12-byte header; 00554CD0/00555360 establish the 20h node/head
and corresponding header. Producers write links0/4/8, color10h/1Ch and nil11h/1Dh;
constructors publish header+4, set nil1, self-link all three words and zero
header+8. Header+0 is opaque. Complete normal 0055A290 establishes key+Ch,
nested tree+10h, color1Ch/nil1Dh after successful child copy. Producer FH3
C6DB11/actions and copy children are context only, not reconstructed here.

Full 127-byte prefixes match accepted 005572F0 after ONLY three individually
proved E8 rel32 fields (two diagnostic targets and the actual owning subtree).
Raw complete spans, field offsets/targets and all unmasked bytes are retained.
This proves the repeated native schedule; it does not promote whole erasers.

## Admitted path, exact comparisons and reset order

Both fragments require a nonnull valid stable owned tree/head;
first_owner=last_owner=tree, first_node=initial head.left and
last_node=fresh head. Output is a writable private 8-byte iterator cell.
Selection inputs remain stable. Raw TEST/CMP preserves first-owner TEST,
initial head.left load, owner/first comparisons, last-owner TEST, fresh head
load, owner/last comparisons, and then a fresh root load. Positive full-path
outcomes are preconditions. There is no invented guard, abort, bad-path stub,
diagnostic callback or partial dispatch.

Actual owning subtree cleanup precedes CURRENT head reload/parent=self;
reload head BEFORE count0 then left=self; reload head/right=self; reload
head.left and capture result BEFORE output.owner=tree then output.node=result.
EAX returns output. Opaque owner+0 is not written; the prefix retains the head sentinel.
Raw byte/DWORD accesses avoid invented C++ overlays and pointer arithmetic.
There is no x87/NaN operation.

Whole 0055B490 captures CURRENT head then head.left, passes self twice with
those first/last values and private output to the 0055B230 fragment. Only stack
argument writes occur before helper selection, so valid stable actual headers
prove both positive owner/range comparisons; diagnostics/partial erasure are
untaken. Afterward it reloads and frees CURRENT head, zeros header+4 then +8,
leaves opaque+0 untouched, and returns DWORD0. The native XOR EAX,EAX is defined;
the exported pseudocode's void signature omits it. No cached-head free or
zero-head no-op is introduced.

## ABI, scope and evidence limits

Native prefixes have ECX tree, EDX unused, five stacked words
(output,first owner/node,last owner/node), RET14h/EAXoutput. New fastcall Source
interfaces retain that input shape; full-path preconditions exclude ordinary
diagnostic/partial routes. Whole 0055B490 has ECX header, unused EDX, no stacked
inputs, plain RET/EAX0. Source reserves ignored EDX and exposes DWORD0.
Physical input shape and emitted RET do not establish original ABI compatibility.

Readable finite uniquely owned actual trees/sentinels and matching current-CRT
allocations are required. No null/topology/sharing/alias repair or provider is
added. Owned scopes have no local EH registration/catch; producer EH is distinct.
Private helper/Source frames and output placement, register/flag residue,
original CRT heap identity, faults/SEH/stack exhaustion and game execution remain
unverified. Neither partial erasers nor whole InputSettings record readiness
is promoted. Excluded partial ranges are 004CEFC0..004CF008 and
0055B2B0..0055B2F8; each typed body excludes the preceding NOP90 byte.

Nine complete native context bodies cover 978 bytes/360 current instruction
starts; owned Source coverage is 306 bytes/121 instructions. All live bytes
equal installed disk bytes without masks; all current starts match Capstone
5.0.7. Project/program were verified as `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Ghidra was read-only. Existing differential seeds
do not execute these new APIs; no new tests or native/game claims are added.

## Exact native transfers, with excluded/context routes explicit

Admitted cleanup: `004cef8a CALL 004c1890`, `0055b27a CALL 0055af60`.
Whole destructor: `0055b4a6 CALL 0055b230`, `0055b4af CALL 00bf65ac`.
Excluded diagnostics: `004cef5a CALL 00bf6713`, `004cef76 CALL 00bf6713`,
`0055b24a CALL 00bf6713`, `0055b266 CALL 00bf6713`.
Excluded partial context: `004cefca CALL 00bf6713`, `004cefd9 CALL 004b9f40`,
`004cefe7 CALL 004cdec0`, `0055b2ba CALL 00bf6713`,
`0055b2c9 CALL 00552e20`, `0055b2d7 CALL 0055ac80`.
Accepted comparator context: `0055730a CALL 00bf6713`,
`00557326 CALL 00bf6713`, `0055733a CALL 00554f10`,
`0055737a CALL 00bf6713`, `00557389 CALL 005531f0`, `00557397 CALL 00556dc0`.
Producer context: `004c2702 CALL 00bf681b`, `004c4df3 CALL 004c2700`,
`0055a2b3 CALL 00bf681b`, `0055a2f2 CALL 00558570`,
`00554cd2 CALL 00bf681b`, `00555363 CALL 00554cd0`.

The exact MSVC Win32 Source build and all three existing suites passed in 9.36
seconds. All 26 report transfers and 26 Markdown claims passed with no unchecked
claims or warnings. Static COFF inspection confirms both inlined selection/reset
schedules, actual owning targets, current-head cleanup and defined EAX0. Source
code bodies are 117/117/85 bytes versus native 127/127/52-byte scopes; these are
code sizes, not stack frames or original binary/ABI/runtime equivalence.
