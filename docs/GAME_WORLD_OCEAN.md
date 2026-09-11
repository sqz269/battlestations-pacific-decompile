# Ocean, rain and effect manager in GGame::OnMove

Addresses: 004B6260, 004BCAA0, 004D1100, 00801400, 00865AB0, 00865B80, 00865CF0, 00866C60,
008671A0, 00867790, 00867D00, 00867EE0, 00BBD310, 00BBDDD0, 00BBE8B0, 004CF700, 004F9B30,
00419440, 00419510, 00B6DA70

Packet: `game_world_ocean_effects`. Anchor `BSP_Game_OnMove` at 004E4A40 (read only; see
`docs/GAME_ON_MOVE_MAP.md`). Reconstruction in `include/bsp/world_ocean.hpp` and
`src/world_ocean.cpp`. Machine-readable facts in `reports/game_world_ocean.json`.

All call-site facts below come from `disasm-raw 004E4A40 --length 2808`; the callee facts come
from the Ghidra listing, not from pseudocode alone, wherever x87 or register inputs are involved.

## Correction to the packet contract

The contract described the one-shot at `game+0x1EE7` as the step that "arms" the ocean and effect
updates. It does not. 004B6260 reads the network session object at `game+0x1EF0` and 004BCAA0
adjusts two integer counters at `game+0x740` and `game+0x73C` through the subobject at
`game+0x650`, then notifies through 007FA1B0 (segment 49, keywords `unlocks`,
`selectedmissionid`, `savedlobbyfilters`). Nothing in either routine touches `game+0x19E8`,
`game+0x19FC`, the ocean or any effect. The latch is a once-per-mission session statistic that
happens to live in the same state-0x0D block. The ocean and effect updates are gated only by
`game+0x19E8 != 0` and by the simulation gate; they need no arming.

## Call sites in the game update

### One-shot 004E4E00-004E4E2A

Inside the `game+0x5D4 == 0x0D` block that opens at 004E4DF4, before the two effect lists.

```
004e4e00: cmp byte ptr [esi + 0x1ee7], 0     ; already counted -> skip
004e4e09: lea ecx, [esi + 0x1ef0]            ; ECX = network session, not the game
004e4e0f: call 0x4b6260
004e4e14: test al, al                        ; false -> skip the count, still latch
004e4e18: cmp dword ptr [esi + 0x624], 0     ; non-zero -> skip the count, still latch
004e4e21: push 1
004e4e23: mov ecx, esi                       ; ECX = the game object
004e4e25: call 0x4bcaa0
004e4e2a: mov byte ptr [esi + 0x1ee7], 1     ; latched on every path that reaches it
```

The map records `004b6260()` with no receiver. The receiver is `game+0x1EF0`, the same subobject
the network tick 00778560 and the drain loop 00776230 use.

### Ocean and rain 004E52F9-004E5325

Inside the simulation gate, after the seven award-tracker calls and after the entity, marker and
bot-manager updates owned by `game_world_entities`.

```
004e52f9: cmp dword ptr [esi + 0x19e8], 0    ; whole block skipped when null
004e5302: call 0x865cf0                      ; no ECX set: a static getter
004e5307: mov ecx, eax
004e5309: call 0x865ab0                      ; __thiscall, no stack arguments
004e530e: fld dword ptr [esi + 0x21f0]       ; the scaled delta
004e5314: push ecx                           ; stack reservation
004e5315: mov ecx, dword ptr [esi + 0x19fc]
004e531b: fstp dword ptr [esp]               ; arg1 = scaled delta
004e531e: push ecx                           ; arg0 = game+0x19FC
004e531f: mov ecx, dword ptr [esi + 0x19e8]  ; ECX = the ocean owner
004e5325: call 0xbbddd0
```

### Effect manager 004E535F-004E5377

```
004e535f: fld dword ptr [esi + 0x21f0]
004e5365: mov edx, dword ptr [esi + 0x19fc]
004e536b: push edx                           ; arg1 = game+0x19FC
004e536c: push ecx                           ; stack reservation
004e536d: fstp dword ptr [esp]               ; arg0 = scaled delta
004e5370: call 0x4d1100                      ; getter, consumes nothing
004e5375: mov ecx, eax
004e5377: call 0x867ee0                      ; consumes both pushes, RET 8
```

The map records this as `004d1100(scaledDelta, game+0x19FC)`. That is wrong: 004D1100 takes no
arguments. The two pushes belong to 00867EE0, and no `add esp` appears between the calls because
00867EE0 returns with `RET 8`. 004D1100 is a locked lazy singleton getter (it calls
`BSP_SingletonLifetime_GetManager`, `operator new(0x28)`, 004CF700 and
`BSP_SingletonLifetime_Register`, caching into 00F8765C), which is the same shape as 004DE4B0 in
the particle step and as 00865CF0 here.

