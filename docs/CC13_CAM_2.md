# Camera and session reconstruction pass over segments 45 and 43 (packet cc13-cam2)

Segment 45 (`0078D880-007A4860`: camera, movie camera, MovCam) and segment 43
(`0076AA00-00787040`: voice, MMultiPlayer session plumbing, multiscore, recon level). Code:
`src/engine_seg43_45_cc13.cpp`, `include/bsp/engine_seg43_45_cc13.hpp` (namespace
`bsp::cc13cam2`), registered in `cmake/startup.cmake`. It reuses `cc13cam::CameraRef`,
`MovCamMotion`, `MovCamTurn`, the limit blocks, `scale_to_length_00793730/00793800` and
`camera_ref_resolve_00793ac0` from `src/cc13_camera_controllers.cpp`, the native bit cursor
(`native_bit_cursor_*`), `extract_camera_matrix_angles_0042d2e0` and
`ship_ai_firepower_wrap_angle_00605070`.

Every name is a hypothesis. Nothing here is ABI compatible or game-validated, and nothing is bound
into a host: entities are host handles, engine helpers are host callbacks, NativeStrings are
`std::string`. The x87 chains were traced from the disk bytes
(`local/tools_scene2/sc2_x87.py` of the cc13 tree, block by block) and written in double with a
float cast at each native `FSTP`, which matches 53-bit precision control when every product of
two floats is exact; CRT `sqrt` is `std::sqrt`. `tools/x87_dc_form_scan.py` over segment 45 found
DC forms only in functions this packet did not transcribe from listing text.

## Status

| Item | Exported | Reconstructed | Build-tested | Validated |
| --- | --- | --- | --- | --- |
| CameraRef family (10) and path-record observers (3) | yes | yes | yes | no |
| MovCam motion, look and callbacks (16) | yes | yes | yes | no |
| Movie-camera track navigation (5) and keyframe events (12 addresses) | yes | yes | yes | no |
| Session MovCam broadcasts (7), BLACKBAR serializers (2), slot reset | yes | yes | yes | no |
| Chat, countdown, voice, start gate (11) | yes | yes | yes | no |
| Backfilled records: band_0076 (6), hud_movie_camera (14) | yes | existing code | yes | no |

Ledger: see section 8 for the record count; call-site rows in `reports/cc13_cam2_calls.json`.

## 1. Camera references and path records

The camera reference (vtable `00D04660`, scalar dtor `00793900`) is an observer: base words
`+4..+10h`, `+14h` entity, `+18h` point, `+24h` radius. The observer helpers take ECX = the
observed entity and EDX = the observer object (`0078E3EA`, `0079034C`, `007970AF`).

- `00793920` / `007939B0`: an entity whose `+5Eh` byte is set is dropped; point or radius set,
  the other cleared; registered when the entity survives. `00793A40` copies and registers
  without the `+5Eh` test. `00790320` (assign) does the same but never unregisters the entity it
  overwrites (listing `00790320..00790356`); the possible double registration is kept.
- `00793F10` is `00793AC0`'s non-radius branch on an arbitrary ref (point through the entity
  world, or, with the level flag, through that world with its translation replaced by
  `00F8758C..94` and `0085DAD0`). `00793CD0` stores such a transformed point and zeroes the
  radius. `00793DC0` and `00796F00` freeze the resolved point and clear the entity without
  unregistering.
- `00793E60` builds a 4Ch-byte observer (vtable `00D043D8`) that registers even a null entity;
  its owner was not found.
- The path record (camera `+47Ch` pointer array) has two watched entities, `+14h` and `+24h`
  (`00796F40` handles both). `0078E3D0` / `0078E410`, which the ledger called MovCam setters,
  swap exactly those two slots, each guarded against the other: they are the path record's
  anchor and second-entity setters (renamed, section 7). `00796F40` on release: anchor ->
  point frozen through `00793F10(level)`; mode 3 also moves `+28h` into the world and becomes
  2; mode 5 adds `0042D2E0`'s x/y angles of the anchor world to `+34h/+38h` and becomes 4;
  second entity in mode 1 -> its position `+FCh` into `+28h`, mode 2.

## 2. MovCam

Fields as in `docs/CC13_CAMERA_CONTROLLERS.md` section 13. The limit block `[+4D0h]` is read by
`007943D0` as `+4Ch` velocity cap, `+34h` step cap, `+50h` turn-rate cap, `+38h` turn-step cap;
section 13 called `+4Ch`/`+50h` accelerations, which is how `00794B80`/`00794D60` use them, so
both readings stand.

- `007943D0` integrate: |v| (sqrt of the float-spilled `(y*y + x*x) + z*z`, zero unless above
  `1e-10`) above `+4Ch` -> rescaled (`00793730`); `+3A0h += f(v*dt)`; the step from `+3B8h`
  above `+34h` -> rescaled; the same for the angles with `+3D4h`, `+50h`, `+3DCh`, `+38h`.
- `00794F70` braking steer: `r = |angles - previous| / +50h`; the wanted turn is
  `d - f(f(delta*r)*0.5)` per axis; its length times 1000 (`00CE47A0`), capped at 2.0
  (`00D7A308`), scales `+50h`; `00793800`'s vector is added to `+3D4h`. A non-null target arms
  the follow cone with growth 0.9 (`00CE3860`), not section 13's 0.875 (`00794D60`).
- `00795300` look basis: identity, row 2 the normalised direction to the target, row 1 the
  global `00F8758C..94` unless `|row2.y| >= 0.9999` (`00D04680`), then the mover's `+84h` row;
  `0085DC80`.
