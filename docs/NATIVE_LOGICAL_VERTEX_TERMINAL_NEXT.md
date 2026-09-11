# Logical vertex non-null terminal discovery

`native_logical_vertex_terminal_next`, 2026-09-11, based on main `5bf77a3`.

**The full logical-vertex owner remains conditional and is not ready for source
reconstruction.** No reviewed producer establishes the concrete pointee profile
of a non-null logical vertex `+4C`. Constructor zeroing does not prove that field
stays null after publication. No new terminal domain is admitted by this packet.
The previously ready `B49570` slot-return leaf is unchanged; it is not new progress.

This is the targeted continuation of
[the prior full-owner discovery](NATIVE_LOGICAL_VERTEX_OWNER_NEXT.md), not a second
claim to have reconstructed its shell. The companion
[report](../reports/native_logical_vertex_terminal_next.json) contains 40 freshly
matched saved-Ghidra/PE spans totaling 17,318 bytes, source/provider hashes, exact
candidate exclusions, and conditional file/address ownership. Ghidra was the
existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; every live read
used the project/program guard in `bsp.py ghidra`.

## Terminal contract that must remain open

The complete `B62010..B62097` body installs `D62B68`, captures the current `+4C`,
and, when non-null, uses real `InterlockedDecrement` on captured pointee `+4`
(`B62047`). On zero it reads that pointee's **current** table and calls slot zero
at `B62057`. Only after that release/terminal returns does `B62059` clear owner
`+4C`. It subsequently captures current `+50`, conditionally scalar-frees through
`BF65AC`, and clears `+50` at the returning-free continuation `B62070`.
It then disarms the unwind state and invokes `BD30F0` to write `CEB130`.

`BD30E0` is the common zero-reference dispatcher: it invokes current slot `+4`
with stack deleting flag 1. Finding that dispatcher or the base deleting wrapper
`B620A0` cannot identify an unknown pointee's actual deleting function or allocator.
`B620A0` itself calls `B62010` and conditionally frees; it does not close this cycle.

The preceding `B4B5D0` teardown still has its established order: current optional
renderer guard; dynamic `+19F4` lock and `B4B3F0` physical registration removal;
current-global `B268E0` registry removal; captured current declaration `+68`
decrement/zero terminal; then captured current physical `+58` decrement/zero
terminal; normal guard leave; full base teardown. The normal base call is made
with the logical unwind state disarmed. No owner `+58`/`+68` clear is invented.

Fresh pins include the entire 280-byte logical destructor, 32-byte deleting
wrapper, 105-byte slot return, 135-byte base destructor, both unwind maps and
FuncInfo records, and all five associated funclets/handlers. The logical unwind
is guard `B21110` then base `B62010`; the base unwind is `B48D70 -> BD30F0`.
There is no added unwind action for the separate `+19F4` lock. This preserves the
previously discovered hidden tail and EH contract; it is not execution evidence.

## Concrete producer and escape evidence

The base constructor `B61E20..B61F84` zeros `+4C` at `B61E92`. Its current direct
callers remain `B4A9B0` and `B4BC00`. `D62B68` has the constructor/destructor
stores; `D61D6C` has the two derived constructor stores and logical destructor
store. These are current static references, not an exhaustive lifetime proof.
The 13-DWORD logical profile ends at `D61DA0`, where another physical-base profile
starts. Likewise the pool routine `B62590` following the base profile's 13 DWORDs
must not be treated as an extra logical-vertex virtual method.

* `B287C0..B288AA` (`ECX` renderer, stack count/flags/declaration, `EAX` result,
  `RET 0C`) calls the full `B4BC00` constructor at `B28804`. It publishes the raw
  returned stream to the renderer `+1AAC` array at `B28847`, increments its count,
  then calls the current renderer virtual `+58`. When `AL == 1`, it additionally
  publishes the raw stream to the `+19B0` array at `B2888E`, and returns it to its
  caller. These escaped aliases prevent extending constructor-null evidence to
  the whole object's life. This named factory remains a bounded dependency;
  existing typed constructor fragments do not supply a full actual producer.
* `AE47E0..AE4C1C` is the current direct caller of borrowed constructor `B4A9B0`.
  Its native ABI is `ECX` terrain owner, ten stack DWORDs, `RET 28`; the decompiler
  argument list is not authoritative. Calls at `AE4941` and `AE4A83` publish the
  stream into terrain owner `+54` at `AE494F`/`AE4A91`. Stores at
  `AE495C`/`AE4A9E` write **logical `+54` = `80000000`**, not logical `+4C`.
  It passes both input and new logical streams to `B85B80` draw-section attachment
  (`AE4963`/`AE496E`, `AE4AA5`/`AE4AB0`) and the section to `B73C60` geometry
  attachment (`AE499A`, `AE4AD8`). Its later release of the terrain-held pointer
  and clearing of terrain `+54` do not erase those published aliases.
  `AE47E0`, `B4A9B0`, and `B85B80` have no full actual implementation in the scoped
  lookup; `B73C60` is named with a read-only attachment contract, not a full provider.