## Calling conventions and RET sizes

| Address | Convention | Receiver | Stack arguments | RET | State |
| --- | --- | --- | --- | --- | --- |
| 00865CF0 | `__cdecl`, returns `void*` | none | none | `RET` | reconstructed |
| 00865AB0 | `__thiscall` | the 00F875C0 descriptor | none | `RET` | reconstructed |
| 00BBDDD0 | `__thiscall` | `game+0x19E8` | `(void* node, float delta)` | `RET 8` | reconstructed |
| 00BBD310 | `__thiscall` | `owner+0x3C` | `(void* node, float delta)` | `RET 8` | reconstructed |
| 00867EE0 | `__thiscall` | the 004D1100 singleton | `(float delta, void* node)` | `RET 8` | reconstructed |
| 004D1100 | `__cdecl`, returns `void*` | none | none | `RET` | reconstructed |
| 004B6260 | `__thiscall`, returns `bool` in AL | `game+0x1EF0` | none | `RET` | reconstructed |
| 004BCAA0 | `__thiscall` | the game object | `(char add)` | `RET 4` | reconstructed |
| 00BBE8B0 | `__thiscall` | one shore-wave layer | `(node, vec3*, float, float, float)` | `RET 0x14` | analyzed |
| 00801400 | `__thiscall` | descriptor+0x38 | `(float delta, vec3* pos, float* axis)` | `RET 0xC` | analyzed |
| 004F9B30 | `__fastcall(out, a, b)` | out in ECX | `(float* b)` | `RET 4` | reconstructed |
| 00419510 | `__fastcall(out, v)` | out in ECX | none | `RET` | reconstructed |
| 00419440 | `__thiscall`, result in ST0 | the vector | none | `RET` | reconstructed |

Every one of the seven packet addresses has a real Ghidra function body, so the orchestrator can
apply names directly. None needed defining.

## Objects

### `game+0x19FC`, the reference node

Both the ocean and the effect manager receive this pointer and treat it as a scene transform: a
dirty byte at `+0x5C` whose bit 1 gates `BSP_Transform_RefreshWorldMatrix` (00B6DB70), a world
matrix whose row at `+0x110` is an axis and whose row at `+0x120` is the world position. It is the
same object 00AF0C50 receives in the post-simulation step alongside a camera value, and
`docs/APP_INIT_GAME_ENTRY.md` records `BSP_Settings_ApplyAll` pushing `settings+0x54` into it. The
class is not identified; the layout above is what the three consumers actually read. Treating it
as "the world/eye reference node" is a hypothesis.

### `game+0x19E8`, the ocean owner

00BBDDD0 is a thin forwarder: it loads `this+0x3C`, returns when it is null, and re-pushes the two
arguments unchanged for 00BBD310 with ECX set to that pointer. So `game+0x19E8` is an owner and
`owner+0x3C` is the ocean object proper. 00BBDDD0 is the last function of segment 93, whose string
keywords are `oceanheightmap` and `shorewavetexturesource0..2`.

Recovered ocean fields, all from 00BBD310 and 00BBE8B0:

| Offset | Meaning | Evidence |
| --- | --- | --- |
| `+0x18` | shore-wave layer array (`T**`) | `MOV ECX,[ESI+0x18]; MOV ECX,[ECX+EDI*4]` at 00BBD434 |
| `+0x1C` | layer count | loop bound at 00BBD414 and 00BBD457 |
| `+0x30` | scene node hidden when disabled | `MOV ECX,[ESI+0x30]` at 00BBD327 |
| `+0x34` | float passed to every layer as its third argument | 00BBD447 |
| `+0x38` | vec3 passed to every layer as its second argument | `LEA EBP,[ESI+0x38]` at 00BBD425 |
| `+0x44` | right axis of the frame | written at 00BBD3C8-3D6 |
| `+0x50` | up axis of the frame | written at 00BBD39F-3B3 |
| `+0x5C` | float, rewritten to 1.0f every tick | 00BBD417 |
| `+0x90` | int, cleared every tick | 00BBD41C |
| `+0x98` | enabled flag | `CMP byte ptr [ESI+0x98],0` at 00BBD316 |

### `&DAT_00F875C0`, the rain descriptor

