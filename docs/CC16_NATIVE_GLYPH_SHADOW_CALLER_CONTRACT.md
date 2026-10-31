# Glyph raw-shadow caller contract

This is a design-only connection from exact Source42 `1fc25e4dd34b7eddf466119e63fe0ebcc3cffb34` to the whole raw shadow primitive `33d604f86a39c690a2ef4345952deb232b5e6da0`. No Glyph/Origin/other Source is edited or merged. Source43's separate body metadata/static/build admission cannot supply a concrete prepared Glyph producer, lifetime graph or runtime permission.

The minimal Source cut removes `NativeGuiTextGlyphQuadStorageCalls::shadow_00ab6c30`, includes the existing `gui_text_style.hpp`, and appends exactly two private borrows **after** Source42's complete `raw93_frame_`, `raw93_context_`, `raw93_acquired_` prefix:

```cpp
NativeGuiTextShadowStorageFrame& shadow_frame_;
NativeNodeSetParentAcquired& shadow_acquired_;
```

The mandatory acquisition constructor changes from three to five reference arguments, preserving the existing three initializers and appending the two new reference initializers. It creates no backing, owner, default, frame preparation, retain or `noexcept` promise. Copy/deletion behavior and the complete old public/private field prefix stay unchanged. The existing exact Glyph-entry friend can access these borrows; no new Origin/Cause friendship, writer, issuer or public flag is needed. The report gives the exact proposed constructor and call snippet; they are not implemented APIs.

## Genuine context and distinct records

The context route is already real: Source42's private `raw93_context_` is `NativeGuiTextRawModelDefaultStorageContext&`; its `sections` is `NativeGuiTextRawModelSectionStorageContext&`, whose `parenting` is the existing `NativeNodeParentingContext&`. The raw85 implementation itself passes that SAME context to actual `B6E680` and its `node.scenes.children` to `B6D890`. Therefore the proposed caller passes `a.raw93_context_.sections.parenting` directly. It does not reconstruct a context, infer a node association from semantic Text/Model objects or add a shared GlyphContext operation frame. Context identity alone proves neither reached raw-node validity nor its full current-target/lifetime binding.

The new `shadow_frame_` owns the existing parenting-frame subobject as ordinary caller backing, and the separately borrowed acquisition must be fresh. Neither may reuse `a.raw93_frame_.sections.parenting`, `a.raw93_acquired_.sections.parenting`, `a.raw93_acquired_.sections.model.parenting`, any nested constructor/recursive frame or an already entered/complete/failed record. Prepare distinct address-stable frame/acquisition backing before Glyph entry, including for every nested/reentrant Glyph invocation. The whole required Context, current node/profile/scene/tree/import domain and all callback-selected generations remain genuine independently retained caller obligations. No frame allocation/selection, reset, replay, default bank, finite cap or shortage policy occurs at the reached call.

The new frame/acquisition and their mutable homes must be separate from the ten Glyph homes, the fourteen ordinary locals, raw Text/child/current nodes, constructor records and borrowed provider/context/constant storage under the existing Source disjointness contracts. In particular, the new outgoing homes cannot alias the ordinary offset result or the RGBA words whose value/lifetime is being passed. This explicit ordinary Source backing contract excludes invented physical Native-stack aliases.

## Exact reached caller epoch

The saved BSP project/program was verified before the read-only batch. Native `00AB9DCE..00AB9E09` is a 60-byte, 15-instruction window ending with the direct call at `00AB9E05 -> 00AB6C30`. `00AB9D9D` separately establishes EBX=1. Actual argument order is:

