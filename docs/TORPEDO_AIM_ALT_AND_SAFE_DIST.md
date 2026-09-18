# The glide slope's two inputs are the release altitude and the release distance

Addresses: 009D4850, 009D4874, 009D487A, 009D4884, 009D4886, 009D4890, 009D48A2, 009D48A6,
009D48AC, 009D48AF, 009D48BE, 009D48C1, 009D48CF, 009D2DA0, 009D2E14, 009D2E19, 009D2E1F,
009D2E22, 009D2E28, 009D2E2E, 009D2E57, 009BDE80, 009C2AC0, 009C18C0, 009A66C0, 009BCE50,
009BCEB8, 007E93C2, 00CF3F20.

Packet `cc8_torpedo_aim_alt_and_safe_dist`, owner `agent/cc8-torpedo-run-in`, on `792ec7b1e`.
Reading only.

`docs/TORPEDO_MOVETO_TICK.md` left the move-to glide slope with two labelled substitutions: the
aim altitude `approach+3Ch + approach+34h` and the distance `task+438h`, with a note that the
authored `Pilot/Torpedo/SafeDist` of 700 at the same numeric offset was suggestive and not
evidence. **Both are now read, and neither field is the torpedo task's.** The real pair is already
computed on this host's slot.

## 1. Where the torpedo task sets them

**The deciding instructions are `009D48AF` and `009D48A2`.**

`009BDE80` has four callers. The torpedo task's arm `009D4850 BSP_BotTaskTorpedo_TickArm` reaches
it at `009D48CF`:

```
009d4874  FLD   float ptr [EDI + 0x134]   ; approach+134h, the elapsed run time
009d487a  FLD   double ptr [0x00cf3f20]   ; 15.0
009d4880  FCOMIP ST0,ST1
009d4884  JBE   009d4890                  ; 15.0 <= elapsed -> approach+7Ch
009d4886  MOVSS XMM0,[EDI + 0x80]         ; else approach+80h
009d4890  MOVSS XMM0,[EDI + 0x7c]
009d48a2  FSTP  [ESP + 0x8]               ; third argument  = that distance
009d48a6  LEA   ECX,[ESI + 0x544]         ; this = the state at task+544h
009d48ac  FLD   [EDI + 0x78]
009d48af  FADD  [EDI + 0x74]              ; approach+78h + approach+74h
009d48ba  FSTP  [ESP + 0x4]               ; second argument = the release altitude
009d48cc  FSTP  [ESP]                     ; first argument  = the release altitude again
009d48cf  CALL  009bde80
```

So for a torpedo bomber:

| state field | value | what it is |
| --- | --- | --- |
| `+30h` | `approach+74h + approach+78h` | the **release altitude**, 12 m in this installation |
| `+34h` | the same | so `max(+34h + targetY, +30h)` is the release altitude plus the target's height |
| `+38h` | `approach+7Ch` or `+80h` | the **release distance**, `TorpReleaseDistNear` 450 or `Far` 650 |

The switch is the same one the aim tick uses at `009D18CD`: at or past **15.0 seconds** of elapsed
run time (`approach+134h` against the double at `00CF3F20`) the glide aims at `approach+7Ch`, the
near distance; before that at `approach+80h`, the far one. So the aircraft is aimed further out
early in its run and closer in later.

## 2. The same values at construction

The arm refreshes only the state at `task+544h`. The state at the other slot gets its values from
`009C2AC0` at construction, inside `009D2DA0`, and they are **the same expressions**:

```
009d2e14  MOVSS XMM0,[ESI + 0x7c]     ; approach+7Ch, from the same 15.0 branch above it
009d2e19  FLD   [ESI + 0x78]
009d2e1f  FADD  [ESI + 0x74]          ; approach+78h + approach+74h
009d2e22  MOVSS [ESP + 0x34],XMM0     ; -> `mode`, which the tick uses as rangeLow
009d2e28  LEA   ECX,[ESI + 0x14c]     ; the state is embedded at approach+14Ch
009d2e2e  FSTP  [ESP + 0x38]          ; -> `near` and `far`, both the release altitude
```

`009D2E36`'s `FST` then `009D2E3A`'s `FSTP` put that one value into both of `009C2AC0`'s `near` and
`far` slots, so the constructed state matches the refreshed one exactly. **Whichever state a
torpedo bomber flies, its glide slope is built from the release altitude and the release distance.**

