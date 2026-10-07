# Owning explicit Boolean payload and FakeAllocated

Addresses: `008F5A00`, `008F3940`, `006CADD0`. Source baseline:
`a7da55ba6`. Ghidra was read through `bsp.py` in
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

The ordinary Source parser now retains a canonical owning byte and presence for
one explicit `B` token equal to `true` or `false`, case-insensitively, followed
by `;`. Raw diagnostics remain available. The actual FakeAllocated reader keeps
its B type gate, uses that byte when present, and otherwise keeps the existing
`scene_property_bool` raw fallback. A present false value remains data even when
its diagnostic vector is cleared; a missing property remains absent.

This is a bounded data binding. Four fresh Win32 TUs strictly compile; the
connected probe is prepared but **has not been linked or run**. Root must rebuild
every SceneProperty header consumer before linking it. Old generated copy and
assignment code can omit the new members even if they fit old padding. No old
77-object support snapshot was linked or executed, and no native layout/ABI,
runtime or original-game compatibility is claimed.

## Native receipts

Every span below uses an inclusive first and last byte, followed by an explicit
exclusive end. Full caller/allocator/registration operations remain separate
from the inspected value path.

| Containing function | Inspected span | Coverage and contract |
| --- | --- | --- |
| `008F5A00` | `008F5D4F..008F5DDF`; end `008F5DE0` | Partial: B arm only. Existing type 3 dispatch, optional B match, recognized true/false, successful existing-byte overwrite, or fresh type-3 producer. Other typed/context/error/parser operations are unbound here. |
| `00467CC0` | `00467CC0..00467CE9`; end `00467CEA` | Complete normal keyword matcher inspected, support only. Peek `00467CC3 -> 008D8A70`, compare `00467CCE -> 00438E10`, consume on match `00467CD9 -> 008D8960`; both returns clean one argument (`RET 4`). |
| `008F3940` | `008F3940..008F39A0`; end `008F39A1` | Complete normal body inspected; Source adopts data only. Allocate 38h, set type 3 at +4 and byte at +0Ch, initialize fields, insert through `008F33F0`; both paths `RET 8`. Allocation-miss insertion is not a Source recovery contract. |
| `008EF1F0` | `008EF1F0..008EF220`; end `008EF221` | Complete normal Boolean constructor inspected, support only. Input byte copied to +0Ch, type 3, `RET 4` at `008EF21E`. |
| `008F4F60` | `008F4F8D..008F4FC1`; end `008F4FC2` | Partial: Boolean clone arm, support only. Fresh 38h allocation, source byte read at `008F4F9F`, constructor call at `008F4FA6`, ordinal +34h copy; `RET`. Other clone types/EH/faults remain external. |
| `008F0700` | `008F0736..008F0745`; end `008F0746` | Partial: Boolean assignment arm, support only. Destination-type dispatch at `008F0703`; source byte read `008F073A`, destination byte write `008F073D`, `RET 4` at `008F0743`. No declaration/type/namespace identity copy is inferred. |
| `006CADD0` | `006CB222..006CB249`; end `006CB24A` | Partial: FakeAllocated predicate. Key `00CF88C8`, Find at `006CB22D`, null/type-3 checks, byte +0Ch truth test. EBX-zero anchor `006CAE13` belongs to the reviewed mode-1 path. Stock/class/allocation/other modes remain external. |

The parser initializes a false temporary and an ephemeral match-success byte.
`008F5D8A` matches true, `008F5DA1` matches false; true writes 1 and either match
sets success. Existing records receive +0Ch at `008F5DC2` only on success.
Fresh records call `008F3940` at `008F5DD6`, including the no-match path. This
packet admits recognized literals only: it does not adopt fresh empty defaults
or existing empty/no-match assignment rules. Source `has_boolean` means owning
data is available; it is not that native success byte, native +2Ch, or a replay
action retained through group copies.

The keyword compare's normal nonnull path reaches CRT `_stricmp` through
`00438E10`. Source reuses the existing comparison for ordinary ASCII spellings;
locale, embedded NUL, native tokenizer buffers/reentry/faults and binary ABI are
outside admission. No numerical conversion provider is added.

## Storage, merge and actual consumer

