# Type5 independent reader text correction (CC12)

This correction is **Source 0, text/AST reviewed, and unexecuted**. Exactly six
reader modules are selected. No reader, Capstone decoder, old Root helper, compiler,
target, Native entry, provider identity query or Ghidra operation was executed.
No Source, shared metadata or tests changed, and no future artifact counts,
runtime receipts, standalone duplicate57 admission or game validation are claimed.

## Authoritative selection and preserved inputs

The fresh worker family is this worktree's `local/t5readerfix20261008a`.
`selected_modules.json` is the authoritative module selection;
`reader_inputs_and_limits_v2.json` is the authoritative input/API contract.
Materialization belongs to the primary agent in a fresh Root family.

| Module | Status | SHA-256 |
| --- | --- | --- |
| raw_formats | corrected | `719c65ba0d49394ee891ff377fd80c9a9f57ff3d7c1ef91d5a530165142b369e` |
| register_imports | unchanged | `49944f52c7691c5c5ed9fc0234d4d9811e3e4c1f0f8043c63143db2af4431d7b` |
| caller_frames | unchanged | `3dac624f85852f2023ccd46d0254d29f91d3a09aa26e95c506c200d340dc046e` |
| code_gate | corrected | `6018340ef76afbd167dda4d9551485a72d5ed5bdfb2cab4d7f013205c82a146d` |
| observations | corrected | `f50a70b0e0067412c58b9c009db1fa28ea2c7bbb1af4dfc2b7e91c8d49bf14fa` |
| protocol_review | unchanged | `48139b9cd79f70f8c6c565b7e8ff2f54a86c629ee137e7c450a61d3fb9c1a5d2` |

The selected reader predecessor remains exactly 37 files: 35 relative manifest
entries plus its manifest and receipt. Its receipt SHA-256 is
`ccdb04aa48eea7a283d98c3202b575e04e82fcab16d1a431ec59c8532b912110` and manifest SHA-256 is
`4ed1f54911c780d04771dc2d781855d7c403a4a4ded66dffd192efb8af506490`. The complete-helper predecessor remains exactly
29 files; its receipt SHA-256 is `87a1c57034377cfdf8e4e9d3b1d61346455869aec32bc2bc558d54157e13c62c`.
All seven tracked exact families and all **23,238 strict project-local pins**,
including the current six Source inputs, match before/after. The 203 external
installed/provider/Native associations are retained only as historical metadata;
none of those external paths was freshly opened, hashed or identity-queried.
Root `local/t7p2` and `local/t4p3` fixtures were not read.

The selected probe is byte-identical, SHA-256
`859e8d19d2c5f98ab23f18eadd191b3812f9b7517acde2187b36b0e5195e9b2d`.
The selected recipe_v2 is byte-identical, SHA-256
`b09092f24d5c7ac0bc53319df6c7d588c58e1133efdd84682781174f97e27cc6`.
The unchanged register/import, ordinary-call-frame and protocol modules retain
their previous conservative contracts. Their APIs and `run(family)`/CLI use are
unchanged. PE and Gate parsers gain optional `raw_bytes` arguments so observations
can parse the exact immutable bytes named by their emitted pins.

## Justified reader corrections

COFF now preserves the declared size of an uninitialized section with **no raw
file bytes** when its raw pointer is zero. It cannot fabricate zero-filled code
evidence. Tables, primary/auxiliary indices and names are bounded, and executable
bodies require complete positive raw backing and nonoverlapping entry extents.

Every raw weak symbol now follows its full bounded chain, including unused
symbols. SEARCH_NOLIBRARY (1) and SEARCH_ALIAS (3) keep their distinct recorded
policies; missing/auxiliary indices, unsupported modes, cycles and undefined
terminals stop. Retained code still requires full raw/linked bytes, every relocation
and actual same-VA aliases. Neither weak mode is claimed as tested on a fixture.

