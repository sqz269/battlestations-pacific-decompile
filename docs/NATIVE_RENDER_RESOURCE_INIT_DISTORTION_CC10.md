# B107F0 distortion and depth-frame continuation

This packet owns only `[00B11E40,00B11EF4)`, 180 bytes of the existing
`BSP_RenderResources_InitializeMembers_PartialEntry`. The last included
instruction is the five-byte `CALL B0FC10` at B11EEF; inclusive end B11EF3.
B11EF4 begins the excluded next 20h post-effect allocation. The containing
function's original ECX service, three DWORD argument cells and eventual
B13026 RET0C remain unchanged; this source fragment executes no native return.

The live Ghidra bytes match the configured original PE, SHA-256
`f5551df85e217b6d516ee9400d69b85cb39642b72739a37c3f906579c1826efb`.
Read-only boundary evidence is retained in
`local/render_init_distortion_next_boundaries.json`; byte evidence is in
`local/render_init_distortion_next_live_bytes.txt`. Ghidra project is
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. No Ghidra
mutation or new complete-function claim belongs to this packet.

## Exact caller order

1. B11E45 allocates actual 26Ch storage, publishes raw allocation to retained
   ESP14, arms state54 and conditionally calls B4F0C0 at B11E62. Its constants
   retain their actual individual read order. No whole-object initialization.
2. Push inherited EDI halfheight and EBP halfwidth; capture the constructor
   result as initializer receiver. Disarm state, then publish that result,
   including null, to service+30 at B11E76. B11E79 calls B4F560 on the captured
   result, **not** a reloaded service field. No null fallback is added.
3. A true AL branches directly to B11EA6 without reading service+30. False AL
   reloads current service+30 into EDI at B11E82. Null skips both release and
   clear. Otherwise B11E8D captures the current CE2220 import and calls it on
   captured EDI+4. Its returned value, not a new count read, controls terminal
   dispatch. Only at zero do B11E97/B11E99 reload the captured owner's current
   profile and slot0; B11E9D dispatches that captured owner's existing canonical
   companion. It can differ from the newly constructed owner.
4. B11E9F clears current service+30 after normal decrement/terminal return,
   overwriting any callback replacement. An exception prevents this store.
   No orphan repair or extra release is performed when callbacks change +30.
5. B11EA8 allocates actual 40h frame, updates ESP14, arms state55 and
   conditionally calls B1FBB0 at B11EC5. B11ECE freshly reads original third
   argument cell word08; B11ED5 reads original height; B11EDA publishes the
   frame result/null to +1C8; B11EE0 reads original width. The third argument
   read is retained even though B0FC10 ignores its value. Disarm and call
   existing complete B0FC10 at B11EEF with width,height,third word.

## Actual providers and admission

Use the shared CRT allocation provider, complete B4F0C0/B4F560,
`NativeDistortionInitializationBlock`, complete B1FBB0/B0FC10, original import
cell and the existing actual-owner registry/count. Root owns the separate
D61F1C `NativeRenderPassReference` extension, whose deleting target is B4F540
and whose `NativeRenderPassCompanionContext::distortion` points to the actual
`NativeDistortionLifetimeContext`. This caller does not port those callees.

Retain the distortion block, its three post construction blocks/names,
scene/frame/camera/viewport receipts, canonical pass companion, lifetime
context, depth-factory acquisition record and complete predecessor chain.
Bind the lifetime context's scene publication by reference to
`block.scene_owner_3c()`, never to a copied snapshot. The scene companion can
self-delete; its publication then remains stale and must not be dereferenced.
All contexts and blocks survive callbacks and dependent stages; reset and
destruction require the existing explicit external quiescence contracts.

Canonical metadata must be bound before service+30 is published and before
B4F560, including its capability-failure path. Binding changes no native bytes
or count. **The actual 26Ch allocation's +34 camera/+38 frame/+3C scene preimage
must already satisfy the real cleanup contract whenever early or reentrant
terminal release is possible.** B4F0C0 and the initializer's false capability
prefix do not initialize those fields. This source supplies neither zeros nor
a replacement allocator nor a cleanup promise for arbitrary heap preimages.
A nonnull scene preimage must have a matching live scene companion; the fresh
block's initially null scene publication does not prove that condition.

False-return replacement owners must already have their own canonical
registry entries and valid contexts. The caller resolves the captured current
owner at zero, rather than assuming its own newly constructed companion.
Nested holders, surfaces and textures keep their existing direct lifetime
paths. No blanket registration, second count, automatic unbind or raw deletion.

B0FC10 uses the same frame/surface factory/renderer/import domains: capture
current +1C8, clear its depth, load current renderer slot94/B2A9A0, allocate
the real depth surface, reload +1C8 to retain it and drop its creator through
the current decrement import. Preserve its actual acquisition diagnostics.

## Frontier and limits

At B11EF4, ESI is the original service, EBX is FFFFFFFF, EBP stays halfwidth,
ESP14 is raw 40h frame allocation, mask is zero and EH state is -1. On true
initializer return EDI remains halfheight; on false return it contains captured
service+30 bits (possibly null or retired storage, never a diagnostic dereference).
Original dimension cells, remaining spills and the same three argument cells
remain borrowed. The exact predecessor/context/entry/argument identities are
one-use state, not native resource credits or a machine stack.

This is a normal-path source fragment. Failures retain publication and
acquisition records and fail the whole predecessor chain. Existing callee
failure behavior remains authoritative. Full initialization/destruction,
native FH3/SEH, private-stack aliases, arbitrary profiles and gameplay are not
established. Earlier fragments retain their published static/build boundaries.

The predecessor header adds only a one-use successor identity and phases.
The stage checks the same original context/entry chain and actual provider
domains, including the exact scene publication cell. Root's bridge implementation
8e1f6f19b was integrated at 91243a334 before this packet's build.

Strict `scripts/build.ps1` passed MSVC Win32 Release with `/MD`; all three
existing CTests passed. The report verifier checked six direct call rows with
zero failures. Both indirect operands and all 53 instructions/eight calls were
checked against the complete live listing and 180 PE-matching bytes. No new
test or probe was added. Root's separate companion lifecycle fixture does not
establish this composed initializer or its capability-failure path at runtime.
