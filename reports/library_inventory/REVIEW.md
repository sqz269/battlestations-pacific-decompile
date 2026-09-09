# Inventory import review

The discovery import is consistent with the post-import local export: all 8,780
rename tags match the exported names, no duplicate addresses occur in the
10,334-entry tag ledger, and no tag overlaps the reconstruction or descriptive
name ledgers. The important correction is interpretation: names and confidence
labels do not establish that the corresponding behavior can be skipped.

## Current evidence

The post-export snapshot is dated 2026-09-09T19:27:40Z and contains 62,283
internal functions, with 62,725 total reported by Ghidra. The older SUMMARY
uses 62,191 internal functions. This review checks local exports and logs;
it does not claim a fresh live Ghidra verification or independent source-level
validation of every library match.

The first import JSON records 377 events: 288 renames and 89 creations. The
bulk import records 9,958 events: 8,404 renames and 1,554 bookmarks. Address
`00a72650` appears twice across those logs; therefore event totals must not
be used as unique-address totals. Both run logs report exit 0. The post-run
log reports re-export exit 0, 587 complete local function exports, and final
exit 0. No log status indicates a failed operation. Final-name comparison
supports the imported rename outcome; bookmark persistence and byte/source
equivalence are not independently verified by that comparison.

At the import-review baseline there were 170 reconstructed routine entries and 60 fragment entries,
with zero tag overlaps. The descriptive name ledger also has zero overlap.
This protects previously named work from this import, but does not turn
other tagged addresses into non-targets.

## Confidence categories are different kinds of evidence

- Lua: 324 tags, of which 282 high, 40 medium, and 2 low confidence. Its
  detailed inventory contains version/source/table evidence and explicitly
  identified native modifications. Even a high-confidence source match does
  not mean an unmodified library build has equivalent allocation or exposed
  Lua-library behavior.
- zlib: 52 tags, 46 high and 6 medium. The source attribution evidence is
  stronger than a generic instruction-shape heuristic; linked wrappers and
  caller contracts still need integration evidence.
- CRT unmatched: 322 high-confidence tags are principally block attribution
  and samples/FID context, not 322 individually established exact symbols.
- STL instantiations (4,007), probable STL (1,554 bookmarks), and throw sites
  (564) are mechanical classifications. Calling `_invalid_parameter_noinfo`
  or containing an STL throw string does not prove the whole routine is stock
  library code. Game-specific comparator, ordering, ownership, and mutation
  rules may live in exactly these wrappers.
- Compiler-shaped destructors, thunks, static initialization, and trivial
  bodies identify implementation shape. Their associated object lifetime,
  pointer adjustment, global startup effects, and field meanings still matter.
- RTTI slots (289), Dyn block (213), metrics wrappers (121), telemetry and
  pipe blocks are not synonymous with importable third-party source. In-house
  Dyn and engine wrappers remain reconstruction dependencies. Stub proposals
  in SUMMARY are hypotheses requiring caller/behavior review, not authorization
  to delete or no-op the code.

The current VFS work demonstrates why this distinction matters: its ordered
tree/list helpers determine priority and fallback order. Nearby tagged
`00bdb4f0` is classified as STL from a <=96-byte checked-iterator shape;
`00bdece0`, `00bded80`, and `00bdfd80` are classified from throw strings.
These tags must not short-circuit contract recovery when those addresses are
reached. Already audited `00bddc80`, `00bddaa0`, `00bdc680`, `00be1330`,
and `00be1740` are not overwritten by the tag ledger.

## Status correction

The prior status script subtracted all non-FUN names and called the remainder
reconstruction candidates. That included heuristic names and in-house RTTI
names, making rename activity appear to reduce implementation scope. It also
called FUN names untagged despite 1,554 provisional bookmark-only entries.

`tools/status.py` now reports names and tag categories separately, retains raw
reconstruction coverage, and explicitly avoids deriving a reduced denominator
from naming. It was run successfully against the current local snapshot.
No new tests were added. The independent review initially left the user-modified
`reports/current_status.txt` untouched; the integration follow-up below refreshes it.

The old SUMMARY's candidate estimates and calendar completion projections
remain historical inventory estimates. They are not current coverage metrics,
verified remaining workload, or forecasts supported by completed gameplay
subsystems. The inventory review itself changed neither tags nor names nor
Ghidra state. Machine-readable counts and findings are in `review.json`.

## Integration follow-up

The resumed mounted-stream work verified the live `bsp` project and target
program, recovered the missing 96-byte provider callback at `00bda690` after a
disk-byte comparison, and applied five descriptive annotations with previous
comments preserved. The project was saved and affected exports refreshed.
The snapshot at 2026-09-09T20:11:25Z contains 62,284 internal functions and
62,726 total; all 8,780 inventory rename tags still match, with no overlap
with the descriptive naming ledger.

`reports/current_status.txt` now uses that snapshot and the corrected status
tool: 170 reconstructed entries and 63 fragment entries. The three new
fragments cover mounted read-only open, single-alias substitution and FileStore
population. Stock zlib compilation is not counted as reconstructed routines.
The roadmap now records stock-source reuse and the remaining game-specific
contracts. See `docs/MOUNTED_RESOURCE_STREAMS.md` for the passing cache-backed
font probe and its limits; neither this follow-up nor name comparison establishes
all bookmark persistence, exact library-source equivalence or gameplay parity.
