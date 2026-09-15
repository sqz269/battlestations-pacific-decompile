# Resource-instance ownership and the unit-part admission route (AP)

Addresses: `007137F0`, `00B891A0`, `0071AED0`, `0071ACD0`, `00B89F20`, `00B87930`, `00718B30`, `00718D00`, `00B89DB0`, `00B89FC0`, `00B89150`, `00B89980`, `00711080`, `00BD30E0`.

The concrete game-resource dispatch route now has a source implementation of its instance allocation, construction and terminal destruction. The selected-set release helper can resolve these source-owned profiles without dereferencing an original numeric vtable address. The executable still needs the graph-building caller and actual class/model publication before admitting populated raw parts.

## The recovered connection

Live table CFD8CC, also written by the established 74h game-resource constructor71B810, contains slot+8=7137F0 and slot+10=71AED0. The complete21-byte7137F0 wrapper forwards its two stack arguments through an x87 float load/store to B891A0. That builder calls the resource's slot+10, then builds the returned instance. Its full assembly is not reconstructed here.

71AED0 allocates **8Ch**, calls71ACD0 with the resource, and returns the instance. The constructed CFD8E0 table has slot0=BD30E0, slot4=718D00 and slot8=71B710. The instance has root+0C and five checked vectors at3C/4C/5C/6C/7C, matching the storage used by the unit-part collision/entry callers. This establishes a concrete game-resource route; it does not prove the live class+50 publication or every possible resource profile.

The selected-set terminal is therefore concrete for CFD8E0: **711080 -> BD30E0 -> 718D00(flags1) -> 718B30 -> B89DB0**. The base D63244 profile instead uses B89FC0. `native_resource_instance_selected_callbacks` supplies these two complete profile bindings. Other profiles require their own bindings.

## Implemented bodies

| Address | Inclusive end | Bytes | Native ABI and operation |
| --- | --- | ---: | --- |
| B87930 | B87966 | 55 | ECX ignored; EAX24h allocation; RET. Prepare links/color/nil, preserve payload. |
| B89F20 | B89FBD | 158 | ECX3Ch instance, stack resource, EAX same, RET4. Construct base and retain resource. |
| 71ACD0 | 71AD44 | 117 | ECX8Ch instance, stack resource, EAX same, RET4. Construct game instance. |
| 71AED0 | 71AF2F | 96 | ECX resource, EAX8Ch instance/null, RET. Allocate and construct. |
| B89150 | B8919F | 80 | ECX map, stack node, RET4. Clear tree nodes and owned pointer-vector storage. |
| B89DB0 | B89E8E | 223 | ECX instance, RET. Release resource/root, map and base vectors. |
| 718B30 | 718C1C | 237 | ECX game instance, RET. Listener and five vectors, then base. |
| B89FC0 | B89FDD | 30 | ECX base instance, stack flags, EAX original, RET4. Conditional physical deletion. |
| 718D00 | 718D1D | 30 | ECX game instance, stack flags, EAX original, RET4. Conditional physical deletion. |

The nine complete physical bodies total1,026 bytes. Descriptive names remain hypotheses, not recovered symbols. Source destructors add borrowed access in EDX; these are not binary entry replacements.

B89F20 writes CEB130/refcount1, D63244/resource8, two zeroed vector triplets, the actual34h tree head and38h count, then increments the **current** resource8 count. It leaves root0C and proxy words untouched. 71ACD0 adds CFD8E0 and five zeroed triplets, retaining all five proxies. **A newly constructed instance is not destructor-ready until the builder establishes root0C.** The implementation does not invent a null root or a populated graph.

718B30 captures root0C, clears a matching actual listener through the canonical renderer path, then frees/zeros vectors7C,6C,5C,4C,3C. B89DB0 captures resource8, decrements it and dispatches current slot0 only at zero, clears8 after return, then unlinks/releases current root0C. It destroys map30, vectors20/10 and the ref-counted base. Root0C remains a stale value. Neither destructor frees the instance allocation; the scalar wrappers test bit0 only after destruction.

B89150 follows right recursively, reads current vector data14, captures left before any free, frees data, zeroes14/18/1C, frees the node, then continues left. Its24h nodes contain a key atC and a checked vector at10; **the key meaning remains unclassified**. The map does not release the pointers in those vectors. The actual destructor supplies the same valid tree's complete begin/end range, establishing B89980's full-range branch. The private source specialization preserves that branch's head reload/reset order; other iterator ranges are not reconstructed.

