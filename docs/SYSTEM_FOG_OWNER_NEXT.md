# System fog owner: bounded next packet

Read-only discovery, 2026-09-10. The camera's `+184h` slot points to a
`94h`-byte refcounted object constructed by `00B84E50`, with vtable
`00D63180`. This is the owner projected by `SystemFogState`, including the
existing `BSP_AmbientLight_GetColor` getter. **FogOwner / AmbientLight remain
semantic hypotheses, not recovered class names.** `00B84A90` is unrelated
render-texture work and is excluded.

The core initialization and ownership operations are ready for a bounded
implementation packet. They do not establish complete scene construction or
replace the current diagnostic inputs: directional colors are deliberately
unwritten by the native constructor and real values arrive through the
separate environment producer described below.

## Constructor and initial values

`[00B84E50,00B84F61)` is a complete 273-byte constructor body. Input is
ECX = object; EAX returns the same object; plain `RET`, no stack arguments.
The assembly resolves the decompiler's incorrect `void` return and overlapping
constant globals. There are no calls. It briefly writes base vtable
`00CEB130`, initializes refcount `+04h` to 1, then installs `00D63180`.
Both float4 records at `+08h` and `+18h` become positive-zero bits.

| Owner offset | Initial float32 | Exact bits | Constant address | Setter |
| --- | ---: | --- | --- | --- |
| 68 | 0.4 | 3ECCCCCD | 00CE7804 | 00B84D00 |
| 6C | 200 | 43480000 | 00CE386C | 00B84D10 |
| 70 | 7000 | 45DAC000 | 00D63188 | 00B84D20 |
| 74 | 200 | 43480000 | 00CE386C | 00B84D30 |
| 78 | 0.4 | 3ECCCCCD | 00CE7804 | 00B84D40 |
| 7C | -120 | C2F00000 | 00CE77F8 | 00B84D50 |
| 80 | 800 | 44480000 | 00CE3950 | 00B84D60 |
| 84 | 0.97 | 3F7851EC | 00CE77E8 | 00B84D80 |
| 88 | -200 | C3480000 | 00CE77E4 | 00B84DC0 |
| 8C | -140 | C30C0000 | 00CE77E0 | 00B84DE0 |
| 90 | 0.995 | 3F7EB852 | 00CE77DC | 00B84E00 |

Decimal values are display approximations where float32 cannot represent
them exactly. Every constructor constant was compared with the installed PE.
`00D63188` is the 7000 constant immediately after the two-entry vtable; it is
not a third virtual function.

**The constructor does not write `[+28h,+68h)` at all.** Do not invent four
zero directional records, copy `MaterialLighting`, or claim value-initialized
C++ storage models the native allocation. An in-place initializer can preserve
supplied backing bytes there; normal consumers need an established directional
producer before those values can be considered initialized.

## Setter and lifetime contracts

The two color setters are `[00B84C40,00B84C5E)` and
`[00B84C70,00B84C8E)`: ECX object, one source pointer on the stack, `RET4`.
They copy four integer words in forward read/store order. The directional
setter `[00B84FA0,00B84FC8)` takes source pointer then index on the stack,
returns with `RET8`, and copies to `object+28h+16*index` without bounds checks.
The eleven scalar setters in the table copy their stack argument using
`MOVSS` and return with `RET4`; they do not themselves perform x87 conversion.
Exact complete extents and hashes are in the report.

`[00B71940,00B71981)` is the camera owner setter: ECX camera, stack owner,
`RET4`. Its order is significant:

1. Capture old `camera+184h`; identical old/new pointers return immediately.
2. Publish the new pointer at `00B71951`.
3. If new is nonnull, `InterlockedIncrement(new+4)`.
4. If old is nonnull, `InterlockedDecrement(old+4)`; on zero call old vtable+0.

The owner vtable contains `00BD30E0` at +0 and `00B84F70` at +4.
The already named `BSP_RefCounted_InvokeDeletingDestructor` at
`[00BD30E0,00BD30EE)` calls vtable+4 with flag 1; it does not decrement the
count itself. `[00B84F70,00B84F94)` is the deleting destructor: ECX object,
stack flags, `RET4`, EAX = original address after the normal path. It installs
the owner vtable, calls `BSP_RefCounted_DestroyBase` at
`[00BD30F0,00BD30F7)` to restore `00CEB130`, and calls `_free` (`00BF65AC`)
when flags bit 0 is set. No field owns additional allocations.

The Ghidra listing omits `[00B84F8B,00B84F8E)` after `_free`; disk disassembly
and matching Ghidra memory show `ADD ESP,4`. The free path falls through to
the same EAX/return sequence. Do not preserve a false no-return interpretation.

Camera construction `[00B71A80,00B71CDC)` writes null to `+184h` at
`[00B71AE3,00B71AE9)`, with ESI camera and EBX zero. Its other allocations,
scene-node construction and renderer queries are outside this packet.
Camera destruction `[00B71F10,00B71FD2)` releases the slot in
`[00B71F68,00B71F8A)`: ESI camera, EBX zero, EBP `InterlockedDecrement`, EDI
scratch. It decrements/releases while the old pointer remains published,
then clears `+184h` after the virtual callback returns. This differs from the
setter's publication order. `00B71FE0..00B71FFF` wraps the full camera
destructor and scene-node deallocation; reconstructing it is unnecessary for
an isolated fog-slot release fragment.

## Real allocation, publication and field producers

The world path inside `BSP_Game_ConstructWorld` uses
`[004DF6A3,004DF7A5)`:

