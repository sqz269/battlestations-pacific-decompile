# CC12 raw layer property readiness 75

This is read-only readiness evidence, with no new Source body, test, runtime, or ABI credit. The verified complete AC4C50 body has entry `00AC4C50`, exclusive end `00AC4D28`: **216 bytes, 66 instructions**. The earlier provisional 214-byte bound cut into `RET4` at AC4D25. Live bytes at AC4D20 are `5F 5E 83 C4 10 C2 04 00`, matching the unchanged installed Original. Native ABI is ECX actual layer/page, one stacked visitor pointer, `RET4`; no return value is specified.

## Exact reader and mutation order

Each visitor call receives the same three contiguous8h pairs, 24 bytes total, and reloads the current visitor profile/slot+0C. For the real CE44FC Lua-reader profile, +0C is BD68D0; the dynamic call cannot be replaced by that target unconditionally.

| Read | Key pair | Field pair | Fallback pair |
| --- | --- | --- | --- |
| Priority, AC4C8B | `{0, actual D5CB00}` | `{1, address of local DWORD}` | `{1, 0}` |
| RenderOrder, AC4CDD | `{0, actual D5CAF4}` | `{2, actual page+11C}` | `{2, +0.0f bits}` |

The first local DWORD has a preimage; the caller does not initialize it before the reader. After that reader returns, AC4C8D captures it once, then writes page+FC at AC4C94 and page+118 at AC4C9A from the same captured value. RenderOrder is written directly through its field pair. No later priority synchronization repairs changes made by the second callback.

Only after RenderOrder returns does AC4CDF reload the current page profile and +78 target; AC4CE6 calls that exact target on the current page. The layer's own table is D5BE38 (+18 points to AC4C50), and its own +78 cell D5BEB0 points to **AC59A0 scene/camera-store acquisition**, not an empty hook. Other current profiles require their genuine target implementations.

After +78 returns, AC4CE8 captures current page+EC scene **before** AC4CEF reads current page+4C node, then AC4CF2 calls B6D890. Zero XYZ words are written only after root propagation returns; AC4D13 calls AA7DC0 with their actual address. Finally AC4D1B calls **whole** AAA710 with the same visitor; it is a call followed by the layer epilogue, not an omitted or optional continuation. The old semantic `before_properties` comment's AC4C9E hook address is not this actual call site.

## Real providers already present

| Contract | Existing Source provider | Remaining binding obligation |
| --- | --- | --- |
| Visitor current+0C | `NativeGuiWidgetPropertyDispatch`; BD68D0 `read_native_lua_field_or_default_00bd68d0` | Exact current target, actual reader/Lua state, initialized lookup/StoreValue scratch, same raw string domain and live resolver/SSE cells. Normal-return domain only; no default/no-op dispatcher. |
| Layer's own current+78 | `acquire_native_gui_scene_storage_00ac59a0` | Real `NativeGuiSceneAcquisitionContext`, persistent frame/acquired graph, actual pools/types/manager/scene/string domains. The full raw body exists; this does not compose a normal game host or authorize substituting AC59A0 for a changed target. |
| B6D890 | `propagate_native_node_root_storage_00b6d890` | Actual node and captured outer24h scene, genuine `NativeNodeSceneChildDispatch` and stable reached attachment frames. The typed GUI scene helper is a different interface. |
| AA7DC0 | `set_native_gui_widget_local_position_00aa7dc0` | Existing transform scratch/bindings, bounds scratch and actual D7A280 half cell. It captures all XYZ DWORDs before stores, then recomposes actual transform and bounds; no logical widget substitution. |
| Reader children services | BD5F50 key enumeration, BD8E20 enter, BD7A20 leave; raw list allocation/count and AA83A0 detach helpers | Keep actual reader, Lua owner/ref storage, raw key array and Source frames alive. BD7A20 is the existing vector-pop adjustor, not a missing leaf. |

These providers are present in the inspected Source. Availability of their bodies is separate from canonical production binding, normal Text activation, or terminal ownership/retirement proof.

## Whole AAA710 remains incomplete

Raw prefix and interaction Source cover exactly `[AAA710, AAACD4)`. The missing continuation has entry `00AAACD4`, exclusive end `00AAAECF`: **507 bytes**, including key enumeration, child construction/load, string cleanup and the whole-function epilogue. The current logical loader/builder cannot stand in for it.

The first tail stage initializes only the tags of 250 records of eight bytes to -1, preserves payload preimages, zeros the count, and dispatches the current reader+18 target (CE44FC's own target is BD5F50). It can reuse the actual `NativeLuaReaderKeys` layout at **property scratch+4C**, with count at +81C, within the existing 820h scratch. It must not substitute a vector snapshot: the signed count is read again after each child continuation.

String-key children then use actual raw string construction, AA2490 type inference, **AA6560 actual raw factory**, canonical B74EB0 model-slot allocation/B75030 construction, optional AA83A0 detach, raw list publication, B6E680 parenting, child current+74, reader current+04, child current+18, reader current+08, child current+78, and consumed-before-getter name return. Real returned partial widget/node/list/reader state must survive entered failure; a parent operation must retain the same graph and borrowed Lua/runtime domains. This document does not recover the private native EH frame or add cleanup.

The concrete missing factory contract is AA6560. Existing `GuiWidgetOwnerRuntime::construct_child_00aa6560(GuiLayoutWidget&)` is a retained logical-owner fragment admitting Group, Icon, FrameBox, ClipBox, Text, Section and Listbox. It also constructs/binds a model and has a `retire_tree` failure catch. Substituting it for raw AA6560 would duplicate the tail's subsequent model creation and change failure behavior. Original's raw fastcall switch accepts type1..18 (AA2490 infers child IDs2..18), returns the actual widget pointer, and includes additional types and the special Listbox factory path. Its complete raw dispatch/provider composition is not supplied by that fragment. A next bounded packet should establish this raw factory contract and real per-type providers; unsupported branches must stay explicit rather than mapping to a base widget or skipping children.

AA2490's raw-header helper already exists, but it uses host `_stricmp` and compiled class-name strings. That is its existing bounded CRT/literal contract, not proof of the newly recovered BF7FBF parent using the Original live locale/error services and actual class-literal identities. A future full tail must either preserve that stated restricted domain or close the explicit comparer/literal binding; it must not silently call it canonical Original CRT ownership.

## Smallest truthful whole-body interface proposal

A future `read_native_gui_layer_properties_00ac4c50(actual_page, actual_visitor, scratch, bindings, operation)` needs actual Priority/RenderOrder views; shared argument staging of 24 bytes (18h), a priority DWORD preimage and XYZ words; the existing visitor dispatcher; exact current+78 dispatch to real acquisition frames; real root/position providers; and a genuinely complete AAA710 provider. These are proposed Source contracts, not an Original stack-layout claim.

Bindings/frame availability may be checked before entry. Current callback-written vtables, scene/node pointers and field values must be loaded at their actual call sites. An unsupported target discovered after a reader is an **entered** failure with retained prior stores, not a harmless preflight rejection. The Source owner must guard the whole partial graph; neither a no-op tail nor a shared-pointer alias over logical ownership closes that obligation.

**Verdict: not ready for a closed whole-body raw implementation using only supplied providers.** The 216-byte schedule is established, and a prefix could be labelled partial explicitly, but adding a fake/default AAA710 callback would conceal the missing work. No Source lease or edit is taken. Names/comments and Ghidra analysis remain unchanged. The report records exact call sites, byte/source hashes, and the readiness boundary; it makes no build/runtime/gameplay or new Original-body claim.
