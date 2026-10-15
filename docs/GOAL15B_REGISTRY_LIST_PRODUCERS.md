# Registry list producers

These three complete helpers allocate the existing `0Ch` sentinel/node records
and initialize an existing list header. Correct library-shaped names are
retained; they remain descriptive hypotheses rather than recovered symbols.
Allocation uses the genuine `singleton_lifetime_allocate` object service with
native and source sizes both `0Ch`, matching the established CRT free service.
No host callback, owner record, payload constructor or allocator policy is added.

| Native name | Inclusive byte body | Original ABI | Coverage | Source entry |
| --- | --- | --- | --- | --- |
| `STL_List_BuyHeadNode_0041bfd0` | `0041BFD0..0041BFE9`, 26 bytes | no arguments or receiver; EAX node; plain RET | complete | `buy_native_registry_list_head_0041bfd0` |
| `STL_List_BuyNode_0041c1f0` | `0041C1F0..0041C222`, 51 bytes | next, previous, value-slot address on stack; incoming ECX unused; EAX node; RET 0Ch | complete | `buy_native_registry_list_node_0041c1f0` |
| `STL_List_Construct_0041E270` | `0041E270..0041E285`, 22 bytes | ECX existing list header; EAX same header; ESI preserved; plain RET | complete | `construct_native_registry_list_0041e270` |

The source uses cdecl, stdcall and one-argument Win32 fastcall respectively.
These calling shapes do not establish native stack/register/exception ABI
equivalence. In particular, the source's by-value arguments are not writable
aliases to an original native frame's argument cells.

## Producer layout and exact schedules

`Cc13TerrainGridLayerListNode` in `cc13_avoid1.hpp` is reused, with compile-time
checks for size `0Ch`, next `+0h`, previous `+4h`, and payload `+8h`. Actual byte
stores avoid value-initializing an unread field. These helpers allocate raw
node storage; they neither create nor destroy a terrain-grid-layer payload.
The existing registry schema places its list header at registry `+4h`, with
proxy/header `+0h`, sentinel `+4h`, and count `+8h`. The constructor accepts
only that existing raw `0Ch` header and declares no outer class or lifetime.

The sentinel producer has `0041BFD2 CALL 00BF681B`, with PUSH 0Ch at
`0041BFD0`; ADD ESP,4 at `0041BFD7` balances the one allocation argument.
On return, `0041BFDA..0041BFDE`
checks the allocation address and writes next=self if nonzero. Then
`0041BFE0..0041BFE7` computes allocation+4, checks that slot address, and writes
previous=self. There is no payload write. EAX still contains the allocated node
at RET (`0041BFE9`).

The node producer similarly allocates `0Ch`: `0041C1F2 CALL 00BF681B`, followed
by ADD ESP,4 at `0041C1F7`. Its exact subsequent order is:

1. Guard allocation address; load the first stack argument and write next at
   `0041C1FE..0041C202`.
2. Compute and guard allocation+4; load the second argument and write previous
   at `0041C20B..0041C20F`.
3. Compute and guard allocation+8; load the third argument's address at
   `0041C218`, dereference it at `0041C21C`, then store that payload pointer at
   `0041C21E`. RET 0Ch begins at `0041C220` and ends at byte `0041C222`, proving
   the three consumed stack arguments.

The source must not snapshot `*value_address` before allocation. A registered
new handler can change the pointed-to slot before allocation succeeds. It must
also preserve that read after both link writes: an alias to either written slot
must observe the new bytes. Raw `memcpy` reads/writes preserve that schedule;
the supplied slot must remain readable until its actual use.

The address guards are on the individual field addresses, not a blanket
"allocation failed, return null" path. The source retains their 32-bit wrapped
address computations. The admitted concrete allocation service returns valid
nonnull storage or throws; no source provider returning null/wrapped invalid
storage is installed, and no recovery behavior for such storage is invented.

For the constructor, `0041E273 CALL 0041BFD0` completes before any header store.
It writes the returned head at header+4 (`0041E278`), zeroes count at header+8
(`0041E27B`), and returns the same header via EAX (`0041E282`). Proxy/header+0
remains untouched. If allocation throws, the helper has performed no header
stores; no catch, retry, rollback or extra initialization is added here.

## Allocation service and all consumers

`operator_new` (`00BF681B`) was checked in pseudocode and complete assembly.
It calls malloc, retries via `__callnewh(size)` on failure, returns a nonnull
raw pointer in EAX, or throws bad_alloc. The existing source implementation
uses `std::malloc`, `_callnewh`, and `std::bad_alloc` with `host_bytes`; the
request is `{object, 0Ch, 0Ch}`. Its existing source CRT handler and exception
identity are qualified boundaries, not a replacement native CRT identity.

All live references were inspected:

* Registry constructor `00424730`: `00424763 CALL 0041BFD0`, followed by the
  sentinel and count stores in the raw interior header.
* Same constructor: `004247B7 CALL 0041C1F0`, `00424813 CALL 0041C1F0`, and
  `0042486F CALL 0041C1F0`, each with next=sentinel,
  previous=sentinel.previous, and the address of a payload local.
* Stream loader `004248A0`: `0042494F CALL 0041C1F0`, with the same argument
  arrangement for the deserialized payload local.

Both use incoming ECX for their list object, but the node producer ignores it;
its required inputs are the three stack arguments. No xrefs were found to
the standalone `0041E270` constructor, so no reachable runtime path is asserted.

The existing `Cc13AvoidZoneRegistryHost` producer methods can bind these source
entries; no shared host interface or caller wiring is modified by this packet.

## Verification and limits

Analysis used `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; the live
client verifies identity per batch. Complete owned assembly/prototypes,
producer code, allocator body and all native caller setups were inspected.
MSVC x86 `/std:c++17 /EHsc /W4 /WX /permissive- /O2 /c /Iinclude`
compilation passed with zero warnings. Validation results are recorded in
`reports/goal15b_registry_list_producers.json`. All eight report call rows and
eight markdown call claims passed with no failures, unchecked claims or warnings.
Primary registers `src/native_registry_list_producers.cpp`, runs the required
full Win32 build and existing checks, and owns saved Ghidra annotation/export
refresh. No new tests, native differential execution, game runtime proof,
original ABI compatibility or complete outer-class claim is included.

Primary integration: registered MSVC Win32 build and all three existing
CTest suites pass. Ghidra annotations are saved and exports refreshed.
See [the batch receipt](../reports/goal15b_reconstruction_batch.json) for
the precise Source fixture/static-proof scope and remaining Native limits.
