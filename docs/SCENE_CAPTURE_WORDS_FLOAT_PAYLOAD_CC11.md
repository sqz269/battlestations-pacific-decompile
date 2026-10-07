# Retained CaptureRange/Value DWORDs — CC11

The called scene reader now copies successful explicit `F` payload bytes into
the existing `CaptureRange` and `CaptureValue` raw words without rescanning
diagnostics. Each field updates independently. All previous raw scanning,
presence, failure/no-reset, defaults, and creation-gate policies remain intact.
This is a connected source data binding, not a whole native initializer port.

Baseline: `bd64c7aed`, packet `cc11_scene_capture_words_float_payload`. Owned
native address: `006F2780`. The three tracked files are this document,
[the report](../reports/scene_capture_words_float_payload_cc11.json), and
`src/game_hosts_scene_contents.cpp`. Ghidra was read-only, verified against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

## Native stored-word path

The complete 110-instruction normal body `006F2780..006F2935` was read; its
terminal `RET` is at `006F2935` (exclusive end `006F2936`). `006F2781` retains
the ECX object in ESI. The holder is at object+`C0`, with the property bag at
holder+`8`.

| Key | Literal / Find | Present record | Absent key | Unit store |
| --- | --- | --- | --- | --- |
| CaptureRange | `00CFAE50` / `006F27F3 → 008F2260` | `006F27FC MOV EAX,[record+0C]` | `006F2801 MOV EAX,1F4h` (500) | `006F280C → +7A0` |
| CaptureValue | `00CFAE40` / `006F281A → 008F2260` | `006F2823 MOV EAX,[record+0C]` | `006F2828 MOV EAX,3E8h` (1000) | `006F2836 → +7A4` |

Both paths copy a DWORD without checking the record type or converting a
float numerically to an integer. A retained F7.25 therefore supplies
`40E80000`, and F-0 supplies `80000000`; neither becomes integer 7 or 0.

All **14 direct call rows verified, 0 failed**. The report includes base
LandFort/static-body initialization `007482B0`, the object+`758` child setup
`008ED720`, eight property lookups `008F2260`, and four effective-mode calls
`004BCA50`. Those broader setup/lifetime/admission services remain external;
call-site verification does not reconstruct their bodies or the complete
initializer. Level, LevelUpSeconds, other raw-word reads, and mode policy are
outside this change. No new math, x87, or original-ABI recovery was performed.

## Source binding and consumers

Private `retain_scene_raw_word` is called only for CaptureRange and CaptureValue
in the existing `created.instance` block. It copies case-insensitive explicit
`F` plus `has_float` through `memcpy`. Otherwise it preserves the old behavior:
missing or empty diagnostics skip; case-sensitive `I` first tries
`scene_scan_int`; failed I and every other raw type try the same
`scene_scan_float`; a successful float fallback copies its bytes. Failed
conversion leaves both the field's presence flag and raw word unchanged.
Successful reads assign both, independently of the other capture field.

The old SOURCE integer prefix behavior remains: raw I `17.25tail` gives integer
17, while I `.5tail` fails the integer scan and retains the float word
`3F000000`. These are preserved source fallbacks, not new native typing claims.
Bare `F;` remains unavailable/raw with no `has_float` or replay/action flag.

Existing connected consumers are unchanged:

- `GameUnitsHost` initialization copies the raw words to `capture_range_7a0`
  and `capture_value_7a4` (`src/game_hosts_units.cpp:14943`). The range getter
  converts that int32 to float, with existing class/index fallback 500; AI
  routes it into `AiTailArrivalValueInputs.capture_radius`
  (`src/game_hosts_ai.cpp:3910`). Its casts and math are unchanged.
- The value getter returns the int32, with existing class/index fallback 1000.
  `GameShipAiHost::Impl::build_capture_buildings` puts it in capture state
  (`src/game_hosts_ship_ai.cpp:12211`); the existing SOURCE capture tick
  compares against its float cast (`src/command_building_capture.cpp:88`).
  No capture tick or threshold arithmetic was executed or re-recovered here.

Native null keys write constants 500/1000. The SOURCE helper preserves existing
record state on a skipped read, as the old reader did; fresh record defaults
and the unit sink's absent-field defaults remain 500/1000. This does not claim
native repeated-initialization, malformed-input, or empty-F context behavior.

## Focused proof and admission

One ignored probe includes the actual production consumer/library TU and a
fresh parser TU. MSVC Win32 `/W4 /WX /fp:strict` compile and run both returned
**0**. PE `014C` and an embedded `asInvoker` manifest were verified. All 77
immutable support inputs (three libraries and 74 objects), their response file,
and 17 installed files matched before/after link/probe. Active parser/library/
retention paths are fresh; enlarged `SceneProperty` records do not cross into
cached support objects. Main's full build/CTests remain primary-owned.

- Installed 15 library files yield 22 SOURCE groups. `CommandBuilding` retains
  I500/I1000 (`commandbuilding.props:6`/`:7`). USN1 `usn_1_marshall.scn` `CB2`,
  with `Common`, `LandFort`, `CommandBuilding` groups, retains I100/I10000
  (lines 1300/1301).
  These integer witnesses match the old SOURCE fallback exactly.
- A separate fresh-F declaration, with no inherited I/type conflict, retains
  `40E80000`/`80000000` after diagnostic removal. It also checks owning group
  capture/redeclaration, prefix/signed-zero data, I-first/failed-I fallback,
  raw F/non-I/case policy, malformed/missing/raw-empty no-reset, independent
  field successes, and unchanged SOURCE defaults.
- Fresh JM06 parsing gives 96 authored entities and zero authored capture-word
  bags. This is input inventory, not native or host emitter/class admission.

Artifacts are `local/cc11_scene_capture_words_float_probe.cpp`, `.cmd`, `.obj`,
`.exe`, `local/cc11_scene_capture_words_float_scene.obj`, and compile/probe logs;
the report pins their hashes. There is no installed-F or capture-game claim.

The [successful nonempty F parser contract](SCENE_FLOAT_NONEMPTY_PAYLOAD_CC11.md)
uses modern MSVC `std::sscanf("%f")` directly into float once, for finite ordinary
C-locale decimal-prefix tokens in closed NUL-free input below `400h` and
compatible ordinary unique-key declarations/owning group updates. Historical
VS2005 CRT numerical/error/FP-status/extended-ST0 parity is unverified. Native
existing-record typing, empty-F context, duplicate/implicit conflicts, alias/
forward/cyclic group admission, enum declaration/namespace storage, Lua/VFS,
native class/kind/mode admission, allocator/EH/fault/reentry/lifetime/global
interfaces, original ABI, and whole-game behavior remain separate boundaries.
