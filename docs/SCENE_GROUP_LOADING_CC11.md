# Scene library group capture and redeclaration

Packet `cc11_scene_group_loading`; source baseline `52bc66be1`.
Owned addresses: `008F67B0`, `008F5A00`, `008F54F0`.
Ghidra project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified before live read-only batches.

The library binds parents when a group declaration is loaded. Later parents
overwrite earlier parents, then authored values overwrite that result. A
redeclared group reopens its existing bag and retains fields omitted by the
new declaration. Previously captured child bags do not follow later parent
changes. The former source implementation stored every declaration separately
and resolved parents lazily, keeping the first parent on conflicts and ignoring
later declarations of an existing group.

The bounded source binding now stores an owning captured bag in the first
case-insensitive group row. Entity application remains a separate fill-missing
merge, followed by authored overwrite. No installed group or JM06 submarine
projected value changed in the focused comparison. This does not close the
whole loader, parser, VFS, binary ABI or original-game behavior.

| Routine | Coverage and evidence |
| --- | --- |
| `008F67B0` | Complete normal body previously audited, `008F67B0..008F6B97`; this packet binds only supported group reuse/capture/publication semantics. File/VFS/error policies remain partial. |
| `008F5A00` | Parent loop `008F5A47..008F5ACD` and child branch `008F66B9..008F6747` checked; existing whole typed-parser reconstruction remains partial. |
| `008F54F0` | Normal merge body checked, including raw returning tail through `008F566E`; stored Ghidra body ends at `008F5658`. Layout/allocation/alias/failure contracts are not ported. |
| `008F4F60` | Supporting type-6 clone producer `008F50ED..008F5133` checked; all twelve arms have prior merge-packet evidence. No new clone reconstruction ownership. |
| `008F41F0` | Complete supporting normal bag clone, `008F41F0..008F4289`; recursively clones each property record. |
| `008EF780` | Complete supporting type-6 constructor body read: type at `+4h`, owned child at `+0Ch`, child's owner record at `+110h`. |
| `00469B60` / `00468ED0` | Complete supporting lookup bodies read; case-insensitive name comparison, unguarded missing-result dereference. |
| `008F0700` | Complete supporting destination-type value assignment switch read; metadata is not a general record replacement. |
| `0046CF40` | Supporting ordinary entity calls only, `0046D2F4..0046D34E`; no new whole-reader coverage claim. |

## Exact native order and copy depth

`008F6A69 -> 008F2DB0` tests the global library's group name.
`008F6A7F -> 00469B60` returns the existing bag; otherwise a new 114h bag is
allocated and initialized. `008F6AD1..008F6AD7` passes ECX=bag and stack
arguments tokenizer, flag 0 and saved library receiver to `008F5A00`.
The existing bag is mutated directly. `008F6ADC..008F6ADE` skips publication
when the name already existed; only a new bag is inserted by
`008F6B42 -> 008F29A0`, after parsing. The ordinary singleton caller publishes
that library as `00E18678`; the host binds a single consistent registry, not
arbitrary mismatched global/receiver registries.

Before consuming the authored opening brace, `008F5AA9 -> 00469B60` resolves
each parent and `008F5AB3 -> 008F54F0` merges it with flag 0, in list order.
There is no retained parent-name reference in this native bag path. Present
scalar keys are assigned at `008F560D -> 008F0700`; destination type selects
the storage operation. Present type-6 children recurse at `008F55F6` with
the same merge flag. Missing records are cloned at `008F561D -> 008F4F60`,
their `+34h` ordinal is copied at `008F562E`, then inserted at `008F563B`.

For type 6, `008F510D` loads the source child pointer at `+0Ch` into ECX,
`008F5110 -> 008F41F0` creates a new bag, and `008F5118 -> 008EF780` wraps
it in a new type-6 record. The bag clone allocates a fresh 114h bag, iterates
every source node, calls the record clone at `008F4263`, and inserts each new
record at `008F426C`. Nested type-6 records repeat this process; no fixed
eight-level cutoff or shared child bag is introduced. Structural cycles or
aliasing are outside the admitted input domain, not evidence of a safe clone.

Authored scalar setters run after the parent loop. The child branch reuses
an existing child at `008F66D6`, or attaches a missing child at `008F672E`,
then parses into it at `008F6740`. An empty authored child does not clear
inherited records. Source arrays/strings/children are owning value containers;
native reference-kind, enum-declaration, array representation and ordinals
remain separate incomplete storage contracts.

## Source admission and failure boundary

`PropertyLibrary::add_group` captures already-present parents in declaration
order, then applies the authored block. Reopening a row starts from its stored
bag; omitted fields and nested defaults survive. `merge_group_into` applies
that captured bag with fill-missing semantics and performs no later parent
traversal. The former arbitrary depth-8 base traversal is removed.

The source block parser receives only a `SceneLexer`, error vector and local
`Cursor`; it cannot mutate or call back into the group registry. Therefore
parsing the source body before capture is equivalent on admitted well-formed
declarations. This is not an ordering claim for parser callbacks or malformed
inputs. Parent lists remain in authored order, scalar type letters and tokens
are retained, and empty child records remain present. Host vector order is not
a claim about native hash iteration or `+34h` ordinals.

