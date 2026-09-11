# The world's matrix interpolator list (`00904600`, the "timed attachments")

Addresses: 00904600, 00905080, 008ADE00, 009045C0, 00904E50, 004CB030, 0042ED50, 004134F0,
00413920, 00B64640, 00B646E0, 00B64780, 00F876A4 (data).

Packet `cc_world_update`, worktree `agent/cc-unit-tick`. Ghidra read-only. Reconstruction in
`include/bsp/world_entity_update.hpp` and `src/world_entity_update.cpp`; machine-readable facts in
`reports/world_update.json`.

## The name is recovered, the feature is not a "timed attachment"

The packet inherited the working title *world timed attachments* from
`docs/UNIT_INSTANCE_UPDATE.md`, whose follow-up described `world+4B0h` as "the world's deadline
list walked after every entity update". Reading the producer settles what it actually is.

`00905080`'s only caller is `008ADE00`, a Lua binding. That function builds the literal
`luaMW_AddMatrixInterpolator failed:` and is registered in the `MW_` binding table as
`AddMatrixInterpolator` (`src/mission_lua_host.cpp`, `{"AddMatrixInterpolator", 0x008ADE00U}`).
**`AddMatrixInterpolator` is a recovered name**, from the shipped string and the shipped table, not
a hypothesis. The list is a list of matrix interpolators a mission script registers on an entity;
each one animates that entity's local transform from a captured pose to a pose offset over a
duration, then retires itself.

The `BSP_World_`/`BSP_Lua_` prefixes on the three ledger names are this repository's convention.

## The container

`world+4B0h` is an MSVC `std::list` compiled with `_SECURE_SCL` on, already recorded in
`include/bsp/world_construct.hpp`:

| Field | Offset | Evidence |
| --- | --- | --- |
| `_Myfirstiter` | `+4B0h` | `004CB07B LEA EDI,[ESI+4B0h]`, the ctor's ECX |
| `_Myhead` | `+4B4h` | `004CB091`, the sentinel `004C3080` returns |
| `_Mysize` | `+4B8h` | `004CB094` writes 0; `00904BCD ADD [EDI+8],-1` |

Node layout is `{_Next +0h, _Prev +4h, value +8h}`. **The node is `6Ch` bytes and the value is
`64h`**, established twice and independently:

* `009045C2 PUSH 6Ch` in the buy-node `009045C0`.
* `00904E70 MOV ESI,28F5C28h` in `00904E50`, the `_Incsize` overflow guard. That is
  `0xFFFFFFFF / 100`, and VC9's `list::max_size()` divides by `sizeof(value_type)`, so the value
  type is exactly 100 bytes.

The record is stored **inline in the node**, not behind a pointer: `00904659` reads the entity as
`[node+8]` and `009051CF` writes the start time at `[node+68h]`, which is `record+60h`, the last
dword of a `64h`-byte record.

## The record, from its producer `00905080`

`void __thiscall(world, entity, float tx, float ty, float tz, float rx, float ry, float rz,
float duration)`, `RET 20h` at `009051DA`, body `00905080..009051DC`. **Eight stack dwords**, taken
from the `RET` immediate (`20h / 4 = 8`), not from the pushes at the call site.

The node is bought and spliced in front of the head sentinel, that is, appended at the back
(`00905092..009050B4`), `00904E50(this, 1)` raises `_Mysize`, and then every field is written
through a freshly re-read `_Myhead->_Prev`:

| Record offset | Node offset | Type | Meaning | Write site |
| --- | --- | --- | --- | --- |
| `+00h` | `+08h` | ptr | the entity being animated | `009050D3` |
| `+04h` | `+0Ch` | float | translation offset x | `009050F8` |
| `+08h` | `+10h` | float | translation offset y | `00905103` |
| `+0Ch` | `+14h` | float | translation offset z | `00905111` |
| `+10h` | `+18h` | float | rotation about x, radians | `00905138` |
| `+14h` | `+1Ch` | float | rotation about y, radians | `00905146` |
| `+18h` | `+20h` | float | rotation about z, radians | `00905154` |
| `+1Ch` | `+24h` | float | duration, seconds | `00905178` |
| `+20h..+5Fh` | `+28h..+67h` | float[16] | the entity's local 4x4 captured at registration | `0090519D` |
| `+60h` | `+68h` | float | `DAT_00F876A4` at registration | `009051CF` |

The captured matrix comes from `entity+74h`: `00905196 ADD EBX,74h` on the entity pointer (EBX was
reloaded from the argument slot at `009050CF`), then `004134F0(node+28h, entity+74h)`, the x87
4x4 copy. `00955970`, the `MDestroyer` target of vtable slot `0D8h`, reads the same 16 dwords from
`entity+74h` (`009559C8 LEA ESI,[EBX+74h]; MOV ECX,10h`), so `entity+74h` is the entity's own local
transform.

