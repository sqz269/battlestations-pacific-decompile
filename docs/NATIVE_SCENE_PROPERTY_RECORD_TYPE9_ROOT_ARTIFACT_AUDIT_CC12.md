# Type9 Root artifact peer audit (CC12)

**PASS: independent artifact review; Source credit 0.** The audit read the sealed
`local/t9p1` family and ran new Python artifact readers. It did not launch the
probe, compiler, linker, old helper, accepted Root reader, Native image or Ghidra.
Root's separate qualified Source1 decision is metadata commit
`e02d9a050b194c2eadce9a3b884a9dd7e2ca53ad`. The sealed summary's earlier
Source0/pending state remains historical. This audit changes neither decision
nor shared ledgers.

The reviewed family remains exactly **481 files** (480 artifacts plus seal).
Its 103572-byte seal SHA256 is
`1086ac44ab534b26118b1bc83a577be8a1a1109a11e1db295f574a9e68d855db`.
The audit also preserved all 23,517 Root prior inputs and the 23,691-file union
of prior reader families and their immutable inputs, including the independent
Dup57 27-file and Type9 27-file text families.

## Raw code and ABI evidence

The readers independently decoded the three raw I386 COFF objects, actual map,
I386 PE imports/exports and Gate40 version2 (`CC128E36`, base `3F000000`).
All mapped executable definitions were considered, including EH/cold definitions;
no executable non-function-type definitions happened to be present in this build.

| Quantity | Derived result |
| --- | ---: |
| Mapped executable symbols / physical TU bodies | 31 / 29 |
| Symbol relocation checks / physical operands / alias rechecks | 286 / 280 / 6 |
| Complete nonoverlapping packed code spans | 47 |
| Whole Main bytes / instructions | 6195 / 1570 |
| Raw wrapper bytes / instructions | 191 / 68 |
| Ordinary Source / Original wrappers | 26/7 and 25/7 |

The COFF reader checks complete symbol/auxiliary/section bounds, weak fallback
chains, cycles, actual mapped fallback addresses, DIR32/REL32 addends and every
linked operand. Two actual canonical weak aliases use mode1 NOLIBRARY; mode3
ALIAS is supported without assuming that this build contains one. All bytes of
each linked body match the PE and the packed runtime span. Unknown helpers,
targets or unclassified CFG edges stop the reader. Register import calls require
dataflow through joins and kills. Both ordinary wrappers are retained and fully
decoded; the selected source and actual Main have no invocation of either.

Native `[008EF360,008EF3D4)` is 116 bytes and 36 instructions, SHA256
`dce8fb8621af17a5791a0e80cca91982def8914d2f3a9e516ee1a2344f05b5e3`.
Source and bound Original preserve 108 literal bytes. Only CALL operand ranges
`[51,55)` and `[66,70)` bind to the current private CDECL size adapter and actual
six-byte memcpy IAT thunk. RET pops 12. A symbolic proof of the complete
61-byte/17-instruction adapter ties its true `[entryESP+4]` size to both request
sizes in `{object=3, native_bytes=size, host_bytes=size}`, with plain RET and the
actual allocation result in EAX.

Normal cookie code is 14 bytes plus two CC alignment bytes; its normal path
preserves EAX. The stack probe is 43 bytes/19 instructions plus five CC bytes:
internal `cs10` is offset20, JB22 targets34, RET is33, and JMP41 targets20.
Extents use actual map function-owner `f` records, preserving the internal label.
Cold sized-delete16 -> unsized-delete5 -> free-IAT6 is closed, with the actual
11-byte unsized-delete alignment kept separate. `___report_gsfailure` is the
sole named unexpanded frontier. No cold EH/OOM/GS execution is admitted.

## Main and recorded observations

