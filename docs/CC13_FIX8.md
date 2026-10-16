# cc13-fix8: terrain grid loop, ocean-damp type test, camera capture routines

Packet cc13-fix8 (2026-10-16). Items 1 and 2 edit `src/landscape_lua_seg55_argon.cpp`
(rows in `docs/ARGON_SEG55_LANDSCAPE_LUA.md`); item 3 is new code in `src/cc13_fix8.cpp` /
`include/bsp/cc13_fix8.hpp`. Names are hypotheses, not recovered symbols. Everything here is
reconstructed and build-tested only; nothing is fixture-, ABI- or game-validated.

Reach: none of these functions is reached by bsp_game. `LandscapeHost` has no implementation
in src (00883BB0, the only caller of 00882AC0, is itself uncalled there), and the camera
routines have no caller in src. No behaviour switch was needed.

## 1. 00882AC0 grid-cell loop (00883507..008839E2)

The earlier host model (`get_grid_cell_00adea70(hf, gx, gz)` and
`create_grid_cell_physics_body_00c31aa0`) was wrong on two counts: 00ADEA70 is
`MOV EAX,[ECX+1Ch]; RET` on a tile, and no CALL 00C31AA0 exists in the loop. The listing:

- EBP = `[landscape+3D0h]` for the whole loop. Outer counter `[ESP+2Ch]` to 00ADAC30
  (`[hf+38h]`), pixel step `[ESP+30h] += 12Ch`; inner counter `[ESP+18h]` to 00ADAC40
  (`[hf+3Ch]`), step `[ESP+3Ch]` reset to 0 per column and `+= 12Ch`.
- `tile = [[hf+40h] + ([hf+38h] * gz + gx) * 4]` (00883552..0088355F, `[hf+38h]` re-read each
  cell); a null tile skips the cell (00883564).
- `0088356C CALL 00ADEA70` (ECX = tile) gives the block; its `+0Ch` kind selects the shape
  source: for kind 0 `00883662 CALL 00ADEA70` again and `+34h` is taken; for kind 2
  `0088362F CALL 00ADEA70` again and `+40h` is taken, with sets `[ESP+0A8h] = 1` and copies block `+2Ch` / `+30h` into `[ESP+9Ch]` /
  `[ESP+0A0h]`; any other kind leaves the data pointer 0 (00883622 `XOR ECX,ECX`) and does not
  store the two floats (they keep the previous stack contents; modelled by keeping the struct
  across iterations, start value unknown).
- The 64h-byte shape descriptor at `[ESP+48h]` is fully stored inline: `+08h = 8` (0 first),
  `+10h = 5`, an identity 3x4 pose (1.0f from 00D7A24C), `+44h = +48h = 21h`, `+4Ch = +50h =
  9.375f` (00D0E658; 10.0f from 00CE38B8 is stored first and overwritten). Modelled as
  `TerrainTileShapeDesc` with hypothetical field names.
- Pose: a copy of `landscape+74h` (REP MOVSD 10h dwords, not `+CCh` as for the hulls) with
  `x = [hf+80h] + float(gx*300)`, `y = 0.0f + 0.0f` (`FLDZ; FADD ST0,ST0`), `z = float(gz*300) +
  [hf+84h]` added to the translation (0088384F..00883905).
- The actor: `[[00E188A8]+18h]->00C5D580(&desc at [ESP+168h])` (00883921) after
  `00C336C0(&desc+14h, &pose)` (0088390C); the shape array is `operator new(8)` (008837EA)
  holding `&shape`, freed after the push_back (0088399D); `[ESP+1C4h] |= 1`; userData
  `[ESP+1DCh] = 0`. As for the hulls (cc13-fix7), the NxActorDesc default stores, 00C336C0,
  00C5D580 and the shape-array allocation are inside the host hook
  `create_grid_tile_actor_00c5d580(shape, pose)`.
- The actor is pushed into `+374h` with the inline push_back (00420830 when full).

New hooks: `height_field_tiles_across_38`, `height_field_tile_40`, `height_field_origin_x_80`,
`height_field_origin_z_84`, `tile_block_00adea70`, `create_grid_tile_actor_00c5d580`;
removed: `get_grid_cell_00adea70`, `create_grid_cell_physics_body_00c31aa0`,
`TerrainGridCellDesc`.

## 2. 00882420 type test

`ESI = [node]; ESI += 0Ch` (00882444..0088244A) is taken first, then `CALL 00B74310`
(`MOV EAX,[01090034]`), `PUSH EAX`, `CALL [ESI]` with `ECX = node` (00882457), `TEST AL,AL`
(0088245B). Now `host.generated_model_current_type_00b74310()` then
`host.node_is_type_vslot0c(node, type)`. `GeneratedModelNode::matches_current_type` stays only
for 00881600, which makes the same call and was not in this packet.

