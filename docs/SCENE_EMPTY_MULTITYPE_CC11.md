# Present empty MultiType bags in JM06

Packet `cc11_scene_empty_bag`, 2026-10-06. Source baseline:
`696e72ee3c28a0f8bffc08ff35df9251dfd409ba`. Read-only Ghidra target:
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

**Verdict:** no parser or generation-gate behavior change is warranted. A present
empty authored `MultiType` child remains present. When group defaults already
supply that child, the native parser reuses it rather than clearing it. JM06's
tutorial submarines author `GenerateInGame = B false` but inherit
`GenerateInEngineMovie = B true` from `MultiEntity`. The ordinary mode-9 fallback
therefore explains their admission. Empty authored syntax does not remove the
eight inherited multiplayer entries or the movie-generation default.

## Native producer, presence and order

| Routine | Coverage in this packet | Receipts |
| --- | --- | --- |
| `0046CF40` | partial: bag creation/group merges `0046D2C3..0046D34E`, gate call `0046D426`; other entity/registration arms not closed here | Construct bag, resolve each listed group, merge before parsing the authored body |
| `008F5A00` | partial: entry/argument setup, child branch `008F66B9..008F6747`, empty-body exit `008F6760..008F6784`; scalar/enum/error arms remain outside this closure | Reuse existing child or allocate/attach a missing child before recursive parsing |
| `0046C550` | partial: presence branch `0046C58C..0046C6AB`, mode 9 `0046CB43..0046CC58`; other mode/geometry/class arms are existing work | Missing record selects deferred path; present record uses child pointer and mode gating |

