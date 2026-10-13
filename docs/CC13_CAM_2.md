# Camera and session reconstruction pass over segments 45 and 43 (packet cc13-cam2)

Segment 45 (`0078D880-007A4860`: camera, movie camera, MovCam) and segment 43
(`0076AA00-00787040`: voice, MMultiPlayer session plumbing, multiscore, recon level). Code:
`src/engine_seg43_45_cc13.cpp`, `include/bsp/engine_seg43_45_cc13.hpp` (namespace
`bsp::cc13cam2`), registered in `cmake/startup.cmake`. It reuses `cc13cam::CameraRef`,
`MovCamMotion`, `MovCamTurn`, the limit blocks, the modifier classes,
`scale_to_length_00793730/00793800`, `camera_ref_resolve_00793ac0` and
`camera_direction_delta_00794750` from `src/cc13_camera_controllers.cpp`; the native bit cursor
(`native_bit_cursor_*`); `extract_camera_matrix_angles_0042d2e0`;
`ship_ai_firepower_wrap_angle_00605070`; `wrapped_angle_subtract_00438b10`; and
`band0076_cc13::PeerRows` / `movcam_angle_windows_contain_0078e2b0`.

Every name is a hypothesis. Nothing here is ABI compatible or game-validated, and nothing is bound
into a host: entities are host handles, engine helpers are host callbacks, NativeStrings are
`std::string`, linked lists are vectors. The x87 chains were traced from the disk bytes
(`local/tools_scene2/sc2_x87.py` of the cc13 tree, block by block, and instruction by instruction
where the linear trace muddled the stack) and written in double with a float cast at each native
`FSTP`; that matches 53-bit precision control when every product of two floats is exact. CRT
`sqrt` is `std::sqrt`. `tools/x87_dc_form_scan.py` over segment 45 lists 35 functions with DC/DE
forms; the one DC form in code transcribed here (`00799999`, `DC C9` then `FXCH`) was decoded from
its bytes.

## Status

| Item | Exported | Reconstructed | Build-tested | Validated |
| --- | --- | --- | --- | --- |
| CameraRef family (10), path-record observers (3), target link | yes | yes | yes | no |
| MovCam motion, look, turn (incl. `00797180`), callbacks, interpolators (17) | yes | yes | yes | no |
| MovCam pose publication, vehicle cameras, orbit point, offset approach (8) | yes | yes | yes | no |
| Movie-camera tracks (5), keyframe links (5), keyframe events (12 addresses) | yes | yes | yes | no |
| Modifier slots (9), ocean settings (3), log/interpolator/event/observer helpers (4) | yes | yes | yes | no |
| Session: MovCam broadcasts (7), BLACKBAR (2), chat (3), countdown (3), voice (2), start gate (3), sends/loading (8), slot reset | yes | yes | yes | no |
| Peer quality (2), MNetwork client and locks (7) | yes | yes | yes | no |
| Backfilled records: band_0076 (6), hud_movie_camera (14) | yes | existing code | yes | no |

Ledger: 111 new function records (source `src/engine_seg43_45_cc13.cpp`) and 20 backfilled
records, 5 names (4 renames and `00797180`). Call-site rows: `reports/cc13_cam2_calls.json`
(31 rows, 0 failed).

## 1. Camera references and path records

The camera reference (vtable `00D04660`, scalar dtor `00793900`) is an observer: base words
`+4..+10h`, `+14h` entity, `+18h` point, `+24h` radius. The observer helpers take ECX = the
observed entity and EDX = the observer object (`0078E3EA`, `0079034C`, `00797156`).

- `00793920` / `007939B0`: an entity whose `+5Eh` byte is set is dropped; point or radius set,
  the other cleared; registered when the entity survives. `00793A40` copies and registers
  without the `+5Eh` test. `00790320` (assign) does the same but never unregisters the entity it
  overwrites (listing `00790320..00790356`); the possible double registration is kept.
