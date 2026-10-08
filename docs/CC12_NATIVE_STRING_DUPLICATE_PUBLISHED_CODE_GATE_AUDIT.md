# CC12 published native string duplicate code-gate audit

**Finding: the older `local/dup57p1` runtime code gate is incomplete.** Its ten
spans preserve the physical duplicate and selected adapters, but omit normal
probe, provider-verifier, and observer code. Recommend reopening or narrowing
that older complete-helper-coverage claim. This read-only audit adds **0 Source**
and changes no existing admission. The later independent `local/t2p5` and
`local/ref48p2` qualifications remain separate and retained.

The published `reports/native_string_duplicate_cc12.json` currently records
Source admission 1 for `00438E40`, qualified to the current successful/null
allocation/copy domain. Its primary publication identifies exactly 95 sealed
artifacts plus `seal.json` (96 files), seal SHA-256
`7a759f86390ca916952a15c7c3bb2508204472e6c2bfb346c68f453f4d85c852`.
The original 12,740 prior pins plus 373 extension pins are 13,113 distinct
path/size/hash associations. These records were preserved without rewriting
their original paths or hashes. A snapshot of the published report accompanies
this audit's new evidence.

## Independent retained-code accounting

Fresh parsers inspect raw COFF symbol/auxiliary records, relocations, the linker
map, the linked PE32/I386 image, and the packed 937-byte gate. They do not import
Root recipes, launch/decode helpers, serialized COFF/static-gate conclusions,
or previous accepted helper implementations. No native process, compiler,
provider, or Ghidra query is performed. Object files are read as data only.

The link retains **28 physical TU bodies / 30 logical function symbols**:
duplicate 2, canonical 9, probe 17. Every retained body is checked at its actual
mapped address over its whole size, all literal bytes, and all relocation
operands. There are **233 distinct physical relocation operands / 239
symbol-level checks**; six checks repeat through two weak names. Another 67
defined COFF functions and five weak records are not retained by this link;
they are recorded separately and are not reported as missing linked bodies.

The two retained weak records use **mode 1,
`IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY`**. This is distinct from mode 3,
`IMAGE_WEAK_EXTERN_SEARCH_ALIAS`. `??_Eexception` selects the actual defined
`??_Gexception` fallback (TargetIndex 292, VA `28001150`, 45 bytes), and
`??_Ebad_alloc` selects `??_Gbad_alloc` (TargetIndex 298, VA `28001120`, 45 bytes).
Both names resolve to the same physical address for each pair; whole fallback
sizes, bytes, and three relocations per body are checked.

The ten gate spans are nonoverlapping and byte-exact, but cover only seven TU
bodies: both duplicate functions, three canonical bodies, and the ordinary/raw
callers. The other three spans are the complete 14-byte cookie helper and the
six-byte `memcpy` and `_CxxThrowException` thunks. The cookie's two trailing
`CC` bytes are padding. A span's completeness does not establish completeness
of the surrounding helper graph.

## Missing linked bodies

Each omitted probe body is listed below. Normal reporting code is included
because it produces the observations on which the old runtime receipt relies.
The `fail` helper is an unexercised harness error reporter, not an EH admission.
Inlined capture checks, byte observations, post-provider checks, and JSON
emission also reside in the omitted whole `main` body.

| Body | VA | Bytes | Classification |
| --- | --- | ---: | --- |
| `main` | `28001FC0` | 3679 | Normal harness; 1004 instructions |
| `read_bytes` | `28001A30` | 56 | Normal gate-file input |
| `read_gate` | `28001A70` | 496 | Normal packed-gate reader |
| `live_code_equal` | `280017A0` | 150 | Normal code bookends |
| `hash_bytes` | `28001640` | 337 | Normal provider hashing |
| `nt_headers` | `28001840` | 81 | Normal provider PE validation |
| `at_rva` | `28001280` | 278 | Normal provider RVA access |
| `physical_export` | `280018A0` | 389 | Normal physical export lookup |
| `export_prefix_equal` | `280013A0` | 620 | Normal relocated-prefix comparison |
| `verify_module` | `28001C60` | 652 | Normal provider verification |
| `add_flags` | `28001200` | 113 | Normal expected-flag calculation |
| `wide_json` | `28001EF0` | 181 | Normal observation serialization |
| `printf` | `28002E30` | 47 | Normal observation serialization |
| `__local_stdio_printf_options` | `28001FB0` | 6 | Normal reporting support |
| `fail` | `28001610` | 38 | Unexercised harness error reporting |

The actual 43-byte `__chkstk`/`__alloca_probe` at `28002F60` is also absent;
`main` calls it at `28001FCB` for its stack frame. Its five trailing `CC` bytes
are padding. Nine normal six-byte import thunks are absent: the seven BCrypt
operations OpenAlgorithmProvider, GetProperty, CloseAlgorithmProvider,
CreateHash, HashData, FinishHash, and DestroyHash (`28002EE0` through
`28002F04`); `K32GetMappedFileNameW` (`28002F0A`); and `memset` (`28003C64`).
Every thunk's actual `FF 25` operand and PE IAT import are checked.

