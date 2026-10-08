# CC12 Type8 independent reader text correction

This packet supplies six AST-reviewed reader texts for future Root artifacts.
It remains **Source0**: no reader import/execution, C++ compilation, target or
Native execution, provider query, Ghidra query/mutation, acceptance or prelaunch
receipt. Reserved Root t4p3 and active t7p2 were not read.

Family: `J:\PROG\battlestations-pacific-decompile-cc12_type8_independent_reader_text_correction\local\t8readerfix20261008a`. The authoritative selection is `selected_modules.json` and the
self-contained adoption guide is `HANDOFF.txt`. Public calls remain
`read_static(final_family_path, identity_dict)` and
`read_recorded(final_family_path, identity_dict)`; the 18-key identity template
and selected probe contract are byte-identical to the original Type8 reader.

Seal schema `bsp-text-only-reader-family-seal-v1`, exact recursive count **23**, only
exclusion `seal.json` itself. Seal 6401 bytes,
SHA256 `245a5d17b310a64b011cd0a44c06950e2c0f3289c72735dd5e431e2fc8a77331`. This doc and report are outside the local family.

| Selected module | Bytes | SHA256 |
|---|---:|---|
| raw_formats.py.txt | 9404 | `2dd7e322765b8ef9e1a227d7801880b3c15dcb7cf36af5923df9e506897455f7` |
| register_imports.py.txt | 3045 | `8c8f38af5e47df3c0c8ceadf513eb5f3b29eb3504021128f8144cd8e24c919c4` |
| formal_targets.py.txt | 5126 | `fae4e4bc776cbcef1514e8c117fe10bd8a1f8cca85cf4501b369e5a7d920e18e` |
| type8_identity.py.txt | 2166 | `c6f2fa1cfd09df4d32f9c73632718cc13140ef91083e03c0650964591c953856` |
| type8_code_reader.py.txt | 32907 | `845c05face16801f33bf174699e780dbf046b0ff9ebc6db69e187398bfac559e` |
| type8_capture_decoder.py.txt | 17437 | `57bcc8ef79b311c8ca1432005d540e250c0d47358491d8e869d7c897a3405fd7` |

Three texts changed: raw_formats, type8_code_reader and type8_capture_decoder.
The register binding, formal target and Type8 identity modules are unchanged.
The common parser and generic CFG text were copied as text from the separately
sealed Type7 correction; no Type7 argument, capture or payload contract was used.

The COFF parser now bounds all file tables, retains BSS declared storage without
fabricated raw bytes, verifies primary symbol relocation references and retains
weak1/3 TagIndex chain/cycle checks. Executable sections require complete file
bytes. The static reader retains all three TU bodies, aliases and handlers,
relocations, recursively classified helpers and cold delete/free edges. The
existing actual next MAP f-owner bounds remain; cs10 and the stack JB edge are
explicitly checked, and a complete MAP inventory is emitted.

The CFG separates reachable CxxThrow-bound INT3 guards from a maximal contiguous
unreachable trailing NOP/INT3 suffix immediately following a reachable RET/JMP
or validated no-return guard. Interior INT3, UD2, unknown edges and reachable
fallout still stop. Full bytes, extents, counts and listings remain intact;
only wrapper terminal-RET assertions use the validated semantic view.

The recorded decoder pins the exact parsed bytes for static_gate, frozen_inputs,
runtime, process stdout, process exit and process args and rechecks all six.
Saved stdout must match runtime JSON, exit must be zero, and args/cwd must name
the selected Root EXE. Static span addresses/sizes must match the packed Gate.
Provider reads remain limited to validated frozen paths with physical_NT_path.
Actual serialized fields are distinguished from expected frozen PE/file identity.
The six reported allocations/frees describe four roots and two copied children;
they do not measure all process heap operations.

Type8 remains Base49/t8_probe, Gate40v2, three TUs with its private CDECL Source
byte-allocation adapter and canonical bridge. Source112B/34I has104 literal
bytes and CALL operands47..50/62..65. DATA/COUNT/FLAG are T+4/+8/+12 with RET12.
Four56-byte roots, copied children11/7, Capture140/Registers108, two guarded
inputs48, opaque payload16 and byte counts7/11/11/7 are unchanged. DF0/0,
full EAX roots, mask8d5, ES recorded-only and copied ECX/EDX unasserted remain.
Every protocol assignment is AST-identical, and both the Source112 domain and
capture-domain checks are text-identical. Type9 COUNT-first rules do not apply.

Preservation before/after passed for **23436** project-local files and current
Source4. Selected old reader26/helper19 and reused generic-text31 inventories
match exactly, as do26 underlying old family inventories. The201 external
installed-toolchain/provider/Native rows embedded in the old23580-row inventory
remain frozen history and were never reopened. Nine old metadata copies and
all old failure/revision artifacts remain intact. No new generation failure
occurred; readers were never run.

Root still owns the complete Main review, private adapter request size and
canonical routing, all targets/order/DF/gates/comparisons/failures, allocation
and child-before-root lifetime order, ordinary wrappers/handlers and cold
frontiers. Actual compiled symbols, bodies, relocations, aliases, helper spans,
Main and handler counts remain unknown until fresh Root artifacts are read.
This text packet provides no static build, ABI, startup or gameplay proof.

The selected probe remains29906 bytes SHA256
`4b50d1f627bfcc19a9cc2a1cd03c6d6b519342bf848dce18e5acae26a93f08d4`;
the selected recipe remains54002 bytes SHA256
`96aa02347e03b5cbafbbaf8d766ba613d3656eece578d5b8659ec50e109247ed`.
See `reports/cc12_type8_independent_reader_text_correction.json` for pins and
machine-readable preservation/text-validation evidence.
