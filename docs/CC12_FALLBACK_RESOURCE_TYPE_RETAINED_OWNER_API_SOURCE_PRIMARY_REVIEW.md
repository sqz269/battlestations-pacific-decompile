# Retained fallback owner API primary review

The existing GameNativeTypeStorage now retains the independent fallback guard,
four actual descriptor words, and a stable borrowed view. The explicit
initialize_fallback_00b86a00 API checks the existing common storage identities,
constructs the genuine Scene provider over the owner's canonical cells, and
calls the existing fallback initializer with exact name token D631F4 and the
same caller-supplied counter. No counter getter runs before guard/name
publication. The private bootstrap counter relation remains a caller
precondition; current VFS construction already establishes that relation.

Root reviewed both Source changes and the unchanged delegated schedule:
sticky guard/name, Scene initialization, alternating Scene-parent loads and
receiver stores, then genuine counter increment and old own-ID publication.
The stack-local provider/context are synchronous borrowed adapters. Consumers
retain only the owner's actual cells. This owner remains noncopyable/nonmovable
and must survive its users and shared singleton drain.

The worker's compiler layout diagnostics preserve all 49 existing owner fields;
size grows from564 to592 bytes. Guard, four words and retained view are appended
at564/568/584. These are Source offsets, not original fixed-address layout.
Surrounding consumers were recompiled. The VFS object's16 changed functions
retain their lengths, mnemonic sequences and ordered indexed relocation
targets; all53 changed instruction operands are single size/offset literals
increased by28. The renderer object's complete code and relocation sequences
are unchanged, although its physical object changed. Game main is byte-identical.
Only the owner constructor changes among existing owner function bodies; its
two new API functions are explicit additions. No new startup edge invokes them.

The normal MSVC Win32 Release build and all three existing CTests passed.
The121-byte/39-instruction owner initializer has exact real REL32 operands
at+0F/+36/+6A for the storage identity check, Scene provider constructor and
fallback initializer, with no direct counter getter. The seven-byte view getter
returns actual retained bindings. All498 normal code sections across the four
worker-compiled current owner/provider objects equal the worker's bytes.
The five immediate counter-external project targets have actual unique Core
definitions and freshly indexed genuine providers. Full normal objects for
the owner, fallback/mesh/light/sound/singleton providers and both affected App
consumers are retained, including symbols, AUX, relocations and all code.

The Source634 owner-API checkpoint retains634 selected project inputs,
88 whole Core objects, three App objects and231 positive selected Core
definitions. No input path is added: only the existing owner header/cpp change.
Of the prior88 Core objects,87 are byte-identical; one of three App objects
is byte-identical. Three additional selected-definition proofs cover the two
new APIs and the existing storage identity check; they are not three new
Native functions. All638 preceding Source634 frozen pins replayed, and a
separate638-pin owner-API snapshot preserves this changed state.

Root replayed1654 worker/manifest pin occurrences across1004 original paths:
1025 report/dependency/compile/COFF/layout references plus629 payload references.
All125 baseline Git blobs, two modified and123 unchanged worker project inputs,
236 external compiler include files and seven reported compiler components
verify. The sole current context difference from the worker is the already
accepted Compact CMake registration. All629 payload hashes,630 ZIP members and
CRCs pass. The earlier634 selected-input checkpoint remains distinct from the
generated project's complete compile-item count and the125-file worker scope.

This adds ordinary retained owner backing and explicit initialization capability.
It adds no Native body, ledger/GPR mutation, original ABI/FH3/fault claim, fixed
address mapping, CRT/absolute ID order, startup insertion, active classifier or
complete resource-loading/retirement graph. No new game execution occurred.
Current audio endpoint absence and complete startup/gameplay gates remain.

Report: [primary evidence and build receipt](../reports/cc12_fallback_resource_type_retained_owner_api_source_primary_review.json).
