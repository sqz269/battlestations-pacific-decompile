# Landing, pad and inferior range retained float words

Packet `cc11_scene_landing_words_float_payload`, baseline `5faf26916`.
Only the three existing scene reads now use the unchanged
`retain_scene_raw_word` helper. A successful retained explicit F supplies its
owning binary32 bits even after diagnostic tokens are removed. Legacy scans,
Source order LandingRange → LandingPointRange → InferiorRange, the existing
`created.instance` gate, independent assignments and defaults remain unchanged.
Ownership is `006F2780`, `src/game_hosts_scene_contents.cpp`, this document and
`reports/scene_landing_words_float_payload_cc11.json`; no header/API/test change.

## Native data path

The complete 110-instruction normal body `006F2780..006F2935` was inspected in
the preceding readiness audit. It retains ECX in ESI at `006F2781`, obtains the
holder at unit+C0 and bag at holder+8, and ends with POP ESI at `006F2934`, RET
at `006F2935` (exclusive end `006F2936`). The descriptive current name is
`BSP_CommandBuilding_InitializeFromProperties`; a semantic signature or return
type is not established by this data adoption. Ghidra queries used the existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, read-only.

| Key / literal | Find → callee | Null test / branch | Found word | Absent word | Unit store |
| --- | --- | --- | --- | --- | --- |
| LandingRange / `00CFAE30` | `006F284C → 008F2260` | `006F2851/53` | `006F2855 [record+0C]` | `006F285A 500` | `006F285F +7C4` |
| InferiorRange / `00CFAE20` | `006F2873 → 008F2260` | `006F2878/7A` | `006F287C [record+0C]` | `006F2881 200` | `006F288C +7C8` |
| LandingPointRange / `00CFAE0C` | `006F289A → 008F2260` | `006F289F/28A1` | `006F28A3 [record+0C]` | `006F28A8 500` | `006F28B3 +7CC` |

There is no record-type test or numeric conversion on any found path. This
supports memcpy of retained F bytes into the existing int32 word, rather than
a numeric float-to-int conversion. Native absence writes a default on each
initializer call. Source missing/empty/failed reads instead preserve existing
flags and words; this packet preserves that policy and does not claim repeated
native initialization or native holder/fault equivalence.

| Routine | Coverage |
| --- | --- |
| `006F2780` | Complete normal body inspected; reconstruction is partial, adopting only the three existing data reads. Base initialization `007482B0`, unit+758 setup `008ED720`, native storage/lookup and mode-dependent tail remain supporting external contracts. |
| `008F5BE0..008F5C91` and scalar constructors/clones | Previously accepted successful-nonempty F storage support, not newly reconstructed here. |

All 14 direct-call rows in the complete initializer reverified against live
Ghidra: zero failures. MinShootingRange is present natively (`00CFADF8`, Find
`006F28C1`, load `006F28CA`, absent 100 at `006F28CF`, store +7D0 at
`006F28D4`) and installed as I100, but lacks a connected current Source
field/reader/consumer. It is excluded. MaxShootingRange has no lookup in this
complete initializer or named current Source path; no global absence claim.

## Source connection and preserved policies

The raw fallback still uses exact uppercase I-first `scene_scan_int`, then
`scene_scan_float` even after a failed I scan; other raw type letters also try
the float scan. Only a successful read assigns `present=true` and the word.
Missing, raw-empty, malformed and bare-F-without-payload inputs skip without
reset. Each field succeeds or fails independently. Successful explicit F is
recognized case-insensitively with `has_float`; its diagnostic tokens are not
read. The helper definition is unchanged from the baseline.

The existing GameUnits creation sinks feed `landing_range_7c4`,
`landing_point_range_7cc` and `inferior_range_7c8`, with defaults 500/500/200.
Landing's getter converts the retained int32 numerically to float for active
AI/ShipAI landing-radius consumers, returning 500 for missing/wrong-kind units.
The enabled ScriptOrders building-pad pass calls the pad getter (500 for
missing/wrong-kind units), then `BuildingPadModel::adopt_landing_pads_006f5cc0`
and its existing wrapped-int-square predicate. The inferior getter (200 for
missing units) feeds enabled garrison and airfield-parent adoption. Those
getters, gates, marker order, holder math, comparisons and handlers are
unchanged and are not newly recovered or executed by this packet.

## Focused proof and installed effect

The ignored probe includes the fresh actual production helper/library TU and
a separately compiled fresh parser. MSVC Win32 `/W4 /WX /fp:strict` compile and
execution both returned zero; its embedded manifest is `asInvoker`. The actual
creation caller is compiled, while the probe directly calls the production
reader with the real record fields. Whole SceneReaderBinding, emitter, class
admission, downstream AI and game behavior are not executed.

- Installed 15 library inputs discover the existing 22 groups. CommandBuilding
  I500/I500/I200 and USN1 CB2 I1500/I1000/I1560 remain unchanged in Source field
  order. These are integer witnesses, not installed F behavior.
- A separate fresh F group has no inherited I conflict: 7.25tail/-0/2.5 retain
  `40E80000/80000000/40200000` after diagnostic removal. Owning copy/capture
  survives source-copy mutation and parent redeclaration; the new group values
  produce `3FA00000/40480000/C0800000`.
- Legacy I-first, failed-I float, raw F/R/lowercase-i, missing/malformed/empty,
  bare F, independent success/no-reset, defaults and unchanged Capture/Level
  sentinels pass. JM06 freshly parses 96 recursive authored entities with zero
  authored bags containing these three keys; this is authored inventory only.

All 77 immutable support inputs, the link response and 17 installed files have
matching pre/post hashes. Cached objects are uninvoked link dependencies;
active parser/helper/library paths are fresh. Probe artifacts use the unique
`local/cc11_scene_landing_words_float_` prefix; exact hashes and paths are in
the report. Root owns the full integrated main build/CTests and independent
integration probe; no worker full build or game run occurred.

## Admission and unresolved contracts

The [successful-nonempty F contract](SCENE_FLOAT_NONEMPTY_PAYLOAD_CC11.md) uses
modern MSVC `std::sscanf("%f", &float)` once for finite ordinary C-locale
decimal-prefix tokens in closed, NUL-free input shorter than 400h. The legacy
raw `strtod` fallback remains unchanged. Historical VS2005 CRT numerical,
error, FP-status and extended-ST0 parity is unverified. NaN/overflow/locale and
native allocation/fault/ABI domains are not admitted or newly recovered.

Existing-type/empty-F/duplicate/implicit-context rules remain a separate parser
dependency; bare F carries no retained payload or persistent read-action flag.
Ordinary owning group capture stays within its previously admitted unique-key,
compatible-type domain; unsupported forward/self/cyclic/structurally aliased
groups are not claimed. Enum declaration/namespace/storage/provider and Lua/VFS
integration remain incomplete. Prior native `008F54F0` tail decoding remains
qualified by its partial flow-repair receipt, not whole group-body/ABI proof.
This packet establishes a connected Source data adoption and focused fixture
proof, not native ABI compatibility or original-game validation.
