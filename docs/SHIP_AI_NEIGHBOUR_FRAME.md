# Neighbour frame preparation and refresh: 009F0EA0

Packet `orch6_neighbour_refresh_m` adds `ship_ai_neighbour_frame_refresh_009f0ea0`
over borrowed controller/unit/settings fields and the existing obstacle-node,
motion and box-host types. It covers the complete normal frame preparation,
ageing, ordered refresh and compaction sequence. It does not implement raw
controller ABI, observer destruction, or new versions of the box algorithms.
The older `ship_ai_neighbour_list_refresh_009f0ea0` remains an ageing-only
projection in `src/ship_ai_sector_scan.cpp`; runtime replacement is separate.

Base main was `87f9a9e0`. The authorized incoming L dependency `ce18a0d6` was
merged as `dbd407e1`, retaining both independent startup registrations. Fresh
nodes use that constructor-field projection once; this frame routine preserves
their produced state and never reconstructs them per call.

## Native boundary and source coverage

| Entry | Native body | Coverage |
|---|---|---|
| 009F0EA0 | 009F0EA0..009F115D, 702 bytes, 188 instructions | Complete normal frame sequence over borrowed semantic fields; external callbacks explicit |
| 009EAE20 | 009EAE20..009EAFB4 | Existing near-box projection called directly |
| 009EAFC0 | 009EAFC0..009EB651, with separately defined 009EB4D1..009EB610 arc arm | Existing avoid-box projection called directly; its documented math limits remain |
| 0064A610 | 0064A610..0064A667 | Destructor body inspected; execution supplied by caller's actual lifetime binding |

Native ABI is ECX=controller, one float stack argument dt, RET 4 at 009F115B.
The only caller is 009F51E4 in 009F50E0..009F5253. EDI there is the navigation
controller, and the caller pushes a float-spilled dt at 009F51DA..009F51E1.
The following chain calls are 009E04E0 at 009F51F3 and sector refresh 009EF230
at 009F51FA. The frame function has nine direct call sites, one virtual call,
and zero flow gaps.

The source returns the existing `ShipAiNeighbourRefreshResult` for inspection;
the original returns no specified result. The borrowed list view keeps stable
slot backing and a live signed count. Native backing is the 128-pointer inline
array at +608; no validation, count clamp, slot clearing, or reallocation is
invented. Consumers must supply native-valid storage and lifetimes.

## Exact preparation order

Preparation runs even if the initial neighbour count is zero.

1. Capture self unit from controller+3FC. If its byte +C8 is zero, call
   00414DB0 on that captured unit. This existing pose service recursively
   refreshes parent/world transforms and sets the cache byte.
2. Reload controller+3FC for velocity dispatch, but read world XZ from the
   **previously captured unit's** +FC/+104. Call the reloaded unit's virtual
   +34, then read returned X and Z; ignore Y. The concrete 00CFC3D0 table uses
   00812090, which returns the caller's three-float buffer, RET 4, filled from
   body axis +94/+98/+9C times body-axis speed. The host preserves actual
   virtual dispatch; no single concrete target is assumed for every unit.
3. Compute squared XZ length with the original x87 multiplies, addition and
   float32 spill. Compare the spilled word against the exact double at
   00CE3820 (`kShipAiSectorLengthEpsilon`). Ordered greater takes the existing
   ST0 sqrt service 00BF7030, with its result spilled, reloaded and spilled
   again. Less/equal or unordered produces +0.
4. Reload self unit and capture its class pointer +538 **before** calling
   00424C40. After that call, read the captured class's +500 and the returned
   settings owner's +1B8. Multiply and spill to float32. These are real
   class top speed and `ShipAvoidance.NearbyShip_MyMinSpdRatio`, not a cap or
   fabricated minimum speed.
5. Preserve the original COMISS/FCOMI branches. A magnitude strictly below
   float 00D7A23C (the existing 0.001f constant) zeros both velocity components.
   Otherwise, scale upward only if the spilled floor is ordered greater than
   magnitude. Divide floor by magnitude, spill the gain to float32, then
   multiply and spill each original velocity component. Unordered floor
   comparisons preserve the input pair. There is no general finite filter.
6. Reload self unit for extents. Produce full length * the exact widened
   0.55f double at 00CEC8F0 and full beam * double 0.75 at 00CEC9D8, each with
   the native x87 store. The latter argument is unread by 009EAFC0 but is
   still produced. Reuse existing constants; do not substitute half extents.

The assembly fragments in the new source preserve the relevant arithmetic
instructions and float spills. Added source frames hold explicit pointers and
the borrowed CRT access; they are not native calling-convention replacements.

## Lifetime, filter and box sequence

For each slot, subtract dt on x87, spill to float, reload and store lifetime
before comparing against zero. Only ordered-positive lifetime survives. The
expired path calls 0064A610, then 00BF65AC, then increments the removed count.
The native destructor first sets its base vtable, unregisters a nonnull owner
with 006952A0, and destroys callback-owner storage with 00695870. It does not
free the allocation; the following cdecl free call does, with ADD ESP,4.
The source requires both lifetime operations through explicit host methods.