## 3. Camera capture routines (segment 89)

The Ghidra body of 00B83F30 is right (564 instructions, ending at its `RET 10h` at 00B847AF).
The code after the INT3 padding, **00B847C0** to 00B848AE, is a separate function with no
Ghidra function of its own; its `CALL 00B83F30` at 00B847EA is what the earlier ledger note
read as a "self-recursive call site". 00B847C0 has no rel32 or abs32 reference in the image
(`scan-bytes 'c0 47 b8 00'` empty; the positive control `94 d3 f8 00` matches). It is
reconstructed here as well; no Ghidra function was created (no Ghidra writes in this packet),
so verify_report_calls reports its call sites as "in no Ghidra function".

All offsets below are from the body ESP after the register pushes. EH-state anchors:
00B83F30 `[S+1BCh]`, 00B847C0 `[S+1Ch]`, 00B848C0 `[B+74h]`, 00B84A90 `[B+70h]`.

| Address | C++ | ABI | Notes |
|---|---|---|---|
| `00B83F30` | `camera_render_to_frame_targets_00b83f30` | ECX = `const float min_xz[2]`, EDX = `const float max_xz[2]`, stack (float scale, camera, format, targets), EBP frame aligned 40h, RET 10h, EAX = surface | See below. |
| `00B847C0` | `camera_save_tiled_screenshot_bmp_00b847c0` | ECX/EDX passed through, stack (float scale, camera, `const NativeString* name`), RET 0Ch, AL | `00B847EA CALL 00B83F30` with format 15h and null targets; null -> 0. Else `00B84802 CALL 0041E870` ("_.bmp", 00D63170), `00B8481D CALL 004261A0` (name + suffix), `surface->vslot34(out, 0)` (00B84831), the two releases (`00B8484B CALL 00419CC0`, `00B84852 CALL 00BD1510`, `00B84872 CALL 00419CC0`, `00B84879 CALL 00BD1510`), `surface->vslot4(1)` (00B84887), AL = 1. |
| `00B848C0` | `camera_save_screenshot_bmp_00b848c0` | ECX = camera, EDX = `const NativeString* name`, stack targets, RET 4 | Viewport size by `00B848E2 CALL 00B6FDE0` / `00B848E9 CALL 00B1F740` (width) and `00B848F2 CALL 00B6FDE0` / `00B848F9 CALL 00B1F740` (height); ESI = `[00F8D394]` once; `vslot84(w, h, 16h, 2)`, null returns; vslot0C; label "X" (00CE9A38) by `00B8493E CALL 0041E870`; `00B84955 CALL 004C11F0`, `00B84974 CALL 00B1F4D0` (`[camera+A4h]`, camera, targets, label by value, EH -1 first); vslot14; `00B84986 CALL 00B3F450` on the 34h-byte object at `[B+38h]`; renderer vslot7C(&object); `surface->vslot38(&object, &rect, &rect)`, rect {0, 0, w, h}; `00B849D6 CALL 0041E870` (".bmp", 00D63178); `00B849EE CALL 004261A0`; `surface->vslot34(out, 0)`; releases (`00B84A1B CALL 00419CC0`, `00B84A22 CALL 00BD1510`, `00B84A3F CALL 00419CC0`, `00B84A46 CALL 00BD1510`); InterlockedDecrement(surface+4) and vslot0 at zero; `00B84A69 CALL 00B3F4E0`. |
| `00B84A90` | `camera_save_screenshot_bmp_256_00b84a90` | as 00B848C0 | No viewport query; ESI = `[00F8D394]` read first; `vslot84(100h, 100h, 16h, 2)`; rect {0, 0, 100h, 100h} at `[B+24h]`; `00B84AF3 CALL 0041E870`, `00B84B06 CALL 004C11F0`, `00B84B21 CALL 00B1F4D0`, `00B84B33 CALL 00B3F450`, `00B84B85 CALL 0041E870`, `00B84B9D CALL 004261A0`, releases `00B84BC8 CALL 00419CC0` / `00B84BCF CALL 00BD1510` / `00B84BEA CALL 00419CC0` / `00B84BF1 CALL 00BD1510`, `00B84C14 CALL 00B3F4E0`. |

### 00B83F30 in order

1. `00B83F64 CALL 00B6FDE0` / `00B83F6B CALL 00B1F740` (viewport width `[S+40h]`),
   `00B83F78 CALL 00B6FDE0` / `00B83F7F CALL 00B1F740` (height `[S+6Ch]`).
