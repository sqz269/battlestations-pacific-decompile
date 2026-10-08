# Root clear70 normal linked-helper coverage audit

Published `Main/local/cl70p2` has **nine normal6-byte import thunks omitted** from its runtime code gate: memset, seven BCrypt APIs and K32GetMappedFileNameW. All**32 retained symbols/30 distinct fresh5TU bodies** are whole-gated, including all17 probe bodies and full4613-byte main. The cookie14 and stack map-span48 (actual43 code bytes+5 alignment) are also gated, giving the existing32 runtime spans. This factual **Source0 audit** recommends Root review of normal observer thunk coverage; it changes no credit, Source or shared files and makes no broad prerequisite revocation.

The complete raw clear `[004BF8E0,004BF926)` remains **70B/29 instructions**, SHA256`d2bcd01aa9aa3c30eae420c969b256957f769149e7e8bc59be5029d2e473c9d3`. Independent raw COFF/map/linked PE and serialized gate agree for the whole body. All66 bytes outside the sole CALL operand[56,60) match frozen Native exactly; that actual REL32 operand binds to the real current canonical Source free. Header13, append87/size adapter, canonical allocation/free, ordinary/raw/producer/capture helpers and full main belong to this fresh five-TU family. Physical clear/construct bodies and later independently gated families are separate evidence domains.

Every direct omission from retained TU calls is classified below. Each thunk is a complete single FF25 indirect JMP with a verified physical IAT operand to a real PE import; external DLL implementation bodies are not expanded or credited.

| Symbol | Linked VA | Bytes | Classification |
|---|---|---:|---|
| `__CxxThrowException@8` | `39004148` | 6 | cold_std_EH_import_thunk |
| `___std_exception_copy` | `3900413C` | 6 | cold_std_EH_import_thunk |
| `___std_exception_destroy` | `39004142` | 6 | cold_std_EH_import_thunk |
| `_memset` | `3900414E` | 6 | normal_import_thunk |
| `_BCryptOpenAlgorithmProvider@16` | `390033CC` | 6 | normal_import_thunk |
| `_BCryptGetProperty@24` | `390033D2` | 6 | normal_import_thunk |
| `_BCryptCreateHash@28` | `390033DE` | 6 | normal_import_thunk |
| `_BCryptHashData@16` | `390033E4` | 6 | normal_import_thunk |
| `_BCryptFinishHash@16` | `390033EA` | 6 | normal_import_thunk |
| `_BCryptDestroyHash@4` | `390033F0` | 6 | normal_import_thunk |
| `_BCryptCloseAlgorithmProvider@8` | `390033D8` | 6 | normal_import_thunk |
| `_K32GetMappedFileNameW@16` | `390033F6` | 6 | normal_import_thunk |

The normal paths are grounded by actual linked direct edges `main -> verify_module/hash_bytes -> BCrypt/K32MappedFileName`, `main/hash_bytes -> memset`, plus the successful saved module/file/hash/capture checks. Static control-flow reachability is distinguished from observed success: CxxThrowException and std exception copy/destroy belong to unexecuted allocation-failure/EH paths. The canonical deleting-destructor sized-delete library edge and cookie's named GS-report tail remain explicit cold unexpanded frontiers; no generic EH/class closure is invented. This audit focuses on normal observer imports and preserves those cold qualifications.

Two physically shared weak external fallbacks are `_Eexception -> _Gexception` and `_Ebad_alloc -> _Gbad_alloc`. Raw TagIndex and characteristic**1=SEARCH_NOLIBRARY** are decoded without treating1 as3=SEARCH_ALIAS. Actual same mapped VA/full object and linked bytes/logical relocation identity are verified. **366 symbol relocation checks /360 distinct physical operand sites** distinguish alias repetitions from physical code. Full emitted function extents and all32 serialized gate spans are verified byte-for-byte against the linked PE; no clipped prefix/opcode-only scan supplies coverage.

All five saved88-byte captures and entire44-byte allocations decode: empty Source/Original preserve EAX/ECX/EDX; populated volatile outputs stay unasserted; nonvolatiles/ES/ESP/stack canaries/DF0 and terminal CMP mask8D5=44 includingAF0 match. Boxed capture outer guards were checked live by the old probe and are not claimed independently serialized. Seven genuine append-node producer rows and all96 borrowed live payload bytes also match. Three actual44 caller allocation bases and seven actual12 nodes provide ten canonical allocation/free pairs across five clear entries. Current coherent Source-produced nodes and header12 interiors with16+16 guards inside the allocations remain the entire qualified domain. No retired memory reads or global historical address uniqueness is claimed.

Historical actual I386 malloc/free/new-handler IAT/export/MEM_IMAGE/NTfileID/fullSHA/normalized32 bookends are preserved; no new provider query or process runs. Original private heap/CRT, parent97 array, sentinel, iterator/EH, native owning class/World/game remain unadmitted. No Source/Ghidra/ledger/CMake mutation, compiler, old helper/stage replay or new test is performed. In particular, this report does not revoke the later h5s47-span domain or infer its helper coverage from the old Root32 spans.

Root's published report supplies exact376 artifacts+seal377, prior19194 and sealSHA`f1610498b9e36acf6193ea54b80e110350754b3843bf63f1ad6da54241fac630`. Header13 family134, worker callback424, pure-text draft21, Header13 audit22 and all historical pins are preserved in the deduplicated **21365-pin** before/after union. Historical external Root sealer is separately checked. Mutable Main publication is frozen at generation, not asserted live after unrelated updates.

Ignored audit family `J:/PROG/battlestations-pacific-decompile-cc12_parent_header_clear_root_code_gate_audit/local/cl70cov` has exact**19 listed+2 seals=21 actual files**, including every new helper/raw parser/frozen copy/log and any errors/pycache, with exact root receipt/manifest exclusions only. Receipt SHA256`7dce316f63cf6a41b48fdb5d25ca10406616e01ee23d02c2dc0e1cd6dd91100f`; manifest SHA256`6f6e031324b5b49ba2aa9a400224152d1202ab4a4899ed82a7bef32625c842b4`. The report links complete per-symbol byte/relocation/control-edge analysis and every decoded saved observation. Root alone chooses credit correction and any future complete-gate family.