- `00793F10` is `00793AC0`'s non-radius branch on an arbitrary ref (point through the entity
  world, or, with the level flag, through that world with its translation replaced by
  `00F8758C..94` and `0085DAD0`). `00793CD0` stores such a transformed point and zeroes the
  radius. `00793DC0` and `00796F00` freeze the resolved point and clear the entity without
  unregistering. `00796BD0` is the observer base copy (vtable `00CE3C8C`, `+10h` = 1).
- `00793E60` builds a 4Ch-byte observer (vtable `00D043D8`) that registers even a null entity;
  its owner was not found.
- The path record (camera `+47Ch` pointer array) watches two entities, `+14h` and `+24h`
  (`00796F40` releases both). `0078E3D0` / `0078E410`, which the ledger called MovCam setters,
  swap exactly those two slots, each guarded against the other: renamed to the path record's
  anchor and second-entity setters. `00796F40` on release: anchor -> point frozen through
  `00793F10(level)`; mode 3 also moves `+28h` into the world and becomes 2; mode 5 adds
  `0042D2E0`'s x/y angles of the anchor world to `+34h/+38h` and becomes 4; second entity in
  mode 1 -> its position `+FCh` into `+28h`, mode 2.

## 2. MovCam

Fields as in `docs/CC13_CAMERA_CONTROLLERS.md` section 13. `007943D0` reads the limit block
`[+4D0h]` as `+4Ch` velocity cap, `+34h` step cap, `+50h` turn-rate cap, `+38h` turn-step cap.
Section 13 called `+4Ch`/`+50h` accelerations, which is how `00794B80`/`00794D60` use them; both
readings stand.

- `007943D0` integrate: |v| (sqrt of the float-spilled `(y*y + x*x) + z*z`, zero unless above
  `1e-10`) above `+4Ch` -> rescaled (`00793730`); `+3A0h += f(v*dt)`; the step from `+3B8h`
  above `+34h` -> rescaled; the same for the angles with `+3D4h`, `+50h`, `+3DCh`, `+38h`.
- `00794F70` braking steer: `r = |angles - previous| / +50h`; the wanted turn is
  `d - f(f(delta*r)*0.5)` per axis; its length times 1000 (`00CE47A0`), capped at 2.0
  (`00D7A308`), scales `+50h`; `00793800`'s vector is added to `+3D4h`. A non-null target arms
  the follow cone with growth 0.9 (`00CE3860`), not the 0.875 of `00794D60`.
- `00795300` look basis: identity, row 2 the normalised direction to the target, row 1 the
  global `00F8758C..94` unless `|row2.y| >= 0.9999` (`00D04680`), then the mover's `+84h` row;
  `0085DC80`.
- `00797070` look target (observer at camera `+10h`): vehicles map to their `+3D0h`; in mode 1 an
  even sub-mode `+388h` becomes odd; aiming stores `+3E4h` and the aimed angles `+4ECh/+4F0h`.
  `00797230` turn-by and `00797180` turn-to (no Ghidra function; the code between `00797070`'s
  padding and `00797230`) clear the target, set the wanted angles `+418h/+41Ch` (relative or
  absolute) and either snap or set the angular limit to `00414C60(angles - wanted) / time`.
- `0079D960` / `0079DA90`: the arrival callbacks (`+49Ch`/`+4ACh`, `+4A4h`/`+4B0h`): the name is
  copied, emptied (`00CE3A0C`), the argument vector taken and freed after `00887E50`.
  `0079D870` stores the secondary callback and sets the look target.
- `007901D0` applies one `007900B0` interpolator record: `u = f((clock-start)/duration)`,
  `value = f(from + (to-from)*u)` with the product on the stack, `to` once `u >= 1`; ids 1..5
  write `+50Ch, +510h, +504h, +508h, +514h` (turnH, turnV, walkH, walkV, zoom inputs), 6..8 call
  `005CB9F0`, `005CBA20`, `0078E480`. `0079F6D0` appends a record to `[+4D8h]`.
