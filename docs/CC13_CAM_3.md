# Camera and session reconstruction, third pass (packet cc13-cam3)

This pass continues `docs/CC13_CAM_2.md` (packets cc13-cam2/cam2b) over segments 45 (`0078D880-007A4860`)
and 43 (`0076AA00-00787040`). The code is appended to `src/engine_seg43_45_cc13.cpp` and
`include/bsp/engine_seg43_45_cc13.hpp` (namespace `bsp::cc13cam2`, section "Packet cc13-cam3").
The conventions are the same as cam2's. Entities and engine objects are host handles, and engine
helpers are host callbacks. x87 chains are written in double, with a float cast at each native `FSTP`.
Every DE/DC-form reg-reg x87 instruction transcribed here was decoded from its bytes:
`0079C515 DE E2`, `0079C619 DE FA`, `0079C4D7 DE CB`, `0079099B DC CA`, and `00790993` and
`007909CB DE EA`. All names are hypotheses. Nothing here is ABI compatible, bound into a host or
game-validated.

## Status

| Item | Exported | Reconstructed | Build-tested | Validated |
| --- | --- | --- | --- | --- |
| MovCam frame `0079C200` (input latch, per-mode extrapolation) | yes | yes | yes | no |
| MovCam constructor/destructor, keyframe ConstructAt/Destruct/Reset, track ctor/dtor, movie-camera dtor (8) | yes | yes | yes | no |
| Vehicle-camera window editor `0079F9E0`, sector debug draw `007906E0` | yes | yes | yes | no |
| Session: SetMode, peer added/removed, game setup, host remove player, MT_CONNSTATE builder (6) | yes | yes | yes | no |
| NetTable variant and E0h message helpers (5), small session helpers (4) | yes | yes | yes | no |

Ledger: 26 function records (status `reconstructed_build_tested`, source
`src/engine_seg43_45_cc13.cpp`) and 3 renames. Call-site rows: `reports/cc13_cam3_calls.json`
(93 rows, 0 failed under `tools/verify_report_calls.py`).

## 1. `0079C200` is the MovCam's per-frame extrapolation, not another camera class

The cam2 handoff read `0079C200` as a separate input-driven camera. It is slot `+4h` of the MovCam's
`+170h` sub-object vtable `00D04B1C` (`.rdata 00D04B20`). Its other slots are `+0h` `0079D830`,
`+8h` the fixed step `007A91A0` and `+Ch` the snapshot `00790460`. ECX is camera+170h, and every
camera call it makes passes `LEA ECX,[ESI-170h]`. The `+224h..+38Ch` offsets in its listing are
therefore camera `+394h..+4FCh`. These are the position, theta/rho, follow target and zoom that
cam2 had already named, and the snapshot that `00790460` writes. Renamed
`BSP_MovCam_ExtrapolateFrame`.

- **Input latch.** The rising-edge test `004C43C0` covers actions `78h..7Bh` (into bytes
  `+51Ch..+51Fh`) and `82h`, `83h`, `81h` (into `+534h..+536h`). Results are ORed in, so they stay set
  until the step consumes them. The floats `+520h..+530h` take `+24h` of the input records for actions
  `7Ch..80h`. The meanings of these actions are not identified.
- **Modes 0, 1, 3, 4, 6.** The new position is the snapshot plus velocity times dt. The angles are the
  snapshot plus rate times dt; with a follow target, `00795180(dt)` sets them instead. `007904D0`
  then runs.
- **Mode 2 (deck).** zoom `+460h` = `((dt*+514h) + 1.0) * +4F4h`, clamped to `[1, 21]`. The field of
  view becomes `00F889B4 / zoom`. The angles are handled as above. Then `+74h` =
  `RotX'(theta) * RotY'(rho) * +420h * world(+38Ch)`, and `00790540`/`00799750` are skipped.
- **Mode 5 (path).** The parameter `+468h` starts from the snapshot `+4FCh` and moves toward
  `+46Ch` at the top speed times dt, stopping 0.01 short. Then either the knot path `007972F0` runs,
  or `007B04C0` places the camera and `00794070` aims along the path direction. When dt > 1e-4, the
  velocity and the angle rates are rewritten as (now - snapshot)/dt.
- **Mode 7.** The eye is at `+4C4h` and the aim at `+4B8h`, each in its entity's frame when that entity
  is set.
- **Tail, all modes.** The pose bytes are cleared, `0042ED50` runs on the children, then `004329D0(dt)`.

**Correction to `cc13_camera_controllers.hpp`.** The extrapolation shows that `+3A0h` is a velocity
and `+3CCh/+3D0h` are angular rates, both per second. `MovCamMotion::position_3a0` and
`MovCamTurn::angles_3cc` name them as a position and angles. That file was left unedited. Its
`PathCamera::parameter_shift_4fc` is likewise `00790460`'s snapshot of `+468h`, which `0079C200`
reads as the starting parameter.

## 2. Lifetimes

- **MovCam `0079F480` / `0079D640`.** `+4D0h` is not the camera's own data. It is the shared block
  `[[00E198C4]+98h]`, and the four `Reset*` bindings restore it at construction. The destructor frees
  the following in order: the interpolator payloads, the path records (`0079C830`), the two
  LuaVariant argument vectors, the list, the two callback NativeStrings and the record storage. It
  then runs `00695870` on the CameraRef at `+3F0h` and the base `0042A830`.
- **Keyframe ConstructAt `0079C910` (x, y, z, entity).** When the entity is being destroyed,
  SetEntity `00799D70` fails. Its position is then baked into the three vectors and the key keeps no
  entity. ConstructAt does not write `+C0h`, which the default constructor `00799B00` sets to -1.0.
  Destruct `00799CA0` and Reset `00799EA0` are short and recorded as written.
