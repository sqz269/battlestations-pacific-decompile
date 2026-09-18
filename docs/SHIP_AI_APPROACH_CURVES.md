# The range curve the ship AI's standoff choice samples

Addresses: 00955A40, 009523C0, 00952530, 00954940, 009E6E80, 009E5530, 009F1BC0, 0095F080,
0095EB40, 00419010, 00CE3938, 00CE3CA8, 00CE3958

Packet `cc8_ship_ai_approach_curves`, worker `agent/cc8-ship-approach-curves`. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; Ghidra was **read-only** for this
packet: no rename, comment, prototype, function creation or save. Every descriptive name below is a
hypothesis, not a recovered symbol. `reports/ship_ai_approach_curves.json` carries the
machine-checkable rows (16 call rows, 0 failures under `tools/verify_report_calls.py`);
`include/bsp/ship_ai_approach_curves.hpp` and `src/ship_ai_approach_curves.cpp` carry the
projection.

Vocabulary follows `docs/SHIP_AI_APPROACH_UPDATE.md`: `nested` is the ring object, `unit` the
entity at `[brain+0AA8h]`.

## What the two objects are

They are **bare arrays of sixty floats**. No header, no vtable, no count: three independent reads
agree.

| read | evidence |
| --- | --- |
| `00954940` fills `3Ch` dwords from `this` and returns `this` | body `00954940-00954951`, the whole routine |
| `00955A40` bounds its index at `3Bh` and reads `[ECX+0ECh]` for the saturated tail | `00955A79 CMP EAX,0x3b`, `00955A83 FLD float ptr [ECX + 0xec]` |
| `009523C0` walks `param_1[0]` to `param_1[0x3B]` | the unrolled body plus the `iVar2 < 0x3c` tail loop |

The instances are `nested+12C0h` and `nested+13B0h`, `0F0h = 60*4` apart, and `nested+14A0h` (the
traffic list) begins immediately after the second. `009E5530` clears both when the nested object is
constructed: `009E55C3 LEA ECX,[EDI + 0x12c0]` then `009E55C9 CALL 00954940`, and `009E55CE LEA
ECX,[EDI + 0x13b0]` then `009E55D4 CALL 00954940`. The gap between those two LEAs is the object
size.

### The x axis

`00955A40` both subtracts and divides by the **same** double at `00CE3938`, which is `50.0`
(bytes `00 00 00 00 00 00 49 40`). The decompiler printed both operands as `_DAT_00ce3938` and
warned about overlapping globals, so the listing settles it: `00955A45 FLD double ptr
[0x00ce3938]`, `00955A4B FSUB ST1,ST0`, then `00955A6B FDIVRP ST2,ST0` against the value still in
the register. The index is therefore `x/50 - 1`, and **sample `i` is the value at `x = 50*(i+1)`
metres**: the curve spans 50 m to 3000 m.

That is exactly the domain of the 119-step scan (`x = 50` in steps of `25`, so `50 .. 3000`) and
exactly the sweep `0095F080` performs.

### What the samples mean, and who writes them

The producer is already read, in `docs/SHIP_AI_BEARING_RATING.md`, section "`0095F080`, the
60-sample range profile": `0095F080` forces its query range to `50.0f` and calls the
expected-damage estimate `0095EB40` sixty times, stepping the range by the same `50.0` double at
`0095F140`. `009F1BC0` is its only caller and calls it twice:

| site | `this` | output | `prefer_long_range` | countdown |
| --- | --- | --- | --- | --- |
| `009F2F11` | the own unit, `[owner+0AA8h]` | `nested+12C0h` | 1 | `nested+1220h`, re-armed to `1.5f` at `009F2F16` |
| `009F2FB1` | the target, `[owner+0B20h]` | `nested+13B0h` | 0 | `nested+1224h`, re-armed to `2.0f` at `009F2FB6`, gated by `target->vtable[5Ch](5)` at `009F2F3C` |