- Pose publication: `00799750` and `007998C0` both build `M = (RotX * RotY) * T` (`00413920`,
  operand order read from the stack offsets), store it at `+74h`, clear `+C8h/+10Ch`, invalidate
  the children (`0042ED50`) and push `+74h` into the camera node (slot `+34h`). `00799750` uses
  `-0.0 - rho` / `-0.0 - theta`; `007998C0` (`SetPosRotZoom`) sets mode 8, `+420h` = identity,
  resets the shake (`0042C4C0(0,0,0,1,1,1)`), converts degrees with `0.01745329238474369`, and
  ends with `00794000`; it sets no zoom.
- `0079F710` vehicle camera pose: `P` = row 3 of `(element +8h) * target world`; delta =
  `00794750(entity - P, target forward +ECh)`; window centres `f((lo+hi)*0.5 + (hi <= lo ? pi :
  0))`; out = `(|wrap(delta.pitch - centre_pitch)|, |wrap(delta.yaw - centre_yaw)|)`; the bool is
  `0078E2B0`. The bounds test is `count < index`, so `index == count` passes and grows the array
  (`005CD640`) by one element first. `007A0DD0` returns only the bool.
- `007A01F0` is a debug draw of the vehicle camera sectors (renamed from "UpdateDeckPosition").
  `0078FAF0` moves a value toward a target at `|d|*dt*1.4` without overshoot and integrates it;
  there is no angle wrap (renamed). `007905B0` builds an orbit point through `RotX(-b)*RotY(-a)`.
- `007904D0` wraps (`00605070`) and clamps theta to `+-pi/2`; `00794360` seeds position and
  `theta/rho = -0.0 - (x, y)` of `0042D2E0` on the mover matrix; `00794000` clears the motion
  and tail-jumps to the step sub-object's slot `+Ch`; `0078E480` sets the zoom and the FOV
  `f([00F889B4] / zoom)`; `0079F400` finds a vehicle camera by name (5Ch-byte elements).

## 3. Movie-camera tracks, keyframes and modifiers

Tracks (`+68h` key list, `+74h` camera, `+7Ch` current key; keys `+F0h` start, `+F4h` blend,
`+DAh` id). `00797A00` sets the camera clock `+3B0h` to the current key's start, stepping back
one key first when asked and the clock already sits there (`00796680` is the postfix `it--`; the
dereference reads the moved iterator, listing `00797A93..00797AB5`). `00797AF0` advances or
holds at `start+blend` against the next start (999999 / 999998 sentinels). `00797F50` and
`00798E70` compute the editor's key time span. `0078E8E0` stops the camera (`0042A910`, FOV
0.6981317).

Keyframe links: `00795BD0` (fixed position, only without a modifier), `007A0E00` (a partner makes
a look-at key; none makes a camera key with a new 7Ch-byte modifier from `007A06D0`), `007A2D30`
(start shift, clamped at 0, then append/pair; a camera key goes to `+498h` and pairs in `+414h`),
`007A2CD0` (append/pair, then `0076D6D0` when a session exists), `007A2BB0` (a camera key and a
linked look-at key sharing the id word `+3C0h`, which increments). `0078E540` replaces the event.

Keyframe events: constructors `00799FA0`, `0079A040`, `0079A0E0` (sunk depth -10.0 from
`00CE6848`), the three identical set-entity bodies `007995F0`/`00799620`/`00799650` (observer
at event `+10h`), the name setter `0079C000`, and the four destructors (one body: `0042BCA0`,
base vtable `00D046AC`, pooled name; listings compared).

Modifier slots: `0078E780` owner, type ids 1/2/3 (`0078E800`, `0079DC30`, `0079DCF0`),
`0079DD00` the gamecamera's camera, `0078E870` its tick (camera `+170h` slot 4), `0079CF80`
begin (`0079CBD0`, then camera slot `+124h` with the owner camera's `+3B8h`; this answers the
"+124h argument" open item of CC13_CAMERA_CONTROLLERS section 11), and the destructors
`0079DC40` (camera killed through `00926D90(1)`) and `0079DB90`.

