# Whole actual raw Widget child lookup

Baseline is `cb6d945fa6607a6c3f55a9bd218d3a57c952bcd4`. This packet owns only `native_gui_widget_child_lookup_storage.hpp/.cpp`, this document and [its report](../reports/cc16_native_gui_widget_child_lookup_storage.json). Root additionally authorized and leased the finite B6D800 address primitive in the same files. Existing semantic/typed APIs remain unchanged; Original/vendor/helper/duplicate-body credit is zero.

Whole AA7E00..AA7EB0 is177 bytes/70 instructions. Native ECX is the captured actual Widget; stack homes are the actual query-header pointer and one unused DWORD; EAX returns the current child or0, RET8. **The second DWORD is never read and there is no recursion.** Only direct children are searched; the parent's name is never tested.

```cpp
void* find_native_gui_widget_child_storage_00aa7e00(void* captured_actual_parent,
    volatile std::uint32_t (&actual_homes)[2]);
```

This ordinary C++ interface borrows actual raw storage; it creates no Widget, list, node, NativeString, pool, default name, provider, context, record or authority. The caller supplies real two-DWORD homes; home0 is captured once at its native phase, home1 stays unread for every value. Query header/data are not preflighted or copied. Empty lists and node-null-only paths do not dereference the query header.

## Exact ring and name schedule

| Native sites | Source operation |
| --- | --- |
|7E00..0D | Capture initial Widget+68 head before reading home0; compute Widget+64 list identity, then load initialhead->next. |
|7E10..1E | CMP EBX,EBX sets ZF; still read CURRENT list+4 end before testing sibling equality. The AA7E17 invalid call is unreachable. No null-owner guard is invented. |
|7E24..35 | Fresh current-head guard, real SDK BF6713 if equal, then CURRENT sibling+8 payload and child+4C node nonnull gate. |
|7E37..47 | Another fresh head guard; after its callback reload payload+8 and current child+4C. Private B6D800 returns wrapped32-bit node+54, with no dereference or typed-node cast. |
|7E4C..7F | Query length FIRST, child-name length SECOND. Unequal skips; bothzero match without data reads. Otherwise query data FIRST, child data SECOND; owning-SDK `_stricmp(child,query)`, resultzero matches. |
|7E94..9E | Fresh match guard, then return CURRENT sibling+8 after any handler mutation. Do not return the earlier tested child snapshot. |
|7E83..8F | Fresh advance guard, then CURRENT sibling->next; reload end on the next visit. No count, visited set, iteration bound, restart or retain is added. |

Four BF6713 calls are reachable. Each may return or throw through the existing genuine SDK transport; the Source does not assume no-return, recheck after a returned handler, or add recovery. Captured list/sibling/query identities match Native nonvolatile-register roles; reached head/payload/next/header/data reloads observe valid callback mutations. Home0 mutation does not replace the already captured query pointer.

Private B6D800 is exactly `LEA EAX,[ECX+54h]; RET` (4 bytes/2 instructions). Its existing typed `NativeNodeStorage& -> NativeString&` interface is unchanged and is not supplied by reinterpreting raw pointers. The authorized private arithmetic primitive adds no helper/body credit.

The existing raw `equal_native_string_headers_00435c40(left,right)` correctly reads left length then right length, but right data then left data and compares(left,right). Using(child,query) would reverse AA7E00's required length reads; swapping arguments would reverse its required data reads and comparer arguments. Its Native39-op/89-byte reference and Source are pinned unchanged. This body therefore preserves the exact inlined raw comparison and reuses the SAME concrete owning-SDK `_stricmp`; no replacement comparator/default/provider is added. Redundant Native zero tests are resolved only from already equal captured DWORD lengths, without restricting their domain.

## Actual consumer and lifetime contract

The raw graph is the SAME Page/default-Type2 Group and WidgetChildren domain already produced by `GameNativeGuiPageApplication`, `NativeGuiWidgetPropertyChildrenRawContext` and actual widget base/child-link/lifetime services. The list allocator address is Widget+64, current head+68; count+6C is never read. Ring entries have next+0 and payload+8. Eligible child's current scene node is+4C; its actual narrow8B name header is+54. No `GuiLayoutWidget`, logical `GuiWidgetOwner` or typed-node conversion supplies this function.

`NativeFrontEndFrameOperationsBindings`, `NativeFrontEndFrameTitlePositionBindings` and `NativeGuiGridLookupCalls` can directly adapt their existing actual-parent/header/unused operands into real two-DWORD homes and invoke this entry. Keep the captured actual receiver and actual header; do not resolve a newer Page/Frame publication or add another shared owner/context. Those application bindings are not implemented in this packet and remain explicit integration work.

Caller serialization keeps every reached/captured parent, initial/current head, sibling, payload, scene-node and name/header/data backing valid through SDK callbacks and return/escape. Current global publication may change while the captured receiver stays alive. The returned current child is an unretained borrow from that existing graph. This lookup neither changes native reference counts nor creates longer-lived acquisition records. No local Native allocation/ownership requires cleanup; Source exceptions propagate directly with no new catch, rollback, retry, terminal policy or automatic action. Existing application records/owners govern the surrounding operation and failure lifetime.

## Evidence and qualification

The entire saved70-op listing and39-op comparison reference were read. Read-only `bsp.py ghidra count` used `Client.verify` for configured `C:/Users/sqz269/bsp.gpr`, project `bsp`, program `/battlestationspacific.exe`; live/snapshot counts were77632. No Ghidra mutation occurred. Independent installed-PE audit pins the supported SHA,177-byte body,4-byte getter,89-byte comparison reference, all seven static E8 targets and exact dead-guard CMP/MOV/JZ bytes. Six shared exports are pinned as raw bytes;28 unchanged Source/tool inputs and both new Source files use explicit Git-LF pins.

Static Source checks cover once-only home0/unusedhome1, five distinct current-head read sites/four reachable guards, initial/head/length/data/payload/next order, absence of count/recursion/snapshots/typed casts and absence of added owner/failure policy. No new tests, compiler/build, probe, Native/game process, process control, CMake/ledger edit or activation occurred. Root owns common MSVC Win32 compilation; prior Source081 or other cohort builds do not validate this new Source.

Owning-SDK `_stricmp` and existing BF6713 preserve their selected Source CRT/locale/error/handler policy. Original BF7FBF/BF66EF encoded globals, debugger/Watson behavior, private stack/public aliases, faults, register ABI/RET8 identity, SEH and runtime/game equivalence remain unproved. There are no FP operations or FP environment changes in this Source body. Actual caller/graph binding, Page/full frontend readiness, whole ABA8D0 and changed ABAED0 remain false.
