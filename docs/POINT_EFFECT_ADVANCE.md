# Point-effect transform and sample advancement

Addresses: `00867D00`.

`advance_point_effect_00867d00` reconstructs the complete 475-byte body at
`00867D00..00867EDA`. The original ABI is `ECX = actual 114h point`, followed by
stack float delta and an unused reference DWORD; both exits use `RET 8` at
`00867ECD` and `00867ED8`. No meaningful return value is established. The existing
name `BSP_Effect_AdvanceTransform` remains a descriptive hypothesis.

The installed executable SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The complete function SHA256 is
`be0a788463577630bf0662686b3634ced8be81b4dfe2f76b4b278d2aecb1ed6c`.
The saved `bsp.gpr` / `battlestationspacific.exe` target was verified through the
repository CLI; all 475 bytes matched live Ghidra and the installed PE, including
both three-byte return instructions. The indexed direct caller is `00867EE0`.

## Actual ownership and dispatch

`PointEffectAdvanceRuntime` borrows the same rows, child-event dispatch,
`866440` manager global/lifetime access, parent-reference mapping, and scene
runtime used by construction and child updates. `PointEffectAdvanceNodes`
requires a pure mapping from the exact existing parent transform to its actual
`NativeNodeStorage`, exposing the live released byte `+44`. The current point
node is its existing `NativeNodeBinding`; no owner, count, registration, camera,
or native storage is synthesized by advancement.

1. When byte `+0A` is nonzero, run complete `866F50` restart. Only after it returns,
   execute `FLD delta; FADD age80; FSTP age80`. A thrown restart leaves age intact.
2. For a captured nonnull parent `+8C`, inspect its actual released byte `+44`.
   If clear, refresh its world through real `B6DB70` only when valid bit2 is
   absent. Capture the current point-node dispatch binding; multiply full
   `relativeD0 * parent.worldF0` through real `413920` into a local matrix. Then
   load captured virtual `+34` and invoke it on the **current** point node.
   The canonical multiply has no callback, so retaining that dispatch binding
   preserves the native table-before-multiply/method-after-multiply ordering.
3. If the parent released byte is nonzero, execute complete `42D9A0` on the
   actual owned parent field with consumed null, then complete `867B10` stop.
   Only after both return, set byte `+09 = 1`. Parent release precedes field
   clearing; stop exceptions do not set `+09` or undo prior state.
4. Continue into sampling. Advancement itself never increments `+88`, rewrites
   `relativeD0`, updates `cached_world90`, or looks up root/reference globals.

The virtual `+34` implementation is required; the established native plain-node
binding routes it to complete `B6E870`. This permits its actual world-change
callbacks. `B6DB70` itself currently consists of hierarchy recursion and matrix
operations and has no callback. No injected refresh hook exists in this source.

## Sampling and x87 order

Sampling tests the **full DWORDs** `+28/+2C`. If both are zero, timer `+48` stays
unchanged. Otherwise `FLD timer; FADD delta; FLD interval; FXCH; FCOMI; FSTP ST1`
compares elapsed while still in the x87 register. Only ordered greater samples;
equality, less and unordered store elapsed as float and return.

On sampling, discard the unspilled elapsed value. Copy current XYZ `+3C..44` to
previous XYZ `+30..38` with sequential x87 loads/stores. Capture current node
`+110`, refresh this captured node if bit2 is absent, and continue using it even
if external instrumentation were to replace the point field. Load all three
world-translation DWORDs into SSE registers before writing any current XYZ.

Derived values require signed DWORD `+88 > 1` and `COMISS delta,+0` ordered
greater. For velocity, spill all three current-minus-previous differences to
float first. Reload **current** timer after refresh, add original delta through
x87, and spill the span to float. Divide the already-spilled numerators using
the native x87 sequence, spill three quotients, then copy them through x87 to
`+5C..64`. No zero-denominator check or clamp is added. Displacement independently
recomputes and spills all three differences before x87 copies to `+68..70`.
Every successful sample resets timer to positive zero, even when count, delta,
or current flags prevent derived output writes.