So the two curves are **expected damage over a 20-second window against range**: `nested+12C0h` is
what this ship can do to the target at range x, `nested+13B0h` what the target can do to it. With
`prefer_long_range` the own curve's sample `i` is additionally reduced by the whole number
`60 - i` (`0095F11A FISUB`), which tilts it toward long ranges.

No authored script row reaches these arrays. They are computed every 1.5 s and 2.0 s from the
unit's gunnery categories, the weapon class descriptors behind them and the target's armour, all of
which `0095EB40` reads. The keys are the ones `docs/SHIP_AI_BEARING_RATING.md` lists
(`DamageMin`, `DamageMax`, `WaterDamage`, `FireDamage`, `FireChance`, `Blast.*`), not a curve
resource.

## `00955A40`, the sample

`float __thiscall(float* this, float x)`, `RET 4` at `00955A66`, `00955A8A` and `00955AC2`, body
`00955A40-00955AC4`, result in `ST0`. **complete**. The `RET 4` is what fixes the argument list at
one float (checklist rule 7); `ECX` is the array.

```
t = (float)(x - 50.0)                          00955A41..00955A4F, FSUB then FSTP float
if not (t > 0) return samples[0]               00955A57 FLDZ, 00955A59 FCOMI, 00955A5B JC,
                                               00955A63 FLD float ptr [ECX]
u = (float)(t / 50.0)                          00955A6B FDIVRP, 00955A6F FSTP float
i = (int)u                                     00955A73 CVTTSS2SI (truncation)
if i >= 0x3B return samples[59]                00955A79 CMP EAX,3Bh, 00955A7F JL,
                                               00955A83 FLD float ptr [ECX + 0ECh]
return 00419010(0, samples[i], 1,              00955A94 FISUB against the saved index,
                samples[i+1], (float)(u - i))  00955A98 FSTP float, 00955ABC CALL 00419010
```

The three `FSTP float` stores are load-bearing: the quotient is rounded to float **before**
`CVTTSS2SI` truncates it, so the index is the float-rounded one. The projection rounds at exactly
those three points.

## `009523C0`, the peak

`float __fastcall(float* this)`, body `009523C0-00952525`. **complete**. A running maximum seeded
with `samples[0]`, unrolled eight wide for seven passes over indices 1 to 56, with a tail loop to
`0x3B`. Every test is `if (best < sample) best = sample`. Nothing tests for an empty curve.

`009E71EF` calls it on `nested+12C0h`, and `009E7244 FDIV` divides the own sample by the result, so
in the scan it is the unit's best expected damage at any range, the normaliser of the third factor.

## `00952530`, the effective range

`float __fastcall(float* this)`, `RET` at `009525B4`, body `00952530-009525B4`. **complete**. The
search runs **downward**: the index starts at `3Bh` with the pointer at `param_1+0E4h`
(`samples[57]`), each pass tests `[2] [1] [0] [-1] [-2] [-3]` and subtracts 0 to 5 from the index,
breaking on the first sample greater than `0.0`. With no hit the index and pointer both drop by
six and the loop ends at `while (-1 < iVar2)`, which is exactly ten passes and never reads below
`samples[0]`. The answer is formed at `009525A1 MOV [ESP],EDX` after `ADD EDX,1`, `009525A4 FILD`,
`009525A7 FMUL double ptr [00CE3938]`, `009525AD FSTP float`:

> **the longest range at which the curve is still positive**, or `0.0` when no sample is.

`009E71AE` calls it on `nested+13B0h`, and `009E71B3 FADD double ptr [0x00CE3CA8]` adds `300.0`
(bytes `00 00 00 00 00 c0 72 40`). The seed of the standoff scan is therefore **300 m outside the
range at which the target can still hurt this ship**.

## `00954940`, the clear

