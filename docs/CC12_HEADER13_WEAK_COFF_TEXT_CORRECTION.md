# HEADER13 weak COFF schema correction, r03 TEXT

The selected r02 generator emits `weak_tag_index` from the actual COFF auxiliary
TagIndex, but its retained-body enumerator reads `weak_target_index`. The latter
field does not exist. The Root static stage stopped at recipe line 706 with
`KeyError: 'weak_target_index'` after the fresh h13p2 compile/link.

The r03 TEXT candidate corrects that one consumer key. It also makes the
mechanically required active-family changes to fresh `Main/local/h13p3`, including
a versioned `input_contract.r02.py.txt` with only its EXPECTED_FAMILY changed.
Source credit remains zero. Root has not approved or executed this candidate.
The failed h13p2 family was not modified or replayed.

## Exact change

The local family is
`J:/PROG/battlestations-pacific-decompile-cc12_header13_weak_coff_TEXT_correction/local/h13weakfix`.
The selected r02 generator SHA256 is
`c30a8e242ec0432bacbf38d76199dfffc244220970620cb81b4b1b5a1b7dbbc4`.
The frozen actual h13p2 recipe is byte-identical to that selected r02.

| Artifact | Bounded change |
| --- | --- |
| `header13_final_generator.r03.py.txt` | Line 16 FAMILY and line 28 IDENTITY.family: h13p2 to h13p3; line 706: `target['weak_target_index']` to `target['weak_tag_index']` |
| `input_contract.r02.py.txt` | Only line 9 EXPECTED_FAMILY: h13p2 to h13p3 |
| `Root_generation_selection.proposed.r03.json` | Active identity family, new generator pin, two appended reviewed TEXT pins, and the active recipe/input-contract materialization origins |

The recipe changes by three bytes in length; the family path substitutions have
identical lengths. Full reverse byte comparison reproduces r02 exactly.
Normalizing the three permitted constants in the complete r03 AST reproduces
the complete old AST. Of 70 top-level function/class definitions, only
`enumerate_retained_tu_bodies` changes, at the single subscript key. Every other
function/class, receipt guard, optimization check, launch function, relocation
rule, CFG check, provider rule, strict trailing-data check, and compile/link
argument remains unchanged. Every input-contract function is also unchanged.

The proposed selection remains `accepted:false`. It retains all 68 original
reviewed-artifact rows and appends the two new TEXT artifacts, giving 70. Exactly
76 of 78 materialization rows remain unchanged. The two changed rows are
`recipe.py` and `input_contract.py`. All old Q15 and D39 pin tables and Source10
rows remain byte-identical in the recipe. The active reader set uses the fourteen
unchanged Q entries plus the one family-only input-contract variant; the old
Q15 input bytes and old input-contract materialization under
`selected_reader_TEXT/input_contract.r01.py.txt` remain preserved as historical
reviewed inputs. No other active Q module contains an h13p2 path.

The two remaining h13p2 strings in the proposal are historical origin pins for
`Root_selection_generation_context.json`. Root explicitly instructed retaining
those immutable origins. They are not an active family identity or permission
to reuse old objects/processes. The selected context remains SHA256
`ec1db0a4fc1bbdd07f2db4d27fd20fbfdb713383424690f222ff54f8789919be`.
The recipe, corrected reader, and proposed selection agree exactly on h13p3.

## Actual raw COFF evidence

A new worker-owned stdlib parser read only the frozen actual leaf/canonical/probe
objects, map, and linked PE. It did not import or execute any selected parser,
recipe, reader, or helper. It bounds the ordinary 18-byte COFF symbol slots,
auxiliary records, string table, sections and relocations; requires primary-symbol
TagIndex targets; detects weak cycles; preserves modes 1 and 3 as distinct; and
requires a defined terminal fallback.

| Actual object | Weak records | Modes | Retained weak map aliases |
| --- | ---: | --- | ---: |
| leaf.obj | 0 | none | 0 |
| canonical.obj | 7 | seven mode 1 NOLIBRARY | 2 |
| probe.obj | 0 | none | 0 |

The two mapped weak records are:

| Weak alias | Raw slot to TagIndex | Defined fallback | Same linked VA | Full extent | Relocations |
| --- | --- | --- | --- | ---: | ---: |
| `??_Eexception@std@@UAEPAXI@Z` | 293 to 292 | `??_Gexception@std@@UAEPAXI@Z` | 4B0010E0 | 45 bytes | 3 |
| `??_Ebad_alloc@std@@UAEPAXI@Z` | 299 to 298 | `??_Gbad_alloc@std@@UAEPAXI@Z` | 4B0010B0 | 45 bytes | 3 |

All seven raw weak chains terminate at defined symbols. The two retained chains
have the same map address as their defined fallbacks and the same complete linked
extent. Independent relocation reconstruction verifies all six operands and all
remaining bytes of those full 45-byte bodies. The five unmapped weak records
are retained in evidence without inventing linked addresses or bodies.

These actual objects contain no mode 3 record. Mode 3 ALIAS support and the full
mode 1/3 chain requirements are preserved by unchanged parser/gate text; this
packet does not claim an actual mode 3 runtime test. The selected raw parser uses
`TagIndex` in its chain evidence, while the recipe's `coff()` wrapper exposes that
same auxiliary DWORD as `weak_tag_index`. The corrected enumerator now consumes
the field the wrapper actually produces. No default value, shortened chain,
policy fallback, or weaker map/extent check was introduced.

## Preservation and qualification

The worker froze and verified exactly 29 individually authorized files, including
18 named files from h13p2, the selected r02 candidate/proposal, five selected Q
origins, and four integrated context documents/reports. Original and frozen size
and SHA256 pins are unchanged before and after. This is a bounded input-subset
claim; the worker did not inventory or claim exact membership of the growing
Root family. `probe_helper_formats.py` is absent from the actual selected imports;
Root confirmed it is not a dependency.

The family includes raw weak evidence, complete raw/AST diffs, proposal checks,
before/after pins, all worker utilities/logs/stops, frozen inputs, metadata, and
commit records. `seal.json` lists every other family file. Its final exact count
and SHA256 are delivered after the two-file commit so commit evidence is sealed
without a circular hash.

No further concrete schema mismatch was found in this bounded review. This is
not a fresh static-gate pass: no selected module was imported/executed, and no
compiler, linker, Native/Ghidra query, provider, target, or old runtime stage ran.
The historical Root text receipt was frozen only as evidence; no new Root receipt
was authored or accepted. No Main Source, ledger, or Root file changed. Root owns
complete independent TEXT review, new h13p3 materialization, fresh compilation and
linking, full helper/map/CFG qualification, new receipts, and any runtime checks.