The MSVC Win32 implementation uses explicit x87/SSE stages and preserves the
ambient precision setting. For example, timer=interval=1 and delta=2^-25 samples
with x87 extended precision, despite the sum rounding to1 if prematurely stored
as float. With x87 precision set to24 bits, that same gate does not sample.
SNaN current-to-previous copies must retain the native x87 conversion; sampled
world-to-current copies use MOVSS bit preservation.

The older `world_ocean` `EffectSampleState` helper now shares these age/gate/
derived-value stages. Its scope remains a semantic sampling adapter receiving
an already-sampled position. It does not implement the complete point owner,
restart, attachment, stop, or live-node acquisition.

## Native call sites

| Site | Target |
|---|---|
| `867D0D` | `866F50` restart |
| `867D49` | `B6DB70` captured parent refresh |
| `867D69` | `413920` full 4x4 multiply |
| `867D78` | captured node table `+34`, current point-node receiver |
| `867D7F` | `42D9A0` consumed parent replacement |
| `867D86` | `867B10` stop |
| `867DD7` | `B6DB70` captured sampling-node refresh |

## Validation and limits

Both changed source files compiled with MSVC Win32 `/O2 /W4 /WX /fp:strict`.
`local/point_advance_probe_ah.cpp` executes the complete original 475 bytes with
both `RET8` exits and explicit ESP checks. Its six direct callee adapters invoke
the same canonical restart, refresh, multiply, consume and stop implementations
used by C++. The indirect world-matrix adapter invokes the fixture's current
binding and real `B6E870`. Native pointer fields `+8C/+110` are temporarily
translated between actual raw nodes and their existing canonical companions;
the point's actual storage and atomic count remain the same.

The fixture starts with complete point creation, native definition/component
ownership, three native rumble events, actual node/string pools, and the same
singleton/insertion/lifetime domains. It compares all 114h owner bytes between
original and reconstructed advancement, normalizing only pointer fields
`+0C/+18/+84/+8C/+110`. It checks both x8764 and x87-24 precision: the finite
threshold, no-sample/no-flags branches, signed high-bit count, full DWORD flags,
SNaN copy bits, full 4x4 parent multiplication, released-parent stop, and restart.
A controlled current world-matrix override invokes real `B6E870`, then replaces
the actual point-node field and timer with existing live owners; the following
sample observes that replacement. No refresh bridge injects state changes.
Real teardown releases point/definition/components/events/nodes. The old semantic
helper separately passes the finite threshold regression using shared stages.

Evidence artifacts: `local/point-advance-byte-evidence-ah.json`,
`local/point-advance-probe-ah.log`, and `reports/point_effect_advance.json`.
Probe linking includes `/MANIFEST:EMBED`. Primary integration runs the shared
full build and existing tests separately.

This proves the stated original-byte and actual-owner fixture behavior, not a
native binary replacement or gameplay. FPU status words and unmasked exception
delivery were not compared. Captured owners/spans must remain valid; concurrent
mutation is outside this C++ binding contract. Native exception dispatch and
throwing native virtual0 are not executed; the canonical intrusive terminal
interface is nonthrowing. No physical-device rumble or rendered game result is
claimed.

## Primary integration validation

The combined strict Win32 build and both existing CTests passed. Complete original manager231-byte/RET8 versus C++ now composes this advancement with actual nonempty frame jobs, child updates, restart, released-parent stop, retirement and pending deletion. Normalized114h states and lifetime counts match. See [LIVE_EFFECT_UPDATE.md](LIVE_EFFECT_UPDATE.md) and `local/live-effect-frame-probe-ah.log`.

Recovered signatures, names and evidence comments were saved in Ghidra, checked by readback with prior comments preserved, and re-exported. Native exception ABI, physical device output and gameplay remain unvalidated.
