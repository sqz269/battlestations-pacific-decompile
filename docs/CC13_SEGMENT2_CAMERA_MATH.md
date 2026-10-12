# Segment 2: the camera movers (packet cc13-net3, segment 2)

Partition segment 2 runs from `0042A920` to `0043E840`. This packet leaves out
`00435D30..0043CF9B`, the profile and Lua-text cluster that `docs/CC13_PROFILE_SAVE.md` covers.

Status vocabulary: *named* means a Ghidra rename and ledger record. *Reconstructed* means C++ in
`src/segment2_camera_cc13.cpp`, which so far is build-tested only. Nothing here is game-validated.

## Triage

`local/net3_triage.py` and `local/net3_triage2.py` work from the index and a text search of
`src/`, `include/` and `docs/`. The segment, minus the excluded cluster, holds 287 functions:
- 60 already have reconstruction records;
- 23 are EH/CRT/STL helpers;
- 29 are cited by existing C++;
- 145 had no mention anywhere.

Almost no strings point into the segment (8 references, 3 of them `vector<T> too long`). What it
holds is the **camera-mover class family**: the base class, every subclass's per-frame step, and
the vector/matrix helpers they share.

Earlier work this packet builds on:
- `docs/MISSION_CAMERA.md` and `src/mission_camera.cpp` (cc9, ShipCaptain) reconstruct the base
  publish `004329D0`, the water probe `0042F0C0`, the shake setter `0042C4C0` and the constructor
  fields of `00432750`.
- `docs/HUD_PICK_SEGMENT_QUERY.md` covers the movie camera.
- `docs/CC13_CAMERA_CONTROLLERS.md` documents `BSP_GameCameraModifier_EnsureCamera`, which
  chooses a mover class by entity kind.

This packet does not touch those files.

## Never-defined bodies created

The undefined regions were found by comparing each function's last Ghidra instruction with the
next function start (`local/net3_bodyends.py`). Each region was then swept from its first
instruction to a `RET` followed by `INT3` padding up to a 16-byte boundary (`local/net3_sweep.py`).
Seven bodies were created under the Ghidra write lock and the program was saved:

| body | references (disk scan) |
|---|---|
| 00430020..004307EF | vtable `00CE3E60` slot DCh (`00CE3F3C`); also absolute dwords in `.rdata` and at `0060E4EC` |
| 00431480..00431F13 | `CALL` at `00434F2B` (in 00434E50); absolute at `007A008C` (in `BSP_MovCam_DumpPositions`) |
| 00431F20..00431F3C | none found |
| 004335C0..004337F0 | vtable `00CEC5F0` slot DCh (`00CEC6CC`) |
| 00434140..0043480D | vtable `00CF6250` slot DCh (`00CF632C`) |
| 00434810..00434E44 | vtable `00CF4410` slot DCh (`00CF44EC`) |
| 00434E50..0043511E | vtable `00CF4238` slot DCh (`00CF4314`) |

Afterwards `_free` (`00BF9DC8`, thunks `00BF65AC`/`00BF6989`) still reads
`void __cdecl _free(void *)`, not noreturn.

**Listing defect left in place.** Ghidra's body for `00434E50` stops at `00434F4F` and resumes
at `00434FF2`. A stale code unit at `00434F52` (an `INT3` decoded mid-instruction) overlaps the
real fall-through `00434F50: CMP dword ptr [EAX+19CCh],0`. The bridge has no clear-code endpoint,
so the overlap remains. Decompiles of 00434E50 miss `00434F50..00434FF1`; read that stretch with
`bsp.py disasm-raw`.

## The class map

Each mover class has its own vtable; slot DCh is its per-frame step `(mover, float dt) RET 4`.
The class ids come from `docs/ENTITY_CLASS_IDS.md`. The kind each class is built for comes from
`BSP_GameCameraModifier_EnsureCamera` (`docs/CC13_CAMERA_CONTROLLERS.md`) and from `0067B4D0`.

| vtable | class | built by | for | slot DCh step |
|---|---|---|---|---|
| `00CE3B28` | 4Ch base | `00432750` BSP_CameraMover_ConstructBase | - | `00435410` BSP_CameraMover_StepBase (forwards to `004329D0`) |
| `00CE3E60` | 4Eh | `00435120` | INTF_BOMBVIEW, torpedo or no target | `00430020` BSP_TorpedoViewCamera_Step |
| `00CE3FD0` | 4Eh | `00435280` | INTF_BOMBVIEW, any other target | `00430DA0` BSP_BombViewCamera_Step |
| `00CEC1F8` | 53h | `005176F0` | kind 45h, MAirfield | `00433590` BSP_AirfieldCamera_Step |
| `00CEC5F0` | 56h | `00519490`, `005198F0` | not established | `004335C0` (provisional name) |
| `00CF6250` | - | `00650830`, `00651370` | kind 8, MSubmarine | `00434140` BSP_SubmarineCamera_Step (calls `00433800`/`00433CB0`, which read `ShipCamera.LengthMult`) |
| `00CF4410` | 4Ch | `006065D0` | kind 0Fh, planes | `00434810` BSP_PlaneCamera_Step |
| `00CF4238` | 51h | `00605130` | any other kind | `00434E50` BSP_DefaultUnitCamera_Step (calls `00431480`, which calls `0042F800`, which reads the `PlaneCamera/*` and `BombCamera/BombSub*` tunables) |
| `00CF5CE8` | 50h | `BSP_ShipCaptainCamera_Construct` | kind 6, ships | cc9's `docs/MISSION_CAMERA.md` |

