# Owning explicit integer payload and existing word consumers

Addresses: `008F5A00`, `008F3710`, `006F2780`. Source baseline `1affd1099`.
Ghidra target verified through `bsp.py`: `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; read-only throughout.

The ordinary parser now stores one owning int32 and presence after a successful
nonempty single-token explicit I read with a closing semicolon. It uses the
unchanged `scene_scan_int` (`std::sscanf("%d")`) and keeps raw diagnostics.
Only the existing private `retain_scene_raw_word` prefers that payload. Its
seven real initializer fields can therefore retain integer data after their
diagnostic vectors are cleared. Retained F, exact old raw scans, missing/failure/
raw-empty/no-reset behavior, field order, selectors, gates and defaults remain
unchanged. No global scanner, enum map/provider or model method is added.

Four fresh actual Win32 TUs strictly compile. The ignored connected fixture is
prepared but **not linked or run**. Root must fully rebuild every SceneProperty
header consumer before linked verification; old generated copies can omit new
members regardless of observed sizeof/padding. Neither old 77-input support
nor pre-I Boolean integration binaries were used. Native ABI, whole initializer,
runtime and original-game parity remain unclaimed.

## Native evidence and coverage

Spans are inclusive first/last bytes with an explicit exclusive end.

| Entry | Inspected span | Coverage |
| --- | --- | --- |
| `008F5A00` | I arm `008F5B48..008F5BDF`; end `008F5BE0`; supporting per-key success reset `008F5AF4` | Partial: type-0 arm only. Declared type 0 bypasses explicit I lookup; otherwise peek/compare/consume I. Guard/force-read, successful existing +0Ch overwrite or fresh record. Implicit/context/error/other type behavior is not implemented here. |
| `008D8F10` | `008D8F10..008D8F32`; end `008D8F33` | Complete normal guard inspected, support only. Peek at `008D8F1A`, BF7533 call at `008D8F20`, actual `%d` format `00CE3A34`; conversion-count equality to 1, nonconsuming, `RET`. |
| `008D9AD0` | `008D9AD0..008D9B35`; end `008D9B36` | Complete normal read inspected, supporting existing raw typed-reader reconstruction. `%d` conversion; success byte stored before cached-token copying/consume state; failure whitespace recovery and false/zero return. `RET 4` at `008D9B21` and `008D9B33`. |
| `008F3710` | `008F3710..008F376C`; end `008F376D` | Complete normal body inspected, Source data binding only. Allocate 38h, type 0 at +4, supplied DWORD at +0Ch (`008F3730`), initialized bookkeeping and insert; both paths `RET 8`. Allocation-miss insertion and other callers' contexts remain external. |
| `008EF140` | `008EF140..008EF16C`; end `008EF16D` | Complete normal type-0 constructor inspected, support only. Input DWORD copied at `008EF151`, type 0; `RET 4` at `008EF16A`. |
| `008F4F60` | type-0 arm `008F4FF9..008F502C`; end `008F502D` | Partial: independent clone allocation, source DWORD +0Ch, constructor `008EF140`, ordinal +34h copy, `RET`. Type-0 jump-table entry at `008F52B8` is `008F4FF9`. Other arms/EH/faults remain separate. |
| `008F0700` | type-0 arm `008F0716..008F0725`; end `008F0726` | Partial: source +0Ch copied to destination +0Ch; `RET 4` at `008F0723`. Destination type dispatch remains supporting; declaration/type/namespace identity is not copied or inferred. |
| `006F2780` | `006F2780..006F2935`; end `006F2936` | Complete normal 110-instruction body inspected; Source adoption only the existing seven connected stored-word fields. Other setup/effective-mode fields and inactive MinShootingRange remain unchanged/unbound by this packet. |

The parser resets its ephemeral read-success byte at `008F5AF4`. At
`008F5B92`, a force-read flag either invokes the actual reader or allows the
nonconsuming numeric guard first. Guard miss gives EAX zero without setting
success. `008F5BB2` reads an integer; existing record +0Ch is written at
`008F5BC6` only on success. A missing record calls `008F3710` at `008F5BD6`.
Bare/failed existing assignments and fresh empty-zero semantics need declaration
context and remain deferred. A real newly created zero later copied as group
data would be a value, not a permanent read-success action. Source `has_integer`
marks retained data, not that ephemeral byte or a replay flag.

The original reader and guard reach BF7533 with `%d` at `00CE3A34`. Existing
`NativeSceneTokenValueCalls` explicitly exposes the host CRT boundary over
original raw tokenizer storage; that service is not silently substituted for
SceneLexer. The admitted Source binding reuses the existing modern scanner
once for ordinary in-range signed C-decimal-prefix text. Original VS2005 CRT
overflow, locale, error/status, recovery, alias and ABI behavior is unverified.
No new numerical, x87 or raw-tokenizer ABI recovery is claimed.

## Connected storage and consumer

SceneProperty appends `integer_value` and `has_integer`. The parser sets them
after its unchanged generic raw scan only for type I, one nonempty value and
current `;`, with a successful scanner result. Failed/bare/multitoken/unclosed
forms keep the old partial raw path with no integer payload. Mathematical
int32 range, NUL-free text and the verified `<400h` ordinary token domain are
admission requirements, not a new malformed-input recovery policy.

Existing aggregate/value copies, recursive owning vectors and compatible
whole-record assignment carry the new data. Eager group capture still takes
existing fields, ordered later-parent overwrite, then authored overwrite;
entity group merge independently fills missing fields. The admitted body has
no registry callbacks, so parsing cannot mutate captured parents. Native
type-0 clone/assignment evidence supports independent scalar storage within
compatible unique-key, already-present acyclic unaliased group inputs. It does
not establish original allocator ownership, hash order/ordinal or fault/rollback
equivalence. Missing/forward/self/cyclic/structurally aliased parents and
duplicate/type/scalar-child conflicts remain unsupported native domains.

Only `retain_scene_raw_word` adopts this data, beneath its retained-F branch.
Otherwise the exact old uppercase-I integer scan, failed-I float fallback,
non-I float fallback, and success-only present/raw assignment remain. Missing,
failed and raw-empty reads preserve existing sentinels. Parsed lowercase I is
recognized by the native-style keyword comparison; raw lowercase-I fallback
without a payload retains its prior behavior. Other actual integer consumers
still read diagnostics; no diagnostic-removal claim is made for them.

The compiled actual creation path already calls the helper for these fields:

| Key | Native Find | Native stored word -> unit field | Native absent default |
| --- | --- | --- | --- |
| Level | `006F27A8` | `006F27B1 -> +770h` (`006F27BE`) | 0 |
| LevelUpSeconds | `006F27CC` | `006F27D5 -> +76Ch` (`006F27DF`) | 10 |
| CaptureRange | `006F27F3` | `006F27FC -> +7A0h` (`006F280C`) | 500 |
| CaptureValue | `006F281A` | `006F2823 -> +7A4h` (`006F2836`) | 1000 |
| LandingRange | `006F284C` | `006F2855 -> +7C4h` (`006F285F`) | 500 |
| LandingPointRange | `006F289A` | `006F28A3 -> +7CCh` (`006F28B3`) | 500 |
| InferiorRange | `006F2873` | `006F287C -> +7C8h` (`006F288C`) | 200 |

There is no native type conversion on these +0Ch loads. Source retains its
existing created-instance gate, Capture/Level/Range->Pad->Inferior order and
selector loop. game_hosts_units.cpp already assigns the retained fields or its
defaults at spawn; that file and downstream math are unchanged. Native null
defaults are distinct from the Source helper's skipped-record/no-reset policy.
MinShootingRange has no connected field and is excluded; MaxShootingRange is
not present in this initializer. Native enum +28h declaration/namespace identity
and Lua remain blocked storage/provider contracts; no E value becomes an I.

## Installed witnesses and prepared verification

The pinned 17-file corpus contains 14 library `.props`, global.enums, USN1 and
JM06. Fresh authored lexical inventory finds 141 explicit I assignments,
zero bare I, and 27 distinct ordinary in-range signed decimal values. This is
input evidence, not proof that every original value/parser/consumer is exact.
Six connected keys each have two authored I witnesses:

| Source | Seconds | CaptureRange | CaptureValue | LandingRange | LandingPointRange | InferiorRange |
| --- | --- | --- | --- | --- | --- | --- |
| commandbuilding.props | 10 | 500 | 1000 | 500 | 500 | 200 |
| USN1 lines 1300..1305 | 600 | 100 | 10000 | 1500 | 1000 | 1560 |

Level is E CommandBuildingLevels:Basic in the library. The Source raw reader
skips that nonnumeric diagnostic; it remains without integer data or enum
identity. Matching a downstream default zero is not resolved native enum proof.

The ignored fixture at `local/cc11_scene_typed_int_payload/probe.cpp` compiles
the actual parser/private helper and keeps the actual caller connection in the
included production TU. It prepares checks for all seven fields, decimal
prefixes/-0, owning copy, cleared/opposing diagnostics, present zero versus
absence, following key, group capture/ordered parent/authored overwrite and
fill-missing. It also checks unchanged F data, B/F/V3 copy metadata, exact raw
fallback/no-reset behavior, twelve installed words after diagnostic removal,
Level E Basic Source skip, and USN1/JM06 source syntax. Bare/failed cases test
Source preservation only, not native empty/default/error contracts. No fake
enum/class callback is used, and the full SceneReader created-instance gate,
native initializer, units/AI/emitter and game are not executed.

Fresh parser, unchanged factory, unchanged air_operations and probe/actual
scene-reader TUs compile `/W4 /WX /fp:strict`, exit 0. `/showIncludes` pins 184
actual inputs (181 project source/header/recipe plus three Lua headers), all
stable pre/post, and 17 stable installed files. Compiled support inputs are
zero. Link/run/PE manifest/full main rebuild/all CTests remain pending root.
The unique recipe, commands, object/input hashes and log are under the same
ignored directory; native direct-call and diff checks are recorded in
`reports/scene_typed_int_payload_cc11.json`.

Dependencies: prior eager owning group capture, successful nonempty F and
connected Capture/Level/Landing word adoption, and owning B (fully rebuilt and
independently checked by root before this packet). The previous group merge
flow repair remains partial tail decode ending exclusive `008F566F`; stored
`008F54F0` body still ends `008F5658`, as recorded in its separate closed report.

## Primary integration

Main `f3027c0f415f6760a83eb3021b09728c86ab47a3` passed the full Win32 build and all three CTests after every SceneProperty consumer rebuilt. Root independently compiled four actual TUs; 182 current Source/header inputs, 77 current main link inputs, 17 installed inputs and the response file were stable before/after the manifested PE32 probe. All seven actual common-reader fields, copies and diagnostic-removal/opposing-raw cases, compatible group capture/overwrite/fill-missing, decimal-prefix/zero and legacy Source fallback/no-reset checks passed. Twelve installed I witnesses retained their words; Level E Basic stayed a separate unresolved Source skip with no enum provider. The actual creation connection compiled, while whole SceneReader/initializer/units/AI/game remained unexecuted. All26 native direct calls verified. Prior worker pending statements above are historical preparation receipts, now closed only within this bounded Source domain.
