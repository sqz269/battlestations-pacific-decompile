# Whole raw camera-store removal (packet 89)

Packet 89 implements the whole raw `AA46B0` erase alias first, then the whole
raw `AA4B30` manager parent. It uses the existing raw tree, Source CRT free,
returning SDK validation and actual raw name accessor. No typed map, new owner,
callback logger, camera/scene release or normal activation is added.

Published Root `1c4fc481b` was merged with exit 0 into this worker only, producing
baseline `f892b125d64a5c4f1c90b1150b7663c5470aef91`. Source is confined to the
existing `native_gui_camera_store_map.hpp/.cpp`; no CMake or Main edits. Only
AA46B0 and AA4B30 are annotated. Existing raw map allocation/insertion/find/create
bodies and the separate typed removal API remain unchanged.

## Whole erase alias and admitted Source domain

`erase_native_gui_camera_store_iterator_00aa46b0` forwards exactly tree,
unused EDX, output, input owner and input node to the accepted whole raw
`erase_native_int_pointer_tree18_iterator_0086e8a0`. The complete 679B/231-ins
Original bodies differ only at thirteen CALL operands and the handler pointer;
the remaining 623 bytes match. Whole minimum/maximum/rotations/increment and EH
maps/actions were audited in [packet 87](CC12_NATIVE_GUI_MANAGER_TREE_ERASE_READINESS_87.md).
There is no second erase algorithm or missing typed semantic substitute.

The native ABI is ECX tree, unconsumed EDX, three stack words output/owner/node,
EAX output and RET0C. Actual header is opaque0/head4/unsignedcount8; 18h nodes
hold left0/parent4/right8/keyC/payload10/colorbyte14/nilbyte15. The accepted body
advances the actual by-value iterator, transplants/rebalances, frees the original
node only, then reloads/decrements current unsigned count iff nonzero and stores
the captured current owner/node output. No payload release or owner==tree gate
is added. Source wrappers/private frames, current SDK/Source CRT, owning Source
exception RTTI, Original heap/encoded-handler/FH3/SEH and binary ABI limits are
inherited unchanged. The new alias has no additional context or defaults.

## Whole parent schedule

`remove_native_gui_camera_store_00aa4b30` is a new Source interface taking the
actual manager, requested store identity and required nondefault three-pointer
`NativeGuiCameraStoreRemoveLiterals` view. Its two format pointers and empty
pointer are opaque and passed only to genuine RET logging; bytes are never
scanned/copied. Metadata stays stable and independent of retired backing. This
does not supply an Original literal owner or hardcoded callable VA.

Original AA4B30 is `[AA4B30,AA4C1D)`, 237B/81 instructions: ECX manager, one
stack store, void/RET4. The Source implementation preserves:

1. Real 4254B0 RET, then current head at manager+C, tree address manager+8 and
   begin from head.left. Capture head once in the actual private 16B local.
   Its iterator is at 0; its output is at 8, and the **same output.node+C word**
   holds the initial end until the final erase writes its result. Output.owner
   remains uninitialized until that write, as in Original.
2. Current iterator owner/cursor locals, owner/tree checks and every current
   owner.head check, with real SDK `_invalid_parameter_noinfo` continuation if
   its installed handler returns. Mismatch uses real `4BDA70` on the same local
   iterator, then reloads cursor and owner; the initial end is not recaptured.
3. On the first payload match, check current head, reload current node+10 store,
   read its current **camera/node +18** (scene is +1C), call genuine B6D800 for
   the same raw name header at +54, then read name+4. Only null selects the
   required F8BC60 empty pointer. Evaluate this even though logging is RET.
4. Recheck current head, reload the payload for logging, invoke genuine RET,
   recheck current head again and reload current payload for free. If nonnull,
   `singleton_lifetime_free` frees it in the same owning Source CRT domain as
   raw AA5070 creation. **Only after free**, clear the captured node+10.
5. Pass actual tree, private output and captured current owner/node to the whole
   erase alias and return. Only the first match is removed. No bool, retained
   camera/scene action, usage-count decrement, rollback or extra cleanup exists.

All current raw pointer accesses use the established volatile helpers. The
private Source frame/API are explicit substitutions for private Native stack
identity. The live initialized tree, surviving links and output must remain
valid/aligned through reached accesses. The camera must be an actual live raw
`NativeNodeStorage` with its initialized `NativeString` member; a cast from
`SceneResource` or a logical layout is not admitted. No unowned aliases into
private argument/EH scratch, malformed/concurrent mutation, after-free backing
access, failed-entry retry or complete graph lifetime proof is added.

## Real trivial providers and annotation correction

Verified BSP wrapper batches used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Installed bytes independently match live bytes:
4254B0 is exactly `C3`; B6D800 is exactly `8D 41 54 C3` (LEA EAX,[ECX+54]; RET).
The existing actual Source accessor returns `node.name_54`; the private logger
projects the actual RET body and is not an unresolved-service stub. MSVC spells
the Source inline-assembly RET as `C2 00 00` (RET0), the same no-stack-pop
behavior. No helper byte/Native ABI equivalence is claimed. Object inspection
confirms actual logger calls and argument evaluation remain.

Earlier AA4B30 comments describing a post-free flow gap are stale. The current
whole listing continues from BF65AC at AA4BF6 to AA4BFB, clears node+10 at
AA4BFE, calls erase at AA4C0E and reaches RET4 at AA4C1A. A dated correction is
appended, preserving prior evidence. The old AA46B0 whole-body `xlen_throw`
label is replaced by descriptive `BSP_GuiCameraStoreTree_EraseIterator`.
Both owned entries use annotation lock/history/save and refreshed exports;
other library names and unrelated entries are untouched. No flow repair or
broader type/tag mutation is claimed.

## Focused proof and limits

Fresh production-object compile is MSVC Win32 `/MD /O2 /W4 /WX`. Canonical
`scripts/build.ps1` exits 0 and all three existing checks pass. The old unrelated
`spawn_request_id_matches` LNK4006 warning remains. No persistent test or CMake
change was added.

Exactly one manifested private Source case uses the actual accepted raw head
allocator, valid explicitly initialized raw tree links/count, and real AA5070
creation/insertion of two Source-CRT stores/nodes. Removing the second exercises
real checked increment before the match. `_get_heap_handle` and Windows
`HeapWalk` observe actual owning-CRT BUSY allocation identities before/after:
the matched store and node cease to be busy; the other store/node survive.
Count goes 2 to 1 and remaining root/extrema/links/payload are correct. No freed
store/node backing is read; the observation uses numeric allocation identities.

The fixture's real Source `NativeNodeStorage` initializes the reached name and
count37 sentinel only, with a null opaque scene argument. Unchanged count/name
prove no reached camera action or copy; they do **not** establish a complete
Native camera constructor, camera lifetime, scene owner or ABI. Source fixture
literal arrays do not claim Original fixed-VA identities. Cleanup explicitly
frees the surviving payload separately from raw-node erasure and releases the
fixture name; no implicit payload owner is invented.

The case observes final heap/tree results. **Free -> clear -> erase order is
static Source/Original and compiled-object proof**, not an instant dynamic observation at
the free call. There is no new Native differential, invalid-handler/exception
case, Native FH3/SEH/private-stack/heap/ABI proof, populated scene cleanup,
Source layout/ancestor lifetime admission or game activation claim. All exact
Original bytes, call-site rows, Source hashes and artifact hashes are recorded
in the JSON receipt before any future build reuse.
The live report verifier checks all 25 declared Original direct CALL sites
with zero failures; that check is separate from Source execution and Native ABI.
