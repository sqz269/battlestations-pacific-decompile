# Native node animator lifetime and sample finalization

Addresses: 00b780d0, 00b786e0, 00b78720, 00b77990, 00b75ee0, 00b92610, 00b77530, 00b76790, 00b778c0, 00b79bc0, 00b79d44, 00b7a0ba

The resource postprocessor's two animator families now have complete source finalizers and physical destructor bodies, with the two inlined object-initialization fragments preserved separately. This closes the lifetime and slot20 dependencies identified in [AT](NATIVE_RESOURCE_INSTANCE_POSTPROCESS_AT.md), using the [AU registry](NATIVE_ANIMATION_REGISTRY_AU.md) and existing borrowed-pointer storage. The [AV skin finalizer](NATIVE_SKIN_MODEL_BINDINGS_AV.md) is also implemented. The full B79BC0 postprocessor and populated graph admission remain open.

Source: `native_node_animator_lifetime.hpp/.cpp`. Evidence: `reports/native_node_animator_lifetime_aw.json`. Descriptive names are hypotheses, not recovered symbols. New source context parameters change destructor ABIs; native FH3/SEH and gameplay are not proved.

## Objects and dispatch

The ordinary animator is 30h bytes with profile D62E60; the optimized animator is 38h bytes with profile D62EB0. Both contain reference count4, translation8/C/10, angles14/18/1C, borrowed track-pointer array20/24/28 and retained registry2C. Optimized adds byte30 and borrowed compact-track item34, leaving padding31..33 untouched.

| Native profile | Slot0 | Slot4 | Slot20 |
| --- | --- | --- | --- |
| D62E60 | BD30E0 | B786E0 | B77990 |
| D62EB0 | BD30E0 | B78720 | B75EE0 |
| D62EF4 registry | BD30E0 | B79BA0 | Not used here |

The table bytes were checked live against the installed PE. Slot0 dispatches through the current slot4 deleting wrapper; a source lifetime provider must resolve the same live table and execute the complete target. The source destructor borrows the existing `NativeResourceAnimatorLifetime` protocol. Other animator slots, including evaluation/update services, are not implemented or admitted by this batch.

## Initialization fragments

`B79D44..B79DA9` is the optimized object-write fragment inside B79BC0. It writes CEB130/ref1, zero track header and registry, zero six transform floats, then publishes D62EB0, writes byte30 according to whether the earlier type predicate produced a nonnull compatible skin node, and clears borrowed item34. The original caller allocates **before** running that type predicate. Source initialization receives its result and preserves that boundary; it does not perform allocation or type lookup.

`B7A0BA..B7A116` is the ordinary fragment: CEB130/ref1, D62E60, zero header/registry and transforms. Unlike the optimized fragment, its derived profile is published before the remaining fields. The source functions reproduce object writes only; original caller scratch-vector stores in these spans remain part of the full caller. They are **not standalone recovered constructors**, nor a claim that B79BC0 is reconstructed.

Three native fragment wrappers compare all 38h output bytes, including ordinary trailing poison and both optimized predicate outcomes. The wrappers preserve the original bytes and provide scratch stack space for the caller-local stores. No Ghidra function is created at either interior fragment address.

## Destruction and pointer arrays

`B780D0` is ECX animator/plain RET. It stamps D62E60, captures registry2C, decrements its reference count and invokes its current slot0 only at zero, then clears animator2C. Callback changes to that field are overwritten by the clear. It then resizes the **current** borrowed track array to zero and frees its current data, stamps D5C104 and invokes the canonical BD30F0 base destructor, ending with CEB130. Pointer/capacity remain as the native dangling values; borrowed tracks and compact item34 are not destroyed.

`B786E0` and `B78720` are ECX self/stack flags/EAX original self/RET4 deleting wrappers. Both destroy payload and free self only when bit0 is set. The optimized wrapper first stamps D62EB0, then enters the common destructor which stamps D62E60. The source adds a borrowed lifetime context and does not recreate native FH3 registration/unwind funclets.

The complete B76790 reserve and B77530 resize bodies match canonical B1C500 and B1C770 after direct-call normalization, with the reserve targets equal and the resize's single specialized reserve target resolving to that same verified alias. Reuse those existing functions on the actual `NativeRenderPointerArrayStorage` at animator+20. Reserve copies borrowed identities and publishes after free; resize zeroes new cells and does not release pointed tracks. No second pointer-container algorithm is added.

## Sample finalization and arithmetic

`B778C0` is ECX registry, stacked signed index/raw float bits, RET8. It grows the registry sample array at14 to index+1 if required, x87-copies the old sample to a float temporary, compares with FCOMIP and writes the selected bits with MOVSS. Incoming wins for equality or unordered comparisons. An incoming signaling-NaN bit pattern can remain signaling in the selected stored payload even though comparison raises the native invalid flag. Replacing this with a host maximum operation changes edge behavior.

`B77990`, ECX animator/plain RET, walks the current signed track count and current backing. For each nonnull track it captures registry2C and track+1C **before** potential sample-array growth, then performs the native old-sample x87 spill/comparison/write sequence at that ordinal index. Changes made during allocation affect later iterations; they do not replace the already captured registry or incoming value for the current iteration.

`B92610`, ECX compact item/stack registry/RET4, walks current item+C backing/current item+10 signed count, stride18h. It x87-copies each record's scalar at0 into the call argument and invokes B778C0 by ordinal index. That argument copy can quiet a signaling NaN before the maximum helper runs. `B75EE0`, ECX optimized animator/plain RET, delegates to B92610 when current item34 is nonnull, using current registry2C. Byte30 does not gate finalization.

All four bodies use their complete reassembled instruction schedules. The compiled bodies match original bytes after call displacement normalization. The finalizers' call targets are checked independently; both resize calls reach the same ECX/stack/RET4 adapter, whose complete body calls AU's `resize_native_float_array_00818030`. It borrows the same actual sample header. No raw pseudocode, invented globals or unresolved successful stubs are compiled.

## Saved analysis and validation limits

The project/program are `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. Fourteen reference bodies total1101 bytes; two constructor fragments total195 bytes. All bytes and relevant table words match live Ghidra and the unchanged installed PE. The supported locked definition tool created B75EE0 from its complete19-byte body and saved the program. Two deleting-wrapper repairs restored six bytes, and the pointer reserve repair restored nine bytes; those stored bodies have no remaining call gaps.

The complete physical destructor is B780D0..B78161 (146 bytes). Flow repair decoded its41-byte post-free tail and saved it, but the stored function still ends at B78138. Therefore mechanical verification reports **20 calls checked, one failed**: B7814C's BD30F0 call is in no stored Ghidra function. The other19 rows pass. The failed row remains in the report; physical byte/source coverage does not establish Ghidra body ownership. No function deletion/recreation or global no-return change was used.

Strict MSVC Win32 and both existing CTests pass. The ignored probe provides25 paired records: three constructor object images, six complete lifetimes (including null item, flags0, callback replacement of current storage and producer backing/count mutation), and16 exceptional-float maximum variants. It compares actual allocations, free preimages/order, registry and pool state, ownership callbacks, output words and x87/SSE status; repeated processes produce identical images. Real CRT allocation/free and OS interlocked operations are used. Registry callbacks invoke canonical BD30E0 and complete AU registry destruction over the real string-pool provider. A separate source exception after reference decrement preserves the recorded partial state; it is not a native FH3 test.

Complete virtual evaluation services, native exception ABI, remaining temporary arrays/B630F0/camera sequencing, full B79BC0, B891A0 populated admission and gameplay remain required. No worker was dispatched and no repository test suite was added.