`00CE3E60` and `00CE3FD0` both answer class 4Eh (`docs/ENTITY_CLASS_IDS.md` noted the
ambiguity). They are the two INTF_BOMBVIEW cameras, built by `0067B4D0`. That function is called
on the interface's `+8Ch` by the 26h arm (`docs/IN_MISSION_INTERFACE_MANAGER.md`):
- **Torpedo view.** With no target, or a target of kind 2Bh (MTorpedo), it allocates 3CCh bytes,
  calls `00435120` (stored at `+50h`), then `0042FEE0(target)`.
- **Bomb view.** Otherwise it allocates 3F8h bytes, calls `00435280` (stored at `+4Ch`), then
  `004307F0(target, launcher)`.
- Either way `BSP_Game_SetCameraMover` installs the camera.

The bomb view embeds a tick element at `+170h` (vtable `00CE3F94`). Its slot `+4h` is `00430970`
(BSP_BombViewCamera_TickElementSlot4), which reads the `BombCamera` follow-distance tunables.

## Reconstructed

All of these are in `src/segment2_camera_cc13.cpp`; calls into other subsystems go through
`WeaponViewHost`.

- **`00435120` / `00435280`, the constructor tails.**
  - Both run `00432750`, set their vtables (the bomb view also sets `+10h` `00CE3FB4`, `+24h`
    `00CE3FAC` and `+170h` `00CE3F94`), and embed the target observer at `+384h`. The observer
    has vtable `00CE3CF0`, `+4h..+Ch = 0`, `+10h = 1`, and its `+14h` is the target, which is
    camera `+398h`.
  - The bomb view also zeroes `+3F0h`/`+3F4h`.
- **`0042FEE0` (torpedo, RET 4).**
  - On a target change it unregisters the old pair (`006952A0`, ECX = old target, EDX =
    observer) and registers the new one (`00694A60`).
  - It copies the world camera node's matrix into the camera: node `[game+19FCh]`, `+F0h`, through
    `004134F0`, after `00B6DB70` when bit 2 of node `+5Ch` is clear.
  - It saves the start position, forward and up at `+3A8h`/`+3B4h`/`+3C0h`.
  - `+39Ch` gets the target's world position, or the camera position when there is no target.
- **`004307F0` (bomb, RET 8).** The same observer update, then `+3D8h = 1.0`. With a target it
  also:
  - copies the node matrix;
  - sets `+3CCh` to `launcher->vtable[34h]()` (the launcher is not null-checked) and `+3DCh` to
    the BSS vector `00F87574`;
  - saves forward and up at `+39Ch`/`+3A8h` and the position at `+3C0h`;
  - sets `+3E8h = 0.1` and `+3ECh = 0.22`.

  It always zeroes `+3F0h`/`+3F4h`.
- **`00430DA0`, the bomb view step.** blend = `dt * tuning+E8h` (`BombCamera/CameraPosSmooth`).
  1. The target point `+3B4h` follows the target's world position. It is kept at least 3.0 high
     (`00CE3854`), unless the target is a running torpedo (kind 2Bh with `[+314h]+F4h > 0`) that
     is already at or below -15.0 (double `00CE3D58`).
  2. The wanted point is the target point plus the offset `+3DCh`.
  3. The camera position `+3C0h` drifts by `dt * +3CCh`. With a running torpedo and a wanted
     point below 0, its height is capped at -3.0 (`00CE3D50`).
  4. The forward `+39Ch` blends towards the unit vector to the wanted point. The distance is 0
     when the square is at most 1e-10, and the division is not guarded.
  5. The up `+3A8h` blends towards (0, 1, 0) at blend / 3.0.
  6. Rows 2/1 get forward and up, then `0085DC80` orthonormalizes; row 3 gets the position.
  7. `0042D610` orbits the pose about world up by `+3F0h`, then about row 0 by `+3F4h`, both
     through the wanted point.
  8. `+C8h` and `+10Ch` are cleared, the children are invalidated (`0042ED50`), `00414DB0`
     refreshes the camera, and `[+1BCh]->vtable[34h](&camera+CCh)` publishes.

  The symbolic trace (`tools/x87_sym.py 00430DA0`) gives the float spill points the C++ keeps.
- **`0042D610` (RET 0Ch).** `len = 00419440(axis)`. The axis is divided by `-len`, and
  `0085E8C0(matrix, point, &unit, len * angle)` rotates the matrix about that axis through the
  point. With the unit axes the step passes, this is a rotation by `angle` about `-axis`.

## Named only

`00432750`, `00435410`, `00430020`, `00430970`, `00433590`, `004335C0`, `00434140`, `00434810`
and `00434E50`, with the evidence in the table above.

## Handoff

Totals: 7 bodies created, 15 Ghidra renames with evidence (14 class, constructor and step names, plus
`0042D610`), 6 reconstruction records, build-tested only. Tools: `local/net3_*.py` in
the cc13-net2 worktree (triage, body ends, sweep, rel32 scan, vtable compare).

Next, in order of value:

- `00430020`, the torpedo view step (2000 bytes). It uses segment queries
  (`BSP_SpatialIndex_QuerySegment`) and the initialized `.data` tunables at
  `00E08074..00E08084`.
- `00430970`, the bomb view tick slot.
- The other classes' steps: `00433590`, `004335C0`, `00434140`, `00434810`, `00434E50`,
  `00431480`, `0042F800`, `00433800`, `00433CB0`.
- The 4Ch base slots (`0042B8xx..0042BBxx`, mostly small accessors), and the math helpers with
  callers outside the segment: `0042B570`, `0042C1B0`, `0042C750`, `0042C940`, `0042E180`.
- The overlap at `00434F52`.
