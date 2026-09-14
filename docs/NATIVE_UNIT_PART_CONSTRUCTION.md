# Native unit-part instance construction

Address: `007135C0..00713723`, inclusive; 356 bytes.

`construct_native_unit_part_007135c0` reconstructs the complete caller and its
source exception cleanup. Original ECX is fresh storage; the stack contains
`(unit, selected_part_set)`, `RET8`, and EAX returns the original storage.
The ten checked calls belong to eight functions. This is a new C++ interface
with required complete providers, not a complete collision/render runtime or
a binary replacement. Ghidra was read-only in this worker.

## Actual storage and producers

Every checked producer requests `1AC` bytes through `00BF681B` and only calls
the constructor when that allocation is nonnull. Three sites pass null unit;
the others pass the current canonical unit. No padded production owner is
introduced. `NativeUnitPartConstructionView` borrows cells of that same actual
allocation; each member identity must designate the indicated embedded object.
The passed unit uses the existing canonical `NativeUnitObserverAlias` identity;
it is not a new observer, unit or scene companion.

| Cells | Producer and meaning |
| --- | --- |
| `+00` | `7135E2` writes base `CE89E8`; after `4E6480`, `7135FA` writes derived `CFD7B8` |
| `+4C`, `+164` | unit owner aliases, written by this constructor; final `7136E5..EB` reloads `+164` into `+4C` |
| `+160` | selected-set pointer, transferred into the instance; cleanup `711080` decrements that object's `+4`, dispatches slot zero at zero count, then clears the cell |
| `+168` checked vector | proxy untouched; begin/end/capacity `+16C/+170/+174` zeroed in order; group producer `713380`/`713270` builds `10h` records |
| `+178`, `+188`, `+194` lists | proxy words untouched; `7103A0`, `7103C0`, `7103C0` return circular `30h` sentinels into `+17C/+18C/+198`; each following count is zero |
| `+184` | byte zero after first list completes and before second sentinel allocation |
| `+1A0/+1A4/+1A8` | data/count/capacity, the existing `NativeRenderPointerArrayStorage` header; zeroed in order; `711C60` writes pointers through this exact header |

The existing `collision_shapes.hpp`/`hit_narrowphase.hpp` offset definitions
remain authoritative for base collision fields. Complete base `4E6480` is
required: it initializes bounds, child capacity two and an eight-byte child
allocation, observer-like link cells and owner aliases. Its first allocation
failure occurs before this constructor owns any completed base cleanup state.
The available shape-only projection does not replace the base operation.

`NativeNodeStorage` from `native_node_construction.hpp` is the canonical render
prefix produced by `B6F5A0`, with native `NativeString name_54`. The selected
set's current `+0C` must bind that actual node; no synthetic render root is
created. `B6F960` takes that node and a string header, skips self-header copy,
resizes `name_54`, then reloads source/destination fields and copies the current
destination length when source length is nonzero. No complete implementation
of this exact setter exists locally, so it remains a required whole provider.
The selected-set allocation/virtual `+8` ownership producer remains unbound.

The constructor calls `713380` before obtaining unit slot `+10`. The unit test
uses the original argument, even if a provider rebinds `+164`. After canonical
`41E870` CString construction, it reloads current `+160`, then selected-set
`+0C`, before the name setter. Final owner publication also reloads `+164`.
For a nonnull original unit, `712440` runs, then current count `+190` decides
whether to call `710AD0`. `711C60` runs for both null and nonnull unit.
`710BB0` selection can consume `view.identity` and the same `+16C` vector.

## Native calls and complete required boundaries

| Site | Target | Contract |
| --- | --- | --- |
| `7135E8` | `4E6480` | complete collision-base field/allocation initialization, ECX=model, RET |
| `71362F` | `7103A0` | allocate `30h` sentinel, self-link next/previous; ECX list is unused; RET |
| `71364D`, `713665` | `7103C0` | same native sentinel allocation at a distinct existing entry; RET |
| `713689` | `713380` | whole group construction from selected-set checked records, ECX=model, RET |
| `713699` | unit virtual `+10` | current table entry, ECX original unit, no stack args, C-string EAX; dynamic target unbound |
| `7136A0` | `41E870` | existing canonical CString constructor, ECX eight-byte header, text, RET4 |
| `7136BA` | `B6F960` | whole native node-name setter, ECX current root, header pointer, RET4 |
| `7136D7` | `419CC0` | existing raw singleton getter; **no arguments, bare RET**; three pre-pushed release arguments survive |
| `7136DE` | `BD1510` | existing actual pool return, ECX pool; `(data,length+1,1)`, RET0C |
| `7136F2` | `712440` | complete collision-record decode, shape-list construction/publication and bounds; EAX discarded |
| `713701` | `710AD0` | whole kind-dependent spatial attachment, ECX=model, RET; not just setting byte `+184` |
| `713708` | `711C60` | whole name-filtered tail pointer-array population, ECX=model, RET |