`void* __fastcall(void* this)` returning `this`, body `00954940-00954951`. **complete**. A
`3Ch`-dword zero fill and nothing else. It is the curve's only constructor. Call sites:
`009E55C9` and `009E55D4` (the two instances, in `009E5530`), `009E6E8F` (inside `009E6E80`) and
`009E839E` (in `FUN_009E8360`, not read by this packet).

## The scan at `009E71A5`, re-checked

`docs/SHIP_AI_APPROACH_UPDATE.md` read this arm before the curve objects were known. Re-checked
against the listing, its reading is **correct and needs no change**:

| what | listing |
| --- | --- |
| `EBX` is the target curve | `009E71A5 LEA EBX,[ESI + 0x13b0]` |
| `EBP` is the own curve | `009E71B9 LEA EBP,[ESI + 0x12c0]` |
| the seed uses the target curve | `009E71AC MOV ECX,EBX`, `009E71AE CALL 00952530`, `009E71B3 FADD double [00CE3CA8]`, `009E71C1 FSTP [EDI]` |
| the reference uses the own curve | `009E71BF MOV ECX,EBP`, `009E71EF CALL 009523C0`, `009E71FC FSTP [ESP+30h]` |
| `p(x)` is the own curve | `009E7215 MOV ECX,EBP`, `009E721A CALL 00955A40` |
| `q(x)` is the target curve | `009E7228 MOV ECX,EBX`, `009E722D CALL 00955A40` |
| a non-positive `p` skips the step | `009E7236 FLDZ`, `009E7238 FLD [ESP+14h]` (so `ST0 = p`), `009E723C FCOMI ST0,ST1`, `009E723E JBE` |
| the weight's y0 is `2.0f` | `009E7261 FLD float ptr [0x00CE3958]`, bytes `00 00 00 40` |

So `src/ship_ai_approach_update.cpp` was not changed by this packet. What the curve recovery adds
is the meaning: the scan minimises

```
max(1, them(x)) * (nested+1284h / us(x)) * interp(0, 2, 1, 1, us(x) / best_us)
```

which prefers a range where the target's expected damage is low, this ship's is high, and this
ship is near its own best.

## Host methods

`src/game_hosts_ship_ai.cpp`. The two curve objects now live on the controller as
`approach_curve_own` (`nested+12C0h`) and `approach_curve_target` (`nested+13B0h`).

| host method | native | what it does now |
| --- | --- | --- |
| `StandoffBinding::curve_base_00952530` | `00952530` at `009E71AE` | `ship_ai_approach_curve_effective_range_00952530` on the target curve |
| `StandoffBinding::curve_reference_009523c0` | `009523C0` at `009E71EF` | `ship_ai_approach_curve_peak_009523c0` on the own curve |
| `StandoffBinding::curve_primary_00955a40` | `00955A40` at `009E721A` | `ship_ai_approach_curve_sample_00955a40` on the own curve |
| `StandoffBinding::curve_secondary_00955a40` | `00955A40` at `009E722D` | `ship_ai_approach_curve_sample_00955a40` on the target curve |
| `Impl::ensure_approach_curves` | `00954940` at `009E55C9`, `009E55D4` | the two clears, once per controller |
| `ApproachUpdateBinding::refresh_approach_curves` | `0095F080` at `009F2F11`, `009F2FB1` | the two refills, gated by `nested+1220h` and `nested+1224h` and re-arming them with `1.5f` and `2.0f` |

Two substitutions are labelled, both recorded as unimplemented so a reader can see them in the log:

- the target refill's `target->vtable[5Ch](5)` gate at `009F2F3C` has no probe in this process, so
  the presence of a target stands in for it
  (`ShipAiApproach::curve_target_kind_005c [009f2f3c]`);
- the query blocks at `nested+127Ch` and `nested+1238h` are not modelled. Only the two constants
  the profile path writes are set, `20.0f` (`009F2EA1`, `00CE3930`) and `30.0f` (`009F2EB1`,
  `00CE38C8`); the rest of the `44h`-byte block is the zero it is born with. **Partial**: the fill
  is partial even once a device list exists.