- `00797070` look target (observer at camera `+10h`, listing `00797156`): vehicles map to their
  `+3D0h`; in mode 1 an even sub-mode `+388h` becomes odd; aiming stores `+3E4h` and the aimed
  angles `+4ECh/+4F0h`. `00797230` turn-by clears the target, sets the wanted angles
  `+418h/+41Ch`, and either snaps or sets the angular limit to `00414C60(angles - wanted) / time`.
- `0079D960` / `0079DA90`: the arrival callbacks (`+49Ch`/`+4ACh`, `+4A4h`/`+4B0h`): the name is
  copied, emptied (`00CE3A0C`), the argument vector taken and freed after `00887E50`.
  `0079D870` stores the secondary callback and sets the look target.
- `007901D0` applies one `007900B0` interpolator record: `u = f((clock-start)/duration)`,
  `value = f(from + (to-from)*u)` with the product on the stack, `to` once `u >= 1`; ids 1..5
  write `+50Ch, +510h, +504h, +508h, +514h` (turnH, turnV, walkH, walkV, zoom inputs), 6..8 call
  `005CB9F0`, `005CBA20`, `0078E480`.
- `007904D0` wraps (`00605070`) and clamps theta to `+-pi/2`; `00794360` seeds position and
  `theta/rho = -0.0 - (x, y)` of `0042D2E0` on the mover matrix; `00794000` clears the motion
  and tail-jumps to the step sub-object's slot `+Ch`; `0078E480` sets the zoom and the FOV
  `f([00F889B4] / zoom)`; `0079F400` finds a vehicle camera by name (5Ch-byte elements).

## 3. Movie-camera tracks and keyframe events

Tracks (`+68h` key list, `+74h` camera, `+7Ch` current key; keys `+F0h` start, `+F4h` blend,
`+DAh` id). `00797A00` sets the camera clock `+3B0h` to the current key's start, stepping back
one key first when asked and the clock already sits there (`00796680` is the postfix `it--`, the
dereference reads the moved iterator, listing `00797A93..00797AB5`). `00797AF0` advances or
holds at `start+blend` against the next start (999999 / 999998 sentinels). `00797F50` and
`00798E70` compute the editor's key time span. `0078E8E0` stops the camera (`0042A910`, FOV
0.6981317).

Keyframe events: constructors `00799FA0`, `0079A040`, `0079A0E0` (sunk depth -10.0 from
`00CE6848`), the three identical set-entity bodies `007995F0`/`00799620`/`00799650` (observer
at event `+10h`), the name setter `0079C000`, and the four destructors (one body: `0042BCA0`,
base vtable `00D046AC`, pooled name; listings compared).

## 4. Session plumbing (segment 43)

- MovCam broadcasts `0076D390..0076E750`: `__thiscall(session)`, transport `[session+188h]`
  (`0076D3ED`), one stack message each; payload offsets in the header. `0076D6D0` also bumps
  session `+280h`.
- `0076CE40` / `0076CE70`: MT_MOVCAM_BLACKBAR serializers on the native bit cursor.
- `00772170` chat send (host relays via `00772020`; a client sends to the server and echoes to
  the chat screen `00E198B4`); `0076F8C0` / `0076F9E0` message constructors.
- Countdown `00771240`, `00772B30`, `00770940` (stop = `+28h` 1).
- Voice `00774DC0`, `00774E30`: in modes 1/2, stamp the sender's voice time and send to the team
  (in play, game `+5D4h == 0Dh`) or to every unmuted player.
- Start gate `0076B400`, `0076FEC0`, `007713A0`: the multiplayer frame-delta gate. It returns dt
  in single player, so cc9's mission-frame host (which returns dt) already matches mode 0;
  nothing here is bound.

## 5. Records backfilled for existing code

`src/band_0076_cc13.cpp`: `007864E0`, `00786BE0`, `00785F90`, `00785FC0`, `00778740` (complete
for their scope, the three small ones re-checked against the listing) and `00786150` (partial:
the accumulator step only). `src/hud_movie_camera.cpp` (cc9's file, not edited): `0078FCF0`
re-traced and recorded complete; thirteen more recorded as `reconstructed_partial_build_tested`
from that header's coverage note; `00799D70` is the keyframe's SetEntity in the image, of which
the source models only the resolution.

## 6. For cc9

Gameplay state, left alone (names only, no records):

- Units, groups, roles: `00778860`, `007788B0`, `007788D0`, `00779820`, `00779AA0`, `00779B60`,
  `00779C60`, `00779D50`, `00779EB0`, `0077A2F0`, `0077A5D0`, `0077A650`, `0077B050`,
  `0077BD70`, `0077C470`, `0077C880`, `0077C980`, `0077CA60`, `0077CC80`, `0077CD80`,
  `0077D270`, `0077D3F0`, `0077D580`, `0077F2D0`, `0077F360`, `0077F3F0`, `0077F480`,
  `0077F510`, `0077F7B0`, `00778820`.
- Recon levels: `00779850`, `00779BB0`, `00779D00`, `0077B0C0`, `00793670`.
- Orders and formations: `0077A020`, `0077A050`, `0077A080`, `0077A1A0`, `0077A220`, `007796F0`.
- Entity replication and dispatch: `0076DA60`, `0076F210`, `0076F2B0`, `00779780`,
  `00779E20`, `00779F80`, `00779FC0`, `0077B280`, `0077BA60`, `0077BCF0`, `0077C160`,
  `0077C540`, `0077CCC0`, `0077D7F0`, `0077EB50`, `0077F090`, `0077F5E0`, `0077FAD0`,
  `0077FE80`.
- Multiplayer scoring: `00780C50`, `00780DB0`.

## 7. Names changed

See the handoff for the list applied through `tools/ghidra_annotate.py`.

## 8. Handoff

(written at the end of the packet)
