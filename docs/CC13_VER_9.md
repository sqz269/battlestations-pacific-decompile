# CC13 VER 9: strike approach, image against host (cc13-ver9)

Packet cc13-ver9 was a read-only verification done for cc9. It follows `docs/CC13_VER_6.md`, which found that the
release gates match the bytes and that the strike gap comes from upstream. It asked two questions:

1. Why do USN21 dive aircraft reach aimglide at about 1080 m?
2. What keeps aircraft inside the AA envelope on the run-in?

Method:

- Each step was compared with the disk bytes (`bsp.py disasm-raw`) and the host source.
- `tools/x87_dc_form_scan.py` was run over `009FB800`-`009FBB1F`, `009C4220`-`009C447D`, `009C62B0`-`009C6FFB`,
  `009C5400`-`009C5800`, `009C1850`-`009C1C00` and `009D07B0`-`009D0B00`.
  - The only reg-reg FMULs are `009FBAD9 D8 C9` (span * scale, commutative) and `009C6875 DC C9`, which is
    already audited in ver6.
- Existing logs only; no runs.
  - `J:\PROG\battlestations-pacific-decompile-cc9-ships44\local\s46_e_u21d.log` (USN21).
  - `J:\PROG\battlestations-pacific-decompile-cc9-physics1\local\p6u1_gj.log` (USN01).

Status of the C++ added here:

- `src/strike_approach_ver_cc13.cpp` and `include/bsp/strike_approach_ver_cc13.hpp` are reconstructed and
  build-tested, behind `kStrikeCtlCruiseCapBound = false`.
- They are not wired. All seven callers are in `src/game_hosts_units.cpp`, which lua56 leases, so every host
  change below is an adoption note. Nothing is game-validated.

## Findings

| # | where | image | host | symptom |
| --- | --- | --- | --- | --- |
| 1 | `009FBA88`-`009FBABF` | limit = min(Ceiling - 50, ctl+394h) whenever `[state+4]+0Ch` is non-null | `has_squadron = false` at `game_hosts_units.cpp:19013` (torpedo attackrun), `:21662` and `:30454` (other moveto seams), `:22205` (the shared `st_cruise_pitch_009fba50` helper), `:24792` (dive moveto), `:25017` (dive attackrun) and `:27828` (torpedo moveto); the cap is 1450 | the dive run-in tops out at about 1388 m instead of about 1300. The torpedo moveto climbs to 1450 m, not the authored 500, at 37-50 m/s |
| 2 | `009C83F8`-`009C8417`, `009C8310` | engaged = latch \|\| (ctl+370h == 2 && target). At mode 1 the flight stays in moveto/follow until the latch, then enters flyabove directly (`009C8379`) | `:4669` pins mode 2 (known pin; `docs/FOLLOWER_ATTACK_HANDOVER.md` s.3, s.11) | the 16-18 km attackrun exists only in the host. 30 of 126 USN21 dive deaths are on its 1450 m leg |
| 3 | `009C3F92`-`009C3FF5` | B8 = BC = U(1.6, 1.8) * desc+268h, stream 1, after the +B4h draw | `:24287` pins 1.6 * r | the latch is about 130 m late on average, so aimglide starts about 55 m higher. Adding the draw shifts stream 1 |

### 1. The ctl+394h cap

`ECX` is `state+4` at every call checked:

- `009C43D9`, with `ESI = EDI+4` at `009C423D` (dive attackrun);
- `009D0A6F`, with `ESI = EDI+4` at `009D07CD` (torpedo attackrun);
- `009C1AFF`, with `ESI = EDI+4` at `009C19A8` (moveto).

So `[ECX]+0Ch` is the block `009F9CE0` copies from unit+9D4h. It is also the block the torpedo goaway reads at
`009D0E7C`, which the host already models as CruisingAlt (`:19483`). The comment at `:19412-19414` calls it a
different object, and the bytes contradict it.

Writers of ctl+394h:

| writer | value |
| --- | --- |
| `007F2BE8` (block init) | tuning+53Ch, General/CruisingAlt = 1000 |
| `009C8920` | DiveBomb/CruisingAlt = 1300 (`planeglobals.lua:578`) |
| `009D4A70` | Torpedo/CruisingAlt = 500 (`:611`) |
| `0089F550` | the Lua squadron altitude |

Measured, torpedo (`torpedo Cabot_sqn04 glide census`, `s46_e_u21d.log:55542-74677`):

- The leader's moveto commands 1450 from R = 16019 m to R = 7782 m.
- It climbs 35 -> 1450 m, and spends n = 251-1051 at 37-72 m/s. It closes 5026 m in 800 census ticks, about
  56 m/s on average, against about 72 m/s level.
- The image levels at 500 m.

Measured, dive (`009FBA50 terms` rows):

- The command is clamped at 1450 for the first 960 attackrun ticks.
- Every flyabove entry is at 1357-1392 m.

### 2. Death census, USN21 dive

The census joins `death row` to each aircraft's furthest state:

| furthest state | deaths | killer_cat 1 / 5 / 6 |
| --- | --- | --- |
| attackrun | 30 | 0 / 2 / 28 (alt 1371-1467 m, killer range 1.5-2.7 km) |
| flyabove | 44 | 18 / 2 / 24 |
| aimglide | 52 | 27 / 5 / 20 |

Most of these aircraft reach flyabove or aimglide. Those that survive the dive release: `Cabot_sqn03` and
`Cabot_sqn03|.-2` dived from 1060/1111 m and released at 165-262 m.

## Q1 answer: the 1080 m aimglide entry is authored

- `planeglobals.lua:579` gives DiveBomb `BeginAltRange = {1000, 1200}`, commented "e ket magassag kozott kezdi meg a
  leboritast": the push-over begins between these two heights.
- The image rolls in at `009C67B0` when the span `009C65FD` reaches 0, that is when 3-s R <= 0.7h + 200.
- From the logged flyabove entry and the flyabove descent at 0.8 * DropAngle (`009FB800` read whole), the roll-in
  lands in that band.
- Findings 1 and 3 together lower the entry to about 930-990 m.

Verified equal to the image:

- `009FB800`: both arms, the clamp to [0, reference] and the min against max(angle * k, floor).
- `009FBA50` apart from the cap.
- Approach+A4h = 0.95 * desc+188h (`009C3F00`).
- +ACh = tuning+4CCh and +B0h = 4D0h - 4CCh (`009C3F39`-`009C3F68`).
- The +A8h draw.
- The `009C7A94` decay, which is inert: ctl+39Ch is 1000 at init and 9999 after the profile.
- The attackrun near-field call `009C42B8`: mode 1, zero weights.

## Interaction with physics6 (V1 speed floor, R1 reversal floor)

- Finding 1 interacts. The torpedo climb at 37 m/s is the low-speed regime, and the 500 m cap removes most of it.
- Finding 2 interacts through the follow law's sink and lag (the E2 table).
- Finding 3 does not interact.
- None of the three touches pull-outs.

## Uncertainty

- ctl+394h's live value depends on the profile cadence and the +380h/+38Dh/+3A9h gates. The model takes the script
  value, then the task value, then 1000. If the block is per unit rather than per squadron, a wingman that never runs
  the leader-only profile reads 1000.
- The release counts are predictions, not results:
  - finding 1: dive +1-3, torpedo upward and possibly large;
  - finding 3: dive +0-2;
  - finding 2: not computable statically.

## Handoff

- **For cc9/lua56.**
  - Feed `cin.has_squadron` / `cin.squadron_limit` at the seven sites through
    `strike_cruise_altitude_with_ctl_cap_009fba9b`.
  - Add the B8 draw beside `kAttackDistDrawBound`.
  - Correct the `:19412` comment.
- **Open.** Whether unit+9D4h is per squadron; the image's follow law (`009BEE56` hold arm), which blocks finding 2.