Also: `007996C0` appends to `moviecamera.log` with a one-time `********` banner; `0078D940` /
`0078D9F0` copy an A0h-byte ocean effect settings record at object `+20h` / `+2Ch`; `0078E510`
deletes the ocean environment's `+E4h` member.

## 4. Session plumbing (segment 43)

- MovCam broadcasts `0076D390..0076E750`: `__thiscall(session)`, transport `[session+188h]`,
  one stack message each; payload offsets in the header. `0076D6D0` also bumps session `+280h`.
- `0076CE40` / `0076CE70`: MT_MOVCAM_BLACKBAR serializers on the native bit cursor.
- `00772170` chat send (the host relays via `00772020`; a client sends to the server and echoes
  to the chat screen `00E198B4`); `0076F8C0` / `0076F9E0` message constructors.
- Countdown `00771240`, `00772B30`, `00770940` (stop = `+28h` 1).
- Voice `00774DC0`, `00774E30`: in modes 1/2, stamp the sender's voice time and send to the team
  (in play, game `+5D4h == 0Dh`) or to every unmuted player.
- Start gate `0076B400`, `0076FEC0`, `007713A0`: the multiplayer frame-delta gate. It returns dt
  in single player, so cc9's mission-frame host (which returns dt) already matches mode 0.
- Sends: loading begin/end `0076C190`/`0076C230` (MT_LOADINGINFO 0Eh), `0076C790` kick,
  `00772330` to a player slot (local slot -> `0052CF60`), `00772AF0` to a remote player,
  `0076C500` queued broadcast, `0076D280` music level, `00770BD0` host relay to a team,
  `0076E340` slot reset.
- Peer rows: `00786010` row quality; `007865D0` smooths `+4..+Ch` from the transport's latency
  (`0.15..0.6`) and pending count (`8..16`) through `00419010` and `007862C0`.
- MNetwork: client open/disconnect (`+9Ch` = 2), the two tail jumps, clear-and-search, and the two
  transport locks (`EnterCriticalSection`, depth `+18h`).

## 5. Records backfilled for existing code