Note also that `ECX` is `ESI + 0x14Ch` here, so the state is embedded in the **approach** record,
not at the `task+508h`/`+544h` offsets `docs/BOT_TASK_STATES.md` line 120 lists. Both are true of
different base pointers; this packet did not reconcile them.

## 3. `task+438h` is the depth charge's, and the 700 is a coincidence

`docs/BOT_TASK_STATES.md` line 160, the source of both substitutions, is step 3 of **"The
depth-charge per-tick, `009A66C0`"**, not of the torpedo arm. Its `approach+3Ch + approach+34h` is
the depth charge's aim altitude - line 262 draws it between `Pilot/DepthCharge/AimAltRange/1` = 20
and `/2` = 60 - and its `task+438h` is the depth charge's own field. `docs/TORPEDO_MOVETO_TICK.md`
carried both across to the torpedo task, and that was wrong.

**And the 700 is a coincidence, provably.** An exhaustive `store_census` over offset `438h` finds
only two writers below `00A00000`: `007E93C2` inside `BSP_GameTuning_LoadFromPlaneGlobals`, which
is the tuning singleton's `Pilot/Torpedo/SafeDist`, and `009BCEB8` inside `009BCE50`:

```
009bceb8  MOV dword ptr [ESI + 0x438], 0x00d20900
```

an **immediate data pointer**, almost certainly a vtable for a sub-object embedded at that offset.
So the task record's `+438h` is not a float at all, and its numeric agreement with the singleton's
`SafeDist` row is nothing but two objects having a field at the same displacement.

## 4. What this means for the wiring

`docs/TORPEDO_MOVETO_TICK.md` section 5 listed items 3 and 4 as labelled substitutions. **Both are
retired, and both values are already on this host's slot:**

* the release altitude `approach+74h + approach+78h` is what
  `src/game_hosts_units.cpp`'s attack-run branch already passes to
  `command_altitude_and_throttle`, and the descent census prints it as `base=12.00
  (74h=0.00 78h=12.00)`;
* the release distances `approach+7Ch` and `+80h` are seeded by
  `torpedo_seed_run_speeds_009d0484` from `kTorpReleaseDistNearSPNormal` 450 and
  `kTorpReleaseDistFarSPNormal` 650, and the aim census already prints `speed_80`.

The 15-second switch is `approach+134h`, which the host carries as `elapsed_134`. So the glide
slope can be wired with **no substitution at all** beyond the ones
`docs/TORPEDO_RELEASE_GEOMETRY.md` already labelled for the difficulty row.

For USN01's aircraft, 1488 m out against a 650 m far distance, the commanded altitude at `t = 1.0`
would be `12 + (1488 - 650) * 0.70` = about **600 m**, and it would walk down to 12 m as the range
closes to 650. That is the profile, with real numbers.

## ABI

* `009D4850` `BSP_BotTaskTorpedo_TickArm`, the arm; `EDI` is the approach and `ESI` the task at
  `009D4874`-`009D48CF`.
* `009D2DA0`, the torpedo task constructor's state setup; `ESI` is the approach at
  `009D2E14`-`009D2E57`.
* `009BDE80` `BSP_BotStateMoveTo_SetRanges`, three stores, as recorded.

## Uncertainty

* Which of `task+508h`/`+544h` and `approach+14Ch` name the same object. The arm's `LEA
  ECX,[ESI+0x544]` and the constructor's `LEA ECX,[ESI+0x14c]` use different base pointers and this
  packet did not reconcile them; the values written are identical either way, which is what the
  wiring needs.
* `009BCE50`'s identity beyond the immediate it stores.
* The other two `009BDE80` callers, `007B7BA0` and `009CD0D0`.

## Host methods

**None.** Reading only.

## Corrections

Appended to the doc it amends, and verified present there.

* `docs/TORPEDO_MOVETO_TICK.md` section 3, section 5 items 3 and 4, and its follow-ups 2 and 3.
  Its aim-altitude and distance fields were the depth charge's, read from a line of
  `docs/BOT_TASK_STATES.md` that describes `009A66C0`. The torpedo task's are the release altitude
  and the release distance, and both are already computed.

## no_ghidra_function

None. `009D4850`, `009D2DA0`, `009BDE80`, `009C2AC0`, `009A66C0` and `009BCE50` all have Ghidra
functions.

## Validation

No run: reading only, no behaviour changed.

## Follow-up packets

1. **Wire the glide slope** when the files free, now with no substitution.
2. **Reconcile `task+508h`/`+544h` with `approach+14Ch`.**
3. **The other two `009BDE80` callers.**
