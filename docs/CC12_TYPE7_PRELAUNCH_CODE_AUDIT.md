# CC12 Type7 independent prelaunch code audit

Independent raw-file and manual code review found no blocking static mismatch
in the selected fresh Root t7p2 bytes. This is **Source0**, with no compiler or
target execution and no prelaunch acceptance. Root owns its separate decision.
Root recipe, accepted readers and helpers were never imported or executed.
No Native/Ghidra/current installed-provider query or Root write occurred.

The worker family is `J:\PROG\battlestations-pacific-decompile-cc12_type7_prelaunch_code_audit\local\t7preaudit`. Its authoritative completion record is
`prelaunch_code_audit_findings.json`; the initial raw summary's pending flags
are closed by the CFG, indirect/trap and manual frame readouts. Seal schema
`bsp-independent-prelaunch-audit-family-seal-v1` covers **157 files**, including every attempt and
any pycache (0 present), with only seal.json itself excluded.
Seal 40626 bytes, SHA256 `13553cc573948e97d64a83e2b595c114f9d170a0c66f90e61a285fa439861837`.

The independently decoded three raw COFF objects contain35 logical mapped code
symbols and33 unique TU bodies. There are243 logical relocation checks and237
physical operand sites, with6 alias rechecks. All33 full TU bodies plus18
classified helpers equal the51 packed Gate spans and linked PE bytes. The
Source63 body has19 instructions and zero relocations; it exactly equals63
bytes re-extracted solely from a copied frozen GAME PE at008EF270. Native SHA256
is `9010f6007c93fa311d1e2d7f6c89a06aa802af3575be8e5c22edd26aa36d6872`.

EXE20480 bytes SHA256
`c891d2dd728e289e286eb41bf06c413fd03679a8df7dcd657feb8017b8740298`;
Gate9093 bytes SHA256
`53572247dead900fcaec97f0b8322258e3522aab1cd3023bd9043846787a2864`.
The Base47000000 PE has DYNAMICBASE clear; every retained span is wholly raw
executable PE data. BSS contributes declared storage, never file-offset0 bytes.
Weak1/3 chains, alias relocations, code-only symbols and the EH handler are
included. Helpers include cookie14 plus excluded2CC, stack43 plus excluded5CC,
full sized-delete16 to unsized-delete5 plus excluded11CC, and free/import6-byte
thunks. Actual next MAP f owners define helper bounds; cs10 is an internal
non-f label. GS failure is the sole named unexpanded local frontier.

All3359 bytes and877 instructions of Main47002270 were manually reviewed.
Independent CFG calculation proves30 selected dominance relationships and51
guard-failure edges in19 failure groups. Failure paths cannot reach either
target site or success JSON. Calls470028A9 and470029C6 lie outside cycles;
Source47001000 withDF1 precedes and dominates unchanged OriginalRX withDF0.
Canonical allocation47002501 receives request[3,56,56] in an exactly-two loop.
Its request host_bytes is the actual malloc/new-handler size in the canonical
allocator. Full disjointness and post-call root/input/capture comparisons
dominate the next stage. Two56-byte roots, both full116-byte captures and both
full44-byte guarded inputs are saved before the exactly-two free loop47002BC7.
The initial provider loop and post-free loop each process three entries; final
code gate47002C5B dominates success JSON47002CD9.

Raw wrapper47003030 is156 bytes/59 instructions. Its target is the first entry
stack formal at[EBP+8]; EBP equals wrapper-entry ESP minus4. Saved B=EBP-20,
target T=B-8, pointerDWORD atT+4 and RET4 restore B. The actual dead argument
slot and value are captured before PUSHFD; only then does CLD restore callerDF0.
All full-word register, seed, stack/capture guard, ES equality and selectedDF
checks were reviewed. Defined flags use mask8C5/value44; AF remains unasserted.
T is derived from captured B, not independently captured. The two ordinary
wrappers are retained/gated and have no Main call edge: Source18B/5I and
Original66B/26I, including5 provably unreachable trailingCC. Its29B/9I EH handler
is retained. Six register-indirect calls have dominating preserved IAT loads;
two constructor calls resolve to entry-stack formals. The one reachable INT3
binds the actual CxxThrow thunk; the five remaining INT3 bytes are full-body
unreachable trailing alignment after RET.

Provider verify972B/303I, relocation-adjusted prefix620B/192I and post244B/82I
were reviewed in full. Packed CRT IATs resolve to actual PE heap imports. One
frozen ucrtbase copy was independently parsed for all three non-forwarded
exports, raw32-byte prefixes, machine/timestamp/image size and physical hash.
No current provider file was opened. Runtime post-free code checks IAT equality,
open-file identity/size, mapped-file hash and adjusted prefix; it does not repeat
MEM_IMAGE, mapped NT path, GetProcAddress or independent live PE header equality.
Those distinctions remain explicit in the manual review and findings.

The audit copied22 chosen direct Root bindings plus frozen GAME and provider
files. All24 Root originals and worker copies match before/after, as do current
Source4 and the selected sealed31-file text family. Root remains unsealed and
this packet makes no final Root inventory claim. Recorded62 generation input,
184 header and7 library rows are metadata context; this worker ran no compiler.
The selected probe and recipe match their original text pins byte-for-byte.

An initial metadata revision script stopped on a text-anchor indentation typo;
a subsequent attempt found its generated file absent. Both stops and the
unexecuted initial parser are retained. Corrected raw, CFG and extra-file audits
passed. Static operation counts describe the selected two canonical roots;
they are not total process heap instrumentation or observed runtime counts.
No ABI replacement, class, startup or gameplay proof is supplied.

See `reports/cc12_type7_prelaunch_code_audit.json` for complete pins, exact
instruction addresses, frame slots, comparison extents and scope boundaries.
