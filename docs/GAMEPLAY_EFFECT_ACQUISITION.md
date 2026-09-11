# Gameplay effect acquisition

Addresses: 008700e0, 00871b50, 00871ba0, 0086f930, 00869990, 0086ebe0,
0086ab70, 004dc6a0.

Packet `orch4_effect_acquisition_m` reconstructs the complete normal acquisition
sequence and both name wrappers. The manager, name index, actual 24h definition
allocation and identity assignment execute their existing reconstructed routines.
The definition's current virtual slot+8 (`00870400`) remains an explicit required
component-loader service. Descriptive names are hypotheses, not recovered symbols.

## Native behavior and ABI

`008700E0` consumes ECX=manager and stack output-pointer, signed ID, flag;
EAX returns the output address, RETCh at `00870369`. ID<=0 writes null directly;
it neither reads nor releases an old output reference. A cache hit finds the
signed ID in the weak map, atomically increments the pointed definition's real
+4 reference word, and rereads the map value for the final output store.

On a miss, resolve CURRENT `[00E188A8]+1A0C`, obtain globals and `Effects`, and
release the globals temporary before obtaining `Effects[ID]`. This is the
embedded game Lua state, not the mission owner at +1A08. Only a zero LOW BYTE of
the flag enables the table-type check: flag `100h` therefore still rejects a
non-table. Rejection writes null before releasing the definition/Effects Lua
references. A nonzero low byte does not manufacture a missing table; native Lua
lookup/error behavior remains applicable.

Retain `definition.Name`, convert it with the existing Lua string accessor, and
construct the first pooled NativeString before allocating the actual 24h owner.
The inline constructor is the previously recovered `00870240..00870279` fragment:
references1, component pointer/count/capacity0, name0, untouched words14/18.
Call its current virtual+8 with the retained Lua definition, then write ID18 and
deep-copy the temporary name to1C/20 through `0086B870`.

Insert `(ID, fresh pointer)` uniquely through the native `0086F930` contract.
An existing equivalent ID keeps its value, including an ID inserted reentrantly
by component loading. The fresh reference is not incremented for the weak map.
The native local is a raw pointer: a losing fresh allocation receives no invented
release or rollback in this reconstruction. The returned existing pointer also
receives no hit-path increment on this miss path.

Release the temporary NativeString, Name Lua reference, definition Lua reference,
and Effects Lua reference in that order. Only THEN read the captured cache node's
current value into output (`00870345..0087034F`). The C++ implementation retains
the standard-map iterator across cleanup and performs this final value reload.
The fixture mutates that value during pooled-string release to make the ordering
observable. Erasing the captured node or invalidating the manager while it is in
use remains invalid storage, not a newly defined recovery path.

`00871B50` consumes ECX=manager, stack output/name-header/flag, returns the output
address and RETCh. A name with length0 writes null without reading its data or
calling the name index. Otherwise execute concrete `00871750` followed by
concrete `008700E0`. `00871BA0` consumes ECX=output, EDX=name, one stack flag,
returns output and RET4 at `00871BC7`. It captures the two addresses before the
concrete `004C1650` singleton getter, then forwards through `00871B50`.

## Reused library contracts and Ghidra corrections

- `0086F930`: signed unique insertion into the 18h-node integer/pointer map.
  Native result is owner/node iterator plus inserted byte+8; EAX=result, RET8.
  Search precedes allocation, and duplicate keys preserve the prior pointer.
- `00869990`: checked iterator predecessor, ECX=mutable owner/node pair; RET.
  End moves to maximum, otherwise walk left/right descendants or ancestors.
  Restored its explicit ECX pointer prototype, so decompilation can see that it
  changes the local iterator. The one-argument `__fastcall` analysis model records
  register placement; it does not recover the original source-language convention.
- `0086EBE0`: ordinary node insertion, count increment and red-black rebalance;
  ECX=map, four stack inputs, EAX=output iterator, RET10h at `0086EDC9`.
  Only count>=1FFFFFFEh takes the length-error branch. Removed its incorrect
  whole-function `STL_xlen_throw` inventory tag/bookmark, preserving prior values.
- `0086AB70`: allocate18h, write links0/4/8, signed keyC, raw pointer10,
  supplied color14 and nil15=0; padding16/17 stays untouched. Five stack inputs,
  EAX=node, RET14h. The pointer copy performs no reference operation.

The canonical existing `std::map` is reused; no STL pseudocode is compiled.
Native checked-iterator validation, huge-container limits, allocation failure,
exception layout, native SEH and binary calling compatibility remain boundaries.
No flow override or function body was changed in this acquisition packet.

## Startup integration and validation

`004DC6A0` now calls concrete `00871BA0`; its former whole-acquisition host method
is removed. `GlobalSubsystemContext::effect_acquisition` supplies the shared
manager publication/domain, string storage and actual Lua/component dispatch.
The static name-index binding must be installed through its existing setup API.
The startup vector append and zero-reference virtual dispatch remain required
services. Other consumers, including the warning owner, still have their prior
acquisition binding and can be connected independently.

MSVC Win32 Release and both existing tests passed. One focused acquisition fixture
uses real Lua 5.1.1 and actual definition bytes to check low-byte flag handling,
temporary cleanup before output publication, cache-hit atomic retain, reentrant
duplicate insertion and concrete name wrappers. The existing startup fixture was
updated to use concrete name lookup/manager cache hits and passed with its eight
Lua file loads and reference-order assertions. Test and library provenance is in
`reports/gameplay_effect_acquisition.json`. This is not gameplay or native ABI proof.

## Follow-up packets

- Component loader `00870400`, its 13 constructors/readers, and current virtual
  dispatch. Sound reader `0086EF60` depends on `00868BF0` and sample acquisition
  `00A83FD0`; tracer uses a secondary pointer at+80.
- Startup reference-vector append `004D9C00`: its begin/end/capacity-pointer
  layout differs from the definition's pointer/count/capacity component array.
- Bind remaining consumers such as warning initialization to concrete acquisition.
- Stored cleanup-body extents remain incomplete; see `GHIDRA_BODY_EXTENTS.md` for
  the actual bridge gate rejection and the four evidence-backed end addresses.