A live node whose owner is null or whose actual owner+5E is nonzero gets
no_pose_68=true; no_arc and geometry remain unchanged. For other survivors:

- Synchronize the existing semantic owner-deleted cache at the near routine's
  own native entry read, and call the existing near refresh. It receives the
  same persistent node/motion owner; it does not clear validity flags.
- Capture controller+3F0's signed party value, then read observed owner+54
  **even when the captured party is negative**. Negative skips later filter
  calls. Otherwise, reload self unit, call 0080E160, and read its returned
  director's byte +241. A zero byte skips the settings lookup. Nonzero reads
  a fresh 00424C40 result and tests its raw byte +4. Only then compare parties:
  observed party 3 accepts immediately; otherwise reload controller+3F0 and
  accept if it is 3 or equals the observed value.
- Copy current cached self minimum Y from +1C0 before maximum Y from +1BC,
  using the native x87 copy behavior. Pass those, the captured self position,
  floored velocity, both produced extents, and the filter's canonical low byte
  to the existing avoid refresh. Synchronize the semantic owner-deleted cache
  at that routine's own entry read as well.
- If any earlier node was removed, copy the surviving pointer to index minus
  removed count. Reload the live count after callbacks for the loop bound.
  At the end subtract removed count from the current count only if positive.
  Trailing pointer slots remain stale, as in the native array.

`owner_gone_5e` is a semantic cache of the actual owner's byte, not a new native
node store. Pure view/binding accessors expose stable fields and perform no
world updates. Actual callbacks may change the referenced class, settings,
party and count; the reads above preserve the native ordering. Replacing or
reallocating the list backing during a callback is outside the native inline
array contract.

## Corrections and remaining boundaries

The L assessment incorrectly called the party-filter source “class+241”.
0080E160..0080E166 is `MOV EAX,[ECX+738]; RET`: this is the unit's **director**,
not class+538. Therefore the filter reads director+241. The L initializer code
is unaffected; root owns correction of L's documentation/report during merge.

The prior ageing-only helper does not prepare frame inputs or invoke either
box refresh. It must not be counted as this full sequence merely because it
has the same native address suffix. This packet leaves that existing helper
and its runtime call site unchanged.

Both box implementations retain their existing evidence limits. In particular,
native 009EB2B5/009EB2B7 proceeds for unordered shrink, while existing C++
`if (!(shrink > 0.0f))` takes the no-pose arm. This separate discrepancy is
documented without changing unowned box code. No substitute model bounds,
settings values, heading, velocity, observer lifetime, or validity flag is
introduced by the frame routine.

The runtime adapter still needs actual class/settings storage, unit virtual
dispatch and pose owners, cached self Y bounds, observed model bounds and the
remaining existing near/avoid host services. It must also supply real node
destruction/free and retain L's initialized node/motion owners. No gameplay or
installed-mission execution is claimed by this core packet.

## Focused original-byte proof

Win32 Release and both existing CTests passed. The ignored fixture checks 11
cases and **394/394 canonical words**: callback order and unit identities,
all avoid argument words, lifetimes, validity gates, count and stale slots.
It covers empty-list preparation, mixed finite/NaN expiry and gone owners,
velocity scaling, sub-threshold velocity, NaN input/floor and overflow,
director/settings short circuits, self/class/party mutation across callbacks,
and a live-count reduction. Native/source x87 TOP remains balanced.

All 702 original frame bytes match the installed PE and live Ghidra. Four
absolute constant operands are relocated into the fixture image. Nine direct
calls and the velocity virtual slot bind to explicit fixture services shared
by the native and source runs. The source probe supplies link-time fixture
replacements for the two box routines; the production library implementations
are unchanged. These callbacks record arguments, they do not implement fake
production geometry. Destructor/free callbacks record lifetime order over
fixture-owned storage; they do not prove observer lifetime or allocator parity.

Both sides use the existing recovered ST0 sqrt service. Its dispatch word is
the verified initial PE virtual-data zero-fill at 0109DD78, not a guessed live
game runtime setting. Exception handling is unreachable in these nonnegative
sqrt cases; an unexpected callback aborts the fixture. Tests use masked
exceptions, 53-bit x87 precision and nearest rounding. Other control modes,
exception delivery, all registers/status flags, independent box geometry,
actual observer teardown and gameplay are excluded.

```powershell
./scripts/build.ps1
python local/prepare_neighbour_frame_probe.py
cmd /c local\build_neighbour_frame_probe.cmd
./local/neighbour_frame_probe.exe
python tools/verify_report_calls.py reports/ship_ai_neighbour_frame.json
```

Preparation needs the configured installed PE, read-only Ghidra connection,
and Python pefile/capstone. Probe compilation uses VS 2026 vcvars32, /O2
/fp:strict, current bsp_core.lib and /MANIFEST:EMBED. Exact sources, listings,
original bytes, relocation manifest, records and logs are hashed in the report.
