# Retained scene runway F values — CC11

The active airfield creation reader now uses successful nonempty explicit `F`
payloads for `RunwayWidth` and `RunwayLength`, independently of diagnostic
tokens. The existing raw scans remain the fallback. Both dimensions and
`runway_from_scene` are written only when both reads succeed. This is a bounded
source data binding, not a port of the complete native airfield initializer.

Baseline: `a7cf82d9a`, packet `cc11_scene_runway_float_payload`. Owned address:
`006D3C10`. Owned tracked files are this document, its
[report](../reports/scene_runway_float_payload_cc11.json), and
`src/game_hosts_scene_contents.cpp`. Ghidra was read-only; target verification
used `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

## Native producer and stored-value path

All 175 assembly instructions and 108 pseudocode lines of `006D3C10` were read.
Its normal stored body ends with `RET` at `006D3EA1` (exclusive end `006D3EA2`).
`006D3C2B` retains the input object from ECX in ESI; `006D3C5A` sets EBX to
object+`72C`. `006D3C83..006D3C8F` selects holder kind 1; the bag is holder+`8`.
The kind-2 descriptor route is separate and unchanged.

| Key | Literal | Find call | Type 0 read | Other-type read |
| --- | --- | --- | --- | --- |
| RunwayWidth | `00CF8AE8` | `006D3D0E → 008F2260` | `006D3D19 CVTSI2SS [record+0C]` | `006D3D20 MOVSS [record+0C]` |
| RunwayLength | `00CF8AD8` | `006D3D39 → 008F2260` | `006D3D44 CVTSI2SS [record+0C]` | `006D3D4B MOVSS [record+0C]` |

The comparison is at record+`4`; zero is the integer record type. The native
nonzero branch is broader than the source's retained-`F` adoption. Lookup
results are dereferenced without null checks, so native missing-key behavior
is not a graceful skip or a default dimension.

`006D3D6A` calls `006BF0D0`. Its complete seven-instruction body obtains the
holder from `[ECX+80]`, stores the first float at holder+`B0` (`006BF0DC`) and the
second at holder+`B4` (`006BF0EA`), calls `006BC960` at `006BF0F2`, and returns
with `RET 8` at `006BF0F7`. This setter/refresh is supporting evidence, not a
new reconstructed body. No new x87, math, or original-ABI binding is claimed.

The report retains every direct call in the containing function plus the
setter's refresh call: **16 direct rows verified, 0 failed**. Three unresolved
virtual calls are explicitly excluded. The other calls include base scene
initialization `00955420`, air-ops setup/load `006C0D20`/`006CADD0`, skill
`00927A80`, shape construction `006D1990`, spatial services
`0098B7D0`/`0042E630`/`0098BA10`, allocation `00BF681B`, and gunnery construction
`00864580`. Reading their call sites does not reconstruct these dependencies
or establish the whole initializer's class/admission/lifetime contract.

## Connected source behavior

`retain_airfield_runway` is a private helper called from the existing
`SceneReaderBinding::instantiate_entity` `deck.is_airfield` gate. Logging and
the gate are unchanged. It selects case-insensitive explicit `F` plus
`has_float`; otherwise it requires the same nonempty diagnostics and calls
the same `scene_scan_float`, including for `I` and other raw types. It first
checks both properties' availability, then reads width and length with the
existing short-circuit order. Failure leaves both dimensions and the presence
flag unchanged. Bare `F;` remains raw/unavailable; no empty-F action is added.

The existing `GameUnitsHost` landing-holder reader consumes
`deck.runway_width`/`runway_length` into `width_b0`/`length_b4`
(`src/game_hosts_units.cpp:11737`). Its absence/refusal policy and subsequent
landing-sequencer math are unchanged. SOURCE skip/refusal for missing,
raw-empty, or malformed pairs remains host policy, not native fault parity.

The owning `SceneProperty` float survives ordinary group capture, merge, and
authored overwrite. `has_float` is SOURCE value availability, not native
declaration identity or a persistent read-success/empty-action flag. The
accepted [nonempty F parser contract](SCENE_FLOAT_NONEMPTY_PAYLOAD_CC11.md)
uses modern MSVC `std::sscanf("%f")` directly into float once, for admitted
finite ordinary C-locale decimal-prefix tokens in closed NUL-free input below
`400h`. Historical VS2005 CRT numerical/error/FP-status/extended-ST0 parity is
unverified. Empty-F existing-record context, duplicate/implicit type conflicts,
enum declaration/namespace storage, and Lua/VFS provider integration remain
separate unresolved contracts.

## Focused source evidence

The ignored probe includes the actual production scene-contents TU and a fresh
`scene_file.cpp`. MSVC Win32 `/W4 /WX /fp:strict` compilation and execution both
returned **0**. It links the prior immutable 77 support inputs (three libraries
and 74 objects); every pre/post-link hash and the response-file hash matched.
Active parser/library/consumer paths are fresh, and no enlarged `SceneProperty`
is passed to cached support objects. The executable is PE `014C` with an
embedded `asInvoker` manifest. Main's full build/CTests remain primary-owned.

- Installed 15 library files yield 22 SOURCE groups. In
  `universe/scenes/missions/usn/usn_1_marshall.scn`, `Airfield2` (`AirField`,
  line 1095) has groups `Common`, `LandingZone`, `MotherShipPlanes`,
  `CommandBuildingInferior`, `MultiEntity`; authored F420 length/F30 width are
  at lines 1193/1194. Resolved retained bits match the old SOURCE scans, and
  the same pair succeeds after diagnostic removal.
- One focused scenario checks owning group capture/redeclaration, prefix
  conversion, signed zero, mixed retained/raw input, exact existing raw F/I/
  non-I scans, malformed/missing/raw-empty failure, pair failure without reset,
  and unchanged unrelated deck fields. Bare empty F is explicitly unavailable.
- Fresh JM06 parsing finds 96 authored entities and zero authored runway bags.
  All 17 installed input hashes match before and after the probe.

Artifacts: `local/cc11_scene_runway_float_probe.cpp`, `.cmd`, `.obj`, `.exe`,
`local/cc11_scene_runway_float_scene.obj`, compile/probe logs. The report pins
their paths and hashes. This validates source bag retention through the actual
helper. It does not execute the whole scene emitter, unit class/mode admission,
native holder refresh, landing task, original CRT, ABI, or game differential.
Native allocator/global/EH/fault/reentry/ownership behavior remains external.
