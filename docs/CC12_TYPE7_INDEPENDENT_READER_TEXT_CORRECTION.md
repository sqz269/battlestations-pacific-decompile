# Type7 independent reader text correction (CC12)

**Text/AST review complete; Source credit 0.** Six selected reader modules remain
unimported and unexecuted. This packet supplies no approval or prelaunch receipt.
It makes no claim about a new compiled Type7 family. There was no target/compiler
execution, Native/provider/Ghidra query, or read of active Root `local/t4p3`.

The new family is `J:\PROG\battlestations-pacific-decompile-cc12_type7_independent_reader_text_correction\local\t7readerfix20261008a`. It contains exactly
**31 files including seal**, SHA256
`340ac03d590fe9bde5f00440717ebbc6b6c972d6e132a5953266a53cfbba8f73`. Its `bsp-text-only-reader-family-seal-v1`
schema lists absolute path, size and SHA256 for every file except `seal.json`
itself. Failure evidence and the first new code-text revision are included.

## Concrete review and corrections

The original Type7 text already represented BSS correctly: nonzero declared size
with raw pointer0 exposes no file bytes. It also already followed full weak
TagIndex chains for NOLIBRARY1/ALIAS3, rejected cycles, used the next map `f`
function owner instead of internal `cs10`, and consumed the actual
`physical_NT_path` static-identity field. These contracts are preserved.

The corrected raw COFF reader adds explicit header, section, symbol/string,
auxiliary and relocation bounds, rejects relocation references to auxiliary or
missing symbols, and checks defined weak fallbacks. Mapped executable bodies
explicitly require real whole-section file bytes. BSS virtual size never becomes
bytes read from object file offset0.

The original CFG rejected every INT3 that was not immediately a CxxThrow guard.
The new text first determines reachability. A reachable INT3 still requires an
actual bound CxxThrow call. Unreachable INT3 is allowed only in a contiguous
trailing NOP/INT3 suffix immediately after a reachable RET/JMP or validated
no-return guard. Interior arbitrary INT3, UD2, unclassified edges, unknown
helpers and reachable fallthrough still stop. The complete raw extent remains
in the byte inventory, instruction count, full listing and runtime gate.

A separate semantic view is used only for the ordinary/raw wrapper RET checks;
it removes only that already-proven trailing suffix. The first new revision
still used the last raw instruction for those checks, so it is preserved under
`attempts/` with the correction record. No reader was run to discover or resolve
this textual issue. The Type4 padding stop reported by the integrator motivated
the review; this worker did not read that active family or assume its counts
apply to Type7.

The static text emits the complete map inventory and checks internal `cs10` at
stack+20, JB22->34 and the next function-owner extent. Full byte/relocation
inventories, cookie14 plus2CC, stack43 plus5CC, cold delete16->5->free6 and the
explicit GS frontier remain covered by the future reader contract. Counts are
explicitly separated into logical symbols/relocations, distinct TU bodies and
physical operands, alias rechecks, complete gate spans and mapped handler
identities. Every actual count awaits Root's fresh artifacts.

The recorded decoder now pins `static_gate.json`, `frozen_inputs.json` and
`runtime.json` from the exact bytes parsed, reports the pins and checks they
remain unchanged. It checks saved stdout JSON, process exit0 and arguments,
static gate/executable pins and actual span address/size consistency. Provider
PE reads use validated frozen paths only. It retains `physical_NT_path` and
distinguishes static expected file identity/frozen PE fields from the probe's
serialized runtime observations. Live file identity and live PE headers are
not separately serialized. Allocation/free counts denote the selected canonical
root operations; they are not an instrumented process-wide heap trace.

## Unchanged selection and adoption

The selected 24-file reader family and 17-file complete-helper family are
unchanged. Their manifests remain respectively
`024c75de0861042230b6fe140722f2033dcd7304bbe702912251f4d2a7870211` and
`43d2bc3ffe3ddc54db10287336eaf93e775035654b3da82a0b0fb71d399fe7d5`.
Selected probe SHA256 remains
`f81298a5fd0e3c1ef1f3b7e78bbf93bc41b11731ebf316b67036c1cb987c126e`;
selected recipe remains
`53cc10fa004029f613281e450847a9dac5c4844832318599b4362dc8ba101e99`.

Base47000000, `t7_probe`, Gate32v2/CC128E27, Capture116/Registers84, two56-byte
roots, two44-byte guarded inputs and opaque12-byte payloads remain unchanged.
Source's selected contract is63 literal bytes/19 instructions/no relocations,
one pointer DWORD at T+4 and RET4. T remains derived from B-8, not independently
captured. Source DF1 and Original DF0, full EAX root, ECX0, opaque middle-word
EDX, ES equality, flags mask8C5/value44 and unasserted undefined AF are unchanged.
These are selected text contracts, not fresh binary/runtime measurements.

Only raw_formats, type7_code_reader and type7_capture_decoder changed. The
register/formal helpers, identity module, 18-key identity template and selected
probe contract are byte-identical to the original text family. External APIs
remain `read_static(final_family_path, identity_dict)` and
`read_recorded(final_family_path, identity_dict)`. `selected_modules.json` is the
authoritative final selection; earlier derivation records are history.

| Selected module text | Bytes | SHA256 |
| --- | ---: | --- |
| raw_formats.py.txt | 9404 | `2dd7e322765b8ef9e1a227d7801880b3c15dcb7cf36af5923df9e506897455f7` |
| register_imports.py.txt | 3045 | `8c8f38af5e47df3c0c8ceadf513eb5f3b29eb3504021128f8144cd8e24c919c4` |
| formal_targets.py.txt | 5112 | `a9fdf5e083d74687a50d1fcf5b8b9c65f56fa2f54266a6dedb45e6ff2b31d7f2` |
| type7_identity.py.txt | 2153 | `bbcf891f6578f0c78c4161f9822194e9b18f1ae68f68a7b9c0634be95eadeb9c` |
| type7_code_reader.py.txt | 32152 | `d91beba0cb1e6d2d8d416b9284709acf27e934cae6b2eeca237e36ab18cd2f3f` |
| type7_capture_decoder.py.txt | 14796 | `da8ea9302d843ddb689f3be2e2b0159c46ec95eebac9353d9ead7f43ae6a3fd0` |

Root must copy these texts into a fresh family, pin the adopted scripts/config,
derive actual code and provider artifacts and perform the full Main review.
The recorded decoder is for saved post-process observations only. No worker
authored acceptance or prelaunch receipt is supplied.

## Preservation and validation

All six final modules passed AST parsing and structural inspection without
imports or execution. The selected probe/recipe/layout and literal payloads
are unchanged. Exact selected-family inventories and25 underlying old family
inventories passed before/after checks; **23,379 project-local immutable files**
and strict current Source4 hashes agree.

The old23,556-row inventory embeds201 external installed toolchain, header,
library, provider and original-Native paths. The first metadata pin attempt
stopped at the external-path guard before reading the external file. That stop
is preserved. Those201 records are retained as frozen historical metadata and
were not reopened or rehashed. Seven prior metadata snapshots likewise remain
historical copies; mutable current Main metadata is not an immutable input.

No C++/CMake/shared ledger or Ghidra edit was made. Root materialization,
actual reader execution, prelaunch review, Source admission, startup and game
validation remain separate work.