The source composes the existing `construct_native_string_cstring_0041e870`
and `destroy_native_string_header_0041dd20` overloads taking
`NativeStringRawPoolContext`. Those call the actual shared pool getter and
storage operations; no private CRT, pool, STL, Lua, renderer or scene port is
introduced. No placeholder provider implementation is supplied by this packet.

## Unwind contract and listing repairs

Handler `C84B64..C84B6D` loads FuncInfo `DB33F4` and jumps to native FH3.
Its eight-state map begins at `DB3418`; each predecessor is `state-1`.

| State | Thunk, inclusive | Required cleanup |
| --- | --- | --- |
| 0 | `C84B00..C84B07` | `4E6570(model)` resets base vptr and releases current child allocation |
| 1 | `C84B08..C84B15` | `711080(model+160)` releases selected set |
| 2 | `C84B16..C84B23` | `712B40(model+168)` destroys group elements, frees backing, then clears vector words |
| 3 | `C84B24..C84B31` | `710F90(model+178)` jumps to whole list cleanup `710870` |
| 4 | `C84B32..C84B3F` | `710FC0(model+188)` jumps to whole list cleanup `7108E0` |
| 5 | `C84B40..C84B4D` | `710FC0(model+194)` |
| 6 | `C84B4E..C84B5B` | `711000(model+1A0)` resizes to zero then releases current backing |
| 7 | `C84B5C..C84B63` | `41DD20` temporary name destruction |

Reached failure states are -1 before base return; 2 before first sentinel; 3
before second; 4 before third; 6 before group construction and after normal
temporary destruction begins; and 7 during node-name assignment. State 7 is
lowered to 6 **before** normal temporary pool release. Constructor failure
never frees the outer `1AC` allocation: its allocating caller owns that cleanup.

Misleading inherited no-return metadata truncates cleanup pseudocode after
CRT release. Raw evidence shows `710870..7108B7` and `7108E0..710927` walk
**all** list elements, release the sentinel and clear `list+4` before RET;
`712B62` resumes at `ADD ESP,4` then clears all three vector words through
`712B7C`; `711012..711016` restores stack/ESI and returns. Providers must
include these instructions. Supporting ownership receipts identify unclaimed
tails separately; the eight unwind thunks are currently defined, while the
FH3 handler itself remains unclaimed in the captured project.

Source cleanup covers C++ provider failures with returning, nonthrowing cleanup
providers. A failure from the raw-pool getter during normal name destruction
uses state 6. A failure during source unwind terminates through its `noexcept`
cleanup boundary. Native FH3/SEH registrations, native exceptions, Lua longjmp,
hardware faults, asynchronous observation and fault-time state stores are not
equivalence claims. The compiler may hoist local cleanup-state stores between
nonthrowing field writes; receiver writes use volatile accesses in native order.

## Evidence and validation

All ten allocating sites and full register-write listings are retained in
`local/part_callers_context.txt` and the per-function register evidence:
`476C69`, `484825`, `50734D`, `681011`, `6819EA`, `850A62`, `85F227`,
`8830F1`, `87BE58`, `87BE9B`. The null-owner sites are `50734D` (EBX=0 from
`5072DC`), `681011` and `6819EA` (literal zero). The caller's selector/detail
contract uses the corrected `NATIVE_UNIT_HEALTH_PARTS.md` reading.

MSVC Win32 Release build and both existing CTests pass. Three controlled
original-byte/source comparisons execute all 356 caller bytes with explicit
provider rebindings: null owner, empty name/no attach, and rebound selected set
and owner/nonempty name/attach. They compare all 428 allocation bytes, 144
sentinel bytes, pre-call receiver hashes, provider order, return identity,
pooled-string effects and zero ESP delta. The fixture uses canonical
`NativeUnitObserverPrefixStorage`, `NativeNodeStorage`, pointer-array storage
and a real constructed `NativeStringPoolStorage`; only the unresolved provider
operations are controlled boundaries. Six focused source injections check
every reached distinct cleanup state, including real pooled-name cleanup.
Native provider bodies and native FH3 are not executed by this caller probe.

The retained final `/O2 /fp:strict` object confirms receiver stores: source
`+41` base vptr, `+6D` owner alias, `+72` derived vptr, `+7B` selected set,
`+82` owner, `+87/+90/+99` group words; list and tail header stores follow in
order. `+193..+19B` reloads owner before final alias publication. Full object,
relocations, linked archive dependencies, discovered compiler headers, SDK
libraries, build commands, native bytes, probe inputs/outputs and tools are
frozen in the self-contained `local/part_construction_retained` tree with a
SHA-256 manifest. This is source/fixture evidence, not game validation.


## Integration review, batch AA

Correction from `docs/ORCH6_RECONSTRUCTION_AA.md`: The integrator defined C84B64..C84B6D and saved the FH3 selector name. Internal post-free listing gaps were repaired and exports refreshed. Explicit post-free tails were decoded, but Ghidra stored ownership remains truncated for710870,7108E0 and711000; these providers remain required whole bodies. The712B40 internal gap is repaired. All30 direct/tail/caller rows pass. Native unwind remains untested.
