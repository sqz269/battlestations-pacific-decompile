# Animation and bone resource type IDs (R30)

## Result

The CRT entries at `00CD82F0..00CD833E` and `00CD8340..00CD838E` initialize the process descriptors for `cAnimationResource` and `cBoneResource`. `NativeResourceExtraTypeIds` reconstructs both complete 79-byte bodies over the existing `TypeIdCounterLifetime` and `NativeMeshResourceTypeIds` scene bootstrap. `GameNativeTypeStorage` owns the stable guards and four-word descriptors and exposes them through `resource_extra_types()`.

Each descriptor is `[own ID, scene ID, root ID, name address]`. The exact cells are:

| Type | Guard | Descriptor | Name word | Native name |
| --- | --- | --- | --- | --- |
| animation | `01090264` | `01090268[4]` | `01090274 = 00D63258` | `cAnimationResource` |
| bone | `01090265` | `01090278[4]` | `01090284 = 00D6326C` | `cBoneResource` |

This resolves the wider meaning left open by the R29 profile getters: their current `01090274` and `01090284` slot-14 cells are the descriptor type-name words. The R30 source does not edit the R29 profile implementation.

## Ordering and failure state

Both entries test the current guard, then publish guard `1` and the name before calling `00B869C0` on canonical scene storage `01090210`. They load the current scene and root IDs into registers before either destination store. They then call `006FAC20`, load the current counter, store the incremented counter, and finally store the captured old value as the descriptor own ID.

The early guard is deliberately sticky. An exception from either shared bootstrap leaves the guard and any preceding writes in place, and a later call returns without repairing that partial state. No fallback counter, scene domain, rollback, or cleanup was added.

The CRT table entries are `00CE35B4 -> 00CD82F0` and `00CE35B8 -> 00CD8340`; `00CE35F0 -> 00CD8690` establishes that both precede the mesh initializer. `GameNativeTypeStorage::initialize_resource_types` retains its three existing selector calls, invokes these two entries, then continues with the mesh family.

## Evidence and limits

`reports/native_resource_extra_type_ids_r30.json` records complete live/disk-matching body bytes, the CRT span through the mesh entry, both name strings, and all four direct call rows. The strict MSVC Win32 `/MD` build passed all three existing CTests. An ignored focused `/W4 /WX /fp:strict /MD` probe linked with an embedded manifest and verified current parent-cell capture, current counter selection, sequential publication, and sticky guard behavior.

This is source reconstruction with original control/data-order evidence. It does not establish drop-in native ABI, native FH3/SEH behavior, or gameplay validation.