The two `0095F080` sites sit in the span of `009F1BC0` past `009F1DBF` that packet
`ship_ai_approach_frame_state_tail` still owns, which is why they are bound in the host rather
than projected into `src/ship_ai_approach_update.cpp`.

## Validation

Build: `scripts/build.ps1`, Win32 Release, `/W4 /WX`, clean. Tests: `ctest --test-dir build/win32
-C Release`, all passing, with one case added to `tests/math_tests.cpp` for the index rule
(a 50 m boundary, a 175 m interpolation, the 3000 m saturation, the peak and the empty curve).

USN02, 3000 mission frames at 0.05 s:

| | before (main, 50370e8ab) | after |
| --- | --- | --- |
| gunnery | `shots=734 hull=180 deaths=2 total_damage=18525.6` | `shots=734 hull=180 deaths=2 total_damage=18525.6` |
| `curve_primary_00955a40` | `UNIMPLEMENTED calls=999600` | gone from the unimplemented list |
| `curve_reference_009523c0`, `curve_base_00952530` | `UNIMPLEMENTED` | gone |
| standoff census | absent | `choices=8400 curve_refreshes=2450`, fourteen ships, every one `first=300.0 last=300.0` |

**The gunnery census did not move, and it should not have.** The curve rules run, but their input
is zero, for a reason the packet can name exactly:

- `0095F080` called `0095EB40` **147000** times, which is `2450 * 60`, so both sweeps ran in full.
- Every one of those calls returned at the range gate `0095EB6E`: the ship AI host's
  `ShipAiFirepower::unit_max_weapon_range_0494 [0095eb62]` is a placeholder `0.0f` and the sweep's
  first range is `50.0`, so `b[0] >= [unit+494h]` holds at every sample. The log shows
  `UNIMPLEMENTED calls=147000` for it, and `category_device_count` is never reached at all.
- The target curve is therefore all zeros, so `00952530` answers `0.0` and the seed is
  `0 + 300 = 300`.
- The own curve is `samples[i] = i - 60` after `0095F080`'s `prefer_long_range` bias, so every
  sample is negative, `009E723C` skips every one of the 119 steps, and the `300.0` seed survives
  as the choice.
- The placeholder this packet replaced returned `0.0f` from `00952530` and produced the same
  `300.0`. That is why the two censuses are identical rather than merely close.

USN01, same settings, for the record: `summary mission gunnery damage queued_hits=23 dispatched=23
hit_records=23 hull=23 part=0 fires=0 floods=0 attributions=23 deaths=1 kill_credits=1
total_damage=220.0 first_hit=0.60 s`, with `standoff choices=0 curve_refreshes=0` because no ship
in that mission reaches the approach sub-state.

## Corrections

Appended to `docs/SHIP_AI_APPROACH_UPDATE.md` under "Corrections" as an appended section; that
packet's own text was not rewritten. Both entries confirm its reading rather than overturn it:
which curve is `p` and which is `q`, and the widths of the `300.0` and `2.0` constants.

## no_ghidra_function

none. All four addresses are the start of an existing Ghidra function, and every call site in
`reports/ship_ai_approach_curves.json` was checked against the live listing by
`python tools/verify_report_calls.py` (16 rows, 0 failures).

## Uncertainty

- The sixty-element extent is fixed by three independent reads and by the `0F0h` gap between the
  instances, but nothing in the image bounds-checks the samples, so a producer that filled fewer
  than sixty would leave stale floats for `009523C0` and `00952530` to find.
- `00955A40` rounds to float three times and the projection rounds at the same three points, but
  the image keeps the intermediate in an 80-bit x87 register. On a value within an ulp of a sample
  boundary the two can choose adjacent indices.
- `FUN_009E8360`'s call to `00954940` at `009E839E` was not read. It is in the traffic-record
  follow-up packet's territory, and whether it clears a third instance of this class is unknown.

