# Raw system-registry constructor

`construct_native_system_constant_registry_00b5bf70(fresh, context)` now constructs the actual 10h `D62A3C` owner through the application's actual singleton manager and string pool. It uses fixed local name/record/state storage and runs the recovered constructor cleanup before a source C++ exception escapes. No failed `NativeSystemConstantRegistryOperation` or lifetime-binding guard retains the owner. The parent still owns its allocation and subsequent free.

This is a new raw-context overload. The existing binding/operation interfaces retain their previous contracts and implementations. Their failed-operation guards are not cleared, reinterpreted or bypassed by this path.

## Caller contract

```cpp
NativeStringRawPoolContext strings{actual_01090aa8, actual_01090aa4, actual_01090aa0};
NativeSystemConstantRegistryRawContext registry{actual_0108fe94, strings};
auto* owner = construct_native_system_constant_registry_00b5bf70(fresh16, registry);
```

The caller supplies fresh, DWORD-aligned, writable 10h storage from the matching allocation/free domain. The context borrows the application's actual mutable cells; it owns no domain, provider or allocation. Keep these references valid throughout normal execution and cleanup. The function supplies its own fixed 8h name and 20h record scratch. It does not require a caller-owned continuation frame. Provider calls must leave valid readable native storage and array extents; arbitrary invalid extents, stale memory, hardware faults and concurrent mutation are outside this C++ interface.

Successful construction leaves the same owner published at current `0108FE94`, registered in the manager obtained from actual `01090AA0`, with 52 actual 20h records and capacity 64. The implementation includes the existing `shader_system_registry.inc` descriptors once. Strings resolve the current actual `01090AA8` pool before each allocation/return, including current `01090AA4` return gating; no `NativeStringStorage::release noexcept` adapter intervenes.

Failure does not imply rollback. In particular, a failing base constructor can leave publication and registration referring to the owner even though the native parent next frees it. Unpublished reserve buffers and uncounted partial row strings can leak. The raw path preserves those native effects.

## Pinned native basis and parent gate

The input is exactly evidence commit `4c73a48882c1307fe6078840f878035a51e0c132`, based on `0a7c5c7035e01af98bc9136db86c5817a9dca690`. Its report preserves all 106 states, 210 state writes, 52 literal intervals, six child FH3 maps and 20 live-Ghidra/PE-equal spans totaling 12,708 bytes. Those inputs were copied into this packet's immutable local manifest; they were not regenerated from a partial decompile. Project/program identity is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. No worker Ghidra writes were performed.

The original parent bytes were checked separately: `B32833` allocates 10h, `B3283B` captures EAX, `B32841` arms state 27, and `B3284D` calls B5BF70 with ECX equal to the captured allocation. `CBDED9..CBDEE6` loads `[FH3 EBP-460h]` and directly calls `BF65AC`. There is no registry destructor or operation retirement in that parent funclet. The raw overload therefore releases only the constructor's armed children and never frees its supplied owner.

| Native entry | Full native body | Raw source composition |
|---|---|---|
| B5BF70 | B5BF70..B5DEFB, 8,076 bytes | Base, current array, existing 52 descriptors, fixed state frame |
| B5B9E0 | B5B9E0..B5BA70, 145 bytes | Actual publication/registration; guard/root child cleanup |
| B5BA80 | B5BA80..B5BB18, 153 bytes | Unregister current publication, clear it, captured guard, root profile |
| B5BBC0 | B5BBC0..B5BC5C, 157 bytes | Actual record writes and independently armed partial string cleanup |
| B5BD10 | B5BD10..B5BE06, 247 bytes | Reserve/copy forward, release old strings forward, free then publish |
| B5BED0 | B5BED0..B5BF3E, 111 bytes | Grow at equality, copy current destination, increment only after return |
| B5BF50 | B5BF50..B5BF66, 23 bytes | B5BE10 zero-count specialization, then current data free |
| B38310 | B38310..B38381, 114 bytes | Shared record-copy implementation, new actual-pool overload, no local EH |

The original constructor ABI is ECX=fresh owner, EAX=same owner, plain RET. Child base entries and B5BF50 also use ECX with plain RET. B5BBC0 takes name/id/second/first/array on the stack and RET14h; B5BD10/B5BED0/B38310 each take their one native stack argument and RET4. Source overloads add explicit context references and are not binary replacements.

## Cleanup states and disarming

With A equal to the native aligned FH3 establisher EBP, the captured owner is `[A-3Ch]`, the name header is A-38h, and the record is A-30h. Body EBP later holds owner+4 and must not be mistaken for that establisher. Handler CC1113 uses FuncInfo DF9AB4 and map DF9AD8.

| Source state | Native funclet and receiver | Next state |
|---:|---|---:|
| -1 | None; base has not returned | -1 |
| 0 | CC0DC0 -> B5BA80(captured owner) | -1 |
| 1 | CC0DC8 -> B5BF50(captured owner+4) | 0 |
| 2+2n | CC0DD3+10h*n -> 41DD20(current name header) | 1 |
| 3+2n | CC0DDB+10h*n -> B34CC0(current record) | 2+2n |

