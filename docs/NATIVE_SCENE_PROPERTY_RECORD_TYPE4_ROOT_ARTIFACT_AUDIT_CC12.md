# Type4 Root artifact independent audit (CC12)

The independent audit passes for the sealed Type4 two-entry storage record. This
peer report remains **Source 0**. It supplies evidence for the primary integrator's
separate admission decision; it does not mutate the historical Root summary or
claim game, class, World, exception, allocator-boundary, or broader-domain proof.

## Sealed input and independent method

Root's final notice binds `local/t4p3` to exactly **457 files** (456 artifacts plus
the seal). `seal.json` is 98,544 bytes, SHA-256
`bd2cefbe278f318b0b6dbe948a021bd7018093bab3153f35358bd00f2e9c3602`.
The 2,427-byte historical Root summary remains Source 0, SHA-256
`6aa7543fb10bba32a198ca2e84947d66e899dcb64e237f433d45bfaa491f5813`.
No actual Root artifact was opened before that final notice.

Eight worker-owned raw readers were prepared from previously sealed references,
then materialized and executed once against the final artifacts. No Root accepted
reader or helper was imported or executed. The worker did not compile, launch the
target, run tests, query/mutate Ghidra, query live provider identity, or change
Source. PE, COFF, MAP, Gate32v2, full CFGs, relocations, captures, storage bytes,
frozen provider exports, and saved build records were independently read.

## Actual closure and control flow

The three fresh object files yield **31 logical retained symbols, 29 physical TU
bodies and 18 complete external helpers: 47 whole gated spans**. The 315 COFF
sections retain declared uninitialized sizes without fabricating raw zero bytes.
There are 219 logical relocation checks over 213 distinct physical operands, with
six alias rechecks. All seven weak symbols use SEARCH_NOLIBRARY (mode 1); their
complete primary-index fallback chains are cycle-free. Two mapped E/G pairs cover
the same complete exception/bad_alloc bodies and relocations; five unused entries
are discarded. SEARCH_ALIAS (mode 3) is supported by the parser but is not observed
in these objects and is not claimed as exercised.

The Source leaf at `0x43001000` equals all 56 Original bytes from `0x008EF230`:
16 instructions, zero calls, zero relocations, SHA-256
`b6b40a5cbb0a523d345de24f97ac879a4e5a56c8d9a1f121f636fd33ce4dfd98`.
The frozen game PE, saved Native before/after byte files and both saved Ghidra byte
outputs agree. The raw caller, both ordinary wrappers and actual 29-byte Original
EH handler were fully reviewed. The retained ordinary Original entry is
`0x43002C00`: its reachable RET at `0x43002C3F` leaves five unreachable trailing
INT3 bytes. **All 69 bytes remain inside the full gate**. The handler includes its
two leading NOPs. The worker's explicit CFG suffix rule proves reachability and
does not trim these bytes or turn unreachable cold coverage into execution.

The whole linked Main (`0x43002080`, 2,934 bytes, 791 instructions) was manually
read, including failure paths. Its 85 actual calls include 41 calls dominating the
success record. The worker independently derived 47 conditional failure edges and
21 selected phase-dominance pairs. Exactly two nonrepeatable raw target sites
dominate success: Source first, then the unchanged 56-byte Original RX allocation.
CFG register/import tracking handles joins, volatile and partial-register kills;
independent formal tracking proves the target is the entry-stack argument in each
indirect caller. Full Main dataflow supplies its concrete values. No ordinary
wrapper is called dynamically by Main.

The order is pre-code/provider checks, Original RX preparation, two canonical
56-byte root allocations, Source entry, Original entry, post-code/RX checks,
saving both captures and both live storages, two root frees, RX release, provider
bookends, final code check, then success output. Numeric loop bounds establish
three provider iterations and two allocations/frees. The stack probe is 43 bytes
with five separate alignment INT3; the cookie check is 14 bytes with two separate
alignment INT3. The cold delete chain is 16 -> 5 -> free thunk 6 bytes. The named
GS-failure destination remains unexpanded and unadmitted.

