# Raw Text Font operand readiness, packet 141

ABA8D0's two Font operands are now tied to their Native producer and lookup identity. The existing Source font graph supplies genuine decoded data, actual image references and stable descriptor associations. It does not supply the single raw Font allocation addressed by Native Text+108. A descriptor, `FontData` or `NativeFontResources` cast cannot close that gap.

This is a bounded read-only readiness result. No Source, ledger, Ghidra, CMake or host activation changed; no build, test, probe or Native body execution ran. The 136 whole-body audit is reused, not repeated or promoted to Source closure.

## Baseline and evidence

The worker fast-forwarded cleanly from captured 139 commit `1beffbe14ae8de24471f295abd77766c899a724e` to the requested published Main `aa4d5609e84f0b9a7f2f32079358deb98746e0dd`, with merge exit 0. That baseline includes 137 cursor, 138 vector-copy and 139 raw wide assignment. Origin subsequently advanced to `048d7dd5f4f23596fee17a3c9ae1564ce9f47edf`; this report remains pinned to aa4d5609e. All 42 frozen Source/tool inputs are unchanged between the captured-139 and audited baseline commits. No moving Main working files were used.

Every fresh live query used `bsp.py ghidra`, whose client verifies project `bsp`, program `/battlestationspacific.exe`, x86 language and image base before querying. Configuration names `C:/Users/sqz269/bsp.gpr`. Fresh byte spans equal the unchanged installed Original, SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The private packet directory is `local/cc12_native_gui_raw_font_operand_readiness_141/`. `native_inputs.json` records byte extents/hashes; `operand_evidence.json` contains selected instructions, receiver transport and 13 direct-call rows; `source_inputs.json` pins actual Source bytes and Git blobs. Existing exports were copied without regeneration. Whole producer byte footprints are preservation evidence, not claims of a new whole semantic/EH audit.

Reused 136 manifest SHA256 is `e3861d8922a1a9a2ece8b1c00b995440a5cbce5371670f2b75c9fdb0b1b2cd3a`. Its preserved ABA8D0 bytes are `[ABA8D0,ABAEC7)`, 1527 bytes/451 starts, SHA256 `81b87216443b038af05450c9d23a2348da40c546753edbba95265cdb6cea8ea8`. Sealed 136/139 artifacts were not replaced.

## The two actual operands

| Site | Actual operation and order |
| --- | --- |
| ABA8EC / ABA8FA | Ensure draw sections, then copy the input into the private wide header. These precede the Font read. |
| ABA8FF / ABA907 | Load CURRENT Text+108 into ECX; compare BYTE at that raw Font+48 with zero. There is no null admission here, including equal-empty input. |
| ABA90B / ABA91C / ABA945 | Only after the BYTE read, store local EH state 0; optionally uppercase through the current mapping; reach the existing length gates and comparer. |
| ABADDE / ABADE8 / ABADEC | On the reached late shadow path after intervening geometry/material work, reload CURRENT Text+108. Nonnull reads WORD+14 with MOVZX, then MOVSX from AX. The null arm gives height zero. |

The late null arm does not make a null upfront Font valid. A changed Text+108 may select a different still-live Font at the late read; no early pointer or height snapshot is equivalent. The height is a signed lowword. Neither measured Text width nor a descriptor scale ratio can substitute for it. The later +7C read previously checked in 136 belongs to the primary MATERIAL, not this Font.

Original receiver/stack/EH transport stays as recorded in 136: ECX actual Text, one actual 8-byte wide-header argument, RET4; ten physical cleanup states. This packet adds no Native FH3, fault-timing, register, catch or CRT-locale compatibility claim.

## Native Font identity and producer

The producer path is concrete, not inferred from type names:

