# 004B4B00 and 004C3CB0 in the HUD

cc9-platform2, 2026-09-25, packet `cc9_hud_camera_team_lists`. Names are descriptive hypotheses,
not recovered symbols. It follows docs/UNIMPLEMENTED_RANKING_2.md.

## 1. 004B4B00, the "camera unit": bound

`__cdecl`, no arguments, plain RET (listing 004B4B00..004B4B3C):
- `[00E188D8]`, the controlled unit, answering vtable +5Ch(5) returns itself (004B4B17).
- Else, answering +5Ch(18h) (004B4B2A, a squadron), it returns its `+3D0h`.
- Else it returns null.

**It reads no camera.** The host comments saying "there is no in-mission camera at game+19FCh"
misread it. The routine is a selector over the controlled unit.

**The host.**
- The minimap (005C154E) handed back the controlled unit, and the markers update (00643685) a
  non-zero token. Both kept their records. The markers runtime's own `camera_unit()` returns null,
  but nothing calls it.
- With `kHudCameraUnitBound`, both callers use `camera_unit_004b4b00` (`src/game_hosts_hud_world.cpp`),
  which gives the exact answer.
- The kind-18h arm needs a squadron's `+3D0h`, which this host does not expose. It stays a record
  (`*::camera_unit_squadron_leader`, 004B4B2A) answering none.

## 2. 004C3CB0, the "team unit list": not a mirror, left recorded

docs/LOCAL_PLAYER_UNIT_LISTS.md and docs/RECON_SLOT_LISTS.md section 3 settle the lists 004C3CB0
walks:
- the player record's `+30h` object holds five recon triples: own (+DD8h), enemy (+DE4h),
  neutral (+DF0h), unknown (+DFCh) and their union (+E08h);
- 004C3CB0 walks the heads of **own, enemy and unknown** (the heads +DDCh, +DE8h and +E00h).

So screen 29h's pick and the minimap consider detected enemies and unknown contacts as well as the
local side. The host's stand-ins do not:
- 29h walks the local party's live units;
- the minimap walks every created unit.

Binding 004C3CB0 means reading the recon lists, which live in the gunnery host (the recon slots,
`src/game_hosts_gunnery.cpp`). It is a real gap for the gunnery worker's queue, not a records-only
row, so it is left recorded here.

## 3. Predictions for the 004B4B00 pair, written before it

One tree (main 089eaa9d0 plus this), `local\bin\cu_off` against `local\bin\cu_on`, USN04
4700/4500, `BSP_GUNNERY_RNG_STREAMS=1`, 1600x900.

| row | OFF | ON |
| --- | --- | --- |
| HudMinimap::camera_unit | 9,160 unimplemented | 9,160 concrete |
| HudMarkers::camera_unit | 9,158 unimplemented | 9,158 concrete |
| `*::camera_unit_squadron_leader` | none | none (the Lexington is kind 5) |

- **Unimplemented total.** It falls by 18,318.
- **Summary lines.** All identical: both answers equal the stand-ins' for a kind-5 controlled unit.

## 4. The pair

`local\cu_off_usn04.log` against `local\cu_on_usn04.log`, both at 1600x900.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,399,232 | 2,380,914 (-18,318; predicted -18,318) |
| HudMinimap::camera_unit | 9,160 unimplemented | 9,160 concrete |
| HudMarkers::camera_unit | 9,158 unimplemented | 9,158 concrete |

- **Summary lines.** All 185 are identical.
- **Result.** Every prediction holds. **`kHudCameraUnitBound` is ON.**