`SceneProperty` appends `uint8_t boolean_value` and `has_boolean`. After the
unchanged generic raw scan, `parse_property_body` sets them only when the type
is B, there is exactly one recognized literal and the current token is `;`.
Bare B, nonliteral, multitoken and unclosed forms retain the existing partial
raw parser behavior and receive no owning Boolean payload. This is not native
malformed-input recovery or a contextual redeclaration implementation.

The existing Source `merge_property_block` copies missing records by value and
assigns whole compatible records on overwrite. Its nested blocks are owning
vectors. `PropertyLibrary::add_group` captures an existing bag, then merges
ordered parents with overwrite, then authored fields with overwrite. Parsing
the admitted body has no registry callback; it cannot mutate parent snapshots.
Entity group merge separately fills missing fields. Those existing operations
carry the new byte and presence without shared pointers or action metadata.
Native clone and assignment independently copy the Boolean byte, supporting
this binding within compatible, unique-key, ordinary explicit B records.

Missing/forward/self/cyclic/structurally aliased group parents, duplicate keys,
scalar-child conflicts, implicit declarations, existing empty context and
declared-type conflicts remain outside the native claim. The Source group's
validation/exception behavior is not native fault, rollback or partial-publish
proof. Native enum declaration +28h and namespace/provider identity remain a
separate incomplete storage integration; no enum map or model method is added.
Current SceneProperty has owning F/V3 and this B payload; I remains the existing
raw/scanner path, distinct from the unused model's integer field.

Only `read_scene_deck_006cadd0` adopts the new payload. Its production calls at
the held-back and created entity paths remain connected to
`air_ops_load_from_scene_006cadd0`; the latter's existing true branch stores
state 6 and a zero timer for a fresh Source slot. NumSlots/MaxInAirPlanes,
Type/count/Arm, gates, defaults and launch-request behavior are unchanged. The
global `scene_property_bool` and its Hidden/MultiType/director callers remain
raw-backed; no diagnostic-removal claim is made for them.

## Installed inputs and pending connected proof

The pinned installed corpus contains 14 library `.props` files, `global.enums`,
USN1 and JM06 (17 files). A fresh lexical inventory finds 579 closed recognized
B assignments: `false` 460, `true` 113, `TRUE` 3, `FALSE` 3. That is authored
input evidence, not native parser execution or game admission. In `ship.props`,
MotherShipPlanes' eight FakeAllocated values are B false at lines
113/120/127/134/141/148/155/162. The previously integrated predicate fixture
verified false flags, NumSlots 4/MaxInAirPlanes 12 and four Source downstream
slots; this packet's owning-payload execution is pending.

The ignored connected fixture compiles actual parser, unchanged Boolean
provider, actual scene-reader/helper and air-operations TUs. It is prepared to
check true/TRUE/false/FALSE, owning copy after original-byte mutation,
diagnostic clearing and opposing raw text, present false versus absence,
following key retention, wrong type and unchanged raw fallback. It then checks
child capture before parent redeclaration, later-parent and authored overwrite,
and entity fill-missing in the admitted B domain. The installed portion expects
15 library files/22 groups, eight retained false bytes surviving diagnostic
removal, and the unchanged four-slot Source loader result. USN1/JM06 authored
FakeAllocated absence and JM06's 96 authored entities remain separate input
checks. The numerical Source class callback is not an original enum/class
provider; the whole SceneReader/emitter, AI, ABI and game are not exercised.

`local/cc11_scene_typed_bool_compile.cmd` completed with `/std:c++17 /W4 /WX
/fp:strict /MD /O2 /Gy` for all four TUs. `/showIncludes` pins 184 actual compile
inputs (181 project source/header/recipe inputs and three Lua headers), stable
before/after, plus 17 stable installed inputs. Zero compiled support inputs were
used. The prepared recipe and link command are
`local/cc11_scene_typed_bool_recipe.md` and
`local/cc11_scene_typed_bool_link.cmd`; they require root's complete header
consumer rebuild and freshly pinned link inputs. Linked probe/full build/CTest
results remain pending root. Direct-call verification and `git diff --check`
are recorded in `reports/scene_typed_bool_payload_cc11.json`.
