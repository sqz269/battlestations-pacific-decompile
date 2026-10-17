# CC13 fix 5: the follow band's SafeAlt floor (finding R3)

Switch: `kFollowBandSafeAltFloorBound` in `src/game_hosts_units.cpp:5776`, landed **OFF**.
With it OFF the host is unchanged. Grant: the cc9 gameplay lead, 2026-10-17. Scope: the band tail
only (image 009C16E5-009C183B, plus the state+88h value latched by follow Enter 009BED80). The R9
release arm in `run_follow_tick_009c1fd0` is not touched.

## Image, re-read from the listing

Follow Enter `009BED80`:

```
009bed84  CALL 0042E740            ; BSP_GameTuning_GetSingleton
009bed89  MOV  ECX,[ESI+6Ch]       ; the state's Pilot/Follow block
009bed8c  ADD  EAX,380h
009bed92  CALL 009BE150            ; copy singleton+380h.. into the block (float by float)
009bed9a  MOV  EAX,[ESI+6Ch]
009bedb0  FLD  float ptr [EAX+8]   ; block+08h = singleton+388h
009bedbb  FSTP float ptr [ESI+88h] ; state+88h, latched once at Enter
```

singleton+388h is `pilot_follow_safe_alt`, Pilot/Follow/SafeAlt
(`include/bsp/game_tuning_singleton.hpp:270`, loader row `src/game_tuning_singleton.cpp:234`),
authored 60 in this installation's planeglobals.lua line 498. The host already loads it; nothing new is
invented.

The band, `009BFEE0`'s tail:

- 009C16E5-009C16FB: a = float(block+04h + leaderY) (FADD then FSTP float), b = state+88h.
- 009C16FF-009C1725: FCOMIP a,b; JBE -> L = a; else L = b. So L = b only when a > b and ordered;
  an unordered pair keeps a.
- 009C1734-009C1740: c = float(leaderY - 120.0), with 120.0 the **double** at 00D1F3F8.
- 009C1744-009C175A: FCOMIP c,L; JBE keeps L; else L = c. So L = c only when c > L and ordered.
- 009C1779-009C17B9: the ceiling, min(singleton+210h, leaderY + 120.0), unchanged.
- 009C17C6-009C183B: station Y (state+34h) and steer Y (state+48h) clamped into [L, cap], unchanged.

No branch skips 009C171B-009C175A. The tail has three entries in 009BFEE0, all before it: 009C0021
(the latched/hold path, JMP 009C16D2), 009C123C (JMP 009C16C0, the lead-pursuit copy to +60h) and
009C15BB (JMP 009C16D2). A jump census of the whole body (disasm 009BFEE0, every `J* 0x009c16..18`)
finds no other target inside the tail. That settles U4 of docs/CC13_VERIFY15.md: the host's
"-120 only with Phase A" has no image counterpart.

Precision: every intermediate is stored to a float slot before the compare, and leaderY - 120.0 and
leaderY + 10 are exact in double for in-range altitudes, so the result is the same at 24- and 53-bit
precision control. No assumption about the unit job's thread is needed.

## What the switch changes

All three host sites that build this band:

| Site | OFF (unchanged) | ON |
|---|---|---|
| fly-to geometry, `src/game_hosts_units.cpp:9053-9059` | `state_88 = 1e30`; -120 only via `run_phase_a` (kFollowPhaseABound) | `state_88 = gt.pilot_follow_safe_alt`, `band_floor_image = true` |
| hold-arm stand-in geometry, `:8457-8460` | `state_88 = 1e30`, no -120 | same as fly-to |
| catch-up steer point, `:8416-8425` | floor = leaderY + LeaderFollowAlt | floor = min(that, SafeAlt), then max with float(leaderY - 120.0) |

