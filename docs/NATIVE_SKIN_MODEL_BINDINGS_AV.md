# Native skin binding storage and recursive finalization

Addresses: 00b91000, 00b91590, 00b91460, 00b90c30, 00b64db0, 00b75d80, 00b76570, 00b90cc0, 00b90ea0, 00413920, 00b632d0, 00b79bc0

This batch implements the complete skin-record storage, binding setter and recursive finalizer required by the skin phase of `B79BC0`, analyzed in [AT](NATIVE_RESOURCE_INSTANCE_POSTPROCESS_AT.md). The [AU registry](NATIVE_ANIMATION_REGISTRY_AU.md) is also now implemented. The full postprocessor and `B891A0` populated graph admission remain required; this batch supplies no placeholder caller or gameplay claim.

Source: `include/bsp/native_skin_model_bindings.hpp` and `src/native_skin_model_bindings.cpp`. Evidence: `reports/native_skin_model_bindings_av.json`. Names are descriptive hypotheses, not recovered symbols. Storage/lifetime functions use added source context parameters; they are not drop-in ABI replacements.

## Storage and lifetime

The actual model header at `184/188/18C` contains backing, signed count and signed capacity. Its records are **60h bytes**, unlike the ordinary model's four-byte bone-pointer array at the same offset. Each record contains retained node at 0, translation float3 at 4, angles float3 at 10, scalar bits at 1C and a 64-byte matrix at 20. The postprocessor's five arguments to B90C30 establish this layout, and B91460 establishes its initialization behavior.

| Entry | Original ABI and complete behavior |
| --- | --- |
| B90EA0 | ECX header, signed stacked capacity, RET4. Clamp to at least one, allocate `capacity*60h` with DWORD arithmetic, copy each current record, release old records ascending, free current backing, then publish new backing/capacity. |
| B91460 | ECX header, signed stacked count, RET4. Reserve if needed, initialize grown records, or decrement the current count before each descending release; finally publish requested count. |
| B91590 | ECX model; add 184h and tail-jump to B91460 with the same stacked count. |
| B90C30 | ECX model, stack index/node/translation/angles/scalar, RET14h. Same node skips ownership changes. Otherwise publish incoming, retain it, release old, then reacquire current model backing and write transform fields. |

Reserve copies the node word **without incrementing its reference count**, then performs seven x87 float copies and REP MOVSD16 for the matrix. It rereads current count/backing on successive copy/release iterations. After each old-node release, it clears the captured old record, even if the callback changes the current header. This observed behavior is retained; no balancing retain is invented. A caller growing populated storage must respect the original lifetime contract.

Resize initializes six float fields to positive zero, then node to null, then matrix to identity. **The scalar at 1C remains untouched.** Growth captures the initial count and remaining iteration count; backing stays live. Shrink releases nodes from the current decremented count, permitting callbacks to change the next index/count. The current table is resolved only when InterlockedDecrement returns zero. The source borrows the existing `NativeResourceAnimatorLifetime` slot-zero protocol through `NativeSkinNodeLifetime`; callbacks receive actual nodes. They must execute complete target behavior, not a successful no-op or copied owner.

B90C30 publishes/retains before releasing old, even when that release replaces model backing. Its two float3 fields use ordered x87 FLD/FSTP; the scalar uses raw MOVSS bit transport and does not gain an x87 conversion. New source scalar input is a DWORD for that reason. No exception rollback beyond the observed behavior is added.

## Matrix and hierarchy path

`B75D80` and `B76570` are complete 24-byte ECX animator/stack float3/RET4 setters for offsets 8 and 14 respectively. The source preserves their instruction bytes and x87 alias/status effects. There is no null animator check in their caller.

`B64DB0` takes ECX destination, EDX translation, stacked angles and a second stacked pointer, returns EAX destination and RET8. **The second stack pointer is never read.** Its original calls pass an all-ones vector, but the body does not apply scale from that vector. It stages FSIN then FCOS through single-precision spills for all three angles, forms three matrices using SSE subtraction from immutable negative zero, performs the original two `413920` multiplications, transports current translation words and calls general inverse `B632D0`.

The private arithmetic schedule is reconstructed as typed MSVC Win32 assembly. Calls reuse `multiply_native_camera_matrices_00413920` and `invert_native_camera_matrix_00b632d0`, whose full raw implementations already exist. Only their call displacements and immutable D7A208 (`80000000`) / D7A24C (`3F800000`) addresses relocate. The compiled B64DB0 body matches original bytes after these operands are verified and normalized. This is not a host Euler formula, a conventional scale operation or an approximate inversion.

`B91000` is ECX model/plain RET. First it enumerates current records; every nonnull node receives translation and angles in its current animator. It reloads backing for the angles and reloads node+130 for the second setter. Second, for every record whose node has parent+30 equal to the model, it constructs the inverse pose, multiplies identity by it, writes the 16 resulting floats through ordered x87 copies, and invokes B90CC0 with that node and record index.

`B90CC0` is ECX model, stacked parent node and parent index, RET8. It walks parent child+34 and sibling+3C. For each child, it captures current model count/backing and selects the first record with that node identity. A match uses the parent record's matrix as left operand and the child's inverse pose as right operand, stores the result at child record+20 and recurses. **Unmatched children are not recursed through.** Duplicate records use the first match; there is no cycle guard, hierarchy repair or missing-node fallback. The postprocessor must provide its original valid storage and populated animator dependencies.

## Ghidra and verification

The batch verifies `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Eleven original spans total 5188 bytes, including both canonical matrix bodies, and agree with live Ghidra and the unchanged installed executable. The flow tool repaired 19 bytes after B90F8D's free call, restoring B90EA0's backing/capacity publication; its stored body is complete and no call gaps remain. The report contains the locked repair receipt. All 14 direct/tail call rows pass mechanical verification. The branch-skipped alignment bytes in B91000 are not fall-through code.

The strict MSVC Win32 build and both existing CTests pass. One ignored probe compares five storage/lifetime/finalizer scenarios and 24 inverse-pose numerical variants against relocated original bodies. The scenarios cover grow/shrink, same-node assignment, untouched scalar poison, callback changes to backing/count, empty and populated finalization, unmatched ancestors and duplicate records. Compare actual model/node/animator/allocation images, allocation/free preimages and order, destructor events and x87/SSE status. The math variants include signed zero, denormals, finite values, infinities, quiet/signaling NaNs, DAZ/FTZ modes and destination aliases with translation/angles. B64DB0 has verified normalized instruction identity; both float3 setters have literal byte identity. Results repeat across processes.

The reference path uses actual OS interlocked operations and the same actual CRT allocation/free boundary. Its node destructors are concrete fixture services that mutate the original storage and record current ownership; this does not prove complete game-node destruction. One source allocation failure preserves the original array header and existing allocation. Native FH3/SEH, actual virtual lifetime integration, the remaining animator factories/temp vectors/B630F0/camera sequence, full B79BC0 and populated graph/gameplay validation remain open. No new worker or repository test suite was added.
