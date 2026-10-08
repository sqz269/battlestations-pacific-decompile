# Parent header clear runtime-code gate audit

Source0; no new admission or runtime validation. The accepted `hc70/r01`31 runtime spans cover **all retained normal-path helper bodies from all five TUs**. Independent artifact-only comparison found32 retained linker-map function symbols at30 unique complete TU bodies; all30 are fully gated. The31st span is the complete14-byte actual MSVCRT cookie helper. All17 retained `probe.obj` functions are included, covering capture/callers, producer, gate/physical-module attestation, serialization and failure helper. Missing normal-path helper bodies:0.

The two standard deleting-destructor names without separate span rows are weak aliases, not missing code: `??_Ebad_alloc` shares the45-byte `??_Gbad_alloc` body at230011A0; `??_Eexception` shares the45-byte `??_Gexception` body at230011D0. All9 retained standard exception function symbols resolve to7 covered bodies. These are retained/gated byte evidence; their exception/failure/class paths were not exercised by the accepted normal clear process and receive no class/EH or Original game ownership admission. Standard library names remain unchanged.

| Exact retained standard symbol | VA | Bytes | COFF ownership | Coverage |
|---|---|---:|---|---|
| `??0bad_alloc@std@@QAE@ABV01@@Z` | `23001100` | 48 | `canonical.obj` | full body gated |
| `??0bad_alloc@std@@QAE@XZ` | `23001130` | 24 | `canonical.obj` | full body gated |
| `??0exception@std@@QAE@ABV01@@Z` | `23001150` | 42 | `canonical.obj` | full body gated |
| `??1bad_alloc@std@@UAE@XZ` | `23001180` | 17 | `canonical.obj` | full body gated |
| `??_Ebad_alloc@std@@UAEPAXI@Z` | `230011A0` | 45 | `canonical.obj` | weak alias to `??_Gbad_alloc@std@@UAEPAXI@Z` |
| `??_Gbad_alloc@std@@UAEPAXI@Z` | `230011A0` | 45 | `canonical.obj` | full body gated |
| `??_Eexception@std@@UAEPAXI@Z` | `230011D0` | 45 | `canonical.obj` | weak alias to `??_Gexception@std@@UAEPAXI@Z` |
| `??_Gexception@std@@UAEPAXI@Z` | `230011D0` | 45 | `canonical.obj` | full body gated |
| `?what@exception@std@@UBEPBDXZ` | `23001270` | 14 | `canonical.obj` | full body gated |

The new parser independently read all five raw I386 COFF objects and checked every saved COFF symbol/section byte against those objects. For each map `f` function owned by leaf/header/append/canonical/probe, it resolved a positive code function section or actual weak alias, matched the complete function-section size and linked PE bytes, normalized only actual COFF relocation operands and decoded the full extent without gaps. No old recipe/COFF parser/static helper was imported or executed. Data, RTTI, strings, alignment and external library/import/failure bodies are distinguished from TU function bodies.

An independent parse of the actual sealed `gate.bin` confirmed code_count31 and every serialized address/size/full-code byte against all31 `static_gate.json` spans. This checks the runtime binary rather than relying only on descriptive JSON. Helpers such as coherent/guards/raw_ok/capture_json/module_unchanged/close_module/read_bytes are not separate retained map bodies in this build; their emitted code is within the complete retained functions. The cookie span includes CMP/JNE/RET and the actual named external failure tail JMP; external `___report_gsfailure` and broad library/EH closure remain unadmitted.

No gap requires worker regeneration. Root's forthcoming **new** independent family should still enumerate every retained function from every fresh TU, including unexecuted standard exceptions, record weak alias names while deduplicating identical VA/size/bytes, and require exact coverage-set equality before launch. Gate complete provider/adapter/capture/normal/failure helpers and the actual map-owned cookie helper; do not use a short selected list or infer class/EH credit from retained byte coverage. No new code-text sketch, probe, compilation, fixture or replay is needed for this no-gap audit.

All417 accepted `hc70/r01` files and16,743 prior pins are unchanged:17,160 exact old pins. The separate callback5 Source0 audit113 also remains unchanged. Current main has the Source70 CPP and passed its build/checks, but Root independent admission remains pending; this audit adds Source0. Only existing COFF/map/gate/PE/probe-text artifacts were read, with no live/native/Ghidra queries, mutation, repair, owner/Class/World expansion, compiler/build/static-stage/process/native execution or accepted-helper replay. Generation dispatch metadata is frozen original context only and is not a current-main hash requirement.

Immutable new family `J:\PROG\battlestations-pacific-decompile-cc12_parent_header_clear_code_gate_audit\local\hc70cov`. Coverage SHA256 `a3e1604a1653542349f5fd06bb31d18888c47ea411e0f92ed36548640b207687`; receipt SHA256 `e0dfbb04798f59c8859c6a4f15c3473dade6d0d50e0e6d4350fa93c644bfd8e3`; manifest SHA256 `04bbdbfce45834c834de92e26161341414538641514287443ba904fb457c46c8`. Exact inventory 44 listed+two seals=46 actual files, including helpers/pycache/frozen inputs and exact-root exclusions only. Complete32-symbol proof stays in the sealed coverage artifact; this report lists9 detailed standard-exception symbols, within the requested20-symbol diagnostic cap.