## Corrections from packet cc8_ship_ai_firepower_inputs

Appended, not a rewrite. This packet's own follow-up list was right about one blocker and wrong
about the other. Evidence is in `docs/SHIP_AI_FIREPOWER_INPUTS.md`.

| was | is | evidence |
| --- | --- | --- |
| Follow-up `ship_ai_approach_scan_scale_1284`: "`nested+1284h`, the numerator of the scan's middle factor, has no producer in this process either." | It has one, and it is not a scan constant. `nested+1284h` is word 2 of the own unit's firepower query block at `nested+127Ch`, the per-shot damage cap, and it also caps every profile sample through `ship_ai_firepower_output_cap`. `009F1BC0` writes it, along with words 1, 3 and 4 beside it. | `scan-bytes "84 12 00 00"` finds `009E7286` (the reader) and two writers, `009F2A4E` and `009F2AAD`, both inside `009F1BC0`, against thirty other hits in `.text` as the positive control. `009F2A44 FLD [EBX+370h]` then `009F2A4C FSTP [EBP+1284h]`; `009F2AA2 MOVUPS` from `00CE3D64` (float `10000.0`) then `009F2AA9 MOVSS [EBP+1284h]`. `EBP` is `nested`: `009F2AD1 MOV ECX,[EBP]` then `009F2AD4 MOV ECX,[ECX+0AA8h]`. `docs/SHIP_AI_BEARING_RATING.md` had already named it in the `damage_cap` field comment. |
| Follow-up `ship_ai_firepower_device_list`: "until a unit's gunnery categories reach the ship AI host, the standoff scan cannot choose anything but its 300 m seed." | Correct, and now done. The gunnery host already ran `00956C20` per unit and was discarding `store_any_weapon_max_range` and `store_artillery_max_range`. Capturing them and exposing the category gun lists fills all sixty samples of both curves. | The USN02 census: `curve_own_nonzero=60 curve_target_nonzero=60 max_weapon_range=6136.0` for all fourteen ships, and the chosen range becomes 1450 m for Haguro, 1550 m for Jintsu and 200 m for the twelve destroyers, in place of 300 m for every one of them. |
| Validation: "The gunnery census did not move, and it should not have ... the curve rules run, but their input is zero." | The input is no longer zero and the census still does not move, for a different and further-along reason: the standoff range reaches `009E6870`'s slot weights and stops there. The ring scan's winner is slot 0 in both runs with the same commanded heading, because the tune block at `009E7489` and `009E784B` has no producer and every slot therefore scores alike. | The gunnery aim line is identical to the character across the two runs (`angle_sets=1169733 refusals=185667 shots=734`), as are the three ship AI summary lines and every per-unit ring-scan row. `ShipAiApproach::tune_slot_04 [009e7489]` and `ShipAiApproach::select_tune_reject_04 [009e784b]` are both `UNIMPLEMENTED calls=8400`. |

## Follow-up packets

- `ship_ai_firepower_device_list`: the ship AI host's `FirepowerBinding` answers `unit+494h` with
  `0.0f`, which makes `0095EB40` return at `0095EB6E` for every sample and starves both curves.
  Until a unit's gunnery categories reach the ship AI host, the standoff scan cannot choose
  anything but its 300 m seed. This is now the single blocking input for the whole standoff choice.
- `ship_ai_approach_scan_scale_1284`: `nested+1284h`, the numerator of the scan's middle factor,
  has no producer in this process either. Even with real curves, a zero there collapses every
  score to zero, and because the update test is `FCOMIP` then `JC` (keep when not strictly worse),
  the scan would keep the **last** sample with a positive own value rather than the best one.
- `ship_ai_approach_frame_state_tail`: `009F1BC0` past `009F1DBF`, which owns the two `0095F080`
  call sites this packet had to bind in the host instead.
- `ship_ai_approach_traffic_records`: `009E8360` and its `00954940` call at `009E839E`.