00865CF0 is a magic-static guarded by bit 0 of 00F875FC: it runs 00865B80 once, registers an
`_atexit` destructor and returns `&DAT_00F875C0`. 00865B80 installs the vtable at 00D0D3AC, a
native string holding `"raindrop.tga"` at `+0x04`/`+0x08`, a colour `0x80D0D0FF` at `+0x34` and
float parameters at `+0x0C..+0x2C`, and zeroes `+0x38`. 00865AB0 gates on `+0x38`, so `+0x38` is
the live emitter instance created elsewhere and the object at 00F875C0 is the rain descriptor.

### The 004D1100 singleton, 0x28 bytes

Constructor 004CF700 writes the vtable at 00CE789C, sets `+0x08` from 004C3200 and zeroes
`+0x0C..+0x24`. Consumers fix the layout:

| Offset | Meaning | Evidence |
| --- | --- | --- |
| `+0x08` | `std::list` head node | 008671A0 walks `**(this+8)` and compares against `*(this+8)` |
| `+0x0C` | list size | loop guard at the top of 008671A0 |
| `+0x10` | effect array data (`T**`) | `MOV ESI,[ECX+0x10]` at 00867EEB |
| `+0x14` | effect count | `LEA EBX,[EDX+EAX*4]` at 00867EF4, so `+0x14` is a count, not an end pointer |
| `+0x18` | array capacity | inferred from the 0x28 size and the two-vector shape |
| `+0x1C` | second array data | `MOV [param_1+0x1c]` loop in 00866C60 |
| `+0x20` | second array count | same loop |
| `+0x24` | second array capacity | inferred |

The arrays are `{T* data; int size; int capacity}`, not the MSVC three-pointer `std::vector`.

## What each routine does

### 00865CF0 then 00865AB0, the rain step

00865AB0 returns immediately unless `descriptor+0x38` holds an emitter. Otherwise it reads
`game+0x19FC` twice from `DAT_00E188A8`, refreshes the world matrix on each read when
`node+0x5C & 2` is clear, copies the world position from `node+0x120..0x128` into a local, and
calls 00801400 with `ECX = descriptor+0x38` and the arguments
`(game+0x21F0, &worldPosition, node+0x110)`. The delta is fetched by the callee's caller from the
global game object, not passed down from OnMove.

00801400 gates spawning behind `delta * DAT_00D04368 * (DAT_00F87470 * DAT_00D7A220) < random()`,
where the right-hand side comes from 00BD2F10; it then refreshes the same node and requires
`node+0x124 > 0.0` (world Y above zero) before calling 004845D0, 00867B10 and 008687C0. The
combination of the "raindrop.tga" descriptor, a delta-scaled stochastic gate and an
above-the-waterline test reads as the ambient rain emitter. That reading is a hypothesis; the
three spawn callees were not analysed.

### 00BBDDD0 then 00BBD310, the ocean step

Disabled path (`ocean+0x98 == 0`): `00B6DA70(ECX = ocean+0x30, 0.0f, 0)`, which writes the float
into `node+0xAC` and does not recurse because the second argument is zero. Nothing else runs.

Enabled path, in order:

1. Refresh the reference node's world matrix when `node+0x5C & 2` is clear.
2. Load `V = node[0x110..0x118]`, the matrix row used as the reference axis.
3. `t1 = cross(V, up)`, `t2 = cross(t1, V)`, `up = normalize(t2)`. This is Gram-Schmidt: the
   previous up axis is replaced by its component orthogonal to `V`.
4. `t3 = cross(V, up)`, `right = normalize(t3)`.
5. `if (length(up) < 0.5f)` reset `up = (0, 1, 0)` and `right = (1, 0, 0)`. The tested value is a
   normalize result, so it is exactly 1.0f or 0.0f; the branch fires only on degeneracy, when
   00419510 divided nothing because the crosses collapsed.
6. `ocean+0x5C = 1.0f`, `ocean+0x90 = 0`.
7. For `i` in `[0, ocean+0x1C)`: `00BBE8B0(ECX = layers[i], node, ocean+0x38, ocean+0x34, 1.0f,
   delta)`.

Cross-product operand order is from 004F9B30's own body: `out = a x b` with `a` in EDX and `b` on
the stack. The frame step does not consume the delta at all; the delta goes only to the layers.

Constants: `DAT_00D7A24C = 1.0f`, `DAT_00CE3800 = 0.5f`, `DAT_00D7A218 = 0.0f`.

### 00BBE8B0, the shore-wave layer

Not fully reconstructed: 288 decompiled lines reaching material and texture code that belongs to
other owners (`BSP_Material_RegisterFloatParameter`, `BSP_Material_SetTextureSlot`,
`BSP_ShadowTextureOwner_GetTexture`, and the literals `cScale`, `cAlpha`, `cCloudDiffuseColor`).
The part this packet establishes is the delta advance at 00BBEC06-00BBECA7:

