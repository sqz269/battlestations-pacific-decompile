# CC13_SEG67B: segment 67 plane AI states, second pass (packet cc13-seg67A)

Packet cc13-seg67A, 2026-10-18, on cc9's 2026-10-18 addendum terms (segment 67,
00996120-009F6060). The file names carry `seg67b` because `cc13_seg67a.*` and
`docs/CC13_SEG67A.md` already belong to the earlier cc13-seg67a lineage, and on
this case-insensitive file system `seg67A` would collide with them.

Status words follow AGENTS.md. The cc9 host files are read-only. A row that cc9
or another worker already implements is recorded ledger-only as
`existing_implementation_spot_checked`, and any mismatch goes to cc9 as an
adoption note.

## 1. Priority findings (cc9's three items)

### 1.1 The follow tick's release return, 009C1FFD (priority 1)

`009C1FD0 BSP_BotStateFollow_Tick`, `__thiscall(state, float dt)`, `RET 4`.
Read from disk bytes:

- 009C1FE2 sets cmd(task+18h)+26Ch to 2.
- 009C1FEA calls 009BFD70, the station; AL=0 jumps to 009C234E.
- 009C1FFD tests unit+C25h != 0, and 009C2006 tests unit+C20h > 0. Either one takes
  the release arm, and it does so **before** the follow law (009C2068 CALL 009BFEE0,
  009C2077 CALL 009BEE30).
- The arm writes cmd+2D0h=2, +2BCh=0.0f, +2C4h=0.0f, +2CCh=1,
  +2B4h=[task+8]+190h (TravelSpeed), byte +2B0h=0 and +2D8h=1, then returns. So
  while a release request is queued, the plane holds level flight at TravelSpeed
  instead of keeping station.

The host's `run_follow_tick_009c1fd0` (game_hosts_units.cpp:9330) runs the law
unconditionally and writes none of the seven stores. None of its callers gates on
C20h/C25h. The moveto-follow arm at :35102 returns at the right point but drops the
stores. Adoption note sent to cc9.

The issue stage 007CE9FD lies outside this segment. Its five guards (bytes
007CEA02-007CEA4B) are:

1. the session mode [E188A8]+1FE4h == 2;
2. unit+9E0h;
3. unit+C3Ah;
4. unit+5Dh;
5. unit+900h, which must be 4/5/6/7. 0042A7E0 shows that 4/5 means on the ground.

The host's single `guard_blocked` counter lumps all five together, so the 1224
blocked ticks on USNOS have not been attributed to a guard.

### 1.2 Land / park / land-abort (priority 2)

- **009B09C0, the land/abort on-ground arm 009B0E74-009B0F93.** The host's arm (:33246,
  behind `kLandAbortGroundArmBound`, OFF) matches the bytes, including float(pi/2)
  00CE3830, the COMISS/JA sign test, and the 00419010 yaw with ±1.1. The only
  host-only behaviour is the null-holder refusal.
- **0099B0BE-0099B113, the re-takeoff.** Its condition is:
  !(plane+72Ch)->vt[38h], task->vt[38h], (0042A7E0 IsOnGround || +900h == 6), and
  task->vt[30h]. When all hold, the tick pushes 009CFF40(DL=0) through 00999F50. The
  host's gate (:35681) agrees, taking task vt[38h] as "a head task exists".
- **009B3CF0, the land state rule.** 009B3D0B parks a plane whose +900h is 4/5 unless
  it is in park or abort, and the host agrees. One gap: the final->park edge
  (009B3DD9, 009B3370) is refused at :33497 even with `kLandParkStateBound` ON.
- **009B3370.** The host's predicate (`land_touched_20 && +900h != 7`) matches.

### 1.3 The torpedo goaway's done condition (priority 3)

The host's `torpedo_next_state_009d4030` and `torpedo_goaway_complete_009d3150`
match the bytes.

When squadron+369h (ReloadEnabled, default 1 at 007F2D09) and [00E17BF2] are both
set, 009D3150 is false whenever approach+132h is 0. 009D3420 rewrites +132h every
tick (009D34C5 007B93F0, kind 2Bh, at 009D486F, before the rule at 009D48DE). So
in the binary, a plane that has dropped its torpedo stays in goaway until a reload
restores the 2Bh bit or 009D3210 reports it disengaged. This is binary behaviour,
not a host defect.

## 2. Ledger-only records (existing implementations, spot-checked)

| Address | Name | Host body | Result |
|---|---|---|---|
| 009C1FD0 | BSP_BotStateFollow_Tick | `run_follow_tick_009c1fd0` | mismatch: release arm order and stores (1.1) |
| 009B09C0 | BSP_BotStateLandAbort_Tick | `run_land_abort_tick_009b09c0` | on-ground arm matches |
| 009B3CF0 | BSP_BotTaskLand_StateRule | `land_state_rule_009b3cf0` | matches except final->park |
| 009B3370 | BSP_BotStateLandFinal_ParkRule | inline predicate | matches |