1. AC3910 reads `uppercase_only` through B662F0 at AC3A98 and saves AL at AC3AA1. The literal at D5CAB0 is independently live=disk.
2. AC3AB2 pushes **AC hex bytes (172 decimal)**; AC3AB7 calls BF681B. AC3ABC keeps that returned allocation in ESI. On the nonnull arm, AC3B4D calls AD55C0 with that same receiver, the name, two float-bit arguments and the saved uppercase byte.
3. AC3B52 captures constructor EAX in EBP; AC3B54 jumps over the allocation-null-only `XOR EBP,EBP` at AC3B56. There is no EBP write on the intervening successful path before AC3D8F passes EBP as the AD4C30 receiver at AC3D99. The raw list append is between those calls, before resource loading completes.
4. AC3570 walks the raw registry list. It reads the Font name header at payload+0C/+10, compares equal lengths then BF7FBF, and returns the first matching node's SAME +08 payload at AC35F0. It does not manufacture a descriptor copy.
5. AB8C77 calls actual getter 7371D0, AB8C7F calls AC3570, and AB8C86 stores the returned pointer directly to Text+108. Nonnull +1C supplies Text+1D4; only the miss arm uses actual D7A24C. Getter 7371D0's existing evidence identifies the F8BF44 registry publication, allocation 10h, AC3690 and shared singleton registration.

AD55C0 is `[AD55C0,AD5691)`, 209 bytes/71 starts, SHA256 `0efe2fd5d21694683d3f1535634ed7640d681cd8ac3a366dc1b574e6a4e50173`. Assembly establishes the following storage, without inventing a C++ class layout:

| Offset | Proven initialization / later meaning |
| --- | --- |
| +00 | AD55C0 does not write it; no vtable or reference count is inferred. |
| +04 / +08 | 18h tree head and tree size. **+04 is not a Font reference count.** |
| +0C / +10 | Actual 8-byte narrow name header, zeroed then conditionally resized/copied. |
| +14 | WORD zero only after successful name work; AD4C30 later writes scaled height. +16/+17 are not initialized by AD55C0. |
| +18 / +1C | The two supplied float bit patterns, scale ratio and alpha-texture scale. |
| +20..+44 | Ten explicit DWORD zeros, five 8-byte saved-path headers. |
| +48 | The supplied BYTE, written near return; +49..+4B are not initialized here. |
| +4C / +6C / +8C | Existing AD4C30/FontData evidence identifies the three embedded 20h glyph payload regions; AD55C0 does not initialize them. |

AD4C30 captures its receiver in EBP at AD4C58. At AD4F3A it sign-extends the stream WORD; CVTSI2SS/MULSS with CURRENT Font+18/CVTTSS2SI follow, then AD4F4A stores DX to that SAME Font+14. This is the lowword subsequently sign-extended by ABA8D0. The preserved loader footprint is `[AD4C30,AD5181)`, 1361 bytes/469 starts; only its receiver/height slice was audited here.

The constructor's name work can fail before +14/+48 and embedded glyphs are ready; the registry append precedes loading. A raw consumer needs the actually completed, retained Font graph. This packet does not recover the constructor/registry/loader EH composition or claim that an early appended entry is ready. Native Font retirement AD53A0's raw string/tree/sentinel tail remains separate from the accepted Source resource-owner fragment.

## What current Source genuinely provides

| Current provider | Established correspondence | Boundary relevant to raw ABA8D0 |
| --- | --- | --- |
| `FontRegistryStartup` / `FontRegistryOwnedFont` | Each owned descriptor has a stable address across appends; real Lua metadata and per-entry resource loading. Its native-resource arm checks the SAME live descriptor association. | C++ vector/unique_ptr/list projection; compatibility descriptors are copies. Native 10h registry/list and raw AC-byte Font storage are not supplied by these types. |
| `GameNativeFontApplication` / normal `GameFontHost` | SAME renderer/VFS/string/image domain; real renderer64 B319B0 image acquisition, DAT decoding, one retained operation per entry, canonical publication. Host postchecks descriptor and published-resource identities, consumed input slots and completion. | Successful initial producer, not a raw AD4C30 object/stream/tree/SEH implementation. Native per-field timing and AD51D0 destructive reload are explicitly outside it. |
| `NativeFontResources` / `NativeFontResourceOwners` | Own the SAME unrelocated `FontData` allocation and two transferred actual image references. Descriptor identity selects exactly one live owner; no name-based/default fallback. `signed_height_14()` sign-extends the CURRENT sole `scaled_height` lowword. | Descriptor reference, owner metadata and separate unique data allocation are not physical raw Font fields. Glyph map and embedded records have a typed C++ layout. Actual image +04 counts do not create a Font +04 count. |
| `set_gui_text_font_name_00ab8c30` | Store changed name, selected registry getter, LIVE name lookup, validate SAME descriptor/resource/actual-image domain, publish descriptor, read its current alpha scale, invalidate actual shader. | `GuiTextLifetime::text_` owns a `GuiTextWidget` whose `font` is `const FontDescriptor*`. This is an explicit projection, not an assignment to a Native raw Font field. |
| Source content helper | Reads the descriptor's `uppercase_only` after earlier section work. Late reload of `text.font` resolves CURRENT owner and its current signed height after material work. Existing 52/61 comparison service remains at equality after earlier callbacks. | Correct supplied Source semantics do not imply raw header, BYTE representation, pointer ABI, original CRT or EH placement equivalence. |
| Actual raw Text / existing graphics providers | Raw AB9650 genuinely writes Text+108=0; raw sections, glyph children, cursor and retirement already exist. Real font association, glyph selection, AB8400 buffers, native renderer and stream/layout services are present. | Raw Text default construction does not turn the separate Source `text_.font` member into the same raw operand. These existing lower services must not be described as absent or replaced by default callbacks. |

