# Handoff: the fly-over's geometry, and the two addresses left unread

Packet `cc8_dive_heading`, branch `agent/cc8-dive-heading`. Read
`docs/DIVE_BOMB_TASK.md`, section "Packet `cc8_dive_heading`" (it is the last one) before anything
else; this file is only what that section does not say.

## (a) What is settled, and must not be re-derived

`009C62B0`-`009C7083` is walked whole from the listing: `local/f.ps1` drives `local/x87trace.py`
with frame base **0x98** and a callee table whose every `RET imm` and x87 effect comes from that
callee's own tail (`local/calleefx.py`). The walk reports zero join conflicts, zero unknown call
targets, zero notes and both `RET`s at depth 0 over all 949 instructions. Re-run it with
`./local/f.ps1 <from> <to>` for any window; `./local/f.ps1 all` rewrites
`local/output/fa_trace.txt`. **Do not re-seed the callee table by hand**: `009FB800` is `RET 8`
(`009FB94B`/`96B`/`BA4D`), and the `RET 10h` at `009FBB1A` belongs to a wrapper that calls it at
`009FBB13`; `009C6404` is the one indirect call that pushes nothing and returns a float in ST0.

Settled and recorded there: the span is `max(R - S, 0)` with `R` the planar range; `R` and the
bearing are taken to a three-second lead point; the complete table of writers of `+18h`/`+19h`/
`+1Ah`/`+20h`; the commanded heading as `C + clamp(turn, +/-L)`, a slew limiter; the retraction of
`cc8_dive_race`'s 0.01-rad hypothesis (that test lives inside the `007F0280` avoidance arm); and that
the `C := 210 m` clamp at `009C657C` is unguarded.

## (b) The two addresses to take next, in order

1. **`ctl+3A8h`**, read at `009C64D1` (`MOV AL,[EAX+3A8h]` with `EAX = approach+0Ch`) into base
   `[ESP+27h]`, zeroed at `009C64DD` when there is no controller. With `BL != 0` it makes
   `009C6554`'s `0F85` JNE skip `009C655A`-`009C67FE` - the altitude target, the leave test and the
   roll-in test - so `C` keeps the cruise altitude and the fly-over commands about 1000 m instead of
   210 m. That is the "holds the begin altitude and rolls over" the authored comments describe, and
   it is the only thing found in the whole body that can produce it. Its **producer is unread**.
2. **`flyabove+1Bh`**, written at `009C6813` and tested at `009C6532` with `009C6544`'s `75` JNE,
   which skips the same block by a different route. Also unread.

Two further holes, smaller: `flyabove+1Ch`, set at `009C6919` and tested at `009C691F` and at
`009C6DCD`, where `009C6DDA`'s `75` JNZ **skips the heading write entirely** - the host records
`suppress_heading_1c` as a contract ("this host keeps no flyabove `+1Ch`, so it never suppresses"),
and that is now a known hole. And the dead-band half-width `T` at `009C6A43`: three producers, only
`009C6674` traced; `009C6893` and `009C6911`, both inside the bank arm and both on the common path,
are not.

## (c) Traps this body sets

* base`[ESP+30h]` is the span only until `009C6853`, where the bank arm overwrites it with
  `classDesc+268h * 1.4`. Every read from `009C6881` on is a turn radius.
* base`[ESP+3Ch]` is the bearing until `009C64C9`, where it becomes the commanded altitude `C`.
* base`[ESP+10h]` is scratch, rewritten about forty times; never read it without the frame walk.
* `009C6336` writes base`[ESP+50h]` at `fb=0x9c` and `009C6DC1` reads base`[ESP+4Ch]` at `fb=0xa0`.
  Both spell `[ESP+54h]`/`[ESP+4Ch]` in the raw listing and they are different slots.
* `L` is `pi/2` except when `flyabove+1Bh != 0 && BL == 0`, where the 10 degrees at `009C6497`
  survives.

## (d) The runs

4800-frame USN04 (`--frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames 4800
--mission-frame-seconds 0.05`), in this worktree's `local\`:

| log | binary |
| --- | --- |
| `heading_before.log` | `dbda05ead`, the per-hand-over census only, no rule change |
| `heading_after.log` | the same plus `7b4c74771`, the span minuend and the lead point |

`heading_before.log` reproduces `cc8-dive-race`'s `race_dist.log` to the digit, so the merged tree is
a valid baseline for anything that branch measured. The hand-over line now carries
`span=`, `b=`, `f18=` and `f19=` so the arm that fired is named rather than inferred.
