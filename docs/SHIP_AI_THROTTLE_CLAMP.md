# Unordered clamp and order turn-limit behavior

Addresses: 00415620, 00811D80. Packet `orch6_throttle_clamp_p`.

The public `clamp_float_by_ref_00415620` now preserves an unordered value, and
`unit_ai_order_turn_limit_at_00811d80` sends an unordered upper bound through
its ordinary clamp arm. Both previously converted these inputs to finite
bounds. The C++ signatures and record types are unchanged.

| Routine | Native body, inclusive | Coverage |
| --- | --- | --- |
| `00415620` | `00415620..0041565E`; RET 4 at `00415654` and `0041565C` | Complete selection rule; existing by-value semantic interface |
| `00811D80` | `00811D80..00811E7C`; RET 4 at `00811E5F` and `00811E7A` | Complete existing reader; unordered arm correction, with FP limits below |

These are descriptive C++ names, not recovered symbols or binary-compatible
entry points. Both Ghidra bodies exist and have no observed call-flow gap;
this packet performs no Ghidra mutation or function definition.

## Native comparisons and access order

`00415620` receives a value pointer in ECX, a low pointer in EDX and a high
pointer on the stack. It returns ST0 and removes the high pointer with RET 4.
`00415623..00415631` load/spill/reload the value and low as float32.
`00415635 FCOMI low,value; JA` selects low only for ordered `low > value`.
Only after that fails does `00415639..00415645` fetch and float32-spill high.
`0041564B FCOMI value,high; JBE` retains value on less, equal **or unordered**.
The other path pops value and returns high. The selection rule is:

```cpp
if (low > value) return low;
if (value > high) return high;
return value;
```

Neither the former `value <= high` condition nor `std::clamp` is the evidence
for this rule. The x87 condition flags and the `FSTP ST1` at `00415657` establish
which operand survives. A quiet NaN value remains NaN; an unordered high
does not replace a finite value that passed the low test. An ordered equality
in the second compare retains value, including its signed zero. Native pointer access order is deliberately
not claimed by the existing value-parameter interface.

The reader has ECX = record and one stack pointer to XZ. It saves the record
in ESI at `00811D8C`; sub_a leaves ECX intact up to its clamp call. Sub_b
reloads ECX from ESI at `00811E49` or `00811E66`. Each live-byte gate is followed
by X/Z differences spilled to float32, squared-distance arithmetic on x87,
and one float32 sum spill. `FCOMIP 400,distance_squared; JBE` rejects equal,
greater and unordered distance. The source's strict `< 400.0` gate already
has the correct unordered selection.

`00811DD9` / `00811E3C` compare SSE zero against the upper bound. Their JBE
branches accept nonnegative **or unordered** upper bounds; only ordered
`0 > upper` selects swapped bounds and FCHS. The ordinary source condition
is therefore `!(0.0f > upper)`. The four calls are:

| Call site | Value | Low | High | Returned sign |
| --- | --- | --- | --- | --- |
| `00811DE6` | record+00 | sub_a+08 | sub_a+04 | FCHS at `00811DEB` |
| `00811DF3` | record+00 | sub_a+04 | sub_a+08 | unchanged |
| `00811E4B` | record+00 | sub_b+08 | sub_b+04 | FCHS at `00811E50`; early return |
| `00811E68` | record+00 | sub_b+04 | sub_b+08 | unchanged |

All belong to `00811D80` and target `00415620`. Sub_b replaces sub_a's
result when both slots qualify. Neither routine writes to the record or XZ.
The previous header/ledger also mislocated sub_b's FCHS at `00811E5B`;
that instruction is POP ESI. The FCHS is `00811E50`.

## Producers and actual consumers

The existing `UnitAiOrderRecord` and `UnitAiOrderSubRecord` declarations in
`ship_ai_navigation.hpp` are reused. No new layout/default is introduced.
`00815F30` sets record+04's timer, marks sub_a+0C, writes XZ at sub_a+10/+14,
and writes the two bounds at record+0C/+10. When its old position passes the
distance gate it copies sub_a to sub_b, whose base is record+24. These stores
do not sanitize NaNs. `0080E000` maintains record+00 from sub_a+00 and resets
both bound/flag groups on timer expiry. These producers were inspected as
external evidence; their full reconstruction is not changed or revalidated.