Source startup deliberately appends a stable row before resource loading, with a null native binding until completion; typed Lua conversion and failure ownership differ from Native. Factory binding additionally requires the SAME resource collection and SAME registry before any Text allocation. Source lifetime contracts require retiring Text borrowers before fonts and keeping registry, VFS, renderer, materials and failed operations through the shared drain. They prove a real resource graph, not original list/pool/private-CRT object ABI. A retained descriptor reference alone does not retain every provider.

The concrete raw prerequisite remains an actually initialized AC-byte Font allocation, real tree/name/data/image authority, publication of that SAME allocation through the matching raw registry lookup, and retirement consistent with those allocators and all borrowers. No cast, copied metrics, new facade or parallel default Font can establish it.

## Smallest bounded next Source packet

Whole **AD42C0** is a ready prerequisite: `[AD42C0,AD42F7)`, **55 bytes/17 starts**, SHA256 `941effb874f57a5fe53446c2d9672ccc73ddd763021ba52d0879fa4aeb80a9ef`. It has no incoming arguments, returns the allocation in EAX with plain RET, calls BF681B(18h), conditionally zeros pointer DWORDs at +00/+04/+08 in order, then writes BYTE+14=1 and BYTE+15=0. No local EH, publication, registration, count or name operation occurs; +0C..+13/+16..+17 remain untouched. The Source owning allocator either returns genuine nonnull storage or throws; Native null/invalid-address paths are not a valid supplied allocation domain.

Existing `font_descriptor_construct_00ad55c0` and `tree_construct_00ad5590` in `terrain_particles_cc13.cpp` already write raw offsets, so they must not be duplicated or mislabeled typed FontDescriptor bodies. They currently require `FoliageLoadHost::allocate_tree_head_00ad42c0` and `TerrainHost::native_string_assign_0041dd40`. The latter projects name assignment without Native constructor cleanup/capture proof. Exact ledger lookup and inspection of those implementations establish the concrete dependency boundary; grep alone is not an absence proof.

The next leaf packet can add `allocate_native_font_tree_head_00ad42c0()` in new `include/bsp/native_font_storage.hpp` / `src/native_font_storage.cpp`, using the existing owning Source `singleton_lifetime_allocate({object,0x18,0x18})` and its matching free domain. Preserve the three conditional stores and two BYTE writes; expose no default/factory/provider callback and do not return a semantic object. This is the established Source BF681B/private-CRT boundary, not Original heap/new-handler/exception ABI. Root would register the TU. AD55C0, AC3910/AC3570/AB8C30 raw composition, whole raw loading and retirement still require separate reviewed work after that leaf.

The leaf would close one real allocator prerequisite only. It would not make raw ABA8D0/ABB1D0 ready, add Font ownership to Text, or establish normal activation readiness. No new Source implementation is included in 141.

## Validation and limits

Fresh selected live spans equal disk; all selected instruction/receiver assertions pass; 13 exact direct-call rows are checked by the existing report verifier. Frozen Source bytes match the audited checkout and baseline Git blobs are separately pinned, with no packet Source mutation. The 136/139 manifests remain pinned. There was no build/test/probe and no new body execution, full producer EH audit, Native ABI compatibility, game validation or production activation proof.

The report and private sealed manifest carry exact hashes and copies. Only this document and its JSON receipt are committed; the file-only lease is released after commit and the worktree is held for Root capture.
