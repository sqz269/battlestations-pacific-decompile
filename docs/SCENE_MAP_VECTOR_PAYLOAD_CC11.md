# World-map retained V3 payloads

Packet `cc11_scene_map_vector_payload`, baseline
`4d8c432649bab5e4398a6314edb4eceb9983ea06`. Descriptive names are hypotheses,
not recovered symbols. Native target: `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; all worker queries were read-only.
Evidence and exact artifact hashes: `reports/scene_map_vector_payload_cc11.json`.

The actual world-map reader reparsed `SceneProperty.values` through the separate
raw value decoder, even when the accepted explicit V3 parser had retained three
owning binary32 lanes. It therefore rejected a parsed present-empty V3 and let
changed diagnostic tokens replace a stored vector. Native `004E6C00` copies the
sixteen corner payloads directly. The source reader now does that after its
existing required-property/type checks. Missing data and type errors retain
their existing source boundary; source-authored raw bags retain the old decoder.

## Exact native data path

The stored containing function is `004E6C00..004E7212`, 337 instructions. Its
data-reading projection is already implemented by
`read_world_map_settings_004e6c00`; the full engine function is not reconstructed.

| Span / call | Observed contract |
| --- | --- |
| `004E6C0B/004E6C2F -> 008F2260` | Find BorderSizeX/Y; type 0 uses CVTSI2SS, other admitted scalar values use MOVSS from record `+0Ch`. Existing scalar handling is untouched. |
| `004E6C53 -> 008F2260`, tests at `004E6C58/65` | Find MultiPlayMapSizes and follow its subbag at `+0Ch`; absent record/subbag branches to `004E720E` before global stores. |
| `004E6C72..004E70D7` | Sixteen calls to `008F2260`, in NW/SE pairs for IslandCapture1v1/2v2/3v3/4v4, Duel, Escort, Siege and Competitive. |
| `004E6C77/7A/7D` | First record's three dwords at `+0Ch/+10h/+14h` are loaded directly. |
| `004E6C8A/6CA0/6CB5` | First triple is copied through MOVSS to `00E189A0/A4/A8`. |
| `004E70EF/7101/7113` | Last triple is copied to `00E18A54/58/5C`; all sixteen follow the same payload path, with no tokenizer/CRT numeric call. |
| `004E7121/7135` | Publish the previously read border sizes. |
| `004E713D -> 004D5BD0`, twelve subsequent `004C7150` calls | Existing engine application/border-object contracts; excluded from this source change. |
| `004E720E/720F/7212` | POP ESI, ADD ESP,14h, RET. ECX supplies the source Map bag; semantic return remains unestablished. |

All 32 direct call rows are mechanically checked in the new report: nineteen
property lookups in this data-reading portion and thirteen unchanged tail
calls recorded as external contracts, not as newly reconstructed behavior.
No new math, x87, native register interface or conversion-provider recovery was
performed for this packet.

## Source behavior and callers

`world_map_bounds.cpp::decode` first keeps the existing case-insensitive key/type
checks. When the admitted type is Vector3 and `has_vector3` is true, it assigns
the correct source type/reference kind and uses `memcpy` to copy the three
retained lanes into the existing `ScenePropertyValue`. It does not inspect the
raw diagnostic tokens. This preserves the accepted explicit parser's distinction
between absence and a present-empty positive-zero vector.

When a source-authored bag has no typed payload, the existing raw-token decoder
remains the fallback. Its Vector3 arm requires three tokens and uses the prior
`strtod`-to-float source projection. F/I scalars, missing/type errors, absent
MultiPlayMapSizes preserving output, inverted bounds, selection math, modes and
border policy are untouched. No shared decoder/helper was changed.

The reader is actually called by `GameAvoidZoneRuntime::rebuild` for avoid-zone
bounds and by mission-frame scene initialization for the Lua world border-zone
setup. Those callers require no edits. The header only corrects its description
of the value source; the interface and source object layout are unchanged.

## Focused production evidence

One ignored Win32 probe links freshly compiled actual `scene_file.cpp` and
`world_map_bounds.cpp` TUs with the existing core library. The same probe is
compiled before and after the bounded source edit, with `/W4 /WX /fp:strict`
and `/link /MANIFEST:EMBED`. Both executables exit 0 and have an embedded
`asInvoker` manifest. There are no new tracked tests or CMake changes.

The before executable demonstrates `empty_rejected=1` and
`diagnostic_reparse=1`. The after executable demonstrates both are 0, and
explicitly checks that the empty corner contains three positive-zero lanes.
Both retain missing-corner rejection, required-type checking even when a typed
payload is present, source-authored raw fallback, and absent-subbag output
preservation. This is actual source-parser/reader evidence, not an engine run.

The production parser reads actual installed `scene.props` and JM06 inputs.
Each has sixteen full corner triples: all NW `(-100,0,-100)`, all SE
`(100,0,100)`. Both variants reproduce all 96 lane bits against the retained
payload and prior source decoder. Border sizes remain `20000/20000` for
scene.props and `12000/12000` for JM06. There is no empty installed map corner
in these inputs; the empty-vector regression is a separate admitted fixture.
The scene.props discovery driver only selects its SceneRootProps body and calls
the actual property parser; it does not exercise the private library loader,
enum provider, VFS or full scene-host runtime.

The prior ignored vector/math regression executable was rerun and passed:
1199 V3 declarations (1196 full, three empty), 22 groups, and JM06 40 paths /
1163 points with zero errors. That executable predates this new reader change;
it is supporting parser/path regression evidence, not validation of the changed
reader. The focused before/after executable supplies the latter evidence.

## Admission and remaining boundaries

The accepted explicit V3 parser domain remains a complete finite ordinary
decimal-prefix triple in C numeric locale, or immediate-empty, using closed,
NUL-free fragments shorter than `400h` and an absent/compatible native type-7
record. Its modern MSVC direct-float `%f` provider is not the original VS2005
`00BF7533` scanner. Historical numerical/FP-status/error/extended-ST0 parity is
unverified; source corpus agreement does not establish native exactness.
This packet adds no conversion and makes no new original CRT claim.

Malformed/conflicting/structurally aliased inputs, native pointer faults/SEH,
global identity/reentry, allocator ownership, border-object lifetime, native ABI
and actual-game validation remain outside this owning source projection. Full
`004E6C00`, `004D5BD0` and its border tail are still partial or external here.
Enum/Lua integration still needs stable declaration identity, integer snapshots,
actual parser/provider context and consumer adoption; unused model host methods
were not activated. The prior `008F54F0` repair remains only decoded tail through
exclusive `008F566F`, with stored body still ending `008F5658`.

Worker compilation, focused fixture, installed input comparison and direct-call
checks pass. The primary owns the integrated full Win32 build. No game run,
native differential, Ghidra mutation or push was performed by this worker.

Primary integration: ac4197a647b28e631d95f3d295a99200814a02d8; actual main sources independently recompiled for manifested focused probes, PASS. Full MSVC Win32 Release and all three existing CTests passed. Executable SHA256 97b412a3aaf2d3550b95b9c23b7e29558d40b07599a1d17df5f7b82f0ff6a585. Original ABI, actual runtime binding and game validation remain unclaimed. Build receipt: J:\PROG\battlestations-pacific-decompile\local\cc11_flags4_map_integrated_build.log.