Which vector is which is settled at the Lua call site, not by guessing. In `008ADE00` the four Lua
argument slots are taken in the order 3, 2, 1, 0 (`00B677E0` at `008ADEF7`, `008ADF11`, `008ADF2B`,
`008ADF44`); slot 3 is read as a number and slots 2 and 1 as vector3. At `008ADF92` the stack from
low to high is entity, slot 1's vector, slot 2's vector, the number. Matching that against
`00905080`'s argument slots gives the Lua order

```
AddMatrixInterpolator(entity, translationOffset, rotationOffset, durationSeconds)
```

## `00904600`, the pass

`void __thiscall(world, float)`, `RET 4` at `00904BE0`, body `00904600..00904BE2`, 320 listed
instructions. Sole caller `00904C2B`.

**The float argument is dead.** The frame is `SUB ESP,2E8h` plus four pushes, so the argument sits
at `ESP+2FCh`, and no instruction in the body reads it. Everything is timed off `DAT_00F876A4`,
loaded once per iteration at `0090462F`.

Per record, in order:

1. `elapsed = DAT_00F876A4 - record.start_60h` (`0090464B FSUB [ESI+68h]`).
2. **First gate**: `record.entity->byte_5Ch` (`0090465C`). When clear, skip straight to step 8.
3. `phase = (elapsed) / record.duration_1Ch` (`00904674 FDIV [ESI+24h]`), then two clamps:
   `00904681 FCOMIP` of `0.0f` against the quotient with `JBE` to the upper clamp, and
   `00904B75 COMISS` against `1.0f` (`00D7A24C`) with `JBE` keeping the quotient. Both `JBE`s are
   taken on an unordered compare, so a NaN quotient survives both clamps rather than becoming
   `0.0f`.
4. Build four `64h`-byte matrices on the stack:
   * a translation matrix, identity with row 3 set to `phase * (record+4h, +8h, +0Ch)`
     (`009046EB`, `00904721`, `00904751`, stored at `[ESP+0B8h]`);
   * `00B64640(-(phase * record+10h))`, an x rotation (`009047B1`, angle negated by the `FCHS` at
     `009047AB`);
   * `00B646E0(-(phase * record+14h))`, a y rotation (`0090488F`);
   * `00B64780(-(phase * record+18h))`, a z rotation (`00904997`).
   All three builders are `void __fastcall(float* dst /*ECX*/, const float* angle /*EDX*/)` and
   write a row-major 4x4 whose negated entry is computed as `-0.0f - sin` (`00D7A208`).
5. Four chained `00413920` calls at `00904AC3..00904AD8`. `BSP_Matrix_Multiply4x4` is
   `ECX = left`, stack `(destination, right)`, `RET 8`, result in EAX, so with the eight pushes at
   `00904A77..00904AAC` unwound the chain is

   ```
   out = ((((RotZ * RotY) * RotX) * Translate) * record.base_20h)
   ```

   with the destination of each call becoming the left operand of the next (`MOV ECX,EAX`).
6. `entity->vtable[88h](out)` (`00904AE3`, `ECX = EBX`, the entity loaded at `009046A1`).
7. Invalidate the pose cache: `entity+0C8h = 0`, `entity+10Ch = 0`, then `0042ED50` on every child
   from `entity+48h` linked through `+44h` (`00904AEF..00904B13`). This block is `0042ED50`'s own
   body inlined for the root. Then `entity->vtable[0D8h]()` with no argument (`00904B2A`).
8. **Second gate**, a fresh read of the same byte (`00904B39`). The refresh in step 7 ran a virtual
   on the entity between the two reads, so they are not collapsed. Keep the record when the byte is
   set **and** `elapsed <= record.duration_1Ch` (`00904B49 FLD [ESI+24h]`, `FLD [ESP+18h]`,
   `FCOMIP`, `JA` to the erase). Otherwise erase.
9. Erase: take the successor first (`00904B97 MOV ESI,[ESI]`), unlink
   (`00904BB3..00904BC2`), `_free(node)` (`00904BC5`).

### The 12 bytes Ghidra does not disassemble

`python tools/bsp.py ghidra flow 00904600` reports one gap: `00904BCA..00904BD6`, after the `_free`
call, because Ghidra's non-returning discovery marks the CRT free helper `00BF65AC` as
`CALL_RETURN`. The decompiler therefore shows the erase path ending in `return`, which would mean
at most one record retires per frame. The disk bytes say otherwise:

```
00904bca  83 c4 04        ADD ESP,4              ; free's cdecl cleanup
00904bcd  83 47 08 ff     ADD dword [EDI+8],-1   ; _Mysize -= 1, EDI = world+4B0h
00904bd1  e9 42 fa ff ff  JMP 00904618           ; back to the top of the walk
```

So the pass **continues after an erase** and any number of records can expire in one frame, and
`_Mysize` is maintained there and nowhere else in this routine. The integrator should run
`python tools/ghidra_flow_repair.py 00904600 --apply` before this function is re-exported.

## What the feature does, in one line

A mission script names an entity, a translation offset, a rotation offset and a duration. For the
next `duration` seconds the world rewrites that entity's local transform every frame as the
captured pose pre-multiplied by an offset that ramps linearly from none to the full offset, marks
the entity's subtree pose cache dirty and calls its refresh virtual; then the record is dropped and
the entity keeps the final pose. The rotation angles are applied **negated**, in the composition
order z, then y, then x, then the translation.