- `004DF6A3` requests `94h` bytes through `00BF681B`; `004DF6C5` constructs
  them and retains the result in EBP.
- If `game+19E8h` is nonnull, `004DF6E2` calls `00BBDF20` to publish a counted
  reference in that owner's `+10h` slot. This setter is the same store,
  increment-new, release-old pattern and spans `[00BBDF20,00BBDF5B)`.
- `004DF6EE` publishes the object to the camera at `game+19FCh`, followed by
  release of the allocation's temporary reference at `004DF6F7`.
- `004DF70E` and `004DF71B` replace scalar68 and scalar78 with zero.
- `004DF771` sets color08 to the three stored float32 constants
  `3F25A5A6,3F23A3A4,3F2BABAC` (165/255, 163/255, 171/255 rounded) and
  alpha `0/255`, using an x87 divide by the double at `00CE4B48`.
- When `game+5FCh` exists, four iterations at `004DF786..004DF7A4` copy its
  `+A20h..+A5Fh` records into directional indices 0..3. Absence skips those
  writes; there is no fallback initialization.

This is a candidate factory fragment, not a complete standalone function.
Its external live-ins include ESI game, EBX `InterlockedDecrement`, EDI
the preceding exception-state value, valid parent stack/EH state, and the
camera/world/authored-data owners. Actual native allocation is the
malloc/new-handler/`bad_alloc` wrapper `00BF681B`; host `calloc` is not an
equivalent shortcut. The full world owner lifetime and authored-data loader
remain outside this discovery.

Six direct constructor call sites were found: `004DF6C5`, `0078DB5C`,
`0093CEB7`, `00AC5CAF`, `00BA17FD`, `00BA195E`. GUI construction at
`00AC59A0` allocates `94h`, zeroes scalar68/78 and binds the camera, but its
bounded fog sequence makes no directional writes. `0078DAA0` instead stores
a separate newly constructed owner at environment+`B4h`; do not confuse it
with a camera's current owner.

The complete retained-field update is the independent interior fragment
`[0078D076,0078D180)` in `[0078CFF0,0078D1A9)`:

- EDI is environment state; ESI is camera. It reloads `camera+184h` before
  each setter invocation; no null checks or pointer recapture shortcut.
- Environment `+34h` supplies color08, `+44h..+83h` supplies four directional
  records, and `+84h` supplies color18.
- Scalar environment offsets map to owner offsets as recorded in the JSON
  table. Scalars pass through `FLD/FSTP` before the `MOVSS` setters, so masked
  signaling NaNs, x87 status and alias/read order need to be preserved.
- The preceding water/sky/clear-color work and trailing `00BA0090` call are
  outside the fragment. EBX/EBP are loop scratch and are not independently
  preserved at this interior boundary.

The source-copy fragment `[0078CAA4,0078CCE0)` in
`[0078C9B0,0078CF16)` takes ESI authored block, EDI environment state. It
copies authored `+80h` to environment+34h, `+90h..+CFh` to environment+44h,
and `+118h` to environment+84h; it also reads eleven scalar fields through
x87. It then updates the private owner at environment+B4h with color08,
directional records and scalars. It does not set that private owner's
color18 in this fragment. The authored loader and environment updates are
named-but-incomplete dependencies; this discovery does not infer constant
values for them.

## Proposed disjoint implementation work

Packet `system_fog_owner_core` is ready for review: claim the constructor,
14 setter leaves, deleting destructor, camera setter, world setter and the
two camera slot fragments listed above. Create only
`include/bsp/system_fog_owner.hpp`, `src/system_fog_owner.cpp`,
`docs/SYSTEM_FOG_OWNER.md` and `reports/system_fog_owner_audit.json`;
the integrator owns changes to existing camera/SystemFogState interfaces,
build registration and shared ledgers. Existing getter bodies stay with the
integrated prefix work. Preserve exact initialized versus unwritten fields,
intrusive reference operations and callback-visible publication order.
Any new C++ storage/calling convention must be labeled as such; it is not a
drop-in native constructor.

Packet `system_fog_environment_apply` can follow the core interface contract:
claim only `[0078D076,0078D180)` and new
`include/bsp/environment_fog_apply.hpp`, `src/environment_fog_apply.cpp`,
`docs/ENVIRONMENT_FOG_APPLY.md`, `reports/environment_fog_apply_audit.json`.
It consumes established environment storage and the live camera owner slot;
the interface dependency must be agreed before parallel implementation.
It does not own setters or complete `0078CFF0`.

Keep the authored-copy and world-factory fragments as separate bounded
follow-ups. Do not claim full world, environment, camera, allocation/EH or
renderer reconstruction by filling their unresolved calls with stubs. The
JSON records exact dependencies, candidate address extents and readiness.

## Evidence boundary

All live batches used `bsp.py ghidra`, which verifies the existing `bsp`
project, `/battlestationspacific.exe`, language and image base before queries.
Project file: `C:/Users/sqz269/bsp.gpr`. Thirty-nine selected spans, totaling
1,923 bytes, matched the read-only installed PE, including complete core
bodies, producer/factory fragments, constants and vtable. Per-span SHA-256
values and installed image hash are in `reports/system_fog_owner_next.json`.

No Ghidra annotations, function definitions, ledgers, production code or game
files were changed. Selected raw exports were refreshed/read through the
shared ignored export location. This packet is analysis and byte verification,
not reconstructed, build-tested, fixture-tested, ABI-compatible or game-validated.