Source admission rejects unnamed groups, missing/forward parents, self-parent
and cyclic name dependencies, reported group syntax errors, scalar/child
collisions and incompatible scalar type letters. These throw an explicit
`unsupported library ...` error. The owning source tree cannot represent
native structural aliases. Validation uses a copied candidate before publishing
the source result; this is a host admission boundary, not recovered native
transactional failure or rollback. Unreported malformed tokenizer/value inputs,
native allocation failure and exceptional unwind remain unsupported.

The native loader does not reset the library, test a parser success return,
or roll back prior mutations. Missing-parent lookup dereferences the returned
node without a null check. Existing bags can be partially mutated before a
failure; new bags are published after the parser returns without success
gating. The loader's unknown-token stall, VFS provider mode 32h, open/size/read
ownership and host mode-2 stand-in remain the prior loader audit's limits.

Ordinary entity inheritance is independent: `0046D2F4` pushes flag 1,
`0046D304` resolves the group, `0046D30C` merges it, and `0046D34E` parses
the authored body. First listed groups fill missing values; authored writes
win. Unknown entity group names retain the existing unbound host fallback;
this packet's explicit rejection applies to library parents.

## ABI and stored-body qualification

`008F67B0` uses ECX=library and one pathname stack argument; its terminal
`008F6B95 RET4` and caller `008F7092` establish the calling shape. The caller
overwrites EAX at `008F7097`. `008F5A00` uses ECX=bag and three stack arguments,
returning at `008F6784 RET0Ch`; caller and recursive sites ignore EAX.
Formal semantic types and a meaningful return value are not established.

`008F54F0` uses ECX=destination and stack source/keep-existing flag. Ghidra's
stored body stops after the returning-free call `008F5654 -> 00BF65AC`.
Disk bytes `008F5659..008F566E` restore the exception chain and return at
`008F566C RET8`. Group/entity/recursive callers continue without consuming a
semantic result. No annotation, function definition, flow repair or other
Ghidra mutation was performed. The source helpers expose new C++ interfaces
and are not drop-in native functions.

## Installed inputs and verification

The installed `universe/library` contains fifteen files: one `.enums`, fourteen
`.props`, and 22 unique group declarations. There are no group redeclarations
in these inputs. `global.enums` supplies Common at 1753, MultiEntity at 1772,
GameUnit(MultiEntity) at 1789 and PowerupHandler at 1795 before the `.props`.
The multiple-parent declarations are CommandBuilding(Common,PowerupHandler),
LandFort(Common,MultiEntity) and Landscape(Common,MultiEntity); their parent
fields are disjoint. Ship(Common) at `ship.props:2` supplies
`TorpedoDirector = B false`, and Sub(Ship) at 199 captures it. PathPointCamera
captures PathPoint earlier in the same file. The report pins all file hashes,
declaration names, lines and parent lists.

One ignored fixture includes the actual production translation unit and links
the existing primary game objects/core libraries, without fabricated stubs.
MSVC Win32 `/W4 /WX /fp:strict` compile/link and manifested execution pass.
The scenario demonstrates conflicting ordered parents, authored overwrite,
reopened-row retention, independent nested capture through ten child levels,
later-parent changes visible only to later declarations, and entity flag-1
retention followed by authored overwrite. Its installed comparison checks all
22 group projected bags and all 14 JM06 submarine bags against the baseline
source traversal; type letters, values and nested fields match. This comparison
does not assert native order/layout or the whole VFS loader.

JM06 retains six Hidden TypeB stock records, three PlayerSubs with movie=false,
and tutorial TypeB 01..03 with game=false/movie=true. Each of the fourteen
merged submarine bags retains eight MultiType entries and the library's false
torpedo-director value before the separately bound class-8 exception. No class-8,
Hidden, effective-mode or generation-policy flag changed. The existing installed
scene probe also passes: 96 entities, 11 classes, 14 submarine generators,
46 registration-pass records, zero unregistered classes or recovered errors.

Direct call rows are verified separately from byte-tail evidence.
`git diff --check` passes. No new tracked test, full build or game run was made;
the primary owns integrated Win32/CTest verification. This packet establishes
bounded source/fixture behavior supported by original assembly, not native ABI
compatibility, original-game execution or new gameplay validation.

## Primary integration receipt

Primary merged the packet and passed the complete MSVC Win32 build and all
three existing CTests. The actual production-TU snapshot/installed-input
fixture was independently rerun, and the rebuilt mission scene probe passes.
See the report for the source revision, executable hash and build log.

After verifying BSP project/program and matching disk/live bytes, primary
cleared only the CALL_RETURN override at `008F5654` and decoded the 22-byte
returning tail. `008F566F` is its exclusive end; RET8 occupies `008F566C..008F566E`.
The saved flow report records that stored Ghidra function-body metadata still
ends at `008F5658`. This is partial tail decoding, not a full body repair or
a new native return-type/ABI claim. Whole loader/parser/VFS/storage/failure
and original-game boundaries remain open.
