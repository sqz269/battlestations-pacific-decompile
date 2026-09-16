# AnimationChannels and Bone auxiliary profiles

The actual AnimationChannels profile `00D6328C` and Bone profile `00D632B8`
share these auxiliary contracts. Numeric profile identities select borrowed
current table cells; source code does not manufacture a native vtable.

| Slot | AnimationChannels | Bone | Complete behavior |
| --- | --- | --- | --- |
| `08` | `00B8A060` | `00B8A140` | Read current DWORD at `01090268` / `01090278`. |
| `0C` | `00B8A730` | `00B8A870` | Compare stacked token to the three current descriptor DWORDs in order; stop at first match. |
| `14` | `00B8A070` | `00B8A150` | Read the separate fourth DWORD at `01090274` / `01090284`. Its wider meaning remains unassigned. |
| `18` | `00B8A080` | `00B8A160` | Call the instance's current slot `08` with `(item,node)`; record and creation word are unused. |

The four six-byte getters reuse `read_native_mesh_binding_type_00b931b0` over
the same live cells. The two forty-byte predicates reuse
`matches_native_fallback_type_00b86950`: stack token, `RET4`, `AL` Boolean;
the incoming object in `ECX` is ignored and upper `EAX` is not a Boolean.
No descriptor value is captured at adapter construction.

`NativeResourceExtraItemProfileCalls` implements the existing resource-instance
publication interface. It reads known item profiles' current slots, handles
these getters and predicates, and forwards unrelated entries to the supplied
complete provider. A changed actual slot therefore selects the changed entry.

The identical 24-byte attachment bodies take native `ECX=item` and stacked
`(instance,record,node,creation_word)`, then return with `RET16`. They capture
the current instance table and slot `08`, invoke it with `ECX=instance` and
stacked `(item,node)`, and perform no cleanup or rollback. Their sole indirect
calls are `00B8A092` and `00B8A172`. They have no EH prologue or cleanup state.

`attach_native_extra_item_00b8a080` serves both native bodies. Its bounded
instance domains are current `00D63244+08 = 00B89E90` and
`00CFD8E0+08 = 0071B710`, whose complete source publication implementations
already exist. It calls those implementations directly, preserving base pair
publication and, for the game instance, all five independent classifications.
An unsupported profile or changed terminal is an explicit source error.

## Evidence and limits

The report records all 152 function bytes, both item profile neighborhoods,
both instance profile prefixes, and exact indirect call instructions. Parent
report `native_resource_extra_item_profile_definition_r29.json` preserves the
six missing-function repairs. The worker only queried Ghidra.

The strict MSVC Win32 Release build passed with `/W4 /WX /fp:strict /MD`,
and all three existing CTests passed. One ignored probe passed 20 comparisons
against relocated original getter/predicate bodies. It also exercised the
attachment helper with source-constructed base and game instance storage,
current descriptor reloads, classification buckets and changed-slot rejection.
That attachment exercise is source fixture evidence; only the getters and
predicates were compared with executing original instructions.

These are source interfaces over actual raw storage. They do not supply native
callable ABI, arbitrary instance providers, graph application wiring, FH3/SEH
or hardware-fault identity, or gameplay validation. In particular, pointer-like
Ghidra return types for the predicates are decompiler artifacts of the partial
`AL` write; only the low byte is specified by this source contract.