`bsp::plane_follow_geometry_009bfee0` (`src/plane_follow_law.cpp:659-667`) takes a new input
`band_floor_image` (`include/bsp/plane_follow_law.hpp:264-269`, default false). When set, the floor is
the image's two compares with the JBE arms above, and the -120 bound applies whatever `run_phase_a`
is. When clear, the older code runs as before.

The other floats of 009BE150's copy: the band reads only block+04h (LeaderFollowAlt) and block+08h
(SafeAlt). The host reads LeaderFollowAlt from the same singleton offset +384h already, so nothing else
in the copy affects the band.

## What a pair run should show (ON against OFF)

- For a leader above 50 m, L becomes max(60, leaderY - 120) instead of leaderY + 10. Station and steer
  Y below leaderY + 10 are no longer lifted to leaderY + 10: wingmen whose formation station sits below
  the leader keep that station, down to max(60, leaderY - 120).
- For a leader below 50 m, L = leaderY + 10 as before (min picks it), bounded below by leaderY - 120.
- The spent-member done floor (the same tail on the dive-bomb done tick, 009C7278 -> 009C1FD0) is
  max(60, leaderY - 120) for a leader above 50 m, so a spent member can follow down to 60 m.
- Expected movement: follow-state wingman altitude rows on missions with squadrons (torpedo and
  dive-bomb follow ticks). No RNG draw is in this path.

## Uncertainties

- state+88h is latched at Enter; the host reads the singleton each tick. They agree unless the
  Pilot/Follow tuning changes between Enter and the tick (it is loaded once at 007E2A20; no reload path
  was checked).
- The catch-up inline band clamped only the steer Y; the image tail also clamps the station Y there.
  Closed under the same switch by fix5b below.

## fix5b: the station Y and the catch-up clamp order (same switch)

Two gaps a reviewer found, closed under `kFollowBandSafeAltFloorBound` (no new switch); with it OFF
the host is unchanged.

Image, re-read from the listing of `009BFEE0`:

```
009c17c6  FLD [ESP+2Ch] (Y = state+34h); FLD [ESP+38h] (L); FCOMI L,Y; FLD cap
009c17d4  JBE 009C17E0        ; L > Y ordered -> state+34h = L (cap not tested)
009c17e2  FCOMIP Y,cap
009c17e4  JBE 009C17EB        ; Y > cap ordered -> cap, else Y (unordered Y passes)
009c17f1  MOVSS [ESI+34h],XMM0
009c17f6-009c183b             ; the same select on the steer Y, state+48h
```

The follow tick `009C1FD0` calls `009BFD70` at 009C1FEA, which rewrites the station (state+30h..38h,
docs/PLANE_FORMATION.md), then `009BFEE0` at 009C2068, then `009BEE30` at 009C2077. So
`009BFEE0`'s own station read (009BFF3B, the catch-up k) sees the unclamped station, and `009BEE30`'s
read at 009BEE92 sees the clamped one.

Host (`run_follow_hold_arm_009bee56`, `src/game_hosts_units.cpp`):

- (a) With the switch ON, the station Y is clamped into the band before `in.station_local` is derived,
  on the catch-up and the stand-in hold paths alike. The catch-up k keeps the pre-band station
  (`station_local_pre_band`), as 009BFF3B does.
- (b) The catch-up steer Y uses the image order: `L > Y` -> L without testing the cap. The OFF code
  applied the floor then the cap, so for L > cap it returned the cap.
- The band math is one function now: `bsp::plane_follow_band_009c16d2` and
  `bsp::plane_follow_band_clamp_009c17c6` (`include/bsp/plane_follow_law.hpp`), which
  `plane_follow_geometry_009bfee0`'s tail also calls (same arithmetic as before, moved). The fly-to
  path already used the clamped `geo.station_y`.
- Ceiling NaN arm: the image takes [210h] on an unordered pair, the helper takes leaderY + 120. Only
  a NaN leader Y reaches it; left as it was.
- The size of the effect depends on the formations' station Y offsets, which were not measured. No game
  run was made (none allowed in this packet).