The entity producer constructs its bag at `0046D2C3 -> 008F41A0).
It resolves a group at `0046D304 -> 00469B60` and merges it at
`0046D30C -> 008F54F0`. At `0046D347..0046D34E`, it pushes nullable group
resolver 0, allow-untyped flag 1 and tokenizer EBP, sets ECX to that bag, and calls
`008F5A00`. This is an authored write into the already populated bag, not a
fresh replacement. `0046D426` later calls `0046C550`; `0046D42B` tests its AL.

The parser looks up the key at `008F5B1B -> 008F2260`. On a child body,
`008F66D2..008F66D9` takes an existing record's `+0Ch` child. Only a missing
record allocates the 114h-byte child. `008F672E -> 008F3B60` attaches it before
`008F6740` recursively parses the child. Supporting read of `008F3B60` shows
a property record with type tag 6 and child pointer at `+0Ch`; it calls the
existing insert routine `008F33F0`. This dependency is not newly named or
reconstructed. Empty input reaches the closing-brace exit at
`008F5AE7 -> 008F6760`: an attached record stays present, and an existing child
has not been cleared. Allocation failure and malformed/type-conflicting bags
are not established by this valid installed-input audit.

At `0046C59D`, the gate looks up `MultiType`. `0046C5A6` branches on record
presence, not child contents. Absence takes the deferred path and returns AL=1
at `0046C684`; presence reads the child at `0046C6AB`. In mode 9,
`0046CB4A` looks up `GenerateInGame`; a false byte branches at `0046CB53`
to `0046CC37`. `0046CC3E` looks up `GenerateInEngineMovie` in the parent
bag and `0046CC43` returns that byte. A true game flag instead attempts deferred
creation/stock registration and returns true when the record exists; otherwise
it reaches the same movie fallback. Existing registration-pass behavior is
unchanged.

## Installed inputs and merged bags

Root: `I:/SteamLibrary/steamapps/common/Battlestations Pacific`.

| Input | SHA-256 | Evidence |
| --- | --- | --- |
| `universe/library/global.enums` | `A9F99C0E8FC650E2EF86E404990D56C7F2E4B518FF65BE685ACC3C6999ACE027` | Common lines 1753..1762; MultiEntity 1772..1787; ShipClasses Gato=30, Narwhal=31, TypeB_Jake=93 at 109, 110, 158 |
| `universe/scenes/missions/COTP-IJN/PRCPIJN/ijn_06_prelude_to_midway.scn` | `14AE7C12AC9AD51DCA08F5B9FAC7B00ACE3739CCE5C4F84C4F7FB4AF479EEF0C` | 173026 bytes; fourteen SubmarineGen records |
| Same stem `.ema` | `23C77AD2BED6F2D91A0BD81CF9B00F752D8B60AE3A59A972AFE65D1129BFC8CA` | Existing entity-map input; loader's native `004D52F4..004D5493` policy selects mode 9 under its single-player/unregistered-key conditions |

Every listed submarine has groups `(Common, Sub, Command, MultiEntity)` and an
authored empty `MultiType` child. `global.enums` is also a property-group
producer: `MultiEntity` defines both generation flags as typed B true and
eight typed B true entries within `MultiType`. The derived group's own fields
precede base fields; listed groups fill missing values, then authored fields
overwrite scalar values. Recursive child merging retains existing defaults.

| Entities / scene start lines | Type enum | Authored flags | Merged game / movie |
| --- | --- | --- | --- |
| Narwhal / 5143; Gato / 5163 | 31; 30 | Both absent | true / true |
| TypeB Jake 04..09 / 5615, 5638, 5661, 5684, 5707, 5730 | 93 | Both absent; Hidden=true | true / true |
| PlayerSub 01,03,02 / 5925, 5948, 5971 | 93 | Movie=false; game absent | true / false |
| Tutorial TypeB Jake 01..03 / 5994, 6018, 6042 | 93 | Game=false; movie absent | false / true |

The tutorial flags are at 6006, 6030 and 6054; the adjacent empty children are
at 6008..6009, 6032..6033 and 6056..6057. Common's typed Hidden=false default
and the six authored Hidden=true overrides remain intact. Template strings
do not supply the generation explanation in this producer body.

The unchanged source stores empty children in `parse_property_body`, and
`find_scene_property_block` returns their address even when empty. The ordinary
host merge in `src/game_hosts_scene_contents.cpp` recursively merges children
without clearing them, applies listed groups before authored values, and invokes
the gate on the merged bag. This is distinct from shipyard's sparse launched-unit
bag. `SceneGameMode::EngineMovie` is 9; unrestricted is 10. The existing
entity-map mode policy, class8 director binding, Hidden handling and other
switches are unchanged.

## Parser ABI correction and qualification

The current saved description of `008F5A00` as
`__fastcall(Tokenizer* ECX, char, int)` is contradicted by entry and callers.
`008F5A25` saves ECX as the bag; `008F5A1E` loads the entry-stack +4 tokenizer.
Entry-stack +8 is the allow-untyped flag and +0Ch is the nullable group resolver
used by the optional group-list branch. The recursive setup
`008F6733..008F6740` likewise passes tokenizer/flag/0 with ECX=child.
The terminal `008F6784 RET 0Ch` matches three stack slots. The established
calling convention is `__thiscall`; a semantic return value is not established.
The terminal tokenizer call at `008F6767` is followed only by unwinding, and both
`0046D353` and `008F6745` ignore the parser's EAX. The C++ function's returned
tree is a host interface, not this native ABI. Primary owns the metadata
correction; this packet made no Ghidra/ledger changes.

The existing installed-scene probe exited 0: 96 entities, 11 classes, fourteen
SubmarineGen/MultiType records, 46 registration-pass entries kept, zero
unregistered classes and zero recovered errors. Log:
`local/cc11_scene_empty_probe.log`, SHA-256
`30D01823EA516687D9B957F67137B935C94D6F88D5AB5EE6FEA4B2D998354EDE`.
An ignored diagnostic also confirmed fourteen raw empty children; no redundant
new tracked fixture was retained. The earlier completed
[JM06 pair](TORPEDO_DIRECTOR_JM06_PAIR_CC11.md) already established mode-9 host
execution with eight actual units/seven active and six Hidden units held;
it is source-host runtime evidence, not an original-game admission run.

Changes are comments and evidence only. Direct native call receipts and
`git diff --check` pass. No new game run or full build was performed; primary
owns serialized integration verification. Named but incomplete dependencies
include property-library loader `008F67B0`, existing merge `008F54F0`, child
attachment `008F3B60` and deferred/stock helpers `004693C0`, `0046C450`,
`0046BF20`. This closes valid-input presence/default/order evidence only,
not the whole parser, registration system, native binary ABI or original game.