2. Pixel size `W = trunc(((max.x - min.x) / 9.375) * scale)` (DC F9 read from the bytes:
   `ST1 = ST1 / ST0`) and `H = trunc(((max.z - min.z) / 9.375) * scale)` (DE F1), 9.375 =
   double 00D5D410; FISTP QWORD under `CW | 0C00h`, low dword kept.
3. Saved state: `00B84004 CALL 00B6DB60` (16 floats copied to `[S+140h]`), `+1C4h` fov,
   `+1D4h` near, `+1D8h` far, `+1C8h` aspect; fog scalars `00B8405D CALL 00B84CA0` and
   `00B8406F CALL 00B84CE0` on `[camera+184h]`.
4. Viewport size again (`00B8407D`, `00B84084`, `00B84091`, `00B84098`) for the tile surface;
   `00B840B0 CALL 00B84D00` and `00B840C1 CALL 00B84D40` set both fog scalars to 0.0f.
5. Tiles: `tiles_x = trunc(float(ceil(float(W / float(vw)))))`, `tiles_z = trunc(float(ceil(
   float(float(H) / float(vh)))))` (`00B84106 CALL 00BFA7B0`, `00B8417E CALL 00BFA7B0`; 00BFA7B0
   is the CRT ceil: its non-SSE fallback is `__ceil_default` 00C147F6). Each FILD of a dword is
   fixed up by +2^32 (float 00CE3978) when negative, i.e. read as unsigned.
6. `[camera+188h]` saved; `upp = float(9.375 / scale)`; `00B841DE CALL 00B6FE10` (clear flags 7).
7. Orthographic projection at `[S+0E8h]`: `m0 = 2 / (vw * upp)`, `m5 = 2 / (vh * upp)`,
   `m10 = 1 / (far - near)`, `m14 = near / (near - far)`, `m15 = 1.0f`, rest 0 (the six DE-form
   operations checked from bytes); `00B842C8 CALL 00B6FD60`.
8. Targets: a caller's object gets `InterlockedIncrement(+4)`; otherwise `00B842E8 CALL 00BF681B`
   (40h) then `00B84301 CALL 00B1FBB0` (EH state 0), `00B8432E CALL 00B1FAB0` (index 0, renderer
   vslot128(0)) and `00B84346 CALL 00B1FB00` (renderer vslot12C()), with no null test on the
   constructed object.
9. `renderer->vslot2C()` saved in `[S+1Eh]`; when set, vslot14() now and vslot0C() at the end.
10. The output sides are halved together (shift count `[S+80h]`) until both are <= 1000h; output
    = `vslot84(W >> k, H >> k, format, 2)` (`[S+70h]`), tile = `vslot84(vw2, vh2, format, 2)`
    (`[S+20h]`). A null output returns 0 at 00B843D3 with no release and no restore.
11. Per column i and row j: `x = float(min.x + (i*vw + 0.5*vw) * upp)`, `z = float((float(H)*upp -
    (j*vh + 0.5*vh) * upp) + min.z)`; `00B84509 CALL 00B70200` with eye (x, 10000.0f, z) (float
    00CE3D64), target (x, 0, z), up (0, 0, 1); vslot0C; the label "X" built inline as
    `00B84533 CALL 0041DD40` (1, 1) and `00B8454B CALL 00BF7680` (memcpy, length + 1);
    `00B84564 CALL 004C11F0`, `00B8457D CALL 00B1F4D0`; vslot14; source surface
    `00B84594 CALL 00B1F6D0` on the ARGUMENT targets, else renderer vslot128(0);
    `00B84663 CALL 00B1FF70` (source -> tile); `output->vslot38(tile, {0, 0, min(vw, W),
    min(vh, H)}, {px, py, min(px + vw, W), min(py + vh, H)} >> k)`; `py += vh`; `px += vw` per
    column.
12. Restore: tile vslot4(1); targets released (`InterlockedDecrement(+4)`, vslot0 at zero);
    `camera->vslot38(saved matrix)`; `00B8472A CALL 00B6FBB0`, `00B84739 CALL 00B6FBF0`,
    `00B84748 CALL 00B6FC10`, `00B8475A CALL 00B6FBD0`, `00B84766 CALL 00B6FE10`,
    `00B8477C CALL 00B84D00`, `00B84792 CALL 00B84D40`; returns the output surface.

Uncertain: the renderer slot meanings (0Ch / 14h scene begin / end, 2Ch in-scene, 7Ch back-buffer
capture, 84h create surface with last argument 2, 128h / 12Ch colour / depth surfaces) and
the surface slots (34h save, 38h copy rectangle, 4 release) are inferred from use; 15h / 16h
read as D3DFORMAT values only if the slot forwards one. The x87 arithmetic is written in double
on the assumption of the default 53-bit precision control.
