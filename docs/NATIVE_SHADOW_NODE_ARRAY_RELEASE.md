# Native shadow node-array release

This module reconstructs the complete normal schedule at **00AE1C20..00AE1C5D
(62 bytes, 25 instructions)** within the admitted actual `NativeModelReference`
domain. Source, build, generated-code and game acceptance are separate: this is
an unbuilt source candidate. The descriptive name is a hypothesis. No parent,
standalone EH action or new Ghidra definition receives credit.

| Entry | Original ABI | Coverage |
|---|---|---|
| AE1C20 | ECX actual entry; no public stack arguments; RET | Complete normal body in the documented finite provider domain |

The actual pointer-array header is entry+44: data+44, signed count+48 and signed
capacity+4C. No host owner or duplicate array type is introduced. The existing
full EP AE19B0 provider supplies raw resize semantics. Actual AE0A50-produced
array contents and generic node ownership are still unproved.

## Native ordering and source ABI

The signed initial count gate skips data/node/slot reads when count<=0. Each
positive iteration reloads current data, reads the indexed node, TESTs it, and
captures the exact slot in ESI with LEA **before** the null branch. The LEA keeps
the TEST flags. A nonnull node reaches the real unlink/release provider. Only
after normal return is zero written to the **captured slot**. Null slots receive
no store. The index increments, current signed count is reloaded, and the next
iteration reloads data. Full AE19B0(0) runs on entry+44 after the loop, even when
the initial count gate skipped it. There is no upfront length capture, bulk
clear, rollback, extra cleanup or shortcut count assignment.

The naked source entry adds a borrowed `GeneratedModelLifetimeRuntime*` in EDX.
If S is original entry ESP, the private runtime word is S-4, saved EBX S-8 and
EDI S-12. Only the positive path saves ESI at S-16; its bridge reloads the runtime
via `[ESP+C]`. The final resize consumes its pushed zero with RET4. Every normal
exit restores optional ESI, EDI and EBX, removes only the private word, and ends
in explicit C3. The native body has no public argument words or FP instructions.
Incidental volatile-register results and arbitrary stack aliases are not claimed.

## Concrete model domain

The source-only bridge calls **the same** runtime's `find_actual_node(actual_key)`
and requires `NativeModelReference` with a checked cast. It checks host owner
phase before address formation, the same actual `owner.storage.node` address,
and the same `owner.environment.nodes.attachments`. It creates no binding,
reference, owner or registry. It does not accept the generic base pointer alone
as proof of an actual model.

The bridge calls the existing `unlink_and_release_render_model_00b6dfa0` fragment.
Its actual `NativeModelReference` virtual18 provider checks the **current** model
table slot18 against B6F310 **after unlinking**, at the existing release point.
The bridge does not pre-read or snapshot a native profile/slot. Existing logical
release, point-light, child, retained-resource and terminal providers keep their
own lifetime and current-target requirements.

`NativeNodeBinding` supplies actual-backed +30/+34/+38/+3C/+40 hierarchy fields.
Every reached raw target must have a stable binding in the same canonical scene
runtime. For parentless nodes, +A4 is null or an already-supported source
`RenderNodeRootList` view. That reference-bearing C++ view is **not** a decoder
for arbitrary raw native root-table storage. The parented path does not
dereference +A4 before logical release clears it. No raw-root facade is added.

Release can free model backing and retire its companion. No owner, reference,
model or native storage is accessed after the call; only the previously captured
array cell is cleared. The caller must keep the actual entry and captured cell
valid across release, even if a callback changes the current array data/count.

Missing, wrong, dead or cross-runtime bindings cause source-only `logic_error`
diagnostics before the provider. Invalid inputs inside the existing `noexcept`
unlink/release path can terminate. Native hardware-fault/FH3 behavior and
exceptions through naked frames are unproved. Valid concrete provider domains
remain prerequisites; no failure rollback or successful fallback is invented.

## Calls and evidence

| Site | Target | Preparation / boundary |
|---|---|---|
| AE1C3D | B6DFA0 | ECX current actual node; source bridge supplies the existing canonical concrete lifetime |
| AE1C56 | AE19B0 | ECX entry+44, pushed zero, callee RET4 |
| AD7423 | AE1C20 | AD7421 loads ECX from [EDI] in AD7360's iterator-checked neighborhood |
| AE1E00 | AE1C20 | Unchanged entry ECX; wrapper then RET4, ignoring its public word |

The report carries numeric call rows, all ten checklist boundaries, original
ABI, provider hashes, caller preparations and the retained ER/primary admission.
Fresh guarded queries confirm the complete current body and 62 live bytes equal
the installed PE (SHA b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6).
No Ghidra, ledger or provider source was changed. EP source at base 4d0a0cbd is
accepted; its first combined build failed a CMake timestamp operation before
tests, and the coordinated serial retry is separate. This packet adds no tests
and runs no standalone build or fixture. Actual parent/producer and gameplay
closure remain unproved.
