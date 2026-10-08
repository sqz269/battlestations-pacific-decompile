# Reference payload copy55 code-gate audit (Source0)

The sealed Root `local/ref55p3` fixture has **no missing separately retained normal-path function body** among its four translation units. All 24 normal fixture/provider bodies have full recorded gates. Six retained canonical exception-support bodies are ungated; they remain outside EH and allocation-failure admission.

This is a read-only audit of existing COFF, linker-map and static-gate metadata. Source admission credit, ready credit and new native registrations are all **0**. No fixture/native process, compiler, prior helper/phase, source semantic query or Ghidra operation was run.

## Complete retained-body inventory

| Object | COFF definitions | Retained bodies | Normal gated | Cold gated | Cold ungated | Unmapped definitions |
|---|---:|---:|---:|---:|---:|---:|
| `consumer.obj` | 1 | 1 | 1 | 0 | 0 | 0 |
| `duplicate.obj` | 2 | 2 | 2 | 0 | 0 | 0 |
| `canonical.obj` | 67 | 9 | 2 | 1 | 6 | 58 |
| `probe.obj` | 32 | 19 | 19 | 0 | 0 | 13 |
| **Total** | **102** | **31** | **24** | **1** | **6** | **71** |

The linker map contains 33 TU function rows: 31 distinct bodies plus two weak aliases. `??_Ebad_alloc@std@@UAEPAXI@Z` and `??_Eexception@std@@UAEPAXI@Z` share their corresponding `??_G` body addresses at `0x34001160` and `0x34001190`. They add no new span. Six `$LN` COFF labels are interior labels, not extra function bodies.

The 38 saved gates comprise 25 complete TU bodies (24 normal and one cold) plus 13 library-helper/import-thunk spans. All saved instruction records concatenate exactly to their stated full byte extents and SHA-256 values. `_main` is **5,279 bytes / 1,295 instructions**, not a prefix.

Each body extent was derived from COFF section size and symbol offsets, then matched to an exact decorated map symbol and object origin. A second new metadata calculation applied **265 DIR32/REL32 relocations** across all 31 retained bodies. Its derived bytes match all 25 gated TU bodies, including all 24 normal bodies. This calculation reads no object or PE implementation bytes and executes no reconstructed code.

## Normal verification and capture helpers

All 19 retained `probe.obj` bodies are covered: stdio-options and printf wrappers; ordinary and raw reference-copy callers; read-bytes, read-gate, live-code equality, hashing, NT-header and RVA checks; physical-export and export-prefix checks; module verification; flags and disjoint-range checks; payload observation; wide JSON formatting; failure reporting; and main.

Thirteen additional probe definitions have no separately retained map body: `__vfprintf_l`, address/pointer helpers, module-unchanged and close-module helpers, put32/get32, capture-ok, initializer-list construction/begin/end, hex formatting and array JSON formatting. They are not missing live standalone gates. Inlined occurrences are within mapped caller spans; an unmapped definition is not assigned an invented live address. The JSON report preserves all 71 unmapped TU definitions and their COFF extents.

## Ungated cold exception support

| Body | Linked address | Bytes |
|---|---:|---:|
| `??0exception@std@@QAE@ABV01@@Z` | `0x34001110` | 42 |
| `?what@exception@std@@UBEPBDXZ` | `0x34001230` | 14 |
| `??_Gexception@std@@UAEPAXI@Z` | `0x34001190` | 45 |
| `??1bad_alloc@std@@UAE@XZ` | `0x34001140` | 17 |
| `??0bad_alloc@std@@QAE@ABV01@@Z` | `0x340010C0` | 48 |
| `??_Gbad_alloc@std@@UAEPAXI@Z` | `0x34001160` | 45 |

The allocator successful-allocation branch at `0x340011DA` reaches the return path at `0x340011FA`–`0x34001200`. Its failure/zero-new-handler branch reaches `0x34001201`, calls the already gated bad_alloc default constructor at `0x34001205`, and calls the already gated CxxThrowException thunk at `0x34001214`. The six ungated bodies have no incoming retained-code relocation; saved vtable and exception metadata account for their retained references. Debug relocation rows are provenance only.

Their names, COFF relocations, map provenance and allocator control flow support the cold classification. No EH, OOM, callback-failure, unwind or cold-destructor execution is admitted. Offline reconstruction of their linked bytes adds no runtime gate.

## Preservation and limits

The audit verifies the exact **101-file Root family** and all **18,016 prior path/size/SHA-256 pins** before and after analysis. The fresh ignored family is `local/cc12_reference_payload_copy_code_gate_audit20261008a`; its final validation and recursive artifact manifest preserve these checks and the two committed outputs. Source/binary paths in the preservation lists are hashed only.

The new link-resolution receipt accidentally used two zero-width characters in one JSON field name. That helper and successful receipt are preserved unchanged; `metadata_schema_correction.json` records the plain-ASCII field name used by the final report. No stage was replayed.

The precise claim is complete gate coverage of retained normal bodies in the four audited TUs. This does not claim that every body in the linked executable or its DLLs is gated. CRT startup and other library/DLL internals remain outside this inventory, and six retained cold TU bodies remain ungated. All 38 original gates, the original accepted process receipt and the family seal remain unchanged.

No C++, shared metadata, registry, ledger, Source count or saved analysis was changed. The existing runtime result is historical evidence; this audit adds metadata consistency and preservation evidence only.

The accompanying `reports/cc12_reference_payload_copy_code_gate_audit.json` contains exact symbols, complete retained-body extents, raw/derived bytes, relocation targets, aliases, map lines, cold references and input hashes.