```
00bbec06: CALL 0x00bbdd60           ; EAX = direction vec3 on the wave source
00bbec0b: FLD [EAX] / FLD [EBP+0x18]; [EBP+0x18] is the fifth argument, the delta
                                    ; three components multiplied by the delta
00bbec2c: CALL 0x00bbdd70           ; ST0 = speed
                                    ; three components multiplied by the speed
00bbec5d: FLD double [0x00ce7630]   ; 30.0
                                    ; three components multiplied by that
00bbec81: FADD [ESI+0x1b8] ...      ; layer+0x1B8..0x1C0 += the result
00bbecba: FLD [ESI+0x120] ...       ; world position + scroll
00bbecc6: MOV EDX,[EAX+0x30]        ; virtual +0x30 sets the node position
```

So each layer accumulates a world-space scroll offset `scroll += direction * delta * speed *
30.0f` and then places its node at `worldPosition + scroll`. Every intermediate is stored back as
a float; 30.0 is loaded as a double and stored as a float, which rounds once, so a float multiply
by 30.0f is equivalent.

The layer also runs a distance fade before the scroll: it measures the distance between the
layer's world position and the reference node's world position, subtracts a radius from 00BC3010,
clamps `(distance - _DAT_00CF0DD8) / _DAT_00D20198` into `[0, 1]` and drives
`00B6DA70(layer+0x1C4 * (1 - t), 0)` with the result. The scroll block runs only when
`1 - t != 0`, i.e. only for layers that are not fully faded out. Field names for the two globals
were not recovered.

### 004D1100 then 00867EE0, the effect manager

00867EE0 runs four phases on the singleton, in this order:

1. For every non-null entry of the array at `+0x10`/`+0x14`:
   `00867D00(ECX = entry, delta, node)`.
2. `00866C60(ECX = this, delta, node)`.
3. For every entry of the same array: `00867790(ECX = entry, delta, node)`, then a retire test.
4. `008671A0(ECX = this)`.

The guard in phases 1 and 3 is the `NEG`/`SBB`/`TEST reg, 0xE186EC` idiom (and `0xE19AB0` in the
inner loop of 00867790). `NEG`+`SBB` turns the pointer into 0 or -1 and the constant is the
address of a global, always non-zero, so the whole test reduces to "the pointer is non-null". The
decompiler renders it as `(-(uint)(p != 0) & 0xe186ec) != 0`, which is the same predicate.

Retire test, per entry, after 00867790 returns:

- `entry+0x09 != 0` (finished), and
- `entry+0x1C <= 0` (no outstanding work), and
- `entry+0x0A == 0` (not pinned), and
- every pointer in the entry's own array at `entry+0x0C`/`entry+0x10` is null,

then `0081B010(ECX = this+0x10, &iterator)` erases the entry. The iterator is not advanced after
an erase (0081B010 shifts the array down), and the loop reloads `data` and `size` from `+0x10` and
`+0x14` on every pass, which is what makes the erase safe.

00867D00, per effect instance (complete reconstruction and precise ordering now
in [POINT_EFFECT_ADVANCE.md](POINT_EFFECT_ADVANCE.md)):

- If byte `+0x0A` is nonzero, restart gated rows through 00866F50. After restart
  returns, add delta to age `+0x80` through x87 and spill to float.
- If `instance+0x8C` holds an attachment node: when `attachment+0x44 == 0`, refresh its world
  matrix and push `Matrix_Multiply4x4(local, attachment+0xF0)` through virtual `+0x34` of the
  instance's own node at `instance+0x110`. When `attachment+0x44` is set, the attachment died:
  run 0042D9A0(0) and 00867B10, then set `instance+0x09 = 1` to mark the effect finished.
- If either full DWORD `+0x28/+0x2C` is nonzero, compare x87 `timer + delta`
  against interval `+0x4C` before spilling elapsed to float. Only ordered greater
  samples; other outcomes spill elapsed to `+0x48`. Sampling copies previous XYZ
  through x87, captures the current node, refreshes it if needed, and latches its
  `+0x120` translation DWORDs. Derived outputs require signed DWORD `+0x88 > 1`
  and COMISS-ordered positive delta. Velocity uses separately float-spilled
  differences divided by a freshly read, float-spilled `timer + delta` after
  refresh; displacement recomputes its own differences. Successful samples
  reset `+0x48` to positive zero, including when the derived-output gate fails.