## Reconstruction

`bsp::run_matrix_interpolator_pass_00904600` sequences the pass over a
`std::vector<MatrixInterpolatorRecord>` and a `MatrixInterpolatorHost` with one pure virtual per
native call site. `bsp::matrix_interpolator_compose_009046b5` calls
`multiply_native_camera_matrices_00413920`, the byte-exact `00413920` port in
`src/native_camera_matrix_math.cpp`, so the four multiplies keep the native x87 schedule rather
than being re-derived. `bsp::matrix_interpolator_phase_00904670` keeps the native branch shape,
including the unordered-compare behaviour. `bsp::add_matrix_interpolator_00905080` is the producer.

Nothing here is a drop-in binary replacement: the records are a C++ projection, not the native
node, and the entity is an opaque handle.

## Corrections

### To `docs/UNIT_INSTANCE_UPDATE.md`

| Was | Is | Evidence |
| --- | --- | --- |
| "`00904600` ... walks a `std::list` at `+4B0h` with `DAT_00F876A4` **deadlines**" | The record stores a **start time** at `+60h` and a **duration** at `+1Ch`; the clock is compared against `start + duration` | `009051CF` writes the clock at registration; `00904B49` compares `clock - start` against `+1Ch` |
| The follow-up packet named it `world_timed_attachments`, "the world's deadline list" | It is the matrix interpolator list registered by the Lua binding `AddMatrixInterpolator` | the literal at `008ADE00` and the `MW_` registration table |
| "`00904600`'s per-attachment work was not read past its head" | Read in full, including the flow gap | this document |

### To the decompiler output of `00904600`

Ghidra's pseudocode ends the erase path with `return`. It does not: the undisassembled bytes at
`00904BCA` jump back into the walk. Any reader of the pseudocode alone would conclude that one
record retires per frame and that `_Mysize` is never decremented; both conclusions are wrong.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `entity_local_matrix_slot` | 00CFC3D0+88h, entity+74h, 004134F0 | docs/ENTITY_LOCAL_MATRIX.md | The slot `88h` setter's body, and every other writer of `entity+74h` |
| `entity_refresh_virtual` | 00955970, 00CFC3D0+0D8h | docs/ENTITY_REFRESH_VIRTUAL.md | Slot `0D8h`: it reads `entity+4A4h` and `entity+3Ch`, refreshes the pose block at `+0CCh` through `00414DB0` when `+0C8h` is clear, and calls the scene node's vtable `34h` with `entity+0CCh`. No Ghidra function exists at `00955970` |
| `world_matrix_interpolator_lua` | 008ADE00, 00888760, 00888AA0 | docs/WORLD_MATRIX_INTERPOLATOR_LUA.md | The binding's failure path and what `00888AA0` accepts as an entity table; which mission scripts call it |

## no_ghidra_function

none for the addresses this packet names. `00955970`, listed in the follow-up above, has no Ghidra
function and is **not** named here.

`00904600`'s Ghidra body `00904600..00904BE2` contains the undisassembled range
`00904BCA..00904BD5` described above. The boundary evidence is the `CALL 00BF65AC` at `00904BC5`
(five bytes, so the next instruction starts at `00904BCA`) and the stored instruction at
`00904BD6` (`POP EDI`), with the twelve bytes between them read from the disk image.

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 00904600 | BSP_World_UpdateMatrixInterpolators | exported, analyzed, reconstructed, build-tested |
| 00905080 | BSP_World_AddMatrixInterpolator | analyzed, reconstructed, build-tested |
| 008ADE00 | BSP_Lua_MW_AddMatrixInterpolator | analyzed (argument marshalling only) |
| 009045C0 | STL_list_buynode_009045c0 | analyzed |
| 0042ED50 | BSP_SceneNode_InvalidateSubtreePose | analyzed, reconstructed, build-tested |

Nothing in this packet is fixture-tested, ABI-compatible or game-validated.

## Uncertainties

* The composition order is read from the eight pushes at `00904A77..00904AAC`, unwound against the
  `ECX = left`, stack `(dst, right)` convention recorded for `00413920`. The convention is taken
  from that function's existing ledger record, not re-derived here.
* Whether the engine treats the record's three angles as x, y and z of a fixed convention or as
  something else is not established. The mapping angle to builder is exact
  (`+10h` to `00B64640`, `+14h` to `00B646E0`, `+18h` to `00B64780`); the builders' axes are read
  from which rows they leave as identity.
* `008ADE00`'s failure path, which builds the `luaMW_AddMatrixInterpolator failed:` string, was not
  read. Only the success path's marshalling was.
* `00904250`, the value copy-constructor `009045C0` calls, was not read. The record is assumed to
  be a plain copy because every field is overwritten immediately afterwards.
* The two loads at `00904658`-ish that Ghidra renders as `LIBCRT_unmatched_00bf6713` are this
  build's `_SECURE_SCL` iterator validations, not program logic, and are omitted from the
  reconstruction.