The fixed source frame retains those numerical states for n=0..51. It arms state 0 after the base returns, initializes the array, then arms state 1. Each name resize and literal copy runs under state 1. The even state is armed only after both return; the odd state is armed only after the record constructor returns. Before normal record release it lowers to the even state; before normal name release it lowers to state 1. Normal exit unlinks source cleanup while retaining the final state 1 value, matching the native unlink schedule.

Normal name release uses the captured post-resize data pointer and the current name length+1. Name unwind reads both current header fields. Record cleanup reads its current string header. Neither release clears the header. Array cleanup decrements the current count first and then reloads the current row, working backward; it finally frees the current data pointer and preserves the stale data/capacity fields.

B5BBC0 arms its own state 0 at B5BBF2 after zeroing the record string. It cleans that current string if copying fails, while the outer constructor still owns only its name. The shared B38310 copy has no child cleanup.

B5BD10 and B5BED0's placement funclets call 401130, whose complete body is one RET. The source adds no allocation or row rollback. Failed reserve copies can retain the fresh buffer and copied strings while the outer frame destroys only the old published array. A failed append copy remains outside the current count and is excluded from array string destruction. The source omits only the no-op funclets' unobservable argument computations in the supported readable-storage domain.

B5B9E0/B5BA80 use the actual first manager's section at +10 and its depth at section+18. Registration/removal uses a second manager getter and the current publication. BCFCA0 clears the first matching slot without reducing the manager's array length. Child state 1 releases the captured first guard; state 0 stamps CE3818. A failed B5B9E0 does not unregister or clear publication, and the outer constructor remains at -1. A failed B5BA80 adds only its own guard/root cleanup; it does not invent a missing removal or clear.

The RAII cleanup paths are `noexcept`: a second C++ exception during exception cleanup terminates. Normal service/getter exceptions propagate into the correctly lowered source states. This packet does not reproduce original FH3 runtime stack layout, arbitrary spill aliases, native SEH/hardware-fault unwinding or original CRT throw identity; it does not claim that every injected fixture exception is reachable from the original returning provider.

## Successful destruction and terminal boundary

The raw B5BF50 and B5BA80 overloads compose successful array/base cleanup without an operation, and the fixture checks that schedule. They do not by themselves implement the complete B5DF00 destructor or B5DF70 scalar entry.

At the pinned base, `NativeSystemConstantRegistryLifetimeBinding::destroy_registered` admits a raw-created owner because no guard was installed. However, it creates a legacy `Operation`, whose B5DF00/B5DF70 route allocates an `ArrayOperation` and uses `NativeStringStorage`. A failure retains the legacy guard and terminates under that callback's `noexcept` boundary. It cannot retire a raw owner without an operation.

The actual `NativeSingletonDeletionBindings` / BD0400 finite profile map has no D62A3C/D626F4 registry route at this base. A complete raw terminal migration needs B5DF00, B5DF70, the D626F4/B5BB20 base scalar route where admitted, and explicit borrowed context/profile dispatch. Those addresses and shared dispatch files were not claimed or changed in this packet.

## Validation

The strict Win32 build and both existing CTests passed. One focused fixture passed seven cases: all 52 normal literals, failed base after registration, initial name-getter failure under state 1, child partial-string cleanup under outer state 2, failed reserve copy under state 7, and failures of the already-disarmed normal record and name returns. It checks actual profile/publication/manager slots, captured section depth, current array count, exact release sizes/order and native outstanding acquisitions. The reserve failure deliberately leaves one 128-byte unpublished buffer and one copied string; normal record/name return failures each leave one string. The fixture frees the supplied owner through the parent-compatible free domain without an operation, including before diagnostically clearing failed-base publication.

The machine-readable report contains source/library/fixture/tool hashes and actually loaded module paths. The fixture uses actual raw manager and pool bodies. Test-only entry hooks inject exceptions and record acquisitions/returns; production has no such hooks. Its partial-string case throws after a real resize returns to establish a concrete partial-header preimage for child cleanup. An initial fixture assertion incorrectly expected unregister to shrink the manager count; it was corrected to require a null slot with unchanged length after review of BCFCA0. No production change resulted from that assertion. This is source cleanup validation, not original FH3 execution, ABI parity or game validation.

## Integration clarification

B5BF50 is a resize-to-zero specialization for constructor-produced arrays with
nonnegative current count AND capacity. Its source does not implement the
general B5BE10 negative-count initialization or negative-capacity reserve paths.
The separately integrated NATIVE_SYSTEM_REGISTRY_RAW_TERMINAL packet now provides
B5DF00/B5DF70/B5BB20 and canonical raw singleton dispatch; original FH3/SEH,
private frame aliases and gameplay remain unvalidated.