| Native point | Argument/ordinary operand |
| --- | --- |
| `9DCE`, `9DD4`, `9DD7` | FLD current D5C5C0 offset, XORPS zero, LEA actual color address `[ESP+30]`. |
| `9DDB` | PUSH color DATA pointer first. |
| `9DDC..9DDD` | PUSH ECX allocates the offset home; FSTP immediately overwrites it. This is not authority to read or preserve the prior ECX payload. |
| `9DE0` | PUSH position DWORD zero. |
| `9DE2..9DF4` | Store three RGB zeros and load current D7A24C alpha. |
| `9DFC..9DFF` | PUSH enabled EBX=1; actual child is ECX; late alpha store completes the four-word color. |
| `9E05` | Direct whole shadow helper. |

The existing Source `prepare_shadow(c, a, shadow_offset)` is retained **byte-for-byte**, including its asm. It already spills the exact offset DWORD and prepares RGBA in `a.locals[7..10]` (byte offsets+1C..+28, the ordinary color addressed by the Native LEA and called F3C/RGBA in the brief). After it returns at relative x87 depth zero, keep the existing `boundary(a, 0x00ab9e05, child)` and replace only the opaque call. Stage the four actual new homes in Native PUSH order: `color_argument = bits(a.locals + 7)`, `offset_argument = shadow_offset`, `position_argument = 0`, `enabled_argument = 1`; then call the raw overload with actual `child`, those retained records and the exact context route above. No repeated float load/conversion, color snapshot, constant reseed or extra FP operation is needed.

This deliberately preserves the existing Source x87 schedule. It does **not** claim that ordinary-home writes after `prepare_shadow` reproduce Native PUSH-before-FSTP timing, native instruction faults, private argument-stack aliases or binary/FH3 ABI. Exact physical spill/home timing would require changing the existing asm and lies outside this preservation contract. With the stated independent backing and no callback/Native work between preparation and the direct call, the reached callee receives the already calculated payload and same ordinary RGBA objects. All22 asm blocks/78 x87 forms remain the required invariant, not a pretext for an unproved private-stack claim.

The helper itself is the whole 125-byte/32-instruction primitive, with two MOVSS forms and no x87. It reads position/offset/enable in its original order, captures the color pointer only after Text+160, preserves four sequential color reads/stores, selects CURRENT+4C from captured enable, captures CURRENT+188, and uses its fresh retained parent home. After parenting callbacks it rereads CURRENT+15C and only zero rereads CURRENT+188 for root propagation. Both real callees already exist. This contract adds neither their implementation nor any Native/helper/vendor/duplicate credit.

If the call escapes, preserve prior field/hierarchy/root/scene effects, the same RGBA/offset/backing and nested failure records; do not advance into the later shader/header path or reuse/reset/retry/clean anything automatically. Only actual normal return permits the existing later code to reuse ordinary locals. A completed frame's color pointer can then name reused storage; it is diagnostic history, never proof of historical color values or replay permission. No copied history buffer or restoration is proposed.

## Preservation and open gates

A later owned implementation must mechanically invert both whole Glyph files to exact Source42, limiting changes to the include, virtual removal, mandatory constructor/private borrow additions and existing call replacement. Preserve the real raw93 call/envelope/provider identity, body-return frontier, pool/GPA1/GPA2, current home7, three actual counter values, both header arms, complete catch, listener/code-unit calls, all ordinary homes and all22 asm/78 x87 forms. The existing9E05 diagnostic boundary remains; no new counter or origin edge is introduced. Counter preprocessing and compilation remain Root acceptance work.

Raw shadow invocation at this point is after constructor normal disarm and before the shader-header arm. Concrete calls, public phase/site/complete values or the constructor's private negative receipt do not authorize a Native failure action. The UNKNOWN/HOST negative protocol and all existing null-runtime/legacy/secondary/fault qualifications remain. No new cleanup, Native issuer, terminal policy or NoThrow assumption follows.

Caller Source implementation, genuine prepared producer/frame/context/current-target/lifetime binding, actual raw122 birth where later consumers require it, Original ABI/private aliases/FH3/SEH/faults, game runtime and activation remain false or unproved. This packet changes only its two evidence files and ran no compiler, tests, probe, game/process action or Ghidra mutation.