The focused reader derived the complete Main CFG from those raw bytes: all
1,570 instructions are reachable. Four target slots hold Source, allocated RX
Original, Source and RX Original. The single raw-wrapper call runs in the
four-entry loop. Four root allocations use 56/56-byte object requests; the two
copied children are 16 and four bytes. Fourteen instruction dominance checks
establish the order of code/provider gates, allocation, entries, child frees,
root frees, RX release, final provider checks and success output. Twenty-seven
failure branch paths cannot reach accepted entries, frees or success. The actual
child-free loop bound is two and it precedes the root-free loop of four.

The saved process arguments name the sealed EXE, saved exit is zero, and raw
stdout JSON equals `runtime.json`. Independent byte decoding checks four complete
140-byte captures (108 register bytes plus 16-byte pre/post guards), four entire
56-byte roots, two guarded 48-byte input arrays and both live child byte images.
Observed storage, capture, input and child ranges are pairwise disjoint; each
saved stack frame is disjoint from those ranges. It verifies 29 written and
27 preserved root bytes, four root/two child
allocations, six frees, and the selected loop and saved result's child-before-root
order. This combines raw artifacts with static control flow; it does not create
a new live lifetime trace.

R is captured before the three argument pushes, Q=R-12 and target entry T=Q-4.
COUNT/DATA/FLAG are at T+4/T+8/T+12, read before post-call PUSHFD, and RET12
restores R. Fullword EAX returns the root, EBX/ESI/EDI/EBP are preserved, and DF
is zero. Defined flags use mask `8D5`: retained `44`, copied `ADD(T-24,16)`.
ES values are recorded only. Copied ECX/EDX residuals remain unasserted.

Four recorded CRT bindings (`malloc`, `free`, `_callnewh`, `memcpy`) resolve to
two frozen physical DLL images. The decoder checks their complete SHA256/file
size pins, I386 timestamp/image size, nonforwarded export RVA, recorded IAT/export
addresses and relocation-adjusted 32-byte prefixes against the frozen bytes.
Recorded physical file identity, module bases and before/after checks remain
observations of Root's accepted process. Installed DLL contents were not reopened
and no new provider attestation was performed.

## Build context, stops and preservation

The actual compile `/showIncludes` union independently contains **184 headers**;
its before/after manifests and all frozen header hashes agree. Seven actual
library records and all frozen library hashes agree. The link input names only
the three fresh objects, carries `/MANIFEST:EMBED`, and includes no BSP archive.
The Source TU alone uses `/Oi-`; canonical/probe bodies are independently decoded.

Root's immutable Main build snapshot is revision
`882064808e466755abd99639a3c1d0fe3577a10f`: **4037 inputs**, identical before/after,
and all three existing checks passed (`reconstructed_math`,
`native_math_differential`, `tool_tests`). The log retains the existing LNK4006
duplicate `spawn_request_id_matches` warning. This is sealed historical build
context, not a new build. The summary's nullable header field is a schema-key
mismatch; the actual rows prove184. Its build count is already4037; the earlier
4036 handoff/documentation count was stale.

Three new audit-tool stops are preserved with the attempted readers and outputs:
an audit-added wrong allocator symbol, a PUSH expectation using offset8cc instead
of the actual8d3, and an overstrict assumption that `/Oi-` applied to all TUs.
New revisions corrected these expectations and passed. None changed a Root
artifact, hid a parser stop or exposed an unresolved target-code gap.

The new audit family `J:\PROG\battlestations-pacific-decompile-cc12_type9_root_artifact_audit\local\t9audit` contains exactly
**59 files including its seal**, SHA256
`ab2e3105037f328c96b65b1da3bee7c5618319f3579c6c993ce5d1c3b74dce4c`. The JSON report pins the raw inventories, new
readers, full Main listing, focused proof, decoded captures/providers, build
checks, all three stops and final preservation receipt. Root481 and prior23691
hash/name inventories were checked again before sealing and remain unchanged.

Excluded: new admission by this audit, private Original CRT/EH/OOM/GS behavior,
floating-point element semantics, phase/class ownership, virtual/destructor
dispatch, binary drop-in compatibility, game startup and gameplay validation.
