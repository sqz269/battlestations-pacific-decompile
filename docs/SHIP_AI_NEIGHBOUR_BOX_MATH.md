# Avoid-box floating-point corrections

Addresses: `009EAFC0`, arc arm `009EB4D1`. Packet `orch6_neighbour_box_math_o`.

The existing avoid-box projection now preserves four native unordered branches,
the shrink producer's x87 arithmetic, and the arc's final product spills. No
runtime host, frame preparation, sector scan or near-box implementation changes.
The original field meanings, input types and complete normal control flow remain
in `SHIP_AI_NEIGHBOUR_BOX.md`; the corrections below supersede its conflicting
NaN descriptions and constant value.

| Native range | Coverage |
| --- | --- |
| `009EAFC0..009EB651`, excluding the separately indexed arc arm | Existing complete normal semantic control flow; this packet corrects the listed floating-point behavior and checks all output arms in a bounded fixture |
| `009EB4D1..009EB610` | Existing complete arc projection; final radius/axis products now use x87 and spill before adding/subtracting the center |

`009EB4D1` is not a callable native routine. `JBE 009EB4D1` at `009EB470` enters
with the parent's stack, ESI=node, EDI=settings+180, EBX=observed owner, and turn
in ST0. The arc shares the parent's return protocol. Ghidra indexes it separately;
its calls at `009EB4DE` and `009EB4F6` therefore belong to that indexed body in
the call report. The containing address span lists 468 instructions, including
the 93-instruction arc; both flow checks report zero gaps. There are no missing
callee definitions or orphan call sites in this packet.

The native entry is ECX=node plus nine stack dwords, with `RET 24h` at
`009EAFE5`, `009EB305`, `009EB4CE`, `009EB60E` or `009EB64F`. The only caller is
`009F10FF` in `009F0EA0`, whose actual argument stores at `009F10A0..009F10FA`
were rechecked. The existing typed interface remains a new semantic ABI. Argument
5 is unread; argument 8 is tested only in its low byte.

## Corrections established by the body and probe

| Site | Earlier C++ behavior | Native behavior now preserved |
| --- | --- | --- |
| `009EB2B5..009EB2C5` | Any shrink failing `shrink > 0` collapsed and set `no_pose` | `FCOMIP 0,shrink` / `JC` admits **positive or unordered** shrink; only ordered zero/negative collapses |
| `009EB296`, `009EB329` via `00415510` | Unordered minimum selected its first argument | The helper compares second against first and `JBE` selects the second, including unordered and equal operands |
| `009EB174..009EB182` | NaN closing speed was replaced by 1 | `FCOMIP 1,closing` / `JBE` preserves unordered closing; only ordered values below 1 take the floor |
| `009EB444` via `00415620` | NaN turn was replaced by the upper bound | The first ordered `JA` chooses the low bound; the second `JBE` retains value on unordered comparison |
| `009EB246..009EB2AB` | Double expressions could retain more precision than the native PC24 arithmetic | Reference multiplication, rate divide/multiply, and `1 - capped*projection_time` use the actual x87 sequence and float spills |
| `009EB594..009EB5A0`, `009EB5D2..009EB5DE` | SSE multiplication chose a negative NaN in the final projected offset | x87 product and binary32 spill preserve the native two-NaN selection before the center add/subtract |
| `00D1A8A0` read at `009EB466` | Constant described as exact 3-degree double | Actual bits `3FAACEEA00000000` are widened float `0.052359879016876220703125` |

The constant correction does not change the branch for the adjacent binary32
turn values: both old and correct constants lie above the preceding float and
at/below the recovered threshold float. It corrects the recorded native value.

## Shrink production and output order

The full refresh prepares the shrink inputs before the gate. It obtains the
cached model bounds when a model exists, writes the low-byte filter result to
node `+69`, checks vertical overlap with ordered `JA` branches, and obtains the
actual settings singleton. It writes extents 1 and clears node `+68` before
testing the projected path. A near-copy taken after this point normally keeps
`+68` clear.

It computes center range, subtracts near forward extent `+38` and self half-length,
and compares this gap with `self_half_length * settings+1A4`. For a large enough
gap, the existing reciprocal-length helper normalizes the center delta. Observed
velocity contributes to closing speed, while the `00414C60` call at `009EB144`
uses the self velocity pair: ECX was set to its argument slots at `009EB11D`.
Closing speed is the relative-velocity dot product plus `settings+1D0`, with an
ordered floor at 1. Each native float spill remains part of the calculation.

Arrival time is `max(settings+1A0, arrival_distance/closing)`. Projection time is
the spilled gap/closing value minus arrival time; slack is gap minus closing
times arrival time. Both must be strictly positive and ordered. Observed signed
body speed then produces travel via projection time and `settings+1BC`; travel
magnitude must exceed 1 before shrink is reached.

The reference getter runs first at `009EB241`, then its ST0 result is multiplied
by exact widened `0.05f` from `00D7A270` and spilled. A second body-speed getter
runs at `009EB259`; its spilled float has the sign bit cleared. The producer is:

```
reference = float(reference_speed * double_at_D7A270)
rate = float(max(abs(body_speed), reference) / near_forward_extent * settings[1C0])
capped = min(settings[1C4], rate)        // unordered selects rate
shrink = float(1 - capped * projection_time)
```

The arithmetic operations before each spill honor the caller's x87 precision;
one final C++ double-to-float cast is insufficient at PC24. In the focused finite
case with near forward extent `13.37f` and decay multiplier `0.37f`, the earlier
projection produced extent bits `40A3D71A`; original PC24 code produced
`40A3D718`. Restoring the producer's x87 operations resolves that difference.
This case already matched at PC53.

Ordered shrink zero/negative writes temporary extents 1, sets `+68=1`, and copies
the near center, axes, extents and heading into the avoid fields. Positive or
unordered shrink leaves `+68=0`, produces forward extent `shrink*near_forward`,
and produces beam extent `min(1, float(shrink*6))*near_beam`. The corrected minimum
therefore permits NaN in both projected extents. A later travel-limit rejection
can still copy the near box while leaving `+68=0`; NaN shrink does not itself
mark the pose invalid.

The straight arm preserves old projected heading `+64`. The copy arm updates it
from near heading `+40`. The arc writes the wrapped projected heading and new
axes, then computes radius and center. For a NaN yaw input, the original final
radius/axis multiplication selects positive NaN for the center offsets. The
earlier SSE expression selected negative NaN; both center words differed only
in sign. The new private x87 product helper preserves the native product/spill
boundary for both arc directions.

Owner-gone still writes only `+68=1,+69=1`. Vertical separation writes the near
center, extents 1 and cached Y bounds -1000, preserving axes and projected heading.
No-model input preserves the previous cached bounds. The fixture checks these
preserved fields as well as the changed geometry.

## Evidence and reproducibility

One ignored fixture enters the **full original `009EAFC0`**, with all nine native
stack arguments; it does not substitute M's geometry callback or call only a
shrink fragment. It executes nine original spans totaling 2,307 bytes:

- full avoid body including arc;
- `00414C60` length, `00419260` reciprocal length;
- `00415510` minimum, `00415550` maximum, `00415620` clamp;
- `00438AA0` angle addition, `006BC0C0` heading conversion;
- `0098A8E0` cached bounds copy.

The preparation script verifies each span and constant against the installed PE
and saved live Ghidra bytes, preserving a manifest of 19 direct-call relocations
and 25 absolute data operand relocations. The original square-root calls bind
the same genuine host CRT `_CIsqrt` boundary used by existing `vector_helpers`;
this does not prove independent host CRT versus original VS2005 sqrt parity.
The source host calls the original mapped reciprocal, heading and bounds helpers,
so these dependencies are actual helper execution, not stand-in geometry.

Settings, model presence, observed velocity/body speed/reference speed and yaw
are explicit fixture-owned services. Their arguments and ordering are recorded;
they are not fabricated runtime owners or claims of physics/observer parity.
The native input bridge follows the verified nine-argument layout, but does not
execute the enclosing `009F0EA0` frame or near-box producer. Near geometry and
the incoming self state are explicit fixture inputs.

The final 14 cases run at both PC24 and PC53, nearest rounding with exceptions
masked. Cases include finite/zero/negative shrink, NaN rate, NaN minimum first
operand, NaN closing speed, NaN yaw, finite arc, vertical separation, gone owner,
late near-copy after NaN shrink, no-model cached NaN bounds/filter false, reverse
arc, and the noncommensurate finite producer case. Native filter true uses raw
byte `80h`; argument 5 contains an unread NaN. All 1,188 canonical trace/node
words match. Every raw node byte is compared except the address-valued owner
word; initial padding and unrepresented fields retain their sentinel bytes.
Both native and source x87 TOP balance. The executable mapping is released.

The original source/header snapshots fail 11 of those same 28 invocations; the
corrected source passes all 28. Baseline and final records are retained separately.
From this worktree, reproduce with:

```
python local/prepare_neighbour_box_probe.py
cmd /c local\build_neighbour_box_probe.cmd
local\neighbour_box_probe.exe
```

`build_neighbour_box_baseline.cmd` builds the same final case set against
`neighbour_box_before.cpp` and its preserved header under `local/baseline_include`,
using the current core library for unchanged dependencies. The expected baseline
exit is nonzero. The probe scripts embed an executable manifest. The preparation
script requires Python `pefile`, `capstone`, the installed PE and the configured
Ghidra project; no retired worker path is needed. The report records source,
before/after snapshots, native spans, scripts, executables and log hashes.

Win32 Release, the two existing CTests and the live call gate pass. No tracked
test, Ghidra mutation or gameplay integration is added. This is bounded numerical
and branch evidence, not every-input floating-point parity: other existing double
expressions, unmasked exception/status behavior, arbitrary NaN payloads, native
ABI, physics/observer lifetime and callback mutation equivalence remain outside
the proof. In particular the existing settings host returns a value snapshot;
this packet does not establish equivalence to callbacks mutating the native
retained settings storage during a refresh.