Six omitted canonical physical bodies are cold exception support: exception
copy constructor (`280010D0`, 42 bytes), `what` (`280011F0`, 14), exception
deleting destructor (`28001150`, 45), bad_alloc destructor (`28001100`, 17),
bad_alloc copy constructor (`28001080`, 48), and bad_alloc deleting destructor
(`28001120`, 45). The two omitted six-byte exception copy/destroy thunks are at
`28003C52` / `28003C58`. They are distinct from the normal-path omissions.

The unexpanded cold direct frontier is sized `operator delete`
(`??3@YAXPAXI@Z`, `28002F20`) reached from those exact deleting fallback bodies,
and `___report_gsfailure` (`28003270`) reached by the gated cookie failure tail.
Their target bodies and EH behavior remain unadmitted. The gated throwing
thunk likewise does not establish allocation-failure or unwind behavior.

## Physical interface and recorded observations retained

The published physical routine remains 57 bytes / 30 instructions: actual
borrowed pointer in ECX, incoming EDX not an argument, no target stack arguments,
actual owned result or null in EAX, and plain `RET 0`. All 49 bytes outside the
two four-byte call operands at `[34,38)` and `[44,48)` match the sealed native
bytes. The retained ordinary caller is the complete 9-byte / 2-instruction
`MOV ECX,[ESP+4]; JMP` body, not a `RET 4` substitute.

The complete 61-byte / 17-instruction size adapter reads the genuine retained
size DWORD at its entry `[ESP+4]`, constructs `{3,n,n}`, and calls the unchanged
current canonical allocator. The native duplicate retains that argument until
its final 16-byte cleanup after the actual VCRUNTIME `memcpy` call. Adapter
Native credit is zero. The null path makes neither allocation nor copy call;
the nonnull routine calls the size adapter once and the copy thunk once.
Internal canonical malloc/new-handler retries are preserved without inventing
a dynamic call count or claiming that failure was exercised.

The complete raw wrapper is 128 bytes / 48 instructions. The four serialized
captures each contain **72 bytes**, not an invented larger capture: eight
before registers, eight after registers, and two flags words. Captured caller
ESP `T=001A5728` is unchanged after the plain return; target-entry `T-4` is
inferred from CALL. Nonnull copy-return ESP is `T-32`, and final `ADD 16` yields
`T-16` before saved-register restores and return. Mask `8D5` includes defined
AF for ADD; null XOR uses `8C5`, excluding undefined AF. EBX/EBP/ESI/EDI and raw
caller ESP agree; nonnull volatile ECX/EDX residuals are not prescribed.

The recorded five entries / three results / three frees, 48-byte guarded input,
and eight copied bytes `93 FE 71 6D A4 C2 80 00` are independently cross-checked.
The copies include NUL and precede each matching free; sequential allocation
address reuse is allowed. These are the serialized result/copy observations
actually present, not full receiver snapshots. Main's actual pre/post call
sites and inline observation logic are inspected, but its bytes were not in
the old live code gate.

The four recorded malloc/free/_callnewh/memcpy identities agree with the packed
records and actual PE IAT names, export addresses/RVAs, module bases, I386
machine, file identity/size/SHA, and recorded physical NT paths. The recorded
32-byte relocated prefixes and before/after booleans are preserved. This
audit performs no new live provider attestation; the old verifier and several
of its dependencies were outside the old gate.

## Preservation, recommendation, and limits

Fresh before/after hashes preserve Root's exact 96 files, all 13,113 historical
pins, the Type 2 peer's 44 files, Type 5 draft's 20, Type 9 draft's 16, and the
later Type 2 / reference families' 128 / 120 files. Including the published
report, the union contains 13,538 distinct files. All original associations
remain intact. The new sealed audit family includes both parser stops and
their separate recovery helpers: the first attempted to locate discarded
canonical definitions, and the second attempted a discarded weak fallback.
The successful parser explicitly distinguishes retained from discarded code.

Root should reopen or narrow the older claim of a complete normal/helper code
gate, using the exact omission inventory above. This finding does not revoke
the later independently gated duplicate/canonical/provider/code domains in
`local/t2p5` (seal
`77695fa9e0f34908b7b6d90221f3c688a564a33c5b43691d06c11525ec068eb3`)
and `local/ref48p2` (seal
`dbbe46b953d6087ab99e167df9458678aa39b96234c4890f21c5ed24cd128660`).

Limits remain: current successful/null readable nonwrapping string domain;
DF clear for current CRT calls; no blanket FP/MXCSR/segment/DF preservation;
no forced failure/new-handler reentry or naked-frame unwind; no original
caller integration, private heap/CRT/EH, owning class/destructor/vtable,
game ABI, or gameplay proof. This metadata-only audit changes no Source,
header, CMake, ledger, or Ghidra state and adds no tests.