Native `009E3DCD` in `009E3C00` calls this reader using
`unit + A98h + 54h * [unit+B40h]`, formed from `[navigator+3Ch]` at
`009E3DB5..009E3DC1`. It stores the return at `009E3DD5`, multiplies it by
the two lateral-axis components, and adds those products into the working
XZ at `009E3DE2..009E3E03`. Thus a NaN retained here can reach the path-point
coordinates; replacing it with a finite bound is a concrete caller change.
The current `GameShipAiHost` path-follower adapter calls the same C++ reader
on its published order slot. No runtime host is edited and no mission NaN
occurrence or gameplay effect is claimed.

The public helper also has current C++ callers in `ship_ai_approach_update.cpp`
(native `009E6DBF` / `009E6E2A`), `ship_ai_arm_final_step.cpp` (`009DE8CD`),
and `ship_ai_navigation_arm_tail.cpp` (`009EED4B`). They inherit its corrected
return rule. The last directly returns a clamped stopping-distance ratio;
the escape-steering helper stores it as turn. Their surrounding arithmetic
and downstream NaN handling are outside this packet. The native xref
inventory contains 35 sites in 15 functions; bounded argument-setup listings
are retained for every site. This inventory does not promote those callers
to reconstructed or independently fixture-tested status.

## Original-byte proof and reproduction

The ignored fixture is one small differential probe: three direct clamp
inputs (NaN in each operand), and six complete reader inputs exercising the
ordinary finite arm, NaN blend, NaN upper bound in each slot, negative sub_b
with early return, all four clamp call sites, sub_b precedence, and unordered
distance rejection. Each is run with masked exceptions, round-to-nearest,
and x87 PC24 and PC53. These are selected caller FP modes, not a claim about
the live game's control word.

The installed executable's SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
`prepare_throttle_clamp_probe.py` compares all 316 code bytes and the original
400.0/30.0 constants with live Ghidra bytes through verified BSP CLI queries.
Its two complete spans are relocated into an owned executable allocation;
all four relative CALLs target the original clamp, and three absolute data
operands target the original constants. No native helper, OS service or
library operation inside these routines is substituted.

The probe checks exact returned float32 bits against both original execution
and concrete expected words. It checks preservation of all 54h native record
bytes (including four bytes beyond the represented C++ record), all 50h C++
record bytes, the query bytes, and x87 TOP. It restores the FP control mode
and frees its executable allocation. No game state, world owner or observer
is fabricated. All 18 invocation pairs and 2,136 result/preservation bytes
pass. The preserved old source fails 12 pairs. Win32 Release and both existing
CTests pass. Exact baseline differences and SHA256 manifests are in
`reports/ship_ai_throttle_clamp.json`.
The report gate passes all 40 rows (36 unique sites, including the four
reader calls repeated in the native relocation manifest).

From this retained worker worktree, reproduce with:

```powershell
python tools/ghidra_export.py verify-seeds
./scripts/build.ps1
python local/prepare_throttle_clamp_probe.py
cmd /c local\build_throttle_clamp_baseline.cmd
./local/throttle_clamp_baseline.exe  # expected failure of preserved old source
Copy-Item local/throttle_clamp_probe_records.jsonl local/throttle_clamp_baseline_records.jsonl
cmd /c local\build_throttle_clamp_probe.cmd
./local/throttle_clamp_probe.exe
python tools/verify_report_calls.py reports/ship_ai_throttle_clamp.json
```

Run each next executable only after its compile succeeds. Probe executables
use `/MANIFEST:EMBED`. Python requires the existing `pefile` and `capstone`
dependencies and the configured live BSP project. The baseline uses the
preserved pre-edit module object and the same core library; the final probe
links the CMake-built core. All fixture source, scripts, original bytes,
relocation metadata, logs and hashes remain under this worktree's `local/`.
There is no dependency on the O worktree for reproduction.

## Limits

The probe establishes these selected masked quiet-NaN paths, return bits,
input preservation and balanced x87 stack. It does not prove unmasked
exceptions, signaling-NaN status behavior, FP status-flag equivalence,
arbitrary pointer alias/fault/access ordering, or binary ABI replacement.
The existing reader's C++ squared-distance expression still does not encode
every native x87 intermediate spill; its unchanged expression is not an
all-input PC24/PC53 parity claim, especially near the radius boundary.
No other throttle/heading/math routine in the module is revalidated here.
No additional tracked tests, runtime consumers, global owners or geometry
callbacks are introduced. Build and fixture results are not game validation.