- **Tracks `0079CFC0` / `0079A190`.** The destructor deletes the keys virtually, front first.
- **Movie camera destructor `0079A260`.** It reseeds **RNG stream 1** (ECX = 1), not stream 0. The seed
  is the clock (`[01090AB0]` slot `+20h`, counter/frequency) in milliseconds, truncated. The host gets
  it through `seed_stream_00bd2fd0`. This is for cc9's RNG ledger: stream 0 is untouched.

## 3. Debug tools

- **`0079F9E0`, renamed `BSP_MovCam_EditVehicleCameraWindow`.** Within the MovCam's walk inputs, a
  horizontal or vertical deflection beyond ±0.5 stores the current rho or theta into the target class's
  vehicle-camera window (element `+388h`, fields `+48h..+54h`). It logs a Lua-shaped line to
  `moviecamera.log`, for example
  `<class name>; mov.cam.pos.: [<index+1>]\n\t\t\t\t["MaxHorzAngle"] = <rho as %f>,`, and shows message
  `1/3/0/2` through `005CBA50`. With both inputs idle it still logs the header. With both active it
  does nothing.
- **`007906E0`.** It writes the element's name at the window's centre point. It draws four rays to the
  corners, eight segments along each window edge, and a cross at the window centre. All points are at
  distance 4.0 through `007905B0`.

## 4. Session (segment 43)

- **`0076FAD0` SetMode.** This was read from the listing. The singleton call at `0076FB53` takes no
  argument: its `PUSH EBP` is the prologue's save. Mode 1 rebuilds the server (`00A42170`) and replays
  the scene records. Mode 2 builds the client (`00A41C70`). Mode 0 sends `MQuitMsg` (message 19) when
  a connected client leaves, and resets the HUD globals. Mode 0 does not delete the transports, and
  with a world present it only sets `+27Dh`.
- **`00774AB0` / `00772EC0`, peer added and removed.** These handle the transport list, the peer rows,
  Live talker registration and mute, and `XNetUnregisterKey`. The image has two hazards. The added
  path reads the player record again after the null-record branch, which would fault. The removed
  path passes `0D74h` as the address for a null peer. The reconstruction returns `false` or keeps the
  call as written, and says so.
- **`00773860`.** It sends six NativeString lists (kinds 0..5, one message each) and one packed
  message of three bit vectors and three byte arrays with their counts. The lists themselves are not
  identified. The 256-byte payload is not bounded in the image.
- **`007702E0`, host removes a player.** In order: deselect, unit hand-over (`0076F2B0`, cc9's),
  MT_REMOVEPLAYER, `004D0FC0`, Live removal, recount, MT_SERVERINFO, the debug line and the system
  chat line.
- **`00786390`, the MT_CONNSTATE builder.** It copies `+4h/+8h/+Ch` of each of the eight peer
  rows. Its caller, the rows tick `007866B0`, was reconstructed in parallel by cc13-cam2b
  (`peer_rows_update_007866b0`, `CC13_CAM_2.md` section 9), so this pass's duplicate of it was
  dropped at the merge. Both readings have the swap-and-rescan. One thing to note from this pass: two
  peers claiming the same index would make that rescan loop forever.
- **NetTable.** `00781430` / `007814E0` / `00781540` handle the key variants. `00781F60` builds
  message E0h and `00781590` writes it, in the wire order the batch-3 notes of
  `multiplayer_session_cc13.hpp` give.
- **Small helpers.** `0076C450` stamps a player's voice time. `0076C740` classifies a join (0 or 2).
  `0076D220` resets the online flag. `0076E7F0` is a prefix test at a position, not a search, and is
  renamed `BSP_WideString_MatchesAt`.

## 5. Names changed (ledger and Ghidra, `tools/ghidra_annotate.py --apply`)

| Address | Was | Now |
| --- | --- | --- |
| `0079C200` | `BSP_MovCam_UpdateModePose` | `BSP_MovCam_ExtrapolateFrame` |
| `0079F9E0` | `BSP_MovCam_DumpPositions` | `BSP_MovCam_EditVehicleCameraWindow` |
| `0076E7F0` | `BSP_WideString_FindSubstring` | `BSP_WideString_MatchesAt` |

The prior Ghidra values are in `local/ghidra-annotations-20261013T071309Z.json` of the worktree.

## 6. For cc9

No gameplay function was reconstructed. The session code reaches `0076F2B0` (unit hand-over),
`004D0FC0` and `0077E4C0` only as host callbacks. The movie-camera destructor reseeds RNG stream 1
from the clock (section 2). The list in `CC13_CAM_2.md` section 6 still stands.

## 7. Handoff

Still open, in order of value:

1. **`00776930`, the DLC check** (654 lines). It uses the Lua state `SCRIPTS/datatables/DLC.lua`
   (`DLCTable`) and wide strings, comparing the masks at `+170h/+174h` with the client's.
2. **The step `007A91A0`** (7 KB, slot `+8h`). It is the consumer of the latched inputs
   `+51Ch..+536h`. Reading it would name the actions `78h..83h`.
3. **`00799B00`**, the keyframe default constructor. cc9's `hud_movie_camera.hpp` covers it, but it
   has no ledger record. `0079D830` (slot 0 of `00D04B1C`) sits inside Ghidra's
   `CG_adjustor_thunk_0079d820` and needs its own function.
4. **The game setup lists** `00E196CC..00E19788`: their writers would name them.
5. **`0076E3C0`** (blob vector append) and the STL rows from the cam2 handoff, which are tag
   candidates.

Build: `scripts/build.ps1` passes. No game runs were made.