## Captured domain and provider precision

Both 124-byte captures decode independently (92-byte register record at the
established layout; post-guard offset 108), as do both direct 56-byte root buffers.
Source enters with DF=1 and opaque value `0x7FC35A91`; Original enters with DF=0
and `0x80000000`. Distinct identity DWORDs are `0xE79B42D1` and `0x3AC5682F`.
Both stack arguments are read before PUSHFD can overwrite their dead slots.
RET 8, full EAX=root, ECX=0, EDX=value, ES, nonvolatile registers, DF, stack guards,
all Capture guards, 41 written root bytes and 15 untouched bytes agree. XOR's
defined flag mask is `0x8C5`, expected `0x44`; AF is not asserted. The caller clears
DF after capturing flags and before CRT reentry. There is no floating arithmetic
interpretation of either value and no allocator interior guard or postfree read.

The malloc/free/_callnewh provider decode uses only the frozen 1,114,240-byte UCRT
DLL, SHA-256 `60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`.
Original-generation FileID and NT-path metadata are bound to the packed Gate and
recorded process; no copy identity or fresh live identity is substituted. The
32-byte prefixes require zero, zero and one HIGHLOW relocation respectively.

Complete `verify_module` (972 bytes, 303 instructions) and `export_prefix_equal`
(620 bytes, 192 instructions) were manually reviewed. Before either entry, the
probe checks MEM_IMAGE/AllocationBase, mapped-path versus held-handle NT path,
FileID/size, full mapped-file SHA, live PE tuple, GetProcAddress/raw export RVA and
the relocation-adjusted live prefix. After both entries, root frees and RX release,
it repeats the IAT target, held-handle FileID/size, full mapped-file SHA and live
prefix checks. It **does not repeat** MEM_IMAGE, mapped NT path or GetProcAddress
afterward. The serialized normalized prefix is the expected relocated raw prefix
which the gated probe compares against live memory, not a separate live dump.

## Provenance, preservation and limits

The saved three compile commands and sole link command bind the actual objects:
MSVC Win32 `/MD /O2 /W4 /WX /fp:strict /EHsc /Gy /GL-`, fixed base `0x43000000`,
`/OPT:REF /OPT:NOICF`, explicit ordinary-Original retention and embedded asInvoker
manifest. Sixty-three frozen inputs, 184 unique consumed headers and seven consumed
libraries match before/after; no BSP archive is linked. Raw PE resource parsing
confirms the embedded manifest and disabled dynamic base. One recorded process
exits zero with one Source and one Original entry. These are audited saved records;
the worker did not replay them.

The frozen Main build at `7b244d3b8bc3204b1e37759a90842c28a0b78b63` binds 4,037
Source inputs before/after and records passing reconstructed_math,
native_math_differential and tool_tests. Its mutable Main originals are historical
context only. Final preservation verifies **24,643 immutable pins and 31 exact
families**, including the complete Root457 family and all earlier frozen families.
The four previously pinned Source files remain unchanged.

The Root initial alignment-reader stop and worker initial overly broad printf
format-location helper stop remain in their sealed history. Corrected readers do
not alter the object files, executable, probe recipe or recorded process. Early
preparation and raw-readout pending flags are historical snapshots; this final
peer report records completed independent review. Cold EH coverage, GS failures,
private CRT behavior, allocation boundary proof, OOM, ordinary-Original dynamic
execution, class/World lifetime, game startup and gameplay remain unadmitted.

The machine-readable report is
`reports/native_scene_property_record_type4_root_artifact_audit_cc12.json`.
Worker raw outputs, detailed instruction listings, source/manual reviews and
preservation bookends are in this worktree's `local/t4audit20261008a`; `seal.json`
is written last, with absolute artifact paths and only itself excluded from its
manifest. No tests were added; metadata JSON validation and `git diff --check`
are the applicable worker checks. Integration and any Source-credit decision
remain separate primary-agent actions.