`src/band_0076_cc13.cpp`: `007864E0`, `00786BE0`, `00785F90`, `00785FC0`, `00778740` (complete
for their scope; the three small ones re-checked against the listing) and `00786150` (partial:
the accumulator step only). `src/hud_movie_camera.cpp` (cc9's file, not edited): `0078FCF0`
re-traced and recorded complete; thirteen more recorded as `reconstructed_partial_build_tested`
from that header's coverage note. `00799D70` is the keyframe's SetEntity in the image, of which
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
  `00779E20`, `00779F80`, `00779F90`, `00779FC0`, `00779FF0`, `0077B280`, `0077BA60`,
  `0077BCF0`, `0077C160`, `0077C540`, `0077CCC0`, `0077D7F0`, `0077EB50`, `0077F090`,
  `0077F5E0`, `0077FAD0`, `0077FE80`.
- Multiplayer scoring: `00780C50`, `00780DB0`.

## 7. Names changed (ledger and Ghidra, `tools/ghidra_annotate.py --apply`)

| Address | Was | Now |
| --- | --- | --- |
| `0078E3D0` | `BSP_MovCam_SetObservedPrimary` | `BSP_CameraPathRecord_SetAnchorEntity` |
| `0078E410` | `BSP_MovCam_SetObservedSecondary` | `BSP_CameraPathRecord_SetSecondEntity` |
| `007A01F0` | `BSP_MovCam_UpdateDeckPosition` | `BSP_MovCam_DebugDrawVehicleCameras` |
| `0078FAF0` | `BSP_MovCam_ApproachWrappedAngle` | `BSP_MovieCamera_ApproachOffset` |
| `00797180` | (no name, no function) | `BSP_MovCam_TurnTo` (ledger only) |

Prior Ghidra values: `local/ghidra-annotations-20261013T063104Z.json` in the worktree.

## 8. Handoff

Done (branch `agent/cc13-cam2`): 111 new reconstructions and 20 backfilled records across
segments 45 and 43, five names, build passing. About 210 of the 343 listed functions remain
without a record. In rough order of value:

1. **Segment 45, large camera bodies.** `0079C200` (named `BSP_MovCam_UpdateModePose`, 237
   lines) is not the MovCam: it reads the input actions and switches on `+214h` with offsets
   `+224h..+38Ch`; it is another camera class (the `+2F0h` zoom and `+254h/+258h` angles match
   `00790460`'s snapshot). `0079F9E0` (the `mov.cam.pos.` dump), `007906E0` (the sector debug
   draw), `00791360`/`007A2370` (keyframe bit write/read), `0079C040`/`0078FD70` (the Lua vector
   readers; `src/game_hosts_lua.cpp` only approximates them), the keyframe and track
   constructors/destructors (`00799B00`, `00799CA0`, `00799EA0`, `0079C910`, `0079CFC0`,
   `0079A190`, `0079A260`) and the MovCam constructor/destructor (`0079F480`, `0079D640`).
2. **`00797180`** should be created as a Ghidra function (it sits between `00797070`'s INT3
   padding at `00797176` and `00797230`); the ledger name is in place.
3. **Segment 43 session.** `0076FAD0` SetMode, `00772EC0`/`00774AB0` peer removed/added,
   `00773860` game setup message, `007702E0` host remove peer, `00776930` DLC check, the peer
   rows update `007866B0` (row reordering by record `+1Ch`), `00786390` CONNSTATE, the NetTable
   sync helpers (`00781430`, `007814E0`, `00781540`, `00781590`, `00781F60`), `MNetwork` constructors
   already called by `src/online_seg71_cc13.cpp`, and the small mopup helpers
   (`0076C450`, `0076C740`, `0076D220`, `0076E3C0`, `0076E7F0`).
4. **STL rows** (`STL_*`, `*_Tidy_*`, `PushPair`, `PointerArray*`, about 56): candidates for
   tags rather than records.

Nothing is bound into a host, so no game run exercises this code; all of it is build-tested only.

## 9. Follow-up packet cc13-cam2b

Branch `agent/cc13-cam2b`.

- `00797180` (`BSP_MovCam_TurnTo`) is now a Ghidra function, defined by
  `tools/ghidra_define_function.py 00797180 00797222` under the write lock (record
  `reports/cc13_cam2b_defined_functions.json`). The analyzer setting was not touched.
- Tags (`config/tags`, action `reviewed`, no rename): 41 named container helpers as
  `stl_instantiation` (evidence `cc13cam2b_stl`), 15 small named game helpers as `game_code`
  (evidence `cc13cam2b_game_named`: the pooled NativeString releases, the Lua variant pair copies,
  the three class-id tests, the seven modifier curve pair pushers, the atomic pointer append).
- Reconstructed (build-tested only), 5 records:
  - `00791360` / `007A2370`: the keyframe bit-stream write and read on the native bit-cursor
    codecs; the codec arguments (zero flag, signed flag, scale, bits) were checked at
    `00791377..00791393`. The image passes each float through `FLD`/`FSTP`, so an sNaN would be
    quieted; the source passes raw bits. The reader folds a parent that is being destroyed into
    the local point and drops it, and returns before the modifier step for an unknown type.
  - `0079C040` / `0078FD70`: the Lua vector-table test and the xyz reader (missing components are
    uninitialised stack in the image).
  - `007866B0`: the peer-rows update (row reordering by record slot with swap-and-rescan, probes on
    all rows, the 0.2 s quality period on rows 1..7, MT_CONNSTATE at 0.5).

Still open after cam2b: items 1 and 3 of section 8, less the functions above.
