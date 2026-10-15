# Registry terrain-grid-layer list clear

`0041DED0` clears the avoid-zone registry's owning list of terrain-grid layers.
The packet filename retains its initial `zone_group_clear` label; the manager's
zone-group array is a different record. Names below are descriptive hypotheses.

| Routine | Native body, inclusive | Original ABI | Coverage | New C++ entry |
| --- | --- | --- | --- | --- |
| Registry clear layers | `0041DED0..0041DF34` | ECX registry; no stack arguments; plain RET; no defined result | complete | `native_avoid_zone_registry_clear_layers_0041ded0` |

The live project was `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. `bsp.py brief`, live count/prototype, pseudocode,
complete assembly and xrefs were checked on 2026-10-15. No Ghidra writes,
flow repairs, or shared-source edits were made by this packet.

## Record and producer evidence

Reuse `Cc13AvoidZoneRegistryNative`, `Cc13TerrainGridLayerListNode` and
`Cc13TerrainGridLayerNative` in `include/bsp/cc13_avoid1.hpp`; no duplicate
record declaration is introduced. Registry `+8h` is the sentinel pointer and
`+Ch` the count. A node is `0Ch` bytes: next `+0h`, previous `+4h`, payload `+8h`.

`0041BFD0` allocates a `0Ch` sentinel and makes its next/previous links point to
itself; it does not initialize the payload slot. `0041C1F0` allocates a `0Ch`
node and stores its three supplied next, previous and payload words in that
order. These producer bodies establish the node layout.

The other producer is `BSP_AvoidZoneRegistry_LoadFromStream` (`004248A0`):

* `004248BA..004248C1` retains ECX as the registry and clears it before loading.
* `00424904..00424924` allocates `28h` bytes, writes vtable `00CE38CC`, and zeros
  the grid-vector first/last/end words at payload `+1Ch/+20h/+24h`.
* `00424927..0042492F` passes the stream to `BSP_TerrainGridLayer_Deserialize`.
* `00424938..0042494F` supplies the new payload to the node producer using the
  registry's sentinel; `00424954..0042496A` increments the list count and links
  the node at the tail.

Existing `cc13_avoid1.cpp` also constructs the same registry in
`cc13_avoid_registry_construct_00424730` and already exposes
`Cc13AvoidZoneRegistryHost::clear_layers_0041ded0`. The new helper supplies that
operation's body for a host binding; this packet does not change those callers.

## Complete schedule and host boundaries

| Site in `0041DED0` | Native service | Body checked and contract |
| --- | --- | --- |
| `0041DEE9`, `0041DF06` | `LIBCRT_unmatched_00bf6713` | `00BF6713..00BF6722`: five zero words passed to `00BF66EF`, caller discards `14h`, plain RET. Host preserves invalid-parameter policy and may return. |
| `0041DEFB` | payload vtable slot `+0h` | ECX payload; PUSH 1 at `0041DEF9`. Producer vtable `00CE38CC` selects `0041F640`; body inspected in pseudocode and assembly. Host must dispatch the actual payload vtable. |
| `0041DF20` | `_free`, `00BF65AC` | Library thunk jumps to CRT free at `00BF9DC8`; one node pointer on stack, ADD ESP,4 at `0041DF25`. Host releases with the matching allocator. |

The normal produced payload destructor (`0041F640..0041F681`) sets the layer
vtable, frees a non-null grid pointer, zeroes its three grid-vector words, and
frees the layer itself when the supplied low-byte flags have bit zero set.
It returns the old layer address in EAX and uses RET 4. The clear routine ignores
that result. The host callback accepts the full pushed word `1` and represents
the virtual dispatch, rather than hardcoding this one vtable implementation.

`0041DED3..0041DED7` exits immediately for count zero without touching the
sentinel. Each iteration loads the first node and checks it against the current
sentinel at `0041DEE0..0041DEE9`. A non-null payload receives scalar deletion at
`0041DEEE..0041DEFB`. Crucially, `0041DEFD..0041DF06` reloads the sentinel and
front after that callback and repeats the checked-iterator report. The source
preserves this observable ordering, including callback changes to the list.

The additional comparison at `0041DF0B..0041DF0E` occurs after a returning
invalid handler. Only a non-sentinel node is then unlinked in both directions,
freed and followed by decrementing the current registry count
(`0041DF10..0041DF28`). The next loop condition reads that count again. Normal
empty completion leaves the sentinel self-linked and the count zero; the
sentinel, list proxy, registry vtable and cached maximum cell height are not
reset or freed by this routine. Null payloads still have their list node removed.

All xrefs were inspected:

* Registry destruction (`0041F0D0`): `0041F0FB CALL 0041DED0`, before the
  separate list destructor.
* Stream reload (`004248A0`): `004248C1 CALL 0041DED0`, before consuming stream
  records.

Neither call supplies stack arguments.
`0041DED0` preserves EDI and ESI through its own PUSH/POP pairs; its indirect
and host services retain their separate calling-convention obligations. The
new C++ interface does not promise native ABI or register/stack equivalence.

## Corrections and limits

The existing `Cc13AvoidZoneRegistryHost` comment says its invalid-parameter host
must not return. This packet does not adopt that restriction: the inspected
wrapper ends in plain RET, and the newer
`native_invalid_parameter_noinfo_call.hpp` explicitly promises neither
`noexcept` nor `noreturn`. Default CRT policy may terminate, but user-handler
policy belongs to the host. If a handler returns without making a malformed
list safe, native unchecked reads or nontermination remain possible. The
reconstruction adds neither a repair policy nor sentinel-payload initialization.

This is complete control-flow reconstruction under explicit host services,
not a drop-in binary replacement. Payload dynamic type, matching allocators,
list lifetime and invalid-handler effects remain host responsibilities.
No game execution, payload ABI compatibility or runtime equivalence is claimed.

## Validation

`verify_report_calls.py` passed all 11 concrete report call rows and both
markdown call claims with no failures or warnings. The indirect vtable call
is deliberately reported as indirect; its instruction and normal producer
target were checked separately above. MSVC x86 compilation with
`/std:c++17 /EHsc /W4 /c /Iinclude` passed with no warnings; layout assertions
compiled. Details are in `reports/goal15_zone_group_clear.json`.
The primary integrator registers
`src/native_zone_group_clear.cpp`, runs the complete required build and existing
checks, and handles Ghidra annotation. No new test cases were added.

Primary integration update: the registered MSVC Win32 build and all three
existing CTest suites passed. Ghidra annotation/save and standard export
refresh are complete. See [the batch receipt](../reports/goal15_reconstruction_batch.json).
These checks do not execute or establish original ABI/gameplay equivalence for this entry.
