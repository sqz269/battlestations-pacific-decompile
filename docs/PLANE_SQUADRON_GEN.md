# USN04's rejected Kingfisher PlaneSquadronGen (packet `cc9_plane_squadron_gen_rejection`)

Addresses: 0046D3C5 (read only; the branch is in `BSP_SceneFile_ReadEntityBlock`, body
0046CF40..0046D927).

Worker cc9-side-ai, 2026-09-26. Ghidra was read only. The data is this installation's
`universe/Scenes/missions/USN/usn_19_coralus.scn` (2024-08-09) and
`scripts/missions/usn/usn_19_coralus.lua` (2024-08-26). The USN04 menu entry loads that scene
(log line `scene universe/Scenes/missions/USN/usn_19_coralus.scn ... 58 entities`).

## 1. Answer: the host matches the image, and nothing was bound

The rejected row is `entity "moviefisher" (PlaneSquadronGen)`, template
`USA\AIR\OS2U Kingfisher`, `SubType = Reconplane`. Its property block holds `Hidden = B true`.
It has no `Party` and no `WingCount`.

The image's scene reader drops every `Hidden` row at load:

```
0046d39d  CMP byte ptr [EDI + 0x4],0x0     ; the load honours Hidden
0046d3b3  JZ  0x0046d3cb
0046d3b5  PUSH 0xce5708                    ; "Hidden"
0046d3bc  CALL 0x008f2260                  ; property lookup on the entity bag
0046d3c1  CMP byte ptr [EAX + 0xc],0x0
0046d3c5  JNZ 0x0046d5e4                   ; set: past the creation, into the hidden-record branch
```

`docs/LUA_GENERATE_OBJECT_HOST.md` and `docs/SCENE_RECORD_MAP.md` already cover this branch. The
host reproduces it: the scene summary says `34 rejected ... 34 held back by Hidden (0046d3c5,
the GenerateObject pool)`, and every rejection in the census is a Hidden hold-back. The same
census counts the Kingfisher among them (`scene class PlaneSquadronGen seen=2 generated=1
rejected=1`).

The other PlaneSquadronGen row, `movieval` (a Japanese D3A Val, `WingCount = 3`), has no
`Hidden` line. It is generated at load, and the intro movie takes it by `FindEntity("movieval")`
at script line 3313 and kills it at line 3248.

**So at mission clock 0.05 the image has no Kingfisher unit**, neither on a catapult nor in the
air. The row is a phase-2 cutscene prop. The script's `luaMoveToPh2` generates it at line 2178:
- `GenerateObject("moviefisher")`;
- `SetInvincible`, `Scoring_IgnoreEntityKill` and `SquadronSetTravelAlt(1100)`;
- `PilotMoveToRange(Mission.moviefisher, Mission.Shoho, 3900)`;
- the movie camera targets it.

No class-table row, descriptor field, owner check or plane-class lookup is involved.

## 2. Why the Kingfisher never appears in the pairs

`luaMoveToPh2` is a blackout callback: `Blackout(true, "luaMoveToPh2", 3)` at script lines
573, 579 and 585.
- **USN04 4700/4500.** The phase-1 condition is not met within 225 s. There are no
  `luaMoveToPh2` arms.
- **E2, USN04 9200/9000.** The condition holds from about mission frame 4600 (230 s). The call
  is re-issued on every Think pass: 73 arms in `local\psm_on_e2.log`. The blackout summary
  reports 79 arms, 5 completions and 2 callbacks, the last one `luaIngameMovieBOStart`, so the
  callback never runs.

This is the case `docs/MISSION_BLACKOUT.md`, section "A re-issued blackout never calls back",
already records:
- `luaObj_Completed` leaves the objective active (`--obj.Active = false` is commented out).
- Every call re-arms the fade to 3.0001 s.
- The Think pass returns every 3.0 s of fixed steps.
- The image also leaves the callback to frame-time drift. The host's lockstep frames remove that
  drift.

So the Kingfisher's absence in these pairs is the lockstep harness, not the generator.

## 3. Consequences for the pick arm (`docs/PICK_SQUADRON_MEMBERS.md`)

Even when generated, moviefisher arrives at phase 2, long after 004C3CB0 has built the pick lists
once at load. It never reaches a pick list. USN04 cannot exercise the squadron member arm by
this row in the image or in the host.

## 4. Not verified

The host's `GenerateObject` route for a held-back squadron (`create_unit_from_scene_record_0046db4b`,
`docs/PLANE_SQUADRON_HOST.md` section 6, "unvalidated by a run") has not run on moviefisher. The
row carries no `WingCount` and no `Party`. The defaults it would take were not read. A run that
reaches phase 2 is needed first: a BomberWave == 5 trigger, or a harness frame time that is not
locked to the fixed step.

## 5. Records

- No switch, no code, no pairs. The predictions would be "identical" by construction.
- No names added.
- no_ghidra_function: none. 0046D3C5 is in 0046CF40.