External helper starts and next bounds require actual same-section MAP `f` owners;
internal labels such as cs10 cannot shorten a function. The selected helper shapes
remain stack 43 + 5 alignment bytes, cookie 14 + 2, unsized delete 5 + 11,
sized delete 16 and free thunk 6. Full executable raw extents are required.

CFG review now distinguishes an actual post-CxxThrow guard from a maximal,
unreachable trailing NOP/INT3 suffix. Such a suffix must follow a reachable
RET, JMP or proved no-return CxxThrow transfer. Reachable fallthrough is checked
after reachability; reachable traps, UD2 and other unexplained unreachable
instructions still stop. Complete TU gate bytes, full instruction listings and
helper-bound/alignment bytes are retained. Cold byte coverage remains distinct
from execution of exception or GS-failure paths.

Observation decoding no longer calls ctypes/WinAPI identity helpers or opens
installed DLL paths. A static CRT physical pin selects the exact original/frozen
pair in `frozen_inputs.json`; only a contained frozen copy is decoded. Original
FileID and NT-path values remain generation metadata. The precise comparison is
`static_gate.CRT[].identity.physical_NT_path` versus runtime `modules[].mapped_path`.
No frozen-copy FileID is substituted for the original identity.

Every observation input is cached, parsed and pinned as the same byte string,
then rechecked unchanged. This includes static/frozen/runtime JSON, stdout,
stderr, exit and args, the EXE/Gate, captures, storages, guarded texts, children,
Original/bound bytes and frozen provider DLLs. Runtime JSON must equal parsed
stdout; the saved exit must be zero, args/cwd must bind the sole fresh executable,
stderr must be empty, and static EXE/Gate pins must match the parsed bytes. The
additional required files are listed in the authoritative updated input contract.

## Unchanged domain and remaining evidence

The selected probe still uses four fresh TUs (source, duplicate, canonical, probe),
Gate36v1 and Capture132 (100 register bytes, post offset 116). Its stack order is
TEXT at T+4, WORD18 at T+8 and WORD08 at T+12, with RET 12 and T=R-16. There is one
Source and one Original entry, both DF=0; ordinary wrappers remain unexecuted.
There are two 56-byte roots and two 6-byte children, four allocation/free pairs,
and child frees precede root frees. Each guarded input is 40 bytes with an opaque
8-byte array, NUL at offset 5 and both later bytes preserved. The opaque words,
33 written/23 untouched root bytes, ECX/EDX unasserted residuals and XOR flag mask
8C5/value44 with AF excluded are unchanged. No Type7 domain or identity was used.

Selected probe source review confirms the provider timing precisely. Before the
entries it checks MEM_IMAGE/AllocationBase, mapped versus held-handle NT path,
FileID/size, raw SHA, live PE tuple, GetProcAddress/EAT and adjusted live prefix.
After four frees and RX release it repeats the IAT, held-handle FileID/size,
raw SHA and adjusted live prefix checks. It does not repeat MEM_IMAGE, NT-path
query or GetProcAddress afterward. The normalized serialized prefix is an expected
relocated raw prefix compared by the probe against live memory, not a live dump.

AST review confirms unchanged domain constants, capture loops and Source/duplicate
qualification logic. No selected reader function was called and no parser fixture
was evaluated. The primary agent must inspect actual emitted full Main/formals,
CFG/listings, provider order and artifact-specific counts, and must author fresh
generation receipts. Any conservative stop requires preserved evidence and a new
reviewed text revision. This text correction authorizes no process or admission.

The initial metadata bookkeeping stop on an old relative-path seal format is
preserved with its first utility; a new v2 utility normalized those records and
passed. Both old reader text and correction diffs remain in the fresh family.
The machine-readable report is
`reports/cc12_type5_independent_reader_text_correction.json`. The final all-file
seal includes utilities, copies, diffs and the preserved stop, with only itself
excluded from its artifact list. Metadata JSON/hash validation and
`git diff --cached --check` are the applicable final repository checks.