00866C60 publishes the two arguments into the globals `_DAT_00F87608` (delta) and `DAT_00F8760C`
(node), then walks the second array at `+0x1C`/`+0x20` and dispatches each entry through
`004C1130()`'s `+0x04` object, virtual `+0x04`, followed by one virtual `+0x08` with argument 1.
This reads as the per-frame draw or queue submission for the effect set; it was not analysed
further because 004C1130's object belongs to the render side.

00867790 walks the entry's own array at `+0x0C`/`+0x10`, calls virtual `+0x28` with
`(delta, node)` on every non-null child whose `child+0x18` is neither 1 nor 4, then releases
children that report themselves finished: virtual `+0x08` returning non-zero, or `child+0x0C`
clear, leads to virtual `+0x30`, 00867320 and an `InterlockedDecrement` on the child's refcount at
`child+0x04`.

008671A0 drains the deferred-destroy `std::list` at `+0x08`: for each node it calls the stored
object's virtual `+0x04` with argument 1 (the MSVC scalar deleting destructor), clears the slot,
unlinks the node and frees it.

### 004B6260 and 004BCAA0, the mission-start latch

004B6260 returns `session+0xF4 != 0 && session+0x29C == 0`.

004BCAA0 returns immediately when `DAT_00F8A2FC` is null or when its virtual `+0x198` predicate is
false. Otherwise, with `add` true it writes `active + 1` and `total + 1`; with `add` false it
writes `active - 1` floored at zero through an unsigned compare against 1 at 004BCAD6 and leaves
the total alone. Both stores go through `ECX = game+0x650` at offsets `+0xF0` and `+0xEC`, which
land on `game+0x740` and `game+0x73C`. It then calls `007FA1B0(ECX = game+0x650, 0, 0x00BD53C0)`.
0x00BD53C0 points into `.text`, so it is a code pointer, not a string; the notification's shape
was not recovered.

## Reconstruction

`include/bsp/world_ocean.hpp` and `src/world_ocean.cpp` provide:

- `OceanVec3` plus `cross_004f9b30`, `length_00419440` and `normalize_00419510`, each following
  the native operand and rounding order. `length_00419440` rounds each square to a float, keeps
  `xx + yy` unrounded as the listing does, and takes the square root of the stored float.
- `ocean_orthonormalize_00bbd310`, the frame step including the degeneracy reset, the 1.0f scale
  write and the cleared layer count.
- `shore_wave_scroll_step_00bbec06` and `advance_shore_wave_scroll_00bbec06`, the only place the
  ocean spends the delta.
- `advance_effect_sample_00867d00`, a sampling-only semantic adapter. It now
  shares exact x87 stages with the complete actual-owner advancement routine;
  restart, attachment, stop and live-node refresh belong to that full routine.
- `session_counts_mission_004b6260` and `adjust_mission_counters_004bcaa0`.
- `WorldOceanHost` with one method per native call site in this slice, plus
  `arm_mission_start_latch_004e4e00` and `run_ocean_and_effects_tick` as the two sequence
  routines. `updates_between_ocean_and_effects` is present only so a host preserves the native
  order of 00740E10, 0094C8F0 and 008EB110, which belong to other packets.

This is build-tested on the Win32 MSVC target with warnings as errors, and one focused case in
`tests/math_tests.cpp` covers the degeneracy reset and the pass-through of an already orthogonal
axis. It is not fixture-tested, not ABI-compatible and not game-validated; the structs project
only the touched fields and the host methods take `void*` because the classes are not identified.

## Uncertainties

- The class of `game+0x19FC` is not recovered. It is used as a transform with a dirty byte at
  `+0x5C` and matrix rows at `+0x110` and `+0x120`; "world reference node" is a description of
  use, not a symbol.
- The rain reading of 00F875C0 rests on the `"raindrop.tga"` literal in 00865B80 and the
  above-water test in 00801400. The three spawn callees were not opened.
- 00BD2F10's range decides whether 00801400's comparison is a probability gate or its inverse.
- `_DAT_00CF0DD8`, `_DAT_00D20198`, `_DAT_00CED0E0` and `_DAT_00CE7630`'s role names in the layer
  fade are not recovered; only 00CE7630's value (30.0) is used here.
- 007FA1B0 and the code pointer 0x00BD53C0 were not analysed.
- `+0x18`/`+0x24` as array capacities is inferred from the object size, not observed.

## What remains

- 00BBE8B0's material and texture half, which needs the renderer owner's types.
- 00866C60's dispatch through 004C1130, likewise render-side.
- 00867790's child virtuals `+0x08`, `+0x28`, `+0x30` and the refcount protocol at `child+0x04`.
- 0081B010, the array erase, treated here as a shift-down erase from its use.
- The identity of `game+0x650`, `game+0x624` and `session+0xF4`/`+0x29C`.