Seven42-byte vector member specializations match canonical BD0220 instruction-for-instruction after normalizing branch addresses:4FDC20,B883A0,718A40,718A70,718AA0,718AD0,718B00. Their actual free and ordered zero stores are reused.

## Exceptions and dispatch domains

Five original FH3 maps and their actions are retained. Base-constructor allocation failure unwinds vectors20/10 and BD30F0; the game constructor's state stays-1. Factory state0 frees its captured allocation. Game destruction holds its six member actions live until entering the base. Base destruction owns map/vector/vector/base cleanup while resource dispatch can throw. No unwind retries resource release or root release after that callback fails.

711080 now accepts an optional pure, nonthrowing current-profile slot reader. Its existing process-resident table behavior is preserved when that reader is absent. Both domains decrement first, capture the current profile only at zero, dispatch the captured slot0 target, and clear the original cell after return. The new instance binding uses actual checked table identities and canonical BD30E0, whose slot4 dispatch captures the profile again. It executes source functions rather than original numeric code addresses.

External retained-resource terminal dispatch remains a required complete binding; this packet does not supply the still-open GameResource resource-zero destructor. The root binding uses the existing complete node lifetime. Native FH3/fault delivery, private native stack aliases and malformed/concurrently changing trees remain outside the source contract.

## Verification

Five paired original-byte cases compare constructors, base/game destruction, populated pointer-vector trees, scalar flags81h/2 and the selected-set terminal chain. Nonnull roots are real `NativePlainNodeReference` objects in the actual initialized raw node pool; their terminal path returns the slot and unregisters the lifetime. Actual CRT allocation preimages, free order/preimages, raw owner images, listener state and resource counts match. The fixture's retained resource has an explicitly fixture-owned terminal; it is not evidence of the game's resource terminal implementation.

Three source cases cover factory allocation failure, listener failure and retained-resource terminal failure. They verify the recovered cleanup states and allocation retention; original FH3 handlers remain guarded and unreached. The existing AO six-pair/one-unwind suite also passes with its previous normalized hashes, covering the default process-resident table domain. All comparisons repeat with identical hashes. No repository tests were added. Strict Win32 build, both existing CTests and eight native seed checks pass.

The reference includes B89980,711080 and BD30E0 in addition to the nine new bodies:1,282 original bytes match live Ghidra and the unchanged PE. Twenty-eight direct-call rows pass stored ownership checks; three calls in B89DB0's decoded tail are recorded separately. Ten listing gaps were decoded under the Ghidra write lock. **B89DB0's stored body still endsB89E32**, although its decoded physical tail reachesB89E8E. Script execution remains disabled; AP did not attempt or bypass that gate.

Evidence: `reports/native_model_admission_ap.json`, `reports/native_resource_instance_flow_ap.json`, and local retained inputs under `local/resource_instance_ap/`.

The next required work is B891A0's actual graph construction, 71B710's typed instance publication, complete external resource terminal ownership, and the live class+50 writer. The conditional null-model behavior of87BCC0 does not establish populated admission or gameplay parity.

## Integrated validation

Commit `0dfaba808cf21fd76108aa625f8d888e994ce992` passes the strict Win32 build and both existing CTests. All five paired resource-instance cases, three source failure-cleanup cases and the six-pair/one-unwind part-destruction regression pass against that library with repeat-stable normalized images. The 120-frame USN01 compatibility run passes finite-trajectory, stationary Airfield2, avoidance, generic-tick, participant, world-list and observer/pending-owner checks. The preserved executable SHA-256 is `0b3217ac7ff704229e3b516907f7bd07c9253786525c027adf3c047a5fad3476`. An immutable manifest retains 863 inputs, 223 artifacts and the union of 90 linked production objects for both probes, including compiler/header/library inputs, original bytes, exception maps, saved Ghidra receipts and mission artifacts. B89DB0 has a decoded physical tail outside its stored function body. Graph construction, typed publication, the external game-resource terminal, class+50 admission, native FH3 and gameplay remain open.

## Correction from NATIVE_MODEL_GRAPH_AQ.md

B89E90 supplies the item slot8 type token as the tree key and appends eight-byte `{node,item}` pairs to its mapped vector. AP's pointer-vector wording was incomplete about element width and pair order; its destructor free/zero behavior is unchanged. B891A0 sets root0C to the first constructed node, or null for an empty graph. See [NATIVE_MODEL_GRAPH_AQ.md](NATIVE_MODEL_GRAPH_AQ.md). AP's sealed proof remains tied to its original compiled inputs.
