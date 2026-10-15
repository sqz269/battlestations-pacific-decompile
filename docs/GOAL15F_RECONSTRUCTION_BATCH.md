# GOAL15F avoidance singleton and raw container dependencies

This batch reconstructs the complete avoidance singleton getter and seven raw
container helpers. It also connects the getter's registered owner to the
existing finite Source shutdown dispatcher, using the recovered deleting slot
and the same actual publication-cell bindings as construction.

| Entry | Complete Source operation |
| --- | --- |
| 004218E0 | Fast/slow avoidance publication, captured parent lock, allocation/construction, registration and recovered unwind operations. |
| 0041DDF0 | Free the raw vector storage conditionally and zero its three pointer words. |
| 0041F230 | Clear the actual hull list, then free its current sentinel and null its head. |
| 0041E3A0 | Independent complete list-tidy body with the same operation. |
| 00419960 | Raw two-DWORD range copy with the physical register/stack interface retained. |
| 0041C7E0 | Raw two-DWORD fill with fresh aliased input reads. |
| 0041E3C0 | Complete fill-dispatch wrapper with its masked ECX metadata. |
| 0041F4E0 | Complete copy-dispatch wrapper with its masked ECX metadata. |

## Ownership composition

The manager's recovered state-2 and state-1 cleanup actions now invoke the
complete F230 list-tidy and DDF0 vector-tidy entries. Its normal destructor
retains its separate inline head/vector release schedule. No critical-section,
group or deferred-body rollback is added to those unwind actions.

The existing `NativeSingletonDeletionBindings` appends `avoid_manager` at offset
204, making the Source structure 208 bytes while preserving every earlier field
offset. The finite dispatcher admits both the recovered CE38F4 profile and the
matched actual Source avoidance table, supplying the real scalar deleter's EDX
context explicitly. Missing bindings retain the existing Source contract error.
The original raw manager pop/reload/free body is unchanged.

This closes the registered shutdown path for the explicitly bound Source owner.
It does not change Original virtual dispatch: target-in-EDX `CALL EDX` cannot
simultaneously supply the added Source context, and no compatible Original ABI
or automatic production application binding is asserted.

## Verification

The registered MSVC Win32 build and all three existing CTests passed. Three
archive-linked Source fixtures passed: populated registered singleton shutdown,
all three genuine-heap tidy operations, and POD8 alias/null/wrap/register/stack
behavior. The singleton fixture retains its real registered entry through the
complete drain, destroys a genuine group and body storage, and verifies the
retained external body queue and actual publication cells.

All 275 bytes in the seven raw helper kernels match Native bytes after exactly
seven verified CALL operand substitutions. The singleton getter is 189 Native
bytes versus 287 current Source bytes; those bodies are not byte-equated. The
existing compiled Source manager body remains exactly 167 bytes with all eight
relocations unchanged. Actual guard relocations now reach the reconstructed
tidy entries; the previous standalone hull-list/scalar kernels are unchanged.

All 31 report call rows passed: 27 have live function containment and four have
explicitly qualified raw/uncreated containment. Eighteen parsed documentation
claims passed. Independent singleton EH and POD8 ABI reviews found no blockers.

Nine Original physical prototypes and eleven evidence comments were saved in
the existing Ghidra project, with prior values recorded; 39 owned/dependent
exports and the snapshot were refreshed. Ghidra's automatic `this` stays a
void-pointer marker for the POD8 wrappers' raw ECX metadata. An initial scalar
typing attempt inserted an extra stack parameter; the correction was saved
under the write lock, and both final four-parameter layouts and decompilations
were verified. No recovered class or extra EDX input is claimed.

Eight complete function records and two scoped Source-integration fragments
were added without removing earlier reconstruction records. The companion
[machine-readable receipt](../reports/goal15f_reconstruction_batch.json)
records the exact tested Source revision, final artifacts and evidence logs.

The singleton getter's Native EH map has two states. The current compiler also
emits a termination action, giving the current Source map three states. Its
allocation guard ends before publication/registration, and its captured lock
guard remains active through normal Leave and the final publication reload.
These current C++ operations do not establish Original FH3/SEH, hardware-fault,
exception-type, private-heap or double-exception identity.

The POD8 physical-call census distinguishes nine live Ghidra calls from four
additional disk/live-byte CALL sites in uncreated wrapper ranges. No guessed
nearest-function membership or recovered symbol is asserted for those ranges.
Earlier semantic and typed records remain separate historical evidence.

Native execution, drop-in ABI compatibility and gameplay remain unverified.