The distinct compressed metadata route is also concrete: `B93800..B93874`
selects the geometry's last stream, reads its declaration through current virtual
`+24`, allocates `declaration_count * 20h` raw bytes, reads the payload, reselects
the stream, and calls `B61D90`. That 10-byte setter stores **`+50`**, while the
13-byte `B61E10` getter returns `[+50] + index * 20h`. `B61F90` lazily allocates
and copies an eight-DWORD record into that same `+50` buffer. The existing typed
mesh payload/record fragments do not bind a retained owner to `+4C`.

## Excluded search candidates

Raw PE opcode scans and a bounded graph prioritizer produced 498 decoded-offset
candidates and 31 candidates near geometry/memory references. Nearest preceding
function entries and graph proximity were used only for prioritization. They
are not containment, alias, type, or whole-program absence proofs. The following
reviewed candidates illustrate the actual exclusions; unreviewed candidates are
not asserted harmless.

| Candidate | Exact reason it does not establish logical `+4C` |
| --- | --- |
| `B4EC78` | The actual instruction is `mov [ecx+24C],eax`, inside raw block `B4EC60..B4EC81`; a nearest-entry assignment to the four-byte getter at `B4EC50` is invalid. |
| `B42350` / `B43309` | `mov [ecx+4C],eax` is one contiguous shader constant output among offsets `+38..+54`; its source is scene data `+188`. Earlier `B4290D` reads a draw-section stream count at that other object's `+4C`; the logical stream is separately loaded at `B4294B`. |
| `BA7530` / `BA7841` | The store writes an SSE-derived float into the returned locked vertex payload, amid repeated position/attribute writes. It does not store an owner pointer into the logical object. |
| `BC6060` / `BC632F` | `EAX` comes from logical virtual `+10` Lock at `BC62AD`. Stores to payload `+0C,+2C,+4C,+6C` form repeated vertex records, followed by current logical `+14` Unlock at `BC6373`. |
| `B8CC90` / `B8D0CE` | The value is an SSE-derived UV component in a raw vertex output array, adjacent to paired stores at `+20/+24`, `+48/+4C`, and `+70/+74`. |
| `B97100` / `B971DA` | `lea ecx,[esi+4C]` selects an embedded array in a different owner. The constructor installs `D638F8` at `B97144` and builds repeated arrays at `+0C,+1C,+2C,+3C,+4C`; it is not a `D61D6C` logical vertex object. |

## Available providers and the missing binding

Current source/header hashes are recorded for the existing actual providers:
CPU declaration `D61D1C -> BD30E0/B48CA0`, pool `108FD38` and type sizes
`D61CC0`; private physical vertex `D61E34 -> BD30E0/B4BB40`; pooled physical
vertex `D61E7C -> BD30E0/B4C230`, pool `108FDE0`; actual unregister and optional
guard functions; and the shared scalar-free lifetime domain. They already
resolve their established `+68`, `+58`, guard, registration, and `+50` roles.

The retained-memory providers additionally support actual profiles
`D15AD8 -> BD30E0/8D4470` and `D642C0 -> BD30E0/BB8F90`. There is no recovered
logical `+4C` producer proving either profile belongs to this field. Current
`B23640` retained-slot assignment references are texture loader sites
`B2C624`/`B2C80E`; they do not establish a logical vertex assignment. Choosing a
known provider because its terminal is available would fabricate the domain.

The next full owner packet remains conditional: roots `B4B5D0`, `B4BF10`,
`B49570`, `B62010`; files `include/bsp/native_logical_vertex_owner.hpp`,
`src/native_logical_vertex_owner.cpp`, `docs/NATIVE_LOGICAL_VERTEX_OWNER.md`, and
`reports/native_logical_vertex_owner_audit.json`. It needs a concrete non-null
`+4C` writer and its profile/current-zero terminal/deleting function/allocation
domain, or a complete scoped lifetime proof accounting for all admitted escapes.
Only after that evidence exists can additional terminal addresses be assigned.
No fabricated address list, generic terminal callback, or constructor-null
restriction makes this packet ready. The missing contract continues to block
general logical-vertex teardown and callers such as `B24840`.

Validation here is read-only evidence matching and artifact consistency. No C++
was changed, compiled, fixture-tested, ABI-replaced, or game-validated, and no
Ghidra annotation or shared metadata was changed.
